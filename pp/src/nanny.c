/** 
*	\file nanny.c
*	A nanny to hold your hand through character generation
*
*	This module provides functionality for creating new accounts, new
*	characters, and retiring old characters. It also includes fucntionality for
*	Mud mail
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
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/telnet.h>
#include <mysql/mysql.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

char echo_off_str [] = {(char) IAC,(char) WILL,(char) TELOPT_ECHO, '\0'};
char echo_on_str []  = {(char) IAC,(char) WONT,(char) TELOPT_ECHO,
						'\r', '\n', '\0'};

#define QECHO_OFF		SEND_TO_Q (echo_off_str, d);
#define QECHO_ON			SEND_TO_Q (echo_on_str, d);

#define MAX_PC_LIMIT		100

int				new_accounts = 0;

/*                                                                          *
 * function: display_unread_messages                                        *
 *                                                                          *
 *                                                                          */
void display_unread_messages (DESCRIPTOR_DATA *d)
{
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	int				unread = 0;

	if ( !d->account || !d->account->name )
		return;

	mysql_safe_query ("SELECT flags FROM hobbitmail WHERE account = '%s' ORDER BY timestamp DESC", d->account->name);

        if ( ( result = mysql_store_result ( database ) ) == NULL ) {
	  snprintf (buf, MAX_STRING_LENGTH, "Warning: display_unread_messages(): %s", mysql_error ( database ) );
          system_log( buf, TRUE );
          return;
        }
	
	while ( ( row = mysql_fetch_row( result ) ) != NULL ) {
		if ( !IS_SET (strtol(row[0], NULL, 10), MF_READ) )
			unread++;
	}

	mysql_free_result ( result ); 
	result = NULL;

	if ( !unread )
		return;

	snprintf (buf, MAX_STRING_LENGTH,  "#6There %s %d unread Hobbit-Mail%s awaiting your attention!#0\n\n", unread > 1 ? "are" : "is", unread, unread > 1 ? "s" : "");
	SEND_TO_Q (buf, d);
	
	return;
}

void display_main_menu (DESCRIPTOR_DATA *d)
{
	char	*temp_arg = NULL;
	
/* Menu for logged in accounts */
	temp_arg = get_text_buffer (NULL, text_list, "menu1");
	SEND_TO_Q (temp_arg, d);
	display_unread_messages (d);
	SEND_TO_Q ("Your Selection: ", d);
	d->connected = CON_ACCOUNT_MENU;
}

char *encrypt_buf (char *buf)
{
	char	*temp_arg = NULL;
	extern char *crypt(const char *key, const char *salt);

	temp_arg = crypt (buf, "CR");
	return str_dup (temp_arg);
	
}

int check_password (char *pass, char *encrypted)
{
	char	*p = NULL;
	int		return_value = 0;

	p = encrypt_buf (pass);

	return_value = (strcmp (p, encrypted) == 0);

	if ( p != NULL && strlen (p) > 1 )
		mem_free (p);

	return return_value;
}

void nanny_login_choice (DESCRIPTOR_DATA *d, char *argument)
{
	SITE_INFO	*site = NULL;
	int			is_banned = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	argument = one_argument (argument, buf);

	while ( *buf && buf [strlen (buf) - 1] == ' ' )
		buf [strlen (buf) - 1] = '\0';

	*buf = toupper (*buf);

/****Banned sites ******/
        if ( banned_site && banned_site->name ) {
        	for ( site = banned_site; site; site = site->next ) {
				if ( !str_cmp (banned_site->name, "*") ) {
					is_banned++;
					break;
				}
				if ( strstr (d->host, site->name) ) {
					is_banned++;
					break;
				}
        	}
        }
                
        if ( is_banned && *buf != 'L' ) {   
        	snprintf (buf, MAX_STRING_LENGTH,  "\nYour site is currently banned from connecting to %s.\n"
                              "As a result of the large number of people we get from certain Internet\n"
                              "domains, this may not be a result of anything you've done; if you feel that\n"
                              "this is the case, please email our staff at %s and we\n"
                              "will do our best to resolve this issue. Our apologies for the inconvenience.\n", MUD_NAME, STAFF_EMAIL);
       		SEND_TO_Q (buf, d);
			snprintf (buf, MAX_STRING_LENGTH,  "\nPlease press ENTER to disconnect from the server.\n");
            SEND_TO_Q (buf, d);
            d->connected = CON_PENDING_DISC;
		return;
        }     
/******* end banned sites *********/

/******* basic open menu options *****/

	if ( strcmp(buf, "C") && strcmp(buf, "L") && strcmp(buf, "X") ) {
		SEND_TO_Q ("That is not a valid option, friend.\n", d);
		strcpy(buf, "");
		SEND_TO_Q ("Your Selection: ", d);
		return;
	}


/****** maintanence ******/

		if ( maintenance_lock ) {
			SEND_TO_Q ("\nSorry, but we are currently closed for maintenance.\n\n", d);
			strcpy(buf, "");
			SEND_TO_Q ("Your Selection: ", d);
			return;
		}
/******* end maintenance *****/

/****** Logging in to an account ******/
	else if ( !strcmp(buf, "L")) {
		SEND_TO_Q ("Your account name: ", d);
		d->connected = CON_ENTER_ACCT_NME;
		return;
	}

/****** end log in to existing account *****/

/***** create a new account *****/
	
	else if ( !strcmp(buf, "C")) {
		if ( strstr (d->host, "localhost") && reference_ip (NULL, d->host) ) {
			SEND_TO_Q ("#1\nWe apologize, but our records indicate that an account has already been\n"
	  				   "registered from this IP. If you have legitimate reasons for obtaining another\n"
	   				   "login account, such as sharing your internet connection with other individuals,\n"
	   				   "or you are unaware of any other accounts used over your connection, please\n"
	   				   "contact the administrative staff at "STAFF_EMAIL" for assistance.\n\n", d);
			strcpy(buf, " ");
			SEND_TO_Q ("Your Selection: ", d);
			return;
		}
		temp_arg = get_text_buffer (NULL, text_list, "account_application");
		SEND_TO_Q (temp_arg, d);
		SEND_TO_Q ("What would you like to name your login account? ", d);
		d->connected = CON_NEW_ACCT_NAME;
		return;
	}
/****** end creation of new account ******/

/***** exit opening menu screeen ******/		
	else if ( !strcmp(buf, "X")) {
		close_socket(d);
		return;
	}
/***** end of exiting *****/

	return;
}

