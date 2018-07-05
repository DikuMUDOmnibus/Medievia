/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "structs.h"
#include <unistd.h>
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

#define STATE(d) ((d)->connected)

/* extern variables */
extern char global_color;
extern struct str_app_type str_app[];
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct descriptor_data *descriptor_list;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern struct char_data *character_list;

/* extern procedures */

void do_shout(struct char_data *ch, char *argument, int cmd);
bool in_a_shop(struct char_data *ch);
void stop_follower(struct char_data *ch);
void tell_clan(int clan, char *argument);
extern int file_to_string(char *name, char *buf);
void locker_stuff_for_player(struct char_data *ch);
extern void prepare_for_quit(struct char_data *, bool);
extern void SaveAndExtractFreight(struct char_data *stpCh);

void do_editnotes(struct char_data *ch, char *argument, int cmd)
{
char buf[MAX_STRING_LENGTH];
char filename[MAX_INPUT_LENGTH];

    sprintf(filename,"../notes/%s",GET_NAME(ch));
    file_to_string(filename,buf);
    ch->specials.tmpstring=str_dup(buf);
    ch->desc->str=&ch->specials.tmpstring;
    ch->desc->max_str=8192;
    strcpy(ch->desc->editing,"Notes file");
    ch->desc->oneline=2;
}
void do_qui(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("You have to write quit - no less, to quit!\n\r",ch);
    return;
}

