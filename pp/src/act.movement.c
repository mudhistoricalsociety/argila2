/** 
*	\file act.movement.c
*	Movement in all forms for players
*
*	The purpose of this module is to provied functionality for movement in all
*   forms, handles followers, mounts, tracks, and temporary dwellings
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
#include <unistd.h>
#include <sys/stat.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

int drowned (CHAR_DATA *ch)
{
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	double	damage = 0.0;
	int	roll = 0;

	if ( !IS_MORTAL (ch) || number(0,9) ) {
		return 0;
	}

	roll = number(1,4);

	if ( roll == 1 )
		act ("Searing needles of pain lance through your rapidly-dimming field of vision as your lungs burn from the lack of air.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	else if ( roll == 2 )
		act ("Your vision blurs as your lungs scream for air, your heart pounding painfully in your chest.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	else if ( roll == 3 )
		act ("A numbing, chilly dullness settles in upon your mind as it slowly dies from the lack of air.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	else if ( roll == 4 )
		act ("Your limbs and chest burn in agony, and you instinctively open your mouth to gasp for air, only to choke on icy water as it fills your lungs.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

	act ("Thrashing sluggishly, $n struggles in vain against the water.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	/** Saps both fatigue AND bloodloss points - drowning is bad news; saps fatigue faster, so it immobilizes the player, while sapping general damage more slowly to repesent how agonizing a death drowning can be.**/

	damage = (double)number(15,35);
	damage /= 100;
	damage = (ch->max_move * damage);

	if ( ch->move < (int) damage )
		ch->move = 0;
	else ch->move -= (int) damage;

	damage = (double)number(2,15);
	damage /= 100;
	damage = (ch->max_hit * damage);

	if ( damage > 0.0 && general_damage (ch, (int) damage) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Drowned: %s Room: %d", ch->tname, ch->in_room);
		system_log (buf, TRUE);
		return 1;
	}

	return 0;
}