void nanny_create_guest (DESCRIPTOR_DATA *d, char *argument)
{
	CHAR_DATA	*ch = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;

	if ( d->account && d->account->name && *d->account->name ) {
		free_account (d->account);
		d->account = NULL;
	}

	if ( d->character &&
		IS_SET (d->character->flags, FLAG_GUEST) &&
		d->character->tname &&
		*d->character->tname &&
		*argument && !str_cmp (argument, "recreate") ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s", d->character->tname);
		
		extract_char (d->character);
		create_guest_avatar (d);
	}
	else{
		create_guest_avatar (d);
	}		
	

	ch = d->character;

	ch->color = d->color;

	/*  If we're recreating, we're either recovering from a reboot or returning a dead guest to the lounge, in which case we can skip a lot of this???. */

	pc_to_game (ch);

	char_to_room (ch, OOC_LOUNGE);

	if ( !str_cmp (argument, "recreate") ) {
		act ("$n is incarnated in a soft glimmer of light.", TRUE, d->character, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		
		temp_arg = char_short(d->character);
		snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0 has logged in as %s, from %s.", temp_arg, ch->tname, ch->desc->host);
		buf[2] = toupper(buf[2]);
		send_to_gods (buf);
		d->connected = CON_PLYNG;
		guest_conns++;
		
		temp_arg = char_short (d->character);
		snprintf (buf, MAX_STRING_LENGTH,  "%s has logged in from %s as %s.", temp_arg, d->host, d->character->tname);
		*buf = toupper(*buf);
		system_log (buf, FALSE);
       	       	mysql_safe_query ("UPDATE newsletter_stats SET guest_logins=guest_logins+1");
		do_look (ch, "", 0);
	}
	else {
		if ( finished_booting )	{	/*  Dead Guest being returned to the lounge. */
			act ("$n appears in a sudden glimmer of light, looking slightly dazed.", TRUE, ch, 0, 0, TO_ROOM | TO_ACT_FORMAT);
		}
		else {
                act ("You feel your form briefly waver before it solidifies into yet another new guise, returned safely to the pleasant confines of Club Endore.", FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
		}
	}
	
	return;
}

void nanny_guest_arrive (DESCRIPTOR_DATA *d, char *argument)
{
	return;
}

void nanny_ask_password (DESCRIPTOR_DATA *d, char *argument)
{
	SITE_INFO	*site = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			is_banned = 0;
	int			i = 0;
	
	if ( d->account != NULL )
		free_account (d->account);

	d->account = NULL;

	if ( !*argument ) {
		close_socket (d);
		return;
	}

	if ( strstr (argument, " ") ) {
		SEND_TO_Q ("\nThe account name cannot contain whitespace.\n", d);
		menu_choice (d);
		return;
	}

	if ( !isalpha (*argument) ) {
		SEND_TO_Q ("\nThe first character of the account name MUST be a letter.\n", d);
		menu_choice (d);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "%s", argument);

	for ( i = 0; i < strlen (buf); i++ ) {
		if ( !isalpha (buf [i]) && !isdigit (buf [i]) ) {
			SEND_TO_Q ("\nIllegal characters in account name (letters/numbers only).\n", d);
			menu_choice (d);
			return;
		}
	}

	if ( str_cmp (CAP(argument), "Anonymous") )
		d->account = load_account (argument);

	if ( !str_cmp (argument, "Anonymous") || !d->account ) {
		SEND_TO_Q ("\nNo such account. If you wish to create a new account,\n"
		           "please choose option 'C' from the main menu.\n", d);
		menu_choice (d);
		return;
	}

	if ( banned_site && banned_site->name ) {
		for ( site = banned_site; site; site = site->next ) {
			if ( !str_cmp (banned_site->name, "*") ) {
				is_banned++;
				break;
			}
			
			if ( strstr (d->host, site->name) ) {
				is_banned++;
				break;
			}
		}
	}
                
        if ( is_banned && !IS_SET (d->account->flags, ACCOUNT_NOBAN) ) {   
                snprintf (buf, MAX_STRING_LENGTH,  "\nYour site is currently banned from connecting to %s.\n"
                              "As a result of the large number of people we get from certain Internet\n"
                              "domains, this may not be a result of anything you've done; if you feel that\n"
                              "this is the case, please email our staff at %s and we\n"
                              "will do our best to resolve this issue. Our apologies for the inconvenience.\n", MUD_NAME, STAFF_EMAIL);
                SEND_TO_Q (buf, d);
                snprintf (buf, MAX_STRING_LENGTH,  "\nPlease press ENTER to disconnect from the server.\n");
                SEND_TO_Q (buf, d);
                d->connected = CON_PENDING_DISC;
		return;
        }     

	if ( str_cmp (d->account->last_ip, d->host) )
		SEND_TO_Q ("\r\nPlease enter your password carefully - your access to the game server will be\n"
			   "suspended for a one-hour period if you repeatedly fail your logins!\r\n\r\n", d);

	SEND_TO_Q ("Password: ", d);

	QECHO_OFF;

	d->connected = CON_PWDCHK;
	
	return;

}

/*                                                                          *
 * function: nanny_check_password        < e.g.> Password: *********        *
 *                                                                          *
 *                                                                          */
void nanny_check_password (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		buf3 [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES	*result = NULL;
	int         nFailedLogins = 0;
	int			nSharedIP = 0;

	if ( !*argument ) {
		close_socket (d);
		return;
	}

	if ( !check_password (argument, d->account->pwd) ) {
		if ( str_cmp (d->account->last_ip, d->host) ) {
			mysql_safe_query ("INSERT INTO failed_logins VALUES ('%s', '%s', UNIX_TIMESTAMP())", d->account->name, d->host);
			mysql_safe_query ("SELECT * FROM failed_logins WHERE hostname = '%s' AND timestamp >= (UNIX_TIMESTAMP() - 60*60)", d->host);
			if ( ( result = mysql_store_result ( database ) ) != NULL ) {
			  nFailedLogins = mysql_num_rows( result );
			  mysql_free_result ( result ); result = NULL;
			}
			else {
			  snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_check_password(): %s", mysql_error ( database ) );
			  system_log( buf, TRUE );
			  close_socket( d ); /* something bad happened during login... disconnect them (JWW) */
			  return;
			}

			if ( nFailedLogins >= 3 && !IS_SET (d->account->flags, ACCOUNT_NOBAN) ) {
				if ( d->account && d->account->admin ) {
					snprintf (buf, MAX_STRING_LENGTH,  "Staff Security Notificiation [%s]", d->account->name); 
					snprintf (buf2, MAX_STRING_LENGTH,  "The following host, %s, has been temporarily banned from the MUD server for incorrectly failing to log into your game account three times within one hour.\n\nUse the BAN command in-game to see which player accounts, if any, are affected by the ban; this should help you track down the perpetrator.", d->host);
					send_email (d->account, STAFF_EMAIL, "Account Security <"STAFF_EMAIL">", buf, buf2);
				}
				else if ( d->account ) {
					snprintf (buf, MAX_STRING_LENGTH,  "Account Security Notification"); 
					snprintf (buf2, MAX_STRING_LENGTH,  "Hello,\n\nA user has just been temporarily sitebanned from the MUD server for three failed login attempts to your game account, %s, originating from a foreign IP and occurring within the past hour.\n\nIf this individual was not you, please notify the staff by replying directly to this message; someone may be attempting to access your account illegally. Otherwise, you will regain your access privileges automatically one hour from the time of this email notification.\n\nThank you for playing!\n\n\n\nWarmest Regards,\nThe Admin Team", d->account->name);
					snprintf (buf3, MAX_STRING_LENGTH,  "Account Security <%s>", STAFF_EMAIL);
					send_email (d->account, NULL, buf3, buf, buf2);
				}
				SEND_TO_Q ("\n\nYour access to this server has been suspended for one hour due to repeated\n"
					   "incorrect password attempts. Please email the staff if you have any questions.\n\n"
					   "Additionally, the registered owner of this account has been notified via email.\n\n", d);
				ban_host (d->host, "Password Security System", -2);
				d->connected = CON_PENDING_DISC;
				return;
			}
		}
		SEND_TO_Q ("\n\nIncorrect password - have you forgotten it? Visit here to obtain a new one:\n\nhttp://www.yourmud.org/forums/profile.php?mode=sendpassword\n\nPassword: ", d);
		d->account->pwd_attempt++;
		save_account(d->account);
		return;
	}

	QECHO_ON;

	d->color = d->account->color;

        if ( !strstr (d->host, "www.yourmud.org") ) {
	  mysql_safe_query ("SELECT username FROM forum_users WHERE user_last_ip != '(null)' AND user_last_ip = '%s' AND username != '%s'", d->account->last_ip, d->account->name);
	  if ( ( result = mysql_store_result ( database ) ) != NULL ) {
	    nSharedIP = mysql_num_rows ( result );
	    mysql_free_result ( result ); result = NULL;
	  }
	  else {
	    snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_check_password(): %s", mysql_error ( database ) );
	    system_log( buf, TRUE );
	  }
        }
        
	if ( !IS_SET (d->account->flags, ACCOUNT_IPSHARER) ) {                                                                                                                                                     
	        snprintf (buf, MAX_STRING_LENGTH,  "%s [%s] has logged in.%s\n", d->account->name, d->host, (nSharedIP) ? " #1Possible IP sharing detected.#0" : "");
        	send_to_gods (buf);
        	snprintf (buf, MAX_STRING_LENGTH,  "%s [%s] has logged in.%s", d->account->name, d->host, (nSharedIP) ? " Possible IP sharing detected." : "");
        	system_log(buf, FALSE);
	}

	if ( d->account->last_ip && *d->account->last_ip )
	  mem_free (d->account->last_ip);
	d->account->last_ip = str_dup (d->host);

	save_account (d->account);

	d->account->pwd_attempt = 0;

	display_main_menu (d);
	
	return;
}

void nanny_new_account (DESCRIPTOR_DATA *d, char *argument)
{
	ACCOUNT_DATA	*account = NULL;
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	char		buf2[MAX_STRING_LENGTH] = {'\0'};
	int			i = 0;

	snprintf (buf, MAX_STRING_LENGTH, "%s", argument);

	if ( !*buf ) {
		close_socket (d);
		return;
	}

	if ( strlen (buf) > 36 ) {
		SEND_TO_Q ("\nPlease choose an account name of less than 36 characters.\n", d);
		menu_choice (d);
		return;
	}

	if ( strstr (buf, " ") ) {
		SEND_TO_Q ("\nThe account name cannot contain whitespace.\n", d);
		menu_choice (d);
		return;
	}

	if ( !isalpha (*buf) ) {
		SEND_TO_Q ("\nThe first character of the account name MUST be a letter.\n", d);
		menu_choice (d);
		return;
	}

	for ( i = 0; i < strlen (buf); i++ ) {
		if ( !isalpha (buf [i]) && !isdigit (buf [i]) ) {
			SEND_TO_Q ("\nIllegal characters in account name (letters/numbers only). Please hit ENTER.\n", d);
			menu_choice (d);
		return;
		}

		if ( i )
			buf [i] = tolower (buf [i]);
		else
			buf [i] = toupper (buf [i]);
	}

	if ( str_cmp (CAP(argument), "Anonymous") && !(account = load_account(CAP(argument))) ) {
		snprintf (buf2, MAX_STRING_LENGTH,  "\nApply for a login account named %s? [y/n]  ", buf);
		SEND_TO_Q (buf2, d);
		d->connected = CON_ACCT_POLICIES;
		d->stored = str_dup(buf);
		return;
	}
	else {
		SEND_TO_Q ("\nThat account name has already been taken.\n", d);
		menu_choice (d);
		return;
	}
	return;
}

void nanny_account_policies (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	argument = one_argument (argument, buf);

	*buf = toupper (*buf);

	if ( !*buf ) {
		close_socket (d);
		return;
	}

	if ( *buf == 'Y' ) {
		CREATE (d->account, ACCOUNT_DATA, 1);
		d->account->name = str_dup (d->stored);
		d->stored = str_dup ("");
		d->account->roleplay_points = 0;
		d->account->created_on = time(0);
		temp_arg = get_text_buffer (NULL, text_list, "account_policies");
		SEND_TO_Q (temp_arg, d);
		SEND_TO_Q ("Do you agree? (y/n) ", d);
		d->connected = CON_REFERRAL;
		return;
	}
	else {
		menu_choice (d);
		return;
	}
	return;
}

void nanny_account_referral (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	argument = one_argument (argument, buf);

	*buf = toupper (*buf);

	if ( !*buf ) {
		close_socket (d);
		return;
	}

	if ( *buf == 'Y' ) {
		temp_arg = get_text_buffer (NULL, text_list, "account_referral");
		SEND_TO_Q (temp_arg, d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_ACCT_EMAIL;
		return;
	}
	else {
		menu_choice (d);
		return;
	}
	
	return;
}

void nanny_account_email (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	char		referrer [15];
	char		*temp_arg = NULL;
	
	argument = one_argument (argument, buf);

	*buf = toupper (*buf);

	if ( !*buf ) {
		close_socket (d);
		return;
	}

	if ( isdigit (*buf) && (strtol(buf, NULL, 10) == 1 || strtol(buf, NULL, 10) == 2 || strtol(buf, NULL, 10) == 3 || strtol(buf, NULL, 10) == 4 || strtol(buf, NULL, 10) == 5 || strtol(buf, NULL, 10) == 6 || strtol(buf, NULL, 10) == 7) ) {

		if ( strtol(buf, NULL, 10) == 1 )
			snprintf (referrer, MAX_STRING_LENGTH, "wm");
		else if ( strtol(buf, NULL, 10) == 2 )
			snprintf (referrer, MAX_STRING_LENGTH, "aff");
		else if ( strtol(buf, NULL, 10) == 3 )
			snprintf (referrer, MAX_STRING_LENGTH, "mc");
		else if ( strtol(buf, NULL, 10) == 4 )
			snprintf (referrer, MAX_STRING_LENGTH, "mm");
		else if ( strtol(buf, NULL, 10) == 5 )
			snprintf (referrer, MAX_STRING_LENGTH, "tms");
		else if ( strtol(buf, NULL, 10) == 6 )
			snprintf (referrer, MAX_STRING_LENGTH, "other_tolk");
		else if ( strtol(buf, NULL, 10) == 7 )
			snprintf (referrer, MAX_STRING_LENGTH, "other_mud");
		else
			snprintf (referrer, MAX_STRING_LENGTH, "other");

		mysql_safe_query ("INSERT INTO account_referrals VALUES ('%s', '%s', UNIX_TIMESTAMP())", d->account->name, referrer);

		temp_arg = get_text_buffer (NULL, text_list, "account_email");
		SEND_TO_Q (temp_arg, d);
		SEND_TO_Q ("Your email address: ", d);
		d->connected = CON_EMAIL_CONFIRM;
		return;
	}
	else {
		temp_arg = get_text_buffer (NULL, text_list, "account_referral");
		SEND_TO_Q (temp_arg, d);
		SEND_TO_Q ("Please choose an option between 1 and 7.\n\nYour Selection: ", d);
		return;
	}
}

/*                                                                          *
 * function: nanny_account_email_confirm                                    *
 *                                  < e.g.> Your email address: null@bar.fu *
 *                                                                          *
 *                                                                          */
void nanny_account_email_confirm (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES	*result = NULL;
	int			nUserEmailMatches = 0;

	if ( !*argument ) {
		close_socket (d);
		return;
	}

	if ( !strstr (argument, "@") || !strstr (argument, ".") ) {
		SEND_TO_Q ("\nYour listed email address must include an '@' symbol and a dot.\n\n", d);
		SEND_TO_Q ("Your email address: ", d);
		d->connected = CON_EMAIL_CONFIRM;
		return;
	}

	mysql_safe_query ("SELECT user_email FROM forum_users WHERE user_email = '%s' AND username != '%s'", argument, d->account->name);
        if ( ( result = mysql_store_result ( database ) ) != NULL ) {
          nUserEmailMatches = mysql_num_rows ( result ) ;
          mysql_free_result ( result ) ; result = NULL ;
	}
        else {
	  snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_account_email_confirm(): %s", mysql_error ( database ) );
	  system_log( buf, TRUE );
	}

	if ( nUserEmailMatches > 0 ) {
	  SEND_TO_Q ("\nWe're sorry, but that email address has already been registered\n"
		     "under an existing game  account. Please choose another.\n\n", d);
	  SEND_TO_Q ("Your email address: ", d);
	  d->connected = CON_EMAIL_CONFIRM;
	  return;
	}
        
	snprintf (buf, MAX_STRING_LENGTH,  "Is the address %s correct? [y/n] ", argument);
	SEND_TO_Q (buf, d);
	d->account->email = str_dup(argument);
	d->connected = CON_ACCOUNT_SETUP;
	mysql_free_result (result);
	return;
}

void nanny_new_password (DESCRIPTOR_DATA *d, char *argument)
{       
	char			*password;


	if ( !*argument || strlen(argument) < 6 || strlen (argument) > 25 ) {
		SEND_TO_Q ("\n\nPasswords should be 6 to 25 characters in length.\n\n", d);
		SEND_TO_Q ("Password: ", d);
		return;
	}

	password = encrypt_buf (argument);
	d->account->pwd = str_dup(password);        

	SEND_TO_Q ("\nPlease retype your new password: ", d);

	d->connected = CON_PWDNCNF;

	return;
}

void nanny_change_password (DESCRIPTOR_DATA *d, char *argument)
{
        nanny_new_password (d, argument);

        d->connected = CON_PWDNCNF;
        
        return;
}

void nanny_conf_change_password (DESCRIPTOR_DATA *d, char *argument)
{
	if ( !check_password (argument, d->account->pwd) ) {
		SEND_TO_Q ("\n\nPasswords didn't match.\n\n", d);
		SEND_TO_Q ("Retype password: ", d);
		d->connected = CON_PWDNEW;
		return;
	}

	QECHO_ON;

	SEND_TO_Q ("\n\n#2Account password successfully modified.#0\n\n", d);

	display_main_menu (d);

	save_account (d->account);

	return;
}

void nanny_disconnect (DESCRIPTOR_DATA *d, char *argument)
{
	if ( d->character ) {
		extract_char (d->character);
		d->character = NULL;
	}

	if ( d->account != NULL && d->account->name != NULL ) {
		free_account (d->account);
		d->account = NULL;
	}

	close_socket (d);
	
	return;
}

void setup_new_account (ACCOUNT_DATA *account)
{
	char		*encrypted = NULL;
	char		*password = NULL;
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	char		buf2[MAX_STRING_LENGTH] = {'\0'};
	char		email[MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	temp_arg = generate_password(1, (char **)"8");
	password = str_dup(temp_arg);

	snprintf (buf, MAX_STRING_LENGTH,  "Greetings,\n"
		      "\n"
	              "Thank you for your interest in our community! This\n"
	              "was sent to inform you that your login account has been\n"
	              "created on our server; please don't respond to this email,\n"
	              "as it is merely an automated system notification.\n"
		      "\n"
		      "MUD Account: %s\n"
		      "Password: %s\n"
		      "\n"
		      "Also, note that this username and account combination will allow\n"
		      "you to log into our web-based discussion forum, located at:\n\n"
                      "http://www.yourmud.org/forums/\n"
		      "\n"
		      "Feel free to hop on in and join in our discussions!\n"
      	              "\n"
                      "Best of luck, and again, welcome to %s.\n\n\n"
		      "                                 Warmest Regards,\n"
		      "                                 The Admin Team\n", account->name, password, MUD_NAME);

	encrypted = encrypt_buf(password);
	account->pwd = (char *)alloc ((int)sizeof(encrypted), 15);
	account->pwd = str_dup(encrypted);
	free (encrypted);

	account->newsletter = TRUE;

	snprintf (email, MAX_STRING_LENGTH, "%s <%s>", MUD_NAME, MUD_EMAIL);
	snprintf (buf2, MAX_STRING_LENGTH,  "Welcome to %s!", MUD_NAME);

	send_email (account, NULL, email, buf2, buf);

	save_account(account);
	
	return;
}

/*
* Use this function if you only wish to allow users to reset their passwords,
* rather than change them to a specified (potentially unsecure) pass.
*/

void generate_new_password (ACCOUNT_DATA *account)
{
	char		*encrypted = NULL;
	char		*password = NULL;
	char		buf[MAX_STRING_LENGTH] = {'\0'};
	char		email[MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	temp_arg = generate_password(1, (char **)"8");
	password = str_dup(temp_arg);

	snprintf (buf, MAX_STRING_LENGTH,  "Greetings,\n"
		      "\n"
	              "  As requested, here is the new login password for your\n"
		      "game login account. Please be sure to keep your password\n"
		      "secure; game staff will NEVER ask you to reveal it!\n"
		      "\n"
		      "New Password: %s\n"
		      "\n"
		      "\n"
		      "Warmest Regards,\n"
		      "The Admin Team\n", password);

	encrypted = encrypt_buf(password);
	account->pwd = str_dup(encrypted);

	snprintf (email, MAX_STRING_LENGTH, "%s <%s>", MUD_NAME, MUD_EMAIL);
	
	send_email (account, NULL, email, "Your New Account Password", buf);
	save_account(account);
}

void nanny_account_setup (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	if ( !*argument ) {
		close_socket (d);
		return;
	}

	CAP(argument);

	if ( *argument == 'Y' ) {
		temp_arg = get_text_buffer (NULL, text_list, "thankyou");
		SEND_TO_Q (temp_arg, d);
		
		d->account->last_ip = add_hash (d->host);
		setup_new_account (d->account);
		
		snprintf (buf, MAX_STRING_LENGTH,  "New account: %s [%s].\n", d->account->name, d->host);
		send_to_gods (buf);
		
		snprintf (buf, MAX_STRING_LENGTH,  "New account: %s [%s].", d->account->name, d->host);
		system_log(buf, FALSE);
		SEND_TO_Q ("Press ENTER to disconnect from the server.\n", d);
		d->connected = CON_PENDING_DISC;
		new_accounts++;
        mysql_safe_query ("UPDATE newsletter_stats SET new_accounts=new_accounts+1");
		return;
	}
	else {
		menu_choice (d);
		d->account = NULL;
		return;
	}
	return;
}

void nanny_name_confirm (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( *argument != 'y' && *argument != 'Y' ) {
		SEND_TO_Q ("What name would you like? ", d);
		d->connected = CON_NEW_ACCT_NAME;
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "New player %s [%s].", GET_NAME (d->character), d->host);
	system_log(buf, FALSE);
	strcat (buf, "\n");
	send_to_gods (buf);

	SEND_TO_Q ("Select a password: ", d);
	QECHO_OFF;

	d->connected = CON_LOGIN;
	
	return;
}


void common_message (CHAR_DATA *ch)
{
	int				i = 0;
	COMMON_DATA		*cd = NULL;

	i = ch->pc->common;

	for ( cd = common_list; cd; cd = cd->next )
		if ( !i )
			break;
		else
			i--;

	if ( !cd ) {
		ch->pc->common = 0;
		cd = common_list;
	}

	if ( cd ) {
		send_to_char ("\n", ch);
		send_to_char (cd->message, ch);
		ch->pc->common++;
	}
	return;
}

/*                                                                          *
 * function: post_retirement                                                *
 *                                                                          *
 *                                                                          */
void post_retirement (DESCRIPTOR_DATA *d)
{
	STORED_PC_DATA	*pc = NULL;
	DESCRIPTOR_DATA	*td = NULL;
	CHAR_DATA		*tch = NULL;
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
    char			*date = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	date = timestr(date);
	
	if ( !*d->pending_message->message ) {
		SEND_TO_Q ("\n#2Character retirement aborted.#0\n", d);
		display_main_menu(d);
		return;
	}

	mysql_safe_query ("SELECT name,account FROM %s.pfiles WHERE account = '%s' AND create_state = 2 AND level = 0", PFILES_DATABASE, d->account->name);
	if ( ( result = mysql_store_result (database) ) == NULL ) {
	  snprintf (buf, MAX_STRING_LENGTH, "Warning: post_retirement(): %s", mysql_error ( database));
	  system_log( buf, TRUE );
	  SEND_TO_Q ("\n#2An error occurred. Character retirement aborted.#0\n", d);
	  display_main_menu(d);
	  return;
	}

	while ( (row = mysql_fetch_row(result)) ) {
		mysql_safe_query ("UPDATE %s.pfiles SET create_state=4 WHERE name = '%s'", PFILES_DATABASE, row[0]);
		add_message (row[0], 3, d->account->name, date, "Retired.", "", d->pending_message->message, 0);
		add_message ("Retirements", 2, row[1], date, row[0], "", d->pending_message->message, 0); 
		for ( pc = d->account->pc; pc; pc = pc->next ) {
			if ( !str_cmp (pc->name, row[0]) )
				pc->state = 4;
		}
		for ( td = descriptor_list; td; td = td->next ) {
			if ( !td->account )
				continue;
			if ( !td->account->pc )
				continue;
			for ( pc = td->account->pc; pc; pc = pc->next ) {
				if ( !str_cmp (pc->name, row[0]) )
					pc->state = 4;
			}	
		}
		if ( (tch = load_pc (row[0])) ) {
			death_email (tch);
			unload_pc (tch);
		}
	}
	mysql_free_result ( result ) ; result = NULL ;

	SEND_TO_Q ("\n#2Character retired successfully; you may create a new one now.#0\n", d);
	display_main_menu(d);

	mem_free (date);
	
	return;
}

/*                                                                          *
 * function: nanny_retire                                                   *
 *                                                                          *
 *                                                                          */
void nanny_retire (DESCRIPTOR_DATA *d, char *argument)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	QECHO_ON;

	if ( !*argument ) {
		SEND_TO_Q ("\n#2Character retirement aborted.#0\n", d);
		display_main_menu (d);
		return;
	}

	if ( !check_password (argument, d->account->pwd) ) {
		SEND_TO_Q ("\n#2Incorrect password. Character retirement aborted.#0\n", d);
		display_main_menu (d);
		return;
	}

	mysql_safe_query ("SELECT name,account FROM %s.pfiles WHERE account = '%s' AND create_state = 2", PFILES_DATABASE, d->account->name);
	if ( ( result = mysql_store_result ( database ) ) == NULL ) {
	  snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_retire(): %s", mysql_error ( database ) );
	  system_log( buf, TRUE );
	  SEND_TO_Q ("\n#2An error occurred. Character retirement aborted.#0\n", d);
	  display_main_menu(d);
	  return;
	}

	if ( !mysql_num_rows ( result ) ) {
		SEND_TO_Q ("\n#2You do not currently have a character to retire.#0\n", d);
		display_main_menu (d);
		return;
	}

	while ( (row = mysql_fetch_row(result)) ) {
		if ( get_pc (row[0]) ) {
			SEND_TO_Q ("\n#2You may not retire a character while s/he is logged in.#0\n", d);
			display_main_menu (d);
			return;
		}
	}
	mysql_free_result (result); result = NULL;

	SEND_TO_Q ("\n#2Please document thoroughly the reasoning behind your character retirement\n"
           	   "below; when finished, terminate the editor with an '@' symbol.#0\n\n", d);


        CREATE (d->pending_message, MESSAGE_DATA, 1);
        
        d->str = &d->pending_message->message;
        d->max_str = MAX_STRING_LENGTH;

	d->proc = post_retirement;
	
	return;
}

/*                                                                          *
 * function: nanny_terminate                                                *
 *                                                                          *
 *                                                                          */
void nanny_terminate (DESCRIPTOR_DATA *d, char *argument)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int			id = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	QECHO_ON;

	if ( !*argument ) {
		SEND_TO_Q ("\n#2Account termination aborted.#0\n", d);
		display_main_menu (d);
		return;
	}

	if ( !check_password (argument, d->account->pwd) ) {
		SEND_TO_Q ("\n#2Incorrect password. Account termination aborted.#0\n", d);
		display_main_menu (d);
		return;
	}

	mysql_safe_query ("SELECT user_id FROM forum_users WHERE username = '%s'", d->account->name);
	if ( ( result = mysql_store_result ( database) ) == NULL ) {
	  snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_terminate(): %s", mysql_error ( database ) );
	  system_log( buf, TRUE );
	  SEND_TO_Q ("\n#2An error occurred. Account termination aborted.#0\n", d);
	  display_main_menu(d);
	  return;
	}

	row = mysql_fetch_row (result);
	id = strtol(row[0], NULL, 10);
	mysql_free_result (result); result = NULL;

	mysql_safe_query ("SELECT name FROM %s.pfiles WHERE account = '%s'", PFILES_DATABASE, d->account->name);
	if ( ( result = mysql_store_result ( database) ) == NULL ) {
	  snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_terminate(): %s", mysql_error ( database ) );
	  system_log( buf, TRUE );
	  SEND_TO_Q ("\n#2An error occurred. Account termination aborted.#0\n", d);
	  display_main_menu(d);
	  return;
	}

	while ( (row = mysql_fetch_row(result)) ) {
	  mysql_safe_query ("DELETE FROM player_notes WHERE name = '%s'", row[0]);
	}
	mysql_free_result (result); result = NULL;

	mysql_safe_query ("DELETE FROM forum_user_group WHERE user_id = %d", id);
	mysql_safe_query ("DELETE FROM hobbitmail WHERE account = '%s'", d->account->name);
	mysql_safe_query ("DELETE FROM registered_characters WHERE account_name = '%s'", d->account->name);
	mysql_safe_query ("DELETE FROM forum_users WHERE username = '%s'", d->account->name);
	mysql_safe_query ("DELETE FROM %s.pfiles WHERE account = '%s'", PFILES_DATABASE, d->account->name);
	
	snprintf (buf, MAX_STRING_LENGTH,  "Account %s has been terminated.", d->account->name);
	send_to_gods (buf);

	SEND_TO_Q ("\n#2Account termination successful. Goodbye.#0\n", d);
	snprintf (buf, MAX_STRING_LENGTH,  "\nPlease press ENTER to disconnect from the server.\n");
	SEND_TO_Q (buf, d);
	d->connected = CON_PENDING_DISC;
	return;

}

/*                                                                          *
 * function: display_hobbitmail_inbox                                       *
 *                                                                          *
 *                                                                          */
void display_hobbitmail_inbox (DESCRIPTOR_DATA *d, ACCOUNT_DATA *account)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		imm_buf [MAX_STRING_LENGTH] = {'\0'};
	int		i;

	snprintf (buf, MAX_STRING_LENGTH,  "                              #6Hobbit-Mail: Main Menu#0\n"
		      "                              #6----------------------#0\n\n");

	if ( !account || !account->name ) {
                snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "                  #2There are currently no messages in your inbox.#0\n");
		SEND_TO_Q (buf, d);
		return;
	}

	mysql_safe_query ("SELECT * FROM hobbitmail WHERE account = '%s' ORDER BY timestamp DESC", account->name);

	if ( ( result = mysql_store_result ( database ) ) == NULL ) {
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "                  #2There are currently no messages in your inbox.#0\n");
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\nEnter message number to display, \"new\", or \"exit\": ");
	  	SEND_TO_Q (buf, d);
	  	snprintf (buf, MAX_STRING_LENGTH, "Warning: display_hobbitmail_inbox(): %s", mysql_error ( database ) );
	  	system_log( buf, TRUE );
	  	return;
	}

	if ( mysql_num_rows(result) == 0 ) {
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "                  #2There are currently no messages in your inbox.#0\n");
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\nEnter message number to display, \"new\", or \"exit\": ");
	  	SEND_TO_Q (buf, d);
	}
	else {
	  	i = 0;
	  	while ( (row = mysql_fetch_row(result)) ) {
	    		i++;
			if ( i > 100 ) {
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n...remaining messages truncated. Delete current messages to see them.\n");
				break;
			}
	    		if ( d->account && d->account->admin == TRUE )
	      			snprintf (imm_buf, MAX_STRING_LENGTH, " (%s)", row[3]);
	    		else *imm_buf = '\0';
	    		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%d. #2From:#0 %s%s, #2Re:#0 %s %s %s\n", i, row[2], d->account->admin ? imm_buf : "", row[5],
		     		!IS_SET (strtol(row[1], NULL, 10), MF_READ) ? "#6(unread)#0" : "", IS_SET (strtol(row[1], NULL, 10), MF_REPLIED) ? "#5(replied)#0" : "" );
	  	}
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\nEnter message number to display, \"new\", or \"exit\": ");
	  	page_string (d, buf);
	}

	mysql_free_result (result); result = NULL;

	return;
}

