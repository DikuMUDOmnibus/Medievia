/***************************************************************************
*			 MEDIEVIA CyberSpace Code and Data files 	   *
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
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

#define MED_TOMBSTONES 67
/*#define TREL_TOMBSTONES 20  There are only 19 tombstones */
#define TREL_TOMBSTONES 19
#define MED_SANCTUM 1729
#define TREL_SANCTUM 4835
#define MED_POTTER 616
#define TREL_POTTER 4834
#define MAX_NECROS 1
#define MIN_UNDEAD_PER_LEVEL  2
#define MIN_UNDEAD_PER_PK    10
#define MIN_UNDEAD_PER_DEATH  0
#define MIN_UNDEAD            10
#define STATE(d) ((d)->connected)

void load_necros(void);
void place_necros(int continent, int to_load);
void load_necromancer(int room, struct char_data *victmob);
void spiritize(struct char_data *ch, struct char_data *killer);
void undeadify(struct char_data *ch);
void reanimate(struct descriptor_data *d, int cmd);
void raw_unkill(struct char_data *ch, struct char_data *attacker);
void tell_clan(int clan, char *argument);
void put_all_in_inv(struct char_data *ch);
int med_tombstones_since_last_save=0;
int trel_tombstones_since_last_save=0;

/*   external vars  */
extern struct char_data *character_list;
extern struct char_data *combat_list;
extern struct char_data *mobs[MAX_MOB];
extern struct obj_data *objs[MAX_OBJ];
struct index_data *mob_index;
void make_corpse(struct char_data *ch, char keep_stuff);
extern void save_rooms(struct char_data *ch, int zone);
extern int number_of_players();
extern unsigned long int connect_count;
extern int number_of_rooms;
extern int number_of_zones;
extern char global_color;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern int top_of_world;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern put_obj_in_store(struct obj_data *obj, struct char_data *ch, FILE *fpsave);
extern bool in_a_shop(struct char_data *ch);
extern bool is_shop(int room);
extern int top_of_world;
extern int top_of_zone_table;
extern struct global_clan_info_struct global_clan_info;
extern struct zone_data *zone_table;
extern char free_error[100];
extern void make_ashes(struct char_data *ch, struct char_data *victim);

/*
void write_tombstone(struct char_data *ch, struct char_data *killer){
    extern char *pc_class_types[];
    int trel_tombstones[]= {
	4628,4629,4630,4631,4632,4633,4634,4635,4636,4637,4638,4639,
	4641,4642,4643,4644,4645,4647,4648
    };

    int med_tombstones[]= {
	1015,1016,1018,1019,1020,1021,1022,1024,
	1029,1030,1031,1032,1033,1034,1035,1036,1037,1038,
	1043,1044,1045,1046,1047,1048,1049,1051,1052,
	1058,1059,1060,1061,1062,1063,1064,1065,1066,1067,
	1076,1077,1078,1079,1080,1081,1082,1083,1084,1085,
	1090,1091,1092,1093,1094,1095,1096,1097,1098,1099,
	1105,1106,1107,1108,1109,1110,1111,1112,1113,1114
    };

    int room;
    char class[MAX_STRING_LENGTH];
    char temp[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];
    char buf5[MAX_STRING_LENGTH];
    char *suf;
    int  day;
    int  x;
    int  pad;
    extern struct time_info_data time_info;
    extern const char *month_name[];
    struct extra_descr_data *extra_descr;


    if ( IS_NPC(ch) || (GET_LEVEL(ch) < 12) || (GET_LEVEL(ch) > 30) )
	return;

    if ( GET_LEVEL(ch) < 12 ) {
	if(GET_CONTINENT(ch)==TRELLOR)
	    ch->internal_use = (world[TREL_POTTER]) ? TREL_POTTER:1;
	else
	    ch->internal_use = (world[MED_POTTER]) ? MED_POTTER:1;
	return;
    }

    if( (IS_SET(world[ch->in_room]->room_flags,NEUTRAL)) 
	 && killer && !IS_NPC(killer) )
	return;

    day = time_info.day + 1;  

    if (day == 1)
        suf = "st";
    else if (day == 2)
        suf = "nd";
    else if (day == 3)
        suf = "rd";
    else if (day < 20)
        suf = "th";
    else if ((day % 10) == 1)
        suf = "st";
    else if ((day % 10) == 2)
        suf = "nd";
    else if ((day % 10) == 3)
        suf = "rd";
    else
        suf = "th";

    sprintf(temp,"Here Rests the Soul of %s %s", GET_NAME(ch), ch->player.title);
    pad=(int)((78-strlen(temp))/2);     
    for(x=0;x<pad;x++) 
    	buf1[x]=' '; 
    sprintf(buf1+pad,"%s\n\r",temp);


    sprinttype(ch->player.class,pc_class_types,class);
    if(ch->specials.clan &&  &global_clan_info.clan_name[x][0]) {
	for(x=0;x<MAX_CLANS;x++)
	    if(global_clan_info.clan[x]==ch->specials.clan)
		break;
	sprintf(temp,"A Level %d %s and Member of %s",
		GET_LEVEL(ch),
		class,
		&global_clan_info.clan_name[x][0]);
     } else {
	sprintf(temp,"A Level %d %s",GET_LEVEL(ch),class);
    }
    pad=(int)((78-strlen(temp))/2);     
    for(x=0;x<pad;x++) 
        buf2[x]=' '; 
    sprintf(buf2+pad,"%s\n\r",temp);


sprintf(temp,"Died on the %d%s Day of the %s, Year %d",
	day,
	suf,
	month_name[time_info.month],
	time_info.year);
pad=(int)((78-strlen(temp))/2);     
for(x=0;x<pad;x++) 
    buf3[x]=' '; 
sprintf(buf3+pad,"%s\n\r",temp);


if (!killer || (ch == killer) ) {
    sprintf(temp,"Under Mysterious Circumstances");
} else if (IS_NPC(killer)) {
    sprintf(temp,"By the Hand of %s", killer->player.short_descr);
} else 
    sprintf(temp,"By the Hand of %s %s",
    		GET_NAME(killer),
    		killer->player.title);
pad=(int)((78-strlen(temp))/2);     
for(x=0;x<pad;x++) 
    buf4[x]=' '; 
sprintf(buf4+pad,"%s\n\r",temp);

sprintf(temp,"In %s",zone_table[world[ch->in_room]->zone].name);
pad=(int)((78-strlen(temp))/2);     
for(x=0;x<pad;x++) 
    buf5[x]=' '; 
sprintf(buf5+pad,"%s\n\r",temp);

if(GET_CONTINENT(ch) == TRELLOR)
	room=trel_tombstones[number(0,TREL_TOMBSTONES-1)];
else
	room=med_tombstones[number(0,MED_TOMBSTONES-1)];

ch->internal_use = room;
if ((room<0) || (room>MAX_ROOM))
	return;
if (!world[room])
	{
	sprintf(log_buf,"##Room %d does not exist. No tombstone written.",room);
	log_hd(log_buf);
	return;
	}
send_to_room("A worker arrives to erect a new tombstone.",room);
extra_descr=world[room]->ex_description;
while(extra_descr 
		&& extra_descr->keyword 
		&& strcmp("tombstone",extra_descr->keyword)
		)
	extra_descr=extra_descr->next;
if(!extra_descr->keyword)return;
if(strcmp("tombstone",extra_descr->keyword))
	{
	sprintf(log_buf,"##No tombstone in room %d.",room);
	log_hd(log_buf);
	return;
	}
strcpy(free_error,"extra_descr->description 1 in death.c");
extra_descr->description = my_free(extra_descr->description);
sprintf(log_buf,"\n\r%s%s%s%s%s",buf1,buf2,buf3,buf4,buf5);
extra_descr->description = str_dup(log_buf);
if(GET_CONTINENT(ch) == TRELLOR)
	{
	med_tombstones_since_last_save++;
	if (med_tombstones_since_last_save > 15)
		{
		save_rooms(NULL,10);
		med_tombstones_since_last_save = 0;
		}
	}
else
	{
	trel_tombstones_since_last_save++;
	if (trel_tombstones_since_last_save > 15)
		{
		save_rooms(NULL,72);
		trel_tombstones_since_last_save = 0;
		}
	}
}*/

