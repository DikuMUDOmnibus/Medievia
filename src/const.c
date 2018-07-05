/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/


#include <stdio.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "limits.h"
#include "db.h"


char *god_list[] = {

"Vryce",
"Io",
"Pug",
"Kathy",
"Avi",
"Starblade",
"Zemci",
"Alotta",
"Lucifuge",
"Megan",
"Ugadal",
"Vehn",
"Machine",
"Sultress",
"Raster",
"Shalafi",
"Dragos",
"Achad",
"Semele",
"Firm",
"Asskicker",
"Lena",
"Stu",
"Wakim",
"Stormchylde",
"Drakoth",
"Anubis",
"Yugo",
"Columnus",
"Moment",
"Warlok",
"Vanyel",
"Zimimar",
"Baphomet",
"Soleil",
"$"
};
char *deck[]={
"2c",
"2h",
"2d",
"2s",
"3c",
"3h",
"3d",
"3s",
"4c",
"4h",
"4d",
"4s",
"5c",
"5h",
"5d",
"5s",
"6c",
"6h",
"6d",
"6s",
"7c",
"7h",
"7d",
"7s",
"8c",
"8h",
"8d",
"8s",
"9c",
"9h",
"9d",
"9s",
"10c",
"10h",
"10d",
"10s",
"Jc",
"Jh",
"Jd",
"Js",
"Qc",
"Qh",
"Qd",
"Qs",
"Kc",
"Kh",
"Kd",
"Ks",
"Ac",
"Ah",
"Ad",
"As",
};

const int slot_rooms[]={
1859,1860,1861,1867,1866,1865,1874,1875,1876,1868,1872,1873,1864,1863,1862,
1856,1857,1858,3155,-1
};

/* Each spell and skill must have a wear off message -Vryce */

const char *spell_wear_off_msg[] =
{
  "RESERVED DB.C",                                             /* 0 */
  "You feel less protected.",
  "!Teleport!",
  "You feel less righteous.",
  "You feel a cloak of blindness disolve.",
  "!Burning Hands!",                                           /* 5 */
  "!Call Lightning",
  "You feel more self-confident.",
  "!Chill Touch!",
  "!Clone!",
  "!Color Spray!",                                             /* 10 */
  "!Control Weather!",
  "!Create Food!",
  "!Create Water!",
  "!Cure Blind!",
  "!Cure Critic!",                                             /* 15 */
  "!Cure Light!",
  "You feel better.",
  "You sense the red in your vision disappear.",
  "The detect invisible wears off.",
  "The detect magic wears off.",                               /* 20 */
  "The detect poison wears off.",
  "!Dispel Evil!",
  "!Earthquake!",
  "!Enchant Weapon!",
  "!Energy Drain!",                                            /* 25 */
  "!Fireball!",
  "!Harm!",
  "!Heal",
  "You feel yourself exposed.",
  "!Lightning Bolt!",                                          /* 30 */
  "!Locate object!",
  "!Magic Missile!",
  "You feel less sick.",
  "You feel more vulnerable.",
  "!Remove Curse!",                                            /* 35 */
  "The white aura around your body fades.",
  "!Shocking Grasp!",
  "You feel less tired.",
  "You feel weaker.",
  "!Summon!",                                                  /* 40 */
  "!Ventriloquate!",
  "!Word of Recall!",
  "!Remove Poison!",
  "You feel less aware of your surroundings.",
  "",  /* NO MESSAGE FOR SNEAK */                              /* 45 */
  "!Hide!",    /* NOTE:  I moved #45- 52 to another place -Kahn */
  "!Steal!",   /* These spaces serve only as fillers            */
  "!Backstab!",
  "!Pick Lock!",
  "!Kick!",                                                    /* 50 */
  "!Bash!",
  "!Rescue!",
  "!Identify!",
  "!Animate Dead!",
  "You are not as fearful now.",                               /* 55 */
  "You slowly float to the ground.",
  "Your light slowly fizzes into nothing.",
  "You can no longer see into a persons aura.",
  "!Dispel Magic!",
  "!Conjure Elemental!",                                       /* 60 */
  "!Cure Serious!",
  "!Cause Light!",
  "!Cause Critical!",
  "!Cause Serious!",
  "!Flamestrike!",                                             /* 65 */
  "Your skin does not feel as hard as it used to.",
  "Your force shield shimmers then fades away.",
  "You feel stronger.",
  "!Mass Invisibility!",  /* Uses the invisibility message */
  "!Acid Blast!",                                              /* 70 */
  "!Gate!",
  "The pink aura around you fades away.",
  "!Faerie Fog!",
  "!Drown!",
  "!Demon Fire!",                                              /* 75 */
  "!Turn Undead!",
  "You can no longer see in the dark.",
  "!Sandstorm!",
  "!Hands Of Wind!",
  "You feel better now.",                                      /* 80 */
  "!Refresh!",
  "Your shield of fire slowly dissipates....and blinks out.",
  "!transport!",
  "!mass levitation!",
  "!sense death!",					       /* 85 */
  "Your lungs feel like they are on fire as the breathe water spell wears off.",
  "!shield room!",
  "!chain lighting!",
  "Your purple mapping aura fades and leaves you exhausted",
  "!farsight!",							/* 90 */
  "!ethereal!",
  "!shockwave!",
  "!scribe!",
  "!bloodbath!",
  "!resurrect!",						/* 95 */
  "!UNUSED!",
  "!wizard eye!",
  "!hammer of faith!",
  "You feel the protective veil of darkness vanish.",
  "!frost shards!"
};


