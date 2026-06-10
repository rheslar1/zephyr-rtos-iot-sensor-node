#include "sensor_node/SensorNode.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace sensor_node {
namespace {

bool startsWith(std::string_view value, std::string_view prefix) {
  return value.substr(0U, prefix.size()) == prefix;
}

bool isSha256(std::string_view value) {
  return value.size() == 64U &&
         std::all_of(value.begin(), value.end(), [](const char item) {
           return std::isxdigit(static_cast<unsigned char>(item)) != 0;
         });
}

bool validEndpoint(std::string_view endpoint) {
  return startsWith(endpoint, "mqtts://") || startsWith(endpoint, "https://");
}

std::string fixed1(const double value) {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(1) << value;
  return stream.str();
}

std::string fixed2(const double value) {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2) << value;
  return stream.str();
}

void addEvent(CycleResult& result,
              std::string step,
              const bool passed,
              std::string detail) {
  result.events.push_back(
      CycleEvent{std::move(step),
                 passed ? CycleStatus::Passed : CycleStatus::Failed,
                 std::move(detail)});
}

TelemetryFrame encodeTelemetry(const BoardProfile& board,
                               const NodeConfig& config,
                               const SensorSample& sample) {
  std::ostringstream payload;
  payload << "{"
          << "\"device\":\"" << config.deviceId << "\","
          << "\"board\":\"" << board.boardTarget << "\","
          << "\"firmware\":\"" << config.firmwareVersion << "\","
          << "\"uptime_ms\":" << sample.uptimeMs << ","
          << "\"temperature_c\":" << fixed1(sample.temperatureC) << ","
          << "\"humidity_pct\":" << fixed1(sample.humidityPct) << ","
          << "\"pressure_kpa\":" << fixed2(sample.pressureKpa) << ","
          << "\"battery_mv\":" << sample.batteryMv << ","
          << "\"acceleration_mg\":" << sample.accelerationMg << "}";

  return TelemetryFrame{
      "devices/" + config.deviceId + "/telemetry",
      payload.str(),
      1U};
}

bool validateProvisioning(const BoardProfile& board,
                          const ProvisioningRecord& provisioning,
                          std::string& reason) {
  if (!provisioning.bleBonded) {
    reason = "BLE provisioning bond is missing";
    return false;
  }

  if (!validEndpoint(provisioning.endpointUri)) {
    reason = "telemetry endpoint must use mqtts:// or https://";
    return false;
  }

  if (!isSha256(provisioning.certificateFingerprint)) {
    reason = "certificate fingerprint must be a 64-character hex digest";
    return false;
  }

  if (!provisioning.wifiSsid.empty() && !board.hasNativeWifi &&
      !provisioning.externalWifiBackhaul) {
    reason = board.boardTarget +
             " has no native Wi-Fi; set external Wi-Fi backhaul for SSID " +
             provisioning.wifiSsid;
    return false;
  }

  reason = "BLE bond present, secure endpoint configured";
  if (provisioning.externalWifiBackhaul) {
    reason += ", external Wi-Fi backhaul enabled";
  }
  return true;
}

bool validateSample(const NodeConfig& config,
                    const SensorSample& sample,
                    std::string& reason) {
  if (sample.batteryMv < config.minBatteryMv) {
    reason = "battery " + std::to_string(sample.batteryMv) +
             " mV is below " + std::to_string(config.minBatteryMv) + " mV";
    return false;
  }

  if (sample.temperatureC > config.maxTemperatureC) {
    reason = "temperature " + fixed1(sample.temperatureC) + " C exceeds " +
             fixed1(config.maxTemperatureC) + " C";
    return false;
  }

  if (sample.humidityPct < 0.0 || sample.humidityPct > 100.0) {
    reason = "humidity " + fixed1(sample.humidityPct) +
             " percent is outside sensor range";
    return false;
  }

  reason = "sample accepted at " + fixed1(sample.temperatureC) + " C, " +
           std::to_string(sample.batteryMv) + " mV";
  return true;
}

bool validateBoard(const BoardProfile& board, std::string& reason) {
  if (board.boardTarget != "nrf52840dk/nrf52840") {
    reason = "expected Zephyr board target nrf52840dk/nrf52840, got " +
             board.boardTarget;
    return false;
  }

  if (!board.hasBle || !board.hasSaadc) {
    reason = "nRF52840 profile requires BLE and SAADC support";
    return false;
  }

  reason = board.boardName + " with " + std::to_string(board.ramKib) +
           " KiB RAM and " + std::to_string(board.flashKib) + " KiB flash";
  return true;
}

}  // namespace

