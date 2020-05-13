/** 
*	\file handler.c
*	Mainupulates objects and characters
*
*	This module deals with effects in room or on objects, transfer of objects,
*	transfers of characters, sets up and reads registries, and updates delays.
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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

extern NAME_SWITCH_DATA	*clan_name_switch_list;

int is_hooded (CHAR_DATA *ch)
{
	OBJ_DATA		*obj;

	if ( (obj = get_equip (ch, WEAR_ABOUT)) &&
		 IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		 IS_SET (ch->affected_by, AFF_HOODED) )
		return 1;

	if ( ((obj = get_equip (ch, WEAR_HEAD)) || (obj = get_equip (ch, WEAR_FACE))) &&
		 IS_SET(obj->obj_flags.extra_flags, ITEM_MASK) ) {

		if ( obj->obj_flags.type_flag == ITEM_WORN ||
			 obj->obj_flags.type_flag == ITEM_ARMOR )
			return 1;
	}

	return 0;
}

char *char_names (CHAR_DATA *ch)
{
	OBJ_DATA *obj = NULL;
	
	if ( (obj = get_equip (ch, WEAR_ABOUT)) &&
		 IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		 IS_SET (ch->affected_by, AFF_HOODED) )
		return obj->desc_keys;
	
	if ( ((obj = get_equip (ch, WEAR_HEAD)) || (obj = get_equip (ch, WEAR_FACE))) &&
		 IS_SET(obj->obj_flags.extra_flags, ITEM_MASK)) {
			
		if ( obj->obj_flags.type_flag == ITEM_WORN ) {
			if ( !obj->desc_keys )
				return "obscured";

			return obj->desc_keys;
		}
			
		if ( obj->obj_flags.type_flag == ITEM_ARMOR ) {
			if ( !obj->desc_keys )
				return "obscured";

			return obj->desc_keys;
		}
	}

	return ch->name;
}	


char *fname (char *namelist)
{
	static char holder[30];
	char		*point = NULL;

	if ( !namelist )
		return "";

	for (point = holder; isalpha(*namelist); namelist++, point++)
		*point = *namelist;

	*point = '\0';

	return(holder);
}

char *fname_hyphen (char *namelist)
{
	static char holder [30];
	char		*point = NULL;

	if ( !namelist )
		return "obscured";

	for ( point = holder;
		  isalpha (*namelist) || *namelist == '-' || *namelist == ' ';
		  namelist++, point++ )
		*point = *namelist;

	*point = '\0';

	return holder;
}

int name_is (char *str, char *namelist)
{
	char	*curname = NULL;
	char	*curstr = NULL;

    if ( !str )
		return 0;

	if ( !namelist )
		return 0;

	curname = namelist;
	for (;;)
	{
		for (curstr = str;; curstr++, curname++)
		{
			if ((!*curstr && !isalpha(*curname)) || is_abbrev(curstr,curname))
				return(1);

			if (!*curname)
				return(0);

			if (!*curstr || *curname == ' ')
				break;

			if (LOWER(*curstr) != LOWER(*curname))
				break;
		}

		/* skip to next name */

		for (; isalpha(*curname); curname++);
		if (!*curname)
			return(0);
		curname++;			/* first char of new name */
	}
	return(0);
}

int real_skill (CHAR_DATA *ch, int skill)
{
	if ( !IS_NPC (ch) )
		return ch->skills [skill];
	else
		return vtom (ch->mob->virtual)->skills [skill];
}

/** sn is the spell number. not used right now, but it will be in the future **/
void affect_modify (CHAR_DATA *ch, int type, int loc, int mod, int bitv,
					int add, int sn)
{
	if ( type >= JOB_1 && type <= JOB_3 )
		return;

	if ( type >= CRAFT_FIRST && type <= CRAFT_LAST )
		return;

	if ( type >= MAGIC_SMELL_FIRST && type <= MAGIC_SMELL_LAST )
		return;

	if ( type >= MAGIC_FIRST_SOMA && type <= MAGIC_LAST_SOMA )
		return;

	if ( type == MAGIC_GUARD )
		return;

	if ( type == AFFECT_SHADOW )
		return;

	if (type == MUTE_EAVESDROP )
		return;

	if ( add && bitv )
		SET_BIT (ch->affected_by, bitv);

	else if ( bitv ) {
		REMOVE_BIT (ch->affected_by, bitv);
		mod = -mod;
	}

/*
	switch (type) {
		case SPELL_STRENGTH:
			GET_STR (ch) += mod;
			return;
		
		case SPELL_DEXTERITY:
			GET_DEX (ch) += mod;
			return;
		
		case SPELL_INTELLIGENCE:
			GET_INT (ch) += mod;
			return;
		
		case SPELL_AURA:
			GET_AUR (ch) += mod;
			return;
		
		case SPELL_WILL:
			GET_WIL (ch) += mod;
			return;
		
		case SPELL_CONSTITUTION:
			GET_CON (ch) += mod;
			return;
		
		case SPELL_AGILITY:
			GET_AGI (ch) += mod;
			return;
		
		default:
			break;
	}
*/

	if ( loc >= (SKILL_BRAWLING+10000) && loc <= (LAST_SKILL+10000) ) {
		if ( add )
			ch->skills [loc-10000] += mod;
		else {
			if ( ch->pc && ch->pc->skills [loc-10000] < ch->skills [loc-10000] )
				ch->skills [loc-10000] -= mod;
			else if ( !ch->pc )
				ch->skills [loc-10000] -= mod;
		}
		return;
	}

	else switch (loc) {

		case APPLY_NONE:
		case APPLY_CASH:
		case APPLY_SAVING_BREATH:
		case APPLY_SAVING_SPELL:
		case APPLY_AC:
			break;

		case APPLY_STR:
			GET_STR (ch) += mod;
			break;
		
		case APPLY_DEX:
			GET_DEX (ch) += mod;
			break;
		
		case APPLY_INT:
			GET_INT (ch) += mod;
			break;
		
		case APPLY_AUR:
			GET_AUR (ch) += mod;
			break;
		
		case APPLY_WIL:
			GET_WIL (ch) += mod;
			ch->max_move = calc_lookup (ch, REG_MISC, MISC_MAX_MOVE);
			break;
		
		case APPLY_CON:
			GET_CON (ch) += mod;
			if ( !IS_NPC (ch) ){
				ch->max_hit  = 10 + 6 * GET_CON (ch);
			}
			else {
				ch->max_hit += mod * 6;
				if ( GET_HIT (ch) > ch->max_hit ){
					GET_HIT (ch) = ch->max_hit;
				}
			}
			ch->max_move = calc_lookup (ch, REG_MISC, MISC_MAX_MOVE);
			break;
		
		case APPLY_AGI:
			GET_AGI (ch) += mod;
			break;
		
		case APPLY_AGE:
			ch->time.birth += mod;
			break;
		
		case APPLY_HIT:
			ch->max_hit += mod;	
			break;
		
		case APPLY_MOVE:
			ch->max_move += mod;
			break;
		
		case APPLY_DAMROLL:
			if ( IS_NPC (ch) ){
				GET_DAMROLL (ch) += mod;
			}
			break;
		
		default:
			break;

	} /* switch */
	
	return;
}

void nullify_affects (CHAR_DATA *ch)
{
	AFFECTED_TYPE	*af = NULL;
	OBJ_DATA		*eq = NULL;
	int				i = 0;

			/* Remove affects caused by equipment */

	for ( i = 0; i < MAX_WEAR; i++ ) {

		if ( !(eq = get_equip (ch, i)) ||
			 i == WEAR_BELT_1 ||
			 i == WEAR_BELT_2 ||
			 i == WEAR_BACK )
			continue;

		for ( af = eq->xaffected; af; af = af->next ) {
			if ( (af->a.spell.location != APPLY_OFFENSE &&
				  af->a.spell.location != APPLY_DEFENSE) )
				affect_modify (ch, af->type, af->a.spell.location,
				               af->a.spell.modifier,
				               eq->obj_flags.bitvector, FALSE, 0);
		}
	}


			/* Remove affects caused by spells */

	for (af = ch->hour_affects; af; af = af->next )
		affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier,
					   af->a.spell.bitvector, FALSE, af->a.spell.sn);


			/* Restore base capabilities */

	ch->tmp_str		= ch->str;
	ch->tmp_dex		= ch->dex;
	ch->tmp_intel	= ch->intel;
	ch->tmp_aur		= ch->aur;
	ch->tmp_con		= ch->con;
	ch->tmp_wil		= ch->wil;
	
	return;
}

void reapply_affects (CHAR_DATA *ch)
{
	AFFECTED_TYPE	*af = NULL;
	OBJ_DATA		*eq = NULL;
	int				i = 0;

			/* Add equipment affects back */

	for ( i = 0; i < MAX_WEAR; i++ ) {

		if ( !(eq = get_equip (ch, i)) ||
			 i == WEAR_BELT_1 ||
			 i == WEAR_BELT_2 ||
			 i == WEAR_BACK )
			continue;

		for ( af = eq->xaffected; af; af = af->next )
			if ( af->a.spell.location != APPLY_OFFENSE &&
				  af->a.spell.location != APPLY_DEFENSE )
				affect_modify (ch, af->type, af->a.spell.location,
				               af->a.spell.modifier,
				               eq->obj_flags.bitvector, TRUE, 0);
	}

			/* Add spell affects back */

	for ( af = ch->hour_affects; af; af = af->next )
		affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier,
					   af->a.spell.bitvector, TRUE, af->a.spell.sn);

	/* Make certain values are between 0..25, not < 0 and not > 25! */

	GET_DEX (ch) = MAX (0, MIN (GET_DEX (ch), 25));
	GET_INT (ch) = MAX (0, MIN (GET_INT (ch), 25));
	GET_WIL (ch) = MAX (0, MIN (GET_WIL (ch), 25));
	GET_AUR (ch) = MAX (0, MIN (GET_AUR (ch), 25));
	GET_CON (ch) = MAX (0, MIN (GET_CON (ch), 25));
	GET_STR (ch) = MAX (0, MIN (GET_STR (ch), 25));
	
	return;
}

AFFECTED_TYPE *get_obj_affect_location (OBJ_DATA *obj, int location)
{
	AFFECTED_TYPE	*af = NULL;

	if ( !obj->xaffected )
		return NULL;

	for ( af = obj->xaffected; af; af = af->next )
		if ( af->a.spell.location == location )
			return af;

	return NULL;
}

void remove_obj_affect_location (OBJ_DATA *obj, int location)
{
	AFFECTED_TYPE	*af = NULL;
	AFFECTED_TYPE	*taf = NULL;

	if ( !obj->xaffected )
		return;

	if ( obj->xaffected->a.spell.location == location ) {
		af = obj->xaffected;
		obj->xaffected = obj->xaffected->next;
		mem_free (af);
		return;
	}

	for ( af = obj->xaffected; af->next; af = af->next ){
		if ( af->next->a.spell.location == location ) {
			taf = af->next;
			af->next = taf->next;
			mem_free (taf);
			return;
		}
	}
	
	return;
}

void affect_to_obj (OBJ_DATA *obj, AFFECTED_TYPE *af)
{
	AFFECTED_TYPE	*taf = NULL;

	if ( !obj->xaffected ) {
		obj->xaffected = af;
		return;
	}

	for ( taf = obj->xaffected; taf->next; taf = taf->next )
		; /* cycles through taf */

	taf->next = af;
	
	return;
}

void remove_obj_affect (OBJ_DATA *obj, int type)
{
	AFFECTED_TYPE	*af = NULL;
	AFFECTED_TYPE	*free_af = NULL;

	if ( !obj->xaffected )
		return;

	if ( obj->xaffected->type == type ) {
		af = obj->xaffected;
		obj->xaffected = af->next;
		mem_free (af);
		return;
	}

	for ( af = obj->xaffected; af->next; af = af->next ) {
		if ( af->next->type == type ) {
			free_af = af->next;
			af->next = free_af->next;
			mem_free (free_af);
			return;
		}
	}
	return;
}

AFFECTED_TYPE *get_obj_affect (OBJ_DATA *obj, int type)
{
	AFFECTED_TYPE		*af = NULL;

	if ( !obj->xaffected )
		return NULL;

	for ( af = obj->xaffected; af; af = af->next )
		if ( af->type == type )
			return af;

	return NULL;
}

