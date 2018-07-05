/***************************************************************************
*				 MEDIEVIA CyberSpace Code and Data files		           *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*					   All rights reserved				                   *
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
#include <assert.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>
          
/*#include "/usr/local/lib/gcc-lib/sparc-sun-sunos4.1/2.4.5/include/math.h"*/
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "handler.h"
#include "interp.h"
#include "db.h"
#include "spells.h"

struct char_data *combat_list       = NULL;
struct char_data *combat_next_dude  = NULL;
void fall(struct char_data *ch);

extern void add_object(struct obj_data *obj);
extern struct dex_skill_type dex_app_skill[];
extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;
extern char global_color;
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data  *object_list;
extern struct char_data *pick_victim(struct char_data *ch);
extern struct zone_data zone_table_array[MAX_ZONE];
extern void reanimate(struct descriptor_data *d, int cmd);
extern void make_blood(struct char_data *ch, struct char_data *victim);
extern void make_ashes(struct char_data *ch, struct char_data *victim);
extern bool is_formed(struct char_data *ch);
extern void tweak_corpse(struct obj_data *corpse, struct char_data *mob);
void hit(struct char_data *ch, struct char_data *victim, int type);
void die_formation(struct char_data *ch);
bool is_in_safe(struct char_data *ch, struct char_data *victim);
bool is_first_level(struct char_data *ch, struct char_data *victim);
void remove_from_formation(struct char_data *ch);
void dam_message(int dam, struct char_data *ch, struct char_data *victim,
		 int w_type);
void group_gain( struct char_data *ch, struct char_data *victim );
bool check_parry( struct char_data *ch, struct char_data *victim );
bool check_dodge( struct char_data *ch, struct char_data *victim );
void disarm( struct char_data *ch, struct char_data *victim );
void trip( struct char_data *ch, struct char_data *victim );
void mage_combat(struct char_data *ch);
void thief_combat(struct char_data *ch);
void warrior_combat(struct char_data *ch);
void cleric_combat(struct char_data *ch);
void remove_room_affect(struct room_affect *ra, char type);
void tell_clan(int clan, char *argument);
extern bool is_trapped(struct char_data *ch);

/*
 * Control the fights going on.
 * Called periodically by the main loop.
 */
void perform_violence( void )
{
    struct char_data *ch=NULL;
    int deep;
    deep=0;
    /* first we go through combat list to find a guy fighting someone in
       room -1(he has been extracted)..if found, break and start going through
       combat list from beginning, do this till combat list has been purged
       of all fighting extracted char's*/
    while(1){
        for (ch = combat_list; ch; ch = combat_next_dude )
        {
            deep++;
	    if(deep>5000){
	        log_hd("## infinite combat_list(deep>5000) #1");
	        return;
	    }
	    if(ch->specials.fighting->in_room==-1){
	        sprintf(log_buf,"## %s->specials.fighting->in_room==-1 in perform_violence(%d)#1",GET_NAME(ch),ch->specials.home_number);
	        log_hd(log_buf);
	        stop_fighting(ch);
	        break;
            }
        }
        if(!ch)break;
    }
    deep=0;
    for (ch = combat_list; ch; ch = combat_next_dude )
    {
        deep++;
	if(deep>5000)
		{
	    log_hd("## infinite combat_list(deep>5000)");
	    return;
		}
	combat_next_dude = ch->next_fighting;
	if( !ch->specials.fighting )
		SUICIDE;
	if ( AWAKE(ch) && ch->in_room == ch->specials.fighting->in_room )
		{
	    hit( ch, ch->specials.fighting, TYPE_UNDEFINED );
	    continue;
        }
	else
		{
	    stop_fighting(ch);
        }
    }
}



/*
 * Do one group of attacks.
 */
