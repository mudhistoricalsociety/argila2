/*------------------------------------------------------------------------\
|  wounds.c : Wounds Module                           www.yourmud.org | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  All original code, derived under license from DIKU GAMMA (0.0).        |
\------------------------------------------------------------------------*/

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

#define WOUND_INFECTIONS 1	/*  Set to 1 to enable wound infection; 0 to disable. */

int general_damage (CHAR_DATA *ch, int amount)
{
	return wound_to_char (ch, "bloodloss", amount, 0, 0, 0, 0);
}

void apply_con_penalties (CHAR_DATA *ch)
{
	AFFECTED_TYPE	*af = NULL;

	if ( IS_NPC (ch) )
		return;

	if ( (af = get_affect (ch, AFFECT_LOST_CON)) ) {
		af->a.spell.sn += 2;
		af->a.spell.duration += 16;
	}
	else magic_add_affect (ch, AFFECT_LOST_CON, 16, 0, 0, 0, 2);

	ch->con -= 2;
	ch->tmp_con -= 2;
}

int wound_to_char (CHAR_DATA *ch, char *location, int impact, int type, int bleeding, int poison, int infection)
{
	WOUND_DATA 		*wound = NULL;
	char			*p = NULL;
	int				curdamage = 0;
	int				difficulty_rating = 0;
	float			limit = 0;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			name [MAX_STRING_LENGTH] = {'\0'};
	char			severity [MAX_STRING_LENGTH] = {'\0'};
	char			*temp_arg = NULL;

	if ( impact <= 0 )
		return 0;

	if ( ch->wounds )
		for (wound = ch->wounds; wound; wound = wound->next)
			curdamage += wound->damage;

	curdamage += ch->damage;

	limit = ch->max_hit;

	if ( impact > 0 && impact <= (limit*.02) ) {
		snprintf (severity, 6, "small");
		difficulty_rating = 10;
	}
	else if ( impact > (limit*.02) && impact <= (limit*.10) ) {
		snprintf (severity, 6, "minor");
		difficulty_rating = 12;
	}
	else if ( impact > (limit*.10) && impact <= (limit*.20) ) {
		snprintf (severity, 9, "moderate");
		difficulty_rating = 15;
	}
	else if ( impact > (limit*.20) && impact <= (limit*.30) ) {
		snprintf (severity, 7, "severe");
		difficulty_rating = 16;
	}
	else if ( impact > (limit*.30) && impact <= (limit*.40) ) {
		snprintf (severity, 9, "grievous");
		difficulty_rating = 17;
	}
	else if ( impact > (limit*.40) && impact <= (limit*.50)) {
		snprintf (severity, 9, "terrible");
		difficulty_rating = 18;
	}
	else if ( impact > (limit*.50) ) {
		snprintf (severity, 7, "mortal");
		difficulty_rating = 20;
	}

	if ( type == 2 || type == 4 )
		switch (number(1, 6)) {
			case 1:
				snprintf (name, 5, "slice");
				break;
			case 2:
				snprintf (name, 4, "cut");
				break;
			case 3:
				snprintf (name, 5, "gash");
				break;
			case 4:
				snprintf (name, 6, "slash");
				break;
			case 5:
				snprintf (name, 5, "nick");
				break;
			case 6:
				snprintf (name, 11, "laceration");
				break;
		}

	else if ( type == 0 || type == 1 )
		switch (number(1, 5)) {
			case 1:
				snprintf (name, 9, "puncture");
				break;
			case 2:
				snprintf (name, 9, "piercing");
				break;
			case 3:
				snprintf (name, 5, "stab");
				break;
			case 4:
				snprintf (name, 5, "hole");
				break;
			case 5:
				snprintf (name, 12, "perforation");
				break;
		}

	else if ( type == 3 )
		switch (number(1, 5)) {
			case 1:
				snprintf (name, 7, "bruise");
				break;
			case 2:
				snprintf (name, 7, "bruise");
				break;
			case 3:
				snprintf (name, 10, "contusion");
				break;
			case 4:
				snprintf (name, 10, "contusion");
				break;
			case 5:
				snprintf (name, 6, "crush");
				break;
		}

	else if ( type == 5 )
		switch (number(1,7)) {
			case 1:
				snprintf (name, 10, "frostburn");
				break;
			case 2:
				snprintf (name, 14, "discoloration");
				break;
			case 3:
				snprintf (name, 9, "frostnip");
				break;
			case 4:
				snprintf (name, 15, "waxy-frostbite");
				break;
			case 5:
				snprintf (name, 16, "white-frostbite");
				break;
			case 6:
				snprintf (name, 15, "gray-frostbite");
				break;
			case 7:
				snprintf (name, 16, "black-frostbite");
				break;
		}

	else if ( type == 6 )
		switch (number(1,7)) {
			case 1:
				snprintf (name, 5, "burn");
				break;
			case 2:
				snprintf (name, 5, "sear");
				break;
			case 3:
				snprintf (name, 6, "singe");
				break;
			case 4:
				snprintf (name, 5, "char");
				break;
			case 5:
				snprintf (name, 11, "blistering");
				break;
			case 6:
				snprintf (name, 7, "scorch");
				break;
			case 7:
				snprintf (name, 6, "scald");
				break;
		}

	else if ( type == 7 )	/*  Natural attacks -- teeth. */
		switch (number(1, 5)) {
			case 1:
				snprintf (name, 5, "bite");
				break;
			case 2:
				snprintf (name, 15, "tooth-puncture");
				break;
			case 3:
				snprintf (name, 9, "incision");
				break;
			case 4:
				snprintf (name, 6, "notch");
				break;
			case 5:
				snprintf (name, 5, "gore");
				break;
		}

	else if ( type == 8 )	/*  Natural attacks -- claws. */
		switch (number(1, 5)) {
			case 1:
				snprintf (name, 10, "claw-gash");
				break;
			case 2:
				snprintf (name, 5, "rent");
				break;
			case 3:
				snprintf (name, 5, "tear");
				break;
			case 4:
				snprintf (name, 4, "rip");
				break;
			case 5:
				snprintf (name, 6, "gouge");
				break;
		}

	else if ( type == 9 )	/*  Natural attacks -- fist. */
		switch (number(1,3)) {
			case 1:
				snprintf (name, 7, "bruise");
				break;
			case 2:
				snprintf (name, 9, "abrasion");
				break;
			case 3:
				snprintf (name, 10, "contusion");
				break;
		}

	if ( str_cmp (location, "bloodloss") ) {
		if (!ch->wounds) {
			CREATE (ch->wounds, WOUND_DATA, 1);
			wound = ch->wounds;
			wound->next = NULL;
		}

		else for ( wound = ch->wounds; wound; wound = wound->next ) {
			if ( !wound->next ) {
				CREATE (wound->next, WOUND_DATA, 1);
				wound = wound->next;
				wound->next = NULL;
				break;
			}
		}

		wound->location = str_dup(location);
		wound->damage = impact;
		if (type == 2 || type == 4)
			wound->type = str_dup ("slash");
		else if (type == 0 || type == 1)
			wound->type = str_dup ("pierce");
		else if (type == 3)
			wound->type = str_dup ("blunt");
		else if (type == 5)
			wound->type = str_dup ("frost");
		else if (type == 6)
			wound->type = str_dup ("fire");
		else if (type == 7)
			wound->type = str_dup ("bite");
		else if (type == 8)
			wound->type = str_dup ("claw");
		else if (type == 9)
			wound->type = str_dup ("fist");

		wound->name = str_dup (name);
		wound->severity = str_dup (severity);
		if ( !str_cmp (severity, "severe") && !bleeding )
			wound->bleeding = number(2,3);
		else if ( !str_cmp (severity, "grievous") && !bleeding )
			wound->bleeding = number(3,5);
		else if ( !str_cmp (severity, "terrible") && !bleeding )
			wound->bleeding = number(5,10);
		else if ( !str_cmp (severity, "mortal") && !bleeding )
			wound->bleeding = number(10,20);
		else if ( bleeding )
			wound->bleeding = bleeding;
		if ( IS_SET (ch->act, ACT_NOBLEED) )
			wound->bleeding = 0;
		wound->poison = poison;
		wound->infection = infection;
		wound->healerskill = 0;
		wound->lasthealed = time(0);
		wound->lastbled = time(0);
		wound->next = NULL;

		if ( IS_MORTAL(ch) && wound->bleeding ) {
			temp_arg = expand_wound_loc(wound->location);
			snprintf (buf, MAX_STRING_LENGTH,  "#1You grimace as you feel blood begin to flow from a %s %s on your %s.#0\n", wound->severity, wound->name, temp_arg);
			reformat_string (buf, &p);
			send_to_char ("\n", ch);
			send_to_char (p, ch);
			mem_free (p);
		}
	}
	else
		ch->damage += impact;

	curdamage += impact;

	if ( (curdamage > ch->max_hit) && (IS_MORTAL(ch) || IS_NPC(ch)) ) {
		if ( ch->room && !IS_SET (ch->flags, FLAG_COMPETE) )
			die (ch);
		return 1;
	}

	if ( (curdamage > ch->max_hit*.85) && GET_POS(ch) != POSITION_UNCONSCIOUS && (IS_MORTAL(ch) || IS_NPC(ch)) ) {
		GET_POS (ch) = POSITION_UNCONSCIOUS;
		ch->knockedout = time(0);
		if ( IS_SUBDUER (ch) )
			release_prisoner (ch, NULL);
		do_drop (ch, "all", 0);
		send_to_char ("\n#1Overcome at last, you slip into a deep unconsciousness. . .#0\n", ch);
		snprintf (buf, MAX_STRING_LENGTH,  "$n has been rendered unconscious.");
		act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		snprintf (buf, MAX_STRING_LENGTH,  "%s is mortally wounded and unconscious! [%d]\n", ch->tname, ch->in_room);
		clear_moves (ch);
		clear_current_move (ch);
		if ( !IS_NPC(ch) && !IS_SET (ch->flags, FLAG_GUEST) )
			send_to_gods(buf);
		apply_con_penalties (ch);
		if ( ch->con <= 3 ) {
			if ( ch->room && !IS_SET (ch->flags, FLAG_COMPETE) )
				die (ch);
			return 1;
		}
	}

	return 0;
}

