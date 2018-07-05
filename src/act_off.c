/***************************************************************************
*		 			MEDIEVIA CyberSpace Code and Data files		   		   *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*			   				All rights reserved				   			   *
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
#ifndef __FreeBSD__
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/* extern variables */
extern int port;
extern void fall(struct char_data *ch);
extern char global_color;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct EARTHQUAKE *stpEarthquake;
extern bool in_a_shop(struct char_data *ch);
extern bool IS_IN_BACK(struct char_data *ch);
extern bool IS_IN_FRONT(struct char_data *ch);
extern struct char_data *pick_victim(struct char_data *ch);
extern int fflush(FILE *fp);
extern int system(const char *cmdstring);
extern void spell_cure_critic(byte level, struct char_data *ch, struct char_data *victim, struct obj_data *obj);
void raw_kill(struct char_data *ch,struct char_data *attacker);
void check_killer(struct char_data *ch, struct char_data *victim);
bool is_in_safe(struct char_data *ch, struct char_data *victim);
void list_char_to_char(struct char_data *list, struct char_data *ch,int mode);
void remove_from_formation(struct char_data *ch);
struct obj_data *get_first_throwable(struct char_data *);

/* AVI */
int vfork(void);
int close(int);
int dup(int);
int execl();

void nonblock(int);
void block(int);

int avi_system(new_sock, full, short_name, a1, a2, a3, a4)
int	new_sock;
char    *full, *short_name, *a1, *a2, *a3, *a4;
{
        pid_t   pid;


        if ((pid = vfork()) == 0)
	{
		block(new_sock);
		close(0);
		close(1);
		close(2);
		dup(new_sock);
		dup(new_sock);
		dup(new_sock);
		system("IFS=;/bin/stty sane");
printf("You are now a [sane] subprocess!!!\n");
fflush(stdout);
                (void) execl(full, short_name, a1, a2, a3, a4, (char *) 0);
                _exit(127);
        }

        if (pid == -1)
	{
                return (-1);
        }

/*      istat = signal(SIGINT, SIG_IGN);
        qstat = signal(SIGQUIT, SIG_IGN);

        w = waitpid(pid, &status, 0);
        (void) signal(SIGINT, istat);
        (void) signal(SIGQUIT, qstat);

        return ((w == -1) ? -1: status);
*/
	return pid;
}

void do_pico(struct char_data *ch, char *argument, int cmd)
{
	int			pid;
	char			buf[MAX_STRING_LENGTH];
	struct descriptor_data	*curr;

	if (!ch) return;

	curr = ch->desc;
	if (!curr) return;

	sprintf(buf, "forking on descriptor %d\n\r", curr->descriptor);
	send_to_char(buf, ch);


	pid = avi_system(curr->descriptor, "../src/pico/test", "test", 
			NULL, NULL, NULL, NULL);
	sprintf(buf, "made a child for you on pid %d\n\r", pid);
	send_to_char(buf, ch);

	if (pid == -1)
	{
		return;
	}
	else
	{
		send_to_char("ignoring you now...\n\r", ch);
		curr->ignore_input = 1;
		curr->child_pid = pid;
		return;
	}
}
/* Do not allow a first level char to attack another player until 3rd lv */
/* and do not allow someone to kill a first level player */
bool is_first_level(struct char_data *ch, struct char_data *victim)
{
    if (IS_NPC(victim)||IS_NPC(ch))
    {
	return FALSE;
    }

    if ((!IS_NPC(victim))&&(GET_LEVEL(ch)<3))
    {
	send_to_char(
	    "You may not attack a player until you are level 3.\n\r",ch);
	return TRUE;
    }
    else if ( (GET_LEVEL(ch)>1)&&(GET_LEVEL(victim)==1) )
    {
	send_to_char("You may not attack a first level player.\n\r",ch);
	return TRUE;
    } else {
	return FALSE;
    }
}

bool is_in_safe(struct char_data *ch, struct char_data *victim)
	/* checks to see if PC is in safe room*/
{
    if ( IS_NPC(ch) || IS_NPC(victim) )
	return FALSE;
    if(stpEarthquake)return FALSE;
    if(ch==victim)return FALSE;
    if ( !IS_SET(world[ch->in_room]->room_flags, SAFE) )
	return FALSE;

    send_to_char( "No fighting permitted in this room.\n\r", ch );
    return TRUE;
}

