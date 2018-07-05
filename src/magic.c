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
#include <string.h>
#include <stdio.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "interp.h"
#include "db.h"
#include "holocode.h"

/* Extern structures */
extern void add_object(struct obj_data *obj);
extern void fall(struct char_data *ch);
extern char global_color;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;
extern char MOUNTMOVE;
#define STATE(d)    ((d)->connected)

/* Extern procedures */
extern int amount_blood_in_room(int room);
extern void remove_blood(int room, int blood);
int is_allowed_here(struct char_data *ch, int to_room);
extern bool put_in_formation(struct char_data *leader, struct char_data *follower);
bool saves_spell(struct char_data *ch, sh_int spell);
extern bool is_formed(struct char_data *ch);
int dice(int number, int size);
void check_killer(struct char_data *ch, struct char_data *victim);
void update_pos( struct char_data *victim );
extern void die_formation(struct char_data *ch);
extern int gate_room;
extern int gate_dir;
extern char NODEATHTRAP;
extern const char *szaDetStatusMessages[];
/* Offensive Spells */

void spell_magic_missile(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int dice;
	int counter;
	int dice_roll;

  if( victim && ch && level >= 1 && level <= 50 ){
  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );

	dice=(1+((GET_LEVEL(ch)/5)*2));
	dice_roll=GET_LEVEL(ch);
	for(counter = 1; counter <= dice; counter ++)
		dice_roll+=number(1,8);

	dam=dice_roll;

  if ( saves_spell(victim, SAVING_SPELL) )
	dam*=.69;

  if(damage(ch, victim, dam, SPELL_MAGIC_MISSILE))
	return;
  }
}

void spell_chill_touch(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  int dam;
	int dice_roll;
	int counter;
	int dice;

  if( victim && ch && level >= 1 && level <= 50 ){

  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );

	dice = 3;
	dice_roll=0;
	for(counter=1;counter<=dice;counter++)
		dice_roll+=number(1,6);
	dam = GET_LEVEL(ch)+dice_roll;

  if ( !saves_spell(victim, SAVING_SPELL) ){
    af.type      = SPELL_CHILL_TOUCH;
    af.duration  = 6;
    af.modifier  = -1;
    af.location  = APPLY_STR;
    af.bitvector = 0;
    affect_join(victim, &af, TRUE, TRUE);

    act("You feel a painful stiffening in your muscles!",FALSE,ch,0,victim,TO_VICT);
    act("$N tenses for a moment, and moans in agony.",FALSE,ch,0,victim,TO_CHAR);
    act("$N tenses for a moment, and moans in agony.",FALSE,ch,0,victim,TO_NOTVICT);
  } 

  if(damage(ch, victim, dam, SPELL_CHILL_TOUCH))
	return;
  }
}

void spell_burning_hands(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int dice_roll;
	int counter;
	int dice;

  if( victim && ch && level >= 1 && level <= 50 ){

  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );

	dice_roll=0;
	dice = 5;
	for(counter = 1;counter <= dice; counter++)
		dice_roll+=number(1,6);
	dam = GET_LEVEL(ch)+dice_roll;

  if ( saves_spell(victim, SAVING_SPELL) )
	dam=dam*.69;

  if(damage(ch, victim, dam, SPELL_BURNING_HANDS))
	return;
  }
}

void spell_shocking_grasp(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int counter;
	int dice_roll;
	int dice;

  if( victim && ch && level >= 1 && level <= 50 ){

  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );

	dice=6;
	dice_roll=GET_LEVEL(ch);
	for(counter = 1; counter <= dice; counter++)
		dice_roll+=number(1,7);

	dam=dice_roll;

  if ( saves_spell(victim, SAVING_SPELL) )
	dam=dam*.69;

  if(damage(ch, victim, dam, SPELL_SHOCKING_GRASP))
	return;
  }
}

void spell_lightning_bolt(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	int dice;
	int dice_roll;
	int counter;

  if( victim && ch  ){

  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );

	dice=GET_LEVEL(ch);
	dice_roll=0;
	for(counter = 1; counter <= dice; counter++)
		dice_roll+=number(1,6);

	dam = dice_roll;

  if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING))
	dam*=2;
  if(number(1,20) == 20)
	dam=0;
  else
  	if ( saves_spell(victim, SAVING_SPELL) )
		dam=dam*.69;
	
  if(damage(ch, victim, dam, SPELL_LIGHTNING_BOLT))
	return;
  }
}

void spell_chain_lightning(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj) {
    int room;
    struct char_data *j = NULL;
    struct char_data *k = NULL;

    if(!ch)
	return;
    room=ch->in_room;
    if(room<1 || !world[room])
	return;

    j=world[room]->people;
    while (j && j->next_in_room) {
		/* k is a marker to make sure j->next wasn't
		killed and extracted while being bolted*/
		k = j->next_in_room; 
		if ( IS_NPC(k) && IS_NPC(k->master) &&!k->specials.stpMount)
			spell_lightning_bolt(level,ch,j->next_in_room,NULL);
		if(j->next_in_room != k)
			return; /* this means mob died so we should stop lighting */
		j = j->next_in_room;
    }
    if( (world[room]->people) 
	&& (IS_NPC(world[room]->people))
	&& (IS_NPC(world[room]->people->master))
	)
		spell_lightning_bolt(level,ch,world[room]->people,NULL);
}

void spell_colour_spray(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
	struct affected_type af;
	int dice;
	int counter;
	int dice_roll;

  if( victim && ch){

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer( ch, victim );

    dice=GET_LEVEL(ch);
    dice_roll=0;
    for(counter = 1; counter <= dice; counter++)
	dice_roll+=number(1,5);
    dam = dice_roll;

    if ( !saves_spell(victim, SAVING_SPELL) ){
	if (affected_by_spell(victim, SPELL_BLINDNESS))
	  affect_from_char(victim, SPELL_BLINDNESS);

	af.type      = SPELL_BLINDNESS;
	af.location  = APPLY_HITROLL;
	af.modifier  = -4;
	af.duration  = 1+level;
	af.bitvector = AFF_BLIND;
	affect_to_char(victim, &af);

	af.location = APPLY_AC;
	af.modifier = +40;
	affect_to_char(victim, &af);

	act("The bright burst of colors has blinded you!",FALSE,ch,0,victim,TO_VICT);
        act("$N turns $S eyes away and screams!",FALSE,ch,0,victim,TO_CHAR);
        act("$N turns $S eyes away and screams!",FALSE,ch,0,victim,TO_NOTVICT);
    }

  if(damage(ch, victim, dam, SPELL_COLOUR_SPRAY))
	return;
  }
}

void spell_energy_drain(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int move, mana;

  if( victim && ch ){

  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );

  if(!IS_EVIL(ch)){
    send_to_char("Your inner spirit lacks the strength and conviction needed.\n\r", ch);
    return;
  }
  if (!saves_spell(victim, SAVING_SPELL)){
    if (GET_LEVEL(victim) <= 2) {
      if(damage(ch, victim, 100, SPELL_ENERGY_DRAIN)) /* Kill the sucker */
		return;
    }
    else{
	    mana = dice(GET_LEVEL(ch), 8);
	    GET_MANA(victim) -= mana;
            GET_MANA(ch) += mana;
            move = dice(GET_LEVEL(ch), 4);
            GET_MOVE(ch) += move;
            if(!IS_NPC(victim))
              GET_MOVE(victim) -= move;

      send_to_char("Your life energy is drained!\n\r", victim);

      if(damage(ch, victim, 1, SPELL_ENERGY_DRAIN))
		return;
      }
  } else {
    if (damage(ch, victim, 0, SPELL_ENERGY_DRAIN)) /* Miss */
		return;
  }
  }
}

void spell_fireball(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct char_data *v;
  struct char_data *next_v;

  int dam, otherdam;
	int dice;
	int counter;
	int dice_roll;

  if( victim && ch ){
  if(ch->in_room<1)return;
  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );
  if(world[ch->in_room]->sector_type == SECT_UNDERWATER){
      send_to_char("The fireball fizzles away in the water..\n\r",ch);
      return;
  }

	dice = GET_LEVEL(ch)*2;
	dice_roll=0;
	for(counter=1; counter <= dice; counter++)
		dice_roll+=number(1,4);

	dam=dice_roll;

  if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING))
        dam/=2;
  otherdam = dam;
  if ( saves_spell(victim, SAVING_SPELL) )
	dam*=.69;
  act("You conjure a ball of magical flame and throw it at $N.", FALSE, ch, 0, victim, TO_CHAR);
  act("You are enveloped in flames from $n's fireball.", TRUE, ch, 0, victim, TO_VICT);
  act("$n conjures a ball of magical flame and hurls it at $N.", TRUE, ch, 0, victim, TO_ROOM);
  damage(ch, victim, dam, SPELL_FIREBALL);

  if(IS_SET(world[ch->in_room]->room_flags, INDOORS)){
    act("The area is filled with searing flames from $n's fireball.\n\r", FALSE, ch, 0, 0, TO_ROOM);
    for(v=world[ch->in_room]->people;v;v=next_v){
	      next_v = v->next_in_room;
    	  dam = otherdam * .5;
	      if(GET_LEVEL(v)<32 || (IS_NPC(v)&&!v->specials.stpMount)){
    	    	if(!IS_DEAD(v) && (v != victim)){
          			if(IS_NPC(v->master)){
            			if (saves_spell(v, SAVING_SPELL))
              				dam *= .69;
					    damage(ch, v, dam, SPELL_FIREBALL);
          			}else if((IS_SET(world[ch->in_room]->room_flags, NEUTRAL))||(IS_SET(world[ch->in_room]->room_flags, CHAOTIC))){
 			           if(saves_spell(v, SAVING_SPELL))
            		  		dam *= .69;
            			GET_HIT(v) -= dam;
            			if(GET_HIT(v)<1){
              				send_to_char("You burn to death in the maelstrom of magical flame.\n\r", v);
              				act("$n burns to death in the maelstrom of magical flame.\n\r", TRUE, v, 0, 0, TO_ROOM);
              				raw_kill(v,ch);
            			}
          			}else if(v->master==ch->master){
            			if(saves_spell(v, SAVING_SPELL))
              				dam *= .69;
            			GET_HIT(v) -= dam;
            			if(GET_HIT(v)<1){
              				send_to_char("You burn to death in the maelstrom of magical flame.\n\r", v);
              				act("$n burns to death in the maelstrom of magical flame.\n\r", TRUE, v, 0, 0, TO_ROOM);
              				raw_kill(v,ch);
            			}
          			}
        		}
      		}
    	}
  	}
  	return;
  }
}

void spell_shockwave(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  if( victim && ch ){
  if(ch->in_room<1)return;
  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );
  dam=number(100,250);
  if(world[ch->in_room]->sector_type == SECT_UNDERWATER)
     dam+=dam/2; 
  if ( saves_spell(victim, SAVING_SPELL) )
    dam*=.69;

  if(damage(ch, victim, dam, SPELL_SHOCKWAVE))
	return;
  }
}
void spell_frost_shards(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  if( victim && ch ){
  if(ch->in_room<1)return;
  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );
  dam=dice(GET_LEVEL(ch),8);
  if ( saves_spell(victim, SAVING_SPELL) )
    dam*=.69;

  if(damage(ch, victim, dam, SPELL_FROST_SHARDS))
	return;
  }
}

void spell_armor(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!victim)return;

  if (ch != victim)
    act("$N is protected by your deity.", FALSE, ch, 0, victim, TO_CHAR);
    act("$N is protected by $n's deity.", FALSE, ch, 0, victim, TO_NOTVICT);

  af.type      = SPELL_ARMOR;
  af.duration  = 24;
  af.modifier  = -30;
  af.location  = APPLY_AC;
  af.bitvector = 0;

  affect_join(victim, &af, TRUE, FALSE);
  send_to_char("You feel someone protecting you.\n\r", victim);
}

void spell_earthquake(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
int dam;
struct char_data *v=NULL;
struct char_data *next_v=NULL;

if( ch ){

	dam = dice(GET_LEVEL(ch),4)+GET_LEVEL(ch);
  send_to_char("The earth trembles beneath your feet!\n\r", ch);
  act("$n makes the earth tremble violently!",
      FALSE, ch, 0, 0, TO_ROOM);

 	for(v=world[ch->in_room]->people;v;v=next_v){
    	next_v = v->next_in_room;
    	if(IS_NPC(v)&&IS_NPC(v->master)&&(!IS_DEAD(v))){
			 if(IS_NPC(v)&&v->specials.stpMount)continue;
			if(damage(ch, v, dam, SPELL_EARTHQUAKE))
				continue;
			}else{
				fall(v);
			}
	}


  /*for(tmp_victim = character_list; tmp_victim; tmp_victim = temp)
  {
    temp = tmp_victim->next;
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)
      && !IS_DEAD(tmp_victim) && (IS_NPC(tmp_victim))){
		 if(IS_NPC(tmp_victim)&&tmp_victim->specials.stpMount)continue;
	    if(damage(ch, tmp_victim, dam, SPELL_EARTHQUAKE))
			continue;
		fall(tmp_victim);
    } else
      if (world[ch->in_room]->zone == world[tmp_victim->in_room]->zone)
	send_to_char("The earth trembles and shivers.\n\r", tmp_victim);
  }*/

  }
}

void spell_dispel_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  
  if( victim && ch ){

  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );

  if (IS_EVIL(ch))
    victim = ch;
  
  if (IS_GOOD(victim)) {
    act("The Gods protect $N.", FALSE, ch, 0, victim, TO_ROOM);
    spell_armor(level, ch, victim, 0);
    return;
  }

  if (IS_NEUTRAL(victim))
    {
      act("$N does not seem to be affected.", FALSE, ch, 0, victim, TO_CHAR);
      return;
    }

  if ((GET_LEVEL(victim) < level) || (victim == ch))
    dam = 100;
  else {
    dam = dice(level,6);

      if ( saves_spell(victim, SAVING_SPELL) )
      dam *= .69;
    }

  if(damage(ch, victim, dam, SPELL_DISPEL_EVIL))
	return;
  }
}

void spell_call_lightning(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  extern struct weather_data weather_info;

  if( victim && ch ){

  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );

	dam = dice(GET_LEVEL(ch), 7);
	dam += 2*GET_LEVEL(ch);

  if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {

    if ( saves_spell(victim, SAVING_SPELL) )
		dam*=.69;

    if(damage(ch, victim, dam, SPELL_CALL_LIGHTNING))
		return;
  }
  }
}

void spell_harm(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj){
  	int dam;

  	if( victim && ch ){

  	if(!if_allowed_to_attack(ch,victim))
		return;

  	check_killer( ch, victim );

	dam = level*4;
  
	if(level<GET_LEVEL(victim))
    	if ( saves_spell(victim, SAVING_SPELL) )
      		dam *= .69;

  	if(damage(ch, victim, dam, SPELL_HARM))
		return;
  }
}

void spell_teleport(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
    int to_room;
    extern int top_of_world;      /* ref to the top element of world */

    if(victim){

    if( IS_NPC(victim) && !IS_UNDEAD(ch) )
		return;
    if(world[victim->in_room]->zone==65){
		send_to_char("Doesn't seem to work from the ship.\n\r",victim);
		return;
    }
    if(world[victim->in_room]->zone==198){
		send_to_char("Doesn't seem to work this deep underground.\n\r",victim);
		return;
    }
    do {
		to_room = number(0, top_of_world);
    } while (!world[to_room]
		||IS_SET(world[to_room]->room_flags, PRIVATE)
		||world[to_room]->level_restriction>GET_LEVEL(victim)
		||zone_table[world[to_room]->zone].continent!=GET_CONTINENT(victim)
    	||IS_SET(world[to_room]->room_flags,GODPROOF)
		||world[to_room]->zone==65
		||world[to_room]->zone==198);
    act("$n slowly fades out of existence.", FALSE, victim,0,0,TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, to_room);
    act("$n slowly fades into existence.", FALSE, victim,0,0,TO_ROOM);

    do_look(victim, "", 0);
   }
}

void spell_bless(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(ch && (victim || obj)){

    if (obj) {
      if ( (5*GET_LEVEL(ch) > GET_OBJ_WEIGHT(obj)) &&
	(GET_POS(ch) != POSITION_FIGHTING) &&
	!IS_OBJ_STAT(obj, ITEM_EVIL)) {
	  SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
	  act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR);
      }
    } else {
      if(!victim)return;
      if (affected_by_spell(victim, SPELL_BLESS))
	affect_from_char(victim, SPELL_BLESS);

      send_to_char("You feel righteous.\n\r", victim);
      af.type      = SPELL_BLESS;
      af.duration  = 6+level;
      af.modifier  = 5;
      af.location  = APPLY_HITROLL;
      af.bitvector = 0;
      affect_to_char(victim, &af);
      
      af.location = APPLY_SAVING_SPELL;
      af.modifier = -1;                 /* Make better */
      affect_to_char(victim, &af);
    }
  }
}

void spell_blindness(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if( victim && ch ){

  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );

  if (saves_spell(victim, SAVING_SPELL)) {
      act("$N seems to be unaffected!", FALSE, ch, NULL, victim, TO_CHAR);
      one_hit (victim, ch, TYPE_UNDEFINED);
      return;
  }
  
  if (!affected_by_spell(victim, SPELL_BLINDNESS)) {
    act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You have been blinded!\n\r", victim);
  }
  else
    affect_from_char(victim, SPELL_BLINDNESS);

  af.type      = SPELL_BLINDNESS;
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;
  af.duration  = 1+level;
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);

  af.location = APPLY_AC;
  af.modifier = +40;
  affect_to_char(victim, &af);
  }
}

