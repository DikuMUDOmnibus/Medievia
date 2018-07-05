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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

/* extern variables */

extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct descriptor_data *descriptor_list;
extern struct str_app_type str_app[];
extern bool in_a_shop(struct char_data *ch);
	 
/* extern functions */
char *fname(char *namelist);
int isname(char *arg, char *arg2);
struct obj_data *create_money( int amount );
void do_put(struct char_data *ch, char *argument, int cmd);
bool TradeGet(struct char_data *stpCh, char *szpName);

/* internal functions */
bool can_store(struct char_data *, struct obj_data *);
void do_stuffit(struct char_data *, char *);

/* Internal Globals */
bool stuffitvar=FALSE;

/* procedures related to get */
void get(struct char_data *ch, struct obj_data *obj_object, 
    struct obj_data *sub_object)
{
    char buffer[MAX_STRING_LENGTH];
    int gold;

    if((obj_object->obj_flags.type_flag == ITEM_MONEY) &&
	(!world[ch->in_room]->funct) &&
	(obj_object->obj_flags.value[0]>=1)&&(GET_LEVEL(ch)<31))
    {
       
if(GET_GOLD(ch)+obj_object->obj_flags.value[0]>((GET_LEVEL(ch)*GET_LEVEL(ch))*5555)){
	   if(GET_GOLD(ch)>=((GET_LEVEL(ch)*GET_LEVEL(ch))*5555)){
		send_to_char("You try and get more gold but can't carry any more.\n\r",ch);
		return;
	   }
	   gold=((GET_LEVEL(ch)*GET_LEVEL(ch))*5555)-GET_GOLD(ch);
	   sprintf(buffer,"You get all you can carry, [%d] coins.\n\r",
		gold);
	   send_to_char(buffer,ch);
	   act("$n gets some gold.",1,ch,0,0,TO_ROOM);	   
	   GET_GOLD(ch) +=gold;
	   obj_object->obj_flags.value[0]-=gold;
	   return;
	}
    }
    if (sub_object) {
	obj_from_obj(obj_object);
	obj_to_char(obj_object, ch);
	if (sub_object->carried_by == ch) {
	    act("You get $p from $P.", 0, ch, obj_object, sub_object,
		TO_CHAR);
	    act("$n gets $p from $P.", 1, ch, obj_object, sub_object,
		TO_ROOM);
	} else {
	    act("You get $p from $P.", 0, ch, obj_object, sub_object,
		TO_CHAR);

	    act("$n gets $p from $P.", 1, ch, obj_object, sub_object, TO_ROOM);
	}
    } else {
	obj_from_room(obj_object);
	obj_to_char(obj_object, ch);
	act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
	act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
    }
    if((obj_object->obj_flags.type_flag == ITEM_MONEY) && 
	(obj_object->obj_flags.value[0]>=1))
    {
	obj_from_char(obj_object);
	sprintf(buffer,"There are %d coins.\n\r",
		obj_object->obj_flags.value[0]);
	send_to_char(buffer,ch);
	GET_GOLD(ch) += obj_object->obj_flags.value[0];
	extract_obj(obj_object);
    }
}


