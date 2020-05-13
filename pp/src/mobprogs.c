/** 
*	\file mobprogs.c
*	Provides services for booting, creation and activation of mobile programs
*
*	Description:  All matters dealing with booting, creation and activation of 	*	mobile programs. Eventually these will be supplemented with Lua scripts
*
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

VAR_DATA		*global_vars = NULL;
char			current_line [MAX_STRING_LENGTH];

int mob_get_token (char **ptr, char *token, int token_type)
{
	while ( **ptr == ' ' )
		(*ptr)++;

	if ( token_type == MT_OPEN_PAREN || token_type == MT_COMMA ||
		 token_type == MT_EQUAL ) {

		*token++ = **ptr;
		*token = '\0';

		if ( token_type == MT_OPEN_PAREN && **ptr != '(' )
			return 0;

		if ( token_type == MT_COMMA && **ptr != ',' )
			return 0;

		if ( token_type == MT_EQUAL && **ptr != '=' )
			return 0;

		(*ptr)++;

		return 1;
	}
			
	if ( token_type == MT_VARIABLE ) {
		if ( !isalpha (**ptr) && **ptr != '_' )
			return 0;

		while ( isalpha (**ptr) || isdigit (**ptr) || **ptr == '_' )
			*token++ = *((*ptr)++);

		*token = '\0';

		return 1;
	}

	return 0;
}

void require_open_paren (char **ptr)
{
	while ( **ptr == ' ' )
		(*ptr)++;

	(*ptr)++;
	
	return;
}

void deletevar (CHAR_DATA *mob, char *var_name)
{
	struct var_data		*tvar;
	struct var_data		*tvar2;

	if ( !mob->vartab )
		return;

	if ( !str_cmp (mob->vartab->name, var_name) ) {
		tvar = mob->vartab;
		mob->vartab = tvar->next;

		mem_free (tvar->name);
		mem_free (tvar);
		return;
	}

	for ( tvar = mob->vartab; tvar->next; tvar = tvar->next ) {
		if ( !str_cmp (tvar->next->name, var_name) ) {
			tvar2 = tvar->next;
			tvar->next = tvar2->next;

			mem_free (tvar2->name);
			mem_free (tvar2);
			return;
		}
	}
}

VAR_DATA *getvar (CHAR_DATA *mob, char *var_name)
{
	VAR_DATA	*var = NULL;

	if ( *var_name == '_' )
		var = global_vars;
	else
		var = mob->vartab;

	for ( ; var; var = var->next )
		if ( !str_cmp (var->name, var_name) )
			return var;

	return NULL;
}

VAR_DATA *setvar (CHAR_DATA *mob, char *var_name, int value, int type)
{
	VAR_DATA	*var;

	if ( !(var = getvar (mob, var_name)) ) {

		var = malloc (sizeof (VAR_DATA));

		var->name   = str_dup (var_name);

		if ( *var_name == '_' ) {
			var->next   = global_vars;
			global_vars = var;
		} else {
			var->next   = mob->vartab;
			mob->vartab = var;
		}
	}

	var->type  = type;
	var->value = value;

	return var;
}

void define_variable (CHAR_DATA *mob, MOBPROG_DATA *program, char *argument)
{
	char	var_type [MAX_STRING_LENGTH] = {'\0'};
	char	var_name [MAX_STRING_LENGTH] = {'\0'};
	int		type = 0;

	argument = one_argument (argument, var_type);

	if		( !str_cmp (var_type, "integer") )		type = MP_TYPE_INTEGER;
	else if ( !str_cmp (var_type, "char_data") )	type = MP_TYPE_CHAR_DATA;
	else if ( !str_cmp (var_type, "obj_data") )		type = MP_TYPE_OBJ_DATA;
	else if ( !str_cmp (var_type, "room_data") )	type = MP_TYPE_ROOM_DATA;
	else if ( !str_cmp (var_type, "string") )		type = MP_TYPE_STRING;
	else {
		system_log("Mob program with a variable problem.", TRUE);
		program->line = str_dup (current_line);
		SET_BIT (program->flags, MPF_BROKEN);
	}

	argument = one_argument (argument, var_name);

	if ( !*var_name ) {
		system_log("Variable name problem; no var_name.", TRUE);
		program->line = str_dup (current_line);
		SET_BIT (program->flags, MPF_BROKEN);
	}

	if ( !getvar (mob, var_name) )
		(void)setvar (mob, var_name, 0, type);
		
	return;
}

void mob_char_indirect (CHAR_DATA *mob, char *token, int *value, int *type)
{
	CHAR_DATA	*ch = NULL;
	CHAR_DATA	*person = NULL;
	int			i = 0;

	struct ind_data_t {
		int		which_case;
		char	ind [15];
		int		type;
	} ind_data [] = {
		{ 0,	"str",				MP_TYPE_INTEGER },
		{ 1,	"dex",				MP_TYPE_INTEGER },
		{ 2,	"int",				MP_TYPE_INTEGER },
		{ 3,	"con",				MP_TYPE_INTEGER },
		{ 4,	"wil",				MP_TYPE_INTEGER },
		{ 5,	"aur",				MP_TYPE_INTEGER },
		{ 6,	"hit",				MP_TYPE_INTEGER },
		{ 7,	"maxhit",			MP_TYPE_INTEGER },
		{ 8,	"moves",			MP_TYPE_INTEGER },
		{ 9,	"maxmoves",			MP_TYPE_INTEGER },
		{ 10,	"deity",			MP_TYPE_INTEGER },
		{ 11,	"circle",			MP_TYPE_INTEGER },
		{ 12,	"race",				MP_TYPE_INTEGER },
		{ 13,	"sex",				MP_TYPE_INTEGER },
		{ 14,	"clan_1",			MP_TYPE_INTEGER },
		{ 15,	"piety",			MP_TYPE_INTEGER },
		{ 16,	"offense",			MP_TYPE_INTEGER },
		{ 17,	"clan_2",			MP_TYPE_INTEGER },
		{ 18,	"fightmode",		MP_TYPE_INTEGER },
		{ 19,	"next_in_room",		MP_TYPE_CHAR_DATA },
		{ 20,	"name",				MP_TYPE_STRING },
		{ 21,	"level",			MP_TYPE_INTEGER },
		{ 22,   "fighting",			MP_TYPE_CHAR_DATA },
		{ 23,	"realname",			MP_TYPE_STRING },
		{ 24,	"in_room",			MP_TYPE_INTEGER },
		{ 25,	"equip",			MP_TYPE_OBJ_DATA },
		{ 26,	"inv",				MP_TYPE_OBJ_DATA },
		{ 27,	"reset_room",		MP_TYPE_INTEGER },
		{ 28,	"prisoner",			MP_TYPE_CHAR_DATA },
		{ 0,	"\0",				0 }
	};

	*type = -1;

	if ( !*value )
		return;

	ch = (CHAR_DATA *) *value;

	for ( i = 0; *ind_data [i].ind; i++ ) {
		if ( !str_cmp (ind_data [i].ind, token) )
			break;
	}

	if ( !*ind_data [i].ind )
		return;

	*type = ind_data [i].type;

	switch ( ind_data [i].which_case ) {
		case 0:		*value = GET_STR (ch);					break;
		case 1:		*value = GET_DEX (ch);					break;
		case 2:		*value = GET_INT (ch);					break;
		case 3:		*value = GET_CON (ch);					break;
		case 4:		*value = GET_WIL (ch);					break;
		case 5:		*value = GET_AUR (ch);					break;
		case 6:		*value = GET_HIT (ch);					break;
		case 7:		*value = GET_MAX_HIT (ch);				break;
		case 8:		*value = GET_MOVE (ch);					break;
		case 9:		*value = GET_MAX_MOVE (ch);				break;
		case 10:	*value = ch->deity;						break;
		case 11:	*value = ch->circle;					break;
		case 12:	*value = ch->race;						break;
		case 13:	*value = ch->sex;						break;
		case 14:	*value = 0; /* ch->clan_1; */			break;
		case 15:	*value = ch->ppoints;					break;
		case 16:	*value = ch->offense;					break;
		case 17:	*value = 0; /* ch->clan_2; */			break;
		case 18:	*value = ch->fight_mode;				break;
		case 19:	if ( !is_he_here (mob, ch, 0) )
						person = NULL;
					else {
						person = ch->next_in_room;

						while ( person && !CAN_SEE (mob, person) )
							person = person->next_in_room;
					}
	
					*value = (int) person;

					break;

		case 20:	*value = (int) ch->short_descr;			break;
		case 21:	*value = GET_TRUST (ch);				break;
		case 22:    *value = (int) ch->fighting;			break;
		case 23:	*value = (int) GET_NAME (ch);			break;
		case 24:	*value = ch->in_room;					break;
		case 25:	*value = (int) ch->equip;				break;
		case 26:	
				if ( ch->right_hand )
				*value = (int) ch->right_hand;					
				else
					*value = (int) ch->left_hand; break;
		case 27:	if ( ch->pc )
						*value = 0;

					else if ( ch->mob->reset_zone != 0 || ch->mob->reset_cmd != 0 )
						*value = zone_table [ch->mob->reset_zone].cmd
											[ch->mob->reset_cmd].arg1;
					else
						*value = 0;							break;
		case 28:	*value = (int) (IS_SUBDUER (ch) ?
										ch->subdue : NULL); break;
	}
}

