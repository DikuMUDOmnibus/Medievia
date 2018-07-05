/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   *
***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifndef __FreeBSD__
#include <malloc.h>
#endif

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

/* extern variables */

extern struct str_app_type str_app[];
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct descriptor_data *descriptor_list;
extern char *drinks[];
extern int drink_aff[][3];
extern struct index_data *obj_index;

/* extern functions */

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
			 char *arg, struct obj_data **equipment, int *j);
int FOUNTAINisPresent(struct char_data *ch);

/* Internal Globals */
bool wearall;		/* This var is used to reduce spam on a wear all*/


void name_from_drinkcon(struct obj_data *obj)
{
    int i;
    char *new_name;

    for(i=0; (*((obj->name)+i)!=' ') && (*((obj->name)+i)!='\0'); i++)  ;

    if (*((obj->name)+i)==' ') {
	new_name=str_dup((obj->name)+i+1);
	obj->name = my_free(obj->name);
	obj->name=new_name;
    }
}



void name_to_drinkcon(struct obj_data *obj,int type)
{
    char *new_name;
    extern char *drinknames[];

    CREATE(new_name,char,strlen(obj->name)+strlen(drinknames[type])+2);
    sprintf(new_name,"%s %s",drinknames[type],obj->name);
    obj->name = my_free(obj->name);
    obj->name=new_name;
}