void spell_create_food(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  if(ch){

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = str_dup("mushroom food");
  tmp_obj->short_description = str_dup("A Magic Mushroom");
  tmp_obj->description = str_dup("A really delicious looking magic mushroom lies here.");

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[0] = 5+level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 0;
  tmp_obj->obj_flags.cost_per_day = 1;

  add_object(tmp_obj);

  obj_to_room(tmp_obj,ch->in_room);

  tmp_obj->item_number = 98;

  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
  }
}

void spell_create_water(byte level, struct char_data *ch,
	    struct char_data *victim, struct obj_data *obj)
{
  int water;

  extern struct weather_data weather_info;
  void name_to_drinkcon(struct obj_data *obj,int type);
  void name_from_drinkcon(struct obj_data *obj);

  if(ch && obj){

  if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
    if ((obj->obj_flags.value[2] != LIQ_WATER)
    && (obj->obj_flags.value[1] != 0)) {
      
      name_from_drinkcon(obj);
      obj->obj_flags.value[2] = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
      
    } else {

      water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);
      
      /* Calculate water it can contain, or water created */
      water = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], water);
      
      if (water > 0) {
    obj->obj_flags.value[2] = LIQ_WATER;
    obj->obj_flags.value[1] += water;



    name_from_drinkcon(obj);
    name_to_drinkcon(obj, LIQ_WATER);
    act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
      }
    }
  }
  }
}

void spell_cure_blind(byte level, struct char_data *ch,
	      struct char_data *victim, struct obj_data *obj)
{
  
  if(victim){

  if (affected_by_spell(victim, SPELL_BLINDNESS)) {
    affect_from_char(victim, SPELL_BLINDNESS);
    send_to_char("Your vision returns!\n\r", victim);
  }
  }
}

void spell_cure_critic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  if(victim){

  if(IS_HOVERING(victim))
	{
	send_to_char("This corpse is beyond hope.\n\r",ch);
	return;
	}

  healpoints = dice(3,8)-6+level;
  if(GET_CLASS(victim) == CLASS_MAGIC_USER)healpoints+=GET_LEVEL(victim);
  if(ch!=victim)healpoints*=2;
  if(ch!=victim&&GET_POS(victim)==POSITION_FIGHTING)
	GET_EXP(ch)+=healpoints;
  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
  }
}

void spell_cure_light(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  if(ch && victim){

  if(IS_HOVERING(victim))
	{
	send_to_char("This corpse is beyond hope.\n\r",ch);
	return;
	}

  healpoints = dice(1,8)+(level/3);
  if(ch!=victim)healpoints*=2;
  if(ch!=victim&&GET_POS(victim)==POSITION_FIGHTING)
	GET_EXP(ch)+=healpoints;
  if ( (healpoints+GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  update_pos( victim );

  send_to_char("You feel better!\n\r", victim);
  }
}

void spell_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (obj) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

    /* LOWER ATTACK DICE BY -1 */
    if (obj->obj_flags.type_flag == ITEM_WEAPON)  {
      if (obj->obj_flags.value[2] > 1) {
	obj->obj_flags.value[2]--;
	act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
      }
      else {
	send_to_char("Nothing happens.\n\r", ch);
      }
    }
  } else {
    if(!victim)return;
    if(!if_allowed_to_attack(ch,victim))return;

    if (saves_spell(victim, SAVING_SPELL))
      {
	act("$N seems to be unaffected!", FALSE, ch, NULL, victim, TO_CHAR);
	one_hit( victim, ch, TYPE_UNDEFINED);
	return;
      }
    if (affected_by_spell(ch, SPELL_CURSE))
      affect_from_char(victim, SPELL_CURSE);
    af.type      = SPELL_CURSE;
    af.duration  = 5*level;
    af.modifier  = -5;
    af.location  = APPLY_HITROLL;
    af.bitvector = AFF_CURSE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = 2;
    affect_to_char(victim, &af);

    act("$n briefly reveals a red aura!", FALSE, victim, 0, 0, TO_ROOM);
    act("You feel very uncomfortable.",FALSE,victim,0,0,TO_CHAR);
    one_hit( victim, ch, TYPE_UNDEFINED);
  }
}

void spell_detect_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!victim)return;

  af.type      = SPELL_DETECT_EVIL;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;

  affect_join(victim, &af, TRUE, FALSE);

  send_to_char("Your eyes tingle.\n\r", victim);
}

void spell_detect_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!victim)return;

  af.type      = SPELL_DETECT_INVISIBLE;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVISIBLE;

  affect_join(victim, &af, TRUE, FALSE);

  send_to_char("Your eyes tingle.\n\r", victim);
}

void spell_detect_magic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!victim)return;

  af.type      = SPELL_DETECT_MAGIC;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;

  affect_join(victim, &af, TRUE, FALSE);
  send_to_char("Your eyes tingle.\n\r", victim);
}

void spell_detect_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
    if(!ch || (!victim && !obj))return;

  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
	send_to_char("You can sense poison in your blood.\n\r", ch);
      else
	send_to_char("You feel healthy.\n\r", ch);
    else
      if (IS_AFFECTED(victim, AFF_POISON)) {
	act("You sense that $E is poisoned.",FALSE,ch,0,victim,TO_CHAR);
      } else {
	act("You sense that $E is healthy.",FALSE,ch,0,victim,TO_CHAR);
      }
  } else { /* It's an object */
    if(!obj)return;
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
	(obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3])
	act("Poisonous fumes are revealed.",FALSE, ch, 0, 0, TO_CHAR);
      else
	send_to_char("It looks very delicious.\n\r", ch);
    }
  }
}

void spell_enchant_weapon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int i;

  if(ch && obj){

  if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

    for (i=0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
    return;

    SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);
      
    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = 1 +(level >= 18)+ (level >=20)+ (level>=25)+
      (level >=30)+ (level >=35);

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = 1+(level >= 20)+ (level >=25)+ (level >=30)+
      (level >=35);

    if (IS_GOOD(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
      act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
      act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
    } else {
      act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
	}
    }
  }
}

void spell_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  if(!victim)return;
 
  if (IS_HOVERING(victim)) {
    send_to_char("This corpse is beyond hope.\n\r",ch);
    return;
  }

  if (ch!=victim&&GET_POS(victim)==POSITION_FIGHTING)
	GET_EXP(ch)+=200;
  if (victim==ch)
       GET_HIT(victim) += level*3;
  else
       GET_HIT(victim) += level*6;

  if (GET_HIT(victim) >= hit_limit(victim))
    GET_HIT(victim) = MAX(1, hit_limit(victim)-dice(1,4));

  update_pos( victim );

  act("$n heals $N.", FALSE, ch, 0, victim, TO_NOTVICT);
  act("You heal $N.", FALSE, ch, 0, victim, TO_CHAR);
  send_to_char("A warm feeling fills your body.\n\r", victim);
}


void spell_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;


  if (obj&&ch) {
    if (CAN_WEAR(obj,ITEM_TAKE)) {
      if ( !IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)){
	act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
	act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
	SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
      }
    } else
      act("You fail to make $p invisible.",FALSE,ch,obj,0,TO_ROOM);
  }
  else {              /* Then it is a PC | NPC */
    if(!victim)return; 

    if (!affected_by_spell(victim, SPELL_INVISIBLE)) {
      act("$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM);
      send_to_char("You vanish.\n\r", victim);
    }

    af.type      = SPELL_INVISIBLE;
    af.duration  = 24;
    af.modifier  = -40;
    af.location  = APPLY_AC;
    af.bitvector = AFF_INVISIBLE;
    affect_join(victim, &af, TRUE, FALSE);
  }
}

void spell_locate_object(byte level, struct char_data *ch, char *arg,
  struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *i;
  char name[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
	char mybuf[MAX_INPUT_LENGTH], myname[MAX_INPUT_LENGTH];
  int j, num;
  int words,x;

  if(!ch)return;

  words=1;
  for(x=0;arg[x];x++)
    if(arg[x]==' '){
         while(arg[x]==' ')x++;
         if(arg[x])
        words++;
    }

	if(words==2)
		half_chop(arg,myname,mybuf);
	else
		strcpy(mybuf,arg);

	strcpy(name,mybuf);

  /*strcpy(name, fname(obj->name));*/

  num = atoi(name);

  j=level;
  if(GET_LEVEL(ch)>31)
	j*=2;
  for (i = object_list; i && (j>0); i = i->next){

    if(GET_LEVEL(ch)<35)
    if ((isname(name, i->name))&&(!IS_SET(i->obj_flags.extra_flags,ITEM_INVISIBLE))) {
      if(i->carried_by) {
			sprintf(buf,"%s carried by %s.\n\r",
	  			i->short_description,PERS(i->carried_by,ch));
			send_to_char(buf,ch);
      } else if (i->in_obj) {
			sprintf(buf,"%s in %s.\n\r",i->short_description,
	  			i->in_obj->short_description);
			send_to_char(buf,ch);
      } else {
			sprintf(buf,"%s in %s.\n\r",i->short_description,
	(i->in_room == NOWHERE ? "Used but uncertain." : world[i->in_room]->name));
			send_to_char(buf,ch);
	 }
    j--;
		
    if(j==0){
        send_to_char("You are very confused.\n\r",ch);
        return;
    }
 
    }
    if(GET_LEVEL(ch)>=35)
    if (isname(name, i->name) || num == i->item_number) {
	sprintf(buf, "[%5d] ", i->item_number);
	send_to_char(buf,ch);
      if(i->carried_by) {
			sprintf(buf,"%s carried by %s.\n\r",
	  			i->short_description,PERS(i->carried_by,ch));
      } else if (i->in_obj) {
			if(i->in_obj->carried_by)
				sprintf(buf,"%s in %s carried by %s\n\r",i->short_description,
						i->in_obj->short_description,
						PERS(i->in_obj->carried_by, ch));
			else
				if(i->in_obj->worn_by)
					sprintf(buf, "%s in %s worn by %s\n\r",i->short_description,
							i->in_obj->short_description,
							PERS(i->in_obj->worn_by, ch));
				else
					sprintf(buf,"%s in %s.\n\r",i->short_description,
	  					i->in_obj->short_description);
      } else 
			if(i->worn_by)
				sprintf(buf, "%s worn by %s\n\r", i->short_description,
						PERS(i->worn_by,ch));
			else
				sprintf(buf,"%s in %s.\n\r",i->short_description,
						(i->in_room == NOWHERE ? "Used but uncertain." : 
						world[i->in_room]->name));

	send_to_char(buf, ch);
        j--;
  	  if(j==0){
    	send_to_char("You are very confused.\n\r",ch);
		return;
	  }
 
    }
  }
  if(j==0)
    send_to_char("You are very confused.\n\r",ch);
  if(j==level)
    send_to_char("No such object.\n\r",ch);
}

void spell_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!ch||(!victim&&!obj))return;

  if (victim) {
  if(!if_allowed_to_attack(ch,victim))return;
    check_killer(ch,victim);
    if(!saves_spell(victim, SAVING_PARA))
      {
	af.type = SPELL_POISON;
	af.duration = level*2;
	af.modifier = -2;
	af.location = APPLY_STR;
	af.bitvector = AFF_POISON;

	affect_join(victim, &af, TRUE, TRUE);

	send_to_char("You feel very sick.\n\r", victim);
	act("$N winces as the poison enters $S body.",FALSE,ch,0,victim,TO_CHAR);
	act("$N winces as the poison enters $S body.",FALSE,ch,0,victim,TO_NOTVICT);
      }
    else
	act("$N resists the magic.", FALSE, ch, NULL, victim, TO_CHAR);
    
    if (!( ch == victim ))
      one_hit(victim,ch,TYPE_UNDEFINED);
  } else { /* Object poison */

     if(!obj)return;
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
	(obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 1;
    }
  }
}

void spell_protection_from_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!victim)return;
  if(GET_ALIGNMENT(ch)<350){
    send_to_char("Your soul is impure and the Gods fail to answer your prays.\n\r", ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) )
    send_to_char("You have a righteous, protected feeling!\n\r", victim);

  af.type      = SPELL_PROTECT_FROM_EVIL;
  af.duration  = 24;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_PROTECT_EVIL;
  affect_join(victim, &af, TRUE, FALSE);
}

void spell_protection_from_good(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!victim)return;
  if(GET_ALIGNMENT(ch)>(-350)){
    send_to_char("Your soul is impure and the Gods fail to answer your prays.\n\r", ch);
    return;
  }

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_GOOD) )
    send_to_char("You feel the power of vile darkness surge through your body!\n\r", victim);

  af.type      = SPELL_PROTECT_FROM_GOOD;
  af.duration  = 24;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_PROTECT_GOOD;
  affect_join(victim, &af, TRUE, FALSE);
}

void spell_remove_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  if(!ch || (!victim && !obj))return;

  if (obj) {
    if(obj->item_number==10||obj->item_number==16){
	send_to_char("Would a gift from the gods be cursed?\n\r",ch);
	return;
    }

    if ( IS_SET(obj->obj_flags.extra_flags, ITEM_EVIL) ||
    IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
      act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);

      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
    }
  } else {      /* Then it is a PC | NPC */
    if(!victim)return;
    if (affected_by_spell(victim, SPELL_CURSE) ) {
      act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM);
      act("You feel better.",FALSE,victim,0,0,TO_CHAR);
      affect_from_char(victim, SPELL_CURSE);
    }
  }
}

void spell_remove_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  if(!ch || (!victim && !obj))return;

  if (victim) {
    if(affected_by_spell(victim,SPELL_POISON)) {
      affect_from_char(victim,SPELL_POISON);
      act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
      act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
    }
  } else {
    if(!obj)return;
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
	(obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
      act("The $p steams briefly.",FALSE,ch,obj,0,TO_CHAR);
    }
  }
}

void spell_sanctuary(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!ch || !victim)return;

  /* prevent players from sancing mobs with perminant sanc*/
  if (IS_AFFECTED(victim,AFF_SANCTUARY) && 
      !affected_by_spell(victim,SPELL_SANCTUARY))
  {
    act("$N is already sanctified.",TRUE,ch,0,victim,TO_CHAR);
    return;
  }

  if (!affected_by_spell(victim, SPELL_SANCTUARY))
  {
    act("$n is surrounded by a white aura.",TRUE,victim,0,0,TO_ROOM);
    act("You start glowing.",TRUE,victim,0,0,TO_CHAR);
  }

  af.type      = SPELL_SANCTUARY;
  af.duration  = (level<32) ? 3 : level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_SANCTUARY;
  affect_join(victim, &af, TRUE, FALSE);
}


void spell_fireshield(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!ch)return;
  if(!victim)victim=ch;

  if (!affected_by_spell(victim, SPELL_FIRESHIELD))
  {
    act("$n is surrounded by a deep red shield of fire.",TRUE,victim,0,0,TO_ROOM);
    act("A deep red shield of fire surrounds you.",TRUE,victim,0,0,TO_CHAR);
  }

  af.type      = SPELL_FIRESHIELD;
  af.duration  = 4;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_FIRESHIELD;
  affect_join(victim, &af, TRUE, FALSE);
}


void spell_sleep(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  
  if( !victim || !ch )return;

  if(!if_allowed_to_attack(ch,victim))return;
  check_killer( ch, victim );
  
  /* You can't sleep someone higher level than you*/

  if (GET_LEVEL(victim)>=20){
    act("$N laughs in your face at your feeble attempt.", FALSE, ch, NULL, victim, TO_CHAR);
    act("$N tries to sleep you, but fails horribly.", FALSE, ch, NULL, victim, TO_VICT);
    one_hit(victim,ch,TYPE_UNDEFINED);    
    return;
  }


  if (( !saves_spell(victim, SAVING_SPELL) ) || (GET_LEVEL(ch) > 31) )
    {
      af.type      = SPELL_SLEEP;
      af.duration  = MAX(2,(int)(level/3));
      af.modifier  = 0;
      af.location  = APPLY_NONE;
      af.bitvector = AFF_SLEEP;
      affect_join(victim, &af, TRUE, FALSE);

      if (GET_POS(victim)>POSITION_SLEEPING)
	{
	  act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR);
	  act("$n suddenly curls up and falls asleep.",TRUE,victim,0,0,TO_ROOM);
	  GET_POS(victim)=POSITION_SLEEPING;
	}
      
      return;
    }
  else
    act("$N does not look sleepy!", FALSE, ch, NULL, victim, TO_CHAR);
      
  one_hit(victim,ch,TYPE_UNDEFINED);
}


void spell_strength(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if(!victim)return;

  act("You feel stronger.", FALSE, victim,0,0,TO_CHAR);

  af.type      = SPELL_STRENGTH;
  af.duration  = level;
  af.modifier  = 1+(level>18)+(level>25);
  af.location  = APPLY_STR;
  af.bitvector = 0;

  affect_join(victim, &af, TRUE, TRUE);
}


void spell_ventriloquate(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
    /* Actual spell resides in cast_ventriloquate */
}


