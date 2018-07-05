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

#define MAX_X 35
#define MAX_Y 29
#define MAX_Z 6
#define ROOMS_PER_LEVEL 333
#define WATER_REAPER 1
#define WIND_TUNNEL 2

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
extern char global_color;
extern struct zone_data *zone_table;  
extern int top_of_zone_table;  
extern struct index_data mob_index_array[MAX_MOB]; 
extern int iHighRoom;
extern struct index_data *mob_index;  
extern struct index_data obj_index_array[MAX_OBJ];  
extern struct index_data *obj_index;  
extern void remove_from_formation(struct char_data *ch);
extern int top_of_mobt;  
extern int top_of_objt; 
extern struct descriptor_data *descriptor_list;  
extern int number_of_rooms;
extern void spell_dispel_magic(byte level, struct char_data *ch,struct char_data *victim, struct obj_data *obj);
extern int dice(int number, int size);  
extern int number(int from, int to); 
extern void space_to_underline(char *text);
extern void page_string(struct descriptor_data *d, char *str, int keep_internal); 
extern int file_to_string(char *name, char *buf);
extern bool put_in_formation(struct char_data *leader, struct char_data *follower); 
extern bool is_formed(struct char_data *ch);
extern struct global_clan_info_struct global_clan_info;
extern bool update_clan_eggs(int clan, int eggs);
extern void tell_clan(int clan, char *argument);
int gate_room,gate_dir;

struct moveable{
    int room;
    char type;
    int dir;
    int age;
};

struct moveable *moveables;

/*		/Local variables and text descriptions\			*/
/*----------------------------------------------------------------------*/
char catacombs_loaded;
int rooms[MAX_Z];
int level;
int grid[MAX_X][MAX_Y][MAX_Z];
char type[MAX_X][MAX_Y][MAX_Z];
int entrance_room;


/* These room description variables will be static and sizeable
   they will be even number room name odd number descriptions
   allocated dynamically to save memory */
#define MAX_DESCRIPTIONS 40
#define MAX_DESCRIPTION_TYPE 18
#define BEACH_DES 0
#define CAVERN_DES 1
#define ENTRCAT_DES 2
#define HOLE_DES 3
#define HUGE_DES 4
#define ISLAND_DES 5
#define LAKE_DES 6
#define STREAM_DES 7
#define VERT_DES 8
#define TUNNEL1_DES 9
#define TUNNEL2_DES 10
#define TUNNEL3_DES 11
#define BEND_DES 12
#define CHASM_DES 13
#define WATERFALL_DES 14
#define CAVE_PAINTING_DES 15
#define CAVE_CORREL_DES 16
#define CAVE_MANA_DES 17

char *descriptions[MAX_DESCRIPTION_TYPE][MAX_DESCRIPTIONS];

void link_catacombs(void)
{
int room,keep_trying=0;
char buf[100];

   while(++keep_trying<500000){
	room=number(100,MAX_ROOM);
	if(!world[room])continue;
	if(world[room]->zone==198)continue;
	if(world[room]->zone==1)continue;
	if(world[room]->sector_type<2||world[room]->sector_type>5)continue;
	if(world[room]->dir_option[DOWN])continue;	
	if(world[room]->level_restriction)continue;
	if(zone_table[world[room]->zone].continent==TRELLOR)continue;
	sprintf(buf,"u %d",room);
	do_makedoor(0,buf,MAX_ROOM-1);
	sprintf(buf,"d %d",MAX_ROOM-1);
	do_makedoor(0,buf,room);
	sprintf(buf,"##COMBS linked to room %d",room);
	log_hd(log_buf);
	return;	
   }
}



void dream_weaver(struct char_data *ch)
{
int room,x,y,z,limit,flag;
struct char_data *m;

    for(x=0;x<3;x++){
	for(y=0;y<3;y++){
	    if(ch->master->formation[x][y]){
		if(GET_POS(ch->master->formation[x][y])!=POSITION_SLEEPING)
		    return;
		if(ch->in_room!=ch->master->formation[x][y]->in_room)
		    return;
	    }
	}
    }
    limit=0;
    while(++limit<25000){
	x=number(0,MAX_X);
	y=number(0,MAX_Y);
	z=number(0,MAX_Z);
	if(!grid[x][y][z])continue;
	room=grid[x][y][z];
	if(room>MAX_ROOM-1)continue;
	if(room<1)continue;
	if(!world[room])continue;
	flag=0;
	for(m=world[room]->people;m;m=m->next_in_room)
	   if(IS_NPC(m))
		flag=1;
	if(flag)continue;
	if(world[room]->sector_type)continue;
    	for(x=0;x<3;x++){
	    for(y=0;y<3;y++){
		if(ch->master->formation[x][y]){
		    char_from_room(ch->master->formation[x][y]);
		    char_to_room(ch->master->formation[x][y],room);
		}
	    }
	}
	return;
    }
}

bool stuck_in_water_reaper(struct char_data *ch)
{
    if(!moveables)return(FALSE);
    if(ch->in_room==moveables->room&&moveables->type==WATER_REAPER)
	return(TRUE);
    return(FALSE);
}

void make_catacomb_moveable(void)
{
int x,y,z;
    if(moveables)return;
    while(1){
		x=number(0,MAX_X-1);
		y=number(0,MAX_Y-1);
		z=number(0,MAX_Z-1);
		if(!grid[x][y][z])continue;
		if(world[grid[x][y][z]]->sector_type!=SECT_INSIDE)continue;
		break;
    }
    CREATE(moveables,struct moveable,1);
    if(number(1,100)<50)
		moveables->type=WATER_REAPER;
    else 
		moveables->type=WIND_TUNNEL;
    moveables->room=grid[x][y][z];
    moveables->dir=number(0,3);
    moveables->age=0;
}