void do_drink(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    struct obj_data *temp=NULL;
    struct affected_type af;
    int amount=0;
    int weight=0;
    struct char_data *i=NULL;

    one_argument(argument,buf);
	if(!buf[0]){
		if(!IS_SET(world[ch->in_room]->room_flags,DRINKROOM)){
			send_to_char("You see no water, suitable for drinking, nearby.\n\r",ch);
			return;
		}
		if((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>20)){
			act("Your stomach can't contain any more!",FALSE,ch,0,0,TO_CHAR);
			return;
		}	
		act("You bend down and drink from the water.", TRUE, ch, 0, 0, TO_CHAR);
		act("$n bends down and drinks from the water.", TRUE, ch, 0, 0, TO_ROOM);
		act("You are not thirsty any more.",TRUE, ch, 0, 0, TO_CHAR);
		if (GET_LEVEL(ch)>31)
		  	return;
		GET_COND(ch,THIRST) = 25;
		return;
	}
    if (!str_cmp(buf, "fountain")) {
      	if (FOUNTAINisPresent(ch)) {
			if((GET_COND(ch,DRUNK)>10)&&(GET_COND(ch,THIRST)>0)){
				act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
				act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
				return;
	  		}
			if((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>20)){
				act("Your stomach can't contain any more!",FALSE,ch,0,0,TO_CHAR);
				act("You are full already!",FALSE,ch,0,0,TO_CHAR);
				act("You are not thirsty yet.",FALSE,ch,0,0,TO_CHAR);
				return;
		  	}
			act("You drink from the fountain.", TRUE, ch, 0, 0, TO_CHAR);
			act("$n drinks from the fountain.", TRUE, ch, 0, 0, TO_ROOM);
			act("You are full.", TRUE, ch, 0, 0, TO_CHAR);
			act("You are not thirsty any more.",TRUE, ch, 0, 0, TO_CHAR);
			if (GET_LEVEL(ch)>31)
		  		return;
			GET_COND(ch,FULL) = 25;
			GET_COND(ch,THIRST) = 25;
			return;
      	}else {
			act("You can't find the fountain!",FALSE,ch,0,0,TO_CHAR);
			return;
      	}
    }
    if (!str_cmp(buf, "blood")) {
      	for (i = world[ch->in_room]->people; i ; i = i->next_in_room ){
			if(IS_UNDEAD(i))
				break;
		}
      	if(i){
        	if(ch == i) {        
          		act("You slash an artery and drink the blood.", FALSE, ch, 0,0, TO_CHAR);
          		act("$n slashes an artery and drinks the blood.", FALSE, ch, 0, 0, TO_ROOM);
	  			return;
          	} 
        	if((GET_COND(ch,DRUNK)>10)){
        		act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
        		act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
        		return;
          	}    
        	if((GET_COND(ch,THIRST)>0)){
        		act("Ugh! You decide you're not thirsty enough to drink THAT!",FALSE,ch,0,0,TO_CHAR);
        		act("$n was going to drink the corpse's blood but changed $s mind.", TRUE, ch, 0, 0, TO_ROOM);
        		return;
          	}    
			sprintf(log_buf,"You rip the heart from %s's corpse and drink the blood from it.", GET_NAME(i));
        	act(log_buf, TRUE, ch, 0, 0, TO_CHAR);
			sprintf(log_buf,"$n rips the heart from %s's corpse and drinks the blood from it.", GET_NAME(i));
        	act(log_buf, TRUE, ch, 0, 0, TO_ROOM);
        	act("You are no longer thirsty.",TRUE, ch, 0, 0, TO_CHAR);
        	if ( (GET_LEVEL(ch)>31) || (IS_NPC(ch)) )
          	return;
        	GET_COND(ch,THIRST) = 25;        
        	return;
      	}
    }  
    if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))){
		act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
		return;
    }
    if (temp->obj_flags.type_flag!=ITEM_DRINKCON){
		act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR);
		return;
    }
    if((GET_COND(ch,DRUNK)>10)&&(GET_COND(ch,THIRST)>0)){
		act("You try to drink but miss your mouth!", FALSE, ch, 0, 0, TO_CHAR);
		act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
		return;
    }
    if((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>0)){
		act("Your stomach can't contain any more!",FALSE,ch,0,0,TO_CHAR);
		return;
    }
    if(temp->obj_flags.type_flag==ITEM_DRINKCON){
      	if(temp->obj_flags.value[1]>0){   
		  	sprintf(buf,"$n drinks %s from $p",drinks[temp->obj_flags.value[2]]);
	  		act(buf, TRUE, ch, temp, 0, TO_ROOM);
	  		sprintf(buf,"You drink the %s.\n\r",drinks[temp->obj_flags.value[2]]);
	  		send_to_char(buf,ch);
	  		if (GET_LEVEL(ch)>31)
				return;
		  	if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0 )
				amount = (25-GET_COND(ch,THIRST))/ drink_aff[temp->obj_flags.value[2]][DRUNK];
		  	else
				amount = number(3,10);
		  	amount = MIN(amount,temp->obj_flags.value[1]);
		  	/* You can't subtract more than the object weighs */
		  	weight = MIN(amount, temp->obj_flags.weight);
		  	gain_condition(ch,DRUNK,(int)((int)drink_aff[temp->obj_flags.value[2]][DRUNK]*amount)/4);
	  		gain_condition(ch,THIRST,(int)((int)drink_aff[temp->obj_flags.value[2]][THIRST]*amount)/4);
		  	if(GET_COND(ch,DRUNK)>10)
				act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);
	  		if(GET_COND(ch,THIRST)>20)
				act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);
		  	if(GET_COND(ch,FULL)>20)
				act("You are full.",FALSE,ch,0,0,TO_CHAR);
		  	if(temp->obj_flags.value[3] && (GET_LEVEL(ch)<31)){
		  		/* The shit was poisoned ! */
	  			act("Oops, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
		  		act("$n chokes and utters some strange sounds.",TRUE,ch,0,0,TO_ROOM);
		  		af.type = SPELL_POISON;
	  			af.duration = amount*3;
	  			af.modifier = 0;
		  		af.location = APPLY_NONE;
		  		af.bitvector = AFF_POISON;
	  			affect_join(ch,&af, TRUE, TRUE);
			}
		  	/* empty the container, and no longer poison. */
	  		temp->obj_flags.value[1]-= amount;
		  	if(!temp->obj_flags.value[1]) {  /* The last bit */
				temp->obj_flags.value[2]=0;
				temp->obj_flags.value[3]=0;
				name_from_drinkcon(temp);
		  	}
		  	if(temp->obj_flags.value[1]<=0) { /*empty now then poof*/
				act("It is now empty, and it magically disappears in a puff of smoke!",FALSE,ch,0,0,TO_CHAR);
				extract_obj(temp);
		  	}
		 	 return;
		}
    }
    act("It's empty already.",FALSE,ch,0,0,TO_CHAR);
    return;
}



