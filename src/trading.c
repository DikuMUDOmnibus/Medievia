/***************************************************************************
*					 MEDIEVIA CyberSpace Code and Data files		  	   *
*       Copyright (C) 1991, 1996 INTENSE Software(tm) and Mike Krause	   *
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
#include <memory.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
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

extern struct room_data *world[MAX_ROOM]; /* array of rooms                  */
extern struct char_data *character_list; /* global l-list of chars          */
extern char global_color;
extern struct descriptor_data *descriptor_list;
extern int dice(int number, int size);
extern int number(int from, int to);
extern int iMakeHoloRoom(int x,int y);
extern int get_number(char **name);
extern ush_int Holo[MAXHOLO][MAXHOLO];
struct FREIGHT *gstpFreightList;
struct FREIGHTTYPE gstaFreightTypes[100];
struct TRADABLE gstaTradables[100];
struct TRADESHOP gstaTradeShops[MAXTRADINGSHOPS];

char *szpTradeSign;
struct FREIGHT *stpCreateFreight(int iType);
bool bFreightToChar(struct FREIGHT *stpF, struct char_data *stpC);
void MakeTradeSectionInPaper(void);
void FreightToRoom(struct FREIGHT *stpF, int iRoom);
void ExtractFreight(struct FREIGHT *stpFreight);
bool bFreightFromChar(struct FREIGHT *stpF);
void FreightFromRoom(struct FREIGHT *stpF);
bool bFreightFromChar(struct FREIGHT *stpF);
bool bTradableFromFreight(struct FREIGHT *stpF, int iTradable, int iAmount);
bool bNewTradableToFreight(struct char_data *stpCh, int iTradable, int iAmount);
int isname(char *str, char *namelist);
int iTradeValue(int iTradable, int iShop);
extern double dHoloDistance(int iX1,int iY1,int iX2,int iY2);
bool LoadShop(int iShop);
void SaveShop(int iShop);
void SimulateTradeUsage(int iAmount);


bool TradeGet(struct char_data *stpCh, char *szpName)
{
struct FREIGHT *stpF;
char szBuf[MAX_INPUT_LENGTH*2];
int x;

    if(IS_NPC(stpCh))
        return(FALSE);
    for(stpF=world[stpCh->in_room]->stpFreight;stpF;stpF=stpF->stpNextInRoom){
        if(!stpF->stpCh){
            if(isname(szpName,gstaFreightTypes[stpF->iType].szpName)){
                if(!stpCh->p->stpFreight||stpCh->p->stpFreight->iLocationRoom!=stpCh->in_room){
                    if(stpCh->p->stpFreight)
                        bFreightFromChar(stpCh->p->stpFreight);
                    bFreightToChar(stpF,stpCh);
                    global_color=33;
                    sprintf(szBuf,"You grab control of %s which has been abandoned.\n\r",gstaFreightTypes[stpF->iType].szpShortDescription);
                    send_to_char(szBuf,stpCh);
                    sprintf(szBuf,"$n takes control of %s which has been abandoned.",gstaFreightTypes[stpF->iType].szpShortDescription);
                    act(szBuf,TRUE,stpCh,0,0,TO_ROOM);
                    global_color=0;
                    return(TRUE);
                }else{
                    send_to_char("You realize your cargo is right here with you.\n\r",stpCh);
                    return(TRUE);
                }
            }            
            for(x=0;x<100;x++){
                if(stpF->iaCargo[x]>0){
                    if(!str_cmp(szpName,gstaTradables[x].szpName)){
                        if(!stpCh->p->stpFreight||stpCh->p->stpFreight->iLocationRoom!=stpCh->in_room){
                            send_to_char("You realize you have nothing to carry tradables in here.\n\r",stpCh);
                            return(TRUE);
                        }
                        if(bNewTradableToFreight(stpCh,x,1)){
                            stpF->iaCargo[x]--;
                            gstaTradables[x].iNumInGame--;/* take back (NEW ONE NOT REALLY MADE HERE)*/
                            global_color=33;
                            sprintf(szBuf,"You take a %s of %s from %s and pack it in your cargo.\n\r",gstaTradables[x].szpUnitName,gstaTradables[x].szpName,gstaFreightTypes[stpF->iType].szpShortDescription);
                            send_to_char(szBuf,stpCh);
                            sprintf(szBuf,"$n takes a %s of %s from %s and packs it in $s cargo.\n\r",gstaTradables[x].szpUnitName,gstaTradables[x].szpName,gstaFreightTypes[stpF->iType].szpShortDescription);
                            act(szBuf,TRUE,stpCh,0,0,TO_ROOM);
                            global_color=0;
                            return(TRUE);                            
                        }else{
                            return(TRUE);
                        }
                    }
                }
            }
        }
    }
    return(FALSE);
}

