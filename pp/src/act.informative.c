/** 
*	\file act.informative.c
*	Informational Module provides status and informative prompts 
*
*	The purpose of this module is provide information about the player and the
*	world, using text, various types of writings or prompts.
*
*	Copyright 2005, Mary C. Huston, All rights reserved.
*	Copyright (C) 2004, Shadows of Isildur: Traithe	
*
*	The program(s) may be used and/or copied only with written
*	permission or in accordance with the terms and conditions
*	stipulated in the license from DIKU GAMMA (0.0) and SOI.
*
*	\author Mary Huston
*	\author Email:  auroness@gmail.com
*
******************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "structs.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"


#define skill_lev(val) val >= 70 ? " Master " : val >= 50 ? " Adroit " : val >= 30 ? "Familiar" : " Novice "

char	*frame_built [] = {
    "fragily-built",
    "scantly-built",
    "lightly-built",
    "typically-built",
    "heavily-built",
    "massively-built",
    "gigantically-built",
    "\n"
};


char	*verbal_intox[] = { 
	"sober", 
	"tipsy", 
	"slightly drunk", 
	"drunk", 
	"intoxicated",
	"plastered",
	"\n"};

char	*verbal_hunger[] = { 
	"starving", 
	"hungry", 
	"feeling slightly hungry", 
	"feeling peckish", 
	"quite full",
	"absolutely stuffed",
	"\n"};

char	*verbal_thirst[] = { 
	"dying of thirst", 
	"quite parched", 
	"feeling thirsty", 
	"feeling slightly thirsty",
	"nicely quenched",
	"completely sated",
	"\n"};

void do_credits (CHAR_DATA *ch, char *argument, int cmd)
{
	do_help (ch, "diku_license", 0);
	return;
}

void target_sighted (CHAR_DATA *ch, CHAR_DATA *targ)
{
	VIEWED_DATA		*sighted = NULL;

	if ( !ch || !targ )
		return;

	if ( !ch->sighted ) {
		CREATE (ch->sighted, VIEWED_DATA, 1);
		ch->sighted->target = targ;
		return;
	}
	for ( sighted = ch->sighted; sighted; sighted = sighted->next ) {
		if ( sighted->target == targ )
			return;
		if ( !sighted->next ) {
			CREATE (sighted->next, VIEWED_DATA, 1);
			sighted->next->target = targ;
			return;
		}
	}
}

char *tactical_status (CHAR_DATA *ch)
{
	CHAR_DATA	*tch;
	AFFECTED_TYPE	*af;
	static char 	status [MAX_STRING_LENGTH];
	int		i = 0;

	*status = '\0';

	if ( get_affect (ch, MAGIC_GUARD) || get_affect (ch, AFFECT_GUARD_DIR) )
		snprintf (status + strlen(status), MAX_STRING_LENGTH, " #6(guarding)#0");

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
		if ( (af = get_affect (tch, MAGIC_GUARD)) && ((CHAR_DATA *)af->a.spell.t) == ch )
			i++;
	}

	if ( i > 0 ) {
		if ( i == 1 )
			snprintf (status + strlen(status), MAX_STRING_LENGTH, " #2(guarded)#0");
		else if ( i > 1 )
			snprintf (status + strlen(status), MAX_STRING_LENGTH, " #2(guarded x %d)#0", i);
	}

	i = 0;

	if ( ch->fighting )
		i++;

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
		if ( tch == ch->fighting )
			continue;
		if ( tch->fighting == ch )
			i++;
	}

	if ( i > 0 ) {
		if ( i == 1 )
			snprintf (status + strlen(status), MAX_STRING_LENGTH, " #1(engaged)#0");
		else if ( i > 1 )
			snprintf (status + strlen(status), MAX_STRING_LENGTH, " #1(engaged x %d)#0", i);
	}

	return status;
}

void do_group (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*tch = NULL;
	CHAR_DATA	*top_leader = NULL;
	char		status [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	bool		found = FALSE;
	char		*temp_arg1 = NULL;
	char		*temp_arg2 = NULL;
	char		*temp_arg3 = NULL;
	
	argument = one_argument (argument, buf);

	if ( *buf && !strcasecmp (buf, "open") ) {
		if ( !IS_SET (ch->plr_flags, GROUP_CLOSED) ) {
			send_to_char ("Your group is already open!\n", ch);
			return;
		}
		REMOVE_BIT (ch->plr_flags, GROUP_CLOSED);
		send_to_char ("You will now allow people to follow you.\n", ch);
		return;
	}
	else if ( *buf && !strcasecmp (buf, "close") ) {
		if ( IS_SET (ch->plr_flags, GROUP_CLOSED) ) {
			send_to_char ("Your group is already closed!\n", ch);
			return;
		}
		SET_BIT (ch->plr_flags, GROUP_CLOSED);
		send_to_char ("You will no longer allow people to follow you.\n", ch);
		return;
	}

	if ( !(top_leader = ch->following) )
		top_leader = ch;

	if ( !top_leader ) {
		send_to_char ("You aren't in a group.\n", ch);
		return;
	}

	*status = '\0';
	
	temp_arg1 = char_short (top_leader);
	temp_arg2 = wound_total (top_leader);
	temp_arg3 = tactical_status (top_leader);
	snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 [%s]%s, leading:\n\n", temp_arg1, temp_arg2, temp_arg3);
	buf [2] = toupper(buf[2]);

	for ( tch = top_leader->room->people; tch; tch = tch->next_in_room ) {
		if ( tch->following != top_leader )
			continue;
		if ( !CAN_SEE (ch, tch) )
			continue;
		if ( found != FALSE )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  ",\n");
		*status = '\0';
		
		temp_arg1 = char_short (tch);
		temp_arg2 = wound_total(tch);
		temp_arg3 = tactical_status (tch);
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #5%s#0 [%s]%s", temp_arg1, temp_arg2, temp_arg3);
		found = TRUE;
	}

	strcat (buf, ".\n");

	if ( !found ) {
		send_to_char ("You aren't in a group.\n", ch);
		return;
	}

	send_to_char (buf, ch);
}

void do_point (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		arg1 [MAX_STRING_LENGTH] = { '\0' };
	char		arg2 [MAX_STRING_LENGTH] = { '\0' };
	char		distance [MAX_STRING_LENGTH] = { '\0' };
	char		buffer [MAX_STRING_LENGTH] = { '\0' };
    CHAR_DATA	*target = NULL;
    CHAR_DATA	*tch = NULL;
    ROOM_DIRECTION_DATA     *exit = NULL;
	ROOM_DATA	*room = NULL;
	int			dir = 0;
	int			range = 1;
	char		*temp_arg1 = NULL;
	char		*temp_arg2 = NULL;

	if ( !*argument ) {
		send_to_char ("Usage: point <direction> <target>\n", ch);
		return;
	}
		
		arg_splitter (2, argument, arg1, arg2);
        
        if ( !cmp_strn ("north", arg1, strlen(arg1)) )
        	dir = 0;
        else if ( !cmp_strn ("east", arg1, strlen(arg1)) )
                dir = 1;
        else if ( !cmp_strn ("south", arg1, strlen(arg1)) )
                dir = 2;
        else if ( !cmp_strn("west", arg1, strlen(arg1)) )
                dir = 3;
        else if ( !cmp_strn ("up", arg1, strlen(arg1)) )
                dir = 4;
        else if ( !cmp_strn ("down", arg1, strlen(arg1)) )
                dir = 5;
	else {
		send_to_char ("Usage: point <direction> <target>\n", ch);
		return;
	}

        if ( !PASSAGE(ch, dir)) {
        	send_to_char ("There isn't an exit in that direction.\n", ch);
            return;
        }

        room = vtor(PASSAGE(ch,dir)->to_room);        
        exit = PASSAGE(ch, dir);

       	if ( exit && IS_SET (exit->exit_info, PASSAGE_CLOSED) ) {
        	send_to_char ("Your view is blocked.\n", ch);
                return;
        }

        if ( !(target = get_char_room_vis2 (ch, room->virtual, arg2)) || !has_been_sighted (ch, target) ) {
        	exit = room->dir_option[dir];
                if ( !exit ) {
	                send_to_char ("You don't see them within range.\n", ch);
                        return;
                }
                if ( exit && IS_SET (exit->exit_info, PASSAGE_CLOSED) ) {
        	        send_to_char ("Your view is blocked.\n", ch);
                        return;
                }
                if ( room->dir_option[dir] )
	                room = vtor(room->dir_option[dir]->to_room);
                else room = NULL;
		
                if ( !(target = get_char_room_vis2 (ch, room->virtual, arg2)) || !has_been_sighted (ch, target) ) {
             		exit = room->dir_option[dir];
                        if ( !exit ) {
             	        	send_to_char ("You don't see them within range.\n", ch);
                           	return;
                        }
                        if ( exit && IS_SET (exit->exit_info, PASSAGE_CLOSED) ) {
                                send_to_char ("Your view is blocked.\n", ch);
                                return;
                        }
                        if ( room->dir_option[dir] )
                        	room = vtor(room->dir_option[dir]->to_room);
                        else room = NULL;
                        if ( !(target = get_char_room_vis2 (ch, room->virtual, arg2)) || !has_been_sighted (ch, target) ) {
                                exit = room->dir_option[dir];
                                if ( !exit ) {
                	                send_to_char ("You don't see them within range.\n", ch);
                                        return;
                                }
                    if ( IS_SET (exit->exit_info, PASSAGE_CLOSED) ) {
               	    	send_to_char ("Your view is blocked.\n", ch);
       	                return;
                    }
            	                send_to_char ("You don't see them within range.\n", ch);
                                return;
                        }
				else range = 3;
           	}
			else range = 2;
		}
		else range = 1;

        if ( !target || !CAN_SEE (ch, target) || !has_been_sighted (ch, target) ) {  
        	send_to_char ("You don't see them within range.\n", ch);
            return;
        }

	if ( range == 2 )
		snprintf (distance, MAX_STRING_LENGTH, "far ");
	else if ( range == 3 )
		snprintf (distance, MAX_STRING_LENGTH, "very far ");
	else *distance = '\0';

	snprintf (distance + strlen(distance), MAX_STRING_LENGTH, "to the %s", dirs[dir]);

	temp_arg1 = char_short (target);
	snprintf (buf, MAX_STRING_LENGTH,  "You point at #5%s#0, %s.", temp_arg1, distance);
	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

	temp_arg1 = char_short (ch);
	temp_arg2 = char_short(target);
	snprintf (buf, MAX_STRING_LENGTH,  "%s#0 points at #5%s#0, %s.", temp_arg1, temp_arg2, distance);
	*buf = toupper (*buf);

	snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
	snprintf (buf, MAX_STRING_LENGTH,  "%s", buffer);
	act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
		if ( !CAN_SEE (tch, ch) )
			continue;
		target_sighted (tch, target);
	}
}

void do_title (CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	char			echo [MAX_STRING_LENGTH] = { '\0' };
	char			titlebuf [MAX_STRING_LENGTH] = { '\0' };
	char			bookbuf [MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA		*obj = NULL;
	double			skill = 0;

	if ( !*argument ) {
		send_to_char ("What did you wish to title?\n", ch);
		return;
	}

	if ( !strstr (argument, "\"") && !strstr (argument, "\'") ) {
		send_to_char ("You must enclose the book's desired title in quotation marks.\n", ch);
		return;
	}

	if ( !ch->writes ) {
		send_to_char ("In which script would you like to write? (See the SCRIBE command.)\n", ch);
		return;
	}

	arg_splitter (2, argument, bookbuf, titlebuf);
	

	if ( !(obj = get_obj_in_list_vis (ch, bookbuf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, bookbuf, ch->left_hand)) ) {
		send_to_char ("You need to be holding the book you wish to title.\n", ch);
		return;
	}

	if ( obj->book_title && IS_MORTAL (ch) ) {
		send_to_char ("This work has already been titled.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (obj) != ITEM_BOOK ) {
		send_to_char ("This command only works with books.\n", ch);
		return;
	}

	skill = (ch->skills[ch->writes]*0.50) + (ch->skills[ch->speaks]*0.30) + (ch->skills[SKILL_LITERACY]*0.20);
	skill = (double)((int)(skill));
	skill = (double)MIN(95, (int)skill);


	if ( !*titlebuf ) {
		send_to_char ("What did you wish to title this work?\n", ch);
		return;
	}

	if ( strlen(buf) > 55 ) {
		send_to_char ("There is a 55-character limit on book titles.\n", ch);
		return;
	}

	obj->book_title = add_hash(buf);
	obj->title_skill = (int)(skill);
	obj->title_script = ch->writes;
	obj->title_language = ch->speaks;

	snprintf (echo, MAX_STRING_LENGTH, "You have entitled #2%s#0 '%s'.", obj->short_description, titlebuf);
	act (echo, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	
}


void do_timeconvert (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	int		hour = 0, day = 0, month = 0, year = 0;
	long double	ic_hours = 0, curr_hours = 0, hour_difference = 0;
	long double	second_difference = 0;
	time_t		projected_time = 0;

	if ( !ch->desc || !ch->desc->account ) {
		send_to_char ("Only PCs can use this command.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	hour = strtol (buf, NULL, 10);

	if ( hour > 23 || hour < 1 ) {
		send_to_char ("You must specify an hour between 1 and 23.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	day = strtol (buf, NULL, 10);

	if ( day > 30 || day < 1 ) {
		send_to_char ("You must specify a day between 1 and 30.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	month = strtol (buf, NULL, 10);

	if ( month > 12 || month < 1 ) {
		send_to_char ("You must specify a month between 1 and 12.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);
	year = strtol (buf, NULL, 10);

	if ( year > 2600 || year < 2460 ) {
		send_to_char ("You must specify a year between 2460 and 2600.\n", ch);
		return;
	}
	
	ic_hours += hour;
	ic_hours += day * 24;
	ic_hours += month * 30 * 24;
	ic_hours += year * 12 * 30 * 24;

	curr_hours += (time_info.hour+1);
	curr_hours += (time_info.day+1) * 24;
	curr_hours += (time_info.month+1) * 30 * 24;	
	curr_hours += time_info.year * 12 * 30 * 24;

	hour_difference = ic_hours - curr_hours;

	if ( hour_difference < 0 ) {
		send_to_char ("You cannot convert a time that has already passed.\n", ch);
		return;
	}

	second_difference = hour_difference * 60 * 60;
	second_difference /= 4;

	second_difference += 5 * 60 * 60;			/* Adjusting server's EST clock to GMT. */
	second_difference += ch->desc->account->timezone * 60 * 60;

	projected_time = time(0);
	projected_time += second_difference;

	(void)strftime(buf, 255, "In your timezone, the specified in-game time will fall at or near %I:%M %P, %A %B %e %Y.", localtime(&projected_time));

	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

}


void post_message (DESCRIPTOR_DATA *d);

int armor_total (CHAR_DATA *ch)
{
	OBJ_DATA	*armor = NULL;
	int		total = 0;

	for ( armor = ch->equip; armor; armor = armor->next_content )
		if ( GET_ITEM_TYPE(armor) == ITEM_ARMOR )
			total += armor->o.od.value[0];
	
	return total;
}

char *tilde_eliminator (char *string)
{
	char		*p = NULL;

	while ( (p = strchr (string, '~')) )
		*p = '-';

	return str_dup (string);
}

char *carry_desc (OBJ_DATA *obj)
{
	static char	buf [10] = { '\0' };

	if ( obj->location == WEAR_PRIM || obj->location == WEAR_SEC ||
	     obj->location == WEAR_BOTH )
	     snprintf(buf, 10, "wielding");
		
	else if ( obj->location == WEAR_SHIELD )
		snprintf(buf, 10, "gripping");

	else 
		snprintf(buf, 10, "carrying");
	
	return buf;
}

char *mana_bar (CHAR_DATA *ch)
{
	static 	char buf [25] = { '\0' };
	float	calc = 0;
	float	move = 0;

	if ( ch->mana >= ch->max_mana )
		snprintf (buf, 20,  "#C******#0");

	if ( (move = (float)ch->mana) >= (calc = ch->max_mana * .6667)
	    && ch->mana < ch->max_mana)
		snprintf (buf, 20,  "#C*****#0 ");

	if ( (move = (float)ch->mana) >= (calc = ch->max_mana * .5)
	    && (move = (float)ch->mana) < (calc = ch->max_mana * .6667))
		snprintf (buf, 20,  "#C****#0  ");

	if ( (move = (float)ch->mana) >= (calc = ch->max_mana * .3333)
	    && (move = (float)ch->mana) < (calc = ch->max_mana * .5))
		snprintf (buf, 20,  "#C***#0   ");

	if ( (move = (float)ch->mana) >= (calc = ch->max_mana * .1667)
	    && (move = (float)ch->mana) < (calc = ch->max_mana * .3333))
		snprintf (buf, 20,  "#C**#0    ");

	if ( (move = (float)ch->mana) >= (calc = ch->max_mana * .0001)
	    && (move = (float)ch->mana) < (calc = ch->max_mana * .1667))
		snprintf (buf, 20,  "#C*#0     ");

	if ( ch->mana == 0)
		snprintf (buf, 20,  "       ");

	return buf;
}

char *fatigue_bar (CHAR_DATA *ch)
{
	static 	char buf [25] = { '\0' };
	float	calc = 0;
	float	move = 0;

	if ( ch->move >= ch->max_move )
		snprintf (buf, 20,  "#1||#3||#2||#0");

	if ( (move = (float)ch->move) >= (calc = ch->max_move * .6667)
	    && ch->move < ch->max_move)
		snprintf (buf, 20,  "#1||#3||#2|#0 ");

	if ( (move = (float)ch->move) >= (calc = ch->max_move * .5)
	    && (move = (float)ch->move) < (calc = ch->max_move * .6667))
		snprintf (buf, 20,  "#1||#3||#0  ");

	if ( (move = (float)ch->move) >= (calc = ch->max_move * .3333)
	    && (move = (float)ch->move) < (calc = ch->max_move * .5))
		snprintf (buf, 20,  "#1||#3|#0   ");

	if ( (move = (float)ch->move) >= (calc = ch->max_move * .1667)
	    && (move = (float)ch->move) < (calc = ch->max_move * .3333))
		snprintf (buf, 20,  "#1||#0    ");

	if ( (move = (float)ch->move) >= (calc = ch->max_move * .0001)
	    && (move = (float)ch->move) < (calc = ch->max_move * .1667))
		snprintf (buf, 20,  "#1|#0     ");

	if ( ch->move == 0)
		snprintf (buf, 20,  "       ");


	return buf;
}

char *breath_bar (CHAR_DATA *ch)
{
	AFFECTED_TYPE	*af;
	static 		char buf [25] = { '\0' };
	int		current = 0, max = 0;
	float		calc = 0;
	float		move = 0;

	if ( !(af = get_affect (ch, AFFECT_HOLDING_BREATH)) )
		return NULL;

	current = af->a.spell.duration;
	max = af->a.spell.sn;

	if ( current >= max )
		snprintf (buf, 20,  "#6******#0");

	else if ( (move = (float)current) >= (calc = max * .6667)
	    && current < max)
		snprintf (buf, 20,  "#6*****#0 ");

	else if ( (move = (float)current) >= (calc = max * .5)
	    && (move = (float)current) < (calc = max * .6667))
		snprintf (buf, 20,  "#6****#0  ");

	else if ( (move = (float)current) >= (calc = max * .3333)
	    && (move = (float)current) < (calc = max * .5))
		snprintf (buf, 20,  "#6***#0   ");

	else if ( (move = (float)current) >= (calc = max * .1667)
	    && (move = (float)current) < (calc = max * .3333))
		snprintf (buf, 20,  "#6**#0    ");

	else if ( (move = (float)current) >= (calc = max * .0001)
	    && (move = (float)current) < (calc = max * .1667))
		snprintf (buf, 20,  "#6*#0     ");

	else
		snprintf (buf, 20,  "      ");


	return buf;
}

/* Procedures related to 'look' */

char *find_ex_description (char *word, XTRA_DESCR_DATA *list)
{
	XTRA_DESCR_DATA	*i = NULL;

	for ( i = list; i; i = i->next )
		if ( name_is (word, i->keyword) )
			return (i->description);

	return NULL;
}

void do_compare (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*tch = NULL, *tch2 = NULL;
	char		buf[250] = { '\0' };
	char		arg1[MAX_STRING_LENGTH] = { '\0' };
	char		target2[MAX_STRING_LENGTH] = { '\0' };
	char		*temp_arg1 = NULL;
	char		*temp_arg2 = NULL;

	*target2 = '\0';
	*arg1 = '\0';
	*buf = '\0';

	argument = one_argument (argument, arg1);

      

    if (!(tch = get_char_room_vis (ch, arg1))) {
		send_to_char ("Compare who?\n", ch);
		return;
	}

	(void)one_argument (argument, target2);

	if ( *target2 ) {
		if ( !(tch2 = get_char_room_vis (ch, target2)) ){
			send_to_char ("I don't see the second target.\n", ch);
			return;
		}
	}

	if ( tch2 == ch ) {
		send_to_char ("Compare yourself to yourself? Eh?\n", ch);
		return;
	}

	if ( !tch2 && tch == ch ) {
		send_to_char ("Compare yourself to yourself? Eh?\n", ch);
		return;
	}

	if ( tch == tch2 ) {
		send_to_char ("Comparing something to itself? Brilliant!\n", ch);
		return;
	}

	if ( !tch2 ) {
		if ( tch->height >= ch->height ) {
			if ( (float)tch->height/(float)ch->height > 2.4 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is nearly three times your height!", temp_arg1, frame_built[tch->frame]);
			}
			
			else if ( (float)tch->height/(float)ch->height > 1.7 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is nearly twice your height!", temp_arg1, frame_built[tch->frame]);
			}
			
			else if ( (float)tch->height/(float)ch->height > 1.4 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and towers above you.", temp_arg1, frame_built[tch->frame]);
			}
			
			else if ( (float)tch->height/(float)ch->height > 1.15 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is considerably taller than you.", temp_arg1, frame_built[tch->frame]);
			}
			
			else if ( (float)tch->height/(float)ch->height > 1.06 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is taller than you.", temp_arg1, frame_built[tch->frame]);
			}
			
			else if ( (float)tch->height/(float)ch->height > 1.03 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is a bit taller than you.", temp_arg1, frame_built[tch->frame]);
			}
			
			else{
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and roughly the same height as you.", temp_arg1, frame_built[tch->frame]);
			}
		}
		else if ( tch->height < ch->height ) {
			if ( (float)ch->height/(float)tch->height > 2.4 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is about a third of your height.", temp_arg1, frame_built[tch->frame]);
			}
			
			else if ( (float)ch->height/(float)tch->height > 1.7 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is roughly half your height.", temp_arg1, frame_built[tch->frame]);
			}
			
			else if ( (float)ch->height/(float)tch->height > 1.4 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and you tower over #5%s#0.", temp_arg1, frame_built[tch->frame], !is_hooded(tch) ? HMHR(tch) : 
"it");
			}
			
			else if ( (float)ch->height/(float)tch->height > 1.15 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and you are considerably taller than #5%s#0.", temp_arg1, frame_built[tch->frame], !is_hooded(tch) ? HMHR(tch) : 
"it");
			}
			
			else if ( (float)ch->height/(float)tch->height > 1.06 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and you are taller than #5%s#0.", temp_arg1, frame_built[tch->frame], !is_hooded(tch) ? HMHR(tch) : 
"it");
			}
			
			else if ( (float)ch->height/(float)tch->height > 1.03 ){
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and you are a bit taller than #5%s#0.", temp_arg1, frame_built[tch->frame], !is_hooded(tch) ? HMHR(tch) : 
"it");
			}
			
			else{
				temp_arg1 = char_short(tch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and roughly the same height as you.", temp_arg1, frame_built[tch->frame]);
			}
		}
	}

	else {
		if ( tch->height >= tch2->height ) {
			if ( (float)tch->height/(float)tch2->height > 2.4 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is nearly three times #5%s#0's height!", temp_arg1, frame_built[tch->frame], temp_arg2);
			}

			else if ( (float)tch->height/(float)tch2->height > 1.7 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is nearly twice #5%s#0's height!", temp_arg1, frame_built[tch->frame], temp_arg2);
			}
			
			else if ( (float)tch->height/(float)tch2->height > 1.4 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and towers above #5%s#0.", temp_arg1, frame_built[tch->frame], temp_arg2);
			}
			
			else if ( (float)tch->height/(float)tch2->height > 1.15 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is considerably taller than #5%s#0.", temp_arg1, frame_built[tch->frame], temp_arg2);
			}
			
			else if ( (float)tch->height/(float)tch2->height > 1.06 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is taller than #5%s#0.", temp_arg1, frame_built[tch->frame], temp_arg2);
			}	
			
			else if ( (float)tch->height/(float)tch2->height > 1.03 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is a bit taller than #5%s#0.\n", temp_arg1, frame_built[tch->frame], temp_arg2);
			}
			
			else{
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and roughly the same height as #5%s#0.", temp_arg1, frame_built[tch->frame], temp_arg2);
			}
		}
		if ( tch->height < tch2->height ) {
			if ( (float)tch2->height/(float)tch->height > 2.4 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is about a third of #5%s#0's height.", temp_arg1, frame_built[tch->frame], temp_arg2);
			}
			
			else if ( (float)tch2->height/(float)tch->height > 1.7 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and is roughly half #5%s#0's height.", temp_arg1, frame_built[tch->frame], temp_arg2);
			}
			
			else if ( (float)tch2->height/(float)tch->height > 1.4 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and #5%s#0 towers over #5%s#0.", temp_arg1, frame_built[tch->frame], temp_arg2,
					!is_hooded(tch) ? HMHR(tch) : "it");
			}
			
			else if ( (float)tch2->height/(float)tch->height > 1.15 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and #5%s#0 is considerably taller than #5%s#0.", temp_arg1, frame_built[tch->frame], temp_arg2, !is_hooded(tch) ? HMHR(tch) : "it");
			}
			
			else if ( (float)tch2->height/(float)tch->height > 1.06 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and #5%s#0 is taller than #5%s#0.", temp_arg1, frame_built[tch->frame], temp_arg2,
					!is_hooded(tch) ? HMHR(tch) : "it");
			}
			
			else if ( (float)tch2->height/(float)tch->height > 1.03 ){
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and #5%s#0 is a bit taller than #5%s#0.",temp_arg1, frame_built[tch->frame], temp_arg2,
					!is_hooded(tch) ? HMHR(tch) : "it");
			}
			else{
				temp_arg1 = char_short(tch);
				temp_arg2 = char_short(tch2);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 is %s, and roughly the same height as #5%s#0.", temp_arg1, frame_built[tch->frame], temp_arg2);
			}
		}
	}

	act(buf,FALSE,ch,0,0,TO_CHAR | TO_ACT_FORMAT);

}

char *writing_adj (int skill)
{
	if ( skill <= 10 )
		return "crudely";
	if ( skill <= 20 )
		return "poorly";
	if ( skill <= 30 )
		return "functionally";
	if ( skill <= 40 )
		return "with skill";
	if ( skill <= 50 )
		return "with great skill";
	if ( skill <= 60 )
		return "artfully";
	if ( skill <= 70 )
		return "beautifully";
	return "flawlessly";
}

int decipher_script (CHAR_DATA *ch, int script, int language, int skill)
{
	double		check = 0;
	

	if ( !real_skill (ch, script) || !real_skill (ch, language) )
		return 0;

        if ( skill > 0 && skill <= 15 )
                check = 70;
        else if ( skill > 15 && skill < 30 )
                check = 50;
        else if ( skill >= 30 && skill < 50 )
                check = 30;
        else if ( skill >= 50 && skill < 70 )
                check = 20;
        else if ( skill >= 70 )
                check = 10;
        
	skill_use (ch, script, 0);
	skill_use (ch, language, 0);
	skill_use (ch, SKILL_LITERACY, 0);

        if ( ((ch->skills [script]*.50) + (ch->skills[language]*.30) + (ch->skills[SKILL_LITERACY]*.20)) >= check )
                return 1;
        else return 0;
}