void do_get(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *sub_object=NULL;
    struct obj_data *obj_object=NULL;
    struct obj_data *next_obj=NULL;
    bool found = FALSE;
    bool fail  = FALSE;
    int type   = 3;
    bool alldot = FALSE;
    char allbuf[MAX_STRING_LENGTH];

    argument_interpreter(argument, arg1, arg2);

    /* get type */
    if (!*arg1) {
	type = 0;
    }
    if (*arg1 && !*arg2) {
      alldot = FALSE;
      allbuf[0] = '\0';
      if ((str_cmp(arg1, "all") != 0) &&
	  (sscanf(arg1, "all.%s", allbuf) != 0)){
	strcpy(arg1, "all");
	alldot = TRUE;
      }
      if (!str_cmp(arg1,"all")) {
	type = 1;
      } else {
	type = 2;
      }
    }
    if (*arg1 && *arg2) {
      alldot = FALSE;
      allbuf[0] = '\0';
      if ((str_cmp(arg1, "all") != 0) &&
	      (sscanf(arg1, "all.%s", allbuf) != 0)){
	    strcpy(arg1, "all");
	    alldot = TRUE;
	  }
      if (!str_cmp(arg1,"all")) {
	if (!str_cmp(arg2,"all")) {
	  type = 3;
	} else {
	  type = 4;
	}
      } else {
	if (!str_cmp(arg2,"all")) {
	  type = 5;
	} else {
	  type = 6;
	}
      }
    }

    switch (type) {
	/* get */
	case 0:{ 
	    send_to_char("Get what?\n\r", ch); 
	} break;
	/* get all */
	case 1:{ 
	    if ( (ch->in_room == 3) || (ch->in_room == 4553) )
		{
		send_to_char("Don't be so greedy!\n\r",ch);
		return;
		}
	    sub_object = 0;
	    found = FALSE;
	    fail    = FALSE;
	    for(obj_object = world[ch->in_room]->contents;
		obj_object;
		obj_object = next_obj) {
		next_obj = obj_object->next_content;

		/* IF all.obj, only get those named "obj" */
		if (alldot && !isname(allbuf,obj_object->name)) {
		  continue;
		}
		if (GET_LEVEL(ch) > 34) {
		  get(ch,obj_object,sub_object);
		  continue;
		  found = TRUE;
		}
                if (CAN_SEE_OBJ(ch,obj_object))
                    {
                    if (!CAN_WEAR(obj_object,ITEM_TAKE))
                        {
                        send_to_char("You can't take that.\n\r", ch);
                        fail = TRUE;
                        }
                    else if ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)) 
                        {
                        if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight)
                                <= CAN_CARRY_W(ch)) 
                            {
                            get(ch,obj_object,sub_object);
                            found = TRUE;
                            } 
                        else 
                           {
                            sprintf(buffer,
                                "%s : You can't carry that much weight.\n\r",
                                fname(obj_object->name));
                            send_to_char(buffer, ch);
                            fail = TRUE;
                            }
                        } 
                     else 
                        {
                        sprintf(buffer,
                                "%s : You can't carry that many items.\n\r",
                                fname(obj_object->name));
                        send_to_char(buffer, ch);
                        fail = TRUE;
                        }
                     }
	    }
	    if (!found && !fail)
		send_to_char("You see nothing here.\n\r", ch);
	} break;
	/* get ??? */
	case 2:{
	    sub_object = 0;
	    found = FALSE;
	    fail    = FALSE;
	    obj_object = get_obj_in_list_vis(ch, arg1, 
		world[ch->in_room]->contents);
	    if (obj_object) {
	  	if (GET_LEVEL(ch) >34) {
		  get(ch, obj_object, sub_object);
		  return;
		}
		if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
		    if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <= 
			CAN_CARRY_W(ch)) {
			if (CAN_WEAR(obj_object,ITEM_TAKE)) {
			    get(ch,obj_object,sub_object);
			    found = TRUE;
			} else {
			    send_to_char("You can't take that.\n\r", ch);
			    fail = TRUE;
			}
		    } else {
			sprintf(buffer,
				"%s : You can't carry that much weight.\n\r", 
			    fname(obj_object->name));
			send_to_char(buffer, ch);
			fail = TRUE;
		    }
		} else {
		    sprintf(buffer,
			"%s : You can't carry that many items.\n\r", 
			fname(obj_object->name));
		    send_to_char(buffer, ch);
		    fail = TRUE;
		}
	    } else {
		if(TradeGet(ch,arg1)){
			found=TRUE;
		}else{
		sprintf(buffer,"You do not see a %s here.\n\r", arg1);
		send_to_char(buffer, ch);
		fail = TRUE;
		}
	    }
	} break;
	/* get all all */
	case 3:{ 
	    send_to_char("You must be joking?!\n\r", ch);
	} break;
	/* get all ??? */
	case 4:{
	    found = FALSE;
	    fail    = FALSE; 
	    sub_object = get_obj_in_list_vis(ch, arg2, 
		world[ch->in_room]->contents);
	    if (!sub_object){
		sub_object = get_obj_in_list_vis(ch, arg2, ch->carrying);
	    }
	    if (sub_object) {
		if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
		  if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)){
		    sprintf(buffer,
			"The %s is closed.\n\r",fname(sub_object->name));
		    send_to_char(buffer, ch);
		    return;
		  }
		  for(obj_object = sub_object->contains;
		      obj_object;
		      obj_object = next_obj) {
		    next_obj = obj_object->next_content;

		    /* IF all.obj, only get those named "obj" */
		    if (alldot && !isname(allbuf,obj_object->name)){
		      continue;
		    }
		    if (CAN_SEE_OBJ(ch,obj_object)) {
		      if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
		    if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < 
			CAN_CARRY_W(ch)) {
		      if (CAN_WEAR(obj_object,ITEM_TAKE)) {
			get(ch,obj_object,sub_object);
			found = TRUE;
		      } else {
			send_to_char("You can't take that\n\r", ch);
			fail = TRUE;
		      }
		    } else {
		      sprintf(buffer,
		      "%s : You can't carry that much weight.\n\r", 
			  fname(obj_object->name));
		      send_to_char(buffer, ch);
		      fail = TRUE;
		    }
		      } else {
		    sprintf(buffer,"%s : You can't carry that many items.\n\r", 
			fname(obj_object->name));
		    send_to_char(buffer, ch);
		    fail = TRUE;
		      }
		    }
		  }
		  if (!found && !fail) {
		    sprintf(buffer,"You do not see anything in the %s.\n\r", 
			fname(sub_object->name));
		    send_to_char(buffer, ch);
		    fail = TRUE;
		  }
		} else {
		  sprintf(buffer,"The %s is not a container.\n\r",
		      fname(sub_object->name));
		  send_to_char(buffer, ch);
		  fail = TRUE;
		}
		  } else { 
		sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
		send_to_char(buffer, ch);
		fail = TRUE;
		  }
	      } break;
	case 5:{ 
	  send_to_char(
	    "You can't take a thing from more than one container.\n\r", ch);
	} break;
	case 6:{
	  found = FALSE;
	  fail  = FALSE;
	  sub_object = get_obj_in_list_vis(ch, arg2, 
			   world[ch->in_room]->contents);
	  if (!sub_object){
	    sub_object = get_obj_in_list_vis(ch, arg2, ch->carrying);
	  }
	  if (sub_object) {
	    if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
	      if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)){
	    sprintf(buffer,"The %s is closed.\n\r", fname(sub_object->name));
	    send_to_char(buffer, ch);
	    return;
	      }
	      obj_object = get_obj_in_list_vis(ch, arg1, sub_object->contains);
	      if (obj_object) {
	    if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	      if ((IS_CARRYING_W(ch) + obj_object->obj_flags.weight) < 
		  CAN_CARRY_W(ch)) {
		if (CAN_WEAR(obj_object,ITEM_TAKE)) {
		  get(ch,obj_object,sub_object);
		  found = TRUE;
		} else {
		  send_to_char("You can't take that\n\r", ch);
		  fail = TRUE;
		}
	      } else {
		sprintf(buffer,"%s : You can't carry that much weight.\n\r", 
		    fname(obj_object->name));
		send_to_char(buffer, ch);
		fail = TRUE;
	      }
	    } else {
	      sprintf(buffer,"%s : You can't carry that many items.\n\r", 
		  fname(obj_object->name));
	      send_to_char(buffer, ch);
	      fail = TRUE;
	    }
	      } else {
	    sprintf(buffer,"The %s does not contain the %s.\n\r", 
		fname(sub_object->name), arg1);
	    send_to_char(buffer, ch);
	    fail = TRUE;
	      }
	    } else {
	      sprintf(buffer,
	      "The %s is not a container.\n\r", fname(sub_object->name));
	      send_to_char(buffer, ch);
	      fail = TRUE;
	    }
	  } else {
	    sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
	    send_to_char(buffer, ch);
	    fail = TRUE;
	  }
	} break;
	}
      }


