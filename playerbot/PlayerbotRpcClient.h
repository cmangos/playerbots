#ifndef PLAYERBOTRPCCLIENT_H
#define PLAYERBOTRPCCLIENT_H

#include "rpc/playerbot_rpc.grpc.pb.h" // Generated gRPC file
#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

// Forward declaration
namespace ai { class PlayerbotAI; }

class PlayerbotRpcClient {
public:
    // Constructor now takes the existing stub from the manager
    PlayerbotRpcClient(ai::PlayerbotAI* ai, playerbot_rpc::ActionService::StubInterface* stub);

    // Method to request a macro decision from the action server
    bool GetMacroDecision(const playerbot_rpc::BotStateSnapshot& snapshot, playerbot_rpc::MacroDecisionResponse* response);

private:
    ai::PlayerbotAI* botAI; // Reference to the bot's AI instance for context
    playerbot_rpc::ActionService::StubInterface* stub_; // Now a raw pointer to the shared stub
};

#endif // PLAYERBOTRPCCLIENT_H
