/** 
*	\file hash.c
*	Central Hash Module
*
*	This module reads object and mobile information from files, loads objects
*	and mobiles, sets up spawn points and refreshes zones.
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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

extern struct 	char_data *character_list;
extern struct 	obj_data *object_list;
extern struct 	zone_data *zone_table;
extern char		*null_string;


CHAR_DATA *fread_mobile (int vnum, int zone, FILE *fp)
{
	int				i = 0;
	long			tmp = 0;
	long			tmp2 = 0;
	long			tmp3 = 0;
	long			clan1 = 0;
	long			clan2 = 0;
	long			num = 0;
	char			*p = NULL;
	char			*p2 = NULL;
	ROOM_DATA		*room = NULL;
	CHAR_DATA		*mob = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	int				mob_skills [MAX_SKILLS];
	char			peak_char = {'\0'};


	mob = new_char (0);		/* NPC */

	clear_char (mob);
	mob->mob->currency_type		= 0;	

	mob->mob->virtual		= vnum;
	mob->mob->zone			= zone;

	mob->mob->carcass_vnum		= 0;

#define CHECK_DOUBLE_DEFS 1
#ifdef CHECK_DOUBLE_DEFS
	if ( vtom (vnum) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Mob %d multiply defined!!", vnum);
		system_log(buf, TRUE);
	} else
