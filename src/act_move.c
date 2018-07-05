/***************************************************************************
*         		 MEDIEVIA CyberSpace Code and Data files		           *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*			              All rights reserved				               *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/


#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "holocode.h"
#include "spells.h"

/*   external vars  */
extern char global_color;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern int pulse;
extern struct descriptor_data *descriptor_list; 
extern struct index_data *obj_index;
extern int rev_dir[];
extern char *dirs[]; 
extern int movement_loss[];
extern struct EARTHQUAKE *stpEarthquake;
extern int port;
extern ush_int Holo[MAXHOLO][MAXHOLO];
extern int iMakeHoloRoom(int x,int y);
extern struct zone_data *zone_table;
extern char MOUNTMOVE;

/* external functs */
extern int iTradeShop(struct char_data *stpCh, int iCmd, char *szpArg);
extern ListRoomFreightToChar(struct char_data *ch);
extern void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode, bool show);
extern void list_char_to_char(struct char_data *list, struct char_data *ch, int mode);
extern void make_scent(struct char_data *ch, int room);
extern void dream_weaver(struct char_data *ch);
extern bool is_formed(struct char_data *ch);
extern void undead_gross_out(struct char_data *ch, struct char_data *k);
void death_cry(struct char_data *ch);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
    struct obj_data *list);
extern void ShowTrackMessage(struct char_data *stpCh);

#define SOCIAL_BEGIN_ROOM 1652

void do_socialwarp(struct char_data *ch, char *argument, int cmd)
{
int room=0;

	if( IS_NPC(ch) || !(ch->desc) ) return;
	if(GET_LEVEL(ch)>32){
		send_to_char("For many reasons gods may not use this command.  It is impossible to keep track of where you are and where you were as a god.  \n\rThe MEDLINK zone begins at room 1652 just go there.\n\r",ch);
		return;
	}		
	if(ch->desc->connected == CON_SOCIAL_ZONE) {
		if(ch->p->iLastSocialX || ch->p->iLastSocialY) {
			if(Holo[ch->p->iLastSocialX][ch->p->iLastSocialY]>255)
					room = Holo[ch->p->iLastSocialX][ch->p->iLastSocialY];
			else
					room = iMakeHoloRoom(ch->p->iLastSocialX,ch->p->iLastSocialY);
			if(!world[room]) {
				sprintf(log_buf,"## No where to send home %s from SocialZone\r\n",GET_NAME(ch));
				log_hd(log_buf);
				send_to_char("Error, Please tell vryce what you did.\r\n",ch);
				return;
			}
			act("$n flashes out of MedLink.",TRUE,ch,0,0,TO_ROOM);
			MOUNTMOVE=TRUE;
			char_from_room(ch);
			char_to_room(ch, room);
			MOUNTMOVE=FALSE;
			ch->desc->connected = CON_PLAYING;
			ch->p->iLastSocialX = 0;
			ch->p->iLastSocialY = 0;
			ch->p->iLastInBeforeSocial = 0;
			send_to_char("Poof, You are back to reality.\r\n",ch);
			act("$n flashes back into the realms from MedLink.",TRUE,ch,0,0,TO_ROOM);
			do_look(ch,"",15);
			return;
		} else {
			if(!world[ch->p->iLastInBeforeSocial]) {
				log_hd("## Social Zone Error");
				send_to_char("ERROR: Please tell vryce what you did.\r\n",ch);
				return;
			}
		    room = ch->p->iLastInBeforeSocial;
			act("$n flashes out of MedLink.",TRUE,ch,0,0,TO_ROOM);
			MOUNTMOVE=TRUE;
	        char_from_room(ch);
			char_to_room(ch, room);
			MOUNTMOVE=FALSE;
			ch->desc->connected = CON_PLAYING;
			ch->p->iLastSocialX = 0;
			ch->p->iLastSocialY = 0;
			ch->p->iLastInBeforeSocial = 0;
			act("$n flashes back into the realms from MedLink.",TRUE,ch,0,0,TO_ROOM);
			send_to_char("Poof, You are back to reality.\r\n",ch);
			do_look(ch,"",15);
			return;
		}				
	}	
	if(ch->desc->connected == CON_PLAYING) {
		if( (world[ch->in_room]->zone == 198 ||
		   !(zone_table[world[ch->in_room]->zone].iSocialRestricted)
		   || GET_POS(ch) == POSITION_FIGHTING) &&
		    (!IN_HOLO(ch)) ) {
			send_to_char("You can't go to MedLink now.\r\n",ch);
			return;
		}
		if(IN_HOLO(ch)) {
			ch->p->iLastInBeforeSocial = 0;
			ch->p->iLastSocialX = world[ch->in_room]->holox;
			ch->p->iLastSocialY = world[ch->in_room]->holoy;
			ch->desc->connected = CON_SOCIAL_ZONE;
			act("$n flashes out of existance.",TRUE,ch,0,0,TO_ROOM);
			MOUNTMOVE=TRUE;
			char_from_room(ch);
			char_to_room(ch,SOCIAL_BEGIN_ROOM);
			MOUNTMOVE=FALSE;
			if(ch->specials.stpMount){
				char_from_room(ch->specials.stpMount);
				char_to_room(ch->specials.stpMount,0);
			}
			act("$n flashes into MedLink.",TRUE,ch,0,0,TO_ROOM);
			send_to_char("Poof! You arrive in the MedLink.\r\n",ch);
			do_look(ch,"",15);
			SAVE_CHAR_OBJ(ch, -20);
			return;
		} else {
			ch->p->iLastInBeforeSocial = ch->in_room;
			ch->p->iLastSocialX = 0;
			ch->p->iLastSocialY = 0;
			ch->desc->connected = CON_SOCIAL_ZONE;
			act("$n flashes out of existance.",TRUE,ch,0,0,TO_ROOM);
			MOUNTMOVE=TRUE;
			char_from_room(ch);
			char_to_room(ch,SOCIAL_BEGIN_ROOM);
			MOUNTMOVE=FALSE;
			if(ch->specials.stpMount){
				char_from_room(ch->specials.stpMount);
				char_to_room(ch->specials.stpMount,0);
			}
			act("$n flashes into MedLink.",TRUE,ch,0,0,TO_ROOM);
			send_to_char("Poof! You arrive in the social area.\r\n",ch);
			do_look(ch,"",15);
			SAVE_CHAR_OBJ(ch, -20);
			return;
		}
	}
}


void verify_move(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_STRING_LENGTH];
int dir;

   one_argument(argument,buf);
   if(!buf[0])
	return;
   if(buf[0]=='y'||buf[0]=='Y'){
      dir=ch->specials.direction;
      ch->specials.direction=666; /* OK you asked for it! */
      ch->p->querycommand=0;
      do_move(ch,"",dir+1);
   }
   ch->p->querycommand=0;
}

