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


#include <stdio.h>

#include "structs.h"
#include "mob.h"
#include "obj.h"
#include "utils.h"
#include "db.h"

typedef int    SPEC_FUN  (struct char_data *ch, int cmd, char *argument);

extern struct room_data *world[MAX_ROOM]; /* array of rooms  */
extern struct index_data *mob_index;
extern struct index_data *obj_index;
void boot_the_shops();
void assign_the_shopkeepers();

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
    SPEC_FUN	caveweed;
    SPEC_FUN	darksprite;
    SPEC_FUN	whirlwind;
    SPEC_FUN	caveray;
    SPEC_FUN    egg_collector;
    SPEC_FUN	cityguard;
    SPEC_FUN	guild;
    SPEC_FUN	mage_guild;
    SPEC_FUN	thief_guild;
    SPEC_FUN	warrior_guild;
    SPEC_FUN	cleric_guild;
    SPEC_FUN	guild_guard;
    SPEC_FUN	puff;
    SPEC_FUN	block_way;
    SPEC_FUN	fido;
    SPEC_FUN	janitor;
    SPEC_FUN	snake;
    SPEC_FUN	thief;
    SPEC_FUN	red_dragon;
    SPEC_FUN	blue_dragon;
    SPEC_FUN	green_dragon;
    SPEC_FUN	black_dragon;
    SPEC_FUN	white_dragon;
    SPEC_FUN	mud_school_adept;
    SPEC_FUN	brass_dragon;
    SPEC_FUN	train;
    SPEC_FUN    towncrier;
    SPEC_FUN    prisoner;
    SPEC_FUN    undead_corpse;
    SPEC_FUN    trellor_endar;
    SPEC_FUN    trellor_silor;
    SPEC_FUN    trellor_mika;
    SPEC_FUN    trellor_lokrath;
    SPEC_FUN    trellor_gate;
    SPEC_FUN    trellor_finchlady;
    SPEC_FUN    trellor_drunk_man;
    SPEC_FUN    trellor_lecher;
    SPEC_FUN    trellor_boy;
    SPEC_FUN    trellor_girl;
    SPEC_FUN    trellor_brawler;
    SPEC_FUN    trellor_puppy;
    SPEC_FUN    trellor_citizen_male;
    SPEC_FUN    trellor_citizen_female;
    SPEC_FUN    trellor_vagrant;
    SPEC_FUN    trellor_peasant_female;
    SPEC_FUN    trellor_peasant_male;
    SPEC_FUN    trellor_guard;
    SPEC_FUN    raven;
    mob_index[real_mobile(1)].func = puff;
    mob_index[real_mobile(17047)].func = raven;
    mob_index[real_mobile(704)].func = red_dragon;

    mob_index[real_mobile(211)].func = egg_collector;
    mob_index[real_mobile(17017)].func = whirlwind;
    mob_index[real_mobile(17016)].func = darksprite;
    mob_index[real_mobile(17015)].func = caveweed;
    mob_index[real_mobile(17014)].func = caveray;
    mob_index[real_mobile(9800)].func = undead_corpse;
    mob_index[real_mobile(9511)].func = janitor;

/*  bloodstone */
    mob_index[real_mobile(5729)].func = janitor;
    mob_index[real_mobile(9508)].func = block_way;/*efreeti*/
    mob_index[real_mobile(5702)].func = block_way;
    mob_index[real_mobile(5732)].func = block_way;
    mob_index[real_mobile(5747)].func = block_way;
    mob_index[real_mobile(5725)].func = block_way;
    mob_index[real_mobile(5767)].func = block_way;
    mob_index[real_mobile(5769)].func = block_way;
    mob_index[real_mobile(5770)].func = block_way;
    mob_index[real_mobile(5771)].func = block_way;
    mob_index[real_mobile(5772)].func = block_way;
    mob_index[real_mobile(5724)].func = block_way;
    mob_index[real_mobile(5751)].func = block_way;
    mob_index[real_mobile(5745)].func = block_way;
    mob_index[real_mobile(5773)].func = snake;

/*  Asnor mts. */
    mob_index[real_mobile(6258)].func = block_way;
    mob_index[real_mobile(6247)].func = block_way;

/*  Spirited Heights */
    mob_index[real_mobile(5801)].func = thief;    /* young girl */
    mob_index[real_mobile(5805)].func = snake;    /*giant tarantula*/
    mob_index[real_mobile(5818)].func = snake;    /*water moccasin*/
    mob_index[real_mobile(5819)].func = snake;    /*rattler*/