void do_hit(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    struct char_data *victim=NULL;



    one_argument(argument, arg);

    if (*arg) {
	victim = get_char_room_vis(ch, arg);
	if (victim) {
    	  if(!if_allowed_to_attack(ch,victim))return;
	  if (!IS_NPC(victim)&&cmd!=0){
	    send_to_char("You must MURDER a player.\n\r",ch);
	    return;
	  }
	  if (victim == ch) {
	    send_to_char("You hit yourself..OUCH!.\n\r", ch);
	    act("$n hits $mself, and says OUCH!",
	    FALSE, ch, 0, victim, TO_ROOM);
	  } else {
	    if (is_in_safe(ch,victim)==TRUE){
	      act("$n tries to MURDER $N in a safe room!",
	      FALSE, ch, 0, victim, TO_ROOM);
	      return;
	    }

	    if (is_first_level(ch,victim)==TRUE){
	      return;
	    }
	    
	    if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
	      act("$N is just such a good friend, you simply can't hit $M.",
	      FALSE, ch, 0, victim, TO_CHAR);
	      return;
	    }
	    if ((GET_POS(ch)==POSITION_STANDING) &&
	    (victim != ch->specials.fighting)) {
	      hit(ch, victim, TYPE_UNDEFINED);
	      WAIT_STATE(ch, PULSE_VIOLENCE+2); /* HVORFOR DET?? */
	    } else {
	      send_to_char("You do the best you can!\n\r",ch);
	    }
	  }
	} else {
	  send_to_char("They aren't here.\n\r", ch);
	}
	  } else {
	send_to_char("Hit whom?\n\r", ch);
    }
}


void do_murder(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    struct char_data *victim=NULL;

    one_argument(argument, arg);
    if(!IS_SET(world[ch->in_room]->room_flags,NEUTRAL)&&
	!IS_SET(world[ch->in_room]->room_flags,CHAOTIC)){
        send_to_char("MURDERING is outlawed inside LAWFULL Areas!.\n\r",ch);
        return;
    }
    if (*arg)
    {
	victim = get_char_room_vis(ch, arg);
	if (victim)
	{
	    if (victim == ch)
	    {
		send_to_char("You hit yourself..OUCH!.\n\r", ch);
		act("$n hits $mself, and says OUCH!",
			FALSE, ch, 0, victim, TO_ROOM);
	    } else {
		if ( is_in_safe(ch,victim)==TRUE)
		    return;
		if ( is_first_level(ch,victim)==TRUE)
		    return;
	        if(!victim->specials.fighting&&!IS_NPC(victim)&&
			!victim->desc){
		    global_color=33;
		    send_to_char("You cannot start fighting someone who lost link!\n\r",ch);
	 	    global_color=0;
		    return;
		}
		if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim))
		{
		    act(
		"$N is just such a good friend, you simply can't hit $M.",
		    FALSE, ch,0,victim,TO_CHAR);
		    return;
		}

		if ((GET_POS(ch)==POSITION_STANDING) &&
		(victim != ch->specials.fighting))
		{
		    hit(ch, victim, TYPE_UNDEFINED);
		    WAIT_STATE(ch, PULSE_VIOLENCE+2); /* HVORFOR DET?? */
		} else {
		    send_to_char("You do the best you can!\n\r",ch);
		}
	    }
	} else {
	    send_to_char("They aren't here.\n\r", ch);
	}
    } else {
	send_to_char("Hit whom?\n\r", ch);
    }
}


void do_backstab(struct char_data *ch, char *argument, int cmd)
{
struct char_data *victim=NULL;
char name[256];
byte percent;

    if(in_a_shop(ch)){
		send_to_char("You realize it is not polite to fight in a public shop.\n\r",ch);
		return;
    }
	if(!IS_NPC(ch)&&ch->specials.stpMount){
		global_color=33;
		send_to_char("You think about dis-mounting and backstabing..\n\r",ch);
		global_color=0;
		return;
	}
    one_argument(argument, name);
    if (!(victim = get_char_room_vis(ch, name))) {
		send_to_char("Backstab who?\n\r", ch);
		return;
    }
    if (victim == ch) {
		send_to_char("How can you sneak up on yourself?\n\r", ch);
		return;
    }
    if (is_in_safe(ch,victim)==TRUE){
      	return;
    }
    if (is_first_level(ch,victim)==TRUE) {
      	return;
    }
    if (!ch->equipment[WIELD]) {
		send_to_char("You need to wield a weapon, to make it a success.\n\r",ch);
		return;
    }
    if (ch->equipment[WIELD]->obj_flags.value[3] != 11) {
		send_to_char("Only piercing weapons can be used for backstabbing.\n\r",ch);
		return;
    }
    if(IS_SET(victim->denyclass,DENYBACKSTAB)){
        act("Your attempted backstab had no affect on $M.",TRUE,ch,0,victim,TO_CHAR);
        return;
    }
                        	
    if (victim->specials.fighting) {
		send_to_char("You can't backstab a fighting person, too alert!\n\r", ch);
		return;
    }
    if(!if_allowed_to_attack(ch,victim))return;
    if(!IS_IN_BACK(victim)){
		send_to_char("Its to hard to sneak up on a formation unless your target is in the back.\n\r",ch);
		return;
    }
    check_killer(ch, victim);
    
    percent=number(1,101); /* 101% is a complete failure */

    strcpy(&victim->specials.last_attack[0],GET_NAME(ch));
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    if (AWAKE(victim) && (percent > ch->skills[SKILL_BACKSTAB].learned)) {
	if (damage(ch, victim, 0, SKILL_BACKSTAB))
		return;
	if (!ch) return;
    }
    else
	hit(ch,victim,SKILL_BACKSTAB);

    if (IS_AFFECTED(ch, AFF_SNEAK))
        affect_from_char(ch, SKILL_SNEAK);
}



