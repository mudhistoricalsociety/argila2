/** 
*	\file money.c
*	Provides services for creating, spending, counting, and describing money
*
*	Description:  All matters dealing with coins, money and merchant actions
*
*	The purpose of this module is to provide services for creating, spending,
*	counting, and describing money, as well as receiving change, and putting
*	money in storerooms. Also actions involving merchants and vnpc buyers,
*	including sales, barter and re-sales.
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

/** Contains the ovnums for all currency items accepted by currency_type = 1 **/
extern const int coin_type1[];

/** Contains the ovnums for all currency items accepted by currency_type = 0 **/
extern const int coin_type0[];

/** money avaible to vnpc **/
#define VNPC_COPPER_PURSE	200


#define NO_SUCH_ITEM1 "Don't seem to have that in my inventory. Would you like to buy something else?"
#define NO_SUCH_ITEM2 "I don't see that particular item. Perhaps you misplaced it?"
#define MISSING_CASH1 "A little too expensive for me now -- why don't you try back later?"
#define MISSING_CASH2 "You're a little short on coin, I see; come back when you can afford it."
#define DO_NOT_BUY    "I don't buy those sorts of things, I'm afraid."


/**
 *  Function: do_count
 *
 *  Prepares to counts the number of currency items.
 *
 *
 *      \param ch The PC doing the counting.
 *      \param argument none.
 *      \param cmd none
 *
 *
 *		delay_type is set to DEL_COUNT_COIN. Actual counting takes place
 *		at the next time tick. 
 *
 *
 */
void do_count (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*obj = NULL;

	argument = one_argument (argument, buf);

	if ( is_dark (ch->room) && !get_affect(ch, MAGIC_AFFECT_INFRAVISION) && IS_MORTAL (ch) && !IS_SET (ch->affected_by, AFF_INFRAVIS)) {
		send_to_char ("It's too dark to count anything.\n", ch);
		return;
	}

	if ( !*buf ) {
		snprintf (buf, MAX_STRING_LENGTH,   "You begin searching through your belongings, taking a tally of your wealth.\n");
		send_to_char (buf, ch);
	}
	else {
		if ( !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
			!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) ) {
			send_to_char ("I don't see that group of items.\n", ch);
			return;
		}

		if (!IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) {
			send_to_char ("That isn't money.\n", ch);
			return;
		}

		snprintf (buf, MAX_STRING_LENGTH,   "After a moment of sorting, you determine that there are %d items in the pile.", obj->count);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

    	ch->delay      = 10;

	ch->delay_type = DEL_COUNT_COIN;
	
	return;
}

/**
 *  Function: delayed_count_coin
 *
 *  Counts the number of currency items and adds up thier value.
 *
 *
 *      \param ch The PC doing the counting.
 *
 *
 *		A string is printed to the character with the count of currency
 *		items and thier value
 *
 *
 */
void delayed_count_coin (CHAR_DATA *ch)
{
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		buf2 [MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*tobj = NULL;
	int			money = 0;
	int			location = 0;
	char		*temp_arg = NULL;
	
	if ( ch->right_hand ) {
		if (IS_SET (ch->right_hand->obj_flags.extra_flags, ITEM_CAPITAL)) {
			money += ch->right_hand->coppers * ch->right_hand->count;
			tobj = ch->right_hand;
			snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH, "   %d of %s (right hand): %d coppers\n", tobj->count, tobj->short_description, (int)tobj->coppers*tobj->count);
		}
		else if ( (int)GET_ITEM_TYPE (ch->right_hand) == ITEM_CONTAINER ) {
			for ( tobj = ch->right_hand->contains; tobj; tobj = tobj->next_content ) {
				if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL)) {
					money += tobj->coppers*tobj->count;
					
					temp_arg = obj_short_desc(tobj->in_obj);
					snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH, "   %d of %s (%s): %d coppers\n", tobj->count, tobj->short_description, temp_arg, (int)tobj->coppers*tobj->count);
				}
			}
		}
	}

	if ( ch->left_hand ) {
		if (IS_SET (ch->left_hand->obj_flags.extra_flags, ITEM_CAPITAL)) {
			money += ch->left_hand->coppers * ch->left_hand->count;
			tobj = ch->left_hand;
			snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH, "   %d %s (left hand): %d coppers\n", tobj->count, tobj->short_description, (int)tobj->coppers*tobj->count);
		}
		else if ( GET_ITEM_TYPE (ch->left_hand) == ITEM_CONTAINER ) {
			for ( tobj = ch->left_hand->contains; tobj; tobj = tobj->next_content ) {
				if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL)) {
					money += tobj->coppers*tobj->count;
					
					temp_arg = obj_short_desc(tobj->in_obj);
					snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH, "   %d of %s (%s): %d coppers\n", tobj->count, tobj->short_description, temp_arg, (int)tobj->coppers*tobj->count);
				}
			}
		}
	}

	for ( location = 0; location < MAX_WEAR; location++ ) {
		if ( !(obj = get_equip (ch, location)) )
			continue;
		for ( tobj = obj->contains; tobj; tobj = tobj->next_content ) {
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL)) {
				money += tobj->coppers*tobj->count;
				
				temp_arg = obj_short_desc(obj);
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH, "   %d of %s (%s): %d coppers\n", tobj->count, tobj->short_description, temp_arg, (int)tobj->coppers*tobj->count);
			}
		}
	}

	if ( !money ) {
		send_to_char ("You don't seem to have any coin.\n", ch);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,   "By your count, you have %d coppers' worth in coin:", money);
	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	send_to_char ("\n", ch);
	send_to_char (buf2, ch);
	
	return;
}

/**
 *  Function: stop_counting
 *
 *  Halts the counting of currency items
 *
 *
 *      \param ch The PC doing the counting.
 *
 *
 *     The delay is set to 0, and an echo is printed to character
 *
 *
 */
void stop_counting (CHAR_DATA *ch)
{
	send_to_char ("You bore of counting coins.\n", ch);

	ch->delay = 0;

	ch->delay_obj = NULL;
	
	return;
}

/**
 *  Function: make_change
 *
 *  Calculates the proper change and puts it where it belongs
 *
 *
 *      \param money amount of chnage to be made
 *		\param keeper shopkeep (if there is one)
 *		\param currency_type type of currency
 *		\param tobj a container object to put change into
 *		\param cmd	0 if the change is given to the character
 *		\param cmd	1 if the change goes to the keepers room
 *		\param ch character getting the change
 *
 *     Returns TRUE if change is made, FALSE otherwise.
 *
 *  	\sa keeper_money_to_char, subtract_money, money_to_storeroom,
 *		subtract_keeper_money
 *
 */
int make_change(int money, CHAR_DATA *keeper, int currency_type, OBJ_DATA *tobj, int cmd, CHAR_DATA *ch)
{
	OBJ_DATA	*obj = NULL;
	bool		coin_back = FALSE;
	int 		i = 0;
	int 		j = 0;
	int 		value = 0;
	int 		holder = 0;	
	int 		sorted_values[MAX_COIN];
	
	if (keeper){ /* we have a merchant */
		if (keeper->mob->currency_type == 0){
			for ( i = 0; i < MAX_COIN; i++){
				sorted_values[i] = coin_type0[i];
			}
		}
	
		if (keeper->mob->currency_type == 1) {
			for ( i = 0; i < MAX_COIN; i++){
				sorted_values[i] = coin_type1[i];
			}
		}
	}
	else { /* there is no merchant */
		if (currency_type == 0){
			for ( i = 0; i < MAX_COIN; i++){
				sorted_values[i] = coin_type0[i];
			}
		}
	
		if (currency_type == 1) {
			for ( i = 0; i < MAX_COIN; i++){
				sorted_values[i] = coin_type1[i];
			}
		}
	}
/****** sort array(vnum,value) on the second index *****/

	for (i = 0; i < MAX_COIN; i++){
		for (j = 0; j < MAX_COIN-1; j ++){
			if (!(sorted_values[j+1]) == 0){
				value = (vtoo(sorted_values[j]))->coppers; /* value of the coin */
	   			if (value > (vtoo(sorted_values[j+1]))->coppers){
	   				holder = sorted_values[j+1];
	   				sorted_values[j+1] = sorted_values[j];
	   				sorted_values[j] = holder;
				}
			}
    	}
    } /* end for */
 
 	
/******* end sort ********************/

	if (cmd == 0){ /* change given to the character */
		for (i = MAX_COIN-1; i >= 0; i--){
 			if ( !(sorted_values[i] == 0)) { 
        		obj = load_object(sorted_values [i]);
        		if (vtoo(sorted_values[i])->coppers > 0){
        			if (money/(int)(vtoo(sorted_values[i])->coppers) > 0){
	        			obj->count = money/(int)(vtoo(sorted_values [i])->coppers);
   						money = money % (int)(vtoo(sorted_values [i])->coppers);

						if (tobj){
							obj_to_obj (obj, tobj);
						}
						else {
							obj_to_char (obj, ch);
						}
						coin_back = TRUE;
					}
				}
			}
		}
		return (coin_back);
	}

	if (cmd == 1){ /* change sent to a room */
		for (i = MAX_COIN-1; i >= 0; i--){
 			if ( !(sorted_values[i] == 0)) { 
				obj = load_object(sorted_values[i]);
				if (vtoo(sorted_values[i])->coppers > 0){
					if (money/(int)(vtoo(sorted_values[i])->coppers) > 0){
		    			obj->count = money/(int)(vtoo(sorted_values[i])->coppers);
		    			money = money % (int)(vtoo(sorted_values[i])->coppers);
						obj_to_room (obj, keeper->shop->store_vnum);
		
						coin_back = TRUE;
					}
				}
			}
		}
		return (coin_back);
	}
	
	return (coin_back);
}
/**
 *  Function: keeper_has_money
 *
 *  Checks to determine if a shopkeeper has enough currency in his storeroom to
 *	cover a debit request.
 *
 *
 *      \param keeper The shopkeeper.
 *      \param cost amount to compare currency total to
 *
 *
 *     Returns 0 if currency available is less than the request, and 
 *	   returns 1 if there is not enough currency to cover the request
 *
 *
 */

