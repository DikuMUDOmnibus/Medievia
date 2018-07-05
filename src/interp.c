/***************************************************************************
*					 MEDIEVIA CyberSpace Code and Data files		       *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*							   All rights reserved				           *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>
          
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "interp.h"
#include "db.h"
#include "utils.h"
#include "limits.h"

extern int high_mem;
extern bool guy_deleted;
extern bool stuck_in_water_reaper(struct char_data *ch);
extern void communicating(struct char_data *ch, char *text);
extern void do_reanimate(struct char_data *ch, char *argument, int cmd);
extern bool check_social( struct char_data *ch, char *pcomm,
    int length, char *arg );
extern int number_of_rooms;
extern unsigned long int connect_count;
extern int number_of_zones;
extern char global_color;
extern struct index_data *mob_index; 
extern struct index_data *obj_index;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern int number_socials; 
extern int top_of_world;  
extern int top_of_zone_table;  
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern char grep_text[250];
char dothis[MAX_INPUT_LENGTH];
FILE *log_fh;

struct command_info
{
    char *command_name;             /* Name of this command             */
    DO_FUN *command_pointer;        /* Function that does it            */
    byte minimum_position;          /* Position commander must be in    */
    byte minimum_level;             /* Minimum level needed             */
    int command_number;             /* Passed to function as argument   */
	byte fly;					    /* Can do this command when flying? */
};

/*
 * Note that command number is always non-zero for user commands.
 * Various spec_proc.c functions impose ICKY requirements.
 * Someday I'll NUKE the arg and pass the command name instead.
 *
 * Spec_procs that need fixing:
 *	shop_keeper
 */
