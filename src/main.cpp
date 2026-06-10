#include "sensor_node/SensorNode.hpp"

#include <iostream>
#include <vector>

int main() {
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
  sensor_node::ScriptedSensorReader sensors({sensor_node::demoSample()});
  sensor_node::StaticProvisioningStore provisioning(
      sensor_node::demoProvisioning());
  sensor_node::CapturingTelemetryTransport transport;
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