int is_tunnel_full(struct char_data *ch, int room)
{
struct char_data *i=NULL;
int player=0, npc=0;
	
	if(GET_LEVEL(ch) >= 32 && !IS_NPC(ch) )
		return(FALSE);
		
	for( i = world[room]->people ; i ; i = i->next_in_room) 
	{
		if(IS_NPC(i) && (i != ch) )
			npc++;
		else if( !i->specials.wizInvis && (ch != i) )
			player++;
	}
	if(IS_NPC(ch) && npc >=1)
		return(TRUE);
	else if( player >= 1  && !IS_NPC(ch) )
		return(TRUE);
	else
		return(FALSE);

}		

int is_allowed_here(struct char_data *ch, int to_room)
{
struct room_affect *rap=NULL;

    if(GET_LEVEL(ch) >= 32)
		return(TRUE);
	if(IS_NPC(ch) && !IS_UNDEAD(ch))
		{
		if( IS_SET(world[to_room]->room_flags, NO_MOB|DEATH) )
			return(FALSE);
		for(rap=world[to_room]->room_afs;rap;rap=rap->next)
			if(rap->type==RA_SHIELD)
				return(FALSE);
		}

    if(world[to_room]->level_restriction){
	if(world[to_room]->level_restriction>1&&GET_LEVEL(ch)<world[to_room]->level_restriction){
	    send_to_char("Your low level keeps you from entering.\n\r",ch);
	    return(FALSE);
	}
   if(world[to_room]->level_restriction<0&&(GET_LEVEL(ch)>(world[to_room]->level_restriction*-1))){
	    send_to_char("Your high level keeps you from entering.\n\r",ch);
	    return(FALSE);
	}
    }    
    switch(GET_CLASS(ch)){
	case CLASS_MAGIC_USER:
	    if(IS_SET(world[to_room]->class_restriction,NO_MAGE)){
		global_color=31;
		send_to_char("A strange force blocks your way.\n\r",ch);
		act("A strange force blocks $n's way.\n\r",TRUE,ch,0,0,TO_ROOM);
		global_color=0;
		return(FALSE);
	    }
	    break;
	case CLASS_CLERIC:
	    if(IS_SET(world[to_room]->class_restriction,NO_CLERIC)){
		global_color=31;
		send_to_char("A strange force blocks your way.\n\r",ch);
		act("A strange force blocks $n's way.\n\r",TRUE,ch,0,0,TO_ROOM);
		global_color=0;
		return(FALSE);
	    }
	    break;
	case CLASS_THIEF:
	    if(IS_SET(world[to_room]->class_restriction,NO_THIEF)){
		global_color=31;
		send_to_char("A strange force blocks your way.\n\r",ch);
		act("A strange force blocks $n's way.\n\r",TRUE,ch,0,0,TO_ROOM);
		global_color=0;
		return(FALSE);
	    }
	    break;
	case CLASS_WARRIOR:
	    if(IS_SET(world[to_room]->class_restriction,NO_WARRIOR)){
		global_color=31;
		send_to_char("A strange force blocks your way.\n\r",ch);
		act("A strange force blocks $n's way.\n\r",TRUE,ch,0,0,TO_ROOM);
		global_color=0;
		return(FALSE);
	    }
	    break;
    }
    if(IS_GOOD(ch)&&IS_SET(world[to_room]->align_restriction,NO_GOOD)){
	global_color=31;
	send_to_char("You are overcome by a barrage of ecstasy and stop in your tracks.\n\r",ch);
	global_color=0;
	return(FALSE);
    }
    if(IS_NEUTRAL(ch)&&IS_SET(world[to_room]->align_restriction,NO_NEUTRAL)){
	global_color=31;
	send_to_char("You feel faint and decide not to go that way.\n\r",ch);
	global_color=0;
	return(FALSE);
    }
    if(IS_EVIL(ch)&&IS_SET(world[to_room]->align_restriction,NO_EVIL)){
	global_color=31;
	send_to_char("A shiver runs through your evil bones and you decide not to.\n\r",ch);
	global_color=0;
	return(FALSE);
    }
   return(TRUE);
}

