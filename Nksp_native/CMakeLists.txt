set(NKSP_SRC_DIR "${CMAKE_SOURCE_DIR}/source")
option(LASTCHAOS_NKSP_NATIVE_STRICT_LINK "Disable unresolved-link fallbacks for native nksp blueprint target" ON)

add_executable(lastchaos_nksp_native_blueprint EXCLUDE_FROM_ALL
  "${NKSP_SRC_DIR}/Nksp/Nksp.cpp"
  "${NKSP_SRC_DIR}/Nksp/NkspPlatformAdapters.cpp"
  "${NKSP_SRC_DIR}/Engine/Engine.cpp"
)

target_include_directories(lastchaos_nksp_native_blueprint PRIVATE
  "${NKSP_SRC_DIR}"
  "${NKSP_SRC_DIR}/Nksp"
  "${NKSP_SRC_DIR}/Engine"
  "${NKSP_SRC_DIR}/External/Include/native/directx"
  "${NKSP_SRC_DIR}/External/Include/native/windows"
  "${NKSP_SRC_DIR}/External/Include/vulkan/include"
  "${NKSP_SRC_DIR}/External/Include/spirv/include"
  "${CMAKE_SOURCE_DIR}/porting/include"
  "/opt/homebrew/include"
)

target_compile_definitions(lastchaos_nksp_native_blueprint PRIVATE
  _CLIENT_=1
  PLATFORM_UNIX=1
  PLATFORM_MACOSX=1
  NOT_USE_ASM=1
  ENGINE_INTERNAL=1
  DECL_DLL=
  ENGINE_API=
  G_CHINA=0
  VER_TEST=0
  MULTI_CLIENT=1
  _MT=1
  USE_PORTABLE_C=1
  SE1_VULKAN=1
  LC_SKIP_WIN_GEOM_TYPES=1
)

target_compile_options(lastchaos_nksp_native_blueprint PRIVATE
  -Wno-write-strings
  -Wno-deprecated-declarations
  -Wno-invalid-offsetof
  -Winvalid-source-encoding
)

target_link_libraries(lastchaos_nksp_native_blueprint PRIVATE
  lastchaos_porting
)

if(APPLE)
  if(NOT LASTCHAOS_NKSP_NATIVE_STRICT_LINK)
    target_link_options(lastchaos_nksp_native_blueprint PRIVATE LINKER:-undefined,dynamic_lookup)
    target_link_options(lastchaos_nksp_native_blueprint PRIVATE LINKER:-multiply_defined,suppress)
  endif()
endif()