void do_drop(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    int amount;
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *tmp_object=NULL;
    struct obj_data *next_obj=NULL;
    bool test = FALSE;
	
    if(ch->desc)
	if(ch->desc->connected == CON_SOCIAL_ZONE) {
		send_to_char("You can't drop things here!\r\n",ch);
		return;
	}

    argument=one_argument(argument, arg);
    if(is_number(arg))
    {
      if(strlen(arg)>7&&cmd!=1103){/*cmd 1103 means this was called from casino code */
	send_to_char("Number field too big.\n\r", ch);
	return;
      }
      amount = atoi(arg);
      argument=one_argument(argument,arg);
      if (str_cmp("coins",arg) && str_cmp("coin",arg))
	{
	  send_to_char("Sorry, you can't do that...\n\r",ch);
	  return;
	}
      if(amount<0)
	{
	  send_to_char("Sorry, you can't do that!\n\r",ch);
	  return;
	}
      if(GET_GOLD(ch)<amount)
	{
	  send_to_char("You haven't got that many coins!\n\r",ch);
	  return;
	}
      if(IS_NPC(ch))return;
/* juice
 * added the amount of coins droppped to log message
 */
      if(cmd!=1103){/*if ! called by casino code */
      	   sprintf(log_buf,"%s dropped %d coin[s]",GET_NAME(ch), amount);
           log_hd(log_buf);
      }
      if(ch->specials.dropped_coins>40){
	   send_to_char("I assume you want to drop ALL your coins!\n\r",ch);
	   GET_GOLD(ch)=0;
	   sprintf(log_buf,"### %s dropped coins over 25 times!",GET_NAME(ch));
	   log_hd(log_buf);
	   raw_kill(ch,NULL);
	   return; 
      }
      if(cmd!=1103)
          send_to_char("OK.\n\r",ch);
      ch->specials.dropped_coins++;
      if(amount==0)
	return;
      if(cmd!=1103)
          act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
      tmp_object = create_money(amount);
      obj_to_room(tmp_object,ch->in_room);
      GET_GOLD(ch)-=amount;
      return;
    }

    if (*arg) {
	if (!str_cmp(arg,"all")) {
	    for(tmp_object = ch->carrying;
		tmp_object;
		tmp_object = next_obj) {
		next_obj = tmp_object->next_content;
		if (! IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
		    if (CAN_SEE_OBJ(ch, tmp_object)) {
			sprintf(buffer,
			    "You drop the %s.\n\r", fname(tmp_object->name));
			send_to_char(buffer, ch);
		    } else {
			send_to_char("You drop something.\n\r", ch);
		    }
		    act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
		    obj_from_char(tmp_object);
		    obj_to_room(tmp_object,ch->in_room);
		    test = TRUE;
		} else {
		    if (CAN_SEE_OBJ(ch, tmp_object)) {
			if(tmp_object->item_number ==10||tmp_object->item_number==16)
			    strcpy(buffer,"You would never dream of dropping such a precious keepsake.\n\r");
			else
			sprintf(buffer,
			    "You can't drop the %s, it must be CURSED!\n\r",
			    fname(tmp_object->name));
			send_to_char(buffer, ch);
			test = TRUE;
		    }
		}
	    }
	    if (!test) {
		send_to_char("You do not seem to have anything.\n\r", ch);
	    }
     } else {
	    tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
	    if (tmp_object) {
		if (! IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
		    sprintf(buffer,
			"You drop the %s.\n\r", fname(tmp_object->name));
		    send_to_char(buffer, ch);
		    act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
		    obj_from_char(tmp_object);
		    obj_to_room(tmp_object,ch->in_room);
		} else {
			if(tmp_object->item_number==10||tmp_object->item_number==16)
			    send_to_char("You would never dream of  dropping such a precious keepsake.\n\r",ch);
			else
		   		 send_to_char(
			"You can't drop it, it must be CURSED!\n\r", ch);
		}
	    } else {
		send_to_char("You do not have that item.\n\r", ch);
	    }
	}
    } else {
	send_to_char("Drop what?\n\r", ch);
    }
}

