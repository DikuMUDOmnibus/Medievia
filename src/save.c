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
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>
          
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "spells.h"
#include "interp.h"
#include "holocode.h"

extern char global_color;
extern struct index_data *obj_index;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern int giNewsVersion;
extern struct obj_data *objs[MAX_OBJ];
extern struct obj_data *deteriorate(struct obj_data *obj);
void obj_store_to_char(struct char_data *ch, struct obj_file_elem *object);
extern ush_int Holo[MAXHOLO][MAXHOLO];
#ifndef OPENONLY
bool put_obj_in_store(struct obj_data *obj, struct char_data *ch, FILE *fpsave);
bool obj_to_store(struct obj_data *, struct char_data *, FILE *);
#endif
#ifdef OPENONLY
bool put_obj_in_store(struct obj_data *obj, struct char_data *ch, int fpsave);
bool obj_to_store(struct obj_data *, struct char_data *, int);
#endif
void restore_weight(struct obj_data *obj);
void store_to_char(struct char_file_u *st, struct char_data *ch);
void char_to_store(struct char_data *ch, struct char_file_u *st);
extern int weigh(struct obj_data *obj);
extern struct time_info_data age(struct char_data *ch);
extern int VERSIONNUMBER;
void zero_uchar(struct char_file_u *st);
extern void SaveAndExtractFreight(struct char_data *stpCh);
extern void SaveFreight(struct char_data *stpCh);
extern int iMakeHoloRoom(int x,int y);

#define CHARVERSIONNUMBER 2

void zero_uchar(struct char_file_u *uchar)
{
int i=0;
	uchar->version = 0;					/* Lets initialize all */
	uchar->sex = 0;						/* of char_file_u to   */
	uchar->level = 0;					/* make sure we don't  */
	uchar->birth = 0;					/* read uninitialized  */
	uchar->played = 0;					/* variables.		   */
	uchar->weight = 0;
	uchar->height = 0;
	uchar->title[0] = MED_NULL;
	uchar->siMoreFlags = 0;
	uchar->description[0] = MED_NULL;
	uchar->load_room = 0;
	uchar->abilities.str = 0;
	uchar->abilities.sta = 0; 
	uchar->abilities.intel = 0; 
	uchar->abilities.wis = 0; 
	uchar->abilities.dex = 0; 
	uchar->abilities.con = 0;
	uchar->points.mana = 0; 
	uchar->points.max_mana = 0; 
	uchar->points.hit = 0; 
	uchar->points.max_hit = 0; 
	uchar->points.move = 0; 
	uchar->points.max_move = 0; 
	uchar->points.armor = 0; 
	uchar->points.gold = 0; 
	uchar->points.exp = 0; 
	uchar->points.hitroll = 0; 
	uchar->points.damroll = 0; 				/* Done initializing */

	for (i=0;i<MAX_SKILLS;i++){
		uchar->skills[i].learned = 0;
		uchar->skills[i].recognise = 0;
	}

	for (i=0;i<MAX_AFFECT;i++){
		uchar->affected[i].type = 0;
		uchar->affected[i].duration = 0;
		uchar->affected[i].modifier = 0;
		uchar->affected[i].location = 0;
		uchar->affected[i].bitvector = 0;
	}

	uchar->practices = 0;
	uchar->alignment = 0;
	uchar->last_logon = 0;
	uchar->act = 0;
	uchar->name[0] = MED_NULL;
	uchar->pwd[0] = MED_NULL;

	for (i=0;i<5;i++)
		uchar->apply_saving_throw[i] = 0;

	for (i=0;i<3;i++)
		uchar->conditions[i] = 0;

#ifndef MEDTHIEVIA
	for (i=0;i<77;i++)
#else
	for (i=0;i<75;i++)
#endif
		uchar->unused[i] = 0;


	uchar->new_comm_flags = 0;
	uchar->home_number = 0;
	uchar->wimpy = 0;
	uchar->autoexit = 0;
	uchar->clan_leader =0;
	uchar->clan = 0;
	uchar->ansi_color = 0;
	uchar->eggs = 0;
	uchar->editzone = 0;
	uchar->numkills = 0;
	uchar->numpkills = 0;
	uchar->death_timer  = 0;
	uchar->death_counter = 0;
	uchar->death_corpsehits =0;
	uchar->multi_class = 0;
	uchar->iLastTown = 0;
	uchar->iLastInBeforeSocial = 0;
	uchar->iLastSocialX = 0;
	uchar->iLastSocialY = 0;
    	uchar->god_display = 0;
	uchar->iFlags = 0;
	uchar->iHoloX=0;
	uchar->iHoloY=0;
	uchar->iNewsVersion=0;
	uchar->iMount=0;
	return;
}
	

