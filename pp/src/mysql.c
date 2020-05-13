/*------------------------------------------------------------------------\
|  mysql.c : mySQL Interface Module                   www.yourmud.org | 
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  All original code, derived under license from DIKU GAMMA (0.0).        |
\------------------------------------------------------------------------*/

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>
#include <mysql/mysql.h>
#include <dirent.h>
#include <signal.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

extern MYSQL *database;

MYSQL *database;

void init_mysql (void)
{
	database = mysql_init (database);

	if ( !(mysql_real_connect (database, MYSQL_HOST, MYSQL_USERNAME, MYSQL_PASS, PRIMARY_DATABASE, 0, MYSQL_SOCK, 0)) )       
		abort();

	system_log ("MySQL connection initialized.", FALSE);
	
	return;
}

void refresh_db_connection (void)
{
	mysql_close (database);

	init_mysql();
	
	return;
}

/* Route ALL mysql queries through this wrapper to ensure they are escaped
 properly, to thwart various SQL injection attacks. */

int mysql_safe_query (char *fmt, ...)
{
	va_list         argp;
	int             intdex = 0;
	double          doudex = 0;
	char            *var_s = 0, *out = 0, *var_p = 0;
	char            safe [MAX_STRING_LENGTH] = {'\0'};
	char            query [MAX_STRING_LENGTH] = {'\0'};

	va_start (argp, fmt);

	for ( var_p = fmt, out = query; *var_p != '\0'; var_p++ ) {
		if ( *var_p != '%' ) {
			*out++ = *var_p;
			continue;
		}

		switch ( *++var_p ) {
			case 'c':
				intdex = va_arg (argp, int);
				out += snprintf (out, MAX_STRING_LENGTH,"%c", intdex);

				break;

			case 's':
				var_s = va_arg (argp, char *);
				if ( !var_s ) {
					out += snprintf (out, MAX_STRING_LENGTH," ");

					break;
				}
				mysql_real_escape_string (database, safe, var_s, strlen(var_s));
				out += snprintf (out, MAX_STRING_LENGTH,"%s", safe);

				*safe = '\0';
				break;

			case 'd':
				intdex = va_arg (argp, int);
				out += snprintf (out, MAX_STRING_LENGTH,"%d", intdex);

				break;

			case 'f':
				doudex = va_arg (argp, double);
				out += snprintf (out, MAX_STRING_LENGTH,"%f", doudex);

				break;

			case '%':
				out += snprintf (out, MAX_STRING_LENGTH,"%%");
				break;
		}
	}

	*out = '\0';

	va_end (argp);

	return (mysql_real_query (database, query, strlen(query)));
}

MYSQL_RES *mysql_player_search (int search_type, char *string)
{
	char			query [MAX_STRING_LENGTH];
	static MYSQL_RES	*result;

	snprintf (query, MAX_STRING_LENGTH,"SELECT * FROM %s.pfiles WHERE ", PFILES_DATABASE);

	if ( search_type == SEARCH_KEYWORD )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "keywords LIKE '%%%s%%'", string);
	else if ( search_type == SEARCH_SDESC )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "sdesc LIKE '%%%s%%'", string);
	else if ( search_type == SEARCH_LDESC )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "ldesc LIKE '%%%s%%'", string);
	else if ( search_type == SEARCH_FDESC )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "description LIKE '%%%s%%'", string);
	else if ( search_type == SEARCH_CLAN )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "clans LIKE '%%%s%%'", string);
	else if ( search_type == SEARCH_SKILL )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "skills LIKE '%%%s%%'", string);
	else if ( search_type == SEARCH_ROOM )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "room = %s", string);
	else if ( search_type == SEARCH_LEVEL )
		snprintf (query + strlen(query), MAX_STRING_LENGTH, "level = %s", string);
	else return NULL;

	snprintf (query + strlen(query), MAX_STRING_LENGTH, " ORDER BY name ASC");

	/* Again, use real_query() here rather than safe_query() because the latter is impractical and this command is for staff use only. */

	mysql_real_query (database, query, strlen(query));

	result = mysql_store_result (database);

	return result;
}


void system_log (char *str, bool error)
{
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	char	sha_buf [MAX_STRING_LENGTH] = {'\0'};
	int		timestamp = 0;

	if ( !str || !*str )
		return;

	mysql_escape_string (buf, str, strlen(str));
 
	if ( buf[strlen(buf)-1] == '\n' )
		buf[strlen(buf)-1] = '\0';

	timestamp = (int) time(0);

	snprintf (sha_buf, MAX_STRING_LENGTH, "%d %s", timestamp, buf);

	mysql_safe_query ("INSERT INTO %s.mud (name, timestamp, port, room, error, entry, sha_hash) "
			"VALUES (AES_ENCRYPT('System','%s'), %d, %d, -1, %d, AES_ENCRYPT('%s','%s'), SHA('%s'))", 
			LOG_DATABASE, CRYPT_PASS, timestamp, port, (int) error, buf, CRYPT_PASS, sha_buf);
			
	return;
}

void player_log (CHAR_DATA *ch, char *command, char *str)
{
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	char	sha_buf [MAX_STRING_LENGTH] = {'\0'};
	int	timestamp = 0;

	if ( !str || !*str || !command || !*command || !ch || ch->deleted )
		return;

	mysql_escape_string (buf, str, strlen(str));

	if ( buf[strlen(buf)-1] == '\n' )
		buf[strlen(buf)-1] = '\0';

	timestamp = (int) time(0);

	snprintf (sha_buf, MAX_STRING_LENGTH, "%d %s %s", timestamp, command, buf);

	mysql_safe_query ("INSERT INTO %s.mud (name, account, switched_into, timestamp, port, room, guest, immortal, command, entry, sha_hash) "
			"VALUES (AES_ENCRYPT('%s','%s'), AES_ENCRYPT('%s','%s'), AES_ENCRYPT('%s','%s'), %d, %d, %d, %d, %d, AES_ENCRYPT('%s','%s'), AES_ENCRYPT('%s','%s'), SHA('%s'))", 
			LOG_DATABASE, ch->desc && ch->desc->original ? ch->desc->original->tname : ch->tname, CRYPT_PASS,
			ch->pc && ch->pc->account ? ch->pc->account : "", CRYPT_PASS, ch->desc && ch->desc->original ? 
			ch->tname : "", CRYPT_PASS, timestamp, port, ch->in_room, IS_SET (ch->flags, FLAG_GUEST) ? 1 : 0, 
			GET_TRUST (ch) > 0 && !IS_NPC(ch) ? 1 : 0, command, CRYPT_PASS, buf, CRYPT_PASS, sha_buf);

	/* feed to stdout for any on-server, realtime monitoring */

	printf ("%s [%d]: %s %s\n", ch->tname, ch->in_room, command, str);

	fflush (stdout);
	
	return;
}

void add_profession_skills (CHAR_DATA *ch, char *skill_list)
{
	int		ind = 0;
	char	buf [MAX_STRING_LENGTH] = {'\0'};

	while ( *skill_list ) {
		skill_list = one_argument (skill_list, buf);
		if ( (ind = skill_index_lookup (buf)) == -1 )
			continue;
		ch->skills [ind] = 1;
		ch->pc->skills [ind] = 1;
	}
	
	return;
}

/*                                                                          *
 * function: get_profession_name                                            *
 *                                                                          *
 *                                                                          */
char *get_profession_name (int prof_id)
{
        MYSQL_RES       *result;
        MYSQL_ROW       row;
	char		buf [MAX_STRING_LENGTH];
	static char	prof [MAX_STRING_LENGTH];
	int         nProfessions = 0;

	snprintf (prof, MAX_STRING_LENGTH, "None");

	mysql_safe_query ("SELECT name FROM professions WHERE id = %d", prof_id);

	if ( ( result = mysql_store_result (database) ) == NULL ) {
          snprintf (buf, MAX_STRING_LENGTH, "Warning: get_profession_name(): %s", mysql_error ( database ) );
	system_log (buf, TRUE);
	}
	else {
	  nProfessions = mysql_num_rows (result);
	  row = mysql_fetch_row(result);
	  if ( nProfessions > 0 && row != NULL ) {
	    snprintf (prof, MAX_STRING_LENGTH, "%s", row[0]);
	  }
	  mysql_free_result (result); result = NULL;
	}

	return prof;
}

int is_restricted_profession (CHAR_DATA *ch, char *skill_list)
{
	int		ind = 0;
	char	buf [MAX_STRING_LENGTH] = {'\0'};

	while ( *skill_list ) {
		skill_list = one_argument (skill_list, buf);
		if ( (ind = skill_index_lookup (buf)) == -1 )
			continue;
		if ( (is_restricted_skill (ch, ind)) &&
			  skill_data[ind].skill_type != 3 )
			return 1;
	}

	return 0;
}

/*                                                                          *
 * function: do_professions                                                 *
 *                                                                          *
 *                                                                          */
void do_professions (CHAR_DATA *ch, char *argument, int cmd)
{
	MYSQL_RES	*result = NULL;
    MYSQL_ROW	row;
    char		buf [MAX_STRING_LENGTH] = {'\0'};
    char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	char		skill_list [MAX_STRING_LENGTH] = {'\0'};
    char		profession [MAX_STRING_LENGTH] = {'\0'};
 	int			ind = 0;
 	int			i = 1;
 	int			skill_num = 0;
	int			nProfessions = 0;

        argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "delete") ) {
		if ( !*argument ) {
			send_to_char ("Please include the name of the profession you wish to delete.\n", ch);
			return;
		}

		argument = one_argument (argument, buf);

		mysql_safe_query ("DELETE FROM professions WHERE name = '%s'", buf);

		nProfessions = mysql_affected_rows ( database ) ;
		if ( nProfessions ) {
		  snprintf ( buf2, MAX_STRING_LENGTH, "%d profession%s matching the name '%s' %s deleted.\n", nProfessions, ( nProfessions == 1 ) ? "" : "s", buf, ( nProfessions == 1 ) ? "was" : "were");
		  send_to_char (buf2, ch);
		  return;
		}
		snprintf ( buf2, MAX_STRING_LENGTH, "No professions matching the name '%s' were found.\n", buf);
		send_to_char (buf2,ch);
		return;
	}

	if ( !str_cmp (buf, "list") ) {
		mysql_safe_query ("SELECT * FROM professions ORDER BY name ASC");
		if ( ( result = mysql_store_result (database) ) != NULL ) {
		  if ( mysql_num_rows( result ) > 0 ) {
		    snprintf (b_buf, MAX_STRING_LENGTH, "#2Currently Defined Professions:#0\n\n");
		    while ( (row = mysql_fetch_row(result)) ) {
		      if ( i < 10 )
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "   %d.  #2%15s#0: [%4d] %s\n", i, row[0], (int)strtol(row[3], NULL, 10), row[1]);
		      else snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "   %d. #2%15s#0: [%4d] %s\n", i, row[0], (int)strtol(row[3], NULL, 10), row[1]);
		      i++;
		    }
		    page_string (ch->desc, b_buf);
		  }
		  else {
		    send_to_char ("There are no professions currently in the database.\n", ch);
		  }
		  mysql_free_result ( result ) ; result = NULL ;
		  return;
		}
		else {
		  snprintf (buf, MAX_STRING_LENGTH, "Warning: do_professions(): %s", mysql_error ( database ) );
		system_log (buf, TRUE);
		}
		send_to_char ("An error occurred while trying to list professions.\n", ch);
		return;
	}

	if ( !str_cmp (buf, "add") ) {

		if ( !*argument ) {
			send_to_char ("Add which profession?\n", ch);
			return;
		}

		argument = one_argument (argument, profession);

		if ( !*argument ) {
			send_to_char ("Which skills should this profession start with?\n", ch);
			return;
		}

		*skill_list = '\0';

		while ( *argument ) {
			skill_num++;
			if ( skill_num > 6 ) {
				send_to_char ("You can only define a set of 6 skills per profession.\n", ch);
				return;
			}
			argument = one_argument (argument, buf);
			if ( (ind = skill_index_lookup (buf)) == -1 ) {
				snprintf (buf2, MAX_STRING_LENGTH,  "I couldn't find the '%s' skill in our database. Aborted.\n", buf);
				send_to_char (buf2, ch);
				return;
			}
			if ( *skill_list )
				snprintf (skill_list + strlen(skill_list), MAX_STRING_LENGTH, " %s", buf);
			else snprintf (skill_list + strlen(skill_list), MAX_STRING_LENGTH, "%s", buf);
		}

		mysql_safe_query ("INSERT INTO professions (name, skill_list) VALUES ('%s', '%s')", profession, skill_list);

		send_to_char ("Profession has been added to the database.\n", ch);
		return;
	}

	send_to_char ("Usage: profession (add | delete | list) (<arguments>)\n", ch);
	
	return;
}

void set_hobbitmail_flags (int id, int flag)
{
	mysql_safe_query ("UPDATE hobbitmail SET flags = %d WHERE id = %d", flag, id);
	return;
}

/** One time function to migrate old skills to new skill listing **/
void update_weapon_skills (OBJ_DATA *obj)
{
	if ( GET_ITEM_TYPE (obj) != ITEM_WEAPON )
		return;
	if ( IS_SET (obj->obj_flags.extra_flags, ITEM_NEWSKILLS) )
		return;

	obj->o.od.value[3] += 1;

	if ( obj->o.od.value[3] == SKILL_STAFF )
		obj->o.od.value[3] = SKILL_SHORTBOW;
	else if ( obj->o.od.value[3] == SKILL_POLEARM )
		obj->o.od.value[3] = SKILL_LONGBOW;
	else if ( obj->o.od.value[3] == SKILL_THROWN )
		obj->o.od.value[3] = SKILL_CROSSBOW;
	else if ( obj->o.od.value[3] == SKILL_BLOWGUN )
		obj->o.od.value[3] = SKILL_STAFF;
	else if ( obj->o.od.value[3] == SKILL_SLING )
		obj->o.od.value[3] = SKILL_STAFF;
	else if ( obj->o.od.value[3] == SKILL_SHORTBOW )
		obj->o.od.value[3] = SKILL_POLEARM;

	SET_BIT (obj->obj_flags.extra_flags, ITEM_NEWSKILLS);
	
	return;
}



/*                                                                          *
 * function: reload_mob_resets                                              *
 *                                                                          *
 *                                                                          */
void reload_mob_resets (void)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	mysql_safe_query ("SELECT * FROM mob_resets");
	if ( ( result = mysql_store_result ( database ) ) != NULL ) {
	  while ( (row = mysql_fetch_row ( result ) ) != NULL ) {
	    zone_table [strtol(row[0], NULL, 10)].cmd [strtol(row[1], NULL, 10)].enabled = 1;
	  }
	  mysql_free_result ( result ); result = NULL;
	}
	else {
	  snprintf (buf, MAX_STRING_LENGTH, "Warning: reload_mob_resets(): %s", mysql_error ( database ) );
	system_log (buf, TRUE);
	}
	return;
}

char *resolved_host (char *ip)
{
        MYSQL_RES       *result;
        MYSQL_ROW       row;
	char		buf [MAX_STRING_LENGTH];
	static char	resolved [MAX_STRING_LENGTH];

	*resolved = '\0';

	mysql_safe_query ("SELECT * FROM resolved_hosts WHERE ip = '%s'", ip);

	if ( ( result = mysql_store_result (database) ) != NULL ) {
		row = mysql_fetch_row ( result ) ;
	    
		if ( row != NULL ) {
			if ( time(0) - strtol(row[2], NULL, 10) >= (60 * 60 * 24 * 28) ) {		/* Update listing after it's a month old. */
		      		mysql_safe_query ("DELETE FROM resolved_hosts WHERE ip = '%s'", ip);
				mysql_free_result (result); 
				result = NULL;
		      		return NULL;
		    	}
		    	snprintf (resolved, MAX_STRING_LENGTH, "%s", row[1]);
		    	if ( *resolved == '-' ) {
				mysql_free_result (result); 
				result = NULL;
		      		return ip;
			}
		    	else {
				mysql_free_result (result); 
				result = NULL;
				return resolved;
			}
	  	}
	}
	else {
        	snprintf (buf, MAX_STRING_LENGTH, "Warning: resolved_host(): %s", mysql_error ( database ) );
		system_log (buf, TRUE);
	}

	if ( result )
		mysql_free_result (result);

	return NULL;
}