#endif

	add_mob_to_hash (mob);

	mob->name = fread_string (fp);

	(void)one_argument (mob->name, buf);

	mob->tname = add_hash (CAP(buf));

	while ( (i = strlen (mob->name)) > 0 &&
			(mob->name [i - 1] == '\r' ||
			 mob->name [i - 1] == '\n') )
		mob->name [i - 1] = '\0';

	mob->short_descr	= fread_string (fp);

	while ( (i = strlen (mob->short_descr)) > 0 &&
			(mob->short_descr [i - 1] == '\r' ||
			 mob->short_descr [i - 1] == '\n') )
		mob->short_descr [i - 1] = '\0';

	mob->long_descr	= fread_string (fp);

	while ( (i = strlen (mob->long_descr)) > 0 &&
			(mob->long_descr [i - 1] == '\r' ||
			 mob->long_descr [i - 1] == '\n') )
		mob->long_descr [i - 1] = '\0';

	mob->description	= fread_string (fp);

	fscanf (fp, "%lu ", &tmp);
	mob->act = tmp;
	SET_BIT (mob->act, ACT_ISNPC);

	fscanf(fp, " %ld ", &tmp);
	mob->affected_by = tmp;

	fscanf(fp, " %ld ", &tmp);
	mob->offense = (int)tmp;
	fscanf(fp, " %ld ", &tmp);		/* Was defense */
	mob->race = (int) tmp;
	fscanf(fp, " %ld ", &tmp);
	mob->armor = (int)tmp;

	/* Need to reformat the following -- only need one var in the mob file for hp */

	fscanf(fp, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
	mob->max_hit = (int)tmp3;
	mob->hit = mob->max_hit;

	fscanf(fp, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
	mob->mob->damroll = (int)tmp3;
	mob->mob->damnodice = (int)tmp;
	mob->mob->damsizedice = (int)tmp2;

	mob->move = 50;
	mob->max_move = 50;

	fscanf (fp, " %ld ", &mob->time.birth);

	fscanf(fp, " %ld ", &tmp);
	mob->position = (int)tmp;

	fscanf(fp, " %ld ", &tmp);
	mob->default_pos = (int)tmp;

	fscanf(fp, " %ld ", &tmp);
	mob->sex = (int)tmp;

	fscanf (fp, " %ld ", &tmp);		/* Used for Regi's 7 econs for now*/
	mob->mob->merch_seven = (int)tmp;

	fscanf(fp, " %ld ", &tmp);
	mob->deity = (int)tmp;

	fscanf(fp, " %ld ", &tmp);		/* phys?  what's that? */
	mob->mob->vehicle_type = tmp;

	fscanf(fp, " %ld \n", &tmp);
	mob->hmflags = (int)tmp;

	fscanf (fp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
			&mob->mob->skinned_vnum,
			&mob->circle,
			&mob->cell_1,
			&mob->mob->carcass_vnum,	
			&mob->cell_2,				/*Formerly defense_bonus - free */
			&mob->ppoints,
			&mob->natural_delay,
			&mob->mob->helm_room,
			&mob->body_type,
			&mob->poison_type,
			&mob->nat_attack_type,
			&mob->mob->access_flags,
			&mob->height,
			&mob->frame,
			&mob->mob->noaccess_flags,
			&mob->cell_3);

	fscanf (fp, "%d %d %d %d %d %d %d %d\n",
			&mob->str,
			&mob->intel,
			&mob->wil,
			&mob->aur,
			&mob->dex,
			&mob->con,
			&mob->speaks,
			&mob->agi);
 
	if (  mob->race == RACE_HUMAN) {
		  /* Humanoid NPCs. */
		mob->max_hit = 50 + mob->con * CONSTITUTION_MULTIPLIER;
		mob->hit = mob->max_hit;
	}

	fscanf (fp, "%d %d\n", &mob->flags, &mob->mob->currency_type);

	if ( IS_SET (mob->flags, FLAG_KEEPER) ) {

		mob->shop = get_perm (sizeof (SHOP_DATA));

		(void)fgets (buf, 256, fp);

		sscanf (buf, "%d %d %f %f %f %f %d\n",
				&mob->shop->shop_vnum,
				&mob->shop->store_vnum,
				&mob->shop->markup,
				&mob->shop->discount,
				&mob->shop->econ_markup1,
				&mob->shop->econ_discount1,
				&mob->shop->econ_flags1);

		if (mob->mob->merch_seven > 0) {
			fscanf (fp, "%f %f %d %f %f %d %f %f %d %f %f %d %f %f %d %f %f %d %d\n",
				&mob->shop->econ_markup2,
                                &mob->shop->econ_discount2,
                                &mob->shop->econ_flags2,
                                &mob->shop->econ_markup3,
                                &mob->shop->econ_discount3,
                                &mob->shop->econ_flags3,
                                &mob->shop->econ_markup4,
                                &mob->shop->econ_discount4,
                                &mob->shop->econ_flags4,
                                &mob->shop->econ_markup5,
                                &mob->shop->econ_discount5,
                                &mob->shop->econ_flags5,
                                &mob->shop->econ_markup6,
                                &mob->shop->econ_discount6,
                                &mob->shop->econ_flags6,
                                &mob->shop->econ_markup7,
                                &mob->shop->econ_discount7,
                                &mob->shop->econ_flags7,
				&mob->shop->nobuy_flags);
		} 
		else {
			fscanf (fp, "%f %f %d %f %f %d %d\n",
				&mob->shop->econ_markup2,
				&mob->shop->econ_discount2,
				&mob->shop->econ_flags2,
				&mob->shop->econ_markup3,
				&mob->shop->econ_discount3,	
				&mob->shop->econ_flags3,
				&mob->shop->nobuy_flags);
		}

		for ( i = 0; i <= MAX_DELIVERIES; i++ ) {
			num = fread_number(fp);
			if ( num == -1 )
				break;
			mob->shop->delivery [i] = num;
		}

		fscanf (fp, "%d %d %d %d %d %d %d %d %d %d\n",
				&mob->shop->trades_in [0],
				&mob->shop->trades_in [1],
				&mob->shop->trades_in [2],
				&mob->shop->trades_in [3],
				&mob->shop->trades_in [4],
				&mob->shop->trades_in [5],
				&mob->shop->trades_in [6],
				&mob->shop->trades_in [7],
				&mob->shop->trades_in [8],
				&mob->shop->trades_in [9]);

		if ( mob->shop->store_vnum && (room = vtor (mob->shop->store_vnum)) ) {
			SET_BIT (room->room_flags, STORAGE);
		}
	}
		
	for ( i = 0; i < MAX_SKILLS; i++ )
		mob_skills [i] = 0;

	for ( i = 0; i < (MAX_SKILLS / 10); i++ )
		fscanf (fp, "%d %d %d %d %d %d %d %d %d %d\n",
					&mob_skills [i * 10],
					&mob_skills [i * 10 + 1],
					&mob_skills [i * 10 + 2],
					&mob_skills [i * 10 + 3],
					&mob_skills [i * 10 + 4],
					&mob_skills [i * 10 + 5],
					&mob_skills [i * 10 + 6],
					&mob_skills [i * 10 + 7],
					&mob_skills [i * 10 + 8],
					&mob_skills [i * 10 + 9]);

	for ( i = 0; i < MAX_SKILLS; i++ ){
		mob->skills [i] = mob_skills [i];
	}
	

	mob->clans = fread_string (fp);
	
/*** Lua triggers ***/
   do {
	   while ( (peak_char = getc (fp)) == ' ' ||
			   peak_char == '\t' ||
			   peak_char == '\n')
		   ;
			   
		ungetc (peak_char, fp);
			   
		if ( peak_char != 'R' )
		   break;
			   
		larg_setup_mob_triggers(fp, mob);
			   
   } while (1);
		   
/***** end lua triggers ***/	
		   
	mob->time.played = 0;
	mob->time.logon  = time(0);

	mob->intoxication = 0;
	mob->hunger       = -1;
	mob->thirst       = -1;

	mob->tmp_str	= mob->str;
	mob->tmp_dex	= mob->dex;
	mob->tmp_intel	= mob->intel;
	mob->tmp_aur	= mob->aur;
	mob->tmp_wil	= mob->wil;
	mob->tmp_con	= mob->con;
	mob->tmp_agi	= mob->agi;

	mob->equip = NULL;

	mob->mob->virtual = vnum;

	mob->desc = 0;

	if ( mob->speaks == 0 ) {
		mob->skills [SKILL_SPEAK_WESTRON] = 100;
		mob->speaks = SKILL_SPEAK_WESTRON;
	}

	if ( !mob->skills [mob->speaks] )
		mob->skills [mob->speaks] = 100;

	if ( IS_SET (mob->act, ACT_DONTUSE) ) {
		SET_BIT (mob->flags, FLAG_AUTOFLEE);
		REMOVE_BIT (mob->act, ACT_DONTUSE);
	}

	p = mob->clans;
	p2 = p;
	mob->clans = str_dup ("");

	while ( *p2 ) {

		p = one_argument (p, buf);		/* flags     */
		p = one_argument (p, buf2);		/* clan name */

		if ( !*buf2 )
			break;

		add_clan_id (mob, buf2, buf);
	}

	if ( p2 && *p2 )
		mem_free (p2);

	if ( clan1 == 1 ) {
		clan1 = 0;
		SET_BIT (mob->act, ACT_WILDLIFE);
	}

	if ( clan2 == 1 ) {
		clan2 = 0;
		SET_BIT (mob->act, ACT_WILDLIFE);
	}

	if ( clan1 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%ld", clan1);
		add_clan_id (mob, buf,
						  GET_FLAG (mob, FLAG_LEADER_1) ? "leader" : "member");
		REMOVE_BIT (mob->flags, FLAG_LEADER_1);
	}

	if ( clan2 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%ld", clan2);
		add_clan_id (mob, buf,
						  GET_FLAG (mob, FLAG_LEADER_2) ? "leader" : "member");
		REMOVE_BIT (mob->flags, FLAG_LEADER_2);
	}

	fix_offense (mob);

	mob->max_mana = mob->aur * 5;
	mob->mana = mob->max_mana;

	return (mob);
}

CHAR_DATA *load_mobile (int vnum)
{
	CHAR_DATA		*proto = NULL;
	CHAR_DATA		*new_mobile = NULL;
	MOB_DATA		*mob_info = NULL;

	if ( !(proto = vtom (vnum)) )
		return NULL;


	new_mobile = new_char (0);		/* NPC */

	mob_info = new_mobile->mob;

	memcpy (new_mobile, proto, sizeof (CHAR_DATA));

	new_mobile->mob = mob_info;

	memcpy (new_mobile->mob, proto->mob, sizeof (MOB_DATA));

	/* A mostly unique number.  Can be used to ensure
	   the same mobile is being used between game plays.  */

	new_mobile->coldload_id = get_next_coldload_id (0);
	new_mobile->deleted = 0;

	new_mobile->next = character_list;
	character_list = new_mobile;

	new_mobile->time.birth = time (0);

	new_mobile->max_move = calc_lookup (new_mobile, REG_MISC, MISC_MAX_MOVE);

	if ( !new_mobile->height )
		make_height (new_mobile, 1);

	if ( !new_mobile->frame )
		make_frame (new_mobile);

        	
	if ( IS_SET (new_mobile->affected_by, AFF_HIDE) )
		magic_add_affect (new_mobile, MAGIC_HIDDEN, -1, 0, 0, 0, 0);

	new_mobile->fight_mode = 2;

	if ( IS_SET (new_mobile->flags, FLAG_VARIABLE) ) {
		randomize_mobile (new_mobile);
		REMOVE_BIT (new_mobile->flags, FLAG_VARIABLE);
	}

	new_mobile->clans = str_dup(proto->clans);

	new_mobile->move = new_mobile->max_move;

	new_mobile->mount = NULL;
	new_mobile->wounds = NULL;
	new_mobile->lodged = NULL;
	new_mobile->subdue = NULL;

	new_mobile->mob->owner = NULL;

	if ( new_mobile->speaks == SKILL_HEALING )
		new_mobile->speaks = SKILL_SPEAK_WESTRON;

	return new_mobile;
}

void insert_string_variables (OBJ_DATA *new_obj, OBJ_DATA *proto, char *string)
{
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	char	buf2 [MAX_STRING_LENGTH] = {'\0'};
	char	original [MAX_STRING_LENGTH] = {'\0'};
	char	color [MAX_STRING_LENGTH] = {'\0'};
	char	*point = NULL;
	int		i = 0;
	int		j = 0;
	int		limit = 0;
	bool	modified = FALSE;

	*buf = '\0';
	*buf2 = '\0';

	if ( string == 0 )
		*color = '\0';
	else snprintf (color, MAX_STRING_LENGTH, "%s", string);

	SET_BIT (new_obj->obj_flags.extra_flags, ITEM_VARIABLE);

	if ( !*proto->full_description )
		return;

	snprintf (original, MAX_STRING_LENGTH, "%s", proto->short_description);

	if ( (point = strstr (original, "$color")) ) {
		if ( !*color ) {
			for ( i = 0, limit = 0; *standard_object_colors[i] != '\n'; i++, limit++ )
				;
			limit--;
			snprintf (color, MAX_STRING_LENGTH, "%s", standard_object_colors[number(0,limit)]);
		}
	}
	else if ( (point = strstr (original, "$drabcolor")) ) {
		if ( !*color ) {
			for ( i = 0, limit = 0; *drab_object_colors[i] != '\n'; i++, limit++ )
				;
			limit--;
			snprintf (color, MAX_STRING_LENGTH, "%s", drab_object_colors[number(0,limit)]);
		}
	}
	else if ( (point = strstr (original, "$finecolor")) && !*color ) {
		if ( !*color ) {
			for ( i = 0, limit = 0; *fine_object_colors[i] != '\n'; i++, limit++ )
				;
			limit--;
			snprintf (color, MAX_STRING_LENGTH, "%s", fine_object_colors[number(0,limit)]);
		}
	}

	if ( point ) {
		for ( i = 0; i <= strlen(original); i++ ) {
			if ( original[i] == *point ) {
				modified = TRUE;
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%s", color);
				j = i+1;
				while ( isalpha(original[j]) )
					j++;
				i = j;
			}
			snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%c", original[i]);
		}

		mem_free (new_obj->short_description);
		new_obj->short_description = add_hash(buf2);
	}

	*buf2 = '\0';
	snprintf (original, MAX_STRING_LENGTH, "%s", proto->description);
	point = strstr (original, "$color");
	if ( !point )
		point = strstr (original, "$drabcolor");
	if ( !point )
		point = strstr (original, "$finecolor");

	if ( point ) {
		for ( i = 0; i <= strlen(original); i++ ) {
			if ( original[i] == *point ) {
				modified = TRUE;
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%s", color);
				j = i+1;
				while ( isalpha(original[j]) )
					j++;
				i = j;
			}
			snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%c", original[i]);
		}
	
		mem_free (new_obj->description);
		new_obj->description = add_hash(buf2);
	}

	*buf2 = '\0';
	snprintf (original, MAX_STRING_LENGTH, "%s", proto->full_description);
	point = strstr (original, "$color");
	if ( !point )
		point = strstr (original, "$drabcolor");
	if ( !point )
		point = strstr (original, "$finecolor");

	if ( point ) {
		for ( i = 0; i <= strlen(original); i++ ) {
			if ( original[i] == *point ) {
				modified = TRUE;
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%s", color);
				j = i+1;
				while ( isalpha(original[j]) )
					j++;
				i = j;
			}
			snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%c", original[i]);
		}

		mem_free (new_obj->full_description);
		new_obj->full_description = str_dup (buf2);
	}

	*buf2 = '\0';
	snprintf (original, MAX_STRING_LENGTH, "%s", proto->name);
	point = strstr (original, "$color");
	if ( !point )
		point = strstr (original, "$drabcolor");
	if ( !point )
		point = strstr (original, "$finecolor");

	if ( point ) {
		for ( i = 0; i <= strlen(original); i++ ) {
			if ( original[i] == *point ) {
				modified = TRUE;
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%s", color);
				j = i+1;
				while ( isalpha(original[j]) )
					j++;
				i = j;
			}
			snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%c", original[i]);
		}

		mem_free (new_obj->name);
		new_obj->name = str_dup(buf2);
	}

	if ( (IS_SET (new_obj->obj_flags.extra_flags, ITEM_MASK) && new_obj->obj_flags.type_flag == ITEM_ARMOR) ||
	     (IS_SET (new_obj->obj_flags.extra_flags, ITEM_MASK) && new_obj->obj_flags.type_flag == ITEM_WORN) ) {
		*buf2 = '\0';
		snprintf (original, MAX_STRING_LENGTH, "%s", proto->desc_keys);
		point = strstr (original, "$color");
		if ( !point )
			point = strstr (original, "$drabcolor");
		if ( !point )
			point = strstr (original, "$finecolor");

		if ( point ) {
			for ( i = 0; i <= strlen(original); i++ ) {
				if ( original[i] == *point ) {
					modified = TRUE;
					snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%s", color);
					j = i+1;
					while ( isalpha(original[j]) )
						j++;
						i = j;
				}
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%c", original[i]);
			}
			mem_free (new_obj->desc_keys);
			new_obj->desc_keys = add_hash(buf2);
		}
	}

	if ( new_obj->var_color )
		mem_free (new_obj->var_color);

	if ( !modified )
		new_obj->var_color = add_hash("none");
	else
		new_obj->var_color = add_hash(color);
		
	return;
}

OBJ_DATA *load_object (int vnum)
{
	OBJ_DATA		*proto = NULL;
	OBJ_DATA		*new_obj = NULL;
	WRITING_DATA	*writing = NULL;
	int				i = 0;
	AFFECTED_TYPE	*af = NULL;
	AFFECTED_TYPE	*new_af = NULL;
	AFFECTED_TYPE	*last_af = NULL;

	if ( !(proto = vtoo (vnum)) )
		return NULL;

	new_obj = new_object ();

	memcpy (new_obj, proto, sizeof (OBJ_DATA));

	new_obj->deleted = 0;

	new_obj->xaffected = NULL;

	new_obj->next_content = 0;

	new_obj->var_color = (char *)NULL;

	for ( af = proto->xaffected; af; af = af->next ) {

		new_af = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);

		memcpy (new_af, af, sizeof (AFFECTED_TYPE));

		new_af->next = NULL;

		if ( !new_obj->xaffected )
			new_obj->xaffected = new_af;
		else
			last_af->next = new_af;

		last_af = new_af;
	}

	new_obj->next = object_list;
	object_list = new_obj;
	
	new_obj->count = 1;

	if(new_obj->clock && new_obj->morphto)
	    new_obj->morphTime = time(0) + new_obj->clock * 14*60;

	if ( GET_ITEM_TYPE(new_obj) == ITEM_BOOK ) {
		if ( !new_obj->writing && new_obj->o.od.value[0] > 0) {
			CREATE (new_obj->writing, WRITING_DATA, 1);
			for ( i = 1, writing = new_obj->writing; i <= new_obj->o.od.value[0]; i++ ) {
				writing->message = add_hash ("blank");
				writing->author = add_hash ("blank");
				writing->date = add_hash ("blank");
				writing->ink = add_hash ("blank");
				writing->language = 0;
				writing->script = 0;
				writing->skill = 0;
				writing->torn = FALSE;
				if ( i != new_obj->o.od.value[0] ) {
					CREATE (writing->next_page, WRITING_DATA, 1);
					writing = writing->next_page;
				}
			}
		}
	}

	if ( GET_ITEM_TYPE (new_obj) == ITEM_BOOK )
		new_obj->o.od.value[1] = unused_writing_id();

	if ( GET_ITEM_TYPE (new_obj) == ITEM_PARCHMENT )
		new_obj->o.od.value[0] = unused_writing_id();

	if ( IS_SET(new_obj->obj_flags.extra_flags, ITEM_VARIABLE) )
		insert_string_variables (new_obj, proto, 0);
	else new_obj->var_color = 0;

	if ( GET_ITEM_TYPE(new_obj) == ITEM_WEAPON )
		SET_BIT (new_obj->obj_flags.extra_flags, ITEM_NEWSKILLS);

	new_obj->contains = NULL;
	new_obj->lodged = NULL;
	new_obj->wounds = NULL;
	new_obj->equiped_by = NULL;
	new_obj->carried_by = NULL;
	new_obj->in_obj = NULL;

	if ( !new_obj->item_wear )
		new_obj->item_wear = 100;

	new_obj->coldload_id = get_next_coldload_id (2);

	return new_obj;
}