void mob_obj_indirect (char *token, int *value, int *type)
{
	OBJ_DATA	*obj = NULL;
	int			i = 0;

	struct ind_data_t {
		int		which_case;
		char	ind [15];
		int		type;
	} ind_data [] = {
		{ 0,	"virtual",			MP_TYPE_INTEGER },
		{ 1,	"zone",				MP_TYPE_INTEGER },
		{ 2,	"in_room",			MP_TYPE_INTEGER },
		{ 3,	"name",				MP_TYPE_STRING },
		{ 4,	"short",			MP_TYPE_STRING },
		{ 5,	"carried_by",		MP_TYPE_CHAR_DATA },
		{ 6,	"equiped_by",		MP_TYPE_CHAR_DATA },
		{ 7,	"in_obj",			MP_TYPE_OBJ_DATA },
		{ 8,	"contains",			MP_TYPE_OBJ_DATA },
		{ 9,	"next_content",		MP_TYPE_OBJ_DATA },
		{ 10,	"location",			MP_TYPE_INTEGER },
		{ 11,	"contained_wt",		MP_TYPE_INTEGER },
		{ 12,	"obj_weight",		MP_TYPE_INTEGER },
		{ 13,	"total_weight",		MP_TYPE_INTEGER },
		{ 14,	"oval0",			MP_TYPE_INTEGER },
		{ 15,	"oval1",			MP_TYPE_INTEGER },
		{ 16,	"oval2",			MP_TYPE_INTEGER },
		{ 17,	"oval3",			MP_TYPE_INTEGER },
		{ 18,	"oval4",			MP_TYPE_INTEGER },
		{ 19,	"oval5",			MP_TYPE_INTEGER },
		{ 20,	"cost",				MP_TYPE_INTEGER },
		{ 0,	"\0",				0 }
	};

	*type = -1;

	if ( !*value )
		return;

	obj = (OBJ_DATA *) *value;

	for ( i = 0; *ind_data [i].ind; i++ ) {
		if ( !str_cmp (ind_data [i].ind, token) )
			break;
	}

	if ( !*ind_data [i].ind )
		return;

	*type = ind_data [i].type;

	switch ( ind_data [i].which_case ) {
		case 0:
			*value = obj->virtual;
			break;
		
		case 1:
			*value = obj->zone;
			break;
		
		case 2:	
			*value = obj->in_room;
			break;
		
		case 3:
			(void)one_argument (obj->name, s_buf);
			*value = (int) s_buf;
			break;
		
		case 4:
			*value = (int) obj->short_description;
			break;
		
		case 5:
			*value = (int) obj->carried_by;
			break;
		
		case 6:
			*value = (int) obj->equiped_by;
			break;
		
		case 7:
			*value = (int) obj->in_obj;
			break;
		
		case 8:	
			*value = (int) obj->contains;
			break;
		
		case 9:
			*value = (int) obj->next_content;
			break;
		
		case 10:
			*value = obj->location;
			break;
		
		case 11:
			*value = obj->contained_wt;
			break;
		
		case 12:
			*value = obj->obj_flags.weight;
			break;
		
		case 13:
			*value = OBJ_MASS (obj);
			break;
		
		case 14:
			*value = obj->o.od.value [0];
			break;
		
		case 15:
			*value = obj->o.od.value [1];
			break;
		
		case 16:
			*value = obj->o.od.value [2];
			break;
		
		case 17:
			*value = obj->o.od.value [3];
			break;
		
		case 18:
			*value = obj->o.od.value [4];
			break;
		
		case 19:
			*value = obj->o.od.value [5];
			break;
		
		case 20:
			*value = obj->obj_flags.cost;
			break;
	}
}

