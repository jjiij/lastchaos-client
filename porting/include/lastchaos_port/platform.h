#ifndef LASTCHAOS_PORT_PLATFORM_H
#define LASTCHAOS_PORT_PLATFORM_H

#include <cstdint>
#include <initializer_list>
#include <string>

namespace lastchaos::port {

enum class OperatingSystem {
  Windows,
  Linux,
  macOS,
  Unknown
};

struct EnvironmentInfo {
  OperatingSystem os = OperatingSystem::Unknown;
  int pointer_size = 0;
  std::string executable_directory;
  std::string user_data_directory;
};

OperatingSystem detect_operating_system();
const char* operating_system_name(OperatingSystem os);

std::string join_paths(std::initializer_list<std::string> parts);
std::string executable_directory();
std::string user_data_directory(const std::string& app_name);
std::uint64_t monotonic_milliseconds();

bool ensure_directory(const std::string& path);
EnvironmentInfo capture_environment(const std::string& app_name);

} // namespace lastchaos::port

#endif