struct command_info cmd_info[] =
{
    /*
     * Common movement commands.
     */
    { "north",      do_move,        POSITION_STANDING,  0,  1 ,0},
    { "east",       do_move,        POSITION_STANDING,  0,  2 ,0},
    { "south",      do_move,        POSITION_STANDING,  0,  3 ,0},
    { "west",       do_move,        POSITION_STANDING,  0,  4 ,0},
    { "up",         do_move,        POSITION_STANDING,  0,  5 ,0},
    { "down",       do_move,        POSITION_STANDING,  0,  6 ,0},
    { "cast",       do_cast,        POSITION_SITTING,   0,  9 ,0},
    { "alias",      do_alias,       POSITION_DEAD,      0,  9 ,1},
    { "unalias",    do_unalias,     POSITION_DEAD,      0,  9 ,1},
    { "exits",      do_exits,       POSITION_RESTING,   0,  9 ,0},
    { "get",        do_get,         POSITION_RESTING,   0,  222 ,0},
    { "inventory",  do_inventory,   POSITION_DEAD,      0,  9 ,1},
    { "look",       do_look,        POSITION_RESTING,   0,  15 ,1},
    { "order",      do_order,       POSITION_RESTING,   0,  9 ,0},
    { "rest",       do_rest,        POSITION_RESTING,   0,  9 ,0},
    { "stand",      do_stand,       POSITION_RESTING,   0,  9 ,0},
    { "tell",       do_tell,        POSITION_RESTING,   0,  9 ,0},
    { "wield",      do_wield,       POSITION_RESTING,   0,  9 ,1},
    { "wizhelp",    do_wizhelp,     POSITION_DEAD,      31, 9 ,1},
    { "set",        do_set,         POSITION_DEAD,      33, 9 ,1},
    { "brief",      do_brief,       POSITION_DEAD,      0,  9 ,1},
    { "hide",       do_hide,        POSITION_RESTING,   0,  9 ,0},
    { "compact",    do_compact,     POSITION_DEAD,      0,  9 ,1},
    { "credits",    do_credits,     POSITION_DEAD,      0,  9 ,1},
    { "equipment",  do_equipment,   POSITION_DEAD,      0,  9 ,1},
    { "help",       do_help,        POSITION_DEAD,      0,  9 ,1},
    { "idea",       do_idea,        POSITION_DEAD,      0,  9 ,1},
    { "info",       do_info,        POSITION_DEAD,      0,  9 ,1},
    { "levels",     do_levels,      POSITION_DEAD,      0,  9 ,1},
    { "news",       do_news,        POSITION_DEAD,      0,  9 ,1},
    { "score",      do_score,       POSITION_DEAD,      0,  9 ,0},
    { "shuffle",    do_not_here,    POSITION_RESTING,   0,  1091 ,0},
    { "show",       do_not_here,    POSITION_RESTING,   0,  1096 ,0},
    { "showdeal",   do_not_here,    POSITION_RESTING,   0,  1092 ,0},
    { "sd",         do_not_here,    POSITION_RESTING,   0,  1092 ,0},
    { "hidedeal",   do_not_here,    POSITION_RESTING,   0,  1093 ,0},
    { "hd",         do_not_here,    POSITION_RESTING,   0,  1093 ,0},
    { "description",do_description, POSITION_STANDING,  0,  9 ,0},
    { "downdeal",   do_not_here,    POSITION_RESTING,   0,  1094 ,0},
    { "dd",         do_not_here,    POSITION_RESTING,   0,  1094 ,0},
    { "ante",       do_not_here,    POSITION_RESTING,   0,  1097 ,0},
    { "raise",      do_not_here,    POSITION_RESTING,   0,  1098 ,0},
    { "cards",      do_not_here,    POSITION_RESTING,   0,  1100 ,0},
    { "showstats",  do_showstats,   POSITION_RESTING,   32,  9 ,0},
    { "story",      do_story,       POSITION_DEAD,      0,  9 ,0},
    { "tick",       do_tick,        POSITION_DEAD,      0,  9 ,0},
    { "time",       do_time,        POSITION_DEAD,      0,  9 ,1},
    { "title",      do_title,       POSITION_DEAD,      0,  9 ,1},
    { "typo",       do_typo,        POSITION_DEAD,      0,  9 ,1},
    { "weather",    do_weather,     POSITION_DEAD,      0,  9 ,0},
    { "who",        do_who,         POSITION_DEAD,      0,  9 ,1},
    { "wizlist",    do_wizlist,     POSITION_DEAD,      0,  9 ,1},
    { "ask",        do_ask,         POSITION_RESTING,   0,  328 ,0},
    { "asay",       do_asay,        POSITION_RESTING,   0,  9 ,0}, 
    { "clantalk",   do_clantalk,    POSITION_RESTING,	0,  9 ,0}, 
    { "auction",    do_auction,     POSITION_RESTING,   0,  9 ,0},
    { "emote",      do_emote,       POSITION_DEAD,      0,  9 ,0},
    { ",",          do_emote,       POSITION_DEAD,      0,  9 ,0},    
    { "ftell",      do_grouptell,   POSITION_DEAD,      0,  9 ,0},
    { ";",          do_grouptell,   POSITION_DEAD,      0,  9 ,0},
    { "fsay",       do_grouptell,   POSITION_DEAD,      0,  9 ,0},
    { "clsay",      do_clsay,       POSITION_RESTING,   0,  9 ,0},
    { "insult",     do_insult,      POSITION_RESTING,   0,  9 ,0},
    { "pose",       do_pose,        POSITION_RESTING,   0,  9 ,0},
    { "report",     do_report,      POSITION_DEAD,      0,  9 ,0},
    { "say",        do_say,         POSITION_RESTING,   0,  9 ,0},
    { "'",          do_say,         POSITION_RESTING,   0,  9 ,0},
    { "shout",      do_shout,       POSITION_RESTING,   0,  9 ,0},
    { "whisper",    do_whisper,     POSITION_RESTING,   0,  9 ,0},
    { "setcomm",    do_setcomm,     POSITION_DEAD,      0,  9 ,1},
    { "close",      do_close,       POSITION_RESTING,   0,  221 ,0},
    { "drink",      do_drink,       POSITION_RESTING,   0,  9800,1},
    { "drop",       do_drop,        POSITION_RESTING,   0,  9 ,0},
    { "eat",        do_eat,         POSITION_RESTING,   0,  9801,1},
    { "fill",       do_fill,        POSITION_RESTING,   0,  332 ,0},
    { "give",       do_give,        POSITION_RESTING,   0,  9 ,0},
    { "grab",       do_grab,        POSITION_RESTING,   0,  9 ,0},
    { "hold",       do_grab,        POSITION_RESTING,   0,  9 ,1},
    { "lock",       do_lock,        POSITION_RESTING,   0,  6524 ,0},
    { "offer",      do_not_here,    POSITION_RESTING,   0,  70 ,0},
    { "open",       do_open,        POSITION_RESTING,   0,  220 ,0},
    { "pour",       do_pour,        POSITION_RESTING,   0,  9 ,0},
    { "put",        do_put,         POSITION_RESTING,   0,  223 ,1},
    { "quaff",      do_quaff,       POSITION_RESTING,   0,  9 ,1},
    { "read",       do_read,        POSITION_RESTING,   0,  63 ,0},
    { "recite",     do_recite,      POSITION_RESTING,   0,  9 ,0},
    { "remove",     do_remove,      POSITION_RESTING,   0,  66 ,1},
    { "sip",        do_sip,         POSITION_RESTING,   0,  9 ,1},
    { "talk",       do_talk,        POSITION_RESTING,   0,  9 ,0},
    { "take",       do_get,         POSITION_RESTING,   0,  222 ,0},
    { "junk",       do_tap,         POSITION_RESTING,   0,  9 ,0},
    { "sacrifice",  do_tap,         POSITION_RESTING,   0,  9 ,0},
    { "tap",        do_tap,         POSITION_RESTING,   0,  9 ,0},
    { "taste",      do_taste,       POSITION_RESTING,   0,  9 ,1},
    { "unlock",     do_unlock,      POSITION_RESTING,   0,  6523 ,0},
    { "use",        do_use,         POSITION_RESTING,   0,  9 ,0},
    { "wear",       do_wear,        POSITION_RESTING,   0,  9 ,0},
    { "link",	    do_socialwarp,  POSITION_STANDING,  0,  9 ,0},
    { "disarm",     do_disarm,      POSITION_FIGHTING,  0,  9 ,0},
    { "discard",    do_not_here,    POSITION_RESTING,   0,  1101 ,0},
    { "flee",       do_flee,        POSITION_FIGHTING,  0,  9 ,0},
    { "sit",        do_sit,         POSITION_RESTING,   0,  9 ,0},
    { "sleep",      do_sleep,       POSITION_SLEEPING,  0,  9 ,0},
    { "wake",       do_wake,        POSITION_SLEEPING,  0,  9 ,0},
    { "consider",   do_consider,    POSITION_RESTING,   0,  9 ,0},
    { "donate",     do_donate,      POSITION_RESTING,   0,  9 ,0},
    { "enter",      do_enter,       POSITION_STANDING,  0,  28 ,0},
    { "examine",    do_examine,     POSITION_RESTING,   0,  9 ,0},
    { "follow",     do_follow,      POSITION_RESTING,   0,  9 ,0},
    { "fold",       do_not_here,    POSITION_RESTING,   0,  1099 ,0},
    { "group",      do_group,       POSITION_RESTING,   0,  9 ,0},
    { "leave",      do_leave,       POSITION_STANDING,  0,  9 ,0},
    { "pick",       do_pick,        POSITION_STANDING,  0,  9 ,0},
    { "message",    do_pigeon,	    POSITION_STANDING,  0,  9 ,1},
    { "qui",        do_qui,         POSITION_DEAD,      0,  9 ,0},
    { "quit",       do_quit,        POSITION_DEAD,      0,  13 ,0},
    { "rent",       do_rent,	    POSITION_RESTING,   0,  9 ,0},
    { "return",     do_return,      POSITION_DEAD,      0,  9 ,0},
    { "save",       do_save,        POSITION_DEAD,      0,  9 ,1},
    { "savedisplay",do_display_autosave,POSITION_DEAD,35,9,1},
    { "sneak",      do_sneak,       POSITION_STANDING,  1,  9 ,0},
    { "split",      do_split,       POSITION_RESTING,   0,  9 ,0},
    { "search",	    do_SecDrSearch, POSITION_STANDING,  1,  9 ,0},
    { "tag",        do_tag,         POSITION_STANDING,  0,  9 ,0},
    { "where",      do_where,       POSITION_RESTING,   0,  53 ,0},
    { "wimpy",      do_wimpy,       POSITION_DEAD,      0,  9 ,1},
    { "write",      do_write,       POSITION_STANDING,  0,  149 ,0},
    { "buy",        do_not_here,    POSITION_STANDING,  0,  56 ,0},
    { "sell",       do_not_here,    POSITION_STANDING,  0,  57 ,0},
    { "value",      do_not_here,    POSITION_STANDING,  0,  58 ,0},
    { "list",       do_not_here,    POSITION_STANDING,  0,  59 ,0},
    { "practice",   do_practice,    POSITION_RESTING,   1,  164 ,0},
    { "practise",   do_practice,    POSITION_RESTING,   1,  164 ,0},
    { "pray",       do_pray,        POSITION_DEAD,      0,  9866 ,1},
    { "train",      do_not_here,    POSITION_RESTING,   1,  165 ,0},
    { "deposit",    do_not_here,    POSITION_RESTING,   0,  100 ,0},
    { "withdraw",   do_not_here,    POSITION_RESTING,   0,  101 ,0},
    { "balance",    do_not_here,    POSITION_RESTING,   0,  102 ,0},
    { "send",       do_not_here,    POSITION_STANDING,  1,  100 ,0},
    { "recieve",    do_not_here,    POSITION_STANDING,  0,  101 ,0},
    { "receive",    do_not_here,    POSITION_STANDING,  0,  101 ,0},
    { "rentlocker", do_not_here,    POSITION_STANDING,  0,  224 ,0},
    { "insure",     do_not_here,    POSITION_STANDING,  0,  225 ,0},
    { "claim",      do_not_here,    POSITION_STANDING,  0,  226 ,0},
    { "advance",    do_advance,     POSITION_DEAD,      35, 9 ,0},
    { "sdb",        do_searchdb,    POSITION_DEAD,      33, 9 ,1},
    { "allow",      do_allow,       POSITION_DEAD,      33, 9 ,1},
    { "ban",        do_ban,         POSITION_DEAD,      33, 9 ,1},
    { "disconnect", do_disconnect,  POSITION_DEAD,      33, 9 ,1},
    { "freeze",     do_freeze,      POSITION_DEAD,      33, 9 ,1},
    { "log",        do_log,         POSITION_DEAD,      33, 9 ,1},
    { "logclan",    do_logclan,     POSITION_DEAD,      33, 9 ,1},
    { "loot",       do_steal,       POSITION_STANDING,  1,  9 ,0},
    { "purge",      do_purge,       POSITION_DEAD,      33, 9 ,0},
    { "pull",       do_not_here,    POSITION_RESTING,   0,  3673 ,0},
    { "raingold",   do_rain_gold,   POSITION_DEAD,      35, 9 ,1},
    { "removetag",  do_removetag,   POSITION_DEAD,      34, 9 ,1},
    { "reroll",     do_reroll,      POSITION_DEAD,      34, 9 ,1},
    { "reset",      do_reroll,      POSITION_DEAD,      34, 9 ,1},
    { "sellhome",   do_sellhome,    POSITION_DEAD,      34, 9 ,1},
    { "setdesigner",do_setdesigner, POSITION_DEAD,      35, 9 ,1},
    { "shutdow",    do_shutdow,     POSITION_DEAD,      35, 9 ,1},
    { "shutdown",   do_shutdown,    POSITION_DEAD,      35, 9 ,1},
    { "restar",     do_restar,      POSITION_DEAD,      35, 9 ,1},
    { "restart",    do_restart,     POSITION_DEAD,      35, 9 ,1},
    { "sockets",    do_sockets,     POSITION_DEAD,      33, 9 ,1},
    { "starttag",   do_starttag,    POSITION_DEAD,      33, 9 ,1},
    { "string",     do_string,      POSITION_RESTING,   33, 9 ,0},
    { "wizlock",    do_wizlock,     POSITION_DEAD,      34, 9 ,1},
    { "force",      do_force,       POSITION_DEAD,      33, 9 ,1},
    { "load",       do_load,        POSITION_DEAD,      33, 9 ,0},
    { "noemote",    do_noemote,     POSITION_DEAD,      33, 9 ,1},
    { "nosocial",   do_noemote,     POSITION_DEAD,      33, 9 ,1},
    { "note",       do_note,        POSITION_DEAD,      32, 9 ,1},
    { "notell",     do_notell,      POSITION_DEAD,      33, 9 ,1},
    { "pardon",     do_pardon,      POSITION_DEAD,      33, 9 ,1},
    { "restore",    do_restore,     POSITION_DEAD,      33, 9 ,1},
    { "teleport",   do_teleport,    POSITION_DEAD,      33, 9 ,1},
    { "trans",      do_trans,       POSITION_DEAD,      33, 9 ,0},
    { "transclan",  do_transclan,   POSITION_DEAD,      33, 9 ,0},
    { "trap",       do_trap,        POSITION_STANDING,   1, 9 ,0},
    { "track",	    do_track,	    POSITION_STANDING,   1, 9 ,0},
    { "trackset",   do_trackset,    POSITION_DEAD,      35, 9 ,0},
    { "at",         do_at,          POSITION_DEAD,      33, 9 ,0},
    { "echo",       do_echo,        POSITION_DEAD,      33, 9 ,1},
    { "lecho",      do_lecho,       POSITION_DEAD,      32, 9 ,0},
    { "goto",       do_goto,        POSITION_DEAD,      32, 9 ,0},
    { "snoop",      do_snoop,       POSITION_DEAD,      33, 9 ,0},
    { "stat",       do_stat,        POSITION_DEAD,      32, 9 ,1},
    { "skillstat",  do_skillstat,   POSITION_DEAD,      33, 9 ,1},
    { "survey",	    do_survey,	    POSITION_RESTING,	1,  9 ,1},
    { "switch",     do_switch,      POSITION_DEAD,      33, 9 ,1},
    { "invis",      do_wizinvis,    POSITION_DEAD,      32, 9 ,1},
    { "wizinvis",   do_wizinvis,    POSITION_DEAD,      32, 9 ,1},
    { "holylite",   do_holylite,    POSITION_DEAD,      32, 9 ,1},
    { "immortal",   do_wiz,         POSITION_RESTING,   31, 9 ,1},
    { ":",          do_wiz,         POSITION_RESTING,   31, 9 ,1},
    { "]",          do_imm,         POSITION_DEAD,      35, 9 ,1},
    { "quest",      do_quest,       POSITION_RESTING,   0,  9 ,1},
    { "gossip",     do_gossip,      POSITION_RESTING,   0,  9 ,1},
    { "enrollclan", do_enrollclan,  POSITION_DEAD,      0,  9 ,0},
    { "removeclan", do_removeclan,  POSITION_DEAD,      0,  9 ,0},
    { "clanwho",    do_clanwho,     POSITION_DEAD,      0,  9 ,1},
    { "clanwhere",  do_clanwhere,   POSITION_DEAD,      0,  9 ,1},
    { "autoexit",   do_autoexit,    POSITION_DEAD,      0,  9 ,1},
    { "dream",      do_dream,       POSITION_DEAD,      0,  9 ,0},
    { "throw",      do_throw,       POSITION_FIGHTING,  0,  9 ,0},
    { "charge",     do_charge,      POSITION_FIGHTING,  0,  9 ,0},
    { "leadclan",   do_lead_clan,   POSITION_DEAD,     33,  9 ,0},
    { "gohome",     do_gohome,      POSITION_RESTING,   1,  9 ,0},
    { "discuss",    do_discuss,     POSITION_RESTING,   0,  9 ,1},
    { "saveout",    do_saveout,     POSITION_DEAD,     35,  9 ,1},
    { "color",      do_color,       POSITION_DEAD,      0,  9 ,1},
    { "room",       do_room,        POSITION_RESTING,  15,  9 ,0},
    { "scan",       do_scan,        POSITION_STANDING,  0,  9 ,0},
#ifndef PACIFIST
    { "steal",      do_steal,       POSITION_STANDING,  1,  9 ,0},
    { "backstab",   do_backstab,    POSITION_STANDING,  0,  9 ,0},
    { "bs",         do_backstab,    POSITION_STANDING,  0,  9 ,0},
    { "kill",       do_kill,        POSITION_FIGHTING,  0,  9 ,0},
    { "bash",       do_bash,        POSITION_FIGHTING,  0,  9 ,0},
    { "hit",        do_hit,         POSITION_FIGHTING,  0,  9 ,0},
    { "assist",     do_assist,      POSITION_STANDING,  0,  9 ,0},
    { "kick",       do_kick,        POSITION_FIGHTING,  0,  9 ,0},
    { "murder",     do_murder,      POSITION_FIGHTING,  5,  9 ,0},
    { "rescue",     do_rescue,      POSITION_FIGHTING,  0,  9 ,0},
    { "trip",       do_trip,        POSITION_FIGHTING,  0,  9 ,0},
#endif
    { "ll",         do_stat_room,   POSITION_DEAD,     32,  76 ,0},
    { "llset",      do_ll_set,      POSITION_DEAD,     32,  9 ,0},
    { "meditate",   do_meditate,    POSITION_RESTING,   0,  9 ,0},
    { "meminfo",    do_meminfo,     POSITION_DEAD,     33,  9 ,1},
    { "mount",      do_mount,       POSITION_STANDING, 0,   9 ,0},
    { "unmount",    do_unmount,     POSITION_STANDING, 0,   9 ,0},
    { "dismount",   do_unmount,     POSITION_STANDING, 0,   9 ,0},
    { "camp",       do_camp,	    POSITION_STANDING, 3, 9 ,0},
    { "maparea",    do_areamap,	    POSITION_STANDING, 33, 9 ,0},
    { "listzones",  do_list_zones,  POSITION_DEAD,     0,  9 ,1},
    { "mudinfo",    do_mudinfo,     POSITION_DEAD,      0,  9 ,1},
    { "socials",    do_socials,     POSITION_DEAD,      0,  9 ,1},
    { "gods",       do_gods,        POSITION_DEAD,      33, 9 ,1},
    { "godsend",    do_godsend,     POSITION_DEAD,      33, 9 ,1},
    { "afk",        do_afk,         POSITION_DEAD,      0,  9 ,1},
    { "saverooms",  do_save_rooms,  POSITION_DEAD,      32, 9 ,0},
    { "setgod",     do_setgoddisplay,POSITION_DEAD,     31, 9 ,1},
    { "bug",        do_bug,         POSITION_DEAD,      0,  9 ,1},
    { "makeclan",   do_make_clan,   POSITION_DEAD,      35, 9 ,1},

