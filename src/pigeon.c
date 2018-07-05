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
extern struct descriptor_data *descriptor_list;  
extern int number(int from, int to); 
extern char global_color;

#define MESSAGE_OBJ		1204
#define SOCIAL_ZONE_SPEED 	2
#define PIGEON_SPEED		55
/* ( 5.5miles/second ) */

struct pigeon_message_data {
	char *szpMessage;
	char *szpSender;
	char *szpReceiver;
	int  iTransitTime;
	int  iTotalTime;
	struct pigeon_message_data *next;
};

struct pigeon_message_data *gstpPigeonHeadList=NULL;

void Free_message(struct pigeon_message_data *stpMes)
{
	if(!stpMes) {
		log_hd("## NULL sent to stpFreePigeonMessage!!");
		return;
	}
	if(stpMes->szpSender)
		stpMes->szpSender = my_free(stpMes->szpSender);
	if(stpMes->szpReceiver)
		stpMes->szpReceiver = my_free(stpMes->szpReceiver);
	if(stpMes->szpMessage)
		stpMes->szpMessage = my_free(stpMes->szpMessage);
		
	stpMes->iTransitTime = 0;
	stpMes->iTotalTime = 0;
	stpMes->next = NULL;
	
	my_free(stpMes);
	
}

void do_pigeon(struct char_data *stpCh, char *szpArg, int iCmd)
{
 char Buf1[MAX_INPUT_LENGTH], Buf2[MAX_INPUT_LENGTH];
 struct obj_data *stpPaper=NULL;
 struct char_data *stpVict=NULL;
 struct pigeon_message_data *stpMes=NULL;
 extern int iHoloChDistance(struct char_data *ch, struct char_data *vict);
 
 szpArg = one_argument(szpArg, Buf1);
 one_argument(szpArg, Buf2);
 
 	if(!*Buf1) {
 		send_to_char("Syntax: message <paper> <to whom?>\r\n",stpCh);
 		return;
 	}
 	if(!*Buf2) {
 		send_to_char("Who are you sending this message to??\r\n",stpCh);
 		return;
 	}
 	stpPaper = get_obj_in_list_vis(stpCh, Buf1 ,stpCh->carrying);

	if(!stpPaper) {
 		send_to_char("I can't find that item in your inventory.\r\n",stpCh);
 		return;
	}
 	if(stpPaper->obj_flags.type_flag != ITEM_NOTE) {
	 	send_to_char("That's not a note.\r\n",stpCh);
 		return;
 	}
 	if(!stpPaper->action_description || strlen(stpPaper->action_description)<4) {
		send_to_char("There is nothing written on that page.\r\n",stpCh);
		return;
	}
 	stpVict = get_char_vis(stpCh,Buf2);
 	
 	if(!stpVict) {
 		send_to_char("No such person in the game currently.\r\n",stpCh);
 		return;
 	}
 	if(IS_NPC(stpVict)) {
 		send_to_char("You can not send messages to monsters!\r\n",stpCh);
 		return;
 	}
 	CREATE(stpMes, struct pigeon_message_data, 1);
	stpMes->szpMessage = str_dup(stpPaper->action_description);
	stpMes->szpSender = str_dup(GET_NAME(stpCh));
	stpMes->szpReceiver = str_dup(GET_NAME(stpVict));
	stpMes->iTransitTime = 0;
	if(world[stpVict->in_room]->zone == 180)
		stpMes->iTotalTime = SOCIAL_ZONE_SPEED;
	else
		stpMes->iTotalTime = iHoloChDistance(stpCh, stpVict) / PIGEON_SPEED;
	stpMes->next = gstpPigeonHeadList;
	gstpPigeonHeadList = stpMes;

	obj_from_char(stpPaper);
	extract_obj(stpPaper);
	global_color=36;
	send_to_char("A small imp appears in a puff of smoke and takes your message.\r\n",stpCh);
	act("
\rA billowing cloud of purple smoke seeps from the ground and a small imp 
\rappears. It snatches a message from $n's hand and disappears with the 
\rfading smoke.",FALSE,stpCh,0,0,TO_ROOM);
	global_color=0;
	return;
}

	 	
	
 		
void deliver_message(struct pigeon_message_data *stpMes, struct char_data *stpCh)
{
	struct obj_data *stpPaper;
	char buf[MAX_INPUT_LENGTH];
	
	if(!stpMes || !stpCh) {
		log_hd("## Null Pointer sent to deliver_message!");
		return;
	}
	if(!stpMes->szpMessage || !stpMes->szpSender) {
		log_hd("## Bad Message or sender in deliver_message");
		return;
	}
	stpPaper = read_object(MESSAGE_OBJ, 0);
	stpPaper->action_description = str_dup(stpMes->szpMessage);
	sprintf(buf, "A small, rolled-up note from %s",stpMes->szpSender);
	stpPaper->short_description = str_dup(buf);
	global_color=36;
	act("
\rA billowing cloud of purple smoke seeps up through the ground.  A small
\rimp appears and give a rolled piece of parchment to $n.  It then vanishes
\rwith the fading cloud of smoke.",FALSE,stpCh,0,0,TO_ROOM);
	send_to_char("An imp appears and gives you a message.\r\n",stpCh);
	global_color=0;
	obj_to_char(stpPaper,stpCh);	
	return;
}	
		

void PigeonMessageUpdate(void)
{
	struct pigeon_message_data *stpTemp,*temp,*stpNext;
	struct char_data *stpVict=NULL;
	struct char_data *get_player(char *name);
		
	for(stpTemp = gstpPigeonHeadList; stpTemp; stpTemp=stpNext) {
		stpNext = stpTemp->next;
		if( ++stpTemp->iTransitTime >= stpTemp->iTotalTime ) {
		    if(!stpTemp->szpReceiver) {
		    	log_hd("### Reciever name lost in PigeonMessageUpdate!");
		    	continue;
		    }
			stpVict = get_player(stpTemp->szpReceiver);
			if(stpVict == NULL) {
					REMOVE_FROM_LIST(stpTemp, gstpPigeonHeadList, next);
					Free_message(stpTemp);
					continue;
			}
			deliver_message(stpTemp, stpVict);
			REMOVE_FROM_LIST(stpTemp, gstpPigeonHeadList, next);
			Free_message(stpTemp);
		}
	}
}
			