void spell_word_of_recall(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int loc_nr=0,location,iDist=0;

  void do_look(struct char_data *ch, char *argument, int cmd);
  extern double dHoloDistance(int iX1,int iY1,int iX2,int iY2);

  if(!victim)return;

  if (IS_NPC(victim) || IS_HOVERING(victim))
    return;
 
  if(victim->p->siLastTown)
		if(world[zone_table[victim->p->siLastTown].iRecallRoom])
			loc_nr=zone_table[victim->p->siLastTown].iRecallRoom;
 
  if(loc_nr <= 0) {
	send_to_char("You try to focus but fail...\r\n",victim);
	return;
  }
  iDist = (int) dHoloDistance(
	HOLOX(victim),
	HOLOY(victim),
	HOLORX(loc_nr),
	HOLORY(loc_nr));

  if(iDist > 400) {
	send_to_char("You try to focus your power over such a great distance and fail.\r\n",victim);
	return;
  }		

/*	 
  if(world[victim->in_room]->zone==65){
	send_to_char("Doesn't seem to work from the ship.\n\r",victim);
	return;
  }
  if(world[victim->in_room]->zone==198){
	send_to_char("Try as you may, it doesn't work from this deep underground.\n\r",victim);
	return;
  }
  loc_nr = 1;
  if(GET_CONTINENT(victim)==TRELLOR)
	loc_nr=1371;
*/
 
  location = loc_nr;
  if ( location == -1 )
  {
    send_to_char("You are completely lost.\n\r", victim);
    return;
  }

  /* a location has been found. */
  if(!IS_NPC(victim)&&victim->specials.stpMount){
  		GET_MANA(victim)-=10;
  		if(GET_MANA(victim)<0)
  			GET_MANA(victim)=0;
  }
  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  MOUNTMOVE=TRUE;
  char_from_room(victim);
  char_to_room(victim, location);
  MOUNTMOVE=FALSE;
  act("$n appears in the room with a bright flash of white light.", TRUE, victim, 0, 0, TO_ROOM);
  do_look(victim, "",15);

}


void spell_summon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int target;
  struct descriptor_data *i;
  int iHoloChDistance(struct char_data *ch, struct char_data *vict);

  if(!ch || !victim)return;
  if (IS_SET(world[victim->in_room]->room_flags, SAFE)){
	send_to_char("That person is in a safe area!\n\r",ch);
    return;
  }


  if(IS_NPC(victim)){
     send_to_char("Your prayers go unanswered.\n\r",ch);
     return;
  }
  if(IS_FLYING(victim)){
  	send_to_char("You focus but fail to reach someone flying on a mount.\n\r",ch);
  	return;
  }
  if (GET_LEVEL(victim) > MIN(31,level+3)) {
    send_to_char("You have failed.\n\r",ch);
    return;
  }
  if(iHoloChDistance(ch, victim) > (GET_LEVEL(ch) * 5))  {
	send_to_char("You try to focus your powers over to vast a distance and fail.\r\n",ch);
	return;
  }
/*
  if(world[ch->in_room]->zone==65){
	send_to_char("Doesn't seem to work from the ship.\n\r",ch);
	return;
  }
  if(world[victim->in_room]->zone==65){
	send_to_char("Doesn't seem to work from the ship.\n\r",ch);
	return;
  }
*/
  if(victim->specials.death_timer){
	send_to_char("You cannot seem to summon the corpse.\n\r",ch);
	return;
  }
  if(world[ch->in_room]->zone==198||world[victim->in_room]->zone==198){
	if(world[ch->in_room]->zone!=198||world[victim->in_room]->zone!=198){
	    send_to_char("Doesn't work this deep underground.\n\r",ch);
	    return;
	}
  }
  if(world[ch->in_room]->sector_type == SECT_UNDERWATER
  &&!IS_AFFECTED(ch, AFF_BREATH_WATER)){
	send_to_char("That person cannot breathe water.\n\r",ch);
	return;
  }
  if(IS_SET(world[ch->in_room]->room_flags,NEUTRAL)||
	IS_SET(world[ch->in_room]->room_flags,CHAOTIC)){
	global_color=33;
	send_to_char("The area seems to be protected from summoning!\n\r",ch);
	global_color=0;
	return;
  }
  if(IS_SET(world[victim->in_room]->room_flags,NO_SUMMON)) {
    send_to_char("A magical force greater than your own prevents your summons.\n\r", ch);
    return;
  }
  target = ch->in_room;
  sprintf(log_buf,"(summon) [%s] summons [%s] to %s",
		GET_NAME(ch),GET_NAME(victim), &world[target]->name[0]);
  log_hd(log_buf);
  global_color=31;
  sprintf(log_buf,"(summon) [%s] summons [%s] to %s\n\r",
		GET_NAME(ch),GET_NAME(victim), &world[target]->name[0]);

        for (i = descriptor_list; i; i = i->next)
        if (i->character != ch && !i->connected &&
                GET_LEVEL(i->character) > 30)
	   if(IS_SET(i->character->specials.god_display,GOD_SUMMONS))
		send_to_char(log_buf,i->character);
  act("$n disappears suddenly.",TRUE,victim,0,0,TO_ROOM);

  if(!IS_NPC(victim)&&victim->specials.stpMount){
  		GET_MANA(ch)-=50;
  		if(GET_MANA(ch)<0)
  			GET_MANA(ch)=0;
  }
  
  MOUNTMOVE=TRUE;
  char_from_room(victim);
  char_to_room(victim,target);
  MOUNTMOVE=FALSE;
  act("$n arrives suddenly.",TRUE,victim,0,0,TO_ROOM);
  act("$n has summoned you!",FALSE,ch,0,victim,TO_VICT);
  global_color=0;
  do_look(victim,"",15);
}


void spell_charm_person(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;


  if( !victim || !ch )return;

  if(!if_allowed_to_attack(ch,victim))return;
  if(!IS_NPC(victim)){
	send_to_char("The gods seem to protect the player...\n\r",ch);
	return;
  }
  if ( IS_SET(victim->specials.act, ACT_AGGRESSIVE) ){
	act("$N doesnt seem to like you!", FALSE, ch, NULL, victim, TO_CHAR);
	one_hit(victim,ch,TYPE_UNDEFINED);
	return;
  }

  check_killer( ch, victim );

  /* By testing for IS_AFFECTED we avoid ei. Mordenkainens sword to be */
  /* able to be "recharmed" with duration                              */

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM) &&
      (level >= GET_LEVEL(victim))) {
    if(is_formed(victim)){
	send_to_char("Didn't seem to work.\n\r",ch);
	return;
    }
    if (saves_spell(victim, SAVING_SPELL))
      {
	act("$N doesnt seem to like you!", FALSE, ch, NULL, victim, TO_CHAR);
	one_hit(victim,ch,TYPE_UNDEFINED);
	return;
      }


    put_in_formation(ch->master,victim);

    af.type      = SPELL_CHARM_PERSON;

    if (GET_INT(victim))
      af.duration  = 24*18/GET_INT(victim);
    else
      af.duration  = 24*18;

    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);
  }
}



void spell_sense_life(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

    if(!victim)return;

  if (!affected_by_spell(victim, SPELL_SENSE_LIFE))
    send_to_char("Your feel your awareness improve.\n\r", ch);

  af.type      = SPELL_SENSE_LIFE;
  af.duration  = 5*level;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_SENSE_LIFE;
  affect_join(victim, &af, TRUE, FALSE);
}

void spell_identify(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int i;
  bool found;

  struct time_info_data age(struct char_data *ch);

  /* Spell Names */
  extern char *spells[];

  /* For Objects */
  extern char *item_types[];
  extern char *extra_bits[];
  extern char *apply_types[];
  extern char *affected_bits[];
  extern char *class_rest[];
  int Det;
  int fPercent;

  if(!ch)return;
  if(!obj && !victim)return;

  if (obj) {
    send_to_char("You feel informed:\n\r", ch);

    sprintf(buf, "Object '%s', Item type: ", obj->name);
    sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
    strcat(buf,buf2); strcat(buf,"\n\r");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector,affected_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
    }

    send_to_char("Item is: ", ch);
    sprintbit(obj->obj_flags.extra_flags,extra_bits,buf);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    
    sprintf(buf,"Weight: %d, Value: %d, Level restriction: %d\n\r",
	obj->obj_flags.weight, obj->obj_flags.cost,obj->obj_flags.eq_level);
    send_to_char(buf, ch);

    fPercent =  (float) time(0) / (float) (obj->iDetLife+obj->iBornDate); 
    if(obj->iDetLife == -1)
	Det = 0;
    else if (fPercent >= 1)
	Det = 9;
    else if(fPercent >= .9)
	Det = 8;
    else if(fPercent >= .8)
	Det = 7;
    else if(fPercent >= .7)
	Det = 6;
    else if(fPercent >= .6)
	Det = 5;
    else if(fPercent >= .5)
	Det = 4;
    else if(fPercent >= .4)
	Det = 3;
    else if(fPercent >= .3)
	Det = 2;
    else if(fPercent >= .2)
	Det = 1;
    else if(fPercent >= .1)
	Det = 0;
    else if(fPercent >= 0)
	Det = 0;
    else Det = 10;
    sprintf(log_buf,"%s",szaDetStatusMessages[Det]);
    send_to_char(log_buf,ch);
	

    switch (GET_ITEM_TYPE(obj)) {

    case ITEM_SCROLL : 
    case ITEM_POTION :
      sprintf(buf, "Level %d spells of:\n\r",   obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      if (obj->obj_flags.value[1] >= 1) {
    sprinttype(obj->obj_flags.value[1]-1,spells,buf);
    strcat(buf,"\n\r");
    send_to_char(buf, ch);
      }
      if (obj->obj_flags.value[2] >= 1) {
    sprinttype(obj->obj_flags.value[2]-1,spells,buf);
    strcat(buf,"\n\r");
    send_to_char(buf, ch);
      }
      if (obj->obj_flags.value[3] >= 1) {
    sprinttype(obj->obj_flags.value[3]-1,spells,buf);
    strcat(buf,"\n\r");
    send_to_char(buf, ch);
      }
      break;

    case ITEM_WAND : 
    case ITEM_STAFF : 
      sprintf(buf, "Has %d charges, with %d charges left.\n\r",
	  obj->obj_flags.value[1],
	  obj->obj_flags.value[2]);
      send_to_char(buf, ch);
      
      sprintf(buf, "Level %d spell of:\n\r",    obj->obj_flags.value[0]);
      send_to_char(buf, ch);

      if (obj->obj_flags.value[3] >= 1) {
    sprinttype(obj->obj_flags.value[3]-1,spells,buf);
    strcat(buf,"\n\r");
    send_to_char(buf, ch);
      }
      break;
      
    case ITEM_WEAPON :
      send_to_char("Weapon type: ", ch);
      sprintbit(obj->obj_flags.value[0],class_rest,buf);
      send_to_char(buf, ch);
      send_to_char("\r\n", ch);
      sprintf(buf, "Damage Dice is '%dD%d'\n\r",
	  obj->obj_flags.value[1],
	  obj->obj_flags.value[2]);
      send_to_char(buf, ch);
      if (IS_SET(obj->obj_flags.wear_flags, ITEM_THROW))
	  send_to_char("Item is THROWABLE\n\r",ch);
      break;
    case ITEM_ARMOR :
      sprintf(buf, "AC-apply is %d\n\r",
	  obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      break;

    }

    found = FALSE;

    for (i=0;i<MAX_OBJ_AFFECT;i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
      (obj->affected[i].modifier != 0)) {
    if (!found) {
      send_to_char("Can affect you as:\n\r", ch);
      found = TRUE;
    }

    sprinttype(obj->affected[i].location,apply_types,buf2);
    sprintf(buf,"    Affects : %s By %d\n\r", buf2,obj->affected[i].modifier);
    send_to_char(buf, ch);
      }
    }

  } else {       /* victim */
    if(!victim)return;

    if (!IS_NPC(victim)) {
      sprintf(buf,"%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
	  age(victim).year, age(victim).month,
	  age(victim).day, age(victim).hours);
      send_to_char(buf,ch);

      sprintf(buf,"Height %d cm  Weight %d stones \n\r",
	  GET_HEIGHT(victim), GET_WEIGHT(victim));
      send_to_char(buf,ch);


  sprintf(buf,"Str %d,  Int %d,  Wis %d,  Dex %d,  Con %d\n\r",
  GET_STR(victim),
  GET_INT(victim),
  GET_WIS(victim),
  GET_DEX(victim),
  GET_CON(victim) );
  send_to_char(buf,ch);


    } else {
      send_to_char("You learn nothing new.\n\r", ch);
    }
  }
}


/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct obj_data *burn;

   if( !victim || !ch)return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer( ch, victim );

    dam = number(200,500);

    if ( saves_spell(victim, SAVING_BREATH) )
	dam >>= 1;

    if(damage(ch, victim, dam, SPELL_FIRE_BREATH))
		return;

    /* And now for the damage on inventory */

    if(number(0,50)<GET_LEVEL(ch))
    {
      if (!saves_spell(victim, SAVING_BREATH) )
	{
	  for(burn=victim->carrying ; 
	  burn && !(((burn->obj_flags.type_flag==ITEM_SCROLL) || 
		 (burn->obj_flags.type_flag==ITEM_WAND) ||
		 (burn->obj_flags.type_flag==ITEM_STAFF) ||
		 (burn->obj_flags.type_flag==ITEM_NOTE)) &&
		(number(0,2)==0)) ;
	  burn=burn->next_content);
	  if(burn)
	{
	  act("$o burns",0,victim,burn,0,TO_CHAR);
	  extract_obj(burn);
	}
	}
    }
}


void spell_frost_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
    int dam;
    int hpch;
    struct obj_data *frozen;

    if( !victim || !ch )return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer( ch, victim );

    hpch = GET_HIT(ch);
    if(hpch<10) hpch=10;

    dam = number((hpch/8)+1,(hpch/4));

    if ( saves_spell(victim, SAVING_BREATH) )
	dam >>= 1;

    if(damage(ch, victim, dam, SPELL_FROST_BREATH))
		return;

    /* And now for the damage on inventory */

    if(number(0,50)<GET_LEVEL(ch))
    {
      if (!saves_spell(victim, SAVING_BREATH) )
	{
	  for(frozen=victim->carrying ; 
	  frozen && !(((frozen->obj_flags.type_flag==ITEM_DRINKCON) || 
		   (frozen->obj_flags.type_flag==ITEM_FOOD) ||
		   (frozen->obj_flags.type_flag==ITEM_POTION)) &&
		  (number(0,2)==0)) ;
	  frozen=frozen->next_content); 
	  if(frozen)
	{
	  act("$o breaks.",0,victim,frozen,0,TO_CHAR);
	  extract_obj(frozen);
	}
	}
    }
}


void spell_acid_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
    int dam;
    int hpch;
    int damaged;

    int apply_ac(struct char_data *ch, int eq_pos);
    
    if( !victim || !ch )return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer( ch, victim );
	if(IS_NPC(victim)&&victim->specials.stpMount)return;
    hpch = GET_HIT(ch);
    if(hpch<10) hpch=10;

    dam = number((hpch/8)+1,(hpch/4));

    if ( saves_spell(victim, SAVING_BREATH) )
	dam >>= 1;

    if(damage(ch, victim, dam, SPELL_ACID_BREATH))
		return;

    /* And now for the damage on equipment */

    if(number(0,50)<GET_LEVEL(ch))
    {
      if (!saves_spell(victim, SAVING_BREATH) )
	{
	  for(damaged = 0; damaged<MAX_WEAR &&
	  !((victim->equipment[damaged]) &&
	    (victim->equipment[damaged]->obj_flags.type_flag!=ITEM_ARMOR) &&
	    (victim->equipment[damaged]->obj_flags.value[0]>0) && 
	    (number(0,2)==0)) ; damaged++);  
	  if(damaged<MAX_WEAR)
	{
	  act("$o is damaged.",0,victim,victim->equipment[damaged],0,TO_CHAR);
	  GET_AC(victim)-=apply_ac(victim,damaged);
	  victim->equipment[damaged]->obj_flags.value[0]-=number(1,7);
	  GET_AC(victim)+=apply_ac(victim,damaged);
	  victim->equipment[damaged]->obj_flags.cost = 0;
	}
	}
    }
      }


void spell_gas_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
    int dam;
    int hpch;

    if( !victim || !ch )return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer( ch, victim );

    hpch = GET_HIT(ch);
    if(hpch<10) hpch=10;

    dam = number((hpch/10)+1,(hpch/6));

    if ( saves_spell(victim, SAVING_BREATH) )
	dam >>= 1;

    if(damage(ch, victim, dam, SPELL_GAS_BREATH))
		return;

}


void spell_lightning_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
    int dam;
    int hpch;

    if( !victim || !ch )return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer( ch, victim );

    hpch = GET_HIT(ch);
    if(hpch<10) hpch=10;

    dam = number((hpch/8)+1,(hpch/4));

    if ( saves_spell(victim, SAVING_BREATH) )
	dam >>= 1;

    if(damage(ch, victim, dam, SPELL_LIGHTNING_BREATH))
		return;

}

/*********************************************************************
*                           New spells                  -Kahn        *
*********************************************************************/


