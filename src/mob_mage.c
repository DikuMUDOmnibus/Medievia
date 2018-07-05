/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
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
#include <stdio.h> 
#include <string.h> 
#include <memory.h> 
#include <ctype.h> 
#include <time.h> 
#include "structs.h"
#include "mob.h" 
#include "obj.h" 
#include "utils.h" 
#include "db.h"
#include "handler.h" 
#include "limits.h" 
#include "spells.h"
#include "interp.h"


/*                  /External variables and functions\                    */
/*------------------------------------------------------------------------*/
extern int port;
extern struct room_data *world[MAX_ROOM];  
extern struct char_data *mobs[MAX_MOB];  
extern struct obj_data *objs[MAX_OBJ];  
extern int top_of_world;  
extern struct obj_data *object_list;  
extern struct char_data *character_list;  
extern struct zone_data zone_table_array[MAX_ZONE];  
extern struct zone_data *zone_table;  
extern int top_of_zone_table;  
extern struct index_data mob_index_array[MAX_MOB]; 
extern struct index_data *mob_index;  
extern struct index_data obj_index_array[MAX_OBJ];  
extern struct index_data *obj_index;  
extern int top_of_mobt;  
extern int top_of_objt; 
extern struct descriptor_data *descriptor_list;  
extern int dice(int number, int size);  
extern int number(int from, int to); 
extern void space_to_underline(char *text);
extern void page_string(struct descriptor_data *d, char *str, int keep_internal); 
extern int number_of_rooms;
extern char free_error[100];
extern bool is_formed(struct char_data *ch);
extern void cast_fireshield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_stone_skin( byte level, struct char_data *ch, char *arg,int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_fly( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_shield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_mass_levitation( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_mass_invis( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_fireball( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_frost_shards( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_colour_spray( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_weaken( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_lightning_bolt( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_shockwave( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_plague( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_magic_missile( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_cont_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern struct char_data *pick_victim(struct char_data *ch);


void mage_combat(struct char_data *ch)
{
struct char_data *m=NULL;
int which;

    if(!ch->specials.fighting)return;
    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)&&GET_LEVEL(ch)>=9&&GET_MANA(ch)>=10){
      m=pick_victim(ch->specials.fighting);
      if(!m)return;
      if((GET_LEVEL(ch)>(GET_LEVEL(m)+1))&&GET_LEVEL(ch)>=21&&GET_MANA(ch)>24&&!IS_AFFECTED(m,AFF_PLAGUE)){
        which = number(1,4);
          if(which == 1){
		act("$n chants the magical phrase, 'Vas Nox'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=25;
		if(number(1,100)>(80+GET_INT(ch)))return;
		cast_plague(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
          }
          else{
		act("$n chants the magical phrase, 'Jux Grav Hur'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=10;
		if(number(1,100)>(80+GET_INT(ch)))return;
		cast_lightning_bolt(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
          }
      }
      else{
		act("$n chants the magical phrase, 'Jux Grav Hur'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=10;
		if(number(1,100)>(80+GET_INT(ch)))return;
		cast_lightning_bolt(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;

      }
    }
    if(GET_LEVEL(ch)>=25&&GET_LEVEL(ch)<38&&GET_MANA(ch)>=25){
      m=pick_victim(ch->specials.fighting);
      if(!m)return;
      which = number(1,6);
      if((GET_LEVEL(ch)>(GET_LEVEL(m)+1))&&!IS_AFFECTED(m,AFF_PLAGUE)&&which<3){
	act("$n chants the magical phrase, 'Vas Nox'",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch)-=25;
	if(number(1,100)>(80+GET_INT(ch)))return;
        cast_plague(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	return;
      }
      else if(which == 3){
        if(!IS_AFFECTED(m, AFF_BLIND)){
          act("$n chants the magical phrase, 'Ort Hur'",TRUE,ch,0,0,TO_ROOM);
	  GET_MANA(ch)-=15;
	  if(number(1,100)>(80+GET_INT(ch)))return;
	  cast_colour_spray(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	  return;
        }
        else if(!affected_by_spell(m, SPELL_WEAKEN)){
	  act("$n chants the magical phrase, 'Des Por'",TRUE,ch,0,0,TO_ROOM);
	  GET_MANA(ch)-=15;
	  if(number(1,100)>(80+GET_INT(ch)))return;
	  cast_weaken(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	  return;
        }
        else{
	  act("$n chants the magical phrase, 'Jux Ort'",TRUE,ch,0,0,TO_ROOM);
	  GET_MANA(ch)-=15;
	  if(number(1,100)>(80+GET_INT(ch)))return;
	  cast_curse(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	  return;
        }
      }
      else{
	act("$n chants the magical phrase, 'Vas Corp Hur'",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch)-=25;
	if(number(1,100)>(80+GET_INT(ch)))return;
        cast_shockwave(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	return;
      }
    }
    if(GET_LEVEL(ch)>=21&&GET_MANA(ch)>=25){
      m=pick_victim(ch->specials.fighting);
      if(!m)return;
      if(GET_LEVEL(ch)>(GET_LEVEL(m)+1)&&!IS_AFFECTED(m,AFF_PLAGUE)){
        which = number(1,5);
          if(which > 2){
            if(IS_SET(world[ch->in_room]->room_flags, INDOORS)){
		act("$n chants the magical phrase, 'Por An Flam'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_frost_shards(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
            else{
		act("$n chants the magical phrase, 'Por Flam'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_fireball(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
          }
          else if(which == 2){
            if(!IS_AFFECTED(m, AFF_BLIND)){
		act("$n chants the magical phrase, 'Ort Hur'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_colour_spray(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
            else if(!affected_by_spell(m, SPELL_WEAKEN)){
		act("$n chants the magical phrase, 'Des Por'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_weaken(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
            else{
		act("$n chants the magical phrase, 'Jux Ort'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_curse(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
          }
          else{
		act("$n chants the magical phrase, 'Vas Nox'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=25;
		if(number(1,100)>(80+GET_INT(ch)))return;
		cast_plague(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
          }
      }
    }
    if(GET_LEVEL(ch)>=16&&GET_MANA(ch)>=16){
      m=pick_victim(ch->specials.fighting);
      if(!m)return;
        which = number(1,3);
          if(which > 1){
            if(IS_SET(world[ch->in_room]->room_flags, INDOORS)){
		act("$n chants the magical phrase, 'Por An Flam'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=16;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_frost_shards(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
            else{
		act("$n chants the magical phrase, 'Por Flam'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_fireball(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
          }
          else{
            if(!IS_AFFECTED(m, AFF_BLIND)){
		act("$n chants the magical phrase, 'Ort Hur'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_colour_spray(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
            else if(!affected_by_spell(m, SPELL_WEAKEN)){
		act("$n chants the magical phrase, 'Des Por'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_weaken(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
            else{
		act("$n chants the magical phrase, 'Jux Ort'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_curse(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
          }
    }
    if(GET_LEVEL(ch)>=15&&GET_MANA(ch)>=15){
      m=pick_victim(ch->specials.fighting);
      if(!m)return;
        which = number(1,3);
          if(which > 1){
            if(IS_SET(world[ch->in_room]->room_flags, INDOORS)){
		act("$n chants the magical phrase, 'Ort Hur'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_colour_spray(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
            else{
		act("$n chants the magical phrase, 'Por Flam'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_fireball(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
          }
          else{
            if(!IS_AFFECTED(m, AFF_BLIND)){
		act("$n chants the magical phrase, 'Ort Hur'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_colour_spray(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
            else if(!affected_by_spell(m, SPELL_WEAKEN)){
		act("$n chants the magical phrase, 'Des Por'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_weaken(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
            else{
		act("$n chants the magical phrase, 'Jux Ort'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=15;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_curse(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
            }
          }
    }
    if(GET_LEVEL(ch)>=9&&GET_MANA(ch)>=10){
		m=pick_victim(ch->specials.fighting);
		if(!m)return;
		act("$n chants the magical phrase, 'Jux Grav Hur'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=10;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_lightning_bolt(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
		return;
    }
    if(GET_MANA(ch)>=5){
		m=pick_victim(ch->specials.fighting);
		if(!m)return;
		act("$n chants the magical phrase, 'Ort Jux'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=5;
		if(number(1,100)>(80+GET_INT(ch)))return;
	        cast_magic_missile(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	        return;
    }

}

void mage_noncombat(struct char_data *ch)
{

    if(GET_LEVEL(ch)>=18&&!affected_by_spell(ch, SPELL_STONE_SKIN)){
	act("$n chants the magical phrase, 'Ort Sanct Ylem'",TRUE,ch,0,0,TO_ROOM);
	cast_stone_skin(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
    if(GET_LEVEL(ch)>=7&&!IS_AFFECTED(ch, AFF_FLYING)){
	act("$n chants the magical phrase, 'Uus Por'",TRUE,ch,0,0,TO_ROOM);
	cast_fly(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
    if(GET_LEVEL(ch)>=2&&!IS_AFFECTED(ch, AFF_DETECT_INVISIBLE)){
 	act("$n chants the magical phrase, 'Wis Quas Lor'",TRUE,ch,0,0,TO_ROOM);
	cast_detect_invisibility(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
    if(GET_LEVEL(ch)>=2&&!IS_AFFECTED(ch, AFF_INVISIBLE)){
 	act("$n chants the magical phrase, 'Quas Lor'",TRUE,ch,0,0,TO_ROOM);
	cast_invisibility(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
    if(!ch->equipment[WEAR_LIGHT]&&GET_LEVEL(ch)>=4){
 	act("$n chants the magical phrase, 'In Lor'",TRUE,ch,0,0,TO_ROOM);
	cast_cont_light(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	do_grab(ch,"ball",9);
	return;
    }
    if(is_formed(ch)){
	if(number(1,100)<35)
		if(GET_LEVEL(ch)>=15){
		    act("$n chants the magical phrase, 'Vas Uus Por'",TRUE,ch,0,0,TO_ROOM);
		    cast_mass_levitation(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,0,0);
		    return;
		}
	if(number(1,100)<35)
	        if(GET_LEVEL(ch)>=10){
		     	act("$n chants the magical phrase, 'Vas Quas Lor'",TRUE,ch,0,0,TO_ROOM);
		    cast_mass_invis(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,0,0);
		    return;
		}
    }
    if(GET_LEVEL(ch)>=13&&!affected_by_spell(ch, SPELL_SHIELD)){
 	act("$n chants the magical phrase, 'Ort Grav'",TRUE,ch,0,0,TO_ROOM);
	cast_shield(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
    if(GET_LEVEL(ch)>=16&&!IS_AFFECTED(ch, AFF_FIRESHIELD)){
 	act("$n chants the magical phrase, 'Flam Ort Grav'",TRUE,ch,0,0,TO_ROOM);
	cast_fireshield(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
}