/*  Trellor */ 
    mob_index[real_mobile(5628)].func = thief;
    mob_index[real_mobile(5664)].func = thief;
	/* guildmasters */
    mob_index[real_mobile(5707)].func = mage_guild; /* red robed mage */
    mob_index[real_mobile(702)].func = warrior_guild; /* Adventurer */
    mob_index[real_mobile(304)].func = thief_guild; /* citizen */
    mob_index[real_mobile(6414)].func = cleric_guild; /* village priest */

    mob_index[real_mobile(1546)].func = guild; /* Priestess */
    mob_index[real_mobile(5658)].func = guild; /* Elevera */
    mob_index[real_mobile(1548)].func = guild; /* High Mage of the Guild */
    mob_index[real_mobile(5650)].func = guild; /* Battle Master */

/*
    mob_index[real_mobile(5603)].func = trellor_endar;
    mob_index[real_mobile(5605)].func = trellor_silor;
    mob_index[real_mobile(5606)].func = trellor_mika;
    mob_index[real_mobile(5617)].func = trellor_lokrath;
    mob_index[real_mobile(5601)].func = trellor_gate;
    mob_index[real_mobile(5607)].func = trellor_finchlady;
    mob_index[real_mobile(5608)].func = trellor_drunk_man;
    mob_index[real_mobile(5609)].func = trellor_lecher;
    mob_index[real_mobile(5610)].func = trellor_boy;
    mob_index[real_mobile(5611)].func = trellor_girl;
    mob_index[real_mobile(5612)].func = trellor_brawler;
    mob_index[real_mobile(5621)].func = trellor_puppy;
    mob_index[real_mobile(5622)].func = trellor_citizen_male;
    mob_index[real_mobile(5623)].func = trellor_citizen_female;
    mob_index[real_mobile(5624)].func = trellor_vagrant;
    mob_index[real_mobile(5625)].func = trellor_peasant_female;
    mob_index[real_mobile(5626)].func = trellor_peasant_male;
    mob_index[real_mobile(5602)].func = trellor_guard;
    mob_index[real_mobile(5620)].func = trellor_guard;
*/

/* Githyanki */
    mob_index[real_mobile(13118)].func = prisoner;

/* New Genesia */
    mob_index[real_mobile(7122)].func = janitor;
    mob_index[real_mobile(7107)].func = guild;
    mob_index[real_mobile(7108)].func = guild;
    mob_index[real_mobile(7109)].func = guild;
    mob_index[real_mobile(7110)].func = guild;

/*  City of Medievia */
    mob_index[real_mobile(3060)].func = cityguard; /* medivia guardsman */
    mob_index[real_mobile(3060)].func = cityguard; /* medivia guardsman */
    mob_index[real_mobile(3071)].func = janitor; /* tourist center */
    mob_index[real_mobile(3061)].func = janitor;
    mob_index[real_mobile(3020)].func = guild;  /* guildmasters */
    mob_index[real_mobile(3021)].func = guild;
    mob_index[real_mobile(3022)].func = guild;
    mob_index[real_mobile(3023)].func = guild;
    mob_index[real_mobile(4103)].func = thief;
    mob_index[real_mobile(2)].func = towncrier;

/* Mystical Forest */
    mob_index[real_mobile(6627)].func = janitor;
    mob_index[real_mobile(6618)].func = guild;
    mob_index[real_mobile(6655)].func = guild;
    mob_index[real_mobile(6656)].func = guild;
    mob_index[real_mobile(6657)].func = guild;
    mob_index[real_mobile(6616)].func = cityguard; 
    mob_index[real_mobile(6628)].func = cityguard; 
    mob_index[real_mobile(6644)].func = cityguard; 
    mob_index[real_mobile(6654)].func = cityguard; 
    mob_index[real_mobile(6615)].func = block_way; /*  huge green dragon */
    mob_index[real_mobile(6634)].func = block_way;
    mob_index[real_mobile(6637)].func = block_way;
    mob_index[real_mobile(6636)].func = block_way;
    mob_index[real_mobile(6638)].func = block_way;
    mob_index[real_mobile(6639)].func = block_way;
    mob_index[real_mobile(6660)].func = block_way;
    mob_index[real_mobile(6661)].func = block_way;
    mob_index[real_mobile(6662)].func = block_way;

