/** 
*	\file magic.c
*	Magic/Psionics Module
*
*	This module deals with tradional magic, psionics, dreams and certain
*	application affects.
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
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "structs.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"

int  spell_blend			(CHAR_DATA *ch, CHAR_DATA *victim, int sn);
int  spell_infravision		(CHAR_DATA *ch, CHAR_DATA *victim, int sn);

void magic_infravision		(CHAR_DATA *ch, CHAR_DATA *victim);
void magic_slipperiness		(CHAR_DATA *ch, CHAR_DATA *victim);
void magic_invulnerability	(CHAR_DATA *ch, CHAR_DATA *victim);

#define FRIEND_CHARM		1
#define FRIEND_GROUP		2
#define FRIEND_SELF			4
#define FRIEND_CHURCH		16

#define NUM_NIGHTMARES		0


extern char				*nightmare [];



char *show_charm (CHARM_DATA *charm)
{
	static char	buf [MAX_STRING_LENGTH] = {'\0'};

	*buf = '\0';

	if ( !str_cmp(charm->name, "Tindome Miriel") )
		snprintf (buf, MAX_STRING_LENGTH,  "#CA small sphere of translucent azure radiance hovers nearby.#0\n");
	
	return buf;
}
 

int is_direction (char *argument)
{
	int	i = 0;

	for ( i = 0; i <= 5; i++ ) {
		if ( !cmp_strn (dirs[i], argument, strlen(argument)) )
			return i;
	}

	return -1;
}


void update_target (CHAR_DATA *tch, int affect_type)
{
	if ( affect_type == MAGIC_AFFECT_SLEEP ) {
		do_sleep (tch, "", 0);
		if ( tch->fighting ) {
			if ( tch->fighting == tch )
				stop_fighting (tch->fighting);
			stop_fighting (tch);
		}
	}
}


void remove_charm (CHAR_DATA *ch, CHARM_DATA *ench)
{
	CHARM_DATA	*tmp;

	if ( !str_cmp (ench->name, "Tindome Miriel") ) {
		act ("The glimmering sphere hovering above your right shoulder flickers silently from existence.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act ("The glimmering sphere hovering above $n's right shoulder flickers silently from existence.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	}

	else if ( !str_cmp (ench->name, "Nole Isse") ) {
		act ("You feel your levels of perception return to normal, as heat's hidden lore fades from your grasp once more.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act ("The dim flicker of fae radiance in $N's eyes fades away.", FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	}

	if ( ch->charms && !str_cmp (ch->charms->name, ench->name) )
		ch->charms = NULL;
	else for ( tmp = ch->charms; tmp; tmp = tmp->next ) {
		if ( !str_cmp (tmp->next->name, ench->name) )
			tmp->next = tmp->next->next;
	}
}

void apply_charm (CHAR_DATA *caster, CHAR_DATA *victim, char *name, int duration, int source)
{
	CHARM_DATA	*tmp, *charm;

	CREATE (charm, CHARM_DATA, 1);

	charm->name = add_hash (name);
	charm->original_hours = duration;
	charm->current_hours = charm->original_hours;
	charm->power_source = source;
	charm->next = NULL;

	if ( !victim->charms ) {
		CREATE (victim->charms, CHARM_DATA, 1);
		victim->charms = charm;
	}
	else for ( tmp = victim->charms; tmp; tmp = tmp->next ) {
		if ( !str_cmp (tmp->name, charm->name) ) {	/* Recharge.*/
			tmp->current_hours = charm->current_hours;
			break;
		}
		if ( !tmp->next ) {
			CREATE (tmp->next, CHARM_DATA, 1);
			tmp->next = charm;
			break;
		}
	} 
}

int is_enchanted (CHAR_DATA *victim, char *charm_name)
{
	CHARM_DATA	*tmp;

	for ( tmp = victim->charms; tmp; tmp = tmp->next )
		if ( !str_cmp (tmp->name, charm_name) )
			return 1;

	return 0;
}

int armor_absorption (CHAR_DATA *target, char *location)
{
	OBJ_DATA	*obj;

	if ( !str_cmp (location, "skull") || !str_cmp (location, "reye") ||
		!str_cmp (location, "leye") || !str_cmp (location, "face") ) {
		if ( (obj = get_equip (target, WEAR_HEAD)) && GET_ITEM_TYPE (obj) == ITEM_ARMOR )
			return obj->o.armor.armor_value;
	}

	else if ( !str_cmp (location, "rshoulder") || !str_cmp (location, "lshoulder") ||
		!str_cmp (location, "rupperarm") || !str_cmp (location, "lupperarm") ||
		!str_cmp (location, "rforearm") || !str_cmp (location, "lforearm") ) {
		if ( (obj = get_equip (target, WEAR_ARMS)) && GET_ITEM_TYPE (obj) == ITEM_ARMOR )
			return obj->o.armor.armor_value;
	}

	else if ( !str_cmp (location, "rhand") || !str_cmp (location, "lhand") ) {
		if ( (obj = get_equip (target, WEAR_HANDS)) && GET_ITEM_TYPE (obj) == ITEM_ARMOR )
			return obj->o.armor.armor_value;
	}

	else if ( !str_cmp (location, "thorax") || !str_cmp (location, "abdomen") ) {
		if ( (obj = get_equip (target, WEAR_ABOUT)) && GET_ITEM_TYPE (obj) == ITEM_ARMOR )
			return obj->o.armor.armor_value;
	}

	else if ( !str_cmp (location, "hip") || !str_cmp (location, "groin") ||
		!str_cmp (location, "rthigh") || !str_cmp (location, "lthigh") ||
		!str_cmp (location, "rknee") || !str_cmp (location, "lknee") ||
		!str_cmp (location, "rcalf") || !str_cmp (location, "lcalf") ) {
		if ( ((obj = get_equip (target, WEAR_LEGS_2)) ||
				(obj = get_equip (target, WEAR_LEGS_1)))
				&& GET_ITEM_TYPE (obj) == ITEM_ARMOR )
			return obj->o.armor.armor_value;
	}

	else if ( !str_cmp (location, "rfoot") || !str_cmp (location, "lfoot") ) {
		if ( ((obj = get_equip (target, WEAR_FEET_2)) ||
				(obj = get_equip (target, WEAR_FEET_1)))
				&& GET_ITEM_TYPE (obj) == ITEM_ARMOR )
			return obj->o.armor.armor_value;
	}

	return 0;
}