void reading_check (CHAR_DATA *ch, OBJ_DATA *obj, WRITING_DATA *writing, int page)
{
	static char		output [MAX_STRING_LENGTH] = { '\0' };
	char	*temp_arg = NULL;
	
	if ( !writing || !writing->message ) {
		send_to_char ("There seems to be a problem with this object. Please report it to a staff member.\n", ch);
		return;
	}

	if ( !ch->skills[writing->script] && str_cmp (writing->author, ch->tname) ) {
		snprintf (output, MAX_STRING_LENGTH,  "This document is written in a script entirely unfamiliar to you.");
		act (output, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( !ch->skills[writing->language] && real_skill (ch, writing->script) && str_cmp (writing->author, ch->tname) ) {
		snprintf (output, MAX_STRING_LENGTH,  "Although you recognise the script as %s, the language in which this document is written is unknown to you.", skill_data[writing->script].skill_name);
		act (output, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( !decipher_script(ch, writing->script, writing->language, writing->skill) && (str_cmp (writing->author, ch->tname) || !real_skill (ch, writing->script)) ) {
		snprintf (output, MAX_STRING_LENGTH,  "You find that you can make neither heads nor tails of this document.");
		act (output, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( !page ) {
		temp_arg = writing_adj(writing->skill);
		snprintf (output, MAX_STRING_LENGTH,  "#2On %s#0, %s sigils, scribed %s in %s, bear a message in the %s tongue:",
		obj->short_description, skill_data[writing->script].skill_name, temp_arg,
		writing->ink, skill_data[writing->language].skill_name);
	}
	else {
		temp_arg = writing_adj(writing->skill);
		snprintf (output, MAX_STRING_LENGTH,  "On #2page %d#0, %s sigils, scribed %s in %s, bear a message in the %s tongue:", 
		page-1, skill_data[writing->script].skill_name, temp_arg,
		writing->ink, skill_data[writing->language].skill_name);
	}

	act (output, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	send_to_char ("\n", ch);

	if ( str_cmp (writing->author, ch->tname) ) {
		skill_use (ch, writing->script, 0);
		if ( !number(0,1) )
			skill_use (ch, writing->language, 0);
		if ( !number(0,2) )
			skill_use (ch, SKILL_LITERACY, 0);
	}

	snprintf (output, MAX_STRING_LENGTH,  "%s", writing->message);
	page_string (ch->desc, output);
}

char *article (char *string)
{
	if ( !str_cmp (string, "something") )
		return "";

	if ( *string == 'a' || *string == 'e' || *string == 'i' ||
		*string == 'o' || *string == 'u' )
		return "an ";
	else return "a ";
}

void show_obj_to_char (OBJ_DATA *obj, CHAR_DATA *ch, int mode)
{
	int			found = 0;
	int			crafts_found = 0;
	int			first_seen, i = 0;
	WRITING_DATA	*writing = NULL;
	CHAR_DATA		*tch = NULL;
	OBJ_DATA		*obj2 = NULL;
	OBJ_DATA		*tobj = NULL;
	WOUND_DATA		*wound = NULL;
	LODGED_OBJECT_INFO	*lodged = NULL;
	AFFECTED_TYPE		*af = NULL;
	char		*p = NULL;
	char		output [MAX_STRING_LENGTH] = { '\0' };
	char		buffer [MAX_STRING_LENGTH] = { '\0' };
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		buf2 [MAX_STRING_LENGTH] = { '\0' };
	char		*temp_arg = NULL;
	char		*temp_arg1 = NULL;
	char		*temp_arg2 = NULL;
	
	*buffer = '\0';

/** mode 0 is for omote strings
	mode 1 lanter sdesc with details
	mode 2 is unused
	mode 3 is for invisible things
	mode 4 is unused
	mode 5 needs some skill to see things
	mode 6 is in the dark
	mode 7 is ldesc
**/

	if ((mode == 0) && obj->description) { /** omote string **/

		if ( IS_TABLE (obj) )
			return;

		strcat (buffer, "#2");
		
		if(obj->omote_str) {
		    snprintf (buf, MAX_STRING_LENGTH,  "%s %s", obj->short_description,
			   obj->omote_str);
		    *buf = toupper(*buf);
		    reformat_string (buf, &p);
		    strcat(buffer, p);
		    mem_free (p);
		} 
		else{
			temp_arg = obj_desc (obj);
		    strcat (buffer, temp_arg);
	    }

		if ( get_obj_affect (obj, MAGIC_HIDDEN) )
			strcat (buffer, " (hidden from view)");

		if ( IS_SET (obj->obj_flags.extra_flags, ITEM_VNPC) )
			strcat (buffer, " #6(vNPC)#0");

		if ( GET_ITEM_TYPE (obj) == ITEM_LIGHT &&
			 obj->o.light.hours &&
			 obj->o.light.on )
			strcat (buffer, " #1(lit)#0");

		strcat (buffer, "#0");
		reformat_string (buffer, &p);
		snprintf (buffer, MAX_STRING_LENGTH,  "%s", p);
		mem_free (p);
		buffer [strlen(buffer)-1] = '\0';
	} /** omote string **/

	if ( mode == 7 && obj->description ) { /** Ldesc **/
		strcpy (buffer, "#2");
		temp_arg = obj_desc (obj);
		strcat (buffer, temp_arg);
		strcat (buffer, "#0");
		reformat_string (buffer, &p);
		snprintf (buffer, MAX_STRING_LENGTH,  "%s", p);
		buffer [strlen(buffer)-1] = '\0';
	}


	else if (obj->short_description && ((mode == 1) ||
	      (mode == 2) || (mode==3) || (mode == 4))) { /** short desc **/

		strcpy (buffer, "#2");
		temp_arg = obj_short_desc (obj);
		strcat (buffer, temp_arg);
		strcat (buffer, "#0");

		if ( IS_SET (obj->obj_flags.extra_flags, ITEM_DESTROYED) )
			strcat (buffer, " #1(destroyed)#0");

		if ( GET_TRUST (ch) &&
			 (GET_ITEM_TYPE(obj) == ITEM_MONEY) )
			snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  " (%d)", obj->count);
	}

	else if ( mode == 5 ) { /** need some kind of skill **/

		if ( obj->obj_flags.type_flag == ITEM_PARCHMENT ) {
			if ( !obj->writing_loaded )
				load_writing (obj);
				
			if ( obj->writing && obj->writing->message ) {
				if ( !IS_MORTAL (ch) ) {
					snprintf (buf, MAX_STRING_LENGTH,  "#2[Penned by %s on %s.]#0\n\n", obj->writing->author, obj->writing->date);
					send_to_char (buf, ch);
				}
				
				if ( obj->carried_by && obj->carried_by != ch && IS_MORTAL(ch)) {
					send_to_char ("You aren't quite close enough to make out the words.\n", ch);
					return;	
				}
				
				reading_check(ch, obj, obj->writing, 0);
				return;
			} 
			else {
				snprintf (buf, MAX_STRING_LENGTH,  "%s#0 seems to be blank.", obj->short_description);
				snprintf (buffer, MAX_STRING_LENGTH,  "#2%s\n", CAP(buf));
				return;
			}
		} /*if ( obj->obj_flags.type_flag == ITEM_PARCHMENT ) */

		else if ( obj->obj_flags.type_flag == ITEM_BOOK ) {
			if ( !obj->writing_loaded )
				load_writing (obj);
			
			if ( !obj->open ) {
				if ( obj->full_description && *obj->full_description )
					strcat (buffer, obj->full_description);
				else{
					if (CAN_SEE_OBJ((ch), (obj)))
						temp_arg = obj_short_desc (obj);
					else
						temp_arg = "somethng";
					snprintf (buffer, MAX_STRING_LENGTH,  "   It is #2%s#0.\n", temp_arg);
				}
				
				snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6This book seems to have %d pages remaining.#0\n", obj->o.od.value[0]);
				
				if ( obj->book_title ) {
					if ( decipher_script (ch, obj->title_script, obj->title_language, obj->title_skill) )
						snprintf (buf, MAX_STRING_LENGTH,  "   #6The book has been entitled '%s.'#0\n", obj->book_title);
					else 
						snprintf (buf, MAX_STRING_LENGTH,  "   #6You cannot quite decipher what seems to be this book's title.#0\n");
					
					reformat_string (buf, &p);
					strcat (buffer, "\n");
					strcat (buffer, p);
					mem_free (p);
				}
				page_string (ch->desc, buffer);
				return;
			}
			
			if ( obj->carried_by && obj->carried_by != ch && IS_MORTAL(ch)) {
				send_to_char ("You aren't quite close enough to make out the words.\n", ch);
				return;	
			}		

			if ( !obj->writing || !obj->o.od.value[0] ) {
				send_to_char ("All its pages have been torn out.\n", ch);
				return;
			}

			for ( i = 2, writing = obj->writing; i <= obj->open; i++ ) {
				if ( !writing->next_page )
					break;
				writing = writing->next_page;
			}
			
			if ( writing && writing->message ) {
				if ( str_cmp (writing->message, "blank") ) {
					if ( !IS_MORTAL (ch) ) {
						snprintf (buf, MAX_STRING_LENGTH,  "#2[Penned by %s on %s.]#0\n\n", writing->author, writing->date);
						send_to_char (buf, ch);
					}
					reading_check(ch, NULL, writing, i);
					return;
				} 
				else {
					snprintf (buf, MAX_STRING_LENGTH,  "This page seems to be blank.");
					snprintf (buffer, MAX_STRING_LENGTH,  "%s\n", CAP(buf));
					send_to_char (buffer, ch);	
					return;
				}
			} 
			else {
				snprintf (buf, MAX_STRING_LENGTH,  "This page seems to be blank.");
				snprintf (buffer, MAX_STRING_LENGTH,  "%s\n", CAP(buf));
				send_to_char (buffer, ch);	
				return;
			}
		} /*else if ( obj->obj_flags.type_flag == ITEM_BOOK ) */

		else {
			if ( obj->full_description && *obj->full_description )
				strcpy (buffer, obj->full_description);
			else {
				if (CAN_SEE_OBJ((ch), (obj)))
						temp_arg = obj_short_desc (obj);
					else
						temp_arg = "somethng";
				snprintf (buffer, MAX_STRING_LENGTH,  "   It is #2%s#0.\n", temp_arg);
			}
	
			temp_arg1 = show_damage (obj);
			snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "%s", temp_arg1);
			
			if ( IS_WEARABLE (obj) && obj->size ) {
	                        snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  "\n   #6This garment would fit individuals wearing size %s.#0", sizes_named [obj->size]);
			}

			if ( GET_ITEM_TYPE (obj) == ITEM_WEAPON ) {
				snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6You'd guess it to be a %s weapon.#0\n", skill_data[obj->o.weapon.use_skill].skill_name);

				if ( IS_SET (obj->obj_flags.extra_flags, ITEM_THROWING) )
					snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6This weapon appears ideally suited for throwing.#0\n");
			}
			else if ( GET_ITEM_TYPE (obj) == ITEM_TENT ) {
				snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6You'd guess this tent could shelter %d individual%s.#0\n", obj->o.od.value[1], obj->o.od.value[1] != 1 ? "s" : "");
			}
			else if ( GET_ITEM_TYPE (obj) == ITEM_HEALER_KIT ) {
				if ( ch->skills [SKILL_HEALING] && ch->skills [SKILL_HEALING] >= obj->o.od.value[2] ) {
					snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6Used to Treat:#0 ");
					if ( IS_SET (obj->o.od.value[5], TREAT_ALL) || !obj->o.od.value[5] )
						snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " Any");
					else {
						if ( IS_SET (obj->o.od.value[5], TREAT_SLASH) )
							snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " Slashes");
						if ( IS_SET (obj->o.od.value[5], TREAT_PUNCTURE) )
							snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " Punctures");
						if ( IS_SET (obj->o.od.value[5], TREAT_BLUNT) )
							snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " Contusions");
						if ( IS_SET (obj->o.od.value[5], TREAT_BURN) )
							snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " Burns");
						if ( IS_SET (obj->o.od.value[5], TREAT_FROST) )
							snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " Frostbite");
					}
					snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6Uses Remaining:#0 %d\n", obj->o.od.value[0]);
				}
				else {
					snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6Your training is not yet sufficient for this remedy.#0\n");
				}
			}
			else if ( GET_ITEM_TYPE (obj) == ITEM_REPAIR_KIT ) {
				if ( obj->o.od.value[3] < 0 || obj->o.od.value[5] < 0) { 
					snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #1Error, No item type or skill associated with repair kit!#0");
				}
				else if ( obj->o.od.value[3] && (!ch->skills [(obj->o.od.value[3])] || ch->skills [(obj->o.od.value[3])] < obj->o.od.value[2] )) {
					snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6Your training is not yet sufficient to use these materials.#0\n");
				}
				else {
					snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6Used to Repair:#0 ");
					if ( obj->o.od.value[5] == 0 ) {
						snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #6Any item#0 made with ");
					}
					else {
						snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #6%s items#0 made with ", item_types[(obj->o.od.value[5])]) ;
					}
					if ( obj->o.od.value[3] == 0 ) {
						snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "#6Any skill#0.");
					}
					else {
						snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "the #6%s skill#0.", skill_data[(obj->o.od.value[3])].skill_name) ;
					}
					snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   #6Uses Remaining:#0  %d\n", obj->o.od.value[0]);
				}
			}

			*buf = '\0';

			for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ ) {
				if ( !(af = get_affect (ch, i)) )
					continue;
				if ( !af->a.craft || !af->a.craft->subcraft )
					continue;
				if ( !craft_uses (af->a.craft->subcraft, obj->virtual) )
					continue;
				if ( crafts_found )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  ", ");
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "'%s %s'", af->a.craft->subcraft->command, af->a.craft->subcraft->subcraft_name);
				crafts_found += 1;
			}

			if ( crafts_found ) {
				snprintf (output, MAX_STRING_LENGTH,  "   You realize that you could make use of this item in the following craft%s: %s.", crafts_found != 1 ? "s" : "", buf); 
				reformat_string (output, &p);
				snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n%s", p);
				mem_free (p);
				p = NULL;
			}

			if ( obj->wounds ) {
				snprintf (buf, MAX_STRING_LENGTH,  "   It has ");
				for ( wound = obj->wounds; wound; wound = wound->next ) {
					if ( !wound->next && wound != obj->wounds ){
						temp_arg1 = expand_wound_loc(wound->location);
						snprintf (buf2, MAX_STRING_LENGTH,  "and a %s %s on the %s", wound->severity, wound->name, temp_arg1);
					}
					else{
						temp_arg1 = expand_wound_loc(wound->location);
						snprintf (buf2, MAX_STRING_LENGTH,  "a %s %s on the %s", wound->severity, wound->name, temp_arg1);
					}
					strcat (buf, buf2);
					if ( !wound->next )
						strcat (buf, ".");
					else
						strcat (buf, ", ");
				}
				reformat_string (buf, &p);
				strcat (buffer, "\n");
				strcat (buffer, p);
				mem_free (p);
				p = NULL;
			}

			if ( obj->lodged ) {
				snprintf (buf, MAX_STRING_LENGTH,  "   It has ");
				for ( lodged = obj->lodged; lodged; lodged = lodged->next ) {
					tobj = load_object (lodged->vnum);
					if ( !tobj )
						continue;	
					if ( !lodged->next && lodged != obj->lodged ){
						temp_arg1 = expand_wound_loc(lodged->location);
						snprintf (buf2, MAX_STRING_LENGTH,  "and #2%s#0 lodged in the %s", tobj->short_description, temp_arg1);
					}
					else{
						temp_arg1 = expand_wound_loc(lodged->location);
						snprintf (buf2, MAX_STRING_LENGTH,  "#2%s#0 lodged in the %s", tobj->short_description, temp_arg1 );
					}
					strcat (buf, buf2);
					if ( !lodged->next )
						strcat (buf, ".");
					else
						strcat (buf, ", ");
					extract_obj (tobj);
				}
				reformat_string (buf, &p);
				if ( obj->wounds )
					strcat (buffer, "\n");
				strcat (buffer, p);
				mem_free (p);
				p = NULL;
			}

			first_seen = 1;

			if ( IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE) ) {
				for ( obj2 = obj->contains;
					  obj2;
					  obj2 = obj2->next_content ) {

					if ( CAN_SEE_OBJ (ch, obj2) ) { /* items on table */

						if ( first_seen ){ /* see the table? */
							if (CAN_SEE_OBJ((ch), (obj)))
								temp_arg = obj_short_desc (obj);
							else
								temp_arg = "somethng";
							snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  "\nOn #2%s#0 you see:\n", temp_arg);
						}

						first_seen = 0;
						
						if (CAN_SEE_OBJ((ch), (obj2))) /* things on the table */
							temp_arg = obj_short_desc (obj2);
						else
							temp_arg = "somethng";
						
						snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  "   %s\n", temp_arg);
					}
				}

				for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
				}
			}
			if ( obj->virtual != VNUM_CORPSE ) {
/*
				if ( obj->item_wear == 100 )
					snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  "\n   It appears to be in flawless condition.\n");
*/
			}
		}
	}

	else if ( mode == 6 ) { /** in the dark **/

		strcpy (buffer, "   ");

		if ( CAN_SEE_OBJ (ch, obj) && IS_OBJ_STAT (obj, ITEM_INVISIBLE) )
			strcat (buffer, "(invis) ");
		
		temp_arg1 = obj_short_desc (obj);
		snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  "#2%s#0", temp_arg1);

		if ( GET_TRUST (ch) &&
			 (GET_ITEM_TYPE(obj) == ITEM_MONEY) )
			snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  " (%d)", obj->count);
	}

	if (mode != 3) {
		found = FALSE;
		if (IS_OBJ_STAT(obj,ITEM_INVISIBLE)) {
			 strcat(buffer,"(invisible)");
			 found = TRUE;
		}
	}

	if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_LIGHT ) {
		if ( obj->o.light.hours && obj->o.light.on )
			snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  " #1(lit)#0");
		else if ( obj->o.light.hours && !obj->o.light.on )
			snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  " #1(unlit)#0");
		else
			snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  " #1(spent)#0");
	}

	if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->book_title && decipher_script (ch, obj->title_script, obj->title_language, obj->title_skill) )
		snprintf (buffer, MAX_STRING_LENGTH,  "#2\"%s\"#0", obj->book_title);
	if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->open )
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #1(open)#0");
	else if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_BOOK && !obj->open )
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #1(closed)#0");

	if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_WEAPON && (obj->contains || obj->loaded) )
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #6(loaded)#0");

	if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_SHEATH &&
	     obj->contains ){
		temp_arg1 = see_object_in (ch, obj->contains);
		temp_arg2 = article (temp_arg1);
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " bearing %s#2%s#0", temp_arg2, temp_arg1);

	}
	
	if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_QUIVER && obj->contains ){
		temp_arg1 = see_object_in (ch, obj->contains);
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " bearing #2%ss#0", temp_arg1);
	}
	
	if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
	     obj->o.od.value[0] == obj->o.od.value[1] )
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #1(full)#0");
	else if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
	     obj->o.od.value[0] > obj->o.od.value[1]/2 )
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #1(mostly full)#0");
	else if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
	     obj->o.od.value[0] == obj->o.od.value[1]/2 )
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #1(half full)#0");
	else if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
	     obj->o.od.value[0] < obj->o.od.value[1]/2 && obj->o.od.value[0] > 0 )
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #1(mostly empty)#0");
	else if ( mode == 1 && GET_ITEM_TYPE (obj) == ITEM_INK &&
	     obj->o.od.value[0] <= 0 )
		snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH,  " #1(empty)#0");

	if ( buffer[strlen(buffer)-1] != '\n' )
		strcat (buffer, "\n");

	page_string(ch->desc, buffer);

}

void list_obj_to_char (OBJ_DATA *list, CHAR_DATA *ch, int mode, int show)
{
	OBJ_DATA	*i = NULL;
	int		found = 0, j = 0;
	int		looked_for_tables = 0;
	OBJ_DATA	*obj = NULL;
	bool		clump = FALSE;

	found = FALSE;

	if ( !list || (weather_info[ch->room->zone].state == HEAVY_SNOW && IS_MORTAL(ch) && !IS_SET (ch->room->room_flags, INDOORS)) ) {
		if ( show )
			send_to_char ("Nothing.\n", ch);
		return;
	}

	for ( i = list; i; i = i->next_content ) {
		j++;
	}

	if ( j >= 25 && show != 4 && !list->in_obj )
		clump = TRUE;

	for ( i = list ; i; i = i->next_content ) { 
		if ( CAN_SEE_OBJ (ch, i) ) {

			if ( !mode && !looked_for_tables && IS_TABLE (i) ) {

				for ( obj = i; obj; obj = obj->next_content )
					if ( IS_TABLE (obj) )
						looked_for_tables++;

				if ( looked_for_tables == 1 )
					show_obj_to_char (i, ch, 7);
				else if ( looked_for_tables == 2 )
					send_to_char ("#6There are a couple of furnishings here.#0\n",
								  ch);
				else
					send_to_char ("#6There are several furnishings here.#0\n", ch);
			}

			if ( clump )
				continue;

			show_obj_to_char (i, ch, mode);

			found = TRUE;
		}    
	}  

	if ( clump ) {
		if ( j >= 25 && j < 35 )
			send_to_char ("#2The area is strewn with a number of objects.#0\n", ch);
		else if ( j >= 35 && j < 45 )
			send_to_char ("#2The area is strewn with a sizeable number of objects.#0\n", ch);
		else if ( j >= 45 && j < 55 )
			send_to_char ("#2The area is strewn with a large number of objects.#0\n", ch);
		else if ( j >= 55 && j < 65 )
			send_to_char ("#2The area is strewn with a great number of objects.#0\n", ch);
		else
			send_to_char ("#2The area is strewn with a staggering number of objects.#0\n", ch);
	}
	else if ( !found && show && show != 4 )
		send_to_char("Nothing.\n", ch);
}

void list_char_to_char (CHAR_DATA *list, CHAR_DATA *ch, int mode)
{
	CHAR_DATA	*i = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	int			count = 0;
	int			j = 0; 

	if ( IS_MORTAL(ch) &&
		weather_info[ch->room->zone].state == HEAVY_SNOW &&
		!IS_SET (ch->room->room_flags, INDOORS) )
		return;

	for ( i = list; i; i = i->next_in_room ) {
		if ( i == ch )
			continue;
		j++;
	}

	if ( j < 25 || mode == 4 || ch->room->virtual == AMPITHEATRE ) {
		for ( i = list; i; i = i->next_in_room ) {
			if ( ch != i && ch->vehicle != i ) {
				if ( ch->room->virtual == AMPITHEATRE && IS_MORTAL(i) ) {
					count++;
					continue;
				}
				show_char_to_char (i, ch, mode);
			}
		}
	}
	else {
		if ( j >= 25 && j < 35 )
			send_to_char ("#5The area is filled by a crowd of individuals.\n#0", ch);
		if ( j >= 35 && j < 45 )
			send_to_char ("#5The area is filled by a decently-sized crowd of individuals.\n#0", ch);
		else if ( j >= 45 && j < 55 )
			send_to_char ("#5The area is filled by a sizeable crowd of individuals.\n#0", ch);
		else if ( j >= 55 && j < 65 )
			send_to_char ("#5The area is filled by a large crowd of individuals.\n#0", ch);
		else if ( j > 65 )
			send_to_char ("#5The area is filled by an immense crowd of individuals.\n#0", ch);
	}

	if ( count ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#5There %s %d other %s assembled here for the meeting.#0\n", count != 1 ? "are" : "is", count, count != 1 ? "people" : "person");
		send_to_char (buf, ch);
	}
}

void show_contents (CHAR_DATA *ch, int cmd)
{
	if ( cmd == 1 ) {
		if ( !ch->room->contents ) {
			send_to_char ("\n   None.\n", ch);
			return;
		}

		if ( ch->room->contents ) {
			send_to_char ("\n", ch);
			list_obj_to_char (ch->room->contents, ch, 0, 4);	
		}
		return;
	}
	else if ( cmd == 2 ) {
		if ( (!ch->room->people || !ch->room->people->next_in_room) ) {
			send_to_char ("\n   None.\n", ch);
			return;
		}

		if ( ch->room->people && (ch->room->people->next_in_room || ch->room->people != ch) ) {
			send_to_char ("\n", ch);
			list_char_to_char (ch->room->people, ch, 4);
		}
		return;
	}
}

void do_contents (CHAR_DATA *ch, char *argument, int cmd)
{
	if ( !ch->room->contents && (!ch->room->people || !ch->room->people->next_in_room) ) {
		send_to_char ("   None.\n", ch);
		return;
	}

	if ( ch->room->contents ) {
		send_to_char ("\n", ch);
		list_obj_to_char (ch->room->contents, ch, 0, 4);	
	}

	if ( ch->room->people && (ch->room->people->next_in_room || ch->room->people != ch) ) {
		send_to_char ("\n", ch);
		list_char_to_char (ch->room->people, ch, 4);
	}
}

int enter_exit_msg (CHAR_DATA *ch, char *buffer)
{
	QE_DATA		*qe = NULL;
	char		*addon = NULL;
	bool        isLeaving = FALSE;
	char		*temp_arg = NULL;
	char		*e_dirs [] = {
							"to the north",
							"to the east",
							"to the south",
							"to the west",
							"up",
							"down" };
	char		*a_dirs [] = {
							"the south",
							"the west",
							"the north",
							"the east",
							"below",
							"above" };
		
	

	for ( qe = quarter_event_list; qe; qe = qe->next )
		if ( qe->ch == ch )
			break;

	if ( !qe )
		return 0;

	addon = buffer + strlen (buffer);

	if ( GET_FLAG (ch, FLAG_LEAVING) ) {
	  isLeaving = TRUE;
	}
	else if ( GET_FLAG (ch, FLAG_ENTERING) ) {
	  isLeaving = FALSE;
	}
	else return 0;
	temp_arg = char_short(ch);
	snprintf (addon, MAX_STRING_LENGTH, "#3%s is %s %s%s%s.#0", 
		 CAP(temp_arg), 
		 (isLeaving ? "leaving" : "arriving from"),
		 (isLeaving ? e_dirs [qe->dir] : a_dirs [qe->dir]),
		 ( (qe->travel_str == NULL && ch->travel_str) ? ", " : "" ),
		 ( (qe->travel_str ? qe->travel_str : (ch->travel_str ? ch->travel_str : "" ) ) )
		 );
	addon[2] = toupper (addon[2]);

	return 1;
}

