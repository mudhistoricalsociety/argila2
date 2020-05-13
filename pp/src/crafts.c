/** 
*	\file crafts.c
*	Crafting Module
*
*	This module provides functionality for crafting, including creating new
*	crafts, adding and deleting crafts from a player, determining pre-requesites
*	for a craft, and ordering the list of crafts.
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

#define s(a) send_to_char (a "\n", ch);

int missing_craft_items (CHAR_DATA *ch, AFFECTED_TYPE *af)
{
	char	*p = NULL;
	int 	missing_said = 0;
	int 	i = 0;
	int		item_required [MAX_ITEMS_PER_SUBCRAFT];
	OBJ_DATA  *obj_list [MAX_ITEMS_PER_SUBCRAFT];
	PHASE_DATA	*phase = NULL;
	DEFAULT_ITEM_DATA	*item = NULL;
	
					
	if ( !af->a.craft || !af->a.craft->subcraft )
		return 0;

	memset (item_required, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (int));
	memset (obj_list, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (OBJ_DATA *));

	for ( phase = af->a.craft->subcraft->phases; phase; phase = phase->next ) {
		for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {
			item = af->a.craft->subcraft->items [i];

			if ( !item ) {
				continue;
			}

			if ( !item->items [0] ) { /* No items in list.  Nothing required */
				continue;
			}
			
			obj_list [i] = get_item_obj (ch, item, phase);
		}

		if ( phase->first ) {
			for ( p = phase->first; *p; p++ ) {
				if ( *p == '$' ) {
					p++;
					if ( isdigit (*p) && strtol(p, NULL, 10) < MAX_ITEMS_PER_SUBCRAFT ){
						item_required [strtol(p, NULL, 10)] = 1;
					}
				}
			}
		} /* if (phase->first)   */      
	} /* for( phase = af->a.craft->subcraft->phases... */

	for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {

		if ( !item_required [i] ){
			continue;
		}

		if ( obj_list [i] ){
			continue;
		}
		
		item = af->a.craft->subcraft->items [i];

		if ( !item ) {
			continue;
		}

		if ( item && IS_SET (item->flags, SUBCRAFT_PRODUCED) ){
			continue;
		}
		
		if ( !missing_said ) {
			missing_said = 1;
			send_to_char ("\n#1You are missing one or more items:#0\n\n", ch);
			act ("$n stops doing $s craft.", FALSE, ch, 0, 0, TO_ROOM);
			af->a.craft->timer = 0;
		}

		if ( IS_SET (item->flags, SUBCRAFT_WORN) )
			missing_item_msg (ch, item, "You must wear ");
		
		else if ( IS_SET (item->flags, SUBCRAFT_HELD) )
			missing_item_msg (ch, item, "You must hold ");

		else if ( IS_SET (item->flags, SUBCRAFT_IN_ROOM) )
			missing_item_msg (ch, item, "In the room must be ");

		else if ( IS_SET (item->flags, SUBCRAFT_IN_INVENTORY) )
			missing_item_msg (ch, item, "You must have in your inventory ");

		else if ( IS_SET (item->flags, SUBCRAFT_WIELDED) )
			missing_item_msg (ch, item, "You must wield ");

		else
			missing_item_msg (ch, item, "You need ");
	} /* for ( i = 0;       */

	if ( missing_said )
		return 1;

	return 0;
}

AFFECTED_TYPE *is_craft_command (CHAR_DATA *ch, char *argument)
{
	int 	i = 0;
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	AFFECTED_TYPE 		*af = NULL;
	char			command [MAX_STRING_LENGTH] = {'\0'};
	char			subcraft_name [MAX_STRING_LENGTH] = {'\0'};

	if ( IS_NPC (ch) )
		return NULL;

	if ( IS_MORTAL (ch) && ch->room && IS_SET (ch->room->room_flags, OOC) )
		return NULL;

	
	argument = one_argument (argument, command);
	
	
	snprintf (subcraft_name, MAX_STRING_LENGTH, "%s", argument);

	for ( craft = crafts; craft; craft = craft->next){
		if ( !str_cmp (command, craft->command) &&
			 !str_cmp (subcraft_name, craft->subcraft_name) )
			break;
	}
	
	if ( !craft )
		return NULL;

	if ( !IS_MORTAL(ch) ) {
		for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ ) {
			if ( !(af = get_affect (ch, i)) ){
				break;
			}
			else if ( af->a.craft->subcraft == craft ) {
				return af;
			}
		}

		magic_add_affect (ch, i, -1, 0, 0, 0, 0);
		af = get_affect (ch, i);
		af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);
		af->a.craft->subcraft = craft;
	}       

	else for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ ) {
		if ( !(af = get_affect (ch, i)) ){
			continue;
		}

		if ( af->a.craft->subcraft == craft ){
			break;
		}

		if ( i > CRAFT_LAST )
			return NULL;
	}
	return af;
}

void craft_command (CHAR_DATA *ch, char *command_args,
                                        AFFECTED_TYPE *craft_affect)
{
        char                    *argument;
        char                    buf [MAX_STRING_LENGTH] = {'\0'};
        CHAR_DATA               *target_ch = NULL;
        OBJ_DATA                *target_obj = NULL;
        SUBCRAFT_HEAD_DATA *subcraft;
        AFFECTED_TYPE           *af;
        bool                    sectors = FALSE, pass = FALSE;
        int                     i = 0;
                 
        subcraft = craft_affect->a.craft->subcraft;
                
        argument = one_argument (command_args, buf);            /* Toss subcraft name */
                                
        argument = one_argument (argument, buf);                        /* Target */
                         
        if ( *buf ) {
                if ( IS_SET (subcraft->subcraft_flags, SCF_TARGET_SELF) ) {
                        snprintf (buf, MAX_STRING_LENGTH,  "'%s %s' can only be done on you.\n",
                                        subcraft->command, subcraft->subcraft_name);
                        send_to_char (buf, ch);
                        return;
                }  
        
                else if ( IS_SET (subcraft->subcraft_flags, SCF_TARGET_OBJ) ) {
                        if ( !(target_obj = get_obj_in_dark (ch, buf, ch->right_hand)) &&
                                !(target_obj = get_obj_in_dark (ch, buf, ch->left_hand)) ) {
                                send_to_char ("You don't have that in your inventory.\n", ch);
                                return;
                        }
                }
        
                else if ( IS_SET (subcraft->subcraft_flags, SCF_AREA) ) {
                        send_to_char ("This is an area spell; it takes no target.\n", ch);
                        return;
                }
                                
                else if ( IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR) ) {
                        if ( !(target_ch = get_char_room_vis (ch, buf)) ) {
                                send_to_char ("Your target isn't here.\n", ch);
                                return;
                        }
                }
        } else
                target_ch = ch;
                                                                        
        for ( af = ch->hour_affects; af; af = af->next ) {
                if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST && af->a.craft->timer ) {
                        send_to_char ("You are already crafting something.\n", ch);
                        return;
                }
        }
        
        craft_affect->a.craft->target_ch = target_ch;
        craft_affect->a.craft->target_obj = target_obj;
                 
        if ( get_affect (ch, MAGIC_CRAFT_DELAY) && craft_affect->a.craft->subcraft->delay && IS_MORTAL (ch) ) {
                act ("Sorry, but your OOC craft delay timer is still in place. You'll receive a notification when it expires and you're free to craft delayed items again.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
                return;
        }                       
        
        for ( i = 0; i <= MAX_SECTOR; i++ )
                if ( craft_affect->a.craft->subcraft->sectors[i] )
                        sectors = TRUE;
                        
        if ( sectors ) {
                for ( i = 0; i <= MAX_SECTOR; i++ )
                        if ( craft_affect->a.craft->subcraft->sectors[i]-1 == ch->room->sector_type )
                                pass = TRUE;
                if ( !pass ) {
                        send_to_char ("That craft cannot be performed in this sort of terrain.\n", ch);
                        return;
                }
        }
                                
        for ( i = 0; i <= 5; i++ )
                if ( craft_affect->a.craft->subcraft->seasons[i] )
                        sectors = TRUE;
                
        if ( sectors ) {
                for ( i = 0; i <= 5; i++ )
                        if ( craft_affect->a.craft->subcraft->seasons[i]-1 == time_info.season )
                                pass = TRUE;
                if ( !pass ) {
                        send_to_char ("That craft cannot be performed during this season.\n", ch);
                        return;
                }
        }
                 
        if ( missing_craft_items (ch, craft_affect) )
                return;
                                
        craft_affect->a.craft->phase = NULL;
 
        player_log (ch, craft_affect->a.craft->subcraft->command, craft_affect->a.craft->subcraft->subcraft_name);

       	activate_phase (ch, craft_affect);
}

void do_materials (CHAR_DATA *ch, char *argument, int cmd)
{
	int 		i = 0;
	int			clan_flags = 0;
	int			phase_num = 0;
	AFFECTED_TYPE		*af = NULL;
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	PHASE_DATA 			*phase = NULL;
	char  		buf [MAX_STRING_LENGTH] = {'\0'};
	char 		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char  		clan_name [MAX_STRING_LENGTH] = {'\0'};
	bool		flag_chk = FALSE;
	char   		*p = NULL;
	char		*temp_arg = NULL;
	CHAR_DATA	*temp_char = NULL;
	CLAN_DATA 	*clan_def;
	bool   		first = TRUE;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("For which craft did you wish to obtain a materials list?\n", ch);
		return;
	}

	for ( craft = crafts;
		  craft && str_cmp (craft->subcraft_name, buf);
		  craft = craft->next )
		; /*cycle to the end*/

	if ( !craft ) {
		send_to_char ("No such craft.  Type 'crafts' for a listing.\n", ch);
		return;
	}

	for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ ) {
		if ( !(af = get_affect (ch, i)) )
			continue;

		if ( af->a.craft->subcraft == craft )
			break;
	}

	if ( i > CRAFT_LAST && IS_MORTAL(ch) ) {
		send_to_char ("That craft is not one your character has knowledge of.\n", ch);
		return;
	}

/** IMM information **/
	if ( !IS_MORTAL(ch) ) {
		*buf = '\0';   
		for ( i = 0; i <= 24; i++ ) {
			if ( craft->opening[i] > 0 ){
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s", skill_data[craft->opening[i]].skill_name);
			}
		}

		if ( *buf ) {
			snprintf (buf2, MAX_STRING_LENGTH,  "#6Opening Craft For:#0%s\n", buf);
			send_to_char (buf2, ch);
		}
	}

/** Race check **/
	for ( i = 0; i <= LAST_PC_RACE; i++ ) {
		if ( craft->race[i] )
			flag_chk = TRUE;
	}

	if ( flag_chk ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#6Available To:#0 ");

		for ( i = 0; i <= LAST_PC_RACE; i++ ){
			if ( craft->race[i] ){
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s ", db_race_table[craft->race[i]-1].name);
			}
		}
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
		send_to_char (buf, ch);
	}

/** Clan check **/
	flag_chk = FALSE;
        
	if ( craft->clans ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#6Required Clans:#0 ");
		p = craft->clans;
		
		while ( get_next_clan (&p, clan_name, &clan_flags) ) {
			clan_def = get_clandef (clan_name);

			if ( !clan_def )
				continue;

			if ( !first ){
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "                ");
				}
			temp_arg = get_clan_rank_name(clan_flags);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%-30s [%s]\n", clan_def->literal, temp_arg);
			first = FALSE;
		}
		
		send_to_char (buf, ch);
	}