void free_lodged (LODGED_OBJECT_INFO *lodged)
{
	if ( !lodged )
		return;

	if ( lodged->location && strlen(lodged->location) > 1 )
		mem_free (lodged->location);

	mem_free (lodged);
}

void free_wound (WOUND_DATA *wound)
{
	if ( !wound )
		return;

        if ( wound->location && *wound->location )
                mem_free (wound->location);
 
        if ( wound->type && *wound->type )
                mem_free (wound->type);
        
        if ( wound->name && *wound->name )
                mem_free (wound->name);
        
        if ( wound->severity && *wound->severity )
                mem_free (wound->severity);
                        
        mem_free (wound);
}

void heal_all_wounds (CHAR_DATA *ch)
{
	while ( ch->wounds )
		wound_from_char (ch, ch->wounds);

	while ( ch->lodged )
		lodge_from_char (ch, ch->lodged);

	ch->damage = 0;     

	GET_POS (ch) = POSITION_STANDING;

}

void lodge_from_char (CHAR_DATA *ch, LODGED_OBJECT_INFO *lodged)
{
	LODGED_OBJECT_INFO	*templodged;

	if ( !ch || !lodged )
		return;

	if ( ch->lodged == lodged )
		ch->lodged = ch->lodged->next;

	else {
		for (templodged = ch->lodged; templodged; templodged = templodged->next)
			if (templodged->next == lodged) templodged->next = templodged->next->next;
	}

	free_lodged (lodged);
}

void lodge_from_obj (OBJ_DATA *obj, LODGED_OBJECT_INFO *lodged)
{
	LODGED_OBJECT_INFO	*templodged;

	if ( !obj || !lodged )
		return;

	if ( obj->lodged == lodged )
		obj->lodged = obj->lodged->next;

	else {
		for (templodged = obj->lodged; templodged; templodged = templodged->next)
			if (templodged->next == lodged) templodged->next = templodged->next->next;
	}

	free_lodged (lodged);
}

void wound_from_char (CHAR_DATA *ch, WOUND_DATA *wound)
{
	WOUND_DATA	*tempwound;

	if ( !ch || !wound )
		return;

	if ( ch->wounds == wound )
		ch->wounds = ch->wounds->next;

	else {
		for (tempwound = ch->wounds; tempwound; tempwound = tempwound->next)
			if (tempwound->next == wound) tempwound->next = tempwound->next->next;
	}

	free_wound (wound);
}

void wound_from_obj (OBJ_DATA *obj, WOUND_DATA *wound)
{
	WOUND_DATA	*tempwound;

	if ( !obj || !wound )
		return;

	if ( obj->wounds == wound )
		obj->wounds = obj->wounds->next;

	else {
		for (tempwound = obj->wounds; tempwound; tempwound = tempwound->next)
			if (tempwound->next == wound) tempwound->next = tempwound->next->next;
	}

	free_wound (wound);
}

int is_proper_kit (OBJ_DATA *kit, WOUND_DATA *wound)
{
	if ( !kit->o.od.value[5] || IS_SET (kit->o.od.value[5], TREAT_ALL) )
		return 1;

	if ( !str_cmp (wound->name, "slice") ||
		!str_cmp (wound->name, "cut") ||
		!str_cmp (wound->name, "gash") ||
		!str_cmp (wound->name, "slash") ||
		!str_cmp (wound->name, "nick") ||
		!str_cmp (wound->name, "laceration") ) {
		if ( !IS_SET (kit->o.od.value[5], TREAT_SLASH) )
			return 0;
		else return 1;
	}

	if ( !str_cmp (wound->name, "claw-gash") ||
		!str_cmp (wound->name, "rent") ||
		!str_cmp (wound->name, "tear") ||
		!str_cmp (wound->name, "rip") ||
		!str_cmp (wound->name, "gouge") ) {
		if ( !IS_SET (kit->o.od.value[5], TREAT_SLASH) )
			return 0;
		else return 1;
	}

	if ( !str_cmp (wound->name, "puncture") ||
		!str_cmp (wound->name, "piercing") ||
		!str_cmp (wound->name, "stab") ||
		!str_cmp (wound->name, "hole") ||
		!str_cmp (wound->name, "perforation") ) {
		if ( !IS_SET (kit->o.od.value[5], TREAT_PUNCTURE) )
			return 0;
		else return 1;
	}

	if ( !str_cmp (wound->name, "bite") ||
		!str_cmp (wound->name, "tooth-puncture") ||
		!str_cmp (wound->name, "incision") ||
		!str_cmp (wound->name, "gore") ||
		!str_cmp (wound->name, "notch") ) {
		if ( !IS_SET (kit->o.od.value[5], TREAT_PUNCTURE) )
			return 0;
		else return 1;
	}

	if ( !str_cmp (wound->name, "bruise") ||
		!str_cmp (wound->name, "abrasion") ||
		!str_cmp (wound->name, "contusion") ||
		!str_cmp (wound->name, "crush") ) {
		if ( !IS_SET (kit->o.od.value[5], TREAT_BLUNT) )
			return 0;
		else return 1;
	}

	if ( !str_cmp (wound->name, "frostburn") ||
		!str_cmp (wound->name, "discoloration") ||
		!str_cmp (wound->name, "frostnip") ||
		!str_cmp (wound->name, "waxy-frostbite") ||
		!str_cmp (wound->name, "white-frostbite") ||
		!str_cmp (wound->name, "gray-frostbite") ||
		!str_cmp (wound->name, "black-frostbite") ) {
		if ( !IS_SET (kit->o.od.value[5], TREAT_FROST) )
			return 0;
		else return 1;
	}

	if ( !str_cmp (wound->name, "burn") ||
		!str_cmp (wound->name, "sear") ||
		!str_cmp (wound->name, "singe") ||
		!str_cmp (wound->name, "char") ||
		!str_cmp (wound->name, "blistering") ||
		!str_cmp (wound->name, "scorch") ||
		!str_cmp (wound->name, "scald") ) {
		if ( !IS_SET (kit->o.od.value[5], TREAT_FROST) )
			return 0;
		else return 1;
	}
	
	return 0;
}

void begin_treatment (CHAR_DATA *ch, CHAR_DATA *tch, char *location, int mode)
{
	WOUND_DATA 	*wound = NULL;
	OBJ_DATA 	*kit = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;

	if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_HEALER_KIT )
		kit = ch->right_hand;
	else if ( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_HEALER_KIT )
		kit = ch->left_hand;
	else {
		snprintf (buf, MAX_STRING_LENGTH,  "Having discarded your healer's kit, you cease tending $N.");
		snprintf (buf, MAX_STRING_LENGTH,  "Having discarded $s healer's kit, $n ceases $s ministrations.");
		act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
		act (buf2, FALSE, ch, 0, tch, TO_ROOM | TO_ACT_FORMAT);
		return;
	}

	if ( kit->o.od.value[0] <= 0 ) {
		send_to_char ("That healer's kit no longer contains any useful materials.\n", ch);
		return;
	}

	if ( kit->o.od.value[2] && kit->o.od.value[2] > ch->skills [SKILL_HEALING] ) {
		send_to_char ("You do not have the skill required to employ this remedy.\n", ch);
		return;
	}

	if ( tch->in_room != ch->in_room ) {
		send_to_char ("You cannot treat someone who isn't here!\n", ch);
		return;
	}

	for ( wound = tch->wounds; wound; wound = wound->next ) {
		if ( !str_cmp (wound->location, location) && ( wound->healerskill + 1 < ch->skills [SKILL_HEALING] && wound->healerskill != -1 ) ) {
			if ( !str_cmp (wound->severity, "small") || !str_cmp (wound->severity, "minor") ) {
				snprintf (buf, MAX_STRING_LENGTH,  "The %s %s is too minor to benefit from medical attention.", wound->severity, wound->name);
				act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
				continue;
			}
			if ( !is_proper_kit (kit, wound) ) {
				temp_arg = obj_short_desc(kit);
				snprintf (buf, MAX_STRING_LENGTH,  "#2%s#0 is of no assistance with the %s %s.", temp_arg, wound->severity, wound->name);
				buf[2] = toupper(buf[2]);
				act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
				continue;
			}
			if ( mode ) {
				temp_arg = expand_wound_loc(wound->location);
				snprintf (buf, MAX_STRING_LENGTH,  "You turn your attention to the %s %s on your %s.", wound->severity, wound->name, temp_arg);
				act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
				ch->delay_who = add_hash(location);
				ch->delay_ch	= tch;
				ch->delay_type = DEL_TREAT_WOUND;
				ch->delay = wound->damage - ch->skills[SKILL_HEALING]/10;
				ch->delay = MAX (ch->delay, 2);
				return;
			}
			else {
				temp_arg = expand_wound_loc(wound->location);
				snprintf (buf, MAX_STRING_LENGTH,  "You turn your attention to the %s %s on $N's %s.", wound->severity, wound->name, temp_arg);
				snprintf (buf2, MAX_STRING_LENGTH,  "$n turns $s attention to the %s %s on your %s.", wound->severity, wound->name, temp_arg);
				act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
				act (buf2, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
				ch->delay_who = add_hash (location);
				ch->delay_ch	= tch;
				ch->delay_type = DEL_TREAT_WOUND;
				ch->delay = wound->damage - ch->skills[SKILL_HEALING]/10;
				ch->delay = MAX (ch->delay, 2);
				return;
			}
		}
	}

	snprintf (buf, MAX_STRING_LENGTH,  "No other wounds on that area can benefit from your attention.");
	act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);

	return;
}