void mob_room_indirect (CHAR_DATA *mob, char *token, int *value, int *type)
{
	ROOM_DATA	*room = NULL;
	CHAR_DATA	*person = NULL;
	int			i = 0;

	struct ind_data_t {
		int		which_case;
		char	ind [15];
		int		type;
	} ind_data [] = {
		{ 0,	"people",			MP_TYPE_CHAR_DATA },
		{ 1,	"sector_type",		MP_TYPE_INTEGER },
		{ 2,	"virtual",			MP_TYPE_INTEGER },
		{ 3,	"zone",				MP_TYPE_INTEGER },
		{ 4,	"deity",			MP_TYPE_INTEGER },
		{ 5,	"contents",			MP_TYPE_OBJ_DATA },
		{ 6,	"room_flags",		MP_TYPE_INTEGER },
		{ 7,	"light",			MP_TYPE_INTEGER },
		{ 0,	"\0",				0}
	};

	*type = -1;

	if ( !*value )
		return;

	room = (ROOM_DATA *) *value;

	for ( i = 0; *ind_data [i].ind; i++ ) {
		if ( !str_cmp (ind_data [i].ind, token) )
			break;
	}

	if ( !*ind_data [i].ind )
		return;

	*type = ind_data [i].type;

	switch ( ind_data [i].which_case ) {
		case 0:		
					person = room->people;

					while ( person && !CAN_SEE (mob, person) )
						person = person->next_in_room;

					*value = (int) person;		break;

		case 1:		*value = room->sector_type; break;
		case 2:		*value = room->virtual; break;
		case 3:		*value = room->zone; break;
		case 4:		*value = room->deity; break;
		case 5:		*value = (int) room->contents; break;
		case 6:		*value = room->room_flags; break;
		case 7:		*value = room->light; break;
	}
	
	return;
}

void mob_int_indirect (char *token, int *value, int *type)
{
	if ( !str_cmp (token, "room") )	{
		*value = (int) vtor (*value);
		*type  = MP_TYPE_ROOM_DATA;
	}

	else if ( !str_cmp (token, "obj") ) {
		*value = (int) vtoo (*value);
		*type  = MP_TYPE_OBJ_DATA;
	}

	else if ( !str_cmp (token, "char") ) {
		*value = (int) vtom (*value);
		*type  = MP_TYPE_CHAR_DATA;
	}

	else {
		*value = 0;
		*type  = -1;
	}
	
	return;
}

CHAR_DATA *mp_whohasobj (CHAR_DATA *mob, int obj_virt)
{
	CHAR_DATA	*tch = NULL;
	OBJ_DATA	*tobj = NULL;
	ROOM_DATA	*room = NULL;

	room = vtor (mob->in_room);

	if ( !room )
		return NULL;

		/* If the character is hiding the object in another object,
           this routine will not find it */

	for ( tch = room->people; tch; tch = tch->next_in_room ) {

		if ( tch->right_hand && tch->right_hand->virtual == obj_virt && CAN_SEE_OBJ (mob, tch->right_hand) ) 
			return tch;

		if ( tch->left_hand && tch->left_hand->virtual == obj_virt && CAN_SEE_OBJ (mob, tch->left_hand) ) 
			return tch;

		for ( tobj = tch->equip; tobj; tobj = tobj->next_content )
			if ( tobj->virtual == obj_virt && CAN_SEE_OBJ (mob, tobj) )
				return tobj->equiped_by;
	}

	return NULL;
}