/*                                                                          *
 * function: reload_sitebans                                                *
 *                                                                          *
 *                                                                          */
void reload_sitebans()
{
	SITE_INFO		*site = NULL;
	SITE_INFO		*tmp_site = NULL;
    MYSQL_RES       *result = NULL;
    MYSQL_ROW       row;
	char			buf [MAX_STRING_LENGTH] = {'\0'};

	mysql_safe_query ("SELECT * FROM banned_sites");
	if ( ( result = mysql_store_result (database) ) == NULL ) {
          snprintf (buf, MAX_STRING_LENGTH, "Warning: reload_sitebans(): %s", mysql_error ( database ) );
	  system_log (buf, TRUE);
	  return;
	}

	while ( (row = mysql_fetch_row(result)) ) {
		CREATE (site, SITE_INFO, 1);
		site->name = str_dup(row[0]);
		site->banned_by = str_dup(row[1]);
		site->banned_on = strtol(row[2], NULL, 10);
		site->banned_until = strtol(row[3], NULL, 10);
		if ( !banned_site )
			banned_site = site;
		else for ( tmp_site = banned_site; tmp_site; tmp_site = tmp_site->next ) {
			if ( !tmp_site->next ) {
				tmp_site->next = site;
				break;
			}
		}
	}	

	mysql_free_result (result);
	result = NULL;
	
	return;
}

void save_banned_sites ()
{
	SITE_INFO   *site = NULL;
	FILE        *fp = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( port != PLAYER_PORT )
		return;

	mysql_safe_query ("DELETE FROM banned_sites");

	for ( site = banned_site; site; site = site->next ) {
		mysql_safe_query ("INSERT INTO banned_sites VALUES ('%s', '%s', %d, %d)", site->name, site->banned_by, site->banned_on, site->banned_until);
	}

	if ( !(fp = fopen("online_stats", "w+")) ) {
		system_log (buf, TRUE);
		return;
	}

	fprintf (fp, "%d\n%s~\n", count_max_online, max_online_date);
	fclose (fp);
        
	return;
}

OBJ_DATA *read_saved_obj (MYSQL_ROW row)
{
	OBJ_DATA			*obj = NULL;
	WOUND_DATA			*wound = NULL;
	LODGED_OBJECT_INFO 	*lodged = NULL;
	AFFECTED_TYPE		*af = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		location [MAX_STRING_LENGTH] = {'\0'};
	char		type [MAX_STRING_LENGTH] = {'\0'};
	char		severity [MAX_STRING_LENGTH] = {'\0'};
	char		name [MAX_STRING_LENGTH] = {'\0'};
	char		*p = NULL;
	int			damage = 0;
	int			bleeding = 0;
	int			poison = 0;
	int			vnum = 0;
	int			healerskill = 0;
	int			lasthealed = 0;
	int			lastbled = 0;

	obj = load_object (strtol(row[4], NULL, 10));
	if ( !obj )
		return NULL;

	if ( row[5] && strlen (row[5]) > 3 && str_cmp (row[5], "(null)") ) {
		mem_free (obj->name);
		obj->name = str_dup (row[5]);
	}
	if ( row[6] && strlen (row[6]) > 3 && str_cmp (row[6], "(null)") ) {
		mem_free (obj->short_description);
		obj->short_description = str_dup (row[6]);
	}
	if ( row[7] && strlen (row[7]) > 3 && str_cmp (row[7], "(null)") ) {
		mem_free (obj->description);
		obj->description = str_dup (row[7]);
	}
	if ( row[8] && strlen (row[8]) > 3 && str_cmp (row[8], "(null)") ) {
		mem_free (obj->full_description);
		obj->full_description = str_dup (row[8]);
	}
	if ( row[9] && strlen (row[9]) > 3 && str_cmp (row[9], "(null)") ) {
		mem_free (obj->desc_keys);
		obj->desc_keys = str_dup (row[9]);
	}
	if ( row[10] && strlen (row[10]) > 3 && str_cmp (row[10], "(null)") ) {
		mem_free (obj->book_title);
		obj->book_title = str_dup (row[10]);
	}

	obj->title_skill = strtol(row[11], NULL, 10);
	obj->title_language = strtol(row[12], NULL, 10);
	obj->title_script = strtol(row[13], NULL, 10);
	obj->loaded = load_object(strtol(row[14], NULL, 10));
	obj->obj_flags.extra_flags = strtol(row[15], NULL, 10);
	obj->o.od.value[0] = strtol(row[16], NULL, 10);
	obj->o.od.value[2] = strtol(row[17], NULL, 10);
	obj->o.od.value[3] = strtol(row[18], NULL, 10);
	obj->o.od.value[4] = strtol(row[19], NULL, 10);
	obj->o.od.value[5] = strtol(row[20], NULL, 10);
	obj->o.od.value[6] = strtol(row[21], NULL, 10);
	obj->obj_flags.weight = strtol(row[22], NULL, 10);
	obj->size = strtol(row[23], NULL, 10);
	obj->count = strtol(row[24], NULL, 10);
	obj->obj_timer = strtol(row[25], NULL, 10);
	obj->clock = strtol(row[26], NULL, 10);
	obj->morphto = strtol(row[27], NULL, 10);
	obj->morphTime = strtol(row[28], NULL, 10);

	if ( row[29] && strlen (row[29]) > 3 && str_cmp (row[29], "(null)") ) {
		mem_free (obj->var_color);
		obj->var_color = str_dup (row[29]);
	}
	if ( row[30] && strlen (row[30]) > 3 && str_cmp (row[30], "(null)") ) {
		mem_free (obj->omote_str);
		obj->omote_str = str_dup (row[30]);
	}

	if ( row[31] && strlen (row[31]) > 3 && str_cmp (row[31], "(null)") ) {
		p = str_dup (row[31]);
                while ( 1 ) {			
                        if ( !*p )
                                break;
                        (void)get_line (&p, buf);
                        if ( !*buf || !str_cmp (buf, "(null)") )
                                break;
			CREATE (wound, WOUND_DATA, 1);
			sscanf (buf, "%s %s %s %s %d %d %d %d %d %d\n",
				location, type, severity, name, &damage, &bleeding, &poison,
				&healerskill, &lasthealed, &lastbled);
			wound->location = str_dup (location);
			wound->type = str_dup (type);
			wound->severity = str_dup (severity);
			wound->name = str_dup (name);
			wound->damage = damage;
			wound->bleeding = bleeding;
			wound->poison = poison;
			wound->healerskill = healerskill;
			wound->lasthealed = lasthealed;
			wound->lastbled = lastbled;
			if ( obj->wounds )
				wound->next = obj->wounds;
			else wound->next = NULL;
			obj->wounds = wound;
		}
		if ( p ) {
			mem_free (p);
			p = NULL;
		}
	}

	if ( row[32] && strlen (row[32]) > 3 && str_cmp (row[32], "(null)") ) {
		p = str_dup (row[32]);
                while ( 1 ) {			
                        if ( !*p )
                                break;
                        (void)get_line (&p, buf);
                        if ( !*buf || !str_cmp (buf, "(null)") )
                                break;
			CREATE (lodged, LODGED_OBJECT_INFO, 1);
			sscanf (buf, "%s %d\n", location, &vnum);
			lodged->location = str_dup (location);
			lodged->vnum = vnum;
			if ( obj->lodged )
				lodged->next = obj->lodged;
			else lodged->next = NULL;
			obj->lodged = lodged;
		}
	}

	if ( row[33] && strlen (row[33]) > 3 && str_cmp (row[33], "(null)") ) {
		p = str_dup (row[33]);
                while ( 1 ) {			
                        if ( !*p )
                                break;
                        (void)get_line (&p, buf);
                        if ( !*buf || !str_cmp (buf, "(null)") )
                                break;
			CREATE (af, AFFECTED_TYPE, 1);
			sscanf (buf, "%d %d %d %d %d %d %d\n",
				&af->a.spell.location, &af->a.spell.modifier, &af->a.spell.duration, &af->a.spell.bitvector,
				&af->a.spell.t, &af->a.spell.sn, &af->type);
			if ( obj->xaffected )
				af->next = obj->xaffected;
			else af->next = NULL;
			obj->xaffected = af;
		}
	}

	obj->next_content = NULL;

	return obj;
}


/*                                                                          *
 * function: load_mysql_save_rooms                                          *
 *                                                                          *
 *                                                                          */
void load_mysql_save_rooms ()
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int			i = 0;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	mysql_safe_query ("SELECT * FROM saveroom_objs");

	if ( ( result = mysql_store_result (database) ) == NULL ) {
          snprintf (buf, MAX_STRING_LENGTH, "Warning: load_mysql_save_rooms(): %s", mysql_error ( database ) );
	  system_log (buf, TRUE);
	  return;
	}

	while ( (row = mysql_fetch_row(result)) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Object #%d: %s [%s]", i, row[5], row[0]);
		i++;
		if ( strtol(row[0], NULL, 10) == -1 )
			continue;
		obj_to_room (read_saved_obj(row), strtol(row[0], NULL, 10));
		system_log (buf, FALSE);
	}

	mysql_free_result (result);
	
	return;
}

void store_mysql_obj (OBJ_DATA *obj, char *wc, int pos, int objstack)
{
	WOUND_DATA			*wound = NULL;
	LODGED_OBJECT_INFO	*lodged = NULL;
	AFFECTED_TYPE		*af = NULL;
	TRIGGER_DATA		*trig = NULL;
	int				loaded = 0;
	char			trig_list [MAX_STRING_LENGTH] = {'\0'};
	char			wound_list [MAX_STRING_LENGTH] = {'\0'};
	char			lodged_list [MAX_STRING_LENGTH] = {'\0'};
	char			affect_list [MAX_STRING_LENGTH] = {'\0'};
	char			sdesc [MAX_STRING_LENGTH] = {'\0'};
	char			ldesc [MAX_STRING_LENGTH] = {'\0'};
	char			fdesc [MAX_STRING_LENGTH] = {'\0'};
	char			name [MAX_STRING_LENGTH] = {'\0'};
	char			title [MAX_STRING_LENGTH] = {'\0'};
	char			omote [MAX_STRING_LENGTH] = {'\0'};
	char			varcolor [MAX_STRING_LENGTH] = {'\0'};


	for ( wound = obj->wounds; wound; wound = wound->next ) {
		snprintf (wound_list + strlen(wound_list), MAX_STRING_LENGTH, "%s %s %s %s %d %d %d %d %d %d %d\n",
				wound->location, wound->type, wound->severity, wound->name, wound->damage,
				wound->bleeding, wound->poison, wound->infection, wound->healerskill, wound->lasthealed,
				wound->lastbled);
	}
	
	

	for ( lodged = obj->lodged; lodged; lodged = lodged->next ) {
		snprintf (lodged_list + strlen(lodged_list), MAX_STRING_LENGTH, "%s %d\n", lodged->location, lodged->vnum);
	}

	for ( af = obj->xaffected; af; af = af->next ) {
		if ( af->a.spell.location ) {
			snprintf (affect_list + strlen(affect_list), MAX_STRING_LENGTH, "%d %d %d %d %d %d %d\n",
				af->a.spell.location, af->a.spell.modifier, af->a.spell.duration, af->a.spell.bitvector,
				af->a.spell.t, af->a.spell.sn, af->type);
		}
	}

	*sdesc = '\0';
	*ldesc = '\0';
	*fdesc = '\0';
	*name = '\0';
	*title = '\0';
	*omote = '\0';
	*varcolor = '\0';

	if ( obj->short_description )
		mysql_real_escape_string (database, sdesc, obj->short_description, strlen(obj->short_description));
	if ( obj->description )
		mysql_real_escape_string (database, ldesc, obj->description, strlen(obj->description));
	if ( obj->full_description )
		mysql_real_escape_string (database, fdesc, obj->full_description, strlen(obj->full_description));
	if ( obj->name )
		mysql_real_escape_string (database, name, obj->name, strlen(obj->name));
	if ( obj->book_title )
		mysql_real_escape_string (database, title, obj->book_title, strlen(obj->book_title));
	if ( obj->omote_str )
		mysql_real_escape_string (database, omote, obj->omote_str, strlen(obj->omote_str));
	if ( obj->var_color )
		mysql_real_escape_string (database, varcolor, obj->var_color, strlen(obj->var_color));

	if ( obj->loaded )
		loaded = obj->loaded->virtual;
	else loaded = 0;

	mysql_safe_query ("INSERT INTO saveroom_objs (room, wc, pos, objstack, obj_vnum, name, short_desc, long_desc, full_desc, "
			 "desc_keys, book_title, title_skill, title_language, title_script, loaded, extraflags, oval0, "
			 "oval1, oval2, oval3, oval4, oval5, weight, size, count, timer, clock, morphto, morphTime, "
			 "varcolor, omotestr, wounds, lodged, affects) VALUES ("
			 "%d, %d, %d, %d, %d, '%s', '%s', '%s', '%s', "
			 "'%s', '%s', %d, %d, %d, %d, %d, %d, "
			 "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
			 "'%s', '%s', '%s', '%s', '%s')",
			
			(int) obj->in_room, (int) wc, (int) pos, (int) objstack, obj->virtual, name, sdesc, ldesc, fdesc,
			obj->desc_keys, title, obj->title_skill, obj->title_language, obj->title_script, loaded, (int) obj->obj_flags.extra_flags, obj->o.od.value[0],
			obj->o.od.value[1], obj->o.od.value[2], obj->o.od.value[3], obj->o.od.value[4], obj->o.od.value[5], (int)obj->obj_flags.weight, obj->size, obj->count, 
			obj->obj_timer, obj->clock, obj->morphto, obj->morphTime, varcolor, omote, wound_list, lodged_list, affect_list); 
			
	return;
}

void write_obj_data_mysql (OBJ_DATA *obj, char *wc, int pos, int objstack)
{
	if ( !obj )
		return;

	if ( !vtoo (obj->virtual) )
		return;

/* Save object */

	store_mysql_obj (obj, wc, pos, objstack);

/* Save everything this object contains */

	if ( obj->contains )
		store_mysql_obj (obj->contains, wc, pos, objstack + 1);

/* Save the next object in the list */

	if ( (obj->location == -1 && obj->next_content && obj != obj->next_content ) )
		store_mysql_obj (obj->next_content, wc, pos, objstack);

	if ( obj->next_content && obj->next_content == obj )
		obj->next_content = NULL;

	return;
}

/*                                                                          *
 * function: is_newbie                                                      *
 *                                                                          *
 *                                                                          */
int is_newbie (CHAR_DATA *ch)
{
	MYSQL_RES	*result = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int         nPlayerFiles = 0;

	if ( IS_NPC (ch) )
		return 0;

	mysql_safe_query ("SELECT * FROM %s.pfiles WHERE account = '%s'", PFILES_DATABASE, ch->pc->account);

	if ( ( result = mysql_store_result (database) ) != NULL ) { 
		nPlayerFiles = mysql_num_rows(result);
	  	mysql_free_result (result); result = NULL;
	  	return ( ( nPlayerFiles <= 1 ) ? 1 : 0 ) ;
	}
	else {
	  	snprintf (buf, MAX_STRING_LENGTH, "Warning: is_newbie(): %s", mysql_error ( database ) );
		system_log (buf, TRUE);
	  	return 0; /* assume not a newbie ? */
	}
	
	return(0);
}

int is_yours (char *name, char *account)
{
	MYSQL_RES	*result = NULL;
	int			isYours = 0;

	if ( !name || !*name || !account || !*account )
		return 0;

	mysql_safe_query ("SELECT * FROM reviews_in_progress WHERE char_name = '%s' AND reviewer = '%s' AND (UNIX_TIMESTAMP() - timestamp) <= 60 * 20", name, account);
	if ( ( result = mysql_store_result (database) ) != NULL ) {

		if ( mysql_num_rows(result) > 0 ) {
			isYours = 1;
		}
		mysql_free_result (result);
	}
	else if ( result != NULL )
		mysql_free_result (result);

	return isYours;
}

