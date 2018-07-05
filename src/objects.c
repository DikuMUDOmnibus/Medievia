/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause      *	
*       		   All rights reserved				   *
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
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

extern int VERSIONNUMBER;
extern char global_color;
int weigh(struct obj_data *obj);
void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode,int amount);

int bIsBetter(struct char_data *stpCh, struct obj_data *stpObj)
{
int iWeightToBeat,iBestWeight=0;
struct obj_data *stpCarrying;

	if(stpObj->carried_by==stpCh||stpObj->worn_by==stpCh)
		return(0);
	if(stpObj->carried_by||stpObj->worn_by)
		return(0);
	
	if(stpObj->obj_flags.wear_flags<=1)
		return(0);
	if(stpObj->obj_flags.type_flag==ITEM_NOTE)
		return(0);
	if(GET_LEVEL(stpCh)<stpObj->obj_flags.eq_level)
		return(0);
	if(!stpObj->iValueWeight||stpObj->iWeightVersion<VERSIONNUMBER){
		stpObj->iValueWeight=weigh(stpObj);
		stpObj->iWeightVersion=VERSIONNUMBER;
	}
	iWeightToBeat=(stpObj->iValueWeight+(stpObj->iValueWeight/12));
	for(stpCarrying=stpCh->carrying;stpCarrying;stpCarrying=stpCarrying->next_content){
		if(stpCarrying->obj_flags.wear_flags==stpObj->obj_flags.wear_flags){
			if(stpCarrying->iValueWeight>iBestWeight)
				iBestWeight=stpCarrying->iValueWeight;
		}
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_FINGER)){
		if(stpCh->equipment[WEAR_FINGER_R]&&stpCh->equipment[WEAR_FINGER_R]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_FINGER_R]->iValueWeight;				
		if(stpCh->equipment[WEAR_FINGER_L]&&stpCh->equipment[WEAR_FINGER_L]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_FINGER_L]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_NECK)){
		if(stpCh->equipment[WEAR_NECK_1]&&stpCh->equipment[WEAR_NECK_1]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_NECK_1]->iValueWeight;				
		if(stpCh->equipment[WEAR_NECK_2]&&stpCh->equipment[WEAR_NECK_2]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_NECK_2]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_BODY)){
		if(stpCh->equipment[WEAR_BODY]&&stpCh->equipment[WEAR_BODY]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_BODY]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_HEAD)){
		if(stpCh->equipment[WEAR_HEAD]&&stpCh->equipment[WEAR_HEAD]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_HEAD]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_LEGS)){
		if(stpCh->equipment[WEAR_LEGS]&&stpCh->equipment[WEAR_LEGS]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_LEGS]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_FEET)){
		if(stpCh->equipment[WEAR_FEET]&&stpCh->equipment[WEAR_FEET]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_FEET]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_HANDS)){
		if(stpCh->equipment[WEAR_HANDS]&&stpCh->equipment[WEAR_HANDS]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_HANDS]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_ARMS)){
		if(stpCh->equipment[WEAR_ARMS]&&stpCh->equipment[WEAR_ARMS]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_ARMS]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_SHIELD)){
		if(stpCh->equipment[WEAR_SHIELD]&&stpCh->equipment[WEAR_SHIELD]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_SHIELD]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_ABOUT)){
		if(stpCh->equipment[WEAR_ABOUT]&&stpCh->equipment[WEAR_ABOUT]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_ABOUT]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_WAISTE)){
		if(stpCh->equipment[WEAR_WAISTE]&&stpCh->equipment[WEAR_WAISTE]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_WAISTE]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WEAR_WRIST)){
		if(stpCh->equipment[WEAR_WRIST_R]&&stpCh->equipment[WEAR_WRIST_R]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_WRIST_R]->iValueWeight;				
		if(stpCh->equipment[WEAR_WRIST_L]&&stpCh->equipment[WEAR_WRIST_L]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WEAR_WRIST_L]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_WIELD)){
		if(stpCh->equipment[WIELD]&&stpCh->equipment[WIELD]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[WIELD]->iValueWeight;				
	}
	if(IS_SET(stpObj->obj_flags.wear_flags,ITEM_HOLD)){
		if(stpCh->equipment[HOLD]&&stpCh->equipment[HOLD]->iValueWeight>iBestWeight)
			iBestWeight=stpCh->equipment[HOLD]->iValueWeight;				
	}
	if(iBestWeight<=iWeightToBeat){
		return(1);
	}else{
		return(0);
	}
}


