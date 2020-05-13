/** 
*	\file olc.c
*	On-Line Creation module
*
*	This module allows you to create, and store objects, rooms, mobiles and some
*	progs through the telnet connection. Note: Many of these functions will be
*	re-written for a web-based building module.
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
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <mysql/mysql.h>

#include "structs.h"
#include "protos.h"
#include "decl.h"
#include "utils.h"


#define ROOM_MAX		100000
#define ZONE_SIZE		1000

#define s(a) send_to_char (a "\n", ch);

extern struct constant_data constant_info [];

void do_wearloc (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*obj;
	char		buf [MAX_STRING_LENGTH];
	char		loc [MAX_STRING_LENGTH];
	int		wear_loc, zone = 0, count = 0;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Which wear location did you wish to search for?\n", ch);
		return;
	}
	else snprintf (loc, MAX_STRING_LENGTH, "%s", buf);

	if ( (wear_loc = index_lookup(wear_bits, loc)) == -1 ) {
		send_to_char ("That isn't a valid wear location.\n", ch);
		return;
	}

	*buf = '\0';

	argument = one_argument (argument, buf);

	if ( *buf && isdigit(*buf) ) 
		zone = strtol(buf, NULL, 10);
	else zone = -1;

	*b_buf = '\0';

        snprintf (b_buf, MAX_STRING_LENGTH, "Vnum       Name(s)           Short Desc"
                                          "               Clones\tLocation\n\n");

	for ( obj = full_object_list; obj; obj = obj->lnext ) {
		if ( !obj )
			continue;
		if ( zone != -1 && obj->zone != zone )
			continue;
		if ( !IS_SET (obj->obj_flags.wear_flags, 1 << wear_loc) )
			continue;
		count++;
		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%.5d %-16.16s %-28.28s %.3d\t(R) %05d\n",
		obj->virtual, obj->name, obj->short_description, 999, obj->in_room);
	}

	page_string (ch->desc, b_buf);
	
}

/* some extra arg functions....probably should be moved */

void make_quiet(CHAR_DATA *ch)
{
	SET_BIT(ch->act, PLR_QUIET);
}

void save_world (CHAR_DATA *ch)
{
	int i;
	char	buf [MAX_STRING_LENGTH];

	snprintf (buf, MAX_STRING_LENGTH,  "%s is saving the entire worldfile ... one moment.\n", GET_NAME(ch));
	send_to_gods (buf);
	
	for (i = 0; i < MAX_ZONE; i++)
		save_rooms (ch, i);
}

void do_zmode(CHAR_DATA *ch, char *argument, int cmd)
{
	int mode, zone;
	char buf[80];

	for (; isspace(*argument); argument++);
	if(!*argument) {
		send_to_char("You must supply a mode.\n",ch);
		return;
	}
	if(!isdigit(*argument)) {
		send_to_char("Argument must be a number.\n",ch);
		return;
	}
	zone = ch->room->zone;
	mode = strtol(argument, NULL, 10);

	zone_table[zone].reset_mode = mode;

	snprintf (buf, MAX_STRING_LENGTH, "Zone %d reset mode changed to %d.\n",zone,mode);
	send_to_char(buf,ch);
}

void do_zlife (CHAR_DATA *ch, char *argument, int cmd)
{
	int life, zone;
	char buf[80];

	for (; isspace(*argument); argument++);
	if(!*argument) {
		send_to_char("You must supply a lifespan.\n",ch);
		return;
	}
	if(!isdigit(*argument)) {
		send_to_char("Argument must be a number.\n",ch);
		return;
	}

	zone = vtor (ch->in_room)->zone;

	life = strtol(argument, NULL, 10);

	zone_table [zone].lifespan = life;

	snprintf (buf, MAX_STRING_LENGTH,  "Zone %d lifespan changed to %d minutes.\n", zone, life);
	send_to_char (buf, ch);
}

void do_freeze(CHAR_DATA *ch, char *argument, int cmd)
{
	int num;
	char buf[80];

	for (; isspace(*argument); argument++);

	if (!isdigit(*argument))
	{
		send_to_char("That is not a valid zone number.\n", ch);
		return;
	}

	num = strtol(argument, NULL, 10);
	
	if ( num == 101 ) {
		for ( num = 0; num < MAX_ZONE; num++ )
			SET_BIT (zone_table [num].flags, Z_FROZEN);
		return;
	}

	if(num<0 || num>99) {
		send_to_char("The zone number must be between 0 and 99.\n",ch);
		return;
	}
	if(IS_SET(zone_table[num].flags,Z_FROZEN)) {
		snprintf (buf, MAX_STRING_LENGTH, "Zone %d is already frozen.\n",num);
		send_to_char(buf,ch);
		return;
	} else {
		SET_BIT(zone_table[num].flags,Z_FROZEN);
		snprintf (buf, MAX_STRING_LENGTH, "Zone %d is now frozen, type 'thaw %d' to re-enable mobile activity.\n",num,num);
		send_to_char(buf,ch);
	}
}

void do_thaw(CHAR_DATA *ch, char *argument, int cmd)
{
	int			num;
	int			unfrozen = 0;
	char		buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	num = strtol(buf, NULL, 10);
	
	if ( !str_cmp (buf, "all") ) {
		for ( num = 0; num < MAX_ZONE; num++ ) {

			if ( IS_SET (zone_table [num].flags, Z_FROZEN) )
				unfrozen++;

			REMOVE_BIT (zone_table [num].flags, Z_FROZEN);
		}

		snprintf (buf, MAX_STRING_LENGTH,  "%d zone(s) unfrozen.\n", unfrozen);
		send_to_char (buf, ch);
		return;
	}

	if ( *buf != '0' && !num ) {
		send_to_char ("Expected 0..99 or 'all'\n", ch);
		return;
	}

	if ( num < 0 || num > MAX_ZONE ) {
		send_to_char ("The zone number must be between 0 and MAX_ZONE.\n", ch);
		return;
	}

	if ( !IS_SET (zone_table [num].flags, Z_FROZEN) )
		send_to_char ("That zone isn't frozen.\n", ch);

	REMOVE_BIT (zone_table [num].flags, Z_FROZEN);
}

void fwrite_mobile (CHAR_DATA *tmob, FILE *fp)
{
	int			k, i;
	TRIGGER_DATA		*trig = NULL;


	fprintf (fp, "#%d\n", tmob->mob->virtual);
	fprintf (fp, "%s~\n%s~\n", tmob->name, tmob->short_descr);
	fprintf (fp, "%s~\n", tmob->long_descr);

	fprintf(fp,"%s~\n%lu %ld %d %d %d 5d1+%d %dd%d+%d "
			   "%d %d %d %d %d %d %d %d\n",
		tmob->description,
		tmob->act,
		tmob->affected_by, 
		tmob->offense,
		tmob->race,				/* Was defense */
		tmob->armor,
		tmob->max_hit,
		tmob->mob->damnodice,
		tmob->mob->damsizedice,
		tmob->mob->damroll,
		(int)tmob->time.birth,		/* Was clan_2 */
		tmob->position,
		tmob->default_pos,
		tmob->sex,
		tmob->mob->merch_seven,		/* Was clan_1 */
		tmob->deity,
		tmob->mob->vehicle_type,
		(int)tmob->hmflags);

	fprintf (fp, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
			tmob->mob->skinned_vnum,
			tmob->circle,
			tmob->cell_1,
			tmob->mob->carcass_vnum,
			tmob->cell_2,		/* Formerly tmob->defense_bonus */
			tmob->ppoints,
			tmob->natural_delay,
			tmob->mob->helm_room,
			tmob->body_type,
			tmob->poison_type,
			tmob->nat_attack_type,
			tmob->mob->access_flags,
			tmob->height, 
			tmob->frame,
			tmob->mob->noaccess_flags,
			tmob->cell_3);

		fprintf (fp, "%d %d %d %d %d %d %d %d\n",
			tmob->str,
			tmob->intel,
			tmob->wil,
			tmob->aur,
			tmob->dex,
			tmob->con,
			tmob->speaks,
			tmob->agi);
		
	if ( IS_SET (tmob->flags, FLAG_KEEPER) && !tmob->shop )
		REMOVE_BIT (tmob->flags, FLAG_KEEPER);

	fprintf (fp, "%d %d\n", tmob->flags, tmob->mob->currency_type);

	if ( IS_SET (tmob->flags, FLAG_KEEPER) ) {

		fprintf (fp, "%d %d %f %f %f %f %d\n",
			tmob->shop->shop_vnum,
			tmob->shop->store_vnum,
			tmob->shop->markup,
			tmob->shop->discount,
			tmob->shop->econ_markup1,
			tmob->shop->econ_discount1,
			tmob->shop->econ_flags1);

		if (tmob->mob->merch_seven > 0) {
                        fprintf (fp, "%f %f %d %f %f %d %f %f %d %f %f %d %f %f %d %f %f %d %d\n",
                                tmob->shop->econ_markup2,
                                tmob->shop->econ_discount2,
                                tmob->shop->econ_flags2,
                                tmob->shop->econ_markup3,
                                tmob->shop->econ_discount3,
                                tmob->shop->econ_flags3,
                                tmob->shop->econ_markup4,
                                tmob->shop->econ_discount4,
                                tmob->shop->econ_flags4,
                                tmob->shop->econ_markup5,
                                tmob->shop->econ_discount5,
                                tmob->shop->econ_flags5,
                                tmob->shop->econ_markup6,
                                tmob->shop->econ_discount6,
                                tmob->shop->econ_flags6,
                                tmob->shop->econ_markup7,
                                tmob->shop->econ_discount7,
                                tmob->shop->econ_flags7,
                                tmob->shop->nobuy_flags);
                } 
                else {
			fprintf (fp, "%f %f %d %f %f %d %d\n",
				tmob->shop->econ_markup2,
				tmob->shop->econ_discount2,
				tmob->shop->econ_flags2,
				tmob->shop->econ_markup3,
				tmob->shop->econ_discount3,	
				tmob->shop->econ_flags3,
				tmob->shop->nobuy_flags);
		}

		for ( i = 0; i <= MAX_DELIVERIES; i++ ) {
			if ( !tmob->shop->delivery [i] )
				break;
			fprintf (fp, "%d ", tmob->shop->delivery[i]);
		}
		fprintf (fp, "-1\n");

		fprintf (fp, "%d %d %d %d %d %d %d %d %d %d\n",
			tmob->shop->trades_in [0],
			tmob->shop->trades_in [1],
			tmob->shop->trades_in [2],
			tmob->shop->trades_in [3],
			tmob->shop->trades_in [4],
			tmob->shop->trades_in [5],
			tmob->shop->trades_in [6],
			tmob->shop->trades_in [7],
			tmob->shop->trades_in [8],
			tmob->shop->trades_in [9]);
	}
	
	for ( k = 0; k < (MAX_SKILLS/10); k++ ){
		fprintf (fp, "%d %d %d %d %d %d %d %d %d %d\n",
				tmob->skills [k * 10],
				tmob->skills [k * 10 + 1],
				tmob->skills [k * 10 + 2],
				tmob->skills [k * 10 + 3],
				tmob->skills [k * 10 + 4],
				tmob->skills [k * 10 + 5],
				tmob->skills [k * 10 + 6],
				tmob->skills [k * 10 + 7],
				tmob->skills [k * 10 + 8],
				tmob->skills [k * 10 + 9]);
	}

	fprintf (fp, "%s~\n", tmob->clans);
	
	for ( trig = tmob->triggers; trig; trig = trig->next ){
		fprintf (fp, "R\n%d\n%s:%s~\n",
						trig->type,
						trig->script,
						trig->func);
	}
	
	return;
}
    
void fwrite_object (OBJ_DATA *tobj, FILE *fp)
{
	AFFECTED_TYPE		*af = NULL;
	TRIGGER_DATA		*trig = NULL;
	
	fprintf (fp, "#%d\n", tobj->virtual);
	fprintf (fp, "%s~\n%s~\n%s~\n%s~\n%d %d %ld\n%d %d %d %d\n%d %f\n%d ",
			tobj->name,
			tobj->short_description,
        	tobj->description,
			tobj->full_description,
        	tobj->obj_flags.type_flag, 
			tobj->obj_flags.extra_flags & ~ITEM_LOADS,
        	tobj->obj_flags.wear_flags, 
			tobj->o.od.value [0],
        	tobj->o.od.value [1],
			tobj->o.od.value [2],
        	tobj->o.od.value [3],
			tobj->obj_flags.weight,
        	tobj->silver,
			tobj->o.od.value [4]);
		
	if ( tobj->obj_flags.type_flag == ITEM_INK )
		fprintf (fp, "\n%s~\n", tobj->ink_color);

	if ( tobj->obj_flags.type_flag == ITEM_TENT || tobj->obj_flags.type_flag == ITEM_DWELLING )
		fprintf (fp, "\n%s~\n", tobj->indoor_desc);

	else if ( tobj->obj_flags.type_flag == ITEM_WORN &&
	     IS_SET (tobj->obj_flags.extra_flags, ITEM_MASK) ) {
		if ( !tobj->desc_keys )
			fprintf (fp, "\n~\n");
		else
			fprintf (fp, "\n%s~\n",tobj->desc_keys);
	}

	else if ( tobj->obj_flags.type_flag == ITEM_ARMOR &&
			  IS_SET (tobj->obj_flags.extra_flags, ITEM_MASK) ) {
		if ( !tobj->desc_keys )
			fprintf (fp, "\n~\n");
		else
			fprintf (fp, "\n%s~\n",tobj->desc_keys);
	}

	else {
		if ((tobj->obj_flags.type_flag == ITEM_WORN) ||
		    (tobj->obj_flags.type_flag == ITEM_ARMOR)) 
	        tobj->o.od.value [5] = 0;
			
		fprintf (fp, "%d ", tobj->o.od.value [5]);
	}
				
	fprintf (fp, "%d %d %d %d %d\n",
			tobj->activation,
			tobj->quality,
			tobj->econ_flags,
			tobj->size,
			tobj->count);

	fprintf (fp, "%f %d %d %d %d 0 0\n",
			tobj->coppers, 
			tobj->clock, 
			tobj->morphto, 
			tobj->item_wear, 
			tobj->material);
	

	if ( tobj->wdesc )
       	fprintf (fp, "W\n%d\n%s~\n",
					 tobj->wdesc->language,
					 tobj->wdesc->description);
            
	for ( af = tobj->xaffected; af; af = af->next )
		fprintf (fp, "A\n%d %d\n", af->a.spell.location, af->a.spell.modifier);
	
	for ( trig = tobj->triggers; trig; trig = trig->next )
		fprintf (fp, "R\n%d\n%s:%s~\n",
						trig->type,
						trig->script,
						trig->func);
		
}

void fwrite_room (ROOM_DATA *troom, FILE *fp)
{
	XTRA_DESCR_DATA	*exptr;
	struct room_prog	*rp;
	int					j;
	TRIGGER_DATA	*nrp = NULL;
		
	if ( !troom->description )
		troom->description = add_hash ("No Description Set\n");

	fprintf (fp, "#%d\n%s~\n%s~\n",
				troom->virtual, troom->name, troom->description);
	fprintf (fp, "%d %d %d\n",
				troom->zone, troom->room_flags, troom->sector_type);

	fprintf (fp, "%d\n", troom->deity);

	if ( troom->extra ) {

		fprintf (fp, "A\n");

		for ( j = 0; j < WR_DESCRIPTIONS; j++ )
			fprintf (fp, "%s~\n", troom->extra->weather_desc [j] ? 
				troom->extra->weather_desc [j] : "");

		for ( j = 0; j < 6; j++ )
			fprintf (fp, "%s~\n", troom->extra->alas [j] ?
				troom->extra->alas [j] : "");
	}

	for (j = 0; j < 6; j++) {

		if ( !troom->dir_option [j] )
			continue;

		if ( IS_SET (troom->dir_option[j]->exit_info, PASSAGE_SECRET) )
			if ( IS_SET (troom->dir_option[j]->exit_info, PASSAGE_TRAP) )
				fprintf (fp, "B%d\n", j);
			else
				fprintf (fp, "H%d\n", j);

		else if ( IS_SET (troom->dir_option[j]->exit_info, PASSAGE_TRAP))
			fprintf (fp, "T%d\n", j);
		else
			fprintf (fp, "D%d\n", j);
                       
		if ( troom->dir_option [j]->general_description )
			fprintf(fp, "%s~\n", troom->dir_option[j]->general_description);
		else
			fprintf(fp, "~\n");

		if ( troom->dir_option [j]->keyword )
			fprintf(fp, "%s~\n", troom->dir_option [j]->keyword);
		else
			fprintf(fp, "~\n");

		if ( IS_SET (troom->dir_option [j]->exit_info, PASSAGE_PICKPROOF) )
			fprintf (fp, "2");
		else if ( IS_SET (troom->dir_option [j]->exit_info, PASSAGE_LOCKED) )
			fprintf (fp, "1");
		else if ( IS_SET (troom->dir_option [j]->exit_info, PASSAGE_ISDOOR) )
			fprintf (fp, "1");
		else
			fprintf (fp, "0");

		fprintf(fp, " %d ", troom->dir_option [j]->key);

		fprintf (fp, " %d ", troom->dir_option [j]->pick_penalty);

		if ( troom->dir_option [j]->to_room == NOWHERE )
			fprintf (fp, "-1\n");
		else
			fprintf (fp, "%d\n", troom->dir_option [j]->to_room);
	}

	for ( j = 0; j < 6; j++ ) {
		if ( troom->secrets [j] ) {
			fprintf (fp, "Q%d\n%d\n", j, troom->secrets [j]->diff);
			fprintf (fp, "%s~\n", troom->secrets [j]->stext);
		}
	}

	for ( exptr = troom->ex_description; exptr; exptr = exptr->next )
		if ( exptr->description )
			fprintf (fp, "E\n%s~\n%s~\n", exptr->keyword, exptr->description);

	if ( troom->wdesc )
		fprintf (fp, "W\n%d\n%s~\n",
					troom->wdesc->language, troom->wdesc->description);
                
	if ( troom->prg ) {
		for ( rp = troom->prg; rp; rp = rp->next ) {
			fprintf (fp, "P\n");
			fprintf (fp, "%s~\n", rp->command);
			fprintf (fp, "%s~\n", rp->keys);
			fprintf (fp, "%s~\n", rp->prog);
		}
	} 

	if ( troom->triggers ) {
		for ( nrp = troom->triggers; nrp; nrp = nrp->next ) {
			fprintf (fp, "R\n");
			fprintf (fp, "%d\n", nrp->type);
			fprintf (fp, "%s", nrp->script);
			
			if (nrp->func){
				fprintf (fp, ":%s~\n", nrp->func);
			}
			else {
				fprintf (fp, "~\n");
			}
		}
	} 
	
	fprintf(fp, "S\n");
}

void save_affect_reset (FILE *fp, CHAR_DATA *tmp_mob, AFFECTED_TYPE *af)
{
	CHAR_DATA		*proto;

	if ( af->type == MAGIC_DRAGGER   || af->type == MAGIC_WATCH1 ||
		 af->type == MAGIC_WATCH2    || af->type == MAGIC_WATCH3 ||
         af->type == MAGIC_NOTIFY    || af->type == MAGIC_GUARD  ||
         af->type == MAGIC_SIT_TABLE || af->type == AFFECT_SHADOW ||
		 af->type == MAGIC_CLAN_NOTIFY )
		return;

	if ( af->type == MAGIC_ROOM_FIGHT_NOISE )
		return;

	if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST ) {
		fprintf (fp, "C '%s'\n", af->a.craft->subcraft->subcraft_name);
		return;
	}

	proto = vtom (tmp_mob->mob->virtual);

	if ( af->type == MAGIC_AFFECT_INFRAVISION &&
		 IS_SET (tmp_mob->affected_by, AFF_INFRAVIS) )
		return;

	if ( af->type == MAGIC_AFFECT_INVISIBILITY &&
		 IS_SET (tmp_mob->affected_by, AFF_INVISIBLE) )
		return;

	if ( af->type == MAGIC_HIDDEN &&
		 IS_SET (tmp_mob->affected_by, AFF_HIDE) )
		return;

	fprintf (fp, "A %d %d %d %d %d %d %d 0 0 0 0 0 0\n",
				 af->type,
				 af->a.spell.duration,
				 af->a.spell.modifier,
				 af->a.spell.location,
				 af->a.spell.bitvector,
				 af->a.spell.sn,
				 af->a.spell.t);
}

void fwrite_resets (ROOM_DATA *troom, FILE *fp)
{
	CHAR_DATA		*tmp_mob;
	OBJ_DATA		*to;
	OBJ_DATA		*obj;
	AFFECTED_TYPE	*af;
	RESET_DATA		*reset;
	int				j;
	int				w;

		/* Write room header information if we need to write room affects */

	if ( troom->affects )
		fprintf (fp, "R %d 0 0 0 0 0 0 0 0 0\n", troom->virtual);
		
	for ( af = troom->affects; af; af = af->next ) {
		if ( af->type == MAGIC_ROOM_FIGHT_NOISE )
			continue;
		fprintf (fp, "r %d %d %d %d %d %d %d 0 0 0 0 0 0\n",
				 af->type,
				 af->a.spell.duration,
				 af->a.spell.modifier,
				 af->a.spell.location,
				 af->a.spell.bitvector,
				 af->a.spell.sn,
				 af->a.spell.t);
	}

	for (j = 0; j < 6; j++) {

		if ( !troom->dir_option [j] )
			continue;
                        
		if ( IS_SET (troom->dir_option [j]->exit_info, PASSAGE_PICKPROOF) )
			fprintf (fp, "D 0 %d %d 2\n", troom->virtual, j);

		else if ( IS_SET (troom->dir_option [j]->exit_info, PASSAGE_LOCKED) )
			fprintf (fp, "D 0 %d %d 2\n", troom->virtual, j);

		else if ( IS_SET (troom->dir_option [j]->exit_info, PASSAGE_ISDOOR) )
			fprintf (fp, "D 0 %d %d 1\n", troom->virtual, j);
	}

	if ( troom->people ) {
		for ( tmp_mob = troom->people;
			  tmp_mob;
			  tmp_mob = tmp_mob->next_in_room ) {

			if ( !IS_NPC (tmp_mob) )
				continue;

			if ( IS_SET (tmp_mob->act, ACT_STAYPUT) || IS_SET (tmp_mob->act, ACT_MOUNT) )
				continue;

			fprintf (fp, "M 0 %d 0 %d %d\n", tmp_mob->mob->virtual, troom->virtual, tmp_mob->mount ? tmp_mob->mount->mob->virtual : 0);

			if ( tmp_mob->right_hand ) {
				to = tmp_mob->right_hand;
				fprintf (fp, "G 1 %d 0\n", to->virtual);
				for ( to = to->contains; to; to = to->next_content)
					fprintf (fp, "P 1 %d 0 %d\n", to->virtual,
							to->in_obj->virtual);
			}

			if ( tmp_mob->left_hand ) {
				to = tmp_mob->left_hand;
				fprintf (fp, "G 1 %d 0\n", to->virtual);
				for ( to = to->contains; to; to = to->next_content)
					fprintf (fp, "P 1 %d 0 %d\n", to->virtual,
							to->in_obj->virtual);
			}

			for ( w = 0; w < MAX_WEAR; w++ ) {
				if ( !get_equip (tmp_mob, w) )
					continue;

				obj = get_equip(tmp_mob, w);
				fprintf (fp, "E 1 %d 0 %d\n", obj->virtual, w);
				if ( obj->contains && (GET_ITEM_TYPE(obj) == ITEM_CONTAINER || GET_ITEM_TYPE(obj) == ITEM_KEYRING || GET_ITEM_TYPE(obj) == ITEM_SHEATH || GET_ITEM_TYPE(obj) == ITEM_QUIVER) )				
					fprintf (fp, "s 1 %d %d\n", obj->contains->virtual, obj->contains->count);
			}

			for ( af = tmp_mob->hour_affects; af; af = af->next )
				save_affect_reset (fp, tmp_mob, af);

			for ( reset = tmp_mob->mob->resets; reset; reset = reset->next )
				if ( reset->type == RESET_REPLY )
					fprintf (fp, "m %d %s\n", reset->type, reset->command);
				else
					system_log("ATTEMPT TO SAVE UNKNOWN KIND OF RESET", TRUE);

			if ( IS_RIDER (tmp_mob) ) {
				for ( w = 0; w < MAX_WEAR; w++ ) {
					if ( !get_equip (tmp_mob->mount, w) )
						continue;

					obj = get_equip(tmp_mob->mount, w);
					fprintf (fp, "a 1 %d 0 %d\n", obj->virtual, w);
					if ( obj->contains && (GET_ITEM_TYPE(obj) == ITEM_CONTAINER || GET_ITEM_TYPE(obj) == ITEM_KEYRING || GET_ITEM_TYPE(obj) == ITEM_SHEATH || GET_ITEM_TYPE(obj) == ITEM_QUIVER) )				
						fprintf (fp, "s 1 %d %d\n", obj->contains->virtual, obj->contains->count);
				}
			}
		}
	}
}

void do_zsave(CHAR_DATA *ch, char *arg, int cmd)
{
	int		num, stat, i;

/** Use 226 as cmd to save locked zones**/	
	if ( !str_cmp (arg, "all") ) {
		for ( i = 0; i <= 98; i++ ) {
			if((cmd != 226) && (IS_SET(zone_table[i].flags,Z_LOCKED))){
				continue;
			}
			save_rooms (ch, i);
		}

		if ( port != BUILDER_PORT ) {
			i = 99;
			
			if((cmd != 226) && (IS_SET(zone_table[i].flags,Z_LOCKED))){
				/** do nothing in these cases **/;
			}
			else {
				save_rooms (ch, i);
			}
		}

		if ( port == BUILDER_PORT ) {
			/* system ("../scripts/backup"); //need to write up a script for this */
		}

		update_crafts_file();

		send_to_char ("All in-game zones have been saved.\n", ch);
		return;
	}

	for (; isspace(*arg); arg++){
		; /* runnng through the spaces */
	}

	if (!isdigit(*arg)){
		send_to_char("That is not a valid zone number.\n", ch);
		return;
	}

	num = strtol(arg, NULL, 10);

	if ( num > 99 || num < 0 ) {
		send_to_char ("You must specify a zone between 0 and 99.\n", ch);
		return;
	}

	if ( num == 99 && port == BUILDER_PORT ) {
		send_to_char ("Sorry, but this zone can't be saved on the builder  port.\n", ch);
		return;
	}

	if((cmd != 226) && (IS_SET(zone_table[num].flags,Z_LOCKED))) {
		send_to_char ("That zone is locked. Please try zsaving later.\n", ch);
		return;
	}

	if (num < 0 || num > 99){
		send_to_char("The zone must be between 0 and 99.\n", ch);
		return;
	}
	
	stat = save_rooms (ch, num);

	if(!stat){
		send_to_char("Saved.\n", ch);
	}
	else {
		send_to_char("Zone save failed - you should #2SHUTDOWN DIE#0 as soon as possible.\n",ch);
	}

	return;

}

FILE *open_and_rename (CHAR_DATA *ch, char *name, int zone)
{
	char		buf [MAX_STRING_LENGTH];
	FILE		*fp;

	snprintf (buf, MAX_STRING_LENGTH,  "%s/%s.%d", REGIONS, name, zone);

	if ( (fp = fopen (buf, "w")) == NULL )
		return NULL;

	return fp;
}

int save_rooms (CHAR_DATA *ch, int zone)
{
	char				buf [MAX_INPUT_LENGTH];
	CHAR_DATA			*tmob;
	OBJ_DATA			*tobj;
	ROOM_DATA			*troom;
	FILE				*fz;
	FILE				*fm;
	FILE				*fo;
	FILE				*fr;
	int					room_good;
	int					n;
	int					tmp;
	int					empty_rooms = 0;
	static int			total_empty_rooms = 0;

	snprintf (buf, MAX_STRING_LENGTH,  "Saving rooms in zone %d.", zone);
	system_log(buf, FALSE);

	if ( !(fr = open_and_rename (ch, "rooms", zone)) )
		return 1;

	if ( !(fz = open_and_rename (ch, "resets", zone)) )
		return 1;

	if ( !(fm = open_and_rename (ch, "mobs", zone)) )
		return 1;

	if ( !(fo = open_and_rename (ch, "objs", zone)) )
		return 1;

	fprintf (fz, "#%d\nLead: %s~\n%s~\n%d %d %d %ld %d %d\n",
		zone,
		zone_table [zone].lead,
		zone_table [zone].name,
		zone_table [zone].top,
		zone_table [zone].lifespan,
		zone_table [zone].reset_mode,
		zone_table [zone].flags,
		zone_table [zone].jailer,
		zone_table [zone].jail_room ?
			zone_table [zone].jail_room->virtual : 0);

	*buf = '\0';


	for ( troom = full_room_list; troom; troom = troom->lnext )
		if ( troom->zone == zone ) {

			room_good = 0;

			for ( n = 0; n < 6; n++ )
				if ( troom->dir_option [n] &&
					 troom->dir_option [n]->to_room > 0 )
					room_good = 1;
					
			if ( troom->contents || troom->people )
				room_good = 1;

			if ( strncmp (troom->description, "No Description Set", 18) )
				room_good = 1;

			if ( room_good ) {
				fwrite_room (troom, fr);
				fwrite_resets (troom, fz);
			} else {
				empty_rooms++;
				total_empty_rooms++;
			}
		}

	if ( empty_rooms ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%d empty rooms were not saved for zone %d.",
					empty_rooms, zone);
		system_log(buf, FALSE);

		strcat (buf, "\n");
		send_to_char (buf, ch);
	}

	for ( tmob = full_mobile_list; tmob; tmob = tmob->mob->lnext )
		if ( tmob->mob->zone == zone && !tmob->deleted && !IS_SET (tmob->act, ACT_STAYPUT) )
			fwrite_mobile (tmob, fm);

	for ( tobj = full_object_list; tobj; tobj = tobj->lnext )
		if ( tobj->zone == zone && tobj->virtual != -1 && tobj->virtual != 42 )
			if ( !tobj->deleted ||
				 get_obj_in_list_num (tobj->virtual, object_list) ) {

					/* Remove clan information from extra flags before
					   saving to disk...put it back afterwords */

				tmp = tobj->obj_flags.extra_flags;
				REMOVE_BIT (tobj->obj_flags.extra_flags, ITEM_LEADER);
				REMOVE_BIT (tobj->obj_flags.extra_flags, ITEM_MEMBER);
				REMOVE_BIT (tobj->obj_flags.extra_flags, ITEM_OMNI);
				fwrite_object (tobj, fo);
				tobj->obj_flags.extra_flags = tmp;
			}

	fprintf (fr, "$~\n");
	fprintf (fo, "$~\n");
	fprintf (fm, "$~\n");
	fprintf (fz, "S\n");

	fclose (fr);
	fclose (fz);
	fclose (fm);
	fclose (fo);

	return 0;
}

void do_rclone (CHAR_DATA *ch, char *argument, int cmd)
{
	int			i;
	ROOM_DATA	*source_room;
	ROOM_DATA	*target_room;
	char		buf1 [MAX_INPUT_LENGTH];
	char		buf2 [MAX_INPUT_LENGTH];

	argument = one_argument (argument, buf1);

	if ( !strtol(buf1, NULL, 10) ) {
		send_to_char("You must supply a virtual room number.\n", ch);
		return;
	}

	argument = one_argument (argument, buf2);

	if ( !strtol(buf2, NULL, 10) )
		target_room = ch->room;

	else {
		if ( !(target_room = vtor (strtol(buf2, NULL, 10))) ) {
			send_to_char ("No such target room.\n", ch);
			return;
		}
	}

	if ( !(source_room = vtor (strtol(buf1, NULL, 10))) ) {
		send_to_char ("No such source room.\n", ch);
		return;
	}

	target_room->name			= add_hash(source_room->name);
	target_room->description	= add_hash(source_room->description);
	target_room->sector_type	= source_room->sector_type;
	target_room->room_flags		= source_room->room_flags;
	target_room->zone			= target_room->virtual / ZONE_SIZE;

	if ( source_room->extra ) {
		CREATE (target_room->extra, ROOM_EXTRA_DATA, 1);
		for ( i = 0; i < WR_DESCRIPTIONS; i++ ) {
			if ( source_room->extra->weather_desc [i] )
				target_room->extra->weather_desc [i] =
						str_dup (source_room->extra->weather_desc [i]);
		}
	} else
		target_room->extra = NULL;

	send_to_char ("Done.\n", ch);
}

void do_rdoor (CHAR_DATA *ch, char *argument, int cmd)
{
	char buf1[256], buf2[256];
	int dir;
	ROOM_DATA		*room;

	half_chop(argument, buf1, buf2);

	switch (*buf1) {
		case 'n': dir = 0; break;
		case 'e': dir = 1; break;
		case 's': dir = 2; break;
		case 'w': dir = 3; break;
		case 'u': dir = 4; break;
		case 'd': dir = 5; break;
		default: dir = -1; break;
	}

	if (dir == -1) {
		send_to_char("What direction is that?\n", ch);
		return;
	}

	room = vtor (ch->in_room);

	if ( !room->dir_option [dir] ) {
		send_to_char("There is no exit in that direction.\n", ch);
		return;
	}

	SET_BIT (room->dir_option [dir]->exit_info, PASSAGE_ISDOOR);

	if ( !*buf2 )
		strcpy (buf2, "door");

	room->dir_option [dir]->keyword = add_hash (buf2);

	send_to_char ("Done.\n", ch);
}


/*                                                                          *
 * funtion: do_review                    < e.g.> review <name>              *
 *                                                                          *
 * 09/17/2004 [JWW] - Fixed an instances where mysql result was not freed   *
 *                                                                          */