    { "freport",    do_freport,     POSITION_DEAD,      0, 9 ,0},
    { "unform",     do_unform,      POSITION_DEAD,      0, 9 ,0},
    { "reform",     do_reform,      POSITION_DEAD,      0, 9 ,0},
    { "formation",  do_formation,   POSITION_DEAD,      0, 9 ,1},


    { "showclans",  do_showclans,   POSITION_DEAD,      0, 9 ,1},
    { "editroom",   do_editroom,    POSITION_RESTING,   32, 9 ,0},
    { "makeroom",   do_makeroom,    POSITION_DEAD,      33, 9 ,0},
    { "roomcopy",   do_roomcopy,    POSITION_DEAD,      33, 9 ,0},
    { "undordesc",  do_undordesc,   POSITION_DEAD,      32, 9 ,0},
    { "makeexit",   do_makedoor,    POSITION_DEAD,      33, 9 ,0},
    { "zrestrict",  do_zonerestrict,POSITION_DEAD,      35, 9 ,0},
    { "deleteexit", do_deletedoor,  POSITION_DEAD,      34, 9 ,0},
    { "closedown",  do_closedown,   POSITION_DEAD,      35, 9 ,1},
    { "grep",       do_grep,        POSITION_DEAD,      33, 9 ,1},
    { "openup",     do_openup,      POSITION_DEAD,      35, 9 ,1},
    { "editnotes",  do_editnotes,   POSITION_RESTING,   32, 9 ,0},
    { "odds",       do_not_here,    POSITION_RESTING,   0,  3674 ,0},
    { "pico",       do_pico,        POSITION_RESTING,   35, 9 ,0},
    { "reanimate",  do_reanimate,   POSITION_DEAD,      35, 9 ,1},
    { "savemobs",   do_save_mobs,   POSITION_DEAD,      35, 9 ,0},
    { "saveobjs",   do_save_objs,   POSITION_DEAD,      35, 9 ,0},
    { "zonestats",  do_zonestats,   POSITION_DEAD,      33, 9 ,1},
/* don't allow this any more
    { "togglehost", do_togglehost,  POSITION_DEAD,      35, 9 ,1},
 */
    { "push",       do_push,        POSITION_STANDING,   0, 9 ,0},
    { "password",   do_password,    POSITION_DEAD,       0, 9 ,0},
    { "formtest",   do_formtest,    POSITION_DEAD,       0, 9 ,0},
    { "gshow", 	    do_gshow,       POSITION_DEAD,      35, 9 ,0},
    { "hint",       do_hint,        POSITION_DEAD,      35, 9 ,0},
    { "tweak", 	    do_tweak,	    POSITION_DEAD,	35, 9 ,1},
    { "tweaktest",  do_tweaktest,   POSITION_DEAD, 	35, 9 ,1},
    { "wset",       do_wset,        POSITION_DEAD,      35, 9 ,1},
    { "mobexp",		do_Mobexp,		POSITION_DEAD,	35, 9 ,1},
    { "savechar",   do_savechar,    POSITION_DEAD,	35, 9 ,1},
    { "roomanal",   do_room_analyze,POSITION_DEAD,      35, 9 ,1},
    { "rehelp",     do_rebuild_help,POSITION_DEAD,	35, 9 ,1},
    { "rename",     do_rename,      POSITION_DEAD,      34, 9 ,1},
    { "approve",    do_approve,	    POSITION_DEAD,	34, 9 ,1},
    { "quake",      do_quake,       POSITION_DEAD,	35, 9 ,1},
    { "land",       do_land,        POSITION_RESTING,	0, 9 ,1},
    { "fly",        do_fly,         POSITION_RESTING,	0, 9 ,1},
    { "templog",    do_templog,     POSITION_DEAD,	35, 9 ,1},
    { "describe",   do_not_here,    POSITION_RESTING,	1, 39,0},
	{ "call",       do_CallDragon,  POSITION_STANDING,	1, 9,0},
    { "",           do_not_here,    POSITION_DEAD,       0, 9 }
};