OBJ_DATA *load_colored_object (int vnum, char *color)
{
	OBJ_DATA		*proto = NULL;
	OBJ_DATA		*new_obj = NULL;
	WRITING_DATA	*writing = NULL;
	int				i = 0;
	AFFECTED_TYPE	*af = NULL;
	AFFECTED_TYPE	*new_af = NULL;
	AFFECTED_TYPE	*last_af = NULL;

	if ( !(proto = vtoo (vnum)) )
		return NULL;

	new_obj = new_object ();

	memcpy (new_obj, proto, sizeof (OBJ_DATA));

	new_obj->deleted = 0;

	new_obj->xaffected = NULL;

	new_obj->var_color = (char *)NULL;

	for ( af = proto->xaffected; af; af = af->next ) {

		new_af = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);

		memcpy (new_af, af, sizeof (AFFECTED_TYPE));

		new_af->next = NULL;

		if ( !new_obj->xaffected )
			new_obj->xaffected = new_af;
		else
			last_af->next = new_af;

		last_af = new_af;
	}

	new_obj->next = object_list;
	object_list = new_obj;
	
	new_obj->count = 1;

	if(new_obj->clock && new_obj->morphto)
	    new_obj->morphTime = time(0) + new_obj->clock * 14*60;

	if ( GET_ITEM_TYPE(new_obj) == ITEM_BOOK ) {
		if ( !new_obj->writing && new_obj->o.od.value[0] > 0) {
			CREATE (new_obj->writing, WRITING_DATA, 1);
			for ( i = 1, writing = new_obj->writing; i <= new_obj->o.od.value[0]; i++ ) {
				writing->message = add_hash ("blank");
				writing->author = add_hash ("blank");
				writing->date = add_hash ("blank");
				writing->ink = add_hash ("blank");
				writing->language = 0;
				writing->script = 0;
				writing->skill = 0;
				writing->torn = FALSE;
				if ( i != new_obj->o.od.value[0] ) {
					CREATE (writing->next_page, WRITING_DATA, 1);
					writing = writing->next_page;
				}
			}
		}
	}

	if ( IS_SET(new_obj->obj_flags.extra_flags, ITEM_VARIABLE) )
		insert_string_variables (new_obj, proto, color);
	else new_obj->var_color = 0;

	if ( GET_ITEM_TYPE(new_obj) == ITEM_WEAPON )
		SET_BIT (new_obj->obj_flags.extra_flags, ITEM_NEWSKILLS);

	return new_obj; 
}