void do_saveout(struct char_data *ch, char *argument, int cmd)
{
int iWear;
struct char_data *victim=NULL, *corpse=NULL;
struct descriptor_data *victim_desc=NULL;
unsigned int paymenow=0,takefrombank=0,balance=0;
struct obj_data *obj=NULL,*bottle=NULL;
int crypt=0;
FILE *fh=NULL;
char filename[250],name[250];
char arg[MAX_INPUT_LENGTH*2];

	if ( IS_NPC(ch) && ((ch->nr != 9800) || (!ch->specials.death_timer)))
		return;

	one_argument(argument,arg);
	if(!arg[0])
	{
		send_to_char("Saveout who?\n\r",ch);
		return;
    }

	if (!(victim = get_char_vis(ch, arg)))
	{
		send_to_char("No-one by that name around.\n\r", ch); 
		return; 
    }

/* juice
 * remove victim from void so autorenting isn't a free ride to castle
 * square and so people can't get free ride back from trellor to medievia.
 */
	if (victim->in_room == 0)
	{
		char_from_room(victim);
		char_to_room(victim, victim->specials.was_in_room);
	}

	if (IS_NPC(victim)&&((victim->nr != 9800) || (!victim->specials.death_timer)))
	{
		send_to_char("Your victim is a mob!\n\r",ch);
		return;
	}

	global_color=31;
	send_to_char("You have been SAFELY logged out by a GOD!\n\r",victim);
	sprintf(log_buf,"%s AUTORENTED by the Magic Code", GET_NAME(victim));
	do_wiz(ch, log_buf, 5);     
	send_to_char("Player has been safely logged out.\n\r",ch);
	global_color=0;

	if (victim->nr == 9800)
	{
	/* remove spirit bottle from crypt */
/*		if (world[1729])
			crypt=1729;
		bottle = get_obj_in_list( GET_NAME(victim), world[crypt]->contents );
		if (bottle)
		{  
			if(bottle->contains)
				extract_obj(bottle->contains);
			extract_obj(bottle);
		}
*/
/* extract the corpse mob and put pc back into character_list */
	corpse=victim;
		if(IS_UNDEAD(ch))
		{
			victim=victim->desc->original;  
			victim->next = character_list;
			character_list = victim;
			char_to_room(victim, victim->in_room);
			STATE(corpse->desc) = CON_PLAYING;
			corpse->desc->character = corpse->desc->original;
			corpse->desc->original = 0;
			corpse->desc->character->desc = corpse->desc;
		}
		else
			victim=NULL;
		for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
			if ( corpse->equipment[iWear] )
				obj_to_char( unequip_char( corpse, iWear ), corpse ); 
		while ( corpse->carrying )
			extract_obj( corpse->carrying ); 
		corpse->specials.home_number=20002;
		extract_char(corpse, TRUE ); 
	}

	if(!victim)
		return;
    SaveAndExtractFreight(victim);
	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
		if ( victim->equipment[iWear] )
			obj_to_char( unequip_char( victim, iWear ), victim ); 

/*	locker_stuff_for_player(victim); */

	obj=victim->carrying;
while ( obj )
	{
	paymenow+=obj->obj_flags.cost_per_day;
	obj=obj->next_content;
    }
/*
if(cmd!=99&&!victim->specials.home_number)
	{
 	if(GET_GOLD(victim)>=paymenow)
		{
		GET_GOLD(victim)-=paymenow;
		}
	else
		{
   		if(GET_GOLD(victim)>0)
			{
			takefrombank=paymenow-GET_GOLD(victim);
			GET_GOLD(victim)=0;
   			}
		else
			{
			takefrombank=paymenow;
   			}	       
		strcpy(name,GET_NAME(victim));
		sprintf(filename,"../save/%c/%s.atm",LOWER(name[0]),name);
		if((fh=med_open(filename, "r"))!=NULL)
			{
			open_files++ ;
			fread(&balance,sizeof(unsigned int), 1, fh); 
			med_close(fh);
			open_files-- ;
			if(balance>takefrombank)
	    		balance-=takefrombank;
			else
				balance=0;
           	if((fh=med_open(filename, "w"))!=NULL)
				{
				open_files++ ;
				fwrite(&balance, sizeof(unsigned int),1,fh); 
				med_close(fh); 
				open_files-- ;
				}
			if(balance<=0)
				unlink(filename); 
			}
		}
	obj=read_object(1976,0); 
	obj_to_char(obj,victim);
	}
*/
save_char_obj(victim);
while ( victim->carrying )
	{
    extract_obj( victim->carrying ); 
	}
victim_desc = victim->desc;
victim->specials.home_number=20003;
/*extract_char( victim, TRUE ); */
prepare_for_quit(victim, TRUE);
if ( victim_desc )
	close_socket( victim_desc );
free_char(victim);
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
struct obj_data *bottle;
int crypt=0;
int iWear;
char arg[MAX_INPUT_LENGTH];
struct room_affect *stpR=NULL;
struct char_data *stpMount=NULL;

    if ( IS_NPC(ch) && !IS_UNDEAD(ch) )
		return;

	if(IN_HOLO(ch)){
		for(stpR=world[ch->in_room]->room_afs;stpR;stpR=stpR->next){
			if(stpR->type==RA_CAMP)
				break;
		}
	}		
    if(cmd!=1111&&ch->specials.cost_to_rent!=-2){
        global_color=36;
		if(!IS_NPC(ch)){
			if(stpR){
				send_to_char("Ok you can quit here in the camp area!\n\r",ch);						
    		}else if((ch->specials.home_number)&&(world[ch->in_room]->number==ch->specials.home_number)&&(ch->specials.home_number))
	    		send_to_char("OK you are in your HOME, QUITTING IS SAFE!\n\r",ch);
			else if(GET_LEVEL(ch)>=32)
			    send_to_char("OK GOD, come back soon MEDIEVIA NEEDS YOU!\n\r",ch);
			else {
        	    global_color=31;
			    send_to_char("WE HAVE RENT HERE, YOU WILL LOSE ALL YOUR STUFF!\n\r",ch);
				global_color=0;
        	}
			ch->p->queryfunc=do_quit;
			global_color=1;
			strcpy(ch->p->queryprompt, "Are you sure you want to quit? (y/n)>");
			global_color=0;
			ch->p->querycommand=1111;
			return;
	    }
	}
    if(cmd==1111){
		one_argument(argument, arg);
		if(arg[0]!='y'&&arg[0]!='Y'){
		    ch->p->querycommand=0;
		    return;
		}
    }
    ch->p->querycommand=0;

    if ( GET_POS(ch) == POSITION_FIGHTING )
    {
		send_to_char( "No way! You are fighting.\n\r", ch );
		return;
    }
    if ( GET_POS(ch) < POSITION_STUNNED )
    {
		send_to_char( "You're not DEAD yet.\n\r", ch );
		return;
    }
	SaveAndExtractFreight(ch);
    if(ch->specials.cost_to_rent!=-2){
        if((GET_LEVEL(ch)<32)&&(world[ch->in_room]->number!=ch->specials.home_number)&&(!stpR))
	        ch->specials.cost_to_rent=-1;     /* so all inv is wiped out! */

        if(IS_UNDEAD(ch))
		    ch->specials.cost_to_rent=-20; /* corpses get to rent for free */
		if(stpR)
			ch->specials.cost_to_rent=-20;
		if(ch->specials.stpMount&&ch->in_room==ch->specials.stpMount->in_room){
			stpMount=ch->specials.stpMount;
		}
        SAVE_CHAR_OBJ(ch,ch->specials.cost_to_rent);
    }

    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
        if ( ch->equipment[iWear] )
            obj_to_char( unequip_char( ch, iWear ), ch ); 
    while ( ch->carrying )
        extract_obj( ch->carrying ); 
    if(ch->specials.clan){
       sprintf(log_buf,"[CLAN] %s has LEFT the game.",GET_NAME(ch));
       tell_clan(ch->specials.clan,log_buf);
    }
    sprintf(log_buf," %s has left the game",GET_NAME(ch));
    if(strcmp(GET_NAME(ch),"Starblade"))
    do_wiz(ch,log_buf,5);

global_color=34;
send_to_char("
\r                *_   _   _   _   _   _   _   _   _   _  *
\r        ^       | `_' `-' `_' `-' `_' `-' `_' `-' `_' `-|       ^
\r        |       |   ",ch);
global_color=32;
send_to_char("Farewell Friend!  Come back soon!",ch);
global_color=34;
send_to_char("   |       |
\r        |  (*)  |_   _   _   _   _   _   _   _   _   _  |  \\^/  |
\r        | _<%>_ | `_' `-' `_' `-' `_' `-' `_' `-' `_' `-| _(#)_ |
\r       o+o \\ / \\0                                       0/ \\ / (=)
\r        0'\\ ^ /\\/            ",ch);
global_color=31;
send_to_char("MEDIEVIA IV",ch);
global_color=34;
send_to_char("                \\/\\ ^ /`0
\r          /_^_\\ |          medievia.com 4000            | /_^_\\ 
\r          || || |                                       | || ||
\r__________d|_|b_T_______________________________________T_d|_|b__________

",ch);
global_color=0;
    act( "$n has left the game.", TRUE, ch, 0, 0, TO_ROOM );

    if(ch->desc->original)
	{
	while ((ch->desc->original) && (ch->desc->original->carrying))
		extract_obj( ch->desc->original->carrying );
/*	free_char(ch->desc->original);*/
	}

    /* remove spirit bottle from crypt */
    if (world[1729])
        crypt=1729;
    bottle = get_obj_in_list( GET_NAME(ch), world[crypt]->contents );
    if (bottle)
        {  
        if(bottle->contains)
                extract_obj(bottle->contains);
        extract_obj(bottle);
        }
    ch->specials.home_number=20004;
    /*extract_char( ch, TRUE );*/ 
    prepare_for_quit(ch, TRUE);
    
    /*
     * Can't jam back to menu here because all EQ is gone.
     */
    if ( ch->desc )
	close_socket( ch->desc );
    free_char(ch);
	if(stpMount)
		extract_char(stpMount,TRUE);
}



void do_save(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("No SAVING, you must RENT at hotel.\n\r",ch);
    send_to_char("The [savedisplay command] toggles displaying of autosaves.\n\r",ch); }



void do_not_here(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("Sorry, but you cannot do that here!\n\r",ch);
}


void do_track(struct char_data *stpCh, char *szpArgument, int iCmd)
{
	char szName[MAX_INPUT_LENGTH];
	struct char_data *stpVict=NULL;
	
	
	one_argument(szpArgument,szName);
	
	if(!szName || !*szName) {
		send_to_char("You must enter a name to track, use track off to stop tracking.\r\n",stpCh);
		return;
	}
	if(!str_cmp(szName,"off")) {
		send_to_char("You stop tracking..\r\n",stpCh);
		stpCh->specials.hunting = NULL;
		return;
	}
	
	if(GET_CLASS(stpCh) != CLASS_THIEF &&
	   !IS_SET(stpCh->player.multi_class,MULTI_CLASS_THIEF)){	
	   		send_to_char("Your attempts at tracking fail horribly.\r\n",stpCh);
	   		return;
	}
	stpVict = get_char_vis(stpCh,szName);
	
	if(!stpVict) {
		send_to_char("There is no one matching that name.\r\n",stpCh);
		return;
	}
	
	if(IS_NPC(stpVict)) {
		send_to_char("You can't track mobiles..\r\n",stpCh);
		return;
	}
				
	stpCh->specials.hunting = stpVict;
	
	sprintf(log_buf, "You start tracking %s.\r\n",GET_NAME(stpVict));
	send_to_char(log_buf,stpCh);
	return;	

}
void do_sneak(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    byte percent;

    if ( IS_UNDEAD(ch) )
	{
	send_to_char("How can you sneak rattling all those chains?",ch);
	return;
	}

	if(GET_CLASS(ch)!=CLASS_THIEF)
		if(!IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF)){
			send_to_char("Silly mudder, sneaking is for thieves!\n\r",ch);
			return;
		}
	if(!IS_NPC(ch)&&ch->specials.stpMount){
		send_to_char("You wish you could sneak while mounted...\n\r",ch);
		return;
	}
    if (IS_AFFECTED(ch, AFF_SNEAK)){
	affect_from_char(ch, SKILL_SNEAK);
        send_to_char("You stop sneaking.\n\r", ch);
	return;
    }
    else{
      send_to_char("You attempt to move silently.\n\r", ch);
    }
    if(world[ch->in_room]->zone==198)return;
    percent=number(1,101); /* 101% is a complete failure */

    if (percent > ch->skills[SKILL_SNEAK].learned +
	dex_app_skill[GET_DEX(ch)].sneak)
	return;

    af.type = SKILL_SNEAK;
    af.duration = GET_LEVEL(ch);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SNEAK;
    affect_to_char(ch, &af);
}



void do_hide(struct char_data *ch, char *argument, int cmd)
{
    byte percent;

    send_to_char("You attempt to hide yourself.\n\r", ch);

    if (IS_AFFECTED(ch, AFF_HIDE))
	REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

    percent=number(1,101); /* 101% is a complete failure */

    if (percent > ch->skills[SKILL_HIDE].learned +
	dex_app_skill[GET_DEX(ch)].hide)
	return;

    SET_BIT(ch->specials.affected_by, AFF_HIDE);
}


void do_steal(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim=NULL;
    struct obj_data *obj=NULL;
    char victim_name[240];
    char obj_name[240];
    char buf[240];
    int percent;
    int gold, eq_pos;
    bool ohoh = FALSE;

	if(IS_DEAD(ch))
	{
	send_to_char("You're too weak to carry another thing!",ch);
	return;
	}

    argument = one_argument(argument, obj_name);
    one_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name))) {
	send_to_char("Steal what from who?\n\r", ch);
	return;
    } else if (victim == ch) {
	send_to_char("You practice stealing something from yourself!\n\r", ch);
	return;
    }