void nanny_composing_message (DESCRIPTOR_DATA *d, char *argument)
{
	DESCRIPTOR_DATA		*td = NULL;
	MUDMAIL_DATA		*message = NULL;
	ACCOUNT_DATA		*account = NULL;
	char				*date = NULL;

	date = timestr(date);
	
	CREATE (message, MUDMAIL_DATA, 1);
	message->from = str_dup(d->pending_message->poster);
	message->subject = str_dup(d->pending_message->subject);
	message->message = str_dup(d->pending_message->message);
	message->from_account = str_dup(d->account->name);
	message->date = str_dup (date);
	message->flags = 0;

	account = load_account (d->stored);
	
	save_hobbitmail_message (account, message);

	mem_free (message->from);
	mem_free (message->subject);
	mem_free (message->message);
	mem_free (message->from_account);
	mem_free (message->date);
	mem_free (message);

	unload_message (d->pending_message);
	d->pending_message = NULL;

	if ( !account ) {
		SEND_TO_Q ("#1Your message was not delivered; there was an error.#0\n\n", d);
	}
	else {
		SEND_TO_Q ("#2Thanks! Your Hobbit-Mail has been delivered to the specified account.#0\n\n", d);
		for ( td = descriptor_list; td; td = td->next ) {
			if ( !td->account || !td->account->name || !td->character || td->connected != CON_PLYNG )
				continue;
			if ( !str_cmp (td->account->name, account->name) )
				SEND_TO_Q ("#6\nA new Hobbit-Mail has arrived for your account!#0\n", td);
		}
	}

	if ( account != NULL )
		free_account (account);

	d->stored = str_dup ("");

	display_hobbitmail_inbox (d, d->account);

	d->connected = CON_MAIL_MENU;

	return;
}

void nanny_compose_message (DESCRIPTOR_DATA *d, char *argument)
{

	if ( !*argument ) {
		SEND_TO_Q ("\n", d);

		display_hobbitmail_inbox (d, d->account);

		d->connected = CON_MAIL_MENU;
		return;
	}

	if ( isdigit (*argument) ) {
		SEND_TO_Q ("\nRegarding? ", d);
		return;
	}

	d->pending_message->subject = add_hash(argument);

	SEND_TO_Q ("\n#2Enter message; terminate with an '@' when completed. Once finished,\n", d);
	SEND_TO_Q ("hit ENTER again to send and return to the main Hobbit-Mail menu.#0\n\n", d);

	d->pending_message->message = NULL;
	d->str = &d->pending_message->message;
	d->max_str = MAX_STRING_LENGTH;
	d->connected = CON_COMPOSING_MESSAGE;
	
	return;
}

void nanny_compose_subject (DESCRIPTOR_DATA *d, char *argument)
{
	if ( !*argument ) {

		SEND_TO_Q ("\n", d);

		display_hobbitmail_inbox (d, d->account);

		d->connected = CON_MAIL_MENU;
		return;
	}

	if ( isdigit (*argument) ) {
		SEND_TO_Q ("\nAs whom do you wish to send the message? ", d);
		return;
	}

	CREATE (d->pending_message, MESSAGE_DATA, 1);
	d->pending_message->poster = add_hash (argument);
	d->connected = CON_COMPOSE_MESSAGE;
	SEND_TO_Q ("\nRegarding? ", d);
	return;
}

/*                                                                          *
 * function: nanny_compose_mail_to                                          *
 *                                                                          *
 *                                                                          */
void nanny_compose_mail_to (DESCRIPTOR_DATA *d, char *argument)
{
	CHAR_DATA		*tch = NULL;
	ACCOUNT_DATA	*account = NULL;	
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !*argument ) {
		SEND_TO_Q ("\n", d);

		display_hobbitmail_inbox (d, d->account);

		d->connected = CON_MAIL_MENU;
		return;
	}

	if ( !(tch = load_pc (argument)) ) {
                mysql_safe_query ("SELECT name FROM %s.pfiles WHERE keywords LIKE \'%%%s%%\' LIMIT 1", PFILES_DATABASE, argument);
                if ( ( result = mysql_store_result (database) ) != NULL ) {
			if ( mysql_num_rows(result) > 0 ) {
		    		row = mysql_fetch_row(result);
		    		tch = load_pc (row[0]);
		  	}
		  	mysql_free_result (result); result = NULL;
		}
		else {
			snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_compose_mail_to(): %s", mysql_error ( database ) );
		  	system_log( buf, TRUE );
		}

		if ( !tch ) {
			SEND_TO_Q ("#1\nI am sorry, but that PC could not be found.#0\n", d);
			SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
			return;
		}

	}

	account = NULL;
	account = load_account (tch->pc->account);

	if ( !account ) {
		SEND_TO_Q ("#1\nThere seems to be a problem with that PC's account.#0\n", d);
		SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
		unload_pc (tch);
		return;
	}

	mysql_safe_query ("SELECT COUNT(*) FROM hobbitmail WHERE account = '%s'", account->name);

        if ( ( result = mysql_store_result (database) ) != NULL ) {
		row = mysql_fetch_row(result);
		if ( strtol(row[0], NULL, 10) >= 100 ) {
			SEND_TO_Q ("#1\nSorry, but that person's mailbox is currently full.#0\n", d);
			SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
			mysql_free_result (result); result = NULL;
			return;
		}
		mysql_free_result (result); result = NULL;
	}
	else {
		snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_compose_mail_to(): %s", mysql_error ( database ) );
	  	system_log( buf, TRUE );
	}

	if ( !str_cmp (account->name, "Guest") ) {
		SEND_TO_Q ("#1\nSorry, but Hobbit-Mail cannot be sent to the guest account.#0\n", d);
		SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
		unload_pc (tch);
		return;
	}

	d->stored = str_dup (account->name);

	unload_pc (tch);

	free_account (account);

	SEND_TO_Q ("\nAs whom do you wish to send the message? ", d);
	d->connected = CON_COMPOSE_SUBJECT;
	
	return;
}

/*                                                                          *
 * function: nanny_mail_menu                                                *
 *                                                                          *
 *                                                                          */
void nanny_mail_menu (DESCRIPTOR_DATA *d, char *argument)
{
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
	int				i = 1;
	char			imm_buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !argument || !*argument || (!isdigit (*argument) && cmp_strn (argument, "new", strlen(argument)) && cmp_strn (argument, "exit", strlen(argument))) ) {
		SEND_TO_Q("Enter message number to display, \"new\", or \"exit\": ", d);
		return;
	}

	if ( !cmp_strn (argument, "exit", strlen(argument)) ) {
		display_main_menu (d);
		return;
	}

	if ( !cmp_strn (argument, "new", strlen(argument)) ) {
		d->connected = CON_COMPOSE_MAIL_TO;
		SEND_TO_Q ("\nTo which PC's player do you wish to send a message? ", d);
		return;
	}

	if ( isdigit (*argument) ) {
		mysql_safe_query ("SELECT * FROM hobbitmail WHERE account = '%s' ORDER BY timestamp DESC", d->account->name);

		if ( ( result = mysql_store_result (database) ) == NULL ) {
			snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_mail_menu(): %s", mysql_error ( database ) );
		  	system_log( buf, TRUE );
		  	SEND_TO_Q ("\n#1I am sorry, but that message could not be found.#0\n", d);
		  	SEND_TO_Q("\nEnter message number to display, \"new\", or \"exit\": ", d);
		  	return;
		}

		if ( !mysql_num_rows (result) ) {
			SEND_TO_Q ("\n#1I am sorry, but that message could not be found.#0\n", d);
			SEND_TO_Q("\nEnter message number to display, \"new\", or \"exit\": ", d);
			return;
		}

		while ( (row = mysql_fetch_row(result)) ) {
			if ( i == strtol(argument, NULL, 10))
				break;
			if ( i >= 100 ) {
				row = NULL;
				break;
			}
			i++;
		}
			
		if ( !row ) {
			mysql_free_result (result); result = NULL;
			SEND_TO_Q ("\n#1I am sorry, but that message could not be found.#0\n", d);
			SEND_TO_Q("\nEnter message number to display, \"new\", or \"exit\": ", d);
			return;
		}

		*imm_buf = '\0';

		if ( d->account->admin )
			snprintf (imm_buf, MAX_STRING_LENGTH, " (%s)", row[3]);

		snprintf (buf, MAX_STRING_LENGTH,  "\n#2Subject:#0 %s\n#2From:#0 %s%s\n#2Date:#0 %s\n\n%s\n", row[5], row[2], d->account->admin ? imm_buf : "", row[4], row[6]);
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "Enter \"delete\", \"reply\", or \"exit\": ");

		set_hobbitmail_flags (strtol(row[8], NULL, 10), MF_READ);

		SEND_TO_Q (buf, d);

		d->stored = (char *)strtol(row[8], NULL, 10);

		d->connected = CON_READ_MESSAGE;

		mysql_free_result (result); result = NULL;

	}
	return;
}

/*                                                                          *
 * function: nanny_read_message                                             *
 *                                                                          *
 *                                                                          */
void nanny_read_message (DESCRIPTOR_DATA *d, char *argument)
{
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !*argument || isdigit (*argument) ) {
		SEND_TO_Q ("Enter \"delete\", \"reply\", or \"exit\": ", d);
		d->connected = CON_READ_MESSAGE;
		return;
	}

	if ( cmp_strn (argument, "delete", strlen(argument)) && cmp_strn (argument, "reply", strlen(argument)) && cmp_strn (argument, "exit", strlen(argument)) ) {
		SEND_TO_Q ("Enter \"delete\", \"reply\", or \"exit\": ", d);
		d->connected = CON_READ_MESSAGE;
		return;
	}

	if ( !cmp_strn (argument, "reply", strlen(argument)) ) {
		set_hobbitmail_flags ((int) d->stored, MF_READ | MF_REPLIED);
		mysql_safe_query ("SELECT from_account FROM hobbitmail WHERE account = \'%s' AND id = %d", d->account->name, (int) d->stored);
		if ( ( result = mysql_store_result (database) ) == NULL ) {
		  snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_read_message(): %s", mysql_error ( database ) );
		  system_log( buf, TRUE );
		  SEND_TO_Q ("Enter \"delete\", \"reply\", or \"exit\": ", d);
		  d->connected = CON_READ_MESSAGE;
		  return;
		}
		row = mysql_fetch_row(result);
		d->stored = str_dup (row[0]);
		mysql_free_result ( result ) ; result = NULL ;
		d->connected = CON_COMPOSE_SUBJECT;
		SEND_TO_Q ("\nAs whom do you wish to send the reply? ", d);
		return;
	}

	if ( !cmp_strn (argument, "delete", strlen(argument)) ) {
		mysql_safe_query ("DELETE FROM hobbitmail WHERE account = '%s' AND id = %d", d->account->name, (int)d->stored);
		snprintf (buf, MAX_STRING_LENGTH,  "\n#1The specified Hobbit-Mail has been deleted from your account.#0\n\n");
		SEND_TO_Q (buf, d);

		display_hobbitmail_inbox (d, d->account);

		d->connected = CON_MAIL_MENU;
		return;
	}

	if ( !cmp_strn (argument, "exit", strlen(argument)) ) {

		SEND_TO_Q ("\n", d);

		display_hobbitmail_inbox (d, d->account);

		d->connected = CON_MAIL_MENU;
		return;
	}
	return;
}

void nanny_connect_select (DESCRIPTOR_DATA *d, char *argument)
{
	char			*pc_names[MAX_PC_LIMIT];
	STORED_PC_DATA	*pc = NULL;
	FILE			*fp = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			*temp_arg = NULL;
	int				tally = 0;
	int				block = 0;
	int				i = 0;
	char			c = '\0';

	if ( d->account->color && !d->color )
		d->color = 1;

	if ( !*argument) {
		display_main_menu (d);
		return;
	}

	while ( isspace ((c = tolower (*argument))) )
		argument++;



/*** Log in character ***/
	if ( c == 'e' || strtol(argument, NULL, 10) == 1 ) {
		if ( !d->account->pc ) {
			SEND_TO_Q ("There are currently no PCs registered under this account.\n", d);
			display_main_menu (d);
			return;
		}
		SEND_TO_Q ("\nWhich character would you like to log in?\n\n", d);
		for ( pc = d->account->pc; pc; pc = pc->next ) {
			pc_names[i] = add_hash(pc->name);
			i++;
			tally++;
		}
		for ( i = 0; i < tally; i++ ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%d. %s\n", i+1, pc_names[i]);
			mem_free (pc_names[i]);
			SEND_TO_Q(buf, d);
		}
		SEND_TO_Q ("\nYour Selection: ", d);
		d->connected = CON_CHOOSE_PC;
		return;
	}

/****** Guest login from account menu ****/
	else if ( c == 'g' || strtol(argument, NULL, 10) == 2 ) {
		d->connected = CON_CREATE_GUEST;
		nanny_create_guest (d, "recreate");
		return;
	}
	
/*** Create a new character ***/
	else if ( c == 'c' || strtol(argument, NULL, 10) == 3 ) {
		for ( pc = d->account->pc; pc; pc = pc->next ) {
			if ( pc->state != STATE_DIED )
				block++;
		}
		if ( (fp = fopen("application_lock", "r")) && !d->account->admin ) {
			SEND_TO_Q ("\n#2Sorry, but new character applications are not being accepted\n"
           			"at this time. Please try back later; thank you for your interest.#0\n", d);
			fclose(fp);
			display_main_menu (d);
			return;
		}
		if ( block && !d->account->admin ) {
			SEND_TO_Q ("\n#2Sorry, but you may only have one character submitted and/or in the\n"
                                   "game at any one time; you'll have to wait until your current character\n"
                                   "dies or is retired by the admins to create a new one.#0\n", d);
			display_main_menu (d);
			return;
		}
		d->character = new_char(1);
		clear_char (d->character);
		d->character->race = -1;
		d->character->desc = d;
		temp_arg = get_text_buffer (NULL, text_list, "help_name");
		SEND_TO_Q (temp_arg, d);
		SEND_TO_Q ("\nWhat would you like to name your new character? ", d);
		d->connected = CON_NAME_CONFIRM;
		return;
	}


/*** Delete pending applications ***/
	else if ( c == 'd' || strtol(argument, NULL, 10) == 4 ) {
		if ( !d->account->pc ) {
			SEND_TO_Q ("There are currently no PCs registered under this account.\n", d);
			display_main_menu (d);
			return;
		}
		SEND_TO_Q ("\nWhich character would you like to delete?\n\n", d);
		for ( pc = d->account->pc; pc; pc = pc->next ) {
			pc_names[i] = add_hash(pc->name);
			i++;
			tally++;
		}
		for ( i = 0; i < tally; i++ ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%d. %s\n", i+1, pc_names[i]);
			mem_free (pc_names[i]);
			SEND_TO_Q(buf, d);
		}
		SEND_TO_Q ("\nYour Selection: ", d);
		d->connected = CON_DELETE_PC;
		return;
	}

/*** retire character ***/
	else if ( c == 'r' || strtol(argument, NULL, 10) == 5 ) {
		if ( IS_SET (d->account->flags, ACCOUNT_NORETIRE) ) {
			SEND_TO_Q ("\n#1Your account has been flagged NORETIRE by an administrator, likely due\n"
				   "to abuse on your part of the retirement code. To retire your PC, you will\n"
				   "need to contact the administrative staff and ask permission.#0\n", d);
			display_main_menu (d);
			return;
		}
		SEND_TO_Q ("\nWhile we highly discourage people from lightly throwing away their characters, we\n"
		"also recognize that this is a game, and that there comes a point at which there is\n"
		"little fun to be had from playing a character. Please make this decision carefully;\n"
		"it is #1ABSOLUTELY IRREVOCABLE#0! To confirm, please entire your password below:\n\n"
		"Your Account Password: ", d);
		QECHO_OFF;
		d->connected = CON_RETIRE;
	}
	
/*** View Characters ***/
	else if ( c == 'v' || strtol(argument, NULL, 10) == 6 ) {
		if ( !d->account->pc ) {
			SEND_TO_Q ("There are currently no PCs registered under this account.\n", d);
			display_main_menu (d);
			return;
		}
		SEND_TO_Q ("\nWhich character would you like to view?\n\n", d);
		for ( pc = d->account->pc; pc; pc = pc->next ) {
			pc_names[i] = add_hash(pc->name);
			i++;
			tally++;
		}
		for ( i = 0; i < tally; i++ ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%d. %s\n", i+1, pc_names[i]);
			mem_free (pc_names[i]);
			SEND_TO_Q(buf, d);
		}
		SEND_TO_Q ("\nYour Selection: ", d);
		d->connected = CON_VIEW_PC;
		return;
	}

/*** Update email ***/
	else if ( strtol(argument, NULL, 10) == 7 ) {
		SEND_TO_Q ("\nYour registered email address: ", d);
		SEND_TO_Q (d->account->email, d);
		SEND_TO_Q ("\n", d);
		SEND_TO_Q ("\nEnter the desired email address: ", d);
		d->connected = CON_CHG_EMAIL;
		return;
	}

/*** Change password ***/
	else if ( strtol(argument, NULL, 10) == 8 ) {
		SEND_TO_Q ("Enter a new password: ", d);
		QECHO_OFF;
		d->connected = CON_PWDNEW;
		return;
	}

/*** Toggle ANSI colors ***/	
	else if ( strtol(argument, NULL, 10) == 9 ) {
		if ( d->color ) {
			d->color = 0;
			d->account->color = 0;
			save_account(d->account);
			SEND_TO_Q ("ANSI color disabled.\n", d);
			display_main_menu(d);
			return;
		}
		else {
			d->color = 1;
			d->account->color = 1;
			save_account(d->account);
			SEND_TO_Q ("ANSI color #2enabled#0.\n", d);
			SEND_TO_Q ("#1NOTE: For best results, a default of #0white#1 or #0whitish-gray#1 text is recommended.\n", d);
			display_main_menu(d);
			return;
		}
	}

/*** Toggle Newsletter ***/
	else if ( strtol(argument, NULL, 10) == 10 ) {
		if ( d->account->newsletter ) {
			d->account->newsletter = FALSE;
			save_account(d->account);
			SEND_TO_Q ("\n#1You will no longer receive our weekly newsletter via email.#0\n", d);
			display_main_menu(d);
			return;
		}
		else {
			d->account->newsletter = TRUE;
			save_account(d->account);
			SEND_TO_Q ("\n#2You will now receive our weekly newsletter via email.#0\n", d);
			display_main_menu(d);
			return;
		}
	}

/*** MUD mail access **/
	else if ( strtol(argument, NULL, 10) == 11 ) {

		display_hobbitmail_inbox (d, d->account);

		d->connected = CON_MAIL_MENU;
		return;
	}


/*** logging out ***/
	else if ( strtol(argument, NULL, 10) == 12 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s [%s] has logged out.\n", d->account->name, d->host);
		send_to_gods (buf);
		snprintf (buf, MAX_STRING_LENGTH,  "%s [%s] has logged out.", d->account->name, d->host);
		system_log(buf, FALSE);
		close_socket (d);
		return;
	}

	else {
		SEND_TO_Q ("\n#1Sorry, but that isn't a valid option.#0\n\nYour Selection: ", d);
	}
	
	return;
}


