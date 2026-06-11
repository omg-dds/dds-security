cmake_minimum_required(VERSION 3.12)
project(opendds_security_interop CXX)

find_package(OpenDDS REQUIRED)

add_executable(opendds_security_interop
  ../ShapeType_main.cxx
)

target_compile_definitions(opendds_security_interop
  PRIVATE OPENDDS
)

opendds_target_sources(opendds_security_interop
  ../ShapeType.idl
  OPENDDS_IDL_OPTIONS --no-default-nested
)

target_link_libraries(opendds_security_interop OpenDDS::Security OpenDDS::Rtps_Udp)
