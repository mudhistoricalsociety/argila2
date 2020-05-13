/*------------------------------------------------------------------------\
|  staff.c : Staff Command Module                     www.yourmud.org     | 
|  Copyright (C) 2006, Project Argila: Auroness                           |
|                                                                         |
|  All rights reserved. See Licensing documents included.                 |
|                                                                         |
|  Based upon Shadows of Isildur RPI Engine                               |
|  Copyright C. W. McHenry [Traithe], 2004                                |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <mysql/mysql.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"
#include "sys/stat.h"

extern char *player_bits[];
extern char *exit_bits[];

#define IMOTE_OPCHAR '^'

void do_guide (CHAR_DATA *ch, char *argument, int cmd)
{
	ACCOUNT_DATA	*account = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !IS_SET (ch->flags, FLAG_GUEST) ) {
		send_to_char ("Sorry, but only guests may use this command.\n", ch);
		return;
	}

	if ( ch->pc->is_guide ) {
		send_to_char ("You've already logged in as a Guide.\n", ch);
		return;
	}

	if ( !ch->desc->account || !str_cmp (ch->desc->account->name, "Guest") ) {
		send_to_char ("Sorry, but your guest login was not matched to an account.\n", ch);
		return;
	}

	account = load_account (ch->desc->account->name);

	if ( !account ) {
		send_to_char ("Sorry, but your guest login was not matched to an account.\n", ch);
		return;
	}

	if ( !is_guide (account->name) ) {
		send_to_char ("Your account has not been granted Guide status by the staff.\n", ch);
		free_account (account);
		return;
	}

	if ( !*argument ) {
		send_to_char ("You'll need to include the password to your account.\n", ch);
		free_account (account);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !check_password (buf, account->pwd) ) {
		send_to_char ("Sorry, but you've entered an incorrect password.\n", ch);
		free_account (account);
		return;
	}


	ch->pc->is_guide = TRUE;
	send_to_char ("You have successfully logged in and been granted Guide status.\n", ch);
	
	return;
}

CHAR_DATA *is_switched (CHAR_DATA *ch)
{
	CHAR_DATA *tch = NULL;

	for ( tch = character_list; tch; tch = tch->next ) {
		if ( tch->deleted )
			continue;
		if ( !tch->desc )
			continue;
		if ( tch->desc->original && tch->desc->original == ch )
			return tch;
	}

	return NULL;
}

void do_assign (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		admin [MAX_STRING_LENGTH] = {'\0'};
	char		clan [MAX_STRING_LENGTH] = {'\0'};

	if ( !*argument ) {
		send_to_char ("Usage: assign (add | remove) <admin name> (<clan name>)\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( str_cmp (buf, "add") && str_cmp (buf, "remove") ) {
		send_to_char ("Usage: assign (add | remove) <admin name> (<clan name>)\n", ch);
		return;
	}

	argument = one_argument (argument, admin);
	argument = one_argument (argument, buf2);

	mysql_real_escape_string (database, clan, buf2, strlen(buf2));

	if ( !*admin ) {
		send_to_char ("Usage: assign (add | remove) <admin name> (<clan name>)\n", ch);
		return;
	}

	if ( islower (*admin) )
		*admin = toupper (*admin);

	if ( *clan && islower (*clan) )
		*clan = toupper (*clan);

	if ( !str_cmp (buf, "add") ) {
		mysql_safe_query ("INSERT INTO clan_assignments VALUES ('%s', '%s')", clan, admin);
		send_to_char ("The specified assignment has been added to the database.\n", ch);
		return;
	}
	else if ( !str_cmp (buf, "remove") ) {
		if ( *clan )
			mysql_safe_query ("DELETE FROM clan_assignments WHERE clan_name = '%s' AND imm_name = '%s'", clan, admin);
		else
			mysql_safe_query ("DELETE FROM clan_assignments WHERE imm_name = '%s'", admin);
		
		if ( *clan )
			send_to_char ("The specified assignment has been removed from the database.\n", ch);
		else
			send_to_char ("All clan assignments for the specified admin have been removed.\n", ch);
		return;
	}

	send_to_char ("Usage: assign (add | remove) <admin name> <clan name>\n", ch);
	return;
}

void do_roster (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*tch = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		admin [MAX_STRING_LENGTH] = {'\0'};
	char		title [MAX_STRING_LENGTH] = {'\0'};

	if ( !*argument ) {
		send_to_char ("Usage: roster (add | remove) <admin name> (<title>)\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( str_cmp (buf, "add") && str_cmp (buf, "remove") ) {
		send_to_char ("Usage: roster (add | remove) <admin name> (<title>)\n", ch);
		return;
	}

	argument = one_argument (argument, admin);
	argument = one_argument (argument, buf2);

	mysql_real_escape_string (database, title, buf2, strlen(buf2));

	if ( !*admin || (!*title && !str_cmp (buf, "add")) ) {
		send_to_char ("Usage: roster (add | remove) <admin name> (<title>)\n", ch);
		return;
	}

	if ( islower (*admin) )
		*admin = toupper (*admin);

	if ( islower (*title) )
		*title = toupper (*title);

	if ( !str_cmp (buf, "add") && !(tch = load_pc (admin)) ) {
		send_to_char ("That PC could not be found in our database.\n", ch);
		return;
	}

	if ( !str_cmp (buf, "add") && tch->pc->level < 3 ) {
		send_to_char ("All staff listed on the roster must be at least level 3.\n", ch);
		return;
	}

	if ( tch )
		unload_pc (tch);

	if ( !str_cmp (buf, "add") ) {
		mysql_safe_query ("INSERT INTO staff_roster VALUES ('%s', '%s', %d)", admin, title, (int)time(0));
		send_to_char ("The specified individual has been added to the staff roster.\n", ch);
		return;
	}
	else if ( !str_cmp (buf, "remove") ) {
		mysql_safe_query ("DELETE FROM staff_roster WHERE name = '%s'", admin);
		mysql_safe_query ("DELETE FROM clan_assignments WHERE imm_name = '%s'", admin);
		send_to_char ("The specified individual has been removed from the roster.\n", ch);
		return;
	}

	send_to_char ("Usage: roster (add | remove) <admin name> (<title>)\n", ch);
	return;
}

void do_nuke (CHAR_DATA *ch, char *argument, int cmd)
{
	ACCOUNT_DATA	*account = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};

	if ( !*argument ) {
		send_to_char ("Nuke which account?\n", ch);
		return;
	}

	*argument = toupper (*argument);
	argument = one_argument (argument, buf);

	if ( !(account = load_account (buf)) ) {
		send_to_char ("There is no account by that name in our database.\n", ch);
		return;
	}

	if ( GET_TRUST (ch) < 5 && account->admin ) {
		send_to_char ("Only level 5 admins may delete other admin accounts.\n", ch);
		return;
	}

	if ( !str_cmp (buf, IMPLEMENTOR_ACCOUNT) ) {
		send_to_char ("Naughty, naughty...\n", ch);
		return;
	}

	if ( !*argument || *argument != '!' ) {
		snprintf (buf2, MAX_STRING_LENGTH,  "Please type \'nuke %s !\', sans quotes, to confirm.\n", buf);
		send_to_char (buf2, ch);
		return;
	}

	mysql_safe_query ("DELETE FROM forum_users WHERE username = '%s'", buf);

	snprintf (buf2, MAX_STRING_LENGTH,  "Account %s has been excised from the database.\n", buf);
	send_to_char (buf2, ch);
	
	return;
}

void do_register (CHAR_DATA *ch, char *argument, int cmd)
{
	ACCOUNT_DATA	*account = NULL;
	int				i = 0;
	char			name [MAX_STRING_LENGTH] = {'\0'};
	char			email [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, name);
	argument = one_argument (argument, email);

	if ( !*name || !*email ) {
		send_to_char ("Usage: register <new account name> <email address>\n", ch);
		return;
	}

	if ( strlen(name) > 36 ) {
		send_to_char ("The account name must be 36 characters or less.\n", ch);
		return;
	}

	if ( !isalpha (*name) ) {
		send_to_char ("The first character of the account name MUST be a letter.\n", ch);
		return;
	}

	for ( i = 0; i < strlen(name); i++ ) {
		if ( !isalpha (name [i]) && !isdigit (name [i]) ) {
			send_to_char ("Account names may only contain letters and numbers.\n", ch);
			return;
		}
	}

	*name = toupper (*name);

	if ( !str_cmp (name, "Anonymous") || !str_cmp (name, IMPLEMENTOR_ACCOUNT) ) {
		send_to_char ("That account name cannot be registered.\n", ch);
		return;
	}

	if ( (account = load_account (name)) ) {
		send_to_char ("That account name has already been taken.\n", ch);
		return;
	}

	if ( !strstr (email, "@") || !strstr (email, ".") ) {
		send_to_char ("Email addresses must be given in the form of \'address@domain.com\'.\n", ch);
		return;
	}

	new_accounts++;
	mysql_safe_query ("UPDATE newsletter_stats SET new_accounts=new_accounts+1");

	CREATE (account, ACCOUNT_DATA, 1);

	account->email = str_dup (email);
	account->name = str_dup (name);
	account->created_on = time(0);

	setup_new_account(account);

	send_to_char ("The new account has been registered, and notification has been emailed.\n", ch);
	
	return;
}

void post_log (DESCRIPTOR_DATA *d)
{
	CHAR_DATA	*ch = NULL;
	char		report [MAX_STRING_LENGTH] = {'\0'};

    ch = d->character;
    if ( !*d->pending_message->message ) {
    	send_to_char ("No newsletter report posted.\n", ch);
        return;
    }
        
	mysql_real_escape_string (database, report, d->pending_message->message, strlen(d->pending_message->message));

	if ( !str_cmp (ch->delay_who, "blog") ) {
		mysql_safe_query ("INSERT INTO building_log VALUES ('%s', '%s', %d)", ch->tname,
		report, (int) time(0));
		send_to_char ("Your building report has been filed.\n", ch);
		return;
	}
	
	else if ( !str_cmp (ch->delay_who, "clog") ) {
		mysql_safe_query ("INSERT INTO coding_log VALUES ('%s', '%s', %d)", ch->tname,
		report, (int) time(0));
		send_to_char ("Your coding report has been filed.\n", ch);
		return;
	}
	
	else if ( !str_cmp (ch->delay_who, "plog") ) {
		mysql_safe_query ("INSERT INTO plot_log VALUES ('%s', '%s', %d)", ch->tname,
		report, (int) time(0));
		send_to_char ("Your plot report has been filed.\n", ch);
		return;
	}
	
	else if ( !str_cmp (ch->delay_who, "alog") ) {
		mysql_safe_query ("INSERT INTO announcements VALUES ('%s', '%s', %d)", ch->tname,
		report, (int) time(0));
		send_to_char ("Your announcement has been filed.\n", ch);
		return;
	}
	
	else if ( !str_cmp (ch->delay_who, "wlog") ) {
		mysql_safe_query ("INSERT INTO website_log VALUES ('%s', '%s', %d)", ch->tname,
		report, (int) time(0));
		send_to_char ("Your website update has been filed.\n", ch);
		return;
	}
	
	else send_to_char ("There seems to be a problem with this command.\n", ch);

	ch->delay_who = NULL;
	unload_message (d->pending_message);
	
	return;
}

void do_clog (CHAR_DATA *ch, char *argument, int cmd)
{
	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	
	send_to_char ("Enter a coding report to be included in the next newsletter:\n", ch);

    make_quiet (ch);

    CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);

    ch->desc->str = &ch->desc->pending_message->message;
    ch->desc->max_str = MAX_STRING_LENGTH;

	ch->delay_who = str_dup ("clog");

    ch->desc->proc = post_log;
    
    return;
}

void do_alog (CHAR_DATA *ch, char *argument, int cmd)
{
	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	send_to_char ("Enter a general staff announcement to be included in the next newsletter:\n", ch);

	make_quiet (ch);

	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);

	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;

	ch->delay_who = str_dup ("alog");

	ch->desc->proc = post_log;

	return;
}

void do_blog (CHAR_DATA *ch, char *argument, int cmd)
{
	
	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	send_to_char ("Enter a building report to be included in the next newsletter:\n", ch);
	
	make_quiet (ch);
	
	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	
	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	
	ch->delay_who = str_dup ("blog");
	
	ch->desc->proc = post_log;
	
	return;
}

void do_wlog (CHAR_DATA *ch, char *argument, int cmd)
{
	
	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	send_to_char ("Enter a website report to be included in the next newsletter:\n", ch);
	
	make_quiet (ch);
	
	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	
	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	
	ch->delay_who = str_dup ("wlog");
	
	ch->desc->proc = post_log;
	
	return;
}

void do_plog (CHAR_DATA *ch, char *argument, int cmd)
{
	
	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	send_to_char ("Enter a plot report to be included in the next newsletter:\n", ch);
	
	make_quiet (ch);
	
	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	
	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	
	ch->delay_who = str_dup ("plog");
	
	ch->desc->proc = post_log;
	
	return;
}

void do_wclone (CHAR_DATA *ch, char *argument, int cmd)
{
	char	    	buf [MAX_STRING_LENGTH] = {'\0'};
	OBJ_DATA	    *fbook = NULL;
	OBJ_DATA		*tbook = NULL;
	WRITING_DATA	*fwriting = NULL;
	WRITING_DATA	*twriting = NULL;
	char			*temp_arg1 = NULL;
	char			*temp_arg2 = NULL;

	argument = one_argument (argument, buf);
	fbook = get_obj_in_list_vis (ch, buf, ch->right_hand);

	if ( !fbook )
		fbook = get_obj_in_list_vis (ch, buf, ch->left_hand);

	if ( !fbook )
		fbook = get_obj_in_list_vis (ch, buf, ch->room->contents);

	if ( !fbook ) {
		send_to_char ("Which written object did you wish to clone from?\n", ch);
		return;
	}
	
	if ( GET_ITEM_TYPE (fbook) != ITEM_PARCHMENT &&
		 GET_ITEM_TYPE (fbook) != ITEM_BOOK ) {
		send_to_char ("The object you want to clone from isn't a parchment or book.\n", ch);
		return;
	}

	if ( !fbook->writing_loaded )
		load_writing (fbook);

	argument = one_argument (argument, buf);
	tbook = get_obj_in_list_vis (ch, buf, ch->right_hand);

	if ( !tbook )
		tbook = get_obj_in_list_vis (ch, buf, ch->left_hand);

	if ( !tbook )
		tbook = get_obj_in_list_vis (ch, buf, ch->room->contents);

	if ( !tbook ) {
		send_to_char ("Which written object did you wish to clone to?\n", ch);
		return;
	}
	
	if ( GET_ITEM_TYPE (fbook) != ITEM_PARCHMENT &&
		 GET_ITEM_TYPE (fbook) != ITEM_BOOK ) {
		send_to_char ("The object you want to clone to isn't a parchment or book.\n", ch);
		return;
	}

	if ( GET_ITEM_TYPE (fbook) != GET_ITEM_TYPE (tbook) ) {
		send_to_char ("This command only works with two written objects of the same type.\n", ch);
		return;
	}

	if ( !fbook->writing ) {
		send_to_char ("The specified source has no writing in it.\n", ch);
		return;
	}

	if ( fbook == tbook ) {
		send_to_char ("Source and destination can't be the same.\n", ch);
		return;
	}

	for ( fwriting = fbook->writing; fwriting; fwriting = fwriting->next_page ) {
		if ( fwriting == fbook->writing ) {
			if ( !tbook->writing ){
				CREATE (tbook->writing, WRITING_DATA, 1);
			}
			
			twriting = tbook->writing;
			twriting->message = add_hash(fwriting->message);
			twriting->author = add_hash(fwriting->author);
			twriting->date = add_hash (fwriting->date);
			twriting->ink = add_hash (fwriting->ink);
			twriting->script = fwriting->script;
			twriting->skill = fwriting->skill;
			twriting->torn = FALSE;
			twriting->language = fwriting->language;
			twriting->next_page = NULL;
		}
		else for ( twriting = tbook->writing; twriting; twriting = twriting->next_page ) {
			if ( !twriting->next_page ) {
				CREATE (twriting->next_page, WRITING_DATA, 1);
				twriting->next_page->message = add_hash(fwriting->message);
				twriting->next_page->author = add_hash(fwriting->author);
				twriting->next_page->date = add_hash (fwriting->date);
				twriting->next_page->ink = add_hash (fwriting->ink);
				twriting->next_page->script = fwriting->script;
				twriting->next_page->skill = fwriting->skill;
				twriting->next_page->torn = FALSE;
				twriting->language = fwriting->language;
				twriting->next_page->next_page = NULL;
				break;
			}
		}
	}

	tbook->o.od.value[0] = fbook->o.od.value[0];

	if ( fbook->book_title && *fbook->book_title ) {
		tbook->book_title = add_hash(fbook->book_title);
		tbook->title_skill = fbook->title_skill;
		tbook->title_script = fbook->title_script;
		tbook->title_language = fbook->title_language;
	}

	temp_arg1 = obj_short_desc(fbook);
	temp_arg2 = obj_short_desc(tbook);
	snprintf (buf, MAX_STRING_LENGTH,  "Writing cloned from #2%s#0 to #2%s#0.", temp_arg1, temp_arg2);
	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

	save_writing (tbook);

	return;
}

void do_wizlock (CHAR_DATA *ch, char *argument, int cmd)
{
	if ( maintenance_lock ) {
		unlink ("maintenance_lock");
	}
	else {
		system ("touch maintenance_lock");
	}
	
	send_to_char ("Wizlock toggled.\n", ch);
}

void do_stayput (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*tch = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !*argument ) {
		send_to_char ("Toggle who's stayput flag?\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("They aren't here.\n", ch);
		return;
	}

	if ( !IS_NPC (tch) ) {
		send_to_char ("This command is for use on NPCs only.\n", ch);
		return;
	}

	if ( port != PLAYER_PORT ) {
		send_to_char ("This command is for use on the player port only.\n", ch);
		return;
	}

	if ( IS_SET (tch->act, ACT_STAYPUT) ) {
		send_to_char ("This mobile will no longer save over reboots.\n", ch);
		REMOVE_BIT (tch->act, ACT_STAYPUT);
		return;
	}
	else {
		send_to_char ("This mobile will now save over reboots.\n", ch);
		SET_BIT (tch->act, ACT_STAYPUT);
		save_stayput_mobiles();
		return;
	}
	
	return;
}

void delete_role (CHAR_DATA *ch, char *argument)
{
	int			i = 1;
	ROLE_DATA	*role = NULL;
	bool		deleted = FALSE;

	if ( !isdigit (*argument) ) {
		send_to_char ("Syntax: role display <role number>\n", ch);
		return;
	}

	if ( strtol(argument, NULL, 10) == 1 ) {
		if ( GET_TRUST(ch) == 4 && str_cmp (role_list->poster, ch->tname) ) {
			send_to_char ("Sorry, but you are only allowed to delete your own roles.\n", ch);
			return;
		}
		
		if ( !role_list->next )
			role_list = NULL;
		else
			role_list = role_list->next;
		
		deleted = TRUE;
	}
	else {
		for ( role = role_list, i = 1; role && role->next; role = role->next, i++ ) {
			if ( i + 1 == strtol(argument, NULL, 10)) {
				if ( GET_TRUST(ch) == 4 && str_cmp (role->next->poster, ch->tname) ) {
					send_to_char ("Sorry, but you are only allowed to delete your own roles.\n", ch);
					return;
				}
			
				role->next = role->next->next;
				deleted = TRUE;
				break;
			}
		}
	}

	if ( !deleted ) {
		send_to_char ("That role number was not found.\n", ch);
		return;
	}

	send_to_char ("The specified special role has been removed as an option from chargen.\n", ch);
	save_roles();
	
	return;
}

void display_role (CHAR_DATA *ch, char *argument)
{
	char	output [MAX_STRING_LENGTH] = {'\0'};
	int		i = 1;
	ROLE_DATA	*role;

	if ( !isdigit (*argument) ) {
		send_to_char ("Syntax: role display <role number>\n", ch);
		return;
	}

	for ( role = role_list; role; role = role->next ) {
		if ( i == strtol(argument, NULL, 10))
			break;
		i++;
	}

	if ( !role ) {
		send_to_char ("That role number was not found.\n", ch);
		return;
	}

	snprintf (output, MAX_STRING_LENGTH,
			"\n#2Role Summary:#0  %s\n"
            "#2Posting Admin:#0 %s\n"
            "#2Posted On:#0     %s\n"
            "#2Point Cost:#0    %d\n"
			"\n"
			"%s", role->summary, role->poster, role->date, role->cost, role->body);

	page_string (ch->desc, output);
	
	return;
}

void list_roles (CHAR_DATA *ch)
{
	char		output [MAX_STRING_LENGTH] = {'\0'};
	ROLE_DATA	*role = NULL;
	int		i = 1;

	*output = '\0';

	for ( role = role_list; role; role = role->next ) {
		snprintf (output + strlen(output), MAX_STRING_LENGTH,  "%6d. %s\n", i, role->summary);
		i++;
	}

	if ( !*output ) {
		send_to_char ("No special roles are currently available in chargen.\n", ch);
		return;
	}

	send_to_char ("\n#2The following special roles have been made available in chargen:\n#0", ch);
	page_string (ch->desc, output);
	
	return;
}

/*                                                                          *
 * funtion: post_role                   < e.g.> role new 2 clothier         *
 *                                                                          *
 *                                                                          */
void post_role (DESCRIPTOR_DATA *d)
{
	CHAR_DATA	*ch = NULL;
	ROLE_DATA	*role = NULL;
	char		*date = NULL;

	ch = d->character;

	if ( !*ch->pc->msg ) {
		send_to_char ("Role post aborted.\n", ch);
		return;
	}

	if ( !role_list ) {
		CREATE (role_list, ROLE_DATA, 1);
		role = role_list;
	}
	else{
		for ( role = role_list; role; role = role->next ) {
			if ( !role->next ) {
				CREATE (role->next, ROLE_DATA, 1);
				role = role->next;
				break;
			}
		}
	}

	/* Get a date string from current time ( default = "" ) */
	date = timestr(date);
	role->cost = ch->delay_info1;
	role->summary = str_dup (ch->delay_who);
	role->body = str_dup (ch->pc->msg);
	role->date = str_dup (date);
	role->poster = str_dup (ch->pc->account);
	role->timestamp = (int) time(0);

	ch->delay_who = NULL;
	ch->delay_info1 = 0;

	save_roles ();

	mem_free (date);
	
	return;
}

void new_role (CHAR_DATA *ch, char *argument)
{
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	int		cost = 0;

	argument = one_argument (argument, buf);

	if ( IS_NPC (ch) ) {
		send_to_char ("Please don't post roles while switched. Thanks!\n", ch);
		return;
	}

	if ( !isdigit (*buf) ) {
		send_to_char ("Syntax: role new <point cost> <summary line>\n", ch);
		return;
	}
	
	cost = strtol(buf, NULL, 10);

	if ( cost < 1 || cost > 50 ) {
		send_to_char ("Permissible costs are from 1-50, inclusive.\n", ch);
		return;
	}

	if ( !*argument ) {
		send_to_char ("Syntax: role new <point cost> <summary line>\n", ch);
		return;
	}

	send_to_char ("\n#2Enter a detailed summary of the role you wish to post, to\n"
        	      "give prospective players a better idea as to what sort of RP\n"
              	      "will be required to successfully portray it.#0\n", ch);

	make_quiet (ch);
	ch->delay_info1 = cost;
	ch->delay_who = str_dup (argument);
	ch->pc->msg = (char *)alloc((int)sizeof(char), 1);
	*ch->pc->msg = '\0';
	ch->desc->str = &ch->pc->msg;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->desc->proc = post_role;
}

void update_role (CHAR_DATA *ch, char *argument)
{
	ROLE_DATA	*role = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			i = 0;

	argument = one_argument (argument, buf);

	if ( !*buf || !isdigit (*buf) ) {
		send_to_char ("Please specify a role number to update.\n", ch);
		return;
	}

	if ( strtol(buf, NULL, 10) == 1 ) {
		role = role_list;
	}
	else for ( role = role_list, i = 1; role; role = role->next, i++ ) {
		if ( i == strtol(buf, NULL, 10))
			break;
	}

	if ( !role ) {
		send_to_char ("I could not find that role number to update.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Please specify which field in the role to update.\n", ch);
		return;
	}
	else if ( !str_cmp (buf, "contact") ) {
		argument = one_argument (argument, buf);
		if ( !*buf ) {
			send_to_char ("Please specify who the new point of contact should be.\n", ch);
			return;
		}
		if ( !load_account (buf) ) {
			send_to_char ("That is not the name of a currently registered MUD account.\n", ch);
			return;
		}
		mem_free (role->poster);
		role->poster = str_dup (CAP(buf));
		send_to_char ("The point of contact for that role has been updated.\n", ch);
		save_roles();
	}
	else if ( !str_cmp (buf, "cost") ) {
		argument = one_argument (argument, buf);
		if ( !*buf || !isdigit (*buf) ) {
			send_to_char ("Please specify what the new cost should be.\n", ch);
			return;
		}
		role->cost = strtol(buf, NULL, 10);
		send_to_char ("The cost for that role has been updated.\n", ch);
		save_roles();
	}
}

void do_role (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( IS_NPC(ch) ) {
		send_to_char ("Sorry, but this can't be done while switched.\n", ch);
		return;
	}

	if ( !str_cmp (buf, "new") )
		new_role (ch, argument);
	else if ( !str_cmp (buf, "list") )
		list_roles (ch);
	else if ( !str_cmp (buf, "display") )
		display_role (ch, argument);
	else if ( !str_cmp (buf, "delete") )
		delete_role (ch, argument);
	else if ( !str_cmp (buf, "update") )
		update_role (ch, argument);
	else
		send_to_char ("Syntax: role (new|list|display|delete|update) <argument(s)>\n", ch);
}

void do_clockout (CHAR_DATA *ch, char *argument, int cmd)
{
	if ( !IS_NPC(ch) ) {
		send_to_char ("Eh?\n", ch);
		return;
	}

	if ( port == BUILDER_PORT )
		return;

	if ( !ch->shop ) {
		send_to_char ("This command is only for NPC shopkeepers.\n", ch);
		return;
	}

	REMOVE_BIT (ch->flags, FLAG_KEEPER);
	send_to_char ("You will no longer sell things until you 'clockin'.\n", ch);
} 

void do_clockin (CHAR_DATA *ch, char *argument, int cmd)
{
	if ( !IS_NPC(ch) ) {
		send_to_char ("Eh?\n", ch);
		return;
	}

	if ( port == BUILDER_PORT )
		return;

	if ( !ch->shop ) {
		send_to_char ("This command is only for NPC shopkeepers.\n", ch);
		return;
	}

	SET_BIT (ch->flags, FLAG_KEEPER);
	send_to_char ("You are now open for business again.\n", ch);
} 

void do_roll (CHAR_DATA *ch, char *argument, int cmd)
{
	int		die, number, total;
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	
	*buf = '\0';

	argument = one_argument (argument, buf);

	if ( !*buf || isalpha(*buf) || strtol(buf, NULL, 10) > 1000 || strtol(buf, NULL, 10) < 1 ) {
		send_to_char ("Usage: 'roll X Y', equivalent to rolling XdY.\n", ch);
		return;
	}

	die = strtol(buf, NULL, 10);

	argument = one_argument (argument, buf);

	if ( !*buf || isalpha(*buf) || strtol(buf, NULL, 10) > 1000 || strtol(buf, NULL, 10) < 1 ) {
		send_to_char ("Usage: 'roll X Y', equivalent to rolling XdY.\n", ch);
		return;
	}

	number = strtol(buf, NULL, 10);

	total = dice(die,number);

	snprintf (buf, MAX_STRING_LENGTH,  "Rolling %dd%d... #2%d#0\n", die,number,total);
	send_to_char (buf, ch);
}
	
