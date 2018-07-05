/***************************************************************************
*		          MEDIEVIA CyberSpace Code and Data files		           *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*			                All rights reserved				               *
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
#include <math.h>
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "db.h"
#include "handler.h"
#include "limits.h"
#include "spells.h"
#include "holocode.h"
#include "trading.h"

struct room_data *world[MAX_ROOM]; /* array of rooms                  */
extern struct char_data *character_list; /* global l-list of chars          */
extern char global_color;
extern int number_of_rooms;
extern struct char_data *mobs[MAX_MOB];
extern struct descriptor_data *descriptor_list;
extern int giaHoloMobPlacementByLevel[21][2];
extern int dice(int number, int size);
extern bool DigitString(char *szpText);
extern struct char_data *ACTTO; 
extern int number(int from, int to);
extern void remove_room_affect(struct room_affect *ra, char type);
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern struct TRADESHOP gstaTradeShops[MAXTRADINGSHOPS];
extern void MakeTradeShop(int iRoom);

ush_int Holo[MAXHOLO][MAXHOLO];
struct HOLOROOMS *stpHoloRTemplates[256];
long int liFetchNumber(FILE *fp);
int iHighRoom,iNextRoom;/* used to see what room to start a scrolling list of available rooms at*/
int giaMobPlacement[20][50];/* 20 kinds of sectors with up to 50 mobs approved for each */
struct room_data *stpFirstHoloRoom,*stpLastHoloRoom;
struct HoloSurvey *survey_list=NULL;
int igLINKW,igLINKE,igLINKN,igLINKS;
int igHolomade,igHoloReuse;
void DrawHoloMap(struct char_data *ch, int width, int length);
void FreightFromRoom(struct FREIGHT *stpF);
void FreightToRoom(struct FREIGHT *stpF, int iRoom);
void HoloMobilePlacement(int iX, int iY, int iRoom);

char *szaSurveyDists[] = {
"hundreds of miles away in the distance",
"far off in the skyline",
"many miles away at great distance",
"far off many miles away",
"tens of miles away in the distance",
"far off in the distance",
"several miles away",
"off in the distance",
"not far from here",
"in near vicinity",
"in the immediate area"
};


void do_camp(struct char_data *ch, char *argument, int cmd)
{
struct room_affect *stpR;
struct HoloSurvey *stpS;

	for(stpR=world[ch->in_room]->room_afs;stpR;stpR=stpR->next){
		if(stpR->type==RA_CAMP){
			stpR->timer=60;
			global_color=31;
			send_to_char("You throw more wood on the fire and get the campsite ready.\n\r",ch);
			act("$n throws more wood on the fire.",TRUE,ch,0,0,TO_ROOM);
			global_color=0;
			GET_MOVE(ch)/=2;
			return;
		}
	}
	if(!IN_HOLO(ch)){
		send_to_char("You realize you should only camp in the wilderness.\n\r",ch);
		return;
	}	
	if(world[ch->in_room]->sector_type==SECT_WATER_NOSWIM||world[ch->in_room]->sector_type==SECT_WATER_SWIM){
		send_to_char("You laugh at the idea of making a fire here.\n\r",ch);
		return;	   
	}
	GET_MOVE(ch)/=10;
	GET_HIT(ch)/=2;
	CREATE(stpR,struct room_affect,1);
	stpR->type=RA_CAMP;
	stpR->timer=60;
	stpR->value=0;
	stpR->ch=NULL;
	stpR->text=NULL;
	stpR->room=ch->in_room;
	stpR->next=world[ch->in_room]->room_afs;
	world[ch->in_room]->room_afs=stpR;
	global_color=31;
	send_to_char("You busy yourself preparing a campfire..lighting it expertly..it ignites instantly..orange and red flames shoot up from the timber.\n\r",ch);	
	act("$n prepares and expertly lights a campfire...its ignites quickly..orange and red flames shooting up from the timber.",TRUE,ch,0,0,TO_ROOM);
	global_color=0;
	CREATE(stpS,struct HoloSurvey,1);	
	stpS->next=survey_list;
	survey_list=stpS;
	stpS->description=str_dup("you see pillar of smoke rising from a campfire.");
	stpS->iX=world[ch->in_room]->holox;
	stpS->iY=world[ch->in_room]->holoy;
	/* IF YOU CHANGE THIS CHANGE ROOM_AFS.C WHERE ROOM AFFECT CAMP IS REMOVED AND IT REMOVES THIS. IT USES THE DISTANCE TO VERIFY */
	stpS->dist=77;
}

void do_areamap(struct char_data *ch, char *argument, int cmd)
{
int Wid=15, Len=10;
char buf[100],buf2[100];
extern struct weather_data weather_info;
extern struct time_info_data time_info;
extern void do_look(struct char_data *ch,char *argument,int cmd);
extern void do_exits(struct char_data *ch,char *argument,int cmd);

	if(IS_SET(ORIGINAL(ch)->specials.act, PLR_BRIEF) && cmd != 9) {
		do_look(ch,"\0",15);
		if( ch->desc  && !ORIGINAL(ch)->specials.autoexit )
			do_exits(ch,"",9); 
		return;
	}
	if(*argument && GET_LEVEL(ch)>32 )
		argument = one_argument(argument, buf);
	one_argument(argument, buf2);
	if(*buf && *buf2) {
		Wid=atoi(buf);
		Len=atoi(buf2);
		if(Wid<=2 || Wid>=80)
			Wid=15;
		if(Len<=2 || Len>=25)
			Len=10;
	} else {
		Wid=15;
		Len=10;
		if(weather_info.sky==0) {
			Wid *= 1.6;
			Len *= 1.3;
		}
		if(weather_info.sky>=2) {
			Wid *= .5;
			Len *= .7;
		}
		if(time_info.hours >=10 && time_info.hours<=15) {
			Wid *= 1.5;
			Len *= 1.3;
		}
		if(time_info.hours >= 20 && time_info.hours <= 4) {
			Wid *= .7;
			Len *= .8;
		}
		if(!IS_NPC(ch)&&IS_FLYING(ch)){
			Wid=30;
			Len=15;
		}
	}	
 	DrawHoloMap(ch,Wid,Len);
 	send_to_char("\033[0m\033[0m",ch);
 	return;
}