OBJ_DATA *fread_object (int vnum, int zone, FILE *fp)
{
	OBJ_DATA			*obj = NULL;
	float				tmpf = 0;
	int					tmp = 0;
	char				chk[50];
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	XTRA_DESCR_DATA		*new_descr = NULL;
	XTRA_DESCR_DATA		*tmp_descr = NULL;
	AFFECTED_TYPE		*af = NULL;
	AFFECTED_TYPE		*taf = NULL;
	char				peak_char = {'\0'};

	obj = new_object ();

	clear_object (obj);

	obj->virtual			= vnum;
	obj->zone				= zone;

#if CHECK_DOUBLE_DEFS
	if ( vtoo (vnum) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "OBJ %d multiply defined!!", vnum);
		system_log(buf, TRUE);
	} else
#endif
	add_obj_to_hash (obj);

	obj->name		= fread_string(fp);
	obj->short_description	= fread_string(fp);
	obj->description	= fread_string(fp);
	obj->full_description	= fread_string(fp);

	if ( !strcmp (obj->full_description, "(null)") ) {
		snprintf (buf, MAX_STRING_LENGTH,  "NOTE:  Object %d with '(null)' full description fixed.",
				 obj->virtual);
		system_log(buf, TRUE);
		obj->full_description = null_string;
	}

	/* *** numeric data *** */

	fscanf(fp, " %d ", &tmp);
	obj->obj_flags.type_flag = tmp;
	fscanf(fp, " %d ", &tmp);
	obj->obj_flags.extra_flags = tmp;
	fscanf(fp, " %d ", &tmp);
	obj->obj_flags.wear_flags = tmp;
	fscanf(fp, " %d ", &tmp);
	obj->o.od.value[0] = tmp;
	fscanf(fp, " %d ", &tmp);
	obj->o.od.value[1] = tmp;
	fscanf(fp, " %d ", &tmp);
	obj->o.od.value[2] = tmp;
	fscanf(fp, " %d ", &tmp);
	obj->o.od.value[3] = tmp;

	fscanf(fp, " %d ", &tmp);		/* Weight */

	obj->obj_flags.weight = tmp;

	fscanf(fp, " %f\n", &tmpf);
	obj->silver = tmpf;					/* Changed to silver from cost */

	fscanf(fp, " %d ", &tmp);
	obj->o.od.value[4] = tmp;

	if ( obj->obj_flags.type_flag == ITEM_INK )
		obj->ink_color = fread_string (fp);

	else if ( GET_ITEM_TYPE(obj) == ITEM_TENT ||
			GET_ITEM_TYPE(obj) == ITEM_DWELLING ) {
			
			obj->indoor_desc = fread_string (fp);
			
			if ( obj->indoor_desc &&
				*obj->indoor_desc &&
				!str_cmp (obj->indoor_desc, "(null)") ) {
			
				mem_free (obj->indoor_desc);
				obj->indoor_desc = NULL;
			}
		}

	else if ( IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
			obj->obj_flags.type_flag == ITEM_WORN ) 
			
			obj->desc_keys = fread_string (fp);
		
	else if ( IS_SET (obj->obj_flags.extra_flags, ITEM_MASK) &&
			  obj->obj_flags.type_flag == ITEM_ARMOR ) 
			
			obj->desc_keys   = fread_string (fp);
       	
	else {
			fscanf (fp, " %d",
			&obj->o.od.value [5]);
		}
	
 
	fscanf (fp, " %d %d %d %d %d\n",
				&obj->activation,
				&obj->quality,
				&obj->econ_flags,
				&obj->size,
				&obj->count);

	fscanf (fp, "%f %d %d %d %d %d %d\n",
				&obj->coppers,
				&obj->clock,
				&obj->morphto,
				&obj->item_wear,
				&obj->material,
				&tmp,
				&tmp);
	

	if ( GET_ITEM_TYPE(obj) == ITEM_INK ) {
		obj->clock = 0;
		obj->morphto = 0;
	}

	/* *** extra descriptions *** */

	obj->ex_description = 0;
	obj->wdesc = 0;

	do {
		while ( (peak_char = getc (fp)) == ' ' || peak_char == '\t' ||
				peak_char == '\n')
		       	;

		ungetc (peak_char, fp);

		if ( peak_char != 'E' )
			break;

		fscanf (fp, " %s \n", chk);

		new_descr = get_perm (sizeof (struct extra_descr_data));

		new_descr->keyword = fread_string(fp);
		new_descr->description = fread_string(fp);

			/* Add descr's in same order as read so that they
			   can get written back in same order */

		new_descr->next = NULL;

		if ( !obj->ex_description )
			obj->ex_description = new_descr;
		else {
			tmp_descr = obj->ex_description;
			while ( tmp_descr->next )
				tmp_descr = tmp_descr->next;
			tmp_descr->next = new_descr;
		}
	} while (1);

	tmp = 0;

	do {
		while ( (peak_char = getc (fp)) == ' ' || peak_char == '\t' ||
				peak_char == '\n')
			;

		ungetc (peak_char, fp);

		if ( peak_char != 'A' )
			break;

		fscanf (fp, " %s \n", chk);

		af = get_perm (sizeof (AFFECTED_TYPE));

		af->type		= 0;
		af->a.spell.duration	= -1;
		af->a.spell.bitvector	= 0;
		af->a.spell.sn			= 0;
		af->next		= NULL;

		fscanf (fp, " %d %d\n", &af->a.spell.location, &af->a.spell.modifier);

		if ( af->a.spell.location || af->a.spell.modifier ) {

			tmp++;

			if ( !obj->xaffected )
				obj->xaffected = af;
			else {
				for ( taf = obj->xaffected; taf->next; taf = taf->next )
					;
				taf->next = af;
			}
		}

	} while (1);