const int rev_dir[] =
{
    2,
    3,
    0,
    1,
    5,
    4
};

const int movement_loss[]=
{
    1,  /* Inside     */
    2,  /* City       */
    2,  /* Field      */
    3,  /* Forest     */
    4,  /* Hills      */
    6,  /* Mountains  */
    4,  /* Water_Swim */
    1,  /* Water No-Swim */
    7,  /* Desert     */
    5,  /* Underwater */
    1,  /* Air */
    4,  /* Swamp */
    3,  /* Jungle */
    4,  /* Arctic */
    1,  /* MANA */
};

const char *dirs[] =
{
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "\n"
};

const char *weekdays[7] =
{
    "the Day of Spirituality",
    "the Day of Justice",
    "the Day of Honor",
    "the Day of Compassion",
    "the Day of Sacrifice",
    "the Day of Valor",
    "the Day of Honesty"
};

const char *month_name[17] =
{
    "Month of Winter Night",           /* 0 */
    "Month of Trellor's Founding",
    "Month of the Frost Giant",
    "Month of Wondrous Creation",
    "Month of the Grand Struggle",
    "Month of Spring Dawning",
    "Month of Medievia's Birth",
    "Month of Futility",
    "Month of the Drolem",
    "Month of Summer Flame",
    "Month of Sin",
    "Month of the Scorpion",
    "Month of Vecna's Terror",
    "Month of Autumn Twilight",
    "Month of Dark Sorcery",
    "Month of the Blood Moon",
    "Month of the Great Evil"
};

const int sharp[] = {
   0,
   0,
   0,
   1,    /* Slashing */
   0,
   0,
   0,
   0,    /* Bludgeon */
   0,
   0,
   0,
   0
};  /* Pierce   */

const char *where[] =
{
    "<used as light>      ",
    "<worn on finger>     ",
    "<worn on finger>     ",
    "<worn around neck>   ",
    "<worn around neck>   ",
    "<worn on body>       ",
    "<worn on head>       ",
    "<worn on legs>       ",
    "<worn on feet>       ",
    "<worn on hands>      ",
    "<worn on arms>       ",
    "<worn as shield>     ",
    "<worn about body>    ",
    "<worn about waist>   ",
    "<worn around wrist>  ",
    "<worn around wrist>  ",
    "<wielded>            ",
    "<held>               "
};

const char *drinks[]=
{
    "water",
    "beer",
    "wine",
    "ale",
    "dark ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local speciality",
    "slime mold juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt water",
    "pepsi"
};

const char *drinknames[]=
{
    "water",
    "beer",
    "wine",
    "ale",
    "ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local",
    "juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt",
    "pepsi"
};

/* this array says from what level to what level mobs to load for this 
   distance in the holoworld from medievia. */
   