/* juice
 * allow pc's to be stole from in neutral/chaotic zones if in position
 * dead.  Use this instead of IS_HOVERING so people dont' drop link to
 * avoid being corpse being looted.
 */
    if ( !IS_NPC(victim) && GET_POS(victim) == POSITION_DEAD &&
	 !IS_SET(world[victim->in_room]->room_flags, CHAOTIC)) {
		send_to_char("Looting is only permitted in Chaotic zones.\r\n",ch);
		return;
    }

    if ( !IS_NPC(victim) && GET_POS(victim) != POSITION_DEAD) {
      send_to_char("Due to misuse, you can't steal from other living players.\n\r", ch);
      return;
    }

    if (IS_SET(world[ch->in_room]->room_flags,SAFE)){
      send_to_char("No stealing permitted in safe areas!\n\r",ch);
      return;
    }
 
    if(!IS_NPC(victim))
	WAIT_STATE(ch, 100); /* 25 seconds */  
    else
	WAIT_STATE(ch, 10); /* It takes TIME to steal */

    /* 101% is a complete failure */
    percent=number(1,101) - dex_app_skill[GET_DEX(ch)].p_pocket;

   percent += AWAKE(victim) ? 10 : -50;

    if (GET_POS(victim) < POSITION_SLEEPING)
	percent = -1; /* ALWAYS SUCCESS */

    if (GET_LEVEL(victim)>31) /* NO NO With Imp's and Shopkeepers! */
	percent = 101; /* Failure */

    if (str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {

      if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
	
	
	for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
	  if (victim->equipment[eq_pos] &&
	  (isname(obj_name, victim->equipment[eq_pos]->name)) &&
	  CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
	obj = victim->equipment[eq_pos];
	break;
	  }
	
	if (!obj) {
	  act("$E has not got that item.",FALSE,ch,0,victim,TO_CHAR);
	  return;
	} else { /* It is equipment */
	  if(obj->item_number==10||obj->item_number==16){
		send_to_char("You would steal an object given to this soul by GOD???\n\r",ch);
		return;
	  }
	  /* added by raster so players cant steal invis from mobs */
	  if(IS_NPC(victim) && IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
	  	send_to_char("You try to steal the object but fail.\r\n",ch);
	  	return;
	  }
	  
	  if ((GET_POS(victim) > POSITION_DEAD)) {
	send_to_char("Steal the equipment now? Impossible!\n\r", ch);
	return;
	  } else {
	act("You unequip $p and steal it.",FALSE, ch, obj ,0, TO_CHAR);
	act("$n steals $p from $N.",FALSE,ch,obj,victim,TO_NOTVICT);
	act("$n steals $p from you.",FALSE,ch,obj,victim,TO_VICT);
	obj_to_char(unequip_char(victim, eq_pos), ch);
	  }
	}
      } else {  /* obj found in inventory */
	
	percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */
	
	if (percent > ch->skills[SKILL_STEAL].learned) {
	  ohoh = TRUE;
	  act("Oops..", FALSE, ch,0,0,TO_CHAR);
	  act("$n tried to steal something from you!",
		FALSE,ch,0,victim,TO_VICT);
	  act("$n tries to steal something from $N.",
		TRUE, ch, 0, victim, TO_NOTVICT);
	} else { /* Steal the item */
	  if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	  if(obj->item_number==10||obj->item_number==16){
		send_to_char("You would steal an object given to this soul by GOD???\n\r",ch);
		return;
	  }
	  if(obj->item_number==211){
		send_to_char("You would break the egg in the attempt\n\r",ch);
		return;
	  }
	  if(IS_NPC(victim) && IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
	  	send_to_char("You try to steal the object but fail.\r\n",ch);
	  	return;
	  }
	  obj_from_char(obj);
	  obj_to_char(obj, ch);
	  send_to_char("Got it!\n\r", ch);
	if (IS_HOVERING(victim))
	  {
	  sprintf(log_buf,"$n takes the %s from you.", obj->short_description);
          act(log_buf, FALSE,ch,0,victim,TO_VICT);
          sprintf(log_buf,"$n takes the %s from $N.", obj->short_description);
          act(log_buf, TRUE, ch, 0, victim, TO_NOTVICT);
          }
	}
	  } else
	send_to_char("You cannot carry that much.\n\r", ch);
	}
      }
    } else { /* Steal some coins */
      if (percent > ch->skills[SKILL_STEAL].learned) {
	ohoh = TRUE;
	act("Oops..", FALSE, ch,0,0,TO_CHAR);
	act("You discover that $n has $s hands in your wallet.",
	    FALSE,ch,0,victim,TO_VICT);
	act("$n tries to steal gold from $N.",
	    TRUE, ch, 0, victim, TO_NOTVICT);
      } else {
	/* Steal some gold coins */
	gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
	gold = MIN(1782, gold);
	if (gold > 0) {
	  GET_GOLD(ch) += gold;
	  GET_GOLD(victim) -= gold;
	  sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
	  send_to_char(buf, ch);
	} else {
	  send_to_char("You couldn't get any gold...\n\r", ch);
	}
      }
    }

    if (ohoh && IS_NPC(victim) && AWAKE(victim) && GET_LEVEL(ch)<32){
      if (IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
	sprintf(log_buf,"%s Don't you *EVER* do that again!",GET_NAME(ch));
	do_talk(victim, log_buf, 0);
      } else {
	hit(victim, ch, TYPE_UNDEFINED);
      }
    } 