int is_being_reviewed (char *name, char *account)
{
	MYSQL_RES	*result = NULL;
	int		isBeingReviewed = 0;

	if ( !name || !*name || !account || !*account )
		return 0;

	mysql_safe_query ("SELECT * FROM reviews_in_progress WHERE char_name = '%s' AND reviewer != '%s' AND (UNIX_TIMESTAMP() - timestamp) <= 60 * 20", name, account);
	if ( ( result = mysql_store_result (database) ) != NULL ) {

		if ( mysql_num_rows(result) > 0 ) {
			isBeingReviewed = 1;
		}
		mysql_free_result (result);
	}
	else if ( result != NULL )
		mysql_free_result (result);

	return isBeingReviewed;
}

/*                                                                          *
 * function: is_guide                                                       *
 *                                                                          *
 *                                                                          */
int is_guide (char *username)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			user_id = 0;

	if ( !username || !*username )
		return 0;

	mysql_safe_query ("SELECT user_id FROM forum_users WHERE username = '%s'", username);
	if ( ( result = mysql_store_result (database) ) != NULL ) { 
		row = mysql_fetch_row(result);
	  	if ( row ) {
	    		user_id = strtol(row[0], NULL, 10);
	    		mysql_safe_query ("SELECT user_id FROM forum_user_group WHERE user_id = %d AND group_id = 2521", user_id);
	        	mysql_free_result (result); result = NULL;
	    		if ( ( result = mysql_store_result (database) ) != NULL ) {
	      			row = mysql_fetch_row(result);
	      			if ( row ) {
		      			mysql_free_result (result); result = NULL;
					return 1;
	      			}
	      			else mysql_free_result (result); result = NULL;
	    		}
		}
	    	else {
	      		snprintf (buf, MAX_STRING_LENGTH, "Warning: is_guide(): %s", mysql_error ( database ) );
			system_log (buf, TRUE);
	    	}
	}
	else { 
		snprintf (buf, MAX_STRING_LENGTH, "Warning: is_guide(): %s", mysql_error ( database ) );
		system_log (buf, TRUE);
	}

	return 0;

}

/*                                                                          *
 * function: do_history                                                     *
 *                                                                          *
 *                                                                          */
void do_history (CHAR_DATA *ch, char *argument, int cmd)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		name [255];
	int        	nResponses = 0;

        if ( !ch->delay || ch->delay_type != DEL_APP_APPROVE ) {
		send_to_char ("You are not currently reviewing an application.\n", ch);
                return;
        }

	*name = '\0';

        strcpy (name, ch->delay_who);
        mem_free (ch->delay_who);
        ch->delay_who = NULL;
        ch->delay = 0;

	if ( !*name ) {
		send_to_char ("You are not currently reviewing an application.\n", ch);
		return;
	}

	mysql_safe_query ("SELECT * FROM virtual_boards WHERE board_name = 'Applications' AND subject LIKE '%% %s' ORDER BY timestamp ASC", name);
	if ( ( result = mysql_store_result (database) ) != NULL ) {
	  nResponses = mysql_num_rows(result);
	  *b_buf = '\0';
	  while ( (row = mysql_fetch_row(result) ) != NULL ) {
	    snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH,	
		     "\n#6Reviewed By:#0 %s\n"
		     "#6Reviewed On:#0 %s\n\n"
		     "%s\n--", row[3], row[4], row[5]);
	  }
	  mysql_free_result (result);
	  result = NULL;

	  if ( nResponses ) {
	    strcat (b_buf, "\n");
	    page_string (ch->desc, b_buf);
	  }
	  else {
	    send_to_char ("This application has not yet had any administrator responses.\n", ch);
	  }
	}
	else {
          snprintf (buf, MAX_STRING_LENGTH, "Warning: do_history(): %s", mysql_error ( database ) );
	  system_log (buf, TRUE);
	}
	return;
}

void do_writings (CHAR_DATA *ch, char *argument, int cmd)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	int			i = 0;

	if ( !*argument ) {
		send_to_char ("How do you wish to access the writings database?\n", ch);
		return;
	}

	argument = one_argument (argument, buf);

	if ( !str_cmp (buf, "by") ) {
		argument = one_argument (argument, buf);
		if ( !*buf ) {
			send_to_char ("For whom do you wish to search in the writings database?\n", ch);
			return;
		}
		argument = one_argument (argument, buf2);
		if ( !str_cmp (buf2, "containing") ) {
			argument = one_argument (argument, buf2);
			if ( !*buf2 ) {
				send_to_char ("Which strings did you wish to search for in these writings?\n", ch);
				return;
			}
			mysql_safe_query ("SELECT db_key FROM player_writing WHERE author = '%s' AND writing LIKE '%%%s%%' GROUP BY db_key ORDER BY db_key ASC", buf, buf2);
		}
		else mysql_safe_query ("SELECT db_key FROM player_writing WHERE author = '%s' GROUP BY db_key ORDER BY db_key ASC", buf);
		result = mysql_store_result(database);
		if ( !result || !mysql_num_rows(result) ) {
			if ( result )
				mysql_free_result (result);
			send_to_char ("No writings were found in the database for that PC.\n", ch);
			return;
		}
	
		*b_buf = '\0';
		snprintf (b_buf, MAX_STRING_LENGTH, "\nThe database contains the following keys to writings authored by #6%s#0:\n\n", CAP(buf));
		i = 0;

		while ( (row = mysql_fetch_row(result)) ) {
                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "   %-15d", (int)strtol(row[0], NULL, 10));
			i++;
			if ( !(i % 4) )
				strcat (b_buf, "\n");
		}

		if ( (i % 4) )
			strcat (b_buf, "\n");

		page_string (ch->desc, b_buf);

		mysql_free_result(result);
		result = NULL;
	}
	else if ( !str_cmp (buf, "containing") ) {
		argument = one_argument (argument, buf2);
		if ( !*buf2 ) {
			send_to_char ("Which strings did you wish to search for in these writings?\n", ch);
			return;
		}
		mysql_safe_query ("SELECT db_key FROM player_writing WHERE writing LIKE '%%%s%%' GROUP BY db_key ORDER BY db_key ASC", buf2);
		result = mysql_store_result(database);
		if ( !result || !mysql_num_rows(result) ) {
			if ( result )
				mysql_free_result (result);
			send_to_char ("No writings were found in the database matching that string.\n", ch);
			return;
		}
	
		*b_buf = '\0';
		snprintf (b_buf, MAX_STRING_LENGTH, "\nThe database contains the following keys to writings matching that string:\n\n");
		i = 0;

		while ( (row = mysql_fetch_row(result)) ) {
                        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "   %-15d", (int)strtol(row[0], NULL, 10));
			i++;
			if ( !(i % 4) )
				strcat (b_buf, "\n");
		}

		if ( (i % 4) )
			strcat (b_buf, "\n");

		page_string (ch->desc, b_buf);

		mysql_free_result(result);
		result = NULL;
	}
	else if ( !str_cmp (buf, "display") ) {
		argument = one_argument (argument, buf);
		if ( !*buf ) {
			send_to_char ("Which database key did you wish to view the writing for?\n", ch);
			return;
		}
		if ( !isdigit (*buf) ) {
			send_to_char ("You'll need to specify a numeric database key to view.\n", ch);
			return;
		}

		mysql_safe_query ("SELECT * FROM player_writing WHERE db_key = %d ORDER BY page ASC", strtol(buf, NULL, 10));
		result = mysql_store_result (database);
		
		if ( !result || !mysql_num_rows(result) ) {
			if ( result )
				mysql_free_result (result);
			send_to_char ("The database contains no writing under that key.\n", ch);
			return;
		}

		*b_buf = '\0';

		while ( (row = mysql_fetch_row(result)) ) {
			snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "\n#6Author:#0       %s\n"
							"#6Written On:#0   %s\n"
							"#6Written In:#0   %s and %s\n"
							"#6Page Number:#0  %d\n"
							"\n"
							"%s", row[1], row[3], skill_data[strtol(row[5], NULL, 10)].skill_name, skill_data[strtol(row[6], NULL, 10)].skill_name, (int)strtol(row[2], NULL, 10), row[8]);
		}

		page_string (ch->desc, b_buf);

		mysql_free_result(result);
		result = NULL;
	}
	else if ( !str_cmp (buf, "key") ) {
		argument = one_argument (argument, buf);
		if ( !*buf ) {
			send_to_char ("Which object did you wish to key a writing to?\n", ch);
			return;
		}
		if ( !(obj = get_obj_in_dark (ch, buf, ch->right_hand)) ) {
			send_to_char ("I don't see that object.\n", ch);
			return;
		}
		if ( GET_ITEM_TYPE (obj) != ITEM_BOOK && GET_ITEM_TYPE (obj) != ITEM_PARCHMENT ) {
			send_to_char ("You may only key writings to books or parchment objects.\n", ch);
			return;
		}
		argument = one_argument (argument, buf);
		if ( !*buf ) {
			send_to_char ("Which writing entry did you wish to clone to this object?\n", ch);
			return;
		}
		if ( !isdigit (*buf) ) {
			send_to_char ("You'll need to specify a numeric database key to clone.\n", ch);
			return;
		}
		mysql_safe_query ("SELECT COUNT(*) FROM player_writing WHERE db_key = %d", strtol(buf, NULL, 10));
		result = mysql_store_result(database);
		if ( !result || !mysql_num_rows(result) ) {
			if ( result )
				mysql_free_result (result);
			send_to_char ("The database contains no keys matching that number.\n", ch);
			return;
		}
		row = mysql_fetch_row(result);
		if ( (GET_ITEM_TYPE(obj) == ITEM_PARCHMENT && strtol(row[0], NULL, 10) > 1) ||
			(GET_ITEM_TYPE(obj) == ITEM_BOOK && strtol(row[0], NULL, 10) > obj->o.od.value[0]) ) {
			send_to_char ("That writing key contains too many pages for this object to hold.\n", ch);
			return;
		}
		if ( GET_ITEM_TYPE (obj) == ITEM_PARCHMENT )
			obj->o.od.value[0] = strtol(buf, NULL, 10);
		else obj->o.od.value[1] = strtol(buf, NULL, 10);
		load_writing (obj);
		if ( GET_ITEM_TYPE (obj) == ITEM_PARCHMENT )
			obj->o.od.value[0] = unused_writing_id();
		else obj->o.od.value[1] = unused_writing_id();
		save_writing (obj);
		send_to_char ("Writing has been successfully cloned and keyed to the object.\n", ch);
		mysql_free_result(result);
		result = NULL;
		return;
	}
	else send_to_char ("That isn't a recognized option for this command.\n", ch);
	
	return;
}	

void load_all_writing (void)
{
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
	WRITING_DATA	*writing = NULL;
	OBJ_DATA		*obj = NULL;
	bool			loaded = FALSE;
	int				i = 0;

	mysql_safe_query ("SELECT * FROM player_writing");

	result = mysql_store_result (database);
	if ( !result || !mysql_num_rows(result) ) {
		if ( result )
			mysql_free_result (result);
		return;
	}

	while ( (row = mysql_fetch_row(result)) ) {
		for ( obj = object_list; obj; obj = obj->next ) {
			if ( obj->deleted )
				continue;

			if ( GET_ITEM_TYPE (obj) == ITEM_PARCHMENT && obj->o.od.value[0] == strtol(row[0], NULL, 10) ) {
				if ( !obj->writing )
					CREATE (obj->writing, WRITING_DATA, 1);
				obj->writing->author = add_hash(row[1]);
				obj->writing->date = add_hash(row[3]);
				obj->writing->ink = add_hash(row[4]);
				obj->writing->language = strtol(row[5], NULL, 10);
				obj->writing->script = strtol(row[6], NULL, 10);
				obj->writing->skill = strtol(row[7], NULL, 10);
				obj->writing->message = add_hash(row[8]);
				obj->writing_loaded = TRUE;
				break;
			}
			else if ( GET_ITEM_TYPE (obj) == ITEM_BOOK && obj->o.od.value[1] == strtol(row[0], NULL, 10) ) {
				if ( !obj->writing )
					CREATE (obj->writing, WRITING_DATA, 1);
				writing = obj->writing;
				for ( i = 1; i <= obj->o.od.value[0]; i++ ) {
					if ( !writing->next_page && i + 1 <= obj->o.od.value[0] ) {
						CREATE (writing->next_page, WRITING_DATA, 1);
					}
					if ( i == strtol(row[2], NULL, 10) ) {
						writing->author = add_hash(row[1]);
						writing->date = add_hash(row[3]);
						writing->ink = add_hash(row[4]);
						writing->language = strtol(row[5], NULL, 10);
						writing->script = strtol(row[6], NULL, 10);
						writing->skill = strtol(row[7], NULL, 10);
						writing->message = add_hash(row[8]);
						loaded = TRUE;
						break;
					}
					writing = writing->next_page;
				}
				if ( loaded ) {
					obj->writing_loaded = TRUE;
					loaded = FALSE;
					break;
				}
			}
		}
	}

	if ( result )
		mysql_free_result (result);
	result = NULL;
	
	return;
}

void load_writing (OBJ_DATA *obj)
{
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
	WRITING_DATA	*writing = NULL;
	int				id = 0;
	int				i = 0;

	if ( obj->writing_loaded )
		return;

	if ( GET_ITEM_TYPE (obj) != ITEM_BOOK && GET_ITEM_TYPE (obj) != ITEM_PARCHMENT )
		return;

	obj->writing_loaded = TRUE;

	if ( GET_ITEM_TYPE (obj) == ITEM_BOOK )
		id = (obj)->o.od.value[1];
	else id = (obj)->o.od.value[0];

	mysql_safe_query ("SELECT * FROM player_writing WHERE db_key = %d ORDER BY db_key,page ASC", id);

	result = mysql_store_result (database);
	if ( !result || !mysql_num_rows(result) ) {
		if ( result )
			mysql_free_result (result);
		return;
	}

	if ( GET_ITEM_TYPE (obj) == ITEM_PARCHMENT ) {
		row = mysql_fetch_row(result);
		if ( !row )
			return;
		if ( !obj->writing )
			CREATE (obj->writing, WRITING_DATA, 1);
		writing = obj->writing;
		writing->author = add_hash(row[1]);
		writing->date = add_hash(row[3]);
		writing->ink = add_hash(row[4]);
		writing->language = strtol(row[5], NULL, 10);
		writing->script = strtol(row[6], NULL, 10);
		writing->skill = strtol(row[7], NULL, 10);
		writing->message = add_hash(row[8]);
		if ( result )
			mysql_free_result (result);
		return;
	}
	else if ( GET_ITEM_TYPE (obj) == ITEM_BOOK ) {
		if ( !obj->writing )
			CREATE (obj->writing, WRITING_DATA, 1);
		while ( (row = mysql_fetch_row(result)) ) {
			writing = obj->writing;
			for ( i = 1; i <= obj->o.od.value[0]; i++ ) {
				if ( !writing->next_page && i + 1 <= obj->o.od.value[0] ) {
					CREATE (writing->next_page, WRITING_DATA, 1);
				}
				if ( i == strtol(row[2], NULL, 10) ) {
					writing->author = add_hash(row[1]);
					writing->date = add_hash(row[3]);
					writing->ink = add_hash(row[4]);
					writing->language = strtol(row[5], NULL, 10);
					writing->script = strtol(row[6], NULL, 10);
					writing->skill = strtol(row[7], NULL, 10);
					writing->message = add_hash(row[8]);
					break;
				}
				writing = writing->next_page;
			}
		}
	}

	if ( result )
		mysql_free_result (result);
	result = NULL;
	
	return;
}

