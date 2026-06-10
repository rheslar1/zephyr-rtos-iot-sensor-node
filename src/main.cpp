#include "sensor_node/SensorNode.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

void printUsage(const char* programName) {
  std::cout << "Usage: " << programName << " [mode]\n\n"
            << "Modes:\n"
            << "  --cycle              Run the nominal accepted sensor cycle\n"
            << "  --low-battery        Run a cycle with a rejected low-battery sample\n"
            << "  --offline            Run a cycle with telemetry transport offline\n"
            << "  --bad-cert           Run a cycle with invalid provisioning material\n"
            << "  --tight-power        Run a cycle that exceeds the current budget\n"
            << "  --unconfirmed-image  Run a cycle with an unconfirmed MCUboot image\n"
            << "  --help               Show this help text\n";
}

}  // namespace

int main(int argc, char** argv) {
  const std::string mode = argc > 1 ? argv[1] : "--cycle";
  if (mode == "--help") {
    printUsage(argv[0]);
    return 0;
  }

  auto board = sensor_node::nrf52840DevelopmentKit();
  sensor_node::NodeConfig config{
      "rheslar-nrf52840-001",
      "2026.06.10-nrf52840",
      60000U,
      3200U,
      70.0,
      35.0,
      220U};
  auto image = sensor_node::demoImage();
  auto sample = sensor_node::demoSample();
  auto provisioningRecord = sensor_node::demoProvisioning();
  bool transportOnline = true;

  if (mode == "--low-battery") {
    sample.batteryMv = 2990U;
  } else if (mode == "--offline") {
    transportOnline = false;
  } else if (mode == "--bad-cert") {
    provisioningRecord.certificateFingerprint = "not-a-sha256";
  } else if (mode == "--tight-power") {
    config.maxAverageCurrentUa = 3.0;
  } else if (mode == "--unconfirmed-image") {
    image.confirmed = false;
  } else if (mode != "--cycle") {
    printUsage(argv[0]);
    return 1;
  }

  sensor_node::ScriptedSensorReader sensors({sample});
  sensor_node::StaticProvisioningStore provisioning(provisioningRecord);
  sensor_node::CapturingTelemetryTransport transport(transportOnline);
  sensor_node::McubootUpdateValidator updateValidator(40U);
  sensor_node::Nrf52840PowerProfiler powerProfiler;
  sensor_node::ZephyrSensorNode node(board,
                                     config,
                                     image,
                                     sensors,
                                     provisioning,
                                     transport,
                                     updateValidator,
                                     powerProfiler);
  sensor_node::TextCycleReporter reporter(std::cout);

  std::cout << "Zephyr RTOS IoT Sensor Node\n";
  std::cout << "Board target: " << board.boardTarget << '\n';
  std::cout << "SoC: " << board.soc << '\n';
  std::cout << "Radio: BLE + 802.15.4";
  std::cout << ", Wi-Fi via external backhaul when provisioned\n\n";

  const auto result = node.runCycle();
  reporter.publish(result);

  return result.accepted ? 0 : 1;
}
