/***************************************************************************
*					 MEDIEVIA CyberSpace Code and Data files		       *
*       Copyright (C) 1992, 1996 INTENSE Software(tm) and Mike Krause	   *
*			   				All rights reserved				               *
***************************************************************************/
/***************************************************************************
* This program belongs to INTENSE Software, and contains trade secrets of  *
* INTENSE Software.  The program and its contents are not to be disclosed  *
* to or used by any person who has not received prior authorization from   *
* INTENSE Software.  Any such disclosure or use may subject the violator   *
* to civil and criminal penalties by law.                                  *
***************************************************************************/


#include <sys/types.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#ifndef __FreeBSD__
#include <malloc.h>
#endif
#include <errno.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h> 

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"

#define NUM_DRAGON_CRYSTALS 20
#define STATE(d) ((d)->connected)
extern void MoveFlyingPeople(void);
extern void Shake(void);
extern void StopFlying(struct char_data *stpCh);
extern void StartEarthquake(void);
extern void move_catacomb_moveable(void);
extern void DragonControl(void);
extern void stalactite(void);
extern char catacombs_loaded;
extern struct obj_data *make_newspaper(void);
extern int giNewsVersion;
extern struct obj_data *DA_paper;
#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
extern int bzero(char *b, int length);
extern int getrlimit(int resource, struct rlimit *rlptr);
extern int setrlimit(int resource, const struct rlimit *rlptr);
extern int socket( int domain, int type, int protocol );
extern int listen( int s, int backlog );
#endif
extern int malloc_debug(int level);
extern bool is_formed(struct char_data *ch);
extern int return_perc(int amount, int times);
extern int getdtablesize(void);
extern struct EARTHQUAKE *stpEarthquake;
extern struct index_data *obj_index;
extern int  port;
extern char MOUNTMOVE;
extern void perform_hovering(void);
extern void perform_undead(void);
extern auto_group_all_mobs(void);
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern FILE *hold1;
extern FILE *hold2;
extern FILE *log_fh;
extern struct char_data *character_list;
extern void free_zone(int zone);
extern void cleric_preprompt(struct char_data *ch);
extern void setup_raction_design(void);
extern void fire_raction(int room, struct char_data *ch, int action_num);
/*extern int getrlimit(int resource, struct rlimit *rlptr);*/
/*extern int setrlimit(int resource, const struct rlimit *rlptr);*/
/*extern int socket( int domain, int type, int protocol );*/
extern void room_affect_update(void);
#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
extern int setsockopt( int s, int level, int optname,
    void *optval, int optlen );
extern int bind( int s, struct sockaddr *name, int namelen );
#endif
extern int select( int width, fd_set *readfds, fd_set *writefds,
    fd_set *exceptfds, struct timeval *timeout );
/*extern int listen( int s, int backlog );*/
#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
extern int getsockname( int s, struct sockaddr *name, int *namelen );
extern int accept( int s, struct sockaddr *addr, int *addrlen );
extern int getpeername( int s, struct sockaddr *name, int *namelen );
#endif
extern void roll_slot(struct char_data *ch);
extern void load_clan_info(void);
extern void do_editor(struct char_data *ch, char *arg, int cmd);
#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
extern int gettimeofday( struct timeval *tp, struct timezone *tzp );
extern char * inet_ntoa( struct in_addr *in );
#endif
extern void load_dragon_crystal(void);
extern void mob_non_combat_inteligence(void);
extern void damage_room_update(void);
struct char_data *vryce;
#define DFLT_PORT   4000
#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME    256
#define OPT_USEC    250000      /* Microseconds per pass    */


extern int errno;
extern int top_trivia;

/* externs */

extern char greetings[MAX_STRING_LENGTH];
extern char greetings_ansi[MAX_STRING_LENGTH];
extern struct raction_timing *raction_seconds;
extern struct room_actions *ractions[MAX_RACTION];
extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct time_info_data time_info;  /* In db.c */
extern char help[];
extern char *trivia[];
extern struct ban_t *ban_list;        /* In db.c */
extern int slot_rooms[];
extern int iFlyStoreRoom;

struct char_data *ACTTO;
char daytimedown=FALSE;
char MOUNTMOVE;
/* Internal declarations */
void do_trivia(void);

/* local globals */
int high_mem;
char get_host;
char grep_text[250];
char global_color;
char grep_flag;
long med_current_time;
struct timeval med_now_time;
unsigned long int connect_count;
struct descriptor_data *descriptor_list, *next_to_process, *sort_d;
struct char_data *finger_ch;
char use_memory_debug;
char use_hash_table;
int pulse;
int iaShownHints[20], iShownHintsP;
char COLRED[]="\033[1m\033[31m";
char COLYEL[]="\033[1m\033[33m";
char COLGRN[]="\033[1m\033[32m";
char COLBLU[]="\033[1m\033[34m";
time_t ttNextQuake;
/*	this is a flag set by close_socket and check_idling to indicate that 
	the guy has been deleted so we don't try to use a freed data structure.
	The game loop was trying to force a prompt to the no longer exitsting 
	descriptor.  Descriptors can be freed when player quits, rents, or types 
	in bad password. point_update was trying to continute updating chars
	who had already been autorented by check_idling.
*/
bool guy_deleted = FALSE;

char NOPKCHANGEMESSAGE;
char NODEATHTRAP;

int god    = 0;      /* all new chars are gods! */
int slow_death = 0;  /* Shut her down, Martha, she's sucking mud */
int medievia_shutdown = 0;    /* clean shutdown */
int maxdesc;
char str_boot_time[MAX_INPUT_LENGTH];


void shutdown_request(void);
void logsig(void);
void hupsig(void);

char * get_from_q(struct txt_q *queue);
void game_loop(int s);
int init_socket(int port);
int new_connection(int s);
int new_descriptor(int s);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void close_socket(struct descriptor_data *d);
void flush_queues(struct descriptor_data *d);
void nonblock(int s);
void parse_name(struct descriptor_data *desc, char *arg);
int number_playing(void);
void sort_descriptors(void);

/* extern functions */
extern void PigeonMessageUpdate(void);
struct char_data *make_char(char *name, struct descriptor_data *desc);
void boot_db(void);
void zone_update(void);
void affect_update( void ); /* In spells.c */
void point_update( void );  /* In limits.c */
void mobile_activity(void);
void string_add(struct descriptor_data *d, char *str);
void perform_violence(void);
void stop_fighting(struct char_data *ch);
void show_string(struct descriptor_data *d, char *input);
void sort_board(void);
void donation_whine(void);
int  port	= DFLT_PORT;

void do_togglehost(struct char_data *ch, char *argument, int cmd)
{
    if(get_host)
	send_to_char("turning off.\n\r",ch);
    else
	send_to_char("turning on.\n\r",ch);
    if(get_host)get_host=0;
    else get_host=1;
}