int mp_get_eval_token (CHAR_DATA *mob, char **p, int *token_type, int *value)
{
	VAR_DATA	*var  = NULL;
	char		token [MAX_INPUT_LENGTH];
	int			low_number = 0;
	int			high_number = 0;
	int			current_type = 0;
	int			current_value = 0;
	int			obj_virt = 0;
	CHAR_DATA	*tch1 = NULL;
	CHAR_DATA	*tch2 = NULL;

	*value = 0;
	*token_type = 0;

	while ( isspace (**p) )
		(*p)++;

		/* Number */

	if ( isdigit (**p) ) {
		while ( isdigit (**p) ) {
			*value = *value * 10 + ( **p - '0' );
			(*p)++;
		}
		*token_type = MP_TYPE_INTEGER;
		return 1;
	}

	if		( !strncmp (*p, "==", 2) ) *token_type = MP_EQUAL;
	else if ( !strncmp (*p, ">=", 2) ) *token_type = MP_GREATER_EQUAL;
	else if ( !strncmp (*p, "<=", 2) ) *token_type = MP_LESSER_EQUAL;
	else if ( !strncmp (*p, "!=", 2) ) *token_type = MP_NOT_EQUAL;
	else if ( !strncmp (*p, "<>", 2) ) *token_type = MP_NOT_EQUAL;
	else if ( !strncmp (*p, "&&", 2) ) *token_type = MP_AND;
	else if ( !strncmp (*p, "||", 2) ) *token_type = MP_OR;

	if ( *token_type ) {
		(*p) += 2;
		return 1;
	}

	if      ( **p == '+' ) *token_type = MP_PLUS;
	else if ( **p == '/' ) *token_type = MP_DIV;
	else if ( **p == '*' ) *token_type = MP_MULT;
	else if ( **p == '-' ) *token_type = MP_SUB;
	else if ( **p == '(' ) *token_type = MP_OPEN_PAR;
	else if ( **p == ')' ) *token_type = MP_CLOSE_PAR;
	else if ( **p == '<' ) *token_type = MP_LESS_THAN;
	else if ( **p == '>' ) *token_type = MP_GREATER_THAN;
	else if ( **p == '%' ) *token_type = MP_MOD;
	else if ( **p == ',' ) *token_type = MP_COMMA;

	if ( *token_type ) {
		(*p)++;
		return 1;
	}

	/* Numbers and operations are exhausted, try functions, then variables */

	if ( !mob_get_token (p, token, MT_VARIABLE) )
		return 0;

	if ( (var = getvar (mob, token)) ) {
		current_type = var->type;
		current_value = var->value;
	}

    else {
		if ( !str_cmp (token, "number") ) {

			require_open_paren (p);

			low_number = mp_eval_eq (mob, p);
			high_number = mp_eval_eq (mob, p);

			*value = number (low_number, high_number);
			*token_type = MP_TYPE_INTEGER;
		}

		else if ( !str_cmp (token, "dice") ) {

			require_open_paren (p);

			*value = dice (mp_eval_eq (mob, p), mp_eval_eq (mob, p));
			*token_type = MP_TYPE_INTEGER;
		}

		else if ( !str_cmp (token, "mud_hour") ) {
			*value = time_info.hour;
			*token_type = MP_TYPE_INTEGER;
		}

		else if ( !str_cmp (token, "mud_day") ) {
			*value = time_info.day;
			*token_type = MP_TYPE_INTEGER;
		}

		else if ( !str_cmp (token, "mud_month") ) {
			*value = time_info.month;
			*token_type = MP_TYPE_INTEGER;
		}

		else if ( !str_cmp (token, "mud_year") ) {
			*value = time_info.month;
			*token_type = MP_TYPE_INTEGER;
		}

		else if ( !str_cmp (token, "mud_minute") ) {
			*value = 4 * (15 * 60 - (next_hour_update - time (0))) / 60;
			*token_type = MP_TYPE_INTEGER;
		}

		else if ( !str_cmp (token, "is_following") ) {
			require_open_paren (p);

			tch1 = (CHAR_DATA *) mp_eval_eq (mob, p);
			tch2 = (CHAR_DATA *) mp_eval_eq (mob, p);

			*value = (tch2->following == tch1);
			*token_type = MP_TYPE_INTEGER;
		}

		else if ( !str_cmp (token, "is_wielding") ) {

			require_open_paren (p);

			tch1 = (CHAR_DATA *) mp_eval_eq (mob, p);

			if ( get_equip (tch1, WEAR_PRIM) ||
				 get_equip (tch1, WEAR_SEC) ||
				 get_equip (tch1, WEAR_BOTH) )
				*value = 1;
			else
				*value = 0;

			*token_type = MP_TYPE_INTEGER;

			require_open_paren (p);
		}

		else if ( !str_cmp (token, "whohasobj") ) {

			require_open_paren (p);

			tch1 = (CHAR_DATA *) mp_eval_eq (mob, p);

			obj_virt = mp_eval_eq (mob, p);

			*value = (int) mp_whohasobj (mob, obj_virt);
			*token_type = MP_TYPE_CHAR_DATA;
		}

		else if ( !str_cmp (token, "get_equip") ) {

			require_open_paren (p);

			tch1 = (CHAR_DATA *) mp_eval_eq (mob, p);

			if ( !mob_get_token (p, token, MT_VARIABLE) )
				return 0;

			*value = 0;

			if ( !str_cmp (token, "FINGER_R") )	*value = (int) get_equip (tch1, WEAR_FINGER_R);
			else if ( !str_cmp (token, "FINGER_L") )	*value = (int) get_equip (tch1, WEAR_FINGER_L);
			else if ( !str_cmp (token, "NECK_1") )		*value = (int) get_equip (tch1, WEAR_NECK_1);
			else if ( !str_cmp (token, "NECK_2") )		*value = (int) get_equip (tch1, WEAR_NECK_2);
			else if ( !str_cmp (token, "BODY_1") )		*value = (int) get_equip (tch1, WEAR_BODY_1);
			else if ( !str_cmp (token, "BODY_2") )		*value = (int) get_equip (tch1, WEAR_BODY_2);
			else if ( !str_cmp (token, "BODY_3") )		*value = (int) get_equip (tch1, WEAR_BODY_3);
			else if ( !str_cmp (token, "BODY_4") )		*value = (int) get_equip (tch1, WEAR_BODY_4);
			else if ( !str_cmp (token, "BODY_5") )		*value = (int) get_equip (tch1, WEAR_BODY_5);
			else if ( !str_cmp (token, "HEAD") )		*value = (int) get_equip (tch1, WEAR_HEAD);
			else if ( !str_cmp (token, "LEGS_1") )		*value = (int) get_equip (tch1, WEAR_LEGS_1);
			else if ( !str_cmp (token, "LEGS_2") )		*value = (int) get_equip (tch1, WEAR_LEGS_2);
			else if ( !str_cmp (token, "FEET_1") )		*value = (int) get_equip (tch1, WEAR_FEET_1);
			else if ( !str_cmp (token, "FEET_2") )		*value = (int) get_equip (tch1, WEAR_FEET_2);
			else if ( !str_cmp (token, "HANDS") )		*value = (int) get_equip (tch1, WEAR_HANDS);
			else if ( !str_cmp (token, "ARMS") )		*value = (int) get_equip (tch1, WEAR_ARMS);
			else if ( !str_cmp (token, "SHIELD") )		*value = (int) get_equip (tch1, WEAR_SHIELD);
			else if ( !str_cmp (token, "ABOUT") )		*value = (int) get_equip (tch1, WEAR_ABOUT);
			else if ( !str_cmp (token, "WAIST") )		*value = (int) get_equip (tch1, WEAR_WAIST);
			else if ( !str_cmp (token, "WRIST_R") )		*value = (int) get_equip (tch1, WEAR_WRIST_R);
			else if ( !str_cmp (token, "WRIST_L") )		*value = (int) get_equip (tch1, WEAR_WRIST_L);
			else if ( !str_cmp (token, "PRIM") )		*value = (int) get_equip (tch1, WEAR_PRIM);
			else if ( !str_cmp (token, "SEC") )			*value = (int) get_equip (tch1, WEAR_SEC);
			else if ( !str_cmp (token, "BOTH") )		*value = (int) get_equip (tch1, WEAR_BOTH);
			else if ( !str_cmp (token, "BELT_1") )		*value = (int) get_equip (tch1, WEAR_BELT_1);
			else if ( !str_cmp (token, "BELT_2") )		*value = (int) get_equip (tch1, WEAR_BELT_2);
			else if ( !str_cmp (token, "BACK") )		*value = (int) get_equip (tch1, WEAR_BACK);

			*token_type = MP_TYPE_OBJ_DATA;

			require_open_paren (p);		/* Looking for close paren, actually */

		}

		else 
			return 0;	/* Not a variable, and not a function or special var */

			/* If we get here, the function was evaluated. */

		current_type  = *token_type;
		current_value = *value;
	}

	while ( !strncmp (*p, "->", 2) ) {

		(*p) += 2;

		if ( !mob_get_token (p, token, MT_VARIABLE) )	/* looking after -> */
			return 0;

		if ( current_type == MP_TYPE_CHAR_DATA )
			mob_char_indirect (mob, token, &current_value, &current_type);
		else if ( current_type == MP_TYPE_ROOM_DATA )
			mob_room_indirect (mob, token, &current_value, &current_type);
		else if ( current_type == MP_TYPE_OBJ_DATA )
			mob_obj_indirect (token, &current_value, &current_type);
		else if ( current_type == MP_TYPE_INTEGER )
			mob_int_indirect (token, &current_value, &current_type);
		else
			return 0;

		if ( current_type == -1 )
			return 0;
	}

	*value = current_value;
	*token_type = current_type;

	return 1;
}