void do_review (CHAR_DATA *ch, char *argument, int cmd)
{
	int				silent_review = 0;
	CHAR_DATA			*review_ch;
        MYSQL_RES       		*result;
	char				buf [MAX_INPUT_LENGTH];
	char				name [MAX_INPUT_LENGTH];

	argument = one_argument (argument, name);

	if ( IS_NPC (ch) ) {
		send_to_char ("You must RETURN before reviewing any applications.\n", ch);
		return;
	}

	if ( port == BUILDER_PORT ) {
		send_to_char ("Applications may only be reviewed on the player port.\n", ch);
		return;
	}

	if ( !isalpha (*name) ) {
		send_to_char ("Illegal name.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf == '!' )
		silent_review = 1;

	if ( !(review_ch = load_pc (name)) ) {
		send_to_char ("There is no pfile associated with that name, "
					  "sorry.\n", ch);
		return;
	}

	if ( IS_MORTAL(ch) && (!is_newbie (review_ch) || IS_SET (review_ch->plr_flags, PRIVATE)) ) {
		send_to_char ("There is no playerfile associated with that name, sorry.\n", ch);
		return;
	}

	if ( review_ch->pc->create_state != 1 ) {
                snprintf (buf, MAX_STRING_LENGTH,  "It appears that this application has already been reviewed and responded to.\n");
		send_to_char (buf, ch);
		unload_pc (review_ch);
		return;
	}

	if ( is_being_reviewed (review_ch->tname, ch->desc->account->name) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "That application is currently checked out - try again later.\n");
		send_to_char (buf, ch);
		unload_pc (review_ch);
		return;
	}

	spitstat (review_ch, ch->desc);

	ch->delay_type  = DEL_APP_APPROVE;
	ch->delay		= 3 * 60;
	ch->delay_who	= str_dup (name);

	mysql_safe_query ("SELECT * FROM virtual_boards WHERE board_name = 'Applications' AND subject LIKE '%% %s'", review_ch->tname);

	if ( ( result = mysql_store_result ( database ) ) != NULL ) {
	  if ( mysql_num_rows ( result ) >= 1 ) {
	    send_to_char ("\n#6Please type HISTORY to review previous responses to this application.#0\n", ch);
	  }
	  mysql_free_result ( result );
	  result = NULL;
	}
	send_to_char ("\n#2Please ACCEPT or DECLINE this application.#0\n", ch);

	if ( !is_yours (review_ch->tname, ch->desc->account->name) ) {
		mysql_safe_query ("INSERT INTO reviews_in_progress VALUES ('%s', '%s', UNIX_TIMESTAMP())", review_ch->tname, ch->desc->account->name);
	}

	unload_pc (review_ch);
}

void show_reg_by_name (CHAR_DATA *ch, char *reg_name)
{
	char			buf [MAX_INPUT_LENGTH];
	int				index = 0;
	int				count = 0;
	REGISTRY_DATA	*reg = NULL;
	char			*temp_arg = NULL;

	if ( !*reg_name )
		strcpy (buf, "registry");
	else
		strcpy (buf, reg_name);

	send_to_char ("\n", ch);

	if ( (index = lookup_value (buf, REG_REGISTRY)) == -1 ) {
		send_to_char ("No such registry.\n", ch);
		return;
	}

	reg = registry [index];

	for ( reg = registry [index]; reg; reg = reg->next ) {
		if (index == REG_MISC ) {
			temp_arg = lookup_string (reg->value, REG_MISC_NAMES);
			snprintf (buf, MAX_STRING_LENGTH,  "  %20s          %s\n",
						temp_arg,
						reg->string);
		}
		
		else if ( index == REG_CAP || index == REG_OV || index == REG_LV ){
			temp_arg = lookup_string (reg->value, REG_SKILLS);	
			snprintf (buf, MAX_STRING_LENGTH,  "  %20s          %s\n",
						temp_arg,
						reg->string);
		}
		
		else if ( index == REG_MAX_RATES){
			temp_arg = skill_data[reg->value].skill_name;	
			snprintf (buf, MAX_STRING_LENGTH,  "  %20s      %s\n",
						temp_arg,
						reg->string);
		}
		else
			snprintf (buf, MAX_STRING_LENGTH,  "  %-15s%s", reg->string, ++count % 4 ? "" : "\n");

		send_to_char (buf, ch);
	}

	send_to_char ("\n", ch);
}

void olist_show (OBJ_DATA *obj, int type, int header)
{
	char		buf [MAX_STRING_LENGTH];
	char		wear_loc [MAX_STRING_LENGTH];
	char		armor_type [MAX_STRING_LENGTH];

	switch ( type ) {

	case ITEM_WEAPON:

		if ( header )
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				"Virt# Dam  Weight  Value Delay Qty 1st-key           Short Description\n");

		(void)one_argument (obj->name, buf);

		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				"%.5d%s%1dd%-2d %3d.%.2d  %5d  %3d  %2d  %-13.13s  %-28.28s\n",
				obj->virtual, *obj->full_description ? " ": "*",
				obj->o.weapon.dice, obj->o.weapon.sides,
				obj->obj_flags.weight / 100, obj->obj_flags.weight % 100,
				obj->obj_flags.cost, obj->o.weapon.delay, obj->quality,
				buf, obj->short_description);

		break;

	case ITEM_ARMOR:

		if ( header )
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				"Virt# AC Weight  Value Type   Wear-Flgs 1st-key        Short Description\n");

		(void)one_argument (obj->name, buf);

		switch ( obj->o.armor.armor_type ) {
			case 0:  strcpy (armor_type, "Quilted "); break;
			case 1:  strcpy (armor_type, "Leather "); break;
			case 2:  strcpy (armor_type, "Ring  "); break;
			case 3:  strcpy (armor_type, "Scale "); break;
			case 4:  strcpy (armor_type, "Mail  "); break;
			case 5:  strcpy (armor_type, "Plate "); break;
			default: strcpy (armor_type, "????? "); break;
		}

		*wear_loc = '\0';

		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FINGER) )
			strcat (wear_loc, "FING ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_NECK) )
			strcat (wear_loc, "NECK ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_BODY) )
			strcat (wear_loc, "BODY ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HEAD) )
			strcat (wear_loc, "HEAD ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_LEGS) )
			strcat (wear_loc, "LEGS ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FEET) )
			strcat (wear_loc, "FEET ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HANDS) )
			strcat (wear_loc, "HAND ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ARMS) )
			strcat (wear_loc, "ARMS ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_SHIELD) )
			strcat (wear_loc, "SHLD ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_ABOUT) )
			strcat (wear_loc, "ABOT ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WAIST) )
			strcat (wear_loc, "WSTE ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_WRIST) )
			strcat (wear_loc, "WRST ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_HAIR) )
			strcat (wear_loc, "HAIR ");
		if ( IS_SET (obj->obj_flags.wear_flags, ITEM_WEAR_FACE) )
			strcat (wear_loc, "FACE ");

		if ( strlen (wear_loc) > 10 ) {
			wear_loc [14] = '+';
			wear_loc [15] = '\0';
		}

		else
			while ( strlen (wear_loc) < 10 )
				strcat (wear_loc, " ");

		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				"%.5d%s%2d %3d.%.2d  %5d %s %s%-13.13s  %-24.24s\n",
				obj->virtual, *obj->full_description ? " " : "*",
				obj->o.armor.armor_value,
				obj->obj_flags.weight / 100, obj->obj_flags.weight % 100,
				obj->obj_flags.cost, armor_type, wear_loc, buf,
				obj->short_description);

		break;

	case ITEM_DRINKCON:
	case ITEM_FOUNTAIN:
		if ( header )
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				"Virt#     Weight ##drnk  Cap Contents                   Short Description\n");

		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				 "%.5d%s %5d.%02d %5d %5d %-25s    %s\n",
				 obj->virtual,
				 (obj->full_description && *obj->full_description) ? " " : "*",
				 obj->obj_flags.weight / 100, obj->obj_flags.weight % 100,
				 obj->o.drinkcon.volume,
				 obj->o.drinkcon.capacity,
				 !obj->o.drinkcon.volume ? "(empty)" :
				 vtoo (obj->o.drinkcon.liquid) ?
				 vtoo (obj->o.drinkcon.liquid)->short_description : "not set",
				 obj->short_description);

		break;

	case ITEM_FOOD:
		if ( header )
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				"Virt # Foodval bites spel1 spel2 spel3 spel4 Short Description\n");

		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				 "%.5d%s %5d %5d %5d %5d %5d %5d   %s\n",
				 obj->virtual,
				 (obj->full_description && *obj->full_description) ? " " : "*",
				 obj->o.food.food_value,
				 obj->o.food.bites,
				 obj->o.food.spell1,
				 obj->o.food.spell2,
				 obj->o.food.spell3,
				 obj->o.food.spell4,
				 obj->short_description);
		break;
	case -1:
	default:
		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				"%.5d%s %5d  %-25.25s  %-25.25s",
				obj->virtual, (obj->full_description && *obj->full_description)
									? " " : "*",
				obj->obj_flags.cost, obj->name,
				obj->short_description);
		if ( obj->obj_flags.type_flag == ITEM_DRINKCON && obj->o.od.value[1] && obj->o.od.value[2] && vtoo(obj->o.od.value[2]))
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, " #6(%s)#0", vtoo(obj->o.od.value[2])->short_description);
		strcat (b_buf, "\n");
		break;
	}
}

void do_olist (CHAR_DATA *ch, char *argument, int cmd)
{
	int			header = 1;
	int			type = -1;
	int			inclusive;
	int			zone = -1;
	int			yes_key1 = 0;
	int			yes_key2 = 0;
	int			yes_key3 = 0;
	int			count = 0;
	OBJ_DATA	*obj;
	char		key1 [MAX_STRING_LENGTH] = { '\0' };
	char		key2 [MAX_STRING_LENGTH] = { '\0' };
	char		key3 [MAX_STRING_LENGTH] = { '\0' };
	char		buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Selection Parameters:\n\n", ch);
		send_to_char ("   +/-<object keyword>       Include/exclude object keyword.\n", ch);
		send_to_char ("   <zone>                    Objects from zone only.\n", ch);
		send_to_char ("   <item-type>               Include items of item-type.\n", ch);
		send_to_char ("\nExample:   olist +sword -rusty weapon 10\n", ch);
		send_to_char ("will only get non-rusty swords of type weapon from zone 10.\n", ch);
		return;
	}

	while ( *buf ) {

		inclusive = 1;

		if ( strlen (buf) > 1 && isalpha (*buf) &&
			 (type = index_lookup (item_types, buf)) != -1 ) {
			argument = one_argument (argument, buf);
			continue;
		}

		if ( isdigit (*buf) ) {

			if ( (zone = strtol(buf, NULL, 10)) >= MAX_ZONE ) {
				send_to_char ("Zone not in range 0..99\n", ch);
				return;
			}

			argument = one_argument (argument, buf);
			continue;
		}

		switch ( *buf ) {

		case '-':
			inclusive = 0;
			if ( !buf [1] ) {
				send_to_char ("Expected keyname after 'k'.\n", ch);
				return;
			}

			if ( !*key1 ) {
				yes_key1 = inclusive;
				strcpy (key1, buf + 1);
			} else if ( !*key2 ) {
				yes_key2 = inclusive;
				strcpy (key2, buf + 1);
			} else if ( *key3 ) {
				send_to_char ("Sorry, at most three keywords.\n", ch);
				return;
			} else {
				yes_key3 = inclusive;
				strcpy (key3, buf + 1);
			}

			break;
			
		case '+':

			if ( !buf [1] ) {
				send_to_char ("Expected keyname after 'k'.\n", ch);
				return;
			}

			if ( !*key1 ) {
				yes_key1 = inclusive;
				strcpy (key1, buf + 1);
			} else if ( !*key2 ) {
				yes_key2 = inclusive;
				strcpy (key2, buf + 1);
			} else if ( *key3 ) {
				send_to_char ("Sorry, at most three keywords.\n", ch);
				return;
			} else {
				yes_key3 = inclusive;
				strcpy (key3, buf + 1);
			}

			break;

		case 'z':

			argument = one_argument (argument, buf);

			if ( !isdigit (*buf) || strtol(buf, NULL, 10) >= MAX_ZONE ) {
				send_to_char ("Expected valid zone after 'z'.\n", ch);
				return;
			}

			zone = strtol(buf, NULL, 10);

			break;
		}

		argument = one_argument (argument, buf);
	}

	*b_buf = '\0';

	for ( obj = full_object_list; obj; obj = obj->lnext ) {

		if ( zone != -1 && obj->zone != zone )
			continue;

		if ( type != -1 && obj->obj_flags.type_flag != type )
			continue;

		if ( *key1 ) {
			if ( yes_key1 && !name_is (key1, obj->name) )
				continue;
			else if ( !yes_key1 && name_is (key1, obj->name) )
				continue;
		}

		if ( *key2 ) {
			if ( yes_key2 && !name_is (key2, obj->name) )
				continue;
			else if ( !yes_key2 && name_is (key2, obj->name) )
				continue;
		}

		if ( *key3 ) {
			if ( yes_key3 && !name_is (key3, obj->name) )
				continue;
			else if ( !yes_key3 && name_is (key3, obj->name) )
				continue;
		}

		count++;

		if ( count < 200 )
			olist_show (obj, type, header);

		header = 0;
	}

	if ( count > 200 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "You have selected %d objects (too many to print).\n",
					  count);
		send_to_char (buf, ch);
		return;
	}

	page_string (ch->desc, b_buf);
}

/*                                                                          *
 * funtion: do_show                      < e.g.> show (k|a|l|v|m|n|o|q|r|u|z) *
 *                                                                          *
 *                                                                          */
void do_show (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch;
	DESCRIPTOR_DATA		*d;
	char			buf [MAX_STRING_LENGTH];
	char 			buf1[MAX_STRING_LENGTH]; /* type of show (k|a|l... */
	char			buf2[MAX_STRING_LENGTH]; /* first option/value */
	char			buf3[MAX_STRING_LENGTH]; /* second option/value */
	char			buf4[MAX_STRING_LENGTH]; /* third option/value */
	char			tmp[MAX_STRING_LENGTH];
	char 			output[MAX_STRING_LENGTH];
	char			*date =  NULL;
	char			*obj_name;
	char			*mob_name;
	char			*clan_name = {'\0'};
	char			*zone_str;
	int 			mtop=0;
	int				count; 
	int				count2;
	int				otop=0;
	int				numdex;
	int				index;
	int				count_total_rooms;
	int				docs = 0;
	int				keepers_only = 0;
	int				got_line = 0;
	int				ok_only = 0;
	int				craft_tot=0;
	int				big_counts_only = 0;
	int				undesc_only = 0;
	int 			stayput = 0;
	int				chars = 0;
	int				accounts = 0;
	int				search_type = 0;
	bool			found = FALSE;
	OBJ_DATA		*objdex; 
	CHAR_DATA		*chardex; 
	ROOM_DATA		*troom;
	SUBCRAFT_HEAD_DATA	*craft;
	MYSQL_RES		*result;
	MYSQL_ROW		row;
	time_t			current_time;

	arg_splitter(4, argument, buf1, buf2, buf3, buf4);

	if ( IS_MORTAL (ch) && !IS_NPC(ch) && !ch->pc->is_guide) {
		send_to_char ("Eh?\n", ch);
		return;
	}

	if ( ch->pc->is_guide && IS_MORTAL(ch) ) {
		if ( *buf1 != 'l' && *buf1 != 'L' ) {
			send_to_char ("Type SHOW L to get a list of applications in the queue.\n", ch);
			return;
		}
	}

	if ( !*buf1 ) {
		s ("   a           per zone stats");
		s ("   c           characters matching search");
		s ("   k           shopkeepers");
		s ("   l           applications");
		s ("   m           mobiles");
		s ("   n           clans for mobiles");
	    s ("   o           objects");
		s ("   p           PCs with specified points");
		s ("   q           objects with ok flag");
		s ("   r           rooms");
		s ("   s           summary building info");
        s ("   u           undescribed rooms");
		s ("   v           registry.");
		s ("   z           zones");
	}

	switch (*buf1) {
	
		case 'a':	/** Summary of rooms, players, mob, and objects by zone **/
			snprintf( tmp, MAX_STRING_LENGTH, "    Name                     Rooms     Plyrs     Mobs     Objs\n\n");
			for(index = 0; index < MAX_ZONE; index++) {
				if(strcmp(zone_table[index].name, "Empty Zone")) {
					snprintf( tmp + strlen(tmp), MAX_STRING_LENGTH, "%2d - %-16.16s", index, zone_table[index].name);
					count = 0;
					numdex = 0;
					for ( troom = full_room_list; troom; troom = troom->lnext ){
						if ( troom->zone == index ) {
							count++;
							if(!(strncmp(troom->description, "No Description Set", 18))){
								numdex++;
							}
						}
					}
					snprintf (tmp + strlen(tmp), MAX_STRING_LENGTH, "%10d/%-4d", count, numdex);
					count = 0;
					count2 = 0;
					for(chardex = character_list; chardex; chardex = chardex->next) { 
						if ( !IS_NPC(chardex) )
							continue;
						/* counts PCs only in room Q000  */
						if ( chardex->in_room / 1000 == index )
							count++;
					}
			
					for (chardex = character_list; chardex; chardex = chardex->next){
						if (!IS_NPC(chardex) && chardex->room && chardex->room->zone == index){
							count2++;
						}
					}
					snprintf (tmp + strlen(tmp), MAX_STRING_LENGTH, "%6d%10d", count2, count);
					count = 0;

/** will count thee different types of objecs, not a full count of all items **/
					for ( objdex = object_list; objdex; objdex = objdex->next ){
						if ( !objdex->deleted && objdex->zone == index ){
							count++;
						}
					}

				snprintf (tmp + strlen(tmp), MAX_STRING_LENGTH,"%10d\n",count);
			}
		}
		page_string(ch->desc,tmp);
		break;


	case 'c': /** show characters matching search paramaters **/
		if ( !str_cmp (buf2, "keyword") )
			search_type = SEARCH_KEYWORD;
		else if ( !str_cmp (buf2, "sdesc") )
			search_type = SEARCH_SDESC;
		else if ( !str_cmp (buf2, "ldesc") )
			search_type = SEARCH_LDESC;
		else if ( !str_cmp (buf2, "fdesc") )
			search_type = SEARCH_FDESC;
		else if ( !str_cmp (buf2, "clan") )
			search_type = SEARCH_CLAN;
		else if ( !str_cmp (buf2, "skill") )
			search_type = SEARCH_SKILL;
		else if ( !str_cmp (buf2, "room") )
			search_type = SEARCH_ROOM;
		else if ( !str_cmp (buf2, "race") )
			search_type = SEARCH_RACE;
		else if ( !str_cmp (buf2, "level") && GET_TRUST (ch) >= 5 )
			search_type = SEARCH_LEVEL;
		else {
			if ( GET_TRUST (ch) < 5 )
				send_to_char ("Search for: keyword, sdesc, ldesc, fdesc, clan, skill, or room.\n", ch);
			else send_to_char ("Search for: keyword, sdesc, ldesc, fdesc, clan, skill, room, or level.\n", ch);
			return;
		}
		
		result = mysql_player_search (search_type, buf3);
		if ( !result || !mysql_num_rows (result) ) {
			send_to_char ("No playerfiles matching your search were found.\n", ch);
			if ( result )
				mysql_free_result (result);
			return;
		}
		
		snprintf (buf, MAX_STRING_LENGTH,  "#6Playerfiles Matching Search: %d#0\n\n", (int)mysql_num_rows(result));
		
		index = 1;
		
		while ( (row = mysql_fetch_row (result)) ) {
			if ( strtol(row[8], NULL, 10) == 2 )
				snprintf (buf3, MAX_STRING_LENGTH,  " #2(Approved)#0");
			
			else if ( strtol(row[8], NULL, 10) == 4 )
				snprintf (buf3, MAX_STRING_LENGTH,  " #1(Deceased)#0");
			
			else if ( strtol(row[8], NULL, 10) == 3 )
				snprintf (buf3, MAX_STRING_LENGTH,  " #5(Suspended)#0");
			
			else snprintf (buf3, MAX_STRING_LENGTH,  " #6(Pending)#0");
			snprintf (buf2, MAX_STRING_LENGTH,  "%4d. %-20s%s\n", index, row[0], buf3);
			if ( strlen(buf) + strlen(buf2) >= MAX_STRING_LENGTH ){
				send_to_char("Too many playerfiles being accesed - Please restrict your search", ch);
				break;
			}
			else snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s", buf2);
			index++;
		}
		page_string (ch->desc, buf);
		mysql_free_result (result);
		return;
		
	case 'v':			/** registry entry **/
		show_reg_by_name (ch, buf2);
		break;

	case 'p':		/** show by point criteria **/
		if ( !*buf2 ) {
			send_to_char ("What did you wish the point criterion to be?\n", ch);
			return;
		}
		if ( !isdigit (*buf2) ) {
			send_to_char ("Please specify a number of points.\n", ch);
			return;
		}
		*buf = '\0';
                for (d = descriptor_list; d; d = d->next) {
                	if (d->character && IS_MORTAL (d->character) && (d->connected == CON_PLYNG) && (d->character->in_room != NOWHERE) &&
				d->account && d->account->roleplay_points <= strtol(buf2, NULL, 10) &&
				(time(0) - d->character->pc->last_rpp >= 60*60*24*60) ) {
				found = TRUE;
                        	snprintf (output, MAX_STRING_LENGTH,  "%-20s - %s [%d]\n",
                                	d->character->tname,
                                        vtor (d->character->in_room)->name,
                                        vtor (d->character->in_room)->virtual);
				if ( strlen(output) + strlen(buf) >= MAX_STRING_LENGTH ) {
					send_to_char ("Too many matches found - please use a more selective criterion!\n", ch);
					return;
				}
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s", output);
                        }
                }

		if ( found ) {
			snprintf (output, MAX_STRING_LENGTH,  "#2The following PCs online currently have %d roleplay points or less and\n"
				      "have not been awarded or deducted within the last 60 days:#0\n\n%s", (int)strtol(buf2, NULL, 10), buf);
		}
		else {
			snprintf (output, MAX_STRING_LENGTH,  "#1No PCs currently online have %d roleplay points or less who have not\n"
				      "been awarded or deducted within the last 60 days.#0\n", (int)strtol(buf2, NULL, 10));
		}

		page_string (ch->desc, output);

		break;

	case 'l':			/** Pending applications **/

		date = timestr(date);
		snprintf ( buf, MAX_STRING_LENGTH, "\nApplication Queue, as of #2%s#0:\n\n", date );
		mem_free ( date ) ;
		send_to_char (buf, ch);

		mysql_safe_query ("SELECT name,birth,account,plrflags FROM %s.pfiles WHERE create_state = 1 ORDER BY birth ASC", PFILES_DATABASE);
		result = mysql_store_result (database);

		if ( result ) {
			while ( (row = mysql_fetch_row(result)) ) {
				tch = load_pc (row[0]);
				if ( !tch )
					continue;
				if ( IS_MORTAL (ch) && (!is_newbie(tch))) {
					unload_pc (tch);
					continue;
				}
				got_line++;
				snprintf (buf, MAX_STRING_LENGTH,  " %-20s", row[0]);
				current_time = strtol(row[1], NULL, 10);
				date = (char *) malloc (256) ;
				date[0] = '\0' ;
				if ( asctime_r ( localtime ( &current_time ), date ) != NULL ) {
				  date [ strlen ( date ) - 1 ] = '\0';
				}
				if ( (time(0) - current_time) - (60*60*48) >= 1 )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#1[Submitted: %s]#0", date); 
				else if ( (time(0) - current_time) - (60*60*24) >= 1 )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#3[Submitted: %s]#0", date); 
				else snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2[Submitted: %s]#0", date);
				mem_free ( date );

				if ( !IS_NPC(ch) && is_newbie(tch) && !IS_MORTAL(ch) ) 
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " #6(New)#0");

				if ( !IS_NPC(ch) && is_yours (row[0], ch->pc->account) )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " #3(Yours)#0");
				else if ( !IS_NPC(ch) && is_being_reviewed (row[0], ch->pc->account) )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " #2(Checked Out)#0");

				if ( !IS_NPC(ch) && IS_SET (tch->plr_flags, PRIVATE) && !IS_MORTAL(ch) ) 
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " #1(Private)#0");		
				strcat (buf, "\n");
				send_to_char (buf, ch);
				unload_pc (tch);
			}		

			mysql_free_result (result);
			result = NULL;
		}

		if ( !got_line )
			send_to_char ("  None\n", ch);

		break;

	case 'k':		/** shows shopkeepers and thier location **/
		keepers_only = 1;
		mob_name = buf3;
		zone_str = NULL;

		if ( !isdigit (*buf2) )
			mob_name = str_dup(buf2);
		else
			zone_str = buf2;

		snprintf (tmp, MAX_STRING_LENGTH, "Vnum  K    Name(s)           Short desc"
					 "              Clones\t\tIn Room\n\n");
		for ( chardex = full_mobile_list; chardex; chardex = chardex->mob->lnext ) {

			if ( !IS_NPC (chardex) )
				continue;

			if ( keepers_only && !chardex->shop )
				continue;

			if ( zone_str && chardex->mob->zone != strtol(zone_str, NULL, 10) )
				continue;

			if ( !*chardex->name )
				continue;

			if ( (*mob_name && strstr(chardex->name,mob_name)) || !*mob_name) {
				if ( IS_SET (chardex->flags, FLAG_KEEPER) )
					snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "%.5d * %-16.16s  %-28.28s  %.3d\t\t%.5d\n", chardex->mob->virtual, chardex->name, chardex->short_descr, 999, chardex->in_room);
				
			}
		}
		page_string (ch->desc, tmp);
		break;

	case 'm':			/** shows summary of all mobiles **/

		mob_name = buf3;
		zone_str = NULL;

		if ( !isdigit (*buf2) )
			mob_name = str_dup(buf2);
		else
			zone_str = buf2;

		snprintf (tmp, MAX_STRING_LENGTH, "Vnum  K    Name(s)           Short desc\n\n");
		for ( chardex = full_mobile_list; chardex; chardex = chardex->mob->lnext ) {

			if ( !IS_NPC (chardex) )
				continue;

			if ( zone_str && chardex->mob->zone != strtol(zone_str, NULL, 10) )
				continue;

			if ( !*chardex->name )
				continue;

			if ( ((*mob_name && strstr(chardex->name,mob_name)) ||!*mob_name)){
				if	(!IS_SET (chardex->flags, FLAG_KEEPER)){
					snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH,
						"%.5d   %-16.16s  %-28.28s\n",
						chardex->mob->virtual, chardex->name,
						chardex->short_descr);
				}
				else{
					snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, 
						"%.5d * %-16.16s  %-28.28s\n",
						chardex->mob->virtual, chardex->name,
						chardex->short_descr);
				}
			}
		}
		page_string (ch->desc, tmp);
		break;
	
		case 'n':		/** NPC clans members **/	
			clan_name = buf3;
			zone_str = NULL;

			if ( !isdigit (*buf2) )
				clan_name = str_dup(buf3);
			else
				zone_str = buf2;
			
			snprintf(tmp, MAX_STRING_LENGTH, "Vnum    Name(s)       Short desc\n");
			
			for ( chardex = full_mobile_list; chardex; chardex = chardex->mob->lnext ) {
			
				if ( !IS_NPC (chardex) )
					continue;

				if ( zone_str && chardex->mob->zone != strtol(zone_str, NULL, 10) )
					continue;

				if ( !*chardex->name )
					continue;
				
				if (!*chardex->clans)
					continue;
					
				if ((*clan_name && strstr(chardex->clans, clan_name)) || !*clan_name){
					snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "%.5d   %-12.12s  %-s\n\t%s\n\n", chardex->mob->virtual, chardex->name, chardex->short_descr, chardex->clans);
				}
			}
			page_string (ch->desc, tmp);

			break;


	case '!':	/** will show only large groups of specified object **/
		big_counts_only = 1;

	case 'q':		/** Object only with ok flag **/
		ok_only = 1;

	case 'o':		/** Summary of all objects **/

		obj_name = str_dup(buf3);
		zone_str = NULL;

		if ( !isdigit (*buf2) )
			obj_name = buf2;
		else
			zone_str = buf2;

		snprintf (tmp, MAX_STRING_LENGTH, "Vnum       Name(s)           Short desc"
					  "               Clones\tLocation\n\n");
		for ( objdex = full_object_list; objdex; objdex = objdex->lnext ) {

			if ( zone_str && objdex->zone != strtol(zone_str, NULL, 10) )
				continue;

			if ( ok_only && !IS_SET (objdex->obj_flags.extra_flags, ITEM_OK) )
				continue;

			if ( big_counts_only && objdex->count < 100 )
				continue;

			if ( (*obj_name && strstr (objdex->name, obj_name)) || !*obj_name ) {
				if ( !*objdex->name )
					continue;
				if ( objdex->in_obj )
					snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, 						"%.5d  %-16.16s  %-28.28s  %.3d\t(I)%.5d#0\n", objdex->virtual, objdex->name, objdex->short_description, 999, objdex->in_obj->virtual);

				else if ( objdex->carried_by )
					snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "%.5d  %-16.16s  %-28.28s  %.3d\t(C)%.5d#0\n", objdex->virtual, objdex->name, objdex->short_description, 999, objdex->carried_by->mob ? objdex->carried_by->mob->virtual : 0);

				else if ( objdex->equiped_by )
					snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "%.5d  %-16.16s  %-28.28s  %.3d\t(E)%.5d#0\n", objdex->virtual, objdex->name, objdex->short_description, 999, objdex->equiped_by->mob ? objdex->equiped_by->mob->virtual : 0);

				else
					snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "%.5d  %-16.16s  %-28.28s  %.3d\t(R) %05d#0\n", objdex->virtual, objdex->name, objdex->short_description, 999, objdex->in_room);
			}
		}
		page_string (ch->desc, tmp);
		break;


	case 'u':		/** Shows undescribed room only */

		*tmp = 0;
		for ( troom = full_room_list; troom; troom = troom->lnext ) {

			if ( strncmp (troom->description, "No Description Set", 18) )
				continue;

			if ( IS_SET (troom->room_flags, STORAGE) )
				continue;

			snprintf (tmp + strlen(tmp), MAX_STRING_LENGTH, "[%5d] %s", troom->virtual,troom->name);
			snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "\n");
		}
		page_string (ch->desc, tmp);
		return;
		
	case 'r':	/** All Rooms **/

		if ( buf1 [1] == '*' )
			ok_only = 1;

		*tmp = 0;
		for ( troom = full_room_list; troom; troom = troom->lnext ) {

			if ( *buf3 && strtol(buf3, NULL, 10) > troom->virtual)
				continue;

			if ( *buf4 && strtol(buf4, NULL, 10) < troom->virtual)
				continue;

			if ( !ok_only && troom->zone != strtol(buf2, NULL, 10) )
				continue;

			if ( undesc_only && !IS_SET (troom->room_flags, STORAGE) &&
				 strncmp (troom->description, "No Description Set", 18) )
				continue;

			if ( ok_only && !IS_SET (troom->room_flags, ROOM_OK) )
				continue;

			if ( IS_SET (troom->room_flags, STORAGE) )
				strcat (tmp, "S");
			else
				strcat (tmp, " ");

			if ( troom->prg )
				strcat (tmp, "P");
			else
				strcat (tmp, " ");

			snprintf (tmp + strlen(tmp), MAX_STRING_LENGTH, "[%5d] %s", troom->virtual,troom->name);

			for ( numdex = 0; numdex < 6; numdex++ ) {
				if ( troom->dir_option [numdex] &&
					 troom->dir_option [numdex]->to_room != -1) {
					switch (numdex) {
					case 0:
						snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "  N [%5d]",
							troom->dir_option [numdex]->to_room);
						break;
					case 1:
						snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "  E [%5d]",
							troom->dir_option [numdex]->to_room);
						break;
					case 2:
						snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "  S [%5d]",
							troom->dir_option [numdex]->to_room);
						break;
					case 3:
						snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "  W [%5d]",
							troom->dir_option [numdex]->to_room);
						break;
					case 4:
						snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "  U [%5d]",
							troom->dir_option [numdex]->to_room);
						break;
					case 5:
						snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "  D [%5d]",
							troom->dir_option [numdex]->to_room);
						break;
					}
				}
			}
			snprintf (tmp + strlen (tmp), MAX_STRING_LENGTH, "\n");
		}
		page_string (ch->desc, tmp);
		return;
		
	case 'z':		/** Shows zone name and numbers **/
		for ( index = 0; index < MAX_ZONE; index += 5 ) {
			snprintf (tmp, MAX_STRING_LENGTH, "##%2d %-11.11s ##%2d %-11.11s ##%2d %-11.11s ##%2d %-11.11s ##%2d %-11.11s\n", index, zone_table[index].name, 
index + 1, zone_table [index+1].name, index + 2, zone_table [index+2].name, index + 3, zone_table [index+3].name, index + 4, zone_table [index+4].name);
			send_to_char (tmp, ch);
		}
		return;
		
	
        case 's':		/** shows build summart overall stats **/
                numdex = 0;
                otop = 0;
                mtop = 0;
                                
                for ( count_total_rooms = 0, troom = full_room_list;
                          troom;
                          troom = troom->lnext, count_total_rooms++ )
                        if(!(strncmp(troom->description,"No Description Set", 18)) ||
                           !strncmp(troom->description, "   No Description Set", 21) )
                                numdex++;
                
                for ( objdex = full_object_list; objdex; objdex = objdex->lnext )
                        if ( !objdex->deleted )
                                otop++;

                for ( chardex = full_mobile_list; chardex; chardex = chardex->mob->lnext )
                        if ( !chardex->deleted && IS_NPC (chardex) )
                                mtop++;
                
                for ( chardex = character_list; chardex; chardex = chardex->next ) {
			if ( chardex->deleted )
				continue;
			if ( IS_NPC (chardex) && IS_SET (chardex->act, ACT_STAYPUT) )
                                stayput++;
		}

                for ( craft = crafts; craft; craft = craft->next )
                        craft_tot++;
                        
                mysql_safe_query ("SELECT count(name) from %s.pfiles", PFILES_DATABASE);
                result = mysql_store_result (database);
                if ( result ) {
                        row = mysql_fetch_row (result);
                        if ( row )
                                chars = strtol(row[0], NULL, 10);
                        mysql_free_result (result);
			result = NULL;
                }
                                
                mysql_safe_query ("SELECT count(username) from forum_users");
                result = mysql_store_result (database);
                if ( result ) {
                        row = mysql_fetch_row (result);
                        if ( row )  
                                accounts = strtol(row[0], NULL, 10);
                        mysql_free_result (result);
			result = NULL;
                }

                mysql_safe_query ("SELECT count(*) from player_writing");
                result = mysql_store_result (database);
                if ( result ) {
                        row = mysql_fetch_row (result);
                        if ( row )
                                docs = strtol(row[0], NULL, 10);
                        mysql_free_result (result);
			result = NULL;
                }
                
                snprintf(tmp, MAX_STRING_LENGTH,
                		   "\n   Described Rooms:  %-5d\n"
                             "   Total Rooms:      %-5d\n"
                             "   Total Mobiles:    %-5d\n"
                             "   Total Objects:    %-5d\n"
                             "   Total Crafts:     %-5d\n\n"
                             "   Total Characters: %-5d\n"
                             "   Total Accounts:   %-5d\n"
                             "   Player Writings:  %-5d\n\n"
			     			 "   Stayput Mobiles:  %-5d\n",
                             count_total_rooms-numdex, count_total_rooms, mtop, otop, craft_tot, chars, accounts, docs, stayput);
                send_to_char(tmp, ch);
		return;
	default:
		send_to_char("Not a valid show option.\n",ch);
		return;
	}
}

void do_zname (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH];

	while ( isspace (*argument) )
		argument++;

	snprintf (buf, MAX_STRING_LENGTH,  "zset %d name %s", ch->room->zone, argument);

	command_interpreter (ch, buf);
}

void do_rlink(CHAR_DATA *ch, char *argument, int cmd)
{

	char			buf1 [MAX_INPUT_LENGTH];
	char			buf2 [MAX_INPUT_LENGTH];
	int				dir;
	int				cha_rnum;
	ROOM_DATA		*troom;
	
	argument = one_argument (argument, buf1);
	argument = one_argument (argument, buf2);
	
	if ( !*buf2 ) {
		send_to_char ("Syntax:  rlink <direction> <room-number>\n", ch);
		return;
	}

	switch ( *buf1 ) {
		case 'n': dir = 0; break;
		case 'e': dir = 1; break;
		case 's': dir = 2; break;
		case 'w': dir = 3; break;
		case 'u': dir = 4; break;
		case 'd': dir = 5; break;
		default: dir = -1; break;
	}

	if (dir == -1) {
		send_to_char("What direction is that?\n", ch);
		return;
	}

	cha_rnum = ch->in_room;

	if ( !(troom = vtor (strtol(buf2, NULL, 10))) ) {
		send_to_char("No room exists with that number.\n", ch);
		return;
	}

	if ( vtor (ch->in_room)->dir_option[dir] )
		vtor (vtor (ch->in_room)->dir_option[dir]->to_room)->dir_option[rev_dir[dir]] = 0;

	CREATE (vtor (cha_rnum)->dir_option [dir], struct room_direction_data, 1);
	vtor (cha_rnum)->dir_option[dir]->general_description = 0;
	vtor (cha_rnum)->dir_option[dir]->keyword = 0;
	vtor (cha_rnum)->dir_option[dir]->exit_info = 0;
	vtor (cha_rnum)->dir_option[dir]->key = -1;
	vtor (cha_rnum)->dir_option[dir]->to_room = troom->virtual;

	CREATE(troom->dir_option[rev_dir[dir]], struct room_direction_data, 1);
	troom->dir_option[rev_dir[dir]]->general_description = 0;
	troom->dir_option[rev_dir[dir]]->keyword = 0;
	troom->dir_option[rev_dir[dir]]->exit_info = 0;
	troom->dir_option[rev_dir[dir]]->key = -1;
	troom->dir_option[rev_dir[dir]]->to_room = cha_rnum;

	send_to_char("Done.\n", ch);
}

void do_rcret(CHAR_DATA *ch, char *argument, int cmd)
{
	char buf1[80], buf2[80];
	int dir, dif;
	struct secret *r_secret;	

	half_chop(argument,buf1,buf2);
	if(!*buf2) {
		send_to_char("Usage:  rcret <dir> <difficulty>\n",ch);
		return;
	}
	dif=strtol(buf2, NULL, 10);

	switch (*buf1) {
	case 'n': dir = 0; break;
	case 'e': dir = 1; break;
	case 's': dir = 2; break;
	case 'w': dir = 3; break;
	case 'u': dir = 4; break;
	case 'd': dir = 5; break;
	default: dir = -1; break;
	}

	if (dir == -1) {
		send_to_char("What direction is that?\n", ch);
		return;
	}

	if ( vtor (ch->in_room)->secrets[dir]) {
		send_to_char("The old secret description was: \n\n", ch);
		send_to_char(vtor (ch->in_room)->secrets[dir]->stext, ch);
		r_secret=vtor (ch->in_room)->secrets[dir];
	} else {
		CREATE(r_secret,struct secret,1);
	}
	r_secret->diff=dif;
	vtor (ch->in_room)->secrets[dir]=r_secret;

	make_quiet(ch);
	send_to_char("Enter a new secret description.  Terminate with an '@'\n", ch);
	ch->desc->str = &r_secret->stext;
	r_secret->stext=0;
	ch->desc->max_str = 2000;

	send_to_char("Done.\n", ch);
}

void do_rexit(CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf1 [MAX_INPUT_LENGTH];
	char			buf2 [MAX_INPUT_LENGTH];
	int				dir;
    int				cha_rnum;
	ROOM_DATA		*troom;

	half_chop(argument, buf1, buf2);

	if(!strcmp(buf2,"")) {
		send_to_char("No room specified...aborting...\n",ch);
		return;
	}

	switch (*buf1) {
	case 'n': dir = 0; break;
	case 'e': dir = 1; break;
	case 's': dir = 2; break;
	case 'w': dir = 3; break;
	case 'u': dir = 4; break;
	case 'd': dir = 5; break;
	default: dir = -1; break;
	}

	if (dir == -1) {
		send_to_char("What direction is that?\n", ch);
		return;
	}
	cha_rnum = ch->in_room;

	if ( !(troom = vtor (strtol(buf2, NULL, 10))) ) {
		send_to_char("No room exists with that number.\n", ch);
		return;
	}

	if ( !vtor (ch->in_room)->dir_option[dir])
		CREATE (vtor (cha_rnum)->dir_option[dir], struct room_direction_data,1);

	vtor (cha_rnum)->dir_option[dir]->general_description = 0;
	vtor (cha_rnum)->dir_option[dir]->keyword = 0;
	vtor (cha_rnum)->dir_option[dir]->exit_info = 0;
	vtor (cha_rnum)->dir_option[dir]->key = -1;
	vtor (cha_rnum)->dir_option[dir]->to_room = troom->virtual;

	send_to_char("Done.\n", ch);
}

void do_rname(CHAR_DATA *ch, char *argument, int cmd)
{

	for (; isspace(*argument); argument++); /* Get rid of whitespaces */

	vtor (ch->in_room)->name = add_hash(argument);

	send_to_char("Done.\n", ch);
}

void do_rinit(CHAR_DATA *ch, char *argument, int cmd)
{
	int			dir = -1;
	int			virt_nr;
	int			i, j;
	ROOM_DATA	*troom;
	char		arg1 [MAX_INPUT_LENGTH];
	char		buf [MAX_STRING_LENGTH];

	argument = one_argument(argument, arg1);

	if (!*arg1) {
		send_to_char("You must supply a new room number, or 'n' to use the next available slot.\n", ch);
		return;
	}

	virt_nr = strtol(arg1, NULL, 10);

	if (!isdigit(*arg1) && *arg1 != 'n' ) {
		send_to_char("The argument must be a digit, or 'n' to use the next available slot.\n", ch);
		return;
	} else if ( vtor (virt_nr) && *arg1 != 'n' ) {
		send_to_char("That room number already exists.\n",ch);
		return;
	}
	else if ( *arg1 == 'n' ) {
		for ( i = ch->room->zone * 1000; i <= ch->room->zone * 1000 + 999; i++ )
			if ( !vtor(i) )
				break;
		virt_nr = i;
	}

	if (virt_nr > ROOM_MAX) {
		snprintf (buf, MAX_STRING_LENGTH,  "Room numbers cannot exceed %d, You need to recompile "
					  "and increase ROOM_MAX", ROOM_MAX);
		send_to_char(buf,ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf )
		dir = index_lookup (dirs, buf);

	troom = allocate_room (virt_nr);

	troom->virtual = virt_nr;
	troom->contents = NULL;
	troom->people = NULL;
	troom->light = 0; /* Zero light sources */
	troom->zone = virt_nr / ZONE_SIZE;
	troom->name = add_hash ("New Room");
	troom->description = add_hash ("No Description Set.\n");
	troom->ex_description = NULL;
	troom->wdesc = NULL;
	troom->extra = NULL;
	for(j=0;j<6;j++)
		troom->dir_option[j] = 0; 

	if ( dir != -1 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s %d", dirs [dir], troom->virtual);
		do_rlink (ch, buf, 0);
	} else {
		snprintf (buf, MAX_STRING_LENGTH,  "Room %d has been initialized.\n", virt_nr);
		send_to_char(buf, ch);
	}
}

void do_rxchange(CHAR_DATA *ch, char *argument, int cmd)
{
	char arg1 [MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH], *result;
	char *tail, *end;
	int i=0, j=0,k=0;

	half_chop(argument,arg1,arg2);

	if(!(tail=strstr(ch->room->description,arg1))) {
		send_to_char("Old string not found.\n",ch);
		return;
	}
	i=strlen (ch->room->description);
	j=strlen (tail);
	end = (tail+=strlen(arg1));
	k=strlen(end);
	if(!arg2[0]) {
		CREATE(result,char,i+j+k);
		strncpy(result, ch->room->description,(i-j));
		strcat(result,arg2);
		strcat(result,end+1);
	} else {
		CREATE(result,char,i+j+k+1);
		strncpy(result, ch->room->description,(i-j));
		strcat(result,arg2);
		strcat(result,end);
	}

	mem_free (ch->room->description);
	ch->room->description = NULL;
	ch->room->description = add_hash(result);
}

void post_rdesc (DESCRIPTOR_DATA *d)
{
	CHAR_DATA	*ch;
	ROOM_DATA	*room;

	ch = d->character;
	room = vtor (ch->delay_info1);
	ch->delay_info1 = 0;

	if ( !*d->pending_message->message ) {
		send_to_char ("No room description posted.\n", ch);
		return;
	}

	room->description = add_hash(d->pending_message->message);
	d->pending_message = NULL;
}

void do_rdesc(CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH];
	ROOM_DATA	*room;

	room = ch->room;

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "reformat") ) {
		reformat_desc (ch->room->description, &ch->room->description);
		send_to_char (ch->room->description, ch);
		return;
	}

	if ( *buf ) {
		send_to_char ("The only valid parameter to rdesc is 'reformat'.\n",
					  ch);
		return;
	}

	if ( room->description ) {
		send_to_char("The old description was: \n", ch);
		send_to_char(room->description, ch);
	}

	act ("$n begins editing a room description.", FALSE, ch, 0, 0, TO_ROOM);

	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->delay_info1 = room->virtual;

		send_to_char ("Please enter the new description; terminate with an '@'\n\n", ch);
		send_to_char ("1-------10--------20--------30--------40--------50--------60---65\n", ch);	
		make_quiet (ch);
	
	TOGGLE (room->room_flags, ROOM_OK);
	ch->desc->proc = post_rdesc;
}

