#include "sensor_node/SensorNode.hpp"

#include <cassert>
#include <sstream>
#include <string>

namespace {

bool contains(const std::string& value, const std::string& needle) {
  return value.find(needle) != std::string::npos;
}

sensor_node::NodeConfig config() {
  return sensor_node::NodeConfig{
      "rheslar-nrf52840-001",
      "2026.06.10-nrf52840",
      60000U,
      3200U,
      70.0,
      35.0,
      220U};
}

sensor_node::CycleResult runOnce(
    const sensor_node::BoardProfile& board,
    const sensor_node::NodeConfig& nodeConfig,
    const sensor_node::OtaImageMetadata& image,
    const sensor_node::SensorSample& sample,
    const sensor_node::ProvisioningRecord& provisioning,
    sensor_node::CapturingTelemetryTransport& transport) {
  sensor_node::ScriptedSensorReader sensors({sample});
  sensor_node::StaticProvisioningStore store(provisioning);
  sensor_node::McubootUpdateValidator updateValidator(40U);
  sensor_node::Nrf52840PowerProfiler powerProfiler;
  sensor_node::ZephyrSensorNode node(board,
                                     nodeConfig,
                                     image,
                                     sensors,
                                     store,
                                     transport,
                                     updateValidator,
                                     powerProfiler);
  return node.runCycle();
}

void acceptsNominalNrf52840Cycle() {
  sensor_node::CapturingTelemetryTransport transport;
  const auto result = runOnce(sensor_node::nrf52840DevelopmentKit(),
                              config(),
                              sensor_node::demoImage(),
                              sensor_node::demoSample(),
                              sensor_node::demoProvisioning(),
                              transport);

  assert(result.accepted);
  assert(result.failureReason == "cycle accepted");
  assert(result.events.size() == 7U);
  assert(transport.frames().size() == 1U);
  assert(contains(transport.frames().front().payload, "nrf52840dk/nrf52840"));
  assert(result.power.averageCurrentUa <= config().maxAverageCurrentUa);
}

void rejectsUnsupportedBoardTarget() {
  auto board = sensor_node::nrf52840DevelopmentKit();
  board.boardTarget = "native_sim";
  sensor_node::CapturingTelemetryTransport transport;

  const auto result = runOnce(board,
                              config(),
                              sensor_node::demoImage(),
                              sensor_node::demoSample(),
                              sensor_node::demoProvisioning(),
                              transport);

  assert(!result.accepted);
  assert(contains(result.failureReason, "expected Zephyr board target"));
  assert(transport.frames().empty());
}

void rejectsUnsignedOrUnconfirmedImage() {
  auto image = sensor_node::demoImage();
  image.signatureValid = false;
  sensor_node::CapturingTelemetryTransport transport;

  const auto result = runOnce(sensor_node::nrf52840DevelopmentKit(),
                              config(),
                              image,
                              sensor_node::demoSample(),
                              sensor_node::demoProvisioning(),
                              transport);

  assert(!result.accepted);
  assert(contains(result.failureReason, "signature"));
  assert(transport.frames().empty());
}

void rejectsNativeWifiAssumption() {
  auto provisioning = sensor_node::demoProvisioning();
  provisioning.externalWifiBackhaul = false;
  sensor_node::CapturingTelemetryTransport transport;

  const auto result = runOnce(sensor_node::nrf52840DevelopmentKit(),
                              config(),
                              sensor_node::demoImage(),
                              sensor_node::demoSample(),
                              provisioning,
                              transport);

  assert(!result.accepted);
  assert(contains(result.failureReason, "no native Wi-Fi"));
}

void rejectsLowBatterySample() {
  auto sample = sensor_node::demoSample();
  sample.batteryMv = 2990U;
  sensor_node::CapturingTelemetryTransport transport;

  const auto result = runOnce(sensor_node::nrf52840DevelopmentKit(),
                              config(),
                              sensor_node::demoImage(),
                              sample,
                              sensor_node::demoProvisioning(),
                              transport);

  assert(!result.accepted);
  assert(contains(result.failureReason, "battery"));
}

void reportsTelemetryTransportFailure() {
  sensor_node::CapturingTelemetryTransport transport(false);

  const auto result = runOnce(sensor_node::nrf52840DevelopmentKit(),
                              config(),
                              sensor_node::demoImage(),
                              sensor_node::demoSample(),
                              sensor_node::demoProvisioning(),
                              transport);

  assert(!result.accepted);
  assert(contains(result.failureReason, "offline"));
}

void reporterIncludesPowerEvidence() {
  sensor_node::CapturingTelemetryTransport transport;
  const auto result = runOnce(sensor_node::nrf52840DevelopmentKit(),
                              config(),
                              sensor_node::demoImage(),
                              sensor_node::demoSample(),
                              sensor_node::demoProvisioning(),
                              transport);
  std::ostringstream output;
  sensor_node::TextCycleReporter reporter(output);
  reporter.publish(result);

  assert(contains(output.str(), "cycle=PASS"));
  assert(contains(output.str(), "power-profiler"));
  assert(contains(output.str(), "projected_days"));
}

}  // namespace

int main() {
  acceptsNominalNrf52840Cycle();
  rejectsUnsupportedBoardTarget();
  rejectsUnsignedOrUnconfirmedImage();
  rejectsNativeWifiAssumption();
  rejectsLowBatterySample();
  reportsTelemetryTransportFailure();
  reporterIncludesPowerEvidence();
  return 0;
}
