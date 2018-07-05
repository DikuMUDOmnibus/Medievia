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
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <dirent.h>
/*#include <limits.h>*/
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
struct obj_data *DA_paper;

struct obj_data *make_newspaper(void)
{
FILE *fhn;
DIR *dp; struct dirent *dirp; 
struct obj_data *obj=NULL;
struct extra_descr_data *new_descr=NULL;
char filename[200],keyword[100],*ptr;

        obj=read_object(1973,0);
        if((fhn=fopen("../newspaper/current", "r"))==NULL){
            return(obj);
        }
        open_files++;
	if(obj->action_description!=objs[obj->item_number]->action_description)
            obj->action_description = my_free(obj->action_description);
        obj->action_description=fread_string(fhn);
        fclose(fhn);
        open_files--;

        if((dp=opendir("../newspaper"))!=NULL){
            while((dirp=readdir(dp))!=NULL){
                if(!strcmp(dirp->d_name,".")||!strcmp(dirp->d_name,".."))
                    continue; 
		if(!strstr(dirp->d_name, "current_"))
		    continue;
		sprintf(filename,"../newspaper/%s",dirp->d_name);
        	if((fhn=fopen(filename, "r"))==NULL){
		    sprintf(log_buf,"##cannot open %s",filename);
		    log_hd(log_buf);
            	    return(NULL);
        	}
		ptr=strstr(dirp->d_name,"_");
		ptr++;
		strcpy(keyword,ptr);
	        CREATE(new_descr, struct extra_descr_data, 1);
        	new_descr->keyword      = strdup(keyword);
        	new_descr->description  = fread_string(fhn);
        	new_descr->next         = obj->ex_description;
        	obj->ex_description     = new_descr;
		fclose(fhn);
            }   
            closedir(dp); 
        }else{
	    log_hd("## Cannot open the directory NEWSPAPER");
	    return(NULL);
        }
        return(obj);
}

struct obj_data *get_a_copy(void)
{
struct obj_data *obj;
struct extra_descr_data *e=NULL, *n=NULL;

    obj=read_object(1973,0);
    if(obj->action_description!=objs[obj->item_number]->action_description)
        obj->action_description = my_free(obj->action_description);
    obj->action_description=DA_paper->action_description;
    e=DA_paper->ex_description;
    while(e){
	CREATE(n, struct extra_descr_data, 1);
	n->keyword=e->keyword;
	n->description=e->description;
	n->next=obj->ex_description;
	obj->ex_description=n;
	e=e->next;
    }
    return(obj);
}

int paper_box(struct char_data *ch, int cmd, char *arg)
{
struct obj_data *obj=NULL;

   if(cmd!=56)return(0);
   if(GET_GOLD(ch)<5){  
        send_to_char("You try and dig out 5 coins but come up short...\n\r",ch);
        return(1);
   }
   GET_GOLD(ch)-=5;
   obj=get_a_copy();
   if(!obj){
        send_to_char("Sorry all out of papers.\n\r",ch);
        return(1);
   }
   act("$n puts some coins in the box and takes a newspaper.", TRUE,ch, 0, 0,TO_ROOM);
   send_to_char("You put 5 coins in the box and grab the latest edition.\n\r",ch);
   obj_to_char(obj,ch);
   return(1);
 }

