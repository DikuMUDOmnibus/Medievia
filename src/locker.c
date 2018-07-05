#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "db.h"
#include "locker.h"

/* Internal function declarations */
void get_locker_init(struct char_data *, struct locker_info *, char *);
bool look_in_locker(struct char_data *, char *, struct locker_info);
bool open_locker(struct char_data *, char *, struct locker_info);
bool close_locker(struct char_data *, char *, struct locker_info);
bool get_from_locker(struct char_data *, char *, struct locker_info *);
bool put_in_locker(struct char_data *, char *, struct locker_info *);
bool can_rent(struct char_data *, struct obj_data *, bool);
bool rent_locker(struct char_data *, char *, struct locker_info);
bool insure_locker_one(struct char_data *, char *, struct locker_info);
bool new_locker(struct char_data *, char *, struct locker_info *);
bool extend_locker(struct char_data *, char *, struct locker_info *);
bool insure_locker_two(struct char_data *, char *, struct locker_info);
bool write_out_locker(struct char_data *, char *, struct locker_info *, bool);
bool check_locker_rent(struct char_data *, char *, struct locker_info *);
bool object_to_locker(struct char_data *, struct obj_data *,struct locker_info *);
void zero_one_elem(struct obj_file_elem *);
void object_from_elem(struct obj_data *, struct obj_file_elem);
void elem_from_object(struct obj_data *, struct obj_file_elem *);

/* External function declarations */
extern struct obj_data *deteriorate(struct obj_data *obj);
extern struct obj_data *objs[MAX_OBJ];
extern int weigh(struct obj_data *obj);
extern void extract_obj(struct obj_data *);
extern void do_get(struct char_data *, char *, int);
extern struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list);
extern char global_color;
extern struct zone_data *zone_table;
extern struct room_data *world[MAX_ROOM];
extern int VERSIONNUMBER;

int u_store_it(struct char_data *ch, int cmd, char *arg){
	int x=0;
	struct locker_info locker;

	locker.month=0;
	locker.day=0;
	locker.year=0;
	locker.rented = 0;
	locker.filename[0]='\0';
	locker.value=0;
	locker.insurance=0;

    if(IS_NPC(ch))return(FALSE); /* Act like there is no spec proc for 
					Mobs and undeads. */
	for (x=0;x<MAX_LOCKER_ITEMS+1;x++){
		zero_one_elem(&locker.items[x]);
	}
	locker.num_items=0;

    if(ch->p)
      if(ch->p->queryintcommand)cmd=ch->p->queryintcommand;

    switch(cmd){
        case 15: /* look */
	    if(!check_locker_rent(ch,arg,&locker))
		return(TRUE);
            return(look_in_locker(ch, arg, locker));
            break;
        case 220: /* open */
	    if(!check_locker_rent(ch,arg,&locker))
		return(TRUE);
            return(open_locker(ch, arg, locker));
            break;
        case 221: /* close */
	    if(!check_locker_rent(ch,arg,&locker))
		return(TRUE);
            return(close_locker(ch, arg, locker));
            break;
        case 222: /* get */
            if(!check_locker_rent(ch,arg,&locker))
                return(TRUE);
            return(get_from_locker(ch, arg, &locker));
            break;
        case 223: /* put */
            if(!check_locker_rent(ch,arg,&locker))
                return(TRUE);
	    return(put_in_locker(ch, arg, &locker));
            break;
        case 224: /* rent */
            if(!check_locker_rent(ch,arg,&locker))
                return(TRUE);
            return(rent_locker(ch, arg, locker));
            break;
        case 225: /* insure */
            if(!check_locker_rent(ch,arg,&locker))
                return(TRUE);
            return(insure_locker_one(ch, arg, locker));
            break;
        case 226: /* claim */
            if(!check_locker_rent(ch,arg,&locker))
                return(TRUE);
            if(!locker.rented){
	        send_to_char("You do not have a locker rented!\n\r", ch);
    		return(TRUE);
	    }
            if(locker.opened!=10){
                send_to_char("Your locker has not been robbed!\n\r", ch);
                return(TRUE);
            }
            return(TRUE);
            break;
        case 227: /*rent a new locker for one month?*/
	    if (IS_NPC(ch)){
		send_to_char("You can't store anything here, you're a MONSTER!!!\n\r", ch);
		return(FALSE);
	    }
	    check_locker_rent(ch,arg,&locker);
            return(new_locker(ch, arg, &locker));
            break;
        case 228: /* extend locker rent */
            if(!check_locker_rent(ch,arg,&locker))
                return(TRUE);
            return(extend_locker(ch, arg, &locker));
            break;
        case 229: /* insure locker */
            if(!check_locker_rent(ch,arg,&locker))
                return(TRUE);
            return(insure_locker_two(ch, arg, locker));
            break;
        default:
            return(FALSE);
            break;
    }
return(FALSE);
}