AFFECTED_TYPE *get_affect (CHAR_DATA *ch, int affect_type)
{
	AFFECTED_TYPE 		*af = NULL;

	for ( af = ch->hour_affects;
		  af && af->type != affect_type;
		  af = af->next);

	return af;
}


/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's.
*/

void affect_to_char (CHAR_DATA *ch, AFFECTED_TYPE *af)
{
	af->next = ch->hour_affects;
	ch->hour_affects = af;

	affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier,
					   af->a.spell.bitvector, TRUE, af->a.spell.sn);
					   
	return;
}

void remove_affect_type (CHAR_DATA *ch, int type)
{
	AFFECTED_TYPE	*af = NULL;

	if ( (af = get_affect (ch, type)) )
		affect_remove (ch, af);
		
	return;
}

/* Remove an affected_type structure from a char (called when duration
   reaches zero).  Pointer *af must never be NIL!  Frees mem and calls 
   affect_location_apply.
*/

void affect_remove (CHAR_DATA *ch, AFFECTED_TYPE *af)
{
	AFFECTED_TYPE	*taf = NULL;

	affect_modify (ch, af->type, af->a.spell.location, af->a.spell.modifier,
					   af->a.spell.bitvector, FALSE, af->a.spell.sn);

	/* remove structure *af from linked list */

	if (ch->hour_affects == af)
		ch->hour_affects = af->next;

	else {
		for (taf = ch->hour_affects; taf && taf->next != af; taf = taf->next )
			;

		if ( !taf ) {
			return;
		}

		taf->next = af->next;
	}

	if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST )
		mem_free (af->a.craft);

	mem_free (af);
	
	return;
}

void char_from_room (CHAR_DATA *ch)
{
	CHAR_DATA	*i = NULL;
	ROOM_DATA	*room = NULL;
	AFFECTED_TYPE	*af = NULL;

	if (ch->in_room == NOWHERE) {
		system_log("NOWHERE extracting char from room (handler.c, char_from_room)", TRUE);
		abort();
	}

	if ( ch->room == NULL )
		return;

	room = ch->room;

	if ( room && room->sector_type == SECT_LEANTO && IS_MORTAL(ch))
		room->deity--;

	if ( ch == room->people )  /* head of list */
		 room->people = ch->next_in_room;

	else {   /* locate the previous element */

		for (i = room->people; i; i = i->next_in_room) {
			if ( i->next_in_room == ch ) {
				i->next_in_room = ch->next_in_room;
				break;
			}
		}

	}

	ch->in_room = NOWHERE;
	ch->room = NULL;
	ch->next_in_room = NULL;

	if ( (af = get_affect (ch, MAGIC_SIT_TABLE)) )
		remove_affect_type (ch, MAGIC_SIT_TABLE);

	if ( room )
		room_light (room);
		
	return;
}


/* place a character in a room */
void char_to_room (CHAR_DATA *ch, int room_num)
{
	OBJ_DATA	*obj = NULL;
	ROOM_DATA	*room = NULL;
	ROOM_DATA	*troom = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			curr = 0;
	char		*temp_arg = NULL;

	if ( ch->in_room > 100000 && vtor(ch->was_in_room) ) {
		troom = vtor(ch->was_in_room);
		load_save_room (troom);
		
		for ( obj = troom->contents; obj; obj = obj->next_content ) {
			if ( obj->deleted )
				continue;
	
			if ( obj->o.od.value[0] == ch->in_room ) {
				if ( !(room = vtor (obj->o.od.value[0])) ){
					room = generate_dwelling_room (obj);
				}
				temp_arg = obj_short_desc(obj);
				snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 rustles as its occupants stir.", temp_arg);
				buf[2] = toupper(buf[2]);
				send_to_room (buf, obj->in_room);
				
				REMOVE_BIT (obj->obj_flags.extra_flags, ITEM_VNPC);
				room->occupants++;
				break;
			}
		}
	
		if ( !room ){
			room = vtor(ch->was_in_room);
		}

		if ( !room ){
			room = vtor (0);
		}
	}
	else if ( !(room = vtor (room_num)) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Room %d doesn't exist in char_to_room()! (%s)", room_num, ch->tname);
		system_log (buf, TRUE);
		room = vtor (0);
	}
	
	if ( room->people && room->people != ch )
		ch->next_in_room = room->people;
	else
		ch->next_in_room = NULL;
	
	room->people = ch;
	
	ch->in_room = room->virtual;
	ch->room = room;
	
	if ( !IS_NPC (ch) )
		SET_BIT (room->room_flags, PC_ENTERED);
	
	if ( room->sector_type == SECT_UNDERWATER && !get_affect (ch, AFFECT_HOLDING_BREATH) ) {
		if ( IS_MORTAL(ch) ) {
			act ("You take a deep breath just before plunging into the water. . .", FALSE, ch, 0, 0, TO_CHAR);
			send_to_char ("\n", ch);
			curr = MAX(ch->skills [SKILL_SWIMMING] * number(1,3), 25);
			magic_add_affect (ch, AFFECT_HOLDING_BREATH, curr, 0, 0, 0, curr);
		}
	}
	else if ( room->sector_type != SECT_UNDERWATER && get_affect (ch, AFFECT_HOLDING_BREATH) ) {
		if ( IS_MORTAL(ch) ) {
			send_to_char ("\n", ch);
			act ("You inhale deeply, filling your lungs with much-needed air.", FALSE, ch, 0, 0, TO_CHAR);
			send_to_char ("\n", ch);
		}
		remove_affect_type (ch, AFFECT_HOLDING_BREATH);
	}
	
	room_light (ch->room);
	
	if ( !room->psave_loaded )
		load_save_room (room);
	
	return;
}

void obj_to_char (OBJ_DATA *obj, CHAR_DATA *ch)   /* STACKing */
{
	OBJ_DATA	*tobj = NULL;
	bool		stacked_obj = FALSE;

			/* Do object stacking */

	if ( IS_SET (obj->obj_flags.extra_flags, ITEM_STACK) &&
		 !(GET_ITEM_TYPE (obj) == ITEM_FOOD &&
		   obj->o.food.food_value != obj->o.food.bites) ) {

		if ( ch->right_hand && ch->right_hand->virtual == obj->virtual ) {
			tobj = ch->right_hand;
			obj->count += tobj->count;
			extract_obj (tobj);
			stacked_obj = TRUE;
			ch->right_hand = obj;
			obj->carried_by = ch;
			obj->location = -1;
			obj->in_room = NOWHERE;
			obj->in_obj = NULL;
			return;
		}

		else if ( ch->left_hand && ch->left_hand->virtual == obj->virtual ) {
			tobj = ch->left_hand;
			obj->count += tobj->count;
			extract_obj (tobj);
			stacked_obj = TRUE;
			ch->left_hand = obj;
			obj->carried_by = ch;
			obj->location = -1;
			obj->in_room = NOWHERE;
			obj->in_obj = NULL;
			return;
		}
	}

	if ( ((ch->right_hand && ch->left_hand) && !stacked_obj) || get_equip (ch, WEAR_BOTH) ) {
		send_to_char ("Since your hands are full, you set the object on the ground.\n", ch);
		obj_to_room (obj, ch->in_room);
		obj->in_obj = NULL;
		return;
	}

	if ( !ch->right_hand )
		ch->right_hand = obj;
	else
		ch->left_hand = obj;

	obj->carried_by = ch;

	obj->next_content = NULL;

	obj->location = -1;

	obj->in_room = NOWHERE;

	obj->in_obj = NULL;

	room_light (ch->room);
	
	return;
}

OBJ_DATA *split_obj (OBJ_DATA *obj, int count)
{
	OBJ_DATA	*new_obj = NULL;

	if ( obj->count < 1 || count >= obj->count )
		return obj;

	obj->count -= count;

	new_obj = load_object (obj->virtual);
	new_obj->count = count;

	if ( obj->carried_by ) {

		new_obj->location = -1;

		new_obj->carried_by = obj->carried_by;

		new_obj->in_room = NOWHERE;

		IS_CARRYING_N (obj->carried_by)++;
	}

	else if ( obj->in_obj ) {
		new_obj->next_content = obj->in_obj->contains;
		obj->in_obj->contains = new_obj;
	}

	return new_obj;
}

void obj_from_char (OBJ_DATA **obj, int count)   /* STACKing */
{
	int			fluid = 0;
	int			volume = 0;
	CHAR_DATA	*ch = NULL;

	ch = (*obj)->carried_by;

	if ( ch == NULL ) {
		return;
	}

	if ( GET_ITEM_TYPE (*obj) == ITEM_DRINKCON ) {
		fluid = (*obj)->o.drinkcon.liquid;
		volume = (*obj)->o.drinkcon.volume;
	}

		/* Take a partial number of objs? */

	if ( count != 0 && count < (*obj)->count ) {

		(*obj)->count -= count;

		if ( IS_SET ((*obj)->obj_flags.extra_flags, ITEM_VARIABLE) )
			*obj = load_colored_object ((*obj)->virtual, (*obj)->var_color);
		else *obj = load_object ((*obj)->virtual);

		(*obj)->count = count;

		(*obj)->carried_by = NULL;


		if ( GET_ITEM_TYPE (*obj) == ITEM_DRINKCON ) {
			(*obj)->o.drinkcon.liquid = fluid;
			(*obj)->o.drinkcon.volume = volume;
		}

		return;
	}

	/* Remove object from inventory */

	if ( ch->right_hand == *obj )
		ch->right_hand = NULL;
	else if ( ch->left_hand == *obj )
		ch->left_hand = NULL;

	IS_CARRYING_N (ch)--;

	(*obj)->carried_by = NULL;
	(*obj)->next_content = NULL;
	(*obj)->equiped_by = NULL;
	(*obj)->in_room = NOWHERE;
	(*obj)->in_obj = NULL;

	room_light (ch->room);
	
	return;
}

void equip_char (CHAR_DATA *ch, OBJ_DATA *obj, int pos)
{
	char 		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( ch == 0 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#1OBJECT MORPHING BUG! NULL ch pointer. Crash averted. Object vnum %d.#0\n", obj->virtual);
		send_to_gods (buf);
		system_log(buf, TRUE);
		return;
	}

	if ( pos < 0 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#1OBJECT MORPHING BUG! Crash averted. Position %d, vnum %d, character %s.#0\n", pos, obj->virtual, ch->tname);
		send_to_gods (buf);
		system_log(buf, TRUE);
		return;
	}

	if ( get_equip (ch, pos) ) 
		return;


	if ( obj->in_room != NOWHERE ) {
		system_log("EQUIP: Obj is in_room when equip.", TRUE);
		return;
	}

	obj->location = pos;

	if ( pos != WEAR_PRIM &&
		 pos != WEAR_SEC &&
		 pos != WEAR_BOTH &&
		 pos != WEAR_SHIELD ) {
		obj->next_content = ch->equip;
		ch->equip = obj;
	}

	if ( pos == WEAR_PRIM ||
		 pos == WEAR_SEC ||
		 pos == WEAR_BOTH ||
		 pos == WEAR_SHIELD ){
		obj->carried_by = ch;
	}

	obj->equiped_by = ch;
	
	return;
}

OBJ_DATA *unequip_char (CHAR_DATA *ch, int pos)
{
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*tobj = NULL;

	obj = get_equip (ch, pos);

	if ( !obj )
		return NULL;

	if ( IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) && is_hooded(ch) )
		REMOVE_BIT (ch->affected_by, AFF_HOODED);

	if ( ch->equip == obj )
		ch->equip = ch->equip->next_content;
	else {
		for ( tobj = ch->equip; tobj; tobj = tobj->next_content )
			if ( tobj->next_content && obj == tobj->next_content ) {
				tobj->next_content = obj->next_content;
				break;
			}
	}

	obj->location = -1;
	obj->equiped_by = NULL;
	obj->next_content = NULL;

	if ( ch->aiming_at ) {
		ch->aiming_at->targeted_by = NULL;
		ch->aiming_at = NULL;
		ch->aim = 0;
		ch->delay_info1 = 0;
		ch->delay_who = NULL;
		send_to_char ("You stop aiming your weapon.\n", ch);
	}

	return (obj);
}