void check_psionic_talents (CHAR_DATA *ch)
{
	ACCOUNT_DATA	*account;
	int		chance = 0, roll = 0;
	int		cur_talents = 0, i = 0, j = 1;
	int		talents [8] = { SKILL_CLAIRVOYANCE,
					SKILL_DANGER_SENSE,
					SKILL_EMPATHIC_HEAL,
					SKILL_HEX,
					SKILL_MENTAL_BOLT,
					SKILL_PRESCIENCE,
					SKILL_SENSITIVITY,
					SKILL_TELEPATHY };
	bool		check = TRUE, again = TRUE, awarded = FALSE, block = FALSE;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*date = NULL;

	if ( is_newbie (ch) || ch->aur <= 15 )
		return;

	if ( ch->pc && ch->pc->account ) {
		if ( !(account = load_account (ch->pc->account)) )
			return;
		if ( IS_SET (account->flags, ACCOUNT_NOPSI) )
			block = TRUE;
		if ( account->roleplay_points < 2 )
			block = TRUE;
		free_account (account);
	}
	else return;

	if ( block )
		return;

	if ( ch->aur < 16 )
		return;
	else if ( ch->aur == 16 )
		chance = 5;
	else if ( ch->aur == 17 )
		chance = 10;
	else if ( ch->aur == 18 )
		chance = 20;
	else if ( ch->aur == 19 )
		chance = 30;
	else if ( ch->aur == 20 )
		chance = 45;
	else if ( ch->aur == 21 )
		chance = 50;
	else if ( ch->aur == 22 )
		chance = 60;
	else if ( ch->aur == 23 )
		chance = 70;
	else if ( ch->aur == 24 )
		chance = 80;
	else
		chance = 95;

	chance += number(1,10);
	chance = MIN(chance, 95);

	for ( i = 0; i <= 7; i++ )
		if ( ch->skills[talents[i]] )
			cur_talents++;

	while ( check && cur_talents <= 4 ) {
		if ( number(1,100) <= chance ) {
			again = TRUE;
			while ( again ) {
				roll = talents[number(0,7)];
				if ( !ch->skills [roll] ) {
					ch->skills [roll] = 1;
					cur_talents++;
					again = FALSE;
					awarded = TRUE;
				}
				chance /= 2;
			}
			if ( cur_talents >= 4 )
				check = FALSE;
		}
		else check = FALSE;
	}

	if ( !awarded )
		return;

	snprintf (buf, MAX_STRING_LENGTH,  "This character rolled positive for the following talents:\n\n");

	for ( i = 0; i <= 7; i++ )
		if ( ch->skills [talents[i]] )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   %d. %s\n", j++, skill_data[talents[i]].skill_name); 

	date = timestr(date);

	add_message ("Psi_talents", 2, "Server", date, ch->tname, "", buf, 0);
	add_message (ch->tname, 3, "Server", date, "Psionic Talents.", "", buf, 0);

	mem_free (date);
}


