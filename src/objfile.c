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

extern struct char_data *character_list;
extern struct zone_data *zone_table;
extern FILE *hold2,*obj_f;
extern struct obj_data  *objs[MAX_OBJ];
extern struct index_data obj_index_array[MAX_OBJ];
extern int VERSIONNUMBER;
extern struct obj_data *DA_paper;
struct char_data *mobs[MAX_MOB];

extern long int liGetNumber(FILE *new, char *szpTag);
extern void add_object(struct obj_data *obj);
extern struct index_data *obj_index;
extern int weigh(struct obj_data *obj);
extern void write_filtered_text(FILE *fh, char *text);

/* read an object from OBJ_FILE */
struct obj_data *load_object(int nr, int eq_level, FILE *new)
{
struct obj_data *obj=NULL;
int tmp, iAffs=0;
char szTag[81];
struct extra_descr_data *new_descr=NULL;

    if(new){
        if (!(obj_f = fopen(OBJ_FILE, "r"))){
	    	perror( OBJ_FILE );
			SUICIDE;
    	}
		open_files++;
    }
    fseek(obj_f,obj_index[nr].pos,0);
    CREATE(obj, struct obj_data, 1);
    clear_object(obj);
    obj->ex_description = NULL;
    obj->iDetLife = 8035200; /* trois mois a le jour */

	if(new)
		fprintf(new,"#%d\n",nr);
    sprintf(log_buf,"Loading object#%d",nr);
    obj->name			= fread_string(obj_f);
	if(new){
		write_filtered_text(new,obj->name);
	}
    obj->short_description	= fread_string(obj_f);
	if(new)
		write_filtered_text(new,obj->short_description);
    obj->description		= fread_string(obj_f);
	if(new)
		write_filtered_text(new,obj->description);
    obj->action_description	= fread_string(obj_f);
	if(new)
		write_filtered_text(new,obj->action_description);


/****** READ IN VARIABLES TAG/VALUE PAIRS *******/
	do{
		fscanf(obj_f," %s ",szTag);
		if(!strncasecmp(szTag,"TYP",3)){
		    fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.type_flag	= tmp;
			if(new)
				fprintf(new,"TYP %d ",tmp);
		}else if(!strncasecmp(szTag,"EXT",3)){
		    fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.extra_flags	= tmp;
			if(new)
				fprintf(new,"EXT %d ",tmp);
		}else if(!strncasecmp(szTag,"WEA",3)){
		    fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.wear_flags	= tmp;
			if(new)
				fprintf(new,"WEA %d\n",tmp);
		}else if(!strncasecmp(szTag,"VAL",3)){
		    fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.value[0]	= tmp;
		    fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.value[1]	= tmp;
		    fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.value[2]	= tmp;
		    fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.value[3]	= tmp;
			if(new)
				fprintf(new,"VAL %d %d %d %d\n",obj->obj_flags.value[0],obj->obj_flags.value[1],obj->obj_flags.value[2],obj->obj_flags.value[3]);
		}else if(!strncasecmp(szTag,"WGT",3)){
		    fscanf(obj_f, " %d ", &tmp);   obj->obj_flags.weight	= tmp;
			if(new)
				fprintf(new,"WGT %d ",tmp);
		}else if(!strncasecmp(szTag,"COS",3)){
		    fscanf(obj_f, " %d ", &tmp); obj->obj_flags.cost		= tmp;
			if(new)
				fprintf(new,"COS %d ",tmp);
		}else if(!strncasecmp(szTag,"CPD",3)){
		    fscanf(obj_f, " %d \n", &tmp); obj->obj_flags.cost_per_day	= tmp;
			if(new)
				fprintf(new,"CPD %d\n",tmp);
		}else if(!strncasecmp(szTag,"DET",3)){
		    fscanf(obj_f, " %d \n", &tmp); 
			if(tmp == -1)
				obj->iDetLife = tmp;
			else
				obj->iDetLife	= tmp*86400;
			if(new)
				fprintf(new,"DET %d\n",tmp);

		}else if(!strncasecmp(szTag,"LOO",3)){
			CREATE(new_descr, struct extra_descr_data, 1);
			new_descr->keyword	= fread_string(obj_f);
			new_descr->description	= fread_string(obj_f);
			new_descr->next		= obj->ex_description;
			obj->ex_description	= new_descr;
			if(new){
				fprintf(new,"LOO ");
				write_filtered_text(new,new_descr->keyword);
				write_filtered_text(new,new_descr->description);
			}
		}else if(!strncasecmp(szTag,"AFF",3)){
			fscanf(obj_f, " %d ", &tmp);   
			if(tmp==8){ /* level restriction is different. */
	   			fscanf(obj_f, " %d ", &tmp); 
			   	obj->obj_flags.eq_level=tmp;
				if(new)
					fprintf(new,"AFF %d %d\n",8,tmp);
				
			}else{
				if(iAffs>=MAX_OBJ_AFFECT)
					SUICIDE;
				obj->affected[iAffs].location	= tmp;
				if(new)
					fprintf(new,"AFF %d ",tmp);
				fscanf(obj_f, " %d ", &tmp); obj->affected[iAffs].modifier	= tmp;
				if(new)
					fprintf(new,"%d\n",tmp);
				iAffs++;
			}
		}
	}while(szTag[0]!='#');
    for (;(iAffs < MAX_OBJ_AFFECT);iAffs++){
		obj->affected[iAffs].location	= APPLY_NONE;
		obj->affected[iAffs].modifier	= 0;
    }
    obj->in_room	= NOWHERE;
    obj->next_content	= NULL;
    obj->carried_by	= NULL;
    obj->in_obj		= NULL;
    obj->contains	= NULL;
    obj->item_number	= nr;  
    if(!obj->obj_flags.eq_level)
        obj->obj_flags.eq_level	= eq_level;
    if(new){
		free_obj(obj);
		fclose(obj_f);
		open_files--;
		return(NULL);
    }
    return (obj);  
}


