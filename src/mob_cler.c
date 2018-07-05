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
extern void cast_cause_serious( byte level, struct char_data *ch, char *arg,int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_fly( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_harm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_armor( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_demonfire( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_hammer_of_faith( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_mass_levitation( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_mass_invis( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_flamestrike( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_call_lightning( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_cure_critic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_magic_missile( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_cont_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern struct char_data *pick_victim(struct char_data *ch);
extern void cast_dispel_magic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_heal( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
extern void cast_bless(byte, struct char_data *, char *, int, struct char_data *, struct obj_data *);

void cleric_combat(struct char_data *ch)
{
struct char_data *m=NULL;

    if(!ch->specials.fighting)return;

	/* Nasty clerics dispelling their victims!!
	   They should be ashamed of themselves*/

	if(GET_LEVEL(ch)>=16&&GET_MANA(ch)>=15){
		m=pick_victim(ch->specials.fighting);
		if(!m)return;
		if(GET_LEVEL(ch)>=GET_LEVEL(m)
			&&(IS_AFFECTED(m, AFF_SANCTUARY)
			||IS_AFFECTED(m, AFF_FIRESHIELD))){
			act("$n chants the magical phrase, 'An Ort'", TRUE,ch,0,0,TO_ROOM);
			GET_MANA(ch)-=15;
			if(number(1,130)<100)return;
			cast_dispel_magic(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
			return;
		}
	}

	if(GET_LEVEL(ch)>=14&&GET_MANA(ch)>50
		&&((GET_HIT(ch)<(.3*GET_MAX_HIT(ch))))){
		act("$n chants the magical phrase, 'Vas Mani'",TRUE,ch,0,0,TO_ROOM);
		GET_MANA(ch)-=50;
		cast_heal(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	}
    if(GET_LEVEL(ch)>=25&&GET_MANA(ch)>=31){
      m=pick_victim(ch->specials.fighting);
      if(!m)return;
      if((GET_ALIGNMENT(ch)>349&&GET_ALIGNMENT(m)<349)||
        (GET_ALIGNMENT(ch)<(-349)&&GET_ALIGNMENT(m)>349)||
        ((GET_ALIGNMENT(ch)>(-350)&&GET_ALIGNMENT(ch)<350)&&
        (GET_ALIGNMENT(m)>349||GET_ALIGNMENT(m)<(-349)))){
          act("$n chants the magical phrase, 'In Vas Grav'", TRUE,ch,0,0,TO_ROOM);
          GET_MANA(ch)-=31;
          if(number(1,100)>(80+GET_WIS(ch)))return;
          cast_hammer_of_faith(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
          return;
      }
    }
    if(GET_LEVEL(ch)>=23&&GET_MANA(ch)>=27&&GET_ALIGNMENT(ch)<(-349)){
	m=pick_victim(ch->specials.fighting);
	if(!m)return;
	act("$n chants the magical phrase, 'Kal Corp Flam'",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch)-=27;
        if(number(1,100)>(80+GET_WIS(ch)))return;
        cast_demonfire(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	return;
    }
    if(GET_LEVEL(ch)>=20&&GET_MANA(ch)>=20&&OUTSIDE(ch)&&
      (weather_info.sky>=SKY_RAINING)){
	m=pick_victim(ch->specials.fighting);
	if(!m)return;
	act("$n chants the magical phrase, 'Kal Jux Grav'",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch)-=20;
        if(number(1,100)>(80+GET_WIS(ch)))return;
        cast_call_lightning(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	return;
    }
    if(GET_LEVEL(ch)>=10&&GET_MANA(ch)>=15){
	m=pick_victim(ch->specials.fighting);
	if(!m)return;
      if(GET_ALIGNMENT(m)<(-349)){
	act("$n chants the magical phrase, 'An Jux'",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch)-=15;
        if(number(1,100)>(80+GET_WIS(ch)))return;
        cast_dispel_evil(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	return;
      }
    }
    if(GET_LEVEL(ch)>=12&&GET_MANA(ch)>=20&&OUTSIDE(ch)&&
      (weather_info.sky>=SKY_RAINING)){
	m=pick_victim(ch->specials.fighting);
	if(!m)return;
	act("$n chants the magical phrase, 'Kal Jux Grav'",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch)-=20;
        if(number(1,100)>(80+GET_WIS(ch)))return;
        cast_call_lightning(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	return;
    }
    if(GET_LEVEL(ch)>=15&&GET_MANA(ch)>=30){
	m=pick_victim(ch->specials.fighting);
	if(!m)return;
	act("$n chants the magical phrase, 'In Corp'",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch)-=30;
        if(number(1,100)>(80+GET_WIS(ch)))return;
    cast_harm(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	return;
    }
    if(GET_LEVEL(ch)>=11&&GET_MANA(ch)>=20){
	m=pick_victim(ch->specials.fighting);
	if(!m)return;
	act("$n chants the magical phrase, 'Kal Flam'",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch)-=20;
        if(number(1,100)>(80+GET_WIS(ch)))return;
    cast_flamestrike(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	return;
    }
    if(GET_MANA(ch)>=17){
	m=pick_victim(ch->specials.fighting);
	if(!m)return;
	act("$n chants the magical phrase, 'An Mani'",TRUE,ch,0,0,TO_ROOM);
	GET_MANA(ch)-=17;
        if(number(1,100)>(80+GET_WIS(ch)))return;
    cast_cause_serious(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,m,0);
	return;
    }

}

void cleric_noncombat(struct char_data *ch)
{

    if(GET_LEVEL(ch)>=12&&!IS_AFFECTED(ch, AFF_FLYING)){
	act("$n chants the magical phrase, 'Uus Por'",TRUE,ch,0,0,TO_ROOM);
	cast_fly(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
    if(GET_LEVEL(ch)>=5&&!IS_AFFECTED(ch, AFF_DETECT_INVISIBLE)){
	act("$n chants the magical phrase, 'Wis Quas Lor'",TRUE,ch,0,0,TO_ROOM);
	cast_detect_invisibility(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
    if(!ch->equipment[WEAR_LIGHT]&&GET_LEVEL(ch)>=2){
	act("$n chants the magical phrase, 'In Lor'",TRUE,ch,0,0,TO_ROOM);
	cast_cont_light(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	do_grab(ch,"ball",9);
	return;
    }
	if(GET_LEVEL(ch)>=5&&!IS_AFFECTED(ch, SPELL_BLESS)){
		act("$n chants the magical phrase, 'Sanct Ort'",TRUE,ch,0,0,TO_ROOM);
		cast_bless(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
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
	        if(GET_LEVEL(ch)>=17){
		    act("$n chants the magical phrase, 'Vas Quas Lor'",TRUE,ch,0,0,TO_ROOM);
		    cast_mass_invis(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,0,0);
		    return;
		}
    }
    if(!affected_by_spell(ch, SPELL_ARMOR)){
	act("$n chants the magical phrase, 'Bet Sanct'",TRUE,ch,0,0,TO_ROOM);
	cast_armor(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
    if(GET_HIT(ch)<GET_MAX_HIT(ch)&&GET_LEVEL(ch)>9){
	act("$n chants the magical phrase, 'Kal Mani'",TRUE,ch,0,0,TO_ROOM);
	cast_cure_critic(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
    if(GET_LEVEL(ch)>=22&&!IS_AFFECTED(ch, AFF_FIRESHIELD)){
	act("$n chants the magical phrase, 'Flam Ort Grav'",TRUE,ch,0,0,TO_ROOM);
	cast_fireshield(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	return;
    }
}