void do_dreams (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	DREAM_DATA	*dream;
	int			count = 0;

	if ( IS_NPC (ch) ) {
		send_to_char ("This is a PC only command.\n\r", ch);
		return;
	}

	if ( !ch->pc->dreamed ) {
		send_to_char ("You don't recall anything memorable.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	for ( dream = ch->pc->dreamed; dream; dream = dream->next )
		count++;

	if ( !*buf ) {
		snprintf (buf, MAX_STRING_LENGTH,  "You recall having %d dream%s.\n\r",
					  count, count > 1 ? "s" : "");
		send_to_char (buf, ch);
		return;
	}

	if ( !isdigit (*buf) ) {
		send_to_char ("Either type 'dreams' or 'dreams <number>'.\n\r", ch);
		return;
	}

	if ( strtol(buf, NULL, 10) > count || strtol(buf, NULL, 10) < 1 ) {
		send_to_char ("That number doesn't correspond to a dream.\n\r", ch);
		return;
	}

	count = strtol(buf, NULL, 10) - 1;

	for ( dream = ch->pc->dreamed; count; count-- )
		dream = dream->next;

	page_string (ch->desc, dream->dream);
}

void scary_dream (CHAR_DATA *ch)
{
}

void dream (CHAR_DATA *ch)
{
	DREAM_DATA		*dream;
	DREAM_DATA		*dreamed;

	if ( !ch->pc || !ch->pc->dreams )
		return;

	if ( GET_POS (ch) != POSITION_SLEEPING )
		return;

	dream = ch->pc->dreams;

	ch->pc->dreams = dream->next;
	dream->next = NULL;

	if ( !ch->pc->dreamed )
		ch->pc->dreamed = dream;
	else {
		for ( dreamed = ch->pc->dreamed; dreamed->next; )
			 dreamed = dreamed->next;

		dreamed->next = dream;
	}

	send_to_char ("While asleep, you have a dream.\n\n\r", ch);

	page_string (ch->desc, dream->dream);

	save_char (ch, TRUE);
}

void post_dream (DESCRIPTOR_DATA *d)
{
	char		*p = NULL;
   	char        *date = NULL;
	DREAM_DATA	*dream;
	CHAR_DATA	*ch;

	p = d->character->delay_who;
	ch = d->character->delay_ch;

	if ( !p || !*p ) {
		send_to_char ("Dream aborted.\n\r", d->character);
		unload_pc (d->character->delay_ch);
		return;
	}

	CREATE (dream, DREAM_DATA, 1);

	dream->dream = d->character->delay_who;
	dream->next  = ch->pc->dreams;

	d->character->delay_who = NULL;

	ch->pc->dreams = dream;

	send_to_char ("Dream added.\n\r", d->character);

   	date = timestr(date);

	add_message (GET_NAME (ch),                          /* PC board */
				 3,                                     /* message # */
				 GET_NAME (d->character),                /* Imm name */
				 date,
                 		"Entry via GIVEDREAM command.",
                 		"",	
                 		dream->dream,
				MF_DREAM);

	unload_pc (ch);

	mem_free (date);
}

void do_givedream (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*who;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !(who = load_pc (buf)) ) {
		send_to_char ("No such PC.\n\r", ch);
		return;
	}

	if ( who->pc->create_state != 2 ) {	
		send_to_char ("PC is not in state 2.\n\r", ch);
		unload_pc (who);
		return;
	}

	make_quiet (ch);

	ch->delay_who = NULL;

	ch->delay_ch = who;
	ch->desc->str = &ch->delay_who;
	ch->desc->max_str = MAX_INPUT_LENGTH;
	ch->desc->proc = post_dream;
}



int friendly (CHAR_DATA *ch, CHAR_DATA *friend, int friendship)
{
	if ( IS_SET (friendship, FRIEND_SELF) )
		if ( friend == ch )
			return 1;

	if ( IS_SET (friendship, FRIEND_CHURCH) )
		if ( ch->deity && ch->deity == friend->deity )
			return 1;

	if ( friend->following != ch )
		return 0;

	if ( IS_SET (friendship, FRIEND_GROUP) && IS_AFFECTED (friend, AFF_GROUP) )
		return 1;

	return 0;
}

int resisted (CHAR_DATA *ch, CHAR_DATA *enemy, int spell)
{
	if ( (number (1,20) + (ch->circle - enemy->circle) + (ch->wil - enemy->wil) ) <= GET_WIL(enemy) )
		return 0;

	return 1;
}



void heal_wound (CHAR_DATA *ch, WOUND_DATA *wound, int damage)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	

	if ( (wound->damage - damage) <= 0 ) {
		temp_arg = expand_wound_loc(wound->location);
		snprintf (buf, MAX_STRING_LENGTH,  "A %s %s on your %s is healed!", wound->severity, wound->name, temp_arg);
		wound->damage -= damage;
		wound_from_char (ch, wound);
	}
	else {
		temp_arg = expand_wound_loc(wound->location);
		snprintf (buf, MAX_STRING_LENGTH,  "A %s %s on your %s mends rapidly.", wound->severity, wound->name, temp_arg);
		wound->damage -= damage;
	}

	act (buf, TRUE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
}

void magic_heal1 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(1,3));

}

void magic_heal2 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(2,3));

}

void magic_heal3 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(3,3));

}

void magic_heal4 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(4,3));

}

void magic_heal5 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(5,3));

}

void magic_heal6 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(6,3));

}

void magic_heal7 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(7,3));

}

void magic_heal8 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(8,3));

}

void magic_heal9 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(9,3));

}

void magic_heal10 (CHAR_DATA *ch)
{
	WOUND_DATA 	*wound;

	for ( wound = ch->wounds; wound; wound = wound->next )
		heal_wound (ch, wound, dice(10,3));

}

int apply_affect (CHAR_DATA *ch, int type, int duration, int power)
{
	AFFECTED_TYPE		*af;

	if ( (af = get_affect (ch, type)) ) {

		if ( af->a.spell.duration == -1 )
			return 1;

		if ( af->a.spell.duration < duration )
			af->a.spell.duration = duration; 

		if ( type >= MAGIC_SMELL_FIRST && type <= MAGIC_SMELL_LAST )
			af->a.smell.aroma_strength = power;

		return 1;
	}

	CREATE (af, AFFECTED_TYPE, 1);

    af->type              = type;
    af->a.spell.duration  = duration;
    af->a.spell.modifier  = power;
    af->a.spell.location  = 0;
    af->a.spell.bitvector = 0;
    af->a.spell.t         = 0;
    af->a.spell.sn        = 0;

    affect_to_char (ch, af);

	return 0;
}

