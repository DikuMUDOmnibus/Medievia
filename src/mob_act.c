/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992 - 1995 INTENSE Software(tm) and Mike Krause	   *
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
#include "utils.h"
#include "db.h"
#include "handler.h"

bool stuck_in_water_reaper(struct char_data *ch);

extern struct EARTHQUAKE *stpEarthquake;
extern int port;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern bool is_formed(struct char_data *ch);
extern struct str_app_type str_app[];
extern bool put_in_formation(struct char_data *leader, struct char_data *follower);
extern void remove_from_formation(struct char_data *ch);
extern void mage_noncombat(struct char_data *ch);
extern void warrior_noncombat(struct char_data *ch);
extern void thief_noncombat(struct char_data *ch);
extern void cleric_noncombat(struct char_data *ch);
extern bool is_trapped(struct char_data *ch);
extern void try_trap(struct char_data *ch);
extern bool stuck_in_water_reaper(struct char_data *ch);
extern struct descriptor_data *descriptor_list;
extern char *track_messages[5][2][2];
extern char track_colors[5][2];
extern char global_color;

void shout_hunting_message(struct char_data *ch)
{
struct descriptor_data *i;
struct char_data *hunted=NULL;
void do_shout(struct char_data *ch, char *argument, int cmd);
int x=0, y=0;

	if(!ch->specials.hunting)
		return;

	for( i = descriptor_list ; i ; i = i->next )
		if(i->character == ch->specials.hunting) {
			hunted = ch->specials.hunting->master;
			break;
		}
	
	if(hunted) {
		for(y=0;y<3;y++)
		  for(x=0;x<3;x++) {
			if(hunted->formation[y][x])
				if(world[hunted->formation[y][x]->in_room]->zone == world[ch->in_room]->zone) 
			   		send_to_char("You get the strange sensation that you're being followed.\r\n",hunted->formation[y][x]);
		  }
	}
}
	
		
int choose_scent(struct char_data *ch, struct char_data *hunted)
{ 
int door, last=-1;
struct room_affect *rap;
struct char_data *i;
int bestdoor=-1, besttime=0; 

	for(i = world[ch->in_room]->people ; i ; i = i->next_in_room )
		if(i == hunted) 
			return(7);
					
	
	for (door = 0; door <= 5; door++)
		if(EXIT(ch, door)) {
			if (EXIT(ch, door)->to_room != NOWHERE && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) ) {
				/* Check for scent trace */
				if(EXIT(ch, door)->to_room == ch->specials.was_in_room_pk)
					last = door; 
				for(rap = world[EXIT(ch, door)->to_room]->room_afs ; rap ; rap = rap->next)
			 	
					if( (rap->type == RA_SCENT) && (rap->ch == hunted) 
					&& ( (rap->value > besttime) || (rap->value == besttime && door != last) ) ) {
						besttime = rap->value;
						bestdoor = door;
					}
		
			}
		}
  

	if( bestdoor == -1 ) 
		return(6);
	else return(bestdoor);
}
	

void auto_group_mob(struct char_data *ch)
{
struct char_data *m;
struct char_data *next_m=NULL;
    if(!IS_MOB(ch))return;
    if(ch->nr==9800)return;
    if(ch->nr==9801)return;
    if(ch->in_room<1)return;
    if(ch->specials.fighting)return;
    if(is_formed(ch))return;
    if(is_trapped(ch))return;
    if(ch->specials.hunting)return;
	if(ch->specials.stpMount)return;
	if(IS_SET(ch->player.siMoreFlags,DRAGON))return;
    if(IS_SET(ch->specials.act,ACT_SPEC))return;
    for(m=world[ch->in_room]->people;m;m=next_m)
    {
	next_m = m->next_in_room;
	if(!IS_MOB(m))continue;
	if(m==ch)continue;
	if(m->specials.fighting)continue;
	if(m->master!=m)continue;
	if(GET_CLASS(m)==CLASS_OTHER&&m->nr!=ch->nr)continue;
	if(GET_CLASS(ch)==CLASS_OTHER&&m->nr!=ch->nr)continue;
  	if(!AWAKE(m))continue;
	if(GET_CLASS(ch)!=CLASS_OTHER){
	    if(GET_ALIGNMENT(ch)>(GET_ALIGNMENT(m)+250))continue;
	    if(GET_ALIGNMENT(ch)<(GET_ALIGNMENT(m)-250))continue;
	}
        if(m->nr==9800)continue;
        if(m->nr==9801)continue;
  	if(IS_SET(m->specials.act, ACT_SPEC))continue;
	if(IS_SET(m->specials.act,ACT_SENTINEL)&&!IS_SET(ch->specials.act,ACT_SENTINEL))
	    continue;
	if(!IS_SET(m->specials.act,ACT_SENTINEL)&&IS_SET(ch->specials.act,ACT_SENTINEL))
	    continue;
	if(m->specials.hunting) continue;
	if(put_in_formation(m,ch)){
            act("*You start following $n*",TRUE,m,0,ch,TO_VICT);
            act("*$N starts following you*",TRUE,m,0,ch,TO_CHAR);
            act("<* $N starts following $n *>.",TRUE,m,0,ch,TO_NOTVICT);
	    return;
	}
    }
}

