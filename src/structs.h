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


#include <sys/types.h>
#include <time.h>
#include "room_afs.h"
typedef signed char		sbyte;
typedef unsigned char		ubyte;
typedef signed short int	sh_int;
typedef unsigned short int	ush_int;
typedef char			bool;
typedef char			byte;

typedef	struct char_data	CHAR_DATA;
typedef	struct obj_data		OBJ_DATA;

#define PULSE_ZONE     240
#define PULSE_MOBILE    16
#define PULSE_VIOLENCE  18
#define PULSE_HOVER    240  /* 1 Minute  */
#define PULSE_UNDEAD  1200  /* 5 Minutes */
#define WAIT_SEC         4
#define WAIT_ROUND       4

#define MAX_STRING_LENGTH   8192
#define MAX_INPUT_LENGTH     160
#define MAX_MESSAGES          60

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  31
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

/* flags for god info message toggles */
#define GOD_INOUT	1	/* players in and out */
#define GOD_ONOFF	2	/* god channel */
#define GOD_ZONERESET	4	/* displaying zone resets */
#define GOD_ERRORS	8	/* displaying all logged errors */
#define GOD_DEATHS	16	/* display all deaths and where */
#define GOD_SUMMONS	32	/* displaying summons (who and where) */

/* flags for class restrictions to rooms */
#define NO_THIEF	1
#define NO_MAGE 	2
#define NO_CLERIC	4
#define NO_WARRIOR	8

/* flags for alignment restrictions in rooms */
#define NO_EVIL		1
#define NO_NEUTRAL	2
#define NO_GOOD		4

 
struct EARTHQUAKE {  
    bool bRealQuake;
    float iRictor;
    int iLengthInSeconds;
    time_t time_tStartTime;
    int iNumShakes;
    time_t time_tNextShake;
};
  


/* Room actions structures */
struct room_actions {
	int type;         /*type: chatter, stat_change etc                */
	int in_room;      /*room action takes place in                    */
	int action_num;   /*The number in actions array this is stored    */
	int chance;       /* 1-100 percent chance of firing this action   */
 	long arg1;        /*multiple uses arg1,2,3 depending on action    */
	long arg2;
	long arg3;
	int method;       /*methods: player, room etc...                  */
	int fired_next;   /*which action to fire next if this one fired   */
	int notfired_next;/*which action to fire if this one did not fire */
	char *text1;      /*multiple uses text1,2,3 depending on action   */
	char *text2;
	char *text3;
	int autostart;
};
struct raction_timing{    /*simple stores the number of seconds since 1970*/
	int action;   /*comm.c looks to see if that time passed, if so*/
	time_t time;      /*it fires the action and removes from list     */
	struct raction_timing *next;
};
struct raction_oe_design{ /*Due to the complicated methods of prompting   */
	char *chance;     /*for user input during OE and the difficult    */
	int chance_next;  /*differences of what input is needed for OE'ing*/
	char *method;     /*actions, I designed the OE action prompter to */
	int method_next;  /*use the same code for all actions.  It will do*/
	char *arg1;       /*this because the coder of new actions will use*/
	int arg1_next;    /*this structure to tell the code what the      */
	char *arg2;       /*action needs in various fields and also to    */
	int arg2_next;    /*move from field to field in the best order,   */
	char *arg3;       /*skipping fields quite often.  Otherwise the   */
	int arg3_next;    /*function for OE'ing prompts would become well */
	char *text1;      /*in excess of 1000 lines of if statements.     */
	int text1_next;   /*This structure is used in a array of pointers */
	char *text2;      /*of this type that the coder will set up.  The */
	int text2_next;   /*array number will equal the action type number*/
	char *text3;      /*just like the action_list array for the type  */
	int text3_next;   /*menu. (What came first the chicken or egg?)   */
};

/*
 * Site ban structure.
 */
struct ban_t
{
	char *name;             /* Name of site */
	struct ban_t *next;     /* next in list */
	char *reason;
	char *date;
	char *god;
};

/* ======================================================================= */

/* these are for room extra_flags */
#define TRADE_NO_WAGON 	1
#define TRADE_NO_MULE  	2
#define TRADE_NO_HORSE 	4
/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database    */

/* Bitvector For 'room_flags' */

