/***************************************************************************
*					 MEDIEVIA CyberSpace Code and Data files		       *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*						   All rights reserved				               *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/

    
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "interp.h"
#include "holocode.h"
#include "trading.h"
#include "dragon.h"

void add_object(struct obj_data *obj);
extern char global_color;
extern struct DRAGONSTRUCT gstaDragons[MAXDRAGONS];
extern struct zone_data *zone_table;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct descriptor_data *descriptor_list;
extern ush_int Holo[MAXHOLO][MAXHOLO];
extern int iMakeHoloRoom(int x,int y);
extern void RemoveDragon(struct char_data *stpDragon);
extern struct HOLOROOMS *stpHoloRTemplates[256];
extern char MOUNTMOVE;
extern int iFlyStoreRoom;
/* External procedures */
void TouchRooms(int room);
void sanity_check_object_list();
void reset_zone(int zone);
int str_cmp(char *arg1, char *arg2);
void stop_fighting(struct char_data *ch);
void die_formation(struct char_data *ch);
extern char *index();
void prepare_for_quit(struct char_data *, bool);
void obj_from_container(struct obj_data *);
void FreightAutoFollow(struct char_data *stpCh, int iToRoom);

char *fname(char *namelist)
{
    static char holder[30];
    register char *point;

    for (point = holder; isalpha(*namelist); namelist++, point++)
	*point = *namelist;

    *point = '\0';

    return(holder);
}

int isname(char *str, char *namelist)
{
    register char *curname, *curstr;

    curname = namelist;
    for (;;)
    {
		for (curstr = str;; curstr++, curname++){
	    	if (!*curstr && !isalpha(*curname))
				return(1);
		    if (!*curname)
				return(0);
		    if (!*curstr || *curname == ' ')
				break;
		    if (LOWER(*curstr) != LOWER(*curname))
				break;
		}
	/* skip to next name */
		for (; isalpha(*curname); curname++);
		if (!*curname)
	    	return(0);
		curname++;          /* first char of new name */
    }
}



void affect_modify(struct char_data *ch, byte loc, byte mod, long bitv, bool add)
{
    int maxabil;

    if (add) {
	SET_BIT(ch->specials.affected_by, bitv);
    } else {
	REMOVE_BIT(ch->specials.affected_by, bitv);
	mod = -mod;
    }


    maxabil = (IS_NPC(ch) ? 25:18);

    switch(loc)
    {
	case APPLY_NONE:
	    break;

	case APPLY_STR:
	    GET_STR(ch) += mod;
	    break;

	case APPLY_DEX:
	    GET_DEX(ch) += mod;
	    break;

	case APPLY_INT:
	    GET_INT(ch) += mod;
	    break;

	case APPLY_WIS:
	    GET_WIS(ch) += mod;
	    break;

	case APPLY_CON:
	    GET_CON(ch) += mod;
	    break;

	case APPLY_SEX:
	    /* ??? GET_SEX(ch) += mod; */
	    break;

	case APPLY_CLASS:
	    /* ??? GET_CLASS(ch) += mod; */
	    break;

	case APPLY_LEVEL:
	    /* ??? GET_LEVEL(ch) += mod; */
	    break;

	case APPLY_AGE:
	    ch->player.time.birth -= ((long)SECS_PER_MUD_YEAR*(long)mod); 
	    break;

	case APPLY_CHAR_WEIGHT:
	    GET_WEIGHT(ch) += mod;
	    break;

	case APPLY_CHAR_HEIGHT:
	    GET_HEIGHT(ch) += mod;
	    break;

	case APPLY_MANA:
	    ch->points.max_mana += mod;
	    break;

	case APPLY_HIT:
	    ch->points.max_hit += mod;
	    break;

	case APPLY_MOVE:
	    ch->points.max_move += mod;
	    break;

	case APPLY_GOLD:
	    break;

	case APPLY_EXP:
	    break;

	case APPLY_AC:
	    GET_AC(ch) += mod;
	    break;

	case APPLY_HITROLL:
	    GET_HITROLL(ch) += mod;
	    break;

	case APPLY_DAMROLL:
	    GET_DAMROLL(ch) += mod;
	    break;

	case APPLY_SAVING_PARA:
	    ch->specials.apply_saving_throw[0] += mod;
	    break;

	case APPLY_SAVING_ROD:
	    ch->specials.apply_saving_throw[1] += mod;
	    break;

	case APPLY_SAVING_PETRI:
	    ch->specials.apply_saving_throw[2] += mod;
	    break;

	case APPLY_SAVING_BREATH:
	    ch->specials.apply_saving_throw[3] += mod;
	    break;

	case APPLY_SAVING_SPELL:
	    ch->specials.apply_saving_throw[4] += mod;
	    break;

	default:
	    sprintf(log_buf,"##Unknown apply adjust attempt [%d] to %s (handler.c, affect_modify).",loc,GET_NAME(ch));
	    log_hd(log_buf);
	    break;

    } /* switch */
}