int magic_add_affect (CHAR_DATA *ch, int type, int duration, int modifier,
					  int location, int bitvector, int sn)
{
	AFFECTED_TYPE	*af;

	if ( (af = get_affect (ch, type)) ) {

		if ( af->type > MAGIC_CLAN_MEMBER_BASE &&
			 af->type < MAGIC_CLAN_OMNI_BASE + MAX_CLANS )
			af->a.spell.t++;

		if ( af->a.spell.duration == -1 )	/* Perm already */
			return 0;

		if ( duration == -1 )
			af->a.spell.duration = duration;
		else
			af->a.spell.duration += duration;

		af->a.spell.modifier = MAX (af->a.spell.modifier, modifier);
		return 0;
	}

	CREATE (af, AFFECTED_TYPE, 1);
	
	af->type			  = type;
	af->a.spell.duration  = duration;
	af->a.spell.modifier  = modifier;
	af->a.spell.location  = location;
	af->a.spell.bitvector = bitvector;
	af->a.spell.t		  = 0;
	af->a.spell.sn		  = sn;

	affect_to_char (ch, af);

	return 1;
}

void job_add_affect (CHAR_DATA *ch, int type, int days, int pay_date, int cash,
					 int count, int object_vnum, int employer)
{
	AFFECTED_TYPE	*af;

	if ( (af = get_affect (ch, type)) )
		affect_remove (ch, af);

	af = (AFFECTED_TYPE *) alloc (sizeof (AFFECTED_TYPE), 13);

	af->type				= type;
	af->a.job.days			= days;
	af->a.job.pay_date		= pay_date;
	af->a.job.cash			= cash;
	af->a.job.count			= count;
	af->a.job.object_vnum	= object_vnum;
	af->a.job.employer		= employer;

	affect_to_char (ch, af);
}

int magic_add_obj_affect (OBJ_DATA *obj, int type, int duration, int modifier,
                                          int location, int bitvector, int sn)
{
	AFFECTED_TYPE   *af;

	if ( (af = get_obj_affect (obj, type)) ) {
		af->a.spell.duration += duration;
		af->a.spell.modifier = MAX (af->a.spell.modifier, modifier);
		return 0;
	}

	CREATE (af, AFFECTED_TYPE, 1);

	af->type	= type;
	af->a.spell.duration	= duration;
	af->a.spell.modifier	= modifier;
	af->a.spell.location	= location;
	af->a.spell.bitvector	= bitvector;
	af->a.spell.sn		= sn;

	affect_to_obj (obj, af);

	return 1;
}