void DrawHoloMap(struct char_data *ch, int width, int length) 
{
int iLeft, iRight, iTop, iBottom;
int iChHere[2];
register int iVertLoop, iHorzLoop; 
int iLastColor=100, iTemplate;
  /* reminder -> stpHoloRTemplates */
char cTemp;
char buf[8600];
bool bColor=0;
  
  *buf = '\0';
  cTemp = '\0';
  
  if(ch->specials.ansi_color==69)
  	bColor = TRUE;
  	
  iChHere[0] = HOLOX(ch);
  iChHere[1] = HOLOY(ch);

  if(!IS_NPC(ch)&&IS_FLYING(ch)){
  	iChHere[0]=ch->p->stpFlying->iX;
  	iChHere[1]=ch->p->stpFlying->iY;
	switch(ch->p->stpFlying->iDir){
		case NORTH:
		  	sprintf(buf,"%sFlying Northbound...%s\n\r",YEL(ch),NRM(ch));
			break;			
		case SOUTH:
		  	sprintf(buf,"%sFlying Southbound...%s\n\r",YEL(ch),NRM(ch));
			break;			
		case EAST:
		  	sprintf(buf,"%sFlying Eastbound...%s\n\r",YEL(ch),NRM(ch));
			break;			
		case WEST:
		  	sprintf(buf,"%sFlying Westbound...%s\n\r",YEL(ch),NRM(ch));
			break;			
		case -1:
		  	sprintf(buf,"%sFlying in circles...%s\n\r",YEL(ch),NRM(ch));
			break;			
		default:
			SUICIDE;
			return;
	}
  }else{
	  sprintf(buf,"%s%s%s\n\r",YEL(ch),world[ch->in_room]->name,NRM(ch));
  }   
  iLeft = iChHere[0] - (width/2);
  iRight = iChHere[0] + (width/2);
  iTop = iChHere[1] + (length/2);
  iBottom = iChHere[1] - (length/2);
  
  if(iLeft < 0) 		iLeft = 0;
  if(iRight > 1999)		iRight = 1999;
  if(iTop > 1999)		iTop = 1999;
  if(iBottom < 0)		iBottom = 0;
  
  /* Dont send the room name now!, the buf is just appended by map */
  for(iVertLoop = iBottom;iVertLoop <= iTop;iVertLoop++) {
  		for(iHorzLoop = iLeft;iHorzLoop <= iRight;iHorzLoop++) {
 		/* If a HOLOROOM is loaded at this x,y */
 		
  			if(Holo[iHorzLoop][iVertLoop] > 255) {
  				iTemplate = world[Holo[iHorzLoop][iVertLoop]]->holoroom;
  				/* The You Are Here # Symbol */
  				if(iHorzLoop == iChHere[0] && iVertLoop == iChHere[1]) {
  					cTemp = '#';
  					if(bColor)  						
  						strcat(buf,"\033[1m\033[37m");
  					iLastColor = 37;
  				} else {
  				/* Otherwise Draw a Map char */	
  					cTemp = stpHoloRTemplates[iTemplate]->cMapChar;
  					  /* if color, check if you need to change, if so, change colors */
  					  if(bColor)
  					    if(stpHoloRTemplates[iTemplate]->siMapColor)
  						if(stpHoloRTemplates[iTemplate]->siMapColor != iLastColor) {
  							if(stpHoloRTemplates[iTemplate]->siMapColor > 100 && (iLastColor > 100)) 
  								sprintf(buf,"%s\033[%dm",buf,(stpHoloRTemplates[iTemplate]->siMapColor-100));
  							else if(stpHoloRTemplates[iTemplate]->siMapColor > 100)
  								sprintf(buf,"%s\033[0m\033[%dm",buf,(stpHoloRTemplates[iTemplate]->siMapColor-100));
  							else if(iLastColor < 100)	
  								sprintf(buf,"%s\033[%dm",buf,stpHoloRTemplates[iTemplate]->siMapColor);
  							else 
  								sprintf(buf,"%s\033[1m\033[%dm",buf,stpHoloRTemplates[iTemplate]->siMapColor);
 							iLastColor = stpHoloRTemplates[iTemplate]->siMapColor;
  						}
  				}
  				buf[(strlen(buf)+1)]=NULL;
  				buf[strlen(buf)]=cTemp;
  			} else {				
  				iTemplate = Holo[iHorzLoop][iVertLoop];
  				cTemp = stpHoloRTemplates[iTemplate]->cMapChar;
  				if(bColor)
  					if(stpHoloRTemplates[iTemplate]->siMapColor)
  					if(stpHoloRTemplates[iTemplate]->siMapColor != iLastColor) {
  							if(stpHoloRTemplates[iTemplate]->siMapColor > 100 && (iLastColor > 100)) 
  								sprintf(buf,"%s\033[%dm",buf,(stpHoloRTemplates[iTemplate]->siMapColor-100));
  							else if(stpHoloRTemplates[iTemplate]->siMapColor > 100)
  								sprintf(buf,"%s\033[0m\033[%dm",buf,(stpHoloRTemplates[iTemplate]->siMapColor-100));
  							else if(iLastColor < 100)	
  								sprintf(buf,"%s\033[%dm",buf,stpHoloRTemplates[iTemplate]->siMapColor);
  							else 
  								sprintf(buf,"%s\033[1m\033[%dm",buf,stpHoloRTemplates[iTemplate]->siMapColor);
 					
 							iLastColor = stpHoloRTemplates[iTemplate]->siMapColor;
  					}
  
  				buf[(strlen(buf)+1)]=NULL;
  				buf[strlen(buf)]=cTemp;
  			}

  		}
  	strcat(buf,"\r\n");
  }
  send_to_char(buf,ch);
 
}  
  