int keeper_has_money (CHAR_DATA *keeper, int cost)
{
	ROOM_DATA	*store = NULL;
	OBJ_DATA	*obj = NULL;
	int		money = 0;

	if ( !keeper->shop )
		return 0;
	if ( !keeper->shop->store_vnum )
		return 0;
	if ( !(store = vtor(keeper->shop->store_vnum)) )
		return 0;

	if ( store->contents &&
		GET_ITEM_TYPE (store->contents) == ITEM_MONEY && 
		keeper_uses_currency_type (keeper->mob->currency_type, store->contents) ) {
		
		money = store->contents->coppers*store->contents->count;
	}
	
	for ( obj = store->contents; obj; obj = obj->next_content ) {
		if ( obj->next_content &&
			GET_ITEM_TYPE(obj->next_content) == ITEM_MONEY &&
			keeper_uses_currency_type (keeper->mob->currency_type, obj->next_content) ) {
			money += obj->next_content->coppers*obj->next_content->count;
		}
	}

	if ( money < cost )
		return 0;
	else return 1;
}


/**
 *  Function: keeper_uses_currency_type
 *
 *  Determines if object is in the list of vnums for the given currency_type.
 *
 *		\param currency_type the type of money used by the shopkeeper
 *		\param obj the object that may or may not be a currency unit
 *
 *     Returns 1 if the object is a currency unit that keeper uses, 
 *     returns a 0 if the object is NOT a currency unit.
 *
 *
 */
int keeper_uses_currency_type (int currency_type, OBJ_DATA *obj)
{

	int 	index = 0;
	
	if ( currency_type == 0 ) {
		while (coin_type0 [index]){
			if ((obj->virtual) == coin_type0 [index])
				return 1;
			index ++;
			
		}
	}
	else if (currency_type == 1){
		while (coin_type1 [index]){
			if ((obj->virtual) == coin_type1 [index])
				return 1;
			index ++;
			
		}
	}
	/*** item is not a recognized currency item **/
	return 0; 
}

/**
 *  Function: keeper_money_to_char
 *
 *  Gives money to character.
 *
 *		\param keeper the shopkeeper giving away the money
 *		\param ch the character getting the money
 *		\param money the value (in coppers) to be paid
 *
 *
 *     	Prints an echo to the character and calls make_change
 *
 *      
 *
 */

void keeper_money_to_char (CHAR_DATA *keeper, CHAR_DATA *ch, int money)
{
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*tobj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			location = 0;
	char		*temp_arg = NULL;

	
	if ( ch->right_hand ) {
		if ((int)GET_ITEM_TYPE (ch->right_hand) == ITEM_CONTAINER ) {
			tobj = ch->right_hand;
		}
	}

	if ( ch->left_hand ) {
		if ( GET_ITEM_TYPE (ch->left_hand) == ITEM_CONTAINER ) {
			tobj = ch->left_hand;
		}
	}
	
	for ( location = 0; location < MAX_WEAR; location++ ) {
		if ( !(obj = get_equip (ch, location)) )
			continue;
		
		if ( GET_ITEM_TYPE (obj) != ITEM_CONTAINER )
			continue;
        
        if ( IS_SET(obj->o.container.flags, CONT_CLOSED) )
			continue;
		tobj = obj;		
	}
	
	if (tobj){
		temp_arg = obj_short_desc(tobj);
		snprintf (buf, MAX_STRING_LENGTH,   "$N gives you some coins in return, which you tuck away in %s.", temp_arg);
		(void)make_change(money, keeper, 0, tobj, 0, ch);
	}
	else {
		snprintf (buf, MAX_STRING_LENGTH,   "$N gives you some coins in return.");
		(void)make_change(money, keeper, 0, '\0', 0, ch);
	}
	
		
	act (buf, TRUE, ch, 0, keeper, TO_CHAR | TO_ACT_FORMAT);

	return;
}

/**
 *  Function: can_subtract_money
 *
 *  Checks to see if the character has enough money on him or in his gear.
 *
 *		\param ch character who is losing the money
 *		\param coppers_to_subtract amount to be removed
 *		\param currency_type type of currency items to be removed
 *     
 *
 *  	Returns a 0 if character doesn't have enough money, and 
 *		returns a 1 is there is enough money.
 *
 *
 */
int can_subtract_money (CHAR_DATA *ch, int coppers_to_subtract, int currency_type)
{
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*tobj = NULL;
	int			money = 0;
	int 		location = 0;
	int			index = 0;
	
	if ((obj = ch->right_hand) && 
		 (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&
		 (currency_type == 0)) {
		 while (coin_type0 [index]){
			if ((obj->virtual) == coin_type0 [index]){
				money += ch->right_hand->coppers * ch->right_hand->count;
			}
			index ++;
		}
	}

	
	if ( (obj = ch->right_hand) && 
		 (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&			
		 (currency_type == 1)) {
		index = 0;
		while (coin_type1 [index]){
			if ((obj->virtual) == coin_type1 [index]){
				money += ch->right_hand->coppers * ch->right_hand->count;
			}
			index ++;
		}
	}
	
	if ( (obj = ch->right_hand) && 	
		 ( GET_ITEM_TYPE (obj) == ITEM_CONTAINER )) { 
		for ( tobj = obj->contains; tobj; tobj = tobj->next_content ) {
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 0)) {
				index = 0;
				while (coin_type0 [index]){
					if ((tobj->virtual) == coin_type0 [index]){
						money += tobj->coppers * tobj->count;
					}
					index ++;
				}
			}
		
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 1)){
				index = 0;
				while (coin_type1 [index]){
					if ((tobj->virtual) == coin_type1 [index]){
						money += tobj->coppers * tobj->count;
					}
					index ++;
				}
			}
		}
	}
	
	if ( (obj = ch->left_hand) &&
		 (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&
		 (currency_type == 0)) {
		index = 0;
		while (coin_type0 [index]){
			if ((obj->virtual) == coin_type0 [index]){
				money += ch->left_hand->coppers * ch->left_hand->count;
			}
			index ++;
		}
	}
	
	if ( (obj = ch->left_hand) &&
		 (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&
		 (currency_type == 1)){
		index = 0;
		while (coin_type1 [index]){
			if ((obj->virtual) == coin_type1 [index]){
				money += ch->left_hand->coppers * ch->left_hand->count;
			}
			index ++;
		}
	}
		
		
	if ( (obj = ch->left_hand) &&
		 ( GET_ITEM_TYPE (obj) == ITEM_CONTAINER )) { 
		for ( tobj = obj->contains; tobj; tobj = tobj->next_content ) {
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 0)) {
				index = 0;
				while (coin_type0 [index]){
					if ((tobj->virtual) == coin_type0 [index]){
						money += tobj->coppers * tobj->count;
					}
					index ++;
				}
			}
					
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 1)){
				index = 0;
				while (coin_type1 [index]){
					if ((tobj->virtual) == coin_type1 [index]){
						money += tobj->coppers * tobj->count;
					}
					index ++;
				}
			}
		}
	}
		
	
	for ( location = 0; location < MAX_WEAR; location++ ) {
		if ( !(obj = get_equip (ch, location)) )
			continue;
		
		if ( GET_ITEM_TYPE (obj) != ITEM_CONTAINER )
			continue;
        
        if ( IS_SET(obj->o.container.flags, CONT_CLOSED) )
			continue;
		
		for ( tobj = obj->contains; tobj; tobj = tobj->next_content ) {
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 0)) {
				index = 0;
				while (coin_type0 [index]){
					if ((tobj->virtual) == coin_type0 [index]){
						money += tobj->coppers * tobj->count;
					}
					index ++;
				}
			}
			
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 1)){
				index = 0;
				while (coin_type1 [index]){
					if ((tobj->virtual) == coin_type1 [index]){
						money += tobj->coppers * tobj->count;
					}
					index ++;
				}
			}
		}
	}
		
	if ( money < coppers_to_subtract )
		return 0;

	return 1;
}
/**
 *  Function: subtract_money
 *
 *	Finds money on the character, then removes the amount needed  
 *
 *		\param ch the character paying the money
 *		\param coppers_to_subtract the amount to be removed
 *		\param currency_type the type of currency to look for
 *
 *
 *		Removes the money from the character, and prints an echo to the player
 *		and the room
 *
 *
 */

void subtract_money (CHAR_DATA *ch, int coppers_to_subtract, int currency_type)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	*tobj = NULL;
	OBJ_DATA 	*obj = NULL;
	OBJ_DATA	*next_obj = NULL;
	bool		coin_back = FALSE;
	bool		container = FALSE;
	int			money = 0;
	int			location = 0;
	int			ind = 0;
	char		*temp_arg = NULL;

	ind = 0;
	for ( location = 0; location < MAX_WEAR; location++ ) {
		if ( !(obj = get_equip (ch, location)) )
			continue;
			
		if ( GET_ITEM_TYPE (obj) != ITEM_CONTAINER )
			continue;
            
        if ( IS_SET(obj->o.container.flags, CONT_CLOSED) )
			continue;
		
/*** obj is an open container on the character at this point ***/
/*** tobj is something inside the container ***/

		for ( tobj = obj->contains; tobj; tobj = next_obj ) {
			next_obj = tobj->next_content;
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 0)){ 
				ind = 0;
				while (coin_type0 [ind]){
					if ((tobj->virtual) == coin_type0 [ind]){
						money += tobj->coppers*tobj->count;
						obj_from_obj(&tobj, tobj->count);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = TRUE;
					}
					ind ++;
				}
			}
			
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 1)){ 
				ind = 0;
				while (coin_type1 [ind]){
					if ((tobj->virtual) == coin_type1 [ind]){
						money += tobj->coppers*tobj->count;
						obj_from_obj(&tobj, tobj->count);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = TRUE;
					}
					ind ++;
				}
			}
		}
	}  