#define DARK        1
#define DEATH		2
#define NO_MOB		4
#define INDOORS		8
#define LAWFULL		16
#define NEUTRAL		32
#define CHAOTIC		64
#define NO_MAGIC	128
#define TUNNEL		256
#define PRIVATE		512
#define SAFE		1024
#define GODPROOF	2048
#define BLOCKING	4096
#define FIRE		8192
#define	GAS		16384
#define COLD		32768
#define HOME		65536
#define NO_SUMMON	131072
#define DRINKROOM   	262144
#define CHURCH		524288
/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR      1
#define EX_CLOSED      2
#define EX_LOCKED      4
#define EX_RSCLOSED    8
#define EX_RSLOCKED   16
#define EX_PICKPROOF  32
#define EX_SECRET     64
#define EX_HIDDEN     128
#define EX_ILLUSION   256

/* For 'Sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_DESERT          8
#define SECT_UNDERWATER      9
#define SECT_AIR             10
#define SECT_SWAMP           11
#define SECT_JUNGLE          12
#define SECT_ARCTIC          13
#define SECT_MANA            14
struct room_direction_data
{
    char *general_description;       /* When look DIR.                    */ 
    char *keyword;                   /* for open/close                    */  
    char *entrance;		             /* what to show in room player enters*/
    char *exit;			             /* what to show when player exits    */
    sh_int exit_info;                /* Exit info                         */
    sh_int key;                      /* Key's number (-1 for no key)      */
    int to_room;                     /* Where direction leeds (NOWHERE)   */
};

/* ========================= Structure for room ========================== */
struct room_data
{
    int number;               /* Rooms number                       */
    sh_int zone;                 /* Room zone (for resetting)          */
    unsigned char sector_type;             /* sector type (move/hide)            */
    int extra_flags;		 /* misc flags			       */
	sh_int hurt;
    unsigned char class_restriction;    /* restrict these classes from room   */
    sh_int level_restriction;    /* +20 or -20 etc levels restricted   */
    sh_int align_restriction;    /* restrict these aligns from room    */
    int mount_restriction;	 /* restrict these mount types         */
    sh_int move_mod;		 /* movement points modifier	       */
    int pressure_mod;	 	 /* air pressure mod		       */
    int temperature_mod;  	 /* temperature mod		       */
    char *name;                  /* Rooms name 'You are ...'           */
    char *description;           /* Shown when entered                 */
    struct extra_descr_data *ex_description; /* for examine/look       */
    struct room_direction_data *dir_option[6]; /* Directions           */
    int room_flags;           /* DEATH,DARK ... etc                 */ 
    byte light;                  /* Number of lightsources in room     */
    int (*funct)();              /* special procedure                  */
    struct room_affect *room_afs;/* list of room_affects	       */
    struct obj_data *contents;   /* List of items in room              */
    struct char_data *people;    /* List of NPC / PC in room           */
	struct room_data *holoprev;
	struct room_data *holonext;
	unsigned char holoroom;
	sh_int holox;
	sh_int holoy;
	struct FREIGHT *stpFreight;
};



/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data
{
    int hours;
    int day;
    int month;
    int year;
};

/* These data contain information about a players time data */
struct time_data
{
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  int played;      /* This is the total accumulated time played in secs */
};

/* ======================================================================== */

/* How much light is in the land ? */

#define SUN_DARK    0
#define SUN_RISE    1
#define SUN_LIGHT   2
#define SUN_SET     3

/* And how is the sky ? */

#define SKY_CLOUDLESS   0
#define SKY_CLOUDY  1
#define SKY_RAINING 2
#define SKY_LIGHTNING   3

struct weather_data
{
    int pressure;   /* How is the pressure ( Mb ) */
    int change; /* How fast and what way does it change. */
    int sky;    /* How is the sky. */
    int sunlight;   /* And how much sun. */
};


struct damage_rooms
{
    int room_num;
    int damage_type;
    int damage_amt;
    struct damage_rooms *next;
};




/* ***********************************************************
*  The following structures are related to descriptor_data   *
*********************************************************** */



struct txt_block
{
    char *text;
    struct txt_block *next;
};

typedef struct txt_q
{
    struct txt_block *head;
    struct txt_block *tail;
} TXT_Q;



/* modes of connectedness */