void do_quaff (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*obj;

	argument = one_argument (argument, buf);

	if ( !(obj = get_obj_in_dark (ch, buf, ch->right_hand)) ) {
		act ("You can't find that.", FALSE, ch, 0, 0, TO_CHAR);
		return;
	}

	if ( obj->obj_flags.type_flag != ITEM_POTION ) {
		act ("$p isn't a potion.", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	ch->delay_type = DEL_QUAFF;
	ch->delay_info1 = (int) obj;
	
	if(ch->fighting) {
	    act("Fending off the blows, you somehow manage to open the vial.  At length, despite the pressure, you down its contents.",
		FALSE, ch, obj, 0, TO_CHAR);
	    ch->delay = 30;
	} else {
	    act("You open the vial, steeling yourself before taking the first swallow.",
		FALSE, ch, obj, 0, TO_CHAR);
	    ch->delay = 15;
	}
}

void delayed_quaff (CHAR_DATA *ch)
{
	OBJ_DATA	*obj;


	ch->delay = 0;

	obj = (OBJ_DATA *)ch->delay_info1;

	obj_from_char (&obj, 0);

	act ("You quaff $p.", TRUE, ch, obj, 0, TO_CHAR);
	act ("$n quaffs $p.", TRUE, ch, obj, 0, TO_ROOM);

	extract_obj (obj);
}

void apply_perfume (CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj)
{
	if ( obj->o.perfume.doses <= 0 ) {
		act ("$p has no more aroma.", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	obj->o.perfume.doses--;

	if ( ch != victim ) {
		act ("You apply the scent of $p onto $N.",
					FALSE, ch, obj, victim, TO_CHAR);
		act ("$N applies the scent of $p onto you.",
					FALSE, victim, obj, ch, TO_CHAR);
		act ("$n applies the scent of $p onto $N.",
					FALSE, ch, obj, victim, TO_NOTVICT);
	} else {
		act ("You apply the scent of $p to yourself.",
					FALSE, ch, obj, 0, TO_CHAR);
		act ("$n applies the scent of $p to $mself.",
					FALSE, ch, obj, 0, TO_ROOM);
	}

	apply_affect (victim, obj->o.perfume.type, obj->o.perfume.duration,
						  obj->o.perfume.aroma_strength);
}

void do_apply (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*obj;
	CHAR_DATA	*victim;

	argument = one_argument (argument, buf);

	if ( !(obj = get_obj_in_dark (ch, buf, ch->right_hand)) ) {
		send_to_char ("You don't have that.\n\r", ch);
		return;
	}

	if ( obj->obj_flags.type_flag != ITEM_SALVE &&
		 obj->obj_flags.type_flag != ITEM_PERFUME ) {
		act ("$p isn't a perfume or a salve.", FALSE, ch, obj, 0, TO_CHAR);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf )
		victim = ch;
	else {
		if ( !str_cmp (buf, "to") || !str_cmp (buf, "on") )
			argument = one_argument (argument, buf);

		if ( !(victim = get_char_vis (ch, buf)) ) {
			send_to_char ("You don't see them.\n\r", ch);
			return;
		}

		if ( victim->fighting ) {
			act ("You can't help $N while $E is fighting.",
					FALSE, ch, 0, victim, TO_CHAR);
			return;
		}

		if ( victim->delay ) {
			act ("$N is too busy at the moment.",
					FALSE, ch, 0, victim, TO_CHAR);
			return;
		}
	}

	if ( obj->obj_flags.type_flag == ITEM_PERFUME ) {
		apply_perfume (ch, victim, obj);
		return;
	}

	if ( victim != ch && GET_POS (victim) >= SIT ) {
		act ("$N must be laying down first.\n\r", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	obj_from_char (&obj, 0);

	act ("You gingerly apply $p on $N.", FALSE, ch, obj, victim, TO_CHAR);
	act ("$n applies $p to $N.", TRUE, ch, obj, victim, TO_ROOM);

	
	extract_obj (obj);
}


void do_see (CHAR_DATA *ch, char *argument, int cmd)
{
	int					save_room_num;
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA			*tch;

	if ( !*argument ) {
		send_to_char ("Who would you like to scry?\n", ch);
		return;
	}

	if ( !real_skill (ch, SKILL_CLAIRVOYANCE) ) {
		send_to_char ("See how?  You are not clairvoyant.\n\r", ch);
		return;
	}

	if ( GET_MOVE (ch) <= 40 ) {
		send_to_char ("You are too fatigued for that.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	tch = get_char_vis (ch, buf);

	if ( !tch || GET_TRUST (tch) ) {
		weaken (ch, 0, -10, "using SEE and failed");
		send_to_char ("Midway through your trance, you realize that you cannot "
					  "locate\n\r"
					  "that person.\n\r", ch);
		return;
	}
		
	if ( tch == ch ) {
		send_to_char ("You don't mean yourself, do you?\n\r", ch);
		return;
	}

	weaken (ch, 0, -40, "using SEE");

	if ( !skill_use (ch, SKILL_CLAIRVOYANCE, 0) ) {
		act ("You sense $N, but cannot see $M.", FALSE, ch, 0, tch, TO_CHAR);
		return;
	}

	if ( get_affect (tch, MAGIC_HIDDEN) || is_hooded (tch) ) {

		if ( get_affect (tch, MAGIC_HIDDEN) && tch->in_room == ch->in_room )
			act ("You sense that person is here somewhere!",
					FALSE, ch, 0, 0, TO_CHAR);

		else if ( is_hooded (tch) && tch->in_room == ch->in_room )
			act ("That person is someone here!",
					FALSE, ch, 0, 0, TO_CHAR);

		else
			act ("You sense $N, but cannot see $M.",
					FALSE, ch, 0, tch, TO_CHAR);
		return;
	}

	save_room_num = ch->in_room;

	char_from_room(ch);
	char_to_room(ch, tch->in_room);

	do_look (ch, "", 0);

	char_from_room (ch);
	char_to_room (ch, save_room_num);
}

void do_heal (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*tch;

	if ( !real_skill (ch, SKILL_EMPATHIC_HEAL) ) {
		send_to_char ("You have no idea how to do that.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that person here.\n\r", ch);
		return;
	}

	if ( tch == ch ) {
		send_to_char ("You would not benefit by psionically healing yourself.\n\r", ch);
		return;
	}

	if ( GET_TRUST (tch) ) {
		act ("$E is particularly undeserving of that.",
					FALSE, ch, 0, tch, TO_CHAR);
		act ("$N kindly tried to heal you.", FALSE, tch, 0, ch, TO_CHAR);
		return;
	}

	act ("You lay your hand on $N.", FALSE, ch, 0, tch, TO_CHAR);
	act ("$N gently lays $S hand upon you.", FALSE, tch, 0, ch, TO_CHAR);
	act ("$n gently touches $N.", FALSE, ch, 0, tch, TO_NOTVICT);
	sense_activity (ch, SKILL_EMPATHIC_HEAL);

	ch->delay_info1 = strtol(buf, NULL, 10);
	ch->delay_ch	= tch;
	ch->delay_type = DEL_EMPATHIC_HEAL;
	ch->delay		= 10;
}

void reset_heal_delay (CHAR_DATA *ch, CHAR_DATA *tch)
{
	ch->delay_ch	= tch;
	ch->delay_type = DEL_EMPATHIC_HEAL;
	ch->delay		= 10;
}

void delayed_heal (CHAR_DATA *ch)
{
	CHAR_DATA		*tch;
	WOUND_DATA		*wound;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	int				woundtype = 0;
	float			damage = 0;
	char			*temp_arg = NULL;
	
	tch = ch->delay_ch;

	if ( !tch->wounds ) {
		send_to_char ("There are no more wounds left to heal.\n", ch);
		return;
	}

	if ( !skill_use (ch, SKILL_EMPATHIC_HEAL, 0) ) {
		act ("You grimace in pain as the link between you suddenly dissipates.", FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
		act ("$n grimaces in pain as the link between you suddenly dissipates.", FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
		act ("$n grimaces in pain, slumping back with a sigh.", FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
		return;
	}

	if ( !is_he_here (ch, tch, 1) ) {

		if ( is_he_somewhere (tch) )
			act ("$N has left, you can no longer heal $M.",
					FALSE, ch, 0, tch, TO_CHAR);
		else
			act ("Your target has left.", FALSE, ch, 0, 0, TO_CHAR);

		return;
	}

	for ( wound = ch->wounds; wound; wound = wound->next )
		damage += wound->damage;

	damage += ch->damage;
	damage += tch->wounds->damage;

	if ( damage > ch->max_hit*.84 ) {
		act ("Grimacing in pain, you sever the link, unable to endure it any longer.", FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
		act ("$n grimaces in pain, and you feel the link between you severed.", FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
		act ("$n grimaces in pain, slumping back with a sigh.", FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
		return;
	}

	damage = tch->wounds->damage;

	if ( !str_cmp (tch->wounds->type, "slash") )
		woundtype = 2;
	else if ( !str_cmp (tch->wounds->type, "pierce") )
		woundtype = 1;
	else if ( !str_cmp (tch->wounds->type, "blunt") )
		woundtype = 3;
	else if ( !str_cmp (tch->wounds->type, "fire") )
		woundtype = 6;
	else if ( !str_cmp (tch->wounds->type, "frost") )
		woundtype = 5;
	else if ( !str_cmp (tch->wounds->type, "bite") )
		woundtype = 7;
	else if ( !str_cmp (tch->wounds->type, "claw") )
		woundtype = 8;
	else if ( !str_cmp (tch->wounds->type, "fist") )
		woundtype = 9;

	temp_arg = expand_wound_loc(tch->wounds->location);
	snprintf (buf, MAX_STRING_LENGTH,  "You grit your teeth in pain as a %s %s appears on your %s, and the"
                      " corresponding wound on $N fades quickly away.", tch->wounds->severity, tch->wounds->name, temp_arg);
	act (buf, FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
	
	temp_arg = expand_wound_loc(tch->wounds->location);
	snprintf (buf, MAX_STRING_LENGTH,  "$n gasps in pain as a %s %s suddenly appears on $s %s, and the"
                      " corresponding wound on you fades quickly away.", tch->wounds->severity, tch->wounds->name, temp_arg);
	act (buf, FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
	
	temp_arg = expand_wound_loc(tch->wounds->location);
	snprintf (buf, MAX_STRING_LENGTH,  "$n gasps in pain as a %s %s suddenly appears on $s %s, and the"
                      " corresponding wound on $N fades quickly away.", tch->wounds->severity, tch->wounds->name, temp_arg);
	act (buf, FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);

	wound_to_char (ch, tch->wounds->location, damage, woundtype, tch->wounds->bleeding, tch->wounds->poison, tch->wounds->infection);
	wound_from_char (tch, tch->wounds);

	reset_heal_delay (ch, tch);
}

void do_hex (CHAR_DATA *ch, char *argument, int cmd)
{
	int				duration, modifier;
	CHAR_DATA		*tch;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !real_skill (ch, SKILL_HEX) ) {
		send_to_char ("You shiver at the thought.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that person here.\n\r", ch);
		return;
	}

	if ( IS_MORTAL (ch) && !IS_MORTAL (tch) ) {
		send_to_char ("Immortals are total losers.  It can't get any worse for them.\n\r", ch);
		return;
	}

	if ( GET_HIT (ch) + GET_MOVE (ch) <= 35 ) {
		send_to_char ("You can't concentrate hard enough for that right now.\n\r", ch);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "Hexing %s", tch->tname);
	weaken (ch, 0, -20, buf);
	sense_activity (ch, SKILL_HEX);

	if ( !skill_use (ch, SKILL_HEX, 0) ) {
		send_to_char ("You lose your concentration, and your malignant energies dissipate.\n", ch);
		return;
	}

	act ("You channel a stream of malignant psychic energy into $N, entwining $M in an ethereal web of ill-fortune and grief.", FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
	act ("A chill runs down your spine.", FALSE, ch, 0, tch, TO_VICT);

	
	modifier = ch->skills [SKILL_HEX]/5 + number(1,10);
	duration = ch->skills [SKILL_HEX]/5 + number(1,48);

	magic_add_affect (tch, MAGIC_AFFECT_CURSE, duration, modifier, 0, 0, 0);

	tch->curse += ch->skills [SKILL_HEX]/5 + number(1,5);
}

void sense_activity (CHAR_DATA *user, int talent)
{
	CHAR_DATA	*ch;
	int 		modifier = 0;

	for ( ch = user->room->people; ch; ch = ch->next_in_room ) {
		if ( ch == user )
			continue;
		if ( !real_skill (ch, SKILL_SENSITIVITY) )
			continue;

		modifier += user->skills [talent] / 3;
		modifier -= ch->skills [SKILL_SENSITIVITY] / 5;

		if ( !skill_use (ch, SKILL_SENSITIVITY, modifier) )
			return;

		if ( talent == SKILL_TELEPATHY )
			act ("You sense $N gather $S energies and project a stream of thought into the aether.", FALSE, ch, 0, user, TO_CHAR | TO_ACT_FORMAT);
		else if ( talent == SKILL_SENSITIVITY )
			act ("You sense $N open $S psyche to the world around $M.", FALSE, ch, 0, user, TO_CHAR | TO_ACT_FORMAT);
		else if ( talent == SKILL_MENTAL_BOLT )
			act ("You sense a thrumming torrent of psychic energies gathering about $N.", FALSE, ch, 0, user, TO_CHAR | TO_ACT_FORMAT);
		else if ( talent == SKILL_EMPATHIC_HEAL )
			act ("You sense $N's psyche flow outward in a gentle wave.", FALSE, ch, 0, user, TO_CHAR | TO_ACT_FORMAT);
		else if ( talent == SKILL_HEX )
			act ("You sense $N's psychic energies quest outward in a malignant tendril.", FALSE, ch, 0, user, TO_CHAR | TO_ACT_FORMAT);
	}
}

void do_sense (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char *adjs [] = { "dim", "dull", "glimmering", "gleaming", "bright",
	  		  "radiant", "brilliant", "breathtakingly bright", "stunningly brilliant", "overwhelmingly radiant", "blinding" };
	int				aura;

	if ( !real_skill (ch, SKILL_SENSITIVITY) ) {
		send_to_char ("You strain, but sense nothing.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that person here.\n\r", ch);
		return;
	}

	if ( GET_HIT (ch) + GET_MOVE (ch) < 10 ) {
		send_to_char ("You don't feel strong enough at the moment.\n\r", ch);
		return;
	}

	weaken (ch, 0, -10, buf);
	sense_activity (ch, SKILL_SENSITIVITY);

	if ( !skill_use (ch, SKILL_SENSITIVITY, 0) ) {
		act ("You cannot muster a sense of $N's spiritual strength.", FALSE,
					ch, 0, tch, TO_CHAR);
		return;
	}

	aura = MIN (GET_AUR (tch), 22);

	if ( aura <= 9 )
		aura = 12;
	else if ( aura <= 13 )
		aura = 13;

	snprintf (buf, MAX_STRING_LENGTH,  "You sense that %s spirit is %s.\n",
				  ch == tch ? "your" : "$N's", adjs [aura - 12]);

	act (buf, FALSE, ch, 0, tch, TO_CHAR);
}

void do_bolt (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !real_skill (ch, SKILL_MENTAL_BOLT) ) {
		send_to_char ("You don't know how to do that.\n\r", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("You don't see that person here.\n\r", ch);
		return;
	}

	if ( ch == tch ) {
		send_to_char ("Would that be wise?\n\r", ch);
		return;
	}

	if ( GET_MOVE (ch) <= 40 ) {
		send_to_char ("You don't feel rested enough at the moment.\n\r", ch);
		return;
	}

	act ("You gather your psychic energies, preparing to unleash them upon $N.", FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
	sense_activity (ch, SKILL_MENTAL_BOLT);

	ch->delay_ch	= tch;
	ch->delay_type  = DEL_MENTAL_BOLT;
	ch->delay	= 15 - ch->skills[SKILL_MENTAL_BOLT]/10;
}

void delayed_bolt (CHAR_DATA *ch)
{
	CHAR_DATA	*tch;
	int			divisor = 1;
	int			roll;
	int			reflect = 0;
	int			modifier = 1;
	int			stun = 0;
	int			bolt_fatigue = 0;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	tch = ch->delay_ch;

	if ( !is_he_here (ch, tch, 1) ) {
		send_to_char ("Your victim isn't here anymore.\n\r", ch);
		return;
	}

	if ( !skill_use (ch, SKILL_MENTAL_BOLT, 0) ) {
		act ("You lose your concentration, and your energies dissipate ineffectually.", FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
		weaken (ch, 0, -15, 0);
		return;
	}

	bolt_fatigue = -40;
	bolt_fatigue += number(1,10);
	bolt_fatigue += ch->skills [SKILL_MENTAL_BOLT]/5;
	bolt_fatigue = MIN (-5, bolt_fatigue);

	weaken (ch, 0, bolt_fatigue, 0);

	if ( !str_cmp (tch->tname, IMPLEMENTOR_ACCOUNT) )
		reflect++;

	if ( (roll = number(1,100)) > ch->skills [SKILL_MENTAL_BOLT] ) {
		if ( roll % 5 == 0 )
			reflect++;
		else
			divisor = 2;
	}
	else {
		if ( roll % 5 == 0 )
			modifier = ch->skills [SKILL_MENTAL_BOLT]/10;
	}

	if ( !str_cmp (ch->tname, IMPLEMENTOR_ACCOUNT) )
		reflect = 0;

	if ( reflect ) {
		if ( (number(1,22) - (tch->wil - ch->wil)) <= tch->wil ) {
			act ("A wave of psychic energies slams into your mind, but you grit your teeth and manage to hold it at bay.", FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
			act ("You send your energies hurling into $N's mind, but they slam up against a psychic barrier.", FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
			act ("$N grits $S teeth, paling for a moment", FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
			return;
		}
		else {
			act ("A wave of psychic energy floods your mind, but with a great amount of effort, you manage to redirect it to its source.", FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
			act ("You send your energies hurling into $N's mind, but to your horror, they are redirected and come rushing forcefully back into your psyche!", FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
			act ("$n staggers backward with a surprised look on $s face.", FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);
			ch->stun += (number(1,ch->skills[SKILL_MENTAL_BOLT])/6) / divisor * modifier;
			return;
		}
	}

	act ("You send your energies hurling violently into $N's psyche, who staggers backward in shock.", FALSE, ch, 0, tch, TO_CHAR | TO_ACT_FORMAT);
	act ("A searing wave of psychic energy slams into your mind, sending you reeling.", FALSE, ch, 0, tch, TO_VICT | TO_ACT_FORMAT);
	act ("$N staggers backward, face contorted in shock.", FALSE, ch, 0, tch, TO_NOTVICT | TO_ACT_FORMAT);

	stun = (number(1,ch->skills[SKILL_MENTAL_BOLT]));
	stun /= divisor;
	stun *= modifier;
	tch->stun += stun;

/** debug statement ??? **/
	snprintf (buf, MAX_STRING_LENGTH,  "bolt_fatigue: %d divisor: %d modifier: %d stun: %d", bolt_fatigue, divisor, modifier, stun);
}

void do_prescience (CHAR_DATA *ch, char *argument, int cmd)
{
	if ( !real_skill (ch, SKILL_PRESCIENCE) ) {
		send_to_char ("That's not something you know how to do.\n\r", ch);
		return;
	}

	if ( IS_NPC (ch) ) {
		send_to_char ("This is a PC only command.\n\r", ch);
		return;
	}

	if ( GET_MOVE (ch) <= 50 ) {
		send_to_char ("You are too weary to muster the necessary concentration.\n\r", ch);
		return;
	}

	weaken (ch, 0, -50, "Prescience");

	if ( !skill_use (ch, SKILL_PRESCIENCE, 0) ) {
		act ("You begin spiralling downward into meditative contemplation, preparing an attempt to part the Veil of the future. At the last moment, however, a distracting thought races through your mind, and your concentration is ruined.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	send_to_char ("\n", ch);
	act ("#6You begin spiralling downward into meditative contemplation, preparing to part the Veil of the future, and the mysteries of the Valar. At long last your mind is focused and tranquil; you begin to ponder your question:#0", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

	ch->desc->str = &ch->delay_who;
	ch->desc->proc = post_prescience;
		ch->desc->max_str = MAX_INPUT_LENGTH;
}

void post_prescience (DESCRIPTOR_DATA *d)
{
	CHAR_DATA		*ch;
	char			*date = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	date = timestr(date);

	ch = d->character;

	snprintf (buf, MAX_STRING_LENGTH,  "#3%s:#0 %d Aura, %d Prescience.",
	GET_NAME(ch), GET_AUR (ch), ch->skills [SKILL_PRESCIENCE]);

	add_message ("Prescience", 2, GET_NAME (ch), date, buf, "", ch->delay_who, MF_DREAM);

	mem_free (date);

	ch->delay_who = NULL;
}

void do_reach (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*target;
	char		*p;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		msg_to_send [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;

			/* Skill use gets done by whipser_it.  whisper_it is not
			   called unless the target is online.
			*/

	if ( !real_skill (ch, SKILL_TELEPATHY) ) {
		send_to_char ("You have not been able to develop telepathic abilities "
					  "as of yet.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Reach whom with what message?\n", ch);
		return;
	}

	while ( *argument == ' ' )
		argument++;

	if ( !*argument ) {
		send_to_char ("What message did you wish to send?\n", ch);
		return;
	}

	if ( !(target = get_char_nomask (buf)) ) {
		send_to_char ("Who did you want to reach?\n", ch);
		return;
	}

	if ( target == ch ) {
		send_to_char ("That really isn't necessary, is it?\n", ch);
		return;
	}

	send_to_char ("You concentrate on your message, projecting it outward.\n", ch);
	sense_activity (ch, SKILL_TELEPATHY);

	whisper_it (ch, SKILL_TELEPATHY, argument, msg_to_send);

	if ( skill_use (target, SKILL_TELEPATHY, 0) ){
		temp_arg = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,  "A fleeting vision of #5%s#0 accompanies an ethereal whisper in your mind:", temp_arg);
		}
	else
		snprintf (buf, MAX_STRING_LENGTH,  "An ethereal voice touches your mind:");

	act (buf, FALSE, target, 0, 0, TO_CHAR | TO_ACT_FORMAT);

	reformat_say_string (msg_to_send, &p);
	snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"\n", p);
	send_to_char (buf, target);

	weaken (ch, 0, -10, "Reach (succeeded)");
}