void save_writing (OBJ_DATA *obj)
{
	WRITING_DATA	*writing = NULL;
	int				i = 1;

	if ( !obj->writing )
		return;
	
	if ( GET_ITEM_TYPE (obj) != ITEM_PARCHMENT && GET_ITEM_TYPE (obj) != ITEM_BOOK )
		return;

	if ( GET_ITEM_TYPE (obj) == ITEM_PARCHMENT ) {
		writing = obj->writing;
		if ( !writing || !writing->date || !str_cmp (writing->date, "blank") || !writing->language || !writing->script )
			return;
		if ( !obj->o.od.value[0] )
			obj->o.od.value[0] = unused_writing_id();
		mysql_safe_query ("DELETE FROM player_writing WHERE db_key = %d", obj->o.od.value[0]);
		mysql_safe_query ("INSERT INTO player_writing VALUES (%d, '%s', 1, '%s', '%s', %d, "
				"%d, %d, '%s', %d)", obj->o.od.value[0], writing->author, writing->date,
				writing->ink, writing->language, writing->script, writing->skill, writing->message, (int)time(0));
	}

	if ( GET_ITEM_TYPE (obj) == ITEM_BOOK ) {
		if ( !obj->o.od.value[1] )
			obj->o.od.value[1] = unused_writing_id();
		mysql_safe_query ("DELETE FROM player_writing WHERE db_key = %d", obj->o.od.value[1]);
		for ( writing = obj->writing; writing != NULL && i <= obj->o.od.value[0]; writing = writing->next_page, i++ ) {
			if ( !writing || !writing->date || !str_cmp (writing->date, "blank") || !writing->language || !writing->script )
				continue;
			mysql_safe_query ("INSERT INTO player_writing VALUES (%d, '%s', %d, '%s', '%s', %d, "
					"%d, %d, '%s', %d)", obj->o.od.value[1], writing->author, i, writing->date,
					writing->ink, writing->language, writing->script, writing->skill, writing->message, (int) time(0));
			if ( !writing->next_page )
				break;
		}
	}
	return;
}

int unused_writing_id (void)
{
	MYSQL_RES	*result = NULL;
	bool		again = TRUE;
	int			id = 0;

	do {
		id = rand();
		mysql_safe_query ("SELECT db_key FROM player_writing WHERE db_key = %d", id);
		result = mysql_store_result (database);
		if ( !result || !mysql_num_rows(result) ) {
			if ( result )
				mysql_free_result (result);
			return id;
		}
		mysql_free_result (result);
		result = NULL;
	} while ( again );

	return id;
}

void save_dreams (CHAR_DATA *ch)
{
	DREAM_DATA		*dream = NULL;

	if ( !ch || !ch->tname )
		return;

	mysql_safe_query ("DELETE FROM %s.dreams WHERE name = '%s'", PFILES_DATABASE, ch->tname);

	if ( ch->pc->dreams ) {
      	 	for ( dream = ch->pc->dreams; dream; dream = dream->next ) {
			mysql_safe_query ("INSERT INTO %s.dreams VALUES('%s', %d, '%s')", PFILES_DATABASE, ch->tname, 0, dream->dream);
		}
	}
        
	if ( ch->pc->dreamed ) {                 
        	for ( dream = ch->pc->dreamed; dream; dream = dream->next ) {
			mysql_safe_query ("INSERT INTO %s.dreams VALUES('%s', %d, '%s')", PFILES_DATABASE, ch->tname, 1, dream->dream);
		}
	}
	
	return;
}

void load_dreams (CHAR_DATA *ch)
{
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
	DREAM_DATA		*dream = NULL;
	DREAM_DATA		*dream_list = NULL;

	if ( !ch || !ch->tname )
		return;

	mysql_safe_query ("SELECT * FROM %s.dreams WHERE name = '%s'", PFILES_DATABASE, ch->tname);
	result = mysql_store_result (database);

	while ( (row = mysql_fetch_row(result)) ) {
		CREATE (dream, DREAM_DATA, 1);
		dream->dream = str_dup (row[2]);
		dream->next = NULL;

		if ( strtol(row[1], NULL, 10) > 0 ) {	/* Already dreamed. */
			if ( ch->pc->dreamed )
				dream_list = ch->pc->dreamed;
			else ch->pc->dreamed = dream;
		}
		else {
			if ( ch->pc->dreams )
				dream_list = ch->pc->dreams;
			else ch->pc->dreams = dream;
		}

		if ( dream_list ) {
			while ( dream_list->next != NULL )
				dream_list = dream_list->next;
			dream_list->next = dream;
		}

		dream_list = NULL;
	}

	mysql_free_result (result);

	return;
}

void perform_pfile_update (CHAR_DATA *ch)
{
	int	i = 0;

	if ( !ch->pc )
		return;

	if ( !ch->skills [SKILL_SWIMMING] ) {
		open_skill (ch, SKILL_SWIMMING);
		starting_skill_boost (ch, SKILL_SWIMMING);
	}

	if ( ch->skills [SKILL_SUBDUE] )
		ch->skills [SKILL_SUBDUE] = 0;

	if ( !ch->pc->level ) {
	        for ( i = 1; i <= MAX_SKILLS; i++ ) {   
        	        if ( ch->skills[i] > calc_lookup (ch, REG_CAP, i) )  
        	                ch->skills[i] = calc_lookup (ch, REG_CAP, i);  
        	}       

                update_crafts (ch);
	}
        
	for ( i = 1; i <= LAST_SKILL; i++ )
		ch->pc->skills [i] = ch->skills [i];

	if ( ch->str > 25 )
		ch->str = 25;
	if ( ch->dex > 25 )
		ch->dex = 25;
	if ( ch->agi > 25 )
		ch->agi = 25;
	if ( ch->aur > 25 )
		ch->aur = 25;
	if ( ch->con > 25 )
		ch->con = 25;
	if ( ch->wil > 25 )
		ch->wil = 25;
	if ( ch->intel > 25 )
		ch->intel = 25;

	ch->tmp_str = ch->str;
	ch->tmp_dex = ch->dex;
	ch->tmp_agi = ch->agi;
	ch->tmp_con = ch->con;
	ch->tmp_aur = ch->aur;
	ch->tmp_intel = ch->intel;
	
	return;
}

void process_queued_review (MYSQL_ROW row)
{
        CHAR_DATA       *tch = NULL;
        CHAR_DATA		*tmp_ch = NULL;
        ACCOUNT_DATA    *account = NULL;   
        char            buf [MAX_INPUT_LENGTH];
        char            buf2 [MAX_INPUT_LENGTH];
        char            email [255];
        char			subject [255];
        long int        time_elapsed = 0;
        bool            accepted = FALSE;
                              
        if ( !row )
                return;
                              
        if ( !(tch = load_pc (row[0])) ) {
                return;
        }
                
        for ( tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next ) {
                if ( tmp_ch->deleted )
                        continue; 
                if ( tmp_ch->pc && tmp_ch->pc->edit_player && !str_cmp (tmp_ch->pc->edit_player->tname, tch->tname) ) {
                        send_to_char ("The PC in your editor has been closed for an application response.\n", tmp_ch);
                        tmp_ch->pc->edit_player = NULL;
                }
        }

        while ( tch->pc->load_count > 1 )
                unload_pc (tch);
              
        if ( tch->pc->create_state != 1 ) {
                unload_pc (tch);
                return;
        }
                
        accepted = strtol(row[7], NULL, 10);  
        *buf = '\0';
	*subject = '\0';
                              
        /* Acceptance; process the new PC for entry into the game */
                              
        if ( accepted ) {
                setup_new_character (tch);

                mysql_safe_query ("UPDATE newsletter_stats SET accepted_apps=accepted_apps+1");

                mysql_safe_query ("UPDATE professions SET picked=picked+1 WHERE id=%d", tch->pc->profession);

                snprintf (buf, MAX_STRING_LENGTH,  "Greetings,\n"
                              "\n"
                              "   Thank you for your interest in %s! This is an automated\n"
                              "system notification sent to inform you that your application for a character\n"
                              "named %s has been ACCEPTED by the reviewer, and you that may\n"
                              "enter the game at your earliest convenience. We'll see you there!\n"
                              "\n"
                              "%s left the following comments regarding your application:\n"
                              "\n%s", MUD_NAME, tch->tname, row[1], row[8]);
		snprintf (subject, MAX_STRING_LENGTH, "#2Accepted:#0 %s", tch->tname);
		snprintf (buf, MAX_STRING_LENGTH,  "%s\n", row[8]);
		add_message (tch->tname, 3, row[1], NULL, "Application Acceptance", "", buf, 0);
        }
        else {
                tch->pc->create_state = STATE_REJECTED;

                mysql_safe_query ("UPDATE newsletter_stats SET declined_apps=declined_apps+1");

                snprintf (buf, MAX_STRING_LENGTH,  "\n#6Unfortunately, your application was declined on its most recent review.\n\n%s left the following comment(s) explaining why:#0\n"
                              "\n%s", row[1], row[8]);
                if ( buf [strlen(buf)-1] != '\n' )
                        strcat (buf, "\n");                
                tch->pc->msg = add_hash (buf);

                snprintf (buf, MAX_STRING_LENGTH,  "Greetings,\n"
                              "\n"
                              "   Thank you for your interest in %s! This is an\n"
                              "automated system notification to inform you that your application for\n"
                              "a character named %s was deemed inappropriate by the reviewer, and\n"
                              "therefore was declined. However, don't despair! This is a relatively\n"
                              "common occurrence, and nothing to worry about. Your application has\n"
                              "been saved on our server, and you may make the necessary changes simply\n"
                              "by entering the game as that character. You will be dropped back\n"
                              "into the character generation engine, where you may make corrections.\n"
                              "\n" 
                              "%s left the following comments regarding your application:\n"
                              "\n%s", MUD_NAME, tch->tname, row[1], row[8]);
		snprintf (subject, MAX_STRING_LENGTH, "#1Declined:#0 %s", tch->tname);
        }
        
        if ( (account = load_account (tch->pc->account)) && *buf ) {
                snprintf (email, MAX_STRING_LENGTH, "%s <%s>", MUD_NAME, MUD_EMAIL);
                send_email (account, NULL, email, "Re: Your Character Application", buf);
                if ( accepted && tch->pc->special_role ) {
                        snprintf (email, MAX_STRING_LENGTH, "%s Player <%s>", MUD_NAME, account->email);
                        free_account (account);
                        if ( (account = load_account(tch->pc->special_role->poster)) ) {
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
        
	if ( *subject ) {
		snprintf (buf, MAX_STRING_LENGTH,  "%s\n", row[8]);
		add_message ("Applications", 2, row[1], NULL, subject, "", buf, 0);
	}

        time_elapsed = time(0) - tch->time.birth; 
        mysql_safe_query ("INSERT INTO application_wait_times VALUES (%d)", (int)time_elapsed);
                              
        mysql_safe_query ("DELETE FROM queued_reviews WHERE char_name = '%s'", tch->tname);
        
	if ( tch->name && *tch->name && str_cmp (row[2], tch->name) ) {
		mem_free (tch->name);
		tch->name = str_dup (row[2]);
	}

	if ( tch->short_descr && *tch->short_descr && str_cmp (row[3], tch->short_descr) ) {
		mem_free (tch->short_descr);
		tch->short_descr = str_dup (row[3]);
	}

	if ( tch->long_descr && *tch->long_descr && str_cmp (row[4], tch->long_descr) ) {
		mem_free (tch->long_descr);
		tch->long_descr = str_dup (row[4]);
	}

	if ( tch->description && *tch->description && str_cmp (row[5], tch->description) ) {
		mem_free (tch->description);
		tch->description = str_dup (row[5]);
		reformat_string (tch->description, &tch->description);
	}

	if ( tch->pc && tch->pc->creation_comment && *tch->pc->creation_comment && str_cmp (row[6], tch->pc->creation_comment) ) {
		mem_free (tch->pc->creation_comment);
		tch->pc->creation_comment = str_dup (row[6]);
	}

	unload_pc (tch);
        
	return;
}

void process_reviews (void)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;

	if ( port != PLAYER_PORT )
		return;

	mysql_safe_query ("SELECT * FROM queued_reviews");
	if ( (result = mysql_store_result (database) ) != NULL ) {

		while ( (row = mysql_fetch_row(result)) ) {
			process_queued_review (row);
		}
		
		mysql_free_result (result);
	}
	return;
}

CHAR_DATA *load_char_mysql (char *name)
{
	int			ind = 0;
	int			lev = 0;
	int			damage = 0;
	int			bleeding = 0;
	int			i = 0;
	int			poison = 0;
	int			infection = 0;
	int			healerskill = 0;
	int			lasthealed = 0;
	int			lastbled = 0;
	char		wound_name [100];
	char		severity [100];
	char		type [100];
	char		location [100];
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		buf2 [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES	*result = NULL;
	MYSQL_ROW		row;
	CHAR_DATA		*ch = NULL;
	WOUND_DATA		*wound = NULL;
	WOUND_DATA		*tmpwound = NULL;
	LODGED_OBJECT_INFO	*lodged = NULL;
	LODGED_OBJECT_INFO 	*tmplodged = NULL;
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	AFFECTED_TYPE		*af = NULL;

	if ( !name || !*name )
		return NULL;

	mysql_safe_query ("SELECT * FROM %s.pfiles WHERE name = '%s'", PFILES_DATABASE, name);
	result = mysql_store_result (database);

	if ( !result || !mysql_num_rows(result) ) {
		if ( result != NULL )
			mysql_free_result (result);
		return NULL;
	}

	row = mysql_fetch_row (result);

	if ( !row ) {
		return NULL;
	}

	ch = new_char(1);
	clear_char(ch);
		
	ch->tname = str_dup(row[0]);
	ch->name = str_dup(row[1]);
	ch->pc->account = str_dup(row[2]);
	ch->short_descr = str_dup(row[3]);
	ch->long_descr = str_dup(row[4]);
	ch->description = str_dup(row[5]);
	ch->pc->msg = str_dup(row[6]);
	ch->pc->creation_comment = str_dup(row[7]);
	ch->pc->create_state = strtol(row[8], NULL, 10);
	ch->pc->nanny_state = strtol(row[9], NULL, 10);
	ch->pc->role = strtol(row[10], NULL, 10);

	if ( cmp_strn (row[13], "~", 1) && cmp_strn (row[13], " ", 1) ) {
		if ( !ch->pc->special_role )
			CREATE (ch->pc->special_role, ROLE_DATA, 1);
		ch->pc->special_role->summary = str_dup (row[11]);
		ch->pc->special_role->body = str_dup (row[12]);
		ch->pc->special_role->date = str_dup (row[13]);
		ch->pc->special_role->poster = str_dup (row[14]);
		ch->pc->special_role->cost = strtol(row[15], NULL, 10);
	}

	ch->pc->app_cost = strtol(row[16], NULL, 10);
	ch->pc->level = strtol(row[17], NULL, 10);
	ch->sex = strtol(row[18], NULL, 10);
	ch->deity = strtol(row[19], NULL, 10);
	ch->race = strtol(row[20], NULL, 10);
	ch->in_room = strtol(row[21], NULL, 10);

	ch->str = strtol(row[22], NULL, 10);
	ch->intel = strtol(row[23], NULL, 10);
	ch->wil = strtol(row[24], NULL, 10);
	ch->con = strtol(row[25], NULL, 10);
	ch->dex = strtol(row[26], NULL, 10);
	ch->aur = strtol(row[27], NULL, 10);
	ch->agi = strtol(row[28], NULL, 10);
	ch->pc->start_str = strtol(row[29], NULL, 10);
	ch->pc->start_intel = strtol(row[30], NULL, 10);
	ch->pc->start_wil = strtol(row[31], NULL, 10);
	ch->pc->start_con = strtol(row[32], NULL, 10);
	ch->pc->start_dex = strtol(row[33], NULL, 10);
	ch->pc->start_aur = strtol(row[34], NULL, 10);
	ch->pc->start_agi = strtol(row[35], NULL, 10);
	ch->tmp_str = ch->str;
	ch->tmp_intel = ch->intel;
	ch->tmp_wil = ch->wil;
	ch->tmp_con = ch->con;
	ch->tmp_dex = ch->dex;
	ch->tmp_aur = ch->aur;
	ch->tmp_agi = ch->agi;

	ch->time.played = strtol(row[36], NULL, 10);
	ch->time.birth = strtol(row[37], NULL, 10);
	ch->time.logon = time(0);

	/* 	Time? row[38]		*/

	ch->offense = strtol(row[39], NULL, 10);
	ch->hit = strtol(row[40], NULL, 10);
	ch->max_hit = strtol(row[41], NULL, 10);
	ch->nat_attack_type = strtol(row[42], NULL, 10);
	ch->move = strtol(row[43], NULL, 10);
	ch->max_move = strtol(row[44], NULL, 10);
	ch->circle = strtol(row[45], NULL, 10);
	ch->ppoints = strtol(row[46], NULL, 10);
	ch->fight_mode = strtol(row[47], NULL, 10);
	ch->color = strtol(row[48], NULL, 10);
	ch->speaks = strtol(row[49], NULL, 10);
	ch->flags = strtol(row[50], NULL, 10);
	ch->plr_flags = strtol(row[51], NULL, 10);
	ch->pc->boat_virtual = strtol(row[52], NULL, 10);
	ch->speed = strtol(row[53], NULL, 10);
	ch->pc->mount_speed = strtol(row[54], NULL, 10);
	ch->pc->sleep_needed = strtol(row[55], NULL, 10);
	ch->pc->auto_toll = strtol(row[56], NULL, 10);
	ch->coldload_id = strtol(row[57], NULL, 10);
	ch->affected_by = strtol(row[58], NULL, 10);

	if ( str_cmp (row[59], "~") && str_cmp (row[59], " ") ) {
		while ( 1 ) {
			if ( !*row[59] )
				break;
			(void)get_line (&row[59], buf);
			if ( !*buf || !str_cmp (buf, "~") || !str_cmp (buf, " ") )
				break;
			sscanf (buf, "%s ", buf2);
			if ( !*buf2 )
				break;

			if ( !str_cmp (buf2, "Subcraft") ) {
				sscanf (buf, "Subcraft '%s'", buf2);
				buf2 [strlen(buf2) - 1] = '\0';
				for ( craft = crafts; craft && str_cmp (craft->subcraft_name, buf2);
					craft = craft->next )
					;
				if ( !craft )
					continue;
				for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ )
					if ( !get_affect (ch, i) )
						break;
				if ( i > CRAFT_LAST )
					continue;
				magic_add_affect (ch, i, -1, 0, 0, 0, 0);
				af = get_affect (ch, i);
				af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);
				af->a.craft->subcraft = craft;
			}
			else if ( !str_cmp (buf2, "Affect") ) {
				af = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);		

				sscanf (buf, "Affect %d %d %d %d %d %d %d\n",
					&af->type,
					&af->a.spell.duration,
					&af->a.spell.modifier,
					&af->a.spell.location,
					&af->a.spell.bitvector,
					&af->a.spell.sn,
					&af->a.spell.t);
				af->next = NULL;
       	                 	if ( af->a.spell.location <= APPLY_CON )
       	                        	affect_to_char (ch, af);
       	                 	else if ( af->a.spell.location >= FIRST_APPLY_SKILL &&
       	                                af->a.spell.location <= LAST_APPLY_SKILL )
                                	affect_to_char (ch, af);  
                        	else {
                                	af->next = ch->hour_affects;
                                	ch->hour_affects = af;
                        	}
			}
		}
	}

	ch->age = strtol(row[60], NULL, 10);
	ch->intoxication = strtol(row[61], NULL, 10);
	ch->hunger = strtol(row[62], NULL, 10);
	ch->thirst = strtol(row[63], NULL, 10);
	ch->height = strtol(row[64], NULL, 10);
	ch->frame = strtol(row[65], NULL, 10);
	ch->damage = strtol(row[66], NULL, 10);
	ch->lastregen = strtol(row[67], NULL, 10);
	ch->last_room = strtol(row[68], NULL, 10);
	ch->mana = strtol(row[69], NULL, 10);
	ch->max_mana = strtol(row[70], NULL, 10);
	ch->pc->last_logon = strtol(row[71], NULL, 10);
	ch->pc->last_logoff = strtol(row[72], NULL, 10);
	ch->pc->last_disconnect = strtol(row[73], NULL, 10);
	ch->pc->last_connect = strtol(row[74], NULL, 10);
	ch->pc->last_died = strtol(row[75], NULL, 10);

	if ( strtol(row[76], NULL, 10) > 0 )
		SET_BIT (ch->affected_by, AFF_HOODED);

	if ( strlen(row[77]) > 1 )
		ch->pc->imm_enter = str_dup (row[77]);
	if ( strlen(row[78]) > 1 )
		ch->pc->imm_leave = str_dup (row[78]);
	if ( strlen(row[79]) > 1 )
		ch->pc->site_lie = str_dup (row[79]);
	if ( strlen(row[80]) > 1 && str_cmp (row[80], "(null)") )
		ch->voice_str = str_dup (row[80]);

	if ( str_cmp (row[81], "~") && str_cmp (row[81], " ") ) {
		while ( 1 ) {
			row[81] = one_argument (row[81], buf);
			row[81] = one_argument (row[81], buf2);
			if ( !*buf2 )
				break;
			add_clan_id (ch, buf2, buf);
		}
	}

	if ( str_cmp (row[82], "~") && str_cmp (row[82], " ") ) {
		while ( 1 ) {
			if ( !row[82] || !*row[82] )
				break;
			(void)get_line (&row[82], buf);
			if ( !*buf || !str_cmp (buf, "~") || !str_cmp (buf, " ") )
				break;
			sscanf (buf, "%s %d\n", buf2, &lev);
			if ( !*buf2 )
				continue;
			if ( (ind = skill_index_lookup (buf2)) == -1 )
				continue;
			ch->skills [ind] = lev;
			ch->pc->skills [ind] = ch->skills [ind];
		}
	}

	if ( str_cmp (row[83], "~") && str_cmp (row[83], " ") ) {
		while ( 1 ) {
			if ( !row[83] || !*row[83] )
				break;
			(void)get_line (&row[83], buf);
			if ( !*buf )
				break;

			sscanf (buf, "%s %s %s %s %d %d %d %d %d %d %d", location,
			type, severity, wound_name, &damage, &bleeding, &poison, &infection,
			&healerskill, &lasthealed, &lastbled);

			CREATE (wound, WOUND_DATA, 1);
			wound->next = NULL;

			wound->location = str_dup (location);
			wound->type = str_dup (type);
			wound->severity = str_dup (severity);
			wound->name = str_dup (wound_name);
			wound->damage = damage;
			wound->bleeding = bleeding;
			wound->poison = poison;
			wound->infection = infection;
			wound->healerskill = healerskill;
			wound->lasthealed = lasthealed;
			wound->lastbled = lastbled;

			if ( !ch->wounds )
				ch->wounds = wound;
			else {
				tmpwound = ch->wounds;
				while ( tmpwound->next )
					tmpwound = tmpwound->next;
				tmpwound->next = wound;
			}
		}
	}

	if ( str_cmp (row[84], "~") && str_cmp (row[84], " ") ) {
		while ( 1 ) {
			if ( !row[84] || !*row[84] )
				break;
			(void)get_line (&row[84], buf);
			if ( !*buf )
				break;

			sscanf (buf, "%s %d", location, &i);

			if ( !*location )
				continue;

			CREATE (lodged, LODGED_OBJECT_INFO, 36);
			lodged->next = NULL;

			lodged->location = str_dup (location);
			lodged->vnum = i;

			if ( !ch->lodged )
				ch->lodged = lodged;
			else {
				tmplodged = ch->lodged;
				while ( tmplodged->next )
					tmplodged = tmplodged->next;
				tmplodged->next = lodged;
			}
		}
	}

	ch->writes = strtol(row[85], NULL, 10);

	ch->pc->profession = strtol(row[86], NULL, 10);

	ch->was_in_room = strtol(row[87], NULL, 10);

	if ( row[88] && strlen(row[88]) > 1 && str_cmp (row[88], "(null)") )
		ch->travel_str = str_dup (row[88]);

	ch->pc->last_rpp = strtol(row[89], NULL, 10);

	fix_offense (ch);

	load_dreams (ch);

	if ( !ch->coldload_id )
		ch->coldload_id = get_next_coldload_id (1);

	if ( ch->race == 28 ) {
		ch->max_hit = 200 + GET_CON(ch)*CONSTITUTION_MULTIPLIER;
		ch->armor = 3;
	} else {
		ch->max_hit = 50 + GET_CON(ch)*CONSTITUTION_MULTIPLIER;
		ch->armor = 0;
	}

	if ( !ch->max_mana && (ch->skills [SKILL_BLACK_WISE] || ch->skills [SKILL_WHITE_WISE] || ch->skills [SKILL_GREY_WISE]) ) {
		ch->max_mana = 100;
		ch->mana = ch->max_mana;
	}

	ch->pc->is_guide = is_guide(ch->pc->account);

	if ( ch->speaks == SKILL_HEALING )
		ch->speaks = SKILL_SPEAK_WESTRON;

	mysql_free_result (result);

	return ch;
}