void hit( struct char_data *ch, struct char_data *victim, int type )
{
    int     chance,percent;
    char buffer[MAX_STRING_LENGTH];
    struct char_data *tmp=NULL;

	if (IS_DEAD(ch) && (victim->specials.fighting == ch)){
	    for (tmp = world[ch->in_room]->people; tmp; tmp = tmp->next_in_room ){
			if((tmp!=ch)&&(tmp!=victim)&&(tmp->specials.fighting==victim) 
			&&(!IS_NPC(tmp))&&(victim->specials.fighting)&&(ch->specials.fighting)
			&&!IS_DEAD(tmp)){
				stop_fighting(ch);
				stop_fighting(victim);
				sprintf(log_buf,"%s So!  You thought you could trick me into fighting a corpse!", GET_NAME(tmp) );
				do_talk(victim, log_buf, 0);
				sprintf(log_buf,"stops fighting for a moment and then attacks %s with a vengeance.",GET_NAME(tmp));
				do_emote(victim, log_buf,0);
				set_fighting( victim, tmp );
				return;
			}
		}
    }
    if(IS_MOB(ch)&&GET_MOB_WAIT(ch))
        GET_MOB_WAIT(ch)--;
    /*
     * Inviso attacks.  Not.
     */
    if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
    {
	act( "$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM );

	if ( affected_by_spell( ch, SPELL_INVISIBLE ) )
	    affect_from_char( ch, SPELL_INVISIBLE );

	REMOVE_BIT( ch->specials.affected_by, AFF_INVISIBLE );
	ch->specials.wizInvis = FALSE;
    }
    if (IS_AFFECTED(ch, AFF_SNEAK))
	affect_from_char(ch, SKILL_SNEAK);
    /*
     * First attack.
     * After each attack, check to see if ch has fled or is dead.
     */
    if ( one_hit(ch, victim, type) )
	return;
    if ( ch->in_room != victim->in_room )
	return;
    if ( GET_POS(victim) == POSITION_DEAD )
	return;

/********************MOB COMBAT INTELIGENCE HERE*****************************/
    
    if (IS_MOB(ch)&&ch->nr!=9800&&!GET_MOB_WAIT(ch)
	&&GET_MOB_POS(ch)==POSITION_STANDING&&ch->in_room>0){
		switch(GET_CLASS(ch)){
		    case CLASS_OTHER:
	    		break;
		    case CLASS_MAGIC_USER:
		        mage_combat(ch);
			break;
		    case CLASS_CLERIC:
		        cleric_combat(ch);
			break;
		    case CLASS_WARRIOR:
			warrior_combat(ch);
			break;
		    case CLASS_THIEF:
			thief_combat(ch);
			break;
		    default:
			break;
		}
    }
    if ( ch->in_room != victim->in_room )
	return;
    if ( GET_POS(victim) == POSITION_DEAD )
	return;
    if ( IS_HOVERING(victim) )
	return;

    /*
     * Second attack.
     */
    chance=IS_NPC(ch) ? 2 * GET_LEVEL(ch)
                       : ch->skills[SKILL_SECOND_ATTACK].learned * 2 / 3;
    if (IS_NPC(ch) && GET_CLASS(ch) == CLASS_WARRIOR)
	chance=100;
    if ( number(1, 100) < chance )
	if (one_hit( ch, victim, type ))
		return;
    if ( ch->in_room != victim->in_room )
	return;
    if ( GET_POS(victim) == POSITION_DEAD )
	return;
    if ( IS_HOVERING(victim) )
	return;
    /*
     * Third attack.
     */

    chance  = IS_NPC(ch) ? GET_LEVEL(ch)
			 : ch->skills[SKILL_THIRD_ATTACK].learned * 2 / 3; 
    if ( type == SKILL_BACKSTAB )
	chance /= 2;
    if (IS_NPC(ch) && GET_CLASS(ch) == CLASS_WARRIOR)
	chance=100;
    if ( number(1, 100) < chance )
	if (one_hit( ch, victim, type ))
		return;
    if ( ch->in_room != victim->in_room )
	return;
    if ( GET_POS(victim) == POSITION_DEAD )
	return;
    if ( IS_HOVERING(victim) )
	return;

    /*
     * Fourth attack.
     */
    chance  = IS_NPC(ch) ? GET_LEVEL(ch) * 2 / 3 : 0;
    if (IS_NPC(ch) && GET_CLASS(ch) == CLASS_WARRIOR)
	chance=100;
    if ( number(1, 100) < chance )
	if(one_hit( ch, victim, type ))
		return;

    if ( GET_POS(victim) == POSITION_DEAD )
	return;
    if ( ch->in_room != victim->in_room )
	return;
    if ( IS_HOVERING(victim) )
	return;

	if (GET_MAX_HIT(victim) > 0)
	    percent = (100*GET_HIT(victim))/GET_MAX_HIT(victim); 
	else
	    percent = -1; /* How could MAX_HIT be < 1?? */
	if (IS_NPC(victim))
	    strcpy(buffer, victim->player.short_descr); 
	else
	    strcpy(buffer, GET_NAME(victim)); 
	if (percent >= 100)
	    strcat(buffer, " is in an excellent condition.\n\r"); 
	else if (percent >= 90)
	    strcat(buffer, " has a few scratches.\n\r"); 
	else if (percent >= 83)
	    strcat(buffer, " has a nasty looking welt on the forehead.\n\r"); 
	else if (percent >= 76)
	    strcat(buffer, " has some small wounds and bruises.\n\r"); 
	else if (percent >= 69)
	    strcat(buffer, " has some minor wounds.\n\r"); 
	else if (percent >= 62)
	    strcat(buffer, " winces in pain.\n\r"); 
	else if (percent >= 55)
	    strcat(buffer, " has quite a few wounds.\n\r"); 
	else if (percent >= 48)
	    strcat(buffer, " grimaces with pain.\n\r"); 
	else if (percent >= 41)
	    strcat(buffer, " has some nasty wounds and bleeding cuts.\n\r"); 
	else if (percent >= 36)
	    strcat(buffer, " has some large, gaping wounds.\n\r"); 
	else if (percent >= 29)
	    strcat(buffer, " looks pretty awful.\n\r"); 
	else if (percent >= 22)
	    strcat(buffer, " has many greivous wounds.\n\r"); 
	else if (percent >= 15)
	    strcat(buffer, " is covered with blood from oozing wounds.\n\r"); 
	else if (percent >= 8)
	    strcat(buffer, " pales visibly as death nears.\n\r"); 
	else if (percent >= 0)
	    strcat(buffer, " barely clings to life.\n\r"); 
	global_color=35;
	send_to_char(buffer, ch); 
	global_color=0;

   /*
    * Wimp out?
    */
    if (IS_NPC(victim))
    {
	if (IS_AFFECTED(victim, AFF_WIMPY) && 
	    GET_HIT(victim) < GET_MAX_HIT(victim)/5)
	{
	    do_flee( victim, "", 0 );
	}
    } else {
	if (victim->specials.wimpy >= GET_HIT(victim) )
	{
	    do_flee( victim, "", 0 );
	}
    }

return;
}



/*
 * Hit one guy once. Return TRUE if victim is dead/extracted or something.
 * If FALSE, then ok to continue fighting.
 */
bool one_hit( struct char_data *ch, struct char_data *victim, int type )
{
    struct obj_data *wielded=NULL;
    int w_type;
    int victim_ac, calc_thaco=0;
    int dam;
    int diceroll;

    extern int thaco[4][36];
    extern byte backstab_mult[];
    extern struct str_app_type str_app[];
    extern struct dex_app_type dex_app[];

    if ( type != SKILL_BACKSTAB ){
    	victim=pick_victim(victim);
	}
    if(!victim)
	return(TRUE);
    /*
     * Can't beat a dead char!
     */

    if ( GET_POS(victim) == POSITION_DEAD )
	return(TRUE);
    /*
     * This happens when people flee et cetera during multi attacks.
     */
    if ( ch->in_room != victim->in_room )
	return(TRUE);
    if(ch->in_room==-1||victim->in_room==-1){
        sprintf(log_buf,"## %s vs %s room -1 in one_hit",
                        GET_NAME(ch),GET_NAME(victim));
        log_hd(log_buf);
        return(TRUE);
    } 

    /*
     * Figure out the type of damage message.
     */
    wielded = ch->equipment[WIELD];
    w_type  = TYPE_HIT;
    if ( wielded && wielded->obj_flags.type_flag == ITEM_WEAPON )
    {
	switch ( wielded->obj_flags.value[3] )
	{
	case 0: case 1: case 2:             w_type = TYPE_WHIP;     break;
	case 3:                             w_type = TYPE_SLASH;    break;
	case 4: case 5: case 6:             w_type = TYPE_CRUSH;    break;
	case 7:                             w_type = TYPE_BLUDGEON; break;
	case 8: case 9: case 10: case 11:   w_type = TYPE_PIERCE;   break;
	}
    }
    if ( type == SKILL_BACKSTAB ) {
	w_type  = SKILL_BACKSTAB;
	if(IS_AFFECTED(ch, AFF_SNEAK))
		calc_thaco=-35;
    }

    /*
     * Calculate to-hit-armor-class-0 versus armor class.
     * thaco for monsters is set in hitroll.
     */
    if ( !IS_NPC(ch) )
	calc_thaco += thaco[(int) GET_CLASS(ch)-1][(int) GET_LEVEL(ch)];
    else
	calc_thaco += MAX(-40, (200 - GET_LEVEL(ch)*8));

/*
    else if(GET_CLASS(ch)==CLASS_OTHER)
	if(GET_LEVEL(ch)>34)
		calc_thaco+=thaco[CLASS_WARRIOR-1][35];
	else
		calc_thaco+=thaco[CLASS_WARRIOR-1][(int) GET_LEVEL(ch)];
    else if(GET_LEVEL(ch)>34)
	calc_thaco+=thaco[(int) GET_CLASS(ch)-1][35];
    else
	calc_thaco+=thaco[(int) GET_CLASS(ch)-1][(int) GET_LEVEL(ch)];
*/

    calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
    calc_thaco -= GET_HITROLL(ch);

    victim_ac = GET_AC(victim); /* Used to be /10 */
    if ( AWAKE(victim) )
	victim_ac += dex_app[GET_DEX(victim)].defensive;
    victim_ac = MAX( -100, victim_ac );

    /*
     * The moment of excitement!
     */
    diceroll = number(1, 200);

    if ( diceroll < 191 && AWAKE(victim)
    &&  (diceroll <= 10 || diceroll < calc_thaco - victim_ac) )
    {
	/* Miss. */
	return(damage( ch, victim, 0, w_type ));
    }


    /*
     * Hit.
     * Calc damage.
     */
    if ( wielded )
	dam = dice( wielded->obj_flags.value[1], wielded->obj_flags.value[2] );
    else if ( IS_NPC(ch) )
	dam = dice( ch->specials.damnodice, ch->specials.damsizedice );
    else
	dam = number( 0, 2 );

    /*
     * Bonuses.
     */
    dam += str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    if (GET_CLASS(ch) == CLASS_WARRIOR)
	dam += GET_DAMROLL(ch);
    else if (GET_CLASS(ch) == CLASS_THIEF)
	dam += number(((int)((float)GET_DAMROLL(ch)/2.0)),GET_DAMROLL(ch));
    else
	dam += number(0,GET_DAMROLL(ch));

    /*
     * Bonus for hitting weak opponents.
     * Bonus for backstabbing.
     */
    if ( GET_POS(victim) < POSITION_FIGHTING )
	dam *= 1 + ( 2 * ( POSITION_FIGHTING - GET_POS(victim) ) ) / 3;

    if ( type == SKILL_BACKSTAB )
	dam *= backstab_mult[(int) GET_LEVEL(ch)];

    dam = MAX(1, dam);

    if ( (diceroll>190) && (type!=SKILL_BACKSTAB) )
	if (GET_CLASS(ch)==CLASS_WARRIOR)
	    if(number(1,20)==20){
		dam*=3;
		global_color=31;
		act("$n CRITICALLY hits $N",FALSE,ch,NULL,victim,TO_ROOM);
		act("You CRITICALLY hit $N",FALSE,ch,NULL,victim,TO_CHAR);
		global_color=0;
	    }

    if ( type == SKILL_BACKSTAB )
	type = TYPE_UNDEFINED;    /* Prevents multi backstabbing/combat */

    return(damage( ch, victim, dam, w_type ));
}

void position_change(struct char_data *ch)
{
    global_color=34;
    switch(GET_MOB_POS(ch)){
	case POSITION_SLEEPING:
	    send_to_char("You wake up and stand.\n\r",ch);
	    act( "$n wakes and stand up!",
        	FALSE, ch, NULL, NULL, TO_ROOM );
	    GET_MOB_POS(ch)=POSITION_STANDING;
	    break;
	case POSITION_SITTING:
	    send_to_char("You stand up.\n\r",ch);
	    act( "$n stands up!",
        	FALSE, ch, NULL, NULL, TO_ROOM );
	    GET_MOB_POS(ch)=POSITION_STANDING;
	    break;
	case POSITION_BACK:
	    send_to_char("You get off your back and get on your knees.\n\r",ch);
	    act( "$n gets off $s back and starts kneeling.",
        	FALSE, ch, NULL, NULL, TO_ROOM );
	    GET_MOB_POS(ch)=POSITION_KNEELING;
	    break;
	case POSITION_BELLY:
	    send_to_char("You push off your belly and get up on your knees.\n\r",ch);
	    act( "$n pushes up off $s belly and gets on $s knees.",
        	FALSE, ch, NULL, NULL, TO_ROOM );
	    GET_MOB_POS(ch)=POSITION_KNEELING;
	    break;
	case POSITION_KNEELING:
	    send_to_char("You jump up from a kneeling position.\n\r",ch);
	    act( "$n jumps up on $s feet from a kneeling position!",
        	FALSE, ch, NULL, NULL, TO_ROOM );
	    GET_MOB_POS(ch)=POSITION_STANDING;
	    break;
    }
    global_color=0;
}


/*
 * Inflict damage from a hit.
 * Return TRUE if victim dies.
 */
bool damage( struct char_data *ch, struct char_data *victim,
	    int dam, int attacktype )
{
    int from_room;
    struct message_type *messages=NULL;
    int i, j, nr, max_hit, mana_gain;
    int ch_al,vi_al,al=1;
    int hit_limit(struct char_data *ch);
    struct descriptor_data *ii=NULL;
    if ( GET_POS(victim) == POSITION_DEAD )
	return(FALSE);
    if(ch->in_room==-1||victim->in_room==-1){
	sprintf(log_buf,"## %s vs %s room -1 in damage",
			GET_NAME(ch),GET_NAME(victim));
	log_hd(log_buf);
        return(FALSE);
    }
    /*
     * Certain attacks are forbidden.
     */
    if ( victim != ch )
    {
	if ( is_in_safe( ch, victim ) )
	    return(FALSE);
	if ( is_first_level( ch, victim ) )
	    return(FALSE);
	check_killer( ch, victim );
    }
    /* mages spells are more effective at first */
    if(GET_CLASS(ch)==CLASS_MAGIC_USER&&GET_POS(victim)!=POSITION_FIGHTING)
	dam+=dam*(int)(GET_MANA(ch)/MAX(1,GET_MAX_MANA(ch)));

    /*
     * An eye for an eye, a tooth for a tooth, a life for a life.
     */
    if ( GET_POS(victim) > POSITION_STUNNED && ch != victim )
    {
	if ( !victim->specials.fighting )
	    set_fighting( victim, ch );
	GET_POS(victim) = POSITION_FIGHTING;
    }

    if ( GET_POS(ch) > POSITION_STUNNED && ch != victim )
    {
	if ( !ch->specials.fighting )
	    set_fighting( ch, victim );

	/*
	 * If victim is charmed, ch might attack victim's master.
	 */
	if ( IS_NPC(ch) && IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM)
	&& victim->master != ch && victim->master->in_room == ch->in_room
	&& (victim->master != victim) && number(0,9) == 0 )
	{
	    if ( ch->specials.fighting )
		stop_fighting(ch);
	    hit( ch, victim->master, TYPE_UNDEFINED );
	    return(FALSE);
	}
    }

    /*
     * More charm stuff.
     */

    if ( IS_AFFECTED( ch, AFF_CHARM ) ) {
	remove_from_formation( ch ); 
	REMOVE_BIT(ch->specials.affected_by, AFF_CHARM);
    }
	    
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
	dam /= 2;
    if(dam>0)
    if ( IS_AFFECTED(victim, AFF_FIRESHIELD)){
	if ((IS_UNDEAD(ch))
                  && ( victim->nr == 9801 )
                  && ( victim->specials.death_timer != 0 )
                  ){

	act("An invisible aura protects $N from your fireshield.",
        TRUE,victim,0,ch,TO_CHAR);
        act("An invisible aura protects $N from $n's fireshield.",
        TRUE,victim,0,ch,TO_NOTVICT);
        act("An invisible aura protects you from $n's fireshield.",
        TRUE,victim,0,ch,TO_VICT);
	} else{
	act("Energy rebounds from your fireshield hitting $N.",
	TRUE,victim,0,ch,TO_CHAR);
	act("Energy rebounds from $n's fireshield hitting $N.",
	TRUE,victim,0,ch,TO_NOTVICT);
	act("Energy rebounds from $n's fireshield hitting you.",
	TRUE,victim,0,ch,TO_VICT);
        if ( (GET_LEVEL(ch)<32) || IS_NPC(ch) ){

    	    GET_HIT(ch) -= (int)(dam*MIN(35,GET_LEVEL(victim))*1.5/100);
	}
	if(GET_HIT(ch)<1)GET_HIT(ch)=1;
	}
    }
    /*
     * Damage modifiers.
     */

    if ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && GET_ALIGNMENT(ch) < -349)||
        (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && GET_ALIGNMENT(ch) > 349))
	dam /= 2;

    dam = MAX(0, dam);

    /* don't let undead corpses do damage unless fighting a necromancer 
       that was loaded by game.
     */

    if ( IS_UNDEAD(ch) )
	dam = 0;

    if 	(IS_UNDEAD(ch) 
		&& (victim->nr == 9801) 
		&& (victim->specials.death_timer != 0)
	)
	{
	act("You feel your hand aided by a divine force.",
        	TRUE,ch,0,victim,TO_CHAR);
	act("$n fights as if aided by a divine force.",
        	TRUE,ch,0,victim,TO_ROOM);
	dam = number(50, (int)(victim->points.max_hit/5));
	}

    if	(IS_UNDEAD(victim) 
		&& (ch->nr == 9801) 
		&& (ch->specials.death_timer != 0 )
	)
	{ 
	dam = (int) GET_HIT(victim)/16;
        act("$n struggles against $N's divine power.",
                TRUE,ch,0,victim,TO_NOTVICT);
	}

    /*
     * Can't hurt god, but she likes to see the messages.
     */
    if ( GET_LEVEL(victim) >= 32 && !IS_NPC(victim) )
	dam = 0;

	/* when a dragon is fighting his prey, dont kill him */
	if(IS_SET(ch->player.siMoreFlags,DRAGON)
	&&victim==ch->specials.hunting
	&&((GET_HIT(victim)-dam)<15))
		dam=0;
    /*
     * Level 32 gods can't do damage 
     */
     if(GET_LEVEL(ch) == 32)
	dam = 0;

    /*
     * Check for parry, mob disarm, and trip.
     * Print a suitable damage message.
     */
    if ( attacktype >= TYPE_HIT && attacktype < TYPE_SUFFERING )
    {
	if ( IS_NPC(ch) &&
	(GET_CLASS(ch)==CLASS_THIEF||GET_CLASS(ch)==CLASS_WARRIOR)
	&& !GET_MOB_WAIT(ch) && number( 1, 100 ) < GET_LEVEL(ch) / 4 )
	    disarm( ch, victim );
	if ( IS_NPC(ch) && GET_CLASS(ch)==CLASS_THIEF&&
	    !GET_MOB_WAIT(ch)&&number( 1, 100 )<GET_LEVEL(ch) / 2 )
	    trip( ch, victim );
	if(IS_NPC(ch))position_change(ch);

	if ( (GET_CLASS(victim) == CLASS_WARRIOR) 
		  || IS_SET(victim->player.multi_class,MULTI_CLASS_WARRIOR)
		 )
		if (check_parry( ch, victim ))
	    	return(FALSE);

	if ( (GET_CLASS(victim) == CLASS_THIEF) 
		  || (GET_CLASS(victim) == CLASS_OTHER)
		  || IS_SET(victim->player.multi_class,MULTI_CLASS_THIEF)
		 )
		if (check_dodge( ch, victim ))
	    	return(FALSE);
/*
	if ( (GET_CLASS(ch) == CLASS_THIEF) && check_dodge( ch, victim ) )
	    return(FALSE);
*/
	if ( ch->equipment[WIELD] == NULL )
	    dam_message(dam, ch, victim, TYPE_HIT);
	else
	    dam_message(dam, ch, victim, attacktype);
    }
    else
    {
	for ( i = 0; i < MAX_MESSAGES; i++ )
	{
	    if ( fight_messages[i].a_type != attacktype )
		continue;

	    nr  = dice( 1, fight_messages[i].number_of_attacks );
	    j   = 1;
	    for ( messages = fight_messages[i].msg; j < nr && messages; j++ )
		messages = messages->next;

	    if ( !IS_NPC(victim) && GET_LEVEL(victim) > 31)
	    {
		act( messages->god_msg.attacker_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR );
		act( messages->god_msg.victim_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_VICT );
		act( messages->god_msg.room_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT );
	    }
	    else if ( dam == 0 )
	    {
		act( messages->miss_msg.attacker_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR );
		act( messages->miss_msg.victim_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_VICT );
		act( messages->miss_msg.room_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT );
	    }
	    else if ( GET_POS(victim) == POSITION_DEAD )
	    {
		act( messages->die_msg.attacker_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR );
		act( messages->die_msg.victim_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_VICT );
		act( messages->die_msg.room_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT );
	    }
	    else
	    {
		act( messages->hit_msg.attacker_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR );
		act( messages->hit_msg.victim_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_VICT );
		act( messages->hit_msg.room_msg,
		    FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT );
	    }
	}
    }

    /*
     * Hurt the victim.
     * Reward the perpetrator.
     * Life is hard.
     */
    GET_HIT(victim) -= dam;
    update_pos( victim );
    if ( ch != victim )
	gain_exp( ch, GET_LEVEL(victim) * dam ,NULL);

    /*
     * Inform the victim of his new state.
     * Use send_to_char, because act() doesn't send to DEAD people.
     */
    switch( GET_POS(victim) )
    {
    case POSITION_MORTALLYW:
        global_color=31;
	act( "$n is mortally wounded, and will die soon, if not aided.",
	    TRUE, victim, 0, 0, TO_ROOM );
	send_to_char( 
	    "You are mortally wounded, and will die soon, if not aided.\n\r",
	    victim );
        global_color=0;
	break;

    case POSITION_INCAP:
        global_color=31;
	act( "$n is incapacitated and will slowly die, if not aided.",
	    TRUE, victim, 0, 0, TO_ROOM );
	send_to_char(
	    "You are incapacitated and will slowly die, if not aided.\n\r",
	    victim );
        global_color=0;
	break;

    case POSITION_STUNNED:
        global_color=31;
	act( "$n is stunned, but will probably recover.",
	    TRUE, victim, 0, 0, TO_ROOM);
	send_to_char("You are stunned, but may recover.\n\r",
	    victim );
        global_color=0;

	break;

    case POSITION_DEAD:
        global_color=31;
	act( "$n is DEAD!!", TRUE, victim, 0, 0, TO_ROOM );
	send_to_char( "You have been KILLED!!\n\r\n\r", victim );
        global_color=0;
	break;

    default:
	max_hit = hit_limit( victim );
	global_color=31;
	if ( dam > max_hit / 5 )
	    send_to_char( "That really did HURT!\n\r", victim );
	global_color=0;

	if ( GET_HIT(victim) < max_hit/5 )
	{
	    global_color=31;
	    send_to_char( "You wish you would stop BLEEDING so much!\n\r",
		victim );
	    global_color=0;
	}
	break;
    }

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC(victim) && victim->desc == NULL )
    {
	from_room = victim->in_room;
	do_flee( victim, "", 0 );
	if (victim->in_room != from_room)
		return(FALSE);
	if (victim->specials.fighting )
	{
	    act( "$n is rescued by divine forces.",
		FALSE, victim, 0, 0, TO_ROOM );
	    victim->specials.was_in_room = victim->in_room;
	    char_from_room(victim);
	    char_to_room(victim, 0);
	}
	return(FALSE);
    }

    /*
     * Sleep spells.
     */
    if ( !AWAKE(victim) )
    {
	if ( victim->specials.fighting )
	    stop_fighting( victim );
    }

    /*
     * Payoff for killing things.
     */
    if ( GET_POS(victim) == POSITION_DEAD )
    {
/*	if ( IS_NPC(victim) || victim->desc != NULL )*/
	die_formation(victim);
	group_gain( ch, victim );
	make_blood(ch, victim);
	/*  Give some mana to magic_users .... */
	if(GET_CLASS(ch)==CLASS_MAGIC_USER){
	    mana_gain=number(1,GET_LEVEL(victim)*3)*number(1,1+GET_LEVEL(ch)/10);
	    ch_al=GET_ALIGNMENT(ch);
	    vi_al=GET_ALIGNMENT(victim);
	    if(ch_al<-333&&vi_al<=-333)al=1;
	    else if(ch_al<-333&&vi_al>=-333&&vi_al<=333)al=2;
	    else if(ch_al<-333&&vi_al>=333)al=3;
	    else if(ch_al>-333&&ch_al<=333&&vi_al<=-333)al=2;
	    else if(ch_al>-333&&ch_al<=333&&vi_al>=-333&&vi_al<=333)al=1;
	    else if(ch_al>-333&&ch_al<=333&&vi_al>=333)al=2;
	    else if(ch_al>333&&vi_al<=-333)al=3;
	    else if(ch_al>333&&vi_al>=-333&&vi_al<=333)al=2;
	    else if(ch_al>333&&vi_al>=333)al=1;
	    mana_gain*=al;
	    if(mana_gain+GET_MANA(ch)>GET_MAX_MANA(ch))GET_MANA(ch)=GET_MAX_MANA(ch);
	    else GET_MANA(ch)+=mana_gain;
	    global_color=34;
	    if(al==1)
	        send_to_char("Your spirits were similiar, your power gain was minimal.\n\r",ch);
	    if(al==2)
		send_to_char("Thy spirits were of a modest difference, You feel the power seep into your body.\n\r",ch);
	    if(al==3)
	 	send_to_char("Thy spirits were diametrically opposed, You are flooded with power.\n\r",ch);
	    global_color=0;
	}
	if(IS_NPC(victim)){
	    if(!victim->in_room)
		{
		sprintf(log_buf,"##%s is nowhere! killed by %s in room %d",
			GET_NAME(victim),GET_NAME(ch),ch->in_room);
		log_hd(log_buf);
		}
	    else if(!world[victim->in_room])
		{
		sprintf(log_buf,"##%s is out of this world in room %d, fixing....",
			GET_NAME(victim),
			victim->in_room);
		log_hd(log_buf);
		}
	    else
		zone_table_array[world[victim->in_room]->zone].numkills++;
	}
	if ( !IS_NPC(victim) )
	{

           global_color=31; 
           for (ii = descriptor_list; ii; ii = ii->next)
           	if (ii->character != ch 
				&& !ii->connected&&GET_LEVEL(ii->character) > 32)
			if(IS_SET(ii->character->specials.god_display,GOD_DEATHS)){
		    sprintf(log_buf,"(death) %s[%d] killed by %s[%d]\n\r",
			GET_NAME(victim),
			GET_LEVEL(victim),
                        (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
			GET_LEVEL(ch)
			);
		    send_to_char(log_buf,ii->character);
		}
	   global_color=0;
	    sprintf( log_buf, "%s %s[%d] killed by %s[%d] at %d",
			((GET_LEVEL(ch) == 32) && !IS_NPC(ch)) ? "##" : "",
			GET_NAME(victim),
			GET_LEVEL(victim),
			(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
			GET_LEVEL(ch),
			world[victim->in_room]->number
			);
	    log_hd( log_buf );
            if(!IS_NPC(victim)&&!IS_NPC(ch))
	    {
	      if(IS_NPC(ch)||IS_SET(world[victim->in_room]->room_flags,CHAOTIC))
	      if ( GET_EXP(victim) > exp_table[(int) GET_LEVEL(victim)] )
	      {
	  	GET_EXP(victim) =  exp_table[(int) GET_LEVEL(victim)] +
	  	   ((GET_EXP(victim)-exp_table[(int) GET_LEVEL(victim)])/2);
	      }
	    }
	}
	raw_kill( victim ,ch);
	return(TRUE);
    }

    return(FALSE);
}



/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( struct char_data *ch, struct char_data *victim )
{

    return; /* no killers */
    if(IS_SET(world[ch->in_room]->room_flags,NEUTRAL)||
	IS_SET(world[ch->in_room]->room_flags,CHAOTIC))
	return;
    /*
     * No attack in safe room anyways.
     */
    if (IS_SET( world[ch->in_room]->room_flags, SAFE) )
	return;
    
    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim) )
	return;
    if ( IS_SET(victim->specials.affected_by, AFF_KILLER) )
	return;
    if ( IS_SET(victim->specials.affected_by, AFF_THIEF) )
	return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->specials.affected_by, AFF_CHARM) )
    {
	if ( ch->master == ch )
	{
	    sprintf( log_buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch) );
	    log_hd( log_buf );
	    affect_from_char( ch, SPELL_CHARM_PERSON );
	    REMOVE_BIT( ch->specials.affected_by, AFF_CHARM );
	    return;
	}

	send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
	GET_EXP(ch)=1000;
	GET_MANA(ch)=1;
	GET_GOLD(ch)=1;
	GET_MOVE(ch)=1;
	GET_HIT(ch)=1;
	send_to_char(" THAT WAS A MISTAKE!\n\r", ch);
	SET_BIT(ch->master->specials.affected_by, AFF_KILLER);
	die_formation(ch);
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch) || ch == victim )
	return;

