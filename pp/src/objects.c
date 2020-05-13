/** 
*	\file objects.c
*	Provides services for using objects 
*
*	The purpose of this module is to provide utilities and functionality for
*	commands using different objects.
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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

extern const int herbArray[HERB_NUMSECTORS * HERB_RARITIES][5]; 

char *show_damage (OBJ_DATA *obj)
{
	OBJECT_DAMAGE	*damage = NULL;
	char			*p = NULL;
	static char		buf [MAX_STRING_LENGTH] = {'\0'};


	if ( obj->damage )
		snprintf (buf, MAX_STRING_LENGTH,  "It bears ");

	for ( damage = obj->damage; damage; damage = damage->next ) {
		if ( !damage->next && damage != obj->damage )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "and a ");
		else
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "a ");

		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s %s", damage->severity, damage->name);
		if ( !damage->next )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  ".#0\n");
		else snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  ", ");
	}

	if ( *buf ) {
		reformat_string (buf, &p);
		snprintf (buf, MAX_STRING_LENGTH,  "\n   %s", p);
		mem_free (p);
	}

	return buf;
}

void damage_to_obj (OBJ_DATA *obj, int impact, int type)
{
	OBJECT_DAMAGE	*damage = NULL;
	char			name [MAX_STRING_LENGTH] = {'\0'};
	char			severity [MAX_STRING_LENGTH] = {'\0'};

	
	/*** function is buggy, and only useful to testors, so let's skip it for now ***/
	return;

	/*  No object damage from bare fists. */
	if ( type == 9 )
		return;

	if ( !obj->damage ) {
		CREATE (obj->damage, OBJECT_DAMAGE, 1);
		damage = obj->damage;
		
	}
	else for ( damage = obj->damage; damage; damage = damage->next ) {
		/* 
		* if ( !damage->next ) { 
		* 	CREATE (damage->next, OBJECT_DAMAGE, 1);
		* 	damage = damage->next;
		* 	break;
		* }
		*/
		send_to_gods("Inside for loop");
	}

	if ( impact <= 1 )
		snprintf (severity, 10, "miniscule");

	else if ( impact <= 2 )
		snprintf (severity,6, "small");

	else if ( impact <= 4 )
		snprintf (severity, 5, "minor");

	else if ( impact <= 10 )
		snprintf (severity, 9, "moderate");

	else if ( impact <= 20 )
		snprintf (severity, 6, "large");

	else if ( impact <= 30 )
		snprintf (severity, 5, "deep");

	else if ( impact <= 40 )
		snprintf (severity, 8, "massive");

	else if ( impact > 40 )
		snprintf (severity, 8, "terrible");

/* 	Damage on fabric-type items, e.g. cloth and leather */

	if ( (GET_ITEM_TYPE (obj) == ITEM_ARMOR && obj->o.od.value[0] <= 2) ||
		  GET_ITEM_TYPE (obj) == ITEM_WORN ) {
		if ( type == 2 || type == 4 ) {
			switch (number (1,4)) {
				case 1:
					snprintf (name, 6, "slice");
					break;
				case 2:
					snprintf (name, 4, "cut");
					break;
				case 3:
					snprintf (name, 6, "slash");
					break;
				case 4:
					snprintf (name, 5, "gash");
					break;
			}
		}

		else if ( type == 0 || type == 1 ) {
			switch ( number(1,4) ) {
				case 1:
					snprintf (name, 9, "puncture");
					break;
				case 2:
					snprintf (name, 5, "hole");
					break;
				case 3:
					snprintf (name, 12, "perforation");
					break;
				case 4:
					snprintf (name, 9, "piercing");
					break;
			}
		}
		
		else if ( type == 3 || type == 7 || type == 8 ) {
			switch ( number(1,3) ) {
				case 1:
					snprintf (name, 5, "tear");
					break;
				case 2:
					snprintf (name, 4, "rip");
					break;
				case 3:
					snprintf (name, 5, "rent");
					break;
			}
		}
		
		else if ( type == 5 ) {
			snprintf (name, 12, "freeze-burn");
		}
		
		else if ( type == 6 ) {
			snprintf (name, 5, "char");
		}			

		else if ( type == 10 ) {
			snprintf (name, 11, "bloodstain");
		}
	}

/* 	Damage on all other [hard] items, e.g. heavy metal armor or weapons */

	else {
		if ( type == 2 || type == 4 ) {
			switch (number (1,4)) {
				case 1:
					snprintf (name, 5, "nick");
					break;
				case 2:
					snprintf (name, 5, "chip");
					break;
				case 3:
					snprintf (name, 6, "rivet");
					break;
				case 4:
					snprintf (name, 5, "gash");
					break;
			}
		}
		
		else if ( type == 0 || type == 1 ) {
			switch ( number(1,3) ) {
				case 1:
					snprintf (name, 9, "puncture");
					break;
				case 2:
					snprintf (name, 6, "gouge");
					break;
				case 3:
					snprintf (name, 12, "perforation");
					break;
			}
		}
		
		else if ( type == 3 || type == 7 || type == 8 ) {
			switch ( number(1,4) ) {
				case 1:
					snprintf (name, 6, "gouge");
					break;
				case 2:
					snprintf (name, 6, "rivet");
					break;
				case 3:
					snprintf (name, 5, "nick");
					break;
				case 4:
					snprintf (name, 5, "dent");
					break;
			}
		}
		
		else if ( type == 5 ) {
			snprintf (name, 12, "freeze-burn");
		}

		else if ( type == 6 ) {
			snprintf (name, 5, "char");
		}
	}

	damage->severity = str_dup (severity);
	damage->name = str_dup (name);

	if (type == 2 || type == 4)
		damage->type = str_dup ("slash");   
	
	else if (type == 0 || type == 1)
		damage->type = str_dup ("pierce");
	
	else if (type == 3)
		damage->type = str_dup ("blunt");
	
	else if (type == 5)
		damage->type = str_dup ("frost");
	
	else if (type == 6)
		damage->type = str_dup ("fire");
	
	else if (type == 7)
		damage->type = str_dup ("bite");
	
	else if (type == 8)
		damage->type = str_dup ("claw");

	if ( strcmp (name, "bloodstain") ) {
		damage->damage = impact;
		obj->item_wear -= impact;
	}
	return;
}

char *obj_condition (OBJ_DATA *obj)
{
	return NULL;
}

