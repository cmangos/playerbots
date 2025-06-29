#include <iostream>
#include <string>
#include <grpcpp/grpcpp.h>
#include "action_service_impl.h" // Our service implementation

void RunServer() {
    std::string server_address("0.0.0.0:50051"); // Listen on all interfaces, port 50051
    ActionServiceImpl service;

    grpc::ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);

    // Finally assemble the server.
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Action Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

int main(int argc, char** argv) {
    // Initialize any global state or configurations here if needed
    // For example, connecting to Redis, loading AI models, etc.

    RunServer();
    return 0;
}