void spell_fear(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj)
{
    if(!victim || !ch)return;
    if(!if_allowed_to_attack(ch,victim))return;


    if (saves_spell(victim, SAVING_SPELL)) {
      send_to_char("For a moment you feel compelled to run away, but you fight back the urge.\n\r", victim);
      act("$N doesnt seem to be the yellow bellied slug you thought!",
	  FALSE, ch, NULL, victim, TO_CHAR);
      one_hit(victim, ch, TYPE_UNDEFINED);
      return;
    }
    send_to_char("You suddenly feel very frightened, and you attempt to flee!\n\r", victim);
    do_flee(victim, "", 151);
  }

void spell_refresh(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj)
{
    int dam;

    if(!ch || !victim)return;

    dam = dice(level, 4) + level;
    dam = MAX(dam, 20);

    if ((dam + GET_MOVE(victim)) > move_limit(victim))
	GET_MOVE(victim) = move_limit(victim);
    else
	GET_MOVE(victim) += dam;

    send_to_char("You feel less tired.\n\r", victim);

  }

void spell_fly(byte level, struct char_data *ch,
	       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if(!ch)return;
    if(!victim)ch=victim;


    if (!affected_by_spell(victim, SPELL_FLY)) {
      act("You feel lighter than air!", TRUE, ch, 0, victim, TO_VICT);
      if (victim != ch){
	if (CAN_SEE(ch,victim)) 
	  act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_CHAR);
      }else
	send_to_char("Your feet rise up off the ground.\n\r", ch);
      if (CAN_SEE(ch,victim))
	act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_NOTVICT);
    }
    af.type = SPELL_FLY;
    af.duration = GET_LEVEL(ch) + 3;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_FLYING;
    affect_join(victim, &af, TRUE, FALSE);
}


void spell_breath_water(byte level, struct char_data *ch,
	       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if(!ch || !victim)return;

    act("You feel your lungs harden and heat up!", TRUE, ch, 0, victim,TO_VICT);
    if (victim != ch) {
	act("$N gasps and breathes deeper.", TRUE, ch, 0, victim, TO_CHAR);
      } else {
	send_to_char("You gasp and breathe in deeply.\n\r", ch);
      }
    act("$N gasps and breathes in deeply .", TRUE, ch, 0, victim, TO_NOTVICT);

    af.type = SPELL_BREATH_WATER;
    af.duration = GET_LEVEL(ch) + 10;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_BREATH_WATER;
    affect_to_char(victim, &af);
  }