void do_grip (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*obj = NULL;

	if ( !*argument && !(obj = get_equip (ch, WEAR_PRIM)) && !(obj = get_equip (ch, WEAR_BOTH)) ) {
		send_to_char ("What item did you wish to shift your grip on?\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf && !(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->left_hand)) ) {
		send_to_char ("You don't have that in your inventory.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (obj) != ITEM_WEAPON ) {
		send_to_char ("The grip command is only for use with weapons.\n", ch);
		return;
	}

	if ( obj->o.od.value[3] == SKILL_BRAWLING ) {
		send_to_char ("The grip command cannot be used with this weapon type.\n", ch);
		return;
	}


	if ( obj->o.od.value[3] != SKILL_MEDIUM_EDGE && obj->o.od.value[3] != SKILL_MEDIUM_PIERCE &&
		obj->o.od.value[3] != SKILL_MEDIUM_BLUNT ) {
		if ( (ch->str <= 19) ||
			 (obj->o.od.value[3] != SKILL_HEAVY_EDGE &&
			  obj->o.od.value[3] != SKILL_HEAVY_PIERCE &&
			  obj->o.od.value[3] != SKILL_HEAVY_BLUNT) )
			 {
			send_to_char ("You cannot shift your grip upon that weapon.\n", ch);
			return;
		}
	}
	
	argument = one_argument (argument, buf);

	if ( (!*buf && obj->location == WEAR_PRIM) || !str_cmp (buf, "both") ) {
		if ( ch->right_hand && ch->left_hand ) {
			send_to_char ("You'll need to free your other hand to switch to a two-handed grip.\n", ch);
			return;
		}
		if ( obj->location == WEAR_BOTH ) {
			send_to_char ("You are already gripping your weapon in both hands.\n", ch);
			return;
		}
		snprintf (buf, MAX_STRING_LENGTH,  "You shift to a two-handed grip on #2%s#0.", obj->short_description);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		snprintf (buf, MAX_STRING_LENGTH,  "$n shifts to a two-handed grip on #2%s#0.", obj->short_description);
		act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		obj->location = WEAR_BOTH;
	}
	else if ( !(*buf && obj->location == WEAR_BOTH) || !str_cmp (buf, "single") ) {
		if ( obj->location == WEAR_PRIM || obj->location == WEAR_SEC ) {
			send_to_char ("You are already gripping your weapon in your primary hand.\n", ch);
			return;
		}
		snprintf (buf, MAX_STRING_LENGTH,  "You shift to a single-handed grip on #2%s#0.", obj->short_description);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		snprintf (buf, MAX_STRING_LENGTH,  "$n shifts to a single-handed grip on #2%s#0.", obj->short_description);
		act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		obj->location = WEAR_PRIM;
	}
}

void refresh_colors (CHAR_DATA *keeper)
{
	ROOM_DATA	*room = NULL;
	OBJ_DATA	*tobj = NULL;
	OBJ_DATA	*next_obj = NULL;
	int			i = 0;
	int			j = 0;
	int			reload_objs [500];

        if ( !IS_NPC (keeper) || !IS_SET (keeper->flags, FLAG_KEEPER) || !keeper->shop )
                return;
        
        if ( !(room = vtor(keeper->shop->store_vnum)) )
                return;

        for ( tobj = room->contents; tobj; tobj = next_obj ) {
		next_obj = tobj->next_content;
                if ( keeper_makes (keeper, tobj->virtual) && IS_SET (tobj->obj_flags.extra_flags, ITEM_VARIABLE) && !number(0,1) ) {
			reload_objs [i] = tobj->virtual;
			i++;
			extract_obj (tobj);
		}
        }
	
	if ( i ) {
		for ( j = 0; j < i; j++ ) {
			tobj = load_object (reload_objs[j]);
			if ( tobj )
				obj_to_room (tobj, room->virtual);
		}
	}
}


void do_switch_item (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*right = NULL;
	OBJ_DATA	*left = NULL;

	right = ch->right_hand;
	left = ch->left_hand;

	if ( !right && !left ) {
		act ("You have nothing to switch!", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( (right && right->location == WEAR_BOTH) ||
	     (left && left->location == WEAR_BOTH) ) {
		act ("You must grip that in both hands!", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	ch->right_hand = NULL;
	ch->left_hand = NULL;

	if ( right && right->location != WEAR_BOTH ) {
		act ("You shift $p to your left hand.", FALSE, ch, right, 0, TO_CHAR | TO_ACT_FORMAT);
		ch->left_hand = right;
	}

	if ( left && left->location != WEAR_BOTH ) {
		act ("You shift $p to your right hand.", FALSE, ch, left, 0, TO_CHAR | TO_ACT_FORMAT);
		ch->right_hand = left;
	}

}


void clear_omote(OBJ_DATA *obj)
{

        if(obj->omote_str) {
            mem_free(obj->omote_str);
            obj->omote_str = (char *)NULL;
        }
}

int can_obj_to_container (OBJ_DATA *obj, OBJ_DATA *container, char **msg, int count)
{
	OBJ_DATA	*tobj = NULL;
	int			i = 0;
	static char		message [160];
	char		*temp_arg = NULL;
	
	if ( count > obj->count || count <= 0 )
		count = obj->count;

	*msg = message;

	if ( GET_ITEM_TYPE (container) == ITEM_SHEATH) {
		if ( GET_ITEM_TYPE (obj) != ITEM_WEAPON ) {
			snprintf (message, MAX_STRING_LENGTH, "Sheaths are only for storing weaponry.\n");
			return 0;
		}
		if ( container->contains ) {
			snprintf (message, MAX_STRING_LENGTH, "This sheath already contains a weapon.\n");
			return 0;
		}
		if ( container->contained_wt + OBJ_MASS (obj) > container->o.od.value [0] ) {
			snprintf (message, MAX_STRING_LENGTH, "That weapon is too large for the sheath.\n");
			return 0;
		}
		return 1;
	}

	if ( GET_ITEM_TYPE (container) == ITEM_KEYRING ) {
		if ( GET_ITEM_TYPE (obj) != ITEM_KEY ) {
			snprintf (message, MAX_STRING_LENGTH, "Keyrings are only able to hold keys!\n");
			return 0;
		}
		for ( tobj = container->contains, i = 1; tobj; tobj = tobj->next_content )
			i++;
		if ( i + 1 > container->o.od.value[0] ) {
			snprintf (message, MAX_STRING_LENGTH, "There are too many keys on this keyring to add another.\n");
			return 0;
		}
		return 1;
	}

	if ( GET_ITEM_TYPE (container) == ITEM_WEAPON && container->o.weapon.use_skill == SKILL_SLING ) {
		if ( GET_ITEM_TYPE (obj) != ITEM_BULLET ) {
			snprintf (message, MAX_STRING_LENGTH, "You'll need to find proper ammunition use with the sling.\n");
			return 0;
		}
		if ( container->contains ) {
			snprintf (message, MAX_STRING_LENGTH, "The sling is already loaded!\n");
			return 0;
		}
		if ( obj->count > 1 ) {
			snprintf (message, MAX_STRING_LENGTH, "The sling is only capable of throwing one projectile at a time.\n");
			return 0;
		}
		return 1;
	}

	if ( GET_ITEM_TYPE (container) == ITEM_QUIVER) {
		if ( GET_ITEM_TYPE (obj) != ITEM_MISSILE ) {
			snprintf (message, MAX_STRING_LENGTH, "Quivers are only for storing arrows and bolts.\n");
			return 0;
		}
		for ( tobj = container->contains, i = 0; tobj; tobj = tobj->next_content )
			i += tobj->count;
		if ( i + obj->count > container->o.od.value[0] ) {
			snprintf (message, MAX_STRING_LENGTH, "The quiver can't hold that many missiles.\n");
			return 0;
		}
		return 1;
	}

	if ( GET_ITEM_TYPE (container) != ITEM_CONTAINER) {
		snprintf (message, MAX_STRING_LENGTH, "%s is not a container.\n",
						  container->short_description);
		*message = UPPER (*message);
		return 0;
	}

	if ( container == obj ) {
		snprintf (message, MAX_STRING_LENGTH, "You can't do that.\n");
		return 0;
	}

	if ( IS_SET (container->o.od.value [1], CONT_CLOSED) ) {
		snprintf (message, MAX_STRING_LENGTH, "%s is closed.\n", container->short_description);
		*message = UPPER (*message);
		return 0;
	}

	if ( count > 1 ) {
		if ( container->contained_wt + obj->obj_flags.weight * count >
						container->o.od.value [0] ) {
			snprintf (message, MAX_STRING_LENGTH, "That much won't fit.\n");
			return 0;
		}
	}

	else if ( (container->contained_wt + OBJ_MASS (obj)) > 
				container->o.od.value [0] ) {
		temp_arg = obj_short_desc (obj);			
		snprintf (message, MAX_STRING_LENGTH, "%s won't fit.\n", temp_arg);
		*message = UPPER (*message);
		return 0;
	}

	return 1;
}

#define NO_TOO_MANY		1
#define NO_TOO_HEAVY	2
#define NO_CANT_TAKE	3
#define NO_CANT_SEE		4
#define NO_HANDS_FULL	5

int can_obj_to_inv (OBJ_DATA *obj, CHAR_DATA *ch, int *error, int count)
{

	if ( count > obj->count || count <= 0 )
		count = obj->count;

	*error = 0;

	if ( !obj->in_obj && !CAN_SEE_OBJ (ch, obj) ) {
		*error = NO_CANT_SEE;
		return 0;
	}

	if ( !CAN_WEAR (obj, ITEM_TAKE) || IS_SET (obj->obj_flags.extra_flags, ITEM_PITCHED) ) {
		*error = NO_CANT_TAKE;
		return 0;
	}

	if ( ch->right_hand && (ch->right_hand->virtual == obj->virtual)
             && (GET_ITEM_TYPE (ch->right_hand) == ITEM_MONEY) )
		return 1;

	if ( ch->left_hand && (ch->left_hand->virtual == obj->virtual)
             && (GET_ITEM_TYPE (ch->left_hand) == ITEM_MONEY) )
		return 1;

	if ( (ch->right_hand && ch->left_hand) || get_equip (ch, WEAR_BOTH) ) {
		*error = NO_HANDS_FULL;
		return 0;
	}

		/* Check out the weight */

	if ( !(obj->in_obj && obj->in_obj->carried_by == ch) ) {

		if ( (IS_CARRYING_W (ch) + (count * obj->obj_flags.weight)) > CAN_CARRY_W (ch) && IS_MORTAL (ch) ) {
			*error = NO_TOO_HEAVY;
			return 0;
		}

	}

	return 1;
}

int obj_activate (CHAR_DATA *ch, OBJ_DATA *obj)
{
	if ( !obj->activation )
		return 0;

	
		/* Deal with one time activation on object */

	if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_MULTI_AFFECT) )
		obj->activation = 0;

		/* Oops, I guess that killed him. */

	if ( GET_POS (ch) == POSITION_DEAD )
		return 1;

	return 0;
}

void get (CHAR_DATA *ch, OBJ_DATA *obj, int count)
{
	OBJ_DATA	*container = NULL;

	if ( IS_SET (obj->obj_flags.extra_flags, ITEM_TIMER) && obj->virtual != VNUM_CORPSE ) {
		REMOVE_BIT (obj->obj_flags.extra_flags, ITEM_TIMER);
		obj->obj_timer = 0;
	}

	if ( !obj->in_obj ) {

		if ( IS_SET (obj->tmp_flags, SPA_DROPPED) ) {
			send_to_char ("That can't be picked up at the moment.\n", ch);
			return;
		}

		obj_from_room (&obj, count);

		clear_omote(obj);

		act("You get $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
		act("$n gets $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

		obj_to_char (obj, ch);

		if ( obj->activation &&
			 IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT) )
			obj_activate (ch, obj);

		return;
	}

		/* Don't activate object if it is in an object we're carrying */

	container = obj->in_obj;

	obj_from_obj (&obj, count);

	act ("You get $p from $P.", FALSE, ch, obj, container, TO_CHAR | TO_ACT_FORMAT);
	act("$n gets $p from $P.", TRUE, ch, obj, container, TO_ROOM | TO_ACT_FORMAT);

	obj_to_char (obj, ch);

	if ( obj->activation &&
		 IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT) )
		obj_activate (ch, obj);
	
	return;
}

int just_a_number (char *buf)
{
	int		i = 0;

	if ( !*buf )
		return 0;

	for ( i = 0; i < strlen (buf); i++ )
		if ( !isdigit (buf [i]) )
			return 0;

	return 1;
}

void do_junk (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("What did you wish to junk?\n", ch);
		return;
	}

	if ( !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) ) {
		send_to_char ("You don't seem to be holding that.\n", ch);
		return;
	}

	obj_from_char (&obj, 0);

	if ( port == PLAYER_PORT ) {
		obj_to_room (obj, JUNKYARD);
		obj->obj_timer = 96;		/*  Junked items saved for 1 RL day. */
		SET_BIT (obj->obj_flags.extra_flags, ITEM_TIMER);
	}
	else {
		extract_obj (obj);
	}


	snprintf (buf, MAX_STRING_LENGTH,  "You junk #2%s#0.", obj->short_description);
	act (buf, FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
	
	return;
}

#define CONTAINER_LOC_NOT_FOUND	0
#define CONTAINER_LOC_ROOM		1
#define CONTAINER_LOC_INVENTORY	2
#define CONTAINER_LOC_WORN		3
#define CONTAINER_LOC_UNKNOWN	4

void do_get (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*hitch = NULL;
	ROOM_DATA		*hitch_room = NULL;
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*container = NULL;
	int				container_loc = CONTAINER_LOC_UNKNOWN;
	int				count = 0;
	int				error = 0;
	SECOND_AFFECT	*sa = NULL;
	char			arg1 [MAX_STRING_LENGTH] = {'\0'};
	char			arg2 [MAX_STRING_LENGTH] = {'\0'};
	bool			coldload_id = FALSE;

	*arg1 = '\0';
	*arg2 = '\0';

	if ( IS_MORTAL (ch) && IS_SET (ch->room->room_flags, OOC) ) {
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	argument = one_argument (argument, arg1);

	if ( just_a_number (arg1) ) {
		count = strtol(arg1, NULL, 10);
		argument = one_argument (argument, arg1);
	}
	else if ( !str_cmp (arg1, ".c") ) {
		coldload_id = TRUE;
		argument = one_argument (argument, arg1);
	}

	argument = one_argument (argument, arg2);

	if ( !str_cmp (arg2, "from") || !str_cmp (arg2, "in") )
		argument = one_argument (argument, arg2);

	if ( !str_cmp (arg2, "ground") || !str_cmp (arg2, "room") ) {
		argument = one_argument (argument, arg2);
		container_loc = CONTAINER_LOC_ROOM;
	}

	else if ( !str_cmp (arg2, "worn") || !str_cmp (arg2, "my") ) {
		argument = one_argument (argument, arg2);
		container_loc = CONTAINER_LOC_WORN;
	}

	else if ( !cmp_strn (arg2, "inventory", 3) ) {
		argument = one_argument (argument, arg2);
		container_loc = CONTAINER_LOC_INVENTORY;
	}

	if ( *arg2 &&
		 container_loc == CONTAINER_LOC_UNKNOWN &&
		 (hitch = get_char_room_vis (ch, arg2)) &&
		 hitch->mob &&
		 hitch->mob->vehicle_type == VEHICLE_HITCH &&
		 (hitch_room = vtor (hitch->mob->virtual)) ) {

		if ( !(obj = get_obj_in_list_vis (ch, arg1, hitch_room->contents)) ) {
			act ("You don't see that in $N.", FALSE, ch, 0, hitch, TO_CHAR | TO_ACT_FORMAT);
			return;
		}

		if ( !can_obj_to_inv (obj, ch, &error, count) ) {

			if ( error == NO_CANT_TAKE )
				act ("You can't take $o.", TRUE, ch, obj, 0, TO_CHAR);
			else if ( error == NO_TOO_MANY )
				act ("You can't handle so much.", TRUE, ch, 0, 0, TO_CHAR);
			else if ( error == NO_TOO_HEAVY )
				act ("You can't carry so much weight.",TRUE, ch, 0, 0, TO_CHAR);
			else if ( error == NO_CANT_SEE )
				act ("You don't see it.", TRUE, ch, 0, 0, TO_CHAR);
			else if ( error == NO_HANDS_FULL )
				act ("Your hands are full!", TRUE, ch, 0, 0, TO_CHAR);
			return;
		}

		obj_from_room (&obj, count);

		act ("You take $p from $N.", FALSE, ch, obj, hitch, TO_CHAR | TO_ACT_FORMAT);
		act ("$n takes $p from $N.", FALSE, ch, obj, hitch, TO_NOTVICT | TO_ACT_FORMAT);

		char_from_room (ch);
		char_to_room (ch, hitch->mob->virtual);

		act ("$n reaches in and takes $p.", FALSE, ch, obj, hitch, TO_NOTVICT | TO_ACT_FORMAT);

		char_from_room (ch);
		char_to_room (ch, hitch->in_room);

		obj_to_char (obj, ch);

		return;
	}

	else if ( *arg2 ) {

		if ( container_loc == CONTAINER_LOC_UNKNOWN &&
			 !(container = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
			 !(container = get_obj_in_dark (ch, arg2, ch->left_hand)) &&
			 !(container = get_obj_in_dark (ch, arg2, ch->equip)) &&
			 !(container = get_obj_in_list_vis (ch, arg2, ch->room->contents)) )
			container_loc = CONTAINER_LOC_NOT_FOUND;

		else if ( container_loc == CONTAINER_LOC_ROOM &&
				  !(container = get_obj_in_list_vis (ch, arg2, ch->room->contents)) ) {
			container_loc = CONTAINER_LOC_NOT_FOUND;
		}

		else if ( container_loc == CONTAINER_LOC_INVENTORY &&
				  !(container = get_obj_in_dark (ch, arg2, ch->right_hand)) &&
				  !(container = get_obj_in_dark (ch, arg2, ch->left_hand)) )
			container_loc = CONTAINER_LOC_NOT_FOUND;

		else if ( container_loc == CONTAINER_LOC_WORN &&
				  !(container = get_obj_in_dark (ch, arg2, ch->equip)) )
			container_loc = CONTAINER_LOC_NOT_FOUND;

		if ( container_loc == CONTAINER_LOC_NOT_FOUND ) {
			send_to_char ("You neither have nor see such a container.\n", ch);
			return;
		}

		if ( GET_ITEM_TYPE (container) != ITEM_CONTAINER &&
		     GET_ITEM_TYPE (container) != ITEM_QUIVER &&
		     (GET_ITEM_TYPE (container) != ITEM_WEAPON &&
		      container->o.weapon.use_skill != SKILL_SLING) &&
	             GET_ITEM_TYPE (container) != ITEM_SHEATH &&
		     GET_ITEM_TYPE (container) != ITEM_KEYRING ) {
			act ("$o isn't a container.", TRUE, ch, container, 0, TO_CHAR);
			return;
		}

                if ( IS_SET (container->o.container.flags, CONT_CLOSED) ) {
                        send_to_char ("That's closed!\n", ch);
            		return;
                }

		if ( container_loc == CONTAINER_LOC_UNKNOWN ) {
			if ( container->carried_by )
				container_loc = CONTAINER_LOC_INVENTORY;
			else if ( container->equiped_by )
				container_loc = CONTAINER_LOC_WORN;
			else
				container_loc = CONTAINER_LOC_ROOM;
		}
	}

	if ( !*arg1 ) {
		send_to_char ("Get what?\n", ch);
		return;
	}

	if ( !str_cmp (arg1, "all") ) {

		for ( obj = (container ? container->contains : ch->room->contents);
			  obj && !can_obj_to_inv (obj, ch, &error, 1);
			  obj = obj->next_content )
			;

		if ( !obj ) {
			send_to_char ("There is nothing left you can take.\n", ch);
			return;
		}

		ch->delay_type = DEL_GET_ALL;
		ch->delay_who = (char *) container; 
		ch->delay_info1 = container_loc;
		ch->delay = 4;
	}

	else if ( *arg2 && !str_cmp (arg2, "all") ) {
		send_to_char ("You'll have to get things one at a time.\n", ch);
		return;
	}

	if ( !container ) {

		if ( !obj && isdigit (*arg1) && coldload_id ) {
			if ( !(obj = get_obj_in_list_id (strtol(arg1, NULL, 10), ch->room->contents)) || obj->in_room != ch->in_room ) {
				send_to_char ("You don't see that here.\n", ch);
				return;
			}
		}

		if ( !obj && !(obj = get_obj_in_list_vis (ch, arg1, ch->room->contents)) ) {
			send_to_char ("You don't see that here.\n", ch);
			return;
		}

		if ( !can_obj_to_inv (obj, ch, &error, count) ) {

			if ( error == NO_CANT_TAKE )
				act ("You can't take $o.", TRUE, ch, obj, 0, TO_CHAR);
			else if ( error == NO_TOO_MANY )
				act ("You can't handle so much.", TRUE, ch, 0, 0, TO_CHAR);
			else if ( error == NO_TOO_HEAVY )
				act ("You can't carry so much weight.",TRUE, ch, 0, 0, TO_CHAR);
			else if ( error == NO_CANT_SEE )
				act ("You don't see it.", TRUE, ch, 0, 0, TO_CHAR);
			else if ( error == NO_HANDS_FULL )
				act ("Your hands are full!", TRUE, ch, 0, 0, TO_CHAR);			
			return;
		}

		if ( (sa = get_second_affect (ch, SPA_GET_OBJ, obj)) )
			return;

		get (ch, obj, count);
		return;
	}

				/* get obj from container */

	if ( !obj && !(obj = get_obj_in_dark (ch, arg1, container->contains)) ) {
		act ("You don't see that in $o.", TRUE, ch, container, 0, TO_CHAR);
		return;
	}

	if ( !can_obj_to_inv (obj, ch, &error, count) ) {

		if ( error == NO_CANT_TAKE )
			act ("You cannot take $o.", TRUE, ch, obj, 0, TO_CHAR);
		else if ( error == NO_TOO_HEAVY )
			send_to_char ("You can't carry so much weight.\n", ch);
		else if ( error == NO_TOO_MANY )
			send_to_char ("Your can't handle so much.\n", ch);
		else if ( error == NO_HANDS_FULL )
			act ("Your hands are full!", TRUE, ch, 0, 0, TO_CHAR);
		return;
	}

	if ( container && container != obj )
		obj->in_obj = container;
	if ( !container->contains )
		container->contains = obj;

	get (ch, obj, count);
}

void do_take (CHAR_DATA *ch, char *argument, int cmd)
{
	int				worn_object = 0;
	CHAR_DATA		*victim = NULL;
	OBJ_DATA		*obj = NULL;
	char			obj_name [MAX_STRING_LENGTH] = {'\0'};
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};

	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	argument = one_argument (argument, obj_name);

	if ( !*obj_name ) {
		send_to_char ("Take what?\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "from") )
		argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Take from whom?\n", ch);
		return;
	}

	if ( !(victim = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see them.\n", ch);
		return;
	}

	if ( victim == ch ) {
		send_to_char ("Why take from yourself?\n", ch);
		return;
	}

	if ( !(obj = get_obj_in_list_vis (ch, obj_name, victim->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, obj_name, victim->left_hand)) ) {

		if ( !(obj = get_obj_in_list_vis (ch, obj_name, victim->equip)) ) {
			act ("You don't see that on $N.", TRUE, ch, 0, victim, TO_CHAR);
			return;
		}

		if ( GET_TRUST (ch) ) {
			(void)unequip_char (victim, obj->location);
			obj_to_char (obj, victim);
		} else
			worn_object = 1;
	}

	if ( GET_POS (victim) == SLEEP && !GET_TRUST (ch) ) {

		wakeup (victim);

		if ( GET_POS (victim) != SLEEP ) {

			act ("$N awakens as you touch $M.", TRUE, ch, 0, victim, TO_CHAR);
			act ("$n awakens $N as $e touches $M.",
					FALSE, ch, 0, victim, TO_NOTVICT);

			if ( !get_affect (victim, MAGIC_AFFECT_PARALYSIS) )
				return;
		}
	}
		
	if ( !(GET_TRUST (ch) ||
		   GET_POS (victim) <= SLEEP ||
		   get_affect (victim, MAGIC_AFFECT_PARALYSIS) ||
		   IS_SUBDUEE (victim) ||
		   IS_MOUNT (victim)) ) {
		act ("$N prevents you from taking $p.", TRUE, ch, obj, victim, TO_CHAR);
		act ("$N unsuccessfully tries to take $p from you.",
				TRUE, victim, obj, ch, TO_CHAR);
		act ("$n prevents $N from taking $p.",
				FALSE, victim, obj, ch, TO_NOTVICT);
		return;
	}

	if ( worn_object ) {

		strcpy (buf2, locations [obj->location]);
		*buf2 = tolower (*buf2);

		snprintf (buf, MAX_STRING_LENGTH,  "You begin to remove $p from $N's %s.", buf2);
		act (buf, TRUE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);

		snprintf (buf, MAX_STRING_LENGTH,  "$n begins removing $p from $N's %s.", buf2);
		act (buf, FALSE, ch, obj, victim, TO_NOTVICT | TO_ACT_FORMAT);

		snprintf (buf, MAX_STRING_LENGTH,  "$N begins removing $p from your %s.", buf2);
		act (buf, TRUE, victim, obj, ch, TO_CHAR | TO_ACT_FORMAT);

		ch->delay_info1 = (int) obj;
		ch->delay_info2 = obj->location;
		ch->delay_ch    = victim;
		ch->delay_type  = DEL_TAKE;
		ch->delay       = 15;

		return;
	}

	obj_from_char (&obj, 0);
	obj_to_char (obj, ch);

	if ( !GET_TRUST (ch) ) {
		act ("$n takes $p from you.", TRUE, victim, obj, ch, TO_CHAR | TO_ACT_FORMAT);
		act ("$n takes $p from $N.", FALSE, ch, obj, victim, TO_NOTVICT | TO_ACT_FORMAT);
	}

	clear_omote(obj);

	act ("You take $p from $N.", TRUE, ch, obj, victim, TO_CHAR);

	if ( obj->activation &&
		 IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT) )
		obj_activate (ch, obj);

	return;
}

void delayed_take (CHAR_DATA *ch)
{
	OBJ_DATA		*obj = NULL;
	CHAR_DATA		*victim = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	ch->delay = 0;
	victim = ch->delay_ch;
	obj = (OBJ_DATA *) ch->delay_info1;

	if ( !is_he_here (ch, victim, TRUE) ) {
		send_to_char ("Your victim left before you could finish taking the "
					  "object.\n", ch);
		return;
	}

	if ( get_equip (victim, ch->delay_info2) != obj ) {
		send_to_char ("The thing you were after is gone now.", ch);
		return;
	}

	if ( GET_POS (victim) == SLEEP && !GET_TRUST (ch) ) {

		wakeup (victim);

		if ( GET_POS (victim) != SLEEP ) {

			act ("$N awakens as struggle with $M.",
					TRUE, ch, 0, victim, TO_CHAR);
			act ("$n awakens $N as $e struggles with $M.",
					FALSE, ch, 0, victim, TO_NOTVICT);

			if ( !get_affect (victim, MAGIC_AFFECT_PARALYSIS) )
				return;
		}
	}
		
	if ( !(GET_TRUST (ch) ||
		   GET_POS (victim) <= SLEEP ||
		   get_affect (victim, MAGIC_AFFECT_PARALYSIS) ||
		   IS_SUBDUEE (victim) ||
		   IS_MOUNT (victim)) ) {
		act ("$N prevents you from taking $p.", TRUE, ch, obj, victim, TO_CHAR);
		act ("$N unsuccessfully tries to take $p from you.",
				TRUE, victim, obj, ch, TO_CHAR);
		act ("$n prevents $N from taking $p.",
				FALSE, victim, obj, ch, TO_NOTVICT);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "$n removes and takes $p from $N's %s.",
				  locations [obj->location]);
	act (buf, FALSE, ch, obj, victim, TO_NOTVICT | TO_ACT_FORMAT);

	snprintf (buf, MAX_STRING_LENGTH,  "$N removes and takes $p from your %s.",
				  locations [obj->location]);
	act (buf, TRUE, victim, obj, ch, TO_CHAR | TO_ACT_FORMAT);

	snprintf (buf, MAX_STRING_LENGTH,  "You remove and take $p from $N's %s.",
				  locations [obj->location]);
	act (buf, TRUE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);

	(void)unequip_char (victim, obj->location);
	obj_to_char (obj, ch);

	if ( obj->activation &&
		 IS_SET (obj->obj_flags.extra_flags, ITEM_GET_AFFECT) )
		obj_activate (ch, obj);

	return;
}

void get_break_delay (CHAR_DATA *ch)
{
	if ( ch->delay_info1 == CONTAINER_LOC_ROOM )
		send_to_char ("You stop picking things up.\n", ch);
	else
		send_to_char ("You stop removing things.\n", ch);

	ch->delay = 0;

	return;
}

void delayed_get (CHAR_DATA *ch)
{
	OBJ_DATA		*container = NULL;
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*first_obj = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	int				item_num = 0;
	int				container_num = 0;
	int				error = 0;
	char			*locs [5] = { "", "room", "inventory", "worn", "" };

	if ( ch->delay_who ) { 
			/* Makes sure that this container is in the room */

		container = (OBJ_DATA *) ch->delay_who;

		if ( ch->delay_info1 == CONTAINER_LOC_ROOM )
			obj = ch->room->contents;
		else if ( ch->delay_info1 == CONTAINER_LOC_INVENTORY )
			obj = ch->right_hand;
		else if ( ch->delay_info1 == CONTAINER_LOC_WORN )
			obj = ch->equip;
		else
			obj = NULL;

		for ( ; obj; obj = obj->next_content ) {

			container_num++;

			if ( obj == container )
				break;
		}

		if ( !obj ) {
			send_to_char ("You can't get anything else.\n", ch);
			return;
		}

		first_obj = container->contains;
	} else
		first_obj = ch->room->contents;

	for ( obj = first_obj; obj; obj = obj->next_content ) {

		if ( !IS_OBJ_VIS (ch, obj) )
			continue;

		item_num++;

		if ( can_obj_to_inv (obj, ch, &error, 0) ) {

			if ( container_num )
				snprintf (buf, MAX_STRING_LENGTH,  "#%d %s #%d",
					item_num, locs [ch->delay_info1], container_num);
			else
				snprintf (buf, MAX_STRING_LENGTH,  "#%d", item_num);

			do_get (ch, buf, 0);

			if ( obj->carried_by != ch )
				printf ("Oh boy...couldn't pick up %d\n", obj->virtual);
			else
				ch->delay = 4;

			return;
		}
	}

	send_to_char ("...and that's about all you can get.\n", ch);

	ch->delay = 0;
	
	return;
}

void delayed_remove (CHAR_DATA *ch)
{
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*eq = NULL;
	OBJ_DATA	*temp_obj = NULL;

	obj = (OBJ_DATA *) ch->delay_who;

	if ( !obj ) {
		ch->delay_type = 0;
		ch->delay_who = 0;
		ch->delay = 0;
		return;
	}

	if ( obj->location == WEAR_WAIST ) {
		if ( (eq = get_equip (ch, WEAR_BELT_1)) ) {
			act ("$p falls free.", TRUE, ch, eq, 0, TO_CHAR);
			act ("$n drops $p.", TRUE, ch, eq, 0, TO_ROOM);
			temp_obj = unequip_char (ch, WEAR_BELT_1);
			obj_to_room (temp_obj, ch->in_room);
		}

		if ( (eq = get_equip (ch, WEAR_BELT_2)) ) {
			act ("$p falls free.", TRUE, ch, eq, 0, TO_CHAR);
			act ("$n drops $p.", TRUE, ch, eq, 0, TO_ROOM);
			temp_obj = unequip_char (ch, WEAR_BELT_2); 
			obj_to_room (temp_obj, ch->in_room);
		}
	}

	if ( obj->loaded && obj->o.weapon.use_skill != SKILL_CROSSBOW )
		do_unload (ch, "", 0);

	if ( IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		 IS_SET (ch->affected_by, AFF_HOODED) )
		do_hood (ch, "", 0);

	

	if ( obj->location == WEAR_PRIM || obj->location == WEAR_SEC ||
		obj->location == WEAR_BOTH || obj->location == WEAR_SHIELD )
		temp_obj = unequip_char (ch, obj->location);
	else {
		temp_obj = unequip_char (ch, obj->location);
		obj_to_char (temp_obj, ch);
	}
	
	act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM);

	ch->delay = 0;
	ch->delay_type = 0;
	ch->delay_who = 0;
	
	return;
}

void drop_all (CHAR_DATA *ch)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buffer [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*tobj = NULL;
	OBJ_DATA	*obj = NULL;
	char		*temp_arg = NULL;

	if ( ch->right_hand ) {
		act ("You drop:", FALSE, ch, 0, 0, TO_CHAR);
		act ("$n drops:", FALSE, ch, 0, 0, TO_ROOM);
	}

	while ( ch->right_hand || ch->left_hand ) {

		if ( ch->right_hand )
			obj = ch->right_hand;
		else obj = ch->left_hand;

		act ("   $p", FALSE, ch, obj, 0, TO_CHAR);	
		act ("   $p", FALSE, ch, obj, 0, TO_ROOM);

		obj_from_char (&obj, 0);
		obj_to_room (obj, ch->in_room);

	        if ( obj->loaded && obj->o.weapon.use_skill != SKILL_CROSSBOW ) {
	        	temp_arg = obj_short_desc(obj->loaded);
        	    snprintf (buffer, MAX_STRING_LENGTH,  "%s#0 clatters to the ground!", temp_arg);
				*buffer = toupper(*buffer);
                snprintf (buf, MAX_STRING_LENGTH,  "#2%s", buffer);
                act (buf, TRUE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
            	act (buf, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
				tobj = load_object (obj->loaded->virtual);
				extract_obj (obj->loaded);
				obj->loaded = NULL;
                obj_to_room (tobj, ch->in_room);
                obj->loaded = NULL;
        	}

		if ( GET_ITEM_TYPE (obj) == ITEM_WEAPON && obj->o.weapon.use_skill == SKILL_SLING && ch->whirling ) {
			send_to_char ("You cease whirling your sling.\n", ch);
			ch->whirling = 0;
		}
	}
	
	return;
}

void do_drop (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buffer [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*obj =NULL;
	OBJ_DATA	*tobj = NULL;
	ROOM_DATA	*room = NULL;
	int			count = 0;
	char		*temp_arg = NULL;
	
	argument = one_argument (argument, buf);

	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	if ( just_a_number (buf) ) {
		count = strtol(buf, NULL, 10);
		argument = one_argument (argument, buf);
	}

	if ( !*buf ) {
		send_to_char ("Drop what?\n", ch);
		return;
	}

	if ( !str_cmp (buf, "all") ) {

		argument = one_argument (argument, buf);

		if ( *buf ) {
			send_to_char ("You can only 'drop all'.\n", ch);
			return;
		}

		drop_all (ch);

		return;
	}

	if ( !(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->left_hand)) ) {
		send_to_char("You do not have that item.\n", ch);
		return;
	}

	obj_from_char (&obj, count);

	act ("You drop $p.", TRUE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
	act ("$n drops $p.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);

	obj_to_room (obj, ch->in_room);

	if ( obj->activation &&
		 IS_SET (obj->obj_flags.extra_flags, ITEM_DROP_AFFECT) )
		obj_activate (ch, obj);

        if ( obj->loaded && obj->o.weapon.use_skill != SKILL_CROSSBOW ) {
        	temp_arg = obj_short_desc(obj->loaded);
            snprintf (buffer, MAX_STRING_LENGTH,  "%s#0 clatters to the ground!", temp_arg);
			*buffer = toupper(*buffer);
            snprintf (buf, MAX_STRING_LENGTH,  "#2%s", buffer);
            act (buf, TRUE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
            act (buf, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			tobj = load_object (obj->loaded->virtual);
			extract_obj (obj->loaded);
			obj->loaded = NULL;
            obj_to_room (tobj, ch->in_room);
            obj->loaded = NULL;
       	}

	if ( GET_ITEM_TYPE (obj) == ITEM_WEAPON && obj->o.weapon.use_skill == SKILL_SLING && ch->whirling ) {
		send_to_char ("You cease whirling your sling.\n", ch);
		ch->whirling = 0;
	}

	room = ch->room;

	return;
}

void put_on_char (CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj,
				  int count)
{
	int			location = 0;
	ROOM_DATA	*hitch_room = NULL;
	OBJ_DATA	*temp_obj = NULL;

	if ( victim->mob && victim->mob->vehicle_type == VEHICLE_HITCH ) {

		if ( (hitch_room = vtor (victim->mob->virtual)) ) {

			obj_from_char (&obj, count);

			act ("You put $p in $N.", FALSE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);
			act ("$n puts $p in $N.", FALSE, ch, obj, victim, TO_NOTVICT | TO_ACT_FORMAT);

			char_from_room (ch);
			char_to_room (ch, victim->mob->virtual);

			act ("$n reaches in and drops $p.",
					FALSE, ch, obj, victim, TO_NOTVICT | TO_ACT_FORMAT);

			char_from_room (ch);
			char_to_room (ch, victim->in_room);

			obj_to_room (obj, victim->mob->virtual);

			return;
		}

		act ("You can't put $p in $N.", FALSE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);

		return;
	}

	if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HEAD) )
		location = WEAR_HEAD;
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HAIR) )
		location = WEAR_HAIR;
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_EYE) )
		location = WEAR_EYE;
		
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FACE) )
		location = WEAR_FACE;
		
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_MOUTH) )
		location = WEAR_MOUTH;
		
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_THROAT) )
		location = WEAR_THROAT;
		
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_NECK) ){
		if ( get_equip (victim, WEAR_NECK_1) )
			location = WEAR_NECK_2;
		else
			location = WEAR_NECK_1;
	}
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BODY) ){
		if ( get_equip (ch, WEAR_BODY_1) &&
		 	 get_equip (ch, WEAR_BODY_2) &&
		 	 get_equip (ch, WEAR_BODY_3) &&
		 	 get_equip (ch, WEAR_BODY_4)) {
				location = WEAR_BODY_5;
		}
		
		else if ( get_equip (ch, WEAR_BODY_1) &&
		 	 	  get_equip (ch, WEAR_BODY_2) &&
				  get_equip (ch, WEAR_BODY_3)) {
				location = WEAR_BODY_4;
		}
		
		else if ( get_equip (ch, WEAR_BODY_1) &&
		 	 	  get_equip (ch, WEAR_BODY_2)) {
				location = WEAR_BODY_3;
		}
		
		else if ( get_equip (ch, WEAR_BODY_1)) {
				location = WEAR_BODY_2;
		}
		
		else {
			location = WEAR_BODY_1;
		}
		
	}
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ABOUT) )
		location = WEAR_ABOUT;
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BACK) )
		location = WEAR_BACK;
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_SHOULDER) ) {
		if ( get_equip (victim, WEAR_SHOULDER_L) )
			location = WEAR_SHOULDER_R;
		else
			location = WEAR_SHOULDER_L;
	}
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ARMS) )
		location = WEAR_ARMS;
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WRIST) ) {
		if ( get_equip (victim, WEAR_WRIST_L) )
			location = WEAR_WRIST_R;
		else
			location = WEAR_WRIST_L;
	}
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HANDS) )
		location = WEAR_HANDS;
		
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FINGER) ) {
		if ( get_equip (victim, WEAR_FINGER_L) )
			location = WEAR_FINGER_R;
		else
			location = WEAR_FINGER_L;
	}
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WAIST) )
		location = WEAR_WAIST;
		
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_LEGS) ){
		if ( get_equip (victim, WEAR_LEGS_1) &&
			 get_equip (victim, WEAR_LEGS_2))
			location = WEAR_LEGS_3;
		else if (get_equip (victim, WEAR_LEGS_1))
			location = WEAR_LEGS_2;
		else
			location = WEAR_LEGS_1;
	}
	
	else if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FEET) ){
		if ( get_equip (victim, WEAR_FEET_1) )
			location = WEAR_FEET_2;
		else
			location = WEAR_FEET_1;
	}
	
	else {
		act ("You can't put $p on $M.", TRUE, ch, obj, victim, TO_CHAR);
		return;
	}

	if ( IS_MOUNT (victim) &&
		 !IS_SET (obj->obj_flags.extra_flags, ITEM_MOUNT) ) {
		act ("$N is a mount.  You can't put $p on it.",
				TRUE, ch, obj, victim, TO_CHAR);
		return;
	}

	if ( get_equip (victim, location) ) {
		temp_obj = get_equip (victim, location);
		act ("$N is already wearing $p.", FALSE, ch, temp_obj, victim, TO_CHAR);
		return;
	}

	if ( GET_POS (victim) == SLEEP && !GET_TRUST (ch) ) {

		wakeup (victim);

		if ( GET_POS (victim) != SLEEP ) {

			act ("$N awakens as you touch $M.",
					TRUE, ch, 0, victim, TO_CHAR);
			act ("$n awakens $N as $e touches $M.",
					FALSE, ch, 0, victim, TO_NOTVICT);

			if ( !get_affect (victim, MAGIC_AFFECT_PARALYSIS) )
				return;
		}
	}
		
	if ( !(GET_TRUST (ch) ||
		   GET_POS (victim) <= SLEEP ||
		   get_affect (victim, MAGIC_AFFECT_PARALYSIS) ||
		   IS_MOUNT (victim)) ) {
		act ("$N stops you from putting $p on $M.",
				TRUE, ch, obj, victim, TO_CHAR);
		act ("$N unsuccessfully tries to put $p from you.",
				TRUE, victim, obj, ch, TO_CHAR);
		act ("$n stops $N from putting $p on $M.",
				FALSE, victim, obj, ch, TO_NOTVICT);
		return;
	}

	ch->delay_type = DEL_PUTCHAR;
	ch->delay = 7;
	ch->delay_ch = victim;
	ch->delay_info1 = (int) obj;
	ch->delay_info2 = location;

	act ("$n begins putting $p on $N.", FALSE, ch, obj, victim, TO_NOTVICT | TO_ACT_FORMAT);
	act ("$n begins putting $p on you.", TRUE, ch, obj, victim, TO_VICT | TO_ACT_FORMAT);
	act ("You begin putting $p on $N.", TRUE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);
	
	return;
}