int get_number(char **name) {

	int		i = 0;
	char	*ppos = NULL;
	char	number [MAX_INPUT_LENGTH] = { '\0' };

	if ((ppos = (char *) strchr(*name, '.'))) { /* find a decimal point */
		*ppos++ = '\0';			/* move past the decimal */
		strcpy(number,*name);   /* copy all of name to number */
		strcpy(*name, ppos);    /* name now starts past the decimal */

		for(i=0; *(number+i); i++)
			if (!isdigit(*(number+i)))
				return(0);

		return(strtol(number, NULL, 10));
	}

	return(1);
}

OBJ_DATA *get_carried_item (CHAR_DATA *ch, int item_type)
{
	if ( !ch )
		return NULL; 
	
	if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == item_type )
		return ch->right_hand;
	
	if ( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == item_type )    
		return ch->left_hand;
	
	return NULL;
}

/* Search a given list for an object, and return a pointer to that object */
OBJ_DATA *get_obj_in_list(char *name, OBJ_DATA *list)
{
	OBJ_DATA 	*i = NULL;
	int 		j = 0;
	int			number = 0;
	char 		tmpname[MAX_INPUT_LENGTH];
  	char 		*tmp = NULL;

	strcpy(tmpname,name);
	tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(0);
 
	for (i = list, j = 1; i && (j <= number); i = i->next_content) {
		if (name_is(tmp, i->name) || (GET_ITEM_TYPE(i) == ITEM_BOOK && i->book_title && name_is(tmp, i->book_title)) ) {
			if (j == number) 
				return(i);
			j++;
		}
	}

	return(0);
}

OBJ_DATA *get_obj_in_list_num(int num, OBJ_DATA *list)
{
	OBJ_DATA	*obj = NULL;

	for ( obj = list; obj; obj = obj->next_content)
		if ( obj->virtual == num ) 
			return obj;
		
	return NULL;
}

/*search the entire world for an object, and return a pointer  */
OBJ_DATA *get_obj(char *name)
{
	OBJ_DATA 	*i = NULL;
	int 		j = 0;
	int			number = 0;
	char		tmpname[MAX_INPUT_LENGTH];
	char 		*tmp = NULL;

	strcpy(tmpname,name);
	tmp = tmpname;
  if(!(number = get_number(&tmp)))
    return(NULL);

	for (i = object_list, j = 1; i && (j <= number); i = i->next) {

		if ( i->deleted )
			continue;

		if (name_is(tmp, i->name) || (GET_ITEM_TYPE(i) == ITEM_BOOK && i->book_title && name_is(tmp, i->book_title)) ) {
			if (j == number)
				return(i);
			j++;
		}
	}

	return(NULL);
}

CHAR_DATA *get_char_id (int coldload_id)
{
	CHAR_DATA *ch = NULL;

	for ( ch = character_list; ch; ch = ch->next ) {
		if ( ch->deleted )
			continue;
		if ( ch->coldload_id == coldload_id )
			return ch;
	}

	return NULL;
}

OBJ_DATA *get_obj_in_list_id (int coldload_id, OBJ_DATA *list)
{
	OBJ_DATA *obj = NULL;

	for ( obj = list; obj; obj = obj->next_content ) {
		if ( obj->deleted )
			continue;
		if ( obj->coldload_id == coldload_id )
			return obj;
	}

	return NULL;
}

OBJ_DATA *get_obj_id (int coldload_id)
{
	OBJ_DATA *obj = NULL;

	for ( obj = object_list; obj; obj = obj->next ) {
		if ( obj->deleted )
			continue;
		if ( obj->coldload_id == coldload_id )
			return obj;
	}

	return NULL;
}

/* search a room for a char, and return a pointer if found..  */
CHAR_DATA *get_char_room (char *name, int room)
{
	CHAR_DATA 	*i = NULL;
	int 		j = 0;
	int			number = 0;
	char 		tmpname[MAX_INPUT_LENGTH];
	char 		*tmp = NULL;
	char		*temp_arg = NULL;
	
	strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
    return(0);

	for (i = vtor (room)->people,j = 1;
		i && (j <= number);
		i = i->next_in_room){
		
		temp_arg = char_names(i);
		if (name_is(tmp, temp_arg)) {
			if (j == number)
        return(i);
			j++;
		}
	}
	return(0);
}

/* search all over the world for a char, and return a pointer if found */
CHAR_DATA *get_char (char *name)
{
	CHAR_DATA	*i = NULL;
	int			j = 0;
	int			number = 0;
	char		tmpname [MAX_INPUT_LENGTH];
	char		*tmp = NULL;
	char		*temp_arg = NULL;
	
	strcpy (tmpname, name);
	tmp = tmpname;

	if ( !(number = get_number (&tmp)) )
		return NULL;

	for (i = character_list, j = 1; i && (j <= number); i = i->next) {

		if ( i->deleted )
			continue;

		temp_arg = char_names(i);
		if (name_is(tmp, temp_arg)) {
			if (j == number)
				return(i);
			j++;
		}
	}

	return NULL;
}

/* search all over the world for a char, and return a pointer if found */
CHAR_DATA *get_mob_vnum (int virtual)
{
	CHAR_DATA	*i = NULL;

	for (i = character_list; i; i = i->next) {

		if ( i->deleted )
			continue;

		if ( !IS_NPC (i) )
			continue;

		if ( i->mob->virtual == virtual )
			return i;
	}

	return 0;
}

/* search all over the world for a char, and return a pointer if found */
CHAR_DATA *get_char_nomask (char *name)
{
	CHAR_DATA	*i = NULL;
	int			j = 0;
	int			number = 0;
	char		tmpname [MAX_INPUT_LENGTH];
	char		*tmp = NULL;

	strcpy (tmpname, name);
	tmp = tmpname;

	if ( !(number = get_number (&tmp)) )
		return 0;

	for (i = character_list, j = 1; i && (j <= number); i = i->next) {

		if ( i->deleted )
			continue;

		if ( name_is (tmp, GET_NAMES (i)) ) {
			if (j == number)
				return(i);
			j++;
		}
	}

	return 0;
}

void obj_to_room (OBJ_DATA *object, int room)   /* STACKing */
{
	int				add_at_top = 0;
	ROOM_DIRECTION_DATA	*exit_room = NULL;
	ROOM_DATA		*r = NULL;
	ROOM_DATA		*troom = NULL;
	OBJ_DATA		*tobj = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			*temp_arg = NULL;

	if ( !object )
		return;

	r = vtor (room);

	if ( !r )
		return;

	if ( r->contents &&
		 IS_SET (object->obj_flags.extra_flags, ITEM_STACK) ) {

		if ( r->contents->virtual == object->virtual ) {
			r->contents->count += object->count;
			extract_obj (object);
			room_light (r);
			return;
		}

		for ( tobj = r->contents;
			  tobj;
			  tobj = tobj->next_content ) {

			if ( tobj->virtual != object->virtual )
				continue;

			tobj->count += object->count;
			extract_obj (object);
			room_light (r);
			return;
		}
	}

	if ( add_at_top ) {
		object->next_content = r->contents;
		r->contents = object;
	}
	else {
		if ( !r->contents ) {
			r->contents = object;
			object->next_content = NULL;
		}
		else for ( tobj = r->contents; tobj; tobj = tobj->next_content ) {
			if ( !tobj->next_content ) {
				tobj->next_content = object;
				object->next_content = NULL;
				break;
			}
		}
	}

	object->in_room = room;
	object->carried_by = 0;
	object->equiped_by = NULL;
	object->in_obj = NULL;
	object->location = -1;

	room_light (r);

	exit_room = r->dir_option[DIR_DOWN];

        if ( exit_room ) {
        	troom = vtor (exit_room->to_room);
            
            if ( troom && troom->sector_type != SECT_UNDERWATER ){
               	troom = NULL;
            }
        }

	if ( troom && (object->obj_flags.weight/100 >= 1 || GET_ITEM_TYPE (object) == ITEM_MONEY) ) {
		temp_arg = obj_short_desc(object);
		snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 slowly sinks from sight down into the water.", temp_arg);
		buf[2] = toupper(buf[2]);
		send_to_room (buf, r->virtual);
		obj_from_room (&object, 0);
		obj_to_room (object, troom->virtual);
		
		temp_arg = obj_short_desc(object);
		snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 slowly sinks into view from above.", temp_arg);
		buf[2] = toupper(buf[2]);
		send_to_room (buf, object->in_room);
	}	
}

/* Take an object from a room */
void obj_from_room (OBJ_DATA **obj, int count)   /* STACKing */
{
	int			fluid = 0;
	int			volume = 0;
	OBJ_DATA	*tobj = NULL;
	ROOM_DATA	*room = NULL;

	room = vtor ((*obj)->in_room);

	if ( !room )
		return;

	if ( GET_ITEM_TYPE (*obj) == ITEM_DRINKCON ) {
		fluid = (*obj)->o.drinkcon.liquid;
		volume = (*obj)->o.drinkcon.volume;
	}

		/* NPC jailbags disappear, unless they are taken */

	if ( (*obj)->virtual == VNUM_JAILBAG )
		 (*obj)->obj_timer = 0;

			/* Take a partial number of objs? */

	if ( count != 0 && count < (*obj)->count ) {

		(*obj)->count -= count;

		if ( IS_SET ((*obj)->obj_flags.extra_flags, ITEM_VARIABLE) )
			*obj = load_colored_object ((*obj)->virtual, (*obj)->var_color);
		else
			*obj = load_object ((*obj)->virtual);

		(*obj)->count = count;

		if ( GET_ITEM_TYPE (*obj) == ITEM_DRINKCON ) {
			(*obj)->o.drinkcon.liquid = fluid;
			(*obj)->o.drinkcon.volume = volume;
		}

		room_light (room);

		(*obj)->in_room = NOWHERE;
		(*obj)->next_content = NULL;

		return;
	}

			/* Remove object from the room */

	if ( room->contents == *obj )
		room->contents = (*obj)->next_content;
	else {
		for ( tobj = room->contents;
			  tobj->next_content;
			  tobj = tobj->next_content ){
			if ( tobj->next_content == *obj ) {
				tobj->next_content = (*obj)->next_content;
				break;
			}
		}
	}

	
	(*obj)->in_room = NOWHERE;
	(*obj)->next_content = NULL;

	room_light (room);

	remove_obj_affect (*obj, MAGIC_HIDDEN);
	
	return;
}

void obj_to_obj (OBJ_DATA *obj, OBJ_DATA *container)
{
	OBJ_DATA	*tobj = NULL;

	if ( !obj || !container )
		return;

	if ( IS_SET (obj->obj_flags.extra_flags, ITEM_STACK) &&
		 (tobj = get_obj_in_list_num (obj->virtual, container->contains)) ) {
		obj->count += tobj->count;
		extract_obj (tobj);
	}

	obj->in_obj = container;
	obj->equiped_by = NULL;
	obj->carried_by = NULL;
	if ( container->contains && !is_obj_in_list (obj, container->contains) )
		obj->next_content	= container->contains;
	else if ( !container->contains )
		container->contains = obj;
	else {
		extract_obj (obj);
		return;
	}

	container->contains = obj;
	obj->location = -1;

	for ( tobj = container; tobj; tobj = tobj->in_obj )
		tobj->contained_wt += OBJ_MASS (obj);
		
	return;
}

