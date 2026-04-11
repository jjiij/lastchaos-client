#include "lastchaos_port/platform.h"

#include <iostream>

int main() {
  const auto info = lastchaos::port::capture_environment("LastChaos");
  std::cout << "OS=" << lastchaos::port::operating_system_name(info.os) << "\n";
  std::cout << "PointerSize=" << info.pointer_size << "\n";
  std::cout << "ExeDir=" << info.executable_directory << "\n";
  std::cout << "UserDataDir=" << info.user_data_directory << "\n";
  return 0;
}