void delayed_putchar (CHAR_DATA *ch)
{
	int				location = 0;
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*tobj = NULL;
	CHAR_DATA		*victim = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};

	ch->delay = 0;
	victim = ch->delay_ch;
	obj = (OBJ_DATA *) ch->delay_info1;
	location = ch->delay_info2;

	if ( !is_he_here (ch, victim, TRUE) ) {
		send_to_char ("Your victim left before you could finish dressing "
					  "them.\n", ch);
		return;
	}

	if ( !is_obj_in_list (obj, ch->right_hand) &&
		!is_obj_in_list (obj, ch->left_hand) ) {
		act ("You no longer have the thing you were putting on $N.",
				FALSE, ch, 0, victim, TO_CHAR);
		act ("$n stops putting something on $N.", TRUE, ch, 0, victim, TO_ROOM);
		act ("$n stops putting something on you.",
				TRUE, ch, 0, victim, TO_VICT);
		return;
	}


	tobj = get_equip (victim, ch->delay_info2);

	if ( tobj && tobj != obj ) {
		act ("You discover that $N is already wearing $p.",
				TRUE, ch, tobj, victim, TO_CHAR);
		act ("$n stops dressing $N.", FALSE, ch, 0, victim, TO_NOTVICT);
		return;
	}

	if ( GET_POS (victim) == SLEEP && !GET_TRUST (ch) ) {

		wakeup (victim);

		if ( GET_POS (victim) != SLEEP ) {

			act ("$N awakens as struggle with $M.",
					TRUE, ch, 0, victim, TO_CHAR);
			act ("$n awakens $N as $e struggles with $M.",
					FALSE, ch, 0, victim, TO_NOTVICT);

			if ( !get_affect (victim, MAGIC_AFFECT_PARALYSIS) )
				return;
		}
	}
		
	if ( !(GET_TRUST (ch) ||
		   GET_POS (victim) <= SLEEP ||
		   get_affect (victim, MAGIC_AFFECT_PARALYSIS) ||
		   IS_MOUNT (victim)) ) {
		act ("$N prevents you from taking $p.", TRUE, ch, obj, victim, TO_CHAR);
		act ("$N unsuccessfully tries to take $p from you.",
				TRUE, victim, obj, ch, TO_CHAR);
		act ("$n prevents $N from taking $p.",
				FALSE, victim, obj, ch, TO_NOTVICT);
		return;
	}

	obj_from_char (&obj, 0);
	equip_char (victim, obj, location);

	strcpy (buf2, locations [location]);
	*buf2 = tolower (*buf2);

	snprintf (buf, MAX_STRING_LENGTH,  "$n puts $p on $N's %s.", buf2);
	act (buf, FALSE, ch, obj, victim, TO_NOTVICT | TO_ACT_FORMAT);

	snprintf (buf, MAX_STRING_LENGTH,  "$N puts $p on your %s.", buf2);
	act (buf, TRUE, victim, obj, ch, TO_CHAR | TO_ACT_FORMAT);

	snprintf (buf, MAX_STRING_LENGTH,  "You put $p on $N's %s.", buf2);
	act (buf, TRUE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);

	if ( obj->activation &&
		 IS_SET (obj->obj_flags.extra_flags, ITEM_WEAR_AFFECT) )
		obj_activate (ch, obj);
	
	return;
}