void get_locker_init(struct char_data *ch, struct locker_info *locker,char *arg){
    int  counter = 0,x=0;
    char name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    FILE *fh;
	bool new_locker_type=FALSE;
	struct obj_data *obj;

    strcpy(name,GET_NAME(ch));
    sprintf(locker->filename,"%s/%c/%s.usi2", SAVE_DIR, LOWER(name[0]),name);
	if((fh=med_open(locker->filename,"r"))!=NULL){
	    new_locker_type=TRUE;
	    med_close(fh);
	}else{
	    sprintf(locker->filename,"%s/%c/%s.usi", SAVE_DIR, LOWER(name[0]),name);
	    if((fh=med_open(locker->filename,"r"))!=NULL){
        	new_locker_type=FALSE;
        	med_close(fh);
	    }
	}

	if(new_locker_type)
	    sprintf(locker->filename,"%s/%c/%s.usi2",SAVE_DIR,LOWER(name[0]),name);
  
    one_argument(arg, buf);

    if((fh=med_open(locker->filename, "r"))!=NULL){
        open_files++;
        locker->rented=1;
        fread(&locker->opened, sizeof(char),1,fh);
        fread(&locker->day, sizeof(int),1,fh);
        fread(&locker->month, sizeof(int),1,fh);
        fread(&locker->year, sizeof(int),1,fh);
        fread(&locker->insurance, sizeof(long),1,fh);
	if(new_locker_type){
            fread(&locker->items[counter], sizeof(struct obj_file_elem), 1, fh);

       	    while(locker->items[counter].item_number){
            	locker->value+=locker->items[counter].value[0];
            	fread(&locker->items[++counter], sizeof(struct obj_file_elem),1,fh);
            }
	}else{  /*got an old locker format..convert to new*/
            fread(&x, sizeof(int), 1, fh);/*get the old object num*/
					  /*convert it to the new format*/

	    if(x>0)
		obj=read_object(real_object(x),0);
	    else
		obj=NULL;

	    if(obj!=NULL){
		elem_from_object(obj, &locker->items[counter]);
		locker->value+=locker->items[counter].value[0];
	    }

	    /*Do the rest of the items in the old locker format
	      converting them along the way to the new locker
	      format. */

            while(locker->items[counter].item_number){
		fread(&x, sizeof(int), 1, fh);/*read old value field*/
                fread(&x, sizeof(int), 1, fh);/*read next old object*/
		if(x>0)
            	    obj=read_object(real_object(x),0);
		else
		    obj=NULL;
		if(obj!=NULL){
            	    elem_from_object(obj, &locker->items[++counter]);
            	    locker->value+=locker->items[counter].value[0];
		}else{
		    counter++;
		}
	     }
			
	    /*Now that we've converted it over, lets get rid of the 
	      old locker file*/

	    unlink(locker->filename);

	    /*and write it out the new way. first we must tell it the
	      new file name*/

	    sprintf(locker->filename,"%s/%c/%s.usi2", SAVE_DIR, LOWER(name[0]),name);
	}
	locker->num_items = counter;
        med_close(fh);
        open_files--;
	if(!new_locker_type)
	    write_out_locker(ch,arg,locker,TRUE);
	}
}