int iTradeShop(struct char_data *stpCh, int iCmd, char *szpArg)
{
int iX,iFreight=-1,iShop,iTrade=-1,iAmount=0,iPayoff,iToShop;
char szBuf[500];
struct FREIGHT *stpFreight;

	if(IS_NPC(stpCh))
		return(FALSE);
	for(iShop=0;iShop<MAXTRADINGSHOPS;iShop++){
		if(gstaTradeShops[iShop].iX==world[stpCh->in_room]->holox&&gstaTradeShops[iShop].iY==world[stpCh->in_room]->holoy){
			break;		
		}
	}
	if(iShop==MAXTRADINGSHOPS)
		SUICIDE;
	
	switch(iCmd){
		case 76: /* LL  to show shop stats */
			global_color=33;
			send_to_char("           Listed as Tradeable # Tradable name and Quantity.\n\r______________________________________________________________________________\n\r",stpCh);
			global_color=32;
			for(iX=0;iX<100;iX+=4){
				sprintf(szBuf,"%02d-%11s %03d %02d-%11s %03d %02d-%11s %03d %02d-%11s %03d\n\r",
				iX,gstaTradables[iX].szpName,gstaTradeShops[iShop].iaStock[iX],
				iX+1,gstaTradables[iX+1].szpName,gstaTradeShops[iShop].iaStock[iX+1],
				iX+2,gstaTradables[iX+2].szpName,gstaTradeShops[iShop].iaStock[iX+2],
				iX+3,gstaTradables[iX+3].szpName,gstaTradeShops[iShop].iaStock[iX+3]);
				send_to_char(szBuf,stpCh);
			}		
			global_color=0;
			return(TRUE);
			break;
		case 59: /* LIST  */
			sprintf(szBuf,"%sList of %sTradables%s and %sHauling Beasts%s\n\r~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\r",BLU(stpCh),GRN(stpCh),BLU(stpCh),YEL(stpCh),BLU(stpCh));
			send_to_char(szBuf,stpCh);	
			for(iX=0;iX<100;iX++){
				if(gstaFreightTypes[iX].szpName){
					sprintf(szBuf,"Price %6d Size %4d Weight %4d %s%s%s\n\r",
							gstaFreightTypes[iX].iPrice,
							gstaFreightTypes[iX].iMaxSize,
							gstaFreightTypes[iX].iMaxWeight,
							YEL(stpCh),
							gstaFreightTypes[iX].szpName,
							BLU(stpCh));										
					send_to_char(szBuf,stpCh);		
				}
			}		
			for(iX=0;iX<5;iX++){
				if(gstaTradeShops[iShop].iaTrading[iX]!=-1){
					sprintf(szBuf,"Price %6d for one %s of %s%s%s\n\r",
							gstaTradables[gstaTradeShops[iShop].iaTrading[iX]].iWorth,
							gstaTradables[gstaTradeShops[iShop].iaTrading[iX]].szpUnitName,
							GRN(stpCh),
							gstaTradables[gstaTradeShops[iShop].iaTrading[iX]].szpName,
							BLU(stpCh));
					send_to_char(szBuf,stpCh);
				}
			}
			return(TRUE);
			break;
		case 56: /* BUY   */
			szpArg=one_argument(szpArg,szBuf);
			if(!szBuf[0]){
				send_to_char("Buy what?\n\r",stpCh);
				return(TRUE);
			}
			if(isdigit(szBuf[0])){
				iAmount=atoi(szBuf);
				if(iAmount<1||iAmount>10000){
					send_to_char("How many you want again??\n\r",stpCh);
					return(TRUE);
				}
				one_argument(szpArg,szBuf);			
				if(!szBuf[0]){
					send_to_char("Buy what?\n\r",stpCh);
					return(TRUE);
				}
			}else{
				iAmount=1;
			}
			for(iFreight=0;iFreight<100;iFreight++){
				if(gstaFreightTypes[iFreight].szpName)
					if(isname(szBuf,gstaFreightTypes[iFreight].szpName))
						break;
			}
			for(iTrade=0;iTrade<5;iTrade++){
				if(gstaTradeShops[iShop].iaTrading[iTrade]!=-1)
					if(isname(szBuf,gstaTradables[gstaTradeShops[iShop].iaTrading[iTrade]].szpName))
						break;
			}
			if(iFreight==100&&iTrade==5){
				global_color=32;
				sprintf(log_buf,"We have no %s...\n\r",szBuf);
				send_to_char(log_buf,stpCh);
				global_color=0;
				return(TRUE);
			}
			if(iFreight>=0&&iFreight<=99){
				if(GET_GOLD(stpCh)<gstaFreightTypes[iFreight].iPrice){
					global_color=31;
					sprintf(szBuf,"The %s costs %d gold coins..you realize your short.\n\r",gstaFreightTypes[iFreight].szpName,gstaFreightTypes[iFreight].iPrice);
					send_to_char(szBuf,stpCh);
					global_color=0;
					return(TRUE);
				}
				if(stpCh->p->stpFreight){
					if(stpCh->p->stpFreight->iLocationRoom==stpCh->in_room){
						global_color=31;
						sprintf(szBuf,"Your %s is right here..if you dont want it tether it to a post somewhere and come back so it is not in this area.\n\r",gstaFreightTypes[stpCh->p->stpFreight->iType].szpName);
						send_to_char(szBuf,stpCh);
						global_color=0;
						return(TRUE);					
					}
					stpCh->p->stpFreight->stpCh=NULL;
					stpCh->p->stpFreight->szOwnersName[0]=NULL;
					stpCh->p->stpFreight=NULL;
				}		
				GET_GOLD(stpCh)-=gstaFreightTypes[iFreight].iPrice;
				stpFreight=stpCreateFreight(iFreight);
				bFreightToChar(stpFreight,stpCh);
				FreightToRoom(stpFreight,stpCh->in_room);
				global_color=32;
				sprintf(szBuf,"You pay %d gold and take control of your %s.\n\r",gstaFreightTypes[iFreight].iPrice,gstaFreightTypes[iFreight].szpName);
				send_to_char(szBuf,stpCh);				
				sprintf(szBuf,"$n hands some gold over and buys %s.",gstaFreightTypes[iFreight].szpShortDescription);
				act(szBuf,TRUE,stpCh,0,0,TO_ROOM);
				global_color=0;
				return(TRUE);								
			}
			if(iTrade>=0&&iTrade<5){
				if(!stpCh->p->stpFreight){
					global_color=31;
					send_to_char("You realize you cant buy tradables without having a means to transport them.\n\rPerhaps you think, its time to buy a mule or horse drawn wagon...\n\r",stpCh);
					global_color=0;
					return(TRUE);
				}
				if(stpCh->p->stpFreight->iLocationRoom!=stpCh->in_room){
					global_color=31;
					sprintf(szBuf,"You realize your %s is not here.\n\r",gstaFreightTypes[stpCh->p->stpFreight->iType].szpName);
					send_to_char(szBuf,stpCh);
					global_color=0;
					return(TRUE);					
				}
				if(GET_GOLD(stpCh)<gstaTradables[gstaTradeShops[iShop].iaTrading[iTrade]].iWorth*iAmount){
					global_color=31;
					sprintf(szBuf,"The %s costs %d gold coins..you realize your short.\n\r",szBuf,gstaTradables[gstaTradeShops[iShop].iaTrading[iTrade]].iWorth*iAmount);
					send_to_char(szBuf,stpCh);
					global_color=0;
				}			
				if(!bNewTradableToFreight(stpCh,gstaTradeShops[iShop].iaTrading[iTrade],iAmount)){
					return(TRUE);				
				}
				GET_GOLD(stpCh)-=gstaTradables[gstaTradeShops[iShop].iaTrading[iTrade]].iWorth*iAmount;
				global_color=32;
				sprintf(szBuf,"You pay %d gold and a servant packs %d %s(s) of %s into your %s.\n\r",
					gstaTradables[gstaTradeShops[iShop].iaTrading[iTrade]].iWorth*iAmount,
					iAmount,
					gstaTradables[gstaTradeShops[iShop].iaTrading[iTrade]].szpUnitName,
					gstaTradables[gstaTradeShops[iShop].iaTrading[iTrade]].szpName,
					gstaFreightTypes[stpCh->p->stpFreight->iType].szpName
				);
				send_to_char(szBuf,stpCh);				
				sprintf(szBuf,"$n hands some gold over and buys %d %s(s) of %s.",iAmount,gstaTradables[gstaTradeShops[iShop].iaTrading[iTrade]].szpUnitName,gstaTradables[gstaTradeShops[iShop].iaTrading[iTrade]].szpName);
				act(szBuf,TRUE,stpCh,0,0,TO_ROOM);
				global_color=0;
				return(TRUE);
			}
			break;
		case 57: /* SELL  */
			szpArg=one_argument(szpArg,szBuf);
			if(!szBuf[0]){
				global_color=31;
				send_to_char("Sell what?\n\r",stpCh);
				global_color=0;
				return(TRUE);
			}
			if(isdigit(szBuf[0])){
				iAmount=atoi(szBuf);
				if(iAmount<1||iAmount>10000){
					send_to_char("How many you want to sell again??\n\r",stpCh);
					return(TRUE);
				}
				one_argument(szpArg,szBuf);			
				if(!szBuf[0]){
					global_color=31;
					send_to_char("Sell what?\n\r",stpCh);
					global_color=0;
					return(TRUE);
				}
			}else{
				iAmount=1;
			}
			if(!stpCh->p->stpFreight){
				global_color=31;
				send_to_char("You realize you have no hauling beasts, wagons or tradables to sell...\n\r",stpCh);
				global_color=0;
				return(TRUE);
			}
			if(stpCh->p->stpFreight->iLocationRoom==stpCh->in_room){
				if(isname(szBuf,gstaFreightTypes[stpCh->p->stpFreight->iType].szpName)){
					for(iTrade=0;iTrade<100;iTrade++){
						if(stpCh->p->stpFreight->iaCargo[iTrade]>0){
							global_color=31;
							send_to_char("You realize you would not sell it while stuff is packed in it.\n\r",stpCh);
							global_color=0;
							return(TRUE);							
						}
					}
					GET_GOLD(stpCh)+=gstaFreightTypes[stpCh->p->stpFreight->iType].iPrice/2;
					global_color=32;
					sprintf(szBuf,"You are handed %d gold and some men take %s away.\n\r",gstaFreightTypes[stpCh->p->stpFreight->iType].iPrice/2,gstaFreightTypes[stpCh->p->stpFreight->iType].szpShortDescription);
					send_to_char(szBuf,stpCh);
					sprintf(szBuf,"$n is handed some gold and some men take %s away.",gstaFreightTypes[stpCh->p->stpFreight->iType].szpShortDescription);
					act(szBuf,TRUE,stpCh,0,0,TO_ROOM);
					global_color=0;
					sprintf(szBuf,"../save/%c/%s.freight",LOWER(stpCh->player.name[0]),GET_NAME(stpCh));
					unlink(szBuf);
					ExtractFreight(stpCh->p->stpFreight);
					stpCh->p->stpFreight=NULL;
					return(TRUE);
				}
				for(iTrade=0;iTrade<100;iTrade++){
					if(stpCh->p->stpFreight->iaCargo[iTrade]>0&&isname(szBuf,gstaTradables[iTrade].szpName)){
						if(stpCh->p->stpFreight->iaCargo[iTrade]<iAmount){
							sprintf(szBuf,"You notice you only have %d %ss of %s.\n\r",stpCh->p->stpFreight->iaCargo[iTrade],gstaTradables[iTrade].szpUnitName,gstaTradables[iTrade].szpName);
							global_color=31;
							send_to_char(szBuf,stpCh);
							global_color=0;
							return(TRUE);
						}

						iPayoff=iTradeValue(iTrade,iShop)*iAmount;
						GET_GOLD(stpCh)+=iPayoff;
						bTradableFromFreight(stpCh->p->stpFreight,iTrade,iAmount);
						if((gstaTradeShops[iShop].iaStock[iTrade]+iAmount)<=100){
							iToShop=iAmount;
						}else{
							iToShop=100-gstaTradeShops[iShop].iaStock[iTrade];
						}
						gstaTradeShops[iShop].iaStock[iTrade]+=iToShop;
						SaveShop(iShop);
						SimulateTradeUsage(iToShop);
						global_color=32;
						sprintf(szBuf,"You help servants take %d %ss of %s from your %s.\n\r",
							iAmount,
							gstaTradables[iTrade].szpUnitName,
							gstaTradables[iTrade].szpName,
							gstaFreightTypes[stpCh->p->stpFreight->iType].szpName);
						send_to_char(szBuf,stpCh);
						sprintf(szBuf,"In a moment a servent returns and hands you %d gold.\n\r",iPayoff);
						send_to_char(szBuf,stpCh);
						sprintf(szBuf,"You watch as $n sells %d %ss of %s.",
							iAmount,
							gstaTradables[iTrade].szpUnitName,
							gstaTradables[iTrade].szpName);
						act(szBuf,TRUE,stpCh,0,0,TO_ROOM);
						global_color=0;
						return(TRUE);					
					}
				}				
				global_color=31;
				send_to_char("You realize you do not have that to sell.\n\r",stpCh);
				global_color=0;
				return(TRUE);
			}else{
				global_color=31;
				sprintf(szBuf,"You realize your %s is not here.\n\r",gstaFreightTypes[stpCh->p->stpFreight->iType].szpName);
				send_to_char(szBuf,stpCh);
				global_color=0;
				return(TRUE);
			}
			break;
		case 58: /* VALUE */
			one_argument(szpArg,szBuf);
			if(!szBuf[0]){
				global_color=31;
				send_to_char("Value what?\n\r",stpCh);
				global_color=0;
				return(TRUE);
			}
			if(stpCh->p->stpFreight&&stpCh->p->stpFreight->iLocationRoom==stpCh->in_room){
				if(isname(szBuf,gstaFreightTypes[stpCh->p->stpFreight->iType].szpName)){
					sprintf(szBuf,"We will give you %d gold coins for %s.",gstaFreightTypes[stpCh->p->stpFreight->iType].iPrice,gstaFreightTypes[stpCh->p->stpFreight->iType].szpShortDescription);
					global_color=32;						
					send_to_char(szBuf,stpCh);
					global_color=0;
					return(TRUE);									
				}
				for(iTrade=0;iTrade<100;iTrade++){
					if(gstaTradables[iTrade].szpName&&isname(szBuf,gstaTradables[iTrade].szpName)){
						global_color=32;
						sprintf(szBuf,"We will pay you %d gold for each %s.\n\r",iTradeValue(iTrade,iShop),gstaTradables[iTrade].szpUnitName);							
						send_to_char(szBuf,stpCh);
						global_color=0;
						return(TRUE);
					}
				}
				global_color=31;
				send_to_char("You realize they dont buy or sell that here.\n\r",stpCh);				
				global_color=0;
				return(TRUE);
			}else{
				global_color=31;
				send_to_char("You realize you have nothing here to get a price for..\n\r",stpCh);
				global_color=0;
				return(TRUE);				
			}
			break;
		case 39: /* Describe */
			one_argument(szpArg,szBuf);
			if(!szBuf[0]){
				global_color=31;
				send_to_char("Describe what?\n\r",stpCh);
				global_color=0;
				return(TRUE);
			}
			for(iFreight=0;iFreight<100;iFreight++){
				if(gstaFreightTypes[iFreight].szpName)
					if(isname(szBuf,gstaFreightTypes[iFreight].szpName))
						break;
			}
			if(iFreight>=0&&iFreight<=99){
				global_color=32;
				sprintf(szBuf,"You look at %s...\n\r",gstaFreightTypes[iFreight].szpShortDescription);
				send_to_char(szBuf,stpCh);
				send_to_char(gstaFreightTypes[iFreight].szpDescription,stpCh);
				sprintf(szBuf,"The shop keeper tells you that the %s is a good buy today.\n\rHe says size wise, it will hold roughly %d apples.\n\rAlso he says it will suport a weight of roughly %d pounds.\n\r",gstaFreightTypes[iFreight].szpShortDescription,
gstaFreightTypes[iFreight].iMaxSize,gstaFreightTypes[iFreight].iMaxWeight);
				send_to_char(szBuf,stpCh);
				global_color=0;
				return(TRUE);				
			}		
			break;
	}
	return(FALSE);
}