int do_simple_move(struct char_data *ch, int cmd, int following)
/* Assumes, 
    1. That there is no master and no followers.
    2. That the direction exists. 

   Returns :
   1 : If succes.
   0 : If fail
  -1 : If dead.
*/
{
    char tmp[80];
    int was_in;
    int need_movement=0,iMovement,x;
    struct obj_data *obj=NULL;
    struct char_data *k=NULL;
    struct char_data *k_next=NULL;	
    bool has_boat;
    bool has_wing;
    int to_room;
    int iWear;
    int is_tunnel_full(struct char_data *ch, int room);
	struct char_data *stpMoveCh;    

	if(IS_UNDEAD(ch)&&number(1,100)<8){
		send_to_char("You fall down.\n\r",ch);
		act("$n falls down.",TRUE,ch,0,0,TO_ROOM);
		GET_POS(ch) = POSITION_RESTING;
	}

    if (special(ch, cmd+1, ""))  /* Check for special routines (North is 1) */
		return(FALSE);
	if(ch->specials.stpMount&&!IS_NPC(ch))
		stpMoveCh=ch->specials.stpMount;
	else
		stpMoveCh=ch;
	if(ch->desc && ch->desc->connected != CON_SOCIAL_ZONE)
	if(GET_BREATH(stpMoveCh)<=0){
		global_color=32;
		send_to_char("You COLLAPSE, totally out of breath!\n\r",stpMoveCh);
		act("$n makes strange strangling sounds and COLLAPSES out of breath.",TRUE,stpMoveCh,0,0,TO_ROOM);
		if(stpMoveCh==ch){
			GET_HIT(ch)/=2;
			GET_POS(ch) = POSITION_RESTING;
		}
		global_color=0;
		GET_BREATH(stpMoveCh)=0;
		return(FALSE);
	}else if(GET_BREATH(stpMoveCh)<15){
		global_color=32;
		send_to_char("You STOP and PANT loud, sucking in air!\n\r",stpMoveCh);
		act("$n STOPS, Panting and heaving, loudly sucking in air.",TRUE,stpMoveCh,0,0,TO_ROOM);
		global_color=0;		
		return(FALSE);
	}else if(GET_BREATH(stpMoveCh)<30){
		global_color=32;
		send_to_char("You pant loudly, almost out of breath.\n\r",stpMoveCh);
		act("$n pants loudly, almost out of breath.",TRUE,stpMoveCh,0,0,TO_ROOM);
		global_color=0;		
	}
    if(world[ch->in_room]->zone==198){
		if(number(0,38)==25){
		    if(world[ch->in_room]->sector_type==SECT_INSIDE){
				if(IS_AFFECTED(ch,AFF_FLYING)){
			    	act("$n hits $s head on a stalactite.",TRUE,ch,0,0,TO_ROOM);
				    send_to_char("You hit your head on a stalactite.\n\r",ch);
			    	return(FALSE);
				}else{
				    act("$n trips over a small stalagmite and stumbles.",TRUE,ch,0,0,TO_ROOM);
				    send_to_char("You trip over a small stalagmite and stumble.\n\r",ch);
				    return(FALSE);
				}
		    }
		}
    }
    need_movement = (movement_loss[world[ch->in_room]->sector_type]
     +world[ch->in_room]->move_mod
     +world[EXIT(ch, cmd)->to_room]->move_mod
     +movement_loss[world[EXIT(ch, cmd)->to_room]->sector_type]);

    if(!is_allowed_here(ch, EXIT(ch, cmd)->to_room))
		return(FALSE);
   
   	/* Tunnel Full Checking  - Raster */
   	if(IS_SET(world[EXIT(ch, cmd)->to_room]->room_flags, TUNNEL))
   		if(is_tunnel_full(ch, EXIT(ch, cmd)->to_room)) {
   			send_to_char("You try to enter the room, but it is too cramped to proceed.\r\n",ch);
   			return(FALSE);
 		}
 		   
    if ((world[ch->in_room]->sector_type == SECT_WATER_NOSWIM) ||
     (world[EXIT(ch, cmd)->to_room]->sector_type == SECT_WATER_NOSWIM)) {
		has_boat = FALSE;

		/* See if char is carrying a boat */
		for (obj=ch->carrying; obj; obj=obj->next_content)
	    	if (obj->obj_flags.type_flag == ITEM_BOAT)
			has_boat = TRUE;

		if(IS_AFFECTED(ch,AFF_FLYING) || GET_LEVEL(ch) >34) 
			has_boat=TRUE;
	    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    		if ( ch->equipment[iWear] )
        	  	if(ch->equipment[iWear]->obj_flags.type_flag==ITEM_BOAT ||
            	 ch->equipment[iWear]->obj_flags.type_flag==ITEM_FLY)
	          		has_boat = TRUE;
		if (!has_boat) {
		    send_to_char("You need a boat to go there.\n\r", ch);
	    	return(FALSE);
		}
    }
    if ((world[ch->in_room]->sector_type == SECT_AIR) || 
     (world[EXIT(ch, cmd)->to_room]->sector_type == SECT_AIR)) {
		has_wing = FALSE;

/* See if char is flying 
		for (obj=ch->carrying; obj; obj=obj->next_content)
	    	if (obj->obj_flags.type_flag == ITEM_FLY)
				has_wing = TRUE;

		if(IS_AFFECTED(ch,AFF_FLYING))
			has_wing=TRUE;
*/
        for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    	    if ( ch->equipment[iWear] )
                if(ch->equipment[iWear]->obj_flags.type_flag==ITEM_FLY)
		    has_wing = TRUE;

		if (!has_wing) {
		    send_to_char("You aren't flying.\n\r", ch);
	    	return(FALSE);
		}
    }
    if(world[ch->in_room]->zone != 180){ /*Social Zone*/
		if(ch->specials.stpMount&&!IS_NPC(ch)){
			iMovement=GET_MOVE(ch->specials.stpMount);
		}else{
			iMovement=GET_MOVE(ch);
		}
    	if((iMovement<need_movement) && (IS_UNDEAD(ch) || !IS_NPC(ch))) {
			if(!following){
				if(!IS_NPC(ch)&&ch->specials.stpMount)
					send_to_char("Your mount is too exhausted.\n\r",ch);
				else
			 	   	send_to_char("You are too exhausted.\n\r",ch);
			}else{
				if(!IS_NPC(ch)&&ch->specials.stpMount)
					send_to_char("Your mount is too exhausted to follow.\n\r",ch);
				else
			    	send_to_char("You are too exhausted to follow.\n\r",ch);
			}
			return(FALSE);
		}
	}

    was_in = ch->in_room;
    to_room=world[was_in]->dir_option[cmd]->to_room;
    if(!IS_NPC(ch)){
    	if(ch->specials.direction!=666){
		    if(IS_SET(world[to_room]->room_flags,NEUTRAL)&& /*law to neut*/
			 !IS_SET(world[was_in]->room_flags,NEUTRAL)&&
		  	 !IS_SET(world[was_in]->room_flags,CHAOTIC)){
    	 	   ch->p->queryfunc=verify_move;
	    	    strcpy(ch->p->queryprompt, "ENTER Neutral PlayerKill Area? (y/n)>");
    	    	ch->p->querycommand=1111;
				ch->specials.direction=cmd;
				return(FALSE);
	    	}
		    if(IS_SET(world[to_room]->room_flags,CHAOTIC)&& /*law to cha*/
			 !IS_SET(world[was_in]->room_flags,NEUTRAL)&&
			 !IS_SET(world[was_in]->room_flags,CHAOTIC)){
		        ch->p->queryfunc=verify_move;
        		strcpy(ch->p->queryprompt, "ENTER CHAOTIC PlayerKill Area? (y/n)>");
		        ch->p->querycommand=1111;
				ch->specials.direction=cmd;
				return(FALSE);
		    }
		    if(IS_SET(world[to_room]->room_flags,CHAOTIC)&& /*neut to cha*/
			 !IS_SET(world[was_in]->room_flags,CHAOTIC)){
		        ch->p->queryfunc=verify_move;
		        strcpy(ch->p->queryprompt, "ENTER CHAOTIC PlayerKill Area? (y/n)>");
		        ch->p->querycommand=1111;
				ch->specials.direction=cmd;
				return(FALSE);
			}
			if(IS_SET(world[to_room]->room_flags,NEUTRAL)	/*cha to neut*/
			 &&!IS_SET(world[was_in]->room_flags,NEUTRAL)){
				ch->p->queryfunc=verify_move;
				strcpy(ch->p->queryprompt, "ENTER NEUTRAL PlayerKill Area? (y/n)>");
				ch->p->querycommand=111;
				ch->specials.direction=cmd;
				return(FALSE);
		    }
	    }
    }
    if(ch->specials.direction)/*used for verification of pk move */
		ch->specials.direction=0;    

    if( (GET_LEVEL(ch)<32) &&  (IS_UNDEAD(ch) || !IS_NPC(ch)) )
     	if(ch->desc)
			if(ch->desc->connected != CON_SOCIAL_ZONE){
				GET_MOVE(stpMoveCh) -= need_movement;
			}
    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
		global_color=1;
 	if(!world[ch->in_room]->dir_option[cmd]->exit||(world[ch->in_room]->dir_option[cmd]->exit&&!strstr(world[ch->in_room]->dir_option[cmd]->exit,"$n"))){
	    if(IS_AFFECTED(ch,AFF_FLYING)){
	        sprintf(tmp, "$n flies %s.", dirs[cmd]);
	    }else{
			if(!ch->specials.stpMount){
		    	sprintf(tmp, "$n leaves %s.", dirs[cmd]);
			}else{
		    	sprintf(tmp, "$n riding %s leaves %s.",ch->specials.stpMount->player.short_descr, dirs[cmd]);
		    }
		    act(tmp, TRUE, ch, 0,0,TO_ROOM);
		}
	}else{
	    act(world[ch->in_room]->dir_option[cmd]->exit, TRUE, ch, 0,0,TO_ROOM);
	}
		global_color=0;
    }	
	/* So breath stuff dosn't run in social */
	if(ch->desc && ch->desc->connected != CON_SOCIAL_ZONE)
		if(GET_LEVEL(ch)<32&&!IS_UNDEAD(ch)){
			x=pulse-ch->specials.iLastMovePulse;
			if(x<0)x=0;
			x=6-x;
			if(x<0)x=0;	
			GET_BREATH(stpMoveCh)-=10*x;
			if(GET_BREATH(stpMoveCh)<0)
				GET_BREATH(stpMoveCh)=0;
		}
	ch->specials.iLastMovePulse=pulse;
    char_from_room(ch);
    char_to_room(ch, world[was_in]->dir_option[cmd]->to_room);
    global_color=1;
    if (!IS_AFFECTED(ch, AFF_SNEAK)){
		if(!world[was_in]->dir_option[cmd]->entrance||(world[was_in]->dir_option[cmd]->entrance&&!strstr(world[was_in]->dir_option[cmd]->entrance,"$n"))){
			if(!ch->specials.stpMount){
			   act("$n has arrived.", TRUE, ch, 0,0, TO_ROOM); 
			}else{
		    	sprintf(tmp, "$n riding %s arrived.",ch->specials.stpMount->player.short_descr);
				act(tmp,TRUE,ch,0,0,TO_ROOM);			
			}
		}else{
		   act(world[was_in]->dir_option[cmd]->entrance, TRUE, ch, 0,0, TO_ROOM); 
		}
    }
    global_color=0;
	if(ch->specials.was_in_room_pk!=ch->in_room &&
	 (world[ch->in_room]->holox || world[ch->in_room]->holoy) &&
	 world[ch->in_room]->funct!=iTradeShop &&
	 !IS_SET(ORIGINAL(ch)->specials.act, PLR_BRIEF) ) {
		do_areamap(ch,"\0",15);
		if(ch->specials.hunting)
			ShowTrackMessage(ch);
		ListRoomFreightToChar(ch);
		list_obj_to_char(world[ch->in_room]->contents, ch, 0,FALSE);
		list_char_to_char(world[ch->in_room]->people, ch, 0);		
	} else if(ch->specials.was_in_room_pk!=ch->in_room) {
		/* Run as Cmd 16 so track messages will run only when
		   the player walks into the room
		*/
	       do_look(ch, "\0",16);
             if( ch->desc  && !ORIGINAL(ch)->specials.autoexit )
	     	do_exits(ch,"",9);
	}

	if(IS_SET(world[ch->in_room]->room_flags, FIRE)) 
	{
		if(GET_LEVEL(ch) < 32 && !IS_NPC(ch) ) {
			GET_HIT(ch) -= dice(5,7);
			global_color=31;
			send_to_char("Your body starts BURNING as you enter the FIRE filled room.\r\n",ch);
			act("$n's body starts BURNING brightly as $e enters the room.", FALSE, ch, 0, 0, TO_ROOM);
			global_color=0;
			if(GET_HIT(ch) < 0) {
					global_color=31;
					send_to_char("Your body BURNS brightly and turns to ASH. You are dead!\r\n",ch);
					act("$n's body turns to ash.", FALSE, ch, 0, 0, TO_ROOM);
					global_color=0;
					raw_kill(ch,NULL);
					return(1);
			}
		}	
	}
	
		
    if(IS_UNDEAD(ch))
    	for (k = world[ch->in_room]->people; k; k = k_next){
			k_next=k->next_in_room;
			undead_gross_out(ch,k);
		}

	if(!IS_NPC(ch) && !IS_UNDEAD(ch) &&
	 (world[ch->in_room]->sector_type <= 5 || world[ch->in_room]->sector_type == 8) )
		make_scent(ch, ch->in_room);
		
    if(!IS_NPC(ch) || IS_UNDEAD(ch))
    	for (k = world[ch->in_room]->people; k; k = k_next){
			k_next=k->next_in_room;
	    if ( IS_MOB(k) ){
			if(!strcmp(&k->specials.last_attack[0],GET_NAME(ch))){
		    	if(IS_AFFECTED(ch, AFF_SNEAK)){
					send_to_char("You sneak into the room\n\r",ch);
					return(1);
	     	    }
		     	if((GET_LEVEL(k)<GET_LEVEL(ch)) 
			     && (!IS_SET(k->specials.act, ACT_AGGRESSIVE))){
				    do_emote(k,"screams from fright!",0);
				    do_flee(k,"",0);
				    return(1);
		     	} else {
				    if(!IS_UNDEAD(k))
				    if((number(0,100)-GET_LEVEL(ch))<50) { 
						sprintf(log_buf,"AH HA! I have been looking for you %s!",GET_NAME(ch));
						do_say(k,log_buf,0);
						hit(k,ch,0);
						return(1);
				    }
				}
		    }
		}
    }
    return(1);
}