if (!IS_NPC(victim))
	SAVE_CHAR_OBJ(victim, -20);
}



void do_practice(struct char_data *ch, char *arg, int cmd)
{
  /* Call "guild" with a null string for an argument.
     This displays the character's skills. */

    if (arg[0] != '\0'){
        send_to_char("You can only practice in a guild.\n\r", ch);
    }else{
        (void) guild (ch, cmd, "");
    }
}


void do_idea(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
	return;
    }

    /* skip whites */
    for (; isspace(*argument); argument++);

    if (!*argument)
    {
	send_to_char("That doesn't sound like a good idea to me.  Sorry.\n\r",
	    ch);
	return;
    }

    if (!(fl = med_open(IDEA_FILE, "a")))
    {
	perror ("do_idea");
	send_to_char("Could not open the idea-file.\n\r", ch);
	return;
    }

    open_files++;
    sprintf(str, "**%s[%d]: %s\n",
    GET_NAME(ch), world[ch->in_room]->number, argument);
    fputs(str, fl);
    med_close(fl);
    open_files--;
    send_to_char("Ok.  Thanks.\n\r", ch);
}


void do_note(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_STRING_LENGTH];
    char note_file[MAX_STRING_LENGTH];
 
    if (IS_NPC(ch))
    {
        send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
        return;
    }

    sprintf(note_file,"../notes/%s",GET_NAME(ch));

    /* skip whites */
    for (; isspace(*argument); argument++);
 
    if (!*argument)
    {
        send_to_char("Pardon?  SYNTAX > note blah blah blah ...\n\r",ch);
        return;
    }
 
    if (!(fl = med_open(note_file, "a")))
    {
        perror ("do_note");
	sprintf(log_buf,"Sorry, can't open the file %s.\n\r", note_file);
        send_to_char(log_buf, ch);
        return;
    }

    open_files++; 
    sprintf(str, "**%s[%d]: %s\n",
    GET_NAME(ch), world[ch->in_room]->number, argument);
    fputs(str, fl);
    med_close(fl);
    open_files--;
    send_to_char("Message is saved in your note file.\n\r", ch);
    act("$n jots down a note and the game saves it to $s file.",FALSE,ch,0,0,TO_ROOM);
}





