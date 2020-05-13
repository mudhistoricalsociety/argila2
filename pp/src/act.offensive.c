/** 
*	\file act.offensive.c
*	Offensive actions for players
*
*	This module provieds functionality for offensive activities, such as weapon
*	use, and archery, as well as guarding and fleeing. 
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
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

void do_throw (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*tobj = NULL;
	OBJ_DATA	*armor1 = NULL;
	OBJ_DATA	*armor2 = NULL;
	ROOM_DATA	*troom = NULL;
	ROOM_DIRECTION_DATA		*exit_room = NULL;
	CHAR_DATA	*tch = NULL;
	bool		can_lodge = FALSE;
	bool		ranged = FALSE;
	int			dir = 0;
	int			result = 0;
	int			location = 0;
	int			wear_loc1 = 0;
	int			wear_loc2 = 0;
	int			wound_type = 0;
	float		damage = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		buf3 [MAX_STRING_LENGTH] = {'\0'};
	char		smite_location [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	char		*temp_arg1 = NULL;
	char		*temp_arg2 = NULL;
	OBJ_DATA	*temp_obj = NULL;
	
        if ( IS_SWIMMING (ch) ) {
                send_to_char ("You can't do that while swimming!\n", ch);
                return;
        }

        if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
                send_to_char ("You cannot do this in an OOC area.\n", ch);
                return;
        }

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("What did you wish to throw?\n", ch);
		return;
	}

	if ( !(tobj = get_obj_in_list (buf, ch->right_hand)) &&
		!(tobj = get_obj_in_list (buf, ch->left_hand)) ) {
		send_to_char ("You aren't holding that in either hand.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("At what did you wish to throw?\n", ch);
		return;
	}

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		if ( (dir = is_direction (buf)) == -1 ) {
			send_to_char ("At what did you wish to throw?\n", ch);
			return;
		}
	}

	if ( ch->fighting ) {
		send_to_char ("You are currently engaged in melee combat and cannot throw.\n", ch);
		return;
	}

	if ( ch->balance <= -15 ) {
		send_to_char ("You're far too off-balance to attempt a throw.\n", ch);
		return;
	}

	if ( tobj->obj_flags.weight/100 > ch->str*2 ) {
		send_to_char ("That object is too heavy to throw effectively.\n", ch);
		return;
	}

	if ( tobj->count > 1 ) {
		send_to_char ("You may only throw one object at a time.\n", ch);
		return;
	}

	if ( !tch && dir != -1 ) {
		if ( (exit_room = PASSAGE (ch, dir)) )
			troom = vtor (PASSAGE (ch, dir)->to_room);

		if ( !troom ) {
			send_to_char ("There is no exit in that direction.\n", ch);
			return;
		}

		if ( exit_room && IS_SET (exit_room->exit_info, PASSAGE_CLOSED) ) {
			send_to_char ("Your view is blocked.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);

		if ( *buf ) {
			tch = get_char_room_vis2 (ch, troom->virtual, buf);
			if ( !has_been_sighted (ch, tch) )
				tch = NULL;
			if ( !tch ) {
				send_to_char ("You do not see anyone like that in this direction.\n", ch);
				return;
			}
		}

		if ( !tch ) {
			snprintf (buf, MAX_STRING_LENGTH,  "You hurl #2%s#0 %sward.", tobj->short_description, dirs[dir]);
			act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

			snprintf (buf, MAX_STRING_LENGTH,  "$n hurls #2%s#0 %sward.", tobj->short_description, dirs[dir]);
			act (buf, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

			obj_from_char (&tobj, 0);

			obj_to_room (tobj, troom->virtual);

			snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 flies in, hurled from %s.", tobj->short_description, verbose_dirs[rev_dir[dir]]);
			buf[2] = toupper(buf[2]);
			send_to_room (buf, troom->virtual);

			return;
		}
	}

	if ( tch ) {
		if ( tch == ch ) {
			send_to_char ("That wouldn't require much of a throw...\n", ch);
			return;
		}

		if ( ch->room != tch->room && dir != -1 ) {
			if ( (exit_room = PASSAGE (ch, dir)) )
				troom = vtor (PASSAGE (ch, dir)->to_room);

			if ( !troom ) {
				send_to_char ("There is no exit in that direction.\n", ch);
				return;
			}

			if ( exit_room && IS_SET (exit_room->exit_info, PASSAGE_CLOSED) ) {
				send_to_char ("Your view is blocked.\n", ch);
				return;
			}
		}

		result = calculate_missile_result (ch, SKILL_THROWN, ch->balance*-10, tch, 0, NULL, tobj, NULL, &location, &damage);
		damage = (int)(damage);

	        wear_loc1 = body_tab [0] [location].wear_loc1;
        	wear_loc2 = body_tab [0] [location].wear_loc2;
 
	        if ( wear_loc1 ) {   
        	        armor1 = get_equip (tch, wear_loc1);
                	if ( armor1 && GET_ITEM_TYPE (armor1) != ITEM_ARMOR )  
                        	armor1 = NULL;
        	}
        	if ( wear_loc2 ) {
                	armor2 = get_equip (tch, wear_loc2);
                	if ( armor2 && GET_ITEM_TYPE (armor2) != ITEM_ARMOR )
                        	armor2 = NULL;
        	}

		if ( damage > 3 )
			criminalize (ch, tch, tch->room->zone, CRIME_KILL);

		if ( ch->room != tch->room ) {
			temp_arg1 = char_short(tch);
			temp_arg2 = char_short (ch);
			snprintf (buf, MAX_STRING_LENGTH,  "You hurl #2%s#0 %sward, toward #5%s#0.", tobj->short_description, verbose_dirs[dir], temp_arg1);
			snprintf (buf2, MAX_STRING_LENGTH,  "#5%s#0 hurls #2%s#0 %sward, toward #5%s#0.", temp_arg2, tobj->short_description, verbose_dirs[dir], temp_arg1);
			buf2[2] = toupper(buf2[2]);
			act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			act (buf2, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		}
		else {
			temp_arg1 = char_short(tch);
			temp_arg2 = char_short(ch);
			snprintf (buf, MAX_STRING_LENGTH,  "You hurl #2%s#0 forcefully at #5%s#0.", tobj->short_description, temp_arg1);
			snprintf (buf2, MAX_STRING_LENGTH,  "#5%s#0 hurls #2%s#0 forcefully at #5%s#0.", temp_arg2, tobj->short_description, temp_arg1);
			snprintf (buf3, MAX_STRING_LENGTH,  "#5%s#0 hurls #2%s#0 forcefully at you.", temp_arg2, tobj->short_description);
			act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
			act (buf2, FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
			act (buf3, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
		}

		if ( GET_ITEM_TYPE (tobj) == ITEM_WEAPON && (tobj->o.weapon.hit_type == 0 ||
			tobj->o.weapon.hit_type == 1 || tobj->o.weapon.hit_type == 2 || tobj->o.weapon.hit_type == 4) ) {
			if ( result != CRITICAL_HIT && armor1 && armor1->o.armor.armor_type >= 2 )
				can_lodge = FALSE;
			else if ( result != CRITICAL_HIT && armor2 && armor2->o.armor.armor_type >= 2 )
				can_lodge = FALSE;
			else if ( result != CRITICAL_HIT && tch->armor && tch->armor >= 4 )
				can_lodge = FALSE;
			else can_lodge = TRUE;
		}
		
		temp_arg = figure_location (tch, location);
		snprintf (smite_location, MAX_STRING_LENGTH, "%s", temp_arg);


		if ( result == MISS ) {
			snprintf (buf, MAX_STRING_LENGTH,  "It misses completely.");
			snprintf (buf2, MAX_STRING_LENGTH,  "It misses completely.");
		}
		else if ( result == CRITICAL_MISS ) {
			snprintf (buf, MAX_STRING_LENGTH,  "It flies far wide of any target.");
			snprintf (buf2, MAX_STRING_LENGTH,  "It flies far wide of any target.");
		}
		else if ( result == SHIELD_BLOCK ) {
			temp_obj = get_equip(tch, WEAR_SHIELD);
			temp_arg = obj_short_desc(temp_obj);
			snprintf (buf, MAX_STRING_LENGTH,  "It glances harmlessly off #2%s#0.", temp_arg);
			snprintf (buf2, MAX_STRING_LENGTH,  "It glances harmlessly off #2%s#0.", temp_arg);
		}
		else if ( result == GLANCING_HIT ) {
			temp_arg = expand_wound_loc (smite_location);
			snprintf (buf, MAX_STRING_LENGTH,  "It grazes %s on the %s.", HMHR(tch), temp_arg);
			snprintf (buf2, MAX_STRING_LENGTH,  "It grazes you on the %s.", temp_arg);
			
		}
		else if ( result == HIT ) {
			if ( can_lodge ) {
				temp_arg = expand_wound_loc(smite_location);
				snprintf (buf, MAX_STRING_LENGTH,  "It lodges in %s %s.", HSHR(tch), temp_arg);
				snprintf (buf2, MAX_STRING_LENGTH,  "It lodges in your %s.", temp_arg);
			}
			else {
				temp_arg = expand_wound_loc(smite_location);
				snprintf (buf, MAX_STRING_LENGTH,  "It strikes %s on the %s.", HMHR(tch), temp_arg);
				snprintf (buf2, MAX_STRING_LENGTH,  "It strikes you on the %s.", temp_arg);
			}
		}
		else if ( result == CRITICAL_HIT ) {
			if ( can_lodge ) {
				temp_arg = expand_wound_loc(smite_location);
				snprintf (buf, MAX_STRING_LENGTH,  "It lodges deeply in %s %s!", HSHR(tch), temp_arg);
				snprintf (buf2, MAX_STRING_LENGTH,  "It lodges deeply in your %s!", temp_arg);
			}
			else {
				temp_arg = expand_wound_loc(smite_location);
				snprintf (buf, MAX_STRING_LENGTH,  "It strikes %s solidly on the %s.", HMHR(tch), temp_arg);
				snprintf (buf2, MAX_STRING_LENGTH,  "It strikes you solidly on the %s.", temp_arg);
			}
		}

		if ( ch->room != tch->room ) {
			send_to_room ("\n", ch->in_room);
			act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			send_to_room ("\n", tch->in_room);
			act (buf2, FALSE, tch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			act (buf, FALSE, tch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		}
		else {
			send_to_room ("\n", ch->in_room);
			act (buf, TRUE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
			act (buf2, TRUE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
			act (buf, TRUE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
		}

		obj_from_char (&tobj, 0);

		if ( (result == HIT || result == CRITICAL_HIT) && can_lodge ) {
			lodge_missile (tch, tobj, smite_location);
		}
		else obj_to_room (tobj, tch->in_room);

		if (damage > 0) {
         	       if ( !IS_NPC (tch) ) {
                	        tch->delay_ch = ch;
                        	tch->delay_info1 = tobj->virtual;
                	}
			if ( ch->room != tch->room )
				ranged = TRUE;
			if ( GET_ITEM_TYPE (tobj) == ITEM_WEAPON )
				wound_type = tobj->o.weapon.hit_type;
			else wound_type = 3;
                	if ( wound_to_char (tch, smite_location, (int)(damage), wound_type, 0, 0, 0) ) {
                        	if ( ranged )
                                	send_to_char ("\nYour target collapses, dead.\n", ch);
                        	ch->ranged_enemy = NULL; 
                        	return;
                	}
                	if ( !IS_NPC (tch) ) {
                        	tch->delay_ch = NULL;
                        	tch->delay_info1 = 0;
                	}
        	}

		if ( ch->agi <= 9 )
			ch->balance += -20;
		else if ( ch->agi > 9 && ch->agi <= 13 )
			ch->balance += -17;
		else if ( ch->agi > 13 && ch->agi <= 15 )
			ch->balance += -15;
		else if ( ch->agi > 15 && ch->agi <= 18 )
			ch->balance += -12;
		else ch->balance += -10;

		ch->balance = MAX (ch->balance, -50);

		npc_archery_retaliation (tch, ch);

		return;
	}

	send_to_char ("There has been an error; please report your command syntax to the staff.\n", ch);
}

void do_whirl (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*sling = NULL;

        if ( IS_SWIMMING (ch) ) {
                send_to_char ("You can't do that while swimming!\n", ch);
                return;
        }

	if ( ch->fighting ) {
		send_to_char ("You're fighting for your life!\n", ch);
		return;
	}

	if ( !get_equip (ch, WEAR_PRIM) ) {
		send_to_char ("You need to be wielding a loaded sling to use this command.\n", ch);
		return;
	}

	sling = get_equip (ch, WEAR_PRIM);

	if ( sling->o.weapon.use_skill != SKILL_SLING ) {
		send_to_char ("This command is for use with slings only.\n", ch);
		return;
	}

	if ( !sling->contains ) {
		send_to_char ("The sling needs to be loaded, first.\n", ch);
		return;
	}

	act ("You begin whirling $p, gathering momentum for a shot.", FALSE, ch, sling, 0, TO_CHAR | TO_ACT_FORMAT);
	act ("$n begins whirling $p, gathering momentum for a shot.", TRUE, ch, sling, 0, TO_ROOM | TO_ACT_FORMAT);

	ch->whirling = 1;
}

int projectile_shield_block (CHAR_DATA *ch, int result)
{
	OBJ_DATA	*shield_obj = NULL;
	int		roll = 0;

	if ( result == CRITICAL_HIT )
		return 0;

	if ( (shield_obj = get_equip (ch, WEAR_SHIELD)) && number(1,25) <= ch->dex ) {
		skill_use (ch, SKILL_BLOCK, 0);
		roll = number(1,85);
		if ( roll <= ch->skills [SKILL_BLOCK] ) {
			return 1;
		}
	}

	return 0;
}

int calculate_missile_result (CHAR_DATA *ch, int ch_skill, int att_modifier, CHAR_DATA *target, int def_modifier, OBJ_DATA *weapon, OBJ_DATA *missile, AFFECTED_TYPE *spell, int *location, float *damage)
{
	OBJ_DATA	*armor1 = NULL;
	OBJ_DATA	*armor2 = NULL;
	int			roll = 0;
	int			defense = 0;
	int			assault = 0;
	int			result = 0;
	int			wear_loc1 = 0;
	int			wear_loc2 = 0;
	int			body_type = 0;

	/* Determine result of hit attempt. */

	if ( !CAN_SEE (target, ch) )
		def_modifier -= number (15,30);

	skill_use (target, SKILL_DODGE, 0);

	roll = number(1,85);
	roll += def_modifier;
	roll = MIN (roll, 85);

	if ( roll > target->skills [SKILL_DODGE] ) {
		if ( roll % 5 == 0 || roll == 1 )
			defense = RESULT_CF;
		else
			defense = RESULT_MF;
	}
	else {
		if ( roll % 5 == 0 )
			defense = RESULT_CS;
		else
			defense = RESULT_MS;
	}

	skill_use (ch, ch_skill, 0);

        if ( weather_info[ch->room->zone].fog == THIN_FOG )   
                att_modifier += 5;
        if ( weather_info[ch->room->zone].fog == THICK_FOG )
                att_modifier += 10;
        if ( weather_info[ch->room->zone].state == STEADY_RAIN )
                att_modifier += 15;
        if ( weather_info[ch->room->zone].state == HEAVY_RAIN )
                att_modifier += 20;
        if ( weather_info[ch->room->zone].state == STEADY_SNOW )
                att_modifier += 25;

	roll = number(1,85);
	roll += att_modifier;
	roll = MIN (roll, 85);
 
	ch->aim = 0;

	if ( ch->skills [ch_skill] ) {
		if ( roll > ch->skills [ch_skill] ) {
			if ( roll % 5 == 0 || roll == 1 )
				assault = RESULT_CF;
			else
				assault = RESULT_MF;
		}
		else if ( roll <= ch->skills [ch_skill] ) {
			if ( roll % 5 == 0 )
				assault = RESULT_CS;
			else
				assault = RESULT_MS;
		}
	}
	else {
		if ( roll > ch->skills [SKILL_OFFENSE] ) {
			if ( roll % 5 == 0 || roll == 1 )
				assault = RESULT_CF;
			else
				assault = RESULT_MF;
		}
		else if ( roll <= ch->skills [SKILL_OFFENSE] ) {
			if ( roll % 5 == 0 )
				assault = RESULT_CS;
			else
				assault = RESULT_MS;
		}
	}

	if ( assault == defense ) {
		if ( number(1,10) > 5 )
			result = GLANCING_HIT;
		else
			result = MISS;
	}
	if ( assault == RESULT_CS ) {
		if ( defense == RESULT_MS )
			result = HIT;
		else if ( defense == RESULT_MF )
			result = HIT;
		else if ( defense == RESULT_CF )
			result = CRITICAL_HIT;
	}
	else if ( assault == RESULT_MS ) {
		if ( defense == RESULT_CS )
			result = MISS;
		else if ( defense == RESULT_MF )
			result = HIT;
		else if ( defense == RESULT_CF )
			result = CRITICAL_HIT;
	}
	else if ( assault == RESULT_MF ) {
		if ( defense == RESULT_CS )
			result = CRITICAL_MISS;
		else if ( defense == RESULT_MS )
			result = MISS;
		else if ( defense == RESULT_CF )
			result = GLANCING_HIT;
	}
	else if ( assault == RESULT_CF ) {
		if ( defense == RESULT_CS )
			result = CRITICAL_MISS;
		else if ( defense == RESULT_MS )
			result = CRITICAL_MISS;
		else if ( defense == RESULT_MF )
			result = MISS;
	}

        if ( !AWAKE (target) && assault != RESULT_CF )
                result = CRITICAL_HIT;
        
        if ( (result == HIT || result == GLANCING_HIT) && projectile_shield_block (target, result) )
                result = SHIELD_BLOCK;
 
	if ( result == MISS || result == CRITICAL_MISS || result == SHIELD_BLOCK )
		return result;

	/* Determine damage of hit, if applicable. */

	*damage = 0;

        body_type = 0;

        roll = number (1, 100);
        *location = -1;

        while ( roll > 0 )
                roll = roll - body_tab [body_type] [++(*location)].percent;  

        wear_loc1 = body_tab [body_type] [*location].wear_loc1;
        wear_loc2 = body_tab [body_type] [*location].wear_loc2;

	if ( wear_loc1 ) {
		armor1 = get_equip (target, wear_loc1);
		if ( armor1 && GET_ITEM_TYPE (armor1) != ITEM_ARMOR )
			armor1 = NULL;
	}
	if ( wear_loc2 ) {
		armor2 = get_equip (target, wear_loc2);
		if ( armor2 && GET_ITEM_TYPE (armor2) != ITEM_ARMOR )
			armor2 = NULL;
	}

	if ( missile ) {
		if ( GET_ITEM_TYPE (missile) == ITEM_MISSILE || GET_ITEM_TYPE (missile) == ITEM_BULLET )
			*damage = (float)dice(missile->o.od.value[0], missile->o.od.value[1]);
		else if ( GET_ITEM_TYPE (missile) == ITEM_WEAPON )
			*damage = (float)dice(missile->o.od.value[1], missile->o.od.value[2]);
		else *damage = (float)dice(1,3);

		if ( weapon ) 			/* Launched from a bow/sling */
			*damage += weapon->o.od.value[4];

		if ( weapon || IS_SET (missile->obj_flags.extra_flags, ITEM_THROWING) ) {
			if ( result == HIT ) {			/* Multiply in critical hits to raw damage amount. */
				*damage += number(3,5);
				*damage *= 1.75;
			}
			else if ( result == CRITICAL_HIT ) {
				*damage += number(4,6);
				*damage *= 2.75;
			}

		}

		if ( armor1 != NULL ) {
			*damage -= armor1->o.armor.armor_value;
			*damage += weapon_armor_table [1][armor1->o.armor.armor_type];
		}
		if ( armor2 != NULL ) {
			*damage -= armor2->o.armor.armor_value;
			*damage += weapon_armor_table [1][armor2->o.armor.armor_type];
		}
		if ( target->armor ) {
			*damage -= target->armor;
		}

		if ( weapon ) {
			if ( result != GLANCING_HIT) {		/* Shot lodged in target, piercing the armor. */
				if ( *location == 2 )
					*damage *= 1.75;
				else if ( *location == 3 )
					*damage *= 2.25;
				else if ( *location == 4 )
					*damage *= 3.00;
			}
		}

		if ( missile ) {
			if ( GET_ITEM_TYPE (missile) != ITEM_WEAPON && GET_ITEM_TYPE (missile) != ITEM_MISSILE && GET_ITEM_TYPE (missile) != ITEM_BULLET )
				*damage = 0;
		}
	}

	if ( weapon && weapon->o.weapon.use_skill == SKILL_SLING ) {
		if ( att_modifier < 0 )
			att_modifier *= -1;
		if ( att_modifier <= 1 )
			*damage *= 0.250;
		else if ( att_modifier > 1 && att_modifier <= 2 )
			*damage *= 0.500;
		else if ( att_modifier > 2 && att_modifier <= 3 )
			*damage *= 0.750;
		
	}

	return result;
}

