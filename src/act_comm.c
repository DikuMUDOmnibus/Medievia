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


#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#ifndef __FreeBSD__
#include <malloc.h>
#endif

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

/* extern variables */
extern bool guy_deleted;
extern char global_color;
extern char NOPKCHANGEMESSAGE;
struct room_data *world[MAX_ROOM]; /* array of rooms                  */
extern struct obj_data  *objs[MAX_OBJ];
extern struct descriptor_data *descriptor_list;
extern struct global_clan_info_struct global_clan_info;
extern char echo_text(struct char_data *ch, char *text);
extern void align_string(struct char_data *ch, char *buf);

/* Internal function declarations */
void shout_stuff(struct char_data *, char *, int, int);
void shout_stuff_one(struct char_data *, char *, int, int);
char *set_vector_text(int, int);

void CorruptString(char *szpNew, char *szpString, int iPercent)
{
  int iLen=0;
  int iCLevel, i;
  char *szReplaceChars = "...___..........................aa............._____";
  iLen = strlen(szpString);

  iCLevel = iLen * (float) ((float) iPercent/100);

  strcpy(szpNew,szpString);

  for(i=0;i<iCLevel;i++)
	szpNew[number(0,iLen-1)] = szReplaceChars[number(0,50)];
  return;

}

void do_auction(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *to=NULL;
    int x, flag=0;
    char buf[MAX_STRING_LENGTH];

    if(get_total_level(ch)<6){
	send_to_char("
\rSorry due to the abusive people in the world, we have made it so you 
\rcannot auction till level 6. At that point we will know that you are 
\rserious about the mud and are not here just to cause trouble, if you 
\rneed to speak to a god use the PRAY command.\n\r",ch);
	return;
    }    

    if (IS_SET(ORIGINAL(ch)->specials.act, PLR_NOTELL))
    {
	send_to_char("The gods have shut you up!\n\r", ch);
	return;
    }
    if(!ch->desc
	||IS_SET(ORIGINAL(ch)->specials.new_comm_flags,PLR_NOAUCTION)){
        send_to_char("Your auction is turned off!", ch); 
        return; 
    }
    for (x = 0; *(argument + x) == ' '; x++); 
    if (!*(argument + x))
        send_to_char("Yes, but WHAT do you want to say?\n\r", ch); 
    else{
        for(to=descriptor_list; to; to=to->next){
	   
        if((ch!=to->character)
		&&!to->connected
		&&!IS_SET(ORIGINAL(to->character)->specials.new_comm_flags,
			PLR_NOAUCTION) 
		&&!IS_SET(ORIGINAL(to->character)->specials.plr_flags, 
			PLR_WRITING)
		){
		global_color=35;
                sprintf(buf,"%s Auctions, '%s'.\n\r",(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument+x); 
                send_to_char(buf,to->character); 
		global_color=0;
                flag++; 
	    }
	}
        if(flag){
	    global_color=35;
            sprintf(buf,"%s[%d] people heard you auction, '%s'.\n\r", MAG(ch), flag, argument+x); 
            send_to_char(buf,ch); 
	    global_color=0;
        }else{
            send_to_char("There are no people in Medievia with Auction turned on.", ch); 
	}
    }
}

void do_discuss(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *to=NULL;
    int x, flag=0;
    char gflag=0;
    char buf[MAX_STRING_LENGTH];


    if(!ch||!argument)return;
    if(get_total_level(ch)<6){
	send_to_char("
Sorry due to the abusive people in the world, we have made it so you 
cannot discuss till level 6. At that point we will know that you are 
serious about the mud and are not here just to cause trouble, if you 
need to speak to a god use the PRAY command.\n\r",ch);
	return;
    }    
    if (IS_SET(ORIGINAL(ch)->specials.act, PLR_NOTELL))
    {
	send_to_char("The gods have shut you up!\n\r", ch);
	return;
    }
    if(!ch->desc
	|| IS_SET(ORIGINAL(ch)->specials.new_comm_flags,PLR_NODISCUSS)){
        send_to_char("Your Discussion channel is turned off!\n\r", ch); 
        return; 
    }
    for(to=descriptor_list; to; to=to->next){
	if(to->character&&GET_LEVEL(to->character)<33)break;
	if(to->character&&!to->connected
	&&!IS_SET(ORIGINAL(to->character)->specials.new_comm_flags,PLR_NODISCUSS)
	&&GET_LEVEL(to->character)>32){
	    gflag=1;
	    break;
	}
    }
    if(!gflag){
	send_to_char("No GODS are present with DISCUSSION channel turned on.\n\r",ch);
	return; 
    }
    for (x = 0; *(argument + x) == ' '; x++); 
    if (!*(argument + x))
        send_to_char("Yes, but WHAT do you want to Discuss?\n\r",ch); 
    else{
        for(to=descriptor_list; to; to=to->next){
	if((ch!=to->character)
		&&!to->connected
		&&!IS_SET(ORIGINAL(to->character)->specials.new_comm_flags,
			PLR_NODISCUSS) 
		&&!IS_SET(ORIGINAL(to->character)->specials.plr_flags,
			PLR_WRITING)
		){
		if(to->character->specials.ansi_color==69)
                   sprintf(buf,"\033[1m\033[35m%s Discusses, '%s'.\n\r\033[0m",(IS_NPC(ch) ? ch ->player.short_descr : GET_NAME(ch)),argument+x); 
		else
                   sprintf(buf,"%s Discusses, '%s'.\n\r",(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument+x); 
                send_to_char(buf,to->character); 
                flag++; 
	    }
	}
        if(flag){
	  if(ch->specials.ansi_color==69)
            sprintf(buf,"\033[1m\033[35m[%d] people heard you Discuss, '%s'.\n\r\033[0m", flag, argument+x); 
	  else
            sprintf(buf,"[%d] people heard you Discuss, '%s'.\n\r", flag, argument+x); 
            send_to_char(buf,ch); 
        }else{
            send_to_char("There are no people in Medievia with DISCUSSION CHANNEL turned on.", ch); 
	}
    }
}

void do_quest(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *to=NULL;
    int x, flag=0;
    char buf[MAX_STRING_LENGTH];
    char gflag=0;

    if(!ch||!argument)return;
    if(get_total_level(ch)<6){
	send_to_char("
Sorry due to the abusive people in the world, we have made it so you 
cannot quest till level 6. At that point we will know that you are 
serious about the mud and are not here just to cause trouble, if you 
need to speak to a god use the PRAY command.\n\r",ch);
	return;
    }    
    if (IS_SET(ORIGINAL(ch)->specials.act, PLR_NOTELL))
    {
	send_to_char("The gods have shut you up!\n\r", ch);
	return;
    }
    if(!ch->desc||IS_SET(ORIGINAL(ch)->specials.new_comm_flags,PLR_NOQUEST)){
        send_to_char("Your Quest channel is turned off!", ch); 
        return; 
    }
    for(to=descriptor_list; to; to=to->next){
	if(to->character&&GET_LEVEL(to->character)<33)break;
	if(to->character&&!to->connected
	&&!IS_SET(ORIGINAL(to->character)->specials.new_comm_flags,PLR_NOQUEST)
	&&GET_LEVEL(to->character)>32){
	    gflag=1;
	    break;
	}
    }
    if(!gflag){
	send_to_char("The QUEST channel is not turned on.\n\r",ch);
	return; 
    }


    for (x = 0; *(argument + x) == ' '; x++); 
    if (!*(argument + x))
        send_to_char("Yes, but WHAT do you want to quest channel say?\n\r",ch); 
    else{
        for(to=descriptor_list; to; to=to->next){
	   
	if((ch!=to->character)
		&&!to->connected
		&&!IS_SET(ORIGINAL(to->character)->specials.new_comm_flags,
			PLR_NOQUEST) 
		&&!IS_SET(ORIGINAL(to->character)->specials.plr_flags,
			PLR_WRITING)
		){
		if(to->character->specials.ansi_color==69)
                   sprintf(buf,"\033[1m\033[35m%s Quests, '%s'.\n\r\033[0m",(IS_NPC(ch) ? ch ->player.short_descr : GET_NAME(ch)),argument+x); 
		else
                   sprintf(buf,"%s quests, '%s'.\n\r",(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument+x); 
                send_to_char(buf,to->character); 
                flag++; 
	    }
	}
        if(flag){
	  if(ch->specials.ansi_color==69)
            sprintf(buf,"\033[1m\033[35m[%d] people heard you quest, '%s'.\n\r\033[0m", flag, argument+x); 
	  else
            sprintf(buf,"[%d] people heard you quest, '%s'.\n\r", flag, argument+x); 
            send_to_char(buf,ch); 
        }else{
            send_to_char("There are no people in Medievia with Quest turned on.", ch); 
	}
    }
}

void do_gossip(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *to=NULL;
    int x, flag=0;
    char buf[MAX_STRING_LENGTH];
    int m,s,t;

    if(!ch||!argument)return;

    if(get_total_level(ch)<6&&!IS_NPC(ch)){
	send_to_char("
Sorry due to the abusive people in the world, we have made it so you 
cannot gossip till level 6. At that point we will know that you are 
serious about the mud and are not here just to cause trouble, if you 
need to speak to a god use the PRAY command.\n\r",ch);
	return;
    }
    if(ch->desc && (ch->desc->connected != CON_SOCIAL_ZONE) && GET_LEVEL(ch) < 33 ) {
	send_to_char("You can only use gossip in MedLink.\r\n",ch);
	return;
    }  
    if (IS_SET(ORIGINAL(ch)->specials.act, PLR_NOTELL))
    {
	send_to_char("The gods have shut you up!\n\r", ch);
	return;
    }
    if(!IS_NPC(ch))
      if(!ch->desc
		||IS_SET(ORIGINAL(ch)->specials.new_comm_flags,PLR_NOGOSSIP)){
        send_to_char("Your gossip is turned off!", ch); 
        return; 
     }
    
    if(   (GET_LEVEL(ch)<32)
		&&(ORIGINAL(ch)->p)
		&&(ORIGINAL(ch)->p->gossip_secs+(60*5)>1+time(0))){
	t=(ORIGINAL(ch)->p->gossip_secs+(60*5))-time(0);
	m=t/60;
	s=t%60;
	sprintf(log_buf,"Gossiping is ONCE per 5 minutes, you have to wait another %d min %d sec.\n\r",m,s);
	send_to_char(log_buf,ch);
	return;
    }
    if(IS_NPC(ch)&&IS_AFFECTED( ch, AFF_CHARM ) )return;

    for (x = 0; *(argument + x) == ' '; x++); 
    if (!*(argument + x))
        send_to_char("Yes, but WHAT do you want to gossip?\n\r", ch); 
    else{
        for(to=descriptor_list; to; to=to->next){
	   
	if( (to->character&&ch!=to->character)
	    && ( (to->connected == CON_SOCIAL_ZONE) ||
		 (GET_LEVEL(to->character) < 33) )
	    
	    &&!IS_SET(ORIGINAL(to->character)->specials.new_comm_flags, 
		PLR_NOGOSSIP)
	    &&!IS_SET(ORIGINAL(to->character)->specials.plr_flags, 
		PLR_WRITING)){
		if(to->character->specials.ansi_color==69)
                   sprintf(buf,"\033[1m\033[35m%s gossips, '%s'.\n\r\033[0m",(IS_NPC(ch) ? ch ->player.short_descr : GET_NAME(ch)),argument+x); 
		else
                   sprintf(buf,"%s gossips, '%s'.\n\r",(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument+x); 
                send_to_char(buf,to->character); 
                flag++; 
	    }
	}
        if(flag){
	  if(ORIGINAL(ch)->p)
	  	ORIGINAL(ch)->p->gossip_secs=time(0);
	  if(ch->specials.ansi_color==69)
            sprintf(buf,"\033[1m\033[35m[%d] people heard you gossip, '%s'.\n\r\033[0m", flag, argument+x); 
	  else
            sprintf(buf,"[%d] people heard you gossip, '%s'.\n\r", flag, argument+x); 
            send_to_char(buf,ch); 
        }else{
            send_to_char("There are no people in Medievia with Gossip turned on.", ch); 
	}
    }


}

void do_pray(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *to=NULL;
    int x, flag=0;
    char buf[MAX_STRING_LENGTH];
    void reanimate(struct descriptor_data *d, int cmd);

    if(IS_NPC(ORIGINAL(ch))){
	send_to_char("Good try, but the GODS ignore you!", ch);
        return; 
    }

    if(IS_SET(world[ch->in_room]->room_flags, CHURCH) 
        && IS_UNDEAD(ch)
        && (ORIGINAL(ch)->specials.death_timer <1)
    )
    {
        /*ORIGINAL(ch)->specials.numpkills -=
                number(1, (int)(ch->specials.numpkills/5));*/
        ORIGINAL(ch)->specials.numpkills = ch->specials.numpkills/2;
        if(ORIGINAL(ch)->specials.numpkills < 1)
                ORIGINAL(ch)->specials.numpkills = 0;
    reanimate(ch->desc, 0);
    guy_deleted = TRUE;
    return;    
    }

    if (IS_SET(ORIGINAL(ch)->specials.act, PLR_NOTELL))
    {
	send_to_char("The gods have shut you up!\n\r", ch);
	return;
    }
    for (x = 0; *(argument + x) == ' '; x++); 
    if (!*(argument + x))
        send_to_char("Yes, but WHAT do you want to pray?\n\r", ch); 
    else{
        for(to=descriptor_list; to; to=to->next){
	   
	if((!to->connected)&&(GET_LEVEL(to->character)>=31)
		&&(!to->character->specials.afk)
		&&(!to->character->specials.wizInvis)
		&&!IS_SET(ORIGINAL(to->character)->specials.plr_flags,
			PLR_WRITING)){
                sprintf(buf,"%s prays '%s'.\n\r", GET_NAME(ch),argument+x); 
		global_color=34;
                send_to_char(buf,to->character); 
		global_color=0;
                flag++; 
	    }
	}
        if(flag){
            sprintf(buf,"[%d] God(s) heard you pray '%s'.\n\r", flag, argument+x); 
	    global_color=1;
            send_to_char(buf,ch); 
	    global_color=0;
        }else{
            send_to_char("There are no GODS in Medievia at the moment.", ch); 
	}
    }
}

void do_dream(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *to=NULL;
    int x;
    char buf[MAX_STRING_LENGTH];
    char flag=0; 
    int iHoloChDistance(struct char_data *ch, struct char_data *vict);
    
    if(GET_POS(ch) != POSITION_SLEEPING){
	send_to_char("You daydream a while...\n\r",ch);
	return;
    }
    if(!ch||!argument)return;
    for (x = 0; *(argument + x) == ' '; x++); 
    if (!*(argument + x))
        send_to_char("Yes, but WHAT do you want to dream?\n\r", ch); 
    else{
        for(to=descriptor_list; to; to=to->next){
	   if(!to->character) continue;
 	  if((!to->connected)&&(GET_POS(to->character)==POSITION_SLEEPING)&&(ch!=to->character)){
			if(iHoloChDistance(ch,to->character) < 250) {
                   sprintf(buf,"%s dreams '%s'.\n\r",GET_NAME(ch),argument+x); 
                   send_to_char(buf,to->character); 
                   flag++; 
            }
	  }
	}
        if(flag){
            sprintf(buf,"[%d] people heard you dream '%s'.\n\r", flag,argument+x); 
            send_to_char(buf,ch); 
        }else{
            send_to_char("There are no sleeping people in Medievia right now\n\r",ch); 
	}
    }
}


void do_asay(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *to=NULL;
    int x;
    char buf[MAX_STRING_LENGTH];
    char ch_alignment[10], toalignment[10], flag=0;
    if(!ch||!argument)return;
    to=world[ch->in_room]->people; 
    for (x = 0; *(argument + x) == ' '; x++); 
    if (!*(argument + x))
        send_to_char("Yes, but WHAT do you want to say?\n\r", ch); 
    else{
        align_string(ch,ch_alignment);
	for(; to; to=to->next_in_room){
	    align_string(to, toalignment);
            if((!strcmp(ch_alignment, toalignment))
		&&(ch!=to)
		&&!IS_NPC(to)
		&&!IS_SET(ORIGINAL(to)->specials.plr_flags, PLR_WRITING)){
               sprintf(buf,"%s says to all %s people, '%s'.\n\r",(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),ch_alignment,argument + x); 
                send_to_char(buf,to); 
                flag++; 
	    }
	}
        if(flag){
            sprintf(buf,"[%d] %s people heard you say, '%s'.\n\r", flag, ch_alignment, argument +x); 
            send_to_char(buf,ch); 
        }else{
            sprintf(buf,"There are no %s people in the room.\n\r",ch_alignment); 
            send_to_char(buf,ch); 
	}
    }
}

void align_string(struct char_data *ch, char *buf)
{
    if(GET_ALIGNMENT(ch)<=-333)
	strcpy(buf,"Evil");
    else if(GET_ALIGNMENT(ch)>=333)
	strcpy(buf,"Good");
    else{
        strcpy(buf,"Neutral");
    }
}



void do_clsay(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *to=NULL;
    int x;
    char buf[MAX_STRING_LENGTH]; 
    char class[10],flag=0;
    to=world[ch->in_room]->people;

    if(!ch||!argument)return;
    for (x = 0; *(argument + x) == ' '; x++); 
    if (!*(argument + x))
        send_to_char("Yes, but WHAT do you want to say?\n\r", ch); 
    else{
	switch(GET_CLASS(ch)){
		case CLASS_WARRIOR:
			sprintf(class, "Warriors");
			break;
		case CLASS_THIEF:
			sprintf(class, "Thiefs");
			break;
		case CLASS_CLERIC:
			sprintf(class, "Cleric's");
			break;
		case CLASS_MAGIC_USER:
			sprintf(class, "Magic User's");
			break;
		default:
			return;
	
	}
	for(; to; to=to->next_in_room)
	    if((GET_CLASS(ch)==GET_CLASS(to))
		&&(ch!=to)
		&&!IS_SET(ORIGINAL(to)->specials.plr_flags, PLR_WRITING)){
	       sprintf(buf,"%s says to all %s, '%s'.\n\r",(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),class,argument + x); 
		send_to_char(buf,to);
		flag++;
	    }
	if(flag){
            sprintf(buf,"[%d] %s heard you say, '%s'.\n\r", flag, class,
		argument +x); 
            send_to_char(buf,ch); 
	}else{
	    sprintf(buf,"There are no %s in the room.\n\r",class);
	    send_to_char(buf,ch);
	}
    }
}


void do_talk(struct char_data *ch, char *argument, int cmd) {
    struct char_data *vict=NULL;
    char name[100], message[MAX_STRING_LENGTH],
	buf[MAX_STRING_LENGTH];

    half_chop(argument,name,message);

    if(!*name || !*message)
	send_to_char("Who do you want to talk to.. and what??\n\r", ch);
    else if (!(vict = get_char_room_vis(ch, name)))
	send_to_char("No-one by that name here..\n\r", ch);
    else if (vict == ch) {
	act("$n talks quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
	send_to_char(
	    "You can't seem to get your mouth close enough to your ear...\n\r",
	     ch);
    }
    else if(IS_SET(ORIGINAL(vict)->specials.plr_flags,PLR_WRITING))
	act("$E is editing right now, try again later.", FALSE, ch, 0, vict, TO_CHAR);
    else {
        global_color=36;
	sprintf(buf,"$n says to you, '%s'.",message);
	act(buf, FALSE, ch, 0, vict, TO_VICT);
	sprintf(buf,"You say to $N, '%s'.",message);
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
	sprintf(buf,"$n says to $N, '%s'.",message);
	act(buf, FALSE, ch, 0, vict, TO_NOTVICT);
	global_color=0;
    }
}


void do_say(struct char_data *ch, char *argument, int cmd)
{
    int i;
    char buf[MAX_STRING_LENGTH];

    for (i = 0; *(argument + i) == ' '; i++);

    if (!*(argument + i))
	send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
    else {
	if((GET_ZONE(ch)==198)&&!IS_NPC(ch)){
	    if(echo_text(ch, argument)){
		global_color=36;
		sprintf(buf,"$n says, '%s'.", log_buf);
		act(buf,FALSE,ch,0,0,TO_ROOM);
		sprintf(buf,"You say, '%s'.", log_buf);
		act(buf,FALSE,ch,0,0,TO_CHAR);
		global_color=0;
		return;
	    }
	}
	global_color=36;
	sprintf(buf,"$n says, '%s'.", argument + i);
	act(buf,FALSE,ch,0,0,TO_ROOM);
	sprintf(buf,"You say, '%s'.", argument + i);
	act(buf,FALSE,ch,0,0,TO_CHAR);
	global_color=0;
    }
}


void do_shout(struct char_data *ch, char *argument, int cmd)
{
    if(!ch||!argument)return;

    if( !IS_NPC(ch) && (get_total_level(ch)<3) ){
	send_to_char("
\rSorry due to the abusive people in the world, we have made it so you 
\rcannot shout till level 3. At that point we will know that you are 
\rserious about the mud and are not here just to cause trouble, if you 
\rneed to speak to a god use the PRAY command.\n\r",ch);
	return;
    }    

    if ((ch->desc) 
		&& !IS_UNDEAD(ch)
		&& (IS_SET(ORIGINAL(ch)->specials.act, PLR_NOSHOUT)))
    {
	send_to_char("You can't shout!!\n\r", ch);
	return;
    }

    
    if (GET_MOVE(ch) < 15)
    {
	send_to_char("You are too exhausted to shout.\n\r", ch);
	return;
    }

    GET_MOVE(ch) -= 15;

    for (; *argument == ' '; argument++);

    if (!(*argument))
	send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n\r", ch);
    else {
	global_color=35;
	NOPKCHANGEMESSAGE=TRUE;
	shout_stuff(ch, argument, 0, 0);
	NOPKCHANGEMESSAGE=FALSE;
	global_color=0;
    }
}


void do_tell(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict=NULL;
    struct char_data *v=NULL;
    char name[100], message[MAX_STRING_LENGTH],
 	buf[MAX_STRING_LENGTH];
   


    if (IS_SET(ORIGINAL(ch)->specials.act, PLR_NOTELL))
    {
	send_to_char("Your message didn't get through!!\n\r", ch);
	return;
    }
    if(GET_MOVE(ch)<10){
	send_to_char("You are too tired to use tell.\n\r",ch);
	return;
    }
	
    half_chop(argument,name,message);

    if(!*name || !*message)
	send_to_char("Who do you wish to tell what??\n\r", ch);
    else if (!(vict = get_char_vis(ch, name)))
	send_to_char("Nobody by that name here.\n\r", ch);
    else if ( ((GET_POS(vict) == POSITION_SLEEPING) && (GET_LEVEL(ch) < 31))
			|| ( !IS_NPC(vict) && (!vict->desc) )
	    	|| IS_SET(ORIGINAL(ch)->specials.act, PLR_NOTELL)
			)
		{
		if(vict->desc)
		act("$E can't hear you.",FALSE,ch,0,vict,TO_CHAR);
		else
		act("$E lost link.",FALSE,ch,0,vict,TO_CHAR);
		}
    else if(IS_SET(vict->specials.plr_flags, PLR_WRITING) && GET_LEVEL(ch) < 35 )
	act("$E is editing right now, try again later.", FALSE, ch, 0, vict, TO_CHAR);
    else
    {
	
	if((vict->desc && vict->desc->connected != CON_SOCIAL_ZONE) && (GET_LEVEL(vict)<32) && (GET_LEVEL(ch)<32) ) {
		send_to_char("They are not in MedLink!\r\n",ch);
		return;
	}

	if(ch->desc && ch->desc->connected != CON_SOCIAL_ZONE && (GET_LEVEL(ch)<32) && (GET_LEVEL(vict)<32) ) {
		send_to_char("You are not in MedLink!\r\n",ch);
		return;
	}

	global_color=31;
	sprintf(buf,"%s tells you, '%s'.\n\r",
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
	send_to_char(buf, vict);
	sprintf(buf,"You tell %s, '%s'.\n\r",
	    (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)), message);
	send_to_char(buf, ch);
	if(strcmp("Vryce",GET_NAME(ch))&&strcmp("Vryce",GET_NAME(vict)))
	if((v=get_char_room("Vryce",ch->in_room))
	   ||(v=get_char_room("Vryce",vict->in_room))){
	     sprintf(buf,"-=>%s tells %s, ",
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
		(IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)));
	     send_to_char(buf,v);
	     global_color=33;
	     sprintf(buf,"'%s'.\n\r",message);
	     send_to_char(buf,v);
	     global_color=31;
        }

	if(GET_LEVEL(ch) < 35)
		GET_MOVE(ch)-=10;
	if(vict->specials.afk){
	     act("$N has AFK on, $E may not see your tell.",TRUE, ch, 0,vict, TO_CHAR);
	     if(vict->specials.afk_text[0]){
		send_to_char("Message: ",ch);
	 	send_to_char(vict->specials.afk_text,ch);
		send_to_char("\n\r",ch);
	     }
	}
	global_color=0;
    }
}



void do_whisper(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict=NULL;
    char name[100], message[MAX_STRING_LENGTH],
	buf[MAX_STRING_LENGTH];

    half_chop(argument,name,message);

    if(!*name || !*message)
	send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
    else if (!(vict = get_char_room_vis(ch, name)))
	send_to_char("No-one by that name here..\n\r", ch);
    else if (vict == ch) {
	global_color=34;
	act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
	send_to_char(
	    "You can't seem to get your mouth close enough to your ear...\n\r",
	     ch);
	global_color=0;
    }else if (IS_SET(ORIGINAL(vict)->specials.plr_flags, PLR_WRITING))
	act("$E is editing right now, try again later.",FALSE,ch,0,vict,TO_CHAR);
    else
    {
	global_color=34;
	sprintf(buf,"$n whispers to you, '%s'.",message);
	act(buf, FALSE, ch, 0, vict, TO_VICT);
	sprintf(buf,"You whisper to $N, '%s'.",message);
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
	act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
	global_color=0;
    }
}


void do_ask(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict=NULL;
    char name[100], message[MAX_STRING_LENGTH],
	buf[MAX_STRING_LENGTH];

    half_chop(argument,name,message);

    if(!*name || !*message)
	send_to_char("Who do you want to ask something, and what??\n\r", ch);
    else if (!(vict = get_char_room_vis(ch, name)))
	send_to_char("No-one by that name here.\n\r", ch);
    else if (vict == ch)
    {
	act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
	send_to_char("You think about it for a while...\n\r", ch);
    }
    else if(IS_SET(ORIGINAL(vict)->specials.plr_flags, PLR_WRITING))
	act("$E is editing right now, try again later.",FALSE,ch,0,0,TO_CHAR);
    else
    {
	global_color=34;
	sprintf(buf,"$n asks you, '%s'.",message);
	act(buf, FALSE, ch, 0, vict, TO_VICT);
	sprintf(buf,"You ask $N, '%s'.",message);
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
	act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
	global_color=0;
    }
}



#define MAX_NOTE_LENGTH 8000      /* arbitrary */

void do_write(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *paper = NULL, *pen = NULL;
    char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
	buf[MAX_STRING_LENGTH];

    argument_interpreter(argument, papername, penname);

    if (!ch->desc)
	return;

    if (!*papername)  /* nothing was delivered */
    {   
	send_to_char(
	    "Write? on what? with what?\n\r", ch);
	return;
    }
    if (*penname) /* there were two arguments */
    {
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
	{
	    sprintf(buf, "You have no %s.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
	{
	    sprintf(buf, "You have no %s.\n\r", penname);
	    send_to_char(buf, ch);
	    return;
	}
    }
    else  /* there was one arg.let's see what we can find */
    {           
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
	{
	    sprintf(buf, "There is no %s in your inventory.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (paper->obj_flags.type_flag == ITEM_PEN)  /* oops, a pen.. */
	{
	    pen = paper;
	    paper = 0;
	}
	else if (paper->obj_flags.type_flag != ITEM_NOTE)
	{
	    send_to_char("That thing has nothing to do with writing.\n\r", ch);
	    return;
	}

	/* one object was found. Now for the other one. */
	if (!ch->equipment[HOLD])
	{
	    sprintf(buf, "You can't write with a %s alone.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (!CAN_SEE_OBJ(ch, ch->equipment[HOLD]))
	{
	    send_to_char("The stuff in your hand is invisible! Yeech!!\n\r", ch);
	    return;
	}
	
	if (pen)
	    paper = ch->equipment[HOLD];
	else
	    pen = ch->equipment[HOLD];
    }
	    
    /* ok.. now let's see what kind of stuff we've found */
    if (pen->obj_flags.type_flag != ITEM_PEN)
    {
	act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
    }
    else if(paper->item_number==1973){
	act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
    }
    else if (paper->obj_flags.type_flag != ITEM_NOTE)
    {
	act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
/*    else if (strlen(paper->action_description)>1){
        send_to_char("There's something written on it already.\n\r", ch);
*/
    }else
    {
	/* we can write - hooray! */

	act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
	if(!paper->action_description){
	    paper->action_description=str_dup("BlAnK");
	}else if(strlen(paper->action_description)<1){
	    if(objs[paper->item_number]->action_description!=paper->action_description)
	        paper->action_description=my_free(paper->action_description);
	    paper->action_description=str_dup("BlAnK");
	}
	if(objs[paper->item_number]->action_description==paper->action_description){
	    paper->action_description=str_dup("new");
	    strcpy(paper->action_description,"");
	}
	ch->desc->str = &paper->action_description;
	ch->desc->max_str = MAX_NOTE_LENGTH;
	strcpy(ch->desc->editing,"Piece of paper");
	sprintf(log_buf,"%s editing a paper",GET_NAME(ch));
	log_hd(log_buf);
    }
}

void shout_stuff(struct char_data *ch, char *arg, int distance, int room){
	/* 	Get a list of the rooms that this room exits to
		then do that for each of those room.  once we are 
		two levels deep, start sending to the rooms
		the shouted text */

	char shout_buf[MAX_INPUT_LENGTH+100];
	char other_buf[MAX_INPUT_LENGTH+100];
	char corrupt_buf[MAX_INPUT_LENGTH+100];
	char corrupt_name_buf[MAX_INPUT_LENGTH+100];
	int door = 0, iPercent=0, iDist;
	struct char_data *i;
	struct descriptor_data *p;
	int iHoloChDistance(struct char_data *ch, struct char_data *vict);
	shout_buf[0]='\0';
	other_buf[0]='\0';


    distance++;

	if(world[ch->in_room]->holox || world[ch->in_room]->holoy) {
		global_color=35;
		for(p = descriptor_list;p;p=p->next) {
			if(p->connected != CON_PLAYING &&
			   p->connected != CON_SOCIAL_ZONE &&
			   p->connected != CON_UNDEAD &&
			   p->connected != CON_HOVERING)
				continue;
			if(p->character == ch) continue;
		  	iDist =iHoloChDistance(ch, p->character);
			if( iDist < 300 ) {
				iPercent = iDist / 5.5;
				CorruptString(corrupt_buf,arg,iPercent);
				CorruptString(corrupt_name_buf,GET_NAME(ch),iPercent);
				sprintf(shout_buf,"You hear %s shout, '%s'.\r\n",corrupt_name_buf,corrupt_buf);
				send_to_char(shout_buf,p->character);
			}
		}
		sprintf(other_buf, "You shout, '%s'.\n\r", arg);
		send_to_char(other_buf,ch);
		global_color=0;
		return;
	}
							

	/* First shout to this room */
	global_color=35;
	sprintf(other_buf, "You shout, '%s'.\n\r", arg);
	sprintf(shout_buf, "You hear someone shout, '%s'.\n\r", arg);
	send_to_char(other_buf, ch);
    for (i = world[ch->in_room]->people; i; i = i->next_in_room)
       if ((i->desc)
	   &&(i->desc!=ch->desc)
	   &&(!IS_SET(ORIGINAL(i)->specials.plr_flags, PLR_WRITING)))
            send_to_char(shout_buf, i);
	global_color=0;
	
    for(door=0;door<=5;door++){
       if(EXIT(ch, door)){
           if((EXIT(ch, door)->to_room != NOWHERE)
              && (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))){
				  shout_stuff_one(ch,shout_buf,distance,EXIT(ch,door)->to_room);
           }
       }
   }
}

void shout_stuff_one(struct char_data *ch, char *arg, int distance, int room){
	int door;
	struct char_data *i;
	int char_room_orig;

	distance++;
	/* Shout in this room first */
	global_color=35;
    for (i = world[room]->people; i; i = i->next_in_room)
       if ((i->desc)
	   &&!IS_SET(ORIGINAL(i)->specials.plr_flags, PLR_WRITING))
            send_to_char(arg, i);
	global_color=0;

	char_room_orig=ch->in_room;

	if(distance>2)
		return;
 
	char_from_room(ch);
	char_to_room(ch, room);

	for(door=0;door<=5;door++){
		if(EXIT(ch, door)){
			if((EXIT(ch,door)->to_room != NOWHERE)
				&& (!IS_SET(EXIT(ch,door)->exit_info, EX_CLOSED))
				&& (!(EXIT(ch,door)->to_room == char_room_orig))){
				shout_stuff_one(ch,arg,distance,EXIT(ch,door)->to_room);
			}
		}
	}
	char_from_room(ch);
	char_to_room(ch, char_room_orig);
}

char *set_vector_text(int vector, int door){
	char vector_text[80];
	switch(door){
		case NORTH:
			switch(vector){
				case NORTH:
					strcpy(vector_text,"the south");
					break;
				case EAST:
					strcpy(vector_text, "the southheast");
					break;
				case WEST:
					strcpy(vector_text, "the southwest");
					break;
				case UP:
					strcpy(vector_text, "below you and to the south");
					break;
				case DOWN:
					strcpy(vector_text, "above you and to the south");
					break;
				default:
					strcpy(vector_text, "this room");
			}
			break;
		case EAST:
			switch(vector){
				case NORTH:
					strcpy(vector_text, "the southwest");
                    break;
				case EAST:
					strcpy(vector_text, "the west");
					break;
				case SOUTH:
					strcpy(vector_text, "the northwest");
					break;
				case UP:
					strcpy(vector_text, "below you and to the west");
					break;
				case DOWN:
					strcpy(vector_text, "above you and to the west");
					break;
				default:
					strcpy(vector_text, "this room");
					break;
			}
			break;
		case SOUTH:
            switch(vector){
                case EAST:
                    strcpy(vector_text, "the northwest");
                    break;
                case SOUTH:
                    strcpy(vector_text, "the north");
                    break;
                case WEST:
                    strcpy(vector_text, "the northeast");
                    break;
                case UP:
                    strcpy(vector_text, "below you and to the north");
                    break;
                case DOWN:
                    strcpy(vector_text, "above you and to the north");
                    break;
                default:
                    strcpy(vector_text, "this room");
                    break;
			}
			break;
		case WEST:
            switch(vector){
                case NORTH:
                    strcpy(vector_text, "the southeast");
                    break;
                case SOUTH:
                    strcpy(vector_text, "the northeast");
                    break;
                case WEST:
                    strcpy(vector_text, "the east");
                    break;
                case UP:
                    strcpy(vector_text, "below you and to the east");
                    break;
                case DOWN:
                    strcpy(vector_text, "above you and to the west");
                    break;
                default:
                    strcpy(vector_text, "this room");
                    break;
			}
			break;
		case UP:
            switch(vector){
                case NORTH:
                    strcpy(vector_text, "below you and to the south");
                    break;
                case EAST:
                    strcpy(vector_text, "below you and to the west");
                    break;
                case SOUTH:
                    strcpy(vector_text, "below you and to the south");
                    break;
                case WEST:
                    strcpy(vector_text, "below you and to the east");
                    break;
                case DOWN:
                    strcpy(vector_text, "below you");
                    break;
                default:
                    strcpy(vector_text, "this room");
                    break;
			}
			break;
		case DOWN:
            switch(vector){
                case NORTH:
                    strcpy(vector_text, "above you and to the south");
                    break;
                case EAST:
                    strcpy(vector_text, "above you and to the west");
                    break;
                case SOUTH:
                    strcpy(vector_text, "above you and to the south");
                    break;
                case WEST:
                    strcpy(vector_text, "above you and to the east");
                    break;
                case DOWN:
                    strcpy(vector_text, "below you");
                    break;
                default:
                    strcpy(vector_text, "this room");
                    break;
			}
			break;
		default:
			strcpy(vector_text, "some unknown area");
			break;
		}
	return(vector_text);
}