const int giaHoloMobPlacementByLevel[21][2]=
{
	{1,6}, /*Miles from medievia    0 to 49   */	
	{5,10}, /*Miles from medievia   50 to 99   */	
	{9,12}, /*Miles from medievia  100 to 149  */	
	{11,14}, /*Miles from medievia  150 to 199  */	
	{13,16}, /*Miles from medievia  200 to 249  */	
	{15,18}, /*Miles from medievia  250 to 299  */	
	{17,20}, /*Miles from medievia  300 to 349  */	
	{19,22}, /*Miles from medievia  350 to 399  */	
	{19,23}, /*Miles from medievia  400 to 449  */	
	{20,24}, /*Miles from medievia  450 to 499  */	
	{21,24}, /*Miles from medievia  500 to 549  */	
	{22,25}, /*Miles from medievia  550 to 599  */	
	{22,26}, /*Miles from medievia  600 to 649  */	
	{22,26}, /*Miles from medievia  650 to 699  */	
	{23,27}, /*Miles from medievia  700 to 749  */	
	{24,28}, /*Miles from medievia  750 to 799  */	
	{25,29}, /*Miles from medievia  800 to 849  */	
	{26,30}, /*Miles from medievia  850 to 899  */	
	{28,32}, /*Miles from medievia  900 to 949  */	
	{30,34}, /*Miles from medievia  950 to 999  */	
	{35,40} /*Miles from medievia  1000 to 1049 */	
};

const int drink_aff[][3] =
{
    { 0,1,10 },  /* Water    */
    { 3,2,5 },   /* beer     */
    { 5,2,5 },   /* wine     */
    { 2,2,5 },   /* ale      */
    { 1,2,5 },   /* ale      */
    { 6,1,4 },   /* Whiskey  */
    { 0,1,8 },   /* lemonade */
    { 10,0,0 },  /* firebr   */
    { 3,3,3 },   /* local    */
    { 0,4,-8 },  /* juice    */
    { 0,3,6 },
    { 0,1,6 },
    { 0,1,6 },
    { 0,2,-1 },
    { 0,1,-2 },
    { 0,1,5 }
};

const char *color_liquid[]=
{
    "clear",
    "brown",
    "clear",
    "brown",
    "dark",
    "golden",
    "red",
    "green",
    "clear",
    "light green",
    "white",
    "brown",
    "black",
    "red",
    "clear",
    "black"
};

const char *fullness[] =
{
    "less than half ",
    "about half ",
    "more than half ",
    ""
};

#define	K	* 1000
#define M	K K
const int exp_table[36+1] =
{
	    0,
	    1  ,   450  ,   900  ,     4 K,     8 K,	/*  5 */
	   16 K,    32 K,    64 K,   128 K,   256 K,	/* 10 */
	  640 K,  1248 K,  2600 K,  4800 K,  7000 K,	/* 15 */
	10000 K,    13 M,    18 M,    28 M ,   56 M,	/* 20 */
	   80 M,   130 M,   200 M,   280 M,   380 M,	/* 25 */
	  520 M,   750 M,   880 M,  1100 M,  1300 M,	/* 30 */
	 1500 M,  1550 M,  1600 M,  1900 M,  2000 M,	/* 35 */
	0x7FFFFFFF
};
#undef	M
#undef	K