void ListRoomFreightToChar(struct char_data *stpCh)
{
struct FREIGHT *stpF;

	global_color=32;
	for(stpF=world[stpCh->in_room]->stpFreight;stpF;stpF=stpF->stpNextInRoom){
		send_to_char(gstaFreightTypes[stpF->iType].szpShortDescription,stpCh);
		send_to_char("\n\r",stpCh);	
	}	
	global_color=0;
}

struct FREIGHT *stpCreateFreight(int iType)
{
struct FREIGHT *stpFreight=NULL,*stpF;

	CREATE(stpFreight,struct FREIGHT,1);
	memset((char *)stpFreight,(char)'\0',(int)sizeof(struct FREIGHT));
	stpFreight->iType=iType;	
	if(!gstpFreightList){
		gstpFreightList=stpFreight;
	}else{
		stpF=gstpFreightList;
		while(stpF->stpNext)
			stpF=stpF->stpNext;
		stpF->stpNext=stpFreight;	
	}
	return(stpFreight);
}

void ExtractFreight(struct FREIGHT *stpFreight)
{
struct FREIGHT *stpF;
int iX;

	if(stpFreight->iLocationRoom)
		FreightFromRoom(stpFreight);
	if(stpFreight->stpCh)
		bFreightFromChar(stpFreight);
	for(iX=0;iX<100;iX++){
		if(stpFreight->iaCargo[iX]){
			bTradableFromFreight(stpFreight,iX,stpFreight->iaCargo[iX]);
		}
	}
	if(stpFreight==gstpFreightList){
		gstpFreightList=stpFreight->stpNext;
	}else{
		for(stpF=gstpFreightList;stpF&&stpF->stpNext!=stpFreight;stpF=stpF->stpNext)
			;
		if(!stpF)
			SUICIDE;
		stpF->stpNext=stpFreight->stpNext;				
	}	
	stpFreight=my_free(stpFreight);
}

