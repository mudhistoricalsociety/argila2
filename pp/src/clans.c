/** 
*	\file clans.c
*	Action and information regarding clans  
*
*	The purpose of this module is provide information about clans, changes to
*	clans, and effects of clans
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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

extern NAME_SWITCH_DATA	*clan_name_switch_list;

char *display_clan_ranks (CHAR_DATA *ch, CHAR_DATA *observer)
{
	CLAN_DATA	*clan = NULL;
	static char	buf [MAX_STRING_LENGTH] = { '\0' };
	char		*argument = NULL, *argument2 = NULL;
	char		ranks_list [MAX_STRING_LENGTH] = { '\0' };
	char		clan_name [MAX_STRING_LENGTH] = { '\0' };
	char		clan_name2 [MAX_STRING_LENGTH] = { '\0' };
	char		name [MAX_STRING_LENGTH] = { '\0' };
	int			flags = 0, flags2 = 0, clans = 0;
	bool		first = TRUE;
	char		*temp_arg = NULL;
	
	argument = observer->clans;

	*ranks_list = '\0';

	snprintf (buf, MAX_STRING_LENGTH,  "You recognize that %s carries the rank of", HSSH(ch));

    while ( get_next_clan (&argument, clan_name, &flags) ) {
    	argument2 = ch->clans;
        
        while ( get_next_clan (&argument2, clan_name2, &flags2) ) {
        	
        	if ( !str_cmp (clan_name, "osgi_citizens") ||
            	 !str_cmp (clan_name, "mm_denizens") )
                continue;
			
			clan = get_clandef (clan_name);
			
			if ( !clan )
				continue;
            
            if ( !str_cmp (clan_name, clan_name2) ) {
				if ( *ranks_list ) {
					strcat (buf, ranks_list);
			
					if ( !first )
						strcat (buf, ",");
				
					*ranks_list = '\0';
				}
						
				snprintf (name, MAX_STRING_LENGTH, "%s", clan->literal);
				
				if ( !cmp_strn (clan->literal, "The ", 4) )
					*name = tolower(*name);
				
				if ( cmp_strn (clan->literal, "the ", 4) ){
					temp_arg = get_clan_rank_name (flags2);
					snprintf (name, MAX_STRING_LENGTH, "the %s", clan->literal);
					snprintf (ranks_list + strlen(ranks_list), MAX_STRING_LENGTH, " %s in %s", temp_arg, name); 
				}
						
				first = FALSE;
				clans++;
            }
        }
    }

	if ( *ranks_list ) {
		if ( clans > 1 )
			strcat (buf, " and");
	
		strcat (buf, ranks_list);
	} 

	strcat (buf, ".");

	if (clan)
		free(clan);  /* clan = get_clandef (clan_name); */
		
	if ( clans )
		return buf;
	else
		return NULL;
		
}

void notify_captors (CHAR_DATA *ch)
{
	CLAN_DATA	*clan_def = NULL;
	DESCRIPTOR_DATA	*d = NULL;
	char		*p = NULL;
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};
	int			clan_flags = 0;

	if ( !ch->room )
		return;

	if ( !is_in_cell (ch, ch->room->zone) )
		return;

	for ( d = descriptor_list; d; d = d->next ) {
		if ( !d->character || d->connected != CON_PLYNG )
			continue;
		if ( !d->character->room )
			continue;
		p = d->character->clans;
		while ( get_next_clan (&p, clan_name, &clan_flags) ) {
	                if ( (clan_def = get_clandef (clan_name)) )   
	                        strcpy (clan_name, clan_def->literal);
			if ( !clan_def )
				continue;
			if ( !clan_def->zone )
				continue;
			if ( clan_def->zone == ch->room->zone ) {
				send_to_char ("\n#3A PC prisoner has logged into one of your cells and requires attention.#0\n", d->character);
				break;
			}
		}
	}
	return;
}

void show_waiting_prisoners (CHAR_DATA *ch)
{
	CLAN_DATA	*clan_def = NULL;
	DESCRIPTOR_DATA	*d;
	char	*p = NULL;
	char	clan_name [MAX_STRING_LENGTH] = {'\0'};
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	int		clan_flags = 0;
	int		prisoners = 0;

	p = ch->clans;
                
        while ( get_next_clan (&p, clan_name, &clan_flags) ) {
                if ( (clan_def = get_clandef (clan_name)) )
                	strcpy (clan_name, clan_def->literal);
		if ( !clan_def )
			continue;
		if ( !clan_def->zone )
			continue;
		if ( !zone_table [clan_def->zone].jailer )
			continue;
		for ( d = descriptor_list; d; d = d->next ) {
			if ( !d->character || d->connected != CON_PLYNG )
				continue;
			if ( !d->character->room )
				continue;
			if ( is_area_enforcer (d->character) )
				continue;
			if ( is_in_cell (d->character, clan_def->zone) )
				prisoners++;
		}
	}

	if ( prisoners > 0 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "\n#3There %s currently %d PC prisoner%s awaiting your attention.\n", prisoners != 1 ? "are" : "is", prisoners, prisoners != 1 ? "s" : "");
		send_to_char (buf, ch);
	}
	return;
}


void do_promote (CHAR_DATA *ch, char *argument, int cmd)
{
	int			clan_flags = 0;
	CHAR_DATA	*tch = NULL;
	CLAN_DATA	*clan = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};
	int			flags = 0;
	char		*temp_arg = NULL;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Promote whom?\n\r", ch);
		return;
	}

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("Nobody is here by that name.\n", ch);
		return;
	}

	if ( tch == ch ) {
		send_to_char ("Sorry - unfortunately, promotions just don't work that way...\n", ch);
		return;
	}

	argument = one_argument (argument, clan_name);

	if ( !*clan_name ) {
		send_to_char ("Which clan did you wish to make the promotion in?\n", ch);
		return;
	}

	if ( !get_clan_long_short (ch, clan_name, &clan_flags) ) {
		send_to_char ("You are not a member of such a clan.\n", ch);
		return;
	}

	if ( !IS_SET (clan_flags, CLAN_LEADER) && (clan_flags < CLAN_SERGEANT || clan_flags > CLAN_COMMANDER) && !IS_SET (clan_flags, CLAN_MASTER) ) {
		send_to_char ("You are not a leader of that clan.\n", ch);
		return;
	}

		/* Keep clan_name as the short name */

	if ( (clan = get_clandef_long (clan_name)) )
		strcpy (clan_name, clan->name);

	argument = one_argument (argument, buf);

	if ( !(flags = clan_flags_to_value (buf)) ) {
		send_to_char ("I don't recognize the specified rank.\n", ch);
		return;
	}

	if ( clan_flags >= CLAN_SERGEANT && clan_flags <= CLAN_COMMANDER && (flags < CLAN_RECRUIT || flags > CLAN_COMMANDER) ) {
		send_to_char ("You'll need to specify a military rank, e.g. Recruit, Private, etc.\n", ch);
		return;
	}
	
	else if ( clan_flags >= CLAN_APPRENTICE && clan_flags <= CLAN_MASTER && (flags < CLAN_APPRENTICE || flags > CLAN_MASTER) ) {
		send_to_char ("You'll need to specify a guild rank, e.g. Apprentice, Journeyman, etc.\n", ch);
		return;
	}

	if ( flags >= clan_flags ) {
		send_to_char ("You do not have the authority to make this promotion.\n", ch);
		return;
	}

	temp_arg = get_clan_rank_name(flags);
	snprintf (buf, MAX_STRING_LENGTH,  "You promote $N to the rank of %s.", temp_arg);
	act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
	
	temp_arg = get_clan_rank_name(flags);
	snprintf (buf, MAX_STRING_LENGTH,  "$n has promoted you to the rank of %s.", temp_arg);
	
	act (buf, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
	act ("$n has promoted $N.", FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);

	add_clan (tch, clan_name, flags);
	
	return;
}

