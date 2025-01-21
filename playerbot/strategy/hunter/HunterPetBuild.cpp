#include "HunterPetBuild.h"

uint16 HunterPetBuild::GetTPCostOfBuild()
{
    int tpCost = 0;
    for (HunterPetBuildSpellEntity spell : spells)
    {
        tpCost += spell.TPCost;
    }

    return tpCost;
}

void HunterPetBuild::InitHunterPetBuildSpellEntityList()
{
#pragma region HunterPetBuildSpellEntities
    // Arcane Resistance
    HunterPetBuildSpellEntity rank1ArcaneResistance = HunterPetBuildSpellEntity(1, 20, 5, 24493, 24495);
    HunterPetBuildSpellEntity rank2ArcaneResistance = HunterPetBuildSpellEntity(2, 30, 15, 24497, 24508);
    HunterPetBuildSpellEntity rank3ArcaneResistance = HunterPetBuildSpellEntity(3, 40, 45, 24500, 24509);
    HunterPetBuildSpellEntity rank4ArcaneResistance = HunterPetBuildSpellEntity(4, 50, 90, 24501, 24510);
    HunterPetBuildSpellEntity rank5ArcaneResistance = HunterPetBuildSpellEntity(5, 60, 105, 27052, 27350); // TBC

    // Avoidance
    HunterPetBuildSpellEntity rank1Avoidance = HunterPetBuildSpellEntity(1, 30, 15, 35694, 35699); // TBC
    HunterPetBuildSpellEntity rank2Avoidance = HunterPetBuildSpellEntity(2, 60, 25, 35698, 35700); // TBC

    // Bite
    HunterPetBuildSpellEntity rank1Bite = HunterPetBuildSpellEntity(1, 1, 1, 17253, 17254);
    HunterPetBuildSpellEntity rank2Bite = HunterPetBuildSpellEntity(2, 8, 4, 17255, 17262);
    HunterPetBuildSpellEntity rank3Bite = HunterPetBuildSpellEntity(3, 16, 7, 17256, 17263);
    HunterPetBuildSpellEntity rank4Bite = HunterPetBuildSpellEntity(4, 24, 10, 17257, 17264);
    HunterPetBuildSpellEntity rank5Bite = HunterPetBuildSpellEntity(5, 32, 13, 17258, 17265);
    HunterPetBuildSpellEntity rank6Bite = HunterPetBuildSpellEntity(6, 40, 17, 17259, 17266);
    HunterPetBuildSpellEntity rank7Bite = HunterPetBuildSpellEntity(7, 48, 21, 17260, 17267);
    HunterPetBuildSpellEntity rank8Bite = HunterPetBuildSpellEntity(8, 56, 25, 17261, 17268);
    HunterPetBuildSpellEntity rank9Bite = HunterPetBuildSpellEntity(9, 64, 29, 27050, 27348); // TBC

    // Charge
    HunterPetBuildSpellEntity rank1Charge = HunterPetBuildSpellEntity(1, 1, 5, 7371, 7370);
    HunterPetBuildSpellEntity rank2Charge = HunterPetBuildSpellEntity(2, 12, 9, 26177, 26184);
    HunterPetBuildSpellEntity rank3Charge = HunterPetBuildSpellEntity(3, 24, 13, 26178, 26185);
    HunterPetBuildSpellEntity rank4Charge = HunterPetBuildSpellEntity(4, 36, 17, 26179, 26186);
    HunterPetBuildSpellEntity rank5Charge = HunterPetBuildSpellEntity(5, 48, 25, 26201, 26202);
    HunterPetBuildSpellEntity rank6Charge = HunterPetBuildSpellEntity(6, 60, 29, 27685, 28343);

    // Claw
    HunterPetBuildSpellEntity rank1Claw = HunterPetBuildSpellEntity(1, 1, 1, 16827, 2980);
    HunterPetBuildSpellEntity rank2Claw = HunterPetBuildSpellEntity(2, 8, 4, 16828, 2981);
    HunterPetBuildSpellEntity rank3Claw = HunterPetBuildSpellEntity(3, 16, 7, 16829, 2982);
    HunterPetBuildSpellEntity rank4Claw = HunterPetBuildSpellEntity(4, 24, 10, 16830, 3667);
    HunterPetBuildSpellEntity rank5Claw = HunterPetBuildSpellEntity(5, 32, 13, 16831, 2975);
    HunterPetBuildSpellEntity rank6Claw = HunterPetBuildSpellEntity(6, 40, 17, 16832, 2976);
    HunterPetBuildSpellEntity rank7Claw = HunterPetBuildSpellEntity(7, 48, 21, 3010, 2977);
    HunterPetBuildSpellEntity rank8Claw = HunterPetBuildSpellEntity(8, 56, 25, 3009, 3666);
    HunterPetBuildSpellEntity rank9Claw = HunterPetBuildSpellEntity(9, 64, 29, 27049, 27347); // TBC

    // Cobra Reflexes
    HunterPetBuildSpellEntity rank1CobraReflexes = HunterPetBuildSpellEntity(1, 30, 15, 25076, 25077); // TBC

    // Cower
    HunterPetBuildSpellEntity rank1Cower = HunterPetBuildSpellEntity(1, 5, 8, 1742, 1747);
    HunterPetBuildSpellEntity rank2Cower = HunterPetBuildSpellEntity(2, 15, 10, 1753, 1748);
    HunterPetBuildSpellEntity rank3Cower = HunterPetBuildSpellEntity(3, 25, 12, 1754, 1749);
    HunterPetBuildSpellEntity rank4Cower = HunterPetBuildSpellEntity(4, 35, 14, 1755, 1750);
    HunterPetBuildSpellEntity rank5Cower = HunterPetBuildSpellEntity(5, 45, 16, 1756, 1751);
    HunterPetBuildSpellEntity rank6Cower = HunterPetBuildSpellEntity(6, 55, 18, 16997, 16998);
    HunterPetBuildSpellEntity rank7Cower = HunterPetBuildSpellEntity(7, 65, 21, 27048, 27346); // TBC

    // Dash
    HunterPetBuildSpellEntity rank1Dash = HunterPetBuildSpellEntity(1, 30, 15, 23099, 23100);
    HunterPetBuildSpellEntity rank2Dash = HunterPetBuildSpellEntity(2, 40, 20, 23109, 23111);
    HunterPetBuildSpellEntity rank3Dash = HunterPetBuildSpellEntity(3, 50, 25, 23110, 23112);

    // Dive
    HunterPetBuildSpellEntity rank1Dive = HunterPetBuildSpellEntity(1, 30, 15, 23145, 23146);
    HunterPetBuildSpellEntity rank2Dive = HunterPetBuildSpellEntity(2, 40, 20, 23147, 23149);
    HunterPetBuildSpellEntity rank3Dive = HunterPetBuildSpellEntity(3, 50, 25, 23148, 23150);

    // Fire Breath
    HunterPetBuildSpellEntity rank1FireBreath = HunterPetBuildSpellEntity(1, 1, 5, 34889, 34890); // TBC
    HunterPetBuildSpellEntity rank2FireBreath = HunterPetBuildSpellEntity(2, 60, 25, 35323, 35324); // TBC

    // Fire Resistance
    HunterPetBuildSpellEntity rank1FireResistance = HunterPetBuildSpellEntity(1, 20, 5, 23992, 24440);
    HunterPetBuildSpellEntity rank2FireResistance = HunterPetBuildSpellEntity(2, 30, 15, 24439, 24441);
    HunterPetBuildSpellEntity rank3FireResistance = HunterPetBuildSpellEntity(3, 40, 45, 24444, 24463);
    HunterPetBuildSpellEntity rank4FireResistance = HunterPetBuildSpellEntity(4, 50, 90, 24445, 24464);
    HunterPetBuildSpellEntity rank5FireResistance = HunterPetBuildSpellEntity(5, 60, 105, 27053, 27351); // TBC

    // Frost Resistance
    HunterPetBuildSpellEntity rank1FrostResistance = HunterPetBuildSpellEntity(1, 20, 5, 24446, 24475);
    HunterPetBuildSpellEntity rank2FrostResistance = HunterPetBuildSpellEntity(2, 30, 15, 24447, 24476);
    HunterPetBuildSpellEntity rank3FrostResistance = HunterPetBuildSpellEntity(3, 40, 45, 24448, 24477);
    HunterPetBuildSpellEntity rank4FrostResistance = HunterPetBuildSpellEntity(4, 50, 90, 24449, 24478);
    HunterPetBuildSpellEntity rank5FrostResistance = HunterPetBuildSpellEntity(5, 60, 105, 27054, 27352); // TBC

    // Furious Howl
    HunterPetBuildSpellEntity rank1FuriousHowl = HunterPetBuildSpellEntity(1, 20, 5, 24604, 24609);
    HunterPetBuildSpellEntity rank2FuriousHowl = HunterPetBuildSpellEntity(2, 30, 15, 24605, 24608);
    HunterPetBuildSpellEntity rank3FuriousHowl = HunterPetBuildSpellEntity(3, 40, 45, 24603, 24607);
    HunterPetBuildSpellEntity rank4FuriousHowl = HunterPetBuildSpellEntity(4, 50, 90, 24599, 24599);

    // Gore
    HunterPetBuildSpellEntity rank1Gore = HunterPetBuildSpellEntity(1, 1, 1, 35290, 35299); // TBC
    HunterPetBuildSpellEntity rank2Gore = HunterPetBuildSpellEntity(2, 8, 4, 35291, 35300); // TBC
    HunterPetBuildSpellEntity rank3Gore = HunterPetBuildSpellEntity(3, 16, 7, 35292, 35302); // TBC
    HunterPetBuildSpellEntity rank4Gore = HunterPetBuildSpellEntity(4, 24, 10, 35293, 35303); // TBC
    HunterPetBuildSpellEntity rank5Gore = HunterPetBuildSpellEntity(5, 32, 13, 35294, 35304); // TBC
    HunterPetBuildSpellEntity rank6Gore = HunterPetBuildSpellEntity(6, 40, 17, 35295, 35305); // TBC
    HunterPetBuildSpellEntity rank7Gore = HunterPetBuildSpellEntity(7, 48, 21, 35296, 35306); // TBC
    HunterPetBuildSpellEntity rank8Gore = HunterPetBuildSpellEntity(8, 56, 25, 35297, 35307); // TBC
    HunterPetBuildSpellEntity rank9Gore = HunterPetBuildSpellEntity(9, 63, 29, 35298, 35308); // TBC

    // Great Stamina
    HunterPetBuildSpellEntity rank1GreatStamina = HunterPetBuildSpellEntity(1, 10, 5, 4187, 4195);
    HunterPetBuildSpellEntity rank2GreatStamina = HunterPetBuildSpellEntity(2, 12, 10, 4188, 4196);
    HunterPetBuildSpellEntity rank3GreatStamina = HunterPetBuildSpellEntity(3, 18, 15, 4189, 4197);
    HunterPetBuildSpellEntity rank4GreatStamina = HunterPetBuildSpellEntity(4, 24, 25, 4190, 4198);
    HunterPetBuildSpellEntity rank5GreatStamina = HunterPetBuildSpellEntity(5, 30, 50, 4191, 4199);
    HunterPetBuildSpellEntity rank6GreatStamina = HunterPetBuildSpellEntity(6, 36, 75, 4192, 4200);
    HunterPetBuildSpellEntity rank7GreatStamina = HunterPetBuildSpellEntity(7, 42, 100, 4193, 4201);
    HunterPetBuildSpellEntity rank8GreatStamina = HunterPetBuildSpellEntity(8, 48, 125, 4194, 4202);
    HunterPetBuildSpellEntity rank9GreatStamina = HunterPetBuildSpellEntity(9, 54, 150, 5041, 5048);
    HunterPetBuildSpellEntity rank10GreatStamina = HunterPetBuildSpellEntity(10, 60, 185, 5042, 5049);
    HunterPetBuildSpellEntity rank11GreatStamina = HunterPetBuildSpellEntity(11, 70, 215, 27062, 27364); // TBC

    // Growl
    HunterPetBuildSpellEntity rank1Growl = HunterPetBuildSpellEntity(1, 1, 0, 2649, 1853);
    HunterPetBuildSpellEntity rank2Growl = HunterPetBuildSpellEntity(2, 10, 0, 14916, 14922);
    HunterPetBuildSpellEntity rank3Growl = HunterPetBuildSpellEntity(3, 20, 0, 14917, 14923);
    HunterPetBuildSpellEntity rank4Growl = HunterPetBuildSpellEntity(4, 30, 0, 14918, 14924);
    HunterPetBuildSpellEntity rank5Growl = HunterPetBuildSpellEntity(5, 40, 0, 14919, 14925);
    HunterPetBuildSpellEntity rank6Growl = HunterPetBuildSpellEntity(6, 50, 0, 14920, 14926);
    HunterPetBuildSpellEntity rank7Growl = HunterPetBuildSpellEntity(7, 60, 0, 14921, 14927);
    HunterPetBuildSpellEntity rank8Growl = HunterPetBuildSpellEntity(8, 70, 0, 27047, 27344); // TBC

    // Lightning Breath
    HunterPetBuildSpellEntity rank1LightningBreath = HunterPetBuildSpellEntity(1, 1, 1, 24844, 24845);  // TBC
    HunterPetBuildSpellEntity rank2LightningBreath = HunterPetBuildSpellEntity(2, 12, 5, 25008, 25013);
    HunterPetBuildSpellEntity rank3LightningBreath = HunterPetBuildSpellEntity(3, 24, 10, 25009, 25014);
    HunterPetBuildSpellEntity rank4LightningBreath = HunterPetBuildSpellEntity(4, 36, 15, 25010, 25015);
    HunterPetBuildSpellEntity rank5LightningBreath = HunterPetBuildSpellEntity(5, 48, 20, 25011, 25016);
    HunterPetBuildSpellEntity rank6LightningBreath = HunterPetBuildSpellEntity(6, 60, 25, 25012, 25017);

    // Natural Armor
    HunterPetBuildSpellEntity rank1NaturalArmor = HunterPetBuildSpellEntity(1, 10, 1, 24545, 24547);
    HunterPetBuildSpellEntity rank2NaturalArmor = HunterPetBuildSpellEntity(2, 12, 5, 24549, 24556);
    HunterPetBuildSpellEntity rank3NaturalArmor = HunterPetBuildSpellEntity(3, 18, 10, 24550, 24557);
    HunterPetBuildSpellEntity rank4NaturalArmor = HunterPetBuildSpellEntity(4, 24, 15, 24551, 24558);
    HunterPetBuildSpellEntity rank5NaturalArmor = HunterPetBuildSpellEntity(5, 30, 25, 24552, 24559);
    HunterPetBuildSpellEntity rank6NaturalArmor = HunterPetBuildSpellEntity(6, 36, 50, 24553, 24560);
    HunterPetBuildSpellEntity rank7NaturalArmor = HunterPetBuildSpellEntity(7, 42, 75, 24554, 24561);
    HunterPetBuildSpellEntity rank8NaturalArmor = HunterPetBuildSpellEntity(8, 48, 100, 24555, 24562);
    HunterPetBuildSpellEntity rank9NaturalArmor = HunterPetBuildSpellEntity(9, 54, 125, 24629, 24631);
    HunterPetBuildSpellEntity rank10NaturalArmor = HunterPetBuildSpellEntity(10, 60, 150, 24630, 24632);
    HunterPetBuildSpellEntity rank11NaturalArmor = HunterPetBuildSpellEntity(11, 70, 175, 27061, 27362); // TBC

    // Nature Resistance
    HunterPetBuildSpellEntity rank1NatureResistance = HunterPetBuildSpellEntity(1, 20, 5, 24494, 24494);
    HunterPetBuildSpellEntity rank2NatureResistance = HunterPetBuildSpellEntity(2, 30, 15, 24511, 24511);
    HunterPetBuildSpellEntity rank3NatureResistance = HunterPetBuildSpellEntity(3, 40, 45, 24512, 24512);
    HunterPetBuildSpellEntity rank4NatureResistance = HunterPetBuildSpellEntity(4, 50, 90, 24513, 24513);
    HunterPetBuildSpellEntity rank5NatureResistance = HunterPetBuildSpellEntity(5, 60, 105, 27055, 27354); // TBC

    // Poison Spit
    HunterPetBuildSpellEntity rank1PoisonSpit = HunterPetBuildSpellEntity(1, 15, 5, 35387, 35388); // TBC
    HunterPetBuildSpellEntity rank2PoisonSpit = HunterPetBuildSpellEntity(2, 45, 20, 35389, 35390); // TBC
    HunterPetBuildSpellEntity rank3PoisonSpit = HunterPetBuildSpellEntity(3, 60, 25, 35392, 35391); // TBC

    // Prowl
    HunterPetBuildSpellEntity rank1Prowl = HunterPetBuildSpellEntity(1, 30, 15, 24450, 24451);
    HunterPetBuildSpellEntity rank2Prowl = HunterPetBuildSpellEntity(2, 40, 15, 24452, 24454);
    HunterPetBuildSpellEntity rank3Prowl = HunterPetBuildSpellEntity(3, 50, 45, 24453, 24455);

    // Scorpid Poison
    HunterPetBuildSpellEntity rank1ScorpidPoison = HunterPetBuildSpellEntity(1, 8, 10, 24583, 24584);
    HunterPetBuildSpellEntity rank2ScorpidPoison = HunterPetBuildSpellEntity(2, 24, 15, 24586, 24588);
    HunterPetBuildSpellEntity rank3ScorpidPoison = HunterPetBuildSpellEntity(3, 40, 20, 24587, 24589);
    HunterPetBuildSpellEntity rank4ScorpidPoison = HunterPetBuildSpellEntity(4, 56, 25, 24640, 24641);
    HunterPetBuildSpellEntity rank5ScorpidPoison = HunterPetBuildSpellEntity(5, 64, 29, 27060, 27361); // TBC

    // Screech
    HunterPetBuildSpellEntity rank1Screech = HunterPetBuildSpellEntity(1, 8, 10, 24423, 24424);
    HunterPetBuildSpellEntity rank2Screech = HunterPetBuildSpellEntity(2, 24, 15, 24577, 24580);
    HunterPetBuildSpellEntity rank3Screech = HunterPetBuildSpellEntity(3, 40, 20, 24578, 24581);
    HunterPetBuildSpellEntity rank4Screech = HunterPetBuildSpellEntity(4, 56, 25, 24579, 24582);
    HunterPetBuildSpellEntity rank5Screech = HunterPetBuildSpellEntity(5, 64, 29, 27051, 27349); // TBC

    // Shadow Resistance
    HunterPetBuildSpellEntity rank1ShadowResistance = HunterPetBuildSpellEntity(1, 20, 5, 24488, 24490);
    HunterPetBuildSpellEntity rank2ShadowResistance = HunterPetBuildSpellEntity(2, 30, 15, 24505, 24514);
    HunterPetBuildSpellEntity rank3ShadowResistance = HunterPetBuildSpellEntity(3, 40, 45, 24506, 24515);
    HunterPetBuildSpellEntity rank4ShadowResistance = HunterPetBuildSpellEntity(4, 50, 90, 24507, 24516);
    HunterPetBuildSpellEntity rank5ShadowResistance = HunterPetBuildSpellEntity(5, 60, 105, 27056, 27353); // TBC

    // Shell Shield
    HunterPetBuildSpellEntity rank1ShellShield = HunterPetBuildSpellEntity(1, 20, 15, 26064, 26065);

    // Thunderstomp
    HunterPetBuildSpellEntity rank1Thunderstomp = HunterPetBuildSpellEntity(1, 30, 15, 26090, 26094);
    HunterPetBuildSpellEntity rank2Thunderstomp = HunterPetBuildSpellEntity(2, 40, 20, 26187, 26189);
    HunterPetBuildSpellEntity rank3Thunderstomp = HunterPetBuildSpellEntity(3, 50, 25, 26188, 26190);

    // Warp
    HunterPetBuildSpellEntity rank1Warp = HunterPetBuildSpellEntity(1, 60, 1, 35346, 35348); // TBC
#pragma endregion

    // Position in buildLink   Rank of Spell   Spell Info
    spellRankEntityMapping =
    {
        {
            0,
            {
                {1,rank1ArcaneResistance},
                {2,rank2ArcaneResistance},
                {3,rank3ArcaneResistance},
                {4,rank4ArcaneResistance},
#ifdef MANGOSBOT_ONE
                {5,rank5ArcaneResistance}
#endif
            }
        },
        {
            1,
            {
                {1,rank1Bite},
                {2,rank2Bite},
                {3,rank3Bite},
                {4,rank4Bite},
                {5,rank5Bite},
                {6,rank6Bite},
                {7,rank7Bite},
                {8,rank8Bite},
#ifdef MANGOSBOT_ONE
                {9,rank9Bite},
#endif
            },
        },
        {
            2,
            {
                {1,rank1Charge},
                {2,rank2Charge},
                {3,rank3Charge},
                {4,rank4Charge},
                {5,rank5Charge},
                {6,rank6Charge},
            }
        },
        {
            4,
            {
                {1,rank1Claw},
                {2,rank2Claw},
                {3,rank3Claw},
                {4,rank4Claw},
                {5,rank5Claw},
                {6,rank6Claw},
                {7,rank7Claw},
                {8,rank8Claw},
#ifdef MANGOSBOT_ONE
                {9,rank9Claw}
#endif
            }
        },
        {
            5,
            {
                {1,rank1Cower},
                {2,rank2Cower},
                {3,rank3Cower},
                {4,rank4Cower},
                {5,rank5Cower},
                {6,rank6Cower},
#ifdef MANGOSBOT_ONE
                {7,rank7Cower}
#endif
            }
        },
        {
            6,
            {
                {1,rank1Dash},
                {2,rank2Dash},
                {3,rank3Dash},
            }
        },
        {
            8,
            {
                {1,rank1Dive},
                {2,rank2Dive},
                {3,rank3Dive},
            }
        },
        {
            9,
            {
                {1,rank1FireResistance},
                {2,rank2FireResistance},
                {3,rank3FireResistance},
                {4,rank4FireResistance},
#ifdef MANGOSBOT_ONE
                {5,rank5FireResistance}
#endif
            }
        },
        {
            10,
            {
                {1,rank1FrostResistance},
                {2,rank2FrostResistance},
                {3,rank3FrostResistance},
                {4,rank4FrostResistance},
#ifdef MANGOSBOT_ONE
                {5,rank5FrostResistance}
#endif   
            }
        },
        {
            12,
            {
                {1,rank1FuriousHowl},
                {2,rank2FuriousHowl},
                {3,rank3FuriousHowl},
                {4,rank4FuriousHowl}
            }
        },
        {
            13,
            {
                {1,rank1GreatStamina},
                {2,rank2GreatStamina},
                {3,rank3GreatStamina},
                {4,rank4GreatStamina},
                {5,rank5GreatStamina},
                {6,rank6GreatStamina},
                {7,rank7GreatStamina},
                {8,rank8GreatStamina},
                {9,rank9GreatStamina},
                {10,rank10GreatStamina},
#ifdef MANGOSBOT_ONE
                {11,rank11GreatStamina}
#endif
            }
        },
        {
            14,
            {
                {1,rank1Growl},
                {2,rank2Growl},
                {3,rank3Growl},
                {4,rank4Growl},
                {5,rank5Growl},
                {6,rank6Growl},
                {7,rank7Growl},
#ifdef MANGOSBOT_ONE
                {8,rank8Growl},
#endif
            }
        },
        {
            16,
            {
#ifdef MANGOSBOT_ONE
                {1,rank1LightningBreath},
#endif
                {2,rank2LightningBreath},
                {3,rank3LightningBreath},
                {4,rank4LightningBreath},
                {5,rank5LightningBreath},
                {6,rank6LightningBreath}
            }
        },
        {
            17,
            {
                {1,rank1NaturalArmor},
                {2,rank2NaturalArmor},
                {3,rank3NaturalArmor},
                {4,rank4NaturalArmor},
                {5,rank5NaturalArmor},
                {6,rank6NaturalArmor},
                {7,rank7NaturalArmor},
                {8,rank8NaturalArmor},
                {9,rank9NaturalArmor},
                {10,rank10NaturalArmor},
#ifdef MANGOSBOT_ONE
                {11,rank11NaturalArmor}
#endif
            }
        },
        {
            18,
            {
                {1,rank1NatureResistance},
                {2,rank2NatureResistance},
                {3,rank3NatureResistance},
                {4,rank4NatureResistance},
#ifdef MANGOSBOT_ONE
                {5,rank5NatureResistance}
#endif
            }
        },
        {
            20,
            {
                {1,rank1Prowl},
                {2,rank2Prowl},
                {3,rank3Prowl}
            }
        },
        {
            21,
            {
                {1,rank1ScorpidPoison},
                {2,rank2ScorpidPoison},
                {3,rank3ScorpidPoison},
                {4,rank4ScorpidPoison},
#ifdef MANGOSBOT_ONE
                {5,rank5ScorpidPoison}
#endif
            }
        },
        {
            22,
            {
                {1,rank1Screech},
                {2,rank2Screech},
                {3,rank3Screech},
                {4,rank4Screech},
#ifdef MANGOSBOT_ONE
                {5,rank5Screech}
#endif
            }
        },
        {
            24,
            {
                {1,rank1ShadowResistance},
                {2,rank2ShadowResistance},
                {3,rank3ShadowResistance},
                {4,rank4ShadowResistance},
#ifdef MANGOSBOT_ONE
                {5,rank5ShadowResistance}
#endif
            }
        },
        {
            25,
            {
                {1,rank1ShellShield}
            }
        },
        {
            26,
            {
                {1,rank1Thunderstomp},
                {2,rank2Thunderstomp},
                {3,rank3Thunderstomp},
            }
        }
#ifdef MANOGSBOT_ONE
        {
            28,
            {
                {1,rank1Avoidance},
                {2,rank2Avoidance},
            }
        },
        {
            29,
            {
                {1,rank1CobraReflexes}
            }
        },
        {
            30,
            {
                {1,rank1FireBreath},
                {2,rank2FireBreath}
            }
        },
        {
            32,
            {
                {1,rank1Gore},
                {2,rank2Gore},
                {3,rank3Gore},
                {4,rank4Gore},
                {5,rank5Gore},
                {6,rank6Gore},
                {7,rank7Gore},
                {8,rank8Gore},
                {9,rank9Gore},
            }
        },
        {
            33,
            {
                {1,rank1PoisonSpit},
                {2,rank2PoisonSpit},
                {3,rank3PoisonSpit},
            }
        },
        {
            34,
            {
                {1,rank1Warp}
            }
        }
#endif
    };
}