void do_putalldot(struct char_data *ch, char *name, char *target, int cmd)
{
	struct obj_data *tmp_object=NULL;
	struct obj_data *next_object=NULL;
	char buf[MAX_STRING_LENGTH];
	bool found = FALSE;
	char mybuf[MAX_INPUT_LENGTH];

	/* If "put all.object bag", get all carried items
	 * named "object", and put each into the bag.
	 */
	for (tmp_object = ch->carrying; tmp_object;
	     tmp_object = next_object) {
	     next_object = tmp_object->next_content;
	   if (isname(name, tmp_object->name) ) {
	      sprintf(buf, "%s %s", name, target);
	      found = TRUE;
		  stuffitvar = TRUE;
	      do_put(ch, buf, cmd);
		  stuffitvar = FALSE;
	}
       }
	if (!found) {
       send_to_char("You don't have one.\n\r", ch);
     } else {/* if */
		sprintf(mybuf, "You put all items named %s in the %s.\n\r", name, target);
		send_to_char(mybuf, ch);
		act("$n puts some stuff away.", TRUE, ch, NULL, NULL, TO_ROOM);
	}
 
}

void do_put(struct char_data *ch, char *argument, int cmd)
{
    char buffer[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    struct obj_data *obj_object=NULL;
    struct obj_data *sub_object=NULL;
    struct char_data *tmp_char=NULL;
    int bits;
    char allbuf[MAX_STRING_LENGTH];

    argument_interpreter(argument, arg1, arg2);

    if (*arg1) {
      if (*arg2) {
	if(!strcmp(arg1,"all")){
	  do_stuffit(ch, arg2);
	  return;
	}
	allbuf[0] = '\0';
	if (sscanf(arg1, "all.%s", allbuf) != 0) {
	  do_putalldot(ch, allbuf, arg2, cmd);
	  return;
	}
  	obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);

  	if (obj_object) {
	  if(!can_store(ch, obj_object))
	    return;
    	  bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &sub_object);
	  if (sub_object) {
	    if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
	      if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
	      	if (obj_object == sub_object) {
		  if (!stuffitvar)
		    send_to_char("You attempt to fold it into itself, but fail.\n\r", ch);
		  return;
	      	} /* can't put obj in self */
	      	if (((sub_object->obj_flags.weight) + 
   						(obj_object->obj_flags.weight)) <
		  				(sub_object->obj_flags.value[0])) {
							if(!stuffitvar)
								send_to_char("Ok.\n\r", ch);
							if (bits==FIND_OBJ_INV) {
								obj_from_char(obj_object);
		  							/* make up for above line */
		  							IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(obj_object);
		  							obj_to_obj(obj_object, sub_object);
								} else {
		  							obj_from_char(obj_object);
		  							obj_to_obj(obj_object, sub_object);
								}
								if(!stuffitvar)
								act("$n puts $p in $P.",
									TRUE, ch, obj_object, sub_object, TO_ROOM);
	      					} else { /* not enuf room in cont */
							if(!stuffitvar)
								send_to_char("It won't fit.\n\r", ch);
							else
								act("$p won't fit.", TRUE, ch, obj_object, sub_object, TO_CHAR);
	      					}
	    				} else /* container is closed */
	      					send_to_char("It seems to be closed.\n\r", ch);
	      			} else { /* arg2 is not a container */
	    				sprintf(buffer,"The %s is not a container.\n\r",
						fname(sub_object->name));
	    				send_to_char(buffer, ch);
	      			}
	    		} else { /* don't have the container */
					if(!stuffitvar){
	      			sprintf(buffer, "You dont have the %s.\n\r", arg2);
	      			send_to_char(buffer, ch);
					}
	    		}
	  		} else { /* don't have the obj to put */
				if(!stuffitvar){
	    		sprintf(buffer, "You dont have the %s.\n\r", arg1);
	    		send_to_char(buffer, ch);
				}
	  		}
		} else { /* no arg2 */
	  		sprintf(buffer, "Put %s in what?\n\r", arg1);
	  		send_to_char(buffer, ch);
		}
	} else { /* no arg1 */
		send_to_char("Put what in what?\n\r",ch);
	}
}