void do_order(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int org_room,x,y;
    struct char_data *victim=NULL;

    half_chop(argument, name, message);

    if (!*name || !*message)
	send_to_char("Order who to do what?\n\r", ch);
    else if (!(victim = get_char_room_vis(ch, name)) &&
	     str_cmp("follower", name) && str_cmp("followers", name))
	    send_to_char("That person isn't here.\n\r", ch);
    else if (ch == victim)
	send_to_char("You obviously suffer from schitzophrenia.\n\r", ch);

    else {
	if (IS_AFFECTED(ch, AFF_CHARM)) {
	    send_to_char(
		"Your superior would not aprove of you giving orders.\n\r",ch);
	    return;
	}

	if (victim) {
	    sprintf(buf, "$N orders you to '%s'", message);
	    act(buf, FALSE, victim, 0, ch, TO_CHAR);
	    act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);

	    if ( (victim->master!=ch) || !IS_AFFECTED(victim, AFF_CHARM) )
		act("$n has an indifferent look.",
		    FALSE, victim, 0, 0, TO_ROOM);
	    else {
	      send_to_char("Ok.\n\r", ch);
	      command_interpreter(victim, message);
	    }
	} else {  /* This is order "followers" */
	    sprintf(buf, "$n issues the order '%s'.", message);
	    act(buf, FALSE, ch, 0, victim, TO_ROOM);

	    org_room = ch->in_room;

	    for (x=0;x<3;x++)
	    for(y=0;y<3;y++) {
		if(ch->formation[x][y])
		if (org_room == ch->formation[x][y]->in_room)
		    if (IS_AFFECTED(ch->formation[x][y], AFF_CHARM)) {
			found = TRUE;
			command_interpreter(ch->formation[x][y], message);
		      }
		  }
	    if (found)
		send_to_char("Ok.\n\r", ch);
	    else
		send_to_char(
		    "Nobody here are loyal subjects of yours!\n\r", ch);
	}
    }
}



void do_flee(struct char_data *ch, char *argument, int cmd)
{
	int belt_quest(struct char_data *ch, int cmd, char *arg);
    int i, attempt, loose, die;
	struct room_affect *rap;
    void gain_exp(struct char_data *ch, int gain, struct char_data *victim);
	extern bool is_trapped(struct char_data *ch);
	float fLeftHp;

	if(!AWAKE(ch))
		return;
	if(is_trapped(ch))	
		return;
    if(world[ch->in_room]->zone==198){
       if(number(0,100)<30){
	   global_color=32;
	   if(IS_AFFECTED(ch,AFF_FLYING)){
		act("$n tries to flee and hits $s head on some stalactite.",TRUE,ch,0,0,TO_ROOM);
		send_to_char("You try and flee but hit your head on some stalactite!\n\r",ch);
	   }else{
		act("$n tries to flee but trips over some stalagmite.",TRUE,ch,0,0,TO_ROOM);
		send_to_char("You try and flee but trip over some stalagmite!",ch);
	   }
	   global_color=0;
	   return;
       }
    }
    global_color=31;

    if (!(ch->specials.fighting)) {
	for(i=0; i<6; i++) {
	    attempt = number(0, 5);  /* Select a random direction */
	    if (CAN_GO(ch, attempt) &&
	    !IS_SET(world[EXIT(ch, attempt)->to_room]->room_flags, DEATH)) {
            if((!IS_SET(world[ch->in_room]->room_flags,NEUTRAL)&&
		!IS_SET(world[ch->in_room]->room_flags,CHAOTIC))&&
		(IS_SET(world[EXIT(ch,attempt)->to_room]->room_flags,NEUTRAL)||
		IS_SET(world[EXIT(ch,attempt)->to_room]->room_flags,CHAOTIC)))
		    continue;
		if(IS_NPC(ch))
        for(rap=world[EXIT(ch,attempt)->to_room]->room_afs;rap;rap=rap->next)
			if(rap->type==RA_SHIELD)
				return;
		act("$n panics, and attempts to flee.",
			TRUE, ch, 0, 0, TO_ROOM);
		if ((die = do_simple_move(ch, attempt, FALSE))== 1) {
		    /* The escape has succeded */
		    send_to_char("You flee head over heels.\n\r", ch);
		    global_color=0;
		    return;
		} else {
		    if (!die) act("$n tries to flee, but is too exhausted!",
			TRUE, ch, 0, 0, TO_ROOM);
		    global_color=0;
		    return;
		}
	    }
	} /* for */
	/* No exits was found */
	send_to_char("PANIC! You couldn't escape!\n\r", ch);
	global_color=0;
	return;
    }

    if(IS_UNDEAD(ch) 
	&& (ch->specials.fighting)
	&& (ch->specials.fighting->nr == 9801)
	&& (ch->specials.fighting->specials.death_timer)
	&& (ORIGINAL(ch)->specials.death_timer < 1)
	)
	{
	send_to_char(
	"You should not flee when your spirit is at stake!\n\r",ch);
	return;
	}

    for(i=0; i<6; i++) {
	attempt = number(0, 5);  /* Select a random direction */
	if (CAN_GO(ch, attempt) &&
	   !IS_SET(world[EXIT(ch, attempt)->to_room]->room_flags, DEATH)) {
            if((!IS_SET(world[ch->in_room]->room_flags,NEUTRAL)&&
		!IS_SET(world[ch->in_room]->room_flags,CHAOTIC))&&
		(IS_SET(world[EXIT(ch,attempt)->to_room]->room_flags,NEUTRAL)||
		IS_SET(world[EXIT(ch,attempt)->to_room]->room_flags,CHAOTIC)))
		    continue;
		if(IS_NPC(ch))
        for(rap=world[EXIT(ch,attempt)->to_room]->room_afs;rap;rap=rap->next)
			if(rap->type==RA_SHIELD)
				return;
	    act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
	    if ((die = do_simple_move(ch, attempt, FALSE))== 1) { 
	      /* The escape has succeded */
		if(!ch->specials.fighting)return;
		if(GET_LEVEL(ch)>=GET_LEVEL(ch->specials.fighting)){
			loose = GET_MAX_HIT(ch->specials.fighting)
			      - GET_HIT(ch->specials.fighting);
			loose *= GET_LEVEL(ch->specials.fighting);
			loose = abs(loose);
		}else{
			loose = GET_LEVEL(ch->specials.fighting)-GET_LEVEL(ch);
			loose*=1000;

		}
		if(loose>(GET_EXP(ch)/10))loose=GET_EXP(ch)/10;
		if ((!IS_NPC(ch)) && (IS_NPC(ch->specials.fighting)) )
		    gain_exp(ch, -loose, NULL);
		global_color=31;
		send_to_char("You flee head over heels.\n\r", ch);
		
		/* If more than 50% Hp left, start tracking */

		fLeftHp = (float) GET_HIT(ch->specials.fighting) / (float) GET_MAX_HIT(ch->specials.fighting);		
		if(IS_NPC(ch->specials.fighting) && (ch->specials.fighting != ch) 
		   && (fLeftHp > .5) )
		    if(ch->specials.fighting->master->specials.fighting == ch)
			ch->specials.fighting->master->specials.hunting = ch;
		    else
			ch->specials.fighting->specials.hunting = ch;
	
		if (ch->specials.fighting->specials.fighting == ch)
		    stop_fighting(ch->specials.fighting);
		stop_fighting(ch);
		global_color=0;
		if ( (int)world[ch->in_room]->funct == (int)belt_quest )
			(*world[ch->in_room]->funct)(ch, 9, "");
		return;
	    } else {
		if (!die) act("$n tries to flee, but is too exhausted!",
		    TRUE, ch, 0, 0, TO_ROOM);
		global_color=0;
		return;
	    }
	}
    } /* for */

    /* No exits was found */
    global_color=31;
    send_to_char("PANIC! You couldn't escape!\n\r", ch);
    global_color=0;
}

