/***************************************************************************
*					 MEDIEVIA CyberSpace Code and Data files		       *   
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*							   All rights reserved				           * 
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

/*   external vars  */
extern char *GET_REAL_NAME(struct char_data *ch);
struct index_data *mob_index;
extern int number_of_players();
extern unsigned long int connect_count;
extern int number_of_rooms;
extern int number_of_zones;
extern struct zone_data *zone_table;
extern char global_color;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern char NODEATHTRAP;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern put_obj_in_store(struct obj_data *obj, struct char_data *ch, FILE *fpsave);
extern bool in_a_shop(struct char_data *ch);
extern char *fread_paper(FILE *fl);
extern int top_of_world;
extern int top_of_zone_table;
/* extern procedures */
bool put_in_formation(struct char_data *leader, struct char_data *follower);
void hit(struct char_data *ch, struct char_data *victim, int type);
void gain_exp(struct char_data *ch, int gain, struct char_data *victim);
extern bool is_formed(struct char_data *ch);
void cast_burning_hands( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_chill_touch( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_colour_spray( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_energy_drain( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_fireball( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_magic_missile( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_blindness( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_curse( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_sleep( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_armor( byte level, struct char_data *ch, char *arg, int type,
	struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_bless( byte level, struct char_data *ch, char *arg, int type,
	struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_cure_light( byte level, struct char_data *ch, char *arg, int type,
	struct char_data *tar_ch, struct obj_data *tar_obj );
/* Dragon breath .. */
void cast_fire_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_frost_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_acid_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_gas_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_lightning_breath( byte level, struct char_data *ch, char *arg,
	       int type, struct char_data *tar_ch,
	       struct obj_data *tar_obj );


/* Data declarations */

struct social_type
{
  char *cmd;
  int next_line;
};


/* ********************************************************************
*  Special procedures for rooms                                       *
******************************************************************** */

void do_push(struct char_data *ch, char *argument, int cmd)
{
int new_class, iWear;
    if(IS_NPC(ch))return;
    if(cmd==1000){
        if(!argument||!argument[0]){
	    ch->p->querycommand=0;
	    return;
        }
	if(ch->specials.eggs<1000){	
	    send_to_char("
You must help fight the WAR!  Vryce will make you reborn into a new 
player only if you have collected 1000 eggs from the catacombs.\n\r",ch);
	    ch->p->querycommand=0;
	    return;

	}
	switch(argument[0]){
	    case 'M':
	    case 'm':
		if(GET_CLASS(ch)==CLASS_MAGIC_USER||
		        IS_SET(ch->player.multi_class,MULTI_CLASS_MAGIC_USER)){
		    send_to_char("You have done that already!\n\r",ch);
		    return;
		}
		new_class=CLASS_MAGIC_USER;
		break;
	    case 'C':
	    case 'c':
		if(GET_CLASS(ch)==CLASS_CLERIC||
		        IS_SET(ch->player.multi_class,MULTI_CLASS_CLERIC)){
		    send_to_char("You have done that already!\n\r",ch);
		    return;
		}
		new_class=CLASS_CLERIC;
		break;
	    case 'W':
	    case 'w':
		if(GET_CLASS(ch)==CLASS_WARRIOR||
		        IS_SET(ch->player.multi_class,MULTI_CLASS_WARRIOR)){
		    send_to_char("You have done that already!\n\r",ch);
		    return;
		}
		new_class=CLASS_WARRIOR;
		break;
	    case 'T':
	    case 't':
		if(GET_CLASS(ch)==CLASS_THIEF||
		        IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF)){
		    send_to_char("You have done that already!\n\r",ch);
		    return;
		}
		new_class=CLASS_THIEF;
		break;
	    default:
		send_to_char("Huh? Try again.\n\r",ch);
		return;
		break;
	}
	ch->specials.eggs-=1000;
	switch(GET_CLASS(ch)){
	    case CLASS_MAGIC_USER:
		SET_BIT(ch->player.multi_class,MULTI_CLASS_MAGIC_USER);
		break;
	    case CLASS_CLERIC:
		SET_BIT(ch->player.multi_class,MULTI_CLASS_CLERIC);
		break;
	    case CLASS_WARRIOR:
		SET_BIT(ch->player.multi_class,MULTI_CLASS_WARRIOR);
		break;
	    case CLASS_THIEF:
		SET_BIT(ch->player.multi_class,MULTI_CLASS_THIEF);
		break;
	}
        for(iWear = 0; iWear < MAX_WEAR ; iWear++)
                if(ch->equipment[iWear])
                        obj_to_char(unequip_char(ch,iWear), ch);

	GET_CLASS(ch)=new_class;
	GET_LEVEL(ch)=1;
	GET_EXP(ch)=1;
	set_title(ch);
	ch->points.max_hit=25;
	ch->points.max_mana=150;
	ch->points.max_move=150;
	GET_HIT(ch)=25;
        GET_MANA(ch)=150;
	GET_MOVE(ch)=150;
	ch->specials.practices+=25;
send_to_room("
The room is filled with a blinding light for 3 full breaths...you feel 
yourself flung as if in a hurricane...your sense of balance fails you..
you're falling...THWOOP!  You blink to clear your eyes.
",ch->in_room);
	if(GET_CLASS(ch)==CLASS_THIEF){
            ch->skills[ SKILL_SNEAK     ].learned = 10;
            ch->skills[ SKILL_HIDE      ].learned =  5;
            ch->skills[ SKILL_STEAL     ].learned = 15;
            ch->skills[ SKILL_BACKSTAB  ].learned = 10;
            ch->skills[ SKILL_PICK_LOCK ].learned = 10;
	}
	ch->p->querycommand=0;
	return;
    }
    if(ch->in_room!=6151){
	send_to_char("You cannot do that here.\n\r",ch);
	return;
    }
    if(GET_LEVEL(ch)!=31){
	send_to_char("You are ZAPPED by the button!\n\r",ch);
	return;
    }
    if(get_total_level(ch)>=124)return;
    send_to_room("A powerful voice roars across the mountain...
You ... are ... *NOT* ... <Ready> ... for godhood!  You are not wise enough,
not OLD enough!  Not complete!  Go back.. Go back...... Go back as a newborn!
Learn what there is to be learned from the mortal realm...learn all things
great and small...learn all skills..learn all classes..be everything..
Come back to me, Mortal when you are Ready!
",6151);
    strcpy(ch->p->queryprompt,"Pick a class you have not mastered yet.
([M]age [C]leric [W]arrior [T]hief) or ENTER to abort> ");
    ch->p->querycommand=1000;
    ch->p->queryfunc=do_push;
    return;
}





int horneg_mage_room(struct char_data *ch, int cmd, char *arg)
{
struct char_data *mob=NULL;
char gotit=0;


mob = get_char_room("horneg", ch->in_room); 
if (ch==mob)
	return(FALSE);
if (mob) 
	{
    if(GET_LEVEL(ch)>31)return(FALSE); 
	if(mob->specials.fighting)return(FALSE);
	if(ch->equipment[WEAR_NECK_1])
	    if(ch->equipment[WEAR_NECK_1]->item_number==10009)
	       	gotit=1;
	if(ch->equipment[WEAR_NECK_2])
	    if(ch->equipment[WEAR_NECK_2]->item_number==10009)
	       gotit=1;
	if(!gotit)
		{
	    act("$n says, 'You are NOT Protected!' $n makes $N disappear!.", TRUE,mob, 0, ch, TO_ROOM); 
	    char_from_room(ch); 
	    char_to_room(ch, 10003); 
	    do_look(ch, "", 15); 
	    return(TRUE);
		}
     }
     return(FALSE);
}

int horneg_sarcophogus_room(struct char_data *ch, int cmd, char *arg)
{
struct char_data *mob=NULL;
int r_num;
struct obj_data *sarcophagus=NULL, *corpse=NULL;

     if(cmd==222
		&& ( isname("sarcophagus",arg)
			|| isname("large",arg)
			|| isname("stone",arg)
		)
	){
     sarcophagus=get_obj_in_list("sarcophagus",world[ch->in_room]->contents);
     if(!sarcophagus)
	return(FALSE);
     else 
	corpse = get_obj_in_list("corpse",sarcophagus->contains);
     if(!corpse)
	return(FALSE);
     mob = get_char_room_vis(ch, "ghost"); 
     if (!mob) {
	act("A large veil of mist swirls up from the sarcophogus and shifts into the form of a beautiful young lady!",TRUE,ch,0,0,TO_ROOM);
	send_to_char("A large veil of mist swirls up from the sarcophogus and shifts into the form of a beautiful young lady!\n\r",ch);
 	r_num = real_mobile(10014);
	mob = read_mobile(r_num, REAL); 
	char_to_room(mob, ch->in_room); 
	do_emote(mob,"looks at you with fire in her eyes!",0);
	do_say(mob,"You have defiled my sanctuary, now You Must Die!",0);
	do_close(mob,"wall",0);
	hit(mob,ch,0);
	return(FALSE);
     }
     }
     return(FALSE);
}

int convention(struct char_data *ch, int cmd, char *arg)
{
int room;
    if(cmd!=28)return(FALSE);

    send_to_char("You feel dizzy and disoriented!\n\r",ch);
    send_to_char("Your whole being begins fading in and out.\n\r",ch);
    send_to_char("Past dreams confront you from all angles.\n\r",ch);
    send_to_char("You pass out.\n\r",ch);
    send_to_char("You wake up feeling groggy, remembering what must have been a dream...\n\r",ch); 
    act("$n steps into the portal and vanishes.\n\r",TRUE,ch,0,0,TO_ROOM);
    if(world[ch->in_room]->number==25)room=204;
    else room=25;
    char_from_room(ch);
    char_to_room(ch,room);
    do_look(ch,"",9);
    return(TRUE);
}

char *how_good(int percent)
{
    if (percent == 0)
	return ( " (not learned)");
    if (percent <= 10)
	return ( " (awful)");
    if (percent <= 20)
	return ( " (bad)");
    if (percent <= 40)
	return ( " (poor)");
    if (percent <= 55)
	return ( " (average)");
    if (percent <= 70)
	return ( " (fair)");
    if (percent <= 80)
	return ( " (good)");
    if (percent <= 85)
	return ( " (very good)");

    return ( " (Superb)");
}

/* Gives a random Mobile the Crystal, for Tear Fountain Questing */
void load_dragon_crystal(void)
{
int random, r_num,tries;
struct char_data *mob=NULL;
struct obj_data *obj=NULL;
    tries=0;
    r_num=real_object(1399);
    if(r_num<0){
		log_hd("###Some idiot removed object 1399 for tear fountain##");
		return;
    }
    while(tries<1500){
	tries++;
	random=number(100,top_of_world-1);
	if(world[random]&&world[random]->zone<196)
        for(mob=world[random]->people;mob;mob=mob->next_in_room)
	    if(mob
		&& IS_NPC(mob)
		&& !in_a_shop(mob)
		&& !(mob->specials.death_timer) /* not a corpse or necro */
		&& (!world[mob->in_room]->level_restriction)
		){
		    obj=read_object(r_num,0);
		    obj_to_char(obj,mob);
		    return;
	    }
    }
    log_hd("## gave up loading a dragon crystal after 1500 tries");
}


int mythago_wood(struct char_data *ch, int cmd, char *arg)
{
int room;

    if ( (GET_LEVEL(ch) >=32) && IS_AFFECTED(ch,AFF_INVISIBLE) )
        return(FALSE);

    send_to_char("A tree falls down and thumps you on the head!\n\r",ch);
    send_to_char("Your whole being begins fade in and out.\n\r",ch);
    send_to_char("Past dreams confronts you from all angles.\n\r",ch);
    send_to_char("You pass out.\n\r",ch);
    send_to_char("As you wake up, feeling groggy, you remember what must have been a dream..\n\rAbout being in the spooky wood...NNNaaaa.\n\r",ch); 
    act("You notice $n was here a second ago and is now, Mmmm, just gone?\n\r",TRUE,ch,0,0,TO_ROOM);
    char_from_room(ch);
    do{
	room=number(5,100);
    } while(room == 79 || room == 82);
/*    }while(room>=2900&&room<=2999);    */
   
/* skip these rooms because they were deleted them from the city and 
   re-used in trellor.  People were transporting from medievia to trellor.
 */ 
	if(room==79)
		return(FALSE);
	if(room==82)
		return(FALSE);

    char_to_room(ch,room);
    return(TRUE);
}

int swamp_bog(struct char_data *ch, int cmd, char *arg)
{
int n;

    n=1050-(GET_WEIGHT(ch)+IS_CARRYING_W(ch))*2;
    if(cmd<1||cmd>6)return(FALSE);
    if(IS_AFFECTED(ch,AFF_FLYING)){
	send_to_char("You FLY just above the bog so you don't get stuck!\n\r",ch);
	return(FALSE);
    }
    if(number(0,n)<100){
	send_to_char("You try to free your feet from the bog mud but fail.\n\r",ch);
	send_to_char("Perhaps you are TOO HEAVY! (hint)\n\r",ch);
	act("$n tries to move but is stuck in the bog mud.",TRUE,ch,0,0,TO_ROOM);
        return(TRUE);
    }
    return(FALSE);

}
int river_tree(struct char_data *ch, int cmd, char *arg)
{
int gotohere=2838;
    
    if ( (GET_LEVEL(ch) >=32) && IS_AFFECTED(ch, AFF_INVISIBLE))
       return(FALSE);

    while(gotohere==2838){
	gotohere=number(2800,2885);
    }
    send_to_char("The Old Man Willow tree GRABS YOU, in a steel-like grip!\n\r",ch);
    send_to_char("As you struggle to break free, it pulls you down its gaping throat.\n\r",ch);
    send_to_char("You are mysteriously transported into the middle of a spooky wood!\n\r\n\r",ch);

    act("The Old Man Willow tree GRABS $n, in a steel like grip!",TRUE,ch,0,0,TO_ROOM);
    act("As $n struggles to break free, it pulls $n down its gaping throat.",TRUE,ch,0,0,TO_ROOM);
    act("$n is gone.",TRUE,ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, gotohere);
    do_look(ch,"",15);
    return(TRUE);
}

int tear_fountain(struct char_data *ch, int cmd, char *arg)
{
struct obj_data *obj=NULL;
int a;

    if(cmd!=70)return(FALSE);

    for(obj=ch->carrying;obj;obj=obj->next_content)
	if(obj->item_number==1399)break;
    if(!obj){
	send_to_char("\n\rthe Dragon growls, 'YOU DO NOT HAVE IT!'\n\r",ch); 
	send_to_char("'YOU THINK I AM A FOOL MORTAL!'\n\r",ch);
	send_to_char("'BE GONE! BEFORE I LOOSE MY TEMPER!'\n\r",ch);
	send_to_char("\n\r\n\rZZZZZAAAAaaaaaPPPP!!\n\rYou are brought to your knees from the PAIN!\n\r",ch);
	GET_HIT(ch)=10;
	return(TRUE);
    }
    extract_obj(obj);
    act("The Ground TREMBLES beneath your feet!",TRUE,ch,0,0,TO_ROOM);
    act("The Dragon shouts, 'You have FOUND IT!'",TRUE,ch,0,0,TO_ROOM);
    act("The Ground TREMBLES beneath your feet!",TRUE,ch,0,0,TO_CHAR);
    act("The Dragon shouts, 'You have FOUND IT!'",TRUE,ch,0,0,TO_CHAR);
	if(ch->specials.practices > 175)
		{
	    sprintf(log_buf,"As a Reward I give you this orb!");
	    act(log_buf,TRUE,ch,0,0,TO_ROOM);	
	    act(log_buf,TRUE,ch,0,0,TO_CHAR);
		obj_to_char(read_object(1411,REAL),ch);
    	sprintf(log_buf,"## %s Offered a Dragon Crystal for an orb.",
			GET_NAME(ch));
    	log_hd(log_buf);
		}
	else
		{
		a=number(20,75);
	    sprintf(log_buf,"As a Reward I give you %d Practice Points!",a);
	    act(log_buf,TRUE,ch,0,0,TO_ROOM);	
	    act(log_buf,TRUE,ch,0,0,TO_CHAR);	
	    do_gossip(ch,
			"I FOUND THE DRAGON CRYSTAL AND HAVE RECEIVED MY REWARD!",0);
	    ch->specials.practices+=a;
    	sprintf(log_buf,"## %s Offered a Dragon Crystal for %d pracs.",
			GET_NAME(ch),a);
    	log_hd(log_buf);
		}
    load_dragon_crystal();
    return(TRUE);
}

int death_trap(struct char_data *ch, int cmd, char *arg)
{
/*int to_room;*/
    if(IS_HOVERING(ch) || IS_DEAD(ch) || NODEATHTRAP)
		return(FALSE);
    
    if(GET_LEVEL(ch)>31)
		return(FALSE);
    send_to_char("[DEATH TRAP!] YOU ARE DEAD!\n\r",ch);
/*
	if(GET_CONTINENT(ch) == TRELLOR)
		to_room=230;
	else
		to_room=3628;
    char_from_room(ch);
	char_to_room(ch,to_room);
*/  
    raw_kill(ch,NULL);
    return(1);
}


int Spirited_Heights_DT(struct char_data *ch, int cmd, char *arg)
/* 
Hanna tells you, 'My request is corpses in room 5164 (deathtrap) be 
moved to room 2982 with no notice of the transfer.'.
*/
{
int in_room;

if(IS_HOVERING(ch) || IS_DEAD(ch) )
	return(FALSE);

in_room = ch->in_room;
if ( (GET_LEVEL(ch) >=31) )
        return(FALSE);
send_to_char("Your limp body hangs impaled on the stalagmite, bleeding and  weezing.\n\r",ch);
send_to_char("After a few hours, the weezing finally stops.\n\r",ch);
send_to_char("YOU ARE DEAD!\n\r",ch);
/*
char_from_room(ch);
char_to_room(ch,2982);
*/
raw_kill(ch,NULL);
return(1);
}


int river_current(struct char_data *ch, int cmd, char *arg)
{
int room;
int toroom=0;
int iWear;

    if(IS_NPC(ch))return(0);
    if (GET_LEVEL(ch) > 31) return(0);
    if(ch->specials.fighting)return(0);
    if(IS_AFFECTED(ch,AFF_FLYING))return(0);
    for(iWear=0;iWear < MAX_WEAR; iWear++)
      if(ch->equipment[iWear])
        if(ch->equipment[iWear]->obj_flags.type_flag==ITEM_FLY)return(0);
    if(number(0,10)<10)return(0);
    room=world[ch->in_room]->number;

	if(room==734) toroom=6216;
	else if(room==6216) toroom=6220;
	else if(room==6220) toroom=6215;
	else if(room==6215) toroom=6219;
	else if(room==6219) toroom=6218;
	else if(room==6218) toroom=6217;
	else if(room==6217) toroom=6221;
	else if(room==6221) toroom=6222;
	else if(room==6222) toroom=6223;
	else if(room==6223) toroom=6225;
	else if(room==6225) toroom=6224;
	else if(room==6224) toroom=735;
    else if((room>=706)&&(room<=725))
		toroom=room+1;
	else if(room==726) toroom=729;
    else if((room>=729)&&(room<=742))
		toroom=room+1;
    else if(room==743) toroom=749;
    else if((room>=749)&&(room<758)) toroom=room+1;
    else if(room==758)toroom=764;
    else if((room>=764)&&(room<=773))toroom=room+1;

    char_from_room(ch);
    char_to_room(ch,toroom);     

    send_to_char("\n\r**THE RIVER CURRENT PULLED YOU DOWNSTREAM!**\n\r",ch);
    act("The current pulls $n next to you.", FALSE, ch,0,0,TO_ROOM);
    do_look(ch, "", 15);
    return(1);
}

int guild(struct char_data *ch, int cmd, char *arg)
{
  char prac_buf[16000];
  int mednumber,nnumber, number, i, percent;

  extern char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];
  extern struct int_app_type int_app[26];

static char *medt_skills[] = {
    "throw",
    "scan",
    "meditate",
    "trap",
    "track",
    "\n"
    };
static char *medw_skills[] = {
    "charge",
    "meditate",
    "\n"
    };

  static char *nw_skills[] = {
    "secondattack",
    "disarm",
    "thirdattack",
    "parry",
    "\n"
    };

  static char *nt_skills[] = {
    "trip",
    "dodge",
    "dual",
    "disarm",
    "\n"
    };
  
  static char *w_skills[] = {
    "kick",  /* No. 50 */
    "bash",
    "rescue",
    "\n"
    };

  static char *t_skills[] = {
    "sneak",   /* No. 45 */
    "hide",
    "steal",
    "backstab",
    "pick",
    "\n"
    };

#if 0
  static char *ex_skills[] = {
    "recall",
    "/n"
    };
#endif
  
  if(GET_LEVEL(ch)>=35)ch->specials.practices=100;

  if ((cmd != 164) && (cmd != 170)) return(FALSE);
  
  for(; *arg==' '; arg++);
  
  switch (GET_CLASS(ch)) {
  case CLASS_MAGIC_USER :{
    if (!*arg) {
      sprintf(prac_buf, "You have got %d practice sessions left.\n\r", ch->specials.practices);
      sprintf(prac_buf, "%sGuild practicing now costs from 1 to 3 (die roll) practice points.\n\r", prac_buf);
      sprintf(prac_buf, "%sYou can practice any of these spells:\n\r",prac_buf);

      for(i=0; *spells[i] != '\n'; i++)
         if (spell_info[i+1].spell_pointer &&
	     (spell_info[i+1].min_level_magic <= GET_LEVEL(ch))) {
 	   sprintf(prac_buf, "%s%s[%s%20s%s][%s%14s%s][%s%4d%s]\n\r", 
		prac_buf,
		BLU(ch), NRM(ch),
		spells[i],
		BLU(ch), NRM(ch),
		how_good(ch->skills[i+1].learned),
		BLU(ch), NRM(ch),
		use_mana(ch, i+1),
		BLU(ch));
         }
      page_string(ch->desc, prac_buf, 1);
      return(TRUE);
    }

    number = old_search_block(arg,0,strlen(arg),spells,FALSE);
    if(number == -1) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
    if (GET_LEVEL(ch) < spell_info[number].min_level_magic) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
    if (ch->specials.practices <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
    if (ch->skills[number].learned >= 95) {
      send_to_char("You are already learned in this area.\n\r", ch);
      return(TRUE);
    }
    
    send_to_char("You Practice for a while...\n\r", ch);
    if(ch->specials.practices>3)
	ch->specials.practices-=dice(1,3);
    else
	if(ch->specials.practices)ch->specials.practices--;

    percent = ch->skills[number].learned+MAX(25,int_app[GET_INT(ch)].learn);
    ch->skills[number].learned = MIN(95, percent);
    
    if (ch->skills[number].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return(TRUE);
    }
    
  } break;

  case CLASS_THIEF: {
    if (!*arg) {
      sprintf(prac_buf,"You have got %d practice sessions left.\n\r",
          ch->specials.practices);
      sprintf(prac_buf, "%sYou can practice any of these skills:\n\r", 
          prac_buf);
      for(i=0; *t_skills[i] != '\n';i++) {
	  sprintf(prac_buf, "%s%s[%s%20s%s][%s%14s%s]\n\r",
	    prac_buf,
	    BLU(ch), NRM(ch),
     	    t_skills[i],
	    BLU(ch), NRM(ch),
  	    how_good(ch->skills[i+45].learned),
	    BLU(ch));
      }
      for(i=0; *nt_skills[i] != '\n';i++) {
	 sprintf(prac_buf, "%s%s[%s%20s%s][%s%14s%s]\n\r",
	    prac_buf,
	    BLU(ch), NRM(ch),
	    nt_skills[i],
	    BLU(ch), NRM(ch),
	    how_good(ch->skills[i+SKILL_TRIP].learned),
	    BLU(ch));
      }
      for(i=0; *medt_skills[i] != '\n';i++) {
	 sprintf(prac_buf, "%s%s[%s%20s%s][%s%14s%s]\n\r",
	    prac_buf,
	    BLU(ch), NRM(ch),
 	    medt_skills[i],
	    BLU(ch), NRM(ch),
	    how_good(ch->skills[i+SKILL_THROW].learned),
	    BLU(ch));
      }
      page_string(ch->desc, prac_buf, 1);
      return(TRUE);
    }
    number = search_block(arg,t_skills,FALSE);
    nnumber = search_block(arg,nt_skills, FALSE);
    mednumber = search_block(arg,medt_skills, FALSE);    
    if((number == -1) && (nnumber == -1) && (mednumber == -1)) {
      send_to_char("You do not know of this skill...\n\r", ch);
      return(TRUE);
    }
    if (ch->specials.practices <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
    if (nnumber == -1 && mednumber == -1)
    {
       if (ch->skills[number+SKILL_SNEAK].learned >= 90) {
          send_to_char("You are already learned in this area.\n\r", ch);
          return(TRUE);
       }
       send_to_char("You Practice for a while...\n\r", ch);
       if(ch->specials.practices>3)
           ch->specials.practices-=dice(1,3);
       else
	   if(ch->specials.practices)ch->specials.practices--;
       percent = ch->skills[number+SKILL_SNEAK].learned +
       MIN(int_app[GET_INT(ch)].learn, 12);
       ch->skills[number+SKILL_SNEAK].learned = MIN(90, percent);
       if (ch->skills[number+SKILL_SNEAK].learned >= 90) {
          send_to_char("You are now learned in this area.\n\r", ch);
          return(TRUE);
       }
    } else if(number == -1 && mednumber == -1)
    {
	if (ch->skills[nnumber+SKILL_TRIP].learned >= 90) {
	  send_to_char("You are already learned in this area.\n\r", ch);
	  return(TRUE);
	}
	send_to_char("You Practice for a while...\n\r", ch);
        if(ch->specials.practices>3)
           ch->specials.practices-=dice(1,3);
        else
	   if(ch->specials.practices)ch->specials.practices--;
	percent = ch->skills[nnumber+SKILL_TRIP].learned +
	MIN(int_app[GET_INT(ch)].learn, 12);
	ch->skills[nnumber+SKILL_TRIP].learned = MIN(90, percent);
	if (ch->skills[nnumber+SKILL_TRIP].learned >= 90) {
	  send_to_char("You are now learned in this area.\n\r", ch);
	  return(TRUE);
        }
    } else if(number == -1 && nnumber == -1)
    {
	if (ch->skills[mednumber+SKILL_THROW].learned >= 90) {
	  send_to_char("You are already learned in this area.\n\r", ch);
	  return(TRUE);
	}
	send_to_char("You Practice for a while...\n\r", ch);
        if(ch->specials.practices>3)
           ch->specials.practices-=dice(1,3);
        else
	   if(ch->specials.practices)ch->specials.practices--;
	percent = ch->skills[mednumber+SKILL_THROW].learned +
	MIN(int_app[GET_INT(ch)].learn, 12);
	ch->skills[mednumber+SKILL_THROW].learned = MIN(90, percent);
	if (ch->skills[mednumber+SKILL_THROW].learned >= 90) {
	  send_to_char("You are now learned in this area.\n\r", ch);
	  return(TRUE);
        }
    }
    
  } break;
    
  case CLASS_CLERIC     :{
    if (!*arg) {
      sprintf(prac_buf, "You have got %d practice sessions left.\n\r", ch->specials.practices);
      sprintf(prac_buf, "%sGuild practicing now costs from 1 to 3 (die roll) practice points.\n\r", prac_buf);
      sprintf(prac_buf, "%sYou can practice any of these spells:\n\r",prac_buf);
      for(i=0; *spells[i] != '\n'; i++)
         if (spell_info[i+1].spell_pointer &&
             (spell_info[i+1].min_level_cleric <= GET_LEVEL(ch))) {
           sprintf(prac_buf, "%s%s[%s%20s%s][%s%14s%s][%s%4d%s]\n\r",
                prac_buf,
		BLU(ch), NRM(ch),
                spells[i],
		BLU(ch), NRM(ch),
                how_good(ch->skills[i+1].learned),
	 	BLU(ch), NRM(ch),
                use_mana(ch, i+1),
		BLU(ch));
         }
      page_string(ch->desc, prac_buf, 1);
      return(TRUE);
    }

    number = old_search_block(arg,0,strlen(arg),spells,FALSE);
    if(number == -1) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
    if (GET_LEVEL(ch) < spell_info[number].min_level_cleric) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
    if (ch->specials.practices <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
    if (ch->skills[number].learned >= 95) {
      send_to_char("You are already learned in this area.\n\r", ch);
      return(TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    if(ch->specials.practices>3)
    ch->specials.practices-=dice(1,3);
    else
	if(ch->specials.practices)ch->specials.practices--;

    
    percent = ch->skills[number].learned+MAX(25,int_app[GET_INT(ch)].learn);
    ch->skills[number].learned = MIN(95, percent);
    
    if (ch->skills[number].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return(TRUE);
    }
  } break;
    
  case CLASS_WARRIOR: {
    if (!*arg) {
      sprintf(prac_buf,"You have got %d practice sessions left.\n\r",
      ch->specials.practices);
      sprintf(prac_buf,"%sYou can practice any of these skills:\n\r", prac_buf);
      for(i=0; *w_skills[i] != '\n';i++) {
          sprintf(prac_buf, "%s%s[%s%20s%s][%s%14s%s]\n\r",
            prac_buf,
            BLU(ch), NRM(ch),
            w_skills[i],
            BLU(ch), NRM(ch),
            how_good(ch->skills[i+SKILL_KICK].learned),
            BLU(ch));
      }
      for(i=0; *nw_skills[i] != '\n';i++) {
         sprintf(prac_buf, "%s%s[%s%20s%s][%s%14s%s]\n\r",
            prac_buf,
            BLU(ch), NRM(ch),
            nw_skills[i],
            BLU(ch), NRM(ch),
            how_good(ch->skills[i+SKILL_SECOND_ATTACK].learned),
            BLU(ch));
      }
      for(i=0; *medw_skills[i] != '\n';i++) {
         sprintf(prac_buf, "%s%s[%s%20s%s][%s%14s%s]\n\r",
            prac_buf,
            BLU(ch), NRM(ch),
            medw_skills[i],
            BLU(ch), NRM(ch),
	    how_good(ch->skills[i+SKILL_CHARGE].learned),
	    BLU(ch));
      }
      page_string(ch->desc, prac_buf, 1);
      return(TRUE);
    }
    number = search_block(arg, w_skills, FALSE);
    nnumber = search_block(arg, nw_skills, FALSE);
    mednumber = search_block(arg, medw_skills, FALSE);
    if((number == -1) && (nnumber == -1 && (mednumber == -1))) {
      send_to_char("You do not have ability to practice this skill!\n\r", ch);
      return(TRUE);
    }
    if (ch->specials.practices <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
    if (nnumber == -1 && mednumber == -1)
      {
    if (ch->skills[number+SKILL_KICK].learned >= 85) {
      send_to_char("You are already learned in this area.\n\r", ch);
      return(TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    if(ch->specials.practices>3)
    ch->specials.practices-=dice(1,3);
    else
	if(ch->specials.practices)ch->specials.practices--;

    
    percent = ch->skills[number+SKILL_KICK].learned +
      MIN(12, int_app[GET_INT(ch)].learn);
    ch->skills[number+SKILL_KICK].learned = MIN(85, percent);
    
    if (ch->skills[number+SKILL_KICK].learned >= 85) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return(TRUE);
    }
      } else if((number == -1) && (mednumber == -1))
    {
	if (ch->skills[nnumber+SKILL_SECOND_ATTACK].learned >= 85) {
	  send_to_char("You are already learned in this area.\n\r", ch);
	  return(TRUE);
	}
	send_to_char("You Practice for a while...\n\r", ch);
        if(ch->specials.practices>3)
            ch->specials.practices-=dice(1,3);
        else
	    if(ch->specials.practices)ch->specials.practices--;
	percent = ch->skills[nnumber+SKILL_SECOND_ATTACK].learned +
	MIN(12, int_app[GET_INT(ch)].learn);
	ch->skills[nnumber+SKILL_SECOND_ATTACK].learned = MIN(85, percent);
	if (ch->skills[nnumber+SKILL_SECOND_ATTACK].learned >= 85) {
	  send_to_char("You are now learned in this area.\n\r", ch);
	  return(TRUE);
        }
      }else if((number==-1)&&(nnumber==-1))
    {
	if (ch->skills[mednumber+SKILL_CHARGE].learned >= 85) {
	  send_to_char("You are already learned in this area.\n\r", ch);
	  return(TRUE);
	}
	send_to_char("You Practice for a while...\n\r", ch);
        if(ch->specials.practices>3)	
            ch->specials.practices-=dice(1,3);
        else
	    if(ch->specials.practices)ch->specials.practices--;
	percent = ch->skills[mednumber+SKILL_CHARGE].learned +
	MIN(12, int_app[GET_INT(ch)].learn);
	ch->skills[mednumber+SKILL_CHARGE].learned = MIN(85, percent);
	if (ch->skills[mednumber+SKILL_CHARGE].learned >= 85) {
	  send_to_char("You are now learned in this area.\n\r", ch);
	  return(TRUE);
        }
      }

  } break;
  }
  return (TRUE);
}



int train(struct char_data *ch, int cmd, char *arg)
{
    char    buf[MAX_STRING_LENGTH];
    sbyte   *pAbility       = NULL;
    sbyte   *pTmpAbility    = NULL;
    int     cost            = 35;


    /*
     * Check for right command.
     * Strip white space on arg.
     */
    if ( cmd != 165 )
	return FALSE;

    while ( *arg == ' ' )
	arg++;

    if ( *arg == '\0' )
    {
	sprintf( buf, "You have %d practice sessions left.\n\r",
	    ch->specials.practices );
	send_to_char( buf, ch );
	send_to_char("Training costs you 35 Practice points except for..\n\r",ch);
	send_to_char("The following costs 20 practice points for classes.\n\r",ch);
	send_to_char("WARRIOR str sta, MAGE int, CLERIC wis, THIEF dex sta.\n\r",ch);
	arg = "foo";
    }

    if ( !str_cmp( arg, "str" ) )
    {
	if ( GET_CLASS(ch) == CLASS_WARRIOR )
	    cost    = 20;
	pAbility    = &ch->abilities.str;
	pTmpAbility = &ch->tmpabilities.str;
    }

    else if ( !str_cmp( arg, "int" ) )
    {
	if ( GET_CLASS(ch) == CLASS_MAGIC_USER )
	    cost    = 20;
	pAbility    = &ch->abilities.intel;
	pTmpAbility = &ch->tmpabilities.intel;
    }

    else if ( !str_cmp( arg, "wis" ) )
    {
	if ( GET_CLASS(ch) == CLASS_CLERIC )
	    cost    = 20;
	pAbility    = &ch->abilities.wis;
	pTmpAbility = &ch->tmpabilities.wis;
    }

    else if ( !str_cmp( arg, "con" ) )
    {
	pAbility    = &ch->abilities.con;
	pTmpAbility = &ch->tmpabilities.con;
    }

    else if ( !str_cmp( arg, "dex" ) )
    {
	if ( GET_CLASS(ch) == CLASS_THIEF )
	    cost    = 20;
	pAbility    = &ch->abilities.dex;
	pTmpAbility = &ch->tmpabilities.dex;
    }

    else if ( !str_cmp( arg, "sta" ) )
    {
	if ( GET_CLASS(ch) == CLASS_WARRIOR )
	    cost    = 20;
	if ( GET_CLASS(ch) == CLASS_THIEF )
	    cost    = 20;
	pAbility    = &ch->abilities.sta;
	pTmpAbility = &ch->tmpabilities.sta;
    }

    else
    {
	send_to_char( "You can train in: str int wis con dex sta.\n\r", ch );
	return TRUE;
    }

    if ( cost > ch->specials.practices )
    {
	send_to_char( "You don't have enough practices.\n\r", ch );
	return TRUE;
    }

    if ( *pAbility >= 18 || *pTmpAbility >= 18 )
    {
	send_to_char( "That ability is already at maximum.\n\r", ch );
	return TRUE;
    }

    if(ch->specials.practices>=cost)
	ch->specials.practices    -= cost;
    else{
	send_to_char("Sorry not enough practices points left.\n\r",ch);
	return TRUE;
    }
    *pAbility                 += 1;
    *pTmpAbility                    += 1;
    send_to_char( "Your ability increases!\n\r", ch );

    return TRUE;
}

/* ********************************************************************
*  General special procedures for mobiles                                      *
******************************************************************** */

/* SOCIAL GENERAL PROCEDURES

If first letter of the command is '!' this will mean that the following
command will be executed immediately.

"G",n      : Sets next line to n
"g",n      : Sets next line relative to n, fx. line+=n
"m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
"w",n      : Wake up and set standing (if possible)
"c<txt>",n : Look for a person named <txt> in the room
"o<txt>",n : Look for an object named <txt> in the room
"r<int>",n : Test if the npc in room number <int>?
"s",n      : Go to sleep, return false if can't go sleep
"e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
	     contents of the **thing
"E<txt>",n : Send <txt> to person pointed to by thing
"B<txt>",n : Send <txt> to room, except to thing
"?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
	     Will as usual advance one line upon sucess, and change
	     relative n lines upon failure.
"O<txt>",n : Open <txt> if in sight.
"C<txt>",n : Close <txt> if in sight.
"L<txt>",n : Lock <txt> if in sight.
"U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */
void exec_social(struct char_data *npc, char *cmd, int next_line,
		 int *cur_line, void **thing)
{
  bool ok;

  void do_move(struct char_data *ch, char *argument, int cmd);
  void do_open(struct char_data *ch, char *argument, int cmd);
  void do_lock(struct char_data *ch, char *argument, int cmd);
  void do_unlock(struct char_data *ch, char *argument, int cmd);
  void do_close(struct char_data *ch, char *argument, int cmd);

  if (GET_POS(npc) == POSITION_FIGHTING)
    return;

  ok = TRUE;

  switch (*cmd) {

    case 'G' :
      *cur_line = next_line;
      return;

    case 'g' :
      *cur_line += next_line;
      return;

    case 'e' :
      act(cmd+1, FALSE, npc, *thing, *thing, TO_ROOM);
      break;

    case 'E' :
      act(cmd+1, FALSE, npc, 0, *thing, TO_VICT);
      break;

    case 'B' :
      act(cmd+1, FALSE, npc, 0, *thing, TO_NOTVICT);
      break;

    case 'm' :
      do_move(npc, "", *(cmd+1)-'0'+1);
      break;

    case 'w' :
      if (GET_POS(npc) != POSITION_SLEEPING)
	ok = FALSE;
      else
	GET_POS(npc) = POSITION_STANDING;
      break;

    case 's' :
      if (GET_POS(npc) <= POSITION_SLEEPING)
	ok = FALSE;
      else
	GET_POS(npc) = POSITION_SLEEPING;
      break;

    case 'c' :  /* Find char in room */
      *thing = get_char_room_vis(npc, cmd+1);
      ok = (*thing != 0);
      break;

    case 'o' : /* Find object in room */
      *thing = get_obj_in_list_vis(npc, cmd+1, world[npc->in_room]->contents);
      ok = (*thing != 0);
      break;

    case 'r' : /* Test if in a certain room */
      ok = (npc->in_room == atoi(cmd+1));
      break;

    case 'O' : /* Open something */
      do_open(npc, cmd+1, 0);
      break;

    case 'C' : /* Close something */
      do_close(npc, cmd+1, 0);
      break;

    case 'L' : /* Lock something  */
      do_lock(npc, cmd+1, 0);
      break;

    case 'U' : /* UnLock something  */
      do_unlock(npc, cmd+1, 0);
      break;

    case '?' : /* Test a random number */
      if (atoi(cmd+1) <= number(1,100))
	ok = FALSE;
      break;

    default:
      break;
  }  /* End Switch */

  if (ok)
    (*cur_line)++;
  else
    (*cur_line) += next_line;
}



void npc_steal(struct char_data *ch,struct char_data *victim)
{
    int gold;

    if(IS_NPC(victim) || (GET_LEVEL(victim)>31)) return;

    if (AWAKE(victim) && (number(0,GET_LEVEL(ch)) == 0)) {
	act("You discover that $n has $s hands in your wallet.",
	    FALSE,ch,0,victim,TO_VICT);
	act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
    } else {
	/* Steal some gold coins */
	gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
	if (gold > 0) {
	    GET_GOLD(ch) += gold;
	    GET_GOLD(victim) -= gold;
	}
    }
}


int snake(struct char_data *ch, int cmd, char *arg)
{
    void cast_poison( byte level, struct char_data *ch, char *arg, int type,
      struct char_data *tar_ch, struct obj_data *tar_obj );

   if(cmd) return FALSE;

    if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
    
    if ( ch->specials.fighting && 
	(ch->specials.fighting->in_room == ch->in_room) &&
	number(0 , 99) < 3 * GET_LEVEL(ch) )
	{
	    act("You bite $N!", 1, ch, 0, ch->specials.fighting, TO_CHAR);
	    act("$n bites $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
	    act("$n bites you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
	    cast_poison( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
		 ch->specials.fighting, 0);
	    return TRUE;
	}
    return FALSE;
}

int thief(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *cons=NULL;

   if(cmd) return FALSE;

    if(GET_POS(ch)!=POSITION_STANDING)return FALSE;

    for(cons = world[ch->in_room]->people; cons; cons = cons->next_in_room )
	if((!IS_NPC(cons)) && (GET_LEVEL(cons)<32) && (number(1,5)==1))
	    npc_steal(ch,cons); 

    return TRUE;
}


int red_dragon(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict=NULL;

    if (cmd) return FALSE;

    if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
    
    if(!ch->specials.fighting) return FALSE;

    /* Find a dude to do evil things upon ! */

    for (vict = world[ch->in_room]->people; vict; vict = vict->next_in_room )
	if (vict->specials.fighting==ch ){
	  act("$n breathes a cone of fire, enveloping you.",1, ch, 0, 0, TO_VICT);
	  cast_fire_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	}

    if (!vict){
	act("$n's fiery breath scorches the area.",
           1, ch, 0, 0, TO_NOTVICT);
	cast_fire_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
		 ch->specials.fighting, 0);
      }

    return TRUE;
}

int white_dragon(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict=NULL;

    return FALSE;

    if(cmd) return FALSE;

    if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
    
    if(!ch->specials.fighting) return FALSE;

    /* Find a dude to do evil things upon ! */
    for (vict = world[ch->in_room]->people; vict; vict = vict->next_in_room )
		if (vict->specials.fighting==ch && number(0,1)==0){
	  act("$n breathes frost.",1, ch, 0, 0, TO_ROOM);
	  cast_frost_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		}

	if (!vict)
	  if (number(0,1) == 0){
	act("$n breathes frost.",1, ch, 0, 0, TO_ROOM);
	cast_frost_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
		  ch->specials.fighting, 0);
	  }

    return TRUE;
}

int black_dragon(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;

    return FALSE;

    if(cmd) return FALSE;

    if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
    
    if(!ch->specials.fighting) return FALSE;

    /* Find a dude to do evil things upon ! */

    for (vict = world[ch->in_room]->people; vict; vict = vict->next_in_room )
	if (vict->specials.fighting==ch && number(0,2)==0)
	    break;

    if (!vict)
      if (number(0,1) == 0)
	{
	vict = ch->specials.fighting;
	if (vict == NULL)
	  return FALSE;
      }
      else
	return FALSE;

    act("$n breathes acid.",1, ch, 0, 0, TO_ROOM);
    cast_acid_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

    return TRUE;
}

int blue_dragon(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;

    return FALSE;

    if(cmd) return FALSE;

    if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
    
    if(!ch->specials.fighting) return FALSE;

    /* Find a dude to do evil things upon ! */

    for (vict = world[ch->in_room]->people; vict; vict = vict->next_in_room)
	if (vict->specials.fighting==ch && number(0,2)==0)
	    break;

    if (!vict)
      if (number(0,1) == 0)
	{
	vict = ch->specials.fighting;
	if (vict == NULL)
	  return FALSE;
      }     
      else
	return FALSE;

    act("$n breathes lightning.",1, ch, 0, 0, TO_ROOM);
    cast_lightning_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

    return TRUE;
}

int green_dragon(struct char_data *ch, int cmd, char *arg)
{
    return FALSE;

    if ( cmd ) return FALSE;

    if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
    
    if(!ch->specials.fighting) return FALSE;

    if(number(0,1)==0) return FALSE;

    act("$n breathes gas.",1, ch, 0, 0, TO_ROOM);
    cast_gas_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);

    return TRUE;
}

int brass_dragon(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;

    return FALSE;

    if ( cmd == 4 && ch->in_room == 5065 )
    {
	act( "The brass dragon says '$n isn't invited'",
	    FALSE, ch, 0, 0, TO_ROOM );
	send_to_char( "The brass dragon says 'you're not invited'\n\r", ch );
	return TRUE;
    }

    if ( cmd )
	return FALSE;

    if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;

    if (!ch->specials.fighting) return FALSE;

    if (number(0,1)==0)
    {
	act("$n breathes gas.",1,ch, 0, 0, TO_ROOM);
	cast_gas_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
	return TRUE;
    }

    for (vict = world[ch->in_room]->people; vict; vict = vict->next_in_room)
	if (vict->specials.fighting==ch && number(0,1)==0)
	    break;

    if (!vict)
	if (number(0,1) == 0)
	{
	    vict = ch->specials.fighting;
	    if (vict == NULL)
		return FALSE;
	}
	else
	    return FALSE;

    act("$n breathes lightning.",1, ch, 0, 0, TO_ROOM);
    cast_lightning_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

    return TRUE;

}
  
int block_way_suites(struct char_data *ch, int cmd, char *arg)
{
int room, to_room;
struct char_data *k=NULL;
char flag,people;
    flag=0;people=0;
    if(cmd!=1&&cmd!=2&&cmd!=3&&cmd!=4&&cmd!=5&&cmd!=6)
	return FALSE;
    room=ch->in_room;
    if(!world[room]->dir_option[cmd-1])return FALSE;
    to_room=world[room]->dir_option[cmd-1]->to_room;
    if(world[to_room]->level_restriction==1){
        for (k = world[to_room]->people; k; k = k->next_in_room){
            if (IS_MOB(k))continue;
	    people++;
	    if(k==ch->master)
		flag=1;
        }
        if(!people||flag)return FALSE;

        act( "That suite is taken, sorry.",
            FALSE, ch, 0, k, TO_CHAR );
        return TRUE;
    }
    return FALSE;

}

/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */
/* blocks way to any room level restrcited to 1*/
int block_way(struct char_data *ch, int cmd, char *arg)
{
int room, to_room;
struct char_data *k=NULL;

    if (cmd>6 || cmd<1)
	return FALSE;
    room=ch->in_room;
    if(!world[room]->dir_option[cmd-1])return FALSE;
    to_room=world[room]->dir_option[cmd-1]->to_room;
    if(world[to_room]->level_restriction==1){
        for (k = world[room]->people; k; k = k->next_in_room)
          if ( IS_MOB(k) )
              if (mob_index[k->nr].func)
		  break;
        if(!k)return FALSE;

        if(!IS_NPC(k)&&GET_LEVEL(k)>33){
          act( "$N bows before $n and allows $m to pass.",
            FALSE, k, 0, ch, TO_ROOM );
          act( "$N bows before you, allowing you to pass.",
            FALSE, k, 0, ch, TO_CHAR );
          return FALSE;
        }
        act( "$N shoves $n back, and blocks $s way.",
            FALSE, ch, 0, k, TO_ROOM );
        act( "$N shoves you back, and blocks your way.",
            FALSE, ch, 0, k, TO_CHAR );
        return TRUE;
    }
    return FALSE;

}
int guild_guard(struct char_data *ch, int cmd, char *arg)
{
    if (cmd>6 || cmd<1)
	return FALSE;

    if ( ( GET_CLASS(ch) != CLASS_MAGIC_USER
	&& ch->in_room == 3017 && cmd == 3 )
    ||   ( GET_CLASS(ch) != CLASS_CLERIC
	&& ch->in_room == 3004 && cmd == 1 )
    ||   ( GET_CLASS(ch) != CLASS_THIEF
	&& ch->in_room == 3027 && cmd == 2 )
    ||   ( GET_CLASS(ch) != CLASS_WARRIOR
	&& ch->in_room == 3021 && cmd == 2 )
	)
    {
	act( "The guard humiliates $n, and blocks $s way.",
	    FALSE, ch, 0, 0, TO_ROOM );
	send_to_char(
	    "The guard humiliates you, and blocks your way.\n\r", ch );
	return TRUE;
    }

    return FALSE;

}

int puff(struct char_data *ch, int cmd, char *arg)
{
    void do_say(struct char_data *ch, char *argument, int cmd);

    if (cmd)
	return(0);

    switch (number(0, 160))
    {
	case 5:
	    do_say(ch, "Hey buddie, gotta bone?", 0);
	   return(1);
	case 2:
	    do_say(ch, "Hey man, been to Daddy O's today?", 0);
	    return(1);
	case 8:
	    do_say(ch, "Any of you guys seen any cute female pups around here?", 0);
	    return(1);
	case 9:
	    do_say(ch, "Like WOOF!, man.", 0);
	    return(1);
	case 10:
	    do_emote(ch, "scratches his ear.",0);
	    return(1);
	case 11:
	    do_emote(ch, "chases his tail.",0);
	    return(1);
	case 12:
	    do_emote(ch, "looks at you.",0);
	    return(1);
	case 13:
	    do_emote(ch, "piddles on the ground.",0);
	    return(1);

	default:
	    return(0);
    }
}

int prisoner(struct char_data *ch, int cmd, char *arg)
{
    void do_say(struct char_data *ch, char *argument, int cmd);

    if (cmd)
	return(0);

    switch (number(0, 60))
    {
	case 5:
	    do_say(ch, "Please take my life and free me from this hell!", 0);
	   return(1);
	case 2:
	    do_emote(ch, "struggles to get free but only causes herself more pain.",0);
	    return(1);
	case 8:
	    do_emote(ch, "looks at you with pleading eyes.",0);
	    return(1);
	case 9:
	    do_emote(ch, "tries to pick the lock on the shackles and gets zapped",0);
	    return(1);
	case 10:
	    do_emote(ch, "sighs sadly.",0);
	    return(1);
	case 11:
	    do_emote(ch, "begins to cry uncontrolably.",0);
	    return(1);
	case 12:
	    do_say(ch, "If I could only kill Gith for what he has done!",0);
	    return(1);
	case 13:
	    do_emote(ch, "mumbles to herself quietly.",0);
	    return(1);

	default:
	    return(0);
    }
}


int towncrier(struct char_data *ch, int cmd, char *arg)
{
char buf[MAX_STRING_LENGTH];
 
    void do_say(struct char_data *ch, char *argument, int cmd);
 
    if (cmd)
        return(0);
 
    if(GET_POS(ch) == POSITION_FIGHTING){
        if(number(0,10))
           do_say(ch,"I declare that I die entirely too often!!",0);
        return(1);
    }
 
    switch (number(0, 20))
    {
        case 1:
            sprintf(buf, "I declare that Medievia now has %d rooms and %d zones!", number_of_rooms+4000000, 1+number_of_zones);
            do_say(ch, buf, 0);
           return(1);
        case 2:
            do_say(ch, "Hear Ye! Hear Ye!  Be it known that on the first day of\n\rthe fourth month in the year of the Dragon, nineteen hundred and ninety three \n\r Medievia declared itself a sovereign land!", 0);
            return(1);
        case 3:
            sprintf(buf, "Let it be proclaimed throughout the land that there\n\rare now %d fair citizens in Medievia", number_of_players());
            do_say(ch, buf, 0);
            return(1);
        case 4:
            sprintf(buf, "Hear Ye!  Hear Ye!  Be it known that there have been %ld passersby in Medievia since the 11th month of the year of the Dragon, nineteen hundred and ninety three", connect_count);
            do_say(ch,buf,0);
            return(1);
        case 5:
        case 6:
        case 7:
            sprintf(buf, "Its %d o'clock and all's well...",
            ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)));
            do_say(ch,buf,0);
            return(1);
        default:
            return(0);
    }
}


int janitor(struct char_data *ch, int cmd, char *arg)
{
    struct obj_data *i=NULL;

    if (cmd || !AWAKE(ch))
	return(FALSE);

    for (i = world[ch->in_room]->contents; i; i = i->next_content) {
	if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) && 
      ((i->obj_flags.type_flag == ITEM_DRINKCON) ||
	  (i->obj_flags.cost <= 10))) {
	    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);

	    obj_from_room(i);
	    obj_to_char(i, ch);
	    return(TRUE);
	}
    }
    return(FALSE);
}

int cityguard(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *tch=NULL, *evil=NULL;
    struct char_data *next_tch=NULL;
    int max_evil;
    char buf[MAX_STRING_LENGTH];

    if (cmd || !AWAKE(ch) || (GET_POS(ch) == POSITION_FIGHTING))
	return (FALSE);

    max_evil = 300;
    evil = 0;

    for (tch=world[ch->in_room]->people; tch; tch = next_tch) {
      next_tch=tch->next_in_room;
      if(IS_SET(tch->specials.affected_by, AFF_KILLER)) {
	sprintf(buf,
 "%s is a KILLER! PROTECT THE INNOCENT!  BANZAI!!! CHARGE!!! ARARARAGGGHH!",
     GET_NAME(tch));
	hit(ch, tch, TYPE_UNDEFINED);
	return (TRUE);
      } else
	if (IS_SET(tch->specials.affected_by, AFF_THIEF)) {
	  sprintf(buf,
 "%s is a THIEF! PROTECT THE INNOCENT!  BANZAI!!! CHARGE!!! ARARARAGGGHH!",
     GET_NAME(tch));
	  hit(ch, tch, TYPE_UNDEFINED);
	  return (TRUE);
	}
      if (tch->specials.fighting) {
	if ((GET_ALIGNMENT(tch) < max_evil) &&
		(GET_LEVEL(tch) > 6) &&
		(IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
	  max_evil = GET_ALIGNMENT(tch);
	  evil = tch;
	}
      }
    }

    if (evil && !IS_EVIL(evil->specials.fighting))
    {
	act(
 "$n screams 'PROTECT THE INNOCENT!  BANZAI!!! CHARGE!!! ARARARAGGGHH!'",
     FALSE, ch, 0, 0, TO_ROOM);
	hit(ch, evil, TYPE_UNDEFINED);
	return(TRUE);
    }

    return(FALSE);
}

int raven(struct char_data *ch, int cmd, char *arg)
{
    void do_say(struct char_data *ch, char *argument, int cmd);

    if (cmd)
	return(0);

    switch (number(0, 15))
    {
	case 3:
	    do_say(ch, "Nevermore", 0);
	   return(1);

	default:
	    return(0);
    }
}

/*********************************************************************
*  Special procedures for shops                                      *
*********************************************************************/

int pet_shops(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH], pet_name[MAX_STRING_LENGTH];
    int pet_room;
    struct char_data *pet=NULL;

    pet_room = 100;

    if (cmd==59) { /* List */
	send_to_char("Available pets are:\n\r", ch);
	for(pet = world[pet_room]->people; pet; pet = pet->next_in_room) {
	    sprintf(buf, "%8d - %s\n\r",
		3*GET_EXP(pet), pet->player.short_descr);
	    send_to_char(buf, ch);
	}
	return(TRUE);
    } else if (cmd==56) { /* Buy */
 	if(is_formed(ch)&&ch->master!=ch){
	    send_to_char("You must be your own leader.\n\r",ch);
	    return(TRUE);
	}
	arg = one_argument(arg, buf);
	arg = one_argument(arg, pet_name);
	/* Pet_Name is for later use when I feel like it */

	if (!(pet = get_char_room(buf, pet_room))) {
	    send_to_char("There is no such pet!\n\r", ch);
	    return(TRUE);
	}

	if (GET_GOLD(ch) < (GET_EXP(pet)*3)) {
	    send_to_char("You don't have enough gold!\n\r", ch);
	    return(TRUE);
	}

	GET_GOLD(ch) -= GET_EXP(pet)*3;

	/*
	 * Should be some code here to defend against weird monsters
	 * getting loaded into the pet shop back room.  -- Furey
	 */
	pet = read_mobile(pet->nr, REAL);
	GET_EXP(pet) = 0;
	SET_BIT(pet->specials.affected_by, AFF_CHARM);

	if (*pet_name) {
	    sprintf(buf,"%s %s", pet->player.name, pet_name);
	    pet->player.name = my_free(pet->player.name);
	    pet->player.name = str_dup(buf);     

	    sprintf( buf,
	"%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
	      pet->player.description, pet_name);
	    pet->player.description = my_free(pet->player.description);
	    pet->player.description = str_dup(buf);
	}

	char_to_room(pet, ch->in_room);
	put_in_formation(ch->master,pet);

	/* Be certain that pet's can't get/carry/use/weild/wear items */
	IS_CARRYING_W(pet) = 1000;
	IS_CARRYING_N(pet) = 100;

	send_to_char("May you enjoy your pet.\n\r", ch);
	act("$n bought $N as a pet.",FALSE,ch,0,pet,TO_ROOM);

	return(TRUE);
    }

    /* All commands except list and buy */
    return(FALSE);
}


int magic_cloud(struct char_data *ch, int cmd, char *arg)
{

    switch(cmd){
	case 1:
	    send_to_char("The section of the cloud you are standing on seperates
\rfrom the cloud body. You find yourself being whisked away at high velocity!
\rYou watch the land of Medievia scroll underneath you as you head towards the
\rLabrynth. The clouds slows down and places you gently down on your feet.
\r", ch);	
	    act("$n steps on a section of the cloud and is whisked away!",
			TRUE,ch,0,0,TO_ROOM);
	    char_from_room(ch);
   	    char_to_room(ch, 300);
	    act("A magical cloud sweeps down and $n steps off!",TRUE,ch,0,0,TO_ROOM);	
	    return(TRUE);
	    break;
	case 2:
	    send_to_char("The section of the cloud you are standing on seperates
\rfrom the cloud body. You find yourself being whisked away at high velocity!
\rYou watch the land of Medievia scroll underneath you as you head towards the
\rCrossroads. The clouds slows down and places you gently down on your feet.
\r", ch);	
	    act("$n steps on a section of the cloud and is whisked away!",
			TRUE,ch,0,0,TO_ROOM);
	    char_from_room(ch);
   	    char_to_room(ch, 909);
	    act("A magical cloud sweeps down and $n steps off!",
			TRUE,ch,0,0,TO_ROOM);	
	    return(TRUE);
	    break;
	case 3:
	    send_to_char("The section of the cloud you are standing on seperates
\rfrom the cloud body. You find yourself being whisked away at high velocity!
\rYou watch the land of Medievia scroll underneath you as you head towards the
\rGraveyard. The clouds slows down and places you gently down on your feet.
\r", ch);	
	    act("$n steps on a section of the cloud and is whisked away!",
			TRUE,ch,0,0,TO_ROOM);
	    char_from_room(ch);
   	    char_to_room(ch, 1007);
	    act("A magical cloud sweeps down and $n steps off!",
			TRUE,ch,0,0,TO_ROOM);	
	    return(TRUE);
	    break;
	case 4:
	    send_to_char("The section of the cloud you are standing on seperates
\rfrom the cloud body. You find yourself being whisked away at high velocity!
\rYou watch the land of Medievia scroll underneath you as you head towards
\rCastle Square. The clouds slows down and places you gently down on your feet.
\r", ch);	
	    act("$n steps on a section of the cloud and is whisked away!",
			TRUE,ch,0,0,TO_ROOM);
	    char_from_room(ch);
   	    char_to_room(ch, 1);
	    act("A magical cloud sweeps down and $n steps off!",
			TRUE,ch,0,0,TO_ROOM);	
	    return(TRUE);
	    break;
	case 5:
	    send_to_char("The section of the cloud you are standing on seperates
\rfrom the cloud body. You find yourself being whisked away at high velocity!
\rYou watch the land of Medievia scroll underneath you as you head towards the
\rTemple. The clouds slows down and places you gently down on your feet.
\r",
ch);	
	    act("$n steps on a section of the cloud and is whisked away!",
			TRUE,ch,0,0,TO_ROOM);
	    char_from_room(ch);
   	    char_to_room(ch, 929);
	    act("A magical cloud sweeps down and $n steps off!",
			TRUE,ch,0,0,TO_ROOM);	
	    return(TRUE);
	    break;
	case 6:
	    send_to_char("The section of the cloud you are standing on seperates
\rfrom the cloud body. You find yourself being whisked away at high velocity!
\rYou watch the land of Medievia scroll underneath you as you head towards 
\rTear! The clouds slows down and places you gently down on your feet.
\r",
ch);	
	    act("$n steps on a section of the cloud and is whisked away!",
			TRUE,ch,0,0,TO_ROOM);
	    char_from_room(ch);
   	    char_to_room(ch, 2600);
	    act("A magical cloud sweeps down and $n steps off!",
			TRUE,ch,0,0,TO_ROOM);	
	    return(TRUE);
	    break;
	default:
	    return(FALSE);
	    break;
    }

}


int atm_machines_helper(struct char_data *ch, int cmd, char *arg,char *filename)
{
unsigned int deposit=0;
unsigned int balance=0;
unsigned int withdrawal=0;
FILE *fh;
char buf[MAX_STRING_LENGTH];
unsigned int numdone=0;

    if((fh=med_open(filename, "r"))!=NULL){
		numdone=fread(&balance,sizeof(unsigned int), 1, fh);
		med_close(fh);
    }
    one_argument(arg, buf);

    if (cmd==102) { /* balance */
		sprintf(buf, "Your balance is %d Gold coins\n\r", balance);
		send_to_char(buf, ch);
		return(TRUE);
    } else if (cmd==101) { /* Withdrawal */

		withdrawal=(unsigned int)atoi(arg);
		if(withdrawal>balance){
	    	send_to_char("Insufficient funds.\n\r", ch);
	    	return FALSE;
        }
		balance-=withdrawal;
		GET_GOLD(ch)+=withdrawal;
	    if((fh=med_open(filename, "w"))!=NULL){
			numdone=fwrite(&balance, sizeof(unsigned int),1,fh);
			med_close(fh);
	    	if(numdone<1)
		    	send_to_char("Transmission error to bank!", ch);
	    }
		if(balance<=0)unlink(filename);
		send_to_char("Transaction completed.\n\r", ch);
		sprintf(log_buf,"[%d]-Withdrawn, leaving a balance of [%d].\n\r",withdrawal,balance);
		send_to_char(log_buf,ch);
		sprintf(log_buf,"%s withdrawals %d, for a balance of %d.",GET_NAME(ch),withdrawal,balance);
		log_hd(log_buf);
		act("$n withdraws some money.",FALSE,ch,0,0,TO_ROOM);
		return(TRUE);
    } else if(cmd==100) { /* deposit */
		if(GET_LEVEL(ch)<2){
	    	send_to_char("You must be level 2+ to use our services.\n\r",ch);
	    	return(TRUE);
		}
		deposit=(unsigned int)atoi(buf);
		if(GET_GOLD(ch)<0)GET_GOLD(ch)=0;
		if(deposit>GET_GOLD(ch)){
	    	send_to_char("You realize you don't have that many coins.\n\r", ch);
	    	return(TRUE);
		}	
		if((balance+deposit)>1000000000){
	    	send_to_char("SORRY 1,000,000,000 GOLD TOTAL is the limit\n\r",ch);
	    	return(TRUE);
		}
		GET_GOLD(ch)-=deposit;
		balance+=(deposit-(int)(deposit*.036));
	    if((fh=med_open(filename, "w"))!=NULL){
			numdone=fwrite(&balance, sizeof(unsigned int),1,fh);
			med_close(fh);
	    	if(numdone<1)
		    	send_to_char("Transmission error to bank!", ch);
	    }
		sprintf(log_buf,"Transaction completed for a charge of [%d] coins.\n\r",(int)(deposit*.036));
		send_to_char(log_buf, ch);
		sprintf(log_buf,"[%d]-Deposited, leaving a balance of [%d].\n\r",deposit,balance);
		send_to_char(log_buf,ch);
		sprintf(log_buf,"%s deposits %d, for a balance of %d.",GET_NAME(ch),deposit,balance);
		log_hd(log_buf);
		act("$n deposits some money.",FALSE,ch,0,0,TO_ROOM);
		return(TRUE);
    }

    /* All commands except list and buy */
    return(FALSE);
}


int atm_machines(struct char_data *ch, int cmd, char *arg)
{
char filename[MAX_STRING_LENGTH];
char name[MAX_STRING_LENGTH],clan[MAX_INPUT_LENGTH], *arg2;

	if(IS_NPC(ch))
	{
		send_to_char("We do not serve monsters!",ch);
		return(FALSE);
	}
    if(cmd!=102&&cmd!=101&&cmd!=100)
    	return(FALSE);
    arg2=one_argument(arg,clan);
    if(!str_cmp("clan",clan)){
		if(!ch->specials.clan)
			return(FALSE);
    	sprintf(filename,"../clan/clan_%d.atm",ch->specials.clan);
	    return(atm_machines_helper(ch,cmd,arg2,filename));
    }
    strcpy(name,GET_NAME(ch));
    sprintf(filename,"%s/%c/%s.atm", SAVE_DIR, LOWER(name[0]),name);
    return(atm_machines_helper(ch,cmd,arg,filename));
}


int thief_guild(struct char_data *ch, int cmd, char *arg)
{

char buf[MAX_STRING_LENGTH];
int mednumber,nnumber, number, i, percent;
extern struct int_app_type int_app[26];

static char *medt_skills[] = {
    "throw",
    "scan",
    "meditate",
    "trap",
    "\n"
    };

static char *nt_skills[] = {
    "trip",
    "dodge",
    "dual",
    "disarm",
    "\n"
    };
  
static char *t_skills[] = {
    "sneak",   /* No. 45 */
    "hide",
    "steal",
    "backstab",
    "pick",
    "\n"
    };

if(GET_LEVEL(ch)>=35)ch->specials.practices=100;

if ((cmd != 164) && (cmd != 170)) return(FALSE);
  
for(; *arg==' '; arg++);
 
if( (GET_CLASS(ch)!=CLASS_THIEF)
    && (!IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF))
	)
    {
    send_to_char("You watch the lesson but can't comprehend a thing.\n\r", ch);
    return(TRUE);
    }

if (!*arg)
	{
	sprintf(buf,"You have %d practice sessions left.\n\r",
	ch->specials.practices);
	send_to_char(buf, ch);
	send_to_char("You can practice any of these skills:\n\r", ch);
	for(i=0; *t_skills[i] != '\n';i++)
		{
		send_to_char(t_skills[i], ch);
		send_to_char(how_good(ch->skills[i+45].learned), ch);
		send_to_char("\n\r", ch);
		}
	for(i=0; *nt_skills[i] != '\n';i++)
		{
		send_to_char(nt_skills[i], ch);
		send_to_char(how_good(ch->skills[i+SKILL_TRIP].learned), ch);
		send_to_char("\n\r", ch);
		}
	for(i=0; *medt_skills[i] != '\n';i++)
		{
		send_to_char(medt_skills[i], ch);
		send_to_char(how_good(ch->skills[i+SKILL_THROW].learned), ch);
		send_to_char("\n\r", ch);
		}
	return(TRUE);
	}
number = search_block(arg,t_skills,FALSE);
nnumber = search_block(arg,nt_skills, FALSE);
mednumber = search_block(arg,medt_skills, FALSE);    
if((number == -1) && (nnumber == -1) && (mednumber == -1))
	{
	send_to_char("You do not know of this skill...\n\r", ch);
	return(TRUE);
    }
if (ch->specials.practices <= 0)
	{
	send_to_char("You do not seem to be able to practice now.\n\r", ch);
	return(TRUE);
    }

if (nnumber == -1 && mednumber == -1)
	{
	if (ch->skills[number+SKILL_SNEAK].learned >= 90)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	send_to_char("You Practice for a while...\n\r", ch);
	if(ch->specials.practices>3)
		ch->specials.practices-=dice(1,3);
	else
		if(ch->specials.practices)ch->specials.practices--;
	percent = ch->skills[number+SKILL_SNEAK].learned +
			MIN(int_app[GET_INT(ch)].learn, 12);
	ch->skills[number+SKILL_SNEAK].learned = MIN(90, percent);
	if (ch->skills[number+SKILL_SNEAK].learned >= 90)
		{
		send_to_char("You are now learned in this area.\n\r", ch);
		return(TRUE);
		}
    }
else if(number == -1 && mednumber == -1)
	{
	if (ch->skills[nnumber+SKILL_TRIP].learned >= 90)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	send_to_char("You Practice for a while...\n\r", ch);
	if(ch->specials.practices>3)
		ch->specials.practices-=dice(1,3);
	else
		if(ch->specials.practices)ch->specials.practices--;
	percent = ch->skills[nnumber+SKILL_TRIP].learned +
				MIN(int_app[GET_INT(ch)].learn, 12);
	ch->skills[nnumber+SKILL_TRIP].learned = MIN(90, percent);
	if (ch->skills[nnumber+SKILL_TRIP].learned >= 90)
		{
		send_to_char("You are now learned in this area.\n\r", ch);
		return(TRUE);
		}
    }
else if(number == -1 && nnumber == -1)
    {
	if (ch->skills[mednumber+SKILL_THROW].learned >= 90)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	send_to_char("You Practice for a while...\n\r", ch);
	if(ch->specials.practices>3)
		ch->specials.practices-=dice(1,3);
	else
		if(ch->specials.practices)ch->specials.practices--;
	percent = ch->skills[mednumber+SKILL_THROW].learned +
		MIN(int_app[GET_INT(ch)].learn, 12);
	ch->skills[mednumber+SKILL_THROW].learned = MIN(90, percent);
	if (ch->skills[mednumber+SKILL_THROW].learned >= 90)
		{
		send_to_char("You are now learned in this area.\n\r", ch);
		return(TRUE);
		}
    }
return (TRUE);    
}


int mage_guild(struct char_data *ch, int cmd, char *arg)
{

char buf[MAX_STRING_LENGTH];
int number, i, percent;

extern char *spells[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern struct int_app_type int_app[26];

  
if(GET_LEVEL(ch)>=35)ch->specials.practices=100;

if ((cmd != 164) && (cmd != 170)) return(FALSE);
  
for(; *arg==' '; arg++);
 
if( (GET_CLASS(ch)!=CLASS_MAGIC_USER)
	&& (!IS_SET(ch->player.multi_class,MULTI_CLASS_MAGIC_USER))
	)
	{
	send_to_char("You could watch the lesson but wouldn't comprehend a thing.\n\r", ch);
	return(TRUE);
	}

if (!*arg)
	{
	sprintf(buf,"You have %d practice sessions left.\n\r",
		 ch->specials.practices);
    send_to_char(buf, ch);
    send_to_char("Guild Practicing between 1 and 3(Die Roll) practice points,\n\r",ch);
    send_to_char("You can practice any of these spells:\n\r", ch);
    for(i=0; *spells[i] != '\n'; i++)
    if (spell_info[i+1].spell_pointer
		&& (spell_info[i+1].min_level_magic <= GET_LEVEL(ch))) 
		{
		sprintf(buf," %20s",spells[i]);
		send_to_char(buf, ch);
		sprintf(buf,"%14s",how_good(ch->skills[i+1].learned));
		send_to_char(buf, ch);
		sprintf(buf," %4d",use_mana(ch,i+1));
		send_to_char(buf, ch);
		send_to_char("\n\r", ch);
		}
	return(TRUE);
    }

number = old_search_block(arg,0,strlen(arg),spells,FALSE);
if(number == -1) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
if (GET_LEVEL(ch) < spell_info[number].min_level_magic) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
if (ch->specials.practices <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
if (ch->skills[number].learned >= 95) {
      send_to_char("You are already learned in this area.\n\r", ch);
      return(TRUE);
    }
    
send_to_char("You Practice for a while...\n\r", ch);
if(ch->specials.practices>3)
	ch->specials.practices-=dice(1,3);
else
	if(ch->specials.practices)ch->specials.practices--;

percent = ch->skills[number].learned+MAX(25,int_app[GET_INT(ch)].learn);
ch->skills[number].learned = MIN(95, percent);
    
if (ch->skills[number].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return(TRUE);
    }
return (TRUE);
}

 
int warrior_guild(struct char_data *ch, int cmd, char *arg)
{

char buf[MAX_STRING_LENGTH];
int mednumber,nnumber, number, i, percent;
extern struct int_app_type int_app[26];

static char *medw_skills[] = {
    "charge",
    "meditate",
    "\n"
    };

static char *nw_skills[] = {
    "secondattack",
    "disarm",
    "thirdattack",
    "parry",
    "\n"
    };

static char *w_skills[] = {
    "kick",  /* No. 50 */
    "bash",
    "rescue",
    "\n"
    };

if(GET_LEVEL(ch)>=35)ch->specials.practices=100;

if ((cmd != 164) && (cmd != 170)) return(FALSE);
  
for(; *arg==' '; arg++);

if( (GET_CLASS(ch)!=CLASS_WARRIOR)
    && (!IS_SET(ch->player.multi_class,MULTI_CLASS_WARRIOR))
	)
    {
    send_to_char("You watch the lesson but don't comprehend a thing.\n\r", ch);
    return(TRUE);
    }
  
if (!*arg)
	{
	sprintf(buf,"You have %d practice sessions left.\n\r",
		ch->specials.practices);
	send_to_char(buf, ch);
	send_to_char("You can practice any of these skills:\n\r", ch);
	for(i=0; *w_skills[i] != '\n';i++)
		{
		send_to_char(w_skills[i], ch);
		send_to_char(how_good(ch->skills[i+SKILL_KICK].learned), ch);
		send_to_char("\n\r", ch);
		}
	for(i=0; *nw_skills[i] != '\n';i++)
		{
		send_to_char(nw_skills[i],ch);
		send_to_char(how_good(ch->skills[i+SKILL_SECOND_ATTACK].learned), ch);
		send_to_char("\n\r", ch);
		}
	for(i=0; *medw_skills[i] != '\n';i++)
		{
		send_to_char(medw_skills[i],ch);
		send_to_char(how_good(ch->skills[i+SKILL_CHARGE].learned), ch);
		send_to_char("\n\r", ch);
		}
	return(TRUE);
    }
number = search_block(arg, w_skills, FALSE);
nnumber = search_block(arg, nw_skills, FALSE);
mednumber = search_block(arg, medw_skills, FALSE);
if((number == -1) && (nnumber == -1 && (mednumber == -1))) {
      send_to_char("You cannot practice this skill!\n\r", ch);
      return(TRUE);
    }
if (ch->specials.practices <= 0)
	{
	send_to_char("You do not seem to be able to practice now.\n\r", ch);
	return(TRUE);
    }
if (nnumber == -1 && mednumber == -1)
	{
    if (ch->skills[number+SKILL_KICK].learned >= 85)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	send_to_char("You Practice for a while...\n\r", ch);
    if(ch->specials.practices>3)
		ch->specials.practices-=dice(1,3);
    else
		if(ch->specials.practices)ch->specials.practices--;

    percent = ch->skills[number+SKILL_KICK].learned +
		MIN(12, int_app[GET_INT(ch)].learn);
    ch->skills[number+SKILL_KICK].learned = MIN(85, percent);
    
    if (ch->skills[number+SKILL_KICK].learned >= 85)
		{
		send_to_char("You are now learned in this area.\n\r", ch);
		return(TRUE);
		}
	}
else if((number == -1) && (mednumber == -1))
    {
	if (ch->skills[nnumber+SKILL_SECOND_ATTACK].learned >= 85)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	send_to_char("You Practice for a while...\n\r", ch);
	if(ch->specials.practices>3)
		ch->specials.practices-=dice(1,3);
	else
	    if(ch->specials.practices)ch->specials.practices--;
	percent = ch->skills[nnumber+SKILL_SECOND_ATTACK].learned +
		MIN(12, int_app[GET_INT(ch)].learn);
	ch->skills[nnumber+SKILL_SECOND_ATTACK].learned = MIN(85, percent);
	if (ch->skills[nnumber+SKILL_SECOND_ATTACK].learned >= 85)
		{
		send_to_char("You are now learned in this area.\n\r", ch);
		return(TRUE);
		}
	}
else if((number==-1)&&(nnumber==-1))
    {
	if (ch->skills[mednumber+SKILL_CHARGE].learned >= 85)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	send_to_char("You Practice for a while...\n\r", ch);
	if(ch->specials.practices>3)	
		ch->specials.practices-=dice(1,3);
	else
	    if(ch->specials.practices)ch->specials.practices--;
	percent = ch->skills[mednumber+SKILL_CHARGE].learned +
		MIN(12, int_app[GET_INT(ch)].learn);
	ch->skills[mednumber+SKILL_CHARGE].learned = MIN(85, percent);
	if (ch->skills[mednumber+SKILL_CHARGE].learned >= 85)
		{
		send_to_char("You are now learned in this area.\n\r", ch);
		return(TRUE);
        }
	}

return (TRUE);
}


int cleric_guild(struct char_data *ch, int cmd, char *arg)
{

char buf[MAX_STRING_LENGTH];
int number, i, percent;

extern char *spells[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern struct int_app_type int_app[26];

 if(GET_LEVEL(ch)>=35)ch->specials.practices=100;

 if ((cmd != 164) && (cmd != 170)) return(FALSE);
  
 for(; *arg==' '; arg++);
 
if( (GET_CLASS(ch)!=CLASS_CLERIC)
    && (!IS_SET(ch->player.multi_class,MULTI_CLASS_CLERIC))
	)
    {
    send_to_char("You watch the lesson but don't comprehend a thing.\n\r", ch);
    return(TRUE);
    }
 
if (!*arg)
	{
    sprintf(buf,"You have got %d practice sessions left.\n\r", ch->specials.practices);
    send_to_char(buf, ch);
    send_to_char("You can practice any of these spells:\n\r", ch);
    for(i=0; *spells[i] != '\n'; i++)
    	if (spell_info[i+1].spell_pointer &&
			(spell_info[i+1].min_level_cleric <= GET_LEVEL(ch)))
			{
			sprintf(buf," %20s ",spells[i]);
			send_to_char(buf, ch);
			sprintf(buf,"%14s",how_good(ch->skills[i+1].learned));
			send_to_char(buf, ch);
			sprintf(buf," %4d",use_mana(ch,i+1));
			send_to_char(buf, ch);
			send_to_char("\n\r", ch);
			}
	return(TRUE);
    }

number = old_search_block(arg,0,strlen(arg),spells,FALSE);
if(number == -1) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
if (GET_LEVEL(ch) < spell_info[number].min_level_cleric) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
if (ch->specials.practices <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
if (ch->skills[number].learned >= 95) {
      send_to_char("You are already learned in this area.\n\r", ch);
      return(TRUE);
    }
send_to_char("You Practice for a while...\n\r", ch);
if(ch->specials.practices>3)
    ch->specials.practices-=dice(1,3);
else
	if(ch->specials.practices)ch->specials.practices--;

    
percent = ch->skills[number].learned+MAX(25,int_app[GET_INT(ch)].learn);
ch->skills[number].learned = MIN(95, percent);
    
if (ch->skills[number].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return(TRUE);
    }
return (TRUE);
}

void ravine(struct char_data *ch, int to_room)
{
int dam, weight, spin;

char *action[] = {
	"careen into",
	"bounce off",
	"land on",
	"smack into",
	"ricochet off",
	"slide into"
	};
char *hard_place[] = {
	"boulder",
	"root",
	"sapling",
	"sharp rock",
	"branch",
	"patch of thorns"
	};
char *body_part[] = {
	"back",
	"leg",
	"kneecap",
	"shin",
	"head",
	"elbow"
	};
char *wound[] = {
	"break",
	"bruise",
	"crack",
	"shatter",
	"fracture",
	"scrape"
	};

/* invis gods arent' affected.  Let visible ones be affected so they can
 * find the room you land in.
 */
if ( (GET_LEVEL(ch) >=32) && IS_AFFECTED(ch,AFF_INVISIBLE) )
	return;

act( "$n loses $s balance and slides down a steep ravine towards the river.\n\r", FALSE, ch, NULL, NULL, TO_ROOM);
char_from_room(ch);
char_to_room(ch, to_room);
global_color=32;
send_to_char("You lose your balance and slide down a steep ravine towards the river.\n\r",ch);
global_color=0;

if( !IS_NPC(ch) && (GET_LEVEL(ch) > 31) )
	{
	act( "$n dives gracefully into the river from the slope above.", FALSE, ch, NULL, NULL, TO_ROOM);
	global_color=34;
	send_to_char("You slide down the slope but regain control, diving gracefully into the river.\n\r",ch);
	global_color=0;
	return;
	}

while(GET_HIT(ch) > 0)
	{
	if (number(1,100) < ((GET_DEX(ch) - 12)*4) )
		{
		sprintf(log_buf, "<%dhp %dm %dmv>\n\r",
            GET_HIT(ch), GET_MANA(ch), GET_MOVE(ch));
		send_to_char(log_buf,ch);
		spin=number(2,6);
		global_color=34;
		sprintf(log_buf,"Nimbly regaining control, you flip %d times in the air, and dive into the river.\n\r",spin);
		send_to_char(log_buf,ch);
		sprintf(log_buf,"$n flips %d times in the air, diving gracefully into the river.\n\r",spin);
 		act( log_buf, FALSE, ch, NULL, NULL, TO_ROOM);
		global_color=0;
		sprintf(log_buf, "<%dhp %dm %dmv>\n\r",
			GET_HIT(ch), GET_MANA(ch), GET_MOVE(ch));
		send_to_char(log_buf,ch);
		return;
		}
	else
		{
		sprintf(log_buf, "<%dhp %dm %dmv>\n\r",
            GET_HIT(ch), GET_MANA(ch), GET_MOVE(ch));
		send_to_char(log_buf,ch);
		global_color=31;
		sprintf(log_buf,"You %s a %s and %s your %s.\n\r",
			action[number(0,5)], hard_place[number(0,5)],
			wound[number(0,5)], body_part[number(0,5)] );
		send_to_char(log_buf,ch);
		global_color=0;
		weight = IS_CARRYING_W(ch) + GET_WEIGHT(ch);
		dam = (int)(number((int)(weight/3),weight));
		dam /= 10;
		GET_HIT(ch) -= dam;
		}
	}

sprintf(log_buf, "<%dhp %dm %dmv>\n\r",
	GET_HIT(ch), GET_MANA(ch), GET_MOVE(ch));
send_to_char(log_buf,ch);
global_color=31;
send_to_char("You limply bounce off a rock and splash into the river.\n\r",ch);
act( "$n's limp body bounces off a rock and splashes into the river.", FALSE, ch, NULL, NULL, TO_ROOM);
global_color=0;
sprintf(log_buf, "<%dhp %dm %dmv>\n\r",
	GET_HIT(ch), GET_MANA(ch), GET_MOVE(ch));
send_to_char(log_buf,ch);
raw_kill(ch,ch);
}


int belt_quest(struct char_data *ch, int cmd, char *arg)
{
void ravine(struct char_data *ch, int to_room);

ravine(ch,3350);
return(FALSE);
}

int moongate(struct char_data *ch, int cmd, char *arg)
{
    if(cmd!=28)return(FALSE);

    send_to_char("You step into the swirling silver portal...\n\r",ch);
    send_to_char("Your vision blackens for a moment. When it returns, you find yourself\n\r",ch);
    send_to_char("drifting through an endless gray mist.  A bright ball of light in the\n\r",ch);
    send_to_char("distance rushes towards you, engulfing your body, blinding you.\n\r",ch);
    send_to_char("Your vision returns, and you find yourself back on the material plane.\n\r",ch); 
    act("$n steps into the silvery moongate; $s body shimmers briefly, then vanishes. \n\r",TRUE,ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch,475);
    act("A pillar of flame erupts before the altar.  As is fades, $n materializes in its place.\n\r",TRUE,ch,0,0,TO_ROOM);
    do_look(ch,"",9);
    return(TRUE);
}

int feeders(struct char_data *ch, int cmd, char *arg)
{
    if((cmd != 2) || (IS_NPC(ch)))return(FALSE);
    act("$n leaves east through the narrow tunnel.  You see a bright flash seconds after $e leaves.\n\r",TRUE,ch,0,0,TO_ROOM);
    send_to_char("You feel dizzy and disoriented!\n\r",ch);
    send_to_char("Your whole being begins to fade in and out.\n\r",ch);
    send_to_char("You pass out.\n\r",ch);
    send_to_char("You wake up feeling groggy, peering about you...\n\r",ch); 
    act("$n moves east, farther into the narrow tunnel.\n\r",TRUE,ch,0,0,TO_ROOM);
    if(world[ch->in_room]->number==15053){
      char_from_room(ch);
      char_to_room(ch,15021);
      do_look(ch,"",9);
    }
    if(world[ch->in_room]->number==15051){
      char_from_room(ch);
      char_to_room(ch,15031);
      do_look(ch,"",9);
    }
    act("$n appears in the corner of the cavern with a bright flash and fading crackling sound.\n\r",TRUE,ch,0,0,TO_ROOM);
    return(TRUE);
}

int illusionary_room(struct char_data *ch, int cmd, char *arg)
{
    if((world[ch->in_room]->number==6324)&&(cmd==2)){
      act("$n enters the workshop.  A faint scream seems to echo from far away.\n\r",TRUE,ch,0,0,TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, 6331);
      do_look(ch,"",9);
      send_to_char("Just as your foot is about to touch the floor, you notice\n\r",ch);
      send_to_char("everything in the room is slightly transparent. In utter shock,\n\r",ch);
      send_to_char("you pass straight through the floor and fall into darkness...\n\r",ch);
      send_to_char("With a loud *SMACK*, you land in frigid, black waters.\n\r",ch); 
      act("$n has a look of total disbelief and horror as $e falls straight through the floor!\n\r",TRUE,ch,0,0,TO_ROOM);
      char_from_room(ch);
      char_to_room(ch,2096);
      act("A loud scream echos throughout the chamber.  $n falls from the darkness\n\r",TRUE,ch,0,0,TO_ROOM);
      act("above and lands painfully in the icy waters.\n\r",TRUE,ch,0,0,TO_ROOM);
      GET_HIT(ch)-=dice(5,12);
      if(GET_HIT(ch)<1){
        send_to_char("You land in a pool of frigid water. A stalagmite hidden just beneath the surface\n\r",ch);
        send_to_char("of the lake pierces your eye, impaling your brain and shatters your skull.\n\r",ch);
        raw_kill(ch,0);
        return(TRUE);
      }
      else{
        do_look(ch,"",9);
        return(TRUE);
      }
    }
    else if((world[ch->in_room]->number==6330)&&(cmd==3)){
      act("$n enters the workshop.  A faint scream seems to echo from far away.\n\r",TRUE,ch,0,0,TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, 6331);
      do_look(ch,"",9);
      send_to_char("Just as your foot is about to touch the floor, you notice\n\r",ch);
      send_to_char("everything in the room is slightly transparent. In utter shock,\n\r",ch);
      send_to_char("you pass straight through the floor and fall into darkness...\n\r",ch);
      send_to_char("With a loud *SMACK*, you land in frigid, black waters.\n\r",ch); 
      act("$n has a look of total disbelief and horror as $e falls straight through the floor!\n\r",TRUE,ch,0,0,TO_ROOM);
      char_from_room(ch);
      char_to_room(ch,2096);
      act("A loud scream echos throughout the chamber.  $n falls from the darkness\n\r",TRUE,ch,0,0,TO_ROOM);
      act("above and lands painfully in the icy waters.\n\r",TRUE,ch,0,0,TO_ROOM);
      GET_HIT(ch)-=dice(5,12);
      if(GET_HIT(ch)<1){
        send_to_char("You land in a pool of frigid water. A stalagmite hidden just beneath the surface\n\r",ch);
        send_to_char("of the lake pierces your eye, impaling your brain and shatters your skull.\n\r",ch);
        raw_kill(ch,0);
        act("$n floats face down in the water...\n\r",TRUE,ch,0,0,TO_ROOM);
        return(TRUE);
      }
      else{
        do_look(ch,"",9);
        return(TRUE);
      }
    }
   else
     return(FALSE);
}

int pit_traps(struct char_data *ch, int cmd, char *arg)
{
  if(world[ch->in_room]->number==15067 && !IS_NPC(ch)){
    if(number(1,20) > GET_DEX(ch)){
      act("$n lets out a brief scream as $e disappears beneath the oil.\n\r",TRUE,ch,0,0,TO_ROOM);
      char_from_room(ch);
      send_to_char("You slip on the oil, loosing your footing...\n\r",ch);
      send_to_char("Darkness engulfs you as you fall downward through a rough hewn tunnel.\n\r",ch);
      char_to_room(ch, 1159);
      GET_HIT(ch) -= dice(3,10);
      if(GET_HIT(ch) < 1){
        send_to_char("You land hard on the cold stone floor, snapping your neck on a stalagmite.\n\r",ch);
        raw_kill(ch,0);
        act("$n falls from a dark hole in the ceiling, landing hard on the cavern floor.\n\r",TRUE,ch,0,0,TO_ROOM);
        act("You hear a loud *crunch* as $s neck hits a protruding rock, breaking it.\n\r",TRUE,ch,0,0,TO_ROOM);
        return(TRUE);
      }
      else{
        send_to_char("You land painfully on the stone floor, bruised and cut.\n\r",ch);
        act("$n falls from a dark hole in the ceiling, landing hard on the cavern floor.\n\r",TRUE,ch,0,0,TO_ROOM);
        do_look(ch,"",9);
        return(TRUE);
      }
    }
  }
  else if(world[ch->in_room]->number==15060 && !IS_NPC(ch)){
    if(number(1,20) > GET_DEX(ch)){
      act("$n lets out a brief scream as $e disappears beneath the oil.\n\r",TRUE,ch,0,0,TO_ROOM);
      char_from_room(ch);
      send_to_char("You slip on the oil, loosing your footing...\n\r",ch);
      send_to_char("Darkness engulfs you as you fall downward through a rough hewn tunnel.\n\r",ch);
      char_to_room(ch, 1152);
      GET_HIT(ch) -= dice(3,10);
      if(GET_HIT(ch) < 1){
        send_to_char("You land hard on the cold stone floor, snapping your neck on a stalagmite.\n\r",ch);
        raw_kill(ch,0);
        act("$n falls from a dark hole in the ceiling, landing hard on the cavern floor.\n\r",TRUE,ch,0,0,TO_ROOM);
        act("You hear a loud *crunch* as $s neck hits a protruding rock, breaking it.\n\r",TRUE,ch,0,0,TO_ROOM);
        return(TRUE);
      }
      else{
        send_to_char("You land painfully on the stone floor, bruised and cut.\n\r",ch);
        act("$n falls from a dark hole in the ceiling, landing hard on the cavern floor.\n\r",TRUE,ch,0,0,TO_ROOM);
        do_look(ch,"",9);
        return(TRUE);
      }
    }
  }
  else if(world[ch->in_room]->number==973 && !IS_NPC(ch)){
    if(number(1,20) > GET_DEX(ch)){
      act("$n lets out a brief scream as $e disappears beneath the oil.\n\r",TRUE,ch,0,0,TO_ROOM);
      char_from_room(ch);
      send_to_char("You slip on the oil, loosing your footing...\n\r",ch);
      send_to_char("Darkness engulfs you as you fall downward through a rough hewn tunnel.\n\r",ch);
      char_to_room(ch, 2066);
      GET_HIT(ch) -= dice(3,10);
      if(GET_HIT(ch) < 1){
        send_to_char("You feel your legs exit the shoot and you begin falling.  A jagged, protruding\n\r",ch);
        send_to_char("rock shatters your face and skull just before your head exits the tunnel.\n\r",ch);
        raw_kill(ch,0);
        act("$n slides out of a dark hole in the cavern wall, landing in the cold water.\n\r",TRUE,ch,0,0,TO_ROOM);
        act("$n floats motionless, face down, in the water.\n\r",TRUE,ch,0,0,TO_ROOM);
        return(TRUE);
      }
      else{
        send_to_char("You slam into the walls of the shoot painfully before falling out the other end,\n\r",ch);
        send_to_char("landing in a pool of icy waters.\n\r",ch);
        act("$n slides out of a dark hole in the cavern wall, landing in the cold water.\n\r",TRUE,ch,0,0,TO_ROOM);
        do_look(ch,"",9);
        return(TRUE);
      }
    }
  }
  else if((world[ch->in_room]->number==3053 ||
           world[ch->in_room]->number==3054 ||
           world[ch->in_room]->number==3058 ||
           world[ch->in_room]->number==3061) && !IS_NPC(ch)){
    if(number(1,500) < IS_CARRYING_W(ch)){
      act("The floor crumbles under $n's feet and $e falls screaming into the blackness of the pit.\n\r",TRUE,ch,0,0,TO_ROOM);
      char_from_room(ch);
      send_to_char("The floor under your feet crumbles and falls away...\n\r",ch);
      send_to_char("Deserately you grab for something to hold onto, but it's too late.\n\r",ch);
      send_to_char("You plummet into the blackness of the pit, splattering powerfully on the\n\r",ch);
      send_to_char("hard rocks miles below.\n\r",ch);
      char_to_room(ch, 3302);
      if(GET_LEVEL(ch)<32){
        act("$n falls from the darkness above, slamming into the hard stone floor.. Dead!\n\r",TRUE,ch,0,0,TO_ROOM);
        raw_kill(ch,0);
        return(TRUE);
      }
      else{
        act("$n slowly floats down to the rocky cavern floor.\n\r",TRUE,ch,0,0,TO_ROOM);
        return(TRUE);
      }
    }
  }
  return(FALSE);
}

int silver_shrine(struct char_data *ch, int cmd, char *arg)
{
int room;
    if(cmd!=28)return(FALSE);

    send_to_char("The room around you slowly dematerializes.\n\r",ch);
    send_to_char("You float through a silvery, misty void for a few moments.\n\r",ch);
    send_to_char("The mist clears and you find yourself in a forest clearing.\n\r",ch);
    act("$n steps into the portal and vanishes.\n\r",TRUE,ch,0,0,TO_ROOM);
    if(world[ch->in_room]->number==813)room=3596;
    else room=813;
    char_from_room(ch);
    char_to_room(ch,room);
    do_look(ch,"",9);
    return(TRUE);
}

int waterfall(struct char_data *ch, int cmd, char *arg)
{
    if((world[ch->in_room]->number==774)&&(cmd==6)){
      act("$n allows $mself to be pulled over the waterfall's edge.  You hear $s\n\r",TRUE,ch,0,0,TO_ROOM);
      act("helpless screams of terror as $e nears the bottom.\n\r",TRUE,ch,0,0,TO_ROOM);
      send_to_char("You are thrown powerfully over the waterfall's edge and fall helplessly.\n\r",ch);
      char_from_room(ch);
      char_to_room(ch, 775);
      if(GET_LEVEL(ch)>31){
        send_to_char("You float harmlessly to the ground below.\n\r", ch);
        do_look(ch, "", 9);
        act("$n floats down from the top of the falls.\n\r",TRUE,ch,0,0,TO_ROOM);
        return(TRUE);
      }
      else if(number(1,4)<2){
        act("$n screams helplessly as $e falls from the top of the waterfall.\n\r",TRUE,ch,0,0,TO_ROOM);
        act("Landing on an unforgiving rock, $s body splits into several pieces from.\n\r",TRUE,ch,0,0,TO_ROOM);
        act("the powerful impact.  $n has died a gruesome death.\n\r",TRUE,ch,0,0,TO_ROOM);
        send_to_char("You land on an unforgiving rock below, your body shatters from the impact.\n\r", ch);
        send_to_char("You die instantly.\n\r", ch);
        raw_kill(ch,0);
        return(TRUE);
      }
      else{
        act("$n screams helplessly as $e falls from the top of the waterfall.\n\r",TRUE,ch,0,0,TO_ROOM);
        GET_HIT(ch) -= dice(100,3);
        if(GET_HIT(ch)<0){
          act("Landing on an unforgiving rock, $s body splits into several pieces from\n\r",TRUE,ch,0,0,TO_ROOM);
          act("the powerful impact.  $n has died a gruesome death.\n\r",TRUE,ch,0,0,TO_ROOM);
          send_to_char("You land on an unforgiving rock below, your body shatters from the impact.\n\r", ch);
          send_to_char("You die instantly.\n\r", ch);
          raw_kill(ch,0);
          return(TRUE);
        }
        else{
          act("$e lands in the water with a loud *SMACK* and is forced under the.\n\r",TRUE,ch,0,0,TO_ROOM);
          act("surface.  A moment later, $e comes back up, gasping for air.\n\r",TRUE,ch,0,0,TO_ROOM);
          send_to_char("You land painfully in the water and are forced under by the powerful current.\n\r", ch);
          send_to_char("You slam against the river floor and quickly resurface, gasping for air.\n\r", ch);
          do_look(ch,"",9);
          return(TRUE);
        }
      }
    }
   else
     return(FALSE);
}
