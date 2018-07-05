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
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "structs.h" /*hard to live without this line :) */
#include "mob.h"
#include "obj.h"
#include "limits.h"
#include "utils.h"
#include "spells.h"
#include "db.h"

extern bool guy_deleted;
extern char global_color;
extern struct time_info_data time_info;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct damage_rooms *daroom_list;

/* External procedures */

void update_pos( struct char_data *victim );                 /* in fight.c */
struct time_info_data age(struct char_data *ch);
void tell_clan(int clan, char *argument);
extern void sort_descriptors(void);
extern struct obj_data *extract_obj(struct obj_data *);

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

    if (age < 15)
	return(p0);                               /* < 15   */
    else if (age <= 29) 
	return (int) (p1+(((age-15)*(p2-p1))/15));  /* 15..29 */
    else if (age <= 44)
	return (int) (p2+(((age-30)*(p3-p2))/15));  /* 30..44 */
    else if (age <= 59)
	return (int) (p3+(((age-45)*(p4-p3))/15));  /* 45..59 */
    else if (age <= 79)
	return (int) (p4+(((age-60)*(p5-p4))/20));  /* 60..79 */
    else
	return(p6);                               /* >= 80 */
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
      return(ch->points.max_mana);
}


int hit_limit(struct char_data *ch)
{
    int max;

    if (!IS_NPC(ch))
      max = (ch->points.max_hit) +
	(graf(age(ch).year, 2,4,17,14,8,4,3));
    else 
      max = (ch->points.max_hit);


/* Class/Level calculations */

/* Skill/Spell calculations */
    
  return (max);
}


int move_limit(struct char_data *ch)
{
    int max;

    if (!IS_NPC(ch))
	/* HERE SHOULD BE CON CALCULATIONS INSTEAD */
	max = (ch->points.max_move) + 
	  graf(age(ch).year, 50,70,160,120,100,40,20);
    else
	max = ch->points.max_move;

/* Class/Level calculations */

/* Skill/Spell calculations */

  return (max);
}




/* manapoint gain pr. game hour */
int mana_gain(struct char_data *ch)
{
    int gain;
    int divisor = 1000;

    if(IS_NPC(ch)) {
	/* Neat and fast */
	gain = GET_LEVEL(ch)*2;
    } else {
/*	gain = graf(age(ch).year, 2,4,6,8,6,5,8);
*/
	gain = graf(age(ch).year, 2,20,24,28,30,35,40);

	/* Class calculations */

	/* Skill/Spell calculations */

	/* Position calculations    */
	switch (GET_POS(ch)) {
	    case POSITION_SLEEPING: divisor = 1; break;
	    case POSITION_RESTING:  divisor = 4; break;
	    case POSITION_SITTING:  divisor = 8; break;
	}
	gain += (MAX(0,(GET_INT(ch)-13))+MAX(0,(GET_WIS(ch)-13))) / divisor;


	if (GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC){
	    if(world[ch->in_room]->zone==198){
		gain*=(world[ch->in_room]->mount_restriction+1);
		if(world[ch->in_room]->move_mod==2)/*mana room*/
		    gain*=10;
	    }else
              gain += gain;
        }
        else if(GET_CLASS(ch) == CLASS_THIEF)
          gain += gain/2;

        if(world[ch->in_room]->sector_type==SECT_MANA)
	       gain*=3;
        if(ch->equipment[WEAR_FINGER_R]){
          if(ch->equipment[WEAR_FINGER_R]->item_number==705)
            gain += 50;
        }
        if(ch->equipment[WEAR_FINGER_L]){
          if(ch->equipment[WEAR_FINGER_L]->item_number==705)
            gain += 50;
        }
    }

    if (IS_AFFECTED(ch,AFF_POISON))
	gain >>= 2;

    if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
	gain >>= 2;
 
  return (gain);
}