/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data *ch)
{
    struct affected_type *af=NULL;
    int i,j;

    for(i=0; i<MAX_WEAR; i++) {
	if (ch->equipment[i])
	    for(j=0; j<MAX_OBJ_AFFECT; j++)
		affect_modify(ch, ch->equipment[i]->affected[j].location,
			      ch->equipment[i]->affected[j].modifier,
			      ch->equipment[i]->obj_flags.bitvector, FALSE);
    }


    for(af = ch->affected; af; af=af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    ch->tmpabilities = ch->abilities;

    for(i=0; i<MAX_WEAR; i++) {
	if (ch->equipment[i])
	    for(j=0; j<MAX_OBJ_AFFECT; j++)
		affect_modify(ch, ch->equipment[i]->affected[j].location,
			      ch->equipment[i]->affected[j].modifier,
			      ch->equipment[i]->obj_flags.bitvector, TRUE);
    }


    for(af = ch->affected; af; af=af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

    /* Make certain values are between 0..25, not < 0 and not > 25! */

    i = (IS_NPC(ch) ? 25 :18);

    GET_DEX(ch) = MAX(0,MIN(GET_DEX(ch), i));
    GET_INT(ch) = MAX(0,MIN(GET_INT(ch), i));
    GET_WIS(ch) = MAX(0,MIN(GET_WIS(ch), i));
    GET_CON(ch) = MAX(0,MIN(GET_CON(ch), i));
    GET_STR(ch) = MAX(0,MIN(GET_STR(ch), i));
}



/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char( struct char_data *ch, struct affected_type *af )
{
    struct affected_type *affected_alloc=NULL;

    CREATE(affected_alloc, struct affected_type, 1);

    /**affected_alloc = *af;*/
    affected_alloc->type=af->type;
    affected_alloc->duration=af->duration;
    affected_alloc->modifier=af->modifier;
    affected_alloc->location=af->location;
    affected_alloc->bitvector=af->bitvector;

    affected_alloc->next = ch->affected;
    ch->affected = affected_alloc;

    affect_modify(ch, af->location, af->modifier,
		  af->bitvector, TRUE);
    affect_total(ch);
}



/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NIL! Frees mem and calls 
   affect_location_apply                                                */
void affect_remove( struct char_data *ch, struct affected_type *af )
{
    struct affected_type *hjp=NULL;

    if(!ch->affected)
		SUICIDE;

    if(!af)return;
    affect_modify(ch, af->location, af->modifier,
		  af->bitvector, FALSE);


    /* remove structure *af from linked list */

    if (ch->affected == af) {
		/* remove head of list */
		ch->affected = af->next;
    } else {
		for(hjp = ch->affected; (hjp->next) && (hjp->next != af); hjp = hjp->next);
			if (hjp->next != af) {
	    		log_hd("##FATAL : Could not locate affected_type in ch->affected.(handler.c, affect_remove)");
				SUICIDE;
			}
			hjp->next = af->next; /* skip the af element */
    	}
    af = my_free ( af );

    affect_total(ch);
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char( struct char_data *ch, byte skill)
{
    struct affected_type *hjp=NULL;
    struct affected_type *next_affect=NULL;

    for(hjp = ch->affected; hjp; hjp = next_affect){
	next_affect=hjp->next;
	if (hjp->type == skill)
	    affect_remove( ch, hjp );
    }

}



/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates 
   not affected                                                        */
bool affected_by_spell( struct char_data *ch, byte skill )
{
    struct affected_type *hjp=NULL;

    for (hjp = ch->affected; hjp; hjp = hjp->next)
	if ( hjp->type == skill )
	    return( TRUE );

    return( FALSE );
}



void affect_join( struct char_data *ch, struct affected_type *af,
		  bool set_dur, bool acc_mod )
{
    struct affected_type *hjp=NULL;
    bool found = FALSE;

    for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
	if ( hjp->type == af->type ) {
	    
	    if (!set_dur)
		af->duration = hjp->duration;

	    if (acc_mod)
		af->modifier += hjp->modifier;

	    affect_remove(ch, hjp);
	    affect_to_char(ch, af);
	    found = TRUE;
	}
    }
    if (!found)
	affect_to_char(ch, af);
}

/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
    struct char_data *i=NULL;

    if (ch->in_room == NOWHERE) {
		sprintf(log_buf,"##NOWHERE extracting char from room (handler.c,char_from_room)[%s]",GET_NAME(ch));
		log_hd(log_buf);
		SUICIDE;
    }
    if(!IS_NPC(ch)||ch->nr==9800){
        zone_table[world[ch->in_room]->zone].population--;
		if(!zone_table[world[ch->in_room]->zone].population)
	    	zone_table[world[ch->in_room]->zone].time_to_empty=1;
	}
   	if (ch->equipment[WEAR_LIGHT])
	if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
	    if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light is ON */
			world[ch->in_room]->light--;
   	if (ch == world[ch->in_room]->people)  /* head of list */
		world[ch->in_room]->people = ch->next_in_room;
   	else    /* locate the previous element */
   	{
		for (i = world[ch->in_room]->people; 
     		i->next_in_room != ch; i = i->next_in_room);

		i->next_in_room = ch->next_in_room;
   	}
    ch->specials.was_in_room_pk=ch->in_room;
    ch->in_room = NOWHERE;
    ch->next_in_room = NULL;
	if(ch->specials.stpMount&&!MOUNTMOVE&&world[ch->specials.was_in_room_pk]->zone!=180){
		ch->specials.stpMount->specials.stpMount=NULL;
		ch->specials.stpMount=NULL;
	}
	
}


/* place a character in a room */
void char_to_room(struct char_data *ch, int room)
{
struct descriptor_data *i=NULL;
struct room_affect *ra=NULL;
char flag=0;

	if(!IS_NPC(ch)&&room==iFlyStoreRoom&&!ch->p->stpFlying){
		send_to_char("FLY CODE BLOCK, please don't do that! :)  \n\r",ch);
		char_to_room(ch,ch->specials.was_in_room_pk);
		return;	
	}
    if(!world[room]){
		send_to_char("ROOM doesn't exist, please tell Vryce exactly what you just did.\n\r",ch);
		room=1;
    }
	if(IS_SET(world[room]->room_flags,BLOCKING)){
		send_to_char(world[room]->description,ch);
		GET_HIT(ch)-=(dice(stpHoloRTemplates[world[room]->holoroom]->hurt_num_die,stpHoloRTemplates[world[room]->holoroom]->hurt_size_die)+stpHoloRTemplates[world[room]->holoroom]->hurt_base);		
		if(GET_HIT(ch)<10)
			GET_HIT(ch)=10;
		char_to_room(ch,ch->specials.was_in_room_pk);
		return;
	}
	    
    if(!IS_NPC(ch)||ch->nr==9800)
    if(world[room]->zone<196)
    if(!zone_table[world[room]->zone].populated){
        global_color=34;
        for (i = descriptor_list; i; i = i->next)
            if (i->character && !i->connected &&
                GET_LEVEL(i->character) > 30)
                if(IS_SET(i->character->specials.god_display,GOD_ZONERESET)){
                    sprintf(log_buf,"(ZONE) %s, Has been entered.\n\r",zone_table[world[room]->zone].name);
                    send_to_char(log_buf,i->character);
                }
        global_color=0;
		reset_zone(world[room]->zone);
    }
    if(world[room]->zone<196)
    if(!IS_NPC(ch)||ch->nr==9800){
		zone_table[world[room]->zone].time_to_empty=0;
        zone_table[world[room]->zone].population++;
    }
    ch->next_in_room = world[room]->people;
    world[room]->people = ch;
    ch->in_room = room;
    ch->specials.direction=0;

	/* DO Sneaking TIMER and RECALL TOWN SETTING */

    if(!IS_NPC(ch)) {
        ch->p->sneaked_room = 0;
	if(zone_table[(int) world[room]->zone].iRecallRoom > 0)
		ch->p->siLastTown = world[room]->zone;
    }	

    if((IS_SET(world[ch->in_room]->room_flags,NEUTRAL)||
	IS_SET(world[ch->in_room]->room_flags,CHAOTIC))&&
	(!IS_SET(world[ch->specials.was_in_room_pk]->room_flags,NEUTRAL)&&
	!IS_SET(world[ch->specials.was_in_room_pk]->room_flags,CHAOTIC))){
	global_color=31;
        if(IS_SET(world[ch->in_room]->room_flags,NEUTRAL))
	    send_to_char("\n\rWARNING! You HAVE ENTERED a NEUTRAL PLAYER KILLING Area!\n\r",ch);
        if(IS_SET(world[ch->in_room]->room_flags,CHAOTIC))
	    send_to_char("\n\rWARNING! You HAVE ENTERED a CHAOTIC PLAYER KILLING Area!\n\r",ch);
	global_color=0;
    }
    else
    if(IS_SET(world[ch->specials.was_in_room_pk]->room_flags,NEUTRAL)&&
	IS_SET(world[ch->in_room]->room_flags,CHAOTIC)){
	global_color=31;
	send_to_char("\n\rWARNING! You HAVE ENTERED a CHAOTIC PLAYER KILLING Area!\n\r",ch);
	global_color=0;
    }
    else
    if((IS_SET(world[ch->specials.was_in_room_pk]->room_flags,NEUTRAL)||
	IS_SET(world[ch->specials.was_in_room_pk]->room_flags,CHAOTIC))&&
	(!IS_SET(world[ch->in_room]->room_flags,CHAOTIC)&&
	!IS_SET(world[ch->in_room]->room_flags,NEUTRAL))){
	global_color=31;
	send_to_char("\n\rNOTICE: You are leaving a PLAYER KILLING Area.\n\r",ch);
	global_color=0;
    }
    if (ch->equipment[WEAR_LIGHT])
	if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
	    if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]) /* Light ON */
		world[room]->light++;
    

    if(IS_AFFECTED(ch,AFF_MAP_CATACOMBS)&&GET_ZONE(ch)==198){
        ra=world[room]->room_afs;
        while(ra){
            if(ra->type==RA_MAP){
                if(ra->ch==ch){
                    ra->value++;
                    flag=1;
                }
            }
	    ra=ra->next;
        }
        if(!flag){
            CREATE(ra,struct room_affect,1);
            ra->type=RA_MAP;
            ra->timer=GET_LEVEL(ch)*4;
            ra->value=1;
            ra->text=str_dup(GET_NAME(ch));
            ra->ch=ch;
            ra->room=room;
            ra->next=world[room]->room_afs;
            world[room]->room_afs=ra;
        }
		
    }
    if(world[room]->zone==197&&ch->desc){
    	if(Holo[world[room]->holox-1][world[room]->holoy]<256){
			iMakeHoloRoom(world[room]->holox-1,world[room]->holoy);    	
    	}
    	if(Holo[world[room]->holox+1][world[room]->holoy]<256){
			iMakeHoloRoom(world[room]->holox+1,world[room]->holoy);    	
    	}
    	if(Holo[world[room]->holox][world[room]->holoy-1]<256){
			iMakeHoloRoom(world[room]->holox,world[room]->holoy-1);    	
    	}
    	if(Holo[world[room]->holox][world[room]->holoy+1]<256){
			iMakeHoloRoom(world[room]->holox,world[room]->holoy+1);    	
    	}
    }
	if(IS_NPC(ch))
		return;
	TouchRooms(room);
	if(ch->p->stpFreight){
	    if(world[room]->dir_option[NORTH]&&world[room]->dir_option[NORTH]->to_room==ch->p->stpFreight->iLocationRoom)
			FreightAutoFollow(ch,room);	
	    else if(world[room]->dir_option[SOUTH]&&world[room]->dir_option[SOUTH]->to_room==ch->p->stpFreight->iLocationRoom)
			FreightAutoFollow(ch,room);	
	    else if(world[room]->dir_option[EAST]&&world[room]->dir_option[EAST]->to_room==ch->p->stpFreight->iLocationRoom)
			FreightAutoFollow(ch,room);	
	    else if(world[room]->dir_option[WEST]&&world[room]->dir_option[WEST]->to_room==ch->p->stpFreight->iLocationRoom)
			FreightAutoFollow(ch,room);	
	    else if(world[room]->dir_option[UP]&&world[room]->dir_option[UP]->to_room==ch->p->stpFreight->iLocationRoom)
			FreightAutoFollow(ch,room);	
	    else if(world[room]->dir_option[DOWN]&&world[room]->dir_option[DOWN]->to_room==ch->p->stpFreight->iLocationRoom)
			FreightAutoFollow(ch,room);	
	}
	if(!IS_NPC(ch)&&ch->specials.stpMount&&MOUNTMOVE){/* move the mount too */
		char_from_room(ch->specials.stpMount);
		char_to_room(ch->specials.stpMount,room);
	}
}