void save_char_mysql (CHAR_DATA *ch)
{
	AFFECTED_TYPE	*af = NULL;
	WOUND_DATA		*wound = NULL;
	LODGED_OBJECT_INFO	*lodged_obj = NULL;
	MYSQL_RES		*result = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			wounds [MAX_STRING_LENGTH] = {'\0'};
	char			lodged [MAX_STRING_LENGTH] = {'\0'};
	char			skills_buf [MAX_STRING_LENGTH] = {'\0'};
	char			affects [MAX_STRING_LENGTH] = {'\0'};
	int				i = 0;
	int				hooded = 0;

	if ( IS_NPC (ch) || IS_SET (ch->flags, FLAG_GUEST) )
		return;

	if ( ch->wounds ) {
		*buf = '\0';
		for ( wound = ch->wounds; wound; wound = wound->next ){
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s %s %s %s %d %d %d %d %d %d %d\n",
				wound->location, wound->type,
				wound->severity, wound->name,
				wound->damage, wound->bleeding,
				wound->poison,
				wound->infection, wound->healerskill,
				wound->lasthealed,
				wound->lastbled);
		}

		mysql_real_escape_string (database, wounds, buf, strlen(buf));

	}

	*buf = '\0';
	for ( i = 0; i <= MAX_SKILLS; i++ ) {
		if ( real_skill (ch, i) ){
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%-15s   %d\n", skill_data[i].skill_name, real_skill (ch, i));
		}
	}

	snprintf (skills_buf, MAX_STRING_LENGTH, "%s", buf);

	*buf = '\0';
	for ( af = ch->hour_affects; af; af = af->next ) {
		if ( (af->type < MAGIC_CLAN_MEMBER_BASE ||
			  af->type > MAGIC_CLAN_OMNI_BASE + MAX_CLANS) &&
			 (af->type < CRAFT_FIRST ||
			  af->type > CRAFT_LAST) &&
			  af->type != MAGIC_CLAN_NOTIFY &&
			  af->type != MAGIC_NOTIFY &&
			  af->type != MAGIC_WATCH1 &&
			  af->type != MAGIC_WATCH2 &&
			  af->type != MAGIC_WATCH3 &&
			  af->type != MAGIC_GUARD &&
			  af->type != AFFECT_SHADOW &&
			 (af->type > CRAFT_LAST || af->type < CRAFT_FIRST) ){

			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,
				"Affect	%d %d %d %d %d %d %d\n",
				af->type, af->a.spell.duration,
				af->a.spell.modifier,
				af->a.spell.location,
				af->a.spell.bitvector,
				af->a.spell.sn,
				af->a.spell.t);
		}

		else if ( af->type >= CRAFT_FIRST && af->type <= CRAFT_LAST ){
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,
					  "Subcraft     '%s'\n",
					  af->a.craft->subcraft->subcraft_name);
		}
	}
	
	snprintf (affects, MAX_STRING_LENGTH, "%s", buf);

	*buf = '\0';
	for ( lodged_obj = ch->lodged; lodged_obj; lodged_obj = lodged_obj->next ){
		snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s %d\n", lodged_obj->location, lodged_obj->vnum);
	}
	
	snprintf (lodged, MAX_STRING_LENGTH, "%s", buf);

	mysql_safe_query ("SELECT name FROM %s.pfiles WHERE name = '%s'", PFILES_DATABASE, ch->tname);
	result = mysql_store_result (database);

	if ( result && mysql_num_rows(result) >= 1 ) {	/* Update an existing PC  record. */
		mysql_free_result (result);
		mysql_safe_query ("UPDATE %s.pfiles SET keywords = '%s', account = '%s', sdesc = '%s', ldesc = '%s', description = '%s', msg = '%s', create_comment = '%s', create_state = %d, "
	"nanny_state = %d, role = %d, role_summary = '%s', role_body = '%s', role_date = '%s', role_poster = '%s', role_cost = %d, app_cost = %d, level = %d, sex = %d, deity = %d, "
	"race = %d, room = %d, str = %d, intel = %d, wil = %d, con = %d, dex = %d, aur = %d, agi = %d, start_str = %d, start_intel = %d, start_wil = %d, start_con = %d, start_dex = %d, start_aur = %d, "
	"start_agi = %d, played = %d, birth = %d, time = %d, offense = %d, hit = %d, maxhit = %d, nat_attack_type = %d, move = %d, maxmove = %d, circle = %d, ppoints = %d, fightmode = %d, color = %d, "
	"speaks = %d, flags = %d, plrflags = %d, boatvnum = %d, speed = %d, mountspeed = %d, sleepneeded = %d, autotoll = %d, coldload = %d, affectedby = %d, "
	"affects = '%s', age = %d, intoxication = %d, hunger = %d, thirst = %d, height = %d, frame = %d, damage = %d, lastregen = %d, lastroom = %d, harness = %d, maxharness = %d, "
	"lastlogon = %d, lastlogoff = %d, lastdis = %d, lastconnect = %d, lastdied = %d, hooded = %d, immenter = '%s', immleave = '%s', sitelie = '%s', voicestr = '%s', clans = '%s', skills = '%s', "
	"wounds = '%s', lodged = '%s', writes = %d, profession = %d, was_in_room = %d, travelstr = '%s', last_rpp = %d WHERE name = '%s'",

	PFILES_DATABASE, ch->name, ch->pc->account, ch->short_descr, ch->long_descr, ch->description, ch->pc->msg, ch->pc->creation_comment, ch->pc->create_state, 
	ch->pc->nanny_state, 0, ch->pc->special_role ? ch->pc->special_role->summary : "~", ch->pc->special_role ? ch->pc->special_role->body : "~", ch->pc->special_role ? ch->pc->special_role->date : "~", ch->pc->special_role ? ch->pc->special_role->poster : "~", ch->pc->special_role ? ch->pc->special_role->cost : 0, ch->pc->app_cost, ch->pc->level, ch->sex, ch->deity, 
	ch->race, ch->in_room, ch->str, ch->intel, ch->wil, ch->con, ch->dex, ch->aur, ch->agi, ch->pc->start_str, ch->pc->start_intel, ch->pc->start_wil, ch->pc->start_con, ch->pc->start_dex, ch->pc->start_aur, 
	ch->pc->start_agi, (int) (ch->time.played + time (0) - ch->time.logon), (int) ch->time.birth, (int) time(0), ch->offense, ch->hit, ch->max_hit, ch->nat_attack_type, ch->move, ch->max_move, ch->circle, ch->ppoints, ch->fight_mode, ch->color,
	ch->speaks, (int)ch->flags, (int)ch->plr_flags, ch->pc->boat_virtual, ch->speed, ch->pc->mount_speed, (int)ch->pc->sleep_needed, ch->pc->auto_toll, (int)ch->coldload_id, (int)ch->affected_by, 
	affects, ch->age, ch->intoxication, ch->hunger, ch->thirst, ch->height, ch->frame, ch->damage, (int) ch->lastregen, ch->last_room, ch->mana, ch->max_mana, 
	(int)ch->pc->last_logon, (int)ch->pc->last_logoff, (int)ch->pc->last_disconnect, (int)ch->pc->last_connect, (int)ch->pc->last_died, hooded, ch->pc->imm_enter, ch->pc->imm_leave, ch->pc->site_lie, ch->voice_str, ch->clans, skills_buf, 
	wounds, lodged, ch->writes, ch->pc->profession, ch->was_in_room, ch->travel_str, (int)ch->pc->last_rpp, ch->tname);
		save_dreams (ch);
	}
	else {	/* New PC record. */
		if ( result ){
			mysql_free_result (result);
		}

	mysql_safe_query ("DELETE FROM %s.pfiles WHERE name = '%s'", PFILES_DATABASE, ch->tname);
	mysql_safe_query ("INSERT INTO %s.pfiles (name, keywords, account, sdesc, ldesc, description, msg, create_comment, create_state, "
	"nanny_state, role, role_summary, role_body, role_date, role_poster, role_cost, app_cost, level, sex, deity, "
	"race, room, str, intel, wil, con, dex, aur, agi, start_str, start_intel, start_wil, start_con, start_dex, start_aur, "
	"start_agi, played, birth, time, offense, hit, maxhit, nat_attack_type, move, maxmove, circle, ppoints, fightmode, color, "
	"speaks, flags, plrflags, boatvnum, speed, mountspeed, sleepneeded, autotoll, coldload, affectedby, "
	"affects, age, intoxication, hunger, thirst, height, frame, damage, lastregen, lastroom, harness, maxharness, "
	"lastlogon, lastlogoff, lastdis, lastconnect, lastdied, hooded, immenter, immleave, sitelie, voicestr, clans, skills, "
	"wounds, lodged, writes, profession, was_in_room, travelstr, last_rpp) VALUES "
"('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, "
"%d, %d, '%s', '%s', '%s', '%s', %d, %d, %d, %d, %d, "
"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
"'%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
"%d, %d, %d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', "
"'%s', '%s', %d, %d, %d, '%s', %d)", 

	PFILES_DATABASE, ch->tname, ch->name, ch->pc->account, ch->short_descr, ch->long_descr, ch->description, ch->pc->msg, ch->pc->creation_comment, ch->pc->create_state, 
	ch->pc->nanny_state, 0, ch->pc->special_role ? ch->pc->special_role->summary : "~", ch->pc->special_role ? ch->pc->special_role->body : "~", ch->pc->special_role ? ch->pc->special_role->date : "~", ch->pc->special_role ? ch->pc->special_role->poster : "~", ch->pc->special_role ? ch->pc->special_role->cost : 0, ch->pc->app_cost, ch->pc->level, ch->sex, ch->deity, 
	ch->race, ch->in_room, ch->str, ch->intel, ch->wil, ch->con, ch->dex, ch->aur, ch->agi, ch->pc->start_str, ch->pc->start_intel, ch->pc->start_wil, ch->pc->start_con, ch->pc->start_dex, ch->pc->start_aur, 
	ch->pc->start_agi, (int) (ch->time.played + time (0) - ch->time.logon), (int) ch->time.birth, (int) time(0), ch->offense, ch->hit, ch->max_hit, ch->nat_attack_type, ch->move, ch->max_move, ch->circle, ch->ppoints, ch->fight_mode, ch->color,
	ch->speaks, (int)ch->flags, (int)ch->plr_flags, ch->pc->boat_virtual, ch->speed, ch->pc->mount_speed, (int)ch->pc->sleep_needed, ch->pc->auto_toll, (int)ch->coldload_id, (int)ch->affected_by, 
	affects, ch->age, ch->intoxication, ch->hunger, ch->thirst, ch->height, ch->frame, ch->damage, (int) ch->lastregen, ch->last_room, ch->mana, ch->max_mana, 
	(int)ch->pc->last_logon, (int)ch->pc->last_logoff, (int)ch->pc->last_disconnect, (int)ch->pc->last_connect, (int)ch->pc->last_died, hooded, ch->pc->imm_enter, ch->pc->imm_leave, ch->pc->site_lie, ch->voice_str, ch->clans, skills_buf, 
	wounds, lodged, ch->writes, ch->pc->profession, ch->was_in_room, ch->travel_str, (int)ch->pc->last_rpp);
	}

	return;
}