int mp_eval_eq (CHAR_DATA *mob, char **equation)
{
	int		token_type = 0;
	int		token_value = 0;
	int		left_side = 0;
	int		right_side = 0;
	int		sign_flag = 0;
	int		operation = 0;
	
	left_side = 0;
	operation = MP_PLUS;

	while ( **equation ) {

		sign_flag = -1;

		do {
			sign_flag = -sign_flag;

			if ( !mp_get_eval_token (mob, equation, &token_type, &token_value) )
				return -9998;

		} while ( token_type == MP_SUB );
		
		if ( token_type == MP_OPEN_PAR )
			right_side = mp_eval_eq (mob, equation);

		else if ( token_type == MP_TYPE_INTEGER ||
				  token_type == MP_TYPE_CHAR_DATA ||
				  token_type == MP_TYPE_OBJ_DATA ||
				  token_type == MP_TYPE_ROOM_DATA )
			right_side = token_value;

		else {
			printf ("Failure to understand token-type: %d:%d\n",
					token_value, token_type);
			return -9997;
		}

		right_side = right_side * sign_flag;

		switch (operation) {
			case MP_PLUS:	left_side = left_side + right_side;
							break;

			case MP_SUB:	left_side = left_side - right_side;
							break;

			case MP_DIV:	if ( !right_side ) 
								return -9996;
							left_side = left_side / right_side;
							break;

			case MP_MULT:	left_side = left_side * right_side;
							break;

			case MP_MOD:    left_side = left_side % right_side;
							break;

			case MP_LESS_THAN:
							left_side = (left_side < right_side);
							break;

			case MP_GREATER_THAN:
							left_side = (left_side > right_side);
							break;

			case MP_EQUAL:
							left_side = (left_side == right_side);
							break;

			case MP_GREATER_EQUAL:
							left_side = (left_side >= right_side);
							break;

			case MP_LESSER_EQUAL:
							left_side = (left_side <= right_side);
							break;

			case MP_NOT_EQUAL:
							left_side = (left_side != right_side);
							break;

			case MP_AND:
							left_side = (left_side && right_side) ? 1 : 0;
							break;

			case MP_OR:
							left_side = (left_side || right_side) ? 1 : 0;
							break;
		}

		if ( !mp_get_eval_token (mob, equation, &operation, &token_value) ||
			 operation == MP_CLOSE_PAR || operation == MP_COMMA )
			return left_side;

		if ( operation < MP_LESS_THAN || operation > MP_MULT )
			return -9995;
	}

	return left_side;
}

char *get_prog_token (char **buf, char *token)
{
	char	*p = NULL;

	p = token;

	while ( **buf == ' ' || **buf == '\n' || **buf == '\t' || **buf == '\r' )
		(*buf)++;

	while ( **buf && **buf != '\n' && **buf != ' ' && **buf != '\t' &&
			**buf != '\r' ) {
		*token++ = **buf;
		(*buf)++;
	}

	*token = '\0';

	return p;
}

char *get_line (char **buf, char *ret_buf)	/* Used by aliases too */
{
	char	*p = NULL;

	if ( !*buf )
		return NULL;

	while ( **buf == ' ' || **buf == '\n' || **buf == '\t' || **buf == '\r' )
		(*buf)++;

	p = *buf;

	while ( **buf && **buf != '\n' ) {
		*ret_buf++ = **buf;
		(*buf)++;
	}

	*ret_buf = '\0';

	return *p ? p : NULL;
}

void mob_string_token (char **argument, char *arg_first, int *quoted)
{
	char cEnd;

	while ( isspace( **argument ) )
		(*argument)++;

	cEnd = ' ';

	if ( **argument == '\'' || **argument == '"' ) {
		cEnd = **argument;
		(*argument)++;
		*quoted = 1;
	} else
		*quoted = 0;

	while ( **argument != '\0' ) {

		if ( **argument == cEnd ) {
			(*argument)++;
			break;
		}

		if ( cEnd == ' ' )
			*arg_first = LOWER (**argument);
		else
			*arg_first = **argument;

		arg_first++;
		(*argument)++;
	}

	*arg_first = '\0';

	while ( isspace( **argument ) )
		(*argument)++;

		/* If the next character is (, then we're probably dealing with a 
           function. */

	if ( **argument == '(' ) {

		*arg_first++ = ' ';       /* Make sure there is a space before ( */
        *arg_first = '\0';

		while ( **argument != '\0' && **argument != ')' ) {
			*arg_first++ = LOWER (**argument);
			(*argument)++;
		}

		(*argument)++;
		*arg_first++ = ')';
		*arg_first = '\0';
	}

	while ( isspace( **argument ) )
		(*argument)++;

		/* If the next characters are -> then we are dereferencing a variable
           or function */

	if ( **argument == '-' && *(*argument + 1) == '>' ) {

		(*argument)++;
		(*argument)++;
		*arg_first++ = '-';
		*arg_first++ = '>';
		*arg_first = '\0';

		while ( **argument != '\0' && **argument != ')' && **argument != ' ' ) {
			*arg_first++ = LOWER (**argument);
			(*argument)++;
		}

		*arg_first = '\0';
	}
}

void mob_string (CHAR_DATA *mob, char **p, char *string)
{
	int			type = 0;
	int			value = 0;
	int			quoted = 0;
	char		token [MAX_STRING_LENGTH] = {'\0'};
	char 		buf [MAX_STRING_LENGTH] = {'\0'};
	char		**tmp_ptr;
	char		*tptr = NULL;

	*string = '\0';

	mob_string_token (p, token, &quoted);

	while ( *token ) {

		if ( quoted )
			strcat (string, token);
		else {

			tptr = token;
			tmp_ptr = &tptr;
			
			if ( !mp_get_eval_token (mob, tmp_ptr, &type, &value) ) {
				printf ("Failed to get token ;(\n");
				fflush (stdout);
				snprintf (buf, MAX_STRING_LENGTH,  "Failed to get mobprog token for mob VNUM %d.", mob->mob->virtual);
				system_log (buf, TRUE);
				return;
			}

			if ( type == MP_TYPE_INTEGER )
				snprintf (string + strlen (string), MAX_STRING_LENGTH, "%d", value);
			else if ( type == MP_TYPE_STRING )
				strcat (string, (char *) value);
			else {
				snprintf (string + strlen (string), MAX_STRING_LENGTH, "?%d:%d?", value, type);
				return;
			}
		}

		mob_string_token (p, token, &quoted);
	}
}

