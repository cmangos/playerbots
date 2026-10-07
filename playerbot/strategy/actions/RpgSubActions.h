#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "ChooseRpgTargetAction.h"
#include "UseItemAction.h"
#include "playerbot/strategy/values/LastMovementValue.h"
#include "playerbot/strategy/values/CraftValues.h"
#include "SayAction.h"

namespace ai
{
    class RpgHelper : public AiObject
    {
    public:
        RpgHelper(PlayerbotAI* ai) : AiObject(ai) {}

        void BeforeExecute();
        void AfterExecute(bool doDelay = true,  bool waitForGroup = false, std::string nextAction = "rpg");
        void OnCancel() { resetFacing(guidP()); if (bot->GetTradeData()) bot->TradeCancel(true); };

        virtual GuidPosition guidP() { return AI_VALUE(GuidPosition, "rpg target"); }
        virtual ObjectGuid guid() { return (ObjectGuid)guidP(); }        
        virtual bool InRange() { return AI_VALUE2(float, "distance", "rpg target") <= INTERACTION_DISTANCE * 1.5; }
        void setDelay(bool waitForGroup);
    private:
        void setFacingTo(GuidPosition guidPosition);
        void setFacing(GuidPosition guidPosition);
        void resetFacing(GuidPosition guidPosition);
    };

    class RpgEnabled
    {
    public:
        RpgEnabled(PlayerbotAI* ai) { rpg = std::make_unique<RpgHelper>(ai); }
    protected:
        std::unique_ptr<RpgHelper> rpg;
    };

    class RpgSubAction : public Action, public RpgEnabled
    {
    public:
        RpgSubAction(PlayerbotAI* ai, std::string name = "rpg sub") : Action(ai, name), RpgEnabled(ai) {}

        //Long range is possible?
        virtual bool isPossible() override { return rpg->guidP() && rpg->guidP().GetWorldObject(bot->GetInstanceId());}
        //Short range can we do the action now?
        virtual bool isUseful() override { return rpg->InRange(); }

        virtual bool Execute(Event& event) override { rpg->BeforeExecute();  bool doAction = ai->DoSpecificAction(ActionName(), ActionEvent(event), true); rpg->AfterExecute(doAction, true); DoDelay(); return doAction; }

        virtual std::string GetRpgActionName() const { return "generic rpg action"; };
    protected:
        void DoDelay(){ SetDuration(ai->GetAIInternalUpdateDelay()); }
        virtual std::string ActionName() { return "none"; }
        virtual Event ActionEvent(Event event) { return event; }
    };        

    class RpgStayAction : public RpgSubAction
    {
    public:
        RpgStayAction(PlayerbotAI* ai, std::string name = "rpg stay") : RpgSubAction(ai, name) {}

        //virtual bool isUseful() override { return rpg->InRange() && !ai->HasRealPlayerMaster(); }

        virtual std::string GetRpgActionName() const override { return "idling near"; };

        virtual bool Execute(Event& event) override { rpg->BeforeExecute(); if (bot->GetPlayerMenu()) bot->GetPlayerMenu()->CloseGossip(); rpg->AfterExecute(); DoDelay(); return true; };
    };   

    class RpgWorkAction : public RpgSubAction
    {
    public:
        RpgWorkAction(PlayerbotAI* ai, std::string name = "rpg work") : RpgSubAction(ai, name ) {}

        //virtual bool isUseful() override { return rpg->InRange() && !ai->HasRealPlayerMaster(); }

        virtual std::string GetRpgActionName() const override { return "working next to"; };

        virtual bool Execute(Event& event) override { rpg->BeforeExecute(); bot->HandleEmoteCommand(EMOTE_STATE_USESTANDING); rpg->AfterExecute(); DoDelay(); return true; };
    };

    class RpgEmoteAction : public RpgSubAction
    {
    public:
        RpgEmoteAction(PlayerbotAI* ai, std::string name = "rpg emote") : RpgSubAction(ai, name) {}

       virtual std::string GetRpgActionName() const override { return "chatting with"; };

        //virtual bool isUseful() override { return rpg->InRange() && !ai->HasRealPlayerMaster(); }

        virtual bool Execute(Event& event) override;
    };

