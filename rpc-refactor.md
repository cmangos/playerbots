# RPC Refactor Checklist for Playerbot AI

This document outlines the steps to refactor the playerbot AI decision-making process to happen outside the main game server through RPC, introducing an "Action Server."

## I. Core RPC Infrastructure Setup

*   [x] **Define RPC Interface (`playerbot_rpc.proto`)**
    *   [x] Specify services: `ActionService` (GameServer -> ActionServer) and `GameControlService` (ActionServer/External -> GameServer).
    *   [x] Define request/response messages (e.g., `MacroDecisionRequest`, `MacroDecisionResponse`, `BotCommandRequest`, `BotCommandResponse`).
    *   [x] Define common data structures (e.g., `Position`, `BotStats`, `Entity`, `Quest`, `BotStateSnapshot`).
    *   [x] Choose Protocol Buffers as the Interface Definition Language.
*   [x] **Set up Action Server Project**
    *   [x] Create `action_server/` directory.
    *   [x] Implement basic `action_server/CMakeLists.txt` for gRPC/Protobuf.
    *   [x] Copy `playerbot_rpc.proto` to `action_server/`.
    *   [x] Create placeholder `action_service_impl.h/cpp` for `ActionService`.
    *   [x] Create placeholder `main.cpp` for the action server to start the gRPC service.
    *   [-] Build action server successfully (requires gRPC/Protobuf dev libraries installed in the environment).
*   [x] **Set up Game Server for RPC Client**
    *   [x] Copy `playerbot_rpc.proto` to `playerbot/rpc/`.
    *   [x] Modify root `CMakeLists.txt` to:
        *   [x] Find gRPC and Protobuf libraries.
        *   [x] Add custom command to generate gRPC client stubs from `playerbot/rpc/playerbot_rpc.proto`.
        *   [x] Add generated C++ files to the `playerbots` library sources.
        *   [x] Add include directories for generated files.
        *   [x] Link `playerbots` library against gRPC and Protobuf.
*   [x] **Implement Central RPC Client Manager (`PlayerbotRpcClientMgr`)**
    *   [x] Create `PlayerbotRpcClientMgr.h/cpp`.
    *   [x] Implement as a singleton to manage a shared gRPC channel and `ActionService` stub.
    *   [x] Add `PlayerbotRpcClientMgr.cpp` to game server's `CMakeLists.txt`.
    *   [ ] Ensure `PlayerbotRpcClientMgr::Initialize(sPlayerbotAIConfig.actionServerAddress)` is called appropriately at game server startup (e.g., within `PlayerbotAIConfig::Initialize()` or a similar global init function after configs are loaded, and before any bot AI is created).
*   [x] **Refactor `PlayerbotRpcClient`**
    *   [x] Modify `PlayerbotRpcClient.h/cpp` to use the shared stub from `PlayerbotRpcClientMgr` instead of creating its own channel/stub.
*   [x] **Integrate RPC Client into `PlayerbotAI`**
    *   [x] Add `rpcClient` member (std::unique_ptr<PlayerbotRpcClient>) to `PlayerbotAI.h`.
    *   [x] Add RPC-related method declarations to `PlayerbotAI.h` (`InitRpcClient`, `ShouldUseRpcForDecision`, `ProcessRpcDecision`, `PopulateBotStateSnapshot`).
    *   [x] Create `playerbot/PlayerbotAI_rpc.cpp` for implementations of these methods.
    *   [x] Add `PlayerbotAI_rpc.cpp` to game server's `CMakeLists.txt`.
    *   [x] Implement `PlayerbotAI::InitRpcClient` to use `PlayerbotRpcClientMgr` to initialize its `rpcClient` member.
    *   [x] Ensure `InitRpcClient()` is called in `PlayerbotAI` constructor.
    *   [x] Add conceptual call to `ProcessRpcDecision()` in `PlayerbotAI::UpdateAIInternal()`.

## II. Action Server - AI Logic Implementation