void do_rappend(CHAR_DATA *ch, char *argument, int cmd)
{
	char *new;

	CREATE(new,char,strlen(vtor (ch->in_room)->description)+1);
	strcpy(new,vtor (ch->in_room)->description);
	vtor (ch->in_room)->description = new;

	if (vtor (ch->in_room)->description) {
		send_to_char("The old description was: \n", ch);
		send_to_char(vtor (ch->in_room)->description, ch);
	}

	send_to_char("\nEnter a your additions.  Terminate with an '@'\n", ch);
	send_to_char ("1-------10--------20--------30--------40--------50--------60---65\n", ch);	
	make_quiet(ch);
	ch->desc->str = &(vtor (ch->in_room)->description);
	ch->desc->max_str = 2000;
}

void do_redesc(CHAR_DATA *ch, char *argument, int cmd)
{
	struct extra_descr_data *tmp = NULL;
	struct extra_descr_data	*newdesc = NULL;
	struct extra_descr_data	*testdesc = NULL;
	struct extra_descr_data	*delete_desc = NULL;
	
	char buf[256];

	argument = one_argument(argument, buf);

	for (; isspace(*argument); argument++); /* Get rid of whitespaces */
	
	if(!*buf) {
		send_to_char("No argument specified....aborting...\n",ch);
		return;
	}
	
	for (tmp = vtor (ch->in_room)->ex_description; tmp; tmp = tmp->next) {
		if (!cmp_strn(tmp->keyword, buf, strlen(buf))) {
			break;
		}
	}
	
	if ( !str_cmp (argument, "reformat") ) {
		reformat_desc (tmp->description, &tmp->description);
		send_to_char (tmp->description, ch);
		return;
	}	
	
	
	if (!tmp) {
		CREATE(newdesc,struct extra_descr_data,1);
		newdesc->description = '\0';
		newdesc->next = vtor (ch->in_room)->ex_description;
		newdesc->keyword = add_hash(buf);
		vtor (ch->in_room)->ex_description = newdesc;
		
	} else {
		send_to_char("The old description was: \n\n", ch);
		send_to_char(vtor (ch->in_room)->ex_description->description, ch);
		newdesc = tmp;
	}

	ch->desc->str = &newdesc->description;

		send_to_char("Enter a new description.  Terminate with an '@'\n", ch);
		send_to_char ("1-------10--------20--------30--------40--------50--------60---65\n", ch);	
		make_quiet(ch);
		newdesc->description = 0;
		ch->desc->max_str = 2000;
	
}

void do_rlinkrm(CHAR_DATA *ch, char *argument, int cmd)
{

	char buf[256];
	int dir;
    int cha_rnum, old_rnum;

	(void)one_argument(argument, buf);

	switch (*buf) {
	case 'n': dir = 0; break;
	case 'e': dir = 1; break;
	case 's': dir = 2; break;
	case 'w': dir = 3; break;
	case 'u': dir = 4; break;
	case 'd': dir = 5; break;
	default: dir = -1; break;
	}

	if (dir == -1) {
		send_to_char("What direction is that?\n", ch);
		return;
	}

	cha_rnum = ch->in_room;
	if (vtor (ch->in_room)->dir_option[dir]) {
		old_rnum = vtor (ch->in_room)->dir_option[dir]->to_room;
	} else {
		send_to_char("There is no exit in that direction.\n", ch);
		return;
	}

	vtor (cha_rnum)->dir_option[dir] = 0;
	vtor (old_rnum)->dir_option[rev_dir[dir]] = 0;
	send_to_char("Done.\n", ch);
}

void do_rexitrm(CHAR_DATA *ch, char *argument, int cmd)
{

	char buf[256];
	int dir;
    int cha_rnum;

	(void)one_argument(argument, buf);

	switch (*buf) {
	case 'n': dir = 0; break;
	case 'e': dir = 1; break;
	case 's': dir = 2; break;
	case 'w': dir = 3; break;
	case 'u': dir = 4; break;
	case 'd': dir = 5; break;
	default: dir = -1; break;
	}

	if (dir == -1) {
		send_to_char("What direction is that?\n", ch);
		return;
	}

	cha_rnum = ch->in_room;

	vtor (cha_rnum)->dir_option[dir] = 0;
	send_to_char("Done.\n", ch);
}

void do_wsave(CHAR_DATA *ch, char *argument, int cmd)
{
	send_to_char ("This command has been disabled. Please use 'zsave all'.\n", ch);
	return;
}

void do_rsector(CHAR_DATA *ch, char *argument, int cmd)
{
	int flag, no=1, i;
	char buf[256], buf2[512], buf3[512];


	(void)one_argument(argument, buf);
	if(!strcmp(buf,"")) {
		snprintf (buf3, MAX_STRING_LENGTH, "Current sector type: %s\n", sector_types[vtor (ch->in_room)->sector_type]);
		send_to_char(buf3,ch);
		return;
	}

	if(buf[0]=='?') {
		snprintf (buf2, MAX_STRING_LENGTH, "The following sector types are available:\n\t\t\t");
		for(i=0; *sector_types[i]!='\n'; i++) {
			snprintf( buf2 + strlen(buf2), MAX_STRING_LENGTH, "%-10s ",sector_types[i]);

			if(!(no%4)) (strcat(buf2,"\n\t\t\t"));
			no++;
		}
		strcat(buf2,"\n");
		send_to_char(buf2,ch);
		return;
	}


	flag = index_lookup (sector_types, buf);
	ch->room->sector_type = flag;

	send_to_char("Done.\n", ch);
}


void do_rflags (CHAR_DATA *ch, char *argument, int cmd)
{
	ROOM_DATA	*room;
	int		flag;
	int		no = 1;
	int		i;
	char	buf [256];
	char	buf2 [512];
	char	buf3 [512];

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		sprintbit (ch->room->room_flags, room_bits, buf2);
		snprintf (buf3, MAX_STRING_LENGTH,  "Current room flags: %s\n", buf2);
		send_to_char (buf3, ch);
		return;
	}

	if ( !str_cmp (buf, "mudwide") && !str_cmp (ch->tname, IMPLEMENTOR_ACCOUNT) ) {
		argument = one_argument (argument, buf2);
		if ( !*buf2 ) {
			send_to_char ("Which room flag did you wish to toggle, MUD-wide?\n", ch);
			return;
		}
		if ( (flag = index_lookup (room_bits, buf2)) == -1 ) {
			send_to_char ("No such room flag or deity.\n", ch);
			return;
		}
		argument = one_argument (argument, buf2);
		if ( str_cmp (buf2, "on") && str_cmp (buf2, "off") ) {
			send_to_char ("Did you wish to toggle that flag ON or OFF?\n", ch);
			return;
		}
		for ( room = full_room_list; room; room = room->lnext ) {
			if ( IS_SET (room->room_flags, (1 << flag)) && !str_cmp (buf2, "off") )
				REMOVE_BIT (room->room_flags, 1 << flag);
			else if ( !IS_SET (room->room_flags, (1 << flag)) && !str_cmp (buf2, "on") )
				SET_BIT (room->room_flags, 1 << flag);
		}
		send_to_char ("Done.\n", ch);
		return;
	}

	if ( buf [0] == '?' ) {
		snprintf (buf2, MAX_STRING_LENGTH, "The following room flags are available:\n\t\t\t");
		for(i=0;*room_bits[i]!='\n';i++) {
			snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH, "%-10s ", room_bits[i]);
			if(!(no%4)) (strcat(buf2,"\n\t\t\t"));
			no++;
		}
		strcat(buf2,"\n");
		send_to_char(buf2,ch);
		send_to_char ("\nYou may also specify a church name too.\n", ch);
		return;
	}

	if ( (i = index_lookup (deity_name, buf)) != -1 ) {
		ch->room->deity = i;
		send_to_char ("Deity set.\n", ch);
		return;
	}

	if ( (flag = index_lookup (room_bits, buf)) == -1 ) {
		send_to_char ("No such room flag or deity.\n", ch);
		return;
	}
	
	if ( !str_cmp(buf, "OOC") && GET_TRUST(ch) != 5 ) {
		send_to_char ("Only a level 5 admin can set the OOC bit.\n", ch);
		return;
	}

	if ( !str_cmp (buf, "psave") ) {
		send_to_char ("This flag cannot be toggled.\n", ch);
		return;
	}

	if ( !IS_SET (ch->room->room_flags, (1 << flag)) )
		SET_BIT (ch->room->room_flags, (1 << flag));
	else
		REMOVE_BIT (ch->room->room_flags, (1 << flag));

	send_to_char("Flag (re)set.\n", ch);
}

void do_rddesc(CHAR_DATA *ch, char *argument, int cmd)
{
	int dir;
	char buf[256];

	(void)one_argument(argument, buf);

	switch (*buf) {
	case 'n': dir = 0; break;
	case 'e': dir = 1; break;
	case 's': dir = 2; break;
	case 'w': dir = 3; break;
	case 'u': dir = 4; break;
	case 'd': dir = 5; break;
	default: dir = -1; break;
	}

	if (dir == -1) {
		send_to_char("What direction is that?\n", ch);
		return;
	}

	if(!vtor (ch->in_room)->dir_option[dir]) {
		send_to_char("No room exists in that direction.\n",ch);
		return;
	}

	if (vtor (ch->in_room)->dir_option[dir]->general_description) {
		send_to_char("The old description was: \n", ch);
		send_to_char(vtor (ch->in_room)->dir_option[dir]->general_description, ch);
	}

	send_to_char("\nEnter a new description.  Terminate with an '@'\n", ch);
	send_to_char ("1-------10--------20--------30--------40--------50--------60---65\n", ch);	
	make_quiet(ch);
	ch->desc->str = &(vtor (ch->in_room)->dir_option[dir]->general_description);
	vtor (ch->in_room)->dir_option[dir]->general_description = 0;
	ch->desc->max_str = 2000;
}

void do_rkey(CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_INPUT_LENGTH];
	int			dir;
	ROOM_DATA	*room;

	argument = one_argument (argument, buf);

	switch ( *buf ) {
		case 'n': dir = 0; break;
		case 'e': dir = 1; break;
		case 's': dir = 2; break;
		case 'w': dir = 3; break;
		case 'u': dir = 4; break;
		case 'd': dir = 5; break;

		default : 
			send_to_char("What direction is that?\n", ch);
			return;
	}

	room = vtor (ch->in_room);

	if ( !room->dir_option [dir] ) {
		send_to_char ("There is no exit in that direction.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !isdigit (*buf) ) {
		send_to_char ("Syntax:  rkey <dir> <key-vnum> [pick-penalty]\n", ch);
		return;
	}

	room->dir_option [dir]->key = strtol(buf, NULL, 10);

	*buf = '\0';

	argument = one_argument (argument, buf);

	if ( isdigit (*buf) )
		room->dir_option [dir]->pick_penalty = strtol(buf, NULL, 10);

	send_to_char("Done.\n", ch);
}

void do_rdflag (CHAR_DATA *ch, char *argument, int cmd)
{
	int					dir;
	int					ind;
	ROOM_DIRECTION_DATA *exit_room;
	char				buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( !*buf || (dir = index_lookup (dirs, buf)) == -1 ) {
		send_to_char ("Use a direction:  North, South, East, or West.\n", ch);
		return;
	}

	if ( !(exit_room = ch->room->dir_option [dir]) ) {
		send_to_char ("There is no exit in that direction.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("What flag would you like to set?  (see tags "
					  "exit-bits)\n", ch);
		return;
	}

	if ( (ind = index_lookup (exit_bits, buf)) == -1 ) {
		send_to_char ("Unknown door flag (see tags exit-bits)\n", ch);
		return;
	}

	TOGGLE (exit_room->exit_info, 1 << ind);

	if ( IS_SET (exit_room->exit_info, 1 << ind) )
		snprintf (buf, MAX_STRING_LENGTH,  "Flag %s set.\n", exit_bits [ind]);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "Flag %s removed.\n", exit_bits [ind]);

	send_to_char (buf, ch);
}

void do_minit (CHAR_DATA *ch, char *argument, int cmd)
{
	char 	arg[80];
	char 	buf [MAX_STRING_LENGTH];
	int 	vnum = 0;
	CHAR_DATA *newmob = NULL;
	int 	i = 0;


	if ( IS_NPC (ch) ) {
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	(void)one_argument(argument, arg);
	if(!arg) {
		send_to_char("You must supply a vnum.\n",ch);
		return;
	}
	if(!isdigit(*arg) && *arg != 'n' ) {
		send_to_char("The vnum must be a number, or 'n' to use the next available slot.\n",ch);
		return;
	}
	if ( *arg == 'n' ) {
		for ( i = ch->room->zone * 1000; i <= ch->room->zone * 1000 + 999; i++ )
			if ( !vtom(i) && i != 0 )
				break;
		vnum = i;
	}
	else vnum=atol(arg);
	
	if((vnum<0) || (vnum>99999)) {
		send_to_char("Vnum must be between 0 and 99999.\n",ch);
		return;
	}

	if ( vtom (vnum) ) {
		send_to_char("That vnum is already in use.\n",ch);
		return;
	}

	newmob = new_char (0);		/* MOB */

	clear_char (newmob);

	newmob->mob->virtual = vnum;
	newmob->mob->zone = vnum / ZONE_SIZE;
	
	ch->pc->edit_mob = vnum;

	newmob->act = 0;
	SET_BIT(newmob->act, ACT_ISNPC);

	newmob->name = add_hash ("mob new");
	newmob->short_descr = add_hash ("a new mobile");

	newmob->speaks = SKILL_SPEAK_WESTRON;
	newmob->skills [SKILL_SPEAK_WESTRON] = 100;
	newmob->max_hit = 80;
	newmob->hit = 80;
	newmob->max_move = 80;
	newmob->move = 80;
	newmob->armor = 0;
	newmob->offense = 0;
	newmob->mob->damroll = 0;

	newmob->str   = mob_start_stat;
	newmob->dex   = mob_start_stat;
	newmob->intel = mob_start_stat;
	newmob->wil   = mob_start_stat;
	newmob->aur   = mob_start_stat;
	newmob->con   = mob_start_stat;
	newmob->agi   = mob_start_stat;
	newmob->tmp_str   = mob_start_stat;
	newmob->tmp_dex   = mob_start_stat;
	newmob->tmp_intel = mob_start_stat;
	newmob->tmp_wil   = mob_start_stat;
	newmob->tmp_aur   = mob_start_stat;
	newmob->tmp_con   = mob_start_stat;
	newmob->tmp_agi   = mob_start_stat;
	newmob->mob->damnodice = 1;
	newmob->mob->damsizedice = 2;
	newmob->intoxication = 0;
	newmob->hunger = -1;
	newmob->thirst = -1;
	newmob->equip = NULL;
	
	open_skill (newmob, SKILL_PARRY);
	open_skill (newmob, SKILL_BLOCK);
	open_skill (newmob, SKILL_DODGE);
	open_skill (newmob, SKILL_BRAWLING);

	fix_offense (newmob);

	newmob->natural_delay	= 24;
	newmob->fight_mode = 2;
	newmob->speed = SPEED_WALK;

	newmob->clans = str_dup ("");		/* Null clan list */

	add_mob_to_hash (newmob);

	act ("$n creates a new mobile.\n", TRUE, ch, 0, 0, TO_ROOM);
	snprintf (buf, MAX_STRING_LENGTH,  "Mobile %d has been initialized.\n", vnum);
	send_to_char(buf, ch);
}

void do_oinit (CHAR_DATA *ch, char *argument, int cmd)
{
	char		arg [MAX_STRING_LENGTH];
	char		arg2 [MAX_STRING_LENGTH];
	char		buf [MAX_STRING_LENGTH];
	int			vnum;
	int			type;
	int			num = 1;
	int		i;
	OBJ_DATA	*newobj;

	arg_splitter (2, argument, arg, arg2);
	if( !arg ) {
		send_to_char ("You must supply a vnum.\n", ch);
		return;
	}

	if ( arg [0] == '?' ) {
		snprintf (buf, MAX_STRING_LENGTH,  "The following item types are available:\n");
		for ( type = 0; *item_types [type] != '\n'; type++) {
			snprintf (ADDBUF, MAX_STRING_LENGTH, "%-20s", item_types [type]);
			if ( !(num % 4) )
				strcat (buf, "\n");
			num++;
		}
		strcat (buf, "\n");
		send_to_char (buf, ch);
		return;
	}

	if ( !isdigit (*arg) && *arg != 'n' ) {
		send_to_char ("The vnum must be a number, or 'n' to use the next available slot.\n", ch);
		return;
	}
	if ( *arg == 'n' ) {
		for ( i = ch->room->zone * 1000; i <= ch->room->zone * 1000 + 999; i++ )
			if ( !vtoo(i) && i != 0 )
				break;
		vnum = i;
	}
	else 
		vnum=atol(arg);
	
	if ( vnum == 0 ) {
		send_to_char ("Object 0 must NEVER be used.\n", ch);
		return;
	}
	
	if ( (vnum < 1) || (vnum > 99999) ) {
		send_to_char ("Vnum must be between 1 and 99999.\n", ch);
		return;
	}

	if ( vnum == VNUM_TICKET ) {
		send_to_char ("That vnum is reserved for ticket use.\n", ch);
		return;
	}

	if ( vtoo (vnum) ) {
		send_to_char ("That vnum is already in use.\n", ch);
		return;
	}
	type = index_lookup (item_types, arg2);

	if(type == -1) {
		send_to_char("Not a valid type.\n",ch);
		return;
	}

	newobj = new_object ();

	clear_object (newobj);

	newobj->virtual = vnum;
	newobj->zone = vnum / ZONE_SIZE;

	add_obj_to_hash (newobj);

	ch->pc->edit_obj = vnum;

	newobj->full_description = 0;
	newobj->contains = 0;
	newobj->in_room = NOWHERE;
	newobj->in_obj = 0;
	newobj->next_content = 0;
	newobj->carried_by = 0;
	newobj->full_description = str_dup ("");
	newobj->obj_flags.type_flag = index_lookup (item_types, arg2);

	switch (type) {
		case ITEM_FLUID:
			newobj->name = add_hash ("water");
			newobj->short_description = add_hash ("filled with water");
			newobj->description = add_hash ("");
			newobj->obj_flags.weight = 100;
			break;

		case ITEM_LIQUID_FUEL:
			newobj->name = add_hash ("oil");
			newobj->short_description = add_hash ("filled with oil");
			newobj->description = add_hash ("");
			newobj->obj_flags.weight = 100;
			break;

		case ITEM_TENT :
			newobj->name = add_hash("tent");
			newobj->short_description = add_hash ("a worn tent");
			newobj->description = add_hash ("an old, worn tent");
			SET_BIT (newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[2] = 1;
			newobj->o.od.value[3] = -1;
			newobj->obj_flags.weight = 1500;
			break;

		case ITEM_DWELLING :
			newobj->name = add_hash("shelter");
			newobj->short_description = add_hash ("a hastily-constructed shelter");
			newobj->description = add_hash ("a very crudely and hastily constructed shelter");
			newobj->o.od.value[2] = 1;
			newobj->o.od.value[3] = -1;
			newobj->obj_flags.weight = 1500;
			break;

		case ITEM_LIGHT :
			newobj->name = add_hash("lantern");
			newobj->short_description = add_hash("a brass lantern");
			newobj->description = add_hash("an old dented lantern");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[2] = 20;
			newobj->obj_flags.weight = 500;
			break;

		case ITEM_WEAPON :
			newobj->name = add_hash("sword");
			newobj->short_description = add_hash("a long sword");
			newobj->description = add_hash("a gleaming long sword");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_WIELD);
			newobj->o.od.value[1] = 2;
			newobj->o.od.value[2] = 4;
			newobj->o.od.value[3] = 3;
			newobj->obj_flags.weight = 700;
			break;

		case ITEM_ARMOR :
			newobj->name = add_hash("jacket");
			newobj->short_description = add_hash("a leather jacket");
			newobj->description = add_hash("a heavy leather jacket");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_WEAR_BODY);
			newobj->o.od.value[0] = 2;
			newobj->obj_flags.weight = 700;
			break;

		case ITEM_KEY :
			 newobj->name = add_hash("key");
			 newobj->short_description = add_hash("a key");
			 newobj->description = add_hash("a small silver key");
 			 SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			 newobj->o.od.value[0] = 0;
			 newobj->obj_flags.weight = 100;
			 break;

		case ITEM_DRINKCON :
			newobj->name = add_hash("skin");
			newobj->short_description = add_hash("a water skin");
			newobj->description = add_hash("a leaky skin lies here.");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 10;
			newobj->o.od.value[1] = 0;
			newobj->o.od.value[2] = ITEM_FLUID;
			newobj->o.od.value[3] = 0;
			newobj->obj_flags.weight = 200;
			break;

		case ITEM_FOOD :
			newobj->name = add_hash("bread");
			newobj->short_description = add_hash("a loaf of bread");
			newobj->description = add_hash("a loaf of bread");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 6;
			newobj->o.od.value[3] = 0;
			newobj->obj_flags.weight = 300;
			break;

		case ITEM_CONTAINER :
			newobj->name = add_hash("bag");
			newobj->short_description = add_hash("a rather large bag");
			newobj->description = add_hash("a large brown bag lies here");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 100;
			newobj->o.od.value[1] = 0;
			newobj->o.od.value[2] = -1;
			newobj->obj_flags.weight = 300;
			break;

		case ITEM_BOARD :
			newobj->name = add_hash("board");
			newobj->short_description = add_hash("a board");
			newobj->description = add_hash("a bulletin board is standing here.");
			newobj->o.od.value[2] = 1;
			newobj->obj_flags.weight = 100;
			break;

		case ITEM_USURY_NOTE :
			newobj->name = add_hash("note usury");
			newobj->short_description = add_hash("a usury note");
			newobj->description = add_hash("A usury note lies on the floor.");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 1;
			newobj->o.od.value[1] = 1;
			newobj->o.od.value[2] = 20;
			newobj->o.od.value[3] = 40;
			newobj->obj_flags.weight = 100;
			break;

		case ITEM_BULLET:
			newobj->name = add_hash ("bullet sphere small crude iron");
			newobj->short_description = add_hash ("a crude iron sling bullet");
			newobj->description = add_hash ("A small sphere of crudely-wrought iron has been left here.");
			SET_BIT (newobj->obj_flags.extra_flags, ITEM_STACK);
			SET_BIT (newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->obj_flags.weight = 10;
			newobj->o.od.value[0] = 1;
			newobj->o.od.value[1] = 4;
			newobj->quality = 30;
			break;

		case ITEM_MISSILE:
			newobj->name = add_hash ("arrow iron-tipped ash-shafted flight");
			newobj->short_description = add_hash ("an iron-tipped, ash-shafted flight arrow");
			newobj->description = add_hash ("An arrow has been left here.");
			SET_BIT (newobj->obj_flags.extra_flags, ITEM_STACK);
			SET_BIT (newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->obj_flags.weight = 10;
			newobj->o.od.value[0] = 1;
			newobj->o.od.value[1] = 6;
			newobj->quality = 30;
			break;

		case ITEM_SHEATH:
			newobj->name = add_hash ("sheath brown leather");
			newobj->short_description = add_hash ("a brown leather sheath");
			newobj->description = add_hash ("A brown leather sheath has been left here.");
			SET_BIT (newobj->obj_flags.wear_flags, ITEM_TAKE);
			SET_BIT (newobj->obj_flags.wear_flags, ITEM_WEAR_BELT);
			newobj->o.od.value[0] = 800;
			newobj->quality = 30;
			newobj->obj_flags.weight = 200;
			break;

		case ITEM_KEYRING:
			newobj->name = add_hash ("keyring sturdy iron");
			newobj->short_description = add_hash ("a sturdy iron keyring");
			newobj->description = add_hash ("A sturdy iron keyring lies here, forgotten.");
			SET_BIT (newobj->obj_flags.wear_flags, ITEM_TAKE);
			SET_BIT (newobj->obj_flags.wear_flags, ITEM_WEAR_BELT);
			newobj->o.od.value[0] = 10;
			newobj->quality = 30;
			newobj->obj_flags.weight = 50;
			break;

		case ITEM_QUIVER:
			newobj->name = add_hash ("quiver brown leather");
			newobj->short_description = add_hash ("a brown leather quiver");
			newobj->description = add_hash ("A brown leather quiver lies here.");
			SET_BIT (newobj->obj_flags.wear_flags, ITEM_TAKE);
			SET_BIT (newobj->obj_flags.wear_flags, ITEM_WEAR_SHOULDER);
			newobj->o.od.value[0] = 25;
			newobj->quality = 30;
			newobj->obj_flags.weight = 200;
			break;

		case ITEM_INK:
			newobj->name = add_hash("ink black pot ceramic small");
			newobj->short_description = add_hash ("a small ceramic pot of black ink");
			newobj->description = add_hash("A small ceramic pot has been left here.");
			newobj->ink_color = add_hash ("black ink");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 10;
			newobj->o.od.value[1] = 10;
			newobj->obj_flags.weight = 200;
			break;

		case ITEM_BOOK:
			newobj->name = add_hash("tome leather-bound small");
			newobj->short_description = add_hash ("a small, leather-bound tome");
			newobj->description = add_hash("A small tome has been left here.");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 25;
			newobj->obj_flags.weight = 500;
			break;

		case ITEM_PARCHMENT:
			newobj->name = add_hash("paper parchment sheet");
			newobj->short_description = add_hash ("a sheet of parchment");
			newobj->description = add_hash("A sheet of parchment has been discarded here.");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 10;
			break;

		case ITEM_WRITING_INST:
			newobj->name = add_hash("quill writing");
			newobj->short_description = add_hash ("a writing quill");
			newobj->description = add_hash("A writing quill has been discarded here.");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 25;
			break;

		case ITEM_HEALER_KIT:
			newobj->name = add_hash("kit healer");
			newobj->short_description = add_hash ("a healer's kit");
			newobj->description = add_hash("A healer's kit lies on the floor.");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 500;
			break;

		case ITEM_REPAIR_KIT:
			newobj->name = add_hash("kit repair mending");
			newobj->short_description = add_hash ("a repair kit");
			newobj->description = add_hash("A repair kit lies on the floor.");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE | ITEM_WEAR_BELT);
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 500;
			break;

		default:
			act ("Generic object created.", FALSE, ch, 0, 0, TO_CHAR);
			newobj->name = add_hash("object generic");
			newobj->short_description = add_hash ("a generic object");
			newobj->description = add_hash("Some careless admin has left a generic object here.");
			SET_BIT(newobj->obj_flags.wear_flags, ITEM_TAKE);
			newobj->o.od.value[0] = 0;
			newobj->obj_flags.weight = 100;
			return;
		break;
	}

	if ( IS_SET (newobj->obj_flags.extra_flags, ITEM_MAGIC) )
		REMOVE_BIT (newobj->obj_flags.extra_flags, ITEM_MAGIC);
	if ( IS_SET (newobj->obj_flags.extra_flags, ITEM_INVISIBLE) )
		REMOVE_BIT (newobj->obj_flags.extra_flags, ITEM_INVISIBLE);

	if ( GET_TRUST (ch) < 5 )
		SET_BIT (newobj->obj_flags.extra_flags, index_lookup (extra_bits, "ok"));

	newobj = load_object (vnum);

	obj_to_char (newobj, ch);
	
	act ("$n does a little creatin'.\n", TRUE, ch, 0, 0, TO_ROOM);
	send_to_char ("Done.\n", ch);
}

void do_rblock(CHAR_DATA *ch, char *argument, int cmd)
{
	ROOM_DATA		*troom;
	char arg1[80], arg2[80], arg3[80];
	int zone, width, height, bfree, used=0;

	*s_buf='\0';
	arg_splitter(3,argument,arg1,arg2,arg3);

	zone=strtol(arg1, NULL, 10);
	width=strtol(arg2, NULL, 10);
	height=strtol(arg3, NULL, 10);

	for ( troom = full_room_list; troom; troom = troom->lnext ) {
		if ( troom->zone == zone)
			used++;
	}
	bfree = (ZONE_SIZE-used);
	snprintf (s_buf, MAX_STRING_LENGTH, "bfree:%d\n",bfree);
	send_to_char(s_buf,ch);
	if(bfree<(width*height)) {
		send_to_char("Not enough free rooms for block.\n",ch);
		return;
	}
}

void do_runused (CHAR_DATA *ch, char *argument, int cmd)
{
	int			unused [ZONE_SIZE];
	int			zone;
	int			line_entry;
	int			i;
	char		buf [MAX_STRING_LENGTH];
	ROOM_DATA	*room;

	argument = one_argument (argument, buf);

	if ( !*buf )
		zone = vtor (ch->in_room)->zone;

	else if ( !just_a_number (buf) ||
			  strtol(buf, NULL, 10) < 0 ||
			  strtol(buf, NULL, 10) >= MAX_ZONE ) {
			send_to_char ("Syntax:  runused [zone #]\n", ch);
			return;
		}

	else
		zone = strtol(buf, NULL, 10);

	for ( i = 0; i < ZONE_SIZE; i++ )
		unused [i] = 0;

	for ( room = full_room_list; room; room = room->lnext )
		if ( room->zone == zone )
			unused [room->virtual % ZONE_SIZE] = 1;

	snprintf (buf, MAX_STRING_LENGTH,  "  ");
	for ( i = 0, line_entry = -2; i < ZONE_SIZE; i++ ) {

		if ( unused [i] )
			continue;

		line_entry = (line_entry + 1) % 12;

		if ( line_entry == 11 )
			strcat (buf, "\n  ");

		snprintf (ADDBUF, MAX_STRING_LENGTH, "%-4d ", i + ZONE_SIZE * zone);
	}

	strcat (buf, "\n");

	page_string (ch->desc, buf);
}

void do_munused(CHAR_DATA *ch,char *argument,int cmd)
{
	int			unused [ZONE_SIZE];
	int			zone;
	int			line_entry;
	int			i;
	char		buf [MAX_STRING_LENGTH];
	CHAR_DATA	*mob;

	argument = one_argument (argument, buf);

	if ( !*buf )
		zone = vtor (ch->in_room)->zone;

	else if ( !just_a_number (buf) ||
			  strtol(buf, NULL, 10) < 0 ||
			  strtol(buf, NULL, 10) >= MAX_ZONE ) {
			send_to_char ("Syntax:  munused [zone #]\n", ch);
			return;
		}

	else
		zone = strtol(buf, NULL, 10);

	for ( i = 0; i < ZONE_SIZE; i++ )
		unused [i] = 0;

	for ( mob = full_mobile_list; mob; mob = mob->mob->lnext )
		if ( mob->mob->virtual / ZONE_SIZE == zone )
			unused [mob->mob->virtual % ZONE_SIZE] = 1;

	snprintf (buf, MAX_STRING_LENGTH,  "  ");
	for ( i = 0, line_entry = -2; i < ZONE_SIZE; i++ ) {

		if ( unused [i] )
			continue;

		line_entry = (line_entry + 1) % 12;

		if ( line_entry == 11 )
			strcat (buf, "\n  ");

		snprintf (ADDBUF, MAX_STRING_LENGTH, "%-4d ", i + ZONE_SIZE * zone);
	}

	strcat (buf, "\n");

	page_string (ch->desc, buf);
}

void do_ounused (CHAR_DATA *ch, char *argument, int cmd)
{
	int			unused [ZONE_SIZE];
	int			zone;
	int			line_entry;
	int			i;
	char		buf [MAX_STRING_LENGTH];
	OBJ_DATA	*obj;

	argument = one_argument (argument, buf);

	if ( !*buf )
		zone = vtor (ch->in_room)->zone;

	else if ( !just_a_number (buf) ||
			  strtol(buf, NULL, 10) < 0 ||
			  strtol(buf, NULL, 10) >= MAX_ZONE ) {
			send_to_char ("Syntax:  runused [zone #]\n", ch);
			return;
		}

	else
		zone = strtol(buf, NULL, 10);

	for ( i = 0; i < ZONE_SIZE; i++ )
		unused [i] = 0;

	for ( obj = full_object_list; obj; obj = obj->lnext )
		if ( obj->zone == zone )
			unused [obj->virtual % ZONE_SIZE] = 1;

	snprintf (buf, MAX_STRING_LENGTH,  "  ");
	for ( i = 0, line_entry = -2; i < ZONE_SIZE; i++ ) {

		if ( unused [i] )
			continue;

		line_entry = (line_entry + 1) % 12;

		if ( line_entry == 11 )
			strcat (buf, "\n  ");

		snprintf (ADDBUF, MAX_STRING_LENGTH, "%-4d ", i + ZONE_SIZE * zone);
	}

	strcat (buf, "\n");

	page_string (ch->desc, buf);
}

int index_lookup (char **index, char *lookup)
{
	int		i;
	int 	lenlook;
	
	if (!*lookup)
		return -1;
	
	lenlook = strlen (lookup);
	
	for ( i = 0; *index [i] != '\n'; i++ ){
        if ( !cmp_strn (index [i], lookup, lenlook))
            return i;
	}
	
	return -1;
}

int skill_index_lookup (char *lookup)
{
	int		jdex;
	int 	lenlook;
	
	if (!*lookup)
		return -1;
	
	lenlook = strlen (lookup);
	
	for ( jdex = 1; *skill_data[jdex].skill_name != '\n'; jdex++ ){
        if ( !cmp_strn (skill_data[jdex].skill_name, lookup, lenlook))
       	    return jdex;	
	}

	return -1;
}

void do_tags (CHAR_DATA *ch, char *argument, int cmd)
{
	char		tag_name [MAX_INPUT_LENGTH];
	char		buf [MAX_STRING_LENGTH];
	int			name_no;
	int			i;


	argument = one_argument (argument, tag_name);

	if ( !*tag_name ) {
		for ( name_no = 0; *constant_info [name_no].constant_name; name_no++ ) {
			snprintf (buf, MAX_STRING_LENGTH,  "\t%-20s    %s\n",
					constant_info [name_no].constant_name,
					constant_info [name_no].description);
			send_to_char (buf, ch);
		}
		return;
	}

	for ( name_no = 0; *constant_info [name_no].constant_name; name_no++ )
		if ( !cmp_strn (constant_info [name_no].constant_name, tag_name,
						strlen (tag_name)) )
			break;

	if ( !str_cmp (tag_name, "drinks") ) {
		act ("Type 'show o fluid' and set OVAL2 on the drink object to the desired fluid's VNUM.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( !strcmp (tag_name, "races") || !strcmp (tag_name, "race")){
		strcpy (buf, "\t  ");
		
		for ( i = 0; i <= MAX_RACE; i++){
			snprintf (ADDBUF, MAX_STRING_LENGTH, "%-15s ",
				db_race_table[i].name);
			
			if ( !((i + 1) % 4) )
				strcat (buf, "\n\t");
		
		}
		strcat (buf, "\n");
		send_to_char (buf, ch);
		return;
	}

	if ( !strcmp (tag_name, "skills") || !strcmp (tag_name, "skill")){
		strcpy (buf, "\t  ");
		
		for ( i = 0; i <= LAST_SKILL; i++){
			snprintf (ADDBUF, MAX_STRING_LENGTH, "%-15s ",
				skill_data[i].skill_name);
			
			if ( !((i + 1) % 4) )
				strcat (buf, "\n\t");
		
		}
		strcat (buf, "\n");
		send_to_char (buf, ch);
		return;
	}
	if ( !*constant_info [name_no].constant_name ) {
		send_to_char ("No such tag name, type 'tags' to get a "
					  "listing.\n", ch);
		return;
	}

	strcpy (buf, "\t");

    for ( i = 0; * (char *) constant_info [name_no].index [i] != '\n'; i++ ) {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "%-15s ",
					(char *) constant_info [name_no].index [i]);
		if ( !((i + 1) % 4) )
			strcat (buf, "\n\t");
	}

	strcat (buf, "\n");
	send_to_char (buf, ch);
	
	return;
}

void do_object (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_INPUT_LENGTH];
	char		buf2 [MAX_INPUT_LENGTH];
	OBJ_DATA	*edit_object;

	if ( IS_NPC (ch) ) {
		send_to_char ("Only PCs can use this command.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		if ( ch->pc->edit_obj && vtoo (ch->pc->edit_obj) ) {
			(void)one_argument (vtoo(ch->pc->edit_obj)->name, buf2);
			snprintf (buf, MAX_STRING_LENGTH,  "object %d", ch->pc->edit_obj);
			do_stat (ch, buf, 0);
		} else
			send_to_char ("You're not editing an object.\n", ch);
		return;
	}
	
    if ( just_a_number (buf) && vtoo (strtol(buf, NULL, 10)) )
    	edit_object = vtoo (strtol(buf, NULL, 10));    
        
        else
        if ((edit_object = get_obj_in_list_vis (ch, buf, ch->right_hand)) ||
        	(edit_object = get_obj_in_list_vis (ch, buf, ch->left_hand)) ||
        	(edit_object = get_obj_in_list_vis (ch, buf, ch->room->contents)))
			/** do nothing in these cases **/;
		else {
				send_to_char ("Couldn't find that object.\n", ch);
				ch->pc->edit_obj = 0;
				return;
		}

	if ( edit_object->virtual == VNUM_TICKET ) {
		send_to_char ("Nobody is allowed to edit tickets.  Sorry.\n", ch);
		ch->pc->edit_obj = 0;
		return;
	}

	ch->pc->edit_obj = edit_object->virtual;

	while ( *argument == ' ' )
		argument++;

	if ( *argument )
		do_oset (ch, argument, 0);

	snprintf (buf, MAX_STRING_LENGTH,  "Editing object %s\n", edit_object->name);
	send_to_char (buf, ch);
}

int redefine_objects (OBJ_DATA *proto)
{
	int					change_count = 0;
	OBJ_DATA			*obj;

	for ( obj = object_list; obj; obj = obj->next ) {

		if ( obj->deleted || obj->virtual != proto->virtual )
			continue;

		if ( GET_ITEM_TYPE (obj) == ITEM_TENT || GET_ITEM_TYPE (obj) == ITEM_DWELLING )
			continue;

		change_count++;

		if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_VARIABLE) )
			obj->name 				= proto->name;
		if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_VARIABLE) )
			obj->short_description	= proto->short_description;
		if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_VARIABLE) )
			obj->description		= proto->description;
		if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_VARIABLE) )
			obj->full_description 	= proto->full_description;

		if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_VARIABLE) )
			obj->obj_flags.extra_flags = proto->obj_flags.extra_flags;

	        if ( IS_SET(obj->obj_flags.extra_flags, ITEM_VARIABLE) )
        	        insert_string_variables (obj, proto, 0);
        	else obj->var_color = 0;

		obj->obj_flags.type_flag   = proto->obj_flags.type_flag;

		obj->obj_flags.wear_flags  = proto->obj_flags.wear_flags;
		
		obj->o.od.value [0]		 = proto->o.od.value [0];

		if ( GET_ITEM_TYPE (obj) != ITEM_KEY )
			obj->o.od.value [1]		 = proto->o.od.value [1];

		obj->o.od.value [2]		 = proto->o.od.value [2];
		obj->o.od.value [3]		 = proto->o.od.value [3];
		obj->o.od.value [4]		 = proto->o.od.value [4];
		obj->o.od.value [5]		 = proto->o.od.value [5];

		obj->quality			 = proto->quality;
		obj->econ_flags			 = proto->econ_flags;

		obj->obj_flags.weight    = proto->obj_flags.weight;
		obj->obj_flags.cost      = proto->obj_flags.cost;

		obj->silver				 = proto->silver;
		obj->coppers			 = proto->coppers;

		obj->activation			 = proto->activation;

		obj->wdesc				 = proto->wdesc;

		if ( !obj->equiped_by )
			obj->size			 = proto->size;

		SET_BIT (obj->obj_flags.extra_flags, ITEM_NEWSKILLS);

