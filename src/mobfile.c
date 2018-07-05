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

struct NEWMOBFLAGS{
	long int liDenyBits;
};

struct NEWMOBFLAGS *stpaNewFlags[MAX_MOB];
struct char_data *mobs[MAX_MOB];
extern char use_hash_table;
extern FILE *mob_f;                    
extern struct index_data mob_index_array[MAX_MOB];
extern struct index_data *mob_index;
extern int top_of_mobt;                  /* top of mobile index table       */
extern void write_filtered_text(FILE *fh, char *text);
extern struct char_data *character_list;
extern struct zone_data *zone_table;
extern FILE *hold2;
#define mob_index   mob_index_array

long int liGetNumber(FILE *new, char *szpTag);

int num_mob_classless;
int num_mob_class;
int num_mob_thief;
int num_mob_magic_user;
int num_mob_warrior;
int num_mob_cleric;
int num_mob_other;

#define BACKSTAB 		1
#define KICK			2
#define BASH			4
#define TRIP			8
#define CHARGE			16
#define THROW			32



struct char_data *load_mobile(int nr, int type, FILE *new)
{
int i,x;
long tmp=0, tmp2=0, tmp3=0;
struct char_data *mob=NULL;
char letter;
char szTag[81];

    if(new){
        if (!(mob_f = fopen(MOB_FILE, "r"))){
	    	perror( MOB_FILE );
			SUICIDE;
    	}
		open_files++;
    }
    i = nr;
    fseek(mob_f, mob_index[nr].pos, 0);

