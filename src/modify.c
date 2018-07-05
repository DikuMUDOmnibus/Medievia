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
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "interp.h"
#include "handler.h"
#include "db.h"

#define REBOOT_AT    10  /* 0-23, time of optional reboot if -e lib/reboot */


#define TP_MOB    0
#define TP_OBJ     1
#define TP_ERROR  2

extern FILE *help_fl;
extern struct help_index_element *help_index;
extern int top_of_helpt;
extern struct char_data *mobs[MAX_MOB];
extern struct obj_data *objs[MAX_OBJ];
void show_string(struct descriptor_data *d, char *input);
extern char free_error[100];
extern char menu[];
extern char global_color;
extern int board(struct char_data *ch, int cmd, char *arg);
extern char grep_text[250];
extern int top_trivia;
extern char *trivia[];


char *string_fields[] =
{
    "name",
    "short",
    "long",
    "description",
    "title",
    "delete-description",
    "\n"
};




/* maximum length for text field x+1 */
int length[] =
{
    15,
    60,
    256,
    240,
    60
};




char *skill_fields[] = 
{
    "learned",
    "recognize",
    "\n"
};



/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data *d, char *str)
{
    char *scan;
    int terminator = 0;

    /* determine if this is the terminal string, and truncate if so */
    for (scan = str; *scan; scan++)
       if ( ( terminator = (*scan == '@') != 0 ) )
       {
	    *scan = '\0';
	    break;
       }
    
    if (!(*d->str))
    {
	if (strlen(str) > d->max_str)
	{
	    send_to_char("String too long - Truncated.\n\r",
	       d->character);
	    *(str + d->max_str) = '\0';
	    terminator = 1;
	}
	CREATE(*d->str, char, strlen(str) + 3);
	strcpy(*d->str, str);
    }
    else
    {
	if (strlen(str) + strlen(*d->str) > d->max_str)
	{
	    send_to_char("String too long. Last line skipped.\n\r",
	       d->character);
	    terminator = 1;
	}
	else 
	{
	    if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str) + 
	    strlen(str) + 3)))
	    {
		perror("string_add");
		exit(1);
	    }
	    strcat(*d->str, str);
	}
    }

    if (terminator)
    {
	if(d->max_str==2123)
	    board(d->character, 1000, ""); /* was editing board, this will force a save board */	
	d->str = 0;
	d->max_str=0;
	act("$n finishes writing.",TRUE,d->character,0,0,TO_ROOM);
	if (d->connected == CON_EXDSCR)
	{
	    write_to_q( menu, &d->output );
	    d->connected = CON_SELECT_MENU;
	}
    }
    else
       strcat(*d->str, "\n\r");
}


#undef MAX_STR

/* interpret an argument for do_string */
void quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
    char buf[MAX_STRING_LENGTH];

    /* determine type */
    arg = one_argument(arg, buf);
    if (is_abbrev(buf, "char"))
       *type = TP_MOB;
    else if (is_abbrev(buf, "obj"))
       *type = TP_OBJ;
    else
    {
	*type = TP_ERROR;
	return;
    }

    /* find name */
    arg = one_argument(arg, name);

    /* field name and number */
    arg = one_argument(arg, buf);
    if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))
       return;

    /* string */
    for (; isspace(*arg); arg++);
    for (; ( *string = *arg ) != '\0' ; arg++, string++)
	;

    return;
}
    
     
/* Edit your description */
void do_description(struct char_data *ch, char *arg, int cmd) 
{
	    if(IS_NPC(ch))
		return;
	    send_to_char("Enter a description you would like others to see when they look at you.\r\n",ch);
            ch->desc->str      = &ch->player.description;
            ch->desc->max_str = 240;
            strcpy(ch->desc->editing,"Player description");
	    return;
}