void do_give(struct char_data *ch, char *argument, int cmd)
{
    char obj_name[80], vict_name[80], buf[MAX_STRING_LENGTH];
    char arg[80];
    int amount;
    struct char_data *vict=NULL;
    struct obj_data *obj=NULL;
	
    if(ch->desc)
	if(ch->desc->connected == CON_SOCIAL_ZONE) {
		send_to_char("You can't give things here!\r\n",ch);
		return;
	}

    argument=one_argument(argument,obj_name);
    if(is_number(obj_name))
    {
      if (strlen(obj_name)>7){
	send_to_char("Number field too large.\n\r", ch);
	return;
      }
      amount = atoi(obj_name);
      argument=one_argument(argument, arg);
      if (str_cmp("coins",arg) && str_cmp("coin",arg))
	{
	  send_to_char("Sorry, you can't do that ...\n\r",ch);
	  return;
	}
      if(amount<0)
	{
	  send_to_char("Sorry, you can't do that!\n\r",ch);
	  return;
	}
      if((GET_GOLD(ch)<amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < 33)))
	{
	  send_to_char("You haven't got that many coins!\n\r",ch);
	  return;
	}
      argument=one_argument(argument, vict_name);
      if(!*vict_name)
	{
	  send_to_char("To who?\n\r",ch);
	  return;
	}
      if (!(vict = get_char_room_vis(ch, vict_name)))
	{
	  send_to_char("To who?\n\r",ch);
	  return;
	}
      if(vict==ch){
	send_to_char("As you try, you drop some.\n\r",ch);
	GET_GOLD(ch)-=amount/2;
	if(GET_GOLD(ch)<0)GET_GOLD(ch)=0;
	if(GET_LEVEL(ch) > 31)
		{
		sprintf(log_buf,"%s gives %d gold coins to %s.",
			GET_NAME(ch),amount,GET_NAME(vict));
		log_hd(log_buf);
		}	
	return;
      }
      if(GET_GOLD(vict)+amount>((GET_LEVEL(vict)*GET_LEVEL(vict))*5555)){
	   act("$N simply cannot carry any more gold.",0,ch,0,vict,TO_CHAR);
	   act("$n tries to give you some gold, but you can't carry any more!",0,ch,0,vict,TO_VICT);
	   return;
      }
      send_to_char("Ok.\n\r",ch);
      sprintf(buf,"%s gives you %d gold coins.\n\r",PERS(ch,vict),amount);
      act(buf, 1, ch, 0, vict, TO_VICT);
      act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);
      if (IS_NPC(ch) || (GET_LEVEL(ch) < 33))
	GET_GOLD(ch)-=amount;
      GET_GOLD(vict)+=amount;
      return;
    }

    argument=one_argument(argument, vict_name);


    if (!*obj_name || !*vict_name)
    {
	send_to_char("Give what to who?\n\r", ch);
	return;
    }
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
    {
	send_to_char("You do not seem to have anything like that.\n\r",
	   ch);
	return;
    }
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP))
    {
	if(obj->item_number==10)
	    send_to_char("You hesitate and smack yourself, what WERE you thinking!.\n\r",ch);
	else
	    send_to_char("You can't let go of it! Yeech!!\n\r", ch);
	return;
    }
    if (!(vict = get_char_room_vis(ch, vict_name)))
    {
	send_to_char("No one by that name around here.\n\r", ch);
	return;
    }

    if ((1+IS_CARRYING_N(vict)) > CAN_CARRY_N(vict))
    {
	act("$N seems to have $S hands full.", 0, ch, 0, vict, TO_CHAR);
	act("$n tries to give you $p, but your hands are fun.",0,ch,obj,vict,TO_VICT);
	return;
    }
    if (obj->obj_flags.weight + IS_CARRYING_W(vict) > CAN_CARRY_W(vict))
    {
	act("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR);
	act("$n tries to give you $p, but you can't carry any more.",0,ch,obj,vict,TO_VICT);
	return;
    }
    if(in_a_shop(ch)&&IS_NPC(vict)){
	    obj_from_char(obj);
	    extract_obj(obj);
    }else{
        obj_from_char(obj);
        obj_to_char(obj, vict);
    }
    act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
    act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
    send_to_char("Ok.\n\r", ch);
}