int adjust_wound (CHAR_DATA *ch, WOUND_DATA *wound, int amount)
{
	WOUND_DATA	*twound;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			curdamage = 0;
	char		*temp_arg = NULL;

	if ( !ch || !wound || !amount )
		return 0;

	wound->damage += amount;

	if ( wound->damage <= 0 ) {
		wound_from_char (ch, wound);
		return 0;
	}
	
	temp_arg = downsized_wound (ch, wound);
	snprintf (buf, MAX_STRING_LENGTH,  "%s", temp_arg);
	mem_free (wound->severity);
	wound->severity = NULL;
	wound->severity = str_dup (buf);
	
	if ( amount < 0 )
		return 0;

	for ( twound = ch->wounds; twound; twound = twound->next )
		curdamage += twound->damage;

	curdamage += ch->damage;

	if ( (curdamage > ch->max_hit) && (IS_MORTAL(ch) || IS_NPC(ch)) ) {
		send_to_char ("\n#1The last thing you perceive in this world is the cry of\n"
                              "agony that passes from your lips, borne upon the wings of\n"
                              "your last, labored breath -- may your soul find peace...#0\n\n", ch);
		if ( ch->room && !IS_SET (ch->flags, FLAG_COMPETE) )
			die (ch);
		return 1;
	}

	if ( (curdamage > ch->max_hit*.85) && GET_POS(ch) != POSITION_UNCONSCIOUS && (IS_MORTAL(ch) || IS_NPC(ch)) ) {
		GET_POS (ch) = POSITION_UNCONSCIOUS;
		ch->knockedout = time(0);
		if ( IS_SUBDUER (ch) )
			release_prisoner (ch, NULL);
		do_drop (ch, "all", 0);
		send_to_char ("\n#1Finally overcome with the tremendous pain of your wounds,\n"
			      "you collapse to the ground, unable to fight the inevitable\n"
                              "any longer. Your vision fades away into blackness...#0\n\n", ch);
		snprintf (buf, MAX_STRING_LENGTH,  "$n collapses, rendered unconscious by the pain.");
		act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		snprintf (buf, MAX_STRING_LENGTH,  "%s is mortally wounded! [%d]\n", ch->tname, ch->in_room);
		clear_moves (ch);
		clear_current_move (ch);
		if ( !IS_NPC(ch) && !IS_SET (ch->flags, FLAG_GUEST) )
			send_to_gods(buf);
	}

	return 0;
}