bool look_in_locker(struct char_data *ch, 
		    char *arg, 
		    struct locker_info locker){

    int x = 0;
    int y = 0;
    struct obj_data *obj=NULL;
    char buf[MAX_STRING_LENGTH];

    if(!isname("locker",arg))
	return(FALSE);
            
    if(!locker.rented){
	send_to_char("You do not have a locker rented!\n\r", ch);
	return(TRUE);
    }
 
    if(!locker.opened){
	send_to_char("Your locker is closed. You must open it.\n\r", ch);
        return(TRUE);
    }

    if(locker.opened==10){
	send_to_char("YOU'VE BEEN ROBBED! The locker is empty!\n\r",ch);
	return(TRUE);
    }

    send_to_char("You look in your locker and see:\n\r", ch);

    for(x=0;x<locker.num_items;x++){
	obj=read_object(real_object(locker.items[x].item_number),0);
	if(!obj){
    	    sprintf(log_buf,"Look in locker object #%d doesn't exist",locker.items[x].item_number);
	    log_hd(log_buf);
	    for(y=x;y<locker.num_items-1;y++)
		locker.items[y] = locker.items[y+1];
	    locker.num_items--;
    	}else{
	    sprintf(buf,"%s\n\r",obj->short_description);
	    send_to_char(buf,ch);
	    extract_obj(obj);
	}
    }
    return(TRUE);
}

bool open_locker(struct char_data *ch,
		 char *arg,
		 struct locker_info locker){

    if(!locker.rented){
	send_to_char("You do not have a locker rented!\n\r", ch);
	return(TRUE);
    }

    if(locker.opened){
	send_to_char("Your locker is already opened.\n\r",ch);
	return(TRUE);
    }

    if(locker.opened==10){
	send_to_char("YOU'VE BEEN ROBBED! The locker is empty!\n\r",ch);
	return(TRUE);
    }

    locker.opened=1;

    if(write_out_locker(ch,arg,&locker,FALSE)){
        send_to_char("You open your locker.\n\rRemember to close it before leaving or your stuff will be stolen!\n\r", ch);
	act("$n opens $s locker.",TRUE,ch,0,0,TO_ROOM);
    }else
	send_to_char("File error, please use bug command and explain exactly what you did", ch);
    return(TRUE);
}

bool close_locker(struct char_data *ch,
                 char *arg,
                 struct locker_info locker){

    if(!locker.rented){
	send_to_char("You don't have a locker rented!\n\r",ch);
	return(TRUE);
    }

    if(!locker.opened){
	send_to_char("Your locker is already closed.\n\r",ch);
	return(TRUE);
    }

    if(locker.opened==10)
	send_to_char("YOU'VE BEEN ROBBED! The locker is empty!\n\r",ch);

    locker.opened=0;
	if(write_out_locker(ch,arg,&locker,FALSE)){
	    send_to_char("You close your locker and pray that the place doesn't get robbed.\n\r", ch);
 	act("$n closes $s locker.",TRUE,ch,0,0,TO_ROOM);
    }else
	send_to_char("File error, please use bug command and explain exactly what you did", ch);

    SAVE_CHAR_OBJ(ch,-20);
    return(TRUE);
}

