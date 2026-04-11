cmake_minimum_required(VERSION 3.21)

project(LastChaos LANGUAGES C CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Vulkan SDK
# On macOS, this will typically point to MoltenVK if installed correctly.
# CMake might need additional hints or modules to find Vulkan/MoltenVK.
# We can add find_package(Vulkan REQUIRED) later.

# Add the porting library
add_subdirectory(porting)

option(LASTCHAOS_ENABLE_NKSP_PROBE "Build experimental nksp_probe target for mac/Linux portability work" ON)
if(LASTCHAOS_ENABLE_NKSP_PROBE)
  add_subdirectory(Nksp_probe)
endif()

# Add the Vulkan components (This part needs refinement based on actual Vulkan build system)
# We need to figure out how to build dxvk, vkcommon, and related Vulkan code.
# For now, let's assume we can find CMakeLists.txt for them or will add them later.

# Example: If dxvk had a CMakeLists.txt
# add_subdirectory(source/dxvk)

# For now, let's focus on building the core executables that use these libraries.
# Based on build_macos_app.sh, lastchaos_login_check is built.
# We need to define targets for the main game executable as well.

# If we can't find CMakeLists.txt for dxvk/vkcommon, we might need to compile them manually
# or vendor them with CMakeLists.txt.

# Add a placeholder for the main game executable target.
# This will need to be defined based on the actual game entry point.
# For example:
# add_executable(LastChaosSources Sources/main.cpp Engine/Graphics/Graphics.cpp ...)
# target_link_libraries(LastChaosSources PRIVATE lastchaos_porting Vulkan::Vulkan)

# Link the porting library to executables that need it.
# For example, if LastChaos executable needs it:
# target_link_libraries(LastChaos PRIVATE lastchaos_porting)

# --- macOS Specific Configuration ---
if (APPLE)
    # Add macOS specific frameworks or libraries
    # For Vulkan on macOS, MoltenVK is typically used.
    # The Vulkan SDK installation usually provides MoltenVK.
    # We'll need to find the MoltenVK library and headers.

    # Example: Find Vulkan SDK and MoltenVK
    # find_package(Vulkan REQUIRED) # This might automatically find MoltenVK on macOS
    # target_link_libraries(YOUR_MAIN_TARGET PRIVATE Vulkan::Vulkan)

    # We might need to set specific compiler flags or definitions for macOS
    # target_compile_definitions(YOUR_MAIN_TARGET PRIVATE _DARWIN_C_SOURCE) # Already handled in porting/CMakeLists.txt

    # For macOS app bundle creation, this could be integrated into CMake targets
    # or handled by build scripts.

    # Example: Add a target to create the .app bundle
    # add_custom_target(bundle_macos_app ALL
    #     COMMAND "${ROOT_DIR}/scripts/build_macos_app.sh"
    #     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    #     DEPENDS ${YOUR_MAIN_TARGET}
    # )
endif()