void do_deduct (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch = NULL;
	ACCOUNT_DATA	*account = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	bool			loaded = FALSE;

	if ( !*argument ) {
		send_to_char ("Who would you like to deduct?\n", ch);
		return;
	}

	if ( IS_NPC (ch) ) {
		send_to_char ("This is a PC-only command.\n", ch);
		return;
	}

	argument = one_argument(argument, buf);

	if ( !(tch = get_char_room_vis (ch, buf)) &&
			  !(tch = get_char_vis (ch, buf)) ) {
                if ( !(tch = load_pc (buf)) ) {
                        send_to_char ("No PC with that name was found. Is it spelled correctly?\n", ch);
                        return;
                }
                else loaded = TRUE;
	}

        account = load_account (tch->pc->account);

	if ( !account ) {
		act ("Hmm... there seems to be a problem with that character's account. Please notify the staff.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !isdigit(*buf) ) {
		send_to_char ("You must specify a number of roleplay points to deduct.\n", ch);
		return;
	}

	if ( strtol(buf, NULL, 10) < 1 || strtol(buf, NULL, 10) > account->roleplay_points ) {
		act ("The specified number must be greater than 0 and less than or equal to the named character's RP point total.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	account->roleplay_points -= strtol(buf, NULL, 10);
	account->last_rpp = (int) time(0);
	save_account (account);

	tch->pc->last_rpp = (int) time(0);
	save_char (tch, FALSE);

	if ( loaded )
		unload_pc (tch);

	send_to_char ("#1The specified number of points have been removed from their account.\n", ch);
	send_to_char ("Please be sure to leave a note explaining why on their pfile.#0\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "write %s Roleplay Point Deduction.", tch->tname);
	command_interpreter(ch, buf);

}

void do_award (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	ACCOUNT_DATA	*account = NULL;
	bool			loaded = FALSE;

	if ( !*argument ) {
		send_to_char ("Who would you like to award?\n", ch);
		return;
	}

	if ( IS_NPC(ch) ) {
		send_to_char ("This is a PC-only command.\n", ch);
		return;
	}

	argument = one_argument(argument, buf);

	if ( !(tch = get_char_room_vis (ch, buf)) &&
			  !(tch = get_char_vis (ch, buf)) ) {
		if ( !(tch = load_pc (buf)) ) {
			send_to_char ("No PC with that name was found. Is it spelled correctly?\n", ch);
			return;
		}
		else loaded = TRUE;
	}

	if ( IS_NPC (tch) ) {
		send_to_char ("This command only works for player characters.\n", ch);
		return;
	}

	account = load_account (tch->pc->account);

	if ( !account ) {
		act ("Hmm... there seems to be a problem with that character's account. Please notify the staff.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	if ( !str_cmp (tch->pc->account, ch->pc->account) && port != TEST_PORT ) {
		send_to_char ("Adding roleplay points to your own account is against policy.\n", ch);
		return;
	}

	if ( tch->desc && tch->desc->account && IS_SET (tch->desc->account->flags, ACCOUNT_RPPDISPLAY) )
		send_to_char ("#6Congratulations, you've just been awarded a roleplay point!#0\n", tch);

	account->roleplay_points++;
	account->last_rpp = (int) time(0);
	save_account (account);

	tch->pc->last_rpp = (int) time(0);
	save_char (tch, FALSE);

	if ( loaded )
		unload_pc (tch);

	send_to_char ("#2A roleplay point has been added to that character's account.\n", ch);
	send_to_char ("Please be sure to leave a note explaining why on their pfile.#0\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "write %s Roleplay Point.", tch->tname);
	command_interpreter(ch, buf);

}

void post_email (DESCRIPTOR_DATA *d)
{
	ACCOUNT_DATA	*account = NULL;
	char		from [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !*d->pending_message->message ) {
		send_to_char ("Email aborted.\n", d->character);
		d->pending_message = NULL;
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "%s", d->pending_message->message);
	d->pending_message = NULL;

	snprintf (from, MAX_STRING_LENGTH, "%s <%s>", d->character->tname, d->account->email);

	account = load_account (d->character->delay_who);

	send_email (account, d->account->email, from, d->character->delay_who2, buf);

	REMOVE_BIT (d->character->act, PLR_QUIET);

	send_to_char ("Your email has been sent out. It has been copied to your address for reference.\n", d->character);

	d->character->delay_who = NULL;
	d->character->delay_who2 = NULL;
}

void do_email (CHAR_DATA *ch, char *argument, int cmd)
{
	ACCOUNT_DATA	*account = NULL;
	CHAR_DATA		*tch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("To whom did you wish to send an email?\n", ch);
		return;
	}

	if ( !(tch = load_pc (buf)) ) {
		send_to_char ("That PC's file could not be loaded.\n", ch);
		return;
	}

	if ( !(account = load_account (tch->pc->account)) ) {
		send_to_char ("That character's account could not be loaded.\n", ch);
		unload_pc (tch);
		return;
	}

	ch->delay_who = str_dup (account->name);
	unload_pc (tch);

	if ( !*argument ) {
		send_to_char ("What would you like the subject-line of the email to be?\n", ch);
		return;
	}

	if ( IS_NPC (ch) ) {
		send_to_char ("Only PCs may use this command.\n", ch);
		return;
	}

	send_to_char ("\n#2Enter the message you'd like to email this player:#0\n", ch);

	CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
	ch->desc->str = &ch->desc->pending_message->message;
	ch->desc->max_str = MAX_STRING_LENGTH;
	ch->desc->proc = post_email;
	ch->delay_who2 = str_dup(argument);
	SET_BIT (ch->act, PLR_QUIET);

}

void do_unban (CHAR_DATA *ch, char *argument, int cmd)
{
	SITE_INFO	*site = NULL;
	SITE_INFO	*next_site;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int		i = 0;

	if ( !*argument || !isdigit(*argument) ) {
		send_to_char ("You must specify the number of the site from the banned sitelist.\n", ch);
		return;
	}

	if ( !banned_site ) {
		send_to_char ("There are currently no sites to unban!\n", ch);
		return;
	}

	for ( i = 1, site = banned_site; site; i++, site = next_site ) {
		next_site = site->next;
		if ( strtol(argument, NULL, 10) == 1 && i == 1 ) {
			if ( GET_TRUST(ch) < 5 && str_cmp (site->banned_by, "Password Security System") && str_cmp (ch->tname, site->banned_by) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Sorry, but you'll need to get %s or a level 5 admin to do that.\n", site->banned_by);
				send_to_char (buf, ch);
				return;
			}
			unban_site (site);
			return;
		}
		else if ( i+1 == strtol(argument, NULL, 10) && site->next ) {
			if ( GET_TRUST(ch) < 5 && str_cmp (ch->tname, site->next->banned_by) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Sorry, but you'll need to get %s or a level 5 admin to do that.\n", site->banned_by);
				send_to_char (buf, ch);
				return;
			}
			unban_site (site->next);
			return;
		}
	}

	send_to_char ("That number is not on the banned site list.\n", ch);

}

void disconnect_banned_hosts()
{
	SITE_INFO	*site = NULL;
	DESCRIPTOR_DATA		*d = NULL;
	DESCRIPTOR_DATA		*next_desc = NULL;

	for ( site = banned_site; site; site = site->next ) {
       		for ( d = descriptor_list; d; d = next_desc ) {
        	        next_desc = d->next;
        	        if ( !d->account )
        	                continue;
        	        if ( !IS_SET(d->account->flags, ACCOUNT_NOBAN) ) {
        	                if ( strstr(d->host, site->name) ) {
        	                        close_socket(d);
        	                }
        	        }
        	}
	}
}

void ban_host (char *host, char *banned_by, int length)
{
	SITE_INFO	*site = NULL;
	SITE_INFO	*tmp_site = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	CREATE (site, SITE_INFO, 1);
	site->name = add_hash(host);
	site->banned_by = add_hash(banned_by);
	site->banned_on = time(0);
	if ( length != -1 && length != -2 )
		site->banned_until = time(0) + (60 * 60 * 24 * length);
	else if ( length == -2 )
		site->banned_until = time(0) + (60 * 60);
	else if ( length == -1 )
		site->banned_until = -1;

	if ( !banned_site )
		banned_site = site;
	else for ( tmp_site = banned_site; tmp_site; tmp_site = tmp_site->next ) {
		if ( !tmp_site->next ) {
			tmp_site->next = site;
			break;
		}
	}

	snprintf (buf, MAX_STRING_LENGTH,  "%s has sitebanned %s.\n", banned_by, host);
	send_to_gods (buf);

	save_banned_sites();
	
	return;
}

/*                                                                          *
 * funtion: do_ban                      < e.g.> ban [ <host> <duration> ]   *
 *                                                                          *
 *                                                                          */
void do_ban (CHAR_DATA *ch, char *argument, int cmd)
{
	SITE_INFO	*site = NULL;
    MYSQL_RES   *result = NULL;
    MYSQL_ROW   row;
	char		*start_date = NULL;
	char		*end_date = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		host [MAX_STRING_LENGTH] = {'\0'};
	char		length [MAX_STRING_LENGTH] = {'\0'};
	char		accounts [MAX_STRING_LENGTH] = {'\0'};
	int			i = 0;
	int			j = 0;
	time_t		ban_time, end_ban_time;

	if ( !*argument ) {
		snprintf (buf, MAX_STRING_LENGTH,  "\n#6Currently Banned Sites:#0\n\n");
		if ( !banned_site ) {
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   None.\n");
			send_to_char (buf, ch);
			return;
		}
		else for ( i = 1, site = banned_site; site; i++, site = site->next ) {
			*accounts = '\0';
			mysql_safe_query ("SELECT username,account_flags FROM forum_users WHERE user_last_ip LIKE '%%%s%%'", site->name);
			result = mysql_store_result (database);
			j = 1;
			if ( result ) {
				if ( mysql_num_rows(result) >= 10 )
					snprintf (accounts + strlen(accounts), MAX_STRING_LENGTH, " #110+ accounts affected#0");
				else while ( (row = mysql_fetch_row(result)) ) {
					if ( IS_SET (strtol(row[1], NULL, 10), ACCOUNT_NOBAN) )
						continue;
					if ( j != 1 )
						snprintf (accounts + strlen(accounts), MAX_STRING_LENGTH, ",");
					snprintf (accounts + strlen(accounts), MAX_STRING_LENGTH, " %s", row[0]);
					j++;
				}
				mysql_free_result (result);
			}
			if ( i != 1 )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   %d. #6%s#0 [%s]\n", i, site->name, site->banned_by);
			if ( i > 9 )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " ");
			if ( *accounts )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "      #2Accounts:#0%s\n", accounts);
			else
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "      #2Accounts:#0 None Found\n");

			ban_time = site->banned_on;
			start_date = (char *) malloc (256) ;
			if ( asctime_r ( localtime ( &ban_time ), start_date ) != NULL ) {
			  start_date [ strlen ( start_date ) - 1 ] = '\0';
			}
			else {
			  start_date[0] = '\0';
			}

			if ( site->banned_until != -1 ) {
			  if ( i > 9 )
			    snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " ");
			  snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "      #2Ban Period:#0 %s to ", start_date);

			  end_ban_time = site->banned_until;
			  end_date = (char *) malloc (256) ;
			  if ( asctime_r ( localtime ( &end_ban_time ), end_date ) != NULL ) {
			    end_date [ strlen ( end_date ) - 1 ] = '\0';
			  }
			  else {
			    end_date[0] = '\0';
			  }

			  snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s\n", end_date);
			  mem_free ( end_date ) ;
			  end_date = NULL;
			} 
			else {
			  if ( i > 9 )
			    snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " ");
			  snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "      #2Permanently Banned On:#0 %s\n", start_date);
			}
			mem_free ( start_date );
			start_date = NULL;
		}
		page_string (ch->desc, buf);
		return;
	}

	argument = one_argument (argument, host);
	argument = one_argument (argument, length);

	for ( i = 0; i < strlen (host); i++ ) {
		if ( !isalpha (host [i]) && !isdigit (host[i]) &&
			host[i] != '.' && host[i] != '_' &&
			host[i] != '-' && host[i] != '*' ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Illegal character '%c' in siteban entry.\n", host[i]);
			send_to_char (buf, ch);
			return;
		}
	}

	if ( !*length || (!isdigit (*length) && *length != '!') ) {
		send_to_char ("You need to specify a length of days; otherwise, '!' for a permanent ban.\n", ch);
		return;
	}

	if ( *length != '!' && strtol(length, NULL,10) < 1 ) {
		send_to_char ("The minimum ban period is 1 day.\n", ch);
		return;
	}

	if ( *length == '!' )
		ban_host (host, ch->tname, -1);
	else ban_host (host, ch->tname, strtol(length, NULL,10));

	disconnect_banned_hosts();
}

void do_echo(CHAR_DATA *ch, char *argument, int cmd)
{
	int 	i = 0;
	char 	buf[MAX_STRING_LENGTH] = {'\0'};

	for (i = 0; *(argument + i) == ' '; i++);

	if ( !*(argument + i) ) {
		send_to_char("That must be a mistake...\n", ch);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "%s\n", argument + i);
	send_to_room (buf, ch->in_room);
}

void do_gecho(CHAR_DATA *ch, char *argument, int cmd)
{
	int 		i = 0;
	ROOM_DATA   *troom = NULL;

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
		send_to_char("That must be a mistake...\n", ch);

	else{
		snprintf (s_buf, MAX_STRING_LENGTH, "%s\n", argument + i);

		for (troom = full_room_list; troom; troom = troom->lnext){
			if ( troom->people ){
				send_to_room (s_buf, troom->virtual);
			}
		}
	}
	
	return;
}

void do_broadcast(CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			*p = NULL;
	int				i=0;
	DESCRIPTOR_DATA		*d = NULL;

	for (i = 0; *(argument + i) == ' '; i++);

	if (!*(argument + i))
		send_to_char("That must be a mistake...\n", ch);
	else
	{
		snprintf (s_buf, MAX_STRING_LENGTH, "%s\n", argument + i);
		snprintf (buf, MAX_STRING_LENGTH,  "\a\n#2Staff Announcement:#0\n\n");
		reformat_string (s_buf, &p);
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s", p);

		for ( d = descriptor_list; d; d = d->next )
			SEND_TO_Q (buf, d);

	}
}

void do_zecho (CHAR_DATA *ch, char *argument, int cmd)
{
	int				zone = 0;
	ROOM_DATA		*room = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	
	strcpy (s_buf, argument);

	argument = one_argument (argument, buf);

	if ( !*buf || !strcmp (buf, "?") ) {
		send_to_char ("  zecho <zone> Message        send message to specific "
					  "zone.\n", ch);
		send_to_char ("  zecho Message               send message to zone you "
					  "are in.\n", ch);
		return;
	}

	while ( *argument == ' ' )
		argument++;

	if ( isdigit (*buf) && strtol(buf, NULL, 10) < 100 ) {
		zone = strtol(buf, NULL, 10);
		strcpy (s_buf, argument);
	} else
		zone = ch->in_room / 1000;

	strcat (s_buf, "\n");

	for ( room = full_room_list; room; room = room->lnext )
		if ( room->people && room->zone == zone )
			send_to_room (s_buf, room->virtual);

	snprintf (buf, MAX_STRING_LENGTH,  "(to zone %d): %s", zone, s_buf);
	send_to_char (buf, ch);
	
	return;
}

void do_pecho (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*vict = NULL;
	DESCRIPTOR_DATA 	*d = NULL;
	char		name [MAX_STRING_LENGTH] = {'\0'};
	char        message [MAX_STRING_LENGTH] = {'\0'};
	char        buf [MAX_STRING_LENGTH] = {'\0'};

	half_chop (argument, name, message);

	if ( !*name || !*message )
		send_to_char ("Who do you wish to pecho to and what??\n", ch);

	else if ( !(vict = get_char_nomask (name)) || IS_NPC (vict) ||
(!GET_TRUST (vict) && IS_SET (vict->flags, FLAG_WIZINVIS)) )
		send_to_char ("There is nobody playing the mud by that name.\n", ch);

	else if ( ch == vict )
		send_to_char ("You try to tell yourself something.\n", ch);

	else if ( IS_SET (vict->act, PLR_QUIET) ) {
		send_to_char ("That player is editing, try again later.\n", ch);
		return;
	}

	else {
		if ( IS_MORTAL (ch) && IS_SET (vict->flags, FLAG_ANON) ) {
			send_to_char ("There is nobody playing the mud by that name.\n", ch);
			return;
		}

		if ( !vict->desc && !IS_NPC (vict) ) {
			for ( d = descriptor_list; d; d = d->next ) {
				if ( d == vict->pc->owner ){
					break;
				}
			}

			if ( !d ) {
				send_to_char ("That player has disconnected.\n", ch);
				return;
			}
		}

		send_to_char (message, vict);
		snprintf (buf, MAX_STRING_LENGTH,  "You pecho to %s: '%s'\n",GET_NAME (vict), message);
		send_to_char (buf, ch);
	}
	
	return;
}


void do_transfer (CHAR_DATA *ch, char *argument, int cmd) 
{
	DESCRIPTOR_DATA	*i = NULL;
	CHAR_DATA		*victim = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	int				target = 0;

	argument = one_argument (argument, buf);
	
	if ( !*buf )
		send_to_char ("Who do you wish to transfer?\n", ch);
	
	else if ( str_cmp ("all", buf) ) {
		if (!(victim = get_char_vis(ch,buf)))
			send_to_char("No-one by that name around.\n",ch);
		else {
			act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
			target = ch->in_room;
			char_from_room(victim);
			char_to_room(victim,target);
			act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
			act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
			do_look(victim,"",15);
		}
	} 
	else { /* Trans All */
		if ( GET_TRUST (ch) < 5 ) {
			send_to_char ("Sorry, but TRANSFER ALL is a level 5-only command.\n", ch);
			return;
		}

		if ( !*argument || *argument != '!' ) {
			send_to_char ("Please type 'transfer all !' to confirm.\n", ch);
			return;
		}

	    for (i = descriptor_list; i; i = i->next) {
			if (i->character != ch && !i->connected) {
				victim = i->character;
				act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
				target = ch->in_room;
				char_from_room(victim);
				char_to_room(victim,target);
				act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
				act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
				do_look(victim,"",15);
			}
		}

		send_to_char ("All online PCs have been transferred to your location.\n", ch);
		return;
	}

	return;
}

void do_at (CHAR_DATA *ch, char *argument, int cmd)
{
	char			command [MAX_INPUT_LENGTH] = {'\0'};
	char			loc_str [MAX_INPUT_LENGTH] = {'\0'};
	int				location = 0;
	int				original_loc = 0;
	CHAR_DATA		*target_mob = NULL;
	OBJ_DATA		*target_obj = NULL;
	ROOM_DATA		*target_room = NULL;
	
	half_chop(argument, loc_str, command);

	if (!*loc_str) {
		send_to_char("You must supply a room number or a name.\n", ch);
		return;
	}

	
	if ( isdigit (*loc_str) ) {
		if ( !(target_room = vtor (strtol(loc_str, NULL,10))) ) {
			send_to_char("No room exists with that number.\n", ch);
			return;
		}
			
		location = target_room->virtual;
	}

	else if ((target_mob = get_char_vis (ch, loc_str)))
		location = target_mob->in_room;

	else if ((target_obj = get_obj_vis(ch, loc_str)))
		if (target_obj->in_room != NOWHERE)
			location = target_obj->in_room;
		else {
			send_to_char("The object is not available.\n", ch);
			return;
		}
	else{
		send_to_char("No such creature or object around.\n", ch);
		return;
	}

	/* a location has been found. */

	original_loc = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, location);
	command_interpreter(ch, command);

	/* check if the guy's still there */

	for ( target_mob = vtor (location)->people;
		  target_mob;
		  target_mob = target_mob->next_in_room )
		if ( ch == target_mob ) {
			char_from_room (ch);
			char_to_room (ch, original_loc);
		}
	
	return;
}

void do_goto (CHAR_DATA *ch, char *argument, int cmd)
{
	char		buf [MAX_INPUT_LENGTH] = {'\0'};
	int			location;
	CHAR_DATA	*target_mob = NULL;
	CHAR_DATA	*tch = NULL;
	OBJ_DATA	*target_obj = NULL;
	ROOM_DATA	*troom = NULL;

	clear_pmote(ch);

	argument = one_argument(argument, buf);
	if (!*buf) {
		send_to_char("You must supply a room number or a name.\n", ch);
		return;
	}

	if (isdigit(*buf)) {

		if ( !(troom = vtor (strtol(buf, NULL, 10))) ) {
			send_to_char("No room exists with that number.\n", ch);
			return;
		}

		location = troom->virtual;
	}

	else if ( (target_mob = get_char_vis (ch, buf)) )
		location = target_mob->in_room;

	else if ( (target_obj = get_obj_vis (ch, buf)) ) {
		if ( target_obj->in_room != NOWHERE )
			location = target_obj->in_room;
		else {
			send_to_char ("The object is not available.\n", ch);
			return;
		}
	} else {
		send_to_char ("No such creature or object around.\n", ch);
		return;
	}

	/* a location has been found. */

	if ( ch->in_room == location ) {
		send_to_char ("You're already there.\n", ch);
		return;
	}

	if ( ch->pc ) {
		if ( !ch->pc->imm_leave || !strncmp (ch->pc->imm_leave, "(null)", 6) )
			ch->pc->imm_leave = str_dup ("");

		if ( !ch->pc->imm_enter || !strncmp (ch->pc->imm_enter, "(null)", 6) )
			ch->pc->imm_enter = str_dup ("");

		if ( *ch->pc->imm_leave )
			act (ch->pc->imm_leave, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		else
			act ("$n quietly leaves.", TRUE, ch, 0, 0, TO_ROOM);
	} else
		act ("$n quietly leaves.", TRUE, ch, 0, 0, TO_ROOM);

	if ( !IS_SET (ch->flags, FLAG_WIZINVIS) && *argument != '!'  ) {

		troom = vtor(location);

		for ( tch = troom->people; tch; tch = tch->next_in_room ) {
			if ( !IS_NPC(tch) && IS_MORTAL(tch) ) {
				send_to_char ("You're currently visible to players. Use '!' to confirm the goto.\n", ch);
				return;
			}
		}
	}

	char_from_room (ch);
	char_to_room (ch, location);

	if ( ch->pc && *ch->pc->imm_enter )
		act (ch->pc->imm_enter, TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
	else
		act ("$n suddenly appears.", TRUE, ch, 0, 0, TO_ROOM);

	do_look (ch, "", 15);
	
	return;
}

void pad_buffer (char *buf, int pad_stop)
{
	int		to_pad = 0;
	char	*p = NULL;

	p = ADDBUF;

	for ( to_pad = pad_stop - strlen (buf); to_pad > 0; to_pad--, p++ )
		*p = ' ';

	*p = 0;
}


void charstat (CHAR_DATA *ch, char *name)
{
	int				i = 0;
	int				j = 0;
	int				i2 = 0;
	int				loaded_char = 0;
	int				loads = 0;
	int				econ_zone = 0;
	int				instance = 1;
	int				time_diff = 0;
	int				hours_remaining = 0;
	int				days_remaining = 0;
	int				mins_remaining = 0;
	CHAR_DATA		*k = NULL;
	CHAR_DATA		*tch = NULL;
	AFFECTED_TYPE	*af = NULL;
	AFFECTED_TYPE	*next_affect = NULL;
	CHARM_DATA		*ench = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( strtol(name, NULL,10) ) {
		if ( !(k = vtom (strtol(name, NULL,10))) ) {
			send_to_char ("No such mobile with that vnum.\n", ch);
			return;
		}
		instance = 0;
	}

	else if ( (k = load_pc (name)) )
		loaded_char = 1;

	else if ( !(k = get_char_room_vis (ch, name)) &&
			  !(k = get_char_vis (ch, name)) ) {
		send_to_char ("No player or mobile with that name found.\n", ch);
		return;
	}

	if ( !IS_NPC(k) )
		snprintf (buf, MAX_STRING_LENGTH,  "\n#2Name:#0   %s, a %s PC", k->tname, sex_types[(int)k->sex]);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "\n#2VNUM:#0   %d, a %s %s", k->mob->virtual, sex_types[(int)k->sex],
			instance ? "[INSTANCE]" : "[PROTOTYPE]");

	if ( IS_NPC(k) ) {
		loads = 0;

		for ( tch = character_list; tch; tch = tch->next ) {
			if ( tch->deleted || !IS_NPC (tch) )
				continue;

			if ( tch->mob->virtual == k->mob->virtual )
				loads++;
		}
		snprintf (ADDBUF, MAX_STRING_LENGTH, " [Loaded Instances: %d]", loads);
	}
	else if ( k->pc && k->pc->account ) {
		snprintf (ADDBUF, MAX_STRING_LENGTH, ", registered to account %s", k->pc->account);
	}

	snprintf (ADDBUF, MAX_STRING_LENGTH, " [id %d]\n", k->coldload_id);

	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Keys:#0   %s\n", GET_NAMES(k));

	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2SDesc:#0  %s\n", 
				  k->short_descr ? k->short_descr : "None");

	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2LDesc:#0  %s\n",
				  k->long_descr ? k->long_descr  : "None");

	send_to_char (buf, ch);
	send_to_char ("\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Str:#0 %d/%d", GET_STR (k), k->str);
	pad_buffer (buf, 25);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2MaxHP:#0 %d", GET_MAX_HIT (k));
	pad_buffer (buf, 53);
	if ( instance ) 
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Moves:#0  %d/%d", GET_MOVE(k), GET_MAX_MOVE(k)); 
	else
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Moves:#0  N/A"); 
	send_to_char (buf, ch);
	send_to_char ("\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Dex:#0 %d/%d", GET_DEX (k), k->dex);
	pad_buffer (buf, 25);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Race:#0  %s", db_race_table [k->race].name);
	pad_buffer (buf, 53);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Speak:#0  %s",skill_data[k->speaks].skill_name);
	send_to_char (buf, ch);
	send_to_char ("\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Con:#0 %d/%d", GET_CON (k), k->con);
	pad_buffer (buf, 25);
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Age:#0   %dY %dM %dD %dH", 
	age (k).year, age (k).month, age (k).day, age (k).hour);
	pad_buffer (buf, 53);
	if ( instance )
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Ht/Wt:#0  %d/%d", k->height, get_weight (k) / 100);
	else
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Ht/Wt:#0  N/A");
	send_to_char (buf, ch);
	send_to_char ("\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Int:#0 %d/%d", GET_INT (k), k->intel);
	pad_buffer (buf, 25);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Fight:#0 %s", fight_tab [k->fight_mode].name);
	pad_buffer (buf, 53);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Offns:#0  %d", k->offense);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "\n");
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Wil:#0 %d/%d", GET_WIL (k), k->wil);
	pad_buffer (buf, 25);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Hrnss:#0 %d", k->harness);
	pad_buffer (buf, 53);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2MHrnss:#0 %d", k->max_harness);
	send_to_char (buf, ch);
	send_to_char ("\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Aur:#0 %d/%d", GET_AUR (k), k->aur);
	pad_buffer (buf, 25);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Armor:#0 %d", k->armor);
	pad_buffer (buf, 53);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Speed:#0  %s", speeds[k->speed]);
	send_to_char (buf, ch);
	send_to_char ("\n", ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Agi:#0 %d/%d", GET_AGI(k), k->agi);

	pad_buffer (buf, 25);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Room:#0  %d", k->in_room);
	pad_buffer (buf, 53);
	if ( IS_NPC (k) ) {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Spwn:#0   %d", k->mob->spawnpoint);
	} else {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Cond:#0   %d,%d,%d", 
					  k->intoxication,
					  k->hunger,
					  k->thirst);
	}

	snprintf (ADDBUF, MAX_STRING_LENGTH, "\n");
	send_to_char (buf, ch);

	if ( IS_NPC (k) ) {
		*buf = '\0';
		snprintf (buf, MAX_STRING_LENGTH,  "#2Sum:#0 %d",
			k->str + k->dex + k->con + k->intel + k->wil +
			k->aur + k->agi);
		pad_buffer (buf, 25);
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Carc:#0  %d", k->mob->carcass_vnum);
		pad_buffer (buf, 53);
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#2Skin:#0   %d", k->mob->skinned_vnum);
	        snprintf (ADDBUF, MAX_STRING_LENGTH, "\n");
		send_to_char (buf, ch);
		*buf = '\0';
		pad_buffer (buf, 21);
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Att:#0   %s", attack_names[k->nat_attack_type]);
		pad_buffer (buf, 49);
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#2NDam:#0   %dd%d", k->mob->damnodice, k->mob->damsizedice);
	        snprintf (ADDBUF, MAX_STRING_LENGTH, "\n");
		send_to_char (buf, ch);
	}
	else if ( k->pc ) {
		*buf = '\0';
		snprintf (buf, MAX_STRING_LENGTH,  "#2Sum:#0 %d",
			k->str + k->dex + k->con + k->intel + k->wil +
			k->aur + k->agi);
		pad_buffer (buf, 25);
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Level:#0 %d", k->pc->level);
	        snprintf (ADDBUF, MAX_STRING_LENGTH, "\n");
		send_to_char (buf, ch);
	}

	snprintf (buf, MAX_STRING_LENGTH,  "\n#2Clans:#0  [%s]\n", k->clans && *k->clans ? k->clans : "");
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2States:#0 [%s", position_types [GET_POS (k)]);

	if ( k->intoxication > 0 )
		snprintf (ADDBUF, MAX_STRING_LENGTH, ", drunk");

	if ( !k->hunger )
		snprintf (ADDBUF, MAX_STRING_LENGTH, ", hungry");

	if ( !k->thirst )
		snprintf (ADDBUF, MAX_STRING_LENGTH, ", thirsty");
	
	if ( !IS_NPC (k) ) {
		switch ( k->pc->create_state ) {
			case STATE_REJECTED:
				snprintf (ADDBUF, MAX_STRING_LENGTH, ", REJECTED");
				break;
			case STATE_APPLYING:
				snprintf (ADDBUF, MAX_STRING_LENGTH, ", APPLYING");
				break;
			case STATE_SUBMITTED:
				snprintf (ADDBUF, MAX_STRING_LENGTH, ", SUBMITTED");
				break;
			case STATE_APPROVED:
				snprintf (ADDBUF, MAX_STRING_LENGTH, ", APPROVED");
				break;
			case STATE_SUSPENDED:
				snprintf (ADDBUF, MAX_STRING_LENGTH, ", SUSPENDED");
				break;
			case STATE_DIED:
				snprintf (ADDBUF, MAX_STRING_LENGTH, ", DIED");
				break;
		}
	}

	if ( GET_FLAG (k, FLAG_ENTERING) )
		snprintf (ADDBUF, MAX_STRING_LENGTH, ", ENTERING");

	if ( GET_FLAG (k, FLAG_LEAVING) )
		snprintf (ADDBUF, MAX_STRING_LENGTH, ", LEAVING");

	if ( GET_FLAG (k, FLAG_OSTLER) )
		snprintf (ADDBUF, MAX_STRING_LENGTH, ", OSTLER");

	snprintf (ADDBUF, MAX_STRING_LENGTH, "]\n");
	send_to_char (buf, ch);


	if ( k->mob && k->mob->access_flags ) {

		snprintf (buf, MAX_STRING_LENGTH,  "Access: [");

		for ( i = 0; *room_bits [i] != '\n'; i++ )
			if ( IS_SET (k->mob->access_flags, 1 << i) )
				snprintf (ADDBUF, MAX_STRING_LENGTH, "%s ", room_bits [i]);

		strcat (buf, "]\n");
		send_to_char (buf, ch);
	}

	if ( k->mob && k->mob->noaccess_flags ) {

		snprintf (buf, MAX_STRING_LENGTH,  "Noaccess: [");

		for ( i = 0; *room_bits [i] != '\n'; i++ )
			if ( IS_SET (k->mob->noaccess_flags, 1 << i) )
				snprintf (ADDBUF, MAX_STRING_LENGTH, "%s ", room_bits [i]);

		strcat (buf, "]\n");
		send_to_char (buf, ch);
	}

	snprintf (buf, MAX_STRING_LENGTH,  "#2Flags:#0  [");

	if ( !IS_NPC (k) ) {
		for( i = 0; *player_bits [i] != '\n'; i++ ) 
			if ( IS_SET (k->act, 1 << i) )
				snprintf (ADDBUF, MAX_STRING_LENGTH, " %s", player_bits [i]);

	} else {
		for( i = 0; *action_bits [i] != '\n'; i++ ) 
			if ( IS_SET (k->act, 1 << i) )
				snprintf (ADDBUF, MAX_STRING_LENGTH, " %s", action_bits [i]);

		if ( k->mob->vehicle_type == VEHICLE_BOAT )
			strcat (buf, " Boat");

		else if ( k->mob->vehicle_type == VEHICLE_HITCH )
			strcat (buf, " Hitch");
	}

	if ( GET_FLAG (k, FLAG_GUEST) )
		strcat (buf, " Guest");

	if ( IS_SET (k->plr_flags, NOPETITION) )
		strcat (buf, " NoPetition");

	if ( GET_FLAG (k, FLAG_BRIEF) )
		strcat (buf, " Brief");

	if ( GET_FLAG (k, FLAG_WIZINVIS) )
		strcat (buf, " Wizinvis");

	if ( GET_FLAG (k, FLAG_KILL) && k->fighting )
		strcat (buf, " Killing");
	else if ( !GET_FLAG (k, FLAG_KILL) && k->fighting )
		strcat (buf, " Hitting");

	if ( GET_FLAG (k, FLAG_FLEE) )
		strcat (buf, " Fleeing");

	if ( GET_FLAG (k, FLAG_BINDING) )
		strcat (buf, " Binding");

	if ( GET_FLAG (k, FLAG_VARIABLE) )
		strcat (buf, " Variable");
		
	if ( GET_FLAG (k, FLAG_PACIFIST) )
		strcat (buf, " Pacifist");

	if ( IS_SET (k->plr_flags, NEWBIE_HINTS) )
		strcat (buf, " Hints");

	if ( IS_SET (k->plr_flags, MENTOR) )
		strcat (buf, " Mentor");

	if ( IS_SET (k->plr_flags, NEWBIE) )
		strcat (buf, " Newbie");

	if ( IS_SET (k->act, ACT_NOBIND) )
		strcat (buf, " NoBind");

	if ( IS_SET (k->act, ACT_NOBLEED) )
		strcat (buf, " NoBleed");

	if ( IS_SET (k->act, ACT_FLYING) )
		strcat (buf, " Flying");

	if ( IS_SET (k->act, ACT_PHYSICIAN) )
		strcat (buf, " Physician");

	if ( GET_FLAG (k, FLAG_SEE_NAME) )
		strcat (buf, " See_name");

	if ( GET_FLAG (k, FLAG_AUTOFLEE) )
		strcat (buf, " Autoflee");

	if ( IS_SET (k->act, ACT_PREY) )
		strcat (buf, " Prey");

	if ( GET_FLAG (k, FLAG_NOPROMPT) )
		strcat (buf, " Noprompt");

	if ( GET_FLAG (k, FLAG_OSTLER) )
		strcat (buf, " Ostler");

	if ( GET_FLAG (k, FLAG_ISADMIN) )
		strcat (buf, " IsAdminPC");

	if ( !IS_NPC(k) && k->pc->is_guide )
		strcat (buf, " IsGuide");

	if ( IS_SET (k->plr_flags, NO_PLAYERPORT) )
		strcat (buf, " NoPlayerPort");

	if ( k->color )
		strcat (buf, " ANSI");

	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " ]\n");
	send_to_char (buf, ch);

	if ( IS_NPC (k) && k->mob->owner ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#2Owner:#0  [%s]\n", k->mob->owner);
		send_to_char (buf, ch);
	}

	if ( IS_SET(k->act, ACT_JAILER) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#2Jail Info:#0 [ Cell1: %d Cell2: %d Cell3: %d ].\n", k->cell_1, k->cell_2, k->cell_3);
		send_to_char (buf, ch);
	}

	
	if ( k->desc && k->desc->snoop.snooping &&
		 is_he_somewhere (k->desc->snoop.snooping) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#2Snooping:#0  %s\n", GET_NAME (k->desc->snoop.snooping));
		send_to_char (buf, ch);
	}

	if ( k->desc && k->desc->snoop.snoop_by &&
		 is_he_somewhere (k->desc->snoop.snoop_by) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#2Snooped by:#0  %s\n", GET_NAME (k->desc->snoop.snoop_by));
		send_to_char (buf, ch);
	}

	if ( k->following && is_he_somewhere (k->following) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#2Following:#0 %s\n", GET_NAME (k->following));
		send_to_char (buf, ch);
	}

	if ( k->affected_by ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#2Affect:#0 [ ");
		for ( i = 0; *affected_bits [i] != '\n'; i++ )
			if ( IS_SET (k->affected_by, 1 << i) )
				snprintf (ADDBUF, MAX_STRING_LENGTH, "%s ", affected_bits [i]);
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "]\n");
		send_to_char (buf, ch);
	}

	
	if ( k->charms ) {
		snprintf (buf, MAX_STRING_LENGTH,  "------------ CHARMs ------------\n");
		for ( ench = k->charms; ench; ench = ench->next )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s: %d hours remaining\n", ench->name, ench->current_hours);
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "--------------------------------------\n\n");
		send_to_char (buf, ch);
	}

	for ( af = k->hour_affects; af; af = next_affect ) {

		if ( af == k->hour_affects )
			send_to_char ("\n", ch);

		next_affect = af->next;		/* WatchX affect can delete af */

		if ( af->type == AFFECT_SHADOW ) {

			if ( !af->a.shadow.shadow && af->a.shadow.edge != -1 )
				snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Standing", af->type);

			else if ( !is_he_somewhere (af->a.shadow.shadow) )
				continue;

			else if ( IS_NPC (af->a.shadow.shadow) )
				snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Shadowing %s (%d)", af->type,
						af->a.shadow.shadow->short_descr,
						af->a.shadow.shadow->mob->virtual);
			else
				snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Shadowing PC %s", af->type,
						GET_NAME (af->a.shadow.shadow));

			if ( af->a.shadow.edge != -1 )
				snprintf (ADDBUF, MAX_STRING_LENGTH, " on %s edge.",
						dirs [af->a.shadow.edge]);

			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");

			send_to_char (buf, ch);

			continue;
		}

		if ( af->type == AFFECT_GUARD_DIR ) {

			if ( !af->a.shadow.shadow && af->a.shadow.edge != -1 )
				snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Guarding", af->type);

				snprintf (ADDBUF, MAX_STRING_LENGTH, " the %s exit.",
						dirs [af->a.shadow.edge]);

			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");

			send_to_char (buf, ch);

			continue;
		}

		else if ( af->type == MAGIC_SIT_TABLE ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Sitting at table.\n", af->type);
			send_to_char (buf, ch);
			continue;
		}

		else if ( af->type >= MAGIC_STABLING_PAID && af->type <= MAGIC_STABLING_LAST ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Stabling paid for ID %d, %d IC hours remaining.\n", af->type, af->a.spell.sn, af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if (af->type == MUTE_EAVESDROP) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0    Muting Eavesdropping\n", 
						af->type);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST ) {
/*
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Craft %s subcraft %s\n",
						  af->type,
						  af->a.craft->subcraft->craft_name,
						  af->a.craft->subcraft->subcraft_name);
			send_to_char (buf, ch);
*/
			continue;
		}

		if ( af->type >= JOB_1 && af->type <= JOB_3 ) {

			i = time_info.year * 12 * 30 + time_info.month * 30 + time_info.day;
			i = af->a.job.pay_date - i;

			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Job %d:  Pays %d coins (%d of %d days until "
						  "payday)\n"
						  "                 %d x vnum %s (%d)\n", af->type,
							af->type - JOB_1 + 1, af->a.job.cash,
							i, af->a.job.days,
							af->a.job.count,
							!vtoo (af->a.job.object_vnum) ?
								"UNDEFINED" : vtoo (af->a.job.object_vnum)->
											short_description,
							af->a.job.object_vnum);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_CRAFT_BRANCH_STOP ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   May branch a craft again in %d RL minutes.\n", af->type, af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_PETITION_MESSAGE ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Will not receive petition message again for %d RL minutes.\n", af->type, af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == AFFECT_LOST_CON ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Will regain %d CON points in %d in-game hours.\n", af->type, af->a.spell.sn, af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == AFFECT_HOLDING_BREATH ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Can hold breath for %d more RL seconds.\n", af->type, af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_CRAFT_DELAY ) {
			if ( time(0) >= af->a.spell.modifier )
				continue;
	                time_diff = af->a.spell.modifier - time(0);
        	        days_remaining = time_diff / (60 * 60 * 24);
                	time_diff %= 60 * 60 * 24;
                	hours_remaining = time_diff / (60 * 60);
			time_diff %= 60 * 60;
			mins_remaining = time_diff / 60;
			time_diff %= 60;
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   OOC craft delay timer engaged. [%dd %dh %dm %ds remaining]\n", af->type, days_remaining, hours_remaining, mins_remaining, time_diff);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type >= MAGIC_SKILL_GAIN_STOP && af->type < MAGIC_SPELL_GAIN_STOP ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   May improve %s again in %d RL minutes.\n", 
			af->type, 
			skill_data[af->type - MAGIC_SKILL_GAIN_STOP].skill_name,
			af->a.spell.duration);
			
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type >= MAGIC_FLAG_NOGAIN && af->type <= MAGIC_FLAG_NOGAIN + LAST_SKILL ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Skill gain in %s has been halted by an administrator.\n", af->type, skill_data[af->type-6000].skill_name);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_RAISED_HOOD ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Mob has raised hood due to inclement weather.\n", af->type);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type >= MAGIC_CRIM_BASE &&
			 af->type <= MAGIC_CRIM_BASE + 100 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Wanted in zone %d for %d hours.\n", af->type,
						  af->type - MAGIC_CRIM_BASE, af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_NOTIFY ) {
			if ( is_he_somewhere ((CHAR_DATA *) af->a.spell.t) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Notify pending on %s for %d hours.\n",
					af->type,
					GET_NAME ((CHAR_DATA *) af->a.spell.t), af->a.spell.duration);
				send_to_char (buf, ch);
			}

			continue;
		}

		if ( af->type == MAGIC_CLAN_NOTIFY ) {
			if ( is_he_somewhere ((CHAR_DATA *) af->a.spell.t) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   CLAN Notify pending on %s for %d hours.\n",
					af->type,
					GET_NAME ((CHAR_DATA *) af->a.spell.t), af->a.spell.duration);
				send_to_char (buf, ch);
			}

			continue;
		}

		if ( af->type == MAGIC_GUARD ) {
			if ( is_he_somewhere ((CHAR_DATA *) af->a.spell.t) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Guarding %s.\n", af->type,
					GET_NAME ((CHAR_DATA *) af->a.spell.t));
				send_to_char (buf, ch);
			} else
				send_to_char ("   Guarding someone who's left the game.\n", ch);

			continue;
		}

		if ( af->type >= MAGIC_CRIM_HOODED &&
			 af->type < MAGIC_CRIM_HOODED + 100 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Hooded criminal charge in zone %d for %d RL seconds.\n",
					 af->type, af->type - MAGIC_CRIM_HOODED, 
					 af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_STARED ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Studied by an enforcer.  Won't be studied for "
						  "%d RL seconds.\n",
					 af->type, af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_WARNED ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Warned by an enforcer.  Won't be warned for "
						  "%d RL seconds.\n",
					 af->type, af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_HIDDEN ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Hidden.\n", af->type);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_SNEAK ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Currently trying to sneak.\n", af->type);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_TOLL ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   TOLL:  Collecting %d for going %s.\n",
						  af->type,
						  af->a.toll.charge,
						  dirs [af->a.toll.dir]);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type >= MAGIC_AFFECT_FIRST && af->type <= MAGIC_AFFECT_LAST ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Spell effect, %d RL minutes remaining.\n",
						  af->type,
						  af->a.spell.duration);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_TOLL_PAID ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   TOLL PAID to travel %s.\n",
						  af->type,
						  dirs [af->a.toll.dir]);
			send_to_char (buf, ch);
			continue;
		}

		if ( af->type == MAGIC_WATCH1 ||
		     af->type == MAGIC_WATCH2 ||
		     af->type == MAGIC_WATCH3 )
			continue;

		if ( af->type >= MAGIC_FIRST_SOMA && af->type <= MAGIC_LAST_SOMA ) {
			soma_stat(ch, af);
			continue;
		}

		
	}

	send_to_char ("\n", ch);

	if ( k->shop && IS_SET (k->flags, FLAG_KEEPER) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "  #2Shop Rm:#0  %5d\n",
					  k->shop->shop_vnum);
		send_to_char (buf, ch);

		snprintf (buf, MAX_STRING_LENGTH,  "  #2Store Rm:#0 %5d\n",
					  k->shop->store_vnum);
		send_to_char (buf, ch);

		snprintf (buf, MAX_STRING_LENGTH,  "  #2Markup:#0    %2.2f                \n",
					  k->shop->markup);
		send_to_char (buf, ch);

		if ( IS_SET (k->act, ACT_ECONZONE) ) {
			send_to_char ("   #3NOTE:#0  #5ECONZONE flag overrides standard "
						  "markups and discounts.#0\n", ch);

			econ_zone = zone_to_econ_zone (ch->room->zone);

			if ( econ_zone != -1 ) {

				snprintf (buf, MAX_STRING_LENGTH,  "          Markup/Discounts are those paid in "
							  "your zone:  %d (%s)\n",
							  ch->room->zone,
							  default_econ_info [econ_zone].flag_name);
				send_to_char (buf, ch);

				*buf = '\0';

				for ( i = 0; *default_econ_info [i].flag_name != '\n'; i++ ) {

					if ( (i % 3) == 0 ) {
						if ( *buf ) {
							send_to_char (buf, ch);
							send_to_char ("\n", ch);
						}
						strcpy (buf, "     ");
					}

					else if ( (i % 3) == 1 )
						pad_buffer (buf, 28);

					else
						pad_buffer (buf, 53);

					snprintf (ADDBUF, MAX_STRING_LENGTH, "  %-10.10s: %1.2f/%1.2f",
					  default_econ_info [i].flag_name,
					  default_econ_info [i].obj_econ_info [econ_zone].markup,
					  default_econ_info [i].obj_econ_info [econ_zone].discount);
				}

				if ( *buf ) {
					send_to_char (buf, ch);
					send_to_char ("\n", ch);
				}
			}
		}

		else {
			snprintf (buf, MAX_STRING_LENGTH,  "  #2Discount:#0  %2.2f\n", k->shop->discount);
			send_to_char (buf, ch);

			snprintf (buf, MAX_STRING_LENGTH,  "  #2Econ Markup1:#0 %2.2f", k->shop->econ_markup1);
			pad_buffer (buf, 23);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "   #2Discount1:#0 %2.2f", k->shop->econ_discount1);
			pad_buffer (buf, 40);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "  #2Econ1:#0 [");

			for ( i = 0; str_cmp (econ_flags [i], "\n"); i++ ) {
				if ( IS_SET (k->shop->econ_flags1, 1 << i) ) {
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH, "%s", econ_flags [i]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " ");
				}
			}

			if ( buf [strlen (buf) - 1] != '[' )
				buf [strlen (buf) - 1] = ']';
			else
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "]");

			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
			send_to_char (buf, ch);

			snprintf (buf, MAX_STRING_LENGTH,  "  #2Econ Markup2:#0 %2.2f", k->shop->econ_markup2);
			pad_buffer (buf, 23);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "   #2Discount2:#0 %2.2f", k->shop->econ_discount2);
			pad_buffer (buf, 40);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "  #2Econ2:#0 [");

			for ( i = 0; str_cmp (econ_flags [i], "\n"); i++ ) {
				if ( IS_SET (k->shop->econ_flags2, 1 << i) ) {
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH, "%s",  econ_flags [i]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " ");
				}
			}

			if ( buf [strlen (buf) - 1] != '[' )
				buf [strlen (buf) - 1] = ']';
			else
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "]");

			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
			send_to_char (buf, ch);

			snprintf (buf, MAX_STRING_LENGTH,  "  #2Econ Markup3:#0 %2.2f", k->shop->econ_markup3);
			pad_buffer (buf, 23);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "   #2Discount3:#0 %2.2f", k->shop->econ_discount3);
			pad_buffer (buf, 40);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "  #2Econ3:#0 [");

			for ( i = 0; str_cmp (econ_flags [i], "\n"); i++ ) {
				if ( IS_SET (k->shop->econ_flags3, 1 << i) ) {
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH, "%s",  econ_flags [i]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " ");
				}
			}

			if ( buf [strlen (buf) - 1] != '[' )
				buf [strlen (buf) - 1] = ']';
			else
				strcat (buf, "]");

			strcat (buf, "\n");
			send_to_char (buf, ch);

			snprintf (buf, MAX_STRING_LENGTH,  "  #2Econ Markup4:#0 %2.2f", k->shop->econ_markup4);
			pad_buffer (buf, 23);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "   #2Discount4:#0 %2.2f", k->shop->econ_discount4);
			pad_buffer (buf, 40);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "  #2Econ4:#0 [");

			for ( i = 0; str_cmp (econ_flags [i], "\n"); i++ ) {
				if ( IS_SET (k->shop->econ_flags4, 1 << i) ) {
					strcat (buf, econ_flags [i]);
					strcat (buf, " ");
				}
			}

			if ( buf [strlen (buf) - 1] != '[' )
				buf [strlen (buf) - 1] = ']';
			else
				strcat (buf, "]");

			strcat (buf, "\n");
			send_to_char (buf, ch);


			snprintf (buf, MAX_STRING_LENGTH,  "  #2Econ Markup5:#0 %2.2f", k->shop->econ_markup5);
			pad_buffer (buf, 23);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "   #2Discount5:#0 %2.2f", k->shop->econ_discount5);
			pad_buffer (buf, 40);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "  #2Econ5:#0 [");

			for ( i = 0; str_cmp (econ_flags [i], "\n"); i++ ) {
				if ( IS_SET (k->shop->econ_flags5, 1 << i) ) {
					strcat (buf, econ_flags [i]);
					strcat (buf, " ");
				}
			}

			if ( buf [strlen (buf) - 1] != '[' )
				buf [strlen (buf) - 1] = ']';
			else
				strcat (buf, "]");

			strcat (buf, "\n");
			send_to_char (buf, ch);


			snprintf (buf, MAX_STRING_LENGTH,  "  #2Econ Markup6:#0 %2.2f", k->shop->econ_markup6);
			pad_buffer (buf, 23);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "   #2Discount6:#0 %2.2f", k->shop->econ_discount6);
			pad_buffer (buf, 40);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "  #2Econ6:#0 [");

			for ( i = 0; str_cmp (econ_flags [i], "\n"); i++ ) {
				if ( IS_SET (k->shop->econ_flags6, 1 << i) ) {
					strcat (buf, econ_flags [i]);
					strcat (buf, " ");
				}
			}

			if ( buf [strlen (buf) - 1] != '[' )
				buf [strlen (buf) - 1] = ']';
			else
				strcat (buf, "]");

			strcat (buf, "\n");
			send_to_char (buf, ch);


			snprintf (buf, MAX_STRING_LENGTH,  "  #2Econ Markup7:#0 %2.2f", k->shop->econ_markup7);
			pad_buffer (buf, 23);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "   #2Discount7:#0 %2.2f", k->shop->econ_discount7);
			pad_buffer (buf, 40);
			snprintf (ADDBUF, MAX_STRING_LENGTH, "  #2Econ7:#0 [");

			for ( i = 0; str_cmp (econ_flags [i], "\n"); i++ ) {
				if ( IS_SET (k->shop->econ_flags7, 1 << i) ) {
					strcat (buf, econ_flags [i]);
					strcat (buf, " ");
				}
			}

			if ( buf [strlen (buf) - 1] != '[' )
				buf [strlen (buf) - 1] = ']';
			else
				strcat (buf, "]");

			strcat (buf, "\n");
			send_to_char (buf, ch);

		}

		snprintf (buf, MAX_STRING_LENGTH,  "  #2NoBuy Flags:#0  [");

		for ( i = 0; str_cmp (econ_flags [i], "\n"); i++ ) {
			if ( IS_SET (k->shop->nobuy_flags, 1 << i) ) {
				strcat (buf, econ_flags [i]);
				strcat (buf, " ");
			}
		}

		if ( buf [strlen (buf) - 1] != '[' )
			buf [strlen (buf) - 1] = ']';
		else
			strcat (buf, "]");

		strcat (buf, "\n");
		send_to_char (buf, ch);

		snprintf (buf, MAX_STRING_LENGTH,  "  #2Trades in:#0    [");

		for ( i = 0; i < MAX_TRADES_IN; i++ )
			if ( k->shop->trades_in [i] )
				snprintf (ADDBUF, MAX_STRING_LENGTH, "%s ", item_types [k->shop->trades_in [i]]);

		snprintf (ADDBUF, MAX_STRING_LENGTH, "]\n\n");
		send_to_char (buf, ch);

		if ( k->shop->delivery [0] ) {
			snprintf (buf, MAX_STRING_LENGTH,  "#2Deliveries:#0\n   ");
			j = 1;
			for ( i = 0; i <= MAX_DELIVERIES; i++ ) {
				if ( !k->shop->delivery[i] )
					break;
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2%d:#0 %-8d ", i, k->shop->delivery[i]);
				if ( !(j % 6) )
					strcat (buf, "\n   ");
				j++;
			}
			strcat (buf, "\n\n");
			send_to_char (buf, ch);
		}
 	}

	i2 = 1;
        snprintf (buf, MAX_STRING_LENGTH,  "#2%8.8s:#0 %03d/%03d  %s", "Offense", k->offense,
        	k->offense, i2 % 4 ? "" : "\n"); 
	send_to_char (buf, ch);

	for ( i = 0; i < MAX_SKILLS; i++ ) {
		if ( k->skills [i] && real_skill (k, i) ) {
			i2++;
			snprintf (buf, MAX_STRING_LENGTH,  "#2%8.8s:#0 %03d/%03d  %s", skill_data[i].skill_name, k->skills [i],
						  real_skill (k, i), i2 % 4 ? "" : "\n");
			send_to_char (buf, ch);
		}
	}

	if ( buf [strlen(buf)-1] != '\n' )
		send_to_char ("\n", ch);

	if ( loaded_char )
		unload_pc (k);
}