/* returns NULL if successfull or a string stating why not */
bool bNewTradableToFreight(struct char_data *stpCh, int iTradable, int iAmount)
{
int iWillFitSize=0,iWillFitWeight=0;
char szBuf[1000];

	szBuf[0]=NULL;
	if((gstaTradables[iTradable].iSize*iAmount+stpCh->p->stpFreight->iCurrentSize)>gstaFreightTypes[stpCh->p->stpFreight->iType].iMaxSize){
		iWillFitSize=(gstaFreightTypes[stpCh->p->stpFreight->iType].iMaxSize-stpCh->p->stpFreight->iCurrentSize)/gstaTradables[iTradable].iSize;		
	}	
	if((gstaTradables[iTradable].iWeight*iAmount+stpCh->p->stpFreight->iCurrentWeight)>gstaFreightTypes[stpCh->p->stpFreight->iType].iMaxWeight){
		iWillFitWeight=(gstaFreightTypes[stpCh->p->stpFreight->iType].iMaxWeight-stpCh->p->stpFreight->iCurrentWeight)/gstaTradables[iTradable].iWeight;		
	}	
	if(iWillFitSize||iWillFitWeight){
		if(iWillFitSize>=iWillFitWeight){
			sprintf(szBuf,"You realize the %s is not big enough to hold all of that.\n\rAt most %d %s of %s will fit right now.\n\r",gstaFreightTypes[stpCh->p->stpFreight->iType].szpName,iWillFitSize,gstaTradables[iTradable].szpUnitName,
gstaTradables[iTradable].szpName);
		}else{
			sprintf(szBuf,"You realize the %s will not hold all of that weight.\n\rAt most %d %s of %s will fit right now.\n\r",gstaFreightTypes[stpCh->p->stpFreight->iType].szpName,iWillFitWeight,gstaTradables[iTradable].szpUnitName,
gstaTradables[iTradable].szpName);
		}
		global_color=31;
		send_to_char(szBuf,stpCh);
		global_color=0;
		return(FALSE);
	}
	gstaTradables[iTradable].iNumInGame+=iAmount;
	stpCh->p->stpFreight->iaCargo[iTradable]+=iAmount;
	stpCh->p->stpFreight->iCurrentWeight+=gstaTradables[iTradable].iWeight*iAmount;
	stpCh->p->stpFreight->iCurrentSize+=gstaTradables[iTradable].iSize*iAmount;
	return(TRUE);
}

bool bTradableFromFreight(struct FREIGHT *stpF, int iTradable, int iAmount)
{

	if(stpF->iaCargo[iTradable]<iAmount)
		return(FALSE);
		
	stpF->iaCargo[iTradable]-=iAmount;
	stpF->iCurrentWeight-=gstaTradables[iTradable].iWeight*iAmount;
	stpF->iCurrentSize-=gstaTradables[iTradable].iSize*iAmount;
	gstaTradables[iTradable].iNumInGame-=iAmount;
	return(TRUE);

}

bool bFreightToChar(struct FREIGHT *stpF, struct char_data *stpC)
{
	if(IS_NPC(stpC))
		return(FALSE);
	if(stpC->p->stpFreight)
		return(FALSE);
	stpF->stpCh=stpC;
	strcpy(stpF->szOwnersName,GET_NAME(stpC));
	stpC->p->stpFreight=stpF;
	return(TRUE);
}

bool bFreightFromChar(struct FREIGHT *stpF)
{
	if(!stpF->stpCh)
		return(FALSE);
	stpF->szOwnersName[0]=NULL;
	stpF->stpCh->p->stpFreight=NULL;
	stpF->stpCh->p->stpFreight=NULL;
	stpF->stpCh=NULL;
	return(TRUE);
}

