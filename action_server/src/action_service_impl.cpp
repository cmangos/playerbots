#include "action_service_impl.h"
#include <iostream> // For logging/debugging

grpc::Status ActionServiceImpl::GetMacroDecision(
    grpc::ServerContext* context,
    const playerbot_rpc::MacroDecisionRequest* request,
    playerbot_rpc::MacroDecisionResponse* response) {

    // Log the request (basic example)
    std::cout << "Received GetMacroDecision request for bot GUID: " << request->bot_state().bot_guid() << std::endl;
    if (request->has_bot_state() && request->bot_state().has_current_position()) {
        std::cout << "  Bot Position: map_id=" << request->bot_state().current_position().map_id()
                  << ", x=" << request->bot_state().current_position().x()
                  << ", y=" << request->bot_state().current_position().y()
                  << ", z=" << request->bot_state().current_position().z() << std::endl;
    }

    // Placeholder logic: Always tell the bot to continue its current action.
    // In a real implementation, this is where the AI decision-making would happen.
    response->set_decision_type(playerbot_rpc::MacroDecisionResponse::CONTINUE_CURRENT);
    response->set_justification("Action server received request, placeholder: continue.");

    // Example of how to set a different decision:
    /*
    response->set_decision_type(playerbot_rpc::MacroDecisionResponse::SET_STRATEGY);
    response->set_strategy_name("grind");
    response->set_justification("Action server suggests grinding.");
    */

    std::cout << "Sending MacroDecisionResponse: CONTINUE_CURRENT" << std::endl;
    return grpc::Status::OK;
}