#ifdef NEWLIB
void mcheck_hook(enum mcheck_status STATUS)
{
char *DIEDIEDIE=NULL;

    switch(STATUS){
	case MCHECK_DISABLED:
	    fprintf(stderr,"MCHECK_DISABLED:
	    'mcheck' was not called before the first allocation.  No
          consistency checking can be done.
	    ");
	    break;
        case MCHECK_OK:
            printf("MCHECK_OK
            ");
            break;
        case MCHECK_HEAD:
            fprintf(stderr,"MCHECK_HEAD:
          The data immediately before the block was modified.  This
          commonly happens when an array index or pointer is
          decremented too far.
            ");
            *DIEDIEDIE='V';
            break;
        case MCHECK_TAIL:
            fprintf(stderr,"MCHECK_TAIL:
          The data immediately after the block was modified.  This
          commonly happens when an array index or pointer is
          incremented too far.
            ");
            *DIEDIEDIE='V';
            break;
        case MCHECK_FREE:
            fprintf(stderr,"MCHECK_FREE:
          The block was already freed.
            ");
            *DIEDIEDIE='V';
            break;
    }
}
#endif

int main( int argc, char *argv[] )
{
    int pos,x;
    char *dir	= DFLT_DIR;
    int control;
    FILE *fh;
    struct char_data *i=NULL,*o=NULL;
    struct rlimit	rlp;

#ifdef MEMDEBUG
		use_memory_debug=TRUE;
		log_hd("*******USING MEMORY_DEBUG********");
		malloc_debug(1);
#else
		use_memory_debug=FALSE;
		log_hd("*******NOT USING MEMORY_DEBUG********");
#endif

#ifdef HASHTABLE
		use_hash_table=TRUE;
#else
		use_hash_table=FALSE;
#endif

	MOUNTMOVE=0;
	high_mem=0;
	hold1=fopen("../lib/hold1.dat","w");
	hold2=fopen("../lib/hold2.dat","w");
	log_fh=fopen("/home/medievia/src/data.dat","a");
        get_host=0;
	log_buf[0]='\0';
        fprintf(stderr, "\n\n**CONFIGURING DESCRIPTORS**\n");
	fprintf(stderr, "descriptors limit : %d\n", getdtablesize());
	getrlimit(RLIMIT_NOFILE, &rlp);
	fprintf(stderr, "files old soft    : %d\n", rlp.rlim_cur);
	fprintf(stderr, "files old hard    : %d\n", rlp.rlim_max);
	rlp.rlim_cur = rlp.rlim_max;

	fprintf(stderr, "Setting rlimit______Number files\n");
	setrlimit(RLIMIT_NOFILE, &rlp);
	fprintf(stderr, "descriptors limit : %d\n", getdtablesize());
	fprintf(stderr, "files new soft    : %d\n", rlp.rlim_cur);
	fprintf(stderr, "files new hard    : %d\n\n\n", rlp.rlim_max);

	fprintf(stderr, "\n**CONFIGURING THE STACK**\n");
	getrlimit(RLIMIT_STACK, &rlp);
	fprintf(stderr, "stack old soft    : %d\n", rlp.rlim_cur);
	fprintf(stderr, "stack old hard    : %d\n", rlp.rlim_max);
	rlp.rlim_cur = 40960000;

	fprintf(stderr, "Setting rlimit______Stack size\n");
	setrlimit(RLIMIT_STACK, &rlp);
	fprintf(stderr, "stack new soft    : %d\n", rlp.rlim_cur);
	fprintf(stderr, "stack new hard    : %d\n\n\n", rlp.rlim_max);

    connect_count=0;
    if((fh=fopen("../lib/connected.dat", "r"))!=NULL){
        fread(&connect_count,sizeof(unsigned long int), 1, fh);
        fclose(fh);
    }
    if((fh=fopen("../lib/closedforbusiness", "r"))!=NULL){
	daytimedown=TRUE;
        fclose(fh);
    }
    raction_seconds=NULL;
    setup_raction_design();
    strcpy(no_title,"No Title");
    global_color=0;
    gettimeofday( &med_now_time, NULL );
    med_current_time = med_now_time.tv_sec; 
    strcpy(str_boot_time, ctime(&med_current_time)); 
    load_clan_info();
	for(iShownHintsP=0;iShownHintsP<20; iShownHintsP++)
		iaShownHints[iShownHintsP]=0;
	iShownHintsP=0;    
    for ( pos = 1; pos < argc && argv[pos][0] == '-' ; pos++ )
    {
	switch (*(argv[pos] + 1))
	{
	case 'g':
	    god = 1;
	    log_hd( "God creation mode selected." );
	    break;

	case 'd':
	    if ( argv[pos][2] != '\0' )
		dir = &argv[pos][2];
	    else if (++pos < argc)
		dir = &argv[pos][0];
	    else
	    {
		fprintf( stderr, "Directory arg expected after -d.\n\r" );
		exit( 1 );
	    }
	    break;

	default:
	    fprintf( stderr, "Unknown option -%c.\n\r", argv[pos][1] );
	    exit( 1 );
	    break;
	}
    }
    
    if (pos < argc)
    {
	if (!isdigit(argv[pos][0]))
	{
	    fprintf( stderr, "Usage: %s [-g] [-d pathname] [port #]\n", 
		argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi(argv[pos]) ) <= 1024 )
	{
	    printf( "Illegal port #\n" );
	    exit( 1 );
	}
    }

    sprintf( log_buf, "[Port %d] [Dir %s].", port, dir );
    log_hd( log_buf );

    if ( chdir( dir ) < 0 )
    {
	perror( dir );
	exit( 1 );
    }
    
    srandom( time(0) );

    /*
     * Optional memory tuning.
     */
#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
    mallopt( M_MXFAST, 96 );
    mallopt( M_NLBLKS, 1024 );
    mallopt( M_GRAIN, 4 );
#endif
    /* NOTE GRAIN is REAL important as it will not allow malloc to allocate
       anything less then 4 bytes, and since unix will allocate 0 bytes it
       causes CHAOS! */
    signal( SIGPIPE, SIG_IGN );
    control   = init_socket( port );
    boot_db( );
    sort_board();    
    DA_paper=make_newspaper();
    x=0;
    if(port!=1220)
    while(slot_rooms[x]!=-1){
	sprintf(log_buf,"../casino/slot.%d.dat",slot_rooms[x]);
        if((fh=fopen(log_buf, "r"))!=NULL){
	    fread(&world[slot_rooms[x]]->pressure_mod, sizeof(int), 1, fh);
       	fclose(fh);
        }
	x++;
    }

    game_loop( control );

/* juice
 * search and destroy link dead people too
 */

    i=character_list;
	MOUNTMOVE=TRUE;
    while(i){
        if(!IS_NPC(i) || IS_UNDEAD(i)){
            o=i->next;
			StopFlying(i);			
            do_saveout(i,GET_NAME(i),99);
            i=o;
        }else{
            i=i->next;
        }
    }

    log_hd( "Normal termination of game." );
    exit( 0 );
    return 0;
}
void prompt(struct descriptor_data *point)
{
int row,col,p=101,pp;
struct char_data *player=NULL,*ch=NULL,*stpMoveCh;
char buf[400],*ansih,*ansim,*ansiv,*ansib;

    if ( !IS_SET(ORIGINAL(point->character)->specials.act, PLR_COMPACT) )
		write_to_q( "\n\r", &point->output );

	ch=point->character;

	if(ch->desc->connected == CON_SOCIAL_ZONE&&ch->p&&ch->p->iNewsVersion<giNewsVersion){
		sprintf(buf,"%s[NEWS]%s<MedLink>",YEL(ch),NRM(ch));
		write_to_q(buf,&point->output);
		return;
	}
	if(ch->desc->connected == CON_SOCIAL_ZONE) {
		write_to_q("<MedLink> ", &point->output);
		return;
    }
    if(is_formed(ch)&&ch->specials.fighting){
    	for(row=0;row<3;row++)
	    	for(col=0;col<3;col++){
				if(ch->master->formation[row][col]&&
				ch->master->formation[row][col]->in_room==ch->in_room){
					pp=return_perc(GET_HIT(ch->master->formation[row][col]),GET_MAX_HIT(ch->master->formation[row][col]));
					if(pp<p){
						p=pp;
						player=ch->master->formation[row][col];				
					}
				}		    	
	    	}
		if(player){
		    sprintf(log_buf,"<[%2d%%]%s>",p,GET_NAME(player));
			global_color=34;
			if(p<80)global_color=35;
			if(p<60)global_color=32;
			if(p<40)global_color=33;
			if(p<20)global_color=31;
		    send_to_char(log_buf,ch);
	    	global_color=0;
		}    
	}
	buf[0]=MED_NULL;
	if(IS_SET(ORIGINAL(point->character)->specials.new_comm_flags,PLR_TAGGED))
		sprintf(buf+strlen(buf),"%s[TAGGED!]%s", RED(ch),NRM(ch));
	if(point->character->specials.mail_waiting)
		sprintf(buf+strlen(buf),"%s[MAIL]%s",RED(ch),NRM(ch));
    if(point->character->specials.afk)
		sprintf(buf+strlen(buf),"%s[AFK]%s",CYN(ch),NRM(ch));
	if(point->character->p&&point->character->p->iNewsVersion<giNewsVersion)
		sprintf(buf+strlen(buf),"%s[NEWS]%s",YEL(ch),NRM(ch));
    if(GET_LEVEL(point->character)<32){
		if(ch->specials.stpMount&&!IS_NPC(ch))
			stpMoveCh=ch->specials.stpMount;
		else
			stpMoveCh=ch;
	    if(ch->specials.ansi_color==69){
			if(GET_BREATH(stpMoveCh)>75)
				ansib=COLBLU;					
			else if(GET_BREATH(stpMoveCh)>50)
				ansib=COLGRN;					
			else if(GET_BREATH(stpMoveCh)>25)
				ansib=COLYEL;					
			else
				ansib=COLRED;
			if(GET_HIT(ch)>(GET_MAX_HIT(ch)-(GET_MAX_HIT(ch)/4)))
				ansih=COLBLU;					
			else if(GET_HIT(ch)>(GET_MAX_HIT(ch)/2))
				ansih=COLGRN;										
			else if(GET_HIT(ch)>(GET_MAX_HIT(ch)/4))
				ansih=COLYEL;					
			else 
				ansih=COLRED;					
			if(GET_MANA(ch)>(GET_MAX_MANA(ch)-(GET_MAX_MANA(ch)/4)))
				ansim=COLBLU;					
			else if(GET_MANA(ch)>(GET_MAX_MANA(ch)/2))
				ansim=COLGRN;					
			else if(GET_MANA(ch)>(GET_MAX_MANA(ch)/4))
				ansim=COLYEL;					
			else 
				ansim=COLRED;					
			if(GET_MOVE(stpMoveCh)>(GET_MAX_MOVE(stpMoveCh)-(GET_MAX_MOVE(stpMoveCh)/4)))
				ansiv=COLBLU;					
			else if(GET_MOVE(stpMoveCh)>(GET_MAX_MOVE(stpMoveCh)/2))
				ansiv=COLGRN;					
			else if(GET_MOVE(stpMoveCh)>(GET_MAX_MOVE(stpMoveCh)/4))
				ansiv=COLYEL;					
			else 
				ansiv=COLRED;					
			if(stpMoveCh==ch)
		    	sprintf(buf+strlen(buf), "%s<%s%d%shp %s%d%sm %s%d%smv %s%d%sbr>%s ",
				COL_BLU,
				ansih,
    			GET_HIT(point->character),
				COL_BLU,
				ansim,
				GET_MANA(point->character),
				COL_BLU,
				ansiv,
				GET_MOVE(stpMoveCh),
				COL_BLU,
				ansib,
				GET_BREATH(stpMoveCh),
				COL_BLU,
				COL_NRM);
			else
		    	sprintf(buf+strlen(buf), "%s<%s%d%shp %s%d%sm %s%d%sMountMV %s%d%sMountBR>%s ",
				COL_BLU,
				ansih,
    			GET_HIT(point->character),
				COL_BLU,
				ansim,
				GET_MANA(point->character),
				COL_BLU,
				ansiv,
				GET_MOVE(stpMoveCh),
				COL_BLU,
				ansib,
				GET_BREATH(stpMoveCh),
				COL_BLU,
				COL_NRM);
			
		}else{
			if(stpMoveCh==ch)
		    	sprintf(buf+strlen(buf), "<%dhp %dm %dmv %dbr> ",
	    		GET_HIT(point->character),
				GET_MANA(point->character),
				GET_MOVE(stpMoveCh),
				GET_BREATH(stpMoveCh));
			else				
		    	sprintf(buf+strlen(buf), "<%dhp %dm %dMountMV %dMountBR> ",
	    		GET_HIT(point->character),
				GET_MANA(point->character),
				GET_MOVE(stpMoveCh),
				GET_BREATH(stpMoveCh));
		}
	}else if(IN_HOLO(point->character)){
			sprintf(buf+strlen(buf),"<Room#%d %dX %dY>%s",
	  	world[point->character->in_room]->number,world[point->character->in_room]->holox,world[point->character->in_room]->holoy,
		((IS_AFFECTED(point->character,AFF_INVISIBLE)) ? "(INVISIBLE)>" : ""));
	}else{
		sprintf(buf+strlen(buf),"<Room#%d>%s",
	  	world[point->character->in_room]->number,
		((IS_AFFECTED(point->character,AFF_INVISIBLE)) ? "(INVISIBLE)>" : ""));
	}
	write_to_q( buf, &point->output );

}


/* Accept new connects, relay commands, and call 'heartbeat-functs' */
void game_loop( int control )
{
    fd_set input_set, output_set, exc_set;
    static struct timeval null_time = {0, 0};
    struct timeval last_time, now_time, stall_time;
    register struct descriptor_data *point=NULL;
    struct descriptor_data *next_point=NULL;
	struct char_data *stpCh;
    int mask;
    char *pcomm=NULL;
    bool fStall;
    int autosave=0,i;
    /*int pid;*/ /* AVI */
    time_t cur_seconds;
    struct raction_timing *rt=NULL;

    gettimeofday(&last_time, (struct timezone *) 0);

    maxdesc = control;
	pulse=0;

#ifdef sun
# define sigmask(m) (1 << ((m)-1))
#endif

    mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
	sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
	sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP) |
	sigmask(SIGVTALRM);

#if 0
    mallocmap();
#endif
    /*fire all autostart room actions */
    for(i=0;i<MAX_RACTION;i++){
	if(ractions[i]){
	    if(ractions[i]->autostart){
	        fire_raction(ractions[i]->in_room,0,i);
	    }
	}
    }
    time(&cur_seconds);
    ttNextQuake=cur_seconds+number(600,7200);
    /* Main loop */
    while (!medievia_shutdown)
    {
	/* Check what's happening out there */

	FD_ZERO(&input_set);
	FD_ZERO(&output_set);
	FD_ZERO(&exc_set);
	FD_SET(control, &input_set);
	for (point = descriptor_list; point; point = point->next)
	{
	    FD_SET(point->descriptor, &input_set);
	    FD_SET(point->descriptor, &exc_set);
	    FD_SET(point->descriptor, &output_set);
	}

	sigsetmask(mask);

	/* AVI */
/*
	while ( (pid = waitpid(0, NULL, WNOHANG)) > 0)
	{
		for (point = descriptor_list ; point ; point = point->next)
		{
			if (point->child_pid == pid)
			{
				point->ignore_input = 0;
				point->child_pid = 0;

				send_to_char(
					"You have returned to MEDIEVIA.\n\r", 
					point->character );
			}
		}
	}
*/
	if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) 
	    < 0)
	{
	    perror("Select poll");
	    exit(1);
	}

	sigsetmask(0);

	/* Respond to whatever might be happening */
	
	/* New connection? */
	if (FD_ISSET(control, &input_set))
	    if (new_descriptor(control) < 0)
		perror("New connection");

	/* kick out the freaky folks */
	for (point = descriptor_list; point; point = next_point)
	{
	    next_point = point->next;   
	    if (FD_ISSET(point->descriptor, &exc_set))
	    {
		FD_CLR(point->descriptor, &input_set);
		FD_CLR(point->descriptor, &output_set);
		if ( point->character ){
		    SAVE_CHAR_OBJ(point->character, -20);
		}
		close_socket(point);
	    }
	}

        /* Read input */
        for (point = descriptor_list; point; point = next_point){
            next_point = point->next;
            if (FD_ISSET(point->descriptor, &input_set) && !point->ignore_input)/*AVI*/{
                if (process_input(point) < 0){
                    if ( point->character ){
			SAVE_CHAR_OBJ(point->character, -20);
                    }
                    close_socket(point);
                    continue;
                }
                point->newline = FALSE;
            }    
            else
                point->newline = TRUE;
        }

	/* process_commands; */
	for (point = descriptor_list; point; point = next_to_process){
	    next_to_process = point->next;

	    if (--(point->wait) <= 0
	    && (pcomm = get_from_q( &point->input )) != NULL 
	    && !point->ignore_input) {
                if (	point->character 
			&& (point->character->specials.was_in_room != NOWHERE)
			&& ((point->connected == CON_PLAYING )
				|| (point->connected == CON_HOVERING)
				|| (point->connected == CON_UNDEAD)
				|| (point->connected == CON_SOCIAL_ZONE)
				)
                    ) {
		    if (point->character->in_room != NOWHERE)
			char_from_room(point->character);
		    char_to_room(point->character, 
			point->character->specials.was_in_room);
		    point->character->specials.was_in_room = NOWHERE;
		    act("$n returns from reality.", TRUE, point->character, 0, 0, TO_ROOM);
		    affect_total(point->character);
		}

#ifdef LOGALLINPUT
		if(point->character)
			{
			sprintf(log_buf, 
				"Player: %s  :: pcomm:  %s", GET_NAME(point->character), pcomm);
			log_hd(log_buf);
			}
		else
			{
			log_hd("no character in process output loop");
			}
#endif

		point->wait = 1;
		if ((point->character) && !IS_HOVERING(point->character))
		    point->character->specials.timer = 0;

		if (point->str){
		    if(!point->character)
			point->str=NULL;
		    else
		 	do_editor(point->character,pcomm,point->character->internal_use2);
		}else if (	(point->connected != CON_PLAYING)
				&& (point->connected != CON_HOVERING)
				&& (point->connected != CON_UNDEAD)
				&& (point->connected != CON_SOCIAL_ZONE)
				)
		    nanny(point, pcomm);
		else if (point->showstr_point)
		    show_string(point, pcomm);
		else if(point->character->p->querycommand==19990);/*slots*/
		else if (point->character->p->querycommand)
			{
#ifdef LOGALLINPUT
			sprintf(log_buf,"Q-Player:  %s ::  Argument:  %s", GET_NAME(point->character), pcomm);
			log_hd(log_buf);
#endif
         	(*point->character->p->queryfunc)
              (point->character, pcomm, point->character->p->querycommand);
			}
		else
		    command_interpreter(point->character, pcomm);
		/* Cheesy way to force prompts */
		if((!guy_deleted) )
			{
		    write_to_q( "", &point->output );
			}

		guy_deleted = FALSE;

#ifndef MEDTHIEVIA	// I don't need/want to debug it.
		if((int)pcomm>49000000){
		    sprintf(log_buf,"##PCOMM PREFREE was %d Dec and 0x%x hex",(int)pcomm,(int)pcomm);
		    log_hd(log_buf);
		}else{
#endif
		    pcomm = my_free(pcomm);
#ifndef MEDTHIEVIA
		}
#endif
	    }
	}


	for (point = descriptor_list; point; point = next_point) 
	{
	    next_point = point->next;

	    if (FD_ISSET(point->descriptor, &output_set) && point->output.head && !point->ignore_input)
	    {
		/* give the people some prompts */
		if (point->str){
		    if(!point->character)
			point->str=NULL;
		    else{
			if(point->character->internal_use2==1000)
			    send_to_char(point->character->p->queryprompt2,point->character);
			else
			   if(point->character->internal_use2==0)
			      do_editor(point->character,"",9);
			else
			   if(point->character->internal_use2==1001);
			else
			   if(point->character->internal_use2>1002)
			       write_to_q(point->character->p->queryprompt2, &point->output);
			else
		           write_to_q( "] ", &point->output );
		    }
                }else if (      (point->connected != CON_PLAYING)
                                && (point->connected != CON_HOVERING)
                                && (point->connected != CON_UNDEAD)
				&& (point->connected != CON_SOCIAL_ZONE)
                                ) 
		    ;
		else if(point->character->p->querycommand==19990);/*slots*/
		else if((point->character->p->querycommand)||(point->character->p->queryintcommand))
		    write_to_q(point->character->p->queryprompt, &point->output);
		else if (point->showstr_point)
		    write_to_q( "*** Press return ***", &point->output );
		else
		{
			prompt(point);
		}
		if (process_output(point) < 0)
		{
		    if ( point->character ){
                    	SAVE_CHAR_OBJ(point->character, -20);
		    }
		    close_socket(point);
		}
	    }
	}

	/* See if everyone wants clock FAST. */
	fStall = TRUE;
	for (point = descriptor_list; point; point = next_to_process)
	{
	    if (      (point->connected != CON_PLAYING)
                                && (point->connected != CON_HOVERING)
                                && (point->connected != CON_UNDEAD)
				&& (point->connected != CON_SOCIAL_ZONE)
                                ) 
		continue;
	    if ( point->tick_wait > 0 )
		continue;
	    fStall = TRUE;
	}

	/*
	 * Heartbeat.
	 * All autonomous actions (including fighting and healing)
	 * are subdivisions of the basic pulse at OPT_USEC interval.
	 */
	pulse++;
	/* We must give them breath! cause we are nice */
	if(pulse % 3 == 0){
		for(point=descriptor_list;point;point=point->next){
			if(point->connected!=CON_PLAYING &&
			   point->connected!=CON_UNDEAD  )
				continue;
			if(point->character->specials.stpMount&&!IS_NPC(point->character))
				stpCh=point->character->specials.stpMount;
			else
				stpCh=point->character;
			if(GET_BREATH(stpCh)<40){
				global_color=32;
				send_to_char("You pant loud, breathing hard.\n\r",stpCh);
				act("$n pants loud, breathing hard.",TRUE,stpCh,0,0,TO_ROOM);
				global_color=0;				
			}
			if(GET_BREATH(stpCh)<100){
				if((GET_BREATH(stpCh)+GET_STA(stpCh)/2)<100){
					GET_BREATH(stpCh)+=GET_STA(stpCh)/2;
				}else{
					GET_BREATH(stpCh)=100;
				}
			}
		}
	}

    if(pulse%4 == 0) /* timing for slot machines display*/
	for (point = descriptor_list; point; point = next_point) {
	    next_point = point->next;   
	    if (    (point->connected != CON_PLAYING)
                 && (point->connected != CON_HOVERING)
                 && (point->connected != CON_UNDEAD)
		 && (point->connected != CON_SOCIAL_ZONE)
                 )continue; 
	    if(point->character->p->querycommand==19990){
			roll_slot(point->character);
	    }
	}

	if(pulse % 7 == 0){
		if(world[iFlyStoreRoom]->people)
			MoveFlyingPeople();
	}

	if(pulse % 7 == 0){
		DragonControl();
	}

	if (pulse % 16 == 0){ /*autosave*/
	    for(i=0,point=descriptor_list;point&&i<autosave;point=point->next,i++);
	    if(point){
	        autosave++;
	        if(point->character)
				if (  (point->connected == CON_PLAYING)
                   || (point->connected == CON_HOVERING)
                   || (point->connected == CON_UNDEAD)
		   || (point->connected == CON_SOCIAL_ZONE)
                   ) {
					SAVE_CHAR_OBJ(point->character, -20);
					if(point->character->specials.display_autosave)
		            	send_to_char("[Autosaved]\n\r",point->character);
		    	}
	    }else{
	        autosave=0;
 	    }
	}

	if(pulse%240 == 0){  /*see if zone is empty long enough to free it*/
    	for (i = 0; i <= top_of_zone_table; i++) {
        	if ( zone_table[i].reset_mode == -1 ) /* this zone node not used */
            	    continue;

 			if(zone_table[i].time_to_empty)
		    	zone_table[i].time_to_empty++;

			if(zone_table[i].time_to_empty==11){
		    	free_zone(i);
		    	zone_table[i].time_to_empty=0;
			}
   	    }
	}

    if (pulse % 300 == 0){/*this will make finger.txt for finger@ourmachine*/
    	do_who(finger_ch,"",1219);
    }

	if (pulse % 650 == 0)
		do_trivia();

	if(catacombs_loaded){
	    if(pulse%840==0)
	        stalactite();
            if(pulse%6==0)
	        move_catacomb_moveable();
	}

    if(pulse%2400 == 0){
	    if(obj_index[1399].number<NUM_DRAGON_CRYSTALS){
			for(i=obj_index[1399].number;i<=NUM_DRAGON_CRYSTALS;i++){
		    	load_dragon_crystal();
			}
	    }
	}

	if(pulse%9600==0)
		donation_whine();


	/*Fire room actions if its time*/
 	time(&cur_seconds);
	rt=raction_seconds;
	while(rt&&rt->time<=cur_seconds){
	    fire_raction(ractions[rt->action]->in_room,0,rt->action);
	    raction_seconds=rt->next;
	    rt = my_free(rt);
	    rt=raction_seconds;
	}
	if(cur_seconds>ttNextQuake){
	    StartEarthquake();
	    ttNextQuake=cur_seconds+number(3600,14400);
	}
	if(stpEarthquake&&cur_seconds>=stpEarthquake->time_tNextShake)
	    Shake();
	/*atlantis and deep sea atmosphere text*/
        if(pulse % 500 == 0){
	    switch(number(1,5)){
		case 1:
		    strcpy(log_buf,
"You watch as some small groups of shiny bubbles float up past you.\n\r");
		break;
		case 2:
		    strcpy(log_buf,
"A school of tiny fish meander by as if they are one entity.\n\r");
		break;
		case 3:
		    strcpy(log_buf,
"You hear the magical call of distant whales singing to each other.\n\r");
		break;
		case 4:
		    strcpy(log_buf,
"A lone large fish swims by a few feet away.\n\r");
		break;
		case 5:
		    strcpy(log_buf,
"A soft rumble rolls by..the kind of echoing with no direction to it.\n\r");
		break;
	    }
	    for(point=descriptor_list;point;point=point->next)
		if(point->character&&
		point->character->in_room>0&&world[point->character->in_room]&&
		(world[point->character->in_room]->zone==67||
		world[point->character->in_room]->zone==68))
		    send_to_char(log_buf,point->character);
	}
/*io*/
/* do pulse_void_people */
	if (pulse % PULSE_ZONE == 0)
	    zone_update();

	if (pulse % PULSE_MOBILE == 0)
	    mobile_activity();

#ifndef PACIFIST
	if (pulse % PULSE_VIOLENCE == 0)
	    perform_violence();
#endif

        if (pulse % PULSE_HOVER == 0)
		perform_hovering();

        if (pulse % PULSE_UNDEAD == 0)
		perform_undead();

	if( pulse % 1200 == 0)
		auto_group_all_mobs();

	if( pulse % 260 == 0)
		mob_non_combat_inteligence();

	if (pulse % ((SECS_PER_MUD_HOUR*1000000)/OPT_USEC) == 0)
	{
	    weather_and_time(1);
	    affect_update();
	    point_update();
	    room_affect_update();
	}

	if ( pulse % 22 == 0 )
		damage_room_update();

	if( pulse % 40 == 0 )
		PigeonMessageUpdate();
	/*
	 * Synchronize to an OPT_USEC clock.
	 * Sleep( last_time + OPT_USEC - now ).
	 */
	if ( fStall )
	{
	    gettimeofday( &now_time, NULL );
	    stall_time.tv_usec  = last_time.tv_usec - now_time.tv_usec
			    + OPT_USEC;
	    stall_time.tv_sec   = last_time.tv_sec  - now_time.tv_sec;
	    if ( stall_time.tv_usec < 0 )
	    {
		stall_time.tv_usec += 1000000;
		stall_time.tv_sec--;
	    }
	    if ( stall_time.tv_usec >= 1000000 )
	    {
		stall_time.tv_usec -= 1000000;
		stall_time.tv_sec++;
	    }

	    if ( stall_time.tv_sec > 0
	    || ( stall_time.tv_sec == 0 && stall_time.tv_usec > 0 ) )
	    {
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Select stall" );
		    exit( 0 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
    }
}



char *get_from_q(struct txt_q *queue)
{
struct txt_block *tmp=NULL;
char *dest=NULL;

	if(!queue)
		SUICIDE;
    /* Q empty? */
    if(!queue->head)
		{
		if(queue->tail)
			SUICIDE;
		return (MED_NULL);
    	}

    if(queue->head == queue->tail)
		queue->tail = NULL;
    tmp         = queue->head;
    dest        = tmp->text;
	/*strcpy(dest, tmp->text);*/
    queue->head = tmp->next;
    tmp = my_free( tmp );
    return (dest);
}



void write_to_q(char *txt, struct txt_q *queue)
{
char grep[MAX_STRING_LENGTH+20];
char line[MAX_STRING_LENGTH+20];
struct txt_block *new=NULL;
char *p;

	if(!txt)
		SUICIDE;
	if(!queue)
		SUICIDE;
    if(grep_text[0] && !grep_flag){/*GREP CODE */
	grep_flag=1;
	p=txt;
	while((p=get_next_line(p,line))!=NULL){

		if( line[0]==MED_NULL  )
			SUICIDE;
	    set_to_lower(grep,line);
	    if(!strstr(grep,grep_text)&&!strstr(grep,"return")&&!strstr(grep,"enter"))
           	continue;
	    else    
		write_to_q(line,queue);
	}
	grep_flag=0;
	return;
    }

    CREATE(new, struct txt_block, 1);
    if(!new){
	log_hd("### DIDN'T CREATE NEW IN WRITE_TO_Q COMM.C");
	return;
    }
    CREATE(new->text, char, strlen(txt) + 2);
    if(!new->text){
	log_hd("### DIDN'T CREATE NEW->TEXT IN WRITE_TO_Q COMM.C");
	return;
    }
    strcpy(new->text, txt);

    /* Q empty? */
    if (!queue->head)
    {
		new->next = NULL;
		queue->head = new;
		queue->tail = new;
    }else{
		queue->tail->next = new;
		queue->tail = new;
		new->next = NULL;
    }
}



/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
    char *pbuf;
    while ( ( pbuf = get_from_q( &d->input ) ) != NULL )
	pbuf = my_free( pbuf );
    while ( ( pbuf = get_from_q( &d->output ) ) != NULL )
	pbuf = my_free( pbuf );
	d->output.head = NULL;
	d->output.tail = NULL;
	d->input.head = NULL;
	d->input.tail = NULL;
}



int init_socket(int port)
{
    int s;
    char *opt;
 /*   char *opt=MED_NULL;  
  *    if we try to initialize this var game thinks port 4000 is in use */
    char hostname[MAX_HOSTNAME+1];
    struct sockaddr_in sa;
    struct hostent *hp=NULL;

    gethostname(hostname, MAX_HOSTNAME);
    hp = gethostbyname(hostname);
    if (hp == NULL)
    {
	perror("gethostbyname");
	exit(1);
    }

    sa.sin_family      = hp->h_addrtype;
    sa.sin_port        = htons(port);
    sa.sin_addr.s_addr = 0;
    sa.sin_zero[0]     = 0;
    sa.sin_zero[1]     = 0;
    sa.sin_zero[2]     = 0;
    sa.sin_zero[3]     = 0;
    sa.sin_zero[4]     = 0;
    sa.sin_zero[5]     = 0;
    sa.sin_zero[6]     = 0;
    sa.sin_zero[7]     = 0;

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) 
    {
	perror("Init-socket");
	exit(1);
    }
    if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
	(char *) &opt, sizeof (opt)) < 0) 
    {
	perror ("setsockopt SO_REUSEADDR");
	exit (1);
    }

#ifndef sun
#if defined(SO_DONTLINGER) 
    {
	struct linger ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if (setsockopt(s, SOL_SOCKET, SO_DONTLINGER, &ld, sizeof(ld)) < 0) 
	{
	    perror("setsockopt SO_DONTLINGER");
	    exit( 1 );
	}
    }
#endif
#endif

    if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0)
    {
	perror("bind");
	close(s);
	exit(1);
    }
    listen(s, 3);
    return(s);
}



