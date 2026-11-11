
#include "playerbot/playerbot.h"
#include "ChangeStrategyAction.h"
#include "playerbot/PlayerbotAIConfig.h"

using namespace ai;

bool ChangeCombatStrategyAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    text = text.empty() ? getName() : text;

    if (text == "save")
    {
        ai->SaveCombatOrders();
        ai->TellPlayerNoFacing(requester, "Saved combat orders.");
        return true;
    }

    if (text == "save once")
    {
        ai->SaveCombatOrders(true);
        return true;
    }

    if (text.compare(0, 7, "legacy ") == 0)
    {
        std::string order = text.substr(7);
        ai->SaveCombatOrders(true);

        if (order == "reset")
        {
            ai->ChangeStrategy("-tank,-heal,-dps assist,-offheal,-passive", BotState::BOT_STATE_COMBAT);
            ai->DoSpecificAction("order", Event("co", "clear", requester), true);
            ai->DoSpecificAction("resist", Event("co", "none", requester), true);
            ai->TellPlayerNoFacing(requester, "Combat orders reset.");
        }
        else if (order.compare(0, 7, "resist ") == 0)
            ai->DoSpecificAction("resist", Event("co", order.substr(7), requester), true);
        else if (order.compare(0, 6, "delay ") == 0)
            ai->DoSpecificAction("wait for attack time", Event("co", order.substr(6), requester), true);
        else
        {
            if (order.compare(0, 4, "add ") == 0)
                order = order.substr(4);
            else
                ai->ChangeStrategy("-tank,-heal,-offheal,-dps assist,-passive", BotState::BOT_STATE_COMBAT);

            ai->ChangeStrategy(order, BotState::BOT_STATE_COMBAT);
            const size_t nameStart = order.find_first_not_of('+');
            ai->TellPlayerNoFacing(requester, "Combat order set: " + order.substr(nameStart == std::string::npos ? 0 : nameStart));
        }

        if (!sPlayerbotAIConfig.bExplicitDbStoreSave)
            sPlayerbotDbStore.Save(ai);

        return true;
    }

    if (text == "restore")
    {
        const bool restored = ai->RestoreCombatOrders();
        if (restored && !sPlayerbotAIConfig.bExplicitDbStoreSave)
            sPlayerbotDbStore.Save(ai);

        ai->TellPlayerNoFacing(requester, restored ? "Restored saved combat orders." : "No combat orders have been saved.");
        return true;
    }

    ai->ChangeStrategy(text, BotState::BOT_STATE_COMBAT);

    if (!sPlayerbotAIConfig.bExplicitDbStoreSave)
    {
       if (event.getSource() == "co")
       {
          std::vector<std::string> splitted = split(text, ',');
          CharacterDatabase.BeginTransaction();
          for (std::vector<std::string>::iterator i = splitted.begin(); i != splitted.end(); i++)
          {
             const char* name = i->c_str();
             switch (name[0])
             {
             case '+':
             case '-':
             case '~':
                sPlayerbotDbStore.Save(ai);
                break;
             }
          }
          CharacterDatabase.CommitTransaction();
       }
    }

    if (text.find("?") != std::string::npos)
    {
        ai->PrintStrategies(requester, BotState::BOT_STATE_COMBAT);
    }
    
    return true;
}

bool ChangeNonCombatStrategyAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    text = text.empty() ? getName() : text;

    ai->ChangeStrategy(text, BotState::BOT_STATE_NON_COMBAT);

    if (!sPlayerbotAIConfig.bExplicitDbStoreSave)
    {
       if (event.getSource() == "nc")
       {
          std::vector<std::string> splitted = split(text, ',');
          CharacterDatabase.BeginTransaction();
          for (std::vector<std::string>::iterator i = splitted.begin(); i != splitted.end(); i++)
          {
             const char* name = i->c_str();
             switch (name[0])
             {
             case '+':
             case '-':
             case '~':
                sPlayerbotDbStore.Save(ai);
                break;
             }
          }
          CharacterDatabase.CommitTransaction();
       }
    }

    if (text.find("?") != std::string::npos)
    {
        ai->PrintStrategies(requester, BotState::BOT_STATE_NON_COMBAT);
    }

    return true;
}

bool ChangeDeadStrategyAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    text = text.empty() ? getName() : text;

    ai->ChangeStrategy(text, BotState::BOT_STATE_DEAD);

    if (text.find("?") != std::string::npos)
    {
        ai->PrintStrategies(requester, BotState::BOT_STATE_DEAD);
    }

    return true;
}

bool ChangeReactionStrategyAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    text = text.empty() ? getName() : text;

    ai->ChangeStrategy(text, BotState::BOT_STATE_REACTION);

    if (text.find("?") != std::string::npos)
    {
        ai->PrintStrategies(requester, BotState::BOT_STATE_REACTION);
    }

    return true;
}

bool ChangeAllStrategyAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    std::string strategyName = text.empty() ? strategy : text;

    ai->ChangeStrategy(strategyName, BotState::BOT_STATE_ALL);

    if (!sPlayerbotAIConfig.bExplicitDbStoreSave)
    {
       if (event.getSource() == "nc" || event.getSource() == "co")
       {
          std::vector<std::string> splitted = split(text, ',');
          CharacterDatabase.BeginTransaction();
          for (std::vector<std::string>::iterator i = splitted.begin(); i != splitted.end(); i++)
          {
             const char* name = i->c_str();
             switch (name[0])
             {
             case '+':
             case '-':
             case '~':
             {
                sPlayerbotDbStore.Save(ai);
                break;
             }
             }
          }
          CharacterDatabase.CommitTransaction();
       }
    }

    if (text.find("?") != std::string::npos)
    {
        ai->PrintStrategies(requester, BotState::BOT_STATE_ALL);
    }

    return true;
}