/*** end wear location check **/

	if ((obj = ch->right_hand) && 
		 (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&
		 (currency_type == 0)) {
		ind = 0;
		while (coin_type0 [ind]){
			if ((obj->virtual) == coin_type0 [ind]){
				money += obj->coppers * obj->count;
				extract_obj (obj);
				ch->right_hand = NULL;
			}
			ind ++;
		}
	}
	
	if ( (obj = ch->right_hand) && 
		 (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&			
		 (currency_type == 1)) {
		ind = 0;
		while (coin_type1 [ind]){
			if ((obj->virtual) == coin_type1 [ind]){
				money += obj->coppers * obj->count;
				extract_obj (obj);
				ch->right_hand = NULL;
			}
			ind ++;
		}
	}
	
	
	if ( (obj = ch->right_hand) && 	
		 ( GET_ITEM_TYPE (obj) == ITEM_CONTAINER )) { 
		for ( tobj = obj->contains; tobj; tobj = tobj->next_content ) {
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 0)) {
				ind = 0;
				while (coin_type0 [ind]){
					if ((tobj->virtual) == coin_type0 [ind]){
						money += tobj->coppers*tobj->count;
						obj_from_obj(&tobj, tobj->count);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = TRUE;
					}
					ind ++;
				}
			}
		
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 1)){
				ind = 0;
				while (coin_type1 [ind]){
					if ((tobj->virtual) == coin_type1 [ind]){
						money += tobj->coppers*tobj->count;
						obj_from_obj(&tobj, tobj->count);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = TRUE;
					}
					ind ++;
				}
			}
		}
	} 
/*** end right hand check **/

	if ( (obj = ch->left_hand) &&
		 (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&
		 (currency_type == 0)) {
		ind = 0;
		while (coin_type0 [ind]){
			if ((obj->virtual) == coin_type0 [ind]){
				money += obj->coppers * obj->count;
				extract_obj (obj);
				ch->left_hand = NULL;
			}
			ind ++;
		}
	}
	
	if ( (obj = ch->left_hand) &&
		 (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&
		 (currency_type == 1)){
		ind = 0;
		while (coin_type1 [ind]){
			if ((obj->virtual) == coin_type1 [ind]){
				money += obj->coppers * obj->count;
				extract_obj (obj);
				ch->left_hand = NULL;
			}
			ind ++;
		}
	}
		
		
	if ( (obj = ch->left_hand) &&
		 ( GET_ITEM_TYPE (obj) == ITEM_CONTAINER )) { 
		for ( tobj = obj->contains; tobj; tobj = tobj->next_content ) {
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 0)) {
				ind = 0;
				while (coin_type0 [ind]){
					if ((tobj->virtual) == coin_type0 [ind]){
						money += tobj->coppers*tobj->count;
						obj_from_obj(&tobj, tobj->count);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = TRUE;
					}
					ind ++;
				}
			}	
					
			if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL) &&
				(currency_type == 1)){
				ind = 0;
				while (coin_type1 [ind]){
					if ((tobj->virtual) == coin_type1 [ind]){
						money += tobj->coppers*tobj->count;
						obj_from_obj(&tobj, tobj->count);
						extract_obj (tobj);
						ch->delay_obj = obj;
						container = TRUE;
					}
					ind ++;
				}
			}
		}
	}
/** end left hand **/

	money -= coppers_to_subtract;

	coin_back = make_change(money, '\0', currency_type, ch->delay_obj, 0, ch);

	obj = ch->delay_obj;
	ch->delay_obj = NULL;

	if ( container )
		send_to_char ("\nRifling through your belongings, you retrieve your coin.\n", ch);
	else send_to_char ("\nYou offer up the specified amount.\n", ch);

	if ( coin_back ) {
		if ( obj ){
			temp_arg = obj_short_desc(obj);
			snprintf (buf, MAX_STRING_LENGTH,   "Change is made, which you then deposit in %s.", temp_arg);
		}
  		else
			snprintf (buf, MAX_STRING_LENGTH,   "Change is made for the amount offered.");
		send_to_char ("\n", ch);
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	}

	ch->delay_obj = NULL;
	
	return;
}

/**
 *  Function: do_list
 *
 *  Lists all sellable items in a keepers storeroom
 *
 *		\param	ch - The character asking for the list
 *		\param	argument - type of item and specific shopkeeper
 *		\param	cmd - not used 
 *
 *		Useage, 
 *		- <b>list tunic</b> will  list all tunics
 *		- <b>list copper</b> will list the items for the keeper with copper hair
 *		- <b>list tunic copper</b> will list all tunics from copper-haired man
 *
 *  	Prints a list of available items. Prevents players from buying back
 *		items they've sold, and prevents all others from buying a sold item for
 *		15 minutes to prevent abuse.
 *
 *
 */

void do_list (CHAR_DATA *ch, char *argument, int cmd)
{
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				stock_buf [MAX_STRING_LENGTH] = {'\0'};
	char				buf2 [MAX_STRING_LENGTH] = {'\0'};
	char				output [MAX_STRING_LENGTH] = {'\0'};
	int					i = 0;
	float				val_in_coppers = 0;
	int					header_said = 0;
	float				markup = 0;
	CHAR_DATA			*keeper = NULL;
	CHAR_DATA			*tch = NULL;
	ROOM_DATA			*room = NULL;
	ROOM_DATA			*store = NULL;
	OBJ_DATA			*obj = NULL;
	OBJ_DATA			*tobj = NULL;
	NEGOTIATION_DATA	*neg = NULL;
	char				*temp_arg = NULL;

	room = ch->room;

	argument = one_argument (argument, buf);

	if ( *buf && (keeper = get_char_room_vis (ch, buf)) )
		argument = one_argument (argument, buf);

	else {
		for ( tch = room->people; tch; tch = tch->next_in_room )
			if ( tch != ch && IS_SET (tch->flags, FLAG_KEEPER) )
				break;

		keeper = tch;
	}

	if ( !keeper ) {
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if ( !GET_TRUST (ch) && !CAN_SEE (keeper, ch) ) {
		do_say (keeper, "Who's there?", 0);
		return;
	}

	if ( GET_POS (keeper) <= POSITION_SLEEPING ) {
		act ("$N is not conscious.", TRUE, ch, 0, keeper, TO_CHAR);
		return;
	}

	if ( GET_POS (keeper) == POSITION_FIGHTING ) {
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if ( !keeper->shop || !IS_SET (keeper->flags, FLAG_KEEPER) ) {

		if ( keeper == ch )
			send_to_char ("You don't have a shop.\n", ch);
		else
			act ("$N isn't a shopkeeper.", FALSE, ch, 0, keeper, TO_CHAR);

		return;
	}

	if ( keeper->shop->shop_vnum &&
		 keeper->shop->shop_vnum != ch->in_room ) {
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if ( !(store = vtor (keeper->shop->store_vnum)) ) {
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
				0);
		return;
	}

	if ( !store->psave_loaded )
		load_save_room (store);

	if ( !store->contents ) {
		do_say (keeper, "I have nothing for sale at the moment.", 0);
		return;
	}

	i = 0;

	*output = '\0';

	for ( obj = store->contents; obj; obj = obj->next_content ) {

		i++;

		if ( (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) && keeper_uses_currency_type (keeper->mob->currency_type, obj) ) {
			i--;
			if ( obj->next_content ) {
				continue;
			} else break;
		}

		if ( *buf && !name_is (buf, obj->name) )
			continue;

		if ( !CAN_WEAR (obj, ITEM_TAKE) )
			continue;

/*** Prevent players from buying back items they've sold, and prevent all others from buying a sold item for 15 minutes to prevent abuse ***/

		if ( (obj->sold_by != ch->coldload_id && (time(0) - obj->sold_at <= 60 * 15)) ||
			(obj->sold_by == ch->coldload_id && (time(0) - obj->sold_at <= 60 * 60)) ) 
			continue;

		if ( !header_said ) {
			act ("$N describes $S inventory:", FALSE, ch, 0, keeper, TO_CHAR);
			send_to_char ("\n   #   retail cost    item\n", ch);
			send_to_char ("  ===  ===========    ====\n", ch);
			header_said = 1;
		}

		if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) )
			markup = econ_markup (keeper, obj);
		else markup = econ_discount (keeper, obj);

		val_in_coppers = ( obj->coppers * markup );

		if ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON &&
			 obj->o.drinkcon.volume > 0 &&
			 (tobj = vtoo (obj->o.drinkcon.liquid)) &&
			 GET_ITEM_TYPE (tobj) == ITEM_FLUID ) {
			if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) )
				markup = econ_markup (keeper, tobj);
			else markup = econ_discount (keeper, tobj);
			val_in_coppers += (tobj->coppers * obj->o.drinkcon.volume * markup);
		}

		for ( neg = keeper->shop->negotiations; neg; neg = neg->next ) {
			if ( neg->ch_coldload_id == ch->coldload_id &&
				 neg->obj_vnum == obj->virtual )
				break;
		}

		if ( neg && neg->price_delta )
			val_in_coppers = val_in_coppers * 100.0 / (100.0 + neg->price_delta);
		
		*stock_buf = '\0';

		if ( !keeper_makes (keeper, obj->virtual) )
			snprintf (stock_buf, MAX_STRING_LENGTH, "(%d in stock)", obj->count);

		temp_arg = obj_short_desc (obj);
		snprintf (buf2, MAX_STRING_LENGTH,   "  #1%3d %8.02f cp%s  #2%-55.55s#0", i, val_in_coppers, "   ", temp_arg);

		if ( IS_WEARABLE (obj) || neg ) {
			temp_arg = obj_short_desc(obj);
			if ( strlen(temp_arg) > 40 ) {
				buf2 [62] = '.';
				buf2 [63] = '.';
				buf2 [64] = '.';
			}
			buf2 [65] = '\0';
			strcat (buf2, "#0");
		}
		else {
			temp_arg = obj_short_desc(obj);
			if ( strlen(temp_arg) > 52 ) {
				buf2 [74] = '.';
				buf2 [75] = '.';
				buf2 [76] = '.';
			}
			buf2 [77] = '\0';
			strcat (buf2, "#0");
		}

		if ( IS_WEARABLE (obj) ) {
			if ( obj->size )
				snprintf (buf2 + strlen (buf2), MAX_STRING_LENGTH, " (%s)", sizes_named [obj->size]);
			else if ( keeper_makes (keeper, obj->virtual) )
				snprintf (buf2 + strlen (buf2), MAX_STRING_LENGTH, " (all sizes)");
			else
				strcat (buf2, " (all sizes)");
		}

		if ( IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) )
			strcat (buf2, " #6(used)#0");

		snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH, "%s", neg ? " (neg)" : "");

		strcat (buf2, "\n");

		if ( strlen(output) + strlen(buf2) > MAX_STRING_LENGTH )
			break;

		snprintf (output + strlen(output), MAX_STRING_LENGTH, "%s", buf2);
	}

	if ( !header_said ) {
		if ( *buf )
			act ("$N doesn't have any of those.",
						FALSE, ch, 0, keeper, TO_CHAR);
		else
			act ("Sadly, $N has nothing to sell.",
						FALSE, ch, 0, keeper, TO_CHAR);
	}
	else page_string (ch->desc, output);
	
	return;
}