void do_invite (CHAR_DATA *ch, char *argument, int cmd)
{
	int			clan_flags = 0;
	char		*p = NULL;
	CHAR_DATA	*tch = NULL;
	CLAN_DATA	*clan = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Invite who?\n\r", ch);
		return;
	}

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("Nobody is here by that name.\n", ch);
		return;
	}

	if ( tch == ch ) {
		send_to_char ("You can't invite yourself.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {

		p = ch->clans;

		if ( !get_next_leader (&p, clan_name, &clan_flags) ) {
			send_to_char ("You are not an initiated leader of any clan.\n", ch);
			return;
		}

		if ( (clan = get_clandef (clan_name)) ) {
			send_to_char ("That clan could not be found in the database.\n", ch);
			return;
		}

		strcpy (buf, clan->name);
	}

	else {
		if ( !get_clan_long_short (ch, buf, &clan_flags) ) {
			send_to_char ("You are not a member of such a clan.\n", ch);
			return;
		}

		if ( !IS_SET (clan_flags, CLAN_LEADER) && (clan_flags < CLAN_CORPORAL || clan_flags > CLAN_COMMANDER) && !IS_SET (clan_flags, CLAN_MASTER) ) {
			send_to_char ("You are not a leader of that clan.\n", ch);
			return;
		}

			/* Keep clan_name as the short name */

		if ( (clan = get_clandef_long (buf)) )
			strcpy (buf, clan->name);
	}

	if ( get_clan (tch, buf, &clan_flags) ) {
		act ("$N is already a clan member.", FALSE, ch, 0, tch, TO_CHAR);
		return;
	}

	if ( tch->delay || tch->fighting ) {
		act ("$N is busy right now.", FALSE, ch, 0, tch, TO_CHAR);
		return;
	}

	if ( !AWAKE (tch) ) {
		act ("$N isn't conscious right now.", FALSE, ch, 0, tch, TO_CHAR);
		return;
	}

	clan = get_clandef (buf);

	tch->delay = IS_NPC (tch) && !tch->desc ? 3 : 120;
	tch->delay_type = DEL_INVITE;
	tch->delay_ch = ch;
	tch->delay_who = str_dup (buf);

	snprintf (buf, MAX_STRING_LENGTH,  "You invite $N to join %s.",
				  clan ? clan->literal : clan_name);

	act (buf, FALSE, ch, 0, tch, TO_CHAR);

	snprintf (buf, MAX_STRING_LENGTH,  "$N invites you to join %s.",
				  clan ? clan->literal : clan_name);

	act (buf, FALSE, tch, 0, ch, TO_CHAR);

    act ("$N whispers something to $n about joining a clan.",
						FALSE, tch, 0, ch, TO_NOTVICT | TO_ACT_FORMAT);

}

void invite_accept (CHAR_DATA *ch, char *argument)
{
	CLAN_DATA	*clan;
	char		buf [MAX_STRING_LENGTH];
	char		clan_name [MAX_STRING_LENGTH];

	if ( !ch->delay_who || !*ch->delay_who )
		return;

	ch->delay = 0;

	strcpy (clan_name, ch->delay_who);
	mem_free (ch->delay_who);
	ch->delay_who = NULL;

	if ( !is_he_here (ch, ch->delay_ch, 1) ) {
		send_to_char ("Too late.\n", ch);
		return;
	}

	add_clan (ch, clan_name, CLAN_MEMBER);

	act ("$N accepts your invitation.", FALSE, ch->delay_ch, 0, ch, TO_CHAR);

	clan = get_clandef (clan_name);

	snprintf (buf, MAX_STRING_LENGTH,  "You have been initiated into %s.",
				  clan ? clan->literal : clan_name);

	act (buf, FALSE, ch, 0, 0, TO_CHAR);
}

void do_recruit (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*pal = NULL;
	char		buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		for ( pal = ch->room->people; pal; pal = pal->next_in_room ) {
			if ( IS_NPC (pal) &&
				 pal->following != ch &&
				 is_leader (ch, pal) )
				break;
		}

		if ( !pal ) {
			send_to_char ("Nobody else here will follow you.\n\r", ch);
			return;
		}

		pal->following = ch;

		act ("$N nods to you to follow.", FALSE, pal, 0, ch, TO_CHAR | TO_ACT_FORMAT);
		act ("$N motions to $S clanmates.", FALSE, pal, 0, ch, TO_NOTVICT | TO_ACT_FORMAT);
		act ("$n falls in.", FALSE, pal, 0, ch, TO_ROOM | TO_ACT_FORMAT);
		return;
	}

	if ( !str_cmp (buf, "all") ) {
		act ("$n motions to $s clanmates.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		for ( pal = ch->room->people; pal; pal = pal->next_in_room ) {
			if ( IS_NPC (pal) &&
				 pal->following != ch &&
				 is_leader (ch, pal) ) {
				pal->following = ch;
				act ("$N nods to you to follow.", FALSE, pal, 0, ch, TO_CHAR | TO_ACT_FORMAT);
				act ("$n falls in.", FALSE, pal, 0, ch, TO_ROOM | TO_ACT_FORMAT);
			}
		}

		if ( !pal ) {
			send_to_char ("Nobody else here will follow you.\n\r", ch);
			return;
		}

		return;
	}

	if ( !(pal = get_char_room_vis (ch, buf)) ) {
		send_to_char ("Nobody is here by that name.\n\r", ch);
		return;
	}

	if ( pal == ch ) {
		send_to_char ("Not yourself.\n\r", ch);
		return;
	}

	if ( !is_leader (ch, pal) ) {
		act ("You don't have the authority to recruit $N.", FALSE, ch, 0, pal, TO_CHAR);
		return;
	}

	if ( pal->following == ch ) {
		act ("$N is already following you.", FALSE, ch, 0, pal, TO_CHAR);
		return;
	}

	pal->following = ch;

	act ("$N motions to $S clanmates.", FALSE, pal, 0, ch, TO_NOTVICT | TO_ACT_FORMAT);
	act ("$n falls in.", FALSE, pal, 0, ch, TO_VICT | TO_ACT_FORMAT);
	act ("$N nods to you to follow.", FALSE, pal, 0, ch, TO_CHAR);
	return;
}