std::string toString(const CycleStatus status) {
  return status == CycleStatus::Passed ? "PASS" : "FAIL";
}

BoardProfile nrf52840DevelopmentKit() {
  return BoardProfile{
      "nrf52840dk/nrf52840",
      "Nordic nRF52840 Development Kit",
      "nRF52840 Cortex-M4F",
      256U,
      1024U,
      true,
      true,
      false,
      true};
}

ZephyrSensorNode::ZephyrSensorNode(BoardProfile board,
                                   NodeConfig config,
                                   OtaImageMetadata runningImage,
                                   ISensorReader& sensorReader,
                                   IProvisioningStore& provisioningStore,
                                   ITelemetryTransport& telemetryTransport,
                                   IUpdateValidator& updateValidator,
                                   IPowerProfiler& powerProfiler)
    : board_(std::move(board)),
      config_(std::move(config)),
      runningImage_(std::move(runningImage)),
      sensorReader_(sensorReader),
      provisioningStore_(provisioningStore),
      telemetryTransport_(telemetryTransport),
      updateValidator_(updateValidator),
      powerProfiler_(powerProfiler) {}

CycleResult ZephyrSensorNode::runCycle() {
  CycleResult result;

  auto fail = [&](std::string reason) {
    result.accepted = false;
    result.failureReason = std::move(reason);
    return result;
  };

  std::string reason;
  const bool boardOk = validateBoard(board_, reason);
  addEvent(result, "board-profile", boardOk, reason);
  if (!boardOk) {
    return fail(reason);
  }

  const bool imageOk = updateValidator_.validate(runningImage_, board_, reason);
  addEvent(result, "mcuboot-image", imageOk, reason);
  if (!imageOk) {
    return fail(reason);
  }

  const auto provisioning = provisioningStore_.current();
  const bool provisioningOk = validateProvisioning(board_, provisioning, reason);
  addEvent(result, "ble-provisioning", provisioningOk, reason);
  if (!provisioningOk) {
    return fail(reason);
  }

  result.sample = sensorReader_.read();
  const bool sampleOk = validateSample(config_, result.sample, reason);
  addEvent(result, "sensor-thread", sampleOk, reason);
  if (!sampleOk) {
    return fail(reason);
  }

  result.telemetry = encodeTelemetry(board_, config_, result.sample);
  const bool telemetryOk = telemetryTransport_.publish(result.telemetry);
  reason = telemetryOk ? "published QoS1 telemetry to " + provisioning.endpointUri
                       : telemetryTransport_.lastError();
  addEvent(result, "telemetry-thread", telemetryOk, reason);
  if (!telemetryOk) {
    return fail(reason);
  }

  result.power = powerProfiler_.estimate(config_, board_, result.sample);
  const bool powerOk = result.power.averageCurrentUa <= config_.maxAverageCurrentUa;
  reason = "average " + fixed2(result.power.averageCurrentUa) +
           " uA, projected " + fixed1(result.power.projectedBatteryDays) +
           " days";
  addEvent(result, "power-profiler", powerOk, reason);
  if (!powerOk) {
    return fail(reason);
  }

  result.accepted = true;
  result.failureReason = "cycle accepted";
  addEvent(result,
           "sleep-transition",
           true,
           "entered low-power wait for " +
               std::to_string(result.power.sleepMs) + " ms");
  return result;
}

ScriptedSensorReader::ScriptedSensorReader(std::vector<SensorSample> samples)
    : samples_(std::move(samples)) {
  if (samples_.empty()) {
    throw std::invalid_argument("ScriptedSensorReader requires samples");
  }
}

SensorSample ScriptedSensorReader::read() {
  const auto sample = samples_[nextIndex_];
  nextIndex_ = (nextIndex_ + 1U) % samples_.size();
  return sample;
}

StaticProvisioningStore::StaticProvisioningStore(ProvisioningRecord record)
    : record_(std::move(record)) {}

ProvisioningRecord StaticProvisioningStore::current() const {
  return record_;
}

CapturingTelemetryTransport::CapturingTelemetryTransport(const bool online)
    : online_(online) {}

bool CapturingTelemetryTransport::publish(const TelemetryFrame& frame) {
  if (!online_) {
    lastError_ = "telemetry transport offline";
    return false;
  }

  if (frame.topic.empty() || frame.payload.empty()) {
    lastError_ = "telemetry frame missing topic or payload";
    return false;
  }

  frames_.push_back(frame);
  lastError_.clear();
  return true;
}

std::string CapturingTelemetryTransport::lastError() const {
  return lastError_;
}

