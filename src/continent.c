/***************************************************************************
*                MEDIEVIA CyberSpace Code and Data files                   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause      *
*                          All rights reserved                             *
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
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/*   extern struct char_data *character_list;

external vars  */
extern struct char_data *character_list;
extern char *GET_REAL_NAME(struct char_data *ch);
struct index_data *mob_index;
extern int number_of_players();
extern unsigned long int connect_count;
extern int number_of_rooms;
extern int number_of_zones;
extern char global_color;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern put_obj_in_store(struct obj_data *obj, struct char_data *ch, FILE *fpsave);
extern bool in_a_shop(struct char_data *ch);
extern int top_of_world;
extern int top_of_zone_table;


/* Endar the banker is #5603 */

int trellor_endar(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n laughs out loud suddenly, and nearly falls out of his seat.", 1, ch, 0, 0, TO_ROOM);
			return(TRUE);
			break;
		case 2 :
			act("$n rubs his palms together, an evil grin on his face.", 1, ch, 0, 0, TO_ROOM);
			return(TRUE);
			break;
		case 3 :
			act("$n looks up suddenly from his paperwork.", 1, ch, 0, 0, TO_ROOM);
			return(TRUE);
			act("$n says, \"I'm sorry, what did you want again?\"", 1, ch, 0, 0, TO_ROOM);
			break;
		case 4 :
			act("$n scratches out something on his notepad with his pen.", 1, ch, 0, 0, TO_ROOM);
			return(TRUE);
			break;
		case 5 :
			act("$n mutters, \"And the Earl expects me to be MORE at ease with those damn mercs around...\"", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"I can't trust 'em any more than any commoner.\"", 1, ch, 0, 0, TO_ROOM);
			return(TRUE);
			break;
		default :
                        return(FALSE);
			break;
	}
}


/* Silor the innkeeper is #5605 */

