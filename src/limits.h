/***************************************************************************
*		 MEDIEVIA CyberSpace Code and Data files		   *
*       Copyright (C) 1992, 1995 INTENSE Software(tm) and Mike Krause	   *
*			   All rights reserved				   *
***************************************************************************/

/* Public Procedures */
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);

struct title_type
{
    char *title_m;
    char *title_f;
    int exp;
};