bool get_from_locker(struct char_data *ch,
                 char *arg,
                 struct locker_info *locker){

    int all     = 0;
    int x       = 0;
    int y	= 0;
    int found	= 0;
    char buf[MAX_STRING_LENGTH];
    struct obj_data *obj=NULL;

    one_argument(arg, buf);

    if(!locker->rented){
	do_get(ch,buf,9);
	return(TRUE);
    }

    if(!locker->opened){
	send_to_char("Your locker is closed. You look on the floor.\n\r", ch);
	do_get(ch,buf,9);
	return(TRUE);
    }

    if(locker->opened==10){
	send_to_char("YOU'VE BEEN ROBBED! The locker is empty!\n\r",ch);                return(TRUE);
    }

    if(!strcmp(buf,"all")){
	all = 1;
	for(x=0;x<locker->num_items;x++){
	    obj=read_object(locker->items[x].item_number,0);
	    if(obj)
		object_from_elem(obj,locker->items[x]);
	    if(!obj){
		sprintf(log_buf,"Get all  in locker objec #%d doesn't exist",locker->items[x].item_number);
		log_hd(log_buf);
	    }else{
	    	obj_to_char(obj,ch);
	    	sprintf(log_buf,"You get %s from your locker.\n\r",obj->short_description);
	    	send_to_char(log_buf,ch);
	    }
	}
/*	act("$n gets something from $s locker.",TRUE,ch,0,0,TO_ROOM);*/
	locker->num_items=0;
    }else{
	for(x=0;x<locker->num_items;x++){
	    obj=read_object(locker->items[x].item_number,0);
	    if(!obj){
	       	sprintf(log_buf,"Get1 in locker object #%d doesn't exist",locker->items[x].item_number);
	       	log_hd(log_buf);
		for(y=x;y<locker->num_items-1;y++)        
		    locker->items[y]=locker->items[y+1];  
		    locker->num_items--;
		    x--;
	    	}else{
		    object_from_elem(obj, locker->items[x]);
			
	    	if(isname(buf,obj->name)){
	            found=x;
		    x=100;
		}
		extract_obj(obj);
	    }
	}

	if(x<100){
	    sprintf(arg,"You couldn't find a %s in the locker.\n\r",buf);                   send_to_char(arg,ch);
	    send_to_char("You look on the floor....\n\r",ch);
	    do_get(ch,buf,9);
	    return(TRUE);
	}
	obj=read_object(locker->items[found].item_number,0);
	object_from_elem(obj, locker->items[found]);
	obj_to_char(obj,ch);
    }
    if(locker->num_items>0){
	for(x=found;x<locker->num_items-1;x++)
	    locker->items[x]=locker->items[x+1];

	zero_one_elem(&locker->items[locker->num_items-1]);
    }
    if(write_out_locker(ch,arg,locker,TRUE)){
        if(!all){
            send_to_char("You get it from your locker.\n\r", ch);
/*            act("$n gets something from $s locker.",TRUE,ch,0,0,TO_ROOM);*/
        }
    }else
	send_to_char("File, error, please use the bug command to explain exactly what you did.\n\r", ch);

    SAVE_CHAR_OBJ(ch,-20);
    return(TRUE);
}

bool put_in_locker(struct char_data *ch,char *arg,struct locker_info *locker)
{
int all=0;
int now_num=0;
char buf[MAX_STRING_LENGTH];
struct obj_data *next=NULL;
struct obj_data *obj=NULL;
struct obj_file_elem object;

    one_argument(arg, buf);
    if(!locker->rented){
		send_to_char("You do not have a locker rented.\n\r", ch);
		return(TRUE);
    }
    if(!locker->opened){
		send_to_char("Your locker is closed. You must open it.\n\r", ch);
		return(TRUE);
    }
    if(!buf[0]){
		send_to_char("Put what in your locker?!?\n\r",ch);
		return(TRUE);
    }
    if(!strcmp(buf,"everything")){
	/* return(FALSE); */
	all=1;
	next=ch->carrying;
	now_num=locker->num_items;
	while(next){
	    	obj=next;
	    	if(locker->num_items>=MAX_LOCKER_ITEMS){
			global_color=31;
			send_to_char("As much as you try, you cannot sqeeze one more thing in the locker.\n\r", ch);
			global_color=0;
			next=NULL;
	    	}else{
	    	  if (can_rent(ch,obj,FALSE)){
			next=obj->next_content;
			zero_one_elem(&object);
			elem_from_object(obj, &object);
	            	locker->items[locker->num_items]=object;
	            	locker->num_items++;
	            	sprintf(log_buf,"You put %s in your locker.\n\r",obj->short_description);
	            	send_to_char(log_buf,ch);
	            	extract_obj(obj);
	    	  }else{
			next=obj->next_content;
		  }
	    	}
	} /* while(next) */
/*
		if(locker->num_items!=now_num)
	    	act("$n puts something in $s locker.",TRUE,ch,0,0,TO_ROOM);
*/
    }else{
		if (!(obj=get_obj_in_list_vis(ch,buf,ch->carrying))){
		  send_to_char("You realize you don't have that.\n\r",ch);
		  return(TRUE);
		}
		if (locker->num_items>=MAX_LOCKER_ITEMS){
		  global_color=31;
		  send_to_char("As much as you try, you cannot sqeeze one more thing in the locker.\n\r", ch);
		  global_color=0;
		  return(TRUE);
		}
		if (can_rent(ch,obj,FALSE)){
		  zero_one_elem(&object);
		  elem_from_object(obj, &object);
		  extract_obj(obj);
		  locker->items[locker->num_items]=object;
		  locker->num_items++;
		}
    }
    if(write_out_locker(ch,arg,locker,TRUE)){
        if(!all){
	    	send_to_char("You put it in your locker.\n\r", ch);
/*	    	act("$n puts something in $s locker.",TRUE,ch,0,0,TO_ROOM);*/
		}
    }else
		send_to_char("File error, please use bug command and explain exactly what you did", ch);
    SAVE_CHAR_OBJ(ch,-20);
    return(TRUE);
 }