int hit_gain(struct char_data *ch)
/* Hitpoint gain pr. game hour */
{
    int gain;
    int divisor = 100000;

    if(IS_NPC(ch)) {
	gain = (GET_LEVEL(ch) * 3 ) / 2;
	/* Neat and fast */
    } else {

	gain = graf(age(ch).year, 2,13,15,18,6,4,2);

	/* Class/Level calculations */

	/* Skill/Spell calculations */

	/* Position calculations    */
	switch (GET_POS(ch)) {
	    case POSITION_SLEEPING: divisor = 1; break;
	    case POSITION_RESTING:  divisor = 2; break;
	    case POSITION_SITTING:  divisor = 2; break;
	}
	gain += GET_CON(ch) / divisor;

	if (GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC)
	    gain >>= 1;

        if(ch->equipment[WEAR_FINGER_R]){
          if(ch->equipment[WEAR_FINGER_R]->item_number==704)
            gain += 15;
        }
        if(ch->equipment[WEAR_FINGER_L]){
          if(ch->equipment[WEAR_FINGER_L]->item_number==704)
            gain += 15;
        }

  }

  if (IS_AFFECTED(ch,AFF_POISON))
    {
	gain >>= 2;
	if(damage(ch,ch,2,SPELL_POISON))
		return(gain);
    }

    if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
	gain >>= 2;

  return (gain);
}



int move_gain(struct char_data *ch)
/* move gain pr. game hour */
{
    int gain;
    int divisor = 100000;

    if(IS_NPC(ch) && !IS_UNDEAD(ch)) {
	return(GET_LEVEL(ch));  
	/* Neat and fast */
    } else {
		gain = graf(age(ch).year, 10,11,11,10,7,5,3);

	/* Class/Level calculations */

	if(GET_CLASS(ch)==CLASS_WARRIOR){
		gain += (GET_CON(ch) + GET_DEX(ch));
		gain*=3;
	  switch (GET_POS(ch)) {
	    case POSITION_SLEEPING: divisor = 1; break;
	    case POSITION_RESTING:  divisor = 2; break;
	    case POSITION_SITTING:  divisor = 2; break;
	  }
	  gain /= divisor;
	}
	else if(GET_CLASS(ch)==CLASS_MAGIC_USER){
		gain*=.5;
	  switch (GET_POS(ch)) {
	    case POSITION_SLEEPING: divisor = 1; break;
	    case POSITION_RESTING:  divisor = 3; break;
	    case POSITION_SITTING:  divisor = 6; break;
	  }
	  gain /= divisor;
	}
        else{
	  gain += (GET_CON(ch) + GET_DEX(ch));
	  switch (GET_POS(ch)) {
	    case POSITION_SLEEPING: divisor = 1; break;
	    case POSITION_RESTING:  divisor = 3; break;
	    case POSITION_SITTING:  divisor = 6; break;
	  }
	  gain /= divisor;
	}
	/* Skill/Spell calculations */
    }

        if(ch->equipment[WEAR_FINGER_R]){
          if(ch->equipment[WEAR_FINGER_R]->item_number==704)
            gain += 50;
        }
        if(ch->equipment[WEAR_FINGER_L]){
          if(ch->equipment[WEAR_FINGER_L]->item_number==704)
            gain += 50;
        }

    if (IS_AFFECTED(ch,AFF_POISON))
	gain >>= 2;

    if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
	gain >>= 2;

	if(IS_UNDEAD(ch))
	gain <<= 2;

    return (gain);
}