char *fill[]=
{
    "in", "from", "with", "the", "on", "at", "to", "\n"
};



void command_interpreter( struct char_data *ch, char *pcomm )
{
    int look_at;
    int cmd;
    int i;
    char c;
    char alias[MAX_STRING_LENGTH],command[MAX_STRING_LENGTH],arg[MAX_STRING_LENGTH];
    char filename[MAX_STRING_LENGTH];
    char *dothispointer;
    char extra[MAX_STRING_LENGTH];
    long ct;
    char *tmstr;
    ct          = time(0);
    tmstr       = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';


    if(!GET_NAME(ch)||(strlen(GET_NAME(ch))<2))
	{
	log_hd("###Another player lost their name!");
	SUICIDE;
	}
#ifndef MEDTHIEVIA	// I don't need/want to debug it.
    if((int)pcomm>45000000){
	sprintf(log_buf,"##PCOMM mem address was %d decimal and 0x%x hex",(int)pcomm,(int)pcomm);
	log_hd(log_buf);
	return;
    }
    if((!pcomm) || (!pcomm[0]) )
		return;
#endif
    if((int)pcomm>high_mem)high_mem=(int)pcomm;
    set_to_lower(filename,pcomm);
    if(strstr(filename,"moogooboozoo"))return;/*used for noname mobs*/
    c=pcomm[0];
    if(c!=39&&c!=']'&&c!=';')
		strcpy(dothis,pcomm);
    else
		{
       	pcomm++;
		dothis[0]=c;dothis[1]=' ';dothis[2]=0;
		strcat(dothis,pcomm);
		}
    dothispointer=dothis;
    /*
     * Log players.
     */
    if ( ch->desc && IS_SET(ORIGINAL(ch)->specials.act, PLR_LOG) )
    {
	sprintf( log_buf, "%s::%s: %s\n", tmstr, 
		GET_NAME(ORIGINAL(ch)), dothispointer );
	fputs(log_buf,log_fh);
     }
    if(!IS_NPC(ch))
    if (ch->p->queryintcommand)
       if ((*ch->p->queryintfunc)(ch, ch->p->queryintcommand, dothispointer))
	  return;



    /*
     * No hiding.
     */
    REMOVE_BIT( ch->specials.affected_by, AFF_HIDE );


    /*
     * Implement freeze command.
     */
    if ( (!IS_NPC(ch) || IS_DEAD(ch)) 
	&& IS_SET(ORIGINAL(ch)->specials.act, PLR_FREEZE) )
    {
	send_to_char( "You're totally frozen!\n\r", ch );
	return;
    }
    if(!IS_NPC(ch)&&stuck_in_water_reaper(ch)){
	send_to_char("You are STUCK in a water reaper!  
Type: help water reaper   when your ride is done.\n\r",ch);
	return;
    }
    half_chop(dothispointer, arg, extra);
    if(!IS_NPC(ch))
    for(i=0;i<5;i++)
	if(ch->p->alias[i][0]){
	    half_chop(&ch->p->alias[i][0],alias,command);
	    if(!strcmp(arg,alias)){
		if(extra[0]){
		    strcat(command," ");
 		    strcat(command,extra);
		}
		strcpy(dothis, command);
		break;
	    }
	}else
	    break;

    /*
     * Strip initial spaces and parse command word.
     * Translate to lower case.
     */
    while ( *dothispointer == ' ' )
	dothispointer++;
    
    for ( look_at = 0; dothispointer[look_at] > ' '; look_at++ )
	dothispointer[look_at]  = LOWER(dothispointer[look_at]);

    if ( look_at == 0 )
	return;

    
    /*
     * Look for command in command table.
     */
    for ( cmd = 0; cmd < sizeof(cmd_info)/sizeof(cmd_info[0]); cmd++ )
    {
	if ( GET_LEVEL(ch) < cmd_info[cmd].minimum_level )
	    continue;
	if ( cmd_info[cmd].command_pointer == NULL )
	    continue;
	if ( memcmp( dothispointer, cmd_info[cmd].command_name, look_at ) == 0 )
	    goto LCmdFound;
    }

    /*
     * Look for command in socials table.
     */
    if ( check_social( ch, dothispointer, look_at, &dothispointer[look_at] ) )
	return;


    /*
     * Unknown command (or char too low level).
     */
    send_to_char( "What?!?\n\r", ch );
    return;

 LCmdFound:
    /*
     * Character not in position for command?
     */
	if(!IS_NPC(ch)&&IS_FLYING(ch)&&!cmd_info[cmd].fly){
		send_to_char("You reconsider and pay attention before you fall off.\n\r",ch);
		return;
	}

    if ( GET_POS(ch) < cmd_info[cmd].minimum_position )
    {
	switch( GET_POS(ch) )
	{
	case POSITION_DEAD:
	    send_to_char( "Lie still; you are DEAD.\n\r", ch );
	    break;
	case POSITION_INCAP:
	case POSITION_MORTALLYW:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;
	case POSITION_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;
	case POSITION_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;
	case POSITION_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;
	case POSITION_SITTING:
	    send_to_char( "Maybe you should stand up first?\n\r", ch);
	    break;
	case POSITION_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;
	}
	return;
    }

    /*
     * We're gonna execute it.
     * First look for usable special procedure.
     */
    if ( special( ch, cmd_info[cmd].command_number, &dothispointer[look_at]) )
	return;
      
    /*
     * Normal dispatch.
     */
    (*cmd_info[cmd].command_pointer)
	(ch, &dothispointer[look_at], cmd_info[cmd].command_number);

     
    /*
     * This call is here to prevent gcc from tail-chaining the
     * previous call, which screws up the debugger call stack.
     */
    number( 0, 0 );

    if (!guy_deleted){
		{
    if(!GET_NAME(ch)||(strlen(GET_NAME(ch))<2))
    {
	log_hd("###Another player lost their name! (tail interp).");
	SUICIDE;
    }

    if((ch->in_room>0)&&(GET_ZONE(ch)==198)&&(cmd_info[cmd].command_number>8)) 
	communicating(ch,&dothispointer[look_at]); 
    guy_deleted=FALSE;
		}
    }
}



int search_block(char *arg, char **list, bool exact)
{
    register int i,l;

    /* Make into lower case, and get length of string */
    for(l=0; *(arg+l); l++)
	*(arg+l)=LOWER(*(arg+l));

    if (exact) {
	for(i=0; **(list+i) != '\n'; i++)
	    if (!strcmp(arg, *(list+i)))
		return(i);
    } else {
	if (!l)
	    l=1; /* Avoid "" to match the first available string */
	for(i=0; **(list+i) != '\n'; i++)
	    if (!strncmp(arg, *(list+i), l))
		return(i);
    }

    return(-1);
}


int old_search_block(char *argument,int begin,int length,char **list,int mode)
{
    int guess, found, search;
	
    /* If the word contain 0 letters, then a match is already found */
    found = (length < 1);

    guess = 0;

    /* Search for a match */

    if(mode)
    while ( !found && *(list[guess]) != '\n' )
    {
	found = (length==strlen(list[guess]));
	for ( search = 0; search < length && found; search++ )
	    found=(*(argument+begin+search)== *(list[guess]+search));
	guess++;
    } else {
	while ( !found && *(list[guess]) != '\n' ) {
	    found=1;
	    for(search=0;( search < length && found );search++)
		found=(*(argument+begin+search)== *(list[guess]+search));
	    guess++;
	}
    }

    return ( found ? guess : -1 ); 
}



void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
    int look_at, found, begin;

    found = begin = 0;

    do
    {
	/* Find first non blank */
	for ( ;*(argument + begin ) == ' ' ; begin++);

	/* Find length of first word */
	for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

		/* Make all letters lower case,
		   and copy them to first_arg */
		*(first_arg + look_at) =
		LOWER(*(argument + begin + look_at));

	*(first_arg + look_at)='\0';
	begin += look_at;

    }
    while( fill_word(first_arg));

    do
    {
	/* Find first non blank */
	for ( ;*(argument + begin ) == ' ' ; begin++);

	/* Find length of first word */
	for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

		/* Make all letters lower case,
		   and copy them to second_arg */
		*(second_arg + look_at) =
		LOWER(*(argument + begin + look_at));

	*(second_arg + look_at)='\0';
	begin += look_at;
    }
    while( fill_word(second_arg));
}