	CREATE(stpaNewFlags[nr],struct NEWMOBFLAGS,1);
    CREATE(mob, struct char_data, 1);
    clear_char(mob);
    if(new)
	fprintf(new,"#%d\n",nr);
    /***** String data *** */
    sprintf(log_buf,"Loading mobile #%d",nr);
    mob->player.name = fread_string(mob_f);
    if(new)
		write_filtered_text(new,mob->player.name);
    mob->player.short_descr = fread_string(mob_f);
    if(new)
        write_filtered_text(new,mob->player.short_descr);
    mob->player.long_descr = fread_string(mob_f);
    if(new)
        write_filtered_text(new,mob->player.long_descr);
    mob->player.description = fread_string(mob_f);
    if(new)                
        write_filtered_text(new,mob->player.description);
    mob->player.title = MED_NULL;
    mob->specials.last_attack[0]=MED_NULL;
    mob->abilities.str   = 13;
    mob->abilities.intel = 13;
    mob->abilities.wis   = 13;
    mob->abilities.dex   = 13;
    mob->abilities.con   = 13;
    mob->tmpabilities = mob->abilities;
	mob->player.time.birth = time(0);
	mob->player.time.played = 0;
	mob->player.time.logon  = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;
	for (i = 0; i < 3; i++)
	    GET_COND(mob, i) = -1;
	for (i = 0; i < 5; i++)
	    mob->specials.apply_saving_throw[i] = 0/*MAX(20-GET_LEVEL(mob), 2)*/;
    for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
		mob->equipment[i] = NULL;
    mob->nr = nr;
    mob->desc = NULL;

/****** READ IN VARIABLES TAG/VALUE PAIRS *******/
	do{
		fscanf(mob_f," %s ",szTag);
		if(!strncasecmp(szTag,"ACT",3)){
		    mob->specials.act=liGetNumber(new,"ACT");
            SET_BIT(mob->specials.act, ACT_ISNPC);
		}else if(!strncasecmp(szTag,"AFF",3)){
		    mob->specials.affected_by=liGetNumber(new,"AFF");
		}else if(!strncasecmp(szTag,"ALI",3)){
		    mob->specials.alignment=liGetNumber(new,"ALI");
		}else if(!strncasecmp(szTag,"CLA",3)){
			fscanf(mob_f," %c ",&letter);
			mob->player.class = 0;
	        switch(letter){
			    case 'M':
					mob->player.class=CLASS_MAGIC_USER;
					num_mob_magic_user++;
					break;
			    case 'C':
					mob->player.class=CLASS_CLERIC;
						num_mob_cleric++;
					break;
			    case 'W':
					mob->player.class=CLASS_WARRIOR;
					num_mob_warrior++;
					break;
			    case 'T':
					mob->player.class=CLASS_THIEF;
					num_mob_thief++;
					break;
			    case 'O':
					num_mob_other++;
					break;
			    case 'X':
						mob->player.class=0;
					break;
			    default:
					sprintf(log_buf,"##mob %d has a bad class",nr);
					log_hd(log_buf);
			}
			if(new)
	    		fprintf(new," CLA %c\n",letter);
	        if(letter=='X')num_mob_classless++;
       		else num_mob_class++;
		}else if(!strncasecmp(szTag,"LEV",3)){
			GET_LEVEL(mob)=liGetNumber(new,"LEV");
		}else if(!strncasecmp(szTag,"HRO",3)){
			mob->points.hitroll=liGetNumber(new,"HRO");
		}else if(!strncasecmp(szTag,"ARM",3)){
			mob->points.armor=10*liGetNumber(new,"ARM");
		}else if(!strncasecmp(szTag,"HIT",3)){
			mob->points.max_hit=liGetNumber(new,"HIT");
			mob->points.hit=mob->points.max_hit;
		}else if(!strncasecmp(szTag,"DAM",3)){
			fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
			if(new)
			    fprintf(new,"DAM %ldd%ld+%ld\n",tmp, tmp2, tmp3);
			mob->points.damroll = tmp3;
			mob->specials.damnodice = tmp;
			mob->specials.damsizedice = tmp2;
		}else if(!strncasecmp(szTag,"GOL",3)){
			mob->points.gold=liGetNumber(new,"GOL");
		}else if(!strncasecmp(szTag,"EXP",3)){
			GET_EXP(mob)=liGetNumber(new,"EXP");
			if(GET_EXP(mob)>100000000){
			   	GET_EXP(mob)=1000000000;
	 	  		sprintf(log_buf,"##%s exp to high, adjusting...\n\r",GET_NAME(mob));
	   			log_hd(log_buf);
        	}
			if(GET_EXP(mob)<1){
	   			GET_EXP(mob)=1;
	   			sprintf(log_buf,"##%s exp to low, adjusting...\n\r",GET_NAME(mob));
	   			log_hd(log_buf);
        	}
		}else if(!strncasecmp(szTag,"POS",3)){
		    mob->specials.position=liGetNumber(new,"POS");
		    mob->specials.mob_position=mob->specials.position;
		}else if(!strncasecmp(szTag,"DPO",3)){
			mob->specials.default_pos=liGetNumber(new,"DPO");
		}else if(!strncasecmp(szTag,"SEX",3)){
			mob->player.sex=liGetNumber(new,"SEX");;
		}else if(!strncasecmp(szTag,"MOV",3)){
			mob->points.max_move=liGetNumber(new,"MOV");
			mob->points.move=mob->points.max_move;
		}else if(!strncasecmp(szTag,"STR",3)){
			mob->abilities.str=liGetNumber(new,"STR");
		}else if(!strncasecmp(szTag,"INT",3)){
			mob->abilities.intel=liGetNumber(new,"INT");
		}else if(!strncasecmp(szTag,"WIS",3)){
			mob->abilities.wis=liGetNumber(new,"WIS");
		}else if(!strncasecmp(szTag,"DEX",3)){
			mob->abilities.dex=liGetNumber(new,"DEX");
		}else if(!strncasecmp(szTag,"CON",3)){
			mob->abilities.con=liGetNumber(new,"CON");
		}else if(!strncasecmp(szTag,"STA",3)){
			mob->abilities.sta=liGetNumber(new,"STA");
		}else if(!strncasecmp(szTag,"!KIC",4)){
			SET_BIT(mob->denyclass,DENYKICK);
		}else if(!strncasecmp(szTag,"!BAC",4)){
			SET_BIT(mob->denyclass,DENYBACKSTAB);
		}else if(!strncasecmp(szTag,"!TRI",4)){
			SET_BIT(mob->denyclass,DENYTRIP);
		}else if(!strncasecmp(szTag,"!BAS",4)){
			SET_BIT(mob->denyclass,DENYBASH);
		}else if(!strncasecmp(szTag,"!CHA",4)){
			SET_BIT(mob->denyclass,DENYCHARGE);
		}else if(!strncasecmp(szTag,"!DIS",4)){
			SET_BIT(mob->denyclass,DENYDISARM);
		}else if(!strncasecmp(szTag,"!THR",4)){
			SET_BIT(mob->denyclass,DENYTHROW);
		}else if(!strncasecmp(szTag,"!SOL",4)){
			SET_BIT(mob->denyclass,DENYSOLID);
		}else if(!strncasecmp(szTag,"MOU",3)){
			SET_BIT(mob->player.siMoreFlags,MOUNT);
		}else if(!strncasecmp(szTag,"FLY",3)){
			SET_BIT(mob->player.siMoreFlags,FLY);
		}
	}while(szTag[0]!='#');
    if(new)
    	fprintf(new,"\n");
	if(!mob->points.mana){
		mob->points.mana = 100;
    	for(x=0;x<GET_LEVEL(mob);x++)
	    	mob->points.mana+=number(10,30);
		mob->points.max_mana = mob->points.mana;
	}
	if(!mob->points.move){
		mob->points.move = 82;
		mob->points.max_move = 82;
	}
    if(new){
		free_char(mob);
		fclose(mob_f);
		open_files--;
		return(NULL);
    }
    return(mob);
}

