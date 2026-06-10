#ifndef SENSOR_NODE_SENSOR_NODE_HPP_
#define SENSOR_NODE_SENSOR_NODE_HPP_

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <vector>

namespace sensor_node {

enum class CycleStatus {
  Passed,
  Failed
};

std::string toString(CycleStatus status);

struct BoardProfile {
  std::string boardTarget;
  std::string boardName;
  std::string soc;
  std::size_t ramKib{};
  std::size_t flashKib{};
  bool hasBle{};
  bool hasThreadRadio{};
  bool hasNativeWifi{};
  bool hasSaadc{};
};

BoardProfile nrf52840DevelopmentKit();

struct NodeConfig {
  std::string deviceId;
  std::string firmwareVersion;
  std::uint32_t samplePeriodMs{60000};
  std::uint16_t minBatteryMv{3200};
  double maxTemperatureC{70.0};
  double maxAverageCurrentUa{35.0};
  std::uint16_t batteryCapacityMah{220};
};

struct ProvisioningRecord {
  bool bleBonded{};
  bool externalWifiBackhaul{};
  std::string endpointUri;
  std::string certificateFingerprint;
  std::string wifiSsid;
};

struct OtaImageMetadata {
  std::string version;
  std::string slot;
  std::string sha256;
  std::size_t sizeBytes{};
  bool signatureValid{};
  bool confirmed{};
  std::uint32_t monotonicCounter{};
};

struct SensorSample {
  std::uint64_t uptimeMs{};
  double temperatureC{};
  double humidityPct{};
  double pressureKpa{};
  std::uint16_t batteryMv{};
  std::int16_t accelerationMg{};
};

struct TelemetryFrame {
  std::string topic;
  std::string payload;
  std::uint8_t qos{};
};

struct PowerEstimate {
  std::uint32_t activeMs{};
  std::uint32_t radioMs{};
  std::uint32_t sleepMs{};
  double averageCurrentUa{};
  double projectedBatteryDays{};
};

struct CycleEvent {
  std::string step;
  CycleStatus status{CycleStatus::Failed};
  std::string detail;
};

struct CycleResult {
  bool accepted{};
  std::string failureReason;
  SensorSample sample;
  TelemetryFrame telemetry;
  PowerEstimate power;
  std::vector<CycleEvent> events;
};

class ISensorReader {
 public:
  virtual ~ISensorReader() = default;
  virtual SensorSample read() = 0;
};

class IProvisioningStore {
 public:
  virtual ~IProvisioningStore() = default;
  virtual ProvisioningRecord current() const = 0;
};

class ITelemetryTransport {
 public:
  virtual ~ITelemetryTransport() = default;
  virtual bool publish(const TelemetryFrame& frame) = 0;
  virtual std::string lastError() const = 0;
};

class IUpdateValidator {
 public:
  virtual ~IUpdateValidator() = default;
  virtual bool validate(const OtaImageMetadata& image,
                        const BoardProfile& board,
                        std::string& reason) const = 0;
};

class IPowerProfiler {
 public:
  virtual ~IPowerProfiler() = default;
  virtual PowerEstimate estimate(const NodeConfig& config,
                                 const BoardProfile& board,
                                 const SensorSample& sample) const = 0;
};

class ZephyrSensorNode {
 public:
  ZephyrSensorNode(BoardProfile board,
                   NodeConfig config,
                   OtaImageMetadata runningImage,
                   ISensorReader& sensorReader,
                   IProvisioningStore& provisioningStore,
                   ITelemetryTransport& telemetryTransport,
                   IUpdateValidator& updateValidator,
                   IPowerProfiler& powerProfiler);

  CycleResult runCycle();

 private:
  BoardProfile board_;
  NodeConfig config_;
  OtaImageMetadata runningImage_;
  ISensorReader& sensorReader_;
  IProvisioningStore& provisioningStore_;
  ITelemetryTransport& telemetryTransport_;
  IUpdateValidator& updateValidator_;
  IPowerProfiler& powerProfiler_;
};

class ScriptedSensorReader final : public ISensorReader {
 public:
  explicit ScriptedSensorReader(std::vector<SensorSample> samples);

  SensorSample read() override;

 private:
  std::vector<SensorSample> samples_;
  std::size_t nextIndex_{};
};

class StaticProvisioningStore final : public IProvisioningStore {
 public:
  explicit StaticProvisioningStore(ProvisioningRecord record);

  ProvisioningRecord current() const override;

 private:
  ProvisioningRecord record_;
};

class CapturingTelemetryTransport final : public ITelemetryTransport {
 public:
  explicit CapturingTelemetryTransport(bool online = true);

  bool publish(const TelemetryFrame& frame) override;
  std::string lastError() const override;
  const std::vector<TelemetryFrame>& frames() const;

 private:
  bool online_{};
  std::string lastError_;
  std::vector<TelemetryFrame> frames_;
};

class McubootUpdateValidator final : public IUpdateValidator {
 public:
  explicit McubootUpdateValidator(std::uint32_t currentCounter);

  bool validate(const OtaImageMetadata& image,
                const BoardProfile& board,
                std::string& reason) const override;

 private:
  std::uint32_t currentCounter_{};
};

class Nrf52840PowerProfiler final : public IPowerProfiler {
 public:
  PowerEstimate estimate(const NodeConfig& config,
                         const BoardProfile& board,
                         const SensorSample& sample) const override;
};

class TextCycleReporter {
 public:
  explicit TextCycleReporter(std::ostream& stream);

  void publish(const CycleResult& result) const;

 private:
  std::ostream& stream_;
};

ProvisioningRecord demoProvisioning();
OtaImageMetadata demoImage();
SensorSample demoSample();

}  // namespace sensor_node

#endif  // SENSOR_NODE_SENSOR_NODE_HPP_