int new_connection(int s)
{
    struct sockaddr_in isa;
    /* struct sockaddr peer; */
    int i;
    int t;

    i = sizeof(isa);
    getsockname(s, (struct sockaddr *) &isa, &i);

    if ((t = accept(s, (struct sockaddr *) &isa, &i)) < 0)
    {
	perror("Accept");
	return(-1);
    }
    nonblock(t);

	open_descs++ ;
    return(t);
}



int number_playing(void)
{
    struct descriptor_data *d=NULL;

    int i;

    for ( i = 0, d = descriptor_list ; d ; d = d->next )
	i++;

    return(i);
}



int new_descriptor(int s)
{
    int desc;
    struct descriptor_data *newd=NULL;
    int size;
    struct sockaddr_in sock;
    struct hostent *from=NULL;
    struct ban_t *tmp=NULL;

    if ((desc = new_connection(s)) < 0)
	return (-1);

    if (desc > maxdesc)
	maxdesc = desc;

    CREATE(newd, struct descriptor_data, 1);
    /* init desc data */
    newd->descriptor = desc;
    newd->name = MED_NULL;
    *newd->host = '\0';
    newd->pos = 0;
    newd->connected  = 1;
    newd->wait = 1;
    newd->showstr_head = NULL;
    newd->showstr_point = NULL;
    newd->newline = FALSE;
    newd->str = NULL;
    newd->max_str = 0;
    newd->oneline = 0;
    newd->curline = 0;
    *newd->editing = '\0';
    *newd->buf = '\0';
    *newd->last_input= '\0';
    newd->output.head = NULL;
    newd->output.tail = NULL;
    newd->input.head = NULL;
    newd->input.tail = NULL;
    newd->character = NULL;
    newd->original = NULL;
    newd->snoop.snooping = NULL;
    newd->snoop.snoop_by = NULL;
    newd->ignore_input = 0; /*AVI*/
    newd->child_pid = 0; /*AVI*/
    newd->next = NULL;

    /* find info */
    size = sizeof(sock);
    if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0)
    {
	perror("getpeername");
	*newd->host = '\0';
    }
    else{
#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
	sprintf( log_buf, "Sock.sinaddr: %s", inet_ntoa(&sock.sin_addr));
	log_hd( log_buf );
	strcpy(newd->host, inet_ntoa(&sock.sin_addr));
#else
	sprintf( log_buf, "Sock.sinaddr: %s", (char *)inet_ntoa(sock.sin_addr));
	log_hd( log_buf );
	strcpy(newd->host, (char *)inet_ntoa(sock.sin_addr));
#endif
	if(get_host){
	    from=gethostbyaddr((char *)&sock.sin_addr, sizeof(sock.sin_addr),AF_INET);
	    if ( from ){
	        strncpy(newd->host, from->h_name, 49);
	        *(newd->host + 49) = '\0';
	    }
        }
    }

    /*
     * Swiftest: I added the following to ban sites. I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     */
    for ( tmp = ban_list; tmp; tmp = tmp->next ){
	if ( !str_cmp( tmp->name, newd->host ) ){
	    sprintf(log_buf,"\n\r******MEDIEVIA******\n\rYour site has been banned by %s on %s\n\rREASON: %s\n\r\n\r",tmp->god,tmp->date,tmp->reason);
	    write_to_descriptor( desc,log_buf);
	    sprintf(log_buf,"## %s (banned) tried to connect",newd->host);
	    log_hd(log_buf);
	    close( desc );
		open_descs-- ;
	    newd = my_free( newd );
	    return 0;
	}
        if(strstr(log_buf,tmp->name)){
	    close( desc );
	    open_descs-- ;
	    newd = my_free( newd );
	    return 0;
        }
    }