/* Gain maximum in various points */
void advance_level(struct char_data *ch)
{
    int add_hp;
    int add_mana = 0;
    int add_moves = 0;
    int add_practices;
    int i;
    char buf[MAX_STRING_LENGTH];

    extern struct wis_app_type wis_app[];
    extern struct con_app_type con_app[];

    
    add_hp = con_app[GET_CON(ch)].hitp;

    switch(GET_CLASS(ch))
    {
    case CLASS_MAGIC_USER:
	add_hp      += number(6, 8);
	add_mana    += number(2, (GET_INT(ch) + GET_WIS(ch))/6);
	add_moves   += number(5, (GET_CON(ch) + 2 * GET_DEX(ch)) / 5);
	break;

    case CLASS_CLERIC:
	add_hp      += number(7, 10);
	add_mana    += number(2, (GET_INT(ch) + GET_WIS(ch))/6);
	add_moves   += number(5, (GET_CON(ch) + 2 * GET_DEX(ch)) / 5);
	break;

    case CLASS_THIEF:
	add_hp      += number(8, 13);
	add_moves   += number(5, (GET_CON(ch) + 2 * GET_DEX(ch)) / 5);
	if(IS_SET(ch->player.multi_class,MULTI_CLASS_MAGIC_USER)
	||IS_SET(ch->player.multi_class,MULTI_CLASS_CLERIC))
	    add_mana    += number(2, (GET_INT(ch) + GET_WIS(ch))/6);
	break;

    case CLASS_WARRIOR:
	add_hp      += number(11, 15);
	add_moves   += number(5, (GET_CON(ch) + 2 * GET_DEX(ch)) / 5);
	if(IS_SET(ch->player.multi_class,MULTI_CLASS_MAGIC_USER)
	||IS_SET(ch->player.multi_class,MULTI_CLASS_CLERIC))
	    add_mana    += number(2, (GET_INT(ch) + GET_WIS(ch))/6);
	break;
    }

    add_hp			 = MAX( 1, add_hp);
    add_mana			 = MAX( 0, add_mana);
    add_moves			 = MAX(10, add_moves);
    add_practices		 = wis_app[GET_WIS(ch)].bonus;
    ch->points.max_hit		+= add_hp;
    ch->points.max_mana		+= add_mana;
    ch->points.max_move		+= add_moves;
    ch->specials.practices	+= add_practices;

    sprintf( buf,
	"Your gain is: %d/%d hp, %d/%d m, %d/%d mv %d/%d prac.\n\r",
	add_hp,        GET_MAX_HIT(ch),
	add_mana,      GET_MAX_MANA(ch),
	add_moves,     GET_MAX_MOVE(ch),
	add_practices, ch->specials.practices
	);
    send_to_char( buf, ch );

    if (GET_LEVEL(ch) > 31)
	for (i = 0; i < 3; i++)
	    ch->specials.conditions[i] = -1;
}   


void set_title(struct char_data *ch)
{
    FREE( GET_TITLE(ch) );
    GET_TITLE(ch) = str_dup(
	title_table [GET_CLASS(ch)-1]
	    [(int) GET_LEVEL(ch)] [GET_SEX(ch)==SEX_FEMALE]
	);
    if(!ch->player.title){
	FREE(GET_TITLE(ch));
	GET_TITLE(ch)=str_dup(no_title);
    }
    return;
}