void obj_from_obj (OBJ_DATA **obj, int count)
{
	int			adjust_wt = 0;
	int			fluid = 0;
	int			volume = 0;
	OBJ_DATA	*tobj = NULL;

	if ( !(*obj) )
		return;

	if ( count < 0 )
		count = 0;

	if ( !count )
		count = (*obj)->count;

	if ( GET_ITEM_TYPE (*obj) == ITEM_DRINKCON ) {
		fluid = (*obj)->o.drinkcon.liquid;
		volume = (*obj)->o.drinkcon.volume;
	}

		/* Removing only part of obj from container */

	if ( count < (*obj)->count ) {

		tobj = *obj;

		if ( IS_SET (tobj->obj_flags.extra_flags, ITEM_VARIABLE) )
			*obj = load_colored_object (tobj->virtual, tobj->var_color);
		else *obj = load_object (tobj->virtual);

		tobj->count -= count;

		(*obj)->count = count;
		(*obj)->in_obj = tobj->in_obj;

		if ( GET_ITEM_TYPE (*obj) == ITEM_DRINKCON ) {
			(*obj)->o.drinkcon.liquid = fluid;
			(*obj)->o.drinkcon.volume = volume;
		}

		adjust_wt = (*obj)->count * (*obj)->obj_flags.weight;
	}
		
	else {
		/* Removing obj completely from container */

		adjust_wt = OBJ_MASS (*obj);

		if ( (*obj)->in_obj->contains == *obj )
			(*obj)->in_obj->contains = (*obj)->next_content;
		else {
			for ( tobj = (*obj)->in_obj->contains;
				  tobj->next_content;
				  tobj = tobj->next_content ){
				if ( tobj->next_content == *obj ) {
					tobj->next_content = (*obj)->next_content;
					break;
				}
			}
		}
	}

	for ( tobj = (*obj)->in_obj; tobj; tobj = tobj->in_obj )
		tobj->contained_wt -= adjust_wt;

	(*obj)->next_content = NULL;
	(*obj)->in_obj = NULL;
	
	return;
}

void object_list_new_owner(OBJ_DATA *list, CHAR_DATA *ch)
{
	if (list) {
		object_list_new_owner(list->contains, ch);
		object_list_new_owner(list->next_content, ch);
		list->carried_by = ch;
	}
	
	return;
}

void remove_object_affect (OBJ_DATA *obj, AFFECTED_TYPE *af)
{
	AFFECTED_TYPE	*taf = NULL;

	if ( af == obj->xaffected ) {
		obj->xaffected = af->next;
		mem_free (af);
		return;
	}

	for ( taf = obj->xaffected; taf->next; taf = taf->next ) {
		if ( af == taf->next ) {
			taf->next = af->next;
			mem_free (af);
			return;
		}
	}
	return;
}

void destroy_dwelling (OBJ_DATA *obj)
{
	OBJ_DATA	*tobj = NULL;
	OBJ_DATA	*tobj_next = NULL;
	CHAR_DATA	*tch = NULL;
	CHAR_DATA	*tch_next = NULL;
	ROOM_DATA	*troom = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	if ( !(troom = vtor (obj->o.od.value[0])) )
		return;

	temp_arg = obj_short_desc(obj);
	snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 is suddenly torn down around your ears!", temp_arg);
	buf[2] = toupper(buf[2]);
	send_to_room (buf, troom->virtual);

	for ( tobj = troom->contents; tobj; tobj = tobj_next ) {
		tobj_next = tobj->next_content;
		obj_from_room (&tobj, 0);
		obj_to_room (tobj, obj->in_room);
	}

	for ( tch = troom->people; tch; tch = tch_next ) {
		tch_next = tch->next_in_room;
		char_from_room (tch);
		char_to_room (tch, obj->in_room);
	}
}

void extract_obj (OBJ_DATA *obj)
{
	if ( GET_ITEM_TYPE (obj) == ITEM_DWELLING || GET_ITEM_TYPE (obj) == ITEM_TENT )
		destroy_dwelling (obj);

	while ( obj->contains )
		extract_obj (obj->contains);

	if ( obj->equiped_by != NULL )
		(void)unequip_char (obj->equiped_by, obj->location);

	if ( obj->carried_by != NULL )
		obj_from_char (&obj, 0);
	else if ( obj->in_obj != NULL )
		obj_from_obj (&obj, 0);
	else if ( obj->in_room != NOWHERE )
		obj_from_room (&obj, 0);

	while ( obj->xaffected )
		remove_object_affect (obj, obj->xaffected);

	obj->carried_by = NULL;
	obj->equiped_by = NULL;
	obj->in_obj = NULL;

	obj->deleted = 1;

	knockout = 1;		/* Get obj out of object_list ASAP */
	
	return;
}

void morph_obj (OBJ_DATA *obj)
{
	CHAR_DATA 	*ch = NULL;
	OBJ_DATA 	*container = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA 	*newObj = NULL;
	int 		room = 0;

/** has the object been deleted? **/
	if(obj->deleted){
		return;
	}

/** Is the object in a storage room? **/
	if (obj->in_room &&
	    vtor(obj->in_room) &&
	    IS_SET (vtor(obj->in_room)->room_flags, STORAGE) ){
		return;
	}

/** If the object is supposed to morph, does it have soemthing to morph into, and if it does, will the object dissaper after the morph? or is it replaced**/	
	if(obj->morphto) {
		if(obj->morphto == 86){
			extract_obj(obj);
			return;
		}
		else{
			newObj = load_object(obj->morphto);
		}
	}
	else {
		snprintf (buf, MAX_STRING_LENGTH,  "Object %d has a morph clock, but no morph Objnum\n", obj->virtual);
		system_log (buf, TRUE);
		return;
	}

/** does the new object exist?**/
	if (!newObj) {
		snprintf (buf, MAX_STRING_LENGTH,  "Attempt to load morph obj %d for object %d failed\n", obj->morphto, obj->virtual);
		system_log (buf, TRUE);
		return;
	}

/** Is the old item equipped**/
	if(obj->equiped_by) {
		(void)unequip_char (obj->equiped_by, obj->location);
		equip_char(obj->equiped_by, newObj, obj->location);
	}

/** is the old item carried?**/
	if ( obj->carried_by ) {
		ch = obj->carried_by;
		obj_from_char (&obj, 0);
		obj_to_char(newObj, ch);
	}

/** Is the old object in a container? **/
	if ( obj->in_obj ) {
		container = obj->in_obj;
		obj_from_obj (&obj, 0);
		obj_to_obj(newObj, container);
	}

/** Is the old object in the room? **/
	if ( obj->in_room != NOWHERE ) {
		room = obj->in_room;
		obj_from_room (&obj, 0);
		obj_to_room(newObj, room);
	}

/** Remove any effects from the object **/
	while ( obj->xaffected ){
		remove_object_affect (obj, obj->xaffected);
	}
	
/** Delete the old object **/
	obj->deleted = 1;

	return;

}
	
void remove_sighting (CHAR_DATA *ch, CHAR_DATA *target)
{
	VIEWED_DATA		*sighted = NULL;

	if ( !ch || !target )
		return;

	if ( ch->sighted && ch->sighted->target == target )
		ch->sighted = ch->sighted->next;

	for ( sighted = ch->sighted; sighted; sighted = sighted->next ) {
		if ( sighted->next && sighted->next->target == target )
			sighted->next = sighted->next->next;
	}
	
	return;
}
		
void extract_char (CHAR_DATA *ch)
{
	CHAR_DATA		*k = NULL;
	CHAR_DATA		*next_char = NULL;
	CHAR_DATA		*tch = NULL;
	ROOM_DATA		*troom = NULL;
	OBJ_DATA		*tobj = NULL;
	DESCRIPTOR_DATA		*td = NULL;
	AFFECTED_TYPE		*af = NULL;
	int				was_in = 0;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			*temp_arg = NULL;

	
	if ( port == PLAYER_PORT && IS_NPC(ch) && IS_SET (ch->act, ACT_STAYPUT) ) {
        	mysql_safe_query ("DELETE FROM stayput_mobiles WHERE coldload_id = %d", ch->coldload_id);
		snprintf (buf, MAX_STRING_LENGTH,  "save/mobiles/%d", ch->coldload_id);
		unlink (buf);
	}

	if ( port == PLAYER_PORT && IS_NPC(ch) ) {
		mysql_safe_query ("DELETE FROM reboot_mobiles WHERE coldload_id = %d", ch->coldload_id);
		snprintf (buf, MAX_STRING_LENGTH,  "save/reboot/%d", ch->coldload_id);
		unlink (buf);
	}

        if ( port == PLAYER_PORT && IS_NPC (ch) && ch->mob->reset_cmd ) {
                zone_table [ch->room->zone].cmd [ch->mob->reset_cmd].enabled = 1;
                mysql_safe_query ("INSERT INTO mob_resets VALUES (%d, %d)", ch->room->zone, ch->mob->reset_cmd);
        }

	if ( ch->mob &&
		 IS_SET (ch->act, ACT_VEHICLE) &&
		 (troom = vtor (ch->mob->virtual)) ) {

			/* By mistake, the vehicle might be in the same room as its
               entrance room.  In that case, n/pcs and mobs don't need to
               be moved.
			*/

		if ( troom == ch->room ) {
			act ("$n is destroyed and some people fall out.",
					FALSE, ch, 0, 0, TO_ROOM);
			for ( tch = troom->people; tch; tch = tch->next )
				tch->vehicle = NULL;
		}

		else {
		
			while ( troom->people ) {
				tch = troom->people;
				char_from_room (tch);
				char_to_room (tch, ch->in_room);
				tch->vehicle = NULL;
				act ("$N falls out of $n.", FALSE, ch, 0, tch, TO_NOTVICT);
				act ("You fall out of $N as it is destroyed!",
						FALSE, tch, 0, ch, TO_CHAR);
			}
		
			while ( troom->contents ) {
				tobj = troom->contents;
				obj_from_room (&tobj, 0);
				obj_to_room (tobj, ch->in_room);
			}
		}
	}

	if ( ch->desc && ch->desc->original )
		do_return (ch, "", 0);

	if ( !IS_NPC (ch) && ch->room )
		save_attached_mobiles (ch, 1);

	if ( !IS_NPC (ch) )
		clear_watch (ch);

		/* Clear out guarders */

	for ( k = character_list; k; k = k->next ) {
		if ( k->deleted )
			continue;
		if ( (af = get_affect (k, MAGIC_GUARD)) &&
			 (CHAR_DATA *) af->a.spell.t == ch )
			affect_remove (k, af);
	}

	for ( k = character_list; k; k = k->next ) {
		if ( k->deleted )
			continue;
		if ( k->ranged_enemy == ch ) {
			k->ranged_enemy = NULL;
			k->enemy_direction = NULL;
		}
		if ( k->aiming_at == ch ) {
			k->aiming_at->targeted_by = NULL;
			k->aiming_at = NULL;
			k->aim = 0;
		}
		else if ( k->targeted_by == ch ) {
			k->targeted_by->aiming_at = NULL;
			k->targeted_by->aim = 0;
			k->targeted_by = NULL;
		}
		if ( k->subdue == ch )
			k->subdue = NULL;
		remove_threat (k, ch);
		remove_attacker (k, ch);
		remove_sighting (k, ch);
	}

	if ( !IS_NPC (ch) && ch->pc->edit_player )
		unload_pc (ch->pc->edit_player);

	if ( !IS_NPC (ch) )
		save_char (ch, TRUE);

	if ( !IS_NPC (ch) && !ch->desc ) {
		for ( td = descriptor_list; td; td = td->next )
			if ( td->original == ch )
				do_return (td->character, "", 0);
	}

	if ( ch->in_room == NOWHERE ) {
		system_log("NOWHERE extracting char. (handler.c, extract_char)", TRUE);
		abort();
	}

	stop_followers (ch);

		/* Enable reset for killed mob */

	if ( IS_NPC (ch) ) {
		if ( ch->mob->reset_zone != 0 || ch->mob->reset_cmd != 0 )
			zone_table [ch->mob->reset_zone].cmd [ch->mob->reset_cmd].enabled = 1;
	}

	if ( ch->desc ) {
		if ( ch->desc->snoop.snooping && ch->desc->snoop.snooping->desc )
			ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

		if ( ch->desc->snoop.snoop_by && ch->desc->snoop.snoop_by->desc ) {
			send_to_char ("Your victim is no longer among us.\n\r",
					ch->desc->snoop.snoop_by);
			ch->desc->snoop.snoop_by->desc->snoop.snooping = 0;
		}
		
		ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;
	}

	if ( ch->fighting )
		stop_fighting (ch);

	for (k = combat_list; k ; k = next_char) {	/* FUNDAMENTALLY FLAWED LOGIC */
		next_char = k->next_fighting;
		if ( k->fighting == ch )
			stop_fighting (k);
	}

	/* Must remove from room before removing the equipment! */

	was_in = ch->in_room;
	char_from_room (ch);
	ch->in_room = was_in;

	if ( ch->right_hand )
		extract_obj (ch->right_hand);

	if ( ch->left_hand )
		extract_obj (ch->left_hand);

	while ( ch->equip )
		extract_obj (ch->equip);

	ch->deleted = 1;
	
	if ( IS_NPC (ch) ) {
		while ( ch->hour_affects )
			affect_remove (ch, ch->hour_affects);
	} 

	if ( ch->desc != NULL )
		ch->desc->character = NULL;

	knockout = 1;		/* Get N/PC out of character_list ASAP */

	if ( ch->desc != NULL && !ch->desc->account && !IS_SET (ch->flags, FLAG_GUEST) ) {
		td = ch->desc;
		if ( !maintenance_lock ){
			temp_arg = get_text_buffer (NULL, text_list, "greetings");
			SEND_TO_Q (temp_arg, td);
		}
		else{
			temp_arg = get_text_buffer (NULL, text_list, "greetings.maintenance");
			SEND_TO_Q (temp_arg, td);
		}
		SEND_TO_Q ("Your Choice: ", td);
		td->connected = 1;
		ch->desc = NULL;
		td->character = NULL;
		td->original = NULL;
		td->prompt_mode = 0;
		td->login_time = time_now;
		td->time_last_activity = mud_time;
	}
}