void FreightToRoom(struct FREIGHT *stpF, int iRoom)
{
	if(stpF->iLocationRoom)
		SUICIDE;
	stpF->stpNextInRoom=world[iRoom]->stpFreight;
	world[iRoom]->stpFreight=stpF;
	stpF->iLocationRoom=iRoom;
	if(world[iRoom]->zone==197){
		stpF->iLocationX=world[iRoom]->holox;
		stpF->iLocationY=world[iRoom]->holoy;
	}
}

void FreightFromRoom(struct FREIGHT *stpF)
{
struct FREIGHT *p;

	if(!stpF->iLocationRoom)
		SUICIDE;
	if(stpF==world[stpF->iLocationRoom]->stpFreight){
		world[stpF->iLocationRoom]->stpFreight=stpF->stpNextInRoom;
	}else{
		for(p=world[stpF->iLocationRoom]->stpFreight;p&&p->stpNextInRoom!=stpF;p=p->stpNextInRoom);
		if(!p)
			SUICIDE;	
		p->stpNextInRoom=p->stpNextInRoom->stpNextInRoom;
	}
	stpF->iLocationRoom=0;
	stpF->iLocationX=0;
	stpF->iLocationY=0;
}

/* This function ASSUMES we made sure toroom is NEXT to freights location and
   that following should be done as ch moved, it is called from char_to_room 
   It also assumes ch HAS freight.  It assumes these things for speed so
   this function is only called when shit should happen and all checking
   is done in the callers function for speeds sake.   
*/
void FreightAutoFollow(struct char_data *stpCh, int iToRoom)
{
char szBuf[250];

	if(world[iToRoom]->extra_flags&gstaFreightTypes[stpCh->p->stpFreight->iType].iTypeBits){
		global_color=31;	
		sprintf(szBuf,"%s *REFUSES* to follow you there.\n\r",gstaFreightTypes[stpCh->p->stpFreight->iType].szpShortDescription);
		send_to_char(szBuf,stpCh);
		sprintf(szBuf,"%s *REFUSES* to follow $n",gstaFreightTypes[stpCh->p->stpFreight->iType].szpShortDescription);
		act(szBuf,TRUE,stpCh,0,0,TO_ROOM);
		global_color=0;
		return;
	}
	FreightFromRoom(stpCh->p->stpFreight);
	FreightToRoom(stpCh->p->stpFreight,iToRoom);
	global_color=37;
	sprintf(szBuf,"%s follows you.\n\r",gstaFreightTypes[stpCh->p->stpFreight->iType].szpShortDescription);
	send_to_char(szBuf,stpCh);
	sprintf(szBuf,"%s follows $n.",gstaFreightTypes[stpCh->p->stpFreight->iType].szpShortDescription);
	act(szBuf,TRUE,stpCh,0,0,TO_ROOM);	
	global_color=0;
}

void LoadFreightTypes(void)
{
FILE *fp;
int iNum,iMob;
char szTag[8192];

    if(!(fp=fopen("../lib/medievia.freight", "r"))){
        perror("medievia.freight");
        SUICIDE;
    }
	fscanf(fp," %s ",szTag);
	while(1){
		if(szTag[0]=='$')
			break;
		iNum=atoi(&szTag[1]);
		if(iNum<0||iNum>99)
			SUICIDE;
		while(1){
			fscanf(fp, " %s ",szTag);
			if(szTag[0]=='#'||szTag[0]=='$')
				break;
			if(!strncasecmp(szTag,"NAM",3)){
				gstaFreightTypes[iNum].szpName=fread_string(fp);
			}else if(!strncasecmp(szTag,"SHO",3)){
				gstaFreightTypes[iNum].szpShortDescription=fread_string(fp);
			}else if(!strncasecmp(szTag,"DES",3)){
				gstaFreightTypes[iNum].szpDescription=fread_string(fp);
			}else if(!strncasecmp(szTag,"SIZ",3)){
				fscanf(fp," %d ",&gstaFreightTypes[iNum].iMaxSize);
			}else if(!strncasecmp(szTag,"WGT",3)){
				fscanf(fp," %d ",&gstaFreightTypes[iNum].iMaxWeight);
			}else if(!strncasecmp(szTag,"PRI",3)){
				fscanf(fp," %d ",&gstaFreightTypes[iNum].iPrice);
			}else if(!strncasecmp(szTag,"WAG",3)){
				SET_BIT(gstaFreightTypes[iNum].iTypeBits,WAGONTYPE);
			}else if(!strncasecmp(szTag,"MUL",3)){
				SET_BIT(gstaFreightTypes[iNum].iTypeBits,MULETYPE);
			}else if(!strncasecmp(szTag,"HOR",3)){
				SET_BIT(gstaFreightTypes[iNum].iTypeBits,HORSETYPE);
			}else if(!strncasecmp(szTag,"MOB",3)){
				fscanf(fp," %d ",&iMob);
				gstaFreightTypes[iNum].stpMob=read_mobile(iMob,REAL);
				gstaFreightTypes[iNum].stpMob->in_room=-1;
			}
		}
	}       
	fclose(fp);
}

void MakeTradeShop(int iRoom)
{
int x;

	world[iRoom]->funct=iTradeShop; 
    CREATE(world[iRoom]->ex_description, struct extra_descr_data,1);
    world[iRoom]->ex_description->keyword=str_dup("sign");
    world[iRoom]->ex_description->description=str_dup(szpTradeSign);
    world[iRoom]->ex_description->next=NULL;
	for(x=0;x<MAXTRADINGSHOPS;x++){
		if(gstaTradeShops[x].iX==world[iRoom]->holox&&gstaTradeShops[x].iY==world[iRoom]->holoy){
			world[iRoom]->name=gstaTradeShops[x].szpName;
			world[iRoom]->description=gstaTradeShops[x].szpDescription;
			break;
		}		
	}	
	if(x==MAXTRADINGSHOPS)
		SUICIDE;
}


void SeedLocalEconomy(int iShop)
{
int iTrade,x,iStock;
float fDistFarthestShopFromOrigen,fLocalShopDist,f;

	sprintf(log_buf,"#### Seeding economy at X%d Y%d",gstaTradeShops[iShop].iX,gstaTradeShops[iShop].iY);
	log_hd(log_buf);
	for(iTrade=0;iTrade<100;iTrade++){
		if(!strcmp(gstaTradables[iTrade].szpName,"not defined")){
			gstaTradeShops[iShop].iaStock[iTrade]=50;
			continue;	
		}
		for(x=0;x<5;x++){
			if(gstaTradeShops[iShop].iaTrading[x]==iTrade)
				break;
		}
		if(x<5){/* produced here */
			gstaTradeShops[iShop].iaStock[iTrade]=100;
		}else{
			fDistFarthestShopFromOrigen=0;
			for(x=0;x<MAXTRADINGSHOPS;x++){
				if(!gstaTradeShops[x].iX)
					continue;
				f=(float)dHoloDistance(
						gstaTradeShops[x].iX,gstaTradeShops[x].iY,
						gstaTradables[iTrade].iOrigenX,gstaTradables[iTrade].iOrigenY);
				if(f>fDistFarthestShopFromOrigen)
					fDistFarthestShopFromOrigen=f;
			}
			fLocalShopDist=dHoloDistance(
				gstaTradeShops[iShop].iX,gstaTradeShops[iShop].iY,
			    gstaTradables[iTrade].iOrigenX,gstaTradables[iTrade].iOrigenY);
			iStock=(int)(100-(fLocalShopDist/(fDistFarthestShopFromOrigen/100)));			
			if(iStock>100)
				iStock=100;
			if(iStock<0)
				iStock=0;
			gstaTradeShops[iShop].iaStock[iTrade]=iStock;
		}		
	}
	SaveShop(iShop);
}

