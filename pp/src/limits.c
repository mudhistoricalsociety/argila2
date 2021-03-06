/** 
*	\file limits.c
*	Gain Control Module
*
*	This module is to provides limitiations on skill use, and deals with updates
*	on rooms, connection status, sleep and movement.
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
#include <time.h>
#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "math.h"

int move_gain (CHAR_DATA *ch)
{
	int gain;
	int move_rate;
	int moves_gained;

		/* Move_rate is 100 moves per 5 mins  ( * 10000 for granularity ) */

	move_rate = 10000 * GET_CON (ch) * UPDATE_PULSE / ( 12 * 16 );
			/*  if GET_CON < 12 then move_rate < 10000 */ 
			/*  if GET_CON > 12 then move_rate > 10000 */
			/*  Higher con means you can recover faster */
	gain = move_rate; /*  base value */

	switch ( GET_POS (ch) ) { /*  changes due to position */
		case POSITION_SLEEPING:	gain += (gain); break;
		case POSITION_RESTING:	gain += (gain/2); break;
		case POSITION_SITTING:	gain += (gain/4); break;
		default:	gain = (gain + 0); break;
	}

/*  changes due to hunger/thirst */
	if( !ch->hunger || !ch->thirst )
		gain += (gain/2);

	if ( ch->move_points < 0 )
		ch->move_points = 0;

/* granularity */
	moves_gained = gain/10000;
	ch->move_points += moves_gained;

/* special case for Guests in OOC rooms */
	if ( IS_SET(ch->room->room_flags, OOC) && !IS_SET (ch->flags, FLAG_GUEST) )
		moves_gained = 0;

	return moves_gained;
}


int sleep_needed_in_seconds (CHAR_DATA *ch)
{
	if ( !ch->pc )
		return 0;

	return ch->pc->sleep_needed / 100000;
}

void sleep_credit (CHAR_DATA *ch)
{
		/* We're aiming for straight credit for sleeping, which will
           be 5 seconds per update if UPDATE_PULSE is 5 * 4 (5 seconds) */

	if ( !ch->pc || !ch->desc )
		return;

	ch->pc->sleep_needed -= (UPDATE_PULSE / 4) * 100000;

	if ( ch->pc->sleep_needed < 0 )
		ch->pc->sleep_needed = 0;
}

void sleep_need (CHAR_DATA *ch)
{
	int			need;

	if ( !ch->pc )
		return;
	else			/* To enable sleep, get rid of this next return */
		return;

		/* 5 * 60 is RL seconds (5 RL minutes sleep need per mud day) */
		/* sleep needed is called (60 * 60 * 24) / (UPDATE_PULSE/4)
           times a mud day. */
        /* 10000 is stuck in for granularity */

		/* If sleep needed is too long, assume that staff has set it
           this way. */

	if ( ch->pc->sleep_needed > 10 * 60 * 100000 )
		return;

	need = 5 * 60 * 100000 * (UPDATE_PULSE / 4) / (60 * 60 * 24);

	ch->pc->sleep_needed += need;

	if ( ch->pc->sleep_needed > 10 * 60 * 100000 )
		ch->pc->sleep_needed = 10 * 60 * 100000;
}

void check_linkdead (void)
{
	CHAR_DATA		*tch = NULL;

	for ( tch = character_list; tch; tch = tch->next ) {
		if ( tch->deleted )
			continue;
		if ( IS_NPC(tch) || tch->desc || !IS_MORTAL(tch) || !tch->room )
			continue;
		if ( tch->pc->time_last_activity + PLAYER_DISCONNECT_SECS < mud_time ) {
			do_quit (tch, "", 3);
		}
	}
}