CHAR_DATA *get_char_room_vis (CHAR_DATA *ch, char *name)
{
	CHAR_DATA	*tch = NULL;
	int			j = 1;
	int			number = 0;
	char		*tmp = NULL;
	char		*temp_arg = NULL;

		/* The player may use '.' to indicate last targeted n/pc. */

	if ( !strcmp (name, ".") ) {
		if ( !ch->pc )
			return NULL;

		if ( is_he_here (ch, ch->pc->dot_shorthand, TRUE) )
			return ch->pc->dot_shorthand;
	}

	if ( !strcmp (name, "self") || !strcmp (name, "me") )
		return ch;

	tmp = name;

	if ( !(number = get_number (&tmp)) )
		return NULL;

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
		temp_arg = char_names (tch);
		if ( name_is (tmp, IS_MORTAL (ch) ? temp_arg : tch->name) ) {
			if ( CAN_SEE(ch, tch) || ch == tch ) {

				if ( j == number ) {
					if ( ch->pc )
						ch->pc->dot_shorthand = tch;
					return tch;
				}

				j++;
			}
		}
	}

	return NULL;
}

CHAR_DATA *get_char_room_vis2 (CHAR_DATA *ch, int vnum, char *name)
{
	ROOM_DATA	*room = NULL;
	CHAR_DATA	*tch = NULL;
	int			j = 1;
	int			number = 0;
	char		tmpname [MAX_STRING_LENGTH] = {'\0'};
	char		*tmp = NULL;
	char		*temp_arg = NULL;

	if ( !(room = vtor(vnum)) )
		return NULL;

		/* The player may use '.' to indicate last targeted n/pc. */

	if ( !strcmp (name, ".") ) {
		if ( !ch->pc )
			return NULL;

		if ( is_he_here (ch, ch->pc->dot_shorthand, TRUE) )
			return ch->pc->dot_shorthand;
	}

	if ( !strcmp (name, "self") || !strcmp (name, "me") )
		return ch;

	strcpy (tmpname, name);

	tmp = tmpname;

	if ( !(number = get_number (&tmp)) )
		return NULL;

	for ( tch = room->people; tch; tch = tch->next_in_room ) {
		temp_arg = char_names (tch);
		if ( name_is (tmp, IS_MORTAL (ch) ? temp_arg  : tch->name) ) {
			if ( CAN_SEE(ch, tch) || ch == tch ) {

				if ( j == number ) {
					if ( ch->pc )
						ch->pc->dot_shorthand = tch;
					return tch;
				}

				j++;
			}
		}
	}

	return NULL;
}

CHAR_DATA *get_char_vis(CHAR_DATA *ch, char *name)
{
	CHAR_DATA 	*i = NULL;
	int 		j = 0;
	int			number = 0;
	char		tmpname[MAX_INPUT_LENGTH];
	char 		*tmp = NULL;

	if ( !strcmp (name, "self") || !strcmp (name, "me") )
		return ch;

	/* check location */
	if ((i = get_char_room_vis(ch, name)))
		return(i);

  strcpy(tmpname,name);
	tmp = tmpname;
	if(!(number = get_number(&tmp)))
		return(0);

	for (i = character_list, j = 1; i && (j <= number); i = i->next) {
		
		if ( i->deleted )
			continue;

		if ( name_is (tmp, GET_NAMES (i)) )
			if ( CAN_SEE (ch, i) ) {
				if ( j == number )
					return i;
				j++;
			}
	}

	return 0;
}

OBJ_DATA *get_obj_in_list_vis_not_money (CHAR_DATA *ch, char *name, OBJ_DATA *list)
{
	OBJ_DATA	*obj = NULL;
	int			j = 1;
	int			number = 0;
	char		tmpname[MAX_INPUT_LENGTH];
	char		*tmp = NULL;

	strcpy (tmpname, name);
	tmp = tmpname;

	if ( !(number = get_number (&tmp)) )
		return NULL;

	if ( isdigit (*name) ) {

		if ( !(number = strtol(name, NULL, 10)) )
			return NULL;

		for ( obj = list; number && obj; obj = obj->next_content ) {

			if ( GET_ITEM_TYPE (obj) == ITEM_MONEY )
				continue;

			if ( CAN_SEE_OBJ (ch, obj) && !(--number) )
				return obj;
		}

		return NULL;
	}

	for ( obj = list; obj && (j <= number); obj = obj->next_content ) {

		if ( GET_ITEM_TYPE (obj) == ITEM_MONEY )
			continue;

		if (name_is(tmp, obj->name) || (GET_ITEM_TYPE(obj) == ITEM_BOOK && obj->book_title && name_is(tmp, obj->book_title)) )
			if ( CAN_SEE_OBJ(ch, obj) ) {
				if ( j == number)
					return(obj);
				j++;
			}
	}

	return 0;
}

OBJ_DATA *get_obj_in_list_vis (CHAR_DATA *ch, char *name, OBJ_DATA *list)
{
	OBJ_DATA	*obj = NULL;
	int			j = 1;
	int			number = 0;
	char		tmpname[MAX_INPUT_LENGTH];
	char		*tmp = NULL;

	if ( !name || !*name )
		return NULL;

	if ( !list )
		return NULL;

	strcpy (tmpname, name);
	tmp = tmpname;

	if ( !(number = get_number (&tmp)) )
		return NULL;

	if ( *name == '#' ) {

		if ( !(number = strtol(&name [1], NULL, 10)) )
			return NULL;

		for ( obj = list; number && obj; obj = obj->next_content )
			if ( CAN_SEE_OBJ (ch, obj) && !(--number) )
				return obj;

		return NULL;
	}

	for ( obj = list; obj && (j <= number); obj = obj->next_content ) {
		if (name_is(tmp, obj->name) || (GET_ITEM_TYPE(obj) == ITEM_BOOK && obj->book_title && name_is(tmp, obj->book_title)) )
			if ( CAN_SEE_OBJ(ch, obj) ) {
				if ( j == number)
					return(obj);
				j++;
			}
	}

	return 0;
}

OBJ_DATA *get_obj_in_dark (CHAR_DATA *ch, char *name, OBJ_DATA *list)
{
	OBJ_DATA	*obj = NULL;
	int			j = 1;
	int			number = 0;
	char		tmpname [MAX_INPUT_LENGTH];
	char		*tmp = NULL;

	*tmpname = '\0';

	strcpy (tmpname, name);
	tmp = tmpname;

	if ( !(number = get_number (&tmp)) )
		 return 0;

	if ( *name == '#' ) {

		if ( !(number = strtol(&name [1], NULL, 10)) )
			return NULL;

		for ( obj = list; number && obj; obj = obj->next_content )
			if ( IS_OBJ_VIS (ch, obj) && !(--number) )
				return obj;

		return NULL;
	}

	for ( obj = list; obj && (j <= number); obj = obj->next_content ) {
		if (name_is(tmp, obj->name) || (GET_ITEM_TYPE(obj) == ITEM_BOOK && obj->book_title && name_is(tmp, obj->book_title)) )
			if ( IS_OBJ_VIS (ch, obj) ) {
				if ( j == number )
					return obj;
				j++;
			}
	}

	return 0;
}

/* search the entire world for an object, and return a pointer  */
OBJ_DATA *get_obj_vis(CHAR_DATA *ch, char *name)
{
	OBJ_DATA 	*i = NULL;
	int 		j = 0;
	int			number = 0;
	char		tmpname[MAX_INPUT_LENGTH];
	char 		*tmp = NULL;

	/* scan items carried */
	if ((i = get_obj_in_list_vis(ch, name, ch->right_hand)) )
		return(i);

	if ((i = get_obj_in_list_vis(ch, name, ch->left_hand)) )
		return(i);

	/* scan room */
	if ((i = get_obj_in_list_vis(ch, name, vtor (ch->in_room)->contents)))
		return(i);

  	strcpy(tmpname,name);
	tmp = tmpname;
	
	if(!(number = get_number(&tmp)))
		return(0);

	/* ok.. no luck yet. scan the entire obj list   */
	for (i = object_list, j = 1; i && (j <= number); i = i->next) {

		if ( i->deleted )
			continue;

		if (name_is(tmp, i->name) || (GET_ITEM_TYPE(i) == ITEM_BOOK && i->book_title && name_is(tmp, i->book_title)) )
			if (CAN_SEE_OBJ(ch, i)) {
				if (j == number)
					return(i);
				j++;
			}
	}
	return(0);
}



/** Generic Find, designed to find any object/character                  *
* Calling :                                                              *
*  *arg     is the sting containing the string to be searched for.       *
*           This string doesn't have to be a single word, the routine    *
*           extracts the next word itself.                               *
*  bitv..   All those bits that you want to "search through".            *
*           Bit found will be result of the function                     *
*  *ch      This is the person that is trying to "find"                  *
*  **tar_ch Will be NULL if no character was found, otherwise points     *
* **tar_obj Will be NULL if no object was found, otherwise points        *
*                                                                        *
* The routine returns a pointer to the next word in *arg (just like the  *
* one_argument routine).                                                **/


int generic_find(char *arg, int bitvector, CHAR_DATA *ch,
                   CHAR_DATA **tar_ch, OBJ_DATA **tar_obj)
{
	static char *ignore[] = {
		"the",
		"in",
		"on",
		"at",
		"\n" };

	int 	i = 0;
	char 	name[256];
	bool 	found = FALSE;


	/* Eliminate spaces and "ignore" words */
	while (*arg && !found) {

		for(; *arg == ' '; arg++);

		for(i=0; (name[i] = *(arg+i)) && (name[i]!=' '); i++);
		
		name[i] = 0;
		arg+=i;
		if (search_block(name, ignore, TRUE) > -1)
			found = TRUE;
	}

	if (!name[0])
		return(0);

	*tar_ch  = 0;
	*tar_obj = 0;

/** Find visibile target person in room **/
	if (IS_SET(bitvector, FIND_CHAR_ROOM)) {      
		if ((*tar_ch = get_char_room_vis(ch, name))) {
			return(FIND_CHAR_ROOM);
		}
	}

/** Find visibile target person in the world **/
	if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
		if ((*tar_ch = get_char_vis(ch, name))) {
			return(FIND_CHAR_WORLD);
		}
	}

/** Find equipped object on character **/
	if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
		for(found=FALSE, i=-1; (i< MAX_WEAR) && !found; i++)
			if ( get_equip (ch, i) &&
				 !cmp_strn(name, get_equip (ch, i)->name, strlen(name))) {
				*tar_obj = get_equip (ch, i);
				found = TRUE;
			}
			if (found) {
				return(FIND_OBJ_EQUIP);
			}
	}

/** Find object in characters inventory **/
	if (IS_SET(bitvector, FIND_OBJ_INV)) {
		if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->right_hand))) {
			return(FIND_OBJ_INV);
		}

		if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->left_hand))) {
			return(FIND_OBJ_INV);
		}

		if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->equip)) ) {
			return(FIND_OBJ_INV);
		}
	}

