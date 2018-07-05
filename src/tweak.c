/* Note: Some stuff like, which item value was the dice and acap was 
taken
   from cicrcle but it should convert easily if at all nessicary */
 
/* Weight Constants, used medievia.doc as reference */ 
#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "tweak.h"
#include "db.h"

/* global version number */

int VERSIONNUMBER=3;

extern int number(int, int);
extern void send_to_char(char *, struct char_data *);
extern char *apply_types[];
extern char *one_argument(char *, char *);
extern struct obj_data *read_object(int , int);
extern int is_number(char *);
void extract_obj(struct obj_data *obj);
extern struct obj_data  *objs[MAX_OBJ];

/* Returns the weight of an object as an int,  for  now it goes through the
   affects like a case statement, but makeing table of like 
	affectnum, weight would cut down on the code.....
   Put it in a table anyway for speed. */

int weigh(struct obj_data *obj)
{
int points=0, i=0, temp=0;

    if(!obj) return(0);

    for (i=0;i<MAX_OBJ_AFFECT;i++) {
         points += weights[(int) obj->affected[i].location][1] * obj->affected[i].modifier;
    }
    /* add obj value to weight */
    points += obj->obj_flags.cost * weights[APPLY_GOLD][1];

    switch(obj->obj_flags.type_flag) {
	case ITEM_ARMOR:
	    points += obj->obj_flags.value[0] * weights[25][1];
	    break;
	case ITEM_WEAPON:
 	    /* Better way to calculate weapon weight, slower but more acc */
	    temp = (obj->obj_flags.value[1] * obj->obj_flags.value[2]) + (obj->obj_flags.value[1] * 1);
	    points += (temp / 2) * weights[29][1];
	    break;
	default:	
	    break;
    }
    /*
    if(obj->obj_flags.weight > 10)
	points += weights[28][1] * (obj->obj_flags.weight - 10);
    if(points > W_GOOD_ITEM && obj->obj_flags.weight < 10)
	points += abs(weights[28][1]) * obj->obj_flags.weight;
    */

    return(points);
} 	  
  

/* Explanation of the limits code:
	in the weights struct the last two fields are deticated to limits.
	the third field is the ABSOLUTE lowest possible value that affect 
	could have and the 4rth value is the higest..
	if(make stat ++) {
		if(value < limits)
			value ++
       } 
       else if(value > lowlimit)
		value++	
       break
*/

struct obj_data *tweak(struct obj_data *obj)
{
    int points, w, iLev, iTemp, iYikes = 0;
    int chance=2;    
    struct obj_data *stpOrig=NULL;

    if(!obj) return(NULL);

    stpOrig = obj;
  
    /* check if item should be tweked, inclusion has less checks than * 
     * exclusion                                                      */

    /* have version # applied to ALL objects, even if it dosn't get 
    tweaked so stuff in inventory won't have to go through the tweak() 
    everytime */

    obj->iWeightVersion = VERSIONNUMBER;
    obj->iValueWeight = 0;
  
    
    if( obj->obj_flags.type_flag != ITEM_LIGHT &&
        obj->obj_flags.type_flag != ITEM_WEAPON &&
        obj->obj_flags.type_flag != ITEM_OTHER &&
        obj->obj_flags.type_flag != ITEM_ARMOR &&
        obj->obj_flags.type_flag != ITEM_WORN &&
        obj->obj_flags.type_flag != ITEM_CONTAINER &&
        obj->obj_flags.type_flag != ITEM_BOAT &&
        obj->obj_flags.type_flag != ITEM_TREASURE ) 
    	   return(obj);
    
    
    /* bars */
    if( (obj->item_number == 22) || (obj->item_number >= 1310 && obj->item_number <= 1313) ) 
	return(obj);

    /* now start the weighing */   
    w = weigh(obj);

   /* Get Orig Object */
   if(objs[obj->item_number])
	stpOrig = objs[obj->item_number];
   else 
	return(obj);


   points =  (  ( (w / 100) * number(5,37) ) * (faLevconv[(int) obj->obj_flags.eq_level][1] ) );
 
    if(points <= 0)
	return(obj);

    if(number(1,40) == 39) {
        points *= 2.5;
	   if(number(1,30) == 29)
		points *= 2.5;
    }


/*     	
    chance = number(1,weights[32][1]-1);
*/   
    /* 10% MAX flux on levres */ 
    iLev = obj->obj_flags.eq_level;
   
  
 /* Begin loop to go through and allocate points, if the area isn't 
  found, points is --'d and the loop goes on.. This is kinda stupid but
  i can't think of another way to randomly allocate stuff without lots of 
  extra code. This is really a preliminary throw together though.. There 
  is also some limits code so items don't get negative stats and such. But 
  i'm sure more is needed. Maybe, put a limit on that stat field into the 
  weights struct.*/ 