void do_disband (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*pal = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf || !str_cmp (buf, "all") ) {

		act ("$n motions to $s clanmates.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		act ("You motion to your clanmates.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

		for ( pal = ch->room->people; pal; pal = pal->next_in_room ) {

			if ( pal->following != ch || !IS_NPC (pal) )
				continue;

			if ( is_leader (ch, pal) && AWAKE (pal) ) {
				act ("$N motions to you to stop following.",
						FALSE, pal, 0, ch, TO_CHAR | TO_ACT_FORMAT);
				act ("$n falls out of step.", FALSE, pal, 0, ch, TO_ROOM | TO_ACT_FORMAT);
				pal->following = NULL;
			}
		}

		return;
	}

	if ( !(pal = get_char_room_vis (ch, buf)) ) {
		send_to_char ("Nobody is here by that name.\n\r", ch);
		return;
	}

	if ( pal->following != ch ) {
		act ("$N is not following you.", FALSE, ch, 0, pal, TO_CHAR);
		return;
	}

	if ( !is_leader (ch, pal) ) {
		act ("You can't give $N orders.", FALSE, ch, 0, pal, TO_CHAR);
		return;
	}

	if ( !IS_NPC (pal) ) {
		send_to_char ("This command does not work on PCs.\n", ch);
		return;
	}

	pal->following = NULL;

	act ("You motion to $N.", FALSE, ch, 0, pal, TO_CHAR | TO_ACT_FORMAT);
	act ("$N motions to $n.", FALSE, pal, 0, ch, TO_NOTVICT | TO_ACT_FORMAT);
	act ("$N motions to you to stop following.", FALSE, pal, 0, ch, TO_CHAR);
	act ("$n falls out of step.", FALSE, pal, 0, ch, TO_ROOM | TO_ACT_FORMAT);
}