#define CON_PLAYING			 0
#define CON_GET_NAME			 1
#define CON_GET_OLD_PASSWORD		 2
#define CON_CONFIRM_NEW_NAME		 3
#define CON_GET_NEW_PASSWORD		 4
#define CON_CONFIRM_NEW_PASSWORD	 5
#define CON_GET_NEW_SEX			 6
#define CON_GET_NEW_CLASS		 7
#define CON_READ_MOTD			 8
#define CON_SELECT_MENU			 9
#define CON_RESET_PASSWORD		10
#define CON_CONFIRM_RESET_PASSWORD	11
#define CON_EXDSCR			12
#define CON_CONFIRM_CHAR_DELETE		13
#define CON_SHOW_TITLE			14
#define CON_HOVERING			15
#define CON_UNDEAD			16
#define CON_SOCIAL_ZONE			17

struct snoop_data
{
    struct char_data *snooping; 
    struct char_data *snoop_by;
};

#define MAX_EDITOR_LINES		100

struct descriptor_data
{
    int		descriptor;		/* file descriptor for socket	*/
    char *	name;			/* Copy of the player name	*/
    char	host[50];              	/* hostname			*/
    int		pos;			/* position in player-file	*/
    int		connected;		/* mode of 'connectedness'	*/
    int		wait;			/* wait for how many loops	*/
    char *	showstr_head;		/* for paging through texts	*/
    char *	showstr_point;		/*       -			*/
    bool	newline;		/* prepend newline in output	*/
    char	**str;			/* for the modify-str system	*/
    int		max_str;		/*	-			*/
    char        data[MAX_EDITOR_LINES][MAX_INPUT_LENGTH+1];
    char	oneline;
    int	 	curline;
    char	editing[50];
    char	buf[10000+MAX_INPUT_LENGTH];	/* buffer for raw input	*/
    char	last_input[MAX_INPUT_LENGTH];	/* the last input	*/
    TXT_Q	output;			/* queue of strings to send	*/
    TXT_Q	input;			/* queue of unprocessed input	*/
    CHAR_DATA *	character;		/* linked to char		*/
    CHAR_DATA *	original;		/* for switch / return		*/
    struct snoop_data	snoop;		/* to snoop people		*/
    struct descriptor_data *	next;	/* link to next descriptor	*/
    int         tick_wait;      	/* # ticks desired to wait	*/
    int         child_pid;		/* AVI pid of child process     */
    char        ignore_input;           /* AVI are we ignoring input?   */
	char 		templog;
};

struct msg_type 
{
    char *attacker_msg;  /* message to attacker */
    char *victim_msg;    /* message to victim   */
    char *room_msg;      /* message to room     */
};

struct message_type
{
    struct msg_type die_msg;      /* messages when death            */
    struct msg_type miss_msg;     /* messages when miss             */
    struct msg_type hit_msg;      /* messages when hit              */
    struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
    struct msg_type god_msg;      /* messages when hit on god       */
    struct message_type *next;/* to next messages of this kind.*/
};

struct message_list
{
    int a_type;               /* Attack type				*/
    int number_of_attacks;    /* # messages to chose from		*/
    struct message_type *msg; /* List of messages			*/
};

struct dex_skill_type
{
    sh_int p_pocket;
    sh_int p_locks;
    sh_int traps;
    sh_int sneak;
    sh_int hide;
};

struct dex_app_type
{
    sh_int reaction;
    sh_int miss_att;
    sh_int defensive;
};

struct str_app_type
{
    sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
    sh_int todam;    /* Damage Bonus/Penalty                */
    sh_int carry_w;  /* Maximum weight that can be carrried */
    sh_int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type
{
    byte bonus;       /* how many bonus skills a player can */
		      /* practice pr. level                 */
};

struct int_app_type
{
    byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type
{
    sh_int hitp;
    sh_int shock;
};

/*
 * TO types for act() output.
 */
#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3

#define MAX_CLANS 100
#define MAX_CLAN_MEMBERS 50

struct global_clan_info_struct
{
    int clan[MAX_CLANS];
    unsigned int eggs[MAX_CLANS];
    unsigned int gold[MAX_CLANS];
    char clan_leader[MAX_CLANS][25];
    char clan_name[MAX_CLANS][50];
};

struct clan_info
{
    unsigned int gold;
    int clanhall_medievia;
    int clanhall_trellor;
    int clan;
    int expansion[10];
    unsigned int eggs;
    char clan_name[50];
    char clan_leader[25];
    char members[MAX_CLAN_MEMBERS][25];
};