void spell_cont_light(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj)
{
/*
   creates a ball of light in the hands.
*/
    struct obj_data *tmp_obj;

    if(!ch)return;

    CREATE(tmp_obj, struct obj_data, 1);
    clear_object(tmp_obj);

    tmp_obj->name = str_dup("ball light");
    tmp_obj->short_description = str_dup("A bright ball of light");
    tmp_obj->description = str_dup("There is a bright ball of light on the ground here.");
    tmp_obj->item_number=99;
    tmp_obj->obj_flags.type_flag = ITEM_LIGHT;
    tmp_obj->obj_flags.wear_flags = ITEM_TAKE;
    tmp_obj->obj_flags.value[2] = -1;
    tmp_obj->obj_flags.weight = 1;
    tmp_obj->obj_flags.cost = 0;
    tmp_obj->obj_flags.cost_per_day = 1;

	add_object(tmp_obj);

    obj_to_char(tmp_obj, ch);

    act("$n twiddles $s thumbs and $p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
    act("You twiddle your thumbs and $p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);

  }

#if 0
void spell_animate_dead(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *corpse)
{

    struct char_data *mob;
    struct obj_data *obj_object, *sub_object, *next_obj, *i;
    struct affected_type af;
    char buf[MAX_STRING_LENGTH];
    int number = 100,          /* Number for Zombie */
    r_num;

/*
 some sort of check for corpse hood
*/
    if ((GET_ITEM_TYPE(corpse) != ITEM_CONTAINER) ||
	(!corpse->obj_flags.value[3])) {
	send_to_char("The magic fails abruptly!\n\r", ch);
	return;
      }
    if ((r_num = real_mobile(number)) < 0) {
	send_to_char("Mobile: Zombie not found.\n\r", ch);
	return;
      }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, ch->in_room);

/*
  zombie should be charmed and follower ch
*/

    af.type = SPELL_CHARM_PERSON;
    af.duration = MIN(1, GET_LEVEL(ch) / 3);
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(mob, &af);
    put_in_formation(ch->master,mob);

    GET_EXP(mob) = 100 * GET_LEVEL(ch);
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;

/*
  take all from corpse, and give to zombie
*/

    for (obj_object = corpse->contains; obj_object; obj_object = next_obj) {
	next_obj = obj_object->next_content;
	obj_from_obj(obj_object);
	obj_to_char(obj_object, mob);
      }

/*
   set up descriptions and such
*/
    
    sprintf(buf, "%s body undead", corpse->name);
    mob->player.name = str_dup(buf);
    sprintf(buf, "The undead body of %s", corpse->short_description);
    mob->player.short_descr = str_dup(buf);
    sprintf(buf, "The undead body of %s slowly staggers around.\n\r",corpse->short_description);
    mob->player.long_descr = str_dup(buf);

/*
  set up hitpoints
*/

    mob->points.max_hit = dice((level + 1), 8);
    mob->points.hit = (int) (mob->points.max_hit / 2);

    GET_LEVEL(mob) = GET_LEVEL(ch);
    mob->player.sex = SEX_NEUTRAL;

    SET_BIT(mob->specials.act, ACT_UNDEAD);

/*
  get rid of corpse
*/
    
    act("With mystic power, $n animates $p.", TRUE, ch,
	corpse, 0, TO_ROOM);
    act("$N slowly rises from the ground.", FALSE, ch, 0, mob, TO_ROOM);

    extract_obj(corpse);
  }
#endif

/*  This should be checked out for real mob creating.  */
  
void spell_know_alignment(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj)
{
    int ap;
    char buf[MAX_STRING_LENGTH], name[MAX_STRING_LENGTH];

    if(!victim || !ch)return;

    if (IS_NPC(victim))
	strcpy(name, victim->player.short_descr);
    else
	strcpy(name, GET_NAME(victim));

    ap = GET_ALIGNMENT(victim);

    if (ap > 700)
	sprintf(buf, "%s has an aura as white as the driven snow.\n\r", name);
    else if (ap > 350)
	sprintf(buf, "%s is of excellent moral character.\n\r", name);
    else if (ap > 100)
	sprintf(buf, "%s is often kind and thoughtful.\n\r", name);
    else if (ap > 25)
	sprintf(buf, "%s isn't a bad sort...\n\r", name);
    else if (ap > -25)
	sprintf(buf, "%s doesn't seem to have a firm moral commitment\n\r", name);
    else if (ap > -100)
	sprintf(buf, "%s isn't the worst you've come across\n\r", name);
    else if (ap > -350)
	sprintf(buf, "%s could be a little nicer, but who couldn't?\n\r", name);
    else if (ap > -700)
	sprintf(buf, "%s probably just had a bad childhood\n\r", name);
    else
	sprintf(buf, "I'd rather just not say anything at all about %s\n\r", name);

    send_to_char(buf, ch);

  }

void spell_dispel_magic(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj)
{
    int yes = 0;
    bool mobsanc = FALSE;
    if(level!=-1)
    if(!ch || !victim)return;
    if(!victim && ch){
	send_to_char("Only works on victims\n\r",ch);
	return;
    }

	if(GET_POS(victim) == POSITION_DEAD)
		{
		if(ch)
		send_to_char("The magic of the necromancer is too powerful, and you shirk away in terror.\n\r",ch);
		return;
		}
    if(level!=-1)
    if(!if_allowed_to_attack(ch,victim))return;

/* gets rid of infravision, invisibility, detect, etc */
    yes =TRUE;
    if(ch)
    if (GET_LEVEL(ch) - GET_LEVEL(victim) >= 10)
	yes = TRUE;
    else 
	yes = FALSE;

    if(level==-1)
	yes=TRUE;

    if (GET_LEVEL(victim) - GET_LEVEL(ch) < 10) {

    if (affected_by_spell(victim, SPELL_INVISIBLE))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_INVISIBLE);
	    send_to_char("You feel exposed.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_DETECT_INVISIBLE);
	    send_to_char("You feel less perceptive.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_DETECT_EVIL))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_DETECT_EVIL);
	    send_to_char("You feel less morally alert.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_DETECT_MAGIC))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_DETECT_MAGIC);
	    send_to_char("You stop noticing the magic in your life.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_SENSE_LIFE))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_SENSE_LIFE);
	    send_to_char("You feel less in touch with living things.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_SANCTUARY)) {
	mobsanc = TRUE;
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_SANCTUARY);
	    send_to_char("You don't feel so invulnerable any more.\n\r", victim);
	    act("The white glow around $n's body fades.", FALSE, victim, 0, 0, TO_ROOM);
      }
    }
    if (IS_AFFECTED(victim, AFF_SANCTUARY) && !mobsanc)
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    REMOVE_BIT(victim->specials.affected_by, AFF_SANCTUARY);
	    send_to_char("You don't feel so invulnerable any more.\n\r", victim);
	    act("The white glow around $n's body fades.", FALSE, victim, 0, 0, TO_ROOM);
      }
    if (affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_PROTECT_FROM_EVIL);
	    send_to_char("You feel less morally protected.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_INFRAVISION))
      if (yes || !saves_spell(victim, SAVING_SPELL)) {
    affect_from_char(victim, SPELL_INFRAVISION);
    send_to_char("Your sight grows dimmer.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_FAERIE_FIRE))
      if (yes || !saves_spell(victim, SAVING_SPELL)) {
    affect_from_char(victim, SPELL_FAERIE_FIRE);
    send_to_char("Your pink aura vanishes.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_SLEEP))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_SLEEP);
	    send_to_char("You don't feel so tired.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_CHARM_PERSON))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_CHARM_PERSON);
	    send_to_char("You feel less enthused about your master.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_STRENGTH))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_STRENGTH);
	    send_to_char("You don't feel so strong.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_ARMOR))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_ARMOR);
	    send_to_char("You don't feel so well protected.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_DETECT_POISON))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_DETECT_POISON);
	    send_to_char("You don't feel so sensitive to fumes.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_BLESS))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_BLESS);
	    send_to_char("You don't feel so blessed.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_SHIELD))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_SHIELD);
	    send_to_char("Your magical shield has been dispelled.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_STONE_SKIN))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_STONE_SKIN);
	    send_to_char("Your skin feels like stone no longer.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_FLY))
	if (yes || !saves_spell(victim, SAVING_SPELL)) {
	    affect_from_char(victim, SPELL_FLY);
	    send_to_char("You don't feel lighter than air any more.\n\r", victim);
      }
    if (affected_by_spell(victim, SPELL_FIRESHIELD))
      if (yes || !saves_spell(victim, SAVING_SPELL)) {
    affect_from_char(victim, SPELL_FIRESHIELD);
    send_to_char("Your shield of fire collapses.\n\r", victim);
    act("The shield of fire around $n's body fizzles.", FALSE, victim, 0,0,TO_ROOM);
      }
    if (affected_by_spell(victim, SPELL_BREATH_WATER))
      if (yes || !saves_spell(victim, SAVING_SPELL)) {
    affect_from_char(victim, SPELL_BREATH_WATER);
    send_to_char("Your lungs heave and you cough water.\n\r", victim);
    act("$n's lungs heave and $e coughs water.", FALSE, victim, 0,0,TO_ROOM);
      }
    if (affected_by_spell(victim, SPELL_MAP_CATACOMBS))
      if (yes || !saves_spell(victim, SAVING_SPELL)) {
    affect_from_char(victim, SPELL_MAP_CATACOMBS);
    send_to_char("You get woozy as your purple mapping aura fades.\n\r", victim);
    act("$n gets woozy as the purple mapping aura fades.", FALSE, victim, 0,0,TO_ROOM);
      }
    }
  }

void spell_conjure_elemental(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct obj_data *obj_object;
    struct char_data *elemental;
    int mob_num;
    /*
     *   victim, in this case, is the elemental
     *   object could be the sacrificial object
     */
    if(!ch)return;
    send_to_char("You focus in on yourself, drawing in upon your heartbeat.\n\r",ch);
    mob_num=0;
    /*
    ** objects:
    **     fire  : red stone		fire elemental
    **     water : pale blue stone	water elemental
    **     earth : grey stone		earth elemental
    **     air   : clear stone		air elemental
    */
    obj_object = ch->equipment[HOLD];
    if (obj_object) {if(obj_object->item_number==5243||obj_object->item_number==5233||obj_object->item_number==5239||obj_object->item_number==5230){
	     if(obj_object->item_number==5243)mob_num=3;
	     if(obj_object->item_number==5233)mob_num=4;
	     if(obj_object->item_number==5239)mob_num=5;
	     if(obj_object->item_number==5230)mob_num=6;

	}
    }
    if(!mob_num){
	send_to_char("You cast the spell and fail miserably, perhaps the wrong sacrificial object.\n\r",ch);
	return;
    }
    elemental = read_mobile(mob_num, REAL);
    if(!elemental){
         send_to_char("The spell fizzles at the last minute.\n\r",ch);
         return;
    }
    char_to_room(elemental, ch->in_room);
    act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
    act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
    act("$p explodes with a loud BANG!", TRUE, ch, obj_object, 0, TO_ROOM);
    act("$p explodes with a loud BANG!", TRUE, ch, obj_object, 0, TO_CHAR);
    unequip_char( ch, HOLD );
    extract_obj(obj_object);
    act("Out of the smoke, $N emerges!!", TRUE, ch, 0, elemental, TO_NOTVICT);
    /* charm them for a while */
    elemental->points.max_hit=dice(GET_LEVEL(ch)*GET_LEVEL(ch), 5);
    GET_HIT(elemental)=(GET_MAX_HIT(elemental));
    elemental->points.max_mana=GET_MAX_MANA(ch);
    GET_MANA(elemental)=GET_MAX_MANA(ch);
    elemental->points.max_move=GET_MAX_MOVE(ch);
    GET_MOVE(elemental)=GET_MAX_MOVE(ch);
    GET_LEVEL(elemental)=GET_LEVEL(ch)-1;
    GET_STR(elemental)=18;
    GET_DEX(elemental)=GET_DEX(ch);
    GET_INT(elemental)=GET_INT(ch);
    GET_WIS(elemental)=GET_WIS(ch);
    GET_CON(elemental)=GET_CON(ch);
    GET_AC(elemental)=(100-GET_LEVEL(ch)*5);
    GET_EXP(elemental)=1000;
    GET_SEX(elemental)=GET_SEX(ch);
    GET_HITROLL(elemental)=MAX(GET_LEVEL(ch),20);
    GET_DAMROLL(elemental)=(GET_LEVEL(ch));
    GET_ALIGNMENT(elemental)=0;
    put_in_formation(ch->master,elemental);

    af.type = SPELL_CHARM_PERSON;
    af.duration = 48;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;

    affect_to_char(elemental, &af);
    do_say(elemental,"Growllll...  YOU WILL pay for this!",9);
  }

void spell_cure_serious(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj)
{
    int healpoints;

    if(!ch || !victim)return;

	if(IS_HOVERING(victim))
		{
		send_to_char("This corpse is beyond hope.\n\r",ch);
		return;
		}

    healpoints = dice(2, 8) +(level/2);
    if(GET_CLASS(victim) == CLASS_MAGIC_USER)healpoints+=GET_LEVEL(victim);
    if(ch!=victim)healpoints*=2;
    if(ch!=victim&&GET_POS(victim)==POSITION_FIGHTING)
	GET_EXP(ch)+=healpoints;
    if ((healpoints + GET_HIT(victim)) > hit_limit(victim))
	GET_HIT(victim) = hit_limit(victim);
    else
	GET_HIT(victim) += healpoints;

    update_pos(victim);

    send_to_char("You feel better!\n\r", victim);
  }

void spell_cause_light(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    int dam;
    if(!ch || !victim)return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer(ch, victim);
    
	dam = dice(1, 10) + 1;

    if(damage(ch, victim, dam, SPELL_CAUSE_LIGHT))
		return;

  }

void spell_cause_critical(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj)
{
    int dam;
    if(!ch || !victim)return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer(ch, victim);
    
	dam = dice(3,10)+2;

    if(damage(ch, victim, dam, SPELL_CAUSE_CRITICAL))
		return;

  }

void spell_cause_serious(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj)
{
    int dam;
    if(!ch || !victim)return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer(ch, victim);
    
	dam=dice(2,10)+2;

    if(damage(ch, victim, dam, SPELL_CAUSE_SERIOUS))
		return;
  }

void spell_flamestrike(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    int dam;

    if(!victim || !ch)return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer (ch, victim);
    
    dam = dice(MAX(25,GET_LEVEL(ch)), 6);

    if (saves_spell(victim, SAVING_SPELL))
		dam*=.69;

    if(damage(ch, victim, dam, SPELL_FLAMESTRIKE))
		return;

  }

/*
   magic user spells
*/

void spell_stone_skin(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if(!ch)return;

    if (!affected_by_spell(ch, SPELL_STONE_SKIN)) {
	act("$n's skin turns grey and granite-like.", TRUE, ch, 0, 0, TO_ROOM);
	act("Your skin turns to a stone-like substance.", TRUE, ch, 0, 0, TO_CHAR);
    }
	af.type = SPELL_STONE_SKIN;
	af.duration = level;
	af.modifier = -70;
	af.location = APPLY_AC;
	af.bitvector = 0;
	affect_join(ch, &af, TRUE, FALSE);
}

void spell_shield(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if(!victim || !ch)return;


    if (!affected_by_spell(victim, SPELL_SHIELD)) {
	send_to_char("You are surrounded by a strong magical force shield.\r\n", victim);
	act("$N glows for a moment as $e is protected by a magical force shield.",
	   FALSE, ch, 0, victim, TO_NOTVICT);
    }
	af.type = SPELL_SHIELD;
	af.duration = 8 + level;
	af.modifier = -20;
	af.location = APPLY_AC;
	af.bitvector = 0;
	affect_join(victim, &af, TRUE, FALSE);
}

void spell_weaken(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    float modifier;

    if(!ch || !victim)return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer (ch, victim);
    
    if (!affected_by_spell(victim, SPELL_WEAKEN))
      {
	if (!saves_spell(victim, SAVING_SPELL)) {
	  modifier = (77.0 - level) / 100.0;
	  act("You feel weaker.", FALSE, victim, 0, 0, TO_VICT);
	  act("$n seems weaker.", FALSE, victim, 0, 0, TO_ROOM);
	  
	  af.type = SPELL_WEAKEN;
	  af.duration = (int) level / 2;
	  af.modifier = (int) 0 - (victim->abilities.str * modifier);
	  af.location = APPLY_STR;
	  af.bitvector = 0;

	  affect_to_char(victim, &af);
	}
	else
	  {
	    act("$N isn't the wimp you made $M out to be!", FALSE,
		ch, NULL, victim, TO_CHAR);
	  }
      }
    
    one_hit(victim, ch, TYPE_UNDEFINED);
}


void spell_acid_blast(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj){

    struct char_data *v;
    struct char_data *next_v;
    int dam;

    if(!ch)return;

    dam = dice(level*2, 4) * .80;

    for(v=world[ch->in_room]->people;v;v=next_v){
        next_v = v->next_in_room;
        if(IS_NPC(v)&&IS_NPC(v->master)&&(!IS_DEAD(v))){

	    if(saves_spell(v, SAVING_SPELL)){
		if(damage(ch, v, dam*.69, SPELL_ACID_BLAST))
		    dam*=.90;
	    }else{
                if(damage(ch, v, dam, SPELL_ACID_BLAST))
	            dam*=.90;
	    }
	}
    }
}

     
void spell_faerie_fire(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if(!ch || !victim || !if_allowed_to_attack(ch,victim) )return;

    if (affected_by_spell(victim, SPELL_FAERIE_FIRE)) {
	send_to_char("Nothing new seems to happen\n\r", ch);
	return;
      }
    act("$n points at $N.", TRUE, ch, 0, victim, TO_ROOM);
    act("You point at $N.", TRUE, ch, 0, victim, TO_CHAR);
    act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_ROOM);
    act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_CHAR);

    af.type = SPELL_FAERIE_FIRE;
    af.duration = level;
    af.modifier = +10;
    af.location = APPLY_ARMOR;
    af.bitvector = 0;

    affect_to_char(victim, &af);

  }

void spell_faerie_fog(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim;

    if(!ch)return;

    act("$n snaps $s fingers, and a cloud of purple smoke billows forth",
	TRUE, ch, 0, 0, TO_ROOM);
    act("You snap your fingers, and a cloud of purple smoke billows forth",
	TRUE, ch, 0, 0, TO_CHAR);


    for (tmp_victim = world[ch->in_room]->people; tmp_victim;
	 tmp_victim = tmp_victim->next_in_room) {
		if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
	    	if (GET_LEVEL(tmp_victim) > 31)
				continue;
	    	if (IS_AFFECTED(tmp_victim, AFF_INVISIBLE) || IS_AFFECTED(tmp_victim, AFF_HIDE)) {
				if (!saves_spell(tmp_victim, SAVING_SPELL)) {
		    		act("$n is briefly revealed, but disappears again.", FALSE, tmp_victim, 0, 0, TO_ROOM);		
		    		act("You are briefly revealed, but disappear again.", FALSE, tmp_victim, 0, 0, TO_CHAR);
				}
			}
		}
	}
}

/*	
	  }
	  }
      }
      }
  }
*/

void spell_drown(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj)
{
    int dam;
    if(!ch || !victim)return;
    if(!if_allowed_to_attack(ch,victim))return;
    check_killer( ch, victim);
    
    if (ch->in_room < 0)
	return;
    dam = dice(level, 3);

    act("$n gestures, and a huge pool of water shoots from the ground!\n\r",
	FALSE, ch, 0, 0, TO_ROOM);

    if(damage(ch, victim, dam, SPELL_DROWN))
		return;
  }

void spell_demonfire(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
    int dam;

    if(!ch || !victim )return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer( ch, victim );

    if(GET_ALIGNMENT(ch) > (-350)){
      send_to_char("The Gods of Death and War ignore your prays.\n\r", ch);
      return;
    }
	dam = dice(2*GET_LEVEL(ch),2)+(2*GET_LEVEL(ch));

	if(saves_spell(victim,SAVING_SPELL))
		dam*=.69;
    if(damage(ch, victim, dam, SPELL_DEMONFIRE))
		return;
  }

void spell_turn_undead(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    int dam;

    if(!ch || !victim)return;

    if (!IS_SET(victim->specials.act, ACT_UNDEAD)) {
	act("But $n isn't undead!", FALSE, ch, 0, victim, TO_CHAR);
	return;
      }
    dam = dice(level, 10);
    if(damage(ch, victim, dam, SPELL_TURN_UNDEAD))
		return;
  }

void spell_infravision(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if(!victim || !ch)return;

    if (!IS_AFFECTED(victim, AFF_INFRARED)) {
	send_to_char("Your eyes glow red for a moment.\n\r", victim);
	act("$n's eyes take on a red hue.\n\r", FALSE, victim, 0, 0, TO_ROOM);
    }
	af.type = SPELL_INFRAVISION;
	af.duration = 2 * level;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_INFRARED;
	affect_join(victim, &af, TRUE, FALSE);
}

void spell_sandstorm(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)

{
    int dam, chance, dir, room;

    if((!ch)||(!victim))return;
    if (world[ch->in_room]->sector_type != SECT_DESERT) {
	send_to_char("Nothing seems to happen.\n\r", ch);
	return;
      }

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer (ch, victim);
    
    GET_MOVE(victim) -= (level * 2);
    GET_MOVE(victim) = MAX(GET_MOVE(victim), 0);

    dam = dice(3, level);
    if(damage(ch, victim, dam, SPELL_SANDSTORM))
		return;

    if (!victim || (victim->in_room == NOWHERE)) return;

    chance = (level * 5) - 10;
    if ((chance > number(1, 100)) && world[ch->in_room]->dir_option[dir = number(0, 5)]) {
	act("Your sandstorm hurls $N out of the room!", FALSE, ch, 0, victim, TO_CHAR);
	act("$N is hurled out of the room!", FALSE, ch, 0, victim, TO_NOTVICT);
	act("You are hurled out of the room!", FALSE, ch, 0, victim, TO_VICT);

	room = world[ch->in_room]->dir_option[dir]->to_room;
	char_from_room(victim);
	char_to_room(victim, room);

	act("$n is hurled into the room!", FALSE, victim, 0, 0, TO_ROOM);
      }
  }

void spell_hands_of_wind(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{

	if(!if_allowed_to_attack(ch,victim))
		return;

    act("You sense $S aura drawing closer...", FALSE, ch, 0, victim, TO_CHAR);

    act("$n hurls a stream of violent wind at $N!", FALSE, ch, 0, victim, TO_ROOM);
    act("$N hurls a violent wind at you!", FALSE, victim, 0, ch, TO_CHAR);

    die_formation(victim->master);

    GET_MOVE(ch)/=2;
	send_to_char("Creating such a wind causes you to lose your balance.\n\r",ch);
    GET_POS(ch)=POSITION_SITTING;
	fall(ch);
 }


void spell_plague(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    if(!ch || !victim)return;
    if(!if_allowed_to_attack(ch,victim))return;
    check_killer (ch, victim);
    if(GET_LEVEL(victim)>=GET_LEVEL(ch))
    if (saves_spell(victim, SAVING_SPELL) || (!number(0, 5))) {
	act("$N appears to block the magic.", FALSE, ch, 0, victim, TO_CHAR);
	one_hit(victim,ch,TYPE_UNDEFINED);
	return;
      }


    if(IS_AFFECTED(victim,AFF_PLAGUE)){
	act("$N, sick as $E may be, attacks!", FALSE, ch, 0, victim, TO_CHAR);
	act("$N, sick as $E may be, attacks!", FALSE, ch, 0, victim, TO_ROOM);
        one_hit(victim,ch,TYPE_UNDEFINED);
        return;
    }
    af.type = SPELL_PLAGUE;
    af.duration = level / 5;
    af.modifier = -5;
    af.location = APPLY_STR;
    af.bitvector = AFF_PLAGUE;

    affect_join(victim, &af, FALSE, FALSE);

    act("$n chokes and utters some muffled noises.", FALSE, victim, 0, 0, TO_ROOM);
    act("You choke and utter some muffled noises.", FALSE, victim, 0, 0, TO_CHAR);
    GET_HIT(victim)-=GET_LEVEL(ch);
    GET_HIT(victim)-=number(1,(int)(GET_HIT(victim)*(.01*GET_LEVEL(ch))));
    if(GET_HIT(victim)<1)GET_HIT(victim)=1;
  }

void spell_hammer_of_faith(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
    int dam;

    if(!ch || !victim )return;

    if(!if_allowed_to_attack(ch,victim))return;
    check_killer( ch, victim );

    
	dam = dice(2*GET_LEVEL(ch),5);

        if((GET_ALIGNMENT(ch)==1000&&GET_ALIGNMENT(victim)==(-1000))||
           (GET_ALIGNMENT(ch)==(-1000)&&GET_ALIGNMENT(victim)==1000)){
             dam += 25;
             if(saves_spell(victim,SAVING_SPELL))
		dam*=.69;
             if(damage(ch, victim, dam, SPELL_HAMMER_OF_FAITH)){
               act("A powerful strike from your hammer shatters $N's body and spirit!\n\r", FALSE, ch, 0, victim, TO_CHAR);
               act("$n brings the hammer down on you hard, shattering your body!\n\r", FALSE, ch, 0, victim, TO_VICT);
	       return;
             }
             act("Sparks fly from the head of your hammer and $N shudders from the impact!\n\r", FALSE, ch, 0, victim, TO_CHAR);
             act("You are stunned by the devestating impact of $n's Hammer of Faith!\n\r", FALSE, ch, 0, victim, TO_VICT);
 	     act("Powerful thunder from the heavens echos throughout the area!",FALSE, ch, 0, 0, TO_ROOM);
             return;
        }
        if((GET_ALIGNMENT(ch)>349&&GET_ALIGNMENT(victim)<(-349))||
          (GET_ALIGNMENT(ch)<(-349)&&GET_ALIGNMENT(victim)>349)){
             if(saves_spell(victim,SAVING_SPELL))
		dam*=.69;
             if(damage(ch, victim, dam, SPELL_HAMMER_OF_FAITH)){
               act("A powerful strike from your hammer shatters $N's body and spirit!\n\r", FALSE, ch, 0, victim, TO_CHAR);
               act("$n brings the hammer down on you hard, shattering your body!\n\r", FALSE, ch, 0, victim, TO_VICT);
	       return;
             }
             act("You deliver a devestating blow with your Hammer of Faith.\n\r", FALSE, ch, 0, 0, TO_CHAR);
             act("$n devestates you with $s Hammer of Faith.\n\r", FALSE, ch, 0, victim, TO_VICT);
             return;
        }
        if(((GET_ALIGNMENT(ch)>(-350)&&GET_ALIGNMENT(ch)<350)&&
           (GET_ALIGNMENT(victim)>349||GET_ALIGNMENT(victim)<(-349)))||
           ((GET_ALIGNMENT(ch)>349||GET_ALIGNMENT(ch)<(-349))&&
           (GET_ALIGNMENT(victim)>(-350)&&GET_ALIGNMENT(victim)<350))){
             dam *= .8;
             if(saves_spell(victim,SAVING_SPELL))
		dam*=.69;
             if(damage(ch, victim, dam, SPELL_HAMMER_OF_FAITH)){
               act("A powerful strike from your hammer shatters $N's body and spirit!\n\r", FALSE, ch, 0, victim, TO_CHAR);
               act("$n brings the hammer down on you hard, shattering your body!\n\r", FALSE, ch, 0, victim, TO_VICT);
	       return;
             }
             act("You deliver a powerful blow with your Hammer of Faith.\n\r", FALSE, ch, 0, 0, TO_CHAR);
             act("$n smites you with $s Hammer of Faith.\n\r", FALSE, ch, 0, victim, TO_VICT);
             return;
        }
        if((GET_ALIGNMENT(ch)>350&&GET_ALIGNMENT(victim)>350)||
           (GET_ALIGNMENT(ch)<(-350)&&GET_ALIGNMENT(victim)<(-350))||
           (GET_ALIGNMENT(ch)<350&&GET_ALIGNMENT(ch)>-350&&
           GET_ALIGNMENT(victim)<350&&GET_ALIGNMENT(victim)>-350)){
             dam = 0;
             send_to_char("The shimmering, ghostly image of a warhammer appears in your hands.\n\r", ch);
             act("Your warhammer passes harmlessly through $N.\n\r", FALSE, ch, 0,victim, TO_CHAR);
             act("The shimmering, ghostly image of a warhammer appears in $n's hands.", FALSE, ch, 0, victim, TO_VICT);
             act("$n's spiritual hammer passes harmlessly through you.\n\r", FALSE, ch, 0, victim, TO_VICT);

             return;
        }
        send_to_char("Something fucked up!\n\r", ch);
		return;
  }

/*********************************************************************
*  The following are the higher level procedures that handle the     *
*  abilities of objects/people that cast spells.      -Kahn          *
*********************************************************************/

void cast_burning_hands( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
    switch (type)
    {
    case SPELL_TYPE_SPELL:
	spell_burning_hands(level, ch, victim, 0); 
	break;
	case SPELL_TYPE_SCROLL:
	if (victim)
	  spell_burning_hands(level, ch, victim, 0);
	else if (!tar_obj)
	  spell_burning_hands(level, ch, ch, 0);
	break;
	case SPELL_TYPE_WAND:
	if (victim)
	  spell_burning_hands(level, ch, victim, 0);
	break;
    default : 
	log_hd("Serious screw-up in burning hands!");
	break;
    }
}


void cast_call_lightning( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  extern struct weather_data weather_info;

  if(level>30)level=30;
    switch (type) {
	case SPELL_TYPE_SPELL:
	    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
		spell_call_lightning(level, ch, victim, 0);
	    } else {
		send_to_char("You fail to call the lightning down from the sky!\n\r", ch);
	    }
	    break;
      case SPELL_TYPE_POTION:
	    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
		spell_call_lightning(level, ch, ch, 0);
	    }
	    break;
      case SPELL_TYPE_SCROLL:
	    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
		if(victim) 
		    spell_call_lightning(level, ch, victim, 0);
		else if(!tar_obj) spell_call_lightning(level, ch, ch, 0);
	    }
	    break;
      case SPELL_TYPE_STAFF:
	    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING))
	    {
		for (victim = world[ch->in_room]->people ;
		victim ; victim = victim->next_in_room )
		    if( IS_NPC(victim) )
			spell_call_lightning(level, ch, victim, 0);
	    }
	    break;
      default : 
	 log_hd("Serious screw-up in call lightning!");
	 break;
    }
}


void cast_chill_touch( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_chill_touch(level, ch, victim, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (victim)
      spell_chill_touch(level, ch, victim, 0);
    else if (!tar_obj)
      spell_chill_touch(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (victim)
      spell_chill_touch(level, ch, victim, 0);
    break;          
  default : 
    log_hd("Serious screw-up in chill touch!");
    break;
    }
}


void cast_shocking_grasp( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_shocking_grasp(level, ch, victim, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (victim)
      spell_shocking_grasp(level, ch, victim, 0);
    else if (!tar_obj)
      spell_shocking_grasp(level, ch, ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (victim)
      spell_shocking_grasp(level, ch, victim, 0);
    break;          
  default : 
      log_hd("Serious screw-up in shocking grasp!");
    break;
  }
}


void cast_colour_spray( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_colour_spray(level, ch, victim, 0);
	 break; 
    case SPELL_TYPE_SCROLL:
	 if(victim) 
	    spell_colour_spray(level, ch, victim, 0);
	 else if (!tar_obj)
		spell_colour_spray(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(victim) 
	    spell_colour_spray(level, ch, victim, 0);
	 break;
    default : 
	 log_hd("Serious screw-up in colour spray!");
	 break;
    }
}


void cast_earthquake( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
	    spell_earthquake(level, ch, 0, 0);
	  break;
    default : 
	 log_hd("Serious screw-up in earthquake!");
	 break;
    }
}


void cast_energy_drain( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_energy_drain(level, ch, victim, 0);
	    break;
    case SPELL_TYPE_POTION:
	 spell_energy_drain(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_SCROLL:
	 if(victim)
		spell_energy_drain(level, ch, victim, 0);
	 else if(!tar_obj)
	    spell_energy_drain(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(victim)
		spell_energy_drain(level, ch, victim, 0);
	 break;
    case SPELL_TYPE_STAFF:
	 for (victim = world[ch->in_room]->people ;
	      victim ; victim = victim->next_in_room )
	    if( IS_NPC(victim) )
	       spell_energy_drain(level, ch, victim, 0);
	 break;
    default : 
	 log_hd("Serious screw-up in energy drain!");
	 break;
    }
}


void cast_shockwave( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	  spell_shockwave(level, ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	 if(victim)
		spell_shockwave(level, ch, victim, 0);
	 else if(!tar_obj)
	    spell_shockwave(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(victim)
		spell_shockwave(level, ch, victim, 0);
	 break;
    default : 
	 log_hd("Serious screw-up in shockwave!");
	 break;

    }
}

void cast_fireball( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	  spell_fireball(level, ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	 if(victim)
		spell_fireball(level, ch, victim, 0);
	 else if(!tar_obj)
	    spell_fireball(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(victim)
		spell_fireball(level, ch, victim, 0);
	 break;
    default : 
	 log_hd("Serious screw-up in fireball!");
	 break;

    }
}
void cast_frost_shards( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	  spell_frost_shards(level, ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	 if(victim)
		spell_frost_shards(level, ch, victim, 0);
	 else if(!tar_obj)
	    spell_frost_shards(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(victim)
		spell_frost_shards(level, ch, victim, 0);
	 break;
    default : 
	 log_hd("Serious screw-up in frost shards!");
	 break;

    }
}


void cast_harm( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
    switch (type) {
    case SPELL_TYPE_SPELL:
	 spell_harm(level, ch, victim, 0);
	 break;
    case SPELL_TYPE_POTION:
	 spell_harm(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_STAFF:
	 for (victim = world[ch->in_room]->people ;
	      victim ; victim = victim->next_in_room )
	    if ( IS_NPC(victim) )
	       spell_harm(level, ch, victim, 0);
	 break;
    default : 
	 log_hd("Serious screw-up in harm!");
	 break;

  }
}


void cast_lightning_bolt( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	 spell_lightning_bolt(level, ch, victim, 0);
	 break;
    case SPELL_TYPE_SCROLL:
	 if(victim)
		spell_lightning_bolt(level, ch, victim, 0);
	 else if(!tar_obj)
	    spell_lightning_bolt(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(victim)
		spell_lightning_bolt(level, ch, victim, 0);
	 break;
    default : 
	 log_hd("Serious screw-up in lightning bolt!");
	 break;

  }
}

void cast_chain_lightning( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	 spell_chain_lightning(level, ch, victim, 0);
	 break;
    case SPELL_TYPE_SCROLL:
	 if(victim)
		spell_chain_lightning(level, ch, victim, 0);
	 else if(!tar_obj)
	    spell_chain_lightning(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(victim)
		spell_chain_lightning(level, ch, victim, 0);
	 break;
    default : 
	 log_hd("Serious screw-up in chain lightning!");
	 break;

  }
}

void cast_magic_missile( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_magic_missile(level, ch, victim, 0);
      break;
    case SPELL_TYPE_SCROLL:
	 if(victim)
		spell_magic_missile(level, ch, victim, 0);
	 else if(!tar_obj)
	    spell_magic_missile(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(victim)
		spell_magic_missile(level, ch, victim, 0);
	 break;
    default : 
	 log_hd("Serious screw-up in magic missile!");
	 break;

  }
}

void cast_armor( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
	case SPELL_TYPE_SPELL:
	    spell_armor(level,ch,tar_ch,0);
	    break;
	case SPELL_TYPE_POTION:
	    spell_armor(level,ch,ch,0);
	    break;
	case SPELL_TYPE_SCROLL:
	case SPELL_TYPE_WAND:
	    if (tar_obj) return;
	    if (!tar_ch) tar_ch = ch;
	    spell_armor(level,ch,tar_ch,0);
	    break;
      default : 
	 log_hd("Serious screw-up in armor!");
	 break;
    }
}

void cast_teleport( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SCROLL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SPELL:
    if (!tar_ch)
      tar_ch = ch;
    spell_teleport(level, ch, tar_ch, 0);
    break;
    
  case SPELL_TYPE_WAND:
    if(!tar_ch) tar_ch = ch;
    spell_teleport(level, ch, tar_ch, 0);
    break;

    case SPELL_TYPE_STAFF:
      for (tar_ch = world[ch->in_room]->people ; 
	   tar_ch ; tar_ch = tar_ch->next_in_room)
	 if ( IS_NPC(tar_ch) )
	    spell_teleport(level, ch, tar_ch, 0);
      break;
	    
    default : 
      log_hd("Serious screw-up in teleport!");
      break;
    }
}


void cast_bless( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) ) {
    send_to_char("Nothing seems to happen.\n\r", ch);
    return;
      }
      spell_bless(level,ch,0,tar_obj);
      
    } else              /* Then it is a PC | NPC */
      spell_bless(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    if ( affected_by_spell(ch, SPELL_BLESS) ||
    (GET_POS(ch) == POSITION_FIGHTING)) {
    send_to_char("Nothing seems to happen.\n\r", ch);
    return;
      }
    spell_bless(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
    {
    send_to_char("Nothing seems to happen.\n\r", ch);
    return;
      }
      spell_bless(level,ch,0,tar_obj);
      
    } else {              /* Then it is a PC | NPC */
      
      if (!tar_ch) tar_ch = ch;
      
      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
      (GET_POS(tar_ch) == POSITION_FIGHTING))
    {
    send_to_char("Nothing seems to happen.\n\r", ch);
    return;
      }
      spell_bless(level,ch,tar_ch,0);
    }
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) {        /* It's an object */
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
    {
    send_to_char("Nothing seems to happen.\n\r", ch);
    return;
      }
      spell_bless(level,ch,0,tar_obj);
      
    } else {              /* Then it is a PC | NPC */
      
      if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
      (GET_POS(tar_ch) == POSITION_FIGHTING))
    {
    send_to_char("Nothing seems to happen.\n\r", ch);
    return;
      }
      spell_bless(level,ch,tar_ch,0);
    }
    break;
    default : 
      log_hd("Serious screw-up in bless!");
    break;
  }
}



void cast_blindness( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

    if (IS_SET(world[ch->in_room]->room_flags,SAFE)){
      send_to_char("You can not blind anyone in a safe area!\n\r", ch);
      return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_blindness(level,ch,tar_ch,0);
	break;
    case SPELL_TYPE_POTION:
	spell_blindness(level,ch,ch,0);
	break;
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
	if (tar_obj) return;
	if (!tar_ch) tar_ch = ch;
	spell_blindness(level,ch,tar_ch,0);
	break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = world[ch->in_room]->people ; 
	   tar_ch ; tar_ch = tar_ch->next_in_room)
	    if ( IS_NPC(tar_ch) ) 
	  if (!(IS_AFFECTED(tar_ch, AFF_BLIND)))
	spell_blindness(level,ch,tar_ch,0);
      break;
    default : 
	log_hd("Serious screw-up in blindness!");
      break;
    }
}

void cast_sense_death( byte level, struct char_data *ch, char *arg,int type,  struct char_data *tar_ch, struct obj_data *tar_obj ) {
struct room_affect *rap=NULL;
int room;
  switch (type) {
    case SPELL_TYPE_SPELL:
	room=ch->in_room;
	if(room<1||room>=MAX_ROOM)return;
	if(!world[room])return;
	if(!world[room]->room_afs)return;
	send_to_char("You focus....you call upon death......you sense...\n\r",ch);
	for(rap=world[room]->room_afs;rap;rap=rap->next)
	    if(rap->type==RA_BLOOD)
		if(rap->text){
		    sprintf(log_buf,"%s %d hours ago.\n\r",rap->text,rap->value);
		    send_to_char(log_buf,ch);
		}
	break;
      default : 
	 log_hd("Serious screw-up in control weather!");
	 break;
    }

}

void cast_control_weather( byte level, struct char_data *ch, char *arg,int type,  struct char_data *tar_ch, struct obj_data *tar_obj )
{
    char buffer[MAX_STRING_LENGTH];
    extern struct weather_data weather_info;

  switch (type) {
    case SPELL_TYPE_SPELL:

	    one_argument(arg,buffer);

	    if (str_cmp("better",buffer) && str_cmp("worse",buffer))
	    {
		send_to_char("Do you want it to get better or worse?\n\r",ch);
		return;
	    }

	    if(!str_cmp("better",buffer))
		weather_info.change+=(dice(((level)/3),4));
	    else
		weather_info.change-=(dice(((level)/3),4)); 
	    break;
      default : 
	 log_hd("Serious screw-up in control weather!");
	 break;
    }
}

void cast_shield_room( byte level, struct char_data *ch, char *arg,int type,  struct char_data *tar_ch, struct obj_data *tar_obj )
{
struct room_affect *ra,*rap=NULL;
int room,hours;

  switch (type) {
    case SPELL_TYPE_SPELL:
    	if(!ch)return;
	room=ch->in_room;
    	if(room<0||room>=MAX_ROOM)return;
    	if(!world[room])return;
	ra=world[room]->room_afs;	 
	while(ra){
	    if(ra->type==RA_SHIELD){
	        send_to_char("Your concentration breaks down because the room is already shielded.\n\r",ch);
	    	return;
	    }
	    ra=ra->next;
	}
	hours=GET_LEVEL(ch)/2;
    	CREATE(rap,struct room_affect,1); 
    	rap->type=RA_SHIELD; 
    	rap->timer=hours; 
    	rap->value=0; 
    	rap->text=0;
	rap->ch=0;
    	rap->room=room; 
    	rap->next=world[room]->room_afs; 
    	world[room]->room_afs=rap; 
	send_to_char("You concentrate deep...feeling the area..anchoring yourself...\n\r",ch);
	send_to_room("
           ______...-----===***BOOOOOOOOOM!***===-----...______\n\r",ch->in_room);
	send_to_room("A dazzling shield of energy erupts and grows in size, surrounding the area.\n\r",room);
	break; 
      default : 
	log_hd("##Serious screw-up in shield_room!");
	break;
    }
}



void cast_create_food( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
	    act("$n magically creates a mushroom.",FALSE, ch, 0, 0, TO_ROOM);
	 spell_create_food(level,ch,0,0);
	    break;
    case SPELL_TYPE_SCROLL:
	 act("$n magically creates a mushroom.",FALSE, ch, 0, 0, TO_ROOM); 
	 spell_create_food(level,ch,0,0);
	    break;
    default : 
	 log_hd("Serious screw-up in create food!");
	 break;
    }
}

void cast_scribe( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
struct obj_data *obj=NULL;
char buf[MAX_INPUT_LENGTH];
int obj_num=0;


    if(!ch)return;

    switch (type) {
    	case SPELL_TYPE_SPELL:
		strcpy(buf, arg);
	    if(!buf||!buf[0]){
		send_to_char("Scribe a scroll...but make what scroll?\n\r",ch);
		return;
	    }
	    if(!str_cmp(buf,"identify"))
		obj_num=3040;
	    else if(!str_cmp(buf,"detect magic"))
		obj_num=3012;		    
	    else if(!str_cmp(buf,"detect invisibility"))
		obj_num=3013;		    
	    else if(!str_cmp(buf,"invisibility"))
		obj_num=3014;		    
	    else if(!str_cmp(buf,"faerie fire"))
		obj_num=3015;		    
	    else if(!str_cmp(buf,"refresh"))
		obj_num=3016;		    
	    else if(!str_cmp(buf,"infravision"))
		obj_num=3017;		    
	    else if(!str_cmp(buf,"fly"))
		obj_num=3018;		    
	    else if(!str_cmp(buf,"mass levitation"))
		obj_num=3019;		    
	    else if(!str_cmp(buf,"breathe water"))
		obj_num=3023;		    
	    else if(!str_cmp(buf,"mass invisibility"))
		obj_num=3026;		    
	    else if(!str_cmp(buf,"enchant weapon"))
		obj_num=1363;		    
	    if(!obj_num){
		send_to_char("You must be confused.\n\r",ch);
		return;
	    }
	    obj=ch->equipment[HOLD];
	    if(!obj||obj->obj_flags.type_flag != ITEM_NOTE){
		send_to_char("You remember you must be Holding some sort of paper or note.\n\r",ch);
	   	return;
	    }
	    if(amount_blood_in_room(ch->in_room)<4){
		send_to_char("There doesn't seem to be enough blood here..\n\r",ch);
		return;
	    }
	    act("$n throws $s hands up and $s hands start bleeding terribly.
",TRUE,ch,0,0,TO_ROOM);
	    send_to_char("You throw your hands up in the air and they start bleeding terribly..\n\r
",ch);
	    remove_blood(ch->in_room,4);
	    unequip_char( ch, HOLD ); 
	    extract_obj(obj);
	    obj=read_object(obj_num,REAL);
	    obj_to_char(obj,ch);
	    act("
$n scribes some symbols on the paper with some blood and utters some words..
$e rolls the new scroll up and utters more words....
You feel sudden VERTIGO just as the scroll turns bright yellow for a moment!",TRUE,ch,0,0,TO_ROOM);
	    send_to_char("
You scribe some symbols on the paper with some blood and utter the words...
As you roll the new scroll up you FOCUS .......uttering words as they appear
in your head...your senses go haywire..vertigo slams into you....
You open your eyes just as the scroll cools down...you smile.\n\r",ch);
	    break;
	default:
	    log_hd("Serious screw_up in scribe!");
	    break;
    }
}

void cast_farsight( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
int dir=0,from_room,deep=0,to_room;
char direction[10];
struct FREIGHT *stpFreight=NULL;

    if(!ch)return;
    switch (type) {
    	case SPELL_TYPE_SPELL:
    	case SPELL_TYPE_SCROLL:
	    from_room=ch->in_room;
	    if(!arg||!arg[0]){
		send_to_char("You try and focus....but focusin what direction?\n\r",ch);
		return;
	    }
	    switch(arg[0]){
		case 'n':
		case 'N':
		    dir=NORTH;
		    strcpy(direction,"North");
		    break;
		case 's':
		case 'S':
		    dir=SOUTH;
		    strcpy(direction,"South");
		    break;
		case 'e':
		case 'E':
		    dir=EAST;
		    strcpy(direction,"East");
		    break;
		case 'w':
		case 'W':
		    dir=WEST;
		    strcpy(direction,"West");
		    break;
		case 'u':
		case 'U':
		    dir=UP;
		    strcpy(direction,"Up");
		    break;
		case 'd':
		case 'D':
		    dir=DOWN;
		    strcpy(direction,"Down");
		    break;
		default:
		    send_to_char("You realize you dont understand that direction.\n\r",ch);
		    return;
	    }
	    global_color=33;
	    sprintf(log_buf,"$n inhales...looks %s and folds $s hands across $s chest.",direction);
	    act(log_buf,TRUE,ch,0,0,TO_ROOM);
	    send_to_char("You inhale and concentrate.....\n\r",ch);
	    global_color=0;
	    ch->specials.setup_page=1;
	    page_setup[0]=MED_NULL;
		stpFreight=ch->p->stpFreight;
		NODEATHTRAP=TRUE;
	    while(++deep<10){
			if(world[ch->in_room]->dir_option[dir]){
			    if(!CAN_GO(ch,dir))
					break;
			    if(!is_allowed_here(ch,world[ch->in_room]->dir_option[dir]->to_room))
					break;
			    to_room=world[ch->in_room]->dir_option[dir]->to_room;
			    char_from_room(ch);
			    char_to_room(ch,to_room);
		    	do_look(ch,"",15);
		  	    do_exits(ch,"",9);
			}
	    }
	    ch->specials.setup_page=0;
	    page_string(ch->desc,page_setup,1);
	    char_from_room(ch);
	    char_to_room(ch,from_room);
		ch->p->stpFreight=stpFreight;
		NODEATHTRAP=FALSE;
	    break;
    default : 
	 log_hd("Serious screw-up in farsight!");
	 break;
    }
}

void cast_ethereal( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
int dir=0,from_room,to_room;
char direction[10];
    if(!ch)return;
    switch (type) {
    	case SPELL_TYPE_SPELL:
send_to_char("SPELL is temporarily disabled.\n\r",ch);
return;
	    from_room=ch->in_room;
	    if(!arg||!arg[0]){
		send_to_char("You try and focus....but focus in what direction?\n\r",ch);
		return;
	    }
	    switch(arg[0]){
		case 'n':
		case 'N':
		    dir=NORTH;
		    strcpy(direction,"North");
		    break;
		case 's':
		case 'S':
		    dir=SOUTH;
		    break;
		    strcpy(direction,"South");
		case 'e':
		case 'E':
		    dir=EAST;
		    strcpy(direction,"East");
		    break;
		case 'w':
		case 'W':
		    dir=WEST;
		    strcpy(direction,"West");
		    break;
		case 'u':
		case 'U':
		    dir=UP;
		    strcpy(direction,"Up");
		    break;
		case 'd':
		case 'D':
		    dir=DOWN;
		    strcpy(direction,"Down");
		    break;
		default:
		    send_to_char("You realize you dont understand that direction.\n\r",ch);
		    return;
	    }
	    global_color=33;
	    sprintf(log_buf,"$n inhales...looks %s and folds $s hands across $s chest.",direction);
	    act(log_buf,TRUE,ch,0,0,TO_ROOM);
	    send_to_char("You inhale and concentrate.....\n\r",ch);
	    global_color=0;
	    if(world[ch->in_room]->dir_option[dir]){
		    if(!is_allowed_here(ch,world[ch->in_room]->dir_option[dir]->to_room)){
			act("$n looks visibly shaken...",TRUE,ch,0,0,TO_ROOM);
			return;
		    }
		    sprintf(log_buf,"The image of $n fades some then becomes transparent..$e walks %s.",direction);
		    act(log_buf,TRUE,ch,0,0,TO_ROOM);
		    sprintf(log_buf,"You feel disoriented..then amazingly light..you walk %s....\n\r",direction);
		    send_to_char(log_buf,ch);		
		    to_room=world[ch->in_room]->dir_option[dir]->to_room;
		    char_from_room(ch);
		    char_to_room(ch,to_room);
		    do_look(ch,"",15);
	  	    do_exits(ch,"",9);
		    return;
	    }else{
		send_to_char("That direction seems to push you away.\n\r",ch);
		return;
	    }
	    break;
    default: 
	 log_hd("Serious screw-up in ethereal!");
	 break;
    }
}


void cast_wizard_eye( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
 extern void DrawHoloMap(struct char_data *ch, int wid, int len);
  
 int iWid=50, iLen=17;
 
 if(IS_NPC(ch)) return;

 if(ch->in_room == NOWHERE) return;

 if(!world[ch->in_room]->holox || !world[ch->in_room]->holoy) {
	send_to_char("You feel unable to concentrate your inner eye here.\r\n",ch);
	return;
 }

 if(level >= 30)
	DrawHoloMap(ch,73,24);
 else if(level >= 27)
	DrawHoloMap(ch,60,21);
 else if(level >= 24)
	DrawHoloMap(ch,50,19); 
 else 
	DrawHoloMap(ch,45,17);
 
 act("$n breathes in deeply for a few moments while a white light phases through $s body.",TRUE,ch,0,0,TO_ROOM);
 
 return;
}

void cast_create_water( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
      send_to_char("It is unable to hold water.\n\r", ch);
      return;
    }
    spell_create_water(level,ch,0,tar_obj);
    break;
    default : 
      log_hd("Serious screw-up in create water!");
    break;
  }
}



void cast_cure_blind( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cure_blind(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_blind(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cure_blind(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people ; 
     tar_ch ; tar_ch = tar_ch->next_in_room)

    spell_cure_blind(level,ch,tar_ch,0);
    break;
    default : 
      log_hd("Serious screw-up in cure blind!");
    break;
  }
}

void cast_resurrect( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
char buf[MAX_INPUT_LENGTH];
struct char_data *vict=NULL;
int x,y;
float f, hits, h;

    switch (type) {
    case SPELL_TYPE_WAND:
	vict=tar_ch;
	break;
    case SPELL_TYPE_SPELL:
	if(!arg||!arg[0]){
	    send_to_char("Resurrect who?\n\r",ch);
	    return;
	}
	one_argument(arg,buf);
	vict=get_char_room_vis(ch,buf);
	break;
    }

	if(!vict){
	    send_to_char("You dont see that person..\n\r",ch);
	    return;
	}
	if(!vict->desc){
	    return;
	}
	if(STATE(vict->desc)!=CON_HOVERING){
	    send_to_char("You can only resurrect dead corpses before necromancer takes them away.\n\r",ch);
	    return;
	}
	hits=GET_MAX_HIT(vict);
	h=0;
        for(x=0;x<3;x++){
	    for(y=0;y<3;y++){
	    	if(ch->master->formation[x][y])
		    if(ch->in_room==ch->master->formation[x][y]->in_room)
			if(ch->master->formation[x][y]!=vict)
		    	    h+=(GET_HIT(ch->master->formation[x][y])-10);
	
	    }
	}
  	if(h<hits){
	    send_to_char("You could not gather enough strength from your comrads.\n\r",ch);
	    return;
	}
	f=hits/h;
        for(x=0;x<3;x++){
	    for(y=0;y<3;y++){
	    	if(ch->master->formation[x][y])
		    if(ch->in_room==ch->master->formation[x][y]->in_room)
			if(ch->master->formation[x][y]!=vict){
			    GET_HIT(ch->master->formation[x][y])-=((int)(f*(float)GET_HIT(ch->master->formation[x][y])));
			    send_to_char("You feel your strength drain into the corpse...\n\r",ch->master->formation[x][y]);	
			}
	    }
	}
	STATE(vict->desc)=CON_PLAYING;
	GET_POS(vict)=POSITION_RESTING;
	vict->specials.death_timer=0;
	GET_HIT(vict)=GET_MAX_HIT(vict)/10;
	while ( vict->affected )
            affect_remove( vict, vict->affected );
	SAVE_CHAR_OBJ(vict, -20);
	act("$n bends over the corpse of $N and utters some words.",TRUE,ch,0,vict,TO_NOTVICT);
	act("$n bends over your limp body and utters some words.",TRUE,ch,0,vict,TO_VICT);
	act("You bend over $N's limp body and utter some words.",TRUE,ch,0,vict,TO_CHAR);
	global_color=33;
	send_to_room("A loud WOOSH rips into the area!\n\r",ch->in_room);
	global_color=0;
	do_emote(vict,"SCREAMS suddenly and sits up!",9);
}


void cast_bloodbath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

struct char_data *p;

    if(level>30)level=30;
    switch (type) {
  	case SPELL_TYPE_SPELL:
	    if(amount_blood_in_room(ch->in_room)<10){
		send_to_char("As you start concentrating you realize there is not enough blood here\n\r",ch);
	 	return;
	    }
	    global_color=32;
	    act("$n Throws $s hands high in the air...Thunder rolls close by....
You GASP as $e starts bleeding all over....
",TRUE,ch,0,0,TO_ROOM);
	    
	    remove_blood(ch->in_room,10);
	    for(p=world[ch->in_room]->people;p;p=p->next_in_room){
		if(!IS_NPC(p)){
		    send_to_char("
Your blood HEATS up..you see red.\n\r",p);
		    GET_HIT(p)=GET_MAX_HIT(p);
		}
	    }
	
	    global_color=0;
	break;
    }
}

void cast_cure_critic( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
	if(GET_LEVEL(ch) == 32)
		sprintf(log_buf,"## %s(32) casts cure critic on %s(%d)",
			GET_NAME(ch), GET_NAME(tar_ch), GET_LEVEL(tar_ch));
    spell_cure_critic(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_critic(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cure_critic(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people ; 
     tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_cure_critic(level,ch,tar_ch,0);
    break;
    default : 
      log_hd("Serious screw-up in cure critic!");
    break;

  }
}



void cast_cure_light( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
	if(GET_LEVEL(ch) == 32)
		sprintf(log_buf,"## %s(32) casts cure light on %s(%d)",
			GET_NAME(ch), GET_NAME(tar_ch), GET_LEVEL(tar_ch));
    spell_cure_light(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_cure_light(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cure_light(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people ; 
     tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_cure_light(level,ch,tar_ch,0);
    break;
    default : 
      log_hd("Serious screw-up in cure light!");
    break;
  }
}


void cast_curse( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if (IS_SET(world[ch->in_room]->room_flags, SAFE)){
    send_to_char("You can not curse someone in a safe area!\n\r", ch);
    return;
  }
  if(level>30)level=30;
  
  switch (type) {
    case SPELL_TYPE_SPELL:
	    if (tar_obj)
		spell_curse(level,ch,0,tar_obj);
	    else
		spell_curse(level,ch,tar_ch,0);
	    break;
    case SPELL_TYPE_POTION:
	    spell_curse(level,ch,ch,0);
	    break;
    case SPELL_TYPE_SCROLL:
	    if (tar_obj)   /* It is an object */ 
		spell_curse(level,ch,0,tar_obj);
	    else {              /* Then it is a PC | NPC */
		if (!tar_ch) tar_ch = ch;
		spell_curse(level,ch,tar_ch,0);
	    }
	    break;
    case SPELL_TYPE_STAFF:
	 for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
	    if ( IS_NPC(tar_ch) )
	       spell_curse(level,ch,tar_ch,0);
	 break;
    default : 
	 log_hd("Serious screw-up in curse!");
	 break;
    }
}

void cast_detect_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_detect_evil(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_detect_evil(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_detect_evil(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people ; 
     tar_ch ; tar_ch = tar_ch->next_in_room)
      spell_detect_evil(level,ch,tar_ch,0);
    break;
    default : 
      log_hd("Serious screw-up in detect evil!");
    break;
  }
}


void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_POTION:
    spell_detect_invisibility(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_detect_invisibility(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
		spell_detect_invisibility(level,ch,tar_ch,0);
    break;
    default : 
      log_hd("Serious screw-up in detect invisibility!");
    break;
  }
}

void cast_detect_magic( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_detect_magic(level,ch,tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_detect_magic(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_detect_magic(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
		spell_detect_magic(level,ch,tar_ch,0);
    break;
    default : 
      log_hd("Serious screw-up in detect magic!");
    break;
  }
}

void cast_detect_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_detect_poison(level, ch, tar_ch,tar_obj);
    break;
  case SPELL_TYPE_POTION:
    spell_detect_poison(level, ch, ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {
      spell_detect_poison(level, ch, 0, tar_obj);
      return;
    }
    if (!tar_ch) tar_ch = ch;
    spell_detect_poison(level, ch, tar_ch, 0);
    break;
    default : 
      log_hd("Serious screw-up in detect poison!");
    break;
  }
}


void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_dispel_evil(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_POTION:
    spell_dispel_evil(level,ch,ch,0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_dispel_evil(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    spell_dispel_evil(level, ch, tar_ch,0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people ; 
     tar_ch ; tar_ch = tar_ch->next_in_room)
      if ( IS_NPC(tar_ch) )
    spell_dispel_evil(level,ch,tar_ch,0);
    break;
    default : 
      log_hd("Serious screw-up in dispel evil!");
    break;
  }
}


void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_enchant_weapon(level, ch, 0,tar_obj);
	    break;
    case SPELL_TYPE_SCROLL:
	    if(!tar_obj) return;
	    spell_enchant_weapon(level, ch, 0,tar_obj);
	    break;
    default : 
      log_hd("Serious screw-up in enchant weapon!");
      break;
    }
}


void cast_heal( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	 if(GET_LEVEL(ch) == 32)
		sprintf(log_buf,"## %s(32) heals %s(%d)",
			GET_NAME(ch), GET_NAME(tar_ch), GET_LEVEL(tar_ch));
	 spell_heal(level, ch, tar_ch, 0);
	 break;
    case SPELL_TYPE_POTION:
	 spell_heal(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_STAFF:
	 for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
       spell_heal(level,ch,tar_ch,0);
	 break;
	case SPELL_TYPE_SCROLL:
		if(!tar_ch) tar_ch=ch;
		act("Magical energy surrounds $N's body, healing $m.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
		act("The text turns to smoke and rises off the scroll. $N seems healed.", FALSE, ch, 0, tar_ch, TO_CHAR);
		spell_heal(level, ch, tar_ch, 0);
		break;
    default : 
	 log_hd("Serious screw-up in heal!");
	 break;
    }
}


void cast_invisibility( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    if(!tar_ch)tar_ch=ch;
    if (tar_obj) {
      if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE) )
    send_to_char("Nothing new seems to happen.\n\r", ch);
      else
    spell_invisibility(level, ch, 0, tar_obj);
    } else
	spell_invisibility(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_invisibility(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) {
      if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
    spell_invisibility(level, ch, 0, tar_obj);
    } else { /* tar_ch */
      if (!tar_ch) tar_ch = ch;
      spell_invisibility(level, ch, tar_ch, 0);
    }
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) {
      if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
    spell_invisibility(level, ch, 0, tar_obj);
    } else
      spell_invisibility(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people ; 
	 tar_ch ; tar_ch = tar_ch->next_in_room)
		spell_invisibility(level,ch,tar_ch,0);
    break;
    default : 
      log_hd("Serious screw-up in invisibility!");
    break;
  }
}


void cast_locate_object( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_locate_object(level, ch, arg, 0, tar_obj);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_ch) return;
    spell_locate_object(level, ch, arg, 0, tar_obj);
    break;
    default : 
      log_hd("Serious screw-up in locate object!");
    break;
  }
}


void cast_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if (IS_SET(world[ch->in_room]->room_flags, SAFE)){
    send_to_char("You can not poison someone in a safe area!\n\r",ch);
    return;
  }
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_poison(level, ch, tar_ch, tar_obj);
	    break;
    case SPELL_TYPE_POTION:
	    spell_poison(level, ch, ch, 0);
	    break;
    case SPELL_TYPE_STAFF:
	 for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
	    if ( IS_NPC(tar_ch) )
		  spell_poison(level,ch,tar_ch,0);
	 break;
    default : 
	 log_hd("Serious screw-up in poison!");
	 break;
    }
}


void cast_protection_from_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	spell_protection_from_evil(level, ch, tar_ch, 0);
	break;
    case SPELL_TYPE_POTION:
	spell_protection_from_evil(level, ch, ch, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if(tar_obj) return;
	if(!tar_ch) tar_ch = ch;
	    spell_protection_from_evil(level, ch, tar_ch, 0);
	break;
    case SPELL_TYPE_STAFF:
	for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
		  spell_protection_from_evil(level,ch,tar_ch,0);
	 break;
    default : 
	 log_hd("Serious screw-up in protection from evil!");
	 break;
    }
}

void cast_protection_from_good( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	spell_protection_from_good(level, ch, tar_ch, 0);
	break;
    case SPELL_TYPE_POTION:
	spell_protection_from_good(level, ch, ch, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if(tar_obj) return;
	if(!tar_ch) tar_ch = ch;
	    spell_protection_from_good(level, ch, tar_ch, 0);
	break;
    case SPELL_TYPE_STAFF:
	for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
		  spell_protection_from_good(level,ch,tar_ch,0);
	 break;
    default : 
	 log_hd("Serious screw-up in protection from good!");
	 break;
    }
}


void cast_remove_curse( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
	spell_remove_curse(level, ch, tar_ch, tar_obj);
	break;
    case SPELL_TYPE_POTION:
	spell_remove_curse(level, ch, ch, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if(tar_obj) {
	  spell_remove_curse(level, ch, 0, tar_obj);
	  return;
	}
	if(!tar_ch) tar_ch = ch;
	  spell_remove_curse(level, ch, tar_ch, 0);
	break;
    case SPELL_TYPE_STAFF:
	for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
		  spell_remove_curse(level,ch,tar_ch,0);
	 break;
    default : 
	 log_hd("Serious screw-up in remove curse!");
	 break;
    }
}


void cast_remove_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
	 spell_remove_poison(level, ch, tar_ch, tar_obj);
	 break;
    case SPELL_TYPE_POTION:
	 spell_remove_poison(level, ch, ch, 0);
	 break;
    case SPELL_TYPE_STAFF:
	 for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
		  spell_remove_poison(level,ch,tar_ch,0);
	 break;
    default : 
	 log_hd("Serious screw-up in remove poison!");
	 break;
    }
}


void cast_fireshield( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
	 spell_fireshield(level, ch, 0, 0);
	 break;
    case SPELL_TYPE_SCROLL:
	 if (tar_obj) return;
	 if (!tar_ch) tar_ch = ch;
	   spell_fireshield(level, ch, tar_ch, 0);
	   break;
    case SPELL_TYPE_STAFF:
	 for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
		  spell_fireshield(level,ch,tar_ch,0);
	 break;
    default : 
	 log_hd("Serious screw-up in fireshield!");
	 break;
    }
}


void cast_sanctuary( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	if(GET_LEVEL(ch) == 32)
		sprintf(log_buf,"## %s(32) casts sanctuary on %s(%d)",
			GET_NAME(ch), GET_NAME(tar_ch), GET_LEVEL(tar_ch));
	spell_sanctuary(level, ch, tar_ch, 0);
	break;
    case SPELL_TYPE_POTION:
	spell_sanctuary(level, ch, ch, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if(tar_obj)
		return;
	if(!tar_ch) tar_ch = ch;
	    spell_sanctuary(level, ch, tar_ch, 0);
	    break;
    case SPELL_TYPE_STAFF:
	for (tar_ch = world[ch->in_room]->people ; 
		tar_ch ; tar_ch = tar_ch->next_in_room)
	  spell_sanctuary(level,ch,tar_ch,0);
	break;
    default : 
	log_hd("Serious screw-up in sanctuary!");
	break;
    }
}


void cast_mass_levitation( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SCROLL:
	if (tar_obj) return;
    case SPELL_TYPE_SPELL:
	for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
	    spell_fly(level, ch, tar_ch, 0);
	    break;
    default : 
	 log_hd("Serious screw-up in mass_levitation!");
	 break;
    }
}


void cast_transport( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
int x,y,location;

  switch (type) {
    case SPELL_TYPE_SPELL:
	location=ch->in_room;
        for(x=0;x<3;x++)
	  for(y=0;y<3;y++)
	{
	    if((ch->master->formation[x][y]) 
			&& (location == ch->master->formation[x][y]->in_room))
	        spell_word_of_recall(level, ch, ch->master->formation[x][y],0);
	}
	    break;
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
	 break;
    default : 
	 log_hd("Serious screw-up in mass_levitation!");
	 break;
    }
}


void cast_sleep( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if (IS_SET(world[ch->in_room]->room_flags, SAFE)){
    send_to_char("You can not sleep someone in a safe area!\n\r", ch);
    return;
  }
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_sleep(level, ch, tar_ch, 0);
	    break;
    case SPELL_TYPE_POTION:
	    spell_sleep(level, ch, ch, 0);
	    break;
    case SPELL_TYPE_SCROLL:
	 if(tar_obj) return;
	 if (!tar_ch) tar_ch = ch;
	 spell_sleep(level, ch, tar_ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(tar_obj) return;
	 spell_sleep(level, ch, tar_ch, 0);
	 break;
    case SPELL_TYPE_STAFF:
	 for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
	    if ( IS_NPC(tar_ch) )
		  spell_sleep(level,ch,tar_ch,0);
	 break;
    default : 
	 log_hd("Serious screw-up in sleep!");
	 break;
    }
}


void cast_strength( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_strength(level, ch, tar_ch, 0);
	    break;
    case SPELL_TYPE_POTION:
	    spell_strength(level, ch, ch, 0);
	    break;
    case SPELL_TYPE_SCROLL:
	 if(tar_obj) return;
	 if (!tar_ch) tar_ch = ch;
	 spell_strength(level, ch, tar_ch, 0);
	 break;
    case SPELL_TYPE_STAFF:
	 for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)

		  spell_strength(level,ch,tar_ch,0);
	 break;
    default : 
	 log_hd("Serious screw-up in strength!");
	 break;
    }
}