/**
 *  Function: do_preview
 *
 *  Allows a character to see an itme for sale (read the object's full
 *	description.
 *
 *		\param	ch - character asking for the preview
 *		\param	argument - type of item and specific shopkeeper
 *		\param	cmd - not used  
 *				
 *		Useage: 
 *		- <b>preview tunic</b> will look at the first tunic
 *		- <b>preview 4</b> will look at the fourth item in the shop
 *		- <b>preview tunic copper</b> will look at the first tunic sold by
 *			the copper-haired man
 *			
 *		If found in the keepers inventory, the item will be shown to the
 *		character and he can see the long description of the item.
 *
 *
 */
void do_preview (CHAR_DATA *ch, char *argument, int cmd)
{
	int					i = 0;
	OBJ_DATA			*obj = NULL;
	CHAR_DATA			*keeper = NULL;
	CHAR_DATA			*tch = NULL;
	ROOM_DATA			*room = NULL;
	ROOM_DATA			*store = NULL;
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				buf2 [MAX_STRING_LENGTH] = {'\0'};
	
	room = ch->room;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Preview what?\n", ch);
		return;
	}

	if ( (keeper = get_char_room_vis (ch, buf)) )
		argument = one_argument (argument, buf);

	else {
		for ( tch = room->people; tch; tch = tch->next_in_room )
			if ( tch != ch && IS_SET (tch->flags, FLAG_KEEPER) )
				break;

		keeper = tch;

		if ( !keeper ) {
			send_to_char ("There is no shopkeeper here.\n", ch);
			return;
		}

		if ( !*buf ) {
			act ("PREVIEW what from $N?", TRUE, ch, 0, keeper, TO_CHAR);
			return;
		}
	}

	if ( !keeper ) {
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if ( GET_POS (keeper) <= POSITION_SLEEPING ) {
		act ("$N is not conscious.", TRUE, ch, 0, keeper, TO_CHAR);
		return;
	}

	if ( GET_POS (keeper) == POSITION_FIGHTING ) {
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if ( !keeper->shop ) {

		if ( keeper == ch )
			send_to_char ("You are not a shopkeeper.", ch);
		else
			act ("$N is not a keeper.", FALSE, ch, 0, keeper, TO_CHAR);

		return;
	}

	if ( keeper->shop->shop_vnum &&
		 keeper->shop->shop_vnum != ch->in_room ) {
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if ( !(store = vtor (keeper->shop->store_vnum)) ) {
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
				0);
		return;
	}

	if ( !store->psave_loaded )
		load_save_room (store);

	if ( !store->contents ) {
		do_say (keeper, "I have nothing for sale at the moment.", 0);
		return;
	}

	if ( store->contents && is_number (buf) ) {

		obj = store->contents;

		while ( obj && 
				(IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&
				keeper_uses_currency_type (keeper->mob->currency_type, obj))
				
			obj = obj->next_content;

		for ( i = 1;; i++ ) {	

			if ( !obj )
				break;

			if ( (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL))&& keeper_uses_currency_type (keeper->mob->currency_type, obj) ) {
				i--;
				if ( obj->next_content ) {
					obj = obj->next_content;
					continue;
				} else break;
			}

			if ( i == strtol (buf, NULL, 10) )
				break;

			obj = obj->next_content;
		}

		if ( !obj ) {
			name_to_ident (ch, buf2);
			snprintf (buf, MAX_STRING_LENGTH,   "%s %s", buf2, NO_SUCH_ITEM1);
			do_whisper (keeper, buf, 83);
			return;
		}
	}

	else if ( !(obj = get_obj_in_list_vis_not_money (ch, buf,
					vtor (keeper->shop->store_vnum)->contents)) ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s %s", buf2, NO_SUCH_ITEM1);
		do_whisper (keeper, buf, 83);
		return;
	}

    	act ("$N shows you $p.", FALSE, ch, obj, keeper, TO_CHAR);

	send_to_char ("\n", ch);

	show_obj_to_char (obj, ch, 5);
	
	return;
}

/**
 *  Function: keeper_makes
 *
 *  Determines if the shopkeeper trades in the item specified.
 *
 *		\param	keeper - the shopkeeper in question
 *		\param	ovnum - the object in question
 *
 *  	Returns a 0 if the shopkeeper does not trade in that item, and
 *		a 1 if the item is traded by the merchant.
 *      
 *
 */
int keeper_makes (CHAR_DATA *keeper, int ovnum)
{
	int		i = 0;

	if ( !keeper->shop || !ovnum )
		return 0;

	for ( i = 0; i < MAX_TRADES_IN; i++ )
		if ( keeper->shop->delivery [i] == ovnum )
			return 1;

	return 0;
}

/**
 *  Function: money_to_storeoom
 *
 *  Takes money from the keeper and puts in in his storeroom.
 *
 *		\param keeper the keeper of the store
 *		\param amount the amount of money to transfer
 *
 *		Adds up all money in the storeroom, and in keepers possesion and puts
 *		it in the smallest amount of coins possible.
 *
 *
 *
 */
void money_to_storeroom (CHAR_DATA *keeper, int amount)
{
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*next_obj = NULL;
	ROOM_DATA	*store = NULL;
	int			money = 0;
	

	if ( !keeper->shop )
		return;
	if ( !keeper->shop->store_vnum )
		return;
	if ( !(store = vtor(keeper->shop->store_vnum)) )
		return;

	while ( keeper_has_money (keeper, 1) ) {
		for ( obj = store->contents; obj; obj = next_obj ) {
			next_obj = obj->next_content;
			
			if (keeper_uses_currency_type(keeper->mob->currency_type, obj)){
				if ( (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL))) {
					money += obj->coppers*obj->count;
					obj_from_room(&obj, 0);
					extract_obj (obj);
				}
			}
		}
	}

	money += amount;
	
	(void)make_change(money, keeper, 0, '\0', 1, '\0');
	
	return;       
}

/**
 *  Function: subtract_keeper_money
 *
 *  Subtracts money from a shopkeeper
 *
 *		\param keeper the keeper of the store
 *		\param cost the amount of money to transfer
 *
 *		Adds up all money in the storeroom, deducts the cost and puts
 *		it in the smallest amount of coins possible.
 *
 *
 */
void subtract_keeper_money (CHAR_DATA *keeper, int cost)
{
	OBJ_DATA	*obj= NULL;
	OBJ_DATA	*next_obj =  NULL;
	ROOM_DATA	*store = NULL;
	int 		money = 0;
	
	if ( !keeper->shop )
		return;
	if ( !keeper->shop->store_vnum )
		return;
	if ( !(store = vtor(keeper->shop->store_vnum)) )
		return;

	while ( keeper_has_money (keeper, 1) ) {
		for ( obj = store->contents; obj; obj = next_obj ) {
			next_obj = obj->next_content;
			if ( (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) && keeper_uses_currency_type (keeper->mob->currency_type, obj) ) {
				money += obj->coppers*obj->count;
				obj_from_room(&obj, 0);
				extract_obj (obj);
			}
		}
	}

	money -= cost;

	(void)make_change(money, keeper, 0, '\0', 1, '\0');


	return;   
}

/**
 *  Function: do_buy
 *
 *  Allows a character to purchase an item from a shopkeeper.
 *
 *		\param ch character doing the purchasing
 *		\param argument type of item, count, and specific shopkeeper
 *		\param cmd barter or confirming purchase  
 *
 * 		Useage:
 *			buy [keeper] [count] item [size | !] [ | 1 | 2]
 *		
 *		Item can be either a keyword or the number from the do_list.
 *		When buying sizes, use <b>!</b> if you want a size that you can't wear.
 *		cmd is sent by do_barter or do_accept with a value of 1 for barter,
 *		and 2 when confirming a purchase. Also, checks prevent players from
 *		buying back items they've sold, and prevents all others from buying a
 *		sold item for 15 minutes to prevent abuse 
 *
 *
 *
 */