/* modification of malloc'ed strings in chars/objects */
void do_string(struct char_data *ch, char *arg, int cmd)
{
    char name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
    int field, type;
    struct char_data *mob=NULL;
    struct obj_data *obj=NULL;
    struct extra_descr_data *ed=NULL, *tmp=NULL;

    if (IS_NPC(ch))
       return;

    quad_arg(arg, &type, name, &field, string);

    if (type == TP_ERROR)
    {
	send_to_char(
	 "Syntax:\n\rstring ('obj'|'char') <name> <field> [<string>].\n\r",
	 ch);
	return;
    }

    if (!field)
    {
	send_to_char("No field by that name. Try 'help string'.\n\r",
	   ch);
	return;
    }

    if (type == TP_MOB)
    {
	/* locate the beast */
	if (!(mob = get_char_vis(ch, name)))
	{
	    send_to_char("I don't know anyone by that name...\n\r",
	       ch);
	    return;
	}

	if(IS_PLAYER(mob,"Vryce")&&!IS_PLAYER(ch,"Vryce")){
	    send_to_char("Vryce would not like that!\n\r",ch);
	    do_tell(ch,"vryce I JUST TRIED TO STRING YOU!",9);
	    return;
	}
	if(IS_PLAYER(mob,"Io")){
	    send_to_char("Io would not like that!\n\r",ch);
	    do_tell(ch,"io I JUST TRIED TO STRING YOU!",9);
	    return;
	}
	switch(field)
	{
	    case 1:
		if (!IS_NPC(mob) && GET_LEVEL(ch) <= 34)
		{
		    send_to_char("You can't change that field for players.", ch);
		    return;
		}
		if(!strcmp("Vryce",string))return;
		if(!strcmp("Io",string))return;
		if(!strcmp("Ugadal",string))return;
		if(!strcmp("Firm",string))return;
		if(!strcmp("Sultress",string))return;
		if(IS_NPC(mob))
		    if(mobs[mob->nr]->player.name==mob->player.name)
			mob->player.name=str_dup(mobs[mob->nr]->player.name);
		ch->desc->str = &GET_NAME(mob);
		ch->desc->max_str=100;
		strcpy(ch->desc->editing,"Mob Name");
		ch->desc->oneline=TRUE;
		if (!IS_NPC(mob))
		    send_to_char(
		    "WARNING: You have changed the name of a player.\n\r",
		    ch);
	    break;
	    case 2:
	       if (!IS_NPC(mob))
	       {
		    send_to_char(
		   "That field is for monsters only.\n\r", ch);
		return;
	       }
		if(IS_NPC(mob))
		    if(mobs[mob->nr]->player.short_descr==mob->player.short_descr)
			mob->player.short_descr=str_dup(mobs[mob->nr]->player.short_descr);
	       ch->desc->str = &mob->player.short_descr;
	       ch->desc->max_str=1000;
	       strcpy(ch->desc->editing,"Mob short description");
	       ch->desc->oneline=TRUE;
	    break;
	    case 3:
	       if (!IS_NPC(mob))
	       {
		send_to_char(
		   "That field is for monsters only.\n\r", ch);
		return;
	       }
		if(IS_NPC(mob))
		    if(mobs[mob->nr]->player.long_descr==mob->player.long_descr)
			mob->player.long_descr=str_dup(mobs[mob->nr]->player.long_descr);
	       ch->desc->str = &mob->player.long_descr;
	       ch->desc->max_str=1000;
	       strcpy(ch->desc->editing,"Mob long description");
	    break;
	    case 4:
		if(IS_NPC(mob))
		    if(mobs[mob->nr]->player.description==mob->player.description)
			mob->player.description=str_dup(mobs[mob->nr]->player.description);
	       ch->desc->str = &mob->player.description; 
	       ch->desc->max_str=1000;
	       strcpy(ch->desc->editing,"Mob description");
	       break;
	    case 5:
	       if (IS_NPC(mob))
	       {
		send_to_char("Monsters have no titles.\n\r",
		   ch);
		return;
	       }
	       ch->desc->str = &mob->player.title;
	       ch->desc->max_str=250;
	       strcpy(ch->desc->editing,"Title");
	    break;
	    default:
	       send_to_char(
		  "That field is undefined for monsters.\n\r", ch);
	       return;
	    break;
	}
    }
    else    /* type == TP_OBJ */
    {
	/* locate the object */
	if (!(obj = get_obj_vis(ch, name)))
	{
	    send_to_char("Can't find such a thing here..\n\r", ch);
	    return;
	}

	switch(field)
	{
	    case 1: 
	        if(objs[obj->item_number]->name==obj->name)
		    obj->name=str_dup(objs[obj->item_number]->name);
		ch->desc->str = &obj->name; 
		ch->desc->max_str=80;
		strcpy(ch->desc->editing,"Object name");
		ch->desc->oneline=TRUE;
		break;
	    case 2: 
	        if(objs[obj->item_number]->short_description==obj->short_description)
		    obj->short_description=str_dup(objs[obj->item_number]->short_description);
		ch->desc->str = &obj->short_description; 
		ch->desc->max_str=80;
		strcpy(ch->desc->editing,"Object Short Description");
		ch->desc->oneline=TRUE;
		break;
	    case 3: 
	        if(objs[obj->item_number]->description==obj->description)
		    obj->description=str_dup(objs[obj->item_number]->description);
		ch->desc->str = &obj->description; 
		ch->desc->max_str=80;
		strcpy(ch->desc->editing,"Object Description");
		ch->desc->oneline=TRUE;
		break;
	    case 4:
		if (!*string)
		{
		    send_to_char("You have to supply a keyword.\n\r", ch);
		    return;
		}
		/* try to locate extra description */
		for (ed = obj->ex_description; ; ed = ed->next)
		    if (!ed) /* the field was not found. create a new one. */
		    {
			CREATE(ed , struct extra_descr_data, 1);
			ed->next = obj->ex_description;
			obj->ex_description = ed;
			CREATE(ed->keyword, char, strlen(string) + 1);
			strcpy(ed->keyword, string);
			ed->description = 0;
			ch->desc->str = &ed->description;
			ch->desc->max_str=1000;
			strcpy(ch->desc->editing,"Obj Xtra Description");
			send_to_char("New field.\n\r", ch);
			break;
		    }
		    else if (!str_cmp(ed->keyword, string))
		    /* the field exists */
		    {
/*			ed->description = my_free(ed->description);
*/
			ch->desc->str = &ed->description;
			ch->desc->max_str=1000;
			strcpy(ch->desc->editing,"Obj Xtra Description");
			send_to_char(
			    "Modifying description.\n\r", ch);
			break;
		    }
		ch->desc->max_str = MAX_STRING_LENGTH;
		/* the stndrd (see below) procedure does not apply here */
		return;
	    break;
	    case 6: 
		if (!*string)
		{
		    send_to_char("You must supply a field name.\n\r", ch);
		    return;
		}
		/* try to locate field */
		for (ed = obj->ex_description; ; ed = ed->next)
		    if (!ed)
		    {
			send_to_char("No field with that keyword.\n\r", ch);
			return;
		    }
		    else if (!str_cmp(ed->keyword, string))
		    {
			ed->keyword = my_free(ed->keyword);
			if (ed->description)
			    ed->description = my_free(ed->description);
			
			/* delete the entry in the desr list */                     
			if (ed == obj->ex_description)
			    obj->ex_description = ed->next;
			else
			{
			    for(tmp = obj->ex_description; tmp->next != ed; 
				tmp = tmp->next);
			    tmp->next = ed->next;
			}
			ed = my_free(ed);

			send_to_char("Field deleted.\n\r", ch);
			return;
		    }
	    break;              
	    default:
	       send_to_char(
		  "That field is undefined for objects.\n\r", ch);
	       return;
	    break;
	}
    }
/*
    if (*ch->desc->str)
    {
	*ch->desc->str = my_free(*ch->desc->str);
    }
*/
    if (*string)   /* there was a string in the argument array */
    {
	if (strlen(string) > length[field - 1])
	{
	    send_to_char("String too long - truncated.\n\r", ch);
	    *(string + length[field - 1]) = '\0';
	}
	*ch->desc->str=(char *)realloc(*ch->desc->str, strlen(string) + 1);
	strcpy(*ch->desc->str, string);
	ch->desc->str = 0;
	ch->desc->oneline=0;
	ch->desc->editing[0]=MED_NULL;
	send_to_char("Ok.\n\r", ch);
    }
    else          /* there was no string. enter string mode */
    {
/*	CREATE(*ch->desc->str, char, length[field - 1]);*/
	ch->desc->max_str = length[field - 1];
    }
}