void do_rent(struct char_data *ch,  char *argument, int cmd)
{
    if(IS_DEAD(ch))
	{
	send_to_char("Corpses don't need to rent.  Just use quit.\n\r",ch);
	return;
	}
    if(IS_NPC(ch))return;
    if(GET_LEVEL(ch)<2&&!ch->player.multi_class){
	send_to_char("Sorry, must be level 2 to rent.\n\r",ch);
	return;
    }
    
	if(world[ch->in_room]->number!=101&&
	world[ch->in_room]->number!=3931&&
	world[ch->in_room]->number!=7167&&
	world[ch->in_room]->number!=2633&&
	world[ch->in_room]->number!=2269&&  /* Inn of the broken horn */
	world[ch->in_room]->number!=2574&&  /* Mystical Forest */
	world[ch->in_room]->number!=3480){
	send_to_char("Sorry, you must rent from within a hotel.\n\r",ch);
	ch->p->querycommand=0;
	return;
    }

    if(cmd==4623){
	if(argument[0]=='Y'||argument[0]=='y'){
	    GET_GOLD(ch)-=ch->specials.cost_to_rent;
		    ch->p->querycommand=0;
	    save_char_obj(ch);
	    ch->specials.cost_to_rent=-2;/* so put_obj does not list stuff*/
	    argument[0]=MED_NULL;
	    do_quit(ch,argument,0);
	    return;
	}
	/* otherwise forget it */
	ch->p->querycommand=0;
	ch->specials.cost_to_rent=0;
	return;
    }

    ch->specials.cost_to_rent=0;
    global_color=1;
    send_to_char("\n\r\n\rSCROLLS, WANDS, RODS, STAFFS, NOTES and KEYS and \n\rcertain other items cannot be saved.\n\r",ch);
    send_to_char("Following is cost per/item AND what cannot be rented:\n\r",ch);
    ch->p->queryfunc=do_rent;
    strcpy(ch->p->queryprompt,"Rent the room and quit? (y/n)> ");
    ch->p->querycommand=4623;
    ch->specials.cost_to_rent+=5;
    save_char_obj(ch);
    sprintf(log_buf,"\n\rIt will cost you [%d Gold], to rent with current equipment.\n\r", ch->specials.cost_to_rent);
    send_to_char(log_buf,ch);
    global_color=0;
    if(GET_GOLD(ch)<ch->specials.cost_to_rent){
	global_color=33;
	send_to_char("Sorry, you do not have enough gold!\n\r",ch);
	global_color=0;
	ch->p->querycommand=0;
    }
    return;


}