int iHoloChDistance(struct char_data *ch, struct char_data *vict)
{
int iDist=0;
extern double dHoloDistance(int iX1,int iY1,int iX2,int iY2);

	if(!ch || !vict) return(0);

	iDist = (int) dHoloDistance(
		HOLOX(ch),
		HOLOY(ch),
		HOLOX(vict),
		HOLOY(vict));
	
	return(iDist);
}



void do_survey(struct char_data *ch, char *argument, int cmd)
{
struct HoloSurvey *stpServ;
int iDistance;
int iAngle;
char buf[8192];
bool found=0;
extern double dHoloDistance(int iX1,int iY1,int iX2,int iY2);
extern int iHoloWhere(int iX1, int iY1, int iX2, int iY2, int *ipDistan);
char cDir=-1;
int iMes=0;

	if(!ch) return;
	/* iHoloWhere */
	
	*buf = '\0';

	for(stpServ = survey_list; stpServ; stpServ = stpServ->next) {
		iDistance = (int) dHoloDistance(
			(int) HOLOX(ch),
			(int) HOLOY(ch),
			(int) stpServ->iX,
			(int) stpServ->iY);
		/* Save the math if it's too far away anyway */
		if(iDistance <= stpServ->dist) {
			found = 1;
			iAngle = iHoloWhere(
				(int) HOLOX(ch),
				(int) HOLOY(ch),
				(int) stpServ->iX,
				(int) stpServ->iY,
				&iDistance);
			if(iAngle == -1)
				cDir = 13;
			else if(iAngle >= 360)
				cDir = 12;
			else if(iAngle >= 330)
				cDir = 11;
			else if(iAngle >= 300)
				cDir = 10;
			else if(iAngle >= 270)
				cDir = 9;
			else if(iAngle >= 240)
				cDir = 8;
			else if(iAngle >= 210)
				cDir = 7;
			else if(iAngle >= 180)
				cDir = 6;
			else if(iAngle >= 150)
				cDir = 5;
			else if(iAngle >= 120)
				cDir = 4;
			else if(iAngle >= 90)
				cDir = 3;
			else if(iAngle >= 60)
				cDir = 2;
			else if(iAngle >= 30)
				cDir = 1;
			else if(iAngle >= 0)
				cDir = 12;

			if(iDistance > 200)
				iMes = 0;
			else if(iDistance > 150)
				iMes = 1;
			else if(iDistance > 100)
				iMes = 2;
			else if(iDistance > 75)
				iMes = 3;
			else if(iDistance > 50)
				iMes = 4;
			else if(iDistance > 25)
				iMes = 5;
			else if(iDistance > 15)
				iMes = 6;
			else if(iDistance > 10)
				iMes = 7;
			else if(iDistance > 5)
				iMes = 8;
			else if(iDistance > 1)
				iMes = 9;
			else 
				iMes = 10;
			
			if(cDir == 13)
				sprintf(buf,"In the immediate area, %s.\r\n", stpServ->description ? stpServ->description : "<NULL>");
			else	
				sprintf(buf,"At %d o'clock, %s %s.\r\n", cDir, szaSurveyDists[(int) iMes], stpServ->description ? stpServ->description : "<NULL PLEASE REPORT>");
			send_to_char(buf,ch);
			/* Some God Stuff */
			sprintf(buf,"[DFO: %d] [(%d,%d)]\r\n",
				iDistance, stpServ->iX, stpServ->iY);
			if(GET_LEVEL(ch)>=32)
				send_to_char(buf,ch);				
		}		
	}
	if(!found)
		send_to_char("Your survey of the area yields nothing special.\r\n",ch);
}

void LoadSurveyList(void)
{
FILE *fl;
int iNum=0,iTemp;
struct HoloSurvey *stpServ;
char szTag[8192];

	if(!(fl=fopen("../lib/medievia.survey", "r"))){
		perror("Opening medievia.survey");
		SUICIDE;
	}
	fscanf(fl," %s ",szTag);
	while(1){
		if(szTag[0]=='$')
			break;
		CREATE(stpServ,struct HoloSurvey, 1);
		iNum++;
		while(1) {
			fscanf(fl," %s ",szTag);
			if(szTag[0]=='#'||szTag[0]=='$') {
				stpServ->next = survey_list;
				survey_list = stpServ;
				break;
			}
			if(!strncasecmp(szTag,"DSC",3)){
				stpServ->description = fread_string(fl);
			}else if(!strncasecmp(szTag,"XVAL",4)) {
				fscanf(fl," %d ", &iTemp);
				stpServ->iX = iTemp;
			}else if(!strncasecmp(szTag,"YVAL",4)) {
				fscanf(fl," %d ", &iTemp);
				stpServ->iY = iTemp;
			}else if(!strncasecmp(szTag,"DIST",4)) {
				fscanf(fl," %d ", &iTemp);
				stpServ->dist = iTemp;
			}else if(!strncasecmp(szTag,"BEACON",6)) {
				SET_BIT(stpServ->siFlags, FLAG_BEACON);
			}else if(!strncasecmp(szTag,"NOSEEDARK",9)) {
				SET_BIT(stpServ->siFlags, FLAG_NOSEEDARK);
			}else if(!strncasecmp(szTag,"GLOW",4)) {
				SET_BIT(stpServ->siFlags, FLAG_GLOW);
			}
		}
	}
	fclose(fl);
	fprintf(stderr,"HOLOCODE:  Survey objects... %d loaded into memory.\n",iNum); 
}

int iGetNextRoom(void)
{
int x;

	for(x=iNextRoom;x>10000;x--){
		if(!world[x]){
			iNextRoom=x-1;
			return(x);
		}
	}
	for(x=iHighRoom;x>=iNextRoom;x--){
		if(!world[x]){
			iNextRoom=x-1;
			return(x);
		}
	}
	iNextRoom=0;/*flag holomakers to gointo re-use mode*/
	log_hd("## HCode entering into re-use mode");
	return(-1);
}