void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
    struct char_data *tmp_ch;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];

    if (type != SPELL_TYPE_SPELL) {
	log_hd("Attempt to ventriloquate by non-cast-spell.");
	return;
    }
    for(; *arg && (*arg == ' '); arg++);
    if (tar_obj) {
	sprintf(buf1, "The %s says '%s'\n\r", fname(tar_obj->name), arg);
	sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
	  fname(tar_obj->name), arg);
    }   else {
	sprintf(buf1, "%s says '%s'\n\r", GET_NAME(tar_ch), arg);
	sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
	  GET_NAME(tar_ch), arg);
    }

    sprintf(buf3, "Someone says, '%s'\n\r", arg);

    for (tmp_ch = world[ch->in_room]->people; tmp_ch;
      tmp_ch = tmp_ch->next_in_room) {

	if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
	    if ( saves_spell(tmp_ch, SAVING_SPELL) )
		send_to_char(buf2, tmp_ch);
	    else
		send_to_char(buf1, tmp_ch);
	} else {
	    if (tmp_ch == tar_ch)
		send_to_char(buf3, tar_ch);
	}
    }
}


void cast_word_of_recall( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  struct char_data *tar_ch_next=NULL;

  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_word_of_recall(level, ch, ch, 0);
	    break;
    case SPELL_TYPE_POTION:
	    spell_word_of_recall(level, ch, ch, 0);
	    break;
    case SPELL_TYPE_SCROLL:
	 if(tar_obj) return;
	 if (!tar_ch) tar_ch = ch;
	 spell_word_of_recall(level, ch, tar_ch, 0);
	 break;
    case SPELL_TYPE_WAND:
	 if(tar_obj) return;
	 spell_word_of_recall(level, ch, tar_ch, 0);
	 break;
    case SPELL_TYPE_STAFF:
	for (tar_ch = world[ch->in_room]->people ; 
	    tar_ch ; tar_ch = tar_ch_next)
	{
	    tar_ch_next = tar_ch->next_in_room;
	    if ( !IS_NPC(tar_ch) )
		spell_word_of_recall(level,ch,tar_ch,0);
	}
	break;
    default : 
	 log_hd("Serious screw-up in word of recall!");
	 break;
    }
}


