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
extern struct room_data *world[MAX_ROOM];
extern struct char_data *mobs[MAX_MOB];
extern struct obj_data *objs[MAX_OBJ];
extern int top_of_world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern char global_color;
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
extern void save_room_actions(struct char_data *ch);
extern int number_of_rooms;

/*		           /Action Variables\				  */
/*------------------------------------------------------------------------*/
struct room_actions *ractions[MAX_RACTION];  /*raction storage area*/
struct raction_timing *raction_seconds;
struct raction_oe_design raction_design[23];


void do_raction(int room, struct char_data *ch, int action_num);
void do_editroom_actions(struct char_data *ch, char *argument, int cmd);
/* METHODS */
#define PLAYER 1  /*Do things per roll per player*/
#define ROOM 2    /*Roll once, do to all at once */

/*TO ADD A NEW ACTION:  Up the array raction_design by 1.  Add the action
name and the menu number to raction_list using the next number.  Set up
the raction_design with the proper text to be seen by the action OE user
and the order of fields and which fields to use by the NEXT fields in
raction_design.  Then code what the action does in do_raction.  Keep in
mind you are adding just a primitive tool, the power of actions is the
SEQUENCE of stringing them together, not in singular events.  Make sure
you understand the WHOLE action system first, know the assumptions made
by the system and know the effects of every action.  The purpose of
actions is to allow coding of VR events, like a script in a movie where
the script changes due to player variables, luck and prior events.  The
goal is to allow this vr ability to be made completly online in an easy
to use method by designing basic actions and putting them together
online in a logic sequence, using plain english and code that asks the
proper questions.  Immediate testing and no compiling and using plain
prompted english input makes this method better than any other.
KEEP IN MIND that actions are stored as part of a room, loose the room,
you loose the actions.  Object and Mobile actions will follow.
You must also Up the define below MAX_ACTION_TYPE.*/

char *raction_list[]={
" 0-Chatter          ",
" 1-Stat Change      ",
" 2-Room Change      ",
" 3-Set Action Start ",
" 4-Have Item?       ",
" 5-Item in Inv?     ",
" 6-Item in Equip?   ",
" 7-Load Mobile      ",
" 8-Object to Mobile ",
" 9-Object to Player ",
"10-Load Object      ",
"11-Lock Door        ",
"12-Unlock Door      ",
"13-Open Door        ",
"14-Close Door       ",
"15-Force Mob        ",
"16-Force Player     ",
"17-Player Waits     ",
"18-Said This?       ",
"19-Room occupied?   ",
"20-Make two way exit",
"21-kill exit one way",
"22-zone chatter     ",
"$"
};

#define MAX_TYPE_ACTION 23
#define ARG1 1001
#define ARG2 1002
#define ARG3 1003
#define METHOD 1004
#define TEXT1 1005
#define TEXT2 1006
#define TEXT3 1007
#define END 666

