/** 
*	\file fight.c
*	Central Combat Processor
*
*	This module deals with combat, wounds, rescue and death. 
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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

CHAR_DATA *combat_list = NULL;
CHAR_DATA *combat_next_dude = NULL; /* Next dude global trick           */


void compete (CHAR_DATA *src, CHAR_DATA *tar, int iterations);

/* Weapon attack texts */

void load_messages(void)
{
	FILE *f1;
	int i,type;
	struct message_type *messages;
	char chk[100];

	if (!(f1 = fopen(MESS_FILE, "r"))){
		perror("read messages");
		abort();	
	}

	for (i = 0; i < MAX_MESSAGES; i++)
	{
		fight_messages[i].a_type = 0;
		fight_messages[i].number_of_attacks=0;
		fight_messages[i].msg = 0;
	}

	fscanf(f1, " %s \n", chk);

	while(*chk == 'M')
	{
		fscanf(f1," %d\n", &type);
		for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type!=type) &&
		    (fight_messages[i].a_type); i++);
		if(i>=MAX_MESSAGES){
			system_log ("Too many combat messages.", TRUE);
			abort();
		}

		CREATE(messages,struct message_type,1);
		fight_messages[i].number_of_attacks++;
		fight_messages[i].a_type=type;
		messages->next=fight_messages[i].msg;
		fight_messages[i].msg=messages;

		messages->die_msg.attacker_msg      = fread_string(f1);
		messages->die_msg.victim_msg        = fread_string(f1);
		messages->die_msg.room_msg          = fread_string(f1);
		messages->miss_msg.attacker_msg     = fread_string(f1);
		messages->miss_msg.victim_msg       = fread_string(f1);
		messages->miss_msg.room_msg         = fread_string(f1);
		messages->hit_msg.attacker_msg      = fread_string(f1);
		messages->hit_msg.victim_msg        = fread_string(f1);
		messages->hit_msg.room_msg          = fread_string(f1);
		messages->god_msg.attacker_msg      = fread_string(f1);
		messages->god_msg.victim_msg        = fread_string(f1);
		messages->god_msg.room_msg          = fread_string(f1);
		fscanf(f1, " %s \n", chk);
	}

	fclose(f1);
}




void add_criminal_time (CHAR_DATA *ch, int zone, int penalty_time)
{
	AFFECTED_TYPE	*af;

	magic_add_affect (ch, MAGIC_CRIM_BASE + zone, penalty_time, 0, 0, 0, 0);

	af = get_affect (ch, MAGIC_CRIM_BASE + zone);

	if ( af->a.spell.duration > 36 )
		af->a.spell.duration = 36;
}

void criminalize (CHAR_DATA *ch, CHAR_DATA *vict, int zone, int crime)
{
	int				criminalize_him = 0;
	int				penalty_time = 0;
	AFFECTED_TYPE	*af;
	ROOM_DATA		*room;
	CHAR_DATA		*tch;
	char			*date = NULL;
	char			msg [MAX_STRING_LENGTH] = {'\0'};

	if ( crime == CRIME_KILL )
		penalty_time = 24;
	else if ( crime == CRIME_STEAL )
		penalty_time = 8;
	else if ( crime == CRIME_PICKLOCK )
		penalty_time = 4;
	else
		penalty_time = 4;

    /* This allows someone to avoid criminalization by assisting a guard and
       then turning on the guard, or other similar things. This is bad. */

	if ( !ch || !vict )
		return;

    	if ( ch->fighting )
		return;

	room = ch->room;

	if ( is_area_enforcer (ch) )
		return;

	if ( !IS_SET (room->room_flags, LAWFUL) )
		return;

	if ( vict ) {
		if ( IS_SET (vict->act, ACT_CRIMINAL) )
			return;

		if ( IS_SET (vict->act, ACT_WILDLIFE) )
			return;

		if ( IS_SET (vict->act, ACT_AGGRESSIVE) )
			return;

		if ( IS_SET (vict->act, ACT_PARIAH) )
			return;

		if ( (get_affect (vict, MAGIC_CRIM_BASE + zone) ||
			  get_affect (vict, MAGIC_CRIM_HOODED + zone)) &&
			 !get_affect (ch, MAGIC_CRIM_BASE + zone) )
			return;
	}

	if ( is_hooded (ch) ) {

		if ( (!OUTSIDE (ch) && !IS_LIGHT (room)) ||
			 (OUTSIDE (ch) && IS_NIGHT) ) {

			if ( !(af = get_affect (ch, MAGIC_CRIM_HOODED + zone)) ) 
				magic_add_affect (ch, MAGIC_CRIM_HOODED + zone, 1, penalty_time, 0, 0, 0);
			else
				af->a.spell.modifier += penalty_time;

			for ( tch = ch->room->people; tch; tch = tch->next_in_room )
				enforcer (tch, ch, 1, 1);

			if ( !ch->deleted && !af ) {
				af = get_affect (ch, MAGIC_CRIM_HOODED + zone);
				affect_remove (ch, af);
			}

			return;
		}
				
		else if ( !number (0, 4) )
			criminalize_him = 1;
		else
			magic_add_affect (ch, MAGIC_CRIM_HOODED + zone, 40, penalty_time, 0, 0, 0);

	} else
		criminalize_him = 1;

	if ( criminalize_him ) {
		send_to_char ("An onlooker gasps at your actions and runs off to find "
					  "help!\n", ch);

			/* Add hooded crim affect with penalty of 0 as a marker */

		magic_add_affect (ch, MAGIC_CRIM_HOODED + zone, 40, 0, 0, 0, 0);

		add_criminal_time (ch, zone, penalty_time);

		/* Interconnected city zones */

		if ( zone == 1 ) {
			magic_add_affect (ch, MAGIC_CRIM_HOODED + 3, 40, 0, 0, 0, 0);
			add_criminal_time (ch, 3, penalty_time);
		}
		else if ( zone == 3 ) {
			magic_add_affect (ch, MAGIC_CRIM_HOODED + 1, 40, 0, 0, 0, 0);
			add_criminal_time (ch, 1, penalty_time);
		}

		if ( IS_MORTAL (ch) ) {
       		 	date = timestr(date);
		
			if ( crime == CRIME_KILL )
				snprintf (msg, MAX_STRING_LENGTH, "Flagged wanted for Assault in %s for %d hours. [%d]\n", zone_table[ch->room->zone].name, penalty_time, ch->in_room);
			else if ( crime == CRIME_STEAL )
				snprintf (msg, MAX_STRING_LENGTH, "Flagged wanted for Attempted Theft in %s for %d hours. [%d]\n", zone_table[ch->room->zone].name, penalty_time, ch->in_room);
			else if ( crime == CRIME_PICKLOCK )
				snprintf (msg, MAX_STRING_LENGTH, "Flagged wanted for Breaking and Entering in %s for %d hours. [%d]\n", zone_table[ch->room->zone].name, penalty_time, ch->in_room);
			else
				snprintf (msg, MAX_STRING_LENGTH, "Flagged wanted in %s for %d hours. [%d]\n", zone_table[ch->room->zone].name, penalty_time, ch->in_room);

			if ( !IS_NPC (ch) ) {	
				add_message (ch->tname, 3, "Server", date, "Wanted.", "", msg, 0);
				add_message ("Crimes", 2, "Server", date, ch->tname, "", msg, 0);
			}
		}
	}

		/* Immediate guard response */

	for ( tch = ch->room->people; tch; tch = tch->next_in_room )
		enforcer (tch, ch, 1, 1);
}


void stop_fighting_sounds (CHAR_DATA *ch, ROOM_DATA *room)
{
	int			dir, from_dir;
	ROOM_DIRECTION_DATA	*exit_room;
	ROOM_DATA		*next_room;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			*e_dirs [] = { "to the north", "to the east", "to the south", "to the west", "above", "below" };
	AFFECTED_TYPE		*af;

	if ( !ch->room )
		return;

	if ( !room )
		return;

	for ( dir = 0; dir <= 5; dir++ ) {
		if ( !(exit_room = PASSAGE (ch, dir)) )
			continue;
		if ( !(next_room = vtor (exit_room->to_room)) )
			continue;
		if ( dir == 0 )
			from_dir = 2;
		else if ( dir == 1 )
			from_dir = 3;
		else if ( dir == 2 )
			from_dir = 0;
		else if ( dir == 3 )
			from_dir = 1;
		else if ( dir == 4 )
			from_dir = 5;
		else from_dir = 4;
		if ( next_room->affects &&
			next_room->affects->type == MAGIC_ROOM_FIGHT_NOISE &&
			next_room->affects->a.room.duration == from_dir ) {
				next_room->affects = next_room->affects->next;
		}
		else for ( af = next_room->affects; af; af = af->next ) {
			if ( !af->next )
				break;
			if ( af->next->type != MAGIC_ROOM_FIGHT_NOISE )
				continue;
			if ( af->next->a.room.duration != from_dir )
				continue;
			af->next = af->next->next;
		}
		snprintf (buf, MAX_STRING_LENGTH,  "The sounds of an armed conflict %s have died away.", e_dirs[from_dir]);
		send_to_room (buf, next_room->virtual);
	}
}

void fighting_sounds (CHAR_DATA *ch)
{
	int			dir, from_dir;
	ROOM_DIRECTION_DATA	*exit_room;
	ROOM_DATA		*next_room;
	AFFECTED_TYPE		*af;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			*e_dirs [] = { "the north", "the east", "the south", "the west", "above", "below" };
	bool			found = FALSE;

	for ( dir = 0; dir <= 5; dir++ ) {
		if ( !(exit_room = PASSAGE (ch, dir)) )
			continue;
		if ( !(next_room = vtor (exit_room->to_room)) )
			continue;
		if ( dir == 0 )
			from_dir = 2;
		else if ( dir == 1 )
			from_dir = 3;
		else if ( dir == 2 )
			from_dir = 0;
		else if ( dir == 3 )
			from_dir = 1;
		else if ( dir == 4 )
			from_dir = 5;
		else from_dir = 4;
		for ( af = next_room->affects; af; af = af->next ) {
			if ( af->type != MAGIC_ROOM_FIGHT_NOISE )
				continue;
			if ( af->a.room.duration != from_dir )
				continue;
			found = TRUE;
		}
		if ( !found ) {
			if ( !next_room->affects ) {
				next_room->affects = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);
				next_room->affects->type = MAGIC_ROOM_FIGHT_NOISE;
				next_room->affects->a.room.duration = from_dir;
				next_room->affects->next = NULL;
			}
			else for ( af = next_room->affects; af; af = af->next ) {
				if ( !af->next ) {
					af->next = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);
					af->next->type = MAGIC_ROOM_FIGHT_NOISE;
					af->next->a.room.duration = from_dir;
					af->next->next = NULL;
					break;
				}
			}
			snprintf (buf, MAX_STRING_LENGTH,  "You hear the sounds of armed battle erupt from %s!", e_dirs[from_dir]);
			send_to_room (buf, next_room->virtual);
		}
	}
}

void set_fighting (CHAR_DATA *ch, CHAR_DATA *vict)
{
	AFFECTED_TYPE	*af;

	clear_pmote(ch);

	if ( IS_SET (ch->flags, FLAG_COMPETE) ||
		 IS_SET (vict->flags, FLAG_COMPETE) )
		return;

	if ( ch->fighting ) {
		return;
	}

	if ( ch == vict )		/* No way, not in this game :( */
		return;

	ch->next_fighting = combat_list;
	combat_list = ch;

	if ( (af = get_affect (ch, MAGIC_AFFECT_SLEEP)) )
		affect_remove (ch, af);

	if ( GET_FLAG (ch, FLAG_AUTOFLEE) && AWAKE(ch) ) {
		send_to_char ("You try to escape!\n\r", ch);
		act ("$n tries to escape!",
				FALSE, ch, 0, 0, TO_ROOM);
        	SET_BIT (ch->flags, FLAG_FLEE);
	}

	if ( (IS_SET (vict->act, ACT_PURSUE) ||
		(!morale_broken(vict) && !IS_SET (vict->act, ACT_SENTINEL) &&
		(IS_SET (vict->act, ACT_AGGRESSIVE) || IS_SET (vict->act, ACT_ENFORCER)))) &&
		!vict->following ) {
		/* vict->following = ch; */
		vict->speed = ch->speed;
	}

	if ( (IS_SET (ch->act, ACT_PURSUE) ||
		(!morale_broken(ch) && !IS_SET (ch->act, ACT_SENTINEL) &&
		(IS_SET (ch->act, ACT_AGGRESSIVE) || IS_SET (ch->act, ACT_ENFORCER)))) &&
		!ch->following ) {
		/* vict->following = ch; */
		ch->speed = vict->speed;
	}

        if ( (af = is_crafting(ch)) ) {
                act ("$n stops doing $s craft.", FALSE, ch, 0, 0, TO_ROOM);
                send_to_char ("You stop doing your craft.\n", ch);
                af->a.craft->timer = 0;
        }

        if ( (af = is_crafting(vict)) ) {
                act ("$n stops doing $s craft.", FALSE, vict, 0, 0, TO_ROOM);
                send_to_char ("You stop doing your craft.\n", vict);
                af->a.craft->timer = 0;
        }

	ch->fighting = vict;

	/* fighting_sounds(ch); */

	add_threat (vict, ch, 2);
	add_threat (ch, vict, 2);

	clear_moves (ch);
	clear_moves (vict);

	if ( IS_NPC(ch) &&
		 !ch->desc &&
		 (is_area_enforcer(ch) || ch->race == RACE_WOLF)
		 && IS_SET (ch->flags, FLAG_KILL) )
		do_alert (ch, "", 0);

	if ( IS_NPC(vict) &&
		 !vict->desc &&
		 (is_area_enforcer(vict) || vict->race == RACE_WOLF)
		 && IS_SET (vict->flags, FLAG_KILL) )
		do_alert (vict, "", 0);
}

void stop_fighting (CHAR_DATA *ch)
{
	CHAR_DATA *tch;
	bool		fighting = FALSE;

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
		if ( tch == ch )
			continue;
		if ( tch->fighting )
			fighting = TRUE;
	}

	if (ch == combat_next_dude)
		combat_next_dude = ch->next_fighting;

	if ( combat_list == ch )
		combat_list = ch->next_fighting;
	else {
		for (tch = combat_list; tch && (tch->next_fighting != ch);
		    tch = tch->next_fighting);
		if (!tch) {
			system_log ("Char fighting not found Error (fight.c, stop_fighting)", TRUE);
			sigsegv(SIGSEGV);
		}
		tch->next_fighting = ch->next_fighting;
	}

	ch->next_fighting = 0;
	ch->fighting = 0;
	if ( GET_POS (ch) == FIGHT )
		GET_POS (ch) = STAND;

	REMOVE_BIT (ch->flags, FLAG_KILL);

	if ( ch->mount && !IS_SET (ch->act, ACT_MOUNT) && ch->mount->fighting )
		stop_fighting (ch->mount);

	if ( IS_NPC(ch) ) {
		ch->speed = 0;
		ch->threats = NULL;
		ch->attackers = NULL;
	}
}

void make_statue (CHAR_DATA *ch)
{
	OBJ_DATA		*statue;
	OBJ_DATA		*o;
	WOUND_DATA		*wound;
	WOUND_DATA		*cwound;
	LODGED_OBJECT_INFO	*lodged;
	LODGED_OBJECT_INFO	*clodged;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	int				i;
	OBJ_DATA		*temp_arg = NULL;
	
	statue = load_object (VNUM_STATUE);

	if ( !IS_NPC (ch) )
		snprintf (buf, MAX_STRING_LENGTH,  "statue pc_%s", GET_NAME (ch));
	else {
		(void)one_argument (GET_NAME (ch), buf2);
		snprintf (buf, MAX_STRING_LENGTH,  "statue npc_%s", buf2);
	}

	statue->name = str_dup (buf);

	snprintf (buf, MAX_STRING_LENGTH,  "An eerily lifelike statue of %s looms here.", ch->short_descr);
	statue->description = str_dup(buf);

	snprintf (buf, MAX_STRING_LENGTH,  "an eerily lifelike statue of %s", ch->short_descr);
	statue->short_description = str_dup(buf);

	for ( wound = ch->wounds; wound; wound = wound->next ) {
		if ( !statue->wounds ) {
			CREATE (statue->wounds, WOUND_DATA, 1);
			statue->wounds->location = add_hash(wound->location);
			statue->wounds->type = add_hash(wound->type);
			statue->wounds->name = add_hash(wound->name);
			statue->wounds->severity = add_hash(wound->severity);
			statue->wounds->bleeding = 0;
			statue->wounds->poison = wound->poison;
			statue->wounds->infection = wound->infection;
			statue->wounds->healerskill = wound->healerskill;
			statue->wounds->lasthealed = wound->lasthealed;
			statue->wounds->lastbled = wound->lastbled;
			statue->wounds->next = NULL;
		}
		else for ( cwound = statue->wounds; cwound; cwound = cwound->next ) {
			if ( cwound->next )
				continue;
			CREATE (cwound->next, WOUND_DATA, 1);
			cwound->next->location = add_hash(wound->location);
			cwound->next->type = add_hash(wound->type);
			cwound->next->name = add_hash(wound->name);
			cwound->next->severity = add_hash(wound->severity);
			cwound->next->bleeding = 0;
			cwound->next->poison = wound->poison;
			cwound->next->infection = wound->infection;
			cwound->next->healerskill = wound->healerskill;
			cwound->next->lasthealed = wound->lasthealed;
			cwound->next->lastbled = wound->lastbled;
			cwound->next->next = NULL;
			break;
		}
	}

	for ( lodged = ch->lodged; lodged; lodged = lodged->next ) {
		if ( !statue->lodged ) {
			CREATE (statue->lodged, LODGED_OBJECT_INFO, 1);
			statue->lodged->vnum = lodged->vnum;
			statue->lodged->location = add_hash(lodged->location);
			statue->lodged->next = NULL;
		}
		else for ( clodged = statue->lodged; clodged; clodged = clodged->next ) {
			if ( !clodged->next ) {
				CREATE (clodged->next, LODGED_OBJECT_INFO, 1);
				clodged->next->vnum = lodged->vnum;
				clodged->next->location = add_hash(lodged->location);
				clodged->next->next = NULL;
				break;
			}
		}
	}

	if ( ch->right_hand ) {
		o = ch->right_hand;
		ch->right_hand = NULL;
		o->equiped_by = NULL;
		o->carried_by = NULL;
		o->next_content = NULL;
		obj_to_obj (o, statue);
	}

	if ( ch->left_hand ) {
		o = ch->left_hand;
		ch->left_hand = NULL;
		o->equiped_by = NULL;
		o->carried_by = NULL;
		o->next_content = NULL;
		obj_to_obj (o, statue);
	}

	statue->obj_flags.weight = get_weight (ch) * 5;

	for ( i = 0; i < MAX_WEAR; i++ )
		if ( get_equip (ch, i) ){
			temp_arg = unequip_char(ch, i);
			obj_to_obj (temp_arg, statue);
		}
	IS_CARRYING_N (ch) = 0;

	obj_to_room (statue, ch->in_room);
}