/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch)
{

	if(object->in_room != -1)
		SUICIDE;
	if(object->carried_by)
		SUICIDE;
	if(object->in_obj)
		SUICIDE;
	if(object->worn_by)
    	SUICIDE; 
    object->next_content = ch->carrying;
    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;
	if(!IS_NPC(ch))
		if(object->item_number==10||object->item_number==16)
			ch->p->donated=1;
}


/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
    struct obj_data *tmp=NULL;

    if (object->carried_by->carrying == object)   /* head of list */
	 object->carried_by->carrying = object->next_content;

    else
    {
	for (tmp = object->carried_by->carrying; 
	     tmp && (tmp->next_content != object); 
	      tmp = tmp->next_content); /* locate previous */

	tmp->next_content = object->next_content;
    }

    IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(object->carried_by)--;
    object->carried_by = NULL;
    object->next_content = NULL;
	if(object->in_room != -1)
		SUICIDE;
	if(object->carried_by)
		SUICIDE;
	if(object->in_obj)
		SUICIDE;
	if(object->worn_by) 
   		SUICIDE; 
}



/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data *ch, int eq_pos)
{
  if(!ch->equipment[eq_pos])
	SUICIDE;

  if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
    return 0;

  switch (eq_pos) {

    case WEAR_BODY:
      return (3*ch->equipment[eq_pos]->obj_flags.value[0]);  /* 30% */
    case WEAR_HEAD:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
    case WEAR_LEGS:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
    case WEAR_FEET:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_HANDS:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_ARMS:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_SHIELD:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_FINGER_L:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_FINGER_R:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_NECK_1:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_NECK_2:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_ABOUT:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);  /* 20% */
    case WEAR_WAISTE:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_WRIST_L:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */
    case WEAR_WRIST_R:
      return (ch->equipment[eq_pos]->obj_flags.value[0]);    /* 10% */      
  }
  return 0;
}



