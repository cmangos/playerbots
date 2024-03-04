#pragma once
#include "../Value.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    // List of hostile targets that are in combat with the bot (or bot group)
    class AttackersValue : public ObjectGuidListCalculatedValue, public Qualified
	{
        struct AddGuardiansHelper
        {
            explicit AddGuardiansHelper(set<Unit*>& units) : units(units) {}
            void operator()(Unit* target) const
            {
                units.insert(target);
            }

            set<Unit*>& units;
        };

	public:
        AttackersValue(PlayerbotAI* ai, string name = "attackers", int interval = 2) : ObjectGuidListCalculatedValue(ai, name, interval), Qualified() {}
        virtual list<ObjectGuid> Calculate() override;

        static bool IsValid(Unit* target, Player* player, Player* owner = nullptr, bool checkInCombat = true, bool validatePossibleTarget = true);
        virtual string Format();

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "attackers"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "combat"; }
        virtual string GetHelpDescription()
        {
            return "This value contains all the units attacking the player, it's group or those the player or group is attacking.";
        }
        virtual vector<string> GetUsedValues() { return {"possible targets", "current target" , "attack target" ,  "pull target" }; }
#endif

	private:
        void AddTargetsOf(Group* group, set<Unit*>& targets, set<ObjectGuid>& invalidTargets, bool getOne = false);
        void AddTargetsOf(Player* player, set<Unit*>& targets, set<ObjectGuid>& invalidTargets, bool getOne = false);
        static float GetRange() { return sPlayerbotAIConfig.sightDistance; }

        static bool InCombat(Unit* target, Player* player, bool checkPullTargets = true);

        WorldPosition calculatePos;
    };

    // List of attackers that are currently targeting the bot
    class AttackersTargetingMeValue : public ObjectGuidListCalculatedValue
    {
    public:
        AttackersTargetingMeValue(PlayerbotAI* ai, string name = "attackers targeting me") : ObjectGuidListCalculatedValue(ai, name) {}
        list<ObjectGuid> Calculate() override;
    };
}