void delayed_treatment (CHAR_DATA *ch)
{
	CHAR_DATA	*tch;
	WOUND_DATA 	*wound, *next_wound;
	OBJ_DATA	*kit;
	int		mode = 0;
	char		*location;
	char		buf [MAX_STRING_LENGTH];
	char		buf2 [MAX_STRING_LENGTH];

	tch = ch->delay_ch;

	if ( tch == ch )
		mode = 1;

	location = ch->delay_who;

	if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_HEALER_KIT )
		kit = ch->right_hand;
	else if ( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_HEALER_KIT )
		kit = ch->left_hand;
	else {
		snprintf (buf, MAX_STRING_LENGTH,  "Having discarded your healer's kit, you cease tending $N.");
		snprintf (buf, MAX_STRING_LENGTH,  "Having discarded $s healer's kit, $n ceases $s ministrations.");
		act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
		act (buf2, FALSE, ch, 0, tch, TO_ROOM | TO_ACT_FORMAT);
		return;
	}

	if ( kit->o.od.value[0] <= 0 ) {
		send_to_char ("That healer's kit no longer contains any useful materials.\n", ch);
		return;
	}

	if ( kit->o.od.value[2] && kit->o.od.value[2] > ch->skills [SKILL_HEALING] ) {
		send_to_char ("You do not have the skill required to employ this remedy.\n", ch);
		return;
	}

	if ( ch->right_hand && ch->left_hand ) {
		send_to_char ("You'll need one hand free to treat someone.\n", ch);
		return;
	}

	if ( tch->in_room != ch->in_room ) {
		send_to_char ("You cannot treat someone who isn't here!\n", ch);
		return;
	}

	for ( wound = tch->wounds; wound; wound = next_wound ) {
		next_wound = wound->next;
		if ( !str_cmp (wound->location, location) && (wound->healerskill < ch->skills [SKILL_HEALING] && wound->healerskill != -1 ) && ((str_cmp (wound->severity, "minor") && str_cmp (wound->severity, "small")) || wound->infection ) ) {
			if ( skill_use (ch, SKILL_HEALING, 0) ) {
				if ( mode ) {
					wound->healerskill = ch->skills [SKILL_HEALING];
					if ( kit->o.od.value[1] )
						wound->healerskill += kit->o.od.value[1];
					if ( wound->infection )
						wound->infection = -1;
					if ( kit->o.od.value[3] ) {
						snprintf (buf, MAX_STRING_LENGTH,  "You treat the wound adroitly, making it look considerably better.");
						adjust_wound (tch, wound, kit->o.od.value[3]*-1);
                                                act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
					}
					else 
						snprintf (buf, MAX_STRING_LENGTH,  "You treat and dress the wound skillfully.");
					act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
				}
				else {
					wound->healerskill = ch->skills [SKILL_HEALING];
					if ( kit->o.od.value[1] )
						wound->healerskill += kit->o.od.value[1];
					if ( wound->infection )
						wound->infection = -1;
					if ( kit->o.od.value[3] ) {
						snprintf (buf, MAX_STRING_LENGTH,  "You treat the wound adroitly, making it look considerably better.");
						snprintf (buf2, MAX_STRING_LENGTH,  "$n treats the wound adroitly, making it look considerably better.");
						adjust_wound (tch, wound, kit->o.od.value[3]*-1);
                                                act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
					}
					else {
						snprintf (buf, MAX_STRING_LENGTH,  "You treat and dress the wound skillfully.");
						snprintf (buf2, MAX_STRING_LENGTH,  "$n treats and dresses the wound skillfully.");
					}
					act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
					act (buf2, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
				}
				wound->lasthealed = time(0);
			}
			else {
				if ( mode ) {
					snprintf (buf, MAX_STRING_LENGTH,  "Your ministrations to the wound seem somewhat ineffectual.");
					act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
					wound->healerskill = -1;
				}
				else {
					snprintf (buf, MAX_STRING_LENGTH,  "Your ministrations to the wound seem somewhat ineffectual.");
					snprintf (buf2, MAX_STRING_LENGTH,  "$n's ministrations to your wound seem somewhat ineffectual.");
					act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
					act (buf2, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
					wound->healerskill = -1;
				}
				wound->lasthealed = time(0);
			}
			wound->bleeding = 0;
			break;
		}
	}

	if ( kit )
		kit->o.od.value[0] -= 1;

	if ( kit->o.od.value[0] <= 0 ) {
		send_to_char ("Having used the last of the materials in your kit, you quickly discard it.\n", ch);
		if ( kit->count > 1 ) {
			kit->o.od.value[0] = vtoo(kit->virtual)->o.od.value[0];
			kit->count -= 1;
		}
		else 
			extract_obj (kit);
		return;
	}

	begin_treatment (ch, tch, location, mode);
}

void npc_treatment (CHAR_DATA *ch, CHAR_DATA *mob, char *argument)
{
	WOUND_DATA		*wound;
	char			buf [MAX_STRING_LENGTH];
	char			buf2 [MAX_STRING_LENGTH];
	int				damage = 0;
	float			cost = 0;

	if ( !mob || !IS_SET (mob->act, ACT_PHYSICIAN) ) {
		send_to_char ("I don't see a physician here.\n", ch);
		return;
	}

	if ( !*argument ) {
		send_to_char ("Which wounds did you wish to get treated?\n", ch);
		return;
	}

	if ( mob->delay ) {
		act ("$n appears to be busy.", TRUE, ch, 0, mob, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("How did you wish to be treated?\n", ch);
		return;
	}

	name_to_ident (ch, buf2);
	if ( !ch->wounds ) {
		snprintf (buf, MAX_STRING_LENGTH,  "whisper %s I don't see any wounds on you to treat!", buf2);
		command_interpreter (mob, buf);
		return;
	}

	if ( !str_cmp (buf, "value") ) {
		if ( !*argument ) {
			send_to_char ("For which wounds did you wish to get an appraisal?\n", ch);
			return;
		}
		argument = one_argument (argument, buf);
		if ( !cmp_strn (buf, "all_wounds", strlen(buf)) ) {
			for ( wound = ch->wounds; wound; wound = wound->next ) {
				if ( wound->healerskill )
					continue;
				if ( ch->wounds->next ) {
					cost += wound->damage * 0.85;
					cost += wound->bleeding * 0.85;
				}
				else {
					cost += wound->damage * 0.95;
					cost += wound->bleeding * 0.95;
				}	
			}
			if ( mob->shop )
				cost *= mob->shop->markup;
		}
		else {
			for ( wound = ch->wounds; wound; wound = wound->next ) {
				if ( wound->healerskill )
					continue;
				if ( !strncmp (wound->location, buf, strlen(wound->location)) ) {
					cost += wound->damage * 1.15;
					cost += wound->bleeding * 1.15;
					if ( mob->shop )
						cost *= mob->shop->markup;
					break;
				}
			}
			if ( !wound ) {
				snprintf (buf, MAX_STRING_LENGTH,  "whisper %s I don't see a wound there to treat.", buf2);
				command_interpreter (mob, buf);
				return;
			}
			else if ( wound->healerskill ) {
				snprintf (buf, MAX_STRING_LENGTH,  "whisper %s That wound has already been treated.", buf2);
				command_interpreter (mob, buf);
				return;
			}
		}

		if ( cost < 1 && cost > 0 )
			cost = 1;

		cost = (int) cost;

		if ( !cost ) {
			snprintf (buf, MAX_STRING_LENGTH,  "whisper %s All your wounds have been taken care of - there's nothing I can do.", buf2);
			command_interpreter (mob, buf);
			return;
		}
		snprintf (buf, MAX_STRING_LENGTH,  "whisper %s I'll get you taken care of for a total of %d coppers.", buf2, (int)cost);
		command_interpreter (mob, buf);
		return;
	}

	if ( !cmp_strn (buf, "all_wounds", strlen(buf)) ) {
		for ( wound = ch->wounds; wound; wound = wound->next ) {
			if ( wound->healerskill )
				continue;
			if ( ch->wounds->next ) {
				cost += wound->damage * 0.85;
				cost += wound->bleeding * 0.85;
			}
			else {
				cost += wound->damage * 0.95;
				cost += wound->bleeding * 0.95;
			}
		}
		if ( mob->shop )
			cost *= mob->shop->markup;
	}
	else {
		for ( wound = ch->wounds; wound; wound = wound->next ) {
			if ( !strncmp (wound->location, buf, strlen(wound->location)) ) {
				if ( wound->healerskill )
					continue;
				cost += wound->damage * 1.15;
				cost += wound->bleeding * 1.15;
				if ( mob->shop )
					cost *= mob->shop->markup;
				break;
			}
		}
		if ( !wound ) {
			snprintf (buf, MAX_STRING_LENGTH,  "whisper %s I don't see a wound there to treat.", buf2);
			command_interpreter (mob, buf);
			return;
		}
		else if ( wound->healerskill ) {
			snprintf (buf, MAX_STRING_LENGTH,  "whisper %s That wound has already been treated.", buf2);
			command_interpreter (mob, buf);
			return;
		}
	}

	if ( cost < 1 && cost > 0 )
		cost = 1;

	cost = (int) cost;

	if ( !cost ) {
		snprintf (buf, MAX_STRING_LENGTH,  "whisper %s All your wounds have been taken care of - there's nothing I can do.", buf2);
		command_interpreter (mob, buf);
		return;
	}

	if ( !can_subtract_money (ch, cost, mob->mob->currency_type) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s You seem to be a little short on coin.", buf2);
		do_whisper (mob, buf, 83);
		return; 
	}

	subtract_money (ch, cost, mob->mob->currency_type);
	if ( mob->shop && mob->shop->store_vnum )
		money_to_storeroom (mob, cost);

	send_to_char ("\n", ch);

	act ("$N promptly tends to your wounds.", TRUE, ch, 0, mob, TO_CHAR | TO_ACT_FORMAT);
	act ("$N promptly tends to $n's wounds.", TRUE, ch, 0, mob, TO_ROOM | TO_ACT_FORMAT);

	if ( wound ) {
		if ( skill_use (mob, SKILL_HEALING, 0) ) {
			wound->healerskill = mob->skills [SKILL_HEALING];
			if ( wound->infection )
				wound->infection = -1;
			damage += wound->damage;
		}
		else 
			wound->healerskill = -1;
		wound->bleeding = 0;
	}
	else for ( wound = ch->wounds; wound; wound = wound->next ) {
		if ( wound->healerskill )
			continue;
		if ( skill_use (mob, SKILL_HEALING, 0) ) {
			wound->healerskill = mob->skills [SKILL_HEALING];
			if ( wound->infection )
				wound->infection = -1;
		}
		else 
			wound->healerskill = -1;
		wound->bleeding = 0;
		damage += wound->damage;
	}
}

void do_treat (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*tch = NULL;
	WOUND_DATA	*wound = NULL;
	OBJ_DATA	*kit = NULL;
	int			mode = 0;
	int			location_match = 0;
	int			treatable = 0;
	char		arg [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		buf3 [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;

	(void)one_argument (argument, arg);

	if ( !(tch = get_char_room_vis(ch, arg)) ) {
		for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( !IS_NPC (tch) )
				continue;
			if ( !IS_SET (tch->act, ACT_PHYSICIAN) )
				continue;
			npc_treatment (ch, tch, argument);
			return;
		}
		send_to_char ("You don't see them here.\n", ch);
		return;
	}

	argument = one_argument (argument, arg);

	if ( !ch->skills [SKILL_HEALING] ) {
		send_to_char ("You'd likely only make matters worse. Find a physician!\n", ch);
		return;
	}

	if ( ch->right_hand && GET_ITEM_TYPE (ch->right_hand) == ITEM_HEALER_KIT )
		kit = ch->right_hand;
	else if ( ch->left_hand && GET_ITEM_TYPE (ch->left_hand) == ITEM_HEALER_KIT )
		kit = ch->left_hand;
	else {
		send_to_char ("You need to have a healer's kit handy to treat someone.\n", ch);
		return;
	}

	if ( kit->o.od.value[0] <= 0 ) {
		send_to_char ("That kit no longer contains any useful materials.\n", ch);
		return;
	}

	if ( kit->o.od.value[2] && kit->o.od.value[2] > ch->skills [SKILL_HEALING] ) {
		send_to_char ("You do not have the skill required to employ this remedy.\n", ch);
		return;
	}

	if ( ch->right_hand && ch->left_hand ) {
		send_to_char ("You'll need one hand free to treat someone.\n", ch);
		return;
	}

	if ( tch == ch )
		mode = 1;

	(void)one_argument (argument, arg);

	if ( !*argument ) {
		send_to_char ("Which location on the body are you treating?\n", ch);
		return;
	}

	for ( wound = tch->wounds; wound; wound = wound->next ) {
		if ( !str_cmp (wound->location, arg) )
			location_match += 1;
	}

	if ( !location_match ) {
		if ( mode )
			send_to_char ("You aren't wounded in that area.\n", ch);
		else
			send_to_char ("Your patient is not wounded in that area.\n", ch);
		return;
	}

	else {
		for ( wound = tch->wounds; wound; wound = wound->next )
			if ( wound->healerskill < ch->skills [SKILL_HEALING] && wound->healerskill != -1 )
				treatable += 1;
	}

	if ( !treatable ) {
		if ( mode )
			send_to_char ("You have already been treated by a healer.\n", ch);
		else
			send_to_char ("From the looks of it, your patient has already been treated.\n", ch);
		return;
	}

	else {
		temp_arg = expand_wound_loc(arg);
		if ( mode ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#6Note: Be sure to EMOTE out this scene with proper roleplay!#0\n\nYou begin to work at treating the wounds on your %s...", temp_arg);
			snprintf (buf2, MAX_STRING_LENGTH,  "#6Note: Be sure to EMOTE out this scene with proper roleplay!#0\n\n$n begins to work at treating the wounds on $n's %s...", temp_arg);
			act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			act (buf2, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		}
		else {
			snprintf (buf, MAX_STRING_LENGTH,  "#6Note: Be sure to EMOTE out this scene with proper roleplay!#0\n\nYou begin to work at treating the wounds on $N's %s...", temp_arg);
			snprintf (buf2, MAX_STRING_LENGTH,  "#6Note: Be sure to EMOTE out this scene with proper roleplay!#0\n\n$n begins to work at treating the wounds on your %s...", temp_arg);
			snprintf (buf3, MAX_STRING_LENGTH,  "$n begins to work at treating the wounds on $N's %s...", temp_arg);
			act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
			act (buf2, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
			act (buf3, FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
		}
	}

	begin_treatment (ch, tch, arg, mode);

	return;
}

void do_health	(CHAR_DATA *ch, char *argument, int cmd)
{
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	char	*temp_wound = NULL;
	char	*temp_fat = NULL;
	char	*temp_mana = NULL;
	
	(void)show_wounds(ch, 1);
	
	temp_wound = wound_total(ch);
	temp_fat = fatigue_bar(ch);
	temp_mana = mana_bar(ch);
	
	snprintf (buf, MAX_STRING_LENGTH,  "\nOverall Health:    %s\n"
              "Remaining Stamina: %s\n", temp_wound, temp_fat);

	if ( ch->max_mana && ch->mana )
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "Remaining Mana:    %s\n", temp_mana);

	send_to_char (buf, ch);
}

char *expand_wound_loc (char *location)
{
	static char buf [30];

	if ( !str_cmp(location, "reye") )
		snprintf (buf, MAX_STRING_LENGTH,  "right eye");

	else if ( !str_cmp(location, "leye") )
		snprintf (buf, MAX_STRING_LENGTH,  "left eye");

	else if ( !str_cmp(location, "rshoulder") )
		snprintf (buf, MAX_STRING_LENGTH,  "right shoulder");

	else if ( !str_cmp(location, "lshoulder") )
		snprintf (buf, MAX_STRING_LENGTH,  "left shoulder");

	else if ( !str_cmp(location, "rupperarm") )
		snprintf (buf, MAX_STRING_LENGTH,  "right upper arm");

	else if ( !str_cmp(location, "lupperarm") )
		snprintf (buf, MAX_STRING_LENGTH,  "left upper arm");

	else if ( !str_cmp(location, "relbow") )
		snprintf (buf, MAX_STRING_LENGTH,  "right elbow");

	else if ( !str_cmp(location, "lelbow") )
		snprintf (buf, MAX_STRING_LENGTH,  "left elbow");

	else if ( !str_cmp(location, "rforearm") )
		snprintf (buf, MAX_STRING_LENGTH,  "right forearm");

	else if ( !str_cmp(location, "lforearm") )
		snprintf (buf, MAX_STRING_LENGTH,  "left forearm");

	else if ( !str_cmp(location, "rhand") )
		snprintf (buf, MAX_STRING_LENGTH,  "right hand");

	else if ( !str_cmp(location, "lhand") )
		snprintf (buf, MAX_STRING_LENGTH,  "left hand");

	else if ( !str_cmp(location, "rhip") )
		snprintf (buf, MAX_STRING_LENGTH,  "right hip");

	else if ( !str_cmp(location, "lhip") )
		snprintf (buf, MAX_STRING_LENGTH,  "left hip");

	else if ( !str_cmp(location, "rthigh") )
		snprintf (buf, MAX_STRING_LENGTH,  "right thigh");

	else if ( !str_cmp(location, "lthigh") )
		snprintf (buf, MAX_STRING_LENGTH,  "left thigh");

	else if ( !str_cmp(location, "rknee") )
		snprintf (buf, MAX_STRING_LENGTH,  "right knee");

	else if ( !str_cmp(location, "lknee") )
		snprintf (buf, MAX_STRING_LENGTH,  "left knee");

	else if ( !str_cmp(location, "rcalf") )
		snprintf (buf, MAX_STRING_LENGTH,  "right calf");

	else if ( !str_cmp(location, "lcalf") )
		snprintf (buf, MAX_STRING_LENGTH,  "left calf");

	else if ( !str_cmp(location, "rfoot") )
		snprintf (buf, MAX_STRING_LENGTH,  "right foot");

	else if ( !str_cmp(location, "lfoot") )
		snprintf (buf, MAX_STRING_LENGTH,  "left foot");

	else if ( !str_cmp(location, "rforeleg") )
		snprintf (buf, MAX_STRING_LENGTH,  "right foreleg");

	else if ( !str_cmp(location, "rhindleg") )
		snprintf (buf, MAX_STRING_LENGTH,  "right hindleg");

	else if ( !str_cmp(location, "lforeleg") )
		snprintf (buf, MAX_STRING_LENGTH,  "left foreleg");

	else if ( !str_cmp(location, "lhindleg") )
		snprintf (buf, MAX_STRING_LENGTH,  "left hindleg");

	else if ( !str_cmp(location, "rforepaw") )
		snprintf (buf, MAX_STRING_LENGTH,  "right forepaw");

	else if ( !str_cmp(location, "lforepaw") )
		snprintf (buf, MAX_STRING_LENGTH,  "left forepaw");

	else if ( !str_cmp(location, "rhindpaw") )
		snprintf (buf, MAX_STRING_LENGTH,  "right hindpaw");

	else if ( !str_cmp(location, "lhindpaw") )
		snprintf (buf, MAX_STRING_LENGTH,  "left hindpaw");

	else if ( !str_cmp(location, "rforehoof") )
		snprintf (buf, MAX_STRING_LENGTH,  "right forehoof");

	else if ( !str_cmp(location, "lforehoof") )
		snprintf (buf, MAX_STRING_LENGTH,  "left forehoof");

	else if ( !str_cmp(location, "rhindhoof") )
		snprintf (buf, MAX_STRING_LENGTH,  "right hindhoof");

	else if ( !str_cmp(location, "lhindhoof") )
		snprintf (buf, MAX_STRING_LENGTH,  "left hindhoof");

	else if ( !str_cmp(location, "rforefoot") )
		snprintf (buf, MAX_STRING_LENGTH,  "right forefoot");

	else if ( !str_cmp(location, "lforefoot") )
		snprintf (buf, MAX_STRING_LENGTH,  "left forefoot");

	else if ( !str_cmp(location, "rhindfoot") )
		snprintf (buf, MAX_STRING_LENGTH,  "right hindfoot");

	else if ( !str_cmp(location, "lhindfoot") )
		snprintf (buf, MAX_STRING_LENGTH,  "left hindfoot");

	else if ( !str_cmp(location, "rleg") )
		snprintf (buf, MAX_STRING_LENGTH,  "right leg");

	else if ( !str_cmp(location, "lleg") )
		snprintf (buf, MAX_STRING_LENGTH,  "left leg");

	else if ( !str_cmp(location, "rwing") )
		snprintf (buf, MAX_STRING_LENGTH,  "right wing");

	else if ( !str_cmp(location, "lwing") )
		snprintf (buf, MAX_STRING_LENGTH,  "left wing");

	else if ( !str_cmp(location, "stinger") )
		snprintf (buf, MAX_STRING_LENGTH,  "barbed stinger");

	else snprintf (buf, MAX_STRING_LENGTH, "%s",  location);

	return buf;

}

char *show_wounds (CHAR_DATA *ch, int mode)
{
	WOUND_DATA	*wound;
	LODGED_OBJECT_INFO	*lodged;
	char		buf4[MAX_STRING_LENGTH] = {'\0'};
	char		buf3[MAX_STRING_LENGTH] = {'\0'};
	char		buf2[MAX_STRING_LENGTH] = {'\0'};
	static char	buf[MAX_STRING_LENGTH] = {'\0'};
	int			curdamage = 0;
	char		*temp_arg = NULL;


	if ( ch->wounds && mode == 0 ) {
		*buf = '\0';
		if ( is_hooded (ch) ){
			snprintf (buf2, MAX_STRING_LENGTH,  "It has ");
		}
		else{
			temp_arg = char_short(ch);
			snprintf (buf2, MAX_STRING_LENGTH,  "%s has ", temp_arg);
		}
		
		*buf2 = toupper (*buf2);
		strcat (buf, buf2);
	}

	if ( mode == 1 )
		send_to_char ("\n", ch);

	if ( mode == 1 && get_affect (ch, AFFECT_LOST_CON) ) {
		send_to_char ("You feel considerably less hale from the beating you took.\n", ch);
	}
	else if ( mode == 1 && !ch->wounds && !ch->damage && !ch->lodged ) {
		send_to_char("You are in excellent condition.\n", ch);
		return NULL;
	}

	if ( mode == 1 && ch->damage ) {
		for ( wound = ch->wounds; wound; wound = wound->next ){
			curdamage += wound->damage;
		}
		
		curdamage += ch->damage;
		
		if ( curdamage > 0 && curdamage <= ch->max_hit*.25 )
			snprintf (buf3, MAX_STRING_LENGTH,  "You feel slightly faint.\n");
		
		else if ( curdamage > ch->max_hit*.25 && curdamage < ch->max_hit*.80 )
			snprintf (buf3, MAX_STRING_LENGTH,  "You feel considerably weakened.\n");
		
		else if ( curdamage > ch->max_hit*.80 && curdamage < ch->max_hit*.95 )
			snprintf (buf3, MAX_STRING_LENGTH,  "You feel as if you can barely remain standing.\n");
		
		else if ( curdamage > ch->max_hit*.95 )
			snprintf (buf3, MAX_STRING_LENGTH,  "You feel yourself growing cold and faint...\n");
		
		if ( mode == 1 && get_affect (ch, AFFECT_LOST_CON) )
			send_to_char ("\n", ch);
		send_to_char (buf3, ch);
	}

	if ( ch->wounds && mode == 1 )
		snprintf (buf, MAX_STRING_LENGTH,  "You have ");

	for ( wound = ch->wounds; wound; wound = wound->next ) {

		if ( !wound->next && wound != ch->wounds )
			snprintf (buf4, MAX_STRING_LENGTH,  "and a ");
		else
			snprintf (buf4, MAX_STRING_LENGTH,  "a ");
		
		if ( mode == 0 ) {
			if ( wound->bleeding > 0 && wound->bleeding <= 3 )
				strcat (buf4, "#1lightly bleeding#0 ");
			
			else if ( wound->bleeding > 3 && wound->bleeding <= 6 )
				strcat (buf4, "#1moderately bleeding#0 ");
			
			else if ( wound->bleeding > 6 && wound->bleeding <= 9 )
				strcat (buf4, "#1heavily bleeding#0 ");
			
			else if ( wound->bleeding > 9 )
				strcat (buf4, "#1gushing#0 ");
		}
		
		strcat (buf, buf4);
		temp_arg = expand_wound_loc(wound->location);
		snprintf (buf4, MAX_STRING_LENGTH,  "%s %s on the %s", wound->severity, wound->name, temp_arg);
		
		if ( mode == 1 && wound->infection )
			strcat (buf4, " #3(infected)#0");
		
		if ( mode == 1 && wound->bleeding ) {
			if ( wound->bleeding > 0 && wound->bleeding <= 3)
				strcat (buf4, " #1(bleeding: light)#0");
			
			else if ( wound->bleeding > 3 && wound->bleeding <= 6 )
				strcat (buf4, " #1(bleeding: moderate)#0");
			
			else if ( wound->bleeding > 6 && wound->bleeding <= 9 )
				strcat (buf4, " #1(bleeding: heavy)#0");
			
			else if ( wound->bleeding > 9 )
				strcat (buf4, " #1(bleeding: mortal)#0");
		}
		
		if ( mode == 1 && wound->healerskill > 0 )
			strcat (buf4, " #6(treated)#0");
		
		if ( mode == 1 && wound->healerskill == -1 )
			strcat (buf4, " #4(tended)#0");
		
		strcat (buf, buf4);
		
		if ( !wound->next )
			strcat (buf, ".#0");
		else
			strcat (buf, ", ");
		*buf4 = '\0';
	}

	if ( ch->lodged && mode == 0 ) {
		snprintf (buf2, MAX_STRING_LENGTH,  "%s has ", HSSH(ch));
		*buf2 = toupper(*buf2);	
		
		if ( ch->wounds )
			strcat (buf, "\n\n");
		
		strcat (buf, buf2);
	}

	else if ( ch->lodged && mode == 1 ) {
		if ( ch->wounds )
			strcat (buf, "\n\n");
		strcat (buf, "You have ");
	}

	for ( lodged = ch->lodged; lodged; lodged = lodged->next ) {
		if ( lodged->next ){
			temp_arg = expand_wound_loc(lodged->location);
			snprintf (buf2, MAX_STRING_LENGTH,  "#2%s#0 lodged in the %s, ", vtoo(lodged->vnum)->short_description, temp_arg);
		}
		
		else if ( !lodged->next && lodged != ch->lodged ){
			temp_arg = expand_wound_loc(lodged->location);
			snprintf (buf2, MAX_STRING_LENGTH,  "and #2%s#0 lodged in the %s.", vtoo(lodged->vnum)->short_description, temp_arg);
		}
		
		else if ( !lodged->next && lodged == ch->lodged ){
			temp_arg = expand_wound_loc(lodged->location);
			snprintf (buf2, MAX_STRING_LENGTH,  "#2%s#0 lodged in the %s.", vtoo(lodged->vnum)->short_description, temp_arg);
		}
		
		strcat (buf, buf2);
	}

	if ( mode == 1 && *buf ) {
		send_to_char("\n", ch);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return NULL;
	}
	else if ( mode == 1 )
		return NULL;

	return buf;
}

/** do_diagnose is an staff command only. Mainly large amounts of debug info that can be rewritten in the future to mkae this a viable command.  **/
void do_diagnose (CHAR_DATA *ch, char *argument, int cmd)
{

	int 		i;
	int			damage = 0;
	int			totdam = 0;
	int			loaded_char = 0;
	int			bleeding = 0;
	int			treated = 0;
	int			infected = 0;
	int			tended = 0;
	char 		buf [MAX_STRING_LENGTH];
	char		buf2 [MAX_STRING_LENGTH];
	char		buf3 [MAX_STRING_LENGTH];
	char		arg [MAX_STRING_LENGTH];
	WOUND_DATA 	*wound = NULL;
	CHAR_DATA 	*tch = NULL;
	char		*temp_arg = NULL;
	
	(void)one_argument (argument, arg);

	if ( !IS_MORTAL(ch) ) {
		if ( !(tch = get_char_room_vis(ch, arg)) ) {
			if ( !(tch = get_char(arg)) ) {
				if ( !(tch = load_pc(arg)) ) {
					send_to_char("No such being.\n", ch);
					return;
				}
				else loaded_char = 1;
			}
		}

		send_to_char("\n", ch);
		snprintf (buf, MAX_STRING_LENGTH,  "%s's wounds:", tch->tname);
		*buf = toupper (*buf);
		send_to_char(buf, ch);
		*buf2 = '\0';
		for ( i = 0; i <= strlen(buf); i++ ) {
			if ( i == strlen(buf) )
				strcat(buf2, "\n");
			else
				strcat(buf2, "-");
		}
		
		send_to_char("\n", ch);
		send_to_char(buf2, ch);

		for ( wound = tch->wounds; wound; wound = wound->next ) {
			if (wound == tch->wounds)
				send_to_char("\n", ch);
			damage = wound->damage;
			totdam += damage;
			
			temp_arg = expand_wound_loc(wound->location);
			snprintf (buf3, MAX_STRING_LENGTH,  "A %s %s on the %s.", wound->severity, wound->name, temp_arg);
			pad_buffer(buf3, 45);
			snprintf (buf2, MAX_STRING_LENGTH,  "-> #1Points: %d.#0", damage);
			
			if (wound->infection) {
				strcat(buf2, " #3(I)#0");
				infected = 1;
			}
			
			if (wound->healerskill > 0) {
				strcat(buf2, " #6(Tr)#0");
				treated = 1;
			}
			
			if (wound->healerskill == -1) {
				strcat(buf2, " #4(Te)#0 ");
				tended = 1;
			}
			
			if (wound->bleeding) {
				snprintf (buf, MAX_STRING_LENGTH,  " #1(B:%d)#0", wound->bleeding);
				bleeding = 1;
				strcat(buf2, buf);
			}
			
			strcat(buf2, "\n");
			strcat(buf3, buf2);
			send_to_char(buf3, ch);
		}
		*buf3 = '\0';
		if (infected) strcat(buf3, "#3( I = Infected )#0 ");
		if (bleeding) strcat(buf3, "#1( B = Bleeding )#0 ");
		if (treated) strcat(buf3, "#6( Tr = Treated )#0 ");
		if (tended) strcat(buf3, "#4( Te = Tended ) #0 ");
		if (infected || bleeding || treated) {
			send_to_char("\n", ch);
			send_to_char(buf3, ch);
			send_to_char("\n", ch);
		}
		
		if ( tch->damage ) {
			snprintf (buf3, MAX_STRING_LENGTH,  "\n#5Bloodloss Points: %d.#0\n", tch->damage);
			send_to_char(buf3, ch);
		}
		
		if ( !tch->wounds && tch->damage <= 0) {
			temp_arg = char_short(tch);
			snprintf (buf3, MAX_STRING_LENGTH,  "%s does not currently seem to be wounded.\n", temp_arg);
			*buf3 = toupper (*buf3);
			send_to_char("\n", ch);
			send_to_char(buf3, ch);
		}
		
		else {
			totdam += tch->damage;
			snprintf (buf3, MAX_STRING_LENGTH,  "\n#1Total Injury Points: %d.#0\n", totdam);
			send_to_char(buf3, ch);
			snprintf (buf3, MAX_STRING_LENGTH,  "\n#1Total Injury Point Limit: %d.#0\n", tch->max_hit);
			send_to_char(buf3, ch);
		}
		if (loaded_char)
			unload_pc(tch);
	}
	else {
		send_to_char ("The physician code has not yet been implemented.\n", ch);
		return;
	}
}

char *wound_total (CHAR_DATA *ch)
{
	WOUND_DATA *wound;
	static char buf [75];
	int damage = 0, limit = 0;

	limit = ch->max_hit;

	for ( wound = ch->wounds;
		wound;
		wound = wound->next) {
		damage += wound->damage;
	}

	damage += ch->damage;
	
	if (damage <= 0)
		snprintf (buf, MAX_STRING_LENGTH,  "#1**#3**#2**#0");

	else if (damage && damage <= limit*.1667 )
		snprintf (buf, MAX_STRING_LENGTH,  "#1**#3**#2*#0 ");

	else if (damage > limit*.1667 && damage <= limit*.3333 )
		snprintf (buf, MAX_STRING_LENGTH,  "#1**#3**#0  ");

	else if (damage > limit*.3333 && damage < limit*.6667 )
		snprintf (buf, MAX_STRING_LENGTH,  "#1**#3*#0   ");

	else if ( damage > limit*.6667 && damage < limit*.8335 )
		snprintf (buf, MAX_STRING_LENGTH,  "#1**#0    ");

	else if (damage >= limit*.8335 )
		snprintf (buf, MAX_STRING_LENGTH,  "#1*#0     ");

	return buf;
}

char *figure_location (CHAR_DATA *tar, int location)
{
	int locroll = number(1,100);
	static char loc[50];

	/*  Location 0: Body area. */
	/*  Location 1: Leg area. */
	/*  Location 2: Arm area. */
	/*  Location 3: Head area. */
	/*  Location 4: Neck area. */


	switch (tar->race) {
		case RACE_SNAKE:
		case RACE_SERPENT:	/*  Serpentine creatures. */
			if ( location == 0 || location == 3 ) {
				if ( locroll >= 1 && locroll <= 50 )
					snprintf (loc, 5, "back");
				else if ( locroll > 50 )
					snprintf (loc, 11, "underbelly");
			}
			else if ( location == 1 || location == 2 )
				snprintf (loc, 5, "tail");
			else if ( location == 4 ) {
				snprintf (loc, 5, "neck");
			}
			break;
		case RACE_GOOSE:
		case RACE_BIRD:
		case RACE_BAT:	/*  Winged, without tails; birds, bats, etc. */
			if ( location == 0 ) {
				if ( locroll >= 1 && locroll <= 40 )
					snprintf (loc, 7, "thorax");
				else if ( locroll > 40 && locroll <= 80 )
					snprintf (loc, 8, "abdomen");
				else if ( locroll > 80 ) {
					if ( !number(0,1) )
						snprintf (loc, 5, "rhip");
					else
						snprintf (loc, 5, "lhip");
				}
			}
			else if ( location == 1 ) {
				if ( locroll >= 1 && locroll <= 40 )
					snprintf (loc, 5, "rleg");
				else if ( locroll > 40 && locroll <= 80 )
					snprintf (loc, 5, "lleg");
				else if ( locroll > 80 && locroll <= 90 )
					snprintf (loc, 6, "rfoot");
				else if ( locroll > 90 )
					snprintf (loc, 6, "lfoot");
			}
			else if ( location == 2 ) {
				if ( locroll >= 1 && locroll <= 50 )
					snprintf (loc, 6, "rwing");
				else if ( locroll > 50 )
					snprintf (loc, 6, "lwing");
			}
			else if ( location == 3 ) {
				if ( locroll >= 1 && locroll <= 40 )
					snprintf (loc, 6, "skull");
				else if ( locroll > 40 && locroll <= 50 )
					snprintf (loc, 5, "reye");
				else if ( locroll > 50 && locroll <= 60 )
					snprintf (loc, 5, "leye");
				else if ( locroll > 60 )
					snprintf (loc, 5, "face");
			}
			else if ( location == 4 ) {
				snprintf (loc, 5, "neck");
			}
			break;
		case RACE_TOAD:
		case RACE_SCORPION:	
		case RACE_REPTILE:
		case RACE_INSECT:	
		case RACE_FROG:
		case RACE_FISH:		
		case RACE_BEAR:
		case RACE_ARACHNID:	
		case RACE_AMPHIBIAN:	/*  Four-legged animals with "feet". */
			if ( location == 0 ) {
				if ( locroll >= 1 && locroll <= 50 )
					snprintf (loc, 7, "thorax");
				else if ( locroll > 50 )
					snprintf (loc, 8, "abdomen");
			}
			else if ( location == 1 ) {
				if ( locroll >= 1 && locroll <= 5 )
					snprintf (loc, 6, "groin");
				else if ( locroll > 5 && locroll <= 20 )
					snprintf (loc, 8, "abdomen");
				else if ( locroll > 20 && locroll <= 40 )
					snprintf (loc, 9, "rhindleg");
				else if ( locroll > 40 && locroll <= 60 )
					snprintf (loc, 9, "lhindleg");
				else if ( locroll > 60 && locroll <= 80 )
					snprintf (loc, 5, "tail");
				else if ( locroll > 80 && locroll <= 90 )
					snprintf (loc, 10, "rhindfoot");
				else if ( locroll > 90 )
					snprintf (loc, 10, "lhindfoot");
			}
			else if ( location == 2 ) {
				if ( locroll >= 1 && locroll <= 5 )
					snprintf (loc, 7, "thorax");
				else if ( locroll > 5 && locroll <= 20 )
					snprintf (loc, 8, "abdomen");
				else if ( locroll > 20 && locroll <= 40 )
					snprintf (loc, 9, "rforeleg");
				else if ( locroll > 40 && locroll <= 60 )
					snprintf (loc, 9, "lforeleg");
				else if ( locroll > 60 && locroll <= 70 )
					snprintf (loc, 5, "neck");
				else if ( locroll > 70 && locroll <= 85 )
					snprintf (loc, 10, "rforefoot");
				else if ( locroll > 85 )
					snprintf (loc, 10, "lforefoot");
			}
			else if ( location == 3 ) {
				if ( locroll >= 1 && locroll <= 40 )
					snprintf (loc, 6, "skull");
				else if ( locroll > 40 && locroll <= 50 )
					snprintf (loc, 5, "reye");
				else if ( locroll > 50 && locroll <= 60 )
					snprintf (loc, 5, "leye");
				else if ( locroll > 60 )
					snprintf (loc, 7, "muzzle");
			}
			else if ( location == 4 ) {
				snprintf (loc, 5, "neck");
			}
			break;
		case RACE_WOLF:
		case RACE_RAT:
		case RACE_RABBIT:
		case RACE_MOUSE:
		case RACE_MINK:
		case RACE_FOX:
		case RACE_FERRET:
		case RACE_DOG:
		case RACE_CAT:
		case RACE_BOBCAT:	/*  Four-legged animals with "paws". */
			if ( location == 0 ) {
				if ( locroll >= 1 && locroll <= 50 )
					snprintf (loc, 7, "thorax");
				else if ( locroll > 50 )
					snprintf (loc, 8, "abdomen");
			}
			else if ( location == 1 ) {
				if ( locroll >= 1 && locroll <= 5 )
					snprintf (loc, 6, "groin");
				else if ( locroll > 5 && locroll <= 20 )
					snprintf (loc, 8, "abdomen");
				else if ( locroll > 20 && locroll <= 40 )
					snprintf (loc, 9, "rhindleg");
				else if ( locroll > 40 && locroll <= 60 )
					snprintf (loc, 9, "lhindleg");
				else if ( locroll > 60 && locroll <= 80 )
					snprintf (loc, 5, "tail");
				else if ( locroll > 80 && locroll <= 90 )
					snprintf (loc, 9, "rhindpaw");
				else if ( locroll > 90 )
					snprintf (loc, 9, "lhindpaw");
			}
			else if ( location == 2 ) {
				if ( locroll >= 1 && locroll <= 5 )
					snprintf (loc, 7, "thorax");
				else if ( locroll > 5 && locroll <= 20 )
					snprintf (loc, 8, "abdomen");
				else if ( locroll > 20 && locroll <= 40 )
					snprintf (loc, 9, "rforeleg");
				else if ( locroll > 40 && locroll <= 60 )
					snprintf (loc, 9, "lforeleg");
				else if ( locroll > 60 && locroll <= 70 )
					snprintf (loc, 5, "neck");
				else if ( locroll > 70 && locroll <= 85 )
					snprintf (loc, 9, "rforepaw");
				else if ( locroll > 85 )
					snprintf (loc, 9, "lforepaw");
			}
			else if ( location == 3 ) {
				if ( locroll >= 1 && locroll <= 40 )
					snprintf (loc, 6, "skull");
				else if ( locroll > 40 && locroll <= 50 )
					snprintf (loc, 5, "reye");
				else if ( locroll > 50 && locroll <= 60 )
					snprintf (loc, 5, "leye");
				else if ( locroll > 60 )
					snprintf (loc, 7, "muzzle");
			}
			else if ( location == 4 ) {
				snprintf (loc, 5, "neck");
			}
			break;
		case RACE_SHEEP:
		case RACE_SWINE:
		case RACE_PONY:
		case RACE_HORSE:
		case RACE_WARHORSE:
		case RACE_GOAT:
		case RACE_COW:
		case RACE_DEER:
		case RACE_BOAR:	/*  Animals with "hooves". */
			if ( location == 0 ) {
				if ( locroll >= 1 && locroll <= 50 )
					snprintf (loc, 7, "thorax");
				else if ( locroll > 50 )
					snprintf (loc, 8, "abdomen");
			}
			else if ( location == 1 ) {
				if ( locroll >= 1 && locroll <= 5 )
					snprintf (loc, 6, "groin");
				else if ( locroll > 5 && locroll <= 20 )
					snprintf (loc, 8, "abdomen");
				else if ( locroll > 20 && locroll <= 40 )
					snprintf (loc, 9, "rhindleg");
				else if ( locroll > 40 && locroll <= 60 )
					snprintf (loc, 9, "lhindleg");
				else if ( locroll > 60 && locroll <= 80 )
					snprintf (loc, 5, "tail");
				else if ( locroll > 80 && locroll <= 90 )
					snprintf (loc, 10, "rhindhoof");
				else if ( locroll > 90 )
					snprintf (loc, 10, "lhindhoof");
			}
			else if ( location == 2 ) {
				if ( locroll >= 1 && locroll <= 5 )
					snprintf (loc, 7, "thorax");
				else if ( locroll > 5 && locroll <= 20 )
					snprintf (loc, 8, "abdomen");
				else if ( locroll > 20 && locroll <= 40 )
					snprintf (loc, 9, "rforeleg");
				else if ( locroll > 40 && locroll <= 60 )
					snprintf (loc, 9, "lforeleg");
				else if ( locroll > 60 && locroll <= 70 )
					snprintf (loc, 5, "neck");
				else if ( locroll > 70 && locroll <= 85 )
					snprintf (loc, 10, "rforehoof");
				else if ( locroll > 85 )
					snprintf (loc, 10, "lforehoof");
			}
			else if ( location == 3 ) {
				if ( locroll >= 1 && locroll <= 40 )
					snprintf (loc, 6, "skull");
				else if ( locroll > 40 && locroll <= 50 )
					snprintf (loc, 5, "reye");
				else if ( locroll > 50 && locroll <= 60 )
					snprintf (loc, 5, "leye");
				else if ( locroll > 60 )
					snprintf (loc, 7, "muzzle");
			}
			else if ( location == 4 ) {
				snprintf (loc, 5, "neck");
			}
			break;
		default:	/*  Humanoid body prototype. */
			if ( location == 0 ) {
				if ( locroll >= 1 && locroll <= 40 )
					snprintf (loc, 7, "thorax");
				else if ( locroll > 40 && locroll <= 80 )
					snprintf (loc, 8, "abdomen");
				else if ( locroll > 80 ) {
					if ( !number(0,1) )
						snprintf (loc, 5, "rhip");
					else
						snprintf (loc, 5, "lhip");
				}
			}
			else if ( location == 1 ) {
				if ( locroll >= 1 && locroll <= 3 )
					snprintf (loc, 6, "groin");
				else if ( locroll > 3 && locroll <= 20 )
					snprintf (loc, 7, "rthigh");
				else if ( locroll > 20 && locroll <= 40 )
					snprintf (loc, 7, "lthigh");
				else if ( locroll > 40 && locroll <= 45 )
					snprintf (loc, 6, "rknee");
				else if ( locroll > 45 && locroll <= 50 )
					snprintf (loc, 6, "lknee");
				else if ( locroll > 50 && locroll <= 70 )
					snprintf (loc, 6, "rcalf");
				else if ( locroll > 70 && locroll <= 90 )
					snprintf (loc, 6, "lcalf");
				else if ( locroll > 90 && locroll <= 95 )
					snprintf (loc, 6, "rfoot");
				else if ( locroll > 95 )
					snprintf (loc, 6, "lfoot");
			}
			else if ( location == 2 ) {
				if ( locroll >= 1 && locroll <= 10 )
					snprintf (loc, 10, "rshoulder");
				else if ( locroll > 10 && locroll <= 20 )
					snprintf (loc, 10, "lshoulder");
				else if ( locroll > 20 && locroll <= 35 )
					snprintf (loc, 10, "rupperarm");
				else if ( locroll > 35 && locroll <= 50 )
					snprintf (loc, 10, "lupperarm");
				else if ( locroll > 50 && locroll <= 55 )
					snprintf (loc, 7, "relbow");
				else if ( locroll > 55 && locroll <= 60 )
					snprintf (loc, 7, "lelbow");
				else if ( locroll > 60 && locroll <= 75 )
					snprintf (loc, 9, "rforearm");
				else if ( locroll > 75 && locroll <= 90 )
					snprintf (loc, 9, "lforearm");
				else if ( locroll > 90 && locroll <= 95 )
					snprintf (loc, 6, "rhand");
				else if ( locroll > 95 )
					snprintf (loc, 6, "lhand");
			}
			else if ( location == 3 ) {
				if ( locroll >= 1 && locroll <= 45 )
					snprintf (loc, 6, "skull");
				else if ( locroll > 45 && locroll <= 50 )
					snprintf (loc, 5, "reye");
				else if ( locroll > 50 && locroll <= 55 )
					snprintf (loc, 5, "leye");
				else if ( locroll > 55 )
					snprintf (loc, 5, "face");
			}
			else if ( location == 4 ) {
				snprintf (loc, 5, "neck");
			}
			break;
	}

	return loc;
}

char *downsized_wound (CHAR_DATA *ch, WOUND_DATA *wound)
{
	static char	buf [MAX_STRING_LENGTH];
	int		limit = 0;

	if ( !ch || !wound )
		return NULL;

	*buf = '\0';

	limit = ch->max_hit;
	if ( wound->damage <= (limit*.02) )
		snprintf (buf, 6,  "small");
	else if ( wound->damage > (limit*.02) && wound->damage <= (limit*.10) )
		snprintf (buf, 6,  "minor");
	else if ( wound->damage > (limit*.10) && wound->damage <= (limit*.20) )
		snprintf (buf, 9,  "moderate");
	else if ( wound->damage > (limit*.20) && wound->damage <= (limit*.30) )
		snprintf (buf, 7,  "severe");
	else if ( wound->damage > (limit*.30) && wound->damage <= (limit*.40) )
		snprintf (buf, 9,  "grievous");
	else if ( wound->damage > (limit*.40) && wound->damage <= (limit*.50))
		snprintf (buf, 9,  "terrible");
	else if ( wound->damage > (limit*.50) )
		snprintf (buf, 7,  "mortal");

	return buf;
}

int natural_healing_check (CHAR_DATA *ch, WOUND_DATA *wound)
{
	char		buf[MAX_STRING_LENGTH]; /*  used for debug statements */
	char		woundbuf[MAX_STRING_LENGTH];
	int			roll = 0;
	int			needed = 0;
	int			hr = 0;
	int			upper = 0;
	char		*temp_arg = NULL;

	if ( !str_cmp (wound->severity, "small") || !str_cmp (wound->severity, "minor") )
		hr = 6;
	else if ( !str_cmp (wound->severity, "moderate") )
		hr = 5;
	else if ( !str_cmp (wound->severity, "severe") )
		hr = 4;
	else if ( !str_cmp (wound->severity, "grievous") )
		hr = 3;
	else if ( !str_cmp (wound->severity, "terrible") )
		hr = 2;
	else
		hr = 1;

	wound->lasthealed = time(0);
	
	temp_arg = expand_wound_loc(wound->location);
	snprintf (buf, MAX_STRING_LENGTH,  "Now attempting to heal a %s on the %s...\n",
		wound->name, temp_arg);

	
	if ( wound->infection == -1 ) {
		temp_arg = expand_wound_loc(wound->location);
		snprintf (buf, MAX_STRING_LENGTH,  "You feel the raging infection in a %s %s on your %s gradually subside.", wound->severity, wound->name, temp_arg);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		wound->infection = 0;
	}

	if ( wound->healerskill == -1 ) {
		temp_arg = expand_wound_loc(wound->location);
		snprintf (buf, MAX_STRING_LENGTH,  "You feel as if another treatment may benefit a %s %s on your %s.", wound->severity, wound->name, temp_arg);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		wound->healerskill = 0;
	}

	if ( wound->infection > 0 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Wound is infected -- bypassing...\n");
		return 0;
	}

	needed = hr * ch->con;
	needed = MIN(needed, 95);

	roll = number (1,100);
	if ( GET_POS(ch) == POSITION_SLEEPING ) roll -= 25;
	if ( GET_POS(ch) == POSITION_RESTING ) roll -= 15;
	if ( GET_POS(ch) == POSITION_SITTING ) roll -= 5;
	if ( wound->healerskill && wound->healerskill != -1 )
		roll -= wound->healerskill/3;
	roll = MAX(roll, 1);

	snprintf (buf, MAX_STRING_LENGTH,  "Roll: %d.\n", roll);
	snprintf (buf, MAX_STRING_LENGTH,  "Roll Needed: %d ( %d CON x HR %d ).\n", needed, ch->con, hr);

	*woundbuf = '\0';

	if (roll <= needed) {
		if ( roll % 5 == 0 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Critical healing success.\n");
			if ( ch->con/3 < 3 )
				upper = 3;
			else upper = number (2, ch->con/3);
			wound->damage -= number(1,upper);
			if ( wound->healerskill >= 0 )
				wound->damage -= (wound->healerskill/25);
		}
		else {
			snprintf (buf, MAX_STRING_LENGTH,  "Healing success.\n");
			if ( ch->con/5 < 3 )
				upper = 2;
			else upper = number (2, ch->con/5);
			wound->damage -= number(1,upper);
			if ( wound->healerskill >= 0 )
				wound->damage -= (wound->healerskill/25);
		}
		if ( wound->healerskill == -1 )
			wound->healerskill = 0;
		if ( wound->damage > 0 ) {
			temp_arg = downsized_wound (ch, wound);
			snprintf (buf, MAX_STRING_LENGTH,  "%s", temp_arg);
			mem_free (wound->severity);
			wound->severity = NULL;
			wound->severity = str_dup(buf);
		}
	}
	else if ( roll > needed) {
		if ( roll % 5 == 0 && WOUND_INFECTIONS && wound->healerskill <= 0 && !number(0,19) && 
			str_cmp (wound->severity, "minor") && str_cmp (wound->severity, "small") ) {
			wound->infection = WOUND_INFECTIONS;
			snprintf (buf, MAX_STRING_LENGTH,  "Critical failure! Wound infected...\n");
		}
	}

	if ( wound->damage <= 0 ) {
		wound_from_char (ch, wound);
		return 1;
	}

	return 0;
}

void offline_healing (CHAR_DATA *ch, int since)
{

	WOUND_DATA		*wound, *next_wound;
	long double		healing_time = 0;
	int			checks = 0, i = 0, roll = 0;

	healing_time = time(0) - since;

	checks += ( healing_time / ((BASE_PC_HEALING-ch->con/6)*60));	/*  BASE_PC is in minutes, not seconds. */

	for ( wound = ch->wounds; wound; wound = next_wound ) {

		next_wound = wound->next;

		for ( i = 0; i < checks; i++ ) {
			if ( natural_healing_check(ch, wound) == 1 )
				break;
		}

	}

	for ( i = 0; i < checks; i++ ) {
		if (ch->damage) {
			roll = dice(1,100);
			if ( roll <= (ch->con * 6) ) {
				if ( roll % 5 == 0 )
					ch->damage -= 2;
				else
					ch->damage -= 1;
				ch->lastregen = time(0);
			}
		}
	}
}

void do_bind (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*victim;
	WOUND_DATA	*wound;
	int		bind_time = 0;
	char		buf [MAX_STRING_LENGTH];

	argument = one_argument (argument, buf);

	if ( IS_SET (ch->act, ACT_NOBIND) ) {
		send_to_char ("You can't bind!\n", ch);
		return;
	}

	if ( ch->delay && ch->delay_type == DEL_BIND_WOUNDS ) {
		send_to_char ("You're already binding someone - type STOP first.\n", ch);
		return;
	}

	if ( ch->right_hand && ch->left_hand ) {
		send_to_char ("You'll need a free hand.\n", ch);
		return;
	}

	if ( !*buf )
		victim = ch;

	else if ( !(victim = get_char_room_vis (ch, buf)) ) {
			send_to_char ("You can't see them here.\n\r", ch);
			return;
	}

	for ( wound = victim->wounds; wound; wound = wound->next )
		if ( wound->bleeding )
			bind_time += wound->bleeding;

	if ( !bind_time && victim != ch ) {
		act ("$N doesn't seem to need your assistance.", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	else if ( !bind_time && victim == ch ) {
		act ("You have no wounds in need of binding.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( victim != ch ) {
		act ("You crouch beside $N, carefully beginning to bind $S wounds.", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
		act ("$n crouches beside $N, carefully beginning to bind $S wounds.", FALSE, ch, 0, victim, TO_NOTVICT | TO_ACT_FORMAT);
		act ("$n crouches beside you, carefully beginning to bind your wounds.", FALSE, ch, 0, victim, TO_VICT | TO_ACT_FORMAT);
	}

	else if ( victim == ch ) {
		act ("You slowly begin administering aid to your wounds, attempting to stem the bleeding.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act ("$n slowly begins administering aid to $s wounds, attempting to stem the bleeding.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	}

	SET_BIT (ch->flags, FLAG_BINDING);

	ch->delay_type = DEL_BIND_WOUNDS;
	ch->delay_ch   = victim;
	ch->delay      = bind_time*number(1,2);

}

void delayed_bind (CHAR_DATA *ch)
{
	CHAR_DATA	*victim;
	WOUND_DATA	*wound;

	REMOVE_BIT (ch->flags, FLAG_BINDING);

	if ( !(victim = ch->delay_ch) ) {
		ch->delay_ch = NULL;
		ch->delay_type = 0;
		ch->delay = 0;
		return;
	}

	if ( !is_he_here (ch, victim, TRUE) ) {
		send_to_char ("Your patient is no longer here!\n", ch);
		ch->delay_ch = NULL;
		ch->delay_type = 0;
		ch->delay = 0;
		return;
	}

	if ( victim != ch ) {
		act ("You finish your ministrations; $N's wounds are bound.", FALSE, ch, 0, victim, TO_CHAR | TO_ACT_FORMAT);
		act ("$n finishes $s ministrations; $N's wounds are bound.", FALSE, ch, 0, victim, TO_NOTVICT | TO_ACT_FORMAT);
		act ("$n finishes $s ministrations; your wounds are bound.", FALSE, ch, 0, victim, TO_VICT | TO_ACT_FORMAT);
	}

	else if ( victim == ch ) {
		act ("You finish binding your wounds, and have managed to stem the bleeding.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act ("$n finishes binding $s wounds, and has managed to stem the bleeding.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	}

	for ( wound = victim->wounds; wound; wound = wound->next )
		if ( wound->bleeding )
			wound->bleeding = 0;

	ch->delay_ch = NULL;
	ch->delay_type = 0;
	ch->delay = 0;
}