void nanny_view_pc (DESCRIPTOR_DATA *d, char *argument)
{
	STORED_PC_DATA	*pc = NULL;
	CHAR_DATA		*ch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	int 			i = 0;
	int				j = 2;
	int				k = 0;

	if ( !*argument ) {
		display_main_menu (d);
		return;
	}

	if ( !isdigit(*argument) ) {
		SEND_TO_Q ("\nYour Selection: ", d);
		return;
	}

	if ( strtol(argument, NULL, 10) == 0 ) {
		display_main_menu (d);
		return;
	}

	for ( pc = d->account->pc; pc; pc = pc->next )
		k++;

	if ((strtol(argument, NULL, 10)-1) >= k ) {
		SEND_TO_Q ("\nThat isn't a valid PC. Please pick again.\n\n", d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_VIEW_PC;
		return;
	}

	for ( pc = d->account->pc; pc; pc = pc->next ) {
		if ( (strtol(argument, NULL, 10)-1) == i ) {
			if ( !(ch = load_pc (pc->name)) ) {
				SEND_TO_Q ("\nFor some reason, that character could not be loaded. There may\n"
				           "have been a playerfile corruption. Please contact the admin staff\n"
					   "regarding this particular PC; our apologies for the inconvenience.\n", d);
				display_main_menu (d);
				return;
			}
			else break;
		}
		i++;
	}

	snprintf (buf, MAX_STRING_LENGTH,  "\n%s, a %d year-old %s %s.\n\n",
					GET_NAME(ch),
					GET_AGE(ch),
					sex_types [ch->sex],
					db_race_table [ch->race].name);

	if ( ch->pc->create_state == STATE_APPLYING ) {
		snprintf (buf, MAX_STRING_LENGTH,  "The application has not yet been completed. Please log in to finish it.\n");
		SEND_TO_Q ("\n", d);
		SEND_TO_Q (buf, d);
		display_main_menu (d);
		unload_pc (ch);
		d->character = NULL;
		return;
	}
	else if ( ch->pc->create_state == STATE_REJECTED ) {
		snprintf (buf, MAX_STRING_LENGTH,  "The application was declined. Please try again.\n");
		*buf = toupper (*buf);
		SEND_TO_Q ("\n", d);
		SEND_TO_Q (buf, d);
		d->connected = CON_ACCOUNT_MENU;
		display_main_menu (d);
		unload_pc (ch);
		d->character = NULL;
		return;
	}
	else if ( ch->pc->create_state == STATE_SUBMITTED ) {
		snprintf (buf, MAX_STRING_LENGTH,  "The application is currently awaiting review. Please be patient.\n");
		SEND_TO_Q ("\n", d);
		SEND_TO_Q (buf, d);
		d->connected = CON_ACCOUNT_MENU;
		display_main_menu (d);
		unload_pc (ch);
		d->character = NULL;
		return;
	}

	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "Keywords: %s\n"
		      "Short: %s\n"
		      "Long: %s\n\n"
		      "%s\n", ch->name, ch->short_descr, ch->long_descr, ch->description);

	*buf2 = '\0';
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "     Offense: %s ", skill_lev (ch->offense));
	for ( i = 1; i <= LAST_SKILL; i++ ) {
		if ( !real_skill (ch, i) )
			continue;
		snprintf (buf2, MAX_STRING_LENGTH,  "%12.12s: %s ",
					  skill_data[i].skill_name, skill_lev (ch->skills [i]));
		while ( strlen (buf2) < 18 )
			strcat (buf2, " ");
		strcat (buf, buf2);
		if ( !(j % 3) )
			strcat(buf,"\n");
		j++;
	}
	if ( (j % 3) )
		strcat (buf, "\n");

	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#6Hit ENTER when you are ready to return to the main menu. #0");

	page_string (d, buf);

	d->connected = CON_READING_WAIT;
	unload_pc(ch);
	d->character = NULL;
	
	return;
}

void nanny_reading_wait (DESCRIPTOR_DATA *d, char *argument)
{
	display_main_menu (d);
	SEND_TO_Q ("Your Selection: ", d);
	return;
}

void nanny_delete_pc (DESCRIPTOR_DATA *d, char *argument)
{
	STORED_PC_DATA	*pc = NULL;
	CHAR_DATA		*ch = NULL;
	char			c = '\0';
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	int 			i = 0;
	int				j = 0;

	if ( !*argument ) {
		display_main_menu (d);
		return;
	}

	if ( !isdigit(*argument) ) {
		display_main_menu (d);
		return;
	}

	for ( pc = d->account->pc; pc; pc = pc->next )
		j++;

	if ( strtol(argument, NULL, 10) -1 < 0 ) {
		SEND_TO_Q ("\nThat isn't a valid PC. Please pick again.\n\n", d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_DELETE_PC;
		return;
	}

	if ( strtol(argument, NULL, 10) -1 >= j ) {
		SEND_TO_Q ("\nThat isn't a valid PC. Please pick again.\n\n", d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_CHOOSE_PC;
		return;
	}

	for ( pc = d->account->pc; pc; pc = pc->next ) {
		if ( strtol(argument, NULL, 10) -1 == i ) {
			if ( !(d->character = load_pc (pc->name)) ) {
				SEND_TO_Q ("\n#1That character could not be loaded. The playerfile may have become\n"
                                           "corrupt; please email the staff list about this.#0\n\n", d);
				SEND_TO_Q ("Your Selection: ", d);
				d->connected = CON_DELETE_PC;
				return;
			}
			else break;
		}
		i++;
	}

	if ( d->character->pc->create_state != STATE_APPLYING && d->character->pc->create_state != STATE_SUBMITTED && d->character->pc->create_state != STATE_REJECTED ) {
		SEND_TO_Q ("\n#2Sorry, but you may only delete PCs who are currently in the application\n"
                           "queue, or those whose applications are in progress. If you wish to retire a\n"
                           "PC you are currently playing, you must do so from the account menu.#0\n", d);
		display_main_menu (d);
		unload_pc (d->character);
		d->character = NULL;
		return;
	}

	mysql_safe_query ("DELETE FROM %s.pfiles WHERE name = '%s'", PFILES_DATABASE, d->character->tname);

	c = d->character->tname[0];

	if ( d->character->pc->create_state == STATE_SUBMITTED ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s just deleted the application for %s.", d->account->name, d->character->tname);
		send_to_gods (buf);
	}

	if ( !str_cmp (d->account->pc->name, d->character->tname) )
		d->account->pc = d->account->pc->next;

	for ( pc = d->account->pc; pc; pc = pc->next ) {
		if ( !pc->next )
			break;
		if ( !str_cmp (pc->next->name, d->character->tname) )
			pc->next = pc->next->next;
	}

	SEND_TO_Q ("\n#1This character has been successfully deleted.#0\n", d);
	display_main_menu (d);
	d->connected = CON_ACCOUNT_MENU;
	
	if ( loaded_list == d->character )
		loaded_list = loaded_list->next;
	else for ( ch = loaded_list; ch; ch = ch->next ) {
		if ( ch->next && ch->next == d->character )
			ch->next = ch->next->next;
	}

	d->character = NULL;
	save_account (d->account);

	return;

}

void get_weapon_skills (CHAR_DATA *ch, int *melee, int *ranged)
{
	int		i = 0;
	int		j = 0;
	int		stop_num = 0;

	for ( i = SKILL_LIGHT_EDGE; i <= SKILL_POLEARM; i++ ) {
		if ( real_skill (ch, i) )
			j++;
	}

	if ( j > 1 )
		stop_num = number(1, j);
	else stop_num = 1;

	for ( i = SKILL_LIGHT_EDGE; i <= SKILL_POLEARM; i++ ) {
		if ( real_skill (ch, i) ) {
			j++;
			if ( j == stop_num ) {
				*melee = i;
				break;
			}
		}
	}

	j = 0;

	for ( i = SKILL_THROWN; i <= SKILL_CROSSBOW; i++ ) {
		if ( real_skill (ch, i) )
			j++;
	}

	if ( j > 1 )
		stop_num = number(1, j);
	else stop_num = 1;

	for ( i = SKILL_THROWN; i <= SKILL_CROSSBOW; i++ ) {
		if ( real_skill (ch, i) ) {
			j++;
			if ( j == stop_num ) {
				*ranged = i;
				break;
			}
		}
	}
	return;
}

void equip_newbie (CHAR_DATA *ch)
{
	OBJ_DATA	*tobj = NULL;
	int			melee = 0;
	int			ranged = 0;

	for ( tobj = ch->equip; tobj; tobj = tobj->next_content ) {
		if ( tobj == ch->equip )
			ch->equip = ch->equip->next_content;
		else
			ch->equip->next_content = tobj->next_content;
	}

	get_weapon_skills (ch, &melee, &ranged);
	ch->right_hand = NULL;
	ch->left_hand = NULL;
	/****** Add in the default gear each character starts with below ****/
	


	return;
}


/*  Restarts in-progress applications at the right place in chargen */

int determine_chargen_stage (CHAR_DATA *ch)
{
	return CON_CREATION;
}

/*                                                                          *
 * function: nanny_choose_pc                                                *
 *                                                                          *
 *                                                                          */
void nanny_choose_pc (DESCRIPTOR_DATA *d, char *argument)
{
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
	STORED_PC_DATA	*pc = NULL;
	DESCRIPTOR_DATA	*td = NULL;
	CHAR_DATA		*ch = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	int 			i = 0;
	int				j = 0;
	int				online = 0;
	int				guest = 0;

	if ( !*argument ) {
		display_main_menu (d);
		return;
	}

	if ( !isdigit(*argument) ) {
		display_main_menu (d);
		return;
	}

	if ( strtol(argument, NULL, 10) == 0 ) {
		display_main_menu (d);
		return;
	}

	for ( pc = d->account->pc; pc; pc = pc->next )
		j++;

	if ( strtol(argument, NULL, 10) -1 >= j ) {
		SEND_TO_Q ("\nThat isn't a valid PC. Please pick again.\n\n", d);
		SEND_TO_Q ("Your Selection: ", d);
		d->connected = CON_CHOOSE_PC;
		return;
	}

	for ( pc = d->account->pc; pc; pc = pc->next ) {
		if ( strtol(argument, NULL, 10) -1 == i ) {
			if ( loaded_list ) {
        		if ( !str_cmp (loaded_list->tname, pc->name) ){
               		loaded_list = loaded_list->next;
               	 }
        		else{
               		for ( ch = loaded_list; ch->next; ch = ch->next ) {
						if ( !ch->next ){
							break;
						}
                       	if ( !str_cmp (GET_NAME (ch->next), pc->name) ) {
                       	   		ch->next = ch->next->next;
                              	break;
						}
                	}
                }
        	}

			if ( !(d->character = load_pc (pc->name)) ) {
				SEND_TO_Q ("\nThat character could not be loaded. Please pick again.\n\n", d);
				SEND_TO_Q ("Your Selection: ", d);
				d->connected = CON_CHOOSE_PC;
				return;
			}
			break;
		}
		i++;
	}

	if ( !IS_SET (d->account->flags, ACCOUNT_IPSHARER) && !strstr (d->host, "www.yourmud.org") && port == PLAYER_PORT ) {
		for ( td = descriptor_list; td; td = td->next ) {
			if ( !td->character || !td->account || !td->account->name )
				continue;
			if ( IS_NPC (td->character) )
				continue;
			if ( td->connected != CON_PLYNG )
				continue;
			if ( IS_SET (td->account->flags, ACCOUNT_IPSHARER) )
				continue;
			if ( (!str_cmp (td->host, d->host) || !str_cmp (td->account->name, d->account->name)) &&
			     str_cmp (td->character->tname, d->character->tname) ) {
				SEND_TO_Q ("\n#1Sorry, but it is against policy to have two characters from the\n"
  	                                   "same account or IP address logged in at the same time.#0\n", d);
				display_main_menu (d);
				return;
			}
		}
	}

	if ( d->character->pc->create_state == STATE_REJECTED )
		d->character->pc->create_state = STATE_APPLYING;

	if ( d->character->pc->create_state == STATE_APPLYING ) {
		d->character->desc = d;
		create_menu_options(d);
		return;
	}

	if ( d->character->pc->create_state == STATE_SUBMITTED ) {
		SEND_TO_Q ("\n#6A review of your character application is pending. Please be patient.#0\n", d);
		display_main_menu (d);
		unload_pc (d->character);
		d->character = NULL;
		return;
	}

	if ( d->character->pc->create_state == STATE_DIED ) {
		SEND_TO_Q ("\n#1This character is, unfortunately, deceased.#0\n", d);
		display_main_menu (d);
		unload_pc (d->character);
		d->character = NULL;
		return;
	}

	if ( d->character->pc->create_state == STATE_SUSPENDED ) {
		SEND_TO_Q ("\n#1This character has been suspended by an administrator.#0\n", d);
		display_main_menu (d);
		unload_pc (d->character);
		d->character = NULL;
		return;
	}

	if ( port == PLAYER_PORT && !d->account->admin && maintenance_lock ) {
		SEND_TO_Q ("\n#1Sorry, but the player port is currently closed for maintenance.#0\n", d);
		display_main_menu (d);
		unload_pc (d->character);
		d->character = NULL;
		return;
	}

	if ( port == BUILDER_PORT && !d->account->admin ) {
		SEND_TO_Q ("\n#1Sorry, but this port is for game staff only.#0\n", d);
		display_main_menu (d);
		unload_pc (d->character);
		d->character = NULL;
		return;
	}

	if ( port == PLAYER_PORT && IS_SET (d->character->plr_flags, NO_PLAYERPORT) ) {
		SEND_TO_Q ("\n#6Your admin login does not have player port access privileges.#0\n", d);
		display_main_menu (d);
		unload_pc (d->character);
		d->character = NULL;
		return;
	}

	if ( port == TEST_PORT && !d->account->admin ) {
		SEND_TO_Q ("\n#1Sorry, but this port is for game implementation staff only.#0\n", d);
		display_main_menu (d);
		unload_pc (d->character);
		d->character = NULL;
		return;
	}

	if ( d->account->color )
		d->character->color = 1;
	else
		d->character->color = 0;

	perform_pfile_update (d->character);

	if ( d->account->admin &&
		!d->character->pc->level &&
		!IS_SET (d->character->flags, FLAG_ISADMIN) )
			SET_BIT (d->character->flags, FLAG_ISADMIN);

	if ( !d->account->admin &&
		!d->character->pc->level &&
		IS_SET (d->character->flags, FLAG_ISADMIN) )
			REMOVE_BIT (d->character->flags, FLAG_ISADMIN);

	if ( d->character->pc->owner ) {

		for ( td = descriptor_list; td; td = td->next )
			if ( td == d->character->pc->owner )
				break;

		if ( !td ) {
			d->character->pc->owner = NULL;
			system_log("AVOIDED CRASH BUG:  Entering game with owner set wrong.", TRUE);
		}
	}

		/* Character is re-estabilishing a connection while connected */

	if ( d->character->desc ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s already online, disconnecting old connection.", GET_NAME (d->character));
		system_log(buf, FALSE);
		close_socket (d->character->desc);
	}

	for ( td = descriptor_list; td; td = td->next ) {

		if ( td == d )
			continue;

		if ( td->character == d->character ) {
			close_socket (td);
			break;
		}

	}

	d->character->pc->owner = d;

	d->character->pc->last_connect = time (0);

	ch = get_pc (GET_NAME (d->character));	
	if ( ch && !ch->pc->admin_loaded ) {
		REMOVE_BIT (ch->act, PLR_QUIET);
		d->connected = CON_PLYNG;

		ch->desc = d;
		d->time_last_activity = mud_time;
		ch->pc->time_last_activity = mud_time;

		unload_pc (ch);		/* Reconnected, we have an extra load count */

		if ( d->character->room->virtual == LINKDEATH_HOLDING_ROOM ) {
			char_from_room (d->character);
			char_to_room (d->character, d->character->was_in_room);
		}

		act ("$n has reconnected.", TRUE, ch, 0, 0, TO_ROOM);
		snprintf (buf, MAX_STRING_LENGTH,  "%s has reconnected.", GET_NAME (ch));
		system_log(buf, FALSE);
		strcat (buf, "\n");
		send_to_gods (buf);
		do_look (d->character, "", 0);
		d->prompt_mode = 1;
		d->connected = CON_PLYNG;

		if ( d->character->pc->level )
			show_unread_messages (d->character);
		
		return;
	}

	pc_to_game (d->character);
	d->prompt_mode = 1;
	d->connected = CON_PLYNG;
	d->character->desc = d;

	REMOVE_BIT (d->character->flags, FLAG_ENTERING);
	REMOVE_BIT (d->character->flags, FLAG_LEAVING);

	d->time_last_activity = mud_time;
	d->character->pc->time_last_activity = mud_time;

	if ( d->character->pc->create_state == STATE_DIED ) {

		GET_POS (d->character) = STAND;

		char_to_room (d->character, 666);
		SET_BIT (d->character->flags, FLAG_DEAD);
		snprintf (buf, MAX_STRING_LENGTH,  "#1Dead Character:#0 %s has entered the game.\n",
					  GET_NAME (d->character));
		send_to_gods (buf);
		do_look (d->character, "", 15);

		return;
	}

	if ( (!d->character->in_room || d->character->in_room == NOWHERE) &&
		!d->character->right_hand && !d->character->left_hand && !d->character->equip) {
		equip_newbie(d->character);
	}

	if ( !d->character->skills [SKILL_LISTEN] )
		open_skill (d->character, SKILL_LISTEN);

	load_char_objs (d->character, GET_NAME (d->character));

	if ( !d->character->in_room || d->character->in_room == NOWHERE )
		char_to_room (d->character, OOC_LOUNGE);
	else {
		char_to_room (d->character, d->character->in_room);
	}

	act ("$n enters the area.", TRUE, d->character, 0, 0, TO_ROOM);

	snprintf (buf, MAX_STRING_LENGTH,  "%s last entered the game %s\n",
				  GET_NAME (d->character),
				  d->character->pc->last_logon ?
				  (char *)ctime (&d->character->pc->last_logon) : "never ");
	buf [strlen (buf) - 2] = '.';	/* gets rid of nl created by ctime */
	send_to_gods (buf);

	if ( IS_SET (d->character->flags, FLAG_BINDING) )
		REMOVE_BIT (d->character->flags, FLAG_BINDING);

	snprintf (buf, MAX_STRING_LENGTH,  "save/player/%c/%s.a",
			LOWER (*GET_NAME (d->character)),
			CAP (GET_NAME (d->character)));

	(void)load_saved_mobiles (d->character, buf);
	*buf = '\0';
	
	if ( d->character->pc->last_logoff )
		offline_healing(d->character, d->character->pc->last_logoff);

	if ( d->character->room->virtual == LINKDEATH_HOLDING_ROOM ) {
		char_from_room (d->character);
		char_to_room (d->character, d->character->was_in_room);
		d->character->was_in_room = 0;
		act ("$n enters the area.", TRUE, d->character, 0, 0, TO_ROOM);
	}

	send_to_char ("\n", d->character);
	do_look (d->character, "", 15);

	if ( IS_SET (d->character->plr_flags, NEWBIE) ) {
		send_to_char ("\n#6Please read HELP COMMENCE for information on starting play.#0\n", d->character);
	}
	else if ( !IS_SET (d->account->flags, ACCOUNT_NOVOTE) ) {
		mysql_safe_query ("SELECT last_tms_vote,last_mm_vote,last_tmc_vote FROM forum_users WHERE username = '%s'", d->account->name);
		if ( ( result = mysql_store_result(database) ) != NULL ) {
			row = mysql_fetch_row(result);
			if ( ((time(0) - ((60*60*24) + (60*5))) >= strtol(row[1], NULL, 10)) || ((time(0) - ((60*60*24) + (60*6))) >= strtol(row[2], NULL, 10)) ) {
				send_to_char ("\n#6Have you voted recently? If not, and you wish to support YourMud,\n"
					      "please see HELP VOTE for details. Thank you.#0\n", d->character);
			}
			mysql_free_result (result);
		}
		else {
		  snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_choose_pc(): %s", mysql_error ( database ) );
		  system_log( buf, TRUE );
		}
	}

	if ( *buf ) {
		send_to_char ("\n", d->character);
		send_to_char (buf, d->character);
	}

	show_waiting_prisoners (d->character);
	notify_captors (d->character);

	if ( (d->character->race == RACE_HUMAN)
	     && (d->character->max_hit != 50 + d->character->con*CONSTITUTION_MULTIPLIER) ) {
	      	d->character->max_hit = 50 + d->character->con*CONSTITUTION_MULTIPLIER;	/*  All humanoids are roughly the same,*/
	      	d->character->hit = d->character->max_hit;					/*  in terms of wound-endurance. */
	}

	if ( d->character->pc->level )
		show_unread_messages (d->character);

	if ( d->character->pc->creation_comment && strlen(d->character->pc->creation_comment) > 2 && !d->character->pc->level ) {
		d->pending_message = (MESSAGE_DATA *)alloc (sizeof (MESSAGE_DATA), 1);
		d->pending_message->poster  = str_dup (GET_NAME (d->character));
		d->pending_message->subject = str_dup ("Background Information.");
		d->pending_message->message = str_dup(d->character->pc->creation_comment);
		post_to_mysql_player_board (d);

		d->pending_message = (MESSAGE_DATA *)alloc (sizeof (MESSAGE_DATA), 1);
		d->pending_message->poster  = str_dup (GET_NAME (d->character));
		d->pending_message->subject = str_dup ("My Background.");
		d->pending_message->message = str_dup(d->character->pc->creation_comment);
		post_to_mysql_journal (d);

		d->character->pc->creation_comment = NULL;
	}

	if ( d->character->pc->special_role ) {
		d->pending_message = (MESSAGE_DATA *)alloc (sizeof (MESSAGE_DATA), 1);
		d->pending_message->poster  = str_dup (GET_NAME (d->character));
		d->pending_message->subject = str_dup ("Special Role Selected in Chargen.");
		snprintf (buf, MAX_STRING_LENGTH,
					"Role Name: %s\n"
		            "Role Cost: %d points\n"
                    "Posted By: %s\n"
			        "Posted On: %s\n"
			        "\n"
			        "%s\n",
			        d->character->pc->special_role->summary, d->character->pc->special_role->cost, d->character->pc->special_role->poster, d->character->pc->special_role->date, d->character->pc->special_role->body); 
		
		d->pending_message->message = str_dup(buf);
		post_to_mysql_player_board (d);

		d->character->pc->special_role = NULL;
	}

	online = 0;
	guest = 0;

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

	if ( online >= count_max_online ) {
		count_max_online = online;
        max_online_date = timestr(max_online_date);
	}

	if ( guest >= count_guest_online ) {
		count_guest_online = guest;
	}

	d->character->pc->last_logon = time (0);

	return;

}

/*                                                                          *
 * function: nanny_change_email                                             *
 *                                                                          *
 *                                                                          */
void nanny_change_email (DESCRIPTOR_DATA *d, char *argument)
{
	MYSQL_RES	*result =NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int         nEmailMatches = 0;

	if ( *argument ) {

		if ( strstr (argument, " ") || strstr (argument, ";") || strstr (argument, "\\") || strstr (argument, "(") ) {
			SEND_TO_Q ("\n#1Your input contains illegal characters.#0\n", d);
			display_main_menu (d);
			return;
		}

		mysql_safe_query ("SELECT user_email FROM forum_users WHERE user_email = '%s' and username != '%s'", argument, d->account->name);
		
		if ( ( result = mysql_store_result(database) ) != NULL ) {
		  nEmailMatches = mysql_num_rows( result );
		  mysql_free_result (result); result = NULL;
		}
		else {
		  snprintf (buf, MAX_STRING_LENGTH, "Warning: nanny_change_email(): %s", mysql_error ( database ) );
		  system_log( buf, TRUE );
		  SEND_TO_Q ("\n#1An error occurred.#0\nYour email address was NOT updated in our server.\n", d);
		  display_main_menu (d);
		  return;
		}

		if ( nEmailMatches ) {
		  SEND_TO_Q ("\n#1We're sorry, but that email address has already been registered\n"
			     "under an existing game account. Please choose another.#0\n\n", d);
		  SEND_TO_Q ("Your email address was NOT updated in our server.\n", d);
		  display_main_menu (d);
		  return;
		}

		snprintf (buf, MAX_STRING_LENGTH,  "\nIs the address %s correct? [y/n] ", argument);
		SEND_TO_Q (buf, d);
		d->stored = str_dup(argument);
		d->connected = CON_CHG_EMAIL_CNF;
		return;
	}
	else {
		SEND_TO_Q ("\nYour email address was NOT updated in our server.\n", d);
		display_main_menu (d);
		return;
	}
}

void nanny_change_email_confirm (DESCRIPTOR_DATA *d, char *argument)
{
	CAP(argument);

	if ( *argument == 'Y' ) {
		SEND_TO_Q ("\nYour email address was successfully updated.\n", d);
		d->account->email = str_dup (d->stored);
		mem_free (d->stored);
		d->stored = NULL;
		save_account(d->account);
		display_main_menu (d);
		return;
	}
	else {
		SEND_TO_Q ("\nYour email address was NOT updated in our server.\n", d);
		display_main_menu (d);
		return;
	}
	return;
}


char *role_flag_descs (int bitflag)
{
	if ( bitflag == XTRA_COIN )
		return "Extra Starting Coin";
	else if ( bitflag == APPRENTICE )
		return "Apprenticeship";
	else if ( bitflag == STARTING_ARMOR )
		return "Leather Armor";
	else if ( bitflag == SKILL_BONUS )
		return "Distributed Skill Bonuses";
	else if ( bitflag == XTRA_SKILL )
		return "Extra Skill Choice";
	else if ( bitflag == MAXED_STAT )
		return "Maximized Attribute";
	else if ( bitflag == JOURNEYMAN )
		return "Journeymanship";
	else if ( bitflag == FELLOW )
		return "Fellowship";
	else if ( bitflag == LESSER_NOBILITY )
		return "Lesser Nobility";
	else if ( bitflag == APPRENTICE_MAGE )
		return "Arcane Apprenticeship";

	return "Unknown Role Selection";
}



void spitstat (CHAR_DATA *ch, DESCRIPTOR_DATA *recipient)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	bool		missing_info = FALSE;
	int			req = 0;
	char		*temp_arg = NULL;

	snprintf (buf2, MAX_STRING_LENGTH,  "%d inches", ch->height);

	snprintf (buf, MAX_STRING_LENGTH,  "\nName: %s", ch->tname);
	pad_buffer (buf, 25); 
	snprintf (ADDBUF, MAX_STRING_LENGTH, "Account: %s", ch->pc->account);
	pad_buffer (buf, 50);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "Race: %s",
					db_race_table [ch->race].name);
	SEND_TO_Q (buf, recipient);
	*buf = '\0';

	snprintf (ADDBUF, MAX_STRING_LENGTH, "\nGender: %s", sex_types[ch->sex]);
	pad_buffer (buf, 25);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "Age: %d", ch->age);
	pad_buffer (buf, 50);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "Build: %s, %s\n", ch->height == 1 ? "short" : ch->height == 2 ? "average" : ch->height == 3 ? "tall" : buf2, frames[ch->frame]);
	SEND_TO_Q (buf, recipient);	
	*buf = '\0';

	if ( !ch->name || !*ch->name || !str_cmp (ch->name, "(null)") ) {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "\n#1Keywords:#0 %s\n\n", ch->name);
		missing_info = TRUE;
	}
	else snprintf (ADDBUF, MAX_STRING_LENGTH, "\nKeywords: %s\n\n", ch->name);
	if ( !ch->short_descr || !*ch->short_descr || !str_cmp (ch->short_descr, "(null)") ) {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#1Short Description:#0 %s\n", ch->short_descr);
		missing_info = TRUE;
	}
	else snprintf (ADDBUF, MAX_STRING_LENGTH, "Short Description: %s\n", ch->short_descr);
	if ( !ch->long_descr || !*ch->long_descr || !str_cmp (ch->long_descr, "(null)") ) {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "#1Long Description:#0  %s\n\n", ch->long_descr);
		missing_info = TRUE;
	}
	else snprintf (ADDBUF, MAX_STRING_LENGTH, "Long Description:  %s\n\n", ch->long_descr);

	SEND_TO_Q (buf, recipient);

	if ( !ch->description || !*ch->description || !str_cmp (ch->description, "(null)") ) {
		SEND_TO_Q ("#1Physical Description:#0\n", recipient);
		missing_info = TRUE;
	}
	else SEND_TO_Q ("Physical Description:\n", recipient);

	if ( ch->description ) {
		SEND_TO_Q (ch->description, recipient);
	}

	*buf = '\0';

	if ( !ch->pc )
		CREATE (ch->pc, PC_DATA, 1);

	if ( ch->pc->role ){
		temp_arg = role_flag_descs(ch->pc->role);
		snprintf (ADDBUF, MAX_STRING_LENGTH, "\nPurchased Starter: %s\n", temp_arg);
	}

	if ( ch->pc->special_role && IS_MORTAL(recipient->character))
		snprintf (ADDBUF, MAX_STRING_LENGTH, "\nSelected Role:\n%s", ch->pc->special_role->body);
	else if ( ch->pc->special_role && !IS_MORTAL(recipient->character))
		snprintf (ADDBUF, MAX_STRING_LENGTH, "Selected Role: %s\n", ch->pc->special_role->summary);

	if ( !IS_MORTAL(recipient->character) ) {
		if ( ch->pc->app_cost )
			snprintf (ADDBUF, MAX_STRING_LENGTH, "Application Cost: %d RP Points\n", ch->pc->app_cost);
		if ( ch->pc->special_role )
			req = MAX (ch->pc->special_role->cost, db_race_table[ch->race].race_point_reqs );
		else req = db_race_table[ch->race].race_point_reqs;
		if ( req )
			snprintf (ADDBUF, MAX_STRING_LENGTH, "Application Tier: %d RP Points\n", req);
	}

	SEND_TO_Q (buf, recipient);

	*buf = '\0';

	if ( ch->race == RACE_HUMAN ) {
		snprintf (ADDBUF, 23, "\nStarting Location: ");
		if ( IS_SET (ch->plr_flags, START_ALPHA ))
			snprintf (ADDBUF, 11, "Alpha Room\n");
		else if ( IS_SET (ch->plr_flags, START_BETA) )
			snprintf (ADDBUF, 14, "Beta Room\n");
		else snprintf (ADDBUF, 13, "None Chosen\n");
	}
	
	temp_arg = get_profession_name(ch->pc->profession);
	snprintf (ADDBUF, MAX_STRING_LENGTH, "\nChosen Profession: %s\n", temp_arg );

	if ( !ch->pc->creation_comment || !*ch->pc->creation_comment || (ch->pc->creation_comment && !str_cmp (ch->pc->creation_comment, "(null)")) ) {
		snprintf (ADDBUF, MAX_STRING_LENGTH, "\n#1Background Comment:#0\n%s", ch->pc->creation_comment);
		missing_info = TRUE;
	}
	else snprintf (ADDBUF, MAX_STRING_LENGTH, "\nBackground Comment:\n%s", ch->pc->creation_comment);

	if ( missing_info && IS_MORTAL (recipient->character) )
		snprintf (ADDBUF, 80, "\n#1Items marked in red MUST be completed before you submit your application.#0\n");
	else if ( missing_info && !IS_MORTAL (recipient->character) )
		snprintf (ADDBUF, 80, "\n#1Items marked in red were not completed by the applicant.#0\n");
	if ( *buf )
		SEND_TO_Q (buf, recipient);

	return;
}