void do_eat(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    struct obj_data *temp=NULL;
    struct affected_type af;
    struct char_data *i=NULL;

    one_argument(argument,buf);

    if (!str_cmp(buf, "flesh")) {
      for (i = world[ch->in_room]->people; i ; i = i->next_in_room )
        {
        if (IS_UNDEAD(i))
                break;
        }
 
      if (i) {
	if(ch == i)
	  {
	  act("You rip off a hunk of your own flesh and eat it.", FALSE, ch, 0, 0, TO_CHAR);
	  act("$n rips off a hunk of $s own flesh and eats it.", FALSE, ch, 0, 0, TO_ROOM);
	  return;
	  }
        if((GET_COND(ch,DRUNK)>10))
          {
        act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
        act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
        return;
          }
 
        if((GET_COND(ch,FULL)>0)) /* Stomach full */
          {
        act("Ugh! You decide you're not hungry enough to eat THAT!",FALSE,ch,0,NULL,TO_CHAR);
        act("$n was going to eat the corpse but changed $s mind.", TRUE, ch, 0,
NULL, TO_ROOM);
        return;
          }
        sprintf(log_buf,"You rip off a hunk of flesh from %s's corpse and savagely devour it.", GET_NAME(i));
        act(log_buf, TRUE, ch, 0, NULL, TO_CHAR);
        sprintf(log_buf,"$n rips off a hunk of flesh from %s's corpse and savagely devours it.", GET_NAME(i));
        act(log_buf, TRUE, ch, 0, NULL, TO_ROOM);
        act("You are full.", TRUE, ch, 0, 0, TO_CHAR);
        if ( (GET_LEVEL(ch)>31) || IS_NPC(ch) )
          return;
        
        GET_COND(ch,FULL) = 25;
        return;
      }
    }   
      
    if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if((temp->obj_flags.type_flag != ITEM_FOOD) && (GET_LEVEL(ch) < 33))
    {
	act("Your stomach refuses to eat that!?!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

	if ((temp->obj_flags.eq_level > 23) && (GET_LEVEL(ch) < 35))
		{
		sprintf(log_buf,"%s eats %s at [%d]%s.",GET_NAME(ch), 
            temp->short_description,ch->in_room, world[ch->in_room]->name); 
		log_hd(log_buf);
		}

    if(GET_COND(ch,FULL)>20) /* Stomach full */
    {   
	act("You are too full to eat more!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    act("$n eats $p.",TRUE,ch,temp,0,TO_ROOM);
    act("You eat the $o.",FALSE,ch,temp,0,TO_CHAR);

    gain_condition(ch,FULL,temp->obj_flags.value[0]);

    if(GET_COND(ch,FULL)>20)
	act("You are full.",FALSE,ch,0,0,TO_CHAR);


    if(temp->item_number==215){
	act("That tasted rather strange!!",FALSE,ch,0,0,TO_CHAR);
	act("$n coughs and utters some strange sounds.",FALSE,ch,0,0,TO_ROOM);
	af.type = SPELL_REP_ROOT;
	af.duration = 24;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_REP_ROOT;
	affect_join(ch,&af, TRUE, TRUE);
    }
    if(temp->obj_flags.value[3] && (GET_LEVEL(ch) < 31))
    {
	/* The shit was poisoned ! */
	act("Oops, that tasted rather strange!!",FALSE,ch,0,0,TO_CHAR);
	act("$n coughs and utters some strange sounds.",FALSE,ch,0,0,TO_ROOM);

	af.type = SPELL_POISON;
	af.duration = temp->obj_flags.value[0]*2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_join(ch,&af, TRUE, TRUE);
    }

    extract_obj(temp);
}


void do_pour(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *from_obj=NULL;
    struct obj_data *to_obj=NULL;
    int amount=0;

    argument_interpreter(argument, arg1, arg2);

    if(!*arg1) /* No arguments */
    {
		act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR);
		return;
    }
    if(!(from_obj = get_obj_in_list_vis(ch,arg1,ch->carrying)))
    {
		act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
		return;
    }
    if(from_obj->obj_flags.type_flag!=ITEM_DRINKCON)
    {
	act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if(from_obj->obj_flags.value[1]==0)
    {
	act("The $p is empty.",FALSE, ch,from_obj, 0,TO_CHAR);
	return;
    }

    if(!*arg2)
    {
	act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if(!str_cmp(arg2,"out"))
    {
	act("$n empties $p", TRUE, ch,from_obj,0,TO_ROOM);
	act("You empty the $p.", FALSE, ch,from_obj,0,TO_CHAR);


	from_obj->obj_flags.value[1]=0;
	from_obj->obj_flags.value[2]=0;
	from_obj->obj_flags.value[3]=0;
	name_from_drinkcon(from_obj);
	
	return;

    }

    if(!(to_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if(to_obj->obj_flags.type_flag!=ITEM_DRINKCON)
    {
	act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if (to_obj == from_obj)
      {
	act("A most unproductive effort.",FALSE,ch,0,0,TO_CHAR);
	return;
      }
    
    if((to_obj->obj_flags.value[1]!=0)&&
	(to_obj->obj_flags.value[2]!=from_obj->obj_flags.value[2]))
    {
	act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if(!(to_obj->obj_flags.value[1]<to_obj->obj_flags.value[0]))
    {
	act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    sprintf(buf,"You pour the %s into the %s.",
	drinks[from_obj->obj_flags.value[2]],arg2);
    send_to_char(buf,ch);

    /* New alias */
    if (to_obj->obj_flags.value[1]==0) 
	name_to_drinkcon(to_obj,from_obj->obj_flags.value[2]);

    /* First same type liq. */
    to_obj->obj_flags.value[2]=from_obj->obj_flags.value[2];

    /* Then how much to pour */
    from_obj->obj_flags.value[1]-= (amount=
	(to_obj->obj_flags.value[0]-to_obj->obj_flags.value[1]));

    to_obj->obj_flags.value[1]=to_obj->obj_flags.value[0];

    if(from_obj->obj_flags.value[1]<0)    /* There was to little */
    {
	to_obj->obj_flags.value[1]+=from_obj->obj_flags.value[1];
	amount += from_obj->obj_flags.value[1];
	from_obj->obj_flags.value[1]=0;
	from_obj->obj_flags.value[2]=0;
	from_obj->obj_flags.value[3]=0;
	name_from_drinkcon(from_obj);
    }

    /* Then the poison boogie */
    to_obj->obj_flags.value[3]=
	(to_obj->obj_flags.value[3]||from_obj->obj_flags.value[3]);


    return;
}

void do_sip(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *temp=NULL;

    one_argument(argument,arg);

    if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if(temp->obj_flags.type_flag!=ITEM_DRINKCON)
    {
	act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if(GET_COND(ch,DRUNK)>10) /* The pig is drunk ! */
    {
	act("You simply fail to reach your mouth!",FALSE,ch,0,0,TO_CHAR);
	act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM);
	return;
    }

    if(!temp->obj_flags.value[1])  /* Empty */
    {
	act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    act("$n sips from the $o.",TRUE,ch,temp,0,TO_ROOM);
    sprintf(buf,"It tastes like %s.\n\r",drinks[temp->obj_flags.value[2]]);
    send_to_char(buf,ch);

    gain_condition(ch,DRUNK,
	(int)(drink_aff[temp->obj_flags.value[2]][DRUNK]/4));

    gain_condition(ch,FULL,
	(int)(drink_aff[temp->obj_flags.value[2]][FULL]/4));

    gain_condition(ch,THIRST,
   	 (int)(drink_aff[temp->obj_flags.value[2]][THIRST]/4));


    if(GET_COND(ch,DRUNK)>10)
	act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

    if(GET_COND(ch,THIRST)>20)
	act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

    if(GET_COND(ch,FULL)>20)
	act("You are full.",FALSE,ch,0,0,TO_CHAR);

    if(temp->obj_flags.value[3] && !IS_AFFECTED(ch,AFF_POISON)
    && GET_LEVEL(ch) <31) /* The shit was poisoned ! */
    {
	act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR);

	af.type = SPELL_POISON;
	af.duration = 3;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_to_char(ch,&af);
    }

    temp->obj_flags.value[1]--;

    if(!temp->obj_flags.value[1])  /* The last bit */
    {
	temp->obj_flags.value[2]=0;
	temp->obj_flags.value[3]=0;
	name_from_drinkcon(temp);
    }

    return;

}


void do_taste(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    char arg[MAX_STRING_LENGTH];
    struct obj_data *temp=NULL;

    one_argument(argument,arg);

    if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
	act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    if(temp->obj_flags.type_flag==ITEM_DRINKCON)
    {
	do_sip(ch,argument,0);
	return;
    }

    if(!(temp->obj_flags.type_flag==ITEM_FOOD))
    {
	act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR);
	return;
    }

    act("$n tastes the $o.", FALSE, ch, temp, 0, TO_ROOM);
    act("You taste the $o.", FALSE, ch, temp, 0, TO_CHAR);

    gain_condition(ch,FULL,1);

    if(GET_COND(ch,FULL)>20)
	act("You are full.",FALSE,ch,0,0,TO_CHAR);

    if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)
    && GET_LEVEL(ch) <31) /* The shit was poisoned ! */
    {
	act("Oops, that didn't taste good at all!",FALSE,ch,0,0,TO_CHAR);

	af.type = SPELL_POISON;
	af.duration = 2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_to_char(ch,&af);
    }

    temp->obj_flags.value[0]--;

    if(!temp->obj_flags.value[0])   /* Nothing left */
    {
	act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR);
	extract_obj(temp);
    }

    return;

}



/* functions related to wear */

void perform_wear(struct char_data *ch, struct obj_data *obj_object,
	int keyword)
{

    switch(keyword) {
	case 0 :
		if(!wearall){
	    act("$n lights $p and holds it.", FALSE, ch, obj_object,0,TO_ROOM);
	    act("You light $p and start holding it.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 1 : 
		if(!wearall)
	    act("$n wears $p on $s finger.", TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case 2 : 
		if(!wearall){
	    act("$n wears $p around $s neck.", TRUE, ch, obj_object,0,TO_ROOM);
	    act("You put $p around your neck.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 3 : 
		if(!wearall){
	    act("$n wears $p on $s body.", TRUE, ch, obj_object,0,TO_ROOM);
	    act("You put $p around your body.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 4 : 
		if(!wearall){
	    act("$n wears $p on $s head.",TRUE, ch, obj_object,0,TO_ROOM);
	    act("You put $p on your head.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 5 : 
		if(!wearall){
	    act("$n wears $p on $s legs.",TRUE, ch, obj_object,0,TO_ROOM);
	    act("You put $p on your legs.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 6 : 
		if(!wearall){
	    act("$n wears $p on $s feet.",TRUE, ch, obj_object,0,TO_ROOM);
	    act("You put $p on your feet.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 7 : 
		if(!wearall){
	    act("$n wears $p on $s hands.",TRUE, ch, obj_object,0,TO_ROOM);
	    act("You put $p on your hands.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 8 : 
		if(!wearall){
	    act("$n wears $p on $s arms.",TRUE, ch, obj_object,0,TO_ROOM);
	    act("You put $p on your arms.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 9 : 
		if(!wearall){
	    act("$n wears $p about $s body.",TRUE, ch, obj_object,0,TO_ROOM);
	    act("You put $p about your body.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 10 : 
		if(!wearall){
	    act("$n wears $p about $s waist.",TRUE, ch, obj_object,0,TO_ROOM);
	    act("You put $p about your waist.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 11 : 
		if(!wearall)
	    act("$n wears $p around $s wrist.",TRUE, ch, obj_object,0,TO_ROOM);
	    break;
	case 12 : 
		if(!wearall){
	    act("$n wields $p.",TRUE, ch, obj_object,0,TO_ROOM);
	    act("You wield $p.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 13 : 
		if(!wearall){
	    act("$n grabs $p.",TRUE, ch, obj_object,0,TO_ROOM);
	    act("You grab $p.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
	case 14 : 
		if(!wearall){
	    act("$n starts using $p as shield.",
		TRUE, ch, obj_object,0,TO_ROOM);
	    act("You start using $p as a shield.", TRUE, ch, obj_object,0,TO_CHAR);
		}
	    break;
    }
}



void wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
    char buffer[MAX_STRING_LENGTH];
	bool mywieldvar=TRUE;
	char abuf[MAX_INPUT_LENGTH];

    if ( GET_LEVEL(ch) < obj_object->obj_flags.eq_level )
    {
	if(!wearall){
	sprintf( buffer, "You must be level %d to use this object.\n\r",
	    obj_object->obj_flags.eq_level );
	send_to_char( buffer, ch );
	}
	return;
    }

    switch(keyword) {
	case 0: {  /* LIGHT SOURCE */
	    if (ch->equipment[WEAR_LIGHT])
			{
			if(!wearall)
			send_to_char("You are already holding a light source.\n\r", ch);
			}
		else {
		perform_wear(ch,obj_object,keyword);
		obj_from_char(obj_object);
		equip_char(ch,obj_object, WEAR_LIGHT);
		if (obj_object->obj_flags.value[2])
		    world[ch->in_room]->light++;
	    }
	} break;

	case 1: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) {
		if ((ch->equipment[WEAR_FINGER_L])
		&& (ch->equipment[WEAR_FINGER_R])) {
			if(!wearall)
		    send_to_char(
		"You are already wearing something on your fingers.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    if (ch->equipment[WEAR_FINGER_L]) {
			sprintf(buffer,
			    "You put the %s on your right ring finger.\n\r", 
			    fname(obj_object->name));
			if(!wearall)
			send_to_char(buffer, ch);
			obj_from_char(obj_object);
			equip_char(ch, obj_object, WEAR_FINGER_R);
		    } else {
			sprintf(buffer,
			    "You put the %s on your left ring finger.\n\r", 
			    fname(obj_object->name));
			if(!wearall)
			send_to_char(buffer, ch);
			obj_from_char(obj_object);
			equip_char(ch, obj_object, WEAR_FINGER_L);
		    }
		}
	    } else {
		send_to_char("You can't wear that on your fingers.\n\r", ch);
	    }
	} break;
	case 2: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) {
		if ((ch->equipment[WEAR_NECK_1])
		&& (ch->equipment[WEAR_NECK_2])) {
			if(!wearall)
		    send_to_char(
		    "You can't wear any more around your neck.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    if (ch->equipment[WEAR_NECK_1]) {
			obj_from_char(obj_object);
			equip_char(ch, obj_object, WEAR_NECK_2);
		    } else {
			obj_from_char(obj_object);
			equip_char(ch, obj_object, WEAR_NECK_1);
		    }
		}
	    } else {
		send_to_char("You can't wear that around your neck.\n\r", ch);
	    }
	} break;
	case 3: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) {
		if ((ch->equipment[WEAR_BODY])) {
			if(!wearall)
		    send_to_char(
			"You already wear something on your body.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch,  obj_object, WEAR_BODY);
		}
	    } else {
		send_to_char("You can't wear that on your body.\n\r", ch);
	    }
	} break;
	case 4: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) {
		if ((ch->equipment[WEAR_HEAD])) {
			if(!wearall)
		    send_to_char(
			"You already wear something on your head.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_HEAD);
		}
	    } else {
		send_to_char("You can't wear that on your head.\n\r", ch);
	    }
	} break;
	case 5: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) {
		if ((ch->equipment[WEAR_LEGS])) {
			if(!wearall)
		    send_to_char(
			"You already wear something on your legs.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_LEGS);
		}
	    } else {
		send_to_char("You can't wear that on your legs.\n\r", ch);
	    }
	} break;
	case 6: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) {
		if ((ch->equipment[WEAR_FEET])) {
			if(!wearall)
		    send_to_char(
		    "You already wear something on your feet.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_FEET);
		}
	    } else {
		send_to_char("You can't wear that on your feet.\n\r", ch);
	    }
	} break;
	case 7: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) {
		if ((ch->equipment[WEAR_HANDS])) {
			if(!wearall)
		    send_to_char(
		    "You already wear something on your hands.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_HANDS);
		}
	    } else {
		send_to_char("You can't wear that on your hands.\n\r", ch);
	    }
	} break;
	case 8: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) {
		if ((ch->equipment[WEAR_ARMS])) {
			if(!wearall)
		    send_to_char(
		    "You already wear something on your arms.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_ARMS);
		}
	    } else {
		send_to_char("You can't wear that on your arms.\n\r", ch);
	    }
	} break;
	case 9: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) {
		if ((ch->equipment[WEAR_ABOUT])) {
			if(!wearall)
		    send_to_char(
		    "You already wear something about your body.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_ABOUT);
		}
	    } else {
		send_to_char("You can't wear that about your body.\n\r", ch);
	    }
	} break;
	case 10: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) {
		if ((ch->equipment[WEAR_WAISTE])) {
			if(!wearall)
		    send_to_char(
		    "You already wear something about your waist.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch,  obj_object, WEAR_WAISTE);
		}
	    } else {
		send_to_char("You can't wear that about your waist.\n\r", ch);
	    }
	} break;
	case 11: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) {
		if ((ch->equipment[WEAR_WRIST_L])
		&& (ch->equipment[WEAR_WRIST_R])) {
			if(!wearall)
		    send_to_char(
		"You already wear something around both your wrists.\n\r", ch);
		} else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    if (ch->equipment[WEAR_WRIST_L]) {
			sprintf(buffer,
			"You wear the %s around your right wrist.\n\r", 
			    fname(obj_object->name));
			if(!wearall)
			send_to_char(buffer, ch);
			equip_char(ch,  obj_object, WEAR_WRIST_R);
		    } else {
			sprintf(buffer,
			"You wear the %s around your left wrist.\n\r", 
			    fname(obj_object->name));
			if(!wearall)
			send_to_char(buffer, ch);
			equip_char(ch, obj_object, WEAR_WRIST_L);
		    }
/* juice
 * make it so you actually have to be wearing the bracelet before you can
 * benefit from it.
 */
	            if(obj_object->item_number==10005){
		       GET_COND(ch,FULL)=-1;
		       GET_COND(ch,THIRST)=-1;
	            }
		}
	    } else {
		send_to_char("You can't wear that around your wrist.\n\r", ch);
	    }
	} break;

	case 12:
	    if (CAN_WEAR(obj_object,ITEM_WIELD)
	    || IS_SET(obj_object->obj_flags.wear_flags,ITEM_THROW)) {
		if ((ch->equipment[WIELD])) {
			if(!wearall)
		    send_to_char(
		    "You are already wielding something.\n\r", ch);
		} else {
		    if ((GET_OBJ_WEIGHT(obj_object) >
			str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
			&& (!wearall)) {
			send_to_char("It is too heavy for you to use.\n\r",ch);
		    } else {
				if(GET_LEVEL(ch)<35)
        		switch(GET_CLASS(ch)){
            		case CLASS_MAGIC_USER:

                if(IS_SET(obj_object->obj_flags.value[0], NOT_MAGE)){
					if(!wearall)
					strcpy(abuf,"A Mage cannot wield this weapon.\n\r");
					mywieldvar=FALSE;
				}
               	break;

            		case CLASS_CLERIC:

				if(IS_SET(obj_object->obj_flags.value[0], NOT_CLERIC)){
					if(!wearall)
                	strcpy(abuf,"A cleric cannot wield this weapon.\n\r");
                	mywieldvar=FALSE;
					}
               	break;

            		case CLASS_THIEF:

                if(IS_SET(obj_object->obj_flags.value[0], NOT_THIEF)){
					if(!wearall)
                	strcpy(abuf,"A thief cannot wield this weapon.\n\r");
                	mywieldvar=FALSE;
				}
               	break;

            		case CLASS_WARRIOR:

                if(IS_SET(obj_object->obj_flags.value[0], NOT_WARRIOR)){
					if(!wearall)
					strcpy(abuf,"A warrior cannot wield this weapon.\n\r");
   					mywieldvar=FALSE;
				}
           		break;

					default:
						break;
        		}
				if((obj_object->obj_flags.value[0] & TWO_HANDED)==TWO_HANDED){
					if(ch->equipment[WEAR_SHIELD]){
						if(!wearall)
						strcpy(abuf,"You cannot wield a two-handed weapon while holding a shield.\n\r");	
						mywieldvar=FALSE;
					}
				}

				/* Allow players who used to be thief to wield a 
					back stab enabling weapon. */

				if(IS_SET(obj_object->obj_flags.value[0], DAGGER))
				    if(IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF))
						mywieldvar=TRUE;

				if(mywieldvar){
					perform_wear(ch,obj_object,keyword);
					obj_from_char(obj_object);
					equip_char(ch, obj_object, WIELD);
				}else
					send_to_char(abuf, ch);	
		    }
		}
	    } else {
		send_to_char("You can't wield that.\n\r", ch);
	    }
	    break;

	case 13:
	    if (CAN_WEAR(obj_object,ITEM_HOLD)) {
        if ((ch->equipment[HOLD])) {
			if(!wearall)
		    send_to_char("You are already holding something.\n\r", ch);
		}
		else {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, HOLD);
			}
	    } else {
		send_to_char("You can't hold this.\n\r", ch);
	    }
	    break;
	case 14: {
	    if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) {
		if ((ch->equipment[WEAR_SHIELD])) {
			if(!wearall)
		    send_to_char(
			"You are already using a shield\n\r", ch);
			mywieldvar=FALSE;
		} 
		if(ch->equipment[WIELD])
		if((ch->equipment[WIELD]->obj_flags.value[0] & TWO_HANDED)==TWO_HANDED){
			if(!wearall)
			send_to_char("You cannot wear a shield while wielding a two-handed weapon.\n\r", ch);
			mywieldvar=FALSE;
		}
		if(mywieldvar){
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_SHIELD);
		}
	    } else {
		send_to_char("You can't use that as a shield.\n\r", ch);
	    }
	} break;
	case -1: {
	    sprintf(buffer,"Wear %s where?.\n\r", fname(obj_object->name));
	    send_to_char(buffer, ch);
	} break;
	case -2: {
	    sprintf(buffer,"You can't wear the %s.\n\r",
	   	 fname(obj_object->name));
	    send_to_char(buffer, ch);
	} break;
	default: {
	    log_hd("Unknown type called in wear.");
	} break;
    }
}

int keywordfind(struct obj_data *obj_object)
{
    int keyword;

    keyword = -2;
    if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) keyword = 14;
    if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) keyword = 1;
    if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) keyword = 2;
    if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) keyword = 11;
    if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) keyword = 10;
    if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) keyword = 8;
    if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) keyword = 7;
    if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) keyword = 6;
    if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) keyword = 5;
    if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) keyword = 9;
    if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) keyword = 4;
    if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) keyword = 3;

    return keyword;
}