#define MAX_MED_PLAYERS 250

    if(open_descs>MAX_MED_PLAYERS){
	 write_to_descriptor( desc,"
WE ARE CURRENTLY LIMITED TO 250 PLAYERS ON AT ONCE.
THIS PROBLEM WILL BE FIXED SHORTLY.
YOU MAY LOG ON AFTER OTHERS LOG OFF.
TRY BACK IN A FEW MINUTES.
");
/*
	log_hd("## PLAYER DENIED, too many people on");
*/
	close(desc);
	open_descs--;
	newd = my_free( newd );
	return 0;

    }
    

    
    newd->descriptor = desc;
    newd->next = descriptor_list;
    descriptor_list = newd;
	sort_descriptors();

sprintf(log_buf,"\n\r
\r   ...       ... ...... .....    ...... ...... ...   ... ......    ...
\r   ::::     :::: :::::: ::+::::  :::::: :::::: :::   ::: ::::+:   :::::
\r   +:: :: +: ::+ ::+    +::  :+:   ::   +::    :+:   ::+   ::    +:: :+:
\r   +:+  +:+  +:+ +:+    +:+  +:+   +:   +:+    +:+   +:+   +:    +:+ +:+
\r   ++:   +   :++ :++:+  +:+  ++:   ++   ++:++  +:+   +#:   ++    ++: +++
\r   ++#       +++ +++#+  #++  +++   #+   ++#++  ++#   +++   ++   ++++++#++
\r   +#+       #++ +#+    +#+  ++#   +#   +#+    #++   +#+   +#   +#++#++#+
\r   #+#       #+# +##    ##+  +##   ##   #+#     #+# +##    +#   ##+   #+#
\r   ##+       ### ##+### +###+##  ###+## +####+   ##+##   ##+### #+#   ###
\r   ###       ### ###### #####    ###### ######    ###    ###### ###   ###
\r                               :::::: :::   :::
\r                                +::+  :+:   ::+
\r                                 ++   ++:   :+:
\r                                 #+   #++   +#+
\r                                 ##    #+# +##
\r                                +##+    ##+##
\r                               ######    ###");
write_to_q(log_buf,&newd->output);
    write_to_q( "\n\rWelcome to MEDIEVIA!  Leading the way in M.U.D. development.\n\r", &newd->output );
    write_to_q( "Be sure to type HELP MEDIEVIA to see why there is no turning back.\n\r\n\r",&newd->output );
    write_to_q("
 By what name are you known on Medievia? ", &newd->output );

    return(0);
}



int process_output(struct descriptor_data *t)
{
    char buf[2 * MAX_STRING_LENGTH];
    int ibuf = 0;
    char *pstr=NULL;
    int ilen=0;

    if ( t->newline )
    {
	buf[ibuf++] = '\n';
	buf[ibuf++] = '\r';
    }

    /* Cycle thru output queue */
    while ( ( pstr = get_from_q(&t->output) ) != NULL )
    {  
	if(t->snoop.snoop_by)
	{
	    sprintf(log_buf,">>");
	    write_to_q(log_buf,&t->snoop.snoop_by->desc->output);
	    write_to_q(pstr, &t->snoop.snoop_by->desc->output);
	}
	if(t->templog){
		vryce=get_char("Vryce");
		if(vryce&&vryce->desc){
		    sprintf(log_buf,"%s> ",GET_NAME(t->character));
		    write_to_q(log_buf,&vryce->desc->output);
	    	write_to_q(pstr, &vryce->desc->output);
			vryce=NULL;
		}		
	}
	ilen = strlen( pstr );
	if ( ibuf + ilen > sizeof(buf)-10 )
	    ilen = sizeof(buf)-10 - ibuf;
	memcpy( &buf[ibuf], pstr, ilen );
	ibuf += ilen;
	pstr = my_free( pstr );
	if ( ibuf > sizeof(buf)/2 )
	    break;
    }
    
    buf[ibuf] = '\0';
    return write_to_descriptor( t->descriptor, buf );
}


int write_to_descriptor(int desc, char *txt)
{
    int sofar, thisround, total;

    total = strlen(txt);
    sofar = 0;

    do
    {
	thisround = write(desc, txt + sofar, total - sofar);
	if (thisround < 0)
	{
	    perror("Write to socket");
	    return(-1);
	}
	sofar += thisround;
    } 
    while (sofar < total);

    return(0);
}



int process_input(struct descriptor_data *t)
{
    int sofar, thisround, begin, squelch, i, k, flag;
    char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 250];

	if(!t)
		SUICIDE;
    sofar = 0;
    flag = 0;
    begin = strlen(t->buf);

    /* Read in some stuff */
    do
    {
	if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
	    MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0){
	    sofar += thisround;
	      }
	else
	    if (thisround < 0)
		if(errno != EWOULDBLOCK)
		    {
		    perror("Read1 - ERROR");
		    return(-1);
		}
		else
		    break;
	    else
	    {
		log_hd("EOF encountered on socket read.");
		return(-1);
	    }
    }
    while (!ISNEWL(*(t->buf + begin + sofar - 1))); 

    *(t->buf + begin + sofar) = 0;

    /* if no newline is contained in input, return without proc'ing */
    for (i = begin; !ISNEWL(*(t->buf + i)); i++)
	if (!*(t->buf + i))
	    return(0);

    /* input contains 1 or more newlines; process the stuff */
    for (i = 0, k = 0; *(t->buf + i);)
    {
	if (!ISNEWL(*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))))
	    if(*(t->buf + i) == '\b'||*(t->buf+i)==127)    /* backspace */
		if (k)  /* more than one char ? */
		{
		    if (*(tmp + --k) == '$')
			k--;                
		    i++;
		}
		else
		    i++;  /* no or just one char.. Skip backsp */
	    else
		if (isascii(*(t->buf + i)) && isprint(*(t->buf + i)))
		{
		    /* trans char, double for '$' (printf)  */
		    if ((*(tmp + k) = *(t->buf + i)) == '$')
			*(tmp + ++k) = '$';
		    k++;
		    i++;
		}
		else
		    i++;
	else
	{
	    *(tmp + k) = 0;
	    if(*tmp == '!')
		strcpy(tmp,t->last_input);
	    else
		strcpy(t->last_input,tmp);

	    write_to_q(tmp, &t->input);

	    if(t->snoop.snoop_by)
		{
		    write_to_q("% ",&t->snoop.snoop_by->desc->output);
		    write_to_q(tmp,&t->snoop.snoop_by->desc->output);
		    write_to_q("\n\r",&t->snoop.snoop_by->desc->output);
		}
		if(t->templog){
			vryce=get_char("Vryce");
			if(vryce&&vryce->desc){
			    sprintf(log_buf,"%s> ",GET_NAME(t->character));
			    write_to_q(log_buf,&vryce->desc->output);
	    		write_to_q(tmp, &vryce->desc->output);
	    		write_to_q("\n\r", &vryce->desc->output);
				vryce=NULL;
			}		
		}

	    if (flag)
	    {
		sprintf(buffer, 
		    "Line too long. Truncated to:\n\r%s\n\r", tmp);
		if (write_to_descriptor(t->descriptor, buffer) < 0)
		    return(-1);

		/* skip the rest of the line */
		for (; !ISNEWL(*(t->buf + i)); i++);
	    }

	    /* find end of entry */
	    for (; ISNEWL(*(t->buf + i)); i++);

	    /* squelch the entry from the buffer */
	    for (squelch = 0;; squelch++)
		if ((*(t->buf + squelch) = 
		    *(t->buf + i + squelch)) == '\0')
		    break;
	    k = 0;
	    i = 0;
	}
    }
    return(1);
}



