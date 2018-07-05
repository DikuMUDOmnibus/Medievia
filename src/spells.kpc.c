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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "db.h"
#include "interp.h" 
#include "spells.h"
#include "handler.h"

#define MANA_MU 1
#define MANA_CL 1

#define SPELLO(nr, beat, pos, mlev, clev, mana, tar, func) { \
    spell_info[nr].spell_pointer = (func);    \
    spell_info[nr].beats = (beat);            \
    spell_info[nr].minimum_position = (pos);  \
    spell_info[nr].min_usesmana = (mana);     \
    spell_info[nr].min_level_cleric = (clev); \
    spell_info[nr].min_level_magic = (mlev);  \
    spell_info[nr].targets = (tar);           \
}

/* Global data */
extern char global_color;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct char_data *character_list;
extern char *spell_wear_off_msg[];
extern struct str_app_type str_app[];
extern char free_error[100];


/* Extern procedures */


/* Extern procedures */
extern bool in_a_shop(struct char_data *ch);

void cast_armor( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_teleport( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_bless( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_burning_hands( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_call_lightning( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_person( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_chill_touch( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shocking_grasp( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_clone( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_colour_spray( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_control_weather( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_food( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_water( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_blind( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_critic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_magic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_earthquake( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_energy_drain( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fireball( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_harm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_lightning_bolt( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_chain_lightning( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_locate_object( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_magic_missile( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_protection_from_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sleep( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_strength( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_summon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_word_of_recall( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sense_life( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_identify( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fear( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_refresh( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fly( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cont_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_animate_dead( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_know_alignment( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_magic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_conjure_elemental( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_serious( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_critical( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_serious( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flamestrike( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_stone_skin( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_weaken( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mass_invis( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_acid_blast( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_gate( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_faerie_fire( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_faerie_fog( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_drown( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_demonfire( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_turn_undead( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_infravision( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sandstorm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_hands_of_wind( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_plague( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fireshield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_transport( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mass_levitation( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sense_death( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_breath_water( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shield_room( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_map_catacombs( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_farsight( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_ethereal( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shockwave( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_scribe( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_bloodbath( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_resurrect( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

struct spell_info_type spell_info[MAX_SPL_LIST];

char *spells[]=
{
   "armor",               /* 1 */
   "teleport",
   "bless",
   "blindness",
   "burning hands",
   "call lightning",
   "charm person",
   "chill touch",
   "clone",  
   "colour spray",
   "control weather",     /* 11 */
   "create food",
   "create water",
   "cure blind",
   "cure critic",
   "cure light",
   "curse",
   "detect evil",
   "detect invisibility",
   "detect magic",
   "detect poison",       /* 21 */
   "dispel evil",
   "earthquake",
   "enchant weapon",
   "energy drain",
   "fireball",
   "harm",
   "heal",
   "invisibility",
   "lightning bolt",
   "locate object",      /* 31 */
   "magic missile",
   "poison",
   "protection from evil",
   "remove curse",
   "sanctuary",
   "shocking grasp",
   "sleep",
   "strength",
   "summon",
   "ventriloquate",      /* 41 */
   "word of recall",
   "remove poison",
   "sense life",         /* 44 */

   /* RESERVED SKILLS */
   "SKILL_SNEAK",        /* 45 */
   "SKILL_HIDE",
   "SKILL_STEAL",
   "SKILL_BACKSTAB",
   "SKILL_PICK_LOCK",
   "SKILL_KICK",         /* 50 */
   "SKILL_BASH",
   "SKILL_RESCUE",
   /* NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) */

   "identify",           /* 53 */
   "animate dead",
   "fear",        
   "fly",
   "continual light",
   "know alignment",
   "dispel magic",
   "conjure elemental",  /* 60 */
   "cure serious",
   "cause light",
   "cause critical",
   "cause serious",
   "flamestrike",        /* 65 */
   "stone skin",
   "shield",
   "weaken",
   "mass invisibility",
   "acid blast",         /* 70 */
   "gate",
   "faerie fire",
   "faerie fog",
   "drown",
   "demonfire",          /* 75 */
   "turn undead",
   "infravision",
   "sandstorm",
   "hands of wind",
   "plague",             /* 80 */
   "refresh",
   "fireshield",
   "transport",
   "mass levitation",
   "sense death",	 /* 85 */
   "breathe water",
   "shield room",
   "chain lightning",
   "map catacombs",
   "farsight",		 /* 90 */
   "ethereal",
   "shockwave",
   "scribe",
   "bloodbath",
   "resurrect",		 /* 95 */
   "\n"
};


const byte saving_throws[4][5][36] = {
{
  {16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 4, 4, 4, 0},
  {13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0},
  {15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 3, 3, 3, 3, 1, 1, 1, 1, 0},
  {20,20,20,20,20,18,18,18,18,18,16,16,16,16,16,14,14,14,14,14,12,12,12,12,12,10,10,10,9,9,9,2,2,2,2,0},
  {19,19,19,19,19,16,16,16,16,16,13,13,13,13,13,9,9,9,9,9,8,8,8,8,8,7,7,7,6,6,6,6,2,2,2,0}
}, {
  {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
  {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 0},
  {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 0},
  {20,20,20,20,18,18,18,18,16,16,16,16,14,14,14,14,12,12,12,12,10,10,10,10,8,8,8,8,7,7,7,2,2,2,2,0},
  {19,19,19,19,17,17,17,17,15,15,15,15,13,13,13,13,10,10,10,10,9,9,9,9,8,8,8,8,7,7,7,7,0,0,0,0}
}, {
  {15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 0},
  {16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 0},
  {14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 3, 3, 0},
  {20,20,20,20,18,18,18,18,16,16,16,16,14,14,14,14,12,12,12,12,10,10,10,10,8,8,8,8,7,7,7,2,2,2,2,0},
  {19,19,19,19,17,17,17,17,15,15,15,15,13,13,13,13,10,10,10,10,9,9,9,9,8,8,8,8,7,7,7,7,0,0,0,0}
}, {
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0},
  {17,15,15,14,14,12,12,11,11,10,10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0},
  {19,19,19,17,17,17,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10,9,9,9,8,8,8,6,6,6,6,2,2,2,2,0},
  {20,20,20,18,18,18,17,17,17,16,16,16,15,15,15,14,14,14,13,13,13,12,12,12,11,11,11,9,9,9,9,9,2,2,2,0}
}
};


/*
int use_mana( struct char_data *ch, int sn )
{
    int base = spell_info[sn].min_usesmana;
    int divisor;

    divisor = 3 + GET_LEVEL(ch);
    if ( GET_CLASS(ch) == CLASS_CLERIC )
	divisor -= spell_info[sn].min_level_cleric;
    else
	divisor -= spell_info[sn].min_level_magic;

    if ( divisor != 0 )
	return MAX( base, 100 / divisor );
    else
	return MAX( base, 20 );
}
*/


int use_mana( struct char_data *ch, int sn )
{
int base = spell_info[sn].min_usesmana;
int divisor;
 
divisor = 2 + GET_LEVEL(ch);
 
/* CLERIC ONLY SPELL
 * If spell min level of mage = 32 it means spell is cleric only
 * In this case there is only one min level to consider.
 * Since routine is called it means the guy is/was cleric 
 */
if(spell_info[sn].min_level_magic == 32)
	divisor -= spell_info[sn].min_level_cleric;

 
/* MAGE ONLY SPELL
 * Same reasoning
 */
else if(spell_info[sn].min_level_cleric == 32)
	divisor -= spell_info[sn].min_level_magic;


/* MAGE/CLERIC SPELL
 * Use cleric level for ACTUAL cleric and mage value for ACTUAL mage
 * Use Worse value for actual thief or warrior
 */
else if ( GET_CLASS(ch) == CLASS_CLERIC )
	divisor -= spell_info[sn].min_level_cleric;
else if ( GET_CLASS(ch) == CLASS_MAGIC_USER )
	divisor -= spell_info[sn].min_level_magic;
else 
	divisor -= MAX(spell_info[sn].min_level_cleric,
		       spell_info[sn].min_level_magic);

 
/* calculation of mana cost */
if ( divisor < 2 )
	return MAX( base, 50 );
else
	return MAX( base, 100 / divisor );
}


void affect_update( void )
{
static struct affected_type *af, *next_af_dude;
static struct char_data *i;
struct char_data *i_next;
struct obj_data *weapon;
int type;

for (i = character_list; i; i = i_next)
	{
	i_next = i->next;
	for (af = i->affected; af; af = next_af_dude) 
		{
	    next_af_dude = af->next;
	    if (af->duration >= 1)
	  		af->duration--;
	    else if (af->duration == -1)
	  		af->duration = -1;  /* GODS only!  unlimited */
	    else 
			{
	        type=af->type;
			affect_remove(i, af);
			if(type>0&&type<MAX_SPL_LIST&&!affected_by_spell( i,type))
			{
	  	    if (*spell_wear_off_msg[type] && spell_wear_off_msg[type][0]!='!') 
				{
	    		send_to_char(spell_wear_off_msg[type], i);
	    		send_to_char("\n\r", i);
	 	    	}
			}
	    	}
		}
	if ( i->equipment[WIELD]
		&& (GET_OBJ_WEIGHT(i->equipment[WIELD])
			> str_app[STRENGTH_APPLY_INDEX(i)].wield_w)
		)
		{
		weapon = unequip_char(i, WIELD);
		obj_to_char(weapon, i);
		act("You feel sapped of strength and drop $p.",
       		FALSE, i, weapon, 0, TO_CHAR);
		act("$n is too weak to hold $p and drops it.",
   	         FALSE, i, weapon, 0, TO_ROOM);
		}
	}

 }


void say_spell( struct char_data *ch, int si )
{
    char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    int j, offs;
    struct char_data *temp_char=NULL;


    struct syllable {
	char org[10];
	char new[10];
    };

    struct syllable syls[] = {
    { " ", " " },
    { "ar", "abra"   },
    { "au", "kada"    },
    { "bless", "fido" },
  { "blind", "nose" },
  { "bur", "mosa" },
    { "cu", "judi" },
    { "de", "oculo"},
    { "en", "unso" },
    { "light", "dies" },
    { "lo", "hi" },
    { "mor", "zak" },
    { "move", "sido" },
  { "ness", "lacri" },
  { "ning", "illa" },
    { "per", "duda" },
    { "ra", "gru"   },
  { "re", "candus" },
    { "son", "sabru" },
  { "tect", "infra" },
    { "tri", "cula" },
    { "ven", "nofo" },
    {"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
    {"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
    {"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
    {"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
    };



    strcpy(buf, "");
    strcpy(splwd, spells[si-1]);

    offs = 0;

    while(*(splwd+offs)) {
	for(j=0; *(syls[j].org); j++)
	    if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) {
		strcat(buf, syls[j].new);
		if (strlen(syls[j].org))
		    offs+=strlen(syls[j].org);
		else
		    ++offs;
	    }
    }


    sprintf(buf2,"$n utters the words, '%s'", buf);
    sprintf(buf, "$n utters the words, '%s'", spells[si-1]);
    global_color=1;
    for(temp_char = world[ch->in_room]->people;
	temp_char;
	temp_char = temp_char->next_in_room)
	if(temp_char != ch) {
	    if (GET_CLASS(ch) == GET_CLASS(temp_char))
		act(buf, FALSE, ch, 0, temp_char, TO_VICT);
	    else
		act(buf2, FALSE, ch, 0, temp_char, TO_VICT);

	}
    global_color=0;
}



bool saves_spell(struct char_data *ch, sh_int save_type)
{
    int save;

    /* Negative apply_saving_throw makes saving throw better! */

    save = ch->specials.apply_saving_throw[save_type];

    if (!IS_NPC(ch)) {
	save += saving_throws[GET_CLASS(ch)-1][save_type][(int) GET_LEVEL(ch)];
	/*if (GET_LEVEL(ch) > 31)
	    return(TRUE);*/
    }
	if (IS_NPC(ch)){
		if(GET_CLASS(ch)==CLASS_OTHER)
			if(GET_LEVEL(ch)>34)
				save+=saving_throws[CLASS_WARRIOR][save_type][35];
			else
				save+=saving_throws[CLASS_WARRIOR][save_type][(int) GET_LEVEL(ch)];
		else
			if(GET_LEVEL(ch)>34)
				save+=saving_throws[GET_CLASS(ch)-1][save_type][35];
			else
				save+=saving_throws[GET_CLASS(ch)-1][save_type][(int) GET_LEVEL(ch)];
	}

    return(MAX(1,save) < number(1,20));
}



char *skip_spaces(char *string)
{
    for(;*string && (*string)==' ';string++);

    return(string);
}



/* Assumes that *argument does start with first letter of chopped string */

void do_cast(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *tar_obj=NULL;
  struct char_data *tar_char=NULL;
  char name[MAX_INPUT_LENGTH], buf[50],spell[MAX_INPUT_LENGTH],flag;
  int  spl, i,words,x;
  bool target_ok;

  if (IS_NPC(ch))
    return;
  
  if (GET_LEVEL(ch)<32&&GET_CLASS(ch)!=CLASS_CLERIC&&GET_CLASS(ch)!=CLASS_MAGIC_USER) {
      if(!IS_SET(ch->player.multi_class,MULTI_CLASS_MAGIC_USER)&&
 	 !IS_SET(ch->player.multi_class,MULTI_CLASS_CLERIC)){
		send_to_char("Stick to fighting!\n\r",ch);
		return;
	}
  }

  argument = skip_spaces(argument);
  
  /* If there is no chars in argument */
  if (!(*argument)) {
    send_to_char("Cast which what where? Syntax: cast spell [target] no {'}'s needed\n\r", ch);
    return;
  }
  x=0;
  while(argument[x]){
      	if(argument[x++]=='\''){
	    send_to_char("No quotes needed in Medievia, parser will figure it out.\n\r",ch);
	    return;
      	}
  }

  if(in_a_shop(ch)){
    send_to_char("Somehow this room seems to block all magic!\n\r",ch);
    return;
  }

  /*count words*/
  words=1;
  for(x=0;argument[x];x++)
	if(argument[x]==' '){
	     while(argument[x]==' ')x++;
	     if(argument[x])
		words++;
	}  
  
  if(words==1){
	/* one word, must be spell, no target given*/
	set_to_lower(&spell[0],argument);
	name[0]=MED_NULL;
  }else if(words==2){
	/* 2 words, first check to see if both words make a spell*/
	set_to_lower(&spell[0],argument);
	spl = old_search_block(spell, 0, strlen(spell),spells, 0);
	if(((spl>0&&spl<=44)||spl>=53)&&spell_info[spl].spell_pointer){
	    /*both 2 words made a spell, go with it, no target*/
	    name[0]=MED_NULL;
	}else{
	    /*both words didn't, second must be a target*/
	    half_chop(argument,buf,name);
	    set_to_lower(&spell[0],buf);
	}
  }else{
	/*if >=3 words last word must be target, 1&2 must be spell*/
	argument=one_argument(argument,buf);
	strcat(buf," ");
	argument=one_argument(argument,spell);
	strcat(buf,spell);
        argument = skip_spaces(argument);
	strcpy(name,argument);
	set_to_lower(&spell[0],buf);
  }
  spl = old_search_block(spell, 0, strlen(spell),spells, 0);
  
  if (!spl) {
    send_to_char("Your lips do not move, no magic appears.\n\r",ch);
    return;
  }

  if ( ((spl > 0 && spl <= 44) || spl >= 53) && spell_info[spl].spell_pointer )
  {
    if (GET_POS(ch) < spell_info[spl].minimum_position) {
      switch(GET_POS(ch)) {
      case POSITION_SLEEPING :
    send_to_char("You dream about great magical powers.\n\r", ch);
    break;
      case POSITION_RESTING :
    send_to_char("You can't concentrate enough while resting.\n\r",ch);
    break;
      case POSITION_SITTING :
    send_to_char("You can't do this sitting!\n\r", ch);
    break;
      case POSITION_FIGHTING :
    send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
    break;
      default:
    send_to_char("It seems like you're in a pretty bad shape!\n\r",ch);
    break;
      } /* Switch */
    }   else {
     
 
    flag=0;
    if (GET_LEVEL(ch) < 32) {
    if ((GET_CLASS(ch) == CLASS_MAGIC_USER
	||IS_SET(ch->player.multi_class,MULTI_CLASS_MAGIC_USER)) &&
	(spell_info[spl].min_level_magic <= GET_LEVEL(ch))) {
	flag=1;
    }
    if ((GET_CLASS(ch) == CLASS_CLERIC
	||IS_SET(ch->player.multi_class,MULTI_CLASS_CLERIC)) &&
	(spell_info[spl].min_level_cleric <= GET_LEVEL(ch))) {
      flag=1;
    }
    }
    if(!flag&&GET_LEVEL(ch)<32){
      send_to_char("Sorry, you can't do that.\n\r", ch);
      return;
    }
      
      /* **************** Locate targets **************** */
      
      target_ok = FALSE;
      tar_char = 0;
      tar_obj = 0;
      
      if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {


    if (*name) {
      if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
	if ( ( tar_char = get_char_room_vis(ch, name) ) != NULL )
	  target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	if ( ( tar_char = get_char_vis(ch, name) ) != NULL )
	  target_ok = TRUE;
      
      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	if ( ( tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)) != NULL )
	  target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
      {
	tar_obj = get_obj_in_list_vis( ch, name, world[ch->in_room]->contents );
	if ( tar_obj != NULL )
	  target_ok = TRUE;
      }

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	if ( ( tar_obj = get_obj_vis(ch, name) ) != NULL )
	  target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
	for(i=0; i<MAX_WEAR && !target_ok; i++)
	  if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
	tar_obj = ch->equipment[i];
	target_ok = TRUE;
	  }
      }

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
	if (str_cmp(GET_NAME(ch), name) == 0) {
	  tar_char = ch;
	  target_ok = TRUE;
	}
      
    } else { /* No argument was typed */
      
      if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
	if (ch->specials.fighting) {
	  tar_char = ch;
	  target_ok = TRUE;
	}

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
	if (ch->specials.fighting) {
	  /* WARNING, MAKE INTO POINTER */
	  tar_char = ch->specials.fighting;
	  target_ok = TRUE;
	}

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	tar_char = ch;
	target_ok = TRUE;
      }
      
    }

      } else {
    target_ok = TRUE; /* No target, is a good target */
      }

      if (!target_ok) {
    if (*name) {
      if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
	send_to_char("Nobody here by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	send_to_char("Nobody playing by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	send_to_char("You are not carrying anything like that.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	send_to_char("Nothing here by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	send_to_char("Nothing at all by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
	send_to_char("You are not wearing anything like that.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	send_to_char("Nothing at all by that name.\n\r", ch);
      
    } else { /* Nothing was given as argument */
      if (spell_info[spl].targets < TAR_OBJ_INV)
	send_to_char("Whom should the spell be cast upon?\n\r", ch);
      else
	send_to_char("What should the spell be cast upon?\n\r", ch);
    }
    return;
      } else { /* TARGET IS OK */
    if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
      send_to_char("You can not cast this spell upon yourself.\n\r", ch);
      return;
    }
    else if ((tar_char != ch) && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
      send_to_char("You can only cast this spell upon yourself.\n\r", ch);
      return;
    } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
      send_to_char("You are afraid that it could harm your master.\n\r", ch);
      return;
    }
      }
      
      if (GET_LEVEL(ch) < 32) {
    if (GET_MANA(ch) < use_mana(ch, spl)) {
      send_to_char("You can't summon enough energy to cast the spell.\n\r", ch);
      return;
    }
      }
      
      if (spl != SPELL_VENTRILOQUATE)  /* :-) */
    say_spell(ch, spl);
      
      if(GET_LEVEL(ch)<35)
         WAIT_STATE(ch, spell_info[spl].beats);
      
      if ((spell_info[spl].spell_pointer == 0) && spl>0)
    send_to_char("Sorry, this magic has not yet been implemented :(\n\r", ch);
      else {
    if (number(1,101) > ch->skills[spl].learned) { /* 101% is failure */
      send_to_char("You lost your concentration!\n\r", ch);
      GET_MANA(ch) -= (use_mana(ch, spl)>>1);
      return;
    }
    sprintf(buf,"You cast the spell, using %d mana.\n\r",(use_mana(ch, spl)));
    global_color=1;
    send_to_char(buf,ch);
    global_color=0;

    if(tar_char&&IS_UNDEAD(tar_char))
	{
	send_to_char("You don't wanna waste mana on a corpse!!!\n\r",ch);
	return;
	}
    if (!IS_SET(spell_info[spl].targets, TAR_IGNORE))
    ((*spell_info[spl].spell_pointer) (GET_LEVEL(ch), ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj));
    else
    ((*spell_info[spl].spell_pointer) (GET_LEVEL(ch), ch, name, SPELL_TYPE_SPELL, tar_char, tar_obj));

    GET_MANA(ch) -= (use_mana(ch, spl));
      }
      
    }   /* if GET_POS < min_pos */
    
    return;
  }

  switch (number(1,5)){
  case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
  case 2: send_to_char("Olle Bolle Snop Snyf?\n\r",ch); break;
  case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch); break;
  case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch); break;
  default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch); break;
  }
}


void assign_spell_pointers(void)
{
    int i;

    for(i=0; i<MAX_SPL_LIST; i++)
	spell_info[i].spell_pointer = 0;


    /* From spells1.c */

    SPELLO(32,18,POSITION_FIGHTING, 1, 32, 5,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_magic_missile);

    SPELLO( 8,18,POSITION_FIGHTING, 3, 32, 15,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_chill_touch);

    SPELLO( 5,18,POSITION_FIGHTING, 5, 32, 15,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_burning_hands);

    SPELLO(37,18,POSITION_FIGHTING, 7, 32, 15,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_shocking_grasp);

    SPELLO(30,18,POSITION_FIGHTING, 9, 32, 10,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_lightning_bolt);

    SPELLO(88,18,POSITION_FIGHTING, 13, 32, 22,
     TAR_IGNORE, cast_chain_lightning);

    SPELLO(89,6,POSITION_STANDING, 10, 28, 50,
     TAR_IGNORE, cast_map_catacombs);

    SPELLO(90,6,POSITION_STANDING, 17, 32, 15,
     TAR_IGNORE, cast_farsight);

    SPELLO(91,6,POSITION_STANDING, 19, 32, 20,
     TAR_IGNORE, cast_ethereal);

    SPELLO(92,15,POSITION_FIGHTING, 25, 32, 23,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_shockwave);

    SPELLO(93,6,POSITION_STANDING, 22, 32, 50,
     TAR_IGNORE, cast_scribe);

    SPELLO(94,6,POSITION_STANDING, 32, 26, 50,
     TAR_IGNORE, cast_bloodbath);

    SPELLO(95,6,POSITION_STANDING, 32, 27, 150,
     TAR_IGNORE, cast_resurrect);

    SPELLO(10,18,POSITION_FIGHTING, 11, 32, 15,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_colour_spray);

    SPELLO(25,18,POSITION_FIGHTING, 13, 32, 35,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_energy_drain);

    SPELLO(26,18,POSITION_FIGHTING, 15, 32, 12,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_fireball);

    SPELLO(23,18,POSITION_FIGHTING, 32, 7, 18,
     TAR_IGNORE, cast_earthquake);

    SPELLO(22,18,POSITION_FIGHTING, 32, 10, 15,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_evil);

    SPELLO( 6,18,POSITION_FIGHTING, 32, 12, 20,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_call_lightning);

    SPELLO(27,18,POSITION_FIGHTING, 32, 18, 24,
     TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_harm);



    /* Spells2.c */

    SPELLO( 1,18,POSITION_STANDING, 5,  1, 5,
     TAR_CHAR_ROOM, cast_armor);

    SPELLO( 2,18,POSITION_FIGHTING, 8, 32, 35,
     TAR_SELF_ONLY, cast_teleport);

    SPELLO( 3,18,POSITION_STANDING,32,  5, 5,
     TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, cast_bless);

    SPELLO( 4,18,POSITION_FIGHTING, 8,  6, 5,
     TAR_CHAR_ROOM, cast_blindness);

    SPELLO(7,18,POSITION_STANDING, 14, 32, 5,
     TAR_CHAR_ROOM | TAR_SELF_NONO, cast_charm_person);

/*  SPELLO( 9,18,POSITION_STANDING,15, 32, 40,
     TAR_CHAR_ROOM, cast_clone);    */

    SPELLO(11,18,POSITION_STANDING,10, 13, 25,
     TAR_IGNORE, cast_control_weather);

    SPELLO(12,18,POSITION_STANDING,32,  3, 5,
     TAR_IGNORE, cast_create_food);

    SPELLO(13,18,POSITION_STANDING,32,  2, 5,
     TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_create_water);

    SPELLO(14,18,POSITION_STANDING,32,  4, 5,
     TAR_CHAR_ROOM, cast_cure_blind);

    SPELLO(15,18,POSITION_FIGHTING,32,  9, 20,
     TAR_CHAR_ROOM, cast_cure_critic);

    SPELLO(16,18,POSITION_FIGHTING,32,  1, 15,
     TAR_CHAR_ROOM, cast_cure_light);

    SPELLO(17,18,POSITION_STANDING,12, 32, 20,
     TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_curse);

    SPELLO(18,18,POSITION_STANDING,32,  4, 5,
     TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_detect_evil);

    SPELLO(19,18,POSITION_STANDING, 2,  5, 5,
     TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_detect_invisibility);

    SPELLO(20,18,POSITION_STANDING, 2,  3, 5,
     TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_detect_magic);

    SPELLO(21,18,POSITION_STANDING,32,  2, 5,
     TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_detect_poison);

    SPELLO(24,36,POSITION_STANDING,12, 32, 100,
     TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_weapon);

    SPELLO(28,18,POSITION_FIGHTING,32, 14, 50,
     TAR_CHAR_ROOM, cast_heal);

    SPELLO(29,18,POSITION_STANDING, 4, 32, 5,
     TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP, cast_invisibility);

    SPELLO(31,27,POSITION_STANDING, 6, 10, 20,
     TAR_OBJ_WORLD, cast_locate_object);

    SPELLO(33,18,POSITION_STANDING,32,  8, 10,
     TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_poison);

    SPELLO(34,18,POSITION_STANDING,32,  6, 5,
     TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_protection_from_evil);

    SPELLO(35,18,POSITION_STANDING,32, 12, 5,
     TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, cast_remove_curse);

    SPELLO(36,18,POSITION_STANDING,32, 13, 75,
     TAR_CHAR_ROOM, cast_sanctuary);

    SPELLO(38,18,POSITION_STANDING,3, 32, 15,
     TAR_CHAR_ROOM, cast_sleep);

    SPELLO(39,18,POSITION_STANDING, 7, 32, 20,
     TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_strength);

    SPELLO(40,18,POSITION_STANDING,32,  8, 50,
     TAR_CHAR_WORLD, cast_summon);

    SPELLO(41,18,POSITION_STANDING, 1, 32, 5,
     TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast_ventriloquate);

    SPELLO(42,18,POSITION_RESTING,32, 11, 5,
     TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_word_of_recall);

    SPELLO(43,18,POSITION_STANDING,32,  9, 5,
     TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_remove_poison);

    SPELLO(44,18,POSITION_STANDING,32,  7, 5,
     TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_sense_life);

    SPELLO(45,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
    SPELLO(46,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
    SPELLO(47,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
    SPELLO(48,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
    SPELLO(49,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
    SPELLO(50,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
    SPELLO(51,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);
    SPELLO(52,0,POSITION_STANDING,25,25,200, TAR_IGNORE, 0);

    SPELLO(53,32,POSITION_STANDING,10,10,12,
	TAR_CHAR_ROOM | TAR_OBJ_INV, cast_identify);

    /*  New Spells  -Kahn*/

#if 0
    SPELLO(54,32,POSITION_STANDING,32, 6, 12, TAR_OBJ_ROOM,
	   cast_animate_dead);
#endif

    SPELLO(55,18,POSITION_FIGHTING, 6, 32, 7, TAR_CHAR_ROOM,
	   cast_fear);

    SPELLO(56,24,POSITION_STANDING, 7, 12, 10, TAR_CHAR_ROOM,
	   cast_fly);

    SPELLO(86,24,POSITION_STANDING, 10, 12, 10, TAR_CHAR_ROOM,
	   cast_breath_water);

    SPELLO(57,18,POSITION_STANDING, 4, 2, 7, TAR_IGNORE,
	   cast_cont_light);

    SPELLO(58,18,POSITION_STANDING, 8, 5, 9, TAR_CHAR_ROOM,
	   cast_know_alignment);

    SPELLO(59,18,POSITION_FIGHTING, 11, 16, 15, TAR_CHAR_ROOM
	   | TAR_OBJ_ROOM, cast_dispel_magic);

    SPELLO(60,30,POSITION_STANDING, 9, 32, 33, TAR_IGNORE,
	   cast_conjure_elemental);

    SPELLO(61,25,POSITION_FIGHTING, 32, 5, 17, TAR_CHAR_ROOM,
	   cast_cure_serious);

    SPELLO(62,18,POSITION_FIGHTING, 32, 1, 15, TAR_CHAR_ROOM,
	   cast_cause_light);

    SPELLO(63,18,POSITION_FIGHTING, 32, 9, 20, TAR_CHAR_ROOM,
	   cast_cause_critical);

    SPELLO(64,18,POSITION_FIGHTING, 32, 5, 17, TAR_CHAR_ROOM,
	   cast_cause_serious);

    SPELLO(65,18,POSITION_FIGHTING, 32, 13, 20, TAR_CHAR_ROOM,
	   cast_flamestrike);

    SPELLO(66,24,POSITION_STANDING, 18, 32, 12, TAR_CHAR_ROOM|TAR_SELF_ONLY,
	   cast_stone_skin);

    SPELLO(67,24,POSITION_STANDING, 13, 32, 12, TAR_CHAR_ROOM|TAR_SELF_ONLY,
	   cast_shield);

    SPELLO(68,18,POSITION_FIGHTING, 7, 32, 20, TAR_CHAR_ROOM,
	   cast_weaken);

    SPELLO(69,30,POSITION_STANDING, 15, 17, 20, TAR_IGNORE,
	   cast_mass_invis);

    SPELLO(70,18,POSITION_FIGHTING, 20, 32, 16, TAR_IGNORE,
	   cast_acid_blast);

    SPELLO(71,40,POSITION_STANDING, 26, 19, 275, TAR_IGNORE,
	   cast_gate);


    SPELLO(72,18,POSITION_FIGHTING, 4, 2, 6, TAR_CHAR_ROOM,
	   cast_faerie_fire);

    SPELLO(73,18,POSITION_STANDING, 10, 14, 12, TAR_IGNORE,
	   cast_faerie_fog);

    SPELLO(74,18,POSITION_FIGHTING, 32, 18, 15, TAR_CHAR_ROOM,
	   cast_drown);

    SPELLO(75,18,POSITION_FIGHTING, 32, 25, 30, TAR_CHAR_ROOM,
	   cast_demonfire);

    SPELLO(76,18,POSITION_FIGHTING, 32, 15, 12, TAR_CHAR_ROOM,
	   cast_turn_undead);

    SPELLO(77,24,POSITION_STANDING, 6, 9, 5, TAR_CHAR_ROOM,
	   cast_infravision);

    SPELLO(78,18,POSITION_FIGHTING, 20, 20, 25, TAR_IGNORE,
	   cast_sandstorm);

    SPELLO(79,35,POSITION_STANDING, 32, 15, 70, TAR_CHAR_ROOM,
	   cast_hands_of_wind);

    SPELLO(80,18,POSITION_FIGHTING, 21, 32, 25, TAR_CHAR_ROOM,
	   cast_plague);

    SPELLO(81,24,POSITION_STANDING, 5, 3, 12, TAR_CHAR_ROOM,
	   cast_refresh);

    SPELLO(82,18,POSITION_STANDING, 16, 22, 45, TAR_CHAR_ROOM|TAR_SELF_ONLY,
	   cast_fireshield);

    SPELLO(83,26,POSITION_FIGHTING,32, 16, 50,
     TAR_IGNORE, cast_transport);

    SPELLO(84,21,POSITION_STANDING,10, 15, 30,
     TAR_IGNORE, cast_mass_levitation);

    SPELLO(85,18,POSITION_STANDING,12, 5, 7,
     TAR_IGNORE, cast_sense_death);

    SPELLO(87,28,POSITION_STANDING, 11, 32, 75, TAR_IGNORE,
	   cast_shield_room); 
} 


/* SPELLO(nr, beat, pos, mlev, clev, mana,tar, func) */