struct obj_data *read_object(int nr, int eq_level)
{
    struct obj_data *obj=NULL;
    int i;

    struct extra_descr_data *new_descr=NULL, *cp_descr=NULL;

    if(nr<0||nr>MAX_OBJ||!objs[nr]){
	sprintf(log_buf,"##No object #%d in read_obj",nr);
	log_hd(log_buf);
	return(NULL);
    }
    CREATE(obj, struct obj_data, 1);
    clear_object(obj);

#ifdef HASHTABLE
    if(objs[nr]->obj_flags.type_flag!=ITEM_DRINKCON)
        obj->name		= objs[nr]->name;
    else
	obj->name		= str_dup(objs[nr]->name);
    obj->short_description	= objs[nr]->short_description;
    obj->description		= objs[nr]->description;
    obj->action_description	= objs[nr]->action_description;
#else
    obj->name		= str_dup(objs[nr]->name);
    obj->short_description	= str_dup(objs[nr]->short_description);
    obj->description		= str_dup(objs[nr]->description);
    obj->action_description	= str_dup(objs[nr]->action_description);
#endif

    /* *** numeric data *** */
    obj->obj_flags.type_flag	= objs[nr]->obj_flags.type_flag;
    obj->obj_flags.extra_flags	= objs[nr]->obj_flags.extra_flags;
    obj->obj_flags.wear_flags	= objs[nr]->obj_flags.wear_flags;
    obj->obj_flags.value[0]	= objs[nr]->obj_flags.value[0];
    obj->obj_flags.value[1]	= objs[nr]->obj_flags.value[1];
    obj->obj_flags.value[2]	= objs[nr]->obj_flags.value[2];
    obj->obj_flags.value[3]	= objs[nr]->obj_flags.value[3];
    obj->obj_flags.weight	= objs[nr]->obj_flags.weight;
    obj->obj_flags.cost		= objs[nr]->obj_flags.cost;
    obj->obj_flags.cost_per_day	= objs[nr]->obj_flags.cost_per_day;
    obj->iBornDate = (int) time(NULL);
    obj->iDetLife = objs[nr]->iDetLife;
    obj->iLastDet = 0;
    /* *** extra descriptions *** */
    cp_descr=objs[nr]->ex_description;
    obj->ex_description = NULL;
    while (cp_descr)
    {
	CREATE(new_descr, struct extra_descr_data, 1);
	new_descr->keyword	= str_dup(cp_descr->keyword);
	new_descr->description	= str_dup(cp_descr->description);
	new_descr->next		= obj->ex_description;
	obj->ex_description	= new_descr;
	cp_descr=cp_descr->next;
    }