/*    if ( IS_SET(ch->specials.affected_by, AFF_KILLER) )
	return;
*/
    send_to_char( "*** You are now a KILLER!! ***\n\r", ch );
    send_to_char(" YOU LOST HALF YOUR EXP POINTS!\n\r", ch);
    GET_EXP(ch)-=GET_EXP(ch)/2;
    SET_BIT(ch->specials.affected_by, AFF_KILLER);
    return;
}

/*
 * Check for parry.
 */
bool check_parry( struct char_data *ch, struct char_data *victim )
{
    int percent;
    int chance;

    if ( victim->equipment[WIELD] == NULL )
	return FALSE;
    if(GET_MOB_POS(ch) == POSITION_SLEEPING)  return FALSE;
    if(GET_MOB_POS(ch) == POSITION_SITTING)  return FALSE;
    if(GET_MOB_POS(ch) == POSITION_BACK)  return FALSE;
    if(GET_MOB_POS(ch) == POSITION_BELLY)  return FALSE;
    if(GET_MOB_POS(ch) == POSITION_KNEELING)  return FALSE;
    if ( ch->equipment[WIELD] == NULL && number ( 1, 101 ) >= 50 )
        return FALSE;

    if ( !IS_NPC(victim) )
		chance	= victim->skills[SKILL_PARRY].learned / 3;
	else if (GET_CLASS(victim) == CLASS_WARRIOR)
		chance = MIN(5, MAX(30, 2 * GET_LEVEL(victim)));
	else
		chance = 0;

	chance += (4 * str_app[GET_STR(victim)].todam);
    percent = number(1, 101) - (GET_LEVEL(victim) - GET_LEVEL(ch));
    if ( percent >= chance )
	return FALSE;
/*
    act( "$n parries $N's attack.", FALSE, victim, NULL, ch, TO_NOTVICT );
*/
    act( "$n parries your attack.", FALSE, victim, NULL, ch, TO_VICT );
    act( "You parry $N's attack.",  FALSE, victim, NULL, ch, TO_CHAR );
    return TRUE;
}