char *char_short (CHAR_DATA *ch)
{      
 	OBJ_DATA	*obj = NULL;
	static char	buf [MAX_STRING_LENGTH] = { '\0' };
    char 		phrase [MAX_STRING_LENGTH] = { '\0' };

	if ( !ch )
		return NULL;
	
    snprintf (phrase, 20, "gigantic");
    if (ch->height < 108)
        snprintf (phrase, 20, "towering");
    if (ch->height < 84)
        snprintf (phrase, 20, "very tall");
    if (ch->height < 77)
        snprintf (phrase, 20, "tall");
    if (ch->height < 71)
        snprintf (phrase, MAX_STRING_LENGTH,  "%s", frame_built [ch->frame]);
    if (ch->height < 66)
        snprintf (phrase, 20, "short");
    if (ch->height < 61)
        snprintf (phrase, 20, "very short");
    if (ch->height < 54)
        snprintf (phrase, 20, "extremely short");
    if (ch->height < 36)
        snprintf (phrase, 20, "tiny");

	if ( ( obj = get_equip(ch, WEAR_ABOUT) ) &&
		IS_SET(obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET(ch->affected_by, AFF_HOODED) ) {
		if ( ch->height >= 54 || ch->height < 36 )
			snprintf (buf, MAX_STRING_LENGTH, "a %s, %s person",phrase,obj->desc_keys);
		else
			snprintf (buf, MAX_STRING_LENGTH, "an %s, %s person",phrase,obj->desc_keys);
		return buf;
	}

	if ( ((obj = get_equip(ch, WEAR_HEAD)) || (obj = get_equip(ch, WEAR_FACE)))
		&& IS_SET(obj->obj_flags.extra_flags, ITEM_MASK)) {
			
		if ( obj->obj_flags.type_flag == ITEM_WORN ) 
		    snprintf (buf, MAX_STRING_LENGTH, "the %s, %s person",phrase,
			    obj->desc_keys);
		else
		    snprintf (buf, MAX_STRING_LENGTH, "the %s, %s person",phrase,
			    obj->desc_keys);
		return buf;
       		
	}

	if ( ( obj = get_equip(ch, WEAR_FACE) )
		&& IS_SET(obj->obj_flags.extra_flags, ITEM_MASK)) {
			
		if ( obj->obj_flags.type_flag == ITEM_WORN ) 
		    snprintf (buf, MAX_STRING_LENGTH, "the %s, %s person",phrase,
			    obj->desc_keys);
		else
		    snprintf (buf, MAX_STRING_LENGTH, "the %s, %s person",phrase,
			    obj->desc_keys);
		return buf;
       		
	}
	return ch->short_descr;
}	

char *char_long (CHAR_DATA *ch, int show_tname)
{      
	OBJ_DATA	*obj = NULL;
	static char	buf [MAX_STRING_LENGTH] = { '\0' };
    	char 		phrase [20] = { '\0' };
	char		tname [MAX_STRING_LENGTH] = { '\0' };
	
    snprintf (phrase, MAX_STRING_LENGTH, "gigantic");
    if (ch->height < 108)
        snprintf (phrase, MAX_STRING_LENGTH, "towering");
    if (ch->height < 84)
        snprintf (phrase, MAX_STRING_LENGTH, "very tall");
    if (ch->height < 77)
        snprintf (phrase, MAX_STRING_LENGTH, "tall");
    if (ch->height < 71)
        snprintf (phrase, MAX_STRING_LENGTH, "%s",frame_built[ch->frame]);
    if (ch->height < 66)
        snprintf (phrase, MAX_STRING_LENGTH, "short");
    if (ch->height < 61)
        snprintf (phrase, MAX_STRING_LENGTH, "very short");
    if (ch->height < 54)
        snprintf (phrase, MAX_STRING_LENGTH, "extremely short");
    if (ch->height < 36)
        snprintf (phrase, MAX_STRING_LENGTH, "tiny");
	
	if ( show_tname && is_hooded (ch) )
		snprintf (tname, MAX_STRING_LENGTH, "(%s) ", ch->tname);
	else
		*tname = '\0';

	if ( ( obj = get_equip(ch, WEAR_ABOUT) ) &&
		IS_SET(obj->obj_flags.extra_flags, ITEM_MASK) &&
		IS_SET(ch->affected_by, AFF_HOODED) ) {
		    if(ch->pmote_str) {
			if(*(ch->pmote_str) == '\''){
			    snprintf (buf, MAX_STRING_LENGTH, "A %s, %s person%s %s",phrase,
				obj->desc_keys, 
				tname, ch->pmote_str);
				return buf;
			} else {
			    snprintf (buf, MAX_STRING_LENGTH, "A %s, %s person%s %s",phrase,
                                obj->desc_keys, 
                                tname, ch->pmote_str);
                                return buf;
			}
		    } else {
			snprintf (buf, MAX_STRING_LENGTH, "A %s, %s person %sis here.",phrase,
				obj->desc_keys, tname);
				return buf;
		   }
	}
		
	if ( ((obj = get_equip(ch, WEAR_HEAD)) || (obj = get_equip(ch, WEAR_FACE)))
		&& IS_SET(obj->obj_flags.extra_flags, ITEM_MASK)) {
			
		if ( obj->obj_flags.type_flag == ITEM_WORN )
		    if(ch->pmote_str) {
			if(*(ch->pmote_str) == '\'')
			    snprintf (buf, MAX_STRING_LENGTH, "A %s, %s %sperson%s",phrase,
				    obj->desc_keys, 
				    tname, ch->pmote_str);
			else
                            snprintf (buf, MAX_STRING_LENGTH, "A %s, %s %sperson %s",phrase,
                                    obj->desc_keys,
                                    tname, ch->pmote_str);
			} else
		            snprintf (buf, MAX_STRING_LENGTH, "A %s, %s %sperson is here.",phrase,
			        obj->desc_keys, tname);
		else {
		    if(ch->pmote_str) {
			if(*(ch->pmote_str) == '\'')
			    snprintf (buf, MAX_STRING_LENGTH, "A %s, %s %sperson%s",phrase,
			            obj->desc_keys, 
				    tname, ch->pmote_str);       		
			else
                            snprintf (buf, MAX_STRING_LENGTH, "A %s, %s %sperson %s",phrase,
                                    obj->desc_keys,
                                    tname, ch->pmote_str);
		    } else
		        snprintf (buf, MAX_STRING_LENGTH, "A %s, %s %sperson is here.",phrase,
			    obj->desc_keys, tname);       		
		}	
	        return buf;
	}

	if(ch->pmote_str) {
	    if(*(ch->pmote_str) == '\'')
		snprintf (buf, MAX_STRING_LENGTH,  "%s%s", ch->short_descr, ch->pmote_str);
	    else
		snprintf (buf, MAX_STRING_LENGTH,  "%s %s", ch->short_descr, ch->pmote_str);
	    
	    buf[0] = (('a' <= buf[0]) && (buf[0] <= 'z')) 
			? (char)(buf[0] - 32) : buf[0];
	    CAP (buf);
	    return buf;
	} 

	    return ch->long_descr;
}	

void stink_to_room_description (int type, char *buf)
{
	strcpy (buf, "#D");
	buf += 2;

	switch ( type ) {
		case MAGIC_AKLASH_ODOR:
			strcpy (buf, "An overwhelming stench of rotten food, decaying "
						 "animals and dead things\npervades the area.");
			break;

		case MAGIC_ROSE_SCENT:
			strcpy (buf, "The pleasant fragrance of roses fills the air.");
			break;

		case MAGIC_JASMINE_SCENT:
			strcpy (buf, "A subtle, alluring aroma of jasmine lingers upon the "
						 "air.");
			break;

		case MAGIC_SEWER_STENCH:
			strcpy (buf, "The stench of raw sewage permeates this area, and "
						 "everything in it.");
			break;

		case MAGIC_SOAP_AROMA:
			strcpy (buf, "The pleasant and comforting scent of soap fills the "
						 "area.");
			break;

		case MAGIC_CINNAMON_SCENT:
			strcpy (buf, "An unmistakable and pleasant fragrance of cinnamon "
						 "floats upon the air.");
			break;

		case MAGIC_LEORTEVALD_STENCH:
			strcpy (buf, "A strong, pungent odor of decaying corpses hangs "
						 "heavily in the air.");
			break;

		case MAGIC_YULPRIS_ODOR:
			strcpy (buf, "The unpleasant, almost sickening stench of rotting "
						 "food makes breathing difficult.");
			break;

		case MAGIC_FRESH_BREAD:
			strcpy (buf, "A fine aroma of freshly baked bread fills the area, "
						 "making the mouth water.");
			break;

		case MAGIC_MOWN_HAY:
			strcpy (buf, "The rich fragrance of new-mown hay pervades the "
						 "area.");
			break;

		case MAGIC_FRESH_LINEN:
			strcpy (buf, "This area bears the comforting scent of freshly "
						 "laundered linen.");
			break;

		case MAGIC_INCENSE_SMOKE:
			strcpy (buf, "The smoke of incense fills the air with its heady scent.");
			break;

		case MAGIC_WOOD_SMOKE:
			strcpy (buf, "The smell of seasoned wood being burned fills the air.");
			break;

		default:
			strcpy (buf, "There is an unrecognizable odor here.");
			break;
	}

	strcat (buf, "#0");
}

void stink_to_description (int type, char *buf)
{
	switch ( type ) {
		case MAGIC_AKLASH_ODOR:
			strcpy (buf, "$n #5smells of rotten food, decaying animals and "
						 "dead things.");
			break;

		case MAGIC_ROSE_SCENT:
			strcpy (buf, "$n #Dsmells pleasantly and faintly of roses.");
			break;

		case MAGIC_JASMINE_SCENT:
			strcpy (buf, "$n #Dhas a subtle scent of jasmine about $mself.");
			break;

		case MAGIC_SEWER_STENCH:
			strcpy (buf, "$n #Dsmells as if $e had been wallowing in raw "
						 "sewage.");
			break;

		case MAGIC_SOAP_AROMA:
			strcpy (buf, "$n #Dsmells fresly bathed.");
			break;

		case MAGIC_CINNAMON_SCENT:
			strcpy (buf, "$n #Dhas about $mself the faint odor of cinnamon.");
			break;

		case MAGIC_LEORTEVALD_STENCH:
			strcpy (buf, "$n #Dsmells of decaying corpses.");
			break;

		case MAGIC_YULPRIS_ODOR:
			strcpy (buf, "$n #Dbears a foul odor, reminiscent of rotting food.");
			break;

		case MAGIC_FRESH_BREAD:
			strcpy (buf, "$n #Dsmells pleasantly of fresh-baked bread.");
			break;

		case MAGIC_MOWN_HAY:
			strcpy (buf, "$n #Dhas about $mself the fresh fragrance of new-mown "
						 "hay.");
			break;

		case MAGIC_FRESH_LINEN:
			strcpy (buf, "$n #Dis enveloped in the comforting scent of freshly "
						 "laundered linen.");
			break;

		case MAGIC_INCENSE_SMOKE:
			strcpy (buf, "$n #Dis surrounded by the pleasant scent of incense.");
			break;

		case MAGIC_WOOD_SMOKE:
			strcpy (buf, "$n #Dhas the unmistakable aroma of burning firewood.");
			break;

		default:
			strcpy (buf, "$n #Dhas an unrecognizable odor.");
			break;
	}

	strcat (buf, "#0");
}

int get_stink_message (CHAR_DATA *ch, ROOM_DATA *room, char *stink_buf,
					   CHAR_DATA *smeller)
{
	int				smeller_is_admin = 0;
	AFFECTED_TYPE	*smell_af = NULL;
	AFFECTED_TYPE	*af = NULL;
	AFFECTED_TYPE	*affect_list = NULL;

	if ( ch )
		affect_list = ch->hour_affects;
	else
		affect_list = room->affects;

	if ( smeller && GET_TRUST (smeller) )
		smeller_is_admin = 1;

	for ( af = affect_list; af; af = af->next ) {

		if ( af->type < MAGIC_SMELL_FIRST ||
			 af->type > MAGIC_SMELL_LAST )
			continue;

		if ( !smeller_is_admin && af->a.smell.aroma_strength < 500 )
			continue;

		if ( !smell_af ) {
			smell_af = af;
			continue;
		}

		if ( af->a.smell.aroma_strength > smell_af->a.smell.aroma_strength )
			smell_af = af;
	}

	if ( !smell_af )
		return 0;

	*stink_buf = '\0';

	if ( smeller && GET_TRUST (smeller) )
		snprintf (stink_buf, MAX_STRING_LENGTH, "(%d %d) ",
						smell_af->a.smell.duration,
						smell_af->a.smell.aroma_strength);

	if ( smeller_is_admin && smell_af->a.smell.aroma_strength < 500 )
		snprintf (stink_buf + strlen (stink_buf), MAX_STRING_LENGTH, "(weak) ");

	if ( ch )
		stink_to_description (smell_af->type, stink_buf + strlen (stink_buf));
	else
		stink_to_room_description (smell_af->type, stink_buf + strlen (stink_buf));

	return 1;
}



void show_char_to_char (CHAR_DATA *i, CHAR_DATA *ch, int mode)
{
	int		       	percent = 0;
	double			curdamage = 0;
	int				location = 0;
	OBJ_DATA		*eq = NULL;
	OBJ_DATA		*blindfold = NULL;
	OBJ_DATA		*tobj = NULL;
	AFFECTED_TYPE	*af = NULL;
	ROOM_DATA		*troom = NULL;
	WOUND_DATA		*wound = NULL;
	CHARM_DATA		*charm = NULL;
	char			*p = NULL;
	char			stink_buf [MAX_STRING_LENGTH] = { '\0' };
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	char			buffer [MAX_STRING_LENGTH] = { '\0' };
	char			*dir_names [] = {
						"to the north",	"to the east", "to the south",
						"to the west", "above", "below" };
	char			buf2[MAX_STRING_LENGTH] = { '\0' };
	char			buf3[MAX_STRING_LENGTH] = { '\0' };
	char			*temp_arg = NULL;
	char			*temp_arg1 = NULL;
	char			*temp_arg2 = NULL;
	char			*temp_arg3 = NULL;
	bool			found = FALSE;

	if ( mode == 4 )
		mode = 0;

	if ( !mode ) {

        if ( IS_SUBDUEE (i) ) {
            if ( i->subdue == ch )
                act("You have #5$N#0 in tow.", FALSE, ch, 0, i, TO_CHAR);
            return;
        }

		if ( !CAN_SEE (ch, i) ) {
			if ( !GET_FLAG (i, FLAG_WIZINVIS) &&
				 get_affect (ch, MAGIC_AFFECT_SENSE_LIFE) )
				send_to_char ("\nYou sense a hidden life form in the room.\n\n",
							  ch);
			return;
		}
		blindfold = get_equip (i, WEAR_BLINDFOLD);
		temp_arg = char_long(i, 0);
		if ( (blindfold) ||
			 !(temp_arg) ||
			 GET_POS (i) != i->default_pos ||
			 IS_SWIMMING (i) ) {
			 
			if ( char_short(i) ) {
				strcpy ( buffer, "#5" );
				temp_arg = char_short(i);
				strcat ( buffer, temp_arg );
				strcat ( buffer, "#0" );
				buffer [2] = UPPER (buffer [2]);

			} else
				strcpy(buffer,"#5A nameless one#0");

			if ( IS_SWIMMING (i) )
				strcat(buffer," is here, swimming.");
				
			else switch(GET_POS(i)) {
			
				case POSITION_STUNNED  :
					if ( blindfold )
						strcat(buffer," is here, stunned and blindfolded.");
					else
						strcat(buffer," is here, stunned.");
					break;
				
				case POSITION_UNCONSCIOUS    :
					if ( blindfold )
						strcat(buffer," is lying here, unconscious and blindfolded.");
					else
						strcat(buffer," is lying here, unconscious.");
					break;
				
				case POSITION_MORTALLYW:
					if ( blindfold )
						strcat(buffer," is lying here, mortally wounded and blindfolded.");
					else
						strcat(buffer," is lying here, mortally wounded.");
					break;
				
				case POSITION_DEAD     :
					strcat(buffer," is lying here, dead.");
					break;
				
				case POSITION_STANDING :
					if ( blindfold )
						strcat (buffer," is standing here, blindfolded.");
					else
						strcat (buffer," is standing here.");
					break;
				
				case POSITION_SITTING  :
					if ( blindfold )
						strcat(buffer," is sitting here, blindfolded");
					if (i->pmote_str) {
						snprintf (buffer, MAX_STRING_LENGTH,  "#5");
						temp_arg = char_long(i, GET_TRUST (ch));
						strcat (buffer, temp_arg);
						strcat (buffer, "#0");
						break;
					}
					else
						strcat(buffer," is sitting here");

					if ( (af = get_affect (i, MAGIC_SIT_TABLE)) &&
						 is_obj_in_list (af->a.table.obj, i->room->contents) ) {
						tobj = af->a.table.obj;
						
						if (CAN_SEE_OBJ((i), (tobj))) /* item on your table */
							temp_arg = obj_short_desc (tobj);
						else
							temp_arg = "somethng";
							
						snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  " at #2%s#0.", temp_arg);
					} else
						strcat (buffer, ".");
					break;
					
				case POSITION_RESTING  :
					if ( blindfold )
						strcat(buffer," is resting here, blindfolded");
					else {
						if (i->pmote_str) {
							snprintf (buffer, MAX_STRING_LENGTH,  "#5");
							temp_arg = char_long(i, GET_TRUST (ch));
							strcat (buffer, temp_arg);
							strcat (buffer, "#0");
							break;
						}
						else
							strcat(buffer," is resting here");
					}				
					if ( (af = get_affect (i, MAGIC_SIT_TABLE)) &&
						 is_obj_in_list (af->a.table.obj, i->room->contents) ) {
						tobj = af->a.table.obj;
						
						if (CAN_SEE_OBJ((i), (tobj))) /* item on your table */
							temp_arg = obj_short_desc (tobj);
						else
							temp_arg = "somethng";
							
						snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  " at #2%s#0.", temp_arg);
					} else
						strcat (buffer, ".");
					break;
					
				case POSITION_SLEEPING :
					if ( blindfold )
						strcat(buffer," is sleeping here, blindfolded");
					else
						strcat(buffer," is sleeping here");

					if ( (af = get_affect (i, MAGIC_SIT_TABLE)) &&
						 is_obj_in_list ((OBJ_DATA *) af->a.spell.t, i->room->contents) ) {
						tobj = (OBJ_DATA *) af->a.spell.t;
						
						if (CAN_SEE_OBJ((i), (tobj))) /* item at his table */
							temp_arg = obj_short_desc (tobj);
						else
							temp_arg = "somethng";
							
						snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH,  " at #2%s#0.", temp_arg);
					} else
						strcat (buffer, ".");

					break;
				
				case POSITION_FIGHTING :
					if (i->fighting) {

						if ( IS_SET (i->flags, FLAG_SUBDUING) ) {
							if ( blindfold )
								strcat (buffer, " is here, #1wrestling blindfolded#0 against ");
							else
								strcat (buffer, " is here, #1wrestling#0 ");
						} else {
							if ( blindfold )
								strcat(buffer," is here, #1fighting blindfolded#0 against ");
							else
								strcat(buffer," is here, #1fighting#0 ");
						}

						if (i->fighting == ch)
							strcat(buffer," you!");
						else {
							if ( i->in_room == i->fighting->in_room ) {
								temp_arg = char_short(i->fighting);
								strcat(buffer, temp_arg);
								strcat(buffer, ".");
                            }
							else
								strcat(buffer, "someone who has already left.");
						}
					} else /* NIL fighting pointer */
							strcat(buffer," is here struggling with thin air.");
					break;
				
				default : strcat (buffer," is floating here."); 
					break;
			}
			if ( i->desc && i->desc->idle )
				strcat (buffer, " #1(idle)#0");

			if ( (GET_TRUST(i) || get_affect(ch,MAGIC_AFFECT_SENSE_LIFE)) && get_affect (i, MAGIC_HIDDEN) )
				strcat (buffer, " #1(hidden)#0");

			if ( GET_FLAG (i, FLAG_WIZINVIS) )
				strcat (buffer, " #C(wizinvis)#0");

			if ( get_affect (i, MAGIC_AFFECT_INVISIBILITY) )
				strcat (buffer, " #1(invisible)#0");

			if ( get_affect (i, MAGIC_AFFECT_CONCEALMENT) &&
				 (i == ch || GET_TRUST (i) ||
                  get_affect (ch, MAGIC_AFFECT_SENSE_LIFE)) )
				strcat (buffer, " #1(blend)#0");

			if ( IS_SET (i->act, PLR_QUIET) && !IS_NPC (i) )
				strcat(buffer," #1(editing)#0");

			if ( !IS_NPC(i) && !i->desc && !i->pc->admin_loaded )
				strcat(buffer, " #1(link dead)#0");

			if ( !IS_MORTAL (ch) && i->pc && !i->desc && i->pc->admin_loaded )
				strcat(buffer, " #3(loaded)#0");

			if ( !IS_NPC (i) && IS_SET (i->plr_flags, NEW_PLAYER_TAG) )
				strcat (buffer, " #2(new player)#0");

			if ( i->desc && i->desc->original && !IS_MORTAL (ch))
				strcat(buffer, " #2(animated)#0");
			strcat(buffer,"#0\n");
			reformat_string (buffer, &p);
			send_to_char(p, ch);
			mem_free (p);
		}

		else { /* npc with long */
			if ( IS_SUBDUER (i) ) {
				if ( IS_RIDER(i) ){
					temp_arg1 = char_short(i);
                	temp_arg2 = char_short(i->mount);
                	temp_arg3 = char_short(i->subdue);
                	snprintf (buffer, MAX_STRING_LENGTH, "#5%s#0, mounted on #5%s#0, has #5%s#0 in tow.\n", temp_arg1, temp_arg3, i->subdue == ch ? "you" : temp_arg3);
                }
                else{
                temp_arg1 = char_short(i);
                temp_arg2 = char_short(i->subdue); 
                snprintf (buffer, MAX_STRING_LENGTH, "#5%s#0 has #5%s#0 in tow.\n", temp_arg1, i->subdue == ch ? "you" : temp_arg2);
                }
                
                buffer [2] = UPPER (buffer [2]);
				send_to_char(buffer,ch);
            }

			else if ( IS_RIDER (i) && !IS_SUBDUER(i) ) {
				temp_arg1 = char_short (i);
				temp_arg2 = char_short (i->mount);
				snprintf (buffer, MAX_STRING_LENGTH,  "%s#0 %s %s#5%s#0.",
						 temp_arg1, AWAKE (i) ?
						   "sits atop" : "is asleep upon", AWAKE (i->mount) ?
						   "" : "a sleeping mount, ", temp_arg2);

				*buffer = toupper (*buffer);
				snprintf (buf2, MAX_STRING_LENGTH,  "#5%s", buffer);
				snprintf (buffer, MAX_STRING_LENGTH,  "%s", buf2);
				act (buffer, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			}

			else if ( IS_RIDEE (i) ) {

				if ( ch == i->mount ) {
					temp_arg = char_short (i);
					snprintf (buffer, MAX_STRING_LENGTH,  "You sit upon %s#5%s#0.\n", AWAKE (i) ? "" : "a sleeping mount, ", temp_arg);

					send_to_char (buffer, ch);
				}
			}

			else if ( IS_HITCHEE (i) ) {

				if ( i == ch ){
					temp_arg = char_short (i);
					snprintf (buffer, MAX_STRING_LENGTH,  "#5You#0 ride upon #5%s#0.\n", temp_arg);
				}
				else{
					temp_arg1 = char_short (i);
					temp_arg2 = char_short (i->hitcher);
					snprintf (buffer, MAX_STRING_LENGTH,  "#5%s#0 is here, hitched to #5%s#0.\n", temp_arg1, i->hitcher == ch ? "you" : temp_arg2);
				}
				
				buffer [2] = toupper (buffer [2]);
				send_to_char (buffer, ch);
			}

			else {
				*buffer = '\0';


				if ( (GET_FLAG (i, FLAG_ENTERING) ||
					  GET_FLAG (i, FLAG_LEAVING)) &&
					  enter_exit_msg (i, buffer) ) {
					if ( i->desc && !IS_NPC (i) && i->desc->idle )
						strcat (buffer, "#1(idle)#0 ");

					if ( GET_FLAG (i, FLAG_WIZINVIS) )
						strcat (buffer, " #C(wizinvis)#0");

					if ( get_affect (i, MAGIC_HIDDEN) )
						strcat (buffer, " #1(hidden)#0");

					if ( get_affect (i, MAGIC_AFFECT_INVISIBILITY) )
					   strcat(buffer," #1(invisible)#0");

					if ( get_affect (i, MAGIC_AFFECT_CONCEALMENT) )
						strcat (buffer, " #1(blend)#0");

					if ( IS_SET (i->act, PLR_QUIET) && !IS_NPC (i) )
						strcat (buffer," #1(editing)#0");

					if ( i->desc && i->desc->original && !IS_MORTAL (ch))
						strcat(buffer, " #2(animated)#0");

					if ( !IS_NPC(i) && !i->desc && !i->pc->admin_loaded )
						strcat(buffer, " #1(link dead)#0");

					if ( !IS_MORTAL(ch) && !IS_NPC(i) && !i->desc && i->pc->admin_loaded )
						strcat(buffer, " #3(loaded)#0");

					if ( !IS_NPC(i) && IS_SET (i->plr_flags, NEW_PLAYER_TAG) )
						strcat (buffer, " #2(new player)#0");

					strcat (buffer, "#0\n");
					reformat_string (buffer, &p);
					send_to_char (p, ch);
					mem_free (p);
				
				} else if ( (af = get_affect (i, AFFECT_SHADOW)) &&
						  af->a.shadow.edge != -1 ) {
					temp_arg = 	char_short(i);  
					snprintf (buf, MAX_STRING_LENGTH,  "%s", temp_arg);
					*buf = toupper (*buf);
					snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH, 
						"#5%s is here, #0standing %s.", buf, dir_names [af->a.shadow.edge]);
					act (buffer, FALSE, ch, 0, i, TO_CHAR | TO_ACT_FORMAT);
				
				} else if ( (af = get_affect (i, AFFECT_GUARD_DIR)) &&
						  af->a.shadow.edge != -1 ) {
					temp_arg = char_short(i);
					snprintf (buf, MAX_STRING_LENGTH,  "%s", temp_arg);
					*buf = toupper(*buf);
					snprintf (buffer + strlen (buffer), MAX_STRING_LENGTH, 
						"#5%s#0 is here, guarding the %s exit.", buf, dirs [af->a.shadow.edge]);
					act (buffer, FALSE, ch, 0, i, TO_CHAR | TO_ACT_FORMAT);
				
				} else {
					snprintf (buffer, MAX_STRING_LENGTH,  "#5");
					temp_arg = char_long(i, GET_TRUST (ch));
					strcat (buffer, temp_arg);
					if ( get_affect (i, MAGIC_HIDDEN) )
						strcat (buffer, " #1(hidden)#0");

					if ( i->desc && i->desc->idle )
						strcat (buffer, " #1(idle)#0");

					if ( GET_FLAG (i, FLAG_WIZINVIS) )
						strcat (buffer, " #C(wizinvis)#0");

					if ( get_affect (i, MAGIC_AFFECT_INVISIBILITY) )
					   strcat(buffer," #1(invisible)#0");

					if ( get_affect (i, MAGIC_AFFECT_CONCEALMENT) )
						strcat (buffer, " #1(blend)#0");

					if ( IS_SET (i->act, PLR_QUIET) && !IS_NPC (i) )
						strcat (buffer," #1(editing)#0");

					if ( i->desc && i->desc->original && !IS_MORTAL (ch))
						strcat(buffer, " #2(animated)#0");

					if ( !IS_NPC(i) && !i->desc && !i->pc->admin_loaded )
						strcat(buffer, " #1(link dead)#0");

					if ( !IS_MORTAL(ch) && !IS_NPC(i) && !i->desc && i->pc->admin_loaded )
						strcat(buffer, " #3(loaded)#0");

					if ( !IS_NPC(i) && IS_SET (i->plr_flags, NEW_PLAYER_TAG) )
						strcat (buffer, " #2(new player)#0");

					strcat (buffer, "#0\n");
					reformat_string (buffer, &p);
					send_to_char (p, ch);
					mem_free (p);
				}
			}
		}

		if ( get_stink_message (i, NULL, stink_buf, ch) ) {
			act (stink_buf, FALSE, i, 0, ch, TO_VICT);
		}

		if ( (af = get_affect (i, MAGIC_TOLL)) &&
			 af->a.toll.room_num == i->in_room ) {
			snprintf (buffer, MAX_STRING_LENGTH,  "$n #Dis collecting tolls from people leaving "
							 "%s.#0", dirs [af->a.toll.dir]);
			act (buffer, TRUE, i, 0, ch, TO_VICT);
		}

	} 
	else if (mode == 1) {

		if (i->description) {

			if  ( ( ((eq = get_equip(i, WEAR_HEAD)) || (eq = get_equip(i, WEAR_FACE))) &&
			    (IS_SET(eq->obj_flags.extra_flags, ITEM_MASK))) ||
			    ((eq = get_equip(i, WEAR_ABOUT)) &&
			    (IS_SET(eq->obj_flags.extra_flags, ITEM_MASK)) &&
			    (IS_SET(i->affected_by,AFF_HOODED)))) {
			    send_to_char("This person's features are not visible.\n",ch);
			} else
			    send_to_char(i->description, ch);
		} else
			act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

		/* Show a character to another */

		if (GET_MAX_HIT(i) > 0)
			percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
		else
			percent = -1; /* How could MAX_HIT be < 1?? */

		temp_arg = char_short(i);
		strcpy (buffer, temp_arg);
		*buffer = toupper (*buffer);

		if ( IS_SET (i->act, ACT_VEHICLE) ) {
			if ( percent >= 100 )
				strcat (buffer, " is in pristine condition.\n");
			else if ( percent >= 90 )
				strcat (buffer, " is slightly damaged.\n");
			else if ( percent >= 70 )
				strcat (buffer, " is damaged, but still functional.\n");
			else if ( percent >= 50 )
				strcat (buffer, " is badly damaged.\n");
			else if ( percent >= 25 )
				strcat (buffer, " is barely functional.\n");
			else if ( percent >= 10 )
				strcat (buffer, " has sustained a great deal of damage!\n");
			else if ( percent >= 0 )
				strcat (buffer, " is about ready to collapse inward!\n");
			send_to_char ("\n", ch);
			send_to_char (buffer, ch);
		}

		*buf2 = '\0';
		*buf3 = '\0';

		if ( i->damage && !IS_SET (i->act, ACT_VEHICLE) && !is_hooded(i) ) {
			for ( wound = i->wounds; wound; wound = wound->next )
				curdamage += wound->damage;
			curdamage += i->damage;
			if ( curdamage > 0 && curdamage <= i->max_hit*.25 ){
				temp_arg = char_short(i);
				snprintf (buffer, MAX_STRING_LENGTH,  "%s's face looks slightly pale.\n", temp_arg);
			}	
			else if ( curdamage > i->max_hit*.25 && curdamage < i->max_hit*.80 ){
				temp_arg = char_short(i);
				snprintf (buffer, MAX_STRING_LENGTH,  "%s's face looks rather pallid.\n", temp_arg);
			}
			
			else if ( curdamage > i->max_hit*.80 && curdamage < i->max_hit*.95 ){
				temp_arg = char_short(i);			
				snprintf (buffer, MAX_STRING_LENGTH,  "%s's face looks quite ashen.\n", temp_arg);
			}
			
			else if ( curdamage > i->max_hit*.95 ){
				temp_arg = char_short(i);			
				snprintf (buffer, MAX_STRING_LENGTH,  "%s's face looks deathly pale.\n", temp_arg);
			}
			
			*buffer = toupper(*buffer);
			send_to_char("\n", ch);
			send_to_char(buffer, ch);
		}

		if ( !IS_SET (i->act, ACT_VEHICLE) && (i->wounds || i->lodged) ) {
			temp_arg = show_wounds(i,0);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s", temp_arg);
			send_to_char("\n", ch);
			strcat (buf3, buf2);
		}

		act (buf3, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		*buf3 = '\0';

		if ( (!i->damage || is_hooded(i)) &&
				!IS_SET (i->act, ACT_VEHICLE) &&
				!i->wounds &&
				!i->lodged ) {
			
			send_to_char("\n", ch);
			temp_arg = char_short(i);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s appears to be in excellent condition.", temp_arg);
			*buf2 = toupper(*buf2);
			reformat_string (buf2, &p);
			send_to_char (p, ch);
			mem_free (p);
			p = NULL;
		}

		if ( i != ch && display_clan_ranks (i, ch) ) {
			temp_arg = display_clan_ranks (i, ch);
			send_to_char ("\n", ch);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s", temp_arg);
			*buf2 = toupper(*buf2);
			reformat_string (buf2, &p);
			send_to_char (p, ch);
			mem_free (p);
			p = NULL;
		}

		if ( i->mob &&
			 i->mob->vehicle_type == VEHICLE_HITCH &&
			 (troom = vtor (i->mob->virtual)) &&
			 (troom->people || troom->contents) &&
			 i->room != troom ) {
			snprintf (buf, MAX_STRING_LENGTH,  "\nOn board, you see:\n");
			send_to_char (buf, ch);
			if ( troom->people )
				list_char_to_char (troom->people, ch, 0);
			if ( troom->contents )
				list_obj_to_char (troom->contents, ch, 0, TRUE);
		}

		for ( charm = i->charms; charm; charm = charm->next ) {
			if ( *show_charm(charm) ){
				temp_arg = show_charm(charm);			
				snprintf (buf3 + strlen(buf3), MAX_STRING_LENGTH, "%s", temp_arg);
			}
		}

		if ( *buf3 ) {
			act (buf3, FALSE, ch, 0, 0, TO_CHAR);
		}

		*buf3 = '\0';
		if ( i == ch ) {
			send_to_char ("\n", i);
			do_equipment (i, "", 0);
		}
		else {
			if ( i->equip || i->right_hand || i->left_hand )
				send_to_char ("\n", ch);
		        if ( i->right_hand ) {
				if ( !IS_SET (i->act, ACT_MOUNT) ) {
                			if ( i->right_hand->location == WEAR_PRIM || i->right_hand->location == WEAR_SEC )
                        			snprintf (buf, MAX_STRING_LENGTH,   "<wielded in right hand>  ");
                			else if ( i->right_hand->location == WEAR_BOTH )
                        			snprintf (buf, MAX_STRING_LENGTH,   "<wielded in both hands>  ");
                			else if ( i->right_hand->location == WEAR_SHIELD )
                        			snprintf (buf, MAX_STRING_LENGTH,   "<gripped in right hand>  ");
               				else snprintf (buf, MAX_STRING_LENGTH,      "<carried in right hand>  ");
				}
				else snprintf (buf, MAX_STRING_LENGTH,      "<carried on back>        ");
                		send_to_char (buf, ch);
				show_obj_to_char (i->right_hand, ch, 1);
        		}
		        if ( i->left_hand ) {
				if ( !IS_SET (i->act, ACT_MOUNT) ) {
                			if ( i->left_hand->location == WEAR_PRIM || i->left_hand->location == WEAR_SEC )
                       	 		snprintf (buf, MAX_STRING_LENGTH,   "<wielded in left hand>   ");
                			else if ( i->left_hand->location == WEAR_BOTH )
                       	 		snprintf (buf, MAX_STRING_LENGTH,   "<wielded in both hands>  ");
                			else if ( i->left_hand->location == WEAR_SHIELD )
                       	 		snprintf (buf, MAX_STRING_LENGTH,   "<gripped in left hand>   ");
               				else snprintf (buf, MAX_STRING_LENGTH,      "<carried in left hand>   ");
				}
				else snprintf (buf, MAX_STRING_LENGTH,      "<carried on back>        ");

                		send_to_char (buf, ch);
				show_obj_to_char (i->left_hand, ch, 1);
        		}

		        if ( i->equip && (i->left_hand || i->right_hand) )
                		send_to_char ("\n", ch);
                		
				for ( location = 0; location < MAX_WEAR; location++ ) {

					eq = get_equip (i, location);
	
					if ( !(eq) )
						continue;

					if ( eq == i->right_hand || eq == i->left_hand )
						continue;

			/* only layer 1 i hidden */
					if ((location == WEAR_BODY_1) &&
						(get_equip (i, WEAR_BODY_2) ||
						get_equip (i, WEAR_BODY_3) ||
						get_equip (i, WEAR_BODY_4) ||
						get_equip (i, WEAR_BODY_5)))
						continue;
						
			/* all lower layers hidden */
					if ((location == WEAR_LEGS_1) &&
						(get_equip (i, WEAR_LEGS_2) ||
						get_equip (i, WEAR_LEGS_3)))
						continue;

					if ((location == WEAR_LEGS_2) &&
						(get_equip (i, WEAR_LEGS_3)))
						continue;
					
					send_to_char (where [location], ch);
					
					if ( location == WEAR_BLINDFOLD || IS_OBJ_VIS (ch, eq) )
						show_obj_to_char (eq, ch, 1);
					else
						send_to_char ("#2something#0\n", ch);
	
					found = TRUE;
				}
			}
	}

	else if ( mode == 3 ) {
		if ( CAN_SEE (ch, i) ) {
			send_to_char ("   ", ch);
			act ("$N", FALSE, ch, 0, i, TO_CHAR);
		}
	}
}

