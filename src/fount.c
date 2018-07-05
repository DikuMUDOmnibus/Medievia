/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/


#include <strings.h>
#include <stdio.h>
#include <ctype.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "db.h"

extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
int real_object(int i);

/*************************************************************************
* Figures out if a fountain is present in the room                       *
*************************************************************************/

int FOUNTAINisPresent (struct char_data *ch)
{
  struct obj_data *tmp=NULL;
  bool found = FALSE;
  tmp=NULL;

  for (tmp = world[ch->in_room]->contents;
       tmp != NULL && !found;
       tmp = tmp->next_content) {
    if (((tmp->item_number == real_object(5220)) ||
     (tmp->item_number == real_object(16920)) ||
     (tmp->item_number == real_object(3135)) ||
     (tmp->item_number == real_object(7102)) ||
     (tmp->item_number == real_object(1200)) ||
     (tmp->item_number == real_object(15017)) ||
     (tmp->item_number == real_object(18042)) ||
     (tmp->item_number == real_object(6327)) ||
     (tmp->item_number == real_object(1300))) &&
    (CAN_SEE_OBJ(ch, tmp))) {
      found = TRUE;
    }
  }

  return found;

}

void do_fill(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  struct obj_data *to_obj=NULL;
  int amount;
  void name_to_drinkcon(struct obj_data *obj, int type);
  struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, \
		       struct obj_data *list);
  
  	one_argument(argument, buf);
  	if(!*buf){
      	act("What do you want to fill?",FALSE,ch,0,0,TO_CHAR);
     	return;
  	}
  	if(!(to_obj = get_obj_in_list_vis(ch,buf,ch->carrying))){
      	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      	return;
    }
  	if(FOUNTAINisPresent(ch)||IS_SET(world[ch->in_room]->room_flags,DRINKROOM)) {
    	if(to_obj->obj_flags.type_flag!=ITEM_DRINKCON){
    		act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
    		return;
      	}
    	if((to_obj->obj_flags.value[1]!=0)&&(to_obj->obj_flags.value[2]!=0)){
    		act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
    		return;
     	}
    	if(!(to_obj->obj_flags.value[1]<to_obj->obj_flags.value[0])){
    		act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
    		return;
      	}
    	act ("You fill $p!",TRUE,ch,to_obj,0,TO_CHAR);
    	/* First same type liq. */
    	to_obj->obj_flags.value[2]=0;
    	/* Then how much to pour */
    	amount= to_obj->obj_flags.value[0]-to_obj->obj_flags.value[1];
    	to_obj->obj_flags.value[1]=to_obj->obj_flags.value[0];
  	} else {
    	act("You realize there is no fountain or stream around.",FALSE,ch,0,0,TO_CHAR);
  	}
  return;
}