void mob_non_combat_inteligence(void)
{
struct char_data *m=NULL;
struct char_data *next_m=NULL;

    for(m=character_list;m;m=next_m){
	next_m=m->next;
	if(!IS_MOB(m))continue;
	if(!IS_MOB(m->master))continue;
	if(m->specials.fighting)continue;
	if(m->in_room<1)continue;
	if (m->nr==9800) continue;
	if(is_trapped(m)){
	    act("$n struggles to get out of the trap.",TRUE,m,0,0,TO_ROOM);
	    continue;
	}
	if(GET_POS(m)!=POSITION_STANDING)continue;
	/* spec mobs should shout stuff too */
/*
	if(m->specials.hunting)
		shout_hunting_message(m);
*/
	if(IS_SET(m->specials.act, ACT_SPEC))continue;
	
	switch(GET_CLASS(m)){
	    case CLASS_MAGIC_USER:
		if(number(1,100)<75)continue;
		mage_noncombat(m);
		continue;
	    case CLASS_CLERIC:
		if(number(1,100)<75)continue;
		cleric_noncombat(m);
		continue;
	    case CLASS_WARRIOR:
		if(number(1,100)<85)continue;
		warrior_noncombat(m);
		continue;
	    case CLASS_THIEF:
		if(number(1,100)<85)continue;
		thief_noncombat(m);
		continue;
	    default:
		continue;
	}
    }
}

void auto_group_all_mobs(void)
{
struct char_data *m=NULL;

    for(m=character_list;m;m=m->next){
	if(!IS_MOB(m))continue;
	if(is_formed(m)){
	    if(number(0,100)<20)
		remove_from_formation(m);
	    continue;
	}
	if(number(0,100)<50){
	    auto_group_mob(m);
	    continue;
	}
    }
}