HunterPetBuild::HunterPetBuild(uint32 familyId)
{
    InitHunterPetBuildSpellEntityList();
    GetSpells(familyId);
}

HunterPetBuild::HunterPetBuild(HunterPetBuild* build, std::string buildLink)
{
    InitHunterPetBuildSpellEntityList();
    spells = build->spells;
    ReadSpells(buildLink);
}

// Checks if the spells listed are valid for the level. As TP varies due to level 
// and loyalty of a given pet that check will be done separately.
bool HunterPetBuild::CheckBuild(uint32 level, std::ostringstream* out)
{
    for (auto& entry : spells)
    {
        if (entry.Level > level)
        {
            *out << "Build is for a higher level.";
            return false;
        }
    }
    return true;
}

bool HunterPetBuild::CheckBuildLink(std::string buildLink, std::ostringstream* out)
{
#ifdef MANGOSBOT_ZERO
    int maxFamilyBuilds = 27;
#endif
#ifdef MANGOSBOT_ONE
    int maxFamilyBuilds = 34;
#endif
    for (int ii = 0; ii < maxFamilyBuilds; ii++)
    {
        // if 4th digit skip if '-' other wise invalid format so reject build link.
        if ((ii + 1) % 4 == 0)
            if (buildLink[ii] == '-')
                continue;
            else
                return false;
        std::string stringValidRanks = "0123456789AB";
        if (stringValidRanks.find(buildLink[ii]) == std::string::npos)
            return false;

        uint8 rank;
        std::stringstream ss;
        ss << buildLink[ii];
        ss >> std::hex >> rank;

        if (spellRankEntityMapping[ii].find(rank) == spellRankEntityMapping[ii].end())
            return false;
    }
}

void HunterPetBuild::ReadSpells(std::string buildLink)
{

}

void HunterPetBuild::GetSpells(uint32 familyId)
{
}