#define MAX_NPC_CORPSE_TIME 12 /* 3 RL hours -- corpses are saved. */
#define MAX_PC_CORPSE_TIME 96 /* 1 RL day -- corpses are saved. */

void make_corpse (CHAR_DATA *ch)
{
	OBJ_DATA		*corpse;
	OBJ_DATA		*o;
	WOUND_DATA		*wound;
	WOUND_DATA		*cwound;
	LODGED_OBJECT_INFO	*lodged;
	LODGED_OBJECT_INFO	*clodged;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	int				i;
	OBJ_DATA		*temp_arg = NULL;
	
	corpse = load_object (VNUM_CORPSE);

	if ( !IS_NPC (ch) )
		snprintf (buf, MAX_STRING_LENGTH,  "corpse pc_%s", GET_NAME (ch));
	else {
		(void)one_argument (GET_NAME (ch), buf2);
		snprintf (buf, MAX_STRING_LENGTH,  "corpse npc_%s", buf2);
	}

	corpse->name = str_dup (buf);

	snprintf (buf, MAX_STRING_LENGTH,  "The corpse of %s is lying here.", ch->short_descr);
	corpse->description = str_dup(buf);

	snprintf (buf, MAX_STRING_LENGTH,  "the corpse of %s", ch->short_descr);
	corpse->short_description = str_dup(buf);

	for ( wound = ch->wounds; wound; wound = wound->next ) {
		if ( !corpse->wounds ) {
			CREATE (corpse->wounds, WOUND_DATA, 1);
			corpse->wounds->location = add_hash(wound->location);
			corpse->wounds->type = add_hash(wound->type);
			corpse->wounds->name = add_hash(wound->name);
			corpse->wounds->severity = add_hash(wound->severity);
			corpse->wounds->bleeding = 0;
			corpse->wounds->poison = wound->poison;
			corpse->wounds->infection = wound->infection;
			corpse->wounds->healerskill = wound->healerskill;
			corpse->wounds->lasthealed = wound->lasthealed;
			corpse->wounds->lastbled = wound->lastbled;
			corpse->wounds->next = NULL;
		}
		else for ( cwound = corpse->wounds; cwound; cwound = cwound->next ) {
			if ( cwound->next )
				continue;
			CREATE (cwound->next, WOUND_DATA, 1);
			cwound->next->location = add_hash(wound->location);
			cwound->next->type = add_hash(wound->type);
			cwound->next->name = add_hash(wound->name);
			cwound->next->severity = add_hash(wound->severity);
			cwound->next->bleeding = 0;
			cwound->next->poison = wound->poison;
			cwound->next->infection = wound->infection;
			cwound->next->healerskill = wound->healerskill;
			cwound->next->lasthealed = wound->lasthealed;
			cwound->next->lastbled = wound->lastbled;
			cwound->next->next = NULL;
			break;
		}
	}

	for ( lodged = ch->lodged; lodged; lodged = lodged->next ) {
		if ( !corpse->lodged ) {
			CREATE (corpse->lodged, LODGED_OBJECT_INFO, 1);
			corpse->lodged->vnum = lodged->vnum;
			corpse->lodged->location = add_hash(lodged->location);
			corpse->lodged->next = NULL;
		}
		else for ( clodged = corpse->lodged; clodged; clodged = clodged->next ) {
			if ( !clodged->next ) {
				CREATE (clodged->next, LODGED_OBJECT_INFO, 1);
				clodged->next->vnum = lodged->vnum;
				clodged->next->location = add_hash(lodged->location);
				clodged->next->next = NULL;
				break;
			}
		}
	}

	if ( ch->right_hand ) {
		o = ch->right_hand;
		ch->right_hand = NULL;
		o->equiped_by = NULL;
		o->carried_by = NULL;
		o->next_content = NULL;
		obj_to_obj (o, corpse);
	}

	if ( ch->left_hand ) {
		o = ch->left_hand;
		ch->left_hand = NULL;
		o->equiped_by = NULL;
		o->carried_by = NULL;
		o->next_content = NULL;
		obj_to_obj (o, corpse);
	}

	if ( ch->mob ) {
		if ( GET_FLAG (ch, FLAG_WILLSKIN) ) {
			corpse->o.od.value [2] = -ch->mob->skinned_vnum;
			corpse->o.od.value [3] = -ch->mob->carcass_vnum;
		} else {
			corpse->o.od.value [2] = ch->mob->skinned_vnum;
			corpse->o.od.value [3] = ch->mob->carcass_vnum;
		}
	} else {
		corpse->o.od.value [2] = 0;
		corpse->o.od.value [3] = 0;
	}

	corpse->obj_flags.weight = get_weight (ch);

	if ( IS_NPC (ch) )
		corpse->obj_timer = MAX_NPC_CORPSE_TIME;
	else
		corpse->obj_timer = MAX_PC_CORPSE_TIME;

	SET_BIT (corpse->obj_flags.extra_flags, ITEM_TIMER);

	for ( i = 0; i < MAX_WEAR; i++ ) {
		if ( get_equip (ch, i) ) {
			if ( GET_ITEM_TYPE (get_equip(ch, i)) == ITEM_CONTAINER && IS_SET (ch->plr_flags, NEW_PLAYER_TAG) )
				continue;
			temp_arg = unequip_char(ch, i);
			obj_to_obj (temp_arg, corpse);
		}
	}

	IS_CARRYING_N (ch) = 0;

	obj_to_room (corpse, ch->in_room);
}

void remove_guest_skills (CHAR_DATA *ch)
{
	int		i;

	if ( !IS_SET (ch->flags, FLAG_GUEST) )
		return;

	for ( i = 0; i <= MAX_SKILLS; i++ ) {
                ch->skills[i] = 0;
                ch->pc->skills[i] = 0;
        }

        ch->speaks = db_race_table[ch->race].race_speaks;
        ch->skills [ch->speaks] = calc_lookup(ch, REG_CAP, db_race_table[ch->race].race_speaks);
        ch->pc->skills [ch->speaks] = ch->skills [ch->speaks];

}

void death_email (CHAR_DATA *ch)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES	*result;
	MYSQL_ROW	row;
	ACCOUNT_DATA	*account;
	FILE		*fp;
	bool		found = FALSE;

	if ( !ch || !ch->pc || !ch->pc->account )
		return;

	if ( !(account = load_account (ch->pc->account)) )
		return;

	if ( !(fp = fopen(ch->tname, "w+")) )
		return;

	fprintf (fp, 	"From: %s <%s>\n"
		     	"To: %s\n"
			"X-Sender: %s"
			"Mime-Version: 1.0\n"
			"Content-type: text/plain;charset=\"us-ascii\"\n"
			"Organization: %s\n"
			"Subject: %s's Recent Demise\n"
			"\n", MUD_NAME, MUD_EMAIL, account->email, MUD_EMAIL, MUD_NAME, ch->tname);

	result = NULL;

	fprintf (fp, 	"Hello,\n\n"
			"   Our records indicate that your PC, %s, has recently passed on. For\n"
			"your convenience and future reference, we have taken the liberty of\n"
			"compiling all of %s journal entries, in-game board posts and in-\n"
			"character writings; they are attached below.\n\n"
			"   Thanks for playing, and we hope to see you back again soon.\n"
			"\n"
			"\n"
			"                                           Best Regards,\n"
			"                                           The Admin Team\n\n", ch->tname, HSHR(ch));
 
	mysql_safe_query ("SELECT * FROM player_journals WHERE name = '%s' ORDER BY post_number ASC", ch->tname);
	result = mysql_store_result(database);
	if ( mysql_num_rows (result) ) {
		fprintf (fp, "Journal Entries:\n\n");
		while ( (row = mysql_fetch_row(result)) ) {
			found = TRUE;
			fprintf (fp, "--\nDate: %s\nSubject: %s\n\n%s\n", row[4], row[2], row[5]);
		}
		fprintf (fp, "\n");
		mysql_free_result (result);
		result = NULL;
	}

	mysql_safe_query ("SELECT * FROM boards WHERE author = '%s' ORDER BY board_name,post_number ASC", ch->tname);
	result = mysql_store_result(database);
	if ( mysql_num_rows (result) ) {
		fprintf (fp, "In-Game Board Posts:\n\n");	
		while ( (row = mysql_fetch_row(result)) ) {
			found = TRUE;
			fprintf (fp, "--\nDate: %s [%s]\nSubject: %s\n\n%s\n", row[5], row[4],row[2], row[6]);
		}
		fprintf (fp, "\n");
		mysql_free_result (result);
		result = NULL;
	}

	mysql_safe_query ("SELECT * FROM player_writing WHERE author = '%s' ORDER BY db_key,page ASC", ch->tname);
	result = mysql_store_result(database);
	if ( mysql_num_rows(result) ) {
		fprintf (fp, "In-Character Writings:\n\n");
		while ( (row = mysql_fetch_row(result)) ) {
			found = TRUE;
			fprintf (fp, "--\nDate: %s\n\n%s\n", row[3], row[8]);
		}
		fprintf (fp, "\n");
		mysql_free_result (result);
		result = NULL;
	}

	if ( !found )
		fprintf (fp, "--\nNo writing was found in our database.\n");

	fclose (fp);

	snprintf (buf, MAX_STRING_LENGTH,  "/usr/sbin/sendmail %s < %s", account->email, ch->tname);
	system (buf);

	unlink (ch->tname);

	free_account (account);
	account = NULL;
}

