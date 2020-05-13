/** 
*	\file act.other.c
*	Miscellanous actions for players
*
*	This module provieds functionality for miscellanous actions not covered in
*	other files. These include prsion actions, stealth, assorted skill-related
*	actions, clan related activities. Other actions related to gameplay such as
*	quit, commence, timed actions, and various affect updates.
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

extern const int forageables [NUM_FORAGEABLES];

int is_in_cell (CHAR_DATA *ch, int zone)
{
	CHAR_DATA	*jailer;

	if ( !zone_table[zone].jailer )
		return 0;

	jailer = vtom (zone_table [zone].jailer);

	if ( !jailer )
		return 0;

	if ( ch->in_room == jailer->cell_1 )
		return 1;
	else if ( ch->in_room == jailer->cell_2 )
		return 1;
	else if ( ch->in_room == jailer->cell_3 )
		return 1;

	return 0;
}


void do_commence (CHAR_DATA *ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA		*td = NULL;
	bool		beta_start = FALSE;
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;

	if ( !IS_SET (ch->plr_flags, NEWBIE) ) {
		send_to_char ("It appears that you've already begun play...\n", ch);
		return;
	}

	if ( IS_SET (ch->flags, FLAG_GUEST) ) {
		send_to_char ("Sorry, but guests are not allowed into the gameworld.\n", ch);
		return;
	}

	act ("$n decides to take the plunge, venturing off into the world for the very first time!", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	snprintf (buf, MAX_STRING_LENGTH,  "\n#6Welcome to %s!#0\n\n", MUD_NAME);
	send_to_char (buf, ch);

	if ( ch->race == 1 || IS_SET (ch->plr_flags, START_BETA) ) {
		char_from_room (ch);
		char_to_room (ch, BETA_START_LOC);
		ch->was_in_room = 0;
		add_clan (ch, "beta_denizens", CLAN_MEMBER);
		if ( !ch->skills [SKILL_SPEAK_BLACK_SPEECH] || ch->skills [SKILL_SPEAK_BLACK_SPEECH] < 30 ) {
			ch->skills [SKILL_SPEAK_BLACK_SPEECH] = 30;
			ch->pc->skills [SKILL_SPEAK_BLACK_SPEECH] = 30;
		}
		beta_start = TRUE;
	}
	else if (IS_SET (ch->plr_flags, START_ALPHA)){
		char_from_room (ch);
		char_to_room (ch, ALPHA_START_LOC);
		ch->was_in_room = 0;
		add_clan (ch, "alpha_citizens", CLAN_MEMBER);
		if ( !ch->skills [SKILL_SPEAK_WESTRON] || ch->skills [SKILL_SPEAK_WESTRON] < 30 ) {
			ch->skills [SKILL_SPEAK_WESTRON] = 30;
			ch->pc->skills [SKILL_SPEAK_WESTRON] = 30;
		}
		
	}
	else {
		char_from_room (ch);
		char_to_room (ch, GAMMA_START_LOC);
		ch->was_in_room = 0;
		add_clan (ch, "gamma_citizens", CLAN_MEMBER);
		if ( !ch->skills [SKILL_SPEAK_DUNAEL] || ch->skills [SKILL_SPEAK_DUNAEL] < 30 ) {
			ch->skills [SKILL_SPEAK_DUNAEL] = 30;
			ch->pc->skills [SKILL_SPEAK_DUNAEL] = 30;
		}
	}
	
	REMOVE_BIT (ch->plr_flags, NEWBIE);
	ch->time.played = 0;
	do_save (ch, "", 0);

	act ("$n has entered YourWorld for the very first time!", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	temp_arg = char_short(ch);
	snprintf (buf, MAX_STRING_LENGTH, "#3[%s has entered YourWorld for the first time at the %s.]#0", temp_arg, (beta_start) ? "Beta Room" : "Alpha Room");

	for ( td = descriptor_list; td; td = td->next ) {
		if ( !td->character || td->connected != CON_PLYNG )
			continue;
		if ( !is_brother (ch, td->character) )
			continue;
		if ( IS_SET (td->character->plr_flags, MENTOR) ) {
			act (buf, TRUE, ch, 0, td->character, TO_VICT | TO_ACT_FORMAT);
		}
	}
	return;
}		


void delayed_ooc (CHAR_DATA *ch)
{
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	char	buffer [MAX_STRING_LENGTH] = {'\0'};
	char	*temp_arg = NULL;
	
	send_to_char ("You are now exiting to the OOC lounge...\n", ch);
	send_to_char ("\n#1Remember, the use of this lounge is a privilege -- do not abuse it!\n"
	              "To return to the in-character parts of the grid, use the IC command.#0\n\n", ch);

	temp_arg = char_short(ch);
	snprintf (buf, MAX_STRING_LENGTH,  "%s#0 has exited to the OOC lounge.)", temp_arg);
	*buf = toupper(*buf);
	snprintf (buffer, MAX_STRING_LENGTH,  "(#5%s", buf);
	snprintf (buf, MAX_STRING_LENGTH,  "%s", buffer);

	act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	ch->was_in_room = ch->room->virtual;
	char_from_room (ch);
	char_to_room (ch, OOC_LOUNGE);

	temp_arg = char_short(ch);
	snprintf (buf, MAX_STRING_LENGTH,  "%s#0 has entered the OOC lounge.", temp_arg);
	*buf = toupper(*buf);
	snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
	snprintf (buf, MAX_STRING_LENGTH,  "%s", buffer);

	act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	do_look (ch, "", 0);

	ch->delay = 0;
	return;
}

void do_quit (CHAR_DATA *ch, char *argument, int cmd)
{
	char		arg [MAX_INPUT_LENGTH] = {'\0'};
	int			dwelling = 0;
	int			outside = 0;
	bool		block = FALSE;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*obj = NULL;
	CHAR_DATA	*tch = NULL;
	DESCRIPTOR_DATA		*d = NULL;
	char		*temp_arg = NULL;

	if ( ch->desc && ch->desc->original ) {
		send_to_char ("Not while you are switched.  RETURN first.\n\r", ch);
		return;
	}

	if ( is_switched (ch) )
		return;

	if ( IS_NPC (ch) ) {
		send_to_char ("NPC's cannot quit.\n\r", ch);
		return;
	}

	
	argument = one_argument (argument, arg);	

	if ( !IS_SET (ch->flags, FLAG_GUEST) &&
		IS_MORTAL (ch) &&
		!IS_SET (ch->room->room_flags, SAFE_Q) &&
		ch->desc &&
		cmd != 3 ) {
		send_to_char ("You may not quit in this area. Most inns, taverns, and so forth are\n"
		              "designated as safe-quit areas; if you are in the wilderness, you may\n"
     		              "also use the CAMP command to create a safe-quit room. #1If you simply\n"
 		              "drop link without quitting, your character will remain here, and may\n"
    		              "be harmed or injured while you are away. We make no guarantees!#0\n", ch);
		return;
	}

	if ( GET_POS (ch) == POSITION_FIGHTING && !IS_SET (ch->flags, FLAG_GUEST) ) {
		send_to_char ("You can't quit during combat!\n", ch);
		return;
	}

	if ( IS_SUBDUEE (ch) ) {
		act ("$N won't let you leave!", FALSE, ch, 0, ch->subdue, TO_CHAR);
		return;
	}

	if ( IS_MORTAL (ch) &&
		 IS_SET (ch->room->room_flags, LAWFUL) &&
		 get_affect (ch, MAGIC_CRIM_BASE + ch->room->zone) && cmd != 3 ) {
		send_to_char ("You may not exit the game here.  While a criminal in "
					  "this area you may not quit\nin lawful places.\n", ch);
		return;
	}

	if ( IS_HITCHER (ch) && !cmd ) {
		send_to_char ("You cannot quit out with a hitched mount; try CAMPING or the STABLE.\n", ch);
		return;
	}

	if ( IS_SUBDUER (ch) )
		do_release (ch, "", 0);

	if ( !IS_NPC (ch) && ch->pc->edit_player ) {
		unload_pc (ch->pc->edit_player);
		ch->pc->edit_player = NULL;
	}

	remove_affect_type (ch, MAGIC_SIT_TABLE);

	act ("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
	act ("$n leaves the area.", TRUE, ch, 0, 0, TO_ROOM);

	snprintf (s_buf, MAX_STRING_LENGTH,  "%s has left the game.\n", GET_NAME (ch));

	if ( !ch->pc->admin_loaded )
		send_to_gods (s_buf);

	d = ch->desc;

	snprintf (s_buf, MAX_STRING_LENGTH,  "%s is quitting.  Saving character.", GET_NAME (ch));
	system_log (s_buf, FALSE);

	if ( ch->pc )	/* Not sure why we wouldn't have a pc, but being careful */
		ch->pc->last_logoff = time (0);

	if ( ch->in_room > 100000 ) {
		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( tch != ch && IS_MORTAL(tch) )
				block = TRUE;
		}
		dwelling = ch->in_room;
		outside = ch->was_in_room;
	}

	extract_char (ch);

	if ( dwelling > 100000 && !block && vtor(outside) ) {
		for ( obj = (vtor(outside))->contents; obj; obj = obj->next_content ) {
			if ( obj->o.od.value[0] == dwelling && GET_ITEM_TYPE (obj) == ITEM_TENT ) {
				SET_BIT (obj->obj_flags.extra_flags, ITEM_VNPC);
				temp_arg = obj_short_desc(obj);
				snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 grows still as its occupants settle in.", temp_arg);
				buf[2] = toupper(buf[2]);
				send_to_room (buf, obj->in_room);
				break;
			}
		}
	}

	ch->desc = NULL;

	if ( !d )
		return;

	d->character = NULL;

	if ( IS_SET (ch->flags, FLAG_GUEST) ) {
		snprintf(buf2, MAX_STRING_LENGTH, "%s", ch->tname);
		if (strstr(buf2, "Special Guest")){
			d->connected = CON_PENDING_DISC;
			nanny(d, "");
			return;
		}
		else { /* normal guest */
			d->connected = CON_ACCOUNT_MENU;
			nanny(d, "");
			return;
		}

		d->connected = CON_ACCOUNT_MENU;
		nanny (d, "");
	}
	d->connected = CON_ACCOUNT_MENU;
	nanny (d, "");
	return;
}

void do_save(CHAR_DATA *ch, char *argument, int cmd)
{
	char		*person = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( GET_TRUST (ch) ) { /* saving a help document */
		person = one_argument (argument, buf);

		if ( *buf ) {
			save_document (ch, argument);
			return;
		}
	}

	if (IS_NPC(ch) )
		return;

	snprintf (buf, MAX_STRING_LENGTH,  "Saving %s.\n\r", GET_NAME(ch));
	send_to_char (buf, ch);
	save_char (ch, TRUE);
	return;
}

void do_sneak (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_INPUT_LENGTH] = {'\0'};
	int			dir = 0;
	
	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}

	if ( !real_skill (ch, SKILL_SNEAK) ) {
		send_to_char ("You just aren't stealthy enough to try.\n", ch);
		return;
	}

	if ( IS_SUBDUER (ch) ) {
		send_to_char ("You can't sneak while you have someone in tow.\n", ch);
		return;
	}

	if ( IS_ENCUMBERED (ch) ) {
		send_to_char ("You are too encumbered to sneak.\n\r", ch);
		return;
	}

	if ( IS_SWIMMING (ch) ) {
		send_to_char ("You can't do that while swimming!\n", ch);
		return;
	}

	if ( ch->speed == SPEED_JOG ||
		 ch->speed == SPEED_RUN ||
         ch->speed == SPEED_SPRINT ) {
		snprintf (buf, MAX_STRING_LENGTH,  "You can't sneak and %s at the same time.\n",
					  speeds [ch->speed]);
		send_to_char (buf, ch);
		return;
	}

	argument = one_argument (argument, buf);

	if (IS_NPC(ch) && IS_SET(ch->affected_by, AFF_SNEAK)) {
		if (cmd == 0) snprintf (buf, 6,  "north");
		if (cmd == 1) snprintf (buf, 5,  "east");
		if (cmd == 2) snprintf (buf, 5,  "south");
		if (cmd == 3) snprintf (buf, 5,  "west");
		if (cmd == 4) snprintf (buf, 3,  "up");
		if (cmd == 5) snprintf (buf, 5,  "down");
	}

	if ( !*buf ) {
		send_to_char ("Sneak in what direction?\n\r", ch);
		return;
	}

	if ( (dir = index_lookup (dirs, buf)) == -1 ) {
		send_to_char ("Sneak where?\n\r", ch);
		return;
	}

	if ( !CAN_GO (ch, dir) ) {
		if ( ch->room->extra && ch->room->extra->alas [dir] )
            send_to_char (ch->room->extra->alas [dir], ch);
        else
            send_to_char ("There is no exit that way.\n", ch);
		return;
	}

	skill_use (ch, SKILL_SNEAK, 0);

	if ( odds_sqrt (ch->skills [SKILL_SNEAK]) >= number (1, 100) ||
		 !would_reveal (ch) )
		magic_add_affect (ch, MAGIC_SNEAK, -1, 0, 0, 0, 0);

	else {
		remove_affect_type (ch, MAGIC_HIDDEN);
		act ("$n attempts to be stealthy.", TRUE, ch, 0, 0, TO_ROOM);
	}

	do_move (ch, "", dir);
	return;
}