int iMakeHoloRoom(int x,int y)
{
int iRoom=0;
unsigned short int usiMap;
struct char_data *vict=NULL, *next_v=NULL;
struct obj_data *obj=NULL, *next_o=NULL;
struct room_affect *ra=NULL,*ran=NULL;
struct FREIGHT *stpF=NULL,*stpFNext=NULL;

	if(Holo[x][y]>255){

/******************************************************************/
/******************************************************************/
/******************************************************************/
              /* TO BE REMOVED AFTER INITIAL TESTING*/
		if(world[Holo[x][y]]&&(world[Holo[x][y]]->holox!=x||world[Holo[x][y]]->holoy!=y)){
			log_hd("##Holocode seems confused, error 6130");
			SUICIDE;
		}		
/******************************************************************/
/******************************************************************/
/******************************************************************/
		if(world[Holo[x][y]]){
			return(Holo[x][y]);
		}else{
			log_hd("##Holocode seems confused, error 6150");
			SUICIDE;
		}
	}	
	if(!stpHoloRTemplates[Holo[x][y]]){
		sprintf(log_buf,"## need to make HoloRoom %d at %d by %d",Holo[x][y],x,y);
		log_hd(log_buf);
		return(0);		
	}
	if(Holo[x][y]==215)/*blocking link room*/
		return(0);
	if(iNextRoom)
		iRoom=iGetNextRoom();
	if(!iNextRoom){/*reuse mode*/
		igHoloReuse++; 
		iRoom=stpLastHoloRoom->number;
		stpLastHoloRoom=stpLastHoloRoom->holoprev;
		stpLastHoloRoom->holonext=NULL;
		Holo[world[iRoom]->holox][world[iRoom]->holoy]=world[iRoom]->holoroom;
		if(world[iRoom]->dir_option[NORTH])
			world[iRoom]->dir_option[NORTH]=my_free(world[iRoom]->dir_option[NORTH]);
		if(world[iRoom]->dir_option[SOUTH])
			world[iRoom]->dir_option[SOUTH]=my_free(world[iRoom]->dir_option[SOUTH]);
		if(world[iRoom]->dir_option[EAST])
			world[iRoom]->dir_option[EAST]=my_free(world[iRoom]->dir_option[EAST]);
		if(world[iRoom]->dir_option[WEST])
			world[iRoom]->dir_option[WEST]=my_free(world[iRoom]->dir_option[WEST]);
		if(world[iRoom]->dir_option[UP])
			world[iRoom]->dir_option[UP]=my_free(world[iRoom]->dir_option[UP]);
		if(world[iRoom]->dir_option[DOWN])
			world[iRoom]->dir_option[DOWN]=my_free(world[iRoom]->dir_option[DOWN]);
		/* stick freight in void cause room is being reused... when room is remade they will be put back */
		for(stpF=world[iRoom]->stpFreight;stpF;stpF=stpFNext){
			stpFNext=stpF->stpNextInRoom;
			FreightFromRoom(stpF);
			FreightToRoom(stpF,0);
			stpF->iLocationX=world[iRoom]->holox;
			stpF->iLocationX=world[iRoom]->holoy;
		}
		for(vict=world[iRoom]->people;vict;vict=next_v){
			next_v=vict->next_in_room;
			if(IS_NPC(vict)&&!IS_DEAD(vict)){
				extract_char(vict,TRUE);
			}else{
				sprintf(log_buf,"##HCode re-using a room with a player(%s)",GET_NAME(vict));
				log_hd(log_buf);				
			}
		}                                                                                                                                            
		for(obj=world[iRoom]->contents;obj;obj=next_o){
			next_o=obj->next_content;
			if(obj->item_number!=9811)
				extract_obj(obj);
		}                                                                                                     
		for(ra=world[iRoom]->room_afs;ra;ra=ran){
			ran=ra->next;
			remove_room_affect(ra,0);			
		}                                                                                                                                                
		usiMap=Holo[x][y];
		Holo[x][y]=iRoom;
	}else{	
		igHolomade++;
		usiMap=Holo[x][y];
		Holo[x][y]=iRoom;
		CREATE(world[iRoom],struct room_data, 1);
		number_of_rooms++;
	}
	world[iRoom]->number=iRoom;
	world[iRoom]->stpFreight=NULL;
	world[iRoom]->funct=NULL;
	world[iRoom]->name=stpHoloRTemplates[usiMap]->name;
	while((world[iRoom]->description=stpHoloRTemplates[usiMap]->description[number(0,9)])==NULL);
	world[iRoom]->zone=197;
	world[iRoom]->room_flags=stpHoloRTemplates[usiMap]->room_flags;
	world[iRoom]->sector_type=stpHoloRTemplates[usiMap]->sector_type;
	world[iRoom]->extra_flags=stpHoloRTemplates[usiMap]->extra_flags;
	world[iRoom]->class_restriction=stpHoloRTemplates[usiMap]->class_restriction;
	world[iRoom]->level_restriction=stpHoloRTemplates[usiMap]->level_restriction;
	world[iRoom]->mount_restriction=stpHoloRTemplates[usiMap]->mount_restriction;
	world[iRoom]->move_mod=stpHoloRTemplates[usiMap]->move_mod;
	world[iRoom]->pressure_mod=stpHoloRTemplates[usiMap]->pressure_mod;
	world[iRoom]->temperature_mod=stpHoloRTemplates[usiMap]->temperature_mod;
	world[iRoom]->holoroom=usiMap;
	world[iRoom]->holox=x;
	world[iRoom]->holoy=y;
	world[iRoom]->holoprev=NULL;
	world[iRoom]->holonext=stpFirstHoloRoom;
	if(usiMap==216)
		MakeTradeShop(iRoom);
	stpFirstHoloRoom=world[iRoom];
	if(!stpLastHoloRoom){
		stpLastHoloRoom=stpFirstHoloRoom;
	}
	/* see if any freight was in this room when it was remade */
	for(stpF=world[0]->stpFreight;stpF;stpF=stpFNext){
		stpFNext=stpF->stpNextInRoom;
		if(stpF->iLocationX==x&&stpF->iLocationY==y){
			FreightFromRoom(stpF);
			FreightToRoom(stpF,iRoom);
		}
	}
	if(Holo[x-1][y]>255||Holo[x-1][y]==215){
		if(Holo[x-1][y]==215){
			CREATE(world[iRoom]->dir_option[WEST], struct room_direction_data, 1);
			world[iRoom]->dir_option[WEST]->to_room=igLINKW;
			CREATE(world[igLINKW]->dir_option[EAST], struct room_direction_data, 1);
			world[igLINKW]->dir_option[EAST]->to_room=iRoom;
		}else{
			CREATE(world[iRoom]->dir_option[WEST], struct room_direction_data, 1);
			world[iRoom]->dir_option[WEST]->to_room=Holo[x-1][y];
			CREATE(world[Holo[x-1][y]]->dir_option[EAST], struct room_direction_data, 1);
			world[Holo[x-1][y]]->dir_option[EAST]->to_room=iRoom;
		}					
	}	
	if(Holo[x+1][y]>255||Holo[x+1][y]==215){
		if(Holo[x+1][y]==215){
			CREATE(world[iRoom]->dir_option[EAST], struct room_direction_data, 1);
			world[iRoom]->dir_option[EAST]->to_room=igLINKE;
			CREATE(world[igLINKE]->dir_option[WEST], struct room_direction_data, 1);
			world[igLINKE]->dir_option[WEST]->to_room=iRoom;
		}else{
			CREATE(world[iRoom]->dir_option[EAST], struct room_direction_data, 1);
			world[iRoom]->dir_option[EAST]->to_room=Holo[x+1][y];
			CREATE(world[Holo[x+1][y]]->dir_option[WEST], struct room_direction_data, 1);
			world[Holo[x+1][y]]->dir_option[WEST]->to_room=iRoom;
		}					
	}	
	if(Holo[x][y-1]>255||Holo[x][y-1]==215){
		if(Holo[x][y-1]==215){
			CREATE(world[iRoom]->dir_option[NORTH], struct room_direction_data, 1);
			world[iRoom]->dir_option[NORTH]->to_room=igLINKN;
			CREATE(world[igLINKN]->dir_option[SOUTH], struct room_direction_data, 1);
			world[igLINKN]->dir_option[SOUTH]->to_room=iRoom;
		}else{
			CREATE(world[iRoom]->dir_option[NORTH], struct room_direction_data, 1);
			world[iRoom]->dir_option[NORTH]->to_room=Holo[x][y-1];
			CREATE(world[Holo[x][y-1]]->dir_option[SOUTH], struct room_direction_data, 1);
			world[Holo[x][y-1]]->dir_option[SOUTH]->to_room=iRoom;
		}					
	}	
	if(Holo[x][y+1]>255||Holo[x][y+1]==215){
		if(Holo[x][y+1]==215){
			CREATE(world[iRoom]->dir_option[SOUTH], struct room_direction_data, 1);
			world[iRoom]->dir_option[SOUTH]->to_room=igLINKS;
			CREATE(world[igLINKS]->dir_option[NORTH], struct room_direction_data, 1);
			world[igLINKS]->dir_option[NORTH]->to_room=iRoom;
		}else{
			CREATE(world[iRoom]->dir_option[SOUTH], struct room_direction_data, 1);
			world[iRoom]->dir_option[SOUTH]->to_room=Holo[x][y+1];
			CREATE(world[Holo[x][y+1]]->dir_option[NORTH], struct room_direction_data, 1);
			world[Holo[x][y+1]]->dir_option[NORTH]->to_room=iRoom;
		}					
	}	
   	HoloMobilePlacement(x,y,iRoom);
	return(iRoom);
}