void LoadTradeShops(void)
{
FILE *fp;
int iNum,x=0,y;
char szTag[8192];

	for(x=0;x<MAXTRADINGSHOPS;x++){
		gstaTradeShops[x].iX=0;
		for(iNum=0;iNum<5;iNum++){
			gstaTradeShops[x].iaTrading[iNum]=-1;
		}
	}
	x=0;
    if(!(fp=fopen("../lib/medievia.tradeshops", "r"))){
        perror("medievia.tradeshops");
        SUICIDE;
    }
	fscanf(fp," %s ",szTag);
	while(1){
		if(szTag[0]=='$')
			break;
		iNum=atoi(&szTag[1]);
		if(iNum<0||iNum>MAXTRADINGSHOPS)
			SUICIDE;
		x=0;
		while(1){
			fscanf(fp, " %s ",szTag);
			if(szTag[0]=='#'||szTag[0]=='$')
				break;
			if(!strncasecmp(szTag,"NAM",3)){
				gstaTradeShops[iNum].szpName=fread_string(fp);
			}else if(!strncasecmp(szTag,"DES",3)){
				gstaTradeShops[iNum].szpDescription=fread_string(fp);
			}else if(!strncasecmp(szTag,"X",1)){
				fscanf(fp," %d ",&gstaTradeShops[iNum].iX);
			}else if(!strncasecmp(szTag,"Y",1)){
				fscanf(fp," %d ",&gstaTradeShops[iNum].iY);
			}else if(!strncasecmp(szTag,"TRA",3)){
				if(x>4)
					SUICIDE;
				fscanf(fp," %d ",&gstaTradeShops[iNum].iaTrading[x++]);
			}
		}
	}       
	fclose(fp);
    if(!(fp=fopen("../lib/tradeshopsign.txt", "r"))){
        perror("medievia.tradeshops");
        SUICIDE;
    }
	szpTradeSign=fread_string(fp);
	fclose(fp);
	for(x=0;x<MAXTRADINGSHOPS;x++){
		if(gstaTradeShops[x].iX==0)
			continue;
		Holo[gstaTradeShops[x].iX][gstaTradeShops[x].iY]=216;/* set to a shop*/
	}
	for(x=0;x<MAXTRADINGSHOPS;x++){
		if(gstaTradeShops[x].iX==0)
			continue;
		if(!LoadShop(x)){
			SeedLocalEconomy(x);
		}
		for(y=0;y<5;y++){
			if(gstaTradeShops[x].iaTrading[y]!=-1){
				gstaTradables[gstaTradeShops[x].iaTrading[y]].iOrigenX=gstaTradeShops[x].iX;
				gstaTradables[gstaTradeShops[x].iaTrading[y]].iOrigenY=gstaTradeShops[x].iY;
			}
		}		
	}
}

void LoadTradables(void)
{
FILE *fp;
int iNum;
char szTag[8192];

    if(!(fp=fopen("../lib/medievia.tradables", "r"))){
        perror("medievia.tradables");
        SUICIDE;
    }
	fscanf(fp," %s ",szTag);
	while(1){
		if(szTag[0]=='$')
			break;
		iNum=atoi(&szTag[1]);
		if(iNum<0||iNum>99)
			SUICIDE;
		while(1){
			fscanf(fp, " %s ",szTag);
			if(szTag[0]=='#'||szTag[0]=='$')
				break;
			if(!strncasecmp(szTag,"NAM",3)){
				gstaTradables[iNum].szpName=fread_string(fp);
			}else if(!strncasecmp(szTag,"DES",3)){
				gstaTradables[iNum].szpDescription=fread_string(fp);
			}else if(!strncasecmp(szTag,"UNI",3)){
				gstaTradables[iNum].szpUnitName=fread_string(fp);
			}else if(!strncasecmp(szTag,"SIZ",3)){
				fscanf(fp," %d ",&gstaTradables[iNum].iSize);
			}else if(!strncasecmp(szTag,"WGT",3)){
				fscanf(fp," %d ",&gstaTradables[iNum].iWeight);
			}else if(!strncasecmp(szTag,"ORX",3)){
				fscanf(fp," %d ",&gstaTradables[iNum].iOrigenX);
			}else if(!strncasecmp(szTag,"ORY",3)){
				fscanf(fp," %d ",&gstaTradables[iNum].iOrigenY);
			}else if(!strncasecmp(szTag,"VAL",3)){
				fscanf(fp," %d ",&gstaTradables[iNum].iWorth);
			}
		}
	}       
	fclose(fp);
	for(iNum=0;iNum<100;iNum++){
		if(!gstaTradables[iNum].szpName)
			gstaTradables[iNum].szpName=str_dup("not defined");
	}
}

void BootTradingSystem(void)
{
	gstpFreightList=NULL;
	fprintf(stderr,"TRADING: Setting up Trading system..\n");
	fprintf(stderr,"TRADING: Loading and Defining Moveable Freight..\n");
	LoadFreightTypes();
	fprintf(stderr,"TRADING: Loading and Defining Tradables....\n");
	LoadTradables();
	fprintf(stderr,"TRADING: Loading and Setting Trade Shops....\n");
	LoadTradeShops();
	fprintf(stderr,"TRADING: Updating Trading section of paper....\n");
	MakeTradeSectionInPaper();
}

int iTradeValue(int iTradable, int iShop)
{
double dDistance;
register int iOutcome;
int iStock,x;
	
	for(x=0;x<5;x++){/*make sure we dont produce this here*/
		if(gstaTradeShops[iShop].iaTrading[x]==iTradable){
			iOutcome=gstaTradables[iTradable].iWorth;
			iOutcome/=2;
			return(iOutcome);
		}
	}	
	dDistance=dHoloDistance(
					gstaTradeShops[iShop].iX,
					gstaTradeShops[iShop].iY,
					gstaTradables[iTradable].iOrigenX,
					gstaTradables[iTradable].iOrigenY
			 	);
	iOutcome=gstaTradables[iTradable].iWorth;
	iOutcome+=(int)(gstaTradables[iTradable].iWorth*dDistance/20);
	iStock=gstaTradeShops[iShop].iaStock[iTradable];
	iStock=100-iStock; /* Reverse it */
	iOutcome+=iOutcome*=(iStock/12);
	return(iOutcome);
}