/** Sector check **/
	flag_chk = FALSE;
	
	for ( i = 0; i <= MAX_SECTOR; i++ ) {
		if ( craft->sectors[i] ){
			flag_chk = TRUE; 
		}
	}
	if ( flag_chk ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#6Only Usable In:#0 ");

		for ( i = 0; i <= MAX_SECTOR; i++ ){
			if ( craft->sectors[i] ){
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s ", 
sector_types[craft->sectors[i]-1]);
			}
		}
		
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
		send_to_char (buf, ch);
	}

/** Season Check **/
	flag_chk = FALSE;

	for ( i = 0; i <= MAX_SEASONS; i++ ) {
		if ( craft->seasons[i] ){
			flag_chk = TRUE; 
		}
	}
	
	if ( flag_chk ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#6Usable During:#0 ");

	for ( i = 0; i <= MAX_SEASONS; i++ ){
		if ( craft->seasons[i] ){
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s ", seasons[craft->seasons[i]-1]);
		}
	}

	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
	send_to_char (buf, ch);
	}
	
/** Phases **/
	flag_chk = FALSE;
        
	for ( phase = craft->phases; phase; phase = phase->next ) {

		phase_num++;

		snprintf (buf, MAX_STRING_LENGTH,  "#5Phase %d:#0  %d seconds", phase_num, phase->phase_seconds);

		if ( phase->skill ) {      
			snprintf (ADDBUF, MAX_STRING_LENGTH, ", %s skill utilized.\n", skill_data[phase->skill].skill_name);
			  
		}
		
		else if ( phase->attribute > 0 ) {      
			snprintf (ADDBUF, MAX_STRING_LENGTH, ", %s attribute tested.\n", attrs[phase->attribute]);
			 
		}

		else {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "\n");
			  
		}
		
		send_to_char (buf, ch);
		
		if ( phase->tool ){
			missing_item_msg (ch, phase->tool, "Tool required:  ");
		}

		for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {
			if ( craft->items [i] &&
				 craft->items [i]->phase == phase &&
				!IS_SET (craft->items [i]->flags, SUBCRAFT_PRODUCED) ) {

					if (IS_SET (craft->items[i]->flags, SUBCRAFT_HELD) ||   
						IS_SET (craft->items[i]->flags, SUBCRAFT_IN_INVENTORY) )
						missing_item_msg (ch, craft->items [i],
										"Held (Reusable):  ");
					else if ( IS_SET (craft->items[i]->flags, SUBCRAFT_IN_ROOM) )
						missing_item_msg (ch, craft->items [i],
										"In Room (Reusable):  ");
					else if ( IS_SET (craft->items[i]->flags, SUBCRAFT_USED) )
						missing_item_msg (ch, craft->items [i],
										"Held or in Room (Consumed):  ");
			}

			else if ( craft->items [i] &&
					  craft->items [i]->phase == phase &&
					  IS_SET (craft->items [i]->flags, SUBCRAFT_PRODUCED) )
						missing_item_msg (ch, craft->items [i],
										"#6Produced:#0  ");
		}

		if ( phase->load_mob && vtom(phase->load_mob) ) {
			temp_char = vtom(phase->load_mob);
			temp_arg = char_short(temp_char);
			snprintf (buf, MAX_STRING_LENGTH,  "#6Produced:#0 #5%s#0\n", temp_arg);
			send_to_char (buf, ch);
		}

		if ( GET_TRUST (ch) ) {
			if ( phase->attribute > 0 ) {
				snprintf (buf, MAX_STRING_LENGTH,  "  Attribute %s vs %dd%d\n",
						attrs [phase->attribute],
						phase->dice,
						phase->sides);
				send_to_char (buf, ch);
			}

			if ( phase->skill ) {
				snprintf (buf, MAX_STRING_LENGTH,  "  Skill %s vs %dd%d\n",
						skill_data[phase->skill].skill_name,
						phase->dice,
						phase->sides);
				send_to_char (buf, ch);
			}
		}
	}

	if ( craft->delay ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#6OOC Delay Timer:#0 %d RL Hours\n", craft->delay);
		send_to_char (buf, ch);
	}
}

void do_crafts (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch = NULL;
	int				has_a_craft = 0;
	int				i = 0;
	int				j = 0;
	AFFECTED_TYPE 	*af = NULL;
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	SUBCRAFT_HEAD_DATA	*tcraft = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char 			name [MAX_STRING_LENGTH] = {'\0'};
	char 			output [MAX_STRING_LENGTH] = {'\0'};
	char  			craft_name [MAX_STRING_LENGTH] = {'\0'};
	char 			subcraft [MAX_STRING_LENGTH] = {'\0'};
	char 			command [MAX_STRING_LENGTH] = {'\0'};
	bool  			close_chk = FALSE;
	bool			opening_chk = FALSE;
	bool			category_chk = FALSE;
	bool			sectors_chk = FALSE;
	bool			seasons_chk = FALSE;
	bool			flag_chk = FALSE;
                                 
/** NPC? **/                                 
        if ( IS_NPC (ch) ) {
                send_to_char ("You shouldn't be using this command...\n", ch);
                return;
        }
                        
        argument = one_argument (argument, buf);

/** Mortals checking their available crafts **/
        if ( IS_MORTAL (ch) ) {
                if ( !*buf ) {
                        snprintf (output, MAX_STRING_LENGTH,  "You currently have crafts in the following areas:\n\n");
                        for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ ) {
                                if ( (af = get_affect (ch, i)) ) {
                                        if ( !strstr (output, af->a.craft->subcraft->craft_name) ) {
						j++;
						snprintf (output + strlen(output), MAX_STRING_LENGTH,  "   #6%-20s#0", af->a.craft->subcraft->craft_name);
						if ( !(j % 3) ){
							snprintf (output + strlen(output), MAX_STRING_LENGTH,  "\n");
					}
                                }
					has_a_craft = 1;
                        }
			} /* for i = CRAFT_FIRST*/

			if ( (j % 3) ){
				snprintf (output + strlen(output), MAX_STRING_LENGTH,  "\n");
			}

			if ( !has_a_craft ){
                                send_to_char ("You have no knowledge of any crafts.\n", ch);
                }
                else {
				page_string (ch->desc, output);
			}
		} /*if ( !*buf )*/
                else {
			j = 0;
			if ( !str_cmp (buf, "all") ){
	                        snprintf (output, MAX_STRING_LENGTH,  "You know the following crafts:\n\n");
			}
			else {
	                        snprintf (output, MAX_STRING_LENGTH,  "You know the following #6%s#0 crafts:\n\n", buf);
			}

                        for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ ) {
                                if ( (af = get_affect (ch, i)) ) {
					if ( str_cmp (buf, "all") &&
					     str_cmp (buf, af->a.craft->subcraft->craft_name) ){
						continue;
					}
					
					j++;
					snprintf (name, MAX_STRING_LENGTH, "%s %s", af->a.craft->subcraft->command, af->a.craft->subcraft->subcraft_name);
                                        snprintf (output + strlen(output), MAX_STRING_LENGTH,  "   #6%-30s#0", name);
                                        has_a_craft = 1;
					
					if ( !(j % 2) ){
						snprintf (output + strlen(output), MAX_STRING_LENGTH,  "\n");
                                }
                        }
			} /* for ( i = CRAFT_FIRST */

			if ( (j % 2) )
				snprintf (output + strlen(output), MAX_STRING_LENGTH,  "\n");

			if ( !has_a_craft ){
                                send_to_char ("You have no knowledge of any crafts.\n", ch);
			}
			else {
				page_string (ch->desc, output);
			}
                }
                                
                return;  
	} /* if ( IS_MORTAL (ch) */
                
/** Immortals commands for crafts **/
/** List all crafts **/
        if ( (!*buf && !ch->pc->edit_craft) || !str_cmp (buf, "list") ) {
                list_all_crafts (ch);
                return;
        }
                
/** Start a new craft **/
        else if ( !str_cmp (buf, "new") ) {
                argument = one_argument (argument, buf);
                snprintf (craft_name, MAX_STRING_LENGTH, "%s", buf);
		
                argument = one_argument (argument, buf);
                snprintf (subcraft, MAX_STRING_LENGTH, "%s", buf);

                argument = one_argument (argument, buf);
                snprintf (command, MAX_STRING_LENGTH, "%s", buf);
                
                if ( !*buf || !*craft_name || !*subcraft || !*command ) {
                        send_to_char ("The syntax is as follows: craft new <craft> <subcraft> <command>\n", ch);
                        return;
                }

                for ( craft = crafts; craft; craft = craft->next ) {
                        if ( !craft->next ) {
                                CREATE (craft->next, SUBCRAFT_HEAD_DATA, 1);
                                craft = craft->next;
                                break;
                        }
                }

                craft->craft_name = add_hash(craft_name);
                craft->subcraft_name = add_hash(subcraft);
                craft->command = add_hash(command);
                ch->pc->edit_craft = craft;
                send_to_char ("New craft initialized and opened for editing.\n", ch);
		mysql_safe_query ("INSERT INTO new_crafts VALUES ('%s', '%s', '%s', '%s')", craft->command, craft->subcraft_name, craft->craft_name, ch->tname);
                return;
        }
         
/** Clone a craft **/
        if ( !str_cmp (buf, "clone") ) {
                if ( !ch->pc->edit_craft ) {
                        send_to_char ("You'll need to open the craft you want to clone, first.\n", ch);
                        return;
                }

                argument = one_argument (argument, buf);
                if ( !*buf ) {
                        send_to_char ("You'll need to specify a craft name for the cloned craft.\n", ch);
                        return;
                }
		else {
			snprintf (craft_name, MAX_STRING_LENGTH, "%s", buf);
		}
		
                argument = one_argument (argument, buf);
                if ( !*buf ) {
                        send_to_char ("You'll need to specify a subcraft name for the cloned craft.\n", ch);
                        return;
                }
                else if ( !str_cmp (buf, ch->pc->edit_craft->subcraft_name) ) {
                        send_to_char ("You'll need to specify a different subcraft name for the cloned craft.\n", ch);
                        return;
                }
		else {
			snprintf (subcraft, MAX_STRING_LENGTH, "%s", buf);
		}

                argument = one_argument (argument, buf);
                if ( !*buf ) {
                        send_to_char ("You'll need to specify a command for the cloned craft.\n", ch);
                        return;
                }
		else {
			snprintf (command, MAX_STRING_LENGTH, "%s", buf);
		}
		
                CREATE (tcraft, SUBCRAFT_HEAD_DATA, 1);
                memset (tcraft, 0, sizeof(SUBCRAFT_HEAD_DATA));
                memcpy (tcraft, ch->pc->edit_craft, sizeof (SUBCRAFT_HEAD_DATA));
                CREATE (tcraft->phases, PHASE_DATA, 1);
                memset (tcraft->phases, 0, sizeof(PHASE_DATA));
                memcpy (tcraft->phases, ch->pc->edit_craft->phases, sizeof (PHASE_DATA));
		
                if (ch->pc->edit_craft->items > 0) {
                        for ( i = 0; ch->pc->edit_craft->items[i]; i++ ) {
                                CREATE (tcraft->items[i], DEFAULT_ITEM_DATA, 1);
                                memset (tcraft->items[i], 0, sizeof(DEFAULT_ITEM_DATA));
                        }
                        memcpy (tcraft->items, ch->pc->edit_craft->items, sizeof (DEFAULT_ITEM_DATA));
                }

                ch->pc->edit_craft = NULL;
                tcraft->craft_name = add_hash(craft_name);
                tcraft->subcraft_name = add_hash(subcraft);
                tcraft->command = add_hash(command);
                tcraft->next = NULL;

                for ( craft = crafts; craft; craft = craft->next ) {
                        if ( !craft->next ) {
                                CREATE (craft->next, SUBCRAFT_HEAD_DATA, 1);
                                craft->next = tcraft;
                                ch->pc->edit_craft = tcraft;
                                send_to_char ("Craft cloned; new craft opened for editing.\n", ch);
				mysql_safe_query ("INSERT INTO new_crafts VALUES ('%s', '%s', '%s', '%s')", tcraft->command, tcraft->subcraft_name, tcraft->craft_name, ch->tname);
                                return;
                        }
                }
        }
         