/*** Lua triggers ***/
	do {
			while ( (peak_char = getc (fp)) == ' ' ||
					peak_char == '\t' ||
					peak_char == '\n')
				;
	
			ungetc (peak_char, fp);
	
			if ( peak_char != 'R' )
				break;
	
			larg_setup_obj_triggers(fp, obj);
	
		} while (1);
		
/***** end lua triggers ***/	
	if ( tmp > 20 ) 
		printf ("Object %d has %d affects\n", obj->virtual, tmp);

	obj->in_room = NOWHERE;
	obj->next_content = 0;
	obj->carried_by = 0;
	obj->equiped_by = 0;
	obj->in_obj = 0;
	obj->contains = 0;

	if ( obj->count == 0 )
		obj->count = 1;

	if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_SHIELD) )
		REMOVE_BIT (obj->obj_flags.wear_flags, ITEM_WEAR_SHIELD);

	return obj;
}

/** Mobs get to choose a height range (tall, average, short) and that has to be translated into actual height */
void make_height (CHAR_DATA *mob, int range)
{
	int		height = 0;
	float	adjust = 0;
		
	if ( mob->sex == SEX_MALE) {
			height = dice (db_race_table [mob->race].male_ht_dice,
						   db_race_table [mob->race].male_ht_sides);
			height += db_race_table [mob->race].male_ht_constant;
		
		if (range == 0){
			adjust = number(.85, 1.05);
			mob->height = (int)(height * adjust);
		}
		else if (range == 1){
			adjust = number(.95, 1.05);
			mob->height = (int)(height * adjust);
		}
		
		else { /* range == 2 */
			adjust = number(.95, 1.15);
			mob->height = (int)(height * adjust);
		}

		return;
	}

	else if ( mob->sex == SEX_FEMALE  || mob->sex == SEX_NEUTRAL){
		height = dice (db_race_table [mob->race].female_ht_dice,
					   db_race_table [mob->race].female_ht_sides);

		height += db_race_table [mob->race].female_ht_constant;

		if (range == 0){
			adjust = number(.85, 1.05);
			mob->height = (int)(height * adjust);
		}
		else if (range == 1){
			adjust = number(.95, 1.05);
			mob->height = (int)(height * adjust);
		}
		
		else { /* range == 2 */
			adjust = number(.95, 1.15);
			mob->height = (int)(height * adjust);
		}

	return;
	}
}