/** find visible object in the room **/ 
	if ( IS_SET (bitvector, FIND_OBJ_ROOM) ) {
		if ( (*tar_obj = get_obj_in_list_vis(ch, name, vtor (ch->in_room)->contents)) ) {
			return (FIND_OBJ_ROOM);
		}
	}

/** Find visible object in the world **/
	if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
		if ((*tar_obj = get_obj_vis(ch, name))) {
			return(FIND_OBJ_WORLD);
		}
		return(0);
	}
	return(0);
}

/* Return TRUE if obj with vnum is equipt */

bool get_obj_in_equip_num (CHAR_DATA *ch, long vnum)
{
	OBJ_DATA	*eq = NULL;

	for ( eq = ch->equip; eq; eq = eq->next_content )
		if ( eq->virtual == vnum )
			return 1;

	return 0;
}

void update_delays (void) 
{
	AFFECTED_TYPE	*af = NULL;
	CHAR_DATA		*ch = NULL;
	CHAR_DATA		*ch_next = NULL;


/** Affects on Character **/
	for ( ch = character_list; ch; ch = ch_next ) {
		ch_next = ch->next; 

		if ( !ch || ch->deleted || !ch->room || ch->in_room == NOWHERE )
			continue;

		if ( IS_MORTAL(ch) && (af = get_affect (ch, AFFECT_HOLDING_BREATH)) ) {
			if ( --af->a.spell.duration <= 0 ) {
				if ( drowned (ch) ){
					continue;
				}
			}
		}

		if ( ch->stun > 0 ) {
			ch->stun--;

			if ( ch->stun <= 0 ) {
				act ("You shake your head, recovering from the stun.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				act ("$n shakes $s head, seeming to recover.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			}
		}

		if ( ch->roundtime ) {
			ch->roundtime--;
			
			if ( ch->roundtime <= 0 ) {
				act ("You can perform another action, now.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			}
		}

		if ( ch->balance < 0 ) {
			ch->balance++;
			
			if ( ch->balance == 0 ) {
				send_to_char ("You feel as if you have fully regained your balance.\n", ch);
			}
		}

		if ( ch->aiming_at && ch->aim < 15 ) {
			ch->aim++;
			if ((ch->in_room == ch->aiming_at->in_room &&
				ch->aim >= 5 &&
				IS_NPC(ch)) ||
				ch->aim == 15 ) {
				send_to_char ("You feel as if you have the best aim you're able to take.\n", ch);

				if ( IS_NPC(ch) && !ch->desc ) {
					if ( CAN_SEE (ch, ch->aiming_at) &&
						(IS_SET (ch->act, ACT_AGGRESSIVE) || !IS_SUBDUEE (ch->aiming_at)) ) {
						do_fire (ch, "", 0);
					}
					else {
						ch->aiming_at = NULL;
						ch->aim = 0;
					}
				}
			}
		}

		if ( ch->whirling && ch->whirling < 4 ) {
			ch->whirling++;
			if ( ch->whirling == 4 ) {
				send_to_char ("You've got the sling whirling about as fast as you can manage.\n", ch);
			}
		}

/** Deal with different types of delays **/
		if ( ch->deleted || !ch->delay || --ch->delay )
			continue;

		switch ( ch->delay_type ) {
			case DEL_PITCH:			delayed_pitch (ch);				break;
			case DEL_OOC:			delayed_ooc (ch);				break;
			case DEL_FORAGE:		delayed_forage (ch);			break;
			case DEL_RUMMAGE:		delayed_rummage(ch);			break;
			case DEL_QUAFF:			delayed_quaff (ch);				break;
			case DEL_APP_APPROVE:	ch->delay += 10;				break;
			case DEL_SKIN_1:		delayed_skin_new1(ch);			break;
            case DEL_SKIN_2:		delayed_skin_new2(ch);			break;
            case DEL_SKIN_3:		delayed_skin_new3(ch);			break;
			case DEL_COUNT_COIN:	delayed_count_coin (ch);		break;
			case DEL_IDENTIFY:		delayed_identify (ch);			break;
			case DEL_GATHER:		delayed_gather (ch);			break;
			case DEL_COMBINE:		delayed_combine (ch);			break;
			case DEL_WHAP:			delayed_whap (ch);				break;
			case DEL_WATER_REMOVE:	delayed_remove (ch);			break;
			case DEL_GET_ALL:		delayed_get (ch);				break;
			case DEL_EMPATHIC_HEAL: delayed_heal (ch);				break;
			case DEL_MENTAL_BOLT:	delayed_bolt (ch);				break;
			case DEL_SEARCH:		delayed_search (ch);			break;
			case DEL_PICK:			delayed_pick (ch);				break;
			case DEL_ALERT:			delayed_alert (ch);				break;
			case DEL_INVITE:		break_delay (ch);				break;
			case DEL_CAMP1:			delayed_camp1 (ch);				break;
			case DEL_CAMP2:			delayed_camp2 (ch);				break;
			case DEL_CAMP3:			delayed_camp3 (ch);				break;
			case DEL_CAMP4:			delayed_camp4 (ch);				break;
			case DEL_TAKE:			delayed_take (ch);				break;
			case DEL_PUTCHAR:		delayed_putchar (ch);			break;
			case DEL_STARE:			delayed_study (ch);				break;
			case DEL_HIDE:			delayed_hide (ch);				break;
			case DEL_SCAN:			delayed_scan (ch);				break;
			case DEL_QUICK_SCAN:	delayed_quick_scan (ch);		break;
			case DEL_HIDE_OBJ:		delayed_hide_obj (ch);			break;
			case DEL_PICK_OBJ:		delayed_pick_obj (ch);			break;
			case DEL_BIND_WOUNDS: 	delayed_bind (ch);				break;
			case DEL_TREAT_WOUND:	delayed_treatment(ch);			break;
			case DEL_LOAD_WEAPON:	delayed_load(ch);				break;
			case DEL_TRACK:			delayed_track(ch);				break;
		}
	}
	return;
}

void break_delay (CHAR_DATA *ch)
{

	if ( ch->aim ) {
		send_to_char ("You cease aiming your weapon.\n", ch);
		ch->aim = 0;
		if ( ch->aiming_at ) {
			ch->aiming_at->targeted_by = NULL;
			ch->aiming_at = NULL;
		}
		return;
	}

	if ( ch->whirling ) {
		send_to_char ("You cease whirling your sling.\n", ch);
		ch->whirling = 0;
		return;
	}

	switch (ch->delay_type) {

		case DEL_LOAD_WEAPON:
			send_to_char ("You cease loading your weapon.\n", ch);
			ch->delay = 0;
			break;

		case DEL_WATER_REMOVE:
			send_to_char ("You cease attempting to remove that item.\n", ch);
			ch->delay = 0;
			break;

		case DEL_BIND_WOUNDS:
			REMOVE_BIT(ch->flags, FLAG_BINDING);
			ch->delay = 0;
			send_to_char ("You cease your ministrations.\n", ch);
			break;

		case DEL_TRACK:
			ch->delay = 0;
			send_to_char ("You discontinue your search for tracks.\n", ch);
			break;

		case DEL_TREAT_WOUND:
			ch->delay = 0;
			send_to_char ("You cease your ministrations.\n", ch);
			break;

		case DEL_BACKSTAB:
			ch->delay = 0;
			send_to_char("You abort your backstab attempt.\n\r",ch);
			break;

		case DEL_PURCHASE_ITEM:
			ch->delay = 0;
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->delay_info2 = 0;
			ch->delay_ch = NULL;
			send_to_char ("You decide against making the purchase.\n\r", ch);
			break;

		case DEL_PITCH:
			ch->delay = 0;
			ch->delay_obj = NULL;
			send_to_char ("You cease pitching the tent.\n\r", ch);
			break;

		case DEL_OOC:
			ch->delay = 0;
			send_to_char ("You decide against going to the OOC lounge.\n\r", ch);
			break;
			
		case DEL_FORAGE:
			ch->delay = 0;
			send_to_char ("You stop foraging.\n\r", ch);
			break;

		case DEL_WEAVESIGHT:
			ch->delay = 0;
			send_to_char ("You blink and shake your heard, ceasing your concentration.\n\r", ch);
			break;

		case DEL_RUMMAGE:
			ch->delay = 0;
			send_to_char ("You stop rummaging.\n\r", ch);
			break;

		case DEL_APP_APPROVE:
			ch->delay = 0;
			send_to_char ("You pass on this application.\n\r", ch);
			if ( ch->pc->msg ) {
				send_to_char ("Your prepared message was deleted.\n\r", ch);
				mem_free (ch->pc->msg);
				ch->pc->msg = NULL;
			}
			break;

		case DEL_SKIN_1:
		case DEL_SKIN_2:
		case DEL_SKIN_3:
			ch->delay = 0;
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->delay_info2 = 0;
			act ("You abruptly end your skinning.",
					TRUE, ch, 0, 0, TO_CHAR);
			act ("$n abruptly stops skinning.",
					FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			break;

		case DEL_COUNT_COIN:
			stop_counting (ch);
			break;

		case DEL_IDENTIFY:
			ch->delay = 0;
			if ( ch->delay_who ) {
				mem_free (ch->delay_who);
				ch->delay_who = NULL;
			}
			act ("You stop trying to identify the flora.",
					TRUE, ch, 0, 0, TO_CHAR);
			break;

		case DEL_GATHER:
			ch->delay = 0;
			if ( ch->delay_who ) {
				mem_free (ch->delay_who);
				ch->delay_who = NULL;
			}
			act ("You stop gathering.", TRUE, ch, 0, 0, TO_CHAR);
			break;

		case DEL_COMBINE:
			ch->delay_who = NULL;		/* DON'T DEALLOCATE, OK? :) */
			ch->delay = 0;
			act ("You stop combining.  Unfortunately your ingrediates are "
				 "ruined.", FALSE, ch, 0, 0, TO_CHAR);
			break;

		case DEL_GET_ALL:
			get_break_delay (ch);
			break;

		case DEL_EMPATHIC_HEAL:
			ch->delay = 0;
			ch->delay_ch = NULL;
			act ("You stop your healing concentration.", FALSE, ch, 0, 0, TO_CHAR);
			break;

		case DEL_MENTAL_BOLT:
			ch->delay = 0;
			ch->delay_ch = NULL;
			act ("You stop your mental attack.", FALSE, ch, 0, 0, TO_CHAR);
			break;

		case DEL_SEARCH:
			ch->delay = 0;
			act ("You stop searching.", FALSE, ch, 0, 0, TO_CHAR);
			act ("$n stops searching.", TRUE, ch, 0, 0, TO_ROOM);
			break;

		case DEL_PICK:
			ch->delay = 0;
			act ("You stop trying to pick the lock.", FALSE, ch, 0, 0, TO_CHAR);
			break;

		case DEL_ALERT:
			ch->delay = 0;
			act ("You forget about responding to the alert.", FALSE, ch, 0, 0, TO_CHAR);
			break;

		case DEL_INVITE:
			ch->delay = 0;
			send_to_char ("You decline to join.\n", ch);
			if ( is_he_here (ch, ch->delay_ch, 1) )
				act ("$N declines your offer.",
						FALSE, ch->delay_ch, 0, ch, TO_CHAR);
			break;

		case DEL_CAMP1:
		case DEL_CAMP2:
		case DEL_CAMP3:
		case DEL_CAMP4:
			ch->delay = 0;
			send_to_char ("You stop building your camp.\n", ch);
			act ("$n stops building $s camp.", TRUE, ch, 0, 0, TO_ROOM);
			break;

		case DEL_TAKE:
			ch->delay = 0;
			send_to_char ("You stop trying to remove the object.\n", ch);
			act ("$n stops trying to get the object from the body.",
						FALSE, ch, 0, 0, TO_ROOM);
			break;

		case DEL_PUTCHAR:
			ch->delay = 0;
			send_to_char ("You stop doing what you're doing.\n", ch);
			act ("$n 'stops doing what $e's doing.", FALSE, ch, 0, 0, TO_ROOM);
			break;

		case DEL_STARE:
			ch->delay = 0;
			break;

		case DEL_HIDE:
			ch->delay = 0;
			send_to_char ("You stop trying to hide.\n", ch);
			break;

		case DEL_SCAN:
			ch->delay = 0;
			break;

		case DEL_QUICK_SCAN:
			ch->delay = 0;
			break;

		case DEL_HIDE_OBJ:
			ch->delay = 0;
			break;

		case DEL_PICK_OBJ:
			ch->delay = 0;
			break;
	}

	ch->delay_type = 0;
}

AFFECTED_TYPE *is_room_affected (AFFECTED_TYPE *af, int type)
{
	while ( af ) {
		if ( af->type == type )
			return af;

		af = af->next;
	}

	return NULL;
}

void add_room_affect (AFFECTED_TYPE **af, int type, int duration)
{
	AFFECTED_TYPE	*raffect = NULL;

	if ( (raffect = is_room_affected (*af, type)) ) {
		raffect->a.room.duration += duration;
		return;
	}

	raffect = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);

	raffect->type = type;
	raffect->a.room.duration = duration;
	raffect->next = *af;

	*af = raffect;
	
	return;
}

int is_in_room (CHAR_DATA *ch, CHAR_DATA *target)
{
	CHAR_DATA	*tch = NULL;

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
		if ( tch == target )
			return 1;
	}

	return 0;
}

int add_registry (int reg_index, int value, char *string)
{
    REGISTRY_DATA       *new_reg = NULL;
    static int          init = 0;

    for ( ; init < MAX_REGISTRY; init++ )       /* Only executes once in */
        registry [init] = NULL;                 /* the life of the game  */

    if ( value == -1 ) {
	abort();
    }

    CREATE (new_reg, REGISTRY_DATA, 1);

    new_reg->string = str_dup(string);
    new_reg->value = value;
    new_reg->next = registry [reg_index];
    registry [reg_index] = new_reg;

    return 0;
}

void free_registry (void)
{
	int		i = 0;

	while ( registry[i] ){
		i++;
	}
	
	i--;
	while ( i ) {
		mem_free (registry[i]->string);
		mem_free (registry[i]);
		i--;
	}

	mem_free (registry[0]->string);
	mem_free (registry[0]);
}

int lookup_value (char *string, int reg_index)
{
    REGISTRY_DATA       *reg = NULL;

    if ( !string )
        return -1;

    for ( reg = registry [reg_index]; reg; reg = reg->next )
        if ( !str_cmp (string, reg->string) )
            return reg->value;

    return -1;
}

char *lookup_string (int value, int reg_index)
{
    REGISTRY_DATA       *reg = NULL;

    for ( reg = registry [reg_index]; reg; reg = reg->next )
        if ( value == reg->value )
            return reg->string;

    return NULL;
}

void reg_read_ov_lv_cap (FILE *fp_reg, char *buf)
{
	int			sn = 0;
	char		*argument = NULL;
	char		skill_name [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};

	while ( !feof (fp_reg) ) {

		(void)fgets (buf, MAX_STRING_LENGTH, fp_reg);

		if ( *buf && buf [strlen (buf) - 1] == '\n' )
			buf [strlen (buf) - 1] = '\0';

		if ( !*buf || *buf == '#' )
			continue;
	
		argument = one_argument (buf, skill_name);

		if ( !str_cmp (skill_name, "[end]") )
			return;

		if ( (sn = skill_index_lookup (skill_name)) == -1 ) {
			printf ("Unknown skill name in registry file: %s\n", skill_name);
			abort();
		}

		argument = one_argument (argument, buf2);

		add_registry (REG_OV, sn, buf2);

		argument = one_argument (argument, buf2);

		add_registry (REG_LV, sn, buf2);

		argument = one_argument (argument, buf2);

		add_registry (REG_CAP, sn, buf2);
	}
	return;
}

void reg_read_skill_max_rates (FILE *fp_reg, char *buf)
{
	int			sn = 0;
	char		*argument = NULL;
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		skill_name [MAX_STRING_LENGTH] = {'\0'};

	while ( !feof (fp_reg) ) {

		(void)fgets (buf, MAX_STRING_LENGTH, fp_reg);

		if ( *buf && buf [strlen (buf) - 1] == '\n' )
			buf [strlen (buf) - 1] = '\0';

		if ( !*buf || *buf == '#' )
			continue;

		argument = one_argument (buf, skill_name);

		if ( !str_cmp (buf, "[end]") )
			return;

		if ( (sn = skill_index_lookup (skill_name)) == -1 ) {
			printf ("Unknown skill name in registry file: %s\n", skill_name);
			abort();
		}

		argument = one_argument (argument, buf2);

		if ( !isdigit (*buf2) ) {
			printf ("Invalid hourly rate for skill %s\n", skill_name);
			abort();
		}

		add_registry (REG_MAX_RATES, sn, buf2);
	}
	return;
}

void reg_read_variables (FILE *fp_reg, char *buf)
{
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		buf3 [MAX_STRING_LENGTH] = {'\0'};
	char		*argument = NULL;
	int			value = 0;

	while ( !feof (fp_reg) ) {

		(void)fgets (buf, MAX_STRING_LENGTH, fp_reg);

		if ( *buf && buf [strlen (buf) - 1] == '\n' )
			buf [strlen (buf) - 1] = '\0';

		if ( !*buf || *buf == '#' )
			continue;

		argument = one_argument (buf, buf2);

		if ( !str_cmp (buf2, "[end]") )
			return;

		argument = one_argument (argument, buf3);

		value = strtol(buf3, NULL, 10);

		if ( !value ) {
			system_log("ILLEGAL VARIABLE VALUE FOR VARIABLE:", TRUE);
			system_log(buf2, TRUE);
			system_log(buf3, TRUE);
			abort();
		}

		if ( !str_cmp (buf2, "pulse_violence") )
			pulse_violence = value;
		else {
			system_log("UNKNOWN VARIABLE:", TRUE);
			system_log(buf2, TRUE);
			abort();
		}
	}
}

void insert_combat_msg (COMBAT_MSG_DATA *cm)
{
	COMBAT_MSG_DATA		*tcm = NULL;
	COMBAT_MSG_DATA		*prev = NULL;

	if ( !cm_list ) {
		cm_list = cm;
		return;
	}

	if ( cm->off_result <= cm_list->off_result ) {
		cm->next = cm_list;
		cm_list = cm;
		return;
	}

	for ( tcm = cm_list; tcm; tcm = tcm->next ) {

		if ( cm->off_result <= tcm->off_result )
			break;

		prev = tcm;
	}

	if ( !tcm )
		prev->next = cm;
	else {
		cm->next = prev->next;
		prev->next = cm;
	}
	
	return;
}

void add_combat_message (char *line)
{
	int				r1 = 0;
	int				r2 = 0;
	char			*argument = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	static COMBAT_MSG_DATA	*cm;
	static int		party = 0;

	if ( !party ) {
		argument = one_argument (line, buf);

		if ( (r1 = index_lookup (rs_name, buf)) == -1 ) {
			system_log("Illegal 1st result in registry, line:", TRUE);
			system_log(line, TRUE);
			abort();
		}

		argument = one_argument (argument, buf);

		if ( (r2 = index_lookup (rs_name, buf)) == -1 ) {
			system_log("Illegal 2nd result in registry, line:", TRUE);
			system_log(line, TRUE);
			abort();
		}

		argument = one_argument (argument, buf);

		CREATE (cm, COMBAT_MSG_DATA, 1);

		*buf = toupper (*buf);

		if ( *buf == 'I' || *buf == 'P' || *buf == 'B' ||
			 *buf == 'D' || *buf == 'F' || *buf == '*' )
			cm->table = *buf;
		else {
			system_log("Illegal table name:  table, off-result:", TRUE);
			system_log(buf, TRUE);
			system_log(rs_name [r1], TRUE);
			abort();
		}

		cm->off_result = r1;
		cm->def_result = r2;
		cm->next = NULL;

	} else
		argument = line;

	while ( isspace (*argument) )
		argument++;

	if ( party == 0 )
		cm->def_msg = str_dup (argument);
	else if ( party == 1 )
		cm->off_msg = str_dup (argument);
	else
		cm->other_msg = str_dup (argument);

	if ( party == 0 )
		insert_combat_msg (cm);

	party++;

	if ( party > 2 )
		party = 0;
		
	return;
}


#define TABLE_USE_TABLE 		1
#define TABLE_COMBAT_MESSAGE	2

void reg_read_tables (FILE *fp_reg, char *buf)
{
	int			table_id = 0;
	int			sn = 0;
	char		skill_name [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		*argument = NULL;

	while ( !feof (fp_reg) ) {

		(void)fgets (buf, MAX_STRING_LENGTH, fp_reg);

		if ( *buf && buf [strlen (buf) - 1] == '\n' )
			buf [strlen (buf) - 1] = '\0';

		if ( !*buf || *buf == '#' )
			continue;
	
		argument = one_argument (buf, buf2);

		if ( !str_cmp (buf2, "[end]") )
			return;

		if ( !cmp_strn (buf2, "end-", 4) && table_id ) {
			table_id = 0;
			continue;
		}

		if ( !table_id ) {
			if ( !str_cmp (buf2, "begin-use-table") )
				table_id = TABLE_USE_TABLE;
			else if ( !str_cmp (buf2, "begin-combat-message-table") )
				table_id = TABLE_COMBAT_MESSAGE;
			else {
				system_log("UNKNOWN TABLE NAME:", TRUE);
				system_log(buf2, TRUE);
				abort();
			}

			continue;
		}

		if ( table_id == TABLE_USE_TABLE ) {
			if ( (sn = skill_index_lookup (buf2)) == -1 ) {
				printf ("Unknown skill in USE-TABLE: %s\n", skill_name);
				abort();
			}

			argument = one_argument (argument, buf2);

			if ( !strtol(buf2, NULL, 10) ) {
				system_log("ILLEGAL/NO VALUE IN REGISTRY, USE-TABLE, SKILL:", TRUE);
				system_log(skill_data[sn].skill_name, TRUE);
				abort();
			}

			use_table [sn].delay = strtol(buf2, NULL, 10);
		}

		else if ( table_id == TABLE_COMBAT_MESSAGE ) {
			add_combat_message (buf);
		}
	}
	return;
}


void read_registry (void)
{
	FILE	*fp_reg;
	char	buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !(fp_reg = fopen (REGISTRY_FILE, "r")) ) {
		perror ("Unable to open registry!");
		abort();
	}

	while ( !feof (fp_reg) ) {

		(void)fgets (buf, MAX_STRING_LENGTH, fp_reg);

		if ( *buf && buf [strlen (buf) - 1] == '\n' )
			buf [strlen (buf) - 1] = '\0';
		
		if ( !*buf || *buf == '#' )
			continue;

		if ( !str_cmp (buf, "[END]") )
			break;

		else if ( !str_cmp (buf, "[OV-LV-CAP]") )
			reg_read_ov_lv_cap (fp_reg, buf);

		else if ( !str_cmp (buf, "[VARIABLES]") )
			reg_read_variables (fp_reg, buf);

		else if ( !str_cmp (buf, "[TABLES]") )
			reg_read_tables (fp_reg, buf);

		else if ( !str_cmp (buf, "[SKILL-MAX-RATES]") )
			reg_read_skill_max_rates (fp_reg, buf);

		else if ( !str_cmp (buf, "[CRAFTS]") )
			reg_read_crafts (fp_reg, buf);
	}

	fclose (fp_reg);
	
	return;
}

void setup_registry (void)
{

	add_registry (REG_REGISTRY, REG_REGISTRY,	"Registry");
	add_registry (REG_REGISTRY,	REG_DURATION,	"Duration");
	add_registry (REG_REGISTRY,	REG_OV,			"OV");
	add_registry (REG_REGISTRY,	REG_LV,			"LV");
	add_registry (REG_REGISTRY,	REG_CAP,		"Cap");
	add_registry (REG_REGISTRY,	REG_MISC,		"Misc");
	add_registry (REG_REGISTRY, REG_MAX_RATES,  "Rates");

	
	add_registry (REG_SKILLS, SKILL_SPEAK_ADUNAIC,	"Adunaic");
	add_registry (REG_SKILLS, SKILL_ALCHEMY,		"Alchemy");
	add_registry (REG_SKILLS, SKILL_SCRIPT_ANGERTHAS_DAERON,
													"Angerthas-Daeron");
	add_registry (REG_SKILLS, SKILL_SCRIPT_ANGERTHAS_EREBOR,
													"Angerthas-Erebor");
	add_registry (REG_SKILLS, SKILL_SCRIPT_ANGERTHAS_MORIA,
													"Angerthas-Moria");
	add_registry (REG_SKILLS, SKILL_SCRIPT_ARNORIAN_TENGWAR,
													"Arnorian-Tengwar");
	add_registry (REG_SKILLS, SKILL_SPEAK_ATLIDUK, 	"Atliduk");
	add_registry (REG_SKILLS, SKILL_SENSITIVITY,	"Aura-Sight");
	add_registry (REG_SKILLS, SKILL_BACKSTAB,		"Backstab");
	add_registry (REG_SKILLS, SKILL_BAKING,			"Baking");
	add_registry (REG_SKILLS, SKILL_BARTER,			"Barter");
	add_registry (REG_SKILLS, SKILL_SCRIPT_BELERIAND_TENGWAR,
													"Beleriand-Tengwar");
	add_registry (REG_SKILLS, SKILL_SPEAK_BLACK_SPEECH,
													"Black-Speech");	
	add_registry (REG_SKILLS, SKILL_BLACK_WISE,		"Black-Wise");
	add_registry (REG_SKILLS, SKILL_BLOCK,			"Block");
	add_registry (REG_SKILLS, SKILL_BRAWLING,		"Brawling");
	add_registry (REG_SKILLS, SKILL_BREWING,		"Brewing");
	add_registry (REG_SKILLS, SKILL_CANDLERY,		"Candlery");
	add_registry (REG_SKILLS, SKILL_SCRIPT_CERTHAS_DAERON,
													"Certhas-Daeron");
	add_registry (REG_SKILLS, SKILL_CLAIRVOYANCE,	"Clairvoyance");
	add_registry (REG_SKILLS, SKILL_CLIMB,			"Climb");
	add_registry (REG_SKILLS, SKILL_COOKERY,		"Cookery");
	add_registry (REG_SKILLS, SKILL_CROSSBOW,		"Crossbow");
	add_registry (REG_SKILLS, SKILL_DUAL,			"Dual-Wield");
	add_registry (REG_SKILLS, SKILL_DANGER_SENSE,	"Danger-Sense");
	add_registry (REG_SKILLS, SKILL_DISARM,			"Disarm");
	add_registry (REG_SKILLS, SKILL_DISTILLING,		"Distilling");
	add_registry (REG_SKILLS, SKILL_DODGE,			"Dodge");
	add_registry (REG_SKILLS, SKILL_SPEAK_DUNAEL,	"Dunael");
	add_registry (REG_SKILLS, SKILL_EMPATHIC_HEAL,	"Empathy");
	add_registry (REG_SKILLS, SKILL_FARMING,		"Farming");
	add_registry (REG_SKILLS, SKILL_FORAGE,			"Forage");
	add_registry (REG_SKILLS, SKILL_GEMCRAFT,		"Gemcraft");
	add_registry (REG_SKILLS, SKILL_GLASSWORK,		"Glasswork");
	add_registry (REG_SKILLS, SKILL_GREY_WISE,		"Grey-Wise");
	add_registry (REG_SKILLS, SKILL_SCRIPT_GONDORIAN_TENGWAR,
													"Gondorian-Tengwar");
	add_registry (REG_SKILLS, SKILL_SPEAK_HARADAIC,	"Haradaic");
	add_registry (REG_SKILLS, SKILL_HEALING,		"Healing");
	add_registry (REG_SKILLS, SKILL_HEAVY_BLUNT,	"Heavy-Blunt");
	add_registry (REG_SKILLS, SKILL_HEAVY_EDGE,		"Heavy-Edge");
	add_registry (REG_SKILLS, SKILL_HEAVY_PIERCE,	"Heavy-Pierce");
	add_registry (REG_SKILLS, SKILL_HERBALISM,		"Herbalism");
	add_registry (REG_SKILLS, SKILL_HEX,			"Hex");
	add_registry (REG_SKILLS, SKILL_HIDE,			"Hide");
	add_registry (REG_SKILLS, SKILL_HIDEWORKING,	"Hideworking");
	add_registry (REG_SKILLS, SKILL_HUNT,			"Hunt");
	add_registry (REG_SKILLS, SKILL_SPEAK_KHUZDUL,	"Khuzdul");
	add_registry (REG_SKILLS, SKILL_SPEAK_LABBA,	"Labba");
	add_registry (REG_SKILLS, SKILL_LIGHT_BLUNT,	"Light-Blunt");
	add_registry (REG_SKILLS, SKILL_LIGHT_EDGE,		"Light-Edge");
	add_registry (REG_SKILLS, SKILL_LIGHT_PIERCE,	"Light-Pierce");
	add_registry (REG_SKILLS, SKILL_LISTEN,			"Listen");
	add_registry (REG_SKILLS, SKILL_LITERACY,		"Literacy");
	add_registry (REG_SKILLS, SKILL_LONGBOW,		"Longbow");
	add_registry (REG_SKILLS, SKILL_PICK,			"Picklock");
	add_registry (REG_SKILLS, SKILL_MENTAL_BOLT,	"Psychic-Bolt");
	add_registry (REG_SKILLS, SKILL_MEDIUM_BLUNT,	"Medium-Blunt");
	add_registry (REG_SKILLS, SKILL_MEDIUM_EDGE,	"Medium-Edge");
	add_registry (REG_SKILLS, SKILL_MEDIUM_PIERCE,	"Medium-Pierce");
	add_registry (REG_SKILLS, SKILL_METALCRAFT,		"Metalcraft");
	add_registry (REG_SKILLS, SKILL_MINING,			"Mining");

	add_registry (REG_SKILLS, SKILL_SPEAK_NAHAIDUK, "Nahaiduk");
	add_registry (REG_SKILLS, SKILL_SPEAK_NORLIDUK, "Norliduk");
	add_registry (REG_SKILLS, SKILL_SCRIPT_NORTHERN_TENGWAR,
													"Northern-Tengwar");
	add_registry (REG_SKILLS, SKILL_SCRIPT_NUMENIAN_TENGWAR,
													"Numenian-Tengwar");
	add_registry (REG_SKILLS, SKILL_SPEAK_ORKISH,   "Orkish");
	add_registry (REG_SKILLS, SKILL_PARRY,			"Parry");
	add_registry (REG_SKILLS, SKILL_POLEARM,		"Polearm");
	add_registry (REG_SKILLS, SKILL_POTTERY,		"Pottery");
	add_registry (REG_SKILLS, SKILL_PRESCIENCE,		"Prescience");
	add_registry (REG_SKILLS, SKILL_SPEAK_PUKAEL,	"Pukael");
	add_registry (REG_SKILLS, SKILL_SPEAK_QUENYA,  	"Quenya");
	add_registry (REG_SKILLS, SKILL_SCRIPT_QUENYAN_TENGWAR,
													"Quenyan-Tengwar");
	add_registry (REG_SKILLS, SKILL_RIDE,			"Ride");
	add_registry (REG_SKILLS, SKILL_RITUAL,			"Ritual");
	add_registry (REG_SKILLS, SKILL_SPEAK_ROHIRRIC, "Rohirric");
	add_registry (REG_SKILLS, SKILL_SAIL,			"Sail");
	add_registry (REG_SKILLS, SKILL_SCRIPT_SARATI,	"Sarati");
	add_registry (REG_SKILLS, SKILL_SCAN,			"Scan");
	add_registry (REG_SKILLS, SKILL_SEAFARING,		"Seafaring");
	add_registry (REG_SKILLS, SKILL_SEARCH,			"Search");
	add_registry (REG_SKILLS, SKILL_SHORTBOW,		"Shortbow");
	add_registry (REG_SKILLS, SKILL_SPEAK_SILVAN,   "Silvan");
	add_registry (REG_SKILLS, SKILL_SPEAK_SINDARIN, "Sindarin");
	add_registry (REG_SKILLS, SKILL_SKIN,			"Skin");
	add_registry (REG_SKILLS, SKILL_SLING,			"Sling");
	add_registry (REG_SKILLS, SKILL_SNEAK,			"Sneak");
	add_registry (REG_SKILLS, SKILL_STAFF,			"Staff");
	add_registry (REG_SKILLS, SKILL_STEAL,			"Steal");
	add_registry (REG_SKILLS, SKILL_STONECRAFT,		"Stonecraft");
	add_registry (REG_SKILLS, SKILL_SUBDUE,			"Subdue");
	add_registry (REG_SKILLS, SKILL_SWIMMING,		"Swim");
	add_registry (REG_SKILLS, SKILL_SPEAK_TALATHIC,	"Talathic");
	add_registry (REG_SKILLS, SKILL_TELEPATHY,		"Telepathy");
	add_registry (REG_SKILLS, SKILL_SCRIPT_TENGWAR,	"Tengwar");
	add_registry (REG_SKILLS, SKILL_TEXTILECRAFT,	"Textilecraft");
	add_registry (REG_SKILLS, SKILL_THROWN,			"Throwing");
	add_registry (REG_SKILLS, SKILL_TRACKING,		"Tracking");
	add_registry (REG_SKILLS, SKILL_SPEAK_UMITIC,	"Umitic");
	add_registry (REG_SKILLS, SKILL_SPEAK_WESTRON,	"Westron");
	add_registry (REG_SKILLS, SKILL_WHITE_WISE,		"White-Wise");
	add_registry (REG_SKILLS, SKILL_WOODCRAFT,		"Woodcraft");

	add_registry (REG_MISC_NAMES, MISC_DELAY_OFFSET,"Delayoffset");
	add_registry (REG_MISC_NAMES, MISC_MAX_CARRY_N,	"Maxcarry_n");
	add_registry (REG_MISC_NAMES, MISC_MAX_CARRY_W,	"Maxcarry_w");
	add_registry (REG_MISC_NAMES, MISC_MAX_MOVE,	"Maxmove");
	add_registry (REG_MISC_NAMES, MISC_STEAL_DEFENSE,"Stealdefense");

	add_registry (REG_MISC,   MISC_DELAY_OFFSET,	"(str + dex) / 6");
	add_registry (REG_MISC,	  MISC_MAX_CARRY_N,		"dex / 3 + 4");
	add_registry (REG_MISC,	  MISC_MAX_CARRY_W,		"str * 2500");
	add_registry (REG_MISC,   MISC_MAX_MOVE,		"(con + wil) / 2 * 5 + 25");
	add_registry (REG_MISC,	  MISC_STEAL_DEFENSE,	"(int + dex) / 2");

	read_registry ();
	
	return;
}

void load_clan_registry (void)
{
	FILE	*fp_dr = NULL;
	NAME_SWITCH_DATA	*last_name = NULL;
	NAME_SWITCH_DATA	*name_switch = NULL;
	char				*argument = NULL;
	char	token [MAX_STRING_LENGTH] = {'\0'};
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	
	if ( !(fp_dr = fopen (CLAN_REGISTRY, "r")) ) {
		perror (CLAN_REGISTRY);
		system_log("Couldn't open CLAN_REGISTRY.", TRUE);
		return;
	}

	while ( fgets (buf, MAX_STRING_LENGTH - 1, fp_dr)) {

		if ( buf [strlen (buf) - 1] == '\n' ){
			buf [strlen (buf) - 1] = '\0';
		}
		
		argument = one_argument (buf, token);

		if ( !str_cmp (token, "newclanname")) {
			name_switch = (struct name_switch_data *)alloc ((int)sizeof (struct name_switch_data), 38);
			argument = one_argument (argument, token);
			name_switch->old_name = str_dup (token);
			argument = one_argument (argument, token);
			name_switch->new_name = str_dup (token);

			if ( !last_name ){
				clan_name_switch_list = name_switch;
			}
			else{
				last_name->next = name_switch;
			}
			
			last_name = name_switch;
			continue;
		}

		if ( !str_cmp (token, "clandef") ) {
			add_clandef (argument);
			continue;
		}
	}

	fclose (fp_dr);
}