void check_idling (DESCRIPTOR_DATA *d)
{
		/* Unmark people who aren't really idle */

	if ( d->idle &&
		 d->time_last_activity + PLAYER_IDLE_SECS > mud_time ) {
		d->idle = 0;
		return;
	}

	if ( d->original )
		return;

	if ( !d->character && d->time_last_activity + DESCRIPTOR_DISCONNECT_SECS < mud_time &&
		(d->connected <= CON_ACCOUNT_MENU || d->connected == CON_PENDING_DISC) ) {
		close_socket (d);
		return;
	}

		/* Disconnect those people idle for too long */

	if ( d->idle &&
		 d->time_last_activity + PLAYER_DISCONNECT_SECS < mud_time ) {

		if ( d->character && (IS_NPC(d->character) || !IS_MORTAL(d->character)) )
			return;

		if ( d->original )
			return;

		/*  Idle PCs in the chargen process */

		if ( d->character && !d->character->room ) {
			close_socket (d);
		}

		if ( d->character && d->connected && d->character->room )
			do_quit (d->character, "", 3);

		return;
	}

		/* Warn people who are just getting to be idle */

	if ( !d->idle &&
		 d->time_last_activity + PLAYER_IDLE_SECS < mud_time ) {

		if ( d->character ) {
			if ( d->connected == CON_PLYNG )
				SEND_TO_Q ("Your thoughts begin to drift. #2(Idle)#0\n\r", d);
			else 
				SEND_TO_Q ("\n\rYour attention is required to prevent disconnection from the server. #2(Idle)#0\n\r", d);
		}

		d->idle = 1;
	}
}

void check_idlers ()
{
	DESCRIPTOR_DATA		*d, *d_next;

	for ( d = descriptor_list; d; d = d_next ) {
		d_next = d->next;
		check_idling (d);
	}
}