/*
 * Check for dodge.
 */
bool check_dodge( struct char_data *ch, struct char_data *victim )
{
    int percent;
    int chance;

    if ( !IS_NPC(victim) )
        chance  = victim->skills[SKILL_DODGE].learned / 3;
	else if (GET_CLASS(victim)== CLASS_THIEF)
        	chance = MIN(5, MAX(30, 2 * GET_LEVEL(victim)));
	else if (GET_CLASS(victim)== CLASS_OTHER)
        	chance = MIN(5, MAX(30, 2 * GET_LEVEL(victim)));
	else
			chance = 0;

	chance += dex_app_skill[GET_DEX(victim)].hide;
    percent = number(1, 101) - (GET_LEVEL(victim) - GET_LEVEL(ch));
    if ( percent >= chance )
        return FALSE;
/*
    act( "$n dodges $N's attack.", FALSE, victim, NULL, ch, TO_NOTVICT );
*/
    act( "$n dodges your attack.", FALSE, victim, NULL, ch, TO_VICT );
    act( "You dodge $N's attack.", FALSE, victim, NULL, ch, TO_CHAR );
    return TRUE;
}


/*
 * Load fighting messages into memory.
 */
void load_messages(void)
{
    FILE *f1;
    int i,type;
    struct message_type *messages=NULL;
    char chk[100];

    if (!(f1 = fopen(MESS_FILE, "r"))){
	perror("read messages");
	exit(0);
    }

	open_files++;
    for (i = 0; i < MAX_MESSAGES; i++)
    { 
	fight_messages[i].a_type = 0;
	fight_messages[i].number_of_attacks=0;
	fight_messages[i].msg = 0;
    }

    fscanf(f1, " %s \n", chk);

    while(*chk == 'M')
    {
	fscanf(f1," %d\n", &type);
	for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type!=type) &&
	    (fight_messages[i].a_type); i++);
	if(i>=MAX_MESSAGES){
	    log_hd("Too many combat messages.");
	    exit(0);
	}

	CREATE(messages,struct message_type,1);
	fight_messages[i].number_of_attacks++;
	fight_messages[i].a_type=type;
	messages->next=fight_messages[i].msg;
	fight_messages[i].msg=messages;

	messages->die_msg.attacker_msg      = fread_string(f1);
	messages->die_msg.victim_msg        = fread_string(f1);
	messages->die_msg.room_msg          = fread_string(f1);
	messages->miss_msg.attacker_msg     = fread_string(f1);
	messages->miss_msg.victim_msg       = fread_string(f1);
	messages->miss_msg.room_msg         = fread_string(f1);
	messages->hit_msg.attacker_msg      = fread_string(f1);
	messages->hit_msg.victim_msg        = fread_string(f1);
	messages->hit_msg.room_msg          = fread_string(f1);
	messages->god_msg.attacker_msg      = fread_string(f1);
	messages->god_msg.victim_msg        = fread_string(f1);
	messages->god_msg.room_msg          = fread_string(f1);
	fscanf(f1, " %s \n", chk);
    }

    fclose(f1);
	open_files--;
}