/** Look for sub-crafts within craft category **/
	for ( craft = crafts; craft; craft = craft->next ) {
		if ( !str_cmp (craft->subcraft_name, buf) ){
                        break;
		}
                if ( !str_cmp (craft->craft_name, buf) ) {
			flag_chk = TRUE;
                        break;
                }
        }
         
	if ( flag_chk ) {
                snprintf (output, MAX_STRING_LENGTH,  "\nWe have the following #6%s#0 crafts:\n\n", buf);
		
                for ( craft = crafts; craft; craft = craft->next ) {
			if ( !str_cmp (craft->craft_name, buf) ){
				snprintf (output + strlen(output), MAX_STRING_LENGTH,  "#6Craft:#0 %-12s #6Subcraft:#0 %-20s #6Command:#0 %-10s\n", craft->craft_name, craft->subcraft_name, craft->command);
			}
                }
                page_string (ch->desc, output);
                return;
        }
         

/** Check for crafts on a loaded PC **/
        if ( (tch = load_pc(buf)) ) {
                snprintf (output, MAX_STRING_LENGTH,  "\n#6%s#0 has the following crafts on %s pfile:\n\n", tch->tname, HSHR(tch));

                for ( af = tch->hour_affects; af; af = af->next ) {
                        if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST ) {
				snprintf (output + strlen(output), MAX_STRING_LENGTH,  "#6Craft:#0 %-12s #6Subcraft:#0 %-20s #6Command:#0 %-10s\n", af->a.craft->subcraft->craft_name, af->a.craft->subcraft->subcraft_name, af->a.craft->subcraft->command);
                        }
                }

                unload_pc (tch);
                page_string (ch->desc, output);
                return;
        }
         
/** Opening a craft for editing **/
	if ( !craft && *buf && !category_chk ){ 
		close_chk = TRUE;
	}
                
	if ( ch->pc->edit_craft ){
                craft = ch->pc->edit_craft;
	}
                
	if ( !craft && !category_chk ) {
                send_to_char ("No such subcraft.\n", ch);
                return;
        }
         
        if ( !ch->pc->edit_craft || ch->pc->edit_craft != craft ) {
                ch->pc->edit_craft = craft;
                send_to_char ("Craft has been opened for editing.\n", ch);
                return;
        }
	else if ( ch->pc->edit_craft && close_chk ) {
                send_to_char ("Craft closed and written to memory.\n", ch);
                ch->pc->edit_craft = NULL;
                return;
        }
         
/** Listing the craft **/
        *b_buf = '\0';
        snprintf (b_buf, MAX_STRING_LENGTH, "#6Craft:#0 %s #6Subcraft:#0 %s #6Command:#0 %s\n", craft->craft_name, craft->subcraft_name, craft->command);
        
/** Race listing **/
        for ( i = 0; i <= LAST_PC_RACE; i++ ) {
                if ( craft->race[i] ) {
			opening_chk = TRUE;
                }
        }
         
	if ( opening_chk ){
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  #6Race:#0");
		
                for ( i = 0; i <= LAST_PC_RACE; i++ ) {
                	if ( craft->race[i] ) {
                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  %s", db_race_table[craft->race[i]-1].name);
                    }
				} 
		
				snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "\n");
        }
        
/** clan listing **/
	if ( craft->clans ){
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  #6Clans:#0 %s\n", craft->clans);
	}
                
	opening_chk = FALSE;
        *buf = '\0';
        for ( i = 0; i <= 24; i++ ) {
                if ( craft->opening[i] ) {
			opening_chk = TRUE;   
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s", skill_data[craft->opening[i]].skill_name);
                }
        }
         
/** Sector listing **/
	if ( opening_chk ){
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  #6Base:#0  %s\n", buf);
	}
                
	for ( i = 0; i <= MAX_SECTOR; i++ ){
		if ( craft->sectors [i] ){
			sectors_chk = TRUE;
		}
	}
                        
	if ( sectors_chk ) {
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  #6Sectors:#0 ");
		for ( i = 0; i <= MAX_SECTOR; i++ ){
			if ( craft->sectors[i] ){
                                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%s ", sector_types[craft->sectors[i]-1]);
			}
		}

                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "\n");
        }
         
/** Season listing **/
	for ( i = 0; i <= 5; i++ ){
		if ( craft->seasons [i] ){
			seasons_chk = TRUE;
		}
	}
                        
	if ( seasons_chk ) {
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  #6Seasons:#0 ");
		for ( i = 0; i <= 5; i++ ){
			if ( craft->seasons[i] ){
                                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%s ", seasons[craft->seasons[i]-1]);
			}
		}
		
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "\n");
        }
         
/** Delay listing **/
	if ( craft->delay ){
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  #6OOC Delay Timer:#0 %d RL Hours\n", craft->delay);
	}
                
/** Failure listings **/	
	if ( craft->failure ) {
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  #6Fail:#0 %s\n", craft->failure);
	}
                
	if ( craft->failobjs ){
                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  #6Failobjs:#0 %s\n", craft->failobjs);
	}
                
        display_craft (ch, craft);
        
	return;
}      

void do_remcraft (CHAR_DATA *ch, char *argument, int cmd)
{
        CHAR_DATA                       *edit_mob;
        AFFECTED_TYPE           *af;
        AFFECTED_TYPE           *next_affect;
        SUBCRAFT_HEAD_DATA      *craft;
        char                            buf [MAX_STRING_LENGTH] = {'\0'};
        bool                    craft_category = FALSE;
                        
        argument = one_argument (argument, buf);
         
        if ( *buf == '?' ) {
                s ("Start by using the mob command on a PC.");
                s ("");
                s ("remcraft <subcraft-name>            Use crafts to get names");
                s ("");
                s ("   remcraft swordblank ");
                s ("");
                return;
        }
 
        if ( !(edit_mob = ch->pc->edit_player) ) {
                send_to_char ("Start by using the MOB command on a PC.\n", ch);
                return;
        }
                 
        for ( craft = crafts;
                  craft;
                  craft = craft->next ) {
                if ( !str_cmp (craft->subcraft_name, buf) )
                        break;
                if ( !str_cmp (craft->craft_name, buf) ) {
                        craft_category = TRUE;
                        break;
                }
        }
 
	if ( !str_cmp (buf, "all") )
		craft_category = TRUE;

        if ( !craft && !craft_category ) {
                send_to_char ("No such craft.  Type 'crafts' for a listing.\n", ch);
                return;
        }
 
        for(af=edit_mob->hour_affects; af; af = next_affect) {
        
            next_affect = af->next; 
        
            if(af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST) {
                if ( !str_cmp (buf, "all") || !str_cmp (af->a.craft->subcraft->craft_name, buf) ) {
                        remove_affect_type (edit_mob, af->type);
                        continue;
                }
                if(!str_cmp(af->a.craft->subcraft->subcraft_name, buf)) {
                    remove_affect_type(edit_mob, af->type);
                    send_to_char ("Ok.\n", ch);
                    break;
                }       
            }
        }
        
        if ( craft_category )
                send_to_char ("Craft category removed.\n", ch);
}

void do_addcraft (CHAR_DATA *ch, char *argument, int cmd)
{
        int                                     i;
        CHAR_DATA                       *edit_mob;
        AFFECTED_TYPE           *af, *next_affect;
        SUBCRAFT_HEAD_DATA      *craft;
        char                            buf [MAX_STRING_LENGTH] = {'\0'};
        bool                    craft_category = FALSE;
        
        argument = one_argument (argument, buf);
                  
        if ( *buf == '?' ) {
                s ("Start by using the mob command on a PC.");
                s ("");
                s ("addcraft <subcraft-name>            Use crafts to get names");
                s ("         <item list 1>,");
                s ("         <item list 2>, ...");
                s ("");
                s ("<item list> is item/flag or (item/flag item/flag ...)");
                s (""); 
                s ("Example:");
        s ("                       v-no item list 1 specified");
                s ("   addcraft swordblank , (- 10001 10002), 10003");
        s ("             item list 2-^^^^^^^^^^^^^^^  ^^^^^-item list 3");
                s ("");
                return;
        }
         
        if ( !(edit_mob = ch->pc->edit_player) ) {
                send_to_char ("Start by using the MOB command on a PC.\n", ch);
                return;
        }
         
        for ( craft = crafts;
                  craft;
                  craft = craft->next ) {
                if ( !str_cmp (craft->subcraft_name, buf) )
                        break;
                if ( !str_cmp (craft->craft_name, buf) )
                        craft_category = TRUE;
        }

	if ( !str_cmp (buf, "all") )
		craft_category = TRUE;
         
        if ( !craft && !craft_category ) {
                send_to_char ("No such craft or subcraft.  Type 'crafts' for a listing.\n", ch);
                return;
        }
         
        for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ )
                if ( !get_affect (edit_mob, i) )
                        break;
                        
        if ( i > CRAFT_LAST ) {
                send_to_char ("Sorry, this PC already has the maximum allowed crafts"
                                          ".\n", ch);
                return;
        }
         
        if ( craft_category ) {
                for ( af = edit_mob->hour_affects; af; af = next_affect ) {
                        next_affect = af->next;
                        if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST ) {
                                if ( !str_cmp (af->a.craft->subcraft->craft_name, buf) )
                                        remove_affect_type (edit_mob, af->type);
                        }
                }
                for ( craft = crafts; craft; craft = craft->next ) {
                        if ( !strcmp ("all", buf) || !strcmp (craft->craft_name, buf) ) {   
                                for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ )
                                        if ( !get_affect (edit_mob, i) )
                                                break;
                                magic_add_affect (edit_mob, i, -1, 0, 0, 0, 0);
                                af = get_affect (edit_mob, i);
                                af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);
                                af->a.craft->subcraft = craft;
                        }
                }
        }
        else {
                magic_add_affect (edit_mob, i, -1, 0, 0, 0, 0);
                af = get_affect (edit_mob, i);
                af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);
                af->a.craft->subcraft = craft;
        }
         
        send_to_char ("Craft(s) added.\n", ch);
}

void update_crafts (CHAR_DATA *ch)
{
	SUBCRAFT_HEAD_DATA	*craft;
	AFFECTED_TYPE		*af;
        int                     i = 0;

	for ( craft = crafts; craft; craft = craft->next ) {
        	if ( !strcmp (craft->craft_name, "general") && !has_craft (ch, craft) && has_required_crafting_skills (ch, craft) ) {
                	for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ )
                        	if ( !get_affect (ch, i) )
                                	break;
                        magic_add_affect (ch, i, -1, 0, 0, 0, 0);
                        af = get_affect (ch, i);
                        af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);
                        af->a.craft->subcraft = craft;
                }
                if ( is_opening_craft (ch, craft) && has_required_crafting_skills (ch, craft) && !has_craft (ch, craft) ) {
              		for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ )
                        	if ( !get_affect (ch, i) )
                	                break;
                        magic_add_affect (ch, i, -1, 0, 0, 0, 0);
                        af = get_affect (ch, i);
                        af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);
                        af->a.craft->subcraft = craft;
                }
	}
}