bool can_store(struct char_data *ch, struct obj_data *obj){
	if(obj->item_number==10||obj->item_number==16){
		if(!stuffitvar)
		send_to_char("You remember the gods want it to be visible at all times.\n\r",ch);
		return(FALSE);
	}

	if(obj->item_number==9822){
		if(!stuffitvar)
		send_to_char("Too dangerous putting a trap into something.\n\r",ch);
		return(FALSE);
	}

	if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER) {
		if(!stuffitvar)
		send_to_char("The container doesn't fit in there.\n\r", ch);
		return(FALSE);
	}

	return(TRUE);
}

void do_stuffit(struct char_data *ch, char *container){
	struct obj_data *cur_obj;
	struct obj_data *next_obj;
	char put_line[255];
	char first_name[255];
	int counter=0;

	put_line[0]='\0';
	first_name[0]='\0';

	stuffitvar = TRUE;	
	for(cur_obj=ch->carrying;cur_obj;cur_obj=next_obj){
		next_obj=cur_obj->next_content;
		strcpy(first_name,cur_obj->name);	
		while((first_name[counter]!='\0') && (first_name[counter]!=' '))
			counter++;
		first_name[counter]='\0';	
		counter = 0;
		sprintf(put_line,"%s %s", first_name, container);
		do_put(ch, put_line, 0);
	}	
	stuffitvar = FALSE;
	act("$n packs up $s stuff.", TRUE, ch, NULL, NULL, TO_ROOM);
	act("You pack up your stuff.", TRUE, ch, NULL, NULL, TO_CHAR);
}