void create_menu_options (DESCRIPTOR_DATA *d)
{
	CHAR_DATA	*ch = d->character;

	if ( !ch->tname ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "NAME");
		return;
	}

	if ( ch->pc->nanny_state == STATE_GENDER ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "SEX");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_SPECIAL_ROLES ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "CLASSIFIEDS");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_RACE ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "RACE");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_LOCATION ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "LOCATION");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_AGE ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "AGE");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_ATTRIBUTES ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "ATTRIBUTES");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_FRAME ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "HEIGHT");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_SDESC ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "SHORT");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_KEYWORDS ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "KEYWORDS");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_LDESC ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "LONG");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_FDESC ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "DESCRIPTION");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_SKILLS ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "SKILLS");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_PROFESSION ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "PROFESSION");
		return;
	}

	else if ( ch->pc->nanny_state == STATE_COMMENT ) {
		ch->pc->create_state = STATE_APPLYING;
		create_menu_actions (d, "COMMENT");
		return;
	}

	if ( ch->pc->create_state == STATE_SUBMITTED ) {
		SEND_TO_Q ("\nCommands:  QUIT, CHECK\n\n> ", d);
		d->connected = CON_CREATION;
		return;
	}

	spitstat (ch, d);

	if ( is_newbie (ch) ) 
		SEND_TO_Q ("\nCommands:  Age, Attributes, Comment, Description, Frame, Height, Keywords, Location\n"
			     "           Long, Profession, Quit, Race, Sex, Short, and Skills.\n", d);
	else
		SEND_TO_Q ("\nCommands:  Age, Attributes, Classifieds, Comment, Description, Frame, Height,\n"
			     "           Keywords, Location, Long, Profession, Quit, Race, Sex, Short, and Skills.\n", d);

	if ( d->character->pc->msg && *d->character->pc->msg != '~' ) {
		SEND_TO_Q ("\n#6Note:      Your application has been processed; type REVIEW to read the response.#0\n", d);
	}

	SEND_TO_Q ("\n#2Note:      When ready, please use SUBMIT to finalize your application.\n#0", d);

	SEND_TO_Q ("\n> ", d);

	d->connected = CON_CREATION;
	
	return;
}

void attribute_priorities (DESCRIPTOR_DATA *d, char *arg)
{
	int			bonus = 0;
	int			attr = 0;
	int			i = 0;
	int			skill = 0;
	CHAR_DATA	*ch = d->character;
	int			attr_starters [] =   { 16, 15, 12, 12, 11, 10, 8 };
	int			attr_priorities [] = { -1, -1, -1, -1, -1, -1, -1 };
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		msg [MAX_STRING_LENGTH] = {'\0'};

	ch->str = 0;

	for ( i = 0; i < 7; i++ ) {
		arg = one_argument (arg, buf);

		if ( !*buf ) {
			SEND_TO_Q ("\n#2Please enter all attributes in descending "
					   "priority.\nExample:  STR DEX"
					   " CON WIL INT AUR AGI#0\n", d);
			d->connected = CON_CREATION;
			return;
		}

		attr = index_lookup (attrs, buf);

		if ( attr == -1 ) {
			snprintf (msg, MAX_STRING_LENGTH, "\n#1'%s' is not recognized as an attribute.\n#0", buf);
			SEND_TO_Q (msg, d);
			return;
		}

		if ( attr_priorities [attr] != -1 ) {
			snprintf (msg, MAX_STRING_LENGTH, "The attribute '%s' is duplicated in your list.\n",
						  buf);
			SEND_TO_Q (msg, d);
			return;
		}

		attr_priorities [attr] = i;
	}

		/* Add in bonus 7 attribute points randomly */

	for ( bonus = 8; bonus; ) {

		attr = number (0, 6);

		if ( attr_starters [attr_priorities [attr]] < 18 ) {
			attr_starters [attr_priorities [attr]]++;
			bonus--;
		}
	}

		/* Assign actual numbers */

	ch->str   = attr_starters [attr_priorities [0]];
	ch->dex   = attr_starters [attr_priorities [1]];
	ch->con   = attr_starters [attr_priorities [2]];
	ch->wil   = attr_starters [attr_priorities [3]];
	ch->intel = attr_starters [attr_priorities [4]];
	ch->aur   = attr_starters [attr_priorities [5]];
	ch->agi   = attr_starters [attr_priorities [6]];

	ch->pc->start_str		= ch->str;
	ch->pc->start_dex		= ch->dex;
	ch->pc->start_con		= ch->con;
	ch->pc->start_wil		= ch->wil;
	ch->pc->start_intel		= ch->intel;
	ch->pc->start_aur		= ch->aur;
	ch->pc->start_agi		= ch->agi;

		/* Reset skills */

	for ( skill = 1; skill < MAX_SKILLS; skill++ ) {
		d->character->skills [skill] = 0;
		d->character->pc->skills [skill] = 0;
	}
	return;
}

void sex_selection (DESCRIPTOR_DATA *d, char *arg)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*ch = d->character;

	arg = one_argument (arg, buf);

	if ( toupper (*buf) == 'M' || !str_cmp (buf, "male") )
		ch->sex = SEX_MALE;
	else if ( toupper (*buf) == 'F' || !str_cmp (buf, "female") )
		ch->sex = SEX_FEMALE;
	else
		SEND_TO_Q ("Please choose MALE or FEMALE.\n", d);
		
	return;
}

void nanny_race_confirm (DESCRIPTOR_DATA *d, char *arg)
{
	int		i = 0;
	char	*temp_arg = NULL;
	
	if ( !*arg ) {
		SEND_TO_Q ("Do you still wish to play a ", d);
		SEND_TO_Q (db_race_table [d->character->race].name, d);
		SEND_TO_Q ("? [y/n] ", d);
		return;
	}

	CAP (arg);

	if ( *arg == 'Y' ) {
	        for ( i = 1; i <= LAST_SKILL; i++ ) {
			d->character->skills [i] = 0;
			d->character->pc->skills [i] = 0;
        	}

		d->connected = CON_CREATION;

		if ( d->character->race == RACE_HUMAN )
			d->character->pc->nanny_state = STATE_LOCATION;
		else d->character->pc->nanny_state = STATE_AGE;

		create_menu_options (d);

		if ( db_race_table[d->character->race].race_point_reqs )
			d->character->pc->app_cost += MAX(db_race_table[d->character->race].race_point_reqs/2,1);

		if ( db_race_table[d->character->race].race_age_limits[0] >
						d->character->age )
			d->character->age =
						db_race_table[d->character->race].race_age_limits[0];
		if ( db_race_table[d->character->race].race_age_limits[1] <
						d->character->age )
			d->character->age =
						db_race_table[d->character->race].race_age_limits[1];

		return;
	}
	else {
		SEND_TO_Q ("\n", d);
		temp_arg = get_text_buffer (NULL, text_list, "race_select");
		SEND_TO_Q (temp_arg, d);
		*b_buf = '\0';
		
		
		for ( i = 0; i <= MAX_RACE; i++ ) {
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
			"%d.  #2%-18s#0    %d.  #2%-18s#0    %d.  #2%-18s#0\n",
			i+1, d->account->roleplay_points >= db_race_table [i].race_point_reqs ? db_race_table [i].name : "",
			i+2, d->account->roleplay_points >= db_race_table [i+1].race_point_reqs ? db_race_table [i+1].name : "",
			i+3, d->account->roleplay_points >= db_race_table [i+2].race_point_reqs ? db_race_table [i+2].name : "");
			
			i++;
			i++;
		}
		
		
		strcat (b_buf, "\nYour Desired Race: ");
		page_string (d, b_buf);
		d->character->race = -1;
		d->connected = CON_RACE_SELECT;
		return;
	}
	
	return;
}

void nanny_privacy_confirm (DESCRIPTOR_DATA *d, char *arg)
{

	if ( !*arg ) {
		SEND_TO_Q ("\n#2Do you wish to flag your application as private? [y/n]#0 ", d);
		return;
	}

	CAP (arg);

	if ( *arg == 'Y' ) {
		SET_BIT (d->character->plr_flags, PRIVATE);
	}
	else {
		REMOVE_BIT (d->character->plr_flags, PRIVATE);
	}

	d->character->pc->nanny_state = 0;

	create_menu_options (d);
	
	return;
}