const char *title_table[4][36][2] =
{
    {
	{ "the Man",			"the Woman"			},

	{ "the Apprentice of Magic",	"the Apprentice of Magic"	},
	{ "the Spell Student",		"the Spell Student"		},
	{ "the Scholar of Magic",	"the Scholar of Magic"		},
	{ "the Delver in Spells",	"the Delveress in Spells"	},
	{ "the Medium of Magic",	"the Medium of Magic"		},

	{ "the Scribe of Magic",	"the Scribess of Magic"		},
	{ "the Seer",			"the Seeress"			},
	{ "the Sage",			"the Sage"			},
	{ "the Illusionist",		"the Illusionist"		},
	{ "the Abjurer",		"the Abjuress"			},

	{ "the Invoker",		"the Invoker"			},
	{ "the Enchanter",		"the Enchantress"		},
	{ "the Conjurer",		"the Conjuress"			},
	{ "the Magician",		"the Witch"			},
	{ "the Creator",		"the Creator"			},

	{ "the Savant",			"the Savant"			},
	{ "the Magus",			"the Craftess"			},
	{ "the Wizard",			"the Wizard"			},
	{ "the Warlock",			"the War Witch"		},
	{ "the Sorcerer",			"the Sorceress"		},

	{ "the Elder Sorcerer",		"the Elder Sorceress"		},
	{ "the Grand Sorcerer",		"the Grand Sorceress"		},
	{ "the Great Sorcerer",		"the Great Sorceress"		},
	{ "the Golem Maker",		"the Golem Maker"		},
	{ "the Greater Golem Maker",	"the Greater Golem Maker"	},

	{ "the Demon Summoner",		"the Demon Summoner"		},
	{ "the Greater Demon Summoner",	"the Greater Demon Summoner"	},
	{ "the Dragon Charmer",		"the Dragon Charmer"		},
	{ "the Greater Dragon Charmer",	"the Greater Dragon Charmer"	},
	{ "the Master of all Magic",	"the Master of all Magic"	},

	{ "the Mage Hero",		"the Mage Heroine"		},
	{ "the Immortal Warlock",	"the Immortal Enchantress"	},
	{ "the Deity of Magic",		"the Deity of Magic"		},
	{ "the Supremity of Magic",	"the Supremity of Magic"	},
	{ "the Implementor",		"the Implementress"		}
    },

    {
	{ "the Man",			"the Woman"			},

	{ "the Believer",		"the Believer"			},
	{ "the Attendant",		"the Attendant"			},
	{ "the Acolyte",		"the Acolyte"			},
	{ "the Novice",			"the Novice"			},
	{ "the Missionary",		"the Missionary"		},

	{ "the Adept",			"the Adept"			},
	{ "the Deacon",			"the Deaconess"			},
	{ "the Vicar",			"the Vicaress"			},
	{ "the Priest",			"the Priestess"			},
	{ "the Minister",		"the Lady Minister"		},

	{ "the Canon",			"the Canon"			},
	{ "the Levite",			"the Levitess"			},
	{ "the Curate",			"the Curess"			},
	{ "the Monk",			"the Nun"			},
	{ "the Healer",			"the Healess"			},

	{ "the Chaplain",		"the Chaplain"			},
	{ "the Expositor",		"the Expositress"		},
	{ "the Bishop",			"the Bishop"			},
	{ "the Arch Bishop",		"the Arch Lady of the Church"	},
	{ "the Patriarch",		"the Matriarch"			},

	{ "the Elder Patriarch",	"the Elder Matriarch"		},
	{ "the Grand Patriarch",	"the Grand Matriarch"		},
	{ "the Great Patriarch",	"the Great Matriarch"		},
	{ "the Demon Killer",		"the Demon Killer"		},
	{ "the Greater Demon Killer",	"the Greater Demon Killer"	},

	{ "the Avatar of an Immortal",	"the Avatar of an Immortal"	},
	{ "the Avatar of a Deity",	"the Avatar of a Deity"		},
	{ "the Avatar of a Supremity",	"the Avatar of a Supremity"	},
	{ "the Avatar of Implementors",	"the Avatar of Implementors"	},
	{ "the Master of all Divinity",	"the Mistress of all Divinity"	},

	{ "the Cardinal Hero",		"the Cardinal Heroine"		},
	{ "the Immortal Cardinal",	"the Immortal Cardinal"		},
	{ "the Deity",			"the Deity"			},
	{ "the Supreme Master",		"the Supreme Mistress"		},
	{ "the Implementor",		"the Implementress"		}
    },

    {
	{ "the Man",			"the Woman"			},

	{ "the Pilferer",		"the Pilferess"			},
	{ "the Footpad",		"the Footpad"			},
	{ "the Filcher",		"the Filcheress"		},
	{ "the Pick-Pocket",		"the Pick-Pocket"		},
	{ "the Sneak",			"the Sneak"			},

	{ "the Pincher",		"the Pincheress"		},
	{ "the Cut-Purse",		"the Cut-Purse"			},
	{ "the Snatcher",		"the Snatcheress"		},
	{ "the Sharper",		"the Sharpress"			},
	{ "the Rogue",			"the Rogue"			},

	{ "the Robber",			"the Robber"			},
	{ "the Magsman",		"the Magswoman"			},
	{ "the Highwayman",		"the Highwaywoman"		},
	{ "the Burglar",		"the Burglaress"		},
	{ "the Thief",			"the Thief"			},

	{ "the Knifer",			"the Knifer"			},
	{ "the Quick-Blade",		"the Quick-Blade"		},
	{ "the Killer",			"the Murderess"			},
	{ "the Brigand",		"the Brigand"			},
	{ "the Cut-Throat",		"the Cut-Throat"		},

	{ "the Spy",			"the Spy"			},
	{ "the Grand Spy",		"the Grand Spy"			},
	{ "the Master Spy",		"the Master Spy"		},
	{ "the Assassin",		"the Assassin"			},
	{ "the Greater Assassin",	"the Greater Assassin"		},

	{ "the Crimelord",		"the Crime Mistress"		},
	{ "the Infamous Crimelord",	"the Infamous Crime Mistress"	},
	{ "the Greater Crimelord",	"the Greater Crime Mistress"	},
	{ "the Master Crimelord",	"the Master Crime Mistress"	},
	{ "the Godfather",		"the Godmother"			},

	{ "the Assassin Hero",		"the Assassin Heroine"		},
	{ "the Immortal Assasin",	"the Immortal Assasin"		},
	{ "the Deity of Thieves",	"the Deity of Thieves"		},
	{ "the Supreme Master",		"the Supreme Mistress"		},
	{ "the Implementor",		"the Implementress"		}
    },

    {
	{ "the Man",			"the Woman"			},

	{ "the Swordpupil",		"the Swordpupil"		},
	{ "the Recruit",		"the Recruit"			},
	{ "the Sentry",			"the Sentress"			},
	{ "the Fighter",		"the Fighter"			},
	{ "the Soldier",		"the Soldier"			},

	{ "the Warrior",		"the Warrior"			},
	{ "the Veteran",		"the Veteran"			},
	{ "the Swordsman",		"the Swordswoman"		},
	{ "the Fencer",			"the Fenceress"			},
	{ "the Combatant",		"the Combatess"			},

	{ "the Hero",			"the Heroine"			},
	{ "the Myrmidon",		"the Myrmidon"			},
	{ "the Swashbuckler",		"the Swashbuckleress"		},
	{ "the Mercenary",		"the Mercenaress"		},
	{ "the Swordmaster",		"the Swordmistress"		},

	{ "the Lieutenant",		"the Lieutenant"		},
	{ "the Champion",		"the Lady Champion"		},
	{ "the Dragoon",		"the Lady Dragoon"		},
	{ "the Cavalier",		"the Lady Cavalier"		},
	{ "the Knight",			"the Lady Knight"		},

	{ "the Grand Knight",		"the Grand Knight"		},
	{ "the Master Knight",		"the Master Knight"		},
	{ "the Paladin",		"the Paladin"			},
	{ "the Grand Paladin",		"the Grand Paladin"		},
	{ "the Demon Slayer",		"the Demon Slayer"		},

	{ "the Greater Demon Slayer",	"the Greater Demon Slayer"	},
	{ "the Dragon Slayer",		"the Dragon Slayer"		},
	{ "the Greater Dragon Slayer",	"the Greater Dragon Slayer"	},
	{ "the Underlord",		"the Underlord"			},
	{ "the Overlord",		"the Overlord"			},

	{ "the Knight Hero",		"the Knight Heroine"		},
	{ "the Immortal Warlord",	"the Immortal Lady of War"	},
	{ "the Deity of War",		"the Deity of War"		},
	{ "the Supreme Master of War",	"the Supreme Mistress of War"	},
	{ "the Implementor",		"the Implementress"		}
    }
};