long int liGetNumber(FILE *new, char *szpTag)
{
char szData[81];
long lTmp, lTmp2, lTmp3, lOutcome;

	fscanf(mob_f," %s ",szData);
	if(charinstring(szData,'d')&&charinstring(szData,'+')){
		sscanf(szData, " %ldd%ld+%ld ", &lTmp, &lTmp2, &lTmp3);
		if(new)
		    fprintf(new,"%s %ldd%ld+%ld ",szpTag,lTmp, lTmp2, lTmp3);
		lOutcome=dice(lTmp, lTmp2)+lTmp3;
    }else{
    	lOutcome=atol(szData);
		if(new)
		    fprintf(new,"%s %ld ",szpTag, lOutcome);
    }
	return(lOutcome);
}


/* generate index table for object or monster file */
struct index_data *generate_indices_mobs(FILE *fl, int *top,
	struct index_data *index)
{
    char buf[MAX_STRING_LENGTH];
    int i=0;
    int number;
    rewind(fl);
    for (;;)
    {
	if (fgets(buf, 81, fl))
	{
	    if (*buf == '#')
	    {
		/* allocate new cell */
		if ( i >= MAX_INDEX )
		{
		    perror( "Too many indexes" );
			SUICIDE;
		}
		i++;		
		sscanf(buf, "#%d", &number);
     		index[number].pos=ftell(fl);
		index[number].number = 0;
		index[number].func=NULL;
	    }
	    else 
		if (*buf == '$'){    /* EOF */
     		    index[number].pos=0;
		    break;
		}
	}
	else
	{
	    perror("generate indices");
	    SUICIDE;
	}
    }
    *top = i - 2;
    for(i=0;i<MAX_MOB;i++)
	if(index[i].pos)
	    mobs[i]=load_mobile(i,0,0);
    sprintf(log_buf,"#mobs with class %d - Without class %d - Total %d",num_mob_class,num_mob_classless,num_mob_classless+num_mob_class);
    log_hd(log_buf);
    sprintf(log_buf,"MOBS: Mage=%d Cleric=%d Warrior=%d Thief=%d Other=%d",num_mob_magic_user,num_mob_cleric,num_mob_warrior,num_mob_thief,num_mob_other);
    log_hd(log_buf);
    return(index);
}


struct char_data *read_mobile(int nr, int type)
{
    int i,x;
    struct char_data *mob=NULL;
    struct obj_data *obj=NULL;
    struct specialsP_data *ptmp=NULL;

    i = nr;

    if(nr<0||nr>MAX_MOB||!mobs[nr]){
		sprintf(log_buf,"##NO MOBILE #%d in READ_MOBILE",nr);
		log_hd(log_buf);
		return(NULL);
    }

    CREATE(mob, struct char_data, 1);
    clear_char(mob);
    if(nr==9800){
		CREATE( ptmp, struct specialsP_data, 1);
		mob->p=ptmp;
    }
    /***** String data *** */

#ifdef HASHTABLE
    mob->player.name = mobs[nr]->player.name;
    mob->player.short_descr = mobs[nr]->player.short_descr;
    mob->player.long_descr = mobs[nr]->player.long_descr;
    mob->player.description = mobs[nr]->player.description;
#else
    mob->player.name = str_dup(mobs[nr]->player.name);
    mob->player.short_descr = str_dup(mobs[nr]->player.short_descr);
    mob->player.long_descr = str_dup(mobs[nr]->player.long_descr);
    mob->player.description = str_dup(mobs[nr]->player.description);
#endif