void cast_summon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_summon(level, ch, tar_ch, 0);
	    break;
      default : 
	 log_hd("Serious screw-up in summon!");
	 break;
    }
}


void cast_charm_person( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  if(level>30)level=30;
  switch (type) {
	case SPELL_TYPE_SPELL:
	    spell_charm_person(level, ch, tar_ch, 0);
	    break;
      case SPELL_TYPE_SCROLL:
	 if(!tar_ch) return;
	 spell_charm_person(level, ch, tar_ch, 0);
	 break;
      case SPELL_TYPE_STAFF:
	 for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
	    if ( IS_NPC(tar_ch) )
		  spell_charm_person(level,ch,tar_ch,0);
	 break;
      case SPELL_TYPE_WAND:
	if (tar_ch&&IS_NPC(tar_ch))
	  spell_charm_person(level, ch, tar_ch, 0);
	break;
      default : 
	 log_hd("Serious screw-up in charm person!");
	 break;
    }
}


void cast_sense_life( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_sense_life(level, ch, ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_sense_life(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    spell_sense_life(level, ch, ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people ; 
     tar_ch ; tar_ch = tar_ch->next_in_room)
	spell_sense_life(level,ch,tar_ch,0);
    break;
    default : 
      log_hd("Serious screw-up in sense life!");
    break;
  }
}


void cast_identify( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_identify(level, ch, tar_ch, tar_obj);
    break;
  case SPELL_TYPE_SCROLL:
    spell_identify(level, ch, tar_ch, tar_obj);
    break;
  default:
      log_hd("Serious screw-up in identify!");
    break;
    }
}