AFFECTED_TYPE *is_crafting (CHAR_DATA *ch)
{
	AFFECTED_TYPE *af, *next_af;

        for ( af = ch->hour_affects; af; af = next_af ) {
                next_af = af->next;
                if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST && af->a.craft->timer ) {
			return af;
                }
        }

	return NULL;
}

int has_required_crafting_skills (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
	PHASE_DATA		*phase;
	char			clan_name [MAX_STRING_LENGTH] = {'\0'};
	int			i, flags = 0, flags2 = 0;
	char			*c1;
	bool			required_race = FALSE, required_clan = FALSE;

	for ( i = 0; i <= 24; i++ ) {
		if ( craft->race[i] )
			required_race = TRUE;
	}

	if ( required_race ) {
		for ( i = 0; i <= 24; i++ ) {
			if ( craft->race[i] && ch->race == craft->race[i] - 1 )
				required_race = FALSE;
		}
	}

	if ( required_race )
		return 0;

	if ( craft->clans && strlen(craft->clans) > 3 ) {
		required_clan = TRUE;
	        for ( c1 = craft->clans; get_next_clan (&c1, clan_name, &flags);) {
                	if ( get_clan (ch, clan_name, &flags2) && flags2 >= flags )
                        	required_clan = FALSE;
        	}
	}

	if ( required_clan )
		return 0;

	for ( i = 0; i <= 24; i++ ) {
		if ( craft->opening[i] > 0 && !real_skill (ch, craft->opening[i]) )
			return 0;
	}

	for ( phase = craft->phases; phase; phase = phase->next ) {
		if ( phase->req_skill ) {
			if ( !real_skill (ch, phase->req_skill) )
				return 0;
		}
		if ( phase->skill ) {
			if ( !real_skill (ch, phase->skill) )
				return 0;
		}
	}

	return 1;

}

int is_opening_craft (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
	int		i = 0;

	for ( i = 0; i <= 24; i++ ) {
		if ( craft->opening[i] > 0 && real_skill (ch, craft->opening[i]) )
			return 1;
	}

	return 0;

}

char *craft_argument (char *argument, char *arg_first)
{
	char cEnd;

	if ( argument == NULL )
		return "";
    
	while ( isspace( *argument ) )
		argument++;

	if ( *argument == '(' ) {
		argument++;
		strcpy (arg_first, "(");
		return argument;
	}

	if ( *argument == ')' ) {
		argument++;
		strcpy (arg_first, ")");
		return argument;
	}

	cEnd = ' ';

	while ( *argument != '\0' ) {

		if ( *argument == '(' || *argument == ')' )
			break;

		if ( *argument == cEnd ) {
			argument++;
			break;
		}

		if ( cEnd == ' ' )
			*arg_first = LOWER (*argument);
		else
			*arg_first = *argument;

		arg_first++;
		argument++;
	}

	*arg_first = '\0';

	while ( isspace( *argument ) )
		argument++;

	return argument;
}

