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

#include <signal.h>

void *my_free(void *ptr);

extern int open_files;
extern int open_descs;

extern int IS_PLAYER(struct char_data *ch, char *player);
extern struct weather_data weather_info;
extern char log_buf[MAX_STRING_LENGTH];
extern char page_setup[100000];

extern int kill(pid_t pid, int sig);
#ifndef sun
extern getpid();
#endif

#define TRUE  1
#define FALSE 0

#define GARBAGE 0xcc

#define MED_NULL 0

#define REMOVE_FROM_LIST(item, head, next)      \
   if ((item) == (head))                \
      head = (item)->next;              \
   else {                               \
      temp = head;                      \
      while (temp && (temp->next != (item))) \
         temp = temp->next;             \
      if (temp)                         \
         temp->next = (item)->next;     \
   }                                    \

#define IS_SOCIAL(ch) (( ((ch)->desc) \
                        &&  ((ch)->desc->connected == CON_SOCIAL_ZONE )  \
                    ))

#define SUICIDE (kill(getpid(),SIGQUIT))

#define GET_CONTINENT(ch) (zone_table[world[(ch)->in_room]->zone].continent)

#define ORIGINAL(ch) (IS_SWITCHED((ch)) ? (ch)->desc->original : (ch) )

#define IS_SWITCHED(ch) ( ((ch)->desc) \
			&&  ((ch)->desc->original)  \
		    )

#define IS_HOVERING(ch) (( ((ch)->desc) \
			&&  ((ch)->desc->connected == CON_HOVERING )  \
		    ))

#define IS_UNDEAD(ch) ( ((ch)->desc) \
			&&  ((ch)->desc->connected == CON_UNDEAD  )  \
		    )

#define IS_DEAD(ch) ( ((ch)->desc) \
			&& (    ((ch)->desc->connected == CON_UNDEAD  )  \
                             || ((ch)->desc->connected == CON_HOVERING)  \
                           ) \
		    )

#define SAVE_CHAR_OBJ(ch,cost) \
        if ( IS_UNDEAD(ch) ) \
                { \
		(ch)->desc->original->in_room = (ch)->in_room; \
                (ch)->desc->original->specials.cost_to_rent=(cost); \
                save_char_obj((ch)->desc->original); \
                (ch)->desc->original->specials.cost_to_rent=0; \
                } \
        else \
                { \
                (ch)->specials.cost_to_rent=(cost); \
                save_char_obj((ch)); \
                (ch)->specials.cost_to_rent=0; \
                }

#define MIN(a,b)	((a) < (b) ? (a) : (b))
#define MAX(a,b)	((a) > (b) ? (a) : (b))

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 

#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define CREATE(result, type, number)  do {\
    if (!((result) = (type *) calloc ((number), sizeof(type))))\
	{ perror("malloc failure"); abort(); } } while(0)

#define FREE(p) do { if ( (p) != NULL ) free((p)); } while (0) 

#define IS_SET(flag,bit)  ((flag) & (bit))

#define IS_FLYING(ch) (IS_SET((ch)->player.siMoreFlags,FLY))

#define IN_HOLO(ch) (world[(ch)->in_room]->zone==197 ? TRUE : FALSE)

#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->specials.affected_by, (skill)) )

#define IS_DARK(room)  (!world[room]->light && \
	    (IS_SET(world[room]->room_flags, DARK) || \
	     ( ( world[room]->sector_type != SECT_INSIDE && \
		world[room]->sector_type != SECT_CITY ) && \
	      (weather_info.sunlight == SUN_SET || \
	       weather_info.sunlight == SUN_DARK))))

#define IS_LIGHT(room)  \
	(world[room]->light || !IS_SET(world[room]->room_flags, DARK))

#define ROOM_FLAGGED(loc, flag) ( IS_SET(world[(loc)]->room_flags,(flag)) )
 
#define SET_BIT(var,bit)  ((var) = (var) | (bit))

#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
	"Superb" )))))))