void roomstat (CHAR_DATA *ch, char *name)
{
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				buf2 [MAX_STRING_LENGTH] = {'\0'};
	ROOM_DATA			*rm = NULL;
	CHAR_DATA			*k = NULL;
	OBJ_DATA			*j = NULL;
	XTRA_DESCR_DATA		*desc = NULL;
	int					i = 0;
	AFFECTED_TYPE		*herbed = NULL;


	rm = vtor (ch->in_room);
	snprintf (buf, MAX_STRING_LENGTH,  "[%4d]: %s\n", rm->virtual, rm->name);
	send_to_char(buf, ch);

	sprinttype (rm->sector_type, *sector_types, buf2);
	snprintf (buf, MAX_STRING_LENGTH,  "Sector type[%s] ", buf2);
	send_to_char(buf, ch);
			
	sprintbit ((long) rm->room_flags,room_bits,buf2);
	snprintf (buf, MAX_STRING_LENGTH,  "Rflags[%s]\n",buf2);
	send_to_char (buf,ch);

	send_to_char("Description:\n", ch);
	send_to_char(rm->description, ch);
	send_to_char("\n",ch);	
	if(rm->ex_description) {
		send_to_char("Ex-desc keyword(s)[",ch);
		for (desc = rm->ex_description; desc; desc = desc->next) {
			send_to_char(desc->keyword,ch);
			send_to_char(" ",ch);
		}
		send_to_char("]\n",ch);
	}

	herbed = is_room_affected(rm->affects, HERBED_COUNT);

	if(!herbed) 
	    send_to_char("\nThis room has not been herbed.\n\n", ch);
	else {
	    snprintf (buf2, MAX_STRING_LENGTH, "This room has been herbed %d times (Duration = %d)\n\n",
		    herbed->a.herbed.timesHerbed, herbed->a.herbed.duration);
	    send_to_char(buf2, ch);
	}
	    
	send_to_char ("------- Chars present -------\n", ch);
	for (k = rm->people; k; k = k->next_in_room) {
		if ( IS_NPC (k) )
			act ("NPC: $N", FALSE, ch, 0, k, TO_CHAR);
		else
			act ("PC:  $N", FALSE, ch, 0, k, TO_CHAR);
	}
	strcat(buf, "\n");
	send_to_char(buf, ch);

	strcpy(buf, "--------- Contents ---------\n");
	for (j = rm->contents; j; j = j->next_content)
	{
		strcat(buf, j->name);
		strcat(buf, "\n");
	}
	strcat(buf, "\n");
	send_to_char(buf, ch);

	send_to_char("------- Exits defined -------\n", ch);
	for (i = 0; i <= 5; i++) {
		if (rm->dir_option[i]) {
			snprintf (buf, MAX_STRING_LENGTH, "Direction %s . Keyword : %s\n",
			        dirs[i], rm->dir_option[i]->keyword);
			send_to_char(buf, ch);
			strcpy(buf, "Description:\n  ");
			if(rm->dir_option[i]->general_description)
		  	strcat(buf, rm->dir_option[i]->general_description);
			else
				strcat(buf,"UNDEFINED\n");
			send_to_char(buf, ch);
			sprintbit(rm->dir_option[i]->exit_info,exit_bits,buf2);
			snprintf (buf, MAX_STRING_LENGTH,  "Exit flag: %s \nKey no: %d  (pick penalty %d)\nTo room (Virtual: %d)\n",
			        buf2, rm->dir_option[i]->key, rm->dir_option [i]->pick_penalty,
					rm->dir_option[i]->to_room);
			send_to_char(buf, ch);
		}
	}
	
	send_to_char("------- Extra descriptions -------\n", ch);
	if ( rm->extra ) {
		for ( i = 0; i < WR_DESCRIPTIONS; i++ ){
			if (rm->extra->weather_desc [i]){
				snprintf (buf, MAX_STRING_LENGTH, "%s:  %s", weather_room [i], rm->extra->weather_desc [i]);
				send_to_char(buf, ch);
			}
		}
		
		send_to_char("\n", ch);
		
		for ( i = 0; i < 6; i++ ){
			if (rm->extra->alas [i]){
				snprintf (buf, MAX_STRING_LENGTH,"%s alas:  %s", dirs[i], rm->extra->alas [i] );
				send_to_char(buf, ch);
			}
		}
	}
}

/*                                                                          *
 * funtion: acctstat                    < e.g.> stat acct traithe           *
 *                                                                          *
 *                                                                          */
void acctstat (CHAR_DATA *ch, char *name)
{
	ACCOUNT_DATA	*account = NULL;
	STORED_PC_DATA	*pc = NULL;
	char			*date = NULL;
	time_t			account_time;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			line [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row = NULL;
	int				messages = 0;

	if ( !*name || strtol(name, NULL,10) ) {
		send_to_char ("Please specify an account name.\n", ch);
		return;
	}
	if ( !(account = load_account(name)) ) {
		send_to_char ("That account does not exist.\n", ch);
		return;
	}
	

	*buf = '\0';
	*line = '\0';

	snprintf (buf, MAX_STRING_LENGTH, "\n   #2Account Name:#0            %s\n", account->name);
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Registered Email:#0        %s\n", account->email);
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Last Logged In From:#0     %s\n", account->last_ip);
	
	/* Retreive a list of other users from this IP ( default = no-op )*/
	mysql_safe_query ("SELECT username FROM forum_users WHERE user_last_ip = '%s' AND username != '%s'", account->last_ip, account->name);
	if( ( result = mysql_store_result( database ) ) != NULL ) {
	  if ( !IS_SET (account->flags, ACCOUNT_IPSHARER) && mysql_num_rows (result) > 0  
	       && str_cmp( account->name, "Guest" ) 
	       && str_cmp( account->last_ip, "(null)" )
	       && !strstr( account->last_ip, "www.yourmud.org" ) ) {
	    snprintf ( buf + strlen( buf ), MAX_STRING_LENGTH, "   #1Accounts Sharing IP:#0    " );
	    while ( ( row = mysql_fetch_row( result ) ) ) {
	      snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s", row[0]);
	    }
	    snprintf (buf + strlen(buf), MAX_STRING_LENGTH, "#0\n");
	  }	
	  mysql_free_result (result);
	  result = NULL;
	}
	else {
	  system_log("Warning: MySql returned a NULL result in staff.c::acctstat() while fetching account sharing info.", TRUE);
	}

	/* Retreive the timestamp for Last Activity ( default = "" ) */
	date = (char *) malloc ( 256) ; 
	date[0] = '\0' ;
	mysql_safe_query ( "SELECT lastlogon FROM %s.pfiles WHERE account = '%s' ORDER BY lastlogon DESC LIMIT 1", PFILES_DATABASE, account->name ) ;
	if ( ( result = mysql_store_result ( database ) ) != NULL ) {
	  row = mysql_fetch_row( result ) ;
	  
	  if ( row && *row[0] ) {
	    account_time = (time_t) strtol(row[0], NULL, 10);
	    if ( asctime_r( localtime( &account_time ), date ) != NULL ) { 
	      date [strlen(date)-1] = '\0' ; /* chop the newline asctime_r tacks on */
	    }
	  }
	  
	  mysql_free_result ( result );
	  result = NULL;
	}
	else {
	  system_log("Warning: MySql returned a NULL result in staff.c::acctstat() while fetching lastlogin info.", TRUE);
	}
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Last PC Activity On:#0     %s\n", ( date[0] ) ? date : "None Recorded") ;     
	mem_free (date);

	/* Account Registration Date ( default = "None Recorded" ) */
	date = (char *) malloc (256) ; 
	date[0] = '\0' ;
	account_time = account->created_on;
	asctime_r( localtime( &account_time ), date ) ;
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Account Registered On:#0   %s", ( date[0] ) ? date : "None Recorded") ;
	mem_free (date);

	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Accrued Roleplay Points:#0 %d\n", account->roleplay_points);

	if ( account->last_rpp > 0 ) {
		account_time = (time_t) account->last_rpp;
		date = (char *) malloc (256) ; 
		date[0] = '\0' ;
		asctime_r( localtime( &account_time ), date ) ;
		date[strlen(date)-1] = '\0';
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Last RPP Award On:#0       %s\n", account->last_rpp > 0 ? date : "None Recorded");
		mem_free (date);
	}

	/* Retreive the number of stored Hobbitmails ( default = 0 ) */
	messages = 0 ;
	mysql_safe_query ("SELECT COUNT(*) FROM hobbitmail WHERE account = '%s'", account->name);
	if ( ( result = mysql_store_result(database) ) != NULL ) {
	  messages = ( ( row = mysql_fetch_row( result ) ) != NULL ) 
	    ? strtol( row[0], NULL,10 ) 
	    : 0 ;
	  mysql_free_result ( result ) ;
	  result = NULL;
	}
	else {
	  system_log("Warning: MySql returned a NULL result in staff.c::acctstat() while fetching hmail count", TRUE);
	}
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Stored Hobbitmails:#0      %d\n", messages);

	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Web Forum Posts:#0         %d\n", account->forum_posts);

	if ( account->flags ) {
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Account Flags Set:#0      ");
		if ( IS_SET (account->flags, ACCOUNT_NOPETITION) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " NoPetition");
		if ( IS_SET (account->flags, ACCOUNT_NOGUEST) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " NoGuest");
		if ( IS_SET (account->flags, ACCOUNT_NOPSI) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " NoPsi");
		if ( IS_SET (account->flags, ACCOUNT_NOBAN) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " NoBan");
		if ( IS_SET (account->flags, ACCOUNT_NORETIRE) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " NoRetire");
		if ( IS_SET (account->flags, ACCOUNT_NOVOTE) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " NoVoteReminder");
		if ( IS_SET (account->flags, ACCOUNT_IPSHARER) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " IPSharingOK");
		if ( IS_SET (account->flags, ACCOUNT_RPPDISPLAY) )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " RPPDisplay");
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
	}
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2ANSI Color Setting:#0      %s\n", account->color ? "On" : "Off");
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Downloaded Sourcecode:#0   %s\n", account->code ? "Yes" : "No");  
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "   #2Newsletter Status:#0       %s\n", account->newsletter ? "Subscribed" : "Opted Out");

	if ( account->pc ) {
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
		for ( pc = account->pc; pc; pc = pc->next ) {
			if ( strlen(buf) + strlen(line) >= MAX_STRING_LENGTH )
				break;
			snprintf (line, MAX_STRING_LENGTH, "   #2Registered Character:#0    %s\n", pc->name);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s", line);
		}
	}

	if ( account != NULL )
		free_account (account);

	page_string (ch->desc, buf);
}