void save_char_obj( struct char_data *ch )
{
    struct  char_file_u uchar;
    char    strsave[MAX_INPUT_LENGTH],c;
    int     iWear,bytes_written=0;

    FILE *  fpsave  = NULL;

    if ( IS_NPC(ch) || (GET_LEVEL(ch) < 2 &&!ch->player.multi_class))
        return;
	if(ch->p->stpFreight)
	    SaveFreight(ch);

    c=LOWER(ch->player.name[0]);

    sprintf( strsave, "%s/%c/%s", SAVE_DIR, c, ch->player.name);
	fpsave = med_open(strsave, "wb");
	if (fpsave == NULL)
	{
	    sprintf( log_buf, "## ERRORCannot open fileSave_char_obj: %s", strsave);
	    log_hd( log_buf );
	    perror( log_buf );
	    send_to_char("Sorry disk error.\n\r",ch);
	    return;
    	}
	else{  /* We've successfully opened the file, lets write to it*/
		open_files++;
		zero_uchar( &uchar );

/*
		uchar.version = CHARVERSIONNUMBER;						
		uchar.sex = 0;					
		uchar.level = 0;					
		uchar.birth = 0;					
		uchar.played = 0;				
		uchar.weight = 0;
		uchar.height = 0;
		uchar.title[0] = MED_NULL;
		uchar.siMoreFlags = 0;
		uchar.description[0] = MED_NULL;
		uchar.load_room = 0;
		uchar.abilities.str = 0;
		uchar.abilities.sta = 0; 
		uchar.abilities.intel = 0; 
		uchar.abilities.wis = 0; 
		uchar.abilities.dex = 0; 
		uchar.abilities.con = 0;
		uchar.points.mana = 0; 
		uchar.points.max_mana = 0; 
		uchar.points.hit = 0; 
		uchar.points.max_hit = 0; 
		uchar.points.move = 0; 
		uchar.points.max_move = 0; 
		uchar.points.armor = 0; 
		uchar.points.gold = 0; 
		uchar.points.exp = 0; 
		uchar.points.hitroll = 0; 
		uchar.points.damroll = 0; 		

		for (i=0;i<MAX_SKILLS;i++){
			uchar.skills[i].learned = 0;
			uchar.skills[i].recognise = 0;
		}

		for (i=0;i<MAX_AFFECT;i++){
			uchar.affected[i].type = 0;
			uchar.affected[i].duration = 0;
			uchar.affected[i].modifier = 0;
			uchar.affected[i].location = 0;
			uchar.affected[i].bitvector = 0;
		}

		uchar.practices = 0;
		uchar.alignment = 0;
		uchar.last_logon = 0;
		uchar.act = 0;
		uchar.name[0] = MED_NULL;
		uchar.pwd[0] = MED_NULL;

		for (i=0;i<5;i++)
			uchar.apply_saving_throw[i] = 0;

		for (i=0;i<3;i++)
			uchar.conditions[i] = 0;

		for (i=0;i<100;i++)
			uchar.unused[i] = 0;

*/

    	char_to_store( ch, &uchar );

    	if ( ch->in_room < 2 )
			uchar.load_room = 1;
    	else{
			if(!world[ch->in_room]){
				sprintf(log_buf,"## bad ch->in-room in save_char_obj (%d)",ch->in_room);
				log_hd(log_buf);
				ch->in_room=1;
			}  /* end if */
			uchar.load_room = world[ch->in_room]->number;
		} /* end else */

		bytes_written = fwrite(&uchar, sizeof(uchar), 1, fpsave);
		if (bytes_written==0){
	    	sprintf( log_buf,"## ERRORcannot write ucharSave_char_obj: %s",strsave);
	    	log_hd( log_buf );
	    	perror("PERROR REPORTS->");
	    	send_to_char("Sorry, CHAR_DATA disk write error\n\r",ch);
	    	restore_weight( ch->carrying );
		if (fpsave != NULL){
			med_close(fpsave);
			open_files--;
		}
	    	return;
    	}

    	if(ch->specials.cost_to_rent==-1){

		if(fpsave!=NULL){
			med_close(fpsave);
			open_files--;
		}
		return;/* didn't rent */
   	}

    	if ( !obj_to_store( ch->carrying, ch, fpsave ) ){
	    	sprintf( log_buf, "## ERRORcannotobjtostoreSave_char_obj: %s",strsave);
	    	log_hd( log_buf );
	    	perror("PERROR REPORTS->");
	    	send_to_char("Sorry, OBJECTS disk write error\n\r",ch);
	    	restore_weight( ch->carrying );

	    	if ( fpsave != NULL ){
				med_close( fpsave );
				open_files--;
			}
	    	return;
    	}

    	restore_weight( ch->carrying );

    	for ( iWear = 0; iWear < MAX_WEAR; iWear++ ){
			if ( ch->equipment[iWear] ){
	    		if ( !obj_to_store( ch->equipment[iWear], ch, fpsave ) ){
	        		sprintf( log_buf, "## ERRORSave_char_obj: %s", strsave );
	        		log_hd( log_buf );
	        		perror("PERROR REPORTS->");
	        		restore_weight( ch->carrying );
					send_to_char("Sorry EQUIP disk write error\n\r", ch);

	        		if ( fpsave != NULL ){
		    			med_close( fpsave );
						open_files--;
					}
					return;
	    		}

	    		restore_weight( ch->equipment[iWear] );
			}
    	}

    	if ( fpsave != NULL ){
			med_close( fpsave );
			open_files--;
		}

    	return;
	}
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( struct descriptor_data *d, char *name )
{
    char    strsave[MAX_INPUT_LENGTH],c;
    struct  char_file_u uchar;
    struct  char_data *ch=NULL;
    struct specialsP_data *ptmp=NULL;
	int		bytes_read=0;
    void clear_pData(struct char_data *ch);

#ifndef OPENONLY
    FILE *  fpsave  = NULL;
#endif

#ifdef OPENONLY
	int fpsave = -1;
#endif

    CREATE( ch, struct char_data, 1 );
    CREATE( ptmp, struct specialsP_data, 1);
    d->character    = ch;
    clear_char( ch );
    ch->p=ptmp;
	ch->p->stpFreight=NULL;
	ch->specials.stpMount=NULL;
    ch->desc        = d;
    c=LOWER(name[0]);
    sprintf( strsave, "%s/%c/%s", SAVE_DIR, c, name );

	fpsave = med_open(strsave, "rb");
	if (fpsave==NULL){
		sprintf(log_buf,"##load_char_obj cannot open player file %s",strsave);
		return FALSE;
	}

	open_files++;

	bytes_read = fread(&uchar, sizeof(uchar), 1, fpsave);
	if (bytes_read == 0)
		goto LError;

    reset_char( ch );
    clear_pData( ch );  
  GET_NAME(ch) = str_dup(name);
    ch->formation[0][1]=ch;
    ch->master=ch;
    store_to_char( &uchar, ch );

    if(GET_LEVEL(ch)<31){
        if(ch->specials.conditions[THIRST]<0)
		    ch->specials.conditions[THIRST]=0;
        if(ch->specials.conditions[FULL]<0)
		    ch->specials.conditions[FULL]=0;
   	}

    while ( bytes_read  ){
		struct  obj_file_elem   object;

		bytes_read = fread( &object, sizeof(object), 1, fpsave );
		if ( ferror( fpsave ) )
	    	goto LError;

		if ( feof( fpsave ) )
			break;
		obj_store_to_char( ch, &object );
    }

    goto LSuccess;

 LError:
    sprintf( log_buf, "Load_char_obj: %s", strsave );
    perror( log_buf );
    if ( fpsave != NULL ){
		med_close( fpsave );
		open_files--;
	}
    return FALSE;

 LSuccess:

    if ( fpsave != NULL ){
		med_close( fpsave );
		open_files--;
	}
    ch->specials.wizInvis=0;
    ch->specials.birthday=age(ch).year;
    if(GET_LEVEL(ch)>=35)GET_GOLD(ch)=1000000000;
    if(ch->specials.home_number>19999)ch->specials.home_number=1;
    return TRUE;
}



void obj_store_to_char(struct char_data *ch, struct obj_file_elem *object)
{
    struct obj_data *obj=NULL;
    int j;
    int nr;

    void obj_to_char(struct obj_data *object, struct char_data *ch);

    if ( ( nr = real_object(object->item_number) ) > -1 ) 
    {
	obj = read_object( nr, 0 );
	if(!obj){
	   sprintf(log_buf,"## obj_to_char object %d does not exist",nr);
	   log_hd(log_buf);
	   return;
	}
	/*obj->obj_flags.value[0] = object->value[0];*/
	obj->obj_flags.value[1] = object->value[1];
	obj->obj_flags.value[2] = object->value[2];
	obj->obj_flags.value[3] = object->value[3];

	obj->obj_flags.extra_flags = object->extra_flags;
	/*obj->obj_flags.weight      = object->weight;*/
	obj->obj_flags.timer       = object->timer;
	obj->obj_flags.eq_level    = object->eq_level;
	obj->obj_flags.bitvector   = object->bitvector;

	for(j=0; j<MAX_OBJ_AFFECT; j++)
	    obj->affected[j] = object->affected[j];
    obj->iValueWeight=object->iWeight;
    obj->iWeightVersion=object->iWeightVersion;
    obj->iBornDate = object->iItemBorn;
    obj->iLastDet = object->iLastDet;
    obj->iDetLife = object->iItemLasts;

	if(!obj->iValueWeight||obj->iWeightVersion!=VERSIONNUMBER){
		obj->iValueWeight=weigh(obj);
		obj->iWeightVersion=VERSIONNUMBER;
		obj->iBornDate = (int) time(NULL);
		obj->iLastDet = 0;
		obj->iDetLife = objs[obj->item_number]->iDetLife;
		
	}
	obj = deteriorate(obj);
	obj_to_char(obj, ch);
    }else{
	sprintf(log_buf,"#######NOT REAL OBJECT IN PLAYER FILE-%s",GET_NAME(ch));
	log_hd(log_buf);
	sprintf(log_buf,"item[%d]",object->item_number);
	log_hd(log_buf);
    }
}



#ifndef OPENONLY
bool obj_to_store( struct obj_data *obj, struct char_data *ch, FILE *fpsave){
#endif
#ifdef OPENONLY
bool obj_to_store(struct obj_data *obj, 
				  struct char_data *ch, 
				  int fpsave){
#endif

    struct obj_data *tmp=NULL;

    if ( obj == NULL )
		return TRUE;

    /* Write depth first (so weights come out right) */
    if ( !obj_to_store( obj->contains, ch, fpsave ) )
		return FALSE;

    if ( !obj_to_store( obj->next_content, ch, fpsave) )
		return FALSE;

    if ( !put_obj_in_store( obj, ch, fpsave ) )
		return FALSE;

    /* Adjust container weights of up-linked items */
    for ( tmp = obj->in_obj; tmp; tmp = tmp->in_obj )
		GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

    return TRUE;
}


/*
 * Write one object to the file.
 */
#ifndef OPENONLY
bool put_obj_in_store(struct obj_data *obj, struct char_data *ch, FILE *fpsave){
#endif
#ifdef OPENONLY
bool put_obj_in_store(struct obj_data *obj, 
					  struct char_data *ch, 
					  int fpsave){
#endif

    int iAffect,type;
    struct  obj_file_elem   object;
	int bytes_written=0;

    type=GET_ITEM_TYPE(obj);
/*    if(ch->p->querycommand==4623)*/
    if(ch->specials.cost_to_rent!=-20)  /* not autosaving */
    	if(   type==ITEM_KEY
			||type==ITEM_SCROLL
			||type==ITEM_WAND
			||type==ITEM_STAFF
			||type==ITEM_POTION
			||IS_SET(obj->obj_flags.extra_flags, ITEM_NO_RENT)){
       		if(ch->p->querycommand==4623){
				sprintf(log_buf,"[SAVING %s IS NOT ALLOWED]\n\r",obj->short_description);
				global_color=31;
				send_to_char(log_buf,ch);
				global_color=0;
       		}
		return TRUE;
    }
   
    if(ch->p->querycommand==4623){   
        sprintf(log_buf,"       Storing %s costs[%d-gold]\n\r",obj->short_description,obj->obj_flags.cost_per_day/2);
		global_color=34;
        send_to_char(log_buf,ch);
		global_color=0;
    }

    object.version	= 0;
    object.item_number  = obj->item_number;
    object.value[0]     = obj->obj_flags.value[0];
    object.value[1]     = obj->obj_flags.value[1];
    object.value[2]     = obj->obj_flags.value[2];
    object.value[3]     = obj->obj_flags.value[3];
    object.extra_flags  = obj->obj_flags.extra_flags;
    object.weight       = obj->obj_flags.weight;
    object.timer        = obj->obj_flags.timer;
    object.eq_level	= obj->obj_flags.eq_level;
    object.bitvector    = obj->obj_flags.bitvector;
    for ( iAffect = 0; iAffect < MAX_OBJ_AFFECT; iAffect++ )
		object.affected[iAffect]    = obj->affected[iAffect];
	object.iWeight=obj->iValueWeight;
	object.iWeightVersion=obj->iWeightVersion; 
	object.iItemBorn = obj->iBornDate;
	object.iItemLasts = obj->iDetLife;
	object.iLastDet = obj->iLastDet;
	   
#ifndef OPENONLY
	bytes_written=fwrite(&object, sizeof(object), 1, fpsave);
#endif

#ifdef OPENONLY
	bytes_written=write(fpsave, object, sizeof(object));
#endif

    if ( bytes_written == 0 ){
		log_hd("###FWRITE ERROR IN PUT_OBJ_IN_STORE#######");
		return FALSE;
    }

    if(ch->p->querycommand==4623)
       ch->specials.cost_to_rent+=(obj->obj_flags.cost_per_day/2);

    return TRUE;
}



/*
 * Restore container weights after a save.
 */
void restore_weight(struct obj_data *obj)
{
    struct obj_data *tmp=NULL;

    if ( obj == NULL )
	return;

    restore_weight( obj->contains );
    restore_weight( obj->next_content );
    for ( tmp = obj->in_obj; tmp; tmp = tmp->in_obj )
	GET_OBJ_WEIGHT( tmp ) += GET_OBJ_WEIGHT( obj );
}



void store_to_char(struct char_file_u *st, struct char_data *ch)
{
    int i;

    strncpy( ch->pwd, st->pwd, 11 );

    GET_SEX(ch) = st->sex;
    GET_CLASS(ch) = st->class;
    GET_LEVEL(ch) = st->level;

    ch->player.short_descr = 0;
    ch->player.long_descr = 0;

    if (*st->title)
    {
	CREATE(ch->player.title, char, strlen(st->title) + 1);
	strcpy(ch->player.title, st->title);
    }
    else
	GET_TITLE(ch) = str_dup(no_title);

    if (*st->description)
    {
	CREATE(ch->player.description, char, 
	    strlen(st->description) + 1);
	strcpy(ch->player.description, st->description);
    }
    else
	ch->player.description = 0;

    ch->player.siMoreFlags = st->siMoreFlags;
	REMOVE_BIT(ch->player.siMoreFlags,FLY);

    ch->player.time.birth = st->birth;
    ch->player.time.played = st->played;
    ch->player.time.logon  = time(0);

    for (i = 0; i < MAX_TONGUE; i++)
	ch->player.talks[i] = st->talks[i];

    ch->player.weight = st->weight;
    ch->player.height = st->height;

    ch->abilities = st->abilities;
    ch->tmpabilities = st->abilities;
    ch->points = st->points;
	if (ch->points.max_mana < 100) {
	   ch->points.max_mana = 100;
     } /* if */

    for (i = 0; i < MAX_SKILLS; i++)
	ch->skills[i] = st->skills[i];


    ch->specials.practices    = st->practices;
    ch->specials.alignment    = st->alignment;

    ch->specials.new_comm_flags = st->new_comm_flags;
    ch->specials.home_number = st->home_number;
    ch->specials.wimpy = st->wimpy;
    ch->specials.autoexit = st->autoexit;
    ch->specials.clanleader = st->clan_leader;
    ch->specials.clan = st->clan;
    ch->specials.ansi_color = st->ansi_color;
    ch->specials.eggs = st->eggs;
    ch->specials.editzone = st->editzone;
    ch->specials.numkills = st->numkills;
    ch->specials.numpkills = st->numpkills;
    ch->specials.death_timer = st->death_timer;
    ch->specials.death_counter = st->death_counter;
    ch->specials.death_corpsehits= st->death_corpsehits;
    ch->player.multi_class= st->multi_class;
    ch->specials.god_display = st->god_display;
	if(st->version<1){
		ch->abilities.sta=13;
		ch->tmpabilities.sta=13;
		if(GET_CLASS(ch)==CLASS_WARRIOR || GET_CLASS(ch)==CLASS_THIEF){
			ch->abilities.sta=16;
			ch->tmpabilities.sta=16;
		}			
	}
	if(st->version<2)
		GET_GOLD(ch)=5000;
    if(ch->p) {	
    	ch->p->siLastTown = st->iLastTown;
		ch->p->iLastInBeforeSocial = st->iLastInBeforeSocial;
	  	ch->p->iLastSocialX = st->iLastSocialX;
 		ch->p->iLastSocialY = st->iLastSocialY;
		ch->p->iFlags	= st->iFlags;
		ch->p->iNewsVersion = st->iNewsVersion;
		ch->p->iMount=st->iMount;
		if(ch->p->iNewsVersion>giNewsVersion)
			ch->p->iNewsVersion=giNewsVersion;
    }
    ch->specials.act          = st->act;
    ch->specials.carry_weight = 0;
    ch->specials.carry_items  = 0;
    ch->points.armor          = 100;
    if(GET_CLASS(ch)==CLASS_WARRIOR)
	ch->points.armor-=GET_LEVEL(ch)*2;;
    ch->points.hitroll        = 0;
    ch->points.damroll        = 0;


    for(i = 0; i <= 4; i++)
      ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];

    for(i = 0; i <= 2; i++)
      GET_COND(ch, i) = st->conditions[i];

    /* Add all spell effects */
    for(i=0; i < MAX_AFFECT; i++) {
	if (st->affected[i].type)
	    affect_to_char(ch, &st->affected[i]);
    }
	if(st->iHoloX||st->iHoloY){
		if(Holo[st->iHoloX][st->iHoloY]<=255)
			iMakeHoloRoom(st->iHoloX,st->iHoloY);
		ch->in_room=Holo[st->iHoloX][st->iHoloY];
	}else{
		if(world[st->load_room])
	    		ch->in_room = st->load_room;
	    	else 
    			ch->in_room=1;
	}
    affect_total(ch);

}



/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data *ch, struct char_file_u *st)
{
    int i;
    struct affected_type *af=NULL;
    struct obj_data *char_eq[MAX_WEAR];

	zero_uchar( st );
	
    strncpy( st->pwd, ch->pwd, 11 );

    /* Unaffect everything a character can be affected by */

    for(i=0; i<MAX_WEAR; i++) {
	if (ch->equipment[i])
	    char_eq[i] = unequip_char(ch, i);
	else
	    char_eq[i] = 0;
    }

    for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
	if (af &&
		    (!(af->type == SPELL_INVISIBLE &&
		       af->bitvector == AFF_INVISIBLE &&
		       ch->specials.wizInvis))) {
	    st->affected[i] = *af;
	    st->affected[i].next = 0;
	    /* subtract effect of the spell or the effect will be doubled */
	    affect_modify( ch, st->affected[i].location,
			       st->affected[i].modifier,
			       st->affected[i].bitvector, FALSE);                         
	    af = af->next;
	} else {
	    st->affected[i].type = 0;  /* Zero signifies not used */
	    st->affected[i].duration = 0;
	    st->affected[i].modifier = 0;
	    st->affected[i].location = 0;
	    st->affected[i].bitvector = 0;
	    st->affected[i].next = 0;
	}
    }
/*
    if ((i >= MAX_AFFECT) && af && af->next){
	sprintf(log_buf,"Char_to_store [%s]-to many affects",GET_NAME(ch));
	log_hd( log_buf);
    }*/
    ch->tmpabilities = ch->abilities;

    st->version    = CHARVERSIONNUMBER;
    st->birth      = ch->player.time.birth;
    st->played     = ch->player.time.played;
    st->played    += (long) (time(0) - ch->player.time.logon);
    st->last_logon = time(0);

    ch->player.time.played = st->played;
    ch->player.time.logon = time(0);

    st->siMoreFlags = ch->player.siMoreFlags;
    st->weight   = GET_WEIGHT(ch);
    st->height   = GET_HEIGHT(ch);
    st->sex      = GET_SEX(ch);
    st->class    = GET_CLASS(ch);
    st->level    = GET_LEVEL(ch);
    st->abilities = ch->abilities;
    st->points    = ch->points;
    st->alignment = ch->specials.alignment;
    st->practices = ch->specials.practices;
    st->act       = ch->specials.act;

    st->new_comm_flags = ch->specials.new_comm_flags;
    st->home_number = ch->specials.home_number;
    st->wimpy = ch->specials.wimpy;
    st->autoexit = ch->specials.autoexit;
    st->clan_leader = ch->specials.clanleader;
    st->clan = ch->specials.clan;
    st->ansi_color = ch->specials.ansi_color;
    st->eggs = ch->specials.eggs;
    st->editzone = ch->specials.editzone;
    st->numkills = ch->specials.numkills;
    st->numpkills = ch->specials.numpkills;
    if(IS_UNDEAD(ch) && (ch->specials.death_timer < 1)) 
	st->death_timer = 1;
    else
	st->death_timer = ch->specials.death_timer;
/*
if (port == 4000||port = 4444)
	st->unused[11] = 0;
*/
    st->death_counter = ch->specials.death_counter;
    st->death_corpsehits = ch->specials.death_corpsehits;
    st->multi_class = ch->player.multi_class;
    st->god_display = ch->specials.god_display;
    if(ch->p) {
    	st->iLastTown = ch->p->siLastTown;
		st->iLastInBeforeSocial = ch->p->iLastInBeforeSocial;
    	st->iLastSocialX = ch->p->iLastSocialX;
		st->iLastSocialY = ch->p->iLastSocialY;
		st->iFlags	= ch->p->iFlags;
		st->iNewsVersion = ch->p->iNewsVersion;
		if(ch->specials.stpMount)
			st->iMount=ch->specials.stpMount->nr;
		if(IN_HOLO(ch)){
		   	st->iHoloX=world[ch->in_room]->holox;
	    	st->iHoloY=world[ch->in_room]->holoy;
		}else{
		   	st->iHoloX=0;
	    	st->iHoloY=0;
		}
    }
    st->points.armor   = 100;
    st->points.hitroll =  0;
    st->points.damroll =  0;

    if (GET_TITLE(ch))
	strcpy(st->title, GET_TITLE(ch));
    else
	*st->title = '\0';

    if (ch->player.description)
	strcpy(st->description, ch->player.description);
    else
	*st->description = '\0';


    for (i = 0; i < MAX_TONGUE; i++)
	st->talks[i] = ch->player.talks[i];

    for (i = 0; i < MAX_SKILLS; i++)
	st->skills[i] = ch->skills[i];

    strcpy(st->name, GET_NAME(ch) );

    for(i = 0; i <= 4; i++)
      st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

    for(i = 0; i <= 2; i++)
      st->conditions[i] = GET_COND(ch, i);

    for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
	if (af &&
		    (!(af->type == SPELL_INVISIBLE &&
		       af->bitvector == AFF_INVISIBLE &&
		       ch->specials.wizInvis))) {
	    /* Add effect of the spell or it will be lost */
	    /* When saving without quitting               */
	    affect_modify( ch, st->affected[i].location,
			       st->affected[i].modifier,
			       st->affected[i].bitvector, TRUE);
	    af = af->next;
	}
    }

    for(i=0; i<MAX_WEAR; i++) {
	if (char_eq[i])
	    equip_char(ch, char_eq[i], i);
    }

    affect_total(ch);
}