void do_move(struct char_data *ch, char *argument, int cmd)
{
    char tmp[80];
    int was_in,x,y;
 
    --cmd;
 
if((!ch->player.title)&&(!IS_NPC(ch))){
    log_hd("########no title in do_move(MY CHECK)######");
    log_hd(ch->player.name);
    set_title(ch);
}
if (!world[ch->in_room]->dir_option[cmd])
    {
    send_to_char("Alas, you cannot go that way...\n\r", ch);
    return;
    }

if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLOSED) &&
    (IS_SET(EXIT(ch, cmd)->exit_info, EX_SECRET) ||
     IS_SET(EXIT(ch, cmd)->exit_info, EX_HIDDEN)))
   {
   send_to_char("Alas, you cannot go that way...\r\n",ch);
   return;
   } 

if (stpEarthquake && GET_LEVEL(ch) < 32){
	global_color=31;
	GET_HIT(ch)/=3;
	send_to_char("You try and move during the QUAKE and fall.. . ..\n\r",ch);
	global_color=0;
	return;
}
if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLOSED)
	&& IS_UNDEAD(ch)
    && !IS_SET(world[ch->in_room]->dir_option[cmd]->exit_info,EX_PICKPROOF)
    )
    {
    act("You turn into a ghostly shimmer and try to pass through solid mass.",
		FALSE, ch, 0, ch, TO_CHAR);
    do_simple_move(ch, cmd, TRUE);
    return;
    }

/* Direction is possible */
if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLOSED) )
    {
    if (EXIT(ch, cmd)->keyword)
        {
        sprintf(tmp, "The %s seems to be closed.\n\r",
            fname(EXIT(ch, cmd)->keyword));
        send_to_char(tmp, ch);
        }
    else
        send_to_char("It seems to be closed.\n\r", ch);
    return;
    }
	if(!IS_NPC(ch)&&ch->specials.stpMount&&
	IS_SET(world[world[ch->in_room]->dir_option[cmd]->to_room]->extra_flags, TRADE_NO_HORSE) ){
		global_color=32;
		send_to_char("Your mount refuses to go there.\n\r",ch);
		global_color=0;
		return;	
	}

if (EXIT(ch, cmd)->to_room == NOWHERE)
        send_to_char("Alas, you can't go that way.\n\r", ch);