 while(points > 100) {

    iYikes++;

    if(iYikes > 800) 
	break;
    
    chance = number(1,weights[32][1]-1);

    switch(number(1,20)) {
 	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	  if(obj->affected[0].location == APPLY_NONE) {
		points--;
		break;
          }
	  if(abs(weights[(int) obj->affected[0].location][1]) > points) {
	     points--;
	     break;
  	   }
	   points -= abs(weights[(int) obj->affected[0].location][1]);
	   /* chance system for future possible expansion */
	   if(number(1,weights[32][1]) <= chance) {
	     /* Limits overflow protection code / restrictions */
             if(obj->affected[0].modifier < (weights[(int) obj->affected[0].location][4]+stpOrig->affected[0].modifier)) 
				if(obj->affected[0].modifier < weights[(int) obj->affected[0].location][3])
					obj->affected[0].modifier++;
	     } else if(obj->affected[0].modifier > (stpOrig->affected[0].modifier-weights[(int) obj->affected[0].location][4])) 
			if(obj->affected[0].modifier > weights[(int) obj->affected[0].location][2])
				obj->affected[0].modifier--;
	   break;
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	 if(obj->affected[1].location == APPLY_NONE) {
                points--;
                break;
          }

          if(abs(weights[(int) obj->affected[1].location][1]) > points) {
             points--;
             break;
           }
           points -= abs(weights[(int) obj->affected[1].location][1]);
           if(number(1,weights[32][1]) <= chance) {
             /* Limits overflow protection code */
             if(obj->affected[1].modifier < (weights[(int) obj->affected[1].location][4]+stpOrig->affected[1].modifier))
     			if(obj->affected[1].modifier < weights[(int) obj->affected[1].location][3])                       
		 			obj->affected[1].modifier++;
           } else if(obj->affected[1].modifier > (stpOrig->affected[1].modifier-weights[(int) obj->affected[1].location][4]))
           		if(obj->affected[1].modifier > weights[(int) obj->affected[1].location][2]) 
                	obj->affected[1].modifier--;
	   break;
	case 11:
	 /*
	   if(obj->obj_flags.type_flag == ITEM_ARMOR) {
		if(points < weights[25][1]) {
		   points--;
		   break;
                }
	        points -= weights[25][1];
	        if(number(1,weights[32][1]) <= chance) {
                   if(obj->obj_flags.value[0] < weights[25][3]) 
		     obj->obj_flags.value[0]++;
		} else   
		   if(obj->obj_flags.value[0] > weights[25][2]) 
		      obj->obj_flags.value[0]--;		 
		break;
           } else {
	       points--;
	       break;
	   }
	 */
	 break;
	case 12:
        case 13:
           if(obj->obj_flags.type_flag == ITEM_WEAPON) {
                if(points < weights[26][1]) {
		   points--;
                   break;
		}
                points -= weights[26][1];
                if(number(1,weights[32][1]) <= chance) {
		    if(obj->obj_flags.value[2] < (weights[26][4]+stpOrig->obj_flags.value[2]))  
                       obj->obj_flags.value[2]++;
                 } 
		 else if(obj->obj_flags.value[2] > (stpOrig->obj_flags.value[2]-weights[26][4])) 
                      obj->obj_flags.value[2]--;
		 break;
                /* if not weapon, subtract points and go on */
           } else {
		points--;
		break;
          }
	case 14:
	case 15:
           if(obj->obj_flags.type_flag == ITEM_WEAPON) {
                if(points < weights[27][1]) {
		   points--;
                   break;
		}
                points -= weights[27][1];
                if(number(1,weights[32][1]) <= chance) {
                  if(obj->obj_flags.value[1] < (weights[27][4]+stpOrig->obj_flags.value[1])) 
                    obj->obj_flags.value[1]++;
                } 
                else if(obj->obj_flags.value[1] > (stpOrig->obj_flags.value[1]-weights[27][4])) 
                    obj->obj_flags.value[1]--;

		break;
            } else {
                points--;
                break;
            }
	case 16:
	   if(points < weights[30][1]) {
		points--;
		break;
	   }
	   if(iLev <= 1) 
	      break;
	   iTemp = iLev / 10;
	   points -= weights[30][1];
     
	   if(number(1,2) == 1) {
		if( (obj->obj_flags.eq_level >= (iLev + iTemp) ) ||
		    (obj->obj_flags.eq_level >= weights[30][3] ) )
		   break;
		else 
		   obj->obj_flags.eq_level++;
           } else if( (obj->obj_flags.eq_level <= (iLev - iTemp) ) ||
		      (obj->obj_flags.eq_level <= weights[30][2]) )
	 	     break;
	          else
		    obj->obj_flags.eq_level--;
	   break;
	case 17:
        case 18:
	   if(points > 100 && obj->obj_flags.cost < 100000000)  {
		obj->obj_flags.cost += 100;
		points -= 100;
		break;
	   } else {
		points--;
		break;
	   }
	case 19:
        case 20:
	  if( points < weights[28][1] )  {
	       points--;
	       break;
	   }
	   points -= weights[28][1];

	   if( number(1,2) == 1) {
	      /* checks on weapon weight overflow */
	      if(obj->obj_flags.type_flag == ITEM_WEAPON && obj->obj_flags.weight >= 23) 
	      	  break;
	      else
	      	  obj->obj_flags.weight++;
	   } else if(obj->obj_flags.weight > 0)
		obj->obj_flags.weight--;
	     
	  break;
	default:
	  points--;
	  break;
    }
 }
 obj->iValueWeight = weigh(obj);