void assignment (CHAR_DATA *mob, MOBPROG_DATA *program, char *target_name, char **p)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	VAR_DATA	*target;

	if ( !(target = getvar (mob, target_name)) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Assignment to unknown variable: %s", target_name);
		system_log (buf, TRUE);
		snprintf (buf, MAX_STRING_LENGTH,  "trigger %s, mob %d", program->trigger_name, mob->mob->virtual);
		system_log (buf, TRUE);
		program->line = str_dup (current_line);
		SET_BIT (program->flags, MPF_BROKEN);
		return;
	}

	if ( !mob_get_token (p, buf, MT_EQUAL) ) {
		system_log("Assignment needs equal.", TRUE);
		program->line = str_dup (current_line);
		SET_BIT (program->flags, MPF_BROKEN);
		return;
	}

	if ( target->type == MP_TYPE_INTEGER ) {
		target->value = mp_eval_eq (mob, p);
		return;
	}

	if ( target->type == MP_TYPE_CHAR_DATA ||
		 target->type == MP_TYPE_OBJ_DATA  ||
		 target->type == MP_TYPE_ROOM_DATA ) {
		target->value = mp_eval_eq (mob, p);
		return;
	}

	else if ( target->type == MP_TYPE_STRING ) {

		mob_string (mob, p, buf);
		
		if ( target->value )
			mem_free ((char *) target->value);

		target->value = (int) str_dup (buf);
	}
}

#define MAX_PROG_LINES	1000
#define MAX_DEPTH	500

void mobprog (CHAR_DATA *ch, CHAR_DATA *mob, MOBPROG_DATA *program, int trigger,
			  char *argument, int *ret)
{
	char	line [MAX_STRING_LENGTH] = {'\0'};
	char	command [MAX_STRING_LENGTH] = {'\0'};
	char	*line_ptr = NULL;
	char	*prog_ptr = NULL;
	char	*prog = NULL;
	char	token [MAX_STRING_LENGTH] = {'\0'};
	int		levels [MAX_PROG_LINES];
	char	*level_ptrs [MAX_PROG_LINES];
	int		level_ifs [MAX_DEPTH];
	int		lines;
	int		level;
	int		cur_level;
	int		line_no;
	int		i;
	int		temp_arg = 0;
	
	return;

	if ( ch->desc ) {
		*ret = 0;
		return;
	}

	if ( port == BUILDER_PORT )
		return;

	prog = program->prog;

	for ( lines = 0; lines < MAX_PROG_LINES; lines++ ) {
		levels [lines] = 0;
		level_ptrs [lines] = NULL;
	}

	(void)setvar (mob, "mob", (int) mob, MP_TYPE_CHAR_DATA);
	(void)setvar (mob, "ch", (int) ch, MP_TYPE_CHAR_DATA);
	
	temp_arg = (int) vtor (mob->in_room);
	(void)setvar (mob, "room", temp_arg, MP_TYPE_ROOM_DATA);

	prog_ptr = prog;

	level = 0;
	lines = 0;

	while ( (level_ptrs [lines] = get_line (&prog_ptr, line)) ) {
		
		line_ptr = line;

		(void)get_prog_token (&line_ptr, token);

		if ( !str_cmp (token, "if") || !str_cmp (token, "while") )
			levels [lines] = level++;

		else if ( !str_cmp (token, "elseif") ||
				  !str_cmp (token, "else") )
			levels [lines] = level - 1;

		else if ( !str_cmp (token, "endif") || !str_cmp (token, "endwhile") )
			levels [lines] = --level;

		else
			levels [lines] = level;

		lines++;
	}

	line_no = 0;

			/* Re-entry for a delayed trigger */

	if ( ch->trigger_id == trigger ) {

			/* Reset delayed trigger if trigger re-activated (vs
			   the delay timer expired) */

		if ( !ch->trigger_delay ) {

				/* Make it so all ifs (elseifs..) appear executed */

			for ( i = 0; i < MAX_DEPTH; i++ )
				level_ifs [i] = 1;

			line_no = ch->trigger_line;
		}

		ch->trigger_id    = 0;
		ch->trigger_delay = 0;
	}

	prog_ptr = level_ptrs [line_no];

	while ( get_line (&prog_ptr, line) ) {

		strcpy (current_line, line);

		if ( GET_POS (ch) == POSITION_DEAD ) {
			*ret = 0;
			return;
		}

		line_ptr = line;

		(void)get_prog_token (&line_ptr, token);

/******TODO figure out what to do in the following cases 
		if ( *token == '!' )
			;

		else if ( !*token || *token == '\n' || *token == '\r' || *token == ' ' )
			;
		else 
*********************/
		if ( !str_cmp (token, "ci") ) {

			mob_string (mob, &line_ptr, command);

			if ( *command )
				command_interpreter (mob, command);

			if ( mob->deleted )
				return;

			if ( mob->delay ||
				 GET_FLAG (mob, FLAG_ENTERING) ||
				 GET_FLAG (mob, FLAG_LEAVING) ) {

				SET_BIT (mob->flags, FLAG_INHIBITTED);
				mob->trigger_delay = 1;
				mob->trigger_line  = line_no + 1;
				mob->trigger_id    = trigger;
				return;
			}
		}

		else if ( !str_cmp (token, "global") || !str_cmp (token, "var") )
			define_variable (mob, program, line_ptr);

		else if ( !str_cmp (token, "while") ) {

			require_open_paren (&line_ptr);

			cur_level = levels [line_no];

			if ( !mp_eval_eq (mob, &line_ptr) ) {

				line_no++;

				while ( levels [line_no] != cur_level )
					line_no++;
			}
		}

		else if ( !str_cmp (token, "if") || !str_cmp (token, "elseif") ) {

			cur_level = levels [line_no];

			if ( !str_cmp (token, "if") )
				level_ifs [cur_level] = 0;

			if ( !level_ifs [cur_level] )
				require_open_paren (&line_ptr);

			if ( level_ifs [cur_level] || !mp_eval_eq (mob, &line_ptr) ) {

				while ( levels [line_no + 1] != cur_level )
					line_no++;

			} else
				level_ifs [cur_level] = 1;
		}

		else if ( !str_cmp (token, "else") ) {

			cur_level = levels [line_no];

			if ( level_ifs [cur_level] ) {

				while ( levels [line_no + 1] != cur_level )
					line_no++;
			}
		}

		else if ( !str_cmp (token, "reject_command") ) {
			*ret = 0;
			return;
		}

		else if ( !str_cmp (token, "endwhile") ) {

			cur_level = levels [line_no];

			if ( line_no > 0 )  /* If there is an endwhile on the first line */
				line_no--;

			while ( line_no && levels [line_no] != cur_level )
				line_no--;

			line_no--;		/* Gets incremented at the end of the while */
		}

/** TODO find out what to do with this statement **/
		/* else if ( !str_cmp (token, "endif") ) */
			/** do nothing in this case**/
			/* ; */

		else if ( !str_cmp (token, "delay") ) {

				/* A previously delayed trigger is forgotten */

			mob->trigger_delay = mp_eval_eq (mob, &line_ptr);
			mob->trigger_line  = line_no + 1;
			mob->trigger_id    = trigger;
			return;
		}

		else if ( !str_cmp (token, "alarm") )
			mob->alarm = mp_eval_eq (mob, &line_ptr);

		else if ( !str_cmp (token, "return") )
			return;

		else
			assignment (mob, program, token, &line_ptr);

		prog_ptr = level_ptrs [++line_no];
	}
	
	return;
}