int check_account_flags (char *host)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int			flag = 0;

	mysql_safe_query ("SELECT account_flags FROM forum_users WHERE user_last_ip = '%s'", host);
	result = mysql_store_result (database);

	while ( (row = mysql_fetch_row(result)) )
		flag = strtol(row[0], NULL, 10);

	mysql_free_result (result);

	return flag;
}

char *reference_ip (char *guest_name, char *host)
{
	static char	gname [MAX_STRING_LENGTH];
	MYSQL_RES	*result;
	MYSQL_ROW	row;

	if ( strstr (host, "www.yourmud.org") )
		return guest_name;

	mysql_safe_query ("SELECT username FROM forum_users WHERE user_last_ip = '%s'", host);
	result = mysql_store_result (database);

	while ( (row = mysql_fetch_row(result)) ) {
		snprintf (gname, MAX_STRING_LENGTH, "%s", row[0]);
		mysql_free_result (result);
		return gname;
	}

	mysql_free_result (result);

	return guest_name;	
}

void load_reboot_mobiles ()
{
	FILE		*fp = NULL;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	CHAR_DATA	*mob = NULL;
	int			i = 0;
	int			j = 0;
	char		query [MAX_STRING_LENGTH] = {'\0'};
	
	mysql_safe_query ("SELECT * FROM reboot_mobiles");
	result = mysql_store_result (database);

	while ( (row = mysql_fetch_row(result)) ) {
		i++;
		j++;
		snprintf (query, MAX_STRING_LENGTH,"Mobile #%d, Load #%d", (int)strtol(row[2], NULL, 10), j);
		snprintf (query, MAX_STRING_LENGTH,"save/reboot/%s", row[2]);
		fp = fopen (query, "r");
		if ( !fp )
			continue;
		mob = load_a_saved_mobile (strtol(row[0], NULL, 10), fp, FALSE);
		if ( mob )
			char_to_room (mob, strtol(row[1], NULL, 10));
		fclose (fp);
	}

	mysql_free_result (result);
	
	return;
}

void load_stayput_mobiles ()
{
	FILE		*fp = NULL;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	CHAR_DATA	*mob = NULL;
	char		query [MAX_STRING_LENGTH] = {'\0'};

	if ( port != PLAYER_PORT )
		return;

	mysql_safe_query ("SELECT * FROM stayput_mobiles");
	result = mysql_store_result (database);

	system_log ("Loading stayput mobiles...", FALSE);

	while ( (row = mysql_fetch_row(result)) ) {
		snprintf (query, MAX_STRING_LENGTH,"save/mobiles/%s", row[2]);
		fp = fopen (query, "r");
		if ( !fp )
			continue;
		mob = load_a_saved_mobile (strtol(row[0], NULL, 10), fp, FALSE);
		if ( mob )
			char_to_room (mob, strtol(row[1], NULL, 10));
		mob = NULL;
		fclose (fp);
	}

	mysql_free_result (result);
	
	return;
}

void save_reboot_mobiles ()
{
	CHAR_DATA	*mob = NULL;
	FILE		*fp = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !finished_booting )
		return;

	for ( mob = character_list; mob; mob = mob->next ) {
		if ( mob->deleted )
			continue;
		if ( !IS_NPC (mob) && mob->desc && !mob->desc->original )
			continue;
		if ( IS_RIDEE(mob) && !IS_NPC (mob->mount) )
			continue;
		if ( IS_HITCHEE(mob) && !IS_NPC (mob->hitcher) )
			continue;
		if ( mob->deleted )
			continue;
		if ( !mob->room )
			continue;
		snprintf (buf, MAX_STRING_LENGTH,  "save/reboot/%d", mob->coldload_id);
		fp = fopen (buf, "w");
		save_mobile (mob, fp, "STAYPUT", 0);
		fclose (fp);

		mysql_safe_query ("DELETE FROM reboot_mobiles WHERE coldload_id = %d", mob->coldload_id);
		mysql_safe_query ("INSERT INTO reboot_mobiles VALUES (%d, %d, %d)", mob->mob->virtual, mob->in_room, mob->coldload_id);
	}
	
	return;
}

void save_stayput_mobiles ()
{
	CHAR_DATA	*mob = NULL;
	FILE		*fp = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !finished_booting )
		return;

	for ( mob = character_list; mob; mob = mob->next ) {
		if ( mob->deleted )
			continue;
		if ( !IS_SET (mob->act, ACT_STAYPUT) )
			continue;
		if ( !IS_NPC (mob) && mob->desc && !mob->desc->original )
			continue;
		if ( IS_HITCHEE (mob) )
			continue;
		if ( IS_RIDEE (mob) )
			continue; 
		snprintf (buf, MAX_STRING_LENGTH,  "save/mobiles/%d", mob->coldload_id);
		fp = fopen (buf, "w");
		save_mobile (mob, fp, "STAYPUT", 0);
		fclose (fp);

		mysql_safe_query ("DELETE FROM stayput_mobiles WHERE coldload_id = %d", mob->coldload_id);
		mysql_safe_query ("INSERT INTO stayput_mobiles VALUES (%d, %d, %d)", mob->mob->virtual, mob->in_room, mob->coldload_id);
	}
	
	return;
}

void insert_newsletter_into_website (int timestamp, char *newsletter)
{
	mysql_safe_query ("INSERT INTO newsletters VALUES (%d, '%s')", timestamp, newsletter);
	
	return;
}

void update_website_statistics (void)
{
	ROOM_DATA 	*room = NULL;
	CHAR_DATA	*mob = NULL;
	OBJ_DATA	*obj = NULL;
	int			craft_tot = 0;
	int			objects = 0;
	int			mobs = 0;
	int			rooms = 0;

	for ( obj = full_object_list; obj; obj = obj->lnext )
		if ( !obj->deleted )
			objects++;

	for ( mob = full_mobile_list; mob; mob = mob->mob->lnext )
		if ( !mob->deleted )
			mobs++;

	for ( room = full_room_list; room; room = room->lnext )
		rooms++;

	mysql_safe_query ("DELETE FROM building_totals");

	mysql_safe_query ("INSERT INTO building_totals VALUES (%d, %d, %d, %d)", rooms, objects, mobs, craft_tot);

	return;
}

void load_tracks (void)
{
	ROOM_DATA	*room = NULL;
	TRACK_DATA	*track = NULL;
	TRACK_DATA	*tmp_track = NULL;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;

	mysql_safe_query ("SELECT * FROM tracks");
	result = mysql_store_result (database);

	if ( !result )
		return;

	while ( (row = mysql_fetch_row(result)) ) {
		if ( !(room = vtor(strtol(row[0], NULL, 10))) )
			continue;
		CREATE (track, TRACK_DATA, 1);
		track->next = NULL;
		if ( !room->tracks )
			room->tracks = track;
		else for ( tmp_track = room->tracks; tmp_track; tmp_track = tmp_track->next ) {
			if ( !tmp_track->next ) {
				tmp_track->next = track;
				break;
			}
		}
		track->race = strtol(row[1], NULL, 10);
		track->from_dir = strtol(row[2], NULL, 10);
		track->to_dir = strtol(row[3], NULL, 10);
		track->hours_passed = strtol(row[4], NULL, 10);
		track->speed = strtol(row[5], NULL, 10);
		track->flags = strtol(row[6], NULL, 10);
	}

	mysql_free_result (result);
	result = NULL;
	
	return;
}

void save_tracks (void)
{
	ROOM_DATA	*room = NULL;
	TRACK_DATA	*track = NULL;


	if ( port != PLAYER_PORT )
		return;

	mysql_safe_query ("DELETE FROM tracks");

	for ( room = full_room_list; room; room = room->lnext ) {
		if ( !room->tracks ){
			continue;
		}
		for ( track = room->tracks; track; track = track->next ) {
			if ( !track ){
				continue;
			}
			if ( !IS_SET (track->flags, PC_TRACK) ){
				continue;
			}

			mysql_safe_query ("INSERT INTO tracks VALUES(%d, %d, %d, %d, %d, %d, %d)", room->virtual, track->race, track->from_dir, track->to_dir, track->hours_passed, track->speed, (int) track->flags); 
		}
	}
	return;
}

void save_hobbitmail_message (ACCOUNT_DATA *account, MUDMAIL_DATA *message)
{
	char	query [MAX_STRING_LENGTH] = {'\0'};

	if ( !account || !account->name || !message )
		return;

	mysql_safe_query (query, "INSERT INTO hobbitmail (account, flags, from_line, from_account, sent_date, subject, message, timestamp)"
			" VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s', UNIX_TIMESTAMP())",
	account->name, (int) message->flags, message->from, message->from_account, message->date, message->subject, message->message);
	
	return;
}

void save_account (ACCOUNT_DATA *account)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	DESCRIPTOR_DATA	*td = NULL;
	char		name [256];
	int			id = 0;
	int			gid = 0;

	result = NULL;

	if ( !account || !account->name || !*account->name )
		return;

	snprintf (name, MAX_STRING_LENGTH, "%s", account->name);

	/* Store the updated account info to the database */

	mysql_safe_query ("SELECT username FROM forum_users WHERE username = '%s'", account->name);
	result = mysql_store_result(database);

	if ( mysql_num_rows(result) >= 1 ) {
		mysql_safe_query ("UPDATE forum_users SET user_password = '%s', roleplay_points = %d, user_color = %d, user_last_ip = '%s', user_regdate = %d, user_subscription = %d, user_email = '%s', account_flags = %d, last_rpp = %d WHERE username = '%s'", account->pwd, account->roleplay_points, account->color, account->last_ip, account->created_on, account->newsletter, account->email, account->flags, account->last_rpp, account->name);
		mysql_free_result (result); 		
	}
	else {
		mysql_safe_query ("SELECT user_id FROM forum_users ORDER BY user_id DESC LIMIT 1");
		result = mysql_store_result (database);
		if ( result && mysql_num_rows(result) >= 1 ) {
			row = mysql_fetch_row (result);
			id = strtol(row[0], NULL, 10);
			mysql_free_result(result);
		}
		else id = 0;
		id++;

		mysql_safe_query ("INSERT INTO forum_users (user_id, username, user_password, roleplay_points, user_color, user_last_ip, user_regdate, user_subscription, user_email, user_viewemail, user_notify_pm, user_popup_pm, user_notify) VALUES(%d, '%s', '%s', %d, %d, '%s', %d, %d, '%s', '1', '1', '1', '0')", id, account->name, account->pwd, account->roleplay_points, account->color, account->last_ip, account->created_on, account->newsletter, account->email);
		mysql_safe_query ("UPDATE forum_users SET user_viewemail = 0 WHERE username = '%s'", account->name);
		mysql_safe_query ("SELECT group_id FROM forum_user_group ORDER BY group_id DESC LIMIT 1");

		result = mysql_store_result(database);
		if ( result && mysql_num_rows(result) >= 1 ) {
			row = mysql_fetch_row(result);
			gid = strtol(row[0], NULL, 10);
			mysql_free_result(result);
		}
		else gid = 0;
		gid++;
		mysql_safe_query ("INSERT INTO forum_groups (group_name, group_description, group_single_user, group_moderator) VALUES ('', 'Personal User', 1, 0)");
		mysql_safe_query ("INSERT INTO forum_user_group (user_id, group_id, user_pending) VALUES (%d, %d, 0)", id, gid);
		return;
	}

	/* Update all accounts of this name currently logged in with new info */

	for ( td = descriptor_list; td; td = td->next ) {
		if ( !td->account || !td->account->name || !*td->account->name )
			continue;
		if ( str_cmp (td->account->name, name) )
			continue;
		if ( td->account == account )
			continue;
		free_account (td->account);
		td->account = NULL;
		td->account = load_account (name);
	}
	
	return;
} 

ACCOUNT_DATA *new_account (void)
{
	ACCOUNT_DATA	*account = NULL;

	account = (ACCOUNT_DATA *)alloc ((int)sizeof (ACCOUNT_DATA), 19);

	account->name = NULL;
	account->pwd = NULL;
	account->email = NULL;
	account->last_ip = NULL;
	account->pc = NULL;
	account->roleplay_points = 0;
	account->created_on = 0;
	account->pwd_attempt = 0;
	account->color = 0;
	account->web_login = 0;
	account->admin = FALSE;
	account->newsletter = TRUE;
	account->flags = 0;
	account->forum_posts = 0;
	account->timezone = 0;
	account->deleted = 0;
	account->code = 0;
	account->last_rpp = 0;
	account->next = NULL;

	return account;
}


/*                                                                          *
 * function: load_account                                                   *
 *                                                                          *
 *                                                                          */