void do_typo(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
	return;
    }

    /* skip whites */
    for (; isspace(*argument); argument++);

    if (!*argument)
    {
	send_to_char("Pardon?  SYNTAX > typo blah blah blah ...\n\r",
	    ch);
	return;
    }

    if (!(fl = med_open(TYPO_FILE, "a")))
    {
	perror ("do_typo");
	send_to_char("Could not open the typo-file.\n\r", ch);
	return; 
    }
    open_files++;
    sprintf(str, "**%s[%d]: %s\n",GET_NAME(ch), world[ch->in_room]->number, argument);
    fputs(str, fl);
    med_close(fl);
     open_files--;
    send_to_char("Ok.  Thanks.\n\r", ch);
}





void do_bug(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
	send_to_char("You are a monster! Bug off!\n\r", ch);
	return;
    }

    /* skip whites */
    for (; isspace(*argument); argument++);

    if (!*argument)
    {
	send_to_char("Pardon? Syntax > bug blah blah blah... \n\r",
	    ch);
	return;
    }

    if (!(fl = med_open(BUG_FILE, "a")))
    {
	perror ("do_bug");
	send_to_char("Could not open the bug-file.\n\r", ch);
	return;
    }

    open_files++;
    sprintf(str, "**%s[%d]: %s\n",GET_NAME(ch), world[ch->in_room]->number, argument);
    fputs(str, fl);
    med_close(fl);
    open_files--;
    send_to_char("Ok.\n\r", ch);
}