void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode, bool show) 
{
struct obj_data *i=NULL,*prev=0;
int found,amount=0;

	global_color=36;
    found=FALSE;
    for(i=list;i;i=i->next_content){ 
		if (CAN_SEE_OBJ(ch,i)){
	    	if(!prev||(i->item_number!=prev->item_number)){
				if(prev)
	            	show_obj_to_char(prev, ch, mode,amount);
				prev=i;
				amount=0;
	    	}
	    	found = TRUE;
	    	amount++;
		}    
    }  
    if(prev)
	 	show_obj_to_char(prev, ch, mode,amount);
    if((!found)&&(show)) 
    	send_to_char("Nothing\n\r", ch);
    global_color=0;
}

void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode,int amount) 
{
char buffer[MAX_STRING_LENGTH];
int found, iBetter;

    buffer[0] = '\0';
	iBetter=bIsBetter(ch,object);
	if((iBetter==1)&&(mode==0||mode==1||mode==2||mode==3||mode==4)){
		global_color=31;
		send_to_char("-->",ch);
		global_color=0;	
	}
    if((mode==0)&&object->description){
		if(amount>1)
	    	sprintf(buffer,"(%d) %s",amount,object->description);
		else
	    	strcpy(buffer,object->description);
    }else if (object->short_description && ((mode == 1) ||
	  			(mode == 2) || (mode==3) || (mode == 4))) {
		if(amount>1)
	    	sprintf(buffer,"(%d) %s",amount,object->short_description);
		else
	    	strcpy(buffer,object->short_description);
    }else if(mode == 5) {
		if (object->obj_flags.type_flag == ITEM_NOTE){
	    	if(  object->action_description&&strlen(object->action_description)>5){
				strcpy(buffer, "There is something written upon it:\n\r\n\r");
				strcat(buffer, object->action_description);
	        	global_color=32;
				page_string(ch->desc, buffer, 1);
				global_color=0;
	    	}else
				act("It's blank.", FALSE, ch,0,0,TO_CHAR);
	    	return;
		}
		else if((object->obj_flags.type_flag != ITEM_DRINKCON)){
	    	strcpy(buffer,"You see nothing special.");
		}else /* ITEM_TYPE == ITEM_DRINKCON */{
	    	strcpy(buffer, "It looks like a drink container.");
		}
    }
    if (mode != 3) { 
		found = FALSE;
		if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
	     	strcat(buffer,"(invisible)");
	     	found = TRUE;
		}
		if (IS_OBJ_STAT(object,ITEM_EVIL) && IS_AFFECTED(ch,AFF_DETECT_EVIL)) {
	     	strcat(buffer,"..It glows red!");
	     	found = TRUE;
		}
		if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
	     	strcat(buffer,"..It glows blue!");
	     	found = TRUE;
		}
		if (IS_OBJ_STAT(object,ITEM_GLOW)) {
	    	strcat(buffer,"..It has a soft glowing aura!");
	    	found = TRUE;
		}
		if (IS_OBJ_STAT(object,ITEM_HUM)) {
	    	strcat(buffer,"..It emits a faint humming sound!");
	    	found = TRUE;
		}
    }
    strcat(buffer, "\n\r");
    global_color=1;
    page_string(ch->desc, buffer, 1);
    global_color=0;
}