/** Frame is choosen by PC and random choice for mobs **/
void make_frame (CHAR_DATA *mob)
{
    int     frame = 0;
	
		
    if ( mob->sex == SEX_MALE ) {
            frame = dice (db_race_table [mob->race].male_fr_dice,
                           db_race_table [mob->race].male_fr_sides);
        frame += db_race_table [mob->race].male_fr_constant;

		mob->frame = frame;

        return;
    }

    else if ( mob->sex == SEX_FEMALE  || mob->sex == SEX_NEUTRAL)
        frame = dice (db_race_table [mob->race].female_fr_dice,
                       db_race_table [mob->race].female_fr_sides);

    frame += db_race_table [mob->race].female_fr_constant;

	mob->frame = frame;

    return;
}

CHAR_DATA *get_live_mob (int vnum, int zone, int reset_cmd)
{
	CHAR_DATA	*tch = NULL;

	for ( tch = character_list; tch; tch = tch->next ) {
		if ( tch->deleted )
			continue;
		if ( !IS_NPC (tch) || !tch->room || tch->mob->virtual != vnum )
			continue;
		if ( tch->mob->reset_cmd == reset_cmd && tch->mob->reset_zone == zone )
			return tch;
	}

	return NULL;
}

void initialize_new_spawnpoints (void)
{
	int		cmd_no = 0;
	int		i = 0;

	for ( i = 0; i <= 99; i++ ) {
		if ( !zone_table [i].cmd )
			continue;
		for ( cmd_no = 0; ; cmd_no++ ) {
			if ( zone_table[i].cmd[cmd_no].command == 'S' )
				break;
			if ( zone_table[i].cmd[cmd_no].command != 'M' )
				continue;	
		}
	}
	return;
}

#define ZCMD zone_table[zone].cmd[cmd_no]