/*  tree */
    mob_index[real_mobile(4158)].func = snake;

    mob_index[real_mobile(10016)].func = snake;
    mob_index[real_mobile(15040)].func = snake;   /*spider in Cliff*/
    mob_index[real_mobile(6113)].func = snake;    /*spiders in Castle Med*/
    mob_index[real_mobile(5121)].func = snake;    /*spiders in Drow Pit*/
    mob_index[real_mobile(6920)].func = snake;
    mob_index[real_mobile(18502)].func = snake;   /*mamba in wilderness*/
    mob_index[real_mobile(18515)].func = snake;   /*ant in wilderness*/
    mob_index[real_mobile(18526)].func = snake;   /*banelar in wilderness*/

    boot_the_shops();
    assign_the_shopkeepers();
}

/* assign special procedures to objects */
void assign_objects(void)
{
    int board(struct char_data *ch, int cmd, char *arg);
    int paper_box(struct char_data *ch, int cmd, char *arg);

    obj_index[real_object(3099)].func = board;
    obj_index[real_object(1972)].func = paper_box;
}

void assign_room(int room, int (*funct)())
{
char buf[100];
    if(world[room]){
    	world[room]->funct=funct;
    }else{
    	sprintf(buf,"## no room %d in assign_room",room);
    	log_hd(buf);
    }
}

