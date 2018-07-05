/***************************************************************************
*				 MEDIEVIA CyberSpace Code and Data files		           *
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
#include "holocode.h"

		/* external variables and functions */
extern struct room_data *world[MAX_ROOM];
extern struct char_data *mobs[MAX_MOB];
extern struct obj_data *objs[MAX_OBJ];
extern int top_of_world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern char global_color;
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
extern int number_of_rooms;
extern char free_error[100];
extern int port;
extern struct HoloSurvey *survey_list;

bool is_trapped(struct char_data *ch)
{
struct room_affect *rap=NULL;
int room;

	if(!IS_NPC(ch))
		return(FALSE);
	room=ch->in_room;
	if(room<1||!world[room])
		return(FALSE);
	for(rap=world[room]->room_afs;rap;rap=rap->next)
		if(rap->ch==ch)
			return(TRUE);
	return(FALSE);
}

void make_ashes(struct char_data *ch, struct char_data *victim){
	struct room_affect *rap=NULL;
	char my_buf[160];
	int room;

	if(!ch||!victim)return;
	if(victim)room=victim->in_room;
	if(room<0||room>=MAX_ROOM)return;
	if(!world[room])return;

	sprintf(my_buf, "The ashes of %s who was slain by the god %s are scattered here.\n\r", GET_NAME(victim), GET_NAME(ch));

	CREATE(rap, struct room_affect, 1);
	rap->type=RA_ASHES;
	rap->timer=127;
	rap->value=0;
	rap->ch=NULL;
	rap->text=strdup(my_buf);
	rap->room=room;
	rap->next=world[room]->room_afs;
	world[room]->room_afs=rap;
}

void make_gas(int room_number){
	struct room_affect *rap=NULL;
	
	CREATE(rap, struct room_affect, 1);
	rap->type=RA_METHANE;
	rap->timer=number(13,25);
	rap->value=0;
	rap->ch=NULL;
	rap->text=NULL;
	rap->room=room_number;
	rap->next=world[room_number]->room_afs;
	world[room_number]->room_afs=rap;
}


void make_scent(struct char_data *ch, int room)
{
struct room_affect *rap=NULL;
int exists=0;

	if( !world[room] || !ch )
		return;

	rap=world[room]->room_afs;
	while(rap) {
		if(rap->type==RA_SCENT) {
			if(rap->ch == ch) {
				rap->timer = 10;
				rap->value = (int) time(NULL) ;
				exists = 1;
			}
		}
		rap = rap->next;
	}

	if(!exists) {
		CREATE(rap,struct room_affect,1);
		rap->type = RA_SCENT;
		rap->ch	= ch;
		rap->timer = 10;
		rap->value = (int) time(NULL);
		rap->room = room;
		rap->next=world[room]->room_afs;
		world[room]->room_afs=rap;
	}
}