ACCOUNT_DATA *load_account (char *name)
{
	ACCOUNT_DATA 		*account = NULL;
	STORED_PC_DATA		*pc = NULL;
	char				buf [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES			*result = NULL;
	MYSQL_ROW			row;
	
	if ( !*name )
		return NULL;

	if ( islower (*name) )
		CAP (name);

	if ( !str_cmp (name, "Anonymous") )
		return NULL;

	mysql_safe_query ("SELECT roleplay_points,user_password,user_last_ip,user_subscription,user_regdate,user_email,user_color,account_flags,user_posts,user_timezone,downloaded_code,last_rpp FROM forum_users WHERE username = '%s'", name);

	if ( ( result = mysql_store_result (database) ) == NULL ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Error: load_account(): %s", mysql_error (database));
		system_log (buf, TRUE);
		return NULL;
	}
	if ( mysql_num_rows(result) == 0 ) {
		if ( result )
			mysql_free_result (result);
		return NULL;
	}
	if ( mysql_num_fields(result) < 10 ) {
		if ( result )
			mysql_free_result (result);
		return NULL;
	}

   	CREATE (account, ACCOUNT_DATA, 1);

	row = mysql_fetch_row (result);

	account->name = str_dup (name);
	account->newsletter = TRUE;
	account->roleplay_points = strtol(row[0], NULL, 10);
	account->pwd = str_dup (row[1]);
	account->last_ip = str_dup (row[2]);
	account->newsletter = strtol(row[3], NULL, 10);
	account->created_on = strtol(row[4], NULL, 10);
	account->email = str_dup (row[5]);
	account->color = strtol(row[6], NULL, 10);
	account->flags = strtol(row[7], NULL, 10);
	account->forum_posts = strtol(row[8], NULL, 10);
	account->timezone = strtol(row[9], NULL, 10);
	if ( row[10] && *row[10] )
		account->code = strtol(row[10], NULL, 10);
	else account->code = 0;
	account->last_rpp = strtol(row[11], NULL, 10);

	account->pc = NULL;
	account->admin = FALSE;
	account->deleted = 0;

	mysql_free_result (result);
	result = NULL;

        mysql_safe_query ("SELECT name,level,create_state FROM %s.pfiles WHERE account = '%s' ORDER BY birth ASC", PFILES_DATABASE, account->name);

	if ( ( result = mysql_store_result (database) ) != NULL ) {
		while ( (row = mysql_fetch_row(result)) ) {
			if ( !account->pc ) {
				CREATE (account->pc, STORED_PC_DATA, 1);
				pc = account->pc;
			}       
			else {  
				CREATE (pc->next, STORED_PC_DATA, 1);
				pc = pc->next;
			}
			pc->name = str_dup (row[0]);
			pc->level = strtol(row[1], NULL, 10);
			if ( pc->level >= 1 )
				account->admin = TRUE;
			pc->state = strtol(row[2], NULL, 10);
			pc->next = NULL;
                }
              	mysql_free_result (result);
	        result = NULL;
        }
	else {
		snprintf (buf, MAX_STRING_LENGTH,  "Error: load_account(): %s", mysql_error ( database ));
		system_log (buf, TRUE);
		return NULL;
	}

	return account;
}

void save_roles (void)
{
	char		text [MAX_STRING_LENGTH] = {'\0'};
	char		date_buf [MAX_STRING_LENGTH] = {'\0'};
	char		summary [MAX_STRING_LENGTH] = {'\0'};
	ROLE_DATA	*role = NULL;

	mysql_safe_query ("DELETE FROM special_roles");

	for ( role = role_list; role; role = role->next ) {
		*summary = '\0';
		*date_buf = '\0';
		*text = '\0';

		mysql_safe_query ("INSERT INTO special_roles (summary, poster, date, cost, body, timestamp) VALUES ('%s', '%s', '%s', %d, '%s', %d)",
		role->summary, role->poster, role->date, role->cost, role->body, role->timestamp);
	}
	
	return;
}

void reload_roles (void)
{
	ROLE_DATA	*role = NULL;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;

	mysql_safe_query ("SELECT * FROM special_roles");
	result = mysql_store_result(database);

	if ( !result || !mysql_num_rows(result) ) {
		if ( result )
			mysql_free_result (result);
		system_log ("No roles stored in database.", FALSE);
		return;
	}

	while ( (row = mysql_fetch_row (result)) ) {
		if ( !role_list ) {
			CREATE (role_list, ROLE_DATA, 1);
			role = role_list;
		}
		else {
			CREATE (role->next, ROLE_DATA, 1);
			role = role->next;
		}
		role->summary = str_dup (row[0]);
		role->poster = str_dup (row[1]);
		role->date = str_dup (row[2]);
		role->cost = strtol(row[3], NULL, 10);
		role->body = str_dup (row[4]);
		role->timestamp = strtol(row[5], NULL, 10);
		role->next = NULL;
	}

	mysql_free_result (result);
	
	return;
}

void mark_as_read (CHAR_DATA *ch, int number)
{
	mysql_safe_query ("UPDATE player_notes SET flags = 1 WHERE name = '%s' AND post_number = %d", ch->tname, number);
	
	return;
}

void display_mysql_board_message (CHAR_DATA *ch, char *board_name, int msg_num)
{
	char		query [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;

	mysql_safe_query ("SELECT * FROM boards WHERE board_name = '%s' AND post_number = %d", board_name, msg_num);
	result = mysql_store_result(database);

	if ( !result || !mysql_num_rows (result) ) {
		if ( result )
			mysql_free_result (result);
		send_to_char ("That message does not seem to exist in the database.\n", ch);
		return;
	}

	row = mysql_fetch_row (result);

	*b_buf = '\0';
	*query = '\0';

	if ( *row[4] )
		snprintf (query, MAX_STRING_LENGTH," (%s)", row[4]);
	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "\n#6Date:#0    %s%s\n", row[5], !IS_MORTAL(ch) && *query ? query : "");
	if ( !IS_MORTAL(ch) ) {
		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "#6Author:#0  %s\n", row[3]);
	}
	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "#6Subject:#0 %s\n", row[2]);
	snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, "\n%s", row[6]);

	page_string (ch->desc, b_buf);

	mysql_free_result (result);
	
	return;
}

void retrieve_mysql_board_listing (CHAR_DATA *ch, char *board_name)
{
	char		query [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;

	mysql_safe_query ("SELECT * FROM boards WHERE board_name = '%s' ORDER BY post_number DESC", board_name);
	result = mysql_store_result(database);

	if ( !result || !mysql_num_rows (result) ) {
		if ( result )
			mysql_free_result (result);
		send_to_char ("No messages found for this listing.\n", ch);
		return;
	}

	*b_buf = '\0';
	while ( (row = mysql_fetch_row (result)) ) {
		if ( strlen(row[2]) > 46 ) {
			snprintf (query, MAX_STRING_LENGTH,"%s", row[2]);
			query[43] = '.';
			query[44] = '.';
			query[45] = '.';
			query[46] = '\0';
		}
		else snprintf (query, MAX_STRING_LENGTH,"%s", row[2]);
	        snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, " #6%3d:#0 %24s - %s\n", (int)strtol(row[1], NULL, 10), row[5], query);
	}

	if ( !*b_buf ) {
		send_to_char ("There are currently no posts on this board.\n", ch);
		mysql_free_result (result);
		return;
	}
	else {
		page_string (ch->desc, b_buf);
		mysql_free_result (result);
	}
	
	return;
}

void post_to_mysql_board (DESCRIPTOR_DATA *d)
{
	char		ooc_date [MAX_STRING_LENGTH] = {'\0'};
	char		ic_date [MAX_STRING_LENGTH] = {'\0'};
	char		date_buf [MAX_STRING_LENGTH] = {'\0'};
	char		message [MAX_STRING_LENGTH] = {'\0'};
	char		subject [MAX_STRING_LENGTH] = {'\0'};
	char		board_name [MAX_STRING_LENGTH] = {'\0'};
	char		*date = NULL;
	char		*suf = NULL;
	int			free_slot = 0, day;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;

	if ( !*d->pending_message->message ) {
		send_to_char ("No message posted.\n", d->character);
		unload_message (d->pending_message);
		d->pending_message = NULL;
		return;
	}

    date = timestr(date);

	*date_buf = '\0';
        day = time_info.day + 1;
        if ( day == 1 )
                suf = "st";
        else if ( day == 2 )
                suf = "nd";
        else if ( day == 3 )
                suf = "rd";
        else if ( day < 20 )
                suf = "th";
        else if ( (day % 10) == 1 )
                suf = "st";     
        else if ( (day % 10) == 2 )
                suf = "nd";
        else if ( (day % 10) == 3 )
                suf = "rd";
        else 
                suf = "th";

        if ( time_info.holiday == 0 &&
                !(time_info.month == 1 && day == 12) &&
                !(time_info.month == 4 && day == 10) &&
                !(time_info.month == 7 && day == 11) && 
                !(time_info.month == 10 && day == 12) )
                snprintf (date_buf, MAX_STRING_LENGTH, "%d%s %s, %d SR", day, suf, month_short_name[time_info.month], time_info.year);
        else { 
                if ( time_info.holiday > 0 ) {
                        snprintf (date_buf, MAX_STRING_LENGTH, "%s, %d SR", holiday_short_names[time_info.holiday], time_info.year);
                }
                else if ( time_info.month == 1 && day == 12 )
                        snprintf (date_buf, MAX_STRING_LENGTH, "Erukyerme, %d SR", time_info.year);
                else if ( time_info.month == 4 && day == 10 )
                        snprintf (date_buf, MAX_STRING_LENGTH, "Lairemerende, %d SR", time_info.year);
                else if ( time_info.month == 7 && day == 11 )
                        snprintf (date_buf, MAX_STRING_LENGTH, "Eruhantale, %d SR", time_info.year);
                else if ( time_info.month == 10 && day == 12 )
                        snprintf (date_buf, MAX_STRING_LENGTH, "Airilaitale, %d SR", time_info.year);
        }

        if ( isalpha (*date_buf) )
                *date_buf = toupper(*date_buf);

	mysql_real_escape_string (database, ic_date, date_buf, strlen(date_buf));
	mysql_real_escape_string (database, ooc_date, date, strlen(date));
	mysql_real_escape_string (database, board_name, d->pending_message->poster, strlen(d->pending_message->poster));
	mysql_real_escape_string (database, subject, d->pending_message->subject, strlen(d->pending_message->subject));
	mysql_real_escape_string (database, message, d->pending_message->message, strlen(d->pending_message->message));

	mysql_safe_query ("SELECT * FROM boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1", board_name);
	result = mysql_store_result(database);

	if ( !result ) {
		send_to_char ("There seems to be a problem with the database listing.\n", d->character);
		return;
	}

	free_slot = 1;
	if ( (row = mysql_fetch_row (result)) ) {
		free_slot = strtol(row[1], NULL, 10);
		free_slot++;
	}

	mysql_safe_query ("INSERT INTO boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', '%s', %d)",
	d->pending_message->poster, free_slot, d->pending_message->subject, d->character->tname, date, date_buf, 
	d->pending_message->message, (int) time(0));

	if ( result )
		mysql_free_result (result);

	unload_message (d->pending_message);
	d->pending_message = NULL;
	mem_free (date);
	
	return;
}

/* This is mainly a "test" command - not really functional for any one speciifc purpose! */

void do_mysql (CHAR_DATA *ch, char *argument, int cmd)
{
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	ROOM_DATA			*room = NULL;
	CHAR_DATA			*tch = NULL;
	CHAR_DATA			*temp_char = NULL;
	OBJ_DATA			*obj = NULL;
	TRACK_DATA			*track = NULL;
	WOUND_DATA			*wound = NULL;
	WOUND_DATA			*next_wound = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		severity [MAX_STRING_LENGTH] = {'\0'};
	int			purse = 0;
	int			i = 0;
	int			time_passed = 0;
	int			num_wounds = 0;
	int			points = 0;
	struct time_info_data passed;
	char		*temp_arg = NULL;

	if ( !strstr (ch->tname, IMPLEMENTOR_ACCOUNT) ) {
		send_to_char ("Eh?\n", ch);
		return;
	}

	if ( !*argument ) {
		send_to_char ("Eh?\n", ch);
		return;
	}

	if ( !str_cmp (argument, "loop") ) {
		while (1) {
		}
	}

	if ( !str_cmp (argument, "host") ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Host: %s\nServer: %s\n Status: %s\n", mysql_get_host_info(database), mysql_get_server_info(database), mysql_stat (database));
		send_to_char (buf, ch);
		return;
	}

	if ( !str_cmp (argument, "refresh") ) {
		refresh_db_connection();
		send_to_char ("Connection refresh triggered.\n", ch);
		return;
	}

	if ( !str_cmp (argument, "test") ) {
		snprintf (buf, MAX_STRING_LENGTH,  "File name %s, line number %ld", __FILE__, (long)__LINE__);
		send_to_char (buf, ch);
		return;
	}

	if ( !cmp_strn (argument, "testcraft", 9) ) {
		argument = one_argument (argument, buf);
		argument = one_argument (argument, buf);

		if ( !(tch = load_pc (buf)) ) {
			send_to_char ("No such PC.\n", ch);
			return;
		}

		if ( !*argument ) {
			send_to_char ("Which craft did you wish to test?\n", ch);
			unload_pc (tch);
			return;
		}

		for ( craft = crafts; craft; craft = craft->next ) {
			if ( !str_cmp (craft->subcraft_name, argument) )
				break;
		}

		if ( !craft ) {
			send_to_char ("No such craft.\n", ch);
			unload_pc (tch);
			return;
		}

		snprintf (buf, MAX_STRING_LENGTH,  "meets_skill_requirements: %d\nhas_required_crafting_skills: %d\n",
			meets_skill_requirements (tch, craft), has_required_crafting_skills (tch, craft));

		send_to_char (buf, ch);

		unload_pc (tch);

		return;
	}

	if ( !str_cmp (argument, "website") ) {
		update_website_statistics();
		return;
	}

	if ( !str_cmp (argument, "stayputs") ) {
		load_stayput_mobiles();
		send_to_char ("Stayput mobiles reloaded from database.\n", ch);
		return;
	}

	if ( !cmp_strn (argument, "wounds", 6) ) {
		argument = one_argument (argument, buf);
		if ( *argument ) {
			if ( !str_cmp (argument, "delete") ) {
				mysql_safe_query ("DELETE FROM wound_data");
				send_to_char ("Data deleted.\n", ch);
				return;
			}
			num_wounds = strtol(argument, NULL, 10);
			while ( num_wounds > 0 ) {
				temp_arg = figure_location(ch,0);
				wound_to_char (ch, temp_arg, number(1,60), number(0,9), 0, 0, 0);
				num_wounds--; 
			}
		}

		disable_timer_abort = TRUE;

		for ( wound = ch->wounds; wound; wound = next_wound ) {
			next_wound = wound->next;
			temp_arg = expand_wound_loc(wound->location);
			snprintf (buf, MAX_STRING_LENGTH,  "%s %s on the %s: ", wound->severity, wound->name, temp_arg);
			*buf = toupper(*buf);
			time_passed = 0;
			snprintf (severity, MAX_STRING_LENGTH, "%s", wound->severity);
			points = wound->damage;
			while ( wound->damage > 0 && !wound->infection ) {
				natural_healing_check (ch, wound);
				time_passed += (BASE_PC_HEALING - ch->con/6) * 60;
			}
			passed = real_time_passed (time_passed, 0);
			mysql_safe_query ("INSERT INTO wound_data VALUES ('%s', %d, %d, %d, %d)", severity, points, time_passed, wound->infection, ch->con);
			wound_from_char (ch, wound);
		}		
	}

	if ( !str_cmp (argument, "crash") ) {
                tch = NULL;
                if ( *tch->pc->account )
                        tch->pc->account = NULL;
	}

	if ( !str_cmp (argument, "sell") ) {
        	for ( tch = character_list; tch; tch = tch->next ) {
	       		if ( tch->deleted )
				continue;
 			if ( !IS_NPC (tch) || !IS_SET (tch->flags, FLAG_KEEPER) )
                        	continue;
			purse = number(100, 450);
			while ( purse > 0 )
                        	purse -= vnpc_customer (tch, purse);
                }
		send_to_char ("VNPC customers have been triggered.\n", ch);
		return;
	}

	if ( !str_cmp (argument, "saverooms2") ) {
		load_save_room (ch->room);
		send_to_char ("Room loaded.\n", ch);
		return;
	}

	if ( !str_cmp (argument, "tracks") ) {
		for ( room = full_room_list; room; room = room->lnext ) {
			for ( track = room->tracks; track; track = track->next )
				i++;
		}
		snprintf (buf, MAX_STRING_LENGTH,  "There are #2%d#0 tracks currently on the grid.\n", i);
		send_to_char (buf, ch);
	}

	if ( !str_cmp (argument, "locks") ) {
		for ( room = full_room_list; room; room = room->lnext ) {
			for ( i = 0; i < 6; i++ ) {
				if ( room->dir_option[i] && IS_SET (room->dir_option[i]->exit_info, PASSAGE_ISDOOR) && IS_SET (room->dir_option[i]->exit_info, PASSAGE_LOCKED) && room->dir_option[i]->pick_penalty < 30 )
					room->dir_option[i]->pick_penalty = 30;
			}
		}
	}

	if ( !str_cmp (argument, "moon") ) {
		moon_light = 1;
	}

	if ( !cmp_strn (argument, "death_email", 11) ) {
		argument = one_argument (argument, buf);
		if ( !*argument )
			death_email (ch);
		else {
			temp_char = load_pc(argument);
			death_email (temp_char);
		}
	}

	if ( !str_cmp (argument, "saverooms") )
		save_player_rooms();

	if ( !str_cmp (argument, "pc-sold") ) {
		for ( tch = character_list; tch; tch = tch->next ) {
			if ( tch->deleted )
				continue;

			if ( !IS_SET (tch->flags, FLAG_KEEPER) || !tch->shop )
				continue;

			if ( !(room = vtor(tch->shop->store_vnum)) )
				continue;

			if ( !room->contents )
				continue;

			for ( obj = room->contents; obj; obj = obj->next_content ) {
				if ( GET_ITEM_TYPE (obj) == ITEM_MONEY )
					continue;
				if ( !keeper_makes (tch, obj->virtual) && !IS_SET (obj->obj_flags.extra_flags, ITEM_PC_SOLD) )
					SET_BIT (obj->obj_flags.extra_flags, ITEM_PC_SOLD);
			}
		}
	
		send_to_char ("PC-Sold flags have been reset on all objects.\n", ch);
	}

	if ( !str_cmp (argument, "writing") ) {
		for ( obj = object_list; obj; obj = obj->next )
			if ( GET_ITEM_TYPE (obj) == ITEM_PARCHMENT || GET_ITEM_TYPE (obj) == ITEM_BOOK )
				save_writing (obj);
	}

	if ( !str_cmp (argument, "blank_ids") ) {
		for ( obj = object_list; obj; obj = obj->next ) {
			if ( GET_ITEM_TYPE (obj) == ITEM_PARCHMENT )
				obj->o.od.value[0] = 0;
			else if ( GET_ITEM_TYPE (obj) == ITEM_BOOK )
				obj->o.od.value[1] = 0;
		}
	}
	
	return;
}