void objstat (CHAR_DATA *ch, char *name)
{
	OBJ_DATA			*j = NULL;
	OBJ_DATA			*j2 = NULL;
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	char				buf2 [MAX_STRING_LENGTH] = {'\0'};
	char				copper_buf [MAX_STRING_LENGTH] = {'\0'};
	char *bow_type[] = { "Longbow", "Shortbow", "Crossbow", "Sling" };
	int					i =0 ;
	int					instance = 0;
	float				cost = 0;
	AFFECTED_TYPE		*af  = NULL;
	char				*temp_arg = NULL;
	


	
	if ( !*name && !strtol(name, NULL,10) ) {
		send_to_char ("Please specifiy an object name or vnum.\n", ch);
		return;
	}

	if ( just_a_number (name) && vtoo (strtol(name, NULL,10)) )
		j = vtoo (strtol(name, NULL,10));

	else if ( (j = get_obj_in_list_vis (ch, name, ch->right_hand)) ||
			(j = get_obj_in_list_vis (ch, name, ch->left_hand)) ||
			  (j = get_obj_in_list_vis (ch, name, ch->room->contents)) )
		instance = 1;

	else if ( !IS_NPC (ch) && !*name )
		j = vtoo (ch->pc->edit_obj);

	if ( !j ) {
		send_to_char ("No such object.\n", ch);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "\n#2VNUM:#0  %d [%s] [#2%s#0] [id %d]\n", j->virtual,
		instance ? "#2Instance#0" : "#2Prototype#0",
		item_types[(int)GET_ITEM_TYPE(j)], j->coldload_id);
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Keys: #0 %s\n", j->name);
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2SDesc:#0 %s\n#2LDesc:#0 %s\n\n", j->short_description,
		j->description);
	send_to_char(buf, ch);

	if ( j->obj_flags.type_flag == ITEM_INK ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#2Ink String:#0  %s\n", j->ink_color);
		send_to_char (buf, ch);
	}

	if ( j->obj_flags.wear_flags ) {
		snprintf (buf, MAX_STRING_LENGTH, "#2Wearbits:#0    ");
		for(i=0;(*wear_bits[i] != '\n');i++)
			if(IS_SET(j->obj_flags.wear_flags,(1<<i)))
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,"%s ",wear_bits[i]);
		strcat(buf,"\n");
		send_to_char(buf, ch);
	}

	if ( j->obj_flags.bitvector ) {
		snprintf (buf, MAX_STRING_LENGTH, "#2Bitvector:#0   ");
		for(i=0;(*affected_bits[i] != '\n');i++)
			if(IS_SET(j->obj_flags.bitvector,(1<<i)))
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,"%s ",affected_bits[i]);
		strcat(buf,"\n");
		send_to_char(buf, ch);
	}

	if ( j->obj_flags.extra_flags ) {
		snprintf (buf, MAX_STRING_LENGTH, "#2Extra Flags:#0 ");
		for(i=0;(*extra_bits[i] != '\n');i++)
			if(IS_SET(j->obj_flags.extra_flags,(1<<i)))
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,"%s ",extra_bits[i]);
		strcat(buf,"\n");
		send_to_char(buf,ch);
	}

	if ( j->econ_flags ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#2Econ Flags:#0  ");
		for ( i = 0; str_cmp (econ_flags [i], "\n"); i++ ) {
			if ( IS_SET (j->econ_flags, 1 << i) ) {
				strcat (buf, econ_flags [i]);
				strcat (buf, " ");
			}
		}
		strcat (buf, "\n");
		send_to_char (buf, ch);
	}

	*buf = 0;

	if ( j->size ) {
		if ( IS_WEARABLE (j) )
			snprintf (buf, MAX_STRING_LENGTH,  "#2Size:#0 %s (%s)\n",
							sizes_named [j->size], sizes [j->size]);
		else
			snprintf (buf, MAX_STRING_LENGTH,  "#2Size:#0 %s (%s) #1[N/A]#0\n",
							sizes_named [j->size], sizes [j->size]);
	}

	else if ( IS_WEARABLE (j) )
		snprintf (buf, MAX_STRING_LENGTH,  "#2Size:#0 Any\n");

	if ( *buf )
		send_to_char (buf, ch);

	if ( GET_ITEM_TYPE (j) == ITEM_FLUID )
		snprintf (copper_buf, MAX_STRING_LENGTH, "%d.%02df (%d)", (int)j->coppers / 100, (int)j->coppers % 100, (int)j->coppers);
	else
		snprintf (copper_buf, MAX_STRING_LENGTH, "%df", (int)j->coppers);

	cost = j->coppers;

	snprintf (buf, MAX_STRING_LENGTH,  "#2Weight:#0      %d.%02d", j->obj_flags.weight/100, j->obj_flags.weight % 100);
	pad_buffer (buf, 32);
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Full Weight:#0  %d.%02d", OBJ_MASS(j)/100, OBJ_MASS(j)%100);
	pad_buffer (buf, 61);
        snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Timer:#0  %d\n", j->obj_timer);
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "#2Cost:#0        %.02f cp", cost);
	pad_buffer (buf, 32);
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Quality:#0 %d", j->quality);
	pad_buffer (buf, 61);
        snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Cond.:#0  %d%%\n\n", j->item_wear);

	send_to_char (buf, ch);

	switch (j->obj_flags.type_flag) {
		case ITEM_FLUID:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval0 - Alcohol:#0 %d per sip\n"
						  "#2Oval1 - Water:#0   %d per sip\n"
						  "#2Oval2 - Food:#0    %d per sip\n",
						  j->o.fluid.alcohol,
						  j->o.fluid.water,
						  j->o.fluid.food);
			break;

		case ITEM_LIGHT: 

			if ( (j2 = vtoo (j->o.light.liquid)) ) {
				snprintf (buf2, MAX_STRING_LENGTH,  "%d (%s)", j2->virtual, j2->short_description);
				if ( GET_ITEM_TYPE (j2) != ITEM_LIQUID_FUEL )
					snprintf (buf2 + strlen (buf2), MAX_STRING_LENGTH, " [Need %s; this is %s]\n",
							 item_types [ITEM_LIQUID_FUEL],
							 item_types [(int) GET_ITEM_TYPE (j2)]);
			}

			else if ( j->o.light.v4 < 33 ) {
				sprinttype (j->o.light.v4, *drinks, buf2);
				strcat (buf2, " [NEEDS OBJECT ASSIGNED]");
			}

			else
				snprintf (buf2, MAX_STRING_LENGTH,  "%d (NO SUCH OBJECT EXISTS)",
							   j->o.light.liquid);

			snprintf (buf, MAX_STRING_LENGTH,              "#2Oval0 - Capacity:#0 %d\n"
						  "#2Oval1 - Hours:#0    %d\n"
						  "#2Oval2 - Liquid:#0   %s\n"
						  "#2Oval3 - On:#0       %d (0 is no, 1 is yes)\n",
						  j->o.light.capacity,
						  j->o.light.hours,
						  buf2,
						  j->o.light.on);
			break;

		case ITEM_FOOD :
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval0 - Food Value:#0 %d\n"
						  "#2Oval5 - Bites:#0 %3d\n",
					j->o.od.value [0],
					j->o.od.value [5]);
			break;
		
		case ITEM_WORN :
			snprintf (buf, MAX_STRING_LENGTH,  "#2MKeys:#0 %s\n", j->desc_keys);
			break;

		case ITEM_POTION:
		case ITEM_SCROLL:
		case ITEM_SALVE:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval0 - Dormancy 1:#0 %d IC Hours%s\n", j->o.od.value[0], !j->o.od.value[0] ? " (Instantaneous)" : "");
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval1 - Duration 1:#0 %d IC Hours%s\n", j->o.od.value[1], !j->o.od.value[1] ? " (Instantaneous)" : "");
			
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval3 - Dormancy 2:#0 %d IC Hours%s\n", j->o.od.value[3], !j->o.od.value[3] ? " (Instantaneous)" : "");
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval4 - Duration 2:#0 %d IC Hours%s\n", j->o.od.value[4], !j->o.od.value[4] ? " (Instantaneous)" : "");
			send_to_char (buf, ch);

			*buf = '\0';

			break;

		case ITEM_WEAPON :
			if ( j->o.weapon.use_skill != SKILL_SHORTBOW && j->o.weapon.use_skill != SKILL_LONGBOW 
			     && j->o.weapon.use_skill != SKILL_CROSSBOW && j->o.weapon.use_skill != SKILL_SLING )  {
			        snprintf (buf, MAX_STRING_LENGTH,  "#2Hands (Oval0):#0 #1[DEPRECATED - BASED ON WEAPON SKILL]#0\n");
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Damage (Oval1/2):#0 %dd%d\n", j->o.od.value[1], j->o.od.value[2]);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Weapon Type (Type):#0 %s\n", skill_data[j->o.od.value[3]].skill_name);
				if ( j->o.od.value[4] >= 0 && j->o.od.value[4] <= 4 )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Hit Theme (Oval4):#0 %d (%s)\n", j->o.od.value[4], weapon_theme[j->o.od.value[4]]);
				else snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Hit Theme (Oval4):#0 %d (None!)", j->o.od.value[4]);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Delay (Oval5):#0 %d\n", j->o.od.value[5]);
			}
			else {
				snprintf (buf, MAX_STRING_LENGTH,  "#2Hands (Oval0):#0 %d\n", j->o.od.value[0]);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Bow Type (Oval1):#0 %s\n", bow_type[j->o.od.value[1]]);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Accuracy Modifier (Oval2):#0 %d\n", j->o.od.value[2]);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Skill Used (Oval3):#0 %s [skill number %d]\n", skill_data[j->o.od.value[3]].skill_name, j->o.od.value[3]);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Damage Modifier (Oval4):#0 %d\n", j->o.od.value[4]);
			}
			break;

		case ITEM_ARMOR :
			snprintf (buf, MAX_STRING_LENGTH,  "#2AC-Apply:#0 [%d]\n#2Type:#0 [%d: %s]\n#2MKeys:#0 %s\n",
		 		j->o.od.value [0],j->o.od.value[1], armor_types[j->o.od.value[1]], j->desc_keys);
			break;

		case ITEM_NPC_OBJECT :
			snprintf (buf, MAX_STRING_LENGTH,  "#2NPC VNUM:#0 %d [#5%s#0]\n", j->o.od.value [0], j->o.od.value[0] > 0 ? vtom(j->o.od.value[0])->short_descr : "none");
			break;

		case ITEM_CONTAINER :
			if ( !IS_SET (j->obj_flags.extra_flags, ITEM_TABLE))
			snprintf (buf, MAX_STRING_LENGTH,              "#2Oval0 - Capacity:#0  %d  (%d.%.2d lbs)\n"
						 "#2Oval1 - Lockflags:#0 %d\n"
						 "#2Oval2 - Key#0:       %d\n"
						 "#2Oval3 - Pick Penl:#0 %d\n"
						 "#2Oval4 - N/A:#0       %d\n"
						 "#2Oval5 - Corpse:#0    %s\n",
					j->o.container.capacity,
					j->o.container.capacity / 100,
					j->o.container.capacity % 100,
					j->o.container.flags,
					j->o.container.key,
					j->o.container.pick_penalty,
					j->o.container.v4,
					j->virtual == VNUM_CORPSE ? "Yes" : "No");
			else
			snprintf (buf, MAX_STRING_LENGTH,              "#2Oval0 - Capacity:#0    %d  (%d.%.2d lbs)\n"
						 "#2Oval1 - Lockflags:#0   %d\n"
						 "#2Oval2 - Key:#0         %d\n"
						 "#2Oval3 - Pick Penl:#0   %d\n"
						 "#2Oval4 - N/A:#0         %d\n"
						 "#2Oval5 - Seating Cap:#0 %d\n",
					j->o.container.capacity,
					j->o.container.capacity / 100,
					j->o.container.capacity % 100,
					j->o.container.flags,
					j->o.container.key,
					j->o.container.pick_penalty,
					j->o.container.v4,
					j->o.container.table_max_sitting);

			break;

		case ITEM_DRINKCON:
		case ITEM_FOUNTAIN:

			if ( (j2 = vtoo (j->o.drinkcon.liquid)) ) {
				snprintf (buf2, MAX_STRING_LENGTH,  "%d (%s)", j2->virtual, j2->short_description);
				if ( GET_ITEM_TYPE (j2) != ITEM_FLUID )
					snprintf (buf2 + strlen (buf2), MAX_STRING_LENGTH, " [Need %s; this is %s]\n",
							 item_types [ITEM_FLUID],
							 item_types [(int) GET_ITEM_TYPE (j2)]);
			}

			else if ( j->o.drinkcon.liquid )
				strcat (buf2, " [None set]");

			else if ( j->o.drinkcon.liquid < 33 ) {
				sprinttype (j->o.drinkcon.liquid, *drinks, buf2);
				strcat (buf2, " [NEEDS OBJECT ASSIGNED]");
			}

			else
				snprintf (buf2, MAX_STRING_LENGTH,  "%d (NO SUCH OBJECT EXISTS)",
							   j->o.drinkcon.liquid);

			snprintf (buf, MAX_STRING_LENGTH,              "#2Oval0 - Capacity:#0 %d\n"
						  "#2Oval1 - Contains:#0 %d\n"
						  "#2Oval2 - Liquid:#0   %s\n",
						j->o.drinkcon.capacity,
						j->o.drinkcon.volume,
						buf2);
			break;

		case ITEM_KEY :
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval0 - Keytype:#0  %d\n", j->o.od.value[0]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval1 - Keyed To:#0 %d\n", j->o.od.value[1]);
			break;

		case ITEM_TICKET:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval0 - Ticket Number:#0 %d\n"
				      "#2Oval1 - Keeper VNUM:#0   %d\n",
						  j->o.ticket.ticket_num,
						  j->o.ticket.keeper_vnum);
			break;

		case ITEM_PERFUME:
			snprintf (buf, MAX_STRING_LENGTH,              "#2Oval0 - Perfume Type:#0 %d\n"
						  "#2Oval1 - Duration:#0     %d\n"
						  "#2Oval2 - Aroma Str:#0    %d\n"
						  "#2Oval3 - Doses:#0        %d\n",
							j->o.perfume.type,
							j->o.perfume.duration,
							j->o.perfume.aroma_strength,
							j->o.perfume.doses);
			break;

		case ITEM_QUIVER:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval0 - Missile Capacity:#0 %d\n",
							j->o.od.value[0]);
			break;

		case ITEM_KEYRING:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval0 - Key Capacity:#0 %d\n", j->o.od.value[0]);
			break;

		case ITEM_SHEATH:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval0 - Capacity:#0 %d (%d lbs.)\n", j->o.od.value[0], j->o.od.value[0]/100);
			break;

		case ITEM_MISSILE:
		case ITEM_BULLET:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Base Damage:#0 %dd%d\n", j->o.od.value[0], j->o.od.value[1]);
			break;

		case ITEM_INK:
	                snprintf (buf, MAX_STRING_LENGTH,  "#2Oval 0 -  Dips Left:#0  %d\n", j->o.od.value [0]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 1 -  Capacity:#0   %d\n", j->o.od.value[1]);
			break;

		case ITEM_HEALER_KIT:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval 0 -  Uses Remaining:#0  %d\n", j->o.od.value[0]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 1 -  Healing Bonus:#0   %d\n", j->o.od.value[1]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 2 -  Required Skill:#0  %d\n", j->o.od.value[2]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 3 -  Immediate Regen:#0 %d\n", j->o.od.value[3]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Treats These Wound Types:#0 ");
			if ( !j->o.od.value[5] || IS_SET (j->o.od.value[5], TREAT_ALL) ) {
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " All\n");
				break;
			}
			if ( IS_SET (j->o.od.value[5], TREAT_BLUNT) )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " Contusion");
			if ( IS_SET (j->o.od.value[5], TREAT_SLASH) )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " Slash");
			if ( IS_SET (j->o.od.value[5], TREAT_PUNCTURE) )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " Puncture");
			if ( IS_SET (j->o.od.value[5], TREAT_BURN) )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " Burn");
			if ( IS_SET (j->o.od.value[5], TREAT_FROST) )
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " Frost");
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
			break;

		case ITEM_REPAIR_KIT:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval 0 -  Uses Remaining:#0  %d\n", j->o.od.value[0]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 1 -  Mending Bonus:#0   %d\n", j->o.od.value[1]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 2 -  Required Skill:#0  %d\n", j->o.od.value[2]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Skill needed to use kit:#0   %s\n", 
				 (j->o.od.value[3] > 0) ? skill_data[(j->o.od.value[3])].skill_name : (j->o.od.value[3] < 0) ? "#1None#0" : "#1Any#0");
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Repairs This Item Types:#0   ");
			if ( j->o.od.value[5] == 0 ) {
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#1All#0\n");
				break;
			}
			else if ( j->o.od.value[5] < 0 ) {
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#1None#0\n");
				break;
			}
			else {
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s\n", item_types[(j->o.od.value[5])]);
				break;
			}

			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
			break;

		case ITEM_TENT:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval 0 -  Destination Room:#0 %d\n", j->o.od.value[0]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 1 -  Max Occupancy:#0    %d\n", j->o.od.value[1]);
			break;

		case ITEM_DWELLING:
			snprintf (buf, MAX_STRING_LENGTH,  "#2Oval 0 -  Destination Room:#0 %d\n", j->o.od.value[0]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 1 -  Max Occupancy:#0    %d\n", j->o.od.value[1]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 2 -  Lockflags:#0        %d\n", j->o.od.value[2]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 3 -  Key:#0              %d\n", j->o.od.value[3]);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Oval 4 -  Pick Penalty:#0     %d\n", j->o.od.value[4]);
			break;

		default :
			if ( j->virtual == 666 )
				snprintf (buf, MAX_STRING_LENGTH,  "#2Original Object VNUM:#0 %d\n", j->o.od.value[0]);
			else snprintf (buf, MAX_STRING_LENGTH, "#2Values 0-5:#0 [%d] [%d] [%d] [%d] [%d] [%d]\n",
				j->o.od.value [0],
				j->o.od.value [1],
				j->o.od.value [2],
				j->o.od.value [3],
				j->o.od.value [4],
				j->o.od.value [5]);
			break;
		}

	send_to_char(buf, ch);

	if ( j->contains ) {
		strcpy(buf, "\n#2Contains#0[ ");
		
		for(j2=j->contains;j2;j2 = j2->next_content) {
			temp_arg = fname(j2->name);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,"%s ", temp_arg);
		}
		strcat(buf,"]\n");
		send_to_char(buf, ch);
	}

	for ( af = j->xaffected; af; af = af->next ) {

		if ( af->type == MAGIC_HIDDEN )
			continue;
		if ( af->a.spell.location >= 10000 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "    #2Affects:#0 %s by %d\n",
					skill_data[af->a.spell.location-10000].skill_name,  af->a.spell.modifier);
			send_to_char (buf, ch);
		}
	}			

	if(j->clock || j->morphto) {
	    int month,day,hour;
	    
	    month = j->clock / (24*30);
	    hour = j->clock - month * 24*30;

	    day   = hour / 24;
	    hour -= day*24;
	    
	    snprintf (buf, MAX_STRING_LENGTH,  
		   "\n#2Morphing Information:#0\n#2Clock:#0   %d %d %d\n#2MorphTo:#0 %d\n",
		    month, day, hour, j->morphto);
	    send_to_char(buf, ch);

	    if(j->morphTime) {
		int delta, days, hours, minutes;

		delta = j->morphTime - time(0);

		days = delta/86400;
		delta -= days * 86400;

		hours = delta/3600;
		delta -= hours*3600;

		minutes = delta/60;
		
	        snprintf (buf, MAX_STRING_LENGTH,  "This object will morph in %d days, %d hours, and %d minutes\n", days, hours, minutes);
		send_to_char(buf, ch);
	    }
	}

	if ( j->count > 1 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "\n#2Count:#0 %d\n", j->count);
		send_to_char (buf, ch);
	}
}

/*                                                                          *
 * funtion: emailstat                    < e.g.> stat email null@bar.fu     *
 *                                                                          *
 * 09/17/2004 [JWW] - Fixed two instances where mysql result was not freed  *
 *                    by consolodating if / else / return logic             *
 *                                                                          */
void emailstat (CHAR_DATA *ch, char *argument)
{
    MYSQL_RES   *result = NULL;
    MYSQL_ROW   row;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			i = 0;

	if ( !*argument ) {
		send_to_char ("You'll need to either specify a partial or full email address to search for.\n", ch);
		return;
	}

	if ( strchr (argument, ';') ) {
		send_to_char ("You have included an illegal character in your query.\n", ch);
		return;
	}

	mysql_safe_query ("SELECT username, user_email FROM forum_users WHERE user_email LIKE '%%%s%%' ORDER BY username ASC", argument);
	result = mysql_store_result ( database ) ;
	if ( result == NULL || mysql_num_rows ( result ) == 0 ) {
	  send_to_char ("No login accounts were found with email addresses matching your query.\n", ch);
	}
	else if ( mysql_num_rows ( result ) > 255 ) {
	  send_to_char ("More than 255 records were found matching this query. Aborting.\n", ch);
	}
	else { 

	  *buf = '\0';
	  snprintf (buf, MAX_STRING_LENGTH,  "\n#6The following user accounts were found to match your query:#0\n\n");
	  while ( ( row = mysql_fetch_row ( result ) ) != NULL ) {
	    i++;
	    snprintf ( buf + strlen(buf), MAX_STRING_LENGTH, "   %-25s: %s\n", row[0], row[1] ) ;
	  }	  
	  snprintf (buf + strlen(buf), MAX_STRING_LENGTH, "\n#6Matching Records Found: %d#0\n", i ) ;
	  page_string ( ch->desc, buf ) ;

	}

	if ( result ) {
	  mysql_free_result ( result ) ;
	}
	else {
	  system_log ("Warning: MySql returned a NULL result in staff.c::emailstat() while fetching matching accounts.", TRUE);
	}
	return ;
}

void do_stat(CHAR_DATA *ch, char *argument, int cmd)
{
	char 	arg1[MAX_STRING_LENGTH] = {'\0'};
	char 	arg2[MAX_STRING_LENGTH] = {'\0'};

	half_chop(argument, arg1,arg2);

	if (!*arg1) {
		send_to_char("Invalid argument to stat\n\tUsage: stat <room|char|obj|acct|email> name\n",ch);
		return;
	} else {
		switch(*arg1) {
			case 'r':
				roomstat(ch,arg2);
				return;
			case 'c':
			case 'm':
				charstat(ch,arg2);
				return;
			case 'o':
				objstat(ch,arg2);
				return;
			case 'e':
				emailstat (ch, arg2);
				return;
			case 'a':
				acctstat(ch,arg2);
				return;
			default:
				send_to_char("Invalid argument to stat\n\tUsage: stat <room|char|obj|acct|email> name\n",ch);
				return;
		}

	}
}

void save_character_state (CHAR_DATA *tch)
{
	AFFECTED_TYPE	*af = NULL;
	PHASE_DATA		*phase = NULL;
	VIEWED_DATA		*sighted = NULL;
	char			craft_name [MAX_STRING_LENGTH] = {'\0'};
	int				phase_num = 0;	

	if ( tch->fighting && tch->fighting->in_room == tch->in_room ) {
		mysql_safe_query ("INSERT INTO copyover_fighting VALUES (%d, %d)", tch->coldload_id, tch->fighting->coldload_id);
	}

	if ( tch->aiming_at ) {
		mysql_safe_query ("INSERT INTO copyover_aiming VALUES (%d, %d)", tch->coldload_id, tch->aiming_at->coldload_id);
	}

	if ( tch->sighted ) {
		for ( sighted = tch->sighted; sighted; sighted = sighted->next ) {
			mysql_safe_query ("INSERT INTO copyover_sighted_targets VALUES (%d, %d)", tch->coldload_id, sighted->target->coldload_id);
		}
	}

	if ( tch->subdue && GET_FLAG (tch, FLAG_SUBDUER) ) {
		mysql_safe_query ("INSERT INTO copyover_subduers VALUES (%d, %d)", tch->coldload_id, tch->subdue->coldload_id);
	}
/*
	if ( tch->delay_type ) {
		if ( !tch->delay_who )
			snprintf (who, MAX_STRING_LENGTH, " ");
		else snprintf (who, MAX_STRING_LENGTH, "%s", tch->delay_who);
		mysql_safe_query ("INSERT INTO copyover_delays VALUES (%d, %d, %d, %d, %d, '%s')",
			tch->coldload_id, tch->delay, tch->delay_type, tch->delay_info1, tch->delay_info2, who);
	}
*/
	if ( tch->following ) {
		mysql_safe_query ("INSERT INTO copyover_followers VALUES (%d, %d)", tch->coldload_id, tch->following->coldload_id);
	}

	if ( (af = is_crafting (tch)) ) {
		phase_num = 0;
		for ( phase = af->a.craft->subcraft->phases; phase; phase = phase->next ) {
			phase_num++;
			if ( phase == af->a.craft->phase )
				break;
		}
		snprintf (craft_name, MAX_STRING_LENGTH, "%s %s", af->a.craft->subcraft->command, af->a.craft->subcraft->subcraft_name);
		mysql_safe_query ("INSERT INTO copyover_crafts VALUES (%d, '%s', %d, %d)",
		tch->coldload_id, craft_name, phase_num, af->a.craft->timer);
	}
}

void save_world_state (void)
{
	ROOM_DATA	*room = NULL;
	int		i;

	for ( room = full_room_list; room; room = room->lnext ) {
		for ( i = 0; i < 6; i++ ) {
			if ( !room->dir_option[i] )
				continue;
			if ( !IS_SET (room->dir_option[i]->exit_info, PASSAGE_ISDOOR) )
				continue;
			if ( IS_SET (room->dir_option[i]->exit_info, PASSAGE_CLOSED) &&
				IS_SET (room->dir_option[i]->exit_info, PASSAGE_LOCKED) )		/* 2 for locked and closed, 1 for closed, 0 for open. */
				mysql_safe_query ("INSERT INTO copyover_doors VALUES (%d, %d, 2)", room->virtual, i);
			else if ( IS_SET (room->dir_option[i]->exit_info, PASSAGE_CLOSED) )
				mysql_safe_query ("INSERT INTO copyover_doors VALUES (%d, %d, 1)", room->virtual, i);
			else mysql_safe_query ("INSERT INTO copyover_doors VALUES (%d, %d, 0)", room->virtual, i);
		}
	}
}

void prepare_copyover (CHAR_DATA *ch, int cmd)
{
	DESCRIPTOR_DATA		*d = NULL;
	DESCRIPTOR_DATA		*d_next = NULL;
	AFFECTED_TYPE		*af = NULL;
	FILE			*fp = NULL;
	OBJ_DATA		*obj = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	bool			obj_recorded = FALSE;
	int				count = 0;

	system_log ("Entering prepare_copyover() to set up the reboot.", FALSE);

	fp = fopen (".copyover_data", "w");

	mysql_safe_query ("DELETE FROM players_online");

	/* Prepare the soft reboot file for use after server bootup. */

	for ( d = descriptor_list; d; d = d_next ) {
		d_next = d->next;

		if ( !d->account || !d->character || d->connected != CON_PLYNG || (d->character && d->character->in_room == NOWHERE) ) {
			if ( d->character ) {
				if ( d->connected == CON_PLYNG )
					unload_pc (d->character);
				d->character = NULL;
			}
			write_to_descriptor (d, "\n\n#6We are rebooting the server - come back in a moment!#0\n\r\n");
			close_socket (d);
			continue;
		}
		if ( d->character && d->original ) {
			mysql_safe_query ("INSERT INTO copyover_animations VALUES (%d, %d)", d->original->coldload_id,
			d->character->coldload_id);
			do_return (d->character, "", 0);
		}
		if ( d->character ) {
			if ( cmd == 1 )
				write_to_descriptor (d, "\n#1The server is recovering from a crash. Please wait...#0\n\n");
			else
				write_to_descriptor (d, "\n#2The server is rebooting. Please wait...#0\n\n");
		}
		if ( d->character && d->connected == CON_PLYNG && d->character->room && d->character->in_room != NOWHERE ) {
			if ( GET_POS(d->character) == POSITION_FIGHTING ) 
				fprintf (fp, "%d %s %s %s %d", d->descriptor, d->host, d->account->name, d->character->tname, POSITION_STANDING);
			else
				fprintf (fp, "%d %s %s %s %d", d->descriptor, d->host, d->account->name, d->character->tname, GET_POS(d->character));
			count = 0;
			if ( GET_POS (d->character) == POSITION_SITTING && (af = get_affect (d->character, MAGIC_SIT_TABLE)) ) {
				if ( af->a.table.obj && af->a.table.obj->in_room == d->character->in_room ) {
					for ( obj = d->character->room->contents; obj; obj = obj->next_content ) {
						if ( str_cmp (af->a.table.obj->name, obj->name) )
							continue;
						else {
							count++;
							if ( obj == af->a.table.obj ) {
								fprintf (fp, " %d%s~\n", count, af->a.table.obj->name);
								obj_recorded = TRUE;
								break;
							}
						}
					}
				}
				remove_affect_type (d->character, MAGIC_SIT_TABLE);
			}
			if ( !obj_recorded )
				fprintf (fp, " 0none~\n");
			save_char (d->character, TRUE);
		}
		obj_recorded = FALSE;
	}

	fprintf (fp, "-1\n");
	fclose (fp);

	if ( port == BUILDER_PORT && cmd != 1 )
		do_zsave (NULL, "all", 0);

	save_player_rooms();

	if ( port == PLAYER_PORT ) {
		save_stayput_mobiles();
		save_tracks();
		save_banned_sites();
		save_world_state();
	}

	if ( port == BUILDER_PORT )
		update_crafts_file();
	
	snprintf (buf, MAX_STRING_LENGTH,  "%d", port);
	snprintf (buf2, MAX_STRING_LENGTH,  "%d", s);
	chdir ("..");
	execl ("bin/server", "bin/server", buf, "-c", buf2, (char *)NULL);
	system_log("execl() failed!", TRUE);
	chdir ("lib");

	send_to_gods ("Reboot execl() failed. Aborting and resuming normal operation... please do not attempt again.");
}

void check_reboot (void)
{
	DESCRIPTOR_DATA		*d = NULL;
	FILE			*fp = NULL;
	bool			block = FALSE;

	for ( d = descriptor_list; d; d = d->next ) {
		if ( !d->character )
			continue;
		if ( !d->character->pc )
			continue;
		if ( d->connected != CON_PLYNG && d->character->pc->nanny_state )
			block = TRUE;
                if ( d->character->pc->create_state == STATE_APPLYING )  
                        block = TRUE;
		if ( IS_SET (d->character->act, PLR_QUIET) )
			block = TRUE;
		if ( d->character->fighting && IS_NPC (d->character->fighting) )
			block = TRUE;
		if ( d->character->subdue && IS_NPC (d->character->subdue) )
			block = TRUE;
		if ( d->character->aiming_at && IS_NPC (d->character->aiming_at) )
			block = TRUE;
		if ( d->character->following && IS_NPC (d->character->following) )
			block = TRUE;
	}

	if ( !block ) {
		if ( !(fp = fopen (".reboot", "w")) ) {
			system_log ("Error creating reboot file.", TRUE);
			return;
		}
		fprintf (fp, "Reboot Queue\n");
		fclose (fp);
		unlink ("booting");
		prepare_copyover (NULL, 0);
	}
}

void do_shutdown (CHAR_DATA *ch, char *argument, int cmd)
{
	DESCRIPTOR_DATA		*d = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		arg [MAX_STRING_LENGTH] = {'\0'};
	FILE		*fs = NULL;
	bool		block = FALSE;

	argument = one_argument(argument, arg);

	if ( !*arg ) {
        	send_to_char ("Specify a shutdown mode please.", ch);
        	return;
    	}

	if ( !str_cmp (arg, "die") ) {
		system ("touch booting");
		snprintf (buf, MAX_STRING_LENGTH,  "%s is shutting down the server...\n", GET_NAME(ch));
		send_to_gods (buf);
		system_log(buf, FALSE);
		snprintf (buf, MAX_STRING_LENGTH,  "\n#1--- ANNOUNCEMENT ---#0\n#1We are shutting the server down for maintenance. Please check\nback later; our apologies for the inconvenience.#0\n#1--------------------\n#0");
		send_to_all_unf (buf);
		shutd = 1;
		if ( (fs=fopen (".reboot", "w")) == NULL ) {
			system_log ("Error creating shutdown file.", TRUE);
			return;
		}
		if ( ch->desc->original )
			snprintf (buf, MAX_STRING_LENGTH, "%s\n",GET_NAME(ch->desc->original));
		else
			snprintf (buf, MAX_STRING_LENGTH, "%s\n",GET_NAME(ch));
		fputs(buf,fs);
		fclose(fs);
	}
	else if ( !str_cmp (arg, "stop") ) {
		if ( !pending_reboot ) {
			send_to_char ("No reboot is currently pending.\n", ch);
			return;
		}
		pending_reboot = FALSE;
		snprintf (buf, MAX_STRING_LENGTH,  "%s has cancelled the pending reboot.", ch->tname);
		send_to_gods(buf);
	}
	else if (!str_cmp(arg, "reboot"))
	{
		if ( port == PLAYER_PORT && GET_TRUST (ch) < 5 ) {
			send_to_char ("You'll need to wait for the 4 AM PST auto-reboot.\n", ch);
			return;
		}

		for ( d = descriptor_list; d; d = d->next ) {
			if ( !d->character )
				continue;
			if ( d->character && IS_NPC (d->character) ) {
				continue;
			}	
			if ( d->connected != CON_PLYNG && d->character->pc->nanny_state )
				block = TRUE;
	                if ( d->character->pc->create_state == STATE_APPLYING )  
        	                block = TRUE;
			if ( IS_SET (d->character->act, PLR_QUIET) )
				block = TRUE;
			if ( d->character->fighting && IS_NPC (d->character->fighting) )
				block = TRUE;
			if ( d->character->subdue && IS_NPC (d->character->subdue) )
				block = TRUE;
			if ( d->character->aiming_at && IS_NPC (d->character->aiming_at) )
				block = TRUE;
			if ( d->character->following && IS_NPC (d->character->following) )
				block = TRUE;
		}

		argument = one_argument (argument, arg);
		if ( block && *arg != '!' ) {
			if ( pending_reboot ) {
				send_to_char ("There is already a reboot pending.\n", ch);
				return;
			}
			send_to_char ("\n", ch);
			snprintf (buf, MAX_STRING_LENGTH,  "%s has queued a server reboot.", ch->tname);
			send_to_gods (buf);
			pending_reboot = TRUE;
			return;
		}
		if ( !(fs = fopen (".copyover_data", "w")) ) {
			send_to_char ("#1Copyover file not writeable; reboot aborted. Please notify the staff.#0\n", ch);
			system_log ("Error creating copyover file.", TRUE);
		}
		fclose (fs);
		if ( !(fs = fopen (".reboot", "w")) ) {
			system_log ("Error creating reboot file.", TRUE);
			return;
		}
		if ( ch->desc->original )
			snprintf (buf, MAX_STRING_LENGTH, "%s\n",GET_NAME(ch->desc->original));
		else
			snprintf (buf, MAX_STRING_LENGTH, "%s\n",GET_NAME(ch));
		fputs (buf, fs);
		fclose (fs);
		unlink ("booting");
		prepare_copyover(ch, 0);
	}
	else
		send_to_char("Specify shutdown parameter:  Die or Reboot ?\n", ch);
}

void clear_watch (CHAR_DATA *ch)
{
	CHAR_DATA		*tch = NULL;
	AFFECTED_TYPE	*af = NULL;

	for ( tch = character_list; tch; tch = tch->next ) {

		if ( tch->deleted || IS_NPC (tch) )
			continue;

		if ( (af = get_affect (tch, MAGIC_WATCH1)) &&
			  af->a.spell.t == (int) ch )
			affect_remove (tch, af);

		if ( (af = get_affect (tch, MAGIC_WATCH2)) &&
			  af->a.spell.t == (int) ch )
			affect_remove (tch, af);

		if ( (af = get_affect (tch, MAGIC_WATCH3)) &&
			  af->a.spell.t == (int) ch )
			affect_remove (tch, af);
	}
}

void do_watch (CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	AFFECTED_TYPE	*af = NULL;
	CHAR_DATA		*tch = NULL;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		clear_watch (ch);
		send_to_char ("Ok.\n", ch);
		return;
	}

	if ( !(tch = get_pc (buf)) ) {
		send_to_char ("PC not found.  Did you enter the full name?\n", ch);
		return;
	}

	if ( (af = get_affect (tch, MAGIC_WATCH1)) && af->a.spell.t == (int) ch ) {
		send_to_char ("Watch removed.\n", ch);
		affect_remove (tch, af);
		return;
	}

	if ( (af = get_affect (tch, MAGIC_WATCH2)) && af->a.spell.t == (int) ch ) {
		send_to_char ("Watch removed.\n", ch);
		affect_remove (tch, af);
		return;
	}

	if ( (af = get_affect (tch, MAGIC_WATCH3)) && af->a.spell.t == (int) ch ) {
		send_to_char ("Watch removed.\n", ch);
		affect_remove (tch, af);
		return;
	}

	if ( !get_affect (tch, MAGIC_WATCH1) ) {
		magic_add_affect (tch, MAGIC_WATCH1, -1, 0, 0, 0, 0);
		get_affect (tch, MAGIC_WATCH1)->a.spell.t = (int) ch;
		send_to_char ("Watch added.\n", ch);
	}

	else if ( !get_affect (tch, MAGIC_WATCH2) ) {
		magic_add_affect (tch, MAGIC_WATCH2, -1, 0, 0, 0, 0);
		get_affect (tch, MAGIC_WATCH2)->a.spell.t = (int) ch;
		send_to_char ("Watch added.\n", ch);
	}

	else if ( !get_affect (tch, MAGIC_WATCH3) ) {
		magic_add_affect (tch, MAGIC_WATCH3, -1, 0, 0, 0, 0);
		get_affect (tch, MAGIC_WATCH3)->a.spell.t = (int) ch;
		send_to_char ("Watch added.\n", ch);
	}

	else {
		send_to_char ("Three other people watching...try snooping.\n", ch);
		return;
	}
}

void do_snoop (CHAR_DATA *ch, char *argument, int cmd)
{
	char		arg [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*victim = NULL;

	if (!ch->desc)
		return;

	(void)one_argument(argument, arg);

	if ( !*arg )
		strcpy (arg, GET_NAME (ch));

	if ( IS_NPC (ch) ) {
		send_to_char ("Mixing snoop and switch is bad for your health.\n", ch);
		return;
	}

	if ( !(victim = get_char_vis (ch, arg)) ) {
		send_to_char ("No such person around.\n", ch);
		return;
	}

	if ( !victim->desc ) {
		send_to_char ("There's no link.. nothing to snoop.\n", ch);
		return;
	}

	if ( !victim->pc ) {
		send_to_char ("Sorry... can only snoop PCs!\n", ch);
		return;
	}

	if ( victim == ch ) {

		send_to_char ("You stop snooping.\n", ch);

		if ( ch->desc->snoop.snooping ) {
			if ( ch->desc->snoop.snooping->pc->level ) {
				snprintf (buf, MAX_STRING_LENGTH,  "%s stops snooping you.", GET_NAME (ch));
				send_to_char (buf, ch->desc->snoop.snooping);
			}
			ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
			ch->desc->snoop.snooping = 0;
		}

		return;
	}

	if ( victim->desc->snoop.snoop_by ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s is snooping already.",
					  GET_NAME (victim->desc->snoop.snoop_by));
		send_to_char (buf, ch);
		return;
	}

	if ( victim->pc->level ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s is snooping you.", GET_NAME (ch));
		send_to_char (buf, victim);
	}

	send_to_char ("Ok.\n",ch);

	if(ch->desc->snoop.snooping)
		ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

	ch->desc->snoop.snooping = victim;
	victim->desc->snoop.snoop_by = ch;
	return;
}