/*
 * Set position of a victim.
 */
void update_pos( struct char_data *victim )
{
    if ( GET_HIT(victim) > 0 )
    {
    	if ( GET_POS(victim) <= POSITION_STUNNED )
	    GET_POS(victim) = POSITION_STANDING;
	return;
    }

    if ( IS_NPC(victim) || GET_HIT(victim) <= -11 )
    {
	GET_POS(victim) = POSITION_DEAD;
	return;
    }

         if ( GET_HIT(victim) <= -6 ) GET_POS(victim) = POSITION_MORTALLYW;
    else if ( GET_HIT(victim) <= -3 ) GET_POS(victim) = POSITION_INCAP;
    else                              GET_POS(victim) = POSITION_STUNNED;

    return;
}


void do_assist(struct char_data *ch, char *argument, int cmd)
{
char name[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
struct char_data *vict=NULL;

	one_argument(argument, name);
	if(!*name)
		send_to_char("Who do you wish to assist?\n\r",ch);
	else if(!(vict=get_char_room_vis(ch, name)))
		send_to_char("No-one by that name is here.\n\r",ch);
	else if(ch == vict){
		send_to_char("You try to help yourself...\n\r",ch);
		send_to_char("Then you realize that you need no help!\n\r",ch);
	     }
	else if(!vict->specials.fighting){
		sprintf(buf,"%s is not fighting at the moment!\n\r",name);
		send_to_char(buf,ch);
	     }
	else {  
		if(IS_NPC(vict)){
		    send_to_char("You must go to ARENA to fight a player!\n\r",ch);
		    return;
		}
		set_fighting(ch,vict->specials.fighting);
		send_to_char("YOU JOIN THE FIGHT!\n\r",ch);
		act("$n joins the fight!", FALSE, ch, 0,vict,TO_ROOM);	
	}
}
/*
 * Start fights.
 */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
    if(ch->specials.fighting){
	log_hd("### line 835,set_fighting,fight.c, was fighting!");
	return;
    }

    ch->next_fighting = combat_list;
    combat_list = ch;

    if(IS_AFFECTED(ch,AFF_SLEEP))
	affect_from_char(ch,SPELL_SLEEP);

    ch->specials.fighting = vict;
    GET_POS(ch) = POSITION_FIGHTING;
    if(IS_NPC(ch))GET_MOB_WAIT(ch)=0;
    return;
}



/*
 * Stop fights.
 */
void stop_fighting(struct char_data *ch)
{
    struct char_data *tmp=NULL;

    if(!ch->specials.fighting)
		SUICIDE;

    if (ch == combat_next_dude)
	combat_next_dude = ch->next_fighting;

    if (combat_list == ch)
       combat_list = ch->next_fighting;
    else
    {
	for (tmp = combat_list; tmp && (tmp->next_fighting != ch); 
	    tmp = tmp->next_fighting)
	    ;
	if (!tmp)
	{
	    log_hd( "##Stop_fighting: char not found" );
	    SUICIDE;
	}
	tmp->next_fighting = ch->next_fighting;
    }

    ch->next_fighting = 0;
    ch->specials.fighting = 0;
    GET_POS(ch) = POSITION_STANDING;
    update_pos(ch);

    return;
}



