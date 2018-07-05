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
extern bool in_a_shop(struct char_data *ch);

void tweak_corpse(struct obj_data *corpse, struct char_data *mob)
{
int x,y;
struct obj_data *obj=NULL;

   if(in_a_shop(mob))return;


   if(GET_CLASS(mob) == CLASS_ANIMAL) return;
   if(GET_CLASS(mob) == CLASS_OTHER) return;
           /*load greens*/
    if(number(1,100)<GET_LEVEL(mob)){
	y=number(1,GET_LEVEL(mob)/5);
	for(x=0;x<y;x++){
	   obj=read_object(1368,0);
	   obj_to_obj(obj,corpse);	
	}
    }

	/* misty */
    if(number(1,100)<7){
	obj=read_object(2015,0);
	obj_to_obj(obj,corpse);
    }

	/* identify */
    if(number(1,100)<5){
	obj=read_object(3040,0);
	obj_to_obj(obj,corpse);
    }

	/* sanc orbs */
    if(number(1,100)<3){
	obj=read_object(1411,0);
	obj_to_obj(obj,corpse);
    }

}