const char *item_types[] =
{
    "UNDEFINED",
    "LIGHT",
    "SCROLL",
    "WAND",
    "STAFF",
    "WEAPON",
    "FIRE WEAPON",
    "MISSILE",
    "TREASURE",
    "ARMOR",
    "POTION",
    "WORN",
    "OTHER",
    "TRASH",
    "TRAP",
    "CONTAINER",
    "NOTE",
    "LIQUID CONTAINER",
    "KEY",
    "FOOD",
    "MONEY",
    "PEN",
    "BOAT",
    "FLY",
    "MAGIC REGEN",
    "\n"
};

const char *wear_bits[] =
{
    "TAKE",
    "FINGER",
    "NECK",
    "BODY",
    "HEAD",
    "LEGS",
    "FEET",
    "HANDS",
    "ARMS",
    "SHIELD",
    "ABOUT",
    "WAISTE",
    "WRIST",
    "WIELD",
    "HOLD",
    "THROW",
    "LIGHT-SOURCE",
    "\n"
};

const char *extra_bits[] =
{
    "GLOW",
    "HUM",
    "DARK",
    "LOCK",
    "EVIL",
    "INVISIBLE",
    "MAGIC",
    "NODROP",
    "BLESS",
    "ANTI-GOOD",
    "ANTI-EVIL",
    "ANTI-NEUTRAL",
    "NO_RENT",
    "\n"
};

const char *extra_room_flags[] =
{
	"TRADE_NO_WAGON",
	"TRADE_NO_MULE",
	"TRADE_NO_HORSE",
	"\n"
};
const char *room_bits[] =
{
    "DARK",
    "DEATH",
    "NO_MOB",
    "INDOORS",
    "LAWFULL",
    "NEUTRAL",
    "CHAOTIC",
    "NO_MAGIC",
    "TUNNEL",
    "PRIVATE",
    "SAFE",
    "GODPROOF",
    "BLOCKING",
    "FIRE",
    "GAS",
    "COLD",
    "HOME",
    "NO_SUMMON",
    "DRINKROOM",
	"CHURCH",
    "\n"
};