void do_hood (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*obj = NULL;

	if ( !(obj = get_equip (ch, WEAR_ABOUT)) ||
	     (!IS_SET (obj->obj_flags.extra_flags, ITEM_MASK)) ) {
	    send_to_char ("You are not wearing a hooded item.\n\r", ch);
	    return;
	}

	if ( !IS_SET (ch->affected_by, AFF_HOODED) ) {

	    act ("You raise $p's hood, obscuring your face.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);

	    act ("$n raises $p's hood, concealing $s face.",
					FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

	    SET_BIT (ch->affected_by, AFF_HOODED); 

	    return;
	}

	REMOVE_BIT (ch->affected_by, AFF_HOODED); 

	act ("You lower $p's hood, revealing your features.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
	act ("$n lowers $p's hood, revealing $s features.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

	return;
}

void do_hide (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	
	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}

	if ( !real_skill (ch, SKILL_HIDE) ) {
		send_to_char ("You lack the skill to hide.\n", ch);
		return;
	}

	if ( IS_SET (ch->room->room_flags, NOHIDE) ) {
		send_to_char ("This room offers no hiding spots.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf ) {

		if ( !(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
			!(obj = get_obj_in_dark (ch, buf, ch->left_hand)) ) {
			send_to_char ("You don't have that.\n", ch);
			return;
		}

		if ( !get_obj_in_list_vis (ch, buf, ch->right_hand) &&
			!get_obj_in_list_vis (ch, buf, ch->left_hand) ) {
			act ("It is too dark to hide it.", FALSE, ch, 0, 0, TO_CHAR);
			return;
		}

		act ("You begin looking for a hiding spot for $p.",
				FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);

		ch->delay_type = DEL_HIDE_OBJ;
		ch->delay_info1 = (int) obj;
		ch->delay = 5;

		return;
	}

	if ( IS_ENCUMBERED (ch) ) {
		send_to_char ("You are too encumbered to hide.\n", ch);
		return;
	}

	send_to_char ("You start trying to conceal yourself.\n", ch);

	ch->delay_type = DEL_HIDE;
	ch->delay	   = 5;
	return;
}

void delayed_hide (CHAR_DATA *ch)
{
	int			mod = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*obj = NULL;

	ch->delay_type = 0;

	send_to_char ("You settle down in what looks like a good spot.\n", ch);

	switch ( ch->room->sector_type ) {
		case SECT_INSIDE:		mod = 0;   break;
		case SECT_CITY:			mod = 0;   break;
		case SECT_ROAD:			mod = 10;  break;
		case SECT_TRAIL:		mod = 10;  break;
		case SECT_FIELD:		mod = 20;  break;
		case SECT_WOODS:		mod = -10; break;
		case SECT_FOREST:		mod = -20; break;
		case SECT_HILLS:        mod = -20; break;
		case SECT_MOUNTAIN:     mod = -20; break;
		case SECT_SWAMP:        mod = -10; break;
		case SECT_PASTURE:		mod = 20;  break;
		case SECT_HEATH:		mod = 20;  break;
    }
	
	if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_LIGHT )
		obj = ch->right_hand;

	if ( obj ) {
		if ( obj->o.light.hours > 0 && obj->o.light.on ) {

			act ("You put out $p so you won't be detected.",
						FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act ("$n put out $p.",
						FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

			obj->o.light.on = 0;
		}
		obj = NULL;
	}

	if ( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_LIGHT )
		obj = ch->left_hand;

	if ( obj ) {
		if ( obj->o.light.hours > 0 && obj->o.light.on ) {

			act ("You put out $p so you won't be detected.",
						FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act ("$n put out $p.",
						FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

			obj->o.light.on = 0;
		}
		obj = NULL;
	}

	for ( obj = ch->equip; obj; obj = obj->next_content ) {
		if ( obj->obj_flags.type_flag != ITEM_LIGHT )
			continue;

		if ( obj->o.light.hours > 0 && obj->o.light.on ) {

			act ("You put out $p so you won't be detected.",
						FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act ("$n put out $p.",
						FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

			obj->o.light.on = 0;
		}
	}

	if ( get_affect (ch, MAGIC_HIDDEN) ) {
		room_light (ch->room);
		return;
	}

	if ( skill_use (ch, SKILL_HIDE, mod) ) {
		magic_add_affect (ch, MAGIC_HIDDEN, -1, 0, 0, 0, 0);
		snprintf (buf, MAX_STRING_LENGTH,  "[%s hides]", ch->tname);
		act (buf, TRUE, ch, 0, 0, TO_NOTVICT | TO_IMMS);
	}

	room_light (ch->room);
	return;
}

void delayed_hide_obj (CHAR_DATA *ch)
{
	OBJ_DATA		*obj = NULL;
	AFFECTED_TYPE	*af = NULL;

	obj = (OBJ_DATA *) ch->delay_info1;

	if ( obj != ch->right_hand && obj != ch->left_hand ) {
		send_to_char ("You don't have whatever you were hiding anymore.\n", ch);
		return;
	}

	obj_from_char (&obj, 0);
	obj_to_room (obj, ch->in_room);

	remove_obj_affect (obj, MAGIC_HIDDEN); 		/* Probably doesn't exist */

	af = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);

	af->type = MAGIC_HIDDEN;
	af->a.hidden.duration = -1;
	af->a.hidden.hidden_value = ch->skills [SKILL_HIDE];
	af->a.hidden.coldload_id = ch->coldload_id;

	act ("You hide $p.", FALSE, ch, obj, 0, TO_CHAR);

	af->next = obj->xaffected;
	obj->xaffected = af;

	act ("$n hides $p.", FALSE, ch, obj, 0, TO_ROOM);
	return;
}

void do_palm (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		target [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*tch = NULL;
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*tobj = NULL;
	AFFECTED_TYPE	*af = NULL;
	int			modifier = 0;
	bool		into = FALSE;
	bool		from = FALSE;
	char		*temp_arg = NULL;

        if ( IS_SWIMMING (ch) ) {
                send_to_char ("You can't do that while swimming!\n", ch);
                return;
        }

	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}

	if ( !real_skill (ch, SKILL_STEAL) ) {
		send_to_char ("You lack the skill to palm objects.\n\r", ch);
		return;
	}

	argument = one_argument (argument, target);

	if ( !*target ) {
		send_to_char ("What did you wish to palm?\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		if ( ch->right_hand && ch->left_hand ) {
			send_to_char ("One of your hands needs to be free before attempting to palm something.\n", ch);
			return;
		}
		if ( !(obj = get_obj_in_list_vis (ch, target, ch->room->contents)) ) {
			send_to_char ("You don't see that here.\n", ch);
			return;
		}
		if ( obj->obj_flags.weight/100 > 3 ) {
			send_to_char ("That's too heavy for you to pick up very stealthily.\n", ch);
			return;
		}
		if ( !IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE) ) {
			send_to_char ("That cannot be picked up.\n", ch);
			return;
		}
		obj_from_room (&obj, 0);
		clear_omote (obj);
		act ("You carefully attempt to palm $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
		if ( !skill_use (ch, SKILL_STEAL, obj->obj_flags.weight/100) )
			act ("$n attempts to surreptitiously take $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
		obj_to_char (obj, ch);
	}
	else {
		if ( !str_cmp (buf, "into") ) {
			argument = one_argument (argument, buf);
			into = TRUE;
		}
		else if ( !str_cmp (buf, "from") ) {
			argument = one_argument (argument, buf);
			from = TRUE;
		}
		else {
			send_to_char ("Do you wish to palm INTO or FROM a container?\n", ch);
			return;
		}

		if ( into && (tch = get_char_room_vis (ch, buf)) ) {
			if ( tch == ch ) {
				send_to_char ("Somehow I doubt you're quite THAT sneaky...\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			if ( from && ch->right_hand && ch->left_hand ) {
				send_to_char ("One of your hands needs to be free before attempting to palm something.\n", ch);
				return;
			}
			if ( (!(tobj = get_obj_in_list (target, ch->right_hand)) &&
				!(tobj = get_obj_in_list (target, ch->left_hand))) ) {
				send_to_char ("What did you wish to palm into it?\n", ch);
				return;
			}
			if ( !(obj = get_obj_in_list_vis (ch, buf, tch->equip)) ) {
				send_to_char ("What on your victim did you wish to palm it into?\n", ch);
				return;
			}
			if ( GET_ITEM_TYPE (obj) != ITEM_CONTAINER ) {
				send_to_char ("You can only palm items into containers.\n", ch);
				return;
			}
			if ( tobj->obj_flags.weight/100 > 3 ) {
				send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
				return;
			}
			modifier = tch->skills [SKILL_SCAN]/5;
			modifier += tobj->obj_flags.weight/100;
			modifier += 15;
			temp_arg = char_short(tch);
			snprintf (buf, MAX_STRING_LENGTH,  "You carefully slide $p into #5%s#0's $P.", temp_arg);
			act (buf, FALSE, ch, tobj, obj, TO_CHAR | TO_ACT_FORMAT);
			if ( !skill_use (ch, SKILL_STEAL, modifier) ) {
				temp_arg = obj_short_desc(obj);
				snprintf (buf, MAX_STRING_LENGTH,  "$n approaches you and surreptitiously attempts to slip $p into #2%s#0!", temp_arg); 
				act (buf, FALSE, ch, tobj, tch, TO_VICT | TO_ACT_FORMAT);
				
				temp_arg = obj_short_desc(obj);
				snprintf (buf, MAX_STRING_LENGTH,  "$n approaches $N and surreptitiously atttempts to slip $p into #2%s#0.", temp_arg); 
				act (buf, FALSE, ch, tobj, tch, TO_NOTVICT | TO_ACT_FORMAT);
			}
		        for ( af = tobj->xaffected; af; af = af->next )
       	         		affect_modify (ch, af->type, af->a.spell.location,
       	               			af->a.spell.modifier, obj->obj_flags.bitvector, FALSE, 0);
			if ( ch->right_hand == tobj )
				ch->right_hand = NULL;
			else if ( ch->left_hand == tobj )
				ch->left_hand = NULL;
			obj_to_obj (tobj, obj);
		}
		else if ( (obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) ) {
			if ( from && ch->right_hand && ch->left_hand ) {
				send_to_char ("One of your hands needs to be free before attempting to palm something.\n", ch);
				return;
			}
			if ( from && !(tobj = get_obj_in_list_vis (ch, target, obj->contains)) ) {
				send_to_char ("You don't see such an item in that container.\n", ch);
				return;
			}
			else if ( into && (!(tobj = get_obj_in_list (target, ch->right_hand)) &&
				!(tobj = get_obj_in_list (target, ch->left_hand))) ) {
				send_to_char ("What did you wish to palm into it?\n", ch);
				return;
			}
			if ( tobj->obj_flags.weight/100 > 3 ) {
				send_to_char ("That's too heavy for you to palm very stealthily.\n", ch);
				return;
			}
			if ( from ) {
				act ("You carefully attempt to palm $p from $P.", FALSE, ch, tobj, obj, TO_CHAR | TO_ACT_FORMAT);
				if ( !skill_use (ch, SKILL_STEAL, tobj->obj_flags.weight/100) )
					act ("$n gets $p from $P.", FALSE, ch, tobj, obj, TO_ROOM | TO_ACT_FORMAT);
				obj_from_obj (&tobj, 0);
				obj_to_char (tobj, ch);
			}
			else if ( into ) {
				if ( GET_ITEM_TYPE (obj) != ITEM_CONTAINER ) {
					send_to_char ("You can only palm items into containers.\n", ch);
					return;
				}
			 	if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE) ) {
					act ("You carefully slide $p into $P.", FALSE, ch, tobj, obj, TO_CHAR | TO_ACT_FORMAT);
					if ( !skill_use (ch, SKILL_STEAL, tobj->obj_flags.weight/100) )
						act ("$n attempts to surreptitiously place $p into $P.", FALSE, ch, tobj, obj, TO_ROOM | TO_ACT_FORMAT);
				}
				else {
					act ("You carefully slide $p onto $P.", FALSE, ch, tobj, obj, TO_CHAR | TO_ACT_FORMAT);
					if ( !skill_use (ch, SKILL_STEAL, tobj->obj_flags.weight/100) )
						act ("$n attempts to surreptitiously place $p atop $P.", FALSE, ch, tobj, obj, TO_ROOM | TO_ACT_FORMAT);
				}
			        for ( af = tobj->xaffected; af; af = af->next )
       	         		affect_modify (ch, af->type, af->a.spell.location,
       	                        		af->a.spell.modifier, obj->obj_flags.bitvector, FALSE, 0);
				if ( ch->right_hand == tobj )
					ch->right_hand = NULL;
				else if ( ch->left_hand == tobj )
					ch->left_hand = NULL;
				obj_to_obj (tobj, obj);
			}
		}
		else if ( (obj = get_obj_in_list (buf, ch->equip)) ) {
			if ( GET_ITEM_TYPE (obj) != ITEM_SHEATH ) {
				send_to_char ("That isn't a sheath.\n", ch);
				return;
			}
			if ( !IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WRIST) ) {
				send_to_char ("Palm only works with wrist sheaths.\n", ch);
				return;
			}
			if ( !(tobj = get_obj_in_list (target, obj->contains)) && from ) {
				send_to_char ("The specified weapon isn't in the sheath.\n", ch);
				return;
			}
			else if ( into ) {
				if ( !(tobj = get_obj_in_list (target, ch->right_hand)) &&
					!(tobj = get_obj_in_list_vis (ch, target, ch->left_hand)) ) {
					send_to_char ("What weapon did you want to palm?\n", ch);
					return;
				}
			}
			if ( !tobj ) {
				send_to_char ("What did you wish to palm?\n", ch);
				return;
			}
			if ( GET_ITEM_TYPE (tobj) != ITEM_WEAPON ) {
				send_to_char ("That isn't a weapon.\n", ch);
				return;
			}
			if ( tobj->obj_flags.weight/100 > 3 ) {
				send_to_char ("That weapon is too heavy for you to palm.\n", ch);
				return;
			}
			if ( tobj->o.od.value [0] != 2 ) {
				send_to_char ("This weapon is too unwieldly to palm.\n", ch);
				return;
			} 
			if ( from ) {
				if ( ch->right_hand && ch->left_hand ) {
					send_to_char ("One of your hands needs to be free before attempting to palm something.\n", ch);
					return;
				}
				act ("You carefully flick $p from $P into your hand.", FALSE, ch, tobj, obj, TO_CHAR | TO_ACT_FORMAT);
				if ( !skill_use (ch, SKILL_STEAL, tobj->obj_flags.weight/100) )
					act ("$n attempts to draw $p surreptitiously.", FALSE, ch, tobj, 0, TO_ROOM | TO_ACT_FORMAT);
				obj_from_obj (&tobj, 0);
				if ( ch->right_hand )
					ch->left_hand = tobj;
				else ch->right_hand = tobj;
				if ( (tobj->o.od.value[0] == 0 || tobj->o.od.value[0] == 2) && !get_equip (ch, WEAR_PRIM) )
					equip_char (ch, tobj, WEAR_PRIM);
				else if ( (tobj->o.od.value[0] == 1 || tobj->o.od.value[0] == 2) && !get_equip (ch, WEAR_SEC) )
					equip_char (ch, tobj, WEAR_SEC);
				else if ( tobj->o.od.value[0] == 3 && !get_equip (ch, WEAR_BOTH) )
					equip_char (ch, tobj, WEAR_BOTH);
			}
			else if ( into ) {
				if ( obj->contains ) {
					send_to_char ("That sheath already bears a weapon.\n", ch);
					return;
				}
				act ("You carefully slide $p into $P.", FALSE, ch, tobj, obj, TO_CHAR | TO_ACT_FORMAT);
				if ( !skill_use (ch, SKILL_STEAL, tobj->obj_flags.weight/100) )
					act ("$n attempts to surreptitiously sheathe $p.", FALSE, ch, tobj, 0, TO_ROOM | TO_ACT_FORMAT);
			        for ( af = tobj->xaffected; af; af = af->next )
                			affect_modify (ch, af->type, af->a.spell.location,
                               			af->a.spell.modifier, obj->obj_flags.bitvector, FALSE, 0);
				if ( ch->right_hand == tobj )
					ch->right_hand = NULL;
				else if ( ch->left_hand == tobj )
					ch->left_hand = NULL;
				obj_to_obj (tobj, obj);
			}
		}
		else {
			if ( from )
				send_to_char ("What did you want to palm from?\n", ch);
			else send_to_char ("What did you wish to palm into?\n", ch);
		}
	}
	return;
}

void do_steal (CHAR_DATA *ch, char *argument, int cmd)
{
	char		target [MAX_INPUT_LENGTH] = {'\0'};
	char		buf [MAX_INPUT_LENGTH] = {'\0'};
	char		command [MAX_INPUT_LENGTH] = {'\0'};
	char		obj_name [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*victim = NULL;
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*tobj = NULL;
	OBJ_DATA	*new_obj = NULL;
	int			i = 0;
	int			j = 0;
	int			obj_num = 0;
	int			amount  = 0;
	int			modifier  = 0;
	int			count = 0;

	snprintf (command, MAX_STRING_LENGTH, "%s", argument);

        if ( IS_SWIMMING (ch) ) {
                send_to_char ("You can't do that while swimming!\n", ch);
                return;
        }

	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}

	if ( !real_skill (ch, SKILL_STEAL) ) {
		send_to_char ("You lack the skill to steal.\n\r", ch);
		return;
	}

	if ( ch->right_hand || ch->left_hand ) {
		send_to_char ("Both your hands need to be free before attempting to steal something.\n", ch);
		return;
	}

	argument = one_argument (argument, target);

	if ( !*target ) {
		send_to_char ("From whom did you wish to steal?\n", ch);
		return;
	}

	if ( *target ) {
		if ( !(victim = get_char_room_vis (ch, target)) ) {
			send_to_char ("Steal from whom?", ch);
			return;
		}

		if ( victim == ch ) {
			send_to_char ("You can't steal from yourself!\n\r", ch);
			return;
		}

		if ( victim->fighting ) {
			act ("$N's moving around too much for you to attempt a grab.", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
			return;
		}

		if ( !IS_NPC(victim) && !IS_MORTAL (victim) && !victim->pc->mortal_mode ) {
			send_to_char ("The immortal cowers as you approach.  You just don't have the heart!\n\r", ch); 
			return;
		}

		argument = one_argument (argument, obj_name);

		if ( *obj_name && *obj_name != '!' ) {

			if ( IS_SET (ch->plr_flags, NEW_PLAYER_TAG) && IS_SET (ch->room->room_flags, LAWFUL) && *argument != '!' ) {
				snprintf (buf, MAX_STRING_LENGTH,  "You are in a lawful area. If you're caught stealing, "
					      "you may be killed or imprisoned. To confirm, "
					      "type \'#6steal %s !#0\', without the quotes.", command);
				act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				return;
			}

			if ( (tobj = get_equip (victim, WEAR_BELT_1)) &&
				 CAN_SEE_OBJ (ch, tobj) &&
				 name_is (obj_name, tobj->name) )
				obj = tobj;

			else if ( (tobj = get_equip (victim, WEAR_BELT_2)) &&
				 CAN_SEE_OBJ (ch, tobj) &&
				 name_is (obj_name, tobj->name) )
				obj = tobj;

			if ( !obj ) {
				send_to_char ("You don't see that on their belt.\n", ch);
				return;
			}
	
			modifier = victim->skills [SKILL_SCAN]/5;
			modifier += obj->obj_flags.weight/100;
			modifier += 15;

			if ( skill_use (ch, SKILL_STEAL, modifier) ) {
				act ("You approach $N, deftly slipping $p from $S belt and moving off before you can be noticed.", FALSE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);
				(void)unequip_char (victim, obj->location);
				obj_to_char (obj, ch);
				return;
			}
			else {
				if ( skill_use (victim, SKILL_SCAN, 0) ) {
					act ("You approach $N cautiously, but at the last moment, before you can make the grab, $E glances down and notices your attempt!", FALSE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);
					act ("Suddenly, you glance down, and notice $m attempting to lift $p from your belt!", FALSE, ch, obj, victim, TO_VICT | TO_ACT_FORMAT);
					send_to_char ("\n", ch);
					criminalize (ch, victim, victim->room->zone, CRIME_STEAL);
					return;
				}
				else {
					act ("You approach $N cautiously, but at the last moment $E turns away, and your attempt is stymied. Thankfully, however, you have not been noticed.", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
					return;
				}
			} 
		}
		else {

			if ( IS_SET (ch->plr_flags, NEW_PLAYER_TAG) && IS_SET (ch->room->room_flags, LAWFUL) && *obj_name != '!' ) {
				snprintf (buf, MAX_STRING_LENGTH,  "You are in a lawful area. If you're caught stealing, "
					      "you may be killed or imprisoned. To confirm, "
					      "type \'#6steal %s !#0\', without the quotes.", command);
				act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				return;
			}

			for ( obj = victim->equip; obj; obj = obj->next_content )
				if ( GET_ITEM_TYPE (obj) == ITEM_CONTAINER && !CAN_WEAR (obj, ITEM_WEAR_FEET) && !CAN_WEAR (obj, ITEM_WEAR_WRIST) )
					break;

			if ( !obj ) {
				send_to_char ("They don't seem to be wearing any containers you can pilfer from.\n", ch);
				return;
			}

			i = 0;

			for ( tobj = obj->contains; tobj; tobj = tobj->next_content )
				i++;
		
			obj_num = number(0, i);

			j = 0;

			for ( tobj = obj->contains; tobj; tobj = tobj->next_content, j++ )
				if ( j == obj_num )
					break;
			
                	if ( IS_SET(obj->o.container.flags, CONT_CLOSED) )
				modifier += 15;

			modifier = victim->skills [SKILL_SCAN]/10;

			if ( tobj )
				modifier += tobj->obj_flags.weight;

			if ( skill_use (ch, SKILL_STEAL, modifier) ) {
				if ( tobj ) {
					act ("You approach $N cautiously, slipping a hand inside $p. A moment later you move stealthily away, having successfully lifted something!", FALSE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);
					if ( tobj->count >= 6 ) {
						amount = number(1,6);
						tobj->count -= amount;
						new_obj = load_object(tobj->virtual);
						new_obj->count = amount;
						obj_to_char (new_obj, ch);
						if ( tobj->count <= 0 ) {
							tobj->count = 1;
							extract_obj(tobj);
						}
					}
					else { 
						count = tobj->count;
						count = MAX (1, tobj->count);
						amount = number(1,count);
						tobj->count -= amount;
						new_obj = load_object(tobj->virtual);
						new_obj->count = amount;
						obj_to_char (new_obj, ch);
						if ( tobj->count <= 0 ) {
							tobj->count = 1;
							extract_obj (tobj);
						}
					}
					return;
				}
				else {
					act ("You approach $N cautiously, slipping your hand stealthily into $p. A moment later you withdraw, having been unable to find anything to lift, though your attempt has gone unnoticed.", FALSE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);
					return;
				}
			}
			else {
				if ( skill_use (victim, SKILL_SCAN, 0) ) {
					act ("You approach $N cautiously, moving to slip your hand into $p. At the last moment, $E glances down, noticing your attempt!", FALSE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);
					act ("You glance down, your gaze having been attracted by a hint of movement; you notice $n's hand moving toward $p, in an attempt to pilfer from you!", FALSE, ch, obj, victim, TO_VICT | TO_ACT_FORMAT);
					send_to_char ("\n", ch);
					criminalize (ch, victim, victim->room->zone, CRIME_STEAL);
					return;
				}
				else {
					act ("You approach $N cautiously, but at the last moment $E turns away, fate having allowed $M to evade your attempt. Thankfully, it would seem that you were unnoticed.", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
					return;
				}
			}
		}				
	}
	return;
}


void do_idea (CHAR_DATA *ch, char *argument, int cmd)
{

	send_to_char ("Please submit your ideas for consideration on our web forum, located at:\n\n"
		      "   #6http://www.yourmud.org/forums/#0\n", ch);
	return;

}

void post_typo (DESCRIPTOR_DATA *d)
{
	CHAR_DATA *ch;
	char	buf2 [MAX_STRING_LENGTH];
	char	msg [MAX_STRING_LENGTH];
	char	*date = NULL;
	char	*date2 = NULL;
	time_t	account_time;

        account_time = d->account->created_on;
        date = (char *)asctime (localtime (&account_time));

	date2 = timestr(date2);

	ch = d->character;
	if ( !*d->pending_message->message ) {
		send_to_char ("No typo report posted.\n", ch);
		mem_free (date);
		mem_free (date2);
		return;
	}

	snprintf (buf2, MAX_STRING_LENGTH,  "%s", zone_table[ch->room->zone].name);

        snprintf (msg, MAX_STRING_LENGTH, "From: %s [%d]\n", ch->tname, ch->in_room);
        snprintf (msg + strlen(msg), MAX_STRING_LENGTH, "\n");
	snprintf (msg + strlen(msg), MAX_STRING_LENGTH, "%s", ch->desc->pending_message->message);

	if (!(board_lookup ("Typos")))
		add_board (1, "Typos", "Typos");
	add_message ("Typos", 2, ch->desc->account->name, date2, buf2, "", msg, 0);
			
	send_to_char("Thank you! Your typo report has been entered into our tracking system.\n\r", ch);

	unload_message (d->pending_message);

	mem_free (date);
	mem_free (date2);
	return;
}

void do_typo (CHAR_DATA *ch, char *argument, int cmd)
{

	if ( IS_NPC (ch) ) {
		send_to_char ("Mobs can't submit bug reports.\n\r", ch);
		return;
	}

	if ( IS_SET (ch->flags, FLAG_GUEST) ) {
		send_to_char ("Sorry, but guests don't have access to this command...\n", ch);
		return;
	}

	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	send_to_char ("Enter a typo report to be submitted to the admins. Terminate\n"
	              "the editor with an '@' symbol. Please note that your post\n"
	              "will be stamped with all pertinent contact information; no\n"
		      "need to include that in the body of your message. Thanks for\n"
		      "doing your part to help improve our world!\n", ch);

	make_quiet (ch);

	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);

	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->desc->proc = post_typo;
}



void do_bug (CHAR_DATA *ch, char *argument, int cmd)
{

	send_to_char ("This command has been disabled; we have found in past experience that bug\n"
		      "reports are best posted on our web forum, in case they aren't actually bugs.\n"
		      "However, if you feel this is an urgent issue, please email the staff.\n", ch);
	return;
}


void do_compact (CHAR_DATA *ch, char *argument, int cmd)
{
	if ( IS_SET (ch->flags, FLAG_COMPACT) ) {
		send_to_char ("You are now in the uncompacted mode.\n\r", ch);
		REMOVE_BIT (ch->flags, FLAG_COMPACT);
		return;
	}

	send_to_char ("You are now in compact mode.\n\r", ch);
	SET_BIT (ch->flags, FLAG_COMPACT);
	return;
}

void sa_stand (SECOND_AFFECT *sa)
{
	if ( !is_he_somewhere (sa->ch) )
		return;

	if ( GET_POS (sa->ch) == FIGHT ||
		 GET_POS (sa->ch) == STAND )
		return;

	do_stand (sa->ch, "", 0);
	return;
}

void sa_get (SECOND_AFFECT *sa)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !is_he_somewhere (sa->ch) )
		return;

	REMOVE_BIT (sa->obj->tmp_flags, SPA_DROPPED);

	if ( sa->obj == sa->ch->right_hand || sa->obj == sa->ch->left_hand )
		return;
	
	snprintf (buf, MAX_STRING_LENGTH,  "get .c %d", sa->obj->coldload_id);

	command_interpreter (sa->ch, buf);
	return;
}

void sa_wear (SECOND_AFFECT *sa)
{
	int			num = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !is_he_somewhere (sa->ch) )
		return;

	if ( !(num = is_obj_in_list (sa->obj, sa->ch->right_hand)) &&
		!(num = is_obj_in_list (sa->obj, sa->ch->left_hand)) ) {
		if ( !IS_SET (sa->ch->flags, FLAG_COMPETE) ||
			 !is_obj_in_list (sa->obj, sa->ch->room->contents) )
			return;

		extract_obj (sa->obj);

		return;
	}
	
	if ( CAN_WEAR (sa->obj, ITEM_WIELD) )
		snprintf (buf, MAX_STRING_LENGTH,  "wield .c %d", sa->obj->coldload_id);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "wear .c %d", sa->obj->coldload_id);

	if ( !CAN_WEAR (sa->obj, ITEM_WEAR_SHIELD) && GET_ITEM_TYPE (sa->obj) != ITEM_SHIELD )
		command_interpreter (sa->ch, buf);
	return;
}

void sa_close_door (SECOND_AFFECT *sa)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	snprintf (buf, MAX_STRING_LENGTH,  "close %s", sa->info);
	command_interpreter (sa->ch, buf);

	snprintf (buf, MAX_STRING_LENGTH,  "lock %s", sa->info);
	command_interpreter (sa->ch, buf);
	return;
}

void sa_knock_out (SECOND_AFFECT *sa)
{
	if ( !is_he_somewhere (sa->ch) )
		return;

	if ( GET_POS (sa->ch) != SLEEP )
		return;

	if ( get_affect (sa->ch, MAGIC_AFFECT_SLEEP) )
		return;

	GET_POS (sa->ch) = REST;

	send_to_char ("You regain consciousness, flat on the ground.", sa->ch);
	act ("$n regains consciousness.", FALSE, sa->ch, 0, 0, TO_ROOM);
	return;
}

void sa_move (SECOND_AFFECT *sa)
{
	if ( !is_he_somewhere (sa->ch) )
		return;

	if ( GET_POS (sa->ch) < FIGHT )
		return;

	do_move (sa->ch, "", sa->info2);
	return;
}

void add_second_affect (int type, int seconds, CHAR_DATA *ch, OBJ_DATA *obj,
						char *info, int info2)
{
	SECOND_AFFECT		*sa = NULL;

	CREATE (sa, SECOND_AFFECT, 1);

	sa->type	= type;
	sa->seconds	= seconds;
	sa->ch		= ch;
	sa->obj		= obj;
	sa->info2	= info2;

	if ( info )
		sa->info = str_dup (info);
	else
		sa->info = NULL;

	sa->next = second_affect_list;

	second_affect_list = sa;
	return;
}

SECOND_AFFECT *get_second_affect (CHAR_DATA *ch, int type, OBJ_DATA *obj)
{
	SECOND_AFFECT	*sa = NULL;

	for ( sa = second_affect_list; sa; sa = sa->next )
		if ( (!ch || sa->ch == ch) && sa->type == type )
			if ( !obj || obj == sa->obj )
				return sa;

	return NULL;
}

void remove_second_affect (SECOND_AFFECT *sa)
{
	SECOND_AFFECT		*sa_list = NULL;

	if ( sa == second_affect_list ) {

		second_affect_list = sa->next;

		if ( sa->info )
			mem_free (sa->info);

		mem_free (sa);
		return;
	}

	for ( sa_list = second_affect_list; sa_list; sa_list = sa_list->next )
		if ( sa_list->next == sa )
			sa_list->next = sa->next;

	if ( sa->info )
		mem_free (sa->info);

	mem_free (sa);
	return;
}

void second_affect_update (void)
{
	SECOND_AFFECT		*sa = NULL;
	SECOND_AFFECT		*sa_t = NULL;
	SECOND_AFFECT		*next_sa = NULL;

	for ( sa = second_affect_list; sa; sa = next_sa ) {

		next_sa = sa->next;

		if ( --(sa->seconds) > 0 )
			continue;

		if ( sa == second_affect_list )
			second_affect_list = sa->next;
			
		else {
			for ( sa_t = second_affect_list; sa_t->next; sa_t = sa_t->next ){
				if ( sa_t->next == sa ) {
					sa_t->next = sa->next;
					break;
				}
			}
		}

		second_affect_active = 1;

		switch ( sa->type ) {
			case SPA_STAND:
				sa_stand (sa);
				break;
			case SPA_GET_OBJ:
				sa_get (sa);
				break;
			case SPA_WEAR_OBJ:
				sa_wear (sa);
				break;
			case SPA_CLOSE_DOOR:
				sa_close_door (sa);
				break;
			case SPA_WORLD_SWAP: 
				break;
			case SPA_KNOCK_OUT:
				sa_knock_out (sa);
				break;
			case SPA_MOVE:
				sa_move (sa);	
				break;
			case SPA_ESCAPE:
				break;
			case SPA_RESCUE:
				sa_rescue (sa);	
				break;
		}

		second_affect_active = 0;

		if ( sa->info ) {
			mem_free (sa->info);
			sa->info = NULL;
		}

		mem_free (sa);
	}
	return;
}

void prisoner_release (CHAR_DATA *ch, int zone)
{
	CHAR_DATA	*tch = NULL;
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*bag = NULL;
	OBJ_DATA	*tobj = NULL;
	OBJ_DATA	*next_obj = NULL;
	OBJ_DATA	*tobj2 = NULL;
	OBJ_DATA	*next_obj2 = NULL;
	OBJ_DATA	*tobj3 = NULL;
	OBJ_DATA	*next_obj3 = NULL;
	ROOM_DATA	*room = NULL;
	int			dir = 0;
	int			jail_vnum = 0;
	bool		jailed = FALSE;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		msg [MAX_STRING_LENGTH] = {'\0'};
	char		*date = NULL;

		/* Make sure prisoners are in their cell */

	for ( dir = 0; dir <= 5; dir++ ) {
		if ( !ch->room->dir_option[dir] )
			continue;
		
		if ( !(room = vtor (ch->room->dir_option[dir]->to_room)) )
			continue;
		
		for ( tch = room->people; tch; tch = tch->next_in_room ) {
			if ( IS_SET (tch->act, ACT_JAILER) ) {
				jailed = TRUE;
				jail_vnum = tch->in_room;
				break;
			}
		}
	}

	if ( !jailed )
		return;

	send_to_room ("The jailer flings open the cell door.\n\r", ch->in_room);
	act ("The jailer brutally knocks $n unconscious.", FALSE, ch, 0, 0, TO_ROOM);
	act ("The jailer brutally knocks you unconscious.", FALSE, ch, 0, 0, TO_CHAR);

	GET_POS (ch) = SLEEP;

	act ("The jailer removes $n and closes the door before you can react.",
			FALSE, ch, 0, 0, TO_ROOM);

	char_from_room (ch);

	snprintf (buf, MAX_STRING_LENGTH,  "A prison bag, labeled \'Belongings for %s\' sits here.", ch->short_descr);

	bag = NULL;

	for ( obj = object_list; obj; obj = obj->next ) {
		if ( obj->deleted )
			continue;
		if ( GET_ITEM_TYPE (obj) != ITEM_CONTAINER )
			continue;
		if ( !str_cmp (obj->description, buf) ) {
			bag = load_object (VNUM_JAILBAG);
			for ( tobj = obj->contains; tobj; tobj = next_obj ) {
				next_obj = tobj->next_content;
				if ( IS_SET (tobj->obj_flags.extra_flags, ITEM_ILLEGAL) )
					continue;
				for ( tobj2 = tobj->contains; tobj2; tobj2 = next_obj2 ) {
					next_obj2 = tobj2->next_content;
					if ( IS_SET (tobj2->obj_flags.extra_flags, ITEM_ILLEGAL) )
						extract_obj (tobj2);
					if ( GET_ITEM_TYPE (tobj2) == ITEM_WEAPON )
						extract_obj (tobj2);
					for ( tobj3 = tobj2->contains; tobj3; tobj3 = next_obj3 ) {
						next_obj3 = tobj3->next_content;
						if ( IS_SET (tobj3->obj_flags.extra_flags, ITEM_ILLEGAL) )
							extract_obj (tobj3);
						if ( GET_ITEM_TYPE (tobj3) == ITEM_WEAPON )
							extract_obj (tobj3);
					}					
				}
				obj_from_obj (&tobj, 0);
				obj_to_obj (tobj, bag);
			}
			extract_obj (obj);
			obj_to_char (bag, ch);
		}
	}		

	if ( zone == 13 )
		char_to_room (ch, ALPHA_RELEASE_ROOM);
	else if ( zone == 2 )
		char_to_room (ch, BETA_RELEASE_ROOM);
	else if ( zone == 5 )
		char_to_room (ch, GAMMA_RELEASE_ROOM); 
	else
		char_to_room (ch, jail_vnum);

	date = timestr(date);
	
	if ( bag )
		snprintf (msg, MAX_STRING_LENGTH, "Released from prison in %s with belongings intact.\n", zone_table[ch->room->zone].name);
	else
		snprintf (msg, MAX_STRING_LENGTH, "Released from prison in %s. Belongings were not found!\n", zone_table[ch->room->zone].name);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Released:#0 %s", ch->tname);

	add_message (ch->tname, 3, "Server", date, "Released.", "", msg, 0);
	add_message ("Prisoners", 2, "Server", date, buf, "", msg, 0);
 
}

void rl_minute_affect_update (void)
{
	CHAR_DATA		*ch = NULL;
	AFFECTED_TYPE	*af = NULL;
	AFFECTED_TYPE	*next_af = NULL;

	next_minute_update += 60;		/* This is a RL minute */

	for ( ch = character_list; ch; ch = ch->next ) {

		if ( ch->deleted )
			continue;

		if ( !ch->room )
			continue;

		if ( IS_SET (ch->room->room_flags, OOC) )
			continue;

		if ( ch->mana < ch->max_mana && !IS_SET (ch->room->room_flags, OOC) ) {
			ch->mana += (ch->aur / 3) + number(2,6);
		}

		for ( af = ch->hour_affects; af; af = next_af ) {

			next_af = af->next;

			if ( af->type == MAGIC_SIT_TABLE )
				continue;

			/*** NOTE:  Make sure these are excluded in hour_affect_update ***/

			if ( af->type >= MAGIC_SPELL_GAIN_STOP && af->type <= MAGIC_CRAFT_BRANCH_STOP ) {
				if ( --af->a.spell.duration <= 0 )
					affect_remove (ch, af);
			}

			if ( af->type == MAGIC_PETITION_MESSAGE ) {
				if ( --af->a.spell.duration <= 0 )
					affect_remove (ch, af);
			}

			if ( af->type >= MAGIC_FIRST_SOMA &&
			     af->type <= MAGIC_LAST_SOMA ) {
				soma_rl_minute_affect(ch, af);
			}
		}
	}
	return;
}

void character_stink (CHAR_DATA *ch, AFFECTED_TYPE *ch_stink)
{
	int				aroma_strength = 0;
	AFFECTED_TYPE	*room_stink = NULL;

	aroma_strength = ch_stink->a.smell.aroma_strength - 500;

	if ( aroma_strength <= 0 )
		return;

	room_stink = is_room_affected (ch->room->affects, ch_stink->type);

	if ( room_stink &&
		 room_stink->a.smell.aroma_strength > aroma_strength )
		return;

	if ( room_stink &&
		 room_stink->a.smell.duration == -1 )
		return;

	if ( !room_stink ) {
		room_stink = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);
		room_stink->type = ch_stink->type;
		room_stink->next = ch->room->affects;

		ch->room->affects = room_stink;
	}

		/* Increase duration, but not above aroma_strength */

	if ( room_stink->a.smell.duration < aroma_strength ) {
		room_stink->a.smell.duration += aroma_strength / 100;
		if ( room_stink->a.smell.duration > aroma_strength )
			room_stink->a.smell.duration = aroma_strength;
	}

		/* Same with aroma_strength */

	if ( room_stink->a.smell.aroma_strength < aroma_strength ) {
		room_stink->a.smell.aroma_strength += aroma_strength / 100;
		if ( room_stink->a.smell.aroma_strength > aroma_strength )
			room_stink->a.smell.aroma_strength = aroma_strength;
	}
	return;
}

void ten_second_update (void)
{
	CHAR_DATA	*ch, *ch_next;
	AFFECTED_TYPE	*af;
	AFFECTED_TYPE	*next_af;
	AFFECTED_TYPE	*room_stink;

	for ( ch = character_list; ch; ch = ch_next ) {

		ch_next = ch->next;

		if ( ch->deleted )
			continue;

		if ( !ch->room )
			continue;
			
		if ( IS_SET (ch->room->room_flags, OOC) )
			continue;
		
		if ( (ch->room->sector_type == SECT_LAKE || ch->room->sector_type == SECT_RIVER ||
			ch->room->sector_type == SECT_OCEAN || ch->room->sector_type == SECT_REEF ||
			ch->room->sector_type == SECT_UNDERWATER) &&
			!number (0,1) ) {
			if ( swimming_check (ch) )
				continue;		/* PC drowned. */
		}

		for ( af = ch->hour_affects; af; af = next_af ) {

			next_af = af->next;

			if ( af->type == MAGIC_SIT_TABLE )
				continue;

			/***** Exclusion must be made in hour_affect_update!!! ******/

			if ( af->type >= MAGIC_SMELL_FIRST &&
				 af->type <= MAGIC_SMELL_LAST ) {

				room_stink = is_room_affected (ch->room->affects, af->type);

				if ( af->a.smell.duration != -1 &&
					 (!room_stink ||
					  room_stink->a.smell.aroma_strength - 500 <
								af->a.smell.aroma_strength) ) {
					 
					af->a.smell.duration -= 10;
					af->a.smell.aroma_strength -= 10;

					if ( af->a.smell.duration <= 0 ||
						 af->a.smell.aroma_strength <= 0 ) {
						affect_remove (ch, af);
						continue;
					}
				}

				if ( af->a.smell.duration > 500 &&
					 af->a.smell.aroma_strength > 500 )
					character_stink (ch, af);
			}

			else if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST ) {

				if ( IS_NPC (ch) )
					continue;

				if ( !af->a.craft->timer )
					continue;

				af->a.craft->timer -= 10;

				if ( af->a.craft->timer < 0 )
					af->a.craft->timer = 0;

				if ( !af->a.craft->timer )
					activate_phase (ch, af);
			}

			else if ( af->type >= MAGIC_FIRST_SOMA &&
				  af->type <= MAGIC_LAST_SOMA ) {
				soma_ten_second_affect(ch, af);
			}

			else if ( af->type >= MAGIC_CRIM_HOODED &&
				 af->type < MAGIC_CRIM_HOODED + 100 ) {

				af->a.spell.duration -= 10;

					/* If ch is fighting, keep the temp flag on him so
                       we can criminalize him after the fight with the
                       guard is over. */

				if ( ch->fighting && af->a.spell.duration < 1 )
					af->a.spell.duration = 1;

				if ( af->a.spell.duration <= 0 )
					affect_remove (ch, af);
			}

			else if ( af->type == MAGIC_STARED || af->type == MAGIC_WARNED ) {
				af->a.spell.duration -= 10;

				if ( af->a.spell.duration <= 0 )
					affect_remove (ch, af);
			}
		}

		if ( ch->deleted )
			continue;
			
		if ( ch->mob && ch->mob->resets )
			activate_resets (ch);
	}
	return;
}

void payday (CHAR_DATA *ch, CHAR_DATA *employer, AFFECTED_TYPE *af)
{
	int			t = 0;
	int			i = 0;
	OBJ_DATA	*tobj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*temp_obj = NULL;

	if ( time_info.holiday >= 1 ) {
		send_to_char ("Check again after the feastday has ended.\n", ch);
		return;
	}

	t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

	af->a.job.pay_date = t + af->a.job.days;

	if ( af->a.job.cash ) {

		obj = load_object (DEFAULT_COIN_VNUM);
		obj->count = af->a.job.cash;
		obj_to_char (obj, ch);

		if ( employer ) {
			snprintf (buf, MAX_STRING_LENGTH,  "$N pays you %d coppers for all your hard work.", af->a.job.cash);
			act (buf, TRUE, ch, 0, employer, TO_CHAR | TO_ACT_FORMAT);
			act ("$N pays $n some coins.", FALSE, ch, 0, employer, TO_NOTVICT | TO_ACT_FORMAT);
		} else {
			snprintf (buf, MAX_STRING_LENGTH,  "You are paid %d coppers.\n", af->a.job.cash);
			send_to_char (buf, ch);
		}
	}

	if ( af->a.job.count && (tobj = vtoo (af->a.job.object_vnum)) ) {

		for ( i = af->a.job.count; i; i-- ){
			temp_obj = load_object (af->a.job.object_vnum);
			obj_to_char (temp_obj, ch);
		}
		
		if ( employer ) {
			snprintf (buf, MAX_STRING_LENGTH,  "$N pays you %d x $p.", af->a.job.count);
			act (buf, FALSE, ch, tobj, employer, TO_CHAR | TO_ACT_FORMAT);
			act ("$N pays $n with $o.", TRUE, ch, tobj, employer, TO_NOTVICT | TO_ACT_FORMAT);
		} else {
			snprintf (buf, MAX_STRING_LENGTH,  "You are paid %d x $p.", af->a.job.count);
			act (buf, FALSE, ch, tobj, 0, TO_CHAR | TO_ACT_FORMAT);
			act ("$n is paid with $o.", TRUE, ch, tobj, 0, TO_ROOM | TO_ACT_FORMAT);
		}
	}

	snprintf (buf, MAX_STRING_LENGTH,  "Your next payday is in %d days.\n", af->a.job.days);
	send_to_char (buf, ch);
	
	return;
}

void do_payday (CHAR_DATA *ch, char *argument, int cmd)
{
	int				i = 0;
	int				t = 0;
	CHAR_DATA		*employer = NULL;
	OBJ_DATA		*tobj = NULL;
	AFFECTED_TYPE	*af = NULL;
	CHAR_DATA		*tch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buffer [MAX_STRING_LENGTH] = {'\0'};

	if ( time_info.holiday >= 1 ) {
		send_to_char ("Check again after the feastday has ended.\n", ch);
		return;
	}

	t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

	for ( i = JOB_1; i <= JOB_3; i++ ) {

		if ( !(af = get_affect (ch, i)) )
			continue;

		if ( af->a.job.employer )
			employer = vtom (af->a.job.employer);

			/* PC action to get paid if it is time */

		if ( t >= af->a.job.pay_date ) {
			if ( !af->a.job.employer ) {
				payday (ch, NULL, af);
				continue;
			}
			for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
				if ( IS_NPC (tch) && tch->mob->virtual == af->a.job.employer ) {
					payday (ch, tch, af);
					continue;
				}
			}
		}

			/* Either its not time to be paid, or employer is not around */

		if ( employer ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s#0 will pay you ", employer->short_descr);
			*buf = toupper (*buf);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
			snprintf (buf, MAX_STRING_LENGTH,  "%s", buffer);
		} 
		else{
			strcpy (buf, "You get paid ");
		}
		
		if ( af->a.job.cash ) {
			snprintf (ADDBUF, MAX_STRING_LENGTH, "%d coppers", af->a.job.cash);
			if ( af->a.job.count && vtoo (af->a.job.object_vnum) )
				strcat (buf, " and ");
		}

		if ( af->a.job.count && (tobj = vtoo (af->a.job.object_vnum)) ) {
			if ( af->a.job.count == 1 )
				strcat (buf, "$p");
			else
				snprintf (ADDBUF, MAX_STRING_LENGTH, "%d x $p",
							af->a.job.count);
		}

		snprintf (ADDBUF, MAX_STRING_LENGTH, ", every %d day%s.  Next #3payday#0 in %d "
					"day%s.",
					af->a.job.days, af->a.job.days == 1 ? "" : "s",
					af->a.job.pay_date - t,
					af->a.job.pay_date - t == 1 ? "" : "s");

		act (buf, TRUE, ch, tobj, employer, TO_CHAR | TO_ACT_FORMAT);
	}
	return;
}

void hour_affect_update (void)
{
	int				t = 0;
	CHAR_DATA		*ch = NULL;
	AFFECTED_TYPE	*af = NULL;
	AFFECTED_TYPE	*next_af = NULL;

	t = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;

	for ( ch = character_list; ch; ch = ch->next ) {

		if ( ch->deleted || !ch->room )
			continue;

		if ( ch->room && IS_SET (ch->room->room_flags, OOC) )
			continue;

		for ( af = ch->hour_affects; af; af = next_af ) {

			next_af = af->next;

			if ( af->type == MAGIC_SIT_TABLE )
				continue;

            if ( af->type >= JOB_1 && af->type <= JOB_3 ) {
				
				if ( t < af->a.job.pay_date )
                	continue;

                if ( af->a.job.employer )
                	continue;

				/** uncommnet below for automatic paydays **/
                /* payday (ch, NULL, af); */
                continue;
            }

			if ( af->type >= MAGIC_AFFECT_FIRST &&
				af->type <= MAGIC_AFFECT_LAST )
				continue;

			if ( af->type >= MAGIC_SMELL_FIRST &&
				 af->type <= MAGIC_SMELL_LAST )
				continue;

			if ( af->type >= MAGIC_SKILL_GAIN_STOP &&
				 af->type <= MAGIC_SKILL_GAIN_STOP + LAST_SKILL )
				continue;

			if ( af->type >= MAGIC_FLAG_NOGAIN && 
				af->type <= MAGIC_FLAG_NOGAIN + LAST_SKILL )
				continue;

			if ( af->type >= MAGIC_CRIM_HOODED &&
				 af->type < MAGIC_CRIM_HOODED + 100 )
				continue;

			if ( af->type == MAGIC_STARED )
				continue;

			if ( af->type == MAGIC_RAISED_HOOD )
				continue;

			if ( af->type == MAGIC_CRAFT_DELAY || af->type == MAGIC_CRAFT_BRANCH_STOP )
				continue;

			if ( af->type == MAGIC_PETITION_MESSAGE )
				continue;

			if ( af->type == AFFECT_HOLDING_BREATH )
				continue;

			if ( af->type == MAGIC_GUARD ||
				 af->type == AFFECT_SHADOW ||
				 (af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST) )
				continue;

			if ( af->a.spell.duration > 0 )
				af->a.spell.duration--;

			if ( af->a.spell.duration )
				continue;

			else if ( af->type >= MAGIC_CRIM_BASE + 1 && af->type <= MAGIC_CRIM_BASE + 99 )	
				prisoner_release (ch, af->type - MAGIC_CRIM_BASE);

			else if ( af->type == AFFECT_LOST_CON ) {
				send_to_char ("You finally feel at your best once more.\n", ch);
				ch->con += af->a.spell.sn;
				ch->tmp_con += af->a.spell.sn;
			}

				/******************************************/
				/*   SPELL WEAR-OFF MESSAGE               */
				/******************************************/

			else if ( af->type == MAGIC_AFFECT_TONGUES )
				ch->speaks = af->a.spell.modifier;			/* Revert to former language */

			affect_remove (ch, af);
		}
	}
}

void stop_followers (CHAR_DATA *ch)
{
	CHAR_DATA		*tch;

	for ( tch = character_list; tch; tch = tch->next ) {
		if ( tch->deleted )
			continue;
		if ( tch->following == ch )
			tch->following = NULL;
	}
}

int is_circle_spell (CHAR_DATA *ch, char *spell_name)
{
	char		**tree;
	int			tree_circle;
	int			i;

	tree = religion_trees [ch->deity].tree;

    tree_circle = 0;

	for ( i = 0; *tree [i] != '\n'; i++ ) {

		if ( strtol(tree [i], NULL, 10) != 0 ) {
			tree_circle = strtol(tree [i], NULL, 10);

			if ( tree_circle > ch->circle )
				return 0;
		}

		else if ( !str_cmp (tree [i], spell_name) )
			return 1;
	}

	return 0;
}


void open_skill (CHAR_DATA *ch, int skill)
{
	if ( IS_NPC (ch) ){
		ch->skills [skill] += calc_lookup (ch, REG_OV, skill);
	}
	else {
		ch->pc->skills [skill] = calc_lookup (ch, REG_OV, skill);
		ch->skills [skill] += ch->pc->skills [skill];
	}
}

#define MIN_PREREQ	15

int prereq_skill (CHAR_DATA *ch, CHAR_DATA *victim, int skill,
				  int prereq1, int prereq2)
{
	char	buf [MAX_STRING_LENGTH];

	if ( prereq1 && victim->skills [prereq1] < MIN_PREREQ ) {

		snprintf (buf, MAX_STRING_LENGTH,  "$N cannot learn '%s' until $S learns '%s' sufficiently.", skill_data[skill].skill_name, skill_data[prereq1].skill_name);
		
		act (buf, TRUE, ch, 0, victim, TO_CHAR);

		snprintf (buf, MAX_STRING_LENGTH,  "$N tries to teach you '%s', but cannot until you learn '%s' sufficiently.", skill_data[skill].skill_name, skill_data[prereq1].skill_name);
		
		act (buf, TRUE, victim, 0, ch, TO_CHAR);

		return 0;
	}

	else if ( prereq2 && victim->skills [prereq2] < MIN_PREREQ ) {

		snprintf (buf, MAX_STRING_LENGTH,  "$N cannot learn '%s' until $E learns '%s' sufficiently.",skill_data[skill].skill_name, skill_data[prereq2].skill_name);
		act (buf, TRUE, ch, 0, victim, TO_CHAR);

		snprintf (buf, MAX_STRING_LENGTH,  "$N tries to teach you '%s', but cannot until you learn '%s' sufficiently.", skill_data[skill].skill_name, skill_data[prereq2].skill_name);
		
		act (buf, TRUE, victim, 0, ch, TO_CHAR);

		return 0;
	}

	open_skill (victim, skill);

	return 1;
}

int meets_craft_teaching_requirements (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
    PHASE_DATA  *phase = NULL;

    for ( phase = craft->phases; phase; phase = phase->next ){
    	if ( phase->skill &&
			( !real_skill (ch, phase->skill) ||
			ch->skills[phase->skill] < (int)((phase->dice*phase->sides) * .75))){
        
        	return 0;
        }
    }
    
    return 1;
}

int meets_craft_learning_requirements (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
    PHASE_DATA	*phase = NULL;

    for ( phase = craft->phases; phase; phase = phase->next ) {
    	if ( phase->skill &&
    		(!real_skill (ch, phase->skill) ||
        	ch->skills[phase->skill] < (int)((phase->dice*phase->sides) * .33))){
      		return 0;
        }
	}
    
    return 1;
}

void do_teach (CHAR_DATA *ch, char *argument, int cmd)
{
	char 		arg1 [80];
	char		arg2 [80];
	char		buf [MAX_STRING_LENGTH];
	int			i =  0;
	CHAR_DATA		*victim = NULL;
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	AFFECTED_TYPE		*af = NULL;

	if ( IS_SET (ch->room->room_flags, OOC) ) {
		send_to_char ("This is not allowed in OOC areas.\n", ch);
		return;
	}

	half_chop(argument, arg1,arg2);

	if ( !*arg1 ) {
		send_to_char ("Teach what?\n\r", ch);
		return;
	}

	if ( !*arg2 ) {
		send_to_char ("Teach what to who?\n\r", ch);
		return;
	}

	if( !(victim = get_char_room_vis (ch, arg2)) ) {
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if ( (i = skill_index_lookup (arg1)) == -1) {
		for ( craft = crafts; craft; craft = craft->next ) {
			if ( ch->desc->original && !str_cmp (craft->subcraft_name, arg1) )
				break;
			if ( !str_cmp (craft->subcraft_name, arg1) && has_craft (ch, craft) )
				break;
		}

		if ( craft ) {
			if ( !ch->desc->original && !meets_craft_teaching_requirements (ch, craft) ) {
				send_to_char ("You aren't proficient enough to teach that craft yet.\n", ch);
				return;
			}
			if ( !meets_craft_learning_requirements (victim, craft) ) {
				send_to_char ("The intricacies of this craft seem to be beyond your pupil at this time.\n", ch);
				return;
			}
			if ( has_craft (victim, craft) ) {
				send_to_char ("They already know that craft.\n", ch);
				return;
			}
			for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ )
				if ( !get_affect (victim, i) )
					break;
			magic_add_affect (victim, i, -1, 0, 0, 0, 0);
			af = get_affect (victim, i);
			af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);
			af->a.craft->subcraft = craft;
			snprintf (buf, MAX_STRING_LENGTH,  "You teach $N '%s %s'.", craft->command, craft->subcraft_name);
			act (buf, FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
			snprintf (buf, MAX_STRING_LENGTH,  "$n has taught you '%s %s'.", craft->command, craft->subcraft_name);
			act (buf, FALSE, ch, 0, victim, TO_VICT | TO_ACT_FORMAT);
			snprintf (buf, MAX_STRING_LENGTH,  "$n teaches $N something.");
			act (buf, FALSE, ch, 0, victim, TO_NOTVICT | TO_ACT_FORMAT);
			return;
		}
	}

	if ( (i = skill_index_lookup (arg1)) == -1) {
		send_to_char("No such skill or craft.\n\r",ch);
		return;
	}


	if ( !real_skill (ch, i) ) {
		send_to_char("You don't know that skill!\n\r",ch);
		return;
	}

	if(ch->skills[i]<50) {
		send_to_char("You don't yet know that skill well enough.\n\r",ch);
		return;
	}

	if ( real_skill (victim, i) ) {
		send_to_char ("They already know that.\n\r", ch);
		return;
	}

	if ( !trigger (ch, argument, TRIG_TEACH) )
		return;

	switch (i) {
		case SKILL_LIGHT_EDGE:
		case SKILL_MEDIUM_EDGE:
		case SKILL_HEAVY_EDGE:
		case SKILL_LIGHT_BLUNT:
		case SKILL_MEDIUM_BLUNT:
		case SKILL_HEAVY_BLUNT:
		case SKILL_LIGHT_PIERCE:
		case SKILL_MEDIUM_PIERCE:
		case SKILL_HEAVY_PIERCE:
		case SKILL_SHORTBOW:
		case SKILL_LONGBOW:
		case SKILL_CROSSBOW:
		case SKILL_THROWN:
		case SKILL_STAFF:
		case SKILL_POLEARM:
			if ( (victim->skills [SKILL_LIGHT_EDGE] ||
				victim->skills [SKILL_MEDIUM_EDGE] ||
				victim->skills [SKILL_HEAVY_EDGE] ||
				victim->skills [SKILL_LIGHT_BLUNT] ||
				victim->skills [SKILL_MEDIUM_BLUNT] ||
				victim->skills [SKILL_HEAVY_BLUNT] ||
				victim->skills [SKILL_LIGHT_PIERCE] ||
				victim->skills [SKILL_MEDIUM_PIERCE] ||
				victim->skills [SKILL_HEAVY_PIERCE] ||
				victim->skills [SKILL_LONGBOW] ||
				victim->skills [SKILL_CROSSBOW] ||
				victim->skills [SKILL_SHORTBOW] ||
				victim->skills [SKILL_THROWN] ||
				victim->skills [SKILL_STAFF] ||
				victim->skills [SKILL_POLEARM]) &&
				victim->offense < MIN_PREREQ ) {
				snprintf (buf, MAX_STRING_LENGTH,  "$N cannot learn %s until $E learns offense better.", skill_data[i].skill_name);
				act (buf, TRUE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);

				snprintf (buf, MAX_STRING_LENGTH,  "$N tries to teach you '%s', but cannot until "
						 "you learn offense better.", skill_data[i].skill_name);
				act (buf, TRUE, victim, 0, ch, TO_CHAR | TO_ACT_FORMAT);
				return;
			}

			open_skill (victim, i);
			break;

		case SKILL_DISARM:
			open_skill (victim, i);
			break;

		case SKILL_PARRY:
		case SKILL_BLOCK:
		case SKILL_PICK:
		case SKILL_SEARCH:
		case SKILL_SCAN:
		case SKILL_CLIMB:
		case SKILL_FORAGE:
		case SKILL_HIDE:
		case SKILL_LISTEN:
			open_skill (victim, i);
			break;
			
		case SKILL_MENTAL_BOLT:
		case SKILL_PRESCIENCE:
		case SKILL_CLAIRVOYANCE:
		case SKILL_EMPATHIC_HEAL:
		case SKILL_DANGER_SENSE:
		case SKILL_TELEPATHY:
		case SKILL_HEX:
		case SKILL_SENSITIVITY:
			send_to_char ("Psionics cannot be taught.\n", ch);
			return;

		case SKILL_HERBALISM:
			if ( !prereq_skill (ch, victim, SKILL_HERBALISM, SKILL_FORAGE, 0) )
				return;
			break;

		case SKILL_ALCHEMY:
			if ( !prereq_skill (ch, victim, SKILL_ALCHEMY, SKILL_HERBALISM, 0) )
				return;
			break;

		case SKILL_SNEAK:
			if ( !prereq_skill (ch, victim, SKILL_SNEAK, SKILL_HIDE, 0) )
				return;
			break;

		case SKILL_DUAL:
			if ( victim->offense < 26 ) {
				send_to_char ("Their offense is too low for them to "
							  "learn dual wield.\n\r",ch);
				return;
			}
			open_skill (victim, i);
			break;

		case SKILL_RITUAL:
			send_to_char ("Ritual can only be learned through conversion.\n\r",
						  ch);
			break;

		case SKILL_BACKSTAB:
			if ( !prereq_skill (ch, victim, SKILL_BACKSTAB,
									SKILL_LIGHT_PIERCE, SKILL_HIDE) )
				return;
			break;

		case SKILL_SKIN:
			if ( !prereq_skill (ch, victim, SKILL_SKIN, 0, 0) )
				return;
			break;
		case SKILL_STEAL:
			if ( !prereq_skill (ch, victim, SKILL_STEAL,
									SKILL_SNEAK, SKILL_HIDE) )
				return;
			break;
		case SKILL_SPEAK_ATLIDUK:
		case SKILL_SPEAK_ADUNAIC:
		case SKILL_SPEAK_HARADAIC:
		case SKILL_SPEAK_WESTRON:
		case SKILL_SPEAK_DUNAEL:
		case SKILL_SPEAK_LABBA:
		case SKILL_SPEAK_NORLIDUK:
		case SKILL_SPEAK_ROHIRRIC:
		case SKILL_SPEAK_TALATHIC:
		case SKILL_SPEAK_UMITIC:
		case SKILL_SPEAK_NAHAIDUK:
		case SKILL_SPEAK_PUKAEL:
		case SKILL_SPEAK_SINDARIN:
		case SKILL_SPEAK_QUENYA:
		case SKILL_SPEAK_SILVAN:
		case SKILL_SPEAK_KHUZDUL:
		case SKILL_SPEAK_ORKISH:
		case SKILL_SPEAK_BLACK_SPEECH:
			break;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "$N teaches you '%s'.", skill_data[i].skill_name);
	act (buf, TRUE, victim, 0, ch, TO_CHAR);
	open_skill (victim, i);

    if ( IS_MORTAL(victim) )
		update_crafts (victim);

	send_to_char("Done.\n\r",ch);
}

void add_memory (CHAR_DATA *add, CHAR_DATA *mob)
{
	struct memory_data	*memory = NULL;
	char				name [MAX_STRING_LENGTH] = {'\0'};

	if ( IS_NPC (add) )
		(void)one_argument (GET_NAMES (add), name);
	else
		strcpy (name, GET_NAME (add));

	for ( memory = mob->remembers; memory; memory = memory->next ) 
		if ( !strcmp (memory->name, name) )
			return;

	CREATE (memory, struct memory_data, 1);
		
	memory->name = add_hash (name);
	memory->next = mob->remembers;

	mob->remembers = memory;
}	

void forget (CHAR_DATA *ch, CHAR_DATA *foe)
{
	struct memory_data		*mem;
	struct memory_data		*tmem;

	if ( !ch->remembers )
		return;

	if ( !strcmp (GET_NAME (foe), ch->remembers->name) ) {
		mem = ch->remembers;
		ch->remembers = ch->remembers->next;
		mem_free (mem);
		return;
	}

	for ( mem = ch->remembers; mem->next; mem = mem->next ) {
		if ( !strcmp (GET_NAME (foe), mem->next->name) ) {
			tmem = mem->next;
			mem->next = tmem->next;
			mem_free (tmem);
			return;
		}
	}
}

void do_forage (CHAR_DATA *ch, char *argument, int cmd)
{
	int			sector_type = 0;

	if ( !real_skill (ch, SKILL_FORAGE) ) {
		send_to_char ("You don't have any idea how to forage!\n\r", ch);
		return;
	}

	if ( is_dark (ch->room) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS) ) {
		send_to_char ("It's too dark to forage.\n\r", ch);
		return;
	}

	

	sector_type = vtor (ch->in_room)->sector_type;

	if ( sector_type != SECT_WOODS &&
		 sector_type != SECT_FOREST &&
		 sector_type != SECT_FIELD &&
		 sector_type != SECT_PASTURE &&
		 sector_type != SECT_HEATH &&
		 sector_type != SECT_HILLS ) {
		send_to_char ("This place is devoid of anything edible.\n\r", ch);
		return;
	}

	send_to_char ("You begin rummaging for something edible.\n\r", ch);
	act ("$n begins examining the flora.", TRUE, ch, 0, 0, TO_ROOM);

	ch->delay_type = DEL_FORAGE;
	ch->delay	    = 25;
	return;
}


void delayed_forage (CHAR_DATA *ch)
{
	OBJ_DATA	*obj = NULL;
	int			seasonal_penalty = 0;
	int			temp = 0;
	int			base_temp = 75;
	int			range = 20;
		
	temp =  weather_info[ch->room->zone].temperature;
	seasonal_penalty = (temp > base_temp) ? (temp - base_temp) : (base_temp - temp );
	seasonal_penalty = (seasonal_penalty > range) ? (seasonal_penalty - range) : 0;

	if ( skill_use (ch, SKILL_FORAGE, seasonal_penalty) ) {

		obj = NULL;
		obj = load_object (forageables[number(0,NUM_FORAGEABLES - 1)]);
		if ( !obj )
			obj = load_object (161);
		if ( obj ) {
			act ("You pick $p.", FALSE, ch, obj, 0, TO_CHAR);
			act ("$n finds $p hidden in the flora.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT );
			obj_to_char (obj, ch);
			
			if ( obj->in_room != NOWHERE )
				act ("$n leaves $p on the the ground.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT );
		} 
		else {
			send_to_char ("You deserved something, but the MUD is bare.\n\r", ch);
			return;
		}

	} 
	else {
		send_to_char ("Your efforts to find food are of no avail.\n\r", ch);
		act ("$n stops searching the flora.", TRUE, ch, 0, 0, TO_ROOM);
	}
	return;
}

int has_a_key (CHAR_DATA *mob)
{
	if ( mob->right_hand && GET_ITEM_TYPE (mob->right_hand) == ITEM_KEY )
		return 1;
	if ( mob->left_hand && GET_ITEM_TYPE (mob->left_hand) == ITEM_KEY )
		return 1;

	return 0;
}

void do_knock (CHAR_DATA *ch, char *argument, int cmd)
{
	int			door;
	int			target_room;
	int			trigger_info;
	int			key;
	char		dir [MAX_STRING_LENGTH];
	char		buf [MAX_STRING_LENGTH];
	char		key_name [MAX_STRING_LENGTH];
	CHAR_DATA	*tch;
	ROOM_DATA	*room;

	argument = one_argument (argument, buf);
	argument = one_argument (argument, dir);

	if ( (door = find_door (ch, buf, dir)) == -1 )
		return;

	if ( !IS_SET (PASSAGE (ch, door)->exit_info, PASSAGE_ISDOOR) ) {
		send_to_char ("That's not a door.\n\r", ch);
		return;
	}

	else if ( !IS_SET (PASSAGE (ch, door)->exit_info, PASSAGE_CLOSED) ) {
		send_to_char ("It's already open!\n\r", ch);
		return;
	}

	target_room = PASSAGE (ch, door)->to_room;

	if ( !vtor (target_room) ) {
		send_to_char ("Actually, that door doesn't lead anywhere.\n\r", ch);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "You hear tapping from the %s.\n\r", dirs [rev_dir [door]]);
	send_to_room (buf, target_room);

	act ("You rap on the door.", FALSE, ch, 0, 0, TO_CHAR);
	act ("$n raps on the door.", FALSE, ch, 0, 0, TO_ROOM);

		/* The trigger command is activated for mobs in the room that ch
		   occupies.  So, we have to switch to the target_room to get the
		   trigger to happen in that room.
		*/

	room = vtor (target_room);

	ch->room = room;
	
	trigger_info = trigger (ch, argument, TRIG_KNOCK);

	ch->room = vtor (ch->in_room);

		/* An unfortunate drag:  If the trigger was activated, that doesn't
		   necessarily mean some mob shouldn't automatically open the door.
		   But, the way this is implemented, no mob will unlock the door
		   if the trigger was activated.
		*/
	if ( !trigger_info )
		return;

	key = room->dir_option [rev_dir [door]]->key;

	for ( tch = room->people; tch; tch = tch->next_in_room ) {

		if ( !IS_NPC (tch) ||
			 !AWAKE (tch) ||
			 GET_POS (tch) == FIGHT ||
			 !CAN_SEE (tch, tch) )			/* Too dark if he can't see self. */
			continue;

		if ( !has_a_key (tch) )
			continue;
		
		if ( !is_brother (ch, tch) &&
			 !(IS_SET (ch->act, ACT_ENFORCER) &&
			   IS_SET (tch->act, ACT_ENFORCER)) )
			continue;

		if ( !IS_SET (room->dir_option[rev_dir[door]]->exit_info, PASSAGE_LOCKED) || has_key (tch, NULL, key) ) {
			(void)one_argument (room->dir_option [rev_dir [door]]->keyword, key_name);
			snprintf (buf, MAX_STRING_LENGTH,  "unlock %s %s", key_name, dirs [rev_dir [door]]);
			command_interpreter (tch, buf);
			snprintf (buf, MAX_STRING_LENGTH,  "open %s %s", key_name, dirs [rev_dir [door]]);
			command_interpreter (tch, buf);
			snprintf (buf, MAX_STRING_LENGTH,  "%s %s", key_name, dirs [rev_dir [door]]);
			add_second_affect (SPA_CLOSE_DOOR, IS_NPC (ch) ? 5 : 10,
							   tch, NULL, buf, 0);
			return;
		}
	}
}

void do_accuse (CHAR_DATA *ch, char *argument, int cmd)
{
	int				hours = -1;
	CHAR_DATA		*victim = NULL;
	AFFECTED_TYPE	*af = NULL;
	char			buf [MAX_STRING_LENGTH];
	char			tmpbuf [MAX_STRING_LENGTH];
	CHAR_DATA		*tmp = NULL;

	/*   accuse { pc | mob } [hours]                                 */

	if ( IS_MORTAL (ch) &&
		 !is_area_leader (ch) ) {
		send_to_char ("You cannot accuse anyone here.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !(victim = get_char_room_vis (ch, buf)) ) {

		victim = get_char (buf);

		if ( !victim ) {

		    /* Let's imms accuse no matter what */
		    if (! IS_MORTAL(ch)) {
				tmp = load_pc(buf);
				
				if(!tmp)  {
			    	send_to_char("No PC with that name.\n", ch);
			    	return;
				}
			
				victim = tmp;
				
		    }
		    else {
		        send_to_char ("Nobody is here by that name.\n", ch);
		        return;
		    }
		}

		if ( IS_NPC (victim) ) {
			send_to_char ("Who?\n", ch);
			return;
		}
	}

		/* Only if the char is logged in */
	    /* unfortunately, is_area_leader() requires the player to
	       be logged in.  That means an imm can set an area leader
	       as wanted.  WHile not perfect, its not unreasonable all
	       in all. */
 
	if ( is_area_leader (victim) ) {
		act ("$N is a leader, you can't accuse $M.",
					FALSE, ch, 0, victim, TO_CHAR);
		if (tmp)
			unload_pc(tmp);
			
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf ) {
		if ( !strtol(buf, NULL, 10)) {
			send_to_char ("What number of hours do you intend to use?\n", ch);
		if (tmp)
			unload_pc(tmp);
			
		return;
		}

		hours = strtol(buf, NULL, 10);
	}
		
	if ( (af = get_affect (victim, MAGIC_CRIM_BASE + ch->room->zone)) ) {

		if ( hours == -1 && af->a.spell.duration == -1 ) {
			snprintf (tmpbuf, MAX_STRING_LENGTH, "%s is already permanently wanted.", victim->short_descr);
			send_to_char(tmpbuf, ch);
			
			if (tmp)
				unload_pc(tmp);
			
			return;
		}

		if ( hours == -1 ) {
			snprintf (tmpbuf, MAX_STRING_LENGTH, "%s is already permanently wanted.", victim->short_descr);
			send_to_char(tmpbuf, ch);
		}

		af->a.spell.duration = hours;
	}
	else {
		magic_add_affect (victim, MAGIC_CRIM_BASE + ch->room->zone,
						  hours, 0, 0, 0, 0);
	}

	if (tmp)
		unload_pc(tmp);
	
	send_to_char ("Ok.\n", ch);
	
	return;
}

void do_pardon (CHAR_DATA *ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA		*td = NULL;
	CHAR_DATA		*victim = NULL;
	AFFECTED_TYPE	*af = NULL;
	char			buf [MAX_STRING_LENGTH];
	char			tmpbuf [MAX_STRING_LENGTH];
	CHAR_DATA		*tmp = NULL;

	if ( IS_MORTAL (ch) &&
		 !is_area_leader (ch) ) {
		send_to_char ("You cannot pardon anyone here.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !(victim = get_char_room_vis (ch, buf)) ) {

		victim = get_char_vis (ch, buf);

		if ( !victim ) {
		    if( !IS_MORTAL(ch)) {
			tmp = load_pc(buf);
			if(!tmp)  {
			    send_to_char("No PC with that name.\n", ch);
			    return;
			}
			victim = tmp;
		    } else {
			
			send_to_char ("Nobody is here by that name.\n\r", ch);
			return;
		    }
		}

		if ( IS_NPC (victim) ) {
			send_to_char ("Who?\n\r", ch);
			return;
		}
	}

	if ( !(af = get_affect (victim, MAGIC_CRIM_BASE + ch->room->zone)) ) {

		if ( IS_SET (victim->act, ACT_PARIAH) ) {
		    snprintf (tmpbuf, MAX_STRING_LENGTH, "%s is a pariah, and cannot be pardoned.", victim->short_descr);
            send_to_char(tmpbuf, ch);

		if (tmp) 
			unload_pc(tmp);
		
		return;
		}

		snprintf (tmpbuf, MAX_STRING_LENGTH, "%s isn't wanted for anything.", 
			victim->short_descr);
		
		send_to_char(tmpbuf, ch);

		if (tmp)
			unload_pc(tmp);
		
		return;
	}

	if ( IS_NPC(victim) ) {
		if ( af->type >= MAGIC_CRIM_BASE + 1 && af->type <= MAGIC_CRIM_BASE + 99 )	
			prisoner_release (victim, af->type - MAGIC_CRIM_BASE);
	}
	else {
		for ( td = descriptor_list; td; td = td->next ) {
			if ( td->connected != CON_PLYNG )
				continue;
			if ( !td->character )
				continue;
			if ( td->character == victim ) {
				if ( af->type >= MAGIC_CRIM_BASE + 1 && af->type <= MAGIC_CRIM_BASE + 99 )	
					prisoner_release (victim, af->type - MAGIC_CRIM_BASE);
				break;
			}
		}
	}

	affect_remove (victim, af);

	if (tmp)
		unload_pc(tmp);
	
	send_to_char ("Ok.\n\r", ch);
	return;
}


void do_nod (CHAR_DATA *ch, char *argument, int cmd)
{
	int			opened_a_door = 0;
	int			dir;
	char		buf [MAX_STRING_LENGTH];
	char		key_name [MAX_STRING_LENGTH];
	CHAR_DATA	*victim;

	argument = one_argument (argument, buf);

        if ( ch->room->virtual == AMPITHEATRE && IS_MORTAL(ch) ) {
                if ( !get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
			!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand) ) {
                        send_to_char ("You decide against making a commotion. PETITION to request to speak.\n", ch);
                        return;
                }
        }

	if ( !*buf ) {
		act ("You nod.", FALSE, ch, 0, 0, TO_CHAR);
		act ("$n nods.", TRUE, ch, 0, 0, TO_ROOM);
		return;
	}

	if ( !(victim = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that person.\n\r", ch);
		return;
	}

	if ( IS_NPC (victim) &&
		 AWAKE (victim) &&
		 !victim->fighting &&
		 CAN_SEE (victim, ch) &&
		 is_brother (ch, victim) &&
		 has_a_key (victim) ) {

		argument = one_argument (argument, buf);

		if ( !*buf ) {
			for ( dir = 0; dir < 6; dir++ ) {

				if ( !PASSAGE (ch, dir) )
					continue;

				if ( IS_SET (PASSAGE (ch, dir)->exit_info, PASSAGE_LOCKED) && !has_key (victim, NULL, PASSAGE (ch, dir)->key) )
					continue;

				(void)one_argument (PASSAGE (victim, dir)->keyword, key_name);
				snprintf (buf, MAX_STRING_LENGTH,  "unlock %s %s", key_name, dirs [dir]);
				command_interpreter (victim, buf);
				snprintf (buf, MAX_STRING_LENGTH,  "open %s %s", key_name, dirs [dir]);
				command_interpreter (victim, buf);
				snprintf (buf, MAX_STRING_LENGTH,  "%s %s", key_name, dirs [dir]);
				add_second_affect (SPA_CLOSE_DOOR, 10,
								   victim, NULL, buf, 0);

				opened_a_door = 1;
			}
		}
		else {
			dir = is_direction (buf);
			if ( dir == -1 || !PASSAGE (ch, dir) ) {
				send_to_char ("There is no exit in that direction.\n", ch);
				return;
			}
			if ( IS_SET (PASSAGE (ch, dir)->exit_info, PASSAGE_LOCKED) && !has_key (victim, NULL, PASSAGE (ch, dir)->key) ){
				act ("$N nods to you.", TRUE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
				act ("You nod to $n.", TRUE, ch, 0, victim, TO_VICT | TO_ACT_FORMAT);
				act ("$N nods to $n.", TRUE, ch, 0, victim, TO_NOTVICT | TO_ACT_FORMAT);
				
				
			}
			else {
				(void)one_argument (PASSAGE (victim, dir)->keyword, key_name);
				snprintf (buf, MAX_STRING_LENGTH,  "unlock %s %s", key_name, dirs [dir]);
				command_interpreter (victim, buf);
				snprintf (buf, MAX_STRING_LENGTH,  "open %s %s", key_name, dirs [dir]);
				command_interpreter (victim, buf);
				snprintf (buf, MAX_STRING_LENGTH,  "%s %s", key_name, dirs [dir]);
				add_second_affect (SPA_CLOSE_DOOR, 10,
								   victim, NULL, buf, 0);

				opened_a_door = 1;
			}
		}

		if ( opened_a_door )
			return;
	}

	act ("You nod to $N.", TRUE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
	act ("$n nods to you.", TRUE, ch, 0, victim, TO_VICT | TO_ACT_FORMAT);
	act ("$n nods to $N.", TRUE, ch, 0, victim, TO_NOTVICT | TO_ACT_FORMAT);
}

void do_camp (CHAR_DATA *ch, char *argument, int cmd)
{
	int		sector_type= 0;
	int		 block = 0;
	struct room_prog *p;

	if ( is_switched (ch) )
		return;
	if ( ch->desc && ch->desc->original ) {
		send_to_char ("Not while switched. Use RETURN first.\n", ch);
		return;
	}

	for ( p = ch->room->prg; p; p = p->next )
		if ( !str_cmp (p->keys, "lean-to leanto") && !str_cmp (p->command, "enter") )
			block++;

	if ( block ) {
		send_to_char ("Someone else appears to have already made camp here.\n", ch);
		return;
	}

	if ( IS_MORTAL (ch) && IS_SET (ch->room->room_flags, SAFE_Q) ) {
		delayed_camp4 (ch);
		return;
	}

	sector_type = ch->room->sector_type;

	if ( port == BUILDER_PORT || port == TEST_PORT ) {
		send_to_char ("Camping is not allowed on this port.\n", ch);
		return;
	}


        if ( sector_type != SECT_WOODS && sector_type != SECT_FOREST &&
                 sector_type != SECT_FIELD && sector_type != SECT_HILLS ) {
                send_to_char ("You can only camp in the woods, forest, a "
                                          "field or the hills.\n\r", ch);
                return;
        }

/*
	send_to_char ("You'll need to pitch a tent or find suitable shelter.\n", ch);
	return;
*/

	send_to_char ("You search for a suitable location to build a lean-to.\n\r", ch);
	act ("$n begins looking around in the brush.", TRUE, ch, 0, 0, TO_ROOM);

	ch->delay_type = DEL_CAMP1;
	ch->delay	   = 30;
	return;
}

void delayed_camp1 (CHAR_DATA *ch)
{
	send_to_char ("Finding a safe location behind a tree, you begin constructing\n"
		      "a frame out of fallen branches, found nearby.\n", ch);
	act ("$n starts constructing a frame using stripped\ntree branches.",
			FALSE, ch, 0, 0, TO_ROOM);

	ch->delay_type = DEL_CAMP2;
	ch->delay	   = 30;
	return;
}

void delayed_camp2 (CHAR_DATA *ch)
{
	send_to_char ("You begin sewing leafy branches to the frame with long pieces\n"
		      "of saw grass.\n", ch);

	act ("$n begins sewing leafy branches with grass\nto the lean-to's frame.",
					FALSE, ch, 0, 0, TO_ROOM);

    ch->delay_type = DEL_CAMP3;
	ch->delay      = 30;
	return;
}

void delayed_camp3 (CHAR_DATA *ch)
{
	ROOM_DATA		*room = ch->room;

	send_to_char ("Finally finished, you enter your lean-to, carefully adjusting\n"
		      "its covering of leaves and bark along the way.\n", ch);

	act ("Finished, $n enters $s lean-to.", TRUE, ch, 0, 0, TO_ROOM);

	SET_BIT (room->room_flags, SAFE_Q);
	do_quit (ch, "", 1);
	REMOVE_BIT (room->room_flags, SAFE_Q);
	return;
}

void delayed_camp4 (CHAR_DATA *ch)
{
	do_quit (ch, "", 0);
	return;
}

void knock_out (CHAR_DATA *ch, int seconds)
{
	SECOND_AFFECT		*sa;

	if ( GET_POS (ch) > SLEEP ) {
		send_to_char ("You stagger to the ground, losing consciousness!\n\r", ch);
		act ("$n staggers to the ground, losing consciousness.", FALSE, ch, 0, 0, TO_ROOM);
		GET_POS (ch) = SLEEP;
	}

	if ( (sa = get_second_affect (ch, SPA_KNOCK_OUT, NULL)) ) {
		if ( sa->seconds < seconds )
			sa->seconds = seconds;
		return;
	}

	add_second_affect (SPA_KNOCK_OUT, seconds, ch, NULL, NULL, 0);
}

void do_tables (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA		*obj = NULL;
	CHAR_DATA		*tmp = NULL;
	AFFECTED_TYPE	*af_table = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	int				table_count = 0;
	char			*temp_arg = NULL;
	
	send_to_char ("\n", ch);

	for ( obj = ch->room->contents; obj; obj = obj->next_content ) {

		if ( GET_ITEM_TYPE (obj) != ITEM_CONTAINER ||
			 !IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE) )
			continue;

		table_count++;

		temp_arg = obj_desc (obj);
		snprintf (buf, MAX_STRING_LENGTH,  "#6%s#0\n", temp_arg);

		for ( tmp = ch->room->people; tmp; tmp = tmp->next_in_room ) {
			af_table = get_affect (tmp, MAGIC_SIT_TABLE);
			if ( af_table && is_at_table (tmp, obj) && tmp != ch ) {
				temp_arg = char_short(tmp);
				snprintf (buf2, MAX_STRING_LENGTH,  "    #5%s#0 is seated here.\n", CAP(temp_arg));
				strcat (buf, buf2);
			}
		}

		send_to_char (buf, ch);
	}

	if ( !table_count )
		send_to_char ("   None.\n", ch);

	return;
}


int is_toll_paid (CHAR_DATA *ch, int dir)
{
	CHAR_DATA			*collector;
	AFFECTED_TYPE		*af;

	if ( !is_human (ch) )
		return 1;

	if ( !(collector = levy_collector (ch->room, dir)) )
		return 1;

	if ( is_brother (collector, ch) )
		return 1;

	if ( !CAN_SEE (collector, ch) )
		return 1;

	if ( !(af = get_affect (ch, MAGIC_TOLL_PAID)) )
		return 0;

	if ( af->a.toll.dir == dir &&
		 af->a.toll.room_num == ch->in_room )
		return 1;

	return 0;
}

void do_pay (CHAR_DATA *ch, char *argument, int cmd)
{
		/* pay
		   pay [<char | direction>] # [all | char]
		*/

	int				max_pay = 0;
	int				num_payees = 0;
	int				dir = -1;
	int				seen_tollkeeper = 0;
	int				currency_type = 0;
	int				i;
	CHAR_DATA		*collector;
	CHAR_DATA		*tch;
	CHAR_DATA		*payees [50];
	AFFECTED_TYPE	*af;
	AFFECTED_TYPE	*af_collector;
	char			buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		for ( dir = 0; dir < 6; dir++ ) {
			if ( !(tch = levy_collector (ch->room, dir)) ||
				 tch == ch )
				continue;

			af = get_affect (tch, MAGIC_TOLL);

            seen_tollkeeper = 1;

            snprintf (buf, MAX_STRING_LENGTH,  "$n wants %d token%s to cross %s.",
						  af->a.toll.charge,
						  af->a.toll.charge == 1 ? "" : "s",
                          dirs [af->a.toll.dir]);
            act (buf, FALSE, tch, 0, ch, TO_VICT);
        }

        if ( !seen_tollkeeper )
           	send_to_char ("There are no toll keepers here.\n", ch);
		return;
	}

	if ( (dir = index_lookup (dirs, buf)) != -1 ) {
		collector = levy_collector (ch->room, dir);
		if ( !collector ) {
			send_to_char ("There is nobody here collecting tolls that way.\n",
						  ch);
			return;
		}
	}

	else if ( (collector = get_char_room_vis (ch, buf)) ) {
		if ( !(af = get_affect (collector, MAGIC_TOLL)) ) {
			act ("$N isn't collecting a toll.\n",
					FALSE, ch, 0, collector, TO_CHAR);
			return; 
		}

		dir = af->a.toll.dir;
	}

	else if ( !just_a_number (buf) ) {
		send_to_char ("You don't see that person collecting tolls.\n", ch);
		return;
	}

	else {			/* A number WITHOUT a char/dir */
		max_pay = strtol(buf, NULL, 10);

		for ( dir = 0; dir < 6; dir++ )
			if ( (collector = levy_collector (ch->room, dir)) )
				break;

		if ( !collector ) {
			send_to_char ("Nobody is collecting tolls here.\n", ch);
			return;
		}
	}

	if ( GET_POS (collector) <= SLEEP ) {
		act ("$N isn't conscious.", FALSE, ch, 0, collector, TO_CHAR);
		return;
	}

	if ( GET_POS (collector) == FIGHT ) {
		act ("$N is fighting!", FALSE, ch, 0, collector, TO_CHAR);
		return;
	}

			/* We know dir and who to pay now (collector).  If we don't know
               how much to pay, figure that out. */

	if ( !max_pay ) {
		argument = one_argument (argument, buf);

		if ( !just_a_number (buf) ) {
			send_to_char ("How much are you willing to pay?\n", ch);
			return;
		}

		max_pay = strtol(buf, NULL, 10);
	}

	af_collector = get_affect (collector, MAGIC_TOLL);

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "for") )
		argument = one_argument (argument, buf);

	if ( !*buf ) {

		if ( (af = get_affect (ch, MAGIC_TOLL_PAID)) &&
			 af->a.toll.dir == dir ) {
			send_to_char ("You already have permission to cross.\n", ch);
			return;
		}

		payees [num_payees++] = ch;
	}

	else if ( !str_cmp (buf, "all") ) {

		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( !is_human (tch) )
				continue;

			if ( tch != ch )
				 if ( tch->deleted || 
				 	  tch->following != ch ||
				 	  !CAN_SEE (ch, tch) )
					continue;

			if ( (af = get_affect (tch, MAGIC_TOLL_PAID)) &&
				 af->a.toll.dir == dir )
				continue;

			payees [num_payees++] = tch;
		}

		if ( !num_payees ) {
			send_to_char ("There is nobody to pay for.\n", ch);
			return;
		}
	}

	else if ( (tch = get_char_room_vis (ch, buf)) ) {

		if ( !is_human (tch) ) {
			send_to_char ("Only humans need pay tolls.\n", ch);
			return;
		}

		if ( (af = get_affect (tch, MAGIC_TOLL_PAID)) &&
			 af->a.toll.dir == dir ) {
			act ("$N already has permission to cross.",
					FALSE, ch, 0, tch, TO_CHAR);
			return;
		}

		payees [num_payees++] = tch;
	}

	else {
		send_to_char ("Pay for whom?\n", ch);
		return;
	}

	if ( af_collector->a.toll.charge > max_pay ) {
		snprintf (buf, MAX_STRING_LENGTH,  "$N wants %d token%s per toll.",
					  af_collector->a.toll.charge,
					  af_collector->a.toll.charge == 1 ? "" : "s");
		act (buf, FALSE, ch, 0, collector, TO_CHAR);
		return;
	}

	if ( !collector->mob )
		currency_type = 0;
		
	if ( collector->mob->currency_type > 1 )
		currency_type = 0;
	else currency_type = 1;

	if ( !can_subtract_money (ch, af_collector->a.toll.charge * num_payees, currency_type) ) {
		send_to_char ("You don't have enough coin to pay the toll.\n", ch);
		return;
	}

	act ("$n pays $N a toll for:", FALSE, ch, 0, collector, TO_NOTVICT);

	snprintf (buf, MAX_STRING_LENGTH,  "$n pays you a toll of %d token%s for:",
				  af_collector->a.toll.charge * num_payees,
				  af_collector->a.toll.charge * num_payees == 1 ? "" : "s");
	act (buf, FALSE, ch, 0, collector, TO_VICT);

	snprintf (buf, MAX_STRING_LENGTH,  "You pay $N a toll of %d token%s for:",
				  af_collector->a.toll.charge * num_payees,
				  af_collector->a.toll.charge * num_payees == 1 ? "" : "s");
	act (buf, FALSE, ch, 0, collector, TO_CHAR);

	for ( i = 0; i < num_payees; i++ ) {
		if ( ch == payees [i] )
			act ("   yourself", FALSE, ch, 0, 0, TO_CHAR);
		else
			act ("   you", FALSE, payees [i], 0, 0, TO_CHAR);

		act ("   $n", FALSE, payees [i], 0, ch, TO_ROOM);

		magic_add_affect (payees [i], MAGIC_TOLL_PAID, -1, 0, 0, 0, 0);

		af = get_affect (payees [i], MAGIC_TOLL_PAID);
		af->a.toll.dir = dir;
		af->a.toll.room_num = ch->in_room;
	}

	subtract_money (ch, af_collector->a.toll.charge * num_payees, currency_type);
}


void do_accept (CHAR_DATA *ch, char *argument, int cmd)
{
	if (  (ch->delay_type != DEL_APP_APPROVE &&
		 ch->delay_type != DEL_INVITE &&
		 ch->delay_type != DEL_PURCHASE_ITEM) ) {
		send_to_char ("No transaction is pending.\n\r", ch);
		return;
	}

	if ( ch->delay_type == DEL_APP_APPROVE ) {
		answer_application (ch, argument, 345);
		return;
	}
	else if ( ch->delay_type == DEL_INVITE ) {
		invite_accept (ch, argument);
		return;
	}
	else if ( ch->delay_type == DEL_PURCHASE_ITEM ) {
		do_buy (ch, "", 2);
		return;
	}
	else send_to_char ("No transaction is pending.\n\r", ch);
}

void activate_refresh (CHAR_DATA *ch)
{
        ch->move = GET_MAX_MOVE (ch);
}

void do_decline (struct char_data *ch, char *argument, int cmd)
{
		if ( ch->delay_type != DEL_PURCHASE_ITEM &&
		ch->delay_type != DEL_APP_APPROVE &&
		ch->delay_type != DEL_INVITE ) {
		send_to_char ("No transaction is pending.\n\r", ch);
		return;
	}

	if ( ch->delay_type == DEL_INVITE ) {
		break_delay (ch);
		return;
	}

	if ( ch->delay_type == DEL_APP_APPROVE ) {
		answer_application (ch, argument, 0);
		return;
	}

	if ( ch->delay_type == DEL_PURCHASE_ITEM ) {
		break_delay (ch);
		return;
	}

	if ( ch->delay_who ) {
		mem_free (ch->delay_who);
		ch->delay_who = NULL;
	}

	ch->delay = 0;
}
