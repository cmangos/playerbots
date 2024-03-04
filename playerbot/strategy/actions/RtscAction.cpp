
#include "playerbot/playerbot.h"
#include "RtscAction.h"


using namespace ai;

bool RTSCAction::Execute(Event& event)
{
	string command = event.getParam();
	Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

	if (!requester)
		return false;

	if (command != "reset" && !requester->HasSpell(RTSC_MOVE_SPELL))
	{
		requester->learnSpell(RTSC_MOVE_SPELL, false);
		ai->TellPlayerNoFacing(requester, "RTS control enabled.");
		ai->TellPlayerNoFacing(requester, "Aedm (Awesome energetic do move) spell trained.");
	}
	else if (command == "reset")
	{
		if (requester->HasSpell(RTSC_MOVE_SPELL))
		{
			requester->removeSpell(RTSC_MOVE_SPELL);
			ai->TellPlayerNoFacing(requester, "RTS control spell removed.");
		}

		RESET_AI_VALUE(bool, "RTSC selected");
		RESET_AI_VALUE(string, "RTSC next spell action");

		for (auto value : ai->GetAiObjectContext()->GetValues())
			if (value.find("RTSC saved location::") != std::string::npos)
				RESET_AI_VALUE(WorldPosition, value.c_str());

		return true;
	}

	bool selected = AI_VALUE(bool, "RTSC selected");

	if (command == "select" && !selected)
	{
		SET_AI_VALUE(bool, "RTSC selected", true);
		requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 5036);
		return true;
	}
	else if (command == "cancel")
	{
		RESET_AI_VALUE(bool, "RTSC selected");
		RESET_AI_VALUE(string, "RTSC next spell action");
		if(selected)
			requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 6372);
		return true;
	}
	else if (command == "toggle")
	{
		if (!selected)
		{
			SET_AI_VALUE(bool, "RTSC selected", true);
			requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 5036);
		}
		else
		{
			SET_AI_VALUE(bool, "RTSC selected", false);
			requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 6372);
		}

		return true;
	}
	else if (command.find("save here ") != std::string::npos)
	{
		string locationName = command.substr(10);

		WorldPosition spellPosition(bot);
		SET_AI_VALUE2(WorldPosition, "RTSC saved location", locationName, spellPosition);

		Creature* wpCreature = bot->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getO(), TEMPSPAWN_TIMED_DESPAWN, 2000.0f);
		wpCreature->SetObjectScale(0.5f);

		return true;
	}
	else if (command.find("unsave ") != std::string::npos)
	{
		string locationName = command.substr(7);

		RESET_AI_VALUE2(WorldPosition, "RTSC saved location", locationName);

		return true;
	}
	if (command.find("save ") != std::string::npos || command == "move")
	{	
		SET_AI_VALUE(string, "RTSC next spell action", command);			

		return true;
	}
	if (command.find("show ") != std::string::npos)
	{
		string locationName = command.substr(5);
		WorldPosition spellPosition = AI_VALUE2(WorldPosition, "RTSC saved location", locationName);

		if (spellPosition)
		{
			Creature* wpCreature = bot->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getO(), TEMPSPAWN_TIMED_DESPAWN, 2000.0f);
			wpCreature->SetObjectScale(0.5f);
		}

		return true;
	}
	if (command.find("show") != std::string::npos)
	{
		ostringstream out; out << "saved: ";

		for (auto value : ai->GetAiObjectContext()->GetValues())
			if (value.find("RTSC saved location::") != std::string::npos)
				if (AI_VALUE2(WorldPosition, "RTSC saved location", value.substr(21).c_str()))
					out << value.substr(21).c_str() << ",";

		out.seekp(-1, out.cur);
		out << ".";

		ai->TellPlayerNoFacing(requester, out);
	}
	if (command.find("go ") != std::string::npos)
	{
		string locationName = command.substr(3);
		WorldPosition spellPosition = AI_VALUE2(WorldPosition, "RTSC saved location", locationName);

		if(spellPosition)
			return MoveToSpell(requester, spellPosition, false);

		return true;
	}
	else if (command == "last")
	{
		WorldPosition spellPosition = AI_VALUE(WorldPosition, "see spell location");
		if (spellPosition)
			return MoveToSpell(requester, spellPosition);
	}

    if (command == "jump")
    {
        WorldPosition spellPosition = AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
        if (!spellPosition)
        {
            SET_AI_VALUE(string, "RTSC next spell action", command);
        }
        else
        {
            WorldPosition jumpPosition = AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
            if (!jumpPosition)
            {
                RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
                RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
                ai->ChangeStrategy("-rtsc jump", BotState::BOT_STATE_NON_COMBAT);
                ostringstream out;
                out << "Can't finish previous jump! Cancelling...";
                ai->TellError(requester, out.str());
            }
            else
            {
                ostringstream out;
                out << "Another jump is in process! Use 'rtsc jump reset' to stop it";
                ai->TellError(requester, out.str());
                return false;
            }
        }

        return true;
    }
    if (command == "jump reset")
    {
        RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
        RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
        ai->ChangeStrategy("-rtsc jump", BotState::BOT_STATE_NON_COMBAT);

        return true;
    }

	return false;
}