/* db stuff *********************************************** */


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char *one_word(char *argument, char *first_arg )
{
    int found, begin, look_at;

    found = begin = 0;

    do
    {
	for ( ;isspace(*(argument + begin)); begin++);

	if (*(argument+begin) == '\"') {  /* is it a quote */

	    begin++;

	    for (look_at=0; (*(argument+begin+look_at) >= ' ') && 
		(*(argument+begin+look_at) != '\"') ; look_at++)
		*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

	    if (*(argument+begin+look_at) == '\"')
		begin++;

	} else {

	    for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
		*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

	}

	*(first_arg + look_at) = '\0';
	begin += look_at;
    }
    while (fill_word(first_arg));

    return(argument+begin);
}



#define	MAX_HELP	600

struct help_index_element *build_help_index(FILE *fl, int *num)
{
    int nr = -1, issorted, i;
    struct help_index_element *list = 0, mem;
    char buf[81], tmp[81], *scan;
    long pos;

    CREATE( list, struct help_index_element, MAX_HELP );

    for (;;)
    {
	pos = ftell(fl);
	fgets(buf, 81, fl);
	*(buf + strlen(buf) - 1) = '\0';
	scan = buf;
	for (;;)
	{
	    /* extract the keywords */
	    scan = one_word(scan, tmp);

	    if (!*tmp)
		break;

	    if ( ++nr >= MAX_HELP )
	    {
		perror( "Too many help keywords." );
		exit( 1 );
	    }

	    list[nr].keyword	= str_dup(tmp);
	    list[nr].pos	= pos;
	}

	/* skip the text */
	do
	    fgets(buf, 81, fl);
	while (*buf != '#');
	if (*(buf + 1) == '~')
	    break;
    }