const std::vector<TelemetryFrame>& CapturingTelemetryTransport::frames() const {
  return frames_;
}

McubootUpdateValidator::McubootUpdateValidator(const std::uint32_t currentCounter)
    : currentCounter_(currentCounter) {}

bool McubootUpdateValidator::validate(const OtaImageMetadata& image,
                                      const BoardProfile& board,
                                      std::string& reason) const {
  if (!image.signatureValid) {
    reason = "MCUboot image signature is not valid";
    return false;
  }

  if (!image.confirmed) {
    reason = "running image is not confirmed for rollback safety";
    return false;
  }

  if (!isSha256(image.sha256)) {
    reason = "image sha256 must be a 64-character hex digest";
    return false;
  }

  if (image.monotonicCounter < currentCounter_) {
    reason = "image rollback counter " + std::to_string(image.monotonicCounter) +
             " is older than device counter " + std::to_string(currentCounter_);
    return false;
  }

  const std::size_t maxImageBytes = (board.flashKib * 1024U) / 2U;
  if (image.sizeBytes == 0U || image.sizeBytes > maxImageBytes) {
    reason = "image size " + std::to_string(image.sizeBytes) +
             " exceeds MCUboot slot budget " + std::to_string(maxImageBytes);
    return false;
  }

  reason = "confirmed " + image.version + " in " + image.slot +
           " with rollback counter " + std::to_string(image.monotonicCounter);
  return true;
}

PowerEstimate Nrf52840PowerProfiler::estimate(const NodeConfig& config,
                                              const BoardProfile&,
                                              const SensorSample& sample) const {
  const std::uint32_t activeMs =
      sample.accelerationMg > 120 ? 18U : 12U;
  const std::uint32_t radioMs = sample.accelerationMg > 120 ? 34U : 24U;
  const std::uint32_t busyMs = activeMs + radioMs;
  const std::uint32_t sleepMs =
      config.samplePeriodMs > busyMs ? config.samplePeriodMs - busyMs : 0U;

  constexpr double kActiveCurrentUa = 5200.0;
  constexpr double kRadioCurrentUa = 7600.0;
  constexpr double kSleepCurrentUa = 4.0;
  const double periodMs = static_cast<double>(std::max(config.samplePeriodMs, 1U));
  const double averageCurrentUa =
      ((static_cast<double>(activeMs) * kActiveCurrentUa) +
       (static_cast<double>(radioMs) * kRadioCurrentUa) +
       (static_cast<double>(sleepMs) * kSleepCurrentUa)) /
      periodMs;
  const double batteryUah = static_cast<double>(config.batteryCapacityMah) * 1000.0;
  const double projectedBatteryDays = batteryUah / averageCurrentUa / 24.0;

  return PowerEstimate{activeMs,
                       radioMs,
                       sleepMs,
                       averageCurrentUa,
                       projectedBatteryDays};
}

TextCycleReporter::TextCycleReporter(std::ostream& stream) : stream_(stream) {}

void TextCycleReporter::publish(const CycleResult& result) const {
  stream_ << "cycle=" << (result.accepted ? "PASS" : "FAIL")
          << " reason=\"" << result.failureReason << "\"\n";

  for (const auto& event : result.events) {
    stream_ << "  [" << toString(event.status) << "] " << event.step << ": "
            << event.detail << '\n';
  }

  if (!result.telemetry.topic.empty()) {
    stream_ << "  telemetry topic=" << result.telemetry.topic
            << " qos=" << static_cast<unsigned int>(result.telemetry.qos)
            << '\n';
  }

  if (result.power.activeMs != 0U || result.power.radioMs != 0U) {
    stream_ << "  power active_ms=" << result.power.activeMs
            << " radio_ms=" << result.power.radioMs
            << " sleep_ms=" << result.power.sleepMs
            << " avg_ua=" << fixed2(result.power.averageCurrentUa)
            << " projected_days=" << fixed1(result.power.projectedBatteryDays)
            << '\n';
  }
}

ProvisioningRecord demoProvisioning() {
  return ProvisioningRecord{
      true,
      true,
      "mqtts://iot.example.com:8883",
      "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
      "factory-wifi"};
}

OtaImageMetadata demoImage() {
  return OtaImageMetadata{
      "2026.06.10-nrf52840",
      "slot0",
      "3b7e1f0c9d4a5b682817263544e5f60718c9aabbccddeeff0011223344556677",
      372U * 1024U,
      true,
      true,
      42U};
}

SensorSample demoSample() {
  return SensorSample{120000U, 24.6, 44.5, 101.32, 3670U, 36};
}

}  // namespace sensor_node