void gain_exp( CHAR_DATA *ch, int gain, CHAR_DATA *victim )
{
	char my_buf[255];

    if(!ch)
		return;
    if(IS_NPC(ch))
		return;

 
    if(!IS_NPC(ch)
		&& victim
		&& IS_SET(world[ch->in_room]->room_flags,NEUTRAL)
		&& (gain>0)
		&& !IS_NPC(victim)){
	global_color=31;
	send_to_char("EXPERIENCE Gain canceled (IN NEUTRAL AREA)\n\r",ch);
	global_color=0;
	return;
    }

    if(!IS_NPC(ch)
        && victim
        && !IS_SET(world[ch->in_room]->room_flags,CHAOTIC)
        && (gain>0)
        && !IS_NPC(victim)){
    global_color=31;
    send_to_char("Experience gain canceled (LAWFUL AREA)n\r",ch);
 	sprintf(my_buf, "##[%s] killed [%s] IN A LAWFUL ZONE!", GET_NAME(ch),
GET_NAME(victim));
	log_hd(my_buf);	
    global_color=0;
    return;
    }
 
   if(victim && !IS_SET(ch->p->iFlags, FLG_APPROVED) &&
	(GET_LEVEL(ch) > 5) && (gain > 0)) {
	global_color=36;
	send_to_char("
\rSince your name has not yet been approved by a god, the game will no longer 
\rlet you gain experience until your name is approved approved by a level 
\r34+ god. Please use the 'who' command to find out what gods are online so 
\ryou can get your name approved.\r\n",ch);   
	return;
    }

    if(!IS_SET(ch->p->iFlags, FLG_APPROVED) && (gain > 0) )
	gain = 0;

    if ( !IS_NPC(ch) )
    {
	if ( GET_LEVEL(ch) >= 31 )
	    return;

	switch ( GET_CLASS(ch) )
	{
	case CLASS_THIEF:      gain += gain / 8; break;
	case CLASS_MAGIC_USER: gain += gain / 4; break;
	case CLASS_CLERIC:     gain += gain / 2; break;
	}
    }
    if(gain<0){
	sprintf(log_buf,"YOU LOST %d EXPERIENCE!\n\r",gain);
	global_color=31;
	send_to_char(log_buf,ch);
	global_color=0;
    }

    GET_EXP(ch) += gain;
    if ( GET_EXP(ch) < 0 )
	GET_EXP(ch) = 0;

    if ( IS_NPC(ch) )
	return;

    while ( GET_EXP(ch) >= exp_table[GET_LEVEL(ch)+1] )
    {
        if(ch->specials.clan){
	    sprintf(log_buf,"[CLAN] %s has LEVELED!\n\r",GET_NAME(ch));
	    tell_clan(ch->specials.clan,log_buf);
        }
	send_to_char( "\n\r***You raise a level!!***\n\r", ch );
	GET_LEVEL(ch) += 1;
	advance_level( ch );
	sort_descriptors();

	set_title( ch );
    }

    return;
}


void gain_exp_regardless( CHAR_DATA *ch, int gain )
{

    if(IS_NPC(ch))
	return;

    GET_EXP(ch) += gain;
    if ( GET_EXP(ch) < 0 )
	GET_EXP(ch) = 0;

    if ( IS_NPC(ch) )
	return;

    while ( GET_EXP(ch) >= exp_table[GET_LEVEL(ch)+1] )
    {
	send_to_char( "You raise a level!!  ", ch );
	GET_LEVEL(ch) += 1;
	advance_level( ch );
	sort_descriptors();
	set_title( ch );
    }

    return;
}

void gain_condition(struct char_data *ch,int condition,int value)
{
    bool intoxicated;

    if(GET_COND(ch, condition)==-1) /* No change */
	return;

    intoxicated=(GET_COND(ch, DRUNK) > 0);

    GET_COND(ch, condition)  += value;

    GET_COND(ch,condition) = MAX(0,GET_COND(ch,condition));
    GET_COND(ch,condition) = MIN(24,GET_COND(ch,condition));

    if(GET_COND(ch,condition))
	return;

    if(IS_DEAD(ch))
	return;
    switch(condition){
	case FULL :
	{
	    send_to_char("You are hungry.\n\r",ch);
	    return;
	}
	case THIRST :
	{
	    send_to_char("You are thirsty.\n\r",ch);
	    return;
	}
	case DRUNK :
	{
	    if(intoxicated)
		send_to_char("You are now sober.\n\r",ch);
	    return;
	}
	default : break;
    }

}

/* Inflict damage on the poor people */

void damage_room_update(void)
{
struct damage_rooms *d_room;
struct char_data *i;

	for( d_room = daroom_list ; d_room ; d_room = d_room->next)
	{
		if(!world[d_room->room_num])
			continue;
			
		switch(d_room->damage_type) {
			case FIRE:
				if(!IS_SET(world[d_room->room_num]->room_flags, FIRE))
					break;
				for( i = world[d_room->room_num]->people ; i ; i = i->next_in_room ) 
				{
					if(!IS_NPC(i) && GET_LEVEL(i) < 32 && !IS_HOVERING(i)) {
						GET_HIT(i) -= number(1, d_room->damage_amt);
						global_color = 31;
						send_to_char("\r\nAn intense fire fills the room, burning the flesh on your body.\r\n",i);
						act("$n's body chars and burns as the fire licks $s skin!", TRUE, i, 0, 0, TO_ROOM);
						global_color=0;
						if(GET_HIT(i) < 0) {
							global_color=31;
							send_to_char("Your body shrivels up in the heat of the fire, You are DEAD!\r\n",i);
							act( "$n is consumed by the fire and dies!!", TRUE, i, 0, 0, TO_ROOM );
							raw_kill(i,NULL);
							continue;
						}
						continue;
					}
				}
				break;

			case COLD:
				if(!IS_SET(world[d_room->room_num]->room_flags, COLD))
					break;
				for( i = world[d_room->room_num]->people ; i ; i = i->next_in_room ) 
				{
					if(!IS_NPC(i) && GET_LEVEL(i) < 32 && !IS_HOVERING(i)) {
						GET_HIT(i) -= number(1, d_room->damage_amt);
						global_color = 37;
						send_to_char("\r\nA biting cold rips through your body, freezing your blood and muscles!\r\n",i);
						act("$n bends over in pain as the biting cold rushes through $s body.", TRUE, i, 0, 0, TO_ROOM);
						global_color=0;
						if(GET_HIT(i) < 0) {
							global_color=31;
							send_to_char("The cold finnaly gets to you, You are DEAD!\r\n",i);
							act( "$n turns into a icicle, $n is DEAD!!", TRUE, i, 0, 0, TO_ROOM );
							raw_kill(i,NULL);
							continue;
						}
						continue;
					}
				}
				break;


			case GAS:
				if(!IS_SET(world[d_room->room_num]->room_flags, GAS))
					break;
				for( i = world[d_room->room_num]->people ; i ; i = i->next_in_room ) 
				{
					if(!IS_NPC(i) && GET_LEVEL(i) < 32 && !IS_HOVERING(i)) {
						GET_HIT(i) -= number(1, d_room->damage_amt);
						global_color = 33;
						send_to_char("\r\nNoxious fumes fill the area.. You choke and gag on the poisonous vapors!\r\n",i);
						act("$n gags and chokes on the noxious fumes that fill the area.", TRUE, i, 0, 0, TO_ROOM);
						global_color=0;
						if(GET_HIT(i) < 0) {
							global_color=31;
							send_to_char("Your lungs finnaly give out from breathing the fumes, You are DEAD!\r\n",i);
							act( "$n keels over from the toxic fumes, $n is DEAD!!", TRUE, i, 0, 0, TO_ROOM );
							raw_kill(i,NULL);
							continue;
						}
						continue;
					}
				}
				break;
			
			default:
				log_hd("## INVALID type in damage_room_update!");
				break;
		}
	}
}
							
												

/* Update both PC's & NPC's and objects*/
void point_update( void )
{   
    void decay_corpses(void);
    void update_char_objects( struct char_data *); /* handler.c */
    /*struct obj_data *extract_obj(struct obj_data *);  handler.c */
    struct char_data *i=NULL, *next_dude=NULL;

  /* characters */
    for (i = character_list; i; i = next_dude) {
	next_dude = i->next;

	if(i->in_room == -1) continue;
	if(IS_NPC(i)&&i->specials.stpMount&&IS_FLYING(i->specials.stpMount))
		continue;
	/* Don't tick if your social */
	if(i->desc)
	  if(i->desc->connected == CON_SOCIAL_ZONE) {
		check_idling(i);
		continue;	
	  }

	if (GET_POS(i) > POSITION_STUNNED) {
	    GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
	    GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
	    GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
	} else if (GET_POS(i) == POSITION_STUNNED) {
	    GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
	    GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
	    GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
	    update_pos( i );
	} else if   (GET_POS(i) == POSITION_INCAP)
		{
	    if(damage(i, i, 1, TYPE_SUFFERING))
			return;
		}
	else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW))
		{
	    if(damage(i, i, 2, TYPE_SUFFERING))
			return;
		}
	if ( (i->nr == 9800) && (i->specials.death_timer) )
		{
		if (guy_deleted)
			{
			guy_deleted = FALSE;
			continue;
			}
		else
			check_idling(i);
		}

	if (!IS_NPC(i))
		{
	    update_char_objects(i);
	    if (GET_LEVEL(i) < 35)
			{
			if ( i->desc && i->desc->tick_wait > 0 )
				i->desc->tick_wait--;
			check_idling(i);
			if (guy_deleted)
				{
				guy_deleted = FALSE;
				continue;
				}
			}
		}

	if(!IS_NPC(i)&&i->in_room>0
	&&(GET_LEVEL(i)<32)
	&&world[i->in_room]->sector_type==SECT_UNDERWATER
	&& !IS_HOVERING(i)
	&&!IS_AFFECTED(i,AFF_BREATH_WATER)){
	    send_to_char("You choke as water fills your lungs.\n\r",i);
	    act("$n chokes and struggles as $s lungs fill with water",TRUE,i,0,0,TO_ROOM);
	    GET_MOVE(i)-=GET_LEVEL(i)*8;
	    if(GET_MOVE(i)<0)GET_MOVE(i)=0;
	    GET_HIT(i)-=GET_LEVEL(i)*3;
		if(GET_HIT(i) < 0)
			{
			send_to_char( "You have drowned!!\n\r\n\r", i);
			act( "$n is DEAD!!", TRUE, i, 0, 0, TO_ROOM );
			raw_kill(i,NULL);
			continue;
			}
	}
	   			
/* juice
 * necromancer mobs stay in game for 30 minutes or so and then recycle
 */
	if( (i->nr == 9801) && i->specials.death_timer )
		i->specials.death_timer ++;
	
	gain_condition(i,FULL,-1);
	gain_condition(i,DRUNK,-1);
	gain_condition(i,THIRST,-1);
    } /* for */

