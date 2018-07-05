/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
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
#include "casino.h"

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
extern void space_to_underline(char *text);
extern void page_string(struct descriptor_data *d, char *str, int keep_internal);
struct obj_data *create_money( int amount );


extern int number_of_rooms;
extern char *deck[];
void shuffle(struct char_data *ch);
void deal(struct char_data *ch, char *arg, int type);
void show_cards(struct char_data *ch, char *arg);
void discard(struct char_data *ch, char *arg);

int return_card_by_position(struct char_data *ch,int pos)
{
int x,s;
    s=0;   
    for(x=0;x<52;x++){
	if(ch->p->cards[x]!=NO_CARD)
	    s++;
	if(s==pos)
	    return(x);
    }
    return(-1);
}
int return_fold(struct char_data *ch)
{
int x;

    for(x=0;x<52;x++)
	if(ch->p->cards[x]==FOLD)
	    return(TRUE);
    return(FALSE);
}

int slot_machine(struct char_data *ch, int cmd, char *arg)
{
int bet,room,base,jackpot;

    if(IS_NPC(ch))return FALSE;
    if(ch->in_room == 0) return FALSE;
    if(cmd!=3673&&cmd!=3674)return FALSE;
    room=ch->in_room;
    bet=world[room]->temperature_mod;
    if(cmd==3674){
	base=world[room]->move_mod;
	jackpot=world[room]->pressure_mod;
	sprintf(log_buf,"\n\r---==<(*Medievia SLOT Machine Room #[%d]*)>==---\n\r",room);
	send_to_char(log_buf,ch);
	sprintf(log_buf,"             Betting Amount: %d\n\r",bet);
	send_to_char(log_buf,ch);
	sprintf(log_buf,"            Current Jackpot: %d\n\r",jackpot);
	send_to_char(log_buf,ch);
	sprintf(log_buf,"            Odds of winning: 1 in %d\n\r",(int)((base*base)/5));
	send_to_char(log_buf,ch);
	return(TRUE);
    }
    if(GET_GOLD(ch)<bet){
	send_to_char("You dig deep in your pockets for more gold, Nope, not enough.\n\r",ch);
	return(TRUE);
    }
    GET_GOLD(ch)-=bet;
    ch->p->querycommand=19990;/*this will force comm.c to roll slots and use a timing Q*/
    ch->internal_use=room;/*incase some idiot trans's or summons him */
    world[room]->pressure_mod+=bet;
    sprintf(log_buf,"You put in %d and Pull the Lever....\n\r",bet);
    send_to_char(log_buf,ch);
    sprintf(log_buf,"$n puts in %d and Pulls the Lever....\n\r",bet);
    act(log_buf,TRUE,ch,0,0,TO_ROOM);
    ch->p->slots[0][0]=-1;
    return(TRUE);
}

void slot_win(struct char_data *ch,int type)
{
int winnings;
int jackpot;
struct obj_data *tmp_object=NULL;

    jackpot=world[ch->internal_use]->pressure_mod;
    winnings=0;
    switch(type){
	case 1: /* won on Top row*/
	   winnings=jackpot/2;
	   send_to_char("You WON! On the Top ROW! 1/2 the Jackpot!\n\r",ch);
	   act("Winner on the top row! You hear the sound of the coins falling.",TRUE,ch,0,0,TO_ROOM);
	   break;	
	case 2: /* won on Middle row*/
	   winnings=jackpot;
	   send_to_char("You WON! ON THE MIDDLE ROW! The WHOLE Jackpot!\n\r",ch);
	   act("Winner ON THE MIDDLE ROW! You hear the sound of the coins falling.",TRUE,ch,0,0,TO_ROOM);
	   break;	
	case 3: /* won on Bottom row*/
	   winnings=jackpot/2;
	   send_to_char("You WON! On the Bottom ROW! 1/2 the Jackpot!\n\r",ch);
	   act("Winner on the bottom row! You hear the sound of the coins falling.",TRUE,ch,0,0,TO_ROOM);
	   break;	
	case 4: /* won on diagonal*/
	case 5:
	   winnings=jackpot/4;
	   send_to_char("You WON! On the Diagonal 1/4 the Jackpot.\n\r",ch);
	   act("Winner on the diagonal! You hear the sound of the coins falling.",TRUE,ch,0,0,TO_ROOM);
	   break;	
    }
    world[ch->internal_use]->pressure_mod-=winnings;
    tmp_object = create_money(winnings);
    obj_to_room(tmp_object,ch->internal_use);
}