void reset_zone (int zone)
{
	int			cmd_no = 0;
	int			count_vnum_in_room = 0;
	int			i = 0;
	int			current_room = -1;
	CHAR_DATA		*mob = NULL;
	CHAR_DATA		*tmob = NULL;
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*tobj = NULL;
	OBJ_DATA		*temp_obj = NULL;
	ROOM_DATA		*room = NULL;
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	AFFECTED_TYPE		*af = NULL;
	RESET_AFFECT		*ra = NULL;
	RESET_DATA			*reset = NULL;
	char				buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !zone_table [zone].cmd )
		return;

	for ( cmd_no = 0; ; cmd_no++ ) {

		if ( ZCMD.command == 'S' )
			break;

		if ( ZCMD.command == 'M' ) {			/* Mob to room */

			mob = NULL;

			if ( !ZCMD.enabled )
				continue;

			ZCMD.enabled = 0;

			if ( /*(port != PLAYER_PORT) &&*/ (mob = load_mobile (ZCMD.arg1)) ) {
				mob->mob->reset_zone = zone;
				mob->mob->reset_cmd = cmd_no;

				if ( !mob->height && !IS_SET (mob->flags, FLAG_VARIABLE) )
					make_height (mob, 1);

				if ( !mob->frame )
					make_frame (mob);

				mob->mob->spawnpoint = ZCMD.arg3;
				char_to_room (mob, ZCMD.arg3);

				if ( ZCMD.arg4 && (tmob = load_mobile (ZCMD.arg4)) ) {
					tmob->mount = mob;
					mob->mount = tmob;
					tmob->mob->spawnpoint = ZCMD.arg3;
					char_to_room (tmob, ZCMD.arg3);
				}	
			} else if ( !fCopyOver ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Unable to load mob virtual %d!", ZCMD.arg1);
				system_log(buf, TRUE);
			}
		}

		else if ( ZCMD.command == 'R' ) {		/* Defining room */
			current_room = ZCMD.arg1;
			continue;
		}

		else if ( ZCMD.command == 'A' ||		/* Affect on char */
				  ZCMD.command == 'r' ) {		/* Affect on room */

			if ( !ZCMD.arg1 ) {
				system_log("ZCMD is zero.", TRUE);
				continue;
			}

			if ( !mob )
				continue;

			ra = (RESET_AFFECT *) ZCMD.arg1;

			if ( get_affect (mob, ra->type) )
				continue;

            af = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);

			af->type              = ra->type;
			af->a.spell.duration  = ra->duration;
			af->a.spell.modifier  = ra->modifier;
			af->a.spell.location  = ra->location;
			af->a.spell.bitvector = ra->bitvector;
			af->a.spell.sn        = ra->sn;
			af->a.spell.t         = ra->t;
			af->next              = NULL;

			if ( ZCMD.command == 'r' ) {
				af->next = vtor (current_room)->affects;
				vtor (current_room)->affects = af;
			} else
				affect_to_char (mob, af);

			continue;
		}

		else if ( ZCMD.command == 'm' ) {

			if ( !mob )
				continue;

			if ( ZCMD.arg1 == RESET_REPLY ) {
				reset = (RESET_DATA *)alloc ((int)sizeof (RESET_DATA), 33);

				reset->type = RESET_REPLY;

				reset->command = str_dup ((char *) ZCMD.arg2);

				reset->when.month  = -1;
				reset->when.day    = -1;
				reset->when.hour   = -1;
				reset->when.minute = -1;
				reset->when.second = -1;

				reset_insert (mob, reset);
			}
		}

		else if ( ZCMD.command == 'C' ) {

			if ( !mob )
				continue;

			if ( !ZCMD.arg1 )
				continue;

			for ( craft = crafts;
                  craft && str_cmp (craft->subcraft_name, (char *) ZCMD.arg1);
                  craft = craft->next )
                ; /* cycling through crafts */

            if ( !craft ) {

				snprintf (buf, MAX_STRING_LENGTH,  "RESET: No such craft %s on mob %d, room %d", craft->subcraft_name, mob->mob->virtual, mob->in_room);

				system_log(buf, TRUE);
            }

		    for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ )
				if ( !get_affect (mob, i) )
					break;

			magic_add_affect (mob, i, -1, 0, 0, 0, 0);

			af = get_affect (mob, i);

			af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);

			af->a.craft->subcraft = craft;
		}

		else if ( ZCMD.command == 'O' ) {

			obj = NULL;

			count_vnum_in_room = 0;

			for ( tobj = vtor (ZCMD.arg3)->contents;
				  tobj;
				  tobj = tobj->next_content )
				if ( tobj->virtual == ZCMD.arg1 )
					count_vnum_in_room++;

			if ( count_vnum_in_room < ZCMD.arg2 &&
				 (obj = load_object (ZCMD.arg1)) )
				obj_to_room (obj, ZCMD.arg3);
		}

		else if ( ZCMD.command == 'P' ) {
			if ( !obj )
				continue;

			if ( (tobj = load_object (ZCMD.arg1)) )
				obj_to_obj (tobj, obj);
		}

		else if ( ZCMD.command == 'G' ) {

			obj = NULL;

			if ( !mob )
				continue;

			if ( (obj = load_object (ZCMD.arg1)) ) {
				obj_to_char (obj, mob);
			}
		}

		else if ( ZCMD.command == 'E' ) {
			
			obj = NULL;

			if ( !mob )
				continue;

			if ( (obj = load_object (ZCMD.arg1)) ) {

				if ( IS_WEARABLE (obj) )
					obj->size = get_size (mob);

				equip_char (mob, obj, ZCMD.arg3);

				cmd_no++;
				if ( ZCMD.command == 's' ) {
					temp_obj = load_object(ZCMD.arg1);
					obj_to_obj (temp_obj, obj);
					if ( ZCMD.arg2 > 1 )
						for ( i = 1; i < ZCMD.arg2; i++ ){
							temp_obj = load_object(ZCMD.arg1);
							obj_to_obj (temp_obj, obj);
						}
				}
				else{
					cmd_no--;
				}
			}
		}

		else if ( ZCMD.command == 'a' ) {
			
			obj = NULL;

			if ( !tmob )
				continue;

			if ( (obj = load_object (ZCMD.arg1)) ) {

				if ( IS_WEARABLE (obj) )
					obj->size = get_size (tmob);

				equip_char (tmob, obj, ZCMD.arg3);

				cmd_no++;
				if ( ZCMD.command == 's' ) {
					temp_obj = load_object(ZCMD.arg1);
					obj_to_obj (temp_obj, obj);
					if ( ZCMD.arg2 > 1 ){
						for ( i = 1; i < ZCMD.arg2; i++ ){
							temp_obj = load_object(ZCMD.arg1);
							obj_to_obj (temp_obj, obj);
						}
					}
				}
				else{
					cmd_no--;
				}
			}
		}

		else if ( ZCMD.command == 'D' ) {

			room = vtor (ZCMD.arg1);

			if ( !room || !room->dir_option [ZCMD.arg2] )
				continue;

			switch (ZCMD.arg3) {
				case 0:  
					 REMOVE_BIT (room->dir_option [ZCMD.arg2]->exit_info,
								 PASSAGE_LOCKED | PASSAGE_CLOSED);
					 break;
				case 1:  
					SET_BIT (room->dir_option [ZCMD.arg2]->exit_info,
								 PASSAGE_CLOSED);
					 REMOVE_BIT	(room->dir_option [ZCMD.arg2]->exit_info,
								 PASSAGE_LOCKED);
					 break;
				case 2:
					 SET_BIT(room->dir_option [ZCMD.arg2]->exit_info,
							 PASSAGE_LOCKED | PASSAGE_CLOSED);
					 break;
				default:
					 break;
			}
		}
	}  /* for */

	zone_table [zone].age = 0;
	
	return;
}

#undef ZCMD