bool can_rent(struct char_data *ch,struct obj_data *obj,bool saveout){
    int type = 0;

    if(strstr(obj->short_description,"A bright ball of light")){
	if(!saveout){
	    global_color=31;
	    send_to_char("The bright ball of light simply refuses to be stored!\n\r",ch);
	    global_color=0;
	}
	return(FALSE);
    }
    if(obj->item_number==10005){
	if(!saveout){
	    global_color=31;
	    send_to_char("You may not store the Bracelt of Life\n\r",ch);
	    global_color=0;
	}
	return(FALSE);
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)){
	if(!saveout){	
	    global_color=31;
	    if((obj->item_number==10)||(obj->item_number==16))
	        send_to_char("You would never trust leaving THAT in a locker!\n\r", ch);
	    else
	   	send_to_char("You can't let go! It must be cursed!\n\r", ch);
		global_color=0;
	}
	return(FALSE);
    }

    if ( GET_ITEM_TYPE(obj) == ITEM_CONTAINER ){
	if(!saveout){
	    global_color=31;
	    send_to_char("You cannot store a container.\n\r", ch);
	    global_color=0;
	}
	return(FALSE);
    }

    type=GET_ITEM_TYPE(obj);
    if(type==ITEM_KEY||
	type==ITEM_SCROLL||
	type==ITEM_WAND||
	type==ITEM_STAFF||
	type==ITEM_POTION||
	IS_SET(obj->obj_flags.extra_flags, ITEM_NO_RENT)){
	    if(!saveout){
		sprintf(log_buf,"[STORING %s IS NOT ALLOWED]\n\r",obj->short_description);
		global_color=31;
		send_to_char(log_buf,ch);
		global_color=0;
	    }
	    return(FALSE);
    }

    if(obj->item_number==10005){  /* Bracelet of Life */
	if(!saveout){
	    sprintf(log_buf,"[STORING %s IS NOT ALLOWED]\n\r",obj->short_description);
	    global_color=31;
	    send_to_char(log_buf,ch);
	    global_color=0;
	}
	return(FALSE);
    }

    return(TRUE);
}

bool rent_locker(struct char_data *ch,
                 char *arg,
                 struct locker_info locker){

    char buf[MAX_STRING_LENGTH];

    if(!locker.rented){
	if(GET_GOLD(ch)<150000){
	    send_to_char("You do not have the 150000 Gold Coins it costs to rent a locker\n\r for a month.\n\r",ch);
	    return(TRUE);
	}
    	ch->p->queryintfunc=u_store_it;
    	strcpy(ch->p->queryprompt,"Rent a locker for a month at a cost of 150000 Gold Coins? (y/n)>");
    	ch->p->queryintcommand=227;
    	return(TRUE);
    }

	if(locker.month>=11){
		locker.month=0;
		locker.year++;
	}
    sprintf(buf,"Your locker rent is paid up till %d/%d/%d.\n\r",locker.month+1,locker.day,locker.year);
    send_to_char(buf, ch);
    ch->p->queryintfunc=u_store_it;
    strcpy(ch->p->queryprompt,"Pay for an additional month at 150000 Gold Coins? (y/n)> ");
    ch->p->queryintcommand=228;
    return(TRUE);
}