void roll_slot(struct char_data *ch)
{
char slot[3][3];
char base,c,w;
int x;
    for(x=0;x<3;x++){
	slot[0][x]=ch->p->slots[0][x];
	slot[1][x]=ch->p->slots[1][x];
	slot[2][x]=ch->p->slots[2][x];
    }
    if(ch->p->slots[0][2]){/*done rolling*/
	w=0;
        if(slot[0][0]==slot[0][1]&&slot[0][1]==slot[0][2]){
	    slot_win(ch,1);
	    w=TRUE;
        }else if(slot[1][0]==slot[1][1]&&slot[1][1]==slot[1][2]){
	    slot_win(ch,2);
	    w=TRUE;
        }else if(slot[2][0]==slot[2][1]&&slot[2][1]==slot[2][2]){
	    slot_win(ch,3);
	    w=TRUE;
        }else if(slot[0][0]==slot[1][1]&&slot[1][1]==slot[2][2]){
	    slot_win(ch,4);
	    w=TRUE;
        }else if(slot[0][2]==slot[1][1]&&slot[1][1]==slot[2][0]){
	    slot_win(ch,5);
	    w=TRUE;
	}
    	for(x=0;x<3;x++){
	    ch->p->slots[0][x]=0;
	    ch->p->slots[1][x]=0;
	    ch->p->slots[2][x]=0;
	}
        ch->internal_use=0;
        ch->p->querycommand=0;
	if(!w)
	   send_to_char("[Sorry no winning combinations]\n\r",ch);
	return;
    }
    if(slot[0][0]==-1){
	ch->p->slots[0][0]=0;
	return;
    }
    base=world[ch->internal_use]->move_mod;
    if(!slot[0][0]){/*roll 1st dial*/
	c=number(35,base+35);
        slot[0][0]=c;
	c++;
	if(c>base+35)c=35;
        slot[1][0]=c;
        c++;
	if(c>base+35)c=35;
        slot[2][0]=c;
    }
    else if(!slot[0][1]){/*roll 2nd dial*/
	c=number(35,base+35);
        slot[0][1]=c;
	c--;
	if(c<35)c=35+base;
        slot[1][1]=c;
        c--;
	if(c<35)c=35+base;
        slot[2][1]=c;
    }
    else if(!slot[0][2]){/*roll 3rd dial*/
	c=number(35,base+35);
        slot[0][2]=c;
	c++;
	if(c>base+35)c=35;
        slot[1][2]=c;
        c++;
	if(c>base+35)c=35;
        slot[2][2]=c;
    }
    for(x=0;x<3;x++){
	ch->p->slots[0][x]=slot[0][x];
	ch->p->slots[1][x]=slot[1][x];
	ch->p->slots[2][x]=slot[2][x];
    }
    log_buf[0]='[';
    log_buf[2]=']';   
    log_buf[3]='[';
    log_buf[5]=']';
    log_buf[6]='[';
    log_buf[8]=']';
    log_buf[9]='\n';
    log_buf[10]='\r';
    log_buf[11]=MED_NULL;
    send_to_char("\n\r**SLOTS**\n\r",ch);
    for(x=0;x<3;x++){
	if(slot[x][0])
	   log_buf[1]=slot[x][0];
	else
	   log_buf[1]=' ';
	if(slot[x][1])
	   log_buf[4]=slot[x][1];
	else
	   log_buf[4]=' ';
	if(slot[x][2])
	   log_buf[7]=slot[x][2];
	else
	   log_buf[7]=' ';
	send_to_char(log_buf,ch);
    }
	send_to_char("\n\r",ch);
}