void SaveShop(int iShop)
{
FILE *fh;
char szFilename[200];

	sprintf(szFilename,"../economy/X%3dY%3d.TradeShop",gstaTradeShops[iShop].iX,gstaTradeShops[iShop].iY);
    if(!(fh=med_open(szFilename, "w"))){
        perror("medievia.tradeshops");
		SUICIDE;
    }
	fwrite(&gstaTradeShops[iShop].iaStock[0],sizeof(int),100,fh);
	med_close(fh);
}

bool LoadShop(int iShop)
{
FILE *fh;
char szFilename[200];

	sprintf(szFilename,"../economy/X%3dY%3d.TradeShop",gstaTradeShops[iShop].iX,gstaTradeShops[iShop].iY);
    if(!(fh=fopen(szFilename, "r"))){
		return(FALSE);
    }
	fread(&gstaTradeShops[iShop].iaStock[0],sizeof(int),100,fh);
	fclose(fh);
	return(TRUE);
}

void SimulateTradeUsage(int iAmount)
{
int iShop,iTrade,iLoops=0,iTries;

	if(iAmount<1)
		return;
	while(iLoops++<1000){
		iShop=number(0,MAXTRADINGSHOPS);
		if(gstaTradeShops[iShop].iX){
			iTries=0;
			while(iTries<500){
				iTrade=number(0,99);
				if(gstaTradeShops[iShop].iaStock[iTrade]>0){
					gstaTradeShops[iShop].iaStock[iTrade]--;
					iAmount--;
					SaveShop(iShop);
					break;
				}
			}
			if(iAmount==0)
				return;
		}			
	}
	SUICIDE;		
}

void ListTradablesInFreight(struct char_data *stpCh, struct FREIGHT *stpF)
{
register int iTrade;
char szBuf[250];
int iCount=0;

	for(iTrade=0;iTrade<100;iTrade++){
		if(stpF->iaCargo[iTrade]>0){
			iCount++;
			sprintf(szBuf,"       %s%d %s%s's of %s%s.\n\r",
				YEL(stpCh),
				stpF->iaCargo[iTrade],
				BLU(stpCh),
				gstaTradables[iTrade].szpUnitName,
				RED(stpCh),
				gstaTradables[iTrade].szpName);					
			send_to_char(szBuf,stpCh);
		}
	}
	if(!iCount){
		send_to_char("You find nothing packed there.\n\r",stpCh);
	}
}

bool ExamineFreight(struct char_data *stpCh, char *szpName)
{
struct FREIGHT *stpF;
char szBuf[250],*szpTmp,szTmpName[MAX_INPUT_LENGTH*2];
int iNumAt,iNumWanted;

	strcpy(szTmpName,szpName);
	szpTmp=szTmpName;
	if(!(iNumWanted=get_number(&szpTmp)))
		return(FALSE);
	
	for(stpF=world[stpCh->in_room]->stpFreight,iNumAt=1;stpF&&(iNumAt<=iNumWanted);stpF=stpF->stpNextInRoom){
		if(isname(szpTmp,gstaFreightTypes[stpF->iType].szpName)){
			if(iNumAt!=iNumWanted){
				iNumAt++;
				continue;
			}
			global_color=32;
			sprintf(szBuf,"You examine what is packed in the %s..\n\r",gstaFreightTypes[stpF->iType].szpName);
			send_to_char(szBuf,stpCh);
			ListTradablesInFreight(stpCh,stpF);
			global_color=0;
			if(stpF->szOwnersName[0])
				sprintf(szBuf,"%sEtched on a saddle bag is the name %s%s%s.%s\n\r",BLU(stpCh),CYN(stpCh),stpF->szOwnersName,BLU(stpCh),NRM(stpCh));
			else
				sprintf(szBuf,"%sEtched on a saddle bag is the name %sto worn away to be read%s.%s\n\r",BLU(stpCh),CYN(stpCh),BLU(stpCh),NRM(stpCh));
			send_to_char(szBuf,stpCh);
			return(TRUE);
		}
	}
	return(FALSE);
}

bool LookAtFreight(struct char_data *stpCh, char *szpArgument)
{
struct FREIGHT *stpF;
char szArg[MAX_INPUT_LENGTH];

	one_argument(szpArgument,szArg);
	for(stpF=world[stpCh->in_room]->stpFreight;stpF;stpF=stpF->stpNextInRoom){
		if(isname(szArg,gstaFreightTypes[stpF->iType].szpName)){
			global_color=32;
			send_to_char(gstaFreightTypes[stpF->iType].szpDescription,stpCh);
			send_to_char("You think about Examining what is packed in it...\n\r",stpCh);
			global_color=0;		
			return(TRUE);
		}					
	}
	return(FALSE);
}

/* these 2 funcs are just to quickly put the fake freight mobs there so socials work */
void FreightMobsToRoom(int iRoom)
{
struct FREIGHT *f;

	for(f=world[iRoom]->stpFreight;f;f=f->stpNextInRoom){
		if(f->iLocationRoom==iRoom){
			if(gstaFreightTypes[f->iType].stpMob->in_room!=iRoom){
	           gstaFreightTypes[f->iType].stpMob->next_in_room=world[iRoom]->people;
	           world[iRoom]->people=gstaFreightTypes[f->iType].stpMob;
    	       gstaFreightTypes[f->iType].stpMob->in_room=iRoom;
			}
		}                       			
	}
}

void FreightMobsFromRoom(int iRoom)
{
register int x;
struct char_data *stpMob;

	for(x=0;x<100;x++){
		if(gstaFreightTypes[x].stpMob){
			if(gstaFreightTypes[x].stpMob->in_room==iRoom){
				if(world[iRoom]->people==gstaFreightTypes[x].stpMob){
					world[iRoom]->people=gstaFreightTypes[x].stpMob->next_in_room;
					gstaFreightTypes[x].stpMob->in_room=-1;
				}else{
					for(stpMob=world[iRoom]->people;stpMob->next_in_room!=gstaFreightTypes[x].stpMob;stpMob=stpMob->next_in_room);
					stpMob->next_in_room=stpMob->next_in_room->next_in_room;
					gstaFreightTypes[x].stpMob->in_room=-1;
				}
			}
		}
	}
}