void do_brief(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch) && !IS_SWITCHED(ch) )
	return;

    if (IS_SET(ORIGINAL(ch)->specials.act, PLR_BRIEF))
    {
	send_to_char("Brief mode off.\n\r", ch);
	REMOVE_BIT(ORIGINAL(ch)->specials.act, PLR_BRIEF);
    }
    else
    {
	send_to_char("Brief mode on.\n\r", ch);
	SET_BIT(ORIGINAL(ch)->specials.act, PLR_BRIEF);
    }
}


void do_compact(struct char_data *ch, char *argument, int cmd)
{
    if (!ch->desc)
	return;

    if (IS_SET(ORIGINAL(ch)->specials.act, PLR_COMPACT))
    {
	send_to_char( "Compact mode off.\n\r", ch);
	REMOVE_BIT(ORIGINAL(ch)->specials.act, PLR_COMPACT);
    }
    else
    {
	send_to_char( "Compact mode on.\n\r", ch);
	SET_BIT(ORIGINAL(ch)->specials.act, PLR_COMPACT);
    }
}

void do_group(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("         Medievia uses Formations, type help formations
         ----------------------------------------------
Formations are everything groups are on other muds, but with the added
fun of formations. (follow, formation, freport, reform, unform, fsay, ftell, ;)\n\r",ch);

}
void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp=NULL;
  int i=0;
  bool equipped=FALSE;

  one_argument(argument,buf);

    if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
	temp = ch->equipment[HOLD];
	equipped = TRUE;
      if ((temp==0) || !isname(buf, temp->name)) {
	    act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
	    return;
      }
    }

  if (temp->obj_flags.type_flag!=ITEM_POTION)
  {
    act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(in_a_shop(ch) || IS_SET(world[ch->in_room]->room_flags, NO_MAGIC)){
	send_to_char("Somehow this room blocks all magic!\n\r",ch);
	return;
  }

  act("$n works the seal off of $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You work the seal off of $p.",FALSE,ch,temp,0,TO_CHAR);
  act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You quaff $p which dissolves.",FALSE,ch,temp,0,TO_CHAR);
  WAIT_STATE(ch, PULSE_VIOLENCE);
  for (i=1; i<4; i++)
    if (temp->obj_flags.value[i] >= 1)
      ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
	((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, 0));

    if (equipped)
	unequip_char(ch, HOLD);

    extract_obj(temp);
}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct obj_data *scroll=NULL, *obj=NULL;
    struct char_data *victim=NULL;
    int i, bits;
    bool equipped;

    equipped = FALSE;
    obj = 0;
    victim = 0;
    if(IS_UNDEAD(ch))return;
    argument = one_argument(argument,buf);

    if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
	scroll = ch->equipment[HOLD];
	equipped = TRUE;
      if ((scroll==0) || !isname(buf, scroll->name)) {
	    act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
	    return;
    }
    }

  if (scroll->obj_flags.type_flag!=ITEM_SCROLL)
  {
    act("Recite is normally used for scrolls.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

    if (*argument) {
      bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
	FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
	if (bits == 0) {
	    send_to_char(
		"No such thing around to recite the scroll on.\n\r", ch);
	    return;
	}
    } else {
	victim = ch;
    }

    act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  act("You recite $p which dissolves.",FALSE,ch,scroll,0,TO_CHAR);

  for (i=1; i<4; i++)
    if (scroll->obj_flags.value[i] >= 1)
      ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
      ((byte) scroll->obj_flags.value[0],
      ch, "", SPELL_TYPE_SCROLL, victim, obj));

    if (equipped)
	unequip_char(ch, HOLD);

    extract_obj(scroll);
}



