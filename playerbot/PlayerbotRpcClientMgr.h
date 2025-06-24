#ifndef PLAYERBOTRPCCLIENTMGR_H
#define PLAYERBOTRPCCLIENTMGR_H

#include "rpc/playerbot_rpc.grpc.pb.h" // Generated gRPC file
#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <mutex>

// Singleton class to manage the gRPC channel and stub for the ActionService
class PlayerbotRpcClientMgr {
public:
    static PlayerbotRpcClientMgr& instance();

    // Call this during server startup or when config is loaded
    void Initialize(const std::string& server_address);
    bool IsInitialized() const;

    // Provides access to the client stub
    // Returns nullptr if not initialized or connection failed
    playerbot_rpc::ActionService::StubInterface* GetClientStub();

private:
    PlayerbotRpcClientMgr(); // Private constructor for singleton
    ~PlayerbotRpcClientMgr();

    // Delete copy constructor and assignment operator
    PlayerbotRpcClientMgr(const PlayerbotRpcClientMgr&) = delete;
    PlayerbotRpcClientMgr& operator=(const PlayerbotRpcClientMgr&) = delete;

    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<playerbot_rpc::ActionService::StubInterface> stub_;

    bool initialized_ = false;
    std::string server_address_;
    std::mutex init_mutex_;
};

#define sPlayerbotRpcClientMgr PlayerbotRpcClientMgr::instance()

#endif // PLAYERBOTRPCCLIENTMGR_H