    /* we might as well sort the stuff */
    do
    {
	issorted = 1;
	for (i = 0; i < nr; i++)
	    if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0)
	    {
		mem = list[i];
		list[i] = list[i + 1];
		list[i + 1] = mem;
		issorted = 0;
	    }
    }
    while (!issorted);

    *num = nr;
    return(list);
}



void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
    if (!d)
		return;
    if(grep_text[0]){
		write_to_q(str,&d->output);
		return;
    }
    if (keep_internal)
    {
		CREATE(d->showstr_head, char, strlen(str) + 1);
		strcpy(d->showstr_head, str);
		d->showstr_point = d->showstr_head;
    }
    else
		d->showstr_point = str;

    show_string(d, "");
}



void show_string(struct descriptor_data *d, char *input)
{
    char buffer[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;

	buffer[0] = '\0';
	buf[0]    = '\0';

    one_argument(input, buf);

    if (*buf)
    {
	if (d->showstr_head)
	{
	    strcpy(free_error,"d->showstr_head 1 in show_string modify.c");
	    d->showstr_head = my_free(d->showstr_head);
	}
	d->showstr_point = NULL;
        if(d->character)
	   if(d->character->specials.ansi_color==69){
		send_to_char("\033[0m",d->character);
	   }
	return;
    }

    /* show a chunk */
      for (scan = buffer;; scan++, d->showstr_point++)
	if((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) &&
	    ((toggle = -toggle) < 0))
	    lines++;
	else if (!*scan || (lines >= 22))
	{
	    *scan = '\0';
	    write_to_q( buffer, &d->output );

	    /* see if this is the end (or near the end) of the string */
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    if (!*chk)
	    {
		if (d->showstr_head)
		{
	            strcpy(free_error,"d->showstr_head 2 in show_string modify.c");
		    d->showstr_head = my_free(d->showstr_head);
		}
		d->showstr_point = NULL;
	        if(d->character)
		   if(d->character->specials.ansi_color==69){
			send_to_char("\033[0m",d->character);
		   }
	    }
	    return;
	}
}

void do_rebuild_help(struct char_data *ch, char *arg, int cmd)
{
	int counter=0;
	char mybuf[MAX_INPUT_LENGTH];
	FILE *fh;

	top_of_helpt=0;
	help_index = my_free(help_index);
	rewind(help_fl);
    help_index = build_help_index(help_fl, &top_of_helpt);
	top_trivia=0;
	if((fh=med_open("../lib/trivia.txt","r"))!=NULL){
		fscanf(fh,"%d\n",&top_trivia);
		for(counter=0;counter <= top_trivia; counter++){
			if(!feof(fh)){
				fgets(mybuf, sizeof(mybuf), fh); 
				trivia[counter]=strdup(mybuf);
			}
		}
		med_close(fh);
	}
}