#define MAX_NPC_CORPSE_TIME 8
#define MAX_PC_CORPSE_TIME 3

void make_corpse(struct char_data *ch, char keep_stuff)
{
extern struct obj_data *objs[MAX_OBJ];
struct obj_data *corpse=NULL, *o=NULL, *head=NULL;
struct obj_data *money=NULL;
struct room_affect *rap=NULL;
char buf[MAX_STRING_LENGTH];
struct char_data *centipede=NULL;
int i;

    if(ch->in_room==-1){
	sprintf(log_buf,"## %s tried to make_corpse in room -1",GET_NAME(ch));
	log_hd(log_buf);
	ch->specials.home_number=ch->specials.home_number+30000;
	return;
    }
    if(is_trapped(ch)){
	for(rap=world[ch->in_room]->room_afs;rap;rap=rap->next)
	    if(rap->ch==ch)break;
	if(rap){
	    remove_room_affect(rap,1);
	}
    }

    /*FOLLOWING IS FOR CENTIPEDE CATACOMB CODE*/
    if(IS_NPC(ch)){
	if(ch->nr>=17009&&ch->nr<17012){
	    centipede=read_mobile(ch->nr+1,REAL);
	    char_to_room(centipede,ch->in_room);
	    centipede=read_mobile(ch->nr+1,REAL);
	    char_to_room(centipede,ch->in_room);
	    act("The Giant Centipede foils death by splitting in two!",FALSE,ch,NULL,NULL,TO_ROOM);
	    return;
	}
    }
    CREATE(corpse, struct obj_data, 1);
    clear_object(corpse);

    corpse->item_number	= NOWHERE;
    corpse->in_room	= NOWHERE;
    corpse->name	= str_dup("corpse");

    sprintf( buf, "the corpse of %s lies here rotting.", 
      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->description = str_dup(buf);

    sprintf( buf, "the corpse of %s",
      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->short_description = str_dup(buf);

    if(IS_NPC(ch)||!keep_stuff)
        corpse->contains = ch->carrying;
	if(!IS_NPC(ch))
		{
		head = read_object(9825,REAL);
		sprintf( log_buf,"The mummified head of %s impaled on a stake",
			GET_NAME(ch));
		if(objs[9825]->short_description!=head->short_description)
			head->short_description = my_free(head->short_description);
		head->short_description = str_dup(log_buf);
		if(objs[head->item_number]->description!=head->description)
			head->description = my_free(head->description);
		head->description=str_dup(log_buf);
		head->obj_flags.cost=300;
		if((GET_LEVEL(ch)>22) && (world[ch->in_room]->zone != 15))
			{
			head->obj_flags.cost=
				MIN(10000,MAX(300,(GET_LEVEL(ch)*ch->specials.numpkills)));
			}
		obj_to_obj( head, corpse );
		}
    if ( (IS_NPC(ch) && GET_GOLD(ch) > 0 )||
	(!IS_NPC(ch)&&GET_GOLD(ch)>0&&!keep_stuff))
    {
	money = create_money(GET_GOLD(ch));
	GET_GOLD(ch)=0;
	obj_to_obj(money, corpse);
    }

    corpse->obj_flags.type_flag    = ITEM_CONTAINER;
    corpse->obj_flags.wear_flags   = ITEM_TAKE;
    corpse->obj_flags.value[0]     = 0; /* You can't store stuff in a corpse */
    corpse->obj_flags.value[3]     = 1; /* corpse identifier */
   
    if(IS_NPC(ch)||!keep_stuff)
        corpse->obj_flags.weight       = GET_WEIGHT(ch)+IS_CARRYING_W(ch);
    else
        corpse->obj_flags.weight       = GET_WEIGHT(ch);
    corpse->obj_flags.eq_level     = 0;
    if (IS_NPC(ch))
    {
	corpse->obj_flags.cost_per_day = 100000;
	corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
    }
    else
    {
	corpse->obj_flags.cost_per_day = 200000;
	corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;
    }

    if(IS_NPC(ch)||!keep_stuff)
    for ( i = 0; i < MAX_WEAR; i++ )
    {
	if ( ch->equipment[i] )
	    obj_to_obj( unequip_char(ch, i), corpse );
    }
    if(IS_NPC(ch)||!keep_stuff){
        ch->carrying	= 0;
        IS_CARRYING_N(ch)	= 0;
        IS_CARRYING_W(ch)	= 0;
    }
	add_object(corpse);

    for ( o = corpse->contains; o; o->in_obj = corpse, o = o->next_content )
	;

    object_list_new_owner( corpse, 0 );
    obj_to_room( corpse, ch->in_room );
    tweak_corpse(corpse,ch);
    return;
}


/* When ch kills victim */
void change_alignment(struct char_data *ch, struct char_data *victim)
{

    if ( GET_ALIGNMENT(victim) < 350 && GET_ALIGNMENT(victim) > -350 )
	GET_ALIGNMENT(ch) = GET_ALIGNMENT(ch);
    else
	GET_ALIGNMENT(ch) -= GET_ALIGNMENT(victim)/250;

    GET_ALIGNMENT(ch) = MIN( 1000, MAX( -1000, GET_ALIGNMENT(ch) ) );
}



void death_cry(struct char_data *ch)
{
    int door, was_in;
    char message[MAX_STRING_LENGTH];
    global_color=33;
    act( "You shiver in disgust as $n lets out a final, agonizing moan before death.",
	FALSE, ch, 0, 0, TO_ROOM );

    if ( IS_NPC(ch) )
	strcpy(message, "You hear the pitiful wail of some dying creature.");
    else
	strcpy(message, "You hear a blood curdling death cry.");
    global_color=0;
    was_in = ch->in_room;
    if(was_in<0||!world[was_in])return;
    for ( door = 0; door <= 5; door++ )
    {
	if (CAN_GO(ch, door))
	{
	    ch->in_room = world[was_in]->dir_option[door]->to_room;
	    if ( ch->in_room == was_in )
		continue;
	    act( message, FALSE, ch, 0, 0, TO_ROOM );
	    ch->in_room = was_in;
	}
    }
}


/* juice */
/* Players soloing get maximum experience of (((level)**3.79)*50/level)
 * Players grouped will have max exp adjusted according to the number
 *  of group members and their level.
 */
void group_gain( struct char_data *ch, struct char_data *victim )
{
char buf[MAX_STRING_LENGTH];
int no_players, share, max_exp;
int totallevels;
int max_level, min_level, level_diff;
struct char_data *k=NULL;
struct char_data *  tmp_ch=NULL;
int                 tmp_share;
int x,y;

/*
* Monsters don't get kill xp's.
* Dying of mortal wounds doesn't give xp to anyone!
*/
if ( IS_NPC(ch) || ch == victim )
	return;

/*if(!IS_NPC(ch) && !IS_NPC(victim) &&
	(!IS_SET(world[victim->in_room]->room_flags,CHAOTIC)
	||!IS_SET(world[victim->in_room]->room_flags,NEUTRAL))
	)
	return;*/

if(ch->in_room != victim->in_room)
	return;

if ( !is_formed(ch) ) /* killer is soloing */
       {
	share = GET_EXP(victim);
        if(share < 0) share = 0;
	max_exp=(int)(pow((double)GET_LEVEL(ch), 3.79) 
			* 50.0 
			/ (double)GET_LEVEL(ch)
			);
	tmp_share = MIN( max_exp, share);
	sprintf( buf,"You are awarded %d experience points for the battle.\n\r",tmp_share, share );
	send_to_char( buf, ch );
	gain_exp( ch, tmp_share ,victim);
        if(IS_NPC(victim))
	    ch->specials.numkills++;
	else if (!(world[victim->in_room]->zone == 15))
	    ch->specials.numpkills++;
	change_alignment( ch, victim );
	}
else   /* killer is grouped */
	{
		k = ch->master;  /* killer is a follower */
	totallevels = 0;
	max_level   = GET_LEVEL(k);
	min_level   = GET_LEVEL(k);
	level_diff  = 0;
	/* # group members used for computing exp share */
	no_players  = 0;
	for (x=0;x<3;x++)
	    for(y=0;y<3;y++)
		{ /* count players in group */
		if(k->formation[x][y])
		if (k->formation[x][y]->in_room == ch->in_room)
			{
			totallevels += GET_LEVEL(k->formation[x][y]);
			if ((min_level > GET_LEVEL(k->formation[x][y])) && (!IS_NPC(k->formation[x][y])) )
				min_level = GET_LEVEL(k->formation[x][y]);
			if ((max_level < GET_LEVEL(k->formation[x][y])) && (!IS_NPC(k->formation[x][y])) )
				max_level = GET_LEVEL(k->formation[x][y]);
			if( !IS_NPC(k->formation[x][y]) && (GET_LEVEL(k->formation[x][y]) < 32) ) 
				no_players ++;
			}
		}

	if ( no_players == 0 )
		{
		send_to_char("Your formation appears to consist of only gods and mobs.  No exp awarded.\n\r", ch);
		return;
		}
	level_diff = max_level - min_level;
	share = GET_EXP(victim);
	if(IS_NPC(victim))
		share   += share * (no_players - 1) / 10;

	if ( (GET_LEVEL(ch) > 31) && (min_level < 31) )
		{
		sprintf(buf,"## %s killed %s while grouped with a mortal.",
			GET_NAME(ch),GET_NAME(victim) );
		log_hd(buf);
		global_color = 1;
		send_to_char("Let mortals kill for themselves!\n\r",ch);
		global_color = 0;
		}
	if(IS_NPC(victim)||IS_SET(world[victim->in_room]->room_flags,
CHAOTIC)){
	for (x=0;x<3;x++)
	    for(y=0;y<3;y++)
		{ /* award exp to everyone in group */
		tmp_ch = k->formation[x][y];
		if(tmp_ch)
		/*if(!IS_NPC(victim) && !IS_NPC(tmp_ch)
    		&& !IS_SET(world[victim->in_room]->room_flags,CHAOTIC)
    		)
			continue;*/
		if ((tmp_ch->in_room == ch->in_room)
		   && ( (tmp_ch->master == ch->master) 
			|| (tmp_ch->master == ch) 
			|| (tmp_ch == ch)
		        || (tmp_ch == k)
			)
		   && (!IS_NPC(tmp_ch))
		   )
			{
			max_exp=(int)(pow((double)GET_LEVEL(tmp_ch), 3.79) 
					* 50.0 
					/ (double)GET_LEVEL(tmp_ch)
					);
			if (level_diff == 0)
				{
				max_exp *= (1 + (0.15 * no_players) );
				send_to_char("A tough battle! You're drenched with blood, but much more experienced.\n\r", tmp_ch);
				}
			else if (level_diff == 1) 
				{
				max_exp *= (1 + (0.13 * no_players) );
				send_to_char( "What a battle! You come out of it bloodier, but more experienced.\n\r",tmp_ch);
				}
			else if (level_diff < 7) 
				{
				max_exp *= (1 + (0.1 * no_players) );
				send_to_char(  "Each member of the group benefits the others.\n\r", tmp_ch);
				}
			else if (level_diff < 11) 
				{
				max_exp *= (1 + (0.05 * no_players) );
				send_to_char(  "You begin to understand the benefits of grouping.\n\r", tmp_ch);
				}
			else if (level_diff < 16) 
				{
				max_exp *= .5;
				send_to_char(  "Your comrades are out of your league.  You learn much less.\n\r", tmp_ch);
				}
			else 
				{
				max_exp *= 0.1;
				send_to_char(  "Your group is so unbalanced you gain almost nothing.\n\r", tmp_ch);
				}
			tmp_share = MIN( max_exp, GET_LEVEL(tmp_ch) * share/ totallevels );
			if(tmp_share < 0) tmp_share = 0;
			sprintf( buf,"You receive %d exps of %d total.\n\r",tmp_share, share );
			send_to_char( buf, tmp_ch );
			gain_exp( tmp_ch, tmp_share ,victim);
		        if(IS_NPC(victim))
			    tmp_ch->specials.numkills++;
			else if (!(world[victim->in_room]->zone == 15))
			    tmp_ch->specials.numpkills++;
			change_alignment( tmp_ch, victim );
			} /* if */
		} /* for */
	} /* if victim is a mob or room is CHAOTIC */
	} /* else killer is grouped */
} /* group_gain */


void dam_message( int dam_inflicted, 
                  struct char_data *ch, 
                  struct char_data *victim,
		  int w_type )
{
    static char *attack_table[] =
    {
	"hit", "pound", "pierce", "slash", "whip", "claw",
	"bite", "sting", "crush"
    };

    char buf1[256], buf2[256], buf3[256];
    char *vs, *vp;
    char *attack;
    char punct;
    int  dam;

/* juice  dam is damage as a percentage of the victim's current hp.   */
/*        If you do 200 damage against a mob with 3K hp, you'll only  */
/*        tickle him.  Later, when he's in an awful condition and has */
/*        only 250 hp, the same 200 dam hit will FUBAR him.           */
    if (victim->points.hit < 1)
	{
        dam = 100;
    /* check to make sure we're not dividing by zero */
        }
    else
        {
        dam = (int) ((dam_inflicted * 100)/(victim->points.hit));
        }       

if (dam_inflicted == 0)  {vs = "miss";             vp  = "misses";           }
    else if (dam <= 2)   {vs = "tickle";           vp  = "tickles";          }
    else if (dam <= 3)   {vs = "scratch";          vp  = "scratches";        }
    else if (dam <= 4)   {vs = "bruise";           vp  = "bruises";          }
    else if (dam <= 5)   {vs = "hit";              vp  = "hits";             }
    else if (dam <= 6)   {vs = "injure";           vp  = "injures";          }
    else if (dam <= 7)   {vs = "wound";            vp  = "wounds";           }
    else if (dam <= 8)   {vs = "smash";            vp  = "smashes";          }
    else if (dam <= 9)   {vs = "smite";            vp  = "smites";           }
    else if (dam <= 10)  {vs = "decimate";         vp  = "decimates";        }
    else if (dam <= 12)  {vs = "devastate";        vp  = "devastates";       }
    else if (dam <= 13)  {vs = "maim";             vp  = "maims";            }
    else if (dam <= 14)  {vs = "shatter";          vp  = "shatters";         }
    else if (dam <= 16)  {vs = "mutilate";         vp  = "mutilates";        }
    else if (dam <= 18)  {vs = "dismember";        vp  = "dismembers";       }
    else if (dam <= 20)  {vs = "disembowel";       vp  = "disembowels";      }
    else if (dam <= 26)  {vs = "pulverize";        vp  = "pulverizes";       }
    else if (dam <= 38)  {vs = "massacre";         vp  = "massacres";        }
    else if (dam <= 45)  {vs = "demolish";         vp  = "demolishes";       }
    else if (dam <= 50)  {vs = "obliterate";       vp  = "obliterates";      }
    else if (dam <= 56)  {vs = "annihilate";       vp  = "annihilates";      }
    else if (dam <= 63)  {vs = "liquify";          vp  = "liquifies";        }
    else if (dam <= 69)  {vs = "atomize";          vp  = "atomizes";         }
    else if (dam <= 75)  {vs = "FUBAR";            vp  = "FUBARS";           }
    else if (dam <= 99)  {vs = "decapitate";       vp  = "decapitates";      } 
    else                 {vs = "kill";             vp  = "kills";            }


    w_type  -= TYPE_HIT;
    if ( w_type >= sizeof(attack_table)/sizeof(attack_table[0]) )
    {
	log_hd( "Dam_message: bad w_type" );
	w_type  = 0;
    }
    punct   = (dam_inflicted == 0) ? '.' : '!';

    if ( w_type == 0 )
    {
	sprintf( buf1, "$n %s $N%c", vp, punct );
	sprintf( buf2, "You %s $N%c", vs, punct );
	sprintf( buf3, "$n %s you%c", vp, punct );
    }
    else
    {
	attack  = attack_table[w_type]; 
	sprintf( buf1, "$n's %s %s $N%c", attack, vp, punct ); 
	sprintf( buf2, "Your %s %s $N%c", attack, vp, punct ); 
	sprintf( buf3, "$n's %s %s you%c", attack, vp, punct );
    }
    if(IS_NPC(victim))global_color=32; 
    else global_color=33;
    if(dam>0)
       act( buf1, FALSE, ch, NULL, victim, TO_NOTVICT );
    act( buf2, FALSE, ch, NULL, victim, TO_CHAR );
    act( buf3, FALSE, ch, NULL, victim, TO_VICT );
    global_color=0;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( struct char_data *ch, struct char_data *victim )
{
    struct obj_data *obj=NULL;

    if ( victim->equipment[WIELD] == NULL )
	return;
    if(GET_MOB_POS(ch) == POSITION_SLEEPING)  return;
    if(GET_MOB_POS(ch) == POSITION_SITTING)  return;
    if(GET_MOB_POS(ch) == POSITION_BACK)  return;
    if(GET_MOB_POS(ch) == POSITION_BELLY)  return;
    if(GET_MOB_POS(ch) == POSITION_KNEELING)  return;

    if (((ch->equipment[WIELD] == NULL) && (number (1,101) >= 50)))
		{
	    act( "$n tries to disarm you and fails!",
		FALSE, ch, NULL, victim, TO_VICT );
	    act( "You try and disarm $N and fail!",
		FALSE, ch, NULL, victim, TO_CHAR );
        return;
    	}
    obj=victim->equipment[WIELD];

    if (IS_OBJ_STAT(obj,ITEM_INVISIBLE))
		{
		if ( !(IS_AFFECTED(ch, AFF_DETECT_INVISIBLE)) )
			{
	    	act( "$n tries to disarm your invisible weapon and misses!",
				FALSE, ch, NULL, victim, TO_VICT );
	    	act( "You try and disarm $N's invisible weapon and miss!",
				FALSE, ch, NULL, victim, TO_CHAR );
	    	return;
			}
    	}

    if(ch->equipment[WIELD]==NULL){
 	    act( "$n disarms you barehanded and sends your weapon flying!",
		FALSE, ch, NULL, victim, TO_VICT );
	    act( "You disarm $N barehanded and send $S weapon flying!",
		FALSE, ch, NULL, victim, TO_CHAR );
	    act( "$n disarms $N barehanded and sends $S weapon flying!",
		FALSE, ch, NULL, victim, TO_NOTVICT );

    }else{
	    act( "$n disarms you and sends your weapon flying!",
		FALSE, ch, NULL, victim, TO_VICT );
	    act( "You disarm $N and send $S weapon flying!",
		FALSE, ch, NULL, victim, TO_CHAR );
	    act( "$n disarms $N and sends $S weapon flying!",
		FALSE, ch, NULL, victim, TO_NOTVICT );
    }

	if(victim->equipment[WIELD])
		{
		if(!IS_NPC(victim))
			{
            obj_to_char( unequip_char(victim,WIELD), victim );
            }
        else
            {
            act( "$N quickly ducks down and grabs $S weapon.",
                FALSE, ch, NULL, victim, TO_NOTVICT );
            if(victim->equipment[HOLD])
                obj_to_char(unequip_char(victim, HOLD), victim);
            equip_char(victim, unequip_char(victim,WIELD), HOLD);
            }
		}
}



/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip( struct char_data *ch, struct char_data *victim )
{
    if(GET_MOB_POS(ch)!=POSITION_STANDING)return;
    if(GET_POS(ch)!=POSITION_STANDING)return;
    if(IS_AFFECTED(victim,AFF_FLYING)){
        act( "$n tries to trip you, but you fly above the kick!",
            FALSE, ch, NULL, victim, TO_VICT );
        act( "You try and trip $N but $N flies above your kick!",
            FALSE, ch, NULL, victim, TO_CHAR );
/*
        act( "$n tries to trip $N but $N flies above the kick!",
            FALSE, ch, NULL, victim, TO_NOTVICT );
*/
        return;
    }
    act( "$n trips you and you go down!",
        FALSE, ch, NULL, victim, TO_VICT );
    act( "You trip $N and $N goes down!",
        FALSE, ch, NULL, victim, TO_CHAR );
    act( "$n trips $N and $N goes down!",
        FALSE, ch, NULL, victim, TO_NOTVICT );

    if(damage( ch, victim, 1, SKILL_TRIP ))
		return;
    WAIT_STATE( ch, PULSE_VIOLENCE*2 );
    WAIT_STATE( victim, PULSE_VIOLENCE*2 );
	if(IS_NPC(victim))
    	GET_MOB_POS(victim) = POSITION_SITTING;
	else
		GET_POS(victim) = POSITION_SITTING;
	fall(victim);

    return;
}

void fall(struct char_data *ch)
{
struct char_data *person=NULL, *victim=NULL;
int belt_quest(struct char_data *ch, int cmd, char *arg);
int dam;
int bonus;
int to_room;
int weight;

if( !ch 
	|| (ch->in_room  < 1) 
	|| (ch->in_room >= MAX_ROOM)
	|| (!world[ch->in_room])
	|| (GET_POS(ch) <= POSITION_STUNNED) 
	)
	return;

if (GET_HIT(ch) < 5)
	return;

if (!EXIT(ch,5))  /* if no exit down, return */
	return;

if (IS_SET(EXIT(ch, 5)->exit_info, EX_CLOSED))
	return;

to_room = EXIT(ch, 5)->to_room;

if(IS_SET(world[to_room]->room_flags,NEUTRAL)
	&& !IS_SET(world[ch->in_room]->room_flags,NEUTRAL)
	)
	return;
	
if(IS_SET(world[to_room]->room_flags,CHAOTIC)
	&& !IS_SET(world[ch->in_room]->room_flags,CHAOTIC)
	)
	return;
	
/* if room below has any restrictions at all return */
if (IS_NPC(ch) && IS_SET(world[to_room]->room_flags, NO_MOB))
	return;
if(world[to_room]->level_restriction)
	return;
if(world[to_room]->class_restriction)
	return;
if(world[to_room]->align_restriction)
	return;

bonus = dex_app_skill[GET_DEX(ch)].hide;
if (GET_LEVEL(ch) > 34)
	bonus += 5;
if (number(1,100) > (25 - bonus))
	{
	send_to_char("You nearly topple over the edge.\n\r",ch);
	act( "$n nearly topples over the edge.", FALSE, ch, NULL, NULL, TO_ROOM);
	return;
	}

if(ch->specials.fighting)
	stop_fighting(ch);
global_color=31;
send_to_char("You lose your balance and topple over the edge!\n\r",ch);
act( "$n loses $s balance and topples over the edge!!", FALSE, ch, NULL, NULL, TO_ROOM );
send_to_char("You let out a blood curdling scream!\n\r",ch);
act( "$n lets out a blood curdling scream!", FALSE, ch, NULL, NULL, TO_ROOM );
global_color=0;

char_from_room(ch);
char_to_room(ch,to_room);
act( "You hear a blood curdling scream!", FALSE, ch, NULL, NULL, TO_ROOM);
act( "$n falls from above! You scramble, to avoid being squished.", FALSE, ch, NULL, NULL, TO_ROOM );
if (   (world[to_room]->sector_type==SECT_UNDERWATER)
	|| (world[to_room]->sector_type==SECT_WATER_SWIM)
	|| (world[to_room]->sector_type==SECT_WATER_NOSWIM)
	)
	{
	send_to_char("You make a huge splash falling into the water!\n\r",ch);
	act( "$n makes a huge splash falling in the water!", FALSE, ch, NULL, NULL, TO_ROOM );
	}
else if(IS_NPC(ch) || (GET_LEVEL(ch) < 32))
	{
	weight = IS_CARRYING_W(ch) + GET_WEIGHT(ch);
	dam = number((int)(weight/3),weight);
	GET_HIT(ch) = MAX(1, (GET_HIT(ch)-dam));
    act(". ", TRUE,ch,0,ch,TO_CHAR);
    act(". ", TRUE,ch,0,ch,TO_CHAR);
    act(". ", TRUE,ch,0,ch,TO_CHAR);
    act("SPLAT!!!", TRUE,ch,0,ch,TO_CHAR);
	}
do_look(ch, "", 15);
GET_MOB_POS(ch)=POSITION_STANDING;
/* if exit down, make everyone fall */
if ((EXIT(ch,5)) && (EXIT(ch, 5)->to_room != ch->in_room)) 
	{
	person=world[ch->in_room]->people;
	while(person 
		&& (person->in_room > 1) 
		&& (person->in_room < MAX_ROOM)
		&& (world[person->in_room]))
		{
		if (person==ch)
			{
			person=person->next_in_room;
			}
		else
			{
			victim=person;
			person=person->next_in_room;
			act("Scrambling to avoid $n's fall, you stumble and try to avoid falling yourself.", TRUE,ch,0,victim,TO_VICT);
			fall(victim);
			}
		}
	if (ch
		&& (GET_LEVEL(ch) < 32)
		&& (ch->in_room > 1) 
		&& (ch->in_room < MAX_ROOM)
		&& (world[ch->in_room]))
		{
		act("You see the drop below and struggle to avoid falling further.",
			TRUE,ch,0,0,TO_CHAR);
		fall(ch);
		}
	}

if ( (int)world[ch->in_room]->funct == (int)belt_quest )
	(*world[ch->in_room]->funct)(ch, 9, "");
   
}/*end of fall() */