void add_to_default_list (DEFAULT_ITEM_DATA *items, char *flag_vnum)
{
	int				ind;
	int				ovnum;
	int				i;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !items->item_counts )
		items->item_counts = 1;

	if ( *flag_vnum == '(' || *flag_vnum == ')' )
		return;

		/* '-' means to erase all vnums */

	if ( *flag_vnum == '-' ) {
		memset (items->items, 0, MAX_DEFAULT_ITEMS);
		return;
	}

	if ( (*flag_vnum == 'x' || *flag_vnum == 'X') && isdigit (flag_vnum [1]) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%c", flag_vnum[1]);
		if ( flag_vnum[2] && isdigit(flag_vnum[2]) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%c", flag_vnum[2]);
		if ( flag_vnum[3] && isdigit(flag_vnum[3]) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%c", flag_vnum[3]);
		if ( flag_vnum[4] && isdigit(flag_vnum[4]) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%c", flag_vnum[4]);
		items->item_counts = strtol(buf, NULL, 10);
		return;
	}

	if ( !isdigit (*flag_vnum) ) {
		if ( (ind = index_lookup (item_default_flags, flag_vnum)) == -1 ) {
			;
		} else
			SET_BIT (items->flags, 1 << ind);

		return;
	}

	ovnum = strtol(flag_vnum, NULL, 10);

	if ( !ovnum || !vtoo (ovnum) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "NOTE:  vnum %s does not exist for CRAFTS!", flag_vnum);
		system_log (buf, TRUE);
	}

	for ( i = 0; i < MAX_DEFAULT_ITEMS; i++ ) {

		if ( !items->items [i] ) {
			items->items [i] = ovnum;
			break;
		}

		if ( items->items [i] == ovnum ) {
			break;
		}
	}

	if ( i >= MAX_DEFAULT_ITEMS ) {
		system_log ("WARNING:  Too many items specified in default item list!", TRUE);
		snprintf (buf, MAX_STRING_LENGTH,  "Item %d not added.  Max allowed in list is %d items.",
				 ovnum, MAX_DEFAULT_ITEMS);
		system_log (buf, TRUE);
	}
}

char *read_item_list (DEFAULT_ITEM_DATA **items, char *list, PHASE_DATA *phase)
{
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				*argument;
	DEFAULT_ITEM_DATA	*deflt;

	*items = (DEFAULT_ITEM_DATA *)alloc (sizeof (DEFAULT_ITEM_DATA), 27);

	deflt = *items;

	deflt->phase = phase;

	argument = craft_argument (list, buf);

	if ( !*buf )
		return argument;

	if ( *buf != '(' ) {
		add_to_default_list (deflt, buf);
		return argument;
	}

	do {
		argument = craft_argument (argument, buf);
		add_to_default_list (deflt, buf);
	} while ( *buf != ')' );

	return argument;
}

PHASE_DATA *new_phase ()
{
	PHASE_DATA		*phase;

	phase = (PHASE_DATA *)alloc (sizeof (PHASE_DATA), 26);
	phase->attribute = -1;

	return phase;
}

char *read_extended_text (FILE *fp, char *first_line)
{
        int                     continues;
        char            line [MAX_STRING_LENGTH] = {'\0'};
        char            buf [MAX_STRING_LENGTH] = {'\0'};
        char            *buf_end = {'\0'};
        char            *buf_start = {'\0'};

	*buf = '\0';
	*line = '\0';

        strcpy (buf, first_line);

        *line = '\0';

        while ( 1 ) {

                continues = 0;

                for ( buf_start = buf; *buf_start == ' '; )
                        buf_start++;

                buf_end = buf_start + strlen (buf_start) - 1;

                while ( 1 ) {

                        if ( buf_end == buf_start )
                                break;

                        if ( *buf_end == ' ' ||
                                 *buf_end == '\n' ||
                                 *buf_end == '\r' )
                                buf_end--;
        
                        else if ( *buf_end == '\\' ) {
                                continues = 1;
                                *buf_end = '\0';
                                break;
                        }
        
                        else
                                break;

                        buf_end [1] = '\0';
                }
                
                strcat (line, buf_start);

                if ( !continues )
                        break;
                
                (void)fgets (buf, MAX_STRING_LENGTH - 1, fp);
        }
                                
        return str_dup (line);
}

void subcraft_line (FILE *fp_reg, char *line)
{
	int					ind;
	int					items_num;
	int					sn, i;
	char				c [10];
	char				*argument = NULL;
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	PHASE_DATA			*phase = NULL;
	SUBCRAFT_HEAD_DATA	*subcraft;


	argument = one_argument (line, buf);

	if ( !str_cmp (buf, "craft") ) {

		if ( !crafts ) {
			crafts = subcraft = (SUBCRAFT_HEAD_DATA *)alloc (sizeof (SUBCRAFT_HEAD_DATA), 25);
			memset(crafts, 0, sizeof(SUBCRAFT_HEAD_DATA));
		} else {
			for ( subcraft = crafts; subcraft->next; subcraft = subcraft->next )
			;

			subcraft->next = (SUBCRAFT_HEAD_DATA *)alloc (sizeof (SUBCRAFT_HEAD_DATA), 25);
			subcraft = subcraft->next;
			memset(subcraft, 0, sizeof(SUBCRAFT_HEAD_DATA));
		}

		subcraft->crafts_start = ftell (fp_reg) - strlen (line) - 1;

		subcraft->clans = add_hash ("");

		argument = one_argument (argument, buf);
		subcraft->craft_name = str_dup (buf);

		argument = one_argument (argument, buf);

		if ( str_cmp (buf, "subcraft") ) {
			system_log ("Missing 'subcraft' on craft line:", TRUE);
			system_log (line, TRUE);
			system_log (buf, TRUE);
			abort();
		}

		argument = one_argument (argument, buf);
		subcraft->subcraft_name = str_dup (buf);

		argument = one_argument (argument, buf);

		if ( str_cmp (buf, "command") ) {
			system_log ("Missing 'command' on craft line:", TRUE);
			system_log (line, TRUE);
			system_log (buf, TRUE);
			abort();
		}

		argument = one_argument (argument, buf);
		subcraft->command = str_dup (buf);

		return;
	}

	if ( !crafts ) {
		system_log ("Missing first 'craft' line.", TRUE);
		system_log (line, TRUE);
		abort();
	}


		/* Determine current subcraft */

	for ( subcraft = crafts; subcraft->next; subcraft = subcraft->next )
		;

	if ( !subcraft->phases &&
		 (!str_cmp (buf, "fail:") ||
		  !str_cmp (buf, "failure:")) ) {
		subcraft->failure = read_extended_text (fp_reg, argument);
		return;
	}

	if ( !subcraft->phases && 
		(!str_cmp (buf, "failobj:") ||
		 !str_cmp (buf, "failobjs:")) ) {
		subcraft->failobjs = read_extended_text (fp_reg, argument);
		return;
	}


	if ( !subcraft->phases && !str_cmp (buf, "flags:") ) {

		argument = one_argument (argument, buf);

		while ( *buf ) {

			if ( (ind = index_lookup (subcraft_flags, buf)) != -1 )
				SET_BIT (subcraft->subcraft_flags, 1 << ind);
			else {
				system_log ("Subcraft_flags error (line, subcraft):", TRUE);
				system_log (line, TRUE);
				system_log (subcraft->subcraft_name, TRUE);
			}

			argument = one_argument (argument, buf);
		}

		if ( IS_SET (subcraft->subcraft_flags, SCF_OFFENSIVE) &&
			 IS_SET (subcraft->subcraft_flags, SCF_DEFENSIVE) ) {
			REMOVE_BIT (subcraft->subcraft_flags, SCF_DEFENSIVE);
		}

		if ( IS_SET (subcraft->subcraft_flags, SCF_TARGET_OBJ) &&
			 IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR) ) {
			REMOVE_BIT (subcraft->subcraft_flags, SCF_DEFENSIVE);
		}

		return;
	}

	else if ( !str_cmp (buf, "sectors:") ) {

		if ( !*argument )
			return;

		argument = one_argument (argument, buf);

		while ( *buf ) {

			if ( (ind = index_lookup (sector_types, buf)) != -1 ) {
				for ( i = 0; i <= MAX_SECTOR; i++ ) {
					if ( !subcraft->sectors[i] ) {
						subcraft->sectors[i] = ind+1;
						break;
					}
				}
			}
			else {
				system_log ("Illegal subcraft sector:", TRUE);
				system_log (buf, TRUE);
			}

			argument = one_argument (argument, buf);
		}
		return;
	}

	else if ( !str_cmp (buf, "seasons:") ) {

		if ( !*argument )
			return;

		argument = one_argument (argument, buf);

		while ( *buf ) {

			if ( (ind = index_lookup (seasons, buf)) != -1 ) {
				for ( i = 0; i <= 5; i++ ) {
					if ( !subcraft->seasons[i] ) {
						subcraft->seasons[i] = ind+1;
						break;
					}
				}
			}

			argument = one_argument (argument, buf);
		}
		return;
	}

	else if ( !str_cmp (buf, "opening:") ) {

		argument = one_argument (argument, buf);

		while ( *buf ) {

			if ( strtol(buf, NULL, 10) <= LAST_SKILL ) {
				for ( i = 0; i <= 24; i++ ) {
					if ( !subcraft->opening[i] ) {
						subcraft->opening[i] = strtol(buf, NULL, 10);
						break;
					}
				}
			}

			argument = one_argument (argument, buf);
		}
		return;
	}

	else if ( !str_cmp (buf, "race:") ) {

		argument = one_argument (argument, buf);

		while ( *buf ) {

			for ( i = 0; i <= LAST_PC_RACE; i++ ) {
				if ( !subcraft->race[i] ) {
					subcraft->race[i] = strtol(buf, NULL, 10);
					break;
				}
			}

			argument = one_argument (argument, buf);
		}
		return;
	}

	else if ( !str_cmp (buf, "clans:") ) {

		subcraft->clans = add_hash (argument);

		return;
	}

	else if ( !str_cmp (buf, "ic_delay:") ) {

		argument = one_argument (argument, buf);

		subcraft->delay = strtol(buf, NULL, 10);

		return;
	}

	if ( !str_cmp (buf, "phase") || !str_cmp (buf, "phase:") ) {
		if ( !subcraft->phases )
			subcraft->phases = new_phase ();
		else {
			for ( phase = subcraft->phases; phase->next; phase = phase->next )
				;

			phase->next = new_phase ();
		}

		return;
	}

		/* Determine current phase */

	if ( subcraft->phases ) {
		for ( phase = subcraft->phases; phase->next; phase = phase->next )
			;
	}

	while ( *argument == ' ' )
		argument++;

	if ( !str_cmp (buf, "end") )
		subcraft->crafts_end = ftell (fp_reg);
		
	else if ( !str_cmp (buf, "1st:") )
		phase->first = read_extended_text (fp_reg, argument);

	else if ( !str_cmp (buf, "2nd:") )
		phase->second = read_extended_text (fp_reg, argument);

	else if ( !str_cmp (buf, "3rd:") )
		phase->third = read_extended_text (fp_reg, argument);

	else if ( !str_cmp (buf, "self:") )
		phase->self = read_extended_text (fp_reg, argument);

	else if ( !str_cmp (buf, "1stfail:") ||
			  !str_cmp (buf, "failure:") )
		phase->failure = read_extended_text (fp_reg, argument);

	else if ( !str_cmp (buf, "2ndfail:") )
		phase->second_failure = read_extended_text (fp_reg, argument);

	else if ( !str_cmp (buf, "3rdfail:") )
		phase->third_failure = read_extended_text (fp_reg, argument);

	else if ( !str_cmp (buf, "f:") ) {

		argument = one_argument (argument, buf);

		while ( *buf ) {

			if ( (ind = index_lookup (phase_flags, buf)) != -1 )
				SET_BIT (phase->flags, 1 << ind);
			else {
				system_log ("Illegal subcraft flags:", TRUE);
				system_log (buf, TRUE);
			}

			argument = one_argument (argument, buf);
		}
	}

	else if ( !str_cmp (buf, "s:") || !str_cmp (buf, "skill:") ) {

		argument = one_argument (argument, buf);

		if ( (sn = index_lookup (attrs, buf)) != -1 )
			phase->attribute = sn;

		else if ( (sn = skill_index_lookup (buf)) == -1 ) {
			system_log ("Illegal skill name in craft:", TRUE);
			system_log (buf, TRUE);
		}

		else
			phase->skill = sn;

		argument = one_argument (argument, buf);

		if ( !str_cmp (buf, "vs") || !str_cmp (buf, "vs.") )
			argument = one_argument (argument, buf);

		if ( 3 != (sscanf (buf, "%d%[dD]%d", &phase->dice, c, &phase->sides))) {
			system_log ("Parm error in skill: (s:) of craft:", TRUE);
			system_log (buf, TRUE);
		}

		if ( phase->skill && (phase->dice < 1 || phase->sides < 1) ) {
			phase->dice = 1;
			phase->sides = 100;
		}
	}

else if ( !str_cmp (buf, "a:") || !str_cmp (buf, "attr:") ) {

		argument = one_argument (argument, buf);

		if ( (sn = index_lookup (attrs, buf)) != -1 )
			phase->attribute = sn;

		else if ( (sn = index_lookup (attrs, buf)) == -1 ) {
			system_log ("Illegal attribute name in craft:", TRUE);
			system_log (buf, TRUE);
		}

		else
			phase->attribute = sn;

		argument = one_argument (argument, buf);

		if ( !str_cmp (buf, "vs") || !str_cmp (buf, "vs.") )
			argument = one_argument (argument, buf);

		if ( 3 != (sscanf (buf, "%d%[dD]%d", &phase->dice, c, &phase->sides))) {
			system_log ("Parm error in attribute: (a:) of craft:", TRUE);
			system_log (buf, TRUE);
		}

		if ( phase->attribute > -1  && (phase->dice < 1 || phase->sides < 1) ) {
			phase->dice = 1;
			phase->sides = 100;
		}

		
	}
	else if ( !str_cmp (buf, "t:") ) {
		argument = one_argument (argument, buf);

		if ( isdigit (*buf) )
			phase->phase_seconds = strtol(buf, NULL, 10);
		else {
			system_log ("Illegal t: (time) value declared in subcraft:", TRUE);
			system_log (buf, TRUE);
		}
	}

	else if ( !str_cmp (buf, "mob:") ) {
		argument = one_argument (argument, buf);

		if ( isdigit (*buf) )
			phase->load_mob = strtol(buf, NULL, 10);
	}

	else if ( !str_cmp (buf, "tool:") )
		(void)read_item_list (&phase->tool, argument, phase);

	else if ( (isdigit (*buf) && buf [1] == ':') ||
		 (isdigit (*buf) && isdigit (buf [1]) && buf [2] == ':') ) {

		items_num = strtol(buf, NULL, 10);
		if ( subcraft->items [items_num] ) {
			abort();
		}

		(void)read_item_list (&subcraft->items [strtol(buf, NULL, 10)], argument, phase);
	}

	else if ( !str_cmp (buf, "cost:") ) {
		argument = one_argument (argument, buf);

		while ( *buf ) {

			if ( !str_cmp (buf, "hits") ) {
				argument = one_argument (argument, buf);
				if ( !just_a_number (buf) ) {
					break;
				}

				phase->hit_cost = strtol(buf, NULL, 10);
			}

			else if ( !str_cmp (buf, "moves") ) {
				argument = one_argument (argument, buf);
				if ( !just_a_number (buf) ) {
					break;
				}

				phase->move_cost = strtol(buf, NULL, 10);
			}

			argument = one_argument (argument, buf);
		}
	}

	else if ( !str_cmp (buf, "open:") ) {

		argument = one_argument (argument, buf);

		if ( !str_cmp (buf, "target") ) {
			REMOVE_BIT (phase->flags, PHASE_OPEN_ON_SELF);
			argument = one_argument (argument, buf);
		} else
			SET_BIT (phase->flags, PHASE_OPEN_ON_SELF);

		if ( (sn = skill_index_lookup (buf)) == -1 ) {
			;
		} else
			phase->open_skill = sn;
	}

	else if ( !str_cmp (buf, "req:") || !str_cmp (buf, "require:") ) {

		
		argument = one_argument (argument, buf);

		if ( !str_cmp (buf, "target") ) {
			REMOVE_BIT (phase->flags, PHASE_REQUIRE_ON_SELF);
			argument = one_argument (argument, buf);
		} else
			SET_BIT (phase->flags, PHASE_REQUIRE_ON_SELF);

		if ( (sn = skill_index_lookup (buf)) == -1 ) {
			;
		} else
			phase->req_skill = sn;

		argument = one_argument (argument, buf);

		if ( *buf == '>' )
			SET_BIT (phase->flags, PHASE_REQUIRE_GREATER);
		else if ( *buf == '<' )
			REMOVE_BIT (phase->flags, PHASE_REQUIRE_GREATER);
		else {
			;
		}

		argument = one_argument (argument, buf);

		if ( !just_a_number (buf) ) {
			;
		}

		phase->req_skill_value = strtol(buf, NULL, 10);
	}

	else {
		system_log ("Did not know what to do with craft line:", TRUE);
		system_log (line, TRUE);
	}
}

void reg_read_crafts (FILE *fp_reg, char *buf)
{
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	char			*argument;

	while ( !feof (fp_reg) ) {

		(void)fgets (buf, MAX_STRING_LENGTH, fp_reg);

		if ( *buf && buf [strlen (buf) - 1] == '\n' )
			buf [strlen (buf) - 1] = '\0';

		if ( !*buf || *buf == '#' )
			continue;
	
		argument = one_argument (buf, buf2);

		if ( !str_cmp (buf2, "[end-crafts]") ) {
			return;
		}

		subcraft_line (fp_reg, buf);
	}
}

void boot_crafts ()
{
	FILE	*fp_crafts;
	char	buf [MAX_STRING_LENGTH] = {'\0'};

	if ( port == BUILDER_PORT || port == TEST_PORT )
		system ("./ordercrafts.pl ../regions/crafts");

	if ( !(fp_crafts = fopen (CRAFTS_FILE, "r")) ) {
		system_log ("The crafts file could not be read!", TRUE);
		return;
	}

	while ( !feof (fp_crafts) ) {

		(void)fgets (buf, MAX_STRING_LENGTH, fp_crafts);

		if ( *buf && buf [strlen (buf) - 1] == '\n' )
			buf [strlen (buf) - 1] = '\0';

		if ( !*buf || *buf == '#' )
			continue;

		if ( !str_cmp (buf, "[END]") )
			break;

		if ( !str_cmp (buf, "[CRAFTS]") )
			reg_read_crafts (fp_crafts, buf);
	}

	fclose (fp_crafts);
}

