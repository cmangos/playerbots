cmake_minimum_required(VERSION 3.10)
project(ActionServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Protobuf
find_package(Protobuf REQUIRED)

# Find gRPC
find_package(gRPC REQUIRED)

# Add action_server executable
add_executable(action_server src/main.cpp src/action_service_impl.cpp)

# Include directories for Protobuf and gRPC generated files
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Link libraries
target_link_libraries(action_server
    PRIVATE
    ${Protobuf_LIBRARIES}
    gRPC::grpc++
    gRPC::grpc++_reflection # Optional, but useful for debugging
)

# Custom command to generate protobuf and gRPC C++ files
# This assumes playerbot_rpc.proto is in the same directory as this CMakeLists.txt
set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/playerbot_rpc.proto)
set(PROTO_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(
    OUTPUT
        ${PROTO_GENERATED_DIR}/playerbot_rpc.pb.h
        ${PROTO_GENERATED_DIR}/playerbot_rpc.pb.cc
        ${PROTO_GENERATED_DIR}/playerbot_rpc.grpc.pb.h
        ${PROTO_GENERATED_DIR}/playerbot_rpc.grpc.pb.cc
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROTO_GENERATED_DIR}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
        --cpp_out=${PROTO_GENERATED_DIR}
        --grpc_out=${PROTO_GENERATED_DIR}
        --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating Protobuf and gRPC C++ files from ${PROTO_FILE}"
)

# Add generated files to the action_server target sources
target_sources(action_server
    PRIVATE
        ${PROTO_GENERATED_DIR}/playerbot_rpc.pb.h
        ${PROTO_GENERATED_DIR}/playerbot_rpc.pb.cc
        ${PROTO_GENERATED_DIR}/playerbot_rpc.grpc.pb.h
        ${PROTO_GENERATED_DIR}/playerbot_rpc.grpc.pb.cc
)

# Ensure generated files are created before compiling action_server
add_custom_target(generate_proto_files ALL DEPENDS
    ${PROTO_GENERATED_DIR}/playerbot_rpc.pb.h
    ${PROTO_GENERATED_DIR}/playerbot_rpc.pb.cc
    ${PROTO_GENERATED_DIR}/playerbot_rpc.grpc.pb.h
    ${PROTO_GENERATED_DIR}/playerbot_rpc.grpc.pb.cc
)

add_dependencies(action_server generate_proto_files)

install(TARGETS action_server DESTINATION bin)
