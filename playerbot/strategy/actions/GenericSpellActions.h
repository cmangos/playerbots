#pragma once
#include "playerbot/strategy/Action.h"
#include "playerbot/PlayerbotAIConfig.h"

namespace ai
{
    class CastSpellAction : public Action
    {
    public:
        CastSpellAction(PlayerbotAI* ai, string spell);
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_SINGLE; }
        virtual bool Execute(Event& event) override;
        virtual bool isPossible() override;
		virtual bool isUseful() override;

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptCast() const override { return true; }

        bool HasReachAction() { return !GetReachActionName().empty(); }
        
    protected:
        const uint32& GetSpellID() const { return spellId; }
        const string& GetSpellName() const { return spellName; }
        void SetSpellName(const string& name, string spellIDContextName = "spell id");

        Unit* GetTarget() override;
        virtual string GetTargetName() override { return "current target"; }
        virtual string GetTargetQualifier() { return ""; }
        virtual string GetReachActionName() { return "reach spell"; }

        virtual NextAction** getPrerequisites() override;

    protected:
		float range;

    private:
        string spellName;
        uint32 spellId;
    };

    class CastPetSpellAction : public CastSpellAction
    {
    public:
        CastPetSpellAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) {}

        virtual bool isPossible() override;

    protected:
        virtual string GetTargetName() override { return "current target"; }
        string GetReachActionName() override { return ""; }
    };

	//---------------------------------------------------------------------------------------------------------------------
	class CastAuraSpellAction : public CastSpellAction
	{
	public:
		CastAuraSpellAction(PlayerbotAI* ai, string spell, bool isOwner = false) : CastSpellAction(ai, spell), isOwner(isOwner) {}
		virtual bool isUseful() override;

    protected:
        virtual string GetReachActionName() override { return "reach spell"; }

    protected:
        bool isOwner;
	};

    //---------------------------------------------------------------------------------------------------------------------
    class CastMeleeSpellAction : public CastSpellAction
    {
    public:
        CastMeleeSpellAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) 
        {
            range = ATTACK_DISTANCE;
        }

    protected:
        virtual string GetReachActionName() override { return "reach melee"; }
    };

    //---------------------------------------------------------------------------------------------------------------------
    class CastMeleeAoeSpellAction : public CastSpellAction
    {
    public:
        CastMeleeAoeSpellAction(PlayerbotAI* ai, string spell, float radius) : CastSpellAction(ai, spell), radius(radius)
        {
            range = ATTACK_DISTANCE;
        }

        virtual bool isUseful() override;

    protected:
        virtual string GetReachActionName() override { return ""; }

    protected:
        float radius;
    };

    //---------------------------------------------------------------------------------------------------------------------
    class CastMeleeDebuffSpellAction : public CastAuraSpellAction
    {
    public:
        CastMeleeDebuffSpellAction(PlayerbotAI* ai, string spell, bool isOwner = true) : CastAuraSpellAction(ai, spell, isOwner)
        {
            range = ATTACK_DISTANCE;
        }
    
    protected:
        virtual string GetReachActionName() override { return "reach melee"; }
    };
    
    class CastRangedDebuffSpellAction : public CastAuraSpellAction
    {
    public:
        CastRangedDebuffSpellAction(PlayerbotAI* ai, string spell, bool isOwner = true) : CastAuraSpellAction(ai, spell, isOwner) {}
    
    protected:
        virtual string GetReachActionName() override { return "reach spell"; }
    };

    class CastMeleeDebuffSpellOnAttackerAction : public CastAuraSpellAction
    {
    public:
        CastMeleeDebuffSpellOnAttackerAction(PlayerbotAI* ai, string spell, bool isOwner = true) : CastAuraSpellAction(ai, spell, isOwner)
        {
            range = ATTACK_DISTANCE;
        }

    protected:
        string GetReachActionName() override { return "reach melee"; }
        string GetTargetName() override { return "attacker without aura"; }
        string GetTargetQualifier() override { return GetSpellName(); }
        virtual string getName() override { return GetSpellName() + " on attacker"; }
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_AOE; }
    };

    class CastRangedDebuffSpellOnAttackerAction : public CastAuraSpellAction
    {
    public:
        CastRangedDebuffSpellOnAttackerAction(PlayerbotAI* ai, string spell, bool isOwner = true) : CastAuraSpellAction(ai, spell, isOwner) {}
        
    protected:
        virtual string GetReachActionName() override { return "reach spell"; }
        virtual string GetTargetName() override { return "attacker without aura"; }
        virtual string GetTargetQualifier() override { return GetSpellName(); }
        virtual string getName() override { return GetSpellName() + " on attacker"; }
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_AOE; }
    };

	class CastBuffSpellAction : public CastAuraSpellAction
	{
	public:
		CastBuffSpellAction(PlayerbotAI* ai, string spell) : CastAuraSpellAction(ai, spell) { }
        virtual string GetTargetName() override { return "self target"; }
	};

	class CastEnchantItemAction : public CastSpellAction
	{
	public:
	    CastEnchantItemAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) { }
        virtual string GetTargetName() override { return "self target"; }
        virtual bool isPossible() override;
	};

    //---------------------------------------------------------------------------------------------------------------------

    class CastHealingSpellAction : public CastAuraSpellAction
    {
    public:
        CastHealingSpellAction(PlayerbotAI* ai, string spell, uint8 estAmount = 15.0f) : CastAuraSpellAction(ai, spell, true), estAmount(estAmount) {}
        
    protected:
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_AOE; }
        virtual string GetTargetName() override { return "self target"; }
        virtual string GetReachActionName() override { return "reach party member to heal"; }

    protected:
        uint8 estAmount;
    };

    class CastAoeHealSpellAction : public CastHealingSpellAction
    {
    public:
    	CastAoeHealSpellAction(PlayerbotAI* ai, string spell, uint8 estAmount = 15.0f) : CastHealingSpellAction(ai, spell, estAmount) {}
		virtual string GetTargetName() override { return "party member to heal"; }
        virtual bool isUseful() override;
    };

	class CastCureSpellAction : public CastSpellAction
	{
	public:
		CastCureSpellAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) {}
		virtual string GetTargetName() override { return "self target"; }
	};

	class PartyMemberActionNameSupport 
    {
	public:
		PartyMemberActionNameSupport(string spell) : name(spell + " on party") {}
		string getName() { return name; }

	private:
		string name;
	};

    class HealPartyMemberAction : public CastHealingSpellAction, public PartyMemberActionNameSupport
    {
    public:
        HealPartyMemberAction(PlayerbotAI* ai, string spell, uint8 estAmount = 15.0f) : CastHealingSpellAction(ai, spell, estAmount), PartyMemberActionNameSupport(spell) {}
        virtual string getName() override { return PartyMemberActionNameSupport::getName(); }
		virtual string GetTargetName() override { return "party member to heal"; }
    };

    class HealHotPartyMemberAction : public HealPartyMemberAction
    {
    public:
        HealHotPartyMemberAction(PlayerbotAI* ai, string spell) : HealPartyMemberAction(ai, spell) {}
        virtual bool isUseful() override;
    };

	class ResurrectPartyMemberAction : public CastSpellAction
	{
	public:
		ResurrectPartyMemberAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) {}
		
    protected:
        virtual string GetTargetName() override { return "party member to resurrect"; }
        virtual string GetReachActionName() override { return "reach party member to heal"; }
	};
    //---------------------------------------------------------------------------------------------------------------------

    class CurePartyMemberAction : public CastSpellAction, public PartyMemberActionNameSupport
    {
    public:
        CurePartyMemberAction(PlayerbotAI* ai, string spell, uint32 dispelType) : CastSpellAction(ai, spell), PartyMemberActionNameSupport(spell), dispelType(dispelType) {}
    
    protected:
        virtual string GetReachActionName() override { return "reach party member to heal"; }
        virtual string getName() override { return PartyMemberActionNameSupport::getName(); }
        virtual string GetTargetName() override { return "party member to dispel"; }
        virtual string GetTargetQualifier() override { return std::to_string(dispelType); }

    protected:
        uint32 dispelType;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class BuffOnPartyAction : public CastBuffSpellAction, public PartyMemberActionNameSupport
    {
    public:
        BuffOnPartyAction(PlayerbotAI* ai, string spell, bool ignoreTanks = false) : CastBuffSpellAction(ai, spell), PartyMemberActionNameSupport(spell), ignoreTanks(ignoreTanks) {}
        
    protected:
        virtual string getName() override { return PartyMemberActionNameSupport::getName(); }
        virtual string GetTargetName() override { return "friendly unit without aura"; }
        virtual string GetTargetQualifier() override { return GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }

    protected:
        bool ignoreTanks;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class GreaterBuffOnPartyAction : public CastBuffSpellAction, public PartyMemberActionNameSupport
    {
    public:
        GreaterBuffOnPartyAction(PlayerbotAI* ai, string spell, bool ignoreTanks = false) : CastBuffSpellAction(ai, spell), PartyMemberActionNameSupport(spell), ignoreTanks(ignoreTanks) {}

    protected:
        virtual string getName() override { return PartyMemberActionNameSupport::getName(); }
        virtual string GetTargetName() override { return "party member without aura"; }
        virtual string GetTargetQualifier() override { return GetSpellName() + "-" + (ignoreTanks ? "1" : "0"); }

    private:
        bool ignoreTanks;
    };

    //---------------------------------------------------------------------------------------------------------------------

    class PartyTankActionNameSupport
    {
    public:
        PartyTankActionNameSupport(string spell) : name(spell + " on tank") {}
        string getName() { return name; }

    private:
        string name;
    };

    class BuffOnTankAction : public CastBuffSpellAction, public PartyMemberActionNameSupport
    {
    public:
        BuffOnTankAction(PlayerbotAI* ai, string spell) : CastBuffSpellAction(ai, spell), PartyMemberActionNameSupport(spell) {}

    protected:
        virtual string getName() override { return PartyMemberActionNameSupport::getName(); }
        virtual string GetTargetName() override { return "party tank without aura"; }
        virtual string GetTargetQualifier() override { return GetSpellName(); }
    };

    class CastShootAction : public CastSpellAction
    {
    public:
        CastShootAction(PlayerbotAI* ai) : CastSpellAction(ai, "shoot"), rangedWeapon(nullptr), weaponDelay(0), needsAmmo(false) {}
        ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }
        bool Execute(Event& event) override;
        bool isPossible() override;

    protected:
        virtual string GetReachActionName() override { return "reach spell"; }

    private:
        void UpdateWeaponInfo();

    private:
        const Item* rangedWeapon;
        uint32 weaponDelay;
        bool needsAmmo;
    };

    class RemoveBuffAction : public Action
    {
    public:
        RemoveBuffAction(PlayerbotAI* ai, string spell) : Action(ai, "remove aura"), name(spell) {}
        virtual string getName() override { return "remove " + name; }
        virtual bool isUseful() override;
        virtual bool Execute(Event& event) override;

    private:
        string name;
    };

    // racials

    // heal
#ifndef MANGOSBOT_ZERO
    HEAL_ACTION(CastGiftOfTheNaaruAction, "gift of the naaru");
#endif
    HEAL_ACTION(CastCannibalizeAction, "cannibalize");

    // buff

    BUFF_ACTION(CastShadowmeldAction, "shadowmeld");
    BUFF_ACTION(CastBerserkingAction, "berserking");
    BUFF_ACTION(CastBloodFuryAction, "blood fury");
    BUFF_ACTION(CastStoneformAction, "stoneform");
    BUFF_ACTION(CastPerceptionAction, "perception");

    // spells

#ifndef MANGOSBOT_ZERO
    SPELL_ACTION(CastManaTapAction, "mana tap");
    SPELL_ACTION(CastArcaneTorrentAction, "arcane torrent");
#endif

    class CastWarStompAction : public CastSpellAction
    {
    public:
        CastWarStompAction(PlayerbotAI* ai) : CastSpellAction(ai, "war stomp") {}
    };

    //cc breakers

    BUFF_ACTION(CastWillOfTheForsakenAction, "will of the forsaken");
    BUFF_ACTION_U(CastEscapeArtistAction, "escape artist", !ai->HasAura("stealth", AI_VALUE(Unit*, "self target")));
#ifdef MANGOSBOT_TWO
    SPELL_ACTION(CastEveryManforHimselfAction, "every man for himself");
#endif

    class CastSpellOnEnemyHealerAction : public CastSpellAction
    {
    public:
        CastSpellOnEnemyHealerAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) {}
        
    protected:
        virtual string GetReachActionName() override { return "reach spell"; }
        virtual string GetTargetName() override { return "enemy healer target"; }
        virtual string GetTargetQualifier() override { return GetSpellName(); }
        virtual string getName() override { return GetSpellName() + " on enemy healer"; }
    };

    class CastSnareSpellAction : public CastRangedDebuffSpellAction
    {
    public:
        CastSnareSpellAction(PlayerbotAI* ai, string spell) : CastRangedDebuffSpellAction(ai, spell) {}
        
    protected:
        virtual string GetReachActionName() override { return "reach spell"; }
        virtual string GetTargetName() override { return "snare target"; }
        virtual string GetTargetQualifier() override { return GetSpellName(); }
        virtual string getName() override { return GetSpellName() + " on snare target"; }
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }
    };

    class CastCrowdControlSpellAction : public CastRangedDebuffSpellAction
    {
    public:
        CastCrowdControlSpellAction(PlayerbotAI* ai, string spell) : CastRangedDebuffSpellAction(ai, spell) {}
        
    private:
        virtual string GetReachActionName() override { return "reach spell"; }
        virtual string GetTargetName() override { return "cc target"; }
        virtual string GetTargetQualifier() override { return GetSpellName(); }
        virtual ActionThreatType getThreatType() { return ActionThreatType::ACTION_THREAT_NONE; }
    };

    class CastProtectSpellAction : public CastSpellAction
    {
    public:
        CastProtectSpellAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell) {}
        virtual bool isUseful() override { return CastSpellAction::isUseful() && !ai->HasAura(GetSpellName(), GetTarget()); }

    protected:
        virtual string GetReachActionName() override { return "reach spell"; }
        virtual string GetTargetName() override { return "party member to protect"; }
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }
    };

    class InterruptCurrentSpellAction : public Action
    {
    public:
        InterruptCurrentSpellAction(PlayerbotAI* ai) : Action(ai, "interrupt current spell") {}
        virtual bool isUseful() override;
        virtual bool Execute(Event& event) override;
    };

    //--------------------//
    //   Vehicle Actions  //
    //--------------------//

    class CastVehicleSpellAction : public CastSpellAction
    {
    public:
        CastVehicleSpellAction(PlayerbotAI* ai, string spell) : CastSpellAction(ai, spell)
        {
            range = 120.0f;
            SetSpellName(spell, "vehicle spell id");
        }

        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
        virtual bool isPossible() override;

    protected:
        virtual ActionThreatType getThreatType() override { return ActionThreatType::ACTION_THREAT_NONE; }
        virtual string GetTargetName() override { return "current target"; }
        virtual string GetReachActionName() override { return ""; }
    };

    class CastHurlBoulderAction : public CastVehicleSpellAction
    {
    public:
        CastHurlBoulderAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "hurl boulder") {}
    };

    class CastSteamRushAction : public CastVehicleSpellAction
    {
    public:
        CastSteamRushAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "steam rush") {}
    };

    class CastRamAction : public CastVehicleSpellAction
    {
    public:
        CastRamAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "ram") {}
    };

    class CastNapalmAction : public CastVehicleSpellAction
    {
    public:
        CastNapalmAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "napalm") {}
    };

    class CastFireCannonAction : public CastVehicleSpellAction
    {
    public:
        CastFireCannonAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "fire cannon") {}
    };

    class CastSteamBlastAction : public CastVehicleSpellAction
    {
    public:
        CastSteamBlastAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "steam blast") {}
    };

    class CastIncendiaryRocketAction : public CastVehicleSpellAction
    {
    public:
        CastIncendiaryRocketAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "incendiary rocket") {}
    };

    class CastRocketBlastAction : public CastVehicleSpellAction
    {
    public:
        CastRocketBlastAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "rocket blast") {}
    };

    class CastGlaiveThrowAction : public CastVehicleSpellAction
    {
    public:
        CastGlaiveThrowAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "glaive throw") {}
    };

    class CastBladeSalvoAction : public CastVehicleSpellAction
    {
    public:
        CastBladeSalvoAction(PlayerbotAI* ai) : CastVehicleSpellAction(ai, "blade salvo") {}
    };
}