else if (ch->master==ch && !is_formed(ch)){
	MOUNTMOVE=TRUE;
    do_simple_move(ch,cmd,FALSE);
	MOUNTMOVE=FALSE;
}else
    {
    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master!=ch)
        && (ch->in_room == ch->master->in_room))
        {
        send_to_char(
            "The thought of leaving your master makes you weep.\n\r",
ch);
             act("$n wants to leave you!.", FALSE, ch, 0, 0, TO_ROOM);
        }
    else
        {
        was_in=ch->in_room;
		MOUNTMOVE=TRUE;
        	if (do_simple_move(ch, cmd, TRUE) == 1)
            {
            for(x=0;x<3;x++)
               for(y=0;y<3;y++)
                  {
                  if(ch->formation[x][y]&&ch->formation[x][y]!=ch)
                  if ((was_in == ch->formation[x][y]->in_room) &&
                    (GET_POS(ch->formation[x][y]) >= POSITION_STANDING))
                    {
                    global_color=1;
                    act("You follow $N.",
                        FALSE,ch->formation[x][y], 0, ch, TO_CHAR);
                    cmd++;
                    send_to_char("\n\r",ch->formation[x][y]);
                    global_color=0;
                    do_move(ch->formation[x][y], argument, cmd);
                    cmd--;
                    }
                  }
            }
         	MOUNTMOVE=FALSE;
         }
    }
}

int find_door(struct char_data *ch, char *type, char *dir)
{
    char buf[MAX_STRING_LENGTH];
    int door;
    char *dirs[] = 
    {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"
    };

    if (*dir) /* a direction was specified */
    {
	if ((door = search_block(dir, dirs, FALSE)) == -1) /* Partial Match */
	{
	    send_to_char("That's not a direction.\n\r", ch);
	    return(-1);
	}

	if (EXIT(ch, door))
	    if (EXIT(ch, door)->keyword)
		if (isname(type, EXIT(ch, door)->keyword))
		    return(door);
		else
		{
		    sprintf(buf, "I see no %s there.\n\r", type);
		    send_to_char(buf, ch);
		    return(-1);
		}
	    else
		return(door);
	else
	{
	    send_to_char( "There is no door there.\n\r", ch);
	    return(-1);
	}
    }
    else /* try to locate the keyword */
    {
	for (door = 0; door <= 5; door++)
	    if (EXIT(ch, door))
		if (EXIT(ch, door)->keyword)
		    if (isname(type, EXIT(ch, door)->keyword))
			return(door);

	sprintf(buf, "I see no %s here.\n\r", type);
	send_to_char(buf, ch);
	return(-1);
    }
}


void do_open(struct char_data *ch, char *argument, int cmd)
{
    int door, other_room;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    struct room_direction_data *back=NULL;
    struct obj_data *obj=NULL;
    struct char_data *victim=NULL;

    argument_interpreter(argument, type, dir);

    if (!*type)
	send_to_char("Open what?\n\r", ch);
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
	ch, &victim, &obj))

	/* this is an object */

	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\n\r", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("But it's already open!\n\r", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
	    send_to_char("You can't do that.\n\r", ch);
	else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	    send_to_char("It seems to be locked.\n\r", ch);
	else
	{
	    REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	    send_to_char("You open it.\n\r", ch);
	    act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
	}
    else if ((door = find_door(ch, type, dir)) >= 0)

	/* perhaps it is a door */

	if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	    send_to_char("That's impossible, I'm afraid.\n\r", ch);
        else if ( IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
		 IS_SET(EXIT(ch, door)->exit_info, EX_SECRET) &&
		 GET_LEVEL(ch) <= 34 ) {
		sprintf(log_buf,"I see no %s here.\r\n", type);
	        send_to_char(log_buf,ch);
        } else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	    send_to_char("It's already open!\n\r", ch);
	else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	    send_to_char("It seems to be locked.\n\r", ch);
	else
	{
	    REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
	    if (EXIT(ch, door)->keyword)
		act("$n opens the $F.", FALSE, ch, 0, EXIT(ch, door)->keyword,
		    TO_ROOM);
	    else
		act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);
	    send_to_char("You open it.\n\r", ch);
	    /* now for opening the OTHER side of the door! */
	    if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if ( ( back = world[other_room]->dir_option[rev_dir[door]] ) != 0 ) 
		    if (back->to_room == ch->in_room)
		    {
			REMOVE_BIT(back->exit_info, EX_CLOSED);
			if (back->keyword)
			{
			    sprintf(buf,
				"The %s is opened from the other side.\n\r",
				fname(back->keyword));
				send_to_room(buf, EXIT(ch, door)->to_room);
/*				act("The $F is opened from the other side.", FALSE, ch, 0, EXIT(ch, door)->keyword, TO_ROOM);*/
			}
			else
			    send_to_room(
			    "The door is opened from the other side.\n\r",
			    EXIT(ch, door)->to_room);
/*			    act("The door is opened from the other side.", FALSE, ch, 0, EXIT(ch, door)->keyword, TO_ROOM);*/
		    }                        
	}
}


void do_close(struct char_data *ch, char *argument, int cmd)
{
    int door, other_room;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    struct room_direction_data *back=NULL;
    struct obj_data *obj=NULL;
    struct char_data *victim=NULL;


    argument_interpreter(argument, type, dir);

    if (!*type)
	send_to_char("Close what?\n\r", ch);
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
	ch, &victim, &obj))

	/* this is an object */

	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\n\r", ch);
	else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("But it's already closed!\n\r", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
	    send_to_char("That's impossible.\n\r", ch);
	else
	{
	    SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	    send_to_char("Ok.\n\r", ch);
	    act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
	}
    else if ((door = find_door(ch, type, dir)) >= 0)

	/* Or a door */

	if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	    send_to_char("That's absurd.\n\r", ch);
        else if ( IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
                 IS_SET(EXIT(ch, door)->exit_info, EX_SECRET) ) {
	    sprintf(buf,"I see no %s there.\r\n",type);
	    send_to_char(buf,ch);
        }
	else if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	    send_to_char("It's already closed!\n\r", ch);
	else
	{
	    SET_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
	    if (EXIT(ch, door)->keyword)
		act("$n closes the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
		    TO_ROOM);
	    else
		act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
	    send_to_char("Ok.\n\r", ch);
	    /* now for closing the other side, too */
	    if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if ( ( back = world[other_room]->dir_option[rev_dir[door]] ) != 0 )
		    if (back->to_room == ch->in_room)
		    {
			SET_BIT(back->exit_info, EX_CLOSED);
			if (back->keyword)
			{
			    sprintf(buf,
				"The %s closes quietly.\n\r", back->keyword);
			    send_to_room(buf, EXIT(ch, door)->to_room);
			}
			else
			    send_to_room(
				"The door closes quietly.\n\r",
				EXIT(ch, door)->to_room);
		    }                        
	}
}


int has_key(struct char_data *ch, int key)
{
    struct obj_data *o=NULL;

    for (o = ch->carrying; o; o = o->next_content)
	if (o->item_number == key)
	    return(1);

    if (ch->equipment[HOLD])
	if (ch->equipment[HOLD]->item_number == key)
	    return(1);

    return(0);
}