*   [ ] **Flesh out `ActionServiceImpl::GetMacroDecision`**
    *   [ ] Basic parsing of `BotStateSnapshot`.
    *   [ ] Implement logic for simple decisions (e.g., if low health -> suggest "flee" strategy or "use potion" command).
    *   [ ] Adapt parts of existing `PlayerbotAI` strategy selection logic to operate on `BotStateSnapshot` data.
    *   [ ] Gradually expand decision-making capabilities (questing, grinding, travel).
*   [ ] **Data Storage for Action Server (e.g., Redis)**
    *   [ ] Choose and integrate a Redis client library for C++.
    *   [ ] Update `action_server/CMakeLists.txt` for the Redis client.
    *   [ ] Define data structures to store in Redis (e.g., bot's long-term goals, persistent macro state).
    *   [ ] Implement logic in `ActionServiceImpl` to read/write from/to Redis as part of decision-making.

## III. Game Server - Detailed RPC Integration

*   [ ] **Complete `PlayerbotAI::PopulateBotStateSnapshot`**
    *   [ ] Systematically gather all relevant data from `Player* bot` and its environment.
        *   [ ] Accurate BotClass and BotRace mapping to proto enums.
        *   [ ] Comprehensive nearby entities (players, NPCs, game objects), including their type, faction, hostility, etc.
        *   [ ] Detailed active quest information, including progress on all objectives.
        *   [ ] Relevant inventory items (e.g., quest items, consumables count).
        *   [ ] Spell cooldowns relevant for macro decisions.
        *   [ ] Bot's current effective strategies.
        *   [ ] Reputation with relevant factions.
        *   [ ] Known flight paths / travel nodes.
        *   [ ] Group member information.
    *   [ ] Optimize data collection to minimize performance impact.
*   [ ] **Complete `PlayerbotAI::ProcessRpcDecision`**
    *   [ ] Implement handling for all `MacroDecisionResponse::DecisionType` cases:
        *   [ ] `SET_STRATEGY`: Ensure correct `BotState` is used when applying.
        *   [ ] `EXECUTE_COMMAND`: Securely handle command execution.
        *   [ ] `TRAVEL_TO`: Integrate with `TravelMgr` or relevant travel actions. This likely needs a robust "travel <x> <y> <z> [mapid]" command/action.
        *   [ ] `TARGET_ENTITY`: Correctly set target and potentially interrupt current action.
    *   [ ] Add logic for error handling and fallbacks if Action Server is unavailable or returns errors.
*   [ ] **Implement `GameControlService` on Game Server**
    *   [ ] Create `GameControlServiceImpl.h/cpp` inheriting from `playerbot_rpc::GameControlService::Service`.
    *   [ ] Implement `SendBotCommand` method:
        *   [ ] Find target bot by GUID.
        *   [ ] Pass command string to `PlayerbotAI::HandleCommand`.
        *   [ ] Return appropriate success/failure response.
    *   [ ] Set up a gRPC server instance within the game server process.
        *   [ ] This might run in a separate thread.
        *   [ ] Configure listening port.
        *   [ ] Ensure proper startup and shutdown with the main server.
    *   [ ] Implement security/authentication for `GameControlService` if exposed externally.

## IV. Data Synchronization and State Management

*   [ ] **Game Server -> Action Server Flow**
    *   [ ] Ensure `BotStateSnapshot` is sent on each `GetMacroDecision` call.
    *   [ ] Evaluate if periodic state pushes (even without a decision request) are needed for certain data types, or if on-demand is sufficient.
*   [ ] **Action Server State Persistence**
    *   [ ] Finalize what bot-specific macro state the Action Server needs to persist (e.g., in Redis).
    *   [ ] Implement save/load logic for this persistent state.
*   [ ] **Consistency Model**
    *   [ ] Define how to handle potential inconsistencies if the Action Server's cached/stored state diverges from the Game Server's real-time state. (Game server is source of truth).

## V. Testing

*   [ ] **Unit Tests**
    *   [ ] For `ActionServiceImpl` decision logic snippets.
    *   [ ] For `PlayerbotAI::PopulateBotStateSnapshot` data gathering.
    *   [ ] For `PlayerbotAI::ProcessRpcDecision` response handling.
    *   [ ] For `GameControlServiceImpl` command processing.
*   [ ] **Integration Tests**
    *   [ ] Test RPC communication channel (GameServer client to ActionServer).
    *   [ ] Test `GetMacroDecision` call and response flow with basic decisions.
    *   [ ] Test `SendBotCommand` call and response flow.
*   [ ] **Performance Tests**
    *   [ ] Measure latency of `GetMacroDecision` RPC calls.
    *   [ ] Assess overhead of `PopulateBotStateSnapshot`.
    *   [ ] Monitor overall bot responsiveness and server performance impact.
*   [ ] **End-to-End Scenario Tests**
    *   [ ] Bot completing a simple quest guided by Action Server.
    *   [ ] Bot selecting a grinding spot based on Action Server logic.

## V. Testing Strategy Details

This section expands on the testing approaches for the RPC refactor.

### V.A. Unit Tests

*   **Scope:** Focus on individual components in isolation.
*   **Action Server (`action_server`):**
    *   **`ActionServiceImpl`:**
        *   Test specific decision logic branches within `GetMacroDecision`. Mock `BotStateSnapshot` inputs to trigger different decision paths (e.g., low health leading to a "flee" suggestion, presence of a quest mob leading to a "target" suggestion).
        *   If Redis is used: Test Redis interaction logic (e.g., correctly reading/writing bot goals). This might involve a test-specific Redis instance or mocking the Redis client.
    *   **Helper/Utility Functions:** Any new utility functions for parsing state, evaluating conditions, etc.
*   **Game Server (`playerbot` module):**
    *   **`PlayerbotRpcClientMgr`:** Test initialization and stub provision.
    *   **`PlayerbotRpcClient`:** Test `GetMacroDecision` call logic, including deadline handling and error parsing from `grpc::Status`. (Requires a mock gRPC server or service).
    *   **`PlayerbotAI::PopulateBotStateSnapshot`:**
        *   Given a `Player* bot` in a specific state (e.g., in combat, specific quests active, certain items in inventory), verify that the generated `BotStateSnapshot` protobuf message is populated correctly with all expected fields.
        *   Test edge cases (e.g., no target, no quests, empty inventory).
    *   **`PlayerbotAI::ProcessRpcDecision`:**
        *   Given a mock `MacroDecisionResponse` from the Action Server, verify that `PlayerbotAI` correctly interprets it and initiates the appropriate game server actions (e.g., calls `ChangeStrategy`, queues a command, updates `TravelTarget`).
    *   **`GameControlServiceImpl` (when implemented):**
        *   Test `SendBotCommand` logic: correctly finding the bot, passing the command, and returning status. Mock `PlayerbotAI::HandleCommand`.
*   **Tools:** Use a C++ testing framework like Google Test (gtest) or Catch2.

### V.B. Integration Tests

*   **Scope:** Test interactions between components or services.
*   **Game Server <-> Action Server RPC Communication:**
    *   **Basic Connectivity:** Verify that the Game Server can successfully connect to the Action Server and make a `GetMacroDecision` call, and the Action Server can receive it and respond.
    *   **Data Serialization/Deserialization:** Ensure `BotStateSnapshot` and `MacroDecisionResponse` are correctly serialized and deserialized across the RPC boundary. Test with various field values and edge cases.
    *   **Error Handling:**
        *   Test Action Server being unavailable: Game Server should handle this gracefully (e.g., fallback to local AI, log error).
        *   Test RPC timeouts: Game Server should correctly handle calls that exceed the deadline.
        *   Test Action Server returning error statuses: Game Server should interpret these correctly.
*   **Action Server <-> Redis (if used):**
    *   Verify that the Action Server can connect to, read from, and write to the Redis instance correctly.
*   **GameControlService RPC (when implemented):**
    *   Test an external client (or the Action Server itself) sending a command via `SendBotCommand` and verify the command is relayed to the correct bot on the Game Server.
*   **Tools/Setup:**
    *   May require running both Action Server and Game Server instances (or mock versions).
    *   Could use Docker Compose to orchestrate services for testing environments.

### V.C. Performance Tests

*   **Scope:** Measure performance characteristics and identify bottlenecks.
*   **`GetMacroDecision` RPC Latency:**
    *   Measure round-trip time for `GetMacroDecision` calls under various loads (e.g., number of concurrent requests, complexity of decision).
    *   Identify if network latency or Action Server processing time is the dominant factor.
*   **`PopulateBotStateSnapshot` Overhead:**
    *   Profile the time taken by `PlayerbotAI::PopulateBotStateSnapshot` on the Game Server to understand its impact on the game loop, especially with many bots active.
*   **Action Server Throughput:**
    *   Determine how many decisions per second the Action Server can handle.
*   **Resource Utilization:**
    *   Monitor CPU, memory, and network usage of both Game Server and Action Server during load tests.
*   **Tools:** Profiling tools (e.g., gprof, Valgrind for C++), load testing frameworks (e.g., k6, Locust, or custom scripts), system monitoring tools.

### V.D. End-to-End (E2E) / Scenario Tests

*   **Scope:** Test complete user stories or bot behaviors involving the entire RPC flow.
*   **Setup:** Requires a fully running Game Server and Action Server, potentially with a populated game world.
*   **Scenarios:**
    *   **Quest Completion:**
        1.  Bot starts with a simple "collect X items" or "kill Y mobs" quest.
        2.  Game Server sends state to Action Server.
        3.  Action Server decides the bot should pursue the quest, identifies the target mob/item area, and responds with a travel/target/strategy decision.
        4.  Game Server makes the bot execute this (travel, engage, loot).
        5.  Repeat until quest objectives are met.
        6.  Action Server guides bot to quest turn-in NPC.
    *   **Grinding Spot Selection:**
        1.  Bot is idle.
        2.  Action Server analyzes bot level, class, and potentially nearby mob density/level (if this info is passed or cached) and suggests a suitable grinding spot/strategy.
        3.  Bot travels to the spot and begins grinding.
    *   **Dynamic Strategy Change:**
        1.  Bot is grinding. A hostile high-level player appears nearby.
        2.  `BotStateSnapshot` includes the hostile player.
        3.  Action Server decides the bot should switch to a "flee" or "defensive" strategy and responds.
        4.  Game Server applies the new strategy.
    *   **Command Execution via `GameControlService`:**
        1.  Use a test client to send a command (e.g., "follow player X", "use hearthstone") to a bot via the `GameControlService` RPC endpoint on the Game Server.
        2.  Verify the bot executes the command.
*   **Verification:** Observe bot behavior in-game, check logs on both servers, and verify game state changes (e.g., quest completion, inventory changes).

### V.E. Test Environment Considerations

*   **Dedicated Test Environment:** Essential to avoid impacting development or production.
*   **Mocking:** For unit and some integration tests, mock external services (e.g., a mock Action Server for testing Game Server client logic, or a mock Redis for Action Server logic).
*   **Data Management:** For E2E tests, a way to reset or set up specific game world states (e.g., bot characters with specific quests/levels/items) will be beneficial.

This detailed testing strategy should provide good coverage for the RPC refactor.

## VI. Deployment and Configuration

*   [ ] Add configuration options to `aiplayerbot.conf`:
    *   [x] `ActionServer.Address` (e.g., `localhost:50051`)
    *   [x] `ActionServer.RpcTimeoutMs` (e.g., `500`)
    *   [x] `ActionServer.EnableDecisionMaking` (true/false global toggle)
    *   [ ] `GameControlServer.Port` (e.g., `50052`, if different from ActionServer)
    *   [ ] `GameControlServer.Enable` (true/false)
*   [ ] Document how to build and run the Action Server.
*   [ ] Document new configuration options.

## VII. Future Considerations (Post-Initial Implementation)

*   [ ] More sophisticated AI logic in Action Server.
*   [ ] Bi-directional streaming for more dynamic updates.
*   [ ] Secure RPC channels (TLS).
*   [ ] Metrics and monitoring for the Action Server.
*   [ ] Scalability of the Action Server (e.g., handling many bots).

---
*Marked items ([x]) are considered structurally complete or initiated as per current progress.*
*Items with ([-]) indicate partial completion or known environmental limitations.*