void TouchRooms(int room)
{
	if(world[room]->holoprev){
		world[room]->holoprev->holonext=world[room]->holonext;
		if(!world[room]->holoprev->holonext)
			stpLastHoloRoom=world[room]->holoprev->holonext;
		world[room]->holonext=stpFirstHoloRoom;
		stpFirstHoloRoom=world[room];
	}
	if(world[room]->dir_option[NORTH]){
		room=world[room]->dir_option[NORTH]->to_room;
		if(world[room]->holoprev){
			world[room]->holoprev->holonext=world[room]->holonext;
			if(!world[room]->holoprev->holonext)
				stpLastHoloRoom=world[room]->holoprev->holonext;
			world[room]->holonext=stpFirstHoloRoom;
			stpFirstHoloRoom=world[room];
		}
	}
	if(world[room]->dir_option[SOUTH]){
		room=world[room]->dir_option[SOUTH]->to_room;
		if(world[room]->holoprev){
			world[room]->holoprev->holonext=world[room]->holonext;
			if(!world[room]->holoprev->holonext)
				stpLastHoloRoom=world[room]->holoprev->holonext;
			world[room]->holonext=stpFirstHoloRoom;
			stpFirstHoloRoom=world[room];
		}
	}
	if(world[room]->dir_option[EAST]){
		room=world[room]->dir_option[EAST]->to_room;
		if(world[room]->holoprev){
			world[room]->holoprev->holonext=world[room]->holonext;
			if(!world[room]->holoprev->holonext)
				stpLastHoloRoom=world[room]->holoprev->holonext;
			world[room]->holonext=stpFirstHoloRoom;
			stpFirstHoloRoom=world[room];
		}
	}
	if(world[room]->dir_option[WEST]){
		room=world[room]->dir_option[WEST]->to_room;
		if(world[room]->holoprev){
			world[room]->holoprev->holonext=world[room]->holonext;
			if(!world[room]->holoprev->holonext)
				stpLastHoloRoom=world[room]->holoprev->holonext;
			world[room]->holonext=stpFirstHoloRoom;
			stpFirstHoloRoom=world[room];
		}
	}
	if(world[room]->dir_option[UP]){
		room=world[room]->dir_option[UP]->to_room;
		if(world[room]->holoprev){
			world[room]->holoprev->holonext=world[room]->holonext;
			if(!world[room]->holoprev->holonext)
				stpLastHoloRoom=world[room]->holoprev->holonext;
			world[room]->holonext=stpFirstHoloRoom;
			stpFirstHoloRoom=world[room];
		}
	}
	if(world[room]->dir_option[DOWN]){
		room=world[room]->dir_option[DOWN]->to_room;
		if(world[room]->holoprev){
			world[room]->holoprev->holonext=world[room]->holonext;
			if(!world[room]->holoprev->holonext)
				stpLastHoloRoom=world[room]->holoprev->holonext;
			world[room]->holonext=stpFirstHoloRoom;
			stpFirstHoloRoom=world[room];
		}
	}
}