    mob->player.title = MED_NULL;
    mob->specials.last_attack[0]=MED_NULL;
    /* *** Numeric data *** */

    mob->specials.act = mobs[nr]->specials.act;
    SET_BIT(mob->specials.act, ACT_ISNPC);

    mob->specials.affected_by = mobs[nr]->specials.affected_by;

    mob->specials.alignment = mobs[nr]->specials.alignment;
   
	GET_LEVEL(mob) = GET_LEVEL(mobs[nr]);
    mob->abilities.str   = mobs[nr]->abilities.str;
    mob->abilities.intel = mobs[nr]->abilities.intel;
    mob->abilities.wis   = mobs[nr]->abilities.wis;
    mob->abilities.dex   = mobs[nr]->abilities.dex;
    mob->abilities.con   = mobs[nr]->abilities.con;
    mob->abilities.sta   = mobs[nr]->abilities.sta;
    if (GET_LEVEL(mob) >= 30){
        mob->abilities.str   = 18;
        mob->abilities.intel = 18;
        mob->abilities.wis   = 18;
        mob->abilities.dex   = 18;
        mob->abilities.con   = 18;
    }else{
		if(mob->abilities.str==13)
	        mob->abilities.str   = MIN(18,13 + number(0,(GET_LEVEL(mob) % 6)));
		if(mob->abilities.intel==13)
    	    mob->abilities.intel = MIN(18,13 + number(0,(GET_LEVEL(mob) % 6)));
		if(mob->abilities.wis==13)
        	mob->abilities.wis   = MIN(18,13 + number(0,(GET_LEVEL(mob) % 6)));
		if(mob->abilities.dex==13)
	        mob->abilities.dex   = MIN(18,13 + number(0,(GET_LEVEL(mob) % 6)));
		if(mob->abilities.con==13)
    	    mob->abilities.con   = MIN(18,13 + number(0,(GET_LEVEL(mob) % 6)));

        switch ( GET_CLASS(mob) )
            {
            case CLASS_MAGIC_USER:
				if(mob->abilities.intel==13)
	                mob->abilities.intel = MIN(18,16 + number(0,2));
                break;
            case CLASS_CLERIC:
				if(mob->abilities.wis==13)
     	           mob->abilities.wis   = MIN(18,16 + number(0,2));
                break;
            case CLASS_WARRIOR:
				if(mob->abilities.str==13)
	                mob->abilities.str   = MIN(18,16 + number(0,2));
                break;
            case CLASS_THIEF:
				if(mob->abilities.dex==13)
                	mob->abilities.dex   = MIN(18,16 + number(0,2));
                break;
            default:
                break;
            }
        }
    mob->tmpabilities = mob->abilities;

	mob->player.siMoreFlags=mobs[nr]->player.siMoreFlags;	
	mob->points.hitroll = mobs[nr]->points.hitroll;
	
	mob->points.armor = mobs[nr]->points.armor;

	mob->points.max_hit = mobs[nr]->points.max_hit;
	mob->points.hit = mob->points.max_hit;

	mob->points.max_move = mobs[nr]->points.max_move;
	mob->points.move = mob->points.max_move;

	mob->points.damroll =  mobs[nr]->points.damroll;
	mob->specials.damnodice = mobs[nr]->specials.damnodice;
	mob->specials.damsizedice = mobs[nr]->specials.damsizedice;

	if(!mob->points.mana){
		mob->points.mana = 100;
        for(x=0;x<GET_LEVEL(mob);x++)
	    	mob->points.mana+=number(10,30);
		mob->points.max_mana = mob->points.mana;
	}

	mob->points.gold = mobs[nr]->points.gold;

	GET_EXP(mob) = GET_EXP(mobs[nr]);

	mob->specials.position = mobs[nr]->specials.position;
        mob->specials.mob_position=mobs[nr]->specials.mob_position;
	mob->specials.default_pos = mobs[nr]->specials.default_pos;

	mob->player.sex = mobs[nr]->player.sex;

	mob->player.class = mobs[nr]->player.class;

	mob->player.time.birth = time(0);
	mob->player.time.played = 0;
	mob->player.time.logon  = time(0);
    if (mob->player.sex == SEX_MALE)
    	{
		mob->player.weight = number(120,180);
    	mob->player.height = number(160,200);
    	}
    else
    	{
    	mob->player.weight = number(100,160);
    	mob->player.height = number(150,180);
    	}

