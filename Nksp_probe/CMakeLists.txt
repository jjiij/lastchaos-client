set(NKSP_SRC_DIR "${CMAKE_SOURCE_DIR}/source")

add_executable(nksp_probe
  "${NKSP_SRC_DIR}/Nksp/Nksp.cpp"
  "${NKSP_SRC_DIR}/Engine/Engine.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/stubs.cpp"
)

target_include_directories(nksp_probe PRIVATE
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

target_compile_definitions(nksp_probe PRIVATE
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

target_compile_options(nksp_probe PRIVATE
  -Wno-write-strings
  -Wno-deprecated-declarations
  -Wno-invalid-offsetof
  -Winvalid-source-encoding
)

target_link_libraries(nksp_probe PRIVATE
  lastchaos_porting
)

# Nksp.cpp references the full Serious Engine; we do not link the whole Engine yet.
# Allow the link step to succeed so CI can verify compilation; symbols resolve when
# a full Engine dylib/static archive is wired in later.
if(APPLE)
  target_link_options(nksp_probe PRIVATE LINKER:-undefined,dynamic_lookup)
  target_link_options(nksp_probe PRIVATE LINKER:-multiply_defined,suppress)
endif()
