#include "lastchaos_port/platform.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <optional>
#include <string>

#if defined(_WIN32)
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#  include <netdb.h>
#  include <sys/socket.h>
#  include <unistd.h>
#endif

namespace {

struct LoginEntry {
  std::string name;
  std::string address;
  std::string port;
  std::string full_users;
  std::string busy_users;
};

std::optional<LoginEntry> decode_login_entry(const std::filesystem::path& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return std::nullopt;
  }

  std::array<char, 6> header{};
  file.read(header.data(), static_cast<std::streamsize>(header.size()));
  if (!file) {
    return std::nullopt;
  }

  std::int32_t i_key = 0;
  file.read(reinterpret_cast<char*>(&i_key), sizeof(i_key));
  if (!file) {
    return std::nullopt;
  }

  unsigned char key = 0;
  file.read(reinterpret_cast<char*>(&key), 1);
  if (!file) {
    return std::nullopt;
  }

  std::int32_t encoded_server_count = 0;
  file.read(reinterpret_cast<char*>(&encoded_server_count), sizeof(encoded_server_count));
  if (!file) {
    return std::nullopt;
  }

  const std::int32_t server_count = encoded_server_count - i_key;
  i_key = encoded_server_count;
  if (server_count <= 0 || server_count > 128) {
    return std::nullopt;
  }

  std::int32_t encoded_len = 0;
  file.read(reinterpret_cast<char*>(&encoded_len), sizeof(encoded_len));
  if (!file) {
    return std::nullopt;
  }

  const std::int32_t line_len = encoded_len - i_key;
  if (line_len <= 0 || line_len > 4096) {
    return std::nullopt;
  }

  std::string plain(static_cast<std::size_t>(line_len), '\0');
  for (std::int32_t i = 0; i < line_len; ++i) {
    unsigned char data = 0;
    file.read(reinterpret_cast<char*>(&data), 1);
    if (!file) {
      return std::nullopt;
    }
    plain[static_cast<std::size_t>(i)] = static_cast<char>(data - key);
    key = data;
  }

  LoginEntry entry;
  std::string name, address, port, full_users, busy_users;
  std::istringstream stream(plain);
  if (!(stream >> name >> address >> port >> full_users >> busy_users)) {
    return std::nullopt;
  }

  entry.name = std::move(name);
  entry.address = std::move(address);
  entry.port = std::move(port);
  entry.full_users = std::move(full_users);
  entry.busy_users = std::move(busy_users);
  return entry;
}

bool connect_to_endpoint(const std::string& host, const std::string& port) {
  addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  addrinfo* result = nullptr;
  const int rc = ::getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
  if (rc != 0 || result == nullptr) {
    std::cerr << "getaddrinfo failed for " << host << ":" << port << "\n";
    return false;
  }

  bool connected = false;
  for (addrinfo* ai = result; ai != nullptr; ai = ai->ai_next) {
#if defined(_WIN32)
    SOCKET sock = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sock == INVALID_SOCKET) {
      continue;
    }
    if (::connect(sock, ai->ai_addr, static_cast<int>(ai->ai_addrlen)) == 0) {
      connected = true;
      ::closesocket(sock);
      break;
    }
    ::closesocket(sock);
#else
    int sock = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sock < 0) {
      continue;
    }
    if (::connect(sock, ai->ai_addr, ai->ai_addrlen) == 0) {
      connected = true;
      ::close(sock);
      break;
    }
    ::close(sock);
#endif
  }

  ::freeaddrinfo(result);
  return connected;
}

}  // namespace

int main() {
  const auto env = lastchaos::port::capture_environment("LastChaos");
  const std::filesystem::path config_path =
      std::filesystem::path(env.executable_directory) / "sl.dta";

  const auto entry = decode_login_entry(config_path);
  if (!entry) {
    std::cerr << "Failed to read " << config_path << "\n";
    return 1;
  }

  std::cout << "LoginEntry=" << entry->name << " " << entry->address << ":" << entry->port << "\n";

  if (!connect_to_endpoint(entry->address, entry->port)) {
    std::cerr << "Connection failed to " << entry->address << ":" << entry->port << "\n";
    return 2;
  }

  std::cout << "Connection OK\n";
  return 0;
}
