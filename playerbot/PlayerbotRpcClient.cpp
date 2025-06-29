#include "PlayerbotRpcClient.h"
#include "PlayerbotAI.h"       // For PlayerbotAI context, logging
#include "PlayerbotAIConfig.h" // For potential config values like server address

// Constructor
PlayerbotRpcClient::PlayerbotRpcClient(ai::PlayerbotAI* ai, playerbot_rpc::ActionService::StubInterface* stub)
    : botAI(ai), stub_(stub) {
    // The stub is now passed in, presumably from PlayerbotRpcClientMgr
    if (botAI && botAI->GetMaster() && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
        std::ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << botAI->GetBot()->GetName() << ",INFO,PlayerbotRpcClient,PlayerbotRpcClient wrapper created, using shared stub.";
        sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
    }
}

// Method to request a macro decision from the action server
bool PlayerbotRpcClient::GetMacroDecision(
    const playerbot_rpc::BotStateSnapshot& snapshot,
    playerbot_rpc::MacroDecisionResponse* rpc_response) {

    if (!stub_) { // Check if the provided stub is valid
        if (botAI && botAI->GetMaster() && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
            std::ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
            out << botAI->GetBot()->GetName() << ",ERROR,PlayerbotRpcClient,GetMacroDecision: RPC stub is NULL.";
            sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
        }
        return false;
    }

    playerbot_rpc::MacroDecisionRequest request;
    request.mutable_bot_state()->CopyFrom(snapshot);

    grpc::ClientContext context;
    // Set a deadline for the RPC call (e.g., 500 milliseconds)
    std::chrono::system_clock::time_point deadline =
        std::chrono::system_clock::now() + std::chrono::milliseconds(sPlayerbotAIConfig.actionServerRpcTimeoutMs); // Configurable timeout
    context.set_deadline(deadline);

    grpc::Status status = stub_->GetMacroDecision(&context, request, rpc_response);

    if (status.ok()) {
        if (botAI && botAI->GetMaster() && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
            std::ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
            out << botAI->GetBot()->GetName() << ",INFO,PlayerbotRpcClient,GetMacroDecision: Success. Decision: " << rpc_response->decision_type();
            if (!rpc_response->justification().empty()) {
                out << " Justification: " << rpc_response->justification();
            }
            sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
        }
        return true;
    } else {
        if (botAI && botAI->GetMaster()) { // Check botAI and master to prevent crashes if they are null
             if (sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
                std::ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                out << botAI->GetBot()->GetName() << ",ERROR,PlayerbotRpcClient,GetMacroDecision: RPC failed. Error code: " << status.error_code()
                    << ", message: " << status.error_message();
                sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
            }
            // Optionally, tell the player master about the error if debugging is enabled
            if (sPlayerbotAIConfig.enableDebugWhispers) {
                 std::ostringstream debugMsg;
                 debugMsg << "RPC Error: " << status.error_message() << " (Code: " << status.error_code() << ")";
                 botAI->TellPlayer(botAI->GetMaster(), debugMsg.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, true);
            }
        }
        return false;
    }
}