void do_buy (CHAR_DATA *ch, char *argument, int cmd) 
{
	int					buy_count = 1;
	float				keepers_cost = 0;
	int					delivery_cost;
	int					i = 0;
	int					regardless = 0;
	int					wants_off_size = 0;
	int					size = -1;
	int					nobarter_flag = 0;
	int					discount = 0;
	float				markup = 0;
	int					keeper_success = 0;
	int					ch_success, flags = 0;
	int					orig_count = 0;
	OBJ_DATA			*obj = NULL;
	OBJ_DATA			*tobj = NULL;
	OBJ_DATA			*temp_obj = NULL;
	CHAR_DATA			*horse = NULL;
	CHAR_DATA			*keeper = NULL;
	CHAR_DATA			*tch = NULL;
	ROOM_DATA			*room = NULL;
	ROOM_DATA			*store = NULL;
	NEGOTIATION_DATA	*neg = NULL;
	char				name [MAX_STRING_LENGTH] = {'\0'};
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				buf2 [MAX_STRING_LENGTH] = {'\0'};
	char				*temp_arg1 = NULL;
	char				*temp_arg2 = NULL;
	
	room = ch->room;

	argument = one_argument (argument, buf);

	if ( cmd != 2 ) {
		if ( !*buf ) {
			send_to_char ("Buy what?\n", ch);
			return;
		}

		if ( (keeper = get_char_room_vis (ch, buf)) && IS_SET (keeper->flags, FLAG_KEEPER) )
			argument = one_argument (argument, buf);

		else {
			for ( tch = room->people; tch; tch = tch->next_in_room )
				if ( tch != ch && IS_SET (tch->flags, FLAG_KEEPER) )
					break;
	
			keeper = tch;

			if ( !*buf ) {
				act ("Buy what from $N?", TRUE, ch, 0, keeper, TO_CHAR);
				return;
			}
		}

		argument = one_argument (argument, buf2);

			/*** buf is a count if buf2 is an object ***/

		if ( isdigit (*buf) && *buf2 &&
			 (isdigit (*buf2) || (keeper && keeper->shop && get_obj_in_list_vis_not_money (ch, buf2, vtor (keeper->shop->store_vnum)->contents))) ) {
			buy_count = strtol (buf, NULL, 10);
			if ( buy_count > 50 ) {
				send_to_char ("You can only buy up to 50 items at a time.\n", ch);
				return;
			}
			strcpy (buf, buf2);
			argument = one_argument (argument, buf2);
		}

		if ( *buf2 == '!' )
			regardless = 1;

		else if ( *buf2 ) {

			size = index_lookup (sizes_named, buf2);

			if ( size == -1 )
				size = index_lookup (sizes, buf2);

			wants_off_size = 1;
		}
	}
	else keeper = ch->delay_ch;

	if ( !keeper || keeper->room != ch->room ) {
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if ( keeper == ch ) {
		send_to_char ("You can't buy from yourself!\n", ch);
		return;
	}

	if ( GET_POS (keeper) <= POSITION_SLEEPING ) {
		act ("$N is not conscious.", TRUE, ch, 0, keeper, TO_CHAR);
		return;
	}

	if ( GET_POS (keeper) == POSITION_FIGHTING ) {
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if ( !GET_TRUST (ch) && !CAN_SEE (keeper, ch) ) {
		do_say (keeper, "Who's there?", 0);
		return;
	}

	if ( !keeper->shop ) {

		if ( keeper == ch )
			send_to_char ("You are not a shopkeeper.", ch);
		else
			act ("$N is not a keeper.", FALSE, ch, 0, keeper, TO_CHAR);

		return;
	}

	if ( !keeper->shop || !IS_NPC(keeper) ) {
		send_to_char ("Are you sure they're a shopkeeper?\n", ch);
		return;
	}
	
	if ( keeper->shop->shop_vnum &&
		 keeper->shop->shop_vnum != ch->in_room ) {
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if ( !(store = vtor (keeper->shop->store_vnum)) ) {
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
				0);
		return;
	}

	if ( !store->psave_loaded )
		load_save_room (store);

	if ( !store->contents ) {
		do_say (keeper, "I have nothing for sale at the moment.", 0);
		return;
	}

	if ( cmd != 2 ) {
		if ( is_number (buf) ) {

			obj = store->contents;

			while 	(obj &&
					(IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) && 
					keeper_uses_currency_type (keeper->mob->currency_type, obj))
				obj = obj->next_content;

			for ( i = 1;; i++ ) {	

				if ( !obj )
					break;

				if ((IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)) &&
					keeper_uses_currency_type (keeper->mob->currency_type, obj) ) {
					i--;
					if ( obj->next_content ) {
						obj = obj->next_content;
						continue;
					} else {
						obj = NULL;
						break;
					}
				}

/*** Prevent players from buying back items they've sold, and prevents all others from buying a sold item for 15 minutes to prevent abuse ***/

				if ( (obj->sold_by != ch->coldload_id && (time(0) - obj->sold_at <= 60 * 15)) ||
					(obj->sold_by == ch->coldload_id && (time(0) - obj->sold_at <= 60 * 60)) ) {
					i--;
					if ( obj->next_content ) {
						obj = obj->next_content;
						continue;
					} else {
						obj = NULL;
						break;
					}
				}

				if ( i == strtol (buf, NULL, 10) )
					break;

				obj = obj->next_content;
			}

			if ( !obj ) {
				send_to_char ("There are not that many items in the keeper's "
							  "inventory.\n", ch);
				return;
			}
		}

		else if ( !(obj = get_obj_in_list_vis_not_money (ch, buf,
						vtor (keeper->shop->store_vnum)->contents)) ) {
			name_to_ident (ch, buf2);
			snprintf (buf, MAX_STRING_LENGTH,   "%s %s", buf2, NO_SUCH_ITEM1);
			do_whisper (keeper, buf, 83);
			return;
		}

		if ( IS_WEARABLE (obj) && wants_off_size ) {
			if ( obj->size && obj->size != size ) {
				act ("$p isn't that size.", FALSE, ch, obj, 0, TO_CHAR);
				return;
			}
		}

		else if ( IS_WEARABLE (obj) && obj->size &&
			 obj->size != get_size (ch) && !regardless && obj->size != size ) {
			act ("$p wouldn't fit you.",
					FALSE, ch, obj, 0, TO_CHAR);
			act ("(End the buy command with ! if you really want it.)",
					FALSE, ch, obj, 0, TO_CHAR);
			return;
		}

		if ( !keeper_makes (keeper, obj->virtual) && buy_count > obj->count ) {
			name_to_ident (ch, buf2);
			snprintf (buf, MAX_STRING_LENGTH,   "%s I only have %d of that in stock at the moment.", buf2, obj->count);
			do_whisper (keeper, buf, 83);
			return;
		}
		else if ( keeper_makes (keeper, obj->virtual) && buy_count > obj->count )
			obj->count = buy_count;

		if ( buy_count < 1 )
			buy_count = 1;
	}

/*** cmd==2 which means do_accept ***/
	else { 

		if ( ch->delay_type != DEL_PURCHASE_ITEM || !ch->delay_obj ) {
			send_to_char ("There is no purchase in progress, I'm afraid.\n", ch);
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->delay_obj = NULL;
			ch->delay_ch = NULL;
			return;
		}

		if ( (obj = ch->delay_obj) && obj->in_room != keeper->shop->store_vnum ) {
			send_to_char ("That item is no longer available for purchase.\n", ch);
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->delay_obj = NULL;
			ch->delay_ch = NULL;
			return;
		}
		else if ( !obj ) {
			send_to_char ("That item is no longer available for purchase.\n", ch);
			ch->delay_type = 0;
			ch->delay_info1 = 0;
			ch->delay_obj = NULL;
			ch->delay_ch = NULL;
			return;
		}

		buy_count = ch->delay_info1;

		if ( buy_count < 1 )
			buy_count = 1;

		ch->delay_type = 0;
		ch->delay_info1 = 0;
		ch->delay_obj = NULL;
		ch->delay_ch = NULL;
	}
/*** end do_accept ***/

	keepers_cost = obj->coppers;

	if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) )
    		keepers_cost *= econ_markup (keeper, obj);
	else keepers_cost *= econ_discount (keeper, obj);

    	keepers_cost = keepers_cost * buy_count;

	if ( keepers_cost < 1 )
		keepers_cost = 1;

	if ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON &&
		 obj->o.drinkcon.volume > 0 &&
		 (tobj = vtoo (obj->o.drinkcon.liquid)) &&
		 GET_ITEM_TYPE (tobj) == ITEM_FLUID ) {
		if ( !IS_SET (tobj->obj_flags.extra_flags, ITEM_PC_SOLD) )
			markup = econ_markup (keeper, tobj);
		else markup = econ_discount (keeper, tobj);;
		keepers_cost += (tobj->coppers * obj->o.drinkcon.volume * markup);
	}

	if ( IS_SET (ch->room->room_flags, LAWFUL) &&
		 IS_SET (obj->obj_flags.extra_flags, ITEM_ILLEGAL) ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s Those are illegal.  I can't sell them.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ( GET_ITEM_TYPE(obj) == ITEM_NPC_OBJECT ) {
		if ( cmd != 2 ) {
			if ( !*buf2 ) {
				send_to_char ("You'll need to specify a name for your new NPC, e.g. \"buy horse Shadowfax\".\n", ch);
				return;
			}
		}
		else {
			snprintf (buf2, MAX_STRING_LENGTH,   "%s", ch->delay_who);
			mem_free (ch->delay_who);
		}
		if ( strlen(buf2) > 26 ) {
			send_to_char ("The NPC's name must be 26 letters or fewer in length.\n", ch);
			return;
		}
		for ( i = 0; i < strlen(buf2); i++ ) {
			if ( !isalpha (buf2[i]) ) {
				send_to_char ("Invalid characters in the proposed NPC name.\n", ch);
				return;
			}
		}
		snprintf (name, MAX_STRING_LENGTH, "%s", buf2);
	}
	
/*** passed by do_barter to do_buy for bartering ***/
	if ( cmd == 1 )	{	

		name_to_ident (ch, buf);

		if ( keepers_cost < 20 ) {
			strcat (buf, " This isn't worth haggling over.");
			do_whisper (keeper, buf, 0);
			return;
		}

		if ( IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) ) {
			strcat (buf, " I won't haggle over a used piece of merchandse.");
			do_whisper (keeper, buf, 0);
			return;
		}

		nobarter_flag = index_lookup (econ_flags, "nobarter");

		if ( nobarter_flag != -1 &&
			 IS_SET (keeper->shop->nobuy_flags, 1 << nobarter_flag) ) {
			strcat (buf, " I'm sorry, but I will not haggle.  My prices "
						 "are fixed, take it or leave it.");
			do_whisper (keeper, buf, 0);
			return;
		}

		if ( nobarter_flag != -1 &&
			 IS_SET (obj->econ_flags, 1 << nobarter_flag) ) {
			strcat (buf, " I'm sorry, but I will not haggle over the price "
						 "of this item.");
			do_whisper (keeper, buf, 0);
			return;
		}