void craft_prepare_message (CHAR_DATA *ch, char *message, CHAR_DATA *n,
		CHAR_DATA *N, CHAR_DATA *T, char *phase_msg,
		OBJ_DATA *tool, OBJ_DATA **obj_list)
{
	int			ovnum = 0;
	char		*p = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	for ( p = buf; *phase_msg; ) {
		if ( *phase_msg == '$' && isdigit (phase_msg [1]) ) {
			phase_msg++;
			if ( isdigit (*phase_msg) &&
				 (ovnum = strtol(phase_msg, NULL, 10)) < MAX_ITEMS_PER_SUBCRAFT ) {
				 temp_arg = see_object (ch, obj_list [ovnum]);
				snprintf (p, MAX_STRING_LENGTH,  "#2%s#0", temp_arg);
				p = p + strlen (p);
			}

			while ( isdigit (*phase_msg) )
				phase_msg++;
		}

		else if ( *phase_msg == '$' ) {

			switch ( phase_msg [1] ) {
			   case 'e':
			   		snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", n ? HSSH (n) : "HSSH-e"); 
			   		break;
			   
			   case 'm':
			   		snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", n ? HMHR (n) : "HMHR-m"); 
			   		break;
			   
			   case 's':
			   		snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", n ? HSHR (n) : "HSHR-s"); 
			   		break;
			   
			   case 'E':
			   		snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", N ? HSSH (N) : "HSSH-E"); 
			   		break;
			   
			   case 'M':
			   		snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", N ? HMHR (N) : "HMHR-M"); 
			   		break;
			   
			   case 'S':
			   		snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", N ? HSHR (n) : "HSHR-S"); 
			   		break;
			   
			   case 't':
			   		temp_arg = see_object (ch, tool);
					snprintf (p, MAX_STRING_LENGTH,  "#2%s#0", temp_arg);
					p = p + strlen (p);
					break;
			   
			   case 'T':
			   		temp_arg = see_person(ch, T);
					snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", T ? temp_arg : "SOMEBODY-T");
					break;
			   
			   case 'n':
			   		temp_arg = see_person (ch, n);
					snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", n ? temp_arg : "SOMEBODY-n");
					break;
			   
			   case 'N':
			   		temp_arg = see_person (ch, N);
					snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", N ? temp_arg : "SOMEBODY-N");
					break;
			}

			phase_msg += 2;
			p = p + strlen (p);
		}

		else {
			*(p++) = *(phase_msg++);
			*p = '\0';
		}
	}

	strcpy (message, buf);
}

OBJ_DATA *obj_list_vnum (CHAR_DATA *ch, OBJ_DATA *list, int vnum)
{
        for ( ; list; list = list->next_content )
                if ( list->virtual == vnum && CAN_SEE_OBJ (ch, list) )
                        return list;
                
        return NULL;
}
 
OBJ_DATA *obj_list_vnum_dark (OBJ_DATA *list, int vnum)
{
        for ( ; list; list = list->next_content )
                if ( list->virtual == vnum )
                        return list;
 
        return NULL;
}

OBJ_DATA *get_item_obj (CHAR_DATA *ch, DEFAULT_ITEM_DATA *item,
						PHASE_DATA *phase)
{
	int				j;
	int				vnum;
	PHASE_DATA		*tmp_phase;
	OBJ_DATA		*tobj;

	for ( j = 0; j < MAX_DEFAULT_ITEMS; j++ ) {

		if ( !(vnum = item->items [j]) )
			continue;

		if ( IS_SET (item->flags, SUBCRAFT_PRODUCED) ) {

			if ( item->phase == phase )
				break;

			for ( tmp_phase = phase->next;
				  tmp_phase;
				  tmp_phase = tmp_phase->next )
				if ( tmp_phase == item->phase )
					return vtoo (vnum);
		}

		if ( IS_SET (item->flags, SUBCRAFT_USED) ) {
			if ( (tobj = obj_list_vnum (ch, ch->right_hand, vnum)) ||
				(tobj = obj_list_vnum (ch, ch->left_hand, vnum)) ) {
				if ( item->item_counts > tobj->count )
					continue;
				else
					return tobj;
			}
			if ( (tobj = obj_list_vnum (ch, ch->room->contents, vnum)) ) {
				if ( item->item_counts > tobj->count )
					continue;
				else
					return tobj;
			}
		}

		if ( IS_SET (item->flags, SUBCRAFT_IN_ROOM)) {
			if ( (tobj = obj_list_vnum (ch, ch->room->contents, vnum)) ) {
				if ( item->item_counts > tobj->count )
					continue;
				else
					return tobj;
			}
		}

		if ( IS_SET (item->flags, SUBCRAFT_IN_INVENTORY) || IS_SET (item->flags, SUBCRAFT_HELD) ) {
			if ( (tobj = obj_list_vnum_dark (ch->right_hand, vnum)) ||
				(tobj = obj_list_vnum_dark (ch->left_hand, vnum))  ) {
				if ( item->item_counts > tobj->count )
					continue;
				else
					return tobj;
			}
		}

		if ( IS_SET (item->flags, SUBCRAFT_WIELDED) ) {
			if ( ((tobj = get_equip (ch, WEAR_BOTH)) &&
				  tobj->virtual == vnum) ||
				 ((tobj = get_equip (ch, WEAR_PRIM)) &&
				  tobj->virtual == vnum) ||
				 ((tobj = get_equip (ch, WEAR_SEC)) &&
				  tobj->virtual == vnum) ) {
				if ( item->item_counts > tobj->count )
					continue;
				else
					return tobj;
			}
		}

		if ( IS_SET (item->flags, SUBCRAFT_WORN) ) {
			for ( tobj = ch->equip; tobj; tobj = tobj->next_content ) {
				if ( tobj->virtual != vnum )
					continue;

				if ( item->item_counts > tobj->count )
					continue;

				if ( tobj->location != WEAR_BOTH &&
					 tobj->location != WEAR_PRIM &&
					 tobj->location != WEAR_SEC )
					return tobj;
			}
		}

		if ( IS_SET (item->flags, SUBCRAFT_WORN) ||
			 IS_SET (item->flags, SUBCRAFT_WIELDED) ||
			 IS_SET (item->flags, SUBCRAFT_HELD) ||
			 IS_SET (item->flags, SUBCRAFT_IN_INVENTORY) ||
			 IS_SET (item->flags, SUBCRAFT_IN_ROOM) ||
			 IS_SET (item->flags, SUBCRAFT_USED) )
			continue;

			/* Grab this item wherever it is */

		if ( ((tobj = obj_list_vnum_dark (ch->right_hand, vnum)) &&
			  tobj->virtual == vnum) ||
			((tobj = obj_list_vnum (ch, ch->left_hand, vnum)) &&
			tobj->virtual == vnum) || 
			 ((tobj = obj_list_vnum (ch, ch->room->contents, vnum)) &&
			  tobj->virtual == vnum) ||
			 ((tobj = get_equip (ch, WEAR_BOTH)) &&
			  tobj->virtual == vnum) ||
			 ((tobj = get_equip (ch, WEAR_PRIM)) &&
			  tobj->virtual == vnum) ||
			 ((tobj = get_equip (ch, WEAR_SEC)) &&
			  tobj->virtual == vnum) ) {
			if ( item->item_counts > tobj->count )
				continue;
			else
				return tobj;
		}
	}

	return NULL;
}

void missing_item_msg (CHAR_DATA *ch, DEFAULT_ITEM_DATA *item, char *header)
{
	int			i;
	int			choice_count = 0;
	OBJ_DATA	*proto_obj;
	char		*p;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( item->item_counts > 1 )
		snprintf (buf, MAX_STRING_LENGTH,  "%s%d of ", header, item->item_counts);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "%s", header);

	for ( i = 0; i < MAX_DEFAULT_ITEMS; i++ )
		if ( item->items [i] )
			choice_count++;

	for ( i = 0; i < MAX_DEFAULT_ITEMS; i++ ) {

		if ( !item->items [i] )
			continue;

		choice_count--;

		proto_obj = vtoo (item->items [i]);

		if ( !proto_obj )
			continue;

		snprintf (ADDBUF, MAX_STRING_LENGTH, "#2%s#0", proto_obj->short_description);
		if ( choice_count )
			strcat (buf, ", ");

		if ( choice_count == 1 )
			strcat (buf, "or ");

		if ( !choice_count )
			strcat (buf, ".");
	}

	reformat_string (buf, &p);
	send_to_char (p, ch);

	mem_free (p);
}

int craft_uses (SUBCRAFT_HEAD_DATA *craft, int vnum)
{
	int			i = 0, j = 0;

	for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {
		if ( !craft->items[i] )
			continue;
		if ( !IS_SET (craft->items[i]->flags, SUBCRAFT_USED) && !IS_SET (craft->items[i]->flags, SUBCRAFT_HELD) &&
			!IS_SET (craft->items[i]->flags, SUBCRAFT_IN_ROOM) && !IS_SET (craft->items[i]->flags, SUBCRAFT_IN_INVENTORY) )
			continue;
		for ( j = 0; j < MAX_DEFAULT_ITEMS; j++ ) {
			if ( craft->items[i]->items[j] == vnum )
				return 1;
		}
	}

	return 0;
}

int craft_produces (SUBCRAFT_HEAD_DATA *craft, int vnum)
{
	int			i = 0, j = 0;

	for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {
		if ( !craft->items[i] )
			continue;
		if ( !IS_SET (craft->items[i]->flags, SUBCRAFT_PRODUCED) )
			continue;
		for ( j = 0; j < MAX_DEFAULT_ITEMS; j++ ) {
			if ( craft->items[i]->items[j] == vnum )
				return 1;
		}
	}

	return 0;
}

SUBCRAFT_HEAD_DATA *branch_craft_backward (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft, int level)
{
	SUBCRAFT_HEAD_DATA	*tcraft = NULL;
	SUBCRAFT_HEAD_DATA	*dcraft = NULL;
	SUBCRAFT_HEAD_DATA	*next_craft = NULL;
	int			i = 0;

	if ( level >= 10 )
		return NULL;

	for ( tcraft = crafts; tcraft; tcraft = next_craft ) {
		next_craft = tcraft->next;
		
		if ( str_cmp (tcraft->craft_name, craft->craft_name) )
			continue;
		
		if ( !str_cmp (tcraft->subcraft_name, craft->subcraft_name) )
			continue;
		
		for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {
			if ( !craft->items[i] )
				continue;
			
			if ( IS_SET (craft->items[i]->flags, SUBCRAFT_PRODUCED) ||
				 IS_SET (craft->items[i]->flags, SUBCRAFT_IN_ROOM) )
				continue;
			
			if ( !tcraft )
				continue;
			
			if ( craft_produces (tcraft, craft->items[i]->items[0]) ) {
				if ( !has_craft (ch, tcraft) && meets_skill_requirements (ch, tcraft) )
					return tcraft;
				
				if ( !meets_skill_requirements (ch, tcraft) )		/* Dead-end; need to improve skills first. */
					continue;
				
				if ( (dcraft = branch_craft_backward (ch, tcraft, ++level)) )	/* Branching craft beneath tcraft. */
					return dcraft;
			}
		}
	}

	return NULL;

}

SUBCRAFT_HEAD_DATA *branch_craft_forward (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft, int level)
{
	SUBCRAFT_HEAD_DATA	*tcraft = NULL;
	SUBCRAFT_HEAD_DATA	*dcraft = NULL;
	SUBCRAFT_HEAD_DATA	*next_craft = NULL;
	int			i = 0;

	if ( level >= 10 )
		return NULL;

	for ( tcraft = crafts; tcraft; tcraft = next_craft ) {
		next_craft = tcraft->next;
		if ( str_cmp (tcraft->craft_name, craft->craft_name) )
			continue;
		if ( !str_cmp (tcraft->subcraft_name, craft->subcraft_name) )
			continue;
		for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {
			if ( !craft->items[i] )
				continue;
			if ( !IS_SET (craft->items[i]->flags, SUBCRAFT_PRODUCED) )
				continue;
			if ( !craft_uses (tcraft, craft->items[i]->items[0]) )
				continue;
			if ( (dcraft = branch_craft_backward (ch, tcraft, ++level)) )
				return dcraft;			
			if ( has_craft (ch, tcraft) || !meets_skill_requirements (ch, tcraft) )
				continue;
			return tcraft;
		}
	}

	return NULL;

}