	for (i = 0; i < 3; i++)
	    GET_COND(mob, i) = -1;

	for (i = 0; i < 5; i++)
	    mob->specials.apply_saving_throw[i] = 0/*MAX(20-GET_LEVEL(mob), 2)*/;


    mob->tmpabilities = mob->abilities;

    for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
	mob->equipment[i] = NULL;

    mob->nr = nr;

    mob->desc = NULL;

    mob->formation[0][1]=mob;
    mob->master=mob;
    /* insert in list */

    mob->next = character_list;
    character_list = mob;

    mob_index[nr].number++;

    if(GET_CLASS(mob)==CLASS_THIEF){/*load 3 throwables */
	if(GET_LEVEL(mob)>=25){
	    obj=read_object(708,0);
	    obj_to_char(obj,mob);	
	    obj=read_object(708,0);
	    obj_to_char(obj,mob);	
	    obj=read_object(708,0);
	    obj_to_char(obj,mob);	
	}else if(GET_LEVEL(mob)>=20){
            obj=read_object(707,0); 
            obj_to_char(obj,mob); 
            obj=read_object(707,0); 
            obj_to_char(obj,mob); 
            obj=read_object(707,0); 
            obj_to_char(obj,mob); 
        }else if(GET_LEVEL(mob)>=15){
            obj=read_object(706,0); 
            obj_to_char(obj,mob); 
            obj=read_object(706,0); 
            obj_to_char(obj,mob); 
            obj=read_object(706,0); 
            obj_to_char(obj,mob); 
        }else if(GET_LEVEL(mob)>=9){
            obj=read_object(4003,0); 
            obj_to_char(obj,mob); 
            obj=read_object(4003,0); 
            obj_to_char(obj,mob); 
            obj=read_object(4003,0); 
            obj_to_char(obj,mob); 
        }else{
            obj=read_object(109,0); 
            obj_to_char(obj,mob); 
            obj=read_object(109,0); 
            obj_to_char(obj,mob); 
            obj=read_object(109,0); 
            obj_to_char(obj,mob); 
        }
    }
	mob->denyclass=mobs[nr]->denyclass;
    return(mob);
}

#define ZCMD zone_table[zone].cmd[cmd_no]
void do_save_mobs(struct char_data *ch, char *argument, int cmd)
{               
FILE *mob_fh,*dead_mobs;
char full_filename[255];
char filename[255];
int mob,zone,loaded,cmd_no;

    if(strcmp(GET_NAME(ch),"Vryce")){
		send_to_char("This is a VRYCE only command.\n\r",ch);
		return;
    }
    one_argument(argument,filename);
    if(!filename||!filename[0]){
		send_to_char("No filename given.\n\r",ch);
		return;
    }
    strcpy(full_filename,"../lib/");
    strcat(full_filename,filename);
    if (!(mob_fh = med_open(full_filename, "w"))){
		send_to_char("Could not open file!\n\r",ch);
		return;
    }
    open_files++;
    send_to_char("Starting ....",ch);
    for(mob=0;mob<MAX_MOB;mob++){
        if(!mobs[mob])
        	continue;
		loaded=0;
        for ( zone = 0; zone < MAX_ZONE; zone++ ){ 
            if ( zone_table[zone].reset_mode == -1 )
            	continue;
	    	for (cmd_no = 0;;cmd_no++){
        		if (ZCMD.command == 'S')
            		    break;
	            if(ZCMD.command=='M'){
				    if(ZCMD.arg1==mob)
						loaded++;
				}   
		    }
   		 }
		if(!loaded){
		    fclose(hold2);hold2=NULL;
    		if (!(dead_mobs = fopen("../lib/dead_mobs.list", "a"))){
				send_to_char("Could not open dead_mobs.list file!\n\r",ch);
				hold2=fopen("../lib/hold2.dat","w");
				return;
    	    }
		    fprintf(dead_mobs,"%d %s\n",mob,mobs[mob]->player.short_descr);
		    fclose(dead_mobs);
	    	hold2=fopen("../lib/hold2.dat","w");
		}
		load_mobile(mob,0,mob_fh);
    }
    fprintf(mob_fh,"#19000\n$~\n");
    med_close(mob_fh);
	open_files--;
    send_to_char("Done!\n\r",ch);
}
#undef ZCMD