const char *exit_bits[] =
{
    "IS-DOOR",
    "CLOSED",
    "LOCKED",
    "UNUSED",
    "UNUSED",
    "PICKPROOF",
    "SECRET",	
    "HIDDEN",
    "ILLUSION",
    "\n"
};

const char *sector_types[] =
{
    "Inside",
    "City",
    "Field",
    "Forest",
    "Hills",
    "Mountains",
    "Water Swim",
    "Water NoSwim",
    "Desert",
    "Underwater",
    "Air",
    "Swamp",
    "Jungle",
    "Arctic",
    "MANA",
    "\n"
};

const char *equipment_types[] =
{
    "Special",
    "Worn on right finger",
    "Worn on left finger",
    "First worn around Neck",
    "Second worn around Neck",
    "Worn on body",
    "Worn on head",
    "Worn on legs",
    "Worn on feet",
    "Worn on hands",
    "Worn on arms",
    "Worn as shield",
    "Worn about body",
    "Worn around waiste",
    "Worn around right wrist",
    "Worn around left wrist",
    "Wielded",
    "Held",
    "\n"
};

/* Should be in exact correlation as the AFF types */

const char *affected_bits[] =
{
    "BLIND",
    "INVISIBLE",
    "DETECT-EVIL",
    "DETECT-INVISIBLE",
    "DETECT-MAGIC",
    "SENSE-LIFE",
    "HOLD",
    "SANCTUARY",
    "GROUP",
    "UNUSED",
    "CURSE",
    "REP_ROOT",
    "POISON",
    "PROTECT-EVIL",
    "PROTECT-GOOD",
    "MORDENS-SWORD",
    "MAP-CATACOMBS",
    "SLEEP",
    "DODGE",
    "SNEAK",
    "HIDE",
    "FEAR",
    "CHARM",
    "FOLLOW",
    "WIMPY",
    "INFARED",
    "THIEF",
    "KILLER",
    "FLYING",
    "PLAGUE",
    "FIRESHIELD",
    "BREATH-WATER",
    "\n"
};

/* Should be in exact correlation as the APPLY types  */

const char *apply_types[] =
{
    "NONE",
    "STR",
    "DEX",
    "INT",
    "WIS",
    "CON",
    "SEX",
    "CLASS",
    "LEVEL",
    "AGE",
    "CHAR_WEIGHT",
    "CHAR_HEIGHT",
    "MANA",
    "HIT_POINTS",
    "MOVE",
    "GOLD",
    "EXP",
    "ARMOR",
    "HITROLL",
    "DAMROLL",
    "SAVING_PARA",
    "SAVING_ROD",
    "SAVING_PETRI",
    "SAVING_BREATH",
    "SAVING_SPELL",
    "\n"
};

const char *pc_class_types[] =
{
    "UNDEFINED",
    "Magic User",
    "Cleric",
    "Thief",
    "Warrior",
    "\n"
};

const char *npc_class_types[] =
{
    "ANIMAL(other)",
    "Magic User",
    "Cleric",
    "Thief",
    "Warrior",
    "\n"
};

const char *action_bits[] =
{
    "SPEC",
    "SENTINEL",
    "SCAVENGER",
    "ISNPC",
    "NICE-THIEF",
    "AGGRESSIVE",
    "STAY-ZONE",
    "WIMPY",
    "AGGRESSIVE_EVIL",
    "AGGRESSIVE_GOOD",
    "AGGRESSIVE_NEUTRAL",
    "\n"
};


const char *player_bits[] =
{
    "BRIEF",
    "NOSHOUT",
    "COMPACT",
    "DONTSET",
    "NOTELL",
    "NOEMOTE",
    " ",
    "FREEZE",
    "\n"
};


const char *position_types[] =
{
    "Dead",
    "Mortally wounded",
    "Incapasitated",
    "Stunned",
    "Sleeping",
    "Resting",
    "Sitting",
    "Fighting",
    "Standing",
    "\n"
};

const char *connected_types[] =
{
    "Playing",
    "Get name",
    "Get old password",
    "Confirm new name",
    "Get new password",
    "Confirm new password",
    "Get sex",
    "Get class",
    "Read message of the day",
    "Select Menu",
    "Reset Password",
    "Confirm Reset Password",
    "Get extra description",
    "Confirm Char Delete",
    "Show Title",
    "Hovering over Corpse",
    "Undead Corpse",
    "\n"
};