void do_summon (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*victim = NULL;
	AFFECTED_TYPE	*af = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Summon whom?\n", ch);
		return;
	}

	victim = get_char (buf);

	if ( !victim ) {
		send_to_char ("Couldn't find that character.\n", ch);
		return;	
	}

	if ( victim == ch ) {
		send_to_char ("You cannot summon yourself.\n", ch);
		return;
	}

	if ( victim->in_room == ch->in_room ) {
		act ("$N is already here.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	if ( (af = get_affect (victim, MAGIC_STAFF_SUMMON)) )
		af->a.spell.t = ch->in_room;
	else {
		magic_add_affect (victim, MAGIC_STAFF_SUMMON, 24, 0, 0, 0, 0);
		af = get_affect (victim, MAGIC_STAFF_SUMMON);
		af->a.spell.t = victim->in_room;
	}

	char_from_room (victim);
	char_to_room (victim, ch->in_room);

	if ( GET_HIT (victim) <= 0 )
		GET_HIT (victim) = 1;

	if ( !AWAKE (victim) )
		GET_POS (victim) = STAND;

	act ("$N has been summoned.", FALSE, ch, 0, victim, TO_CHAR);
	act ("$n summons $N.", FALSE, ch, 0, victim, TO_NOTVICT);
	act ("You have been summoned by $N.", TRUE, victim, 0, ch, TO_CHAR);

	send_to_char ("Use the RETURN command to return to your former location "
				  "when you are ready.\n", victim);
}

void do_switch(CHAR_DATA *ch, char *argument, int cmd)
{
	char			arg [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA		*victim = NULL;

	(void)one_argument(argument, arg);
	
	if (!*arg) {
		do_switch_item (ch, argument, cmd);
		return;
	}

	if (IS_NPC(ch))
		return;

	if ( IS_MORTAL (ch) ) {
		send_to_char ("Eh?\n", ch);
		return;
	}

	if ( ch->pc && ch->pc->create_state == STATE_DIED ) {
		send_to_char ("Huh?\n", ch);
		return;
	}

	if (!(victim = get_char_room_vis(ch,arg)))
		if(!(victim=get_char(arg))) {
		 	send_to_char("They aren't here.\n", ch);
			return;
		}

		if (ch == victim) {
			send_to_char("Seek some help bud, no multi-personality disorders allowed here.\n", ch);
			return;
		}

		if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping) {
			send_to_char("Mixing snoop & switch is bad for your health.\n", ch);
			return;
		}

		if(victim->desc || (!IS_NPC(victim) && !victim->pc->admin_loaded))
			send_to_char("You can't do that, the body is already in use!\n", ch);
		else {
			send_to_char("Ok.\n", ch);
			
			ch->desc->character = victim;
			ch->desc->original = ch;
			if ( ch->desc->original->color )
				victim->color = 1;

			victim->desc = ch->desc;
			ch->desc = 0;
		}
}

void do_return (CHAR_DATA *ch, char *argument, int cmd)
{
	AFFECTED_TYPE	*af = NULL;

	if ( ch->desc ) {
		snprintf (s_buf, MAX_STRING_LENGTH,  "return ch->desc->original: %d",
						(int) ch->desc->original);
	}

	if ( ch->desc && ch->desc->original &&
		 is_he_somewhere (ch->desc->original) ) {
		send_to_char("You return to your original character.\n",ch);
		ch->desc->character = ch->desc->original;
		ch->desc->original = NULL;
		ch->desc->character->desc = ch->desc; 
		ch->desc = NULL;
		return;
	}

	if ( (af = get_affect (ch, MAGIC_STAFF_SUMMON)) ) {
		act ("$n unsummons $mself.", FALSE, ch, 0, 0, TO_ROOM);
		char_from_room (ch);
		char_to_room (ch, af->a.spell.t);
		act ("$n appears.", FALSE, ch, 0, 0, TO_ROOM);
		do_look (ch, "", 0);
		affect_remove (ch, af);
		return;
	}

	if ( IS_MORTAL (ch) )
		send_to_char ("You are where you should be.\n", ch);
	else
		send_to_char ("You are what you should be.\n", ch);
}

void do_force(CHAR_DATA *ch, char *argument, int cmd)
{
	struct descriptor_data 	*i = NULL;
	CHAR_DATA 				*vict = NULL;
	char 				name[100], to_force[100], buf[100]; 

	half_chop(argument, name, to_force);

	if (!*name || !*to_force)
		 send_to_char("Who do you wish to force to do what?\n", ch);
	else if ( str_cmp("all", name) || GET_TRUST(ch) < 5 ) {
		if (!(vict = get_char_room_vis(ch, name)))
			send_to_char("No-one by that name here.\n", ch);
		else
		{
			if ( vict == ch ) {
				send_to_char ("Force yourself?\n", ch);
				return;
			}
			if (GET_TRUST(ch) < GET_TRUST(vict))
				vict = ch;
			snprintf (buf, MAX_STRING_LENGTH,  "$n has forced you to '%s'.", to_force);
			act(buf, FALSE, ch, 0, vict, TO_VICT);
			send_to_char("Ok.\n", ch);
			command_interpreter(vict, to_force);
		}
	} else { /* force all */
    		for (i = descriptor_list; i; i = i->next) {
			if (i->character != ch && !i->connected) {
				vict = i->character;
				if (GET_TRUST(ch) < GET_TRUST(vict))
					send_to_char("Oh no you don't!!\n", ch);
				else {
					snprintf (buf, MAX_STRING_LENGTH,  "$n has forced you to '%s'.", to_force);
					act(buf, FALSE, ch, 0, vict, TO_VICT);
					command_interpreter(vict, to_force);
				}
			}
		}
		send_to_char("Ok.\n", ch);
	}
}

void do_as (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	DESCRIPTOR_DATA	*save_d = NULL;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Syntax:  as <character> <command>\n", ch);
		return;
	}

	if ( !(tch = get_char_room_vis (ch, buf)) ) {
		send_to_char ("They aren't here.\n", ch);
		return;
	}

	if ( !IS_NPC (tch) && !str_cmp (tch->tname, IMPLEMENTOR_ACCOUNT) ) {
		tch = ch;
	}

	if ( !IS_NPC(tch) && !IS_NPC(ch) ) {
		if ( tch->pc->level > ch->pc->level ) {
			send_to_char ("I'm sure they wouldn't appreciate that.\n", ch);
			return;
		}
	}

	(void)one_argument (argument, buf);

	if ( !str_cmp (buf, "switch") ) {
		send_to_char ("Naughty genie...\n", ch);
		return;
	}

	if ( !str_cmp (buf, "quit") ) {
		send_to_char ("Oh, not quit...that would crash the mud...use force "
					  "instead.\n", ch);
		return;
	}

	save_d = tch->desc;
	tch->desc = ch->desc;

	command_interpreter (tch, argument);

	tch->desc = save_d;
}

void do_load (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*mob = NULL;
	OBJ_DATA	*obj = NULL;
	OBJ_DATA	*bow = NULL;
	OBJ_DATA	*ammo = NULL;
	OBJ_DATA	*quiver = NULL;
	ROOM_DATA	*room = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buffer [MAX_STRING_LENGTH] = {'\0'};
	char		arg1 [MAX_STRING_LENGTH] = {'\0'};
	char		arg2 [MAX_STRING_LENGTH] = {'\0'};
	char		type [MAX_STRING_LENGTH] = {'\0'};
	char		num [MAX_STRING_LENGTH] = {'\0'};
	int			number = 0;
	int			i = 0;
	int			ind = 0;
	int			count = 0;
	int			quiver_used = 0;
	int			ammo_found = 0;
	char		*temp_char = NULL;
	char		*temp_obj = NULL;
	
	argument = one_argument (argument, type);
	argument = one_argument (argument, num);

	snprintf (arg1, MAX_STRING_LENGTH, "%s", type);
	snprintf (arg2, MAX_STRING_LENGTH, "%s", num);

	if ( !ammo_found || *argument ) {
		if ( (obj = get_obj_in_dark (ch, arg1, ch->right_hand)) ||
			(obj = get_obj_in_dark (ch, arg1, ch->left_hand)) ) {
			if ( GET_ITEM_TYPE (obj) == ITEM_WEAPON )
				*arg1 = '\0';
			obj = NULL;
		}	
		if ( *arg1 ) {
			if ( !str_cmp (arg2, "from") )
				argument = one_argument (argument, arg2);
			if ( !*arg2 ) {
				for ( i = 0; i < MAX_WEAR; i++ ) {
					if ( !(quiver = get_equip (ch, i)) )
						continue;
					if ( GET_ITEM_TYPE (quiver) == ITEM_QUIVER && get_obj_in_list_vis (ch, arg1, quiver->contains) )
						break;
				}
			}
			else {
				for ( i = 0; i < MAX_WEAR; i++ ) {
					if ( !(quiver = get_equip (ch, i)) )
						continue;
					if ( GET_ITEM_TYPE (quiver) == ITEM_QUIVER && name_is (arg2, quiver->name) )
						break;
				}
			}
			if ( quiver ) {
				for ( ammo = quiver->contains; ammo; ammo = ammo->next_content ) {
					if ( GET_ITEM_TYPE (ammo) == ITEM_MISSILE && name_is (arg1, ammo->name) ) {
						ammo_found++;
						quiver_used++;
						break;
					}
				}
			}
		}
		else {
			for ( i = 0; i < MAX_WEAR; i++ ) {
				if ( !(quiver = get_equip (ch, i)) )
					continue;
				if ( GET_ITEM_TYPE (quiver) == ITEM_QUIVER ) {
					for ( ammo = quiver->contains; ammo; ammo = ammo->next_content ) {
						if ( GET_ITEM_TYPE (ammo) == ITEM_MISSILE ) {
							ammo_found++;
							quiver_used++;
							ch->delay_info2 = ammo->virtual;
							ch->delay_info1 = quiver->virtual;
							break;
						}
					if ( quiver_used )
						break;
					}
				}
			}
		}
	}

	if ( (!ammo || !ammo_found) && !*type && !*num ) {
		send_to_char ("You must have the ammunition you wish to load it with in your inventory.\n", ch);
		return;
	}
	
	if ( ammo_found ) {
		if ( !(bow = get_equip(ch, WEAR_BOTH)) ) {
			send_to_char ("You must first be wielding the weapon you wish to load.\n", ch);
			return;
		}

		if ( bow && bow->loaded ) {
			send_to_char ("That's already loaded.\n", ch);
			return;
		}

		else if ( bow ) {
			if ( bow->o.weapon.use_skill != SKILL_SHORTBOW &&
			     bow->o.weapon.use_skill != SKILL_LONGBOW &&
			     bow->o.weapon.use_skill != SKILL_CROSSBOW ) {
				send_to_char ("You can't load that!\n", ch);
				return;
			}
		}

		if ( !cmp_strn (ammo->name, "arrow", 5) &&
		     !(bow->o.weapon.use_skill == SKILL_LONGBOW || bow->o.weapon.use_skill == SKILL_SHORTBOW) ) {
			send_to_char ("This isn't the right sort of ammunition. A bow uses arrows.\n", ch);
			return;
		}

		if ( !cmp_strn (ammo->name, "bolt", 4) &&
		     bow->o.weapon.use_skill != SKILL_CROSSBOW ) {
			send_to_char ("This isn't the right sort of ammunition. A crossbow uses bolts.\n", ch);
			return;
		}	

		ch->delay_info2 = ammo->virtual;
		if ( quiver )
			ch->delay_info1 = quiver->virtual;

		if ( bow && bow->o.weapon.use_skill == SKILL_CROSSBOW ) {
			temp_obj = obj_short_desc (bow);
			snprintf (buf, MAX_STRING_LENGTH,  "You begin the somewhat laborious task of loading #2%s#0 with #2%s#0.", temp_obj, ammo->short_description);
			act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			temp_char = char_short (ch);
			temp_obj = obj_short_desc (bow);
			snprintf (buf, MAX_STRING_LENGTH,  "%s#0 begins the somewhat laborious task of loading #2%s#0 with #2%s#0.", temp_char, temp_obj, ammo->short_description);
			*buf = toupper (*buf);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
			snprintf (buf, MAX_STRING_LENGTH,  "%s", buffer);
			act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			ch->delay = 15 - ch->skills [SKILL_CROSSBOW]/20;
			ch->delay_type = DEL_LOAD_WEAPON;
			ch->delay_who = ammo->name;
			return;
		}
		else if ( bow && ( bow->o.weapon.use_skill == SKILL_SHORTBOW || bow->o.weapon.use_skill == SKILL_LONGBOW ) ) {
			temp_obj = obj_short_desc(bow);
			snprintf (buf, MAX_STRING_LENGTH,  "You nock #2%s#0, drawing back upon #2%s#0.", ammo->short_description, temp_obj);
			act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			temp_obj = obj_short_desc(bow);
			temp_char = char_short (ch);
			snprintf (buf, MAX_STRING_LENGTH,  "%s#0 nocks #2%s#0 and draws back upon #2%s#0.", temp_char, ammo->short_description, temp_obj);
			*buf = toupper (*buf);
			snprintf (buffer, MAX_STRING_LENGTH,  "#5%s", buf);
			snprintf (buf, MAX_STRING_LENGTH,  "%s", buffer);
			act (buf, FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
			ch->delay = 4 - ch->skills [bow->o.weapon.use_skill]/30;
			ch->delay = MAX (ch->delay, 2);
			ch->delay_type = DEL_LOAD_WEAPON;
			ch->delay_who = ammo->name;
			return;
		}
	}

	if ( IS_MORTAL (ch) ) {
		send_to_char ("You must have a weapon equipped and a missile available.\n", ch);
		return;
	}

	if ( !*type || !*num || *type == '?' ) {
		send_to_char ("Syntax:\n"
					  "   load <'char' | 'obj'> <number>\n"
					  "   load <'player'> <name>\n"
					  "   load <'money'> <amount>\n", ch);
		return;
	}

	number = strtol(num, NULL,10);

	if ( number < 0 || number > 99999 ) {
		send_to_char("Vnum must be from 0 to 99999\n", ch);
		return;
	}

	else if ( is_abbrev (type, "character") || is_abbrev (type, "mobile") ) {
		if ( !(mob = load_mobile (number)) ) {
			send_to_char("There is no mobile with that number.\n", ch);
			return;
 		}

		char_to_room (mob, ch->in_room);
		mob->mob->spawnpoint = ch->in_room;

		act ("$n hastily completes a blue requisition form.",
						TRUE, ch, 0, 0, TO_ROOM);
		act ("$N immediately appears!", FALSE, ch, 0, mob, TO_ROOM);
		act ("$N appears.", FALSE, ch, 0, mob, TO_CHAR);

	}

	else if ( is_abbrev (type, "player") ) {
		if ( port != TEST_PORT ) {
			send_to_char ("Temporarily disabled, to see if this alleviates the crashing...\n", ch);
			return;
		}

		snprintf (buf, MAX_STRING_LENGTH,  "%s", num);

		if ( !*buf ) {
			send_to_char ("Which PC did you wish to load?\n", ch);
			return;
		}

                CREATE (mob->pc, PC_DATA, 1);
                mob = load_char_mysql (buf);
                if ( mob ) {
	                load_char_objs (mob, mob->tname);
       	        	mob->pc->admin_loaded = TRUE;
			SET_BIT (mob->act, ACT_ISNPC);
                        char_to_room (mob, ch->in_room);
			act ("$n hastily completes a green requisition form.",
							TRUE, ch, 0, 0, TO_ROOM);
			act ("$N immediately appears!", FALSE, ch, 0, mob, TO_ROOM);
			act ("$N appears.", FALSE, ch, 0, mob, TO_CHAR);
			return;
		}
		else {
			send_to_char ("That PC could not be loaded from the database.\n", ch);
			return;
		}
	}

	else if (is_abbrev(type, "obj")) {

			/* if load o <count> <vnum>, we need to check for <vnum> */

		argument = one_argument (argument, buf);

		if ( isdigit (*buf) ) {
			count = number;
			number = strtol(buf, NULL, 10);
			argument = one_argument (argument, buf);
		} else
			count = 1;

		obj = NULL;

		if ( *buf ) {
			if ( (ind = index_lookup (standard_object_colors, buf)) != -1 )
				ch->delay_who = add_hash(standard_object_colors[ind]);
			else if ( (ind = index_lookup (fine_object_colors, buf)) != -1 )
				ch->delay_who = add_hash(fine_object_colors[ind]);
			else if ( (ind = index_lookup (drab_object_colors, buf)) != -1 )
				ch->delay_who = add_hash(drab_object_colors[ind]);
			if ( ind != -1 ) {
				if ( !(obj = load_colored_object (number, ch->delay_who)) ) {
					send_to_char ("There is no object with that number.\n", ch);
					return;
				}
				argument = one_argument (argument, buf);
			}
			else {
				if ( !(obj = load_object (number)) ) {
					send_to_char ("There is no object with that number.\n", ch);
					return;
				}
			}
		}
		else {
			if ( !(obj = load_object (number)) ) {
				send_to_char ("There is no object with that number.\n", ch);
				return;
			}
		}

		ch->delay_who = NULL;

		obj->count = count;

		mob = NULL;

		if ( *buf ) {
			if ( !(mob = get_char_room_vis (ch, buf)) ) {
				send_to_char ("Mob was not located.\n", ch);
				extract_obj (obj);
				return;
			}
		}

		if ( mob ) {
			act ("$N hastily completes a red requisition form;\n"
				 "   $p appears in your hands.",
						TRUE, mob, obj, ch, TO_CHAR);
			if ( ch != mob )
				act ("$p was created in $N's hands.",
						TRUE, ch, obj, mob, TO_CHAR);
			obj_to_char (obj, mob);
		} else {
			obj_to_room (obj, ch->in_room);

			act ("$n hastily completes a red requisition form.",
						TRUE, ch, 0, 0, TO_ROOM);
			act ("$p immediately appears!", FALSE, ch, obj, 0, TO_ROOM);
			act ("$p appears.", FALSE, ch, obj, 0, TO_CHAR);
		}
	}
	else
		send_to_char ("Use keyword char, obj or money.\n", ch);

	room = vtor (ch->in_room);

}

/* clean a room of all mobiles and objects */
void do_purge(CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA *vict, *next_v;
	OBJ_DATA *obj, *next_o;
	char name[100];

	if (IS_NPC(ch))
		return;

	(void)one_argument(argument, name);

	if (*name)  /* argument supplied. destroy single object or char */
	{
		if ( (vict = get_char_room_vis (ch, name)) ) {
			if ( !IS_NPC (vict) && !vict->pc->admin_loaded ) {
				send_to_char ("\nNow, now. Let's not be hostile, mmmkay?\n", ch);
				return;
			}
			act ("$n disintegrates $N.", FALSE, ch, 0, vict, TO_ROOM);
			if ( vict->mount )
				vict->mount->mount = NULL;
			vict->mount = NULL;
			extract_char(vict);
		}

		else if ((obj = get_obj_in_list_vis(ch, name,
								vtor (ch->in_room)->contents))) {
			act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
			extract_obj(obj);
		}
		else
		{
			send_to_char("I don't know anyone or anything by that name.\n", ch);
			return;
		}

		send_to_char("Ok.\n", ch);
	}
	else   /* no argument. clean out the room */
	{
		if (IS_NPC(ch))
		{
			send_to_char("Don't... You would only kill yourself..\n", ch);
			return;
		}

		if ( !ch->room->contents ) {
			send_to_char ("There aren't any objects that need purging in this room.\n", ch);
			return;
		}

		act ("$n briefly narrows $s eyes. A moment later, the clutter in the area dissipates away in a glimmering haze of binary code.",
			FALSE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		act ("You narrow your eyes, concentrating. A moment later, the clutter in the area dissipates away in a glimmering haze of binary code.",
			FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

		for (vict = vtor (ch->in_room)->people; vict; vict = next_v)
		{
			next_v = vict->next_in_room;
			if (IS_NPC(vict)) {
				act ("$N is a master mobile.  Kill it if you really want "
						 "it destroyed.", FALSE, ch, 0, vict, TO_CHAR | 
						 TO_ACT_FORMAT);
				
			}
		}

		for (obj = vtor (ch->in_room)->contents; obj; obj = next_o) {
			next_o = obj->next_content;
			if ( GET_ITEM_TYPE (obj) == ITEM_TENT || GET_ITEM_TYPE (obj) == ITEM_DWELLING )
				continue;
			extract_obj(obj);
		}
	}
}

void do_restore(CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA *victim;
	char buf[100];

	(void)one_argument (argument, buf);
	if ( !*buf )
		send_to_char("Who do you wish to restore?\n",ch);
	else {
		if(!(victim = get_char_room_vis(ch, buf)))
			send_to_char("I don't see that person here.\n",ch);
		else {
			heal_all_wounds (victim);
			victim->move = victim->max_move;
			send_to_char ("Done.\n", ch);
			act ("You have been fully healed by $N!",
					FALSE, victim, 0, ch, TO_CHAR);
			GET_POS(victim) = STAND;
		}
	}
}

void do_zlock(CHAR_DATA *ch, char *argument, int cmd)
{
	char arg1[80], arg2[80];
	int zone;

	half_chop(argument, arg1,arg2);

	if(!*arg1) {
		send_to_char("Which zone would you like to lock?\n",ch);
		return;
	}
	if(!*arg2) {
		send_to_char("Turn lock on or off?\n",ch);
		return;
	}
	zone = strtol(arg1, NULL, 10);
	if((zone < 0) || (zone >99)) {
		send_to_char("Zone must be between 0 and 99.\n",ch);
		return;
	}
	if(!strncmp(arg2,"on",2)) {
		if(IS_SET(zone_table[zone].flags, Z_LOCKED)) {
			send_to_char("Zone is already locked.\n",ch);
			return;
		} else {
			SET_BIT(zone_table[zone].flags,Z_LOCKED);
			send_to_char("Zone Locked.\n",ch);
			return;
		}
	} else if(!strncmp(arg2,"of",2)) {
		if(!IS_SET(zone_table[zone].flags, Z_LOCKED)) {
			send_to_char("Zone was not locked.\n",ch);
			return;
		} else {
			REMOVE_BIT(zone_table[zone].flags,Z_LOCKED);
			send_to_char("Zone Unlocked.\n",ch);
			return;
		}
	} else 
		send_to_char("Lock or Unlock which zone?\n",ch);
}

void do_invis (CHAR_DATA *ch, char *argument, int cmd)
{
	if ( !IS_SET (ch->flags, FLAG_WIZINVIS) ) {
		SET_BIT (ch->flags, FLAG_WIZINVIS);
		send_to_char ("You dematerialize.\n", ch);
	} else {
		send_to_char ("You are already invisible.\n", ch);
	}
}

void do_vis(CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	AFFECTED_TYPE	*af;

	if ( (af = get_affect (ch, MAGIC_AFFECT_INVISIBILITY)) )
		affect_remove (ch, af);

	if ( IS_SET (ch->flags, FLAG_WIZINVIS) ) {
		REMOVE_BIT (ch->flags, FLAG_WIZINVIS);
		remove_affect_type (ch, MAGIC_HIDDEN);

		strcpy (buf, "$n materializes before your eyes.");
		act (buf, FALSE, ch, 0, 0, TO_ROOM);

		send_to_char ("You materialize.\n", ch);
	}

	else if ( get_affect (ch, MAGIC_HIDDEN) ) {
		remove_affect_type (ch, MAGIC_HIDDEN);
		send_to_char ("You reveal yourself.\n", ch);
		act ("$n reveals $mself.", FALSE, ch, 0, 0, TO_ROOM);
	}

	else
		send_to_char ("You are already visible.\n", ch);
}


void read_race_data (CHAR_DATA *ch)
{
	FILE		*fp;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*p;
	int			vnum;
	char		race [MAX_STRING_LENGTH] = {'\0'};
	int			count = 0;
	CHAR_DATA	*mob;
	int			ind;

	if ( !(fp = fopen ("race.done", "r")) ) {
		perror ("lib/race.done");
		return;
	}
return;

/**** needs to be changed to new format before it can be used ***/
	while ( fgets (buf, 256, fp) ) {

		p = one_argument (buf, race);

		vnum = strtol(race, NULL,10);

		p = one_argument (p, race);

		ind = race_lookup (race);

		if ( ind == -1 )
			printf ("%05d  %15s; INVALID RACE NAME.", vnum, race);
		else
			printf ("%05d  %15s = %s",
					vnum, race,
					db_race_table [ind].name);

		if ( !(mob = vtom (vnum)) )
			printf ("INVALID VNUM!\n");
		else {
			if ( ind != -1 )
				mob->race = ind;
			printf ("\n");
		}

		count++;
	}

	fclose (fp);

	printf ("%d mobs processed.\n", count);
}

void debug_info (CHAR_DATA *ch, char *argument)
{
	CHAR_DATA	*target;
	CHAR_DATA	*other;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("debug info <target n/pc> [<other n/pc>]\n", ch);
		send_to_char ("   Perception is of target on other.\n", ch);
		return;
	}

	if ( !(target = get_char_room (buf, ch->in_room)) ) {
		send_to_char ("No such target.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf )
		other = ch;
	else if ( !(other = get_char_room (buf, ch->in_room)) ) {
		send_to_char ("No such other.\n", ch);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "is_brother (target, other):   %s\n",
				is_brother (target, other) ? "true" : "FALSE");
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "is_leader (target, other:     %s\n",
				is_leader (target, other) ? "true" : "FALSE");
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "is_area_enforcer (target):    %s\n",
				is_area_enforcer (target) ? "true" : "FALSE");
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "is_area_leader (target):      %s\n",
				is_area_leader (target) ? "true" : "FALSE");
	send_to_char (buf, ch);
}

void write_obj_values (CHAR_DATA *ch)
{
	int			i;
	int			obj_load_count;
	OBJ_DATA	*obj;
	OBJ_DATA	*obj2;
	FILE		*fp;

	if ( !(fp = fopen ("obj_values", "w")) ) {
		send_to_char ("Unable to open file lib/obj_values.\n", ch);
		return;
	}

		/* There are about 45 object types when this was written */

	for ( i = 0; i < 50; i++ ) {
		for ( obj = full_object_list; obj; obj = obj->lnext ) {
			if ( obj->obj_flags.type_flag != i )
				continue;

			obj_load_count = 0;

			for ( obj2 = object_list; obj2; obj2 = obj2->next )
				if ( obj2->virtual == obj->virtual )
					obj_load_count++;

			fprintf (fp, "%05d  %4d %d [%s:%d] %s\n",
						 obj->virtual,
						 (int)obj->silver,
						 (int)obj->coppers,
						 item_types [(int) obj->obj_flags.type_flag],
						 obj_load_count,
						 obj->short_description);
		}
	}

	fclose (fp);
}

void write_obj_silver_copper_values (CHAR_DATA *ch)
{
	OBJ_DATA	*obj;
	FILE		*fp;

	if ( !(fp = fopen ("obj_values_sf", "w")) ) {
		send_to_char ("Unable to open file lib/obj_values_sf.\n", ch);
		return;
	}

		/* There are about 45 object types when this was written */

	for ( obj = full_object_list; obj; obj = obj->lnext ) {

		if ( !obj->coppers )
			continue;

		fprintf (fp, "%05d  %4d %d\n",
					 obj->virtual,
					 (int)obj->silver,
					 (int)obj->coppers);
	}

	fclose (fp);
}

void read_obj_silver_copper_values (CHAR_DATA *ch)
{
	int			ns;
	int			virtual;
	int			silver;
	int			copper;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	FILE		*fp;
	OBJ_DATA	*obj;

	if ( !(fp = fopen ("obj_values_sf", "r")) ) {
		send_to_char ("Unable to open file lib/obj_values_sf.\n", ch);
		return;
	}

	while ( !feof (fp) ) {

		ns = fscanf (fp, "%d %d %d\n", &virtual, &silver, &copper);

		if ( ns != 3 ) {	
			send_to_char ("Scan of less than 3.\n", ch);
			return;
		}

		if ( !(obj = vtoo (virtual)) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Object %d missing.\n", virtual);
			send_to_char (buf, ch);
			continue;
		}

		if ( obj->silver != silver ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Object %d had silver %d->%d\n", virtual, (int)obj->silver, silver);
			send_to_char (buf, ch);
		}

		if (obj->coppers > 0) {
			snprintf (buf, MAX_STRING_LENGTH,  "Object %d coppers  %d->%d\n", virtual, (int)obj->coppers, copper);
			send_to_char (buf, ch);
		}

		if ( copper ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Object %d %d:%d ->  %d:%d\n", virtual, (int)obj->silver, (int)obj->coppers, silver, copper);
			send_to_char (buf, ch);
			obj->coppers = copper;
			redefine_objects (obj);
		}
	}
}


void update_obj_values (CHAR_DATA *ch)
{
	int			vnum;
	int			coppers;
	int			pennies;
	int			line = 0;
	int			unchanged = 0;
	int			changed = 0;
	int			zeroed = 0;
	int			instances = 0;
	OBJ_DATA	*obj;
	char		*argument;
	FILE		*fp;
	char		buf [MAX_STRING_LENGTH];
	char		token [MAX_STRING_LENGTH];

	if ( !(fp = fopen ("obj_values_updated", "r")) ) {
		send_to_char ("Unable to open file lib/obj_values_updated.\n", ch);
		return;
	}

	while ( fgets (buf, 256, fp) ) {

		line++;

		argument = one_argument (buf, token);

		if ( !just_a_number (token) ) {
			snprintf (token, MAX_STRING_LENGTH, "Error on line %d:  %s\n", line, buf);
			send_to_char (token, ch);
			return;
		}

		vnum = strtol(token, NULL,10);

		argument = one_argument (argument, token);

		if ( !just_a_number (token) ) {
			snprintf (token, MAX_STRING_LENGTH, "Error on line %d:  %s\n", line, buf);
			send_to_char (token, ch);
			return;
		}
		
		pennies = strtol(token, NULL,10);

		argument = one_argument (argument, token);

		if ( !just_a_number (token) ) {
			snprintf (token, MAX_STRING_LENGTH, "Error on line %d:  %s\n", line, buf);
			send_to_char (token, ch);
			return;
		}

		coppers = strtol(token, NULL,10);

		if ( !(obj = vtoo (vnum)) ) {
			snprintf (token, MAX_STRING_LENGTH, "Vnum %d no longer exists.\n", vnum);
			send_to_char (token, ch);
			continue;
		}

		if ( obj->coppers == coppers &&
			 obj->silver == pennies ) {
			unchanged++;
			continue;
		}

		if ( coppers == 0 && pennies == 0 ) {
			zeroed++;
			continue;
		}

		if ( vnum > 30 && vnum < 50 ) {
			snprintf (token, MAX_STRING_LENGTH, "Vnum %d cannot be changed.\n", vnum);
			send_to_char (token, ch);
			continue;
		}

		changed++;

		obj->coppers = coppers;
		obj->silver = pennies;

		instances += redefine_objects (obj);
	}
	
	fclose (fp);

	snprintf (buf, MAX_STRING_LENGTH,  "%d unchanged objects.\n", unchanged);
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "%d had zero values and not updated.\n", zeroed);
	send_to_char (buf, ch);

	snprintf (buf, MAX_STRING_LENGTH,  "%d prototypes changed, affecting %d instances.\n",
				  changed, instances);
	send_to_char (buf, ch);
}

void do_debug (CHAR_DATA *ch, char *argument, int cmd)
{
	int			i = 0;
	int			id1 = 0;
	int			id2 = 0;
	OBJ_DATA	*obj = NULL;
	CHAR_DATA	*tch1 = NULL;
	CHAR_DATA	*tch2 = NULL;
	CHAR_DATA	*mob = NULL;
	char		buf [MAX_INPUT_LENGTH] = {'\0'};
	CHAR_DATA	*temp_arg = NULL;

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "writecoppers") ) {
		write_obj_silver_copper_values (ch);
		return;
	}

	if ( !str_cmp (buf, "readcoppers") ) {
		read_obj_silver_copper_values (ch);
		return;
	}

	if ( !str_cmp (buf, "updatevalues") ) {
		update_obj_values (ch);
		return;
	}

	if ( !str_cmp (buf, "cdr") ) {
		mob = vtom (14071);
		if ( !mob ) {
			send_to_char ("Mob not defined:  14071\n", ch);
			return;
		}
		snprintf (buf, MAX_STRING_LENGTH,  "%d: %s\n", mob->mob->virtual, mob->clans);
		send_to_char (buf, ch);
		return;
	}

	if ( !str_cmp (buf, "value") ) {
		write_obj_values (ch);
		return;
	}

	if ( !str_cmp (buf, "quiet") ) {
		SET_BIT (ch->act, PLR_QUIET);
		return;
	}

	if ( !str_cmp (buf, "loud") ) {
		REMOVE_BIT (ch->act, PLR_QUIET);
		return;
	}

	if ( !str_cmp (buf, "info") ) {
		debug_info (ch, argument);
		return;
	}

	if ( !str_cmp (buf, "help") ) {
		write_help ("testhelp", help_list);
		return;
	}

	if ( !str_cmp (buf, "next") ) {
		tch1 = next_random (&id1);
		tch2 = next_random (&id2);
		if ( tch1 )
			printf ("Next:    %s  %s\n", tch1->tname, tch2->tname);
		else
			printf ("No more.\n");
		fflush (stdout);
		return;
	}

	if ( !str_cmp (buf, "races") ) {
		read_race_data (ch);
		return;
	}

	if ( !str_cmp (buf, "createobj") ) {
		for ( i = 0; i < 100; i++ ) {
			obj = load_object (5);
			obj_to_room (obj, 1);
			extract_obj (obj);
			cleanup_the_dead (2);
		}
	}

	if ( !str_cmp (buf, "createmob") ) {
		for ( i = 0; i < 10000; i++ ) {
			temp_arg = new_char (0);
			free_char (temp_arg);
		}
	}

	if ( !str_cmp (buf, "fighting") || !str_cmp (buf, "fight") ) {
		if ( ch->debug_mode & DEBUG_FIGHT )
			ch->debug_mode &= ~DEBUG_FIGHT;
		else
			ch->debug_mode |= DEBUG_FIGHT;
	}

	else if ( !str_cmp (buf, "misc") ) {
		if ( ch->debug_mode & DEBUG_MISC )
			ch->debug_mode &= ~DEBUG_MISC;
		else
			ch->debug_mode |= DEBUG_MISC;
	}

	else if ( !str_cmp (buf, "skills") ) {
		if ( ch->debug_mode & DEBUG_SKILLS )
			ch->debug_mode &= ~DEBUG_SKILLS;
		else
			ch->debug_mode |= DEBUG_SKILLS;
	}

	else if ( !str_cmp (buf, "summary") ) {
		if ( ch->debug_mode & DEBUG_SUMMARY )
			ch->debug_mode &= ~DEBUG_SUMMARY;
		else
			ch->debug_mode |= DEBUG_SUMMARY;
	}

	else if ( !str_cmp (buf, "subdue") ) {
		if ( ch->debug_mode & DEBUG_SUBDUE )
			ch->debug_mode &= ~DEBUG_SUBDUE;
		else
			ch->debug_mode |= DEBUG_SUBDUE;
	}

	else if ( !str_cmp (buf, "x1") ) {
		if ( x1 )
			x1 = 0;
		else
			x1 = 1;
	}

	else if ( !strcmp (buf, "crash") ) {
		printf ("Debug crash core dump:\n");
		fflush (stdout);
		((int *) 0) [-1] = 0;
	}

	else
		send_to_char ("The only modes available:\n"
					  "    fighting, skills, misc.\n", ch);

	send_to_char ("Flags set:  ", ch);

	if ( IS_SET (ch->debug_mode, DEBUG_FIGHT) )
		send_to_char ("FIGHT ", ch);

	if ( IS_SET (ch->debug_mode, DEBUG_MISC) )
		send_to_char ("MISC ", ch);

	if ( IS_SET (ch->debug_mode, DEBUG_SKILLS) )
		send_to_char ("SKILLS ", ch);

	if ( IS_SET (ch->debug_mode, DEBUG_SUMMARY) )
		send_to_char ("SUMMARY ", ch);

	if ( IS_SET (ch->debug_mode, DEBUG_SUBDUE) ) 
		send_to_char ("SUBDUE ", ch);

	if ( x1 )
		send_to_char ("X1 ", ch);

	send_to_char ("\n", ch);
}