void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
    int j;
    if(obj->in_room != -1) 
        SUICIDE; 
    if(obj->carried_by) 
        SUICIDE; 
    if(obj->in_obj) 
        SUICIDE; 
    if(obj->worn_by) 
        SUICIDE; 
    if(pos<0||pos>MAX_WEAR)
		{
		sprintf(log_buf,"##!(ch->equipment[%d]) on %s(obj-%s)",pos,GET_NAME(ch),obj->name);
		log_hd(log_buf);
		SUICIDE;
    	}
    if(ch->equipment[pos])
		{
		sprintf(log_buf,"##!(ch->equipment[%d]) on %s(obj-%s)",pos,GET_NAME(ch),obj->name);
		log_hd(log_buf);
    	SUICIDE;
		}

    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
	if (ch->in_room != NOWHERE) {

	    act("Electricity arcs from $p, striking you. You instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
	    act("Electricity arcs from $p, striking $n; $e instantly drops it.", FALSE, ch, obj, 0, TO_ROOM);
	    obj_to_room(obj, ch->in_room);
            GET_HIT(ch)-=dice(3,10);
            if(GET_HIT(ch)<1)
              GET_HIT(ch)=1;
	    return;
	} else {
	    log_hd("##ch->in_room = NOWHERE when equipping char.");
	}
    }

    ch->equipment[pos] = obj;
	obj->worn_by = ch;

    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	GET_AC(ch) -= apply_ac(ch, pos);

    for(j=0; j<MAX_OBJ_AFFECT; j++)
	affect_modify(ch, obj->affected[j].location,
	  obj->affected[j].modifier,
	  obj->obj_flags.bitvector, TRUE);

    affect_total(ch);
}



struct obj_data *unequip_char(struct char_data *ch, int pos)
{
    int j;
    struct obj_data *obj=NULL;

    if( (pos < 0) || (pos >= MAX_WEAR) )
		SUICIDE;

	if(!ch->equipment[pos]){
		sprintf(log_buf,"## ASSERT unequip_char pos=[%d] ch=[%s]",pos,GET_NAME(ch));
		log_hd(log_buf);
		if(!ch->equipment[pos])
			SUICIDE;
	}
    obj = ch->equipment[pos];
    obj->worn_by = NULL;

    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	GET_AC(ch) += apply_ac(ch, pos);