int is_number(char *str)
{
    int look_at;

    if(*str=='\0')
	return(0);

    for(look_at=0;*(str+look_at) != '\0';look_at++)
	if((*(str+look_at)<'0')||(*(str+look_at)>'9'))
	    return(0);
    return(1);
}


/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg                      */
char *one_argument(char *argument, char *first_arg )
{
    int found, begin, look_at;

	found = begin = 0;
        if(!argument){
	    log_hd("######no argument in one_argument(interp.c)###");
	    return(NULL);
        }
	do
	{
		/* Find first non blank */
		for ( ;isspace(*(argument + begin)); begin++);

		/* Find length of first word */
		for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

			/* Make all letters lower case,
			   and copy them to first_arg */
			*(first_arg + look_at) =
			LOWER(*(argument + begin + look_at));

		*(first_arg + look_at)='\0';
	begin += look_at;
    }
	while (fill_word(first_arg));

    return(argument+begin);
}
/*same as one_argument but leaves case intact */
char *one_arg(char *argument, char *first_arg )
{
    int found, begin, look_at;

	found = begin = 0;
        if(!argument){
	    log_hd("######no argument in one_argument(interp.c)###");
	    return(NULL);
        }
		/* Find first non blank */
	for ( ;isspace(*(argument + begin)); begin++);

		/* Find length of first word */
	for(look_at=0;(*(argument+begin+look_at))&&(*(argument+begin+look_at)>' ');look_at++)
	    *(first_arg+look_at)=(*(argument+begin+look_at));

	*(first_arg+look_at)='\0';
	begin+=look_at;

        for(;isspace(*(argument+begin));begin++);
        return(argument+begin);
}
    


