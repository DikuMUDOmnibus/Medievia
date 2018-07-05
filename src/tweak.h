#define WEIGHT_DAM      700000
#define WEIGHT_HP       50000
#define WEIGHT_MV       3000
#define WEIGHT_MANA     45000
#define WEIGHT_HIT      250000
#define W_SIZE_DICE     1000000
#define W_NUM_DICE      2000000
#define WEIGHT_AC       -10000
#define WEIGHT_ACAP     0
#define WEIGHT_WIS      55000
#define WEIGHT_CON      60000
#define WEIGHT_STR      175000
#define WEIGHT_INT      50000
#define WEIGHT_DEX      175000
#define WEIGHT_AGE      8000
#define WEIGHT_SSPELL   -500000
#define WEIGHT_SPARA    -200000
#define WEIGHT_SBREATH  -300000
#define WEIGHT_SPETRI   -200000
#define WEIGHT_SROD     -300000
#define WEIGHT_WEIGHT   3000
#define WEIGHT_EXP      0
#define WEIGHT_GOLD     1
#define WEIGHT_SEX      0
#define WEIGHT_CLASS    0
#define WEIGHT_LEVEL    0
#define WEIGHT_HEIGHT   3000
#define W_OBJW		500000 /* this is the modifier for the weigh() function */
#define W_GOOD_ITEM	500
#define WEAPON_CONST	50000 /* average damage * weapon const */
#define WEIGHT_LEVRES	500000



/* Global Tweaking Modifier(s) constant */

#define MAX_CHANCE 	4
#define GLOBAL_TWEAK    3

/*      LOCATION		WEIGHT		LOW	HIGH */

int weights[][5] = {
        { APPLY_NONE,           		0,		0,	0,	0 },
        { APPLY_STR,            		WEIGHT_STR,	-10,	10,	3 },
        { APPLY_DEX,            		WEIGHT_DEX,	-10,	10,	3 },
        { APPLY_INT,            		WEIGHT_INT,	-10,	10,	3 },
        { APPLY_WIS,            		WEIGHT_WIS,	-10,	10,	3 },
        { APPLY_CON,            		WEIGHT_CON,	-10,	10, 3 },
        { APPLY_SEX,            		WEIGHT_SEX,	0,	0, 0 },
        { APPLY_CLASS,          		WEIGHT_CLASS,	0,	0,	0 },
        { APPLY_LEVEL,          		WEIGHT_LEVEL,	0,	0,	0 },
        { APPLY_AGE,            		WEIGHT_AGE,	-80,	80,	20 },
        { APPLY_CHAR_WEIGHT,    		WEIGHT_WEIGHT,	-126,	127,	50 },
        { APPLY_CHAR_HEIGHT,    		WEIGHT_HEIGHT,	-126,	127,	50 },
        { APPLY_MANA,           		WEIGHT_MANA,	-126,	127,	50 },
        { APPLY_HIT,            		WEIGHT_HP,	-126,	127,	50 },
        { APPLY_MOVE,           		WEIGHT_MV,	-126,	127,	100 },
        { APPLY_GOLD,           		WEIGHT_GOLD,	0,	0,	0 },
        { APPLY_EXP,            		WEIGHT_EXP,	0,	0,	0 },
        { APPLY_ARMOR,          		WEIGHT_AC,	-126,	127,	50 },
        { APPLY_HITROLL,        		WEIGHT_HIT,	-126,	127,	6 },
        { APPLY_DAMROLL,        		WEIGHT_DAM,	-20,	20,	1 },
        { APPLY_SAVING_PARA,    		WEIGHT_SPARA,	-15,	15,	5 },
        { APPLY_SAVING_ROD,     		WEIGHT_SROD,	-15,	15,	5 },
        { APPLY_SAVING_PETRI,   		WEIGHT_SPETRI, 	-15,	15,	5 },
        { APPLY_SAVING_BREATH,  		WEIGHT_SBREATH,	-15,	15,	3 },
        { APPLY_SAVING_SPELL,   		WEIGHT_SSPELL,	-15,	15,	2 },
		{ 25,			WEIGHT_ACAP,	-30,	30,	0 },
		{ 26,			W_SIZE_DICE,	1,	20,	2 },
		{ 27,			W_NUM_DICE,		1,	20,	1 },
		{ 28,			W_OBJW,			0,	100,	10 },
		{ 29,			WEAPON_CONST,	0,	0,	0 },
		{ 30,			WEIGHT_LEVRES,  0,	31,	2 },
		{ 31,			GLOBAL_TWEAK,	0,	0,	0 },
		{ 32,			MAX_CHANCE,		0,	0,	0 }
};

float faLevconv[][2] = {
	{ 0,	1.7 },
	{ 1,	1.7 },
	{ 2,	1.6 },
	{ 3, 	1.6 },
	{ 4,	1.6 },
	{ 5,	1.6 },
	{ 6,	1.5 },
	{ 7,	1.5 },
	{ 8,	1.5 },
	{ 9,	1.5 },
	{ 10,	1.5 },
	{ 11,	1.5 },
	{ 12, 	1.4 },
	{ 13,	1.4 },
	{ 14,	1.4 },
	{ 15,	1.4 },
	{ 16,	1.4 },
	{ 17,	1.4 },
	{ 18,	1.3 },
	{ 19,	1.2 },
	{ 20,	1.2 },
	{ 21,	1.2 },
	{ 22,	1.2 },
	{ 23,	1.1 },
	{ 24, 	1.1 },
	{ 25,	1.0 },
	{ 26,	1.0 },
	{ 27,	1.0 },
	{ 28,	1.0 },
	{ 29,	1.0 },
	{ 30,	1.0 },
	{ 31,	1.0 },
	{ 32,	0 }, /* So no 32+ object can get tweake */
	{ 33,	0 },
	{ 34,	0 },
	{ 35,	0 }
};