#define HSHR(ch) ((ch)->player.sex ?                    \
    (((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) ((ch)->player.sex ?                    \
    (((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) ((ch)->player.sex ?                    \
    (((ch)->player.sex == 1) ? "him" : "her") : "it")   

#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define GET_BREATH(ch) ((ch)->specials.cBreath)

#define IS_NPC(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC))

#define IS_MOB(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->nr >-1))

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_MOB_POS(ch)     ((ch)->specials.mob_position)

#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

#define GET_NAME(ch)    ((ch)->player.name)

#define GET_TITLE(ch)   ((ch)->player.title)

#define GET_LEVEL(ch)   ((ch)->player.level)

#define GET_CLASS(ch)   ((ch)->player.class)

#define GET_HOME(ch)    ((ch)->player.hometown)

#define GET_STA(ch)     ((ch)->tmpabilities.sta)

#define GET_AGE(ch)     (age(ch).year)

#define GET_STR(ch)     ((ch)->tmpabilities.str)


#define GET_DEX(ch)     ((ch)->tmpabilities.dex)

#define GET_INT(ch)     ((ch)->tmpabilities.intel)

#define GET_WIS(ch)     ((ch)->tmpabilities.wis)

#define GET_CON(ch)     ((ch)->tmpabilities.con)

#define STRENGTH_APPLY_INDEX(ch) \
			(GET_STR(ch))

#define GET_AC(ch)      ((ch)->points.armor)

#define GET_HIT(ch)     ((ch)->points.hit)

#define GET_MAX_HIT(ch) (hit_limit(ch))

#define GET_MOVE(ch)    ((ch)->points.move)

#define GET_MAX_MOVE(ch) (move_limit(ch))

#define GET_MANA(ch)    ((ch)->points.mana)

#define GET_MAX_MANA(ch) (mana_limit(ch))

#define GET_GOLD(ch)    ((ch)->points.gold)

#define GET_EXP(ch)     ((ch)->points.exp)

#define GET_HEIGHT(ch)  ((ch)->player.height)

#define GET_WEIGHT(ch)  ((ch)->player.weight)

#define GET_SEX(ch)     ((ch)->player.sex)

#define GET_HITROLL(ch) ((ch)->points.hitroll)

#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define GET_ZONE(ch) (world[(ch)->in_room]->zone)

#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING)

#define WAIT_STATE(ch, cycle)  (((ch)->desc) ? (ch)->desc->wait = (cycle) : 0)

#define GET_MOB_WAIT(ch) ((ch)->specials.wait)



/* Object And Carry related macros */


#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)

#define CAN_CARRY_N(ch) (5+GET_DEX(ch)/2+GET_LEVEL(ch)/2)

#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)

#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    CAN_SEE_OBJ((ch),(obj)))

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))



/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)   (                                          \
    CAN_SEE(vict, ch) ?                                             \
      (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) :    \
      "someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
    (obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
    fname((obj)->name) : "something")

#define OUTSIDE(ch) (!IS_SET(world[(ch)->in_room]->room_flags,INDOORS))

#define EXIT(ch, door)  (world[(ch)->in_room]->dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) \
			  && (EXIT(ch,door)->to_room != NOWHERE) \
			  && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))
#define IS_THIEF(ch)   (IS_SET((ch)->specials.act, PLR_ISTHIEF));
#define IS_KILLER(ch)  (IS_SET((ch)->specials.act, PLR_ISKILLER));

#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
void    perror      (char *s);
int fread       (void *ptr, int size, int nitems, FILE *stream);
int fwrite      (void *ptr, int size, int nitems, FILE *stream);
void    fclose      (FILE *stream);
int fseek       (FILE *stream, long offset, int ptrname);
int fputs       (char *s, FILE *stream);
int fscanf      (FILE *stream, char *format, ...);
int sscanf      (char *s, char *format, ...);
int fprintf     (FILE *stream, char *format, ...);
int rewind      (FILE *stream);
long    random      (void);
void    srandom     (int seed);
int printf      (char *format, ...);
int sigsetmask  (int mask);
int gethostname (char *name, int namelen);
time_t	time		(time_t *tloc);
#endif

FILE *med_open(char *filename, char *type);
void med_close(FILE *stream);

int	abs		(int i);
int	number		(int from, int to);
int 	dice		(int number, int size);
int 	charinstring	(char *s1, char c);
#if !defined(__FreeBSD__) && !defined(linux) && !defined(sun)
int 	strncasecmp	(char *s1, char *s2, int count);
#endif
int	str_cmp		(char *arg1, char *arg2);
char *	str_dup		(const char *str);
void    log_hd		(char * str);
void    sprintbit	(long vektor, char *names[], char *result);
void    sprinttype	(int type, char *names[], char *result);
struct time_info_data
	mud_time_passed	(time_t t2, time_t t1);
struct time_info_data
	age		(CHAR_DATA *ch);
int	is_number	(char *str);
void	gain_condition	(CHAR_DATA *ch, int condition, int value);
void	set_fighting	(CHAR_DATA *ch, struct char_data *vict);
void	stop_fighting	(CHAR_DATA *ch);
int	do_simple_move	(CHAR_DATA *ch, int cmd, int following);
bool	damage		(CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				int attacktype);
void	hit		(CHAR_DATA *ch, struct char_data *victim,
			    int type);
int	move_limit	(CHAR_DATA *ch);
int	mana_limit	(CHAR_DATA *ch);
int	hit_limit	(CHAR_DATA *ch);
int	guild		(CHAR_DATA *ch, int cmd, char *arg);
void	gain_exp_regardless	(CHAR_DATA *ch, int gain);
void	advance_level	(CHAR_DATA *ch);
void	close_socket	(struct descriptor_data *d);
char *	one_argument	(char *argument, char *first_arg);
char *	one_arg		(char *argument, char *first_arg);
int	isname		(char *arg, char *arg2);
void	page_string	(struct descriptor_data *d, char *str,
			    int keep_internal);
void	gain_exp	(CHAR_DATA *ch, int gain, CHAR_DATA *victim);
void	set_title	(CHAR_DATA *ch);
int get_total_level(struct char_data *ch);
void assign_rooms(void);
void assign_objects(void);
void assign_mobiles(void);
int search_block( char *arg, char **list, bool exact );
void free_obj(struct obj_data *obj);
char *get_next_line(char *in, char *out);
void set_to_upper(char *out, char *in);
void set_to_lower(char *out, char *in);
void char_to_room(CHAR_DATA *ch, int room);
void do_start(CHAR_DATA *ch);
#ifndef __FreeBSD__
char *crypt(char *key, char *salt);
#endif
void char_from_room(CHAR_DATA *ch);
void obj_from_obj(struct obj_data *obj);
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void obj_to_room(struct obj_data *object, int room);
void update_pos( CHAR_DATA *victim );
void clear_object(struct obj_data *obj);
void death_cry( CHAR_DATA *ch );
void obj_from_room(struct obj_data *object);
void obj_to_char(struct obj_data *object, CHAR_DATA *ch);
void cast_lightning_bolt( byte level, CHAR_DATA *ch, char *arg,
     int type, CHAR_DATA *victim, struct obj_data *tar_obj );
void send_to_outdoor( char *messg );
void weather_and_time(int mode);
void night_watchman( void );
int special(CHAR_DATA *ch, int cmd, char *arg);
int process_output(struct descriptor_data *t);
int file_to_string(char *name, char *buf);
bool load_char_obj( struct descriptor_data *d, char *name );
void save_char_obj( CHAR_DATA *ch );
void char_to_store(CHAR_DATA *ch, struct char_file_u *st);
#ifndef OPENONLY
bool obj_to_store( struct obj_data *obj, CHAR_DATA *ch, FILE *fpsave );
#endif
#ifdef OPENONLY
bool obj_to_store( struct obj_data *, CHAR_DATA *, int );
#endif
void check_idling(CHAR_DATA *ch);
void affect_to_char( CHAR_DATA *ch, struct affected_type *af );
bool CAN_SEE( CHAR_DATA *sub, struct char_data *obj );
bool CAN_SEE_OBJ( CHAR_DATA *sub, struct obj_data *obj );
bool check_blind( CHAR_DATA *ch );
void raw_kill(CHAR_DATA *ch, CHAR_DATA *attacker);
void check_killer( CHAR_DATA *ch, struct char_data *victim );
int map_eq_level( CHAR_DATA *mob );
void disarm( CHAR_DATA *ch, struct char_data *victim );
int shop_keeper( CHAR_DATA *ch, int cmd, char *arg );
void send_to_all(char *messg);
void send_to_char(char *messg, CHAR_DATA *ch);
void send_to_room(char *messg, int room);
void act(char *str, int hide_invisible, CHAR_DATA *ch,struct obj_data *obj, void *vict_obj, int type);
int write_to_descriptor(int desc, char *txt);
void write_to_q(char *txt, struct txt_q *queue);
int use_mana( CHAR_DATA *ch, int sn );
bool one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int type);