void LoadLinks(void)
{
FILE *fp;
unsigned short int usiX,usiY;
char cDir;
int iRoom,iNumLinks=0,iX,iY;

	if((fp=fopen("../lib/medievia.hololinks","r"))==NULL){
		perror("Could not open medievia.links");
		exit(1);
	}
	fprintf(stderr,"HOLOCODE:  Loading medievia.links..\n");	
	while(1){
		fscanf(fp," %d %d %c %d \n",&iX,&iY,&cDir,&iRoom);
		usiX=iX;
		usiY=iY;
		if(cDir=='x')
			break;
		igLINKE=0;
		igLINKW=0;
		igLINKN=0;
		igLINKS=0;
		switch(cDir){
			case 'N':
				igLINKN=iRoom;
				break;
			case 'S':
				igLINKS=iRoom;
				break;
			case 'E':
				igLINKE=iRoom;
				break;
			case 'W':
				igLINKW=iRoom;
				break;
		}
		iMakeHoloRoom(usiX,usiY);
		iNumLinks++;
	}
	fclose(fp);
	fprintf(stderr,"HOLOCODE:  %d Rooms(links) made\n",iNumLinks);
	iHighRoom-=iNumLinks;
}

void LoadMap(void)
{
FILE *fp;
int x,y,bytes;
unsigned char c;

	if((fp=fopen("../lib/medievia.raw","rb"))==NULL){
		perror("Could not open medievia.raw");
		exit(1);
	}
	fprintf(stderr,"HOLOCODE:  Loading medievia.raw\n");	
	for(y=0;y<MAXHOLO;y++){
	    for(x=0;x<MAXHOLO;x++){
			bytes=fread(&c,sizeof(c),1,fp);
			if(bytes<1){
				perror("Read problem reading medievia.raw");
				SUICIDE;
			}
			Holo[x][y]=c;
		}
		if((y%25)==0)
			fprintf(stderr,"*");
	}
	fprintf(stderr,"\n");
	fclose(fp);
}

void LoadMobPlacementData(void)
{
FILE *fl;
char szTag[8192];
int iSect,iSlot,iMob;

    if(!(fl=fopen("../lib/medievia.holomobplacement", "r"))){
		perror("Opening medievia.holomobplacement");
		SUICIDE;
   	}
	for(iSect=0;iSect<20;iSect++)
		for(iSlot=0;iSlot<50;iSlot++)
			giaMobPlacement[iSect][iSlot]=0;
	iSect=0;iSlot=0;
    fprintf(stderr,"HOLOCODE:  Loading Mobile Placement Data.\n");
	while(1){
		fscanf(fl," %s ",szTag);
		if(szTag[0]=='$')
			break;
		if(!str_cmp(szTag,"SECT_INSIDE")){
			iSect=0;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_CITY")){
			iSect=1;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_FIELD")){
			iSect=2;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_FOREST")){
			iSect=3;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_HILLS")){
			iSect=4;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_MOUNTAIN")){
			iSect=5;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_WATER_SWIM")){
			iSect=6;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_WATER_NOSWIM")){
			iSect=7;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_DESERT")){
			iSect=8;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_UNDERWATER")){
			iSect=9;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_AIR")){
			iSect=10;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_SWAMP")){
			iSect=11;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_JUNGLE")){
			iSect=12;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_ARTIC")){
			iSect=13;
			iSlot=0;
		}else if(!str_cmp(szTag,"SECT_MANA")){
			iSect=14;
			iSlot=0;
		}else if(DigitString(szTag)){
			iMob=atoi(szTag);
			if(iMob<0||iMob>MAX_MOB)
				SUICIDE;
			if(!mobs[iMob])
				SUICIDE;
			giaMobPlacement[iSect][iSlot++]=iMob;
		}else{
			SUICIDE;
		}
	}
	fclose(fl);
}