/* The trigger command is activated for mobs in the room that ch occupies. */ 

int trigger (CHAR_DATA *ch, char *argument, int trigger)
{
	CHAR_DATA		*mob = NULL;
	ROOM_DATA		*room = NULL;
	MOBPROG_DATA	*prog = NULL;
	int				ret = 1;	/* if false, no more progs, don't do command */

	room = ch->room;

	if ( !room )
		return 1;

	if ( trigger == TRIG_MOBACT || trigger == TRIG_HOUR ||
		 trigger == TRIG_DAY    || trigger == TRIG_ALARM ||
		 (trigger == ch->trigger_id && !ch->trigger_delay) ) {

		for ( prog = ch->prog; prog; prog = prog->next )
			if ( !str_cmp (prog->trigger_name, mobprog_triggers [trigger]) &&
				 !IS_SET (prog->flags, MPF_BROKEN) )
				mobprog (ch, ch, prog, trigger, argument, &ret);

		return ret;
	}

	for ( mob = room->people; mob && ret; mob = mob->next_in_room ) {

		if ( mob == ch )
			continue;

		for ( prog = mob->prog; prog && ret; prog = prog->next ) {
			fflush (stdout);
			if ( !str_cmp (prog->trigger_name, mobprog_triggers [trigger]) &&
				 !IS_SET (prog->flags, MPF_BROKEN) )
				mobprog (ch, mob, prog, trigger, argument, &ret);
		}
	}

	return ret;
}

void add_replace_mobprog_data (CHAR_DATA *ch, CHAR_DATA *mob,
							   char *trigger_name, char *prog_data)
{
	MOBPROG_DATA	*prog = NULL;
	MOBPROG_DATA	*last_prog = NULL;

	for ( prog = mob->prog; prog; prog = prog->next ) {

		last_prog = prog;

		if ( !str_cmp (prog->trigger_name, trigger_name) )
			break;
	}

	if ( !prog ) {
		prog = malloc (sizeof (MOBPROG_DATA));

		if ( last_prog )
			last_prog->next = prog;
		else
			mob->prog = prog;

		prog->trigger_name = str_dup (trigger_name);
		prog->next = NULL;

	} else
		mem_free (prog->prog);

	prog->prog = str_dup (prog_data);
	prog->busy = 0;
	
	return;
}