/*** Search for existing entry in keepers negotiations list ***/

		for ( neg = keeper->shop->negotiations; neg; neg = neg->next ) {
			if ( neg->ch_coldload_id == ch->coldload_id &&
				 neg->obj_vnum == obj->virtual &&
				 neg->true_if_buying )
				break;
		}

		if ( neg ) {
			if ( neg->price_delta > 0 ) {
				strcat (buf, " No, no, I cannot afford to lower the price on "
							 "that again.");
				do_whisper (keeper, buf, 0);
				return;
			}

			strcat (buf, " You're persistent, aren't you?  I said no, and I "
						 "meant no.");
			do_whisper (keeper, buf, 0);
			return;
		}

		keeper_success = combat_roll (keeper->skills [SKILL_BARTER]);
		ch_success = combat_roll (ch->skills [SKILL_BARTER]);

		if      ( ch_success == SUC_CS && keeper_success == SUC_MS )
			discount = 5;
		else if ( ch_success == SUC_CS && keeper_success == SUC_MF )
			discount = 10;
		else if ( ch_success == SUC_CS && keeper_success == SUC_CF )
			discount = 15;

		else if ( ch_success == SUC_MS && keeper_success == SUC_MS )
			discount = 0;
		else if ( ch_success == SUC_MS && keeper_success == SUC_MF )
			discount = 5;
		else if ( ch_success == SUC_MS && keeper_success == SUC_CF )
			discount = 10;

		else if ( ch_success == SUC_MF && keeper_success == SUC_MS )
			discount = 0;
		else if ( ch_success == SUC_MF && keeper_success == SUC_MF )
			discount = 0;
		else if ( ch_success == SUC_MF && keeper_success == SUC_CF )
			discount = 5;

		else
			discount = 0;	/* A CF by ch */

		neg = malloc (sizeof (NEGOTIATION_DATA));
		neg->ch_coldload_id = ch->coldload_id;
		neg->obj_vnum = obj->virtual;
		neg->time_when_forgotten = time (NULL) + 6 * 60 * 60;  /* 6 hours */
		neg->price_delta = discount;
		neg->transactions = 0;
		neg->true_if_buying = 1;
		neg->next = keeper->shop->negotiations;
		keeper->shop->negotiations = neg;

		if ( discount == 0 ) {
			strcat (buf, " The price is as stated.  Take it or leave it.");
			do_whisper (keeper, buf, 0);
			return;
		}

		else if ( discount == 5 )
			strcat (buf, " I like your face, you seem an honest and "
						 "trustworthy sort.  You can have it for ");
		else if ( discount == 10 )
			strcat (buf, " It's just not my day, is it?  All right, you win, "
						 "I'll sell at your price.  It's yours for ");
		else
			strcat (buf, " My word!  I need to learn to count!  At this rate, "
						 "I'll be out of business in a week.  Here, here, take "
						 "your ill-gotten gain and begone.  Take it away for ");

		keepers_cost = keepers_cost * 100 / (100 + discount);

		keepers_cost = (int) keepers_cost;

		snprintf (ADDBUF, MAX_STRING_LENGTH, "%d copper%s", (int) keepers_cost, (int) keepers_cost > 1 ? "s" : "");

		strcat (buf, ".");

		do_whisper (keeper, buf, 0);

		return;
	}

	if ( keepers_cost < 1 )
		keepers_cost = 1;

	keepers_cost = (int) keepers_cost;

		/*** Look up negotiations for this ch/obj on keeper ***/

	for ( neg = keeper->shop->negotiations; neg; neg = neg->next ) {
		if ( neg->ch_coldload_id == ch->coldload_id &&
			 neg->obj_vnum == obj->virtual &&
			 neg->true_if_buying )
			break;
	}

	if ( neg && neg->price_delta )
		keepers_cost = keepers_cost * 100 / (100 + neg->price_delta);

	keepers_cost = (int) keepers_cost;

	if ( cmd != 2 ) {
		orig_count = obj->count;
		obj->count = buy_count;
		if ( obj->in_room != keeper->shop->store_vnum )
			obj->in_room = keeper->shop->store_vnum;
		
		temp_arg1 = obj_short_desc (obj);
		snprintf (buf, MAX_STRING_LENGTH,   "You have opted to purchase %s, for a total of %d copper%s. To confirm, please use the ACCEPT command.", temp_arg1, (int) keepers_cost, (int) keepers_cost > 1 ? "s" : "");
		act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		ch->delay_type = DEL_PURCHASE_ITEM;
		ch->delay_obj = obj;
		ch->delay_info1 = buy_count;
		if ( ch->delay_info1 < 1 )
			ch->delay_info1 = 1;
		ch->delay_ch = keeper;
		obj->count = orig_count;
		if ( GET_ITEM_TYPE(obj) == ITEM_NPC_OBJECT )
			ch->delay_who = str_dup (name);
		return;
	}

	if ( !can_subtract_money (ch, keepers_cost, keeper->mob->currency_type) ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s %s", buf2, MISSING_CASH2);
		do_whisper (keeper, buf, 83);

		return;
	}

	if ( obj->morphTime ) {
		obj->clock = vtoo(obj->virtual)->clock;
		obj->morphTime = time(0) + obj->clock * 10 * 60;
	}

	tobj = obj;
	obj_from_room (&tobj, buy_count);

	if ( keeper_makes (keeper, obj->virtual) &&
		!get_obj_in_list_num (obj->virtual, vtor(keeper->shop->store_vnum)->contents) ) {


		tobj->count = buy_count;
		delivery_cost = obj->coppers;
		delivery_cost *= econ_markup (keeper, obj);
		delivery_cost *= .50;
		delivery_cost = (int) delivery_cost;

		if ( keeper_has_money (keeper, delivery_cost) ) {
			temp_obj = load_object(obj->virtual);
			obj_to_room (temp_obj, keeper->shop->store_vnum);
			subtract_keeper_money (keeper, delivery_cost);

		}
	}

	act ("$n buys $p.", FALSE, ch, tobj, 0, TO_ROOM);
	act ("$N sells you $p.", FALSE, ch, tobj, keeper, TO_CHAR);
	act ("You sell $N $p.", FALSE, keeper, tobj, ch, TO_CHAR);

	subtract_money (ch, keepers_cost, keeper->mob->currency_type);

	name_to_ident (ch, buf2);

	if ( ch->room->zone == 5 || ch->room->zone == 6 ) {
		snprintf (buf, MAX_STRING_LENGTH,   "%s You're lucky I gave it to you for %d copper%s, maggot.", buf2, (int) keepers_cost, (int) keepers_cost > 1 ? "s" : "");
	} else {
		snprintf (buf, MAX_STRING_LENGTH,   "%s A veritable steal at ", buf2);
       	 	snprintf (ADDBUF, MAX_STRING_LENGTH,
       	                "%d copper%s! Enjoy it, my friend.", (int) keepers_cost,
        	               (int) keepers_cost > 1 ? "s" : "");
	}

	do_whisper (keeper, buf, 83);

	money_to_storeroom (keeper, keepers_cost);

	if ( GET_ITEM_TYPE(tobj) == ITEM_NPC_OBJECT ) {
		*name = toupper (*name);
		if ( tobj->o.od.value[0] == 0 || !(horse = load_mobile(tobj->o.od.value[0])) ) {
			send_to_char ("There seems to be a problem. Please inform the staff.\n", ch);
			return;
		}
		send_to_room ("\n", keeper->in_room);
		
		temp_arg1 = char_short(horse);
		snprintf (buf, MAX_STRING_LENGTH,   "%s is released into your custody.", temp_arg1);
		*buf = toupper (*buf);
		snprintf (buf2, MAX_STRING_LENGTH,   "#3%s", buf);
		act (buf2, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);		
		
		temp_arg1 = char_short(horse);
		temp_arg2 = char_short(ch);
		snprintf (buf, MAX_STRING_LENGTH,   "%s#0 is released into #3%s#0's custody.", temp_arg1, temp_arg2);
		*buf = toupper (*buf);
		
		snprintf (buf2, MAX_STRING_LENGTH,   "#3%s", buf);
		act (buf2, FALSE, ch, 0, 0, TO_NOTVICT | TO_ACT_FORMAT);
		char_to_room (horse, ch->in_room);
		SET_BIT (horse->act, ACT_STAYPUT);
		
		snprintf (buf, MAX_STRING_LENGTH,   "%s %s", horse->name, name);
		mem_free (horse->name);
		horse->name = str_dup (buf);
		
		if ( get_clan (ch, "mm_denizens", &flags) ) {
			add_clan (horse, "mm_denizens", CLAN_MEMBER);
		} 
		else if ( get_clan (ch, "osgi_citizens", &flags) ) {
			add_clan (horse, "osgi_citizens", CLAN_MEMBER);
		}
		
		if ( IS_SET (horse->act, ACT_MOUNT) )
			hitch_char (ch, horse);
		
		if ( !IS_NPC (ch) ) {
			horse->mob->owner = str_dup (ch->tname);
			save_char (ch, TRUE);
		}
		return;
	}

	if ( GET_ITEM_TYPE(obj) == ITEM_LIGHT ) {
		tobj->o.od.value[1] = obj->o.od.value[1];
	}

	if ( IS_WEARABLE (tobj) ) {
		if ( size != -1 )
			tobj->size = size;
		else if ( !regardless || !tobj->size )
			tobj->size = get_size (ch);
	}

	if ( GET_ITEM_TYPE (tobj) == ITEM_CONTAINER && tobj->o.od.value[2] > 0 && vtoo (tobj->o.od.value[2]) && GET_ITEM_TYPE (vtoo(tobj->o.od.value[2])) == ITEM_KEY ) {
		obj = load_object (tobj->o.od.value[2]);
		obj->o.od.value[1] = tobj->coldload_id;
		obj_to_obj (obj, tobj);
		obj = load_object (tobj->o.od.value[2]);
		obj->o.od.value[1] = tobj->coldload_id;
		obj_to_obj (obj, tobj);
	}

	if ( ch->right_hand && ch->left_hand ) {
		snprintf (buf, MAX_STRING_LENGTH,   "%s Your hands seem to be full, so I'll just set this down for you to pick up when you've a chance.", ch->tname);
		send_to_char ("\n", ch);
		do_whisper (keeper, buf, 0);
		(void)one_argument (obj->name, buf2);
		send_to_char ("\n", ch);
		snprintf (buf, MAX_STRING_LENGTH,   ": sets *%s down nearby, nodding to ~%s.", buf2, ch->tname);
		command_interpreter (keeper, buf);
		obj_to_room (tobj, keeper->in_room);
	}
	else obj_to_char (tobj, ch);

	if ( neg )
		if ( !neg->transactions++ )
			skill_use (ch, SKILL_BARTER, 0);
			
	return;
}


/**
 *  Function: do_sell
 *
 *  Allows character to sell an item to a shopkeeper.
 *
 *		\param ch character doing the selling
 *		\param argument type of item, count, and specific shopkeeper
 *		\param cmd 1 if do_sell is called by do_barter command
 *
 * 		Useage:
 *			sell [count] item [keeper]
 *
 *		Removes the item from the charater, and gives money to them.
 *		The cost is removed from the keepers storeroom. 
 *
 *
 *
 */