void LoadHoloTemplateRooms(void)
{
FILE *fl;
char szTag[8192],sTemp[100];
int iR,iDescs,iTemp;

    if(!(fl=fopen("../lib/medievia.holorooms", "r"))){
		perror("Opening medievia.holorooms");
		SUICIDE;
   	}
    fprintf(stderr,"HOLOCODE:  Loading and organizing medievia holotemplates.\n");
	fscanf(fl," %s ",szTag);
	while(1){
		if(szTag[0]=='$')
			break;
		iR=atoi(&szTag[1]);		
		if(iR<0||iR>255)
			SUICIDE;
		CREATE(stpHoloRTemplates[iR],struct HOLOROOMS,1);
		/* Intialize Map Chars if entry dosn't have one. */
		stpHoloRTemplates[iR]->cMapChar = ' ';
		stpHoloRTemplates[iR]->siMapColor = 0;								
		iDescs=0;
		while(1){
			fscanf(fl," %s ",szTag);
			if(szTag[0]=='#'||szTag[0]=='$')
				break;
			if(!strncasecmp(szTag,"NAM",3)){
				stpHoloRTemplates[iR]->name=fread_string(fl);
			}else if(!strncasecmp(szTag,"DSC",3)){
				stpHoloRTemplates[iR]->description[iDescs++]=fread_string(fl);
			}else if(!strncasecmp(szTag,"!CL",3)){
				stpHoloRTemplates[iR]->class_restriction&=NO_CLERIC;
			}else if(!strncasecmp(szTag,"!MA",3)){
				stpHoloRTemplates[iR]->class_restriction|=NO_MAGE;
			}else if(!strncasecmp(szTag,"!TH",3)){
				stpHoloRTemplates[iR]->class_restriction|=NO_THIEF;
			}else if(!strncasecmp(szTag,"!WA",3)){
				stpHoloRTemplates[iR]->class_restriction|=NO_WARRIOR;
			}else if(!strncasecmp(szTag,"NOW",3)){
				stpHoloRTemplates[iR]->extra_flags|=TRADE_NO_WAGON;
			}else if(!strncasecmp(szTag,"NOH",3)){
				stpHoloRTemplates[iR]->extra_flags|=TRADE_NO_HORSE;
			}else if(!strncasecmp(szTag,"ARC",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_ARCTIC;
			}else if(!strncasecmp(szTag,"NOM",3)){
				stpHoloRTemplates[iR]->extra_flags|=TRADE_NO_MULE;
			}else if(!strncasecmp(szTag,"DRI",3)){
				stpHoloRTemplates[iR]->room_flags|=DRINKROOM;
			}else if(!strncasecmp(szTag,"AIR",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_AIR;
			}else if(!strncasecmp(szTag,"BLO",3)){
				stpHoloRTemplates[iR]->room_flags|=BLOCKING;
			}else if(!strncasecmp(szTag,"CHA",3)){
				stpHoloRTemplates[iR]->room_flags|=CHAOTIC;
			}else if(!strncasecmp(szTag,"CIT",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_CITY;
			}else if(!strncasecmp(szTag,"DAR",3)){
				stpHoloRTemplates[iR]->room_flags|=DARK;
			}else if(!strncasecmp(szTag,"DEA",3)){
				stpHoloRTemplates[iR]->room_flags|=DEATH;
			}else if(!strncasecmp(szTag,"DES",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_DESERT;
			}else if(!strncasecmp(szTag,"GOD",3)){
				stpHoloRTemplates[iR]->room_flags|=GODPROOF;
			}else if(!strncasecmp(szTag,"FIE",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_FIELD;
			}else if(!strncasecmp(szTag,"FOR",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_FOREST;
			}else if(!strncasecmp(szTag,"HIL",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_HILLS;
			}else if(!strncasecmp(szTag,"HUR",3)){
				fscanf(fl," %dd%d+%d ",
						&stpHoloRTemplates[iR]->hurt_num_die,
						&stpHoloRTemplates[iR]->hurt_size_die,
						&stpHoloRTemplates[iR]->hurt_base);
			}else if(!strncasecmp(szTag,"IND",3)){
				stpHoloRTemplates[iR]->room_flags|=INDOORS;
			}else if(!strncasecmp(szTag,"INS",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_INSIDE;
			}else if(!strncasecmp(szTag,"JUN",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_JUNGLE;
			}else if(!strncasecmp(szTag,"LAW",3)){
				stpHoloRTemplates[iR]->room_flags|=LAWFULL;
			}else if(!strncasecmp(szTag,"MAN",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_MANA;
			}else if(!strncasecmp(szTag,"MOU",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_MOUNTAIN;
			}else if(!strncasecmp(szTag,"NOS",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_WATER_NOSWIM;
			}else if(!strncasecmp(szTag,"NO_",3)){
				stpHoloRTemplates[iR]->room_flags|=NO_MOB;
			}else if(!strncasecmp(szTag,"NEU",3)){
				stpHoloRTemplates[iR]->room_flags|=NEUTRAL;
			}else if(!strncasecmp(szTag,"PRI",3)){
				stpHoloRTemplates[iR]->room_flags|=PRIVATE;
			}else if(!strncasecmp(szTag,"SWA",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_SWAMP;
			}else if(!strncasecmp(szTag,"SWI",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_WATER_SWIM;
			}else if(!strncasecmp(szTag,"TUN",3)){
				stpHoloRTemplates[iR]->room_flags|=TUNNEL;
			}else if(!strncasecmp(szTag,"UND",3)){
				stpHoloRTemplates[iR]->sector_type=SECT_UNDERWATER;
			}else if(!strncasecmp(szTag,"MCHAR",5)){
				fscanf(fl," %s ", sTemp);
				stpHoloRTemplates[iR]->cMapChar	= sTemp[0];
			}else if(!strncasecmp(szTag,"MCOL",4)){
				fscanf(fl," %d ", &iTemp);
				stpHoloRTemplates[iR]->siMapColor = iTemp;
			}	
		}			
	}
	fclose(fl);
}

void BootHoloCode(void)
{
	iNextRoom=iHighRoom-1;
	LoadMap();
	LoadHoloTemplateRooms();
	LoadLinks();
    LoadSurveyList();
	LoadMobPlacementData();
	stpFirstHoloRoom=NULL;
	stpLastHoloRoom=NULL;
	igHolomade=0;
	igHoloReuse=0;
}

long int liFetchNumber(FILE *fp)
{
char szData[81];
long lTmp, lTmp2, lTmp3, lOutcome;

	fscanf(fp," %s ",szData);
	if(charinstring(szData,'d')&&charinstring(szData,'+')){
		sscanf(szData, " %ldd%ld+%ld ", &lTmp, &lTmp2, &lTmp3);
		lOutcome=dice(lTmp, lTmp2)+lTmp3;
    }else{
    	lOutcome=atol(szData);
    }
	return(lOutcome);
}

double dHoloDistance(int iX1,int iY1,int iX2,int iY2)
{
double dChangeX, dChangeY;
double dDist;
	dChangeX = (iX1 - iX2);
	dChangeX *= dChangeX;
	dChangeY = (iY1 - iY2);
	dChangeY *= dChangeY;
	dDist = sqrt( (double) (dChangeX + dChangeY) );
	return(dDist);
}

int iHoloWhere(int iX1, int iY1, int iX2, int iY2, int *ipDistan)
{
int iNx1=0, iNy1=0, iNx2, iNy2, iNx3, iNy3;
double dDist1, dDist2;
double dTandeg, dDeg;
int iFinal;
	iNx2 = iX2 - iX1;
	iNy2 = iY2 - iY1;
	iNx3 = 0;
	iNy3 = iNy2;
	
	*ipDistan = (int) dHoloDistance(iNx1, iNy1, iNx2, iNy2);
	/* Get rid of some nasty situations */
	
	if(iNx2 == 0 && iNy2 == 0) return(-1);
	if(iNx2 == 0 && iNy2 > 0) return(180);
	if(iNx2 == 0 && iNy2 < 0) return(0);
	if(iNy2 == 0 && iNx2 > 0) return(90);
	if(iNy2 == 0 && iNx2 < 0) return(270);
	
	/* AJCENT */
	dDist1	= dHoloDistance(iNx1, iNy1, iNx3, iNy3);
	/* OPP */
	dDist2 = dHoloDistance(iNx3, iNy3, iNx2, iNy2);
	
	dTandeg = dDist2 / dDist1;
	dDeg = atan(dTandeg);

    	iFinal = (dDeg*180)/3.14159265358979323846;
    
	if(iNx2 > 0 && iNy2 > 0)
		return( (90 + (90-iFinal) ) );
	if(iNx2 > 0 && iNy2 < 0)
		return(iFinal);
	if(iNx2 < 0 && iNy2 > 0)
		return( (180+iFinal) );
	if(iNx2 < 0 && iNy2 < 0)
		return( (270 + (90-iFinal) ) );
	return(-1);

}

void HoloMobilePlacement(int iX, int iY, int iRoom)
{
int iDist,iMinLevel,iMaxLevel,iaMobs[50],iNumMobsPerSector,x;
struct char_data *stpMob;

	if(number(0,100)>10)
		return;
	iDist=(int)(dHoloDistance(1000,1000,iX,iY));	
	iMinLevel=giaHoloMobPlacementByLevel[iDist/50][0];	
	iMaxLevel=giaHoloMobPlacementByLevel[iDist/50][1];
	for(iNumMobsPerSector=0;iNumMobsPerSector<50;iNumMobsPerSector++)
		iaMobs[iNumMobsPerSector]=0;
	iNumMobsPerSector=0;
	for(x=0;giaMobPlacement[world[iRoom]->sector_type][x]&&x<50;x++){
		if(GET_LEVEL(mobs[giaMobPlacement[world[iRoom]->sector_type][x]])>=iMinLevel&&GET_LEVEL(mobs[giaMobPlacement[world[iRoom]->sector_type][x]])<=iMaxLevel){
			iaMobs[iNumMobsPerSector++]=giaMobPlacement[world[iRoom]->sector_type][x];			
		}
	}	
	if(!iNumMobsPerSector)
		return; /* could not find any mobs ofr appropiate min/max level */
			
	stpMob=read_mobile(iaMobs[number(0,iNumMobsPerSector-1)],REAL);
	char_to_room(stpMob,iRoom);
}

void HoloAct(int x,int y,int hx, int hy,int xxx,int yyy,char *szpText, bool bInvis, 
            struct char_data *stpCh, struct obj_data *stpObject, 
            struct char_data *stpVict, int iColor, char *szpSense)
{
int xx, yy, iDistance,iAngle,iMes;
char szBuf[300],cDir;
struct descriptor_data *stpS;

	for(stpS=descriptor_list;stpS;stpS=stpS->next){
     	global_color=iColor;
		if(stpS->character){
			if(world[stpS->character->in_room]->sector_type==SECT_INSIDE)
				continue;
			xx=HOLOX(stpS->character);
			yy=HOLOY(stpS->character);
			if(xx<x||xx>xxx||yy<y||yy>yyy)
				continue;
			iAngle=iHoloWhere(xx,yy,hx,hy,&iDistance);
			if(iAngle >= 360)
				cDir = 12;
			else if(iAngle >= 330)
				cDir = 11;
			else if(iAngle >= 300)
				cDir = 10;
			else if(iAngle >= 270)
				cDir = 9;
			else if(iAngle >= 240)
				cDir = 8;
			else if(iAngle >= 210)
				cDir = 7;
			else if(iAngle >= 180)
				cDir = 6;
			else if(iAngle >= 150)
				cDir = 5;
			else if(iAngle >= 120)
				cDir = 4;
			else if(iAngle >= 90)
				cDir = 3;
			else if(iAngle >= 60)
				cDir = 2;
			else if(iAngle >= 30)
				cDir = 1;
			else if(iAngle >= 0)
				cDir = 12;
			else
				cDir = -1;
			if(iDistance > 200)
				iMes = 0;
			else if(iDistance > 150)
				iMes = 1;
			else if(iDistance > 100)
				iMes = 2;
			else if(iDistance > 75)
				iMes = 3;
			else if(iDistance > 50)
				iMes = 4;
			else if(iDistance > 25)
				iMes = 5;
			else if(iDistance > 15)
				iMes = 6;
			else if(iDistance > 10)
				iMes = 7;
			else if(iDistance > 5)
				iMes = 8;
			else if(iDistance > 1)
				iMes = 9;
			else 
				iMes = 10;
			if(cDir == -1)
				sprintf(szBuf,"In the immediate area, you %s %s.\r\n",szpSense,szpText);
			else	
				sprintf(szBuf,"At %d o'clock, %s you %s %s.\r\n", cDir, szaSurveyDists[(int) iMes],szpSense, szpText);
			ACTTO=stpS->character;
			act(szBuf,bInvis,stpCh,stpObject,stpVict,TO_CHAR);
			ACTTO=NULL;
		}			     		
     	global_color=0;   
    }
}