/* [class], [level] (all) */
const int thaco[4][36] =
{
    {
	200,200,200,200,200,200,180,180,180,180,180,160,160,160,160,160,
	140,140,140,140,140,120,120,120,120,120,100,100,100,100,100,80,
	0,0,0,0
    },
    {
	200,200,200,200,180,180,180,160,160,160,140,140,140,130,130,130,
	120,120,120,110,110,100,100,100,100,80,80,80,80,60,60,40,0,0,
	0,0
    },
    {
	200,195,190,180,170,160,150,140,135,130,125,120,120,115,110,110,
	110,100,100,90,80,80,70,60,60,50,40,30,30,20,20,10,0,0,0,0
    },
    {
	200,190,180,170,160,150,140,130,125,120,115,110,110,105,100,100,
	100,90,90,80,70,70,60,50,50,40,30,30,20,20,20,0,-10,-10,-10,-10
    }
};

/* [ch] strength apply (all) */
/*hitroll damroll weight items*/
const struct str_app_type str_app[26] =
{
    { -50,-4,   0,  0 },  /* 0  */
    { -50,-4,   3,  1 },  /* 1  */
    { -30,-4,   3,  2 },
    { -30,-4,  10,  3 },  /* 3  */
    { -20,-3,  25,  4 },
    { -20,-2,  55,  5 },  /* 5  */
    { -10,-1,  80,  6 },
    { -10, 0,  90,  7 },
    {  0, 0, 100,  8 },
    {  0, 0, 100,  9 },
    {  0, 0, 115, 10 }, /* 10  */
    {  0, 0, 115, 11 },
    {  0, 0, 140, 12 },
    {  0, 0, 140, 13 }, /* 13  */
    {  0, 1, 170, 14 },
    {  5, 1, 170, 15 }, /* 15  */
    {  5, 2, 195, 16 },
    {  10, 3, 220, 22 },
    {  10, 4, 250, 30 }, /* 18  */
    {  30, 7, 640, 40 },
    {  30, 8, 700, 40 }, /* 20  */
    {  40, 9, 810, 40 },
    {  40,10, 970, 40 },
    {  50,11,1130, 40 },
    {  60,12,1440, 40 },
    {  70,14,1750, 99 } /* 25            */
};

/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[26] =
{
    {-99,-99,-90,-99,-60},   /* 0 */
    {-90,-90,-60,-90,-50},   /* 1 */
    {-80,-80,-40,-80,-45},
    {-70,-70,-30,-70,-40},
    {-60,-60,-30,-60,-35},
    {-50,-50,-20,-50,-30},   /* 5 */
    {-40,-40,-20,-40,-25},
    {-30,-30,-15,-30,-20},
    {-20,-20,-15,-20,-15},
    {-15,-10,-10,-20,-10},
    {-10, -5,-10,-15, -5},   /* 10 */
    { -5,  0, -5,-10,  0},
    {  0,  0,  0, -5,  0},
    {  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0},   /* 15 */
    {  0,  5,  0,  0,  0},
    {  5, 10,  0,  5,  5},
    { 10, 15,  5, 10, 10},
    { 15, 20, 10, 15, 15},
    { 15, 20, 10, 15, 15},   /* 20 */
    { 20, 25, 10, 15, 20},
    { 20, 25, 15, 20, 20},
    { 25, 25, 15, 20, 20},
    { 25, 30, 15, 25, 25},
    { 25, 30, 15, 25, 25}    /* 25 */
};

/* [level] backstab multiplyer (thieves only) */
const byte backstab_mult[36] =
{
    1,   /* 0 */
    2,   /* 1 */
    2,
    2,
    2,
    2,   /* 5 */
    2,
    2,
    2,
    2,
    3,   /* 10 */
    3,
    3,
    3,
    3,
    3,   /* 15 */
    3,
    4,
    4,
    4,
    4,   /* 20 */
    4,
    4,
    4,
    4,
    5,   /* 25 */
    5,
    5,
    5,
    5,
    5,   /* 30 */
    5,
    7,
    7,
    7,
    10   /* 35 */
};