void do_hour (CHAR_DATA *ch, char *argument, int cmd)
{
	next_hour_update = time (0);
	next_minute_update = time (0);

	times_do_hour_called++;
}

void do_day (CHAR_DATA *ch, char *argument, int cmd)
{
	int		i = 0, times = 0;

	if ( *argument && isdigit (*argument) )
		times = strtol(argument, NULL, 10);

	if ( !times )
		times = 1;

	for ( i = 1; i <= times; i++ ) {

		time_info.hour = 24;

		next_hour_update = time(0);
		next_minute_update = time(0);

		weather_and_time(1);
	}
}

void do_set (CHAR_DATA *ch, char *argument, int cmd)
{
	int			ind;
	char		buf [MAX_INPUT_LENGTH] = {'\0'};
	char		subcmd [MAX_INPUT_LENGTH] = {'\0'};

	argument = one_argument (argument, subcmd);

	if ( !str_cmp (subcmd, "hints") ) {
		if ( !IS_SET (ch->plr_flags, NEWBIE_HINTS) ) {
			SET_BIT (ch->plr_flags, NEWBIE_HINTS);
			send_to_char ("You will now periodically receive game- and syntax-related hints.\n", ch);
			return;
		}
		REMOVE_BIT (ch->plr_flags, NEWBIE_HINTS);
		send_to_char ("You will no longer receive game- and syntax-related hints.\n", ch);
		return;
	}

	if ( !str_cmp (subcmd, "combat") ) {
		if ( !IS_SET (ch->plr_flags, COMBAT_FILTER) ) {
			SET_BIT (ch->plr_flags, COMBAT_FILTER);
			send_to_char ("You will now only see local combat messages on your screen.\n", ch);
			return;
		}
		REMOVE_BIT (ch->plr_flags, COMBAT_FILTER);
		send_to_char ("You will now see all combat messages in your area.\n", ch);
		return;
	}

	if ( !str_cmp (subcmd, "voting") && ch->desc && ch->desc->account ) {
		if ( !IS_SET (ch->desc->account->flags, ACCOUNT_NOVOTE) ) {
			send_to_char ("You will no longer receive voting reminders upon entering the game.\n", ch);
			SET_BIT (ch->desc->account->flags, ACCOUNT_NOVOTE);	
			return;
		}
		REMOVE_BIT (ch->desc->account->flags, ACCOUNT_NOVOTE);
		send_to_char ("You will now be shown voting reminders upon entering the game.\n", ch);
		return;
	}

	if ( !str_cmp (subcmd, "rpp") && ch->desc && ch->desc->account ) {
		if ( IS_SET (ch->flags, FLAG_GUEST) ) {
			send_to_char ("This option is unavailable to guest logins.\n", ch);
			return;
		}
		if ( !IS_SET (ch->desc->account->flags, ACCOUNT_RPPDISPLAY) ) {
			send_to_char ("The output to SCORE will now show your number of roleplay points.\n", ch);
			SET_BIT (ch->desc->account->flags, ACCOUNT_RPPDISPLAY);	
			save_account (ch->desc->account);
			return;
		}
		REMOVE_BIT (ch->desc->account->flags, ACCOUNT_RPPDISPLAY);
		send_to_char ("Your roleplay points will no longer be visible in SCORE.\n", ch);
		save_account (ch->desc->account);
		return;
	}

	if ( !str_cmp (subcmd, "ansi") ) {
		if ( !IS_SET (ch->flags, FLAG_GUEST) ) {
			send_to_char ("This switch is only for guest users.\n", ch);
			return;
		}
		if ( ch->desc->color ) {
			ch->desc->color = 0;
			send_to_char ("ANSI color has been disabled.\n", ch);
			return;
		}
		ch->desc->color = 1;
		send_to_char ("#2ANSI color has been enabled.#0\n", ch);
		return;
	}

	if ( !str_cmp (subcmd, "mute") ) {
		if ( !IS_SET (ch->plr_flags, MUTE_BEEPS) ) {
			SET_BIT (ch->plr_flags, MUTE_BEEPS);
			send_to_char ("You will now no longer hear system beeps upon receiving notifies.\n", ch);
			return;
		}
		REMOVE_BIT (ch->plr_flags, MUTE_BEEPS);
		send_to_char ("You will now hear system beeps upon receiving notifies.\n", ch);
		return;
	}

	if ( !str_cmp (subcmd, "newbie") ) {
		if ( IS_SET (ch->plr_flags, NEW_PLAYER_TAG) ) {
			REMOVE_BIT (ch->plr_flags, NEW_PLAYER_TAG);
			send_to_char ("The #2(new player)#0 tag has been removed from your long description.\n", ch);
			return;
		}
		if ( !IS_MORTAL(ch) ) {
			SET_BIT (ch->plr_flags, NEW_PLAYER_TAG);
			send_to_char ("New player tag has been enabled.\n", ch);
			return;
		}
		send_to_char ("Shame on you - trying to pass yourself off as a clueless newbie?\n", ch);
		return;
	}

	if ( !str_cmp (subcmd, "mentor") ) {
		if ( IS_SET (ch->flags, FLAG_GUEST) ) {
			send_to_char ("Guests cannot toggle this flag.\n", ch);
			return;
		}
		if ( !IS_SET (ch->plr_flags, MENTOR) ) {
			act ("Your mentor flag has been enabled. Be SURE to read the policies listed in #6HELP MENTOR#0 - you WILL be held accountable to them. If you don't agree to them, please type SET MENTOR again now to remove the flag.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			SET_BIT (ch->plr_flags, MENTOR);
			return;
		}
		send_to_char ("Your mentor flag has been disabled.\n", ch);
		REMOVE_BIT (ch->plr_flags, MENTOR);
		return;
	}

	if ( !str_cmp (subcmd, "prompt") ) {
		if ( !GET_FLAG (ch, FLAG_NOPROMPT) ) {
			SET_BIT (ch->flags, FLAG_NOPROMPT);
			send_to_char ("Prompt disabled. Use the HEALTH command to check your character's welfare.\n", ch);
			return;
		}
		else REMOVE_BIT (ch->flags, FLAG_NOPROMPT);
		send_to_char ("Informative prompt enabled.\n", ch);
		return;
	}

	if ( !str_cmp (subcmd, "mana") ) {
		if ( !ch->max_mana ) {
			send_to_char ("You are not a spellcaster.\n", ch);
			return;
		}
		if ( GET_FLAG (ch, FLAG_HARNESS) ) {
			REMOVE_BIT (ch->flags, FLAG_HARNESS);
			send_to_char ("Done.\n", ch);
			return;
		}
		else SET_BIT (ch->flags, FLAG_HARNESS);
		send_to_char ("Done.\n", ch);
		return;
	}
	if ( !str_cmp (subcmd, "pacifist") ) {
		if ( GET_FLAG (ch, FLAG_PACIFIST ) ) {
			REMOVE_BIT (ch->flags, FLAG_PACIFIST);
			send_to_char ("You will now return attacks in combat.\n", ch);
		}
		else {
			SET_BIT (ch->flags, FLAG_PACIFIST);
			send_to_char ("You will now forego returning attacks in combat.\n", ch);
		}
		return;
	}
	if ( ch->pc && (ch->pc->level || IS_NPC(ch) || IS_SET(ch->flags, FLAG_ISADMIN)) && !str_cmp (subcmd, "wiznet") ) {
		if ( GET_FLAG (ch, FLAG_WIZNET) ) {
			REMOVE_BIT (ch->flags, FLAG_WIZNET);
			send_to_char ("You are no longer tuned into the wiznet.\n", ch);
			snprintf (buf, MAX_STRING_LENGTH,  "%s has left the wiznet channel.\n", GET_NAME(ch));
			send_to_gods (buf);
		}

		else {
			SET_BIT (ch->flags, FLAG_WIZNET);
			send_to_char ("You are now tuned into the wiznet.\n", ch);
			snprintf (buf, MAX_STRING_LENGTH,  "%s has rejoined the wiznet channel.\n", GET_NAME(ch));
			send_to_gods (buf);
		}
		return;
	}

	if ( ch->pc && ch->pc->level && !str_cmp (subcmd, "names") ) {
		if ( GET_FLAG (ch, FLAG_SEE_NAME) ) {
			REMOVE_BIT (ch->flags, FLAG_SEE_NAME);
			send_to_char ("'SAY' will no longer show player names.\n", ch);
		}

		else {
			SET_BIT (ch->flags, FLAG_SEE_NAME);
			send_to_char ("'SAY' will show player names.\n", ch);
		}

		return;
	}

	else if ( !IS_MORTAL (ch) && !str_cmp (subcmd, "telepath") ) {
		if ( GET_FLAG (ch, FLAG_TELEPATH) ) {
			REMOVE_BIT (ch->flags, FLAG_TELEPATH);
			send_to_char ("You will no longer receive PC thoughts.\n", ch);
		}
		else {
			send_to_char ("You are now attuned to PC thoughts -- may the gods help you.\n", ch);
			SET_BIT (ch->flags, FLAG_TELEPATH);
		}
	}

	else if ( !IS_MORTAL (ch) && !str_cmp (subcmd, "prompt") ) {
		if ( GET_FLAG (ch, FLAG_NOPROMPT) )
			REMOVE_BIT (ch->flags, FLAG_NOPROMPT);
		else {
			send_to_char ("Prompt removed.  Enter 'set prompt' to get it back.\n", ch);
			SET_BIT (ch->flags, FLAG_NOPROMPT);
		}
	}

	else if ( !IS_MORTAL (ch) && !str_cmp (subcmd, "immwalk") )
		ch->speed = SPEED_IMMORTAL;

	else if ( !IS_MORTAL (ch) && !str_cmp (subcmd, "available") ) {
		if ( IS_SET (ch->flags, FLAG_AVAILABLE) ) {
			REMOVE_BIT (ch->flags, FLAG_AVAILABLE);
			send_to_char ("You will no longer register as available for petitions.\n", ch);
		}

		else {
			SET_BIT (ch->flags, FLAG_AVAILABLE);
			send_to_char ("You will now be listed as available for petitions.\n", ch);
		}

		return;
	}

	else if ( !str_cmp (subcmd, "autoflee") ) {

		TOGGLE (ch->flags, FLAG_AUTOFLEE);

		if ( GET_FLAG (ch, FLAG_AUTOFLEE) )
			send_to_char ("You will flee at the first sign of trouble.\n", ch);
		else
			send_to_char ("You will stand and fight from now on.\n", ch);
	}

	else if ( !str_cmp (subcmd, "frantic") ) {
		ch->fight_mode = 0;
		send_to_char ("You start fighting frantically.\n", ch);
	}

	else if ( !str_cmp (subcmd, "aggressive") ) {
		ch->fight_mode = 1;
		send_to_char ("You start fighting more aggressively.\n", ch);
	}

	else if ( !str_cmp (subcmd, "normal") ) {
		ch->fight_mode = 2;
		send_to_char ("You start fighting normally.\n", ch);
	}

	else if ( !str_cmp (subcmd, "careful") ) {
		ch->fight_mode = 3;
		send_to_char ("You start fighting more carefully.\n", ch);
	}
	else if ( !str_cmp (subcmd, "defensive") ) {
		ch->fight_mode = 4;
		send_to_char ("You start fighting more defensively.\n", ch);
	}

	else if ( ch->pc && ch->pc->level && !str_cmp (subcmd, "mortal") ) {
		ch->pc->mortal_mode = 1;
		send_to_char ("Now playing in mortal mode.\n", ch);
	}

	else if ( ch->pc && ch->pc->level && !cmp_strn (subcmd, "immortal", strlen(subcmd)) ) {
		ch->pc->mortal_mode = 0;
		send_to_char ("Returning to immortal mode.\n", ch);
	}

	else if ( ch->desc && !str_cmp (subcmd, "lines") ) {

		argument = one_argument (argument, buf);

		if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 79 )
			send_to_char ("Expected a number 1..79", ch);
		else
			ch->desc->max_lines = strtol(buf, NULL, 10);
	}

	else if ( ch->desc && !str_cmp (subcmd, "columns") ) {

		argument = one_argument (argument, buf);

		if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 199 )
			send_to_char ("Expected a number 1..199", ch);
		else
			ch->desc->max_columns = strtol(buf, NULL, 10);
	}

	else if ( ch->pc && ch->pc->level && !str_cmp (subcmd, "immenter") ) {

		while ( *argument && *argument == ' ' )
			argument++;

		if ( ch->pc->imm_enter )
			mem_free (ch->pc->imm_enter);

		if ( !*argument ) {
			send_to_char ("You will use the standard immenter message.\n", ch);
			ch->pc->imm_enter = str_dup ("");
			return;
		}

		if ( *argument == '"' || *argument == '\'' )
			send_to_char ("Note:  You probably didn't mean to use quotes.\n",
						  ch);

		ch->pc->imm_enter = str_dup (argument);
	}

	else if ( ch->pc && ch->pc->level && !str_cmp (subcmd, "immleave") ) {

		while ( *argument && *argument == ' ' )
			argument++;

		if ( ch->pc->imm_leave )
			mem_free (ch->pc->imm_leave);

		if ( !*argument ) {
			send_to_char ("You will use the standard immleave message.\n", ch);
			ch->pc->imm_leave = str_dup ("");
			return;
		}

		if ( *argument == '"' || *argument == '\'' )
			send_to_char ("Note:  You probably didn't mean to use quotes.\n",
						  ch);

		ch->pc->imm_leave = str_dup (argument);
	}

	else if ( ch->pc &&
			  IS_RIDER (ch) &&
			  (ind = index_lookup (mount_speeds, subcmd)) != -1 ) {

		if ( ind == SPEED_SWIM ) {
			send_to_char ("Your mount will swim when in water.\n", ch);
			return;
		}

		if ( ind == SPEED_IMMORTAL && !get_trust (ch) ) {
			send_to_char ("Your mount would disintegrate if you tried that.", ch);
			return;
		}

		ch->pc->mount_speed = ind;
	}

	else if ( (ind = index_lookup (speeds, subcmd)) != -1 && *subcmd ) {

		if ( ind == SPEED_SWIM ) {
			send_to_char ("You'll swim when you're in water.\n", ch);
			return;
		}

		if ( ind == SPEED_IMMORTAL && !get_trust (ch) ) {
			send_to_char ("Your muscles would rip away from your bones if you tried that.\n", ch);
			return;
		}

		ch->speed = ind;
		snprintf (buf, MAX_STRING_LENGTH,  "From now on you will %s.\n",speeds[ind]);
		send_to_char (buf, ch);
		clear_travel(ch);

	}

	else {

#define s(a) send_to_char (a "\n", ch);

		s ("\n   #4Movement:#0");
		s ("   Walk speeds  - trudge, pace, walk, jog, run, sprint");
		s ("   Mount speeds - walk, canter, trot, gallop, run");
		s ("");
		s ("   #4Combat Modes:#0");
		s ("   Frantic    - Offense, but no defense (no combat learning)");
		s ("   Aggressive - More offense than defense");
		s ("   Normal     - Balanced offense/defense");
		s ("   Careful    - More defense then offense");
		s ("   Defensive  - Mostly defense, some offense");
		s ("");
		s ("   #4Combat Flags:#0");
		s ("   Autoflee   - Run away immediately if someone attacks");
		s ("   Pacifist   - Forego all attacks, for a defensive bonus");
		s ("");
		s ("   #4Informative:#0");
		s ("   Combat	  - Toggle filtering of non-local combat messages");
		s ("   Hints      - Toggle receipt of game-related hints");
		s ("   Mana       - Toggle display of mana status in prompt");
		s ("   Mentor     - Please see #6HELP MENTOR#0 for details");
		s ("   Mute       - Toggle system beeps upon being notified");
		s ("   Newbie     - Turn off your #2(new player)#0 ldesc tag");
		s ("   Prompt     - Toggle informative prompt on and off");
		s ("   Rpp        - Toggle output of RPP total in SCORE");
		s ("   Voting     - Toggle receipt of voting reminders at login"); 
		if ( !IS_MORTAL(ch) ) {
			s ("\n   #4Staff-Only Commands:#0");
			s ("   Available - Toggle your availability for petitions");
			s ("   Immenter <message>  - Set \"goto\" enter message");
			s ("   Immleave <message>  - Set \"goto\" leave message");
			s ("   Immortal  - Return to immortal mode");
			s ("   Immwalk   - Removes the walking delay when you move");
			s ("   Mortal    - Become a player (for testing)");
			s ("   Names     - Causes 'SAY' to show player names");
			s ("   Telepath  - Allows you to 'listen in' on PC thoughts");
			s ("   Wiznet    - Toggles your presence on the wiznet");
		}
	}

#undef s
}

void do_passwd (CHAR_DATA *ch, char *argument, int cmd)
{
	char		account_name [MAX_INPUT_LENGTH] = {'\0'};
	char		new_pass [MAX_INPUT_LENGTH] = {'\0'};
	ACCOUNT_DATA	*account = NULL;
	DESCRIPTOR_DATA	*d = NULL;
	char			*temp_arg = NULL;

	argument = one_argument (argument, account_name);

	if ( !*account_name ) {
		send_to_char ("Syntax:  passwd <account-name> <password>\n", ch);
		return;
	}

	argument = one_argument (argument, new_pass);

	if ( !*new_pass ) {
		send_to_char ("Syntax:  passwd <account-name> <password>\n", ch);
		return;
	}

	if ( strlen (new_pass) > 10 || strlen (new_pass) < 4 ) {
		send_to_char ("Make the password between 4 and 10 characters.\n", ch);
		return;
	}

	account = NULL;

	for ( d = descriptor_list; d; d = d->next ) {
		if ( !d->account )
			continue;
		if ( !str_cmp (d->account->name, account_name) )
			account = d->account;
	}

	if ( !account ) {
		account = load_account (account_name);
		if ( !account ) {
			send_to_char ("No such login account.\n", ch);
			return;
		}
	}
	
	temp_arg = encrypt_buf (new_pass);
	account->pwd = add_hash(temp_arg);

	save_account (account);

	send_to_char ("Password set.\n", ch);
	return;
}

void do_whap (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*victim;
	OBJ_DATA	*obj;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Whap who?\n", ch);
		return;
	}

	if ( !(victim = get_char_room_vis (ch, buf)) ) {
		send_to_char ("They aren't here.\n", ch);
		return;
	}

	if ( victim == ch ) {
		send_to_char ("You probably don't want to whap yourself.\n", ch);
		return;
	}

	if ( !(obj = get_equip (ch, WEAR_PRIM)) &&
		 !(obj = get_equip (ch, WEAR_SEC)) &&
		 !(obj = get_equip (ch, WEAR_BOTH)) ) {
		send_to_char ("You must be wielding a weapon to whap someone.\n", ch);
		return;
	}

	if ( !IS_NPC (victim) && !str_cmp (IMPLEMENTOR_ACCOUNT, victim->tname) ) {
		act ("The moment you slip behind $m, $e spins around, and in a "
			 "flash $e grabs $p from your hands and whaps you with it!",
			 FALSE, ch, obj, 0, TO_CHAR | TO_ACT_FORMAT);
		act ("You're out like a light!", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act ("HA! $N tries to whap you with $p.",
			FALSE, victim, obj, ch, TO_CHAR | TO_ACT_FORMAT);

		GET_POS (ch) = POSITION_STUNNED;

		act ("$N whaps $n on the head.  $n loses consciousness.",
			TRUE, victim, 0, ch, TO_NOTVICT | TO_ACT_FORMAT);

		ch->delay_type = DEL_WHAP;
		ch->delay = 10;

		return;
        }
         
        if ( !IS_NPC (victim) && !str_cmp ("Nix", victim->tname) ) {
                act ("The moment you slip behind Nix, she spins around, and in a "
                         "flash she grabs\n$p from your hands and whaps you with it!",
                         FALSE, ch, obj, 0, TO_CHAR);
                act ("You're out like a light!", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
                act ("HA!  $N tries to whap you with $p.",
                        FALSE, victim, obj, ch, TO_CHAR | TO_ACT_FORMAT);
         
                GET_POS (ch) = POSITION_STUNNED;
        
                act ("$N whaps $n on the head.  $n looses consciousness.",
                        TRUE, victim, 0, ch, TO_NOTVICT | TO_ACT_FORMAT);
        
                ch->delay_type = DEL_WHAP;
                ch->delay = 10;
                
                return;
	}

	if ( GET_TRUST (victim) > GET_TRUST (ch) ) {
		act ("$N glares as you raise $p.  You think better of it.",
				FALSE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);
		act ("You notice $N trying to whap you and glare at $m.",
				FALSE, victim, 0, ch, TO_CHAR | TO_ACT_FORMAT);
		return;
	}

	act ("You slip behind $N and whap $M with $p.",
			FALSE, ch, obj, victim, TO_CHAR | TO_ACT_FORMAT);

	act ("In a flash, $N slips behind you and forcefully taps\nyou on the "
		 "head with the blunt side of $p.",
			FALSE, victim, obj, ch, TO_CHAR | TO_ACT_FORMAT);

	act ("Just before your eyes close, you see $N standing over\n"
		 "you laughing.",
			FALSE, victim, 0, ch, TO_CHAR | TO_ACT_FORMAT);

	GET_POS (victim) = POSITION_STUNNED;

	act ("$N whaps $n on the head; $e promptly loses consciousness.",
			TRUE, victim, 0, ch, TO_NOTVICT | TO_ACT_FORMAT);

	victim->delay_type = DEL_WHAP;
	victim->delay = 25;
}

void delayed_whap (CHAR_DATA *ch)
{
	if ( GET_POS (ch) == POSITION_STUNNED ) {
		GET_POS (ch) = POSITION_RESTING;
		send_to_char ("You regain consciousness.  Your head aches, "
					  "but otherwise you're fine.\n", ch);
		act ("$n regains consciousness.", TRUE, ch, 0, 0, TO_ROOM);
	}

	ch->delay = 0;
}

void do_maylay (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*victim;
	OBJ_DATA	*obj;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Whap who?\n", ch);
		return;
	}

	if ( !(victim = get_char_room_vis (ch, buf)) ) {
		send_to_char ("They aren't here.\n", ch);
		return;
	}

	if ( victim == ch ) {
		send_to_char ("You probably don't want to whap yourself.\n", ch);
		return;
	}

	if ( !(obj = get_equip (ch, WEAR_PRIM)) &&
		 !(obj = get_equip (ch, WEAR_SEC)) &&
		 !(obj = get_equip (ch, WEAR_BOTH)) ) {
		send_to_char ("You must be wielding a weapon to whap someone.\n", ch);
		return;
	}

	if ( !IS_NPC (victim) && !str_cmp ("Nix", victim->tname) ) {
		act ("The moment you slip behind Nix, she spins around, and in a "
			 "flash she grabs\n$p from your hands and whaps you with it!",
			 FALSE, ch, obj, 0, TO_CHAR);
		act ("You're out like a light!", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		act ("HA!  $N tries to whap you with $p.",
			FALSE, victim, obj, ch, TO_CHAR | TO_ACT_FORMAT);

		GET_POS (ch) = POSITION_STUNNED;

		act ("$N whaps $n on the head.  $n looses consciousness.",
			TRUE, victim, 0, ch, TO_NOTVICT | TO_ACT_FORMAT);

		ch->delay_type = DEL_WHAP;
		ch->delay = 10;

		return;
	}

	if ( GET_TRUST (victim) > GET_TRUST (ch) ) {
		act ("$N glares as you raise $p.  You think better of it.",
				FALSE, ch, obj, victim, TO_CHAR);
		act ("You notice $N trying to whap you and glare at $m.",
				FALSE, victim, 0, ch, TO_CHAR);
		return;
	}

	act ("You slip behind $N and whap $m with your $p.",
			FALSE, ch, obj, victim, TO_CHAR);

	act ("In a flash, $N slips behind you and forcefully taps\nyou on the "
		 "head with the blunt side of $p.",
			FALSE, victim, obj, ch, TO_CHAR);

	act ("Just before your eyes close, you see $N standing over\n"
		 "you laughing.",
			FALSE, victim, 0, ch, TO_CHAR);

	GET_POS (victim) = POSITION_STUNNED;

	act ("$N whaps $n on the head.  $n looses consciousness.",
			TRUE, victim, 0, ch, TO_NOTVICT);

	victim->delay_type = DEL_WHAP;
	victim->delay = 10;
}

void delayed_maylay (CHAR_DATA *ch)
{
	if ( GET_POS (ch) == POSITION_STUNNED ) {
		GET_POS (ch) = POSITION_RESTING;
		send_to_char ("You regain consciousness.  Your head aches, "
					  "but otherwise you're fine.\n", ch);
		act ("$n regains consciousness.", TRUE, ch, 0, 0, TO_ROOM);
	}

	ch->delay = 0;
}

void do_wanted (CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf1 [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA		*tch;
	AFFECTED_TYPE	*af;
	int				criminals = 0;
	int				zone = -1;

	argument = one_argument (argument, buf);

	if ( *buf && isdigit (*buf) )
		zone = strtol(buf, NULL, 10);

	*s_buf = '\0';

	if ( IS_MORTAL (ch) && !is_area_enforcer (ch) ) {
		send_to_char ("You don't know who is wanted around here.\n", ch);
		return;
	}

	if ( IS_MORTAL (ch) )
		zone = ch->room->zone;

	for ( tch = character_list; tch; tch = tch->next ) {

		if ( tch->deleted )
			continue;

		for ( af = tch->hour_affects; af; af = af->next ) {

			if ( af->type < MAGIC_CRIM_BASE ||
				 af->type > MAGIC_CRIM_BASE + 100 )
				continue;

			/*
			   If ch is not an imm, and the criminal is in the
			   same zone, then if the criminal is not wanted in
			   the zone, skip him.
			*/
			if (zone != -1) {
			      if(!is_same_zone(tch->room->zone, zone)) { 
				    continue;
			     } 
			}

			criminals++;

			if ( IS_MORTAL (ch) ) {
				if ( af->a.spell.duration == -1 )
					snprintf (buf, MAX_STRING_LENGTH,  "Forever %s\n", tch->short_descr);
				else
					snprintf (buf, MAX_STRING_LENGTH,  "  %d    %s\n", af->a.spell.duration, tch->short_descr);
			}

			else {

				if ( IS_NPC (tch) )
					snprintf (buf1, MAX_STRING_LENGTH,  "[%d] ", tch->mob->virtual);
				else
					snprintf (buf1, MAX_STRING_LENGTH,  "[%s] ", GET_NAME (tch));

				snprintf (buf, MAX_STRING_LENGTH,  "%5d  %2d      %2d  %s%s\n",
						 tch->in_room, af->a.spell.duration, af->type - MAGIC_CRIM_BASE,
						 buf1, tch->short_descr);
			}

			strcat (s_buf, buf);
		}
	}

	if ( !criminals )
		send_to_char ("No criminals.\n", ch);
	else {
		if ( !IS_MORTAL (ch) )
			ch->desc->header = str_dup ("Room  Hours  Zone  Name\n");
		else if ( criminals )
			ch->desc->header = str_dup ("Hours   Description\n");

		page_string (ch->desc, s_buf);
	}
}

void report_exits (CHAR_DATA *ch)
{
	int				dir;
	ROOM_DATA		*room;
	ROOM_DATA		*troom;

	*b_buf = '\0';

	for ( room = full_room_list; room; room = room->lnext ) {

		for ( dir = 0; dir < 6; dir++ ) {

			if ( !room->dir_option [dir] )
				continue;

			if ( !(troom = vtor (room->dir_option [dir]->to_room)) ) {
				snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
						 "Room %5d %5s doesn't go to room %d",
						 room->virtual, dirs [dir],
						 room->dir_option [dir]->to_room);
			}

			else if ( !troom->dir_option [rev_dir [dir]] ) {
				snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
						 "Room %5d %5s is one-way to room %d\n",
						 room->virtual, dirs [dir],
						 room->dir_option [dir]->to_room);
			}

			else if ( room->virtual !=
					  troom->dir_option [rev_dir [dir]]->to_room ) {
				snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
						 "Room %5d %5s->%5d  BUT  %5d <- %5s %5d\n",
						 room->virtual, dirs [dir], troom->virtual,
						 troom->dir_option [rev_dir [dir]]->to_room,
						 dirs [rev_dir [dir]], troom->virtual);
			}
		}
	}

	page_string (ch->desc, b_buf);
}

void report_mobiles (CHAR_DATA *ch)
{
	int				i;
	int				zone_counts [100];
	int				total_mobs = 0;
	int				recount = 0;
	CHAR_DATA		*tch;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	for ( i = 0; i < 100; i++ )
		zone_counts [i] = 0;

	for ( tch = character_list; tch; tch = tch->next ) {

		if ( tch->deleted || !IS_NPC (tch) )
			continue;

		zone_counts [tch->mob->reset_zone]++;
		total_mobs++;
	}

	for ( i = 0; i < 25; i++ ) {
		snprintf (buf, MAX_STRING_LENGTH,  "[%2d]: %3d     [%2d]: %3d      [%2d]: %3d      [%2d]: %3d\n",
					  i, zone_counts [i],
					  i + 25, zone_counts [i + 25],
					  i + 50, zone_counts [i + 50],
					  i + 75, zone_counts [i + 75]);
		send_to_char (buf, ch);

		recount += zone_counts [i] + zone_counts [i+25] +
				   zone_counts [i+50] + zone_counts [i+75];
	}

	snprintf (buf, MAX_STRING_LENGTH,  "  Total: %d; recount: %d\n", total_mobs, recount);
	send_to_char (buf, ch);
	snprintf (buf, MAX_STRING_LENGTH,  "  Hash dups: %d; Dup'ed len: %d\n",
				  hash_dup_strings, hash_dup_length);
	send_to_char (buf, ch);
}

void report_objects (CHAR_DATA *ch)
{
	int			i = 0;
	int			j;
	FILE		*fp;
	OBJ_DATA	*obj;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};

	if ( !(fp = fopen ("objects", "w+")) )
		perror ("Couldn't open objects");

	fprintf (fp, "Virtual\tQuality\tSize\tCondition\tWeight\tCost\tItemType\tOV1\tOV2\tOV3\tOV4\tOV5\tFlags\tKeywords\tShort\tOneLine\n");

	for ( obj = full_object_list; obj; obj = obj->lnext ) {
		*buf2 = '\0';

		for ( j = 0; str_cmp (econ_flags [j], "\n"); j++ ) {
			if ( IS_SET (obj->econ_flags, 1 << j) ) {
				strcat (buf2, econ_flags [j]);
				strcat (buf2, " ");
			}
		}

          /*          virt qual sz   cnd   wt  cost  it  v0  v1  v2  v3  v4  v5  fl  ky  sd  ld */

		fprintf (fp, "%05d\t%2d\t%5d\t%2d\t%5d\t%5d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\t%s\t%s\n",
			obj->virtual, obj->quality, obj->size, obj->item_wear,
			obj->obj_flags.weight, obj->obj_flags.cost,
			item_types [(int) obj->obj_flags.type_flag],
			obj->o.od.value [0], obj->o.od.value [1], obj->o.od.value [2],
			obj->o.od.value [3], obj->o.od.value [4], obj->o.od.value [5],
			buf2,
			obj->name, obj->short_description, obj->description);

		i++;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "Write %d records.\n", i);

	send_to_char (buf, ch);

	fclose (fp);
}

void revise_objects (CHAR_DATA *ch)
{
	int			ind;
	int			vnum;
	FILE		*fp;
	OBJ_DATA	*obj;
	char		*p;
	char		*argument;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};

	if ( !(fp = fopen ("objects.revised", "r")) )
		perror ("Couldn't open objects");

	while ( fgets (buf, 256, fp) ) {

		p = buf;
		while ( *p ) {
			if ( *p == ',' )
				*p = ' ';
			p++;
		}

		argument = one_argument (buf, buf2);

		vnum = strtol(buf2, NULL, 10);

		if ( !vnum || !(obj = vtoo (vnum)) ) {
			printf ("Vnum %d doesn't work.\n", vnum);
			continue;
		}

		argument = one_argument (argument, buf2);
		argument = buf2;

		obj->econ_flags = 0;

		while ( 1 ) {
			argument = one_argument (argument, buf);

			if ( !*buf )
				break;

			if ( (ind = index_lookup (econ_flags, buf)) == -1 ) {
				printf ("Unknown flag name: %s\n", buf);
				continue;
			}

			SET_BIT (obj->econ_flags, 1 << ind);
		}
	}

	fclose (fp);
	send_to_char ("Done.\n", ch);
}

void report_races (CHAR_DATA *ch)
{
	int			i = 0;
	FILE		*fp;
	CHAR_DATA	*mob;
	char		buf [MAX_STRING_LENGTH];

	if ( !(fp = fopen ("races", "w+")) )
		perror ("Couldn't create races");

	for ( mob = full_mobile_list; mob; mob = mob->mob->lnext ) {
		fprintf (fp, "%05d  %-7.7s %-60.60s\n",
					mob->mob->virtual,
					db_race_table [mob->race].name,
					GET_NAMES (mob));
		i++;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "Wrote %d records.\n", i);
	send_to_char (buf, ch);

	fclose (fp);
}

