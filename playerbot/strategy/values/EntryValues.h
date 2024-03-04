#pragma once

#include "playerbot/playerbot.h"
#include "../Value.h"

namespace ai
{
    class MCRunesValue : public StringCalculatedValue, public Qualified
    {
    public:
        MCRunesValue(PlayerbotAI* ai, string name = "mc runes") : StringCalculatedValue(ai, name, 1), Qualified() {}

        virtual string Calculate() { return "176951,176952,176953,176954,176955,176956,176957"; };

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "mc runes"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "entry"; }
        virtual string GetHelpDescription()
        {
            return "This value contains the entries of the Molten Core runes.";
        }
        virtual vector<string> GetUsedValues() { return { }; }
#endif 
    };
}