void do_lock(struct char_data *ch, char *argument, int cmd)
{
    int door, other_room;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    struct room_direction_data *back=NULL;
    struct obj_data *obj=NULL;
    struct char_data *victim=NULL;


    argument_interpreter(argument, type, dir);

    if (!*type)
	send_to_char("Lock what?\n\r", ch);
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
	ch, &victim, &obj))

	/* this is an object */

	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\n\r", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("Maybe you should close it first...\n\r", ch);
	else if (obj->obj_flags.value[2] < 0)
	    send_to_char("That thing can't be locked.\n\r", ch);
	else if (!has_key(ch, obj->obj_flags.value[2]))
	    send_to_char("You don't seem to have the proper key.\n\r", ch); 
	else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	    send_to_char("It is locked already.\n\r", ch);
	else
	{
	    SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	    send_to_char("*Click*\n\r", ch);
	    act("You hear a soft click as $n locks $p.", FALSE, ch, obj, 0, TO_ROOM);
	}
    else if ((door = find_door(ch, type, dir)) >= 0)

	/* a door, perhaps */

	if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	    send_to_char("That's absurd.\n\r", ch);
        else if ( IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
                  IS_SET(EXIT(ch, door)->exit_info, EX_SECRET) &&
		  GET_LEVEL(ch) <= 34 ) 
	{            
	    sprintf(buf,"I see no %s there.\r\n",type);
            send_to_char(buf,ch);
        }
	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	    send_to_char("You have to close it first, I'm afraid.\n\r", ch);
	else if (EXIT(ch, door)->key < 0)
	    send_to_char("There do not seem to be any keyholes.\n\r", ch);
	else if (!has_key(ch, EXIT(ch, door)->key))
	    send_to_char("You don't have the proper key.\n\r", ch);
	else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	    send_to_char("It's already locked!\n\r", ch);
	else
	{
	    SET_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	    if (EXIT(ch, door)->keyword)
		act("$n locks the $F.", 0, ch, 0,  EXIT(ch, door)->keyword,
		    TO_ROOM);
	    else
		act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
	    send_to_char("*Click*\n\r", ch);
	    /* now for locking the other side, too */
	    if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if ( ( back = world[other_room]->dir_option[rev_dir[door]] ) != 0 )
		    if (back->to_room == ch->in_room)
			SET_BIT(back->exit_info, EX_LOCKED);
	}
}


void do_unlock(struct char_data *ch, char *argument, int cmd)
{
    int door, other_room, iWear;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    struct room_direction_data *back=NULL;
    struct obj_data *obj=NULL,*mykey=NULL, *next_key=NULL;
    struct char_data *victim=NULL;


    argument_interpreter(argument, type, dir);

    if (!*type)
	send_to_char("Unlock what?\n\r", ch);
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
	ch, &victim, &obj))

	/* this is an object */

	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\n\r", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("Silly - it ain't even closed!\n\r", ch);
	else if (obj->obj_flags.value[2] < 0)
	    send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
	else if (!has_key(ch, obj->obj_flags.value[2]))
	    send_to_char("You don't seem to have the proper key.\n\r", ch); 
	else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	    send_to_char("Oh.. it wasn't locked, after all.\n\r", ch);
	else
		{
	    REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	    send_to_char("*Click*\n\r", ch);
	    act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
        for (mykey = ch->carrying; mykey; mykey=next_key){
			next_key=mykey->next;
            if (mykey->item_number == obj->obj_flags.value[2])
				{
                if(mykey->obj_flags.value[1] == 1)
					{
					act("$p crumbles to dust in $n's hand.",
						TRUE, ch, mykey, 0, TO_ROOM);
					act("$p crumbles to dust in your hand.", 
						FALSE, ch, mykey, 0, TO_CHAR);
					extract_obj(mykey);
					}
                if(mykey->obj_flags.value[1] > 1)
                   mykey->obj_flags.value[1]--;
				}
		}
		for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    	  if ( ch->equipment[iWear] )
			if(ch->equipment[iWear]->item_number == obj->obj_flags.value[2]){
				if(ch->equipment[iWear]->obj_flags.value[1]==1){
					act("$p crumbles to dust in $n's hand.",
						TRUE, ch, ch->equipment[iWear], 0, TO_ROOM);
					act("$p crumbles to dust in your hand.",
						FALSE, ch, ch->equipment[iWear], 0, TO_CHAR);
					mykey=ch->equipment[iWear];
            		obj_to_char( unequip_char( ch, iWear ), ch );
					extract_obj(mykey);
				}else{
					if(ch->equipment[iWear]->obj_flags.value[1]>1)
						ch->equipment[iWear]->obj_flags.value[1]--;
				}
			}
	}
    else if ((door = find_door(ch, type, dir)) >= 0)

	/* it is a door */

	if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	    send_to_char("That's absurd.\n\r", ch);
        else if ( IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
                 IS_SET(EXIT(ch, door)->exit_info, EX_SECRET) ) {
            sprintf(buf,"I see no %s there.\r\n",type);
            send_to_char(buf,ch);
        }
	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	    send_to_char("Heck ... it ain't even closed!\n\r", ch);
	else if (EXIT(ch, door)->key < 0)
	    send_to_char("You can't seem to spot any keyholes.\n\r", ch);
	else if (!has_key(ch, EXIT(ch, door)->key))
	    send_to_char("You do not have the proper key for that.\n\r", ch);
	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	    send_to_char("It's already unlocked, it seems.\n\r", ch);
	else
	{
	    REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	    if (EXIT(ch, door)->keyword)
		act("$n unlocks the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
		    TO_ROOM);
	    else
		act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
	    send_to_char("*click*\n\r", ch);
	    /* now for unlocking the other side, too */
	    if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if ( ( back = world[other_room]->dir_option[rev_dir[door]] ) != 0 )
		    if (back->to_room == ch->in_room)
			REMOVE_BIT(back->exit_info, EX_LOCKED);
        for (mykey = ch->carrying; mykey; mykey = mykey->next_content)
            if (mykey->item_number == EXIT(ch, door)->key)
				{
                if(mykey->obj_flags.value[1] == 1)
					{
					act("$p crumbles to dust in $n's hand.",
						TRUE, ch, mykey, 0, TO_ROOM);
					act("$p crumbles to dust in your hand.",
						FALSE, ch, mykey, 0, TO_CHAR);
					extract_obj(mykey);
					}
                if(mykey->obj_flags.value[1] > 1)
                   mykey->obj_flags.value[1]--;
				}
	}
}