void do_put (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buffer [MAX_STRING_LENGTH] = {'\0'};
	char		arg [MAX_STRING_LENGTH] = {'\0'};
	char		*error = NULL;
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*tar = NULL;
	CHAR_DATA	*victim = NULL;
	int			count = 0;

	argument = one_argument (argument, arg);

	if ( just_a_number (arg) ) {
		count = strtol(arg, NULL, 10);
		argument = one_argument (argument, arg);
	}

	if ( !arg ) {
		send_to_char ("Put what?\n", ch);
		return;
	}

	if ( !(obj = get_obj_in_dark (ch, arg, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, arg, ch->left_hand)) ) {
		snprintf (buffer, MAX_STRING_LENGTH,  "You don't have a %s.\n", arg);
		send_to_char (buffer, ch);
		return;
	}

	argument = one_argument (argument, arg);

	if ( !str_cmp (arg, "in") || !str_cmp (arg, "into") )
		argument = one_argument (argument, arg);

	else if ( !str_cmp (arg, "on") ) {

		argument = one_argument (argument, arg);

		if ( !(victim = get_char_room_vis (ch, arg)) ) {
			act ("Put $p on whom?", TRUE, ch, obj, 0, TO_CHAR);
			return;
		}

		put_on_char (ch, victim, obj, count);

		return;
	}

	if ( !*arg ) {
		act ("Put $o into what?", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}
	
	if ( !(tar = get_obj_in_dark (ch, arg, ch->right_hand)) &&
		 !(tar = get_obj_in_dark (ch, arg, ch->left_hand)) &&
		 !(tar = get_obj_in_dark (ch, arg, ch->equip)) &&
		 !(tar = get_obj_in_list_vis (ch, arg, ch->room->contents)) ) {

		if ( (victim = get_char_room_vis (ch, arg)) ) {
			put_on_char (ch, victim, obj, count);
			return;
		}

		snprintf (buffer, MAX_STRING_LENGTH,  "You don't see a %s.\n", arg);
		send_to_char (buffer, ch);
		return;
	}

	if ( GET_ITEM_TYPE (obj) == ITEM_WEAPON && obj->o.weapon.use_skill != SKILL_CROSSBOW && obj->loaded ) {
		send_to_char ("You'll need to unload that first.\n", ch);
		return;
	}

	if ( !can_obj_to_container (obj, tar, &error, count) ) {
		send_to_char (error, ch);
		return;
	}

	obj_from_char (&obj, count);

	if ( IS_SET (tar->obj_flags.extra_flags, ITEM_TABLE) ) {
		act ("$n puts $p on $P.", TRUE, ch, obj, tar, TO_ROOM | TO_ACT_FORMAT);
		act ("You put $p on $P.", FALSE, ch, obj, tar, TO_CHAR | TO_ACT_FORMAT);
	} else {
		act ("$n puts $p into $P.", TRUE, ch, obj, tar, TO_ROOM | TO_ACT_FORMAT);
		act ("You put $p into $P.", FALSE, ch, obj, tar, TO_CHAR | TO_ACT_FORMAT);
	}

	obj_to_obj (obj, tar);
	return;
}


void do_give (CHAR_DATA *ch, char *argument, int cmd)
{
	char			obj_name [MAX_INPUT_LENGTH];
	char			vict_name [MAX_INPUT_LENGTH];
	CHAR_DATA		*vict = NULL;
	OBJ_DATA		*obj = NULL;
	int				count = 0;
	int				error;

	argument = one_argument (argument, obj_name);

	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	if ( just_a_number (obj_name) ) {
		count = strtol(obj_name, NULL, 10);
		argument = one_argument (argument, obj_name);
	}

	if ( IS_SET (ch->act, ACT_MOUNT) ) {
		send_to_char ("Mounts can't use this command.n", ch);
		return;
	}

	if ( !*obj_name ) {
		send_to_char ("Give what?\n", ch);
		return;
	}

	if ( !(obj = get_obj_in_dark (ch, obj_name, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, obj_name, ch->left_hand)) ) {
		send_to_char ("You do not seem to have anything like that.\n", ch);
		return;
	}

	argument = one_argument (argument, vict_name);

	if ( !str_cmp (vict_name, "to") )
		argument = one_argument (argument, vict_name);

	if ( !*vict_name ) {

		if ( obj->obj_flags.type_flag == ITEM_TICKET ) {
			unstable (ch, obj, NULL);
			return;
		}

		send_to_char("Give to whom?\n", ch);
		return;
	}

	if ( !(vict = get_char_room_vis (ch, vict_name)) ) {
		send_to_char("No one by that name around here.\n", ch);
		return;
	}

	if ( vict == ch ) {
		send_to_char ("Give it to yourself? How generous...\n", ch);
		return;
	}

	if ( obj->obj_flags.type_flag == ITEM_TICKET ) {
		unstable (ch, obj, vict);
		return;
	}

	if ( !can_obj_to_inv (obj, vict, &error, count) ) {
		if ( error == NO_HANDS_FULL ) {
			act ("$N's hands are currently occupied, I'm afraid.", TRUE, ch, obj, vict, TO_CHAR | TO_ACT_FORMAT);
			act ("$n just tried to give you $o, but your hands are full.", TRUE, ch, obj, vict, TO_VICT | TO_ACT_FORMAT);
			return;
		}
		else if ( error == NO_TOO_HEAVY ) {
			act ("$N struggles beneath the weight of the object, and so you take it back.", TRUE, ch, obj, vict, TO_CHAR | TO_ACT_FORMAT);
			act ("$n just tried to give you $o, but it is too heavy for you to carry.", TRUE, ch, obj, vict, TO_VICT | TO_ACT_FORMAT);
			return;
		}
		else if ( error == NO_CANT_TAKE ) {
			act ("This item cannot be given.", FALSE, ch, 0, 0, TO_CHAR);
			return;
		}
	}		

	if ( IS_CARRYING_N (vict) + 1 > CAN_CARRY_N (vict) ) {
		if ( CAN_CARRY_N (vict) == 0 )
			act ("$N isn't capable of carrying anyting.",
					FALSE, ch, 0, vict, TO_CHAR);
		else
			act ("$N hands are full.", 0, ch, 0, vict, TO_CHAR);
		return;
	}

	if ( OBJ_MASS (obj) + IS_CARRYING_W (vict) > CAN_CARRY_W (vict) ) {
		act ("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR);
		return;
	}

	obj_from_char (&obj, count);

	act ("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT | TO_ACT_FORMAT);
	act ("$n gives you $p.", 0, ch, obj, vict, TO_VICT | TO_ACT_FORMAT);
	act ("You give $p to $N.", 1, ch, obj, vict, TO_CHAR | TO_ACT_FORMAT);

	obj_to_char (obj, vict);

}

OBJ_DATA *get_equip (CHAR_DATA *ch, int location)
{
	OBJ_DATA	*obj = NULL;

	if ( location == WEAR_SHIELD ) {
		if ( ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_SHIELD )
			return ch->right_hand;
		if ( ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_SHIELD )
			return ch->left_hand;
		return NULL;
	}

	if ( ch->right_hand && ch->right_hand->location == location )
		return ch->right_hand;

	if ( ch->left_hand && ch->left_hand->location == location )
		return ch->left_hand;

	if ( location != WEAR_SHIELD ) {
		for ( obj = ch->equip; obj; obj = obj->next_content )
			if ( obj->location == location )
				return obj;
	}

	return NULL;
}

void do_drink (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*container = NULL;
	OBJ_DATA	*drink = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !(container = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		 !(container = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		 !(container = get_obj_in_list_vis (ch, buf, ch->room->contents)) ) {
		act ("You can't find it.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( GET_ITEM_TYPE (container) != ITEM_DRINKCON &&
		 GET_ITEM_TYPE (container) != ITEM_FOUNTAIN ) {
		act ("You cannot drink from $p.", FALSE, ch, container, 0, TO_CHAR);
		return;
	}

	if ( ch->hunger + ch->thirst > 40 ) {
		send_to_char ("You are too full to consume anything else.\n", ch);
		return;
	}

	if ( container->o.drinkcon.volume == 0 ) {
		act ("$p is empty.", FALSE, ch, container, 0, TO_CHAR);
		return;
	}

	if ( !(drink = vtoo (container->o.drinkcon.liquid)) ) {
		act ("$p appears empty.", FALSE, ch, container, 0, TO_CHAR);
		return;
	}

	act ("You drink $o from $P.", FALSE, ch, drink, container, TO_CHAR);
	act ("$n drinks $o from $P.", TRUE, ch, drink, container, TO_ROOM);

	if ( ch->intoxication != -1 ) {
		if(number(6,20) > ((ch->con + ch->wil) / 2)) 
		    ch->intoxication += drink->o.fluid.alcohol;
		else 
		    ch->intoxication += ((drink->o.fluid.alcohol)/2);
	}

	if ( ch->thirst != -1 )
		ch->thirst += drink->o.fluid.water;

	if ( ch->hunger != -1 )
		ch->hunger += drink->o.fluid.food;

	if ( ch->thirst < 0 && IS_MORTAL(ch) )
		ch->thirst = 0;
	if ( ch->hunger < 0 && IS_MORTAL(ch) )
		ch->hunger = 0;

/*
	if( ch->intoxication > 0) {
            snprintf (buf, MAX_STRING_LENGTH,  "You feel %s.\n",
                verbal_intox  [get_comestible_range(ch->intoxication)]);
            send_to_char (buf, ch);
	}
	if ( ch->intoxication > 10 )
		send_to_char ("You feel intoxicated.\n", ch);
*/

	if ( container->o.drinkcon.volume != -1 )
		container->o.drinkcon.volume--;

	if ( !container->o.drinkcon.volume ) {
		container->o.drinkcon.liquid = 0;
		container->o.drinkcon.spell_1 = 0;
		container->o.drinkcon.spell_2 = 0;
		container->o.drinkcon.spell_3 = 0;
	}
	
	return;
}

void do_eat (CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_INPUT_LENGTH];
	OBJ_DATA		*obj = NULL;

	(void)one_argument (argument, buf);

	if ( !(obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_dark (ch, buf, ch->left_hand)) &&
		 !(obj = get_obj_in_dark (ch, buf, ch->equip)) ) {
		send_to_char ("You can't find it.\n", ch);
		return;
	}

	if ( obj->obj_flags.type_flag != ITEM_FOOD && IS_MORTAL (ch) ) {
		send_to_char ("That isn't food.  You can't eat it.\n", ch);
		return;
	}

	if ( obj->equiped_by && obj->obj_flags.type_flag != ITEM_FOOD ) {
		send_to_char ("You must remove that item before destroying it.\n", ch);
		return;
	}

	if ( ch->hunger + ch->thirst > 40 ) {	
		act ("You are too full to eat!", FALSE, ch, 0, 0, TO_CHAR);
		return;
	}

	if ( obj->equiped_by )
		(void)unequip_char (ch, obj->location);

	if ( obj->o.food.bites <= 1 ) {
		act ("$n eats $p.", TRUE, ch, obj, 0, TO_ROOM);
		act ("You eat the $o.", FALSE, ch, obj, 0, TO_CHAR);
	} else {
		act ("$n eats a bit of $p.", TRUE, ch, obj, 0, TO_ROOM);
		act ("You eat a bite of $o.", FALSE, ch, obj, 0, TO_CHAR);
	}

	ch->hunger += get_bite_value (obj);

	if ( ch->hunger > 40 )
		ch->hunger = 40;

	obj->o.food.bites--;

	if ( ch->hunger > 20 )
		act ("You are full.", FALSE, ch, 0, 0, TO_CHAR);

	if ( IS_MORTAL (ch) ) {

		obj->o.food.spell1 = 0;
		obj->o.food.spell2 = 0;
		obj->o.food.spell3 = 0;
		obj->o.food.spell4 = 0;
	}

	if ( obj->count > 1 && obj->o.food.bites < 1 ) {
		obj->count--;
		obj->o.food.bites = vtoo(obj->virtual)->o.food.bites;
	}
	else if ( obj->o.food.bites < 1 && obj->count <= 1 )
		extract_obj (obj);
	else if ( !IS_MORTAL (ch) ) {
		extract_obj (obj);
		return;
	}
	return;
}

void do_fill (CHAR_DATA *ch, char *argument, int cmd)
{
	int			volume_to_transfer = 0;
	OBJ_DATA	*from = NULL;
	OBJ_DATA	*to = NULL;
	OBJ_DATA	*fuel = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("FILL <object> from/with <object>\n", ch);
		send_to_char ("Example:  fill bucket from well\n", ch);
		send_to_char ("          fill bucket well      (same thing)\n", ch);
		return;
	}

	if ( !(to = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		 !(to = get_obj_in_list_vis (ch, buf, ch->left_hand)) &&
		 !(to = get_obj_in_list_vis (ch, buf, ch->room->contents)) ) {

		if ( get_obj_in_dark (ch, buf, ch->room->contents) ) {
			send_to_char ("It is too dark for that.\n", ch);
			return;
		}

		send_to_char ("Fill what?\n", ch);

		return;
	}

	if ( GET_ITEM_TYPE (to) != ITEM_DRINKCON &&
		 GET_ITEM_TYPE (to) != ITEM_LIGHT ) {
		act ("You can't fill $p.", FALSE, ch, to, 0, TO_CHAR);	
		return;
	}

	if ( GET_ITEM_TYPE (to) == ITEM_LIGHT  &&
		 is_name_in_list ("candle", to->name) ) {
		act ("You can't do that with $p.", FALSE, ch, to, 0, TO_CHAR);
		return;
	}

	if ( to->o.drinkcon.volume == -1 ||
		 (to->o.drinkcon.volume >= to->o.drinkcon.capacity) ) {
		act ("$p is full already.", FALSE, ch, to, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "with") || !str_cmp (buf, "from") )
		argument = one_argument (argument, buf);

	if ( !*buf ) {

		for ( from = ch->room->contents; from; from = from->next_content )
			if ( GET_ITEM_TYPE (from) == ITEM_FOUNTAIN &&
				 CAN_SEE_OBJ (ch, from) )
				break;

		if ( !from ) {
			act ("Fill $p from what?", FALSE, ch, to, 0, TO_CHAR);
			return;
		}
	}

	else if ( !(from = get_obj_in_dark (ch, buf, ch->room->contents)) ) {
		act ("Fill $p from what?", FALSE, ch, to, 0, TO_CHAR);
		return;
	}

	if ( GET_ITEM_TYPE (from) != ITEM_FOUNTAIN &&
		 GET_ITEM_TYPE (from) != ITEM_DRINKCON ) {
		act ("There is no way to fill $p from $P.",
				FALSE, ch, to, from, TO_CHAR);
		return;
	}

	if ( !from->o.drinkcon.volume ) {
		act ("$p is empty.", FALSE, ch, from, 0, TO_CHAR);
		return;
	}

	if ( GET_ITEM_TYPE (to) == ITEM_LIGHT &&
		 to->o.light.liquid != from->o.drinkcon.liquid ) {

		if ( !(fuel = vtoo (to->o.light.liquid)) ) {
			act ("$p is broken.", FALSE, ch, to, 0, TO_CHAR);
			return;
		}

		act ("$p only burns $O.", FALSE, ch, to, fuel, TO_CHAR);

		if ( (fuel = vtoo (from->o.drinkcon.liquid)) )
			act ("$p contains $O.", FALSE, ch, from, fuel, TO_CHAR);

		return;
	}

	if ( to->o.drinkcon.liquid && to->o.drinkcon.volume && to->o.drinkcon.liquid != from->o.drinkcon.liquid ) {
		send_to_char ("You shouldn't mix fluids.\n", ch);
		return;
	}

	temp_arg = vnum_to_liquid_name (from->o.drinkcon.liquid);
	snprintf (buf, MAX_STRING_LENGTH,  "You fill $p from $P with %s.", temp_arg);
	act (buf, FALSE, ch, to, from, TO_CHAR | TO_ACT_FORMAT);

	temp_arg = vnum_to_liquid_name (from->o.drinkcon.liquid);
	snprintf (buf, MAX_STRING_LENGTH,  "$n fills $p from $P with %s.", temp_arg);
	act (buf, TRUE, ch, to, from, TO_ROOM | TO_ACT_FORMAT);

	if ( GET_ITEM_TYPE (to) == ITEM_DRINKCON && !to->o.drinkcon.volume )
		to->o.drinkcon.liquid = from->o.drinkcon.liquid;

	volume_to_transfer = from->o.drinkcon.volume;

	if ( from->o.drinkcon.volume == -1 )
		volume_to_transfer = to->o.drinkcon.capacity;

	if ( volume_to_transfer >
			(to->o.drinkcon.capacity - to->o.drinkcon.volume) )
		volume_to_transfer = to->o.drinkcon.capacity - to->o.drinkcon.volume;

	if ( from->o.drinkcon.volume != -1 )
		from->o.drinkcon.volume -= volume_to_transfer;

	if ( to->o.drinkcon.volume != -1 )
		to->o.drinkcon.volume += volume_to_transfer;


	if ( GET_ITEM_TYPE (from) == ITEM_LIGHT &&
		 !from->o.light.hours &&
		 from->o.light.on ) {
		act ("$p is extinguished.", FALSE, ch, from, 0, TO_CHAR);
		act ("$p is extinguished.", FALSE, ch, from, 0, TO_ROOM);
		from->o.light.on = 0;
	}
}


void do_pour (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*from = NULL;
	OBJ_DATA	*to = NULL;
	int			volume_to_transfer = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("What do you want to pour?\n", ch);
		return;
	}

	if ( !(from = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(from = get_obj_in_dark (ch, buf, ch->left_hand)) ) {
		send_to_char ("You can't find it.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (from) != ITEM_DRINKCON &&
		 GET_ITEM_TYPE (from) != ITEM_LIGHT ) {
		act ("You can't pour from $p.\n", FALSE, ch, from, 0, TO_CHAR);
		return;
	}

	if ( GET_ITEM_TYPE (from) == ITEM_LIGHT &&
		 is_name_in_list ("candle", from->name) ) {
		send_to_char ("You can't pour wax from a candle (yet).\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (from) == ITEM_LIGHT && !from->o.light.hours ) {
		act ("$p contains no fuel.", FALSE, ch, from, 0, TO_CHAR);
		return;
	}

	if ( GET_ITEM_TYPE (from) == ITEM_DRINKCON && !from->o.drinkcon.volume ) {
		act ("$p is empty.", FALSE, ch, from, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf2);

	if ( !str_cmp (buf2, "out") ) {
		do_empty (ch, buf, 0);
		return;
	}

	if ( !*buf2 ) {
		send_to_char ("What do you want to pour it into?", ch);
		return;
	}

	if ( !(to = get_obj_in_dark (ch, buf2, ch->right_hand)) &&
		 !(to = get_obj_in_list_vis (ch, buf2, ch->left_hand)) &&
		 !(to = get_obj_in_list_vis (ch, buf2, ch->room->contents)) ) {
		act ("You can't find it to pour $p into.", FALSE, ch, from, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( GET_ITEM_TYPE (to) != ITEM_DRINKCON &&
		 GET_ITEM_TYPE (to) != ITEM_LIGHT ) {
		act ("You can't pour into $p.", FALSE, ch, to, 0, TO_CHAR);
		return;
	}

	if ( GET_ITEM_TYPE (to) == ITEM_LIGHT &&
		 !to->o.light.hours &&
		 to->o.light.on )
		to->o.light.on = 0;

	if ( GET_ITEM_TYPE (to) == ITEM_LIGHT ) {

		if ( is_name_in_list ("candle", to->name) ) {
			send_to_char ("You can't pour it into a candle.", ch);
			return;
		}

		if ( from->o.light.liquid != to->o.light.liquid ) {
			temp_arg = vnum_to_liquid_name (to->o.light.liquid);
			snprintf (buf, MAX_STRING_LENGTH,  "$p only burns %s.", temp_arg);
			act (buf, FALSE, ch, to, 0, TO_CHAR);
			return;
		}
	}

	if ( to->o.drinkcon.volume &&
		 from->o.drinkcon.liquid != to->o.drinkcon.liquid ) {
		act ("If you want to fill $p with another liquid, then empty it first.",
				FALSE, ch, to, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( to->o.drinkcon.capacity == to->o.drinkcon.volume ) {
		act ("$p is already full.", FALSE, ch, to, 0, TO_CHAR);
		return;
	}
	temp_arg = vnum_to_liquid_name (from->o.drinkcon.liquid);
	snprintf (buf, MAX_STRING_LENGTH,  "You pour %s from $p into $P.", temp_arg);
	act (buf, FALSE, ch, from, to, TO_CHAR | TO_ACT_FORMAT);

	temp_arg = vnum_to_liquid_name (from->o.drinkcon.liquid);
	snprintf (buf, MAX_STRING_LENGTH,  "$n pours %s from $p into $P.", temp_arg);
	act (buf, TRUE, ch, from, to, TO_ROOM | TO_ACT_FORMAT);

	if ( GET_ITEM_TYPE (to) == ITEM_DRINKCON && !to->o.drinkcon.volume )
		to->o.drinkcon.liquid = from->o.drinkcon.liquid;

	volume_to_transfer = from->o.drinkcon.volume;
	if ( volume_to_transfer >
			(to->o.drinkcon.capacity - to->o.drinkcon.volume) )
		volume_to_transfer = to->o.drinkcon.capacity - to->o.drinkcon.volume;

	from->o.drinkcon.volume -= volume_to_transfer;

	to->o.drinkcon.volume += volume_to_transfer;

	if ( GET_ITEM_TYPE (from) == ITEM_LIGHT &&
		 !from->o.light.hours &&
		 from->o.light.on ) {
		act ("$p is extinguished.", FALSE, ch, from, 0, TO_CHAR);
		act ("$p is extinguished.", FALSE, ch, from, 0, TO_ROOM);
		from->o.light.on = 0;
	}
}

/* functions related to wear */

void perform_wear(CHAR_DATA *ch, OBJ_DATA *obj, int keyword)
{
	switch(keyword) {

		case 1 : 
			act("$n wears $p on $s head.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wear $p on your head.",TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 2:
			act ("$n wears $p in $s hair.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			act ("You wear $p in your hair.", TRUE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 3: 
			act ("$n wears $p on $s ears.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
            act ("You wear $p on your ears.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
            break;
            
            case 4:
			act ("$n wears $p over $s eyes.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			act ("You wear $p over your eyes.", TRUE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 5:
			act ("$n wears $p on $s face.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			act ("You wear $p on your face.", TRUE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 6:
			act ("$n has $p in $s mouth.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			act ("You have $p in your mouth.", TRUE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 7:
			act ("$n wears $p around $s throat.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			act ("You wear $p around your throat.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 8 : 
			act("$n wears $p around $s neck.", TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wear $p around your neck.", TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 9 : 
			act("$n wears $p on $s body.", TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wear $p on your body.", TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 10 : 
			act("$n wears $p about $s body.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wear $p about your body.",TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 11 :
			act ("$n stores $p across $s back.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			act ("You store $p across your back.", TRUE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 12:
			act("$n slings $p over $s shoulder.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			break;
			
			case 13 : 
			act("$n wears $p on $s arms.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wear $p on your arms.",TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 14 : 
			act("$n wears $p around $s wrist.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			break;
			
			case 15 : 
			act("$n wears $p on $s hands.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wear $p on your hands.",TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 16 : 
			act("$n wears $p on $s finger.", TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			break;
			
			case 17 : 
			act("$n wears $p about $s waist.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wear $p about your waist.",TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 18 : 	/*  pouches */
			act ("$n affixes $p to $s belt.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			act ("You affix $p to your belt.",TRUE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 19 : 
			act("$n wears $p on $s legs.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wear $p on your legs.",TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 20 : 
			act("$n wears $p on $s feet.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wear $p on your feet.",TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 21 : 
			act("$n wields $p.",TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You wield $p.",TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
		
		case 22 : 
			act("$n starts using $p.", TRUE, ch, obj,0,TO_ROOM | TO_ACT_FORMAT);
			act("You start using $p.",TRUE, ch, obj,0,TO_CHAR | TO_ACT_FORMAT);
			break;
			
			case 23 :	/*  sheathe */
			act ("$n affixes $p to $s belt.", TRUE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
			act ("You affix $p to your belt.",TRUE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			break;
			
			
			
	}
}
void wear (CHAR_DATA *ch, OBJ_DATA *obj_object, int keyword)
{
	char		buffer [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	OBJ_DATA	*temp_obj = NULL;

	if ( IS_SET (obj_object->obj_flags.extra_flags, ITEM_MOUNT) &&
		 !IS_SET (ch->act, ACT_MOUNT) ) {
		send_to_char ("That item is for animals.\n", ch);
		return;
	}

	if ( !IS_SET (obj_object->obj_flags.extra_flags, ITEM_MOUNT) &&
		 IS_SET (ch->act, ACT_MOUNT) ) {
		send_to_char ("You are an animal.  That item isn't for you.\n", ch);
		return;
	}

	if ( obj_object->loaded && GET_ITEM_TYPE (obj_object) == ITEM_WEAPON &&
		obj_object->o.weapon.use_skill != SKILL_CROSSBOW ) {
		send_to_char ("You'll need to unload that, first.\n", ch);
		return;
	}

	
		
	switch ( keyword ) {

/** WEAR_HEAD keyword = 1 **/
		case 1:
			if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) {
				if ( get_equip (ch, WEAR_HEAD) )
					send_to_char ("You already wear something on your head."
								  "\n", ch);
				else {
					perform_wear(ch,obj_object,keyword);
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					equip_char(ch, obj_object, WEAR_HEAD);
				}
			} 
			else
				send_to_char("You can't wear that on your head.\n", ch);
				
			break;

/** WEAR HAIR keyword = 2 **/			
		case 2:
			if ( !CAN_WEAR (obj_object, ITEM_WEAR_HAIR) )
                send_to_char ("You cannot wear that in your hair.\n", ch);
			
			else if ( get_equip (ch, WEAR_HAIR) ){
				temp_obj = get_equip (ch, WEAR_HAIR);
            	act ("You are already wearing $p in your hair.",
                	  FALSE, ch, temp_obj, 0, TO_CHAR);
			}
			
			else {
            	perform_wear (ch, obj_object, keyword);
				if ( obj_object == ch->right_hand )
					ch->right_hand = NULL;
				else if ( obj_object == ch->left_hand )
					ch->left_hand = NULL;
                equip_char (ch, obj_object, WEAR_HAIR);
            }

            break;
                        
/** WEAR EAR keyword = 3 **/
		case 3:
			if ( !CAN_WEAR (obj_object, ITEM_WEAR_EAR) )
                send_to_char ("You cannot wear that on your ears.\n", ch);
			
			else if ( get_equip (ch, WEAR_EAR) ){
				temp_obj = get_equip (ch, WEAR_EAR);
            	act ("You are already wearing $p on your ears.",
                	FALSE, ch, temp_obj, 0, TO_CHAR);
			}
			
            else {
            	perform_wear (ch, obj_object, keyword);
				if ( obj_object == ch->right_hand )
					ch->right_hand = NULL;
				else if ( obj_object == ch->left_hand )
					ch->left_hand = NULL;
                equip_char (ch, obj_object, WEAR_EAR);
            }

            break;

/** WEAR EYE keyword = 4 **/
		case 4:
			if ( !CAN_WEAR (obj_object, ITEM_WEAR_EYE) )
				send_to_char ("You cannot wear that over your eyes.\n", ch);

			else if ( get_equip (ch, WEAR_EYE) )
				send_to_char ("Something already covers your eyes.\n", ch);

			else {
				perform_wear (ch, obj_object, keyword);
				if ( obj_object == ch->right_hand )
					ch->right_hand = NULL;
				else if ( obj_object == ch->left_hand )
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_EYE);
			}

			break;
			
/** WEAR FACE keyword = 5**/			
		case 5:
			if ( !CAN_WEAR (obj_object, ITEM_WEAR_FACE) )
            	send_to_char ("You cannot wear that on your face.\n", ch);

            else if ( get_equip (ch, WEAR_FACE) ){
            	temp_obj = get_equip (ch, WEAR_FACE);
            	act ("You are already wearing $p on your face.",
                	FALSE, ch, temp_obj, 0, TO_CHAR);
				}
				
            else {
            	perform_wear (ch, obj_object, keyword);
				if ( obj_object == ch->right_hand )
					ch->right_hand = NULL;
				else if ( obj_object == ch->left_hand )
					ch->left_hand = NULL;
                equip_char (ch, obj_object, WEAR_FACE);
            }

            break;
                        
/** WEAR MOUTH keyword = 6**/			
		case 6:
			if ( !CAN_WEAR (obj_object, ITEM_WEAR_MOUTH) )
            	send_to_char ("You cannot carry that in your mouth.\n", ch);

            else if ( get_equip (ch, WEAR_MOUTH) ){
            	temp_obj = get_equip (ch, WEAR_MOUTH);
            	act ("You are already carrying $p in your mouth.",
                	FALSE, ch, temp_obj, 0, TO_CHAR);
				}
				
           	else {
           		perform_wear (ch, obj_object, keyword);
				if ( obj_object == ch->right_hand )
					ch->right_hand = NULL;
				else if ( obj_object == ch->left_hand )
					ch->left_hand = NULL;
                equip_char (ch, obj_object, WEAR_MOUTH);
            }

            break;
                        
/** WEAR THROAT keyword = 7**/                        
		case 7:
			if ( !CAN_WEAR (obj_object, ITEM_WEAR_THROAT) )
				send_to_char ("You cannot wear that around your throat.\n", ch);

			else if ( get_equip (ch, WEAR_THROAT) ){
				temp_obj = get_equip (ch, WEAR_THROAT);
				act ("You are already wearing $p around your throat.",
						FALSE, ch, temp_obj, 0, TO_CHAR);
			}
			
			else {
				perform_wear (ch, obj_object, keyword);
				if ( obj_object == ch->right_hand )
					ch->right_hand = NULL;
				else if ( obj_object == ch->left_hand )
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_THROAT);
			}

			break;

/** WEAR NECK keyword = 8 **/      		
		case 8:
			if ( CAN_WEAR (obj_object, ITEM_WEAR_NECK) ) {
				if ( get_equip (ch, WEAR_NECK_1) &&
					 get_equip (ch, WEAR_NECK_2) )
					send_to_char ("You can only wear two things around your "
								  "neck.\n", ch);
				else {
					perform_wear(ch,obj_object,keyword);
					if ( get_equip (ch, WEAR_NECK_1) ) {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char(ch, obj_object, WEAR_NECK_2);
					} else {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char(ch, obj_object, WEAR_NECK_1);
					}
				}
			} 
			
			else
				send_to_char("You can't wear that around your neck.\n", ch);
				
			break;
                        
/** WEAR BODY keyword = 9 **/      			
		case 9:
				if ( CAN_WEAR (obj_object, ITEM_WEAR_BODY) ) {
				if ( get_equip (ch, WEAR_BODY_1) &&
					 get_equip (ch, WEAR_BODY_2) &&
					 get_equip (ch, WEAR_BODY_3) &&
					 get_equip (ch, WEAR_BODY_4) &&
					 get_equip (ch, WEAR_BODY_5) ){
					send_to_char ("You can only wear five things around your "
								  "body.\n", ch);
				}
				else if ( obj_object->size && obj_object->size != get_size (ch) ) {
					if ( obj_object->size > get_size (ch) ) {
						act ("$p won't fit, it is too large.",
								TRUE, ch, obj_object, 0, TO_CHAR);
					}
					else {
						act ("$p won't fit, it is too small.",
								TRUE, ch, obj_object, 0, TO_CHAR);
					}
				}
				else {
					perform_wear(ch, obj_object, keyword);
					if ( get_equip (ch, WEAR_BODY_1) &&
					 	 get_equip (ch, WEAR_BODY_2) &&
					 	 get_equip (ch, WEAR_BODY_3) &&
					 	 get_equip (ch, WEAR_BODY_4)) {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char(ch, obj_object, WEAR_BODY_5);
					}	
					else if ( get_equip (ch, WEAR_BODY_1) &&
					 		  get_equip (ch, WEAR_BODY_2) &&
					 		  get_equip (ch, WEAR_BODY_3)) {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char(ch, obj_object, WEAR_BODY_4);
					}
					
					else if ( get_equip (ch, WEAR_BODY_1) &&
					 		  get_equip (ch, WEAR_BODY_2)) {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char(ch, obj_object, WEAR_BODY_3);
					}
					
					else if ( get_equip (ch, WEAR_BODY_1) ) {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char(ch, obj_object, WEAR_BODY_2);
					}
					
					else {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char(ch, obj_object, WEAR_BODY_1);
					}
				}
			} 
			
			
			else
				send_to_char("You can't wear that around your body.\n", ch);
				
			break;
                        
/** WEAR ABOUT keyword = 10 **/      		
		case 10:
			if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) {
				if ( get_equip (ch, WEAR_ABOUT) ) {
					send_to_char("You already wear something about your body.\n", ch);
				}
				
				else {
					perform_wear(ch,obj_object,keyword);
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					equip_char(ch, obj_object, WEAR_ABOUT);
				}
				
			} 
			
			else
				send_to_char("You can't wear that about your body.\n", ch);
	
			break;
                        
/** WEAR BACK keyword = 11 **/      
		case 11:
			if ( !CAN_WEAR (obj_object, ITEM_WEAR_BACK) )
				send_to_char ("You cannot wear that across your back.\n", ch);

			else if ( get_equip (ch, WEAR_BACK) )
				send_to_char ("You are already wearing something there.\n", ch);

			else {
				perform_wear (ch, obj_object, keyword);
				if ( obj_object == ch->right_hand )
					ch->right_hand = NULL;
				else if ( obj_object == ch->left_hand )
					ch->left_hand = NULL;
				equip_char (ch, obj_object, WEAR_BACK);
			}
			
			break;
                        
/** WEAR SHOULDER keyword = 12*/      
		case 12: 
			if (CAN_WEAR(obj_object,ITEM_WEAR_SHOULDER)) {
				if ( get_equip (ch, WEAR_SHOULDER_L) &&
					 get_equip (ch, WEAR_SHOULDER_R) ) {
					send_to_char("You already wear something on both shoulders.\n", ch);
				} 
				
				else {
					perform_wear(ch,obj_object,keyword);
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					if ( get_equip (ch, WEAR_SHOULDER_L) ) {
						temp_arg = fname(obj_object->name);
						snprintf (buffer, MAX_STRING_LENGTH, "You sling the %s over your right shoulder.\n", temp_arg);
						send_to_char(buffer, ch);
						equip_char(ch,  obj_object, WEAR_SHOULDER_R);
					}
					
					else {
						temp_arg = fname(obj_object->name);
						snprintf (buffer, MAX_STRING_LENGTH, "You sling the %s over your left shoulder.\n", temp_arg);
						send_to_char(buffer, ch);
						equip_char(ch, obj_object, WEAR_SHOULDER_L);
					}
				}
			} 
			
			else 
				send_to_char("You can't wear that on your shoulder.\n", ch);
					
			break;
		

/** WEAR ARMS keyword = 13 **/      			
		case 13:
			if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) {
				if ( get_equip (ch, WEAR_ARMS) )
					send_to_char ("You already wear something on your arms."
								  "\n", ch);
				else if ( obj_object->size &&
						  obj_object->size != get_size (ch) ) {
					if ( obj_object->size > get_size (ch) )
						act ("$p won't fit, it is too large.",
									TRUE, ch, obj_object, 0, TO_CHAR);
					else
						act ("$p won't fit, it is too small.",
									TRUE, ch, obj_object, 0, TO_CHAR);
				}

				else {
					perform_wear(ch,obj_object,keyword);
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					equip_char(ch, obj_object, WEAR_ARMS);
				}
			} else
				send_to_char("You can't wear that on your arms.\n", ch);
				
			break;
                        
/** WEAR WRIST keyword = 14 **/      
		case 14: 
			if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) {
				if ( get_equip (ch, WEAR_WRIST_L) &&
					 get_equip (ch, WEAR_WRIST_R) ) {
					send_to_char(
						"You already wear something around both your wrists.\n", ch);
				} 
				
				else {
					perform_wear(ch,obj_object,keyword);
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					if ( get_equip (ch, WEAR_WRIST_L) ) {
						temp_arg = fname(obj_object->name);
						snprintf (buffer, MAX_STRING_LENGTH, "You wear the %s around your right wrist.\n", temp_arg);
						send_to_char(buffer, ch);
						equip_char(ch,  obj_object, WEAR_WRIST_R);
					}
					
					else {
						temp_arg = fname(obj_object->name);
						snprintf (buffer, MAX_STRING_LENGTH, "You wear the %s around your left wrist.\n", temp_arg);
						send_to_char(buffer, ch);
						equip_char(ch, obj_object, WEAR_WRIST_L);
					}
				}
			} 
			
			else 
				send_to_char("You can't wear that around your wrist.\n", ch);
			
			break;


/** WEAR HANDS keyword = 15 **/
		case 15:
			if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) {
				if ( get_equip (ch, WEAR_HANDS) )
					send_to_char ("You already wear something on your hands."
								  "\n", ch);
				else {
					perform_wear(ch,obj_object,keyword);
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					equip_char(ch, obj_object, WEAR_HANDS);

				}
			} 
			else
				send_to_char("You can't wear that on your hands.\n", ch);
				
			break;
                      
/** WEAR FINGER keyword = 16 **/      
		case 16:
			if ( CAN_WEAR (obj_object, ITEM_WEAR_FINGER) ) {

				if ( get_equip (ch, WEAR_FINGER_L) &&
					 get_equip (ch, WEAR_FINGER_R) )
					send_to_char("You are already wearing something on your "
								 "ring fingers.\n", ch);

				else {
					perform_wear(ch,obj_object,keyword);
					if ( get_equip (ch, WEAR_FINGER_L) ) {
						temp_arg = fname(obj_object->name);
						snprintf (buffer, MAX_STRING_LENGTH, "You slip the %s on your right ring finger.\n", temp_arg);
						send_to_char(buffer, ch);
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
						equip_char (ch, obj_object, WEAR_FINGER_R);
					}

					else {
						temp_arg = fname(obj_object->name);
						snprintf (buffer, MAX_STRING_LENGTH, "You slip the %s on your left ring finger.\n", temp_arg);
						send_to_char(buffer, ch);
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char (ch, obj_object, WEAR_FINGER_L);
					}
				}
			} 
			
			else
				send_to_char("You can't wear that on your ring finger.\n", ch);
				
			break;
                        
/** WEAR WAIST keyword = 17 **/      
		case 17: 
			if (CAN_WEAR(obj_object,ITEM_WEAR_WAIST)) {
				if ( get_equip (ch, WEAR_WAIST) ) {
					send_to_char("You already wear something about your waist.\n", ch);
				} 
				
				else {
					perform_wear(ch,obj_object,keyword);
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					equip_char(ch,  obj_object, WEAR_WAIST);
				}
			} 
			
			else 
				send_to_char("You can't wear that about your waist.\n", ch);
			
			break;
                        
/** WEAR BELT keyword = 18 **/      
		case 18:
			if ( !CAN_WEAR (obj_object, ITEM_WEAR_BELT) )
				send_to_char ("You cannot wear that on your belt.\n", ch);

			else if ( !get_equip (ch, WEAR_WAIST) )
				send_to_char ("You need a belt to wear that.\n", ch);

			else if ( get_equip (ch, WEAR_BELT_1) &&
					  get_equip (ch, WEAR_BELT_2) )
				send_to_char ("Your belt is full.\n", ch);

			else {
				int		belt_loc;

				/* Mostly I expect pouches to be equiped here.
				   put them in the second belt loc first */

				if ( !get_equip (ch, WEAR_BELT_2) )
					belt_loc = WEAR_BELT_2;
				else
					belt_loc = WEAR_BELT_1;

				perform_wear (ch, obj_object, keyword);
				if ( obj_object == ch->right_hand )
					ch->right_hand = NULL;
				else if ( obj_object == ch->left_hand )
					ch->left_hand = NULL;
				equip_char (ch, obj_object, belt_loc);
			}
			break;

/** WEAR LEGS keyword = 19 **/      		
		case 19:
			if ( CAN_WEAR (obj_object, ITEM_WEAR_LEGS) ) {

				if ( get_equip (ch, WEAR_LEGS_1) &&
					 get_equip (ch, WEAR_LEGS_2) &&
					 get_equip (ch, WEAR_LEGS_3))
					send_to_char("You are already wearing something on your "
								 "legs.\n", ch);
								 
				else if ( obj_object->size &&
						  obj_object->size != get_size (ch) ) {
					if ( obj_object->size > get_size (ch) )
						act ("$p won't fit, it is too large.",
							TRUE, ch, obj_object, 0, TO_CHAR);
					else
						act ("$p won't fit, it is too small.",
							TRUE, ch, obj_object, 0, TO_CHAR);
				}

				else {
					perform_wear(ch,obj_object,keyword);
					
					if ( get_equip (ch, WEAR_LEGS_1) &&
						 get_equip (ch, WEAR_LEGS_2)) {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char (ch, obj_object, WEAR_LEGS_3);
					}

					else {
						if ( get_equip (ch, WEAR_LEGS_1)){
							if ( obj_object == ch->right_hand )
								ch->right_hand = NULL;
							else if ( obj_object == ch->left_hand )
								ch->left_hand = NULL;
							equip_char (ch, obj_object, WEAR_LEGS_2);
						}
					}
					
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_LEGS_1);
				}
			} 
			
			else
				send_to_char("You can't wear that on your legs.\n", ch);
			
			break;
/** WEAR FEET keyword = 20 **/      
		case 20:
			if ( CAN_WEAR (obj_object, ITEM_WEAR_FEET) ) {

				if ( get_equip (ch, WEAR_FEET_1) &&
					 get_equip (ch, WEAR_FEET_2) )
					send_to_char("You are already wearing something on your "
								 "feet.\n", ch);
								 
				else if ( obj_object->size &&
						  obj_object->size != get_size (ch) ) {
					if ( obj_object->size > get_size (ch) )
						act ("$p won't fit, it is too large.",
							TRUE, ch, obj_object, 0, TO_CHAR);
					else
						act ("$p won't fit, it is too small.",
							TRUE, ch, obj_object, 0, TO_CHAR);
				}

				else {
					perform_wear(ch,obj_object,keyword);
					if ( get_equip (ch, WEAR_FEET_1) ) {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char (ch, obj_object, WEAR_FEET_2);
					}

					else {
						if ( obj_object == ch->right_hand )
							ch->right_hand = NULL;
						else if ( obj_object == ch->left_hand )
							ch->left_hand = NULL;
						equip_char (ch, obj_object, WEAR_FEET_1);
					}
				}
			} 
			
			else
				send_to_char("You can't wear that on your feet.\n", ch);
			
			break;

/** WEAR WIELD keyword = 21 **/
		case 21:
			if (CAN_WEAR(obj_object, ITEM_WIELD)) {
/*
				if ( !can_handle (obj_object, ch) ) {
					send_to_char ("Your hands are occupied!\n", ch);
					return;
				}
*/
				switch(obj_object->o.od.value[3]) {
					case SKILL_MEDIUM_EDGE:						/*  Medium weapons. */
					case SKILL_MEDIUM_BLUNT:
					case SKILL_MEDIUM_PIERCE:
					case SKILL_LIGHT_EDGE:						/*  Light weapons. */
					case SKILL_LIGHT_BLUNT:
					case SKILL_LIGHT_PIERCE:
					case SKILL_SLING:					
						if ( get_equip (ch, WEAR_PRIM) &&
							 get_equip (ch, WEAR_SEC) ) {
							send_to_char("You are already wielding both a primary and a secondary weapon.\n",ch);
							return;
						}
						if ( get_equip (ch, WEAR_BOTH) ) {
							send_to_char ("You are already wielding a two-handed weapon.\n", ch);
							return;
						}
						send_to_char("OK.\n", ch);
						perform_wear(ch,obj_object,keyword);
						if( !get_equip (ch, WEAR_PRIM) )
							equip_char(ch, obj_object, WEAR_PRIM);
						else
							equip_char(ch, obj_object, WEAR_SEC);
						break;
						
					case SKILL_BRAWLING:
					case SKILL_CROSSBOW:					/*  Everybody uses two hands for ranged/brawling weapons. */
					case SKILL_SHORTBOW:
					case SKILL_LONGBOW:
					case SKILL_POLEARM:
					case SKILL_STAFF:
						if ( get_equip (ch, WEAR_BOTH) ||
							 get_equip (ch, WEAR_PRIM) ||
							 get_equip (ch, WEAR_SEC) ||
							 (ch->right_hand && ch->left_hand) ) {
							send_to_char("You need both hands to wield this weapon.\n",ch);
							return;
						}
						send_to_char("OK.\n", ch);
						perform_wear(ch,obj_object,keyword);
						equip_char(ch, obj_object, WEAR_BOTH);
						break;

					case SKILL_HEAVY_EDGE:						/*  Heavy weapons. */
					case SKILL_HEAVY_BLUNT:
					case SKILL_HEAVY_PIERCE:
						if ( ch->str >= 20 ) {				/*  Extremely strong chars can wield two-handed weapons with one hand. */
							if ( get_equip (ch, WEAR_PRIM) ) {
								send_to_char("You are already wielding a primary weapon.\n", ch);
								return;
							} 
							else if ( get_equip (ch, WEAR_BOTH) ) {
								send_to_char("You are already wielding a two-handed weapon.\n",ch);
								return;
							} 
							else {
								send_to_char("OK.\n", ch);
								perform_wear(ch,obj_object,keyword);
								if ( get_equip (ch, WEAR_SEC) || (ch->right_hand && ch->left_hand) )
									equip_char(ch, obj_object, WEAR_PRIM);
								else
									equip_char(ch, obj_object, WEAR_BOTH);
							}
							break;
						}
						if ( get_equip (ch, WEAR_BOTH) ||
							 get_equip (ch, WEAR_PRIM) ||
							 get_equip (ch, WEAR_SEC) ||
							 (ch->right_hand && ch->left_hand) ) {
							send_to_char("You need both hands to wield this weapon.\n",ch);
							return;
						}
						send_to_char("OK.\n", ch);
						perform_wear(ch,obj_object,keyword);
						equip_char(ch, obj_object, WEAR_BOTH);
						break;
				}

			} else {
				send_to_char("You can't wield that.\n", ch);
			}
			break;

/** WEAR SHIELD keyword = 22 **/
		case 22: 
			if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) {
				if ( get_equip (ch, WEAR_SHIELD) )
					send_to_char ("You are already using a shield.\n", ch);

				else if ( !can_handle (obj_object, ch) )
					send_to_char ("Your hands are occupied.\n", ch);

				else {
					perform_wear(ch,obj_object,keyword);
					temp_arg = fname(obj_object->name);
					snprintf (buffer, MAX_STRING_LENGTH, "You start using the %s.\n",temp_arg);
					send_to_char(buffer, ch);
					equip_char(ch, obj_object, WEAR_SHIELD);
				}
			} 
			else 
				send_to_char("You can't use that as a shield.\n", ch);
				
			break;
			
/** WEAR SHEATHE keyword = 23 **/      
		case 23:
					
			if ( !CAN_WEAR (obj_object, ITEM_WEAR_BELT) )
				send_to_char ("You cannot wear that on your belt.\n", ch);

			else if ( !get_equip (ch, WEAR_WAIST) )
				send_to_char ("You need a belt to wear that.\n", ch);

			else if ( get_equip (ch, WEAR_BELT_1) &&
					  get_equip (ch, WEAR_BELT_2) )
				send_to_char ("Your belt is full.\n", ch);

			else {
				perform_wear(ch,obj_object,keyword);				
				if ( get_equip (ch, WEAR_BELT_2)){
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_BELT_1);
				}
				else {
					perform_wear (ch, obj_object, keyword);
					if ( obj_object == ch->right_hand )
						ch->right_hand = NULL;
					else if ( obj_object == ch->left_hand )
						ch->left_hand = NULL;
					equip_char (ch, obj_object, WEAR_BELT_2);
				}
			}
			break;
                       

/** special cases **/
		case -1: 
			temp_arg = fname(obj_object->name);
			snprintf (buffer, MAX_STRING_LENGTH,"Wear %s where?.\n", temp_arg);
			send_to_char(buffer, ch);
			
			break;
			
		case -2: 
			temp_arg = fname(obj_object->name);
			snprintf (buffer, MAX_STRING_LENGTH,"You can't wear the %s.\n", temp_arg);
			send_to_char(buffer, ch);
		
			break;
		
		default: 
			snprintf (buffer, MAX_STRING_LENGTH, "Unknown type called in wear, obj VNUM %d.", obj_object->virtual);
			system_log(buffer, TRUE);
		 
		 	break;
	}	 /*  end switch */
	return;
}

void do_wear (CHAR_DATA *ch, char *argument, int cmd)
{
	char 		arg1[MAX_STRING_LENGTH] = {'\0'};
	char 		arg2[MAX_STRING_LENGTH] = {'\0'};
	char 		buf[256] = {'\0'};
	char 		buffer[MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA 	*obj_object = NULL;
	int 		keyword = 0;
	

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if (*arg1) {
		if ( !cmp_strn (arg1, ".c", 2) ) {
			obj_object = get_obj_in_list_id (strtol(arg2, NULL, 10), ch->right_hand);
			if ( !obj_object )
				obj_object = get_obj_in_list_id (strtol(arg2, NULL, 10), ch->left_hand);
		}
		else {
			obj_object = get_obj_in_dark (ch, arg1, ch->right_hand);
			if ( !obj_object )
				obj_object = get_obj_in_dark (ch, arg1, ch->left_hand);
		}
		if (obj_object) {
			if (*arg2 && !isdigit (*arg2) ) {
				keyword = search_block(arg2, locations, FALSE); /* Partial Match */
				if (keyword == -1) {
					snprintf (buf, MAX_STRING_LENGTH,  "%s is an unknown body location.\n", arg2);
					send_to_char(buf, ch);
				} 
				
				else {
					wear(ch, obj_object, keyword+1);
				}
				
			} 
			
			else {
				keyword = -2;
/***The list defaults to the last location listed, so wield and sheathe need to be removed from this list. Also shieled is moved to the top, incase there is another location for the object ***/
				if (CAN_WEAR (obj_object, ITEM_WEAR_SHIELD)) keyword = 22;
				if (CAN_WEAR (obj_object, ITEM_WEAR_HEAD)) keyword = 1;
				if (CAN_WEAR (obj_object, ITEM_WEAR_HAIR)) keyword = 2;
				if (CAN_WEAR (obj_object, ITEM_WEAR_EAR)) keyword = 3;
				if (CAN_WEAR (obj_object, ITEM_WEAR_EYE)) keyword = 4;
				if (CAN_WEAR (obj_object, ITEM_WEAR_FACE)) keyword = 5;
				if (CAN_WEAR (obj_object, ITEM_WEAR_MOUTH)) keyword = 6;
				if (CAN_WEAR (obj_object, ITEM_WEAR_THROAT)) keyword = 7;
				if (CAN_WEAR (obj_object, ITEM_WEAR_NECK)) keyword = 8;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BODY)) keyword = 9;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ABOUT)) keyword = 10;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BACK)) keyword = 11;
				if (CAN_WEAR (obj_object, ITEM_WEAR_SHOULDER)) keyword = 12;
				if (CAN_WEAR (obj_object, ITEM_WEAR_ARMS)) keyword = 13;
				if (CAN_WEAR (obj_object, ITEM_WEAR_WRIST)) keyword = 14;
				if (CAN_WEAR (obj_object, ITEM_WEAR_HANDS)) keyword = 15;
				if (CAN_WEAR (obj_object, ITEM_WEAR_FINGER)) keyword = 16;
				if (CAN_WEAR (obj_object, ITEM_WEAR_WAIST)) keyword = 17;
				if (CAN_WEAR (obj_object, ITEM_WEAR_BELT)) keyword = 18;
				if (CAN_WEAR (obj_object, ITEM_WEAR_LEGS)) keyword = 19;
				if (CAN_WEAR (obj_object, ITEM_WEAR_FEET)) keyword = 20;
				/* if (CAN_WEAR (obj_object, ITEM_WIELD)) keyword = 21; */
				/* if (CAN_WEAR (obj_object, ITEM_WEAR_SHEATH)) keyword = 23; */

				if ( obj_object->activation &&
		 			IS_SET (obj_object->obj_flags.extra_flags, ITEM_WEAR_AFFECT) )
					obj_activate (ch, obj_object);

				wear (ch, obj_object, keyword);
			}
		} else {
			snprintf (buffer, MAX_STRING_LENGTH,  "You do not seem to have the '%s'.\n",arg1);
			send_to_char(buffer,ch);
		}
	} else {
		send_to_char("Wear what?\n", ch);
	}
}

void do_wield (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA		*obj_object = NULL;
	int				keyword = 21; /*  see do_wear */
	SECOND_AFFECT	*sa = NULL;
	char			buffer [MAX_STRING_LENGTH] = {'\0'};
	char			arg1 [MAX_STRING_LENGTH] = {'\0'};
	char			arg2 [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if ( !*arg1 ) {
		send_to_char ("Wield what?\n", ch);
		return;
	}

	if ( !str_cmp (arg1, ".c") ) {
		obj_object = get_obj_in_list_id (strtol(arg2, NULL, 10), ch->right_hand);
		if ( !obj_object )
			obj_object = get_obj_in_list_id (strtol(arg2, NULL, 10), ch->left_hand);
	}
	else {
		obj_object = get_obj_in_dark (ch, arg1, ch->right_hand);
		if ( !obj_object )
			obj_object = get_obj_in_dark (ch, arg1, ch->left_hand);
	}

	if ( obj_object ) {
		if ( obj_object->location == WEAR_PRIM || obj_object->location == WEAR_SEC ||
			obj_object->location == WEAR_BOTH ) {
			send_to_char ("You're already wielding that!\n", ch);
			return;
		}

		if ( (sa = get_second_affect (ch, SPA_WEAR_OBJ, obj_object)) )
			return;

		if ( obj_object->activation &&
 			IS_SET (obj_object->obj_flags.extra_flags, ITEM_WIELD_AFFECT) )
			obj_activate (ch, obj_object);

		wear (ch, obj_object, keyword);
	}

	else {
		snprintf (buffer, MAX_STRING_LENGTH,  "You do not seem to have the '%s'.\n",arg1);
		send_to_char(buffer,ch);
	}
}

void do_remove (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*arrow = NULL;
	char		arg1 [MAX_STRING_LENGTH] = {'\0'};
	char		arg2 [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		buffer [MAX_STRING_LENGTH] = {'\0'};
	char		location [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*tch = NULL;
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*eq = NULL;
	OBJ_DATA	*temp_obj = NULL;
	LODGED_OBJECT_INFO	*lodged = NULL;
	int			removed = 0;
	int			target_found = 0;
	int			target_obj = 0;
	int			target_char = 0;
	char		*temp_arg = NULL;
	char		*temp_targ = NULL;
	char		*temp_wound = NULL;
	
	if ( IS_MORTAL (ch) && IS_SET (ch->room->room_flags, OOC) ) {
		send_to_char ("This command has been disabled in OOC zones.\n", ch);
		return;
	}

	argument = one_argument (argument, arg1);

	if ( !*arg1 ) {
		send_to_char ("Remove what?\n", ch);
		return;
	}

	if ( IS_SET (ch->act, ACT_MOUNT) ) {
		send_to_char ("Mounts can't use this command.\n", ch);
		return;
	}

	for ( obj = ch->equip; obj; obj = obj->next_content )
		if ( IS_OBJ_VIS (ch, obj) && name_is (arg1, obj->name) )
			break;

	if ( !obj ) {
		if ( ch->right_hand ) {
			if ( name_is (arg1, ch->right_hand->name) )
				obj = ch->right_hand;
		}
		if ( ch->left_hand ) {
			if ( name_is (arg1, ch->left_hand->name) )
				obj = ch->left_hand;
		}
	}

       	if ( (get_equip (ch, WEAR_BOTH) && obj != get_equip (ch, WEAR_BOTH)) || ((ch->right_hand && obj != ch->right_hand) && (ch->left_hand && obj != ch->left_hand)) ) {
		send_to_char ("Your hands are otherwise occupied, at the moment.\n", ch);
               	return;
       	}

	if ( !obj && *arg1 ) {
		if ( (tch = get_char_room_vis (ch, arg1)) ) {
			target_found++;
			target_char++;
		}
		for ( obj = ch->room->contents; obj; obj = obj->next_content ) {
			if ( IS_OBJ_VIS (ch, obj) && name_is (arg1, obj->name) ) {
				target_found++;
				target_obj++;
				break;
			}
		}
		if ( !tch && !obj ) {
			send_to_char ("Remove what?\n", ch);
			return;
		}
		if ( !target_found ) {
			tch = ch;
			snprintf (arg2, MAX_STRING_LENGTH, "%s", arg1);
			target_found++;
		}
		else {
			(void)one_argument (argument, arg2);
		}
		if ( !*arg2 ) {
			send_to_char ("Remove what?\n", ch);
			return;
		}
		if ( target_char ) {
			if ( GET_POS (tch) > POSITION_RESTING && IS_MORTAL (ch) ) {
				send_to_char ("The target must be resting before you can remove a lodged item.\n", ch);
				return;
			} 
			if ( tch->lodged && name_is (arg2, vtoo(tch->lodged->vnum)->name) ) {
				snprintf (location, MAX_STRING_LENGTH, "%s", tch->lodged->location);

				obj = load_object(tch->lodged->vnum);
				obj->count = 1;
				obj_to_char (obj, ch);
				lodge_from_char (tch, tch->lodged);
				removed++;
			}
			else for ( lodged = tch->lodged; lodged; lodged = lodged->next ) {
				if ( !lodged->next )
					break;
				if ( name_is (arg2, vtoo(lodged->next->vnum)->name) ) {
					snprintf (location, MAX_STRING_LENGTH, "%s", lodged->next->location);

					obj = load_object(lodged->next->vnum);
					obj->count = 1;
					obj_to_char (obj, ch);
					lodge_from_char (tch, lodged);
					removed++;
					break;
				}
			}	
			if ( removed && tch == ch ) {
				*buf = '\0';
				*buf2 = '\0';
				temp_wound = expand_wound_loc(location);
				snprintf (buf, MAX_STRING_LENGTH,  "You carefully work #2%s#0 loose, wincing in pain as removing it opens the wound on your %s anew.", obj->short_description, temp_wound);
				
				temp_arg = char_short(ch);
				temp_wound = expand_wound_loc(location);
				snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 carefully works #2%s#0 loose, wincing in pain as removing it opens the wound on %s %s anew.", temp_arg, obj->short_description, HSHR(ch), temp_wound);
				*buf2 = toupper(*buf2);
				snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf2);
				snprintf (buf2, MAX_STRING_LENGTH,  "%s", buffer);
				act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				act (buf2, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
				wound_to_char (tch, location, dice(obj->o.od.value[0], obj->o.od.value[1]), 0, number(1,4),0,0);
				return;
			}
			else if ( removed && tch != ch ) {
				temp_arg = char_short(tch);
				temp_wound = expand_wound_loc(location);
				snprintf (buf, MAX_STRING_LENGTH,  "You carefully work #2%s#0 loose, wincing as removing it opens the wound on %s's %s anew.", obj->short_description, temp_arg, temp_wound );
				
				temp_arg = char_short(ch);
				temp_targ = char_short(tch);
				temp_wound = expand_wound_loc(location);
				snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 carefully works #2%s#0 loose, wincing as removing it opens the wound on %s's %s anew.", temp_arg, obj->short_description, temp_targ, temp_wound);
				*buf2 = toupper(*buf2);
				snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf2);
				snprintf (buf2, MAX_STRING_LENGTH,  "%s", buffer);
				act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
				act (buf2, FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
				
				temp_arg = char_short(ch);
				temp_wound = expand_wound_loc(location);
				snprintf (buf, MAX_STRING_LENGTH,  "%s#0 carefully works #2%s#0 loose, wincing as removing it opens the wound on your %s anew.", temp_arg, obj->short_description, temp_wound);
				*buf = toupper (*buf);
				snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
				snprintf (buf, MAX_STRING_LENGTH,  "%s", buffer);
				act (buf, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
				wound_to_char (tch, location, dice(obj->o.od.value[0], obj->o.od.value[1]), 0, number(1,4),0,0);
				return;
			}
			else if ( !removed ) {
				send_to_char ("You don't see that -- how could you remove it?\n", ch);
				return;
			}
		}
		else if ( target_obj ) {
			if ( obj->lodged && name_is (arg2, vtoo(obj->lodged->vnum)->name) ) {
				snprintf (location, MAX_STRING_LENGTH, "%s", obj->lodged->location);

				arrow = load_object(obj->lodged->vnum);
				arrow->count = 1;
				obj_to_char (arrow, ch);
				lodge_from_obj (obj, obj->lodged);
				removed++;
			}
			else for ( lodged = obj->lodged; lodged; lodged = lodged->next ) {
				if ( !lodged->next )
					break;
				if ( name_is (arg2, vtoo(lodged->next->vnum)->name) ) {
					snprintf (location, MAX_STRING_LENGTH, "%s", lodged->next->location);

					arrow = load_object(lodged->next->vnum);
					arrow->count = 1;
					obj_to_char (arrow, ch);
					lodge_from_obj (obj, lodged);
					removed++;
					break;
				}
			}	
			if ( removed ) {
				temp_wound = expand_wound_loc(location);
				snprintf (buf, MAX_STRING_LENGTH,  "You retrieve #2%s#0 from #2%s#0's %s.", arrow->short_description, obj->short_description, temp_wound);
				
				temp_arg = char_short(ch);
				temp_wound = expand_wound_loc(location);
				snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 retrieves #2%s#0 from #2%s#0's %s.", temp_arg, arrow->short_description, obj->short_description, temp_wound);
				*buf2 = toupper(*buf2);
				snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf2);
				snprintf (buf2, MAX_STRING_LENGTH,  "%s", buffer);
				act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				act (buf2, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
				return;
			}
			else if ( !removed ) {
				send_to_char ("You don't see that -- how could you remove it?\n", ch);
				return;
			}
		}
	}

	if ( !obj ) {
		send_to_char ("Remove what?\n", ch);
		return;
	}

	if ( obj->location == -1 ) {
		send_to_char ("You don't need to remove that!\n", ch);
		return;
	}

	if ( SWIM_ONLY (ch->room) ) {
		act ("You begin attempting to remove $p, hindered by the water. . .", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
		ch->delay_type = DEL_WATER_REMOVE;
		ch->delay_who = (char *) obj; 
		ch->delay = 45 - number(1,15);
		return;
	}

	if ( obj->location == WEAR_WAIST ) {
		if ( (eq = get_equip (ch, WEAR_BELT_1)) ) {
			act ("$p falls to the floor.", TRUE, ch, eq, 0, TO_CHAR);
			act ("$n drops $p.", TRUE, ch, eq, 0, TO_ROOM);
			temp_obj = unequip_char (ch, WEAR_BELT_1);
			obj_to_room (temp_obj, ch->in_room);
		}

		if ( (eq = get_equip (ch, WEAR_BELT_2)) ) {
			act ("$p falls to the floor.", TRUE, ch, eq, 0, TO_CHAR);
			act ("$n drops $p.", TRUE, ch, eq, 0, TO_ROOM);
			temp_obj = unequip_char (ch, WEAR_BELT_2);
			obj_to_room (temp_obj, ch->in_room);
		}
	}

	if ( obj->loaded && obj->o.weapon.use_skill != SKILL_CROSSBOW )
		do_unload (ch, "", 0);

	if ( IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
		 IS_SET (ch->affected_by, AFF_HOODED) )
		do_hood (ch, "", 0);


	if ( obj->location == WEAR_PRIM || obj->location == WEAR_SEC ||
		obj->location == WEAR_BOTH || obj->location == WEAR_SHIELD )
			temp_obj = unequip_char (ch, obj->location);
	else {
		temp_obj = unequip_char (ch, obj->location);
		obj_to_char (temp_obj, ch);
	}

	act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM);

}

int can_handle (OBJ_DATA *obj, CHAR_DATA *ch)
{
	int		wear_count = 0;

    if ( get_equip (ch, WEAR_BOTH) ) 		wear_count = wear_count + 2;
	if ( get_equip (ch, WEAR_SHIELD) )		wear_count++;
	if ( get_equip (ch, WEAR_PRIM) )		wear_count++;
	if ( get_equip (ch, WEAR_SEC) )			wear_count++;

	if ( wear_count > 2 ) {
		return 0;
	}

	if ( wear_count == 2 )
		return 0;

	if ( obj->o.od.value[0] == 0 && get_equip (ch, WEAR_PRIM) )
		return 0;

	if ( (obj->obj_flags.wear_flags & ITEM_WIELD) &&
	     obj->o.od.value [0] == 3 ) {			/* Two-handed */
		if ( wear_count == 0 )
			return 1;
		else
			return 0;
	}

	return 1;
}


void do_sheathe (CHAR_DATA *ch, char *argument, int cmd)
{
	char		arg1 [MAX_STRING_LENGTH] = {'\0'};
	char		arg2 [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buffer [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*obj_prim = NULL;
	OBJ_DATA	*obj_sec = NULL;
	OBJ_DATA	*obj_both = NULL;
	OBJ_DATA	*sheath = NULL;
	char		*msg = NULL;
	int			i = 0;
	int			sheathed = 0;
	char		*temp_arg = NULL;

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	obj_prim = get_equip (ch, WEAR_PRIM);
	obj_sec  = get_equip (ch, WEAR_SEC);
	obj_both = get_equip (ch, WEAR_BOTH);

	if ( !*arg1 ) {
		if ( obj_prim && IS_OBJ_VIS (ch, obj_prim) )
			obj = obj_prim;

		else if ( obj_sec && IS_OBJ_VIS (ch, obj_sec) )
			obj = obj_sec;

		else if ( obj_both && IS_OBJ_VIS (ch, obj_both) )
			obj = obj_both;
	}

	else if ( obj_prim &&
		 IS_OBJ_VIS (ch, obj_prim) &&
		 name_is (arg1, obj_prim->name) )
		obj = obj_prim;

	else if ( obj_sec &&
		 IS_OBJ_VIS (ch, obj_sec) &&
		 name_is (arg1, obj_sec->name) )
		obj = obj_sec;

	else if ( obj_both &&
		 IS_OBJ_VIS (ch, obj_both) &&
		 name_is (arg1, obj_both->name) )
		obj = obj_both;

	if ( !obj && ((ch->right_hand && GET_ITEM_TYPE(ch->right_hand) == ITEM_WEAPON) ||
			(ch->left_hand && GET_ITEM_TYPE(ch->left_hand) == ITEM_WEAPON)) ) {
		if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_WEAPON )
			obj = ch->right_hand;
		else if ( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_WEAPON )
			obj = ch->left_hand;
	}

	if ( !obj ) {
		if ( !*arg1 )
			send_to_char ("You aren't wielding anything.\n", ch);
		else
			send_to_char ("You aren't wielding that.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (obj) != ITEM_WEAPON || 
             obj->o.weapon.use_skill == SKILL_SHORTBOW ||
	     obj->o.weapon.use_skill == SKILL_LONGBOW ||
	     obj->o.weapon.use_skill == SKILL_CROSSBOW ) {
		send_to_char ("You can only sheath a melee weapon.\n", ch);
		return;
	}

	if ( IS_NPC (ch) && IS_SET(obj->obj_flags.wear_flags, ITEM_WEAR_BELT) ) {
		(void)one_argument (obj->name, buf);
		do_wear (ch, buf, 0);
		return;
	}

	if ( *arg2 ) {
		if ( !(sheath = get_obj_in_list_vis (ch, arg2, ch->equip)) ) {
			send_to_char ("What did you want to sheathe it in?\n", ch);
			return;
		}
		if ( can_obj_to_container (obj, sheath, &msg, 1) )
			sheathed++;
	}
	else if ( !*arg2 ) {
		for ( i = 0; i < MAX_WEAR; i ++ ) {
			if ( !(sheath = get_equip (ch, i)) )
				continue;
			if ( GET_ITEM_TYPE (sheath) != ITEM_SHEATH )
				continue;
			if ( !can_obj_to_container (obj, sheath, &msg, 1) )
				continue;
			sheathed++;
			break;
		}
	}

	if ( !sheathed ) {
		send_to_char("You aren't wearing any sheaths capable of bearing that weapon.\n", ch);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "You sheathe #2%s#0 in #2%s#0.", obj->short_description, sheath->short_description);
	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	temp_arg = char_short (ch);
	snprintf (buf, MAX_STRING_LENGTH,  "%s#0 sheathes #2%s#0 in #2%s#0.", temp_arg, obj->short_description, sheath->short_description);
	*buf = toupper (*buf);
	snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
	act (buffer, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	if ( ch->right_hand == obj )
		ch->right_hand = NULL;
	else if ( ch->left_hand == obj )
		ch->left_hand = NULL;

	obj_to_obj (obj, sheath);
}


void do_draw (CHAR_DATA *ch, char *argument, int cmd)
{
	char		arg1 [MAX_STRING_LENGTH] = {'\0'};
	int			obj_destination = 0;
	int			i = 0;
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*sheath = NULL;

	argument = one_argument (argument, arg1);

	for ( i = 0; i < MAX_WEAR; i++ ) {
		if ( !(sheath = get_equip (ch, i)) )
			continue;
		if ( GET_ITEM_TYPE (sheath) != ITEM_SHEATH )
			continue;
		if ( *arg1 ) {
			if ( !(obj = get_obj_in_list_vis (ch, arg1, sheath->contains)) ) 
				continue;
			break;
		}
		else if ( !*arg1 && sheath->contains ) {
			obj = sheath->contains;
			break;
		}
	}

	if ( !obj && get_equip (ch, WEAR_BELT_1) &&
		GET_ITEM_TYPE (get_equip (ch, WEAR_BELT_1)) == ITEM_WEAPON )
		obj = get_equip (ch, WEAR_BELT_1);
	else if ( !obj && get_equip (ch, WEAR_BELT_2) &&
		GET_ITEM_TYPE (get_equip (ch, WEAR_BELT_2)) == ITEM_WEAPON )
		obj = get_equip (ch, WEAR_BELT_2);

	if ( !obj ) {
		if ( !*arg1 )
			send_to_char ("You have nothing to draw!\n", ch);
		else
			send_to_char ("You don't have that in a sheath.\n", ch);
		return;
	}

	if ( !IS_SET (obj->obj_flags.wear_flags, ITEM_WIELD) ) {
		act ("You can't wield $o.", TRUE, ch, obj, 0, TO_CHAR);
		return;
	}

        switch(obj->o.od.value[3]) {
       		case SKILL_MEDIUM_EDGE:  /*  Medium weapons. */
                case SKILL_MEDIUM_BLUNT:
                case SKILL_MEDIUM_PIERCE:
                    /*  > 17 str wields ME in either hand.
					obj_destination = WEAR_PRIM;
                    break;                                     
                       
          		case SKILL_LIGHT_EDGE: /*  Light weapons. */
                case SKILL_LIGHT_BLUNT:
                case SKILL_LIGHT_PIERCE:
		case SKILL_SLING: 
                	if ( get_equip (ch, WEAR_PRIM) &&
                                get_equip (ch, WEAR_SEC) ) {
        	                send_to_char("You are already wielding both a primary and a secondary weapon.\n",ch);
                                return;
                        }
                        if ( get_equip (ch, WEAR_BOTH) ) {
               	        	send_to_char ("You are already wielding a two-handed weapon.\n", ch);
                                return;
                        }
                        if ( !get_equip (ch, WEAR_PRIM) )
				obj_destination = WEAR_PRIM;
                        else
				obj_destination = WEAR_SEC;
                        break;
		case SKILL_BRAWLING:
             	case SKILL_SHORTBOW:  /*  Everybody uses two hands for ranged weapons, or brawling weapons. */
                case SKILL_LONGBOW:
                case SKILL_CROSSBOW:
                case SKILL_STAFF:
                case SKILL_POLEARM:
                	if ( get_equip (ch, WEAR_BOTH) ||
                        	get_equip (ch, WEAR_PRIM) ||
                                get_equip (ch, WEAR_SEC) ||
                                (ch->right_hand || ch->left_hand) ) {
                                send_to_char("You need both hands to wield this weapon.\n",ch);
                                return;
                        }
			obj_destination = WEAR_BOTH;
			break;
               	case SKILL_HEAVY_EDGE:  /*  Heavy weapons. */
                case SKILL_HEAVY_BLUNT:
                case SKILL_HEAVY_PIERCE:
                        if ( ch->str >= 20 ) { /*  Extremely strong chars can wield two-handed weapons with one hand. */
                        	if ( get_equip (ch, WEAR_PRIM) ) {
	                                send_to_char("You are already wielding a primary weapon.\n", ch);
                                        return;
                                } else if ( get_equip (ch, WEAR_BOTH) ) {
                                	send_to_char("You are already wielding a two-handed weapon.\n",ch);
                                        return;
                                } else {
                                	if ( get_equip (ch, WEAR_SEC) || (ch->right_hand || ch->left_hand) )
						obj_destination = WEAR_PRIM;
                                        else
						obj_destination = WEAR_BOTH;
                                }
                                break;
          	    	}
                        if ( get_equip (ch, WEAR_BOTH) ||
                       		get_equip (ch, WEAR_PRIM) ||
      	                        get_equip (ch, WEAR_SEC) ||
                                (ch->right_hand || ch->left_hand) ) {
                        	send_to_char("You need both hands to wield this weapon.\n",ch);
                                return;
                        }
			obj_destination = WEAR_BOTH;
                        break;
	}

        if ( obj_destination == WEAR_BOTH && (ch->right_hand || ch->left_hand) ) {
        	send_to_char ("You'll need both hands free to draw this weapon.\n", ch);
          	return;
        }
	else if ( (obj_destination == WEAR_PRIM || obj_destination == WEAR_SEC) && (ch->right_hand && ch->left_hand) ) {
                send_to_char ("You'll need a free hand to draw that weapon.\n", ch);
      		return;
	}

	(void)unequip_char (ch, obj->location);

	if ( obj->in_obj )
		obj_from_obj (&obj, 0);
	equip_char (ch, obj, obj_destination);

	if ( obj_destination == WEAR_BOTH )
		ch->right_hand = obj;
	else if ( obj_destination == WEAR_PRIM ) {
		if ( ch->right_hand )
			ch->left_hand = obj;
		else ch->right_hand = obj;
	}
	else if ( obj_destination == WEAR_SEC ) {
		if ( ch->right_hand )
			ch->left_hand = obj;
		else ch->right_hand = obj;
	}

	act ("You draw $p.", TRUE, ch, obj, 0, TO_CHAR);
	act ("$n draws $p.", TRUE, ch, obj, 0, TO_ROOM);
}

void do_skin (CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA		*obj_corpse = NULL;
	char			obj_name [MAX_INPUT_LENGTH];
	CHAR_DATA		*tch = NULL;

	if ( !(ch->right_hand && ch->right_hand->o.od.value[4] == 0) &&
	     !(ch->left_hand && ch->left_hand->o.od.value[4]   == 0) &&
		 !(ch->right_hand && ch->right_hand->o.od.value[4] == 1) &&
	     !(ch->left_hand && ch->left_hand->o.od.value[4]   == 1) &&
		 !(ch->right_hand && ch->right_hand->o.od.value[4] == 4) &&
	     !(ch->left_hand && ch->left_hand->o.od.value[4]   == 4) 
		 ) {
		send_to_char (
			"You need to be holding a stabbing, piercing or slicing weapon in order to skin.\n", ch);
		return;
	}

	argument = one_argument (argument, obj_name);

		/* Allow the user to enter a corpse name in case he wants to
		   say 2.corpse instead of just corpse */

	if ( !*obj_name )
		strcpy (obj_name, "corpse");

	obj_corpse = get_obj_in_list_vis (ch, obj_name, 
						ch->room->contents);
	
	if ( !obj_corpse ) {
		send_to_char ("You don't see a corpse here.\n", ch);
		return;
	}

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
		if ( tch->delay_info1 == (int) obj_corpse ) {
			send_to_char ("Someone's already skinning that corpse at the moment.\n", ch);
			return;
		}
	}

	if ( obj_corpse->virtual != VNUM_CORPSE ) {
		send_to_char ("You can only skin corpses.\n", ch);
		return;
	}

	if ( !obj_corpse->o.od.value [2] ) {
		send_to_char ("After a moment of poking, you realize that it isn't worth skinning.\n", ch);
		return;
	}

	ch->delay_info1 = (int) obj_corpse;

	if ( !real_skill (ch, SKILL_SKIN) )
		send_to_char ("You begin hacking away at the corpse.\n", ch);
	else if ( ch->skills [SKILL_SKIN] < 30 )
		send_to_char ("You begin flailing at the corpse.\n", ch);
	else if ( ch->skills [SKILL_SKIN] < 40 )
		send_to_char ("You begin slashing into the corpse.\n", ch);
	else if ( ch->skills [SKILL_SKIN] < 50 )
		send_to_char ("You begin carving the corpse.\n", ch);
	else if ( ch->skills [SKILL_SKIN] < 60 )
		send_to_char ("You begin operating on the corpse.\n", ch);
	else 
		send_to_char ("You begin delicately skinning the corpse.\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "$n begins skinning #2%s#0.", obj_corpse->short_description);

	act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

    	ch->delay_type = DEL_SKIN_1;
    	ch->delay      = 3;
}

void delayed_skin_new1 (CHAR_DATA *ch)
{

	send_to_char ("You start to cut into the corpse...\n", ch);
	act ("$n starts to cut into the corpse...", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	ch->delay_type = DEL_SKIN_2;
    	ch->delay      = 7;
}

void delayed_skin_new2 (CHAR_DATA *ch)
{
	send_to_char ("You seem to be making progress as you dig into the corpse...\n", ch);
	act ("$n seems to be making progress as $e digs into the corpse...", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

    	ch->delay_type = DEL_SKIN_3;
    	ch->delay      = 10;
}

void delayed_skin_new3 (CHAR_DATA *ch)
{
	OBJ_DATA	*skin = NULL;
	OBJ_DATA	*corpse = NULL;
	OBJ_DATA	*carcass = NULL;
	char		buf [MAX_INPUT_LENGTH];
	char		*p = NULL;

	corpse = (OBJ_DATA *)ch->delay_info1;

	if ( !corpse || sizeof(*corpse) < sizeof(OBJ_DATA) ) {
		send_to_char ("There has been an error with the corpse. Aborting to avoid crash.\n", ch);
		return;
	}

	if ( skill_use (ch, SKILL_SKIN, 0) ) 
	{

		if ( !(skin = load_object (corpse->o.od.value[2])) ) 
		{
			send_to_char ("Problem...please contact an immortal.\n", ch);
			return;
		}

		obj_to_room (skin, ch->in_room);

		strcpy (buf, skin->name);

		for ( p = buf; *p && *p != ' ';)
		{
			p++;
		}

		*p = '\0';

		if (CAN_SEE_OBJ (ch, skin))
		{
			send_to_char ("You have successfully skinned the corpse.\n", ch);
			act ("$n has successfully skinned the corpse.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			do_get (ch, buf, 0);
		}
		else 
		{
			send_to_char ("You demolished the corpse.\n", ch);
			act ("$n demolishes a corpse.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		}
	} 
	else 
	{
		send_to_char ("You demolished the corpse.\n", ch);
		act ("$n demolishes a corpse.", FALSE, ch, 0, 0, TO_ROOM);
	}

	ch->delay_type = 0;
	ch->delay = 0;
	ch->delay_info1 = 0;
	ch->delay_info2 = 0;

	if ( !(carcass = load_object (corpse->o.od.value[3])) ) {
		extract_obj (corpse);
		return;
	}

	extract_obj (corpse);

	obj_to_room (carcass, ch->in_room);
}

/*
    Return an int which is the objnum of a plant suitable for the sector type
    with some probability depending on rarity.
*/

#define RUMM_VERY_RARE	6
#define RUMM_RARE		20
#define RUMM_UNCOMMON	42
#define RUMM_COMMON		70
#define RUMM_VCOMMON	100

#define RUMM_MAX 100

int GetHerbPlant(int sector_type)

{
	int 	i = 0;
	int		sect = 0;
	int		objnum = 0;
	int		rarityIndex = 0;
	int		numHerbs = 0;

    i = number(1, RUMM_MAX);

    if (i <= RUMM_VERY_RARE)
	rarityIndex = 0;
    else if (i <= RUMM_RARE)
	rarityIndex = 1;
    else if (i <= RUMM_UNCOMMON)
	rarityIndex = 2;
    else if (i <= RUMM_COMMON)
	rarityIndex = 3;
    else /* very common */
	rarityIndex = 4; 

    /*
	I could get clever with the rarities in accessing the herb arrays,
	but I won't in case we decide we need to change the rarites and
	stuff.  The code will just look a tad clunky IMHO.
    */

    switch(sector_type) {
	case SECT_FIELD:
			sect = HERB_FIELD;
			break;
	case SECT_PASTURE:	
			sect = HERB_PASTURE;
			break;
	case SECT_WOODS:	
			sect = HERB_WOODS;
			break;
	case SECT_FOREST:
			sect = HERB_FOREST;
			break;
	case SECT_MOUNTAIN:
			sect = HERB_MOUNTAINS;
			break;
	case SECT_SWAMP:
			sect = HERB_SWAMP;
			break;
	case SECT_HEATH:
			sect = HERB_HEATH;
			break;
	case SECT_HILLS:
			sect = HERB_HILLS;
			break;
	default:
			return(0);
	
    }

    numHerbs = herbArray[(sect*HERB_RARITIES)+rarityIndex][0];

    if(numHerbs == 1)
	objnum = herbArray[(sect*HERB_RARITIES)+rarityIndex][1];
    else
	objnum = herbArray[(sect*HERB_RARITIES)+rarityIndex][number(1,numHerbs)];
    return(objnum); 

}

void do_rummage (CHAR_DATA *ch, char *argument, int cmd)
{
        int		sector_type;

        send_to_char("Rummage has been disabled.\n", ch);
	return;
	
        if ( !real_skill (ch, SKILL_HERBALISM) ) {
                send_to_char ("You don't have any idea how to rummage!\n\r", ch);
                return;
        }

        if ( is_dark (ch->room) && !get_affect(ch, MAGIC_AFFECT_INFRAVISION) && !IS_SET (ch->affected_by, AFF_INFRAVIS)) {
                send_to_char ("It's too dark to rummage.\n\r", ch);
                return;
        }


        sector_type = vtor (ch->in_room)->sector_type;

	if(sector_type == SECT_CITY ||
	   sector_type == SECT_INSIDE) {
		send_to_char("You will not find any healthy plants growing here.\n", ch);
		return;
	}

	if(sector_type != SECT_FIELD &&
	   sector_type != SECT_PASTURE &&
	   sector_type != SECT_WOODS &&
	   sector_type != SECT_FOREST &&
	   sector_type != SECT_MOUNTAIN &&
	   sector_type != SECT_SWAMP &&
	   sector_type != SECT_HEATH &&
	   sector_type != SECT_HILLS) {
		send_to_char("Any plant that might be growing here has long since been trampled\n\r", ch);
		return;
	}

        send_to_char ("You begin searching the area for useful plants.\n\r", ch);
        act ("$n begins rummaging through the plant life.", TRUE, ch, 0, 0, TO_ROOM);

        ch->delay_type = DEL_RUMMAGE;
        ch->delay           = 30;
}

void delayed_rummage (CHAR_DATA *ch)
{
    OBJ_DATA	*obj = NULL;
	int 		objnum = 0;
	AFFECTED_TYPE	*herbed = NULL;

	herbed = is_room_affected(ch->room->affects, HERBED_COUNT);

	if(herbed && (herbed->a.herbed.timesHerbed >= MAX_HERBS_PER_ROOM)) {
	    send_to_char("This area has been stripped of all useful plants.\n\r", ch);
	    return;
	}

        if ( skill_use (ch, SKILL_HERBALISM, 0) ) {


	    objnum = GetHerbPlant(ch->room->sector_type);
	    obj = load_object(objnum);

	    if (obj) {
	        obj_to_room(obj, ch->in_room);
	        act("Your rummaging has revealed $p.", FALSE, ch, obj, 0, TO_CHAR);
		act("$n reveals some useful plant life.", TRUE, ch, 0, 0, TO_ROOM);
		
		if(!herbed) {
		    herbed = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);
		    herbed->type = HERBED_COUNT;
		    herbed->a.herbed.timesHerbed = 1;
		    herbed->a.herbed.duration = HERB_RESET_DURATION;
		    herbed->next = ch->room->affects;
		    ch->room->affects = herbed;
		
		} 
		else {
		    herbed->a.herbed.timesHerbed++;
		    herbed->a.herbed.duration = HERB_RESET_DURATION;
		}
		    
            } else
                send_to_char ("You successfully rummaged but there is naught to be found\n\r", ch);

        } 
        else {
            send_to_char ("Your rummaging efforts are of no avail.\n\r",
ch);
                act ("$n stops searching the plant life.", TRUE, ch, 0, 0, TO_ROOM);
        }
        
        return;
}

void do_gather (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*plant = NULL;
	ROOM_DATA	*room = NULL;

	if ( !real_skill (ch, SKILL_HERBALISM) ) {
		send_to_char ("You need training in herbalism before you can identify "
					  "flora.\n", ch);
		return;
	}

	room = ch->room;

	argument = one_argument (argument, buf);

	if ( !(plant = get_obj_in_list_vis (ch, buf, room->contents)) ) {
		send_to_char ("You don't see that here.\n", ch);
		return;
	}

	if ( plant->obj_flags.type_flag != ITEM_PLANT ) {
		act ("$p is not flora.  You can only gather from plants.",
					TRUE, ch, plant, 0, TO_CHAR);
		return;
	}

	act ("You begin to harvest $p.", TRUE, ch, plant, 0, TO_CHAR);
	act ("$n begins to harvest $p.", TRUE, ch, plant, 0, TO_ROOM);

	ch->delay_type = DEL_GATHER;
	ch->delay_who = str_dup (buf);
	ch->delay = 20 - (ch->skills [SKILL_HERBALISM] / 10);
	
	return;
}

void delayed_gather (CHAR_DATA *ch)
{
	OBJ_DATA	*plant = NULL;
	OBJ_DATA	*obj = NULL;
	ROOM_DATA	*room = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	room = ch->room;

	strcpy (buf, ch->delay_who);

	mem_free (ch->delay_who);
	ch->delay_who = NULL;

	if ( !(plant = get_obj_in_list_vis (ch, buf, room->contents)) ) {
		send_to_char ("You failed to gather the contents of the plant before "
					  "it was destroyed.\n", ch);
		return;
	}

	act ("$n's destroys $p.", TRUE, ch, plant, 0, TO_ROOM);

	if ( !plant->o.od.value [0] ||
		 !vtoo (plant->o.od.value [0]) ||
		 !skill_use (ch, SKILL_HERBALISM, 0) ) {
		act ("You destroy $p while failing to gather anything from it.",
				TRUE, ch, plant, 0, TO_CHAR);
	}

	else {

		obj = load_object (plant->o.od.value [0]);

		if ( !CAN_SEE_OBJ (ch, obj) ) {
			act ("You destroy $p", TRUE, ch, plant, 0, TO_CHAR);
			obj_to_room (obj, ch->in_room);
		}

		else {
			act ("You extract $p, destroying $P.",
						TRUE, ch, obj, plant, TO_CHAR);
			act ("$n has gathered $p", TRUE, ch, obj, 0, TO_ROOM);
			obj_to_char (obj, ch);
		}
	}

	obj_from_room (&plant, 0);
	extract_obj (plant);
	
	return;
}

void do_identify (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*plant = NULL;
	ROOM_DATA	*room = NULL;

	send_to_char ("This command is currently disabled.\n", ch);
	return;

	if ( !real_skill (ch, SKILL_HERBALISM) ) {
		send_to_char ("You need training in herbalism before you can identify "
					  "flora.\n", ch);
		return;
	}

	room = ch->room;

	argument = one_argument (argument, buf);

	if ( !(plant = get_obj_in_list_vis (ch, buf, room->contents)) ) {
		send_to_char ("You don't see that here.\n", ch);
		return;
	}

	if ( plant->obj_flags.type_flag != ITEM_PLANT ) {
		act ("$p is not flora.  You can only identify plants.",
					TRUE, ch, plant, 0, TO_CHAR);
		return;
	}

	act ("You begin to examine $p.", TRUE, ch, plant, 0, TO_CHAR);
	act ("$n begins examining $p.", TRUE, ch, plant, 0, TO_ROOM);

	ch->delay_type = DEL_IDENTIFY;
	ch->delay_who = str_dup (buf);
	ch->delay = 15 - (ch->skills [SKILL_HERBALISM] / 10);
	
	return;
}

void delayed_identify (CHAR_DATA *ch)
{
	OBJ_DATA	*plant = NULL;
	OBJ_DATA	*obj = NULL;
	ROOM_DATA	*room = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	room = ch->room;

	strcpy (buf, ch->delay_who);

	mem_free (ch->delay_who);
	ch->delay_who = NULL;

	if ( !(plant = get_obj_in_list_vis (ch, buf, room->contents)) ) {
		send_to_char ("You failed to identify the contents of the plant before "
					  "it was destroyed.\n", ch);
		return;
	}

	act ("$n finishes looking at $p.", TRUE, ch, plant, 0, TO_ROOM);

	if ( !plant->o.od.value [0] ||
		 !vtoo (plant->o.od.value [0]) ||
		 !skill_use (ch, SKILL_HERBALISM, 0) ) {
		act ("You could not determine what $p might yield.",
				TRUE, ch, plant, 0, TO_CHAR);
		return;
	}

	else {
		obj = vtoo (plant->o.od.value [0]);

		act ("You determine that $p would yield $P.", 
				TRUE, ch, plant, obj, TO_CHAR);
	}
	
	return;
}

void do_combine (CHAR_DATA *ch, char *argument, int cmd)
{
	int			k = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	int			ingredients [7] = { 0, 0, 0, 0, 0, 0, 0 };
	OBJ_DATA	*obj_ingredients [7] =
						{ NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	OBJ_DATA	*obj = NULL;
	int			i = 0;
	int			j = 0;
	RECIPE_DATA	*recipe_entry = NULL;

	if ( !real_skill (ch, SKILL_ALCHEMY) ) {
		send_to_char ("You must learn more about alchemy first.\n", ch);
		return;
	}

	if ( !IS_SET (ch->room->room_flags, LAB) ) {
		send_to_char ("Alchemy is a delicate science, to be performed in "
					  "a laboratory.\n", ch);
		return;
	}

	i = 0;

	while (1) {

		argument = one_argument (argument, buf);

		if ( !*buf )
			break;

		if ( i >= 7 ) {
			send_to_char ("No mixture has more than seven ingredients.\n",
						  ch);
			return;
		}

		if ( !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) ) {
			snprintf (buf2, MAX_STRING_LENGTH,  "You don't have %s.\n", buf);
			send_to_char (buf2, ch);
			return;
		}

		if ( obj->obj_flags.type_flag != ITEM_FOOD &&
			 obj->obj_flags.type_flag != ITEM_HERB &&
			 obj->obj_flags.type_flag != ITEM_COMPONENT &&
			 obj->obj_flags.type_flag != ITEM_POTION &&
			 obj->obj_flags.type_flag != ITEM_SALVE &&
			 !IS_SET (obj->obj_flags.extra_flags, ITEM_COMBINABLE) ) {
			act ("No recipe could contain $p.", FALSE, ch, obj, 0, TO_CHAR);
			return;
		}

		for ( j = 0; j < i; j++ )
			if ( obj_ingredients [j] == obj ) {
				send_to_char ("You can't use the same ingredient twice.\n",
							  ch);
				return;
			}

		obj_ingredients [i] = obj;
		ingredients [i] = obj->virtual;

		i++;
	}

	if ( i < 2 ) {
		send_to_char ("You must combine between two and seven ingredients to "
					  "create something.\n", ch);
		return;
	}

	act ("$n mixes several ingredients together.", TRUE, ch, 0, 0, TO_ROOM);
	send_to_char ("You carefully mix the ingredients together.\n", ch);

	sort_int_array (ingredients, 7);

	for ( recipe_entry = recipe_list;
		  recipe_entry;
		  recipe_entry = recipe_entry->next ) {

		if ( recipe_entry->ingredient [0] == ingredients [0] &&
			 recipe_entry->ingredient [1] == ingredients [1] &&
			 recipe_entry->ingredient [2] == ingredients [2] &&
			 recipe_entry->ingredient [3] == ingredients [3] &&
			 recipe_entry->ingredient [4] == ingredients [4] &&
			 recipe_entry->ingredient [5] == ingredients [5] &&
			 recipe_entry->ingredient [6] == ingredients [6] ) {

			if ( !vtoo (recipe_entry->obj_created) ) {
				send_to_char ("You discover this recipe is a jip!\n", ch);
				return;
			}

				/* Remove recipe ingredients, except reuse items */

			for ( i = 0; i < 7; i++ ) {

				if ( !obj_ingredients [i] )
					continue;

						/* obj_ingredients is in a different order than
                           the recipe calls for.  To determine extraction,
                           we have find the ingredient that matches the recipe
                           for the slot */

				for ( k = 0; k < 7; k++ ) {

					if ( !ingredients [k] )
						continue;

					if ( obj_ingredients [i]->virtual == ingredients [k] )
						break;
				}

				if ( k == 7 || !recipe_entry->reuse [k] ) {
					extract_obj (obj_ingredients [i]);
					ingredients [k] = 0;	/* Don't look at this again */
				}
			}

			ch->delay_type = DEL_COMBINE;
			ch->delay	   = 101 - ch->skills [SKILL_ALCHEMY];
			ch->delay_who  = (char *) recipe_entry->obj_created;

			return;
		}
	}

			/* False recipe, remove all ingredients */

	for ( i = 0; i < 7; i++ )
		if ( obj_ingredients [i] ) {
			obj_from_char (&obj_ingredients [i], 0);
			extract_obj (obj_ingredients [i]);
		}

	ch->delay_type = DEL_COMBINE;
	ch->delay      = 101 - ch->skills [SKILL_ALCHEMY] / 2;
	ch->delay_who  = NULL;
	
	return;
}

void delayed_combine (CHAR_DATA *ch)
{
	OBJ_DATA	*obj = NULL;

	if ( !real_skill (ch, SKILL_ALCHEMY) ) {
		send_to_char ("You must learn more about alchemy first.\n", ch);
		return;
	}

	if ( !IS_SET (ch->room->room_flags, LAB) ) {
		send_to_char ("You needed all that laboratory equipment to complete "
					  " the mixture.", ch);
		return;
	}

	if ( !ch->delay_who ||
		 !skill_use (ch, SKILL_ALCHEMY, 0) ) {
		act ("You create a nasty mess which you quickly throw away.",
						FALSE, ch, 0, 0, TO_CHAR);
		act ("$n mixes a nasty mess which $e quickly tosses away.",
						TRUE, ch, 0, 0, TO_ROOM);
		return;
	}

	obj = load_object ((int) ch->delay_who);

	if ( !obj ) {
		send_to_char ("There is something wrong with this recipe...it won't "
					  "work.\n", ch);
		return;
	}

	act ("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
	act ("$n creates $p.", TRUE, ch, obj, 0, TO_ROOM);

	if ( CAN_CARRY_OBJ (ch, obj) )
		obj_to_char (obj, ch);
	else {
		act ("You can't carry $p.", FALSE, ch, obj, 0, TO_CHAR);
		obj_to_room (obj, ch->in_room);
	}
	
	return;
}


void do_empty (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*container = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;

	argument = one_argument (argument, buf);

	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("That is not allowed in OOC areas.\n", ch);
		return;
	}

	if ( !(container = get_obj_in_dark (ch, buf, ch->right_hand)) &&
		!(container = get_obj_in_dark (ch, buf, ch->left_hand)) ) {
		send_to_char ("You don't have that object.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (container) == ITEM_CONTAINER ) {

		if ( container->contains ) {
			act ("You turn $p upside down, spilling its contents:",
					FALSE, ch, container, 0, TO_CHAR);
			act ("$n turns $p upside down, spilling its contents:",
					TRUE, ch, container, 0, TO_ROOM);
		}

		while ( container->contains ) {
			obj = container->contains;
			obj_from_obj (&obj, 0);
			obj_to_room (obj, ch->in_room);
			act ("    $p", FALSE, ch, obj, 0, TO_CHAR);
			act ("    $p", FALSE, ch, obj, 0, TO_ROOM);
		}

		return;
	}

	if ( GET_ITEM_TYPE (container) == ITEM_POTION ) {
		act ("You spill the contents of $p on the ground.",
					FALSE, ch, container, 0, TO_CHAR);
		act ("$n spills the contents of $p on the ground.",
					FALSE, ch, container, 0, TO_ROOM);
		extract_obj (container);
		return;
	}

	if ( GET_ITEM_TYPE (container) == ITEM_DRINKCON ) {

		if ( !container->o.drinkcon.volume ) {
			act ("$o is already empty.", FALSE, ch, container, 0, TO_CHAR);
			return;
		}

		act ("You spill the contents of $p on the ground.",
					FALSE, ch, container, 0, TO_CHAR);
		act ("$n spills the contents of $p on the ground.",
					FALSE, ch, container, 0, TO_ROOM);

		container->o.drinkcon.liquid = 0;
		container->o.drinkcon.volume = 0;
		container->o.drinkcon.spell_1 = 0;
		container->o.drinkcon.spell_2 = 0;
		container->o.drinkcon.spell_3 = 0;

		return;
	}

	if ( GET_ITEM_TYPE (container) == ITEM_LIGHT &&
		 !is_name_in_list ("candle", container->name) ) {

		if ( !container->o.light.hours ) {
			act ("$o is already empty.", FALSE, ch, container, 0, TO_CHAR);
			return;
		}

		if ( container->o.light.on )
			light (ch, container, FALSE, TRUE);

		temp_arg = vnum_to_liquid_name (container->o.light.liquid);
		snprintf (buf, MAX_STRING_LENGTH,  "You empty %s from $p on the ground.", temp_arg);
		act (buf, FALSE, ch, container, 0, TO_CHAR);

		temp_arg = vnum_to_liquid_name (container->o.light.liquid);
		snprintf (buf, MAX_STRING_LENGTH,  "$n empties %s from $p on the ground.", temp_arg);
		act (buf, FALSE, ch, container, 0, TO_ROOM);

		container->o.light.hours = 0;

		return;
	}

	act ("You can't figure out how to empty $p.", FALSE, ch, container, 0, TO_CHAR);
				
	return;
}

void do_blindfold (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*victim = NULL;
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !(victim = get_char_room_vis (ch, buf)) ) {
		send_to_char ("There's no such person here.\n", ch);
		return;
	}

	if ( victim == ch ) {
		send_to_char ("Wear a blindfold if you want to blindfold yourself.\n", ch);
		return;
	}

	if ( get_equip (victim, WEAR_BLINDFOLD) ) {
		act ("$N is already blindfolded.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	if ( ch->right_hand && IS_SET (ch->right_hand->obj_flags.wear_flags, ITEM_WEAR_BLINDFOLD) )
		obj = ch->right_hand;

	else if ( ch->left_hand && IS_SET (ch->left_hand->obj_flags.wear_flags, ITEM_WEAR_BLINDFOLD) )
		obj = ch->left_hand;

	if ( !obj ) {
		send_to_char ("You don't have a blindfold available.\n", ch);
		return;
	}

	if ( !AWAKE (victim) && number (0, 4) ) {
		if ( wakeup (victim) ) {
			act ("You've awoken $N.", FALSE, ch, 0, victim, TO_CHAR);
			act ("$N wakes you up while trying to blindfold you!",
						FALSE, ch, 0, victim, TO_VICT);
			act ("$n wakes $N up while trying to bindfold $M.",
						FALSE, ch, 0, victim, TO_NOTVICT);
		}
	}

	if ( !(!AWAKE (victim) ||
		   get_affect (victim, MAGIC_AFFECT_PARALYSIS) ||
		   IS_SUBDUEE (victim)) ) {
		act ("$N won't let you blindfold $M.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	if ( obj->carried_by )
		obj_from_char (&obj, 0);

	act ("$N blindfolds you!", TRUE, victim, 0, ch, TO_CHAR);
	act ("You place $p over $N's eyes.", FALSE, ch, obj, victim, TO_CHAR);
	act ("$n places $p over $N's eyes.", FALSE, ch, obj, victim, TO_NOTVICT);

	equip_char (victim, obj, WEAR_BLINDFOLD);
}

/*

   Create a head object.

   Modify corpse object to be 'headless'.

*/

void do_behead (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*corpse = NULL;
	OBJ_DATA	*tool = NULL;
	OBJ_DATA	*head = NULL;

	argument = one_argument (argument, buf);

	if ( !(corpse = get_obj_in_list_vis (ch, buf, ch->room->contents)) ) {
		if ( get_obj_in_list_vis (ch, buf, ch->right_hand) || get_obj_in_list_vis (ch, buf, ch->left_hand) )
			send_to_char ("Drop the corpse to behead it.\n", ch);
		else
			send_to_char ("You don't see that corpse here.\n", ch);
		return;
	}

	if ( corpse->virtual != VNUM_CORPSE ) {
		act ("$o is not a corpse.", FALSE, ch, corpse, 0, TO_CHAR);
		return;
	}

	if ( IS_SET (corpse->o.container.flags, CONT_BEHEADED) ) {
		act ("$p looks headless already.", FALSE, ch, corpse, 0, TO_CHAR);
		return;
	}

	if ( !(tool = get_equip (ch, WEAR_PRIM)) &&
		 !(tool = get_equip (ch, WEAR_SEC))  &&
		 !(tool = get_equip (ch, WEAR_BOTH)) ) {
		act ("You need a slicing or chopping weapon to behead $p.",
					FALSE, ch, corpse, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( tool->o.weapon.hit_type < 0 ||
		 tool->o.weapon.hit_type > 5 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Your weapon, $p, vnum %d, doesn't have a valid hit_type "
				 "set.  This is a bug.  Please petition and report it.  "
				 "Thanks.", tool->virtual);
		act (buf, FALSE, ch, tool, 0, TO_CHAR);
		snprintf (buf, MAX_STRING_LENGTH,  "Weapon vnum %d has illegal hit type %d",
					  tool->virtual, tool->o.weapon.hit_type);
		system_log(buf, TRUE);
		return;
	}

	if ( tool->o.weapon.hit_type == 0 ||		/* 0 = stab */
		 tool->o.weapon.hit_type == 1 ||		/* 1 = pierce */
		 tool->o.weapon.hit_type == 3 ) {		/* 3 = bludgeon */
		snprintf (buf2, MAX_STRING_LENGTH,  "You can't %s the head off with $p.  Try a weapon "
					   "that will slice or chop.",
					   weapon_theme [tool->o.weapon.hit_type]);
		act (buf2, FALSE, ch, tool, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	head = load_object (VNUM_HEAD);

	head->name = str_dup ("head");
	if ( !strncmp (corpse->short_description, "the corpse of ", 14) )
		strcpy (buf2, corpse->short_description + 14);
	else
		strcpy (buf2, "some unfortunate creature");

	snprintf (buf, MAX_STRING_LENGTH,  "The head of %s rests here.", buf2);
	head->description = str_dup (buf);

	snprintf (buf, MAX_STRING_LENGTH,  "the head of %s", buf2);
	head->short_description = str_dup (buf);

	snprintf (buf, MAX_STRING_LENGTH,  "You %s the head from $p.",
				  weapon_theme [tool->o.weapon.hit_type]);
	act (buf, FALSE, ch, corpse, 0, TO_CHAR | TO_ACT_FORMAT);

	snprintf (buf, MAX_STRING_LENGTH,  "$n %ses the head from $p.",
				  weapon_theme [tool->o.weapon.hit_type]);
	act (buf, FALSE, ch, corpse, 0, TO_ROOM | TO_ACT_FORMAT);

	mem_free (corpse->description);
	mem_free (corpse->short_description);

	snprintf (buf, MAX_STRING_LENGTH,  "The headless corpse of %s is lying here.", buf2);
	corpse->description = str_dup (buf);

	snprintf (buf, MAX_STRING_LENGTH,  "the headless corpse of %s", buf2);
	corpse->short_description = str_dup (buf);

	strcpy (buf2, corpse->name);

	mem_free (corpse->name);

	snprintf (buf, MAX_STRING_LENGTH,  "headless %s", buf2);
	corpse->name = str_dup (buf);

	head->obj_flags.weight = corpse->obj_flags.weight / 10;
	corpse->obj_flags.weight -= head->obj_flags.weight;

	SET_BIT (corpse->o.container.flags, CONT_BEHEADED);

	head->obj_timer = corpse->obj_timer;

	obj_to_room (head, ch->in_room);
}

void light (CHAR_DATA *ch, OBJ_DATA *obj, int on, int on_off_msg)
{
		/* Automatically correct any problems with on/off status */

	if ( obj->o.light.hours <= 0 )
		obj->o.light.on = 0;

	if ( !on && !obj->o.light.on )
		return;

	if ( on && obj->o.light.hours <= 0 )
		return;

	if ( on && obj->o.light.on )
		return;

	obj->o.light.on = on;

	if ( on && get_affect (ch, MAGIC_HIDDEN) ) {
		if ( would_reveal (ch) )
			act ("You reveal yourself.", FALSE, ch, 0, 0, TO_CHAR);
		else
			act ("The light will reveal your hiding place.",
					FALSE, ch, 0, 0, TO_CHAR);

		remove_affect_type (ch, MAGIC_HIDDEN);
	}

	if ( on ) {
		room_light (ch->room);	/* lighten before messages */
		if ( on_off_msg ) {
			act ("You light $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act ("$n lights $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
		}
	} else {
		if ( on_off_msg ) {
			act ("You put out $p.", FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
			act ("$n puts out $p.", FALSE, ch, obj, 0, TO_ROOM | TO_ACT_FORMAT);
		}
		room_light (ch->room);	/* darken after messages */
	}
}

void do_light (CHAR_DATA *ch, char *argument, int cmd)
{
	int			on = 1;
	int			room_only = 0;
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "room") ) {
		room_only = 1;
		argument = one_argument (argument, buf);
	}

	if ( !*buf ) {
		send_to_char ("Light what?\n", ch);
		return;
	}

	if ( room_only ) {
		obj = get_obj_in_dark (ch, buf, ch->room->contents);
	} else {
		obj = get_obj_in_dark (ch, buf, ch->equip);
		if ( !obj )
			obj = get_obj_in_dark (ch, buf, ch->right_hand);
		if ( !obj )
			obj = get_obj_in_dark (ch, buf, ch->left_hand);
	}

	if ( !obj ) {
		send_to_char ("You don't see that light source.\n", ch);
		return;
	}

	if ( obj->obj_flags.type_flag != ITEM_LIGHT ) {
		act ("You cannot light $p.", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "off") )
		on = 0;

	if ( !on && !obj->o.light.on ) {
		act ("$p isn't lit.", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	if ( on && obj->o.light.hours <= 0 ) {
		act ("$p will no longer light.", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	if ( on && obj->o.light.on ) {
		act ("$p is already lit.", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	light (ch, obj, on, TRUE);
	
	return;
}

void do_smell (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Smelling rooms doesn't work yet.\n", ch);
		return;
	}

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that person.\n", ch);
		return;
	}

	if ( !get_stink_message (tch, NULL, buf, ch) ) {
		act ("$N does not have any peculiar smells.",
				FALSE, ch, 0, tch, TO_CHAR);
		return;
	}

	act (buf, FALSE, ch, 0, tch, TO_CHAR);
	act ("$N smells you.", FALSE, ch, 0, tch, TO_VICT);
	act ("$n smells $N.", FALSE, ch, 0, tch, TO_NOTVICT);
	
	return;
}