    ch->equipment[pos] = NULL;

    for(j=0; j<MAX_OBJ_AFFECT; j++)
	affect_modify(ch, obj->affected[j].location,
	  obj->affected[j].modifier,
	  obj->obj_flags.bitvector, FALSE);

    affect_total(ch);
    return(obj);
}


int get_number(char **name)
{
    int i;
    char *ppos;
    char number[MAX_INPUT_LENGTH];
    
    if ( ( ppos = index(*name, '.') ) != NULL )
    {
	*ppos++ = '\0';
	strcpy(number,*name);
	strcpy(*name, ppos);

	for(i=0; *(number+i); i++)
	    if (!isdigit(*(number+i)))
		return(0);

	return(atoi(number));
    }

    return(1);
}


/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
    struct obj_data *i=NULL;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
 
    for (i = list, j = 1; i && (j <= number); i = i->next_content)
	if (isname(tmp, i->name)) {
	    if (j == number) 
		return(i);
	    j++;
	}

    return(0);
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
    struct obj_data *i=NULL;

    for (i = list; i; i = i->next_content)
	if (i->item_number == num) 
	    return(i);
	
    return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(char *name)
{
    struct obj_data *i=NULL;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

    for (i = object_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, i->name)) {
	    if (j == number)
		return(i);
	    j++;
	}

    return(0);
}


/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
    struct obj_data *i=NULL;

    for (i = object_list; i; i = i->next)
	if (i->item_number == nr) 
	    return(i);

    return(0);
}





/* search a room for a char, and return a pointer if found..  */

struct char_data *get_char_room(char *name, int room)
{
    struct char_data *i=NULL;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
    return(0);

    for (i = world[room]->people, j = 1; i && (j <= number); i = i->next_in_room)
	if (isname(tmp, GET_NAME(i))) {
	    if (j == number)
	return(i);
	    j++;
	}

    return(0);
}



struct char_data *get_player(char *name)
{
 struct char_data *i=NULL;

 for(i = character_list; i; i=i->next)
	if(!IS_NPC(i) && isname(name,GET_NAME(i))) 
		return(i);

 return(NULL);
}

/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(char *name)
{
    struct char_data *i=NULL;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);

    for (i = character_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, GET_NAME(i)) && (i->in_room != NOWHERE) ) {
	    if (j == number)
		return(i);
	    j++;
	}

    return(0);
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
    struct char_data *i=NULL;

    for (i = character_list; i; i = i->next)
	if (i->nr == nr)
	    return(i);

    return(0);
}




/* put an object in a room */
void obj_to_room(struct obj_data *object, int room)
{
if(object->in_room != -1)
	SUICIDE;
if(object->carried_by)
	SUICIDE;
if(object->in_obj)
	SUICIDE;
if(object->worn_by) 
    SUICIDE; 

if( (room < 0) || (room >= MAX_ROOM) )
	{
	sprintf(log_buf,"Room [%d] out of bounds in obj_to_room.  object(%s)",
		room,object->name);
	log_hd(log_buf);
	SUICIDE;
	}

object->next_content = world[room]->contents;
world[room]->contents = object;
object->in_room = room;
object->carried_by = NULL;
}


/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
    struct obj_data *i=NULL;

    /* remove object from room */

    if (object == world[object->in_room]->contents)  /* head of list */
       world[object->in_room]->contents = object->next_content;

    else     /* locate previous element in list */
    {
	for (i = world[object->in_room]->contents; i && 
	   (i->next_content != object); i = i->next_content);

	i->next_content = object->next_content;
    }

    object->in_room = NOWHERE;
    object->next_content = NULL;
	if(object->in_room != -1)
		SUICIDE;
	if(object->carried_by)
		SUICIDE;
	if(object->in_obj)
		SUICIDE;
	if(object->worn_by) 
    	SUICIDE; 
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
    struct obj_data *tmp_obj=NULL;

	if(obj->in_room != -1)
	    SUICIDE; 
	if(obj->carried_by)
	    SUICIDE; 
	if(obj->in_obj)
	    SUICIDE; 
	if(obj->worn_by) 
	    SUICIDE; 

    obj->next_content = obj_to->contains;
    obj_to->contains = obj;
    obj->in_obj = obj_to;

    for(tmp_obj = obj->in_obj; tmp_obj;
      GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj), tmp_obj = tmp_obj->in_obj);
}


/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
struct obj_data *tmp=NULL, *obj_from=NULL;

if (obj->in_obj)
	{
	obj_from = obj->in_obj;
	if (obj == obj_from->contains)   /* head of list */
	   obj_from->contains = obj->next_content;
	else
		{
	    for (tmp = obj_from->contains; 
			tmp && (tmp->next_content != obj);
			tmp = tmp->next_content); /* locate previous */
	    if (!tmp)
			SUICIDE;
	    tmp->next_content = obj->next_content;
		}

	/* Subtract weight from containers container */
	for(tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj)
	    GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

	GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

	/* Subtract weight from char that carries the object */
	if (tmp->carried_by)
	    IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);

	obj->in_obj = NULL;
	obj->next_content = NULL;
    }
else
	{
	perror("Trying to object from object when in no object.");
	SUICIDE;
    }

if(obj->in_room != -1)
    SUICIDE; 
if(obj->carried_by)
    SUICIDE; 
if(obj->in_obj)
    SUICIDE; 