void mobile_activity(void)
{
register struct char_data *ch=NULL;
struct descriptor_data *i=NULL;
#ifndef PACIFIST
struct char_data *tmp_ch=NULL;
struct char_data *next_ch=NULL;
#endif
struct obj_data *obj=NULL, *best_obj=NULL, *my_obj=NULL;
int door, max,room;
struct room_affect *rap=NULL;
char stop;
extern void do_say(struct char_data *ch, char *argument, int cmd);
extern void do_emote(struct char_data *ch, char *argument, int cmd);


#ifdef PACIFIST
int peace;
#endif

    void do_move(struct char_data *ch, char *argument, int cmd);
    void do_get(struct char_data *ch, char *argument, int cmd);
    if(stpEarthquake)return;
    /* Examine all mobs. */
    for ( ch = character_list; ch; ch = next_ch)
    {
	next_ch=ch->next;
	if ( !IS_MOB(ch) )
	    continue;
	if(ch->in_room<0)
		continue;
	if(ch->specials.stpMount)
		continue;
	if(IS_SET(ch->player.siMoreFlags,DRAGON))
		continue;
	if(ch->equipment[HOLD]
    	&& !ch->equipment[WIELD]
    	&& (CAN_WEAR(ch->equipment[HOLD],ITEM_WIELD)
       		 || IS_SET(ch->equipment[HOLD]->obj_flags.wear_flags,ITEM_THROW))
		&& !number(0,10)
    	)
    	{
		my_obj = unequip_char(ch, HOLD);
		equip_char(ch, my_obj, WIELD);
		act("$n wields $p.",TRUE, ch, ch->equipment[WIELD],0,TO_ROOM);
	    }

	if(is_trapped(ch))continue;

	/* Examine call for special procedure */
	if ( IS_SET(ch->specials.act, ACT_SPEC) )
	{
	    if (!mob_index[ch->nr].func)
	    {
		sprintf( log_buf, "Mobile_activity: MOB[%d]%s no func",
			ch->nr,GET_NAME(ch));
		if(port!=1220)
		   log_hd( log_buf );
		REMOVE_BIT( ch->specials.act, ACT_SPEC );
	    }
	    else
	    {
		if ( (*mob_index[ch->nr].func) (ch, 0, "") )
		    continue;
	    }
	}
	/* That's all for busy monster */
	if ( !AWAKE(ch) || ch->specials.fighting )
	    continue;


	/* Scavenge */
	if ( IS_SET(ch->specials.act, ACT_SCAVENGER)
	&& world[ch->in_room]->contents && number(0,10) == 0 )
	{
	    max         = 1;
	    best_obj    = 0;
	    for ( obj = world[ch->in_room]->contents; obj;
		obj = obj->next_content )
	    {
		if ( CAN_GET_OBJ(ch, obj) && obj->obj_flags.cost > max )
		{
		    best_obj    = obj;
		    max         = obj->obj_flags.cost;
		}
	    }

	    if ( best_obj )
	    {
		obj_from_room( best_obj );
		obj_to_char( best_obj, ch );
		act( "$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM );
	    }
	}
	/* Wander */
	if( ch->specials.hunting) {
		door = choose_scent(ch, ch->specials.hunting);
		if(number(1,18)==7 && door != 6)
			shout_hunting_message(ch);
		if(door == 6) 
			ch->specials.hunting = NULL;
	} else	/* run the normal door=number(0,37) routine */		
		door=number(0,37);
    if((GET_ZONE(ch)!=198)||(GET_ZONE(ch)==198&&door<4))/*no up down in catacombs*/
	if(door<=5&&world[ch->in_room]->dir_option[door]){
	room=EXIT(ch,door)->to_room;
	if(room)
	if(ch->nr != 9801)
	if(ch->nr != 9800)
	if(ch->master==ch)/*must be leader to move */
	if ( !IS_SET(ch->specials.act, ACT_SENTINEL))
	if(GET_POS(ch) == POSITION_STANDING)
	if(CAN_GO(ch, door))
	if(!IS_SET(world[room]->room_flags, NO_MOB|DEATH) )
	{
	    stop=0;
	    for(rap=world[room]->room_afs;rap;rap=rap->next){
		if(rap->type==RA_SHIELD){
		    stop=1;
		    send_to_room("The room's shield HUMS and brightens as something runs into it close by.\n\r",rap->room);
		}
	    }
	    if (ch->specials.last_direction == door)
	    {
		ch->specials.last_direction = -1;
	    }else if ((!IS_SET(ch->specials.act, ACT_STAY_ZONE)
	    || world[room]->zone == world[ch->in_room]->zone)&&(!stop))
	    {
		ch->specials.last_direction = door;
		do_move( ch, "", ++door );
		try_trap(ch);
		auto_group_mob(ch);

	    }
	}
	}
	/* Aggress */
	/* juice -- don't be agressive if someone switched with mob */
	if ( (IS_SET(ch->specials.act, ACT_AGGRESSIVE) && (!ch->desc)) ||
		 ( (ch->specials.hunting) && (!ch->desc) ) )
	{

#ifdef PACIFIST
		peace = number(1,100);

		switch (peace)
			{
			case 1:
				do_emote(ch, "smiles at you and adorns your weapon with a wreath of daisies.",0);
				break;
			case 2:
				do_emote(ch, "smiles and shakes your hand.",0);
				do_say(ch,"Peace, friend",0);
				break;
			default:
				break;
			}
#else
	    for ( tmp_ch = world[ch->in_room]->people; tmp_ch;
	    tmp_ch = tmp_ch->next_in_room )
	    {
		if (GET_LEVEL(tmp_ch) > 31 && tmp_ch->specials.holyLite)
		    continue;
		if ( IS_NPC(tmp_ch) || !CAN_SEE(ch, tmp_ch) )
		    continue;
		if ( IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch) )
		    continue;

	    /* i'll give you a buck if you tell me what that sneak equation means */
	    if(IS_AFFECTED(tmp_ch, AFF_SNEAK) && tmp_ch->p->sneaked_room == 0 &&
	       tmp_ch != ch->specials.hunting) {
	    	if( !(GET_LEVEL(ch) > (GET_LEVEL(tmp_ch)+5)) )
	    		continue;
	    	else if(number(1,100)>20)
	    		continue;
		}
	    if(stuck_in_water_reaper(tmp_ch))
	        continue;
	    /* Make sure the hunted person is attacked first */
	 	if(IS_SET(ch->specials.act, ACT_AGGRESSIVE) && 
	 	   tmp_ch != ch->specials.hunting)	 
		if ( ( IS_SET(ch->specials.act, ACT_AGGR_EVIL)
		    && IS_EVIL(tmp_ch) ) 
		||   ( IS_SET(ch->specials.act, ACT_AGGR_NEUT)
		    && IS_NEUTRAL(tmp_ch) ) 
		||   ( IS_SET(ch->specials.act, ACT_AGGR_GOOD)
		    && IS_GOOD(tmp_ch) )
		||   ( ch->specials.act & ACT_AGGR_ALL ) == 0 )
		{
		  hit(ch, tmp_ch, 0);
		  break;
		}
		
	    if ( ch->specials.hunting )
	    	if( (ch->specials.hunting == tmp_ch) && !IS_HOVERING(tmp_ch) ) {
				 global_color = track_colors[(int) GET_CLASS(ch)][0];
				 act( track_messages[(int) GET_CLASS(ch)][0][0],FALSE, ch,0,tmp_ch,TO_VICT);
	   			 act( track_messages[(int) GET_CLASS(ch)][0][1], FALSE, ch, 0, tmp_ch, TO_NOTVICT);
				 global_color=0;
				 hit(ch,tmp_ch,0);
	   			 break;
	   		}

	 }
#endif
	}

    }
	/* update sneaked people */
	for( i = descriptor_list ; i ; i = i->next )
	{
		if( i->connected == CON_PLAYING && i->character->in_room != NOWHERE)
			 i->character->p->sneaked_room = i->character->in_room;
	}

}