int play_cards(struct char_data *ch, int cmd, char *arg)
{
int x;
char buf[MAX_INPUT_LENGTH];

    if(IS_NPC(ch))return FALSE;
    switch(cmd){
	case 1091:	/*Shuffle*/
	    shuffle(ch);
	    return(TRUE);
	case 1092:	/*deal card face up(showing)*/
	    deal(ch,arg,HAVE_CARD_SHOWING);
	    return(TRUE);
	case 1093:	/*deal to player so only he sees it*/
	    deal(ch,arg,HAVE_CARD_HIDDEN);
	    return(TRUE);
	case 1094:	/*deal card face down(no one sees it yet)*/
	    deal(ch,arg,HAVE_CARD_DOWN);
	    return(TRUE);
	case 1101:	/*discard*/
	    discard(ch, arg);
	    return(TRUE);
	case 1099:	/*fold*/
	    act("$n puts $s cards face down and Folds.",TRUE,ch,0,0,TO_ROOM);
	    send_to_char("You put your cards face down and Fold.",ch);
	    for(x=0;x<52;x++)
		if(ch->p->cards[x]!=NO_CARD)
		    ch->p->cards[x]=FOLD;
	    return(TRUE);
	    break;
	case 1098:	/*raise*/
	    one_argument(arg,buf);
	    if(!buf[0]){
		send_to_char("Syntax:  raise #coinds.\n\r",ch);
		return(TRUE);
	    }
	    x=atoi(buf);
	    if(x<1)return(TRUE);
	    if(x>1000000000)return(TRUE);
	    if(GET_GOLD(ch)<x){
		send_to_char("You recount your money and blush.\n\r",ch);
		return(TRUE);
	    }
	    sprintf(log_buf,"$n says 'I Raise %d...'",x);
	    act(log_buf,TRUE,ch,0,0,TO_ROOM);
	    sprintf(log_buf,"You say 'I Raise %d...'",x);
	    act(log_buf,TRUE,ch,0,0,TO_CHAR);
	    sprintf(log_buf,"%d coins",x);
	    do_drop(ch,log_buf,1103);
	    ch->specials.dropped_coins=0;
	    return(TRUE);
	    break;
	case 1097:	/*ante*/
	    one_argument(arg,buf);
	    if(!buf[0]){
		send_to_char("Syntax:  anty #coinds.\n\r",ch);
		return(TRUE);
	    }
	    x=atoi(buf);
	    if(x<1)return(TRUE);
	    if(x>1000000000)return(TRUE);
	    if(GET_GOLD(ch)<x){
		send_to_char("You recount your money and blush.\n\r",ch);
		return(TRUE);
	    }
	    sprintf(log_buf,"$n pushes %d coins into the pile to begin.",x);
	    act(log_buf,TRUE,ch,0,0,TO_ROOM);
	    sprintf(log_buf,"You push %d coins into the pile to begin.",x);
	    act(log_buf,TRUE,ch,0,0,TO_CHAR);
	    sprintf(log_buf,"%d coins",x);
	    do_drop(ch,log_buf,1103);
	    ch->specials.dropped_coins=0;
	    return(TRUE);
	    break;
	case 1096:	/*show  shows your cards*/
	    if(return_fold(ch)){
		send_to_char("You have folded.\n\r",ch);
		return(TRUE);
	    }
	    for(x=0;x<52;x++)
		if(ch->p->cards[x]!=NO_CARD)
		    ch->p->cards[x]=HAVE_CARD_SHOWING;
	    act("You turn all your cards over so everyone can see them.",TRUE,ch,0,0,TO_CHAR);
	    act("$n turns all of $m cards over so you can see.",TRUE,ch,0,0,TO_ROOM);
	    return(TRUE);
	    break;
	case 1100:	/*cards or cards all    shows cards*/
	    show_cards(ch,arg);
	    return(TRUE);
	    break;
	default:
	    return(FALSE);

    }
}
int return_next_card(struct char_data *ch)
{
int x,card;
    x=0;
    while(x<52&&ch->p->deck[x]==-1)
	x++;
    if(x>=52){
	send_to_char("You have no cards in deck!?\n\r",ch);
	return(0);
    }
    card=ch->p->deck[x];
    ch->p->deck[x]=-1;/*no card*/
    return(card);
}