void try_trap(struct char_data *ch)
{
int room=0;
struct room_affect *rap=NULL;

    if(!IS_NPC(ch))return;
    room=ch->in_room;
    if(room<1||!world[room])return;
    for(rap=world[room]->room_afs;rap;rap=rap->next){
	if(rap->type==RA_TRAP&&!rap->value)break;
    }
    if(!rap)return;
    if(number(1,100)>85){
	global_color=32;
	act("You notice $n avoid a trap.",TRUE,ch,0,0,TO_ROOM);
	
	global_color=0;
	return;
    }    
    
    rap->ch=ch;   /* make trap point to critter     */
    rap->value=1; /* set trap to GOT SOMEONE! :)    */
    rap->timer=20; /* set it to stay for 5 mud hours */

    global_color=32;
    act("**SNAP!**  $n steps into a Trap!!",TRUE,ch,0,0,TO_ROOM);
    global_color=0;
    remove_from_formation(ch);
}

void do_trap(struct char_data *ch, char *argument, int cmd)
{
byte percent;
int room, skill, num_traps=0;
struct room_affect *rap=NULL;
struct obj_data *obj=NULL;

    if(IS_NPC(ch))return;
    if (GET_LEVEL(ch)<33 &&GET_CLASS(ch)!= CLASS_THIEF
	&&!IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF)){
	send_to_char("You better leave trapping skills to thiefs.\n\r",ch);
	return;
    }
    if(ch->specials.fighting)return;
    room=ch->in_room;
    if(room<1||!world[room])return;
    if(world[room]->zone==1){
	send_to_char("Traps have been outlawed inside the city of Medievia.\n\r",ch);
	return;
    }
    for(obj=ch->carrying;obj;obj=obj->next_content){
	if(obj->item_number==9822){
	    break;
	}
    }
    if(!obj){
	send_to_char("You realize you have no traps to lay...\n\r",ch);
	return;
    }
    for(rap=world[room]->room_afs;rap;rap=rap->next)
	if(rap->type==RA_TRAP)num_traps++;
    if(num_traps>=3){
	send_to_char("Try as you might, you can't fit another trap in this room.\n\r",ch);
	return;
    }
    percent=number(1,100); /* 101% is a complete failure */
    skill=ch->skills[SKILL_TRAP].learned;
    if (percent > skill) {
	global_color=33;
	send_to_char("SNAP, %&!^^@~!!! you almost broke your hand and you broke the trap!....\n\r",ch);
	act("**SNAP** .. $n tried to lay a trap and nearly lost a hand.",TRUE,ch,0,0,TO_ROOM);
	global_color=0;
	obj_from_char(obj);
	extract_obj(obj);
	obj=read_object(9823,0);/*broken trap*/
	obj_to_room(obj,room);	
	return;
    }
    global_color=33;
    send_to_char("You skillfully set and place the trap...backing away carefully.\n\r",ch);    
    act("You watch $n skillfully set and place a trap.\n\r",TRUE,ch,0,0,TO_ROOM);
    global_color=0;
    CREATE(rap,struct room_affect,1);
    rap->type=RA_TRAP;
    rap->timer=12;
    rap->value=0;
    rap->ch=ch;
    sprintf(log_buf,"%s",GET_NAME(ch));
    rap->text=str_dup(log_buf);
    rap->room=room;
    rap->next=world[room]->room_afs;
    world[room]->room_afs=rap;
    obj_from_char(obj);
    extract_obj(obj);

}