int fill_word(char *argument)
{
    return ( search_block(argument,fill,TRUE) >= 0);
}



/* determine if a given string is an abbreviation of another */
int is_abbrev(char *arg1, char *arg2)
{
    if (!*arg1)
       return(0);

    for (; *arg1; arg1++, arg2++)
       if (LOWER(*arg1) != LOWER(*arg2))
	  return(0);

    return(1);
}




/* return first 'word' plus trailing substring of input string */
void half_chop(char *string, char *arg1, char *arg2)
{
    for (; isspace(*string); string++);

    for (; !isspace(*arg1 = *string) && *string; string++, arg1++);

    *arg1 = '\0';

    for (; isspace(*string); string++);

    for (; ( *arg2 = *string ) != '\0'; string++, arg2++)
	;
}



int special(struct char_data *ch, int cmd, char *arg)
{
    register struct obj_data *i=NULL;
    register struct char_data *k=NULL;
    int j;

    /* special in room? */
    if (world[ch->in_room]->funct)
       if ((*world[ch->in_room]->funct)(ch, cmd, arg))
	  return(1);

    /* special in equipment list? */
    for (j = 0; j <= (MAX_WEAR - 1); j++)
       if (ch->equipment[j] && ch->equipment[j]->item_number>=0)
	  if (obj_index[ch->equipment[j]->item_number].func)
	     if ((*obj_index[ch->equipment[j]->item_number].func)
		(ch, cmd, arg))
		return(1);

    /* special in inventory? */
    for (i = ch->carrying; i; i = i->next_content)
	if (i->item_number>=0)
	    if (obj_index[i->item_number].func)
	   if ((*obj_index[i->item_number].func)(ch, cmd, arg))
	      return(1);

    /* special in mobile present? */
    for (k = world[ch->in_room]->people; k; k = k->next_in_room)
       if ( IS_MOB(k) )
	  if (mob_index[k->nr].func)
	     if ((*mob_index[k->nr].func)(ch, cmd, arg))
		return(1);

    /* special in object present? */
    for (i = world[ch->in_room]->contents; i; i = i->next_content)
       if (i->item_number>=0)
	  if (obj_index[i->item_number].func)
	     if ((*obj_index[i->item_number].func)(ch, cmd, arg))
		return(1);


    return(0);
}