void SaveFreight(struct char_data *stpCh)
{
FILE *fh;
char szFilename[200];
int iVersionNumber=3;

	sprintf(szFilename,"../save/%c/%s.freight",LOWER(stpCh->player.name[0]),GET_NAME(stpCh));
    if(!(fh=med_open(szFilename, "wb"))){
        perror("saving someones freight, see core:");
		SUICIDE;
    }
	fwrite(&iVersionNumber,sizeof(int),1,fh);
	fwrite(&stpCh->p->stpFreight->iType,sizeof(int),1,fh);
	fwrite(&stpCh->p->stpFreight->liStatus,sizeof(long int),1,fh);
	fwrite(&stpCh->p->stpFreight->iLocationX,sizeof(int),1,fh);
	fwrite(&stpCh->p->stpFreight->iLocationY,sizeof(int),1,fh);
	fwrite(&stpCh->p->stpFreight->iLocationRoom,sizeof(int),1,fh);
	fwrite(&stpCh->p->stpFreight->iCurrentWeight,sizeof(int),1,fh);
	fwrite(&stpCh->p->stpFreight->iCurrentSize,sizeof(int),1,fh);
	fwrite(&stpCh->p->stpFreight->iaCargo,sizeof(int)*100,1,fh);
	med_close(fh);
}

/* This all purpose function sees if person has freight, if so it creates it,
loads it, if needed makes holoroom, and then puts the freight in the room */
bool bLoadFreight(struct char_data *stpCh)
{
FILE *fh;
char szFilename[200];
int iType,iVersionNumber,iRoom;

	
	sprintf(szFilename,"../save/%c/%s.freight",LOWER(stpCh->player.name[0]),GET_NAME(stpCh));
    if(!(fh=med_open(szFilename, "rb"))){
		return(FALSE);
    }
	fread(&iVersionNumber,sizeof(int),1,fh);
	fread(&iType,sizeof(int),1,fh);
	stpCh->p->stpFreight=stpCreateFreight(iType);
	stpCh->p->stpFreight->iType=iType;
	fread(&stpCh->p->stpFreight->liStatus,sizeof(long int),1,fh);
	fread(&stpCh->p->stpFreight->iLocationX,sizeof(int),1,fh);
	fread(&stpCh->p->stpFreight->iLocationY,sizeof(int),1,fh);
	fread(&iRoom,sizeof(int),1,fh);
	fread(&stpCh->p->stpFreight->iCurrentWeight,sizeof(int),1,fh);
	fread(&stpCh->p->stpFreight->iCurrentSize,sizeof(int),1,fh);
	fread(&stpCh->p->stpFreight->iaCargo,sizeof(int)*100,1,fh);
	strcpy(stpCh->p->stpFreight->szOwnersName,GET_NAME(stpCh));
	med_close(fh);
	if(stpCh->p->stpFreight->iLocationX||stpCh->p->stpFreight->iLocationY){
		if(Holo[stpCh->p->stpFreight->iLocationX][stpCh->p->stpFreight->iLocationY]<=255)
			iMakeHoloRoom(stpCh->p->stpFreight->iLocationX,stpCh->p->stpFreight->iLocationY);
		stpCh->p->stpFreight->iLocationRoom=0;
		iRoom=Holo[stpCh->p->stpFreight->iLocationX][stpCh->p->stpFreight->iLocationY];
	}
	FreightToRoom(stpCh->p->stpFreight,iRoom);
	return(TRUE);
}

void SaveAndExtractFreight(struct char_data *stpCh)
{

	if(IS_NPC(stpCh))
		return;
	if(!stpCh->p->stpFreight)
		return;
	SaveFreight(stpCh);
	ExtractFreight(stpCh->p->stpFreight);
	stpCh->p->stpFreight=NULL;	
}

void MakeTradeSectionInPaper(void)
{
#define SHOP 0
#define ITEM 1
#define BUYPRICE 2
#define SELLPRICE 3
#define STOCK 4
extern struct time_info_data time_info; 
extern const char *month_name[];
int iA[100][5],iShop,iItem,x,y,iMarker=0,iNumStock=0,day;
int iaShop[MAXTRADINGSHOPS];
FILE *fh;
char *suf;

	day = time_info.day + 1;   /* day in [1..35] */

    if (day == 1)
        suf = "st";
    else if (day == 2)
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
 

	for(x=0;x<100;x++)
		for(y=0;y<4;y++)
			iA[x][y]=-1;
	for(x=0;x<MAXTRADINGSHOPS;x++)
		iaShop[x]=0;	
#ifndef MEDTHIEVIA
	while(iMarker<100){	
#else
	while(iMarker<38){
#endif
		for(iShop=0;iShop<MAXTRADINGSHOPS&&iMarker<100;iShop++){
			if(!gstaTradeShops[iShop].szpName)continue;
			for(iItem=0;iItem<100&&iMarker<100;iItem++){
				if(!strcmp(gstaTradables[iItem].szpName,"not defined"))
					continue;
				if(gstaTradeShops[iShop].iaStock[iItem]==iNumStock){
					if(iaShop[iShop]>12)
						continue;
					iA[iMarker][SHOP]=iShop;
					iA[iMarker][ITEM]=iItem;
					iA[iMarker][BUYPRICE]=gstaTradables[iItem].iWorth;
					iA[iMarker][SELLPRICE]=iTradeValue(iItem,iShop);
					iA[iMarker][STOCK]=iNumStock;
					iaShop[iShop]++;
					iMarker++;
				}
			}
		}
		iNumStock++;
	}
    if(!(fh=fopen("../newspaper/current_trading", "w"))){
        perror("medievia.freight");
        SUICIDE;
    }
	fprintf(fh,"                     MEDIEVIA TRADING SECTION\n      Representing all local markets and economies since 383\n------------------------------------------------------------------------\n");
	fprintf(fh,"       TRADESHOP                    ITEM      INSTOCK    BUY     SELL\n");
#ifndef MEDTHIEVIA
	for(x=0;x<100;x++){
#else
	for(x=0;x<38;x++){
#endif
		fprintf(fh,"%35s %12s %3d %8d %8d\n",
			gstaTradeShops[iA[x][SHOP]].szpName,
			gstaTradables[iA[x][ITEM]].szpName,
			iA[x][STOCK],iA[x][BUYPRICE],iA[x][SELLPRICE]);
	}
	fprintf(fh,"At most 12 top needs from market listed.\n");
	fprintf(fh,"INSTOCK is how many this shop has of ITEM in stock.\n");
	fprintf(fh,"BUY is how much it costs to but the ITEM.\n");
	fprintf(fh,"SELL is how much you can sell the ITEM for at this TRADESHOP.\n");
	fprintf(fh,"We do not post where the items are produced.\n");
	fprintf(fh,"Prices as of the %d%s Day of the %s, Year %d.\n~\n", 
        day,
        suf,
        month_name[time_info.month],
        time_info.year);
	fclose(fh);
}