bool insure_locker_one(struct char_data *ch,
                 char *arg,
                 struct locker_info locker){

    char buf[MAX_STRING_LENGTH];

    if(!locker.rented)
	send_to_char("You do not have a locker rented.\n\r",ch);
	return(TRUE);

    sprintf(buf,"Your insured amount is %ld and the value of your items is %ld.\n\r", locker.insurance, locker.value);
    send_to_char(buf,ch);

    if(locker.insurance>=locker.value){
	send_to_char("You are fully insured.\n\r", ch);
	return(TRUE);
    }

    if(GET_GOLD(ch)<=(0.1*(locker.value-locker.insurance))){
	send_to_char("You cannot afford insurance at this time.\n\r",ch);
	return(TRUE);
    }

    sprintf(buf,"It will cost %ld Gold Coins to fully insure your items.\n\r", (long)(0.1*(locker.value-locker.insurance)));
    send_to_char(buf,ch);
    ch->p->queryintfunc=u_store_it;
    strcpy(ch->p->queryprompt,"Pay the insurance? (y/n)>");
    ch->p->queryintcommand=229;
    return(TRUE);
}

bool new_locker(struct char_data *ch,
                 char *arg,
                 struct locker_info *locker){

    char buf[MAX_STRING_LENGTH];
	struct tm *thetime;
	long t=0;

    one_argument(arg, buf);

    if(buf[0]=='Y'||buf[0]=='y'){
		t=time(0);	
		thetime=localtime(&t);
		locker->month     = thetime->tm_mon+1;
		locker->year      = thetime->tm_year;
		locker->day       = thetime->tm_mday;
		locker->opened    = 0;
		locker->rented	  = 1;
		locker->insurance = 0;
		locker->num_items = 0;
		if(locker->month>12){locker->month=1;locker->year++;}
		if(write_out_locker(ch, arg, locker,FALSE)){
			send_to_char("Your locker has been set up\n\rAt a cost of 150000 Gold Coins.\n\rREMEMBER you must come back within a month to pay for next months rent!\n\r", ch);
		GET_GOLD(ch)-=150000;
    	}else
			send_to_char("File error, please use bug command and explain exactly what you did.\n\r", ch);
    }
    ch->p->queryintcommand=0;
    return(TRUE);
}

bool extend_locker(struct char_data *ch,
                 char *arg,
                 struct locker_info *locker){

	char buf[MAX_STRING_LENGTH];

	one_argument(arg, buf);

    if(GET_GOLD(ch)<150000){
	send_to_char("You do not have the 150000 Gold Coins it costs to rent a locker\n\r for another month.\n\r",ch);
	ch->p->queryintcommand=0;
	return(TRUE);
    }

    if(buf[0]=='Y'||buf[0]=='y'){
	if(locker->month>=11){
	    locker->month=0;
	    locker->year++;
	}else{locker->month++;}
		if(write_out_locker(ch,arg,locker,FALSE)){
		    send_to_char("DONE! Your account has been updated.\n\r", ch);
		    sprintf(buf,"%s's locker rent is paid up till %d/%d/%d.\n\r",GET_NAME(ch),locker->month+1,locker->day,locker->year);
		    log_hd(buf);
		    GET_GOLD(ch)-=150000;
	    }else
		send_to_char("File error, please use bug command and explain exactly what you did", ch);
    }
    ch->p->queryintcommand=0;
    return(TRUE);
}

bool insure_locker_two(struct char_data *ch,
                 char *arg,
                 struct locker_info locker){

	char buf[MAX_STRING_LENGTH];

    if(buf[0]=='Y'||buf[0]=='y'){
		if(write_out_locker(ch,arg,&locker,FALSE))
			send_to_char("DONE! Your account has been updated.\n\r", ch);
    }
    ch->p->queryintcommand=0;
    return(TRUE);
}