void do_meditate(struct char_data *ch, char *argument, int cmd)
{
byte percent;
int room;
int skill;

    if ((GET_CLASS(ch) != CLASS_WARRIOR) && GET_LEVEL(ch)<33
	&&GET_CLASS(ch)!= CLASS_THIEF
	&&!IS_SET(ch->player.multi_class,MULTI_CLASS_WARRIOR)
	&&!IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF)){
	send_to_char("You better leave all the martial arts to fighters.\n\r",
	    ch);
	return;
    }
    if(ch->specials.fighting)return;
    send_to_char("You meditate for a while....\n\r",ch);
    if(GET_POS(ch)!=POSITION_RESTING){
	send_to_char("You are not comfortable enough to focus...\n\r",ch);
	return;	
    }
    room=ch->in_room;
    if(room<1||!world[room])return;
   
    if(!world[room]->people||world[room]->people!=ch||world[room]->people->next_in_room){
	send_to_char("Too many people around for you to focus...\n\r",ch);
	return;
    }
    percent=number(1,135); /* 101% is a complete failure */
    skill=ch->skills[SKILL_MEDITATE_W].learned;
    if(ch->skills[SKILL_MEDITATE_T].learned>skill)
		skill=ch->skills[SKILL_MEDITATE_T].learned;

    if (percent > skill) {
	send_to_char("Just could not concentrate....\n\r",ch);
    	WAIT_STATE(ch, PULSE_VIOLENCE*2);
        GET_MOVE(ch)-=GET_MOVE(ch)/4;
	return;
    }
    if(GET_MAX_MOVE(ch)/4>GET_MOVE(ch)){
	send_to_char("You are just to tired...\n\r",ch);
	return;
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
    GET_MOVE(ch)-=GET_MAX_MOVE(ch)/4;
    spell_cure_critic(GET_LEVEL(ch), ch, ch, NULL);
    if(number(1,100)<50){
    	spell_cure_critic(GET_LEVEL(ch), ch, ch, NULL);
	if(number(1,100)<50)
	    spell_cure_critic(GET_LEVEL(ch), ch, ch, NULL);
    }
}
void  do_charge(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim, *next_v;
    byte percent;
    int dam,flag=0;
    int row,col;

    if(in_a_shop(ch)){
	send_to_char("You realize it is not polite to fight in a public shop.\n\r",ch);
	return;
    }
    if ((GET_CLASS(ch) != CLASS_WARRIOR) && GET_LEVEL(ch)<33
	&&!IS_SET(ch->player.multi_class,MULTI_CLASS_WARRIOR)){
	send_to_char("You better leave all the martial arts to fighters.\n\r",
	    ch);
	return;
    }
    for (victim = world[ch->in_room]->people; victim; victim = next_v){
		next_v=victim->next_in_room;
		if(ch->in_room != victim->in_room) 
			continue;
        if(!IS_NPC(victim)||!IS_NPC(victim->master))
			continue;
        if (is_in_safe(ch,victim)==TRUE){
            continue;
    	}
    	if (is_first_level(ch,victim)==TRUE){
      	    continue;
    	}
		if(IS_SET(victim->denyclass,DENYCHARGE))
			continue;
    	if(!if_allowed_to_attack(ch,victim))continue;
    	check_killer(ch, victim);
    	percent=number(1,101); /* 101% is a complete failure */
    	if (percent > ch->skills[SKILL_CHARGE].learned) {
	    if(damage(ch, victim, 0, SKILL_CHARGE))
			continue;
		if(!ch) continue;
	    GET_POS(ch) = POSITION_RESTING;
		fall(ch);
    	} else {
	    dam=0;
	    dam+=number(1,(int)(GET_LEVEL(ch)*1.5));
	    dam+=number(1,(GET_STR(ch)-16)*4);
	    dam+=number(1,(GET_DEX(ch)-16)*3);
	    dam+=number(1,GET_DAMROLL(ch));
	    if(dam>100)dam=100;
	    if(dam+10<GET_LEVEL(ch))dam=GET_LEVEL(ch)-10;
	    if(damage(ch, victim, dam, SKILL_CHARGE))
			continue;
		if(!ch) return;
      	GET_POS(victim) = POSITION_SITTING;
      	if(number(1,100)<50)
			GET_MOB_POS(victim)=POSITION_BELLY;
      	else 
			GET_MOB_POS(victim)=POSITION_BACK;
	    if(!IS_NPC(victim))
	    	WAIT_STATE(victim, PULSE_VIOLENCE*2);
	    else
		GET_MOB_WAIT(victim)+=2;
	    flag=1;
		fall(victim);
    	}
    }
    if(flag){
	if(!IS_NPC(ch))
            WAIT_STATE(ch, PULSE_VIOLENCE*3);
	else
	    GET_MOB_WAIT(ch)+=3;
	if(!IS_IN_FRONT(ch)){
	    for(row=0;row<3;row++){
		for(col=0;col<3;col++){
		    if(ch->master->formation[row][col]==ch){
			dam=number(0,2);
			next_v=ch->master->formation[0][dam];
			ch->master->formation[row][col]=next_v;
			ch->master->formation[0][dam]=ch;
			act("$n falls back into the formation in front.",TRUE,ch,0,0,TO_ROOM);
			act("You fall back into the formation in front.",TRUE,ch,0,0,TO_CHAR);
			if(next_v){
			    sprintf(log_buf,"%s gets jostled further back as %s finishes charging.",GET_NAME(next_v),GET_NAME(ch));
			    send_to_room(log_buf,ch->in_room);
			}
		    }
		}
	    }
	}
    }
}

