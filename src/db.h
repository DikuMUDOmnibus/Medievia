/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   *
***************************************************************************/


/* data files used by the game system */

#define MAX_ROOM    65534
#define MAX_MOB	    20000
#define MAX_OBJ	    20000
#define MAX_INDEX   20000
#define MAX_RACTION 20000
#define MAX_ZONE    200
#define MAX_TRIVIA  1500
#define DFLT_DIR          "../lib"        /* default data directory     */
#define SAVE_DIR          "../save"       /* save directory             */

#define WORLD_FILE        "medievia.wld" /* room definitions           */
#define MOB_FILE          "medievia.mob" /* monster prototypes    */
#define OBJ_FILE          "medievia.obj" /* object prototypes     */
#define ZONE_FILE         "medievia.zon" /* zone defs & command tables */
#define SHOP_FILE         "medievia.shp" /* shop messages and markups  */
#define GREETINGS_FILE    "greetings.txt" /* initial greetings screen   */
#define CREDITS_FILE      "credits.txt"   /* for the 'credits' command  */
#define NEWS_FILE         "news.txt"      /* for the 'news' command     */
#define MOTD_FILE         "motd.txt"      /* messages of today          */
#define MODS_FILE         "mods.txt"      /* our mods                   */
#define STORY_FILE        "story.txt"     /* game story                 */
#define TIME_FILE         "time.txt"      /* game calendar information  */
#define IDEA_FILE         "ideas.txt"     /* for the 'idea'-command     */
#define TYPO_FILE         "typos.txt"     /*         'typo'             */
#define BUG_FILE          "bugs.txt"      /*         'bug'              */
#define MESS_FILE         "messages.txt"  /* damage message             */
#define SOCIAL_FILE       "social.txt"    /* messgs for social acts     */
#define HELP_KWRD_FILE    "help_key.txt"  /* for HELP <keywrd>          */
#define HELP_PAGE_FILE    "help.txt"      /* for HELP <CR>              */
#define INFO_FILE         "info.txt"      /* for INFO                   */
#define WIZLIST_FILE      "wizlist.txt"   /* for WIZLIST                */
#define POSEMESS_FILE     "poses.txt"     /* for 'pose'-command         */
#define TRIVIA_FILE		  "trivia.txt"    /* for TRIVIA channel*/


/* public procedures in db.c */

void boot_db(void);
int create_entry(char *name);
void zone_update(void);
void init_char(struct char_data *ch);
void clear_char(struct char_data *ch);
void clear_object(struct obj_data *obj);
void reset_char(struct char_data *ch);
void free_char(struct char_data *ch);
int real_room(int virtual);
char *fread_string(FILE *fl);
int real_object(int virtual);
int real_mobile(int virtual);

#define REAL 0
#define VIRTUAL 1

struct obj_data *read_object(int nr, int eq_level);
struct char_data *read_mobile(int nr, int type);
struct obj_data *load_object(int nr, int eq_level, FILE *new);
struct char_data *load_mobile(int nr, int type, FILE *new);

/* structure for the reset commands */
struct reset_com
{
    int zone;
    char command;   /* current command                      */ 
    bool if_flag;   /* if TRUE: exe only if preceding exe'd */
    int arg1;       /*                                      */
    int arg2;       /* Arguments to the command             */
    int arg3;       /*                                      */
    int arg4;	    /*					    */

    /* 
    *  Commands:              *
    *  'M': Read a mobile     *
    *  'O': Read an object    *
    *  'P': Put obj in obj    *
    *  'G': Obj to char       *
    *  'E': Obj to char equip *
    *  'D': Set state of door *
    *  'W': Just like the 'E' command with a percent chance to load *
    */
};

/* continents */
#define MEDIEVIA 1
#define TRELLOR 2

/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
    char *name;             /* name of this zone                  */
    char *szMaintainer;	    /* Adopt a Zone: Maintainer ! 	  */
    int lifespan;           /* how long between resets (minutes)  */
    int age;                /* current age of this zone (minutes) */
    int top;                /* upper limit for rooms in this zone */
    int continent;
    int numkills;
    int population;
    int time_to_empty;
    bool populated;
    int num_resets;
    int reset_mode;         /* conditions for reset (see below)   */
    struct reset_com *cmd;  /* command table for reset             */
	short int siX;	/* HoloCords information for the zone */
	short int siY;
	int iRecallRoom;
	int iSocialRestricted;
    /*
    *  Reset mode:                              *
    *  0: Don't reset, and don't update age.    *
    *  1: Reset if no PC's are located in zone. *
    *  2: Just reset.                           *
    */
};




/* element in monster and object index-tables   */
struct index_data
{
    long pos;
    int number;     /* number of existing units of this mob/obj */
    int (*func)();
};




struct help_index_element
{
    char *keyword;
    long pos;
};

extern const int exp_table[36+1];
extern const char *title_table[4][36][2];