void report_shops (CHAR_DATA *ch)
{
	int			write_count = 0;
	int			i;
	FILE		*fp;
	CHAR_DATA	*mob;
	OBJ_DATA	*tobj;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

/*
I have a request for you...  merchants listing, tab delimited, with:
vnum, short desc, store room, markup, discount, trades in, deliveries 
0-9, and if possible, a listing of what is in their storerooms.

I'd like to give you back vnum, all the markups, discounts, and nobuy
*/

	if ( !(fp = fopen ("shops", "w+")) )
		perror ("Couldn't create shops");

	fprintf (fp, "Vnum \tShop \tStore\tMrkup\tDscnt\tDel-0\tName0\tDel-1\tName1\tDel-2\tName2\tDel-3\tName3\tDel-4\tName4\tDel-5\tName5\tDel-6\tName6\tDel-7\tName7\tDel-8\tName8\tDel-9\tName9\tKeeper Name\ttrade-flags\n");

	for ( mob = full_mobile_list; mob; mob = mob->mob->lnext ) {

		if ( !mob->shop )
			continue;

		snprintf (buf, MAX_STRING_LENGTH,  "%05d\t%5d\t%5d\t%3.2f\t%3.2f\t",
				      mob->mob->virtual, mob->shop->shop_vnum,
					  mob->shop->store_vnum, mob->shop->markup,
					  mob->shop->discount);

		for ( i = 0; i < MAX_DELIVERIES; i++ ) {

			snprintf (ADDBUF, MAX_STRING_LENGTH, "%5d\t", mob->shop->delivery [i]);

			if ( mob->shop->delivery [i] > 0 &&
				 (tobj = vtoo (mob->shop->delivery [i])) )
				snprintf (ADDBUF, MAX_STRING_LENGTH, "%s\t", tobj->short_description);
			else
				snprintf (ADDBUF, MAX_STRING_LENGTH, "\t");
		}

		snprintf (ADDBUF, MAX_STRING_LENGTH, "%s\t", mob->short_descr);

		for ( i = 0; i < MAX_TRADES_IN; i++ )
			if ( mob->shop->trades_in [i] )
				snprintf (ADDBUF, MAX_STRING_LENGTH, "%s ",
						 item_types [mob->shop->trades_in [i]]);

		fprintf (fp, "\t%s\n", buf);

		write_count++;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "Wrote %d records.\n", write_count);
	send_to_char (buf, ch);

	fclose (fp);
}

void get_float (float *f, char **argument)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	*argument = one_argument (*argument, buf);

	sscanf (buf, "%f", f);
}

void get_flags (int *flags, char **argument)
{
	int			ind;
	char		*p;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};

	*argument = one_argument (*argument, buf);

	*flags = 0;

	p = buf;

	while ( 1 ) {
		p = one_argument (p, buf2);

		if ( !*buf2 )
			break;

		if ( (ind = index_lookup (econ_flags, buf2)) == -1 ) {
			printf ("Unknown flag name: %s\n", buf2);
			continue;
		}

		SET_BIT (*flags, 1 << ind);
	}
}

void revise_shops (CHAR_DATA *ch)
{
	int			vnum;
	FILE		*fp;
	CHAR_DATA	*mob;
	char		*p;
	char		*argument;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};

	if ( !(fp = fopen ("shops.revised", "r")) )
		perror ("Couldn't open shops.revised");

	while ( fgets (buf, 256, fp) ) {

		p = buf;
		while ( *p ) {
			if ( *p == ',' )
				*p = ' ';
			p++;
		}

/*
"Vnum ","Shop ","Store","Mrkup","Dscnt","Mrkup1","Dscnt1","Flags1","Mrkup2","Dscnt2","Flags2","Mrkup3","Dscnt3","Flags3","Nobuy"
1173,14173,2951,1.15,0.15,2.30,0.30,"fine khuzan rare valuable magical Swedish noble foreign",1.44,0.19,"ntharda stharda orbaal",0.58,0.07,"poor","junk gargun"
*/
		argument = one_argument (buf, buf2);			/* Keeper vnum */
		vnum = strtol(buf2, NULL, 10);

		if ( !vnum || !(mob = vtom (vnum)) ) {
			printf ("Vnum %d doesn't work.\n", vnum);
			continue;
		}

		argument = one_argument (argument, buf2);		/* Shop */
		argument = one_argument (argument, buf2);		/* Store */

		get_float (&mob->shop->markup, &argument);
		get_float (&mob->shop->discount, &argument);
		get_float (&mob->shop->econ_markup1, &argument);
		get_float (&mob->shop->econ_discount1, &argument);
		get_flags (&mob->shop->econ_flags1, &argument);
		get_float (&mob->shop->econ_markup2, &argument);
		get_float (&mob->shop->econ_discount2, &argument);
		get_flags (&mob->shop->econ_flags2, &argument);
		get_float (&mob->shop->econ_markup3, &argument);
		get_float (&mob->shop->econ_discount3, &argument);
		get_flags (&mob->shop->econ_flags3, &argument);
		get_float (&mob->shop->econ_markup4, &argument);
		get_float (&mob->shop->econ_discount4, &argument);
		get_flags (&mob->shop->econ_flags4, &argument);
		get_float (&mob->shop->econ_markup5, &argument);
		get_float (&mob->shop->econ_discount5, &argument);
		get_flags (&mob->shop->econ_flags5, &argument);
		get_float (&mob->shop->econ_markup6, &argument);
		get_float (&mob->shop->econ_discount6, &argument);
		get_flags (&mob->shop->econ_flags6, &argument);
		get_float (&mob->shop->econ_markup7, &argument);
		get_float (&mob->shop->econ_discount7, &argument);
		get_flags (&mob->shop->econ_flags7, &argument);
		get_flags (&mob->shop->nobuy_flags, &argument);
	}

	fclose (fp);
	send_to_char ("Done.\n", ch);
}

#define ZCMD zone_table[zone].cmd[cmd_no]

void unused_objects (CHAR_DATA *ch)
{
/*
    int             cmd_no;
    OBJ_DATA        *obj = NULL;
    ROOM_DATA       *room;

	for ( obj = full_object_list; obj; obj = obj->lnext ) {
		for ( cmd_no = 0; ; cmd_no++ ) {

			if ( (ZCMD.command == 'O' ||
				  ZCMD.command == 'P' ||
				  ZCMD.command == 'G' ||
				  ZCMD.command == 'E') &&
				 ZCMD.arg1 == obj->virtual )
				

		for ( 
	}
*/
}

void do_report (CHAR_DATA *ch, char *argument, int cmd)
{
	char	buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {
		send_to_char ("   objects   - object list to file 'objects'.\n", ch);
		send_to_char ("   shops     - keeper list to file 'shops'.\n", ch);
		send_to_char ("   race      - race report.\n", ch);
		send_to_char ("   mobiles   - I don't know.\n", ch);
		send_to_char ("   exits     - Don't remember.\n", ch);
	}

	else if ( !str_cmp (buf, "uobjs") )
		unused_objects (ch);
	else if ( !str_cmp (buf, "crash") )
		((int *)0)[-1] = 0;
	else if ( !str_cmp (buf, "revshops") )
		revise_shops (ch);
	else if ( !str_cmp (buf, "revobjs") )
		revise_objects (ch);
	else if ( !str_cmp (buf, "shops") || !str_cmp (buf, "keepers") )
		report_shops (ch);
	else if ( !str_cmp (buf, "objs") || !str_cmp (buf, "objects") )
		report_objects (ch);
	else if ( !str_cmp (buf, "exits") )
		report_exits (ch);
	else if ( !str_cmp (buf, "mobiles") || !str_cmp (buf, "mobs") )
		report_mobiles (ch);
	else if ( !str_cmp (buf, "race") || !str_cmp (buf, "races") )
		report_races (ch);
	else
		send_to_char ("report ?   - for help\n", ch);
}

void do_openskill (CHAR_DATA *ch, char *argument, int cmd)
{
	int			sn;
	CHAR_DATA	*victim;
	char		buf1 [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf1);
	argument = one_argument (argument, buf2);

	if ( (sn = skill_index_lookup (buf1)) == -1 ) {
		if ( (sn = skill_index_lookup (buf2)) == -1 ) {
			send_to_char ("No such skill.  Use the 'show v skills' command.\n", ch);
			return;
		}

		victim = get_char_room_vis (ch, buf1);
	} else
		victim = get_char_room_vis (ch, buf2);

	if ( !victim ) {
		send_to_char ("Either use 'at <person> openskill' or go to his/her room.\n", ch);
		return;
	}

	if ( victim->skills [sn] ) {
		snprintf (buf1, MAX_STRING_LENGTH,  "Skill %s is already at %d on that character.\n",
					   skill_data[sn].skill_name, victim->skills [sn]);
		return;
	}

	open_skill (victim, sn);

	snprintf (buf1, MAX_STRING_LENGTH,  "$N's %s has opened at %d.",
				   skill_data[sn].skill_name, victim->skills [sn]);

	act (buf1, FALSE, ch, 0, victim, TO_CHAR);
}

void do_swap (CHAR_DATA *ch, char *argument, int cmd)
{
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			portchar [4];

	argument = one_argument (argument, buf);

	if ( str_cmp (buf, "worldfile") && str_cmp (buf, "binary") ) {
		send_to_char ("You must type 'swap worldfile' or 'swap binary'.\n", ch);
		return;
	}

	if ( pending_reboot ) {
		send_to_char ("You should cancel the pending reboot before swapping.\n", ch);
		return;
	}

	if ( port == PLAYER_PORT )
		snprintf (portchar, MAX_STRING_LENGTH, "pp");
	else if ( port == TEST_PORT )
		snprintf (portchar, MAX_STRING_LENGTH, "tp");
	else
		snprintf (portchar, MAX_STRING_LENGTH, "bp");

	
        if ( !str_cmp (buf, "worldfile") && !*argument ) {
                if ( port == BUILDER_PORT ) {
                        send_to_char ("On the builder port?\n", ch);
                        return;
                }
	
                if ( GET_TRUST (ch) < 4 ) {
                        send_to_char ("You must specify the zone number to swap.\n", ch);
                        return;
                }
		
                send_to_char ("Transferring ALL builders' changes...\n", ch);
                snprintf (buf, MAX_STRING_LENGTH,  "cp -R " PATH_TO_BP "/regions "PATH_TO_TOPDIR"/%s", portchar);
                system (buf);
                snprintf (buf, MAX_STRING_LENGTH,  "%s has just transferred ALL building changes.\n", ch->tname);
                send_to_gods (buf);
        }
        else if ( !str_cmp (buf, "worldfile") && isdigit(*argument) ) {
                if ( strtol(argument, NULL, 10) == 99 ) {
                        send_to_char ("That's the temproom zone; no swapping allowed.\n", ch);
                        return;
                }
                if ( str_cmp (ch->tname, zone_table[strtol(argument, NULL, 10)].lead) && GET_TRUST(ch) < 4 ) {
                        send_to_char ("You are not the lead for this building project.\n", ch);
                        return;
                }
                if ( strtol(argument, NULL, 10) >= 0 && strtol(argument, NULL, 10) <= 98 ) {
                        snprintf (buf, MAX_STRING_LENGTH,  "Transferring builders' changes to %s...\n",
                                zone_table[strtol(argument, NULL, 10)].name);
                        send_to_char (buf, ch);
                        snprintf (buf, MAX_STRING_LENGTH,  "cp -R " PATH_TO_BP "/regions/*.%d " PATH_TO_TOPDIR "/%s/regions/", (int)strtol(argument, NULL, 10), portchar);
                        system (buf);
                        snprintf (buf, MAX_STRING_LENGTH,  "%s has just swapped over the changes to %s.\n",
                                ch->tname, zone_table[strtol(argument, NULL, 10)].name);
                        send_to_gods (buf);
                }
                else {
                        send_to_char ("Please specify a number between 0 and 99.\n", ch);
                        return;
                }
        }
	else if ( !str_cmp (buf, "binary") ) {
		if ( str_cmp(ch->tname, IMPLEMENTOR_ACCOUNT)) {
			send_to_char ("Please ask the implementor to do it.\n", ch);
			return;
		}
		if ( port == PLAYER_PORT ) {
			system ("cp " PATH_TO_TP "/bin/server " PATH_TO_PP "/bin/dmserver");
			system ("mv " PATH_TO_TP "/bin/dmserver " PATH_TO_PP "/bin/server");
		}
		else if ( port == BUILDER_PORT ) {
			system ("cp " PATH_TO_TP "/bin/server " PATH_TO_BP "/bin/dmserver");
			system ("mv " PATH_TO_BP "/bin/dmserver " PATH_TO_BP "/bin/server");
		}
		send_to_char ("The server binary on this port has been updated.\n", ch);
	}
}


void do_affect (CHAR_DATA *ch, char *argument, int cmd)
{
	int				power_specified = 0;
	int				duration = 0;
	int				power = 0;
	int				affect_no = 0;
	AFFECTED_TYPE	*af  = NULL;
	CHAR_DATA		*tch = NULL;
	OBJ_DATA		*obj = NULL;
	ROOM_DATA		*room = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf || *buf == '?' ) {

		send_to_char (
			"           char <mob> <affect no/name>   delete\n"
			"   affect  obj  <obj> <affect no/name>   <duration> [power]\n"
			"           room       <affect no/name>\n"
		    "\n"
			"Example:\n\n"
			"   affect room 'aklash odor' delete\n"
            "   affect char 2311 5000 1000   (2311 = Incense Smoke)\n\n"
            "If an affect doesn't already exist, one is created by this "
			"command.\n\n"
            "   affect char 2311 4000        (Only modify duration)\n\n"
            "Most affect durations are in mud hours, some are in RL seconds.\n",
			ch);

		return;
	}

	room = ch->room;

	if ( is_abbrev (buf, "character") ) {

		argument = one_argument (argument, buf);

		if ( !(tch = get_char_room_vis (ch, buf)) ) {
			send_to_char ("Couldn't find that character in the room.\n", ch);
			return;
		}
	}

	else if ( is_abbrev (buf, "object") ) {

		argument = one_argument (argument, buf);

		if ( !(obj = get_obj_in_dark (ch, buf, ch->room->contents)) ) {
			send_to_char ("Couldn't find that object in the room.\n", ch);
			return;
		}
	}

	else if ( is_abbrev (buf, "room") ) {
	}

	else {
		send_to_char ("Expected 'character', 'object', or 'room' after affect.",
					  ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( just_a_number (buf) )
		affect_no = strtol(buf, NULL, 10);
	

	argument = one_argument (argument, buf);

	if ( is_abbrev (buf, "delete") ) {

		if ( obj ) {
			af = get_obj_affect (obj, affect_no);
			if ( !af ) {
				send_to_char ("No such affect on that object.\n", ch);
				return;
			}

			remove_obj_affect (obj, affect_no);
			send_to_char ("Affected deleted from object.\n", ch);
			return;
		}

		if ( tch ) {
			af = get_affect (ch, affect_no);
			if ( !af ) {
				send_to_char ("No such affect on that character.\n", ch);
				return;
			}

			affect_remove (ch, af);
			send_to_char ("Affect deleted from character.\n", ch);
			return;
		}

			/* It must be a room affect */

		if ( !(af = is_room_affected (room->affects, affect_no)) ) {
			send_to_char ("No such affect on this room.\n", ch);
			return;
		}

		remove_room_affect (room, affect_no);
		send_to_char ("Affect deleted from room.\n", ch);
		return;
	}

		/* We're not deleting, get the duration and optional power so we
           can either update an affect or create a new one.
        */

	if ( !just_a_number (buf) ) {
		send_to_char ("Duration expected.\n", ch);
		return;
	}

	duration = strtol(buf, NULL, 10);

	argument = one_argument (argument, buf);

	if ( *buf ) {
		if ( !just_a_number (buf) ) {
			send_to_char ("Power should be a number.\n", ch);
			return;
		}

		power_specified = 1;
		power = strtol(buf, NULL, 10);
	}

			/* Affect already exist? */

	if ( ch )
		af = get_affect (tch, affect_no);
	else if ( obj )
		af = get_obj_affect (obj, affect_no);
	else
		af = is_room_affected (room->affects, affect_no);

	if ( af ) {

		af->a.spell.duration = duration;

		if ( power_specified )
			af->a.spell.modifier = power;

		send_to_char ("Affect modified.\n", ch);
		return;
	}

		/* Add a new affect */

	af = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);

	af->type = affect_no;
	
	if ( affect_no >= MAGIC_FIRST_SOMA && affect_no <= MAGIC_LAST_SOMA ) {
		af->a.spell.duration = 4 * duration;
		af->a.soma.latency = 0;
		af->a.soma.minute = 0;
		af->a.soma.max_power = 1000;
		af->a.soma.lvl_power = 700;
		af->a.soma.atm_power = 0;
		af->a.soma.attack = duration * 15;
		af->a.soma.decay = duration * 30;
		af->a.soma.sustain = duration * 45;
		af->a.soma.release = duration * 60;
		if (tch) {
			af->next = tch->hour_affects;
			tch->hour_affects = af;
		}
		else {
			send_to_char("Somatic effects may only be applied to characters.\n", ch );
		}
	}
	else {
		af->a.spell.duration = duration;

		if ( power_specified ) {
			af->a.spell.modifier = power;
		}	
		if ( tch )
			affect_to_char (tch, af);
		else if ( obj )
			affect_to_obj (obj, af);
		else {
			af->next = room->affects;
			room->affects = af;
		}
	}
	send_to_char ("Affect created.\n", ch);
}

void list_all_crafts (CHAR_DATA *ch)
{
	SUBCRAFT_HEAD_DATA		*craft;
	char					*p;

	*b_buf = '\0';
	p = b_buf;

	snprintf (p, MAX_STRING_LENGTH,  "We currently have the following crafts available:\n\n");
	p += strlen (p);

	for ( craft = crafts; craft; craft = craft->next ) {
		snprintf (p, MAX_STRING_LENGTH,  "#6Craft:#0 %-12s #6Subcraft:#0 %-20s #6Command:#0 %-10s\n",
			craft->craft_name, craft->subcraft_name, craft->command);
		p += strlen (p);
	}

	send_to_char ("\n", ch);
	page_string (ch->desc, b_buf);
}

void display_craft (CHAR_DATA *ch, SUBCRAFT_HEAD_DATA *craft)
{
	PHASE_DATA		*phase = NULL;
	DEFAULT_ITEM_DATA	*items = NULL;
	int				i = 0;
	int				j = 0;
	int				phasenum = 1;
	char			flag [MAX_STRING_LENGTH] = {'\0'};
	char			*temp_arg = NULL;
	CHAR_DATA		*temp_char = NULL;

        for ( phase = craft->phases; phase; phase = phase->next, phasenum++ ) {

        	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  #6Phase %d:#0\n", phasenum);
                
                if ( phase->failure )
                	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "1stFail:  %s\n", phase->failure);

                if ( phase->second_failure )
                	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "2ndFail:  %s\n", phase->second_failure);
        
                if ( phase->third_failure )
                	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "3rdFail:  %s\n", phase->third_failure);
                
                if ( phase->first )
                	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "    1st:  %s\n", phase->first);
                        
                if ( phase->second )
                	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "    2nd:  %s\n", phase->second);
                        
                if ( phase->third )
                	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "    3rd:  %s\n", phase->third);
                        
                if ( phase->phase_seconds )  
                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "      T:  %d\n", phase->phase_seconds);
                         
                if ( phase->skill ){
                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  Skill:  %s vs %dd%d\n", skill_data[phase->skill].skill_name, phase->dice, phase->sides);
				}
                else if ( phase->attribute > 0){
                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "  Attri:  %s vs %dd%d\n", attrs[phase->attribute], phase->dice, phase->sides);
                }

                if ( phase->flags ) {
               		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "      F: ");
                        if ( IS_SET (phase->flags, 1 << 0) )
                        	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, " cannot-leave-room");
                        if ( IS_SET (phase->flags, 1 << 1) )
                        	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, " open_on_self");
                        if ( IS_SET (phase->flags, 1 << 2) )
                           	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, " require_on_self");
                        if ( IS_SET (phase->flags, 1 << 3) )
                        	snprintf(b_buf + strlen(b_buf), MAX_STRING_LENGTH, " require_greater");
                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "\n");
               	}
                                                        
               	if ( phase->move_cost )
                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "   Cost:  moves %d\n", phase->move_cost);
                                                 
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
                                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "      %d:  (%s", i, flag);
                                        if ( items->item_counts > 1 )
                                                snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, " x%d", items->item_counts);
                                        for ( j = 0; j <= MAX_DEFAULT_ITEMS; j++ ) {
                                        	if ( items->items[j] && items->items[j] != items->item_counts ) {
                                                	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, " %d", items->items[j]);
                                                }
                                        }
                                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, ")\n");
                                }
                       	}
		}

		if ( phase->load_mob && vtom(phase->load_mob) ){
			temp_char = vtom(phase->load_mob);
			temp_arg = char_short(temp_char);
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "    Mob:  #5%s#0\n", temp_arg);
		}
	}

	page_string (ch->desc, b_buf);

}

void alias_free (ALIAS_DATA *alias)
{
	ALIAS_DATA		*tmp_alias;

	while ( alias ) {
		tmp_alias = alias->next_line;

		if ( alias->command )
			mem_free (alias->command);

		if ( alias->line )
			mem_free (alias->line);

		mem_free (alias);

		alias = tmp_alias;
	}
}

void alias_delete (CHAR_DATA *ch, char *alias_cmd)
{
	ALIAS_DATA		*alias;
	ALIAS_DATA		*tmp_alias;

	if ( !ch->pc->aliases )
		return;

	if ( !str_cmp (alias_cmd, ch->pc->aliases->command) ) {
		alias = ch->pc->aliases;
		ch->pc->aliases = alias->next_alias;
		alias_free (alias);
		return;
	}

	for ( alias = ch->pc->aliases;
		  alias->next_alias;
		  alias = alias->next_alias ) {
		if ( !str_cmp (alias->next_alias->command, alias_cmd) ) {
			tmp_alias = alias->next_alias;
			alias->next_alias = tmp_alias->next_alias;
			alias_free (tmp_alias);
			return;
		}
	}
}

void alias_create (CHAR_DATA *ch, char *alias_cmd, char *value)
{
	ALIAS_DATA	*alias;
	char		line [MAX_STRING_LENGTH] = {'\0'};

	if ( !get_line (&value, line) )
		return;

	alias = (ALIAS_DATA *)alloc ((int)sizeof (ALIAS_DATA), 28);

	alias->command = str_dup (alias_cmd);
	alias->line = str_dup (line);

	alias->next_alias = ch->pc->aliases;
	ch->pc->aliases = alias;

	while ( get_line (&value, line) ) {
		alias->next_line = (ALIAS_DATA *)alloc ((int)sizeof (ALIAS_DATA), 28);
		alias = alias->next_line;

		alias->line = str_dup (line);
	}
}

void post_alias (DESCRIPTOR_DATA *d)
{
	CHAR_DATA		*ch;

	ch = d->character;

	alias_create (ch, (char *) ch->delay_info1, ch->delay_who);

	mem_free ((char *) ch->delay_info1);
	mem_free ((char *) ch->delay_who);
}

ALIAS_DATA *is_alias (CHAR_DATA *ch, char *buf)
{
	ALIAS_DATA	*alias;

	if ( !ch->pc )
		return NULL;

	if ( !*buf )
		return NULL;

	for ( alias = ch->pc->aliases; alias; alias = alias->next_alias )
		if ( !str_cmp (alias->command, buf) )
			return alias;

	return NULL;
}

void do_alias (CHAR_DATA *ch, char *argument, int cmd)
{
	int			seen_alias = 0;
	ALIAS_DATA	*alias;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		alias_cmd [MAX_STRING_LENGTH] = {'\0'};
	char		value [MAX_STRING_LENGTH] = {'\0'};

	if ( !ch->pc ) {
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	if ( GET_FLAG (ch, FLAG_ALIASING) ) {
		send_to_char ("Alias command can't be used by an alias, sorry.\n", ch);
		return;
	}

	argument = one_argument (argument, alias_cmd);

	if ( *alias_cmd == '?' ) {
		send_to_char ("  alias               - list aliases\n", ch);
		send_to_char ("  alias <cmd>         - list value of alias\n", ch);
        send_to_char ("  alias <cmd> =       - set value of alias\n", ch);
        send_to_char ("  alias <cmd> \"value\" - set value of alias\n", ch);
		return;
	}

	if ( !*alias_cmd ) {

		for ( alias = ch->pc->aliases; alias; alias = alias->next_alias ) {
			snprintf (buf, MAX_STRING_LENGTH,  "   %s\n", alias->command);
			send_to_char (buf, ch);
			seen_alias = 1;
		}

		if ( !seen_alias )
			send_to_char ("No aliases defined.\n", ch);

		return;
	}

	argument = one_argument (argument, value);

	if ( *value == '=' ) {
		alias_delete (ch, alias_cmd);

	    ch->delay_who = NULL;

		ch->delay_ch = ch;
		ch->delay_info1 = (int) str_dup (alias_cmd);

		ch->desc->str = &ch->delay_who;
		ch->desc->max_str = STR_MULTI_LINE;
		ch->desc->proc = post_alias;

		return;
	}

	else if ( *value ) {
		alias_delete (ch, alias_cmd);
		alias_create (ch, alias_cmd, value);
		return;
	}

	for ( alias = ch->pc->aliases; alias; alias = alias->next_alias ) {

		if ( str_cmp (alias_cmd, alias->command) )
			continue;

		snprintf (buf, MAX_STRING_LENGTH,  "Alias:  %s\n", alias->command);
		send_to_char (buf, ch);

		for ( ; alias; alias = alias->next_line ) {
			snprintf (buf, MAX_STRING_LENGTH,  "  %s\n", alias->line);
			send_to_char (buf, ch);
			seen_alias = 1;
		}

		break;
	}

	if ( !seen_alias )
		send_to_char ("No such alias.\n", ch);
}

int is_name_legal (char *buf)
{
	int		i;

	if ( strlen (buf) > 15 )
		return 0;

    for ( i = 0; i < strlen (buf); i++ ) {
        if ( !isalpha (buf [i]) ) {
            return 0;
        }

        if ( i )
            buf [i] = tolower (buf [i]);
        else
            buf [i] = toupper (buf [i]);
    }

	return 1;
}

void do_pfile (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*tch;
	char		pfile [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		new_name [MAX_STRING_LENGTH] = {'\0'};
	struct stat	stat_buf;

	if ( GET_TRUST (ch) != 5 ) {
		send_to_char ("No way.\n", ch);
		return;
	}

	argument = one_argument (argument, pfile);

	if ( !*pfile || *pfile == '?' ) {
		send_to_char (
"pfile <pc-name>   activate [<new-name>] - return from cold storage\n"
"                  archive               - place in cold storage\n"
"                  copy <new-name>       - make a copy of a pfile, including\n"
"                                          objects.\n"
"                  delete <pfile>        - delete an active pfile\n"
"                  rename <new-name>     - rename a pfile\n\n", ch);
		return;
	}

	if ( !is_name_legal (pfile) ) {
		send_to_char ("Illegal pfile name.\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( str_cmp (buf, "activate") &&
		 str_cmp (buf, "archive") &&
		 str_cmp (buf, "copy") &&
		 str_cmp (buf, "delete") &&
		 str_cmp (buf, "rename") ) {
		send_to_char ("Unrecognized keyword, type 'pfile ?' for help.\n", ch);
		return;
	}

	if ( !str_cmp (buf, "activate") ) {

		argument = one_argument (argument, new_name);

		if ( !*new_name )
			strcpy (new_name, pfile);

		if ( !is_name_legal (new_name) ) {
			send_to_char ("Illegal new-name.\n", ch);
			return;
		}

		snprintf (buf, MAX_STRING_LENGTH,  "save/player/%c/%s", tolower (*new_name), new_name);

			/* Stat returns 0 if successful, -1 if it fails */

		if ( stat (buf, &stat_buf) == 0 ) {
			send_to_char ("Target pfile (new-name) already exists.\n", ch);
			return;
		}

		snprintf (buf, MAX_STRING_LENGTH,  "save/archive/%s", new_name);

		if ( stat (buf, &stat_buf) == -1 ) {
			send_to_char ("Archive pfile (pc-name) does not exist in the "
						  "archive directory.\n", ch);
			return;
		}

				/* Pfile */

		snprintf (buf, MAX_STRING_LENGTH,  "mv save/archive/%s save/player/%c/%s",
					  new_name, tolower (*new_name), new_name);
		system (buf);

				/* Mobs saved with pfile */

		snprintf (buf, MAX_STRING_LENGTH,  "mv save/archive/%s.a save/player/%c/%s.a",
					  new_name, tolower (*new_name), new_name);
		system (buf);

				/* Player's objects */

		snprintf (buf, MAX_STRING_LENGTH,  "mv save/archive/%s.objs save/objs/%c/%s",
					  new_name, tolower (*new_name), new_name);
		system (buf);

		send_to_char ("Player character re-activated.\n", ch);
		return;
	}

	if ( !str_cmp (buf, "delete") ) {

send_to_char ("Delete operation.\n", ch);
send_to_char (pfile, ch);
send_to_char (" being deleted.\n", ch);
		tch = load_pc (pfile);

		if ( !tch ) {
			send_to_char ("That pfile doesn't exist or isn't active.\n", ch);
			return;
		}

		snprintf (buf, MAX_STRING_LENGTH,  "Load count is %d\n", tch->pc->load_count);
		send_to_char (buf, ch);

		if ( tch->pc->load_count != 1 ) {
			act ("The player can't be online and the pfile can't be loaded by "
                 "another admin when it is deleted.  You or someone else may "
                 "have the pfile 'mob'ed.", FALSE,
				 ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
			unload_pc (tch);
			return;
		}

		unload_pc (tch);

		snprintf (buf, MAX_STRING_LENGTH,  "rm save/player/%c/%s", tolower (*pfile), pfile);
send_to_char (buf, ch); send_to_char ("\n", ch);

		strcat (buf, ".a");
send_to_char (buf, ch); send_to_char ("\n", ch);

		snprintf (buf, MAX_STRING_LENGTH,  "rm save/objs/%c/%s", tolower (*pfile), pfile);
send_to_char (buf, ch); send_to_char ("\n", ch);

		strcat (buf, ".died");
send_to_char (buf, ch); send_to_char ("\n", ch);

		snprintf (buf, MAX_STRING_LENGTH,  "rm save/player/submit/%s", pfile);
send_to_char (buf, ch); send_to_char ("\n", ch);

		send_to_char ("Ok.\n", ch);

		return;
	}

	if ( !str_cmp (buf, "archive") ) {

		for ( tch = character_list; tch; tch = tch->next ) {

			if ( tch->deleted || !tch->pc )
				continue;

			if ( !str_cmp (new_name, tch->tname) ) {
				send_to_char ("That character is currently loaded.  Figure out "
							  "a way to unload it.\n", ch);
				return;
			}
		}
	}

send_to_char ("End of procedure\n", ch);
}

void do_last (CHAR_DATA *ch, char *argument, int cmd)
{
	time_t		local_time;
	CHAR_DATA	*tch;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !(tch = load_pc (buf)) ) {
		send_to_char ("No such PC.\n", ch);
		return;
	}

	if ( tch->pc->last_connect ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Last connect:     %s", ctime (&tch->pc->last_connect));
		send_to_char (buf, ch);
	}

	if ( tch->pc->last_logon ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Last logon:       %s", ctime (&tch->pc->last_logon));
		send_to_char (buf, ch);
	}

	if ( tch->pc->last_logoff ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Last logoff:      %s", ctime (&tch->pc->last_logoff));
		send_to_char (buf, ch);
	}

	if ( tch->pc->last_disconnect ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Last disconnect:  %s",
					  ctime (&tch->pc->last_disconnect));
		send_to_char (buf, ch);
	}

	if ( tch->pc->last_died ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Last died:        %s", ctime (&tch->pc->last_died));
		send_to_char (buf, ch);
	}

	local_time = time (0);

	send_to_char ("\nCurrent Server Time: ", ch);

	send_to_char (ctime (&local_time), ch);

	unload_pc (tch);
}

void do_log (CHAR_DATA *ch, char *argument, int cmd)
{
	int		immortal = 0, guest = 0, syst = 0, descend = 1, port_num = 0, room_num = -1;
	int		days = 0, hours = 0, minutes = 0, results = 0, prev_days = 0, curr_days = 0;
	char		*args = NULL, *date = NULL;
	char		npc [MAX_STRING_LENGTH] = {'\0'};
	char		name [MAX_STRING_LENGTH] = {'\0'};
	char		account [MAX_STRING_LENGTH] = {'\0'};
	char		command [MAX_STRING_LENGTH] = {'\0'};
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		query [MAX_STRING_LENGTH] = {'\0'};
	char		output [MAX_STRING_LENGTH] = {'\0'};
	char		search_args [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_ROW	row;
	MYSQL_RES	*result;
	bool		prev_arg = FALSE, alloc_test = FALSE, error = FALSE, truncated = FALSE;
	time_t		current_time;

	if ( port == PLAYER_PORT ) {
		send_to_char ("To avoid game lag, please do this on the builder port or website.\n", ch);
		return;
	}

	while ( *argument == ' ' )
		argument++;

	if ( !*argument ) {
		mysql_safe_query ("SELECT * FROM %s.mud", LOG_DATABASE);
		result = mysql_store_result (database);
		if ( !result ) {
			send_to_char ("What did you wish to search the logs for?\n", ch);
			return;
		}
		results = mysql_num_rows (result);
		snprintf (buf, MAX_STRING_LENGTH,  "The server's log database currently contains #6%d#0 entries.\n", results);
		send_to_char (buf, ch);
		mysql_free_result (result);
		return;
	}

	*query = '\0';
	*search_args = '\0';
	*name = '\0';
	*account = '\0';
	*npc = '\0';
	*command = '\0';

	port_num = port;

	if ( *argument == '(' ) {
		while ( *++argument != ')' ) {
			if ( !*argument ) {
				send_to_char ("You didn't include a closed parenthetical to end your search parameters.\n", ch);
				return;
			}
			snprintf (search_args + strlen(search_args), MAX_STRING_LENGTH, "%c", *argument);
		}
		if ( *argument == ')' )
			argument++;
	}

	if ( *search_args ) {
		args = str_dup (search_args);
		alloc_test = TRUE;
	}

	while ( args && *args && (args = one_argument (args, buf)) ) {
		if ( !strcasecmp (buf, "port") ) {
			args = one_argument (args, buf);
			if ( !isdigit (*buf) || (strtol(buf, NULL, 10) != PLAYER_PORT && strtol(buf, NULL, 10) != BUILDER_PORT && (strtol(buf, NULL, 10) != TEST_PORT || GET_TRUST (ch) < 5)) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Expected a port number to search on: %d, or %d.\n", PLAYER_PORT, BUILDER_PORT);
				send_to_char (buf, ch);
				mem_free (search_args);
				return;
			}
			else port_num = strtol(buf, NULL, 10);
		}
		else if ( !strcasecmp (buf, "player") ) {
			args = one_argument (args, buf);
			if ( !*buf ) {
				send_to_char ("Expected a player name to search for.\n", ch);
				mem_free (search_args);
				return;
			}
			else snprintf (name, MAX_STRING_LENGTH, "%s", buf);
		}
		else if ( !strcasecmp (buf, "account") ) {
			args = one_argument (args, buf);
			if ( !*buf ) {
				send_to_char ("Expected an account name to search for.\n", ch);
				mem_free (search_args);
				return;
			}
			else snprintf (account, MAX_STRING_LENGTH, "%s", buf);
		}
		else if ( !strcasecmp (buf, "command") ) {
			args = one_argument (args, buf);
			if ( !*buf ) {
				send_to_char ("Expected a command to search for.\n", ch);
				mem_free (search_args);
				return;
			}
			else snprintf (command, MAX_STRING_LENGTH, "%s", buf);
		}
		else if ( !strcasecmp (buf, "npc") ) {
			args = one_argument (args, buf);
			if ( !*buf ) {
				send_to_char ("Expected an NPC name to search for.\n", ch);
				mem_free (search_args);
				return;
			}
			else snprintf (npc, MAX_STRING_LENGTH, "%s", buf);
		}
		else if ( !strcasecmp (buf, "room") ) {
			args = one_argument (args, buf);
			if ( !isdigit (*buf) ) {
				send_to_char ("Expected a room number to search for.\n", ch);
				mem_free (search_args);
				return;
			}
			else room_num = strtol(buf, NULL, 10);
		}
		else if ( !strcasecmp (buf, "minutes") ) {
			args = one_argument (args, buf);
			if ( !isdigit (*buf) ) {
				send_to_char ("Expected a number of minutes to search within.\n", ch);
				mem_free (search_args);
				return;
			}
			else minutes = strtol(buf, NULL, 10);
		}
		else if ( !strcasecmp (buf, "hours") ) {
			args = one_argument (args, buf);
			if ( !isdigit (*buf) ) {
				send_to_char ("Expected a number of hours to search within.\n", ch);
				mem_free (search_args);
				return;
			}
			else hours = strtol(buf, NULL, 10);
		}
		else if ( !strcasecmp (buf, "days") ) {
			args = one_argument (args, buf);
			if ( !isdigit (*buf) ) {
				send_to_char ("Expected a number of days to search within.\n", ch);
				mem_free (search_args);
				return;
			}
			else days = strtol(buf, NULL, 10);
		}
		else if ( !strcasecmp (buf, "guest") )
			guest = TRUE;
		else if ( !strcasecmp (buf, "immortal") ) {
			if ( GET_TRUST (ch) < 5 ) {
				send_to_char ("Only level 5 staff members may search the immortal logs.\n", ch);
				mem_free (search_args);
				return;
			}	
			else immortal = TRUE;
		}
		else if ( !strcasecmp (buf, "system") )
			syst = TRUE;
		else if ( !strcasecmp (buf, "error") )
			error = TRUE;
		else if ( !strcasecmp (buf, "ascend") )
			descend = FALSE;
		else {
			snprintf (buf2, MAX_STRING_LENGTH,  "You've specified an unknown log search parameter: '%s'.\n", buf);
			send_to_char (buf2, ch);
			mem_free (search_args);
			return;
		}
		if ( !*args )
			break;
	}

	if (alloc_test && search_args )
		mem_free (search_args);

	if ( !*search_args && !*argument ) {
		send_to_char ("What did you wish to search the logs for?\n", ch);
		return;
	}

	snprintf (query, MAX_STRING_LENGTH, "SELECT AES_DECRYPT(name,'%s'),AES_DECRYPT(account,'%s'),AES_DECRYPT(switched_into,'%s'),timestamp,port,room,guest,immortal,error,AES_DECRYPT(command,'%s'),AES_DECRYPT(entry,'%s') FROM server_logs.mud WHERE ",
			CRYPT_PASS, CRYPT_PASS, CRYPT_PASS, CRYPT_PASS, CRYPT_PASS);

	if ( *argument )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "(");
	else {
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "(AES_DECRYPT(entry,'%s') LIKE '%%')", CRYPT_PASS);

	}

	while ( *argument && (argument = one_argument (argument, buf)) ) {
		if ( !strcasecmp (buf, "and") || !str_cmp (buf, "&") ) {
			snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND ");
			prev_arg = FALSE;
			continue;
		}
		else if ( !strcasecmp (buf, "or") ) {
			snprintf (query + strlen(query), MAX_STRING_LENGTH, " OR ");
			prev_arg = FALSE;
			continue;
		}
		else if ( prev_arg ) {
			send_to_char ("You'll need to include either \"or\" or \"and\" for multiple terms.\n", ch);
			return;
		}
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "AES_DECRYPT(entry,'%s') LIKE '%%%s%%'", CRYPT_PASS, buf);

		prev_arg = TRUE;
		if ( !*argument ) {
			snprintf (query + strlen(query), MAX_STRING_LENGTH, ")");
			break;
		}
	}

	if ( !syst )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND AES_DECRYPT(name,'%s') != 'System'", CRYPT_PASS);
	else snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND AES_DECRYPT(name,'%s') = 'System'", CRYPT_PASS);


	if ( !immortal )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND immortal = FALSE");
	else snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND immortal = TRUE");

	if ( !guest )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND guest = FALSE");
	else snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND guest = TRUE");

	if ( error )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND error = TRUE");
	else snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND error = FALSE");

	if ( minutes )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND (UNIX_TIMESTAMP() - timestamp) <= 60 * %d", minutes);
	else if ( hours )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND (UNIX_TIMESTAMP() - timestamp) <= 60 * 60 * %d", hours);
	else if ( days )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND (UNIX_TIMESTAMP() - timestamp) <= 60 * 60 * 24 * %d", days);

	if ( !days && !hours && !minutes )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND (UNIX_TIMESTAMP() - timestamp) <= 60 * 60 * 24 * 14");

	snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND port = %d", port_num);

	if ( room_num > -1 )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND room = %d", room_num);

	if ( *name )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND AES_DECRYPT(name,'%s') LIKE '%s'", CRYPT_PASS, name);

	if ( *account )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND AES_DECRYPT(account,'%s') LIKE '%s'", CRYPT_PASS, account);

	if ( *npc )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND AES_DECRYPT(switched_into,'%s') LIKE '%s'", CRYPT_PASS, npc);

	if ( *command )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " AND AES_DECRYPT(command,'%s') LIKE '%s'", CRYPT_PASS, command);


	if ( !descend )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, " ORDER BY timestamp ASC");
	else snprintf (query + strlen(query), MAX_STRING_LENGTH, " ORDER BY timestamp DESC");

	snprintf (query + strlen(query), MAX_STRING_LENGTH, " LIMIT 1000");

	/*
	* Safe to assume that people with access to the log command won't be
	* injecting SQL, and mysql_safe_query() would be a royal pain in the arse
	* to use here.
	*/

	mysql_real_query (database, query, strlen(query));
	result = mysql_store_result (database);

	if ( !result || mysql_num_rows(result) == 0 ) {
		send_to_char ("Your log search did not return any results.\n", ch);
		return;
	}

	*output = '\0';

	while ( (row = mysql_fetch_row (result)) ) {
		curr_days = strtol(row[3], NULL, 10) / (60 * 60);

		if ( curr_days != prev_days ) {
			current_time = strtol(row[3], NULL, 10);
			date = asctime(localtime(&current_time));
			date [strlen(date) -1] = '\0';
			if ( descend )
				snprintf (output + strlen(output), MAX_STRING_LENGTH,  "\n#2Prior to %s:#0\n\n", date);
			else
				snprintf (output + strlen(output), MAX_STRING_LENGTH,  "\n#2After %s:#0\n\n", date);
			prev_days = curr_days;
		}

		if ( *npc )
			snprintf (buf, MAX_STRING_LENGTH,  "%s (%s) [%s]: %s %s\n", row[2], row[0], row[5], row[9], row[10]);
		else if ( *account )
			snprintf (buf, MAX_STRING_LENGTH,  "%s (%s) [%s]: %s %s\n", row[1], row[0], row[5], row[9], row[10]);
		else if ( syst )
			snprintf (buf, MAX_STRING_LENGTH,  "System Message: %s\n", row[10]);
		else snprintf (buf, MAX_STRING_LENGTH,  "%s [%s]: %s %s\n", row[0], row[5], row[9], row[10]);

		if ( strlen (output) + strlen(buf) >= MAX_STRING_LENGTH ) {
			truncated = TRUE;
			snprintf (buf, MAX_STRING_LENGTH,  "...output truncated. Too many results.\n");
			if ( strlen(output) + strlen(buf) < MAX_STRING_LENGTH )
				snprintf (output + strlen(output), MAX_STRING_LENGTH,  "%s", buf);
			else break;
		}
		snprintf (output + strlen(output), MAX_STRING_LENGTH,  "%s", buf);
	}

	if ( !*output ) {
		send_to_char ("Your log search did not return any results.\n", ch);
		mysql_free_result (result);
		return;
	} else {
		results = mysql_num_rows(result);
		snprintf (buf, MAX_STRING_LENGTH,  "#6Entries Matching Your Query:#0 %d%s\r\n%s", results, truncated ? " (truncated)" : "", output);
		page_string (ch->desc, buf);
	}

	mysql_free_result (result);
}