void do_use(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct char_data *tmp_char=NULL;
  struct obj_data *tmp_object=NULL, *stick=NULL;
  int x;
  int bits;

  argument = one_argument(argument,buf);
  for(x=0;x<MAX_WEAR;x++)
  if (ch->equipment[x] != 0 && isname(buf, ch->equipment[x]->name)) {
       break;
  }
  if(x>=MAX_WEAR){
    act("You are not holding or wearing that item!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  if(in_a_shop(ch) || IS_SET(world[ch->in_room]->room_flags, NO_MAGIC)){
	send_to_char("Somehow this room blocks all magic!\n\r",ch);
	return;
  }
    stick = ch->equipment[x];

  if (stick->obj_flags.type_flag == ITEM_STAFF)
  {
	act("$n taps $p three times on the ground.",
	    TRUE, ch, stick, 0,TO_ROOM);
	act("You tap $p three times on the ground.",
	    FALSE,ch, stick, 0,TO_CHAR);
	act("You see $p glow with magic and slowly quiet down.",
	    TRUE, ch, stick, 0,TO_ROOM);
	act("You see $p glow with magic and slowly quiet down.",
	    FALSE,ch, stick, 0,TO_CHAR);

	if (stick->obj_flags.value[2] > 0) { /* Charges left? */
	    stick->obj_flags.value[2]--;
	    ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
	    ((byte) stick->obj_flags.value[0], ch, "",
	    SPELL_TYPE_STAFF, 0, 0));
	WAIT_STATE(ch, PULSE_VIOLENCE);
	} else {
	    send_to_char("The staff seems powerless.\n\r", ch);
	}
    } else if (stick->obj_flags.type_flag == ITEM_REGEN) {
	act("$n uses $p.",
	    TRUE, ch, stick, 0,TO_ROOM);
	act("You use $p.",
	    FALSE,ch, stick, 0,TO_CHAR);

	if (stick->obj_flags.value[2] > 0) { /* Charges left? */
	    stick->obj_flags.value[2]--;
	    ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
	    ((byte) stick->obj_flags.value[0], ch, "",
	    SPELL_TYPE_POTION, ch, 0));
	} else {
	    send_to_char("It is still regenerating.\n\r", ch);
	}
    } else if (stick->obj_flags.type_flag == ITEM_WAND) {

	bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV
	| FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
	if (bits) {
	    if (bits == FIND_CHAR_ROOM) {
		act("$n uses $p on $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
		act("You use $p on $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
   	        act("You see $p glow with magic and slowly quiet down.",
	          TRUE, ch, stick, tmp_char,TO_ROOM);
	        act("You see $p glow with magic and slowly quiet down.",
	          FALSE,ch, stick, tmp_char,TO_CHAR);
	    } else {
	    act("$n use $p on $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
	    act("You use $p on $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
   	    act("You see $p glow with magic and slowly quiet down.",
	       TRUE, ch, stick, tmp_object,TO_ROOM);
	    act("You see $p glow with magic and slowly quiet down.",
	       FALSE,ch, stick, tmp_object,TO_CHAR);
	    }

	    if (stick->obj_flags.value[2] > 0) {
	    /* Is there any charges left? */
		stick->obj_flags.value[2]--;
		((*spell_info[stick->obj_flags.value[3]].spell_pointer)
	      ((byte) stick->obj_flags.value[0], ch, "",
	      SPELL_TYPE_WAND, tmp_char, tmp_object));
	     WAIT_STATE(ch, PULSE_VIOLENCE);
	    } else {
		send_to_char("The wand seems powerless.\n\r", ch);
	    }

	} else {
	    send_to_char("What should the wand be pointed at?\n\r", ch);
	}
    } else {
	send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
  }
}


void do_wimpy(struct char_data *ch, char *argument, int cmd)
{
int wimpy=0;        
char buf[MAX_INPUT_LENGTH];

    one_argument(argument,buf);
    if(!ch||!argument)return;
    if(!*buf||!isdigit(*buf)){
    sprintf(log_buf,"Your wimpy is currently set at [%d].\n\r",ch->specials.wimpy);
    send_to_char(log_buf,ch);
	send_to_char("SYNTAX> wimpy 20, will set wimpy to 20.\n\r",ch);
	return;
    }
    wimpy=atoi(buf);
    if(wimpy<0)return;
    if(wimpy>255){
    sprintf(log_buf,"Your wimpy is currently set at [%d].\n\r",ch->specials.wimpy);
    send_to_char(log_buf,ch);
	send_to_char("Wimpy must be below 256!\n\r",ch);
	return;
    }
    ch->specials.wimpy=wimpy;
    sprintf(log_buf,"Your wimpy is currently set at [%d].\n\r",ch->specials.wimpy);
    send_to_char(log_buf,ch);
}