void close_socket(struct descriptor_data *d)
{
    struct descriptor_data *tmp=NULL;

    process_output( d );
    close( d->descriptor );
	open_descs-- ;
    if ( d->descriptor == maxdesc )
	--maxdesc;

    /* Forget snooping */
    if (d->snoop.snooping)
	d->snoop.snooping->desc->snoop.snoop_by = NULL;

    if (d->snoop.snoop_by)
    {
	send_to_char(
	    "Your victim is no longer among us.
	    \n\r",d->snoop.snoop_by );
	d->snoop.snoop_by->desc->snoop.snooping = NULL;
    }

    if (d->character)
    {
	if (( d->connected == CON_PLAYING )
		|| (d->connected == CON_HOVERING)
                || (d->connected == CON_UNDEAD)
		|| (d->connected == CON_SOCIAL_ZONE)
                ) 
		{
	    act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
	    sprintf(log_buf, "LOSTLINK:Closing link to: %s.",GET_NAME(d->character));
	    log_hd( log_buf );
	    if(strcmp(GET_NAME(d->character),"Starblade"))
	       do_wiz(d->character,log_buf,5);
	    d->character->desc = NULL;
		if ((d->connected == CON_UNDEAD) && (d->original))
			free_char(d->original);
		}
	else
		{
	    sprintf(log_buf, "Losing player: %s.", GET_NAME(d->character) );
	    log_hd( log_buf );
	    free_char( d->character );
		}
    }
    else
    {
	log_hd( "Losing descriptor without char." );
    }

    if (next_to_process == d)
	next_to_process = next_to_process->next;   

    if ( d == descriptor_list )
	descriptor_list = descriptor_list->next;
    else
    {
	/* Locate the previous element */
	for (tmp = descriptor_list; (tmp->next != d) && tmp; tmp = tmp->next)
	    ;
	tmp->next = d->next;
    }
	if(d->name)
		d->name = my_free(d->name);
/*
       if ( d->showstr_head )
		d->showstr_head = my_free(d->showstr_head);
	if(d->showstr_point)
		d->showstr_point = my_free(d->showstr_point);
	if(d->input.head)
		d->input.head = my_free(d->input.head);
	if(d->output.head)
		d->output.head = my_free(d->output.head);
	if(d->output.tail&&d->output.tail!=d->output.head)
		d->output.tail = my_free(d->output.tail);
	if(d->input.tail&&d->input.tail!=d->input.head)
		d->input.tail = my_free(d->input.tail);
*/
    d = my_free(d);
	guy_deleted = TRUE;
}

void sort_descriptor(struct descriptor_data *d)
{
struct descriptor_data *tmp=NULL;

    if(!d->character){
		if(!sort_d){
			sort_d=d;
			d->next=NULL;
			return;
		}
    	tmp=sort_d;
    	while(tmp->next)tmp=tmp->next;
    	tmp->next=d;
    	d->next=NULL;
    	return;
	}
    if(!sort_d){
		sort_d=d;
		d->next=NULL;
    }else if(!sort_d->character){ 
    	d->next=sort_d;
    	sort_d=d;	
    }else if(get_total_level(d->character)>=get_total_level(sort_d->character)){
		d->next=sort_d;
		sort_d=d;
    }else{
		for (tmp = sort_d; tmp; tmp = tmp->next){
	    	if(!tmp->next){
				d->next=NULL;
				tmp->next=d;
				return;
	    	}
			if(!tmp->next->character){
				d->next=tmp->next;
				tmp->next=d;
				return;
			}
	    	if(get_total_level(d->character)>=get_total_level(tmp->next->character)){
				d->next=tmp->next;
				tmp->next=d;
				return;
	    	}
        }
    }
}

void sort_descriptors(void)
{
struct descriptor_data *d=NULL,*e=NULL;

	sort_d=NULL;
	d=descriptor_list;
	while(d){
		e=d;
	    d=d->next;
	    sort_descriptor(e);
	}
	descriptor_list=sort_d;
}


/* AVI */
void nonblock(int s)
{
/*	int oldflags = 0;
	oldflags = fcntl(s, F_GETFL, 0);

	if (fcntl(s, F_SETFL, oldflags || FNDELAY) ==-1)*/
#ifndef sun
	if (fcntl(s, F_SETFL, FNDELAY) == -1)  
#else
	if (fcntl(s, F_SETFL, O_NONBLOCK) == -1)  
#endif
	{
		perror("Noblock");
		exit(1);
	}

}

/* AVI */
void block(int s)
{
	int	oldflags = 0;

	oldflags = fcntl(s, F_GETFL, 0);
	if (oldflags < 0) return;

#ifndef sun
	if (fcntl(s, F_SETFL, oldflags & ~(FNDELAY)) == -1)
#else
	if (fcntl(s, F_SETFL, oldflags & ~(O_NONBLOCK)) == -1)
#endif
	{
		perror("Block");
		exit(1);
	}
}

void send_to_char(char *messg, struct char_data *ch)
{
char buf[MAX_STRING_LENGTH+20];

    if (ch->desc && messg){

      if(global_color&&ch->specials.ansi_color==69){
        sprintf(buf,"\033[1m\033[%dm%s\033[0m",global_color,messg);
	if(!ch->specials.setup_page)
   	     write_to_q(buf, &ch->desc->output);
	else
	     strcat(page_setup,buf);
      }else{	
	  if(!ch->specials.setup_page)
  	       write_to_q(messg, &ch->desc->output);
	  else
	       strcat(page_setup,messg);
      }
    }
}



void send_to_all(char *messg)
{
    struct descriptor_data *i=NULL;
char buf[MAX_STRING_LENGTH+20];
        
    if (messg)
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected)
	      if(global_color&&i->character->specials.ansi_color==69){
        	sprintf(buf,"\033[1m\033[%dm%s\033[0m",global_color,messg);
		write_to_q(buf, &i->output);
	      }else
		write_to_q(messg, &i->output);
}