void do_scan(struct char_data *ch, char *argument, int cmd)
{
    int door;
    char buf[MAX_STRING_LENGTH];
    byte percent;
    char *exits[] =
    {
        "                   Scanning North...",
        "                   Scanning East ...",
        "                   Scanning South...",
        "                   Scanning West ...",
        "                   Scanning Up   ...",
        "                   Scanning Down ..."
    };
    if ((GET_CLASS(ch) != CLASS_THIEF) && GET_LEVEL(ch)<33
	&&!IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF)){
	send_to_char("You better leave scanning to thieves.\n\r",ch);
	return;
    }
    act("$n focuses and remains still...scanning the area...",TRUE, ch, 0,0, TO_ROOM);
    *buf = '\0';
    if ( check_blind(ch) )
        return;
    global_color=34;
    for (door = 0; door <= 5; door++){
	percent=number(1,105); /* 101% is a complete failure */
        if (EXIT(ch, door)){
	    if ((IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
		 (IS_SET(EXIT(ch, door)->exit_info, EX_SECRET) ||
		  IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))) ||
		IS_SET(EXIT(ch, door)->exit_info, EX_ILLUSION)  ) continue; 
            if (EXIT(ch, door)->to_room != NOWHERE) {
		if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
                    sprintf(buf, "%s - [Closed]\n\r",exits[door]);
		    send_to_char(buf,ch);
		}else if (IS_DARK(EXIT(ch, door)->to_room)&&!ch->specials.holyLite&&world[EXIT(ch,door)->to_room]->zone!=198){
                    sprintf(buf, "%s - Too dark to tell\n\r",exits[door]);
		    send_to_char(buf,ch);
                }else{
                    sprintf(buf, "%s\n\r", exits[door]);
		    send_to_char(buf,ch);
		    if (percent > ch->skills[SKILL_SCAN].learned) continue;
		    if(world[EXIT(ch, door)->to_room]->people)
		    	list_char_to_char(world[EXIT(ch, door)->to_room]->people, ch, 0);
		}
	    } 
	}
    }
   global_color=0;
}

void do_throw(struct char_data *ch, char *argument, int cmd)
{
struct char_data *victim=NULL;
struct obj_data *obj=NULL;
char name[256];
byte percent;
int dam;

	if(!ch) {
		log_hd("## Bad ch in do_throw!");
		return;
	}
	
    if(in_a_shop(ch)){
	send_to_char("You realize it is not polite to fight in a public shop.\n\r",ch);
	return;
    }
    one_argument(argument, name);

    if ((GET_CLASS(ch) != CLASS_THIEF) && GET_LEVEL(ch)<33&&!IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF)){	
    	send_to_char("You better leave throwing to thiefs.\n\r",ch);
		return;
    }
    if (!(victim = get_char_room_vis(ch, name))) {
		if (ch->specials.fighting) {
		    victim = ch->specials.fighting;
		} else {
		    send_to_char("You turn about looking for your target...\n\r", ch);
	    	return;
		}
    }
    if (victim == ch) {
		send_to_char("Don't you think you are a bit close to throw to?\n\r", ch);
		return;
    } 
    if(is_in_safe(ch,victim)==TRUE){
      	return;
    }
    if(is_first_level(ch,victim)==TRUE){
      	return;
    }
    if(ch->equipment[HOLD]){
		send_to_char("You don't have a free hand to throw with.\r\n", ch);
		return;
    }
    obj=get_first_throwable(ch);
    if(!obj){
		send_to_char("You don't have anything to throw!\r\n", ch);
		return;
    }
    if (obj->obj_flags.eq_level > GET_LEVEL(ch)){
	send_to_char("You are not experienced enough to use that weapon\r\n", ch);
	return;
    }	
    if(IS_SET(victim->denyclass,DENYTHROW)){
	act("You cannot throw against $N.",TRUE,ch,0,victim,TO_CHAR);
	return;
    }
    if(!if_allowed_to_attack(ch,victim))return;
    check_killer(ch, victim);
    percent=number(1,133); /* 101% is a complete failure */
    if (percent > ch->skills[SKILL_THROW].learned) {
		if(damage(ch, victim, 0, SKILL_THROW)){
			obj_from_char(obj);
			obj_to_room(obj,ch->in_room);
			return;
		}
		if(!ch) return;
		obj_from_char(obj);
		obj_to_room(obj,ch->in_room);
		if(!IS_NPC(ch)){
    	        WAIT_STATE(ch, PULSE_VIOLENCE*2);
		}else{
		    GET_MOB_WAIT(ch)+=4;
		}
	} else {
		dam = dice(obj->obj_flags.value[1], obj->obj_flags.value[2]);
		dam*=6;
		dam+=number(1,GET_LEVEL(ch)*2);
		dam+=number(1,((GET_DEX(ch)-16)*15));
		dam+=number(1,(GET_DEX(ch)-13));
		if(dam>300)dam=300;
		if(dam+10<GET_LEVEL(ch))dam=GET_LEVEL(ch)-10;
		if(damage(ch, victim, dam, SKILL_THROW)){
			obj_from_char(obj);	
			obj_to_room(obj, ch->in_room);
			return;
		}
		if(!ch) return;
		obj_from_char(obj);
		obj_to_char(obj,victim);
	       	WAIT_STATE(ch, PULSE_VIOLENCE*1);
   	}
}