SUBCRAFT_HEAD_DATA *get_related_subcraft (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
	SUBCRAFT_HEAD_DATA	*tcraft= NULL;
	AFFECTED_TYPE		*af = NULL;
	int			i = 0;

	tcraft = branch_craft_backward (ch, craft, 0);

	if ( !tcraft )
		tcraft = branch_craft_forward (ch, craft, 0);

	if ( !tcraft ) {
		for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ ) {
			if ( (af = get_affect (ch, i)) ) {
				if ( str_cmp (af->a.craft->subcraft->craft_name, craft->craft_name) )
					continue;
				tcraft = branch_craft_backward (ch, af->a.craft->subcraft, 0);
				if ( !tcraft )
					tcraft = branch_craft_forward (ch, af->a.craft->subcraft, 0);
				if ( tcraft && tcraft != craft )
					return tcraft;
			}
		}
	}

	if ( tcraft && tcraft != craft )
		return tcraft;

	return NULL;
}

int has_craft (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
	AFFECTED_TYPE		*af;

	for ( af = ch->hour_affects; af; af = af->next ) {
		if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST ) {
			if ( !str_cmp (af->a.craft->subcraft->subcraft_name, craft->subcraft_name) )
				return 1;
		}
	}

	return 0;
}

int meets_skill_requirements (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
	PHASE_DATA		*phase;
	char			clan_name [MAX_STRING_LENGTH] = {'\0'};
	char			*c1;
	int			i, flags = 0, flags2 = 0;
	bool			required_race = FALSE, required_clan = FALSE;

	for ( i = 0; i <= 24; i++ ) {
		if ( craft->race[i] )
			required_race = TRUE;
	}

	if ( required_race ) {
		for ( i = 0; i <= 24; i++ ) {
			if ( craft->race[i] && ch->race == craft->race[i] - 1 )
				required_race = FALSE;
		}
	}

	if ( required_race )
		return 0;

	if ( craft->clans && strlen(craft->clans) > 3 ) {
		required_clan = TRUE;
	        for ( c1 = craft->clans; get_next_clan (&c1, clan_name, &flags);) {
                	if ( get_clan (ch, clan_name, &flags2) && flags2 >= flags )
                        	required_clan = FALSE;
        	}
	}

	if ( required_clan )
		return 0;

	for ( phase = craft->phases; phase; phase = phase->next ) {
                if ( phase->req_skill ) {
                        if ( !real_skill (ch, phase->req_skill) )
                                return 0;
                }
		if ( phase->skill ) {
			if ( !real_skill (ch, phase->skill) || ch->skills[phase->skill] < ((int)((phase->dice*phase->sides) * 0.750)) )
				return 0;
		}
	}

	return 1;
}

void branch_craft (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
	AFFECTED_TYPE		*af = NULL;
	SUBCRAFT_HEAD_DATA	*tcraft = NULL;
	int			num = 0;
	int			i = 0;
	int			stop = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	bool		related = FALSE;

	if ( get_affect (ch, MAGIC_CRAFT_BRANCH_STOP) && port != TEST_PORT )
		return;

	if ( !str_cmp (craft->craft_name, "general") )
		return;

	if ( !(tcraft = get_related_subcraft (ch, craft)) ) {
		for ( tcraft = crafts; tcraft; tcraft = tcraft->next ) {
			if ( tcraft == craft )
				continue;
			if ( !str_cmp (craft->craft_name, tcraft->craft_name) && meets_skill_requirements (ch, tcraft) && !has_craft (ch, tcraft) )
				num++;
		}

		if ( num <= 0 )
			return;

		stop = number(1,num);

		for ( tcraft = crafts, i = 0; tcraft; tcraft = tcraft->next ) {
			if ( tcraft == craft )
				continue;
			if ( !str_cmp (craft->craft_name, tcraft->craft_name) && meets_skill_requirements (ch, tcraft) && !has_craft (ch, tcraft) )
				i++;
			if ( i == stop )
				break;
		}
	}
	else related = TRUE;

	if ( !tcraft || tcraft == craft || has_craft (ch, tcraft) )
		return;

	if ( related )
		snprintf (buf, MAX_STRING_LENGTH,  "You have branched a related subcraft, #6%s %s#0!\n", tcraft->command,tcraft->subcraft_name);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "You have branched a new subcraft, #6%s %s#0!\n", tcraft->command,tcraft->subcraft_name);

	send_to_char (buf, ch);

	for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ ) {
		if ( !get_affect (ch, i) )
			break;
	}

	magic_add_affect (ch, i, -1, 0, 0, 0, 0);
	af = get_affect (ch, i);
	af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);
	af->a.craft->subcraft = tcraft;

        magic_add_affect (ch, MAGIC_CRAFT_BRANCH_STOP, number(30,90), 0, 0, 0, 0);
}

int requires_skill_check (SUBCRAFT_HEAD_DATA *craft)
{
	PHASE_DATA			*phase;

	for ( phase = craft->phases; phase; phase = phase->next ) {
		if ( phase->req_skill || phase->skill )
			return 1;
	}

	return 0;
}

int branching_check (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
	PHASE_DATA		*phase = NULL;
	SUBCRAFT_HEAD_DATA	*tcraft = NULL;
	AFFECTED_TYPE		*af = NULL;
	int			max = 0;
	int			max_amt = 0;
	int			multiplier = 0;
	int			total_crafts = 0;
	int			char_crafts = 0;
	int			i = 0;
	float		percentage = 0;

	/* does the craft check a skill ? */
	for ( phase = craft->phases; phase; phase = phase->next ) {
		if ( phase->skill ) {
			if ( phase->dice*phase->sides > max_amt ) {
				max = phase->skill;
				max_amt = phase->dice*phase->sides;
			}
		}
		if ( phase->req_skill ) {
			if ( phase->req_skill_value > max_amt ) {
				max = phase->req_skill;
				max_amt = phase->req_skill_value;
			}
		}
	}

	/* adds up total number of eligible subcrafts (based on skill) */
	for ( tcraft = crafts; tcraft; tcraft = tcraft->next ) {
		if ( !str_cmp (tcraft->craft_name, craft->craft_name) && meets_skill_requirements (ch, tcraft) )
			total_crafts++;
	}

	/* adds up total number of subcrafts character has */
    for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ ) {
    	if ( !(af = get_affect (ch, i)) )
			continue;
		if ( !str_cmp (af->a.craft->subcraft->craft_name, craft->craft_name) )
			char_crafts++;
	}

	if ( total_crafts <= char_crafts )
		total_crafts = char_crafts + 1;

	percentage = (float)char_crafts / (float)total_crafts;

	if ( percentage >= 0 && percentage <= .33 )
		multiplier = 4;
	else if ( percentage > .33 && percentage <= .60 )
		multiplier = 3;
	else if ( percentage > .60 && percentage <= .75 )
		multiplier = 2;
	else multiplier = 1;

	return MIN ((calc_lookup (ch, REG_LV, max) * multiplier), 65);
}

int figure_craft_delay (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
	PHASE_DATA	*phase;
	int		seconds = 0, skill = 0, numskills = 0;

	seconds = craft->delay * 60 * 60;

	for ( phase = craft->phases; phase; phase = phase->next ) {
		if ( phase->req_skill ) {
			numskills++;
			skill += ch->skills [phase->req_skill];
		}
		if ( phase->skill ) {
			numskills++;
			skill += ch->skills [phase->skill];
		}
	}

	if ( !numskills )
		numskills = 1;

	skill /= numskills;

	if ( skill >= 30 && skill < 50 )
		(int) (seconds *= (.85));
	else if ( skill >= 50 && skill < 70 )
		(int) (seconds *= (.72));
	else if ( skill >= 70 )
		(int) (seconds *= (.61));

	return seconds;
}

