#include "PlayerbotRpcClientMgr.h"
#include "playerbot/PlayerbotAIConfig.h" // For sLog and config access
#include "Logging/Log.h"                 // For sLog access (if not in PlayerbotAIConfig.h)

PlayerbotRpcClientMgr::PlayerbotRpcClientMgr() : initialized_(false) {
    // Constructor can be light, initialization happens in Initialize()
}

PlayerbotRpcClientMgr::~PlayerbotRpcClientMgr() {
    // Cleanup if necessary, though shared_ptr and unique_ptr handle memory.
    // If channel needed explicit shutdown, it would go here.
}

PlayerbotRpcClientMgr& PlayerbotRpcClientMgr::instance() {
    static PlayerbotRpcClientMgr instance;
    return instance;
}

void PlayerbotRpcClientMgr::Initialize(const std::string& server_address) {
    std::lock_guard<std::mutex> lock(init_mutex_);
    if (initialized_) {
        // Already initialized, perhaps log or decide if re-initialization is allowed/needed
        if (server_address_ != server_address) {
            sLog.outString("PlayerbotRpcClientMgr: Attempting to re-initialize with a new address. Current: %s, New: %s",
                           server_address_.c_str(), server_address.c_str());
            // Potentially tear down old channel/stub and recreate
            // For now, let's just log and prevent re-init with different address easily
             stub_.reset();
             channel_.reset();
             initialized_ = false;
        } else {
            sLog.outString("PlayerbotRpcClientMgr: Already initialized with address %s.", server_address.c_str());
            return;
        }
    }

    if (server_address.empty()) {
        sLog.outString("PlayerbotRpcClientMgr: Server address is empty. RPC client will not be initialized.");
        server_address_ = "";
        initialized_ = false; // Mark as not successfully initialized
        stub_.reset();
        channel_.reset();
        return;
    }

    server_address_ = server_address;
    sLog.outString("PlayerbotRpcClientMgr: Initializing RPC client for Action Server at %s", server_address_.c_str());

    try {
        channel_ = grpc::CreateChannel(server_address_, grpc::InsecureChannelCredentials());
        if (!channel_) {
            sLog.outError("PlayerbotRpcClientMgr: Failed to create gRPC channel to %s.", server_address_.c_str());
            initialized_ = false;
            return;
        }

        // You might want to check channel connectivity here, e.g., channel_->GetState(true)
        // or wait for ready with a timeout, though stub creation itself doesn't guarantee connection.
        // grpc::ClientContext context;
        // std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
        // channel_->WaitForConnected(deadline);


        stub_ = playerbot_rpc::ActionService::NewStub(channel_);
        if (!stub_) {
            sLog.outError("PlayerbotRpcClientMgr: Failed to create gRPC stub for ActionService.");
            channel_.reset();
            initialized_ = false;
            return;
        }
        initialized_ = true;
        sLog.outString("PlayerbotRpcClientMgr: Successfully initialized and created stub for Action Server at %s", server_address_.c_str());
    } catch (const std::exception& e) {
        sLog.outError("PlayerbotRpcClientMgr: Exception during initialization for %s: %s", server_address_.c_str(), e.what());
        stub_.reset();
        channel_.reset();
        initialized_ = false;
    }
}

bool PlayerbotRpcClientMgr::IsInitialized() const {
    return initialized_ && stub_ != nullptr;
}

playerbot_rpc::ActionService::StubInterface* PlayerbotRpcClientMgr::GetClientStub() {
    if (!IsInitialized()) {
        // Attempt to re-initialize if not already initialized and address is known
        if (!server_address_.empty()) {
            sLog.outString("PlayerbotRpcClientMgr::GetClientStub: Stub not ready, attempting to re-initialize to %s", server_address_.c_str());
            Initialize(server_address_); // This will re-check initialized_ flag internally
            if(!IsInitialized()) {
                 sLog.outError("PlayerbotRpcClientMgr::GetClientStub: Re-initialization failed.");
                 return nullptr;
            }
        } else {
            sLog.outError("PlayerbotRpcClientMgr::GetClientStub: Manager not initialized and no server address known.");
            return nullptr;
        }
    }
    return stub_.get();
}