void do_wear(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *obj_object=NULL, *tmp_object=NULL, *next_obj=NULL;
    int keyword=0;
    static char *keywords[] = {
	"finger",
	"neck",
	"body",
	"head",
	"legs",
	"feet",
	"hands",
	"arms",
	"about",
	"waist",
	"wrist",
	"shield",
	"\n"
    };

    argument_interpreter(argument, arg1, arg2);

    if ( *arg1 == MED_NULL )
    {
	send_to_char( "Wear what?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg1, "all" ) ) {
	for ( tmp_object = ch->carrying; tmp_object; tmp_object = next_obj ) {
	    int	keyword;
		wearall = TRUE;
	    next_obj = tmp_object->next_content;
		if(tmp_object->item_number==2)
			continue;
	    if ( !CAN_SEE_OBJ( ch, tmp_object ) )
		continue;

	    keyword = keywordfind( tmp_object );
	    if ( keyword != -2 )
		;
	    else if ( CAN_WEAR(tmp_object, ITEM_WIELD) )
		keyword = 12;
	    else if ( tmp_object->obj_flags.type_flag == ITEM_LIGHT )
		keyword = WEAR_LIGHT;
	    else if ( CAN_WEAR(tmp_object, ITEM_HOLD) )
		keyword = 13;
	    else
		continue;
/* juice
 * This is spot where player's Full and Thirst were modified with
 * bracelet of life.  Moved it so this doesn't get modified unless
 * player is actually wearing it.
 */
	    wear( ch, tmp_object, keyword );
	}
    act("$n prepares $mself for battle.", FALSE, ch, NULL,0,TO_ROOM);
    act("You throw on your gear.", TRUE, ch, NULL,0,TO_CHAR);
	wearall=FALSE;
	return;
    }

    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (obj_object)
    {
	if (*arg2)
	{
	    keyword = search_block(arg2, keywords, FALSE);
	    if (keyword == -1)
	    {
		sprintf(buf,
		"%s is an unknown body location.\n\r", arg2);
		send_to_char(buf, ch);
	    } 
	    else
/* juice used to set bracelet of life here */
	    {
		wear(ch, obj_object, keyword+1);
	    }
	}
	else
/* juice used to set bracelet of life here */
	{
		wear(ch, obj_object, keywordfind(obj_object));
	}
    }
    else
    {
	sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
	send_to_char(buffer,ch);
    }

    if(GET_MANA(ch)>GET_MAX_MANA(ch))
		GET_MANA(ch)=GET_MAX_MANA(ch);
}



void do_wield(struct char_data *ch, char *argument, int cmd) {
char arg1[MAX_STRING_LENGTH];
char arg2[MAX_STRING_LENGTH];
char buffer[MAX_STRING_LENGTH];
struct obj_data *obj_object=NULL;
int keyword = 12;

    argument_interpreter(argument, arg1, arg2);
    if (*arg1) {
	obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
	if (obj_object) {
	    wear(ch, obj_object, keyword);
	} else {
		if(!wearall)
	    sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
	    send_to_char(buffer,ch);
	}
    } else {
	send_to_char("Wield what?\n\r", ch);
    }
}


void do_grab(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *obj_object=NULL;

    argument_interpreter(argument, arg1, arg2);

    if (*arg1) {
	obj_object = get_obj_in_list(arg1, ch->carrying);
	if (obj_object) {
	    if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
		wear(ch, obj_object, WEAR_LIGHT);
	    else
		wear(ch, obj_object, 13);
	} else {
	    sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
	    send_to_char(buffer,ch);
	}
    } else {
	send_to_char("Hold what?\n\r", ch);
    }
}

void do_remove(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    struct obj_data *obj_object=NULL, *weapon=NULL;
    int j;

    one_argument(argument, arg1);

    if (*arg1){
    	if(!strcmp(arg1,"all")){
	    for(j=0;j<MAX_WEAR;j++){
      		obj_object = ch->equipment[j];
      		if (obj_object) {
			if ((obj_object->obj_flags.type_flag == ITEM_REGEN) &&
			    (obj_object->obj_flags.value[2] < obj_object->obj_flags.value[1])) {
		   		act("A strong force prevents you from removing $p.",FALSE,ch,obj_object,0,TO_CHAR);
				continue;
			}
         		if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
	    			if(obj_object->item_number==10005){
					GET_COND(ch,FULL)=2;
					GET_COND(ch,THIRST)=2;
	    	    		}
	      			obj_to_char(unequip_char(ch, j), ch);
	      			if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
	          			if (obj_object->obj_flags.value[2])
		      				world[ch->in_room]->light--;
          		} else 
	      		    send_to_char("You're carrying to much.\n\r", ch);
      		}
	    }
	    act("You strip to your birthday suit.",FALSE,ch,NULL,0,TO_CHAR);
	    act("$n strips to $s skivvies.",TRUE,ch,NULL,0,TO_ROOM);
	}else{
		obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
	   	if (obj_object) {
			if ((obj_object->obj_flags.type_flag == ITEM_REGEN) &&
			    (obj_object->obj_flags.value[2] < obj_object->obj_flags.value[1])) {
		   		act("A strong force prevents you from removing $p.",FALSE,ch,obj_object,0,TO_CHAR);
				return;
			}
	       		if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
				obj_to_char(unequip_char(ch, j), ch);
				if(obj_object->item_number==10005){
					GET_COND(ch,FULL)=2;
					GET_COND(ch,THIRST)=2;
	    	    		}
		   		if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
		 			if (obj_object->obj_flags.value[2])
			   			world[ch->in_room]->light--;
		   		act("You stop using $p.",FALSE,ch,obj_object,0,TO_CHAR);
		   		act("$n stops using $p.",TRUE,ch,obj_object,0,TO_ROOM);
	       		} else {
		   		send_to_char("You can't carry that many items.\n\r", ch);
	       		}
	   	} else {
	       		send_to_char("You are not using it.\n\r", ch);
	   	}
	}
    } else {
	send_to_char("Remove what?\n\r", ch);
    }

    if (ch->equipment[WIELD] && 
	(GET_OBJ_WEIGHT(ch->equipment[WIELD]) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w))
    {
		weapon = unequip_char(ch, WIELD);
		obj_to_char(weapon, ch);
		act("You feel sapped of strength and drop $p.",
			FALSE, ch, weapon, 0, TO_CHAR);
		act("$n is too weak to hold $p and drops it.",
			FALSE, ch, weapon, 0, TO_ROOM);
		}

    if(GET_MANA(ch)>GET_MAX_MANA(ch))
	GET_MANA(ch)=GET_MAX_MANA(ch);
}