void lodge_missile (CHAR_DATA *target, OBJ_DATA *ammo, char *smite_location)
{
	LODGED_OBJECT_INFO	*lodged = NULL;

	if ( !target->lodged ) {
		CREATE (target->lodged, LODGED_OBJECT_INFO, 1);
		target->lodged->vnum = ammo->virtual;
		target->lodged->location = add_hash (smite_location);
	}
	else for ( lodged = target->lodged; lodged; lodged = lodged->next ) {
		if ( !lodged->next ) {
			CREATE (lodged->next, LODGED_OBJECT_INFO, 1);
			lodged->next->vnum = ammo->virtual;
			lodged->next->location = add_hash (smite_location);
			break;
		}
		else continue;
	}
}

void fire_sling (CHAR_DATA *ch, OBJ_DATA *sling, char *argument)
{
	CHAR_DATA	*tch = NULL;
	ROOM_DATA	*troom = NULL;
	OBJ_DATA	*ammo = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		smite_location [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		buf3 [MAX_STRING_LENGTH] = {'\0'};
	float		damage = 0;
	int			dir = 0;
	int			location = 0;
	int			result = 0;
	int			attack_mod = 0;
	int			wound_type = 0;
	bool		ranged = FALSE;
	char		*temp_arg = NULL;
	char		*temp_arg1 = NULL;
	char		*temp_arg2 = NULL;
	OBJ_DATA	*temp_obj = NULL;
	
	if ( !ch || !sling )
		return;

	if ( !*argument ) {
		send_to_char ("At what did you wish to fire your sling?\n", ch);
		return;
	}

	if ( !ch->whirling ) {
		send_to_char ("You'll need to begin whirling your sling before you can fire.\n", ch);
		return;
	}

	if ( !sling->contains ) {
		send_to_char ("You'll need to load your sling first.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

        if ( !(tch = get_char_room_vis (ch, buf)) ) {
                if ( (dir = is_direction (buf)) == -1 ) {
                        send_to_char ("At what did you wish to fire?\n", ch);
                        return;
                }
        }        
                                        
        if ( !tch && dir != -1 ) {
                if ( PASSAGE (ch, dir) )
                        troom = vtor (PASSAGE (ch, dir)->to_room);
                        
                if ( !troom ) {
                        send_to_char ("There is no exit in that direction.\n", ch);
                        return;
                }
                        
                argument = one_argument (argument, buf);
                 
                if ( *buf ) {
                        tch = get_char_room_vis2 (ch, troom->virtual, buf);
                        if ( !has_been_sighted (ch, tch) )
                                tch = NULL;
                        if ( !tch ) {
                                send_to_char ("You do not see anyone like that in this direction.\n", ch);
                                return;
                        }
                }
	}

	if ( !tch ) {
		send_to_char ("At whom did you wish to fire your sling?\n", ch);
		return;
	}

	if ( tch == ch ) {
		send_to_char ("Don't be silly.\n", ch);
		return;
	}

	attack_mod -= ch->whirling;
	ch->whirling = 0;

	result = calculate_missile_result (ch, sling->o.weapon.use_skill, attack_mod, tch, 0, sling, sling->contains, NULL, &location, &damage);
	damage = (int)(damage);

	temp_arg = figure_location (tch, location);
	snprintf (smite_location, MAX_STRING_LENGTH, "%s", temp_arg);


	if ( ch->room != tch->room ) {
		temp_arg1 = char_short(tch);
		temp_arg2 = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "You sling #2%s#0 %sward, toward #5%s#0.", sling->contains->short_description, verbose_dirs[dir], temp_arg1);
		snprintf (buf2, MAX_STRING_LENGTH,  "#5%s#0 slings #2%s#0 %sward, toward #5%s#0.", temp_arg2, sling->contains->short_description, verbose_dirs[dir], temp_arg1);
		buf2[2] = toupper(buf2[2]);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act (buf2, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	}
	else {
		temp_arg1 = char_short(tch);
		temp_arg2 = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "You sling #2%s#0 forcefully at #5%s#0.", sling->contains->short_description, temp_arg1);
		snprintf (buf2, MAX_STRING_LENGTH,  "#5%s#0 slings #2%s#0 forcefully at #5%s#0.", temp_arg2, sling->contains->short_description, temp_arg1);
		snprintf (buf3, MAX_STRING_LENGTH,  "#5%s#0 slings #2%s#0 forcefully at you.", temp_arg2, sling->contains->short_description);
		act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
		act (buf, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
		act (buf2, FALSE, ch, 0, tch, TO_NOTVICT| TO_ACT_FORMAT);
	}

	if ( result == MISS ) {
		snprintf (buf, MAX_STRING_LENGTH,  "It misses completely.");
		snprintf (buf2, MAX_STRING_LENGTH,  "It misses completely.");
	}
	else if ( result == CRITICAL_MISS ) {
		snprintf (buf, MAX_STRING_LENGTH,  "It flies far wide of any target.");
		snprintf (buf2, MAX_STRING_LENGTH,  "It flies far wide of any target.");
	}
	else if ( result == SHIELD_BLOCK ) {
		temp_obj = get_equip(tch, WEAR_SHIELD);
		temp_arg = obj_short_desc(temp_obj);
		snprintf (buf, MAX_STRING_LENGTH,  "It glances harmlessly off #2%s#0.", temp_arg);
		snprintf (buf2, MAX_STRING_LENGTH,  "It glances harmlessly off #2%s#0.", temp_arg);
	}
	else if ( result == GLANCING_HIT ) {
		temp_arg = expand_wound_loc (smite_location);
		snprintf (buf, MAX_STRING_LENGTH,  "It grazes %s on the %s.", HMHR(tch), temp_arg);
		snprintf (buf2, MAX_STRING_LENGTH,  "It grazes you on the %s.", temp_arg);	
	}
	else if ( result == HIT ) {
		temp_arg = expand_wound_loc (smite_location);
		snprintf (buf, MAX_STRING_LENGTH,  "It strikes %s on the %s.", HMHR(tch), temp_arg);
		snprintf (buf2, MAX_STRING_LENGTH,  "It strikes you on the %s.", temp_arg);
	}
	else if ( result == CRITICAL_HIT ) {
		temp_arg = expand_wound_loc (smite_location);
		snprintf (buf, MAX_STRING_LENGTH,  "It strikes %s solidly on the %s.", HMHR(tch), temp_arg);
		snprintf (buf2, MAX_STRING_LENGTH,  "It strikes you solidly on the %s.", temp_arg);
	}

	ammo = sling->contains;

	obj_from_obj (&ammo, 0);
	obj_to_room (ammo, tch->in_room);

	if ( result == CRITICAL_MISS && !number(0,1) ) {
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n\nThe missile recedes hopelessly from sight.");
		snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "\n\nThe missile recedes hopelessly from sight.");
		snprintf (buf3 + strlen(buf3), MAX_STRING_LENGTH, "\n\nThe missile recedes hopelessly from sight.");
		extract_obj(ammo);
	}

	if ( ch->room != tch->room ) {
		send_to_room ("\n", ch->in_room);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		send_to_room ("\n", tch->in_room);
		act (buf2, FALSE, tch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act (buf, FALSE, tch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	}
	else {
		send_to_room ("\n", ch->in_room);
		act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
		act (buf2, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
		act (buf, FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
	}

	if (damage > 0) {
                if ( !IS_NPC (tch) ) {
              	        tch->delay_ch = ch;
                     	tch->delay_info1 = ammo->virtual;
               	}
		if ( ch->room != tch->room )
			ranged = TRUE;
		wound_type = 3;
               	if ( wound_to_char (tch, smite_location, (int)damage, wound_type, 0, 0, 0) ) {
                       	if ( ranged )
                               	send_to_char ("\nYour target collapses, dead.\n", ch);
                       	ch->ranged_enemy = NULL; 
                       	return;
               	}
               	if ( !IS_NPC (tch) ) {
                       	tch->delay_ch = NULL;
                       	tch->delay_info1 = 0;
               	}
		if ( damage > 3 )
			criminalize (ch, tch, tch->room->zone, CRIME_KILL);
        }

	npc_archery_retaliation (tch, ch);

}

void do_fire (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*target = NULL;
	CHAR_DATA	*tch = NULL;
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*ammo = NULL;
	OBJ_DATA	*bow = NULL;
	OBJ_DATA	*armor1 = NULL;
	OBJ_DATA	*armor2 = NULL;
	OBJ_DATA	*shield_obj = NULL;
	ROOM_DATA	*room = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		buf3 [MAX_STRING_LENGTH] = {'\0'};
	char		buffer [MAX_STRING_LENGTH] = {'\0'};
	char		smite_location [MAX_STRING_LENGTH] = {'\0'};
	char		*from_direction = NULL;
	char		*p = NULL;
	char		*temp_arg = NULL;
	char		*temp_arg1 = NULL;
	char		*temp_arg2 = NULL;
	int			roll = 0;
	int			ranged = 0;
	int			result = 0;
	int			location = 0;
	int			attack_mod = 0;
	float		damage = 0;
	int			dir = 0;
	bool		switched_target = FALSE;
	
    if ( IS_SWIMMING (ch) ) {
    	send_to_char ("You can't do that while swimming!\n", ch);
        return;
    }

	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}
	
	if ( ((bow = get_equip(ch, WEAR_PRIM)) && GET_ITEM_TYPE(bow) == ITEM_WEAPON && bow->o.weapon.use_skill == SKILL_SLING) ||
		((bow = get_equip (ch, WEAR_SEC)) && GET_ITEM_TYPE(bow) == ITEM_WEAPON && bow->o.weapon.use_skill == SKILL_SLING) ) {
		fire_sling (ch, bow, argument);
		return;
	}

	if ( !ch->aiming_at ) {
		send_to_char ("You aren't aiming at anything.\n", ch);
		return;
	}

	if ( !IS_NPC (ch->aiming_at) &&
		ch->aiming_at->pc->create_state == STATE_DIED ) {
		send_to_char ("You aren't aiming at anything alive.\n", ch);
		return;
	}

	if ( !(bow = get_equip (ch, WEAR_BOTH)) ) {
		send_to_char ("You aren't wielding a ranged weapon!\n", ch);
		ch->aiming_at->targeted_by = NULL;
		ch->aiming_at = NULL;
		ch->aim = 0;
		return;
	}

	if ( !bow->loaded ) {
		send_to_char ("Your weapon isn't loaded...\n", ch);
		ch->aiming_at->targeted_by = NULL;
		ch->aiming_at = NULL;
		ch->aim = 0;
		return;
	}

    if ( IS_SET (ch->plr_flags, NEW_PLAYER_TAG) &&
    	IS_SET (ch->room->room_flags, LAWFUL) &&
    	*argument != '!' ) {
        snprintf (buf, MAX_STRING_LENGTH,  "You are in a lawful area; you would likely be flagged wanted for assault. "
        	"To confirm, type \'#6fire !#0\', without the quotes.");
        act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
    }

	attack_mod -= ch->aim;
	target = ch->aiming_at;
	target->targeted_by = NULL;
	ch->aiming_at = NULL;
	ch->aim = 0;

	if ( !IS_NPC (target) &&
		(!str_cmp (target->tname, IMPLEMENTOR_ACCOUNT)) &&
		!IS_NPC (ch) ){
		target = ch;
	}

	if ( ch->in_room != target->in_room ){
		ranged = 1;
	}

	if ( ranged ) {
		if ( !ch->delay_who ) {
			send_to_char ("You seem to have lost sight of your quarry.\n", ch);
			return;
		}
        
        if ( !cmp_strn ("north", ch->delay_who, strlen(ch->delay_who)) )
        	dir = 0;
        else if ( !cmp_strn ("east", ch->delay_who, strlen(ch->delay_who)) )
        	dir = 1;
        else if ( !cmp_strn ("south", ch->delay_who, strlen(ch->delay_who)) )
        	dir = 2;
        else if ( !cmp_strn("west", ch->delay_who, strlen(ch->delay_who)) )
        	dir = 3;
        else if ( !cmp_strn ("up", ch->delay_who, strlen(ch->delay_who)) )
        	dir = 4;
        else if ( !cmp_strn ("down", ch->delay_who, strlen(ch->delay_who)) )
        	dir = 5;
	}

	/* ranged_projectile_echoes (ch, target, bow, ammo); */

	if ( bow->o.weapon.use_skill == SKILL_SHORTBOW ||
		bow->o.weapon.use_skill == SKILL_LONGBOW ) {
		
		if ( ranged ) {
			temp_arg = char_short (target);
			temp_arg1 = char_short(ch);
			snprintf (buf, MAX_STRING_LENGTH,  "You release the bowstring, launching #2%s#0 %sward through the air toward #5%s#0.", bow->loaded->short_description, ch->delay_who, temp_arg );	
			snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 releases the bowstring, launching #2%s#0 %sward through the air toward #5%s#0.", temp_arg1, bow->loaded->short_description, ch->delay_who, temp_arg); 
			*buf2 = toupper (*buf2);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf2);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s", buffer);
		}
		
		else {
			temp_arg = char_short(ch);
			temp_arg1 = char_short(target);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 releases the bowstring, launching #2%s#0 through the air toward #5%s#0.", temp_arg, bow->loaded->short_description, temp_arg1);
			*buf2 = toupper (*buf2);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf2);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s", buffer);
			snprintf (buf3, MAX_STRING_LENGTH,  "%s#0 releases the bowstring, launching #2%s#0 through the air toward you!", temp_arg, bow->loaded->short_description);
			*buf3 = toupper (*buf3);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf3);
			snprintf (buf3, MAX_STRING_LENGTH,  "%s", buffer);
			snprintf (buf, MAX_STRING_LENGTH,  "You release the bowstring, launching #2%s#0 through the air toward #5%s#0!", bow->loaded->short_description, temp_arg1);
		}
	}
	
	else if ( bow->o.weapon.use_skill == SKILL_CROSSBOW ) {
		if ( ranged ) {
			temp_arg = char_short(ch);
			temp_arg1 = char_short(target);
			snprintf (buf, MAX_STRING_LENGTH,  "You squeeze the trigger, sending #2%s#0 whirring %sward through the air toward #5%s#0.", bow->loaded->short_description, ch->delay_who, temp_arg1);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 squeezes the trigger, sending #2%s#0 whirring %sward through the air toward #5%s#0.", temp_arg, bow->loaded->short_description, ch->delay_who, temp_arg1);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s#0", CAP(buf2));
			snprintf (buf2, MAX_STRING_LENGTH,  "%s", buffer);
		}
		
		else {
			temp_arg = char_short(ch);
			temp_arg1 = char_short(target);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 squeezes the trigger, sending #2%s#0 whirring through the air toward #5%s#0.", temp_arg, bow->loaded->short_description, temp_arg1);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", CAP(buf2));
			snprintf (buf2, MAX_STRING_LENGTH,  "%s", buffer);
			snprintf (buf3, MAX_STRING_LENGTH,  "%s#0 squeezes the trigger, sending #2%s#0 whirring through the air toward you!", temp_arg, bow->loaded->short_description);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", CAP(buf3));
			snprintf (buf3, MAX_STRING_LENGTH,  "%s", buffer);
			snprintf (buf, MAX_STRING_LENGTH,  "You squeeze the trigger, sending #2%s#0 whirring through the air toward #5%s#0!", bow->loaded->short_description, temp_arg1);
		}
	}

	if ( ranged ) {
		act (buf, TRUE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act (buf2, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		snprintf (buf3, MAX_STRING_LENGTH,  "You spy #2%s#0 as it arcs high overhead, streaking %sward.", bow->loaded->short_description, ch->delay_who);
		reformat_string (buf3, &p);
		
		if ( PASSAGE(ch,dir) )
			room = vtor (PASSAGE(ch,dir)->to_room);
		else
			room = NULL;
		
		if ( room && target->room != room ) {
			for ( tch = room->people; tch; tch = tch->next_in_room ){ 
				if ( skill_use (tch, SKILL_SCAN, 0) ){
					send_to_char (p, tch);
				}
			}
			
			if ( room->dir_option[dir] ){	
				room = vtor (room->dir_option[dir]->to_room);
			}
			else {
				room = NULL;
			}
			
			if ( room && target->room != room ) {
				for ( tch = room->people; tch; tch = tch->next_in_room ){ 
					if ( skill_use (tch, SKILL_SCAN, 0) ){
						send_to_char (p, tch);
					}
				}
				
				if ( room->dir_option[dir] ){	
					room = vtor (room->dir_option[dir]->to_room);
				}
				else{
					room = NULL;
				}
				
				if ( room && target->room != room ) {
					for ( tch = room->people; tch; tch = tch->next_in_room ){ 
						if ( skill_use (tch, SKILL_SCAN, 0) ){
							send_to_char (p, tch);
						}
					}
				}
			}
		}
		mem_free (p);
	}
	
	else if ( !ranged ) {
		act (buf, TRUE, ch, 0, target, TO_CHAR | TO_ACT_FORMAT);
		act (buf3, TRUE, ch, 0, target, TO_VICT | TO_ACT_FORMAT);
		act (buf2, TRUE, ch, 0, target, TO_NOTVICT | TO_ACT_FORMAT);
	}

	ammo = load_object(bow->loaded->virtual);
	ammo->count = 1;
	ammo->in_obj = NULL;
	ammo->carried_by = NULL;
	ammo->equiped_by = NULL;

	if ( ranged && ch->delay_who ) {
		if ( !str_cmp (ch->delay_who, "north") )
			from_direction = str_dup ("the south");
		else if ( !str_cmp (ch->delay_who, "east") )
			from_direction = str_dup ("the west");
		else if ( !str_cmp (ch->delay_who, "south") )
			from_direction = str_dup ("the north");
		else if ( !str_cmp (ch->delay_who, "west") )
			from_direction = str_dup ("the east");
		else if ( !str_cmp (ch->delay_who, "up") )
			from_direction = str_dup ("below");
		else if ( !str_cmp (ch->delay_who, "down") )
			from_direction = str_dup ("above");
		if ( ch->delay_who && strlen(ch->delay_who) > 1 )
			mem_free (ch->delay_who);
		ch->delay_who = NULL;
	}
	else{
		from_direction = add_hash ("an indeterminate direction");
	}
	
	attack_mod -= bow->o.od.value[2];

	if ( ch->fighting ) {
		if ( bow->o.weapon.use_skill == SKILL_LONGBOW ) 
			attack_mod += 30;
		else if ( bow->o.weapon.use_skill == SKILL_SHORTBOW )
			attack_mod += 20;
		else if ( bow->o.weapon.use_skill == SKILL_CROSSBOW )
			attack_mod += 10;
	}
	
	if ( !ch->delay_info1 ) {
		if ( bow->o.weapon.use_skill == SKILL_LONGBOW )
			attack_mod += 20;
		else if ( bow->o.weapon.use_skill == SKILL_SHORTBOW )
			attack_mod += 10;
	}
	
	else if ( ch->delay_info1 == 1 ) {
		if ( bow->o.weapon.use_skill == SKILL_LONGBOW )
			attack_mod += 10;
		else if ( bow->o.weapon.use_skill == SKILL_CROSSBOW )
			attack_mod += 10;
	}
	
	else if ( ch->delay_info1 == 2 ) {
		if ( bow->o.weapon.use_skill == SKILL_SHORTBOW )
			attack_mod += 10;
		else if ( bow->o.weapon.use_skill == SKILL_CROSSBOW )
			attack_mod += 15;
	}

	result = calculate_missile_result (ch, bow->o.weapon.use_skill, attack_mod, target, 0, bow, ammo, NULL, &location, &damage);
	damage = (int)(damage);

    if ( (result == CRITICAL_MISS || result == MISS) &&
    		target->fighting &&
    		target->fighting != ch &&
    		number(1,25) > ch->dex ) {
        target = target->fighting;
        result = calculate_missile_result (ch, bow->o.weapon.use_skill, roll, target, 0, bow, ammo, NULL, &location, &damage);
		switched_target = TRUE;
    }

	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';
	*buffer = '\0';

	
	if ( result == MISS ) {
		if ( ranged ) {
			temp_arg = char_short(target);
			snprintf (buf, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward you!\n\nIt misses you completely.", ammo->short_description, from_direction);
			*buf = toupper(*buf);
			snprintf (buffer, MAX_STRING_LENGTH,  "#2%s", buf);
			snprintf (buf, MAX_STRING_LENGTH,  "\n%s", buffer);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward #5%s#0.\n\nIt misses %s completely.", ammo->short_description, from_direction, temp_arg, HMHR(target));
			*buf2 = toupper (*buf2);
			snprintf (buffer, MAX_STRING_LENGTH,  "#2%s", buf2);
			snprintf (buf2, MAX_STRING_LENGTH,  "\n%s", buffer);
			snprintf (buf3, MAX_STRING_LENGTH,  "It misses %s completely.", HMHR(target));
		}
		else {
			snprintf (buf, MAX_STRING_LENGTH,  "\nIt misses %s completely.", HMHR(target));
			snprintf (buf2, MAX_STRING_LENGTH,  "\nIt misses you completely.");
		}
	}  /* Miss. */

	else if ( result == CRITICAL_MISS ) {
		if ( ranged ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, but it flies far wide of striking any target.", ammo->short_description, from_direction);
			*buf = toupper (*buf);
			snprintf (buffer, MAX_STRING_LENGTH,  "#2%s", buf);
			snprintf (buf, MAX_STRING_LENGTH,  "\n%s", buffer);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, but it flies far wide of striking any target.", ammo->short_description, from_direction);
			*buf2 = toupper(*buf2);
			snprintf (buffer, MAX_STRING_LENGTH,  "#2%s", buf2);
			snprintf (buf2, MAX_STRING_LENGTH,  "\n%s", buffer);
			snprintf (buf3, MAX_STRING_LENGTH,  "Your shot flies far wide of striking any target.");
		}
		else if ( !ranged ) {
			snprintf (buf, MAX_STRING_LENGTH,  "It streaks off to one side, far wide of any desired target.");
			snprintf (buf2, MAX_STRING_LENGTH,  "It streaks off to one side, far wide of any desired target.");
		}
	} /* Critical miss. */

	else if ( result == SHIELD_BLOCK ) {
		shield_obj = get_equip (target, WEAR_SHIELD);
		temp_arg = obj_short_desc(shield_obj);
		temp_arg1 = char_short(target);
		if ( ranged ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward you!\n\nIt glances harmlessly off #2%s#0.", ammo->short_description, from_direction, temp_arg );
			snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward #5%s#0.\n\nIt glances harmlessly off #2%s#0.", ammo->short_description, from_direction, temp_arg1, temp_arg);
			snprintf (buf3, MAX_STRING_LENGTH,  "It glances harmlessly off #2%s#0.", temp_arg);
		}
		else if ( !ranged ) {
			snprintf (buf, MAX_STRING_LENGTH,  "It glances harmlessly off #2%s#0.", temp_arg);
			snprintf (buf2, MAX_STRING_LENGTH,  "It glances harmlessly off #2%s#0.", temp_arg);
		}
	}

	else if ( result == HIT ||
				result == GLANCING_HIT ||
				result == CRITICAL_HIT ) {
		
		temp_arg = figure_location (target, location);
		snprintf (smite_location, MAX_STRING_LENGTH, "%s", temp_arg );

		if ( ranged ) {
			if ( result != CRITICAL_HIT && result != HIT ) {
				temp_arg = expand_wound_loc(smite_location);
				temp_arg1 = char_short(target);
				snprintf (buf, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward you!\n\nIt grazes you on the %s.", ammo->short_description, from_direction, temp_arg);
				*buf = toupper(*buf);
				snprintf (buffer, MAX_STRING_LENGTH,  "#2%s", buf);
				snprintf (buf, MAX_STRING_LENGTH,  "\n%s", buffer);
				snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward #5%s#0.\n\nIt grazes %s on the %s.", ammo->short_description, from_direction, temp_arg1, HMHR(target), temp_arg);
				*buf2 = toupper(*buf2);
				snprintf (buffer, MAX_STRING_LENGTH,  "#2%s", buf2);
				snprintf (buf2, MAX_STRING_LENGTH,  "\n%s", buffer);
                
                if ( switched_target ){
                	snprintf (buf3, MAX_STRING_LENGTH,  "It strays off-course, instead striking #5%s#0 on the %s!", temp_arg1, temp_arg);
                }
                else{
                   	snprintf (buf3, MAX_STRING_LENGTH,  "It grazes %s on the %s.", HMHR(target), temp_arg);
                }
			}
			else {
				if ( result == CRITICAL_HIT ){
					temp_arg = expand_wound_loc(smite_location);
					snprintf (buf, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward you!\n\nThe missile lodges deeply in your %s!", ammo->short_description, from_direction, temp_arg);
				}
				else{
					temp_arg = expand_wound_loc(smite_location);
					snprintf (buf, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward you!\n\nThe missile lodges in your %s!", ammo->short_description, from_direction, temp_arg);
				}
				
				*buf = toupper(*buf);
				snprintf (buffer, MAX_STRING_LENGTH,  "#2%s", buf);
				snprintf (buf, MAX_STRING_LENGTH,  "\n%s", buffer);
				if ( result == CRITICAL_HIT ){
					temp_arg = char_short(target);
					temp_arg1 = expand_wound_loc(smite_location);
					snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward #5%s#0.\n\nThe missile lodges deeply in %s %s!", ammo->short_description, from_direction, temp_arg, HSHR(target), temp_arg1);
				}
				else{
					temp_arg = char_short(target);
					temp_arg1 = expand_wound_loc(smite_location);
					snprintf (buf2, MAX_STRING_LENGTH,  "%s#0 comes whirring through the air from %s, headed straight toward #5%s#0.\n\nThe missile lodges in %s %s!", ammo->short_description, from_direction, temp_arg, HSHR(target), temp_arg1);
				}
				*buf2 = toupper(*buf2);
				snprintf (buffer, MAX_STRING_LENGTH,  "#2%s", buf2);
				snprintf (buf2, MAX_STRING_LENGTH,  "\n%s", buffer);
				
				if ( switched_target ) {
					if ( result == CRITICAL_HIT ){
						temp_arg = char_short(target);
						temp_arg1 = expand_wound_loc(smite_location); 
						snprintf (buf3, MAX_STRING_LENGTH,  "It strays off-course, instead lodging deeply in #5%s#0's %s!", temp_arg, temp_arg1);
					}
					else{
						temp_arg = char_short(target);
						temp_arg1 = expand_wound_loc(smite_location); 
						snprintf (buf3, MAX_STRING_LENGTH,  "It strays off-course, instead lodging in #5%s#0's %s!", temp_arg, temp_arg1);
					}
				}   
			
				else {
					if ( result == CRITICAL_HIT ){
						temp_arg = expand_wound_loc(smite_location);
						snprintf (buf3, MAX_STRING_LENGTH,  "The missile lodges deeply in %s %s!", HSHR(target), temp_arg);
					}
					else {
						temp_arg = expand_wound_loc(smite_location);
						snprintf (buf3, MAX_STRING_LENGTH,  "The missile lodges in %s %s!", HSHR(target), temp_arg);
					}
				} 
			}
		}
		else if ( !ranged && result != CRITICAL_HIT && result != HIT ) {
			if ( switched_target ) {
				temp_arg = char_short (target);
				temp_arg1 = expand_wound_loc (smite_location);
				snprintf (buf, MAX_STRING_LENGTH,  "It strays off-course, grazing #5%s#0 on the %s.", temp_arg, temp_arg1 );
				snprintf (buf2, MAX_STRING_LENGTH,  "It strays off-course, grazing you on the %s.", temp_arg1);
			}
			else {
				temp_arg = expand_wound_loc (smite_location);
				snprintf (buf, MAX_STRING_LENGTH,  "It grazes %s on the %s.", HMHR (target), temp_arg);
				snprintf (buf2, MAX_STRING_LENGTH,  "It grazes you on the %s.", temp_arg);
			}
		}
		else if ( !ranged && (result == CRITICAL_HIT || result == HIT) ) {
			if ( switched_target ) {
				temp_arg1 = expand_wound_loc (smite_location);
				temp_arg2 = char_short(target);
				snprintf (buf, MAX_STRING_LENGTH,  "The missile strays off-course, striking #5%s#0 instead, and lodging deeply in %s %s!", temp_arg2, HSHR (target), temp_arg1 );
				snprintf (buf2, MAX_STRING_LENGTH,  "The missile strays off-course, striking you instead, and lodging deeply in your %s!", temp_arg1);
			}
			else {
				if ( result == CRITICAL_HIT ) {
					temp_arg = expand_wound_loc (smite_location);
					snprintf (buf, MAX_STRING_LENGTH,  "The missile lodges deeply in %s %s!", HSHR (target), temp_arg);
					snprintf (buf2, MAX_STRING_LENGTH,  "The missile lodges deeply in your %s!", temp_arg);
				}
				else {
					temp_arg = expand_wound_loc (smite_location);
					snprintf (buf, MAX_STRING_LENGTH,  "The missile lodges in %s %s!", HSHR (target), temp_arg);
					snprintf (buf2, MAX_STRING_LENGTH,  "The missile lodges in your %s!", temp_arg);
				}
			}
		}
	}

	if ( result == GLANCING_HIT ||
		result == MISS ||
		result == SHIELD_BLOCK ) {
		
		if ( !number(0,9) &&
			((armor1 || armor2 || target->armor) ||
			result == MISS ||
			result == SHIELD_BLOCK) ) {
			
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n\nThe missile shatters upon impact at its destination.");
			snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "\n\nThe missile shatters upon impact at its destination.");
			snprintf (buf3 + strlen(buf3), MAX_STRING_LENGTH, "\n\nThe missile shatters upon impact at its destination.");
		}
		
		else {
			obj = load_object(ammo->virtual);
			obj->deleted = 0;
			obj->count = 1;
			obj->obj_timer = 8;			/* 2 RL hours. */
			obj->next_content = NULL;
			obj_to_room (obj, target->in_room);
		}
		extract_obj(bow->loaded);
		bow->loaded = NULL;
	}
	
	else if ( result == CRITICAL_HIT || result == HIT ) {
		lodge_missile (target, ammo, smite_location);
		bow->loaded = NULL;
	}

	if ( ranged ) {
		send_to_char ("\n", target);
		act (buf, TRUE, ch, 0, target, TO_VICT | TO_ACT_FORMAT);
		send_to_char ("\n", ch);
		act (buf3, TRUE, ch, 0, target, TO_CHAR | TO_ACT_FORMAT);
		
		for ( tch = target->room->people; tch; tch = tch->next_in_room ) {
			if ( tch != target && tch != ch ) {
				send_to_char ("\n", tch);
				act (buf2, TRUE, tch, 0, target, TO_CHAR | TO_ACT_FORMAT);
			}
		}
	}
	
	else if ( !ranged ) {
		act (buf2, TRUE, ch, 0, target, TO_VICT | TO_ACT_FORMAT);
		for ( tch = target->room->people; tch; tch = tch->next_in_room ) {
			if ( tch != target ) {
				send_to_char ("\n", tch);
				act (buf, TRUE, tch, 0, target, TO_CHAR | TO_ACT_FORMAT);
			}
		}
	}

	bow->loaded = NULL;

	criminalize (ch, target, target->room->zone, CRIME_KILL);

	ch->enemy_direction = NULL;

	if ( from_direction )
		mem_free (from_direction);
	
	if ( ch->delay_who ) {
		mem_free (ch->delay_who);
		ch->delay_who = NULL;
	}

	if (damage > 0) {
		if ( !IS_NPC (target) ) {
			target->delay_ch = ch;
			target->delay_info1 = ammo->virtual;
		}
		
		if ( wound_to_char (target, smite_location, (int)(damage), 0, 0, 0, 0) ) {
			if ( ranged ){
				send_to_char ("\nYour target collapses, dead.\n", ch);
			}
			ch->ranged_enemy = NULL;
			return;
		}
		
		if ( !IS_NPC (target) ) {
			target->delay_ch = NULL;
			target->delay_info1 = 0;
		}
	}

	for ( tch = target->room->people; tch; tch = tch->next_in_room ) {
		if ( !IS_NPC (tch) || !is_brother (target, tch) )
			continue;
		
		if ( morale_broken(tch) ||
			(!IS_SET (tch->act, ACT_ENFORCER) &&
			!IS_SET (tch->act, ACT_AGGRESSIVE)) )
			evade_attacker (tch, track (tch, ch->in_room));
	}

	npc_archery_retaliation (target, ch);

	return;

}

int has_been_sighted (CHAR_DATA *ch, CHAR_DATA *target)
{
	VIEWED_DATA		*sighted = NULL;

	if ( !ch || !target )
		return 0;

	if ( !IS_MORTAL (ch) )
		return 1;

	if ( IS_NPC (ch) && !ch->desc )
		return 1;		/* We know non-animated NPCs only acquire targets via SCANning; */
					/* don't need anti-twink code for them. */

	for ( sighted = ch->sighted; sighted; sighted = sighted->next ) {
		if ( sighted->target == target )
			return 1;
	}

	return 0;
}

void npc_archery_retaliation (CHAR_DATA *target, CHAR_DATA *ch)
{
	if ( IS_NPC (target) && !target->desc ) {
		add_threat (target, ch, 7);
		
		if ( IS_SET (target->act, ACT_MOUNT) && target->mount )
			add_threat (target->mount, ch, 7);
		
		if ( !target->fighting &&
			!IS_SET (target->flags, FLAG_ENTERING) &&
			!IS_SET (target->flags, FLAG_LEAVING) &&
			!IS_SET (target->flags, FLAG_FLEE) ) {
			do_stand (target, "", 0);
			target->speed = 4;
			
			if ( (track (target, ch->in_room) == -1 &&
				  target->in_room != ch->in_room) ||
				  (!IS_SET (ch->act, ACT_ENFORCER) &&
				  !IS_SET (ch->act, ACT_AGGRESSIVE)) ||
				 
				 	morale_broken (target) )
				evade_attacker (target, -1);
			else
				charge_archer (target, ch);
		}
		
		else if ( (IS_SET (target->act, ACT_MOUNT) &&
				target->mount) &&
				(IS_SET (target->mount->act, ACT_ENFORCER) ||
				IS_SET (target->mount->act, ACT_AGGRESSIVE)) &&
				!target->fighting &&
				!IS_SET (target->flags, FLAG_ENTERING) &&
				!IS_SET (target->flags, FLAG_LEAVING) &&
				!IS_SET (target->flags, FLAG_FLEE) ) {
					
				do_stand (target, "", 0);
				target->speed = 4;
			
				if ( (track (target, ch->in_room) == -1 &&
					target->in_room != ch->in_room) ||
					!(IS_SET (ch->act, ACT_ENFORCER) ||
					IS_SET (ch->act, ACT_AGGRESSIVE)) ||
					morale_broken (target) )
				
					evade_attacker (target, -1);
				else 
					charge_archer (target, ch);		
			}
		}
	return;
}

void do_aim (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*bow = NULL;
	CHAR_DATA	*target = NULL;
	CHAR_DATA	*tch = NULL;
	ROOM_DATA	*room = NULL;
	ROOM_DIRECTION_DATA	*exit_room = NULL;
	char		arg1 [MAX_STRING_LENGTH] = {'\0'};
	char		arg2 [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buffer [MAX_STRING_LENGTH] = {'\0'};
	int			ranged = 0;
	int			dir = 0;
	char		*temp_arg = NULL;
	char		*temp_arg1 = NULL;

    if ( IS_SWIMMING (ch) ) {
    	send_to_char ("You can't do that while swimming!\n", ch);
        return;
    }

	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch) ) {
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}
	
	if ( ch->delay_type == DEL_LOAD_WEAPON ) {
		return;
	}

	argument = one_argument (argument, arg1);
	argument = one_argument (argument, arg2);

	if ( !*arg1 ) {
		send_to_char ("Usage: aim (<direction>) <target>\n", ch);
		return;
	}

	if ( !(bow = get_equip (ch, WEAR_BOTH)) || 
              (bow->o.weapon.use_skill != SKILL_SHORTBOW &&
	       bow->o.weapon.use_skill != SKILL_LONGBOW &&
               bow->o.weapon.use_skill != SKILL_CROSSBOW) ) {
		send_to_char ("You aren't wielding a ranged weapon.\n", ch);
		return;
	}

	if ( !bow->loaded ) {
		send_to_char ("Your weapon isn't loaded.\n", ch);
		return;
	}

	if ( (!cmp_strn ("east", arg1, strlen(arg1)) ||
             !cmp_strn ("west", arg1, strlen(arg1)) ||
             !cmp_strn ("north", arg1, strlen(arg1)) ||
             !cmp_strn ("south", arg1, strlen(arg1)) ||
             !cmp_strn ("up", arg1, strlen(arg1)) ||
             !cmp_strn ("down", arg1, strlen(arg1))) && *arg2 )
		ranged = 1;

	if ( !ranged ) {
		
		
		if ( !(target = get_char_room_vis (ch, arg1)) ) {
			send_to_char ("Who did you want to target?\n", ch);
			return;
		}
		
		if ( target == ch ) {
			send_to_char ("Now, now, now... things can't be THAT bad, can they?\n", ch);
			return;
		}
		
		temp_arg = char_short(target);
		snprintf (buf, MAX_STRING_LENGTH,  "You begin to carefully take aim at #5%s#0.", temp_arg);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		
		temp_arg = char_short(ch);
		temp_arg1 = char_short (target);
		snprintf (buf, MAX_STRING_LENGTH,  "%s#0 begins to carefully take aim at #5%s#0 with #2%s#0.", temp_arg, temp_arg1, bow->short_description);
		
		*buf = toupper(*buf);
		snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
		
		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( tch == ch )
				continue;
			if ( tch == target )
				continue;
			if ( CAN_SEE (tch, ch) )
				act (buffer, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
		}

		temp_arg = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "%s#0 begins to carefully take aim at you with #2%s#0!", temp_arg, bow->short_description);
		*buf = toupper(*buf);
		snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
		
		if ( CAN_SEE (target, ch) )
			act (buffer, FALSE, ch, 0, target, TO_VICT | TO_ACT_FORMAT);
		
		else if ( real_skill (target, SKILL_DANGER_SENSE) ) {
			if ( skill_use (target, SKILL_DANGER_SENSE, 0) )
				send_to_char ("The back of your neck prickles, as if you are being watched closely.\n", target);
		}

		ch->aiming_at = target;
		target->targeted_by = ch;
		return;
	}

	else if ( ranged ) {
		if ( !cmp_strn ("north", arg1, strlen(arg1)) )
			dir = 0;
		else if ( !cmp_strn ("east", arg1, strlen(arg1)) )
			dir = 1;
		else if ( !cmp_strn ("south", arg1, strlen(arg1)) )
			dir = 2;
		else if ( !cmp_strn("west", arg1, strlen(arg1)) )
			dir = 3;
		else if ( !cmp_strn ("up", arg1, strlen(arg1)) )
			dir = 4;
		else if ( !cmp_strn ("down", arg1, strlen(arg1)) )
			dir = 5;

		if ( !PASSAGE(ch, dir)) { 
			send_to_char ("There isn't an exit in that direction.\n", ch);
			return;
		}

		ch->delay_who = str_dup (dirs[dir]);

		room = vtor(PASSAGE(ch,dir)->to_room);

		exit_room = PASSAGE(ch, dir);
		
		if ( exit_room && IS_SET (exit_room->exit_info, PASSAGE_CLOSED) ) {
			send_to_char ("Your view is blocked.\n", ch);
			return;
		}

		
	
		if ( IS_SET (room->room_flags, STIFLING_FOG) ) {
			send_to_char ("The stiflingly heavy fog in that area thwarts any such attempt.\n", ch);
			return;
		}

		if (!(target = get_char_room_vis2 (ch, room->virtual, arg2)) ||
			!has_been_sighted (ch, target) ) {
			exit_room = room->dir_option[dir];
			if ( !exit_room ) {
				send_to_char ("You don't see them within range.\n", ch);
				return;
			}
			
			if ( exit_room && IS_SET (exit_room->exit_info, PASSAGE_CLOSED) ) {
				send_to_char ("Your view is blocked.\n", ch);
				return;
			}
			
			if ( room->dir_option[dir] )
				room = vtor(room->dir_option[dir]->to_room);
			else
				room = NULL;
			
			if ( bow->o.od.value[1] == 3 || !room ) {	/* Sling, eventually. */
				send_to_char ("You don't see them within range.\n", ch);
				return;
			}
			
			
			
			if ( IS_SET (room->room_flags, STIFLING_FOG) ) {
				send_to_char ("The stiflingly heavy fog in that area thwarts any such attempt.\n", ch);
				return;
			}
			
			if (!(target = get_char_room_vis2 (ch, room->virtual, arg2)) ||
				 !has_been_sighted (ch, target) ) {
				exit_room = room->dir_option[dir];
				
				if ( !exit_room ) {
					send_to_char ("You don't see them within range.\n", ch);
					return;
				}
				
				if ( exit_room && IS_SET (exit_room->exit_info, PASSAGE_CLOSED) ) {
					send_to_char ("Your view is blocked.\n", ch);
					return;
				}
				
				if ( room->dir_option[dir] )
					room = vtor(room->dir_option[dir]->to_room);
				else 
					room = NULL;
				
				if ( bow->o.od.value[1] == 2 || !room ) {	/* Crossbows and shortbows; two-room range. */
					send_to_char ("You don't see them within range.\n", ch);
					return;
				}
				
				
				
				if ( IS_SET (room->room_flags, STIFLING_FOG) ) {
					send_to_char ("The stiflingly heavy fog in that area thwarts any such attempt.\n", ch);
					return;
				}
				
				if (!(target = get_char_room_vis2(ch, room->virtual, arg2)) ||
					!has_been_sighted (ch, target) ) {
					exit_room = room->dir_option[dir];
					if ( !exit_room ) {
						send_to_char ("You don't see them within range.\n", ch);
						return;
					}
					if ( IS_SET (exit_room->exit_info, PASSAGE_CLOSED) ) {
						send_to_char ("Your view is blocked.\n", ch);
						return;
					}
					send_to_char ("You don't see them within range.\n", ch);
					return;
				}
				else ch->delay_info1 = 3;
			}
			else ch->delay_info1 = 2;
		}
		
		if ( !target ||
			!CAN_SEE (ch, target) ||
			!has_been_sighted (ch, target) ) {
			send_to_char ("You don't see them within range.\n", ch);
			return;
		}
		
		if ( target == ch ) {
			send_to_char ("Now, now, now... things can't be THAT bad, can they?\n", ch);
			return;
		}
		
		temp_arg = char_short(target);
		snprintf (buf, MAX_STRING_LENGTH,  "You begin to carefully take aim at #5%s#0.", temp_arg);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		
		temp_arg = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "%s#0 turns %sward, taking aim at a distant target with #2%s#0.", temp_arg, dirs[dir], bow->short_description);
		*buf = toupper(*buf);
		ch->ranged_enemy = target;
		snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
		
		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( tch == ch )
				continue;
			if ( tch == target )
				continue;
			if ( CAN_SEE (tch, ch) )
				act (buffer, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
		}
		
		if ( real_skill (target, SKILL_DANGER_SENSE) ) {
			if ( skill_use (target, SKILL_DANGER_SENSE, 0) )
				send_to_char ("The back of your neck prickles, as if you are being watched closely.\n", target);
		}
		ch->aiming_at = target;
		target->targeted_by = ch;
		return;		
	}
	
	return;
}

void do_unload (CHAR_DATA *ch, char *argument, int cmd)
{
	OBJ_DATA	*bow = NULL;
	OBJ_DATA	*arrow = NULL;
	OBJ_DATA	*quiver = NULL;
	int			i = 0;
	char		*error = NULL;
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	char		buffer[MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg1 = NULL;
	char		*temp_arg2 = NULL;
	char		*temp_arg3 = NULL;

	bow = get_bow (ch);

   if ( !bow ) {
        send_to_char ("What did you wish to unload?\n", ch);
    	return;
    }

	if ( !bow->loaded ) {
		send_to_char ("That isn't loaded.\n", ch);
		return;
	}

	if ( !(arrow = bow->loaded) ) {
		send_to_char ("That isn't loaded.\n", ch);
		return;
	}

	temp_arg1 = char_short (ch);
	snprintf (buf, MAX_STRING_LENGTH,  "%s#0 unloads #2%s#0.", temp_arg1, bow->short_description);
	
	*buf = toupper (*buf);
	snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
	act (buffer, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	arrow = load_object(bow->loaded->virtual);
	arrow->in_obj = NULL;
	arrow->carried_by = NULL;

    for ( i = 0; i < MAX_WEAR; i++ ) {
		if ( !(quiver = get_equip (ch, i)) )
        	continue;
        if ( GET_ITEM_TYPE (quiver) == ITEM_QUIVER &&
        	get_obj_in_list_num (arrow->virtual, quiver->contains) &&
            can_obj_to_container (arrow, quiver, &error, 1) )
			break;
    }

	if ( !quiver ) {
    	for ( i = 0; i < MAX_WEAR; i++ ) {
	       	if ( !(quiver = get_equip (ch, i)) )
               		continue;
        	if ( GET_ITEM_TYPE (quiver) == ITEM_QUIVER &&
        		can_obj_to_container (arrow, quiver, &error, 1) )
					break;
			else 
					quiver = NULL;
        }
	}
	
	if ( quiver ) {
		send_to_char ("\n", ch);
		temp_arg1 = obj_short_desc(bow);
		temp_arg2 = obj_short_desc(arrow);
		temp_arg3 = obj_short_desc(quiver);
		snprintf (buf, MAX_STRING_LENGTH,  "You unload #2%s#0, and slide #2%s#0 into #2%s#0.", temp_arg1, temp_arg2, temp_arg3);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		obj_to_obj (arrow, quiver);
        }
	else if ( !quiver ) {
		temp_arg1 = obj_short_desc(bow);
		snprintf (buf, MAX_STRING_LENGTH,  "You unload #2%s#0.\n", temp_arg1);
		send_to_char (buf, ch);
		obj_to_char (arrow, ch);
	}

	bow->loaded = NULL;
	
	return;
}

void delayed_load (CHAR_DATA *ch)
{
	OBJ_DATA	*bow = NULL;
	OBJ_DATA	*ammo = NULL;
	OBJ_DATA	*quiver = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			i = 0;

	if ( !(bow = get_equip (ch, WEAR_BOTH)) ) {
		ch->delay_who = NULL;
		ch->delay = 0;
		ch->delay_type = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		return;
	}

	if ( bow->o.weapon.use_skill != SKILL_SHORTBOW &&
	     bow->o.weapon.use_skill != SKILL_LONGBOW &&
	     bow->o.weapon.use_skill != SKILL_CROSSBOW ) {
		ch->delay_who = NULL;
		ch->delay = 0;
		ch->delay_type = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		return;
	}

	ammo = NULL;

    for ( i = 0; i < MAX_WEAR; i++ ) {
		if ( !(quiver = get_equip (ch, i)) )
			continue;
 		
 		if ( quiver->virtual != ch->delay_info1 ||
 			!get_obj_in_list_num (ch->delay_info2, quiver->contains) )
            continue;
            
        if ( GET_ITEM_TYPE (quiver) == ITEM_QUIVER ) {
        	for ( ammo = quiver->contains; ammo; ammo = ammo->next_content ){
            	if ( GET_ITEM_TYPE (ammo) == ITEM_MISSILE && ammo->virtual == ch->delay_info2 ){
                	break;
                }
                
				if ( ammo )
					break;
            }
        }	
	}
	
	if ( !ammo ) {
		send_to_char ("Having lost your ammunition, you cease loading your weapon.\n", ch);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "You finish loading #2%s#0.\n", bow->short_description);
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "$n finishes loading #2%s#0.", bow->short_description);
	act (buf, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);

	bow->loaded = load_object(ammo->virtual);

	if ( IS_MORTAL(ch) )
		obj_from_obj (&ammo, 1);

	ch->delay_who = NULL;
	ch->delay = 0;
	ch->delay_info1 = 0;
	ch->delay_info2 = 0;
	ch->delay_type = 0;
	
	return;

}

void do_hit (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*victim= NULL;
	CHAR_DATA	*tch =  NULL;
	OBJ_DATA	*obj = NULL;
	int			i = 0;
	int			agi_diff=0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		original [MAX_STRING_LENGTH] = {'\0'};

	snprintf (original, MAX_STRING_LENGTH, "%s", argument);

				/* cmd = 0 if hit,
				   cmd = 1 if kill */

        if ( IS_SWIMMING (ch) ) {
                send_to_char ("You can't do that while swimming!\n", ch);
                return;
        }

	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL (ch) ) {
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}
	
	if ( is_room_affected (ch->room->affects, MAGIC_ROOM_CALM) ) {
		act ("Try as you might, you simply cannot muster the will to break "
			 "the peace that pervades the area.",
				FALSE, ch, 0, 0, TO_CHAR);
		return;
	}
	
	if ( IS_SET (ch->flags, FLAG_PACIFIST) ) {
		send_to_char ("Remove your pacifist flag, first...\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( (obj = get_equip (ch, WEAR_BOTH)) ) {
		if ( obj->o.weapon.use_skill == SKILL_LONGBOW ||
		     obj->o.weapon.use_skill == SKILL_SHORTBOW ||
                     obj->o.weapon.use_skill == SKILL_CROSSBOW ||
		     obj->o.weapon.use_skill == SKILL_SLING ||
                     obj->o.weapon.use_skill == SKILL_THROWN ) {
				send_to_char ("You can't use that in melee combat!\n", ch);
				return;
		}
	}

	if ( get_affect (ch, MAGIC_AFFECT_PARALYSIS) ) {
		send_to_char ("You are paralyzed and unable to fight!\n\r", ch);
		return;
	}

	if ( get_affect (ch, MAGIC_AFFECT_FEAR) ) {
		send_to_char ("You are too afraid to fight!\n\r", ch);
		return;
	}

	if ( IS_SUBDUER (ch) ) {
		act ("You can't attack while you have $N subdued.",
				FALSE, ch, 0, ch->subdue, TO_CHAR);
		return;
	}

        if ( IS_SET (ch->plr_flags, NEW_PLAYER_TAG) && IS_SET (ch->room->room_flags, LAWFUL) && *argument != '!' ) {
                snprintf (buf, MAX_STRING_LENGTH,  "You are in a lawful area; you would likely be flagged wanted for assault. "
                              "To confirm, type \'#6hit %s !#0\', without the quotes.", original);
                act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
        }

	if ( !*buf ) {

		if ( IS_SET (ch->flags, FLAG_FLEE) ) {
			send_to_char ("You stop trying to flee.\n\r", ch);
			REMOVE_BIT (ch->flags, FLAG_FLEE);
			return;
		}

		send_to_char ("Hit whom?\n\r", ch);
		return;
	}

	if ( !(victim = get_char_room_vis (ch, buf)) ) {
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if ( victim == ch ) {
		send_to_char ("You affectionately pat yourself.\n\r", ch);
		act ("$n affectionately pats $mself.", FALSE, ch, 0, victim, TO_ROOM);
		return;
	}

	if ( IS_SET (victim->act, ACT_PREY) && AWAKE (victim) ) {
		if ( !get_affect (ch, MAGIC_HIDDEN) || !skill_use (ch, SKILL_SNEAK, 0) ) {
			act ("As you approach, $N spots you and darts away! Try using a ranged weapon or an ambush from hiding instead.", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
			act ("As $n approaches, $N spots $m and darts away.", FALSE, ch, 0, victim, TO_ROOM | TO_ACT_FORMAT);
			evade_attacker (victim, -1);
			add_threat (victim, ch, 7);
			return;
		}
		else act ("You successfully ambush $N, leaping from your cover to attack!", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
	}

	if ( IS_SET (victim->act, ACT_FLYING) && !IS_SET (ch->act, ACT_FLYING) && AWAKE(victim) ) {
		send_to_char ("They are flying out of reach!\n", ch);
		return;
	}

	if ( IS_SET (victim->act, ACT_VEHICLE) ) {
		send_to_char ("How do you propose to kill an inanimate object, hmm?\n", ch);
		return;
	}

    for ( tch = vtor (ch->in_room)->people; tch; tch = tch->next_in_room ){
	   	if (tch->fighting == victim) {
			if (++i >= 4) {
	    		act ("You can't find an opening to attack $N.",
					FALSE, ch, 0, victim, TO_CHAR);
	    		return;
			}
    	}
	}
	
	REMOVE_BIT (ch->flags, FLAG_FLEE);
	REMOVE_BIT (ch->act, PLR_STOP);

	if ( GET_POS (ch) == POSITION_STANDING &&
		 !ch->fighting &&
		 victim != ch->fighting ) {

		if ( ch->delay && ch->delay_type != DEL_CAST )
			break_delay (ch);

		ch->primary_delay = 0;
		ch->secondary_delay = 0;

		if ( cmd == 0 )
			REMOVE_BIT (ch->flags, FLAG_KILL);
		else
			SET_BIT (ch->flags, FLAG_KILL);

                set_fighting (ch, victim);
                if ( !victim->fighting )
                        set_fighting (victim, ch);

		hit_char (ch, victim, 0);

		WAIT_STATE(ch, 9); 		

		if ( victim->deleted )
			return;

		if ( IS_SET (victim->act, ACT_MEMORY) && IS_NPC (victim) ) 
			add_memory (ch, victim);

			/* Looks like a problem.  If you hit/kill in one hit, then
			   trigger isn't called. */

		trigger (ch, argument, TRIG_HIT);

		if ( ch->fighting == victim && IS_SUBDUEE (victim) )
			stop_fighting (ch);
	}

	else if ( ch->fighting == victim &&
			  !IS_SET (ch->flags, FLAG_KILL) &&
			  AWAKE (ch) && cmd == 1 ) {
		act ("You will try to kill $N.", FALSE, ch, 0, victim, TO_CHAR);
		SET_BIT (ch->flags, FLAG_KILL);
	}

	else if ( ch->fighting == victim &&
			  IS_SET (ch->flags, FLAG_KILL) &&
			  AWAKE (ch) && cmd == 0 ) {
		act ("You will try NOT to kill $N.", FALSE, ch, 0, victim, TO_CHAR);
		REMOVE_BIT (ch->flags, FLAG_KILL);
	}

	else if ( ch->fighting &&
			  (GET_POS (ch) == FIGHT ||
			   GET_POS (ch) == STAND) &&
			  victim != ch->fighting ) {
		
		if ( ch->agi <= 9 )
			ch->balance += -15;
		else if ( ch->agi > 9 && ch->agi <= 13 )
			ch->balance += -13;
		else if ( ch->agi > 13 && ch->agi <= 15 )
			ch->balance += -11;
		else if ( ch->agi > 15 && ch->agi <= 18 )
			ch->balance += -9;
		else ch->balance += -7;
		ch->balance = MAX (ch->balance, -50);

		if ( ch->balance < -15 ) {
			act ("You need more balance before you can try to attack $N!",FALSE,ch,0,victim,TO_CHAR | TO_ACT_FORMAT);
			return;
		}

		if ( ch->fighting ) {

			if ( ch->fighting->fighting == ch ) {

				agi_diff = GET_AGI (ch) - GET_AGI (ch->fighting);

				if ( agi_diff > number(-10,10) && ( number(0,19) != 0 ) ) {
				  
					act ("You fail to shift your attention away from $N.",FALSE,ch,0,ch->fighting,TO_CHAR | TO_ACT_FORMAT | TO_ACT_COMBAT);
					act ("$N fails to shift their attention away from you.",FALSE,ch->fighting,0,ch,TO_CHAR | TO_ACT_FORMAT | TO_ACT_COMBAT);
					act ("$N fails to shift their attention away from $n.", FALSE, ch->fighting, 0, ch, TO_NOTVICT | TO_ACT_FORMAT | TO_ACT_COMBAT);

					return;
				}
			}

			act ("You stop fighting $N.",
			     FALSE, ch, 0, ch->fighting, TO_CHAR);
			act ("You ready yourself for battle with $N.",
			     FALSE, ch, 0, victim, TO_CHAR);
			stop_fighting (ch);

		}

		if ( (is_area_enforcer (victim) || victim->race == RACE_WOLF) && IS_NPC (victim) )
			do_alert (victim, "", 0);

		set_fighting (ch, victim);

		act ("You notice $N's attention shift toward you!",
					FALSE, victim, 0, ch, TO_CHAR);

		if ( cmd == 0 )
			REMOVE_BIT (ch->flags, FLAG_KILL);
		else
			SET_BIT (ch->flags, FLAG_KILL);

		if ( IS_SET (victim->act, ACT_MEMORY) && IS_NPC (victim) ) 
			add_memory (ch, victim);

		trigger (ch, argument, TRIG_HIT);
	}

	else
		send_to_char ("You're doing the best you can!\n\r", ch);
	
	return;
}


void do_nokill (CHAR_DATA *ch, char *argument, int cmd)
{
	send_to_char ("Please spell out all of 'kill' to avoid any mistakes.\n",
				  ch);
	return;
}

void do_kill (CHAR_DATA *ch, char *argument, int cmd)
{
	char			arg [MAX_STRING_LENGTH] = {'\0'};
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			verify [MAX_STRING_LENGTH] = {'\0'};
	char			original [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA		*victim = NULL;
	CHAR_DATA		*tch = NULL;
	OBJ_DATA		*obj = NULL;
    int				i = 0;

	snprintf (original, MAX_STRING_LENGTH, "%s", argument);

    if ( IS_SWIMMING (ch) ) {
    	send_to_char ("You can't do that while swimming!\n", ch);
        return;
    }

	if ( IS_NPC (ch) ) {
		do_hit (ch, argument, 1);
    		return;
	}
	
	if ( IS_SET (ch->room->room_flags, OOC) && IS_MORTAL(ch) ) {
		send_to_char ("You cannot do this in an OOC area.\n", ch);
		return;
	}

	if ( IS_SET (ch->flags, FLAG_PACIFIST) ) {
		send_to_char ("Remove your pacifist flag, first...\n", ch);
		return;
	}

	if ( (obj = get_equip (ch, WEAR_BOTH)) ) {
		if ( obj->o.weapon.use_skill == SKILL_LONGBOW ||
		     obj->o.weapon.use_skill == SKILL_SHORTBOW ||
                     obj->o.weapon.use_skill == SKILL_CROSSBOW ||
		     obj->o.weapon.use_skill == SKILL_SLING ||
                     obj->o.weapon.use_skill == SKILL_THROWN ) {
				send_to_char ("You can't use that in melee combat!\n", ch);
				return;
		}
	}

	argument = one_argument (argument, arg);

	argument = one_argument (argument, verify);

	if ( !*arg ) {
		send_to_char ("Kill who?\n\r", ch);
		return;
	}

	if ( !(victim = get_char_room_vis (ch, arg)) ) {
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	if ( ch == victim ) {
	   	send_to_char ("You slap yourself on the wrist.\n\r", ch);
		return;
	}

	if ( IS_SET (victim->act, ACT_PREY) && AWAKE (victim) ) {
		if ( !get_affect (ch, MAGIC_HIDDEN) || !skill_use (ch, SKILL_SNEAK, 0) ) {
			act ("As you approach, $N spots you and darts away! Try using a ranged weapon or an ambush from hiding instead.", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
			act ("As $n approaches, $N spots $m and darts away.", FALSE, ch, 0, victim, TO_ROOM | TO_ACT_FORMAT);
			evade_attacker (victim, -1);
			add_threat (victim, ch, 7);
			return;
		}
		else act ("You successfully ambush $N, leaping from your cover to attack!", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
	}

	if ( IS_SET (victim->act, ACT_FLYING) && !IS_SET (ch->act, ACT_FLYING) && AWAKE(victim) ) {
		send_to_char ("They are flying out of reach!\n", ch);
		return;
	}

	if ( IS_SET (victim->act, ACT_VEHICLE) ) {
		send_to_char ("How do you propose to kill an inanimate object, hmm?\n", ch);
		return;
	}

        if ( IS_SET (ch->plr_flags, NEW_PLAYER_TAG) && IS_SET (ch->room->room_flags, LAWFUL) && *verify != '!' ) {
                snprintf (buf, MAX_STRING_LENGTH,  "You are in a lawful area; you would likely be flagged wanted for assault. "
                              "To confirm, type \'#6hit %s !#0\', without the quotes.", original);
                act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
        }

        i = 0;
        for ( tch = vtor (ch->in_room)->people; tch; tch = tch->next_in_room ){
            if(tch->fighting == victim) {
                if(++i >= 4) {
                    act ("You can't find an opening to attack $N.",
                        FALSE, ch, 0, victim, TO_CHAR);
                    return;
                }

            }
        }

	if ( (is_area_enforcer (victim) || victim->race == RACE_WOLF) && IS_NPC (victim) )
		do_alert (victim, "", 0);

	if ( ch->fighting ) {
		ch->fighting = victim;
		act ("You will fight $N until $E is dead.",
					FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	if ( IS_NPC (ch) || !GET_TRUST (ch) ) {

		SET_BIT (ch->flags, FLAG_KILL);

		set_fighting (ch, victim);
		if ( !victim->fighting )
			set_fighting (victim, ch);

		hit_char (ch, victim, 0);
	}

   	else {
		if ( !IS_NPC (victim) && *verify != '!' ) {
			send_to_char ("Target is a player character.  Please use "
						  "'KILL <name> !' syntax if \n\ryou really mean it.'\n\r", ch);
			return;
		}

		if ( GET_TRUST (victim) > GET_TRUST (ch) ) {
			victim = ch;
		}

		act ("$n stares at you, narrowing $s eyes. Shortly thereafter, your heart obediently ceases to beat, and you feel death upon you...", FALSE, ch, 0, victim, TO_VICT | TO_ACT_FORMAT);

		act ("You narrow your eyes in concentration, and $N collapses, dead.", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);

		act ("$n stares at $N, narrowing $s eyes. Suddenly, $N collapses, dead.", FALSE, ch, 0, victim, TO_NOTVICT | TO_ACT_FORMAT);

		die (victim);
	}
	return;
}

void do_order (CHAR_DATA *ch, char *argument, int cmd)
{
	int			everyone = 0;
	CHAR_DATA	*victim = NULL;
	CHAR_DATA	*next_in_room = NULL;
	CHAR_DATA	*tch = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		command [MAX_STRING_LENGTH] = {'\0'};
 	char		*temp_arg = NULL;
 	
	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Order whom?\n\r", ch);
		return;
	}

		/* We can order invis chars as well, why not :) */

	if ( !str_cmp (buf, "all") || !str_cmp (buf, "follower") ||
		 !str_cmp (buf, "followers") )
		everyone = 1;

	else if ( !(victim = get_char_room_vis (ch, buf)) &&
		 !(victim = get_char_room (buf, ch->in_room)) ) {
		send_to_char ("They aren't here.\n\r", ch);
		return;
	}

	while ( isspace (*argument) )
		argument++;

	if ( !*argument ) {
		send_to_char ("What is your order?\n\r", ch);
		return;
	}

	strcpy (command, argument);

	if ( victim == ch ) {
		send_to_char ("You don't have to give yourself orders.\n\r", ch);
		return;
	}

	if ( victim ) {

		if ( !is_leader (ch, victim) &&
			 victim->following != ch ) {
			act ("You do not have the authority to order $N.", FALSE, ch, 0, victim, TO_CHAR);
			return;
		}

		temp_arg = char_short (ch);
		snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 orders you to '%s'.\n", temp_arg, command);
		buf [2] = toupper (buf [2]);
		send_to_char (buf, victim);


		if ( victim->mob ) {
			send_to_char ("Ok.\n", ch);
			command_interpreter (victim, command);
		} 
		else {
			temp_arg = char_short (victim);
			snprintf (buf, MAX_STRING_LENGTH,  "You order #5%s#0 to '%s'.\n",
						temp_arg, command);
			send_to_char (buf, ch);
		}

		return;
	}

	for ( tch = ch->room->people; tch; tch = next_in_room ) {

		next_in_room = tch->next_in_room;
		
		if ( !is_leader (ch, tch) &&
			 (tch->following != ch) )
			continue;
		
		temp_arg = char_short (ch);
		snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 orders you to '%s'.\n", temp_arg, command);
		buf [2] = toupper (buf [2]);
		send_to_char (buf, tch);

		if ( tch->mob )
			command_interpreter (tch, command);
	}

	send_to_char ("Ok.\n", ch);
	return;
}

void do_flee (CHAR_DATA *ch, char *argument, int cmd)
{
	int		dir;

	if ( !ch->fighting ) {
		send_to_char ("You're not fighting.\n\r", ch);
		return;
	} 

	if ( get_affect (ch, MAGIC_AFFECT_PARALYSIS) ) {
		send_to_char ("You are paralyzed and unable to flee!\n\r", ch);
		return;
	}

	if ( IS_SET (ch->flags, FLAG_FLEE) ) {
		send_to_char ("You are already trying to escape!\n\r", ch);
		return;
	}

	for ( dir = 0; dir < 6; dir++ )
		if ( CAN_GO (ch, dir) && !IsGuarded (ch->room, dir) )
			break;

	if ( dir == 6 ) {
		send_to_char ("THERE IS NOWHERE TO FLEE!!\n\r", ch);
		return;
	}

	SET_BIT (ch->flags, FLAG_FLEE);

	send_to_char ("You resolve to escape combat. . .\n\r", ch);

	act ("$n's eyes dart about looking for an escape path!",
				FALSE, ch, 0, 0, TO_ROOM);
	return;
}

int flee_attempt (CHAR_DATA *ch)
{
	int				dir = 0;
	int				enemies = 0;
	int				mobless_count = 0;
	int				mobbed_count = 0;
	int				mobless_dirs [6];
	int				mobbed_dirs [6];
	CHAR_DATA		*tch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	ROOM_DATA		*troom = NULL;
/*
	if ( IS_SET (ch->flags, FLAG_SUBDUING) ) {
		REMOVE_BIT (ch->flags, FLAG_FLEE);
		return 0;
	}
*/
	if ( GET_POS (ch) < FIGHT )
		return 0;

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {

		if ( tch->fighting != ch )
			continue;

		if ( GET_POS (tch) != FIGHT && GET_POS (tch) != STAND )
			continue;

		if ( !CAN_SEE (tch, ch) )
			continue;

		enemies++;
	}

        for ( dir = 0; dir < 6; dir++ ) {
        
                if ( !CAN_GO (ch, dir) )
                        continue;
        
                if ( vtor (PASSAGE (ch, dir)->to_room)->people )
                        mobbed_dirs [mobbed_count++] = dir;
                else
                        mobless_dirs [mobless_count++] = dir;
        }

	if ( !mobbed_count && !mobless_count ) {
		send_to_char ("There is nowhere to go!  You continue fighting!\n\r", ch);
		REMOVE_BIT (ch->flags, FLAG_FLEE);
		return 0;
	}

	if ( enemies && number (0, enemies) ) {
		switch (number (1, 3)) {
		case 1: send_to_char ("You attempt escape, but fail . . .\n\r", ch);
				break;
		case 2: send_to_char ("You nearly escape, but are blocked . . .\n\r", ch);
				break;
		case 3: send_to_char ("You continue seeking escape . . .\n\r", ch);
				break;
		}

		act ("$n nearly flees!", TRUE, ch, 0, 0, TO_ROOM);

		return 0;
	}

	if ( mobless_count )
		dir = mobless_dirs [number (0, mobless_count - 1)];
	else
		dir = mobbed_dirs [number (0, mobbed_count - 1)];

	troom = ch->room;

	/* stop_fighting_sounds (ch, troom); */

	if ( ch->fighting ) {
		stop_fighting (ch);
	}

	do_move (ch, "", dir);

	act ("$n #3flees!#0", FALSE, ch, 0, 0, TO_ROOM);

	snprintf (buf, MAX_STRING_LENGTH,  "#3YOU FLEE %s!#0", dirs [dir]);
	act (buf, FALSE, ch, 0, 0, TO_CHAR);

	if ( !enemies )
		snprintf (buf, MAX_STRING_LENGTH,  "\nYou easily escaped to the %s.\n\r", dirs [dir]);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "\nYou barely escaped to the %s.\n\r", dirs [dir]);

	if ( ch->room != troom )
		send_to_char (buf, ch);

	REMOVE_BIT (ch->flags, FLAG_FLEE);

	return 1;
}

	/* In case victim is being guarded, make sure rescue affects are active. */

void guard_check (CHAR_DATA *victim)
{
	CHAR_DATA		*tch;
	AFFECTED_TYPE	*af;

	for ( tch = victim->room->people; tch; tch = tch->next_in_room ) {

		if ( !(af = get_affect (tch, MAGIC_GUARD)) )
			continue;

		if ( (CHAR_DATA *) af->a.spell.t == victim &&
			 !get_second_affect (tch, SPA_RESCUE, NULL) )
			add_second_affect (SPA_RESCUE, 1, tch, (OBJ_DATA *) victim, NULL, 0);
	}
	
	return;
}

void do_guard (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*target = NULL;
	CHAR_DATA		*tch = NULL;
	AFFECTED_TYPE		*af;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	int			dir;

	if ( IS_SET (ch->room->room_flags, OOC) ) {
		send_to_char ("That command cannot be used in an OOC area.\n", ch);
		return;
	}

	if ( (af = get_affect (ch, AFFECT_GUARD_DIR)) )
		affect_remove (ch, af);

	argument = one_argument (argument, buf);

	if ( *buf && !(target = get_char_room_vis (ch, buf)) ) {
                if ( (dir = index_lookup (dirs, buf)) == -1 ) {
                        send_to_char ("Who or what did you want to guard?\n", ch);
                        return;
                }
                
		if ( !(af = get_affect (ch, AFFECT_GUARD_DIR)) )
               		magic_add_affect (ch, AFFECT_GUARD_DIR, -1, 0, 0, 0, 0);
        
                af = get_affect (ch, AFFECT_GUARD_DIR);
                
            	af->a.shadow.shadow = NULL;
                af->a.shadow.edge = dir;

		snprintf (buf, MAX_STRING_LENGTH,  "You will now guard the %s exit.\n", dirs[dir]);
		send_to_char (buf, ch);
		snprintf (buf, MAX_STRING_LENGTH,  "$n moves to block the %s exit.", dirs[dir]);
		act (buf, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		return;

	}

	if ( (!*buf || target == ch) ) {
		if ( !(af = get_affect (ch, MAGIC_GUARD)) && !(af = get_affect (ch, AFFECT_GUARD_DIR)) ) {
			send_to_char ("You are not currently guarding anything.\n", ch);
			return;
		}
		else if ( (af = get_affect (ch, MAGIC_GUARD)) && (tch = (CHAR_DATA *)af->a.spell.t) != NULL ) {
			act ("You cease guarding $N.", TRUE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
			act ("$n ceases guarding you.", FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);	
			act ("$n ceases guarding $N.", FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
			remove_affect_type (ch, MAGIC_GUARD);
			return;
		}
		else if ( (af = get_affect (ch, AFFECT_GUARD_DIR)) ) {
			act ("You cease guarding the exit.", TRUE, ch, 0, 0, TO_CHAR);
			remove_affect_type (ch, AFFECT_GUARD_DIR);
			return;
		}
		else {
			send_to_char ("You cease guarding.\n", ch);
			remove_affect_type (ch, MAGIC_GUARD);
			remove_affect_type (ch, AFFECT_GUARD_DIR);
			return;
		}
	}

	if ( get_affect (target, MAGIC_GUARD) || get_affect (target, AFFECT_GUARD_DIR) ) {
		send_to_char ("They're already trying to guard something themselves!\n", ch);
		return;
	}

	if ( (af = get_affect (ch, MAGIC_GUARD)) )
		affect_remove (ch, af);

	magic_add_affect (ch, MAGIC_GUARD, -1, 0, 0, 0, 0);

	if ( !(af = get_affect (ch, MAGIC_GUARD)) ) {
		send_to_char ("There is a bug in guard.  Please let an admin "
					  "know.\n", ch);
		return;
	}

	af->a.spell.t = (int) target;

	act ("You will now guard $N.", FALSE, ch, 0, target, TO_CHAR | TO_ACT_FORMAT);
	act ("$n moves into position to guard you.", FALSE, ch, 0, target, TO_VICT | TO_ACT_FORMAT);
	act ("$n moves into position to guard $N.", FALSE, ch, 0, target, TO_NOTVICT | TO_ACT_FORMAT);
	
	return;
}
