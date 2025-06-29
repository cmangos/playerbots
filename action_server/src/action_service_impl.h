#ifndef ACTION_SERVICE_IMPL_H
#define ACTION_SERVICE_IMPL_H

#include "playerbot_rpc.grpc.pb.h" // Generated from playerbot_rpc.proto
#include <grpcpp/grpcpp.h>

class ActionServiceImpl final : public playerbot_rpc::ActionService::Service {
public:
    grpc::Status GetMacroDecision(
        grpc::ServerContext* context,
        const playerbot_rpc::MacroDecisionRequest* request,
        playerbot_rpc::MacroDecisionResponse* response) override;
};

#endif // ACTION_SERVICE_IMPL_H