void do_bash(struct char_data *ch, char *argument, int cmd)
{
struct char_data *victim=NULL;
char name[256],flag;
byte percent;
int dam;

    if(in_a_shop(ch)){
		send_to_char("You realize it is not polite to fight in a public shop.\n\r",ch);
		return;
    }
    one_argument(argument, name);
    if ((GET_CLASS(ch) != CLASS_WARRIOR) && GET_LEVEL(ch)<33&&!IS_SET(ch->player.multi_class,MULTI_CLASS_WARRIOR)){
		send_to_char("You better leave all the martial arts to fighters.\n\r",ch);
		return;
    }
    if (!(victim = get_char_room_vis(ch, name))) {
		if (ch->specials.fighting) {
		    victim = ch->specials.fighting;
		} else {
		    send_to_char("Bash whom?\n\r", ch);
	    	return;
		}
    }
    if (victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
    }
    if(is_in_safe(ch,victim)==TRUE){
      	return;
    }
    if (is_first_level(ch,victim)==TRUE){
      return;
    }
    if (!ch->equipment[WIELD]) {
		send_to_char("You need to wield a weapon, to make it a success.\n\r",ch);
		return;
    }
    if(!if_allowed_to_attack(ch,victim))return;
	if(IS_SET(victim->denyclass,DENYBASH)){
		act("Your attempted bash has no affect on $N.",TRUE,ch,0,victim,TO_CHAR);
		return;
	}
    check_killer(ch, victim);
    percent=number(1,126); /* 101% is a complete failure */
    if (!IS_NPC(ch)&&percent > ch->skills[SKILL_BASH].learned){
		if(damage(ch, victim, 0, SKILL_BASH))
			return;
		if(!ch) return;
		GET_POS(ch) = POSITION_SITTING;
		fall(ch);
	}else if( percent > 85 ){
        if(damage(ch, victim, 0, SKILL_BASH))
			return;
		if(!ch) return;
        GET_MOB_POS(ch) = POSITION_SITTING;
		fall(ch);
    }else{
		flag=0;
   		if(!IS_IN_FRONT(victim)){
		    act("You try and bash $M but someone is in front of $M and you hit another..",TRUE,ch,0,victim,TO_CHAR);
		    send_to_char("Just a glancing blow...\n\r",ch);
		    victim=pick_victim(victim);
		    flag=1;
    	}
		dam=0;
		dam+=number(1,GET_LEVEL(ch))+GET_STR(ch);
		if(flag)dam/=2;
		if(damage(ch, victim, dam, SKILL_BASH))
			return;
		if(!ch) return;
		GET_POS(victim) = POSITION_SITTING;
		if(GET_MOB_POS(victim)==POSITION_STANDING)
           	GET_MOB_POS(victim)=POSITION_KNEELING;
		fall(victim);
		if(!IS_NPC(victim))
	    	WAIT_STATE(victim, PULSE_VIOLENCE*2);
		else
	    	GET_MOB_WAIT(victim)+=2;
    }
    if(!IS_NPC(ch))
    	WAIT_STATE(ch, PULSE_VIOLENCE*2);
    else
    	GET_MOB_WAIT(ch)+=2;

}