int swimming_check (CHAR_DATA *ch)
{
	ROOM_DIRECTION_DATA		*exit_room;
	ROOM_DATA			*troom = NULL;
	float				encumb_percent = 0.0;
	double				damage = 0.0;
	bool				encumbered = FALSE, exhausted = FALSE;

	if ( ch->room->sector_type != SECT_RIVER && ch->room->sector_type != SECT_LAKE &&
		ch->room->sector_type != SECT_OCEAN && ch->room->sector_type != SECT_REEF &&
		ch->room->sector_type != SECT_UNDERWATER )
		return 0;

	if ( !IS_MORTAL (ch) )
		return 0;

	/* IS_ENCUMBERED() won't work, since we want to check if the PC is 'lightly encumbered' or above, not fully encumbered or worse. */

	encumbered = (GET_STR (ch) * enc_tab [0].str_mult_wt < IS_CARRYING_W (ch));

	exhausted = (ch->move <= 10);

        encumb_percent = 1.0 - (float) (CAN_CARRY_W (ch) - IS_CARRYING_W (ch)) / (float) CAN_CARRY_W (ch);
	encumb_percent *= 100;

	if ( !encumb_percent )
		encumb_percent = 1;

	encumb_percent = (int)(encumb_percent * 4.5);

	if ( !encumbered && !exhausted )
		return 0;

	if ( skill_use (ch, SKILL_SWIMMING, (int)encumb_percent) ) {
		if ( ch->room->sector_type != SECT_UNDERWATER && encumbered )
			send_to_char ("You manage to tread water, staying afloat despite your encumberance. . .\n", ch);
		return 0;
	}

	exit_room = PASSAGE (ch, DIR_DOWN);
	if ( exit_room ) {
		troom = vtor (exit_room->to_room);
		if ( troom && troom->sector_type != SECT_UNDERWATER )
			troom = NULL;
	}

	if ( !encumbered && !exhausted ) { /* Gradual fatigue sap for swimmers failing their initial skill check. */
		damage = (double)number(3,15);
		damage /= 100;
		damage = (ch->max_move * damage);
		if ( ch->move < (int) damage ) {
			damage -= ch->move;
			ch->move = 0;
			if ( damage > 0.0 && general_damage (ch, (int) damage) )
				return 1;
		}
		else ch->move -= (int) damage;
	}
	else if ( ch->room->sector_type != SECT_UNDERWATER ) {
		if ( (number(1,100) > (int)encumb_percent) && (!troom || !exhausted) ) {
			send_to_char ("You splutter and choke, sinking briefly beneath the surface. . .\n", ch);
			act ("Spluttering and choking, $n sinks briefly beneath the water's surface.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

			/** Saved check for encumbered, non-exhausted swimmer (or an exhausted swimmer who cannot sink any further) inflicts more fatigue damage, eventually inflicting bloodloss damage when they're totally exhausted and have no more fatigue to sap.**/

			damage = (double)number(3,25);
			damage /= 100;
			damage = (ch->max_move * damage);
			if ( ch->move < (int) damage ) {
				damage -= ch->move;
				ch->move = 0;
				if ( damage > 0.0 && general_damage (ch, (int) damage) )
					return 1;
			}
			else ch->move -= (int) damage;
		}
		else if ( troom ) {

			/* Failed check for encumbered swimmer, or exhaustion, sends them further down. */

			if ( encumbered ) {
				send_to_char ("Struggling against the weight of your equipment, you sink below the surface. . .\n", ch);
				act ("Struggling, $n sinks from sight beneath the water's surface.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			}
			else if ( exhausted ) {
				send_to_char ("Utterly exhausted, you sink below the surface. . .\n", ch);
				act ("Looking utterly exhausted, $n sinks from sight beneath the water's surface.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			}
			send_to_char ("\n", ch);
			char_from_room (ch);
			char_to_room (ch, troom->virtual);
			do_look (ch, "", 0);
		}
	}
	else {
		if ( (number(1,100) > (int)encumb_percent) && (!troom || !exhausted) ) {
			;				/** Handle underwater asphyxiation in a different function; only check for a failed swim skillcheck to see if they sink any further due to encumberance. **/
		}
		else if ( troom ) {
			if ( encumbered ) {
				send_to_char ("You continue to sink helplessly beneath the weight of your gear. . .\n", ch);
				act ("Struggling helplessly, $n continues to sink further downward.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			}
			else if ( exhausted ) {
				send_to_char ("Too exhausted to fight it, you sink even further into the water. . .\n", ch);
				act ("Struggling weakly, looking exhausted, $n sinks further downward.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			}
			send_to_char ("\n", ch);
			char_from_room (ch);
			char_to_room (ch, troom->virtual);
			do_look (ch, "", 0);
		}
	}

	return 0;
}

void do_pitch (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			sector_type = 0;
	char		*temp_arg = NULL;
	
	if ( !*argument ) {
		send_to_char ("What did you wish to pitch?\n", ch);
		return;
	}

	if ( !(obj = get_obj_in_list_vis (ch, argument, ch->room->contents)) ) {
		send_to_char ("I don't see that here.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (obj) != ITEM_TENT ) {
		send_to_char ("Only tents need to be pitched.\n", ch);
		return;
	}

	sector_type = ch->room->sector_type;

        if ( sector_type != SECT_WOODS && sector_type != SECT_FOREST &&
                 sector_type != SECT_FIELD && sector_type != SECT_HILLS ) {
                send_to_char ("You may only camp in the woods, forest, a field or the hills.\n\r", ch);
                return;
        }

	temp_arg = obj_short_desc (obj);
	snprintf (buf, MAX_STRING_LENGTH,  "You begin pitching #2%s#0.", temp_arg);
	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

	temp_arg = obj_short_desc (obj);
	snprintf (buf, MAX_STRING_LENGTH,  "$n begins pitching #2%s#0.", temp_arg);
	act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

        ch->delay_type 	= DEL_PITCH;
	if ( IS_MORTAL(ch) )
       		ch->delay       = 30;
	else ch->delay = 1;
	ch->delay_obj	= obj;
	
	return;
}

void delayed_pitch (CHAR_DATA *ch)
{
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	obj = ch->delay_obj;

	temp_arg = obj_short_desc (obj);
	snprintf (buf, MAX_STRING_LENGTH,  "You finish pitching #2%s#0.", temp_arg);
	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

	temp_arg = obj_short_desc (obj);
	snprintf (buf, MAX_STRING_LENGTH,  "$n finishes pitching #2%s#0.", temp_arg);
	act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	SET_BIT (obj->obj_flags.extra_flags, ITEM_PITCHED);

	temp_arg = obj_short_desc (obj);
	snprintf (buf, MAX_STRING_LENGTH,  "%s has been erected here.", temp_arg);
	buf[0] = toupper(buf[0]);

	obj->description = add_hash (buf);

	ch->delay_type = 0;
	ch->delay = 0;
	ch->delay_obj = 0;
	
	return;
}

void do_dismantle (CHAR_DATA *ch, char *argument, int cmd)
{
	ROOM_DATA	*room = NULL;
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	if ( !*argument ) {
		send_to_char ("What did you wish to dismantle?\n", ch);
		return;
	}

	if ( !(obj = get_obj_in_list_vis (ch, argument, ch->room->contents)) ) {
		send_to_char ("I don't see that here.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (obj) != ITEM_TENT || !IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHED) ) {
		send_to_char ("Only pitched tents can be dismantled.\n", ch);
		return;
	}

	if ( (room = vtor (obj->o.od.value[0])) ) {
		if ( room->contents ) {
			send_to_char ("You'll need to clear out the inside of the tent before dismantling it.\n", ch);
			return;
		}
		if ( room->people ) {
			send_to_char ("You cannot dismantle the tent while there are still people inside!\n", ch);
			return;
		}
		room->occupants = 0;
	}

	temp_arg = obj_short_desc (obj);
	snprintf (buf, MAX_STRING_LENGTH,  "You dismantle #2%s#0.", temp_arg);
	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	
	temp_arg = obj_short_desc (obj);
	snprintf (buf, MAX_STRING_LENGTH,  "$n dismantles #2%s#0.", temp_arg);
	act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	REMOVE_BIT (obj->obj_flags.extra_flags, ITEM_VNPC);
	REMOVE_BIT (obj->obj_flags.extra_flags, ITEM_PITCHED);

	obj->o.od.value[0] = 0;

	if ( vtoo (obj->virtual) )
		obj->description = add_hash (vtoo(obj->virtual)->description);
	else {
		temp_arg = obj_short_desc (obj);
		snprintf (buf, MAX_STRING_LENGTH,  "%s is here.", temp_arg);
		buf [0] = toupper(buf[0]);
		obj->description = add_hash (buf);
	}
	
	return;
}

void do_outside (CHAR_DATA *ch, char *argument, int cmd)
{
	do_leave (ch, "", 0);
	return;
}

OBJ_DATA *find_dwelling_obj (int dwelling_room)
{
	ROOM_DATA	*room = NULL;
	OBJ_DATA	*tobj = NULL;

	for ( room = full_room_list; room; room = room->lnext ) {
		for ( tobj = room->contents; tobj; tobj = tobj->next_content ) {
			if ( GET_ITEM_TYPE (tobj) != ITEM_DWELLING && GET_ITEM_TYPE (tobj) != ITEM_TENT )
				continue;
			if ( tobj->o.od.value[0] == dwelling_room )
				return tobj;
		}
	}

	return NULL;
}

ROOM_DATA *generate_dwelling_room (OBJ_DATA *dwelling)
{
	ROOM_DATA	*room = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			target = 0;
	char		*temp_arg = NULL;

	if ( dwelling->o.od.value[0] ) {
		if ( (room = vtor (dwelling->o.od.value[0])) )
			return room;
		else
			room = allocate_room (dwelling->o.od.value[0]);
	}
	else {
		target = 10000 + dwelling->coldload_id;
		while ( (room = vtor(target)) )
			target++;
		room = allocate_room (100000 + target);
		dwelling->o.od.value[0] = room->virtual;
	}

	SET_BIT (room->room_flags, SAFE_Q);
	SET_BIT (room->room_flags, INDOORS);
	SET_BIT (room->room_flags, LIGHT);

	temp_arg = obj_short_desc (dwelling);
	snprintf (buf, MAX_STRING_LENGTH,  "Inside %s", temp_arg);
	room->name = add_hash (buf);

	if ( dwelling->indoor_desc && *dwelling->indoor_desc && str_cmp (dwelling->indoor_desc, "(null)") )
		snprintf (buf, MAX_STRING_LENGTH,  "%s", dwelling->indoor_desc);
	else{
		temp_arg = obj_short_desc(dwelling); 
		snprintf (buf, MAX_STRING_LENGTH,  "   You find yourself inside %s.\n", temp_arg);
	}

	room->description = add_hash (buf);

	room->entrance = dwelling->in_room;

	return room;
}

void track_from_room (ROOM_DATA *room, TRACK_DATA *track)
{
        TRACK_DATA      *temptrack;

        if ( !room || !track )
                return;

        if ( room->tracks == track )
                room->tracks = room->tracks->next;
                        
        else {
            for (temptrack = room->tracks; temptrack; temptrack = temptrack->next)
            	if (temptrack->next == track)
            		temptrack->next = temptrack->next->next;
        }
        
	mem_free (track);
}

void track_expiration (ROOM_DATA *room, TRACK_DATA *track)
{
/** TODO  What is the purpose of limit and how should it be used??? ***/
	int		limit = 0;

	if ( room->sector_type == SECT_INSIDE || room->sector_type == SECT_CITY ||
	     room->sector_type == SECT_ROAD )
		limit = 4;
	
	else if ( room->sector_type == SECT_TRAIL )
		limit = 8;
	
	else if ( room->sector_type == SECT_MOUNTAIN || room->sector_type == SECT_SWAMP )
		limit = 12;
	
	else 
		limit = 16;

	track_from_room (room, track);
	
	return;
}

void update_room_tracks (void)
{
	ROOM_DATA		*room = NULL;
	TRACK_DATA		*track = NULL;
	TRACK_DATA		*next_track = NULL;

	for ( room = full_room_list; room; room = room->lnext ) {
		for ( track = room->tracks; track; track = next_track ) {
			next_track = track->next;
			track->hours_passed++;
			if ( weather_info[room->zone].state == LIGHT_RAIN )
				track->hours_passed++;
			else if ( weather_info[room->zone].state == STEADY_RAIN )
				track->hours_passed += 2;
			else if ( weather_info[room->zone].state == HEAVY_RAIN )
				track->hours_passed += 3;
			else if ( weather_info[room->zone].state == LIGHT_SNOW )
				track->hours_passed += 2;
			else if ( weather_info[room->zone].state == STEADY_SNOW )
				track->hours_passed += 4;
			else if ( weather_info[room->zone].state == HEAVY_SNOW )
				track->hours_passed += 6;
			track_expiration (room, track);
		}
	}
	return;
}

#define TRACK_LIMIT_PER_ROOM		5

void leave_tracks (CHAR_DATA *ch, int to_dir, int from_dir)
{
	TRACK_DATA	*track = NULL;
	WOUND_DATA	*wound = NULL;
	int			bleeding = 0, i = 0;

	if ( !ch || !ch->room )
		return;

	if ( !IS_MORTAL (ch) )
		return;

	if ( IS_SET (ch->act, ACT_FLYING) )
		return;

	if ( ch->room->sector_type == SECT_ROAD || ch->room->sector_type == SECT_CITY )
		return;

		/** amimals too small, flying, swimming or otherwise leave no tracks **/
	if ( ch->race == RACE_BAT 	|| ch->race == RACE_BIRD ||
		 ch->race == RACE_GOOSE || ch->race == RACE_FERRET ||
		 ch->race == RACE_FOX   || ch->race == RACE_FROG ||
		 ch->race == RACE_INSECT || ch->race == RACE_MOUSE ||
		 ch->race == RACE_RAT ||
		 ch->race == RACE_TOAD )
		return;

	for ( wound = ch->wounds; wound; wound = wound->next ) {
		bleeding += wound->bleeding;
	}

	CREATE (track, TRACK_DATA, 1);

	track->race = ch->race;
	track->to_dir = to_dir;
	track->from_dir = from_dir;
	track->speed = ch->speed;
	track->flags = 0;
	track->next = NULL;

	if ( bleeding > 2 )
		SET_BIT (track->flags, BLOODY_TRACK);
	if ( !IS_NPC (ch) )
		SET_BIT (track->flags, PC_TRACK);

	if ( ch->room->tracks )
		track->next = ch->room->tracks;

	ch->room->tracks = track;

	for ( track = ch->room->tracks; track; track = track->next ) {
		i++;
		if ( i >= TRACK_LIMIT_PER_ROOM ) {
			while ( track->next ){
				track_from_room (ch->room, track->next);
			}
			break;
		}
	}
	return;
}

void clear_pmote(CHAR_DATA *ch) 
{
	if(ch->pmote_str) {
    	mem_free(ch->pmote_str);
        ch->pmote_str = (char *)NULL;
    }
    
    return;
}

int check_climb (CHAR_DATA *ch)
{
	if ( get_affect (ch, MAGIC_AFFECT_LEVITATE) )
		return 1;

	if ( IS_SET (ch->act, ACT_FLYING) )
		return 1;

	if ( !IS_MORTAL (ch) )
		return 1;

	if ( !CAN_GO (ch, DIR_DOWN) )
		return 1;

	if ( IS_SET (ch->room->room_flags, FALL) ) {
		send_to_char ("You plummet down!\n", ch);
		act ("$n FALLS!", FALSE, ch, 0, 0, TO_ROOM);
		return 0;
	}

	if ( !IS_SET (ch->room->room_flags, CLIMB) )
		return 1;

	if ( IS_ENCUMBERED (ch) ) {
		send_to_char ("\nYou are too encumbered to maintain your "
					  "balance!  YOU FALL!\n\n", ch);
		act ("$n FALLS!", FALSE, ch, 0, 0, TO_ROOM);
		return 0;
	}

	if ( !skill_use (ch, SKILL_CLIMB, 0) ) {
		send_to_char ("\nYou lose your footing and FALL!\n\n", ch);
		act ("$n FALLS!", FALSE, ch, 0, 0, TO_ROOM);
		return 0;
	}

	return 1;
}

void do_swim (CHAR_DATA *ch, char *argument, int cmd)
{
	int			dir = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( (dir = index_lookup (dirs, buf)) == -1 ) {
		send_to_char ("Swim in which direction?\n", ch);
		return;
	}

	if ( IS_RIDER (ch) ) {
		send_to_char ("Dismount first.\n", ch);
		return;
	}
	else if ( IS_RIDEE (ch) ) {
		send_to_char ("Your rider must dismount first.\n", ch);
		return;
	}

	do_move (ch, "swim", dir);
	
	return;
}

int clear_current_move (CHAR_DATA *ch)
{
	CHAR_DATA	*tch = NULL;
	QE_DATA		*qe = NULL;
	QE_DATA		*tqe = NULL;

	if ( GET_FLAG (ch, FLAG_ENTERING) )
		return 0;

	for ( qe = quarter_event_list; qe; qe = qe->next )
		if ( qe->ch == ch )
			break;

	if ( !qe )
		return 0;

	if ( qe == quarter_event_list )
		quarter_event_list = qe->next;
	else {

		for ( tqe = quarter_event_list; tqe->next != qe; tqe = tqe->next )
			; /* cycling through the quarter-events */

		tqe->next = qe->next;
	}

	REMOVE_BIT (ch->flags, FLAG_LEAVING);
	if (qe->travel_str) mem_free (qe->travel_str);
	mem_free (qe);

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {

		if ( tch == ch || tch->deleted )
			continue;

		if ( tch->following == ch && CAN_SEE (tch, ch) ) {
			clear_moves (tch);
			clear_current_move (tch);
		}
	}

	return 1;
}

void stop_mount_train (CHAR_DATA *stopper)
{
	CHAR_DATA		*ch = NULL;
	CHAR_DATA		*leader = NULL;
	AFFECTED_TYPE	*af = NULL;
	MOVE_DATA		*move = NULL;

	ch = stopper;
	leader = stopper;

	while ( IS_HITCHEE (ch) ) {
		ch = ch->hitcher;
		leader = ch;
	}

	if ( !ch ) {
		printf ("Hey, ch is NULL!\n");
		return;
	}

	while ( ch ) {

		while ( ch->moves ) {
			move = ch->moves;
			ch->moves = move->next;
			if (move->travel_str) 
			  mem_free (move->travel_str);
			mem_free (move);
		}

		if ( (af = get_affect (ch, MAGIC_DRAGGER)) )
			affect_remove (ch, af);

		if ( IS_HITCHER (ch) )
			ch = ch->hitchee;
		else
			ch = NULL;
	}

	if ( stopper == leader )
		send_to_char ("You prevent your hitches from moving.\n", leader);
	else
		act ("You prevent the hitches led by $N from moving.",
			TRUE, stopper, 0, leader, TO_CHAR);

	act ("$N prevents your hitches from moving.",
			TRUE, leader, 0, stopper, TO_CHAR);
	act ("The hitches led by $n are stopped by $N.",
			FALSE, leader, 0, stopper, TO_NOTVICT);
	
	return;
}

void clear_moves (CHAR_DATA *ch)
{
	MOVE_DATA		*move = NULL;
	AFFECTED_TYPE	*af = NULL;

	while ( ch->moves ) {
		move = ch->moves;
		ch->moves = move->next;
		if (move->travel_str) 
		  mem_free (move->travel_str);
		mem_free (move);
	}

	if ( move && (IS_HITCHER (ch) || IS_HITCHEE (ch)) ) {
		stop_mount_train (ch);
		return;
	}

	REMOVE_BIT (ch->flags, FLAG_ENTERING);
	REMOVE_BIT (ch->flags, FLAG_LEAVING);

	if ( (af = get_affect (ch, MAGIC_DRAGGER)) )
		affect_remove (ch, af);
	return;
}

void add_to_qe_list (QE_DATA *qe)
{
	QE_DATA			*tqe = NULL;

	if ( !quarter_event_list ||
		 quarter_event_list->event_time > qe->event_time ) {
		qe->next = quarter_event_list;
		quarter_event_list = qe;
		return;
	}

	for ( tqe = quarter_event_list; tqe->next; tqe = tqe->next )
		if ( tqe->next->event_time > qe->event_time )
			break;

	qe->next = tqe->next;
	tqe->next = qe;
	
	return;
}

int is_moving_toward (CHAR_DATA *target, int dir, CHAR_DATA *archer)
{
	int		line = 0;

	if ( !target || !archer )
		return 0;

	line = track (target, archer->in_room);

	if ( line == dir )
		return 1;

	return 0;
}

int is_moving_away (CHAR_DATA *target, int dir, CHAR_DATA *archer)
{
	ROOM_DATA	*troom = NULL;
	int		line = 0;
	int		i = 0;
	int		range = 0;

	if ( !target || !archer )
		return 0;

	troom = vtor(target->room->dir_option[dir]->to_room);

	line = track (archer, troom->virtual);

	if ( line == dir ) {
		range = is_archer(archer);
		if ( !range )
			return 0;
		troom = archer->room;
		if ( !troom->dir_option[dir] )
			return 0;
		troom = vtor (troom->dir_option[dir]->to_room);
		if ( troom->virtual == target->room->dir_option[dir]->to_room )
			return 1;
		for ( i = 1; i <= range; i++ ) {
			if ( troom->virtual == target->room->dir_option[dir]->to_room )
				return 1;
			if ( !troom->dir_option[dir] )
				break;
			troom = vtor(troom->dir_option[dir]->to_room);
		}
	}

	return 0;
}

void enter_room (QE_DATA *qe)
{
	CHAR_DATA			*victim = NULL;
	OBJ_DATA			*obj = NULL;
	float				encumb_percent = 0.0;
	int					from_dir = 0;
	int					rooms_fallen = 0;
	int					temp;
	int					roomnum;
	int					echo = 0;
	int					died = 0;
	int					i = 0;
	int					sensed = 0;
	int					dir = 0;
	int					observed = 0;
	ROOM_DATA			*next_room = NULL;
	ROOM_DATA			*prevroom = NULL;
	ROOM_DIRECTION_DATA *room_exit = NULL;
	ROOM_DIRECTION_DATA	*exit_test = NULL;
	CHAR_DATA			*ch = NULL;
	CHAR_DATA			*tch = NULL;
	CHAR_DATA			*tmp_ch = NULL;
	AFFECTED_TYPE		*af = NULL;
	AFFECTED_TYPE		*drag_af = NULL;
	CHAR_DATA			*temp_char = NULL;
	char				*temp_arg = NULL;
	
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char        travel_str [MAX_STRING_LENGTH] = {'\0'};
	char		*direction[] = { "south", "west", "north", "east", "down", "up" };
	char		*direction2[] = { "north", "east", "south", "west", "up", "down" };
	char		*rev_d[] = { "the south", "the west", "the north", "the east",
							 "below", "above" };
    char        *dir_names [] = {"to the north", "to the east", "to the south",
    							 "to the west", "above", "below" };
	bool	toward = FALSE, away = FALSE;

	ch = qe->ch;

	if ( !ch ) {
	        if (qe->travel_str) mem_free (qe->travel_str);
		mem_free (qe);
		return;
	}

	if ( !is_he_there (ch, qe->from_room) ) {

		if ( is_he_somewhere (ch) )
			clear_moves (ch);
                if (qe->travel_str) mem_free (qe->travel_str);
		mem_free (qe);

		return;
	}

	if ( !is_toll_paid (ch, qe->dir) ) {
		snprintf (buf, MAX_STRING_LENGTH, "You must pay a toll before crossing %s.", dirs [qe->dir]);
		act (buf, FALSE, ch, 0, 0, TO_CHAR);
		temp_char = levy_collector (ch->room, qe->dir);
		act ("$n is stopped by $N.",
				TRUE, ch, 0, temp_char, TO_NOTVICT);
		
		temp_char = levy_collector (ch->room, qe->dir);
		act ("You stop $N for toll.", FALSE, temp_char, 0, ch, TO_CHAR);
		clear_moves (ch);
		if (qe->travel_str) mem_free (qe->travel_str);
		mem_free (qe);
		return;
	}

	if ( IS_SWIMMING (ch) &&
		 IS_SET (ch->flags, FLAG_LEAVING) &&
		 ch->room->sector_type == SECT_UNDERWATER &&
		 IS_ENCUMBERED (ch) &&
		 IS_MORTAL(ch) ) {
        
        encumb_percent = 1.0 - (float) (CAN_CARRY_W (ch) - IS_CARRYING_W (ch)) / (float) CAN_CARRY_W (ch);
        encumb_percent *= 100;
        encumb_percent = (int)(encumb_percent * 4.5);

		if ( IS_DROWNING(ch) )
			encumb_percent = (int)(encumb_percent * 2.5);

		if ( !skill_use (ch, SKILL_SWIMMING, (int)encumb_percent) ) {
			if ( IS_ENCUMBERED (ch) )
				send_to_char ("The sheer weight of your equipment prevents you from making progress.\n", ch);
			else
				send_to_char ("Your leaden limbs refuse to propel your exhausted body in that direction!\n", ch);
			clear_moves (ch);
			if (qe->travel_str) mem_free (qe->travel_str);
			mem_free (qe);
			return;
		}
	}

	remove_affect_type (ch, MAGIC_TOLL_PAID);

	if ( (af = get_affect (ch, AFFECT_SHADOW)) )
		af->a.shadow.edge = -1;

	travel_str[0] = '\0';
	
	if ( qe && qe->travel_str ) {
	        strcpy(travel_str, qe->travel_str);
	}
	else if ( ch->travel_str != NULL ) {
	        snprintf(travel_str, MAX_STRING_LENGTH, ", %s", ch->travel_str);
	}

	if ( IS_SET (qe->flags, MF_ARRIVAL) ) {

		REMOVE_BIT (ch->flags, FLAG_ENTERING);

		if ( IS_SET (ch->flags, FLAG_LEAVING) )
			printf ("Leaving still set!!\n");

		if ( (af = get_affect (ch, MAGIC_DRAGGER)) &&
			 is_he_here (ch, (CHAR_DATA *) af->a.spell.t, 0) &&
			 number (0, 1) )
			do_wake (ch, GET_NAME ((CHAR_DATA *) af->a.spell.t), 1);

		if ( af )
			affect_remove (ch, af);

		remove_affect_type (ch, MAGIC_SNEAK);

		if ( ch->moves )
			initiate_move (ch);

		if (qe->travel_str)
			mem_free (qe->travel_str);
		
		mem_free (qe);
		qe = NULL;

		return;
	}

	if ( IS_SET (qe->flags, MF_TOEDGE) ) {

		if ( !(af = get_affect (ch, AFFECT_SHADOW)) ) {
			magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

			af = get_affect (ch, AFFECT_SHADOW);

			af->a.shadow.shadow = NULL;
		}
		
		af->a.shadow.edge = qe->dir;

		REMOVE_BIT (ch->flags, FLAG_LEAVING);

		
		if (af->a.shadow.shadow &&
			!is_he_somewhere (af->a.shadow.shadow) )
			send_to_char ("You can no longer see who you are shadowing.\n", ch);
			
		else if (af->a.shadow.shadow &&
				!could_see (ch, af->a.shadow.shadow) ) {
			temp_arg = char_short (af->a.shadow.shadow);
			snprintf (buf, MAX_STRING_LENGTH,  "You lose sight of #5%s#0.", temp_arg );
			send_to_char (buf, ch);
		}

		if ( ch->moves ) {
		        if (qe->travel_str) mem_free (qe->travel_str);
			mem_free (qe);
			initiate_move (ch);
		} else {
			snprintf (buf, MAX_STRING_LENGTH,  "$n stops just %s.", dir_names [qe->dir]);
			act (buf, FALSE, ch, 0, 0, TO_ROOM);
			if (qe->travel_str) mem_free (qe->travel_str);
			mem_free (qe);
		}

		return;
	}

	room_exit = PASSAGE (ch, qe->dir);

	if ( !room_exit ) {

		snprintf (buf, MAX_STRING_LENGTH,  "There doesn't appear to be an exit %s.\n",
					  dirs [qe->dir]);
		send_to_char (buf, ch);

		clear_moves (ch);
		REMOVE_BIT (ch->flags, FLAG_LEAVING);
		if (qe->travel_str) mem_free (qe->travel_str);
		mem_free (qe);

		return;
	}

	if ( !IS_SET (qe->flags, MF_PASSDOOR) &&
		 IS_SET (room_exit->exit_info, PASSAGE_CLOSED) ) {

		if ( room_exit->keyword && strlen (room_exit->keyword) ) {
			temp_arg = fname (room_exit->keyword);
			snprintf (buf, MAX_STRING_LENGTH,  "The %s seems to be closed.\n",
						 temp_arg);
			send_to_char (buf, ch);
		}

		else
			send_to_char ("It seems to be closed.\n", ch);

		temp_arg = fname (room_exit->keyword);
		snprintf (buf, MAX_STRING_LENGTH,  "$n stops at the closed %s.", temp_arg);
		act (buf, TRUE, ch, 0, 0, TO_ROOM);

		clear_moves (ch);
		REMOVE_BIT (ch->flags, FLAG_LEAVING);
		if (qe->travel_str) mem_free (qe->travel_str);
		mem_free (qe);

		return;
	}

	REMOVE_BIT (ch->flags, FLAG_LEAVING);

	if ( ch->targeted_by ) {
		if ( !(toward = is_moving_toward (ch, qe->dir, ch->targeted_by)) && !(away = is_moving_away (ch, qe->dir, ch->targeted_by)) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "You lose sight of your quarry as they move %sward.\n", direction2[qe->dir]);
			send_to_char (buf, ch->targeted_by);
			ch->targeted_by->aiming_at = NULL;
			ch->targeted_by->aim = 0;
			ch->targeted_by = NULL;
		}
		else if ( toward ) {
			snprintf (buf, MAX_STRING_LENGTH,  "You carefully hold your aim as your quarry moves closer...\n");
			send_to_char (buf, ch->targeted_by);
		}
		else if ( away ) {
			snprintf (buf, MAX_STRING_LENGTH,  "You carefully hold your aim as your quarry moves away...\n");
			send_to_char (buf, ch->targeted_by);
		}
		else {
			snprintf (buf, MAX_STRING_LENGTH,  "You lose sight of your quarry.\n");
			send_to_char (buf, ch->targeted_by);
			ch->targeted_by->aiming_at = NULL;
			ch->targeted_by->aim = 0;
			ch->targeted_by = NULL;
		}
	}

	if ( (drag_af = get_affect (ch, MAGIC_DRAGGER)) &&
		 is_he_here (ch, (CHAR_DATA *) drag_af->a.spell.t, 1) && 
		 GET_POS ((CHAR_DATA *) drag_af->a.spell.t) <= SLEEP ) {
		char_from_room ((CHAR_DATA *) drag_af->a.spell.t);
		char_to_room ((CHAR_DATA *) drag_af->a.spell.t, room_exit->to_room);
	} else
		drag_af = NULL;

	if ( IS_SUBDUER (ch) ) {
		char_from_room (ch->subdue);
		char_to_room (ch->subdue, room_exit->to_room);
	}

	if ( IS_MOUNT (ch) && ch->mount && ch->mount->subdue ) {
		char_from_room (ch->mount->subdue);
		char_to_room (ch->mount->subdue, room_exit->to_room);
		do_look (ch->mount->subdue, "", 0);
	}

	temp = GET_POS (ch);
	GET_POS (ch) = SLEEP;		/* Hack to make him not hear messages */

	shadowers_shadow (ch, room_exit->to_room, qe->dir);

	GET_POS (ch) = temp;

	if ( is_mounted (ch) ) {
		char_from_room (ch->mount);
		char_to_room (ch->mount, room_exit->to_room);
	}

	roomnum = ch->in_room;
	prevroom = vtor (roomnum);
	leave_tracks (ch, qe->dir, ch->from_dir);
	char_from_room (ch);

	if ((prevroom->sector_type == SECT_LEANTO)
	     && prevroom->deity <= 15) {
		while ( prevroom->contents ) {
			obj = prevroom->contents;
			obj_from_room (&obj, 0);
			obj_to_room (obj, room_exit->to_room);
			if (echo == 0) send_to_char("You take your belongings with you as you leave.\n", ch);
			echo = 1;
		}
	}

	char_to_room (ch, room_exit->to_room);

	if ( qe->dir == 0 )
		from_dir = 2;
	else if ( qe->dir == 1 )
		from_dir = 3;
	else if ( qe->dir == 2 )
		from_dir = 0;
	else if ( qe->dir == 3 )
		from_dir = 1;
	else if ( qe->dir == 4 )
		from_dir = 5;
	else from_dir = 4;

	ch->from_dir = from_dir;

	if ( !IS_NPC(ch) && !IS_SET (ch->act, ACT_VEHICLE) )
		weaken (ch, 0, -qe->move_cost, NULL);
	else
		weaken (ch, 0, -qe->move_cost/5, NULL);

	do_look (ch, "", 0);

	if ( IS_MORTAL(ch) && ch->speed == SPEED_IMMORTAL ) {
		if ( (prevroom->sector_type != SECT_LAKE && prevroom->sector_type != SECT_RIVER &&
			prevroom->sector_type != SECT_OCEAN && prevroom->sector_type != SECT_REEF &&
			prevroom->sector_type != SECT_UNDERWATER) && SWIM_ONLY (ch->room) ) {
			send_to_char ("\n", ch);
			act ("You enter the water, and begin attempting to swim. . .", FALSE, ch, 0, 0, TO_CHAR);
		}
		else if ( prevroom->sector_type == SECT_UNDERWATER && ch->room->sector_type == SECT_UNDERWATER ) {
			send_to_char ("\n", ch);
			act ("You continue your swim beneath the water's surface. . .", FALSE, ch, 0, 0, TO_CHAR);
		}
		else if ( (prevroom->sector_type == SECT_LAKE || prevroom->sector_type == SECT_RIVER ||
			prevroom->sector_type == SECT_OCEAN || prevroom->sector_type == SECT_REEF ||
			prevroom->sector_type == SECT_UNDERWATER) && (ch->room->sector_type != SECT_LAKE &&
			ch->room->sector_type != SECT_RIVER && ch->room->sector_type != SECT_OCEAN &&
			ch->room->sector_type != SECT_REEF && ch->room->sector_type != SECT_UNDERWATER) ) {
			send_to_char ("\n", ch);
			act ("You climb from the water, dripping.", FALSE, ch, 0, 0, TO_CHAR);
		}
	}
	
/*
	if ( !IS_SET (ch->act, ACT_PREY) && !IS_NPC (ch) ) {
		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( !IS_NPC (tch) || tch->desc )
				continue;
			if ( !IS_SET (tch->act, ACT_PREY) )
				continue;
			if ( get_affect (tch, MAGIC_HIDDEN) )
				continue;
			if ( !CAN_SEE (tch, ch) || (!CAN_SEE (ch, tch) && CAN_SEE (tch, ch)) )
				continue;
			evade_attacker (tch, -1);
			add_threat (tch, ch, 7);
		}
	}
*/
	while ( ch->sighted )
		ch->sighted = ch->sighted->next;	/* Remove list of target sightings. */

	for ( dir = 0; dir <= 5; dir++ ) {
		if ( !(exit_test = PASSAGE (ch, dir)) || !(next_room = vtor (exit_test->to_room)) )
			continue;
		sensed = 0;
		for ( tch = next_room->people; tch; tch = tch->next_in_room ) {
			if ( real_skill (tch, SKILL_DANGER_SENSE) && !is_brother(ch,tch) && IS_MORTAL(tch) && IS_SET (ch->act, ACT_AGGRESSIVE)) {
				if ( skill_use (tch, SKILL_DANGER_SENSE, 20) ) {
					snprintf (buf, MAX_STRING_LENGTH,  "The hairs on the back of your neck prickle as you glance %sward.\n", direction[dir]);
					send_to_char (buf, tch);
				}
			}
			if ( skill_use (ch, SKILL_DANGER_SENSE, 15) && IS_SET (tch->act, ACT_AGGRESSIVE) && !is_brother (tch, ch) && !sensed) {
				snprintf (buf, MAX_STRING_LENGTH,  "A sense of foreboding fills you as you glance %sward.\n", direction2[dir]);
				send_to_char (buf, ch);
				sensed++;
			}
		}
	}

	if ( is_mounted (ch) )
		do_look (ch->mount, "", 0);

	if ( IS_SUBDUER (ch) )
		do_look (ch->subdue, "", 0);

	SET_BIT (qe->flags, MF_ARRIVAL);
	SET_BIT (ch->flags, FLAG_ENTERING);
	qe->event_time = qe->arrive_time;
	qe->from_room = ch->room;

	if ( qe->event_time <= 0 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "$n arrives from %s.", rev_d [qe->dir]);
		act (buf, TRUE, ch, 0, 0, TO_ROOM);
		enter_room (qe);
		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
       	 		if ( is_area_enforcer (tch) && enforcer (tch, ch, 1, 0) > 0 ) {
                		continue;
                	}
		}
	} else {
		if ( is_mounted (ch) && IS_RIDEE (ch) ) {

				/* ch->mount is the rider */

			snprintf (buf, MAX_STRING_LENGTH,  "$n %ss $N in from %s.",
					mount_speeds [ch->mount->pc ? ch->mount->pc->mount_speed :
												  ch->speed],
					rev_d [qe->dir]);
			act (buf, FALSE, ch->mount, 0, ch, TO_NOTVICT);
		}

		else {
			if ( IS_SET (qe->flags, MF_SNEAK) &&
				 would_reveal (ch) &&
				 odds_sqrt (ch->skills [SKILL_SNEAK]) < number (1, 100) ) {
				observed = 0;
				for ( tmp_ch = ch->room->people; tmp_ch; tmp_ch = tmp_ch->next_in_room ) {
					if ( tmp_ch != ch && CAN_SEE (ch, tmp_ch) ) {
						observed = 1;
						break;
					}
				}
				if ( observed )
					send_to_char ("You are observed.\n", ch);
				remove_affect_type (ch, MAGIC_HIDDEN);
				remove_affect_type (ch, MAGIC_SNEAK);
				REMOVE_BIT (qe->flags, MF_SNEAK);
			}

			if ( !IS_SET (qe->flags, MF_SNEAK) ) {

				if ( !IS_SET (ch->act, ACT_FLYING) )
					snprintf (buf, MAX_STRING_LENGTH,  "$n is entering from %s%s.", rev_d [qe->dir],
								 ( drag_af ? ", dragging $N" : travel_str));
				else
					snprintf (buf, MAX_STRING_LENGTH,  "$n flies in from %s%s.", rev_d [qe->dir],
								  drag_af ? ", dragging $N" : "");
				act (buf, TRUE, ch, 0, drag_af ?
							(CHAR_DATA *) drag_af->a.spell.t : 0, TO_NOTVICT);
			} else {
				snprintf (buf, MAX_STRING_LENGTH,  "[%s sneaking in from %s.]", ch->tname,
							  rev_d [qe->dir]);
				act (buf, TRUE, ch, 0, 0, TO_NOTVICT | TO_IMMS);
			}
		}

		add_to_qe_list (qe);
	}

	if ( IS_SUBDUER (ch) )
		act ("$n has $N in tow.", TRUE, ch, 0, ch->subdue, TO_NOTVICT);

	if ( !check_climb (ch) ) {

		clear_moves (ch);
		clear_current_move (ch);

		while ( IS_SET (ch->room->room_flags, CLIMB) ||
				IS_SET (ch->room->room_flags, FALL) ) {

			rooms_fallen++;

			room_exit = PASSAGE (ch, DIR_DOWN);

			if ( !room_exit )
				break;

			act ("$n plummets down!", TRUE, ch, 0, 0, TO_ROOM);
			send_to_char ("\n\nYou plummet down!\n\n", ch);

			char_from_room (ch);
			char_to_room (ch, room_exit->to_room);

			do_look (ch, "", 0);
		}

		if ( SWIM_ONLY (ch->room) ) {
			send_to_char ("You land with a splash!\n", ch);
			act ("$n lands with a splash!", TRUE, ch, 0, 0, TO_ROOM);
		} else {
			send_to_char ("You land with a thud.\n", ch);
			act ("$n lands with a thud!", FALSE, ch, 0, 0, TO_ROOM);
		}

		for ( i = 1; i <= number(1,5)*rooms_fallen; i++ ) {
			temp_arg = figure_location(ch, 1);
			died += wound_to_char (ch, temp_arg, rooms_fallen * number(1,5), 3, 0, 0, 0);
			if ( died )
				break;
		}

		if ( !died && !SWIM_ONLY (ch->room) )
			knock_out (ch, 15);

		if ( drag_af ) {
			victim = (CHAR_DATA *) drag_af->a.spell.t;

			if ( victim && victim->in_room ) {
				char_from_room (victim);
				char_to_room (victim, room_exit->to_room);
			}
			
			victim = NULL;
		}
	}

	else if ( ch->following )
		follower_catchup (ch);

	if ( ch->aiming_at ) {
		send_to_char ("You lose your aim as you move.\n", ch);
		ch->aiming_at->targeted_by = NULL;
		ch->aiming_at = NULL;
		ch->aim = 0;
	}
	
	return;
}

void process_quarter_events (void)
{
	QE_DATA		*qe = NULL;

	for ( qe = quarter_event_list; qe; qe = qe->next )
		qe->event_time--;

	while ( quarter_event_list && quarter_event_list->event_time <= 0 ) {

		qe = quarter_event_list;

		quarter_event_list = qe->next;

		enter_room (qe);
	}
	
	return;
}

void exit_room (CHAR_DATA *ch, int dir, int flags, int leave_time,
				int arrive_time, int speed_name, int needed_movement, char *travel_str)
{
	CHAR_DATA	*mount = NULL;
	CHAR_DATA	*rider = NULL;
	QE_DATA		*qe = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*speed_type_names [] = {
					"walking", "wandering", "slowly walking", "jogging",
					"running", "sprinting" };
	char		buf1 [MAX_STRING_LENGTH] = {'\0'};

	SET_BIT (ch->flags, FLAG_LEAVING);

	CREATE (qe, QE_DATA, 1);

	qe->ch = ch;
	qe->dir = dir;
	qe->speed_type = speed_name;
	qe->flags = flags;
	qe->from_room = ch->room;
	qe->event_time = leave_time;
	qe->arrive_time = arrive_time;
	qe->move_cost = needed_movement;
	qe->next = NULL;
	qe->travel_str = travel_str;

	if ( ch->speed == SPEED_IMMORTAL ) {
		snprintf (buf1, MAX_STRING_LENGTH,  "$n leaves in a flash %s.", dirs [dir]);
		snprintf (buf, MAX_STRING_LENGTH,  "You blast %sward.\n", dirs [dir]);
		send_to_char (buf, ch);
		act (buf1, TRUE, ch, 0, 0, TO_ROOM);
	} else {

		if ( IS_SUBDUER (ch) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "$n starts %s %s, dragging $N along.",
					  speed_type_names [ch->speed], dirs [dir]);
			act (buf, TRUE, ch, 0, ch->subdue, TO_NOTVICT);
		}

		if ( is_mounted (ch) && IS_RIDEE (ch) ) {

			mount = ch;
			rider = ch->mount;

			snprintf (buf, MAX_STRING_LENGTH,  "You make $N %s %s.",
				  mount_speeds [rider->pc ? rider->pc->mount_speed :
											mount->speed],
				  dirs [dir]);
			act (buf, FALSE, rider, 0, mount, TO_CHAR);

			snprintf (buf, MAX_STRING_LENGTH,  "$n starts %s $N %s.",
				  mount_speeds_ing [rider->pc ? rider->pc->mount_speed :
												mount->speed],
				  dirs [dir]);
			act (buf, FALSE, rider, 0, mount, TO_NOTVICT);
		}

		if (IS_SET (flags, MF_SNEAK) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "[%s starts sneaking %s.]",
					  ch->tname, dirs [dir]);
			act (buf, TRUE, ch, 0, 0, TO_NOTVICT | TO_IMMS);
		}

	}

	if ( qe->event_time <= 0 ) {
		enter_room (qe);
		return;
	}

	add_to_qe_list (qe);

	hitches_follow (ch, dir, leave_time, arrive_time);
	followers_follow (ch, dir, leave_time, arrive_time);
	
	return;
}

int calc_movement_charge (CHAR_DATA *ch, int dir, int wanted_time, int flags, int *speed, int *speed_name, float *walk_time)
{
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	int					needed_movement = 0;
	float				cost_modifier = 1.00;
	float				encumb_percent = 1.00;
	float				t = 0;
	AFFECTED_TYPE		*dragger = NULL;
	ROOM_DATA			*target_room = NULL;
	ROOM_DIRECTION_DATA	*room_exit = NULL;

	room_exit = PASSAGE (ch, dir);

	if ( !room_exit || !(target_room = vtor (room_exit->to_room)) )
		return 0;

	if ( dir == DIR_UP &&
		 IS_SET (target_room->room_flags, FALL) &&
		 !get_affect (ch, MAGIC_AFFECT_LEVITATE) && !IS_SET (ch->act, ACT_FLYING) )
		return 0;

	if ( dir == DIR_UP &&
		 IS_SET (target_room->room_flags, CLIMB) &&
		 IS_ENCUMBERED (ch) &&
		 !get_affect (ch, MAGIC_AFFECT_LEVITATE) && !IS_SET (ch->act, ACT_FLYING) )
		return 0;

    		/* TOEDGE means were in the room and just want to get to the edge. */

	if ( IS_SET (flags, MF_TOEDGE) )
		needed_movement = movement_loss [ch->room->sector_type] / 2;

		/* TONEXT_EDGE means we're about to walk into the room and cross it
	           completely to the other edge. */

	else if ( IS_SET (flags, MF_TONEXT_EDGE) )
		needed_movement = movement_loss [target_room->sector_type];

		/* Otherwise, we're just walking from midpoint to midpoint */

	else
		needed_movement = (movement_loss [ch->room->sector_type] +
						   movement_loss [target_room->sector_type]) / 2;

	if ( (dragger = get_affect (ch, MAGIC_DRAGGER)) )
		cost_modifier *= 1.50;
	else if ( IS_SUBDUER (ch) )
		cost_modifier *= 1.50;

	encumb_percent = 1.0 - (float) (CAN_CARRY_W (ch) -
				IS_CARRYING_W (ch)) / (float) CAN_CARRY_W (ch);

	if ( encumb_percent > 0.95 ) {
		printf ("Huh?  %f\n",
	           (float) (CAN_CARRY_W (ch) - IS_CARRYING_W (ch)) /
							(float) CAN_CARRY_W (ch));
		printf ("Very encumbered pc %s in room %d\n", ch->name, ch->in_room);
		snprintf (buf, MAX_STRING_LENGTH,  "You are carrying too much to move.\n");
		send_to_char (buf, ch);
		clear_moves (ch);
		return -1;
	}

	/* 4 pulses per second.  5 seconds/room for a 13 agi N/PC */

	*walk_time = 0;

	*walk_time = 2 * 13.0 * 5.0 / (GET_AGI (ch) ? GET_AGI (ch) : 13);

	if ( !wanted_time )
		wanted_time = (int)(*walk_time * move_speeds [ch->speed]);

	if ( *walk_time == 0 )
		*walk_time = 9.0;

	if ( get_affect (ch, MAGIC_AFFECT_SLOW) )
		*walk_time *= 1.5;

	if ( get_affect (ch, MAGIC_AFFECT_SPEED) )
		*walk_time *= 0.5;

	if ( (SWIM_ONLY (ch->room) || SWIM_ONLY(target_room)) && !IS_SET (ch->act, ACT_FLYING) ) {
		*walk_time *= (1.45 - ((float)(ch->skills [SKILL_SWIMMING]/100)) );
		cost_modifier *= (1.45 - ((float)(ch->skills [SKILL_SWIMMING]/100)) );
		skill_use (ch, SKILL_SWIMMING, 0);
	}

	t = (float)(wanted_time / *walk_time);

	if      ( t < 2.50 && encumb_percent > 0.80 )	 t = 2.50;
	else if ( t < 1.60 && encumb_percent > 0.70 )	 t = 1.60;
	else if ( t < 1.00 && encumb_percent > 0.50 )	 t = 1.00;
	else if ( t < 0.66 && encumb_percent > 0.40 )	 t = 0.66;
	else if ( t < 0.50 && encumb_percent > 0.30 )	 t = 0.50;
	else if ( t < 0.33 ) t = 0.33;

	if      ( t > 1.60001 )	*speed_name = SPEED_CRAWL;
	else if ( t > 1.00001 )	*speed_name = SPEED_PACED;
	else if ( t > 0.66001 )	*speed_name = SPEED_WALK;
	else if ( t > 0.50001 )	*speed_name = SPEED_JOG;
	else if ( t > 0.33001 )	*speed_name = SPEED_RUN;
	else *speed_name = SPEED_SPRINT;

	if ( IS_SWIMMING (ch) && !IS_SET (ch->act, ACT_FLYING) ) {
		t = 1.60;
		*speed_name = SPEED_SWIM;
	}

	if ( encumb_percent > .50 && encumb_percent <= .60 )
		encumb_percent *= 1.25;
	else if ( encumb_percent > .60 && encumb_percent <= .75 )
		encumb_percent *= 2.0;
	else if ( encumb_percent > .75 )
		encumb_percent *= 3.75;

	cost_modifier = cost_modifier / t;
	cost_modifier = cost_modifier * (1.0 + encumb_percent);
	*speed = (int)(*walk_time * t);

	if ( GET_TRUST (ch) && ch->speed == SPEED_IMMORTAL ) {
		*speed_name = SPEED_IMMORTAL;
		needed_movement = 0;
		*speed = 0;
	}

	if ( IS_RIDER (ch) )
		needed_movement = 0;

	if ( IS_SET (ch->act, ACT_MOUNT) )
		needed_movement *= 0.33;

	if ( *speed < 0 )
		*speed = 1;

	return (int) (needed_movement * cost_modifier);
}

void initiate_move (CHAR_DATA *ch)
{
	int					dir = 0;
	int					flags = 0;
	int					needed_movement = 0;
	int					exit_speed = 0;			/* Actually, time */
	int					enter_speed = 0;		/* Actually, time */
	float				walk_time = 0;
	int					speed = 0;
	int					wanted_time = 0;
	int					speed_name = 0;
	CHAR_DATA			*tch = NULL;
	AFFECTED_TYPE		*af = NULL;
	MOVE_DATA			*move = NULL;
	ROOM_DATA			*target_room = NULL;
	ROOM_DIRECTION_DATA	*room_exit = NULL;
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				*move_names [] = {
							  "walking", "wandering", "slowly walking",
							  "jogging", "running", "sprinting", "blasting",
							  "swimming" };
	char				*move_names2 [] = {
							  "walk", "wander", "pace",
							  "jog", "run", "sprint", "blast",
							  "swim" };
	char				buf1 [MAX_STRING_LENGTH] = {'\0'};
	char                travel_str [MAX_STRING_LENGTH] = "";
	char				*temp_arg = NULL;
	

	if ( !ch->moves ) {
		printf ("Nothing to initiate!\n");
		return;
	}

	if ( IS_SET (ch->act, ACT_VEHICLE) ) {
		move_names [ch->speed] = "traveling";
		move_names2 [ch->speed] = "travel";
	}

	if ( ch->race == RACE_BAT ||
		 ch->race == RACE_BIRD  ) {
		move_names [ch->speed] = "flying";
		move_names2 [ch->speed] = "flie";	/* ("flies") */
	}
	else if ( ch->race == RACE_SERPENT || ch->race == RACE_SNAKE) {
		move_names [ch->speed] = "slithering";
		move_names2 [ch->speed] = "slither";
	}
	else if ( ch->race == RACE_FISH ) {
		move_names [ch->speed] = "swimming";
		move_names2 [ch->speed] = "swim";
	}

	if ( IS_HITCHEE (ch) ) {

		if ( !GET_FLAG (ch->hitcher, FLAG_LEAVING) ) {

			if ( IS_RIDEE (ch) )
				act ("Your mount is hitched to $N.",
						TRUE, ch->mount, 0, ch->hitcher, TO_CHAR);

			act ("You can't move while hitched to $N.",
						TRUE, ch, 0, ch->hitcher, TO_CHAR);

			clear_moves (ch);
			return;
		}
	}

	if ( ch->fighting ) {

		dir = ch->moves->dir;

			/* A "PURSUING" mob will follow from a fight after one second */

			/* Non-sentinel aggro/enforcer mobs now follow by default, if morale check is made. */

		if ( (IS_SET (ch->act, ACT_PURSUE) || (!morale_broken(ch) && (IS_SET (ch->act, ACT_AGGRESSIVE) 
			|| (IS_SET (ch->act, ACT_ENFORCER) && !IS_SET (ch->act, ACT_SENTINEL))))) &&
			 ch->following &&
			 is_he_here (ch, ch->following, TRUE) &&
			 GET_FLAG (ch->following, FLAG_FLEE) ) {

			stop_fighting (ch);

			for ( tch = ch->room->people; tch; tch = tch->next_in_room )
				if ( tch->fighting == ch &&
					 tch != ch &&
					 GET_HIT (tch) > 0 ) {
					set_fighting (ch, tch);
					return;
				}

			add_second_affect (SPA_MOVE, 1, ch, NULL, NULL, dir);
		}

		return;
	}

	if ( GET_POS (ch) != STAND ) {
		send_to_char ("You'll need to stand in order to move.\n", ch);
		clear_moves (ch);
		return;
	}

	move = ch->moves;
	ch->moves = move->next;

	dir = move->dir;
	flags = move->flags;
	wanted_time = move->desired_time;
	if (move->travel_str) {
	  snprintf(travel_str, MAX_STRING_LENGTH,", %s",move->travel_str);
	  mem_free (move->travel_str);
	}
	
	mem_free (move);

	room_exit = PASSAGE (ch, dir);

	if ( !room_exit || !(target_room = vtor (room_exit->to_room)) ) {

		if ( ch->room->extra && ch->room->extra->alas [dir] )
			send_to_char (ch->room->extra->alas [dir], ch);
		else
			send_to_char ("Alas, you cannot go that way...\n", ch);

		clear_moves (ch);
		return;
	}

	if ( dir == DIR_UP &&
		 IS_SET (target_room->room_flags, FALL) &&
		 !get_affect (ch, MAGIC_AFFECT_LEVITATE) && !IS_SET (ch->act, ACT_FLYING) ) {
		send_to_char ("Too steep.  You can't climb up.\n", ch);
		clear_moves (ch);
		return;
	}

	if ( dir == DIR_UP &&
		 IS_SET (target_room->room_flags, CLIMB) &&
		 IS_ENCUMBERED (ch) &&
		 !get_affect (ch, MAGIC_AFFECT_LEVITATE) && !IS_SET (ch->act, ACT_FLYING) ) {
		send_to_char ("You're too encumbered to climb up.\n", ch);
		clear_moves (ch);
		return;
	}

	if ( IS_SET (ch->act, ACT_MOUNT) &&
		 IS_SET (target_room->room_flags, NO_MOUNT) ) {
		act ("You can't go there.", TRUE, ch, 0, 0, TO_CHAR);

		if ( IS_RIDEE (ch) )
			act ("$N can't go there.", TRUE, ch->mount, 0, ch, TO_CHAR);

		if ( IS_HITCHEE (ch) )
			act ("$N can't go there.", TRUE, ch->hitcher, 0, ch, TO_CHAR);

		clear_moves (ch);
		return;
	}

	if ( IsGuarded (ch->room, dir) && (IS_MORTAL(ch) || IS_NPC(ch)) ) {
		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( tch == ch )
				continue;
			if ( !CAN_SEE (tch, ch) )
				continue;
			if ( IS_SET (ch->act, ACT_FLYING) && !IS_SET (tch->act, ACT_FLYING) )
				continue;
			if ( (af = get_affect (tch, AFFECT_GUARD_DIR)) ) {
				if ( af->a.shadow.edge == dir ) {
					if ( get_affect (tch, MAGIC_HIDDEN) ) {
						remove_affect_type (tch, MAGIC_HIDDEN);
						act ("$N emerges from hiding and moves to block your egress in that direction.", TRUE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
						act ("$n attempts to move past you, but you emerge from hiding and intercept $m.", TRUE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
					}
					else {
						act ("$N moves to block your egress in that direction.", TRUE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
						act ("$n attempts to move past you, but you intercept $m.", TRUE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
					}
					clear_moves(ch);
					return;
				}
			}
		}
	}

	if ( IS_MORTAL (ch) && (!IS_SET (flags, MF_SWIM) &&
		 SWIM_ONLY (vtor (room_exit->to_room)) &&
		 !IS_SWIMMING (ch)) && !IS_SET (ch->act, ACT_FLYING)  ) {
		send_to_char ("You'll have to swim for it.\n", ch);
		clear_moves (ch);
		return;
	}

	if ( IS_SET (flags, MF_SWIM) &&
		 !IS_SWIMMING (ch) &&
		 !SWIM_ONLY (vtor (room_exit->to_room)) ) {
		send_to_char ("You can't swim there.\n", ch);
		clear_moves (ch);
		return;
	}

	if ( IS_SET (flags, MF_SWIM) || SWIM_ONLY (target_room) ) {
		if ( IS_RIDER (ch) ) {
			send_to_char ("Dismount first.\n", ch);
			return;
		}
		else if ( IS_RIDEE (ch) ) {
			send_to_char ("Your rider must dismount first.\n", ch);
			return;
		}
		move_names [ch->speed] = "swimming";
		move_names2 [ch->speed] = "swim";
	}

	needed_movement = calc_movement_charge (ch, dir, wanted_time, flags, &speed, &speed_name, &walk_time);

	/* Move failed for some reason, i.e. too much encumbrance */

	if ( needed_movement == -1 ) {
		clear_moves (ch);
		return;
	}

	if ( GET_MOVE (ch) < needed_movement ) {
		send_to_char("You are too exhausted.\n",ch);
		clear_moves (ch);
		return;
	}

	if ( (speed + 1) / 2 ) {
		if ( IS_SUBDUER (ch) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "You start %s %s, dragging $N.",
					  move_names [speed_name], dirs [dir]);
			act (buf, TRUE, ch, 0, ch->subdue, TO_CHAR);
			snprintf (buf, MAX_STRING_LENGTH,  "$N drags you with $M to the %s.", dirs[dir]);
			act (buf, TRUE, ch->subdue, 0, ch, TO_CHAR);
		}
		if ( IS_MOUNT (ch) && ch->mount && ch->mount->subdue ) {
			snprintf (buf, MAX_STRING_LENGTH,  "$N drags you with $M to the %s.", dirs[dir]);
			act (buf, TRUE, ch->subdue, 0, ch, TO_CHAR);
			act (buf, TRUE, ch->mount->subdue, 0, ch->mount, TO_CHAR);
		}
	}

	if ( IS_SET (flags, MF_TOEDGE) ) {
		exit_speed = speed;
		enter_speed = 0;
	}

	else if ( IS_SET (flags, MF_TONEXT_EDGE) ) {
		exit_speed = 1;
		enter_speed = speed;
	}

	else {
		exit_speed = (speed + 1) / 2;
		enter_speed = speed / 2;
	}

	if ( get_affect (ch, AFFECT_GUARD_DIR) )
		remove_affect_type (ch, AFFECT_GUARD_DIR);

	if (IS_SET(flags, MF_SNEAK)) {
		snprintf (buf, MAX_STRING_LENGTH,  "You cautiously begin sneaking %sward.\n", dirs [dir]);
		send_to_char(buf, ch);
		exit_speed += 4;
		if ( ch->speed > 2 )
			ch->speed = 2;
	}


	if ( IS_SET (ch->act, ACT_VEHICLE) ) {
		move_names[ch->speed] = add_hash ("travelling");
		travel_str[0] = '\0'; /* override travel strings in vehicles */
	}
	else if ( ( strlen(travel_str) == 0 ) && ( ch->travel_str != NULL ) ) {
	  snprintf(travel_str, MAX_STRING_LENGTH, ", %s", ch->travel_str);
	}

	if ((ch->room->sector_type == SECT_INSIDE && !IS_SET(flags, MF_SNEAK) &&  ch->speed != SPEED_IMMORTAL) ||
	   	(!IS_MORTAL(ch) && (ch->speed != SPEED_IMMORTAL) && !IS_SET(flags, MF_SNEAK))) {
		snprintf (buf, MAX_STRING_LENGTH,  "You begin %s %sward%s.\n", move_names[ch->speed], dirs [dir], travel_str);
		snprintf (buf1, MAX_STRING_LENGTH,  "$n begins %s %sward%s.", move_names[ch->speed], dirs [dir], travel_str);
		send_to_char(buf, ch);
		act (buf1, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	}

	if (ch->room->sector_type != SECT_INSIDE && IS_MORTAL(ch) && !IS_SET (flags, MF_SNEAK)
		&& !IS_SUBDUER (ch) && !is_mounted (ch) && !IS_RIDEE (ch)) {

		switch (weather_info[ch->room->zone].state) {
			case LIGHT_RAIN:
				exit_speed += 2;
				needed_movement *= 1.2;
				snprintf (buf, MAX_STRING_LENGTH,  "You %s %sward through the light rain%s.\n", move_names2[ch->speed], dirs [dir], travel_str);
				snprintf (buf1, MAX_STRING_LENGTH,  "$n %ss %sward through the light rain%s.", move_names2[ch->speed], dirs [dir], travel_str);
				send_to_char(buf, ch);
				act (buf1, TRUE, ch, 0, 0, TO_ROOM);
				break;
			case STEADY_RAIN:
				exit_speed += 4;
				needed_movement *= 1.5;
				snprintf (buf, MAX_STRING_LENGTH,  "You %s %sward through the rain%s.\n", move_names2[ch->speed], dirs [dir], travel_str);
				snprintf (buf1, MAX_STRING_LENGTH,  "$n %ss %sward through the rain%s.", move_names2[ch->speed], dirs [dir], travel_str);
				send_to_char(buf, ch);
				act (buf1, TRUE, ch, 0, 0, TO_ROOM);
				break;
			case HEAVY_RAIN:
				exit_speed += 6;
				needed_movement *= 2;
				snprintf (buf, MAX_STRING_LENGTH,  "You %s %sward, struggling through the lashing rain%s.\n", move_names2[ch->speed], dirs [dir], travel_str);
				snprintf (buf1, MAX_STRING_LENGTH,  "$n %ss %sward, struggling through the lashing rain%s.", move_names2[ch->speed], dirs [dir], travel_str);
				send_to_char(buf, ch);
				act (buf1, TRUE, ch, 0, 0, TO_ROOM);
				break;
			case LIGHT_SNOW:
				needed_movement *= 1.5;
				snprintf (buf, MAX_STRING_LENGTH,  "You %s %sward through the light snowfall%s.\n", move_names2[ch->speed], dirs [dir], travel_str);
				snprintf (buf1, MAX_STRING_LENGTH,  "$n %ss %sward through the light snowfall%s.", move_names2[ch->speed], dirs [dir], travel_str);
				send_to_char(buf, ch);
				act (buf1, TRUE, ch, 0, 0, TO_ROOM);
				break;
			case STEADY_SNOW:
				exit_speed += 8;
				needed_movement *= 2;
				snprintf (buf, MAX_STRING_LENGTH,  "You %s %sward through the steadily falling snow%s.\n", move_names2[ch->speed], dirs [dir], travel_str);
				snprintf (buf1, MAX_STRING_LENGTH,  "$n %ss %sward through the steadily falling snow%s.", move_names2[ch->speed], dirs [dir], travel_str);
				send_to_char(buf, ch);
				act (buf1, TRUE, ch, 0, 0, TO_ROOM);
				break;
			case HEAVY_SNOW:
				exit_speed += 12;
				needed_movement *= 4;
				snprintf (buf, MAX_STRING_LENGTH,  "You %s %sward, struggling through the shrieking snow%s.\n", move_names2[ch->speed], dirs [dir], travel_str);
				snprintf (buf1, MAX_STRING_LENGTH,  "$n %ss %sward, struggling through the shrieking snow%s.", move_names2[ch->speed], dirs [dir], travel_str);
				send_to_char(buf, ch);
				act (buf1, TRUE, ch, 0, 0, TO_ROOM);
				break;
			default:
				snprintf (buf, MAX_STRING_LENGTH,  "You begin %s %sward%s.\n", move_names[ch->speed], dirs [dir], travel_str);
				snprintf (buf1, MAX_STRING_LENGTH,  "$n begins %s %sward%s.", move_names[ch->speed], dirs [dir], travel_str);
				send_to_char(buf, ch);
				act (buf1, TRUE, ch, 0, 0, TO_ROOM);
				break;
		}
	}

	if ( IS_SET (ch->act, ACT_VEHICLE) ) {
		temp_arg = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "%s", temp_arg);
		*buf = toupper(*buf);
		snprintf (buf1, MAX_STRING_LENGTH,  "#5%s#0 begins %s %sward.\n", buf, move_names[ch->speed], dirs[dir]);
		send_to_room (buf1, ch->mob->virtual);
		needed_movement = 0;
	}

	temp_arg = str_dup(travel_str);
	exit_room (ch, dir, flags, exit_speed, enter_speed, speed_name,
			   needed_movement, strlen(travel_str) ? temp_arg : NULL);
			   
	return;
}

int IsGuarded (ROOM_DATA *room, int dir)
{
	AFFECTED_TYPE	*af = NULL;
	CHAR_DATA		*tch = NULL;

	if ( !room->people )
		return 0;

	for ( tch = room->people; tch; tch = tch->next_in_room ) {
		if ( (af = get_affect (tch, AFFECT_GUARD_DIR)) )
			if ( af->a.shadow.edge == dir )
				return 1;
	}

	return 0;
}

void move (CHAR_DATA *ch, char *argument, int dir, int speed)
{
	MOVE_DATA		*move = NULL;
	MOVE_DATA		*tmove = NULL;
	QE_DATA			*qe = NULL;
	CHAR_DATA		*tch = NULL;
	char			buf [MAX_STRING_LENGTH]= "";
	int             i = 0;
	int				j = 0;
	char            *tmp = NULL;
	
	if ( get_affect (ch, MAGIC_TOLL) )
		stop_tolls (ch);

	if ( *argument == '(' ) {
	   tmp = (char *)alloc ((int)strlen (argument), 31);
	    snprintf (buf, MAX_STRING_LENGTH,  "%s", argument);
	    i = 1;
	    j = 0;
	    tmp[j] = '\0';
	        
	    while ( buf[i] != ')' ) {
	        if ( buf[i] == '\0' ) {
		      	send_to_char ("Exactly how is it that you are trying to move?\n", ch);
				clear_moves(ch);
				return;
			}
			if ( buf[i] == '*' ) {
		        send_to_char ("Unfortunately, the use of *object references is not allowed while moving.\n",ch);
				clear_moves(ch);
				return;
			}
			if ( buf[i] == '~' ) {
		        send_to_char ("Unfortunately, the use of ~character references is not allowed while moving.\n",ch);
				clear_moves(ch);
				free(tmp);
				return;
			}
			
			tmp[j++] = buf[i++];
			tmp[j] = '\0';
		}
		
		buf[i] = '\0';
	}
	else {
	    argument = one_argument (argument, buf);	  
	}
	
	CREATE (move, MOVE_DATA, 1);

	move->dir = dir;
	move->desired_time = speed;
	move->next = NULL;
	move->travel_str = tmp;

	if ( get_affect (ch, MAGIC_SNEAK) )
		move->flags = MF_SNEAK;
	else {
		move->flags = 0;
		if ( !ch->mob || !IS_SET (ch->affected_by, AFF_HIDE) )
			remove_affect_type (ch, MAGIC_HIDDEN);
	}

	if ( !str_cmp (buf, "swim") )
	    SET_BIT (move->flags, MF_SWIM);
	  
	if ( !IS_MORTAL (ch) && !str_cmp (buf, "!") )
	    SET_BIT (move->flags, MF_PASSDOOR);
	  
	if ( !str_cmp (buf, "stand") )		/* Stand */
	    move->flags = MF_TOEDGE; 

	if ( !ch->moves ) {

		for ( qe = quarter_event_list; qe && qe->ch != ch; qe = qe->next )
			/**just cycling through the qe **/;

		if ( qe &&
			qe->dir == rev_dir [dir] &&
			GET_FLAG (ch, FLAG_LEAVING) ) {
	
			send_to_char ("You turn around.\n", ch);	
			act ("$n changes directions and returns.", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

			qe->dir = dir;
			qe->event_time = qe->arrive_time;

			SET_BIT (qe->flags, MF_ARRIVAL);
			REMOVE_BIT (ch->flags, FLAG_LEAVING);
			SET_BIT (ch->flags, FLAG_ENTERING);

			for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
				if ( !IS_NPC (tch) )
					continue;
			
				if ( tch->following != ch )
					continue;
				
				for ( qe = quarter_event_list; qe && qe->ch && qe->ch != tch; qe = qe->next )
				/**just cycling through the qe **/;

				send_to_char ("You turn around.\n", tch);	
				act ("$n changes directions and returns.", TRUE, tch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

				if ( qe ) {
					qe->dir = dir;
					qe->event_time = qe->arrive_time;

					SET_BIT (qe->flags, MF_ARRIVAL);
					REMOVE_BIT (tch->flags, FLAG_LEAVING);
					SET_BIT (tch->flags, FLAG_ENTERING);
				}
			}

			return;
		}

		ch->moves = move;

		if ( !GET_FLAG (ch, FLAG_LEAVING) && !GET_FLAG (ch, FLAG_ENTERING) )
			initiate_move (ch);

		return;
	}

	for ( tmove = ch->moves; tmove->next; )
		tmove = tmove->next;

	tmove->next = move;
	
	return;
}

void do_move (CHAR_DATA *ch, char *argument, int dir)
{
	AFFECTED_TYPE	*af = NULL;
	CHAR_DATA		*tch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			command [12];

	if ( !can_move (ch) )
		return;

	for ( af = ch->hour_affects; af; af = af->next ) {
		if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST && af->a.craft->timer ) {
			send_to_char ("You'll need to stop crafting first.\n", ch);
			return;
		}
	}

	/* If you are trying to move, bag the pmote */
	clear_pmote(ch);

	if ( get_affect (ch, MAGIC_TOLL) )
		stop_tolls (ch);

	if ( dir == DIR_UP )
		snprintf (command, MAX_STRING_LENGTH, "up");
	else if ( dir == DIR_DOWN )
		snprintf (command, MAX_STRING_LENGTH, "down");
	else if ( dir == DIR_EAST )
		snprintf (command, MAX_STRING_LENGTH, "east");
	else if ( dir == DIR_WEST )
		snprintf (command, MAX_STRING_LENGTH, "west");
	else if ( dir == DIR_NORTH )
		snprintf (command, MAX_STRING_LENGTH, "north");
	else 
		snprintf (command, MAX_STRING_LENGTH, "south");


	if ( IS_SET (ch->plr_flags, NEW_PLAYER_TAG) &&
		ch->room->dir_option[dir] &&
		vtor(ch->room->dir_option[dir]->to_room) &&
		IS_SET (vtor(ch->room->dir_option[dir]->to_room)->room_flags, FALL) &&
		!IS_SET (ch->act, ACT_FLYING) &&
		*argument != '!' ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#6Moving in that direction will quite likely result in a rather nasty fall. If you're sure about this, type \'%s !\' to confirm.#0", command);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( !is_mounted (ch) && GET_POS (ch) == POSITION_FIGHTING ) {

				/* Make sure nobody is trying to fight us before
				   declining move request */

		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {

			if ( tch == ch )
				continue;

			if ( tch->fighting == ch && GET_POS (tch) >= POSITION_FIGHTING ) {
				send_to_char ("You can't get away - try to FLEE instead!\n", ch);
				return;
			}
		}

	}

	if ( IS_RIDER (ch) )
		ch = ch->mount;

	move (ch, argument, dir, 0);
	
	return;
}

void hitches_follow (CHAR_DATA *ch, int dir, int leave_time, int arrive_time)
{
    MOVE_DATA       *movehf = NULL;

	if ( !IS_HITCHER (ch) ||
		 GET_FLAG (ch->hitchee, FLAG_ENTERING) ||
		 GET_FLAG (ch->hitchee, FLAG_LEAVING) )
		return;

    while ( ch->moves ) {
        movehf = ch->moves;
        ch->moves = movehf->next;
	if (movehf->travel_str) mem_free(movehf->travel_str);
        mem_free (movehf);
    }

	clear_current_move (ch->hitchee);

	move (ch->hitchee, "", dir, leave_time + arrive_time);
	
	return;
}

void followers_follow (CHAR_DATA *ch, int dir, int leave_time, int arrive_time)
{
	CHAR_DATA		*tch = NULL;
	ROOM_DATA		*room_exit = NULL;

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {

		if ( tch == ch || GET_FLAG (tch, FLAG_LEAVING) )
			continue;

		if ( tch->following != ch ) {
			if ( !IS_RIDEE (tch) || tch->mount->following != ch )
				continue;
		}

		if ( IS_HITCHEE (tch) )
			continue;

			/* Check if this mob tch is allow to go to target room */

		if ( IS_NPC (tch) && CAN_GO (tch, dir) && !IsGuarded (ch->room, dir) &&
			 (room_exit = vtor (PASSAGE (tch, dir)->to_room)) ) {

			if ( IS_SET (tch->flags, FLAG_KEEPER) &&
				 IS_SET (room_exit->room_flags, NO_MERCHANT) )
				continue;

			if ( tch->mob->access_flags &&
                 !(tch->mob->access_flags & room_exit->room_flags) )
				continue;

			if ( IS_SET (tch->act, ACT_STAY_ZONE) &&
				  tch->room->zone != room_exit->zone )
				continue;
		}

		if ( GET_POS (tch) == SIT ) {
			act ("You can't follow $N while sitting.",
				 FALSE, tch, 0, ch, TO_CHAR);
			return;
		}

		else if ( GET_POS (tch) == REST ) {
			act ("You can't follow $N while resting.",
				 FALSE, tch, 0, ch, TO_CHAR);
			return;
		}

		else if ( GET_POS (tch) < FIGHT )
			return;

		move (tch, "", dir, leave_time + arrive_time);
	}
	
	return;
}

void follower_catchup (CHAR_DATA *ch)
{
	CHAR_DATA		*tch = NULL;
	QE_DATA			*qe = NULL;

	if ( !ch->room )
		return;

	for ( tch = ch->room->people; tch; tch = tch->next_in_room )
		if ( ch->following == tch )
			break;

	if ( !tch || !GET_FLAG (tch, FLAG_LEAVING) || !CAN_SEE (tch, ch) || IS_SWIMMING (tch) )
		return;

	for ( qe = quarter_event_list; qe->ch != tch; qe = qe->next )
		;

	if ( !qe )
		return;

	if ( ch->aiming_at ) {
		send_to_char ("You lose your aim as you move.\n", ch);
		ch->aiming_at->targeted_by = NULL;
		ch->aiming_at = NULL;
		ch->aim = 0;
	}

	move (ch, "", qe->dir, qe->event_time + qe->arrive_time);
}

void do_east (CHAR_DATA *ch, char *argument, int cmd)
{
	do_move (ch, argument, DIR_EAST);
	return;
}

void do_west (CHAR_DATA *ch, char *argument, int cmd)
{
	do_move (ch, argument, DIR_WEST);
	return;
}

void do_north (CHAR_DATA *ch, char *argument, int cmd)
{
	do_move (ch, argument, DIR_NORTH);
	return;
}

void do_south (CHAR_DATA *ch, char *argument, int cmd)
{
	do_move (ch, argument, DIR_SOUTH);
	return;
}

void do_up (CHAR_DATA *ch, char *argument, int cmd)
{
	do_move (ch, argument, DIR_UP);
	return;
}

void do_down (CHAR_DATA *ch, char *argument, int cmd)
{
	do_move (ch, argument, DIR_DOWN);
	return;
}

int find_door(CHAR_DATA *ch, char *type, char *dir)
{
	char buf[MAX_STRING_LENGTH] = {'\0'};
	int door = 0;
	

	if (*dir) /* a direction was specified */
	{
		if ((door = search_block(dir, dirs, FALSE)) == -1) /* Partial Match */
		{
			send_to_char("That's not a direction.\n", ch);
			return(-1);
		}

		if (PASSAGE(ch, door))
			if (PASSAGE(ch, door)->keyword &&
				strlen (PASSAGE (ch, door)->keyword) )
				if (name_is(type, PASSAGE(ch, door)->keyword))
					return(door);
				else
				{
					snprintf (buf, MAX_STRING_LENGTH,  "I see no %s there.\n", type);
					send_to_char(buf, ch);
					return(-1);
				}
			else
				return(door);
		else
		{
			send_to_char(
				"There is nothing to open or close there.\n", ch);
			return(-1);
		}
	}
	else /* try to locate the keyword */
	{
		for (door = 0; door <= 5; door++)
			if (PASSAGE(ch, door))
				if (PASSAGE(ch, door)->keyword &&
					strlen (PASSAGE (ch, door)->keyword) )
					if (name_is(type, PASSAGE(ch, door)->keyword))
						return(door);

		snprintf (buf, MAX_STRING_LENGTH,  "I see no %s here.\n", type);
		send_to_char(buf, ch);
		return(-1);
	}
}


void do_open(CHAR_DATA *ch, char *argument, int cmd)
{

	int 		door = 0;
	int			other_room = 0;
	char 		buffer [MAX_STRING_LENGTH] = {'\0'};
	char 		type[MAX_INPUT_LENGTH] = {'\0'};
	char		dir[MAX_INPUT_LENGTH] = {'\0'};
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	struct 		room_direction_data *back = NULL;
	OBJ_DATA 	*obj = NULL;
	CHAR_DATA 	*victim = NULL;
	char		*temp_arg = NULL;

	argument_interpreter(argument, type, dir);

	if (!*type)
		send_to_char("Open what?\n", ch);
		
/** Open an entryway for a tent or lean-to **/

	if ( !cmp_strn ("entryway", type, strlen(type)) && ch->room && ch->room->virtual > 10000 ) {

		if ( !(obj = find_dwelling_obj (ch->room->virtual)) ) {
			send_to_char ("Dwelling object not found. Aborting.\n", ch);
			return;
		}
		
		else if ( !IS_SET (obj->o.od.value[2], CONT_CLOSEABLE) ) {
			send_to_char ("This entryway cannot be opened or closed.\n", ch);
			return;
		}
        
        else if ( !IS_SET (obj->o.od.value[2], CONT_CLOSED) ) {
            send_to_char ("The entrance is already open.\n", ch);
            return;
        }
        
        else if ( IS_SET (obj->o.od.value[2], CONT_LOCKED) ) {
        	send_to_char ("I'm afraid that's locked.\n", ch);
            return;
        }
    
    	REMOVE_BIT (obj->o.od.value[2], CONT_CLOSED);
        send_to_char ("You open the entryway.\n", ch);
        act ("$n opens the entryway.", FALSE, ch, obj, 0, TO_ROOM);
        
        temp_arg = obj_short_desc(obj);
		snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 is opened from the other side.", temp_arg);
		buf[2] = toupper(buf[2]);
		send_to_room (buf, obj->in_room);
	}

/** Open an object **/
	else if (generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {

		/** Open a book **/

		if ( GET_ITEM_TYPE(obj) == ITEM_BOOK ) {
			if ( obj->open ) {
				send_to_char ("It's already opened.\n", ch);
				return;
			}
			
			if ( !*dir || strtol(dir, NULL, 10) <= 1 ) {
				if ( !obj->writing || !obj->o.od.value[0]) {
					snprintf (buf, MAX_STRING_LENGTH,  "You open #2%s#0, and notice that it has no pages.\n", obj->short_description);
					send_to_char (buf, ch);
					
					temp_arg = char_short(ch);
					snprintf (buf, MAX_STRING_LENGTH,  "%s#0 opens #2%s#0.", temp_arg, obj->short_description);
					snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", CAP(buf));
					act (buffer, FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
					obj->open = 1;
					return;
				}
				
				snprintf (buf, MAX_STRING_LENGTH,  "You open #2%s#0 to the first page.\n", obj->short_description);
				send_to_char (buf, ch);
				
				temp_arg = char_short(ch);
				snprintf (buf, MAX_STRING_LENGTH,  "%s#0 opens #2%s#0 to the first page.", temp_arg, obj->short_description);
				snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", CAP(buf));
				act (buffer, FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
				obj->open = 1;
				return;
			}
			else if ( strtol(dir, NULL, 10) > obj->o.od.value[0] ) {
				send_to_char ("There aren't that many pages in the book.\n", ch);
				return;
			}
			else {
				snprintf (buf, MAX_STRING_LENGTH,  "You open #2%s#0 to page %d.\n", obj->short_description, (int)strtol(dir, NULL, 10));
				send_to_char (buf, ch);
				
				temp_arg = char_short(ch);
				snprintf (buf, MAX_STRING_LENGTH,  "%s#0 opens #2%s#0.", temp_arg, obj->short_description);
				snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", CAP(buf));
				act (buffer, FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
				obj->open = strtol(dir, NULL, 10);
				return;
			}
		} /* end of open book */
		
/** Door to a tent or other object-dwelling **/
		if ( obj->obj_flags.type_flag == ITEM_DWELLING ||
			 obj->obj_flags.type_flag == ITEM_TENT ) {
			if ( !IS_SET (obj->o.od.value[2], CONT_CLOSEABLE) ) {
				send_to_char ("That cannot be opened or closed.\n", ch);
				return;
			}
			else if ( !IS_SET (obj->o.od.value[2], CONT_CLOSED) ) {
				send_to_char ("That's already open.\n", ch);
				return;
			}
			else if ( IS_SET (obj->o.od.value[2], CONT_LOCKED) ) {
				send_to_char ("I'm afraid that's locked.\n", ch);
				return;
			}
			REMOVE_BIT (obj->o.od.value[2], CONT_CLOSED);
			act ("You open $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act ("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			if ( obj->o.od.value[0] > 0 && vtor (obj->o.od.value[0]) )
				send_to_room ("The entryway is opened from the other side.", obj->o.od.value[0]);
			return;
		} /* end open door to tent */
		
		if (obj->obj_flags.type_flag != ITEM_CONTAINER)
			send_to_char("That's not a container.\n", ch);
 		else if (!IS_SET(obj->o.container.flags, CONT_CLOSED))
			send_to_char("But it's already open!\n", ch);
		else if (!IS_SET(obj->o.container.flags, CONT_CLOSEABLE))
			send_to_char("You can't do that.\n", ch);
		else if (IS_SET(obj->o.container.flags, CONT_LOCKED))
			send_to_char("It seems to be locked.\n", ch);
		else
		{
			REMOVE_BIT(obj->o.container.flags, CONT_CLOSED);
			act("You open $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
		}
		
	} /* end object */
	
/** Open a door in the room **/	
	else if ((door = find_door(ch, type, dir)) >= 0) {

		if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_ISDOOR)){
			send_to_char("That's impossible, I'm afraid.\n", ch);
		}
		
		else if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_CLOSED)){
			send_to_char("It's already open!\n", ch);
		}
		
		else if (IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_LOCKED)){
			send_to_char("It seems to be locked.\n", ch);
		}
		else {
			REMOVE_BIT(PASSAGE(ch, door)->exit_info, PASSAGE_CLOSED);
			if (PASSAGE(ch, door)->keyword &&
				strlen (PASSAGE (ch, door)->keyword) ) {
				act("You open the $T.", FALSE, ch, 0, PASSAGE(ch, door)->keyword, TO_CHAR | TO_ACT_FORMAT);
				act("$n opens the $T.", FALSE, ch, 0, PASSAGE(ch, door)->keyword, TO_ROOM | TO_ACT_FORMAT);
			}
			
			else {
				act("You open the door.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			}
			
/** Open the OTHER side of the door! **/
			if ((other_room = PASSAGE(ch, door)->to_room) != NOWHERE){
				if ((back = vtor (other_room)->dir_option[rev_dir[door]])){
					if (back->to_room == ch->in_room){
						REMOVE_BIT(back->exit_info, PASSAGE_CLOSED);
						if (back->keyword &&
							strlen (back->keyword) ){
							
							temp_arg = fname(back->keyword);
							snprintf (buf, MAX_STRING_LENGTH, 
								"The %s is opened from the other side.\n",
								temp_arg);
							send_to_room(buf, PASSAGE(ch, door)->to_room);
						}
						else{
							send_to_room( 
							"The door is opened from the other side.\n",
							PASSAGE(ch, door)->to_room);
						}
					}
				}
			}
		}	
	}
	return;
}


void do_close(CHAR_DATA *ch, char *argument, int cmd)
{
	int 	door = 0;
	int		other_room = 0;
	char 	type[MAX_INPUT_LENGTH] = {'\0'};
	char	dir[MAX_INPUT_LENGTH] = {'\0'};
	char	buf[MAX_STRING_LENGTH] = {'\0'};
	char 	buffer [MAX_STRING_LENGTH] = {'\0'};
	struct room_direction_data 	*back = NULL;
	OBJ_DATA 	*obj = NULL;
	CHAR_DATA 	*victim = NULL;
	char	*temp_arg = NULL;

	if ( GET_POS (ch) == POSITION_FIGHTING ) {
		send_to_char ("No way! You are fighting for your life!\n\r", ch);
		return;
	}

	argument_interpreter(argument, type, dir);

	if (!*type) 
		send_to_char("Close what?\n", ch);

	else if ( !cmp_strn ("entryway", type, strlen(type)) && ch->room && ch->room->virtual > 10000 ) {
		if ( !(obj = find_dwelling_obj (ch->room->virtual)) ) {
			send_to_char ("Dwelling object not found. Aborting.\n", ch);
			return;
		}

		if ( !IS_SET (obj->o.od.value[2], CONT_CLOSEABLE) ) {
			send_to_char ("This entryway cannot be opened or closed.\n", ch);
			return;
		}
		
		else if ( IS_SET (obj->o.od.value[2], CONT_CLOSED) ) {
			send_to_char ("The entrance is already closed.\n", ch);
			return;
		}

		else if ( IS_SET (obj->o.od.value[2], CONT_LOCKED) ) {
			send_to_char ("I'm afraid that's locked.\n", ch);
			return;
		}

		SET_BIT (obj->o.od.value[2], CONT_CLOSED);
		send_to_char ("You close the entryway.\n", ch);
		act ("$n closes the entryway.", FALSE, ch, obj, 0, TO_ROOM);
		
		temp_arg = obj_short_desc(obj);
		snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 is closed from the other side.", temp_arg);
		buf[2] = toupper(buf[2]);
		send_to_room (buf, obj->in_room);
	}

	else if (generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {

/* this is an object */

		if ( GET_ITEM_TYPE (obj) == ITEM_BOOK ) {
			if ( !obj->open ) {
				send_to_char ("That isn't currently open.\n", ch);
				return;
			}
			
			snprintf (buf, MAX_STRING_LENGTH,  "You close #2%s#0.\n", obj->short_description);
			send_to_char (buf, ch);
			
			temp_arg = char_short(ch);
			snprintf (buf, MAX_STRING_LENGTH,  "%s#0 closes #2%s#0.", temp_arg, obj->short_description);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", CAP(buf));
			act (buffer, FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			obj->open = 0;
			return;
		}

		if ( obj->obj_flags.type_flag == ITEM_DWELLING ||
			 obj->obj_flags.type_flag == ITEM_TENT ) {

			if ( !IS_SET (obj->o.od.value[2], CONT_CLOSEABLE) ) {
				send_to_char ("That cannot be opened or closed.\n", ch);
				return;
			}

			else if ( IS_SET (obj->o.od.value[2], CONT_CLOSED) ) {
				send_to_char ("That's already closed.\n", ch);
				return;
			}

			SET_BIT (obj->o.od.value[2], CONT_CLOSED);
			act ("You close $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act ("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

			if ( obj->o.od.value[0] > 0 && vtor (obj->o.od.value[0]) ){
				send_to_room ("The entryway is closed from the other side.",  obj->o.od.value[0]);
			}
			
			return;
		}

		else if (obj->obj_flags.type_flag != ITEM_CONTAINER){
			send_to_char("That's not a container.\n", ch);
		}
		
		else if (IS_SET(obj->o.container.flags, CONT_CLOSED)){
			send_to_char("But it's already closed!\n", ch);
		}
		
		else if (!IS_SET(obj->o.container.flags, CONT_CLOSEABLE)){
			send_to_char("That's impossible.\n", ch);
		}
		
		else{
			SET_BIT(obj->o.container.flags, CONT_CLOSED);
			act("You close $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
		}		
	}
	
	else if ((door = find_door(ch, type, dir)) >= 0) {

/* Or a door */

		if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_ISDOOR)){
			send_to_char("That's absurd.\n", ch);
		}
		
		else if (IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_CLOSED)){
			send_to_char("It's already closed!\n", ch);
		}
		
		else {
			SET_BIT(PASSAGE(ch, door)->exit_info, PASSAGE_CLOSED);
			if (PASSAGE(ch, door)->keyword &&
				strlen (PASSAGE (ch, door)->keyword) ) {
				act("You close the $T.", 0, ch, 0, PASSAGE(ch, door)->keyword,TO_CHAR | TO_ACT_FORMAT);
				act("$n closes the $T.", 0, ch, 0, PASSAGE(ch, door)->keyword,TO_ROOM | TO_ACT_FORMAT);
			}
			else {
				act("You close the door.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
				act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			}

/* now for closing the other side, too */
			if ((other_room = PASSAGE(ch, door)->to_room) != NOWHERE){
				if ((back = vtor (other_room)->dir_option[rev_dir[door]])){
					if (back->to_room == ch->in_room){
						SET_BIT(back->exit_info, PASSAGE_CLOSED);
						
						if (back->keyword && strlen (back->keyword)){
							snprintf (buf, MAX_STRING_LENGTH, "The %s closes quietly.\n", back->keyword);
							send_to_room(buf, PASSAGE(ch, door)->to_room);
						}
						else{
							send_to_room("The door closes quietly.\n",  PASSAGE(ch, door)->to_room);
						}
					}
				}
			}
		}
	}
	return;
}


OBJ_DATA *has_key (CHAR_DATA *ch, OBJ_DATA *obj, int key)
{
	OBJ_DATA *tobj = NULL;

	if ( ch->right_hand && ch->right_hand->virtual == key ) {
		tobj = ch->right_hand;
		if ( obj && tobj->o.od.value[1] && tobj->o.od.value[1] == obj->coldload_id )
			return tobj;
		else if ( !obj || !tobj->o.od.value[1] )
			return tobj;
	}
	
	if ( ch->left_hand && ch->left_hand->virtual == key ) {
		tobj = ch->left_hand;
		if ( obj && tobj->o.od.value[1] && tobj->o.od.value[1] == obj->coldload_id )
			return tobj;
		else if ( !obj || !tobj->o.od.value[1] )
			return tobj;
	}

	if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_KEYRING ) {
		for ( tobj = ch->right_hand->contains; tobj; tobj = tobj->next_content )
			if ( tobj->virtual == key ) {
				if ( obj && tobj->o.od.value[1] && tobj->o.od.value[1] == obj->coldload_id )
					return tobj;
				else if ( !obj || !tobj->o.od.value[1] )
					return tobj;
			}
	}

	if ( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_KEYRING ) {
		for ( tobj = ch->left_hand->contains; tobj; tobj = tobj->next_content )
			if ( tobj->virtual == key ) {
				if ( obj && tobj->o.od.value[1] && tobj->o.od.value[1] == obj->coldload_id )
					return tobj;
				else if ( !obj || !tobj->o.od.value[1] )
					return tobj;
			}
	}

	return NULL;
}


void do_lock(CHAR_DATA *ch, char *argument, int cmd)
{
	int 	door = 0;
	int		other_room = 0;
	char 	type[MAX_INPUT_LENGTH] = {'\0'};
	char	dir[MAX_INPUT_LENGTH] = {'\0'};
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	struct room_direction_data *back = NULL;
	OBJ_DATA 	*obj = NULL;
	OBJ_DATA	*key = NULL;
	CHAR_DATA 	*victim = NULL;
	char		*temp_arg = NULL;


	argument_interpreter(argument, type, dir);

	if (!*type){
		send_to_char("Lock what?\n", ch);
	}
	else if ( !cmp_strn ("entryway", type, strlen(type)) && ch->room && ch->room->virtual > 10000 ) {
		if ( !(obj = find_dwelling_obj (ch->room->virtual)) ) {
			send_to_char ("Dwelling object not found. Aborting.\n", ch);
			return;
		}
		
		else if ( !IS_SET (obj->o.od.value[2], CONT_CLOSED) ) {
			send_to_char ("You'll need to close the entrance first.\n", ch);
			return;
		}

		else if ( IS_SET (obj->o.od.value[2], CONT_LOCKED) ) {
			send_to_char ("I'm afraid that's already locked.\n", ch);
			return;
		}

		else if ( !(key = has_key (ch, obj, obj->o.od.value[3])) ) {
			send_to_char("You don't seem to have the proper key.\n", ch);
			return;
		}	

		if ( key && !key->o.od.value[1] ){
			key->o.od.value[1] = obj->coldload_id;
		}
		
		SET_BIT (obj->o.od.value[2], CONT_LOCKED);
		send_to_char ("You lock the entryway.\n", ch);
		act ("$n locks the entryway.", FALSE, ch, obj, 0, TO_ROOM);

		temp_arg = obj_short_desc(obj);
		snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 is locked from the other side.", temp_arg);
		buf[2] = toupper(buf[2]);
		send_to_room (buf, obj->in_room);
		return;
	}

	else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {

/* this is an object */

		if ( obj->obj_flags.type_flag == ITEM_DWELLING ) {
			if ( obj->o.od.value[3] <= 0 ) {
				send_to_char ("That cannot be locked.\n", ch);
				return;
			}

			else if ( !IS_SET (obj->o.od.value[2], CONT_CLOSED) ) {
				send_to_char ("You'll need to close it first.\n", ch);
				return;
			}

			else if ( IS_SET (obj->o.od.value[2], CONT_LOCKED) ) {
				send_to_char ("I'm afraid that's already locked.\n", ch);
				return;
			}

			else if ( !(key = has_key (ch, obj, obj->o.od.value[3])) ) {
				send_to_char("You don't seem to have the proper key.\n", ch);
				return;
			}	

			if ( key && !key->o.od.value[1] ){
				key->o.od.value[1] = obj->coldload_id;
			}
			
			SET_BIT (obj->o.od.value[2], CONT_LOCKED);
			act ("You lock $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act("$n locks $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			return;
		}

		else if (obj->obj_flags.type_flag != ITEM_CONTAINER){
			send_to_char("That's not a container.\n", ch);
		}
		
		else if (!IS_SET(obj->o.container.flags, CONT_CLOSED)){
			send_to_char("Maybe you should close it first...\n", ch);
		}
		
		else if ( obj->o.container.key <= 0 ){
			send_to_char("That thing can't be locked.\n", ch);
		}
		
		else if ( !(key = has_key (ch, obj, obj->o.container.key)) ){
			send_to_char("You don't seem to have the proper key.\n", ch);	
		}
		
		else if (IS_SET(obj->o.container.flags, CONT_LOCKED)){
			send_to_char("It is locked already.\n", ch);
		}
		
		else{
			if ( key && !key->o.od.value[1] ){
				key->o.od.value[1] = obj->coldload_id;
			}
			
			SET_BIT(obj->o.container.flags, CONT_LOCKED);
			act ("You lock $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act("$n locks $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
		}
		
	}
	
	else if ((door = find_door(ch, type, dir)) >= 0) {

/* a door, perhaps */

		if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_ISDOOR)){
			send_to_char("That's absurd.\n", ch);
		}
		
		else if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_CLOSED)){
			send_to_char("You have to close it first, I'm afraid.\n", ch);
		}
		
		else if (PASSAGE(ch, door)->key < 0){
			send_to_char("There does not seem to be any keyholes.\n", ch);
		}
		
		else if (!has_key(ch, NULL, PASSAGE(ch, door)->key)){
			send_to_char("You don't have the proper key.\n", ch);
		}
		
		else if (IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_LOCKED)){
			send_to_char("It's already locked!\n", ch);
		}
		
		else{
			SET_BIT(PASSAGE(ch, door)->exit_info, PASSAGE_LOCKED);

			if (PASSAGE(ch, door)->keyword && 
				strlen (PASSAGE (ch, door)->keyword) ) {
				act("You lock the $T.", 0, ch, 0,  PASSAGE(ch, door)->keyword,TO_CHAR | TO_ACT_FORMAT);
				act("$n locks the $T.", 0, ch, 0,  PASSAGE(ch, door)->keyword,TO_ROOM | TO_ACT_FORMAT);
			}
			
			else {
				act("You lock the door.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			}

/* now for locking the other side, too */
			if ((other_room = PASSAGE(ch, door)->to_room) != NOWHERE){
				if ((back = vtor (other_room)->dir_option[rev_dir[door]])){
					if (back->to_room == ch->in_room){
						SET_BIT(back->exit_info, PASSAGE_LOCKED);
					}
				}
			}
		}
	}
	return;
}


void do_unlock(CHAR_DATA *ch, char *argument, int cmd)
{
	int 		door = 0;
	int			other_room = 0;
	char 		type[MAX_INPUT_LENGTH];
	char		dir[MAX_INPUT_LENGTH] = {'\0'};
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	struct room_direction_data 	*back;
	OBJ_DATA 	*obj = NULL;
	OBJ_DATA	*key = NULL;
	char		*temp_arg = NULL;
	CHAR_DATA 	*victim = NULL;


	argument_interpreter(argument, type, dir);

	if (!*type){
		send_to_char("Unlock what?\n", ch);
	}
	
	else if ( !cmp_strn ("entryway", type, strlen(type)) && ch->room && ch->room->virtual > 10000 ) {
		if ( !(obj = find_dwelling_obj (ch->room->virtual)) ) {
			send_to_char ("Dwelling object not found. Aborting.\n", ch);
			return;
		}

		else if ( !IS_SET (obj->o.od.value[2], CONT_LOCKED) ) {
			send_to_char ("I'm afraid that isn't locked.\n", ch);
			return;
		}

		else if ( !(key = has_key (ch, obj, obj->o.od.value[3])) ) {
			send_to_char("You don't seem to have the proper key.\n", ch);
			return;
		}	

		if ( key && !key->o.od.value[1] ){
			key->o.od.value[1] = obj->coldload_id;
		}

		REMOVE_BIT (obj->o.od.value[2], CONT_LOCKED);
		send_to_char ("You unlock the entryway.\n", ch);
		act ("$n unlocks the entryway.", FALSE, ch, obj, 0, TO_ROOM);

		temp_arg = obj_short_desc(obj);
		snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 is unlocked from the other side.", temp_arg);
		buf[2] = toupper(buf[2]);
		send_to_room (buf, obj->in_room);
		return;
	}

	else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
ch, &victim, &obj)) {

	/* this is an object */

		if ( obj->obj_flags.type_flag == ITEM_DWELLING ) {
			if ( obj->o.od.value[3] <= 0 ) {
				send_to_char ("That cannot be unlocked.\n", ch);
				return;
			}

			else if ( !IS_SET (obj->o.od.value[2], CONT_LOCKED) ) {
				send_to_char ("I'm afraid that isn't locked.\n", ch);
				return;
			}

			else if ( !(key = has_key (ch, obj, obj->o.od.value[3])) ) {
				send_to_char("You don't seem to have the proper key.\n", ch);
				return;
			}	

			if ( key && !key->o.od.value[1] ){
				key->o.od.value[1] = obj->coldload_id;
			}

			REMOVE_BIT (obj->o.od.value[2], CONT_LOCKED);
			act ("You unlock $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			return;
		}

		else if (obj->obj_flags.type_flag != ITEM_CONTAINER){
			send_to_char("That's not a container.\n", ch);
		}
	
		else if (!IS_SET(obj->o.container.flags, CONT_CLOSED)){
			send_to_char("It isn't closed.\n", ch);
		}

		else if (obj->o.container.key <= 0){
			send_to_char("Odd - you can't seem to find a keyhole.\n", ch);
		}

		else if (!(key = has_key(ch, obj, obj->o.container.key))){
			send_to_char("You don't have the proper key.\n", ch);	
		}

		else if (!IS_SET(obj->o.container.flags, CONT_LOCKED)){
			send_to_char("Oh.. it wasn't locked, after all.\n", ch);
		}

		else {
			if ( key && !key->o.od.value[1] ){
				key->o.od.value[1] = obj->coldload_id;
			}

			REMOVE_BIT(obj->o.container.flags, CONT_LOCKED);
			act ("You unlock $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
		}

	} else if ((door = find_door(ch, type, dir)) >= 0) {

		/* it is a door */

		if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_ISDOOR)){
			send_to_char("That's absurd.\n", ch);
		}
		else if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_CLOSED)){
			send_to_char("Heck.. it ain't even closed!\n", ch);
		}
		else if (PASSAGE(ch, door)->key < 0){
			send_to_char("You can't seem to spot any keyholes.\n", ch);
		}
		else if (!has_key(ch, NULL, PASSAGE(ch, door)->key)){
			send_to_char("You do not have the proper key for that.\n", ch);
		}
		else if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_LOCKED)){
			send_to_char("It's already unlocked, it seems.\n", ch);
		}
		else {
			REMOVE_BIT(PASSAGE(ch, door)->exit_info, PASSAGE_LOCKED);
			if (PASSAGE(ch, door)->keyword &&
				strlen (PASSAGE (ch, door)->keyword) ) {
				act("You unlock the $T.", 0, ch, 0, PASSAGE(ch, door)->keyword,TO_CHAR | TO_ACT_FORMAT);
				act("$n unlocks the $T.", 0, ch, 0, PASSAGE(ch, door)->keyword,TO_ROOM | TO_ACT_FORMAT);
			}
			else {
				act("$n unlock the door.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
			}

/* now for unlocking the other side, too */
			if ((other_room = PASSAGE(ch, door)->to_room) != NOWHERE){
				if ((back = vtor (other_room)->dir_option[rev_dir[door]])){
					if (back->to_room == ch->in_room){
						REMOVE_BIT(back->exit_info, PASSAGE_LOCKED);
					}
				}
			}
		}
	}

	return;
}

void do_pick (CHAR_DATA *ch, char *argument, int cmd)
{
	int				dir = 0;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA		*locked_obj = NULL;

	if ( !real_skill (ch, SKILL_PICK) ) {
		send_to_char ("You don't know how to pick locks.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("What would you like to pick?\n", ch);
		return;
	}

	if ( !(ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_LOCKPICK) &&
	     !(ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_LOCKPICK) ) {
		send_to_char ("You must be holding a lockpick.\n", ch);
		return;
	}

	if ( !(locked_obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) )
		locked_obj = get_obj_in_dark (ch, buf, ch->right_hand);

	if ( locked_obj ) {

		if ( locked_obj->obj_flags.type_flag != ITEM_CONTAINER &&
			GET_ITEM_TYPE (locked_obj) != ITEM_DWELLING ) {
			act ("You can't pick $p.", FALSE, ch, locked_obj, 0, TO_CHAR);
			return;
		}

		if ( GET_ITEM_TYPE (locked_obj) == ITEM_CONTAINER ) {
			if ( locked_obj->o.od.value[2] <= 0 ) {
				act ("$p looks unpickable.", FALSE, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if ( !IS_SET (locked_obj->o.container.flags, CONT_CLOSED) ) {
				act ("$p is already open.", FALSE, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if ( !IS_SET (locked_obj->o.container.flags, CONT_LOCKED) ) {
				act ("As you start, you discover $p is already unlocked.",
						FALSE, ch, locked_obj, 0, TO_CHAR);
				return;
			}
		}
		else {
			if ( locked_obj->o.od.value[3] <= 0 ) {
				act ("$p looks unpickable.", FALSE, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if ( !IS_SET (locked_obj->o.od.value[1], CONT_CLOSED) ) {
				act ("$p is already open.", FALSE, ch, locked_obj, 0, TO_CHAR);
				return;
			}

			if ( !IS_SET (locked_obj->o.od.value[1], CONT_LOCKED) ) {
				act ("As you start, you discover $p is already unlocked.",
						FALSE, ch, locked_obj, 0, TO_CHAR);
				return;
			}
		}

		act ("You begin picking the lock of $p.",
				FALSE, ch, locked_obj, 0, TO_CHAR);
		act ("$n uses $s tools on $p.", TRUE, ch, locked_obj, NULL, TO_ROOM);

		ch->delay_type	= DEL_PICK_OBJ;
		ch->delay       = 25 - ch->skills [SKILL_PICK] / 10;
		ch->delay_info1 = (int) locked_obj;

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
			send_to_char ("You may pick north, south, east, west, up, or "
						  "down.\n", ch);
			return;
	}

	if ( !PASSAGE (ch, dir) ) {
		send_to_char ("There is no exit in that direction.\n",ch);
		return;
	}

	if ( !IS_SET (PASSAGE (ch, dir)->exit_info, PASSAGE_ISDOOR)) {
		send_to_char ("No door in that direction.\n", ch);
		return;
	}

	if ( !IS_SET (PASSAGE (ch, dir)->exit_info, PASSAGE_LOCKED)) {
		send_to_char("It's already open.\n",ch);
		return;
	}

	act ("You try to pick the $T.",
				FALSE, ch, 0, PASSAGE (ch, dir)->keyword, TO_CHAR);

	if ( number (1, 100) > ch->skills [SKILL_PICK] ) {

		act ("$n uses $s tools on the $T.",
				TRUE, ch, 0, PASSAGE (ch, dir)->keyword, TO_ROOM);

			/* 100% crime in daylight for failure.  1/3 chance if night
               and not guarded. */

		if ( sun_light || is_guarded (NULL, ch) || !number (0, 2) )
			criminalize (ch, NULL, ch->room->zone, CRIME_PICKLOCK);
	}

	ch->delay_type	= DEL_PICK;
    	ch->delay       = 25 - ch->skills [SKILL_PICK] / 10;
	ch->delay_info1 = dir;
	ch->delay_info2 = ch->in_room;
	
	return;
}

void delayed_pick_obj (CHAR_DATA *ch)
{
	int				roll = 0;
	OBJ_DATA		*tobj = NULL;
	OBJ_DATA		*locked_obj = NULL;
	OBJ_DATA		*obj = NULL;

	locked_obj = (OBJ_DATA *) ch->delay_info1;

	if ( !is_obj_here (ch, locked_obj, 1) ) {

		if ( ch->right_hand && ch->right_hand == locked_obj )
			obj = ch->right_hand;

		if ( ch->left_hand && ch->left_hand == locked_obj )
			obj = ch->left_hand;

		if ( obj == locked_obj ) {
			send_to_char ("You stop picking.\n", ch);
			return;
		}
	}

	if ( !IS_SET (locked_obj->o.container.flags, CONT_CLOSED) ||
		 !IS_SET (locked_obj->o.container.flags, CONT_LOCKED) ) {
		send_to_char ("You stop picking.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (locked_obj) == ITEM_CONTAINER ) {
		if ( ch->skills [SKILL_PICK] > locked_obj->o.container.pick_penalty )
			skill_use (ch, SKILL_PICK, locked_obj->o.container.pick_penalty);

		if ( (roll = number (1, 100)) > ch->skills [SKILL_PICK] - locked_obj->o.container.pick_penalty ) {
			if ( !(roll % 5) && (tobj = get_carried_item (ch, ITEM_LOCKPICK)) ) {
				act ("You fail miserably, snapping your pick in the process!", FALSE, ch, 0, 0, TO_CHAR);
				act ("$n curses as $s lockpick snaps.", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
				extract_obj (tobj);
				return;
			}
			act ("You failed.", FALSE, ch, 0, 0, TO_CHAR);
			act ("$n fails to pick $p.", TRUE, ch, locked_obj, 0, TO_ROOM);
			return;
		}
	}
	else if ( GET_ITEM_TYPE (locked_obj) == ITEM_DWELLING ) {
		if ( ch->skills [SKILL_PICK] > locked_obj->o.od.value[4] )
			skill_use (ch, SKILL_PICK, locked_obj->o.od.value[4]);

		if ( (roll = number (1, 100)) > ch->skills [SKILL_PICK] - locked_obj->o.od.value[4] ) {
			if ( !(roll % 5) && (tobj = get_carried_item (ch, ITEM_LOCKPICK)) ) {
				act ("You fail miserably, snapping your pick in the process!", FALSE, ch, 0, 0, TO_CHAR);
				act ("$n curses as $s lockpick snaps.", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
				extract_obj (tobj);
				return;
			}
			act ("You failed.", FALSE, ch, 0, 0, TO_CHAR);
			act ("$n fails to pick $p.", TRUE, ch, locked_obj, 0, TO_ROOM);
			return;
		}
	}

	act ("You have successfully picked the lock.", TRUE, ch, 0, 0, TO_CHAR);
	act ("$n has picked the lock of $p.", TRUE, ch, locked_obj, 0, TO_ROOM | TO_ACT_FORMAT);

	if ( GET_ITEM_TYPE (locked_obj) == ITEM_CONTAINER )
		REMOVE_BIT (locked_obj->o.container.flags, CONT_LOCKED);
	else REMOVE_BIT (locked_obj->o.od.value[2], CONT_LOCKED);
	
	return;
}

void delayed_pick (CHAR_DATA *ch)
{
	ROOM_DATA	*troom = NULL;
	OBJ_DATA	*tobj = NULL;
	int			dir = 0;
	int			roll = 0;

	dir = ch->delay_info1;

	if ( ch->delay_info2 != ch->in_room || !PASSAGE (ch, dir) )
		return;

	if ( IS_SET (PASSAGE (ch, dir)->exit_info, PASSAGE_PICKPROOF) ) {
		send_to_char ("You failed.\n", ch);
		return;
	}

	if ( !IS_SET (PASSAGE (ch, dir)->exit_info, PASSAGE_LOCKED) ) {
		send_to_char ("Someone has unlocked the door for you.\n", ch);
		return;
	}

	/* Don't let PC get any practice out of this lock if it is beyond
           his/her ability. */

	if ( ch->skills [SKILL_PICK] > PASSAGE (ch, dir)->pick_penalty )
		skill_use (ch, SKILL_PICK, PASSAGE (ch, dir)->pick_penalty);

    	if ( (roll = number (1, 100)) > ch->skills [SKILL_PICK] - PASSAGE (ch, dir)->pick_penalty ) {
		if ( !(roll % 5) && (tobj = get_carried_item (ch, ITEM_LOCKPICK)) ) {
			act ("You fail miserably, snapping your pick in the process!", FALSE, ch, 0, 0, TO_CHAR);
			act ("$n curses as $s lockpick snaps.", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			extract_obj (tobj);
			return;
		}
		send_to_char ("You failed.\n", ch);
		return;
	}

	REMOVE_BIT (ch->room->dir_option [dir]->exit_info, PASSAGE_LOCKED);

	if ( (troom = vtor (ch->room->dir_option[dir]->to_room)) )
		REMOVE_BIT (troom->dir_option [rev_dir[dir]]->exit_info, PASSAGE_LOCKED);

	send_to_char ("You successfully picked the lock.\n", ch);
	
	return;
}

void enter_vehicle (CHAR_DATA *ch, CHAR_DATA *ent_mob)
{
	if ( !ent_mob->mob ) {
		send_to_char ("You can't.\n", ch);
		return;
	}

	if ( !vtor (ent_mob->mob->virtual) ) {
		send_to_char ("A note on the entrance says, 'broken'\n", ch);
		system_log ("Attempt to use a broken boat or hitch, enter_vehicle()", TRUE);
		return;
	}

	if ( ch->vehicle ) {
	}
	
	if ( ent_mob->mob->vehicle_type == VEHICLE_HITCH ) {
		act ("You climb into $N.", FALSE, ch, 0, ent_mob, TO_CHAR);
		act ("$n climbs into $N.", TRUE, ch, 0, ent_mob, TO_NOTVICT);
	}

	else if ( ch->room->sector_type != SECT_DOCK )
		send_to_char ("You swim close, grab some netting, and hoist "
					  "yourself aboard.\n", ch);

	char_from_room (ch);

	remove_affect_type (ch, MAGIC_SNEAK);
	remove_affect_type (ch, MAGIC_HIDDEN);

	char_to_room (ch, ent_mob->mob->virtual);

	ch->vehicle = ent_mob;
	ch->coldload_id = ent_mob->coldload_id;

	if ( !IS_NPC (ch) )
		ch->pc->boat_virtual = ent_mob->mob->virtual;

	act ("$n has boarded.", TRUE, ch, 0, 0, TO_ROOM);	

	do_look (ch, "", 15);
	
	return;
}

void do_enter (CHAR_DATA *ch, char *argument, int cmd)
{
	ROOM_DATA	*room = NULL;
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_INPUT_LENGTH] = {'\0'};
	char		tmp [MAX_STRING_LENGTH] = {'\0'};
	int			door = 0;
	CHAR_DATA	*ent_mob = NULL;
	CHAR_DATA	*tch = NULL;
	int			occupants = 0;
	char		*temp_arg = NULL;

	(void)one_argument (argument, buf);


/*** Entering a Vehicle ***/
	if ( *buf && (ent_mob = get_char_room_vis (ch, buf)) &&
		 IS_SET (ent_mob->act, ACT_VEHICLE) ) {
		enter_vehicle (ch, ent_mob);
		return;
	}

/*** entering a tent or dwelling ***/
	if ( *buf && (obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) && (GET_ITEM_TYPE (obj) == ITEM_TENT || GET_ITEM_TYPE(obj) == ITEM_DWELLING) ) {
		argument = one_argument (argument, buf);
		if ( GET_ITEM_TYPE (obj) == ITEM_TENT && !IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHED) ) {
			send_to_char ("You'll need to pitch it first.\n", ch);
			return;
		}
		if ( IS_SET (obj->o.od.value[2], CONT_CLOSED) && !(!IS_MORTAL(ch) && *argument == '!') ) {
			send_to_char ("You'll need to open it before entering.\n", ch);
			return;
		}
		room = generate_dwelling_room (obj);
		for ( tch = room->people; tch; tch = tch->next_in_room )
			occupants++;
		if ( occupants > obj->o.od.value[1] ) {
			send_to_char ("There are already too many people inside.\n", ch);
			return;
		}
		temp_arg = obj_short_desc (obj);
		snprintf (buf, MAX_STRING_LENGTH,  "You enter #2%s#0.", temp_arg);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		snprintf (buf, MAX_STRING_LENGTH,  "$n enters #2%s#0.", temp_arg);
		act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		
		obj->o.od.value[0] = room->virtual;
		ch->was_in_room = ch->in_room;
		char_from_room (ch);
		char_to_room (ch, room->virtual);
		send_to_char ("\n", ch);
		do_look (ch, "", 0);
		snprintf (buf, MAX_STRING_LENGTH,  "$n enters from the outside.");
		act (buf, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		if ( IS_MORTAL (ch) )
			room->occupants++;
		return;
	}

	if (*buf)  /* an argument was supplied, search for door keyword */
	{
		for (door = 0; door <= 5; door++)
			if (PASSAGE(ch, door))
				if (PASSAGE(ch, door)->keyword &&
					strlen (PASSAGE (ch, door)->keyword) )
					if (!str_cmp(PASSAGE(ch, door)->keyword, buf)) {
						do_move(ch, "", door);
						return;
					}
		snprintf(tmp, MAX_STRING_LENGTH, "There is no %s here.\n", buf);
		send_to_char(tmp, ch);
 	}
	else
		if ( IS_SET (vtor (ch->in_room)->room_flags, INDOORS))
			send_to_char("You are already indoors.\n", ch);
		else
		{
			/* try to locate an entrance */
			for (door = 0; door <= 5; door++)
				if (PASSAGE(ch, door))
					if (PASSAGE(ch, door)->to_room != NOWHERE)
						if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_CLOSED) &&
							IS_SET(vtor (PASSAGE(ch, door)->to_room)->room_flags,
							INDOORS))
						{
							do_move(ch, "", door);
							return;
						}
			send_to_char("You can't seem to find anything to enter.\n", ch);
		}
	return;
}


void leave_vehicle (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA		*vehicle = NULL;
	ROOM_DATA		*troom = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( ch->vehicle ) {

		vehicle = ch->vehicle;

		if ( vehicle->mob->virtual != ch->in_room ) {
			send_to_char ("You cannot exit the craft from here.\n", ch);
			return;
		}

		act ("$n disembarks.", TRUE, ch, 0, 0, TO_ROOM);

		troom = vtor (vehicle->in_room);

		if ( troom->sector_type == SECT_REEF ||
			 troom->sector_type == SECT_OCEAN ||
			 is_room_affected (troom->affects, MAGIC_ROOM_FLOOD) ) {
			if ( *buf != '!' ) {
				act ("$N is at sea.", FALSE, ch, 0, vehicle, TO_CHAR);
				send_to_char ("If you jump ship into the water, type:\n"
							  "   #3leave !#0\n", ch);
				return;
			}
		}

		char_from_room (ch);

		char_to_room (ch, vehicle->in_room);

		ch->vehicle = NULL;
		ch->coldload_id = 0;	

		if ( !IS_NPC (ch) )
			ch->pc->boat_virtual = 0;

		act ("$n disembarks from $N.", TRUE, ch, 0, vehicle, TO_ROOM);	

		do_look (ch, "", 15);
		return;
	}
	else if ( IS_SET (ch->room->room_flags, VEHICLE) ) {
		if ( !(vehicle = get_mob_vnum (ch->room->virtual)) ) {
			send_to_char ("You cannot exit the craft from here.\n", ch);
			return;
		}
		act ("$n disembarks.", TRUE, ch, 0, 0, TO_ROOM);

		char_from_room (ch);

		char_to_room (ch, vehicle->in_room);

		ch->vehicle = NULL;
		ch->coldload_id = 0;	

		if ( !IS_NPC (ch) )
			ch->pc->boat_virtual = 0;

		act ("$n disembarks from $N.", TRUE, ch, 0, vehicle, TO_ROOM);	

		do_look (ch, "", 15);
		return;
	}

	send_to_char ("You cannot exit the craft from here.\n", ch);
	
	return;
}

void do_leave (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int 		door = 0;
	int			target = 0;
	char		*temp_arg = NULL;

	if ( ch->vehicle || IS_SET (ch->room->room_flags, VEHICLE) ) {
		leave_vehicle (ch, argument);
		return;
	}

	if ( ch->room->virtual > 100000 ) {
		for ( obj = object_list; obj; obj = obj->next ) {
			if ( obj->deleted )
				continue;
			if ( obj->o.od.value[0] == ch->in_room )
				break;
		}
		if ( obj ) {
			if ( IS_SET (obj->o.od.value[2], CONT_CLOSED) && !(!IS_MORTAL(ch) && *argument == '!') ) {
				send_to_char ("The entryway is closed.\n", ch);
				return;
			}
			
			temp_arg = obj_short_desc (obj);
			snprintf (buf, MAX_STRING_LENGTH,  "You leave #2%s#0.", temp_arg);
			act (buf, TRUE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			snprintf (buf, MAX_STRING_LENGTH,  "$n leaves #2%s#0.", temp_arg);
			act (buf, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		}
		else {
			send_to_char ("The dwelling object seems to be missing. Please report.\n", ch);
			return;
		}
		target = obj->in_room;
		ch->room->occupants--;
		char_from_room (ch);
		char_to_room (ch, target);
		send_to_char ("\n", ch);
		do_look (ch, "", 0);
		if ( obj ) {
			temp_arg = obj_short_desc(obj);
			snprintf (buf, MAX_STRING_LENGTH,  "$n arrives from inside #2%s#0.", temp_arg);
			act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		}
		return;
	}

	if (!IS_SET(vtor (ch->in_room)->room_flags, INDOORS))
		send_to_char("You are outside.. where do you want to go?\n", ch);
	else
	{
		for (door = 0; door <= 5; door++)
			if (PASSAGE(ch, door))
				if (PASSAGE(ch, door)->to_room != NOWHERE)
					if (!IS_SET(PASSAGE(ch, door)->exit_info, PASSAGE_CLOSED) &&
						!IS_SET(vtor (PASSAGE(ch, door)->to_room)->room_flags, INDOORS))
					{
						do_move(ch, "", door);
						return;
					}
		send_to_char("I see no obvious exits to the outside.\n", ch);
	}
	
	return;
}

void do_stand (CHAR_DATA *ch, char *argument, int cmd)
{
	int					already_standing = 0;
	int					dir = 0;
	AFFECTED_TYPE		*af = NULL;
	char				buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !can_move (ch) ) {
		send_to_char ("You can't move.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf ) {
		if ( (dir = index_lookup (dirs, buf)) == -1 ) {
			send_to_char ("Stand where?\n", ch);
			return;
		}

		if ( !(af = get_affect (ch, AFFECT_SHADOW)) ) {

			magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

			af = get_affect (ch, AFFECT_SHADOW);

			af->a.shadow.shadow = NULL;
			af->a.shadow.edge = -1;			/* Not on an edge */
		}

		move (ch, "stand", dir, 0);
		return;
	}

	if ( get_second_affect (ch, SPA_STAND, NULL) )
		return;

	if ( GET_POS (ch) == STAND )
		already_standing = 1;

	switch ( GET_POS (ch) ) {
		case STAND:
			act ("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
			return;
			break;

		case SIT: 

			if ( (af = get_affect (ch, MAGIC_SIT_TABLE)) &&
				 is_obj_in_list ((OBJ_DATA *) af->a.spell.t, ch->room->contents) ) {
				act ("You get up from $p.",
						FALSE, ch, (OBJ_DATA *) af->a.spell.t, 0, TO_CHAR);
				act ("$n gets up from $p.",
						TRUE, ch, (OBJ_DATA *) af->a.spell.t, 0, TO_ROOM);
			} else {
				act ("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
				act ("$n clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
			}

			GET_POS (ch) = STAND;

			break;

		case REST:
			act ("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
			act ("$n stops resting, and clambers on $s feet.",
						TRUE, ch, 0, 0, TO_ROOM);
			GET_POS (ch) = POSITION_STANDING;
			break;

		case SLEEP:
			do_wake (ch, argument, 0);
			break;

		case FIGHT: 
			act ("You are standing and fighting.", FALSE, ch, 0, 0, TO_CHAR);
			break;

		default:
			act ("You stop floating around, and put your feet on the ground.",
			  			FALSE, ch, 0, 0, TO_CHAR);
			act ("$n stops floating around, and puts $s feet on the ground.",
			  			TRUE, ch, 0, 0, TO_ROOM);
		break;
	}

	if ( !already_standing && GET_POS (ch) == STAND )
		follower_catchup (ch);

	if ( GET_POS (ch) != SIT && GET_POS (ch) != SLEEP &&
		 (af = get_affect (ch, MAGIC_SIT_TABLE)) )
		affect_remove (ch, af);
	
	return;
}


void do_sit (CHAR_DATA *ch, char *argument, int cmd)
{
	int				count = 0;
	CHAR_DATA		*tch = NULL;
	SECOND_AFFECT	*sa = NULL;
	OBJ_DATA		*obj = NULL;
	AFFECTED_TYPE	*af = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( (sa = get_second_affect (ch, SPA_STAND, NULL)) )
		remove_second_affect (sa);

	if ( *buf && !(obj = get_obj_in_list_vis (ch, buf, ch->room->contents)) ) {
		send_to_char ("You don't see that to sit at.\n", ch);
		return;
	}

	if ( obj ) {

		if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_TABLE) ) {
			act ("You cannot sit at $p.", FALSE, ch, obj, 0, TO_CHAR);
			return;
		}

		if ( GET_POS (ch) != STAND ) {
			act ("You must be standing before you can sit at $p.",
						FALSE, ch, obj, 0, TO_CHAR);
			return;
		}

		count = 0;

		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {

			if ( tch == ch )
				continue;

			if ( (af = get_affect (tch, MAGIC_SIT_TABLE)) &&
				 af->a.table.obj == obj )
				count++;
		}

		if ( obj->o.container.table_max_sitting != 0 &&
			 count >= obj->o.container.table_max_sitting ) {
			act ("There is no available space at $p.", FALSE, ch, obj, 0, TO_CHAR);
			return;
		}

		magic_add_affect (ch, MAGIC_SIT_TABLE, -1, 0, 0, 0, 0);

		af = get_affect (ch, MAGIC_SIT_TABLE);

		af->a.table.obj = obj;

		GET_POS (ch) = SIT;

		act ("You sit at $p.", FALSE, ch, obj, 0, TO_CHAR);
		act ("$n sits at $p.", TRUE, ch, obj, 0, TO_ROOM);

		return;
	}

	switch(GET_POS(ch)) {
		case POSITION_STANDING : {
			act("You sit down.", FALSE, ch, 0,0, TO_CHAR);
			act("$n sits down.", TRUE, ch, 0,0, TO_ROOM);
			GET_POS(ch) = POSITION_SITTING;
		} break;
		case POSITION_SITTING	: {
			send_to_char("You're sitting already.\n", ch);
		} break;
		case POSITION_RESTING	: {
			act("You stop resting, and sit up.", FALSE, ch,0,0,TO_CHAR);
			act("$n stops resting.", TRUE, ch, 0,0,TO_ROOM);
			GET_POS(ch) = POSITION_SITTING;
		} break;
		case POSITION_SLEEPING : {
			act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
		} break;
		case POSITION_FIGHTING :
			act("YOU'RE FIGHTING!  YOU'LL GET SLAUGHTERED!!!",
					FALSE, ch, 0, 0, TO_CHAR);
			act("You sit down in combat.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n sits down in combat.", TRUE, ch, 0, 0, TO_ROOM);
			GET_POS (ch) = SIT;
			break;
		default : {
			act("You stop floating around, and sit down.", FALSE, ch,0,0,TO_CHAR);
			act("$n stops floating around, and sits down.", TRUE, ch,0,0,TO_ROOM);
			GET_POS(ch) = POSITION_SITTING;
		} break;
	}
	return;
}
			

void do_rest(CHAR_DATA *ch, char *argument, int cmd) {


	if ( !can_move (ch) ) {
		send_to_char ("You can't move.\n", ch);
		return;
	}

	switch(GET_POS(ch)) {
		case POSITION_STANDING : {
			act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
			GET_POS(ch) = POSITION_RESTING;
		} break;
		case POSITION_SITTING : {
			act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
			act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
			GET_POS(ch) = POSITION_RESTING;
		} break;
		case POSITION_RESTING : {
			act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
		} break;
		case POSITION_SLEEPING : {
			act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
			} break;
		case POSITION_FIGHTING : {
			act("Rest while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
		} break;
		default : {
			act("You stop floating around, and stop to rest your tired bones.",
			  FALSE, ch, 0, 0, TO_CHAR);
			act("$n stops floating around, and rests.", FALSE, ch, 0,0, TO_ROOM);
			GET_POS(ch) = POSITION_SITTING;
		} break;
	}
	return;
}


void do_sleep (CHAR_DATA *ch, char *argument, int cmd)
{
	clear_pmote(ch);

	switch ( GET_POS (ch) ) {
		case POSITION_STANDING : 
		case POSITION_SITTING  :
		case POSITION_RESTING  :

			send_to_char ("You fall asleep.\n", ch);

			if ( IS_SET (ch->act, ACT_MOUNT) )
				act ("$n lowers $s head and falls asleep.",
						TRUE, ch, 0, 0, TO_ROOM);
			else
				act ("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);

			GET_POS (ch) = POSITION_SLEEPING;

			break;

		case POSITION_SLEEPING :
			send_to_char("You are already sound asleep.\n", ch);
			break;

		case POSITION_FIGHTING :
			send_to_char ("Sleep while fighting? are you MAD?\n", ch);
			break;

		default :
			act("You stop floating around, and lie down to sleep.",
			  FALSE, ch, 0, 0, TO_CHAR);
			act("$n stops floating around, and lies down to sleep.",
			  TRUE, ch, 0, 0, TO_ROOM);
			GET_POS (ch) = POSITION_SLEEPING;
			break;
	}
	return;
}

int wakeup (CHAR_DATA *ch)
{
	AFFECTED_TYPE		*af = NULL;

	if ( GET_POS (ch) != SLEEP )
		return 0;

	if ( get_affect (ch, MAGIC_AFFECT_SLEEP) )
		return 0;

	send_to_char ("Your sleep is disturbed.\n", ch);

	if ( (af = get_affect (ch, MAGIC_SIT_TABLE)) &&
		 is_obj_in_list (af->a.table.obj, ch->room->contents) )
		GET_POS (ch) = SIT;
	else
		GET_POS (ch) = REST;

	return 1;
}

void do_wake (CHAR_DATA *ch, char *argument, int not_noisy)
{
	CHAR_DATA		*tch = NULL;
	AFFECTED_TYPE	*af = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( *buf ) {
		if ( !(tch = get_char_room_vis (ch, buf)) ) {
			if ( not_noisy )
				send_to_char ("They aren't here.\n", ch);
			return;
		}
	}

		/* Awaken someone else */

	if ( *buf && tch != ch ) {

		if ( GET_POS (ch) == POSITION_SLEEPING ) {
			send_to_char ("You must be awake yourself to do that.\n", ch);
			return;
		}

		if ( GET_POS (tch) != POSITION_SLEEPING ) {
			act ("$E isn't asleep.", TRUE, ch, 0, tch, TO_CHAR);
			return;
		}

		if ( get_affect (tch, MAGIC_AFFECT_SLEEP) ) {
			if ( not_noisy )
				act ("$E doesn't respond.", FALSE, ch, 0, tch, TO_CHAR);
			return;
		}

		if ( get_second_affect (tch, SPA_KNOCK_OUT, NULL) ) {
			if ( not_noisy )
				act ("$E can't be roused.", FALSE, ch, 0, tch, TO_CHAR);
			return;
		}

		act ("You wake $M up.", FALSE, ch, 0, tch, TO_CHAR);
		GET_POS (tch) = POSITION_RESTING;

		act ("You are awakened by $n.", FALSE, ch, 0, tch, TO_VICT);

		if ( (tch = being_dragged (ch)) ) {
			act ("You awaken to discover $N was dragging you!",
					FALSE, ch, 0, tch, TO_CHAR);
			if ( (af = get_affect (tch, MAGIC_DRAGGER)) )
				affect_remove (tch, af);
		}

		return;
	}

			/* Awaken yourself */

	if ( get_affect (ch, MAGIC_AFFECT_SLEEP) ) {
		send_to_char ("You can't wake up!\n", ch);
		return;
	}

	if ( GET_POS (ch) > POSITION_SLEEPING )
		send_to_char ("You are already awake...\n", ch);

	else if ( GET_POS (ch) == POSITION_UNCONSCIOUS )
		send_to_char ("You're out cold, I'm afraid.\n", ch);

	else if ( get_second_affect (ch, SPA_KNOCK_OUT, NULL) ) 
		send_to_char ("Your body is still recovering from trauma.", ch);

	else {
		GET_POS (ch) = REST;

		if ( (tch = being_dragged (ch)) ) {
			if ( GET_POS (ch) == REST ) {
				act ("You awake to discover $N is dragging you!",
						FALSE, ch, 0, tch, TO_CHAR);
			} else {
				act ("You awaken to discover $N was dragging you!",
						FALSE, ch, 0, tch, TO_CHAR);
				if ( (af = get_affect (tch, MAGIC_DRAGGER)) )
					affect_remove (tch, af);
			}
		} else {
			if ( GET_POS (ch) == REST )
				act ("You open your eyes.", FALSE, ch, 0, 0, TO_CHAR);
			else
				act ("You awaken and stand.", FALSE, ch, 0, 0, TO_CHAR);
		}

		act ("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
	}
	return;
}


void do_follow (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*leader = NULL;
	CHAR_DATA	*tch = NULL;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char("Follow whom?\n", ch);
		return;
	}

	if ( !(leader = get_char_room_vis (ch, buf)) ) {
		send_to_char ("There is nobody here by that name.\n", ch);
		return;
	}

	if ( IS_MORTAL (ch) && leader != ch && IS_SET (leader->plr_flags, GROUP_CLOSED) ) {
		send_to_char ("That individual's group is currently closed to new followers.\n", ch);
		act ("$n just attempted to join your group.", TRUE, ch, 0, leader, TO_VICT | TO_ACT_FORMAT);
		return;
	}

	if (leader != ch) {
		if ( leader->following == ch ) {
			send_to_char ("You'll need to ask them to stop following you first.\n", ch);
			return;
		}
		while ( leader->following )
			leader = leader->following;
		ch->following = leader;
		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( tch->following == ch )
				tch->following = leader;
		}
		if (!IS_SET(ch->flags, FLAG_WIZINVIS)) snprintf (buf, MAX_STRING_LENGTH,  "You begin following $N.");
		else if (IS_SET(ch->flags, FLAG_WIZINVIS)) snprintf (buf, MAX_STRING_LENGTH,  "You will secretly follow $N.");
		act (buf, FALSE, ch, 0, ch->following, TO_CHAR);
		snprintf (buf, MAX_STRING_LENGTH,  "$n falls into stride with you.");		
		if (!IS_SET(ch->flags, FLAG_WIZINVIS)) act (buf, FALSE, ch, 0, ch->following, TO_VICT | TO_ACT_FORMAT);
		snprintf (buf, MAX_STRING_LENGTH,  "$n falls into stride with $N.");
		if (!IS_SET(ch->flags, FLAG_WIZINVIS)) act (buf, FALSE, ch, 0, ch->following, TO_NOTVICT | TO_ACT_FORMAT);
		return;
	}

	if (leader == ch && ch->following && ch->following != ch) {
		snprintf (buf, MAX_STRING_LENGTH,  "You will no longer follow $N.");
		act(buf, FALSE, ch, 0, ch->following, TO_CHAR);
		snprintf (buf, MAX_STRING_LENGTH,  "$n is no longer following you.");
		if (!IS_SET(ch->flags, FLAG_WIZINVIS)) act (buf, FALSE, ch, 0, ch->following, TO_VICT | TO_ACT_FORMAT);
		snprintf (buf, MAX_STRING_LENGTH,  "$n stops following $N.");		
		if (!IS_SET(ch->flags, FLAG_WIZINVIS)) act (buf, FALSE, ch, 0, ch->following, TO_NOTVICT | TO_ACT_FORMAT);
		ch->following = 0;
		return;
	}

	if (leader == ch && (!ch->following || ch->following == ch)) {
		send_to_char("You aren't following anyone!\n", ch);
		return;
	}
	
	return;
}

void do_sail (CHAR_DATA *ch, char *argument, int cmd)
{
	int			dir = 0;
	int			to_sector = 0;
	int			we_moved = 0;
	char		direction [MAX_INPUT_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*tch = NULL;

	if ( !ch->vehicle ) {
		send_to_char ("You're not on a sailing vessel.\n", ch);
		return;
	}

	if ( !real_skill (ch, SKILL_SAIL) ) {
		send_to_char ("Seek knowledge of sailing first.\n", ch);
		return;
	}

	if ( ch->in_room != ch->vehicle->mob->helm_room ) {
		send_to_char ("Navigation is done from the helm only.\n", ch);
		return;
	}

	argument = one_argument (argument, direction);

	if ( (dir = index_lookup (dirs, direction)) == -1 ) {
		send_to_char ("Sail where?\n", ch);
		return;
	}

	if ( !CAN_GO (ch->vehicle, dir) ) {
		send_to_char ("You can't sail that way.\n", ch);
		return;
	}

	to_sector = vtor (PASSAGE (ch->vehicle, dir)->to_room)->sector_type;

	if ( is_room_affected (vtor (PASSAGE (ch->vehicle, dir)->to_room)->affects,
				MAGIC_ROOM_FLOOD) ) {
		send_to_char ("The flooded land there is too shallow to sail.\n", ch);
		return;
	}

    if ( to_sector != SECT_REEF &&
		 to_sector != SECT_DOCK &&
		 to_sector != SECT_OCEAN ) {
		send_to_char ("Land lies that way.\n", ch);
		return;
	}

	if ( skill_use (ch, SKILL_SAIL, 0) ) {
		do_move (ch->vehicle, "", dir);
		we_moved = 1;
	}

	else if ( number (0, 4) == 4 ) {
		dir = (dir + 1) % 4;

		if ( !CAN_GO (ch->vehicle, dir) ) {
			send_to_char ("The wind stalls.\n", ch);
			return;
		}

		to_sector = vtor (PASSAGE (ch->vehicle, dir)->to_room)->sector_type;

    	if ( to_sector != SECT_REEF &&
			 to_sector != SECT_DOCK &&
			 to_sector != SECT_OCEAN ) {
			send_to_char ("The wind isn't cooperating.\n", ch);
			return;
		}

		do_move (ch->vehicle, "", dir);

		snprintf (buf, MAX_STRING_LENGTH,  "The wind and currents force you off "
					  "course to the %s.\n", dirs [dir]);
		we_moved = 1;
	}

	else
		send_to_char ("The wind stalls.\n", ch);

	if ( we_moved )
		for ( tch = character_list; tch; tch = tch->next ) {

			if ( tch->deleted )
				continue;

			if ( tch->vehicle == ch->vehicle &&
				 (OUTSIDE (tch) || tch->vehicle->mob->helm_room == tch->in_room) )
				act ("$N sails . . .", FALSE, tch, 0, tch->vehicle, TO_CHAR);
		}
		
	return;
}

/*	track finds the shortest path between a character and a room somewhere
    in the mud.
*/

int search_sequence = 0;
int high_water = 0;
int rooms_scanned = 0;

int track_room (ROOM_DATA *from_room, int to_room)
{
	int			room_set_1 [5000];		/* These hardly need to be more than */
	int			room_set_2 [5000];		/*  about 200 elements...5000 tb safe*/
	int			targs = 0;
	int			options = 0;
	int			i = 0;
	int			dir = 0;
	ROOM_DATA	*room = NULL;
	ROOM_DATA	*opt_room = NULL;
	ROOM_DATA	*target_room = NULL;
	int			*rooms = NULL;
	int			*torooms = NULL;
	int			*tmp = NULL;

	if ( !from_room )
		return -1;

	if ( !(target_room = vtor (to_room)) )
		return -1;

	search_sequence++;
	rooms = room_set_1;
	torooms = room_set_2;
	options = 1;
	rooms [0] = to_room;
	high_water = 0;

	while ( options ) {
		targs = 0;
		
		for ( i = 0; i < options; i++ ) {
			opt_room = vtor (rooms [i]);

			for ( dir = 0; dir < 6; dir++ ) {
				if ( !opt_room->dir_option [dir] ||
					  opt_room->dir_option [dir]->to_room == NOWHERE ){
					continue;
				}

				room = vtor (opt_room->dir_option [dir]->to_room);

				if ( !room ) {
					continue;
				}

				if ( !room->dir_option [rev_dir [dir]] ||
					 room->dir_option [rev_dir [dir]]->to_room != 
		 			opt_room->virtual ) {
					continue;
				}

				if ( room->search_sequence == search_sequence ) {
					continue;
				}

				rooms_scanned++;
				room->search_sequence = search_sequence;

				if ( room->virtual == from_room->virtual ){
					return rev_dir [dir];
				}
				
				torooms [targs++] = room->virtual;

				if ( targs > high_water ){
					high_water = targs;
				}
			} /* for ( dir = 0; dir < 6; dir++ ) */
		} /* for ( i = 0; i < options; i++ ) */

		tmp = rooms;
		rooms = torooms;
		torooms = tmp;		/* Important - must point at other structure. */

		options = targs;
	}

	return -1;
}

int track (CHAR_DATA *ch, int to_room)
{
	int			room_set_1 [5000];		/* These hardly need to be more than */
	int			room_set_2 [5000];		/*  about 200 elements...5000 tb safe*/
	int			targs = 0;
	int			options = 0;
	int			i = 0;
	int			dir = 0;
	int			count = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};	/* KILLER CDR */
	ROOM_DATA	*room = NULL;
	ROOM_DATA	*opt_room = NULL;
	ROOM_DATA	*target_room = NULL;
	int			*rooms = NULL;
	int			*torooms = NULL;
	int			*tmp = NULL;

	if ( !(target_room = vtor (to_room)) )
		return -1;

	if ( to_room == ch->in_room )
		return -1;

	search_sequence++;

	rooms = room_set_1;
	torooms = room_set_2;
	options = 1;
	rooms [0] = to_room;
	high_water = 0;

	while ( options ) {
		targs = 0;

		for ( i = 0; i < options; i++ ) {
			opt_room = vtor (rooms [i]);

			for ( dir = 0; dir < 6; dir++ ) {

				if ( !opt_room->dir_option [dir] ||
					 opt_room->dir_option [dir]->to_room == NOWHERE )
					continue;

				room = vtor (opt_room->dir_option [dir]->to_room);

				if ( !room ) {
					snprintf (buf, MAX_STRING_LENGTH,  "Huh?  Room %d -> %d (bad)\n", opt_room->virtual, opt_room->dir_option [dir]->to_room);
					send_to_char (buf, ch);
					continue;
				}

				if ( !room->dir_option [rev_dir [dir]] ||
					  room->dir_option [rev_dir [dir]]->to_room != opt_room->virtual ){ 
					continue;
				}
				
				if ( room->search_sequence == search_sequence ){
					continue;
				}

				rooms_scanned++;
				room->search_sequence = search_sequence;

				if ( GET_TRUST (ch) ) {
					if ( !strncmp (room->description, "No Description Set", 18) ) {
						if ( count % 12 == 11 ){
							send_to_char ("\n", ch);
						}
						
						count++;

						snprintf (buf, MAX_STRING_LENGTH,  "%5d ", room->virtual);
						send_to_char (buf, ch);
					}
				}

				if ( room->virtual == ch->in_room ){
					return rev_dir [dir];
				}
				
				torooms [targs++] = room->virtual;

				if ( targs > high_water ){
					high_water = targs;
				}
			} /* for ( dir = 0; dir < 6; dir++ ) */
		} /* for ( i = 0; i < options; i++ ) */

		tmp = rooms;
		rooms = torooms;
		torooms = tmp;		/* Important - must point at other structure. */

		options = targs;
	} /* while ( options ) */

	if ( GET_TRUST (ch) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "\nTotal rooms: %d\n", count);
		send_to_char (buf, ch);
	}

	return -1;
}

char *crude_name (int race)
{
	if ( race == RACE_HUMAN )
		return "bipedal creature";
	else if ( race == RACE_BOAR || race == RACE_COW ||
			  race == RACE_GOAT || race == RACE_SHEEP ||
			  race == RACE_SWINE )
		return "four-legged, cloven-hoofed creature";
	else if ( race == RACE_BEAR || race == RACE_BOBCAT || race == RACE_HORSE ||
			  race == RACE_PONY || race == RACE_WARHORSE || race == RACE_WOLF )
		return "four-legged creature";
	else return "nameless creature";
}

char *specific_name (int race)
{
	static char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( race == RACE_HUMAN )
		return "medium-sized human";
	
	else if ( race <= RACE_WOLF ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s", db_race_table [race].name);
		*buf = tolower(*buf);
		return buf;
	}
	else return "nameless creature";
}

char *track_age (int hours_passed)
{
	if ( hours_passed <= 1 )
		return "within the hour";
	else if ( hours_passed <= 4 )
		return "recently";
	else if ( hours_passed <= 6 )
		return "within hours";
	else if ( hours_passed <= 12 )
		return "within a half-day";
	else if ( hours_passed <= 24 )
		return "within a day";
	else if ( hours_passed <= 48 )
		return "within a couple days";
	else if ( hours_passed <= 72 )
		return "within a few days";
	else
		return "within days";
}

char *speed_adj (int speed)
{
	if ( speed == 0 )
		return "a brisk walk";
	else if ( speed == 1 )
		return "a faltering stagger";
	else if ( speed == 2 )
		return "a deliberate pace";
	else if ( speed == 3 )
		return "a swift jog";
	else if ( speed == 4 )
		return "a loping run";
	else
		return "a haphazard sprint";
}

void delayed_track (CHAR_DATA *ch)
{
	TRACK_DATA		*track = NULL;
	bool			found = FALSE;
	int				needed = 0;
	char			*p = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			output [MAX_STRING_LENGTH] = {'\0'};
	char			*temp_arg1 = NULL;
	char			*temp_arg2 = NULL;
	char			*temp_arg3 = NULL;
	
	skill_use (ch, SKILL_TRACKING, 0);

	for ( track = ch->room->tracks; track; track = track->next ) {
		*buf = '\0';
		needed = ch->skills [SKILL_TRACKING];
		needed -= track->hours_passed/4;
		
		if ( IS_SET (track->flags, BLOODY_TRACK) ){
			needed += number(10,25);
		}
		
		needed += number (5, 10);
		needed = MAX (needed, 5);
		
		if ( number(1,100) > needed ){
			continue;
		}
		
		if ( !found ){
			send_to_char ("\n", ch);
		}
		
		found = TRUE;
		if ( needed < 30 ) {
			if ( track->from_dir != track->to_dir ){
				temp_arg1 = crude_name(track->race);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2The %stracks of a %s#0 are here, leading from %s to %s.", IS_SET (track->flags, BLOODY_TRACK) ? "#1blood-pooled#0 " : "", temp_arg1, dirs[track->from_dir], dirs[track->to_dir]);
			}
			else{
				temp_arg1 = crude_name(track->race);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2The %stracks of a %s#0 are here, coming from the %s and then doubling back.", IS_SET (track->flags, BLOODY_TRACK) ? "#1blood-pooled#0 " : "", temp_arg1, dirs[track->from_dir]);
			}
		}
		
		else if ( needed < 50 )  {
			if ( track->from_dir != track->to_dir ){
				temp_arg1 = track_age (track->hours_passed);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2A set of %s tracks#0%s were laid here %s, leading from %s to %s.", db_race_table [track->race].name, IS_SET (track->flags, BLOODY_TRACK) ? ", #1pooled with blood#0, " : "", temp_arg1, dirs[track->from_dir], dirs[track->to_dir]);
			}
			else{
				temp_arg1 = track_age (track->hours_passed);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2A set of %s tracks#0%s were laid here %s, coming from the %s and then doubling back.", db_race_table [track->race].name, IS_SET (track->flags, BLOODY_TRACK) ? ", #1pooled with blood#0, " : "", temp_arg1, dirs[track->from_dir]);
			}
		}
		else if ( needed < 70 ) {
			if ( track->from_dir != track->to_dir ){
				temp_arg1 = specific_name (track->race);
				temp_arg2 = speed_adj (track->speed);
				temp_arg3 = track_age (track->hours_passed);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2A set of %s tracks#0%s were laid here %s at %s, leading from %s to %s.", temp_arg1, IS_SET (track->flags, BLOODY_TRACK) ? ", #1pooled with blood#0, " : "", temp_arg2, temp_arg3, dirs[track->from_dir], dirs[track->to_dir]);
			}
			else{
				temp_arg1 = specific_name (track->race);
				temp_arg2 = speed_adj (track->speed);
				temp_arg3 = track_age (track->hours_passed);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2A set of %s tracks#0%s were laid here %s at %s, coming from the %s and then doubling back.", temp_arg1, IS_SET (track->flags, BLOODY_TRACK) ? ", #1pooled with blood#0, " : "", temp_arg2, temp_arg3, dirs[track->from_dir]);
			}
		}
		else {
			if ( track->from_dir != track->to_dir ){
				temp_arg1 = track_age (track->hours_passed);
				temp_arg2 = speed_adj (track->speed);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2A set of %s tracks#0%s were laid here %s at %s, leading from %s to %s.", db_race_table [track->race].name, IS_SET (track->flags, BLOODY_TRACK) ? ", #1pooled with blood#0, " : "", temp_arg1, temp_arg2, dirs[track->from_dir], dirs[track->to_dir]);
			}
			else{
				temp_arg1 = track_age (track->hours_passed);
				temp_arg2 = speed_adj (track->speed);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2A set of %s tracks#0%s were laid here %s at %s, coming from the %s and then doubling back.", db_race_table [track->race].name, IS_SET (track->flags, BLOODY_TRACK) ? ", #1pooled with blood#0, " : "", temp_arg1, temp_arg2, dirs[track->from_dir]);
			}
		}
		*buf = toupper(*buf);
		snprintf (output + strlen(output), MAX_STRING_LENGTH,  "%s ", buf);
	}

	if ( !found ) {
		send_to_char ("You were unable to locate any tracks in the area.\n", ch);
		return;
	}
	else {
		reformat_string (output, &p);
		page_string (ch->desc, p);
		mem_free (p);
	}
	
	return;
}

void skill_tracking (CHAR_DATA *ch, char *argument, int cmd)
{
	TRACK_DATA		*track = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( ch->room->sector_type == SECT_ROAD || ch->room->sector_type == SECT_CITY ) {
		send_to_char ("Tracking in such an area is all but impossible.\n", ch);
		return;
	}

	if ( !IS_MORTAL (ch) ) {	
		if ( !ch->room->tracks ) {
			send_to_char ("There are no tracks here.\n", ch);
			return;
		}
		snprintf (buf, MAX_STRING_LENGTH,  "The following tracks are here:\n\n");
		for ( track = ch->room->tracks; track; track = track->next ) {
			if ( track->from_dir )
				snprintf (ADDBUF, MAX_STRING_LENGTH, "%s tracks, from the %s, heading %s at a %s, left %d hours ago.", db_race_table [track->race].name, dirs[track->from_dir], dirs[track->to_dir], speeds[track->speed], track->hours_passed);
			else
				snprintf (ADDBUF, MAX_STRING_LENGTH, "%s tracks heading to the %s at a %s, left %d hours ago.", db_race_table [track->race].name, dirs[track->to_dir], speeds[track->speed], track->hours_passed);
			if ( IS_SET (track->flags, PC_TRACK) )
				strcat (buf, " #2(PC)#0");
			if ( IS_SET (track->flags, BLOODY_TRACK) )
				strcat (buf, " #1(bloody)#0");
			if ( IS_SET (track->flags, FLEE_TRACK) )
				strcat (buf, " #6(fled)#0");
			strcat (buf, "\n");
		}
		send_to_char (buf, ch);
		return;
	}

	if ( !real_skill (ch, SKILL_TRACKING) ) {
		send_to_char ("You don't know how to track!\n", ch);
		return;
	}

	if ( is_dark (ch->room) && IS_MORTAL(ch) && !get_affect (ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS) ) {
		send_to_char ("You can't see well enough to pick up any tracks.\n", ch);
		return;
	}

	ch->delay = 8;
	ch->delay_type = DEL_TRACK;
	send_to_char ("You survey the area carefully, searching for tracks...\n", ch);
	act ("$n surveys the area slowly, searching for something.", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	
	return;
}

void do_track (CHAR_DATA *ch, char *argument, int cmd)
{
	int			to_room = 0;
	int			dir = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*tch = NULL;
	bool		nodded = FALSE;
	OBJ_DATA	*temp_obj1 = NULL;
	OBJ_DATA	*temp_obj2 = NULL;
	
	if ( !IS_NPC (ch) ) {
		if ( IS_MORTAL(ch) || !*argument ) {
			skill_tracking (ch, argument, cmd);
			return;
		}
	}

	argument = one_argument (argument, buf);

	if ( isdigit (*buf) ){
		to_room = strtol(buf, NULL, 10);
	}
	else if ( (tch = get_char_vis (ch, buf)) ){
		to_room = tch->in_room;
	}
	else {
		send_to_char ("You can't locate them.\n", ch);
		return;
	}

	if ( ch->in_room == to_room ){
		return;
	}
	
	dir = track (ch, to_room);

	if ( dir == -1 ) {
		send_to_char ("Unknown direction.\n", ch);
		return;
	}

	if ( GET_TRUST (ch) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "[High: %d Scan: %d] Move %s\n", high_water, rooms_scanned, dirs [dir]);
		send_to_char (buf, ch);
	}

	high_water = 0;
	rooms_scanned = 0;

	(void)one_argument (ch->room->dir_option [dir]->keyword, buf2);

	snprintf (buf, MAX_STRING_LENGTH,  "%s %s", buf2, dirs[dir]);

	if ( IS_SET (ch->room->dir_option [dir]->exit_info, PASSAGE_LOCKED) ) {
		if ( ch->room->dir_option[dir]->key ) {
			for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
				temp_obj1 = get_obj_in_list_num (ch->room->dir_option[dir]->key, tch->right_hand);
				temp_obj2 = get_obj_in_list_num (ch->room->dir_option[dir]->key, tch->left_hand);
				if (temp_obj1 || temp_obj2){
					name_to_ident (tch, buf2);
					snprintf (buf, MAX_STRING_LENGTH,  "%s", buf2);
					do_nod (ch, buf, 0);
					nodded = TRUE;
					break;
				}
			}
		}

		if ( !nodded ){
			do_knock (ch, buf, 0);
		}
	}

	else if ( IS_SET (ch->room->dir_option [dir]->exit_info, PASSAGE_CLOSED)){
		do_open (ch, buf, 0);
	}

	do_move (ch, "", dir);

	return;
}

void do_drag (CHAR_DATA *ch, char *argument, int cmd)
{
	int				dir = 0;
	CHAR_DATA		*victim = NULL;
	OBJ_DATA		*obj = NULL;
	AFFECTED_TYPE	*af = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			name [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, name);

	if ( !(victim = get_char_room_vis (ch, name)) ) {
		if ( !(obj = get_obj_in_list_vis (ch, name, ch->room->contents)) ) {
			send_to_char ("Drag who or what?\n", ch);
			return;
		}
	}

	if ( victim ) {
		if ( ch == victim ) {
			send_to_char ("Drag yourself?\n", ch);
			return;
		}

		if ( IS_SWIMMING (ch) ) {
			send_to_char ("You can barely tred water yourself!\n", ch);
			return;
		}
	}

	argument = one_argument (argument, buf);

	if ( (dir = index_lookup (dirs, buf)) == -1 ) {
		send_to_char ("Drag where?\n", ch);
		return;
	}

	if ( victim && AWAKE (victim) ) {
		act ("$N is conscious, you can't drag $M.",
				FALSE, ch, 0, victim, TO_CHAR);
		return;
	}
	
	if ( victim && number (1, 4) == 1 ) {
		do_wake (ch, name, 1);
		if ( GET_POS (victim) > SLEEP )
			return;
	}

	if ( victim && IS_SUBDUER (ch) ) {
		send_to_char ("You can't drag anything while you have someone in tow.\n", ch);
		return;
	}

	if ( obj && !IS_SET (obj->obj_flags.wear_flags, ITEM_TAKE) ) {
		act ("$p is firmly attached.  You can't drag it.",
						FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	magic_add_affect (ch, MAGIC_DRAGGER, -1, 0, 0, 0, 0);

	if ( (af = get_affect (ch, MAGIC_DRAGGER)) )
		af->a.spell.t = obj ? (int) obj : (int) victim;

	do_move (ch, "", dir);
	
	return;
}

int is_mounted (CHAR_DATA *ch)
{
		/* Is a mount mounted, or has a PC mounted something */

	if ( !ch->mount || !is_he_here (ch, ch->mount, FALSE) )
		return 0;

	if ( ch != ch->mount->mount ) {
		ch->mount = NULL;
		return 0;
	}

	return 1;
}

int can_mount (CHAR_DATA *ch, CHAR_DATA *mount)
{
	if ( CAN_SEE (ch, mount) &&
		 IS_SET (mount->act, ACT_MOUNT) &&
		 !IS_SET (ch->act, ACT_MOUNT) &&
		 GET_POS (mount) == STAND &&
		 !is_mounted (mount) )
		return 1;

	return 0;
}

int hitch_char (CHAR_DATA *ch, CHAR_DATA *hitch)
{
	if ( IS_HITCHER (ch) )
		return 0;

	if ( !IS_SET (hitch->act, ACT_MOUNT) )
		return 0;

	if ( IS_HITCHEE (hitch) )
		return 0;

	ch->hitchee = hitch;
	hitch->hitcher = ch;

	return 1;
}

void unhitch_char (CHAR_DATA *ch, CHAR_DATA *hitch)
{
	if ( ch != hitch->hitcher || ch->hitchee != hitch )
		return;

	ch->hitchee = NULL;
	hitch->hitcher = NULL;
	return;
}

void do_hitch (CHAR_DATA *ch, char *argument, int cmd)
{
	int			is_vehicle = 0;
	CHAR_DATA	*hitch = NULL;
	CHAR_DATA	*hitcher = NULL;
	OBJ_DATA	*bridle = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !(hitch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf && IS_HITCHER (ch) ) {
		act ("You currently have $N hitched.",
				TRUE, ch, 0, ch->hitchee, TO_CHAR);
		return;
	}

	if ( hitch->mob && hitch->mob->vehicle_type == VEHICLE_HITCH )
		is_vehicle = 1;

	if ( !IS_SET (hitch->act, ACT_MOUNT) ) {
		act ("$N can't be hitched.", TRUE, ch, 0, hitch, TO_CHAR);
		return;
	}

	if ( IS_HITCHEE (hitch) ) {
		act ("$N is already hitched to $3.",
					TRUE, ch, (OBJ_DATA *) hitch->hitcher, hitch, TO_CHAR);
		return;
	}

	if ( IS_RIDEE (hitch) ) {
		act ("$N is riding that animal.", TRUE, ch, 0, hitch->mount, TO_CHAR);
		return;
	}

	if ( !is_vehicle ) {
		if ( !(bridle = get_equip (hitch, WEAR_NECK_1)) ) {
			act ("$N doesn't have a bridle to grab.",
						TRUE, ch, 0, hitch, TO_CHAR);
			return;
		}

		if ( GET_ITEM_TYPE (bridle) != ITEM_BRIDLE ) {
			act ("$N is wearing $o, which is not a bridle.",
					TRUE, ch, bridle, hitch, TO_CHAR);
			return;
		}
	}

	if ( !*buf && is_vehicle ) {
		send_to_char ("You may only hitch a vehicle to an animal or another "
					  "vehicle.\n", ch);
		return;
	}

	if ( !*buf ) {

		hitch_char (ch, hitch);

		act ("You grab the reins of $N.", TRUE, ch, 0, hitch, TO_CHAR);
		act ("$n grabs the reins of $N.", FALSE, ch, 0, hitch, TO_NOTVICT);
		act ("$N grabs your reins.", TRUE, hitch, 0, ch, TO_CHAR);

		return;
	}

	if ( !str_cmp (buf, "to") )
		argument = one_argument (argument, buf);

	if ( !(hitcher = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	if ( !IS_SET (hitcher->act, ACT_MOUNT) ) {
		act ("$N can't be hitched.", TRUE, ch, 0, hitcher, TO_CHAR);
		return;
	}

	if ( IS_HITCHER (hitcher) ) {
		act ("$N has an animal hitched already.",
				TRUE, ch, 0, hitcher, TO_CHAR);
		return;
	}

	hitch_char (hitcher, hitch);

	act ("$n hitches $N to $3.",
				FALSE, ch, (OBJ_DATA *) hitcher, hitch, TO_NOTVICT);
	act ("$N hitches $3 to you.",
				FALSE, ch, (OBJ_DATA *) hitch, hitcher, TO_VICT);
	act ("You hitch $3 to $N.",
				TRUE, ch, (OBJ_DATA *) hitch, hitcher, TO_CHAR);
	act ("$N hitches you to $3.",
				TRUE, hitch, (OBJ_DATA *) hitcher, ch, TO_CHAR);
				
	return;
}

void do_unhitch (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*hitch = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {

		if ( !is_he_here (ch, ch->hitchee, 0) ) {
			send_to_char ("Unhitch what?\n", ch);
			return;
		}

		hitch = ch->hitchee;
	}

	else if ( !(hitch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	if ( !IS_HITCHEE (hitch) ) {
		act ("$N isn't currently hitched.", TRUE, ch, 0, hitch, TO_CHAR);
		return;
	}

		/* Mount is hitched, but not by us.  Check for non-mount hitch */

	if ( hitch->hitcher != ch && !IS_SET (hitch->hitcher->act, ACT_MOUNT) ) {
		act ("$N is holding that animal's reigns.",
					TRUE, ch, 0, hitch->hitcher, TO_CHAR);
		return;
	}

	unhitch_char (hitch->hitcher, hitch);

	act ("$n unhitches $N.", TRUE, ch, 0, hitch, TO_NOTVICT);
	act ("You unhitch $N.", TRUE, ch, 0, hitch, TO_CHAR);
	act ("$N unhitches you.", TRUE, hitch, 0, ch, TO_CHAR);
	
	return;
}

void do_mount (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*mount = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		for ( mount = ch->room->people; mount; mount = mount->next_in_room )
			if ( can_mount (ch, mount) )
				break;

		if ( !mount ) {
			send_to_char ("There is nothing here you can mount.\n", ch);
			return;
		}
	}

	else if ( !(mount = get_char_room_vis (ch, buf)) ) {
		send_to_char ("There is no such mount here.\n", ch);
		return;
	}

	if ( !can_mount (ch, mount) ) {
		act ("You can't mount $N.", FALSE, ch, 0, mount, TO_CHAR);
		return;
	}

	if ( !real_skill (ch, SKILL_RIDE) && mount->skills [SKILL_RIDE] < 33 ) {
		act ("$N is too wild for you to even try.",
					FALSE, ch, 0, mount, TO_CHAR);
		return;
	}

	if ( real_skill (mount, SKILL_RIDE) < 33 && !skill_use (ch, SKILL_RIDE, 0) ) {
		act ("$N backs away just as you attempt to mount it.",
					FALSE, ch, 0, mount, TO_CHAR);
		act ("You back away as $N approaches you.",
					FALSE, mount, 0, ch, TO_CHAR);
		act ("$N backs away when $n attempts to mount it.",
					FALSE, ch, 0, mount, TO_NOTVICT);
		return;
	}

	if ( mount->skills [SKILL_RIDE] < 90 && !skill_use (mount, SKILL_RIDE, 0) ){
		act ("You manage to straddle $N, but it quickly throws you off.",
					FALSE, ch, 0, mount, TO_CHAR);
		act ("$n is thrown to the ground after attempting to mount $N.",
					FALSE, ch, 0, mount, TO_NOTVICT);
		act ("You throw $N to the ground after $E attempts to mount you.",
					FALSE, mount, 0, ch, TO_CHAR);
		return;
	}

	act ("$n perches $mself on top of $N.",
					FALSE, ch, 0, mount, TO_NOTVICT);
	act ("You mount $N.", FALSE, ch, 0, mount, TO_CHAR);
	act ("$N mounts you.", FALSE, mount, 0, ch, TO_CHAR);

	mount->mount = ch;
	ch->mount = mount;

	unhitch_char (ch, mount);
	
	return;
}

void do_tame (CHAR_DATA *ch, char *argument, int cmd)
{
	send_to_char ("This command is under construction.\n", ch);
	return;
}

void do_dismount (CHAR_DATA *ch, char *argument, int cmd)
{
	if ( !is_mounted (ch) ) {
		ch->mount = NULL;
		send_to_char ("You're not on a mount.\n", ch);
		return;
	}

	act ("$n dismounts from $N.", FALSE, ch, 0, ch->mount, TO_NOTVICT);
	act ("You dismount from $N.", FALSE, ch, 0, ch->mount, TO_CHAR);
	act ("$N gets off of you.", FALSE, ch->mount, 0, ch, TO_CHAR);

	hitch_char (ch, ch->mount);

	ch->mount->mount = NULL;
	ch->mount = NULL;
	
	return;
}

/*
Character          Skill       Meaning
----------         --------    -------------------------------------
  PC               Tame        No meaning
  PC               Ride        If the mount bucks, the rider falls if his
                                 RIDE skill check fails.
                               If RIDE skill is 0 for PC, he cannot attempt
                                 to mount a horse with RIDE skill < 33
  PC               Break       If PC tries to bridle a mount with no TAME:
                                 If PC's BREAK < 50, mount shies away, else
                                 If PC passes BREAK skill check:                
                                   TAME is opened on mount, and is bridled
                                 else 
                                   PC gets kicked for 2d12 and fails bridle     
                               NOTE:  I intended to create a break command, but 
                                      never did, so BREAK skill isn't complete. 
  mob              Tame        If mount has TAME >= 33, PC can always mount.
                               If mount has 0 < TAME < 33, mount will shy bridle
                                 1d100 < mount's TAME * 3
  mob              Ride        If
*/

void do_bridle (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*mount = NULL;
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*bridle = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;

	argument = one_argument (argument, buf);

	if ( IS_HITCHER (ch) ) {
		act ("Unhitch $N first.", TRUE, ch, 0, ch->hitchee, TO_CHAR);
		return;
	}

	if ( !*buf ) {
		send_to_char ("What do you want to bridle?\n", ch);
		return;
	}

	if ( !(mount = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that animal here.\n", ch);
		return;
	}

	if ( !IS_SET (mount->act, ACT_MOUNT) ) {
		act ("$N is not a mount; it can't be bridled.",
				FALSE, ch, 0, mount, TO_CHAR);
		return;
	}

	if ( !(bridle = ch->right_hand) ) {
		send_to_char ("You need to hold a bridle in your right hand.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (bridle) != ITEM_BRIDLE ) {
		act ("$o is not a bridle.", FALSE, ch, bridle, 0, TO_CHAR);
		return;
	}

	if ( (obj = get_equip (mount, WEAR_NECK_1)) ) {
		act ("$N already has $o around its neck.",
					FALSE, ch, obj, mount, TO_CHAR);
		return;
	}

	if ( !real_skill (mount, SKILL_RIDE) ) {

		if ( ch->skills [SKILL_RIDE] < 30 ) {
			act ("$N shies away as attempt to slip $o onto $M.",
					FALSE, ch, bridle, mount, TO_CHAR);
			return;
		}

		if ( !skill_use (ch, SKILL_RIDE, 0) ) {
			act ("$N kicks you as you attempt to bridle $M!",
					FALSE, ch, 0, mount, TO_CHAR);
			act ("You kick $N when $E tries to bridle you.",
					FALSE, mount, 0, ch, TO_CHAR);
			act ("$N kicks $n as $E attempts to bridle $M.",
					FALSE, ch, 0, mount, TO_NOTVICT);
			temp_arg = figure_location(ch, 0);
			wound_to_char (ch, temp_arg, dice(2,12), 3, 0, 0, 0);

			return;
		}

		open_skill (mount, SKILL_TAME);
	}

	else if ( mount->skills [SKILL_RIDE] < 33 ) {

		if ( number (1, 100) < mount->skills [SKILL_RIDE] * 3 ) {
			act ("$N shies away as attempt to slip $p onto $M.",FALSE, ch, bridle, mount, TO_CHAR);
            return;
		}
	}

	act ("You slip $p onto $N.", FALSE, ch, bridle, mount, TO_CHAR);
	act ("$N slips $p over your neck.", FALSE, mount, bridle, ch, TO_CHAR);
	act ("$n slips $p over $N's neck.", FALSE, ch, bridle, mount, TO_NOTVICT);

	obj_from_char (&bridle, 0);

	equip_char (mount, bridle, WEAR_NECK_1);

	hitch_char (ch, mount);
	
	return;
}

void dump_rider (CHAR_DATA *rider, int forced)
{
	if ( !IS_RIDER (rider) )
		return;

	if ( forced || !skill_use (rider, SKILL_RIDE, 0) ) {
		act ("You throw $N off your back.  HEHE HAHA, Walk man, walk!",
				FALSE, rider->mount, 0, rider, TO_CHAR);
		act ("$N bucks and thows you off!",
				FALSE, rider, 0, rider->mount, TO_CHAR);
		act ("$N bucks $n off its back.",
				TRUE, rider, 0, rider->mount, TO_NOTVICT);

		rider->mount->mount = NULL;
		rider->mount = NULL;

		GET_POS (rider) = SIT;
        add_second_affect (SPA_STAND, 5, rider, NULL, NULL, 0);

		return;
 	}

	act ("You buck as you pretend to be REAL scared.",
				FALSE, rider->mount, 0, rider, TO_CHAR);
	act ("$N bucks and thows you off!",
				FALSE, rider, 0, rider->mount, TO_CHAR);
	act ("$N bucks $n off its back.",
				TRUE, rider, 0, rider->mount, TO_NOTVICT);
				
	return;
}

void do_buck (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !IS_SET (ch->act, ACT_MOUNT) ) {
		send_to_char ("This is a mount only command.\n", ch);
		return;
	}

	if ( !IS_RIDEE (ch) ) {
		send_to_char ("You're not riding anybody.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf == '!' )
		dump_rider (ch->mount, TRUE);
	else
		dump_rider (ch->mount, FALSE);
		
	return;
}

void do_shadow (CHAR_DATA *ch, char *argument, int cmd)
{
	AFFECTED_TYPE	*af = NULL;
	CHAR_DATA		*tch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		if ( !(af = get_affect (ch, AFFECT_SHADOW)) ||
			 !af->a.shadow.shadow ) {
			send_to_char ("You aren't shadowing anybody.\n", ch);
			return;
		}

		send_to_char ("Ok.\n", ch);

		af->a.shadow.shadow = NULL;

		return;
	}

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("They aren't here to shadow.\n", ch);
		return;
	}

	if ( tch == ch ) {
		if ( (af = get_affect (ch, AFFECT_SHADOW)) ) {
			send_to_char ("Ok, you stop shadowing.\n", ch);
			af->a.shadow.shadow = NULL;
			return;
		}

		send_to_char ("Ok.\n", ch);
		return;
	}

	magic_add_affect (ch, AFFECT_SHADOW, -1, 0, 0, 0, 0);

	af = get_affect (ch, AFFECT_SHADOW);

	af->a.shadow.shadow = tch;
	af->a.shadow.edge = -1;			/* Not on an edge */

	act ("You will attempt to shadow $N.", FALSE, ch, 0, tch, TO_CHAR);
	
	return;
}

void shadowers_shadow (CHAR_DATA *ch, int to_room, int move_dir)
{
	int					dir = 0;
	CHAR_DATA			*tch = NULL;
	CHAR_DATA			*next_in_room = NULL;
	AFFECTED_TYPE		*af = NULL;
	MOVE_DATA       	*move = NULL;
	ROOM_DIRECTION_DATA	*exit_room = NULL;
	ROOM_DATA			*room = NULL;

	/* ch is leaving in direction dir.  Pick up people in same room and
       have them move to the edge.  Pick up people in surrounding rooms on
       edges and have them move to edge of ch's current room too. */

		/* Handle current room shadowers first */

	for ( tch = ch->room->people; tch; tch = next_in_room ) {

		next_in_room = tch->next_in_room;

		if ( ch == tch )
			continue;

		if ( GET_FLAG (tch, FLAG_LEAVING) || GET_FLAG (tch, FLAG_ENTERING) )
			continue;

		if ( tch->moves )
			continue;

		if ( !(af = get_affect (tch, AFFECT_SHADOW)) )
			continue;

		if ( af->a.shadow.shadow != ch )
			continue;

		if ( !CAN_SEE (tch, ch) )
			continue;

		move = (MOVE_DATA *)alloc (sizeof (MOVE_DATA), 24);
	
		move->dir = move_dir;
		move->flags = MF_TOEDGE;
		move->desired_time = 0;
		move->next = NULL;
		move->travel_str = NULL;

		tch->moves = move;

		initiate_move (tch);
	}

		/* Handle people who are on the edge of this room.  Those people
           will head to the edge of this room joining the room that ch is
           about to enter. */

	for ( dir = 0; dir < 6; dir++ ) {

		if ( !(exit_room = PASSAGE (ch, dir)) )
			continue;

		if ( exit_room->to_room == to_room )		/* The shadowee is returning */
			continue;

		if ( !(room = vtor (exit_room->to_room)) )
			continue;

		for ( tch = room->people; tch; tch = next_in_room ) {

			next_in_room = tch->next_in_room;

			if ( ch == tch )
				continue;		/* Hopefully not possible */

			if ( GET_FLAG (tch, FLAG_LEAVING) || GET_FLAG (tch, FLAG_ENTERING) )
				continue;
			
			if ( tch->moves )
				continue;

			if ( !(af = get_affect (tch, AFFECT_SHADOW)) )
				continue;

			if ( af->a.shadow.shadow != ch )
				continue;

			if ( af->a.shadow.edge != rev_dir [dir] )
				continue;

			if ( !could_see (tch, ch) )		/* Make sure shadowee is visable */
				continue;

				/* Make N/PC enter room of ch as ch leaves */

			move = (MOVE_DATA *)alloc (sizeof (MOVE_DATA), 24);

			move->dir = af->a.shadow.edge;
			move->flags = MF_TONEXT_EDGE;
			move->desired_time = 0;
			move->next = NULL;
			move->travel_str = NULL;

			tch->moves = move;
			
				/* Make N/PC move to edge joining room ch just entered */

			move = (MOVE_DATA *)alloc (sizeof (MOVE_DATA), 24);

			move->dir = move_dir;
			move->flags = MF_TOEDGE;
			move->desired_time = 0;
			move->next = NULL;
			move->travel_str = NULL;

			tch->moves->next = move;

			initiate_move (tch);
		}
	}
	
	return;
}

int get_uniq_ticket (void)
{
	int			tn = 1;
	int			i = 0;
	FILE		*fp_ls = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !(fp_ls = popen ("ls tickets", "r")) ) {
		system_log ("The ticket system is broken, get_uniq_ticket()", TRUE);
		return -1;
	}

		/* The TICKET_DIR should be filled with files that have seven
           digit names (zero padded on the left).
		*/

	while ( !feof (fp_ls) ) {

		if ( !fgets (buf, 80, fp_ls) )
			break;

		for ( i = 0; i < 7; i++ )
			if ( !isdigit (buf [i]) )
				continue;

		if ( tn != strtol(buf, NULL, 10))
			break;

		tn = strtol(buf, NULL, 10) + 1;
	}

	pclose (fp_ls);

	return tn;
}

void do_stable (CHAR_DATA *ch, char *argument, int cmd)
{
	int			ticket_num = 0;
	int			i = 0;
	CHAR_DATA	*animal = NULL;
	CHAR_DATA	*new_hitch = NULL;
	CHAR_DATA	*keeper = NULL;
	AFFECTED_TYPE	*af = NULL;
	OBJ_DATA	*ticket = NULL;
	FILE		*fp = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	bool		paid_for = FALSE;
	char		*temp_arg = NULL;

	for ( keeper = ch->room->people; keeper; keeper = keeper->next_in_room )
		if ( keeper != ch &&
			 keeper->mob &&
			 IS_SET (keeper->flags, FLAG_KEEPER) &&
			 IS_SET (keeper->flags, FLAG_OSTLER) )
			break;

	if ( !keeper ) {
		send_to_char ("There is no ostler here.\n", ch);
		return;
	}

	if ( !ch->hitchee || !is_he_here (ch, ch->hitchee, 0) ) {
		send_to_char ("You have no hitch.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf ) {
		if ( !(animal = get_char_room_vis (ch, buf)) ) {
			send_to_char ("There isn't such an animal here.\n", ch);
			return;
		}

		if ( animal != ch->hitchee ) {
			act ("$N isn't hitched to you.", FALSE, ch, 0, animal, TO_CHAR);
			return;
		}
	}

			/* Make sure mount isn't already mounted */

	if ( ch->hitchee->mount && is_he_here (ch, ch->hitchee->mount, 0) ) {
		name_to_ident (ch->hitchee->mount, buf2);
		temp_arg = char_short (ch->hitchee->mount);
		snprintf (buf, MAX_STRING_LENGTH,  "tell %s Have #5%s#0 dismount your hitch first.", buf2, temp_arg);
		command_interpreter (keeper, buf);
	}

	animal = ch->hitchee;

	i = MAGIC_STABLING_PAID;
	while ( (af = get_affect (ch, i)) ) {
		if ( af->a.spell.sn == animal->coldload_id ) {
			paid_for = TRUE;
 			break;
		}
		i++;
		if ( i > MAGIC_STABLING_LAST ) {
			i = MAGIC_STABLING_PAID;
			break;
		}
	}

	if ( !paid_for && !is_brother (ch, keeper) && !can_subtract_money (ch, 20, keeper->mob->currency_type) ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,  "%s You seem to be a bit short on coin right now.", buf2);
		do_whisper (keeper, buf, 0);
		return;
	} 

        if ( !paid_for ) {
        	CREATE (af, AFFECTED_TYPE, 1);
                af->type = i;
		af->a.spell.sn = animal->coldload_id;
		af->a.spell.duration = 168;
                affect_to_char (ch, af);
		if ( !is_brother (ch, keeper) )
			subtract_money (ch, 20, keeper->mob->currency_type);
	}

	if ( (ticket_num = get_uniq_ticket ()) == -1 ) {
		send_to_char ("OOC:  The ticket system is broken.  Sorry.\n", ch);
		return;
	}

	unhitch_char (ch, animal);

			/* Take reins of hitches hitch, if chained */

	if ( animal->hitchee && is_he_here (animal, animal->hitchee, 0) ) {

		new_hitch = animal->hitchee;
		unhitch_char (animal, new_hitch);

		if ( hitch_char (ch, new_hitch) )
			act ("You take the reigns of $N.",
					FALSE, ch, 0, new_hitch, TO_CHAR);
	}

	ticket = load_object (VNUM_TICKET);

	if ( !ticket ) {
		send_to_char ("OOC:  Your hitch could not be saved.  Please report "
					  "this as a problem.\n", ch);
		return;
	}

	ticket->o.ticket.ticket_num = ticket_num;
	ticket->o.ticket.keeper_vnum = keeper->mob->virtual;
	ticket->o.ticket.stable_date = (int)time(0);

	obj_to_char (ticket, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "The number %d is scrawled on this ostler's ticket.",
				  ticket->o.ticket.ticket_num);
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n\n#6OOC: To retrieve your mount, GIVE this ticket to the ostler\n"
                      "     with whom you stabled it; be sure you don't lose this!#0"); 
	ticket->full_description = str_dup (buf);

	act ("$N gives you $p.", FALSE, ch, ticket, keeper, TO_CHAR | TO_ACT_FORMAT);
	act ("You give $N $p.", FALSE, keeper, ticket, ch, TO_CHAR | TO_ACT_FORMAT);
	act ("$N gives $n $p.", FALSE, ch, ticket, keeper, TO_NOTVICT | TO_ACT_FORMAT);

	act ("$N leads $n to the stables.",
			FALSE, animal, 0, keeper, TO_ROOM | TO_ACT_FORMAT);
	
	snprintf (buf, MAX_STRING_LENGTH,  TICKET_DIR "/%07d", ticket_num);

	if ( !(fp = fopen (buf, "w")) ) {
		perror (buf);
		system_log ("Unable to save ticketed mobile to file.", TRUE);
		return;
	}

	save_mobile (animal, fp, "HITCH", 1);		/* Extracts the mobile */

	fclose (fp);
	
	return;
}

void unstable (CHAR_DATA *ch, OBJ_DATA *ticket, CHAR_DATA *keeper)
{
	CHAR_DATA	*back_hitch = NULL;
	CHAR_DATA	*animal = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		ticket_file [MAX_STRING_LENGTH] = {'\0'};

	if ( !keeper ) {
		for ( keeper = ch->room->people; keeper; keeper = keeper->next_in_room )
			if ( keeper != ch &&
				 keeper->mob &&
				 IS_SET (keeper->flags, FLAG_KEEPER) &&
				 IS_SET (keeper->flags, FLAG_OSTLER) )
				break;

		if ( !keeper ) {
			send_to_char ("There is no ostler here.\n", ch);
			return;
		}
	}

	if ( !keeper->mob ) {		/* Can happen if given specifically to a PC */
		send_to_char ("Only NPCs can be ostlers.\n", ch);
		return;
	}

	if ( !CAN_SEE (keeper, ch) ) {
		act ("It appears that $N can't see you.",
					FALSE, ch, 0, keeper, TO_CHAR);
		return;
	}

	if ( ticket->o.ticket.keeper_vnum != keeper->mob->virtual ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,  "tell %s Sorry, that's not one of my tickets.", buf2);
		command_interpreter (keeper, buf);
		return;
	}

	snprintf (ticket_file, MAX_STRING_LENGTH, TICKET_DIR "/%07d", ticket->o.ticket.ticket_num);

	if ( stat (ticket_file, (struct stat *) buf2) == -1 ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,  "tell %s Yeah, that's my ticket, but I don't have "
					  "anyting in that stall.", buf2);
		command_interpreter (keeper, buf);
		return;
	}

	if ( is_he_here (ch, ch->hitchee, 0) ) {
		back_hitch = ch->hitchee;
		unhitch_char (ch, back_hitch);
	}

	animal = load_saved_mobiles (ch, ticket_file);
	if ( ticket->o.od.value[0] )
		offline_healing (animal, ticket->o.ticket.stable_date);

	if ( !animal ) {
		send_to_char ("OOC:  See an admin.  There is something wrong with that "
					  "ticket.\n", ch);
		if ( back_hitch )
			hitch_char (ch, back_hitch);

		return;
	}

	if ( back_hitch && !ch->hitchee )
		hitch_char (ch, back_hitch);
	else if ( back_hitch )
		hitch_char (ch->hitchee, back_hitch);

	extract_obj (ticket);

	save_char (ch, TRUE);
	unlink (ticket_file);

	act ("$N trots $3 from the stables and hands you the reins.",
			FALSE, ch, (OBJ_DATA *) animal, keeper, TO_CHAR | TO_ACT_FORMAT);
	act ("$N trots $3 from the stables and hands $n the reins.",
			FALSE, ch, (OBJ_DATA *) animal, keeper, TO_NOTVICT | TO_ACT_FORMAT);
	act ("You trot $3 from the stables and hand $N the reins.",
			FALSE, keeper, (OBJ_DATA *) animal, ch, TO_CHAR | TO_ACT_FORMAT);
			
	return;
}

void stop_tolls (CHAR_DATA *ch)
{
	AFFECTED_TYPE	*af = NULL;

	if ( !(af = get_affect (ch, MAGIC_TOLL)) )
		return;

	if ( af->a.toll.room_num == ch->in_room ) {
		send_to_char ("You stop collecting tolls.\n", ch);
		act ("$n stops collecting tolls.", TRUE, ch, 0, 0, TO_ROOM);
	}

	remove_affect_type (ch, MAGIC_TOLL);
	
	return;
}

CHAR_DATA *levy_collector (ROOM_DATA *room, int dir)
{
	CHAR_DATA		*tch = NULL;
	AFFECTED_TYPE	*af = NULL;

	for ( tch = room->people; tch; tch = tch->next_in_room ) {

		if ( tch->deleted )
			continue;

		if ( !(af = get_affect (tch, MAGIC_TOLL)) )
			continue;

		if ( af->a.toll.dir != dir ||
			 af->a.toll.room_num != room->virtual )
			continue;

		return tch;
	}

	return NULL;
}

void do_toll (CHAR_DATA *ch, char *argument, int cmd)
{
	int					dir = 0;
	CHAR_DATA			*tch = NULL;
	AFFECTED_TYPE		*af = NULL;
	ROOM_DIRECTION_DATA	*exit_room = NULL;
	char				buf [MAX_STRING_LENGTH] = {'\0'};

/*

Help:

TOLL

Setup a toll crossing.  Any character may setup a toll crossing in an area
that supports tolls.  All humans are stopped and expected to pay a toll of
your choosing.

   > toll <direction> <amount to collect>

Example:

   > toll north 5         Setup a toll crossing to the North.  The toll is
                          5 coins.

Non-humans are not expected to pay tolls, since they usually don't have money.
People who you cannot see can pass without paying toll, if they choose.

See also:  PAY, SET

*/

	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {
		send_to_char ("toll <direction> <amount to collect>\n", ch);
		return;
	}

	if ( (dir = index_lookup (dirs, buf)) == -1 ) {
		send_to_char ("Expected a direction:  North, South, East, or West.\n"
					  "toll <direction> <amount to collect>\n", ch);
		return;
	}

	if ( (tch = levy_collector (ch->room, dir)) && tch != ch ) {
		act ("$N is already collecting a toll here.\n",
				FALSE, ch, 0, tch, TO_CHAR);
		return;
	}

	if ( !(exit_room = PASSAGE (ch, dir)) ) {
		send_to_char ("There is no exit there.\n", ch);
		return;
	}

	if ( !IS_SET (exit_room->exit_info, PASSAGE_TOLL) ) {
		send_to_char ("It isn't possible to collect tolls in that direction.\n",
					  ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !just_a_number (buf) || strtol(buf, NULL, 10) < 1 ) {
		send_to_char ("How much will your charge for others for the toll?\n"
					  "toll <direction> <amount to collect>\n", ch);
		return;
	}

	if ( tch ) {		/* We just want to modify direction or toll charge */
		af = get_affect (ch, MAGIC_TOLL);

		af->a.toll.dir = dir;
		af->a.toll.charge = strtol(buf, NULL, 10);
		af->a.toll.room_num = ch->in_room;

		snprintf (buf, MAX_STRING_LENGTH,  "You will collect %d penn%s when people pass %s.\n",
					  af->a.toll.charge, af->a.toll.charge > 1 ? "ies" : "y",
					  dirs [af->a.toll.dir]);
		send_to_char (buf, ch);

		return;
	}

	magic_add_affect (ch, MAGIC_TOLL, -1, 0, 0, 0, 0);

	af = get_affect (ch, MAGIC_TOLL);

	af->a.toll.dir = dir;
	af->a.toll.charge = strtol(buf, NULL, 10);
	af->a.toll.room_num = ch->in_room;

	snprintf (buf, MAX_STRING_LENGTH,  "You will collect %d penn%s when people pass %s.\n",
				  af->a.toll.charge, af->a.toll.charge > 1 ? "ies" : "y",
				  dirs [af->a.toll.dir]);
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "$n stands %s, ready to collect tolls.", dirs [dir]);
	act (buf, TRUE, ch, 0, 0, TO_ROOM);
	
	return;
}