if(obj->worn_by) 
    SUICIDE; 
}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
    if (list) {
	object_list_new_owner(list->contains, ch);
	object_list_new_owner(list->next_content, ch);
	list->carried_by = ch;
    }
}

/* Extract an object from the world */
void extract_obj(struct obj_data *obj)
{
    if(obj->in_room != NOWHERE)
 		obj_from_room(obj);
    else if(obj->carried_by)
		obj_from_char(obj);
    else if(obj->in_obj)
		obj_from_container(obj);

    /* if its still carried, worn, etc at this point then give up */
    if(obj->in_room != -1)
		SUICIDE;
    if(obj->carried_by)
		SUICIDE;
    if(obj->in_obj)
		SUICIDE;
    if(obj->worn_by)
		SUICIDE;

    while(obj->contains)
		extract_obj(obj->contains);

	if(obj->item_number>=0)
		(obj_index[obj->item_number].number)--;

	if(obj->next)
		obj->next->prev = obj->prev;
	if(obj->prev)
		obj->prev->next = obj->next;
/*mike added next line friday 8:20pm */
        if(object_list==obj)
	        object_list=obj->next;

	free_obj(obj);
}


/* add object to head of object list */
void add_object(struct obj_data *obj)
{
obj->prev = NULL;
obj->next = object_list;
if(object_list)
	object_list->prev = obj;
object_list = obj;
}

bool in_container(struct obj_data *obj, struct obj_data *container)
{
if (!obj->in_obj)
	return (FALSE);
else if (obj->in_obj == container)
	return (TRUE);
else
	return (in_container(obj->in_obj, container));
}
	

void obj_from_container(struct obj_data *obj)
{
struct obj_data *this_object;

    this_object = obj->in_obj;
    obj->in_obj = NULL;
    if(this_object->contains == obj){
	this_object->contains = obj->next_content;
    }else{
	this_object=this_object->contains;
	while((this_object) && (this_object->next_content != obj))
	    this_object=this_object->next_content;
	if(this_object){
	    this_object->next_content = obj->next_content;
	    return;
	}else{
	    sprintf(log_buf, "## Object #:  %d not in container", obj->item_number);
	    log_hd(log_buf);
	    SUICIDE;
	}
    }
}

void update_object( struct obj_data *obj, int use){

    if (obj->obj_flags.timer > 0)   obj->obj_flags.timer -= use;
    if (obj->contains) update_object(obj->contains, use);
    if (obj->next_content) update_object(obj->next_content, use);
}

void update_char_objects( struct char_data *ch )
{

    int i;

    if (ch->equipment[WEAR_LIGHT])
	if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
	    if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] > 0) {
		(ch->equipment[WEAR_LIGHT]->obj_flags.value[2])--;
		if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2]==0) {
			act("You're light source flickers out.", FALSE, ch, 0, 0, TO_CHAR);
			world[ch->in_room]->light--;
		}
	    }

    for(i = 0;i < MAX_WEAR;i++) 
	if (ch->equipment[i])
	    update_object(ch->equipment[i],2);

    if (ch->carrying) update_object(ch->carrying,1);
}



/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data *ch, bool pull)
{
    struct obj_data *obj=NULL;
    struct char_data *k=NULL, *next_char=NULL;
    struct descriptor_data *t_desc=NULL;
    int l, was_in,iCirclingDragon=FALSE;
    extern struct char_data *combat_list;

	if( (!ch) )
		SUICIDE;

    if ( !IS_NPC(ch) && !ch->desc )
    {
		for ( t_desc = descriptor_list; t_desc; t_desc = t_desc->next )
		{
			if ( t_desc->original == ch )
				do_return( t_desc->character, "", 0 );
		}
    }
	if(IS_SET((ch)->player.siMoreFlags,DRAGON)){
		if(ch->in_room <1){
			iCirclingDragon=TRUE;
		}
		RemoveDragon(ch);
	}
	if ( ch->in_room == NOWHERE &&!iCirclingDragon)
    	{
		log_hd( "##Extract_char: NOWHERE" );
   		SUICIDE;
		}

	die_formation(ch);

    while ( ( obj = ch->carrying ) != NULL )
    {
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
    }
    
    if ( ch->specials.fighting )
	stop_fighting( ch );

    for ( k = combat_list; k ; k = next_char )
    {
	next_char = k->next_fighting;
	if ( k->specials.fighting == ch )
	    stop_fighting( k );
    }

    /* Must remove from room before removing the equipment! */
	if(!iCirclingDragon){
		was_in = ch->in_room;
		if ( ch->in_room != NOWHERE )
			char_from_room( ch );
		if ( !pull )
			char_to_room(ch, 1);
	}

    /* clear equipment_list */
    for ( l = 0; l < MAX_WEAR; l++ )
    {
	if ( ch->equipment[l] )
	    obj_to_room(unequip_char(ch,l), was_in);
    }

    GET_AC(ch) = 100;

    if ( ch->desc && ch->desc->original && !IS_UNDEAD(ch))
	do_return( ch, "", 0 );

    if ( IS_NPC(ch) && ch->nr > -1 )
	mob_index[ch->nr].number--;

    if ( pull )
    {
	if ( ch == character_list )
	   character_list = ch->next;
	else
	{
	    for ( k = character_list; k && k->next != ch; k = k->next )
		;
	    if ( k )
		k->next = ch->next;
	    else
	    {
		log_hd( "##Extract_char: bad char list" );
		SUICIDE;
	    }
	}
	free_char( ch );
    }
}
void extract_char_neutral_zone(struct char_data *ch, bool pull)
{
    struct char_data *k=NULL, *next_char=NULL;
    struct descriptor_data *t_desc=NULL;
    int was_in;

    extern struct char_data *combat_list;


    if ( !IS_NPC(ch) && !ch->desc )
    {
	for ( t_desc = descriptor_list; t_desc; t_desc = t_desc->next )
	{
	    if ( t_desc->original == ch )
		do_return( t_desc->character, "", 0 );
	}
    }

    if ( ch->in_room == NOWHERE )
    {
	log_hd( "##Extract_char: NOWHERE" );
    SUICIDE;
	}

	die_formation(ch);

    if ( ch->specials.fighting )
	stop_fighting( ch );

    for ( k = combat_list; k ; k = next_char )
    {
	next_char = k->next_fighting;
	if ( k->specials.fighting == ch )
	    stop_fighting( k );
    }

    /* Must remove from room before removing the equipment! */
    was_in = ch->in_room;
    char_from_room( ch );
    if ( !pull )
	char_to_room(ch, 1);

    /* clear equipment_list */
    if ( ch->desc && ch->desc->original && !IS_UNDEAD(ch))
	do_return( ch, "", 0 );

    if ( IS_NPC(ch) && ch->nr > -1 )
	mob_index[ch->nr].number--;

}




struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
    struct char_data *i=NULL;
    int j, number;
  char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
    return(0);

    for (i = world[ch->in_room]->people, j = 1; i && (j <= number); i =
i->next_in_room)
	if (isname(tmp, GET_NAME(i)))
	    if (CAN_SEE(ch, i)) {
		if (j == number) 
		    return(i);
		j++;
	    }

    return(0);
}





struct char_data *get_char_vis(struct char_data *ch, char *name)
{
    struct char_data *i=NULL;
    int j, number;
  char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    /* check location */
    if ( ( i = get_char_room_vis(ch, name) ) != 0 )
	return(i);

  strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);

    for (i = character_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, GET_NAME(i)))
	    if (CAN_SEE(ch, i) && (i->in_room != NOWHERE) ) {
		if (j == number)
		    return(i);
		j++;
	    }

    return(0);
}






struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, 
		struct obj_data *list)
{
    struct obj_data *i=NULL;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname,name);
    tmp = tmpname;
    if(!(number = get_number(&tmp)))
	return(0);

    for (i = list, j = 1; i && (j <= number); i = i->next_content)
	if (isname(tmp, i->name))
	    if (CAN_SEE_OBJ(ch, i)) {
		if (j == number)
		    return(i);
		j++;
	    }
    return(0);
}





/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
    struct obj_data *i=NULL;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    /* scan items carried */
    if ( ( i = get_obj_in_list_vis(ch, name, ch->carrying) ) != NULL )
	return(i);

    /* scan room */
    if ( ( i = get_obj_in_list_vis(ch, name, world[ch->in_room]->contents) )
    != NULL )
	return(i);

    strcpy(tmpname,name);
    tmp = tmpname;
    if ( ( number = get_number(&tmp) ) == 0 )
	return(0);

    /* ok.. no luck yet. scan the entire obj list   */
    for (i = object_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, i->name))
	    if (CAN_SEE_OBJ(ch, i)) {
		if (j == number)
		    return(i);
		j++;
	    }
    return(0);
}


struct obj_data *create_money( int amount )
{
    struct obj_data *obj=NULL;
    struct extra_descr_data *new_descr=NULL;
    char buf[80];


    if(amount<=0)
    {
	log_hd("##ERROR: Try to create negative money.");
    SUICIDE;
	}

    CREATE(obj, struct obj_data, 1);
    CREATE(new_descr, struct extra_descr_data, 1);
    clear_object(obj);

    if(amount==1)
    {
	obj->name = str_dup("coin gold");
	obj->short_description = str_dup("a gold coin");
	obj->description = str_dup("One miserable gold coin.");

	new_descr->keyword = str_dup("coin gold");
	new_descr->description = str_dup("One miserable gold coin.");
    }
    else
    {
	obj->name = str_dup("coins gold");
	obj->short_description = str_dup("gold coins");
	obj->description = str_dup("A pile of gold coins.");

	new_descr->keyword = str_dup("coins gold");
	if(amount<10) {
	    sprintf(buf,"There is %d coins.",amount);
	    new_descr->description = str_dup(buf);
	} 
	else if (amount<100) {
	    sprintf(buf,"There is about %d coins",10*(amount/10));
	    new_descr->description = str_dup(buf);
	}
	else if (amount<1000) {
	    sprintf(buf,"It looks like something round %d coins",100*(amount/100));
	    new_descr->description = str_dup(buf);
	}
	else if (amount<100000) {
	    sprintf(buf,"You guess there is %d coins",1000*((amount/1000)+ number(0,(amount/1000))));
	    new_descr->description = str_dup(buf);
	}
	else 
	    new_descr->description = str_dup("There are A LOT of coins");
    }

    new_descr->next = NULL;
    obj->ex_description = new_descr;

    obj->obj_flags.type_flag = ITEM_MONEY;
    obj->obj_flags.wear_flags = ITEM_TAKE;
    obj->obj_flags.value[0] = amount;
    obj->obj_flags.cost = amount;
    obj->item_number = -1;

	add_object(obj);

    return(obj);
}



/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, struct char_data *ch,
		   struct char_data **tar_ch, struct obj_data **tar_obj)
{
    static char *ignore[] = {
	"the",
	"in",
	"on",
	"at",
	"\n" };

    int i;
    char name[256];
    bool found;

    found = FALSE;