void do_rescue(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim=NULL, *tmp_ch=NULL;
    int percent;
    char victim_name[240];

    one_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name))) {
	send_to_char("Whom do you want to rescue?\n\r", ch);
	return;
    }

    if (victim == ch) {
	send_to_char("What about fleeing instead?\n\r", ch);
	return;
    }

    if(IS_UNDEAD(victim) 
	&& (victim->specials.fighting)
	&& (victim->specials.fighting->nr == 9801)
	&& (victim->specials.fighting->specials.death_timer)
	&& (ORIGINAL(victim)->specials.death_timer < 1)
	)
	{
	send_to_char(
	"There's nothing you can do, the gods must decide this battle.",ch);
	return;
	}

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
	send_to_char( "Doesn't need your help!\n\r", ch );
	return;
    }

    if (ch->specials.fighting == victim) {
	send_to_char(
	    "How can you rescue someone you are trying to kill?\n\r",ch);
	return;
    }

    for (tmp_ch=world[ch->in_room]->people; tmp_ch &&
	(tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)
	;

    if (!tmp_ch) {
	act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
	return;
    }

/* beef up rescue for warriors */

    if ((GET_CLASS(ch) != CLASS_WARRIOR) && GET_LEVEL(ch)<33
	&&!IS_SET(ch->player.multi_class,MULTI_CLASS_WARRIOR))
	send_to_char("But only true warriors can do this!", ch);
    else {
	percent=number(1,101); /* 101% is a complete failure */
	if (percent > ch->skills[SKILL_RESCUE].learned) {
	    send_to_char("You fail the rescue.\n\r", ch);
	    return;
	}

	send_to_char("Banzai! To the rescue...\n\r", ch);
	act("You are rescued by $N, you are confused!",
		FALSE, victim, 0, ch, TO_CHAR);
	act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);

	if (victim->specials.fighting == tmp_ch)
	    stop_fighting(victim);
	if (tmp_ch->specials.fighting)
	    stop_fighting(tmp_ch);
	if (ch->specials.fighting)
	    stop_fighting(ch);

	check_killer(ch, tmp_ch);
	/*
	 * so rescuing an NPC who is fighting a PC does not result in
	 * the other guy getting killer flag
	 */
	set_fighting(ch, tmp_ch);
	set_fighting(tmp_ch, ch);

	WAIT_STATE(victim, 2*PULSE_VIOLENCE);
    }

}



void do_kick(struct char_data *ch, char *argument, int cmd)
{
struct char_data *victim=NULL;
char name[256];
byte percent;
int dam;

    if(in_a_shop(ch)){
		send_to_char("You realize it is not polite to fight in a public shop.\n\r",ch);
	return;
    }
    if ((GET_CLASS(ch) != CLASS_WARRIOR) && GET_LEVEL(ch)<33&&!IS_SET(ch->player.multi_class,MULTI_CLASS_WARRIOR)){
		send_to_char("You better leave all the martial arts to fighters.\n\r",ch);
		return;
    }
    one_argument(argument, name);
    if (!(victim = get_char_room_vis(ch, name))) {
		if (ch->specials.fighting) {
		    victim = ch->specials.fighting;
		} else {
		    send_to_char("Kick whom?\n\r", ch);
	    	return;
		}
    }
    if (victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
    }
    if (is_in_safe(ch,victim)==TRUE){
      return;
    }
    if (is_first_level(ch,victim)==TRUE){
      return;
    }
    if(!if_allowed_to_attack(ch,victim))return;
	if(IS_SET(victim->denyclass,DENYKICK)){
		act("Your kick doesn't seem to hurt $N.",TRUE,ch,0,victim,TO_CHAR);
		return;
	}
    check_killer(ch,victim);
    if(!IS_IN_FRONT(ch)){
	send_to_char("You can't get forward enough in your formation to kick.\n\r",ch);
	return;
    }
    if(!IS_IN_FRONT(victim)){
		act("You can't get close enough to $M to use your feet.",TRUE,ch,0,victim,TO_CHAR);
		return;
    }
     /* 101% is a complete failure */
    percent=((10-(GET_AC(victim)/10))<<1) + number(1,111);

    if (percent > ch->skills[SKILL_KICK].learned) {
		if(damage(ch, victim, 0, SKILL_KICK))
			return;
		if(!ch) return;
    	if(!IS_NPC(ch))
    		WAIT_STATE(ch, PULSE_VIOLENCE);
    	else
			GET_MOB_WAIT(ch)+=1;
		return;
    }else{
		dam=0;
		dam+=number(1,GET_LEVEL(ch)-10);
		dam+=number(1,(GET_STR(ch)-16)*2);
		dam+=number(1,(GET_DEX(ch)-16)*1);
		dam+=number(1,GET_DAMROLL(ch)/2);
		if(dam>100)dam=100;
		if(dam+15<GET_LEVEL(ch))
			dam=GET_LEVEL(ch)-15;
		if(damage(ch, victim, dam, SKILL_KICK))
			return;
		if(!ch) return;
    }
    if(!IS_NPC(ch))
    	WAIT_STATE(ch, PULSE_VIOLENCE*2);
    else
		GET_MOB_WAIT(ch)+=2;
}

struct obj_data *get_first_throwable(struct char_data *ch){
    struct obj_data *obj;

    for(obj=ch->carrying;obj;obj=obj->next_content){
        if (IS_SET(obj->obj_flags.wear_flags,ITEM_THROW)&&obj->carried_by)
	 	   return(obj);
	}
    return(NULL);
}