void setup_next_prompt(struct char_data *ch, int next)
{
    switch(next){
	case 1001:
	    strcpy(ch->p->queryprompt,
	    raction_design[(int)ractions[ch->internal_use]->type].arg1);
	    ch->p->querycommand=next;
	    break;
	case 1002:
	    strcpy(ch->p->queryprompt,
	    raction_design[(int)ractions[ch->internal_use]->type].arg2);
	    ch->p->querycommand=next;
	    break;
	case 1003:
	    strcpy(ch->p->queryprompt,
	    raction_design[(int)ractions[ch->internal_use]->type].arg3);
	    ch->p->querycommand=next;
	    break;
	case 1004:
	    strcpy(ch->p->queryprompt,
	    raction_design[(int)ractions[ch->internal_use]->type].method);
	    ch->p->querycommand=next;
	    break;
	case 1005:
	    send_to_char(raction_design[(int)ractions[ch->internal_use]->type].text1,ch);
            global_color=31;
            act("$n starts editing the room Actions",TRUE,ch,0,0,TO_ROOM);
            global_color=0;
            ch->desc->str = &ractions[ch->internal_use]->text1;
            ch->desc->max_str = 2500;
	    ch->p->querycommand=next;
 	    strcpy(ch->p->queryprompt,"\n\rPRESS [RETURN]");
	    break;
	case 1006:
	    send_to_char(raction_design[(int)ractions[ch->internal_use]->type].text2,ch);
            global_color=31;
            act("$n starts editing the room Actions",TRUE,ch,0,0,TO_ROOM);
            global_color=0;
            ch->desc->str = &ractions[ch->internal_use]->text2;
            ch->desc->max_str = 2500;
	    ch->p->querycommand=next;
 	    strcpy(ch->p->queryprompt,"\n\rPRESS [RETURN]");
	    break;
	case 1007:
	    send_to_char(raction_design[(int)ractions[ch->internal_use]->type].text3,ch);
            global_color=31;
            act("$n starts editing the room Actions",TRUE,ch,0,0,TO_ROOM);
            global_color=0;
            ch->desc->str = &ractions[ch->internal_use]->text3;
            ch->desc->max_str = 2500;
	    ch->p->querycommand=next;
 	    strcpy(ch->p->queryprompt,"\n\rPRESS [RETURN]");
	    break;
	case 666:
	    do_editroom_actions(ch,"",9);
	    break;

    }

}
void setup_raction_design(void)
{
/* chatter */

raction_design[0].chance=str_dup(
"
CHANCE:
Enter the percentage 1-100 of this chatter firing.  Remember a lot of
Chatter actions have a 100 percent chance as they are just text talking
about another action that just happened.  The chatter is also a good
way of using chance to start a whole squence though.
Enter 1-100 for the % chance of firing> ");
raction_design[0].chance_next=METHOD;

raction_design[0].method=str_dup(
"
METHOD:
Select a METHOD.  ROOM means this action will roll the chance die just
once and everyone in room will see the same CHATTER text.  With the PLAYER
method, the text can be to_room and to_char.  The Player method as in all
other action will split the FOLLOWING sequences up, each action following
will be duplicated and ran for each player in the room.
1-PLAYER
2-ROOM
Select a number 1 or 2> ");
raction_design[0].method_next=ARG1;

raction_design[0].arg1=str_dup(
"
ARG1:
ROOM #, if this chatter text will happen in this room, type in 0,
else, if for some reason you want to have text show in another room
like You hear a major battle from the north.. enter in the room number
that you wish this chatter to display.
Enter the room number> ");
raction_design[0].arg1_next=TEXT1;

raction_design[0].text1=str_dup(
"
TEXT1:
Enter in your chatter text....
If method was ROOM then this is what the room will see...
If this action or any previous action in this sequence was a PLAYER method,
the text here will be shown to everyone in the room except the player the
sequence is working on.  Any %n in the text will be filled with that players
name. Example: You see %n get hit by the rock slide and fall off the cliff.
");
raction_design[0].text1_next=TEXT2;

raction_design[0].text2=str_dup(
"
TEXT2:
This is what the PLAYER will see. Example:
Your hit by a boulder and go flying....
If the method was ROOM and no previous action was method PLAYER, 
This field will not be used so just @ it. Otherwise the previous text
will be what the room seen and this is what the victim player will see.
");
raction_design[0].text2_next=END;

/*start action in timing Q*/
raction_design[3].chance=str_dup(
"
CHANCE:
Enter the percentage 1-100 of putting the action on the timing Q.
This will almost always be 100.  You may have a complicated sequence
following where this is not 100 and depending on scenarios, another is
put on 1 later, that later sequence should at some point start the whole
scenario over....
Enter 1-100 for the % chance of firing> ");
raction_design[3].chance_next=METHOD;

raction_design[3].method=str_dup(
"
METHOD:
Enter the method. A Room method means this and the following action will
start as just one copy of sequence with the chance die being rolled once.
A Player method will make ALL following actions be duplicated for each
player.
Enter 1-Player
      2-Room
Enter the number 1 or 2> ");
raction_design[3].method_next=ARG1;

raction_design[3].arg1=str_dup(
"
ARG1:
Enter the action number to put on the timing Q.  Remember, no checking
is done by the code, so enter a valid action or the thing will not start.
Enter the action number> ");
raction_design[3].arg1_next=ARG2;

raction_design[3].arg2=str_dup(
"
ARG2:
Enter in the number of seconds from now that the action should fire.
You can introduce an amount of randomness here also.  If you want enter in
here the MINimum number of seconds and on the next prompt enter in the
MAXimum number of seconds. The code will throw a die and pick the time.
Otherwise just enter the exact seconds and on next prompt enter 0.
Enter # seconds> ");
raction_design[3].arg2_next=ARG3;

raction_design[3].arg3=str_dup(
"
ARG3:
Enter in the MAXimum # of seconds, make sure this number is GREATER than
the previous number.
If there is no randomness, just enter 0 here.
Enter # seconds> ");
raction_design[3].arg3_next=END;

/*make two way door*/
raction_design[20].chance=str_dup(
"
CHANCE:
Enter the percentage 1-100 of making the two way door.
This will almost always be 100.  You may have a complicated sequence
following where this is not 100 and depending on scenarios, another is
put on 1 later, that later sequence should at some point start the whole
scenario over....
Enter 1-100 for the % chance of firing> ");
raction_design[20].chance_next=ARG1;

raction_design[20].arg1=str_dup(
"
ARG1:
Enter the room number where exit is FROM. Remember, no checking is done.
Enter the from room number to make the exit> ");
raction_design[20].arg1_next=ARG2;

raction_design[20].arg2=str_dup(
"
ARG2:
Enter the number corisponding to the direction the exit will be made.
Remember, no exit must be there, if there is, no action will be taken.
0-North 1-East 2-South 3-West 4-Up 5-Down
Enter the number for direction>  ");
raction_design[20].arg2_next=ARG3;

raction_design[20].arg3=str_dup(
"
ARG3:
Enter the room number the exit goes TO. Remember no checking is done to see
if the room exists or if an exit in the reverse direction exists.
Enter the room number the exit will go TO> ");
raction_design[20].arg3_next=END;

/*delete door one way*/
raction_design[21].chance=str_dup(
"
CHANCE:
Enter the percentage 1-100 of deleting the exit.
This will almost always be 100.  You may have a complicated sequence
following where this is not 100 and depending on scenarios, another is
put on 1 later, that later sequence should at some point start the whole
scenario over....
Enter 1-100 for the % chance of firing> ");
raction_design[21].chance_next=ARG1;

raction_design[21].arg1=str_dup(
"
ARG1:
Enter the room number where the exit will be deleted.  Remember this deletes
the exit ONW WAY ONLY.  If you want to kill both ways you will need two
actions, one for each direction.
Enter the room number where exit is deleted> ");

raction_design[21].arg1_next=ARG2;

raction_design[21].arg2=str_dup(
"
ARG2:
Enter the direction of the exit to delete.
0-North 1-East 2-South 3-West 4-Up 5-Down
Enter the number for the direction to delete> ");
raction_design[21].arg2_next=END;

/*zone chatter */
raction_design[22].chance=str_dup(
"
CHANCE:
Enter the percentage 1-100 of this chatter firing.  Remember a lot of
Chatter actions have a 100 percent chance as they are just text talking
about another action that just happened.  The chatter is also a good
way of using chance to start a whole squence though.
Enter 1-100 for the % chance of firing> ");
raction_design[22].chance_next=ARG1;

raction_design[22].arg1=str_dup(
"
ARG1:
Enter the zone number to show text in..
Enter the zone number> ");
raction_design[22].arg1_next=TEXT1;

raction_design[22].text1=str_dup(
"
TEXT1:
Enter in your chatter text....
Everyone in zone will see this.
");
raction_design[22].text1_next=END;


}

void do_editroom_actions(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    int number,x;
    struct raction_timing *rt=NULL,*prt=NULL,*crt=NULL;
    struct room_actions *ra=NULL;
    long l;
    if (IS_NPC(ch))
        return;
    one_argument(argument, buf);

   if(!IS_PLAYER(ch,"Vryce")&&!IS_PLAYER(ch,"Io")&&!IS_PLAYER(ch,"Firm")){
	send_to_char("Actions can only be made by Vryce or Io at the moment.\n\r",ch);
	return;
   }
   if(cmd==9){
        global_color=32;
        send_to_char("\n\r\n\r    EDIT ROOM ACTIONS",ch);
        global_color=0;
        ch->p->queryfunc=do_editroom_actions;
        strcpy(ch->p->queryprompt,
         "
 1-Make a NEW Action
 2-Show an action in DETAIL
 3-Delete an Action
 4-Link an Action to an Action(fired)
 5-Link an Action to an Action(not-fired)
 6-List Actions in this room
 7-Put Action in Time Start
 8-Remove Action from Time Start
 9-Make an action AUTOSTART when game starts
10-Save all actions.
q-Done
Make a selection> ");
        ch->p->querycommand=10;
        return;
   }
   if(cmd==10){
	if(buf[0]=='q'){/*done*/
	   ch->p->querycommand=0;
	   return;
	}
	number=atoi(buf);
	switch(number){
	   case 1:/*make*/
		for(number=0;number<MAX_RACTION;number++)
		    if(!ractions[number])
			break;
		if(number>=MAX_RACTION){
		    send_to_char("ACTIONS ARE FULL! Increase MAX_RACTION.\n\r",ch);
		    do_editroom_actions(ch,"",9);
		    return;
		}
                CREATE(ra, struct room_actions, 1);
		ra->autostart=0;
		ra->action_num=number;
		ra->in_room=ch->in_room;
		ra->method=ROOM;
		ractions[number]=ra;
		sprintf(log_buf,"Action [%d] Created, start editing...\n\r",number);
		send_to_char(log_buf,ch);
		ch->p->querycommand=13;
		ch->internal_use=number;
		x=number=0;
		strcpy(ch->p->queryprompt,"
                         Select the Action TYPE\n\r");
		while(raction_list[x][0]!='$'){
		    sprintf(ch->p->queryprompt+strlen(ch->p->queryprompt),
	    	    "%25s", &raction_list[x][0] );
	    	    if ( x % 3 == 0 )
               		strcat(ch->p->queryprompt, "\n\r");
	    	    x++;
		}
		strcat(ch->p->queryprompt,"
Select the action TYPE> ");
		return;
	   case 2: /*Shoe in detail*/
		strcpy(ch->p->queryprompt,"\n\rType in the action Number> ");
		ch->p->querycommand=15;
		return;
	   case 3: /*delete*/
		strcpy(ch->p->queryprompt,"\n\rEnter in the action number to delete> ");
	        ch->p->querycommand=11;
		return;
	   case 4: /*link an action to an action(fired)*/
		strcpy(ch->p->queryprompt,"\n\rEnter in the action number to that you will fired link from> ");
	        ch->p->querycommand=16;
		return;
	   case 5: /*link an action to an action(not-fired)*/
		strcpy(ch->p->queryprompt,"\n\rEnter in the action number to that you will not-fired link from> ");
	        ch->p->querycommand=18;
		return;
	   case 6:
		send_to_char("\n\r\n\r			    ROOM ACTIONS\n\r",ch);
		send_to_char(
"Action  Fired   NotFired  Chance    Method         Action-Type
---------------------------------------------------------------------\n\r",ch);
		for(number=0;number<MAX_RACTION;number++)
		   if(ractions[number]&&ractions[number]->in_room==ch->in_room){
			ra=ractions[number];
			sprintf(log_buf,"[%4d] Fn[%4d] NFn[%4d] Ch[%3d]",number,ra->fired_next,ra->notfired_next,ra->chance);
			switch(ra->method){
			    case 1:
				sprintf(log_buf+strlen(log_buf)," M[%8s] T[%s]\n\r","Player",&raction_list[(int)ra->type][0]);
			        break;
			    case 2:
				sprintf(log_buf+strlen(log_buf)," M[%8s] T[%s]\n\r","Room",&raction_list[(int)ra->type][0]);
			        break;
			    default:
				sprintf(log_buf+strlen(log_buf)," M[%8s] T[%s]\n\r","Unknown",&raction_list[(int)ra->type][0]);
			        break;
			}
			send_to_char(log_buf,ch);
		   }
		do_editroom_actions(ch,"",9);
		return;
	   case 7:/*put action in timing queue*/
		strcpy(ch->p->queryprompt,"\n\rEnter the action # to be put in the firing timing queue> ");
		ch->p->querycommand=20;
		return;
	   case 9:/*set action to autostart when game boots*/
		strcpy(ch->p->queryprompt,"\n\rEnter the action # to be started when game starts> ");
		ch->p->querycommand=22;
		return;
           case 10:
                save_room_actions(ch);
                do_editroom_actions(ch,"",9);
                return;
	   default:
		return;
	}
   }
   if(cmd==11){ /*delete helper*/
	number=atoi(buf);
	if(number<0||number>MAX_RACTION){
	    send_to_char("Number is out of range.\n\r",ch);
	    do_editroom_actions(ch,"",9);
	    return;
	}
	if(!ractions[number]){
	    send_to_char("That action does not exist.\n\r",ch);
	    do_editroom_actions(ch,"",9);
	    return;
	}
	ra=ractions[number];
	ractions[number]=NULL;
	ra->text1 = my_free(ra->text1);
	ra->text2 = my_free(ra->text2);
	ra->text3 = my_free(ra->text3);
	ra = my_free(ra);
	prt=rt=raction_seconds;
	if(rt){
	    if(rt->action==number){
		raction_seconds=rt->next;
		rt = my_free(rt);
		send_to_char("ACTION ALSO DELETED FROM TIMING QUEUE\n\r",ch);
	    }else{
		rt=prt->next;
		while(rt){
		    if(rt->action==number){
			prt->next=rt->next;
			rt = my_free(rt);
			send_to_char("ACTION ALSO DELETED FROM TIMING QUEUE\n\r",ch);
			rt=NULL;
		    }else{
			prt=rt;
			rt=prt->next;
		    }
		}
	    }
	}
        do_editroom_actions(ch,"",9);
	return;
   }
   if(cmd==13){
	number=atoi(buf);
	if(number<0||number>=MAX_TYPE_ACTION){
	    send_to_char("That action type not exist,\n\r",ch);
	    return;
	}
	if(!raction_design[number].chance){
	    send_to_char("That action type not exist, or is in development.\n\r",ch);
	    return;
	}
	ractions[ch->internal_use]->type=number;
	strcpy(ch->p->queryprompt,raction_design[number].chance);
	ch->p->querycommand=14;
	return;
   }
   if(cmd==14){
	number=atoi(buf);
	if(number<1||number>100){
	    send_to_char("The chance must be from 1 to 100.\n\r",ch);
	    return;
	}
	ractions[ch->internal_use]->chance=number;
	setup_next_prompt(ch, 
	 raction_design[(int)ractions[ch->internal_use]->type].chance_next);
	return;	
   }
   if(cmd==15){/*List action in detail*/
	number=atoi(buf);
        if(number<0||number>MAX_RACTION){
            send_to_char("Number is out of range.\n\r",ch);
	    do_editroom_actions(ch,"",9);	    
            return;
        }
        if(!ractions[number]){
            send_to_char("That action does not exist.\n\r",ch);
	    do_editroom_actions(ch,"",9);	    
            return;
        }
	send_to_char("\n\r			DETAILED ACTION DATA\n\r",ch); 
	sprintf(log_buf,"[Action #%d], Type-%s, Room-%d, Method-",number,&raction_list[(int)ractions[number]->type][0],ractions[number]->in_room);
	switch(ractions[number]->method){
	    case PLAYER:
		strcat(log_buf,"PLAYER\n\r");
		break;
	    case ROOM:
		strcat(log_buf,"ROOM\n\r");
		break;
	    default:
		strcat(log_buf,"Undefined!\n\r");
		break;
	}
	send_to_char(log_buf,ch);
	sprintf(log_buf,"Chance-%d, Arg1-%ld, Arg2-%ld, Arg3-%ld\n\r",ractions[number]->chance,ractions[number]->arg1,ractions[number]->arg2,ractions[number]->arg3);
	send_to_char(log_buf,ch);
	sprintf(log_buf,"Action to link to when fired-%d, Action to link to when not fired %d\n\r",ractions[number]->fired_next,ractions[number]->notfired_next);
	send_to_char(log_buf,ch);
	if(ractions[number]->text1){
	   send_to_char("TEXT1: ",ch);
	   send_to_char(ractions[number]->text1,ch);
	}
	if(ractions[number]->text2){
	   send_to_char("TEXT2: ",ch);
	   send_to_char(ractions[number]->text2,ch);
	}
	if(ractions[number]->text3){
	   send_to_char("TEXT3: ",ch);
	   send_to_char(ractions[number]->text3,ch);
	}
	do_editroom_actions(ch,"",9);
	return;
   }
   if(cmd==16){
        number=atoi(buf);
        if(number<0||number>MAX_RACTION){
            send_to_char("Number is out of range.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
        if(!ractions[number]){
            send_to_char("That action does not exist.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
	ch->internal_use=number;
	strcpy(ch->p->queryprompt,"\n\rEnter the action to fired link TO> ");
	ch->p->querycommand=17;
	return;
   }
   if(cmd==17){
        number=atoi(buf);
        if(number<0||number>MAX_RACTION){
            send_to_char("Number is out of range.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
        if(!ractions[number]){
            send_to_char("That action does not exist.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
	sprintf(log_buf,"Action[%d] fired linked to Action[%d]\n\r",ch->internal_use,number);
	ractions[ch->internal_use]->fired_next=number;
	do_editroom_actions(ch,"",9);
        return;
   }
   if(cmd==18){
        number=atoi(buf);
        if(number<0||number>MAX_RACTION){
            send_to_char("Number is out of range.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
        if(!ractions[number]){
            send_to_char("That action does not exist.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
	ch->internal_use=number;
	strcpy(ch->p->queryprompt,"\n\rEnter the action to bot-fired link TO> ");
	ch->p->querycommand=19;
	return;
   }
   if(cmd==19){
        number=atoi(buf);
        if(number<0||number>MAX_RACTION){
            send_to_char("Number is out of range.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
        if(!ractions[number]){
            send_to_char("That action does not exist.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
	sprintf(log_buf,"Action[%d] not-fired linked to Action[%d]\n\r",ch->internal_use,number);
	ractions[ch->internal_use]->notfired_next=number;
	do_editroom_actions(ch,"",9);
        return;
   }
   if(cmd==20){
        number=atoi(buf);
        if(number<0||number>MAX_RACTION){
            send_to_char("Number is out of range.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
        if(!ractions[number]){
            send_to_char("That action does not exist.\n\r",ch);
            do_editroom_actions(ch,"",9);
            return;
        }
	ch->internal_use=number;
	strcpy(ch->p->queryprompt,"\n\rEnter in the number of seconds from now to fire it> ");
	ch->p->querycommand=21;
	return;
   }
   if(cmd==21){/*This actually places the action in timing Q*/
        number=atoi(buf);
	CREATE(rt, struct raction_timing, 1);
	rt->action=ch->internal_use;
	rt->next=NULL;
	time(&l);
	l+=number;
	rt->time=l;
	number=0;
	if(!raction_seconds)
	   raction_seconds=rt;
	else{
	   crt=raction_seconds;
	   if(l<=crt->time){
		rt->next=crt;
		raction_seconds=rt;
	   }else{
		number++;
		prt=crt;
		crt=prt->next;
		while(crt){
		   if(l<=crt->time){
			rt->next=crt;
			prt->next=rt;
			break;
		   }else{
			number++;
			prt=crt;
			crt=prt->next;
		   }
		}
		if(!crt)
		   prt->next=rt;
	   }
	}
	sprintf(log_buf,"\n\rAction[%d] was sorted by time into Q at slot [%d].\n\r",ch->internal_use,number);
	send_to_char(log_buf,ch);
	do_editroom_actions(ch,"",9);
	return;
   }
   if(cmd==22){
	number=atoi(buf);
        if(number<0||number>MAX_RACTION){
            send_to_char("Number is out of range.\n\r",ch);
	    do_editroom_actions(ch,"",9);	    
            return;
        }
        if(!ractions[number]){
            send_to_char("That action does not exist.\n\r",ch);
	    do_editroom_actions(ch,"",9);	    
            return;
	}
	send_to_char("Done.\n\r",ch);
	ractions[number]->autostart=1;
	do_editroom_actions(ch,"",9);
	return;
   }
   if(cmd==ARG1){
	if(!buf[0])return;
	l=atol(buf);
	ractions[ch->internal_use]->arg1=l;
	setup_next_prompt(ch, 
	 raction_design[(int)ractions[ch->internal_use]->type].arg1_next);
	return;	
   }
   if(cmd==ARG2){
	if(!buf[0])return;
	l=atol(buf);
	ractions[ch->internal_use]->arg2=l;
	setup_next_prompt(ch, 
	 raction_design[(int)ractions[ch->internal_use]->type].arg2_next);
	return;	
   }
   if(cmd==ARG3){
	if(!buf[0])return;
	l=atol(buf);
	ractions[ch->internal_use]->arg3=l;
	setup_next_prompt(ch, 
	 raction_design[(int)ractions[ch->internal_use]->type].arg3_next);
	return;	
   }
   if(cmd==METHOD){
	if(!buf[0])return;
	l=atol(buf);
	ractions[ch->internal_use]->method=(char)l;
	setup_next_prompt(ch, 
	 raction_design[(int)ractions[ch->internal_use]->type].method_next);
	return;	
   }
   if(cmd==TEXT1){
	setup_next_prompt(ch, 
	 raction_design[(int)ractions[ch->internal_use]->type].text1_next);
	return;
   }
   if(cmd==TEXT2){
	setup_next_prompt(ch, 
	 raction_design[(int)ractions[ch->internal_use]->type].text2_next);
	return;
   }
   if(cmd==TEXT3){
	setup_next_prompt(ch, 
	 raction_design[(int)ractions[ch->internal_use]->type].text3_next);
	return;
   }
}


/*			    FIRE_RACTION				*/
/* fire_raction starts(fires up) an action at its most advanced state.
The r in raction means ROOM. o(object) & m(mobile) will follow.
It looks to see the actions method, and rolls the die to see if that
action will fire(if the action even needs a die throw).  It then nodes
off and fires subsequent actions calling itself(recursive) depending on
if it fired or not, calling the proper action.  Sorta like traveling 
down a tree and turning at the proper branches.  It does another
important thing, it checks to see if it should start action once for the
room as a whole, or roll and fire/notfire for each player in room.
It then makes sure after a roll_per_player scenario that further down that
path on the tree it will not do a major branch and refire an action
for each player in room for that single action.  That could cause some
possible looping feedback and hang the game.

It ends up calling more primitive function do_raction which actually does
the action.  The tree if imagined would end up looking like 
BRANCHES(fire_raction) and dead end leaves(do_action), with one
leaf sticking out of every fork in the branch*/

void fire_raction(int room, struct char_data *ch, int action_num)
{
struct room_actions *action=NULL;
struct char_data *player=NULL;

    if(!world[room])return; /*better safe than sorry*/
    action=ractions[action_num];
    if(!action)return; /*should never happen*/
    if(ch){ /*Disallow possible feedback */
       if(action->chance){
	   if(number(1,100)>action->chance){ 
		  /*didn't happen*/
	        if(action->notfired_next)
		   fire_raction(room,ch,action->notfired_next);
		return;
	   }
       }
               /*Its gonna happen*/
       do_raction(room,ch,action_num);	
       if(action->fired_next)
	   fire_raction(room,ch,action->fired_next);
       return;
    }
    /*METHOD ROOM: One chance roll to see if action fires if so it call*/
    /*the do_action and that should see that its a room method and do  */
    /*its deal to everyone.  Different actions will deal with this     */
    /*in different ways, many don't even care if a player even exists  */
    /*in the room.  Others may echo to room, while others may have     */
    /*text1,2&3 setup for to_room,to_char,to_vict and use act function.*/
    /*Some actions may even use arg1,2or3 in the OE editing to querry  */
    /*user as how to deal with method room.  For instance action       */
    /*load_obj may ask(should we just load one or one per player in the*/
    /*room), then set up arg2 and deal with it in do_action.  Point    */
    /*being that the creator of actions in OE should just believe the  */
    /*and have faith in the code to ask the proper questions per action*/
    /*depending on his answers so far.				       */
      
    if(IS_SET(action->method,ROOM)){
       if(action->chance){
	   if(number(1,100)>action->chance){ 
		  /*didn't happen*/
	        if(action->notfired_next)
		   fire_raction(room,0,action->notfired_next);
		return;
	   }
       }
               /*Its gonna happen*/
       do_raction(room,NULL,action_num);	
       if(action->fired_next)
	   fire_raction(room,0,action->fired_next);
       return;
    }
    /*METHOD PLAYER: Here we roll one chance of firing roll per player*/
    /*and the whole thing branches off in different directions for    */
    /*each player.  Like in a avalanche, some players may get hit by a*/
    /*boulder which calls other actions that send him tumbling down   */
    /*the mountain into different rooms and sustain damage and maybe  */
    /*making him wait to recover, while other players are missed and  */
    /*simply get some scratches.  There are no limits, in fact the    */
    /*avalanche itself could have a possibility of starting another   */
    /*avalanche further down the mountain, making climbing the thing  */
    /*at lower altitudes dangerous when the zone is busy.  You must   */
    /*remember that once an action sequence hits a METHOD PLAYER type,*/
    /*that all remaining actions down that path for each player will  */
    /*be of a singular variety and can no longer use a binary logic   */
    /*action like (have this?) to fire up another sequence to branch  */
    /*again for each player in room.  SIMPLE RIGHT? Mike ducks.       */

    if(IS_SET(action->method,PLAYER)){
	for(player=world[room]->people;player;player=player->next_in_room){
	    if(player){
               if(action->chance){
	           if(number(1,100)>action->chance){ 
		          /*didn't happen*/
	               if(action->notfired_next)
		          fire_raction(room,player,action->notfired_next);
	           }
               }
                       /*Its gonna happen*/
               do_raction(room,player,action_num);
	       if(action->fired_next)
		   fire_raction(room,player,action->fired_next);	
	    }
	}
 	return;
    }
}
void name_string(char *buf1, char *buf2)
{
int x=0;
    strcpy(buf1,buf2);
    while(buf1[x]){
	if(buf1[x]=='%')
	    buf1[x]='$';
	x++;
    }
}

void action_chatter(int room, struct char_data *ch, struct room_actions *ra)
{
char buf[MAX_STRING_LENGTH];

    if(ra->arg1)
	room=ra->arg1;
    if(!ch){
	send_to_room(ra->text1,room);
    }else{
    	name_string(buf,ra->text1);    
	act(buf,TRUE,ch,0,0,TO_ROOM);
	name_string(buf,ra->text2);
	act(buf,TRUE,ch,0,0,TO_CHAR);
    }
}
void action_zone_chatter(int room, struct char_data *ch, struct room_actions *ra) 
{

struct descriptor_data *p=NULL;

    for(p=descriptor_list;p;p=p->next)
    	if(p->character&&p->character->in_room>0
	&&world[p->character->in_room]->zone==ra->arg1)
	    send_to_char(ra->text1,p->character);
    
}

void action_make_exit(int room, struct char_data *ch, struct room_actions *ra) 
{
char t[10];
     
    switch(ra->arg2){
	case 0:
	    sprintf(t,"n %ld", ra->arg3);
	    break;
	case 1:
	    sprintf(t,"e %ld", ra->arg3);
	    break;
	case 2:
	    sprintf(t,"s %ld", ra->arg3);
	    break;
	case 3:
	    sprintf(t,"w %ld", ra->arg3);
	    break;
	case 4:
	    sprintf(t,"u %ld", ra->arg3);
	    break;
	case 5:
	    sprintf(t,"d %ld", ra->arg3);
	    break;
	default:
	    log_hd("##Bad direction in action_make_exit");
	    return;
    }
    do_makedoor(0,t,ra->arg1);
}

void action_delete_exit(int room, struct char_data *ch, struct room_actions *ra) 
{
char t[10];

    switch(ra->arg2){
	case 0:
	    strcpy(t,"n");
	    break;
	case 1:
	    strcpy(t,"e");
	    break;
	case 2:
	    strcpy(t,"s");
	    break;
	case 3:
	    strcpy(t,"w");
	    break;
	case 4:
	    strcpy(t,"u");
	    break;
	case 5:
	    strcpy(t,"d");
	    break;
	default:
	    log_hd("##Bad direction in action_delete_exit");
	    return;
    }
    do_deletedoor(0,t,ra->arg1);
}

void action_set_action_start(int room, struct char_data *ch, struct room_actions *ra)
{
    struct raction_timing *rt=NULL,*prt=NULL,*crt=NULL;
    long l;
    int n;
    if(!ractions[ra->arg1])
	return;
    if(ra->arg3<=ra->arg2)
	ra->arg3=0;
    if(!ra->arg3){ /*no randomness*/
	time(&l);
	l+=ra->arg2;
    }else{
	n=number((int)ra->arg2,(int)ra->arg3);
	time(&l);
	l+=n;
    }
	CREATE(rt, struct raction_timing, 1);
	rt->action=(int)ra->arg1;;
	rt->next=NULL;
	rt->time=l;
	if(!raction_seconds)
	   raction_seconds=rt;
	else{
	   crt=raction_seconds;
	   if(l<=crt->time){
		rt->next=crt;
		raction_seconds=rt;
	   }else{
		prt=crt;
		crt=prt->next;
		while(crt){
		   if(l<=crt->time){
			rt->next=crt;
			prt->next=rt;
			break;
		   }else{
			prt=crt;
			crt=prt->next;
		   }
		}
		if(!crt)
		   prt->next=rt;
	   }
	}
}
void do_raction(int room, struct char_data *ch, int action_num)
{
	switch(ractions[action_num]->type){
	    case 0:
		action_chatter(room, ch, ractions[action_num]);
		break;
	    case 3:
		action_set_action_start(room, ch, ractions[action_num]);
		break;
	    case 20:
		action_make_exit(room, ch, ractions[action_num]);
		break;
	    case 21:
		action_delete_exit(room, ch, ractions[action_num]);
		break;
	    case 22:
		action_zone_chatter(room, ch, ractions[action_num]);
		break;
	    default:
		break;
	}
}