void nanny_char_name_confirm (DESCRIPTOR_DATA *d, char *arg)
{
	CHAR_DATA	*tch = NULL;
	int 		i = 0;
	STORED_PC_DATA	*pc = NULL;

	if ( !*arg ) {
		SEND_TO_Q ("\n#2Character generation aborted.#0\n", d);
		display_main_menu (d);
		return;
	}

	if (!strncasecmp(d->account->name, arg, strlen(d->account->name))) {
		if ( arg[strlen(arg) - 1] != '!') {
			SEND_TO_Q ("\n#1Your character name may not be similar to your account name.#0\n\n#1Please press ENTER to continue.#0\n", d);
			d->connected = CON_PLAYER_NEW;
			return;
		}
		else {
			arg[strlen(arg) - 1] = 0;
		}
	}

	if ( (tch = load_pc (arg)) ) {
		SEND_TO_Q ("\n#1That character name is already in use. Please press ENTER to continue.#0\n", d);
		d->connected = CON_PLAYER_NEW;
		unload_pc (tch);
		return;
	}

	if ( strlen (arg) > 15 ) {
		SEND_TO_Q ("Please choose a character name of less than 15 characters.\n", d);
		d->connected = CON_PLAYER_NEW;
		return;
	}

	for ( i = 0; i < strlen (arg); i++ ) {
		if ( !isalpha (arg [i]) ) {
			SEND_TO_Q ("Illegal characters in character name (letters only, please).\n", d);
			d->connected = CON_PLAYER_NEW;
			return;
		}

		if ( i )
			arg [i] = tolower (arg [i]);
		else
			arg [i] = toupper (arg [i]);
	}

	if ( !*arg ) {
		SEND_TO_Q ("What would you like to name your new character? ", d);
		d->connected = CON_PLAYER_NEW;
		return;
	}

	SEND_TO_Q ("\n#2Your character has been named. Press ENTER to continue.#0", d);

	if ( !d->account->pc ) {
		CREATE (d->account->pc, STORED_PC_DATA, 1);
		d->account->pc->name = str_dup (arg);
	}
	else for ( pc = d->account->pc; pc; pc = pc->next ) {
		if ( !str_cmp (pc->name, arg) )
			break;
		if ( !pc->next ) {
			CREATE (pc->next, STORED_PC_DATA, 1);
			pc->next->name = str_dup (arg);
			break;
		}
	}
	save_account (d->account);

	CAP (arg);
	d->character = new_char (1);
	clear_char (d->character);
	d->character->tname = add_hash(arg);
	d->character->pc->create_state = STATE_APPLYING;
	d->character->race = -1;
	d->character->pc->account = add_hash(d->account->name);

	d->character->desc = d;

	d->character->short_descr = 0;
	d->character->long_descr = 0;
	d->character->description = 0;

	d->character->time.birth = time(0);
	d->character->time.played = 0;
	d->character->time.logon = time(0);

	d->character->armor = 0;

	d->character->affected_by = 0;

	d->character->intoxication = 0;
	d->character->thirst = 24;
	d->character->hunger = 24;

	d->character->pc->load_count = 1;
	save_char (d->character, FALSE);
	
	d->character->pc->nanny_state = STATE_GENDER;
	d->connected = CON_RACE;
	
	return;
}

void nanny_special_role_selection (DESCRIPTOR_DATA *d, char *arg)
{
	ROLE_DATA	*role = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			i = 0;
	int			j = 0;

	if ( !*arg || !isdigit(*arg) || strtol(arg, NULL, 10) < 0 ) {
		SEND_TO_Q ("#2\nPlease select the number of one of the listed entries, or 0 to pass.#0\n\n", d);
		SEND_TO_Q ("Your Desired Role: ", d);
		return;
	}

	if ( strtol(arg, NULL, 10) == 0 ) {
		if ( d->character->pc->nanny_state )
			d->character->pc->nanny_state = STATE_SKILLS;
		d->connected = CON_CREATION;
		if ( d->character->pc->special_role ) {
			d->character->pc->special_role = NULL;
		}
		create_menu_options (d);
		return;
	}

	i = strtol(arg, NULL, 10);

	for ( j = 1, role = role_list; role; role = role->next, j++ ) {
		if ( role->cost > (d->account->roleplay_points) ) {
			j--;
			continue;
		}
		if ( j == i )
			break;
	}

	if ( !role ) {
		SEND_TO_Q ("#2\nPlease select the number of one of the listed entries, or 0 to pass.#0\n\n", d);
		SEND_TO_Q ("Your Desired Role: ", d);
		return;
	}		

	CREATE (d->character->pc->special_role, ROLE_DATA, 1);
	d->character->pc->special_role->summary = str_dup(role->summary);
	d->character->pc->special_role->body = str_dup(role->body);
	d->character->pc->special_role->poster = str_dup(role->poster);
	d->character->pc->special_role->date = str_dup(role->date);
	d->character->pc->special_role->cost = role->cost;

	SEND_TO_Q ("\n", d);
	snprintf (buf, MAX_STRING_LENGTH,  "#2Role Contact:#0  %s\n", role->poster);
	snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "#2Posted On:#0     %s\n\n", role->date);
	SEND_TO_Q (buf, d);
	SEND_TO_Q (role->body, d);
	SEND_TO_Q ("\n", d);
	SEND_TO_Q ("Do you still wish to choose this role? [y/n]", d);
	d->connected = CON_SPECIAL_ROLE_CONFIRM;
	return;
}

void nanny_special_role_confirm (DESCRIPTOR_DATA *d, char *arg)
{
	int			i = 0;
	ROLE_DATA	*role = NULL;
	char		*temp_arg = NULL;
	
	if ( !*arg ) {
		SEND_TO_Q ("Do you still wish to choose this role? [y/n]", d);
		return;
	}

	CAP (arg);

	if ( *arg == 'Y' ) {
		if ( d->character->pc->nanny_state )
			d->character->pc->nanny_state = STATE_RACE;
		d->connected = CON_CREATION;
		create_menu_options (d);
		return;
	}
	else {
		SEND_TO_Q ("\n", d);
		*b_buf = '\0';
		temp_arg = get_text_buffer (NULL, text_list, "special_role_select");
		SEND_TO_Q (temp_arg, d);
		role = role_list;
		
		for ( role = role_list, i = 1; role; role = role->next, i++ ) {
           	if ( !role )
               	break;
        	if ( role->cost > (d->account->roleplay_points) ) {
            	i--;
                continue;
			}
                
            if ( i < 10 )
            	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%d.  #2%s#0\n", i, role->summary);
                
			else
            	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%d. #2%s#0\n", i, role->summary);
		}
            
        strcat (b_buf, "\nYour Desired Role: ");
        page_string (d, b_buf);
        d->connected = CON_SPECIAL_ROLE_SELECT;
        return;
	}
	return;
}

void race_selection (DESCRIPTOR_DATA *d, char *arg)
{
	char	race_name [MAX_STRING_LENGTH] = {'\0'};

	snprintf (race_name, MAX_STRING_LENGTH, "%s", db_race_table [(strtol(arg, NULL, 10)-1)].name);


	if ( !*arg || !isdigit(*arg) || strtol(arg, NULL, 10) > 30 || strtol(arg, NULL, 10) < 1 ) {
		SEND_TO_Q ("Please select the number of one of the above races.\n", d);
		SEND_TO_Q ("Your Desired Race: ", d);
		return;
	}

	if ( d->account->roleplay_points < db_race_table[(strtol(arg, NULL, 10)-1)].race_point_reqs ) {
		SEND_TO_Q ("\nThat race is currently unavailable to you; accrue more\n"
		           "roleplay point awards and try again in the future.\n", d);
		SEND_TO_Q ("\nYour Desired Race: ", d);
		return;
	}
	else {
		d->character->race = strtol(arg, NULL, 10)-1;
		SEND_TO_Q ("\n", d);
		SEND_TO_Q (db_race_table[d->character->race].race_desc, d);
		SEND_TO_Q ("\n", d);
		SEND_TO_Q ("Do you still wish to play a ", d);
		SEND_TO_Q (db_race_table [d->character->race].name, d);
		SEND_TO_Q ("? [y/n] ", d);
		d->connected = CON_RACE_CONFIRM;
		return;
	}
	
	return;
}

void age_selection (DESCRIPTOR_DATA *d, char *arg)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	CHAR_DATA	*ch = d->character;
	int			lower = 0;
	int			upper = 0;

	d->character->age = 0;

	arg = one_argument (arg, buf);
	d->character->pc->nanny_state = STATE_AGE;

	lower = db_race_table[d->character->race].race_age_limits[0];
	upper = db_race_table[d->character->race].race_age_limits[1];
	

	if ( strtol(buf, NULL, 10) < lower || strtol(buf, NULL, 10) > upper ) {
		SEND_TO_Q ("\n#1Please select an age within the specified range.#0\n", d);
		d->connected = CON_AGE;
		return;
	}

	ch->age = strtol(buf, NULL, 10);
	
	return;
}

int available_roles (int points)
{
	ROLE_DATA	*role = NULL;

	for ( role = role_list; role; role = role->next ) {
		if ( role->cost < points )
			return 1;
	}

	return 0;
}

void location_selection (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf || !isdigit (*buf) || (strtol(buf, NULL, 10) < 1 || strtol(buf, NULL, 10) > 2) ) {
		SEND_TO_Q ("#2Please select a number from the list above.#0\n", d);
		return;
	}

	if ( strtol(buf, NULL, 10) == 1 )
		SET_BIT (d->character->plr_flags, START_ALPHA);
	else if ( strtol(buf, NULL, 10) == 2 )
		SET_BIT (d->character->plr_flags, START_BETA);
	else if ( strtol(buf, NULL, 10) == 3 )
		SET_BIT (d->character->plr_flags, START_GAMMA);

	if ( d->character->pc->nanny_state )
		d->character->pc->nanny_state = STATE_AGE;

	d->connected = CON_CREATION;
	
	return;
}

void height_frame_selection (DESCRIPTOR_DATA *d, char *argument)
{
	int			ind = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		SEND_TO_Q ("#2Enter a frame and height, e.g. 'tall light'.#0\n", d);
		d->character->height = 0;
		return;
	}

	if ( !str_cmp (buf, "short") )
		d->character->height = 1;
	else if ( !str_cmp (buf, "average") )
		d->character->height = 2;
	else if ( !str_cmp (buf, "tall") )
		d->character->height = 3;
	else {
		SEND_TO_Q ("#2\nEnter 'height frame', e.g. 'tall light'.#0\n", d);
		d->character->height = 0;
		return;
	}

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		SEND_TO_Q ("#2\nEnter a frame after the height, e.g. 'tall light'.#0\n", d);
		d->character->height = 0;
		return;
	}

	ind = index_lookup (frames, buf);

	if ( ind < 1 || ind == FRAME_SUPER_MASSIVE ) {
		SEND_TO_Q ("Valid frames are scant, light, medium, heavy and massive.\n", d);
		d->character->height = 0;
		return;
	}

	d->character->frame = ind;

	if ( d->character->pc->nanny_state )
		d->character->pc->nanny_state = STATE_SDESC;

	d->connected = CON_CREATION;
	
	return;
}

int pickable_skill (CHAR_DATA *ch, char *buf)
{
	int		ind = 0;

	ind = skill_index_lookup (buf);
	
	if ( ind == -1 )
		return 0;

	if ( ind == db_race_table [ch->race].race_speaks )
		return 0;

	if ( is_restricted_skill (ch, ind) == 0 )
		return ind;

	return 0;
}

int picks_entitled (CHAR_DATA *ch)
{
	if ( ch->pc && IS_SET (ch->pc->role, XTRA_SKILL) )
		return 7;
	else return 6;
}


/*                                                                          *
 * function: profession_display                                             *
 *                                                                          *
 *                                                                          */
void profession_display (DESCRIPTOR_DATA *d)
{
	int			i = 0;
	CHAR_DATA	*ch = NULL;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int         nProfessions = 0;
	char		*temp_arg = NULL;

	ch = d->character;

	SEND_TO_Q ("\n", d);
	temp_arg = get_text_buffer (NULL, text_list, "professions");
	SEND_TO_Q (temp_arg, d);

	*buf = '\0';

	mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
	if ( ( result = mysql_store_result ( database ) ) == NULL ) {
          snprintf (buf, MAX_STRING_LENGTH, "Warning: profession_display(): %s", mysql_error ( database ) );
          system_log( buf, TRUE );
	}
	else {
	  nProfessions = mysql_num_rows(result) ;
	}

	if ( nProfessions > 0 ) {

	  i = 0;
	  while ( (row = mysql_fetch_row(result)) ) {
	    i++;
	    if ( i < 10 )
	      snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%d.  #2%-18s#0    ", i, !is_restricted_profession (d->character, row[1]) ? row[0] : "");
	    else
	      snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%d. #2%-18s#0    ", i, !is_restricted_profession (d->character, row[1]) ? row[0] : "");
	    if ( !(i % 3) )
	      snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");
	  }
	  if ( (i % 3) )
	    snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\n");

	  snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "\nYour Desired Profession: ");
	  SEND_TO_Q (buf, d);
	  d->connected = CON_PROFESSION;

	}
	else { 
	  SEND_TO_Q ("There are no professions currently defined. Continuing...\n\n", d);
	  d->connected = CON_CREATION;
	  if ( d->character->pc->nanny_state )
	    d->character->pc->nanny_state = STATE_SKILLS;
	  create_menu_options (d);
	}

	if (result) { 
	  mysql_free_result (result); result = NULL;
	}
	return;
}

/*                                                                          *
 * function: profession_selection                                           *
 *                                                                          *
  *                                                                          */
void profession_selection (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int			i = 0;
	int			j = 0;
	int			max = 0;

	if ( !*argument ) {
		profession_display (d);
		return;
	}

	d->character->pc->profession = 0;

	mysql_safe_query ("SELECT count(*) FROM professions");
	if ( ( result = mysql_store_result (database) ) != NULL ) {
	  row = mysql_fetch_row (result);
	  max = strtol(row[0], NULL, 10);
	  mysql_free_result(result); result = NULL;
	}
	else {
          snprintf (buf, MAX_STRING_LENGTH, "Warning: profession_selection(): %s", mysql_error ( database ) );
          system_log( buf, TRUE );
	  snprintf (buf, MAX_STRING_LENGTH,  "\nPlease select a number from 1 to %d.\n\nYour Desired Profession: ", max);
	  SEND_TO_Q (buf, d);
	  return;
	}

	if ( !isdigit (*argument) || strtol(argument, NULL, 10) < 1 || strtol(argument, NULL, 10) > max ) {
	  snprintf (buf, MAX_STRING_LENGTH,  "\nPlease select a number from 1 to %d.\n\nYour Desired Profession: ", max);
	  SEND_TO_Q (buf, d);
	  return;
	}

	mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
	if ( ( result = mysql_store_result (database) ) == NULL ) {
          snprintf (buf, MAX_STRING_LENGTH, "Warning: profession_selection(): %s", mysql_error ( database ) );
          system_log( buf, TRUE );
	  snprintf (buf, MAX_STRING_LENGTH,  "\nAn error occurred.\n\nYour Desired Profession: ");
	  SEND_TO_Q (buf, d);
	  return;
	}

	i = 0;

	while ( (row = mysql_fetch_row(result)) ) {	
		i++;
		if ( !is_restricted_profession (d->character, row[1]) && i == strtol(argument, NULL, 10)) {
			d->character->pc->profession = strtol(row[2], NULL, 10);
			for ( j = 0; j <= LAST_SKILL; j++ ) {
				d->character->skills[j] = 0;
				d->character->pc->skills[j] = 0;
			}
			add_profession_skills (d->character, row[1]);
			break;
		}
		else if ( is_restricted_profession (d->character, row[1]) && i == strtol(argument, NULL, 10)) {
			SEND_TO_Q ("\nThat profession is currently unavailable to you.\n\nYour Desired Profession: ", d);
			mysql_free_result(result);
			return;
		}
	}

	mysql_free_result (result); result = NULL;

	d->connected = CON_CREATION;

	if ( d->character->pc->nanny_state )
		d->character->pc->nanny_state = STATE_SKILLS;

	create_menu_options (d);

	save_char (d->character, FALSE);

	return;
}

void skill_selection (DESCRIPTOR_DATA *d, char *argument)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			skill = 0;
	int			i = 0;
	int			picks_left = 0;
	CHAR_DATA	*ch = NULL;

	ch = d->character;

	picks_left = picks_entitled (ch);

	argument = one_argument (argument, buf);

	if ( !*buf ) {
		skill_display (d);
		return;
	}

	for ( i = 1; i <= LAST_SKILL && *skill_data[i].skill_name != '\n' && picks_left > 0; i++ ) {
		if ( ch->skills [i] && pickable_skill (ch, skill_data[i].skill_name) ) {
			picks_left--;
		}
	}

	if ( !str_cmp (buf, "done") ) {
		if ( picks_left ) {
			skill_display (d);
			return;
		}
		if ( ch->skills[SKILL_LITERACY] ) {
			if ( !ch->skills [SKILL_SCRIPT_SARATI] && !ch->skills [SKILL_SCRIPT_TENGWAR] &&
				!ch->skills [SKILL_SCRIPT_BELERIAND_TENGWAR] && !ch->skills [SKILL_SCRIPT_CERTHAS_DAERON] &&
				!ch->skills [SKILL_SCRIPT_ANGERTHAS_DAERON] && !ch->skills [SKILL_SCRIPT_QUENYAN_TENGWAR] &&
				!ch->skills [SKILL_SCRIPT_QUENYAN_TENGWAR] && !ch->skills [SKILL_SCRIPT_ANGERTHAS_MORIA] &&
				!ch->skills [SKILL_SCRIPT_GONDORIAN_TENGWAR] && !ch->skills [SKILL_SCRIPT_ARNORIAN_TENGWAR] &&
				!ch->skills [SKILL_SCRIPT_NUMENIAN_TENGWAR] && !ch->skills [SKILL_SCRIPT_NORTHERN_TENGWAR] &&
				!ch->skills [SKILL_SCRIPT_ANGERTHAS_EREBOR] ) {
				SEND_TO_Q ("You'll need to pick a script to use with your Literacy skill.\n", d);
				skill_display(d);
				return;
			}
		}
		d->connected = CON_CREATION;
		ch->skills [db_race_table[ch->race].race_speaks] = calc_lookup (ch, REG_CAP, db_race_table[ch->race].race_speaks);
		ch->pc->skills [db_race_table[ch->race].race_speaks] = calc_lookup (ch, REG_CAP, db_race_table[ch->race].race_speaks);
		if ( d->character->pc->nanny_state )
			d->character->pc->nanny_state = STATE_COMMENT;
		save_char (ch, FALSE);
		create_menu_options (d);
		return;
	}

	if ( !(skill = pickable_skill (ch, buf)) ) {
		SEND_TO_Q ("Unknown skill.  Please pick one from the list.\n", d);
		skill_display (d);
		return;
	}

	if ( !picks_left && !ch->skills[skill] ) {
		SEND_TO_Q ("#2You've picked too many.  Remove another skill by typing its name.#0\n", d);
		skill_display (d);
		d->connected = CON_SKILLS;
		return;
	}
	else {
		if ( ch->skills [skill] ) {
			ch->skills [skill] = 0;
			ch->pc->skills [skill] = 0;
			save_char (ch, FALSE);
		}
		else {
			ch->skills [skill] = 1;
			ch->pc->skills [skill] = 1;
			save_char (ch, FALSE);
		}
	}

	d->connected = CON_SKILLS;
	skill_display (d);
	
	return;
}

void skill_display (DESCRIPTOR_DATA *d)
{
	int			i = 0;
	int			col = 0;
	int			picks;
	CHAR_DATA	*ch = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	
	ch = d->character;

	SEND_TO_Q ("\n", d);
	temp_arg = get_text_buffer (NULL, text_list, "skill_select");
	SEND_TO_Q (temp_arg, d);

	picks = picks_entitled (d->character);

	*buf = '\0';

	for ( i = 1; i <= LAST_SKILL && *skill_data[i].skill_name != '\n'; i++ ) {
		if ( !pickable_skill (d->character, skill_data[i].skill_name) )
			continue;

		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "    Skill:  %-17s %s", skill_data[i].skill_name, d->character->skills [i] ? "#2(Chosen)#0" : "        ");

		if ( d->character->skills [i] )
			picks -= 1;

		if ( col++ % 2 )
			snprintf (ADDBUF, MAX_STRING_LENGTH, "\n");
	}

	if ( buf[strlen(buf)-1] != '\n' )
		strcat (buf, "\n");

	SEND_TO_Q (buf, d);

	if ( picks > 1 )
		snprintf (buf, MAX_STRING_LENGTH,  "\n%d picks remaining> ", picks);
	else if ( picks == 1 )
		snprintf (buf, MAX_STRING_LENGTH,  "\n1 pick remaining> ");
	else
		snprintf (buf, MAX_STRING_LENGTH,  "\nEnter DONE to finish or skill name> ");

	SEND_TO_Q (buf, d);

	d->connected = CON_SKILLS;
	return;
}