void move_catacomb_moveable(void)
{
int room, to_room, op_dir,x,y,z,xa,xb,ya,yb,a,b,xx,yy;
struct char_data *p,*pn;
struct obj_data *obj;
char flag;

    if(!moveables){
		make_catacomb_moveable();
		return;
    }
    room=moveables->room;
    moveables->age++;
    if(moveables->age>85){
        if(moveables->type==WATER_REAPER)
       	    send_to_room("The water Reaper suddenly looses its tension and SPLASHES away!\n\r",moveables->room);
		moveables=my_free(moveables);
		return;
    }
    if(world[room]->sector_type==SECT_WATER_NOSWIM){
	if(moveables->type==WATER_REAPER){
	    global_color=33;
	    send_to_room("
The Water Reaper touches the lake and instantly looses its tension.
It collapses in a splash and becomes just more water in the lake.
",room);
	    global_color=0;
	    moveables=my_free(moveables);
	    return;
	}
    }
    if(moveables->type==WIND_TUNNEL){
	global_color=32;
	send_to_room("
ROARING WIND FLOODS THE AREA.....YOU ARE BOMBARDED BY ROCK AND STONE...
THE GROUND SHAKES ...  WALLS RUMBLE...  You hear screaming....  then..
\n\r",room);
	if(moveables->age>10){
	for(p=world[room]->people;p;p=pn){
	    pn=p->next_in_room;
	    if(GET_POS(p)!=POSITION_SITTING){
		remove_from_formation(p);
		act("$n is picked up and FLUNG into the wall!",TRUE,p,0,0,TO_ROOM);
		send_to_char("You are picked up and FLUNG into a wall!\n\r",p);
		if (IS_NPC(p))
		  GET_HIT(p)/=8;
		else
		  GET_HIT(p) -= number(100,450);
		if (GET_HIT(p) < 1) {
		  send_to_char("You feel a sharp pain as your head crashes into the cavern wall.\r\n", p);
		  act("$n falls limpy to the ground, blood pouring from the crack in thier skull.",TRUE,p,0,0,TO_ROOM);
	  	  global_color=31;
		  act("$n is DEAD!\r\n",TRUE,p,0,0,TO_ROOM);
		  send_to_char("You have been KILLED!\r\n\r\n", p);
		  global_color=0;
		  raw_kill(p,NULL);
		}
		if(!IS_NPC(p))
		for(obj=object_list;obj;obj=obj->next){
		    if(obj->item_number==211){
			flag=0;
			if(obj->carried_by==p){
			    flag=1;
			}else if(obj->worn_by==p){
			    flag=1;
			}else if(obj->in_obj){
			    if(obj->in_obj->carried_by==p){
			        flag=1;
			    }else if(obj->in_obj->worn_by==p){
			        flag=1;
			    }
			}
			if(flag&&number(1,100)<50){
		            
			    if(obj->name&&obj->name!=objs[obj->item_number]->name){
		                obj->name = my_free( obj->name);
				obj->name=str_dup(objs[212]->name);
			    }else{
				obj->name=objs[212]->name;
			    }
		            if(obj->description&&obj->description!=objs[obj->item_number]->description){
		                obj->description = my_free( obj->description);
				obj->description=str_dup(objs[212]->description);
			    }else{
				obj->description=objs[212]->description;
			    }
		            if(obj->short_description&&obj->short_description!=objs[obj->item_number]->short_description){
		                obj->short_description = my_free( obj->short_description);
				obj->short_description=str_dup(objs[212]->short_description);
			    }else{
				obj->short_description=objs[212]->short_description;
			    }
			    obj->item_number=212;
			}
		    }
		}	
	    }
        }
        }
    }
    op_dir=0;
    switch(moveables->dir){
	case NORTH:
	    op_dir=SOUTH;
	    break;
	case SOUTH:
	    op_dir=NORTH;
	    break;
	case WEST:
	    op_dir=EAST;
	    break;
	case EAST:
	    op_dir=WEST;
	    break;
    }
    if(world[room]->dir_option[moveables->dir]){
	to_room=world[room]->dir_option[moveables->dir]->to_room;
    }else{
	x=0;
        if(world[room]->dir_option[0])x++;
        if(world[room]->dir_option[1])x++;
        if(world[room]->dir_option[2])x++;
        if(world[room]->dir_option[3])x++;
	x--;/*cause one exit is opposite dir*/
	if(x<1){/* only way out is way in */
	    if(!world[room]->dir_option[op_dir]){
		moveables=my_free(moveables);/*something wrong, bail*/
		return;
	    }
	    if(moveables->type==WIND_TUNNEL){
		moveables=my_free(moveables);
		return;
	    }
	    to_room=world[room]->dir_option[op_dir]->to_room;
	    moveables->dir=op_dir;
	}else{
	    while(1){
		x=number(0,3);
		if(x==op_dir)continue;
		if(!world[room]->dir_option[x])
		    continue;
		moveables->dir=x;
		break;
	    }
	    to_room=world[room]->dir_option[x]->to_room;
	}
    }
    moveables->room=to_room;
    if(moveables->type==WATER_REAPER){
	global_color=32;
	send_to_room("A GIANT WATER REAPER bores into the room...your swept up into its form..\n\r",to_room);
	send_to_room("As the Water Reaper Flows...you flow with it...\n\r",room);
	global_color=0;
        for(p=world[room]->people;p;p=pn){
    	    pn=p->next_in_room;
	    if(IS_NPC(p))continue;
	    if(p->specials.fighting)
		stop_fighting(p);
	    char_from_room(p);
	    char_to_room(p,to_room);	    
	    do_look(p,"",15);
        }
    }
    if(moveables->type==WIND_TUNNEL){
	x=world[to_room]->pressure_mod;
	y=world[to_room]->temperature_mod;
	z=world[to_room]->mount_restriction;
	xa=x-5;if(xa<0)xa=0;
	ya=y-5;if(ya<0)ya=0;
	xb=x+5;if(xb>=MAX_X-1)xb=MAX_X-1;
	yb=y+5;if(yb>=MAX_Y-1)yb=MAX_Y-1;
	for(xx=xa;xx<=xb;xx++)
	    for(yy=ya;yy<yb;yy++){
		if(!grid[xx][yy][z])continue;
		a=x-xx;
		if(a<0)a*=-1;
		b=y-yy;
		if(b<0)b*=-1;
		global_color=32;
		if(a!=0&&b!=0)
		switch(MAX(a,b)){
		    case 1:
			send_to_room("ALL AROUND YOU ROCKS FALL AND DUST KICKS UP...YOUR STUNG BY FLYING DEBRIS!\n\r",grid[xx][yy][z]);
			break;
		    case 2:
			send_to_room("Your heartbeat PULSES as the area ROARS with Power....\n\r",grid[xx][yy][z]);
			break;
		    case 3:
			send_to_room("The ground Vibrates and the air becomes agitated as wind blows closeby!\n\r",grid[xx][yy][z]);
			break;
		    case 4:
			send_to_room("From far away the sounds of wind rumbles the area.\n\r",grid[xx][yy][z]);
			break;
		    case 5:
			send_to_room("The sound of Gushing wind gets your attention.\n\r",grid[xx][yy][z]);
			break;
		    		
		}	
		global_color=0;
	    }		
    }
}

void stalactite(void)
{
struct descriptor_data *d;
int room;

    for(d=descriptor_list;d;d=d->next){
	if(!d->character||d->connected!=CON_PLAYING)
	    continue;
	if(d->character->in_room<1||world[d->character->in_room]->zone!=198)
	    continue;
	if(world[d->character->in_room]->sector_type==SECT_UNDERWATER)
	    continue;
	if (IS_HOVERING(d->character))
	    continue;
	if(number(0,5)==0){
	    room=d->character->in_room;
	    global_color=32;
	    send_to_room("You hear a *CRACK* from above and watch as a stalactite breaks free.\n\r",room);	    
	    if(number(1,30) < GET_DEX(d->character) &&
			GET_POS(d->character) > POSITION_SITTING){
		act("$n Dives out of the way of the falling rocks...",TRUE,d->character,0,0,TO_ROOM);
	        send_to_char("You Dive out of the way of the falling rock.\n\r",d->character);
	    }else{
		GET_HIT(d->character) -= number(50,300);
		if (GET_HIT(d->character) < 1) {
		  send_to_char("You feel a sharp pain as the massive stone penetrates you armor and into your chest.\r\n", d->character);
		  act("$n gasps in terror as the massive stone stabs $s through the chest.",TRUE,d->character,0,0,TO_ROOM);
		  global_color=31;
		  act("$n is DEAD!\r\n",TRUE,d->character,0,0,TO_ROOM);
		  send_to_char("You have been KILLED!\r\n\r\n", d->character);
		  global_color=0;
		  raw_kill(d->character,NULL);
		} else {
		  act("$n is struck and pushed down to $s knees by the falling rock",TRUE,d->character,0,0,TO_ROOM);
		  send_to_char("The falling stalactite POUNDS you to your knees.\n\r",d->character);
		  GET_POS(d->character)=POSITION_SITTING;
		}
	    }
	    global_color=0;
	}
    }

}

int cave_correl(struct char_data *ch, int cmd, char *arg)
{
struct char_data *m;

    if(world[ch->in_room]->light<1)
	return(FALSE);
    global_color=31;
    send_to_room("The Cave Coral Shoots waves of darts towards the light!\n\r",ch->in_room);
    global_color=0;
    for(m=world[ch->in_room]->people;m;m=m->next_in_room){
	if(IS_NPC(m))continue;
	if(IS_AFFECTED(m,AFF_REP_ROOT))continue;
	if(m->equipment[WEAR_LIGHT]){
	    global_color=33;
	    act("$n winces in pain as flying darts pierce his neck and face.",TRUE,m,0,0,TO_ROOM);
	    send_to_char("You wince in pain as flying darts pierce your neck and face.\n\r",m);
	    global_color=0;
	    GET_HIT(m) -= number(100,450);
	    if (GET_HIT(m) < 1) {
		send_to_char("Blood poors from the multitude of fatal wounds.\r\n", m);
		act("$n suffers from bloodloss from the hundreds of wounds.",TRUE,m,0,0,TO_ROOM);
	  	global_color=31;
		act("$n is DEAD!\r\n",TRUE,m,0,0,TO_ROOM);
		send_to_char("You have been KILLED!\r\n\r\n", m);
		global_color=0;
		raw_kill(m,NULL);
	    }
	}
    }
    return(FALSE);
}
int cave_paintings(struct char_data *ch, int cmd, char *arg)
{
struct char_data *mob=NULL;

    if(number(1,100)<75)return(FALSE);
    if(ch->specials.fighting)return(FALSE);
    if (IS_HOVERING(ch)) return(FALSE);
    for(mob=world[ch->in_room]->people;mob;mob=mob->next_in_room){
	if(!IS_NPC(mob)){
	    if(mob->specials.fighting)
		return(FALSE);
	    mob=read_mobile(17018,REAL);
	    char_to_room(mob,ch->in_room);
	    send_to_room("A CHILL runs up your spine as a 2 dimensional cave painting PEELS off the wall.\n\r",ch->in_room);
	    return(TRUE);
	}
    }
    return(FALSE);
}

int egg_collector(struct char_data *ch, int cmd, char *arg)
{
struct obj_data *obj;
struct char_data *k=NULL;
int x=0,room;
char buf[MAX_INPUT_LENGTH];
char buf2[MAX_INPUT_LENGTH];

    if(!world[MAX_ROOM-1])return(FALSE);
    for (k = world[ch->in_room]->people; k; k = k->next_in_room){
        if ( IS_MOB(k) )
            if (mob_index[k->nr].func==egg_collector)
                  break;
    }
    if(!k)return FALSE;

    if(cmd==53){ /*where*/
	act("$n asks for the location of the catacombs..",TRUE,ch,0,0,TO_ROOM);
	send_to_room("\n\r",ch->in_room);
	act("$N looks at a magic orb placed here by Vryce....",TRUE,ch,0,k,TO_ROOM);
	if(!world[MAX_ROOM-1]->dir_option[UP])
	    return(FALSE);
	room=world[MAX_ROOM-1]->dir_option[UP]->to_room;
	act("$n says, 'lets see here...I see an entrance at...'",TRUE,k,0,0,TO_ROOM);
	send_to_room("\n\r",ch->in_room);
	send_to_room(world[room]->name,ch->in_room);
	send_to_room("\n\r",ch->in_room);
	act("$n says, 'in the general area of ....'",TRUE,k,0,0,TO_ROOM);
	send_to_room(zone_table[world[room]->zone].name,ch->in_room);
	send_to_room("\n\r",ch->in_room);
	return(TRUE);
    }
    if(cmd==70){ /* offer */
	if(!arg||!arg[0]){
	    send_to_char("You need to offer a certain amount of eggs, type list.\n\r",ch);
	    return(TRUE);
	}
	arg =one_argument(arg,buf);
	x=atoi(buf);
	switch(x){
		case 650:
			if(ch->specials.eggs<650) {
				send_to_char("You don't have 650 eggs on the talley.\r\n",ch);
				return(TRUE);
			}
			one_argument(arg,buf2);
			if(!buf2 || !*buf2) {
				send_to_char("
\rYou need to specify the object that you wish to have it's life increased.
\rExample: offer 650 bloodstone
\rIMPORTANT: Make sure you SPECIFY the correct item in your inventory so if you
\rhave more than one item with the same name, make sure you are increasing
\rthe life in the correct time.. There will be no refunds for people who
\rincrease the life on the life of the wrong item.. If you had two stones 
\ryour inventory and you wished to increase the life of the second you would
\rtype: offer 650 2.stone.\r\n",ch);
				return(TRUE);
			}
			obj = get_obj_in_list_vis(ch,buf2,ch->carrying);
			if(!obj) {
				send_to_char("No such object was found in your inventory.\r\n",ch);
				return(TRUE);
			}
			if(obj->iDetLife == -1) {
				send_to_char("Why fix something allready perfect?\r\n",ch);
				return(TRUE);
			}
			if(obj->iLastDet == -1) {
				send_to_char("You can only increase the life of an object once.\r\n",ch);
				return(TRUE);
			}
			if(obj->iLastDet > 1) {
				send_to_char("You can not increase the life of an object once it has started to crumble.\r\n",ch);
				return(TRUE);
			}
			ch->specials.eggs -= 650;
			obj->iDetLife += 3888000; /* 40 Days */
			obj->iLastDet = -1;
			send_to_char("
The wizard makes several motions with his hands and says a short encantment.
The room glows with a blinding white light focused on the object. You see
the life power slowly seep into the object. \r\n",ch);
			act("The room is filled with a blinding white white light.",TRUE,ch,0,0,TO_ROOM);
			return(TRUE);
			break; 
		case 40:
			if(ch->specials.eggs<40){
			    send_to_char("You dont have 40 eggs on the tally!\n\r",ch);
		    	return(TRUE);
			}
			if((ch->player.time.birth+SECS_PER_MUD_YEAR)>time(0)){
				send_to_char("You are young enough!\n\r",ch);
				return(TRUE);
			}
			ch->specials.eggs-=40;
			ch->player.time.birth+=SECS_PER_MUD_YEAR;
			send_to_char("You feel ......younger...\n\r",ch);
			act("$n smiles as a blue haze envelopes $m momentarily.",TRUE,ch,0,0,TO_ROOM);
			return(TRUE);
			break;

	    case 500:
			if(ch->specials.practices>69){
			    send_to_char("You can only hold 99 practices so to get 30 more you need to have less \n\rthen 70.\n\r",ch);
			    return(TRUE);
		  	}
			if(ch->specials.eggs<500){
			    send_to_char("You dont have 500 eggs on the tally!\n\r",ch);
		    	return(TRUE);
			}
			ch->specials.eggs-=500;
			ch->specials.practices+=30;
			send_to_char("You feel enlightened...\n\r",ch);
			act("$n smiles as a blue haze envelopes $m momentarily.",TRUE,ch,0,0,TO_ROOM);
			return(TRUE);
			break;
	    case 1000:
			send_to_char("Go to the HIGHEST place in medievia and press button.\n\r",ch);
			return(TRUE);
	
	}
    }

    if(cmd==332){/*fill*/
	do{
	    for(obj=ch->carrying;obj;obj=obj->next_content)
        	if(obj->item_number==211)break;
	    if(obj){
			x++;
			extract_obj(obj);
	    }
	}while(obj);	
	if(x<1){
	    send_to_char("You realize you have no eggs..\n\r",ch);
	    return(1);
	}
	ch->specials.eggs+=x;
	SAVE_CHAR_OBJ(ch,-20);
  	act("$n puts all $s eggs in $N's bag.",TRUE,ch,0,k,TO_ROOM);
	act("You put all your eggs in $N's bag.",TRUE,ch,0,k,TO_CHAR);
	sprintf(log_buf,"$n counts %d eggs in the bag and empties them down a shoot.",x);
	act(log_buf,TRUE,k,0,0,TO_ROOM);
	act("$N smiles at $n and adds the count to $n's tally sheet.",TRUE,ch,0,k,TO_ROOM);
	act("$N smiles at you and adds the count to your tally sheet.",TRUE,ch,0,k,TO_CHAR);
	if(ch->specials.clan){
		global_clan_info.eggs[(int) ch->specials.clan]+=x;
		if(update_clan_eggs(ch->specials.clan,x)){
			sprintf(log_buf,"[CLAN] %s turns in %d EGGS.\n\r",GET_NAME(ch),x);
			tell_clan(ch->specials.clan,log_buf);
		}else{
			send_to_char("An error occured when trying to update your clan file.\n\r",ch);
		}
	}
	return(TRUE);
    }
    if(cmd==328){/*ask*/
	act("$n bends over $s ledger and quickly turns pages....",TRUE,k,0,0,TO_ROOM);
	sprintf(log_buf,"%s Ledger says your tally is %d eggs.",GET_NAME(ch),ch->specials.eggs);
	do_talk(k,log_buf,9);
	return(TRUE);
    }
    if(cmd==59){/*list*/
	send_to_char("
#EGGS  REWARD
-------------
  40   Become one year younger.
 500   30 Practices to help you better fight the war.
 650   Will add a month and a half to a objects life. Make sure to do a
       a 'HELP EQLIFE'  before receiving this award. 
1000   Vryce will make you REBORN into a new class(MULTICLASS).
",ch);
	return(TRUE);
    }
    return(FALSE);
}


int caveweed(struct char_data *ch, int cmd, char *arg)
{

    if(cmd<1||cmd>6)return(FALSE);
    if(number(1,100)<50)return(FALSE);
    send_to_room("The Caveweed makes moving impossible by grabbing everyones legs.\n\r",ch->in_room);
    return(TRUE);
}

int darksprite(struct char_data *ch, int cmd, char *arg)
{
struct char_data *mob=NULL;

    if(number(0,100)<50)return(FALSE);
    if(is_formed(ch))return(FALSE);
    if(ch->specials.fighting)return(FALSE);
    for(mob=world[ch->in_room]->people;mob;mob=mob->next_in_room){
	if(!IS_NPC(mob)){
	    if(mob->specials.fighting)continue;
	    act("$n grabs $N's leg and holds on tight.",TRUE,ch,0,mob,TO_ROOM);
	    act("$n grabs your leg and holds on tight.",TRUE,ch,0,mob,TO_VICT);
	    put_in_formation(mob->master,ch);
    	    return(TRUE);
	}
    }    
    return(FALSE);
}

int caveray(struct char_data *ch, int cmd, char *arg)
{
char flag=0;
struct char_data *m=NULL,*next=NULL;
int room=0;

    if(ch->specials.fighting)return(FALSE);
    if(number(0,100)<70)return(FALSE);
    for(m=world[ch->in_room]->people;m;m=next){
	next=m->next_in_room;
	if(!IS_NPC(m)){
	   if(m->specials.fighting)return(FALSE);
	   flag++;
	   if(flag==1){
			room=grid[world[m->in_room]->pressure_mod][world[m->in_room]->temperature_mod][world[m->in_room]->mount_restriction+1];
		if(room<MAX_ROOM-5000||!world[room])return(FALSE);
		global_color=33;
		send_to_room("
The huge caveray Slides away...THAT WAS NO FLOOR YOU WERE ON!
OOOOoooooooh!
You fall down into the unknown...
",m->in_room);
		send_to_room("
The ceiling above opens and people fall through!
",room);	
		global_color=0;
	   }
	   char_from_room(m);
	   char_to_room(m,room);
	}
    }
    return(TRUE);
}

int whirlwind(struct char_data *ch, int cmd, char *arg)
{
char flag=0;
struct char_data *m=NULL,*next=NULL;
int room=0;

    if(ch->specials.fighting)return(FALSE);
    if(number(0,100)<70)return(FALSE);
    for(m=world[ch->in_room]->people;m;m=next){
		next=m->next_in_room;
		if(!IS_NPC(m)){
		   if(m->specials.fighting)return(FALSE);
		   flag++;
		   if(flag==1){
				room=grid[world[m->in_room]->pressure_mod][world[m->in_room]->temperature_mod][world[m->in_room]->mount_restriction-1];
				if(room<MAX_ROOM-1000||!world[room])return(FALSE);
				global_color=33;
				send_to_room("
The Whirlwind moves Towards you!.....WWWwwWWwWWWWSSHSHSHSsshhSSSHHH!
OOOOoooooooh!
You fling upward into the unknown...
",m->in_room);
				send_to_room("
A LOUD rush of air Pounds into the area and people appear!
",room);	
				global_color=0;
			}
	   		char_from_room(m);
	   		char_to_room(m,room);
		}
    }
    return(TRUE);
}

char echo_text(struct char_data *ch,char *text)
{
char t[MAX_INPUT_LENGTH], word[MAX_STRING_LENGTH];
char p=0;
int w=0;

    if(!text||text[0]==MED_NULL)return(FALSE);
    if(world[ch->in_room]->sector_type==SECT_UNDERWATER)return(FALSE);
    while(text[p])p++;
    p--;
    while(text[p]!=' '&&p>0)p--;
    p++;
    while(p&&isalpha(text[p]))
	word[w++]=text[p++];
    word[w]=MED_NULL;
    if(strlen(word)){
	strcpy(log_buf,text);
	while(text[p])p++;
    	p--;
	while(p&&ispunct(text[p]))text[p--]=MED_NULL;
	strcpy(log_buf,text);
	sprintf(t,"%s %s   %s....  .    .",word,word,word);
	strcat(log_buf,t);
	return(TRUE);
    }
    else return(FALSE);
    

}

int fetch_a_room(char room_type)
{
int x,y,z,s;

    s=0;
    while(++s<50000000){
    	x=number(0,MAX_X);
	y=number(0,MAX_Y);
	z=number(0,MAX_Z);
	if(room_type){
	    if(type[x][y][z]==room_type)
		if(grid[x][y][z])
		    if(grid[x][y][z]>MAX_ROOM-5000&&grid[x][y][z]<MAX_ROOM)
		        return(grid[x][y][z]);
	}else{
	    if(type[x][y][z])
		if(grid[x][y][z])
		    if(grid[x][y][z]>MAX_ROOM-5000&&grid[x][y][z]<MAX_ROOM)
		        return(grid[x][y][z]);
	}
    }
    log_hd("## fetch_a_room in catacomb.c went 50000000 deep");
    return(MAX_ROOM-1001);
}

void load_some(int amount, int mob_num, char type)
{
int x,room,y,z;
struct char_data *mob=NULL;
struct obj_data *obj;

    for(x=0;x<amount;x++){
        room=fetch_a_room(type);
        if(room>0&&world[room]){
	    mob=read_mobile(mob_num,REAL);
	    if(mob){
		char_to_room(mob,room);
		if(mob_num==17035){/*hermit dude */
		    obj=read_object(215,REAL);/*repaerdnim root */
		    obj_to_char(obj,mob);
		}
		if(mob_num==17021){/*dream weaver */
		    obj=read_object(18013,REAL);
		    equip_char(mob,obj,HOLD);
		    obj=read_object(18014,REAL);
		    equip_char(mob,obj,WEAR_FINGER_L);
		}
		y=number(0,5);
		if(y>2)
		    y=number(0,15);
		else
		    y=0;
		for(z=0;z<y;z++){
		    obj=read_object(211,REAL);
		    obj_to_char(obj,mob);
		}
	    }
	}
    }
}

void populate_catacombs(void)
{
int x,y,z,t,a,b,c=0;
struct char_data *mob=NULL;
struct obj_data *obj;

    fprintf(stderr,"CATACOMB: Populating Catacombs......\n");

    for(x=1;x<MAX_ROOM;x++){
	if(world[x]&&world[x]->zone==198){
	    if(number(0,10)==10){
	        y=number(0,5);
		for(z=0;z<y;z++){
		    obj=read_object(211,REAL);
		    obj_to_room(obj,x);
		}
	    }
	}	
    }
    load_some(30,17019,0);  	/* cave mist			*/
    load_some(12,17020,'C');	/* giant spider 		*/
    load_some(90,17035,0);  	/* wise hermit with rep root 	*/
    load_some(70,17036,0);  	/* giant fire wolf 		*/
    load_some(50,17037,'2'); 	/* lichen 			*/
    load_some(65,17038,'B'); 	/* mud man			*/
    load_some(50,17039,0); 	/* blackness 			*/
    load_some(70,17040,0); 	/* rabid rat 			*/
    load_some(30,17041,'L');  	/* serpant 			*/
    load_some(30,17042,'L');  	/* octopus 			*/
    load_some(20,17033,'S'); 	/* giant eels 			*/
    load_some(250,17022,0); 	/* normal bats			*/
    load_some(45,17007,0); 	/* vampire bat 			*/
    load_some(30,17021,0);  	/* dream weaver girl		*/
    load_some(75,17009,0); 	/* Giant Centipedes 		*/
    load_some(100,17005,0); 	/* Cave Bears       		*/
    load_some(60,17000,0); 	/* Chimera			*/
    load_some(62,17001,0); 	/* Banshee			*/
    load_some(54,17002,0); 	/* Maelbreth			*/
    load_some(56,17003,0); 	/* Tornado Golem		*/
    load_some(58,17004,0); 	/* Sludge Mephit		*/
    load_some(160,17015,0); 	/* CaveWeed     		*/
    load_some( 58,17016,0); 	/* DarkSprite  			*/
    
    for(x=0;x<MAX_X;x++) /* crystal dragon */
	for(y=0;y<MAX_Y;y++)
	    for(z=1;z<MAX_Z;z++)
		if(grid[x][y][z]>MAX_ROOM-5001&&grid[x][y][z]<MAX_ROOM-1)
		    if(type[x][y][z]=='H'){
		    	mob=read_mobile(17034,REAL);
		    	char_to_room(mob,grid[x][y][z]);
		    }
    a=0;
    t=0;
    for(x=0;x<MAX_X;x++)
	for(y=0;y<MAX_Y;y++)
	    for(z=1;z<MAX_Z;z++){
		if(grid[x][y][z-1]<MAX_ROOM-5001||grid[x][y][z-1]>MAX_ROOM-1){
		    t++;
		    continue;
		}
		if(grid[x][y][z]<MAX_ROOM-5001||grid[x][y][z]>MAX_ROOM-1){
		    t++;
		    continue;
		}	
		if(grid[x][y][z]&&grid[x][y][z-1])
		    a++;
	    }
    sprintf(log_buf,"CATACOMBS: %d rooms have a room above and %d rooms are OU\n",a,t);
    fprintf(stderr,log_buf);
    for(t=0;t<17;t++){
	a=0;
	while(++a<50000){
	    x=number(0,MAX_X);
	    y=number(0,MAX_Y);
	    z=number(0,MAX_Z-2);
	    if(grid[x][y][z]&&grid[x][y][z+1]){
		b=0;
		if(grid[x][y][z+1]<MAX_ROOM-5001||grid[x][y][z+1]>MAX_ROOM-1)continue;
		if(grid[x][y][z]<MAX_ROOM-5001||grid[x][y][z]>MAX_ROOM-1)continue;
		if(type[x][y][z]=='V')continue;
		if(type[x][y][z+1]=='V')continue;
	        if(world[grid[x][y][z]]->sector_type==SECT_UNDERWATER)continue;
	        if(world[grid[x][y][z+1]]->sector_type==SECT_UNDERWATER)continue;
		for(mob=world[grid[x][y][z]]->people;mob;mob=mob->next_in_room){
		    if(mob->nr==17014)
			b=1;
		    if(mob->nr==17017)
			b=1;
		}
		for(mob=world[grid[x][y][z+1]]->people;mob;mob=mob->next_in_room){
		    if(mob->nr==17014)
			b=1;
		    if(mob->nr==17017)
			b=1;
		}
		if(!b){
		    mob=read_mobile(17014,REAL);
		    char_to_room(mob,grid[x][y][z]);
		    c++;
		    break;
		}
	    }
	}
	if(a>=50000)t=31;
    }
    fprintf(stderr,"CATACOMBS: %d CaveRays placed\n",c);
    c=0;
    for(t=0;t<17;t++){
	a=0;
	while(++a<50000){
	    x=number(0,MAX_X);
	    y=number(0,MAX_Y);
	    z=number(1,MAX_Z);
	    if(grid[x][y][z]&&grid[x][y][z-1]){
		b=0;
		if(grid[x][y][z-1]<MAX_ROOM-5001||grid[x][y][z-1]>MAX_ROOM-1)continue;
		if(grid[x][y][z]<MAX_ROOM-5001||grid[x][y][z]>MAX_ROOM-1)continue;
		if(type[x][y][z]=='V')continue;
		if(type[x][y][z-1]=='V')continue;
	        if(world[grid[x][y][z]]->sector_type==SECT_UNDERWATER)continue;
	        if(world[grid[x][y][z-1]]->sector_type==SECT_UNDERWATER)continue;
		for(mob=world[grid[x][y][z]]->people;mob;mob=mob->next_in_room){
		    if(mob->nr==17017)
			b=1;
		    if(mob->nr==17014)
			b=1;
		}
		for(mob=world[grid[x][y][z-1]]->people;mob;mob=mob->next_in_room){
		    if(mob->nr==17017)
			b=1;
		    if(mob->nr==17014)
			b=1;
		}
		if(!b){
		    mob=read_mobile(17017,REAL);
		    char_to_room(mob,grid[x][y][z]);
		    c++;
		    break;
		}
	    }
	}
	if(a>=50000)t=31;
    }
    fprintf(stderr,"CATACOMBS: %d Whirlpools placed\n",c);
    for(t=0;t<18;t++){/* Thought Slugs */
	x=number(4,MAX_X-5);
	y=number(4,MAX_Y-5);
	z=number(0,MAX_Z);
	for(a=-2;a<2;a++)
	    for(b=-2;b<2;b++)
		if(grid[x+a][y+b][z]){
            	    mob=read_mobile(17013,REAL);
            	    char_to_room(mob,grid[x+a][y+b][z]);
		}
	
    }
}

	/* used for thought slugs and mind reapers */
void communicating(struct char_data *ch, char *text)
{
struct char_data *mob=NULL,*next=NULL;

    if(IS_AFFECTED(ch,AFF_REP_ROOT))return;
    if (IS_HOVERING(ch)) return;
    for(mob=world[ch->in_room]->people;mob;mob=next){
	next=mob->next_in_room;
	if(mob->nr==17013){ /* thought Slug! */
	    send_to_char("YOU SCREAM IN AGONY as your thoughts are RIPPED from your mind!\n\r",ch);
	    act("$n SCREAMS IN AGONY as $s thoughts are RIPPED from $s mind!",TRUE,ch,0,0,TO_ROOM);
	    GET_HIT(ch) -= number(40,150);
	    if (GET_HIT(ch) < 1) {
		send_to_char("The extreme mental pressure causes fatal damage to your brain.\r\n", ch);
		act("$n collapses to the floor, blood oozing from their ears.",TRUE,ch,0,0,TO_ROOM);
		global_color=31;
		act("$n is DEAD!\r\n",TRUE,ch,0,0,TO_ROOM);
		send_to_char("You have been KILLED!\r\n\r\n", ch);
		global_color=0;
		raw_kill(ch,NULL);
	    }
/*
	    if(!mob->specials.fighting)
		act("$n FLINGS off the wall and attacks!",TRUE,mob,0,0,TO_ROOM);
		hit(mob,ch,0);
*/
	}	
    }
}

void cast_map_catacombs( byte level, struct char_data *ch, char *arg,int type, struct char_data *tar_ch, struct obj_data *tar_obj )
{
struct room_affect *ra=NULL;
int room, hours;
char flag=0;
struct affected_type af;
  
  switch (type) {
    case SPELL_TYPE_SPELL:
	if(!ch)return;
	room=ch->in_room;
        if(room<0||room>=MAX_ROOM)return;
        if(!world[room])return;  
	if(world[room]->zone!=198){
	    send_to_char("Try as you may, you cannot seem to map the catacombs without being there.\n\r",ch);
	    return;
	}
  	if ( IS_AFFECTED(ch,AFF_MAP_CATACOMBS) )
  	{
    	    act("Your purple aura is still strong.",TRUE,ch,0,0,TO_CHAR);
    	    return;  
  	}

	hours=GET_LEVEL(ch)*4;
	ra=world[room]->room_afs;
	while(ra){
	    if(ra->type==RA_MAP){
		if(ra->ch==ch){
		    ra->timer=hours;
		    flag=1;	    		    
		}
	    }
	    ra=ra->next;
	}
	if(!flag){
            CREATE(ra,struct room_affect,1);
            ra->type=RA_MAP;
            ra->timer=hours;
            ra->value=1;
            ra->text=str_dup(GET_NAME(ch));
            ra->ch=ch;
            ra->room=room;
            ra->next=world[room]->room_afs;
            world[room]->room_afs=ra;
	}
    	af.type      = SPELL_MAP_CATACOMBS;
    	af.duration  = GET_LEVEL(ch)*2;
    	af.modifier  = 0;
    	af.location  = APPLY_NONE;
    	af.bitvector = AFF_MAP_CATACOMBS;
    	affect_to_char(ch, &af);
	send_to_char("You focus and become ONE with the catacombs.....POOF\n\rA beaming bright purple aura hums around you and anchors to you.\n\r",ch);
    	act("$n focuses and becomes one with the catacombs...POOF!  A bright purple aura Hums around $m and achors to $s body.",TRUE,ch,0,0,TO_ROOM);
	       
        break;
    default :
        log_hd("##Serious screw-up in cast map catacombs!");   
        break; 
    }
}



/* this is the low level helping function to get descriptions
   it has a filename and a SLOT (room_type) so it knows where in
   descriptions array to put the allocated descriptions
   it gtes first room name, then description so array descriptions
   is filled with even names and odd descriptions */

void snarf_descriptions(int type, FILE *fl)
{
int count=0;
    fprintf(stderr,"X");
    while(!feof(fl)){
	descriptions[type][count]=fread_string(fl);
	if(strstr(descriptions[type][count],"$")){
	    descriptions[type][count]=my_free(descriptions[type][count]);
	    return;
	}
	fprintf(stderr,"<");
	count++;
	descriptions[type][count]=fread_string(fl);
	fprintf(stderr,">");
	count++;
    }
}

int pick_description(int type)
{
int count,picked;

    count=0;
    while(descriptions[type][count])count+=2;
    count--;
    picked=number(0,count);
    if(!(picked%2))picked++;  /*make sure odd number for des and not name */
    return(picked);
}

void get_descriptions(void)
{
FILE *fl;

    fprintf(stderr,"CATACOMB: Getting Pre-processed room descriptions.\n");
    if (!(fl=fopen("../catacomb/beach", "r"))){
                perror("cant open beach file");
                exit( 1 );
    }
    snarf_descriptions(BEACH_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/cavern", "r"))){
                perror("cant open cavern file");
                exit( 1 );
    }
    snarf_descriptions(CAVERN_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/entrcat", "r"))){
                perror("cant open entrcat file");
                exit( 1 );
    }
    snarf_descriptions(ENTRCAT_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/hole", "r"))){
                perror("cant open hole file");
                exit( 1 );
    }
    snarf_descriptions(HOLE_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/huge", "r"))){
                perror("cant open huge file");
                exit( 1 );
    }
    snarf_descriptions(HUGE_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/island", "r"))){
                perror("cant open island file");
                exit( 1 );
    }
    snarf_descriptions(ISLAND_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/lake", "r"))){
                perror("cant open lake file");
                exit( 1 );
    }
    snarf_descriptions(LAKE_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/stream", "r"))){
                perror("cant open stream file");
                exit( 1 );
    }
    snarf_descriptions(STREAM_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/vert", "r"))){
                perror("cant open vert file");
                exit( 1 );
    }
    snarf_descriptions(VERT_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/tunnel1", "r"))){
                perror("cant open tunnel1 file");
                exit( 1 );
    }
    snarf_descriptions(TUNNEL1_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/tunnel2", "r"))){
                perror("cant open tunnel2 file");
                exit( 1 );
    }
    snarf_descriptions(TUNNEL2_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/tunnel3", "r"))){
                perror("cant open tunnel3 file");
                exit( 1 );
    }
    snarf_descriptions(TUNNEL3_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/bend", "r"))){
                perror("cant open bend file");
                exit( 1 );
    }
    snarf_descriptions(BEND_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/chasm", "r"))){
                perror("cant open chasm file");
                exit( 1 );
    }
    snarf_descriptions(CHASM_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/painting", "r"))){
                perror("cant open painting file");
                exit( 1 );
    }
    snarf_descriptions(CAVE_PAINTING_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/waterfall", "r"))){
                perror("cant open waterfall file");
                exit( 1 );
    }
    snarf_descriptions(WATERFALL_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/cavemana", "r"))){
                perror("cant open cavemana file");
                exit( 1 );
    }
    snarf_descriptions(CAVE_MANA_DES,fl);
    fclose(fl);

    if (!(fl=fopen("../catacomb/cavecorrel", "r"))){
                perror("cant open cavecorrel file");
                exit( 1 );
    }
    snarf_descriptions(CAVE_CORREL_DES,fl);
    fclose(fl);

    fprintf(stderr,"DONE\n");
}

void catacomb_makedoor(int room, int to_room, int dir)
{
   CREATE(world[room]->dir_option[dir],struct room_direction_data, 1); 
   world[room]->dir_option[dir]->general_description = 0; 
   world[room]->dir_option[dir]->keyword = 0; 
   world[room]->dir_option[dir]->exit = str_dup(" "); 
   world[room]->dir_option[dir]->entrance = str_dup(" ");
   world[room]->dir_option[dir]->exit_info = 0;
   world[room]->dir_option[dir]->key=-1;
   world[room]->dir_option[dir]->to_room=to_room;
}
	
void make_room(int room, int x, int y , int z)
{
int tmp;
    CREATE(world[room], struct room_data, 1); 
    number_of_rooms++; 
    world[room]->number = room; 
	world[room]->stpFreight=NULL;
    world[room]->name=NULL;
    world[room]->description=NULL;
    world[room]->zone = 198;
    world[room]->room_flags=9;
    world[room]->sector_type=0;
    world[room]->funct = 0; 
    world[room]->contents = 0; 
    world[room]->people = 0; 
    world[room]->light = 0;
    world[room]->ex_description = 0; 
    world[room]->extra_flags=0;
    world[room]->class_restriction=0;
    world[room]->level_restriction=0;
    world[room]->align_restriction=0;
    world[room]->mount_restriction=z;
    world[room]->move_mod=0;
	world[room]->holox=0;
	world[room]->holoy=0;
    world[room]->pressure_mod=x;
    world[room]->temperature_mod=y;
    for (tmp = 0; tmp <= 5; tmp++)
		world[room]->dir_option[tmp] = 0; 
	rooms[level]++;
}

int get_next_number(void)
{
int x;
    x=MAX_ROOM-1;
    while(world[x])x--;
    return(x);
}

void weight_room(int x, int y, int z)
{
char w;
	
	if(type[x][y][z])return;
	type[x][y][z]=' ';	
	w=0;
	if(x)
		if(grid[x-1][y][z])
			w++;
	if(x<MAX_X-1)
		if(grid[x+1][y][z])
			w++;
	if(y)
		if(grid[x][y-1][z])
			w++;
	if(y<MAX_Y-1)
		if(grid[x][y+1][z])
			w++;
	type[x][y][z]='0'+w;
	
}

void analyze_room1(int x, int y, int z)
{
char w;

    if(type[x][y][z]=='4'){
    	type[x][y][z]='L';
		if(x&&type[x-1][y][z]=='3')
			type[x-1][y][z]='B';
        if(x<MAX_X-1&&type[x+1][y][z]=='3')
            type[x+1][y][z]='B';
        if(y&&type[x][y-1][z]=='3')
            type[x][y-1][z]='B';
        if(y<MAX_Y-1&&type[x][y+1][z]=='3')
            type[x][y+1][z]='B';
	}    
	if(type[x][y][z]=='3'){
		w=0;
		if(x&&(type[x-1][y][z]=='3'||type[x-1][y][z]=='C'))
			w++;
		if(x<MAX_X-1&&(type[x+1][y][z]=='3'||type[x+1][y][z]=='C'))
			w++;
		if(y&&(type[x][y-1][z]=='3'||type[x][y-1][z]=='C'))
			w++;
		if(y<MAX_Y-1&&(type[x][y+1][z]=='3'||type[x][y+1][z]=='C'))
			w++;
		if(w==3)
			type[x][y][z]='C';
	}
}

void analyze_room2(int x, int y, int z)
{
char w;

	if(type[x][y][z]=='C'){
		w=0;
		if(x&&type[x-1][y][z]=='C')
			w++;
		if(x<MAX_X-1&&type[x+1][y][z]=='C')
			w++;
		if(y&&type[x][y-1][z]=='C')
			w++;
		if(y<MAX_Y-1&&type[x][y+1][z]=='C')
			w++;
		if(w==3)
			type[x][y][z]='H';
	}
	if(type[x][y][z]=='L'){
		w=0;
		if(x&&type[x-1][y][z]=='L')
			w++;
		if(x<MAX_X-1&&type[x+1][y][z]=='L')
			w++;
		if(y&&type[x][y-1][z]=='L')
			w++;
		if(y<MAX_Y-1&&type[x][y+1][z]=='L')
			w++;
		if(w==4)
			type[x][y][z]='I';
	}
}

void analyze_room3(void)
{
int x,y,z,c;
int x1,y1,x2,y2;
int s;

	for(z=0;z<MAX_Z;z++){
	    for(y=0;y<MAX_Y;y++){
	    	for(x=0;x<MAX_X-8;x++){
				if(type[x][y][z]=='2'){
					c=0;
					y1=y;
					x1=x;
					y2=y;
					x2=x;
					while(x2<MAX_X-1&&type[++x2][y][z]=='2')
						c++;
					if(type[x2][y][z]!='2')x2--;
					if(c>=8){
						type[x1][y][z]='h';
						type[x2][y][z]='h';
						for(s=x1+1;s<x2;s++)
							type[s][y][z]='S';
					}
				}					    	
	    	}
	    }		
	    for(x=0;x<MAX_X;x++){
	    	for(y=0;y<MAX_Y-8;y++){
				if(type[x][y][z]=='2'){
					c=0;
					y1=y;
					x1=x;
					y2=y;
					x2=x;
					while(y2<MAX_Y-1&&type[x][++y2][z]=='2')
						c++;
					if(type[x][y2][z]!='2')y2--;
					if(c>=8){
						type[x][y1][z]='h';
						type[x][y2][z]='h';
						for(s=y1+1;s<y2;s++)
							type[x][s][z]='S';
					}
				}					    	
	    	}
	    }		
	}
}

void analyze_room4(void)
{
int x,y,z,c;

    for(z=0;z<MAX_Z;z++){
	for(y=1;y<MAX_Y-1;y++){
	    for(x=1;x<MAX_X-1;x++){
		if(type[x][y][z]=='2'){
		    c=0;
		    if(grid[x-1][y][z]&&grid[x+1][y][z])
			c=1;
		    if(grid[x][y-1][z]&&grid[x][y+1][z])
			c=1;
		    if(!c){
		        type[x][y][z]='b';
		    }else{
			if(number(1,100)<5){
			    type[x][y][z]='c';
			}
		    }
		}
	    }
	}
    }
    c=0;
    while(c<50){
	x=number(0,MAX_X);
	y=number(0,MAX_Y);
	z=number(0,MAX_Z);
	if(type[x][y][z]=='2'&&grid[x][y][z]&&grid[x][y][z]>MAX_ROOM-5000&&grid[x][y][z]<MAX_ROOM){
	    c++;
	    type[x][y][z]='P';
	}
    }
    c=0;
    while(c<70){
	x=number(0,MAX_X);
	y=number(0,MAX_Y);
	z=number(0,MAX_Z);
	if(type[x][y][z]=='2'&&grid[x][y][z]&&grid[x][y][z]>MAX_ROOM-5000&&grid[x][y][z]<MAX_ROOM){
	    c++;
	    type[x][y][z]='M';
	}
    }
    c=0;
    while(c<50){
	x=number(0,MAX_X);
	y=number(0,MAX_Y);
	z=number(0,MAX_Z);
	if(type[x][y][z]=='2'&&grid[x][y][z]&&grid[x][y][z]>MAX_ROOM-5000&&grid[x][y][z]<MAX_ROOM){
	    c++;
	    type[x][y][z]='R';
	}
    }

}


void make_path(int x1, int y1, int x2, int y2)
{
int x,y;

    while(1){
		x=0;y=0;
		if(x1==x2&&y1==y2)return;
		if(x1!=x2&&y1!=y2){
			if(number(1,100)<50){
				x=1;
			}else{
				y=1;
			}
		}else{
			if(x1!=x2){
				x=1;
			}else{
				y=1;
			}	
	    }
	    if(x){
		    if(x1==x2)
		    	continue;
		    if(x1<x2)
		    	x1++;
		    else
		    	x1--;	    
	    }else{
		    if(y1==y2)
		    	continue;
		    if(y1<y2)
		    	y1++;
		    else
		    	y1--;	    
	    }
		if(grid[x1][y1][level])
			return;
		grid[x1][y1][level]=get_next_number();
		make_room(grid[x1][y1][level],x1,y1,level);
    }
}

void cata_connect(int a, int b, int d)
{
    if(!a||!b)return; /* make sure both rooms exist */
    if(world[a]->dir_option[d])return;
    catacomb_makedoor(a, b, d);
}

void edit_details(void)
{
int x,y,z;

    for(z=0;z<MAX_Z;z++)
      	for(x=0;x<MAX_X;x++)
      	    for(y=0;y<MAX_Y;y++){
		if(type[x][y][z]=='S')
		    world[grid[x][y][z]]->sector_type=SECT_UNDERWATER;
		if(type[x][y][z]=='L')
		    world[grid[x][y][z]]->sector_type=SECT_WATER_NOSWIM;
	    }
}

void put_descriptions(void)
{
int x,y,z;

    for(z=0;z<MAX_Z;z++)
      	for(x=0;x<MAX_X;x++)
      	    for(y=0;y<MAX_Y;y++){
	        if(grid[x][y][z])
		switch(type[x][y][z]){
		    case 'B':
			world[grid[x][y][z]]->name=descriptions[BEACH_DES][0];
			world[grid[x][y][z]]->description=descriptions[BEACH_DES][pick_description(BEACH_DES)];
			break;
		    case 'C':
			world[grid[x][y][z]]->name=descriptions[CAVERN_DES][0];
			world[grid[x][y][z]]->description=descriptions[CAVERN_DES][pick_description(CAVERN_DES)];
			break;
		    case 'H':
			world[grid[x][y][z]]->name=descriptions[HUGE_DES][0];
			world[grid[x][y][z]]->description=descriptions[HUGE_DES][pick_description(HUGE_DES)];
			break;
		    case 'I':
			world[grid[x][y][z]]->name=descriptions[ISLAND_DES][0];
			world[grid[x][y][z]]->description=descriptions[ISLAND_DES][pick_description(ISLAND_DES)];
			break;
		    case 'L':
			world[grid[x][y][z]]->name=descriptions[LAKE_DES][0];
			world[grid[x][y][z]]->description=descriptions[LAKE_DES][pick_description(LAKE_DES)];
			break;
		    case 'S':
			world[grid[x][y][z]]->name=descriptions[STREAM_DES][0];
			world[grid[x][y][z]]->description=descriptions[STREAM_DES][pick_description(STREAM_DES)];
			break;
		    case 'V':
			world[grid[x][y][z]]->name=descriptions[VERT_DES][0];
			world[grid[x][y][z]]->description=descriptions[VERT_DES][pick_description(VERT_DES)];
			break;
		    case 'h':
			world[grid[x][y][z]]->name=descriptions[HOLE_DES][0];
			world[grid[x][y][z]]->description=descriptions[HOLE_DES][pick_description(HOLE_DES)];
			break;
		    case '1':
			world[grid[x][y][z]]->name=descriptions[TUNNEL1_DES][0];
			world[grid[x][y][z]]->description=descriptions[TUNNEL1_DES][pick_description(TUNNEL1_DES)];
			break;
		    case '2':
			world[grid[x][y][z]]->name=descriptions[TUNNEL2_DES][0];
			world[grid[x][y][z]]->description=descriptions[TUNNEL2_DES][pick_description(TUNNEL2_DES)];
			break;
		    case '3':
			world[grid[x][y][z]]->name=descriptions[TUNNEL3_DES][0];
			world[grid[x][y][z]]->description=descriptions[TUNNEL3_DES][pick_description(TUNNEL3_DES)];
			break;
		    case 'b':
			world[grid[x][y][z]]->name=descriptions[BEND_DES][0];
			world[grid[x][y][z]]->description=descriptions[BEND_DES][pick_description(BEND_DES)];
			break;
		    case 'c':
			world[grid[x][y][z]]->name=descriptions[CHASM_DES][0];
			world[grid[x][y][z]]->description=descriptions[CHASM_DES][pick_description(CHASM_DES)];
			break;
		    case 'P':
			world[grid[x][y][z]]->name=descriptions[CAVE_PAINTING_DES][0];
			world[grid[x][y][z]]->description=descriptions[CAVE_PAINTING_DES][pick_description(CAVE_PAINTING_DES)];
		        world[grid[x][y][z]]->funct=cave_paintings;
			break;
		    case 'W':
			world[grid[x][y][z]]->name=descriptions[WATERFALL_DES][0];
			world[grid[x][y][z]]->description=descriptions[WATERFALL_DES][pick_description(WATERFALL_DES)];
			break;
		    case 'M':
			world[grid[x][y][z]]->name=descriptions[CAVE_MANA_DES][0];
			world[grid[x][y][z]]->description=descriptions[CAVE_MANA_DES][pick_description(CAVE_MANA_DES)];
			world[grid[x][y][z]]->move_mod=2;/*flags mana gain*/
			break;
		    case 'R':
			world[grid[x][y][z]]->name=descriptions[CAVE_CORREL_DES][0];
			world[grid[x][y][z]]->description=descriptions[CAVE_CORREL_DES][pick_description(CAVE_CORREL_DES)];
		        world[grid[x][y][z]]->funct=cave_correl;
			break;
		    default:
			sprintf(log_buf,"CATACOMBS:  UNKNOWN ROOM TYPE!! type[%d][%d][%d]=%c",x,y,z,type[x][y][z]);
			log_hd(log_buf);
		}	
	    }
}	

    /* makes the exit connections from to all rooms...*/
void make_connections(void)
{
int x,y,z;
int tries;

    for(x=0;x<MAX_X;x++){
      	for(y=0;y<MAX_Y;y++){
      	    for(z=0;z<MAX_Z;z++){
		if(x<MAX_X-1){
		    if(type[x][y][z]=='h'&&type[x+1][y][z]=='S')
		        cata_connect(grid[x][y][z],grid[x+1][y][z],5);
		    else if(type[x][y][z]=='S'&&type[x+1][y][z]=='h')
		        cata_connect(grid[x][y][z],grid[x+1][y][z],4);
		    else
		        cata_connect(grid[x][y][z],grid[x+1][y][z],1);
		}
		if(x>0){
		    if(type[x][y][z]=='h'&&type[x-1][y][z]=='S')
		        cata_connect(grid[x][y][z],grid[x-1][y][z],5);
		    else if(type[x][y][z]=='S'&&type[x-1][y][z]=='h')
		        cata_connect(grid[x][y][z],grid[x-1][y][z],4);
		    else
		        cata_connect(grid[x][y][z],grid[x-1][y][z],3);
		}
		if(y<MAX_Y-1){
		    if(type[x][y][z]=='h'&&type[x][y+1][z]=='S')
		        cata_connect(grid[x][y][z],grid[x][y+1][z],5);
		    else if(type[x][y][z]=='S'&&type[x][y+1][z]=='h')
		        cata_connect(grid[x][y][z],grid[x][y+1][z],4);
		    else
		        cata_connect(grid[x][y][z],grid[x][y+1][z],2);
		}
		if(y>0){
		    if(type[x][y][z]=='h'&&type[x][y-1][z]=='S')
		        cata_connect(grid[x][y][z],grid[x][y-1][z],5);
		    else if(type[x][y][z]=='S'&&type[x][y-1][z]=='h')
		        cata_connect(grid[x][y][z],grid[x][y-1][z],4);
		    else
		        cata_connect(grid[x][y][z],grid[x][y-1][z],0);
		}
	    }
	}
    }
    for(z=0;z<MAX_Z-1;z++){
	tries=0;
	while(tries<15000){
	    tries++;
	    x=number(0,MAX_X-1);
	    y=number(0,MAX_Y-1);
	    if(grid[x][y][z]&&grid[x][y][z+1]){
		type[x][y][z]='V';
		type[x][y][z+1]='V';
		cata_connect(grid[x][y][z],grid[x][y][z+1],5);
		cata_connect(grid[x][y][z+1],grid[x][y][z],4);
	        break;
	    }
	}
	if(tries>=15000){
	    log_hd("## GAVE UP CONENCTING VERTICALLY IN CATACOMBS!!!!");
	}
    }
}

void build_catacombs(void)
{
FILE *buildit;
char builditflag[200];
int x,y,z,a,b,c;

	gate_room=0;
	gate_dir=0; 
	sprintf(builditflag,"catacomb.%d",port);
	if((buildit=fopen(builditflag,"r"))!=NULL)
	{
		catacombs_loaded=TRUE;
		fclose(buildit);
	}else{
		sprintf(log_buf,"Not building catacombs.");
		log_hd(log_buf);
		catacombs_loaded=FALSE;
		return;
	}
   entrance_room=get_next_number();
   sprintf(log_buf,"CATACOMB Entrance room is #%d",entrance_room);
   log_hd(log_buf); 
   get_descriptions();
   moveables=NULL;
   for(x=0;x<MAX_Z;x++)
    	rooms[x]=0;
    	
    for(x=0;x<MAX_X;x++)
      	for(y=0;y<MAX_Y;y++)
      		for(z=0;z<MAX_Z;z++)
      			grid[x][y][z]=0;
      			
	a=number(10,MAX_X-10);
	b=number(10,MAX_Y-10);
    for(level=0;level<MAX_Z;level++){
		sprintf(log_buf,"CATACOMB: Generating Map Skeleton Level %d\n",level);
		fprintf(stderr,log_buf);
		type[a][b][level]='W';
		grid[a][b][level]=get_next_number();
		make_room(grid[a][b][level],a,b,level);
		grid[a][b-1][level]=get_next_number();
		make_room(grid[a][b-1][level],a,b-1,level);
		grid[a+1][b][level]=get_next_number();
		make_room(grid[a+1][b][level],a+1,b,level);
        	grid[a][b+1][level]=get_next_number();
        	make_room(grid[a][b+1][level],a,b+1,level);
        	grid[a-1][b][level]=get_next_number();
        	make_room(grid[a-1][b][level],a-1,b,level);
		make_path(a,b-1,0,0);
		make_path(a-1,b,0,MAX_Y-1);						      
		make_path(a+1,b,MAX_X-1,0);
		make_path(a,b+1,MAX_X-1,MAX_Y-1);
		while(rooms[level]<ROOMS_PER_LEVEL){
			c=1;
			while(c){
				a=number(0,MAX_X);
				b=number(0,MAX_Y);
				if(grid[a][b][level])c=0;
			}
			make_path(a,b,number(0,MAX_X-1),number(0,MAX_Y-1));
		}
		c=1;
		while(c){ /*make sure next level has room ontop*/
			a=number(5,MAX_X-5);
			b=number(5,MAX_Y-5);
			if(grid[a][b][level])c=0;
		}
	}
	fprintf(stderr,"CATACOMB: Weighting rooms.....\n");
	for(level=0;level<MAX_Z;level++)
		for(x=0;x<MAX_X;x++)
			for(y=0;y<MAX_Y;y++)
				weight_room(x,y,level);
	fprintf(stderr,"CATACOMB: Analyzing Rooms Pass 1...Flagging lakes & caverns\n");
	for(level=0;level<MAX_Z;level++)
		for(x=0;x<MAX_X;x++)
			for(y=0;y<MAX_Y;y++)
				analyze_room1(x,y,level);
	fprintf(stderr,"CATACOMB: Analyzing Rooms Pass 2....Possible Islands and Huge Areas\n");
	for(level=0;level<MAX_Z;level++)
		for(x=0;x<MAX_X;x++)
			for(y=0;y<MAX_Y;y++)
				analyze_room2(x,y,level);
	fprintf(stderr,"CATACOMB: Analyzing Rooms Pass 3....Optimizing for underwater Tunnels\n");
	analyze_room3();
	fprintf(stderr,"CATACOMB: Analyzing Rooms Pass 4....Re-Morphing Tunnels\n");
	analyze_room4();

	fprintf(stderr,"CATACOMB: Making room Connections....\n");
	make_connections();
	fprintf(stderr,"CATACOMB: Designing and placing descriptions....\n");
	put_descriptions();
	fprintf(stderr,"CATACOMB: Editing and tweaking details....\n");
	edit_details();
	populate_catacombs();
	link_catacombs();
	iHighRoom=get_next_number();
	fprintf(stderr,"
TEMPLATE CODES FOR CATACOMB MAP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
L=Lake
B=Beach to Lake
C=Cavern
V=VERTICAL up/down across LEVELS
H=HUGE Cavern
I=ISLAND in the middle of a lake
S=Stream in tunnel you swim through UNDERWATER
h=Hole where you enter/exit STREAM(up/down)
E=Entrance to the catacombs
M=Cave Manatite room
R=Cave Correl room
b=Bend in the Tunnal
W=Waterfall
P=Cave Paintings

");
	for(level=0;level<MAX_Z;level++){
		sprintf(log_buf,"Built Catacomb LEVEL %d  %dx%d %d rooms\n",level,MAX_X,MAX_Y,rooms[level]);
		fprintf(stderr,log_buf);
		for(y=0;y<MAX_X+4;y++)
			fprintf(stderr, "@");
		fprintf(stderr, "\n");
		for(y=0;y<MAX_X+4;y++)
			fprintf(stderr, "@");
		fprintf(stderr, "\n");
		for(y=0;y<MAX_Y;y++){
			fprintf(stderr, "@@");
		    for(x=0;x<MAX_X;x++){
		    	if(grid[x][y][level])
		    		fprintf(stderr, "%c",type[x][y][level]);
		    	else
		    		fprintf(stderr, " ");
		    }
		    fprintf(stderr, "@@\n");
		}		      		
		for(y=0;y<MAX_X+4;y++)
			fprintf(stderr, "@");
		fprintf(stderr, "\n");
		for(y=0;y<MAX_X+4;y++)
			fprintf(stderr, "@");
		fprintf(stderr, "\n");
    }
}