void do_search (CHAR_DATA *ch, char *argument, int cmd)
{
	int		dir = 0;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	if ( !real_skill (ch, SKILL_SEARCH) ) {
		send_to_char ("You'll need to learn some search skills first.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf )
		dir = -1;
	else {
		switch ( *buf ) {
			case 'n': dir = 0; break;
			case 'e': dir = 1; break;
			case 's': dir = 2; break;
			case 'w': dir = 3; break;
			case 'u': dir = 4; break;
			case 'd': dir = 5; break;
			default:
				send_to_char ("That's not a valid direction.\n", ch);
				return;
		}
	}

	act ("$n carefully searches the area.", TRUE, ch, 0, 0, TO_ROOM);
	act ("You search carefully...", FALSE, ch, 0, 0, TO_CHAR);

    ch->delay_type  = DEL_SEARCH;
    ch->delay       = 3 + number (0, 2);
	ch->delay_info1 = dir;
}

void delayed_search (CHAR_DATA *ch)
{
	int			dir = 0;
	int			skill_tried = 0;
	int			somebody_found = 0;
	int			search_quality = 0;
	CHAR_DATA		*tch = NULL;
	OBJ_DATA		*obj = NULL;
	AFFECTED_TYPE		*af = NULL;
	char			buf [MAX_STRING_LENGTH] = { '\0' };

	dir = ch->delay_info1;

	if ( dir == -1 ) {

		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			
			if ( tch == ch )
				continue;

			if ( !get_affect (tch, MAGIC_HIDDEN) )
				continue;

			if ( GET_FLAG (tch, FLAG_WIZINVIS) )
				continue;

			if ( !IS_LIGHT (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS))
				continue;

			if ( (ch->room->sector_type == SECT_WOODS ||
				  ch->room->sector_type == SECT_FOREST ||
				  ch->room->sector_type == SECT_HILLS) &&
				 get_affect (tch, MAGIC_AFFECT_CONCEALMENT) )
				continue;

			if ( get_affect (tch, MAGIC_AFFECT_INVISIBILITY) &&
				 !get_affect (ch, MAGIC_AFFECT_SEE_INVISIBLE) )
				continue;

			if ( !skill_tried ) {
				skill_use (ch, SKILL_SEARCH, 0);
				skill_tried = 1;
			}

			search_quality = ch->skills [SKILL_SEARCH] -
								tch->skills [SKILL_HIDE] / 5;

			if ( search_quality <= 0 )
				continue;

			if ( search_quality < number (0, 100) ) {
				if ( !number (0, 10) )
					act ("$n looks right at you, but still doesn't see you.",
							TRUE, ch, 0, tch, TO_VICT);
				continue;
			}

			act ("You are exposed by $N!", FALSE, tch, 0, ch, TO_CHAR);
			act ("You expose $N!", FALSE, ch, 0, tch, TO_CHAR);
			act ("$n exposes $N.", FALSE, ch, 0, tch, TO_NOTVICT);

			if ( enter_exit_msg (ch, buf) )
				act (buf, FALSE, tch, 0, 0, TO_ROOM);

			remove_affect_type (tch, MAGIC_HIDDEN);

			somebody_found = 1;
		}

		if ( !somebody_found ) {
			send_to_char ("You didn't find anybody hiding.\n", ch);
			act ("$n finishes searching.", TRUE, ch, 0, 0, TO_ROOM);
		}

		for ( obj = ch->room->contents; obj; obj = obj->next_content ) {

			if ( !(af = get_obj_affect (obj, MAGIC_HIDDEN)) )
				continue;

			if ( !IS_LIGHT (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS))
				continue;

			if ( af->a.hidden.coldload_id == ch->coldload_id )
				continue;

			if ( !skill_tried ) {
				skill_use (ch, SKILL_SEARCH, 0);
				skill_tried = 1;
			}

			if ( ch->skills [SKILL_SEARCH] <= number (1, 100) )
				continue;

			if ( af->a.hidden.hidden_value && number (1, ch->skills [SKILL_SEARCH]) < number (1, af->a.hidden.hidden_value) )
				continue;

			remove_obj_affect (obj, MAGIC_HIDDEN);

			act ("You reveal $p.", FALSE, ch, obj, 0, TO_CHAR);
			act ("$n reveals $p.", FALSE, ch, obj, 0, TO_ROOM);
		}

		return;
	}

	if ( !ch->room->secrets [dir] ||
	 	 ch->skills [SKILL_SEARCH] < ch->room->secrets [dir]->diff ||
		 !skill_use (ch, SKILL_SEARCH, 5 * ch->room->secrets [dir]->diff) ) {
		send_to_char ("You didn't find anything.\n", ch);
		return;
	}

	send_to_char (ch->room->secrets [dir]->stext, ch);
}

char *get_room_description (CHAR_DATA *ch, ROOM_DATA *room)
{
	if ( IS_MORTAL(ch) && weather_info[room->zone].state == HEAVY_SNOW && !IS_SET (room->room_flags, INDOORS) )
		return "   A howling blanket of white completely obscures your vision.\n";

	else if ( !room->extra || desc_weather[room->zone] == WR_NORMAL ||
			  !room->extra->weather_desc [desc_weather[room->zone]] ) {

		if ( room->extra &&
			 room->extra->weather_desc [WR_NIGHT] && !sun_light )
            return (room->extra->weather_desc [WR_NIGHT]);
		else
            return room->description;

	} else
        return room->extra->weather_desc [desc_weather[room->zone]];
}

void do_look (CHAR_DATA *ch, char *argument, int cmd)
{
	int				temp = 0;
	int				i = 0;
	int				dir = 0;
	int				count = 0;
	char			*ptr = NULL;
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*obj1 = NULL;
	OBJ_DATA		*obj2 = NULL;
	OBJ_DATA		*obj3 = NULL;
	OBJ_DATA		*tobj = NULL;
	CHAR_DATA		*tch = NULL;
	ROOM_DATA		*troom = NULL;
	ROOM_DIRECTION_DATA		*exit_room = NULL;
	AFFECTED_TYPE	*af = NULL;
	bool			contents = FALSE;
	char			bitbuf [MAX_STRING_LENGTH] = { '\0' };
	char			arg1 [MAX_STRING_LENGTH] = { '\0' };
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	char			*temp_arg = NULL;
	int				temp_int = 0;
	int				rnum = 0;
	int				original_loc = 0;
	bool			change = FALSE;
	bool			again = TRUE;
	bool			abrt = FALSE;
	char			*e_dirs [] = { "the north", "the east", "the south", "the west", "above", "below" };


/*** CHECK FOR POSTIONS AND CONDITONS FIRST ***/

	if ( GET_POS (ch) < POSITION_SLEEPING ) {
		send_to_char ("You are unconscious!\n", ch);
		return;
	}

	if ( GET_POS (ch) == POSITION_SLEEPING ) {
		send_to_char ("You are asleep.\n", ch);
		return;
	}
 
 	temp_int = is_blind (ch);
	if (temp_int) {
		send_to_char ("You are blind!\n", ch);
		return;
	}
	

/**** NOW THAT YOU CAN SEE THINGS, WHAT DO YOU SEE? ****/

	if ( !ch->room )
		ch->room = vtor (ch->in_room);

	argument = one_argument (argument, arg1);


/**** LOOKING OUT THE WINDOW IN THE OOC LOUNGE ****/

	if ( !str_cmp (arg1, "window") && ch->in_room == OOC_LOUNGE ) {
		count = 1;
		for ( troom = full_room_list; troom; troom = troom->lnext )
			count++;

		while ( again ) {
			abrt = FALSE;
			rnum = number (1,count);
			for ( troom = full_room_list, i = 1; troom; troom = troom->lnext, i++ ) {
			if ( i != rnum )
					continue;
			
			/** can't see PC's **/		
			for ( tch = troom->people; tch; tch = tch->next_in_room ) 
					if ( !IS_NPC (tch) )
						abrt = TRUE;
			
			/** Storage rooms, or unfinished rooms **/
			if ( IS_SET (troom->room_flags, STORAGE) )
					abrt = TRUE;
			if ( !strncmp (troom->description, "No Description Set", 17) )
					abrt = TRUE;
			if ( !strncmp (troom->description, "   No Description Set", 20) )
					abrt = TRUE;
					
			/** It's Dark **/		
			if ( is_dark (ch->room) && IS_MORTAL (ch) &&
		 			!get_affect (ch, MAGIC_AFFECT_INFRAVISION) &&
		 			!IS_SET (ch->affected_by, AFF_INFRAVIS) )
					abrt = TRUE;
					
			/** Can't see rooms QQ000, or anything in Zone 0 **/		
			if ( troom->virtual % 1000 == 0 )
					abrt = TRUE;
			if ( troom->zone == 0 )
					abrt = TRUE;
					
			/** If we found a room then use the rnum **/		
			if ( !abrt ) {
					again = FALSE;
					rnum = troom->virtual;
				}
			}
		}
		original_loc = ch->in_room;
		act ("$n presses $s face to the window, gazing down at thee world below.", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		act ("You press your face to the window, gazing down at the world below.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		char_from_room(ch);
		char_to_room (ch, rnum);
		send_to_char ("\n", ch);
		if ( !ch->room->light ) {
			ch->room->light = 1;
			change = TRUE;
		}
		do_look (ch, "", 0);
		if ( change )
			ch->room->light = 0;
		char_from_room(ch);
		char_to_room (ch, original_loc);
		if ( IS_MORTAL(ch) )
			ch->roundtime = 15;
		return;
	}

/***** LOOK IN A CERTAIN DIRECTION *****/

	if ( *arg1 && (dir = index_lookup (dirs, arg1)) != -1 ) {

		if ( !(exit_room = PASSAGE (ch, dir)) ) {
			send_to_char ("There is no exit that way.\n", ch);
			return;
		}

		if ( exit_room->general_description &&
			 *exit_room->general_description )
			send_to_char (exit_room->general_description, ch);

		if ( (af = get_affect (ch, AFFECT_SHADOW)) && af->a.shadow.edge == dir &&
			IS_SET (exit_room->exit_info, PASSAGE_CLOSED) ) {
			send_to_char ("Your field of view through that exit is obstructed.\n", ch);
		}

		else if ( (af = get_affect (ch, AFFECT_SHADOW)) &&
			 af->a.shadow.edge == dir ) {
			send_to_char ("You are close enough to see what is in the next "
						  "room:\n\n", ch);

			temp = ch->in_room;

			char_from_room (ch);
			char_to_room (ch, exit_room->to_room);

			list_obj_to_char (ch->room->contents, ch, 0, FALSE);
			list_char_to_char (ch->room->people, ch, 0);

			for ( tch = ch->room->people; tch; tch = tch->next_in_room )
				if ( CAN_SEE (ch, tch) )
					target_sighted (ch, tch);

			char_from_room (ch);
			char_to_room (ch, temp);
		}

		else {

			if ( !(troom = vtor (exit_room->to_room)) ) {
				send_to_char ("The way in that direction is blocked.\n", ch);
				return;
			}

			temp = 0;

			for ( tch = troom->people; tch; tch = tch->next_in_room )
				if ( (af = get_affect (tch, AFFECT_SHADOW)) &&
					 af->a.shadow.edge == rev_dir [dir] &&
					could_see (ch, tch) ) {
					act ("You see $N.", FALSE, ch, 0, tch, TO_CHAR);
					temp = 1;
				}
		}
		return;
	}
		
	if ( is_dark (ch->room) && IS_MORTAL (ch) &&
		 !get_affect (ch, MAGIC_AFFECT_INFRAVISION) &&
		 !IS_SET (ch->affected_by, AFF_INFRAVIS) ) {
		send_to_char ("It is pitch black.\n", ch);
		return;
	}

/**** LOOK INSIDE ANOTHER OBJECT ****/

	if ( !str_cmp (arg1, "in") ) {

		argument = one_argument (argument, arg1);

		if ( !*arg1 ) {
			send_to_char ("Look in what?\n", ch);
			return;
		}

		if ( !(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
			 !(obj = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
			 !(obj = get_obj_in_dark (ch, arg1, ch->equip)) &&
			 !(obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)) ) {
			send_to_char ("You don't see that here.\n", ch);
			return;
		}

/**** LOOK INSIDE A DRINK CONTATINER ***/

		if ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON ) {

			if ( obj->o.drinkcon.volume <= 0 )
				act ("$o is empty.", FALSE, ch, obj, 0, TO_CHAR);

			else {
				if ( obj->o.drinkcon.capacity )
					temp = obj->o.drinkcon.volume * 3 / obj->o.drinkcon.capacity;
				else
					temp = 1;
				temp_arg = vnum_to_liquid_name (obj->o.drinkcon.liquid);
				snprintf (buf, MAX_STRING_LENGTH,  "$o is %sfull of %s.\n",
							  fullness [temp], temp_arg );
				act (buf, FALSE, ch, obj, 0, TO_CHAR);
			}

			return;
		}

/**** LOOK INSIDE A NON-DRINK CONTAINER ***/

		else if ( GET_ITEM_TYPE (obj) == ITEM_CONTAINER ||
		          GET_ITEM_TYPE (obj) == ITEM_QUIVER ||
			  GET_ITEM_TYPE (obj) == ITEM_SHEATH ||
			  (GET_ITEM_TYPE (obj) == ITEM_WEAPON &&
			   obj->o.weapon.use_skill == SKILL_SLING) ||
			  GET_ITEM_TYPE (obj) == ITEM_KEYRING ) {

			if ( IS_SET (obj->o.container.flags, CONT_CLOSED) ) {
				send_to_char ("It is closed.\n", ch);
					return;
				}
			
			temp_arg = fname (obj->name);
			send_to_char (temp_arg, ch);
			
			if ( obj->in_room != NOWHERE )
				send_to_char (" (here) : \n", ch);
			else if ( obj->carried_by )
				send_to_char (" (carried) : \n", ch);
			else
				send_to_char (" (used) : \n", ch);

			list_obj_to_char (obj->contains, ch, 1, TRUE);
		}

		else
			send_to_char("That is not a container.\n", ch);
	}

/**** LOOK OUT OF A TRANSPORT ****/

	else if ( !str_cmp (arg1, "overboard") || !str_cmp (arg1, "over") ) {

		if ( !ch->vehicle ) {
			send_to_char ("You're not on a boat?\n", ch);
			return;
		}

		if ( !OUTSIDE (ch) ) {
			send_to_char ("You can do that on deck, but not from here.\n", ch);
			return;
		}

		if ( is_dark (ch->room) && IS_MORTAL (ch) )
			send_to_char ("It is dark overboard.\n", ch);
		else {
			snprintf (buf, MAX_STRING_LENGTH,  "#6%s#0\n", ch->vehicle->room->name);
			send_to_char (buf, ch);
			temp_arg = get_room_description (ch, ch->vehicle->room);
			send_to_char (temp_arg, ch);
		}

		if ( ch->vehicle && OUTSIDE (ch) ) {
			troom = ch->vehicle->room;
			if ( troom->contents || troom->people->next_in_room ) {
				if ( troom->contents )
					list_obj_to_char (troom->contents, ch, 0, FALSE);
				if ( troom->people->next_in_room )
					list_char_to_char (troom->people, ch, 0);
			}
		}
	}

/**** LOOK AT ITEMS ****/

	else if ( *arg1 ) {

		if ( !str_cmp (arg1, "at") )
			argument = one_argument (argument, arg1);

		if ( !*arg1 ) {
			send_to_char ("Look at what?\n", ch);
			return;
		}

		tch = get_char_room_vis (ch, arg1);
		if (tch) {

			argument = one_argument (argument, arg1);
			
			obj1 = get_obj_in_list_vis (tch, arg1, tch->right_hand);
			obj2 = get_obj_in_list_vis (tch, arg1, tch->left_hand);
			if ( *arg1 && ((obj1) || (obj2))) {
				act ("$N is carrying $p.", FALSE, ch, obj, tch, TO_CHAR);
				send_to_char ("\n", ch);
				if (obj1)
					show_obj_to_char (obj1, ch, 5);
				else
					show_obj_to_char (obj2, ch, 5);
				return;
			}
			obj = get_obj_in_list_vis (tch, arg1, tch->equip);
			if ( *arg1 && (obj)){
				act ("$N is wearing $p.", FALSE, ch, obj, tch, TO_CHAR);
				send_to_char ("\n", ch);
				show_obj_to_char (obj, ch, 5);
				return;
			}

			if ( *arg1 ) {
				act ("$N doesn't have that.", FALSE, ch, 0, tch, TO_CHAR);
				return;
			}

			show_char_to_char (tch, ch, 1);

			return;
		}

/**** READ A MESSAGE BOARD ****/
		
		obj1 = get_obj_in_dark (ch, arg1, ch->right_hand); 
		obj2 = get_obj_in_dark (ch, arg1, ch->left_hand);
		obj3 = get_obj_in_dark (ch, arg1, ch->equip);
				
		if (obj1)
			obj = obj1;
			
		else if	(obj2)
			obj = obj2;
		
		else if (obj3)
			obj = obj3;
			
		else 
			obj = get_obj_in_list_vis (ch, arg1, ch->room->contents);
		
		if (obj && obj->obj_flags.type_flag == ITEM_BOARD){			
			(void)one_argument (obj->name, buf);			
			retrieve_mysql_board_listing (ch, buf);
			return;
		}

/*** EXTRA ROOM DESCRIPTION ***/

		if ( (ptr = find_ex_description (arg1, ch->room->ex_description)) &&
			 *ptr ) {
			page_string (ch->desc, ptr);
			return;
		}

		if ( obj ) {
			show_obj_to_char (obj, ch, 5);
			return;
		}
		
		if ( *arg1 && (!cmp_strn (arg1, "furnishings", strlen(arg1)) || !cmp_strn (arg1, "furniture", strlen(arg1)) || !cmp_strn (arg1, "tables", strlen(arg1))) ) {
			do_tables (ch, "", 0);
			return;
		}

		if ( *arg1 && !cmp_strn (arg1, "objects", strlen(arg1)) ) {
			show_contents (ch, 1);

			return;
		}

		if ( *arg1 && (!cmp_strn (arg1, "crowd", strlen(arg1)) || !cmp_strn (arg1, "individuals", strlen(arg1))) ) {
			show_contents (ch, 2);

			return;
		}

		send_to_char ("You do not see that here.\n", ch);
		
		return;
	}

	else {			
	
/**** GENERAL LOOK - GIVES ROOM DESCRIPTION ****/

		if ( IS_MORTAL (ch) ) {

			snprintf (buf, MAX_STRING_LENGTH,  "#6%s#0", ch->room->name);
			send_to_char (buf, ch);
		} else {
			sprintbit (ch->room->room_flags, room_bits, bitbuf);
			snprintf (buf, MAX_STRING_LENGTH,  "#6%s#0 #2[%d: %s#6%s#2]#0",
							ch->room->name,
							ch->room->virtual,
							bitbuf,
							sector_types [ch->room->sector_type]);

			send_to_char (buf, ch);

			if ( ch->room->prg )
				send_to_char (" [Prog]", ch);

			if ( ch->room->deity && ch->room->sector_type != SECT_LEANTO ) {
				/* Lean-tos use the deity int to record the number of people camping there. */
				snprintf (buf, MAX_STRING_LENGTH,  " Temple of %s\n",
							deity_name [ch->room->deity]);
				send_to_char (buf, ch);
			}

			if (ch->room->sector_type == SECT_LEANTO) {
				snprintf (buf, MAX_STRING_LENGTH,  "\nNumber of Occupants: %d.", (ch->room->deity - 15));
				if (!IS_MORTAL(ch)) send_to_char(buf, ch);
			}
		}

/*** DISPLAY PATH VEHICLE CAN TAKE ***/

		if ( ch->vehicle &&
			 is_he_somewhere (ch->vehicle) &&
			 ch->vehicle->mob->helm_room == ch->in_room ) {

			strcpy (buf, " May sail: [#4");

			for ( dir = 0; dir < 6; dir++ ) {

				if ( !PASSAGE (ch->vehicle, dir) ||
					 !(troom = vtor (PASSAGE (ch->vehicle, dir)->to_room)) )
					continue;

				if ( troom->sector_type == SECT_DOCK ||
					 troom->sector_type == SECT_REEF ||
					 troom->sector_type == SECT_OCEAN ||
					 is_room_affected (troom->affects, MAGIC_ROOM_FLOOD) )
					add_char (buf, toupper (*dirs [dir]));
				}

			strcat (buf, "#0]");
			send_to_char (buf, ch);
		}
				
/*** WEATHER EFFECTS IN ROOM ***/
				
		if ( (weather_info[ch->room->zone].state != HEAVY_SNOW || IS_SET (ch->room->room_flags, INDOORS)) || !IS_MORTAL(ch) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#6Exits:#0 ");

			for ( dir = 0; dir < 6; dir++ ) {
				if ( !PASSAGE (ch, dir) )
					continue;
				
				if ( ch->room->secrets [dir] &&
					 IS_SET (PASSAGE (ch, dir)->exit_info, PASSAGE_CLOSED) && IS_MORTAL(ch) )
					continue;
				
				if ( PASSAGE(ch, dir)->to_room && (troom = vtor(PASSAGE(ch, dir)->to_room)) && IS_SET (troom->room_flags, FALL) )
					snprintf (ADDBUF, MAX_STRING_LENGTH,  "#1%s#0 ", dirs[dir]);
				
				else if ( PASSAGE(ch, dir)->to_room && (troom = vtor(PASSAGE(ch, dir)->to_room)) && (troom->sector_type == SECT_RIVER ||
					troom->sector_type == SECT_LAKE || troom->sector_type == SECT_OCEAN || troom->sector_type == SECT_REEF ||
					troom->sector_type == SECT_UNDERWATER) )
					snprintf (ADDBUF, MAX_STRING_LENGTH,  "#4%s#0 ", dirs[dir]);
				
				else
					snprintf (ADDBUF, MAX_STRING_LENGTH,  "#2%s#0 ", dirs[dir]);
				
				if ( !PASSAGE(ch, dir)->exit_info )
					continue;
				
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "(%s %s) ", IS_SET (PASSAGE(ch, dir)->exit_info, PASSAGE_CLOSED) ? "closed" : "open", PASSAGE(ch, dir)->keyword);
			}

			if ( ch->room->virtual > 100000 ) {
				if ( (tobj = find_dwelling_obj (ch->room->virtual)) && IS_SET (tobj->o.od.value[2], CONT_CLOSED) )
					snprintf (ADDBUF, MAX_STRING_LENGTH,  "#2outside#0 (closed entryway)");
				else snprintf (ADDBUF, MAX_STRING_LENGTH,  "#2outside#0 (entryway)");
			}

			send_to_char ("\n", ch);
			act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		}
		else send_to_char ("\n", ch);

		send_to_char ("\n", ch);
		temp_arg = get_room_description(ch, ch->room);
		send_to_char (temp_arg, ch);

		for ( obj = ch->room->contents; obj; obj = obj->next_content )
			if ( CAN_SEE_OBJ (ch, obj) )
				contents = TRUE;

		for ( tch = ch->room->people; tch; tch = tch->next_in_room )
			if ( CAN_SEE (ch, tch) && ch != tch )
				contents = TRUE;

		if ( contents )
			send_to_char ("\n", ch);

		if ( get_stink_message (NULL, ch->room, arg1, ch) )
			act (arg1, FALSE, ch, 0, 0, TO_CHAR);

		if ( !IS_SET (ch->room->room_flags, INDOORS) && ch->room->sector_type != SECT_UNDERWATER ) {
			*buf = '\0';
			if ( !contents )
				snprintf (buf, MAX_STRING_LENGTH,  "\n");
			
			if ( IS_SET (ch->room->room_flags, STIFLING_FOG) )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#6A stifling fog blankets the area, minimizing visibility.#0\n");
			else if ( weather_info[ch->room->zone].state == LIGHT_RAIN )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#6A light, cool rain is falling here.#0\n");
			else if ( weather_info[ch->room->zone].state == STEADY_RAIN )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#6A copious amount of rain is falling steadily here.#0\n");
			else if ( weather_info[ch->room->zone].state == HEAVY_RAIN )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#6Rain falls in heavy sheets, inundating the area with water.#0\n");
			else if ( weather_info[ch->room->zone].state == LIGHT_SNOW )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#6A light dusting of snow is falling in the area.#0\n");
			else if ( weather_info[ch->room->zone].state == STEADY_SNOW )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#6Snow is falling steadily here, beginning to blanket the area.#0\n");
			else if ( weather_info[ch->room->zone].fog == THIN_FOG )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#6Thin, silvery tendrils of fog permeate the area.#0\n");
			else if ( weather_info[ch->room->zone].fog == THICK_FOG )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#6The area is cloaked in a thick, muted veil of dense white fog.#0\n");
			else if ( weather_info[ch->room->zone].state == CHANCE_RAIN )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#6The air here carries the subtle aroma of impending rain.\n#0");
			if ( contents )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "\n");
			if ( strlen(buf) > 2 )
				send_to_char (buf, ch);
		}

		list_obj_to_char (ch->room->contents, ch, 0, FALSE);

		list_char_to_char (ch->room->people, ch, 0);

		*buf = '\0';

		for ( af = ch->room->affects; af; af = af->next ) {
			if ( af->type == MAGIC_ROOM_FIGHT_NOISE )
				snprintf (ADDBUF, MAX_STRING_LENGTH,  "#1The clamor of an armed conflict emanates from %s!#0\n", e_dirs[af->a.room.duration]);
		}

		if ( *buf ) {
			send_to_char ("\n", ch);
			send_to_char (buf, ch);
		}
	}
	return;
}

int read_pc_message (CHAR_DATA *ch, char *name, char *argument)
{
	CHAR_DATA	*who = NULL;
	MESSAGE_DATA	*message = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	*name = toupper (*name);

	if ( !(who = load_pc (name)) )
		return 0;

	argument = one_argument (argument, buf);

	if ( !strtol(buf, NULL, 10)) {
		send_to_char ("Which message?\n", ch);
		return 1;
	}

	if ( !(message = load_message (name, 7, strtol(buf, NULL, 10))) ) {
		send_to_char ("No such message.\n", ch);
		return 1;
	}

	snprintf (b_buf, MAX_STRING_LENGTH, "#6Date:#0    %s\n"
			"#6Author:#0  %s\n"
			"#6Subject:#0 %s\n\n%s", message->date, message->poster, message->subject, message->message);

	send_to_char ("\n", ch);
	page_string (ch->desc, b_buf);

	if ( ch == who && !message->flags )
		mark_as_read (ch, strtol(buf, NULL, 10));

	unload_message (message);
	unload_pc (who);

	return 1;
}

int read_virtual_message (CHAR_DATA *ch, char *name, char *argument)
{
	MESSAGE_DATA	*message = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	*name = toupper (*name);

	argument = one_argument (argument, buf);

	if ( !strtol(buf, NULL, 10)) {
		send_to_char ("Which message?\n", ch);
		return 1;
	}

	if ( !(message = load_message (name, 5, strtol(buf, NULL, 10))) ) {
		send_to_char ("No such virtual board message.\n", ch);
		return 1;
	}

	snprintf (b_buf, MAX_STRING_LENGTH, "#6Date:#0    %s\n"
			"#6Author:#0  %s\n"
			"#6Subject:#0 %s\n\n%s", message->date, message->poster, message->subject, message->message);

	send_to_char ("\n", ch);
	page_string (ch->desc, b_buf);

	unload_message (message);

	return 1;
}

void do_read (CHAR_DATA *ch, char *argument, int cmd)
{
	int			msg_num = 0;
	OBJ_DATA	*board_obj = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if ( !isdigit (*buf) ) {
		if ( !*buf ) {
			send_to_char ("Which message?\n", ch);
			return;
		}

		if ( !((board_obj = get_obj_in_dark (ch, buf, ch->right_hand)) && (GET_ITEM_TYPE (board_obj) == ITEM_BOOK || GET_ITEM_TYPE(board_obj) == ITEM_PARCHMENT || GET_ITEM_TYPE (board_obj) == ITEM_BOARD)) &&
		 	 !((board_obj = get_obj_in_dark (ch, buf, ch->left_hand)) && (GET_ITEM_TYPE (board_obj) == ITEM_BOOK || GET_ITEM_TYPE(board_obj) == ITEM_PARCHMENT || GET_ITEM_TYPE (board_obj) == ITEM_BOARD)) &&
		 	 !((board_obj = get_obj_in_dark (ch, buf, ch->equip)) && (GET_ITEM_TYPE (board_obj) == ITEM_BOOK || GET_ITEM_TYPE (board_obj) == ITEM_PARCHMENT || GET_ITEM_TYPE (board_obj) == ITEM_BOARD)) &&
			 !((board_obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) && (GET_ITEM_TYPE (board_obj) == ITEM_BOOK || GET_ITEM_TYPE (board_obj) == ITEM_PARCHMENT || GET_ITEM_TYPE (board_obj) == ITEM_BOARD)) ) {
			if ( IS_MORTAL (ch) || !read_pc_message (ch, buf, argument) ) {
				if ( IS_MORTAL (ch) || !read_virtual_message (ch, buf, argument) ) {
					send_to_char ("You can't see that board.\n", ch);
				}
			}
			return;
		}

		if ( GET_ITEM_TYPE (board_obj) == ITEM_BOOK || GET_ITEM_TYPE (board_obj) == ITEM_PARCHMENT ) {
			do_look (ch, board_obj->name, 0);
			return;
		}

		argument = one_argument (argument, buf);
		
		if ( !isdigit (*buf) ) {
			send_to_char ("Which message on that board?\n", ch);
			return;
		}
	}

	else {
		for ( board_obj = ch->room->contents;
			  board_obj;
			  board_obj = board_obj->next_content ) {

			if ( !CAN_SEE_OBJ (ch, board_obj) )
				continue;

			if ( board_obj->obj_flags.type_flag == ITEM_BOARD )
				break;
		}

		if ( !board_obj ) {
			send_to_char ("You do not see a board here.\n", ch);
			return;
		}
	}

	msg_num = strtol(buf, NULL, 10);

	(void)one_argument (board_obj->name, buf);

	display_mysql_board_message (ch, buf, msg_num);
}

void do_examine (CHAR_DATA *ch, char *argument, int cmd)
{
	char		name [MAX_STRING_LENGTH] = { '\0' };
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	int		bits = 0;
	CHAR_DATA	*tmp_char = NULL;
	OBJ_DATA	*tmp_object = NULL;

	snprintf (buf, MAX_STRING_LENGTH,  "at %s", argument);
	do_look (ch, buf, 15);

	argument = one_argument (argument, name);

	if ( !*name ) {
		send_to_char ("Examine what?\n", ch);
		return;
	}

	bits = generic_find (name, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP,
						 ch, &tmp_char, &tmp_object);

	if ( tmp_object ) {
		if ( (GET_ITEM_TYPE (tmp_object)==ITEM_DRINKCON) ||
		     (GET_ITEM_TYPE (tmp_object)==ITEM_CONTAINER) ||
		     (GET_ITEM_TYPE (tmp_object)==ITEM_SHEATH) ) {
			send_to_char ("When you look inside, you see:\n", ch);
			snprintf (buf, MAX_STRING_LENGTH,  "in %s", name);
			do_look (ch, buf, 15);
		}
	}
}

void do_exits(CHAR_DATA *ch, char *argument, int cmd)
{
	int				dir = 0;
	ROOM_DIRECTION_DATA		*exit_room = NULL;
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	ROOM_DATA		*temp_room = NULL;
	
	char *exits[] =
	{	
		"North",
		"East ",
		"South",
		"West ",
		"Up   ",
		"Down "
	};

	if ( is_dark (ch->room) &&
		 IS_MORTAL (ch) &&
		 !get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS)) {
		send_to_char ("You can't see a thing!\n", ch);
		return;
	}

	if ( weather_info[ch->room->zone].state == HEAVY_SNOW && !IS_SET (ch->room->room_flags, INDOORS) && IS_MORTAL (ch) ) {
		send_to_char ("You can't see a thing!\n", ch);
		return;
	}

	*s_buf = '\0';

	for ( dir = 0; dir <= 5; dir++ ) {

		exit_room = PASSAGE (ch, dir);

		if ( !exit_room || exit_room->to_room == NOWHERE )
			continue;

		if ( !vtor (exit_room->to_room) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Room %d lead to nonexistant room %d (do_exits)\n",
					ch->in_room, exit_room->to_room);
			continue;
		}

		if ( IS_SET (exit_room->exit_info, PASSAGE_ISDOOR) ) {
			temp_room = vtor (exit_room->to_room);

			if ( IS_SET (exit_room->exit_info, PASSAGE_CLOSED) ) {

				if ( !ch->room->secrets [dir] ){
					snprintf (s_buf + strlen (s_buf), MAX_STRING_LENGTH, "%s -  %s (Closed)",
							 exits [dir], exit_room->keyword);
				}
				else if ( !IS_MORTAL (ch) && ch->room->secrets [dir] ){
					snprintf (s_buf + strlen (s_buf), MAX_STRING_LENGTH, "%s -  %s (Closed; Secret)",
							 exits [dir], exit_room->keyword);
				}

			}
			else if ( is_dark (temp_room) &&
						IS_MORTAL (ch) &&
						!get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS)){
				snprintf (s_buf + strlen (s_buf), MAX_STRING_LENGTH,
						 "%s - %s (Open) - Too dark to tell",
						 exits [dir], exit_room->keyword);
			}
			else{
				snprintf (s_buf + strlen(s_buf), MAX_STRING_LENGTH,
						 "%s - %s (Open) - %s",
						 exits [dir], exit_room->keyword,
						 temp_room->name);
			}
		}

		else {
			temp_room = vtor (exit_room->to_room);

			if ( is_dark (vtor (exit_room->to_room)) &&
				 IS_MORTAL (ch) &&
				 !get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS))
				snprintf (s_buf + strlen (s_buf), MAX_STRING_LENGTH,
						 "%s - Too dark to tell", exits [dir]);

			else{
				temp_room = vtor (exit_room->to_room);
				snprintf (s_buf + strlen (s_buf), MAX_STRING_LENGTH,
						 "%s - %s", exits [dir],
						 temp_room->name);
			}
		}

		if ( !IS_MORTAL (ch) ) {
			temp_room = vtor (exit_room->to_room);
			snprintf (s_buf + strlen(s_buf), MAX_STRING_LENGTH, " [%d]",
					 temp_room->virtual);
		}

		snprintf (s_buf + strlen (s_buf), MAX_STRING_LENGTH, "\n");
	}

	send_to_char ("Obvious exits:\n", ch);

	if ( *s_buf )
		send_to_char (s_buf, ch);
	else
		send_to_char ("None.\n", ch);
}

int get_stat_range(int score)
{
	if (score <= 4) return 0; /* horrible */
	if (score >= 5 && score <= 6) return 1; /* bad */
	if (score >= 7 && score <= 8) return 2; /* poor */
	if (score >= 9 && score <= 12) return 3; /* average */
	if (score >= 13 && score <= 14) return 4; /* good */
	if (score >= 15 && score <= 17) return 5; /* great */
	if (score >= 18) return 6; /* super */
	return 0; /* horrible as default */
	
}


int get_comestible_range(int num)
{
    if(num <= 0) return 0; /* starving or dying of thirst */
    if(num >= 1 && num <=5) return 1; /* hungry or parched */
    if(num >= 6 && num <=10) return 2; /* slightly hungry or thirsty */
    if(num >= 11 && num <=15) return 3; /* peckish or slightl thirtsy */
    if(num >= 16 && num <=19) return 4; /* full or quenched */
    if(num >= 20) return 5;  /* stuffed or sated */
    return 0;
}

void hunger_thirst_process (CHAR_DATA *ch) 
{
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		update_buf [MAX_STRING_LENGTH] = { '\0' };

	if ( IS_NPC (ch) )
		return;

	*update_buf = '\0';

	if ( get_comestible_range (ch->hunger) != get_comestible_range (ch->hunger-1) ) {
		snprintf (update_buf + strlen(update_buf), MAX_STRING_LENGTH, "%s", verbal_hunger [get_comestible_range (ch->hunger-1)]);
	}

	if ( get_comestible_range (ch->thirst) != get_comestible_range (ch->thirst-1) ) {
		if ( *update_buf )
			snprintf (update_buf + strlen(update_buf), MAX_STRING_LENGTH, ", and ");
		snprintf (update_buf + strlen(update_buf), MAX_STRING_LENGTH, "%s", verbal_thirst [get_comestible_range (ch->thirst-1)]);
	}

	if ( *update_buf ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#6You are %s.#0\n", update_buf);
		send_to_char (buf, ch);
	}

	if ( ch->hunger > 0 )
		ch->hunger--;
	if ( ch->thirst > 0 )
		ch->thirst--;

	if ( ch->thirst < -1 )
		ch->thirst = 1;
	if ( ch->hunger < -1 )
		ch->hunger = 1;
}

char *suffix (int number)
{
	if ( number == 1 )
		return "st";
	else if ( number == 2 )
		return "nd";
	else if ( number == 3 )
		return "rd";
	else if ( number < 20 )
		return "th";
	else if ( (number % 10) == 1 )
		return "st";
	else if ( (number % 10) == 2 )
		return "nd";
	else if ( (number % 10) == 3 )
		return "rd";
	else return "th";
}

void do_score(CHAR_DATA *ch, char *argument, int cmd)
{
	int			i = 0;
	int			weight = 0;
	int			clan_flags = 0;
	int			time_diff = 0;
	int			days_remaining = 0;
	int			hours_remaining = 0;
	struct time_info_data	playing_time;
	AFFECTED_TYPE		*af = NULL;
	AFFECTED_TYPE		*af_table = NULL;
	CLAN_DATA		*clan_def = NULL;
	CHAR_DATA		*rch = NULL;
	char			*p = NULL;
	char			clan_name [MAX_STRING_LENGTH] = { '\0' };
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	bool			first = TRUE;
	char			*temp_arg = NULL;
	struct time_info_data	birth_date;
	static char		*verbal_stats[] ={
				"horrible",
				"bad", 
				"poor", 
				"avg", 
				"good", 
				"great", 
				"super",
				"\n"};

/******** Birth date **********/
	birth_date = time_info;

	birth_date.minute -= age(ch).minute;
	if ( birth_date.minute < 0 ) {
		birth_date.hour -= 1;
		birth_date.minute += 60;
	}
	birth_date.hour -= age(ch).hour;
	if ( birth_date.hour < 0 ) {
		birth_date.day -= 1;
		birth_date.hour += 24;
	}	
	birth_date.day -= age(ch).day;
	if ( birth_date.day <= 0 ) {
		birth_date.month -= 1;
		birth_date.day += 30;
	}
	birth_date.month -= age(ch).month;
	if ( birth_date.month < 0 ) {
		birth_date.year -= 1;
		birth_date.month += 12;
	}
	birth_date.year -= age(ch).year;

	send_to_char ("\n", ch);
	if ( !IS_SET (ch->flags, FLAG_GUEST) )
		snprintf (buf, MAX_STRING_LENGTH,  "#2%s, a %d year old %s %s:#0\n",
					GET_NAME (ch),
					GET_AGE (ch),
					db_race_table[ch->race].name,
					sex_types [ch->sex]);
	else snprintf (buf, MAX_STRING_LENGTH,  "#2Guest Login:#0 %s\n\n", ch->tname);

	send_to_char(buf, ch);

/******* Attributes ****************/
	if ( !IS_SET (ch->flags, FLAG_GUEST) ) {
	if ( IS_MORTAL (ch) )
		snprintf (buf, MAX_STRING_LENGTH,  "Str[#2%s#0] Dex[#2%s#0] Con[#2%s#0] Int[#2%s#0] Wil[#2%s#0] Aur[#2%s#0] Agi[#2%s#0]\n",
		verbal_stats [get_stat_range (GET_STR (ch))],
		verbal_stats [get_stat_range (GET_DEX (ch))],
		verbal_stats [get_stat_range (GET_CON (ch))],
		verbal_stats [get_stat_range (GET_INT (ch))],
		verbal_stats [get_stat_range (GET_WIL (ch))],
		verbal_stats [get_stat_range (GET_AUR (ch))],
		verbal_stats [get_stat_range (GET_AGI (ch))]);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "Str[#2%d#0] Dex[#2%d#0] Con[#2%d#0] Int[#2%d#0] Wil[#2%d#0] Aur[#2%d#0] Agi[#2%d#0]\n",
		GET_STR(ch), GET_DEX(ch), GET_CON(ch), GET_INT(ch),
		GET_WIL(ch), GET_AUR(ch), GET_AGI(ch));

	send_to_char ("\n", ch);
	send_to_char (buf, ch);
	}

/******** Guest Sdesc ************/
	if ( IS_SET (ch->flags, FLAG_GUEST) ) {
		temp_arg = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "You have been incarnated as #5%s#0.\n\n", temp_arg);
		send_to_char (buf, ch);
		snprintf (buf, MAX_STRING_LENGTH,  "You are a #2%s#0 #2%s#0.\n",
					sex_types[ch->sex],
					db_race_table[ch->race].name);
		send_to_char (buf, ch);
	}

/******** Roleplay points ****************/
	if ( !IS_NPC (ch) && !IS_SET (ch->flags, FLAG_GUEST) && ch->desc && ch->desc->account && 
		ch->desc->account->roleplay_points > 0 && IS_SET (ch->desc->account->flags, ACCOUNT_RPPDISPLAY) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Your account has been awarded #2%d#0 roleplay points.\n", ch->desc->account->roleplay_points);
		send_to_char (buf, ch);
	}

/****** Hunger, thirst, and possibly intox. *******/
	snprintf (buf, MAX_STRING_LENGTH,  "You are #2%s#0, and #2%s#0.\n",
		ch->hunger >= 0 ? verbal_hunger [get_comestible_range(ch->hunger)] : "full",
		ch->thirst >= 0 ? verbal_thirst [get_comestible_range(ch->thirst)] : "quenched");
	send_to_char (buf, ch);

/******* Height and Weight ***********/
	if ( !IS_SET (ch->flags, FLAG_GUEST) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "You are #2%d#0 inches tall, and weigh #2%d#0 pounds.\n",
					  ch->height, get_weight (ch) / 100);
		send_to_char (buf, ch);

		snprintf (buf, MAX_STRING_LENGTH,  "You are of #2%s#0 build for one of your people, and wear size #2%s#0 (%s).\n",
					  frames [ch->frame],
					  sizes_named [get_size (ch)], sizes [get_size (ch)]);
		send_to_char (buf, ch);
	}

/****** Combat Mode ************/
	if ( !IS_SET (ch->flags, FLAG_PACIFIST) )
		snprintf (buf, MAX_STRING_LENGTH,  "When in combat, your mode is #2%s#0.\n",
						fight_tab [ch->fight_mode].name);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "You are currently in #2Pacifist#0 mode.\n");

	send_to_char (buf, ch);

	if ( GET_FLAG (ch, FLAG_AUTOFLEE) )
		send_to_char ("If in combat, you will #2FLEE#0.\n", ch);
		
/****** Speaking/Writing Language *********/

	snprintf (buf, MAX_STRING_LENGTH,  "You are currently speaking #2%s#0.\n", skill_data[ch->speaks].skill_name);
	send_to_char (buf, ch);

	if ( ch->writes ) {
		snprintf (buf, MAX_STRING_LENGTH,  "You are currently set to write in #2%s#0.\n", skill_data[ch->writes].skill_name);
		send_to_char (buf, ch);
	}

/****** Encumberance *************/
	for ( i = 0; i <= 3; i++ )
		if ( GET_STR (ch) * enc_tab [i].str_mult_wt >= IS_CARRYING_W (ch) )
			break;

	snprintf (buf, MAX_STRING_LENGTH,  "You are currently #2%s#0.\n", enc_tab [i].encumbered_status);
	send_to_char (buf, ch);

	weight = IS_CARRYING_W (ch);

	if ( weight < 1000 )
		weight = (weight + 50) / 100 * 100;
	else if ( weight < 10000 )
		weight = (weight + 500) / 1000 * 1000;
	else
		weight = (weight + 1250) / 2500 * 2500;

	if ( IS_CARRYING_W (ch) == 0 )
		send_to_char ("You are carrying nothing.\n", ch);
	else {
		snprintf (buf, MAX_STRING_LENGTH,  "You are carrying about #2%d#0 pounds.\n", weight / 100);
		send_to_char (buf, ch);
	}

/******** Birthdate actually used **********/
	if ( !IS_SET (ch->flags, FLAG_GUEST) ) {
		if ( birth_date.year > 0 ){
			temp_arg = suffix(birth_date.day);
			snprintf (buf, MAX_STRING_LENGTH,  "You were born on the #2%d%s#0 day of #2%s#0, #2%d#0.\n", birth_date.day, temp_arg, month_short_name[birth_date.month], birth_date.year);
		}
		else{
			temp_arg = suffix(birth_date.day);
			snprintf (buf, MAX_STRING_LENGTH,  "You were born on the #2%d%s#0 day of #2%s#0, many millenia past.\n", birth_date.day, temp_arg, month_short_name[birth_date.month]);
		}
		send_to_char (buf, ch);
		playing_time = real_time_passed (time (0) - ch->time.logon + ch->time.played, 0);
		snprintf (buf, MAX_STRING_LENGTH, "You have been playing for #2%d#0 days and #2%d#0 hours.\n",
			playing_time.day,
			playing_time.hour);
		send_to_char(buf, ch);
	}

