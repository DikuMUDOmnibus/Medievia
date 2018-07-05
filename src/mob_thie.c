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


/*                  /External variables and functions\                    */
/*------------------------------------------------------------------------*/
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
extern int top_of_mobt;  
extern int top_of_objt; 
extern struct descriptor_data *descriptor_list;  
extern int dice(int number, int size);  
extern int number(int from, int to); 
extern void space_to_underline(char *text);
extern void page_string(struct descriptor_data *d, char *str, int keep_internal); 
extern int number_of_rooms;
extern char free_error[100];
extern struct char_data *pick_victim(struct char_data *ch); 
extern void wear(struct char_data *ch, struct obj_data *obj_object, int keyword);
extern bool IS_IN_FRONT(struct char_data *ch);

void thief_combat(struct char_data *ch)
{
struct char_data *m=NULL;
struct obj_data *wobj=NULL,*obj=NULL;

	if(!ch->specials.fighting)
		return;
	m = pick_victim(ch->specials.fighting); 
	if(!m)
		return;	

    for(wobj=ch->carrying;wobj;wobj=wobj->next_content)
	    if(IS_SET(wobj->obj_flags.wear_flags, ITEM_THROW)){
			do_throw(ch, GET_NAME(m), 9);
		return;
	}


    for(obj=world[ch->in_room]->contents;obj;obj=obj->next_content){
		if(IS_SET(obj->obj_flags.wear_flags,ITEM_THROW)){
	    	obj_from_room(obj);
	    	obj_to_char(obj,ch);
	    	act("$n gets $p.", 1, ch, obj, 0, TO_ROOM);
	    	return;
		}
    }

    if(IS_IN_FRONT(ch)){
		if(number(1,130)>100) return;
		if(number(1,100)<50)
	    	do_trip(ch,GET_NAME(m),9);
	else
	    do_disarm(ch,GET_NAME(m),9);
    }
}


void thief_noncombat(struct char_data *ch)
{
if (!IS_AFFECTED(ch, AFF_HIDE))
	{
	SET_BIT(ch->specials.affected_by, AFF_HIDE);
	return;
    }
if (!IS_AFFECTED(ch, AFF_SNEAK))
	{
	do_sneak(ch,"",9);
	return;
    }
do_scan(ch,"",9);
}
