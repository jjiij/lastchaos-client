cmake_minimum_required(VERSION 3.21)
project(lastchaos_porting CXX)

find_program(GO_EXECUTABLE go)
set(LC_CLIENT_SRC_DIR "${CMAKE_SOURCE_DIR}/source")
option(LASTCHAOS_ENABLE_VULKAN_CONTEXT "Build experimental Engine/VulkanAbstraction/VulkanContext.cpp into lastchaos_porting" OFF)

# Find Vulkan SDK and MoltenVK (for macOS) when available.
find_package(Vulkan QUIET)

add_library(lastchaos_porting
  src/platform.cpp
)

target_include_directories(lastchaos_porting
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(LASTCHAOS_ENABLE_VULKAN_CONTEXT)
  if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "LASTCHAOS_ENABLE_VULKAN_CONTEXT=ON requires Vulkan SDK (Vulkan::Vulkan)")
  endif()
  target_sources(lastchaos_porting PRIVATE
    "${LC_CLIENT_SRC_DIR}/Engine/VulkanAbstraction/VulkanContext.cpp"
  )
  target_include_directories(lastchaos_porting PUBLIC
    "${LC_CLIENT_SRC_DIR}"
    "${LC_CLIENT_SRC_DIR}/Engine/VulkanAbstraction"
  )
  target_link_libraries(lastchaos_porting PUBLIC Vulkan::Vulkan)
endif()

target_compile_features(lastchaos_porting PUBLIC cxx_std_17)

if(WIN32)
  target_compile_definitions(lastchaos_porting PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

add_executable(lastchaos_porting_probe
  src/probe.cpp
)

target_link_libraries(lastchaos_porting_probe PRIVATE lastchaos_porting)
if(Vulkan_FOUND)
  target_link_libraries(lastchaos_porting_probe PRIVATE Vulkan::Vulkan)
endif()
target_compile_features(lastchaos_porting_probe PRIVATE cxx_std_17)

if(APPLE)
  target_compile_definitions(lastchaos_porting PRIVATE _DARWIN_C_SOURCE)

  set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
  set_source_files_properties(src/probe.cpp PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

  add_executable(lastchaos_mac_app MACOSX_BUNDLE src/probe.cpp)
  target_link_libraries(lastchaos_mac_app PRIVATE lastchaos_porting)
  if(Vulkan_FOUND)
    target_link_libraries(lastchaos_mac_app PRIVATE Vulkan::Vulkan)
  endif()
  set_target_properties(lastchaos_mac_app PROPERTIES
    BUNDLE True
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.lastchaos.client"
    MACOSX_BUNDLE_BUNDLE_NAME "LastChaos"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.0"
    MACOSX_BUNDLE_COPYRIGHT "(c) 2026 LastChaos"
  )
endif()

add_executable(lastchaos_login_check
  src/login_check.cpp
)

target_link_libraries(lastchaos_login_check PRIVATE lastchaos_porting)
target_compile_features(lastchaos_login_check PRIVATE cxx_std_17)

if(WIN32)
  target_link_libraries(lastchaos_login_check PRIVATE Ws2_32.lib)
endif()

set(LC_LOGIN_CONFIG_PATH "${CMAKE_CURRENT_BINARY_DIR}/sl.dta")
set(LASTCHAOS_LOGIN_NAME "Local" CACHE STRING "Server display name written into generated sl.dta")
set(LASTCHAOS_LOGIN_HOST "127.0.0.1" CACHE STRING "Login server host/IP written into generated sl.dta")
set(LASTCHAOS_LOGIN_PORT "4001" CACHE STRING "Login server port written into generated sl.dta")
set(LASTCHAOS_LOGIN_VERSION "1000" CACHE STRING "Client version field written into generated sl.dta")
set(LASTCHAOS_LOGIN_USERS "300" CACHE STRING "Initial user count field written into generated sl.dta")

if(GO_EXECUTABLE)
  add_custom_command(
    OUTPUT "${LC_LOGIN_CONFIG_PATH}"
    COMMAND "${GO_EXECUTABLE}" run "${CMAKE_SOURCE_DIR}/tools/sl_dta_tool.go"
            encode -out "${LC_LOGIN_CONFIG_PATH}"
            -entry "${LASTCHAOS_LOGIN_NAME} ${LASTCHAOS_LOGIN_HOST} ${LASTCHAOS_LOGIN_PORT} ${LASTCHAOS_LOGIN_VERSION} ${LASTCHAOS_LOGIN_USERS}"
    DEPENDS "${CMAKE_SOURCE_DIR}/tools/sl_dta_tool.go"
    COMMENT "Generating local sl.dta for Docker login server"
    VERBATIM
  )

  add_custom_target(lastchaos_local_sl_dta ALL DEPENDS "${LC_LOGIN_CONFIG_PATH}")
  add_dependencies(lastchaos_login_check lastchaos_local_sl_dta)
  add_dependencies(lastchaos_porting_probe lastchaos_local_sl_dta)
else()
  message(WARNING "Go toolchain not found; local sl.dta generation target will be unavailable")
endif()