void do_pick(struct char_data *ch, char *argument, int cmd)
{
   byte percent;
    int door, other_room;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    struct room_direction_data *back=NULL;
    struct obj_data *obj=NULL;
    struct char_data *victim=NULL;

    argument_interpreter(argument, type, dir);

   percent=number(1,101); /* 101% is a complete failure */

   if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
      send_to_char("You failed to pick the lock.\n\r", ch);
      return;
    }

    if (!*type)
	send_to_char("Pick what?\n\r", ch);
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
	ch, &victim, &obj))

	/* this is an object */

	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\n\r", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("Silly - it ain't even closed!\n\r", ch);
	else if (obj->obj_flags.value[2] < 0)
	    send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	    send_to_char("Oho! This thing is NOT locked!\n\r", ch);
	else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF))
	    send_to_char("It resists your attempts at picking it.\n\r", ch);
	else
	{
	    REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	    send_to_char("*Click*\n\r", ch);
	    act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
	}
    else if ((door = find_door(ch, type, dir)) >= 0)
	if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	    send_to_char("That's absurd.\n\r", ch);
        else if ( IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
                 IS_SET(EXIT(ch, door)->exit_info, EX_SECRET) ) {
            sprintf(buf,"I see no %s there.\r\n",type);
            send_to_char(buf,ch);
        }
	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	    send_to_char("You realize that the door is already open.\n\r", ch);
	else if (EXIT(ch, door)->key < 0)
	    send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
	else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	    send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
	else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
	    send_to_char("You seem to be unable to pick this lock.\n\r", ch);
	else
	{
	    REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	    if (EXIT(ch, door)->keyword)
		act("$n skillfully picks the lock of the $F.", 0, ch, 0,
		    EXIT(ch, door)->keyword, TO_ROOM);
	    else
		act("$n picks the lock of the.", TRUE, ch, 0, 0, TO_ROOM);
	    send_to_char("The lock quickly yields to your skills.\n\r", ch);
	    /* now for unlocking the other side, too */
	    if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	    if ( ( back = world[other_room]->dir_option[rev_dir[door]] ) != 0 )
		    if (back->to_room == ch->in_room)
			REMOVE_BIT(back->exit_info, EX_LOCKED);
	}
}


void do_enter(struct char_data *ch, char *argument, int cmd)
{
    int door;
    char buf[MAX_INPUT_LENGTH], tmp[MAX_STRING_LENGTH];

    void do_move(struct char_data *ch, char *argument, int cmd);

    one_argument(argument, buf);

    if (*buf)  /* an argument was supplied, search for door keyword */
    {
	for (door = 0; door <= 5; door++)
	    if (EXIT(ch, door))
		if (EXIT(ch, door)->keyword)
		    if (!str_cmp(EXIT(ch, door)->keyword, buf))
		    {
			do_move(ch, "", ++door);
			return;
		    }
	sprintf(tmp, "There is no %s here.\n\r", buf);
	send_to_char(tmp, ch);
    }
    else
	if (IS_SET(world[ch->in_room]->room_flags, INDOORS))
	    send_to_char("You are already indoors.\n\r", ch);
	else
	{
	    /* try to locate an entrance */
	    for (door = 0; door <= 5; door++)
		if (EXIT(ch, door))
		    if (EXIT(ch, door)->to_room != NOWHERE)
			if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
			    IS_SET(world[EXIT(ch, door)->to_room]->room_flags,
			    INDOORS))
			{
			    do_move(ch, "", ++door);
			    return;
			}
	    send_to_char("You can't seem to find anything to enter.\n\r", ch);
	}
}


void do_leave(struct char_data *ch, char *argument, int cmd)
{
    int door;

    void do_move(struct char_data *ch, char *argument, int cmd);

    if (!IS_SET(world[ch->in_room]->room_flags, INDOORS))
	send_to_char("You are outside.. where do you want to go?\n\r", ch);
    else
    {
	for (door = 0; door <= 5; door++)
	    if (EXIT(ch, door))
		if (EXIT(ch, door)->to_room != NOWHERE)
		    if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
			!IS_SET(world[EXIT(ch, door)->to_room]->room_flags,
				INDOORS))
		    {
			do_move(ch, "", ++door);
			return;
		    }
	send_to_char("I see no obvious exits to the outside.\n\r", ch);
    }
}

void do_SecDrSearch(struct char_data *ch, char *argument, int cmd)
{
int door, other_room, chance, found=0, found_key=0;
struct room_direction_data *back=NULL;
char buf[MAX_STRING_LENGTH];
struct obj_data *mykey;
char OK=0;

	char *exits[] =
	{
		"north wall",
		"east wall",
		"south wall",
		"west wall",
		"ceiling",
		"floor"
	};
	
	if(!ch) return;
	if(ch->in_room == NOWHERE) return;

	chance = (ch->abilities.intel+ch->abilities.wis+(ch->abilities.dex)*2)/2;
	if(GET_CLASS(ch) == CLASS_THIEF)
		chance += GET_LEVEL(ch)*1.2;
			
	act("$n attempts to search the room...", TRUE,ch,0,0,TO_ROOM);

	if (number(0,100) < chance) OK=1;
	WAIT_STATE(ch, 10);
	
	for (door = 0; door <= 5; door++) {
	  if (EXIT(ch, door))
	    if ( EXIT(ch, door)->to_room != NOWHERE &&
		 IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) ) {

		if ( !IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN) &&
		    (!IS_SET(EXIT(ch, door)->exit_info, EX_SECRET) || !OK) )
			continue;

	 	/* Key code + crumbleing */
	 	if(IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
 		  if (has_key(ch, EXIT(ch, door)->key)) {
		    REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
		    found_key=1;
		    for (mykey = ch->carrying; mykey; mykey = mykey->next_content)
		    if (mykey->item_number == EXIT(ch, door)->key) {
		      if (mykey->obj_flags.value[1] == 1) {
			send_to_char("The key crumbles in your hands.\r\n",ch);
			extract_obj(mykey);
		      } else if(mykey->obj_flags.value[1] > 1)
			mykey->obj_flags.value[1]--;
		    }
		  } else {
		    sprintf(buf,"You see a faint outline of a %s but can't seem to open it.\r\n",
		    EXIT(ch, door)->keyword);
		    send_to_char(buf,ch);
		    return;
		  }
		} /* if door is locked */

		/* After done with keys, remove Closed bit */
	 	REMOVE_BIT(EXIT(ch, door)->exit_info,EX_CLOSED);
	 	found=1;

	 	/* Message for the char */
	 	if (EXIT(ch, door)->keyword) 
 		  sprintf(buf,"Searching the area, you find a secret %s in the %s.\r\n",
			  EXIT(ch, door)->keyword,
			  exits[door]);
		else 
		  sprintf(buf,"You notice a hidden pannel in the wall and open it.\r\n");
	  	send_to_char(buf, ch);

		/* Now to the people in the room */
		if (EXIT(ch, door)->keyword)
		  sprintf(buf,"$n finds a secret %s in the %s and opens it.",
			  EXIT(ch, door)->keyword,
			  exits[door]);
		else
		  sprintf(buf,"$n finds a secret door and opens it.");
		act(buf,TRUE,ch,0,0,TO_ROOM);

		/* Now remove the closed on the other side */
	 	if ((other_room = EXIT(ch, door)->to_room) != NOWHERE)
	 	  if ( ( back = world[other_room]->dir_option[rev_dir[door]] ) != 0 )
		    if (back->to_room == ch->in_room &&	IS_SET(back->exit_info, EX_CLOSED)) {
		      REMOVE_BIT(back->exit_info, EX_CLOSED);
		      if (IS_SET(back->exit_info, EX_LOCKED) && found_key == 1)
			REMOVE_BIT(back->exit_info, EX_LOCKED);
		    }
				 				
	    } /* if door is a closed/secret door */
	} /* for 0-5 exits */
	
	if(found == 0)
	  send_to_char("Your search of the room yields nothing special.\r\n",ch);
	return;
}                            