int trellor_silor(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n looks at you and chuckles quietly to himself.", 1, ch, 0, 0, TO_ROOM);
			return(TRUE);
			break;
		case 2 :
			act("$n inspects the desktop quite meticulously.", 1, ch, 0, 0, TO_ROOM);
			return(TRUE);
			break;
		case 3 :
			act("$n shakes his head sadly.", 1, ch, 0, 0, TO_ROOM);
			act("$n whispers, \"Sorry, no rooms are available.\"", 1, ch, 0, 0, TO_ROOM);
			act("$n sighs, \"Or maybe there are...I don't know.\"", 1, ch, 0 , 0, TO_ROOM);	
			return(TRUE);
			break;
		case 4 :
			act("$n lifts his head and sniffs at the air.", 1, ch, 0, 0, TO_ROOM);
			act("$n whispers, \"I can smell it...not much time...\"", 1, ch, 0, 0, TO_ROOM);
			act("$n throws back his head and cackles with insane glee!", 1, ch, 0, 0, TO_ROOM);
			return(TRUE);
			break;
		case 5 :
			act("$n chuckles to himself.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Now they brought in the Stormwatch...\"", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"This is gonna get real interestin' soon...\"", 1, ch, 0, 0, TO_ROOM);
			return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* Mika is #5606 */

int trellor_mika(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n nudges you softly with her elbow and points to the stairs.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Upstairs is where the action is...\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n smirks.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Even the Earl himself is one of my regular customers.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n gives out a haughty laugh and smiles at you wickedly.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n giggles discreetly.", 1, ch, 0, 0, TO_ROOM);
			act("$n whispers to you, \"Don't worry, we can accomodate the ladies too.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 5 :
			act("$n checks each of the doors to make sure they are securely bolted.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 6 :
			act("$n giggles discreetly.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Oh, and I thought it was Pherrence again!\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 7 :
			act("$n says, \"I run an honorable establishment, I'll have you know!\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 8 :
			act("$n says, \"Say...you're not with the Stormwatch are you?\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}


/* Lokrath is #5617 */

int trellor_lokrath(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n sniffs sadly.", 1, ch, 0, 0, TO_ROOM);
			act("One of Lokrath's horses nuzzles up to him warmly.", 1, ch, 0, 0, TO_ROOM);
			act("$n breaks out into a smile.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n says, \"I love my horses.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n wipes his hands slowly on his tunic.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"No, it isn't what you think.\"", 1, ch, 0, 0, TO_ROOM);
			act("$n sighs loudly.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n screams in outrage.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"All this talk of marriage!\"", 1, ch, 0, 0, TO_ROOM);
			act("$n mutters to himself.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 5 :
			act("$n throws back his head and cackles with insane glee!", 1, ch, 0, 0, TO_ROOM);
			act("A small dribble of saliva runs down from his mouth.", 1, ch, 0, 0, TO_ROOM);
			act("$n gently pats one of his horses, and caresses it lovingly.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 6 :
			act("$n says, \"Oh, they I don't need them at all...\"", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"All I need is right here!\"", 1, ch, 0, 0, TO_ROOM);
			act("$n gestures towards the entire stable.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 7 :
			act("$n growls like an animal.", 1, ch, 0, 0, TO_ROOM);
			act("$n asks you softly, \"But what do YOU think?\"", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Do you think I am deranged also???\"", 1, ch, 0, 0, TO_ROOM);
			act("$n waves his hand absently, \"Then begone!\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

int trellor_guard(struct char_data *ch, int cmd, char *arg)
{
struct char_data *tch, *evil;
int max_evil, i;
bool callforhelp = TRUE;
 
if (cmd || !AWAKE(ch) )
	return (FALSE);

if((ch->specials.fighting) && (ch->specials.fighting->specials.fighting))
  {
  for(tch=ch->specials.fighting->specials.fighting; tch ; tch=tch->next)
	if((tch->nr == ch->nr) && (tch!=ch))
		{
		callforhelp = FALSE;
		break;
		}
  }

if( (ch->specials.fighting) 
	&& callforhelp
	&& (number(1,(PULSE_VIOLENCE * 40))) )
 {
 for (tch=character_list; tch; tch = tch->next  )
      {
      if ((!tch->specials.fighting) && (tch->nr == ch->nr) )
	{
 	sprintf(log_buf,"calls for help fighting %s.",
        	GET_NAME(ch->specials.fighting));
	do_emote(ch, log_buf, 0);
	if (ch->in_room != tch->in_room)
		{
		do_emote(tch, 
			"runs off to another part of the city to assist a comerade.", 0);
		char_from_room(tch);
		char_to_room(tch, ch->in_room);
		do_emote(tch, 
			"runs in from another part of the city to assist.", 0);
		}
        act("$n joins the fight!", FALSE, tch, 0,ch,TO_ROOM);
	hit(tch, ch->specials.fighting, TYPE_UNDEFINED);
	return(TRUE);
	}
   }
 }

if (ch->specials.fighting)
        return (FALSE);

max_evil = 300;
evil = 0;
for (tch=world[ch->in_room]->people; tch; tch = tch->next_in_room)
  {
  if (tch->specials.fighting) 
	{
	if ((GET_ALIGNMENT(tch) < max_evil) 
		&&  (IS_NPC(tch) || IS_NPC(tch->specials.fighting)))
		{
		max_evil = GET_ALIGNMENT(tch);
		evil = tch;
		}
	}
  }
       
if (evil && !IS_EVIL(evil->specials.fighting))
	{
	act(
	 "$n screams 'PROTECT THE INNOCENT!  BANZAI!!! CHARGE!!! ARARARAGGHH!'",
	 FALSE, ch, 0, 0, TO_ROOM);
	hit(ch, evil, TYPE_UNDEFINED);
	return(TRUE);
	}
 
for (tch=world[ch->in_room]->people; tch; tch = tch->next_in_room)
  {
  if (	CAN_SEE(ch, tch)
	&& ((GET_LEVEL(tch) > 13) && (GET_LEVEL(tch) < 32))
	&& (GET_CLASS(tch) == CLASS_THIEF)
	&& (number(1,100) < GET_LEVEL(tch))
	)
	{
	sprintf(log_buf,
		"%s You're quite a seedy looking character.",GET_NAME(tch));
	do_talk(ch, log_buf, 0);
	sprintf(log_buf,
		"Grins evilly at %s.",GET_NAME(tch));
	do_emote(ch, log_buf, 0);
	do_emote(ch, 
		"smirks and says, \"I haven't had that much action lately.\"", 0);
	sprintf(log_buf,
		"cackles at %s, \"Prepare to die, theiveing scum!\"",
		GET_NAME(tch));
	do_emote(ch, log_buf, 0);
	hit(ch, tch, TYPE_UNDEFINED);
        return(TRUE);
	}
  }

if(ch->nr == 5620)
	{
        i = (number(1,250));
        switch(i) {
                case 1 :
                        act("$n glares at you harshly.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
                        break;
                case 2 :
                        act("$n throws back his head and cackles with insane glee!", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
                        break;
                case 3 :
                        act("$n says, \"Don't look at me, you vagrant.\"", 1, ch,
0, 0, TO_ROOM);
                        act("$n slaps you across the face with his armored hand.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
                        break;
                case 4 :
                        act("$n says, \"I love this job.\"", 1, ch, 0, 0, TO_ROOM);
                        act("$n grins evilly at you.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
                        break;
                default :
                        return(FALSE); 
                        break;
		}
        }
if(ch->nr == 5602)
	{
        i = (number(1,250));
        switch(i) {
                case 2 :
                        act("$n shifts his weight from one foot to the other.",
1, ch, 0, 0, TO_ROOM);
                        act("$n sighs loudly.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
                        break;
                case 3 :
                        act("$n stares at you blankly for a few seconds, then quickly averts his eyes.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
                        break;
                case 4 :
                        act("$n pokes you in the ribs.", 1, ch, 0, 0, TO_ROOM);
                        act("$n says, \"Move along, move along...\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
                        break;
                case 5 :
                        act("$n holds a picture of a young peasant child up to your face.", 1, ch, 0, 0, TO_ROOM);
                        act("$n asks you, \"Have you seen this boy?\"", 1, ch, 0,
0, TO_ROOM);
                        act("$n giggles insanely.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
                        break;
                case 6 :
                        act("$n mutters, \"Don't need them damn Stormwatch mercs...\"", 1, ch, 0, 0, TO_ROOM);
                        act("$n mutters, \"They'll just get in my way.\"", 1, ch,
0, 0, TO_ROOM);
                        return(TRUE);
                        break;
                default :
                        return(FALSE); 
                        break;
		}
        }                


return(FALSE);
}


/* Mobile Virtual #'s are given */
/* Trellor gate guard is #5601 */

int trellor_gate(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n mutters under his breath.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n yawns loudly and shifts his weight.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n eyes you over carefully.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n mutters, \"Why the hell did he send for the Stormwatch??\"", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"I can handle anything that comes through this gate.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* Lady of the finch is #5607 */

int trellor_finchlady(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n nudges you softly with her elbow and points to the bed.", 1, ch, 0, 0, TO_ROOM);
			act("$n rubs herself gently and moans softly.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n smirks and says, \"I've even served the Earl himself once.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n giggles to herself as she eyes you over slowly.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n whispers, \"I don't only work with men, I'll have you know.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 5 :
			act("$n checks the door to make sure it is securely bolted.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 6 :
			act("$n hums softly to herself.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 7 :
			act("$n runs her fingers through her hair.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 8 :
			act("$n says, \"You don't have to lie to me, what do you really like?\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 9 :
			act("$n nudges you with her elbow.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"You're paying by the hour, remember?\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 10 :
			act("$n gives you an innocent look and bats her eyebrows.", 1, ch, 0, 0, TO_ROOM);
			act("$n whispers to you, \"You won't hurt me, will you?\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 11 :
			act("$n says, \"I'm not into that kinky stuff, so don't get any ideas.\"", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"I only make an exception for Pherrence.\"", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"He's the Earl of this town, you know.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* drunken man is #5608 */

int trellor_drunk_man(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n nudges you with his elbow.", 1, ch, 0, 0, TO_ROOM);
			act("$n winks at you suggestively.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n growls like a rabid dog.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Shtop lookin' at me, willya?\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n hiccups loudly and offers you his bottle.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n sighs loudly.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Goddam Earl... he raised da tax on liquor agin!\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 5 :
			act("$n looks to his left and right suddenly.", 1, ch, 0, 0, TO_ROOM);
			act("$n whispers, \"Something's rotten in this town...\"", 1, ch, 0, 0, TO_ROOM);
			act("$n whispers, \"Get out while you can!\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a lecherous old man is #5609 */

int trellor_lecher(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n nudges you with his elbow.", 1, ch, 0, 0, TO_ROOM);
			act("$n winks at you suggestively.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n rubs his hands up and down your body.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Oh, so smooooth!\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n beckons with his hand.", 1, ch, 0, 0, TO_ROOM);
			act("$n calls, \"Come here, my precious...\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n glances about himself furtively.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Damn guards are always around.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 5 :
			act("$n stops a little boy in the street and pats him on the head.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Go on now, young sir, I'll catch up to you later.\"", 1, ch, 0, 0, TO_ROOM);
			act("$n laughs and says, \"I love children a great deal, as you can see.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 6 :
			act("$n looks at a woman passing by and winks at her.", 1, ch, 0, 0, TO_ROOM);
			act("$n slaps the woman's rear and chortles merrily.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Now, that's a fine piece of meat!\"", 1, ch, 0, 0, TO_ROOM);
			act("The woman slaps $n, leaving a hand shaped welt.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 7 :
			act("$n pokes you in the ribs.", 1, ch, 0, 0, TO_ROOM);
			act("$n throws back his head and cackles with insane glee!", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"And no, that wasn't my finger...\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a young boy is #5610 */

int trellor_boy(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n looks at you in awe, his jaw hanging open.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n makes a face at you and sticks out his tongue.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n says, \"I was playing in the warehouse today and found...\"", 1, ch, 0, 0, TO_ROOM);
			act("$n looks around to make sure no one is listening.", 1, ch, 0, 0, TO_ROOM);
			act("$n whispers, \"A trap door!\"", 1, ch, 0, 0, TO_ROOM);
			act("$n frowns.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"I can't open it though, it's locked.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n struts.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"My father says I might be Earl one day.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a young girl is #5611 */

int trellor_girl(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,240));
	switch(i) {
		case 1 :
			act("$n looks at you and giggles shyly.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n sticks her chin out, giving you a pouty look.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Madam Mika says I should start practicing early.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n struts about, trying to imitate your particular swagger.", 1, ch, 0, 0, TO_ROOM);
			act("$n laughs and says, \"Am I close?\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n says, \"I'm gonna be a Lady of the Finch one of these days.\"", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Really, I am!\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a drunken brawler is #5612 */

int trellor_brawler(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n growls like an animal.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n sticks his chin out in your direction.", 1, ch, 0, 0, TO_ROOM);
			act("$n says, \"Go ahead, give it yer best shot!\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n throws back his head and cackles with insane glee!", 1, ch, 0, 0, TO_ROOM);
			act("$n taunts, \"Any of you losers gonna try me?\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("A man sitting at a table gets up and slaps the brawler.", 1, ch, 0, 0, TO_ROOM);
			act("A man screams, \"Shut up!\"", 1, ch, 0, 0, TO_ROOM);  
			act("A man says, \"Just 'cause you can't afford Mika's rates doesn't give you da right ta bitch about it!\"", 1, ch, 0, 0, TO_ROOM);
			act("$n sniffs sadly and breaks out in tears.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 5 :
			act("$n begins to sob uncontrollably.", 1, ch, 0, 0, TO_ROOM);
			act("$n sniffs, \"Why did she have to leave me????\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a small white puppy is #5621 */

int trellor_puppy(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n looks at you curiously.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n barks loudly, catching your attention.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n wags his tail and licks your hand.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a wealthy male citizen is #5622 */

int trellor_citizen_male(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n looks at you curiously.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n eyes you up and down and mutters to himself.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n flicks a coin in your general direction and smirks.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a wealthy female citizen is #5623 */

int trellor_citizen_female(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n looks at you curiously.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n sneers at you with contempt.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n flicks a coin in your general direction and smirks.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a wandering vagrant is #5624 */

int trellor_vagrant(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n looks at you curiously.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n rummages through the garbage.", 1, ch, 0, 0, TO_ROOM);
			act("$n curses under his breath.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n growls like an animal.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a peasant woman is #5625 */

int trellor_peasant_female(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n mutters, \"That blasted butcher...tryin' to cheat me again.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n says, \"Why do we need so many guards?!\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 3 :
			act("$n mutters, \"Mika's pocket is gettin' fat offa my husband!\"", 1, ch, 0, 0, TO_ROOM);
			act("$n sighs loudly.", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n growls, \"That idiot Pherrence is going to run this town into the ground...\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}

/* a peasant man is #5626 */

int trellor_peasant_male(struct char_data *ch, int cmd, char *arg)
{
	int i;

	if(cmd) return FALSE;
	
	i = (number(1,250));
	switch(i) {
		case 1 :
			act("$n mutters, \"That sick bastard Lokrath...and what he does with those poor horses...\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 2 :
			act("$n says, \"Damn mercenaries...you can't trust 'em.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		case 4 :
			act("$n growls, \"That woman...always gettin' in the way.\"", 1, ch, 0, 0, TO_ROOM);
                        return(TRUE);
			break;
		default :
                        return(FALSE); 
			break;
	}
}