void activate_phase (CHAR_DATA *ch, AFFECTED_TYPE *af)
{
	    char numbuf[20];
	    int count;
	    OBJ_DATA *failobj;
	int					i, j;
	int					delay_time = 0;
	int					missing_said = 0;
	int					phase_failed = 0;
	int					skill_value = 0;
	int					dice_value = 0;
	int 				attr_value = 0;
	char				color [MAX_STRING_LENGTH] = {'\0'};
	char				*p;
	char				*first;
	PHASE_DATA			*phase = NULL;
	DEFAULT_ITEM_DATA	*item;
	CHAR_DATA			*tch, *mob;
	CHAR_DATA			*target_ch;
	OBJ_DATA			*tool = NULL;
	OBJ_DATA			*target_obj;
	OBJ_DATA			*obj_list [MAX_ITEMS_PER_SUBCRAFT];
	SUBCRAFT_HEAD_DATA	*subcraft;
	int					item_required [MAX_ITEMS_PER_SUBCRAFT];
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				test_buf [MAX_STRING_LENGTH] = {'\0'};
	
		/* A craft is active as long as af->a.craft->timer is non-zero. */

	if ( af->a.craft->phase )
		phase = af->a.craft->phase;

		/* Shorthand variables */

	subcraft = af->a.craft->subcraft;
	target_ch = af->a.craft->target_ch;
	target_obj = af->a.craft->target_obj;

		/* Point affect at next phase, and setup timer for that phase */

	if ( !phase ) {
		phase = af->a.craft->subcraft->phases;
		af->a.craft->phase = phase->next;
		af->a.craft->timer = phase->phase_seconds;
	} else {
		af->a.craft->phase = phase->next;

		if ( phase->next )
			af->a.craft->timer = phase->phase_seconds;
		else
			af->a.craft->timer = 0;
	}

	if ( !IS_MORTAL(ch) )
		af->a.craft->timer = 0;

	memset (item_required, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (int));
	memset (obj_list, 0, MAX_ITEMS_PER_SUBCRAFT * sizeof (OBJ_DATA *));

	for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {

		item = af->a.craft->subcraft->items [i];

		if ( !item ) {
			continue;
		}

		if ( !item->items [0] )		/* No items in list.  Nothing required */
			continue;

		obj_list [i] = get_item_obj (ch, item, phase);
	}

		/* Determine what is required for this phase based on first person */

	if ( phase->first ) {
		for ( p = phase->first; *p; p++ ) {
			if ( *p == '$' ) {
				p++;
				if ( isdigit (*p) && strtol(p, NULL, 10) < MAX_ITEMS_PER_SUBCRAFT )
					item_required [strtol(p, NULL, 10)] = 1;
			}
		}
	}

	if ( phase->tool && phase->tool->items [0] ) {
		if ( !(tool = get_item_obj (ch, phase->tool, phase)) ) {
			send_to_char ("You are missing a tool.\n", ch);
			af->a.craft->timer = 0;
			return;
		}
	}

		/* Make sure all required objects are accounted for */

	for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {

		if ( !item_required [i] )
			continue;

		if ( obj_list [i] )
			continue;

		item = af->a.craft->subcraft->items [i];

		if ( !item ) {
			continue;
		}

		if ( item && IS_SET (item->flags, SUBCRAFT_PRODUCED) &&
			 item->phase == phase )
			continue;

		if ( !missing_said ) {
			missing_said = 1;
			send_to_char ("\n#1You are missing one or more items:#0\n\n", ch);
			act ("$n stops doing $s craft.", FALSE, ch, 0, 0, TO_ROOM);
			af->a.craft->timer = 0;
		}

		if ( IS_SET (item->flags, SUBCRAFT_WORN) )
			missing_item_msg (ch, item, "You must wear ");
		else if ( IS_SET (item->flags, SUBCRAFT_HELD) )
			missing_item_msg (ch, item, "You must hold ");
		else if ( IS_SET (item->flags, SUBCRAFT_IN_ROOM) )
			missing_item_msg (ch, item, "In the room must be ");
		else if ( IS_SET (item->flags, SUBCRAFT_IN_INVENTORY) )
			missing_item_msg (ch, item, "You must have in your inventory ");
		else if ( IS_SET (item->flags, SUBCRAFT_WIELDED) )
			missing_item_msg (ch, item, "You must wield ");
		else
			missing_item_msg (ch, item, "You need ");
	}

	if ( missing_said )
		return;

		/* Check for skill requirements */

	if ( phase->req_skill ) {

		if ( IS_SET (phase->flags, PHASE_REQUIRE_ON_SELF) )
			skill_value = ch->skills [phase->req_skill];
		else
			skill_value = target_ch->skills [phase->req_skill];

		if ( IS_SET (phase->flags, PHASE_REQUIRE_GREATER) &&
			 skill_value <= phase->req_skill_value )
			phase_failed = 1;
		else if ( !IS_SET (phase->flags, PHASE_REQUIRE_GREATER) &&
				  skill_value >= phase->req_skill_value )
			phase_failed = 1;

		skill_use (ch, phase->req_skill, 0);
	}

	if ( phase->skill ) {
		skill_use (ch, phase->skill, 0);
		dice_value = dice(phase->dice, phase->sides);
		if ( dice_value > ch->skills [phase->skill] )
			phase_failed = 1;
	}
	
	
		/* Removed USED items from wherever they are */

	for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {

		item = af->a.craft->subcraft->items [i];

		if ( !item ) {
			continue;
		}

		if ( /*!item_required [i] ||*/
			item->phase != phase ||
			 !IS_SET (item->flags, SUBCRAFT_USED) )
			continue;

		if ( !obj_list [i] ) {
			act ("You are not using enough of a particular item to finish this craft.", FALSE, ch, 0, 0, TO_CHAR);

			af->a.craft->timer = 0;

			return;
		}

		if ( obj_list[i] && obj_list[i]->var_color && str_cmp (obj_list[i]->var_color, "none") )
			item->color = obj_list[i]->var_color;

		if ( obj_list[i]->count <= item->item_counts )
			extract_obj (obj_list[i]);
		else obj_list[i]->count -= item->item_counts;
	}

		/* Create objects made by this phase */

	for ( i = 0; i < MAX_ITEMS_PER_SUBCRAFT; i++ ) {

		if ( phase_failed )
			continue;

		item = af->a.craft->subcraft->items [i];

		if ( !item ) {
			continue;
		}

		if ( !item->items [0] )
			continue;

		if ( !IS_SET (item->flags, SUBCRAFT_PRODUCED) ||
			 item->phase != phase )
			continue;

			/* We need to account for the quantity of an item */

		*color = '\0';

		for ( j = 0; j < MAX_ITEMS_PER_SUBCRAFT; j++ ) {
			item = af->a.craft->subcraft->items [j];
			if ( item && item->color ) {
				snprintf (color, MAX_STRING_LENGTH, "%s", item->color);
				break;
			}
		}

		item = af->a.craft->subcraft->items [i];

		if ( !item ) {
			continue;
		}

		if ( item->items[0] && vtoo(item->items[0]) ) {
			if ( color && IS_SET (vtoo(item->items[0])->obj_flags.extra_flags, ITEM_VARIABLE) ) {
				ch->delay_who = add_hash (color);
				obj_list [i] = load_colored_object (item->items [0], ch->delay_who);
				ch->delay_who = 0;
			}
			else obj_list [i] = load_object (item->items [0]);
		}

		if ( !obj_list [i] ) {
			act ("An object produced by this craft was not found. Please inform the staff.", FALSE, ch, 0, 0, TO_CHAR);

			af->a.craft->timer = 0;

			return;
		}

		obj_list [i]->count = item->item_counts;

		obj_to_room (obj_list [i], ch->in_room);
	}

	if ( !phase_failed && phase->load_mob && vtom (phase->load_mob) ) {
		char_to_room ((mob = load_mobile (phase->load_mob)), ch->in_room);
		if ( get_affect (mob, MAGIC_HIDDEN) )
			remove_affect_type (mob, MAGIC_HIDDEN);
	}

		/* Check to see if attribute check succeeded.  This really is
		   against the theme of this mud since it is skill based, not
           attribute based, but it was asked for anyway. */

	if ( phase->attribute > -1 ) {

		

		switch ( phase->attribute ) {
			case 0:  attr_value = GET_STR (ch); break;
			case 1:  attr_value = GET_DEX (ch); break;
			case 2:  attr_value = GET_CON (ch); break;
			case 3:  attr_value = GET_WIL (ch); break;
			case 4:  attr_value = GET_INT (ch); break;
			case 5:  attr_value = GET_AUR (ch); break;
			case 6:  attr_value = GET_AGI (ch); break;
		}
		dice_value = dice (phase->dice, phase->sides);
snprintf (test_buf, MAX_STRING_LENGTH,  "%d value against %d", attr_value, dice_value);
send_to_char(test_buf, ch);
		if ( attr_value < dice_value ){
			
			phase_failed = 1;
		}
	}

	if ( phase_failed ) {

				/* 1st person failure message */

		if ( phase->failure ) {
			craft_prepare_message (ch, buf, ch, target_ch, NULL, phase->failure,
								   tool, obj_list);
			if ( *buf == '#' )
				buf[2] = toupper(buf[2]);
			else *buf = toupper(*buf);
			act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		} else if ( af->a.craft->subcraft->failure ) {
			craft_prepare_message (ch, buf, ch, target_ch, NULL,
								   af->a.craft->subcraft->failure, tool,
                                   obj_list);
			if ( *buf == '#' )
				buf[2] = toupper(buf[2]);
			else *buf = toupper(*buf);
			act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		} else
			act ("You fail at your work.", FALSE, ch, 0, 0, TO_CHAR);

				/* 2nd person failure message */

			/* Don't give second to self. */
			if ( !(target_ch == ch) && (phase->second_failure)) {
			craft_prepare_message (ch, buf, ch, target_ch, NULL,
								   phase->second_failure, tool, obj_list);
			if ( *buf == '#' )
				buf[2] = toupper(buf[2]);
			else *buf = toupper(*buf);
			act (buf, FALSE, target_ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		} 
		else {
			act ("$n abruptly stops $s craft.",
						FALSE, target_ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		}

				/* 3rd person failure message */

		if ( phase->third_failure ) {
			craft_prepare_message (ch, buf, ch, target_ch, NULL,
								   phase->third_failure, tool, obj_list);
			if ( *buf == '#' )
				buf[2] = toupper(buf[2]);
			else *buf = toupper(*buf);
			act (buf, FALSE, ch, 0, target_ch, TO_NOTVICT | TO_ACT_FORMAT);
		} else
			act ("$n abruptly stops $s craft.",
						FALSE, ch, 0, target_ch, TO_NOTVICT | TO_ACT_FORMAT);

		    count = 0;

		    p = subcraft->failobjs;

		    while(p != NULL && *p != '\0' && *p != ')') {
			/* punt leading whitespace */
			while(isspace(*p) && *p != '\0') p++;

			/* Make sure there is no whitespace inside */
			while(isspace(*p) && *p != '\0') p++;

			/* Now if we have an 'x', we have a count */ 
			if(*p == 'x') {
			    p++;
			    while(!isdigit(*p) && *p != '\0') p++;
			    for(i=0;isdigit(*p); i++,p++)
                                numbuf[i] = *p;
                            numbuf[i] = '\0';
			    count = strtol(numbuf, NULL, 10);
			}
			
		        while(!isdigit(*p) && *p != '\0') p++;
		        for(i=0;isdigit(*p); i++,p++)
		            numbuf[i] = *p;
		        numbuf[i] = '\0';
			if(i > 0) { /* Then we have some kind of # */
		            failobj = load_object(strtol(numbuf, NULL, 10));
			    if(failobj != NULL) {
				if(count > 0)
	                            failobj->count = count;
				obj_to_room(failobj, ch->in_room);
			    }
			}
		}		

		af->a.craft->timer = 0;

		return;
	}

		/* First person message */

	if ( phase->first || phase->self ) {

		first = phase->first;

			/* Use self text, if it exists,  if PC targets himself */

		if ( phase->self &&
			 target_ch == ch &&
			 IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR) )
			first = phase->self;

		craft_prepare_message (ch, buf, ch, target_ch, NULL,
							   first, tool, obj_list);

		if ( *buf == '#' )
			buf[2] = toupper(buf[2]);
		else *buf = toupper(*buf);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	}

		/* Second person message */

	if ( phase->second && target_ch && target_ch != ch ) {

		craft_prepare_message (target_ch, buf, ch,
							target_ch, NULL, phase->second, tool, obj_list);

		if ( *buf == '#' )
			buf[2] = toupper(buf[2]);
		else *buf = toupper(*buf);
		act (buf, FALSE, target_ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	}

		/* Third person message */

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {

		if ( !phase->third )
			break;

		if ( tch == ch || tch == target_ch )
			continue;

		craft_prepare_message (tch, buf, ch, target_ch, tch,
							   phase->third, tool, obj_list);
		if ( *buf == '#' )
			buf[2] = toupper(buf[2]);
		else *buf = toupper(*buf);
		act (buf, FALSE, tch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	}
	
		/* Cast any spells from this phase */

	if ( phase->spell_type ) {

			/* Handle casting and first and second person messages */

		if ( IS_SET (subcraft->subcraft_flags, SCF_TARGET_SELF) ||
			 IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR) ) {

			apply_affect (target_ch, phase->spell_type, phase->duration,
									   phase->power);
		}
	}

	if ( phase->open_skill ) {
		if ( IS_SET (phase->flags, PHASE_OPEN_ON_SELF) &&
			 !real_skill (ch, phase->open_skill) )
			open_skill (ch, phase->open_skill);

		else if ( !IS_SET (phase->flags, PHASE_OPEN_ON_SELF) &&
				  !real_skill (target_ch, phase->open_skill) )
			open_skill (target_ch, phase->open_skill);
	}

	if ( IS_SET (subcraft->subcraft_flags, SCF_TARGET_CHAR) &&
		 !is_he_here (ch, target_ch, 1) ) {
		send_to_char ("Your target is no longer here.\n", ch);
		af->a.craft->timer = 0;
		return;
	}

	if ( weaken (ch, 0, phase->move_cost, "cost of craft") )
		return;		/* He died */

		/* Immediately activate next phase after a phase with 0 timer */

	if ( !af->a.craft->timer && af->a.craft->phase )
		activate_phase (ch, af);
	else if ( !af->a.craft->phase && requires_skill_check (af->a.craft->subcraft) ) {
		if ( number(1,100) <= branching_check (ch, af->a.craft->subcraft) )
			branch_craft (ch, af->a.craft->subcraft);
	}

	if ( af->a.craft->subcraft->delay && !af->a.craft->phase ) {
		delay_time = (int)(time(0) + figure_craft_delay (ch, af->a.craft->subcraft));
		magic_add_affect (ch, MAGIC_CRAFT_DELAY, -1, delay_time, 0, 0, 0);
	}
}