/****** Position ***************/
	switch(GET_POS(ch)) {
		case POSITION_DEAD :
			send_to_char("You are DEAD!\n", ch); break;
		case POSITION_MORTALLYW :
			send_to_char("You are mortally wounded, and should seek help!\n", ch); break;
		case POSITION_UNCONSCIOUS :
			send_to_char("You are unconscious.\n", ch); break;
		case POSITION_STUNNED :
			send_to_char("You are stunned! You can't move!\n", ch); break;
		case POSITION_SLEEPING :
			af_table = get_affect (ch, MAGIC_SIT_TABLE);
			if ( !af_table || !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t, ch->room->contents) )
				send_to_char("You are sleeping.\n",ch);
			else {
				temp_arg = obj_short_desc(af_table->a.table.obj);
				snprintf (buf, MAX_STRING_LENGTH,  "You are sleeping at #2%s#0.\n", temp_arg);
				send_to_char(buf, ch);
			}
		break;
		case POSITION_RESTING  :
			af_table = get_affect (ch, MAGIC_SIT_TABLE);
			if ( !af_table || !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t, ch->room->contents) )
				send_to_char("You are resting.\n",ch);
			else if ( af_table && af_table->a.table.obj ) {
				temp_arg = obj_short_desc(af_table->a.table.obj);
				snprintf (buf, MAX_STRING_LENGTH,  "You are resting at #2%s#0.\n", temp_arg);
				send_to_char(buf, ch);
			}
		break;
		case POSITION_SITTING  :
			af_table = get_affect (ch, MAGIC_SIT_TABLE);
			if ( !af_table || !is_obj_in_list ((OBJ_DATA *) af_table->a.spell.t, ch->room->contents) )
				send_to_char("You are sitting.\n",ch);
			else {
				temp_arg = obj_short_desc(af_table->a.table.obj);
				snprintf (buf, MAX_STRING_LENGTH,  "You are sitting at #2%s#0.\n", temp_arg);
				send_to_char(buf, ch);
			}
			break;
		case POSITION_FIGHTING :
			if (ch->fighting) {
				send_to_char ("\n", ch);
				act("You are fighting $N.", FALSE, ch, 0,
				     ch->fighting, TO_CHAR);
				
				for ( rch = ch->room->people; rch; rch = rch->next_in_room ) {
					if ( rch->fighting == ch && rch != ch->fighting)				
						act("You are also being attacked by $N.", FALSE, ch, 0,
							rch, TO_CHAR);
				}

				send_to_char("\n",ch);
			}
			else
				send_to_char("You are fighting thin air.\n", ch);
			break;
		case POSITION_STANDING :
			send_to_char("You are standing.\n",ch); break;
		default :
			send_to_char("You are floating.\n",ch); break;
	}

/***** Guarding and Guarded **********/
	for ( rch = ch->room->people; rch; rch = rch->next_in_room ) {
		if ( (af = get_affect (rch, MAGIC_GUARD)) && af->a.spell.t == (int) ch ) {
			if ( first )
				send_to_char ("\n", ch);
			act ("You are being guarded by $N.", FALSE, ch, 0, rch, TO_CHAR);
			first = FALSE;
		}
	}

	if ( first == FALSE )
		send_to_char("\n",ch);
		
	if ( get_affect (ch, AFFECT_GUARD_DIR) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "You are currently guarding the %s exit.\n", dirs[get_affect(ch,AFFECT_GUARD_DIR)->a.shadow.edge]);
		send_to_char (buf, ch);
	}


	if ( !IS_NPC (ch) && ch->pc->create_state == 4 )
		send_to_char ("You are DEAD.\n", ch);

/****** Travel speed ********/
	
	snprintf (buf, MAX_STRING_LENGTH,  "You #2%s#0 when you travel.\n", speeds [ch->speed]);
	send_to_char (buf, ch);


/****** Craft Timer **********/
	if ( (af = get_affect (ch, MAGIC_CRAFT_DELAY)) && time(0) < af->a.spell.modifier ) {
		time_diff = (int)(af->a.spell.modifier - time(0));
		days_remaining = time_diff / (60 * 60 * 24);
		time_diff %= 60 * 60 * 24;
		hours_remaining = time_diff / (60 * 60);
		if ( !days_remaining && !hours_remaining ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Your craft timer has #2less than 1#0 RL hour remaining.\n");
		} else {	
			snprintf (buf, MAX_STRING_LENGTH,  "Your craft timer has approximately");
			if ( days_remaining )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " #2%d#0 RL day%s ", days_remaining, days_remaining > 1 ? "s" : "");
			if ( hours_remaining && days_remaining )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "and");
			if ( hours_remaining )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " #2%d#0 RL hour%s ", hours_remaining, hours_remaining > 1 ? "s" : "");
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "remaining.\n");
		}
		send_to_char (buf, ch);
	}

/***** Spells and other affects for Immortals ****/
	if ( !IS_MORTAL (ch) ) {
		if ( !ch->hour_affects )
			send_to_char ("No spells affect you.\n", ch);
		else {
			for ( af = ch->hour_affects; af; af = af->next ) {
				if ( af->type == AFFECT_SHADOW ) {
					if ( !af->a.shadow.shadow && af->a.shadow.edge != -1 )
						snprintf (buf, MAX_STRING_LENGTH,  "   Standing");

					else if ( !is_he_somewhere (af->a.shadow.shadow) )
						continue;

					else if ( IS_NPC (af->a.shadow.shadow) )
						snprintf (buf, MAX_STRING_LENGTH,  "   Shadowing %s (%d)",
								af->a.shadow.shadow->short_descr,
								af->a.shadow.shadow->mob->virtual);
					else
						snprintf (buf, MAX_STRING_LENGTH,  "   Shadowing PC %s",
								GET_NAME (af->a.shadow.shadow));

					if ( af->a.shadow.edge != -1 )
						snprintf (ADDBUF, MAX_STRING_LENGTH, " on %s edge.",
								dirs [af->a.shadow.edge]);

					strcat (buf, "\n");

					send_to_char (buf, ch);

					continue;
				} /* if ( af->type == AFFECT_SHADOW ) */

				else if ( af->type == MAGIC_SIT_TABLE ) {
					snprintf (buf, MAX_STRING_LENGTH,  "   Sitting at table affect.\n");
					send_to_char (buf, ch);
					continue;
				}

	/**** Payday info for Immortals ***/
				if ( af->type >= JOB_1 && af->type <= JOB_3 ) {

					i = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;
					i = af->a.job.pay_date - i;

					snprintf (buf, MAX_STRING_LENGTH,  "   Job %d:  Pays %d coins (%d of %d days until "
							  "payday)\n"
							  "                 %d x vnum %s (%d)\n",
								af->type - JOB_1 + 1, af->a.job.cash,
								i, af->a.job.days,
								af->a.job.count,
								!vtoo (af->a.job.object_vnum) ?
									"UNDEFINED" : vtoo (af->a.job.object_vnum)->
												short_description,
								af->a.job.object_vnum);
					send_to_char (buf, ch);
					continue;
				}
	/*** Wanted info for Immortals ***/
				if ( af->type >= MAGIC_CRIM_BASE &&
					 af->type <= MAGIC_CRIM_BASE + 100 ) {
					snprintf (buf, MAX_STRING_LENGTH,  "   Wanted in zone %d for %d hours.\n",
						af->type - MAGIC_CRIM_BASE, af->a.spell.duration);
					send_to_char (buf, ch);
					continue;
				}

				if ( af->type >= MAGIC_CRIM_HOODED &&
					 af->type < MAGIC_CRIM_HOODED + 100 ) {
					snprintf (buf, MAX_STRING_LENGTH,  "   Hooded criminal charge in zone %d for "
								  "%d RL seconds.\n",
							 af->type - MAGIC_CRIM_HOODED, 
							 af->a.spell.duration);
					send_to_char (buf, ch);
					continue;
				}

				if ( af->type == MAGIC_STARED ) {
					snprintf (buf, MAX_STRING_LENGTH,  "%d   Studied by an enforcer.  Won't be "
								  "studied for %d RL seconds.\n",
							 af->type, af->a.spell.duration);
					send_to_char (buf, ch);
					continue;
				}

				if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST )
					continue;

				if ( af->type == MAGIC_CRAFT_DELAY || MAGIC_CRAFT_BRANCH_STOP )
					continue;

				if ( af->type == MAGIC_HIDDEN ) {
					snprintf (buf, MAX_STRING_LENGTH,  "%d   Hidden.\n", af->type);
					send_to_char (buf, ch);
					continue;   
				} 

				if ( af->type == MAGIC_SNEAK ) {
					snprintf (buf, MAX_STRING_LENGTH,  "%d   Currently trying to sneak.\n",
								  af->type);
					send_to_char (buf, ch);
					continue;   
				} 
			}
		}
	} /* of Immortal extra information */

/****** Mortal extra information *******/
	if ( get_equip (ch, WEAR_BLINDFOLD) )
		send_to_char ("YOU ARE BLINDFOLDED!\n", ch);
	else if ( is_blind (ch) )
		send_to_char ("YOU ARE BLIND!\n", ch);

	if ( IS_SET (ch->affected_by, AFF_HOODED) )
		send_to_char ("You are currently cloaked and hooded.\n", ch);

	if ( ch->voice_str ) {
		send_to_char ("\nYour current voice string: (#2", ch);
		send_to_char (ch->voice_str, ch);
		send_to_char ("#0)\n", ch);
	}

	if ( ch->travel_str ) {
		if ( !ch->voice_str )
			send_to_char ("\n", ch);
		send_to_char ("Your current travel string: (#2", ch);
		send_to_char (ch->travel_str, ch);
		send_to_char ("#0)\n", ch);
	}

	if ( ch->pmote_str ) {
		if ( !ch->voice_str && !ch->travel_str )
			send_to_char ("\n", ch);
		send_to_char ("Your current pmote: (#2", ch);
		send_to_char (ch->pmote_str, ch);
		send_to_char ("#0)\n", ch);
	}

/***** Clan information ****/
	if ( ch->clans && *ch->clans ) {

		send_to_char ("\nYou are affiliated with the following clans:\n\n", ch);
		
		p = ch->clans;

		while ( get_next_clan (&p, clan_name, &clan_flags) ) {

			if ( (clan_def = get_clandef (clan_name)) )
				strcpy (clan_name, clan_def->literal);

			snprintf (buf, MAX_STRING_LENGTH,  "  %-40s ", clan_name);

			if ( IS_SET (clan_flags, CLAN_LEADER) )
				strcat (buf, "#2(Leader)#0  ");
			else if ( IS_SET (clan_flags, CLAN_RECRUIT) )
				strcat (buf, "#2(Recruit)#0  ");
			else if ( IS_SET (clan_flags, CLAN_RECRUIT) )
				strcat (buf, "#2(Recruit)#0  ");
			else if ( IS_SET (clan_flags, CLAN_PRIVATE) )
				strcat (buf, "#2(Private)#0  ");
			else if ( IS_SET (clan_flags, CLAN_CORPORAL) )
				strcat (buf, "#2(Corporal)#0  ");
			else if ( IS_SET (clan_flags, CLAN_SERGEANT) )
				strcat (buf, "#2(Sergeant)#0  ");
			else if ( IS_SET (clan_flags, CLAN_LIEUTENANT) )
				strcat (buf, "#2(Lieutenant)#0  ");
			else if ( IS_SET (clan_flags, CLAN_CAPTAIN) )
				strcat (buf, "#2(Captain)#0  ");
			else if ( IS_SET (clan_flags, CLAN_GENERAL) )
				strcat (buf, "#2(General)#0  ");
			else if ( IS_SET (clan_flags, CLAN_COMMANDER) )
				strcat (buf, "#2(Commander)#0  ");
			else if ( IS_SET (clan_flags, CLAN_APPRENTICE) )
				strcat (buf, "#2(Apprentice)#0  ");
			else if ( IS_SET (clan_flags, CLAN_JOURNEYMAN) )
				strcat (buf, "#2(Journeyman)#0  ");
			else if ( IS_SET (clan_flags, CLAN_MASTER) )
				strcat (buf, "#2(Master)#0  ");
			else
				strcat (buf, "#2(Member)#0  ");

			strcat (buf, "\n");
			send_to_char (buf, ch);
		}
	}
}



void do_skills (CHAR_DATA *ch, char *argument, int cmd)
{
	int			i = 0;
	int			j = 2;
	int			loaded_char = 0;
	CHAR_DATA		*who = NULL;
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	char			buf2 [MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if ( IS_MORTAL (ch) || !*buf )
		who = ch;
	else if ( !(who = get_char_room_vis (ch, buf)) ) {
		if ( !(who = load_pc (buf)) ) {
			send_to_char ("No body here by that name.\n", ch);
			return;
		} else
			loaded_char = 1;
	}

	*buf = '\0';

	if ( IS_MORTAL (ch) ) {

		snprintf (buf, MAX_STRING_LENGTH,  "\n     #2Offense:#0 %s ", skill_lev (who->offense));

		for ( i = 1; i <= LAST_SKILL; i++ ) {

			if ( !real_skill (who, i) )
				continue;

			snprintf (buf2, MAX_STRING_LENGTH,  "#2%12.12s:#0 %s ",
						  skill_data[i].skill_name, skill_lev (who->skills [i]));

			while ( strlen (buf2) < 18 )
				strcat (buf2, " ");

			strcat (buf, buf2);

			if ( !(j % 3) )
				strcat(buf,"\n");
			j++;
		}
	}

	else {
		snprintf (buf, MAX_STRING_LENGTH,  " #2Offense#0[%03d/%03d] ", who->offense, 50);

		for ( i = 1; i <= LAST_SKILL; i++ ) {
			if ( real_skill (who, i) ) {
				snprintf (ADDBUF, MAX_STRING_LENGTH,
					     "#2%8.8s#0[%03d/%03d] ", skill_data[i].skill_name, who->skills [i],
						 calc_lookup (who, REG_CAP, i));
				if ( !(j % 4) )
					strcat(buf,"\n");
				j++;
			}
		}
	}

	send_to_char (buf, ch);

	if ( buf [strlen(buf)-1] != '\n' )
		send_to_char ("\n", ch);

	if ( loaded_char )
		unload_pc (who);
}

char *time_string (CHAR_DATA *ch)
{
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		*suf = NULL;
	int			day = 0;
	int			minutes = 0;
	int     	high_sun = 0;
    char    	day_buf[25];
    char    	phrase[MAX_STRING_LENGTH];
    static char 	time_str [MAX_STRING_LENGTH] = { '\0' };

	minutes = (int)(4 * (15 * 60 - (next_hour_update - time (0))) / 60);
	
   	high_sun = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);

	snprintf (phrase, MAX_STRING_LENGTH,  "[report error: %d]", time_info.hour);

    	if (time_info.hour == sunset[time_info.month] - 2)
        	snprintf (phrase, MAX_STRING_LENGTH, "late afternoon");
    	else if (time_info.hour == sunset[time_info.month] - 1)
        	snprintf (phrase, MAX_STRING_LENGTH, "dusk");
	else if (time_info.hour == sunset[time_info.month])
		snprintf (phrase, MAX_STRING_LENGTH, "twilight");
    	else if (time_info.hour == 23)
        	snprintf (phrase, MAX_STRING_LENGTH, "before midnight");
    	else if (time_info.hour == 0)
        	snprintf (phrase, MAX_STRING_LENGTH, "midnight");
    	else if (time_info.hour == 1)
        	snprintf (phrase, MAX_STRING_LENGTH, "after midnight");
    	else if (time_info.hour == sunrise[time_info.month] - 1)
        	snprintf (phrase, MAX_STRING_LENGTH, "before dawn");
    	else if (time_info.hour == sunrise[time_info.month])
        	snprintf (phrase, MAX_STRING_LENGTH, "dawn");
    	else if (time_info.hour == sunrise[time_info.month] + 1)
        	snprintf (phrase, MAX_STRING_LENGTH, "early morning");
    	else if (time_info.hour == high_sun - 1)
        	snprintf (phrase, MAX_STRING_LENGTH, "late morning");
    	else if (time_info.hour == high_sun)
        	snprintf (phrase, MAX_STRING_LENGTH, "high sun");
    	else if (time_info.hour == high_sun + 1)
        	snprintf (phrase, MAX_STRING_LENGTH, "early afternoon");

	else if (time_info.hour > high_sun && time_info.hour < sunset[time_info.month] - 2 )
		snprintf (phrase, MAX_STRING_LENGTH,  "afternoon");
    	else if (time_info.hour > sunrise[time_info.month] + 1 && time_info.hour < high_sun - 1)
        	snprintf (phrase, MAX_STRING_LENGTH, "morning");
    	else if (time_info.hour >= sunset[time_info.month] && time_info.hour < 21)
        	snprintf (phrase, MAX_STRING_LENGTH, "evening");
    	else if (time_info.hour >= sunset[time_info.month] && time_info.hour < 23)
        	snprintf (phrase, MAX_STRING_LENGTH, "night time");
    	else if (time_info.hour > high_sun + 1 && time_info.hour < sunset[time_info.hour] - 2)
        	snprintf (phrase, MAX_STRING_LENGTH, "afternoon");
    	else if ( time_info.hour > 1 && time_info.hour < sunrise[time_info.month] - 1 )
        	snprintf (phrase, MAX_STRING_LENGTH, "late at night");

	snprintf (buf, MAX_STRING_LENGTH,  "It is %s ",phrase);

        if (ch && !IS_MORTAL(ch)) {  
                snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "[%d:%s%d %s] ",
                                ((time_info.hour % 12 == 0) ? 12 : ((time_info.hour) % 12)),
                                minutes >= 10 ? "" : "0", minutes,
                                ((time_info.hour >= 12) ? "pm" : "am") );
        }

	day = time_info.day + 1;   /* day in [1..35] */

        if ( day == 1 )
                suf = "st";
        else if ( day == 2 )
                suf = "nd";
        else if ( day == 3 )
                suf = "rd";
        else if ( day < 20 )
                suf = "th";
        else if ( (day % 10) == 1 )
                suf = "st";
        else if ( (day % 10) == 2 )
                suf = "nd";
        else if ( (day % 10) == 3 )
                suf = "rd";
        else
                suf = "th";

	snprintf (day_buf, MAX_STRING_LENGTH, "%d%s", day, suf);
		
	/* Special output for holidays */

	if ( time_info.holiday == 0 &&
		!(time_info.month == 1 && day == 12) &&
		!(time_info.month == 4 && day == 10) &&
		!(time_info.month == 7 && day == 11) &&
		!(time_info.month == 10 && day == 12) )
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "on the %s day of the %s,", day_buf, month_name [(int)time_info.month]);

	else {
		if ( time_info.holiday > 0 ) {
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "on %s,", holiday_names[time_info.holiday]);
		}
		else if ( time_info.month == 1 && day == 12 )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "on Erukyerme, The Prayer to Eru,");
		else if ( time_info.month == 4 && day == 10 )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "on Lairemerende, The Greenfest,");
		else if ( time_info.month == 7 && day == 11 )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "on Eruhantale, Thanksgiving to Eru,");
		else if ( time_info.month == 10 && day == 12 )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "on Airilaitale, The Hallowmas,");
	}

 	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s in the year %d of the Steward's Reckoning.\n",
		season_string [(int)time_info.month],
		time_info.year);

	snprintf (time_str, MAX_STRING_LENGTH, "%s", buf);

	return time_str;
}

void do_time (CHAR_DATA *ch, char *argument, int cmd)
{
	char	buf [MAX_STRING_LENGTH] = { '\0' };
	char	*p = NULL;
	char	*temp_arg = NULL;
	
	temp_arg = time_string(ch);
	snprintf (buf, MAX_STRING_LENGTH,  "\n#6%s#0", temp_arg);

	reformat_string (buf, &p);

	send_to_char ("\n", ch);
	send_to_char (p, ch);

	mem_free (p);
	p = NULL;

	if ( GET_TRUST (ch) ) {
		send_to_char ("\nThe following variables apply:\n", ch);

		if ( IS_LIGHT (ch->room) )
			send_to_char ("   IS_LIGHT is true.\n", ch);
		else
			send_to_char ("   IS_LIGHT is false.\n", ch);

		if ( OUTSIDE (ch) )
			send_to_char ("   OUTSIDE is true.\n", ch);
		else
			send_to_char ("   OUTSIDE is false.\n", ch);

		if ( IS_NIGHT )
			send_to_char ("   IS_NIGHT is true.\n", ch);
		else
			send_to_char ("   IS_NIGHT is false.\n", ch);

		snprintf (buf, MAX_STRING_LENGTH,  "   Light count in room:  %d\n", ch->room->light);
		send_to_char (buf, ch);
	}
}