bool write_out_locker(struct char_data *ch,
                 	  char *arg,
                 	  struct locker_info *locker,
					  bool saveout){

	FILE *fh=NULL;
	int numdone    = 0;
	int x	       = 0;
	int terminator = 0;
	char buf[MAX_STRING_LENGTH];

	if((fh=med_open(locker->filename, "w"))!=NULL){
		open_files++;
		numdone = fwrite(&locker->opened, sizeof(char),1,fh);
		numdone=fwrite(&locker->day,sizeof(int),1,fh);
		numdone=fwrite(&locker->month, sizeof(int), 1,fh);
		numdone=fwrite(&locker->year, sizeof(int), 1, fh);
		numdone=fwrite(&locker->insurance, sizeof(long), 1, fh);
		for(x=0;x<locker->num_items;x++){
			fwrite(&locker->items[x], sizeof(struct obj_file_elem), 1, fh);
		}
		fwrite(&terminator, sizeof(int), 1, fh);
	
		if((numdone<1)&&(!saveout)){
			send_to_char("Sorry, write error, please use bug command.\n\r", ch);
			return(FALSE);
		}else{
			if(!saveout){
				sprintf(buf, "Your locker rent is paid up to %d/%d/%d.\n\r", locker->month+1, locker->day, locker->year);
				send_to_char(buf, ch);
			}
		}
		med_close(fh);
		open_files--;
	}else{
		send_to_char("File error, please use bug command and explain exactly what you did", ch);
		return(FALSE);
	}

	return(TRUE);
}

bool check_locker_rent(struct char_data *ch, char *arg, struct locker_info *locker){

	struct tm *thetime;
	long t = 0;
	long curdays = 0;
	long dayspaidfor = 0;

    if(IS_NPC(ch)){
        send_to_char("Monsters do not need lockers!\n\r",ch);
        return(FALSE);
    }


    get_locker_init(ch, locker, arg);

    t = time(0);
    thetime=localtime(&t);

	curdays=(365*thetime->tm_year)+(30*thetime->tm_mon)+thetime->tm_mday;
    dayspaidfor=(locker->year*365)+(30*locker->month)+locker->day;

    if(dayspaidfor>0&&dayspaidfor<curdays){
        send_to_char("We are sorry, but, since you have not come back to pay for your next\n\rmonths rent we had to sell your property.\n\r", ch);
		unlink(locker->filename);
        ch->p->queryintcommand=0;
        return(FALSE);
    }
	return(TRUE);
}

void locker_stuff_for_player(struct char_data *ch)
{
struct obj_data *obj=NULL,*obj_to_locker=NULL;
int max;
int room;
int x = 0;
time_t t=0;
long curdays=0;
long dayspaidfor=0;
struct tm *thetime;
struct locker_info locker;

    locker.month=0;
    locker.day=0;
    locker.year=0;
    locker.rented = 0;
    locker.filename[0]='\0';
    locker.value=0;
    locker.insurance=0;
    for (x=0;x<MAX_LOCKER_ITEMS+1;x++){
        zero_one_elem(&locker.items[x]);
    }
    locker.num_items=0;

    t = time(0);
    thetime=localtime(&t);

	if(!check_locker_rent(ch,"",&locker))
		return;

	curdays=(365*thetime->tm_year)+(30*thetime->tm_mon)+thetime->tm_mday;

    dayspaidfor=(locker.year*365)+(30*locker.month)+locker.day;
    if(dayspaidfor<curdays){
        return;
    }

    if(!locker.rented)
        return;

    if(locker.num_items>=MAX_LOCKER_ITEMS)
        return;

/* This function rents as much of the most expensive stuff the player has
   that fits into his locker  */

	/*
    if(ch->specials.home_number){
    	char_from_room(ch);
    	char_to_room(ch,ch->specials.home_number);
    	return;
    }
	*/
    room=1;
    if(GET_CONTINENT(ch)==2)room=1371;
    char_from_room(ch);
    char_to_room(ch,room);
    while(1){
    	max=0;
    	obj=NULL;
    	obj_to_locker=NULL;

    	for(obj=ch->carrying;obj;obj=obj->next_content){
			if(can_rent(ch, obj, TRUE)){
       			if(obj->obj_flags.cost_per_day>max){
        			max=obj->obj_flags.cost_per_day;
        			obj_to_locker=obj;
        		}
        	}
		}
    	if((obj_to_locker)&&(locker.num_items<MAX_LOCKER_ITEMS-1)){
        	if(!object_to_locker(ch,obj_to_locker,&locker)){
        		return;
        	}else{
           		extract_obj(obj_to_locker);
        	}
		}

		if((!obj_to_locker)||(locker.num_items>=MAX_LOCKER_ITEMS-1)){
			write_out_locker(ch,"",&locker,TRUE);
        	return;
    	}

    }
}