void list_validate (char *name)
{
	CHAR_DATA	*ch = NULL;
	OBJ_DATA	*obj = NULL;
	int			cycle_count = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	snprintf (buf, MAX_STRING_LENGTH,  "List validate:  %s entered.\n", name);
	system_log (buf, FALSE);

	for ( ch = character_list, cycle_count = 0; ch; ch = ch->next ) {
		if ( cycle_count++ > 10000 ) {
			system_log ("Character list cycle failed.", TRUE);
			((int *) 0) [-1] = 0;
		}
	}

	for ( obj = object_list, cycle_count = 0; obj; obj = obj->next ) {
		if ( cycle_count++ > 10000 ) {
			system_log ("Object list cycle failed.", TRUE);
			((int *) 0) [-1] = 0;
		}
	}

	snprintf (buf, MAX_STRING_LENGTH,  "List validate:  %s completed.\n", name);
	system_log (buf, FALSE);
}

void cleanup_the_dead (int mode)
{
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*next_obj = NULL;
	OBJ_DATA	*prev_obj = NULL;
	CHAR_DATA	*ch = NULL;
	CHAR_DATA	*next_ch = NULL;
	CHAR_DATA	*prev_ch = NULL;

	if ( mode == 1 || mode == 0 ) {
		for ( ch = character_list; ch; ch = next_ch ) {
			next_ch = ch->next;

			if ( !ch->deleted ) {
				prev_ch = ch;
				continue;
			}

			if ( ch == character_list )
				character_list = next_ch;
			else
				prev_ch->next = next_ch;	

			if ( !IS_NPC (ch) ) {
				unload_pc (ch);
				continue;
			}

			free_char (ch);
			ch = NULL;
		}
	}

	if ( mode == 2 || mode == 0 ) {
		for ( obj = object_list; obj; obj = next_obj ) {
			next_obj = obj->next;

			if ( !obj->deleted ) {
				prev_obj = obj;
				continue;
			}

			if ( obj == object_list )
				object_list = next_obj;
			else
				prev_obj->next = next_obj;

			free_obj (obj);
			obj = NULL;
		}
	}
	return;
}

#define ZCMD zone_table[zone].cmd[cmd_no]

void refresh_zone (void)
{
	int			cmd_no = 0;
	int			i = 0;
	int			count_vnum_in_room = 0;
	static int		zone = 0;
	CHAR_DATA		*mob = NULL;
	CHAR_DATA		*tmob = NULL;
	ROOM_DATA		*room = NULL;
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*tobj = NULL;
	OBJ_DATA		*temp_obj = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !zone_table [zone].cmd )
		return;

	if ( IS_FROZEN (zone) )
		return;

	snprintf (buf, MAX_STRING_LENGTH,  "Refreshing zone %d and loading any unloaded psaves...", zone);
	system_log (buf, FALSE);

	for ( room = full_room_list; room; room = room->lnext ) {
		if ( room->zone != zone )
			continue;
		if ( !room->psave_loaded )
			load_save_room (room);
	}

	for ( cmd_no = 0; ; cmd_no++ ) {

		if ( ZCMD.command == 'S' )
			break;

		if ( ZCMD.command == 'D' )
			continue;

		if ( ZCMD.command == 'M' ) {

			mob = NULL;

			if ( !ZCMD.enabled )
				continue;

			ZCMD.enabled = 0;

			if ( port == PLAYER_PORT ) {
				mysql_safe_query ("DELETE FROM mob_resets WHERE zone = %d AND cmd_no = %d", zone, cmd_no);
			}

			if ( (mob = load_mobile (ZCMD.arg1)) ) {
				mob->mob->reset_zone = zone;
				mob->mob->reset_cmd = cmd_no;
				char_to_room (mob, ZCMD.arg3);
			}

/*			act ("$n has arrived.", TRUE, mob, 0, 0, TO_ROOM | TO_ACT_FORMAT); */

			if ( ZCMD.arg4 && (tmob = load_mobile (ZCMD.arg4)) ) {
				tmob->mount = mob;
				mob->mount = tmob;
				tmob->mob->spawnpoint = ZCMD.arg3;
				char_to_room (tmob, ZCMD.arg3);
			} 

			mysql_safe_query ("DELETE FROM mob_resets WHERE zone = %d AND cmd_no = %d", zone, cmd_no);
		}

		else if ( ZCMD.command == 'O' ) {

			obj = NULL;

			count_vnum_in_room = 0;

			for ( tobj = vtor (ZCMD.arg3)->contents;
				  tobj;
				  tobj = tobj->next_content )
				if ( tobj->virtual == ZCMD.arg1 )
					count_vnum_in_room++;

			if ( count_vnum_in_room < ZCMD.arg2 &&
				 (obj = load_object (ZCMD.arg1)) )
				obj_to_room (obj, ZCMD.arg3);
		}

		else if ( ZCMD.command == 'P' ) {
			if ( !obj )
				continue;

			if ( (tobj = load_object (ZCMD.arg1)) )
				obj_to_obj (tobj, obj);
		}

		else if ( ZCMD.command == 'G' ) {

			obj = NULL;

			if ( !mob )
				continue;

			if ( (obj = load_object (ZCMD.arg1)) ) {
				obj_to_char (obj, mob);
			}
		}

		else if ( ZCMD.command == 'E' ) {
			
			obj = NULL;

			if ( !mob )
				continue;

			if ( (obj = load_object (ZCMD.arg1)) )
				equip_char (mob, obj, ZCMD.arg3);
		}

		else if ( ZCMD.command == 'a' ) {
			
			obj = NULL;

			if ( !tmob )
				continue;

			if ( (obj = load_object (ZCMD.arg1)) ) {

				if ( IS_WEARABLE (obj) )
					obj->size = get_size (tmob);

				equip_char (tmob, obj, ZCMD.arg3);

				cmd_no++;
				if ( ZCMD.command == 's' ) {
					temp_obj = load_object(ZCMD.arg1);
					obj_to_obj (temp_obj, obj);
					tobj = load_object(ZCMD.arg1);
					if ( ZCMD.arg2 > 1 )
						for ( i = 1; i < ZCMD.arg2; i++ ){
							temp_obj = load_object(ZCMD.arg1);
							obj_to_obj (temp_obj, obj);
						}
				}
				else cmd_no--;
			}
		}

	}  /* for */

	if ( zone+1 >= MAX_ZONE )
		zone = 0;
	else
		zone++;
		
	return;
}
#undef ZCMD
