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

void prac_skill(struct char_data *ch, int numb, int maxlearn);
char *how_good(int percent);
char *how_smart(int percent);
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
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern put_obj_in_store(struct obj_data *obj, struct char_data *ch, FILE *fpsave);
extern bool in_a_shop(struct char_data *ch);
extern int top_of_world;
extern int top_of_zone_table;

int thief_guild(struct char_data *ch, int cmd, char *arg)
{

char buf[MAX_STRING_LENGTH];
int mednumber,nnumber, number, i;

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
        sprintf(buf,"%27s",how_good(ch->skills[i+45].learned));
        send_to_char(buf, ch);
        send_to_char(how_smart(ch->skills[i+1].recognise),ch);
		send_to_char("\n\r", ch);
		}
	for(i=0; *nt_skills[i] != '\n';i++)
		{
		send_to_char(nt_skills[i], ch);
        sprintf(buf,"%27s",how_good(ch->skills[i+SKILL_TRIP].learned));
        send_to_char(buf, ch);
        send_to_char(how_smart(ch->skills[i+1].recognise),ch);
		send_to_char("\n\r", ch);
		}
	for(i=0; *medt_skills[i] != '\n';i++)
		{
		send_to_char(medt_skills[i], ch);
        sprintf(buf,"%27s",how_good(ch->skills[i+SKILL_THROW].learned));
        send_to_char(buf, ch);
        send_to_char(how_smart(ch->skills[i+1].recognise),ch);
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
	prac_skill(ch,number+SKILL_SNEAK,90);
    }
else if(number == -1 && mednumber == -1)
	{
	if (ch->skills[nnumber+SKILL_TRIP].learned >= 90)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	prac_skill(ch,number+SKILL_TRIP,90);
    }
else if(number == -1 && nnumber == -1)
    {
	if (ch->skills[mednumber+SKILL_THROW].learned >= 90)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	prac_skill(ch,number+SKILL_THROW,90);
    }
return (TRUE);    
}


int mage_guild(struct char_data *ch, int cmd, char *arg)
{

char buf[MAX_STRING_LENGTH];
int number, i;

extern char *spells[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];

  
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
    send_to_char("You can practice any of these spells:\n\r", ch);
    for(i=0; *spells[i] != '\n'; i++)
    if (spell_info[i+1].spell_pointer
		&& (spell_info[i+1].min_level_magic <= GET_LEVEL(ch))) 
		{
        sprintf(buf," %20s ",spells[i]);
        send_to_char(buf, ch);
        sprintf(buf," %4d",use_mana(ch,i+1));
        send_to_char(buf, ch);
        sprintf(buf,"%27s",how_good(ch->skills[i+1].learned));
        send_to_char(buf, ch);
        send_to_char(how_smart(ch->skills[i+1].recognise),ch);
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
prac_skill(ch,number,95);
return(TRUE);
}

 
int warrior_guild(struct char_data *ch, int cmd, char *arg)
{

char buf[MAX_STRING_LENGTH];
int mednumber,nnumber, number, i;

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
        sprintf(buf,"%27s",how_good(ch->skills[i+SKILL_KICK].learned));
        send_to_char(buf, ch);
        send_to_char(how_smart(ch->skills[i+1].recognise),ch);
		send_to_char("\n\r", ch);
		}
	for(i=0; *nw_skills[i] != '\n';i++)
		{
		send_to_char(nw_skills[i],ch);
        sprintf(buf,"%27s",how_good(ch->skills[i+SKILL_SECOND_ATTACK].learned));
        send_to_char(buf, ch);
        send_to_char(how_smart(ch->skills[i+1].recognise),ch);
		send_to_char("\n\r", ch);
		}
	for(i=0; *medw_skills[i] != '\n';i++)
		{
		send_to_char(medw_skills[i],ch);
        sprintf(buf,"%27s",how_good(ch->skills[i+SKILL_CHARGE].learned));
        send_to_char(buf, ch);
        send_to_char(how_smart(ch->skills[i+1].recognise),ch);
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
	prac_skill(ch,number+SKILL_KICK,85);
	}
else if((number == -1) && (mednumber == -1))
    {
	if (ch->skills[nnumber+SKILL_SECOND_ATTACK].learned >= 85)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	prac_skill(ch,number+SKILL_SECOND_ATTACK,85);
	}
else if((number==-1)&&(nnumber==-1))
    {
	if (ch->skills[mednumber+SKILL_CHARGE].learned >= 85)
		{
		send_to_char("You are already learned in this area.\n\r", ch);
		return(TRUE);
		}
	prac_skill(ch,number+SKILL_CHARGE,85);
	}
return (TRUE);
}


int cleric_guild(struct char_data *ch, int cmd, char *arg)
{

char buf[MAX_STRING_LENGTH];
int number, i;

extern char *spells[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];

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
            sprintf(buf," %4d",use_mana(ch,i+1));
            send_to_char(buf, ch);
            sprintf(buf,"%27s",how_good(ch->skills[i+1].learned));
            send_to_char(buf, ch);
            send_to_char(how_smart(ch->skills[i+1].recognise),ch);
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
prac_skill(ch,number,95);
return(TRUE);
}

void do_practice(struct char_data *ch, char *arg, int cmd)
{
  /* Call "guild" with a null string for an argument.
     This displays the character's skills. */

if (arg[0] != '\0')
    {
    send_to_char("You can only practice in a guild.\n\r", ch);
    }
else if (GET_CLASS(ch)==CLASS_CLERIC)
    cleric_guild (ch, cmd, "");
else if (GET_CLASS(ch)==CLASS_THIEF)
    thief_guild(ch, cmd, "");
else if (GET_CLASS(ch)==CLASS_WARRIOR)
    warrior_guild(ch, cmd, "");
else 
    mage_guild (ch, cmd, "");

}