/* Update both PC's & NPC's and objects*/
void point_update (void)
{
	int				cycle_count = 0;
	int				roll = 0;
	int				i = 0;
	int				damage = 0;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	char			*temp_arg1 = NULL;
	char			*temp_arg2 = NULL;
	CHAR_DATA		*ch = NULL;
	CHAR_DATA		*tch = NULL;
	CHAR_DATA		*next_ch = NULL;
	ROOM_DATA		*room = NULL;
	AFFECTED_TYPE	*af =  NULL;
	WOUND_DATA		*wound = NULL;
	WOUND_DATA		*next_wound = NULL;
	struct time_info_data	healing_time;
	struct time_info_data	bled_time;
	struct time_info_data	playing_time;
	static int reduceIntox = 0;

	cycle_count = 0;

	mud_time_str = timestr(mud_time_str);

	for ( i = 0; i <= 99; i++ )
		zone_table[i].player_in_zone = 0;

	for (ch = character_list; ch; ch = next_ch ) {

		if ( !ch )
			continue;

		next_ch = ch->next;

		if ( ch->deleted )
			continue;

		if ( !ch->room )
			continue;

		if ( !IS_NPC (ch) && ch->room )
			zone_table[ch->room->zone].player_in_zone++;

		room = ch->room;

		*ch->short_descr = tolower(*ch->short_descr);

		if ( IS_SET (ch->act, ACT_VEHICLE) ) {
			if ( room->sector_type == SECT_REEF ) {

				for ( tch = character_list; tch; tch = tch->next ) {

					if ( tch->deleted )
						continue;

					if ( tch->vehicle == ch )
						send_to_char ("The boat shudders and you hear the sides of the "
									  "ship scrape against\n\r"
									  "something.\n\r", tch);
				}

				send_to_char ("Ouch!  You're scraping against the reef!\n\r", ch);

				if ( weaken (ch, 10, 0, "boat in REEF") )	/* 10 hits for scraping the reef */
					continue;
			}
		}

		if ( !ch )
			continue;

		if ( GET_POS (ch) >= SLEEP ) {

			if ( ch->pc ) {

				if ( (af = get_affect (ch, MAGIC_AFFECT_PARALYSIS)) ) {

					if ( GET_POS (ch) == STAND ) {
						send_to_char ("You fall down.\n", ch);
						act ("$n falls down.", FALSE, ch, 0, 0, TO_ROOM);
					}

					else if ( GET_POS (ch) == SIT ) {
						send_to_char ("You collapse.\n", ch);
						act ("$n collapses.", FALSE, ch, 0, 0, TO_ROOM);
					}

					GET_POS (ch) = REST;
				}

				if ( (GET_POS (ch) == REST || GET_POS (ch) == SIT) &&
					 sleep_needed_in_seconds (ch) > 7 * 60 &&
					 IS_MORTAL (ch) && !number (0, 8) &&
					 !get_second_affect (ch, SPA_STAND, NULL) ) {
					act ("$n falls alseep.", FALSE, ch, 0, 0, TO_ROOM);
					GET_POS (ch) = SLEEP;
				}

				if ( sleep_needed_in_seconds (ch) == 300 &&
					 IS_MORTAL (ch) ) {
					send_to_char ("You are feeling drowsy.\n", ch);
						/* Adding this is 1 sec.  If we don't do this, the
                           drowsy message appears every 5 seconds. */
					ch->pc->sleep_needed += 100000;
				}

				else if ( sleep_needed_in_seconds (ch) == 350 &&
						  IS_MORTAL (ch) ) {
					send_to_char ("You are feeling sleepy.\n", ch);
					ch->pc->sleep_needed += 100000;
				}

				else if ( sleep_needed_in_seconds (ch) == 400 &&
						  IS_MORTAL (ch) ) {
					send_to_char ("You are about to fall asleep.\n", ch);
					ch->pc->sleep_needed += 100000;
				}

				else if ( GET_POS (ch) >= REST && GET_POS (ch) != FIGHT &&
					 sleep_needed_in_seconds (ch) > 6 * 60 &&
					 number (0, 25) == 25 &&
					 IS_MORTAL (ch) && ch->desc && ch->desc->original == ch ) {
					if ( number (0, 1) )
						act ("You stifle a yawn.", FALSE, ch, 0, 0, TO_CHAR);
					else {

						if ( get_affect (ch, MAGIC_HIDDEN) &&
							 would_reveal (ch) ) {
							remove_affect_type (ch, MAGIC_HIDDEN);
							act ("$n reveals $mself.", TRUE, ch, 0, 0, TO_ROOM);
						}

						act ("You yawn.", FALSE, ch, 0, 0, TO_CHAR);
						act ("$n yawns.", FALSE, ch, 0, 0, TO_ROOM);
					}
				}

				if ( GET_POS (ch) == SLEEP )
					sleep_credit (ch);
				else if ( IS_MORTAL (ch) )
					sleep_need (ch);
			}

			else if ( GET_POS (ch) == POSITION_SLEEPING && ch->desc &&
					  ch->pc && ch->pc->dreams && !number (0, 5) )
				dream (ch);

			if ( GET_MOVE (ch) < GET_MAX_MOVE (ch) )
				GET_MOVE (ch) = MIN (GET_MOVE (ch) + move_gain (ch), GET_MAX_MOVE (ch));
		}

		if ( !ch )
			continue;

		if ( ch->skills [SKILL_EMPATHIC_HEAL] || get_affect(ch, MAGIC_AFFECT_REGENERATION) ) {
			if (ch->damage) {
				healing_time = real_time_passed (time(0) - ch->lastregen, 0);
				if (healing_time.minute >= (BASE_SPECIAL_HEALING - ch->con/6)) {
					roll = dice(1,100);
					if (GET_POS(ch) == POSITION_SLEEPING) roll -= 20;
					if (GET_POS(ch) == POSITION_RESTING) roll -= 10;
					if (GET_POS(ch) == POSITION_SITTING) roll -= 5;
					if ( roll <= (ch->con * 4) ) {
						if ( roll % 5 == 0 )
							ch->damage -= 2;
						else
							ch->damage -= 1;
					}
					ch->lastregen = time(0);
				}
			}
		}
		else {
			if (ch->damage) {
				healing_time = real_time_passed (time(0) - ch->lastregen, 0);
				if (healing_time.minute >= (BASE_PC_HEALING - ch->con/6)) {
					roll = dice(1,100);
					if (GET_POS(ch) == POSITION_SLEEPING) roll -= 20;
					if (GET_POS(ch) == POSITION_RESTING) roll -= 10;
					if (GET_POS(ch) == POSITION_SITTING) roll -= 5;
					if ( roll <= (ch->con * 4) ) {
						if ( roll % 5 == 0 )
							ch->damage -= 2;
						else
							ch->damage -= 1;
					}
					ch->lastregen = time(0);
				}
			}
		}

		if ( !ch )
			continue;

		if ( GET_POS (ch) == POSITION_STUNNED ) {
			if ( (time(0) - ch->laststuncheck) >= number(15,20) ) {
				ch->laststuncheck = time(0);
				GET_POS (ch) = REST;
				send_to_char ("You shake your head vigorously, recovering from your stun.\n", ch);
				snprintf (buf, MAX_STRING_LENGTH,  "$n shakes $s head vigorously, seeming to recover.");
				act (buf, FALSE, ch, 0, 0, TO_ROOM);
				if ( IS_NPC(ch) )
					do_stand(ch, "", 0);
			}
		}

		if ( !ch )
			continue;

		if ( GET_POS (ch) == POSITION_UNCONSCIOUS ) {
			healing_time = real_time_passed (time(0) - ch->knockedout, 0);
			if (healing_time.minute >= 5) {
				GET_POS (ch) = REST;
				send_to_char ("Groaning groggily, you regain consciousness.\n", ch);
				snprintf (buf, MAX_STRING_LENGTH,  "Groaning groggily, $n regains consciousness.");
				act (buf, FALSE, ch, 0, 0, TO_ROOM);
				if ( IS_NPC(ch) )
					do_stand(ch, "", 0);
			}
		}

		if ( !ch )
			continue;

		if (reduceIntox == 3) {
		    if ( (ch->intoxication > 0) &&
		    	(!--ch->intoxication)){
				send_to_char ("You are sober.\n", ch);
				reduceIntox = 0;
			}
		} 
		else {
			reduceIntox++;
		}
		
		if ( !ch )
			continue;

		if ( !IS_NPC (ch) && ch->pc->app_cost && ch->desc ) {
			playing_time = real_time_passed (time(0) - ch->time.logon + ch->time.played, 0);
			if ( playing_time.hour >= 10 && ch->desc->account ) {
				ch->desc->account->roleplay_points -= ch->pc->app_cost;
				if ( ch->desc->account->roleplay_points < 0 )
					ch->desc->account->roleplay_points = 0;
				save_account (ch->desc->account);
				ch->pc->app_cost = 0;
				save_char (ch, TRUE);
			}
		}

		if ( !IS_NPC (ch) && IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ) {
			playing_time = real_time_passed (time(0) - ch->time.logon + ch->time.played, 0);
			if ( playing_time.hour > 12 ) {
				REMOVE_BIT (ch->plr_flags, NEW_PLAYER_TAG);
				act ("You've been playing for over 12 hours, now; the #2(new player)#0 tag on your long description has been removed. Once again, welcome - have fun, and best of luck in your travels!\n", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			}
		}

		if ( !ch )
			continue;

		for (wound = ch->wounds; wound; wound = next_wound ) {
			if ( !ch->wounds )
				break;
			next_wound = wound->next;
			damage += wound->damage;
			healing_time = real_time_passed (time(0) - wound->lasthealed, 0);
			bled_time = real_time_passed (time(0) - wound->lastbled, 0);
			if (IS_MORTAL(ch) &&
				bled_time.minute >= BLEEDING_INTERVAL &&
				wound->bleeding) {
				
				ch->damage += wound->bleeding;
				wound->lastbled = time(0);
				
				if ( wound->bleeding > 0 && wound->bleeding <= 3 ) {
					temp_arg1 = expand_wound_loc(wound->location);
					temp_arg2 = char_short(ch);
					snprintf (buf, MAX_STRING_LENGTH,  "#1Blood continues to seep from a %s %s on your %s.#0", wound->severity, wound->name, temp_arg1);
					snprintf (buf2, MAX_STRING_LENGTH,  "Blood continues to seep from a %s %s on #5%s#0's %s.", wound->severity, wound->name, temp_arg2, temp_arg1);
				}
				
				else if ( wound->bleeding > 3 && wound->bleeding <= 6 ) {
					temp_arg1 = expand_wound_loc(wound->location);
					temp_arg2 = char_short(ch);
					snprintf (buf, MAX_STRING_LENGTH,  "#1Blood flows from a %s %s on your %s.#0", wound->severity, wound->name, temp_arg1);
					snprintf (buf2, MAX_STRING_LENGTH,  "Blood flows from a %s %s on #5%s#0's %s.", wound->severity, wound->name, temp_arg2, temp_arg1);
					}
				
				else if ( wound->bleeding > 6 && wound->bleeding <= 9 ) {
					temp_arg1 = expand_wound_loc(wound->location);
					temp_arg2 = char_short(ch);
					snprintf (buf, MAX_STRING_LENGTH,  "#1Blood flows heavily from a %s %s on your %s!#0", wound->severity, wound->name, temp_arg1);
					snprintf (buf2, MAX_STRING_LENGTH,  "Blood flows heavily from a %s %s on #5%s#0's %s!", wound->severity, wound->name, temp_arg2, temp_arg1);
					}
				
				else if ( wound->bleeding > 9 ) {
					temp_arg1 = expand_wound_loc(wound->location);
					temp_arg2 = char_short(ch);
					snprintf (buf, MAX_STRING_LENGTH,  "#1Blood gushes from a %s %s on your %s!#0", wound->severity, wound->name, temp_arg1);
					snprintf (buf2, MAX_STRING_LENGTH,  "Blood gushes from a %s %s on #5%s#0's %s!", wound->severity, wound->name, temp_arg2, temp_arg1);
				}
				act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				act (buf2, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
				if ( IS_SET (ch->plr_flags, NEW_PLAYER_TAG) )
					act ("#6To stop the bleeding before it's too late, type BIND.#0", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				if ( general_damage (ch, wound->bleeding) )
					continue;

			}
			if ( ch->skills [SKILL_EMPATHIC_HEAL] || get_affect(ch, MAGIC_AFFECT_REGENERATION) ) {
				if (healing_time.minute >= (BASE_SPECIAL_HEALING + (wound->damage/3) - GET_CON(ch)/7 - wound->healerskill/20)) {
					wound->lasthealed = time(0);
					if ( GET_POS(ch) != POSITION_FIGHTING && !IS_SET (ch->room->room_flags, OOC) )
						natural_healing_check(ch, wound);
				}
			}
			else {
				if (healing_time.minute >= (BASE_PC_HEALING + (wound->damage/3) - GET_CON(ch)/7) - wound->healerskill/20) {
					wound->lasthealed = time(0);
					if ( GET_POS(ch) != POSITION_FIGHTING && !IS_SET (ch->room->room_flags, OOC) )
						natural_healing_check(ch, wound);
				}
			}
		}

		if ( (af = get_affect (ch, MAGIC_CRAFT_DELAY)) ) {
			if ( time(0) >= af->a.spell.modifier ) {
				send_to_char ("#6OOC: Your craft delay timer has expired. You may resume crafting delayed items.#0\n", ch);
				remove_affect_type (ch, MAGIC_CRAFT_DELAY);
			}
		}

		if ( ch->damage < 0 )
			ch->damage = 0;

	} /* for */
}

void hourly_update (void)
{
	int					current_time = 0;
	int					hours = 0;
	int					nomsg = 0;
	CHAR_DATA			*ch = NULL;
	CHAR_DATA			*next_ch = NULL;
	OBJ_DATA			*obj = NULL;
	OBJ_DATA			*objj = NULL;
	OBJ_DATA			*next_thing2 = NULL;
	ROOM_DATA			*room = NULL;
	CHARM_DATA			*ench = NULL;
	NEGOTIATION_DATA	*neg = NULL;
	NEGOTIATION_DATA	*new_list = NULL;
	NEGOTIATION_DATA	*tmp_neg = NULL;
	char				your_buf [MAX_STRING_LENGTH] = {'\0'};
	char				room_buf [MAX_STRING_LENGTH] = {'\0'};
	char				room_msg_buf [MAX_STRING_LENGTH] = {'\0'};

	current_time = time (NULL);

	if ( time_info.hour >= 8 && time_info.hour <= 17 )
		add_second_affect (SPA_CORONAN_ARENA, 2, NULL, NULL, NULL, 0);

	for ( ch = character_list; ch; ch = next_ch ) {

		next_ch = ch->next;

		if ( ch->deleted || !ch->room )
			continue;

		for ( ench = ch->charms; ench; ench = ench->next ) {
			ench->current_hours--;
			if ( ench->current_hours <= 0 )
				remove_charm(ch, ench);
		}

		if ( IS_NPC (ch) && IS_SET (ch->flags, FLAG_KEEPER) && ch->shop ) {

			neg = ch->shop->negotiations;

			new_list = NULL;

			while ( neg ) {

				tmp_neg = neg->next;

				if ( neg->time_when_forgotten <= current_time )
					mem_free (neg);
				else {
					neg->next = new_list;
					new_list = neg;
				}

				neg = tmp_neg;
			}

			ch->shop->negotiations = new_list;

		}

		if ( !IS_NPC (ch) ) {
			if ( !IS_SET (ch->room->room_flags, OOC) )
				hunger_thirst_process (ch);
		}

		trigger (ch, "", TRIG_HOUR);
	}

	for ( obj = object_list; obj; obj = obj->next ) {

		if ( obj->deleted )
			continue;

			/* Mob jailbags need to disappear after a time */

		if ( obj->virtual == VNUM_JAILBAG &&
			 obj->obj_timer &&
			 --obj->obj_timer == 0 )
			extract_obj (obj);

		else if ( IS_SET (obj->obj_flags.extra_flags, ITEM_TIMER) && --obj->obj_timer <= 0 ) {

			room = (obj->in_room == NOWHERE ? NULL : vtor (obj->in_room));

			if ( obj->carried_by )
				act ("$p decays in your hands.",
							FALSE, obj->carried_by, obj, 0, TO_CHAR);

			else if ( room && room->people ) {
				act ("$p gradually decays away.",
							TRUE, room->people, obj, 0, TO_ROOM);
				act ("$p gradually decays away.",
				  			TRUE, room->people, obj, 0, TO_CHAR);
			}

        	for( objj = obj->contains; objj; objj = next_thing2) {

				next_thing2 = objj->next_content; /* Next in inventory */

				if ( obj->virtual == VNUM_CORPSE ) {
					SET_BIT (objj->obj_flags.extra_flags, ITEM_TIMER);
					objj->obj_timer = 12;	/*  Stuff from corpses lasts 
								/*  3 RL hours, unless handled by a PC. */
				}

				obj_from_obj (&objj, 0);

				if (obj->in_obj)
					obj_to_obj(objj,obj->in_obj);
				else if (obj->carried_by)
					obj_to_room(objj,obj->carried_by->in_room);
				else if (obj->in_room != NOWHERE)
					obj_to_room(objj,obj->in_room);
				else
					extract_obj (obj);
			}

			extract_obj (obj);
		}

		else if ( GET_ITEM_TYPE (obj) == ITEM_LIGHT &&
				  obj->o.light.on &&
				  obj->o.light.hours ) {

			obj->o.light.hours--;

			hours = obj->o.light.hours;

			if ( !(ch = obj->carried_by) )
				ch = obj->equiped_by;

			switch ( hours ) {
				case 0:
					strcpy (your_buf, "Your $o burns out.");
					strcpy (room_buf, "$n's $o burns out.");
					strcpy (room_msg_buf, "$p burns out.");
					break;

				case 1:
					strcpy (your_buf, "Your $o is just a dim flicker now.");
					strcpy (room_buf, "$n's $o is just a dim flicker now.");
					strcpy (room_msg_buf, "$p is just a dim flicker now.");
					break;

				case 2:
					strcpy (your_buf, "Your $o begins to burn low.");
					strcpy (room_buf, "$n's $o begins to burn low.");
					strcpy (room_msg_buf, "$p begins to burn low.");
					break;

				case 10:
					strcpy (your_buf, "Your $o sputters.");
					strcpy (room_buf, "$n's $o sputters.");
					strcpy (room_msg_buf, "$p sputters.");
					break;

				default:
					nomsg = 1;
					break;
			}
					
			if ( hours == 0 ||
				 (!is_name_in_list ("candle", obj->name) && !nomsg) ) {

				if ( ch ) {
					act (your_buf, FALSE, ch, obj, 0, TO_CHAR);
					act (room_buf, FALSE, ch, obj, 0, TO_ROOM);
				}

				if ( obj->in_room &&
					 (room = vtor (obj->in_room)) &&
					 room->people ) {
					act (room_msg_buf, FALSE, room->people, obj, 0, TO_ROOM);
					act (room_msg_buf, FALSE, room->people, obj, 0, TO_CHAR);
				}
			}

			if ( obj->o.light.hours > 0 )
				continue;

			if ( is_name_in_list ("candle", obj->name) )
				extract_obj (obj);
		}

		if((obj->morphTime) && obj->morphTime < current_time)
		    morph_obj(obj);
	}
}

int remove_room_affect (ROOM_DATA *room, int type)
{
	AFFECTED_TYPE	*af;
	AFFECTED_TYPE	*free_af;

	if ( !room->affects )
		return 0;

	if ( room->affects->type == type ) {
		free_af = room->affects;
		room->affects = free_af->next;
		mem_free (free_af);
		return 1;
	}

	for ( af = room->affects; af->next; af = af->next )
		if ( af->next->type == type ) {
			free_af = af->next;
			af->next = free_af->next;
			mem_free (free_af);
			return 1;
		}

	return 0;
}

void room_affect_wearoff (ROOM_DATA *room, int type)
{
	if ( !remove_room_affect (room, type) )
		return;

	switch ( type ) {
		case MAGIC_ROOM_CALM:
			if ( room )
				send_to_room ("Slowly, the sense of peace dissipates, and things return to normal.\n", room->virtual);
			else
				send_to_all ("The sense of peace everwhere fades.\n\r");
			break;

		case MAGIC_ROOM_LIGHT:
			if ( room )
				send_to_room ("The unnatural light emanations fade.\n\r", room->virtual);
			else
				send_to_all ("The unnatural light emanations fade from the land.\n\r");
			break;

		case MAGIC_ROOM_DARK:
			if ( room )
				send_to_room ("The unnatural darkness fades away.\n\r", room->virtual);
			else
				send_to_all ("The unnatural darkness fades from the land.\n\r");
			break;

		case MAGIC_WORLD_SOLAR_FLARE:
			if ( room )
				send_to_room ("The localized solar flare has ended.\n\r", room->virtual);
			else
				send_outside ("The ball of flame in the sky slowly dies out.\n\r");
			break;
	}
}

void room_update (void)
{
		/* Expire affects on rooms */
/******* needs lots of work *******************
******** rooms don't ahve affects, so the code doens't work
	AFFECTED_TYPE room_affect;
	
	for ( room = full_room_list; room; room = room->lnext ) {
		for ( room_affect = room->affects;
			  room_affect;
			  room_affect = next_room_affect ) {

			next_room_affect = room_affect->next;

			if ( room_affect->type >= MAGIC_SMELL_FIRST &&
				 room_affect->type <= MAGIC_SMELL_LAST )
				continue;

			if ( room_affect->type == MAGIC_ROOM_FIGHT_NOISE )
				continue;

			if ( room_affect->a.room.duration > 0 )
				room_affect->a.room.duration--;

			if ( !room_affect->a.room.duration )
				room_affect_wearoff (room, room_affect->type);
		}
	}
	*****************************/
	return;
}


/**** skill_use returns a 0 for no effect or failure, 1 for success-no skill gain, 2 for success-with skill gain ***/
 
int skill_use (CHAR_DATA *ch, int skill, int diff_mod)
{
	double		roll = 0;
	int			lv = 0;
	int			cap = 0;
	int			skill_lev = 0;
	int			min = 0;
	int			max = 0;
	OBJ_DATA	*obj =NULL;
	AFFECTED_TYPE	*af = NULL;

	if ( !real_skill (ch, skill) )
		return 0;

	lv  = calc_lookup (ch, REG_LV, skill);
	cap = calc_lookup (ch, REG_CAP, skill);
		
	cap = MIN(cap, 85);

	roll = number (1, 85);

	skill_lev = ch->skills [skill];

	skill_lev -= diff_mod;

	if ( ch->stun > 0 )
		skill_lev -= ch->stun*2;

	if ( (af = get_affect (ch, MAGIC_AFFECT_CURSE)) )
		skill_lev -= af->a.spell.modifier;

    for ( obj = ch->equip; obj; obj = obj->next_content ) {
		if ( GET_ITEM_TYPE (obj) == ITEM_WEAPON || GET_ITEM_TYPE (obj) == ITEM_SHIELD )
			continue;
		for ( af = obj->xaffected; af; af = af->next ) {
			if ( af->a.spell.location - 10000 == skill )
				skill_lev += af->a.spell.modifier;
		}
	}

	if ( (obj = ch->right_hand) ) {
		for ( af = obj->xaffected; af; af = af->next ) {
			if ( af->a.spell.location - 10000 == skill )
				skill_lev += af->a.spell.modifier;
		}
	}

	if ( (obj = ch->left_hand) ) {
		for ( af = obj->xaffected; af; af = af->next ) {
			if ( af->a.spell.location - 10000 == skill )
				skill_lev += af->a.spell.modifier;
		}
	}

	skill_lev = MAX (2, skill_lev);
	skill_lev = MIN (80, skill_lev);

	if ( !AWAKE(ch) )
		return 0;

	if ( roll <= skill_lev )
		return 1;

	if ( IS_NPC (ch) ) {
		if ( ch->skills [skill] < cap && lv >= number (1, 100) )
			ch->skills [skill]++;
	}

	else if ( ch->pc->skills [skill] < cap && lv >= number (1, 100) ) {

		if ( !ch->desc || ch->desc->idle )		/* No skill gain idle/discon */
			return 0;

		if ( IS_SET (ch->room->room_flags, OOC) ) 	/*  No skill gain in OOC  areas. */
			return 0;

		if ( !get_affect (ch, MAGIC_SKILL_GAIN_STOP + skill) && 
		     !get_affect (ch, MAGIC_FLAG_NOGAIN + skill) ) {

			ch->skills [skill]++;
			ch->pc->skills [skill]++;

			min = 40;

			max = 40 + ch->skills [skill] + number(1,60);
			max = MIN (180, max);

			magic_add_affect (ch, MAGIC_SKILL_GAIN_STOP + skill, number(min,max), 0, 0, 0, 0);

			return 2;
		}
	}

	return 0;
}