void send_to_outdoor(char *messg)
{
    struct descriptor_data *i=NULL;
char buf[MAX_STRING_LENGTH+20];

    if (messg)
	for (i = descriptor_list; i; i = i->next)
	    if ((!i->connected)
		&&!IS_SET((i->character)->specials.plr_flags, PLR_WRITING))
		if (OUTSIDE(i->character))
		      if(global_color&&i->character->specials.ansi_color==69){
        		sprintf(buf,"\033[1m\033[%dm%s\033[0m",global_color,messg);
			write_to_q(buf, &i->output);
		      }else
		    write_to_q(messg, &i->output);
}


void send_to_room(char *messg, int room)
{
    struct char_data *i=NULL;
char buf[MAX_STRING_LENGTH+20];

    if (messg)
	for (i = world[room]->people; i; i = i->next_in_room)
	    if (i->desc)
		      if(global_color&&i->specials.ansi_color==69){
        		sprintf(buf,"\033[1m\033[%dm%s\033[0m",global_color,messg);
			write_to_q(buf, &i->desc->output);
		      }else
		write_to_q(messg, &i->desc->output);
}



void act(char *str, int hide_invisible, struct char_data *ch,
    struct obj_data *obj, void *vict_obj, int type)
{
    register char *strp, *point, *i = NULL;
    struct char_data *to=NULL;
    char buf[MAX_STRING_LENGTH];
    char bufc[MAX_STRING_LENGTH+20];
		