char *how_good(int percent)
{
if (percent <= 0)
    return ( " (Never Tried)");
if (percent <= 1)
    return ( " (Pitiful)");
if (percent <= 3)
    return ( " (Woefully Inadequate)");
if (percent <= 5)
    return ( " (Inadequate)");
if (percent <= 8)
    return ( " (Beginning Apprentice)");
if (percent <= 10)
    return ( " (Lower Apprentice)");
if (percent <= 12)
    return ( " (Apprentice)");
if (percent <= 14)
    return ( " (Skilled Apprentice)");
if (percent <= 16)
    return ( " (Very Skilled Apprentice)");
if (percent <= 18)
    return ( " (Talented Apprentice)");
if (percent <= 20)
    return ( " (Beginning Disciple)");
if (percent <= 22)
    return ( " (Lower Disciple)");
if (percent <= 24)
    return ( " (Disciple)");
if (percent <= 26)
    return ( " (Skilled Disciple)");
if (percent <= 28)
    return ( " (Very Skilled Disciple)");
if (percent <= 30)
    return ( " (Talented Disciple)");
if (percent <= 32)
    return ( " (Greater Disciple)");
if (percent <= 34)
    return ( " (Skilled Greater Disciple)");
if (percent <= 36)
    return ( " (Beginning Veteran)");
if (percent <= 38)
    return ( " (Lower Veteran)");
if (percent <= 40)
    return ( " (Veteran)");
if (percent <= 42)
    return ( " (Skilled Veteran)");
if (percent <= 44)
    return ( " (Beginning Master)");
if (percent <= 46)
    return ( " (Lower Master)");
if (percent <= 48)
    return ( " (Master)");
if (percent <= 50)
    return ( " (Skilled Master)");
if (percent <= 52)
    return ( " (Very Skilled Master)");
if (percent <= 54)
    return ( " (Beginning Grand Master)");
if (percent <= 56)
    return ( " (Lower Grand Master)");
if (percent <= 58)
    return ( " (Grand Master)");
if (percent <= 60)
    return ( " (Skilled Grand Master)");
if (percent <= 62)
    return ( " (Very Skilled Grand Master)");
if (percent <= 64)
    return ( " (Beginning Supremity)");
if (percent <= 66)
    return ( " (Lower Supremity)");
if (percent <= 68)
    return ( " (Supremity)");
if (percent <= 70)
    return ( " (Skilled Supremity)");
if (percent <= 72)
    return ( " (Very Skilled Supremity)");
if (percent <= 74)
    return ( " (Beginning Highest Supremity)");
if (percent <= 76)
    return ( " (Lower Highest Supremity)");
if (percent <= 78)
    return ( " (Highest Supremity)");
if (percent <= 80)
    return ( " (Skilled Highest Supremity)");
if (percent <= 82)
    return ( " (Glorious High Supremity)");
if (percent <= 84)
    return ( " (Beginning Guildmaster)");
if (percent <= 86)
    return ( " (Lower Guildmaster)");
if (percent <= 88)
    return ( " (Guildmaster)");
if (percent <= 90)
    return ( " (High Guildmaster)");
if (percent <= 93)
    return ( " (Grand Guildmaster)");
if (percent <= 95)
    return ( " (High Grand Guildmaster)");
if (percent <= 99)
    return ( " (Only God Is Your Master)");
return ( " (Godly Ability)");
}

char *how_smart(int percent)
{
if (percent <= 0)
	return("");
if (percent <= 15)
    return ( " [Go Get Another Hint]");
if (percent <= 25)
    return ( " [Almost Stopped Learning]");
if (percent <= 35)
    return ( " [Feeling Uninspired]");
if (percent <= 50)
    return ( " [Still Learning]");
if (percent <= 65)
    return ( " [Steadily Progressing]");
if (percent <= 80)
    return ( " [Feeling Inspired]");
if (percent <= 95)
    return ( " [Constantly Improving]");
return ( " [Can't Stop Learning]");
}

void prac_skill(struct char_data *ch, int numb, int maxlearn)
{
int max,min,cur,num_dice,j;
extern struct spell_info_type spell_info[MAX_SPL_LIST];

if (ch->skills[numb].learned == 0)   /* never tried before*/ 
    {
    max = 0; 
    min = 100; 
    num_dice = MAX(1, 1+4*(spell_info[numb].min_level_magic-GET_LEVEL(ch)));
    for (j=1; j <= num_dice ; j++) 
        {
        cur = number(1,100); 
        max = MAX(max, cur); 
        min = MIN(min, cur); 
        }
    ch->skills[numb].recognise = MAX(1, 100-max); 
    ch->skills[numb].learned = MAX(1, MIN(maxlearn,min)); 
    send_to_char("You watch your guildmaster's demonstration.\n\r",ch); 
    send_to_char("Wow, it doesn't cost even one practice!\n\r",ch);
    return; 
    }
  
/* practicing a spell with a negative recognize assumes recognize value of 1
 * that's why I added the MAX 2 inside of the min
 */
if (ch->skills[numb].recognise <maxlearn) 
    {
    ch->skills[numb].recognise = MIN (maxlearn,MAX(2,ch->skills[numb].recognise) +
1);
    send_to_char("Your guildmaster gives you a hint.\n\r",ch); 
    if(ch->specials.practices) 
    ch->specials.practices--;
    return;
    }
  
send_to_char("Your guildmaster can't imagine any hint you haven't already heard.\n\r",ch);
 
}