void do_castout (CHAR_DATA *ch, char *argument, int cmd)
{
	int			clan_flags = 0;
	int			clan_flags2 = 0;
	CHAR_DATA	*victim = NULL;
	CLAN_DATA	*clan = NULL;
	char		*p;
	char		clan_name [80];
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Castout whom?\n\r", ch);
		return;
	}

	if ( !(victim = get_char_room_vis (ch, buf)) ) {
		send_to_char ("Nobody is here by that name.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	p = ch->clans;

	if ( !*buf ) {
		if ( !get_next_leader (&p, clan_name, &clan_flags) ) {
			send_to_char ("You are not a leader in that clan.  You can't cast out anybody.\n", ch);
			return;
		}

		clan = get_clandef(clan_name);
		if ( !clan ) {
			send_to_char ("No such clan, I am afraid.\n", ch);
			return;
		}
		strcpy (clan_name, clan->name);
	}

	else if ( get_clan_long_short (ch, buf, &clan_flags) ) {
		if ( !IS_SET (clan_flags, CLAN_LEADER) && (clan_flags < CLAN_SERGEANT || clan_flags > CLAN_COMMANDER) && !IS_SET (clan_flags, CLAN_MASTER) ) {
			send_to_char ("You are not a leader in that clan.  You cannot cast anyone out.\n", ch);
			return;
		}

		clan = get_clandef_long(buf);
		if ( !clan ) {
			send_to_char ("No such clan, I am afraid.\n", ch);
			return;
		}
		strcpy (clan_name, clan->name);
	}

	else {
		send_to_char ("You are not a member of such a clan.\n", ch);
		return;
	}

	if ( !get_clan (victim, clan_name, &clan_flags2) ) {
		act ("$N is not part of your clan.\n", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	if ( IS_SET (clan_flags, CLAN_LEADER) || clan_flags <= clan_flags2 ) {
		act ("You are not of sufficient rank to cast $N from the clan.\n",
				FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	remove_clan (victim, clan_name);

	snprintf (buf, MAX_STRING_LENGTH,  "$N is no longer a part of %s.", clan->literal);
	act (buf, FALSE, ch, 0, victim, TO_CHAR);
	snprintf (buf, MAX_STRING_LENGTH,  "$n has stripped you of your clan membership in %s.", clan->literal);
	act (buf, FALSE, ch, 0, victim, TO_VICT | TO_ACT_FORMAT);
	snprintf (buf, MAX_STRING_LENGTH,  "$n has stripped $N of membership in %s.", clan->literal);
	act ("$n has stripped $N of clan membership.", FALSE, ch, 0, victim, TO_NOTVICT | TO_ACT_FORMAT);
}

void do_initiate (CHAR_DATA *ch, char *argument, int cmd)
{
	char		arg1 [MAX_INPUT_LENGTH];
	char		arg2 [MAX_INPUT_LENGTH];
	char		buf [MAX_STRING_LENGTH];
	char		*target_name;
	CHAR_DATA	*target;
	int			to_circle;
	int			circle_levs [] = { 0, 36, 42, 48, 52, 57, 60, 65, 70, 100 };
	/** skill in Ritual must meet circle_levs[to_circle] to be promoted to that circle **/

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if ( !ch->deity ) {
		send_to_char ("Be careful you don't attract the wrath of a god.\n\r",
					  ch);
		return;
	}

	if ( strtol(arg1, NULL, 10)) {
		to_circle = strtol(arg1, NULL, 10);
		target_name = arg2;
	} 
	else if ( strtol(arg2, NULL, 10)) {
		to_circle = strtol(arg2, NULL, 10);
		target_name = arg1;
	} 
	else {
		send_to_char ("You must follow the exact time honored rituals when "
					  "initiating a brother\n\ror sister.\n\r", ch);
		return;
	}

	if ( to_circle < 1 || to_circle > 9 ) {
		send_to_char ("$g doesn't recognize that circle.\n\r", ch);
		return;
	}

	if ( !(target = get_char_room_vis (ch, target_name)) ) {
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if ( to_circle >= ch->circle ) {
		send_to_char ("You cannot initiate at or above your circle.\n\r", ch);
		return;
	}

	if ( target->deity != ch->deity ) {
		act ("Your lesson is lost on $N.", FALSE, ch, 0, target, TO_CHAR);
		return;
	}
	
	if ( target->circle + 1 < to_circle ) {
		act ("$N is not ready for that circle.\n\r",
				FALSE, ch, 0, target, TO_CHAR);
		return;
	} else if ( target->circle + 1 > to_circle ) {
		act ("$N is beyond those teachings.",
				FALSE, ch, 0, target, TO_CHAR);
		return;
	}

	if ( target->skills [SKILL_RITUAL] < circle_levs [to_circle] ) {
		act ("$N cannot be initiated to that circle.",
				FALSE, ch, 0, target, TO_CHAR);
		return;
	}

    target->circle++;

	snprintf (buf, MAX_STRING_LENGTH,  "You quietly instruct $N in the secrets "
				  "of the %d%s circle of $G.", target->circle,
		target->circle == 1 ? "st" :
		target->circle == 2 ? "nd" :
		target->circle == 3 ? "rd" : "th");
	act (buf, FALSE, ch, 0, target, TO_CHAR);

	snprintf (buf, MAX_STRING_LENGTH,  "$N quietly instructs you in the secrets "
				  "of the %d%s circle of $g.", target->circle,
		target->circle == 1 ? "st" :
		target->circle == 2 ? "nd" :
		target->circle == 3 ? "rd" : "th");
	act (buf, FALSE, target, 0, ch, TO_CHAR);
	
	return;
}

void add_clandef (char *argument)
{
	CLAN_DATA	*clan;
	char		buf [MAX_STRING_LENGTH];

	clan = (CLAN_DATA *)alloc (sizeof (CLAN_DATA), 39);

	argument = one_argument (argument, buf);
	clan->name = str_dup (buf);

	argument = one_argument (argument, buf);
	clan->zone = strtol(buf, NULL, 10);

	argument = one_argument (argument, buf);
	clan->literal = str_dup (buf);

	argument = one_argument (argument, buf);
	clan->member_vnum = strtol(buf, NULL, 10);

	argument = one_argument (argument, buf);
	clan->leader_vnum = strtol(buf, NULL, 10);

	argument = one_argument (argument, buf);
	clan->omni_vnum = strtol(buf, NULL, 10);

	clan->next = clan_list;
	clan_list = clan;
}

void write_clan_registry (CHAR_DATA *ch)
{
	FILE				*fp_dr;
	CLAN_DATA			*clan;
	NAME_SWITCH_DATA	*nsp;

	if ( !(fp_dr = fopen (CLAN_REGISTRY ".new", "w")) ) {
		perror (CLAN_REGISTRY ".new");
		system_log("Unable to write clan registry!", TRUE);

		if ( ch )
			send_to_char ("Unable to write clan registry!", ch);
	}

	for ( nsp = clan_name_switch_list; nsp; nsp = nsp->next )
		fprintf (fp_dr, "newclanname %s %s\n",
						nsp->old_name,
						nsp->new_name);

	for ( clan = clan_list; clan; clan = clan->next )
		fprintf (fp_dr, "clandef %s %d \"%s\" %d %d %d\n",
						clan->name,
						clan->zone,
						clan->literal,
						clan->member_vnum,
						clan->leader_vnum,
						clan->omni_vnum);

	fclose (fp_dr);

	system ("mv " CLAN_REGISTRY " " CLAN_REGISTRY ".old");
	system ("mv " CLAN_REGISTRY ".new " CLAN_REGISTRY);

	if ( ch )
		send_to_char ("Clan registry updated.\n", ch);
}

void do_clan (CHAR_DATA *ch, char *argument, int cmd)
{
	int					zone;
	int					leader_obj_vnum;
	int					member_obj_vnum;
	int					new_flags;
	int					clan_flags;
	int					num_clans = 0;
	int					i;
	NAME_SWITCH_DATA	*name_switch;
	NAME_SWITCH_DATA	*nsp;
	OBJ_DATA			*obj;
	CLAN_DATA			*clan;
	CLAN_DATA			*delete_clan;
	CHAR_DATA			*edit_mob = NULL;
	CHAR_DATA			*tch;
	char				*p;
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				clan_name [MAX_STRING_LENGTH] = {'\0'};
	char				oldname [MAX_STRING_LENGTH] = {'\0'};
	char				newname [MAX_STRING_LENGTH] = {'\0'};
	char				literal [MAX_STRING_LENGTH] = {'\0'};
	char				name [MAX_STRING_LENGTH] = {'\0'};
	char				*the_clans [MAX_CLANS] = {'\0'};

	if ( IS_NPC (ch) ) {
		send_to_char ("Sorry, but you can't do this while switched...\n", ch);
		return;
	}

	if ( !*argument ) {

		send_to_char ("Old clan name   ->  New clan name\n", ch);
		send_to_char ("=============       =============\n", ch);

		if ( !clan_name_switch_list )
			send_to_char ("No clans set to be renamed.\n", ch);

		else {
			for ( nsp = clan_name_switch_list; nsp; nsp = nsp->next ) {
				snprintf (buf, MAX_STRING_LENGTH,  "%-15s     %-15s\n", nsp->old_name, nsp->new_name);
				send_to_char (buf, ch);
			}
		}

		send_to_char ("\nClan Name        Full Clan Name\n", ch);
		send_to_char ("===============  =================================\n",
					  ch);

		for ( clan = clan_list; clan; clan = clan->next ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%-15s  %s\n", clan->name, clan->literal);
			send_to_char (buf, ch);

			if ( clan->zone ) {
				snprintf (buf, MAX_STRING_LENGTH,  "                 Enforcement Zone %d\n",
							  clan->zone);
				send_to_char (buf, ch);
			}

			if ( clan->member_vnum ) {
				obj = vtoo (clan->member_vnum);
				snprintf (buf, MAX_STRING_LENGTH,  "                 Member Object (%05d):  %s\n",
					clan->member_vnum,
					obj ? obj->short_description : "UNDEFINED");
				send_to_char (buf, ch);
			}

			if ( clan->leader_vnum ) {
				obj = vtoo (clan->leader_vnum);
				snprintf (buf, MAX_STRING_LENGTH,  "                 Leader Object (%05d):  %s\n",
					clan->leader_vnum,
					obj ? obj->short_description : "UNDEFINED");
				send_to_char (buf, ch);
			}
		}

		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {
		send_to_char ("Syntax for modifying PCs:\n\n", ch);
		send_to_char ("   clan set <clan-name> [<clan-flags>]\n", ch);
		send_to_char ("   clan remove <clan-name>         (or all)\n", ch);
        send_to_char ("\nSyntax for modifying clan definitions:\n", ch);
		send_to_char ("   clan rename <oldclanname> <newclanname>\n", ch);
		send_to_char ("   clan unrename <oldclanname> <newclanname>\n", ch);
		send_to_char ("   clan register <name> <enforcement zone> <long name> "
					  "[<leader obj>] [<member obj>]\n", ch);
		send_to_char ("   clan unregister <name>\n", ch);
		send_to_char ("   clan list\n", ch);
		send_to_char ("\nThe obj vnums are optional.  Specify zone 0 if no "
					  "enforcement zone.\n\nExamples:\n", ch);
		send_to_char ("  > clan set tashalwatch member leader\n", ch);
		send_to_char ("  > clan remove tashalwatch\n", ch);
		send_to_char ("  > clan register TashalWatch 10 'Tashal Watch'\n", ch);
		send_to_char ("  > clan unregister tashalwatch\n", ch);
		send_to_char ("  > clan rename 10 TashalWatch\n", ch);
		send_to_char ("  > clan unrename 10 TashalWatch\n", ch);
		send_to_char ("  > clan list\n", ch);
		return;
	}

	if ( !str_cmp (buf, "list") ) {

		for ( tch = character_list; tch; tch = tch->next ) {

			if ( tch->deleted )
				continue;

			p = tch->clans;

			if ( !p || !*p )
				continue;

			while ( get_next_clan (&p, clan_name, &clan_flags) ) {

				if ( num_clans > MAX_CLANS )		/* Too many in list */
					break;

				for ( i = 0; i < num_clans; i++ )
					if ( !str_cmp (clan_name, the_clans [i]) )
						break;

				if ( i >= num_clans )
					the_clans [num_clans++] = str_dup (clan_name);
			}

		}

		*b_buf = '\0';

		for ( i = 0; i < num_clans; i++ ) {

			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%-15s", the_clans [i]);

			if ( i % 5 == 4 )
				strcat (b_buf, "\n");
			else
				strcat (b_buf, " ");
		}

		page_string (ch->desc, b_buf);

		return;
	}

	if ( !str_cmp (buf, "set") ||
		 !str_cmp (buf, "remove") ||
		 !str_cmp (buf, "delete") ) {

		if ( (!ch->pc->edit_mob && !ch->pc->edit_player) ||
			(!(edit_mob = vtom (ch->pc->edit_mob)) &&
			!(edit_mob = ch->pc->edit_player)) ) {
			send_to_char ("Start by using the MOBILE command.\n", ch);
			return;
		}
	}

	if ( !str_cmp (buf, "set") ) {

		argument = one_argument (argument, clan_name);

		if ( !*clan_name ) {
			send_to_char ("Expected a clan name after 'add'.\n", ch);
			return;
		}

		new_flags = clan_flags_to_value (argument);

		if ( get_clan (edit_mob, clan_name, &clan_flags) )
			remove_clan (edit_mob, clan_name);

		clan_flags = 0;

		while ( 1 ) {

			argument = one_argument (argument, buf);

			if ( !*buf )
				break;

			if ( !str_cmp (buf, "member") )
				TOGGLE (clan_flags, CLAN_MEMBER)		/* {}'s define     */
			else if ( !str_cmp (buf, "leader") )        /* so no ; needed  */
				TOGGLE (clan_flags, CLAN_LEADER)
			else if ( !str_cmp (buf, "recruit") )
				TOGGLE (clan_flags, CLAN_RECRUIT)
			else if ( !str_cmp (buf, "private") )
				TOGGLE (clan_flags, CLAN_PRIVATE)
			else if ( !str_cmp (buf, "corporal") )
				TOGGLE (clan_flags, CLAN_CORPORAL)
			else if ( !str_cmp (buf, "sergeant") )
				TOGGLE (clan_flags, CLAN_SERGEANT)
			else if ( !str_cmp (buf, "lieutenant") )
				TOGGLE (clan_flags, CLAN_LIEUTENANT)
			else if ( !str_cmp (buf, "captain") )
				TOGGLE (clan_flags, CLAN_CAPTAIN)
			else if ( !str_cmp (buf, "general") )
				TOGGLE (clan_flags, CLAN_GENERAL)
			else if ( !str_cmp (buf, "commander") )
				TOGGLE (clan_flags, CLAN_COMMANDER)
			else if ( !str_cmp (buf, "apprentice") )
				TOGGLE (clan_flags, CLAN_APPRENTICE)
			else if ( !str_cmp (buf, "journeyman") )
				TOGGLE (clan_flags, CLAN_JOURNEYMAN)
			else if ( !str_cmp (buf, "master") )
				TOGGLE (clan_flags, CLAN_MASTER)
			else {
				snprintf (literal, MAX_STRING_LENGTH, "Flag %s is unknown for clans.\n", buf);
				send_to_char (literal, ch);
			}
		}
	
		if ( !clan_flags )
			clan_flags = CLAN_MEMBER;

		add_clan (edit_mob, clan_name, clan_flags);

		if ( edit_mob->mob ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%d mobile(s) in world redefined.\n",
						  redefine_mobiles (edit_mob));
			send_to_char (buf, ch);
		}

		return;		/* Return from n/pc specific uses of clan */
	}

	else if ( !str_cmp (buf, "remove") ||
			  !str_cmp (buf, "delete") ) {

		argument = one_argument (argument, clan_name);

		if ( !*clan_name ) {
			send_to_char ("Expected a clan name to remove from n/pc.\n", ch);
			return;
		}

		if ( !str_cmp (clan_name, "all") ) {
			mem_free (ch->clans);
			ch->clans = str_dup ("");
		}

		else if ( !get_clan (edit_mob, clan_name, &clan_flags) ) {
			send_to_char ("N/PC doesn't have that clan.\n", ch);
			return;
		}

		remove_clan (edit_mob, clan_name);

		return;		/* Return from n/pc specific uses of clan */
	}

	else if ( !str_cmp (buf, "unregister") ) {

		argument = one_argument (argument, clan_name);

		if ( !clan_list ) {
			send_to_char ("There are no registered clans.\n", ch);
			return;
		}

		if ( !str_cmp (clan_list->name, clan_name) ) {
			delete_clan = clan_list;
			clan_list = clan_list->next;
		}

		else {
			delete_clan = NULL;

			for ( clan = clan_list; clan->next; clan = clan->next ) {
				if ( !str_cmp (clan->next->name, clan_name) ) {
					delete_clan = clan->next;
					clan->next = delete_clan->next;
					break;
				}
			}

			if ( !delete_clan ) {
				send_to_char ("No such registered clan name.\n", ch);
				return;
			}
		}

		mem_free (delete_clan->name);
		mem_free (delete_clan->literal);
		mem_free (delete_clan);
	}

	else if ( !str_cmp (buf, "rename") ) {

		argument = one_argument (argument, oldname);
		argument = one_argument (argument, newname);

		if ( !*oldname || !*newname ) {
			send_to_char ("rename <oldclanname> <newclanname>\n", ch);
			return;
		}

		name_switch = (struct name_switch_data *)alloc (sizeof (struct name_switch_data),0);
		name_switch->old_name = str_dup (oldname);
		name_switch->new_name = str_dup (newname);

		if ( !clan_name_switch_list )
			clan_name_switch_list = name_switch;
		else {
			for ( nsp = clan_name_switch_list; nsp->next; nsp = nsp->next )
				;
			nsp->next = name_switch;
		}
	}

	else if ( !str_cmp (buf, "unrename") ) {
		argument = one_argument (argument, oldname);
		argument = one_argument (argument, newname);

		if ( !*oldname || !*newname ) {
			send_to_char ("clan unrename <oldclanname> <newclanname>\n", ch);
			send_to_char ("This command deletes a rename entry.\n", ch);
			return;
		}

		if ( !str_cmp (clan_name_switch_list->old_name, oldname) &&
			 !str_cmp (clan_name_switch_list->new_name, newname) ) {
			mem_free (clan_name_switch_list->old_name);
			mem_free (clan_name_switch_list->new_name);
			nsp = clan_name_switch_list;
			clan_name_switch_list = nsp->next;
			mem_free (nsp);
			send_to_char ("Rename entry deleted.\n", ch);
			write_clan_registry (ch);
			return;
		}

		for ( nsp = clan_name_switch_list; nsp->next; nsp = nsp->next )
			if ( !str_cmp (nsp->next->old_name, oldname) &&
				 !str_cmp (nsp->next->new_name, newname) )
				break;
		
		if ( !nsp->next ) {
			send_to_char ("Sorry, no such rename entry pair found.\n", ch);
			return;
		}

		name_switch = nsp->next;
		nsp->next = name_switch->next;

		mem_free (name_switch->old_name);
		mem_free (name_switch->new_name);
		mem_free (name_switch);

		send_to_char ("Rename entry deleted.\n", ch);
	}
	
	else if ( !str_cmp (buf, "register") ) {

		argument = one_argument (argument, name);

		if ( get_clandef (name) ) {
			send_to_char ("That clan has already been registered.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);

		if ( !*name || !*buf || !isdigit (*buf) ) {
			send_to_char ("Type clan ? for syntax.\n", ch);
			return;
		}

		zone = strtol(buf, NULL, 10);

		argument = one_argument (argument, literal);

		if ( !*literal ) {
			send_to_char ("Syntax error parsing literal, type clan ? for "
						  "syntax.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);
		
		if ( !*buf )
			leader_obj_vnum = 0;
		else if ( !isdigit (*buf) ) {
			send_to_char ("Syntax error.  Did you enclose the long name in "
						  "quotes?  Type clan ?.\n", ch);
			return;
		} else {
			leader_obj_vnum = strtol(buf, NULL, 10);
			if ( leader_obj_vnum && !vtoo (leader_obj_vnum) )
				send_to_char ("NOTE:  Leader object doesn't currently "
							  "exist.\n", ch);
		}

		argument = one_argument (argument, buf);

		if ( !*buf )
			member_obj_vnum = 0;
		else if ( !isdigit (*buf) ) {
			send_to_char ("Syntax error parsing member object.  Type clan ? "
						  "for syntax.\n", ch);
			return;
		} else {
			member_obj_vnum = strtol(buf, NULL, 10);
			if ( member_obj_vnum && !vtoo (member_obj_vnum) )
				send_to_char ("NOTE:  Member object doesn't currently "
							  "exist.\n", ch);
		}

		clan = (CLAN_DATA *)alloc (sizeof (CLAN_DATA), 39);

		clan->name = str_dup (name);
		clan->zone = zone;
		clan->literal = str_dup (literal);
		clan->member_vnum = member_obj_vnum;
		clan->leader_vnum = leader_obj_vnum;
		clan->omni_vnum = 0;

		clan->next = clan_list;
		clan_list = clan;
	}

	else {
		send_to_char ("What do you want to do again?  Type clan ? for help.\n",
					  ch);
		return;
	}

	write_clan_registry (ch);
}

int clan_flags_to_value (char *flag_names)
{
	int			flags = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	while ( 1 ) {
		flag_names = one_argument (flag_names, buf);

		if ( !*buf )
			break;

		if ( !str_cmp (buf, "member") )
			SET_BIT (flags, CLAN_MEMBER);
		else if ( !str_cmp (buf, "leader") )
			SET_BIT (flags, CLAN_LEADER);
		else if ( !str_cmp (buf, "memberobj") )
			SET_BIT (flags, CLAN_MEMBER_OBJ);
		else if ( !str_cmp (buf, "leaderobj") )
			SET_BIT (flags, CLAN_LEADER_OBJ);
		else if ( !str_cmp (buf, "recruit") )
			SET_BIT (flags, CLAN_RECRUIT);
		else if ( !str_cmp (buf, "private") )
			SET_BIT (flags, CLAN_PRIVATE);
		else if ( !str_cmp (buf, "corporal") )
			SET_BIT (flags, CLAN_CORPORAL);
		else if ( !str_cmp (buf, "sergeant") )
			SET_BIT (flags, CLAN_SERGEANT);
		else if ( !str_cmp (buf, "lieutenant") )
			SET_BIT (flags, CLAN_LIEUTENANT);
		else if ( !str_cmp (buf, "captain") )
			SET_BIT (flags, CLAN_CAPTAIN);
		else if ( !str_cmp (buf, "general") )
			SET_BIT (flags, CLAN_GENERAL);
		else if ( !str_cmp (buf, "commander") )
			SET_BIT (flags, CLAN_COMMANDER);
		else if ( !str_cmp (buf, "apprentice") )
			SET_BIT (flags, CLAN_APPRENTICE);
		else if ( !str_cmp (buf, "journeyman") )
			SET_BIT (flags, CLAN_JOURNEYMAN);
		else if ( !str_cmp (buf, "master") )
			SET_BIT (flags, CLAN_MASTER);
	}

	return flags;
}

char *value_to_clan_flags (int flags)
{
	if ( IS_SET (flags, CLAN_LEADER) )
		return "leader ";

	else if ( IS_SET (flags, CLAN_MEMBER) )
		return  "member ";

	else if ( IS_SET (flags, CLAN_RECRUIT) )
		return  "recruit ";

	else if ( IS_SET (flags, CLAN_PRIVATE) )
		return  "private ";

	else if ( IS_SET (flags, CLAN_CORPORAL) )
		return  "corporal ";

	else if ( IS_SET (flags, CLAN_SERGEANT) )
		return  "sergeant ";

	else if ( IS_SET (flags, CLAN_LIEUTENANT) )
		return  "lieutenant ";

	else if ( IS_SET (flags, CLAN_CAPTAIN) )
		return  "captain ";

	else if ( IS_SET (flags, CLAN_COMMANDER) )
		return  "commander ";

	else if ( IS_SET (flags, CLAN_APPRENTICE) )
		return  "apprentice ";

	else if ( IS_SET (flags, CLAN_JOURNEYMAN) )
		return  "journeyman ";

	else if ( IS_SET (flags, CLAN_MASTER) )
		return  "master ";

	else if ( IS_SET (flags, CLAN_LEADER_OBJ) )
		return  "leaderobj ";

	else if ( IS_SET (flags, CLAN_MEMBER_OBJ) )
		return  "memberobj ";
		
	else
		return "member ";

	
	
}

char *remove_clan_from_string (char *string, char *old_clan_name)
{
	char		*argument;
	static char	buf [MAX_STRING_LENGTH] = {'\0'};
	char		clan_flags [MAX_STRING_LENGTH] = {'\0'};
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};

	if ( !*string )
		return NULL;

	argument = string;

	*buf = '\0';

	while ( 1 ) {
		argument = one_argument (argument, clan_flags);
		argument = one_argument (argument, clan_name);

		if ( !*clan_name )
			break;

		if ( str_cmp (clan_name, old_clan_name) )
			snprintf (ADDBUF, MAX_STRING_LENGTH, "'%s' %s ", clan_flags, clan_name);
	}

	mem_free (string);
	string = NULL;

	if ( *buf && buf [strlen (buf) - 1] == ' ' )
		buf [strlen (buf) - 1] = '\0';

	return buf;
}

char *add_clan_to_string (char *string, char *new_clan_name, int clan_flags)
{
	char		*argument = NULL;
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};
	char		flag_names [MAX_STRING_LENGTH] = {'\0'};
	static char	buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	argument = string;

			/* Look to see if we're just changing flags */

	while ( 1 ) {

		argument = one_argument (argument, flag_names);
		argument = one_argument (argument, clan_name);

		if ( !*clan_name )
			break;

		if ( !str_cmp (clan_name, new_clan_name) ) {
			(void)remove_clan_from_string (string, new_clan_name);
			break;
		}
	}

		/* Clan is new */

	if ( string && *string ){
		temp_arg = value_to_clan_flags (clan_flags);
		snprintf (buf, MAX_STRING_LENGTH,  "'%s' %s %s",
					temp_arg,
					new_clan_name,
					string);
	}
	else{
		temp_arg = value_to_clan_flags (clan_flags);
		snprintf (buf, MAX_STRING_LENGTH,  "'%s' %s",
					temp_arg,
					new_clan_name);	
	}
	
	while ( buf [strlen (buf) - 1] == ' ' )
		buf [strlen (buf) - 1] = '\0';

	if ( string && *string ) {
		mem_free (string);
		string = NULL;
	}

	return buf;
}

void remove_clan (CHAR_DATA *ch, char *old_clan_name)
{
	char		*argument;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		clan_flags [MAX_STRING_LENGTH] = {'\0'};
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};

	if ( !*ch->clans )
		return;

	argument = ch->clans;

	*buf = '\0';

	while ( 1 ) {
		argument = one_argument (argument, clan_flags);
		argument = one_argument (argument, clan_name);

		if ( !*clan_name )
			break;

		if ( str_cmp (clan_name, old_clan_name) )
			snprintf (ADDBUF, MAX_STRING_LENGTH, "'%s' %s ", clan_flags, clan_name);
	}

	if ( *buf && buf [strlen (buf) - 1] == ' ' )
		buf [strlen (buf) - 1] = '\0';

	if ( ch->clans && *ch->clans != '\0' )
		mem_free (ch->clans);

	ch->clans = add_hash(buf);
}

void add_clan (CHAR_DATA *ch, char *new_clan_name, int clan_flags)
{
	char		*argument = NULL;
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};
	char		flag_names [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	argument = ch->clans;

			/* Look to see if we're just changing flags */

	while ( 1 ) {

		argument = one_argument (argument, flag_names);
		argument = one_argument (argument, clan_name);

		if ( !*clan_name )
			break;

		if ( !str_cmp (clan_name, new_clan_name) ) {
			remove_clan (ch, new_clan_name);
			break;
		}
	}

		/* Clan is new */
	temp_arg = value_to_clan_flags (clan_flags);
	snprintf (buf, MAX_STRING_LENGTH,  "'%s' %s %s",
				  temp_arg,
				  new_clan_name,
				  ch->clans);

	while ( buf [strlen (buf) - 1] == ' ' )
		buf [strlen (buf) - 1] = '\0';

	if ( ch->clans && *ch->clans != '\0' )
		mem_free (ch->clans);

	ch->clans = add_hash(buf);
}

int get_next_clan (char **p, char *clan_name, int *clan_flags)
{
	char		flag_names [MAX_STRING_LENGTH] = {'\0'};

	*p = one_argument (*p, flag_names);

	*clan_flags = clan_flags_to_value (flag_names);

	*p = one_argument (*p, clan_name);

	if ( !*clan_name )
		return 0;

	return 1;
}

char *get_shared_clan_rank (CHAR_DATA *ch, CHAR_DATA *observer)
{
	char		*argument, *argument2;
	char		clan_name [MAX_STRING_LENGTH] = {'\0'}, clan_name2[MAX_STRING_LENGTH] = {'\0'};
	int		flags = 0, flags2 = 0, highest_rank = 0;

	argument = observer->clans;

	while ( get_next_clan (&argument, clan_name, &flags) ) {
		argument2 = ch->clans;
		while ( get_next_clan (&argument2, clan_name2, &flags2) ) {
			if ( !str_cmp (clan_name, "osgi_citizens") ||
				!str_cmp (clan_name, "mm_denizens") )
				continue;
			if ( !str_cmp (clan_name, clan_name2) ) {
				if ( flags2 > highest_rank )
					highest_rank = flags2;
			}
		}
	}

	flags2 = highest_rank;

	if ( flags2 == CLAN_LEADER )
		return "Leadership";
	else if ( flags2 == CLAN_RECRUIT )
		return "Recruit";
	else if ( flags2 == CLAN_PRIVATE )
		return "Private";
	else if ( flags2 == CLAN_CORPORAL )
		return "Corporal";
	else if ( flags2 == CLAN_SERGEANT )
		return "Sergeant";
	else if ( flags2 == CLAN_LIEUTENANT )
		return "Lieutenant";
	else if  ( flags2 == CLAN_CAPTAIN )
		return "Captain";
	else if ( flags2 == CLAN_GENERAL )
		return "General";
	else if ( flags2 == CLAN_COMMANDER )
		return "Commander";
	else if ( flags2 == CLAN_APPRENTICE )
		return "Apprentice";
	else if ( flags2 == CLAN_JOURNEYMAN )
		return "Journeyman";
	else if ( flags2 == CLAN_MASTER )
		return "Master";
	else if ( flags2 > 0 )
		return "Membership";

	return NULL;
}

char *get_clan_rank_name (int flags)
{
	if ( flags == CLAN_LEADER )
		return "Leadership";
	else if ( flags == CLAN_RECRUIT )
		return "Recruit";
	else if ( flags == CLAN_PRIVATE )
		return "Private";
	else if ( flags == CLAN_CORPORAL )
		return "Corporal";
	else if ( flags == CLAN_SERGEANT )
		return "Sergeant";
	else if ( flags == CLAN_LIEUTENANT )
		return "Lieutenant";
	else if  ( flags == CLAN_CAPTAIN )
		return "Captain";
	else if ( flags == CLAN_GENERAL )
		return "General";
	else if ( flags == CLAN_COMMANDER )
		return "Commander";
	else if ( flags == CLAN_APPRENTICE )
		return "Apprentice";
	else if ( flags == CLAN_JOURNEYMAN )
		return "Journeyman";
	else if ( flags == CLAN_MASTER )
		return "Master";
	else if ( flags > 0 )
		return "Membership";

	return NULL;
}

int get_clan_in_string (char *string, char *clan, int *clan_flags)
{
	int			flags;
	char		*argument;
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};

	argument = string;

	while ( get_next_clan (&argument, clan_name, &flags) ) {

		if ( !str_cmp (clan_name, clan) ) {
			*clan_flags = flags;
			return 1;
		}
	}

	return 0;
}

int get_clan (CHAR_DATA *ch, char *clan, int *clan_flags)
{
	int			flags;
	char		*argument;
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};

	if ( !ch->clans )
		return 0;

	argument = ch->clans;

	while ( get_next_clan (&argument, clan_name, &flags) ) {

		if ( !str_cmp (clan_name, clan) ) {
			*clan_flags = flags;
			return 1;
		}
	}

	return 0;
}

int get_clan_long (CHAR_DATA *ch, char *clan_name, int *clan_flags)
{
	CLAN_DATA	*clan;

	if ( !(clan = get_clandef_long (clan_name)) )
		return 0;

	if ( !get_clan (ch, clan->name, clan_flags) )
		return 0;

	return 1;
}

int get_clan_long_short (CHAR_DATA *ch, char *clan_name, int *clan_flags)
{
	if ( get_clan_long (ch, clan_name, clan_flags) )
		return 1;

	return get_clan (ch, clan_name, clan_flags);
}

int get_next_leader (char **p, char *clan_name, int *clan_flags)
{
	char		flag_names [MAX_STRING_LENGTH] = {'\0'};

	while ( 1 ) {

		*p = one_argument (*p, flag_names);

		*clan_flags = clan_flags_to_value (flag_names);

		*p = one_argument (*p, clan_name);

		if ( !*clan_name )
			return 0;

		if ( IS_SET (*clan_flags, CLAN_LEADER) ||
			 IS_SET (*clan_flags, CLAN_LEADER_OBJ) ||
			(*clan_flags >= CLAN_SERGEANT && *clan_flags <= CLAN_COMMANDER) ||
			IS_SET (*clan_flags, CLAN_MASTER) )
			break;
	}

	return 1;
}

CLAN_DATA	*get_clandef (char *clan_name)
{
	CLAN_DATA	*clan;

	for ( clan = clan_list; clan; clan = clan->next )
		if ( !str_cmp (clan->name, clan_name) )
			return clan;

	return NULL;
}

CLAN_DATA *get_clandef_long (char *clan_long_name)
{
	CLAN_DATA	*clan;

	for ( clan = clan_list; clan; clan = clan->next )
		if ( !str_cmp (clan->literal, clan_long_name) )
			return clan;

	return NULL;
}

int is_area_enforcer (CHAR_DATA *ch)
{
	int			flags;
	char		*p;
	CLAN_DATA	*clan_def;
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};

	p = ch->clans;

	while ( get_next_clan (&p, clan_name, &flags) ) {

		if ( !(clan_def = get_clandef (clan_name)) )
			continue;

		if ( clan_def->zone != 0 && ch->room->zone == clan_def->zone )
			return 1;
	}

	return 0;
}

int is_area_leader (CHAR_DATA *ch)
{
	int			flags;
	char		*p;
	CLAN_DATA	*clan_def;
	char		clan_name [MAX_STRING_LENGTH] = {'\0'};

	p = ch->clans;

	while ( get_next_leader (&p, clan_name, &flags) ) {

		if ( !(clan_def = get_clandef (clan_name)) )
			continue;

		if ( clan_def->zone != 0 && ch->room->zone == clan_def->zone )
			return 1;
	}

	return 0;
}
void add_clan_id_string (char *string, char *clan_name, char *clan_flags)
{
	NAME_SWITCH_DATA	*nsp;
	char				*argument;
	char				buf [MAX_STRING_LENGTH];

		/* NOTE:  X->Y Y->Z Z->A   If Y, then Y gets renamed to Z then A. */

	for ( nsp = clan_name_switch_list; nsp; nsp = nsp->next )
		if ( !str_cmp (nsp->old_name, clan_name) )
			clan_name = nsp->new_name;

	argument = string;

	while ( 1 ) {

		argument = one_argument (argument, buf);	/* flags     */
		argument = one_argument (argument, buf);	/* clan name */

		if ( !*buf )
			break;

		if ( !str_cmp (buf, clan_name) )
			return;
	}

		/* string doesn't have that clan, add it */

	if ( string && *string )
		snprintf (buf, MAX_STRING_LENGTH,  "%s '%s' %s", string, clan_flags, clan_name);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "'%s' %s", clan_flags, clan_name);

	mem_free (string);

	string = str_dup (buf);
}

void add_clan_id (CHAR_DATA *ch, char *clan_name, char *clan_flags)
{
	NAME_SWITCH_DATA	*nsp;
	char				*argument;
	char				buf [MAX_STRING_LENGTH];

		/* NOTE:  X->Y Y->Z Z->A   If Y, then Y gets renamed to Z then A. */

	for ( nsp = clan_name_switch_list; nsp; nsp = nsp->next )
		if ( !str_cmp (nsp->old_name, clan_name) )
			clan_name = nsp->new_name;

	argument = ch->clans;

	while ( 1 ) {

		argument = one_argument (argument, buf);	/* flags     */
		argument = one_argument (argument, buf);	/* clan name */

		if ( !*buf )
			break;

		if ( !str_cmp (buf, clan_name) )
			return;
	}

		/* ch doesn't have that clan, add it */

	if ( ch->clans && *ch->clans )
		snprintf (buf, MAX_STRING_LENGTH,  "%s '%s' %s", ch->clans, clan_flags, clan_name);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "'%s' %s", clan_flags, clan_name);

	mem_free (ch->clans);

	ch->clans = str_dup (buf);
}

int is_clan_member (CHAR_DATA *ch, char *clan_name)
{
	int		clan_flags;

	if ( get_clan (ch, clan_name, &clan_flags) )
		return 1;

	return 0;
}

/* the _player version of this routine is called to parse a clan name
   a player might type in.  If clan is registered, then only the full
   name is valid.  If it is not, the short name is appropriate.
*/

int is_clan_member_player (CHAR_DATA *ch, char *clan_name)
{
	CLAN_DATA	*clan;

	if ( is_clan_member (ch, clan_name) ) {		/* Short name match */
		if ( get_clandef (clan_name) )
			return 0;		/* Supplied short name when long existed */
		return 1;
	}

	if ( !(clan = get_clandef_long (clan_name)) )
		return 0;

	return is_clan_member (ch, clan->name);
}

void clan_object_equip (CHAR_DATA *ch, OBJ_DATA *obj)
{
	int			clan_flags;
	CLAN_DATA	*clan;

	for ( clan = clan_list; clan; clan = clan->next ) {
		if ( clan->member_vnum == obj->virtual ||
			 clan->leader_vnum == obj->virtual )
			break;
	}

	if ( !clan )
		return;

	get_clan (ch, clan->name, &clan_flags);

	remove_clan (ch, clan->name);

	if ( clan->member_vnum == obj->virtual )
		SET_BIT (clan_flags, CLAN_MEMBER_OBJ);

	if ( clan->leader_vnum == obj->virtual )
		SET_BIT (clan_flags, CLAN_LEADER_OBJ);

	add_clan (ch, clan->name, clan_flags);
}

void clan_object_unequip (CHAR_DATA *ch, OBJ_DATA *obj)
{
	int			clan_flags;
	CLAN_DATA	*clan;

	for ( clan = clan_list; clan; clan = clan->next ) {
		if ( clan->member_vnum == obj->virtual ||
			 clan->leader_vnum == obj->virtual )
			break;
	}

	if ( !clan )
		return;

	get_clan (ch, clan->name, &clan_flags);

	remove_clan (ch, clan->name);

	if ( clan->member_vnum == obj->virtual )
		REMOVE_BIT (clan_flags, CLAN_MEMBER_OBJ);

	if ( clan->leader_vnum == obj->virtual )
		REMOVE_BIT (clan_flags, CLAN_LEADER_OBJ);

	if ( clan_flags )
		add_clan (ch, clan->name, clan_flags);
}