void discard(struct char_data *ch, char *arg)
{
int x,c;
char p[MAX_INPUT_LENGTH];
char card[MAX_INPUT_LENGTH];
char next[MAX_INPUT_LENGTH];
   
   half_chop(arg,card,next);
   if(!card[0]){
	send_to_char("EXAMPLE:  discard ks 7h Qc ....etc.\n\r",ch);
	return;
   }
   c=0;
   strcpy(p,card);
   while(p[0]){
	if(card[0]&&card[1]){
	    for(x=0;x<52;x++){
		if(!str_cmp(card,&deck[x][0])){
		    if(ch->p->cards[x]!=NO_CARD){
		    	ch->p->cards[x]=NO_CARD;
		    	c++;
		    	sprintf(log_buf,"You discard %s.\n\r",&deck[x][0]);
		    	send_to_char(log_buf,ch);
		    }
		    x=52;
		}
	    }
	}
	if(next[0]){
	   strcpy(p,next);
	   half_chop(p,card,next);
	}else
	   p[0]=MED_NULL;
   }
   if(c){
	sprintf(log_buf,"$n discards %d cards.",c);
	act(log_buf,TRUE,ch,0,0,TO_ROOM);
   }
}

void cards_in_logbuf(struct char_data *ch, struct char_data *player)
{
int x;
    if(!player){
	log_buf[0]=MED_NULL;
	return;
    }
    sprintf(log_buf,"[%15s]-> ",GET_NAME(player));
    for(x=0;x<52;x++){
	if(player->p->cards[x]==HAVE_CARD_DOWN){
	    sprintf(log_buf+strlen(log_buf),"[XX] ");
        }
    }
    for(x=0;x<52;x++){
	if(player->p->cards[x]==HAVE_CARD_HIDDEN){
	    if(ch==player)
		sprintf(log_buf+strlen(log_buf),"(%s) ",&deck[x][0]);
	    else
		sprintf(log_buf+strlen(log_buf),"XX ");
        }
    }
    for(x=0;x<52;x++){
	if(player->p->cards[x]==HAVE_CARD_SHOWING){
	    sprintf(log_buf+strlen(log_buf),"%s ",&deck[x][0]);
        }
    }
    for(x=0;x<52;x++){
	if(player->p->cards[x]==FOLD){
	    sprintf(log_buf+strlen(log_buf),"FF ");
        }
    }
    sprintf(log_buf+strlen(log_buf),"\n\r");
}
void show_cards(struct char_data *ch, char *arg)
{
struct char_data *player=NULL;
char buf[MAX_STRING_LENGTH];

    one_argument(arg,buf);
    if(!buf[0]){
	cards_in_logbuf(ch,ch);
	send_to_char(log_buf,ch);
    }else{
	cards_in_logbuf(ch,ch);
	send_to_char(log_buf,ch);
        for (player= world[ch->in_room]->people;player;player=player->next_in_room){
	    if(player!=ch&&(player->master==ch->master||player->master==ch||ch->master==player)){
		cards_in_logbuf(ch,player);
		send_to_char(log_buf,ch);
            }
        }	
    }
}