void do_prog (CHAR_DATA *ch, char *argument, int cmd)
{
	int			ind = 0;
	int			got_line = 0;
	char		file_name [MAX_INPUT_LENGTH];
	char		*prog_data = NULL;
	char		*trigger_name = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		subcmd [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*edit_mob = NULL;
	CHAR_DATA	*tch = NULL;
	MOBPROG_DATA *prog = NULL;
	FILE		*mp = NULL;
	FILE		*fp = NULL;
	CHAR_DATA	*temp_char = NULL;

	if ( IS_NPC (ch) ) {
		send_to_char ("This is a PC only command.\n\r", ch);
		return;
	}

	if ( !(edit_mob = vtom (ch->pc->edit_mob)) ) {
		edit_mob = ch->pc->edit_player;
		if ( !(ch->pc->edit_player) || !(edit_mob) ) {
			send_to_char ("Start by using the MOBILE command.\n\r", ch);
			return;
		}
	} 

	if ( !IS_NPC (edit_mob) ) {
		send_to_char ("Too dangerous to use this on a PC.  Try a mob.\n", ch);
		return;
	}

	argument = one_argument (argument, subcmd);

	if ( !*subcmd || !str_cmp (subcmd, "?") ) {
		send_to_char ("\n\r", ch);
		send_to_char ("prog clear <trigger>- reset error flags on mobs triggers\n\r", ch);
		send_to_char ("prog load <name>    - load program from lib/mobprogs dir\n\r", ch);
		send_to_char ("prog save           - save ALL programs to mobprogs.0\n\r", ch);
		send_to_char ("prog look <name>    - read a program in lib/mobprogs dir\n\r", ch);
		send_to_char ("prog list           - listing of lib/mobprogs directory\n\r", ch);
		send_to_char ("prog errors         - listing of disabled mob progs\n\r", ch);
		send_to_char ("prog <trigger>      - lists trigger for current mob\n\r", ch);
		return;
	}

	else if ( !str_cmp (subcmd, "load") ) {

		argument = one_argument (argument, file_name);

		if ( file_name [0] == '.' || file_name [0] == '/' ) {
			send_to_char ("Sorry, your programs must be located in the "
						  "lib/mobprogs directory.\n\r", ch);
			send_to_char ("This enforces security.\n\r", ch);
			return;
		}

		snprintf (buf, MAX_STRING_LENGTH,  "mobprogs/%s", file_name);

		if ( !(mp = fopen (buf, "r")) ) {
			send_to_char ("Unable to open file.\n\r", ch);
			return;
		}

		while ( *(trigger_name = fread_string (mp)) ) {

			prog_data = fread_string (mp);
	
			snprintf (buf, MAX_STRING_LENGTH,  ". . . Adding program %s\n\r", trigger_name);
			send_to_char (buf, ch);

			add_replace_mobprog_data (ch, edit_mob, trigger_name, prog_data);
		}

		fclose (mp);
	}

	else if ( (ind = index_lookup (mobprog_triggers, subcmd)) != -1 ) {

		for ( prog = edit_mob->prog; prog; prog = prog->next )
			if ( !str_cmp (prog->trigger_name, mobprog_triggers [ind]) ) {
				page_string (ch->desc, prog->prog);
				break;
			}

		if ( !prog )
			send_to_char ("No such program defined.\n\r", ch);
	}

	else if ( !str_cmp (subcmd, "clear") ) {

		argument = one_argument (argument, subcmd);

		if ( (ind = index_lookup (mobprog_triggers, subcmd)) == -1 ) {
			send_to_char ("No such trigger.\n\r", ch);
			return;
		}

		for ( prog = edit_mob->prog; prog; prog = prog->next )
			if ( !str_cmp (prog->trigger_name, mobprog_triggers [ind]) ) {
				REMOVE_BIT (prog->flags, MPF_BROKEN);
				return;
			}
	}

	else if ( !str_cmp (subcmd, "save") ) {
		
		if ( !mp_dirty )
			send_to_char ("Mob programs don't really need writing...\n\r", ch);
		
		if ( !(mp = open_and_rename (ch, "mobprogs", 0)) ) {
			send_to_char ("Unable to open mobprogs!\n\r", ch);
			return;
		}
		
		mp_dirty = 0;

		for ( tch = full_mobile_list; tch; tch = tch->mob->lnext ) {
			for ( prog = tch->prog; prog; prog = prog->next ) {
				fprintf (mp, "#%d\n", tch->mob->virtual);
				fprintf (mp, "%s~\n", prog->trigger_name);
				fprintf (mp, "%s~\n", prog->prog);
			}
		}

		fprintf (mp, "$~\n");
		fclose (mp);
	}

	else if ( !str_cmp (subcmd, "look") ) {

		argument = one_argument (argument, subcmd);

		if ( subcmd [0] == '.' || subcmd [0] == '/' ) {
			send_to_char ("Oh! Oh!  You, you! ... That's what you are!\n\r", ch);
			return;
		}

		snprintf (buf, MAX_STRING_LENGTH,  "mobprogs/%s", subcmd);

		if ( !(fp = fopen (buf, "r")) ) {
			send_to_char ("Sorry.  I couldn't open that file.\n\r", ch);
			return;
		}

		while ( fgets (buf, 132, fp) ) {
			got_line = 1;
			send_to_char (buf, ch);
		}

		fclose (fp);
	}

	else if ( !str_cmp (subcmd, "errors") ) {
		for ( prog = full_prog_list; prog; prog = prog->next_full_prog ) {
			if ( IS_SET (prog->flags, MPF_BROKEN) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Mob %-5d  %-10s  $N",
							  prog->mob_virtual, prog->trigger_name);
				temp_char = vtom (prog->mob_virtual);
				act (buf, TRUE, ch, 0, temp_char, TO_CHAR);
				snprintf (buf, MAX_STRING_LENGTH,  "   %s\n", prog->line);
				send_to_char (buf, ch);
			}
		}
	}

	else
		send_to_char ("Unknown keyword.\n\r", ch);

	redefine_mobiles (edit_mob);
	
	return;
}

void delayed_trigger_activity ()
{
	CHAR_DATA		*ch = NULL;

	for ( ch = character_list; ch; ch = ch->next ) {

		if ( ch->deleted )
			continue;

		if ( GET_FLAG (ch, FLAG_INHIBITTED) ) {

			if ( !GET_FLAG (ch, FLAG_ENTERING) &&
				 !GET_FLAG (ch, FLAG_LEAVING) &&
				 !ch->delay )
				REMOVE_BIT (ch->flags, FLAG_INHIBITTED);
			else
				continue;
		}

		if ( ch->trigger_delay && !--(ch->trigger_delay) )
			trigger (ch, "", ch->trigger_id);
	}
	
	return;
}

void boot_mobprogs ()
{
	int				virtual = 0;
	char			file_name [MAX_STRING_LENGTH] = {'\0'};
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	MOBPROG_DATA	*prog = NULL;
	MOBPROG_DATA	*last_prog = NULL;
	CHAR_DATA		*mob = NULL;
	FILE			*mp = NULL;
	char			*temp_arg = NULL;

	snprintf (file_name, MAX_STRING_LENGTH, "%s/mobprogs.0", REGIONS);

	if ( !(mp = fopen (file_name, "r")) ) {
		perror ("Unable to open mobprogs.0");
		system_log("ERROR:  NO MOB PROGRAMS WERE LOADED!", TRUE);
		return;
	}

	while ( 1 ) {
		if ( !fgets (buf, 81, mp) ) {
			system_log("Error reading mobprog file:", TRUE);
			perror ("Reading mobprog");
			abort();
		}

		if ( *buf == '#' ) {
			sscanf (buf, "#%d", &virtual);

			if ( !(mob = vtom (virtual)) ) {
				temp_arg = fread_string (mp);
				snprintf (buf, MAX_STRING_LENGTH,  "Mob prog %s lost because mob %d doesn't exist.", temp_arg, virtual);
				system_log(buf, TRUE);
				(void)fread_string (mp);
				continue;
			}

			for ( last_prog = mob->prog;
				  last_prog && last_prog->next;
				  last_prog = last_prog->next )
				;

			prog = malloc (sizeof (MOBPROG_DATA));

			if ( last_prog )
				last_prog->next = prog;
			else
				mob->prog = prog;

			prog->trigger_name	= fread_string (mp);
			prog->prog			= fread_string (mp);
			prog->next			= NULL;
			prog->busy			= 0;
			prog->mob_virtual	= mob->mob ? mob->mob->virtual : -1;

				/* Adding a mob program to this list THIS way will
				   reverse the order from the file, hopefully this is ok. */

			prog->next_full_prog = full_prog_list;
			full_prog_list = prog;

		} else if ( *buf == '$' )
			break;
	}

	fclose (mp);
}