/* objects */
/* go through the object list and process all rotting corpses.
 * ones that have rotted long enough get extracted along with their junk.
 */
	decay_corpses();
}

/* Scan forward through object list looking for corpses to decay.
 * When you find one, push the cur pointer back toward head of list
 * till you find something that's not in the corpse  and won't get 
 * extracted. If you back up all the way to the head of the list then
 * start over, searching forward from the corpse.  If you then reach
 * the end of the list, no big deal. It'll just stop the while(obj) loop
 */
void decay_corpses(void)
{
struct obj_data *obj;
struct obj_data *corpse;
extern bool in_container(struct obj_data *obj, struct obj_data *container);

obj=object_list;
while(obj)
	{
	/* new code for 'use per day' magic items */
	/* i dont know where else to put this :) */
	if ((GET_ITEM_TYPE(obj) == ITEM_REGEN) && (time_info.hours == 0)) {
		obj->obj_flags.value[2] = obj->obj_flags.value[1];
	}

	/* If this is a corpse */
	if ((GET_ITEM_TYPE(obj) == ITEM_CONTAINER) && (obj->obj_flags.value[3]))
		{
		/* timer count down */
		if (obj->obj_flags.timer > 0) 
			obj->obj_flags.timer--;

		if (!obj->obj_flags.timer) 
			{
			corpse = obj;
			if (corpse->carried_by)
				act("$p decays in your hands.",
			    	FALSE, corpse->carried_by, corpse, 0, TO_CHAR);
			else if ((corpse->in_room != NOWHERE) 
				&& (world[corpse->in_room]->people))

				{
			   	act("A quivering horde of maggots consumes $p.", TRUE, 
					world[corpse->in_room]->people, corpse, 0, TO_ROOM);
			    act("A quivering horde of maggots consumes $p.", TRUE, 
					world[corpse->in_room]->people, corpse, 0, TO_CHAR);
				}
			/* back up in the object list till you find something thats
               not in the container you're extracting
             */
			while(obj && ((obj == corpse)||in_container(obj, corpse)) )
				obj=obj->prev;
			/* if you've backed up all the way to the head without
			 * finding anything then then start searching forward
             */
			if(!obj)
				{
				obj=corpse->next;
				while(obj && in_container(obj, corpse))
					obj=obj->next;
				}
			extract_obj(corpse);
			}
		else
			if(obj) obj = obj->next;
		}
	else
		if(obj) obj = obj->next;
	}
}