    class RpgCancelAction : public RpgSubAction
    {
    public:
        RpgCancelAction(PlayerbotAI* ai, std::string name = "rpg cancel") : RpgSubAction(ai, name) {}

        virtual bool isUseful() override {return rpg->InRange();}

        virtual std::string GetRpgActionName() const override { return "leaving"; };

        virtual bool Execute(Event& event) override;
    };

    class RpgTaxiAction : public RpgSubAction
    {
    public:
        RpgTaxiAction(PlayerbotAI* ai, std::string name = "rpg taxi") : RpgSubAction(ai, name) {}

        virtual bool isUseful() override { return rpg->InRange() && !ai->HasRealPlayerMaster() && bot->GetGroup(); }

        virtual std::string GetRpgActionName() const override { return "grabbing a taxi from"; };

        virtual bool Execute(Event& event) override;
    };

    class RpgDiscoverAction : public RpgTaxiAction
    {
    public:
        RpgDiscoverAction(PlayerbotAI* ai, std::string name = "rpg discover") : RpgTaxiAction(ai,name) {}

        virtual bool isUseful() override { return rpg->InRange(); }

        virtual std::string GetRpgActionName() const override { return "discovering taxi paths from"; };

        virtual bool Execute(Event& event) override;
    };

    class RpgStartQuestAction : public RpgSubAction
    {
    public:
        RpgStartQuestAction(PlayerbotAI* ai, std::string name = "rpg start quest") : RpgSubAction(ai, name) {}
        virtual bool Execute(Event& event) override { rpg->BeforeExecute();  bool doAction = ai->DoSpecificAction(ActionName(), ActionEvent(event), true); rpg->AfterExecute(doAction, true, ""); DoDelay(); return doAction; }