/* [dex] apply (all) */
struct dex_app_type dex_app[26] =
{
    {-7,-7, 20},   /* 0 */
    {-6,-6, 15},   /* 1 */
    {-4,-4, 15},
    {-3,-3, 15},
    {-2,-2, 10},
    {-1,-1, 10},   /* 5 */
    { 0, 0, 5},
    { 0, 0, 5},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},   /* 10 */
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0,-5},   /* 15 */
    { 1, 1,-5},
    { 2, 2,-10},
    { 2, 2,-15},
    { 3, 3,-15},
    { 3, 3,-15},   /* 20 */
    { 4, 4,-15},
    { 4, 4,-15},
    { 4, 4,-15},
    { 5, 5,-20},
    { 5, 5,-20}    /* 25 */
};

/* [con] apply (all) */
struct con_app_type con_app[26] =
{
    {-4,20},   /* 0 */
    {-3,25},   /* 1 */
    {-2,30},
    {-2,35},
    {-1,40},
    {-1,45},   /* 5 */
    {-1,50},
    { 0,55},
    { 0,60},
    { 0,65},
    { 0,70},   /* 10 */
    { 0,75},
    { 0,80},
    { 0,85},
    { 0,88},
    { 1,90},   /* 15 */
    { 2,95},
    { 2,97},
    { 3,99},
    { 3,99},
    { 4,99},   /* 20 */
    { 5,99},
    { 5,99},
    { 5,99},
    { 6,99},
    { 7,100}   /* 25 */
};

/* [int] apply (all) */
struct int_app_type int_app[26] =
{
    {  3 },
    {  5 },    /* 1 */
    {  7 },
    {  8 },
    {  9 },
    { 10 },   /* 5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },   /* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 30 },
    { 35 },   /* 15 */
    { 40 },
    { 45 },
    { 50 },
    { 53 },
    { 55 },   /* 20 */
    { 56 },
    { 60 },
    { 70 },
    { 80 },
    { 99 },   /* 25 */
};

/* [wis] apply (all) */
struct wis_app_type wis_app[26] =
{
    { 0 },   /* 0 */
    { 0 },   /* 1 */
    { 0 },
    { 0 },
    { 0 },
    { 1 },   /* 5 */
    { 1 },
    { 1 },
    { 1 },
    { 2 },
    { 2 },   /* 10 */
    { 2 },
    { 2 },
    { 2 },
    { 2 },
    { 3 },   /* 15 */
    { 3 },
    { 4 },
    { 5 },   /* 18 */
    { 6 },
    { 6 },   /* 20 */
    { 6 },
    { 6 },
    { 6 },
    { 6 },
    { 6 }  /* 25 */
};

/*
#define NOT_MAGE    1
#define NOT_CLERIC  2
#define NOT_THIEF   4
#define NOT_WARRIOR 8
#define TWO_HANDED  16
#define DAGGER      32
*/

char *class_rest[]=
    {"!MAGE","!CLERIC","!THIEF","!WARRIOR","TWO_HANDED","DAGGER","\n"};

char track_colors[5][2] =
{
	{ 37,  0 },
	{ 32,  0 },
	{ 36,  0 },
	{ 37,  0 },
	{ 37,  0 }
};

char *track_messages[5][2][2] = 
{  
	{
		{	"$n growls loudly in anger and attacks you!\r\n", 
		    "$n growls and snarls in anger, then attacks $N!\r\n" },
		{	"Other",  "Other"	}
	},
	{
		{	"$n begins chanting arcane words of magic and points a finger at you.\r\n", 
		    "$n begins chanting arcane words of magic and points towards $N.\r\n" },
		{	"Class Mage Message 1",  "Class Mage Message 2"	}
	},
	{
		{	"$n raises $s arms to the sky and shouts, 'Grant me the power to destroy the Heretic!\r\n", 
			"$n raises $s arms to the sky and shouts, 'Grant me the power to destroy the Heretic!\r\n" },
		{	"$n glares at you while grasping $s holy symbol and screams, 'Heathen!'.\r\n",  
			"$n points at $N and screams, 'Heathen!'.\r\n"	}
	},
	{
		{	"$n leaps from $s hiding place in the shadows and attacks you!\r\n", 
		    "$n leaps from $s hiding place in the shadows and attacks $N!\r\n" },
		{	"Class thief 2(tovict)",  "Class thief 2(room)"	}
	},
	{
		{	"$n shouts a loud battle cry and charges towards you!\r\n", 
		    "$n shouts a loud battle cry and charges at $N!\r\n" },
		{	"Class Warrior 2(tovict)",  "Class Warrior 2(room)"	}
	}

		
};	
		