MESSAGE_DATA *load_mysql_message (char *msg_name, int board_type, int msg_number)
{
	MESSAGE_DATA	*message = NULL;
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;

	result = NULL;

	if ( board_type == 0 ) {/* In-game board system. */
		mysql_safe_query ("SELECT * FROM boards WHERE board_name = '%s' AND post_number = %d", msg_name, msg_number);
		result = mysql_store_result(database);
		if ( !result || !mysql_num_rows (result) ) {
			if ( result )
				mysql_free_result (result);
			return NULL;	
		}
		if ( (row = mysql_fetch_row(result)) ) {
			message = (MESSAGE_DATA *)alloc ((int)sizeof (MESSAGE_DATA), 1);
			message->virtual = strtol(row[1], NULL, 10);
			message->poster = add_hash(row[3]);
			message->date = add_hash(row[4]);
			message->icdate = add_hash(row[5]);
			message->subject = add_hash(row[2]);
			message->message = add_hash(row[6]);
			mysql_free_result (result);
			return message;
		}	
	}
	else if ( board_type == 1 ) {		/* Virtual boards. */
		mysql_safe_query ("SELECT * FROM virtual_boards WHERE board_name = '%s' AND post_number = %d", msg_name, msg_number);
		result = mysql_store_result(database);
		if ( !result || !mysql_num_rows (result) ) {
			if ( result )
				mysql_free_result (result);	
			return NULL;
		}
		if ( (row = mysql_fetch_row(result)) ) {
			message = (MESSAGE_DATA *)alloc ((int)sizeof (MESSAGE_DATA), 1);
			message->virtual = msg_number;
			message->poster = add_hash(row[3]);
			message->date = add_hash(row[4]);
			message->subject = add_hash(row[2]);
			message->message = add_hash(row[5]);
			mysql_free_result (result);
			return message;
		}
	}	
	else if ( board_type == 2 ) {		/* Player notes. */
		mysql_safe_query ("SELECT * FROM player_notes WHERE name = '%s' AND post_number = %d", msg_name, msg_number);
		result = mysql_store_result(database);
		if ( !result || !mysql_num_rows (result) ) {
			if ( result )
				mysql_free_result (result);	
			return NULL;
		}
		if ( (row = mysql_fetch_row(result)) ) {
			message = (MESSAGE_DATA *)alloc ((int)sizeof (MESSAGE_DATA), 1);
			message->virtual = msg_number;
			message->poster = add_hash(row[3]);
			message->date = add_hash(row[4]);
			message->subject = add_hash(row[2]);
			message->message = add_hash(row[5]);
			message->flags = strtol(row[6], NULL, 10);
			mysql_free_result (result);
			return message;
		}
	}	
	else if ( board_type == 3 ) {		/* Player journal entries. */
		mysql_safe_query ("SELECT * FROM player_journals WHERE name = '%s' AND post_number = %d", msg_name, msg_number);
		result = mysql_store_result(database);
		if ( !result || !mysql_num_rows(result) ) {
			if ( result )
				mysql_free_result (result);	
			return NULL;
		}
		if ( (row = mysql_fetch_row(result)) ) {
			message = (MESSAGE_DATA *)alloc ((int)sizeof (MESSAGE_DATA), 1);
			message->virtual = msg_number;
			message->poster = add_hash(row[3]);
			message->date = add_hash(row[4]);
			message->subject = add_hash(row[2]);
			message->message = add_hash(row[5]);
			mysql_free_result (result);
			return message;
		}
	}	

	return NULL;
}

int erase_mysql_board_post (CHAR_DATA *ch, char *name, int board_type, char *argument)
{
	MESSAGE_DATA	*message = NULL;

	if ( strlen (name) > 75 )
		return 0;

	CAP (name);

	if ( !isdigit (*argument) )
		return 0;

	if ( strtol(argument, NULL, 10) < 1 )
		return 0;

	if ( board_type == 0 ) {
		if ( !(message = load_message (name, 6, strtol(argument, NULL, 10))) )
			return 0;
		unload_message (message);
		mysql_safe_query ("DELETE FROM boards WHERE post_number = %d AND board_name = '%s'", strtol(argument, NULL, 10), name);
	}
	else if ( board_type == 1 ) {
		if ( !(message = load_message (name, 5, strtol(argument, NULL, 10))) )
			return 0;
	        if ( (!str_cmp (name, "Bugs") || !str_cmp (name, "Typos") || !str_cmp (name, "Ideas") || !str_cmp (name, "Petitions") || !str_cmp (name, "Submissions") ) && GET_TRUST (ch) ) {
                        send_to_char ("Please enter what you did in response to this report:\n", ch);
                        CREATE (ch->desc->pending_message, MESSAGE_DATA, 1);
                        ch->desc->pending_message->message = NULL;
			ch->desc->pending_message->poster = add_hash(message->poster);
                        ch->desc->str = &ch->desc->pending_message->message;
                        ch->desc->max_str = MAX_STRING_LENGTH;
                        ch->desc->proc = post_track_response;
                        ch->delay_who = add_hash (name);
                        ch->delay_info1 = strtol(argument, NULL, 10);
                        make_quiet (ch);
			unload_message (message);
			return 1;
        	}
	        if ( !str_cmp (name, "Prescience") && GET_TRUST (ch) ) {
			if ( !load_pc (message->poster) ) {
				send_to_char ("I couldn't find the PC that left the prescience request.\n", ch);
				return 0;
			}
                        send_to_char ("Enter the dream you'd like to give in response to this prescience request:\n", ch);
		        ch->delay_ch = load_pc (message->poster);
        		ch->desc->str = &ch->delay_who;
        		ch->desc->max_str = STR_MULTI_LINE;
        		ch->desc->proc = post_dream;
                        make_quiet (ch);
			unload_message (message);
        	}
		mysql_safe_query ("DELETE FROM virtual_boards WHERE post_number = %d AND board_name = '%s'", strtol(argument, NULL, 10), name);
		unload_message (message);
	}
	if ( board_type == 2 ) {
		if ( !(message = load_message (name, 7, strtol(argument, NULL, 10))) )
			return 0;
		unload_message (message);
		mysql_safe_query ("DELETE FROM player_notes WHERE post_number = %d AND name = '%s'", strtol(argument, NULL, 10), name);
	}
	if ( board_type == 3 ) {
		if ( !(message = load_message (name, 8, strtol(argument, NULL, 10))) )
			return 0;
		unload_message (message);
		mysql_safe_query ("DELETE FROM player_journals WHERE post_number = %d AND name = '%s'", strtol(argument, NULL, 10), name);
	}

	return 1;
}

int get_mysql_board_listing (CHAR_DATA *ch, int board_type, char *name)
{
	char		query [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int			i = 0;

	result = NULL;

	if ( board_type == 1 )
		mysql_safe_query ("SELECT * FROM virtual_boards WHERE board_name = '%s' ORDER BY post_number DESC", name);
	else if ( board_type == 2 )
		mysql_safe_query ("SELECT * FROM player_notes WHERE name = '%s' ORDER BY post_number DESC", name);
	else if ( board_type == 3 )
		mysql_safe_query ("SELECT * FROM player_journals WHERE name = '%s' ORDER BY post_number DESC", name);

	result = mysql_store_result(database);
	if ( !result || !mysql_num_rows (result) ) {
		if ( result )
			mysql_free_result (result);
		return 0;
	}

	*b_buf = '\0';
	i = 2500;		/* Max posts displayed in b_buf. */
	while ( (row = mysql_fetch_row(result)) ) {
		if ( strlen(row[2]) > 34 ) {
			snprintf (query, MAX_STRING_LENGTH,"%s", row[2]);
			query[31] = '.';
			query[32] = '.';
			query[33] = '.';
			query[34] = '\0';
		}
		else snprintf (query, MAX_STRING_LENGTH,"%s", row[2]);
		snprintf (b_buf + strlen(b_buf), MAX_STRING_LENGTH, " #6%3d#0 - %16s %-10.10s: %s\n", 
		(int)strtol(row[1], NULL, 10), row[4], row[3], query);
		i--;
		if ( i <= 0 )
			break;
	}

	mysql_free_result (result);

	if ( !*b_buf )
		return 0;

	page_string (ch->desc, b_buf);
	return 1;
}

void post_to_mysql_virtual_board (DESCRIPTOR_DATA *d)
{
	char		*date = NULL;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int		free_slot = 0;

	if ( !*d->pending_message->message ) {
		send_to_char ("No message posted.\n", d->character);
		unload_message (d->pending_message);
		d->pending_message = NULL;
		return;
	}

	date = timestr(date);

	mysql_safe_query ("SELECT post_number FROM virtual_boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1", d->pending_message->poster);
	result = mysql_store_result(database);

	if ( !result ) {
		send_to_char ("There seems to be a problem with the database listing.\n", d->character);
		return;
	}

	free_slot = 1;
	if ( (row = mysql_fetch_row (result)) ) {
		free_slot = strtol(row[0], NULL, 10);
		free_slot++;
	}

	mysql_safe_query ("INSERT INTO virtual_boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', %d)",
	d->pending_message->poster, free_slot, d->pending_message->subject, d->character->tname, date, d->pending_message->message, (int)time(0));

	if ( result )
		mysql_free_result (result);

	unload_message (d->pending_message);
	d->pending_message = NULL;
	mem_free (date);
	
	return;
}

void post_to_mysql_player_board (DESCRIPTOR_DATA *d)
{
	char		*date = NULL;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int		free_slot = 0;

	if ( !*d->pending_message->message ) {
		send_to_char ("No message posted.\n", d->character);
		unload_message (d->pending_message);
		d->pending_message = NULL;
		return;
	}

	date = timestr(date);

	mysql_safe_query ("SELECT * FROM player_notes WHERE name = '%s' ORDER BY post_number DESC LIMIT 1", d->pending_message->poster);
	result = mysql_store_result(database);

	if ( !result ) {
		send_to_char ("There seems to be a problem with the database listing.\n", d->character);
		return;
	}

	free_slot = 1;
	if ( (row = mysql_fetch_row (result)) ) {
		free_slot = strtol(row[1], NULL, 10);
		free_slot++;
	}

	mysql_safe_query ("INSERT INTO player_notes VALUES ('%s', %d, '%s', '%s', '%s', '%s', 0, %d)",
	d->pending_message->poster, free_slot, d->pending_message->subject, d->character->tname, date, d->pending_message->message, (int)time(0));

	if ( result )
		mysql_free_result (result);

	unload_message (d->pending_message);
	d->pending_message = NULL;
	mem_free (date);
	
	return;
}

void post_to_mysql_journal (DESCRIPTOR_DATA *d)
{
	char		*date = NULL;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int		free_slot = 0;

	if ( !*d->pending_message->message ) {
		send_to_char ("No message posted.\n", d->character);
		unload_message (d->pending_message);
		d->pending_message = NULL;
		return;
	}

	date = timestr(date);

	mysql_safe_query ("SELECT * FROM player_journals WHERE name = '%s' ORDER BY post_number DESC LIMIT 1", d->character->tname);
	result = mysql_store_result(database);

	if ( !result ) {
		send_to_char ("There seems to be a problem with the database listing.\n", d->character);
		return;
	}

	free_slot = 1;
	if ( (row = mysql_fetch_row (result)) ) {
		free_slot = strtol(row[1], NULL, 10);
		free_slot++;
	}

	mysql_safe_query ("INSERT INTO player_journals VALUES ('%s', %d, '%s', '%s', '%s', '%s')",
	d->pending_message->poster, free_slot, d->pending_message->subject, d->character->tname, date, d->pending_message->message);

	if ( result )
		mysql_free_result (result);

	unload_message (d->pending_message);
	d->pending_message = NULL;
	mem_free (date);
	
	return;
}

void add_message_to_mysql_player_notes (char *name, char *poster, MESSAGE_DATA *message)
{
	char		vboard [MAX_STRING_LENGTH] = {'\0'};
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int			free_slot = 0;

	mysql_safe_query ("SELECT * FROM player_notes WHERE name = '%s' ORDER BY post_number DESC LIMIT 1", vboard);
	result = mysql_store_result(database);
	if ( !result ) {
		return;
	}

	free_slot = 1;
	if ( (row = mysql_fetch_row (result)) ) {
		free_slot = strtol(row[1], NULL,10);
		free_slot++;
	}

	mysql_safe_query ("INSERT INTO player_notes VALUES ('%s', %d, '%s', '%s', '%s', '%s', %d, %d)",
	vboard, free_slot, message->subject, poster, message->date, message->message, (int) message->flags, (int)time(0));

	if ( result )
		mysql_free_result (result);
		
	return;
}

void add_message_to_mysql_vboard (char *name, char *poster, MESSAGE_DATA *message)
{
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;
	int		free_slot = 0;

	mysql_safe_query ("SELECT * FROM virtual_boards WHERE board_name = '%s' ORDER BY post_number DESC LIMIT 1", name);
	result = mysql_store_result(database);

	if ( !result ) {
		return;
	}

	free_slot = 1;
	if ( (row = mysql_fetch_row (result)) ) {
		free_slot = strtol(row[1], NULL,10);
		free_slot++;
	}

	mysql_safe_query ("INSERT INTO virtual_boards VALUES ('%s', %d, '%s', '%s', '%s', '%s', %d)",
	name, free_slot, message->subject, poster, message->date, message->message, (int)time(0));

	if ( result )
		mysql_free_result (result);
	
	return;
}