        virtual std::string GetRpgActionName() const override { return "starting a quest at"; };
    private:
        virtual std::string ActionName() override { return "accept all quests"; }
        virtual Event ActionEvent(Event event) override {WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST); p << rpg->guid(); p.rpos(0);  return  Event("rpg action", p); }
    };

    class RpgEndQuestAction : public RpgStartQuestAction
    {
    public:
        RpgEndQuestAction(PlayerbotAI* ai, std::string name = "rpg end quest") : RpgStartQuestAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "handing in a quest at"; };
    private:
        virtual std::string ActionName() override { return "talk to quest giver"; }
        virtual Event ActionEvent(Event event) override { WorldPacket p(CMSG_QUESTGIVER_COMPLETE_QUEST); p << rpg->guid(); p.rpos(0);  return  Event("rpg action", p); }
    };

    class RpgBuyAction : public RpgSubAction
    {
    public:
        RpgBuyAction(PlayerbotAI* ai, std::string name = "rpg buy") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "buying items from"; };
    private:
        virtual std::string ActionName() override { return "buy"; }
        virtual Event ActionEvent(Event event) override { return Event("rpg action", "vendor"); }
    };
   
    class RpgSellAction : public RpgSubAction
    {
    public:
        RpgSellAction(PlayerbotAI* ai, std::string name = "rpg sell") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "selling items to"; };
    private:
        virtual std::string ActionName() override { return "sell"; }
        virtual Event ActionEvent(Event event) override { return Event("rpg action", "vendor"); }
    };

    class RpgAHSellAction : public RpgSubAction
    {
    public:
        RpgAHSellAction(PlayerbotAI* ai, std::string name = "rpg ah sell") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "posting items on ah at"; };
    private:
        virtual std::string ActionName() override { return "ah"; }
        virtual Event ActionEvent(Event event) override { return Event("rpg action", "vendor"); }
    };

    class RpgAHBuyAction : public RpgSubAction
    {
    public:
        RpgAHBuyAction(PlayerbotAI* ai, std::string name = "rpg ah buy") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "bidding on items on ah at"; };
    private:
        virtual std::string ActionName() override { return "ah bid"; }
        virtual Event ActionEvent(Event event) override
        {
            ProfessionCraftingPlan plan = AI_VALUE(ProfessionCraftingPlan, "profession crafting plan");
            return Event("rpg action",
                ProfessionCraftingPlanValue::ShouldTravelToAuctionHouse(ai, plan) ? "profession" : "vendor");
        }
    };

    class RpgGetMailAction : public RpgSubAction
    {
    public:
        RpgGetMailAction(PlayerbotAI* ai, std::string name = "rpg get mail") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "getting mail from"; };
    private:
        virtual std::string ActionName() override { return "mail"; }
        virtual Event ActionEvent(Event event) override { return Event("rpg action", "take"); }
       
        virtual bool Execute(Event& event) override { bool doAction = RpgSubAction::Execute(event); if (doAction) ai->DoSpecificAction("equip upgrades", event, true); return doAction; }
    };

    class RpgRepairAction : public RpgSubAction
    {
    public:
        RpgRepairAction(PlayerbotAI* ai, std::string name = "rpg repair") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "repairing at"; };
    private:
        virtual std::string ActionName() override { return "repair"; }
    };

    class RpgTrainAction : public RpgSubAction
    {
    public:
        RpgTrainAction(PlayerbotAI* ai, std::string name = "rpg train") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "training a new skill at"; };
    private:
        virtual std::string ActionName() override { return "trainer"; }
        virtual Event ActionEvent(Event event) override { return Event("rpg action",rpg->guidP()); }
    };

    class RpgHealAction : public RpgSubAction
    {
    public:
        RpgHealAction(PlayerbotAI* ai, std::string name = "rpg heal") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "healing"; };

        virtual bool Execute(Event& event) override;
    };

    class RpgHomeBindAction : public RpgSubAction
    {
    public:
        RpgHomeBindAction(PlayerbotAI* ai, std::string name = "rpg home bind") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "making this inn my home at"; };
    private:
        virtual std::string ActionName() override { return "home"; }
    };

    class RpgQueueBgAction : public RpgSubAction
    {
    public:
        RpgQueueBgAction(PlayerbotAI* ai, std::string name = "rpg queue bg") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "queing for a battleground at"; };
    private:
        virtual std::string ActionName() override { SET_AI_VALUE(uint32, "bg type", (uint32)AI_VALUE(BattleGroundTypeId, "rpg bg type")); return "free bg join"; }
    };

    class RpgBuyPetitionAction : public RpgSubAction
    {
    public:
        RpgBuyPetitionAction(PlayerbotAI* ai, std::string name = "rpg buy petition") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "buying a guild pettition from"; };
    private:
        virtual std::string ActionName() override { return "buy petition"; }
    };

    class RpgUseAction : public RpgSubAction
    {
    public:
        RpgUseAction(PlayerbotAI* ai, std::string name = "rpg use") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "using"; };

        virtual bool Execute(Event& event) override {
            rpg->BeforeExecute();
            bool doAction = ai->DoSpecificAction(ActionName(), ActionEvent(event), true);
            rpg->AfterExecute(true);
            DoDelay();
            return doAction;
        }

    private:
        virtual bool isUseful() override;

        virtual std::string ActionName() override { if (rpg->guidP().IsGameObject() && rpg->guidP().GetGameObject(bot->GetInstanceId())->GetGoType() == GAMEOBJECT_TYPE_CHEST) return "add all loot";  return "use"; }
        virtual Event ActionEvent(Event event) override { return Event("rpg action", chat->formatWorldobject(rpg->guidP().GetWorldObject(bot->GetInstanceId()))); }
    };

    class RpgAIChatAction : public RpgSubAction
    {
    public:
        RpgAIChatAction(PlayerbotAI* ai, std::string name = "rpg ai chat") : RpgSubAction(ai, name) {}

        void ManualChat(GuidPosition target, const std::string& line);

        virtual std::string GetRpgActionName() const override { return "ai talking with"; };
    private:
        virtual bool isUseful() override;

        bool SpeakLine();
        bool WaitForLines();
        bool RequestNewLines();
        virtual bool Execute(Event& event) override;

        std::queue<delayedPacket> packets;
        futurePackets futPackets;
    };

    class RpgSpellAction : public RpgSubAction
    {
    public:
        RpgSpellAction(PlayerbotAI* ai, std::string name = "rpg spell") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "casting a spell on"; };
    private:
        virtual bool isUseful() override { return true; }
        virtual std::string ActionName() override { return "cast random spell"; }
        virtual Event ActionEvent(Event event) override { return Event("rpg action", chat->formatWorldobject(rpg->guidP().GetWorldObject(bot->GetInstanceId()))); }
    };

    class RpgCraftAction : public RpgSubAction
    {
    public:
        RpgCraftAction(PlayerbotAI* ai, std::string name = "rpg craft") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return rpg->guidP().IsGameObject() ? "crafting an item at" : "crafting an item near"; };
    private:
        virtual std::string ActionName() override { return "craft random item"; }
        virtual Event ActionEvent(Event event) override { return Event("rpg action", chat->formatWorldobject(rpg->guidP().GetWorldObject(bot->GetInstanceId()))); }
    };

    class RpgTradeUsefulAction : public RpgSubAction
    {
    public:
        RpgTradeUsefulAction(PlayerbotAI* ai, std::string name = "rpg trade useful") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "trading an item to"; };

        bool IsTradingItem(uint32 entry);

        virtual bool Execute(Event& event) override;
    };

    class RpgEnchantAction : public RpgTradeUsefulAction
    {
    public:
        RpgEnchantAction(PlayerbotAI* ai, std::string name = "rpg enchant") : RpgTradeUsefulAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "enchanting an item for"; };

        virtual bool Execute(Event& event) override;
    };

    class RpgDuelAction : public RpgSubAction
    {
    public:
        RpgDuelAction(PlayerbotAI* ai, std::string name = "rpg duel") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "starting a duel with"; };

        virtual bool isUseful() override;
        virtual bool Execute(Event& event) override;
    };

    class RpgItemAction : public UseAction, public RpgEnabled
    {
    public:
        RpgItemAction(PlayerbotAI* ai, std::string name = "rpg item") : UseAction(ai, name), RpgEnabled(ai) {}

        virtual std::string GetRpgActionName() const { return "using an item on"; };

        //Long range is possible?
        virtual bool isPossible() override { return rpg->guidP() && rpg->guidP().GetWorldObject(bot->GetInstanceId()); }
        //Short range can we do the action now?
        virtual bool isUseful() override { return !urand(0,3) || rpg->InRange(); }

        virtual bool Execute(Event& event) override;
    };

    class RpgSpellClickAction : public RpgSubAction
    {
    public:
        RpgSpellClickAction(PlayerbotAI* ai, std::string name = "rpg spell click") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "spellclicking"; };

        virtual bool Execute(Event& event) override;
    };

    class RpgGossipTalkAction : public RpgSubAction
    {
    public:
        RpgGossipTalkAction(PlayerbotAI* ai, std::string name = "rpg gossip talk") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "gossiping with"; };
    private:
        virtual std::string ActionName() override { return "gossip hello"; }
        virtual Event ActionEvent(Event event) override { WorldPacket p(CMSG_GOSSIP_SELECT_OPTION); p << rpg->guid(); p.rpos(0); return Event("rpg action", p); }
    };

    class RpgBankDepositAction : public RpgSubAction
    {
    public:
        RpgBankDepositAction(PlayerbotAI* ai, std::string name = "rpg bank deposit") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "depositing items at"; };

        virtual bool Execute(Event& event) override;
    };

    class RpgBankWithdrawAction : public RpgSubAction
    {
    public:
        RpgBankWithdrawAction(PlayerbotAI* ai, std::string name = "rpg bank withdraw") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "withdrawing items from"; };

        virtual bool Execute(Event& event) override;
    };

#ifndef MANGOSBOT_ZERO
    class RpgGuildBankDepositAction : public RpgSubAction
    {
    public:
        RpgGuildBankDepositAction(PlayerbotAI* ai, std::string name = "rpg guild bank deposit") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "depositing items in guild bank at"; };

        virtual bool Execute(Event& event) override;
    };

    class RpgGuildBankWithdrawAction : public RpgSubAction
    {
    public:
        RpgGuildBankWithdrawAction(PlayerbotAI* ai, std::string name = "rpg guild bank withdraw") : RpgSubAction(ai, name) {}

        virtual std::string GetRpgActionName() const override { return "withdrawing items from guild bank at"; };

        virtual bool Execute(Event& event) override;
    };
#endif
}