void create_menu_actions (DESCRIPTOR_DATA *d, char *arg)
{
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		key [MAX_STRING_LENGTH] = {'\0'};
	int			i = 0;
	int			picks_left = 0;
	MYSQL_RES	*result;
	CHAR_DATA	*ch = d->character;
	CHAR_DATA	*tch = NULL;
	ROLE_DATA	*role = NULL;
	char		*temp_arg = NULL;

	if ( ch->pc->create_state == STATE_APPROVED ) {
		create_menu_options (d);
		return;
	}

	arg = one_argument (arg, key);

	if ( !*key ) {
		SEND_TO_Q ("> ", d);
		return;
	}
		/* A tiny little bit of cleanup on the keywords.  Make sure
		   that the player's name is in the keyword list */

	if ( ch->name && !name_is (ch->tname, ch->name) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s %s", ch->tname, ch->name);
		ch->name = str_dup (buf);
	}

	else if ( !str_cmp (key, "quit") ) {
		unload_pc (d->character);
		d->character = NULL;
		display_main_menu (d);
		return;
	}

	else if ( !str_cmp (key, "check") ) {

		/* show_level_5_count (d); */

		if ( ch->pc->create_state == STATE_SUBMITTED ) {
			SEND_TO_Q ("No response, as of yet. Please try back later.\n", d);
			SEND_TO_Q ("> ", d);
		}

		else if ( ch->pc->create_state == STATE_APPLYING ) {
			create_menu_options (d);
			return;
		}

		else if ( ch->pc->create_state == STATE_REJECTED ) {
			create_menu_options (d);
			d->connected = CON_CREATION;
			d->character->pc->create_state = STATE_APPLYING;
			return;
		}

		return;
	}

    else if ( !str_cmp (key, "name") ) {
    	SEND_TO_Q ("What would you like to name your character? ", d);
        d->connected = CON_NAME_CONFIRM;
        return;
    }

	else if ( ch->pc->create_state == STATE_SUBMITTED ) {
		SEND_TO_Q ("Only the QUIT and CHECK commands function, now. Please hit ENTER.\n", d);
		return;
	}

	else if ( !str_cmp (key, "review") ) {
		if ( d->character->pc->msg &&
			*d->character->pc->msg != '~' &&
			 d->connected == CON_CREATION ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s\n", d->character->pc->msg);
			SEND_TO_Q (buf, d);
			SEND_TO_Q ("> ", d);
			return;
		}
		else {
			SEND_TO_Q ("\r\nNo response has been filed to your application at this point.\r\n\r\n> ", d);
		}
		return;
	}

	else if ( !str_cmp (key, "sex") ) {
		temp_arg = get_text_buffer (NULL, text_list, "sex_select");
		strcpy (b_buf, temp_arg);
		strcat (b_buf, "Your Character's Gender: ");
		page_string (d, b_buf);
		d->connected = CON_SEX;
		return;
	}

	else if ( !str_cmp (key, "attributes") ) {
		temp_arg = get_text_buffer (NULL, text_list, "qstat_message");
		SEND_TO_Q (temp_arg, d);
		SEND_TO_Q ("Desired Attribute Order: ", d);
		d->connected = CON_ATTRIBUTES;
		return;
	}

	else if ( !str_cmp (key, "keywords") ) {
		SEND_TO_Q ("\n", d);
		temp_arg = get_text_buffer (NULL, text_list, "help_pkeywords");
		SEND_TO_Q (temp_arg, d);
		snprintf (buf, MAX_STRING_LENGTH,  "%s (null)", ch->tname);
		if ( ch->name ) {
			if ( str_cmp (ch->name, buf) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Replacing:  %s\n", ch->name);
				SEND_TO_Q (buf, d);
			}
			ch->name = NULL;
		}

		d->str = &ch->name;
		d->max_str = STR_ONE_LINE;
		if ( d->character->pc->nanny_state ){
			d->character->pc->nanny_state = STATE_LDESC;
		}
		return;
	}

	else if ( !str_cmp (key, "short") ) {
		SEND_TO_Q ("\n", d);
		temp_arg = get_text_buffer (NULL, text_list, "help_psdesc");
		SEND_TO_Q (temp_arg, d);

		if ( ch->short_descr ) {
			if ( str_cmp (ch->short_descr, "(null)") ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Replacing:  %s\n", ch->short_descr);
				SEND_TO_Q (buf, d);
			}
			ch->short_descr = NULL;
		}

		d->str = &ch->short_descr;
		d->max_str = STR_ONE_LINE;
		if ( d->character->pc->nanny_state ){
			d->character->pc->nanny_state = STATE_KEYWORDS;
		}
		return;
	}

	else if ( !str_cmp (key, "long") ) {
		SEND_TO_Q ("\n", d);
		temp_arg = get_text_buffer (NULL, text_list, "help_pldesc");
		SEND_TO_Q (temp_arg, d);

		if ( ch->long_descr ) {
			if ( str_cmp (ch->long_descr, "(null)") ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Replacing:  %s\n", ch->long_descr);
				SEND_TO_Q (buf, d);
			}
			ch->long_descr = NULL;
		}

		if ( d->character->pc->nanny_state ){
			d->character->pc->nanny_state = STATE_FDESC;
		}
		
		d->str = &ch->long_descr;
		d->max_str = STR_ONE_LINE;
		return;
	}

	else if ( !str_cmp (key, "description") ) {
		SEND_TO_Q ("\n", d);
		temp_arg = get_text_buffer (NULL, text_list, "help_pdesc");
		SEND_TO_Q (temp_arg, d);

		if ( ch->description ) {
			if ( str_cmp (ch->description, "(null)") ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Replacing:\n\n%s\n", ch->description);
				SEND_TO_Q (buf, d);
			}
			ch->description = NULL;
		}

        SEND_TO_Q ("> 1----*----10---*----20---*----30---*----40---*----50---*----60---*----70---END\n", d);
		d->str = &ch->description;
		d->max_str = MAX_INPUT_LENGTH;
		
		if ( d->character->pc->nanny_state ){
			d->character->pc->nanny_state = STATE_PROFESSION;
		}
		return;
	}

    else if ( !str_cmp (key, "classifieds") ) {
    	for ( role = role_list; role; role = role->next ) {
			if ( d->character->pc->special_role ) {
				if ( (role->cost - d->character->pc->special_role->cost) <= (d->account->roleplay_points) ){
					break;
				}
			}
			else {
				if ( role->cost <= (d->account->roleplay_points) ){
					break;
				}
			}
		}

		if ( !role ) {
			SEND_TO_Q ("\n#2Sorry, but there are currently no posted roles available to you.#0\n\n> ", d);
			return;
		}

		if ( !d->account->roleplay_points ) {
			SEND_TO_Q ("Sorry, but you must have at least one roleplay point first.\n\n> ", d);
			return;
		}

		SEND_TO_Q ("\n", d);
        temp_arg = get_text_buffer (NULL, text_list, "special_role_select");
        SEND_TO_Q (temp_arg, d);
        *b_buf = '\0';
		
		if ( d->character->pc->special_role ) {
			d->character->pc->special_role = NULL;
		}
        
        for ( role = role_list, i = 1; role; role = role->next, i++ ) {
			if ( role->cost > (d->account->roleplay_points) ) {
				i--;
				continue;
			}
		
			if ( i < 10 ){
        		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%d.  #2%s#0\n", i, role->summary);
        	}			
			else{
				snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "%d. #2%s#0\n", i, role->summary);
			}
		}
        
        strcat (b_buf, "\nYour Desired Role: ");
        page_string (d, b_buf);
        d->connected = CON_SPECIAL_ROLE_SELECT;
        return;
	}

	else if ( !str_cmp (key, "race") ) {
		SEND_TO_Q ("\n", d);
		temp_arg = get_text_buffer (NULL, text_list, "race_select");
		SEND_TO_Q (temp_arg, d);
		*b_buf = '\0';
		if ( d->character->race != -1 ) {
			if ( db_race_table[d->character->race].race_point_reqs  ){
				d->character->pc->app_cost -= MAX(db_race_table[d->character->race].race_point_reqs/2, 1);
			}
			
			d->character->race = -1;
		}
		
		for ( i = 0; i <= MAX_RACE; i++ ) {
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,
				"%d.  #2%-18s#0    %d.  #2%-18s#0    %d.  #2%-18s#0\n",
				i+1, d->account->roleplay_points >= db_race_table [i].race_point_reqs ? db_race_table [i].name : "",
				i+2, d->account->roleplay_points >= db_race_table [i+1].race_point_reqs ? db_race_table [i+1].name : "",
				i+3, d->account->roleplay_points >= db_race_table [i+2].race_point_reqs ? db_race_table [i+2].name : "");
						
			i++;
			i++;
		}
		
		strcat (b_buf, "\nYour Desired Race: ");
		page_string (d, b_buf);
		d->connected = CON_RACE_SELECT;
		return;
	}

	else if ( !str_cmp (key, "age") ) {
    	SEND_TO_Q("\n", d);
        temp_arg = get_text_buffer (NULL, text_list, "age_select");
		SEND_TO_Q (temp_arg, d);
		
		snprintf (b_buf, MAX_STRING_LENGTH, "Your Desired Age (%d to %d): ",
					db_race_table[d->character->race].race_age_limits[0],
					db_race_table[d->character->race].race_age_limits[1]);
		
		SEND_TO_Q (b_buf, d);
		d->connected = CON_AGE;
		return;
	}

	else if ( !str_cmp (key, "location") ) {
		if ( d->character->race != RACE_HUMAN ) {
			SEND_TO_Q ("This option is only available and relevant to Human PCs.\n\n>", d);
			return;
		}
		
		temp_arg = get_text_buffer (NULL, text_list, "location");
		strcpy (b_buf, temp_arg);
		strcat (b_buf, "#2Choose starting location:#0 ");
		page_string (d, b_buf);
		d->connected = CON_LOCATION;		
		return;
	}

	else if ( !str_cmp (key, "profession") ) {
		profession_display(d);
		return;
	}

	else if ( !str_cmp (key, "height") || !str_cmp (key, "frame") ) {
		SEND_TO_Q ("\n", d);
		temp_arg = get_text_buffer (NULL, text_list, "height_frame");
		strcpy (b_buf, temp_arg);
		strcat (b_buf, "#2Choose height and frame:#0 ");
		page_string (d, b_buf);
		d->connected = CON_HEIGHT_WEIGHT;
		return;
	}

	else if ( !str_cmp (key, "skills") ) {
		if ( d->character->race < 0 ) {
			SEND_TO_Q ("You'll need to choose a race first.\n", d);
			create_menu_actions (d, "RACE");
			return;
		}
		skill_display (d);
		return;
	}

	else if ( !str_cmp (key, "comment") ) {
		temp_arg = get_text_buffer (NULL, text_list, "comment_help");
		SEND_TO_Q (temp_arg, d);

		if ( ch->pc->creation_comment ) {
			ch->pc->creation_comment = NULL;
		}

		d->str = &ch->pc->creation_comment;
		d->max_str = MAX_INPUT_LENGTH;
		d->character->pc->nanny_state = 0;
		return;
	}

	else if ( !str_cmp (key, "submit")) {
		picks_left = picks_entitled (ch);
		for ( i = 1; i <= LAST_SKILL && *skill_data[i].skill_name != '\n' && picks_left > 0; i++ ) {
        	if ( ch->skills [i] && pickable_skill (ch, skill_data[i].skill_name) ) {
            	picks_left--;
            }
        }

		if ( picks_left > 0 || picks_left < 0) {
			SEND_TO_Q ("\n#2Type SKILLS at the prompt to choose your PC's starting skills.#0\n\n", d);
			SEND_TO_Q ("> ", d);
			return;
		}

        if ( !ch->pc->profession ) {
        	mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
			
			if (((result = mysql_store_result(database)) != NULL) &&
				( mysql_num_rows (result) > 0)){
				SEND_TO_Q ("\n#2You need to select a profession. Please type PROFESSION to do so.#0\n\n", d);
				SEND_TO_Q ("> ", d);
				mysql_free_result (result);
				return;
			}
			mysql_free_result (result);
        }
  
		if ( !ch->sex ) {
			SEND_TO_Q ("\n#2You need to select a gender. Please type SEX to do so.#0\n\n", d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if ( !ch->height ) {
			SEND_TO_Q ("\n#2You need to select a build. Please type HEIGHT to do so.#0\n\n", d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if ( ch->name && !*ch->name ) {
			SEND_TO_Q ("\n#2You need to supply a keyword list. Please type KEYWORDS to do so.#0\n\n", d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if ( ch->long_descr && !*ch->long_descr ) {
			SEND_TO_Q ("\n#2You need to supply a long description. Please type LONG to do so.#0\n\n", d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if ( ch->long_descr && !*ch->short_descr ) {
			SEND_TO_Q ("\n#2You need to supply a short description. Please type SHORT to do so.#0\n\n", d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if ( ch->long_descr && !*ch->description ) {
			SEND_TO_Q ("\n#2You need to supply a description. Please type DESCRIPTION to do so.#0\n\n", d);
			SEND_TO_Q ("> ", d);
			return;
		}

		if ( ch->pc && ch->pc->creation_comment && !*ch->pc->creation_comment ) {
			SEND_TO_Q ("\n#2You need to supply a background. Please type COMMENT to do so.#0\n\n", d);
			SEND_TO_Q ("> ", d);
			return;
		}

		*ch->tname = UPPER (*ch->tname);
		ch->pc->create_state = STATE_SUBMITTED;
		ch->speaks = db_race_table[ch->race].race_speaks;
		ch->in_room = NOWHERE;
		ch->time.played = 0;

		if ( ch->long_descr ) {
			if ( ch->long_descr[strlen(ch->long_descr)-1] != '.' ){
				strcat (ch->long_descr, ".");
			}
			
			if ( !isupper (*ch->long_descr) ){
				*ch->long_descr = toupper (*ch->long_descr);
			}
			
			snprintf (buf, MAX_STRING_LENGTH,  "%s", ch->long_descr);
			ch->long_descr = tilde_eliminator (buf);
		}

		if ( ch->short_descr ) {
			if ( isupper (*ch->short_descr) ){
				*ch->short_descr = tolower (*ch->short_descr);
			}
			
			if ( ch->short_descr[strlen(ch->short_descr)-1] == '.' ){
				ch->short_descr[strlen(ch->short_descr)-1] = '\0';
			}
			
			snprintf (buf, MAX_STRING_LENGTH,  "%s", ch->short_descr);
			ch->short_descr = tilde_eliminator (buf);
		}

		if ( ch->description ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s", ch->description);
			ch->description = tilde_eliminator (buf);
		}

		if ( ch->name ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s", ch->name);
			ch->name = tilde_eliminator (buf);
		}

		if ( ch->pc->creation_comment ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s", ch->pc->creation_comment);
			ch->pc->creation_comment = tilde_eliminator (buf);
		}

		for ( tch = character_list; tch; tch = tch->next ) {
			if ( tch->deleted )
				continue;

			if ( IS_NPC (tch) )
				continue;
	
			if ( GET_TRUST (tch) > 3 || (is_newbie(ch) && tch->pc->is_guide && !IS_SET (ch->plr_flags, PRIVATE) ) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "%s has submitted an application for a new character: %s.\n",
				ch->pc->account, ch->tname);
				send_to_char (buf, tch);
			}
		}

		if ( ch->pc->app_cost < 0 ){
			ch->pc->app_cost = 0;
		}

		SEND_TO_Q ("\nThank you.  Your character application has been submitted.\n"
		           "An administrator may be reviewing your application soon.\n", d);
		SEND_TO_Q ("You will receive an email at the address registered for this\n"
		           "account when the application has been reviewed, along with any\n"
			   "comments the reviewing administrator wished to make.\n"
			   "\n"
			   "Character review generally takes anywhere from 24-48 hours,\n"
			   "depending on the workload of our roleplay admins. We thank you\n"
			   "in advance for your patience, and for your interest in Shadows\n"
			   "of YourMud! We'll see you soon.\n", d);
		
		d->character->time.birth = time(0);
		d->character->time.played = 0;
		d->character->time.logon = time(0);
		display_main_menu (d);
		save_char (d->character, FALSE);
		unload_pc (d->character);
		d->character = NULL;
		return;
	}

	else{
		SEND_TO_Q ("Unknown keyword!\n", d);
	}
	
	create_menu_options (d);
	
	return;
}

void nanny (DESCRIPTOR_DATA *d, char *argument)
{
	switch ( d->connected ) {
		
		case CON_LOGIN:
			nanny_login_choice (d, argument);
			break;
		
		case CON_NEW_ACCT_NAME:
			nanny_new_account (d, argument);
			break;
		
		case CON_ACCT_POLICIES:
			nanny_account_policies (d, argument);
			break;
		
		case CON_REFERRAL:
			nanny_account_referral (d, argument);
			break;
		
		case CON_ACCT_EMAIL:
			nanny_account_email (d, argument);
			break;
		
		case CON_EMAIL_CONFIRM:
			nanny_account_email_confirm (d, argument);
			break;
		
		case CON_ACCOUNT_SETUP:
			nanny_account_setup (d, argument);
			break;
		
		case CON_ENTER_ACCT_NME:
			nanny_ask_password (d, argument);
			break;
		
		case CON_PWDCHK:
			nanny_check_password (d, argument);
			break;
		
		case CON_PENDING_DISC:
			nanny_disconnect (d, argument);
			break;
		
		case CON_ACCOUNT_MENU:
			nanny_connect_select (d, argument);
			break;
		
		case CON_PWDNEW:
			nanny_change_password (d, argument);
			break;
		
		case CON_PWDGET:
			nanny_new_password (d, argument);
			break;
		
		case CON_PWDNCNF:
			nanny_conf_change_password (d, argument);
			break;
		
		case CON_CHG_EMAIL:
			nanny_change_email (d, argument);
			break;
		
		case CON_CHG_EMAIL_CNF:
			nanny_change_email_confirm (d, argument);
			break;
		
		case CON_DELETE_PC:
			nanny_delete_pc (d, argument);
			break;
		
		case CON_CHOOSE_PC:
			nanny_choose_pc (d, argument);
			break;
		
		case CON_VIEW_PC:
			nanny_view_pc (d, argument);
			break;
		
		case CON_READING_WAIT:
			nanny_reading_wait (d, argument);
			break;
		
		case CON_RACE_CONFIRM:
			nanny_race_confirm (d, argument);
			break;
		
		case CON_PRIV_CONFIRM:
			nanny_privacy_confirm (d, argument);
			break;
		
		case CON_NAME_CONFIRM:
			nanny_char_name_confirm (d, argument);
			break;
		
		case CON_TERMINATE_CONFIRM:
			nanny_terminate (d, argument);
			break;
		
		case CON_RETIRE:
			nanny_retire (d, argument);
			break;
		
		case CON_RACE_SELECT:
			race_selection (d, argument);
			break;
		
		case CON_SPECIAL_ROLE_SELECT:
			nanny_special_role_selection (d, argument);
			break;
		
		case CON_SPECIAL_ROLE_CONFIRM:
			nanny_special_role_confirm (d, argument);
			break;
		
		case CON_CREATE_GUEST:
			nanny_create_guest (d, argument);
			break;
		
		case CON_GUEST_ARRIVE:
			nanny_guest_arrive (d, argument);
			break;
		
		case CON_MAIL_MENU:
			nanny_mail_menu (d, argument);
			break;
		
		case CON_COMPOSE_MAIL_TO:
			nanny_compose_mail_to (d, argument);
			break;
		
		case CON_COMPOSE_SUBJECT:
			nanny_compose_subject (d, argument);
			break;
		
		case CON_COMPOSE_MESSAGE:
			nanny_compose_message (d, argument);
			break;
		
		case CON_COMPOSING_MESSAGE:
			nanny_composing_message (d, argument);
			break;
		
		case CON_READ_MESSAGE:
			nanny_read_message (d, argument);
			break;

		case CON_PLAYER_NEW:
				d->connected = CON_CREATION;
				create_menu_options (d);
				break;

		case CON_RACE:
				d->connected = CON_CREATION;
				create_menu_options (d);
				break;

		case CON_AGE:
				age_selection (d, argument);
				if ( age(d->character).year ) {
					d->character->pc->nanny_state = STATE_ATTRIBUTES;
					d->connected = CON_CREATION;
				}
				else d->connected = CON_AGE;
				create_menu_options (d);
				break;

		case CON_HEIGHT_WEIGHT:
				height_frame_selection (d, argument);
				create_menu_options (d);
				break;

		case CON_LOCATION:
				location_selection (d, argument);
				create_menu_options (d);
				break;

		case CON_PROFESSION:
				profession_selection (d, argument);
				break;

		case CON_SKILLS:
				skill_selection (d, argument);
				break;

		case CON_SEX:
				sex_selection (d, argument);
				if ( d->character->pc->nanny_state && d->character->sex ) {
					if ( !available_roles (d->account->roleplay_points) )
						d->character->pc->nanny_state = STATE_RACE;
					else d->character->pc->nanny_state = STATE_SPECIAL_ROLES;
					d->connected = CON_CREATION;
				}
				create_menu_options (d);
				break;

		case CON_ATTRIBUTES:
				attribute_priorities (d, argument);
				if ( d->character->pc->nanny_state && d->character->str ) {
					d->character->pc->nanny_state = STATE_FRAME;
				}
				d->connected = CON_CREATION;
				create_menu_options (d);
				break;

		case CON_CREATION:
				create_menu_actions (d, argument);
				break;

		case CON_WEB_CONNECTION:
	/*			web_process (d, argument); */
				break;
	}
	
	return;
}


void email_acceptance (DESCRIPTOR_DATA *d)
{
	ACCOUNT_DATA	*account = NULL;
	CHAR_DATA	*tch = NULL;
	char		*date = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		email [MAX_STRING_LENGTH] = {'\0'};

	date = timestr(date);
	
	tch = NULL;
	tch = load_pc (d->character->delay_who2);
	if ( !tch )
		return;

	account = NULL;
	account = load_account (tch->pc->account);
	if ( !account )
		return;

	if ( !*d->character->pc->msg ) {
		send_to_char ("No email sent!\n", d->character);
		mem_free (date);
		if ( tch )
			unload_pc (tch);
		return;
	}
	else if ( !tch || !account ) {
		send_to_char ("\nEmail response aborted; there was a problem loading this PC.\n", d->character);
		mem_free (date);
		if ( tch )
			unload_pc (tch);
		return;
	}
	else {
		if ( d->character->pc && IS_MORTAL(d->character) && d->character->pc->is_guide ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Greetings,\n"
			              "\n"
			              "   Thank you for your interest in %s! This is an automated\n"
				      "system notification sent to inform you that your application for a character\n"
				      "named %s has been ACCEPTED by a Guide, and that you may enter\n"
				      "YourWorld at your earliest convenience. We'll see you there!\n"
				      "\n"
				      "%s left the following comments regarding your application:\n"
				      "\n%s", MUD_NAME, tch->tname, d->character->pc->account, d->character->pc->msg);
			account = load_account (tch->pc->account);
			snprintf (email, MAX_STRING_LENGTH, "%s Player Guide <%s>", MUD_NAME, d->account->email);
			send_email (account, NULL, email, "Your Character Application", buf);
		}
		else {		
			snprintf (buf, MAX_STRING_LENGTH,  "Greetings,\n"
			              "\n"
			              "   Thank you for your interest in %s! This is an automated\n"
				      "system notification sent to inform you that your application for a character\n"
				      "named %s has been ACCEPTED by the reviewer, and that you may enter\n"
				      "YourWorld at your earliest convenience. We'll see you there!\n"
				      "\n"
				      "%s left the following comments regarding your application:\n"
				      "\n%s", MUD_NAME, tch->tname, d->character->tname, d->character->pc->msg);
			snprintf (email, MAX_STRING_LENGTH, "%s <%s>", MUD_NAME, MUD_EMAIL);
			send_email (account, NULL, email, "Your Character Application", buf);
		}

		d->pending_message = malloc (sizeof (MESSAGE_DATA));
		d->pending_message->virtual = 0;
		d->pending_message->info = add_hash ("");
		snprintf (buf, MAX_STRING_LENGTH,  "#2Accepted:#0 %s", tch->tname);
		d->pending_message->subject = add_hash(buf);
		d->pending_message->message = add_hash(d->character->pc->msg);

		add_message ("Applications",
			        2,
					d->account->name,
					date,
					d->pending_message->subject,
					d->pending_message->info,
					d->pending_message->message,
					d->pending_message->flags);

       add_message (tch->tname,
       				3,
                    d->account->name,
                    date,
                    "Application Acceptance",
                    d->pending_message->info,
                    d->pending_message->message,
                    d->pending_message->flags);

		unload_message (d->pending_message);
		if ( d->character->pc->msg ) {
			mem_free (d->character->pc->msg);
			d->character->pc->msg = NULL;
		}
		d->pending_message = NULL;

		mysql_safe_query ("UPDATE newsletter_stats SET accepted_apps=accepted_apps+1");

		mysql_safe_query ("UPDATE professions SET picked=picked+1 WHERE id=%d", tch->pc->profession);

		if ( tch->pc->special_role ) {
			snprintf (email, MAX_STRING_LENGTH, "%s Player <%s>", MUD_NAME, account->email);
			free_account (account);
			account = load_account (tch->pc->special_role->poster);
			if ( account ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Hello,\n\n"
				"This email is being sent to inform you that a special role you placed\n"
				"in chargen has been applied for and accepted. The details are attached.\n"
				"\n"
				"Please contact this player at your earliest convenience to arrange a time\n"
				"to set up and integrate their new character. To do so, simply click REPLY;\n"
				"their email has been listed in this message as the FROM address.\n\n"
				"Character Name: %s\n"
				"Role Description: %s\n"
				"Role Post Date: %s\n"
				"\n"
				"%s\n\n", tch->tname, tch->pc->special_role->summary, tch->pc->special_role->date, tch->pc->special_role->body);
				snprintf (buf2, MAX_STRING_LENGTH,  "New PC: %s", tch->pc->special_role->summary);
				send_email (account, STAFF_EMAIL, email, buf2, buf);
				free_account (account);
			}
		} else free_account (account);

	}

	d->character->delay_who = add_hash(tch->tname);
	if ( d->character->delay_who2 ) {
		mem_free (d->character->delay_who2);
		d->character->delay_who2 = NULL;
	}
	d->character->delay_ch = NULL;

	mem_free (date);

	unload_pc (tch);
	
	return;
}

void email_rejection (DESCRIPTOR_DATA *d)
{
	ACCOUNT_DATA	*account = NULL;
	CHAR_DATA	*tch = NULL;
	char		*date = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		email [MAX_STRING_LENGTH] = {'\0'};

	date = timestr(date);
		
	tch = NULL;
	tch = load_pc (d->character->delay_who2);
	if ( !tch )
		return;

	account = NULL;
	account = load_account (tch->pc->account);
	if ( !account )
		return;

	if ( !*d->character->pc->msg ) {
		send_to_char ("No email sent!\n", d->character);
		mem_free (date);
		if ( tch )
			unload_pc (tch);
		return;
	}
	else if ( !tch || !account ) {
		send_to_char ("\nEmail response aborted; there was a problem loading this PC.\n", d->character);
		mem_free (date);
		if ( tch )
			unload_pc (tch);
		return;
	}
	else {
		if ( d->character->pc && IS_MORTAL (d->character) && d->character->pc->is_guide ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Greetings,\n"
			              "\n"
				      "   Thank you for your interest in %s! This is an\n"
				      "automated system notification to inform you that your application for\n"
				      "a character named %s was deemed inappropriate by a Guide, and\n"
				      "therefore was declined. However, don't despair! This is a relatively\n"
				      "common occurrence, and nothing to worry about. Your application has\n"
				      "been saved on our server, and you may make the necessary changes simply\n"
				      "by entering the game  as that character. You will be dropped back\n"
				      "into the character generation engine, where you may make corrections.\n"
				      "\n"
				      "   If you have any questions regarding the comments below, clicking\n"
				      "REPLY to this email will allow you to get in touch with the Guide\n"
				      "who reviewed your application. Please be civil if you do choose to\n"
				      "contact them - remember, they volunteer their free time to help you!\n\n"
				      "%s left the following comments regarding your application:\n"
				      "\n%s", MUD_NAME, tch->tname, d->character->pc->account, d->character->pc->msg);
			account = load_account (tch->pc->account);
			snprintf (email, MAX_STRING_LENGTH, "%s Player Guide <%s>", MUD_NAME, d->account->email);
			send_email (account, NULL, email, "Your Character Application", buf);
		}
		else {		
			snprintf (buf, MAX_STRING_LENGTH,  "Greetings,\n"
			              "\n"
				      "   Thank you for your interest in %s! This is an\n"
				      "automated system notification to inform you that your application for\n"
				      "a character named %s was deemed inappropriate by an administrator, and\n"
				      "therefore was declined. However, don't despair! This is a relatively\n"
				      "common occurrence, and nothing to worry about. Your application has\n"
				      "been saved on our server, and you may make the necessary changes simply\n"
				      "by 'Entering YourWorld' as that character. You will be dropped back\n"
				      "into the character generation engine, where you may make corrections.\n"
				      "\n"
				      "%s left the following comments regarding your application:\n"
				      "\n%s", MUD_NAME, tch->tname, d->character->tname, d->character->pc->msg);
			snprintf (email, MAX_STRING_LENGTH, "%s <%s>", MUD_NAME, MUD_EMAIL);
			send_email (account, NULL, email, "Your Character Application", buf);
		}

		d->pending_message = malloc (sizeof (MESSAGE_DATA));
		d->pending_message->poster = add_hash ("player_applications");
		d->pending_message->virtual = 0;
		d->pending_message->info = add_hash ("");
		snprintf (buf, MAX_STRING_LENGTH,  "#1Declined:#0 %s", tch->tname);
		d->pending_message->subject = add_hash(buf);
		d->pending_message->message = add_hash(d->character->pc->msg);

                add_message ("Applications",
                           	2,
							d->account->name,
            	            date,
                            d->pending_message->subject,
                            d->pending_message->info,
                            d->pending_message->message,
                            d->pending_message->flags);

                unload_message (d->pending_message);

		snprintf (buf, MAX_STRING_LENGTH,  "\n#6Unfortunately, your application was declined on its most recent review.\n\n%s left the following comment(s) explaining why:#0\n"
			      "\n%s", d->account->name, d->character->pc->msg);
		if ( buf [strlen(buf)-1] != '\n' )
			strcat (buf, "\n");

		tch->pc->msg = add_hash (buf);

		mysql_safe_query ("UPDATE newsletter_stats SET declined_apps=declined_apps+1");
	}

	d->character->delay_who = add_hash(tch->tname);
	if ( d->character->delay_who2 ) {
		mem_free (d->character->delay_who2);
		d->character->delay_who2 = NULL;
	}
	d->character->delay_ch = NULL;

	mem_free (date);

	save_char (tch, FALSE);
	unload_pc (tch);
	
	return;
}


void starting_skill_boost (CHAR_DATA *ch, int skill)
{
	ch->skills [skill] += MIN(30,age(ch).year)/2 + number(5,15);
}

void setup_new_character (CHAR_DATA *tch)
{
	int			i = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	AFFECTED_TYPE		*af = NULL;

	/*	Add in auto-selected skills here, as well as psi-check 		*/
	tch->speed = SPEED_WALK;

	tch->pc->create_state = STATE_APPROVED;

	tch->str	= tch->pc->start_str;
	tch->dex	= tch->pc->start_dex;
	tch->intel	= tch->pc->start_intel;
	tch->wil	= tch->pc->start_wil;
	tch->aur	= tch->pc->start_aur;
	tch->con	= tch->pc->start_con;
	tch->agi	= tch->pc->start_agi;

	tch->str   += db_race_table [tch->race].attr_mods [0];
	tch->dex   += db_race_table [tch->race].attr_mods [1];
	tch->con   += db_race_table [tch->race].attr_mods [2];
	tch->wil   += db_race_table [tch->race].attr_mods [3];
	tch->intel += db_race_table [tch->race].attr_mods [4];
	tch->aur   += db_race_table [tch->race].attr_mods [5];
	tch->agi   += db_race_table [tch->race].attr_mods [6];

	tch->tmp_str		= tch->str;
	tch->tmp_con		= tch->con;
	tch->tmp_intel		= tch->intel;
	tch->tmp_wil		= tch->wil;
	tch->tmp_aur		= tch->aur;
	tch->tmp_dex		= tch->dex;
	tch->tmp_agi		= tch->agi;

	tch->max_hit  = 10 + 6 * GET_CON (tch);
	tch->max_move = calc_lookup (tch, REG_MISC, MISC_MAX_MOVE);
	tch->hit      = GET_MAX_HIT (tch);
	tch->move     = GET_MAX_MOVE (tch);

	tch->skills [db_race_table [tch->race].race_speaks] = calc_lookup (tch, REG_CAP, db_race_table[tch->race].race_speaks);
	tch->pc->skills [db_race_table [tch->race].race_speaks] = calc_lookup (tch, REG_CAP, db_race_table[tch->race].race_speaks);

			/* Height */

	make_height (tch, tch->height);
	tch->height += number (1, 8);

	/** frame was chosedn during chargen **/
	
	open_skill (tch, SKILL_SEARCH);
	starting_skill_boost (tch, SKILL_SEARCH);
		
	open_skill (tch, SKILL_LISTEN);
	starting_skill_boost (tch, SKILL_LISTEN);

	open_skill (tch, SKILL_SCAN);
	starting_skill_boost (tch, SKILL_SCAN);

	open_skill (tch, SKILL_DODGE);
	starting_skill_boost (tch, SKILL_DODGE);

	open_skill (tch, SKILL_PARRY);
	starting_skill_boost (tch, SKILL_PARRY);

	open_skill (tch, SKILL_BLOCK);
	starting_skill_boost (tch, SKILL_BLOCK);

	open_skill (tch, SKILL_BRAWLING);
	starting_skill_boost (tch, SKILL_BRAWLING);

	open_skill (tch, SKILL_CLIMB);
	starting_skill_boost (tch, SKILL_CLIMB);

	open_skill (tch, SKILL_SWIMMING);
	starting_skill_boost (tch, SKILL_SWIMMING);

	check_psionic_talents (tch);

	for ( i = 1; i <= LAST_SKILL; i++ ) {
		if ( tch->skills [i] == 1 ) {
			open_skill (tch, i);
			starting_skill_boost (tch, i);
		}
	}

	for ( i = 1; i <= LAST_SKILL; i++ )
		tch->pc->skills [i] = tch->skills [i];

	fix_offense (tch);

	tch->fight_mode = 2;

	tch->pc->nanny_state = 0;

	reformat_desc (tch->description, &tch->description);

	SET_BIT (tch->plr_flags, NEWBIE);
	SET_BIT (tch->plr_flags, NEWBIE_HINTS);
	SET_BIT (tch->plr_flags, NEW_PLAYER_TAG);

	tch->time.played = 0;

	snprintf (buf, MAX_STRING_LENGTH,  "save/objs/%c/%s", tolower(*tch->tname), tch->tname);
	unlink (buf);

	for ( craft = crafts; craft; craft = craft->next ) {
		if ( (!strcmp (craft->craft_name, "general") && has_required_crafting_skills (tch, craft)) ||
			(has_required_crafting_skills (tch, craft) && is_opening_craft (tch, craft)) ) {
			for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ )
				if ( !get_affect (tch, i) )
					break;
			magic_add_affect (tch, i, -1, 0, 0, 0, 0);
			af = get_affect (tch, i);
			af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);
			af->a.craft->subcraft = craft;
		}	
	}

	tch->in_room = NOWHERE;
	return;
}

void answer_application (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA		*tch = NULL;
	CHAR_DATA		*tmp_ch = NULL;
	char			buf [MAX_INPUT_LENGTH] = {'\0'};
	char			name [MAX_INPUT_LENGTH] = {'\0'};
	long int	time_elapsed = 0;

	argument = one_argument(argument, buf);

	if ( !ch->delay_who || !*ch->delay_who ) {
		send_to_char ("Please REVIEW an application first.\n", ch);
		return;
	}

	if ( ch->pc->msg ) {
		mem_free (ch->pc->msg);
		ch->pc->msg = NULL;
	}

	snprintf (name, MAX_STRING_LENGTH, "%s", ch->delay_who);

	mem_free (ch->delay_who);
	ch->delay_who = NULL;
	ch->delay = 0;

	if ( !(tch = load_pc (name)) ) {
		send_to_char ("Couldn't find PC...maybe a pfile corruption?\n\r", ch);
		return;
	}

	while ( tch->pc->load_count > 1 ){
		unload_pc (tch);
	}

	for ( tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next ) {
		if ( tmp_ch->deleted )
			continue;
		
		if ( tmp_ch->pc &&
			 tmp_ch->pc->edit_player &&
			 !str_cmp (tmp_ch->pc->edit_player->tname, tch->tname) ) {
			send_to_char ("The PC in your editor has been closed for an application response.\n", tmp_ch);
			tmp_ch->pc->edit_player = NULL;
		}
	}

	if ( tch->pc->create_state != 1 ) {
		snprintf (buf, MAX_STRING_LENGTH,  "It appears that this application has already been reviewed and responded to.\n");
		send_to_char (buf, ch);
		unload_pc (tch);
		return;
	}

	ch->delay_who = add_hash(tch->pc->account);
	ch->delay_who2 = add_hash(tch->tname);

	if ( cmd != 345 ) {
		send_to_char ("\n#2Please enter the changes necessary for this application to be approved.\n#0", ch);
		send_to_char ("#2This will be sent via email to the player; terminate with an '@' when finished.#0\n", ch);
		make_quiet (ch);
		ch->desc->str = &ch->pc->msg;
		ch->desc->max_str = MAX_STRING_LENGTH;
		ch->desc->proc = email_rejection;
	}

	else if ( cmd == 345 ) {
		send_to_char ("\n#2Please enter any comments or advice you have  regarding this approved character.\n#0", ch);
		send_to_char ("#2This will be sent via email to the player; terminate with an '@' when finished.#0\n", ch);
		make_quiet (ch);
		ch->desc->str = &ch->pc->msg;
		ch->desc->max_str = MAX_STRING_LENGTH;
		ch->desc->proc = email_acceptance;
	}

	ch->pc->msg = NULL;

	if ( tch->pc->create_state > STATE_SUBMITTED ) {
		snprintf (buf, MAX_STRING_LENGTH,  "PC is currently in create state %d.\n\r", tch->pc->create_state);
		send_to_char (buf, ch);
		unload_pc (tch);
	}

/*  Acceptance; process the new PC for entry into the game */

	if ( cmd == 345 )
		setup_new_character (tch);

	else if ( cmd != 345 )
		tch->pc->create_state = STATE_REJECTED;

	time_elapsed = time(0) - tch->time.birth;
	mysql_safe_query ("INSERT INTO application_wait_times VALUES (%d)", (int)time_elapsed);

	mysql_safe_query ("DELETE FROM reviews_in_progress WHERE char_name = '%s'", tch->tname);

	unload_pc (tch);

	return;
}

void menu_choice (DESCRIPTOR_DATA *d)
{
		char		*temp_arg = NULL;

	if ( !maintenance_lock ){
		temp_arg = get_text_buffer (NULL, text_list, "greetings");
		SEND_TO_Q (temp_arg, d);
	}
	else{
		temp_arg = get_text_buffer (NULL, text_list, "greetings.maintenance");
		SEND_TO_Q (temp_arg, d);
	}
	SEND_TO_Q ("Your Selection: ", d);
	d->connected = CON_LOGIN;
		
	return;
}