void do_wizhelp(struct char_data *ch, char *argument, int cmd_arg)
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int no;

    if (IS_NPC(ch))
	return;

    send_to_char(
	"The following privileged comands are available:\n\r\n\r", ch);

    buf[0] = '\0';
    for ( cmd = 0, no = 0; cmd_info[cmd].command_name[0] != '\0'; cmd++ )
    {
	if ( cmd_info[cmd].minimum_level <= 30 )
	    continue;
	if ( cmd_info[cmd].minimum_level > GET_LEVEL(ch) )
	    continue;

	sprintf( buf + strlen(buf), "%-12s", cmd_info[cmd].command_name );
	if ( no % 6 == 0 )
	    strcat(buf, "\n\r");
	no++;
    }

    strcat(buf, "\n\r");
    page_string(ch->desc, buf, 1);
}

int number_of_players()
{
struct char_data *i=NULL;
int players=0;

for (i = character_list;  i ; i = i->next){
   if(!IS_NPC(i))
         players++;
   }
return(players);
}

void do_mudinfo(struct char_data *ch, char *argument, int cmd)
{
int commands;
struct char_data *i;
int mob1_5,mob6_10,mob11_15,mob16_20,mob21_25,mob26_30,mob31_35;
int level;
int number_mobs;
int players=0;
int objects=0;
struct obj_data *k;

	for (k = object_list; k; k = k->next)
		objects++;

 	number_mobs=mob1_5=mob6_10=mob11_15=mob16_20=mob21_25=mob26_30=mob31_35=0;
	for (i = character_list;  i ; i = i->next){
	    if(IS_NPC(i)){
		number_mobs++;
		level=GET_LEVEL(i);
		if(level>=1&&level<=5)mob1_5++;		
		if(level>=6&&level<=10)mob6_10++;		
		if(level>=11&&level<=15)mob11_15++;		
		if(level>=16&&level<=20)mob16_20++;		
		if(level>=21&&level<=25)mob21_25++;		
		if(level>=26&&level<=30)mob26_30++;		
		if(level>=31&&level<=35)mob31_35++;		

	    }else
		players++;
	    
	}
                /* count the commands */
    for(commands=0;commands<sizeof(cmd_info)/sizeof(cmd_info[0]);commands++);
	global_color=32;
	send_to_char("MEDIEVIA ONLINE version 4.01\n\r",ch);
    sprintf(log_buf,"Medievia has [%d] rooms and [%d] zones.\n\r",number_of_rooms+4000000,1+number_of_zones); 
	global_color=33;
    send_to_char(log_buf,ch); 
    sprintf(log_buf,"Medievia has [%d] Commands and [%d] Socials.\n\r",commands, number_socials); 
	send_to_char(log_buf,ch);
	global_color=32;
	sprintf(log_buf,"Medievia has [%d] total Objects loaded now.\n\r",objects);
	send_to_char(log_buf,ch);
	sprintf(log_buf,"Medievia has [%d] total Mobiles loaded now.\n\r",number_mobs);
	send_to_char(log_buf,ch);
	global_color=1;
	send_to_char("Mobile LEVELS breakdown is as follows..\n\r",ch);
	global_color=35;
	sprintf(log_buf,"1-5[%d],6-10[%d],11-15[%d],16-20[%d],21-25[%d],26-30[%d],31-35[%d]\n\r", mob1_5,mob6_10,mob11_15,mob16_20,mob21_25,mob26_30,mob31_35);
	send_to_char(log_buf,ch);
	global_color=33;
	sprintf(log_buf,"There are [%d] Players online now.\n\r",players);
	send_to_char(log_buf,ch);
	sprintf(log_buf,"Medievia has had [%ld] people connect since Jan 1 1994.\n\r",connect_count);
	send_to_char(log_buf,ch);
	global_color=0;
}