void do_weather (CHAR_DATA *ch, char *argument, int cmd)
{
	int				ind = 0;
    char        	w_phrase[50] = { '\0' };
   	char        	buf     [MAX_STRING_LENGTH] = { '\0' };
    char			buf2    [MAX_STRING_LENGTH] = { '\0' };
	char			imm_buf [MAX_STRING_LENGTH];
    int         	wind_case = 0;
    int         	temp_case = 0;
    char        	wind    [20] = { '\0' };
    int         	high_sun = 0;
	AFFECTED_TYPE	*room_af = NULL;

	argument = one_argument (argument, buf);


	if ( !IS_MORTAL (ch) && GET_TRUST(ch) > 4 && *buf) {
		if ( (ind = index_lookup (weather_states, buf)) != -1 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "You have changed the weather state to #6%s#0.\n", weather_states [ind]);
			send_to_char (buf, ch);
			weather_info[ch->room->zone].state = ind;
			return;
		}
		else if ( (ind = index_lookup (weather_clouds, buf)) != -1 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "You have changed cloud state to #6%s#0.\n", weather_clouds [ind]);
			send_to_char (buf, ch);
			weather_info[ch->room->zone].clouds = ind;
			return;
		}
		else if ( (ind = index_lookup (wind_speeds, buf)) != -1 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "You have changed wind speed to #6%s#0.\n", wind_speeds [ind]);
			send_to_char (buf, ch);
			weather_info[ch->room->zone].wind_speed = ind;
			return;
		}
		else if ( (ind = index_lookup (fog_states, buf)) != -1 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "You have changed the fog level to #6%s#0.\n", fog_states [ind]);
			send_to_char (buf, ch);
			weather_info[ch->room->zone].fog = ind;
			return;
		}
		send_to_char ("That is not a recognized weather state.\n", ch);
		return;
	}

	if ( OUTSIDE(ch) ) {

		if ( weather_info[ch->room->zone].temperature < 120 ) {
			wind_case = 0;
			temp_case = 1;
		}
		if ( weather_info[ch->room->zone].temperature < 110 ) {
			temp_case = 2;
			wind_case = 1;
		}
        	if ( weather_info[ch->room->zone].temperature < 100 ) {
			wind_case = 2;
            		temp_case = 3;
		}
        	if ( weather_info[ch->room->zone].temperature < 94 ) {
            		wind_case = 3;
            		temp_case = 3;
        	}
        	if ( weather_info[ch->room->zone].temperature < 90 )
            		temp_case = 4;
        	if ( weather_info[ch->room->zone].temperature < 80 ) {
            		wind_case = 4;
            		temp_case = 5;
        	}
        	if ( weather_info[ch->room->zone].temperature < 75 )
            		temp_case = 5;
        	if ( weather_info[ch->room->zone].temperature < 65 ) {
            		wind_case = 5;
            		temp_case = 6;
        	}
        	if ( weather_info[ch->room->zone].temperature < 55 ) {
            		temp_case = 7;
            		wind_case = 6;
        	}
        	if ( weather_info[ch->room->zone].temperature < 47 ) {
            		wind_case = 7;
            		temp_case = 8;
        	}
        	if ( weather_info[ch->room->zone].temperature < 38 )
            		temp_case = 9;
        	if ( weather_info[ch->room->zone].temperature < 33 ) {
            		wind_case = 8;
            		temp_case = 10;
        	}
        	if ( weather_info[ch->room->zone].temperature < 21 ) {
			wind_case = 9;
            		temp_case = 11;
		}
		if ( weather_info[ch->room->zone].temperature < 11 )
			temp_case = 12;
		if ( weather_info[ch->room->zone].temperature < 1 ) {
			wind_case = 10;
			temp_case = 13;
		}
		if ( weather_info[ch->room->zone].temperature < -10 )
			temp_case = 14;
        
	*buf = '\0';
	*buf2 = '\0';

        high_sun = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);
        
        if (time_info.hour >= sunrise[time_info.month] && time_info.hour <= high_sun - 2)
            snprintf(w_phrase, MAX_STRING_LENGTH, "morning");
        else if (time_info.hour > high_sun - 2 && time_info.hour <= high_sun + 1)
            snprintf(w_phrase, MAX_STRING_LENGTH, "day");
        else if (time_info.hour > high_sun + 1 && time_info.hour < sunset[time_info.month])
            snprintf(w_phrase, MAX_STRING_LENGTH, "afternoon");
        else if (time_info.hour >= sunset[time_info.month] && time_info.hour < 21)
            snprintf(w_phrase, MAX_STRING_LENGTH, "evening");
        else
            snprintf(w_phrase, MAX_STRING_LENGTH, "night");

        if (time_info.season == SPRING)
            snprintf (buf2, MAX_STRING_LENGTH, "spring");
        if (time_info.season == SUMMER)
            snprintf (buf2, MAX_STRING_LENGTH, "summer");
        if (time_info.season == AUTUMN)
            snprintf (buf2, MAX_STRING_LENGTH, "autumn");
        if (time_info.season == WINTER)
            snprintf (buf2, MAX_STRING_LENGTH, "winter");

	*imm_buf = '\0';

	if ( !IS_MORTAL(ch) )
		snprintf (imm_buf, MAX_STRING_LENGTH, " [%d F]", weather_info[ch->room->zone].temperature);

        switch (temp_case) {
		case 0:		snprintf (buf, MAX_STRING_LENGTH,  "It is a dangerously searing %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 1:  	snprintf (buf, MAX_STRING_LENGTH,  "It is a painfully blazing %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 2:  	snprintf (buf, MAX_STRING_LENGTH,  "It is a blistering %s %s%s.\n", buf2, w_phrase, imm_buf); break;
		case 3:		snprintf (buf, MAX_STRING_LENGTH,  "It is a sweltering %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 4:  	snprintf (buf, MAX_STRING_LENGTH,  "It is a hot %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 5:  	snprintf (buf, MAX_STRING_LENGTH,  "It is a temperate %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 6:  	snprintf (buf, MAX_STRING_LENGTH,  "It is a cool %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 7:  	snprintf (buf, MAX_STRING_LENGTH,  "It is a chill %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 8:  	snprintf (buf, MAX_STRING_LENGTH,  "It is a cold %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 9:  	snprintf (buf, MAX_STRING_LENGTH,  "It is a very cold %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 10: 	snprintf (buf, MAX_STRING_LENGTH,  "It is a frigid %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 11: 	snprintf (buf, MAX_STRING_LENGTH,  "It is a freezing %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 12: 	snprintf (buf, MAX_STRING_LENGTH,  "It is a numbingly frigid %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 13: 	snprintf (buf, MAX_STRING_LENGTH,  "It is a painfully freezing %s %s%s.\n", buf2, w_phrase, imm_buf); break;
            	case 14: 	snprintf (buf, MAX_STRING_LENGTH,  "It is a dangerously freezing %s %s%s.\n", buf2, w_phrase, imm_buf); break;

            	default:  	snprintf (buf, MAX_STRING_LENGTH,  "It is a cool %s %s%s.\n", buf2, w_phrase, imm_buf); break;
        }

        send_to_char(buf,ch);

	*buf = '\0';
	*buf2 = '\0';

        if (weather_info[ch->room->zone].state >= LIGHT_RAIN) {
            if (weather_info[ch->room->zone].wind_speed == STORMY)
                snprintf(w_phrase, MAX_STRING_LENGTH, "rolling");
            else if (weather_info[ch->room->zone].wind_speed > BREEZE)
                snprintf(w_phrase, MAX_STRING_LENGTH, "flowing");
            else
                snprintf(w_phrase, MAX_STRING_LENGTH, "brooding");
                                
            switch (weather_info[ch->room->zone].clouds) {
                case LIGHT_CLOUDS: snprintf (buf2, MAX_STRING_LENGTH, "a scattering of grey clouds.\n"); break;
                case HEAVY_CLOUDS: snprintf (buf2, MAX_STRING_LENGTH, "dark, %s clouds.\n",w_phrase); break;
                case OVERCAST:     snprintf (buf2, MAX_STRING_LENGTH, "a thick veil of %s storm clouds.\n",w_phrase); break;
            }
            
            if (weather_info[ch->room->zone].fog == THICK_FOG)
                snprintf (buf2, MAX_STRING_LENGTH, " the fog-shrouded sky.\n");

            switch (weather_info[ch->room->zone].state) {
                case LIGHT_RAIN:    snprintf (buf, MAX_STRING_LENGTH, "A light drizzle is falling from %s",buf2); break;
                case STEADY_RAIN:   snprintf (buf, MAX_STRING_LENGTH, "A steady rain is falling from %s",buf2); break;
                case HEAVY_RAIN:    snprintf (buf, MAX_STRING_LENGTH, "A shower of rain is pouring from %s",buf2); break;
                case LIGHT_SNOW:    snprintf (buf, MAX_STRING_LENGTH, "A light snow is falling from %s",buf2); break;
                case STEADY_SNOW:   snprintf (buf, MAX_STRING_LENGTH, "Snow is falling from %s",buf2); break;
                case HEAVY_SNOW:    snprintf (buf, MAX_STRING_LENGTH, "Blinding snow swarms down from an obscured white sky.\n"); break;
            }
            send_to_char(buf,ch);
        }
        
	*buf = '\0';
	*buf2 = '\0';

        if (weather_info[ch->room->zone].wind_speed) {
            if (weather_info[ch->room->zone].wind_dir == DIR_NORTH_WIND)
                wind_case++;

            switch (wind_case) {

		case 0: snprintf (buf, MAX_STRING_LENGTH,  "searing");	break;
                case 1: snprintf (buf, MAX_STRING_LENGTH,  "scorching");    	break;
                case 2: snprintf (buf, MAX_STRING_LENGTH,  "sweltering");   	break;
                case 3: snprintf (buf, MAX_STRING_LENGTH,  "hot");   	break;
                case 4: snprintf (buf, MAX_STRING_LENGTH,  "warm");  	break;
                case 5: snprintf (buf, MAX_STRING_LENGTH,  "cool");   	break;
                case 6: snprintf (buf, MAX_STRING_LENGTH,  "chill"); 	break;
                case 7: snprintf (buf, MAX_STRING_LENGTH,  "cold"); 		break;
                case 8: snprintf (buf, MAX_STRING_LENGTH,  "frigid"); 	break;
                case 9: snprintf (buf, MAX_STRING_LENGTH,  "freezing"); 	break;
                case 10: snprintf (buf, MAX_STRING_LENGTH,  "arctic"); 	break;

		default: snprintf (buf, MAX_STRING_LENGTH,  "cool"); 	break;
            }
        
            if (weather_info[ch->room->zone].wind_dir == DIR_NORTH_WIND)
                snprintf(wind, MAX_STRING_LENGTH, "%s northerly",buf);
            else
                snprintf(wind, MAX_STRING_LENGTH, "%s westerly",buf);
        }

	*buf = '\0';
	*buf2 = '\0';

        if (weather_info[ch->room->zone].state >= LIGHT_RAIN || !weather_info[ch->room->zone].clouds || weather_info[ch->room->zone].fog == THICK_FOG) {
            switch (weather_info[ch->room->zone].wind_speed) {
                case CALM:   snprintf (buf, MAX_STRING_LENGTH, "The air is calm and quiet.\n"); break;
                case BREEZE: snprintf (buf, MAX_STRING_LENGTH, "There is a %s breeze.\n",wind); break;
                case WINDY:  snprintf (buf, MAX_STRING_LENGTH, "There is a %s wind.\n",wind); break;
                case GALE:   snprintf (buf, MAX_STRING_LENGTH, "A %s gale is blowing.\n",wind); break;
                case STORMY: snprintf (buf, MAX_STRING_LENGTH, "A %s wind whips and churns in a stormy fury.\n",wind); break;
            }
            send_to_char(buf,ch);
        }
        
	*buf = '\0';
	*buf2 = '\0';

        if (weather_info[ch->room->zone].state < LIGHT_RAIN && weather_info[ch->room->zone].clouds && weather_info[ch->room->zone].fog < THICK_FOG) {
            if (weather_info[ch->room->zone].state == NO_RAIN)
                snprintf(w_phrase, MAX_STRING_LENGTH, "rain");
            else
                snprintf(w_phrase, MAX_STRING_LENGTH, "white");

            switch (weather_info[ch->room->zone].clouds) {
                case LIGHT_CLOUDS: snprintf (buf2, MAX_STRING_LENGTH, "Wispy %s clouds",w_phrase); break;
                case HEAVY_CLOUDS: snprintf (buf2, MAX_STRING_LENGTH, "Heavy %s clouds",w_phrase); break;
                case OVERCAST:     snprintf (buf2, MAX_STRING_LENGTH, "A veil of thick %s clouds",w_phrase); break;
            }
            
            switch (weather_info[ch->room->zone].wind_speed) {
                case CALM:      snprintf (buf, MAX_STRING_LENGTH, "%s hang motionless in the sky.\n",buf2); break;
                case BREEZE:    snprintf (buf, MAX_STRING_LENGTH, "%s waft overhead upon a %s breeze.\n",buf2,wind); break;
                case WINDY:     snprintf (buf, MAX_STRING_LENGTH, "%s waft overhead upon the %s winds.\n",buf2,wind); break;
                case GALE:      snprintf (buf, MAX_STRING_LENGTH, "%s rush overhead upon a %s gale.\n",buf2,wind); break;
                case STORMY:    snprintf (buf, MAX_STRING_LENGTH, "%s churn violently in the sky upon the %s winds.\n",buf2,wind); break;
            }
            send_to_char(buf,ch);
        }

	*buf = '\0';
	*buf2 = '\0';
        if (weather_info[ch->room->zone].fog && !(weather_info[ch->room->zone].state >= LIGHT_RAIN && weather_info[ch->room->zone].fog == THICK_FOG)) {
            if (weather_info[ch->room->zone].fog == THIN_FOG)
                send_to_char("A patchy fog floats in the air.\n",ch);
            else
                send_to_char("A thick fog lies heavy upon the land.\n",ch);
        }

	if ( (room_af = is_room_affected (world_affects, MAGIC_WORLD_CLOUDS)) &&
		 (OUTSIDE (ch) || !IS_MORTAL (ch)) ) {
		send_to_char ("Looming black clouds cover the sky, blotting out the sun.\n", ch);
	}

	if ( moon_light >= 1 ) {
		if ( !sun_light )
			send_to_char ("A full and gleaming Ithil limns the area in ghostly argent radiance.\n", ch);
		else
			send_to_char ("Ithil's ethereal silhouette is barely visible in the daylight.\n", ch);
	}

    } else
		send_to_char("You can not see outside from here.\n", ch);
}

HELP_DATA *is_help (HELP_DATA *list, char *topic)
{
	HELP_DATA		*element = NULL;

	for ( element = list; element; element = element->next ) {

		if ( !element->master_element )
			continue;

		if ( !str_cmp (element->keyword, topic) )
				return element->master_element;
	}

	return NULL;
}



void post_help (DESCRIPTOR_DATA *d)
{
	char		*date = NULL;

	mysql_safe_query ("DELETE FROM helpfiles WHERE category = '%s' AND name = '%s'",
	d->character->delay_who, d->character->delay_who2);

	date = timestr(date);
	
	if ( !*d->pending_message->message ) {
		send_to_char ("No help entry written.\n", d->character);
		d->pending_message = NULL;
		d->character->delay_who = NULL;
		d->character->delay_who2 = NULL;
		d->character->delay_info1 = 0;
		mem_free (date);
		return;
	}

	*d->character->delay_who = toupper(*d->character->delay_who);
	*d->character->delay_who2 = toupper(*d->character->delay_who2);

	mysql_safe_query ("INSERT INTO helpfiles VALUES ('%s', '%s', '\n%s\n', '(null)', %d, '%s', '%s')",
	d->character->delay_who2, d->character->delay_who, d->pending_message->message, 
	d->character->delay_info1, date, d->character->tname);

	d->pending_message = NULL;
	d->character->delay_who = NULL;
	d->character->delay_who2 = NULL;
	d->character->delay_info1 = 0;

	mem_free (date);
}

void do_hedit (CHAR_DATA *ch, char *argument, int cmd)
{
	char		topic [MAX_STRING_LENGTH] = { '\0' };
	char		subject [MAX_STRING_LENGTH] = { '\0' };
	char		level [MAX_STRING_LENGTH] = { '\0' };
	int		lvl = 0;

	arg_splitter(3, argument, topic, subject, level);
	

	if ( IS_NPC(ch) ) {
		send_to_char ("This is a PC-only command.\n", ch);
		return;
	}

	if ( !*topic || !*subject || !*level ) {
		send_to_char ("You must specify the topic, subject, and required level for the help entry.\n", ch);
		return;
	}

	if ( !isdigit (*level) ) {
		send_to_char ("You must specify a number for the entry's required access level.\n", ch);
		return;
	}

	lvl = strtol(level, NULL, 10);

	ch->delay_who = str_dup(topic);
	ch->delay_who2 = str_dup(subject);
	ch->delay_info1 = lvl;

	send_to_char ("Enter the text of this database entry:\n", ch);

	make_quiet (ch);

	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);

	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_help;
}

void do_vboards (CHAR_DATA *ch, char *argument, int cmd)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	char		buf [MAX_STRING_LENGTH];
	int		colnum = 1;

	*b_buf = '\0';

	mysql_safe_query ("SELECT board_name FROM virtual_boards GROUP BY board_name ORDER BY board_name ASC");
	result = mysql_store_result (database);

	while ( (row = mysql_fetch_row(result)) ) {
		if ( colnum == 1 )
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%-29s", row[0]);
		else if ( colnum == 2 )
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%-29s", row[0]);
		else if ( colnum == 3 ) {
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%s\n", row[0]);
			colnum = 1;
			continue;
		}
		colnum++;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "#6                      %s Virtual Boards#0\n", MUD_NAME);
	send_to_char (buf, ch);

	if ( !*b_buf )
		send_to_char ("None.\n", ch);
	else page_string (ch->desc, b_buf);

	if ( colnum != 1 )
		send_to_char ("\n", ch);
	
}
		
void log_missing_helpfile (CHAR_DATA *ch, char *string)
{
	char		msg [MAX_STRING_LENGTH] = { '\0' };
	char		subj [MAX_STRING_LENGTH] = { '\0' };
	MYSQL_RES	*result = NULL;

	mysql_safe_query ("SELECT * FROM unneeded_helpfiles WHERE name = '%s' AND (datestamp > UNIX_TIMESTAMP()-60*60*24*30*6)", string);
	result = mysql_store_result(database);

	if ( result && mysql_num_rows(result) >= 1 ) {
		mysql_free_result (result);
		return;
	}

	mysql_safe_query ("DELETE FROM unneeded_helpfiles WHERE name = '%s'", string);

	snprintf (subj, MAX_STRING_LENGTH, "#1Not Found:#0 %s", string);
	snprintf (msg, MAX_STRING_LENGTH, "%s: help %s\n", ch->tname, string);

	add_message ("Helpfiles", 2, ch->tname, 0, subj, "", msg, 0);

        mysql_safe_query ("INSERT INTO unneeded_helpfiles VALUES ('%s', %d)", string, (int) time(0));
}

void do_help (CHAR_DATA *ch, char *argument, int cmd)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		arg1 [MAX_STRING_LENGTH] = { '\0' };
	char		arg2 [MAX_STRING_LENGTH] = { '\0' };
	char		example [MAX_STRING_LENGTH] = { '\0' };
	char		original [MAX_STRING_LENGTH] = { '\0' };
	bool		header_needed = FALSE, category_list = FALSE, soundex = FALSE;
	int		total = 0, j = 1;

	result = NULL;

	*b_buf = '\0';
	

	if ( strstr (argument, ":") ) {
		send_to_char ("Please see #6HELP HELP#0 for proper syntax and usage.\n", ch);
		return;
	}

	snprintf (original, MAX_STRING_LENGTH, "%s", argument);

	argument = one_argument (argument, arg1);

	if ( *argument )
		argument = one_argument (argument, arg2);

	if ( !*arg1 ) {
		if ( IS_MORTAL (ch) || IS_NPC(ch) )
			mysql_safe_query ("SELECT COUNT(*) FROM helpfiles WHERE required_level = 0");
		else
			mysql_safe_query ("SELECT COUNT(*) FROM helpfiles WHERE required_level <= %d", ch->pc->level);
		result = mysql_store_result (database);
		row = mysql_fetch_row (result);
		total = strtol(row[0], NULL, 10);
		mysql_free_result (result);
		result = NULL;

		if ( IS_MORTAL (ch) || IS_NPC (ch) )
			mysql_safe_query ("SELECT category FROM helpfiles WHERE required_level = 0 GROUP BY category ASC");
		else
			mysql_safe_query ("SELECT category FROM helpfiles WHERE required_level <= %d GROUP BY category ASC", ch->pc->level);
		result = mysql_store_result (database);
		*arg2 = '\0';
		while ( (row = mysql_fetch_row (result)) ) {
			snprintf (arg1, MAX_STRING_LENGTH, "#6%-18.18s#0 ", row[0]);
			if ( !(j % 4) )
				strcat (arg1, "\n");
			j++;
			snprintf (arg2 + strlen(arg2), MAX_STRING_LENGTH, "%s", arg1);
		}
		
		if ( (j % 4) == 1 )
			arg2 [strlen(arg2)-1] = '\0';
		mysql_free_result (result);
		result = NULL;

		if ( IS_MORTAL (ch) || IS_NPC(ch) )
			mysql_safe_query ("SELECT * FROM helpfiles WHERE required_level = 0 ORDER BY RAND() DESC LIMIT 1");
		else
			mysql_safe_query ("SELECT * FROM helpfiles WHERE required_level <= %d ORDER BY RAND() DESC LIMIT 1", ch->pc->level);
		result = mysql_store_result (database);

		if ( result ) {
			row = mysql_fetch_row(result);
			snprintf (buf, MAX_STRING_LENGTH,  "Helpfile of the Moment: #6%s#0", row[0]);
			mysql_free_result (result);
			result = NULL;
		}

		snprintf (b_buf, MAX_STRING_LENGTH, "\n                      #6%s Help Database#0\n\n"
				"There are currently #6%d#0 helpfiles accessible to you in our database.\n\n"
				"Helpfile Categories:\n\n%s%s\n%s\n\n"
				"To see a full list of helpfiles within these topics, use \'#6help on <topic name>#0\'.\n\n"
				"For a detailed description of HELP searching syntax, please see \'#6help help#0\'.\n\n"
				"Our helpfiles on the Web: #6http://www.yourmud.org/index.php?display=help#0.\n", MUD_NAME, total, *arg2 ? arg2 : "#6None#0", arg2[strlen(arg2)] == '\n' ? "" : "\n", buf);
		page_string (ch->desc, b_buf);
		return;
	}

	if ( !str_cmp (arg1, "named") && *arg2 )
		mysql_safe_query ("SELECT * FROM helpfiles WHERE name LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC", arg2, ch->pc ? ch->pc->level : 0);
	else if ( !str_cmp (arg1, "containing") && *arg2 )
		mysql_safe_query ("SELECT * FROM helpfiles WHERE (entry LIKE '%%%s%%' OR related_entries LIKE '%%%s%%') AND required_level <= %d ORDER BY category,name ASC", arg2, arg2, ch->pc ? ch->pc->level : 0);
	else if ( !str_cmp (arg1, "on") && *arg2 ) {
		category_list = TRUE;
		mysql_safe_query ("SELECT * FROM helpfiles WHERE category LIKE '%s%%' AND required_level <= %d ORDER BY name ASC", arg2, ch->pc ? ch->pc->level : 0);
	}
	else if ( *arg1 && !*arg2 )
		mysql_safe_query ("SELECT * FROM helpfiles WHERE name LIKE '%s%%' AND required_level <= %d ORDER BY category,name ASC", arg1, ch->pc ? ch->pc->level : 0);
	else if ( *arg1 && *arg2 )
		mysql_safe_query ("SELECT * FROM helpfiles WHERE name = '%s' AND (category = '%s' AND required_level <= %d) ORDER BY category,name ASC", arg2, arg1, ch->pc ? ch->pc->level : 0);

	result = mysql_store_result (database);

	if ( !result ) {
		send_to_char ("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n", ch);
		log_missing_helpfile (ch, original);
		return;
	}

	if ( mysql_num_rows(result) == 0 ) {
		result = NULL;
		mysql_safe_query ("SELECT * FROM helpfiles WHERE category LIKE '%s%%' AND required_level <= %d ORDER BY name ASC", arg1, ch->pc ? ch->pc->level : 0);
		result = mysql_store_result (database);
		if ( !result || mysql_num_rows(result) == 0 ) {
			mysql_safe_query ("SELECT * FROM helpfiles WHERE SOUNDEX(category) LIKE SOUNDEX('%s') AND required_level <= %d ORDER BY category,name ASC", arg1, ch->pc ? ch->pc->level : 0);
			result = mysql_store_result (database);
			if ( !result || mysql_num_rows(result) == 0 ) {
				mysql_safe_query ("SELECT * FROM helpfiles WHERE SOUNDEX(name) LIKE SOUNDEX('%s') AND required_level <= %d ORDER BY category,name ASC", arg1, ch->pc ? ch->pc->level : 0);
				result = mysql_store_result (database);
				if ( !result || mysql_num_rows(result) == 0 ) {
					mysql_safe_query ("SELECT * FROM helpfiles WHERE related_entries LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC", arg1, ch->pc ? ch->pc->level : 0);
					result = mysql_store_result (database);
					if ( !result || mysql_num_rows(result) == 0 ) {
						mysql_safe_query ("SELECT * FROM helpfiles WHERE entry LIKE '%%%s%%' AND required_level <= %d ORDER BY category,name ASC", arg1, ch->pc ? ch->pc->level : 0);
						result = mysql_store_result (database);
						if ( result || mysql_num_rows(result) > 0 )
							category_list = TRUE;
					}
				}
				else soundex = TRUE;
			}
			else category_list = TRUE;
		}
		else category_list = TRUE;
	}
	if ( mysql_num_rows(result) == 1 ) {	
		row = mysql_fetch_row (result);
		if ( !row || GET_TRUST (ch) < strtol(row[4], NULL, 10)) {
			if ( !mysql_num_rows(result) ) {
				send_to_char ("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n", ch);
				mysql_free_result (result);
				log_missing_helpfile (ch, original);
				return;
			}
		}
		if ( mysql_num_rows(result) == 1 ) {
			snprintf (b_buf, MAX_STRING_LENGTH, "\n#6%s: %s#0\n", row[1], row[0]);
			if ( *row[2] != '\n' )
				strcat (b_buf, "\n");
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%s", row[2]);
			if ( *row[3] && str_cmp (row[3], "(null)") )
				snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "#6See Also:#0 %s\n\n", row[3]);
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "#6Last Updated:#0 %s, by %s\n", row[5], CAP(row[6]));
		}
	}

	if ( mysql_num_rows(result) > 1 ) {
		header_needed = TRUE;
		while ( (row = mysql_fetch_row (result)) ) {
			if ( GET_TRUST (ch) < strtol(row[4], NULL, 10))
				continue;
			if ( header_needed ) {
				if ( !soundex )
					snprintf (b_buf, MAX_STRING_LENGTH, "\nYour query matched the following helpfiles:\n\n   ");
				else
					snprintf (b_buf, MAX_STRING_LENGTH, "\nThe following entries most closely matched your spelling:\n\n   ");
				header_needed = FALSE;
			}
			if ( !category_list )
				snprintf (arg2, MAX_STRING_LENGTH, "%13s: %s", row[1], row[0]);
			else snprintf (arg2, MAX_STRING_LENGTH, "%s", row[0]);
			if ( !category_list ) {
				if ( j == 1 ) {
					snprintf (arg1, MAX_STRING_LENGTH, "%3d. #6%s#0", j, arg2);
					snprintf (example, MAX_STRING_LENGTH, "%s", row[0]);
					if ( !strchr (example, ' ') )
						snprintf (example, MAX_STRING_LENGTH, "\nTo pull up the first entry listed, type \'#6help %s %s#0\'.\n", LOW(row[1]), LOW(row[0]));
					else
						snprintf (example, MAX_STRING_LENGTH, "\nTo pull up the first entry listed, type \'#6help %s \"%s\"#0\'.\n", LOW(row[1]), row[0]);
				}
				else snprintf (arg1, MAX_STRING_LENGTH, "\n   %3d. #6%s#0", j, arg2);
			}
			else snprintf (arg1, MAX_STRING_LENGTH, "#6%-22.22s#0   ", arg2);
			if ( !(j % 3) && category_list )
				strcat (arg1, "\n   ");
			j++;
			snprintf (b_buf + strlen (b_buf), MAX_STRING_LENGTH, "%s", arg1);
		}
		if ( (((j-1)%3) && !header_needed) || !category_list )
			strcat (b_buf, "\n");
		if ( *example )
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%s", example);
	}

	mysql_free_result (result);
	result = NULL;	

	if ( *b_buf )
		page_string (ch->desc, b_buf);
	else {
		send_to_char ("No helpfile matching that query was found in the database.\n\n#6Please see HELP HELP to ensure you are using the proper query syntax.#0\n", ch);
		log_missing_helpfile (ch, original);
		return;
	}
}

void do_users (CHAR_DATA *ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA			*d = NULL;
	CHAR_DATA			*tch = NULL;
	char				colon_char = '\0';
	char				buf [MAX_STRING_LENGTH] = { '\0' };
	char				output [MAX_STRING_LENGTH] = { '\0' };
	char				line [MAX_STRING_LENGTH] = { '\0' };
	char				buf2 [MAX_STRING_LENGTH] = { '\0' };
	char				buf3 [MAX_STRING_LENGTH] = { '\0' };
	char				buf4 [MAX_STRING_LENGTH] = { '\0' };

	strcpy (buf, "\n#6Users Currently Connected:\n"
	             "#6--------------------------\n\n#0");

	if ( maintenance_lock )
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2We are currently closed for maintenance.\n\n#0");

	if ( pending_reboot )
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2There is currently a reboot pending.\n\n#0");
	
	send_to_char (buf, ch);

	for ( d = descriptor_list; d; d = d->next ) {

		colon_char = ':';

		*buf2 = '\0';
		*buf3 = '\0';
		*buf4 = '\0';

		if ( d->character && !d->character->desc )
			continue;

		if ( d->character && d->original )
			tch = d->original;
		else
			tch = d->character;

		if ( tch && tch->pc->level )
			colon_char = '*';

		if ( tch && d->account ) {
			snprintf (line, MAX_STRING_LENGTH, "%s  %c ", tch->pc->account, colon_char);
			snprintf (line + strlen(line), MAX_STRING_LENGTH, " #2[%s]#0 ", tch->tname);
			snprintf (line + strlen(line), MAX_STRING_LENGTH, "#3[%s]#0 ", d->host);
			snprintf (line + strlen(line), MAX_STRING_LENGTH, "#5[%d]#0", tch->in_room);
			if ( tch->pc->create_state == STATE_DIED )
				snprintf (line + strlen(line), MAX_STRING_LENGTH, " #1(Dead)#0");
			if ( d->idle )
				snprintf (line + strlen(line), MAX_STRING_LENGTH, " #4(Idle)#0");
			if ( IS_SET (tch->act, PLR_QUIET) )
				snprintf (line + strlen(line), MAX_STRING_LENGTH, " #5(Edit)#0");
			if ( IS_SET (tch->plr_flags, NEW_PLAYER_TAG) )
				snprintf (line + strlen(line), MAX_STRING_LENGTH, " #2(New)#0");
			snprintf (line + strlen(line), MAX_STRING_LENGTH, "\n");
		}

		else if ( !tch && d->account ) {
			snprintf (line, MAX_STRING_LENGTH, "%s -> #2[%s]#0 #3[%s]#0\n", d->account->name, state_name[d->connected], d->host);
		}

		else if ( !tch && !d->account ) {
			snprintf (line, MAX_STRING_LENGTH, "Logging In -> #3[%s]#0\n", d->host);
		}

		if ( strlen(output) + strlen(line) > MAX_STRING_LENGTH )
			break;

		snprintf (output + strlen(output), MAX_STRING_LENGTH,  "%s", line);
	}

	page_string (ch->desc, output);
}



void do_equipment (CHAR_DATA *ch, char *argument, int cmd)
{
	int		location = 0;
	int		found = FALSE;
	OBJ_DATA	*eq = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	*buf = '\0';
	
	if ( ch->right_hand ) {
		if ( ch->right_hand->location == WEAR_PRIM || ch->right_hand->location == WEAR_SEC )
			snprintf (buf, MAX_STRING_LENGTH,   "<wielded in right hand>  ");

		else if ( ch->right_hand->location == WEAR_BOTH )
			snprintf (buf, MAX_STRING_LENGTH,   "<wielded in both hands>  ");

		else if ( ch->right_hand->location == WEAR_SHIELD )
			snprintf (buf, MAX_STRING_LENGTH,   "<gripped in right hand>  ");

		else 
			snprintf (buf, MAX_STRING_LENGTH,      "<carried in right hand>  ");

		send_to_char (buf, ch);
		show_obj_to_char (ch->right_hand, ch, 1);
	}

	if ( ch->left_hand ) {
		if ( ch->left_hand->location == WEAR_PRIM || ch->left_hand->location == WEAR_SEC )
			snprintf (buf, MAX_STRING_LENGTH,   "<wielded in left hand>   ");
		
		else if ( ch->left_hand->location == WEAR_BOTH )
			snprintf (buf, MAX_STRING_LENGTH,   "<wielded in both hands>  ");

		else if ( ch->left_hand->location == WEAR_SHIELD )
			snprintf (buf, MAX_STRING_LENGTH,   "<gripped in left hand>   ");

		else 
			snprintf (buf, MAX_STRING_LENGTH,      "<carried in left hand>   ");

		send_to_char (buf, ch);
		show_obj_to_char (ch->left_hand, ch, 1);
	}

	if ( ch->left_hand || ch->right_hand )
		send_to_char ("\n", ch);

        for ( location = 0; location < MAX_WEAR; location++ ) {

                if ( !(eq = get_equip (ch, location)) )
                        continue;

		if ( eq == ch->right_hand || eq == ch->left_hand )
			continue;

                send_to_char (where [location], ch);

                if ( location == WEAR_BLINDFOLD || IS_OBJ_VIS (ch, eq) )
                        show_obj_to_char (eq, ch, 1);
                else
                        send_to_char ("#2something#0\n", ch);

                found = TRUE;
        }

	if ( !found )
		send_to_char ("You are naked.\n", ch);
	
	return;
}

void do_news(CHAR_DATA *ch, char *argument, int cmd) {

	char * temp_arg = NULL;
	
	temp_arg = get_text_buffer (ch, text_list, "news");
	page_string (ch->desc, temp_arg);
	return;
}

void do_wizlist (CHAR_DATA *ch, char *argument, int cmd){

	char * temp_arg = NULL;
	
	temp_arg = get_text_buffer (ch, text_list, "wizlist");
    page_string (ch->desc, temp_arg);
    return;
}

int show_where_char (CHAR_DATA *ch, int indent)
{
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	if ( !ch || !ch->room )
		return 0;

	memset (buf, ' ', indent * 3);
	buf [indent * 3] = '\0';

	snprintf (ADDBUF, MAX_STRING_LENGTH, "%s",
			 IS_NPC (ch) ? ch->short_descr : ch->tname);

	if ( IS_NPC (ch) )
		snprintf (ADDBUF, MAX_STRING_LENGTH, " (%d)", ch->mob->virtual);

	if ( ch->in_room == -1 )
		strcat (buf, " NOWHERE\n");
	else
		snprintf (ADDBUF, MAX_STRING_LENGTH, " in room %d\n", ch->in_room);

	strcat (b_buf, buf);

	if ( strlen (b_buf) > B_BUF_SIZE - 500 )
		return 0;

	return 1;
}

int show_where_obj (OBJ_DATA *obj, int indent)
{
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	if ( !obj )
		return 0;

	memset (buf, ' ', indent * 3);
	buf [indent * 3] = '\0';

	snprintf (ADDBUF, MAX_STRING_LENGTH, "%s (%d)",
				  obj->short_description,
				  obj->virtual);

	if ( obj->in_room != -1 )
		snprintf (ADDBUF, MAX_STRING_LENGTH, " in room %d\n", obj->in_room);
	else if ( obj->in_obj )
		snprintf (ADDBUF, MAX_STRING_LENGTH, " in obj:\n");
	else if ( obj->carried_by && obj->location == -1 && obj->carried_by->room != NULL )
		snprintf (ADDBUF, MAX_STRING_LENGTH, " carried by:\n");
	else if ( obj->equiped_by && obj->equiped_by->room != NULL )
		snprintf (ADDBUF, MAX_STRING_LENGTH, " equipped by:\n");
	else
		snprintf (ADDBUF, MAX_STRING_LENGTH, " ; not sure\n");

	strcat (b_buf, buf);

	if ( strlen (b_buf) > B_BUF_SIZE - 500 )
		return 0;

	if ( obj->in_obj )
		show_where_obj (obj->in_obj, indent + 1);
	else if ( obj->equiped_by )
		show_where_char (obj->equiped_by, indent + 1);
	else if ( obj->carried_by )
		show_where_char (obj->carried_by, indent + 1);

	return 1;
}

void do_find (CHAR_DATA *ch, char *argument, int cmd)
{
	int			nots = 0;
	int			musts = 0;
	int			zone = -1;
	int			virtual = -1;
	int			type = -1;
	int			i;
	OBJ_DATA	*obj = NULL;
	char		*not_list [50] = {NULL};
	char		*must_list [50] = {NULL};
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	
	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {
		send_to_char ("find +keyword     'keyword' must exist on obj\n", ch);
		send_to_char ("     -keyword     'keyword' must not exist on obj.\n", ch);
		send_to_char ("     zone         Only consider objects in this zone.\n", ch);
		send_to_char ("     vnum         Just find the specific object vnum.\n", ch);
		send_to_char ("\nExamples:\n\n", ch);
		send_to_char ("   > find +bag +leather 10\n", ch);
		send_to_char ("   > find 10001\n", ch);
		send_to_char ("   > find +sword -rusty 12\n", ch);
		return;
	}

	while ( *buf ) {

		if ( *buf == '+' )
			must_list [musts++] = str_dup (buf + 1);

		else if ( *buf == '-' )
			not_list [nots++] = str_dup (buf + 1);

		else if ( isdigit (*buf) ) {
			if ( strtol(buf, NULL, 10) < 100 )
				zone = strtol(buf, NULL, 10);
			else
				virtual = strtol(buf, NULL, 10);
		}

		else if ( (type = index_lookup (item_types, buf)) == -1 ) {
			send_to_char ("Unknown keyword to where obj.\n", ch);
			return;
		}
		if ( (musts < 50) && (nots < 50)){
			argument = one_argument (argument, buf);
		}
		else{
			send_to_char ("Too many keywords to where obj.\n", ch);
			return;
		}
	}

	*b_buf = '\0';

	for ( obj = object_list; obj; obj = obj->next ) {

		if ( obj->deleted )
			continue;

/** find the first obj that must be listed **/
		for ( i = 0; i < musts; i++ )
			if ( !name_is (must_list [i], obj->name) )
				break;

		if ( i != musts )	/* Got out of loop w/o all musts being there */
			continue;

/** find the first obj that is NOT to be lsited **/
		for ( i = 0; i < nots; i++ )
			if ( name_is (must_list [i], obj->name) )
				break;

		if ( i != nots )	/* Got out of loop w/o all nots not being thr */
			continue;

		if ( virtual != -1 && obj->virtual != virtual )
			continue;

		if ( type != -1 && obj->obj_flags.type_flag != type )
			continue;

				/* Zone is a little tricky to determine */

		if ( !show_where_obj (obj, 0) )
			break;
	}

	page_string (ch->desc, b_buf);
}

void do_locate (CHAR_DATA *ch, char *argument, int cmd)
{
	int			num_clans = 0;
	int			i = 0;
	int			musts = 0;
	int			nots = 0;
	int			act_bits = 0;
	int			position = -1;
	int			ind = 0;
	int			virtual = 0;
	int			zone = -1;
	CHAR_DATA	*mob = NULL;
	char		*arg = NULL;
	char		clan_names [10][80] = { { '\0' }, { '\0' } };
	char		not_list [50][80] = { { '\0' }, { '\0' } };
	char		must_list [50][80] = { { '\0' }, { '\0' } };
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		acts [MAX_STRING_LENGTH] = { '\0' };
	
	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {
		send_to_char ("locate [+/-mob name]\n"
					  "       [zone]\n"
					  "       [virtual]\n"
					  "       act \"<action-bits>\"\n"
					  "       [clan name]\n"
					  "       clan <clan name>\n"
					  "       [position]\n", ch);
		return;
	}

	while ( *buf ) {
		
		if ( *buf == '+' )
			strcpy (must_list [musts++], buf + 1);

		else if ( *buf == '-' )
			strcpy (not_list [nots++], buf + 1);

		else if ( isdigit (*buf) ) {
			ind = strtol(buf, NULL, 10);
			if ( ind > 100 )
				virtual = ind;
			else
				zone = ind;
		}

		else if ( !str_cmp (buf, "act") ) {

			argument = one_argument (argument, acts);

			arg = one_argument (acts, buf);

			while ( *buf ) {

				if ( (ind = index_lookup (action_bits, buf)) == 0 ) {
					send_to_char ("No such action-bit:  ", ch);
					send_to_char (buf, ch);
					send_to_char ("\n", ch);
					return;
				}

				SET_BIT (act_bits, 1 << ind);

				arg = one_argument (arg, buf);
			}
		}

		else if ( !str_cmp (buf, "clan") ) {
		
			argument = one_argument (argument, buf);

			if ( num_clans >= 9 ) {
				send_to_char ("Hey, 10 clans is enough!\n", ch);
				return;
			}

			strcpy (clan_names [num_clans++], buf);
		}

		else if ( isalpha (*buf) ) {

			if ( num_clans >= 9 ) {
				send_to_char ("Hey, 10 clans is enough!\n", ch);
				return;
			}

			strcpy (clan_names [num_clans++], buf);
		}

		else {
			send_to_char ("Unknown keyword:  ", ch);
			send_to_char (buf, ch);
			send_to_char ("\n", ch);
			return;
		}
		
		argument = one_argument (argument, buf);
	}

	*b_buf = '\0';

	for ( mob = character_list; mob; mob = mob->next ) {

		if ( mob->deleted )
			continue;

				/* Check act bits against mob's act bits.  The act bits
                   specified on the command line must all be in mob->act */

		if ( zone != -1 && mob->room->zone != zone )
			continue;

		if ( virtual &&
			 (!mob->mob || mob->mob->virtual != virtual) )
			continue;

		if ( position != -1 && GET_POS (mob) != position )
			continue;

		if ( act_bits &&
			 (act_bits & mob->act) != act_bits )
			continue;

/** find first clan name that matches **/
		for ( i = 0; i < num_clans; i++ )
			if ( is_clan_member (mob, clan_names [i]) )
				break;

		if ( num_clans && i >= num_clans )	/* Couldn't find a clan member */
			continue;

		for ( i = 0; i < musts; i++ )
			if ( !name_is (must_list [i], mob->name) )
				break;

		if ( i != musts )	/* Got out of loop w/o all musts being there */
			continue;

		for ( i = 0; i < nots; i++ )
			if ( name_is (not_list [i], mob->name) )
				break;

		if ( i != nots )	/* Got out of loop w/o all nots not being thr */
			continue;

		show_where_char (mob, 0);
	}

	page_string (ch->desc, b_buf);
}

void do_where(CHAR_DATA *ch, char *argument, int cmd)
{
	char 		name[MAX_INPUT_LENGTH] = { '\0' };
	char		buf[MAX_STRING_LENGTH] = { '\0' };
	char		buf2[MAX_STRING_LENGTH] = { '\0' };
	register 	CHAR_DATA 	*i = NULL;
	register 	OBJ_DATA *k = NULL;
	struct descriptor_data *d = NULL;
	char		*temp_arg1 = NULL;
	char		*temp_arg2 = NULL;

	(void)one_argument(argument, name);

	if (!*name) {
		if ( IS_MORTAL (ch) )
		{
			send_to_char("What are you looking for?\n", ch);
			return;
		}
		else
		{		
			for (d = descriptor_list; d; d = d->next) {
				if (d->character && (d->connected == CON_PLYNG) && (d->character->in_room != NOWHERE)) {
					if (d->original){   /* If switched */
						temp_arg1 = fname(d->character->tname);
						snprintf (buf, MAX_STRING_LENGTH,  "%-20s - %s [%d] In body of %s\n", 			  d->original->tname,
						  vtor (d->character->in_room)->name,
						  vtor (d->character->in_room)->virtual,
						  temp_arg1);
					}
					else{
						snprintf (buf, MAX_STRING_LENGTH,  "%-20s - %s [%d]\n",
						  d->character->tname,
						  vtor (d->character->in_room)->name,
						  vtor (d->character->in_room)->virtual);
					}
					
					send_to_char(buf, ch);
				}
			}
			return;
		}
	}

	*buf = '\0';

	for (i = character_list; i; i = i->next) {

		if ( i->deleted )
			continue;

		if (name_is(name, i->name) && CAN_SEE(ch, i) ) {
			if ((i->in_room != NOWHERE) && ((GET_TRUST(ch)>3) ||
			    (vtor (i->in_room)->zone == vtor (ch->in_room)->zone))) {

				if (IS_NPC(i)){
					temp_arg1 = char_short(i);
					snprintf (buf, MAX_STRING_LENGTH,  "%-25.25s- %s ", temp_arg1, vtor (i->in_room)->name);
				}
				else{
					snprintf (buf, MAX_STRING_LENGTH,  "%-25.25s- %s ", i->tname, vtor (i->in_room)->name);
				}
				
				if ( !IS_MORTAL (ch) )
					snprintf (buf2, MAX_STRING_LENGTH, "[%d]\n", vtor (i->in_room)->virtual);
				else
					strcpy(buf2, "\n");

				strcat(buf, buf2);
				send_to_char(buf, ch);

				if ( IS_MORTAL (ch) )
					break;
			}
		}
	}

	if (GET_TRUST(ch) > 3) {
		for (k = object_list; k; k = k->next) {

			if ( k->deleted )
				continue;

			if (name_is(name, k->name) && CAN_SEE_OBJ(ch, k)) {
				if(k->carried_by && !k->carried_by->deleted && k->carried_by->room) {
					temp_arg1 = obj_short_desc (k);
					temp_arg2 = char_short(k->carried_by);
					snprintf (buf, MAX_STRING_LENGTH, "%-20.20s- Carried by %s in room[%d].", temp_arg1, temp_arg2, vtor (k->carried_by->in_room)->virtual);
					strcat(buf,"\n");
					send_to_char(buf,ch);
				}
				else if(k->equiped_by && !k->equiped_by->deleted && k->equiped_by->room ) {
					temp_arg1 = obj_short_desc (k);
					temp_arg2 = char_short(k->equiped_by);
					snprintf (buf, MAX_STRING_LENGTH, "%-20.20s- Equipped by %s in room[%.5d].", temp_arg1, temp_arg2, vtor (k->equiped_by->in_room)->virtual);
					strcat(buf,"\n");
					send_to_char(buf,ch);
				}
				else if(k->in_obj && !k->in_obj->deleted) {
					temp_arg1 = obj_short_desc (k);
					temp_arg2 = obj_short_desc (k->in_obj);
					snprintf (buf, MAX_STRING_LENGTH, "%-20.20s- Inside %s, vnum[%.5d].", temp_arg1, temp_arg2, k->in_obj->virtual);
					strcat(buf,"\n");
					send_to_char(buf,ch);
				}
				else if(k->in_room != NOWHERE) {
					temp_arg1 = obj_short_desc (k);
					snprintf (buf, MAX_STRING_LENGTH,  "%-20.20s- %s [%d]",
						temp_arg1,
						vtor (k->in_room)->name,
						vtor (k->in_room)->virtual);
					strcat(buf,"\n");
					send_to_char(buf, ch);
				}
			}
		}
	}

	if (!*buf)
		send_to_char("Couldn't find any such thing.\n", ch);
}

