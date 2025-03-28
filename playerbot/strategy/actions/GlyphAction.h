#pragma once
#include "playerbot/strategy/Action.h"

namespace ai
{
    class SetGlyphAction : public Action, public Qualified
    {
    public:
        SetGlyphAction(PlayerbotAI* ai, std::string name = "set glyph") : Action(ai, name), Qualified() {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;

        static const ItemPrototype* GetGlyphProtoFromName(std::string glyphName, uint32 classId);
        static uint32 GetGlyphIdFromProto(const ItemPrototype* glyphProto);
        static const ItemPrototype* GetGlyphProtoFromGlyphId(uint32 glyphId, uint32 classId);
        
        static bool isGlyphSlotEnabled(uint8 slot, uint32 level);
        static bool isGlyphAlreadySet(uint32 glyphId, Player* bot);
        static uint8 GetBestSlotForGlyph(uint32 glyphId, Player* bot, uint8 wantedSlot = 99);

        static bool WantsGlyphFromStrategy(uint32 glyphId, Player* bot);
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "set glyph"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This action will apply new glyphs to the bot.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    };    
}