void do_sell (CHAR_DATA *ch, char *argument, int cmd) 
{
	int					objs_in_storage = 0;
	int					sell_count = 1;
	int					nobarter_flag = 0;
	int					keeper_success = 0;
	int					ch_success = 0;
	int					discount = 0;
	int					same_obj = 0;
	float				liquid_discount = 0;
	float				keepers_cost = 0;
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				buf2 [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA			*obj = NULL;
	OBJ_DATA			*tobj = NULL;
	CHAR_DATA			*keeper = NULL;
	CHAR_DATA			*tch = NULL;
	ROOM_DATA			*room = NULL;
	NEGOTIATION_DATA	*neg = NULL;
	
	argument = one_argument (argument, buf);

	room = ch->room;

	if ( isdigit (*buf) ) {
		sell_count = strtol (buf, NULL, 10);
		argument = one_argument (argument, buf);
		if ( sell_count > 50 ) {
			send_to_char ("Sorry, but you can only sell up to 50 items at a time.\n", ch);
			return;
		}
	}

	if ( !*buf ) {
		send_to_char ("Sell what?\n", ch);
		return;
	}

	if ( (keeper = get_char_room_vis (ch, buf)) )
		argument = one_argument (argument, buf);

	else {
		for ( tch = room->people; tch; tch = tch->next_in_room )
			if ( tch != ch && IS_SET (tch->flags, FLAG_KEEPER) )
				break;

		keeper = tch;

		if ( !*buf ) {
			act ("Sell what to $N?", TRUE, ch, 0, keeper, TO_CHAR);
			return;
		}
	}

	if ( !keeper ) {
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if ( keeper == ch ) {
		send_to_char ("You can't sell to yourself!\n", ch);
		return;
	}

	if ( !keeper->shop || !IS_NPC (keeper) ) {
		act ("$N does not seem to be a shopkeeper.", TRUE, ch, 0, keeper, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( GET_POS (keeper) <= POSITION_SLEEPING ) {
		act ("$N is not conscious.", TRUE, ch, 0, keeper, TO_CHAR);
		return;
	}

	if ( GET_POS (keeper) == POSITION_FIGHTING ) {
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if ( !keeper->shop ||
		 (keeper->shop->shop_vnum &&
		  keeper->shop->shop_vnum != ch->in_room) ) {
		do_say (keeper, "I'm sorry.  Please catch me when I'm in my shop.", 0);
		return;
	}

	if ( !(room = vtor (keeper->shop->store_vnum)) ) {
		do_say (keeper, "I've lost my business.  You'll have to go elsewhere.",
				0);
		return;
	}

	if ( !room->psave_loaded )
		load_save_room (room);

	if ( !GET_TRUST (ch) && !CAN_SEE (keeper, ch) ) {
		do_say (keeper, "Who's there?", 0);
		return;
	}

	if ( !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s %s", buf2, NO_SUCH_ITEM2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ( obj->count < sell_count )
		sell_count = obj->count;

	if ( sell_count < 1 )
		sell_count = 1;

	keepers_cost = obj->coppers;
    	keepers_cost *= econ_discount (keeper, obj);
    	keepers_cost = keepers_cost * sell_count;
	keepers_cost = (int) keepers_cost;

	if ( !keeper_makes (keeper, obj->virtual) ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s %s", buf2, DO_NOT_BUY);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ( obj->econ_flags & keeper->shop->nobuy_flags || (GET_ITEM_TYPE(obj) == ITEM_LIGHT && !obj->o.od.value[1])) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s No, I wouldn't even think of buying that.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}
				
	if ( IS_SET (ch->room->room_flags, LAWFUL) &&
		 IS_SET (obj->obj_flags.extra_flags, ITEM_ILLEGAL) ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s I can't buy that.  It's illegal to possess.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}



	if ( vtor (keeper->shop->store_vnum) ) {

		objs_in_storage = 0;

		for ( tobj = vtor (keeper->shop->store_vnum)->contents;
			  tobj;
			  tobj = tobj->next_content ) {
			if ( !(IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL)) && IS_SET (tobj->obj_flags.extra_flags, ITEM_PC_SOLD) )
				objs_in_storage++;
			if ( tobj->virtual == obj->virtual )
				same_obj += tobj->count;
		}

		if ( (!IS_SET (obj->obj_flags.extra_flags, ITEM_STACK) && same_obj >= 6) || same_obj >= 24 ) {
			name_to_ident (ch, buf2);
			snprintf (buf, MAX_STRING_LENGTH,   "%s I have quite enough of those for now, thank you; try back again later.", buf2);
			do_whisper (keeper, buf, 83);
			return;
		}

		if ( objs_in_storage > 125 ) {
			name_to_ident (ch, buf2);
			snprintf (buf, MAX_STRING_LENGTH,   "%s I have too much stuff as it is.  Perhaps you'd like to purchase something instead?", buf2);
			do_whisper (keeper, buf, 83);
			return;
		}
	}

	if ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON &&
		 obj->o.drinkcon.volume > 0 &&
		 (tobj = vtoo (obj->o.drinkcon.liquid)) &&
		 GET_ITEM_TYPE (tobj) == ITEM_FLUID ) {
		liquid_discount = econ_discount (keeper, tobj);
		keepers_cost += (tobj->coppers * obj->o.drinkcon.volume * liquid_discount);
	}

/*** passed by do_barter to do_sell for bartering ***/
	if ( cmd == 1 )	{
		if ( sell_count > 1 ) {
			send_to_char ("You can only barter for one item at a time.\n", ch);
			return;
		}

		name_to_ident (ch, buf);

		if ( keepers_cost < 20 ) {
			strcat (buf, " This isn't worth haggling over.");
			do_whisper (keeper, buf, 0);
			return;
		}

		nobarter_flag = index_lookup (econ_flags, "nobarter");

		if ( nobarter_flag != -1 &&
			 IS_SET (keeper->shop->nobuy_flags, 1 << nobarter_flag) ) {
			strcat (buf, " I'm sorry, but I will not haggle.  My prices "
						 "are fixed, take it or leave it.");
			do_whisper (keeper, buf, 0);
			return;
		}

                if ( IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) ) {
                        strcat (buf, " I won't haggle over a used piece of merchandse.");
                        do_whisper (keeper, buf, 0);
                        return;
                }

		if ( nobarter_flag != -1 &&
			 IS_SET (obj->econ_flags, 1 << nobarter_flag) ) {
			strcat (buf, " I'm sorry, but I will not haggle over the price "
						 "of this item.");
			do_whisper (keeper, buf, 0);
			return;
		}

			/*** Search for existing entry in keepers negotiations list ***/

		for ( neg = keeper->shop->negotiations; neg; neg = neg->next ) {
			if ( neg->ch_coldload_id == ch->coldload_id &&
				 neg->obj_vnum == obj->virtual &&
				 !neg->true_if_buying )
				break;
		}

		if ( neg ) {
			if ( neg->price_delta > 0 ) {
				strcat (buf, " No, no, I will not pay any higher a price.");
				do_whisper (keeper, buf, 0);
				return;
			}

			strcat (buf, " Listen, as much as I like you, I simply cannot "
						 "offer you what you're asking.");
			do_whisper (keeper, buf, 0);
			return;
		}

		keeper_success = combat_roll (keeper->skills [SKILL_BARTER]);
		ch_success = combat_roll (ch->skills [SKILL_BARTER]);

		if      ( ch_success == SUC_CS && keeper_success == SUC_MS )
			discount = 5;
		else if ( ch_success == SUC_CS && keeper_success == SUC_MF )
			discount = 10;
		else if ( ch_success == SUC_CS && keeper_success == SUC_CF )
			discount = 15;

		else if ( ch_success == SUC_MS && keeper_success == SUC_MS )
			discount = 0;
		else if ( ch_success == SUC_MS && keeper_success == SUC_MF )
			discount = 5;
		else if ( ch_success == SUC_MS && keeper_success == SUC_CF )
			discount = 10;

		else if ( ch_success == SUC_MF && keeper_success == SUC_MS )
			discount = 0;
		else if ( ch_success == SUC_MF && keeper_success == SUC_MF )
			discount = 0;
		else if ( ch_success == SUC_MF && keeper_success == SUC_CF )
			discount = 5;

		else
		/*** A CF by ch ***/
			discount = 0;	

		neg = malloc (sizeof (NEGOTIATION_DATA));
		neg->ch_coldload_id = ch->coldload_id;
		neg->obj_vnum = obj->virtual;
		neg->time_when_forgotten = time (NULL) + 6 * 60 * 60; /* 6 hours */
		neg->price_delta = discount;
		neg->transactions = 0;
		neg->true_if_buying = 0;
		neg->next = keeper->shop->negotiations;
		keeper->shop->negotiations = neg;

		if ( discount == 0 ) {
			strcat (buf, " Sorry, but it's just not worth more than my "
						 "initial offer.");
			do_whisper (keeper, buf, 0);
			return;
		}

		else if ( discount == 5 )
			strcat (buf, " I've been looking for these.  It's a pleasure doing "
						 "business with you.  I'll pay you ");
		else if ( discount == 10 )
			strcat (buf, " Perhaps if I go back to bed now, I can salvage some "
						 "small part of my self respect.  I'll pay you ");
		else
			strcat (buf, " It is a dark day.  I'll have to sell my home and "
						 "business just to recoup what I've lost this day."
						 "  I'll give you ");

		keepers_cost = keepers_cost * (100 + discount) / 100;

		snprintf (ADDBUF, MAX_STRING_LENGTH, "%d copper%s", (int) keepers_cost, (int) keepers_cost > 1 ? "s" : "");

		strcat (buf, ".");

		do_whisper (keeper, buf, 0);

		return;
	}

		/* Look up negotiations for this ch/obj on keeper */

	for ( neg = keeper->shop->negotiations; neg; neg = neg->next ) {
		if ( neg->ch_coldload_id == ch->coldload_id &&
			 neg->obj_vnum == obj->virtual &&
			 !neg->true_if_buying )
			break;
	}

	keepers_cost = (int) keepers_cost;



	if ( keepers_cost < 1 ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s Bah, that isn't even worth my time!", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ( neg && neg->price_delta )
		keepers_cost = keepers_cost * (100 + neg->price_delta) / 100;




	if ( !keeper_has_money (keeper, keepers_cost) ) {

		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s %s", buf2, MISSING_CASH1);

		do_whisper (keeper, buf, 83);

		if ( !IS_SET (keeper->act, ACT_PCOWNED) ||
			 !vtor (keeper->shop->store_vnum) )
			return;

		return;
	}
	

	act ("$n sells $p.", FALSE, ch, obj, 0, TO_ROOM);
	act ("You sell $p.", FALSE, ch, obj, 0, TO_CHAR);
	send_to_char ("\n", ch);

	name_to_ident (ch, buf2);
	snprintf (buf, MAX_STRING_LENGTH, "%s Here's the amount we've agreed upon.", buf2);

			/*** Pay customer ***/

	do_whisper (keeper, buf, 83);

	subtract_keeper_money (keeper, keepers_cost);
	obj_from_char (&obj, sell_count);
	
	keeper_money_to_char (keeper, ch, keepers_cost);

	


	if ( keeper_makes (keeper, obj->virtual) )
		extract_obj (obj);
	else {
		SET_BIT (obj->obj_flags.extra_flags, ITEM_PC_SOLD);
		obj->sold_at = (int) time(0);
		obj->sold_by = ch->coldload_id;
		obj_to_room (obj, keeper->shop->store_vnum);
	}

	if ( neg )
		if ( !neg->transactions++ )
			skill_use (ch, SKILL_BARTER, 0);
			
	return;
}

/**
 *  Function: do_value
 *
 *  Determines the value of an item when selling to shopkeepers.
 *
  *		\param ch character asking the price
 *		\param argument item and the shopkeeper
 *		\param cmd not used
 *
 *		Useage:
 *			value item [keeper]
 *			
 *		Informs the character what the item would be worth to the shopkeeper,
 *		it was to be sold.
 *
 *
 */
void do_value (CHAR_DATA *ch, char *argument, int cmd) 
{
	float				keepers_cost = 0;
	float				discount = 0;
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				buf2 [MAX_STRING_LENGTH] = {'\0'};
	char				buf3 [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA			*obj = NULL;
	OBJ_DATA			*tobj = NULL;
	ROOM_DATA			*room = NULL;
	CHAR_DATA			*keeper = NULL;
	CHAR_DATA			*tch = NULL;
	NEGOTIATION_DATA	*neg = NULL;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Appraise what?\n", ch);
		return;
	}

	room = ch->room;

	if ( (keeper = get_char_room_vis (ch, buf)) )
		argument = one_argument (argument, buf);

	else {
		for ( tch = room->people; tch; tch = tch->next_in_room )
			if ( tch != ch && IS_SET (tch->flags, FLAG_KEEPER) )
				break;

		keeper = tch;

		if ( !*buf ) {
			act ("Have $N appraise what?", TRUE, ch, 0, keeper, TO_CHAR);
			return;
		}
	}

	if ( !keeper ) {
		send_to_char ("There is no merchant here.\n", ch);
		return;
	}

	if ( !keeper->shop ) {
		act ("$N does not seem to be a shopkeeper.", TRUE, ch, 0, keeper, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( GET_POS (keeper) <= POSITION_SLEEPING ) {
		act ("$N is not conscious.", TRUE, ch, 0, keeper, TO_CHAR);
		return;
	}

	if ( GET_POS (keeper) == POSITION_FIGHTING ) {
		do_say (keeper, "Have you no eyes!  I'm fighting for my life!", 0);
		return;
	}

	if ( !(obj = get_obj_in_list_vis (ch, buf, ch->right_hand)) &&
		!(obj = get_obj_in_list_vis (ch, buf, ch->left_hand)) ) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s %s", buf2, NO_SUCH_ITEM2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ( !keeper_makes (keeper, obj->virtual)) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s %s", buf2, DO_NOT_BUY);
		do_whisper (keeper, buf, 83);
		return;
	}

	if (obj->econ_flags & keeper->shop->nobuy_flags ||
		(keepers_cost == 0 && (IS_SET (obj->obj_flags.extra_flags, ITEM_CAPITAL)))) {
		name_to_ident (ch, buf2);
		snprintf (buf, MAX_STRING_LENGTH,   "%s No, I wouldn't even think of buying that.", buf2);
		do_whisper (keeper, buf, 83);
		return;
	}

	if ( !GET_TRUST (ch) && !CAN_SEE (keeper, ch) ) {
		do_say (keeper, "Who's there?", 0);
		return;
	}

	keepers_cost = obj->coppers;
    keepers_cost *= econ_discount (keeper, obj);
	keepers_cost = (int) keepers_cost;

	if ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON &&
		 obj->o.drinkcon.volume > 0 &&
		 (tobj = vtoo (obj->o.drinkcon.liquid)) &&
		 GET_ITEM_TYPE (tobj) == ITEM_FLUID ) {
		discount = econ_discount (keeper, tobj);
		keepers_cost += (tobj->coppers * obj->o.drinkcon.volume * discount);
	}

	for ( neg = keeper->shop->negotiations; neg; neg = neg->next ) {
		if ( neg->ch_coldload_id == ch->coldload_id &&
			 neg->obj_vnum == obj->virtual &&
			 !neg->true_if_buying )
			break;
	}

	if ( neg && neg->price_delta )
		keepers_cost = keepers_cost * (100 + neg->price_delta) / 100;

	name_to_ident (ch, buf2);

	*buf3 = '\0';

	snprintf (buf3, MAX_STRING_LENGTH, "%d copper%s", (int) keepers_cost,
				   (int) keepers_cost > 1 ? "s" : "");

	keepers_cost = (int) keepers_cost;

	if ( !keepers_cost )
		snprintf (buf, MAX_STRING_LENGTH,   "%s I'm afraid that isn't even worth my time...", buf2);
	else if ( neg && neg->price_delta )
		snprintf (buf, MAX_STRING_LENGTH,   "%s Remember, I said I'd buy it for %s?", buf2, buf3);
	else
		snprintf (buf, MAX_STRING_LENGTH,   "%s I'd buy %s for... %s.", buf2, obj->count > 1 ? "those" : "that", buf3);

	do_whisper (keeper, buf, 83);

	return;
}
/**
 *  Function: do_barter
 *
 *  Allows character to negotiate a better price for purchase or sale.
 *
 *		\param ch character doing the bartering
 *		\param argument sell or buy
 *		\param cmd not used
 *
 *  	Calls do_buy or do_sell with right arguments for bartering.
 *		Also prints a help line if command is in the wrong format.
 *
 */

void do_barter (CHAR_DATA *ch, char *argument, int cmd)
{
	char	buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !real_skill (ch, SKILL_BARTER) ) {
		send_to_char ("You're not convincing enough to barter, unfortunately.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( is_abbrev (buf, "buy") )
		do_buy (ch, argument, 1);
	else if ( is_abbrev (buf, "sell") )
		do_sell (ch, argument, 1);
	else
		send_to_char ("barter sell ITEM    or\n barter buy ITEM\n", ch);
		
	return;
}


/**
 *  Function: vnpc_customer
 *
 *  Simulate vnpc purchase for a shopkeeper
 *
 *		\param keeper the keeper with the virtual business
 *		\param purse amount the vnpc has to spend
 *
 *  	Simulates a purchase of random items. Shopkeeper stock is depleted,
 *		and the money is added to his storeroom.
 *
 *
 *
 */
int vnpc_customer (CHAR_DATA *keeper, int purse)
{
	ROOM_DATA	*room = NULL;
	OBJ_DATA	*tobj = NULL;
	OBJ_DATA	*drink = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			items_in_list = 0;
	int			target_item = 0;
	int			i = 0;
	int			required_check = 0;
	int			item_cost = 0;
	int			delivery_cost = 0;
	OBJ_DATA	*temp_obj = NULL;

	if ( !IS_NPC (keeper) || !IS_SET (keeper->flags, FLAG_KEEPER) || !keeper->shop )
		return purse;

	if ( IS_SET (keeper->act, ACT_NOVNPC) )
		return purse;

	if ( !(room = vtor(keeper->shop->store_vnum)) )
		return purse;

	for ( tobj = room->contents; tobj; tobj = tobj->next_content ) {
		if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL))
			continue;
		items_in_list++;
	}

	if ( !items_in_list )
		return purse;

	if ( items_in_list == 1 )
		target_item = 1;
	else target_item = number(1, items_in_list);

	for ( tobj = room->contents; tobj; tobj = tobj->next_content ) {
		if (IS_SET (tobj->obj_flags.extra_flags, ITEM_CAPITAL))
			continue;
		i++;
		if ( i == target_item )
			break;			
	}

	if ( !tobj )
		return purse;

	item_cost = tobj->coppers;

	if ( !IS_SET (tobj->obj_flags.extra_flags, ITEM_PC_SOLD) )
		item_cost *= econ_markup(keeper, tobj);
    	else item_cost *= econ_discount (keeper, tobj);

	item_cost = MAX(1, item_cost);

	if ( GET_ITEM_TYPE (tobj) == ITEM_DRINKCON &&
		tobj->o.drinkcon.volume > 0 &&
		(drink = vtoo (tobj->o.drinkcon.liquid)) &&
		GET_ITEM_TYPE (drink) == ITEM_FLUID ) {
		item_cost += drink->coppers * tobj->o.drinkcon.volume;
		if ( !IS_SET (tobj->obj_flags.extra_flags, ITEM_PC_SOLD) )
			item_cost *= econ_markup (keeper, drink);
		else item_cost *= econ_discount (keeper, drink);
	}

	if ( item_cost > VNPC_COPPER_PURSE )
		return item_cost;

	required_check = 55 - (item_cost/4);
	required_check = MAX(3, required_check);

	if ( number(1,100) <= required_check ) {
		snprintf (buf, MAX_STRING_LENGTH,   "Item VNUM: %d Cost: %d Percent Chance: %d Sold!", tobj->virtual, item_cost, required_check);
               	delivery_cost = tobj->coppers;
               	delivery_cost *= econ_markup (keeper, tobj);
               	delivery_cost *= .75;
               	delivery_cost = (int) delivery_cost;
		target_item = tobj->virtual;
		if ( tobj->count > 1 )
			tobj->count--;
		else extract_obj (tobj);
		money_to_storeroom (keeper, item_cost);
        	if ( keeper_makes (keeper, target_item) && !get_obj_in_list_num (target_item, room->contents) ) {
        			temp_obj = load_object(target_item);
                	if ( keeper_has_money (keeper, delivery_cost) ) {
                       		subtract_keeper_money (keeper, delivery_cost);
                       		obj_to_room (temp_obj, keeper->shop->store_vnum);
                	}
        	}
	}
	else snprintf (buf, MAX_STRING_LENGTH,   "Item Cost: %d Percent Chance: %d Not Sold", item_cost, required_check);

	return item_cost;
}