/*		obj->xaffected = proto->xaffected;		Don't copy these */
	}

	return change_count;
}

int redefine_mobiles (CHAR_DATA *proto)
{
	int				i;
	int				change_count = 0;
	CHAR_DATA		*mob;

	for ( mob = character_list; mob; mob = mob->next ) {

		if ( mob->deleted ||
			 !IS_NPC (mob) ||
			 mob->mob->virtual != proto->mob->virtual )
			continue;

		change_count++;

		mob->name		  		= proto->name;
		mob->short_descr		= proto->short_descr;
		mob->long_descr			= proto->long_descr;
		mob->description		= proto->description;

		mob->act		  		= proto->act;
		mob->affected_by		= proto->affected_by;
		mob->mob->damnodice		= proto->mob->damnodice;
		mob->mob->damsizedice	= proto->mob->damsizedice;
		mob->position			= proto->position;
		mob->default_pos		= proto->default_pos;
		mob->hmflags			= proto->hmflags;
	
		mob->str				= proto->str;
		mob->dex				= proto->dex;
		mob->intel				= proto->intel;
		mob->aur				= proto->aur;
		mob->con				= proto->con;
		mob->wil				= proto->wil;
		mob->agi				= proto->agi;

		mob->flags				= proto->flags;
		mob->shop				= proto->shop;

		mob->hit				= proto->hit;
		mob->max_hit			= proto->max_hit;
		mob->move				= proto->move;
		mob->max_move			= proto->max_move;
		mob->armor				= proto->armor;
		mob->offense			= proto->offense;
		mob->mob->damroll		= proto->mob->damroll;
		mob->ppoints			= proto->ppoints;
		mob->nat_attack_type	= proto->nat_attack_type;

		mob->sex				= proto->sex;
		mob->clans				= proto->clans;
		mob->deity				= proto->deity;
		
		mob->circle				  = proto->circle;
		mob->mob->skinned_vnum	  = proto->mob->skinned_vnum;
		mob->mob->carcass_vnum    = proto->mob->carcass_vnum;
		mob->mob->merch_seven	  = proto->mob->merch_seven;
		mob->mob->vehicle_type	  = proto->mob->vehicle_type;
		mob->mob->helm_room		  = proto->mob->helm_room;
		mob->natural_delay		  = proto->natural_delay;
		mob->fight_mode			  = proto->fight_mode;
		mob->race				  = proto->race;
		mob->mob->access_flags	  = proto->mob->access_flags;
		mob->speaks				  = proto->speaks;
		mob->poison_type		  = proto->poison_type;

/*
		mob->height				  = proto->height;
		mob->frame				  = proto->frame;
*/
		mob->age				  = proto->age;

		for (i = 0; i < MAX_SKILLS; i++ )
			mob->skills [i] = proto->skills [i];
	
		mob->str				= proto->str;
		mob->dex				= proto->dex;
		mob->con				= proto->con;
		mob->wil				= proto->wil;
		mob->aur				= proto->aur;
		mob->intel				= proto->intel;

		mob->mob->currency_type			= proto->mob->currency_type;

		mob->prog				  = proto->prog;
	}

	return change_count;
}

void give_oset_help (CHAR_DATA *ch)
{
	page_string (ch->desc, "\n"
       "oset [obj_vnum]\n"
	   "     name           \"keywords\"\n"
	   "     sdesc          \"a short description\"\n"
	   "     ldesc          \"the long description\"\n"
           "     desc           \"the look at/examine description\"\n"
	   "     indoor-desc    indoor description for tents and dwellings\n"
	   "\n"
	   "     delete\n"
	   "     'item-types'                        See:  tags item-types\n"
	   "     'wear-bits'                         See:  tags wear-bits\n"
	   "     'apply-types'  <#>                  Add as next affect-see tags\n"
	   "     'extra-bits'                        See:  tags extra-bits\n"
	   "     slot <0..5>    <magic spell>        See:  show v magic\n"
       "     [econ] <econ-flags>                 See:  tags econ-flags\n"
	   "\n"
	   "     activation     <spell-affect>       See:  show v magic\n"
	   "        (use in conjunction with get-affect, etc - see extra-bits)\n"
	   "\n"
	   "     weight         #.#>\n"
	   "     cost           #p ##f                set penny/copper value"
	   "     size           <sz-name>            small, large, etc\n"
	   "     quality        #                    Odds of breaking\n"
	   "     oval           #[#[#[#]]]           All ovalues\n"
	   "     oval1...oval6  #                    Individual oval\n"
	   "     dam            ##d#                  Natural damage attack\n"
	   "     clock          month day hour	     How long until it morphs\n"
	   "     morphto        objnum               What it morphs to\n"
	   "\n"
	   "NOTE:  object affects (apply-types) are not copied from prototype.\n");
}

void post_odesc (DESCRIPTOR_DATA *d)
{
	CHAR_DATA	*ch;
	OBJ_DATA	*obj;

	ch = d->character;
	obj = vtoo (ch->delay_info1);
	ch->delay_info1 = 0;

	if ( !*d->pending_message->message ) {
		send_to_char ("No object description posted.\n", ch);
		return;
	}
	
	if ( ch->delay_info2 )
		obj->indoor_desc = add_hash (d->pending_message->message);
	else obj->full_description = add_hash(d->pending_message->message);

	ch->delay_info2 = 0;

	d->pending_message = NULL;
}