void do_stand(struct char_data *ch, char *argument, int cmd)
{
    global_color=32;
    switch(GET_POS(ch)) {
	case POSITION_STANDING : { 
	    act("You are already standing.",FALSE, ch,0,0,TO_CHAR);
	} break;
	case POSITION_SITTING   : { 
	    act("You stand up.", FALSE, ch,0,0,TO_CHAR);
	    act("$n clambers to $s feet.",TRUE, ch, 0, 0, TO_ROOM);
	    GET_POS(ch) = POSITION_STANDING;
	} break;
	case POSITION_RESTING   : { 
	    act("You stop resting, and stand up.", FALSE, ch,0,0,TO_CHAR);
	    act("$n stops resting, and clambers to $s feet.",
		TRUE, ch, 0, 0, TO_ROOM);
	    GET_POS(ch) = POSITION_STANDING;
	} break;
	case POSITION_SLEEPING : { 
	    act("You have to wake up first!", FALSE, ch, 0,0,TO_CHAR);
	} break;
	case POSITION_FIGHTING : { 
	    act("Do you not consider fighting as standing?",
		FALSE, ch, 0, 0, TO_CHAR);
	} break;
	default : { 
	    act("You stop floating around, and put your feet on the ground.",
	      FALSE, ch, 0, 0, TO_CHAR);
	    act("$n stops floating around, and puts $s feet on the ground.",
	      TRUE, ch, 0, 0, TO_ROOM);
	} break;
    }
    global_color=0;
}


void do_sit(struct char_data *ch, char *argument, int cmd)
{
    global_color=32;
    switch(GET_POS(ch)) {
	case POSITION_STANDING : {
		if(ch->specials.stpMount)
			do_unmount(ch,"",9);
	    act("You sit down.", FALSE, ch, 0,0, TO_CHAR);
	    act("$n sits down.", FALSE, ch, 0,0, TO_ROOM);
	    GET_POS(ch) = POSITION_SITTING;
	} break;
	case POSITION_SITTING   : {
	    send_to_char("You're sitting already.\n\r", ch);
	} break;
	case POSITION_RESTING   : {
	    act("You stop resting, and sit up.", FALSE, ch,0,0,TO_CHAR);
	    act("$n stops resting.", TRUE, ch, 0,0,TO_ROOM);
	    GET_POS(ch) = POSITION_SITTING;
	} break;
	case POSITION_SLEEPING : {
	    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
	} break;
	case POSITION_FIGHTING : {
	    act("Sit down while fighting? are you MAD?",
		FALSE, ch,0,0,TO_CHAR);
	} break;
	default : {
	    act("You stop floating around, and sit down.",
		FALSE, ch,0,0,TO_CHAR);
	    act("$n stops floating around, and sits down.",
	    	TRUE, ch,0,0,TO_ROOM);
	    GET_POS(ch) = POSITION_SITTING;
	} break;
    }
    global_color=0;
}
	    

void do_rest(struct char_data *ch, char *argument, int cmd)
{
global_color=32;
    switch(GET_POS(ch)) {
	case POSITION_STANDING : {
		if(ch->specials.stpMount)
			do_unmount(ch,"",9);
	    act("You sit down and rest your tired bones.",
		FALSE, ch, 0, 0, TO_CHAR);
	    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
	    GET_POS(ch) = POSITION_RESTING;
	} break;
	case POSITION_SITTING : {
	    act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
	    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
	    GET_POS(ch) = POSITION_RESTING;
	} break;
	case POSITION_RESTING : {
	    act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
	} break;
	case POSITION_SLEEPING : {
	    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
	    } break;
	case POSITION_FIGHTING : {
	    act("Rest while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
	} break;
	default : {
	    act("You stop floating around, and stop to rest your tired bones.",
	      FALSE, ch, 0, 0, TO_CHAR);
	    act("$n stops floating around, and rests.",
	    FALSE, ch, 0,0, TO_ROOM);
	    GET_POS(ch) = POSITION_SITTING;
	} break;
    }
    global_color=0;
}


void do_sleep(struct char_data *ch, char *argument, int cmd)
{

struct char_data *m;

    global_color=32;
	if(!IS_NPC(ch)&&ch->specials.stpMount){
		send_to_char("You try and fall asleep on a beast... it didn't work...\n\r",ch);
		return;
	}
    switch(GET_POS(ch)) {
	case POSITION_STANDING : 
	case POSITION_SITTING  :
	case POSITION_RESTING  : {
	    send_to_char("You go to sleep.\n\r", ch);
	    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
	    GET_POS(ch) = POSITION_SLEEPING;
	} break;
	case POSITION_SLEEPING : {
	    send_to_char("You are already sound asleep.\n\r", ch);
	} break;
	case POSITION_FIGHTING : {
	    send_to_char("Sleep while fighting? are you MAD?\n\r", ch);
	} break;
	default : {
	    act("You stop floating around, and lie down to sleep.",
	      FALSE, ch, 0, 0, TO_CHAR);
	    act("$n stops floating around, and lie down to sleep.",
	      TRUE, ch, 0, 0, TO_ROOM);
	    GET_POS(ch) = POSITION_SLEEPING;
	} break;
    }
    if(GET_POS(ch)!=POSITION_SLEEPING)return;
    global_color=0;
    if(world[ch->in_room]->zone==198){
	for(m=world[ch->in_room]->people;m;m=m->next_in_room)
	    if(m->nr==17021){
		dream_weaver(ch);	
		return;
	    }
    }
}


void do_wake(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *tmp_char=NULL;
    char arg[MAX_STRING_LENGTH];

    global_color=32;

    one_argument(argument,arg);
    if (*arg) {
	if (GET_POS(ch) == POSITION_SLEEPING) {
	    act("You can't wake people up if you are asleep yourself!",
		FALSE, ch,0,0,TO_CHAR);
	} else {
	    tmp_char = get_char_room_vis(ch, arg);
	    if (tmp_char) {
		if (tmp_char == ch) {
		    act("If you want to wake yourself up, just type 'wake'",
			FALSE, ch,0,0,TO_CHAR);
		} else {
		    if (GET_POS(tmp_char) == POSITION_SLEEPING) {
			if (IS_AFFECTED(tmp_char, AFF_SLEEP)) {
			    act("You can not wake $M up!",
				FALSE, ch, 0, tmp_char, TO_CHAR);
			} else {
			    act("You wake $M up.",
			    FALSE, ch, 0, tmp_char, TO_CHAR);
			    GET_POS(tmp_char) = POSITION_STANDING;
			    act("You are awakened by $n.",
			    FALSE, ch, 0, tmp_char, TO_VICT);
			}
		    } else {
			act("$N is already awake.",
				FALSE,ch,0,tmp_char, TO_CHAR);
		    }
		}
	    } else {
		send_to_char("You do not see that person here.\n\r", ch);
	    }
	}
    } else {
	if (IS_AFFECTED(ch,AFF_SLEEP)) {
	    send_to_char("You can't wake up!\n\r", ch);
	} else {
	    if (GET_POS(ch) > POSITION_SLEEPING)
		send_to_char("You are already awake...\n\r", ch);
	    else {
		send_to_char("You wake, and stand up.\n\r", ch);
		act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
		GET_POS(ch) = POSITION_STANDING;
	    }
	}
    }
    global_color=0;
}