    if (!str)
	return;
    if (!*str)
	return;

    if(!world[ch->in_room])return;
    if (type == TO_VICT)
	to = (struct char_data *) vict_obj;
    else if (type == TO_CHAR)
	to = ch;
    else
	to = world[ch->in_room]->people;

    for (; to; to = to->next_in_room)
    {
        if(!vict_obj)(struct char_data *) vict_obj=ch;
	if (to->desc 
	    && (!IS_SET(ORIGINAL(to)->specials.plr_flags, PLR_WRITING))
	    && ((to != ch) 
		|| (type == TO_CHAR)) 
	    && (CAN_SEE(to, ch) 
		|| !hide_invisible 
		|| (type == TO_VICT)) 
	    && (IS_HOVERING(to) 
	    	|| AWAKE(to)) 
	    && !((type == TO_NOTVICT) 
		&& (to == (struct char_data *) vict_obj)))
	{
	    for (strp = str, point = buf;;)
		if (*strp == '$')
		{
		    switch (*(++strp))
		    {
			case 'n': i = PERS(ch, to); break;
			case 'N': i = PERS((struct char_data *) vict_obj, to);
					break;
			case 'm': i = HMHR(ch); break;
			case 'M': i = HMHR((struct char_data *) vict_obj);
					break;
			case 's': i = HSHR(ch); break;
			case 'S': i = HSHR((struct char_data *) vict_obj);
					break;
			case 'e': i = HSSH(ch); break;
			case 'E': i = HSSH((struct char_data *) vict_obj);
					break;
			case 'o': i = OBJN(obj, to); break;
			case 'O': i = OBJN((struct obj_data *) vict_obj, to);
					break;
			case 'p': i = OBJS(obj, to); break;
			case 'P': i = OBJS((struct obj_data *) vict_obj, to);
					break;
			case 'a': i = SANA(obj); break;
			case 'A': i = SANA((struct obj_data *) vict_obj);
					break;
			case 'T': i = (char *) vict_obj; break;
			case 'F': i = fname((char *) vict_obj); break;
			case '$': i = "$"; break;
			default:
			    log_hd("Illegal $-code to act():");
			    log_hd(str);
			    break;
		    }
		    while ( ( *point = *(i++) ) != '\0' )
			++point;
		    ++strp;
		}
		else if (!(*(point++) = *(strp++)))
		    break;

	    *(--point) = '\n';
	    *(++point) = '\r';
	    *(++point) = '\0';

		  if(ACTTO){
			  if(ACTTO==to)return;
		      if(global_color&&ACTTO->specials.ansi_color==69){
    	             sprintf(bufc,"\033[1m\033[%dm%s\033[0m",global_color,CAP(buf));
        	         write_to_q(bufc, &ACTTO->desc->output);
               }else{
	          		write_to_q(CAP(buf), &ACTTO->desc->output);		  
			   }
			   return;
	       }
	      if(global_color&&to->specials.ansi_color==69){
                 sprintf(bufc,"\033[1m\033[%dm%s\033[0m",global_color,CAP(buf));
                 write_to_q(bufc, &to->desc->output);
               }
	       else
	          write_to_q(CAP(buf), &to->desc->output);
	}
	if ((type == TO_VICT) || (type == TO_CHAR))
	    return;
    }
}