int undead_corpse(struct char_data *ch, int cmd, char *arg)
{
if( (cmd)  || (ch->specials.fighting) )
	return(0);

switch(number(0,200))
	{
	case 1:
		do_emote(ch,"lets out a long, agonized moan.",0);
		return(1);
		break;
	case 2:
		if (GET_POS(ch) > POSITION_RESTING)
			{
			do_emote(ch,"trips, falling under the weight of its burden.",0);
			GET_POS(ch)=POSITION_RESTING;
			return(1);
			}
		else
			return(0);
		break;
	case 3:
		do_emote(ch, "emits a pitiful wail, sending shivers up your spine.", 0);
                return(1);
		break;
	case 4:
		do_emote(ch,"removes both arms and switches them.",0);
		return(1);
		break;
	case 5:
		do_emote(ch,"has a sudden muscle spasm and its head rotates 360 degrees.",0);
		return(1);
		break;
	case 6:
		do_emote(ch,"pulls out a section of intestine and eats it.",0);
		return(1);
		break;
	case 7:
		do_emote(ch,"pops out an eyeball and shoves it through an ear.",0);
		return(1);
		break;
	case 8:
		do_emote(ch,"switches a leg with an arm.",0);
		return(1);
		break;
	case 9:
		do_emote(ch,"picks some oozey green stuff from an ear.",0);
		return(1);
		break;
	case 10:
		do_emote(ch,"drips ooze from its nose, then sniffs it back up.",0);
		return(1);
		break;
	default:
		return(0);
	}

}


