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
#include <time.h>
#ifndef __FreeBSD__
#include <malloc.h>
#endif
#include <ctype.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"

int open_files=0;
int open_descs=0;

extern struct time_data time_info;
extern struct descriptor_data *descriptor_list;
extern char global_color;
char    log_buf[MAX_STRING_LENGTH];
char page_setup[100000];
char free_error[200];
FILE *hold1,*hold2;

bool DigitString(char *szpText)
{
int x;

	if(!szpText||!szpText[0])
		return(FALSE);
	for(x=0;x<strlen(szpText);x++)
		if(!isdigit(szpText[x]))
			return(FALSE);
	return(TRUE);
}

void donation_whine(void)
{
struct descriptor_data *d;
int l;
char buf[100];
	for(d=descriptor_list;d;d=d->next){
		if(!d->character)
			continue;
		if((l=get_total_level(d->character))<10)
			continue;
		if(d->character->p->donated)
			continue;
		if(l>124)continue;
		if(l>60)
			l=60;
		l-=10;
		l/=3;
		if(l<=number(0,35)){
		    global_color=31;
		sprintf(buf,"You have played %ld hours and have not donated.\n\r",(d->character->player.time.played + time(0) - d->character->player.time.logon) / 3600);	
		send_to_char(buf,d->character);		 
			global_color=32;
		    send_to_char("
Medievia is growing rapidly and has and will continue to be the best
entertainment on the net.  Medievia IV, code wise and world/data wise is,
we believe, the largest game in the world.  We need donations to help offset
the price of our new machine we will be buying soon.  Please, if you play
Medievia hours each day and find it a great source of entertainment just
type HELP DONATIONS.  Medievia is free but we do need your support.
Please think about supporting your game.  Thankyou.
		    ",d->character);
		    global_color=0;
		}
	}

}

FILE *med_open(char *filename, char *type)
{
FILE *fh;

    if(hold1==NULL){
	log_hd("##serious holding file problem in med_open");
    }else{
        fclose(hold1);
    }
    hold1=NULL;
    fh=fopen(filename,type);
    if(fh==NULL){
  	hold1=fopen("../lib/hold1.dat","w");
	return(NULL);
    }
    return(fh);
}

void med_close(FILE *stream)
{
    fclose(stream);
    if(hold1){
	log_hd("##serious holding file problem in med_close");
    }
    hold1=fopen("../lib/hold1.dat","w");
}


/* get next line, fill in out with it, return pointer after out*/
char *get_next_line(char *in, char *out)
{
char *p;
char *op,opp=0;

   op=out;
   p=in;
   while(p[0]!=MED_NULL&&p[0]!='\n'&&p[0]!='\r'){
	op[opp++]=p[0];
	p++;
   }
   while(p[0]!=MED_NULL&&(p[0]=='\n'||p[0]=='\r')){
	op[opp++]=p[0];
	p++;
   }
   op[opp]=MED_NULL;
   if(p[0]==MED_NULL)return(MED_NULL);
   return(p);
}

int get_total_level(struct char_data *ch)
{
int level;

   if(GET_LEVEL(ch)>31)
	return(GET_LEVEL(ch)*4);
   level=0;
   if(IS_SET(ch->player.multi_class,MULTI_CLASS_MAGIC_USER))
	level+=31;   
   if(IS_SET(ch->player.multi_class,MULTI_CLASS_CLERIC))
	level+=31;   
   if(IS_SET(ch->player.multi_class,MULTI_CLASS_THIEF))
	level+=31;   
   if(IS_SET(ch->player.multi_class,MULTI_CLASS_WARRIOR))
	level+=31;   
   return(GET_LEVEL(ch)+level);
}

void set_to_lower(char *out, char *in)
{
int x;
    x=0;
    while(in[x]){
	out[x]=LOWER(in[x]);
	x++;
    }
    out[x]=MED_NULL;
}
void set_to_upper(char *out, char *in)
{
int x;
    x=0;
/*
    log_hd(in);log_hd("::");
*/
    while(in[x]){
	out[x]=UPPER(in[x]);
	x++;
    }
    out[x]=MED_NULL;
}

void space_to_underline(char *text)
{
int x=-1;
    while(text[++x])
	if(text[x]==' ')text[x]='_';
}

void *my_free(void *ptr)
{

    if (ptr!=NULL)
    {
        if(!strcmp(ptr,"@"))
 	    return(NULL);
	strcpy(ptr,"@");
	free(ptr);
    }
    return(NULL);
}

/*
 * Generates a random number.
 */
int number( int from, int to )
{
    if ( from >= to )
	return from;

    return from + random() % (to - from + 1);
}



/*
 * Simulates dice roll.
 */
int dice( int number, int size )
{
    int r;
    int sum = number;

    for ( r = 1; r <= number; r++ )
	sum += random() % size;

    return sum;
}



/*
 * Compare strings, case insensitive.
 */
int str_cmp( char *arg1, char *arg2 )
{
    int check, i;

    for ( i = 0; arg1[i] || arg2[i]; i++ )
    {
	check = LOWER(arg1[i]) - LOWER(arg2[i]);
	if ( check < 0 )
	    return -1;
	if ( check > 0 )
	    return 1;
    }

    return 0;
}



/*
 * Duplicate a string into dynamic memory.
 */
char *str_dup( const char *str )
{
    char *str_new;

    CREATE( str_new, char, strlen(str) + 1 );
    strcpy( str_new, str );
    return str_new;
}



/* writes a string to the log */
void log_hd( char *str )
{
    long ct;
    char buf[MAX_STRING_LENGTH];
    char *tmstr;
    struct descriptor_data *i=NULL;
    ct		= time(0);
    tmstr	= asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';

    fprintf(stderr, "%s :: %s\n", tmstr, str);
    if(strstr(str,"##")){
	global_color=31;
        for (i = descriptor_list; i; i = i->next)
        if (i->character  && !i->connected &&
                GET_LEVEL(i->character) > 30)
	   if(IS_SET(i->character->specials.god_display,GOD_ERRORS)){
		sprintf(buf,"(MISC info) %s\n\r",str);
		send_to_char(buf,i->character);
	   }
	global_color=0;
    }
}



void sprintbit( long vektor, char *names[], char *result )
{
    long nr;

    *result = '\0';

    for ( nr=0; vektor; vektor>>=1 )
    {
	if ( IS_SET(1, vektor) )
	{
	    if ( *names[nr] != '\n' )
		strcat( result, names[nr] );
	    else
		strcat( result, "Undefined" );
	    strcat( result, " " );
	}

	if ( *names[nr] != '\n' )
	  nr++;
    }

    if ( *result == '\0' )
	strcat( result, "NoBits" );
}



void sprinttype( int type, char *names[], char *result )
{
    int nr;

    for ( nr = 0; *names[nr] != '\n'; nr++ )
	;
    if ( type < nr )
	strcpy( result, names[type] );
    else
	strcpy( result, "Undefined" );
}


/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs/SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
    secs -= SECS_PER_MUD_HOUR*now.hours;

    now.day = (secs/SECS_PER_MUD_DAY) % 35;     /* 0..34 days  */
    secs -= SECS_PER_MUD_DAY*now.day;

    now.month = (secs/SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
    secs -= SECS_PER_MUD_MONTH*now.month;

    now.year = (secs/SECS_PER_MUD_YEAR);        /* 0..XX? years */

    return now;
}



struct time_info_data age(struct char_data *ch)
{
    struct time_info_data player_age;

    player_age = mud_time_passed(time(0),ch->player.time.birth);

    player_age.year += 17;   /* All players start at 17 */

    return player_age;
}

int IS_PLAYER(struct char_data *ch, char *player)
{
if(!ch)
	return(FALSE);

if( IS_NPC(ch) )
	{
	if ( ch->desc && ch->desc->original)
		{
		if( strcmp(player,GET_NAME(ch->desc->original)) )
			return(FALSE);
		else
			return(TRUE);
		}
	else
		return(FALSE);
	}
else
	{
	if( strcmp(player, GET_NAME(ch)) )
		return(FALSE);
	else
		return(TRUE);
	}
}

char *GET_REAL_NAME(struct char_data *ch)
{
if (!ch)
	return("Nobody");

if( IS_NPC(ch) )
	{
	if ( ch->desc && ch->desc->original)
		return(GET_NAME(ch->desc->original));
	else
		return(GET_NAME(ch));
	}
else
	return(GET_NAME(ch));
}

#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
int strncasecmp(char *s1, char *s2, int count)
{
int i, check;

    for(i=0;(s1[i]||s2[i])&&i<count;i++){
		check=LOWER(s1[i])-LOWER(s2[i]);
		if(check<0)
	    	return(-1);
		if(check>0)
		    return(1);
    }
	return(0);
}
#endif

int charinstring(char *s1, char c)
{
char *p;

	for(p=s1;p[0];p++)
		if(p[0]==c)
			return(1);
	return(0);
}