    for( i = 0 ; i < MAX_OBJ_AFFECT ; i++)
    {
	obj->affected[i].location	= objs[nr]->affected[i].location;
	obj->affected[i].modifier	= objs[nr]->affected[i].modifier;
    }

    obj->in_room	= NOWHERE;
    obj->next_content	= NULL;
    obj->carried_by	= NULL;
    obj->in_obj		= NULL;
    obj->contains	= NULL;
    obj->item_number	= nr;  
    obj->obj_flags.eq_level	= objs[nr]->obj_flags.eq_level;

	add_object(obj);
    obj_index[nr].number++;
    if(!obj->iValueWeight||obj->iWeightVersion<VERSIONNUMBER){
       	obj->iValueWeight=weigh(obj);
        obj->iWeightVersion=VERSIONNUMBER;
    }
                        
    return (obj);  
}


void clear_object(struct obj_data *obj)
{
/*
    memset((char *)obj, (char)'\0', (int)sizeof(struct obj_data));
*/
    int i=0;

	obj->insane					= FALSE;  /* used for sanity checking */
    obj->item_number 			= -1;
    obj->in_room      			= NOWHERE;

	for (i=0;i<4;i++)
		obj->obj_flags.value[1] = 0;
    obj->obj_flags.type_flag		= 0;
    obj->obj_flags.wear_flags		= 0;
    obj->obj_flags.extra_flags		= 0;
    obj->obj_flags.weight		= 0;
    obj->obj_flags.cost			= 0;
    obj->obj_flags.cost_per_day		= 0;
    obj->obj_flags.eq_level		= 0;
    obj->obj_flags.timer		= 0;
    obj->obj_flags.bitvector		= 0;

	for (i=0;i<MAX_OBJ_AFFECT;i++){
		obj->affected[i].location		= 0;
		obj->affected[i].modifier		= 0;
	}

    obj->name 					= MED_NULL ;
    obj->description 			= MED_NULL ;  
    obj->short_description 		= MED_NULL; 
    obj->action_description 	= MED_NULL;
    obj->ex_description 		= NULL;
    obj->carried_by 			= NULL;
    obj->worn_by 				= NULL;  

    obj->in_obj 				= NULL;   
    obj->contains 				= NULL;

    obj->next_content 			= NULL;
    obj->next 					= NULL;
    obj->prev 					= NULL;

	obj->iBornDate 				= 0;
	obj->iDetLife				= 0;
	obj->iLastDet				= 0;
}



/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
    struct extra_descr_data *this=NULL, *next_one=NULL;

    if(obj->item_number>=0&&objs[obj->item_number]){
        if(obj->name)
        if(obj->name!=objs[obj->item_number]->name)
             obj->name = my_free( obj->name               );
        if(obj->description)
        if(obj->description!=objs[obj->item_number]->description)
    	    obj->description = my_free( obj->description        );
        if(obj->short_description)
        if(obj->short_description!=objs[obj->item_number]->short_description)
    	    obj->short_description = my_free( obj->short_description  );
	if(obj->item_number==1973){/* newspaper */
	    if(obj->action_description){
		if(obj->action_description!=DA_paper->action_description)
		    if(obj->action_description!=objs[obj->item_number]->action_description)
	    	        obj->action_description = my_free( obj->action_description );
	    }
	}else{
            if(obj->action_description)
	    if(obj->action_description!=objs[obj->item_number]->action_description)
    	        obj->action_description = my_free( obj->action_description );
	}
    }else{
        obj->name = my_free( obj->name               );
    	obj->description = my_free( obj->description        );
    	obj->short_description = my_free( obj->short_description  );
    	obj->action_description = my_free( obj->action_description );
    }
    if(obj->item_number!=1973){/* newspaper */
        for ( this = obj->ex_description; this != NULL; this = next_one ){
	    next_one = this->next;
	    this->keyword = my_free( this->keyword );
	    this->description = my_free( this->description );
	    this = my_free( this );
        }
    }
    obj = my_free(obj);
}