    /* Eliminate spaces and "ignore" words */
    while (*arg && !found) {

	for(; *arg == ' '; arg++)   ;

	for(i=0; (name[i] = *(arg+i)) && (name[i]!=' '); i++)   ;
	name[i] = 0;
	arg+=i;
	if (search_block(name, ignore, TRUE) > -1)
	    found = TRUE;

    }

    if (!name[0])
	return(0);

    *tar_ch  = NULL;
    *tar_obj = NULL;

    if (IS_SET(bitvector, FIND_CHAR_ROOM)) {      /* Find person in room */
	if ( ( *tar_ch = get_char_room_vis(ch, name) ) != NULL ) {
	    return(FIND_CHAR_ROOM);
	}
    }

    if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
	if ( ( *tar_ch = get_char_vis(ch, name) ) != NULL ) {
	    return(FIND_CHAR_WORLD);
	}
    }

    if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
	for(found=FALSE, i=0; i<MAX_WEAR && !found; i++)
	    if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
		*tar_obj = ch->equipment[i];
		found = TRUE;
	    }
	    if (found) {
		return(FIND_OBJ_EQUIP);
	    }
    }

    if (IS_SET(bitvector, FIND_OBJ_INV)) {
	if ( ( *tar_obj = get_obj_in_list_vis(ch, name, ch->carrying) )
	!= NULL ) {
	    return(FIND_OBJ_INV);
	}
    }

    if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
	*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room]->contents);
	if ( *tar_obj != NULL ) {
	    return(FIND_OBJ_ROOM);
	}
    }

    if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
	if ( ( *tar_obj = get_obj_vis(ch, name) ) != NULL ) {
	    return(FIND_OBJ_WORLD);
	}
    }

    return(0);
}

void put_all_in_inv(struct char_data *ch)
{
int no_containers, found, iWear;
struct obj_data *obj=NULL, *item=NULL;

/* remove all eq */
for(iWear=0; iWear<MAX_WEAR; iWear ++)
    if(ch->equipment[iWear])
        obj_to_char(unequip_char(ch,iWear), ch);

/* empty containers */
no_containers=FALSE;
while (!no_containers)
	{
	obj=ch->carrying;
	found=FALSE;
	while((!found) && obj)
		{
		if( (GET_ITEM_TYPE(obj) == ITEM_CONTAINER) && obj->contains)
			{
			while(obj->contains)
				{
				item=obj->contains;
 				obj_from_obj(item);
				obj_to_char(item, ch);
				}
			found=TRUE;
			}
		else
			{
			obj=obj->next_content;
			}
		}
	if(!obj)
		{
		no_containers=TRUE;
		}
	}
}

void prepare_for_quit(struct char_data *ch, bool pull)
{
        struct obj_data *obj=NULL;
        struct char_data *k=NULL;
        struct char_data *next_char=NULL;
        struct descriptor_data *t_desc=NULL;
        int l, was_in;

        extern struct char_data *combat_list;
        sprintf(log_buf, "EXTRACTED! home_number=%d", ch->specials.home_number);        strcpy(ch->specials.afk_text, log_buf);

        if (!IS_NPC(ch) && !ch->desc)
        {
                for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
                {
                        if (t_desc->original == ch)
                                do_return(t_desc->character, "", 0);
                }
        }

        if(ch->in_room == NOWHERE)
                log_hd("##prepare_char:  NOWHERE");

        die_formation(ch);

        while((obj = ch->carrying) != NULL)
                {
                obj_from_char(obj);
                obj_to_room(obj, ch->in_room);
                }

        if(ch->specials.fighting)
                stop_fighting(ch);

        for (k=combat_list; k; k=next_char)
                {
                next_char=k->next_fighting;
                if(k->specials.fighting == ch)
                        stop_fighting(k);
                }

        was_in=ch->in_room;
        if(ch->in_room != NOWHERE)
                char_from_room(ch);
        if (!pull)
                char_to_room(ch, 1);
        for (l = 0; l<MAX_WEAR; l++)
                if(ch->equipment[1])
                        obj_to_room(unequip_char(ch,1), was_in);

        GET_AC(ch) = 100;

        if (ch->desc && ch->desc->original && !IS_UNDEAD(ch))
                do_return(ch, "", 0);

        if (IS_NPC(ch) && ch->nr > -1)
                mob_index[ch->nr].number--;

        if(pull)
                {
                if(ch==character_list)
                        character_list = ch->next;
                else
                        {
                        for (k = character_list; k && k->next != ch; k= k->next)                                ; /*find the char in the list*/
                        if (k)
                                k->next = ch->next;
                        else
                                {
                                log_hd("##prepare_for_quit:  bad char list");
                                SUICIDE;
                                }
                        }
                }
}

/* perform a sanity check on object list by checking for loops.
 * If one is found, then core dump
 */
void sanity_check_object_list()
{
register struct obj_data *cur;

return;

    cur = object_list;
    while(cur && !cur->insane){
	if((cur->in_room != NOWHERE) && !world[cur->in_room])
		SUICIDE;
	if((cur->carried_by) && !GET_NAME(cur->carried_by))
		SUICIDE;
	if((cur->in_obj)&&((cur->item_number<-1)||(cur->item_number> MAX_OBJ)))
		SUICIDE;
	cur->insane = TRUE;
	cur = cur->next;
    }
    if(cur && cur->insane)
		SUICIDE;
    else{
	cur=object_list;
	while(cur){
	    cur->insane = FALSE;
	    cur = cur->next;
	}
    }
}