void do_who (CHAR_DATA *ch, char *argument, int cmd)
{
	int			mortals = 0;
	int			immortals = 0;
	int			guests = 0;
	char		tmp [MAX_STRING_LENGTH] = { '\0' };
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	DESCRIPTOR_DATA		*d = NULL;

	*s_buf='\0';

	if ( IS_MORTAL(ch) )
		strcpy (tmp, "\n#2Available Staff:#0\n\n");
	else
		strcpy (tmp, "\n\n#2Available Staff:#0\n\n");


	for ( d = descriptor_list; d; d = d->next ) {

		if ( !d->character )
			continue;

		if ( d->connected )
			continue;

		if ( IS_MORTAL (d->character) ) {
			if ( d->character->pc &&
				 !d->character->pc->level &&
				 d->character->pc->create_state == 2 && !IS_SET (d->character->flags, FLAG_GUEST) )
				mortals++;
			else if ( IS_SET (d->character->flags, FLAG_GUEST) )
				guests++;
		}

		else if ( d->original && !IS_SET (d->original->flags, FLAG_AVAILABLE) )
			continue;

		else if ( IS_SET (d->character->flags, FLAG_AVAILABLE) ) {
			snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "    %s", 
					d->original ? GET_NAME (d->original) :
								  GET_NAME (d->character));
			strcat (tmp, "\n");
			immortals = 1;
		}
	}

	*buf = '\0';

	if ( !mortals )
		snprintf (s_buf, MAX_STRING_LENGTH,  "\nThere currently aren't any players in YourWorld.\n");
	else if ( mortals == 1 )
		snprintf (s_buf, MAX_STRING_LENGTH,  "\nThere is currently #21#0 player in YourWorld.\n");
	else
		snprintf (s_buf, MAX_STRING_LENGTH,  "\nThere are currently #2%d#0 players in YourWorld.\n", mortals);	

	if ( guests )
		snprintf (s_buf + strlen(s_buf), MAX_STRING_LENGTH, "There %s currently #2%d#0 guest%s visiting our OOC lounge.\n", guests == 1 ? "is" : "are", guests, guests == 1 ? "" : "s");

	snprintf (s_buf + strlen(s_buf), MAX_STRING_LENGTH, "\nOur record is #2%d#0 players, last seen on #2%s#0.", count_max_online, max_online_date);

	if ( IS_MORTAL(ch) )
		strcat (s_buf, "\n");

	strcat (s_buf, tmp);

	if ( !immortals )
		snprintf (s_buf + strlen (s_buf), MAX_STRING_LENGTH, "    None.\n");

	send_to_char (s_buf, ch);
}

void do_scan (CHAR_DATA *ch, char *argument, int cmd)
{
	int					dir = 0;
	char				buf [MAX_STRING_LENGTH] = { '\0' };
	ROOM_DIRECTION_DATA	*exit_room = NULL;
	ROOM_DATA			*next_room = NULL;

	argument = one_argument (argument, buf);

	if ( !real_skill (ch, SKILL_SCAN) || ch->skills [SKILL_SCAN] <= 0 ) {
		send_to_char ("You cannot focus well enough.\n", ch);
		return;
	}

	
	
	if ( IS_SET (ch->room->room_flags, STIFLING_FOG) ) {
		send_to_char ("The thick fog in the area prevents any such attempt.\n", ch);
		return;
	}

	if ( !*buf ) {			/* Quick one-room deep scan */

		for ( dir = 0; dir < 6; dir++ ) {
			if ( (exit_room = PASSAGE (ch, dir)) &&
				 vtor (exit_room->to_room) &&
				 !(IS_SET (exit_room->exit_info, PASSAGE_ISDOOR) &&
				   IS_SET (exit_room->exit_info, PASSAGE_CLOSED)) ) {
				break;
			}
		}

		if ( dir == 6 ) {
			send_to_char ("There is nowhere to scan.\n", ch);
			return;
		}

		if ( OUTSIDE (ch) ) {
			act ("$n scans the horizon in all directions.",
					TRUE, ch, 0, 0, TO_ROOM);
			act ("You focus on the horizon . . .", FALSE, ch, 0, 0, TO_CHAR);
		} else {
			act ("$n peers into the distance in all directions.",
					TRUE, ch, 0, 0, TO_ROOM);
			act ("You peer into the distance . . .", FALSE, ch, 0, 0, TO_CHAR);
		}

		ch->delay_type = DEL_QUICK_SCAN;
		ch->delay      = 2;
		ch->delay_info1= 0;
		ch->delay_info2= 0;			/* Indicates something was seen */
		
		return;
	}
		
	switch ( *buf ) {
		case 'n': dir = 0; break;
		case 'e': dir = 1; break;
		case 's': dir = 2; break;
		case 'w': dir = 3; break;
		case 'u': dir = 4; break;
		case 'd': dir = 5; break;
		default:
			send_to_char ("Which direction would you like to scan?\n", ch);
			return;
	}

	exit_room = PASSAGE (ch, dir);

	if ( !exit_room || !(next_room = vtor (exit_room->to_room)) ) {
		send_to_char ("There's nothing to scan that way.\n", ch);
		return;
	}

	if ( OUTSIDE (ch) ) {
		if ( dir != DIR_UP && dir != DIR_DOWN ) {
			snprintf (buf, MAX_STRING_LENGTH,  "$n scans the %s horizon.", relative_dirs[dir]);
			act (buf, TRUE, ch, 0, 0, TO_ROOM);
			snprintf (buf, MAX_STRING_LENGTH,  "You focus on the %s horizon . . .", relative_dirs[dir]);
			act (buf, FALSE, ch, 0, 0, TO_CHAR);
		}
		else {
                	act ("$n scans the horizon.", TRUE, ch, 0, 0, TO_ROOM);
                	act ("You focus on the horizon . . .", FALSE, ch, 0, 0, TO_CHAR);
		}
	} else {
		act ("$n peers into the distance.", TRUE, ch, 0, 0, TO_ROOM);
		act ("You peer into the distance . . .", FALSE, ch, 0, 0, TO_CHAR);
	}

    ch->delay_type = DEL_SCAN;
    ch->delay      = 2;
	ch->delay_info1= dir;
}

void delayed_quick_scan (CHAR_DATA *ch)
{
	int					dir = 0;
	int					seen = 0;
	CHAR_DATA			*tch = NULL;
	ROOM_DIRECTION_DATA	*exit_room = NULL;
	ROOM_DATA			*next_room = NULL;
	OBJ_DATA			*tobj = NULL;
	char				buf [MAX_STRING_LENGTH] = { '\0' };
	int			penalty = 0;

	dir = ch->delay_info1;

	if ( !dir )
		skill_use (ch, SKILL_SCAN, 0);

	for ( ; dir < 6; ) {
		if ( !(exit_room = PASSAGE (ch, dir)) ||
			 !(next_room = vtor (exit_room->to_room)) ||
			 (IS_SET (exit_room->exit_info, PASSAGE_ISDOOR) &&
			  IS_SET (exit_room->exit_info, PASSAGE_CLOSED)) )
			dir++;
		else
			break;
	}

	if ( dir == 6 ) {
		return;
	}

	seen = 0;

	if ( weather_info[ch->room->zone].fog == THIN_FOG )
		penalty += 5;
	if ( weather_info[ch->room->zone].fog == THICK_FOG )
		penalty += 15;
	if ( weather_info[ch->room->zone].state == STEADY_RAIN )
		penalty += 5;
	if ( weather_info[ch->room->zone].state == HEAVY_RAIN )
		penalty += 15;
	if ( weather_info[ch->room->zone].state == STEADY_SNOW )
		penalty += 20;
	if ( weather_info[ch->room->zone].state == HEAVY_SNOW )
		penalty += 25;

	for ( tch = next_room->people; tch; tch = tch->next_in_room ) {

		if ( !could_see (ch, tch) )
			continue;

		if ( IS_SET (next_room->room_flags, STIFLING_FOG) )
			continue;

		if ( ch->skills [SKILL_SCAN] - penalty < number (1, 100) && !has_been_sighted (ch, tch) )
			continue;

		if ( !seen ) {
			seen = 1;
			snprintf (buf, MAX_STRING_LENGTH,  "%s%syou see:\n", dirs [dir],
					 IS_LIGHT (next_room) ? " " : ", #4where it is dark#0, ");
			*buf = toupper (*buf);
			send_to_char (buf, ch);
		}

		show_char_to_char (tch, ch, 0);

		ch->delay_info2 = 1;		/* We said something */

		target_sighted (ch, tch);
	}

	for ( tobj = next_room->contents; tobj; tobj = tobj->next_content ) {

		if ( !could_see_obj (ch, tobj) )
			continue;

		if ( IS_SET (next_room->room_flags, STIFLING_FOG) )
			continue;

		if ( ch->skills [SKILL_SCAN] - penalty  < number (1, 100) )
			continue;

		if ( !seen ) {
			seen = 1;
			snprintf (buf, MAX_STRING_LENGTH,  "%s%syou see:\n", dirs [dir],
					 IS_LIGHT (next_room) ? " " : ", #4where it is dark#0, ");
			*buf = toupper (*buf);
			send_to_char (buf, ch);
		}

		show_obj_to_char (tobj, ch, 7);

		ch->delay_info2 = 1;
	}

	if ( !seen ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s%syou see nothing.\n", dirs [dir],
				 IS_LIGHT (next_room) ? " " : ", #4where it is dark#0, ");
		*buf = toupper (*buf);
		send_to_char (buf, ch);
	}

	dir++;

	if ( dir == 6 ) {
/*		if ( !ch->delay_info2 )
			send_to_char ("You see nothing.\n", ch);
*/
		return;
	}

    ch->delay_type  = DEL_QUICK_SCAN;
	ch->delay_info1 = dir;
    ch->delay       = 1;
}

void delayed_scan (CHAR_DATA *ch)
{
	int			dir = 0;
	int			blank = 0;
	int			seen = 0;
	int			something_said = 0;
	int			penalty = 0;
	CHAR_DATA	*tch = NULL;
	ROOM_DIRECTION_DATA	*exit_room = NULL;
	ROOM_DATA	*next_room = NULL;
	OBJ_DATA	*tobj = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		*temp_arg = NULL;
	
/*
4.  Scan into darkness.  As suggested by a player (Terisa), scanning            
    into a darkened room should at least have a chance of revealing             
    minimal information (i.e., "You think you see something.").  This           
    should only affect rooms adjacent to the room being scanned from,           
    and should, if possible, only work if the room being scanned is             
    adjacent to a lighted room.  (Vague impressions seen in the dim             
    light.)                                                                     
    Change the message given for "very far away" room to 'something'            
    instead of 'someone'.  
*/

	dir = ch->delay_info1;

	if ( !real_skill (ch, SKILL_SCAN) || ch->skills [SKILL_SCAN] <= 0 ) {
		send_to_char ("You can see nothing there.\n", ch);
		return;
	}

	exit_room = PASSAGE (ch, dir);

	if ( !exit_room || !(next_room = vtor (exit_room->to_room)) ) {
		send_to_char ("There's nothing to scan that way.\n", ch);
		return;
	}

	if ( IS_SET (exit_room->exit_info, PASSAGE_ISDOOR) &&
		 IS_SET (exit_room->exit_info, PASSAGE_CLOSED) ) {
		if ( !IS_LIGHT (ch->room) )
			send_to_char ("You see nothing.\n", ch);
		else
			send_to_char ("Your view is blocked.\n", ch);

		return;
	}

	skill_use (ch, SKILL_SCAN, 0);

	seen = 0;
	blank = 1;

        if ( weather_info[ch->room->zone].fog == THIN_FOG )
                penalty += 5;
        if ( weather_info[ch->room->zone].fog == THICK_FOG )
                penalty += 15;
        if ( weather_info[ch->room->zone].state == STEADY_RAIN )
                penalty += 5;
        if ( weather_info[ch->room->zone].state == HEAVY_RAIN )
                penalty += 15;
        if ( weather_info[ch->room->zone].state == STEADY_SNOW )
                penalty += 20;
        if ( weather_info[ch->room->zone].state == HEAVY_SNOW )
                penalty += 25;

	if ( !next_room->psave_loaded )
		load_save_room (next_room);

	for ( tch = next_room->people; tch; tch = tch->next_in_room ) {

		if ( !could_see (ch, tch) )
			continue;

		if ( IS_SET (next_room->room_flags, STIFLING_FOG) )
			continue;

		if ( ch->skills [SKILL_SCAN] - penalty < number (1, 100) && !has_been_sighted (ch, tch) )
			continue;

		if ( !seen ) {
			seen = 1;
			blank = 0;
			if ( IS_LIGHT (next_room) )
				send_to_char ("Nearby you see:\n", ch);
			else
				send_to_char ("Nearby, #4where it is dark#0, you see:\n", ch);
		}

		show_char_to_char (tch, ch, 0);

		something_said = 1;

		target_sighted (ch, tch);
	}

	for ( tobj = next_room->contents; tobj; tobj = tobj->next_content ) {

		if ( !could_see_obj (ch, tobj) )
			continue;

		if ( IS_SET (next_room->room_flags, STIFLING_FOG) )
			continue;

		if ( ch->skills [SKILL_SCAN] - penalty < number (1, 100) )
			continue;

		if ( !seen ) {
			seen = 1;
			blank = 0;
			if ( IS_LIGHT (next_room) )
				send_to_char ("Nearby you see:\n", ch);
			else
				send_to_char ("Nearby, #4where it is dark#0, you see:\n", ch);
		}

		show_obj_to_char (tobj, ch, 7);

		something_said = 1;
	}

	exit_room = next_room->dir_option [dir];

	if ( !exit_room || !(next_room = vtor (exit_room->to_room)) ||
		 (IS_SET (exit_room->exit_info, PASSAGE_ISDOOR) &&
		  IS_SET (exit_room->exit_info, PASSAGE_CLOSED)) ) {

		if ( !something_said )
			send_to_char ("You detect nothing.\n", ch);

		return;
	}

	if ( !blank ) {
		blank = 1;
		send_to_char ("\n", ch);
	}

	seen = 0;

	if ( !next_room->psave_loaded )
		load_save_room (next_room);

	penalty += 5;

	for ( tch = next_room->people; tch; tch = tch->next_in_room ) {

		if ( !could_see (ch, tch) )
			continue;

		if ( IS_SET (next_room->room_flags, STIFLING_FOG) )
			continue;

		if ( ch->skills [SKILL_SCAN] - penalty < number (1, 100) && !has_been_sighted (ch, tch) )
			continue;

		if ( !seen ) {
			seen = 1;
			blank = 0;
			if ( IS_LIGHT (next_room) )
				send_to_char ("Far away you see:\n", ch);
			else
				send_to_char ("Far away, #4where it is dark#0, you see:\n", ch);
		}
		
		temp_arg = char_short (tch);
		snprintf (buf, MAX_STRING_LENGTH,  "   #5%s#0\n", temp_arg);
		send_to_char (buf, ch);

		something_said = 1;

		target_sighted (ch, tch);
	}

	for ( tobj = next_room->contents; tobj; tobj = tobj->next_content ) {

		if ( !could_see_obj (ch, tobj) )
			continue;

		if ( IS_SET (next_room->room_flags, STIFLING_FOG) )
			continue;

		if ( tobj->obj_flags.weight < 10000 && !CAN_WEAR (tobj, ITEM_TAKE) )
			continue;

		if ( !seen ) {
			seen = 1;
			blank = 0;
			if ( IS_LIGHT (next_room) )
				send_to_char ("Far away you see:\n", ch);
			else
				send_to_char ("Far away, #4where it is dark#0, you see:\n", ch);
		}

		snprintf (buf, MAX_STRING_LENGTH,  "   you see #2%s#0.\n", tobj->short_description);
		send_to_char (buf, ch);

		something_said = 1;
	}

	exit_room = next_room->dir_option [dir];

	if ( !exit_room || !(next_room = vtor (exit_room->to_room)) ||
		 (IS_SET (exit_room->exit_info, PASSAGE_ISDOOR) &&
		  IS_SET (exit_room->exit_info, PASSAGE_CLOSED)) ) {

		if ( !something_said )
			send_to_char ("You detect nothing.\n", ch);

		return;
	}

	if ( !blank ) {
		blank = 1;
		send_to_char ("\n", ch);
	}

	penalty += 10;

	for ( tch = next_room->people; tch; tch = tch->next_in_room ) {

		if ( !could_see (ch, tch) )
			continue;

		if ( IS_SET (next_room->room_flags, STIFLING_FOG) )
			continue;

		if ( ch->skills [SKILL_SCAN] - penalty < number (1, 100) && !has_been_sighted (ch, tch) )
			continue;

		if ( !seen ) {
			seen = 1;
			blank = 0;
			if ( IS_LIGHT (next_room) )
				send_to_char ("Very far away you see:\n", ch);
			else
				send_to_char ("Very far away, #4where it is dark#0, you see:\n", ch);
		}
		
		temp_arg = char_short (tch);
		snprintf (buf, MAX_STRING_LENGTH,  "   #5%s#0\n", temp_arg);
		send_to_char (buf, ch);
		something_said = 1;

		target_sighted (ch, tch);
	}

	if ( !something_said )
		send_to_char ("You detect nothing.\n", ch);
}


void tracking_system_response (CHAR_DATA *ch, MESSAGE_DATA *message)
{
	ACCOUNT_DATA	*account = NULL;
	CHAR_DATA	*tch = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		from [MAX_STRING_LENGTH] = { '\0' };
	char		subject [255] = { '\0' };
	char		type [15] = { '\0' };

	if ( !message )
		return;

	if ( !*ch->desc->pending_message->message )
		return;

	if ( !(account = load_account (message->poster)) ) {
		if ( !(tch = load_pc (message->poster)) )
			return;
		else account = load_account (tch->pc->account);
	}

	if ( !account ) {
		if ( tch != NULL )
			unload_pc (tch);
		return;
	}

	if ( !str_cmp (ch->delay_who, "Bugs") )
		snprintf (type, MAX_STRING_LENGTH, "Bug");
	else if ( !str_cmp (ch->delay_who, "Typos") )
		snprintf (type, MAX_STRING_LENGTH, "Typo");
	else if ( !str_cmp (ch->delay_who, "Ideas") )
		snprintf (type, MAX_STRING_LENGTH, "Idea");
	else if ( !str_cmp (ch->delay_who, "Submissions") )
		snprintf (type, MAX_STRING_LENGTH, "Writing Submission");
	else if ( !str_cmp (ch->delay_who, "Petitions") )
		snprintf (type, MAX_STRING_LENGTH, "Petition");
	else snprintf (type, MAX_STRING_LENGTH, "Issue");

	snprintf (buf, MAX_STRING_LENGTH,  "%s"
                      "\n\n-- Original Report, Filed %s --\n\n"
                      "%s", ch->desc->pending_message->message, message->date, message->message);

	snprintf (from, MAX_STRING_LENGTH, "%s <%s>", ch->tname, ch->desc->account->email);

	if ( !str_cmp (type, "Writing Submission") )
		snprintf (subject, MAX_STRING_LENGTH, "Re: Your Website Submission");
	else if ( !str_cmp (type, "Petition") )
		snprintf (subject, MAX_STRING_LENGTH, "Re: Your Logged Petition");
	else snprintf (subject, MAX_STRING_LENGTH, "Re: Your %s Report", type);
	send_email (account, STAFF_EMAIL, from, subject, buf);
	unload_pc (tch);
}

void post_track_response (DESCRIPTOR_DATA *d)
{
	MESSAGE_DATA	*message = NULL;
	MYSQL_RES	*result = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	*buf = '\0';

	if ( !str_cmp (d->character->delay_who, "Typos") )
		mysql_safe_query ("UPDATE newsletter_stats SET resolved_typos=resolved_typos+1");
	else if ( !str_cmp (d->character->delay_who, "Bugs") )
		mysql_safe_query ("UPDATE newsletter_stats SET resolved_bugs=resolved_bugs+1");

	message = load_message (d->character->delay_who, 5, d->character->delay_info1);
	if ( !*d->pending_message->message ) {
		send_to_char ("No email sent.\n", d->character);
	}
	else send_to_char ("The report poster has been sent an email notification.\n", d->character);
	tracking_system_response(d->character, message);

	mysql_safe_query ("DELETE FROM virtual_boards WHERE post_number = %d AND board_name = '%s'", d->character->delay_info1, d->character->delay_who);

        if ( mysql_safe_query ("SELECT * FROM virtual_boards WHERE board_name = '%s'", d->character->delay_who) )
                return;
        result = mysql_store_result (database);
        if ( mysql_num_rows(result) == 0 ) {
                snprintf (buf, MAX_STRING_LENGTH,  "vboards/%s", d->character->delay_who);
                unlink (buf);
        }

	d->character->delay_who = NULL;
	d->character->delay_info1 = 0;

	mysql_free_result (result);
	result = NULL;

	unload_message (message);
}




void do_jerase (CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if ( IS_SET (ch->flags, FLAG_GUEST) ) {
		send_to_char ("Journals are unavailable to guests.\n", ch);
		return;
	}

	if ( !*buf ) {
		send_to_char ("Which message do you wish to erase?\n", ch);
		return;
	}

	if ( !erase_mysql_board_post (ch, ch->tname, 3, buf) ) {
		send_to_char ("I couldn't find that journal entry.\n", ch);
		return;
	}

	send_to_char ("Journal entry erased successfully.\n", ch);
}

void do_erase (CHAR_DATA *ch, char *argument, int cmd)
{
	int				msg_no = 0;
	MESSAGE_DATA	*message = NULL;
	OBJ_DATA		*obj = NULL;
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	char			buf2 [MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Which message do you wish to erase?\n", ch);
		return;
	}

	if ( IS_NPC (ch) ) {
		send_to_char ("You can't do this while switched.\n", ch);
		return;
	}

	if ( !isdigit (*buf) ) {
		if ( !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) &&
			 !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
			 !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) ) {
			if ( IS_MORTAL(ch) || !erase_mysql_board_post (ch, buf, 2, argument) ) {
				if ( IS_MORTAL (ch) || !erase_mysql_board_post (ch, buf, 1, argument) ) {	
					if ( IS_MORTAL (ch) )
						send_to_char ("You do not see that board here.\n", ch);
					else send_to_char ("Either the specified board is not here, or the specified message does not exist.\n", ch);
					return;
				} else {
					send_to_char ("The specified message has been erased.\n", ch);
					return;
				}
			} else {
				send_to_char ("The specified message has been erased.\n", ch);
				return;
			}
		}

		if ( obj->obj_flags.type_flag != ITEM_BOARD ) {
			act ("$p isn't a board.", FALSE, ch, obj, 0, TO_CHAR);
			return;
		}

		(void)one_argument (obj->name, buf2);
		argument = one_argument (argument, buf);
	}

	else {
		if ( !(obj = get_obj_in_list_vis (ch, "board", ch->room->contents)) &&
			 !(obj = get_obj_in_list_vis (ch, "board", ch->left_hand)) &&
			 !(obj = get_obj_in_list_vis (ch, "board", ch->right_hand)) ) {
			send_to_char ("You do not see that board here.\n", ch);
			return;
		}

		if ( obj->obj_flags.type_flag != ITEM_BOARD ) {
			act ("$p, the first board here, isn't a real board.",
					FALSE, ch, obj, 0, TO_CHAR);
			return;
		}

		(void)one_argument (obj->name, buf2);
	}

	if ( !isdigit (*buf) ) {
		send_to_char ("Expected a message number to erase.\n", ch);
		return;
	}

	msg_no = strtol(buf, NULL, 10);

	if ( !(message = load_message (buf2, 6, msg_no)) ) {
		send_to_char ("That message doesn't exist.\n", ch);
		return;
	}

	if ( !GET_TRUST (ch) && str_cmp (GET_NAME (ch), message->poster) ) {
		send_to_char ("You can only erase your own messages.\n", ch);
		unload_message (message);
		return;
	}

	if ( erase_mysql_board_post (ch, buf2, 0, buf) )
		send_to_char ("The specified message has been erased.\n", ch);
	else send_to_char ("There was a problem erasing that message.\n", ch);

	unload_message (message);
}

int write_virtual_board (CHAR_DATA *ch, char *name, char *argument)
{
	if ( strlen (name) > 15 )
		return 0;

	if ( !isalpha (*name) )
		return 0;

	*name = toupper (*name);

	if ( !*argument ) {
		send_to_char ("Please include a subject for your post.\n", ch);
		return 0;
	}

	while ( *argument == ' ' )
		argument++;

	ch->desc->pending_message = (MESSAGE_DATA *)alloc (sizeof (MESSAGE_DATA), 1);

		/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster  = add_hash (name);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->virtual = -2;
	ch->desc->pending_message->info	 = add_hash ("");
	ch->desc->pending_message->subject = add_hash (argument);
	ch->desc->pending_message->flags = MF_READ;

	make_quiet (ch);

	send_to_char ("Enter your note, terminate with an '@'\n\n", ch);

	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_to_mysql_virtual_board;

	return 1;
}


int write_pc_board (CHAR_DATA *ch, char *name, char *argument)
{
	int				i = 0;
	CHAR_DATA		*who = NULL;

	if ( strlen (name) > 15 )
		return 0;

	for ( i = 0; i <= strlen(name); i++ )
		if ( !isalpha (*name) )
			return 0;

	*name = toupper (*name);

	if ( !(who = load_pc (name)) )
		return 0;

	unload_pc (who);

	if ( !*argument ) {
		send_to_char ("Please include a subject for your post.\n", ch);
		return 0;
	}

	while ( *argument == ' ' )
		argument++;

	ch->desc->pending_message = (MESSAGE_DATA *)alloc (sizeof (MESSAGE_DATA), 1);

		/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster  = add_hash (name);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->virtual = -1;
	ch->desc->pending_message->info	 = add_hash ("");
	ch->desc->pending_message->subject = add_hash (argument);
	ch->desc->pending_message->flags = 0;

	make_quiet (ch);

	send_to_char ("Enter your note, terminate with an '@'\n\n", ch);

	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_to_mysql_player_board;

	return 1;
}


int write_journal (CHAR_DATA *ch, char *argument)
{

	if ( !*argument ) {
		send_to_char ("Please include a subject for your post.\n", ch);
		return 0;
	}

	while ( *argument == ' ' )
		argument++;

	ch->desc->pending_message = (MESSAGE_DATA *)alloc (sizeof (MESSAGE_DATA), 1);
		/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster  = add_hash(ch->tname);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->virtual = -1;
	ch->desc->pending_message->info	 = add_hash ("");
	ch->desc->pending_message->subject = add_hash (argument);

	make_quiet (ch);

	send_to_char ("Type your journal entry; terminate with an '@'\n\n", ch);

	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_to_mysql_journal;

	return 1;
}

void do_write (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA		*obj = NULL;
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	char			title [MAX_STRING_LENGTH] = { '\0' };

	if ( IS_NPC (ch) ) {
		send_to_char ("Write is only available to PCs.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("What did you want to write on?\n", ch);
		return;
	}

	if ( !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) &&
		 !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		 !(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) ) {
		if ( IS_MORTAL (ch) || !write_pc_board (ch, buf, argument) ) {
			if ( IS_MORTAL (ch) || !write_virtual_board (ch, buf, argument) )
				send_to_char ("You can't find that.\n", ch);
		}
		return;
	}

    if ( obj->obj_flags.type_flag == ITEM_PARCHMENT ||
         obj->obj_flags.type_flag == ITEM_BOOK ) {
		do_scribe (ch, buf, 0);
        return;
    }

	if ( !*argument ) {
		send_to_char ("What would you like to write about?\n", ch);
		return;
	}

	if ( obj->obj_flags.type_flag != ITEM_BOARD ) {
		act ("You can't write on $p.", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	while ( *argument == ' ' )
		argument++;

		/* Get the name of the board */

	(void)one_argument (obj->name, buf);

	strcpy (title, argument);

	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);

		/* We need to borrow the poster slot to save the board name */

	ch->desc->pending_message->poster  = add_hash (buf);
	ch->desc->pending_message->message = NULL;
	ch->desc->pending_message->virtual = 0;
	ch->desc->pending_message->info	 = add_hash ("");
	ch->desc->pending_message->subject = add_hash (title);

	ch->desc->pending_message->flags = 0;

	send_to_char ("\n#6Enter your message below. To terminate, use the '@' character. Please ensure\n"
		      "you use proper linebreaks, and that your writing follows the acceptable posting\n"
                      "policies for our in-game boards as outlined in HELP POSTING_POLICIES.#0\n\n"
                      "1-------10--------20--------30--------40--------50--------60--------70--------80\n", ch);

	make_quiet (ch);

	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_message;
}

void do_jwrite (CHAR_DATA *ch, char *argument, int cmd)
{

	if ( IS_SET (ch->flags, FLAG_GUEST) ) {
		send_to_char ("Journals are unavailable to guests.\n", ch);
		return;
	}

	if ( IS_NPC (ch) ) {
		send_to_char ("Journals are only available to PCs.\n", ch);
		return;
	}

	if ( !*argument ) {
		send_to_char ("What did you wish your subject to be?\n", ch);
		return;
	}

	if ( !write_journal (ch, argument) ) {
		send_to_char ("There seems to be a problem with your journal.\n", ch);
		return;
	}
}

int read_journal_message (CHAR_DATA *ch, CHAR_DATA *reader, char *argument)
{
	MESSAGE_DATA	*message = NULL;
	char			name [MAX_STRING_LENGTH] = { '\0' };
	
	snprintf (name, MAX_STRING_LENGTH, "%s", ch->tname);

	if ( !strtol(argument, NULL, 10)) {
		if ( !reader )
			send_to_char ("Which entry?\n", ch);
		else
			send_to_char ("Which entry?\n", reader);
		return 1;
	}

	if ( !(message = load_message (name, 8, strtol(argument, NULL, 10))) ) {
		if ( !reader ) 
			send_to_char ("No such journal entry.\n", ch);
		else
			send_to_char ("No such journal entry.\n", reader);
		return 1;
	}

	snprintf (b_buf, MAX_STRING_LENGTH, "#6Date:#0    %s\n"
			"#6Subject:#0 %s\n\n%s", message->date, message->subject, message->message);

	if ( !reader ) {
		send_to_char ("\n", ch);
		page_string (ch->desc, b_buf);
	}
	else {
		send_to_char ("\n", reader);
		page_string (reader->desc, b_buf);
	}

	unload_message (message);

	return 1;
}

void do_jread (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA *who = NULL;
	char			name [MAX_STRING_LENGTH] = { '\0' };

	if ( IS_SET (ch->flags, FLAG_GUEST) ) {
		send_to_char ("Journals are unavailable to guests.\n", ch);
		return;
	}

	if ( IS_NPC (ch) ) {
		send_to_char ("Journals are only available to PCs.\n", ch);
		return;
	}

	if ( !IS_MORTAL (ch) ) {
		if ( !*argument )
			who = ch;
		else if ( *argument && isdigit (*argument) )
			who = ch;
		else {
			argument = one_argument (argument, name);
			*name = toupper (*name);
			if ( !(who = load_pc (name)) ) {
				send_to_char ("No such PC, I'm afraid.\n", ch);
				return;
			}
		}
		if ( !read_journal_message (who, ch, argument) ) {
			send_to_char ("There seems to be a problem with the journal.\n", ch);
			return;
		}
	}
	else {
		if ( !read_journal_message (ch, NULL, argument) ) {
			send_to_char ("There seems to be a problem with the journal.\n", ch);
			return;
		}
	}
}

void post_message (DESCRIPTOR_DATA *d)
{
	if ( !*d->pending_message->message )
		send_to_char ("No message posted.\n", d->character);
	else
		post_to_mysql_board (d);
}

void add_board (int level, char *name, char *title)
{
	BOARD_DATA	*board = NULL;
	BOARD_DATA	*board_entry = NULL;

			/* Make sure this board doesn't already exist */

	if ( board_lookup (name) )
		return;
	
	CREATE (board_entry, BOARD_DATA, 1);

	board_entry->level = level;
	board_entry->name = add_hash (name);
	board_entry->title = add_hash (title);
	board_entry->next_virtual = 1;

	board_entry->next = NULL;

		/* Add board_entry to end of full_board_list */

	if ( !full_board_list )
		full_board_list = board_entry;

	else {
		board = full_board_list;

		while ( board->next )
			board = board->next;

		board->next = board_entry;
	}
}

BOARD_DATA *board_lookup (char *name)
{
	BOARD_DATA *board = NULL;

	for ( board = full_board_list; board; board = board->next ) {
		if ( !str_cmp (board->name, name) )
			return board;
	}

	return NULL;
}

void show_unread_messages (CHAR_DATA *ch)
{
	int				header = 1;
	char            buf [MAX_STRING_LENGTH] = { '\0' };
	char		query [MAX_STRING_LENGTH] = { '\0' };
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row = NULL;

	mysql_safe_query ("SELECT * FROM player_notes WHERE name = '%s' AND flags = 0 ORDER BY post_number DESC", ch->tname);
	result = mysql_store_result (database);

	*b_buf = '\0';

	while ( (row = mysql_fetch_row(result)) ) {
		if ( header ) {
			snprintf (b_buf, MAX_STRING_LENGTH, "\nUnread messages on your private board:\n\n");
			header = FALSE;
		}
                if ( strlen(row[2]) > 44 ) {
                        snprintf (query, MAX_STRING_LENGTH, "%s", row[2]);
                        query[41] = '.';
                        query[42] = '.';
                        query[43] = '.';
                        query[44] = '\0';
                }
                else snprintf (query, MAX_STRING_LENGTH, "%s", row[2]);
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, " #6%3d#0 - %16s %-10.10s: %s\n", (int)strtol(row[1], NULL, 10), row[4], row[3], query);
	}

	if ( !header )		/* Meaning, we have something to print */
		page_string (ch->desc, b_buf);

	mysql_free_result (result);
	result = NULL;

	mysql_safe_query ("SELECT * FROM virtual_boards WHERE timestamp >= %d GROUP BY board_name ORDER BY board_name ASC", (int)ch->pc->last_logon);
	result = mysql_store_result (database);
	header = TRUE;

	while ( (row = mysql_fetch_row(result)) ) {
		if ( header ) {
			snprintf (b_buf, MAX_STRING_LENGTH, "\nWelcome back! Since you last logged in:\n\n");
			send_to_char (b_buf, ch);
			header = FALSE;
		}
		snprintf (buf, MAX_STRING_LENGTH,  "   - '#2%s#0' was posted to.\n", row[0]);
		send_to_char (buf, ch);
	}

	mysql_free_result (result);
}