/* assign special procedures to rooms */
void assign_rooms(void)
{

    int atm_machines(struct char_data *ch, int cmd, char *arg);
    int train(struct char_data *ch, int cmd, char *arg);
    int post_office(struct char_data *ch, int cmd, char *arg);
    int u_store_it(struct char_data *ch, int cmd, char *arg);
    int river_current(struct char_data *ch, int cmd, char *arg);
    int death_trap(struct char_data *ch, int cmd, char *arg);
    int Spirited_Heights_DT(struct char_data *ch, int cmd, char *arg);
    int tear_fountain(struct char_data *ch, int cmd, char *arg);
    int river_tree(struct char_data *ch, int cmd, char *arg);
    int mythago_wood(struct char_data *ch, int cmd, char *arg);
    int swamp_bog(struct char_data *ch, int cmd, char *arg);
    int convention(struct char_data *ch, int cmd, char *arg);
    int moongate(struct char_data *ch, int cmd, char *arg);
    int feeders(struct char_data *ch, int cmd, char *arg);
    int illusionary_room(struct char_data *ch, int cmd, char *arg);
    int pit_traps(struct char_data *ch, int cmd, char *arg);
    int silver_shrine(struct char_data *ch, int cmd, char *arg);
    int waterfall(struct char_data *ch, int cmd, char *arg);
    int horneg_sarcophogus_room(struct char_data *ch, int cmd, char *arg);
    int horneg_mage_room(struct char_data *ch, int cmd, char *arg);
    int play_cards(struct char_data *ch, int cmd, char *arg);
    int slot_machine(struct char_data *ch, int cmd, char *arg);
    int block_way_suites(struct char_data *ch, int cmd, char *arg);
	int belt_quest(struct char_data *ch, int cmd, char *arg);



/* THIS IS NOT THE ONLY PLACE TO CHANGE FOR A SLOT MACHINE- SEE VRYCE */
    assign_room(3155, slot_machine);
    assign_room(1859, slot_machine);
    assign_room(1860, slot_machine);
    assign_room(1861, slot_machine);
    assign_room(1867, slot_machine);
    assign_room(1866, slot_machine);
    assign_room(1865, slot_machine);
    assign_room(1874, slot_machine);
    assign_room(1875, slot_machine);
    assign_room(1876, slot_machine);
    assign_room(1868, slot_machine);
    assign_room(1872, slot_machine);
    assign_room(1873, slot_machine);
    assign_room(1864, slot_machine);
    assign_room(1863, slot_machine);
    assign_room(1862, slot_machine);
    assign_room(1856, slot_machine);
    assign_room(1857, slot_machine);
    assign_room(1858, slot_machine);
    assign_room(4307, slot_machine);
    assign_room(4308, slot_machine);
    assign_room(3173, slot_machine);
    assign_room(3174, slot_machine);
    assign_room(1894, block_way_suites);
    assign_room(1893, block_way_suites);
    assign_room(1892, block_way_suites);
    assign_room(1891, block_way_suites);
    assign_room(1890, block_way_suites);
    assign_room(1889, block_way_suites);
    assign_room(1888, block_way_suites);
    assign_room(1887, block_way_suites);
    assign_room(1886, block_way_suites);
    assign_room(1885, block_way_suites);
    assign_room(1884, block_way_suites);
    assign_room(1883, block_way_suites);
    assign_room(1880, block_way_suites);
    assign_room(1881, block_way_suites);
    assign_room(1882, block_way_suites);
    assign_room(2358, block_way_suites);
    assign_room(4920, play_cards);
    assign_room(6133, play_cards);
    assign_room(3165, play_cards);
    assign_room(3166, play_cards);
    assign_room(2135, play_cards);
    assign_room(2332, play_cards);
    assign_room(2397, play_cards);
    assign_room(1827, play_cards);
    assign_room(1826, play_cards);
    assign_room(1829, play_cards);
    assign_room(1828, play_cards);
    assign_room(1831, play_cards);
    assign_room(1830, play_cards);
    assign_room(1833, play_cards);
    assign_room(1832, play_cards);
    assign_room(1835, play_cards);
    assign_room(1834, play_cards);
    assign_room(1837, play_cards);
    assign_room(1836, play_cards);
    assign_room(1839, play_cards);
    assign_room(5971, play_cards);
    assign_room(4843, play_cards); /* Io's home */
    assign_room(3507, play_cards); /* Wild Knights */
    assign_room(3508, play_cards); /* Wild Knights */
    assign_room(4918, atm_machines); /* Gypsy Forets */
    assign_room(81, atm_machines);   /* Medievia Bank */
    assign_room(3801, atm_machines); /* Trellor Bank */
    assign_room(268, atm_machines);  /* Legolas' House */
    assign_room(252, atm_machines);  /* Neutrina's House */
    assign_room(258, atm_machines);  /* Sutekh's House */
    assign_room(3380, atm_machines); /* Mystical Forest */
    assign_room(7160, atm_machines); /* New Genesia */
    assign_room(87, train);
    assign_room(80, post_office);    /* Medievia */
    assign_room(7161, post_office);  /* New Genesia */
    assign_room(254, post_office);
    assign_room(2459, post_office);  /*Mystical Forest*/
    assign_room(2404, post_office);  /* Firm's office */
    assign_room(256, post_office);
    assign_room(1254, post_office);
    /* assign_room(2551, magic_cloud); */

    assign_room(897, u_store_it);  /* MIKES HOME */
    assign_room(74, u_store_it);   /* Medievia */
    assign_room(1288, u_store_it);
    assign_room(490, u_store_it); 
    assign_room(269, u_store_it);  
    assign_room(834, u_store_it);  /* New Genesia */
    assign_room(2572, u_store_it);
    assign_room(2612, u_store_it);
    assign_room(3016, u_store_it);
    assign_room(251, u_store_it);
    assign_room(255, u_store_it);
/*
    for(i=707;i<=743;i++)
		assign_room(i, river_current);
    for(i=749;i<=758;i++)
		assign_room(i, river_current);
    for(i=764;i<=773;i++)
		assign_room(i, river_current);
    for(i=6215;i<=6225;i++)
		assign_room(i, river_current);
*/
    assign_room(774, waterfall);
    assign_room(4874, death_trap); /* Lake Contnea */
    assign_room(2610, tear_fountain);
    assign_room(727, river_tree);
    assign_room(2823, mythago_wood);

    assign_room(25, convention);
    assign_room(204, convention);

    /* Bloodstone */
    assign_room(813, silver_shrine);
    assign_room(3596, silver_shrine);

    /* For Cliff of Dahnakriss */
    assign_room(572, moongate);
    assign_room(15051, feeders);
    assign_room(15053, feeders);
    assign_room(6330, illusionary_room);
    assign_room(6324, illusionary_room);
    assign_room(15067, pit_traps);
    assign_room(973, pit_traps);
    assign_room(15060, pit_traps);

    assign_room(10034, horneg_sarcophogus_room);
    assign_room(10038, horneg_mage_room);

    assign_room(3053, pit_traps); /* fire giant pit trap*/
    assign_room(3054, pit_traps); /* fire giant pit trap*/
    assign_room(3058, pit_traps); /* fire giant pit trap*/
    assign_room(3061, pit_traps); /* fire giant pit trap*/

    assign_room(5164, Spirited_Heights_DT);
    assign_room(2784, death_trap);
    assign_room(5414, death_trap);
    assign_room(3351, belt_quest);

    assign_room(1139, death_trap);
    assign_room(1384, death_trap);
}