void write_stats (void)
{
	static FILE		*fp_stats = NULL;
	int				online = 0, guest = 0;
	CHAR_DATA		*tch;
	DESCRIPTOR_DATA	*td;
	char			admins [MAX_STRING_LENGTH] = {'\0'};

	if ( fp_stats == NULL ) {
		if ( !(fp_stats = fopen (STATS_FILE, "a")) )
			return;
		fprintf (fp_stats, "%ld BOOT\n", starttime);
	}

    for ( td = descriptor_list; td; td = td->next) {

        if ( !td->character )
            continue;

        if ( !td->character->pc )
            continue;

        if ( td->character->pc->level )
            continue;

        if ( td->character->pc->create_state != 2 )
            continue;

        if ( td->connected )
            continue;

	if ( IS_SET (td->character->flags, FLAG_GUEST) )
		guest++;

        if ( IS_MORTAL (td->character) && !IS_SET (td->character->flags, FLAG_GUEST) )
            online++;
    }

	fprintf (fp_stats, "%ld STAT %d %d\n", time (0), online, count_max_online);

	*admins = '\0';

	for ( tch = character_list; tch; tch = tch->next ) {

		if ( tch->deleted )
			continue;

		if ( !tch->desc )
			continue;

		if ( !tch->pc || !tch->pc->level )
			continue;

		strcat (admins, GET_NAME (tch));
		strcat (admins, " ");
	}

	fprintf (fp_stats, "%ld ADMINS %s\n", time (0), admins);

	fflush (fp_stats);
}

void list_help (CHAR_DATA *ch, HELP_DATA *help)
{
	int			i = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	*buf = '\0';

	while ( help ) {

		if ( help->master_element ) {

			snprintf (ADDBUF, MAX_STRING_LENGTH, "%-18s ", help->keyword);

			if ( ++i == 4 ) {
				i = 0;
				send_to_char (buf, ch);
				send_to_char ("\n", ch);
				*buf = '\0';
			}
		}

		help = help->next;
	}

	if ( *buf ) {
		send_to_char (buf, ch);
		send_to_char ("\n", ch);
	}
}

void list_texts (CHAR_DATA *ch, TEXT_DATA *list)
{
	int			i = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	*buf = '\0';

	while ( list ) {

		snprintf (ADDBUF, MAX_STRING_LENGTH, "%-18s ", list->name);

		if ( ++i == 4 ) {
			i = 0;
			send_to_char (buf, ch);
			send_to_char ("\n", ch);
			*buf = '\0';
		}

		list = list->next;
	}

	if ( *buf ) {
		send_to_char (buf, ch);
		send_to_char ("\n", ch);
	}
}

TEXT_DATA *find_text (TEXT_DATA *list, char *buf)
{
	while ( list ) {

		if ( !str_cmp (list->name, buf) )
			return list;

		list = list->next;
	}

	return NULL;
}

char *get_text_buffer (CHAR_DATA *ch, TEXT_DATA *list, char *text_name)
{
	TEXT_DATA		*entry;

	if ( (entry = find_text (list, text_name)) )
		return entry->text;

	return "(no info)\n";
}

int get_edit_line_length (char **line)
{
	int		length = 0;

	if ( !**line )
		return length;

	while ( **line && **line != '\n' ) {
		(*line)++;
		length++;
	}

	if ( **line ) {
		length++;
		(*line)++;
	}

	return length;
}

char *get_document (int type, int index)
{
	if ( type == COMPOSE_TYPE_TEXT )
		return ((TEXT_DATA *) index)->text;

	if ( type == COMPOSE_TYPE_HELP || type == COMPOSE_TYPE_BHELP )
		return ((HELP_DATA *) index)->help_info;

	printf ("Type: %d, index: %d\n", type, index); fflush (stdout);

	return NULL;
}

void set_document (int type, int index, char *new_string)
{
	TEXT_DATA		*text;
	HELP_DATA		*element;

	if ( type == COMPOSE_TYPE_TEXT ) {
		text = (TEXT_DATA *) index;
		mem_free (text->text);
		text->text = new_string;
	}

	else if ( type == COMPOSE_TYPE_HELP || type == COMPOSE_TYPE_BHELP ) {
		element = (HELP_DATA *) index;
		mem_free (element->help_info);
		element->help_info = new_string;
	}

	else {
		printf ("type %d, index %d\n", type, index);
	}
}

void edit_string (DESCRIPTOR_DATA *d, char *argument)
{
	int			new_doc_length;
	int			length = 0;
	int			i;
	char		*document;
	char		*new_doc;
	char		*p;

	if ( *argument != '@' ) {
		p = malloc (strlen (d->edit_string) + strlen (argument) + 2);

		strcpy (p, d->edit_string);
		strcat (p, argument);
		strcat (p, "\n");

		mem_free (d->edit_string);

		d->edit_string = p;

		if ( d->edit_line_first >= 100 )
			SEND_TO_Q ("   >", d);
		else
			SEND_TO_Q ("  >", d);

		return;
	}

	document = get_document (d->edit_type, d->edit_index);

	new_doc_length = strlen (document) -
					 d->edit_length +
					 strlen (d->edit_string) + 1;

	new_doc = malloc (new_doc_length);

	*new_doc = '\0';

	p = document;

		/* How many bytes from the beginning of original string? */

/* printf ("line_first - 1 = %d\n", d->edit_line_first); */
	for ( i = 1; i < d->edit_line_first; i++ )
		length += get_edit_line_length (&p);

	memcpy (new_doc, document, length);
/* printf ("Doc %d:\n%s<-", length, new_doc); */
	strcpy (new_doc + length, d->edit_string);
/* printf ("Plus:\n%s<-", new_doc);
   fflush (stdout);
*/
	
		/* Add lines at end of the document */

	strcpy (new_doc + strlen (new_doc),
			document + length + d->edit_length);

	mem_free (d->edit_string);

	set_document (d->edit_type, d->edit_index, new_doc);

	d->edit_index = -1;

	SEND_TO_Q ("Document needs to be saved.\n", d);
}

int doc_parse (CHAR_DATA *ch, char *argument, char **start, int *length,
			   int *start_line, int *doc_type)
{
	int			doc_num;
	int			line_start;
	int			line_end;
	int			line;
	int			i;
	HELP_DATA	*element;
	TEXT_DATA	*text;
	char		*document;
	char		*p;
	char		buf [MAX_STRING_LENGTH];

	*start_line = 1;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		send_to_char ("Expected TEXT, HELP, or BHELP.\n", ch);
		return -1;
	}

	if ( *buf == '.' ) {
		if ( !ch->pc ) {
			send_to_char ("Only use '.' while being a PC.\n", ch);
			return -1;
		}

		if ( ch->pc->doc_index == -1 ) {
			send_to_char ("No document set.  Use DOCUMENT <doc> first.\n", ch);
			return -1;
		}

		*doc_type = ch->pc->doc_type;
		doc_num = ch->pc->doc_index;

		document = get_document (*doc_type, doc_num);
	}

	else if ( !str_cmp (buf, "help") ) {

		argument = one_argument (argument, buf);

		if ( !*buf ) {
			list_help (ch, help_list);
			return -1;
		}

		if ( !(element = is_help (help_list, buf)) ) {
			send_to_char ("No such help topic.\n", ch);
			return -1;
		}

		*doc_type = COMPOSE_TYPE_HELP;
		doc_num = (int) element;
		document = element->help_info;
	}

	else if ( !str_cmp (buf, "bhelp") ) {

		argument = one_argument (argument, buf);

		if ( !*buf ) {
			list_help (ch, bhelp_list);
			return -1;
		}

		if ( !(element = is_help (bhelp_list, buf)) ) {
			send_to_char ("No such bhelp topic.\n", ch);
			return -1;
		}

		*doc_type = COMPOSE_TYPE_BHELP;
		doc_num = (int) element;
		document = element->help_info;
	}

	else if ( !str_cmp (buf, "text") ) {

		argument = one_argument (argument, buf);

		if ( !*buf ) {
			list_texts (ch, text_list);
			return -1;
		}

		if ( !(text = find_text (text_list, buf)) ) {
			send_to_char ("No such text.\n", ch);
			return -1;
		}

		*doc_type = COMPOSE_TYPE_TEXT;
		doc_num = (int) text;
		document = text->text;
	}

	
	else {
		send_to_char ("No such document type, expected on of TEXT, HELP or BHELP.\n", ch);
		return -1;
	}

	ch->pc->doc_type = *doc_type;
	ch->pc->doc_index = doc_num;

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		*length = strlen (document);
		*start = document;
		*start_line = 1;
		return doc_num;
	}

	if ( *buf == '[' ) {
		*length = 0;
		*start = document;
		*start_line = 1;
		return doc_num;
	}

	if ( *buf == ']' ) {
		*length = 0;
		*start = &document [strlen (document)];

		*start_line = 0;

		for ( p = document; *p; (*start_line)++ )
			get_edit_line_length (&p);

		return doc_num;
	}

	if ( !isdigit (*buf) ) {
		send_to_char ("Expected line or range:  ##  or  ##/##\n", ch);
		return -1;
	}

	if ( just_a_number (buf) ) {

		line = strtol(buf, NULL, 10);
		*start_line = line;
		*length = 0;
		*start = document;

		while ( **start && line-- > 0 )
			get_edit_line_length (start);

		return doc_num;
	}

	line_start = strtol(buf, NULL, 10);
	p = buf;

	while ( isdigit (*p) )
		p++;

	if ( !*p || *p != '/' ) {
		send_to_char ("Expected '/' between line numbers.\n", ch);
		return -1;
	}

	p++;

	line_end = strtol(p, NULL, 10);

	if ( line_start > line_end ) {
		send_to_char ("You cannot edit a negative range like that.\n", ch);
		return -1;
	}

	if ( line_start < 1 )
		line_start = 1;

	*start_line = line_start;

	*start = document;

	for ( i = 1; i < line_start; i++ ) {
		get_edit_line_length (start);
		if ( !**start ) {
			send_to_char ("You can't edit beyond the last line like that.\n",
						  ch);
			free(start);			  
			return -1;
		}
	}

	*length = 0;
	p = *start;

	while ( i++ <= line_end ) {
		*length += get_edit_line_length (&p);
		if ( !*p ) {
			(*length)--;
			free(start);
			return doc_num;
		}
	}
	free(start);
	return doc_num;
}

void do_edit (CHAR_DATA *ch, char *argument, int cmd)
{
	int			length;
	int			line_start;
	int			doc_type;
	int			doc_num;
	char		*start;

	if ( !ch->desc ) {
		send_to_char ("This is a PC only command.\n", ch);
		return;
	}

	do_print (ch, argument, cmd);

	while ( isspace (*argument) )
		argument++;

	if ( !*argument )
		return;

	if ( -1 == (doc_num = doc_parse (ch, argument, &start, &length,
									 &line_start, &doc_type)) )
		return;

	ch->desc->edit_string = str_dup ("");

	ch->desc->edit_type = doc_type;
	ch->desc->edit_index = doc_num;
	ch->desc->edit_line_first = line_start;
	ch->desc->edit_length = length;

	if ( ch->desc->edit_line_first >= 100 )
		SEND_TO_Q ("   >", ch->desc);
	else
		SEND_TO_Q ("  >", ch->desc);
}

void do_print (CHAR_DATA *ch, char *argument, int cmd)
{
	int			length;
	int			line_start;
	int			line_length;
	int			doc_type;
	char		*start;
	char		*cur_line;
	char		*p;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( -1 == doc_parse (ch, argument, &start, &length, &line_start,
						  &doc_type) )
		return;

	cur_line = start;

	for ( ; *cur_line && start + length > cur_line; line_start++ ) {

		p = cur_line;

		line_length = get_edit_line_length (&cur_line);

		if ( !*cur_line )
			return;

		snprintf (buf, MAX_STRING_LENGTH,  "%2d>", line_start);

		buf [strlen (buf) + line_length] = '\0';

		memcpy (ADDBUF, p, line_length);

		send_to_char (buf, ch);
	}
}

void save_document (CHAR_DATA *ch, char *argument)
{
	char		*p;
	TEXT_DATA	*text;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	p = argument;
	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "help") ) {
		write_help (HELP_FILE, help_list);
		send_to_char ("Help file written.\n", ch);
		return;
	}

	if ( !str_cmp (buf, "bhelp") ) {
		write_help (BHELP_FILE, bhelp_list);
		send_to_char ("Bhelp file written.\n", ch);
		return;
	}

	if ( !str_cmp (buf, "text") ) {

		text = find_text (text_list, argument);

		if ( !text ) {
			send_to_char ("No such text.\n", ch);
			return;
		}

		write_text (ch, text);
		return;
	}

	if ( *buf == '.' ) {

		if ( ch->pc->doc_index == -1 ) {
			send_to_char ("You don't have a current document set.\n", ch);
			return;
		}

		if ( ch->pc->doc_type == COMPOSE_TYPE_TEXT )
			write_text (ch, (TEXT_DATA *) ch->pc->doc_index);

		else if ( ch->pc->doc_type == COMPOSE_TYPE_HELP )
			write_help (HELP_FILE, help_list);

		else if ( ch->pc->doc_type == COMPOSE_TYPE_BHELP )
			write_help (BHELP_FILE, bhelp_list);

		else {
			send_to_char ("Unknown document type for save.\n", ch);
			return;
		}

		send_to_char ("Written.\n", ch);
		return;
	}

	send_to_char ("Expected '.', TEXT, HELP, or BHELP.\n", ch);
}

void write_text (CHAR_DATA *ch, TEXT_DATA *text)
{
	int			bytes;
	int			bytes_written;
	char		*p;
	FILE		*fp_doc;

	unlink (text->filename);

	if ( !(fp_doc = fopen (text->filename, "w")) ) {
		perror ("save_document");
		send_to_char ("Tragically, there was a problem writing the text.  "
					  "It is probably lost now.\n", ch);
		return;
	}

	p = text->text;
	bytes = strlen (p);

	while ( bytes ) {

		bytes_written = fwrite (p, 1, bytes, fp_doc);

		if ( !bytes_written ) {
			perror ("fwrite of text");
			break;
		}

		bytes -= bytes_written;
		p += bytes_written;
	}

	if ( bytes == strlen (text->text) )
		send_to_char ("No part of the text could be written!\n", ch);
	else if ( bytes )
		send_to_char ("Part of the text could not be written!\n", ch);
	else
		send_to_char ("Document written.\n", ch);

	fclose (fp_doc);
}

HELP_DATA *add_help_topics (CHAR_DATA *ch, HELP_DATA **list, char *argument)
{
	HELP_DATA	*master_element;
	HELP_DATA	*element;
	HELP_DATA	*last_element = NULL;
	char		*p;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	for ( p = one_argument (argument, buf);
		  *buf;
		  p = one_argument (p, buf)) {

		for ( element = *list; ch && element; element = element->next ) {
			if ( !element->master_element )
				continue;

			if ( !str_cmp (element->keyword, buf) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Keyword %s is already in use.\n", buf);
				send_to_char (buf, ch);
				return NULL;
			}
		}
	}

	if ( *list )
		for ( last_element = *list;
			  last_element->next;
			  last_element = last_element->next )
			;

	master_element = (HELP_DATA *)alloc ((int)sizeof (HELP_DATA), 36);
	master_element->keywords = str_dup (argument);
	master_element->help_info = str_dup ("");

	if ( last_element )
		last_element->next = master_element;
	else
		*list = master_element;

	last_element = master_element;

	for ( p = one_argument (argument, buf);
		  *buf;
		  p = one_argument (p, buf)) {
		
		element = (HELP_DATA *)alloc ((int)sizeof (HELP_DATA), 36);

		element->master_element = master_element;
		element->help_info = NULL;
		element->keyword = str_dup (buf);

		last_element->next = element;
		last_element = element;
	}

	return master_element;
}

void delete_help_topics (CHAR_DATA *ch, HELP_DATA **list, char *argument)
{
}


#define AVG_STRING_LENGTH 512
#define MAP_MAX_RADIUS 3
#define MAP_GRID_WIDTH ((MAP_MAX_RADIUS * 2) + 1)
#define MAP_GRID_DEPTH 5

void
fill_map (ROOM_DATA * ptrRoom, int x, int y,
	  int map[MAP_GRID_DEPTH][MAP_GRID_WIDTH][MAP_GRID_WIDTH])
{
  int n = 0, e = 0, s = 0, w = 0;
  static unsigned char radius = 0;
  ROOM_DIRECTION_DATA *ptrNExit = NULL;
  ROOM_DIRECTION_DATA *ptrEExit = NULL;
  ROOM_DIRECTION_DATA *ptrSExit = NULL;
  ROOM_DIRECTION_DATA *ptrWExit = NULL;

  if (!ptrRoom)
    return;

  if (!map[0][x][y])
    {
      map[0][x][y] = ptrRoom->virtual;
    }
  map[1][x][y] = (ptrRoom->sector_type >= 0
		  && ptrRoom->sector_type < 21) ? ptrRoom->sector_type : 21;
  if ((ptrEExit = ptrRoom->dir_option[1]) != NULL)
    {
      map[2][x][y] = (IS_SET (ptrEExit->exit_info, PASSAGE_ISDOOR)) ? 2 : 1;
    }
  if ((ptrSExit = ptrRoom->dir_option[2]) != NULL)
    {
      map[3][x][y] = (IS_SET (ptrSExit->exit_info, PASSAGE_ISDOOR)) ? 2 : 1;
    }
  if ((ptrRoom->dir_option[4]) != NULL)
    {
      map[4][x][y] = 1;
    }
  if ((ptrRoom->dir_option[5]) != NULL)
    {
      map[4][x][y] = (map[4][x][y]) ? 3 : 2;
    }


  if ((y > 0) && !map[0][x][y - 1] && (ptrNExit = ptrRoom->dir_option[0]))
    {
      n = ptrNExit->to_room;
    }
  if (!map[0][x + 1][y] && (x + 1 < MAP_GRID_WIDTH) && (ptrEExit != NULL))
    {
      e = ptrEExit->to_room;
    }
  if (!map[0][x][y + 1] && (y + 1 < MAP_GRID_WIDTH) && (ptrSExit != NULL))
    {
      s = ptrSExit->to_room;
    }
  if (!map[0][x - 1][y] && (x - 1 >= 0)
      && (ptrWExit = ptrRoom->dir_option[3]))
    {
      w = ptrWExit->to_room;
    }

  if (radius > MAP_MAX_RADIUS + 1)
    return;

  radius++;

  if (n)
    {
      fill_map (vtor (n), x, y - 1, map);
    }
  if (e)
    {
      fill_map (vtor (e), x + 1, y, map);
    }
  if (s)
    {
      fill_map (vtor (s), x, y + 1, map);
    }
  if (w)
    {
      fill_map (vtor (w), x - 1, y, map);
    }
  radius--;
}



void
do_map (CHAR_DATA * ch, char *argument, int cmd)
{
  int map[MAP_GRID_DEPTH][MAP_GRID_WIDTH][MAP_GRID_WIDTH];
  char buf[AVG_STRING_LENGTH * 2] = "";
  char buf2[AVG_STRING_LENGTH] = "";
  char arg1[AVG_STRING_LENGTH] = "";
  char bogy[4][2][6] = {
    {" ", " "},
    {"#b(#0", "#b)#0"},
    {"#d(#0", "#d)#0"},
    {"#f(#0", "#f)#0"}
  };
  const char strEWall[3][7] = {
    "#1X#0",
    " ",
    "#1|#0"
  };
  const char strSWall[3][18] = {
    "#1XXXXXXXXXXX#0",
    "          #1X#0",
    "#1----------X#0"
  };
  const char strVExit[4][7] = {
    " ",
    "#6'#0",
    "#6,#0",
    "#6%#0"
  };
  const char *strSect[] = {
    "", "#7", "#7", "#7", "#a",
    "#2", "#2", "#3", "#b", "#e",
    "#6", "#e", "#a", "#3", "#d",
    "", "#4", "#4", "#4", "#6",
    "#c", "#9"
  };
  unsigned char i = 0, j = 0, x = 0, y = 0, nInRoom = 0, bSearch = 0;
  int r = 0;
  CHAR_DATA *rch;
  ROOM_DATA *room;

  if (*argument)
    {
      bSearch = 1;
      argument = one_argument (argument, arg1);
    }

  for (i = 0; i < MAP_GRID_WIDTH; i++)
    {
      for (j = 0; j < MAP_GRID_WIDTH; j++)
	{
	  map[0][i][j] = 0;	/* room */
	  map[1][i][j] = 0;	/* sect */
	  map[2][i][j] = 0;	/* e links */
	  map[3][i][j] = 0;	/* s links */
	  map[4][i][j] = 0;	/* u/d links */
	}
    }
  x = MAP_MAX_RADIUS;
  y = MAP_MAX_RADIUS;

  fill_map (ch->room, x, y, map);

  strcpy (buf, "#0\n");
  for (j = 0; j < MAP_GRID_WIDTH; j++)
    {
      for (i = 0; i < MAP_GRID_WIDTH; i++)
	{
	  if ((r = map[0][i][j]) > 0)
	    {
	      if ((room = vtor (r)))
		{

		  if (i == MAP_MAX_RADIUS && j == MAP_MAX_RADIUS)
		    {
		      sprintf (buf + strlen (buf), " #d<#0%s%6d#d>#0%s%s",
			       strSect[map[1][i][j]],
			       r,
			       strVExit[map[4][i][j]],
			       strEWall[map[2][i][j]]);
		    }
		  else
		    {
		      nInRoom = 0;
		      if ((rch = room->people))
			{
			  for (; rch; rch = rch->next_in_room)
			    {
			      if (bSearch && strstr (rch->name, arg1))
				{
				  nInRoom = 3;
				  break;
				}
			      if (GET_TRUST (rch))
				{
				  nInRoom = 2;
				  break;
				}
			      if (!IS_NPC (rch))
				{
				  nInRoom = 1;
				  break;
				}
			    }
			}
		      sprintf (buf + strlen (buf), " %s%s%6d#0%s%s%s",
			       bogy[nInRoom][0],
			       strSect[map[1][i][j]],
			       r,
			       bogy[nInRoom][1],
			       strVExit[map[4][i][j]],
			       strEWall[map[2][i][j]]);
		    }
		}
	      strcat (buf2, strSWall[map[3][i][j]]);

	    }
	  else
	    {
	      sprintf (buf + strlen (buf), "          %s",
		       ((i < MAP_GRID_WIDTH - 1)
			&& (map[0][i + 1][j])) ? strEWall[0] : " ");
	      if (j < MAP_GRID_WIDTH - 1 && map[0][i][j + 1])
		{
		  strcat (buf2, strSWall[0]);
		}
	      else
		{
		  sprintf (buf2 + strlen (buf2), "          %s",
			   ((i < MAP_GRID_WIDTH - 1)
			    && (map[0][i + 1][j])) ? strEWall[0] : "#1+#0");
		}
	    }
	}
      strcat (buf, "#0\n");
      strcat (buf2, "#0\n");
      strcat (buf, buf2);
      send_to_char (buf, ch);
      buf[0] = '\0';
      buf[1] = '\0';
      buf2[0] = '\0';
      buf[2] = '\0';
    }

  strcpy (buf, "\n  #6'#0 Up       #6,#0 Down     #6%#0 Up/Down\n  ");

  for (i = 0; sector_types[i][0] != '\n'; i++)
    {
      sprintf (buf + strlen (buf), "%s%-10s#0 ", strSect[i], sector_types[i]);
      if (!((i + 1) % 7))
	strcat (buf, "\n  ");
    }

  send_to_char (buf, ch);
}