void make_blood(struct char_data *ch, struct char_data *victim)
{
struct room_affect *rap=NULL;
int room;

    if(!ch||!victim)return;
    if(victim)room=victim->in_room;
    if(room<0||room>=MAX_ROOM)return;
    if(!world[room])return;
    CREATE(rap,struct room_affect,1);
    rap->type=RA_BLOOD;
    rap->timer=number(13,25);
    rap->value=0;
    rap->ch=0;
    sprintf(log_buf,"%s killed by %s",(IS_NPC(victim) ? victim->player.short_descr : GET_NAME(victim)),(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    rap->text=str_dup(log_buf);
    rap->room=room;
    rap->next=world[room]->room_afs;
    world[room]->room_afs=rap;
}

int amount_blood_in_room(int room)
{
struct room_affect *rap;
int blood=0;

    if(!world[room])return(0);
    for(rap=world[room]->room_afs;rap;rap=rap->next){
	if(rap->type==RA_BLOOD)
	    blood++;
    }
    return(blood);
}

void room_affect_text(struct char_data *ch)
{
char buf[MAX_STRING_LENGTH];
struct room_affect *rap=NULL;
struct char_data *p=NULL;
char blood=0,flag=0;
int room;
    
    room=ch->in_room;
    if(!world[room])return;
    if(!world[room]->room_afs)return;
    buf[0]=MED_NULL;
    for(rap=world[room]->room_afs;rap;rap=rap->next){
		if(rap->type==RA_MAP){
		    flag++;
	    	if(flag==1){
	        	global_color=35;
		        sprintf(log_buf,"Purple text hovers like a wisp of smoke.. ..  .   .\n\r");
		        send_to_char(log_buf,ch);
		    }
	    	sprintf(log_buf,"%s %d..",rap->text,rap->value);
		    strcat(buf,log_buf);	    	    
		}
    }
    if(flag){
		strcat(buf,"  .. .\n\r");
		send_to_char(buf,ch);
    }
    buf[0]=MED_NULL;
    for(rap=world[room]->room_afs;rap;rap=rap->next){
		if(rap->type==RA_BLOOD)blood++;
		if(rap->type==RA_CAMP){
			global_color=31;
			if(rap->timer>50)
				send_to_char("A new campfire blares brightly..heating the whole area.\n\r",ch);
			else if(rap->timer>40)
				send_to_char("A campfire burns here, steadily shooting off flame and smoke.\n\r",ch);
			else if(rap->timer>30)
				send_to_char("A campfire sputters here...under the flames the timber glows hot orange.\n\r",ch);
			else if(rap->timer>20)
				send_to_char("An old campfire is here..the flames are out but the timber glows hot red.\n\r",ch);
			else if(rap->timer>10)
				send_to_char("An old campfire is here...its timber is almost out..on its last breath.\n\r",ch);
			else
				send_to_char("An old campfire is here..no flames or hot timber remain.\n\r",ch);
			global_color=0;
		}
		if(rap->type==RA_SHIELD){
	    	global_color=33;
		    send_to_char("Surrounding the area is a shimmering shield, distorting everything you see.\n\r",ch);
		    global_color=0;
		}
		if(rap->type==RA_METHANE){
			global_color=33;
			send_to_char("A cloudy gas creates a haze in the room and emits a faint odor.\n\r", ch);
			global_color=0;
		}
		if(rap->type==RA_ASHES){
			global_color=0;
			send_to_char(rap->text, ch);
		}
		if(rap->type==RA_TRAP){
	    	if(!rap->ch)continue;
		    for(p=world[rap->room]->people;p;p=p->next_in_room)
				if(p==rap->ch)
			    	break;
		    if(p&&!rap->value&&rap->ch->master&&rap->ch->master==ch->master){
				sprintf(log_buf,"You see the trap %s set, it looks un-touched.\n\r",GET_NAME(rap->ch));
				global_color=33;
				send_to_char(log_buf,ch);
				global_color=0;
	    	}else{
				if(rap->value&&rap->ch){
				    for(p=world[rap->room]->people;p;p=p->next_in_room){
						if(rap->ch==p){
						    global_color=33;
						    act("*** You see $N struggling caught in a trap ***",TRUE,ch,0,p,TO_CHAR);
						    global_color=0;
						}
		    		}
				}
	    	}
		}
    }
    if(blood){
		if(blood>10)
	    	strcpy(buf,"Cringe! A Real blood bath has occured here...blech!\n\r");
		else if(blood>8)
		    strcpy(buf,"The whole area is splattered with blood and guts. <shiver>.\n\r");
		else if(blood>7)
		    strcpy(buf,"Blood, brains and other organs are hanging from everything possible...\n\r");
		else if(blood>6)
		    strcpy(buf,"Blood and guts are everywhere, the smell of it makes you cringe.\n\r");
		else if(blood>4)
		    strcpy(buf,"Puddles of blood cover the area.\n\r");
		else if(blood>2)
		    strcpy(buf,"Some blood and guts are on the ground.\n\r");
		else if(blood>1)
		    strcpy(buf,"There is a small pool of blood that has been spilt here.\n\r");
		else if(blood>0)
		    strcpy(buf,"Some drops of blood can be seen on the ground.\n\r");
		global_color=31;
		send_to_char(buf,ch);
		global_color=0;
    }            
}

void remove_room_affect(struct room_affect *ra, char type)
{
int r;
struct room_affect *rap=NULL;
struct char_data *ch=NULL;
struct obj_data *obj=NULL;
struct HoloSurvey *stpS,*stpSRemove;

    r=ra->room;
    if(world[r]->room_afs==ra){
		world[r]->room_afs=ra->next;
    }else{
        rap=world[r]->room_afs;
        while(rap->next&&rap->next!=ra)rap=rap->next;
		if(!rap->next){
		    log_hd("### no room affect found in remove_room_affect");
	    	return;
		}
		rap->next=ra->next;
    }
    if(ra->type==RA_SHIELD){
		global_color=31;
		send_to_room("Vertigo slams into your stomach as the magical shield on the area collapses.\n\r ",r);
		global_color=0;
    }else if(ra->type==RA_CAMP){
    	global_color=31;
		send_to_room("The fire slowly fades and sputters out...\n\r",r);
		/* remove the pillar of smoke from survey list*/
		for(stpS=survey_list;stpS->next;stpS=stpS->next){
			if(stpS->next->iX==world[ra->room]->holox
					 &&stpS->next->iY==world[ra->room]->holoy
					 &&stpS->next->dist==77){
			 	break;
			}
		}
		if(stpS->next){
			stpSRemove=stpS->next;
			stpS->next=stpS->next->next;
			stpSRemove->description=my_free(stpSRemove->description);
			stpSRemove=my_free(stpSRemove);
		}
		global_color=0;    	
    }else if(ra->type==RA_MAP){
		send_to_room("You startle as some purple text POPS with an echo.\n\r",r);
    }else if(ra->type==RA_BLOOD){
		switch(number(0,5)){
		    case 0:
	    		send_to_room("Some blood shimmers and is drawn into the depths of Medievia.\n\r",r);
	    		break;
		    case 1:
				send_to_room("You watch horrified as some blood coalesces into a puddle and rolls away.\n\r",r);
				break;
		    case 2:
				send_to_room("A hand punches through the ground and grabs some entrails, pulling it under.\n\r",r);
				break;
		    case 3:
				send_to_room("A patch of blood turns different colors and slowly becomes part of the ground.\n\r",r);
				break;
		    case 4:
				send_to_room("Pools of blood shift and sink quickly into the ground.\n\r",r);
				break;
		    case 5:
				send_to_room("A dismembered body part catches fire, burns quickly into ashes which blow away.\n\r",r);
				break;
		}
    }else if(ra->type==RA_TRAP){
		if(!ra->value){
		    if(number(1,100)<=10){
				global_color=33;
				send_to_room("**SNAP** A trap misfires and breaks.\n\r",ra->room);
				global_color=0;
				obj=read_object(9823,0);
				obj_to_room(obj,ra->room);		
		    }else{
				global_color=33;
				send_to_room("**SNAP** A trap misfires.\n\r",ra->room);
				global_color=0;
				obj=read_object(9822,0);
				obj_to_room(obj,ra->room);		
	    	}
		}else{
		    if(type==1){
	    	    obj=read_object(9822,0);
	        	obj_to_room(obj,ra->room);		
		    }else{
		        for(ch=world[ra->room]->people;ch;ch=ch->next_in_room)
				    if(ch==ra->ch)
		        		break;
		        if(ch){
				    global_color=33;
				    act("$n struggles and escapes from the Trap!",TRUE,ch,0,0,TO_ROOM);
				    global_color=0;
				    obj=read_object(9822,0);
				    obj_to_room(obj,ra->room);		
	    	    }else{
		    		global_color=33;
				    send_to_room("**SNAP** A trap misfires.\n\r",ra->room);
				    global_color=0;
				    obj=read_object(9822,0);
				    obj_to_room(obj,ra->room);		
    	        }
		    }
 		}
    }
    if(ra->text)
       ra->text = my_free(ra->text);
    ra = my_free(ra);
}

void remove_blood(int room, int blood)
{
struct room_affect *rap,*n_rap;

    rap=world[room]->room_afs;
    while(rap&&blood){
	n_rap=rap->next;
	if(rap->type==RA_BLOOD)
	    remove_room_affect(rap, 123);
	rap=n_rap;
	blood--;
    }
}

void room_affect_update(void)
{
int r;
struct room_affect *ra=NULL,*ran=NULL;

    for(r=0;r<MAX_ROOM;r++){
		if(world[r]&&world[r]->room_afs){
		    for(ra=world[r]->room_afs;ra;ra=ran){
				ran=ra->next;
	        	if(ra->timer!=-1){
	            	ra->timer--;
				    if(ra->type==RA_BLOOD)
						ra->value++;
				    if(ra->timer==1&&ra->type==RA_SHIELD)
						send_to_room("The area dazzles and shimmers..the shield beginning to loose its anchor...\n\r",ra->room);
					if(ra->type==RA_CAMP){
						if(number(0,3)==2){
							global_color=34;
							send_to_room("The fire pops and crackles as it continues to burn.\n\r",r);
							global_color=0;
						}
					}					
				}
		    	if(ra->timer==0)
			    remove_room_affect(ra,0);	    
	    	}
		}
    }
}