void night_watchman(void)
{
    long tc;
    struct tm *t_info=NULL;

    extern int medievia_shutdown;

    void send_to_all(char *messg);

    tc = time(0);
    t_info = localtime(&tc);

    if ((t_info->tm_hour == 8) && (t_info->tm_wday > 0) &&
	(t_info->tm_wday < 6))
    {
	if (t_info->tm_min > 50)
	{
	    log_hd("Leaving the scene for the serious folks.");
	    send_to_all("Closing down. Thank you for flying Medievia.\n\r");
	    medievia_shutdown = 1;
	}
	else if (t_info->tm_min > 40)
	    send_to_all(
		"ATTENTION: Medievia Cyberspace will shut down in 10 minutes.\n\r");
	else if (t_info->tm_min > 30)
	    send_to_all("Warning: The game will close in 20 minutes.\n\r");
    }
}
void do_trivia(void){
	int trivia_index=0,x,yikes=0;
	struct descriptor_data *mydesc;
	char this_item[MAX_INPUT_LENGTH];

	while(1){ /* make sure this hint has not been shown within last 20 times */
		yikes++;
		trivia_index=number(0,top_trivia-1);
		for(x=0;x<20;x++)
			if(trivia_index==iaShownHints[x])
				break;		
		if(yikes>1000)return;
		if(x<20)continue;
		iaShownHints[iShownHintsP]=trivia_index;
		iShownHintsP++;
		if(iShownHintsP==20)
			iShownHintsP=0;
		break;
	}
	if(trivia_index<0||trivia_index>top_trivia)return;
	sprintf(this_item,"%s\n\r",trivia[trivia_index]);
	for(mydesc=descriptor_list;mydesc;mydesc=mydesc->next){
		if((mydesc->character)&&(STATE(mydesc)==CON_PLAYING))
		if(!IS_SET(ORIGINAL(mydesc->character)->specials.new_comm_flags,PLR_NOTRIVIA)){
			global_color=34;
			send_to_char(this_item,mydesc->character);
			global_color=0;
		}
	}
}
	