void add_message (char *name, int msg_type, char *poster,
                  char *date, char *subject, char *info, char *message,
				  long flags)
{
	int				named = 0;
	int				day = 0;
	int				virtual = 0; /* number of the message in board */
	MESSAGE_DATA	*msg = NULL;
	BOARD_DATA		*board = NULL;
	CHAR_DATA		*ch = NULL;
	char			date_buf [MAX_STRING_LENGTH] = { '\0' };
	char			*suf = NULL;

	

	if ( !date ) {
		date = timestr(date);
	}
		/*******************
		* 	 msg_type
		*	1	IG board
		*	2	vboard
		*	3	player notes
		*	4	journal
		**********************/

	if ( msg_type != 1 && msg_type != 2 && msg_type != 3 && msg_type != 4 && !(board = board_lookup (name)) ) {
		printf ("No board for message; board:  '%s'.\n", name);
		return;
	}

	msg = (MESSAGE_DATA *)alloc (sizeof (MESSAGE_DATA),1);

	if ( msg_type == 3 ) {

		if ( !(ch = load_pc (name)) ) {
			system_log ("No such character by name in add_message()!", TRUE);
			return;
		}

		virtual = ch->pc->staff_notes;
		named = 1;

		unload_pc (ch);
	}

	if ( !virtual && name )
		virtual = 1;
		
	else if ( virtual >= board->next_virtual )
		board->next_virtual = virtual + 1;
		
	else
		system_log ("Virtual requested less than board's next virtual, add_message()", TRUE);

	*date_buf = '\0';

	day = time_info.day + 1;
	if ( day == 1 )
		suf = "st";
	else if ( day == 2 )
		suf = "nd";
	else if ( day == 3 )
		suf = "rd";
	else if ( day < 20 )
		suf = "th";
	else if ( (day % 10) == 1 )
		suf = "st";
	else if ( (day % 10) == 2 )
		suf = "nd";
	else if ( (day % 10) == 3 )
		suf = "rd";
	else
		suf = "th";

        if ( time_info.holiday == 0 &&
                !(time_info.month == 1 && day == 12) &&
                !(time_info.month == 4 && day == 10) &&
                !(time_info.month == 7 && day == 11) &&
                !(time_info.month == 10 && day == 12) )
		snprintf (date_buf, MAX_STRING_LENGTH, "%d%s %s, %d SR", day, suf, month_short_name[time_info.month], time_info.year);
        else {
                if ( time_info.holiday > 0 ) {
			snprintf (date_buf, MAX_STRING_LENGTH, "%s, %d SR", holiday_short_names[time_info.holiday], time_info.year);
                }
                else if ( time_info.month == 1 && day == 12 )
                        snprintf (date_buf, MAX_STRING_LENGTH, "Erukyerme, %d SR", time_info.year);
                else if ( time_info.month == 4 && day == 10 )
                        snprintf (date_buf, MAX_STRING_LENGTH, "Lairemerende, %d SR", time_info.year);
                else if ( time_info.month == 7 && day == 11 )
                        snprintf (date_buf, MAX_STRING_LENGTH, "Eruhantale, %d SR", time_info.year);
                else if ( time_info.month == 10 && day == 12 )
                        snprintf (date_buf, MAX_STRING_LENGTH, "Airilaitale, %d SR", time_info.year);
        }

	if ( isalpha (*date_buf) )
		*date_buf = toupper(*date_buf);

	
	msg->virtual	= virtual;
	msg->flags		= flags;
	msg->poster		= add_hash (poster);
	msg->date	 	= add_hash (date);
	msg->subject	= tilde_eliminator (subject);
	msg->info		= add_hash (info);
	msg->message	= tilde_eliminator (message);
	msg->icdate		= add_hash (date_buf);

    if ( msg_type == 1 && ch ) { /* IG boards */
		ch->desc->pending_message = msg;
		post_to_mysql_board (ch->desc);
		return;
	}
        
	else if ( msg_type == 2 ) { /* vboard */
		add_message_to_mysql_vboard (name, poster, msg);
		return;
	}
	
	else if ( msg_type == 3 ) { /*  player notes */
		add_message_to_mysql_player_notes (name, poster, msg);
		return;
	}

	else if ( msg_type == 4 ) { /*  journal */
		ch->desc->pending_message = msg;
		post_to_mysql_journal (ch->desc);
		return;
	}
	if ( !msg_type) {
		unload_message (msg);
		return;
	}

	system_log ("Reached end of add_message()", TRUE);

	unload_message (msg);
}


char *read_a_line (FILE *fp)
{
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	(void)fgets (buf, MAX_STRING_LENGTH, fp);

	if ( *buf )
		buf [strlen (buf) - 1] = '\0';

	return str_dup (buf);
}

MESSAGE_DATA *load_message (char *msg_name, int pc_message, int msg_number)
{
	MESSAGE_DATA	*message = NULL;

	if ( pc_message == 5 ) {
		message = load_mysql_message (msg_name, 1, msg_number);
		return message;
	}
	else if ( pc_message == 6 ) {
		message = load_mysql_message (msg_name, 0, msg_number);
		return message;
	}
	else if ( pc_message == 7 ) {
		message = load_mysql_message (msg_name, 2, msg_number);
		return message;
	}
	else if ( pc_message == 8 ) {
		message = load_mysql_message (msg_name, 3, msg_number);
		return message;
	}

	return message;
	
}

void unload_message (MESSAGE_DATA *message)
{
	if ( message->poster )
		mem_free (message->poster);

	if ( message->date )
		mem_free (message->date);

	if ( message->subject )
		mem_free (message->subject);

	if ( message->info )
		mem_free (message->info);

	if ( message->message )
		mem_free (message->message);

	if ( message->icdate )
		mem_free (message->icdate);

	mem_free (message);
}

void do_notes (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*who = NULL;
	bool			messages = FALSE;
	char			name [MAX_STRING_LENGTH] = { '\0' };

	argument = one_argument (argument, name);

	if ( !*name ) {
		send_to_char ("Which PC or virtual board did you wish to get a listing for?\n", ch);
		return;
	}

	*name = toupper (*name);

	if ( !(who = load_pc (name)) ) {
		messages = get_mysql_board_listing(ch, 1, name);
		if ( !messages ) {
			send_to_char ("No such PC or vboard.\n", ch);
			return;
		}
		else return;
	}

	unload_pc (who);

	messages = get_mysql_board_listing (ch, 2, name);

	if ( !messages )
		send_to_char ("That player does not have any notes.\n", ch);
}

void do_journal (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*who = NULL;
	char			name [MAX_STRING_LENGTH] = { '\0' };
	int			messages = 0;

	if ( IS_SET (ch->flags, FLAG_GUEST) ) {
		send_to_char ("Journals are unavailable to guests.\n", ch);
		return;
	}

	if ( !IS_MORTAL (ch) ) {
		argument = one_argument (argument, name);
		if ( !*name )
			who = ch;
		else {
			*name = toupper (*name);
			if ( !(who = load_pc (name)) ) {
				send_to_char ("No such PC, I'm afraid.\n", ch);
				return;
			}
		}
	}
	else 
		who = ch;

	messages = get_mysql_board_listing (ch, 3, who->tname);
	if ( !messages )
		send_to_char ("No journal entries found.\n", ch);

	if ( who && who != ch )
		unload_pc (who);
}

void do_notify (CHAR_DATA *ch, char *argument, int cmd)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row = NULL;
	CHAR_DATA		*tch = NULL;
	AFFECTED_TYPE	*af = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		buf2 [MAX_STRING_LENGTH] = { '\0' };
	bool		notify = FALSE;
	char		*temp_arg = NULL;
	
	if ( !GET_TRUST (ch) )
		send_to_char ("#6Note:  This command is here solely to facilitate RP, and is therefore strictly OOC.#0\n", ch);

	while ( *argument == ' ' )
		argument++;

	strcpy (buf, argument);

	while ( *buf && *(ADDBUF - 1) == ' ' )
		buf [strlen (buf) - 1] = '\0';

	if ( !*buf ) {

		if ( (af = get_affect (ch, MAGIC_CLAN_NOTIFY)) ) {

			if ( !is_he_somewhere ((CHAR_DATA *) af->a.spell.t) ) {
				send_to_char ("\nThey're no longer online.\n", ch);
				affect_remove (ch, af);
				return;
			}

			send_to_char ("\nClan member has been notified.\n", ch);
			temp_arg = char_short(ch);
			snprintf (buf, MAX_STRING_LENGTH,  "#3[%s is online.]#0\n", temp_arg);
			buf [3] = toupper (buf [3]);

			send_to_char (buf, (CHAR_DATA *) af->a.spell.t);

			if ( !IS_SET (((CHAR_DATA *)af->a.spell.t)->plr_flags, MUTE_BEEPS) )
				send_to_char ("\a", (CHAR_DATA *) af->a.spell.t);

			affect_remove (ch, af);

			return;
		}

		if ( !(af = get_affect (ch, MAGIC_NOTIFY)) ) {
			send_to_char ("\nNobody has notified you that they were online.\n", ch);
			return;
		}

		if ( !is_he_somewhere ((CHAR_DATA *) af->a.spell.t) ) {
			send_to_char ("\nThey're no longer online.\n", ch);
			affect_remove (ch, af);
			return;
		}

		send_to_char ("#3[Notifyee is online.]#0\n", (CHAR_DATA *) af->a.spell.t);
		send_to_char ("Requesting party notified.\n", ch);

		affect_remove (ch, af);

		return;
	}

	if ( is_clan_member_player (ch, buf) ) {

		send_to_char ("\nAll clan members currently online have been notified.\n", ch);
		temp_arg = char_short(ch);
		snprintf (buf2, MAX_STRING_LENGTH,  "#3[%s (%s) is online.  Use NOTIFY to reply in "
					   "kind.]#0", temp_arg, buf);
		buf2 [3] = toupper (buf2 [3]);

		for ( tch = character_list; tch; tch = tch->next ) {

			if ( ch == tch ||
				 tch->deleted ||
				 IS_NPC (tch) )
				continue;

			if ( !is_clan_member_player (tch, buf) )
				continue;

			act (buf2, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);

			magic_add_affect (tch, MAGIC_CLAN_NOTIFY, 2, 0, 0, 0, 0);

			get_affect (tch, MAGIC_CLAN_NOTIFY)->a.spell.t = (int) ch;
		}

		return;
	}

	if ( !(tch = get_pc (buf)) ) {
		if ( !(tch = load_pc (buf)) ) {
			mysql_safe_query ("SELECT name FROM %s.pfiles WHERE keywords LIKE '%%%s%%'", PFILES_DATABASE, buf);
			result = mysql_store_result (database);
			if ( !result || mysql_num_rows(result) == 0 ) {
				mysql_safe_query ("SELECT name FROM %s.pfiles WHERE sdesc LIKE '%%%s%%'", PFILES_DATABASE, buf);
				result = mysql_store_result (database);
			}
			if ( result && mysql_num_rows(result) > 0 && mysql_num_rows(result) <= 50 ) {
				while ( (row = mysql_fetch_row(result)) ) {
					if ( (tch = get_pc (row[0])) ) {
						temp_arg = char_short(ch);
						snprintf (buf, MAX_STRING_LENGTH,  "\a#3[%s is online.  Use NOTIFY to reply in kind.]#0\n", temp_arg);
						buf [4] = toupper (buf [4]);
						send_to_char (buf, tch);
						if ( !IS_SET (tch->plr_flags, MUTE_BEEPS) )
							send_to_char ("\a", tch);
						if ( (af = get_affect (tch, MAGIC_NOTIFY)) ) {
							af->a.spell.t = (int) ch;
							af->a.spell.duration = 2;
							if ( result )
								mysql_free_result(result);
							return;
						}
						magic_add_affect (tch, MAGIC_NOTIFY, 2, 0, 0, 0, 0);
						get_affect (tch, MAGIC_NOTIFY)->a.spell.t = (int) ch;
						notify = TRUE;
					}
				}
				if ( result )
					mysql_free_result(result);
				if(!IS_MORTAL(ch)) {
					if ( notify )
						send_to_char ("\nParty was found online and notified.\n", ch);
					else 
						send_to_char ("\nNo PC online has such a description or alias.  Did you spell it fully and correctly?\n",ch);
				} else {
					send_to_char ("\nIf this individual is online they have been notified.\n", ch);
				}
				return;
			}
			if ( result && mysql_num_rows(result) > 50 ) {
				send_to_char ("\nToo many matches were found. Please try elaborating on the description.\n", ch);
				mysql_free_result(result);
				return;
			}

			if( !IS_MORTAL(ch) )
				send_to_char ("\nNo PC has such a name.  Did you spell it fully and correctly?\n",ch);
			else 
				send_to_char ("\nIf this individual is online they have been notified.\n", ch);
			return;
		}

		unload_pc (tch);
		send_to_char ("\nIf this individual is online they have been notified.\n", ch);
		return;
	}

	send_to_char ("\nIf this individual is online they have been notified.\n", ch);
	temp_arg = char_short(ch);
	snprintf (buf, MAX_STRING_LENGTH,  "#3[%s is online.  Use NOTIFY to reply in kind.]#0\n", temp_arg);
	buf [3] = toupper (buf [3]);

	send_to_char (buf, tch);

	if ( !IS_SET (tch->plr_flags, MUTE_BEEPS) )
		send_to_char ("\a", tch);

	if ( (af = get_affect (tch, MAGIC_NOTIFY)) ) {
		af->a.spell.t = (int) ch;
		af->a.spell.duration = 2;
		return;
	}

	magic_add_affect (tch, MAGIC_NOTIFY, 2, 0, 0, 0, 0);

	get_affect (tch, MAGIC_NOTIFY)->a.spell.t = (int) ch;
}

void post_writing (DESCRIPTOR_DATA *d)
{
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*quill = NULL;
	CHAR_DATA	*ch = NULL;
	OBJ_DATA	*ink = NULL;
	WRITING_DATA	*writing = NULL;
	int			i = 0;
	char		*date = NULL;
	char		message [MAX_STRING_LENGTH] = { '\0' };
	float		mod = 0;

	ch = d->character;

	date = timestr(date);
	
	if ( !(obj = ch->pc->writing_on) ) {
		send_to_char ("That object is no longer there!\n", ch);
		mem_free (date);
		return;
	}

	snprintf (message, MAX_STRING_LENGTH, "%s", d->pending_message->message);

	if ( ch->right_hand && ch->right_hand->obj_flags.type_flag == ITEM_INK ) {
		ink = ch->right_hand;
		if ( ink->o.od.value[1] <= 0 ) {
			send_to_char ("\nHaving exhausted the last of your ink, you discard the now-empty vessel.\n", ch);
			extract_obj (ink);
		}
	}

	if ( ch->left_hand && ch->left_hand->obj_flags.type_flag == ITEM_INK ) {
		ink = ch->left_hand;
		if ( ink->o.od.value[1] <= 0 ) {
			send_to_char ("\nHaving exhausted the last of your ink, you discard the now-empty vessel.\n", ch);
			extract_obj (ink);
		}
	}

	if ( (quill = ch->right_hand) ) {
		if ( GET_ITEM_TYPE(quill) == ITEM_WRITING_INST ) {
			if ( quill->ink_color )
				quill->ink_color = NULL;
		}
		else quill = NULL;
	}

	if ( !quill && ch->left_hand ) {
		quill = ch->left_hand;
		if ( GET_ITEM_TYPE(quill) == ITEM_WRITING_INST ) {
			if ( quill->ink_color )
				quill->ink_color = NULL;
		}
		else quill = NULL;
	}

	mod = (ch->skills[ch->writes]*0.50) + (ch->skills[ch->speaks]*0.30) + (ch->skills[SKILL_LITERACY]*0.20);
	mod = (float)((int)(MIN(95,(int)mod)));

	if ( GET_ITEM_TYPE (obj) == ITEM_PARCHMENT ) {
		CREATE (obj->writing, WRITING_DATA, 1);
		obj->writing->ink = add_hash(ch->delay_who);
		obj->writing->author = add_hash(ch->tname);
		obj->writing->date = add_hash(date);
		obj->writing->language = ch->speaks;
		obj->writing->script = ch->writes;
		obj->writing->message = tilde_eliminator(message);
		obj->writing->skill = (int)mod;
	}

	if ( GET_ITEM_TYPE (obj) == ITEM_BOOK ) {
		for ( i = 2, writing = obj->writing; i <= obj->open; i++ ) {
			if ( obj->writing->next_page ) {
				writing = writing->next_page;
			}
		}
		writing->ink = add_hash(ch->delay_who);
		writing->author = add_hash(ch->tname);
		writing->date = add_hash(date);
		writing->language = ch->speaks;
		writing->script = ch->writes;
		writing->message = tilde_eliminator(message);
		writing->skill = (int)mod;
	}

	ch->pc->writing_on = NULL;
	ch->delay_who = NULL;
	unload_message (d->pending_message);

	skill_use (ch, ch->writes, 0);
	if ( !number(0,1) )
		skill_use (ch, ch->speaks, 0);
	if ( !number(0,2) )
		skill_use (ch, SKILL_LITERACY, 0);

	save_writing (obj);

	mem_free (date);
}

void do_flip (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		buffer [MAX_STRING_LENGTH] = { '\0' };
	char		*temp_arg = NULL;
	
	argument = one_argument (argument, buf);

	if ( !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) ) {
		send_to_char ("What did you want to flip?\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (obj) != ITEM_BOOK ) {
		send_to_char ("That isn't a book, unfortunately.\n", ch);
		return;
	}

	if ( !obj->open ) {
		send_to_char ("You'll need to open it, first.\n", ch);
		return;
	}

	if ( !obj->o.od.value[0] ) {
		send_to_char ("It doesn't have any pages left to flip.\n", ch);
		return;
	}

	if ( *argument && !isdigit(*argument) ) {
		send_to_char ("Which page would you like to flip to?\n", ch);
		return;
	}

	if ( !*argument )
		snprintf (argument, MAX_STRING_LENGTH,  "%d", obj->open+1);

	if ( strtol(argument, NULL, 10) == obj->open ) {
		send_to_char ("It's already open to that page.\n", ch);
		return;
	}

	if ( strtol(argument, NULL, 10) > obj->o.od.value[0] ) {
		snprintf (buf, MAX_STRING_LENGTH,  "There are only %d pages in this book.\n", obj->o.od.value[0]);
		send_to_char (buf, ch);
		return;
	}

	if ( !*argument ) {
		send_to_char ("Which page did you wish to flip to?\n", ch);
		return;
	}

	if ( strtol(argument, NULL, 10) > obj->open + 1 )
		snprintf (buf, MAX_STRING_LENGTH,  "You leaf carefully through #2%s#0 until you arrive at page %d.", obj->short_description, (int)strtol(argument, NULL, 10));
	else
		snprintf (buf, MAX_STRING_LENGTH,  "You turn #2%s's#0 page.", obj->short_description);
	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	if ( strtol(argument, NULL, 10) > obj->open + 1 ){
		temp_arg = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "%s#0 leafs carefully through #2%s#0 until %s arrives at the desired page.", temp_arg, obj->short_description, HSSH(ch)); 
	}
	else{
		temp_arg = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "%s#0 turns #2%s's#0 page.", temp_arg, obj->short_description);
	}
	snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", CAP(buf));
	act (buffer, FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
	obj->open = (int)strtol(argument, NULL, 10);
	
	return;
}

void do_tear (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*parchment = NULL;
	WRITING_DATA	*writing = NULL;
	WRITING_DATA	*page = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		buffer [MAX_STRING_LENGTH] = { '\0' };
	int			i = 0;
	char		*temp_arg = NULL;

	argument = one_argument (argument, buf);

	if ( !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) ) {
		send_to_char ("What did you wish to tear?\n", ch);
		return;
	}

	if ( ch->right_hand && ch->left_hand ) {
		send_to_char ("You must have one hand free.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (obj) != ITEM_PARCHMENT && GET_ITEM_TYPE (obj) != ITEM_BOOK) {
		send_to_char ("I'm afraid that can't be torn.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (obj) == ITEM_BOOK ) {
		if ( !obj->writing_loaded )
			load_writing (obj);
		if ( !obj->open ) {
			send_to_char ("Open it to the page you wish to tear out, first.\n", ch);
			return;
		}
		if ( !obj->writing || !obj->o.od.value[0] ) {
			send_to_char ("It doesn't have any pages left!\n", ch);
			return;
		}
		if ( !(parchment = load_object(61)) ) {
			send_to_char ("The parchment prototype (VNUM 61) appears to be missing. Please inform staff.\n", ch);
			return;
		}
		for ( i = 1, writing = obj->writing; i <= obj->open; i++ ) {
			if ( i == 1 && i == obj->open ) {
				snprintf (buf, MAX_STRING_LENGTH,  "You carefully tear page %d from #2%s#0.", i, obj->short_description);
				page = writing;
				if ( !obj->writing->next_page ) {
					obj->writing = NULL;
				}
				else obj->writing = writing->next_page;
				break;
			}
			else if ( i + 1 == obj->open && obj->open > 1 ) {
				snprintf (buf, MAX_STRING_LENGTH,  "You carefully tear page %d from #2%s#0.", i+1, obj->short_description);
				page = writing->next_page;
				if ( !writing->next_page->next_page ) {
					writing->next_page = NULL;
					obj->open--;
				}
				else {
					writing->next_page = writing->next_page->next_page;
				}
				break;
			}
			writing = writing->next_page;
		}

		act (buf, FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
		temp_arg = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "%s#0 carefully tears a page from #2%s#0.", temp_arg, obj->short_description);
		snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", CAP(buf));
		act (buffer, FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

		if ( *page->message && str_cmp (page->message, "blank") ) {
			CREATE (parchment->writing, WRITING_DATA, 1);
			parchment->writing->ink = add_hash(page->ink);
			parchment->writing->author = add_hash(page->author);
			parchment->writing->date = add_hash(page->date);
			parchment->writing->language = page->language;
			parchment->writing->script = page->script;
			parchment->writing->message = add_hash(page->message);
			parchment->writing->skill = page->skill;
		}

		obj->o.od.value[0]--;
		obj_to_char (parchment, ch);
		save_writing (obj);
		save_writing (parchment);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "You rend #2%s#0 into small pieces, which you then meticulously discard.", obj->short_description);
	act (buf, FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
	temp_arg = char_short(ch);
	snprintf (buf, MAX_STRING_LENGTH,  "%s rends #2%s#0 into small pieces, which %s then meticulously discards.", temp_arg, obj->short_description, HSSH(ch));
	snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", CAP(buf));
	act (buffer, FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

	extract_obj (obj);
}

void do_dip (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*quill = NULL, *ink = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	if ( !(ch->left_hand && ch->left_hand->obj_flags.type_flag == ITEM_WRITING_INST) &&
	     !(ch->right_hand && ch->right_hand->obj_flags.type_flag == ITEM_WRITING_INST) ) {
		send_to_char ("You need to be holding a writing implement.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_WRITING_INST )
			quill = ch->right_hand;
		else quill = ch->left_hand;

		if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_INK )
			ink = ch->right_hand;
		else if	( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_INK )
			ink = ch->left_hand;
	}
	else {
		quill = get_obj_in_list_vis (ch, buf, ch->right_hand);
		if ( !quill )
			quill = get_obj_in_list_vis (ch, buf, ch->left_hand);
		if ( quill && GET_ITEM_TYPE (quill) != ITEM_WRITING_INST ) {
			send_to_char ("You must specify first the writing implement, and then the ink source.\n", ch);
			return;
		}
		argument = one_argument (argument, buf);
		if ( !*buf ) {
			send_to_char ("You must specify both a writing implement and an ink source.\n", ch);
			return;
		}
		ink = get_obj_in_list_vis (ch, buf, ch->right_hand);
		if ( !ink )
			ink = get_obj_in_list_vis (ch, buf, ch->left_hand);
		if ( !ink )
			ink = get_obj_in_list_vis (ch, buf, ch->room->contents);
		if ( ink && GET_ITEM_TYPE (ink) != ITEM_INK ) {
			send_to_char ("You must specify first the writing implement, and then the ink source.\n", ch);
			return;
		}
	}

	if ( !ink ) {
		send_to_char ("You need to have an ink source in one hand, or in the room.\n", ch);
		return;
	}

	if ( !quill ) {
		send_to_char ("You need to be holding a writing implement.\n", ch);
		return;
	}

	if ( ink->o.od.value[0] <= 0 ) {
		send_to_char ("Your ink source seems to be empty.\n", ch);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "You dip #2%s#0 carefully into #2%s#0, liberally coating its tip.", quill->short_description, ink->short_description);
	quill->ink_color = ink->ink_color;
	act(buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	ink->o.od.value[0]--;
}

void do_scribe (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*quill = NULL;
	WRITING_DATA	*writing = NULL;
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	char			buffer [MAX_STRING_LENGTH] = { '\0' };
	int				i = 0;
	int				script = 0;
	char			*temp_arg = NULL;

	if ( IS_NPC (ch) ) {
		send_to_char ("This is only available to PCs.\n", ch);
		return;
	}

	if ( ch->skills [SKILL_LITERACY] < 10 ) {
		send_to_char ("You aren't literate enough to do that.\n", ch);
		return;
	}

	for ( i = SKILL_SCRIPT_SARATI; i <= SKILL_SCRIPT_ANGERTHAS_EREBOR; i++ )
		if ( ch->skills [i] )
			script = 1;

	if ( !script ) {
		send_to_char ("You have no knowledge of any written scripts.\n", ch);
		return;
	}

	if ( !ch->writes ) {
		send_to_char ("In which script would you like to write? (See the SCRIBE command.)\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (!(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) ) {
		send_to_char ("You can't find that!\n", ch);
		return;
	}

	if ( !ch->left_hand && !ch->right_hand ) {
		send_to_char ("You need to be holding a writing implement.\n", ch);
		return;
	}

	if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_WRITING_INST )
		quill = ch->right_hand;
	else if ( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_WRITING_INST )
		quill = ch->left_hand;
	else {
		send_to_char ("You need to be holding a writing implement.\n", ch);
		return;
	}

	if ( !quill->ink_color ) {
		send_to_char ("The writing instrument must be dipped in ink, first.\n", ch);
		return;
	}

	ch->delay_who = quill->ink_color;

	if ( obj->obj_flags.type_flag != ITEM_PARCHMENT &&
	     obj->obj_flags.type_flag != ITEM_BOOK ) {
		act ("That cannot be written on.", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	if ( !obj->writing_loaded )
		load_writing (obj);

	if ( GET_ITEM_TYPE(obj) == ITEM_PARCHMENT && obj->writing ) {
		send_to_char ("That has already been written on.\n", ch);
		return;
	}

	else if ( GET_ITEM_TYPE (obj) == ITEM_BOOK ) {
		if ( !obj->open ) {
			send_to_char ("You need to open it, first.\n", ch);
			return;
		}
		if ( !obj->writing || !obj->o.od.value [0] ) {
			send_to_char ("It doesn't have any pages left!\n", ch);
			return;
		}
		for ( i = 2, writing = obj->writing; i <= obj->open; i++ ) {
			if ( obj->writing->next_page ) {
				writing = writing->next_page;
			}
		}
		if ( str_cmp (writing->message, "blank") ) {
			send_to_char ("That has already been written on.\n", ch);
			return;
		}
	}

	temp_arg = char_short(ch);
	snprintf (buffer, MAX_STRING_LENGTH,  "#5%s#0 begins writing on #2%s#0.", temp_arg, obj->short_description);
	buffer[3] = toupper(buffer[3]);
	act (buffer, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);

	ch->desc->pending_message->message = NULL;

	send_to_char ("Scribe your message; terminate with an '@'. Please keep its length plausible\nfor the size of the writing object, since we have opted against coded limits.\n", ch);
	snprintf (buf, MAX_STRING_LENGTH,  "1-------10--------20--------30--------40--------50--------60--------70\n");
	send_to_char(buf, ch);	

	make_quiet (ch);

	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_writing;
	ch->pc->writing_on = obj;
	
	return;
}

/**************************************************************
do_evaluate will provide information on the structure, quality and other physical charactertics of items

Currently it provides a description of the item, weight, quality, hours of use in light objects, contents of containers
****************************************************************/
void do_evaluate (CHAR_DATA *ch, char *argument, int cmd)
{


	char		arg1 [MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA	*obj = NULL;
	int 		swag = 0;
	int 		guess_weight = 0;
	int			temp = 0;
	char		buffer [MAX_STRING_LENGTH] = { '\0' };
	char		*temp_arg = NULL;

/*** CHECK FOR POSTIONS AND CONDITONS FIRST ***/

	if ( GET_POS (ch) < POSITION_SLEEPING ) {
		send_to_char ("You are unconscious!\n", ch);
		return;
	}

	if ( GET_POS (ch) == POSITION_SLEEPING ) {
		send_to_char ("You are asleep.\n", ch);
		return;
	}

	if ( is_blind (ch) ) {
		send_to_char ("You are blind!\n", ch);
		return;
	}
	
	argument = one_argument (argument, arg1);

	if ( !*arg1 ) {
		send_to_char ("Asses what?\n", ch);
		return;
	}

	if ( !(obj = get_obj_in_dark (ch, arg1, ch->right_hand)) &&
		 !(obj = get_obj_in_dark (ch, arg1, ch->left_hand)) &&
		 !(obj = get_obj_in_dark (ch, arg1, ch->equip))) {
		
		send_to_char ("You don't have that.\n", ch);
		return;
	}

/*** Describe the object ***/	
	if (obj) {
		snprintf (buffer, MAX_STRING_LENGTH,  "\n   #2%s#0\n", obj->short_description); 
		
		if (obj->full_description){
			snprintf (buffer + strlen(buffer), MAX_STRING_LENGTH, "\n   %s", obj->full_description); 
		}
		act (buffer, FALSE, ch, obj, 0, TO_CHAR);
		*buffer = '\0';
	}
	else{
		send_to_char ("You assay something unless you have it.\n", ch);
		return;
	}
/*** Determine the objects quality ***/
	if (obj->quality){
		snprintf (buffer, MAX_STRING_LENGTH,  "\n   It has a %d percent chance of breaking.", obj->quality); 
		act (buffer, FALSE, ch, obj, 0, TO_CHAR);
		*buffer = '\0';
	}

/**** Guess at the Weight based on scan skill **/

		
	if (IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE)){
		swag = number (90, 110);
		guess_weight = ((obj->obj_flags.weight + obj->contained_wt)/swag);

		if (skill_use(ch, 32, 0)){
			if (guess_weight <= 1){
				snprintf (buffer, MAX_STRING_LENGTH,  "\n   You would guess that this item weighs less than a pound.");
			}
			else{  /** weighs more than a pound **/
				snprintf (buffer, MAX_STRING_LENGTH,  "\n   You would guess that this item weighs about %d pounds.",guess_weight);
			}
		}
		else{ /** failed skill check **/
			snprintf (buffer, MAX_STRING_LENGTH,  "\n   You can't even begin to guess how much this weighs.");
		}
	}
	else{ /** no way to check weight - message for non-takeable objects **/
		snprintf (buffer, MAX_STRING_LENGTH,  "\n   You can't even begin to guess how much this weighs.");  
	}

	act (buffer, FALSE, ch, obj, 0, TO_CHAR);
	*buffer = '\0';

/*** end weight guess ***/


/***** Pslim lantern code ***/
	if ( GET_ITEM_TYPE (obj) == ITEM_LIGHT ) {
		if ( obj->o.light.hours <= 0 ){
			snprintf (buffer, MAX_STRING_LENGTH,  "\n  The $o is empty.");
		}
		else {
			temp = obj->o.light.hours;
			snprintf (buffer, MAX_STRING_LENGTH,  "\n   You think the $o will last another %d hours.", temp);
		}
	act (buffer, FALSE, ch, obj, 0, TO_CHAR);
	*buffer = '\0';

	}
	
	
/***** end Pslim lantern code *****/


/**** LOOK INSIDE A DRINK CONTATINER ***/

	if ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON ) {
		if ( obj->o.drinkcon.volume <= 0 ){
			snprintf (buffer, MAX_STRING_LENGTH,  "\n   The $o is empty.");
		}
		else {
			if ( obj->o.drinkcon.capacity ) {
				temp = (obj->o.drinkcon.volume * 3) / obj->o.drinkcon.capacity;
			}	
			else{
				temp = 1;
			}
			temp_arg = vnum_to_liquid_name (obj->o.drinkcon.liquid);
			snprintf (buffer, MAX_STRING_LENGTH,  "\n   The $o is %sfull of %s.", fullness [temp], temp_arg);
			
		}
	act (buffer, FALSE, ch, obj, 0, TO_CHAR);
	*buffer = '\0';

	}

	
/**** LOOK INSIDE A NON-DRINK CONTAINER ***/

	if ( GET_ITEM_TYPE (obj) == ITEM_CONTAINER ||
	    GET_ITEM_TYPE (obj) == ITEM_QUIVER ||
	  	GET_ITEM_TYPE (obj) == ITEM_SHEATH ||
		  
		(GET_ITEM_TYPE (obj) == ITEM_WEAPON &&
	 	obj->o.weapon.use_skill == SKILL_SLING) ||
		  
		GET_ITEM_TYPE (obj) == ITEM_KEYRING ) {

		if ( IS_SET (obj->o.container.flags, CONT_CLOSED) ) {
			send_to_char ("It is closed.\n", ch);
			return;
		}
		temp_arg = fname (obj->name);
		send_to_char (temp_arg, ch);

		if ( obj->in_room != NOWHERE )
			send_to_char (" (here) : \n", ch);
		else if ( obj->carried_by )
			send_to_char (" (carried) : \n", ch);
		else
			send_to_char (" (used) : \n", ch);

		list_obj_to_char (obj->contains, ch, 1, TRUE);
	}
	else{
			send_to_char("\n   It is not a regular container.\n", ch);
	}
	
	return;
}