/*  These will be used to determine usability of a weapon by a player */

#define NOT_MAGE    1
#define NOT_CLERIC  2
#define NOT_THIEF   4
#define NOT_WARRIOR 8
#define TWO_HANDED  16
#define DAGGER		32

#define COL_NRM "\033[1m\033[0m"
#define COL_RED "\033[1m\033[31m"
#define COL_GRN "\033[1m\033[32m"
#define COL_YEL "\033[1m\033[33m"
#define COL_BLU "\033[1m\033[34m"
#define COL_MAG "\033[1m\033[35m"
#define COL_CYN "\033[1m\033[36m"
#define COL_WHT "\033[1m\033[37m"
#define COL_NUL ""

#define _coloron(ch) ((ch)->specials.ansi_color==69 ? TRUE : FALSE)
#define RED(ch) (_coloron((ch))?COL_RED:COL_NUL)
#define GRN(ch) (_coloron((ch))?COL_GRN:COL_NUL)
#define YEL(ch) (_coloron((ch))?COL_YEL:COL_NUL)
#define BLU(ch) (_coloron((ch))?COL_BLU:COL_NUL)
#define MAG(ch) (_coloron((ch))?COL_MAG:COL_NUL)
#define CYN(ch) (_coloron((ch))?COL_CYN:COL_NUL)
#define WHT(ch) (_coloron((ch))?COL_WHT:COL_NUL)
#define NRM(ch) (_coloron((ch))?COL_NRM:COL_NUL)