void raw_kill (CHAR_DATA *ch)
{
	CHAR_DATA	*tch;
	DESCRIPTOR_DATA	*d;

	if ( ch->fighting ) {
		if (ch == combat_next_dude)
			combat_next_dude = ch->next_fighting;

		if ( combat_list == ch )
			combat_list = ch->next_fighting;
		else {
                        for (tch = combat_list; tch && (tch->next_fighting != ch);
                                tch = tch->next_fighting);
                        if (!tch) {
                                system_log ("Char fighting not found Error (fight.c, stop_fighting)", TRUE);
				sigsegv (SIGSEGV);
                        }
                        tch->next_fighting = ch->next_fighting;
		}

		ch->next_fighting = 0;
		ch->fighting = 0;
	}

	for ( tch = character_list; tch; tch = tch->next ) {
		if ( tch->deleted )
			continue;
		if ( tch->aiming_at == ch && ch->room != tch->room )
			act ("$n collapses, slain. You lower your weapon.", FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
	}

	if ( !IS_SET (ch->plr_flags, FLAG_PETRIFIED) )
		make_corpse (ch);
	else make_statue (ch);

	if ( IS_SET (ch->plr_flags, FLAG_PETRIFIED) ) {
		act ("$n suddenly seems to grow stiff, turning to stone before your very eyes!", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		act ("As the hated rays of the sun strike your skin, you suddenly grow stiff, and your mind is awash in helpless rage before -- nothing...", TRUE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	}
	else if ( !ch->mount ) {
		send_to_room ("\n", ch->in_room);
		if ( GET_POS (ch) >= 7 ) 
			act ("$n collapses to the ground, dead.", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		else
			act ("$n expires with a ragged exhalation.", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		ch->mount = NULL;
	}
	else if ( ch->mount && IS_SET (ch->act, ACT_MOUNT) ) {
		send_to_room ("\n", ch->in_room);
		act ("$n collapses, dead, dumping $N to the ground in the process.", TRUE, ch, 0, ch->mount, TO_NOTVICT | TO_ACT_FORMAT);
		act ("$n collapses, dead, dumping you to the ground in the process.", TRUE, ch, 0, ch->mount, TO_VICT | TO_ACT_FORMAT);
		ch->mount->mount = NULL;
		ch->mount = NULL;	
	}
	else if ( ch->mount && !IS_SET (ch->act, ACT_MOUNT) ) {
		send_to_room ("\n", ch->in_room);
		act ("$n falls to the ground from atop $N, dead.", TRUE, ch, 0, ch->mount, TO_NOTVICT | TO_ACT_FORMAT);
		act ("$n falls to the ground from atop you, dead.", TRUE, ch, 0, ch->mount, TO_VICT | TO_ACT_FORMAT);
		ch->mount->mount = NULL;
		ch->mount = NULL;
	}

	if ( (!IS_NPC(ch) && !IS_SET (ch->flags, FLAG_GUEST)) || (IS_SET (ch->flags, FLAG_GUEST) && ch->desc) ) {
		GET_POS(ch) = POSITION_STANDING;
		while ( ch->wounds )
			wound_from_char (ch, ch->wounds);
		while ( ch->lodged )
			lodge_from_char (ch, ch->lodged);
		ch->damage = 0;
		if ( IS_SET (ch->flags, FLAG_GUEST) ) {
			nanny_create_guest (ch->desc, "recreate");
		}
		else {
			d = ch->desc;
			extract_char (ch);
                        SEND_TO_Q ("#0Your character has, regrettably, passed away. Our condolences. . .#0\n\n"
                                   "#0Thank you for playing - we hope to see you back again soon!#0\n", d);
			d->connected = CON_ACCOUNT_MENU;
			nanny (d, "");
			if ( IS_MORTAL (ch) && !ch->pc->mortal_mode ) {
				death_email (ch);
				mysql_safe_query ("UPDATE newsletter_stats SET pc_deaths=pc_deaths+1");
			}
		}
	}
	else {
		extract_char(ch);
	}
}


void die (CHAR_DATA *ch)
{
	int			duration = 0;
	STORED_PC_DATA  	*pc;
	char			*date = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			msg [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	

	
	if ( ch->combat_log ) {
		system_log (ch->combat_log, FALSE);
	}

	if ( !IS_NPC (ch) ) {

		if ( !IS_SET (ch->flags, FLAG_GUEST) ) {
			date = timestr(date);

			if ( IS_SET (ch->plr_flags, FLAG_PETRIFIED) )
				snprintf (msg, MAX_STRING_LENGTH, "Location: %s [%d]\n\nDied of petrification.\n", ch->room->name, ch->in_room);
			else if ( ch->delay_info1 && ch->delay_ch && !IS_SET (ch->flags, FLAG_BINDING) ) {
				snprintf (buf2, MAX_STRING_LENGTH,  " [%s]", ch->delay_ch->tname);
				snprintf (msg, MAX_STRING_LENGTH, "Location: %s [%d]\n\nAssailant: #5%s#0%s\n\nWeapon: #2%s#0\n", ch->room->name, ch->in_room, ch->delay_ch->short_descr, !IS_NPC(ch->delay_ch) ? buf2 : "", vtoo(ch->delay_info1)->short_description);
			}
			else if ( ch->delay_ch && !ch->delay_info1 && !IS_SET (ch->flags, FLAG_BINDING) ) {
				snprintf (buf2, MAX_STRING_LENGTH,  " [%s]", ch->delay_ch->tname);
				snprintf (msg, MAX_STRING_LENGTH, "Location: %s [%d]\n\nAssailant: #5%s#0%s\n", ch->room->name, ch->in_room, ch->delay_ch->short_descr, !IS_NPC(ch->delay_ch) ? buf2 : "");
			}
			else 
				snprintf (msg, MAX_STRING_LENGTH, "Location: %s [%d]\n\nDied of unspecified causes, e.g. bloodloss or falling.\n", ch->room->name, ch->in_room);

			add_message (ch->tname, 3, "Server", date, "Died.", "", msg, 0);
			add_message ("Deaths", 2, "Server", date, ch->tname, "", msg, 0);

			if ( ch->desc && ch->desc->account ) {
				for ( pc = ch->desc->account->pc; pc; pc = pc->next ) {
					if ( !str_cmp (pc->name, ch->tname) ) {
						pc->state = STATE_DIED;
						break;
					}
				}
			}

			mem_free (date);
		}

		if ( !IS_SET (ch->flags, FLAG_GUEST) ) {
			if ( !IS_SET (ch->plr_flags, FLAG_PETRIFIED) )
				snprintf (buf, MAX_STRING_LENGTH,  "%s has been slain!\n", ch->tname); 
			else snprintf (buf, MAX_STRING_LENGTH,  "%s has been petrified!\n", ch->tname);
			send_to_gods (buf);
		}
	
		ch->delay_ch = 0;
		ch->delay_info1 = 0;

			/* NOTE:  af doesn't point to anything after affects are removed */

		if ( duration > 100 )	/* Fix a bug where some players have a LARGE */
			duration = 1;		/* number of hours */

		while ( ch->hour_affects )
			affect_remove (ch, ch->hour_affects);

		ch->pc->create_state = STATE_DIED;
		ch->pc->last_died = time (0);

		save_char (ch, TRUE);
	}

	raw_kill (ch);

	if ( IS_SET (ch->plr_flags, FLAG_PETRIFIED) )
		REMOVE_BIT (ch->plr_flags, FLAG_PETRIFIED);
}

char *replace_string(char *str, char *weapon, char *local)
{
	static char		buf [MAX_STRING_LENGTH] = {'\0'};		/* Leave as static */
	char			*cp;

	cp = buf;

	for (; *str; str++) {
		if (*str == '#') {
			switch(*(++str)) {
			case 'W' :
				for (; *weapon; *(cp++) = *(weapon++));
				break;
			case 'L' :
				for (;*local;*(cp++)=*(local++));
				break;
			default :
				*(cp++) = '#';
				break;
			}
		} else {
			*(cp++) = *str;
		}

		*cp = 0;
	} /* For */

	return(buf);
}

int damage (CHAR_DATA *ch, CHAR_DATA *victim, int dam, int attacktype)
{
	int			i,j,nr;
	struct message_type *messages;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*temp_arg = NULL;
	
	if ( GET_POS(victim) <= POSITION_DEAD ) {
		snprintf (buf, MAX_STRING_LENGTH,  "kill -SEGV %d", getpid());
		system (buf);
	}

	if ((GET_TRUST(victim)>3) && !IS_NPC(victim))
		dam=0;

	if (victim != ch) {
		if ( GET_POS (ch) > POSITION_STUNNED &&
			 !ch->fighting )
			set_fighting (ch, victim);

		if (GET_POS(victim) > POSITION_STUNNED) {
			if (!(victim->fighting))
				set_fighting(victim, ch);
			GET_POS(victim) = POSITION_FIGHTING;
		}

	}

	if ( dam > 0 )
		GET_HIT (victim) -= dam;

			/* God knows how the victim was hit */

	else {

		for(i = 0; i < MAX_MESSAGES; i++) {
			if (fight_messages[i].a_type == attacktype) {
				nr = dice (1, fight_messages[i].number_of_attacks);
				for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
					messages=messages->next;

				if ( !IS_NPC (victim) && !IS_MORTAL (victim) ) {
					temp_arg = get_equip (ch, WEAR_PRIM);
					act (messages->god_msg.attacker_msg, FALSE, ch,
						 temp_arg, victim, TO_CHAR);
					act (messages->god_msg.victim_msg, FALSE, ch,
						 temp_arg, victim, TO_VICT);
					act (messages->god_msg.room_msg, FALSE, ch,
						 temp_arg, victim, TO_NOTVICT);
				} else if (dam != 0) {
					if (GET_POS(victim) == POSITION_DEAD) {
						temp_arg = get_equip (ch, WEAR_PRIM);
						act (messages->die_msg.attacker_msg, FALSE, ch,
								temp_arg, victim, TO_CHAR);
						act (messages->die_msg.victim_msg, FALSE, ch,
								temp_arg, victim, TO_VICT);
						act (messages->die_msg.room_msg, FALSE, ch,
								temp_arg, victim, TO_NOTVICT);
					} else {
						temp_arg = get_equip (ch, WEAR_PRIM);
						act (messages->hit_msg.attacker_msg, FALSE, ch,
								temp_arg, victim, TO_CHAR);
						act (messages->hit_msg.victim_msg, FALSE, ch,
								temp_arg, victim, TO_VICT);
						act (messages->hit_msg.room_msg, FALSE, ch,
								temp_arg, victim, TO_NOTVICT);
					}
				} else { /* Dam == 0 */
					temp_arg = get_equip (ch, WEAR_PRIM);
					act (messages->miss_msg.attacker_msg, FALSE, ch,
								temp_arg, victim, TO_CHAR);
					act (messages->miss_msg.victim_msg, FALSE, ch,
								temp_arg, victim, TO_VICT);
					act (messages->miss_msg.room_msg, FALSE, ch,
								temp_arg, victim, TO_NOTVICT);
				}
			}
		}
	}

	if (GET_POS(victim) < POSITION_DEAD)
		if (ch->fighting == victim)
			stop_fighting(ch);

	if (!AWAKE(victim))
		if (victim->fighting)
			stop_fighting(victim);

	if (GET_POS(victim) == POSITION_DEAD) {
		if (IS_NPC(victim) || victim->desc)
			if (!IS_NPC(victim)) {
				snprintf (buf, MAX_STRING_LENGTH,  "%s killed by %s at %s", GET_NAME(victim), (IS_NPC(ch) ? ch->short_descr : GET_NAME(ch)), vtor (victim->in_room)->name);
				if (IS_NPC(ch) && IS_SET(ch->act,ACT_MEMORY)) 
					forget (ch, victim);
			}
		die(victim);
		return 1;
	}

	return 0;
}

#define		SINGLE		1
#define		PRIMARY		2
#define		SECONDARY	3


void hit_char (CHAR_DATA *ch, CHAR_DATA *victim, int smite_parm)
{
	int			killed = 0;
	OBJ_DATA	*weapon;
	AFFECTED_TYPE	*temp_aff = NULL;

	if ( !ch || !victim )
		return;

	if ( IS_SET (victim->act, ACT_FLYING) && !IS_SET (ch->act, ACT_FLYING) && AWAKE(victim) ) {
		send_to_char ("They are flying out of reach!\n", ch);
		return;
	}

	if ( IS_NPC(ch) && !ch->desc ) {
		ready_melee_weapons (ch);
	}

	if ( IS_NPC (victim) && !victim->desc ) {
		ready_melee_weapons (victim);
	}

	if ( victim->delay && victim->delay_type != DEL_LOAD_WEAPON )
		break_delay (victim);

	if ( ch->delay && ch->delay_type != DEL_LOAD_WEAPON )
		break_delay (ch);

	if ( GET_POS (ch) != POSITION_DEAD && GET_POS (victim) != POSITION_DEAD )
		criminalize (ch, victim, victim->room->zone, CRIME_KILL);

	if ( !ch->fighting && !smite_parm )
		set_fighting (ch, victim);

	if ( IS_SUBDUEE (victim) ) {
		act ("$n takes a swing at $N.", FALSE, ch, 0, victim, TO_NOTVICT);
		act ("$N takes a swing at you.", FALSE, victim, 0, ch, TO_CHAR);
	}

	if ( IS_SUBDUEE (ch) ) {
		if ( ch->fighting )
			stop_fighting (ch);
		return;
	}

	if ( GET_FLAG (ch, FLAG_FLEE) ) {

		if ( !ch->primary_delay && !flee_attempt (ch) )
			ch->primary_delay = 16;

		return;
	}

	guard_check (victim);

	if ( IS_SUBDUER (ch) )
		return;

	if ( get_affect (victim, AFFECT_GUARD_DIR) ) {
		act ("The attack prevents you from continuing to guard the exit.", FALSE, victim, 0, 0, TO_CHAR);
		act ("The attack prevents $n from continuing to guard the exit.", FALSE, victim, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		temp_aff = get_affect (victim, AFFECT_GUARD_DIR);
		affect_remove (victim, temp_aff);
	}

		/* Empty handed attack / Primary / Dual */

	if ( !ch->primary_delay &&
		 (get_equip (ch, WEAR_PRIM) ||
		  get_equip (ch, WEAR_BOTH) ||
		  (!get_equip (ch, WEAR_PRIM) &&
		   !get_equip (ch, WEAR_BOTH) &&
		   !get_equip (ch, WEAR_SEC))) ) {

		if ( GET_FLAG (ch, FLAG_SUBDUING) ) {

			if ( !(weapon = get_equip (ch, WEAR_PRIM)) )
				return;

			if ( weapon->o.weapon.use_skill != SKILL_LIGHT_EDGE )
				return;
		}

		if ( get_affect (ch, MAGIC_AFFECT_DIZZINESS) && !number (0, 3) ) {
			send_to_char ("You battle your dizziness as you ready your attack.", ch);
			act ("$N staggers, appearing dizzy.", TRUE, ch, 0, 0, TO_ROOM);
			ch->primary_delay += 5 * 4;
		} else
			smite (ch, victim, 1);
	}

	if ( ch->deleted || victim->deleted || !victim->room || !ch->room )
		killed = 1;

	if ( !killed && !can_move (victim) && !GET_FLAG (ch, FLAG_KILL) &&
         ch->fighting ) {

		act ("$N can't move.", FALSE, ch, 0, victim, TO_CHAR);

			/* Aggressives don't stop trying to kill */

		if ( !ch->mob || !IS_SET (ch->act, ACT_AGGRESSIVE) )
			stop_fighting (ch);

		return;
	}

		/* Secondary weapon attack */

	if ( !killed && !ch->secondary_delay && get_equip (ch, WEAR_SEC) ) {

		if ( GET_FLAG (ch, FLAG_SUBDUING) &&
			 !((weapon = get_equip (ch, WEAR_SEC)) &&
				weapon->o.weapon.use_skill != SKILL_LIGHT_EDGE ) )
			return;

		smite (ch, victim, 2);
	}

	if ( !ch->deleted && !victim->deleted && ch->fighting &&
		 ch->room && victim->room && IS_SUBDUEE (victim) )
		stop_fighting (ch);

	if ( !ch->deleted && !victim->deleted && ch->fighting && !killed &&
		 !can_move (victim) && !GET_FLAG (ch, FLAG_KILL) ) {

		act ("$N can't move.", FALSE, ch, 0, victim, TO_CHAR);

			/* Aggressives don't stop trying to kill */

		if ( !ch->mob || !IS_SET (ch->act, ACT_AGGRESSIVE) )
			stop_fighting (ch);
	}

	if ( !ch->deleted && !ch->fighting && GET_POS (ch) == FIGHT )
		GET_POS (ch) = STAND;
}	


char *get_dam_word (int damage)
{
	if ( damage <= 6 )
		return " ";

	if ( damage <= 10 )
		return " hard ";

	if ( damage <= 15 )
		return " very hard ";

	if ( damage <= 20 )
		return " extremely hard ";

	return " incredibly hard ";
}

int combat_roll (int ability)
{
	int		r;
	int		roll_result;

	r = number (1, 100);

	if ( ability > 98 )
		ability = 98;

	if ( ability < 5 )
		ability = 5;

	if ( r > ability )
		roll_result = SUC_MF - ((r % 5) ? 0 : 1);
	else
		roll_result = SUC_MS + ((r % 5) ? 0 : 1);

	return roll_result;
}

#define AD (fd + strlen (fd))

void advance (CHAR_DATA *src, CHAR_DATA *tar)
{
	if ( src->distance_to_target >= 2 ) {
		act("You begin advancing on $N, preparing for battle.", FALSE, src, 0, tar, TO_CHAR);
		act("$n begins advancing on $N, preparing for battle.", FALSE, src, 0, tar, TO_NOTVICT);
		act("$n begins advancing on you, preparing for battle.", FALSE, src, 0, tar, TO_VICT);
	}
	else if ( src->distance_to_target == 1 ) {
		act("You close to polearm range on $N!", FALSE, src, 0, tar, TO_CHAR);
		act("$n closes to polearm range on $N!", FALSE, src, 0, tar, TO_NOTVICT);
		act("$n closes to polearm range on you!", FALSE, src, 0, tar, TO_VICT);
	}
	else if ( src->distance_to_target == 0 ) {
		act("You close to melee range on $N!", FALSE, src, 0, tar, TO_CHAR);
		act("$n closes to melee range on $N!", FALSE, src, 0, tar, TO_NOTVICT);
		act("$n closes to melee range on you!", FALSE, src, 0, tar, TO_VICT);
	}
	src->primary_delay = 20;
	src->distance_to_target -= 1;
	tar->distance_to_target -= 1;

}


int smite (CHAR_DATA *src, CHAR_DATA *tar, int attack_num)
{
	float			defense = 0;
	float			attack = 0;
	int			off_success = 0;
	int			def_success = 0;
	int			off_result = 0;
	int			def_result = 0;
	int			defense_hand = 0;
	int			location = 0;
	int			damage = 0;
	int			hit_type = 0;
	int			i = 0;
	int			j = 0;
	int			wear_loc1 = 0;
	int			wear_loc2 = 0;
	char			loc[MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA		*tar_prim = get_equip (tar, WEAR_PRIM);
	OBJ_DATA		*tar_sec  = get_equip (tar, WEAR_SEC);
	OBJ_DATA		*tar_both = get_equip (tar, WEAR_BOTH);
	OBJ_DATA		*src_prim = get_equip (src, WEAR_PRIM);
	OBJ_DATA		*src_sec  = get_equip (src, WEAR_SEC);
	OBJ_DATA		*src_dual = get_equip (src, WEAR_BOTH);
	OBJ_DATA		*attack_weapon = NULL;
	OBJ_DATA		*defense_weapon = NULL;
	OBJ_DATA		*shield = NULL;
	OBJ_DATA		*attack_shield = NULL;
	OBJ_DATA		*eq1 = NULL;
	OBJ_DATA		*eq2 = NULL;
	OBJ_DATA		*broken_eq = NULL;
	CHAR_DATA		*mount;
	AFFECTED_TYPE	*af = NULL;
	int				bonus = 0;
	float			attack_modifier = 0;
	float			defense_modifier = 0;
	float			r1 = 0;
	char			fd [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA		*dch= NULL;
	char			*temp_arg = NULL;

	*fd = 0;

	if ( IS_SET (src->act, ACT_VEHICLE) )
		return 0;

	snprintf (AD, MAX_STRING_LENGTH, "%s [%d hp %d mvs] strike %d %s [%d hp %d mvs]  ",
				GET_NAME (src), GET_HIT (src), GET_MOVE (src), attack_num,
				GET_NAME (tar), GET_HIT (tar), GET_MOVE (tar));

	attack_modifier = 100.0;
	defense_modifier = 100.0;

	if ( src->in_room != tar->in_room )
		return 0;

	if ( attack_num == 1 )
		attack_weapon = src_prim ? src_prim : src_dual;
	else
		attack_weapon = src_sec;

	if ( attack_weapon &&
	     (attack_weapon->o.weapon.use_skill == SKILL_SHORTBOW ||
             attack_weapon->o.weapon.use_skill == SKILL_LONGBOW ||
	     attack_weapon->o.weapon.use_skill == SKILL_CROSSBOW ||
	     attack_weapon->o.weapon.use_skill == SKILL_SLING ||
	     attack_weapon->o.weapon.use_skill == SKILL_THROWN) )
		return 0;

	if ( attack_weapon && GET_ITEM_TYPE (attack_weapon) != ITEM_WEAPON &&
		GET_ITEM_TYPE (attack_weapon) != ITEM_SHIELD )
		attack_weapon = NULL;

	if ( IS_SET (src->flags, FLAG_PACIFIST) )
		return 0;

	if ( attack_weapon )
		snprintf (AD, MAX_STRING_LENGTH, "%s\n\r", attack_weapon->short_description);
	else {
		attack = src->skills [SKILL_BRAWLING];		/* default attack */
		snprintf (AD, MAX_STRING_LENGTH, "BRAWLING\n\r");
	}

	if ( attack_weapon )
		attack = src->skills [attack_weapon->o.weapon.use_skill];

	if ( attack < src->offense ) {
		attack = src->offense;
		snprintf (AD, MAX_STRING_LENGTH, "Using Offense %d ", (int) attack);
	}

	snprintf (AD, MAX_STRING_LENGTH, "ABase %d ", (int) attack);

			/* Weapon bonus/penalty */

	bonus = 100;

	if ( attack_weapon ) {
		for ( af = attack_weapon->xaffected; af; af = af->next )
			switch ( af->a.spell.location ) {
				case APPLY_OFFENSE:
				case APPLY_CLUB:
				case APPLY_SPEAR:
				case APPLY_SWORD:
				case APPLY_DAGGER:
				case APPLY_AXE:
				case APPLY_WHIP:
				case APPLY_POLEARM:
					snprintf (AD, MAX_STRING_LENGTH, "+%d WEAP-AFF ", af->a.spell.modifier);
					bonus += af->a.spell.modifier;
					break;
				default: break;
			}
	}

	if ( (attack_shield = get_equip (src, WEAR_SHIELD)) ) {

		for ( af = attack_shield->xaffected; af; af = af->next ) {
			if ( af->a.spell.location == APPLY_OFFENSE ) {
				snprintf (AD, MAX_STRING_LENGTH, "%d OFF/SHIELD PEN ", af->a.spell.modifier);
				bonus += af->a.spell.modifier;
			}
		}
	}

	if ( bonus < 0 )
		attack_modifier = 0.0;
	else
		attack_modifier = attack_modifier * bonus / 100.0;

	snprintf (AD, MAX_STRING_LENGTH, "weapmod %d ", bonus);

			/* Fatigue penalty */

	if ( GET_MAX_MOVE (src) > 0 )
		j = GET_MOVE (src) * 100 / GET_MAX_MOVE (src);
	else
		j = 0;

	if ( j > 100 )
		j = 100;

	for ( i = 0; j > fatigue [i].percent; i++ )
		;

	snprintf (AD, MAX_STRING_LENGTH, "fatigue %3.2f ", fatigue [i].penalty);

	attack_modifier = attack_modifier * fatigue [i].penalty;

			/* Dual wield penalty */

	if ( attack_num == 2 ) {
		r1 = .60 + .40 * src->skills [SKILL_DUAL] / 100.0;
		snprintf (AD, MAX_STRING_LENGTH, "Dual Pen %3.2f ", r1);
		attack_modifier = attack_modifier * r1;
	}

			/* Encumberance penalty */

	for ( i = 0; i < CUMBERANCE_ENTRIES; i++ ) {
		if ( GET_STR (src) * enc_tab [i].str_mult_wt >= IS_CARRYING_W (src) )
			break;
	}

	attack_modifier = attack_modifier * enc_tab [i].penalty;

	snprintf (AD, MAX_STRING_LENGTH, "Enc %3.2f ", enc_tab [i].penalty);

			/* Fightmode modifier */

	attack_modifier = attack_modifier *
					  fight_tab [src->fight_mode].offense_modifier;

	if ( get_affect (src, MAGIC_AFFECT_FURY) ) {
		attack_modifier = attack_modifier * 1.25;
		snprintf (AD, MAX_STRING_LENGTH, "* 1.25 [FURY] ");
	}

	if ( get_affect (src, MAGIC_AFFECT_DIZZINESS) ) {
		attack_modifier = attack_modifier * 0.75;
		snprintf (AD, MAX_STRING_LENGTH, "* 0.75 [DIZZINESS] ");
	}

	snprintf (AD, MAX_STRING_LENGTH, "FM %3.2f ", fight_tab [src->fight_mode].offense_modifier);
	snprintf (AD, MAX_STRING_LENGTH, " = OFFENSE %3.0f\n", attack * attack_modifier / 100);

			/* DEFENSE */

		/* We need to know which weapon to defend with */

	shield = get_equip (tar, WEAR_SHIELD);

	if ( shield && shield->obj_flags.type_flag != ITEM_SHIELD ) {
		printf ("Non-shield object %d, on %d(%s), at %d\n",
				shield->virtual, IS_NPC (tar) ? tar->mob->virtual : 0,
				tar->name, tar->room->virtual);
		fflush (stdout);
	}

	defense_weapon = NULL;

	if ( tar_both ) {
		defense_weapon = tar_both;
		defense_hand = 1;
	}

	else if ( tar_prim && !tar_sec && !shield ) {
		defense_weapon = tar_prim;
		defense_hand = 1;
	}

	else if ( tar_sec && !tar_prim && !shield ) {
		defense_weapon = tar_sec;
		defense_hand = 2;
	}

	else if ( shield && !tar_prim && !tar_sec ) {
		defense_weapon = shield;
		defense_hand = 1;
	}

	else if ( shield && tar_prim ) {
		if ( tar->primary_delay > tar->secondary_delay ) {
			defense_weapon = shield;
			defense_hand = 2;
		} else {
			defense_weapon = tar_prim;
			defense_hand = 1;
		}
	}

	else if ( shield && tar_sec ) {
		if ( tar->primary_delay > tar->secondary_delay ) {
			defense_weapon = tar_sec;
			defense_hand = 2;
		} else {
			defense_weapon = shield;
			defense_hand = 1;
		}
	}

	else if ( tar_prim && tar_sec ) {
		if ( tar->primary_delay > tar->secondary_delay ) {
			defense_weapon = tar_sec;
			defense_hand = 2;
		} else {
			defense_weapon = tar_prim;
			defense_hand = 1;
		}
	}

	else {
		defense_weapon = NULL;
		defense_hand = 1;
	}

	if ( shield != defense_weapon )
		shield = NULL;

	if ( defense_weapon && GET_ITEM_TYPE (defense_weapon) != ITEM_WEAPON 
		&& GET_ITEM_TYPE (defense_weapon) != ITEM_SHIELD )
		defense_weapon = NULL;

	if ( !defense_weapon )
		defense = tar->skills [SKILL_DODGE];
	else if ( defense_weapon->obj_flags.type_flag == ITEM_SHIELD )
		defense = tar->skills [SKILL_BLOCK];
	else if ( defense_weapon->obj_flags.type_flag == ITEM_WEAPON ) {
		if ( defense_weapon->o.weapon.use_skill == SKILL_LONGBOW ||
		     defense_weapon->o.weapon.use_skill == SKILL_SHORTBOW ||
	             defense_weapon->o.weapon.use_skill == SKILL_CROSSBOW ||
		     defense_weapon->o.weapon.use_skill == SKILL_SLING ||
	             defense_weapon->o.weapon.use_skill == SKILL_THROWN ) {
			defense = tar->skills [SKILL_DODGE];
			defense_weapon = NULL;
		}
		else defense = tar->skills [SKILL_PARRY];
	}
	else {
		defense = 0;
	}

	if ( real_skill (tar, SKILL_DANGER_SENSE) )
		if ( skill_use (tar, SKILL_DANGER_SENSE, 0) )
			defense += tar->skills [SKILL_DANGER_SENSE] / 5;
			/*
			* On a successful use of the Danger Sense skill, if
			* the skill being checked is a defensive combat skill,
			* it grants them a bonus; they are able to sense the
			* impending blow before it lands. (Nexus)
			*/

	if ( IS_SET (tar->flags, FLAG_PACIFIST) )
		defense += 10;

	if ( defense > 95 ) defense = 95;

	/* Unless you're Legolas, using bows in melee isn't so bright... */
	if ( tar->aiming_at )
		defense -= 20;

	snprintf (AD, MAX_STRING_LENGTH, "DBase %d ", (int) defense);

			/* Fatigue penalty */

	if ( GET_MAX_MOVE (tar) > 0 )
		j = GET_MOVE (tar) * 100 / GET_MAX_MOVE (tar);
	else {
		j = 0;
	}

	if ( j > 100 )
		j = 100;

	for ( i = 0; j > fatigue [i].percent; i++ )
		;

	snprintf (AD, MAX_STRING_LENGTH, "Fatigue %3.2f ", fatigue [i].penalty);

	defense_modifier = defense_modifier * fatigue [i].penalty;

			/* Encumberance penalty */

	for ( i = 0; i < CUMBERANCE_ENTRIES; i++ ) {
		if ( GET_STR (tar) * enc_tab [i].str_mult_wt >= IS_CARRYING_W (tar) )
			break;
	}

	snprintf (AD, MAX_STRING_LENGTH, "Enc %3.2f ", enc_tab [i].penalty);

	defense_modifier = defense_modifier * enc_tab [i].penalty;


			/* Fightmode modifier */

	defense_modifier = defense_modifier *
					   fight_tab [tar->fight_mode].defense_modifier;

	snprintf (AD, MAX_STRING_LENGTH, "FM %3.2f ", fight_tab [tar->fight_mode].defense_modifier);

			/* Hand delay modifier */

	if ( defense_hand == 1 )
		r1 = (100 - 5 * tar->primary_delay) / 100.0;
	else
		r1 = (100 - 5 * tar->secondary_delay) / 100.0;

	if ( r1 < .25 )		/* Maximum 75% penalty for being delayed. */
		r1 = .25;

	defense_modifier = defense_modifier * r1;

	snprintf (AD, MAX_STRING_LENGTH, "DelayPen %3.2f ", r1);

			/* Weapon/shield defense */

	if ( defense_weapon ) {

		bonus = 100;

		for ( af = defense_weapon->xaffected; af; af = af->next ) {

			if ( defense_weapon->obj_flags.type_flag == ITEM_SHIELD ) {
				if ( af->a.spell.location == APPLY_BLOCK )
					bonus += af->a.spell.modifier;
			}

			else if ( af->a.spell.location == APPLY_PARRY )
				bonus += af->a.spell.modifier;
		}

		if ( bonus < 0 )
			defense_modifier = 0.0;
		else
			defense_modifier = defense_modifier * bonus / 100.0;

		snprintf (AD, MAX_STRING_LENGTH, "Weapbon %d ", bonus);
	}

	defense = defense * defense_modifier / 100;
	attack = attack * attack_modifier / 100;

	if ( attack_weapon && (attack_weapon->o.od.value[3] == SKILL_MEDIUM_EDGE || attack_weapon->o.od.value[3] == SKILL_MEDIUM_BLUNT ||
		attack_weapon->o.od.value[3] == SKILL_MEDIUM_PIERCE) ) {
		if ( attack_weapon->location == WEAR_SEC )
			attack -= 10;
	}

	if ( attack_weapon && (attack_weapon->o.od.value[3] == SKILL_HEAVY_EDGE || attack_weapon->o.od.value[3] == SKILL_HEAVY_BLUNT ||
		attack_weapon->o.od.value[3] == SKILL_HEAVY_PIERCE) ) {
		if ( attack_weapon->location == WEAR_PRIM )
			attack -= 10;
		if ( attack_weapon->location == WEAR_SEC )
			attack -= 20;
	}

	if ( defense_weapon && (defense_weapon->o.od.value[3] == SKILL_HEAVY_EDGE || defense_weapon->o.od.value[3] == SKILL_HEAVY_BLUNT ||
		defense_weapon->o.od.value[3] == SKILL_HEAVY_PIERCE) ) { 
		if ( defense_weapon->location == WEAR_PRIM )
			defense -= 10;
		if ( defense_weapon->location == WEAR_SEC )
			defense -= 20;
	}

       

	snprintf (AD, MAX_STRING_LENGTH, " = DEFENSE %d\n", (int) defense);

	attack = figure_wound_skillcheck_penalties (src, attack);
	defense = figure_wound_skillcheck_penalties (tar, defense);

	defense = MAX (5, defense);
	attack = MAX (5, attack);

	snprintf (AD, MAX_STRING_LENGTH,  "End Result: %f Attack\n", attack);

	off_success = combat_roll (attack);

	def_success = combat_roll (defense);

        if ( attack_weapon )
                hit_type = attack_weapon->o.weapon.hit_type;
        else {
                if ( src->nat_attack_type == 0 )
                        hit_type = 9;
                else if ( src->nat_attack_type == 1 || src->nat_attack_type == 3 )
                        hit_type = 7;
                else if ( src->nat_attack_type == 2 )
                        hit_type = 8;
        }

		/* Must be standing or fighting AND not in frantic mode */

	if ( (GET_POS (tar) != STAND && GET_POS (tar) != FIGHT) ||
		 tar->fight_mode == 0 ) {
		def_result = RESULT_NONE;
		off_result = ignore_offense [off_success];
		snprintf (AD, MAX_STRING_LENGTH, "IGNORE:  %s = %s\n",
					 cs_name [off_success], rs_name [off_result]);
	}

	else if ( defense_hand == 1 && !defense_weapon ) {
		off_result = dodge_offense [off_success] [def_success];
		def_result = dodge_defense [off_success] [def_success];
		snprintf (AD, MAX_STRING_LENGTH, "DODGE:  %s(%d) = %s(%d);    %s(%d) = %s(%d)\n",
			cs_name [off_success], off_success, rs_name [off_result], off_result,
			cs_name [def_success], def_success, rs_name [def_result], def_result);
	}

	else if ( defense_weapon ) {
		off_result = shield_parry_offense [off_success] [def_success];
		def_result = shield_parry_defense [off_success] [def_success];

		if ( off_result == RESULT_BLOCK && defense_weapon != shield ) {
			off_result = RESULT_PARRY;
			def_result = RESULT_PARRY;
		}

		snprintf (AD, MAX_STRING_LENGTH, "BLOCK/PARRY:  %s = %s;    %s = %s\n",
			cs_name [off_success], rs_name [off_result],
			cs_name [def_success], rs_name [def_result]);
	}

	figure_damage (src,
				   tar,
				   attack_weapon,
				   off_result,
				   &damage,
				   &location);

	temp_arg = figure_location(tar, location);
	snprintf (loc, MAX_STRING_LENGTH, "%s",temp_arg );

        wear_loc1 = body_tab [0] [location].wear_loc1; 
        wear_loc2 = body_tab [0] [location].wear_loc2;
        eq1 = get_equip (tar, wear_loc1);                
        eq2 = get_equip (tar, wear_loc2);

	if ( eq2 && IS_SET (eq2->obj_flags.wear_flags, ITEM_WEAR_ABOUT) && (name_is ("cloak", eq2->name) || name_is("cape", eq2->name)) && number(0,2) )
		eq2 = NULL;

	if ( off_result == RESULT_FUMBLE ) {

		if ( GET_DEX (src) <= number (1, 21) ) {
			if ( attack_weapon && number (0, 1) )
				off_result = RESULT_NEAR_FUMBLE;
			else
				off_result = RESULT_NEAR_STUMBLE;
		}

		else if ( !attack_weapon || number (0, 1) )
			off_result = RESULT_STUMBLE;

		if ( off_result != RESULT_FUMBLE )
			snprintf (AD, MAX_STRING_LENGTH, "offensive result -> %s\n", rs_name [off_result]);
	}

	if ( def_result == RESULT_FUMBLE ) {

		if ( GET_DEX (tar) <= number (1, 21) ) {
			if ( defense_weapon && number (0, 1) )
				def_result = RESULT_NEAR_FUMBLE;
			else
				def_result = RESULT_NEAR_STUMBLE;
		}

		else if ( !defense_weapon || number (0, 1) )
			def_result = RESULT_STUMBLE;

		if ( def_result != RESULT_FUMBLE )
			snprintf (AD, MAX_STRING_LENGTH, "defensive result -> %s\n", rs_name [def_result]);
	}

		/* DA can occur only if defending a primary attacker */

	if ( def_result == RESULT_ADV && tar->fighting != src )
		def_result = RESULT_NONE;

	if ( attack_weapon && (off_result == RESULT_BLOCK || off_result == RESULT_PARRY) ) {

		if ( attack_weapon && ((defense_weapon && attack_weapon->quality <= defense_weapon->quality) || (shield && attack_weapon->quality <= shield->quality)) && number (1,100) > attack_weapon->quality ) {
			if ( shield )
				damage_to_obj (attack_weapon, number(1,MAX(damage,5)), 3);
			else
				damage_to_obj (attack_weapon, number(1,MAX(damage,5)), defense_weapon->o.weapon.hit_type);
			if ( (number(1,100) > attack_weapon->quality && number(1,100) > attack_weapon->item_wear) || attack_weapon->item_wear <= 0 )
				off_result = RESULT_WEAPON_BREAK;
		}
		else if ( attack_weapon && defense_weapon && number (1,100) > defense_weapon->quality ) {
			damage_to_obj (defense_weapon, number(1,MAX(damage,5)), hit_type);
			if ( (number(1,100) > defense_weapon->quality && number(1,100) > defense_weapon->item_wear) || defense_weapon->item_wear <= 0 )
				def_result = RESULT_WEAPON_BREAK;
		}
		else if ( attack_weapon && shield && number(1,100) > shield->quality ) {
			damage_to_obj (shield, number(1,MAX(damage,5)), hit_type);
			if ( (number(1,100) > shield->quality && number(1,100) > shield->item_wear) || shield->item_wear <= 0 )
				def_result = RESULT_SHIELD_BREAK;
		}
		else if ( attack_weapon && ((defense_weapon && attack_weapon->quality > defense_weapon->quality) || (shield && attack_weapon->quality > shield->quality)) && attack_weapon->quality > defense_weapon->quality && number(1,100) > attack_weapon->quality ) {
			if ( shield )
				damage_to_obj (attack_weapon, number(1,MAX(damage,5)), 3);
			else
				damage_to_obj (attack_weapon, number(1,MAX(damage,5)), defense_weapon->o.weapon.hit_type);
			if ( (number(1,100) > attack_weapon->quality && number(1,100) > attack_weapon->item_wear) || attack_weapon->item_wear <= 0 )
				off_result = RESULT_WEAPON_BREAK;
		}
	}

	if ( (off_result == RESULT_HIT1 || off_result == RESULT_HIT2 || off_result == RESULT_HIT3 || off_result == RESULT_HIT4) &&
		attack_weapon && (eq1 || eq2) ) {
		if ( attack_weapon && eq1 && eq1->o.od.value[0] > 2 && attack_weapon->quality <= eq1->quality && number (1,100) > attack_weapon->quality ) {
			damage_to_obj (attack_weapon, number(1,MAX(damage,5)), 3);
			if ( (number(1,100) > attack_weapon->quality && number(1,100) > attack_weapon->item_wear) || attack_weapon->item_wear <= 0 ) {
				off_result = RESULT_WEAPON_BREAK;
				def_result = RESULT_ANY;
			}
		}
		else if ( attack_weapon && eq2 && eq2->o.od.value[0] > 2 && attack_weapon->quality <= eq2->quality && number (1,100) > attack_weapon->quality ) {
			damage_to_obj (attack_weapon, number(1,MAX(damage,5)), 3);
			if ( (number(1,100) > attack_weapon->quality && number(1,100) > attack_weapon->item_wear) || attack_weapon->item_wear <= 0 ) {
				off_result = RESULT_WEAPON_BREAK;
				def_result = RESULT_ANY;
			}
		}
		else if ( attack_weapon && eq1 && number (1,100) > eq1->quality ) {
			if ( GET_ITEM_TYPE (eq1) == ITEM_ARMOR )
				damage_to_obj (eq1, number(1,MAX(damage,5)), hit_type);
			else
				damage_to_obj (eq1, number(1,MAX(damage,5)), hit_type);
			if ( (number(1,100) > eq1->quality && number(1,100) > eq1->item_wear) || eq1->item_wear <= 0 )
				broken_eq = eq1;
		}
		else if ( attack_weapon && eq2 && number (1,100) > eq2->quality ) {
			if ( GET_ITEM_TYPE (eq2) == ITEM_ARMOR )
				damage_to_obj (eq2, number(1,MAX(damage,5)), hit_type);
			else
				damage_to_obj (eq2, number(1,MAX(damage,5)), hit_type);
			if ( number(1,100) > eq2->quality && number(1,100) > eq2->item_wear )
				broken_eq = eq2;
		}
		else if ( attack_weapon && eq1 && eq1->o.od.value[0] > 2 && attack_weapon->quality > eq1->quality && number (1,100) > attack_weapon->quality ) {
			damage_to_obj (attack_weapon, number(1,MAX(damage,5)), 3);
			if ( (number(1,100) > attack_weapon->quality && number(1,100) > attack_weapon->item_wear) || attack_weapon->item_wear <= 0 ) {
				off_result = RESULT_WEAPON_BREAK;
				def_result = RESULT_ANY;
			}
		}
		else if ( attack_weapon && eq2 && eq2->o.od.value[0] > 2 && attack_weapon->quality > eq2->quality && number (1,100) > attack_weapon->quality ) {
			damage_to_obj (attack_weapon, number(1,MAX(damage,5)), 3);
			if ( (number(1,100) > attack_weapon->quality && number(1,100) > attack_weapon->item_wear) || attack_weapon->item_wear <= 0 ) {
				off_result = RESULT_WEAPON_BREAK;
				def_result = RESULT_ANY;
			}
		}
	}

	if ( IS_SET (tar->flags, FLAG_COMPETE) ) {
		if ( attack_weapon )
			return wound_to_char (tar, loc, damage, attack_weapon->o.weapon.hit_type, 0, 0, 0);
		else
			return wound_to_char (tar, loc, damage, src->nat_attack_type, 0, 0, 0);
	}
        combat_results (src,
                        tar,
                        attack_weapon,
                        defense_weapon,
						broken_eq,
                        damage,
                        loc,
                        off_result,
                        def_result,
                        attack_num,
                        fd,
                        off_success,
                        def_success);

        snprintf (AD, MAX_STRING_LENGTH, "---------------------------------------\n");

        for ( dch = src->room->people; dch; dch = dch->next_in_room )
                if ( IS_SET (dch->debug_mode, DEBUG_FIGHT) )
                        send_to_char (fd, dch);

	if ( tar->deleted )
		return 1;
	else {
		if ( IS_RIDER (tar) ) {

			mount = tar->mount;
/*
			if ( mount && (mount->skills [SKILL_RIDE] < 33 ||
				 (mount->skills [SKILL_RIDE] < 66 &&
				  !skill_use (mount, SKILL_RIDE, 0))) ) {
				dump_rider (tar, FALSE);
				flee_attempt (mount);
			}
*/
		}

		return 0;
	}
}

void combat_msg_substitute (char *msg, char *template, CHAR_DATA *src,
							CHAR_DATA *tar, OBJ_DATA *attack_weapon,
						    OBJ_DATA *defense_weapon, char *location,
							int damage)
{
	int		aw_type = 6;
	char	loc[MAX_STRING_LENGTH] = {'\0'};
	char	*temp_arg = NULL;
	
	if ( attack_weapon )
		aw_type = attack_weapon->o.weapon.hit_type;

	*msg = '\0';

	while ( *template ) {

		if ( *template != '#' ) {
			*msg = *template;
			template++;
			msg++;
			*msg = '\0';
		}

		else {
			switch ( template [1] ) {
			case 'S':
				if ( aw_type == 6 )
					strcat (msg, attack_names_plural [src->nat_attack_type]);
				else
					strcat (msg, wtype2 [aw_type * 2 + number (0, 1)]);
				break;
			case 's':
				if ( aw_type == 6 )
					strcat (msg, attack_names [src->nat_attack_type]);
				else
					strcat (msg, wtype [aw_type * 2 + number (0, 1)]);
				break;
			case 'O':
				if ( !attack_weapon )
					strcat (msg, attack_part [src->nat_attack_type]);
				else{
					temp_arg = see_object_in(tar, attack_weapon);
					strcat (msg, temp_arg);
				}
				break;
			case 'o':
				if ( !defense_weapon )
					strcat (msg, "fist");
				else{
					temp_arg = see_object_in(src,defense_weapon); 
					strcat (msg, temp_arg);
				}	
				break;
			case 'L': 
				temp_arg = expand_wound_loc(location);
				snprintf (loc, MAX_STRING_LENGTH, "%s", temp_arg);
				strcat (msg, loc);
				break;
			case 'C':
				strcat (msg, crithits [aw_type * 2 + number (0, 1)]);
				break;
			case 'c': 
				strcat (msg, crithit [aw_type * 2 + number (0, 1)]);
				break;
			case 'B': 
				strcat (msg, break_def [aw_type * 2 + 1]);
				break;
			case 'b': 
				strcat (msg, break_def [aw_type * 2]);
				break;
			case 'D': 
				temp_arg = get_dam_word (damage);
				strcat (msg, temp_arg);
				break;
			case 'R': 
				strcat (msg, "\n\r");
				break;
			default:
				*msg = *template;		/* Not a variable */
				msg [1] = '\0';
				template--;				/* Work around for advance 2 chars */
				break;
         	}

			msg = &msg [strlen (msg)];

			template = &template [2];		/* Advance two characters */
		}
	}
}

void combat_results (CHAR_DATA *src, CHAR_DATA *tar, OBJ_DATA *attack_weapon,
					 OBJ_DATA *defense_weapon, OBJ_DATA *broken_eq, int damage, char *location,
					 int off_result, int def_result, int attack_num,
					 char *fd, int off_success, int def_success)
{
	int					attack_delay = 0;
	int					i = 0;
	int					j = 0;
	int					skill = 0;
	int					delay_modifier = 0;
	int					hit_type = 0;
	int					table = 0;
	int					current_sum = 0;
	int					best_sum = 0;
	AFFECTED_TYPE		*invulnerability = NULL;
	COMBAT_MSG_DATA		*best_cm = NULL;
	COMBAT_MSG_DATA		*tcm = NULL;
	CHAR_DATA			*tch = NULL;
	OBJ_DATA			*tobj = NULL;
	OBJ_DATA			*temp_obj = NULL;
	char				msg1 [MAX_STRING_LENGTH] = {'\0'};
	char				msg2 [MAX_STRING_LENGTH] = {'\0'};
	char				msg3 [MAX_STRING_LENGTH] = {'\0'};
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				log_message [MAX_STRING_LENGTH] = {'\0'};
	char				*temp_arg = NULL;

	*log_message = '\0';

	if ( tar ) {
		snprintf (log_message, MAX_STRING_LENGTH, "by %s", src->short_descr);
		if ( IS_NPC (src) )
			snprintf (log_message + strlen (log_message), MAX_STRING_LENGTH, " (%d)",
						src->mob->virtual);
		else
			snprintf (log_message + strlen (log_message), MAX_STRING_LENGTH, " (PC %s)",
						src->tname);
	}

	if ( attack_weapon )
		snprintf (log_message + strlen (log_message), MAX_STRING_LENGTH, " w/ %s (%d)",
				attack_weapon->short_description, attack_weapon->virtual);

	if ( (off_result < RESULT_HIT || off_result > RESULT_HIT4) &&
		 !number (0, 1) ) {

		skill = SKILL_BRAWLING;

		if ( attack_weapon )
			skill = attack_weapon->o.weapon.use_skill;

		if ( attack_num == 1 )
			skill_use (src, skill, 0);

		else if ( number (0, 1) ) {
			if ( number (0, 1) )
				skill_use (src, SKILL_DUAL, 0);
			else
				skill_use (src, skill, 0);
		}

		fix_offense (src);
	}

	if ( off_result >= RESULT_HIT && off_result <= RESULT_HIT4 &&
		 !number (0, 1) ) {

		if ( GET_POS (tar) == STAND || GET_POS (tar) == FIGHT ) {

			if ( !defense_weapon )
				skill = SKILL_DODGE;
			else if ( defense_weapon->obj_flags.type_flag == ITEM_WEAPON )
				skill = SKILL_PARRY;
			else if ( defense_weapon->obj_flags.type_flag == ITEM_SHIELD )
				skill = SKILL_BLOCK;
			else {
				skill = SKILL_PARRY;
			}

			skill_use (tar, skill, 0);

			fix_offense (tar);
		}
	}


	if ( tar->fight_mode == 0 &&		/* frantic mode */
		 (GET_POS (tar) == STAND || GET_POS (tar) == FIGHT) )
		table = 'F';

	else if ( GET_POS (tar) != STAND && GET_POS (tar) != FIGHT &&
		 tar->fight_mode != 0 )
		table = 'I';		/* Ignore */

	else if ( !defense_weapon )
		table = 'D';		/* Dodge */

	else if ( defense_weapon->obj_flags.type_flag == ITEM_SHIELD )
		table = 'B';		/* Block */

	else
		table = 'P';		/* Parry */

	for ( tcm = cm_list; tcm; tcm = tcm->next ) {

		if ( off_result != tcm->off_result )
			continue;

		if ( tcm->table != '*' && tcm->table != table )
			continue;

		if ( tcm->def_result != RESULT_ANY && tcm->def_result != def_result )
			continue;

		if ( !best_cm ) {
			best_cm = tcm;
			continue;
		}

		/* Hierarchy                */
		/*		x any * msg			*/
		/*		x any t msg			*/
		/*		x y   * msg			*/
		/*		x y   t msg			*/

		current_sum = 0;
		if ( best_cm->def_result != RESULT_ANY )
			current_sum += 2;

		if ( best_cm->table != '*' )
			current_sum += 1;

		if ( current_sum == 3 )
			break;

		best_sum = 0;

		if ( tcm->def_result != RESULT_ANY )
			best_sum += 2;

		if ( tcm->table != '*' )
			best_sum += 1;

		if ( current_sum < best_sum )
			best_cm = tcm;
	}

	combat_msg_substitute (msg1, best_cm->def_msg, src, tar, attack_weapon,
						   defense_weapon, location, damage);
	combat_msg_substitute (msg2, best_cm->off_msg, src, tar, attack_weapon,
						   defense_weapon, location, damage);
	combat_msg_substitute (msg3, best_cm->other_msg, src, tar, attack_weapon,
						   defense_weapon, location, damage);

		/* Oh wait, vehicles get a generic message */

	if ( IS_SET (tar->act, ACT_VEHICLE) ) {
		if ( attack_weapon ) {
			act ("You attack $N with your $o.",
					FALSE, src, attack_weapon, tar, TO_CHAR);
			act ("$n attacks you with his $o.",
					FALSE, src, attack_weapon, tar, TO_VICT);
			act ("$n attacks $N with $s $o.",
					FALSE, src, attack_weapon, tar, TO_NOTVICT);
		} else {
			act ("You kick at $N, trying to destroy it.",
					FALSE, src, 0, tar, TO_CHAR);
			act ("$n kicks at you, trying to break you.",
					FALSE, src, 0, tar, TO_VICT);
			act ("$n kicks at $N, trying to break it.",
					FALSE, src, 0, tar, TO_NOTVICT);
		}
	} else {
		act (msg1, FALSE, tar, attack_weapon, src, TO_CHAR | TO_ACT_FORMAT | TO_ACT_COMBAT);
		act (msg2, FALSE, tar, attack_weapon, src, TO_VICT | TO_ACT_FORMAT | TO_ACT_COMBAT);
		act (msg3, FALSE, tar, attack_weapon, src, TO_NOTVICT | TO_ACT_FORMAT | TO_ACT_COMBAT);
	}

		/* Make sure invulnerability is set last in the IF clause below. */

	if ( damage &&
		 attack_weapon &&
		 !IS_SET (attack_weapon->obj_flags.extra_flags, ITEM_MAGIC) &&
		 (invulnerability = get_affect (tar, MAGIC_AFFECT_INVULNERABILITY)) ) {
		act ("$n appears undamaged!", FALSE, tar, 0, 0, TO_ROOM);
		act ("$n's $o didn't wound you.",
					FALSE, src, attack_weapon, tar, TO_VICT);
	}

	if ( damage ) {

		if ( attack_weapon )
			hit_type = attack_weapon->o.weapon.hit_type;
		else
			hit_type = 6;		/* natural attack */

		if ( off_result != RESULT_HIT  && off_result != RESULT_HIT1 &&
			 off_result != RESULT_HIT2 && off_result != RESULT_HIT3 &&
			 off_result != RESULT_HIT4 )
			system_log ("Damage result without a HIT.", TRUE);
	}

	else if ( off_result == RESULT_STUMBLE ) {
		GET_POS (src) = SIT;
		add_second_affect (SPA_STAND, 8, src, NULL, NULL, 0);

		if ( IS_SET (tar->flags, FLAG_FLEE) )
			flee_attempt (tar);
	}

	else if ( off_result == RESULT_FUMBLE ) {
		if ( (tobj = get_equip (src, WEAR_PRIM)) == attack_weapon && !IS_SET(src->flags, FLAG_COMPETE) ){
			temp_obj = unequip_char (src, WEAR_PRIM);
			obj_to_room (temp_obj, src->in_room);
		}
		
		else if ( (tobj = get_equip (src, WEAR_SEC)) == attack_weapon && !IS_SET(src->flags, FLAG_COMPETE) ){
			temp_obj = unequip_char (src, WEAR_SEC);
			obj_to_room (temp_obj, src->in_room);
		}
		
		else if ( (tobj = get_equip (src, WEAR_BOTH)) == attack_weapon && !IS_SET(src->flags, FLAG_COMPETE) ){
			temp_obj = unequip_char (src, WEAR_BOTH);
			obj_to_room (temp_obj, src->in_room);
		}
		
		else if ( !IS_SET (src->flags, FLAG_COMPETE) ){
			system_log("Disarm, but couldn't find weapons's hand. (attacker)", TRUE);
		}

		if ( !IS_SET (src->flags, FLAG_COMPETE) ) {
			if ( attack_weapon == src->right_hand )
				src->right_hand = NULL;
			else if ( attack_weapon == src->left_hand )
				src->left_hand = NULL;
			add_second_affect (SPA_GET_OBJ, 6, src, attack_weapon, "", 0);
			add_second_affect (SPA_WEAR_OBJ, 10, src, attack_weapon, "", 0);
			SET_BIT (attack_weapon->tmp_flags, SPA_DROPPED);
		}
	}

	else if ( off_result == RESULT_WEAPON_BREAK ) {
		if ( get_equip (src, WEAR_PRIM) == attack_weapon ){
			temp_obj = unequip_char (src, WEAR_PRIM);
			extract_obj (temp_obj);
		}
		
		else if ( get_equip (src, WEAR_SEC) == attack_weapon ){
			temp_obj =  unequip_char (src, WEAR_SEC);
			extract_obj (temp_obj);
		}
		
		else if ( get_equip (src, WEAR_BOTH) == attack_weapon ){
			temp_obj = unequip_char (src, WEAR_BOTH);
			extract_obj (temp_obj);
		}
		
		else{
			system_log("Weapon Break, but couldn't find weapons's hand. (attacker)", TRUE);
	}
		if ( attack_weapon == src->right_hand )
			src->right_hand = NULL;
		else if ( attack_weapon == src->left_hand )
			src->left_hand = NULL;
	}

	else if ( off_result == RESULT_ADV )
		system_log("How could offence get RESULT_ADV?", TRUE);

	if ( def_result == RESULT_FUMBLE ) {
		if ( get_equip (tar, WEAR_PRIM) == defense_weapon && !IS_SET (tar->flags, FLAG_COMPETE) ){
			temp_obj = unequip_char (tar, WEAR_PRIM);
			obj_to_room (temp_obj, tar->in_room);
		}
		
		else if ( get_equip (tar, WEAR_SEC) == defense_weapon && !IS_SET (tar->flags, FLAG_COMPETE) ){
			temp_obj = unequip_char (tar, WEAR_SEC);
			obj_to_room (temp_obj, tar->in_room);
		}
		
		else if ( get_equip (tar, WEAR_BOTH) == defense_weapon && !IS_SET (tar->flags, FLAG_COMPETE) ){
			temp_obj = unequip_char (tar, WEAR_BOTH);
			obj_to_room (temp_obj, tar->in_room);
		}
		
		else if ( get_equip (tar, WEAR_SHIELD) == defense_weapon && !IS_SET (tar->flags, FLAG_COMPETE) ){
			temp_obj = unequip_char (tar, WEAR_SHIELD);
			obj_to_room (temp_obj, tar->in_room);
		}
		
		else if ( !IS_SET (tar->flags, FLAG_COMPETE) ){
			system_log("Disarm, but couldn't find weapons's hand. (defender)", TRUE);
		}
		
		if ( !IS_SET (tar->flags, FLAG_COMPETE) ) {
			if ( defense_weapon == tar->right_hand )
				tar->right_hand = NULL;
			else if ( defense_weapon == tar->left_hand )
				tar->left_hand = NULL;
			add_second_affect (SPA_GET_OBJ, 6, tar, defense_weapon, "", 0);
			add_second_affect (SPA_WEAR_OBJ, 10, tar, defense_weapon, "", 0);
			SET_BIT (defense_weapon->tmp_flags, SPA_DROPPED);
		}
	}

	else if ( def_result == RESULT_WEAPON_BREAK ||
			  def_result == RESULT_SHIELD_BREAK ) {
		if ( get_equip (tar, WEAR_PRIM) == defense_weapon ){
			temp_obj = unequip_char (tar, WEAR_PRIM);
			extract_obj (temp_obj);
		}
		
		else if ( get_equip (tar, WEAR_SEC) == defense_weapon ){
			temp_obj = unequip_char (tar, WEAR_SEC);
			extract_obj (temp_obj);
		}
		
		else if ( get_equip (tar, WEAR_BOTH) == defense_weapon ){
			temp_obj = unequip_char (tar, WEAR_BOTH);
			extract_obj (temp_obj);
		}
		
		else if ( get_equip (tar, WEAR_SHIELD) == defense_weapon ){
			temp_obj = unequip_char (tar, WEAR_SHIELD);
			extract_obj (temp_obj);
		}
		
		else{
			system_log("BREAK, but couldn't find weapons's hand. (defender)", TRUE);
		}

		if ( defense_weapon == tar->right_hand )
			tar->right_hand = NULL;
		else if ( defense_weapon == tar->left_hand )
			tar->left_hand = NULL;
	}

	else if ( def_result == RESULT_STUMBLE ) {
		if ( GET_POS (tar) == FIGHT || GET_POS (tar) == STAND ) {
			GET_POS (tar) = SIT;
			add_second_affect (SPA_STAND, 8, tar, NULL, NULL, 0);
		} else
			def_result = RESULT_NONE;
	}

	if ( attack_weapon )
		hit_type = attack_weapon->o.weapon.hit_type;
	else {
		if ( src->nat_attack_type == 0 )
			hit_type = 9;
		else if ( src->nat_attack_type == 1 || src->nat_attack_type == 3 )
			hit_type = 7;
		else if ( src->nat_attack_type == 2 )
			hit_type = 8;
	}

	if ( broken_eq ) {
		temp_arg = obj_short_desc(broken_eq);
		snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 is destroyed by the blow.", temp_arg);
		buf[2] = toupper(buf[2]);
		act (buf, FALSE, tar, 0, 0, TO_CHAR | TO_ACT_FORMAT | TO_ACT_COMBAT);
		act (buf, FALSE, tar, 0, 0, TO_ROOM | TO_ACT_FORMAT | TO_ACT_COMBAT);
		extract_obj (broken_eq);
	}

	tar->delay_ch = src;
	if ( attack_weapon )
		tar->delay_info1 = attack_weapon->virtual;

	if ( !invulnerability && damage && wound_to_char (tar, location, damage, hit_type, 0, 0, 0) ) {
		def_result = RESULT_DEAD;
	}

	else {

		if ( tar->fighting && IS_SET (tar->act, ACT_VEHICLE) )
			stop_fighting (tar);

		if ( !tar->fighting && !IS_SET (tar->act, ACT_VEHICLE) ) {
			set_fighting (tar, src);
			if ( !AWAKE (tar) && GET_POS(tar) != POSITION_UNCONSCIOUS )
				do_wake (tar, "", 0);
			add_second_affect (SPA_STAND, 8, tar, NULL, NULL, 0);
		}

		if ( !src->fighting )
			set_fighting (src, tar);

		if ( (GET_POS(tar) == POSITION_STUNNED || GET_POS(tar) == POSITION_UNCONSCIOUS)
			 && !IS_SET (src->flags, FLAG_KILL) ) {

			if ( src->fighting )
				stop_fighting (src);

			for ( tch = src->room->people; tch; tch = tch->next_in_room )
				if ( tch->fighting == src && tch != src && GET_HIT (tch) > 0 ) {
					set_fighting (src, tch);
					break;
				}
		}
	}

	tar->delay_ch = NULL;
	tar->delay_info1 = 0;


	if ( def_result == RESULT_PARRY || def_result == RESULT_BLOCK ) {
		if ( get_equip (tar, WEAR_PRIM) == defense_weapon ||
			 get_equip (tar, WEAR_BOTH) == defense_weapon )
			tar->primary_delay++;
		else if ( get_equip (tar, WEAR_SEC) == defense_weapon )
			tar->secondary_delay++;
	}

	else if ( def_result == RESULT_ADV ) {
		tar->primary_delay = 0;
		tar->secondary_delay = 0;
	}

	attack_delay = 0;

				/* Weapon delay / natural delay / calculated delay */

	if ( attack_weapon ) {
		attack_delay += use_table [attack_weapon->o.weapon.use_skill].delay +
						attack_weapon->o.weapon.delay;
		attack_delay += attack_weapon->o.od.value[5];
		if ( (attack_weapon->o.od.value[3] == SKILL_MEDIUM_EDGE || attack_weapon->o.od.value[3] == SKILL_MEDIUM_BLUNT ||
			attack_weapon->o.od.value[3] == SKILL_MEDIUM_PIERCE) && attack_weapon->location == WEAR_BOTH ) 
			attack_delay += 8;
	}

	else if ( IS_NPC (src) )
		attack_delay += src->natural_delay;
	else
		attack_delay += 20 - GET_DEX (src) + use_table [SKILL_BRAWLING].delay;

	snprintf (AD, MAX_STRING_LENGTH, "AttDel %d ", attack_delay);

				/* str+dex delay adjustment */

	delay_modifier = 100;

	delay_modifier = delay_modifier -
					 (GET_STR (src) + GET_DEX (src) - 15);

	snprintf (AD, MAX_STRING_LENGTH, "STRDEXadj %d%% ", delay_modifier);

	attack_delay = attack_delay * delay_modifier / 100;

				/* Fatigue adjustment */

	if ( GET_MAX_MOVE (src) > 0 )
		j = GET_MOVE (src) * 100 / GET_MAX_MOVE (src);
	else
		j = 0;

	if ( j > 100 )
		j = 100;

	for ( i = 0; j > fatigue [i].percent; i++ )
		;

	if ( i == 0 )
		attack_delay += 8;		/* Completely Exhausted */
	else if ( i == 1 )
		attack_delay += 4;		/* Exhausted */

	snprintf (AD, MAX_STRING_LENGTH, "Fatigue %d ", attack_delay);

				/* Fightmode delay adjustment */

	attack_delay += fight_tab [src->fight_mode].delay;

	snprintf (AD, MAX_STRING_LENGTH, "FM %d = %d [%d] (2)=%d\n", attack_delay, attack_delay,
				attack_num, src->secondary_delay);

	if ( attack_num == 1 )
		src->primary_delay = attack_delay;
	else
		src->secondary_delay = attack_delay;
}

void figure_damage (CHAR_DATA *src, CHAR_DATA *tar, OBJ_DATA *attack_weapon,
					int off_result, int *damage, int *location)
{
	OBJ_DATA		*eq;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	int			body_type;
	int			wear_loc1;
	int			wear_loc2;
	int			i;
	float			dam = 0;
	AFFECTED_TYPE	*shock = NULL;
	AFFECTED_TYPE	*af;

	*damage = 0;

			/* Determine hit location */

	body_type = 0;

	i = number (1, 100);
	*location = -1;

	while ( i > 0 )
		i = i - body_tab [body_type] [++(*location)].percent;

	wear_loc1 = body_tab [body_type] [*location].wear_loc1;
	wear_loc2 = body_tab [body_type] [*location].wear_loc2;


			/* Assess automatic damage by NPCs */

	if ( IS_NPC (src) )
		dam += src->mob->damroll;

			/* For weapons, add weapon damage roll and affects */

	if ( attack_weapon ) {
		if ( attack_weapon->o.weapon.dice && attack_weapon->o.weapon.sides ) {
			if ( (attack_weapon->o.od.value[3] == SKILL_MEDIUM_EDGE || attack_weapon->o.od.value[3] == SKILL_MEDIUM_BLUNT ||
				attack_weapon->o.od.value[3] == SKILL_MEDIUM_PIERCE) && attack_weapon->location == WEAR_BOTH ) 
            			dam += dice (attack_weapon->o.weapon.dice, attack_weapon->o.weapon.sides+2);
            		else dam += dice (attack_weapon->o.weapon.dice, attack_weapon->o.weapon.sides);
		} else {
	            snprintf (buf, MAX_STRING_LENGTH,  "Ineffective weapon, vnum %d on mob %d room %d\n",
        	        attack_weapon->virtual, src->mob ? src->mob->virtual : -1, src->in_room);
			system_log (buf, TRUE);
		}

		for ( af = attack_weapon->xaffected; af; af = af->next ) {
			if ( af->a.spell.location == APPLY_DAMROLL )
				dam += af->a.spell.modifier;
		}
	}

			/* For NPCs with no weapons, add natural attack */

	else if ( IS_NPC (src) ) {

		if ( src->mob->damnodice * src->mob->damsizedice < 8 && shock )
			dam += dice (2, 4);
		else
			dam += dice (src->mob->damnodice, src->mob->damsizedice);
	}

			/* For bare handed PCs */

	else if ( shock )
		dam += dice (2, 4);
	else
		dam += number (0, 2);

			/* Subtract the armor protection at the hit location */

	eq = get_equip (tar, wear_loc1);

	if ( eq && eq->obj_flags.type_flag == ITEM_ARMOR )
		if ( attack_weapon || !shock )
			dam -= eq->o.armor.armor_value;

			/* Mobs will have marmor, which is natural armor */

	if ( attack_weapon || !shock )
		dam -= (tar->armor);

			/* Weapon vs armor */

	eq = get_equip (tar, wear_loc2);

	if ( attack_weapon && eq )
		dam += weapon_armor_table [attack_weapon->o.weapon.hit_type]
								  [eq->o.armor.armor_type];
	else if ( !attack_weapon && eq )
		dam += weapon_nat_attack_table [src->nat_attack_type]
								  [eq->o.armor.armor_type];

			/* Multiply by hit location multiplier */

	dam *= (body_tab [body_type] [*location].damage_mult*1.0) /
			    (body_tab [body_type] [*location].damage_div*1.0);

	/* Multiply in critical strike bonus */

	if ( off_result == RESULT_HIT || (off_result == RESULT_HIT && (!attack_weapon && src->nat_attack_type == 0)) )
		dam *= 1;
	else if ( off_result == RESULT_HIT1 )
		dam *= 1.3;
	else if ( off_result == RESULT_HIT2 )
		dam *= 1.5;
	else if ( off_result == RESULT_HIT3 || (off_result == RESULT_HIT && !attack_weapon) ) {
		dam += 2;
		dam *= 1.7;
	}
	else if ( off_result == RESULT_HIT4 ) {
		dam += 3;
		dam *= 2;
	}
	else {
		*damage = 0;
		return;
	}

			/* Subtract/add spell offsets */

	if ( (af = get_affect (tar, MAGIC_AFFECT_ARMOR)) )
		if ( attack_weapon || !shock )
			dam -= af->a.spell.modifier;

		/* Reduce damage by SANCTUARY or BLESS.  Note:  Not cumulative */

	if ( get_affect (tar, MAGIC_AFFECT_CURSE) && !get_affect (tar, MAGIC_AFFECT_BLESS) )
		dam += dam / 4 + 1;

	else if ( get_affect (tar, MAGIC_AFFECT_BLESS) &&
			  !get_affect (tar, MAGIC_AFFECT_CURSE) )
		dam = dam * 3 / 4;

	if ( attack_weapon || src->nat_attack_type > 0 )
		(float) (dam *= COMBAT_BRUTALITY);

	dam = (int)dam;

	if ( dam <= 0 )
		dam = number (0, 1);

	*damage = dam;
}

int weaken (CHAR_DATA *victim, int hits, int moves, char *log_msg)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( hits == 0 && moves == 0 )
		return 0;

	if ( log_msg ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s (%dh/%dm)", log_msg, hits, moves);
		add_combat_log(victim, buf);
	}

	if ( !IS_MORTAL (victim) && !IS_NPC (victim) )
		return 0;

		/* If moves is negative, then we subtract excess moves from hits */

	moves = -moves;

	if ( GET_MOVE (victim) > moves )
		GET_MOVE (victim) -= moves;
	else
		GET_MOVE (victim) = 0;

	if ( GET_POS (victim) == POSITION_DEAD ) {

		if ( moves ) {
			act ("$n dies of exhaustion!", FALSE, victim, 0, 0, TO_ROOM);
			add_combat_log(victim, "Death by exhaustion");
		}

		die (victim);

		return 1;
	}

	return 0;
}

void subdue_resync (CHAR_DATA *ch)
{
	CHAR_DATA		*tch;

		/* ch needs someone else to subdue */

	if ( ch->fighting )
		stop_fighting (ch);

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {

		if ( tch->fighting == ch ) {
			set_fighting (ch, tch);

			act ("You turn to confront $N.", FALSE, ch, 0, tch, TO_CHAR);
			act ("$n turns to confront $N.", FALSE, ch, 0, tch, TO_ROOM);
			act ("$N turns to confront you.", FALSE, tch, 0, ch, TO_CHAR);

			if ( !IS_SET (tch->flags, FLAG_SUBDUING) )
				REMOVE_BIT (ch->flags, FLAG_SUBDUING);
			break;
		}
	}

	if ( !ch->fighting )
		send_to_char ("You stop fighting.\n\r", ch);
}

void remove_subduer (CHAR_DATA *ch)
{
	CHAR_DATA		*tch;
	int				was_in_room;

	for ( tch = ch->room->people; tch == ch; )
		tch = tch->next_in_room;

	was_in_room = ch->in_room;

	char_from_room (ch);

	for ( ; tch; tch = tch->next_in_room )
		if ( tch->fighting == ch )
			subdue_resync (tch);

	char_to_room (ch, was_in_room);
}

void subdue_char (CHAR_DATA *ch, CHAR_DATA *victim)
{
	int				victim_was_room = 0;
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*temp_obj = NULL;
	AFFECTED_TYPE	*af = NULL;

	if ( ch->fighting )
		stop_fighting (ch);

	if ( victim->fighting )
		stop_fighting (victim);

	REMOVE_BIT (ch->flags, FLAG_SUBDUING);
	REMOVE_BIT (ch->flags, FLAG_SUBDUEE);
	REMOVE_BIT (victim->flags, FLAG_SUBDUING);
	REMOVE_BIT (victim->flags, FLAG_SUBDUER);

	SET_BIT (ch->flags, FLAG_SUBDUER);
	SET_BIT (victim->flags, FLAG_SUBDUEE);

	clear_moves (victim);
	clear_current_move (victim);

	ch->subdue = victim;
	victim->subdue = ch;

    if ( GET_POS(victim) < POSITION_STANDING ) {
	    act ("You fall upon your foe and get $M into a firm headlock.", FALSE, ch, 0, victim, TO_CHAR);
	    act ("$N falls upon you and gets you into a firm headlock.", FALSE, victim, 0, ch, TO_CHAR);
	    act ("$n falls upon $N.", FALSE, ch, 0, victim, TO_NOTVICT);
	    act ("You haul $N to $S feet.", FALSE, ch, 0, victim, TO_CHAR);
	    act ("$N hauls you to your feet.", FALSE, victim, 0, ch, TO_CHAR);
	    act ("$e hauls $M to $S feet in a firm headlock.", FALSE, ch, 0, victim, TO_NOTVICT);
        GET_POS(victim) = POSITION_STANDING;
    } else {
	    act ("You grapple $N and lock $M firmly in your grasp.", FALSE, ch, 0, victim, TO_CHAR);
	    act ("$N grapples you and locks you firmly in $S grasp!", FALSE, victim, 0, ch, TO_CHAR);
	    act ("$n grapples $N and locks $M firmly in $s grasp.", FALSE, ch, 0, victim, TO_NOTVICT);
    }
		/* Nobody is trying to subdue ch.  Lets stop or redirect subdue
           attempts against victim */

	victim_was_room = victim->in_room;

	char_from_room (victim);

	remove_subduer (ch);

	char_to_room (victim, victim_was_room);

	remove_subduer (victim);

	if ( ch->mob &&
		 IS_SET (ch->act, ACT_ENFORCER) &&
		 is_hooded (victim) ) {

		obj = get_equip (victim, WEAR_ABOUT);
		if ( obj ) {
			temp_obj = unequip_char (victim, WEAR_ABOUT);
			obj_to_char (temp_obj, victim);
			act ("$n removes your $p.", FALSE, ch, obj, victim, TO_VICT);
			act ("$n removes $N's $p.", FALSE, ch, obj, victim, TO_NOTVICT);
			act ("You remove $N's $p.", FALSE, ch, obj, victim, TO_CHAR);
		}
		
		obj = get_equip (victim, WEAR_HEAD);
		if ( obj ) {
			temp_obj = unequip_char (victim, WEAR_HEAD);
			obj_to_char (temp_obj, victim);
			act ("$n removes your $p.", FALSE, ch, obj, victim, TO_VICT);
			act ("$n removes $N's $p.", FALSE, ch, obj, victim, TO_NOTVICT);
			act ("You remove $N's $p.", FALSE, ch, obj, victim, TO_CHAR);
		}

		if ( (af = get_affect (victim, MAGIC_CRIM_HOODED + ch->room->zone)) )
			add_criminal_time (victim, ch->room->zone, af->a.spell.modifier);
	}
}


void perform_violence (void)
{
	CHAR_DATA	*ch;
	CHAR_DATA	*new_combat_list = NULL;

	for ( ch = combat_list; ch; ch = combat_next_dude ) {

		if ( ch->next_fighting && (!ch->next_fighting->fighting || !ch->next_fighting->room) )
			ch->next_fighting = ch->next_fighting->next_fighting;

		combat_next_dude = ch->next_fighting;

		if ( !ch->fighting || !ch->fighting->room )
			continue;

		if ( get_affect (ch, MAGIC_HIDDEN) ) {
			remove_affect_type (ch, MAGIC_HIDDEN);
			act ("$n reveals $mself.", TRUE, ch, 0, 0, TO_ROOM);
		}

	        if ( IS_NPC (ch) && !IS_SET (ch->flags, FLAG_FLEE) && (IS_SET (ch->flags, FLAG_AUTOFLEE) || morale_broken (ch))) {
			ch->speed = 4;
                	do_flee (ch, "", 0);
                	add_threat (ch, ch->fighting, 5);
			continue;
        	}

		if ( !ch->fighting ) {
			sigsegv(SIGSEGV);
		}

			/* Remove delays from both hands */

		ch->primary_delay -= pulse_violence;
		ch->secondary_delay -= pulse_violence;

		if ( ch->primary_delay < 0 )
			ch->primary_delay = 0;

		if ( ch->secondary_delay < 0 )
			ch->secondary_delay = 0;

			/* Stop fighting if player is physically incapable */

		if ( !AWAKE (ch) ||
			 ch->in_room != ch->fighting->in_room ||
			 get_affect (ch, MAGIC_AFFECT_PARALYSIS) ) {
			stop_fighting (ch);
			continue;
		}

			/* No combat if there are delays */

		if ( ch->primary_delay && ch->secondary_delay )
			continue;

		if ( GET_POS (ch) == STAND )
			GET_POS (ch) = FIGHT;

		if ( GET_POS (ch) != FIGHT )
			continue;

		if ( ch->mount && !ch->mount->fighting )
			set_fighting (ch->mount, ch->fighting);

		hit_char (ch, ch->fighting, 0);

		if ( combat_next_dude && combat_next_dude->deleted )
			system_log("Loss of combat_next_dude in fight.c: perform_violence.", TRUE);
	}

		/* Reverse the combat list */

	new_combat_list = combat_list;

	if ( combat_list ) {
		combat_list = combat_list->next_fighting;
		new_combat_list->next_fighting = NULL;
	}

	while ( combat_list ) {

		ch = combat_list;
		combat_list = combat_list->next_fighting;

		ch->next_fighting = new_combat_list;
		new_combat_list = ch;
	}

	combat_list = new_combat_list;
}

void do_stop (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch;
	AFFECTED_TYPE	*af;

	if ( is_mounted (ch) && ch->mount->moves )
		ch = ch->mount;

	if ( ch->moves ) {

		clear_moves (ch);
		clear_current_move (ch);

		send_to_char ("Movement commands cancelled.\n", ch);

		if ( is_mounted (ch) )
			send_to_char ("Movement commands cancelled.\n", ch->mount);

		return;
	}
  
	if ( (af = is_crafting(ch)) ) {
        	act ("$n stops doing $s craft.", FALSE, ch, 0, 0, TO_ROOM);
                send_to_char ("You stop doing your craft.\n", ch);
                af->a.craft->timer = 0;
                return;
	}

	if ( (af = get_affect (ch, MAGIC_TOLL)) ) {

		if ( af->a.toll.room_num == ch->in_room ) {
			stop_tolls (ch);
			return;
		}

			/* Toll affect should have been there...continue with stop */

		stop_tolls (ch);
	}

	if ( clear_current_move (ch) )
		return;

	if ( ch->delay || ch->aim ) {
		break_delay (ch);
		return;
	}

	if ( GET_TRUST (ch) ) {
		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( tch->fighting ) {
				if ( tch != ch ) {
					act ("You immediately obey $N's command to stop fighting!",
						  TRUE, tch, 0, ch, TO_CHAR);
					act ("$N obeys.", FALSE, ch, 0, tch, TO_CHAR);
				}
				forget (tch, tch->fighting);
				stop_fighting (tch);
			}
			if ( IS_NPC (tch) ) {
				tch->attackers = NULL;
				tch->threats = NULL;
			}
		}
		send_to_char ("All combat in the room has been stopped.\n", ch);
		return;
	}
	
	if ( !ch->fighting ) {
		send_to_char ("You're not fighting anyone.\n\r", ch);
		return;
	}

	if ( ch->fighting->fighting != ch || GET_FLAG (ch->fighting, FLAG_FLEE) ) {
		send_to_char ("You stop fighting.\n\r", ch);
		stop_fighting (ch);
		return;
	}

	SET_BIT (ch->act, PLR_STOP);		/* Same as ACT_STOP */

	if ( IS_SET (ch->fighting->act, PLR_STOP) ) {

		send_to_char ("Your opponent agrees.\n\r", ch);

		act ("Both you and $N stop fighting.\n\r",
			FALSE, ch->fighting, 0, ch, TO_CHAR);

		if ( IS_NPC (ch->fighting) ) {
			remove_threat (ch->fighting, ch);
			remove_attacker (ch->fighting, ch);
		}
		if ( IS_NPC (ch) ) {
			remove_threat (ch, ch->fighting);
			remove_attacker (ch, ch->fighting);
		}

		REMOVE_BIT (ch->act, PLR_STOP);
		REMOVE_BIT (ch->fighting->act, PLR_STOP);

		if ( ch->fighting->fighting )
			stop_fighting (ch->fighting);

		if ( ch->fighting )
			stop_fighting (ch);

		return;
	}

	send_to_char ("You motion for a truce to your opponent.\n\r", ch);
	act ("$N motions for a truce.", FALSE, ch->fighting, 0, ch, TO_CHAR);
	act ("$N motions for a truce with $n.",
			FALSE, ch->fighting, 0, ch, TO_ROOM);
}

void do_release (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*target;

	if ( !IS_SUBDUER (ch) ) {

		if ( !IS_SET (ch->flags, FLAG_SUBDUER) ) {
			send_to_char ("You have no prisoner in tow.", ch);
			return;
		}

		REMOVE_BIT (ch->flags, FLAG_SUBDUER);

		if ( ch->subdue &&
			 is_he_somewhere (ch->subdue) &&
			 ch->subdue->subdue == ch ) {
			REMOVE_BIT (ch->subdue->flags, FLAG_SUBDUER);
			REMOVE_BIT (ch->subdue->flags, FLAG_SUBDUEE);
			send_to_char ("You are no longer a prisoner.\n\r", ch->subdue);
			send_to_char ("Ok.\n\r", ch);
			ch->subdue->subdue = NULL;
			return;
		}

		ch->subdue = NULL;
		send_to_char ("Alright.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf && name_is (buf, GET_NAMES (ch->subdue)) )
		argument = one_argument (argument, buf);

	if ( !*buf ) {
		release_prisoner (ch, NULL);
		return;
	}

	if ( !str_cmp (buf, "to") ) {

		argument = one_argument (argument, buf);

		if ( !*buf ) {
			send_to_char ("Who do you want to release your prisoner to?\n\r", ch);
			return;
		}
	}

	if ( !(target = get_char_room_vis (ch, buf)) ) {
		send_to_char ("There is no such person here to receive your prisoner.\n\r", ch);
		return;
	}

	release_prisoner (ch, target);
}

int release_prisoner (CHAR_DATA *ch, CHAR_DATA *target)
{
	int		quiet = 0;

	if ( !target ) {

		if ( !IS_SUBDUER (ch) ) {
			REMOVE_BIT (ch->flags, FLAG_SUBDUER);
			act ("You have no prisoner.", FALSE, ch, 0, 0, TO_CHAR);
			return 0;
		}

		act ("$N releases you.", FALSE, ch->subdue, 0, ch, TO_CHAR);
		act ("You release $N.", FALSE, ch, 0, ch->subdue, TO_CHAR);
		act ("$n releases $N.", FALSE, ch, 0, ch->subdue, TO_NOTVICT);

		REMOVE_BIT (ch->flags, FLAG_SUBDUER);
		REMOVE_BIT (ch->subdue->flags, FLAG_SUBDUEE);

		ch->subdue->subdue = NULL;
		ch->subdue = NULL;

		return 1;
	}

	if ( IS_SUBDUER (target) ) {
		act ("$N has $S arms full with another prisoner.",
						FALSE, ch, 0, target, TO_CHAR);
		return 0;
	}

	if ( IS_SUBDUEE (target) ) {
		act ("$N is a prisoner himself!", FALSE, ch, 0, target, TO_CHAR);
		return 0;
	}

	if ( GET_POS (target) != STAND ) {
		act ("$N cannot accept your prisoner at the moment.",
						FALSE, ch, 0, target, TO_CHAR);
		return 0;
	}

	if ( !real_skill (target, SKILL_SUBDUE) ) {
		act ("$N isn't able to take care of your prisoner.",
						FALSE, ch, 0, target, TO_CHAR);
		return 0;
	}

	target->subdue = ch->subdue;
	target->subdue->subdue = target;
	ch->subdue = NULL;

	SET_BIT (target->flags, FLAG_SUBDUER);
	REMOVE_BIT (ch->flags, FLAG_SUBDUER);

	act ("You release your prisoner to $N.", FALSE, ch, 0, target, TO_CHAR);
	act ("$N releases his prisoner to you.", FALSE, target, 0, ch, TO_CHAR);

	if ( IS_SET (target->subdue->act, PLR_QUIET) )
		quiet = 1;
	else
		SET_BIT (target->subdue->act, PLR_QUIET);

	act ("$n releases his prisoner to $N.", FALSE, ch, 0, target, TO_NOTVICT);

	if ( !quiet )
		REMOVE_BIT (target->subdue->act, PLR_QUIET);

	act ("You are tossed into the arms of $N.", FALSE, target->subdue, 0, target, TO_CHAR);

	trigger (ch, "", TRIG_PRISONER);

	return 1;
}

/*int retaliate (CHAR_DATA *ch, CHAR_DATA *subject)
{
	if (!IS_NPC(ch))
        return;

    if (IS_SET(ch->act, ACT_WIMPY)) {
        s
	    return 0;

    ;
}*/

int flee_room (CHAR_DATA *ch)
{
	ROOM_DATA		*room_exit;
	ROOM_DATA		*room;
	int				room_exit_virt;
	int				exit_tab [6];
	int				zone;
	int				num_exits = 0;
	int				to_exit;
	int				i;
	char			log_msg [MAX_STRING_LENGTH] = {'\0'};

	room = vtor (ch->in_room);
	zone = room->zone;

    if ( GET_POS (ch) == POSITION_FIGHTING ) {
        do_flee (ch,"",0);
        return 0;
    }

    if ( GET_POS (ch) < POSITION_RESTING )
        return 0;

    if ( GET_POS (ch) < POSITION_STANDING )
        do_stand (ch,"",0);

  	if ( ch->desc && ch->desc->original )
    	return 0;

	for ( i = 0; i < 6; i++ ) {

		if ( !CAN_GO (ch, i) )
			continue;

		room_exit = vtor (PASSAGE (ch, i)->to_room);

		if ( !room_exit ) {
			snprintf (log_msg, MAX_STRING_LENGTH, "ERROR:  Room %d, dir %d doesn't go to %d",
				ch->in_room, i, PASSAGE (ch, i)->to_room);

			system_log(log_msg, TRUE);
			continue;
		}
        exit_tab [num_exits++] = i;
	}

	if ( num_exits == 0 )
		return 0;

	to_exit = number (1, num_exits) - 1;

	if ( vtor (PASSAGE (ch, exit_tab [to_exit])->to_room)->virtual ==
				ch->last_room )
		to_exit = (to_exit + 1) % num_exits;

	room_exit_virt = vtor (PASSAGE (ch, exit_tab [to_exit])->to_room)->virtual;

	ch->last_room = room_exit_virt;

	do_move (ch, "", exit_tab [to_exit]);

	return 1;
}

void do_subdue (CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA		*target = NULL;
	char			*temp_arg = NULL;
	
	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("You can't do that in an OOC area.\n", ch);
		return;
	}

	if ( !db_race_table [ch->race].can_subdue ) {
		snprintf (buf, MAX_STRING_LENGTH,  "A %s cannot subdue.\n",
						db_race_table [ch->race].name);
		send_to_char (buf, ch);
		return;
	}

    	argument = one_argument (argument, buf);

    	if (!*buf && !ch->fighting) {
        	send_to_char ("Subdue whom?\n\r",ch);
        	return;
    	}

	if ( ch->subdue && !is_he_here (ch, ch->subdue, 0) )
		ch->subdue = NULL;

	if ( ch->fighting && !is_he_here (ch, ch->fighting, 0) )
		stop_fighting (ch);

    	if ( ch->fighting && (!*buf || !name_is (buf, GET_NAMES(ch->fighting))))
        	target = ch->fighting;

	else if ( !(target = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see them here.\n\r", ch);
		return;
	}

    	else if (ch == target) {
        	send_to_char("Be serious.\n\r",ch);
        	return;
    	}
/*
    	else if ( !db_race_table [target->race].can_subdue ) {
		act ("$N cannot be subdued.", FALSE, ch, 0, target, TO_CHAR);
		return;
	}
*/
    	else if (ch->subdue && name_is (buf, GET_NAMES(ch->subdue))) {
        	send_to_char("You already have them subdued!\n\r",ch);
        	return;
    	}

    	else if ( IS_SUBDUEE (target) ) {
    		temp_arg = char_short (target->subdue);
		snprintf (buf, MAX_STRING_LENGTH,  "$N has already been subdued by %s.\n", temp_arg);
        	act (buf, FALSE, ch, 0, target, TO_CHAR);
		return;
	}

    	else if ( ch->subdue ) {
		send_to_char ("Release your current prisoner, first.\n",
					  ch);
		return;
	}

	if ( GET_POS (target) != UNCON && GET_POS (target) != SLEEP ) {
		send_to_char ("They must be unconscious or asleep, first.\n", ch);
		return;
	}

    	if ( IS_NPC(ch) && !ch->desc ) {
        	do_sheathe (ch,"",0);
		do_wear (ch, "shield", 0);
        	do_sheathe (ch,"",0);
		if ( ch->right_hand ) {
			(void)one_argument (ch->right_hand->name, buf);
			do_wear (ch, buf, 0);
		}
		if ( ch->left_hand ) {
			(void)one_argument (ch->left_hand->name, buf);
			do_wear (ch, buf, 0);
		}
    	}

	if ( ch->right_hand || ch->left_hand ) {
		send_to_char ("You'll need both hands free to subdue.\n", ch);
		return;
	}

	if ( GET_POS (target) == SLEEP ) {
		GET_POS (target) = STAND;
		if ( number(1,100) <= target->skills [SKILL_LISTEN] ) {
			do_wake (target, "", 0);
			act ("You are startled awake as $N unsuccessfully attempts to grab you!", FALSE, target, 0, ch, TO_CHAR | TO_ACT_FORMAT);
			act ("$n is startled awake as you unsuccessfully attempt to grab $m!", TRUE, target, 0, ch, TO_VICT | TO_ACT_FORMAT);
			return;
		}
		else {
			act ("You are awakened abruptly!", FALSE, target, 0, ch, TO_CHAR | TO_ACT_FORMAT);
		}			
	}

	else if ( ch->fighting ) {
        send_to_char("You abandon your opponent.\n\r",ch);
	    act("$N turns away from you suddenly.",FALSE,ch->fighting,0,ch,TO_CHAR);
		act("$n turns away from $N.",FALSE,ch,0,target,TO_NOTVICT);
	}

	if ( IS_SUBDUER (target) )
		release_prisoner (target, NULL);

	subdue_char (ch, target);
}

void do_escape (CHAR_DATA *ch, char *argument, int cmd)
{
	int			chance;
	int			pressure;
	char		log_message [MAX_STRING_LENGTH] = {'\0'};

	if ( !IS_SUBDUEE (ch) ) {

		if ( ch->fighting )
			do_flee (ch, "", 0);
		else
			send_to_char ("You are not currently subdued.\n", ch);

		return;
	}

	if ( get_second_affect (ch, SPA_ESCAPE, NULL) ) {
		act ("$N is still holding you very tightly.",
				FALSE, ch, 0, ch->subdue, TO_CHAR);
		return;
	}

	chance = 20 + 2 * (GET_STR (ch) + GET_AGI (ch) - GET_STR (ch->subdue)
					- GET_DEX (ch->subdue));

	if ( chance > number (0, 100) ) {
		act ("You manage to wriggle free of $N's grasp!",
				FALSE, ch, 0, ch->subdue, TO_CHAR);
		act ("$N wriggles free of your grasp!",
				FALSE, ch->subdue, 0, ch, TO_CHAR);
		act ("$n escapes from $N's grasp.",
				TRUE, ch, 0, ch->subdue, TO_NOTVICT);

		REMOVE_BIT (ch->subdue->flags, FLAG_SUBDUER);
		REMOVE_BIT (ch->flags, FLAG_SUBDUEE);

		ch->subdue->subdue = NULL;
		ch->subdue = NULL;

		return;
	}

	act ("$N chokes you when you attempt to escape.",
				FALSE, ch, 0, ch->subdue, TO_CHAR);
	act ("You choke $N to prevent $M from escaping.",
				FALSE, ch->subdue, 0, ch, TO_CHAR);
	act ("$n tries and fails to escape $N.",
				TRUE, ch, 0, ch->subdue, TO_NOTVICT);

	pressure = GET_STR (ch->subdue) - 10;

	if ( pressure > 0 ) {

		snprintf (log_message, MAX_STRING_LENGTH, "Choked by %s while escaping", ch->tname);

		if ( wound_to_char (ch, "neck", number(1,pressure),9, 0, 0, 0) ) {
			act ("$n crumples to the ground dead!",
				 FALSE, ch->subdue, 0, ch, TO_CHAR);
			act ("$n chokes $N to death!",
				 FALSE, ch, 0, ch->subdue, TO_NOTVICT);
			return;
		}
	}

	add_second_affect (SPA_ESCAPE, 10, ch, NULL, NULL, 0);
}

void do_choke (CHAR_DATA *ch, char *argument, int cmd)
{
	int		pressure;

	send_to_char ("This command is disabled.\n", ch);
	return;

	if ( !IS_SUBDUER (ch) ) {
		send_to_char ("You can only choke someone you have subdued.\n", ch);
		return;
	}

	pressure = GET_STR (ch) - 10;

	if ( pressure <= 0 ) {
		act ("You are not strong enough to choke $N.",
				FALSE, ch, 0, ch->subdue, TO_CHAR);
		act ("$N applies pressure to your neck, but doesn't hurt you.",
				FALSE, ch->subdue, 0, ch, TO_CHAR);
		return;
	}

	act ("You choke $N.", FALSE, ch, 0, ch->subdue, TO_CHAR);
	act ("$N chokes you!", TRUE, ch->subdue, 0, ch, TO_CHAR);
	act ("$n chokes $N.", FALSE, ch, 0, ch->subdue, TO_NOTVICT);

	if ( wound_to_char (ch, "neck", 2 * pressure, 9, 0, 0, 0) ) {
		act ("$n crumples to the ground dead!",
			 FALSE, ch->subdue, 0, ch, TO_CHAR);
		act ("$n chokes $N to death!", FALSE, ch, 0, ch->subdue, TO_NOTVICT);
	}
}

void fix_offense (CHAR_DATA *ch)
{
	int		i;
	int		best_skill = SKILL_BRAWLING;

	for ( i = SKILL_BRAWLING; i <= SKILL_CROSSBOW; i++ )
		if ( ch->skills [i] > ch->skills [best_skill] )
			best_skill = i;

	ch->offense = ch->skills [best_skill] / 2;
}

void do_compete (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*src;
	CHAR_DATA	*tar;
	int			iterations = 100;
	char		*temp_arg = NULL;


	argument = one_argument (argument, buf);

	if ( str_cmp (ch->tname, IMPLEMENTOR_ACCOUNT) ) {
		send_to_char ("This command is for the implementor only.\n", ch);
		return;
	}

	if ( !(src = get_char_room_vis (ch, buf)) ) {
		send_to_char ("First combatant isn't here.\n\r", ch);
		free(argument);
		return;
	}
	if(temp_arg)
		free(temp_arg);
	temp_arg = argument;
	argument = one_argument (temp_arg, buf);
	
	
	if ( !(tar = get_char_room_vis (ch, buf)) ) {
		send_to_char ("Second combatant isn't here.\n\r", ch);
		return;
	}

	disable_timer_abort = TRUE;
	
	argument = one_argument (argument, buf);
	
	if ( strtol(buf, NULL, 10))
		iterations = strtol(buf, NULL, 10);

	compete (src, tar, iterations);

	disable_timer_abort = FALSE;
	
	return;
}

struct stats_data {
	int		kills;
	int		hits_given;
	int		hits_given_2;
	int		swipes;
	int		swipes_2;
	int		smites;
	int		smites_2;
	int		breaks;
} src_stats, tar_stats;

int compete_smite (CHAR_DATA *src, CHAR_DATA *tar, struct stats_data *stats)
{
	int		old_hits;
	int		killed = 0;

	old_hits = GET_DAM (tar);

	if ( !src->primary_delay &&
		 (get_equip (src, WEAR_PRIM) ||
		  get_equip (src, WEAR_BOTH) ||
		  (!get_equip (src, WEAR_PRIM) &&
		   !get_equip (src, WEAR_BOTH) &&
		   !get_equip (src, WEAR_SEC))) ) {
		stats->swipes++;
		killed = smite (src, tar, 1);
	}

	if ( old_hits != GET_DAM (tar) ) {
		stats->smites++;
		stats->hits_given += GET_DAM (tar) - old_hits;
	}

	if ( killed ) {
		stats->kills++;
		heal_all_wounds (src);
		heal_all_wounds (tar);
		return 1;
	}

	old_hits = GET_DAM (tar);

	if ( !src->secondary_delay &&
		 get_equip (src, WEAR_SEC) ) {
		stats->swipes_2++;
		killed = smite (src, tar, 2);
	}

	if ( old_hits != GET_DAM (tar) ) {
		stats->smites_2++;
		stats->hits_given_2 += GET_DAM (tar) - old_hits;
	}

	if ( killed ) {
		stats->kills++;
		heal_all_wounds (src);
		heal_all_wounds (tar);
		return 1;
	}

	return 0;
}

/** Debug function to test weapons and defense values in close to real-game conditions ***/
void compete (CHAR_DATA *src, CHAR_DATA *tar, int iterations)
{
	int			tick = 0;
	int			i;
	OBJ_DATA	*src_prim		= NULL;
	OBJ_DATA	*tar_prim		= NULL;
	OBJ_DATA	*src_sec		= NULL;
	OBJ_DATA	*tar_sec		= NULL;
	OBJ_DATA	*src_both		= NULL;
	OBJ_DATA	*tar_both		= NULL;
	OBJ_DATA	*src_shield		= NULL;
	OBJ_DATA	*tar_shield		= NULL;
	OBJ_DATA	*obj, *next_obj;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		name [MAX_STRING_LENGTH] = {'\0'};


	if ( get_equip (src, WEAR_PRIM) )
		src_prim = vtoo (get_equip (src, WEAR_PRIM)->virtual);

	if ( get_equip (tar, WEAR_PRIM) )
		tar_prim = vtoo (get_equip (tar, WEAR_PRIM)->virtual);

	if ( get_equip (src, WEAR_SEC) )
		src_sec = vtoo (get_equip (src, WEAR_SEC)->virtual);

	if ( get_equip (tar, WEAR_SEC) )
		tar_sec = vtoo (get_equip (tar, WEAR_SEC)->virtual);

	if ( get_equip (src, WEAR_BOTH) )
		src_both = vtoo (get_equip (src, WEAR_BOTH)->virtual);

	if ( get_equip (tar, WEAR_BOTH) )
		tar_both = vtoo (get_equip (tar, WEAR_BOTH)->virtual);

	if ( get_equip (src, WEAR_SHIELD) )
		src_shield = vtoo (get_equip (src, WEAR_SHIELD)->virtual);

	if ( get_equip (tar, WEAR_SHIELD) )
		tar_shield = vtoo (get_equip (tar, WEAR_SHIELD)->virtual);

	SET_BIT (src->flags, FLAG_COMPETE);
	SET_BIT (tar->flags, FLAG_COMPETE);

	src->primary_delay = 0;
	src->secondary_delay = 0;

	src_stats.kills = 0;
	src_stats.hits_given = 0;
	src_stats.hits_given_2 = 0;
	src_stats.swipes = 0;
	src_stats.swipes_2 = 0;
	src_stats.smites = 0;
	src_stats.smites_2 = 0;
	src_stats.breaks = 0;

	tar->primary_delay = 0;
	tar->secondary_delay = 0;

	tar_stats.kills = 0;
	tar_stats.hits_given = 0;
	tar_stats.hits_given_2 = 0;
	tar_stats.swipes = 0;
	tar_stats.swipes_2 = 0;
	tar_stats.smites = 0;
	tar_stats.smites_2 = 0;
	tar_stats.breaks = 0;

	GET_MOVE (src) = GET_MAX_MOVE (src);
	GET_MOVE (tar) = GET_MAX_MOVE (tar);

	GET_HIT (src) = GET_MAX_HIT (src);
	GET_HIT (tar) = GET_MAX_HIT (tar);

	REMOVE_BIT (src->flags, FLAG_KILL);
	REMOVE_BIT (tar->flags, FLAG_KILL);

	for ( i = 0; i < iterations; i++ ) {

		while ( 1 ) {

			if ( !(tick % 4) )
				second_affect_update ();
			
			if ( tick & 1 ) {
				if ( compete_smite (src, tar, &src_stats) )
					break;

				if ( compete_smite (tar, src, &tar_stats) )
					break;
			} else {
				if ( compete_smite (tar, src, &tar_stats) )
					break;

				if ( compete_smite (src, tar, &src_stats) )
					break;
			}

			if ( src->primary_delay )
				src->primary_delay--;

			if ( src->secondary_delay )
				src->secondary_delay--;

			if ( tar->primary_delay )
				tar->primary_delay--;

			if ( tar->secondary_delay )
				tar->secondary_delay--;

			tick++;
		}

		GET_MOVE (src) = GET_MAX_MOVE (src);
		GET_MOVE (tar) = GET_MAX_MOVE (tar);

		second_affect_update ();
		second_affect_update ();
		second_affect_update ();
		second_affect_update ();
		second_affect_update ();
		second_affect_update ();
		second_affect_update ();
		second_affect_update ();
		second_affect_update ();
		second_affect_update ();

		if ( src_prim && !get_equip (src, WEAR_PRIM) ) {
			src_stats.breaks++;
			obj = load_object (src_prim->virtual);
			obj_to_char (obj, src);
			equip_char (src, obj, WEAR_PRIM);
		}

		if ( tar_prim && !get_equip (tar, WEAR_PRIM) ) {
			tar_stats.breaks++;
			obj = load_object (tar_prim->virtual);
			obj_to_char (obj, tar);
			equip_char (tar, obj, WEAR_PRIM);
		}

		if ( src_sec && !get_equip (src, WEAR_SEC) ) {
			src_stats.breaks++;
			obj = load_object (src_sec->virtual);
			obj_to_char (obj, src);
			equip_char (src, obj, WEAR_SEC);
		}

		if ( tar_sec && !get_equip (tar, WEAR_SEC) ) {
			tar_stats.breaks++;
			obj = load_object (tar_sec->virtual);
			obj_to_char (obj, tar);
			equip_char (tar, obj, WEAR_SEC);
		}

		if ( src_both && !get_equip (src, WEAR_BOTH) ) {
			src_stats.breaks++;
			obj = load_object (src_both->virtual);
			obj_to_char (obj, src);
			equip_char (src, obj, WEAR_BOTH);
		}

		if ( tar_both && !get_equip (tar, WEAR_BOTH) ) {
			tar_stats.breaks++;
			obj = load_object (tar_both->virtual);
			obj_to_char (obj, tar);
			equip_char (tar, obj, WEAR_BOTH);
		}

		if ( src_shield && !get_equip (src, WEAR_SHIELD) ) {
			src_stats.breaks++;
			obj = load_object (src_shield->virtual);
			obj_to_char (obj, src);
		}

		if ( tar_shield && !get_equip (tar, WEAR_SHIELD) ) {
			tar_stats.breaks++;
			obj = load_object (tar_shield->virtual);
			obj_to_char (obj, src);
		}

		tics++;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "Name              Kills    DamP    DamS   StrkP   StrkS   MissP   MissS   Breaks\n\r");
	send_to_room_unf (buf, src->in_room);
	snprintf (buf, MAX_STRING_LENGTH,  "===============   ======   =====   =====  ======  ======  ======  ======  ======\n\r");
	send_to_room_unf (buf, src->in_room);

	strncpy (name, src->name, 15);

	name [15] = '\0';

	while ( strlen (name) < 15 )
		strcat (name, " ");

	snprintf (buf, MAX_STRING_LENGTH,  "%15s   %-6d   %-5d   %-5d  %-6d  %-6d  %-6d  %-6d  %-6d\n\r",
		name,
		src_stats.kills,
		tar_stats.hits_given,
		tar_stats.hits_given_2,
		src_stats.smites,
		src_stats.smites_2,
		src_stats.swipes,
		src_stats.swipes_2,
		src_stats.breaks);
	send_to_room_unf (buf, src->in_room);

	strncpy (name, tar->name, 15);

	name [15] = '\0';

	while ( strlen (name) < 15 )
		strcat (name, " ");

	snprintf (buf, MAX_STRING_LENGTH,  "%15s   %-6d   %-5d   %-5d  %-6d  %-6d  %-6d  %-6d  %-6d\n\r",
		name,
		tar_stats.kills,
		src_stats.hits_given,
		src_stats.hits_given_2,
		tar_stats.smites,
		tar_stats.smites_2,
		tar_stats.swipes,
		tar_stats.swipes_2,
		tar_stats.breaks);
	send_to_room_unf (buf, src->in_room);

	REMOVE_BIT (src->flags, FLAG_COMPETE);
	REMOVE_BIT (tar->flags, FLAG_COMPETE);

	for ( obj = src->room->contents; obj; obj = next_obj ) {
		next_obj = obj->next_content;
		extract_obj (obj);
	}
}

void sa_rescue (SECOND_AFFECT *sa)
{
	int			result;
	CHAR_DATA	*tch,*rescuee;

	if ( !is_he_somewhere (sa->ch) )
		return;

	rescuee = (CHAR_DATA *) sa->obj;
	result = rescue_attempt (sa->ch, rescuee);
	

	if ( result == 2 )		/* can't rescue...stop trying */
		return;

	else if ( result == 0 ) {	/* Failed, try again */

		act ("$n makes another failed attempt at rescuing $N.",
				FALSE, sa->ch, 0, rescuee, TO_NOTVICT);
		act ("$N tries again, but fails to rescue you.",
				FALSE, rescuee, 0, sa->ch, TO_CHAR);
		act ("You try again, but fail to rescue $n.",
				FALSE, rescuee, 0, sa->ch, TO_VICT);

		add_second_affect (SPA_RESCUE, 3, sa->ch, sa->obj, NULL, 0);

		return;
	}

	else if ( result == 1 )		/* Couldn't try...try asap */
		add_second_affect (SPA_RESCUE, 1, sa->ch, sa->obj, NULL, 0);

	else if ( result == 3 ) {

		for ( tch = sa->ch->room->people; tch; tch = tch->next_in_room )
			if ( tch->fighting == rescuee )
				break;

		if ( !tch )
			return;

		if ( !sa->ch->fighting )
			set_fighting (sa->ch, tch);
		else
			sa->ch->fighting = tch;

		if ( tch->fighting ) {
			stop_fighting (tch);
			if (rescuee->fighting)
				stop_fighting (rescuee);
		}
		set_fighting (tch, sa->ch);

		act ("You draw $N's attention.", FALSE, sa->ch, 0, tch, TO_CHAR);
		act ("$N draws your attention.", FALSE, tch, 0, sa->ch, TO_CHAR);
		act ("$N draws $n's attention.", FALSE, tch, 0, sa->ch, TO_NOTVICT);
		act ("You stop fighting $N.", FALSE, rescuee, 0, tch, TO_CHAR);

	}
}

	/* Rescue results:
		0 = failed to rescue (3 sec pause)
			 Normal failure...try again
		1 = no rescue attempt, not possible yet (1 sec pause)
             Not in FIGHT or STAND mode
             Being subdued
		2 = can't rescue (delete sa)
			 Fighting friend, or friend fighting us
             Friend not here
             Friend begin fought
		3 = success
	*/

int rescue_attempt (CHAR_DATA *ch, CHAR_DATA *friend)
{
	CHAR_DATA		*tch;
	int				agi_diff;

	if ( GET_POS (ch) < FIGHT )
		return 1;

	if ( IS_SET (ch->flags, FLAG_SUBDUING) )
		return 1;

	if ( !is_he_here (ch, friend, TRUE) )
		return 2;

	if ( IS_SET (friend->flags, FLAG_SUBDUING) )
		return 1;

	if ( ch->fighting == friend || friend->fighting == ch )
		return 2;

	for ( tch = ch->room->people; tch; tch = tch->next_in_room )
		if ( tch->fighting == friend )
			break;

	if ( !tch )
		return 2;

	agi_diff = (GET_AGI (ch) - GET_AGI (tch)) + 
		
		/*
		 * easier to rescue if you are fighting your friend's enemy
		 * harder to rescue if you are fighting someone else
		 * otherwise rescue as normal
		*/
		
		(ch->fighting == tch)?8:(ch->fighting)?2:5;
	
	if ( agi_diff >= number (1, 10) )
		return 3;
	else if ( number (0, 19) == 0 )
		return 3;
	else
		return 0;
}

void do_rescue (CHAR_DATA *ch, char *argument, int cmd)
{
	int			result;
	CHAR_DATA		*friend;
	CHAR_DATA		*tch;
	SECOND_AFFECT	*sa;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	sa = get_second_affect (ch, SPA_RESCUE, NULL);

	if ( !*buf ) {

		if ( sa ) {
			send_to_char ("You stop trying to rescue.\n", ch);
			remove_second_affect (sa);
			return;
		}

		send_to_char ("Rescue whom?\n", ch);
		return;
	}

	if ( !(friend = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see them here.\n", ch);
		return;
	}

	if ( friend == ch ) {
		send_to_char ("Rescue yourself?\n", ch);
		return;
	}

	if ( friend == ch->fighting ) {
		send_to_char ("You can't rescue your opponent?\n", ch);
		return;
	}

	for ( tch = ch->room->people; tch; tch = tch->next_in_room )
		if ( tch->fighting == friend )
			break;

	if ( !tch ) {
		act ("$N doesn't need rescuing.", FALSE, ch, 0, friend, TO_CHAR);
		return;
	}

	if ( sa ) {

		if ( (CHAR_DATA *) sa->obj == friend ) {
			act ("You're still trying your best to rescue $N.",
					FALSE, ch, 0, friend, TO_CHAR);
			return;
		}

		sa->obj = (OBJ_DATA *) friend;
		act ("You will try to rescue $N now.", FALSE, ch, 0, friend, TO_CHAR);
		return;
	}

	result = rescue_attempt (ch, friend);

	if ( result == 0 ) {
		act ("You try to draw $N's attention.", FALSE, ch, 0, tch, TO_CHAR);
		act ("$N tries to draw your attention.", FALSE, tch, 0, ch, TO_CHAR);
		act ("$n tries to draw $N's attention.", FALSE, ch, 0, tch, TO_NOTVICT);

		add_second_affect (SPA_RESCUE, 3, ch, (OBJ_DATA *) friend, NULL, 0);
	}

	else if ( result == 3 ) {
		act ("You draw $N's attention.", FALSE, ch, 0, tch, TO_CHAR);
		act ("$N draws your attention.", FALSE, tch, 0, ch, TO_CHAR);
		act ("$N draws $n's attention.", FALSE, tch, 0, ch, TO_NOTVICT);

		if ( GET_POS (ch) != POSITION_DEAD && GET_POS (tch) != POSITION_DEAD )
			criminalize (ch, tch, ch->room->zone, CRIME_KILL);

		if ( !tch->fighting )
			set_fighting (tch, ch);
		else
			tch->fighting = ch;
	}

	else if ( result == 1 ) {
		act ("You will try to rescue $N when you can.",
				FALSE, ch, 0, friend, TO_CHAR);

		add_second_affect (SPA_RESCUE, 1, ch, (OBJ_DATA *) friend, NULL, 0);
	}

	else {		/* better be result == 2, shouldn't be possible */
		printf ("Rescue attempt, result = 2\n");
	}
	return;
}

void do_surrender (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*tch;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

    if ( IS_SWIMMING (ch) ) {
    	send_to_char ("You can't do that while swimming!\n", ch);
        return;
	}

	if ( IS_SET (ch->flags, FLAG_SUBDUER) ) {
		send_to_char ("Release your prisoner, first.\n", ch);
		return;
	}

	if ( IS_SET (ch->room->room_flags, OOC) ) {
		send_to_char ("Sorry, but this command is disabled in OOC areas.\n", ch);
		return;
	}

	if ( !*buf && ch->fighting )
		tch = ch->fighting;
	else if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("Surrender to whom?\n", ch);
		return;
	}

	if ( ch == tch ) {
		send_to_char ("Surrender to yourself? Hmm...\n", ch);
		return;
	}

	if ( tch->fighting && tch->fighting != ch ) {
		send_to_char ("They're a little too busy to take prisoners, right now...\n", ch);
		return;
	}

	if ( tch->subdue ) {
		send_to_char ("They are unable to take another prisoner.\n", ch);
		return;
	}

	if ( IS_SET (tch->flags, FLAG_KILL) ) {
		send_to_char ("They don't look particularly interested in taking prisoners...\n", ch);
		return;
	}

	act ("You surrender yourself into $N's custody.", FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
	act ("$n surrenders $mself into your custody.", TRUE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
	act ("$n surrenders $mself into $N's custody.", TRUE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);

	subdue_char (tch, ch);
	return;
}

void do_study (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*crim;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( IS_SET (ch->room->room_flags, OOC) ) {
		send_to_char ("That command is disabled in OOC areas.\n", ch);
		return;
	}

	if ( !*buf ) {
		send_to_char ("Study is used to identify those who are masked.\n", ch);
		return;
	}

	if ( !(crim = get_char_room_vis (ch, buf)) ) {
		send_to_char ("Study whom?\n", ch);
		return;
	}

	if ( crim == ch ) {
		send_to_char ("You like the way you look.\n", ch);
		return;
	}

	do_look (ch, buf, 0);

	act ("You stare at $N.", FALSE, ch, 0, crim, TO_CHAR);
	act ("$n is staring at $N.", FALSE, ch, 0, crim, TO_NOTVICT);
	act ("$n stares at you.", FALSE, ch, 0, crim, TO_VICT);

	if ( !is_hooded (crim) )
		return;
	
	ch->delay_ch = crim;
	ch->delay_type = DEL_STARE;
	ch->delay = 5;
	return;
}

void delayed_study (CHAR_DATA *ch)
{
	AFFECTED_TYPE	*af = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !is_he_here (ch, ch->delay_ch, TRUE) )
		return;

	if ( !is_hooded (ch->delay_ch) )
		return;

	if ( !skill_use (ch, SKILL_SCAN, 0) ) {
		act ("The identity of $N remains a mystery.",
				FALSE, ch, 0, ch->delay_ch, TO_CHAR);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "You discover that $N is %s.", ch->delay_ch->short_descr);
	act (buf, FALSE, ch, 0, ch->delay_ch, TO_CHAR);

	if ( ch->mob && IS_SET (ch->act, ACT_ENFORCER) ) {
		magic_add_affect (ch->delay_ch, MAGIC_STARED, 90, 0, 0, 0, 0);
	        if ( (is_area_enforcer(ch) && (ch->race == 0) )
       	      		&& (ch->in_room / 1000 == 1 || ch->in_room / 1000 == 2 || ch->in_room / 1000 == 3) ) {
                	if ( ch->delay_ch->race >= 1 && CAN_SEE (ch, ch->delay_ch) )  {	/* Beasts in Gondor? Uh oh... */
				criminalize (ch->delay_ch, ch, ch->room->zone, 6);
                	}
        	}
	}

	if ( !get_affect (ch->delay_ch, MAGIC_CRIM_BASE + ch->room->zone) )
		return;

	if ( is_area_enforcer (ch) )
		act ("$E is a criminal!", FALSE, ch, 0, ch->delay_ch, TO_CHAR);

		/* Make criminal hot */

	if ( !(af = get_affect (ch->delay_ch, MAGIC_CRIM_HOODED)) )
		magic_add_affect (ch->delay_ch, MAGIC_CRIM_HOODED + ch->room->zone,
						  400, 0, 0, 0, 0);
	else
		af->a.spell.duration = 400;
		
		/* Probably should check to see if this is an enforcer of the zone */

	if ( ch->mob && is_area_enforcer(ch) )
		enforcer (ch, ch->delay_ch, 1, 1);
}