void do_oset (CHAR_DATA *ch, char *argument, int cmd)
{
	char			*tmp_arg;
	OBJ_DATA		*edit_obj;
	int				ind;
	int				i;
	int				weight_int;
	int				weight_dec;
	int				bonus;
	int				parms;
	int				sides;
	int				dice;
	int				mend_type;
	int				full_description = 0, indoor_description = 0;
	AFFECTED_TYPE	*af;
	char			subcmd [MAX_INPUT_LENGTH];
	char			buf	[MAX_STRING_LENGTH];
	TRIGGER_DATA		*temp_trig = NULL;
	

	
	if ( IS_NPC (ch) ) {
		send_to_char ("Not an NPC command.\n", ch);
		return;
	}

	argument = one_argument (argument, subcmd);

	if ( !*subcmd || *subcmd == '?' ) {
		give_oset_help (ch);
		return;
	}
		
	if ( strtol(subcmd, NULL, 10) != 0 && (edit_obj = vtoo (strtol(subcmd, NULL, 10))) ) {
		ch->pc->edit_obj = edit_obj->virtual;
		argument = one_argument (argument, subcmd);
	}

	else if ( strtol(subcmd, NULL, 10) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "No such object vnum %s\n", subcmd);
		send_to_char (buf, ch);
		send_to_char ("Not editing an object now.\n", ch);
		ch->pc->edit_obj = 0;
		return;
	}

	else if ( !(edit_obj = vtoo (ch->pc->edit_obj)) ) {
		send_to_char ("Start by using the OBJECT command.\n", ch);
		return;
	}

	while ( *subcmd ) {

		if ( GET_TRUST (ch) < 5 )
			SET_BIT (edit_obj->obj_flags.extra_flags, ITEM_OK);

		if ( (ind = index_lookup (item_types, subcmd)) != -1 ) {
			edit_obj->obj_flags.type_flag = ind;
			snprintf (buf, MAX_STRING_LENGTH,  "Type %s set.\n", item_types [ind]);
			send_to_char (buf, ch);
		}
		
		else if ( (ind = index_lookup (wear_bits, subcmd)) != -1 ) {
			if ( ind == index_lookup (wear_bits, "Hold") ) {
				send_to_char ("The HOLD wear bit has been deprecated and is no longer used.\n", ch);
				return;
			}
			if ( ind == index_lookup (wear_bits, "Wshield") ) {
				send_to_char ("The WSHIELD wear bit has been deprecated and is no longer used.\n", ch);
				return;
			}
			if ( IS_SET (edit_obj->obj_flags.wear_flags, 1 << ind) )
				REMOVE_BIT (edit_obj->obj_flags.wear_flags, 1 << ind);
			else
				SET_BIT (edit_obj->obj_flags.wear_flags, 1 << ind);
        }

		else if ( !str_cmp (subcmd, "timer") ) {
			argument = one_argument (argument, buf);
			if ( !*buf ) {
				send_to_char ("What did you want to set the object's timer to?\n", ch);
				return;
			}
			if ( !isdigit (*buf) ) {
				send_to_char ("Expected a numeric value for the new timer.\n", ch);
				return;
			}
			edit_obj->obj_timer = strtol(buf, NULL, 10);
			send_to_char ("The object's timer has been set.\n", ch);
			if ( !edit_obj->obj_timer && IS_SET (edit_obj->obj_flags.extra_flags, ITEM_TIMER) )
				REMOVE_BIT (edit_obj->obj_flags.extra_flags, ITEM_TIMER);
			if ( edit_obj->obj_timer && !IS_SET (edit_obj->obj_flags.extra_flags, ITEM_TIMER) )
				SET_BIT (edit_obj->obj_flags.extra_flags, ITEM_TIMER);
			return;
		}

		else if ( (ind = index_lookup (extra_bits, subcmd)) != -1 ) {

			if (((edit_obj->obj_flags.type_flag  != ITEM_ARMOR) &&
				  (edit_obj->obj_flags.type_flag  != ITEM_WORN) &&
				  (!str_cmp (subcmd, "mask"))) ||
				 ((IS_SET(edit_obj->obj_flags.wear_flags,ITEM_WEAR_ABOUT)) &&
				  (IS_SET(edit_obj->obj_flags.wear_flags,ITEM_WEAR_HEAD)) &&
				  (IS_SET(edit_obj->obj_flags.wear_flags,ITEM_WEAR_FACE)) &&
				  (!str_cmp (subcmd, "mask"))))
				  send_to_char ("A mask must be worn HEAD, ABOUT or FACE and must be ARMOR or WORN.\n", ch); 
			else if ((!IS_SET(edit_obj->obj_flags.wear_flags,ITEM_WEAR_HEAD)) &&
				(!IS_SET(edit_obj->obj_flags.wear_flags,ITEM_WEAR_FACE)) &&
				(edit_obj->obj_flags.type_flag == ITEM_ARMOR) &&
				(!str_cmp (subcmd, "mask")))
				send_to_char ("HEAD or FACE armor can be a mask, but not ABOUT armor.\n",ch);
			else if ( IS_SET (edit_obj->obj_flags.extra_flags, 1 << ind) )
				REMOVE_BIT (edit_obj->obj_flags.extra_flags, 1 << ind);
			else
				SET_BIT (edit_obj->obj_flags.extra_flags, 1 << ind);
        }

		else if ( !cmp_strn (subcmd, "affects", strlen(subcmd)) ) {
			argument = one_argument (argument, buf);

			if ( (ind = skill_index_lookup (buf)) == -1 ) {
				send_to_char ("No such skill.\n", ch);
				return;
			}

			argument = one_argument (argument, buf);

			if ( *buf != '0' && !strtol(buf, NULL, 10) ) {
				send_to_char ("Expected a skill value.\n", ch);
				break;
			}

			if ( get_obj_affect_location (edit_obj, ind) )
				remove_obj_affect_location (edit_obj, ind);

			if ( strtol(buf, NULL, 10) ) {
				send_to_char ("Object skill affect set.\n", ch);
				CREATE (af, AFFECTED_TYPE, 1);

				af->type		= 0;
				af->a.spell.duration	= -1;
				af->a.spell.bitvector	= 0;
				af->a.spell.sn			= 0;
				af->a.spell.location	= ind+10000;
				af->a.spell.modifier	= strtol(buf, NULL, 10);
				af->next		= NULL;

				affect_to_obj (edit_obj, af);
			} else {
				send_to_char ("Object skill affect removed, if applicable.\n", ch);
				remove_obj_affect_location (edit_obj, ind+10000);
			}
		}

		else if ( GET_TRUST (ch) >= 4 && !str_cmp (subcmd, "delete") ) {

			if ( edit_obj->deleted ) {
				edit_obj->deleted = 0;
				send_to_char ("This object is no longer marked for deletion."
							  "\n", ch);
				break;
			}

			if ( get_obj_in_list_num (edit_obj->virtual, object_list) ) {
				send_to_char ("Clear this object from the world first.\n",
							  ch);
				return;
			}

			send_to_char ("WARNING:  This object is cleared from the world.  "
						  "However, the prototype\n"
						  "          cannot be removed until the zone is saved,"
						  "and the mud is rebooted.\n", ch);
			edit_obj->deleted = 1;
		}

		else if ( !str_cmp (subcmd, "treats") ) {
			if ( GET_ITEM_TYPE (edit_obj) != ITEM_HEALER_KIT ) {
				send_to_char ("That item is not a healer's kit.\n", ch);
				return;
			}
			if ( !str_cmp (argument, "all") ) {
				if ( !IS_SET (edit_obj->o.od.value[5], TREAT_ALL) ) {
					edit_obj->o.od.value[5] = 0;
					send_to_char ("This healer's kit will now treat all types of wounds.\n", ch);
					return;
				}
				REMOVE_BIT (edit_obj->o.od.value[5], TREAT_ALL);
				send_to_char ("This healer's kit will no longer treat all types of wounds.\n", ch);
				return;
			}
			else if ( !str_cmp (argument, "blunt") ) {
				if ( !IS_SET (edit_obj->o.od.value[5], TREAT_BLUNT) ) {
					SET_BIT (edit_obj->o.od.value[5], TREAT_BLUNT);
					send_to_char ("This healer's kit will now treat blunt wounds.\n", ch);
					return;
				}
				REMOVE_BIT (edit_obj->o.od.value[5], TREAT_BLUNT);
				send_to_char ("This healer's kit will no longer treat blunt wounds.\n", ch);
				return;
			}
			else if ( !str_cmp (argument, "slash") ) {
				if ( !IS_SET (edit_obj->o.od.value[5], TREAT_SLASH) ) {
					SET_BIT (edit_obj->o.od.value[5], TREAT_SLASH);
					send_to_char ("This healer's kit will now treat slash wounds.\n", ch);
					return;
				}
				REMOVE_BIT (edit_obj->o.od.value[5], TREAT_SLASH);
				send_to_char ("This healer's kit will no longer treat slash wounds.\n", ch);
				return;
			}
			else if ( !str_cmp (argument, "puncture") ) {
				if ( !IS_SET (edit_obj->o.od.value[5], TREAT_PUNCTURE) ) {
					SET_BIT (edit_obj->o.od.value[5], TREAT_PUNCTURE);
					send_to_char ("This healer's kit will now treat puncture wounds.\n", ch);
					return;
				}
				REMOVE_BIT (edit_obj->o.od.value[5], TREAT_PUNCTURE);
				send_to_char ("This healer's kit will no longer treat puncture wounds.\n", ch);
				return;
			}
			else if ( !str_cmp (argument, "burn") ) {
				if ( !IS_SET (edit_obj->o.od.value[5], TREAT_BURN) ) {
					SET_BIT (edit_obj->o.od.value[5], TREAT_BURN);
					send_to_char ("This healer's kit will now treat burn wounds.\n", ch);
					return;
				}
				REMOVE_BIT (edit_obj->o.od.value[5], TREAT_BURN);
				send_to_char ("This healer's kit will no longer treat burn wounds.\n", ch);
				return;
			}
			else if ( !str_cmp (argument, "frost") ) {
				if ( !IS_SET (edit_obj->o.od.value[5], TREAT_FROST) ) {
					SET_BIT (edit_obj->o.od.value[5], TREAT_FROST);
					send_to_char ("This healer's kit will now treat frost wounds.\n", ch);
					return;
				}
				REMOVE_BIT (edit_obj->o.od.value[5], TREAT_FROST);
				send_to_char ("This healer's kit will no longer treat frost wounds.\n", ch);
				return;
			}
			else send_to_char ("Please specify all, blunt, puncture, slash, burn or frost.\n", ch);
			return;
		}

		else if ( !str_cmp (subcmd, "mends") ) {
			if ( GET_ITEM_TYPE (edit_obj) != ITEM_REPAIR_KIT ) {
				send_to_char ("That item is not a repair kit.\n", ch);
				return;
			}
			if ( !str_cmp (argument, "all") ) {
				edit_obj->o.od.value[3] = 0;
				edit_obj->o.od.value[5] = 0;
				send_to_char ("This repair kit will now mend all items.\n", ch);
				return;
			}
			else if ( !str_cmp (argument, "none") ) {
				edit_obj->o.od.value[3] = -1;
				edit_obj->o.od.value[5] = -1;
				send_to_char ("This repair kit will no longer mend any items.\n", ch);
				return;
			}
			else if ( ( mend_type = index_lookup (item_types, argument) ) > 0 ) {

				switch (mend_type) {
				case -1: break;
				case ITEM_WEAPON:
				case ITEM_SHIELD:
				case ITEM_ARMOR:
				case ITEM_WORN:
				case ITEM_CONTAINER:
				case ITEM_QUIVER:
				case ITEM_SHEATH:
				case ITEM_DWELLING:
				case ITEM_TENT:
					if ((edit_obj->o.od.value[5] = (edit_obj->o.od.value[5] == mend_type) ? -1 : mend_type) > 0) {
						snprintf (buf, MAX_STRING_LENGTH, "This repair kit will now mend items of type '%s'.\n",argument);
						send_to_char(buf, ch);
					}
					else if (edit_obj->o.od.value[3] < 0) {
						edit_obj->o.od.value[5] = -1;
						send_to_char ("This repair kit will no longer mend any type of objects.\n", ch);
					}
					else {
						edit_obj->o.od.value[5] = 0;
						snprintf (buf, MAX_STRING_LENGTH, "This repair kit will no longer only mend items of type '%s'.\n",argument);
						send_to_char(buf, ch);
					}
					return;
				default:
					snprintf (buf, MAX_STRING_LENGTH, "You may not create kits for mending items of type '%s'.\n",argument);
					send_to_char(buf, ch);
					return;
				}
			}
			else if ( ( mend_type = skill_index_lookup (argument) ) > 0 ) {
				switch (mend_type) {
				case -1: break;
				case SKILL_METALCRAFT:
				case SKILL_WOODCRAFT:
				case SKILL_TEXTILECRAFT:
				case SKILL_HIDEWORKING:
				case SKILL_STONECRAFT:
				case SKILL_DYECRAFT:
				case SKILL_GLASSWORK:
				case SKILL_GEMCRAFT:
				case SKILL_POTTERY:
					if ((edit_obj->o.od.value[3] = (edit_obj->o.od.value[3] == mend_type) ? -1 : mend_type) > 0) {
						snprintf (buf, MAX_STRING_LENGTH, "This repair kit will now mend items created with the skill '%s'.\n",argument);
						send_to_char(buf, ch);
					}
					else if (edit_obj->o.od.value[5] < 0) {
						edit_obj->o.od.value[3] = -1;
						send_to_char ("This repair kit will no longer mend any type of objects.\n", ch);
					}
					else {
						edit_obj->o.od.value[3] = 0;
						snprintf (buf, MAX_STRING_LENGTH, "This repair kit will no longer only mend items created with the skill '%s'.\n",argument);
						send_to_char(buf, ch);
					}
					return;
				default:
					snprintf (buf, MAX_STRING_LENGTH, "You may not use the skill '%s' for repairs.\n",argument);
					send_to_char(buf, ch);
					return;
				}
			}
			send_to_char ("Please specify all or an item type (see tags item-types).\n", ch);
			return;
		}

		else if ( !str_cmp (subcmd, "color") ) {
			argument = one_argument (argument, buf);

			if ( edit_obj->obj_flags.type_flag != ITEM_INK ) {
				send_to_char ("This object is not an ink!\n", ch);
				return;
			}
			else edit_obj->ink_color = add_hash (buf);
			if ( (i = redefine_objects (edit_obj)) > 0 ) {
				snprintf (buf, MAX_STRING_LENGTH,  "%d objects in world redefined.\n", i);
				send_to_char (buf, ch);
			}
			return;
		}

		else if ( !str_cmp (subcmd, "weight") ) {

			argument = one_argument (argument, buf);

			i = sscanf (buf, "%d.%d", &weight_int, &weight_dec);

	/*** fixes the single digit decimal problem ****/		
			if ( weight_dec < 10 && weight_dec >= 0)
				weight_dec = weight_dec * 10;
	/*** end single digit problem fix ***/
				
			if ( i == 0 ) {
				send_to_char ("Expected weight value.\n", ch);
				break;
			}

			if ( i == 2 && (weight_dec < 0 || weight_dec >= 99) ) {
				send_to_char ("Decimal portion of weight not in 0..99\n", ch);
				break;
			}

			if ( i == 1 )
				edit_obj->obj_flags.weight = weight_int * 100;
			else
				edit_obj->obj_flags.weight = weight_int * 100 + weight_dec;
		}

		else if ( !str_cmp (subcmd, "cost") ) {

			argument = one_argument (argument, buf);

			if ( !isdigit (*buf) ) {
				send_to_char ("Expected a cost value.\n", ch);
				return;
			}

			if ( strtol(buf, NULL, 10) < 0 ) {
				send_to_char ("Must be a number greater than -1.\n", ch);
				break;
			}

			sscanf (buf, "%f", &edit_obj->coppers);
			edit_obj->silver = 0;

		}

		else if ( !str_cmp (subcmd, "size") ) {

			argument = one_argument (argument, buf);

			if ( !isdigit (*buf) ) {
				send_to_char ("Expected a size 1 for each 5 inches.\n", ch);
				break;
			}

			edit_obj->size = strtol(buf, NULL, 10);
		}

		else if ( !str_cmp (subcmd, "quality") ) {

			argument = one_argument (argument, buf);

			if ( !isdigit (*buf) ) {
				send_to_char ("Expected quality value.\n", ch);
				break;
			}

			edit_obj->quality = strtol(buf, NULL, 10);
		}

		else if ( !str_cmp (subcmd, "name") ) {

			argument = one_argument (argument, buf);

			if ( !*buf ) {
				send_to_char ("What, no name?\n", ch);
				break;
			}

			edit_obj->name = add_hash (buf);
		}

		else if ( !str_cmp (subcmd, "desc") ||
				  !str_cmp (subcmd, "descr") ||
				  !str_cmp (subcmd, "description") ) {
			(void)one_argument (argument, buf);
			if ( !str_cmp (buf, "reformat") ) {
				argument = one_argument (argument, buf);
				reformat_desc (edit_obj->full_description,
								 &edit_obj->full_description);
				send_to_char (edit_obj->full_description, ch);
			}
			else
				full_description = 1;
		}

		else if ( !str_cmp (subcmd, "indoor-desc") ) {
			(void)one_argument (argument, buf);
			if ( !str_cmp (buf, "reformat") ) {
				argument = one_argument (argument, buf);
				reformat_desc (edit_obj->indoor_desc,
								 &edit_obj->indoor_desc);
				send_to_char (edit_obj->indoor_desc, ch);
			}
			else {
				full_description = 1;
				indoor_description = 1;
			}
		}
		
		else if ( !str_cmp (subcmd, "sdesc") ) {

			argument = one_argument (argument, buf);

			if ( !*buf ) {
				send_to_char ("What, no short description?\n", ch);
				break;
			}

			edit_obj->short_description = add_hash (buf);
		}

		else if ( !str_cmp (subcmd, "ldesc") ) {

			argument = one_argument (argument, buf);

			if ( !*buf ) {
				send_to_char ("What, no long description?\n", ch);
				break;
			}

			edit_obj->description = add_hash (buf);
		}
		
		else if ( !str_cmp (subcmd, "type") ) {
			if ( GET_ITEM_TYPE (edit_obj) != ITEM_WEAPON ) {
				send_to_char ("This is only for weapons.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			if ( (ind = skill_index_lookup (buf)) == -1 ) {
				send_to_char ("Unknown weapon type. It must be the name of the skill that you\nwish the weapon to use, e.g. \"light-edge\".\n", ch);
				return;
			}
			else edit_obj->o.od.value[3] = ind;
		}

		else if ( !str_cmp (subcmd, "oval") || !str_cmp (subcmd, "oval0") ) {
			if ( GET_ITEM_TYPE (edit_obj) == ITEM_PARCHMENT || GET_ITEM_TYPE (edit_obj) == ITEM_TENT || GET_ITEM_TYPE (edit_obj) == ITEM_DWELLING ) {
				send_to_char ("This oval cannot be edited.\n", ch);
				return;
			}
			for (i = 0; i < 6; i++) {
				tmp_arg = one_argument (argument, buf);

				if ( *buf == '0' || strtol(buf, NULL, 10) ) {
					argument = tmp_arg;
					edit_obj->o.od.value [i] = strtol(buf, NULL, 10);
				} else if ( *buf != '-' )
					break;
			}
		}

		else if ( !str_cmp (subcmd, "oval1") ) {
			if ( GET_ITEM_TYPE (edit_obj) == ITEM_BOOK || GET_ITEM_TYPE (edit_obj) == ITEM_KEY ) {
				send_to_char ("This oval cannot be edited.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10))
				edit_obj->o.od.value [1] = strtol(buf, NULL, 10);
			else {
				send_to_char ("Expected oval1 value.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "oval2") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10))
				edit_obj->o.od.value [2] = strtol(buf, NULL, 10);
			else {
				send_to_char ("Expected oval2 value.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "oval3") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10))
				edit_obj->o.od.value [3] = strtol(buf, NULL, 10);
			else {
				send_to_char ("Expected oval3 value.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "oval4") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10))
				edit_obj->o.od.value [4] = strtol(buf, NULL, 10);
			else {
				send_to_char ("Expected oval4 value.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "oval5") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10))
				edit_obj->o.od.value [5] = strtol(buf, NULL, 10);
			else {
				send_to_char ("Expected oval5 value.\n", ch);
				break;
			}
		
		}
		/*		
		else if ( !str_cmp (subcmd, "mdesc") ) {

			argument = one_argument (argument, buf);

			if ( !*buf ) {
				send_to_char ("You must enter a short style description.\n", ch);
				break;
			}
			
			if ( edit_obj->obj_flags.extra_flags != ITEM_MASK ) {
				send_to_char ("You must set the mask flag before entering mask data.\n",ch);
				break;
			}

			if ( edit_obj->obj_flags.type_flag == ITEM_ARMOR ) 
				edit_obj->o.armor.helm_short = add_hash (buf);
			
			if ( edit_obj->obj_flags.type_flag == ITEM_WORN )
				edit_obj->o.cloak.cloak_short = add_hash (buf); 
		} */
		
		else if ( !str_cmp (subcmd, "mkeys") ) {
				    
			argument = one_argument (argument, buf);

			if (!IS_SET(edit_obj->obj_flags.extra_flags, ITEM_MASK)) {
				send_to_char ("Use <oset mask> to set the mask flag before entering keywords.\n",ch);
				break;
			}

			if ( !*buf ) {
				send_to_char ("You must enter a keyword list.\n", ch);
				break;
			}

			send_to_char ("Remember, the first keyword will be used in the wearer's short description.\n",ch);

			if ( edit_obj->obj_flags.type_flag == ITEM_ARMOR ) 
				edit_obj->desc_keys = add_hash (buf);
			
			if ( edit_obj->obj_flags.type_flag == ITEM_WORN )
				edit_obj->desc_keys = add_hash (buf); 
		}

                else if ( !str_cmp (subcmd, "clock") ) {

			int month,day,hour;

                        argument = one_argument (argument, buf);

                        if ( !isdigit (*buf) ) {
                                send_to_char ("Expected a number of months.\n", ch);
                                break;
                        }
			month = strtol(buf, NULL, 10);

                        argument = one_argument (argument, buf);

                        if ( !isdigit (*buf) ) {
                                send_to_char ("Expected a number of days.\n",ch);
                                break;
                        }
                        day = strtol(buf, NULL, 10);

                        argument = one_argument (argument, buf);

                        if ( !isdigit (*buf) ) {
                                send_to_char ("Expected a number of hours.\n",ch);
                                break;
                        }
                        hour = strtol(buf, NULL, 10);

                        edit_obj->clock = ((month*30*24) + (day*24)+ hour);
                }
                else if ( !str_cmp (subcmd, "morphto") ) {

                        argument = one_argument (argument, buf);

                        if ( !isdigit (*buf) ) {
                                send_to_char ("Expected an object number.\n", ch);
                                break;
                        }

                        edit_obj->morphto = strtol(buf, NULL, 10);
                }


		else if ( !str_cmp (subcmd, "dam") ) {

			bonus = 0;

			argument = one_argument (argument, buf);

			parms = sscanf (buf, "%d%[dD]%d%d", &dice, subcmd, &sides, &bonus);

			if ( parms < 3 ) {
				send_to_char ("The dam parameter format is #d#+/-#.\n", ch);
				break;
			}

			edit_obj->o.od.value [1] = dice;
			edit_obj->o.od.value [2] = sides;

			if ( bonus )
				send_to_char ("Dam bonuses not supported yet.\n", ch);
		}

		else if ( (ind = index_lookup (apply_types, subcmd)) != -1 ) {

			argument = one_argument (argument, buf);

			if ( *buf != '0' && !strtol(buf, NULL, 10)) {
				send_to_char ("Expected an affect value.\n", ch);
				break;
			}

			if ( get_obj_affect_location (edit_obj, ind) )
				remove_obj_affect_location (edit_obj, ind);
			else {

				CREATE (af, AFFECTED_TYPE, 1);

				af->type		= 0;
				af->a.spell.duration	= -1;
				af->a.spell.bitvector	= 0;
				af->a.spell.sn			= 0;
				af->a.spell.location	= ind;
				af->a.spell.modifier	= strtol(buf, NULL, 10);
				af->next		= NULL;

				affect_to_obj (edit_obj, af);
			}
		}



		else if ( !str_cmp (subcmd, "slot") ) {

			argument = one_argument (argument, buf);

			if ( !isdigit (*buf) || (i = strtol(buf, NULL, 10)) >= 6 ) {
				send_to_char ("Expected slot # after 'slot'.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			
		}

		else if ( !str_cmp (subcmd, "econ") ) {
			argument = one_argument (argument, buf);
			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("I could not find that econ flag.\n", ch);
				return;
			}
			TOGGLE (edit_obj->econ_flags, 1 << ind);
		}

		else if ( !str_cmp (subcmd, "econ") ) {

			argument = one_argument (argument, buf);

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("No such econ flag.\n", ch);
				break;
			}

			TOGGLE (edit_obj->econ_flags, 1 << ind);
		}

/** Lua Triggers **/
	else if ( !str_cmp (subcmd, "scriptadd") ){	
		
		if ( !*argument ) {
			send_to_char("Correct format is - scriptadd Type Script_name  Function_name.\nTypes are 1-Timed, 2-Enter, 3-Leave, 4-Tell, 5-Ask, 6-Say, 7-Give\n\nType 1 is the only one implemented at this time\n", ch);
		}
		else {	
		
			temp_trig = (TRIGGER_DATA *)alloc ((int)sizeof (TRIGGER_DATA), 24);
			
			sscanf (argument, "%d%s%s", &temp_trig->type, &temp_trig->script, &temp_trig->func);

			argument = one_argument (argument, buf);
			if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 7 ) {
				send_to_char ("Expected type 1..7\n", ch);
				return;
			}
			temp_trig->type = strtol(buf, NULL, 10);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of script\n", ch);
				return;
			}
			temp_trig->script = add_hash(buf);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of function\n", ch);
				return;
			}
			temp_trig->func = add_hash(buf);
			

			temp_trig->source = 1; //0-room, 1-obj, 2-mobile
			temp_trig->me = edit_obj->virtual;
			
			larg_trigger_add(temp_trig, &edit_obj->triggers);
		}
		return;
	}
	else if ( !str_cmp (subcmd, "scriptdel") ){
	
		if ( !*argument ) {
			send_to_char("Correct format is - scriptdel Type Script_name  Function_name.\nTypes are 1-Timed, 2-Enter, 3-Leave, 4-Tell, 5-Ask, 6-Say, 7-Give\n\nType 1 is the only one implemented at this time\n", ch);
			return;
		}
		else {	
			temp_trig = (TRIGGER_DATA *)alloc ((int)sizeof (TRIGGER_DATA), 24);
			
			argument = one_argument (argument, buf);
			if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 7 ) {
				send_to_char ("Expected type 1..7\n", ch);
				return;
			}
			temp_trig->type = strtol(buf, NULL, 10);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of script\n", ch);
				return;
			}
			temp_trig->script = add_hash(buf);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of function\n", ch);
				return;
			}
			temp_trig->func = add_hash(buf);
			
			temp_trig->source = 1; //0-room, 1-obj, 2-mobile
			temp_trig->me = edit_obj->virtual;
			
			larg_mo_script_delete(temp_trig, edit_obj->triggers);
		}
	}
	
	else if ( !str_cmp (subcmd, "scriptlist") ){
		snprintf (buf, MAX_STRING_LENGTH,  "This object has the following scripts:\n\n Type     Script     Function\n");
		send_to_char (buf, ch);
			
		for (temp_trig = edit_obj->triggers; temp_trig; temp_trig = temp_trig->next){
			snprintf (buf, MAX_STRING_LENGTH,  "%d     %s     %s\n", temp_trig->type, temp_trig->script, temp_trig->func);
			send_to_char (buf, ch);
		}
	}
/** end Lua triggers **/
	else {
		snprintf (buf, MAX_STRING_LENGTH,  "Unknown keyword: %s\n", subcmd);
		send_to_char (buf, ch);
		break;
	}

	argument = one_argument (argument, subcmd);
	}

	if ( (i = redefine_objects (edit_obj)) > 0 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%d objects in world redefined.\n", i);
		send_to_char (buf, ch);
	}

	if ( full_description ) {
		CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
		ch->desc->str = &ch->desc->pending_message->message;
		ch->desc->proc = post_odesc;
		ch->delay_info1 = edit_obj->virtual;
		if ( indoor_description )
			ch->delay_info2 = 1;

			send_to_char ("\nOld description:\n\n", ch);
			if ( !indoor_description )
				send_to_char (edit_obj->full_description, ch);
			else send_to_char (edit_obj->indoor_desc, ch);
			ch->desc->max_str = STR_MULTI_LINE;
			send_to_char ("Please enter the new description; terminate with an '@'\n", ch);
			send_to_char ("1-------10--------20--------30--------40--------50--------60---65\n", ch);	
			make_quiet (ch);
		

		return;
	}
}

void post_oset (DESCRIPTOR_DATA *d)
{
	OBJ_DATA	*edit_obj;

	d->proc = NULL;

	if ( !d->character )
		return;

	if ( !(edit_obj = vtoo (d->character->pc->edit_obj)) ) {
		send_to_char ("Sorry, that object disappeared!\n", d->character);
		return;
	}

	redefine_objects (edit_obj);
}

void do_mobile (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_INPUT_LENGTH];
	CHAR_DATA	*edit_mobile;

	if ( IS_NPC (ch) ) {
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( *buf ) {
		if ( ch->pc->edit_player ) {
			unload_pc (ch->pc->edit_player);
			ch->pc->edit_player = NULL;
		}

		ch->pc->edit_mob = 0;
	}

	if ( !*buf ) {
		if ( ch->pc->edit_mob ) {
			snprintf (buf, MAX_STRING_LENGTH,  "mobile %d", ch->pc->edit_mob);
			do_stat (ch, buf, 0);
		} else if ( ch->pc->edit_player ) {
			snprintf (buf, MAX_STRING_LENGTH,  "mobile %s", GET_NAME (ch->pc->edit_player));
			do_stat (ch, buf, 0);
		} else
			send_to_char ("You're not editing a mobile.\n", ch);
		return;
	}
	
	if ( !(edit_mobile = load_pc (buf)) ) {

	    if ( IS_MORTAL(ch) || (!(edit_mobile = vtom (strtol(buf, NULL, 10))) &&
			 !(edit_mobile = get_char_room_vis (ch, buf))) ) {
			send_to_char ("Couldn't find that mobile.\n", ch);
			ch->pc->edit_mob = 0;
			return;
		}

		if ( edit_mobile &&
			 edit_mobile->mob &&
			 edit_mobile != vtom (edit_mobile->mob->virtual) )
			ch->pc->target_mob = edit_mobile;
		else
			ch->pc->target_mob = NULL;

			/* If PC, then only a parital name was specified, do a load_pc */

		if ( !IS_NPC (edit_mobile) ) {
			if ( !(edit_mobile = load_pc (GET_NAME (edit_mobile))) ) {
				send_to_char ("Unable to load online-PC.\n", ch);
				ch->pc->edit_mob = 0;
				return;
			}
		}
	}

	if ( !IS_NPC (edit_mobile) ) {
		
		if ( !str_cmp (ch->pc->account, edit_mobile->pc->account) && port == PLAYER_PORT && edit_mobile != ch ) {
			send_to_char ("Sorry, but you'll need to get another staff member to edit your character.\n"
				      "Editing of one's own PCs is not permitted, for a variety of reasons.\n", ch);
			return;
		}

		if ( IS_MORTAL(ch) && edit_mobile->pc->create_state != STATE_SUBMITTED ) {
			send_to_char ("You may only open PCs in the application queue to edit.\n", ch);
			return;
		}

		ch->pc->edit_mob = 0;
		ch->pc->edit_player = edit_mobile;

		snprintf (buf, MAX_STRING_LENGTH,  "Editing PLAYER %s\n", GET_NAME (edit_mobile));
		send_to_char (buf, ch);
		return;
	}

	if ( IS_SET (edit_mobile->act, ACT_STAYPUT) ) {
		act ("Stayput mobs should NOT be edited. If you need to make a change, edit the prototype on the BP, zsave, swap, purge any stayput instances here and reload them from the database.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	ch->pc->edit_mob = edit_mobile->mob->virtual;

	snprintf (buf, MAX_STRING_LENGTH,  "Editing mobile %s\n", GET_NAMES (edit_mobile));
	send_to_char (buf, ch);
}

void add_replace_mobprog (CHAR_DATA *ch, CHAR_DATA *mob, char *trigger_name)
{
	MOBPROG_DATA	*prog;
	MOBPROG_DATA	*last_prog = NULL;

	mp_dirty = 1;

	for ( prog = mob->prog; prog; prog = prog->next ) {

		last_prog = prog;

		if ( !str_cmp (prog->trigger_name, trigger_name) )
			break;
	}

	if ( !prog ) {
		CREATE (prog, MOBPROG_DATA, 1);

		if ( last_prog )
			last_prog->next = prog;
		else
			mob->prog = prog;

		prog->trigger_name = str_dup (trigger_name);
		prog->next = NULL;

	} else
		mem_free (prog->prog);

	prog->prog = NULL;
	prog->busy = 0;

	send_to_char ("Enter your mob program.  Terminate with @.\n", ch);

	make_quiet (ch);

	ch->desc->str = &prog->prog;
	ch->desc->proc = post_mset;
		ch->desc->max_str = STR_MULTI_LINE;
}

void give_mset_help (CHAR_DATA *ch)
{
	page_string (ch->desc,
   "\n"
   "mset [mob-vnum]                          Optionally specify mob vnum\n"
   "     name           \"keywords\"\n"
   "     sdesc          \"a short name\"\n"
   "     ldesc          \"The long desc\"\n"
   "     desc           Enter a full description\n"
   "\n"
   "         parms that take a simple numberic value:\n"
   "\n"
   "     armor | ac, moves, state (-1..4)\n"
   "     piety, natural, str, dex, int, wil, con, aur, bite,\n"
   "     height, frame, room (pc's only), sleep\n"
   "\n"
   "     access         <room flags>\n"
   "     noaccess       <room flags>\n"
   "     conds          <drunk #> <full #> <thirst #>   U must use all three\n"
   "     circle         <0..9>\n"
   "     dam            <##d##>                Natural damage attack\n"
   "     [fightmode]    <frantic | aggressive | normal | cautious | defensive>\n"
   "     helm           <room-vnum>          Set boat/hitch enter room\n"
   "       boat, hitch                         Specify vehicle type\n"
   "     hp | hits      <hit-points>\n"
   "     skinobj        <obj-vnum>           What corpse skins into\n"
   "     carcobj        <obj-vnum>           What carcass is left after skinning\n"
   "     merch_seven    <0,1>                Whether or not merch has seven econs\n"
   "     attack         <attack-type>        Natural attack type: claw, etc\n"
   "     allskills | noskills                Gives/removes every skills\n"
   "     delete                              Delete a character.\n"
   "\n"
   "     'position-types'                    Type: tags position-types\n"
   "     'deity'                             Type: tags deity\n"
   "     'races'                             Type: tags races\n"
   "     'skills'       <percent>            Type: tags skills\n"
   "     'action-bits'                       Type: tags action-bits\n"
   "     'affected-bits'                     Type: tags affected-bits\n"
   "     dpos 'position-types'               Default position\n"
   "     trudge, paced, walk, jog, run, sprint\n"
   "\n"
   "     keeper                              Associate shop with mob\n"
   "     ostler                              Mob will act as stablemaster\n"
   "     markup <number>                     To sell multiplier\n"
   "     discount <number>                   To buy multiplier\n"
   "     econ_markup[1-7] <number>           To sell econ flagged multiplier\n"
   "     econ_discount[1-7] <number>         To buy econ flagged multiplier\n"
   "     nobuy <econ flag>                   Won't buy flagged item\n"
   "     econ[1-7] <econ flag>               econ_markup/discount flags\n"
   "     shop                                Shop room number\n"
   "     store                               Store room number\n"
   "     trades \"'item-type' ... \"           Types of items traded\n"
   "     delivery \"<ovnum> ...\"              Objects replaced after buy\n"
   "\n");
}

void post_mdesc (DESCRIPTOR_DATA *d)
{
	CHAR_DATA	*ch = NULL;
	CHAR_DATA	*mob = NULL;

	ch = d->character;
	if ( ch->delay_info1 )
		mob = vtom (ch->delay_info1);
	else
		mob = load_pc (ch->delay_who);
		
	ch->delay_info1 = 0;
	
	if ( ch->delay_who )
		mem_free (ch->delay_who);

	if ( !mob ) {
		send_to_char ("NULL mob pointer... aborting...\n", ch);
		return;
	}

	if ( !*d->pending_message->message ) {
		send_to_char ("No mobile description posted.\n", ch);
		return;
	}
	
	mob->description = add_hash(d->pending_message->message);
	d->pending_message = NULL;
	
	return;
}

void update_crafts_file (void)
{
	SUBCRAFT_HEAD_DATA		*craft;
	DEFAULT_ITEM_DATA		*items;
	PHASE_DATA			*phase;
	FILE				*fp;
	int					i, j;
	char				flag [MAX_STRING_LENGTH];
	bool				open_bool = FALSE;
	bool				sector_bool = FALSE;
	bool				race_bool = FALSE;
	bool				season_bool = FALSE;
    char				opens_for [MAX_STRING_LENGTH] = {'\0'};
    char				buf [MAX_STRING_LENGTH] = {'\0'};
    int					clanned = 0;
    int					craft_tot = 0;
    char				buf_2 [MAX_STRING_LENGTH] = {'\0'};

	if ( port != BUILDER_PORT && port != TEST_PORT )
		return;

	mysql_safe_query ("DELETE FROM crafts");

	for ( craft = crafts; craft; craft = craft->next ) {
		craft_tot++;
		for ( i = 0; i <= LAST_PC_RACE; i++ ) {
			if ( craft->race[i] > 0 ){
				race_bool = TRUE;
			}
		}

		if ( craft->clans ) {
			clanned = TRUE;
		}

		for ( i = 0; i <= 24; i++ ) {
			if ( craft->opening[i] > 0 ) {
				open_bool = TRUE;

				if ( !*opens_for ){
					snprintf (opens_for, MAX_STRING_LENGTH, "%s", skill_data[craft->opening[i]].skill_name);
				}
				else{
					snprintf (opens_for + strlen(opens_for), MAX_STRING_LENGTH, ", %s", skill_data[craft->opening[i]].skill_name);
				}
			}
		}

		for ( phase = craft->phases; phase; phase = phase->next ) {
			if ( phase->skill ) {
				if ( !*buf ){
					snprintf (buf, MAX_STRING_LENGTH,  "%s", skill_data[phase->skill].skill_name);
				}
				else {
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s", skill_data[phase->skill].skill_name);
				}
			}
		}

		mysql_safe_query ("INSERT INTO crafts VALUES ('%s', '%s', '%s', %d, '%s', %d, %d, '%s')", 
		craft->craft_name, 
		craft->subcraft_name,
		craft->command,
		open_bool,
		buf,
		clanned,
		race_bool,
		*opens_for ? opens_for : "none");

		open_bool       = FALSE;
		race_bool       = FALSE;
		clanned = 0;
	}

	if ( !(fp = fopen (CRAFTS_FILE, "w+")) ) {
		system_log ("The crafts file could not be opened for writing.", TRUE);
		return;
	}

	fprintf (fp, "[CRAFTS]\n");

	for ( craft = crafts; craft; craft = craft->next ) {

		craft->crafts_start = ftell(fp);
		fprintf (fp, "craft %s subcraft %s command %s\n", 
			craft->craft_name,
			craft->subcraft_name,
			craft->command);

		if ( craft->failure ){
			fprintf (fp, "  fail: %s\n", craft->failure);
		}
		
		if ( craft->failobjs ){
			fprintf (fp, "  failobjs: %s\n", craft->failobjs);
		}

		if ( craft->delay ){
			fprintf (fp, "  ic_delay: %d\n", craft->delay);
		}

/** Opening craft **/
		for ( i = 0; i <= 24; i++ ) {
			if ( craft->opening[i] > 0 ){
				open_bool = TRUE;
			}
		}

		if ( open_bool) {
			fprintf (fp, " opening:");
			for ( i = 0; i <= 24; i++ ) {
				if ( craft->opening[i] ){
					fprintf (fp, " %d", craft->opening[i]);
				}
			}
			fprintf (fp, "\n");
		}

/** Race requirement */		
		for ( i = 0; i <= LAST_PC_RACE; i++ ) {
			if ( craft->race[i] > 0 ){
				race_bool = TRUE;
			}
		}
		
		if (race_bool){
			fprintf (fp, " race:");
			for ( i = 0; i <= LAST_PC_RACE; i++ ) {
				if ( craft->race[i] > 0 ){
					snprintf (buf_2, MAX_STRING_LENGTH, "  %d", craft->race[i]);
					fprintf (fp, buf_2);
				}
			}
			fprintf (fp, "\n");
		}
/** Clan requirenment **/
		if ( craft->clans  ){
			fprintf (fp, " clans: %s\n", craft->clans);
		}

/** Sector requirenment **/
		for ( i = 0; i <= MAX_SECTOR; i++ ){
			if ( craft->sectors[i] ){
				sector_bool= TRUE;
			}
		}

		if ( sector_bool) {
			fprintf (fp, "sectors:");
			for ( i = 0; i <= MAX_SECTOR; i++ ) {
				if ( craft->sectors[i] ){
					fprintf (fp, " %s", sector_types[craft->sectors[i]-1]);
				}
			}
			fprintf (fp, "\n");
		}
/**Season Requirenment **/
		for ( i = 0; i <= 5; i++ ){
			if ( craft->seasons[i] ){
				season_bool = TRUE;
			}
		}

		if ( season_bool) {
			fprintf (fp, "seasons:");
			for ( i = 0; i <= 5; i++ ) {
				if ( craft->seasons[i] ){
					fprintf (fp, " %s", seasons[craft->seasons[i]-1]);
				}
			}
			fprintf (fp, "\n");
		}
/** Phase messages **/
		for ( phase = craft->phases; phase; phase = phase->next ) {
			
			fprintf (fp, "  phase\n");

			if ( phase->failure )
				fprintf (fp, "1stfail:  %s\n", phase->failure);

			if ( phase->second_failure )
				fprintf (fp, "2ndfail:  %s\n", phase->second_failure);

			if ( phase->third_failure )
				fprintf (fp, "3rdfail:  %s\n", phase->third_failure);

			if ( phase->first )
				fprintf (fp, "    1st:  %s\n", phase->first);   

			if ( phase->second )
				fprintf (fp, "    2nd:  %s\n", phase->second);

			if ( phase->third )
				fprintf (fp, "    3rd:  %s\n", phase->third);

			if ( phase->phase_seconds )
				fprintf (fp, "      t:  %d\n", phase->phase_seconds);

			if ( phase->load_mob )
				fprintf (fp, "    mob:  %d\n", phase->load_mob);
 
			if ( phase->skill )
				fprintf (fp, "  skill:  %s vs %dd%d\n", skill_data[phase->skill].skill_name, phase->dice, phase->sides);
				
			if ( phase->attribute > 0 )
				fprintf (fp, "   attr:  %s vs %dd%d\n", attrs[phase->attribute], phase->dice, phase->sides);

			if ( phase->flags ) {
				fprintf (fp, "      f: ");
				if ( IS_SET (phase->flags, 1 << 0) )
					fprintf(fp, " cannot-leave-room");
				if ( IS_SET (phase->flags, 1 << 1) )
					fprintf(fp, " open_on_self");
				if ( IS_SET (phase->flags, 1 << 2) )
					fprintf(fp, " require_on_self");
				if ( IS_SET (phase->flags, 1 << 3) )
					fprintf(fp, " require_greater");
				fprintf (fp, "\n");
			}

			if ( phase->move_cost )
				fprintf (fp, "   cost:  moves %d\n", phase->move_cost);

/** items needed during phase **/

			if (craft->items > 0) {
				for ( i = 1; craft->items[i]; i++ ) {
					items = craft->items[i];
					if ( items->phase != phase )
						continue;
					if ( items->items && items->items[0] ) {
						if ( IS_SET (items->flags, SUBCRAFT_IN_INVENTORY) )
							snprintf (flag, MAX_STRING_LENGTH, "in-inventory");
						else if ( IS_SET (items->flags, SUBCRAFT_HELD) )
							snprintf (flag, MAX_STRING_LENGTH, "held");
						else if ( IS_SET (items->flags, SUBCRAFT_WIELDED) )
							snprintf (flag, MAX_STRING_LENGTH, "wielded");
						else if ( IS_SET (items->flags, SUBCRAFT_USED) )
							snprintf (flag, MAX_STRING_LENGTH, "used");
						else if ( IS_SET (items->flags, SUBCRAFT_PRODUCED) )
							snprintf (flag, MAX_STRING_LENGTH, "produced");
						else if ( IS_SET (items->flags, SUBCRAFT_WORN) )
							snprintf (flag, MAX_STRING_LENGTH, "worn");
						else snprintf (flag, MAX_STRING_LENGTH, "in-room");
						fprintf (fp, "      %d:  (%s", i, flag);
						if ( items->item_counts > 1 )
							fprintf (fp, " x%d", items->item_counts);
						for ( j = 0; j <= MAX_DEFAULT_ITEMS; j++ ) {
							if ( items->items[j] && items->items[j] != items->item_counts ) {
								fprintf (fp, " %d", items->items[j]);
							}
						}
						fprintf (fp, ")\n");
					}
				}
			}
		}

		fprintf (fp, "end");
		craft->crafts_end = ftell(fp);
		fprintf (fp, "\n");
	}

	fprintf (fp, "[end-crafts]\n");
	fclose(fp);
}

void do_cset (CHAR_DATA *ch, char *argument, int cmd)
{
	char			subcmd [MAX_STRING_LENGTH] = {'\0'};
	char			output [MAX_STRING_LENGTH] = {'\0'};
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	int				phasenum = 0;
	int				i = 0;
	int				j = 0;
	int				dice = 0;
	int				sides = 0;
	int				objnum = 0;
	int				index = 0;
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	SUBCRAFT_HEAD_DATA	*tmpcraft = NULL;
	PHASE_DATA			*phase = NULL;
	DEFAULT_ITEM_DATA	*items = NULL;

	if ( IS_NPC(ch) ) {
		send_to_char ("This is a PC-only command.\n", ch);
		return;
	}

	if ( !ch->pc->edit_craft ) {
		send_to_char ("You must first open a craft to edit via the CRAFT command.\n", ch);
		return;
	}

	craft = ch->pc->edit_craft;

	if ( !*argument ) {
		do_crafts (ch, "", 0);
		return;
	}

	argument = one_argument (argument, subcmd);

	if ( !str_cmp (subcmd, "craft") ) {
		argument = one_argument (argument, buf);
		if ( !isalpha (*buf) ) {
			send_to_char ("Craft name contains illegal characters.\n", ch);
			return;
		}
		craft->craft_name = add_hash (buf);
		snprintf (output, MAX_STRING_LENGTH,  "Craft name changed to '%s'.\n", buf);
		send_to_char (output, ch);
	}

	else if ( !str_cmp (subcmd, "subcraft") ) {
		argument = one_argument (argument, buf);
		if ( !isalpha (*buf) ) {
			send_to_char ("Subcraft name contains illegal characters.\n", ch);
			return;
		}
		craft->subcraft_name = add_hash (buf);
		snprintf (output, MAX_STRING_LENGTH,  "Subcraft name changed to '%s'.\n", buf);
		send_to_char (output, ch);
	}

	else if ( !str_cmp (subcmd, "command") ) {
		argument = one_argument (argument, buf);
		if ( !isalpha (*buf) ) {
			send_to_char ("Command name contains illegal characters.\n", ch);
			return;
		}
		craft->command = add_hash (buf);
		snprintf (output, MAX_STRING_LENGTH,  "Craft command changed to '%s'.\n", buf);
		send_to_char (output, ch);
	}

	else if ( !str_cmp (subcmd, "sectors") ) {
		argument = one_argument (argument, buf);
		if ( (index = index_lookup (sector_types, buf)) == -1 ) {
			send_to_char ("That isn't a recognized sector type.\n", ch);
			return;
		}
		for ( j = -1, i = MAX_SECTOR; i >= 0; i-- ) {
			if ( !craft->sectors[i] )
				j = i;
			else if ( craft->sectors[i]-1 == index ) {
				j = -2;
				break;
			}	
		}
		if ( j == -2 ) {
			craft->sectors[i] = 0;
			send_to_char ("Craft no longer requires the specified sector.\n", ch);
			return;
		}
		else if ( i == -1 && j == -1 ) {
			send_to_char ("That craft's list of required sectors is currently full.\n", ch);
			return;
		}
		else if ( i == -1 && j != -1 ) {
			craft->sectors[j] = index + 1;
			send_to_char ("The specified sector has been added to the list of required sector types.\n", ch);
			return;
		}
	}

	else if ( !str_cmp (subcmd, "seasons") ) {
		argument = one_argument (argument, buf);
		if ( (index = index_lookup (seasons, buf)) == -1 ) {
			send_to_char ("That isn't a recognized seasons.\n", ch);
			return;
		}
		for ( j = -1, i = MAX_SEASONS; i >= 0; i-- ) {
			if ( !craft->seasons[i] )
				j = i;
			else if ( craft->seasons[i]-1 == index ) {
				j = -2;
				break;
			}	
		}
		if ( j == -2 ) {
			craft->seasons[i] = 0;
			send_to_char ("Craft no longer requires the specified season.\n", ch);
			return;
		}
		else if ( i == -1 && j == -1 ) {
			send_to_char ("That craft's list of required seasons is currently full.\n", ch);
			return;
		}
		else if ( i == -1 && j != -1 ) {
			craft->seasons[j] = index + 1;
			send_to_char ("The specified season has been added to the list of required seasons.\n", ch);
			return;
		}
	}

	else if ( !str_cmp (subcmd, "opening") ) {
		argument = one_argument (argument, buf);
		if ( (index = skill_index_lookup (buf)) == -1 ) {
			send_to_char ("That isn't a recognized skill name.\n", ch);
			return;
		}
		for ( j = -1, i = 24; i >= 0; i-- ) {
			if ( !craft->opening[i] )
				j = i;
			else if ( craft->opening[i] == index ) {
				j = -2;
				break;
			}	
		}
		if ( j == -2 ) {
			craft->opening[i] = 0;
			send_to_char ("Craft no longer an opening craft for the specified skill.\n", ch);
			return;
		}
		else if ( i == -1 && j == -1 ) {
			send_to_char ("That craft's list of skills it opens for is currently full.\n", ch);
			return;
		}
		else if ( i == -1 && j != -1 ) {
			craft->opening[j] = index;
			send_to_char ("This craft will now open for the specified skill.\n", ch);
			return;
		}
	}

	else if ( !str_cmp (subcmd, "race") ) {
		argument = one_argument (argument, buf);
		index = race_lookup (buf);
		if ( index == -1 ){
			send_to_char ("Unknown mobile race.\n", ch);
			return;
		}
		else if (db_race_table [index].race_point_reqs == 999 ){
			send_to_char ("Only PCs can be set to use crafts.\n", ch);
			return;
		}
		
		for ( j = -1, i = LAST_PC_RACE; i >= 0; i-- ) {
			if ( !craft->race[i] )
				j = i;
			else if ( craft->race[i]-1 == index ) {
				j = -2;
				break;
			}	
		}
		if ( j == -2 ) {
			craft->race[i] = 0;
			send_to_char ("That race is no longer required for this craft.\n", ch);
			return;
		}
		else if ( i == -1 && j == -1 ) {
			send_to_char ("That craft's list of required races is currently full.\n", ch);
			return;
		}
		else if ( i == -1 && j != -1 ) {
			craft->race[j] = index+1;
			send_to_char ("This craft will now require the specified race.\n", ch);
			return;
		}
	}

/**** clans are non fucntional until they can be rewritten ***
	else if ( !cmp_strn (subcmd, "clan", 4) ) {
		argument = one_argument (argument, buf);
		argument = one_argument (argument, buf2);
		if ( !get_clandef (buf) ) {
			send_to_char ("That is not a recognized clan.\n", ch);
			return;
		}
		if ( get_clan_in_string (craft->clans, buf, &flags) ) {
			if ( !*buf2 ) {
				remove_clan_craft (craft, buf, ch);
				send_to_char ("The specified clan requirement has been removed.\n", ch);
				return;
			}
			else if ( !(flags = clan_flags_to_value(buf2)) ) {
				send_to_char ("That is not a recognized clan rank.\n", ch);
				return;
			}
			else {
				add_clan_craft (craft, buf, flags);	
				
				send_to_char ("The rank requirement for the specified clan has been updated.\n", ch);
				return;
			}
		}
		else {
			if ( !*buf2 )
				snprintf (buf2, MAX_STRING_LENGTH,  "member");
			if ( *buf2 &&
				!(flags = clan_ranks_to_value (craft->clans->clan_name, buf2))) {
				send_to_char ("That is not a recognized clan rank.\n", ch);
				return;
			}
			if ( flags ) {
				add_clan_craft (craft, buf, flags);	
				
			} else {
				add_clan_craft (craft, buf, CLAN_RANK_ZERO);
			}
			send_to_char ("The specified clan and rank have been added as requirements.\n", ch);
			return;
		}
	}
*****************************************/
	else if ( !str_cmp (subcmd, "delete") ) {
		argument = one_argument (argument, buf);
		if ( *buf != '!' ) {
			send_to_char ("If you are SURE you'd like to delete this craft, please use 'cset delete !'.\n", ch);
			return;
		}
		mysql_safe_query ("DELETE FROM new_crafts WHERE subcraft = '%s' AND craft = '%s'", craft->subcraft_name, craft->craft_name);
		snprintf (output, MAX_STRING_LENGTH,  "The %s craft has been deleted.\n", craft->subcraft_name);
		if ( crafts == craft )
			crafts = crafts->next;
		else {
			for ( tmpcraft = crafts; tmpcraft; tmpcraft = tmpcraft->next )
				if ( tmpcraft->next == craft) tmpcraft->next = tmpcraft->next->next;
		}
		send_to_char (output, ch);
		ch->pc->edit_craft = NULL;
		return;
	}

	else if ( !str_cmp (subcmd, "failure") ) {
		craft->failure = add_hash (argument);
		snprintf (output, MAX_STRING_LENGTH,  "Failure string modified.\n");
		send_to_char (output, ch);
	}

	else if ( !cmp_strn (subcmd, "failobjs", 6) ) {
		if ( !*argument ) {
			send_to_char ("What object(s) did you wish to set?\n", ch);
			return;
		}
		if ( !isdigit (*argument) ) {
			send_to_char ("You must specify a list of VNUMs, e.g. 'VNUM1 VNUM2 VNUM3'.\n", ch);
			return;
		}
		if ( strtol(argument, NULL, 10) == 0 ) {
			craft->failobjs = NULL;
			send_to_char ("Failure object list deleted.\n", ch);
			return;
		}
		craft->failobjs = add_hash (argument);
		send_to_char ("Failure object list inserted.\n", ch);
		return;	
	}

	else if ( !cmp_strn (subcmd, "delay", 5) ) {
		if ( !*argument ) {
			send_to_char ("How many OOC hours did you want to set the delay timer to?\n", ch);
			return;
		}
		if ( !isdigit (*argument) ) {
			send_to_char ("You must specify a number of RL hours to set the timer to.\n", ch);
			return;
		}
		craft->delay = strtol(argument, NULL, 10);
		send_to_char ("The craft's OOC delay timer has been set.\n", ch);
		return;	
	}

	else if ( !cmp_strn (subcmd, "phase", 5) || !cmp_strn (subcmd, "phsae", 5) ) {
		if ( !isdigit (subcmd[5]) ) {
			send_to_char ("Please specify a phase number, e.g. 'cset phase2'.\n", ch);
			return;
		}
		snprintf (buf, MAX_STRING_LENGTH,  "%c", subcmd[5]);
		if ( isdigit (subcmd[6]) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%c", subcmd[6]);
		phasenum = strtol(buf, NULL, 10);

		if ( !craft->phases ) {
			CREATE (craft->phases, PHASE_DATA, 1);
			craft->phases = new_phase();
			phase = craft->phases;
		}
		else for ( phase = craft->phases, i = 1; phase; phase = phase->next, i++ ) {
			if ( !phase->next && phasenum == i + 1 ) {
				CREATE (phase->next, PHASE_DATA, 1);
				phase->next = new_phase();
			}
			else if ( i == phasenum )
				break;
			else if ( !phase->next && i++ < phasenum ) {
				snprintf (output, MAX_STRING_LENGTH,  "\nIf you want to add a new phase, please use the\nnext available slot; in this case, %d.\n", i);
				send_to_char (output, ch);
				return;
			}
		}
		argument = one_argument (argument, subcmd);
		if ( !*subcmd ) {
			send_to_char ("What would you like to edit in this phase?\n", ch);
			return;
		}

		if ( !str_cmp (subcmd, "1st") ) {
			argument = one_argument (argument, buf);
			phase->first = add_hash (buf);
			snprintf (output, MAX_STRING_LENGTH,  "The first person echo for phase %d has been modified.\n", phasenum);
			send_to_char (output, ch);
		}
		else if ( !str_cmp (subcmd, "2nd") ) {
			argument = one_argument (argument, buf);
			phase->second = add_hash (buf);
			snprintf (output, MAX_STRING_LENGTH,  "The second person echo for phase %d has been modified.\n", phasenum);
			send_to_char (output, ch);
		}
		else if ( !str_cmp (subcmd, "3rd") ) {
			argument = one_argument (argument, buf);
			phase->third = add_hash (buf);
			snprintf (output, MAX_STRING_LENGTH,  "The third person echo for phase %d has been modified.\n", phasenum);
			send_to_char (output, ch);
		}
		else if ( !str_cmp (subcmd, "failure-1st") ) {
			argument = one_argument (argument, buf);
			phase->failure = add_hash (buf);
			snprintf (output, MAX_STRING_LENGTH,  "The first-person failure echo for phase %d has been modified.\n", phasenum);
			send_to_char (output, ch);
		}
		else if ( !str_cmp (subcmd, "failure-2nd") ) {
			argument = one_argument (argument, buf);
			phase->second_failure = add_hash (buf);
			snprintf (output, MAX_STRING_LENGTH,  "The second-person failure echo for phase %d has been modified.\n", phasenum);
			send_to_char (output, ch);
		}
		else if ( !str_cmp (subcmd, "failure-3rd") ) {
			argument = one_argument (argument, buf);
			phase->third_failure = add_hash (buf);
			snprintf (output, MAX_STRING_LENGTH,  "The third-person failure echo for phase %d has been modified.\n", phasenum);
			send_to_char (output, ch);
		}
		else if ( !str_cmp (subcmd, "flag") ) {
			argument = one_argument(argument, buf);
			if ( (j = index_lookup (phase_flags, buf)) == -1 ) {					
				snprintf (output, MAX_STRING_LENGTH,  "The phase flag \'%s\' does not exist.\n", buf);
				send_to_char (output, ch);
				return;
			}
			else {
				if ( IS_SET (phase->flags, 1 << j) ) {
					REMOVE_BIT (phase->flags, 1 << j);
					snprintf (output, MAX_STRING_LENGTH,  "The \'%s\' phase flag has been removed.\n", buf);
					send_to_char (output, ch);
				}
				else {
					SET_BIT (phase->flags, 1 << j );
					snprintf (output, MAX_STRING_LENGTH,  "The \"%s\" phase flag has been added.\n", buf);
					send_to_char (output, ch);
				}
			}
		}
		else if ( !cmp_strn (subcmd, "mob", 7) ) {
			if ( !*argument ) {
				send_to_char ("Which mob did you want to load on success in this phase?\n", ch);
				return;
			}
			if ( !isdigit (*argument) ) {
				send_to_char ("You'll need to specify a mob's VNUM.\n", ch);
				return;
			}
			if ( strtol(argument, NULL, 10) == 0 )
				send_to_char ("Loaded mobile removed from the specified phase.\n", ch);
			else if ( !vtom(strtol(argument, NULL, 10)) ) {
				send_to_char ("The mob you specified doesn't exist in the database.\n", ch);
				return;
			}
			else send_to_char ("The specified mobile will now load on success in that phase.\n", ch);
			phase->load_mob = strtol(argument, NULL, 10);
		}
		else if ( !str_cmp (subcmd, "delete") ) {
			if ( phasenum == 1 ) {
				send_to_char ("If you want to delete the craft, use 'cset delete'.\n", ch);
				return;
			}
			else for ( phase = craft->phases, i = 1; phase; phase = phase->next, i++ )
				if ( phasenum == i + 1 )
					phase->next = phase->next->next;
			snprintf (output, MAX_STRING_LENGTH,  "Phase %d has been deleted.\n", phasenum);
			send_to_char (output, ch);
		}
		else if ( !str_cmp (subcmd, "time") ) {
			argument = one_argument(argument, buf);
			if ( !isdigit (*buf) ) {
				send_to_char ("The specified value for 'time' must be an integer.\n", ch);
				return;
			}
			if ( strtol(buf, NULL, 10) < 1 ) {
				send_to_char ("The specified value for 'time' must be greater than one.\n", ch);
				return;
			}
			phase->phase_seconds = strtol(buf, NULL, 10);
			snprintf (output, MAX_STRING_LENGTH,  "The time delay for phase %d has been set to %d seconds.\n", phasenum, (int)strtol(buf, NULL, 10));
			send_to_char (output, ch);
		}

		else if ( !str_cmp (subcmd, "check") ) {
			argument = one_argument (argument, buf);
			if ( !isdigit (*buf) ) {
				send_to_char ("The specified 'check' value must be in the format XdY, e.g. 2d35.\n", ch);
				return;
			}
			if ( !phase->skill ) {
				send_to_char ("You must first specify a skill to check against for this phase.\n", ch);
				return;
			}
			sscanf (buf, "%dd%d", &dice, &sides);
			if ( dice < 1 || sides < 0 ) {
				send_to_char ("Both values specified must be greater than zero.\n", ch);
				return;
			}
			phase->dice = dice;
			phase->sides = sides;
			snprintf (output, MAX_STRING_LENGTH,  "The phase's skill check has been changed to %dd%d.\n", dice, sides);
			send_to_char (output, ch);
		}
		
		else if ( !str_cmp (subcmd, "checkattr") ) {
			argument = one_argument (argument, buf);
			if ( !isdigit (*buf) ) {
				send_to_char ("The specified 'check' value must be in the format XdY, e.g. 2d35.\n", ch);
				return;
			}
			if ( phase->attribute == -1 ) {
				send_to_char ("You must first specify an attribute to test against for this phase.\n", ch);
				return;
			}
			sscanf (buf, "%dd%d", &dice, &sides);
			if ( dice < 1 || sides < 0 ) {
				send_to_char ("Both values specified must be greater than zero.\n", ch);
				return;
			}
			phase->dice = dice;
			phase->sides = sides;
			snprintf (output, MAX_STRING_LENGTH,  "The phase's attribute check has been changed to %dd%d.\n", dice, sides);
			send_to_char (output, ch);
		}
		
		else if ( !str_cmp (subcmd, "movecost") ) {
			argument = one_argument (argument, buf);
			if ( !isdigit (*buf) ) {
				send_to_char ("Expected a numeric value.\n", ch);
				return;
			}
			else phase->move_cost = strtol(buf, NULL, 10);
			send_to_char ("The phase's movement cost has been modified.\n", ch);
		}
		else if ( !str_cmp (subcmd, "skill") ) {
			argument = one_argument (argument, buf);
			if ( isdigit(*buf) && strtol(buf, NULL, 10) != 0 ) {
				send_to_char ("The specified 'skill' value must be a skill name.\n", ch);
				return;
			}
			if ( !str_cmp (buf, "none") ) {
				phase->skill = 0;
				send_to_char ("The phase's skill check has been removed.\n", ch);
				return;
			}
			if ( (i = skill_index_lookup (buf)) == -1 ) {
				snprintf (output, MAX_STRING_LENGTH,  "I could not find the '%s' skill in our registry.\n", buf);
				send_to_char (output, ch);
				return;
			}
			phase->skill = i;
			for ( i = 0; i <= strlen(buf); i++ )
				buf[i] = toupper(buf[i]);
			snprintf (output, MAX_STRING_LENGTH,  "Phase %d will now check against the %s skill.\n", phasenum, buf);
			send_to_char (output, ch);
		}
		
		else if ( !str_cmp (subcmd, "attribute") ) {
			argument = one_argument (argument, buf);
			if ( isdigit(*buf) && strtol(buf, NULL, 10) != 0 ) {
				send_to_char ("The specified 'attribute' value must be an attribute name.\n", ch);
				return;
			}
			if ( !str_cmp (buf, "none") ) {
				phase->attribute = 0;
				send_to_char ("The phase's attribute test has been removed.\n", ch);
				return;
			}
			if ( (i = index_lookup (attrs, buf)) == -1 ) {
				snprintf (output, MAX_STRING_LENGTH,  "I could not find the '%s' attribute in our files.\n", buf);
				send_to_char (output, ch);
				return;
			}
			phase->attribute = index_lookup (attrs, buf);
			for ( i = 0; i <= strlen(buf); i++ )
				buf[i] = toupper(buf[i]);
			snprintf (output, MAX_STRING_LENGTH,  "Phase %d will now test  against the %s attribute of %d.\n", phasenum, buf, phase->attribute);
			send_to_char (output, ch);
		}
		
		
		else if ( !cmp_strn (subcmd, "object", 6) ) {
			if ( !isdigit (subcmd[6]) ) {
				send_to_char ("An object number must be specified, e.g. object6.\n", ch);
				return;
			}
			snprintf (buf, MAX_STRING_LENGTH,  "%c", subcmd[6]);
			if ( isdigit(subcmd[7]) )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%c", subcmd[7]);
			objnum = strtol(buf, NULL, 10);
			if ( objnum < 1 ) {
				send_to_char ("The specified object set must be greater than 1, e.g. object6.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			if ( !*buf ) {
				send_to_char ("What object did you wish to set?\n", ch);
				return;
			}
			if ( !craft->items[0] )
				CREATE (craft->items[0], DEFAULT_ITEM_DATA, 1);
			for ( i = 0; i <= MAX_DEFAULT_ITEMS; i++ ) {
				items = craft->items[i];
				if ( i == objnum )
					break;
				if ( !craft->items [i+1] ) {
					if ( i+1 != objnum ) {
						snprintf (output, MAX_STRING_LENGTH,  "Please use the next available slot to add a new\nobject set; in this case, object%d.\n", i+1);
						send_to_char (output, ch);
						return;
						}
					else CREATE (craft->items [i+1], DEFAULT_ITEM_DATA, 1);
				}
			}
			memset (items->items, 0, MAX_DEFAULT_ITEMS);
			items->item_counts = 0;
			if ( *buf == 'X' || *buf == 'x' ) {
				*buf2 = '\0';
				for ( j = 1; j <= strlen(buf); j++ )
					snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%c", buf[j]);
				items->item_counts = strtol(buf2, NULL, 10);
			}
			else if ( isdigit (*buf) ) {
				for ( j = 0; j <= MAX_DEFAULT_ITEMS; j++ ) {
					if ( !items->items[j] ) {
						if ( !vtoo(strtol(buf, NULL, 10)) && strtol(buf, NULL, 10) != 0 ) {
							snprintf (output, MAX_STRING_LENGTH,  "Sorry, but object VNUM %ld could not be loaded for inclusion.\n", strtol(buf, NULL, 10));
							send_to_char (output, ch);
							break;
						}
						if ( (int)strtol(buf, NULL, 10) == 0 ) {
							snprintf (output, MAX_STRING_LENGTH,  "Object set %d has been removed.\n", objnum);
							send_to_char (output, ch);
							for ( j = 0; j <= MAX_DEFAULT_ITEMS; j++ )
								items->items[j] = 0;
							break;
						}
						snprintf (output, MAX_STRING_LENGTH,  "#2%s#0 has been added to object set %d.\n", vtoo(strtol(buf, NULL, 10))->short_description, objnum);
						output[2] = toupper(output[2]);
						send_to_char (output, ch);
						items->items[j] = strtol(buf, NULL, 10);
						break;
					}
				}
			}
			while ( (argument = one_argument(argument, buf)) ) {
				if ( !isdigit (*buf) )
					break;
				for ( j = 0; j <= MAX_DEFAULT_ITEMS; j++ ) {
					if ( !items->items[j] ) {
						if ( !vtoo(strtol(buf, NULL, 10)) && strtol(buf, NULL, 10) != 0 ) {
							snprintf (output, MAX_STRING_LENGTH,  "Sorry, but object VNUM %d could not be loaded for inclusion.\n", (int)strtol(buf, NULL, 10));
							send_to_char (output, ch);
							break;
						}
						if ( strtol(buf, NULL, 10) == 0 ) {
							snprintf (output, MAX_STRING_LENGTH,  "Object set %d has been removed.\n", objnum);
							send_to_char (output, ch);
							items->items[j] = 0;
							break;
						}
						snprintf (output, MAX_STRING_LENGTH,  "#2%s#0 has been added to object set %d.\n", vtoo(strtol(buf, NULL, 10))->short_description, objnum);
						output[2] = toupper(output[2]);
						send_to_char (output, ch);
						items->items[j] = strtol(buf, NULL, 10);
						break;
					}
				}
			}

			if ( !isdigit (*buf) && *buf ) {
				if ( (j = index_lookup (item_default_flags, buf)) == -1 ) {					
					snprintf (output, MAX_STRING_LENGTH,  "The item flag '%s' does not exist.\n", buf);
					send_to_char (output, ch);
					return;
				}
				else {
					items->flags = 0;
					SET_BIT (items->flags, 1 << j );
					if ( !str_cmp ("in-inventory", buf) ) {
						snprintf (output, MAX_STRING_LENGTH,  "The in-inventory flag has been depecrated; set to 'held' instead.\n");
						SET_BIT (items->flags, 1 << 2);
					}
					else snprintf (output, MAX_STRING_LENGTH,  "The item set has been flagged as '%s'.\n", item_default_flags[j]);
					send_to_char (output, ch);
				}
			}
			items->phase = phase;
		}

		else {
			send_to_char ("I'm afraid that isn't a recognized command.\n", ch);
			return;
		}
	}
	else {
		send_to_char ("I'm afraid that isn't a recognized command.\n", ch);
		return;
	}
}

void do_mset (CHAR_DATA *ch, char *argument, int cmd)
{
	char		subcmd [MAX_INPUT_LENGTH] = { '\0' };
	char		buf	[MAX_STRING_LENGTH] = { '\0' };
	char		buf2 [MAX_STRING_LENGTH] = { '\0' };
	char		trigger_name [MAX_INPUT_LENGTH] = { '\0' };
	char		type [MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA	*edit_mob = NULL;
	CHAR_DATA	*tch = NULL;
	WOUND_DATA	*wound = NULL;
	WOUND_DATA	*tempwound = NULL;
	ACCOUNT_DATA	*account = NULL;
	AFFECTED_TYPE	*af = NULL;
	int			i = 0;
	int			j = 0;
	int			delta = 0;
	int			ind = 0;
	int			full_description = 0;
	int			dice = 0;
	int			sides = 0;
	int			bonus = 0;
	int			parms = 0;
	int			error = 0;
	int			damage = 0;
	int			typenum = 0;
	char		c = '\0';
	char		*p = NULL;
	bool		builder_only = FALSE;
	char		*temp_arg = NULL;
	TRIGGER_DATA		*temp_trig = NULL;


/** initial checks and help file**/
	if ( IS_NPC (ch) ) {
		send_to_char ("This is a PC-only command.\n", ch);
		return;
	}

	argument = one_argument (argument, subcmd);

	if ( IS_MORTAL(ch) && str_cmp(subcmd, "sdesc") && str_cmp(subcmd, "ldesc") && str_cmp(subcmd, "name") && str_cmp(subcmd, "desc") ) {
		send_to_char ("Uses: MSET NAME, MSET SDESC, MSET LDESC, MSET DESC or MSET DESC REFORMAT.\n", ch);
		return;
	}

	if ( !*subcmd || *subcmd == '?' ) {
		give_mset_help (ch);
		return;
	}

/** Check VNUM **/

	if ( isdigit (*subcmd) && (edit_mob = vtom (strtol(subcmd, NULL, 10))) ) {
		ch->pc->edit_mob = edit_mob->mob->virtual;
		argument = one_argument (argument, subcmd);
	}

	else if ( isdigit (*subcmd) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "No such mobile vnum %s\n", subcmd);
		send_to_char (buf, ch);
		send_to_char ("Not editing a mobile now.\n", ch);
		ch->pc->edit_mob = 0;
		return;
	}

	else if ( !(edit_mob = vtom (ch->pc->edit_mob)) &&
			  !(edit_mob = ch->pc->edit_player) ) {
		send_to_char ("Start by using the MOBILE command.\n", ch);
		return;
	}

	if ( ch->pc->edit_player )
		edit_mob = ch->pc->edit_player;

/** Start setting variables **/
	while ( *subcmd ) {

/** Variable mob? warning **/
		if ( IS_SET(edit_mob->flags, FLAG_VARIABLE) && str_cmp (subcmd, "variable") ) {
			send_to_char ("Don't mset this mob, unless you're removing the variable flag.\n", ch);
			return;
		}

/** Lua Triggers **/
	else if ( !str_cmp (subcmd, "scriptadd") ){	
		
		if ( !*argument ) {
			send_to_char("Correct format is - scriptadd Type Script_name  Function_name.\nTypes are 1-Timed, 2-Enter, 3-Leave, 4-Tell, 5-Ask, 6-Say, 7-Give\n\nType 1 is the only one implemented at this time\n", ch);
		}
		else {	
		
			temp_trig = (TRIGGER_DATA *)alloc ((int)sizeof (TRIGGER_DATA), 24);
			
			sscanf (argument, "%d%s%s", &temp_trig->type, &temp_trig->script, &temp_trig->func);

			argument = one_argument (argument, buf);
			if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 7 ) {
				send_to_char ("Expected type 1..7\n", ch);
				return;
			}
			temp_trig->type = strtol(buf, NULL, 10);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of script\n", ch);
				return;
			}
			temp_trig->script = add_hash(buf);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of function\n", ch);
				return;
			}
			temp_trig->func = add_hash(buf);
			

			temp_trig->source = 2; //0-room, 1-obj, 2-mobile
			temp_trig->me = edit_mob->mob->virtual;
			
			larg_trigger_add(temp_trig, &edit_mob->triggers);
		}
		return;
	}
	else if ( !str_cmp (subcmd, "scriptdel") ){
	
		if ( !*argument ) {
			send_to_char("Correct format is - scriptdel Type Script_name  Function_name.\nTypes are 1-Timed, 2-Enter, 3-Leave, 4-Tell, 5-Ask, 6-Say, 7-Give\n\nType 1 is the only one implemented at this time\n", ch);
			return;
		}
		else {	
			temp_trig = (TRIGGER_DATA *)alloc ((int)sizeof (TRIGGER_DATA), 24);
			
			argument = one_argument (argument, buf);
			if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 7 ) {
				send_to_char ("Expected type 1..7\n", ch);
				return;
			}
			temp_trig->type = strtol(buf, NULL, 10);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of script\n", ch);
				return;
			}
			temp_trig->script = add_hash(buf);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of function\n", ch);
				return;
			}
			temp_trig->func = add_hash(buf);
			
			temp_trig->source = 2; //0-room, 1-obj, 2-mobile
			temp_trig->me = edit_mob->mob->virtual;
			
			larg_mo_script_delete(temp_trig, edit_mob->triggers);
		}
	}
	
	else if ( !str_cmp (subcmd, "scriptlist") ){
		snprintf (buf, MAX_STRING_LENGTH,  "This Mobile has the following scripts:\n\n Type     Script     Function\n");
		send_to_char (buf, ch);
			
		for (temp_trig = edit_mob->triggers; temp_trig; temp_trig = temp_trig->next){
			snprintf (buf, MAX_STRING_LENGTH,  "%d     %s     %s\n", temp_trig->type, temp_trig->script, temp_trig->func);
			send_to_char (buf, ch);
		}
	}
/** end Lua triggers **/

/** Currency used by this mob **/
		else if ( !str_cmp (subcmd, "currency") ) {
			if ( !IS_NPC (edit_mob) ) {
				send_to_char ("Currency type may only be set on mobiles.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			if ( !*buf ) {
				send_to_char ("Which currency type should they deal in?\n", ch);
				return;
			}
			*buf = tolower(*buf);
			if ( !str_cmp (buf, "1") ) {
				edit_mob->mob->currency_type = 1;
				send_to_char ("That mobile will now deal in type 1 currency.\n", ch);
			}
			else if ( !str_cmp (buf, "0") ) {
				edit_mob->mob->currency_type = 0;
				send_to_char ("That mobile will now deal in type 0 currency.\n", ch);
			}
			else {
				send_to_char ("Specify 1 or 0 currency.\n", ch);
				return;
			}
		}

/** level of PC staff **/
		else if ( !str_cmp (subcmd, "level") ) {

			if ( GET_TRUST (ch) < 5 ) {
				send_to_char ("You cannot assign levels.\n", ch);
				break;
			}

			if ( IS_NPC (ch) ) {
				send_to_char ("Change levels on PCs only.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( !str_cmp (buf, "builder-only") ) {
				builder_only = TRUE;
				argument = one_argument (argument, buf);
			}

			if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 5 ) {
				send_to_char ("Expected level 0..5\n", ch);
				return;
			}
			edit_mob->pc->level = strtol(buf, NULL, 10);
			
			save_char (edit_mob, TRUE);
			send_to_char ("Player level set.\n", ch);
			return;
		}

/** Change Account for PC **/
		else if ( !str_cmp (subcmd, "account") ) {
			if ( GET_TRUST (ch) < 5 ) {
				send_to_char ("This option is only for level 5 admins.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			if ( !*buf ) {
				send_to_char ("Which account did you wish to move this PC to?\n", ch);
				return;
			}
			if ( IS_NPC (edit_mob) ) {
				send_to_char ("This command is for PCs only.\n", ch);
				return;
			}
			if ( edit_mob->pc->account && *edit_mob->pc->account )
				mem_free (edit_mob->pc->account);
			buf[0] = toupper(buf[0]);
			edit_mob->pc->account = str_dup (buf);
			send_to_char ("Account set.\n", ch);
			save_char (edit_mob, FALSE);
			return;
		}

/** No Gain for PC skills **/
		else if ( !str_cmp (subcmd, "nogain") ) {
			if ( GET_TRUST(ch) < 4 ) {
				send_to_char ("You must be at least a level four admin to use this flag.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			if ( !*buf ) {
				send_to_char ("Toggle the nogain flag in which skill?\n", ch);
				return;
			}
			ind = skill_index_lookup (buf);
			if ( ind == -1 ) {
				send_to_char ("Unknown skill.\n", ch);
				return;
			}
			af = get_affect (edit_mob, MAGIC_FLAG_NOGAIN + ind);
			if ( !af ) {
				CREATE (af, AFFECTED_TYPE, 1);
				af->type = MAGIC_FLAG_NOGAIN + ind;
				affect_to_char (edit_mob, af);
				send_to_char ("Skill advancement has been halted in the specified skill.\n", ch);
				if ( !IS_NPC (edit_mob) ) {
					send_to_char ("\n#2Please leave a note explaining the situation for inclusion in their pfile.#0\n", ch);
					snprintf (buf, MAX_STRING_LENGTH,  "%s Advancement Halted: %s.", edit_mob->tname, skill_data[ind].skill_name);
					do_write (ch, buf, 0);
				}
			}
			else {
				affect_remove (edit_mob, af);
				send_to_char ("The character may now resume advancement in the specified skill.\n", ch);
				if ( !IS_NPC (edit_mob) ) {
					send_to_char ("\n#2Please leave a note explaining the situation for inclusion in their pfile.#0\n", ch);
					snprintf (buf, MAX_STRING_LENGTH,  "%s Advancement Resumed: %s.", edit_mob->tname, skill_data[ind].skill_name);
					do_write (ch, buf, 0);
				}
			}
			return;
		}

/** Put a PC to sleep **/
		else if ( !str_cmp (subcmd, "sleep") ) {

			if ( !edit_mob->pc ) {
				send_to_char ("The sleep keyword can only be used against "
							  "PCs.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( !isdigit (*buf) ) {
				send_to_char ("Sleep is in seconds and less than 600 (10 "
							  "minutes).\n", ch);
				break;
			}

			if ( strtol(buf, NULL, 10) > 600 )
				edit_mob->pc->sleep_needed = strtol(buf, NULL, 10);
			else
				edit_mob->pc->sleep_needed = 100000 * strtol(buf, NULL, 10);

			if ( sleep_needed_in_seconds (edit_mob) > 600 || strtol(buf, NULL, 10) > 600) {
				snprintf (buf, MAX_STRING_LENGTH,  "Sleep needed set to %d minutes, %d seconds.\n",
						 sleep_needed_in_seconds (edit_mob) / 60,
						 sleep_needed_in_seconds (edit_mob) % 60);
				send_to_char (buf, ch);
			}
		}

		else if ( !str_cmp (subcmd, "nopetition") ) {
			if ( IS_NPC (edit_mob) || !edit_mob->pc->account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			account = load_account (edit_mob->pc->account);

			if ( !account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			if ( IS_SET (account->flags, ACCOUNT_NOPETITION) ) {
				REMOVE_BIT (account->flags, ACCOUNT_NOPETITION);
				send_to_char ("The petition ban on that PC's account has been removed.\n", ch);
			}
			else {
				SET_BIT (account->flags, ACCOUNT_NOPETITION);
				send_to_char ("A petition ban has been placed on that PC's account.\n", ch);
			}

			save_account (account);

			return;

		}

		else if ( !str_cmp (subcmd, "noplayerport") && GET_TRUST (ch) == 5 ) {
			if ( !IS_SET (edit_mob->plr_flags, NO_PLAYERPORT) ) {
				SET_BIT (edit_mob->plr_flags, NO_PLAYERPORT);
				send_to_char ("That admin is no longer allowed player port access.\n", ch);
				return;
			}

			REMOVE_BIT (edit_mob->plr_flags, NO_PLAYERPORT);
			send_to_char ("That admin will now be allowed to log in to the player port.\n", ch);
			return;
		}

		else if ( !str_cmp (subcmd, "noguest") ) {
			if ( IS_NPC (edit_mob) || !edit_mob->pc->account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			account = load_account (edit_mob->pc->account);

			if ( !account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			if ( IS_SET (account->flags, ACCOUNT_NOGUEST) ) {
				REMOVE_BIT (account->flags, ACCOUNT_NOGUEST);
				send_to_char ("The guest ban on that PC's account has been removed.\n", ch);
			}
			else {
				SET_BIT (account->flags, ACCOUNT_NOGUEST);
				send_to_char ("A guest ban has been placed on that PC's account.\n", ch);
			}

			save_account (account);

			return;

		}

		else if ( !str_cmp (subcmd, "ipsharingok") ) {
			if ( IS_NPC (edit_mob) || !edit_mob->pc->account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			account = load_account (edit_mob->pc->account);

			if ( !account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			if ( IS_SET (account->flags, ACCOUNT_IPSHARER) ) {
				REMOVE_BIT (account->flags, ACCOUNT_IPSHARER);
				send_to_char ("That player will no longer be able to share IPs with others.\n", ch);
			}
			else {
				SET_BIT (account->flags, ACCOUNT_IPSHARER);
				send_to_char ("The IP sharing restrictions have been lifted on that account.\n", ch);
			}

			save_account (account);

			return;

		}

		else if ( !str_cmp (subcmd, "nopsi") ) {
			if ( IS_NPC (edit_mob) || !edit_mob->pc->account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			account = load_account (edit_mob->pc->account);

			if ( !account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			if ( IS_SET (account->flags, ACCOUNT_NOPSI) ) {
				REMOVE_BIT (account->flags, ACCOUNT_NOPSI);
				send_to_char ("The psionics ban on that PC's account has been removed.\n", ch);
			}
			else {
				SET_BIT (account->flags, ACCOUNT_NOPSI);
				send_to_char ("A psionics ban has been placed on that PC's account.\n", ch);
			}

			save_account (account);

			return;

		}

		else if ( !str_cmp (subcmd, "noretire") ) {
			if ( IS_NPC (edit_mob) || !edit_mob->pc->account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			account = load_account (edit_mob->pc->account);

			if ( !account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			if ( IS_SET (account->flags, ACCOUNT_NORETIRE) ) {
				REMOVE_BIT (account->flags, ACCOUNT_NORETIRE);
				send_to_char ("The retirement ban on that PC's account has been removed.\n", ch);
			}
			else {
				SET_BIT (account->flags, ACCOUNT_NORETIRE);
				send_to_char ("A retirement ban has been placed on that PC's account.\n", ch);
			}

			save_account (account);

			return;

		}

		else if ( !str_cmp (subcmd, "noban") ) {
			if ( IS_NPC (edit_mob) || !edit_mob->pc->account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			account = load_account (edit_mob->pc->account);

			if ( !account ) {
				send_to_char ("I couldn't access that character's account.\n", ch);
				return;
			}

			if ( IS_SET (account->flags, ACCOUNT_NOBAN) ) {
				REMOVE_BIT (account->flags, ACCOUNT_NOBAN);
				send_to_char ("That PC's account is no longer exempt from any applicable sitebans.\n", ch);
			}
			else {
				SET_BIT (account->flags, ACCOUNT_NOBAN);
				send_to_char ("That PC's account is now exempt from any applicable sitebans.\n", ch);
			}

			save_account (account);

			return;

		}

		else if ( !str_cmp (subcmd, "state") ) {

			if ( IS_NPC (edit_mob) ) {
				send_to_char ("Mobiles don't have a create state.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( !strtol(buf, NULL, 10) && !isdigit (*buf) ) {
				send_to_char ("Expected a creation state value -1..4.\n", ch);
				break;
			}

			if ( strtol(buf, NULL, 10) > 4 || strtol(buf, NULL, 10) < -1 ) {
				send_to_char ("Expected a creation state value -1..4.\n", ch);
				break;
			}

			edit_mob->pc->create_state = strtol(buf, NULL, 10);

			if ( edit_mob->pc->create_state != STATE_DIED )
				REMOVE_BIT (edit_mob->flags, FLAG_DEAD);
			else
				SET_BIT (edit_mob->flags, FLAG_DEAD);

			if ( edit_mob->pc->create_state != 1 ) {
				strcpy (subcmd, edit_mob->tname);
				*subcmd = toupper (*subcmd);
			} else {
				strcpy (subcmd, edit_mob->tname);
				*subcmd = toupper (*subcmd);
			}
		}

		else if ( !str_cmp (subcmd, "willskin") ) {
			TOGGLE (edit_mob->flags, FLAG_WILLSKIN);
		}

		else if ( !str_cmp (subcmd, "openinghour") ) {
			argument = one_argument (argument, buf);
			if ( !*buf || !isdigit(*buf) || strtol(buf, NULL, 10) > 24 || strtol(buf, NULL, 10) < 1 ) {
				send_to_char ("Expected a value from 1 to 24.\n", ch);
				return;
			}
			if ( !IS_SET (edit_mob->flags, FLAG_KEEPER) ) {
				send_to_char ("This mob isn't a shopkeeper!\n", ch);
				return;
			}
			else edit_mob->shop->opening_hour = strtol(buf, NULL, 10);
			send_to_char ("Done.\n", ch);
			return;
		}

		else if ( !str_cmp (subcmd, "closinghour") ) {
			argument = one_argument (argument, buf);
			if ( !*buf || !isdigit(*buf) || strtol(buf, NULL, 10) > 24 || strtol(buf, NULL, 10) < 1 ) {
				send_to_char ("Expected a value from 1 to 24.\n", ch);
				return;
			}
			if ( !IS_SET (edit_mob->flags, FLAG_KEEPER) ) {
				send_to_char ("This mob isn't a shopkeeper!\n", ch);
				return;
			}
			else edit_mob->shop->closing_hour = strtol(buf, NULL, 10);
			send_to_char ("Done.\n", ch);
			return;
		}

		else if ( !str_cmp (subcmd, "shop_exit") ) {
			argument = one_argument (argument, buf);
			if ( !*buf || !isdigit(*buf) || strtol(buf, NULL, 10) > 5 || strtol(buf, NULL, 10) < 0 ) {
				send_to_char ("Expected a value from 0 to 5.\n", ch);
				send_to_char ("North: 0, East: 1, South: 2,\n", ch);
				send_to_char ("West: 3, Up: 4, Down: 5\n", ch);
				return;
			}
			if ( !IS_SET (edit_mob->flags, FLAG_KEEPER) ) {
				send_to_char ("This mob isn't a shopkeeper!\n", ch);
				return;
			}
			else edit_mob->shop->exit = strtol(buf, NULL, 10);
			send_to_char ("Done.\n", ch);
			return;
		}

		else if ( !str_cmp (subcmd, "keeper") ) {
			if ( IS_SET (edit_mob->flags, FLAG_KEEPER) ) {
				REMOVE_BIT (edit_mob->flags, FLAG_KEEPER);
				send_to_char ("Note:  Keeper flag removed.  Shop data will "
							  "be deleted when the zone\n"
							  "       is saved.\n", ch);
			}

		    else {
				SET_BIT (edit_mob->flags, FLAG_KEEPER);

				if ( !edit_mob->shop )
					CREATE (edit_mob->shop, SHOP_DATA, 1);

				edit_mob->shop->discount = 1.0;
				edit_mob->shop->markup   = 1.0;
			}
		}

		else if ( !str_cmp (subcmd, "ostler") ) {
			if ( GET_FLAG (edit_mob, FLAG_OSTLER) )
				REMOVE_BIT (edit_mob->flags, FLAG_OSTLER);
			else if ( !GET_FLAG (edit_mob, FLAG_KEEPER) )
				send_to_char ("Assign KEEPER flag before OSTLER.\n", ch);
			else
				SET_BIT (edit_mob->flags, FLAG_OSTLER);
		}
			
		else if ( !str_cmp (subcmd, "markup") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a markup."
							  "\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->markup);
			else {
				send_to_char ("Expected a markup multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "discount") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->discount);
			else {
				send_to_char ("Expected a discount multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-discount1") ||
				  !str_cmp (subcmd, "econ_discount1") ||
				  !str_cmp (subcmd, "discount1") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_discount1);
			else {
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-markup1") ||
				  !str_cmp (subcmd, "econ_markup1") ||
				  !str_cmp (subcmd, "markup1") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_markup1);
			else {
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-discount2") ||
				  !str_cmp (subcmd, "econ_discount2") ||
				  !str_cmp (subcmd, "discount2") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_discount2);
			else {
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-markup2") ||
				  !str_cmp (subcmd, "econ_markup2") ||
				  !str_cmp (subcmd, "markup2") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_markup2);
			else {
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-discount3") ||
				  !str_cmp (subcmd, "econ_discount3") ||
				  !str_cmp (subcmd, "discount3") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_discount3);
			else {
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-markup3") ||
				  !str_cmp (subcmd, "econ_markup3") ||
				  !str_cmp (subcmd, "markup3") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_markup3);
			else {
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-discount4") ||
				  !str_cmp (subcmd, "econ_discount4") ||
				  !str_cmp (subcmd, "discount4") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_discount4);
			else {
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-markup4") ||
				  !str_cmp (subcmd, "econ_markup4") ||
				  !str_cmp (subcmd, "markup4") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_markup4);
			else {
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-discount5") ||
				  !str_cmp (subcmd, "econ_discount5") ||
				  !str_cmp (subcmd, "discount5") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_discount5);
			else {
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-markup5") ||
				  !str_cmp (subcmd, "econ_markup5") ||
				  !str_cmp (subcmd, "markup5") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_markup5);
			else {
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-discount6") ||
				  !str_cmp (subcmd, "econ_discount6") ||
				  !str_cmp (subcmd, "discount6") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_discount6);
			else {
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-markup6") ||
				  !str_cmp (subcmd, "econ_markup6") ||
				  !str_cmp (subcmd, "markup6") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_markup6);
			else {
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-discount7") ||
				  !str_cmp (subcmd, "econ_discount7") ||
				  !str_cmp (subcmd, "discount7") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic discount.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_discount7);
			else {
				send_to_char ("Expected a economic discount multiplier.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "econ-markup7") ||
				  !str_cmp (subcmd, "econ_markup7") ||
				  !str_cmp (subcmd, "markup7") ) {
			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a "
							  "economic markup.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '.' || isdigit (*buf) )
				sscanf (buf, "%f", &edit_mob->shop->econ_markup7);
			else {
				send_to_char ("Expected a economic markup multiplier.\n", ch);
				break;
			}
		}

		else if ( (ind = index_lookup (econ_flags, subcmd)) != -1 ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags1, 1 << ind);

			if ( !str_cmp (econ_flags [ind], "nobarter") )
				send_to_char ("Nobarter set as an econ flag.  Did you mean to "
							  "set it as a nobuy flag?\nAs in 'mset nobuy "
							  "nobarter'?\n", ch);
		}

		else if ( !str_cmp (subcmd, "nobuy") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("No such econ flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->nobuy_flags, 1 << ind);
		}

		else if ( !str_cmp (subcmd, "econ1") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("No such econ1 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags1, 1 << ind);
		}

		else if ( !str_cmp (subcmd, "econ2") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("No such econ2 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags2, 1 << ind);
		}

		else if ( !str_cmp (subcmd, "econ3") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("No such econ3 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags3, 1 << ind);
		}

		else if ( !str_cmp (subcmd, "econ4") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("No such econ4 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags4, 1 << ind);
		}

		else if ( !str_cmp (subcmd, "econ5") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("No such econ5 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags5, 1 << ind);
		}

		else if ( !str_cmp (subcmd, "econ6") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("No such econ6 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags6, 1 << ind);
		}

		else if ( !str_cmp (subcmd, "econ7") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting econ "
							  "flags.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				send_to_char ("No such econ7 flag.\n", ch);
				break;
			}

			TOGGLE (edit_mob->shop->econ_flags7, 1 << ind);
		}


		else if ( !str_cmp (subcmd, "shop") ) {

			argument = one_argument (argument, buf);

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a shop."
							  "\n", ch);
				break;
			}

			if ( *buf != '-' && !isdigit (*buf) ) {
				send_to_char ("Expected a room vnum or -1 (any room) after "
							  " the 'shop' keyword.\n", ch);
				break;
			}

			if ( strtol(buf, NULL, 10) > 0 && !vtor (strtol(buf, NULL, 10)) ) {
				send_to_char ("The shop room vnum specified doesn't exist.\n",
							  ch);
				break;
			}

			edit_mob->shop->shop_vnum = strtol(buf, NULL, 10);
		}

		else if ( !str_cmp (subcmd, "store") ) {

			argument = one_argument (argument, buf);

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before setting a store."
							  "\n", ch);
				break;
			}

			if ( *buf != '-' && !isdigit (*buf) ) {
				send_to_char ("Expected a room vnum or -1 (none) after "
							  " the 'store' keyword.\n", ch);
				break;
			}

			if ( strtol(buf, NULL, 10) > 0 && !vtor (strtol(buf, NULL, 10)) ) {
				send_to_char ("The store room vnum specified doesn't exist.\n",
							  ch);
				break;
			}

			edit_mob->shop->store_vnum = strtol(buf, NULL, 10);
		}

		else if ( !str_cmp (subcmd, "delivery") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("Assign the KEEPER flag before using delivery."
							  "\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			p = one_argument (buf, buf2);
			
			do {
				if ( !isdigit (*buf2) ) {
					error = 1;
					snprintf (buf, MAX_STRING_LENGTH,  "%d is not a delivery object vnum.\n",
								  (int)strtol(buf2, NULL, 10));
					send_to_char (buf, ch);
					break;
				}

				if ( !vtoo ((j = strtol(buf2, NULL, 10))) ) {
					error = 1;
					snprintf (buf, MAX_STRING_LENGTH,  "%d is not defined.\n", (int)strtol(buf2, NULL, 10));
					send_to_char (buf, ch);
					break;
				}

				for ( ind = -1, i = MAX_DELIVERIES - 1; i >= 0; i-- ) {

					if ( edit_mob->shop->delivery [i] == -1 )
						edit_mob->shop->delivery [i] = 0;

					if ( !edit_mob->shop->delivery [i] )
						ind = i;

					else if ( edit_mob->shop->delivery [i] == j ) {
						ind = -2;
						break;
					}
				}

				if ( ind == -2 )
					edit_mob->shop->delivery [i] = 0;
				else if ( i == -1 && ind == -1 )
					send_to_char ("Delivery table is full, sorry.\n", ch);
				else if ( i == -1 && ind != -1 )
					edit_mob->shop->delivery [ind] = j;

				p = one_argument (p, buf2);

			} while (*buf2);
		}

		else if ( !str_cmp (subcmd, "newbie") ) {
			if ( !IS_SET (edit_mob->plr_flags, NEWBIE) ) {
				SET_BIT (edit_mob->plr_flags, NEWBIE);
				send_to_char ("Newbie bit enabled.\n", ch);
			}
			else {
				REMOVE_BIT (edit_mob->plr_flags, NEWBIE);
				send_to_char ("Newbie bit removed.\n", ch);
			}
		}

		else if ( !str_cmp (subcmd, "allskills") ) {

			for ( ind = 1; ind <= LAST_SKILL; ind++ ) {
				open_skill (edit_mob, ind);
				if ( IS_NPC (edit_mob) )
					edit_mob->skills [ind] = 100;
				else {
					edit_mob->pc->skills [ind] = 100;
					edit_mob->skills [ind] = 100;
				}
			}
		}

		else if ( !str_cmp (subcmd, "noskills") ) {
			for ( ind = 1; ind <= LAST_SKILL; ind++ ) {
				if ( IS_NPC (edit_mob) )
					edit_mob->skills [ind] = 0;
				else {
					if ( ind == SKILL_SPEAK_WESTRON ) {
						send_to_char ("NOTE:  Leaving Westron set.\n", ch);
						continue;
					}

					i = edit_mob->pc->skills [ind];
					edit_mob->pc->skills [ind] = 0;
					edit_mob->skills [ind] -= i;
				}
			}
		}

		else if ( !str_cmp (subcmd, "speaks") ||
				  !str_cmp (subcmd, "speak") ) {

			argument = one_argument (argument, subcmd);

			if ( (ind = skill_index_lookup (subcmd)) == -1 ) {
				send_to_char ("Unknown language.\n", ch);
				break;
			}

			if ( !edit_mob->skills [ind] )
				send_to_char ("NOTE:  Mob doesn't know that language.\n", ch);

			edit_mob->speaks = ind;
		}

		else if ( !str_cmp (subcmd, "trades") ) {

			if ( !edit_mob->shop ) {
				send_to_char ("That mob isn't a shopkeeper.\n", ch);
				return;
			}

			argument = one_argument (argument, buf);

			p = one_argument (buf, buf2);
			
			do {
				if ( (ind = index_lookup (item_types, buf2)) == -1 ) {
					snprintf (buf, MAX_STRING_LENGTH,  "Unknown item-type: %s\n", buf2);
					send_to_char (buf, ch);
					error = 1;
					break;
				}

				for ( j = -1, i = MAX_TRADES_IN - 1; i >= 0; i-- ) {

					if ( !edit_mob->shop->trades_in [i] )
						j = i;

					else if ( edit_mob->shop->trades_in [i] == ind ) {
						edit_mob->shop->trades_in [i] = 0;
						break;
					}
				}

				if ( j == -1 && i == -1 ) {
					snprintf (buf, MAX_STRING_LENGTH,  "Trades table full, sorry: %s\n", buf2);
					send_to_char (buf, ch);
					error = 1;
					break;
				}

				else if ( j != -1 && i == -1 )
					edit_mob->shop->trades_in [j] = ind;

				p = one_argument (p, buf2);

			} while ( *buf2 );

			if ( error )
				break;
		}


		else if ( !str_cmp (subcmd, "access") ) {

			if ( !IS_NPC (edit_mob) ) {
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			p = one_argument (buf, buf2);
			
			do {
				if ( (ind = index_lookup (room_bits, buf2)) == -1 ) {
					snprintf (buf, MAX_STRING_LENGTH,  "Unknown room-flag: %s\n", buf2);
					send_to_char (buf, ch);
					error = 1;
					break;
				}

				if ( IS_SET (edit_mob->mob->access_flags, 1 << ind) )
					REMOVE_BIT (edit_mob->mob->access_flags, 1 << ind);
				else
					SET_BIT (edit_mob->mob->access_flags, 1 << ind);

				p = one_argument (p, buf2);

			} while ( *buf2 );

			if ( error )
				break;
		}

		else if ( !str_cmp (subcmd, "noaccess") ) {

			if ( !IS_NPC (edit_mob) ) {
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			p = one_argument (buf, buf2);
			
			do {
				if ( (ind = index_lookup (room_bits, buf2)) == -1 ) {
					snprintf (buf, MAX_STRING_LENGTH,  "Unknown room-flag: %s\n", buf2);
					send_to_char (buf, ch);
					error = 1;
					break;
				}

				if ( IS_SET (edit_mob->mob->noaccess_flags, 1 << ind) )
					REMOVE_BIT (edit_mob->mob->noaccess_flags, 1 << ind);
				else
					SET_BIT (edit_mob->mob->noaccess_flags, 1 << ind);

				p = one_argument (p, buf2);

			} while ( *buf2 );

			if ( error )
				break;
		}

		else if ( !str_cmp (subcmd, "jailer") ) {
			if ( IS_SET (edit_mob->act, ACT_JAILER) ) {
				REMOVE_BIT (edit_mob->act, ACT_JAILER);
				send_to_char ("Creature is no longer flagged as a jailer.\n", ch);
			}
			else {
				SET_BIT (edit_mob->act, ACT_JAILER);
				send_to_char ("Creature is now flagged as a jailer. Be SURE to set\n"
					      "the cell1, cell2, and cell3 values.\n", ch);
			}
		}

		else if ( !str_cmp (subcmd, "cell1") || !str_cmp (subcmd, "cell2") || !str_cmp (subcmd, "cell3") ) {
			argument = one_argument (argument, buf);

			if ( (*buf == '0' || (strtol(buf, NULL, 10) && strtol(buf, NULL, 10) >= 0)) && !str_cmp (subcmd, "cell1") ) {
				snprintf (buf2, MAX_STRING_LENGTH,  "Old cell1 VNUM: %d.\nNew cell1 VNUM: %d.\n", edit_mob->cell_1, (int)strtol(buf, NULL, 10));
				edit_mob->cell_1 = strtol(buf, NULL, 10);
				send_to_char (buf2, ch);
			}

			else if ( (*buf == '0' || (strtol(buf, NULL, 10) && strtol(buf, NULL, 10) >= 0)) && !str_cmp (subcmd, "cell2") ) {
				snprintf (buf2, MAX_STRING_LENGTH,  "Old cell2 VNUM: %d.\nNew cell2 VNUM: %d.\n", edit_mob->cell_2, (int)strtol(buf, NULL, 10));
				edit_mob->cell_2 = strtol(buf, NULL, 10);
				send_to_char (buf2, ch);
			}

			else if ( (*buf == '0' || (strtol(buf, NULL, 10) && strtol(buf, NULL, 10) >= 0)) && !str_cmp (subcmd, "cell3") ) {
				snprintf (buf2, MAX_STRING_LENGTH,  "Old cell3 VNUM: %d.\nNew cell3 VNUM: %d.\n", edit_mob->cell_3, (int)strtol(buf, NULL, 10));
				edit_mob->cell_3 = strtol(buf, NULL, 10);
				send_to_char (buf2, ch);
			}

			else if ( strtol(buf, NULL, 10) && strtol(buf, NULL, 10) < 0 ) {
				send_to_char ("Expected a value of at least 0.\n", ch);
				return;
			}

			else {
				send_to_char ("Expected the integer value of the target room's VNUM.\n", ch);
				return;
			}
		}

		else if ( !str_cmp (subcmd, "hits") || !str_cmp (subcmd, "hp") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				edit_mob->hit = strtol(buf, NULL, 10);
				if ( IS_NPC (edit_mob) )
					edit_mob->max_hit = edit_mob->hit;
			} else {
				send_to_char ("Expected hitpoints value.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "desc") ||
				  !str_cmp (subcmd, "descr") ||
				  !str_cmp (subcmd, "description") ) {
			(void)one_argument (argument, buf);
			if ( !str_cmp (buf, "reformat") ) {
				argument = one_argument (argument, buf);
				reformat_desc (edit_mob->description,
								 &edit_mob->description);
				send_to_char (edit_mob->description, ch);
			}
			else
				full_description = 1;
		}

		else if ( !str_cmp (subcmd, "intoxication") ) {

			argument = one_argument (argument, buf);

			if ( !just_a_number (buf) ) {
				send_to_char ("Expected ... intoxication <num>\n", ch);
				break;
			}

			edit_mob->intoxication = strtol(buf, NULL, 10);
		}

		else if ( !str_cmp (subcmd, "hunger") ) {

			argument = one_argument (argument, buf);

			if ( !just_a_number (buf) && *buf != '-' ) {
				send_to_char ("Expected ... hunger <num>\n", ch);
				break;
			}

			edit_mob->hunger = strtol(buf, NULL, 10);
		}

		else if ( !str_cmp (subcmd, "thirst") ) {

			argument = one_argument (argument, buf);

			if ( !just_a_number (buf) && *buf != '-' ) {
				send_to_char ("Expected ... thirst <num>\n", ch);
				break;
			}

			edit_mob->thirst = strtol(buf, NULL, 10);
		}

		else if ( !str_cmp (subcmd, "conds") ) {
			for ( i = 0; i < 3; i++ ) {
				argument = one_argument (argument, buf);

				if ( !isdigit (*buf) && !strtol(buf, NULL, 10)) {
					send_to_char ("You should enter 3 numbers after "
								  "conds.\n", ch);
					send_to_char ("Note:  Use mset intoxication, hunger, "
								  "thirst instead.\n", ch);
					break;
				}

				if ( i == 0 )
					edit_mob->intoxication = strtol(buf, NULL, 10);
				else if ( i == 1 )
					edit_mob->hunger = strtol(buf, NULL, 10);
				else
					edit_mob->thirst = strtol(buf, NULL, 10);
			}

			if ( !isdigit (*buf) )
				break;
		}

		else if ( !str_cmp (subcmd, "room") ) {

			if ( IS_NPC (edit_mob) ) {
				send_to_char ("This only works on PCs.  For an NPC, switch into the target mob, then\nuse GOTO.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( !isdigit (*buf) || !vtor (strtol(buf, NULL, 10)) ) {
				send_to_char ("No such room.\n", ch);
				break;
			}

			if ( edit_mob->room ) {
				char_from_room (edit_mob);
				char_to_room (edit_mob, strtol(buf, NULL, 10));
			} else
				edit_mob->in_room = strtol(buf, NULL, 10);
		}

		else if ( !str_cmp (subcmd, "hitch") ) {
			if ( !IS_NPC (edit_mob) ) {
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			if ( edit_mob->mob->vehicle_type == VEHICLE_HITCH ) {
				edit_mob->mob->vehicle_type = 0;
				send_to_char ("Hitch flag removed.\n", ch);
			}
			else {
				edit_mob->mob->vehicle_type = VEHICLE_HITCH;
				SET_BIT (edit_mob->act, ACT_VEHICLE);
				SET_BIT (edit_mob->act, ACT_MOUNT);	

				snprintf (buf, MAX_STRING_LENGTH,  "Note:  PCs entering hitch goto room %d.\n", 
						  edit_mob->mob->virtual);
				send_to_char (buf, ch);
			}
		}

		else if ( !str_cmp (subcmd, "nobleed") ) {
			if ( IS_SET (edit_mob->act, ACT_NOBLEED) ) {
				send_to_char ("This mobile will now bleed from heavy wounds.\n", ch);
				REMOVE_BIT (edit_mob->act, ACT_NOBLEED);
			}
			else {
				SET_BIT (edit_mob->act, ACT_NOBLEED);
				send_to_char ("This mobile will no longer bleed from heavy wounds.\n", ch);
			}
		}

		else if ( !str_cmp (subcmd, "prey") ) {
			if ( IS_SET (edit_mob->act, ACT_PREY) ) {
				send_to_char ("This mobile will no longer flee from any approaching creature.\n", ch);
				REMOVE_BIT (edit_mob->act, ACT_PREY);
			}
			else {
				send_to_char ("This mobile will now flee from any approaching creature.\n", ch);
				SET_BIT (edit_mob->act, ACT_PREY);
			}
		}
		else if ( !str_cmp (subcmd, "flying") ) {
			if ( IS_SET (edit_mob->act, ACT_FLYING) ) {
				send_to_char ("This mobile will no longer fly.\n", ch);
				REMOVE_BIT (edit_mob->act, ACT_FLYING);
			}
			else {
				SET_BIT (edit_mob->act, ACT_FLYING);
				send_to_char ("This mobile will now fly.\n", ch);
			}
		}

		else if ( !str_cmp (subcmd, "physician") ) {
			if ( IS_SET (edit_mob->act, ACT_PHYSICIAN) ) {
				send_to_char ("This mobile will no longer treat wounds.\n", ch);
				REMOVE_BIT (edit_mob->act, ACT_PHYSICIAN);
			}
			else {
				SET_BIT (edit_mob->act, ACT_PHYSICIAN);
				send_to_char ("This mobile will now treat wounds.\n", ch);
			}
		}

		else if ( !str_cmp (subcmd, "nobind") ) {
			if ( IS_SET (edit_mob->act, ACT_NOBIND) ) {
				send_to_char ("This mobile will now bind its own wounds.\n", ch);
				REMOVE_BIT (edit_mob->act, ACT_NOBIND);
			}
			else {
				SET_BIT (edit_mob->act, ACT_NOBIND);
				send_to_char ("This mobile will no longer bind its own wounds.\n", ch);
			}
		}

		else if ( !str_cmp (subcmd, "boat") ) {

			if ( !IS_NPC (edit_mob) ) {
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			edit_mob->mob->vehicle_type = VEHICLE_BOAT;
			SET_BIT (edit_mob->act, ACT_VEHICLE);

			snprintf (buf, MAX_STRING_LENGTH,  "Note:  PCs entering boat goto room %d.\n", 
						  edit_mob->mob->virtual);
			send_to_char (buf, ch);
		}

		else if ( !str_cmp (subcmd, "helm") ) {

			if ( !IS_NPC (edit_mob) ) {
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				edit_mob->mob->helm_room = strtol(buf, NULL, 10);
				if ( edit_mob->mob->helm_room != 0 && !vtor (edit_mob->mob->helm_room) )
					send_to_char ("NOTE:  No room exists for helm.\n", ch);
			} else {
				send_to_char ("Expected a room vnum after helm.\n", ch);
				break;
			}

			if ( !edit_mob->mob->vehicle_type )
				send_to_char ("NOTE:  Remember to set a vehicle type - BOAT or "
							  "HITCH.\n", ch);
		}

		else if ( !str_cmp (subcmd, "moves") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				edit_mob->move = strtol(buf, NULL, 10);
			} else {
				send_to_char ("Expected moves value.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "damage") ) {
			argument = one_argument (argument, buf);

			if ( strtol(buf, NULL, 10) >= 0 ) {
				edit_mob->damage = strtol(buf, NULL, 10);
				break;
			}

			else send_to_char ("Expected a positive value.\n", ch);

		}

		else if ( !str_cmp (subcmd, "maxmoves") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				edit_mob->max_move = strtol(buf, NULL, 10);
			} else {
				send_to_char ("Expected moves value.\n", ch);
				break;
			}
		}

		else if ( GET_TRUST (ch) >= 4 && !str_cmp (subcmd, "delete") ) {

			if ( !IS_NPC (edit_mob) ) {
				send_to_char ("NPC only setting:  delete\n", ch);
				break;
			}

			if ( edit_mob->deleted ) {
				edit_mob->deleted = 0;
				send_to_char ("This mobile is no longer marked for deletion."
							  "\n", ch);
				break;
			}

			for ( tch = character_list; tch; tch = tch->next ) {

				if ( tch->deleted )
					continue;

				if ( !IS_NPC (tch) )
					continue;

				if ( tch->mob->virtual == edit_mob->mob->virtual )
					break;
			}

			if ( tch ) {
				send_to_char ("Clear this mobile from the world first.\n", ch);
				return;
			}

			send_to_char ("WARNING:  This mobile is cleared from the world.  "
						  "However, the prototype\n"
						  "          cannot be removed until the zone is saved,"
						  "and the mud is rebooted.\n"
						  "          Use the DELETE option again to undo "
						  "deletion.\n", ch);

			edit_mob->deleted = 1;
		}

		else if ( !str_cmp (subcmd, "attack") ) {

			argument = one_argument (argument, buf2);

			if ( (ind = index_lookup (attack_names, buf2)) == -1 ) {
				send_to_char ("No such attack method.\n", ch);
				break;
			}

			edit_mob->nat_attack_type = ind;
		}

		else if ( !str_cmp (subcmd, "natural") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10))
				edit_mob->natural_delay = strtol(buf, NULL, 10);
			else {
				send_to_char ("Expected natural delay value.\n", ch);
				break;
			}
		}

/** Set value for Attributes - willpower **/
		else if ( !str_cmp (subcmd, "WIL") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				delta = edit_mob->wil - strtol(buf, NULL, 10);
				edit_mob->wil = strtol(buf, NULL, 10);
				GET_WIL (edit_mob) -= delta;
			} else {
				send_to_char ("Expected a value for wil.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "STR") ) {
			argument = one_argument (argument, buf);

			if ( strtol(buf, NULL, 10) < 3 || strtol(buf, NULL, 10) > 25 ) {
				send_to_char ("Expected a value of 3 to 25.\n", ch);
				return;
			}

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				delta = edit_mob->str - strtol(buf, NULL, 10);
				edit_mob->str = strtol(buf, NULL, 10);
				GET_STR (edit_mob) -= delta;
			} else {
				send_to_char ("Expected a value for str.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "DEX") ) {
			argument = one_argument (argument, buf);

			if ( strtol(buf, NULL, 10) < 3 || strtol(buf, NULL, 10) > 25 ) {
				send_to_char ("Expected a value of 3 to 25.\n", ch);
				return;
			}

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				delta = edit_mob->dex - strtol(buf, NULL, 10);
				edit_mob->dex = strtol(buf, NULL, 10);
				GET_DEX (edit_mob) -= delta;
			} else {
				send_to_char ("Expected a value for dex.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "CON") ) {
			argument = one_argument (argument, buf);

			if ( strtol(buf, NULL, 10) < 3 || strtol(buf, NULL, 10) > 25 ) {
				send_to_char ("Expected a value of 3 to 25.\n", ch);
				return;
			}

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				delta = edit_mob->con - strtol(buf, NULL, 10);
				edit_mob->con = strtol(buf, NULL, 10);
				GET_CON (edit_mob) -= delta;
			} else {
				send_to_char ("Expected a value for con.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "INT") ) {
			argument = one_argument (argument, buf);

			if ( strtol(buf, NULL, 10) < 3 || strtol(buf, NULL, 10) > 25 ) {
				send_to_char ("Expected a value of 3 to 25.\n", ch);
				return;
			}

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				delta = edit_mob->intel - strtol(buf, NULL, 10);
				edit_mob->intel = strtol(buf, NULL, 10);
				GET_INT (edit_mob) -= delta;
			} else {
				send_to_char ("Expected a value for int.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "AUR") ) {
			argument = one_argument (argument, buf);

			if ( strtol(buf, NULL, 10) < 3 || strtol(buf, NULL, 10) > 25 ) {
				send_to_char ("Expected a value of 3 to 25.\n", ch);
				return;
			}

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				delta = edit_mob->aur - strtol(buf, NULL, 10);
				edit_mob->aur = strtol(buf, NULL, 10);
				GET_AUR (edit_mob) -= delta;
			} else {
				send_to_char ("Expected a value for aur.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "AGI") ) {
			argument = one_argument (argument, buf);

			if ( strtol(buf, NULL, 10) < 3 || strtol(buf, NULL, 10) > 25 ) {
				send_to_char ("Expected a value of 3 to 25.\n", ch);
				return;
			}

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				delta = edit_mob->agi - strtol(buf, NULL, 10);
				edit_mob->agi = strtol(buf, NULL, 10);
				GET_AGI (edit_mob) -= delta;
			} else {
				send_to_char ("Expected a value for agi.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "dam") ) {

			if ( !IS_NPC (edit_mob) ) {
				send_to_char ("NPC only setting:  dam\n", ch);
				break;
			}

			bonus = 0;

			argument = one_argument (argument, buf);

			parms = sscanf (buf, "%d%[dD]%d%d", &dice, &c, &sides, &bonus);

			if ( parms < 3 ) {
				send_to_char ("The dam parameter format is #d#+/-#.\n", ch);
				break;
			}

			edit_mob->mob->damnodice = dice;
			edit_mob->mob->damsizedice = sides;
			edit_mob->mob->damroll = bonus;
		}

		else if ( !str_cmp (subcmd, "armor") || !str_cmp (subcmd, "ac") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10))
				edit_mob->armor = strtol(buf, NULL, 10);
			else {
				send_to_char ("Expected armor/ac value.\n", ch);
				break;
			}
		}

			/* NOTE:  Cannot parse aggressive because that is an NPC act */

		else if ( !str_cmp (subcmd, "frantic") )
			edit_mob->fight_mode = 0;
		else if ( !str_cmp (subcmd, "normal") )
			edit_mob->fight_mode = 2;
		else if ( !str_cmp (subcmd, "cautious") )
			edit_mob->fight_mode = 3;
		else if ( !str_cmp (subcmd, "defensive") )
			edit_mob->fight_mode = 4;

		else if ( !str_cmp (subcmd, "fightmode") ) {

			argument = one_argument (argument, buf);

			if ( !str_cmp (buf, "frantic") )
				edit_mob->fight_mode = 0;
			else if ( !str_cmp (buf, "aggressive") )
				edit_mob->fight_mode = 1;
			else if ( !str_cmp (buf, "normal") )
				edit_mob->fight_mode = 2;
			else if ( !str_cmp (buf, "cautious") )
				edit_mob->fight_mode = 3;
			else if ( !str_cmp (buf, "defensive") )
				edit_mob->fight_mode = 4;
			else {
				send_to_char ("Unknown fight mode.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "circle") ) {
			argument = one_argument (argument, buf);

			if ( isdigit (*buf) && strtol(buf, NULL, 10) <= 9 )
				edit_mob->circle = strtol(buf, NULL, 10);
			else {
				send_to_char ("Expected circle 0..9.\n", ch);
				break;
			}
		}

		else if ( index_lookup (wound_locations, subcmd) != -1 && !full_description ) {

			if ( IS_NPC(edit_mob) ) {
				send_to_char ("This command may only be used on PCs.\n", ch);
				return;
			}

			if ( str_cmp (ch->tname, IMPLEMENTOR_ACCOUNT) ) {
				send_to_char ("Sorry, but you should not be using this command.\n", ch);
				return;
			}

			for ( i = 0; wound_locations[i]; i++ )
				if ( !cmp_strn (subcmd, wound_locations[i], strlen(subcmd)) )
					snprintf (subcmd, MAX_STRING_LENGTH, "%s", wound_locations[i]);	


			if ( (sscanf (argument, "%d %s", &damage, type)) != 2 && damage != 0) {
				send_to_char("You must specify both the damage value and wound type.\n", ch);
				break;
			}

			if ( damage != 0 ) {
				if ( !str_cmp(type, "slash") )
					typenum = 2;
				else if ( !str_cmp(type, "pierce") )
					typenum = 1;
				else if ( !str_cmp(type, "blunt") )
					typenum = 3;
				else if ( !str_cmp(type, "frost") )
					typenum = 5;
				else if ( !str_cmp(type, "burn") )
					typenum = 6;
				else if ( !str_cmp(type, "bite") )
					typenum = 7;
				else if ( !str_cmp(type, "claw") )
					typenum = 8;
				else if ( !str_cmp(type, "fist") )
					typenum = 9;
				else {
					send_to_char("Invalid wound type. Must be set to one of the following:\n", ch);
					send_to_char("slash, pierce, blunt, frost, burn, bite, claw, or fist.\n", ch);
					break;
				}
			}

			if ( (damage <= 100 && damage > 0) && (typenum > 0 && typenum <= 9) ) {
				temp_arg = add_hash(subcmd);
				wound_to_char (edit_mob, temp_arg, damage, typenum, 0, 0, 0);
				break;
			}

			else if (damage == 0) {

				for ( tempwound = edit_mob->wounds;
					  tempwound;
					  tempwound = tempwound->next ) {
					if ( wound != NULL ) continue;
					if ( !str_cmp(tempwound->location, subcmd) )
						wound = tempwound;
				}

				wound_from_char(edit_mob, wound);
				break;

			}

			else {
				if (damage > 50 || damage < 0)
					send_to_char ("Expected a damage value from 0 to 100.\n", ch);
				if (typenum > 9 || typenum < 1)
					send_to_char ("Expected a wound type vaue from 1 to 9.\n", ch);
				break;
			}
		}


		else if ( !str_cmp (subcmd, "piety") ) {
			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10))
				edit_mob->ppoints = strtol(buf, NULL, 10);
			else {
				send_to_char ("Expected piety value.\n", ch);
				break;
			}
		}

		else if ( !str_cmp (subcmd, "skinobj") ) {

			if ( !IS_NPC (edit_mob) ) {
				send_to_char ("You can't set that on a PC.\n", ch);
				break;
			}

			argument = one_argument (argument, buf);

			if ( *buf == '0' || strtol(buf, NULL, 10)) {
				edit_mob->mob->skinned_vnum = strtol(buf, NULL, 10);
				if ( !vtoo (strtol(buf, NULL, 10)) )
					send_to_char ("NOTE:  No such object exists.\n", ch);
			} else {
				send_to_char ("Expected vnum of skinned item.\n", ch);
				break;
			}
		}


                else if ( !str_cmp (subcmd, "carcobj") || !str_cmp (subcmd, "carcass")) {

                        if ( !IS_NPC (edit_mob) ) {
                                send_to_char ("You can't set that on a PC.\n", ch);
                                break;
                        }

                        argument = one_argument (argument, buf);

                        if ( *buf == '0' || strtol(buf, NULL, 10)) {
                                edit_mob->mob->carcass_vnum = strtol(buf, NULL, 10);
                                if ( !vtoo (strtol(buf, NULL, 10)) )
                                        send_to_char ("NOTE:  No such object exists.\n", ch);
                        } else {
                                send_to_char ("Expected vnum of carcass item.\n", ch);
                                break;
                        }
                }

		else if ( !str_cmp (subcmd, "merch_seven") ) {
		    if ( !IS_NPC (edit_mob) ) {
                        send_to_char ("You can't set that on a PC.\n", ch);
                        break;
                    }

		    argument = one_argument (argument, buf);

		    if (*buf != '\0')
		        edit_mob->mob->merch_seven = strtol(buf, NULL, 10);
		    else {
		        send_to_char ("Expected a zero or one for merch_seven\n", ch);
		        break;
		    }
		}
			
		else if ( (ind = index_lookup (position_types, subcmd)) != -1 ) {

			edit_mob->position = ind;

			if ( edit_mob->default_pos == 0 )
				edit_mob->default_pos = ind;
		}
 		else if ( (ind = index_lookup (sex_types, subcmd)) != -1 )
			edit_mob->sex = ind;

		else if ( !str_cmp (subcmd, "deity") ) {

			argument = one_argument (argument, buf);

			if ( ( ind = index_lookup (deity_name, buf) ) != -1 )
				edit_mob->deity = ind;

		}

		else if ( !str_cmp (subcmd, "variable") ) {
			if ( GET_TRUST (ch) < 4 ) {
				send_to_char ("You must be a level four admin to set this bit.\n", ch);
				return;
			}
			if ( !IS_SET (edit_mob->flags, FLAG_VARIABLE) ) {
				SET_BIT (edit_mob->flags, FLAG_VARIABLE);
				send_to_char ("This mob will now randomize at load-up.\n\n", ch);
				send_to_char ("#1Be sure to reboot the builder port before loading this mob up for the first time!#0\n", ch);
				return;
			}
			else {
				REMOVE_BIT (edit_mob->flags, FLAG_VARIABLE);
				send_to_char ("This mob will no longer randomize at load-up.\n", ch);
				return;
			}
		}

		else if ( !str_cmp (subcmd, "isadmin") ) {
			if ( GET_TRUST (ch) < 5 ) {
				send_to_char ("You'll need a level 5 admin to set this bit.\n", ch);
				return;
			}
			if ( IS_NPC (edit_mob) || GET_TRUST (edit_mob) ) {
				send_to_char ("This bit is meant only to be set on mortal PCs.\n", ch);
				return;
			}
			if ( IS_SET (edit_mob->flags, FLAG_ISADMIN) )
				REMOVE_BIT (edit_mob->flags, FLAG_ISADMIN);
			else SET_BIT (edit_mob->flags, FLAG_ISADMIN);
			send_to_char ("IsAdminPC flag toggled on character.\n", ch);
			return;
		}

		else if ( !str_cmp (subcmd, "race") ) {

			argument = one_argument (argument, buf);

			ind = race_lookup (buf);
			if ( ind == -1 ){
				send_to_char ("Unknown mobile race.\n", ch);
				return;
			}
			else if (!IS_NPC (edit_mob) &&
					(db_race_table [ind].race_point_reqs == 999) ){
				send_to_char ("PCs can only be set to PC races.\n", ch);
			}
		else
			edit_mob->race = ind;
			argument = one_argument (argument, buf);
			if ( !str_cmp (buf, "defaults") &&
				(db_race_table [ind].race_point_reqs == 999) ) {
				randomize_mobile(edit_mob);
				send_to_char ("Stats and skills re-initialized to race defaults.\n", ch);
				return;
			}
		}

		else if ( !str_cmp (subcmd, "frame") ) {

			argument = one_argument (argument, buf);

			ind = index_lookup (frames, buf);

			if ( (ind == -1) ) {
				send_to_char ("Expected a frame: scant, light, ... \n", ch);
				break;
			}

			edit_mob->frame = ind;
		}

		else if ( !str_cmp (subcmd, "height") ) {

			argument = one_argument (argument, buf);

			if ( !isdigit (*buf) ) {
				send_to_char ("Expected height.\n", ch);
				break;
			}

			edit_mob->height = strtol(buf, NULL, 10);
		}

		else if ( (ind = skill_index_lookup (subcmd)) != -1 ) {
			argument = one_argument (argument, buf);
			if ( isdigit (*buf) ) {
				if ( strtol(buf, NULL, 10) > 100 || strtol(buf, NULL, 10) < 0 ) {
					send_to_char ("Please specify a value from 0 to 100.\n", ch);
					return;
				}
				if ( ind == SKILL_CLAIRVOYANCE || ind == SKILL_DANGER_SENSE || ind == SKILL_EMPATHIC_HEAL ||
					ind == SKILL_HEX || ind == SKILL_MENTAL_BOLT || ind == SKILL_PRESCIENCE ||
					ind == SKILL_SENSITIVITY || ind == SKILL_TELEPATHY ) {
					if ( str_cmp (ch->tname, IMPLEMENTOR_ACCOUNT) ) {
						send_to_char ("Psionics may only be rolled randomly at character creation.\n", ch);
						return;
					}
				}
				open_skill (edit_mob, ind);
				if ( IS_NPC (edit_mob) )
					edit_mob->skills [ind] = strtol(buf, NULL, 10);
				else {
					edit_mob->pc->skills [ind] = strtol(buf, NULL, 10);
					edit_mob->skills [ind] = strtol(buf, NULL, 10);
				}
			} else {
				send_to_char ("Expected a value after the skill, from 0 to 100.\n", ch);
				break;
			}

			fix_offense (edit_mob);
		}

		else if ( (ind = index_lookup (frames, subcmd) != -1) )
			edit_mob->frame = ind;

		else if ( !str_cmp (subcmd, "autoflee") ) {	/* Need {}'s for macro */
			TOGGLE (edit_mob->flags, FLAG_AUTOFLEE);
		}

		else if ( (ind = index_lookup (action_bits, subcmd)) != -1 ) {
			if ( !strcmp (subcmd, "stayput") ) {
				send_to_char ("See HELP STAYPUT for usage of this functionality.\n", ch);
				return;
			}
			if ( IS_SET (edit_mob->act, 1 << ind) ) {
				REMOVE_BIT (edit_mob->act, 1 << ind);
			}
			else {
				SET_BIT (edit_mob->act, 1 << ind);
			}
		}

		else if ( (ind = index_lookup (affected_bits, subcmd)) != -1 ) {
			if ( IS_SET (edit_mob->affected_by, 1 << ind) )
				REMOVE_BIT (edit_mob->affected_by, 1 << ind);
			else
				SET_BIT (edit_mob->affected_by, 1 << ind);
		}

		else if ( (ind = index_lookup (speeds, subcmd)) != -1 ) {
			edit_mob->speed = ind;
			send_to_char ("Note:  Speed isn't saving on mobs yet.\n", ch);
		}

		else if ( !str_cmp (subcmd, "dpos") ) {

			argument = one_argument (argument, buf);

			if ( !*buf ) {
				send_to_char ("Expected mob position after dpos.\n", ch);
				break;
			}

			if ( (ind = index_lookup (position_types, buf)) == -1 ) {
				send_to_char ("Invalid position.\n", ch);
				break;
			}

			edit_mob->default_pos = ind;
		}

		else if ( !str_cmp (subcmd, "name") ) {

			argument = one_argument (argument, buf);
			
			if ( !*buf ) {
				send_to_char ("Expected a name keywords after name.\n", ch);
				break;
			}

			if ( !IS_NPC (edit_mob) && !name_is (GET_NAME (edit_mob), buf) ) {
				send_to_char ("MAKE SURE THE PLAYERS REAL NAME IS INCLUDED!!!\n"
							  "You don't realize how many problems you are trying to create!\n", ch);
				break;
			}

			edit_mob->name = add_hash (buf);
		}

		else if ( !str_cmp (subcmd, "sdesc") ) {

			argument = one_argument (argument, buf);

			if ( !*buf ) {
				send_to_char ("Short description expected after sdesc.\n",ch);
				break;
			}

			edit_mob->short_descr = add_hash (buf);
		}

		else if ( !str_cmp (subcmd, "ldesc") ) {

			argument = one_argument (argument, buf);

			if ( !*buf ) {
				send_to_char ("Long description expected after sdesc.\n", ch);
				break;
			}

			edit_mob->long_descr = add_hash (buf);
		}

		else if ( !str_cmp (subcmd, "prog") ) {

			argument = one_argument (argument, buf);

			if ( !*buf || (ind = index_lookup (mobprog_triggers, buf)) == -1 ) {
				send_to_char ("That trigger is not known by the mud.\n", ch);
				break;
			}
			
			strcpy (trigger_name, buf);
		}

		else if ( !str_cmp (subcmd, "bite") ) {

			argument = one_argument (argument, buf);

			
		}

		else if ( !str_cmp (subcmd, "archive") ) {

			if ( GET_TRUST (ch) < 5 ) {
				send_to_char ("Only level 5 can archive pfiles.\n", ch);
				return;
			}

			if ( IS_NPC (edit_mob) ) {
				send_to_char ("You can't archive a mobile.", ch);
				return;
			}

			if ( edit_mob->pc->load_count != 1 ) {
				send_to_char ("Pfile is being accessed by more than just you.\n", ch);
				send_to_char ("Player is online or someone else has him/her mob'ed.\n", ch);
				return;
			}

			argument = one_argument (argument, buf);

			if ( str_cmp (buf, GET_NAME (edit_mob)) ) {
				send_to_char ("You are trying to archive a pfile.\n", ch);
				send_to_char ("If you were trying to archive the pfile for 'Marvin', you would type:\n\n   mset archive marvin\n", ch);
				return;
			}

			unload_pc (edit_mob);
			edit_mob = NULL;
			ch->pc->edit_player = NULL;

			*buf = toupper (*buf);

			snprintf (subcmd, MAX_STRING_LENGTH, "save/player/archive/%s", buf);
			snprintf (buf2, MAX_STRING_LENGTH,  "save/player/%c/%s", tolower (*buf), buf);

			if ( (error = rename (buf2, subcmd)) ) {
				perror ("archive");
				snprintf (buf, MAX_STRING_LENGTH,  "Failed, system error %d.\n", error);
				send_to_char (buf, ch);
				return;
			}

			send_to_char ("Pfile archived.\n", ch);

			return;
		}

		else {
			snprintf (buf, MAX_STRING_LENGTH,  "Unknown keyword: %s\n", subcmd);
			send_to_char (buf, ch);
			break;
		}

		argument = one_argument (argument, subcmd);
	}

	if ( !IS_NPC (edit_mob) )
		send_to_char ("Player modified.\n", ch);

	else if ( (i = redefine_mobiles (edit_mob)) > 0 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%d mobile(s) in world redefined.\n", i);
		send_to_char (buf, ch);
	}

	if ( *trigger_name && full_description ) {
		send_to_char ("You can't create a description and a mob program at the "
					  "same time.\nIgnoring mob program.\n", ch);
		*trigger_name = '\0';
	}

	if ( *trigger_name )
		add_replace_mobprog (ch, edit_mob, trigger_name);

	if ( full_description ) {
		if ( !IS_NPC (edit_mob) ){
			send_to_char ("\nEnter a full description.  End with @.\n", ch);
		}
		else{
			send_to_char ("\nEnter the mobile full description.  End with @.\n", ch);
		}
			
		send_to_char ("1-------10--------20--------30--------40--------50--------60---65\n", ch);	
		CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
		ch->desc->str = &ch->desc->pending_message->message;
		ch->desc->proc = post_mdesc;
		
		if ( IS_NPC (edit_mob) ){
			ch->delay_info1 = edit_mob->mob->virtual;
		}
		else{
			ch->delay_who = add_hash(edit_mob->tname);
		}
		
		send_to_char ("\nOld description:\n\n", ch);
		send_to_char (edit_mob->description, ch);
		ch->desc->max_str = STR_MULTI_LINE;
		make_quiet (ch);

		return;
	}
}

void post_mset (DESCRIPTOR_DATA *d)
{
	CHAR_DATA	*edit_mob;

	d->proc = NULL;

	if ( d->character->pc->edit_player )
		edit_mob = d->character->pc->edit_player;
	else
		edit_mob = vtom (d->character->pc->edit_mob);

	if ( !edit_mob )
		send_to_char ("DANGER:  Could not find PC or MOBILE!  Please reboot mud.\n",
					  d->character);
	else if ( IS_NPC (edit_mob) )
		redefine_mobiles (edit_mob);
}

void consider_wearing (CHAR_DATA *ch, OBJ_DATA *obj)
{
	char	buf [MAX_STRING_LENGTH];

	obj_from_room (&obj, 0);

	(void)one_argument (obj->name, buf);

	equip_char (ch, obj, obj->obj_flags.wear_flags);
}

void do_outfit (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_INPUT_LENGTH];
	char		name [MAX_STRING_LENGTH];
	CHAR_DATA	*mob;
	OBJ_DATA	*next_obj;
	OBJ_DATA	*tobj;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Outfit whom?\n", ch);
		return;
	}

	if ( !(mob = get_char_room_vis (ch, buf)) ) {
		send_to_char ("Mobile not available for outfitting.\n", ch);
		return;
	}

	if ( mob == ch )
		send_to_char ("Outfit yourself?  Ok.\n", ch);
	
	for ( tobj = mob->room->contents; tobj; tobj = next_obj ) {

		next_obj = tobj->next_content;
		
		(void)one_argument (tobj->name, name);

		snprintf (buf, MAX_STRING_LENGTH,  "get %s", name);
		command_interpreter(mob, buf);

		if ( GET_ITEM_TYPE (tobj) == ITEM_WEAPON )
			snprintf (buf, MAX_STRING_LENGTH,  "wield %s", name);
		else snprintf (buf, MAX_STRING_LENGTH,  "wear %s", name);

		command_interpreter (mob, buf);
	}
}

void purge_zone (int zone, int *obj_count, int *mob_count, int *stray_count)
{
	CHAR_DATA		*ch;
	CHAR_DATA		*next_ch;
	OBJ_DATA		*obj;
	OBJ_DATA		*next_obj;
	ROOM_DATA		*room;
	int				extracted = 0;
	int				mob_no_resets = 0;
	int				char_count = 0;
	int				mob_resets_enabled = 0;
	int				mob_resets_in_zone = 0;
	int				i;

	*obj_count = 0;
	*mob_count = 0;
	*stray_count = 0;

	for ( ch = character_list; ch; ch = next_ch ) {

		next_ch = ch->next;

		if ( ch->deleted || !IS_NPC (ch) )
			continue;

		char_count++;

		if ( ch->mob->reset_zone == 0 &&
			 ch->mob->reset_cmd == 0 ) {
			extract_char (ch);
			mob_no_resets++;
			continue;
		}

		if ( ch->mob->reset_zone != zone )
			continue;

		if ( zone_table [zone].cmd [ch->mob->reset_cmd].arg3 / 1000 != ch->room->zone )
			(*stray_count)++;

		i = ch->mob->reset_cmd;

		(*mob_count)++;

		extract_char (ch);

		extracted++;
	}

	for ( i = 0; zone_table [zone].cmd [i].command != 'S'; i++ ) {
		if ( zone_table [zone].cmd [i].command == 'M' ) {
			mob_resets_in_zone++;
			if ( zone_table [zone].cmd [i].enabled )
				mob_resets_enabled++;
		}
	}

	printf ("Extracted %d, counted %d\n", extracted, char_count);
	printf ("Mobs in zone without resets: %d\n", mob_no_resets);
	printf ("Mob resets in zone: %d\n", mob_resets_in_zone);
	printf ("Mob resets enabled: %d\n", mob_resets_enabled);
	fflush (stdout);

	for ( room = full_room_list; room; room = room->lnext ) {

		if ( room->zone != zone || IS_SET (room->room_flags, SAVE) )
			continue;

		for ( obj = room->contents; obj; obj = next_obj ) {
			next_obj = obj->next_content;
            extract_obj (obj);
			(*obj_count)++;
		}
	}

	cleanup_the_dead (0);
}

void do_refresh (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_INPUT_LENGTH];
	int			obj_count;
	int			mob_count;
	int			stray_count;
	int			obj_total = 0;
	int			mob_total = 0;
	int			stray_total = 0;
	int			not_frozen = 0;
	int			zone;
	int			i;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("You may refresh all zones with:\n\n   refresh all\n\n"
					  "or, you may refresh a single zone with:\n\n"
					  "   refresh <zone#>\n", ch);
		return;
	}

	if ( !str_cmp (buf, "all") ) {
		for ( i = 0; i < MAX_ZONE; i++ ) {

			if ( !zone_table [i].cmd )
				continue;

			if ( !IS_SET (zone_table [i].flags, Z_FROZEN) )
				not_frozen = 1;

			purge_zone (i, &obj_count, &mob_count, &stray_count);
			reset_zone (i);

			obj_total += obj_count;
			mob_total += mob_count;
			stray_total += stray_count;
		}

		if ( not_frozen )
			send_to_char ("NOTE:  At least one refresh zone was not frozen.\n",
						  ch);

		snprintf (buf, MAX_STRING_LENGTH,  "\nSummary:\n"
					  "   Mobiles destroyed:  %d\n"
					  "     of those, strays: %d\n"
					  "   Objects in rooms:   %d (mobile possessions not "
					  "included)\n", mob_total, stray_total, obj_total);
		send_to_char (buf, ch);
		
		return;
	}

	if ( !just_a_number (buf) || strtol(buf, NULL, 10) < 0 || strtol(buf, NULL, 10) >= MAX_ZONE ) {
		send_to_char ("Expected a zone 0..99 or all as parameter to refresh.\n",
					  ch);
		return;
	}

	zone = strtol(buf, NULL, 10);

	if ( !zone_table [zone].cmd ) {
		send_to_char ("There were no resets loaded for that zone.\n", ch);
		return;
	}

	purge_zone (zone, &obj_total, &mob_total, &stray_total);
	reset_zone (zone);

	if ( !IS_SET (zone_table [zone].flags, Z_FROZEN) )
		send_to_char ("NOTE:  That zone is not frozen.\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "\n\nSummary:\n"
				  "   Mobiles destroyed:  %d\n"
				  "     of those, strays: %d\n"
				  "   Objects in rooms:   %d (mobile possessions not "
				  "included)\n", mob_total, stray_total, obj_total);
	send_to_char (buf, ch);
}

void replace_object (int src, int tar, CHAR_DATA *ch)
{
	OBJ_DATA	*src_obj;
	OBJ_DATA	*tar_obj;
	OBJ_DATA	*obj;
	int			zone;
	int			cmd_no;
	int			replace_count = 0;
	int			affected_zone [100];		/* Wanna use MAX_ZONE, but there
                                               a bug on the photobooks computer,
                                               forcing me to use 100 */
	char		cmd;
	char		buf [MAX_STRING_LENGTH];

	if ( !(src_obj = vtoo (src)) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Object #%d does not exist.\n", src);
		send_to_char (buf, ch);
		return;
	}

	if ( !(tar_obj = vtoo (tar)) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Object #%d does not exist.\n", tar);
		send_to_char (buf, ch);
		return;
	}

	for ( zone = 0; zone < MAX_ZONE; zone++ ) {

		affected_zone [zone] = 0;

		for ( cmd_no = 0; ; cmd_no++ ) {

			cmd = zone_table [zone].cmd [cmd_no].command;

			if ( cmd == 'S' )
				break;

			if ( (cmd == 'M' || cmd == 'O' || cmd == 'P' ||
				  cmd == 'G' || cmd == 'E') &&
				 zone_table [zone].cmd [cmd_no].arg1 == src ) {
				zone_table [zone].cmd [cmd_no].arg1 = tar;
				replace_count++;
				affected_zone [zone] = 1;
			}
		}
	}

	snprintf (buf, MAX_STRING_LENGTH,  "%d object resets replaced.\n", replace_count);
	send_to_char (buf, ch);

	replace_count = 0;

	for ( obj = object_list; obj; obj = obj->next ) {
		if ( obj->virtual == src ) {
			obj->virtual = tar;
			replace_count++;
		}
	}

	snprintf (buf, MAX_STRING_LENGTH,  "%d objects in world replaced.\n", replace_count);
	send_to_char (buf, ch);

    (void) redefine_objects (tar_obj);

	send_to_char ("Based on resets, the following zones were affected:\n"
				  "(WARNING:  There could be more:)\n", ch);
	*buf = '\0';

	for ( zone = 0; zone < MAX_ZONE; zone++ ) {
		if ( affected_zone [zone] )
			snprintf (ADDBUF, MAX_STRING_LENGTH, "   %2d", zone);
	}

	if ( *buf == '\0' )
		send_to_char ("   None.\n", ch);
	else {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "\n");
		send_to_char (buf, ch);
	}
}

void replace_mobile (int src, int tar, CHAR_DATA *ch)
{
	CHAR_DATA	*src_mob;
	CHAR_DATA	*tar_mob;
	CHAR_DATA	*mob;
	int			zone;
	int			cmd_no;
	int			replace_count = 0;
	int			affected_zone [100];		/* Wanna use MAX_ZONE, but there
                                               a bug on the photobooks computer,
                                               forcing me to use 100 */
	char		cmd;
	char		buf [MAX_STRING_LENGTH];

	if ( !(src_mob = vtom (src)) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Mobile #%d does not exist.\n", src);
		send_to_char (buf, ch);
		return;
	}

	if ( !(tar_mob = vtom (tar)) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Mobile #%d does not exist.\n", tar);
		send_to_char (buf, ch);
		return;
	}

	for ( zone = 0; zone < MAX_ZONE; zone++ ) {

		affected_zone [zone] = 0;

		for ( cmd_no = 0; ; cmd_no++ ) {

			cmd = zone_table [zone].cmd [cmd_no].command;

			if ( cmd == 'S' )
				break;

			if ( cmd == 'M' && zone_table [zone].cmd [cmd_no].arg1 == src ) {
				zone_table [zone].cmd [cmd_no].arg1 = tar;
				replace_count++;
				affected_zone [zone] = 1;
			}
		}
	}

	snprintf (buf, MAX_STRING_LENGTH,  "%d mobile resets replaced.\n", replace_count);
	send_to_char (buf, ch);

	replace_count = 0;

	for ( mob = character_list; mob; mob = mob->next ) {
		if ( mob->deleted )
			continue;
		if ( IS_NPC (mob) && mob->mob->virtual == src ) {
			mob->mob->virtual = tar;
			replace_count++;
		}
	}

	snprintf (buf, MAX_STRING_LENGTH,  "%d mobiles in world replaced.\n", replace_count);
	send_to_char (buf, ch);

    (void) redefine_mobiles (tar_mob);

	send_to_char ("Based on resets, the following zones were affected:\n"
				  "(WARNING:  There could be more:)\n", ch);
	*buf = '\0';

	for ( zone = 0; zone < MAX_ZONE; zone++ ) {
		if ( affected_zone [zone] )
			snprintf (ADDBUF, MAX_STRING_LENGTH, "   %2d", zone);
	}

	if ( *buf == '\0' )
		send_to_char ("   None.\n", ch);
	else {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "\n");
		send_to_char (buf, ch);
	}
}

void do_replace (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_INPUT_LENGTH];
	int			src;
	int			tar;
	int			replace_obj = 0;
	char		*syntax = "Syntax:\n   REPLACE OBJ <vnum> WITH <vnum>\n";

		/** NOTE:  The replace procedure is imperfect.  Reset "counts" are
				  not updated when replaced.
		*/

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "object") ||
		!str_cmp (buf, "obj"))
		replace_obj = 1;
		
	else if (!str_cmp (buf, "char") ||
			!str_cmp (buf, "mob") ||
			!str_cmp (buf, "mobile") )
		   	send_to_char("You can only replace objects right now.", ch);
	else {
		send_to_char (syntax, ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !isdigit (*buf) ) {
		send_to_char (syntax, ch);
		return;
	}

	src = strtol(buf, NULL, 10);

	argument = one_argument (argument, buf);

	if ( str_cmp (buf, "with") ) {
		send_to_char (syntax, ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !isdigit (*buf) ) {
		send_to_char (syntax, ch);
		return;
	}

	tar = strtol(buf, NULL, 10);

	if ( replace_obj )
		replace_object (src, tar, ch);
	else
		replace_mobile (src, tar, ch);
}

void recipe_help (CHAR_DATA *ch)
{
	s ("The recipe command allow a builder to add, delete, and list");
	s ("recipes that compose the recipe definition table.");
	s ("");
	s ("   recipe list [vnum]       Lists all items & ingredients in the");
	s ("                            table.  Supply vnum to restrict listing");
	s ("                            Example:  RECIPE LIST 2.2013");
	s ("");
    s ("   recipe add <item> <vnum> <vnum> [<vnum] ... up to 7 ingredients.");
	s ("                            Adds a recipe to the recipe table.");
	s ("");
	s ("   recipe delete <item>     Removes a recipy.  Use the format");
	s ("                            3.201 to remove the third recipe for 201.");
	s ("");
	s ("   recipe save              Save recipe definition table.  YOU MUST");
	s ("                            remember to save the table after making");
	s ("                            changes.");
	s ("");
}

void recipe_get_item_count (char **argument, int *count, int *item)
{
	char	buf [MAX_STRING_LENGTH];
	int		i;

	*count = 0;

	*argument = one_argument (*argument, buf);

	*item = strtol(buf, NULL, 10);

	for ( i = 0; isdigit (buf [i]); i++ )
		;

	if ( buf [i] == '.' ) {
		*count = *item;
		*item = strtol((buf + i + 1), NULL, 10);
	}
}

void recipe_list_items (CHAR_DATA *ch, char *argument)
{
	RECIPE_DATA		*recipe_entry;
	OBJ_DATA		*obj;
	int				i;
	int				specific_item;
	char			buf [MAX_STRING_LENGTH];
	char			buf2 [MAX_STRING_LENGTH];
	int				count_found = 0;
	int				count;

	recipe_get_item_count (&argument, &count, &specific_item);

	for ( recipe_entry = recipe_list;
		  recipe_entry;
		  recipe_entry = recipe_entry->next ) {

		if ( specific_item && recipe_entry->obj_created != specific_item )
			continue;

		if ( count && ++count_found != count )
			continue;

		obj = vtoo (recipe_entry->obj_created);

		snprintf (buf, MAX_STRING_LENGTH,  "[%5d]:  %s\n",
			recipe_entry->obj_created,
			obj ? obj->short_description : "item UNDEFINED");
		send_to_char (buf, ch);

		for ( i = 0; i < 7; i++ ) {

			*buf2 = '\0';

			if ( recipe_entry->ingredient [i] &&
				 (obj = vtoo (recipe_entry->ingredient [i])) &&
				 obj->obj_flags.type_flag != ITEM_FOOD &&
				 obj->obj_flags.type_flag != ITEM_HERB &&
				 obj->obj_flags.type_flag != ITEM_COMPONENT &&
				 obj->obj_flags.type_flag != ITEM_POTION &&
				 obj->obj_flags.type_flag != ITEM_SALVE &&
				 !IS_SET (obj->obj_flags.extra_flags, ITEM_COMBINABLE) )
				strcpy (buf2, "[INVALID INGREDIENT] ");

			if ( recipe_entry->ingredient [i] ) {
				obj = vtoo (recipe_entry->ingredient [i]);
				snprintf (buf, MAX_STRING_LENGTH,  "       %5d:  %s%s%s\n",
					recipe_entry->ingredient [i],
					buf2,
					recipe_entry->reuse [i] ? "(reusable) " : "",
					obj ? obj->short_description : "incredient UNDEFINED");
				send_to_char (buf, ch);
			}
		}

		send_to_char ("\n", ch);

		if ( count )
			return;
	}
}

int get_recipe_item (char **p, int *ovnum, int *reuse)
{
	char	buf [MAX_STRING_LENGTH];

	*p = one_argument (*p, buf);

	if ( !*buf )
		return 0;

	if ( *buf == '*' ) {
		*reuse = 1;
		*ovnum = abs (strtol((buf + 1), NULL, 10));
	} else {
		*reuse = 0;
		*ovnum = abs (strtol(buf, NULL, 10));
	}

	return 1;
}

void recipe_add (CHAR_DATA *ch, char *argument)
{
	int				dummy, item;
	int				i;
	RECIPE_DATA		*recipe_entry;
	RECIPE_DATA		*alc_tmp;
	OBJ_DATA		*obj;
	char			buf [MAX_STRING_LENGTH];

	while ( *argument == ' ' )
		argument++;

	if ( *argument == '?' || !get_recipe_item (&argument, &item, &dummy) ) {
		send_to_char ("   recipe <object> [*]ingredient [*]ingredient ...\n",
					  ch);
		return;
	}

	CREATE (recipe_entry, RECIPE_DATA, 1);

	for ( i = 0; i < 6; i++ ) {

		while ( *argument == ' ' )
			argument++;

		if ( !*argument )
			break;

		get_recipe_item (&argument, &recipe_entry->ingredient [i],
									&recipe_entry->reuse [i]);
	}

	while ( *argument == ' ' )
		argument++;

	if ( i == 6 && *argument ) {
		send_to_char ("A recipe may have a maximum of seven ingredients.\n",
					  ch);
		mem_free (recipe_entry);
		return;
	}

	if ( !recipe_entry->ingredient [1] ) {
		send_to_char ("A recipe must have an item and at least two "
					  "ingredients.", ch);
		mem_free (recipe_entry);
		return;
	}

	recipe_entry->obj_created = item;

	sort_int_array (recipe_entry->ingredient, 7);

	recipe_entry->next = NULL;

	if ( !recipe_list )
		recipe_list = recipe_entry;
	else {
		alc_tmp = recipe_list;

		while ( alc_tmp->next )
			alc_tmp = alc_tmp->next;

		alc_tmp->next = recipe_entry;
	}

	if ( recipe_entry->obj_created && !vtoo (recipe_entry->obj_created) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "NOTE:  Object does not exists:  %d\n",
					recipe_entry->obj_created);
		send_to_char (buf, ch);
	}

	for ( i = 0; i < 7; i++ ) {

		if ( recipe_entry->ingredient [i] &&
			 !vtoo (recipe_entry->ingredient [i]) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "NOTE:  Ingredient does not exists:  %d\n",
						recipe_entry->ingredient [i]);
			send_to_char (buf, ch);
		}

		if ( recipe_entry->ingredient [i] &&
			 (obj = vtoo (recipe_entry->ingredient [i])) &&
			 obj->obj_flags.type_flag != ITEM_FOOD &&
			 obj->obj_flags.type_flag != ITEM_HERB &&
			 obj->obj_flags.type_flag != ITEM_COMPONENT &&
			 obj->obj_flags.type_flag != ITEM_POTION &&
			 obj->obj_flags.type_flag != ITEM_SALVE &&
			 !IS_SET (obj->obj_flags.extra_flags, ITEM_COMBINABLE) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "WARNING:  Ingredient is invalid type:  %d\n",
						recipe_entry->ingredient [i]);
			send_to_char (buf, ch);
		}
	}

	send_to_char ("Recipe added.\n", ch);
}

void recipe_delete (CHAR_DATA *ch, char *argument)
{
	char			buf [MAX_STRING_LENGTH];
	int				count;
	int				count_item_in_list = 0;
	int				item;
	RECIPE_DATA		*recipe_entry;
	RECIPE_DATA		*alc_tmp;

	recipe_get_item_count (&argument, &count, &item);

	if ( !item ) {
		send_to_char ("Syntax:  RECIPE DELETE [<count>.]<obj-vnum>\n\n"
					  "Example:  recipe delete 2.1023    or\n"
					  "          recipe delete 1023\n", ch);
		return;
	}

	for ( recipe_entry = recipe_list;
		  recipe_entry;
		  recipe_entry = recipe_entry->next ) 
		if ( item == recipe_entry->obj_created )
			count_item_in_list++;

	if ( count > count_item_in_list ) {
		send_to_char ("Sorry...not that many recipes for that item.\n", ch);
		return;
	}

	if ( !count && count_item_in_list > 1 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "There are %d of those item recipes.  To avoid "
					  "ambiguity, please use\n"
					  "the <count>.<vnum> notation.\n", count_item_in_list);
		send_to_char (buf, ch);
		return;
	}

	if ( !count )
		count++;

	for ( recipe_entry = recipe_list;
		  recipe_entry;
		  recipe_entry = recipe_entry->next )
		if ( item == recipe_entry->obj_created && !(--count) )
			break;

	if ( recipe_list == recipe_entry ) {
		recipe_list = recipe_entry->next;
		mem_free (recipe_entry);
		send_to_char ("Entry removed.\n", ch);
		return;
	}

	for ( alc_tmp = recipe_list; alc_tmp->next; alc_tmp = alc_tmp->next )
		if ( recipe_entry == alc_tmp->next ) {
			alc_tmp->next = recipe_entry->next;
			mem_free (recipe_entry);
			send_to_char ("Entry removed.\n", ch);
			return;
		}
}

void recipe_save (CHAR_DATA *ch)
{
	FILE			*fp;
	int				i;
	char			buf [MAX_STRING_LENGTH];
	RECIPE_DATA	*recipe_entry;

	if ( (fp = fopen (RECIPE_FILE, "w")) == NULL ) {
		return;
	}

	for ( recipe_entry = recipe_list;
		  recipe_entry;
		  recipe_entry = recipe_entry->next ) {

		snprintf (buf, MAX_STRING_LENGTH,  "#%d", recipe_entry->obj_created);

		for ( i = 0; i < 7; i++ )
			snprintf (ADDBUF, MAX_STRING_LENGTH, " %d",
					 recipe_entry->ingredient [i]);

		for ( i = 0; i < 7; i++ )
			snprintf (ADDBUF, MAX_STRING_LENGTH, " %d", recipe_entry->reuse [i]);

		strcat (buf, "\n");

		fprintf (fp, "%s", buf);
	}

	fprintf (fp, "#0\n");

	fclose (fp);
}

void do_recipe (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {
		recipe_help (ch);
		return;
	}

	if ( !str_cmp (buf, "list") )
		recipe_list_items (ch, argument);

	else if ( !str_cmp (buf, "add") )
		recipe_add (ch, argument);

	else if ( !str_cmp (buf, "delete") || !str_cmp (buf, "remove") )
		recipe_delete (ch, argument);

	else if ( !str_cmp (buf, "save") )
		recipe_save (ch);

	else
		send_to_char ("Syntax error.  Type 'recipe ?' for help.\n", ch);
}

#define TOK_WORD		0
#define	TOK_NEWLINE		1
#define TOK_PARAGRAPH		2
#define TOK_END			3
#define TOK_SENTENCE		4

void advance_spaces (char **s)
{
	while ( **s && (**s == ' ' || **s == '\t') )
		(*s)++;
}

int get_token (char **s, char *token)
{
	static int start_sentence = 0;

	if ( start_sentence ) {
		start_sentence = 0;
		return TOK_SENTENCE;
	}

	*token = '\0';

	if ( **s == '\n' )
		(*s)++;

	if ( !**s )
		return TOK_END;

	if ( **s == '\n' )
		return TOK_NEWLINE;

	if ( **s == ' ' ) {
		advance_spaces (s);
		return TOK_PARAGRAPH;
	}

	while ( **s && **s != ' ' && **s != '\t' && **s != '\n' ) {
		*token = **s;
		start_sentence = (**s == '.');
		token++;
		(*s)++;
	}

	*token = '\0';

	advance_spaces (s);

	return TOK_WORD;
}

void reformat_desc (char *source, char **target)
{
	int		token_value = 0;
	int		first_line = 1;
	int		line_len;
	char	*sou;
	char	token [MAX_STRING_LENGTH];
	char	result [MAX_STRING_LENGTH];
	char	buf [MAX_STRING_LENGTH];

	if ( source == NULL )
		return;
	
        if ( cmp_strn (source, "   ", 3) ) {
                snprintf (buf, MAX_STRING_LENGTH,  "   %s", source);
                mem_free (source);
                source = str_dup (buf);
        }

	sou = source;
	*result = '\0';

	line_len = 0;

	while ( token_value != TOK_END ) {

		token_value = get_token (&sou, token);

		if ( token_value == TOK_PARAGRAPH ) {

			if ( first_line )
				first_line = 0;
			else
				strcat (result, "\n");

			strcat (result, "   ");
			line_len = 3;
			continue;
		}

		if ( token_value == TOK_SENTENCE ) {
			line_len += 1;
			continue;
		}

		if ( token_value == TOK_NEWLINE ) {
			if ( line_len != 0 )
				strcat (result, "\n");		/* Catch up */
			strcat (result, "\n");
			line_len = 0;
			continue;
		}

		if ( token_value == TOK_WORD ) {
			if ( line_len + strlen (token) > MAX_LINE_LENGTH ) {
				strcat (result, "\n");
				line_len = 0;
			}

			strcat (result, token);
			strcat (result, " ");

			line_len += strlen (token) + 1;
		}
	}

	if ( result [strlen (result) - 1] != '\n' )
		strcat (result, "\n");

	*target = str_dup (result);
}

void reformat_string (char *source, char **target)
{
	int		token_value = 0;
	int		first_line = 1;
	int		line_len = 0;
	char	*sou = NULL;
	char	token [MAX_STRING_LENGTH] = {'\0'};
	char	result [MAX_STRING_LENGTH] = {'\0'};

	sou = source;

	while ( token_value != TOK_END ) {

		token_value = get_token (&sou, token);

		if ( token_value == TOK_PARAGRAPH ) {

			if ( first_line )
				first_line = 0;
			else
				strcat (result, "\r\n");

			strcat (result, "   ");
			line_len = 3;
			continue;
		}

		if ( token_value == TOK_SENTENCE ) {
			line_len += 1;
			continue;
		}

		if ( token_value == TOK_NEWLINE ) {
			if ( line_len != 0 )
				strcat (result, "\r\n");
			strcat (result, "\r\n");
			line_len = 0;
			continue;
		}

		if ( token_value == TOK_WORD ) {
			if ( (line_len + strlen (token) > 80) && !(*token == '#' && strlen(token) == 2) ) {
				strcat (result, "\r\n");
				line_len = 0;
			}

			strcat (result, token);
			strcat (result, " ");
			line_len += strlen (token) + 1;
		}
	}

	if ( result [strlen(result)-1] != '\n' )
		strcat (result, "\r\n");

	*target = str_dup (result);
	return;
}

int rdelete (ROOM_DATA *room)
{
	int			dir;
	ROOM_DATA	*troom;
	CHAR_DATA	*tch;

		/* NOTE:  This procedure creates a memory leak.  No biggie, since
                  only builders should be using this procedure, therefore
                  the mud is not live.
        */

	for ( tch = room->people; tch; tch = tch->next_in_room )
		if ( !IS_NPC (tch) )
			return 0;
		
	while ( room->affects )
		remove_room_affect (room, room->affects->type);

	while ( room->people )
		extract_char (room->people);

	while ( room->contents )
		extract_obj (room->contents);

	for ( troom = full_room_list; troom; troom = troom->lnext )
		for ( dir = 0; dir < 6; dir++ )
			if ( troom->dir_option [dir] &&
				 troom->dir_option [dir]->to_room == room->virtual )
				troom->dir_option [dir] = NULL;		/* MEMORY LEAK */

	room->virtual = -room->virtual;
	room->zone = -1;

	return 1;
}

void do_rdelete (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH];
	int			room_num;
	static int	room_del_warn = 0;
	ROOM_DATA	*temp_arg = NULL;

	argument = one_argument (argument, buf);

	if ( !isdigit (*buf) ) {
		send_to_char ("Expected a room virtual number.\n", ch);
		return;
	}

	room_num = strtol(buf, NULL, 10);

	if ( room_num == 0 || !vtor (room_num) ) {
		send_to_char ("No such room.\n", ch);
		return;
	}

	temp_arg = vtor (room_num);
	if ( !rdelete (temp_arg) ) {
		send_to_char ("ROOM NOT DELETED!\n", ch);
		return;
	} else {
		snprintf (buf, MAX_STRING_LENGTH,  "Room %d deleted.\n", room_num);
		send_to_char (buf, ch);

		if ( !room_del_warn ) {
			send_to_char ("WARNING:  It would be unwise to thaw the mud at this point.\n", ch);
			send_to_char ("          Save and reboot before a thawing.\n", ch);
			room_del_warn = 1;
		}
	}
}

void do_rmove (CHAR_DATA *ch, char *argument, int cmd)
{
	ROOM_DATA	*room;
	ROOM_DATA	*troom;
	CHAR_DATA	*tch;
	OBJ_DATA	*tobj;
	char		buf [MAX_STRING_LENGTH];
	int			dir;
	int			target_room_num;

	argument = one_argument (argument, buf);

	if ( !isdigit (*buf) || !strtol(buf, NULL, 10) || !(room = vtor (strtol(buf, NULL, 10))) ) {
		send_to_char ("Invalid source room.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !isdigit (*buf) || !strtol(buf, NULL, 10)) {
		send_to_char ("Invalid target room.\n", ch);
		return;
	}

	target_room_num = strtol(buf, NULL, 10);

	if ( vtor (target_room_num) ) {
		send_to_char ("Target room already exists.\n", ch);
		return;
	}

	if ( wld_tab [room->virtual % ZONE_SIZE] == room )
		wld_tab [room->virtual % ZONE_SIZE] = room->hnext;
	else {
		printf ("%d %d\n", (int) wld_tab [room->virtual % ZONE_SIZE], room->virtual % ZONE_SIZE);
		for ( troom = wld_tab [room->virtual % ZONE_SIZE];
			  troom;
			  troom = troom->hnext ) {

			printf ("%d v %d\n", troom->hnext->virtual, room->virtual);

			fflush (stdout);

			if ( troom->hnext->virtual == room->virtual )
				break;
		}

		troom->hnext = room->hnext;

		room->hnext = wld_tab [target_room_num % ZONE_SIZE];
		wld_tab [target_room_num % ZONE_SIZE] = room;
	}

	for ( tch = room->people; tch; tch = tch->next_in_room ) {

		tch->in_room = target_room_num;

		if ( IS_NPC (tch) )
			tch->mob->zone = target_room_num / ZONE_SIZE;
	}

	for ( tobj = room->contents; tobj; tobj = tobj->next_content ) {
		tobj->in_room = target_room_num;
		tobj->zone = target_room_num / ZONE_SIZE;
	}

		/* Relink rooms */

	for ( troom = full_room_list; troom; troom = troom->lnext ) {
		for ( dir = 0; dir < 6; dir++ ) {
			if ( troom->dir_option [dir] &&
				 troom->dir_option [dir]->to_room == room->virtual )
				troom->dir_option [dir]->to_room = target_room_num;
		}
	}

		/* I'm not bothering with resets, so that may cause some problems */

	room->virtual = target_room_num;
	room->zone = target_room_num / ZONE_SIZE;

	send_to_char ("Done.\n", ch);
}

void do_zset (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH];
	char		subcmd [MAX_STRING_LENGTH];
	int			zone;

	argument = one_argument (argument, buf);

	if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 99 ) {
		send_to_char ("Expected a zone number 0..99\n", ch);
		return;
	}

	zone = strtol(buf, NULL, 10);

	argument = one_argument (argument, subcmd);

	if ( !*subcmd ) {

		snprintf (buf, MAX_STRING_LENGTH,  "Zone:    [%2d]    %s\n", zone, zone_table [zone].name ?
					zone_table [zone].name :
					"Unnamed zone");
		send_to_char (buf, ch);

		snprintf (buf, MAX_STRING_LENGTH,  "Project Lead: %s\n", zone_table [zone].lead);
		send_to_char (buf, ch);

		snprintf (buf, MAX_STRING_LENGTH,  "Jailer:  [%5d] %s\n", zone_table [zone].jailer,
				 vtom (zone_table [zone].jailer) ?
					vtom (zone_table [zone].jailer)->short_descr :
					"Undefined vnum");
		send_to_char (buf, ch);

		snprintf (buf, MAX_STRING_LENGTH,  "Jail:    [%5d] %s\n", zone_table [zone].jail_room_num,
				zone_table [zone].jail_room ?
					zone_table [zone].jail_room->name :
					"Undefined room");
		send_to_char (buf, ch);

		return;
	}

	while ( *subcmd ) {

		argument = one_argument (argument, buf);

		if ( !str_cmp (subcmd, "?")){
		
			send_to_char ("Command	Value expected\n", ch);
			send_to_char ("-----------------------\n", ch);
			send_to_char ("name \t zone's name\n", ch);
			send_to_char ("lead \t staff character name\n", ch);
			send_to_char ("jailer \t Mob VNUM\n", ch);
			send_to_char ("jail \t Room VNUM\n", ch);
			return;
		}
		
		
		if ( !str_cmp (subcmd, "jailer") ) {

			if ( !isdigit (*buf) || !vtom (strtol(buf, NULL, 10)) ) {
				send_to_char ("That mobile could not be found.\n", ch);
				return;
			}

			zone_table [zone].jailer = strtol(buf, NULL, 10);
		}

		else if ( !str_cmp (subcmd, "jail") ) {

			if ( !isdigit (*buf) || !vtor (strtol(buf, NULL, 10)) ) {
				send_to_char ("That room could not be found.\n", ch);
				return;
			}

			zone_table [zone].jail_room_num = strtol(buf, NULL, 10);
			zone_table [zone].jail_room = vtor (strtol(buf, NULL, 10));
		}

		else if ( !str_cmp (subcmd, "name") ) {

			if ( !*buf ) {
				send_to_char ("Pick a name for your zone.\n", ch);
				return;
			}

			zone_table [zone].name = str_dup (buf);
		}

		else if ( !str_cmp (subcmd, "lead") ) {
			if ( GET_TRUST(ch) < 5 ) {
				send_to_char ("You cannot assign project leads.\n", ch);
				return;
			}
			else {
				zone_table [zone].lead = add_hash (CAP(buf));
				send_to_char ("Project lead changed.\n", ch);
				return;
			}
		}
		argument = one_argument (argument, subcmd);
	}
}

void do_mcopy (CHAR_DATA *ch, char *argument, int cmd)
{
	int			room_num;
	int			container_container = 0;
	CHAR_DATA	*mob;
	CHAR_DATA	*new_mob;
	OBJ_DATA	*obj;
	OBJ_DATA	*obj2;
	OBJ_DATA	*new_obj;
	OBJ_DATA	*new_obj2;
	char		buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {
		send_to_char ("mcopy [rvnum] [mob #.]mobname\n", ch);
		return;
	}

	room_num = ch->in_room;

	if ( isdigit (buf [strlen (buf) - 1]) ) {

		room_num = strtol(buf, NULL, 10);

		if ( room_num <= 0 || room_num > 99999 ) {
			send_to_char ("Illegal room number.\n", ch);
			return;
		}

		if ( !vtor (room_num) ) {
			send_to_char ("No such room.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);
	}

	if ( !(mob = get_char_room (buf, room_num)) ) {
		send_to_char ("No such mobile.\n", ch);
		return;
	}

	if ( !IS_NPC (mob) ) {
		send_to_char ("You specified a PC.\n", ch);
		return;
	}

	new_mob = load_mobile (mob->mob->virtual);

	new_mob->act = mob->act;

	for ( obj = mob->equip; obj; obj = obj->next_content ) {

		new_obj = load_object (obj->virtual);
		new_obj->count = obj->count;

		for ( obj2 = obj->contains; obj2; obj2 = obj2->next_content ) {

			new_obj2 = load_object (obj2->virtual);
			new_obj2->count = obj2->count;

			obj_to_obj (new_obj2, new_obj);

			if ( obj2->contains )
				container_container = 1;
		}

		equip_char (new_mob, new_obj, obj->location);
	}

	if ( container_container ) {
		container_container = 0;
		send_to_char ("NOTE:  Mob equiped a container containing a container with stuff.\n", ch);
		send_to_char ("       That 'stuff' was not copied.\n", ch);
	}

	if ( mob->right_hand ) {
		obj = mob->right_hand;
		new_obj = load_object (obj->virtual);
		new_obj->count = obj->count;
		
		for ( obj2 = obj->contains; obj2; obj2 = obj2->next_content ) {
			new_obj2 = load_object (obj2->virtual);
			new_obj2->count = obj2->count;
			obj_to_obj (new_obj2, new_obj);
			
			if ( obj2->contains )
				container_container = 1;
		}
		obj_to_char (new_obj, new_mob);
	}

	if ( mob->left_hand ) {
		obj = mob->left_hand;
		new_obj = load_object (obj->virtual);
		new_obj->count = obj->count;
		
		for ( obj2 = obj->contains; obj2; obj2 = obj2->next_content ) {
			new_obj2 = load_object (obj2->virtual);
			new_obj2->count = obj2->count;
			obj_to_obj (new_obj2, new_obj);
			
			if ( obj2->contains )
				container_container = 1;
		}
		
		obj_to_char (new_obj, new_mob);
	}

	if ( container_container ) {
		container_container = 0;
		send_to_char ("NOTE:  Mob inv with a container containing a container with stuff.\n", ch);
		send_to_char ("       That 'stuff' was not copied.\n", ch);
	}

	char_to_room (new_mob, ch->in_room);

	act ("$N copied and outfitted.", FALSE, ch, 0, new_mob, TO_CHAR);
}

void do_rset (CHAR_DATA *ch, char *argument, int cmd)
{
	int			ind;
	char		buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {
		send_to_char ("Type tags weather-room for possible descriptions.\n", ch);
		send_to_char ("\n\nSyntax:\n     rset <weather-room> [reformat]\n", ch);
		send_to_char ("     rset alas <direction>\n", ch);
		return;
	}

	if ( !str_cmp (buf, "alas") ) {

		argument = one_argument (argument, buf);

		if ( (ind = index_lookup (dirs, buf)) == -1 ) {
			send_to_char ("Expected north, south, east, west, up, or down.\n", ch);
			return;
		}

		if ( !ch->room->extra )
			CREATE (ch->room->extra, ROOM_EXTRA_DATA, 1);
		
		if ( ch->room->extra->alas [ind]) {
			send_to_char ("The alas description was:\n", ch);
			send_to_char (ch->room->extra->alas [ind], ch);
		}

		act ("$n begins editing an alas description.", FALSE, ch, 0, 0, TO_ROOM);

		ch->desc->str = &ch->room->extra->alas [ind];

			make_quiet (ch);
			send_to_char ("\nEnter a new description.  Terminate with an '@'\n", ch);
			ch->room->extra->alas [ind] = 0;
			ch->desc->max_str = 2000;
		

		return;
	}

	if ( (ind = index_lookup (weather_room, buf)) == -1 ) {
		send_to_char ("No such weather-room description.\n", ch);
		return;
	}

	if ( ind == WR_NORMAL ) {
		do_rdesc (ch, argument, 0);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "reformat") ) {

		if ( !ch->room->extra || !ch->room->extra->weather_desc [ind] ) {
			snprintf (buf, MAX_STRING_LENGTH,  "No current %s description.\n", weather_room [ind]);
			send_to_char (buf, ch);
			return;
		}

		reformat_desc (ch->room->extra->weather_desc [ind],
						 &ch->room->extra->weather_desc [ind]);
		send_to_char (ch->room->extra->weather_desc [ind], ch);

		return;
	}


	if ( !ch->room->extra ) {
		CREATE (ch->room->extra, ROOM_EXTRA_DATA, 1);
		
		printf ("Creating extra room data.\n");
		fflush (stdout);
	}

	if ( ch->room->extra->weather_desc [ind]) {
		send_to_char("The old description was: \n", ch);
		send_to_char(ch->room->extra->weather_desc [ind], ch);
	}

	act ("$n begins editing a room description.", FALSE, ch, 0, 0, TO_ROOM);

	ch->desc->str = &ch->room->extra->weather_desc [ind];

		send_to_char ("\nEnter a new description.  Terminate with an '@'\n", ch);
		send_to_char ("1-------10--------20--------30--------40--------50--------60---65\n", ch);	
		make_quiet (ch);
		ch->room->extra->weather_desc [ind] = 0;
		ch->desc->max_str = 2000;
	
}

void do_job (CHAR_DATA *ch, char *argument, int cmd)
{
	int				job;
	int				days = 0;
	int				pay_date;
	int				count = 0;
	int				ovnum = 0;
	int				employer = 0;
	int				cash = 0;
	CHAR_DATA		*edit_mob;
	AFFECTED_TYPE	*af;
	char			buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( *buf == '?' ) {
		s ("Start by using the mob command on a PC.");
		s ("");
		s ("job [1, 2, or 3]");
		s ("    pay <amount>              pay PC in cash by amount");
		s ("    days <pay-period-days>    # days between payday");
		s ("    employer <mob-vnum>       A mob pays PC instead of autopay");
        s ("    objects <count> <ovnum>   Pay PC a number of objects for pay");
        s ("    delete                    Delete the job");
		s ("");
		return;
	}

	if ( !(edit_mob = ch->pc->edit_player) ) {
		send_to_char ("Start by using the MOB command on a PC.\n", ch);
		return;
	}

	if ( strtol(buf, NULL, 10) < 1 || strtol(buf, NULL, 10) > 3 ) {
		send_to_char ("Specify job 1, 2 or 3.\n", ch);
		return;
	}

	job = strtol(buf, NULL, 10) + JOB_1 - 1;

	while ( *argument ) {

		argument = one_argument (argument, buf);

        if ( !str_cmp (buf, "delete") ) {
			if ( (af = get_affect (edit_mob, job)) ) {
		    	affect_remove (edit_mob, af);
				send_to_char ("Job removed.\n", ch);
				return;
			} else
				send_to_char ("That job wasn't assigned.\n", ch);
		}

		if ( !str_cmp (buf, "pay") || !str_cmp (buf, "money") ||
			 !str_cmp (buf, "cash") ) {

			argument = one_argument (argument, buf);
			cash = strtol(buf, NULL, 10);

			if ( cash < 0 )
				cash = 0;

			if ( cash > 500 )
				send_to_char ("That cash amount is a bit high.\n", ch);

			if ( cash > 10000 ) {
				send_to_char ("Cash > 10000.  Disallowed.\n", ch);
				return;
			}
		}

		else if ( !str_cmp (buf, "days") ) {

			argument = one_argument (argument, buf);

			days = strtol(buf, NULL, 10);

			if ( days < 0 )
				days = 0;
		}

		else if ( !str_cmp (buf, "employer") ) {

			argument = one_argument (argument, buf);

			employer = strtol(buf, NULL, 10);

			if ( !vtom (employer) ) {
				send_to_char ("Employer mob is not defined.\n", ch);
				return;
			}
		}

		else if ( !str_cmp (buf, "objects") ) {

			argument = one_argument (argument, buf);

			count = strtol(buf, NULL, 10);

			if ( count < 1 || count > 50 ) {
				send_to_char ("Count should be between 1 and 50.\n", ch);
				return;
			}

			argument = one_argument (argument, buf);

			ovnum = strtol(buf, NULL, 10);

			if ( !vtoo (ovnum) ) {
				send_to_char ("Specified object vnum doesn't exist.\n", ch);
				return;
			}
		}
	}

    if ( days <= 0 ) {
		send_to_char ("Don't forget to specify the number of days until payday.\n", ch);
		return;
	}

	if ( !count && !cash ) {
		send_to_char ("Make the payment in cash or some number of objects.\n", ch);
		return;
	}

	pay_date = time_info.month * 30 + time_info.day +
			   time_info.year * 12 * 30 + days;

	if ( (af = get_affect (edit_mob, job)) )
		affect_remove (edit_mob, af);

	job_add_affect (edit_mob, job, days, pay_date, cash, count, ovnum,
					employer);

	send_to_char ("Ok.\n", ch);
}

void do_mclone (CHAR_DATA *ch, char *argument, int cmd)
{
	int			vnum;
	CHAR_DATA	*newmob;
	CHAR_DATA	*source_mob;
	MOB_DATA	*m;
	char		buf [MAX_STRING_LENGTH];

	if ( !ch->pc ) {
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	if ( !(source_mob = vtom (ch->pc->edit_mob)) ) {
		send_to_char ("You must use 'mob' to target the source prototype.\n",
					  ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf || !strtol(buf, NULL, 10)) {
		send_to_char ("mclone <vnum>   - copy a mob prototype.\n", ch);
		return;
	}

	vnum = strtol(buf, NULL, 10);

	if ( vtom (vnum) ) {
		send_to_char ("That prototype already exists.\n", ch);
		return;
	}

	newmob = new_char (0);		/* MOB */

	m = newmob->mob;

	printf ("Copying mob prototype for %d chars\n", (int)sizeof (CHAR_DATA));
	fflush (stdout);
	memcpy (newmob, source_mob, sizeof (CHAR_DATA));
	printf ("Copying mob thingy for %d chars\n", (int)sizeof (MOB_DATA));
	fflush (stdout);
	memcpy (m, source_mob->mob, sizeof (MOB_DATA));
	printf ("All done.\n");  fflush (stdout);

	newmob->mob = m;

	newmob->mob->virtual = vnum;
	newmob->mob->zone = vnum / ZONE_SIZE;
	newmob->mob->lnext = NULL;
	newmob->mob->hnext = NULL;
	
	add_mob_to_hash (newmob);

	ch->pc->edit_mob = vnum;

	send_to_char ("Ok.\n", ch);
}

void list_mob_resets (CHAR_DATA *ch, CHAR_DATA *mob)
{
	int				reset_num = 0;
	RESET_DATA		*reset;
	char			buf [MAX_STRING_LENGTH];
	char			month [10];
	char			day [10];
	char			hour [10];

	if ( !mob || !mob->mob ) {
		send_to_char ("No mob or not an NPC.\n", ch);
		return;
	}

	if ( !mob->mob->resets ) {
		act ("$N has no resets.", FALSE, ch, 0, mob, TO_CHAR);
		return;
	}

	for ( reset = mob->mob->resets; reset; reset = reset->next ) {

		reset_num++;

		if ( reset->when.month == -1 )
			strcpy (month, "**");
		else
			snprintf (month, MAX_STRING_LENGTH, "%02d", reset->when.month);

		if ( reset->when.day == -1 )
			strcpy (day, "**");
		else
			snprintf (day, MAX_STRING_LENGTH, "%02d", reset->when.day);

		if ( reset->when.hour == -1 ) 
			strcpy (hour, "**");
		else
			snprintf (hour, MAX_STRING_LENGTH, "%02d", reset->when.hour);

		snprintf (buf, MAX_STRING_LENGTH,  "%2d) %s:%s:%s:%02d.%02d %s\n",
				reset_num,
				month,
				day,
				hour,
				reset->when.minute,
				reset->when.second,
				reset->command);
		send_to_char (buf, ch);
	}
}

void reset_insert (CHAR_DATA *ch, RESET_DATA *reset)
{
	RESET_DATA		*treset;
	RESET_DATA		*last_reset = NULL;

	for ( treset = ch->mob->resets;
		  treset;
		  last_reset = treset, treset = treset->next ) {

		if ( treset->when.month > reset->when.month )
			break;

		if ( treset->when.month < reset->when.month )
			continue;

		if ( treset->when.day > reset->when.day )
			break;

		if ( treset->when.day < reset->when.day )
			continue;

		if ( treset->when.hour > reset->when.hour )
			break;

		if ( treset->when.hour < reset->when.hour )
			continue;

		if ( treset->when.minute > reset->when.minute )
			break;

		if ( treset->when.minute < reset->when.minute )
			continue;
	}

	if ( !last_reset ) {
		reset->next = ch->mob->resets;
		ch->mob->resets = reset;
		return;
	}

	reset->next = last_reset->next;
	last_reset->next = reset;
}

int reset_delete (CHAR_DATA *mob, int reset_num)
{
	RESET_DATA	*reset;
	RESET_DATA	*free_reset;
	int			i = 2;

	if ( !mob->mob->resets )
		return 0;

	if ( reset_num == 1 ) {
		free_reset = mob->mob->resets;
		mob->mob->resets = free_reset->next;
		mem_free (free_reset);
		return 1;
	}

	for ( reset = mob->mob->resets; reset && i < reset_num; reset = reset->next)
		i++;

	if ( !reset || !reset->next )
		return 0;

	free_reset = reset->next;
	reset->next = free_reset->next;

	mem_free (free_reset);

	return 1;
}

void do_resets (CHAR_DATA *ch, char *argument, int cmd)
{
	int			month;
	int			day;
	int			hour;
	int			minute = 0;
	int			second;
	int			time_args;
	int			i1, i2, i3, i4;
	RESET_DATA	*reset;
	char		buf [MAX_STRING_LENGTH];

	if ( !ch->pc ) {
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	if ( !ch->pc->target_mob || !is_he_somewhere (ch->pc->target_mob) ) {
		send_to_char ("Use the 'mob' command to set target mobile receiving "
					  "the reset first.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		list_mob_resets (ch, ch->pc->target_mob);
		return;
	}

	if ( !strcmp (buf, "?") || !str_cmp (buf, "help") ) {
		send_to_char ("reset reply 'keywords' text\n", ch);
		send_to_char ("reset delete #\n", ch);
		send_to_char ("resets\n", ch);
		return;
	}

	if ( !strcmp (buf, "delete") ) {
		argument = one_argument (argument, buf);

		if ( !strtol(buf, NULL, 10)) {
			send_to_char ("Delete which numbered reset?\n", ch);
			return;
		}

		if ( reset_delete (ch->pc->target_mob, strtol(buf, NULL, 10)) )
			send_to_char ("Ok.\n", ch);
		else
			send_to_char ("No such reset.\n", ch);

		return;
	}

	if ( !strcmp (buf, "reply") ) {

		reset = (RESET_DATA *)alloc ((int)sizeof (RESET_DATA), 33);

		reset->type = RESET_REPLY;

		reset->command = str_dup (argument);

		reset->when.month  = -1;
		reset->when.day    = -1;
		reset->when.hour   = -1;
		reset->when.minute = -1;
		reset->when.second = -1;

		reset_insert (ch->pc->target_mob, reset);

		return;
	}

	time_args = sscanf (buf, "%d:%d:%d:%d", &i1, &i2, &i3, &i4);

	if ( !time_args ) {
		send_to_char ("Expected a time argument first:\n   MM:DD:HH:MM\n"
					  "   DD:HH:MM\n   HH:MM  or\n   MM\n", ch);
		return;
	}

	month  = -1;
	day    = -1;
    hour   = -1;
	second = 0;

	switch ( time_args ) {

		case 1:    minute = i1;
				   break;

		case 2:    hour = i1;
				   minute = i2;
				   break;

        case 3:    day = i1;
                   hour = i2;
                   minute = i3;
                   break;

		case 4:    month = i1;
                   day = i2;
                   hour = i3;
                   minute = i4;
                   break;
	}

	if ( month != -1 && month > 12 ) {
		send_to_char ("Months are 1 through 12.\n", ch);
		return;
	}

	if ( day != -1 && day > 30 ) {
		send_to_char ("There are only 30 days per month.\n", ch);
		return;
	}

	if ( hour != -1 && hour > 23 ) {
		send_to_char ("Hours are 0 through 23.\n", ch);
		return;
	}

	if ( minute > 59 ) {
		send_to_char ("Minutes are 0 though 59.\n", ch);
		return;
	}

	while ( isspace (*argument) )
		argument++;

	if ( !*argument ) {
		send_to_char ("What reset would you like to put on the mob?\n", ch);
		return;
	}

	reset = (RESET_DATA *)alloc ((int)sizeof (RESET_DATA), 33);

	reset->command = str_dup (argument);

	reset->type = RESET_TIMED;

	reset->when.month  = month;
	reset->when.day    = day;
    reset->when.hour   = hour;
    reset->when.minute = minute;
	reset->when.second = second;

	reset_insert (ch->pc->target_mob, reset);

	strcpy (buf, "Reset added to ");

	name_to_ident (ch->pc->target_mob, ADDBUF);

	send_to_char (buf, ch);
	send_to_char ("\n", ch);

	if ( ch->pc->target_mob->in_room != ch->in_room ) {
		snprintf (buf, MAX_STRING_LENGTH,  "NOTE:  Target mob is in room %d, not in yours.\n",
					ch->pc->target_mob->in_room);
		send_to_char (buf, ch);
	}
}



void do_instruct (CHAR_DATA *ch, char *argument, int cmd)
{
	int			i;
	int			ind;
	CHAR_DATA	*mob;
	char		buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {
		send_to_char ("See help.\n", ch);
		return;
	}

	if ( !(mob = get_char_room_vis (ch, buf)) ) {
		send_to_char ("No such person here.\n", ch);
		return;
	}

		
	if ( !(GET_TRUST (ch)) &&
		 (!IS_NPC (mob) ||
		  !IS_SET (mob->act, ACT_STAYPUT) ||
		  !is_leader (ch, mob))) {
		act ("You can't set flags on $M.", FALSE, ch, 0, mob, TO_CHAR);
		return;
	}

	if ( !IS_NPC (mob) ) {
		send_to_char ("Mobs only.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		if ( !GET_TRUST (ch) ) {
			if ( IS_SET (mob->act, ACT_SENTINEL) )
				send_to_char ("  Sentinel:    Yes.  The mob will not "
							  "wander.\n", ch);
			else
				send_to_char ("  Sentinel:    No.   The mob will "
							  "wander.\n", ch);

			if ( IS_SET (mob->act, ACT_SCAVENGER) )
				send_to_char ("  Scavenger:   Yes.  The mob will tend to pick "
							  "things up.\n", ch);
			else
				send_to_char ("  Scavenger:   No.   The mob will not pick "
							  "things up.\n", ch);

			if ( IS_SET (mob->act, ACT_AGGRESSIVE) )
				send_to_char ("  Aggressive:  Yes.  The mob will attack "
							  "non-allies.\n", ch);
			else
				send_to_char ("  Aggressive:  No.   The mob will not pick "
							  "fights.\n", ch);

			if ( IS_SET (mob->act, ACT_WIMPY) )
				send_to_char ("  Wimpy:       Yes.  The mob will flee at the "
							  "first sign of a fight.\n", ch);
			else
				send_to_char ("  Wimpy:       No.   The mob will stand and "
							  "fight until the death.\n", ch);

			if ( IS_SET (mob->act, ACT_PURSUE) )
				send_to_char ("  Pursue:      Yes.  The mob will follow "
							  "fleeing opponents.\n", ch);
			else
				send_to_char ("  Pursue:      No.   The mob will not follow "
							  "fleeing opponents.\n", ch);

			if ( IS_SET (mob->act, ACT_PASSIVE) )
				send_to_char ("  Passive:     Yes.  The mob will assist a clan brother in a fight.\n", ch);
			else
				send_to_char ("  Passive:     No.   The mob will not assist a clan brother in a fight.\n", ch);
		}

		else {
			for ( i = 0; *action_bits [i] != '\n'; i++ )
				if ( IS_SET (mob->act, 1 << i) )
					snprintf (ADDBUF, MAX_STRING_LENGTH, "%s ", action_bits [i]);

			send_to_char (buf, ch);
		}

		if ( IS_SET (mob->act, ACT_SENTINEL) )
			return;

		if ( !mob->mob->access_flags ) {
			act ("$N will wander anywhere.", FALSE, ch, 0, mob, TO_CHAR);
			return;
		}

		send_to_char ("\nMob wanders into rooms flagged as:\n\n   ", ch);

		for ( i = 0; *room_bits [i] != '\n'; i++ ) {
			if ( IS_SET (mob->mob->access_flags, 1 << i) ) {
				send_to_char (room_bits [i], ch);
				send_to_char (" ", ch);
			}
		}

		send_to_char ("\n", ch);

		return;
	}

	while ( *buf ) {

		if ( GET_TRUST (ch) ) {
			if ( (ind = index_lookup (room_bits, buf)) != -1 ) {
				TOGGLE (mob->mob->access_flags, 1 << ind);
			}

			else if ( (ind = index_lookup (action_bits, buf)) != -1 ) {
				TOGGLE (mob->act, 1 << ind);
			}

			else {
				send_to_char ("No such 'action-bits' or 'room-bits': ", ch);
				send_to_char (buf, ch);
				send_to_char ("\n", ch);
				return;
			}
		}

		else {
			if ( (ind = index_lookup (action_bits, buf)) != -1 &&
				  (1 << ind == ACT_AGGRESSIVE ||
				   1 << ind == ACT_PURSUE ||
				   1 << ind == ACT_SCAVENGER ||
				   1 << ind == ACT_SENTINEL ||
				   1 << ind == ACT_WIMPY ||
				   1 << ind == ACT_PASSIVE) ) {
				TOGGLE (mob->act, 1 << ind);
			}

			else if ( (ind = index_lookup (room_bits, buf)) != -1 ) {
				TOGGLE (mob->mob->access_flags, 1 << ind);
			}

			else {
				send_to_char (
				   "Expected a standing order of:\n\n"
                   "     Aggressive, Passive, Pursue, Scavenger, Sentinel, or Wimpy\n\n"
                   "or a access flag (rooms the mob will wonder into):\n\n"
                   "     Dark, Ruins, Indoors, Lawful, Tunnel, Cave, Deep, "
                   "Fall, Temple,\n"
                   "     Climb, Lab, Road, Wealthy, Poor, Scum, Market, "
                   "Arena, Dock, Wild,\n"
                   "     or Light\n", ch);
				return;
			}
		}

		argument = one_argument (argument, buf);
	}
}