 return(obj);
}


/* tweak utils */

void do_wset(struct char_data *ch, char *argument, int cmd)
{
 int loop=0, aff, weight;
 char buf[200],  arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

 if(ch == NULL || IS_NPC(ch)) return;

 if(!*argument) {
    send_to_char("Current Weight Table:\r\n",ch);
	for(loop=0;loop <= 24;loop++) {
	    sprintf(buf,"(%d) Aff: %s Weight: %d\r\n",loop,apply_types[loop],weights[loop][1]);	
	    send_to_char(buf,ch);
        }
	sprintf(buf,"(25) Aff: Acap Weight: %d\r\n", weights[25][1]);
	send_to_char(buf,ch);
	sprintf(buf,"(26,27) Aff: Dice Weight: %d, %d\r\n",weights[26][1],weights[27][1]);
	send_to_char(buf,ch);
	sprintf(buf,"(28) (Item Weight) Weight: %d\r\n", weights[28][1]);
        send_to_char(buf,ch);
	sprintf(buf,"(29) (Weapon Const) Weight: %d\r\n",weights[29][1]);
	send_to_char(buf,ch);
        sprintf(buf,"(30) (Levres Cost) Weight: %d\r\n",weights[30][1]);
	send_to_char(buf,ch);
        sprintf(buf,"(31) Global Tweak: %d\r\n",weights[31][1]);
	send_to_char(buf,ch); 
   return;
  }    
  argument = one_argument(argument,arg1);	
  one_argument(argument,arg2);

  if(!*arg1 || !*arg2) {
	send_to_char("Usage: wset (aff#) (newweight)\r\n",ch);
	return;
   }
   if(!is_number(arg1) || !is_number(arg2)) {
	send_to_char("Not a number.\r\n",ch);
	return;
   }

   aff = atoi(arg1);
   weight  = atoi(arg2);
   
   if(aff <= 0 || aff > 32) {
	send_to_char("Invalid Affect\r\n",ch);
        return;
   }

   weights[aff][1] = weight;

   send_to_char("Weight Changed.\r\n",ch);
   return;
}    
  


void do_tweaktest(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH], buf[500];
  struct obj_data *obj;
  int itemnum,loop,w,w1;

  if(ch == NULL || IS_NPC(ch)) return;

  one_argument(argument,arg);
  
  itemnum = atoi(arg);	

  if(itemnum == 0) {
	send_to_char("Invalid Object\r\n",ch);
	return;
  }

  obj = read_object(itemnum,0);

  if(!obj) {
	send_to_char("Invalid Object\r\n",ch);
	return;
  }
    
  send_to_char("OBJECT WEIGH/TWEAK TEST SYSTEM.\r\n",ch);
  w = weigh(obj);
  
  extract_obj(obj);

  for(loop=0;loop<=20;loop++) {
     obj = tweak(read_object(itemnum,0));
     w1 = weigh(obj);
     sprintf(buf,"(%d) OW:%d NW:%d [%s:%d %s:%d ",
		loop, w, w1,
		apply_types[(int)obj->affected[0].location],
		obj->affected[0].modifier,
		apply_types[(int)obj->affected[1].location],
		obj->affected[1].modifier);
     if(obj->obj_flags.type_flag == ITEM_WEAPON)
	 sprintf(buf,"%sDice %dD%d",buf,obj->obj_flags.value[1],obj->obj_flags.value[2]);
     if(obj->obj_flags.eq_level > 1)
	 sprintf(buf,"%s LR: %d",buf,obj->obj_flags.eq_level);
     sprintf(buf,"%s]\r\n",buf);
     send_to_char(buf,ch);
     extract_obj(obj);
   }
}

 