void cast_fire_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_fire_breath(level, ch, tar_ch, 0);
	    break;   /* It's a spell.. But people can't cast it! */
      default : 
	 log_hd("Serious screw-up in firebreath!");
	 break;
    }
}

void cast_frost_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_frost_breath(level, ch, tar_ch, 0);
	    break;   /* It's a spell.. But people can't cast it! */
      default : 
	 log_hd("Serious screw-up in frostbreath!");
	 break;
    }
}

void cast_acid_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_acid_breath(level, ch, tar_ch, 0);
	    break;   /* It's a spell.. But people can't cast it! */
      default : 
	 log_hd("Serious screw-up in acidbreath!");
	 break;
    }
}

void cast_gas_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	for (tar_ch = world[ch->in_room]->people ; 
		tar_ch ; tar_ch = tar_ch->next_in_room)
	    if ( !IS_NPC(tar_ch) )
		spell_gas_breath(level,ch,tar_ch,0);
	 break;
	    /* THIS ONE HURTS!! */
      default : 
	 log_hd("Serious screw-up in gasbreath!");
	 break;
    }
}

void cast_lightning_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  if(level>30)level=30;
  switch (type) {
    case SPELL_TYPE_SPELL:
	    spell_lightning_breath(level, ch, tar_ch, 0);
	    break;   /* It's a spell.. But people can't cast it! */
      default : 
	 log_hd("Serious screw-up in lightningbreath!");
	 break;
    }
}

void cast_fear( byte level, struct char_data *ch, char *arg, int type,
	   struct char_data *tar_ch, struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fear(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_fear(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_fear(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
     tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
    spell_fear(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in fear!");
    break;
  }
}

void cast_refresh( byte level, struct char_data *ch, char *arg, int type,
	  struct char_data *tar_ch, struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
	if(GET_LEVEL(ch) == 32)
		sprintf(log_buf,"## %s(32) casts refresh on %s(%d)",
			GET_NAME(ch), GET_NAME(tar_ch), GET_LEVEL(tar_ch));
    spell_refresh(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_refresh(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_refresh(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)

	spell_refresh(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in refresh!");
    break;
  }
}

void cast_fly( byte level, struct char_data *ch, char *arg, int type,
	  struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_fly(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_fly(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_fly(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_fly(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)

	spell_fly(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in fly!");
    break;
  }
}

void cast_breath_water( byte level, struct char_data *ch, char *arg, int type,
	  struct char_data *tar_ch, struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_breath_water(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_breath_water(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_breath_water(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)

	spell_breath_water(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in breath_water!");
    break;
  }
}

void cast_cont_light( byte level, struct char_data *ch, char *arg, int type,
	     struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cont_light(level, ch, 0, 0);
    break;
  case SPELL_TYPE_SCROLL:
    spell_cont_light(level, ch, 0, 0);
    break;
  default:
    log_hd("Serious screw-up in cont_light");
    break;
  }
}

void cast_know_alignment(byte level, struct char_data *ch, char *arg, int type,
	     struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_know_alignment(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_know_alignment(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_know_alignment(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      spell_know_alignment(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in know alignment!");
    break;
  }
}

void cast_dispel_magic( byte level, struct char_data *ch, char *arg, int type,
	       struct char_data *tar_ch, struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_dispel_magic(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_dispel_magic(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_dispel_magic(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      spell_dispel_magic(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in dispel magic!");
    break;
  }
}

void cast_conjure_elemental( byte level, struct char_data *ch,
		char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_conjure_elemental(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_conjure_elemental(level, ch, tar_ch, 0);  
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_conjure_elemental(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
         tar_ch; tar_ch = tar_ch->next_in_room)
      spell_conjure_elemental(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in conjure_elemental!");
    break;
  }
}

void cast_cure_serious( byte level, struct char_data *ch, char *arg, int type,
	       struct char_data *tar_ch, struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
	if(GET_LEVEL(ch) == 32)
		sprintf(log_buf,"## %s(32) casts cure serious on %s(%d)",
			GET_NAME(ch), GET_NAME(tar_ch), GET_LEVEL(tar_ch));
    spell_cure_serious(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cure_serious(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cure_serious(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      spell_cure_serious(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in cure serious!");
    break;
  }
}

void cast_cause_light( byte level, struct char_data *ch, char *arg, int type,
	      struct char_data *tar_ch, struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cause_light(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cause_light(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cause_light(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_cause_light(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in cause light!");
    break;
  }
}

void cast_cause_critical( byte level, struct char_data *ch, char *arg,
	     int type, struct char_data *tar_ch,
	     struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cause_critical(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cause_critical(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cause_critical(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_cause_critical(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in cause critical!");
    break;
  }
}

void cast_cause_serious( byte level, struct char_data *ch, char *arg,
	    int type, struct char_data *tar_ch,
	    struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_cause_serious(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cause_serious(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_cause_serious(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_cause_serious(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in cause serious!");
    break;
  }
}

void cast_flamestrike( byte level, struct char_data *ch, char *arg,
	      int type, struct char_data *tar_ch,
	      struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_flamestrike(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_flamestrike(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_flamestrike(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_flamestrike(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in cause critical!");
    break;
  }
}

void cast_stone_skin( byte level, struct char_data *ch, char *arg,
	     int type, struct char_data *tar_ch,
	     struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
  case SPELL_TYPE_POTION:
  case SPELL_TYPE_SCROLL:
    spell_stone_skin(level, ch, 0, 0);
    break;
  default:
    log_hd("Serious screw-up in stone skin!");
    break;
  }
}

void cast_shield( byte level, struct char_data *ch, char *arg,
	 int type, struct char_data *tar_ch,
	 struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
     spell_shield(level, ch, tar_ch, 0);
     break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_shield(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_shield(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      spell_shield(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in shield!");
    break;
  }
}

void cast_weaken( byte level, struct char_data *ch, char *arg,
	 int type, struct char_data *tar_ch,
	 struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_weaken(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_weaken(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_weaken(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_weaken(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in weaken!");
    break;
  }
}


void cast_mass_invis( byte level, struct char_data *ch, char *arg,
	     int type, struct char_data *tar_ch,
	     struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SCROLL:
	if (tar_obj) return;
  case SPELL_TYPE_SPELL:
	 for (tar_ch = world[ch->in_room]->people ; 
	      tar_ch ; tar_ch = tar_ch->next_in_room)
		if (ch != tar_ch)
		  spell_invisibility(level, ch, tar_ch, 0);
		break;
  default:
    log_hd("Serious screw-up in mass invis!");
    break;
  }
}

void cast_acid_blast( byte level, struct char_data *ch, char *arg,
	     int type, struct char_data *tar_ch,
	     struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_acid_blast(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_acid_blast(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_acid_blast(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_acid_blast(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in acid blast!");
    break;
  }
}

void cast_gate( byte level, struct char_data *ch, char *arg,
	   int type, struct char_data *tar_ch,
	   struct obj_data *tar_obj)
{
char dir,buf[20];

    if(!ch)return;
    if(!world[ch->in_room])return;
    GET_HIT(ch)=GET_HIT(ch)/20;
    dir=0;
    act("$n crosses $s arms on $s chest and fills $s lungs with air.....",TRUE,ch,0,0,TO_ROOM);
    send_to_room("You cross your arms on your chest and breath in deeply...\n\r",ch->in_room);
    if(!world[ch->in_room]->dir_option[NORTH])dir='n';
    if(!world[ch->in_room]->dir_option[SOUTH])dir='s';
    if(!world[ch->in_room]->dir_option[WEST])dir='w';
    if(!world[ch->in_room]->dir_option[EAST])dir='e';
    if(!dir||world[ch->in_room]->zone!=198){
	send_to_char("You focus till you feel you will burst...but..couldn't seem to find the right direction...\n\r",ch);
	return;
    }
    if(gate_room){
	sprintf(buf,"%c",gate_dir);
	do_deletedoor(0,buf,gate_room);
	global_color=31;
	send_to_room("*****WWWOOOOOoooooSHSHHH!*** the portal collapses.\n\r",gate_room);
	global_color=0;
	gate_room=0;
	gate_dir=0;
    }  
    sprintf(buf,"%c 2713",dir);
    do_makedoor(0,buf,ch->in_room);
    gate_room=ch->in_room;
    gate_dir=dir;
    global_color=33;
    send_to_room("
The temperature in the area drops below zero.......then....
                    <<<<* CRACK *>>>>
A sudden wind almost knock you over..you notice a strange portal...
",ch->in_room);
    global_color=0;
}

void cast_faerie_fire( byte level, struct char_data *ch, char *arg,
	      int type, struct char_data *tar_ch,
	      struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_faerie_fire(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_faerie_fire(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_faerie_fire(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
      spell_faerie_fire(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in faerie fire!");
    break;
  }
}

void cast_faerie_fog( byte level, struct char_data *ch, char *arg,
	     int type, struct char_data *tar_ch,
	     struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_faerie_fog(level, ch, 0, 0);
    break;
  case SPELL_TYPE_SCROLL:
    spell_faerie_fog(level, ch, 0, 0);
    break;
  case SPELL_TYPE_STAFF:
    spell_faerie_fog(level, ch, 0, 0);
    break;
  default:
    log_hd("Serious screw-up in faerie fog!");
    break;
  }
}

void cast_drown( byte level, struct char_data *ch, char *arg,
	int type, struct char_data *tar_ch,
	struct obj_data *tar_obj)
{
  /*  Does not work yet  */
}

void cast_demonfire( byte level, struct char_data *ch, char *arg,
	    int type, struct char_data *tar_ch,
	    struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_demonfire(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_demonfire(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_demonfire(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_demonfire(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in demon fire!");
    break;
  }
}
void cast_hammer_of_faith( byte level, struct char_data *ch, char *arg,
	    int type, struct char_data *tar_ch,
	    struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_hammer_of_faith(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_hammer_of_faith(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_hammer_of_faith(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_hammer_of_faith(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in hammer of faith!");
    break;
  }
}

void cast_turn_undead( byte level, struct char_data *ch, char *arg,
	      int type, struct char_data *tar_ch,
	      struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_turn_undead(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_turn_undead(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_turn_undead(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch) && IS_SET(tar_ch->specials.act, ACT_UNDEAD))
	spell_turn_undead(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in turn undead!");
    break;
  }
}

void cast_infravision( byte level, struct char_data *ch, char *arg,
	      int type, struct char_data *tar_ch,
	      struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_infravision(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_POTION:
    spell_infravision(level, ch, ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_infravision(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)

	spell_infravision(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in infravision!");
    break;
  }
}

void cast_sandstorm( byte level, struct char_data *ch, char *arg,
	    int type, struct char_data *tar_ch,
	    struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_sandstorm(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_sandstorm(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_sandstorm(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_sandstorm(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in sandstorm!");
    break;
  }
}

void cast_hands_of_wind( byte level, struct char_data *ch, char *arg,
	    int type, struct char_data *tar_ch,
	    struct obj_data *tar_obj)
{
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_hands_of_wind(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in hands of wind!");
    break;
  }

}

void cast_plague( byte level, struct char_data *ch, char *arg,
	 int type, struct char_data *tar_ch,
	 struct obj_data *tar_obj)
{
  if(level>30)level=30;
  switch (type) {
  case SPELL_TYPE_SPELL:
    spell_plague(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_SCROLL:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_plague(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_WAND:
    if (tar_obj) return;
    if (!tar_ch) tar_ch = ch;
    spell_plague(level, ch, tar_ch, 0);
    break;
  case SPELL_TYPE_STAFF:
    for (tar_ch = world[ch->in_room]->people;
	 tar_ch; tar_ch = tar_ch->next_in_room)
      if (IS_NPC(tar_ch))
	spell_plague(level, ch, tar_ch, 0);
    break;
  default:
    log_hd("Serious screw-up in cause critical!");
    break;
  }
}


#if 0
void cast_animate_dead( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
			spell_animate_dead(level, ch, 0,tar_obj);
			break;

    case SPELL_TYPE_SCROLL:
			if(!tar_obj) return;
			spell_animate_dead(level, ch, 0,tar_obj);
			break;
    default : 
      log_hd("Serious screw-up in Animate Dead!");
      break;
	}
}
#endif