bool object_to_locker(struct char_data *ch, struct obj_data *obj, struct locker_info *locker){
	struct obj_file_elem fobj;

    if(!ch||!obj)return(FALSE);

	elem_from_object(obj,&fobj);
    locker->items[locker->num_items]=fobj;
	locker->num_items++;
    return(TRUE);
}

void zero_one_elem(struct obj_file_elem *t){
	int y=0;

    t->version=0;
    t->item_number=0;
    for(y=0;y<4;y++)
        t->value[y]=0;
    t->extra_flags=0;
    t->wear_loc=0;
    t->weight=0;
    t->eq_level=0;
    t->timer=0;
    t->bitvector=0;
    for(y=0;y<MAX_OBJ_AFFECT;y++){
        t->affected[y].location=0;
        t->affected[y].modifier=0;
    }
    t->iWeight = 0;
    t->iWeightVersion = 0;
    t->iItemBorn = 0;
    t->iItemLasts =0;
    t->iLastDet = 0;    
}

void object_from_elem(struct obj_data *obj, struct obj_file_elem object){
	int x=0;

	for(x=1;x<4;x++)
        obj->obj_flags.value[x]=object.value[x];
    obj->obj_flags.extra_flags=object.extra_flags;
    /*obj->obj_flags.weight=object.weight;*/
    obj->obj_flags.timer=object.timer;
    obj->obj_flags.eq_level=object.eq_level;
    obj->obj_flags.bitvector=object.bitvector;
    for(x=0;x<MAX_OBJ_AFFECT;x++)
        obj->affected[x]=object.affected[x];
	obj->iValueWeight=object.iWeight;
	obj->iWeightVersion=object.iWeightVersion;
        obj->iBornDate = object.iItemBorn;
        obj->iLastDet = object.iLastDet;
        obj->iDetLife = object.iItemLasts;

	if(!obj->iValueWeight||obj->iWeightVersion!=VERSIONNUMBER){
		obj->iValueWeight=weigh(obj);
		obj->iWeightVersion=VERSIONNUMBER;
		obj->iBornDate = (int) time(NULL);
		obj->iLastDet = 0;
		obj->iDetLife = objs[obj->item_number]->iDetLife;
	}
	obj = deteriorate(obj);
}

void elem_from_object(struct obj_data *obj, struct obj_file_elem *object){
	int x=0;

	object->version = 0;
	object->item_number=obj->item_number;
	object->value[0]=obj->obj_flags.value[0];
    for(x=1;x<4;x++)
		object->value[x]=obj->obj_flags.value[x];
	object->extra_flags=obj->obj_flags.extra_flags;
	object->weight=obj->obj_flags.weight;
	object->timer=obj->obj_flags.timer;
	object->eq_level=obj->obj_flags.eq_level;
	object->bitvector=obj->obj_flags.bitvector;
    for(x=0;x<MAX_OBJ_AFFECT;x++)
		object->affected[x]=obj->affected[x];
	object->iWeight=obj->iValueWeight;
	object->iWeightVersion=obj->iWeightVersion;
	object->iItemBorn = obj->iBornDate;
	object->iItemLasts = obj->iDetLife;
	object->iLastDet = obj->iLastDet;
}
