cmake_minimum_required(VERSION 3.5)
set(CMAKE_TOOLCHAIN_FILE /home/burak/vcpkg/scripts/buildsystems/vcpkg.cmake )
project(listservices LANGUAGES CXX)
#include(common.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

set(protobuf_MODULE_COMPATIBLE TRUE)
#find_package(Protobuf CONFIG REQUIRED)
#message(STATUS "Using protobuf ${protobuf_VERSION}")

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

file(GLOB_RECURSE ProtoFiles "${CMAKE_CURRENT_LIST_DIR}/proto/*.proto")
message(STATUS ${ProtoFiles})
add_library(proto-objects OBJECT ${ProtoFiles})

target_link_libraries(proto-objects PUBLIC protobuf::libprotobuf gRPC::grpc++_unsecure gRPC::grpcpp_channelz)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_BINARY_DIR})
set(PROTO_IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/proto")

protobuf_generate(
    TARGET proto-objects
    OUT_VAR PROTO_GENERATED_FILES
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")
set_source_files_properties(${PROTO_GENERATED_FILES} PROPERTIES SKIP_UNITY_BUILD_INCLUSION on)

protobuf_generate(
    TARGET proto-objects
    OUT_VAR PROTO_GENERATED_FILES
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
    # PLUGIN_OPTIONS "generate_mock_code=true"
    IMPORT_DIRS ${PROTO_IMPORT_DIRS}
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")
set_source_files_properties(${PROTO_GENERATED_FILES} PROPERTIES SKIP_UNITY_BUILD_INCLUSION on)

target_include_directories(proto-objects PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")
file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB_RECURSE HDR "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB_RECURSE UIS "${CMAKE_CURRENT_LIST_DIR}/*.ui")
qt5_wrap_ui(UIS_HDRS ${UIS})

add_executable(listservices ${SRCS} ${HDR} ${UIS} ${UIS_HDRS})
target_link_libraries(listservices proto-objects Qt5::Widgets)