void deal(struct char_data *ch, char *arg, int type)
{
struct char_data *player=NULL;
char bufa[MAX_STRING_LENGTH];
char bufb[MAX_STRING_LENGTH];
int num,numplayers,numcards,x,card;
char card_type[25];
    if(ch->master&&ch->master!=ch){
	send_to_char("You are not the dealer, you must be the leader of followers.\n\r",ch);
	return;
    }
    half_chop(arg,bufa,bufb);
    if(!bufa[0]){
	send_to_char("Syntax  showdeal #cards [playername]\n\rIf no name, cards will be dealt to all.\n\r",ch);
	return;
    }
    num=atoi(bufa);
    if(num<1||num>52){
	send_to_char("Number of cards must be from 1 to 52.\n\r",ch);
	return;
    }
    numplayers=0;
    if(!bufb[0])
       for (player= world[ch->in_room]->people;player;player=player->next_in_room)
	   if(player->master==ch)
	       numplayers++;
    numcards=0;
    for(x=0;x<52;x++)
       if(ch->p->deck[x])
	   numcards++;
    if(!bufb[0]){
       if((num*numplayers)>numcards){
	   send_to_char("You don't have enough cards left,\n\r",ch);
	   sprintf(log_buf,"You tried to deal %d cards to %d players with %d cards left.\n\r",num,numplayers,numcards);
	   send_to_char(log_buf,ch);
	   return;
       }
    }else{
       if(num>numcards){
       	   send_to_char("You don't have enough cards left,\n\r",ch);
           return;
       }
    }
    if(type==HAVE_CARD_HIDDEN)
	strcpy(card_type,"");
    else if(type==HAVE_CARD_SHOWING)
	strcpy(card_type,"face up");
    else if(type==HAVE_CARD_DOWN)
	strcpy(card_type,"face down");
    if(!bufb[0]){	/* deal # cards to ALL */
        for (player= world[ch->in_room]->people;player;player=player->next_in_room){
	   if(player->master==ch&&!return_fold(player)){
		for(x=0;x<num;x++){
		    card=return_next_card(ch);
		    player->p->cards[card]=type;
		}		    
		if(player==ch){
		sprintf(log_buf,"$n gives $mself %d card[s] %s.",num,card_type);
		act(log_buf,TRUE,ch,0,0,TO_ROOM);
		sprintf(log_buf,"You give yourself %d card[s] %s.\n\r",num,card_type);
		send_to_char(log_buf,ch);
		}else{
		sprintf(log_buf,"$N gives you %d card[s] %s.",num,card_type);
		act(log_buf,TRUE,player,0,ch,TO_CHAR);
		sprintf(log_buf,"$N gives $n %d card[s] %s.",num,card_type);
		act(log_buf,TRUE,player,0,ch,TO_NOTVICT);
		sprintf(log_buf,"You give %s %d card[s] %s.\n\r",GET_NAME(player),num,card_type);
		send_to_char(log_buf,ch);
		}
	   }
        }
    }else{	/* to a player */
	player=get_char_room_vis(ch, bufb);
	if(!player){
	    sprintf(log_buf,"You don't see %s here.\n\r",bufb);
	    send_to_char(log_buf,ch);
	    return;
	}
	if(return_fold(player)){
	    send_to_char("That player has folded.\n\r",ch);
	    return;
	}
        for(x=0;x<num;x++){
	   card=return_next_card(ch);
	   player->p->cards[card]=type;	
        }		    
	sprintf(log_buf,"$N gives you %d cards[s] %s.",num,card_type);
	act(log_buf,TRUE,player,0,ch,TO_CHAR);
	sprintf(log_buf,"$N gives $n %d card[s] %s.",num,card_type);
	act(log_buf,TRUE,player,0,ch,TO_NOTVICT);
	sprintf(log_buf,"You give %s %d card[s] %s.\n\r",GET_NAME(player),num,card_type);
	send_to_char(log_buf,ch);
    }
}

void shuffle(struct char_data *ch)
{
struct char_data *player=NULL;
int x,y,carda,cardb,a,b;

    if(ch->master&&ch->master!=ch){
	send_to_char("You are not the dealer, you must be the leader of followers.\n\r",ch);
	return;
    }
    for (player= world[ch->in_room]->people;player;player=player->next_in_room){
	if(player->master!=ch)continue;
	for(x=0;x<52;x++){
	    player->p->deck[x]=NO_CARD;
	    player->p->cards[x]=NO_CARD;
	}
	act("$N takes any cards you may have.",TRUE,player,0,ch,TO_CHAR);
	act("You take any cards $N may have.",TRUE,ch,0,player,TO_CHAR);
	act("$n takes any cards $N may have.",TRUE,ch,0,player,TO_NOTVICT);	
    }
    for(x=0;x<52;x++){
	ch->p->deck[x]=NO_CARD;
	ch->p->cards[x]=NO_CARD;
    }
    act("$n puts any cards he has into the deck.",TRUE,ch,0,0,TO_ROOM);
    send_to_char("You put any cards you have into your deck.\n\r",ch);
    for(x=0;x<52;x++)
	ch->p->deck[x]=x;
    y=number(100,1040);
    for(x=0;x<y;x++){
	a=number(0,51);
	b=number(0,51);
	carda=ch->p->deck[a];
	cardb=ch->p->deck[b];
	ch->p->deck[a]=cardb;
	ch->p->deck[b]=carda;
	
    }
    sprintf(log_buf,"$n shuffles the deck %d times.",y/52);
    act(log_buf,TRUE,ch,0,0,TO_ROOM);
    sprintf(log_buf,"You shuffle the deck %d times.\n\r",y/52);
    send_to_char(log_buf,ch);
}