void perform_hovering(void)
{
struct descriptor_data *d=NULL, *d_next=NULL;

/* stay hovering until the next HOVERING pulse */
for ( d = descriptor_list ; d ; d = d_next )
	{
	d_next = d->next;
	if (!d->character || (STATE(d) != CON_HOVERING))
		continue;
	else if (d->character->specials.timer > 1)
		undeadify(d->character);
	else if (d->character->specials.timer > 0)
		{
                send_to_char(
"The necromancer returns in a puff of smoke and pokes your body with the\r
end of his staff as if testing it.  Pleased with the results, he opens\r
a small blue bottle and proceeds with a chant that makes your hair stand\r
on end.  The corpse twists and turns and rises off the ground. Finally\r
with a woosh something is sucked from the body into the bottle. The corpse\r
falls back down to the ground in a heap.  Without your spirit, you are\r
doomed to roam the world undead.\n\r",d->character);
                act(
"The necromancer returns and performs a ritual over the corpse.  It levitates\r
and writhes violently for a few moments until you hear a whoosh and the clink\r
of the old mage sealing $n's spirit into a small blue bottle."
,FALSE,d->character,0,NULL,TO_ROOM);
                }
        else
                {
                send_to_char(
"A figure in a black robe arrives from a puff of smoke nodding his head\r
approvingly at the fresh corpse. Getting straight to work, he traces a\r
circle around the body with his staff. Removing a handful of dried scarabs\r
from a fold in his cloak, he places one over each eye and crumbles the rest,\r
sprinkling them over the body.  Standing outside the circle, he touches the\r
staff to your forehead uttering the words \'Drixnil itum Ptlm\'.  The corpse,\r
the staff, and everything within the circle are engulfed in a strange green\r
flame which he douses after a few moments with some water from a blue bottle.\r
The water has a strange effect on the corpse.  Suddenly the necromancer is\r
gone as quickly as he arrived, leaving some time for the unholy water to have\r
its effect.\n\r",d->character);
                act(
"A figure in a black robe arrives from a puff of smoke. He prepares the\r
corpse, and performs a short ritual, engulfing it in a circle of flame. He\r
douses the flame with a vial of unholy water, then leaves as quickly as he\r
arrived." ,FALSE,d->character,0,NULL,TO_ROOM);
		}
	d->character->specials.timer++;
	}
}



void perform_undead(void)
{
extern struct descriptor_data *descriptor_list;
struct descriptor_data *d=NULL, *d_next=NULL;
int crypt=1;
struct obj_data *urn=NULL, *spirit=NULL, *obj=NULL;

/* load necromancers */
load_necros();

/* empty urns from the crypt */
if (world[1729])
	crypt=1729;

/*
obj = world[crypt]->contents;
while(obj && obj->next_content)
	{
	next = obj->next_content;
	if(obj->next_content->item_number == 9811)
		extract_obj(obj->next_content);
	if(obj->next_content == next)
		obj = obj->next_content;
	}
if((obj) && (obj->item_number==9811))
	extract_obj(obj);
*/

/* delete old spirit bottle from the crypt and make new ones later */
obj = world[crypt]->contents;
while(obj)
	{
	while((obj->next_content) && (obj->next_content->item_number == 9811))
		extract_obj(obj->next_content);
	obj = obj->next_content;
	}
if((world[crypt]->contents) && (world[crypt]->contents->item_number==9811))
	extract_obj(world[crypt]->contents);



/* Make a new urn for undead people.
 */
for ( d = descriptor_list ; d ; d = d_next )
	{
	d_next = d->next;
	if (STATE(d)==CON_UNDEAD)
       {
       if ( (ORIGINAL(d->character)->specials.death_timer < -60)
        &&  (!d->character->specials.fighting)
          )
            {
            sprintf(log_buf,"Code reanimating %s.",
                GET_NAME(d->character));
            log_hd(log_buf);
            reanimate(d,0);
            }
       else
	   {
		urn = read_object(9811, REAL);
		sprintf(log_buf,
		 "A sapphire bottle sits in a niche holding %s's spirit",
		    GET_NAME(d->character)
		    );
		if(objs[urn->item_number]->description!=urn->description)
		    urn->description = my_free(urn->description);
		urn->description=str_dup(log_buf);
		sprintf(log_buf,"%s bottle sapphire",GET_NAME(d->character));
		if(objs[urn->item_number]->name!=urn->name)
		    urn->name = my_free(urn->name);
		urn->name = str_dup(log_buf);
		SET_BIT(urn->obj_flags.value[1], CONT_CLOSEABLE);
		SET_BIT(urn->obj_flags.value[1], CONT_CLOSED);
		spirit=read_object(2999,REAL);
		sprintf( log_buf, 
		  "The glow of %s's spirit softly illuminates the bottle's interior.",
		  GET_NAME(d->character));
		if(objs[2999]->short_description!=spirit->short_description)
		    spirit->short_description = my_free(spirit->short_description);
		spirit->short_description = str_dup(log_buf);
		obj_to_obj( spirit, urn );

		if(world[1729])
		  	obj_to_room(urn, 1729);
		else
			obj_to_room(urn, 1);
	     }
	   }
	}
}



struct char_data *loadundead(struct char_data *ch)
{
int i;
struct char_data *mob=NULL;
struct obj_data *obj=NULL;
struct affected_type af;


mob = read_mobile(9800, REAL);

obj = read_object(9801, REAL);
equip_char(mob,obj, WEAR_WRIST_R);

obj = read_object(9801, REAL);
equip_char(mob,obj, WEAR_WRIST_L);

obj = read_object(9802, REAL);
equip_char(mob,obj, WIELD);

obj = read_object(9803, REAL);
equip_char(mob,obj, WEAR_NECK_1);

obj = read_object(9804, REAL);
equip_char(mob,obj, WEAR_FEET);
 
obj = read_object(9805, REAL);
equip_char(mob,obj, WEAR_SHIELD);
 
obj = read_object(9806, REAL);
equip_char(mob,obj, WEAR_BODY);

if(ch->specials.death_counter <= 9)
	{ 
	obj = read_object(9815, REAL);
	obj_to_char(obj, mob);
	} 

obj = read_object(9807, REAL);
obj_to_char(obj, mob);
 

obj = read_object(9818, REAL);
obj_to_char(obj, mob);
obj = read_object(9818, REAL);
obj_to_char(obj, mob);
obj = read_object(9818, REAL);
obj_to_char(obj, mob);

obj = read_object(9808, REAL);
equip_char(mob,obj, WEAR_LIGHT);
 
obj = read_object(9810, REAL);
sprintf(log_buf,"Something that used to be %s's finger is lying here",
        GET_NAME(ch));
if(objs[obj->item_number]->description!=obj->description)
    obj->description = my_free(obj->description);
obj->description=str_dup(log_buf);
sprintf(log_buf,"This tasty morsel used to be %s's finger.",GET_NAME(ch));
obj->ex_description->description = my_free(obj->ex_description->description);
obj->ex_description->description=str_dup(log_buf);
equip_char(mob,obj, WEAR_FINGER_R);

if(mobs[mob->nr]->player.name!=mob->player.name)
    mob->player.name = my_free(mob->player.name);
mob->player.class = ch->player.class;
mob->player.multi_class = ch->player.multi_class;
strncpy(mob->pwd, ch->pwd,10 );  /* need this to reconnect after lost link */
mob->player.name = str_dup(GET_NAME(ch));
if(mobs[mob->nr]->player.short_descr!=mob->player.short_descr)
    mob->player.short_descr = my_free(mob->player.short_descr);
sprintf(log_buf,"the Undead Corpse of %s",GET_NAME(ch));
mob->player.short_descr = str_dup(log_buf);
if(mobs[mob->nr]->player.long_descr!=mob->player.long_descr)
    mob->player.long_descr = my_free(mob->player.long_descr);
sprintf(log_buf,"The Undead Corpse of %s is here rotting\n",GET_NAME(ch));
mob->player.long_descr = str_dup(log_buf);
 
mob->abilities.str   = 1;
mob->abilities.intel = 14;
mob->abilities.wis   = 14;
mob->abilities.dex   = 14;
mob->abilities.con   = 14;
GET_LEVEL(mob) = GET_LEVEL(ch);
mob->points.hitroll = 0;
mob->points.armor = -300;
mob->points.max_hit = ch->specials.death_corpsehits;
mob->points.hit = mob->points.max_hit;
mob->points.max_move = 813;
mob->points.move = mob->points.max_move;
GET_EXP(mob) = 1;
GET_CLASS(mob) = GET_CLASS(ch);
mob->player.sex = ch->player.sex;
mob->player.time.birth = ch->player.time.birth;
mob->player.weight = 98;
for (i = 0; i < 5; i++)
        mob->specials.apply_saving_throw[i] = -90;
af.type=SPELL_FLY;
af.duration=-1;
af.modifier=0;
af.location=0;
af.bitvector=AFF_FLYING;
af.next=NULL;
affect_to_char(mob,&af);
return(mob);
}



void spiritize(struct char_data *ch, struct char_data *killer)
{
struct affected_type af;
float num;

ch->specials.death_counter++;
/*
if(killer)
  write_tombstone(ch, killer);
*/
ch->specials.timer = 0;
#ifndef PACIFIST
ORIGINAL(ch)->specials.death_timer= 
	MAX(MIN_UNDEAD,( (ch->specials.death_counter * MIN_UNDEAD_PER_DEATH)
				
					));
#else
ORIGINAL(ch)->specials.death_timer=1;
#endif
num=((float)number(1,100))/100.0;
num = num * num * num;
num = MIN(num,5000);
ch->specials.death_corpsehits = hit_limit(ch)
        + (int)(hit_limit(ch) * 5 * num);

SAVE_CHAR_OBJ(ch,-20);

GET_POS(ch)= POSITION_DEAD;
ch->specials.timer = 0;
GET_HIT(ch) = 0;
if(ch->desc)
	STATE(ch->desc) = CON_HOVERING;
af.type=SPELL_DETECT_INVISIBLE;
af.duration=-1;
af.modifier=0;
af.location=0;
af.bitvector=AFF_DETECT_INVISIBLE;
af.next=NULL;
affect_to_char(ch,&af);
af.type=SPELL_INFRAVISION;
af.duration=-1;
af.modifier=0;
af.location=0;
af.bitvector=AFF_INFRARED;
af.next=NULL;
affect_to_char(ch,&af);
act(
"$n's consciousness escapes the painful tomb of its dead body and hovers\r
peacefully above you.  Empowered with extrordinary perception, $e is able\r
to see everything that goes on here, a silent witness to anyone who might\r
try to desecrate the corpse." ,FALSE,ch,0,NULL,TO_ROOM);
send_to_char(
"You're dead now. You can't move or speak.  The world becomes black and your\r
head turns inside out with pain. Unable to scream or move, your senses are\r
overwhelmed with agony and terror. Finally, an escape!  You feel released,\r
lighter than air, nothing but pure consiousness hovering over the hellish\r
pain of your corpse.\n\n\r" ,ch);
}



void undeadify(struct char_data *ch)
{
struct char_data *k=NULL;
struct char_data *corpse=NULL;
struct obj_data *urn=NULL, *spirit=NULL, *obj=NULL;
int ch_in_room;

act(
"A chill fills the air and a pair of greyish hands reach up from below, slowly 
pulling the corpse of $n down into the bowels of the earth until 
all that's left is blood and the stench."
,FALSE,ch,0,NULL,TO_ROOM);

/* make urn for corpse in crypt */
urn = read_object(9811, REAL);
sprintf(log_buf,"A sapphire bottle sits in a niche holding %s's spirit",
		 GET_NAME(ch)
		 );
if(objs[urn->item_number]->description!=urn->description)
    urn->description = my_free(urn->description);
urn->description=str_dup(log_buf);
sprintf(log_buf,"%s bottle sapphire",GET_NAME(ch));
if(objs[urn->item_number]->name!=urn->name)
    urn->name = my_free(urn->name);
urn->name = str_dup(log_buf);
SET_BIT(urn->obj_flags.value[1], CONT_CLOSEABLE);
SET_BIT(urn->obj_flags.value[1], CONT_CLOSED);
spirit=read_object(2999,REAL);
sprintf( log_buf,
	"The glow of %s's spirit softly illuminates the bottle's interior.",
	GET_NAME(ch));
if(objs[2999]->short_description!=spirit->short_description)
    spirit->short_description = my_free(spirit->short_description);
spirit->short_description = str_dup(log_buf);
obj_to_obj( spirit, urn );
if(world[1729])
	obj_to_room(urn, 1729);
else
	obj_to_room(urn, 1);

/* modify player */
/* if no graveyard room number saved by tombstone in ch->internal_use
 * then send char to potter's field
 */
/*
if(GET_CONTINENT(ch)==TRELLOR)
	ch_in_room = (world[TREL_POTTER]) ? TREL_POTTER:1;
else
	ch_in_room = (world[MED_POTTER]) ? MED_POTTER:1;
char_from_room(ch);
*/

if(IN_HOLO(ch))
	ch_in_room = ch->in_room;
else 
	if(!zone_table[world[ch->in_room]->zone].top)
		ch_in_room = ch->in_room;
	else
		ch_in_room = zone_table[world[ch->in_room]->zone].top;


char_from_room(ch);


/*
if(	(ch->internal_use > 0) 
	&& (ch->internal_use < MAX_ROOM) 
	&& (ch->internal_use > 0 ) 
	&& (world[ch->internal_use]) 
	)
	ch->in_room = ch->internal_use;
else
	ch->in_room = ch_in_room;

*/

ch->in_room = ch_in_room;

put_all_in_inv(ch);


obj=ch->carrying;
while(obj)
	{
	while
		(	(obj->next_content) &&
    		(	   (obj->next_content->obj_flags.type_flag==ITEM_KEY)
				|| (obj->next_content->obj_flags.type_flag==ITEM_SCROLL)
           		|| (obj->next_content->obj_flags.type_flag==ITEM_WAND)
            	|| (obj->next_content->obj_flags.type_flag==ITEM_STAFF)
            	|| (obj->next_content->obj_flags.type_flag==ITEM_POTION)
            	|| IS_SET(obj->next_content->obj_flags.extra_flags, ITEM_NO_RENT			)
            )
		)
		extract_obj(obj->next_content);
	obj=obj->next_content;
	}
SAVE_CHAR_OBJ(ch,-20);


corpse=loadundead(ch);
corpse->specials.death_timer=ch->specials.death_timer;
corpse->specials.numkills=ch->specials.numkills;
corpse->specials.numpkills=ch->specials.numpkills;
corpse->specials.ansi_color = ch->specials.ansi_color;
REMOVE_BIT(ch->specials.new_comm_flags, PLR_NOGOSSIP);
char_to_room(corpse, ch_in_room);

/* attatch descriptor to mob */
ch->desc->character = corpse;
ch->desc->original = ch;
corpse->desc = ch->desc;
ch->desc = NULL;
/* delete the PC from character list*/
if(ch==character_list)
	character_list = ch->next;
else
	{
	for ( k=character_list ; (k && (k->next != ch)) ; k = k->next);
	if (k)
		k->next = ch->next;
	else
		{
		log_hd("##Undeadify: bad character_list\n");
		SUICIDE;
		}
	}

STATE(corpse->desc) = CON_UNDEAD;
send_to_char("You slowly rise to your feet, condemed as a zombie until you repent.\r\n",ch);
send_to_char("for your misdoings at a Holy Place.\r\n",ch);

/*
send_to_char(
"
A pair of greyish hands push their way up from below and grab your corpse,\r
dragging you down through tons of earth.  When the hands finally release you\r
something presses heavily against your face. You keep straining to push it\r
away until finally you're free and you realize that you've dug yourself out\r
of your own grave.  You are now one of the undead, doomed to roam the land\r
restlessly and separated from your spirit which is kept sealed away in the\r
Necromancer's Sanctum. One day you may kill the Necromancer and win posession\r
of your spirit.  Until then, you will remain undead.
\n\r",corpse);
*/
                act(
"$n's body slowly rises from the ground. Whatever it was before, it is no 
longer human! (temp).",FALSE,corpse,0,NULL,TO_ROOM); 
}


void reanimate(struct descriptor_data *d, int cmd)

{
struct char_data *corpse=NULL,*player=NULL,*k=NULL,*next_char=NULL;
extern struct char_data *character_list;
int iWear;
int crypt = 1;
struct obj_data *bottle=NULL;

if (!d)
	{
	log_hd("##No descriptor for reanimated player. Aborting.");
	return;
	}

/* remove spirit bottle from crypt */
if (world[1729])
        crypt=1729;
bottle = get_obj_in_list( GET_NAME(d->character), world[crypt]->contents );
if (bottle)
	extract_obj(bottle);

if (d->character->specials.fighting)
	stop_fighting(d->character);
    for ( k = combat_list; k ; k = next_char )
    {
        next_char = k->next_fighting;
        if ( k->specials.fighting == d->character )
            stop_fighting( k );
    }

corpse = d->character;
player = d->original;

act("$n drags the lifeless body of the necromancer off in
the direction of his sanctum.",FALSE,corpse,0,NULL,TO_ROOM);

/* return from corpse */
d->character = d->original;
d->original = NULL;
d->character->desc = d;

/* add player back into character list */
player->next        = character_list;
character_list      = player;

/*
if(GET_CONTINENT(corpse)==TRELLOR)
	player->in_room = (world[TREL_SANCTUM]) ? TREL_SANCTUM:1;
else
	player->in_room = (world[MED_SANCTUM]) ? MED_SANCTUM:1;
*/

char_to_room(player,corpse->in_room);

STATE(d) = CON_PLAYING;

/* get rid of undead corpse */
for(iWear=0; iWear<MAX_WEAR; iWear ++)
	if(corpse->equipment[iWear])
		obj_to_char(unequip_char(corpse,iWear), corpse);
while(corpse->carrying)
	extract_obj(corpse->carrying);
corpse->specials.home_number=20007;
extract_char(corpse, TRUE);
GET_POS(player) = POSITION_RESTING;
GET_HIT(player) = 10;
player->specials.death_timer=0;
while ( player->affected )
	affect_remove( player, player->affected );
SAVE_CHAR_OBJ(player, -20);
if (cmd)
	{
	send_to_char(
"You drag the lifeless body of the Necromancer back to his sanctum and find\r
the bottle containing your spirit.  You're barely strong enough to pull out\r
the stopper.  His hand is already cold, but the magical aura still left in\r
the twisted fingers is enough to free your spirit from its crystal prison.\r
You feel like new!  Your flesh magically grows back, your posessions are\r
returned and you are restored to the condition you were in before your\r
death. Then, before you can comprehend what's happened, the corpse\r
shrivels up and dissolves into a pile of dust.\r
\n\r",player);
	act(
"$n limps in dragging the lifeless body of a Necromancer. The magical aura\r
still left in the dead mage's fingers frees the spirit from the bottle and\r
it reunites with $n, magically bringing him back to life. Before you realize\r
what's happened, the corpse of the necromancer dissolves into a pile of dust."
,FALSE,player,0,NULL,TO_ROOM);
	}
else
	{
	send_to_char("
A divine force takes pity on your wretched existance and reunites you\r
with your spirit, breathing life into your body.\r
\n\r",player);
	}
}

void do_kill(struct char_data *ch, char *argument, int cmd)
{
char arg[MAX_STRING_LENGTH];
struct char_data *victim=NULL;

one_argument(argument, arg);

if (!*arg)
    {
    send_to_char("Slay whom?\n\r", ch);
    return;
    } 
 
victim = get_char_room_vis(ch, arg);

if (!victim)
	{
        send_to_char("They aren't here.\n\r", ch);
	return;
	}

if (!if_allowed_to_attack(ch,victim))
	return;

if (ch == victim)
	{
	send_to_char("Your mother would be so sad.. :(\n\r", ch);
	return;
	}

if ( (GET_LEVEL(ch) < 33) || IS_NPC(ch))
	{
	do_hit(ch,argument,9);
	return;
	}

if( (GET_LEVEL(victim) > GET_LEVEL(ch)) && !IS_NPC(victim) ) {
	send_to_char("I wouldn't try that if I were you.\r\n",ch);
	return;
}

if(IS_PLAYER(victim,"Io"))
	{
	send_to_char("Io would DEFINITELY not like that!\n\r",ch);
	do_tell(ch, "io I JUST TRIED TO KILL YOU!",9);
	return;
        }

if(IS_PLAYER(victim,"Vryce")
	&&!IS_PLAYER(ch,"Io")
        )
	{
	send_to_char("Vryce would DEFINITELY not like that!\n\r",ch);
	do_tell(ch, "vryce I JUST TRIED TO KILL YOU!",9);
	return;
	}

if(IS_PLAYER(victim,"Firm")
        &&!IS_PLAYER(ch,"Vryce")
        &&!IS_PLAYER(ch,"Io")){
          send_to_char("Firm would DEFINITELY not like that!\n\r",ch);
          do_tell(ch, "firm I JUST TRIED TO KILL YOU!", 9);
          return;
}

if (!IS_NPC(victim)) {
  sprintf(log_buf,"##%s[%d] brutally slays %s[%d].\n",
	  GET_NAME(ch),GET_LEVEL(ch),GET_NAME(victim),GET_LEVEL(victim));
  log_hd(log_buf);
}
global_color=31;
act("You chop $M to pieces! Ah! The blood!", FALSE, ch, 0, victim, TO_CHAR);
act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
act("$n brutally slays $N.", FALSE, ch, 0, victim, TO_NOTVICT);
global_color=0;
raw_kill(victim,NULL);
make_ashes(ch, victim);
}

void raw_kill( struct char_data *ch, struct char_data *attacker )
{
int iWear,potter,to_room;
struct char_data *k,*next_char;
void die_formation(struct char_data *ch);

if(!IS_NPC(ch))
	ch->p->querycommand = 0;

	if(ch->specials.death_timer<0)
		return;

die_formation(ch);


if (ch->specials.fighting) 
        stop_fighting( ch );
    for ( k = combat_list; k ; k = next_char )
    {
        next_char = k->next_fighting;
        if ( k->specials.fighting == ch )
            stop_fighting( k );
    }
    if(ch->in_room==-1){
        sprintf(log_buf,"## %s vs room -1 in raw_kill",
                        GET_NAME(ch));
        log_hd(log_buf);
        return;
    } 
    if(attacker &&  attacker->in_room==-1){
        sprintf(log_buf,"## %s vs %s room -1 in raw_kill",
                        GET_NAME(ch),GET_NAME(attacker));
        log_hd(log_buf);
        return;
    } 


death_cry( ch );


if((GET_LEVEL(ch) > 31) && !IS_NPC(ch))
    {
    send_to_char(
	"Instead of dying you merely pass on to another plane.\n\r",ch);
    act("$n passes on to another plane.\n\r", FALSE, ch, 0, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, 0);
    GET_HIT(ch) = 1;
    return;
    }

if(attacker
		&& IS_UNDEAD(attacker)
		&& (ch->nr == 9801)
		&& ch->specials.death_timer
	)
	{
	for(iWear = 0; iWear < MAX_WEAR ; iWear++)
		if(ch->equipment[iWear])
			obj_to_char(unequip_char(ch,iWear), ch);
	while(ch->carrying)
		extract_obj(ch->carrying);
	ch->specials.home_number=20008;
	extract_char( ch, TRUE );
	reanimate(attacker->desc, 1);
	return;
	}
 
if ( IS_NPC(ch) && !IS_DEAD(ch) )
    {
    make_corpse( ch ,0);
    ch->specials.home_number=20009;
    extract_char( ch, TRUE );
    return;
    }
if(IS_NPC(ch)&&!IS_UNDEAD(ch)){
       sprintf(log_buf,"##A NPC(%s) IS NOT BEING EXTRACTED IN RAW_KILL!",ch->player.short_descr); 
       log_hd(log_buf);
 }
 
/* do this so people don't spam the city with VfRuYcCkE corpses */
if(GET_LEVEL(ch) < 5)
    {

	if(GET_CONTINENT(ch)==TRELLOR)
		to_room = (world[1371]) ? 1371:1;
	else
		to_room = (world[3099]) ? 3099:1; /* go to tourist center */
    char_from_room(ch);
	char_to_room(ch,to_room);

	if ( GET_HIT(ch)  <= 0 )
        GET_HIT(ch) = 2;
	if ( GET_MOVE(ch) <= 0 )
        GET_MOVE(ch) = 1;
	if ( GET_MANA(ch) <= 0 )
        GET_MANA(ch) = 1;
	GET_POS(ch) = POSITION_RESTING;
	ch->specials.affected_by    =  0;
    send_to_char(
"The gods take pity on your poor newbie existance by restoring your life and\r
placing you safely out of harm's way.",ch);
    act(
"$n arrives in a puff of smoke, near death.",FALSE,ch,0,NULL,TO_ROOM);
    return;
    }
 
if(IS_DEAD(ch))
	{
	sprintf(log_buf,
	"You're already dead!  Adding 5 minutes.\n\r");
	send_to_char(log_buf,ch);
	act("You are carted back to the graveyard and re-buried.", 
	FALSE, ch, 0, NULL, TO_CHAR);
	act("$n is carted back to the graveyard and re-buried.", 
	FALSE, ch, 0, NULL, TO_ROOM);
	ORIGINAL(ch)->specials.death_timer += 5;
	ch->points.hit = 1;
/*
	if(ch->in_room<0)
		{
	    potter=1;
        }
	else
		{
		if(GET_CONTINENT(ch)==TRELLOR)
			potter = (world[TREL_POTTER]) ? TREL_POTTER:1;
		else
			potter = (world[MED_POTTER]) ? MED_POTTER:1;
		}
	char_from_room(ch);
	char_to_room(ch, potter);
*/
	GET_POS(ch) = POSITION_RESTING;
	act("$n digs $mself back up out of $s grave",FALSE, ch, 0, NULL, TO_ROOM);
	return;
	}

else
    {
	if(IS_HOVERING(ch)) {
		send_to_char("You can not die again!.\r\n",ch);
		return;
	}
		
	ch->specials.affected_by    = 0;
	GET_POS(ch)                 = POSITION_RESTING;
	while ( ch->affected )
		affect_remove( ch, ch->affected );
	if ( GET_HIT(ch)  <= 0 )
        GET_HIT(ch) = 1;
	if ( GET_MOVE(ch) <= 0 )
        GET_MOVE(ch) = 1;
	if ( GET_MANA(ch) <= 0 )
        GET_MANA(ch) = 1;
	if(IS_NPC(ch)||IS_SET(world[ch->in_room]->room_flags,CHAOTIC))
        GET_GOLD(ch)=0;
	SAVE_CHAR_OBJ(ch,-20);
    if(ch->specials.clan)
		{
        sprintf(log_buf,"[CLAN] %s has BEEN KILLED!\n\r",GET_NAME(ch));
        tell_clan(ch->specials.clan,log_buf);
		}
/* NPK suicide counts as NPK death instead of Chaotic because people
   used to time flee at the end of NPK battle so opponent would die
   of mortal wounds and they could loot the corpse
*/
    if((attacker)
		&&	(   /* suicide in npk */
				(	(attacker == ch) 
					&& IS_SET(world[ch->in_room]->room_flags,NEUTRAL)
				)
			||	(	(attacker != ch)
                	&& !IS_NPC(attacker)
               		&& !IS_SET(world[ch->in_room]->room_flags,CHAOTIC)
                )
			)
		)
        {
/* to prevent people from recalling during pk quest, have person stay
   in same room in arena suicide instead of going to trash heap.
*/
		if( (ch == attacker) && (world[ch->in_room]->zone == 15) )
			{
			to_room = ch->in_room;
			WAIT_STATE(ch,3);
			}
		else if(world[ch->in_room]->zone == 65)
			{
			to_room = (world[4298]) ? 4298:1;
			make_corpse(ch,1);
			}
		else if(GET_CONTINENT(ch)==TRELLOR)
			{
			to_room = (world[3942]) ? 3942:1;
	        	make_corpse( ch,1 );
			}
		else
			{
			to_room = (world[280]) ? 280:1;
        		make_corpse( ch,1 );
			}
        extract_char_neutral_zone( ch, FALSE );
        char_from_room(ch);
		char_to_room(ch,to_room);
        send_to_char("
\rYou wake up with a splitting headache that runs clear down to your
\rtoenails.  You thought you had died, but you realize you only feel like it.
\r",ch);
		act("
You see a hand sticking out from under some trash.  You look closer and see 
that its attatched to $n whom you nearly mistake $m for dead.", 
FALSE, ch, 0, NULL, TO_ROOM);
        }
    else
        {
        spiritize(ch,attacker);
        }
    }
}

void do_reanimate(struct char_data *ch, char *argument, int cmd)
{
void reanimate(struct descriptor_data *d, int cmd);
char person[MAX_INPUT_LENGTH];
struct char_data *victim=NULL;


if (IS_NPC(ch))
	return;

one_argument(argument, person);
if (!*person)
	{
        send_to_char("Syntax: reanimate <player>\n\r", ch);
	}
else
	{
	if (!(victim = get_char(person)))
            send_to_char("They aren't here.\n\r", ch);
	else if (victim->specials.fighting)
            send_to_char("They're fighting.\n\r", ch);
	else
		{
		sprintf(log_buf,"%s reanimating %s.",
			GET_NAME(ch),
			GET_NAME(victim));
		log_hd(log_buf);
		if (	(victim->nr == 9800)
			&& (victim->desc)
			&& (victim->specials.death_timer)
			)
			{
			sprintf(log_buf,"%s reanimates %s.",
				GET_NAME(ch),GET_NAME(victim));
			log_hd(log_buf);
			reanimate(victim->desc, 0);
			}
		else
			send_to_char("Can only reanimate the undead.\n\r",ch);
		}
	}
}

void load_necros(void)
{
struct char_data *cur=NULL,*cur_next=NULL;
int iWear;
int trel_necro_count = 0;
int med_necro_count = 0;

for (cur = character_list ; cur ; cur = cur_next){
	cur_next=cur->next;
	if(!IS_NPC(cur))continue;
	if((cur->nr == 9801) && cur->specials.death_timer){
		if((cur->specials.death_timer > 15)
			&& (!cur->specials.fighting)
			&& !number(0,2)
		){
		    for(iWear=0; iWear<MAX_WEAR; iWear ++)
		  	if(cur->equipment[iWear])
				obj_to_char(unequip_char(cur,iWear), cur);
		    while(cur->carrying)
        		extract_obj(cur->carrying);
		    /* COPY TO THE DESCRIPTION SOME TEXT, SO WE KNOW IF THE
			THING WAS FREED BUT YET STILL BEING USED?
			WE CAN COPY THIS CAUSE WE KNOW THE DESCRIPTION WAS
			MALLOC'D AT LEAST THIS BIG*/
		    strcpy(cur->player.description,"THIS NECRO WAS FREED!");
		    act("The Necromancer leaves in search of new victims.",FALSE,cur,0,NULL,TO_ROOM);
		    cur->specials.home_number=20011;
		    extract_char(cur, TRUE);
		}else{
		  
			if(GET_CONTINENT(cur) == TRELLOR)
				trel_necro_count++;
			else
				med_necro_count++;
		}
	}
}

place_necros(TRELLOR,trel_necro_count);
place_necros(MEDIEVIA,med_necro_count);
}


void place_necros(int continent, int to_load)
{
int i, room, tries;
bool loaded;

for(i=0; (i < (MAX_NECROS - to_load)) ; i++)
  {
  loaded = FALSE;
  tries = 0;
  while ( (!loaded) && (tries < 2000) )
  	{
	tries++;
        room = number(100, (top_of_world - 1));
/*
		if (world[room] && (world[room]->zone == 72) && !number(0,4))
			continue;
*/
        if (    (world[room])
			&& (zone_table[world[room]->zone].continent == continent)
			&& (world[room]->people)
			&& (IS_NPC(world[room]->people))
			&& (world[room]->people->nr != 9801)
            && (!is_shop(room))
            && (world[room]->zone != 15)         /* not arena */
            && (world[room]->zone != 95)         /* god's homes */
            && (world[room]->zone != 180)        /* casino */
            && (world[room]->zone != 65)         /* mystara */
            && (world[room]->zone != 198)        /* catacombs */
            && (!world[room]->class_restriction)
            && (!world[room]->level_restriction)
            && (!world[room]->align_restriction)
            && (!world[room]->mount_restriction)
            && (!IS_SET(world[room]->room_flags,GODPROOF))
            && (!IS_SET(world[room]->room_flags,PRIVATE))
            && (!world[room]->funct)
            )
                {
                load_necromancer(room,world[room]->people);
                loaded = TRUE;
                }
        }
  if (tries >=1999)
	log_hd("## Giving up on placing Necromancer");
  }
}


void load_necromancer(int room, struct char_data *victmob)
{
int potions,max_potions,max_pkills=0;
struct char_data *mob=NULL,*cur=NULL;
struct obj_data *obj=NULL,*skull=NULL;

mob = read_mobile(9801, REAL);
mob->specials.death_timer = 1;  /* this is a flag to show code loaded it,
				   and not some pesky god. also used to
				   time re-cycling of necromancers.*/
for (cur = character_list ; cur ; cur = cur->next)
	{
	if(IS_UNDEAD(cur) && (ORIGINAL(cur)->specials.numpkills > max_pkills))
		max_pkills = ORIGINAL(cur)->specials.numpkills;
	}
	
skull = read_object(9814, REAL);
if(max_pkills > 20)
	{
	obj = read_object(13113, REAL);
	obj_to_obj( obj, skull );
	}
if(max_pkills > 30)
	{
	obj = read_object(1411, REAL);
	obj_to_obj( obj, skull );
	}
obj = read_object(9819, REAL);
if(!number(0, MAX(1,(30 - max_pkills))))
	{
	obj->obj_flags.wear_flags += 16384;
	obj->affected[0].location = 5;
	obj->affected[0].modifier = 3;
	obj->affected[1].location = 13;
	obj->affected[1].modifier = 10;
	}
obj_to_obj( obj, skull );
max_potions = number(10, MIN(30, max_pkills));
for(potions=0;potions <= max_potions;potions++)
	{
	obj = read_object(9819, REAL);
	obj_to_obj( obj, skull );
	}
equip_char(mob,skull, HOLD);

obj = read_object(9813, REAL);
equip_char(mob,obj, WEAR_BODY);

char_to_room(mob,room);
do_gossip(victmob, "Beware, the Necromancer!",0);
act(
"A sudden chill in the air announces the arrival of a Necromancer, come in
search of new victims.",FALSE,mob,0,NULL,TO_ROOM);
}

void undead_gross_out(struct char_data *ch, struct char_data *k)
{
int chance;

chance = GET_LEVEL(ch) - GET_LEVEL(k) + 30;

if(world[k->in_room]->sector_type == SECT_CITY)
	chance /=2;

if(	   (k->specials.fighting)
	|| (k->in_room == 11) || (k->in_room == 18) 
	|| (k->in_room == 25) || (k->in_room == 5)
	|| (k->in_room == 7)  || (k->in_room == 10)
	|| (k->in_room == 9)  || (k->in_room == 8)
	)
	chance /= 3;

if( 	   affected_by_spell(k, SPELL_BLESS) 
	|| affected_by_spell(ch, SPELL_BLESS)
	|| in_a_shop(k) 
	|| world[k->in_room]->funct
	|| (k->in_room == 1) || (k->in_room == 4) 
	|| (k->in_room == 3) || (k->in_room == 5)
	|| IS_DEAD(k) 
	|| (mob_index[real_mobile(k->nr)].func)
	|| IS_SET(k->specials.act,ACT_AGGRESSIVE)
	|| IS_SET(k->specials.act,ACT_SENTINEL)
	|| (GET_POS(k) < POSITION_RESTING)
	)
	chance = 0;

if( (number(1,100) > (100 - chance) ) && (GET_LEVEL(k) < 32))
	{
	sprintf(log_buf,
		"screams in terror at the sight of %s.",ch->player.short_descr);
	do_emote(k, log_buf,0);
	do_flee( k, "", 0 );
	}
}


int room_reanimate(struct char_data *ch, int cmd, char *arg)
{
/* If an undead with negative death timer prays, reanimate them
 */

if(    (cmd == 9866)
	&& IS_UNDEAD(ch) 
	&& (ORIGINAL(ch)->specials.death_timer <1) 
  )
    {
	/*ORIGINAL(ch)->specials.numpkills -= 
		number(1, (int)(ch->specials.numpkills/5));*/
        ORIGINAL(ch)->specials.numpkills = ch->specials.numpkills/2;
	if(ORIGINAL(ch)->specials.numpkills < 1)
		ORIGINAL(ch)->specials.numpkills = 0;
    reanimate(ch->desc, 0);
    return(TRUE);
    }
else
    return(FALSE);

}