struct index_data *generate_indices_objs(FILE *fl, int *top,
	struct index_data *index)
{
    char buf[MAX_STRING_LENGTH];
    int i=0;
    int number;
    rewind(fl);
    for (;;)
    {
	if (fgets(buf, 81, fl))
	{
	    if (*buf == '#')
	    {
		/* allocate new cell */
		if ( i >= MAX_INDEX )
		{
		    perror( "Too many indexes" );
			SUICIDE;
		}
		i++;		
		sscanf(buf, "#%d", &number);
     		index[number].pos=ftell(fl);
		index[number].number = 0;
		index[number].func=NULL;
	    }
	    else 
		if (*buf == '$'){    /* EOF */
     		   index[number].pos=0;
		    break;
		}
	}
	else
	{
	    perror("generate indices");
		SUICIDE;
	}
    }
    *top = i - 2;
    for(i=0;i<MAX_OBJ;i++)
	if(index[i].pos)
	    objs[i]=load_object(i,0,NULL);
    return(index);
}

#define ZCMD zone_table[zone].cmd[cmd_no]
void do_save_objs(struct char_data *ch, char *argument, int cmd)
{               
FILE *obj_fh,*dead_objs;
char full_filename[255];
char filename[255];
int obj,zone,loaded,cmd_no;

    if(strcmp(GET_NAME(ch),"Vryce")){
		send_to_char("This is a VRYCE only command.\n\r",ch);
		return;
    }
    one_argument(argument,filename);
    if(!filename||!filename[0]){
		send_to_char("No filename given.\n\r",ch);
		return;
    }
    strcpy(full_filename,"../lib/");
    strcat(full_filename,filename);
    if (!(obj_fh = med_open(full_filename, "w"))){
		send_to_char("Could not open file!\n\r",ch);
		return;
    }
    open_files++;
    send_to_char("Starting..this takes a while...",ch);
    for(obj=0;obj<MAX_OBJ;obj++){
        if(!objs[obj])
        	continue;
		loaded=0;
        for ( zone = 0; zone < MAX_ZONE; zone++ ){ 
            if ( zone_table[zone].reset_mode == -1 )
            	continue;
	    	for (cmd_no = 0;;cmd_no++){
        		if (ZCMD.command == 'S')
            		    break;
	            if(ZCMD.command=='O'){
				    if(ZCMD.arg1==obj)
						loaded++;
				}   
	            if(ZCMD.command=='E'){
				    if(ZCMD.arg1==obj)
						loaded++;
				}   
	            if(ZCMD.command=='G'){
				    if(ZCMD.arg1==obj)
						loaded++;
				}   
	            if(ZCMD.command=='P'){
				    if(ZCMD.arg1==obj)
						loaded++;
				}   
	            if(ZCMD.command=='W'){
				    if(ZCMD.arg1==obj)
						loaded++;
				}   
		    }
   		 }
		if(!loaded){
		    fclose(hold2);hold2=NULL;
    		if (!(dead_objs = fopen("../lib/dead_objs.list", "a"))){
				send_to_char("Could not open dead_objs.list file!\n\r",ch);
				hold2=fopen("../lib/hold2.dat","w");
				return;
    	    }
		    fprintf(dead_objs,"%d %s\n",obj,objs[obj]->short_description);
		    fclose(dead_objs);
	    	hold2=fopen("../lib/hold2.dat","w");
		}
		load_object(obj,0,obj_fh);
    }
    fprintf(obj_fh,"#19999\n$~\n");
    med_close(obj_fh);
	open_files--;
    send_to_char("Done!\n\r",ch);
}
