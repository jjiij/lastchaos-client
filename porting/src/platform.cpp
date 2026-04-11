#include "lastchaos_port/platform.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <system_error>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#include <libgen.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#endif

namespace fs = std::filesystem;

namespace lastchaos::port {

OperatingSystem detect_operating_system() {
#if defined(_WIN32)
  return OperatingSystem::Windows;
#elif defined(__APPLE__)
  return OperatingSystem::macOS;
#elif defined(__linux__)
  return OperatingSystem::Linux;
#else
  return OperatingSystem::Unknown;
#endif
}

const char* operating_system_name(OperatingSystem os) {
  switch (os) {
    case OperatingSystem::Windows: return "Windows";
    case OperatingSystem::Linux: return "Linux";
    case OperatingSystem::macOS: return "macOS";
    default: return "Unknown";
  }
}

std::string join_paths(std::initializer_list<std::string> parts) {
  fs::path result;
  for (const auto& part : parts) {
    if (part.empty()) {
      continue;
    }
    result /= fs::path(part);
  }
  return result.lexically_normal().string();
}

std::string executable_directory() {
#if defined(_WIN32)
  char buffer[MAX_PATH] = {};
  const DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
  if (len == 0) {
    return {};
  }
  return fs::path(buffer).parent_path().string();
#elif defined(__APPLE__)
  char buffer[PATH_MAX] = {};
  uint32_t size = sizeof(buffer);
  if (_NSGetExecutablePath(buffer, &size) == 0) {
    return fs::path(buffer).parent_path().string();
  }
  return fs::current_path().string();
#elif defined(__linux__)
  char buffer[PATH_MAX] = {};
  const ssize_t len = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (len > 0) {
    buffer[len] = '\0';
    return fs::path(buffer).parent_path().string();
  }
  return fs::current_path().string();
#else
  return fs::current_path().string();
#endif
}

std::string user_data_directory(const std::string& app_name) {
#if defined(_WIN32)
  const char* roaming = std::getenv("APPDATA");
  if (roaming != nullptr && *roaming != '\0') {
    return join_paths({roaming, app_name});
  }
  return join_paths({executable_directory(), app_name});
#elif defined(__APPLE__)
  const char* home = std::getenv("HOME");
  if (home != nullptr && *home != '\0') {
    return join_paths({home, "Library", "Application Support", app_name});
  }
  return join_paths({executable_directory(), app_name});
#else
  const char* xdg = std::getenv("XDG_DATA_HOME");
  if (xdg != nullptr && *xdg != '\0') {
    return join_paths({xdg, app_name});
  }
  const char* home = std::getenv("HOME");
  if (home != nullptr && *home != '\0') {
    return join_paths({home, ".local", "share", app_name});
  }
  return join_paths({executable_directory(), app_name});
#endif
}

std::uint64_t monotonic_milliseconds() {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  return static_cast<std::uint64_t>(
    std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

bool ensure_directory(const std::string& path) {
  if (path.empty()) {
    return false;
  }

  std::error_code ec;
  fs::create_directories(fs::path(path), ec);
  return !ec;
}

EnvironmentInfo capture_environment(const std::string& app_name) {
  EnvironmentInfo info;
  info.os = detect_operating_system();
  info.pointer_size = static_cast<int>(sizeof(void*));
  info.executable_directory = executable_directory();
  info.user_data_directory = user_data_directory(app_name);
  return info;
}

} // namespace lastchaos::port
