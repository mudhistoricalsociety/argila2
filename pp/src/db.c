/*------------------------------------------------------------------------\
|  db.c : Database Module                             www.yourmud.org     | 
|  Copyright (C) 2006, Project Argila: Auroness                           |
|                                                                         |
|  All rights reserved. See Licensing documents included.                 |
|                                                                         |
|  Based upon Shadows of Isildur RPI Engine                               |
|  Copyright C. W. McHenry [Traithe], 2004                                |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>
#include <mysql/mysql.h>
#include <signal.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

void schar(CHAR_DATA *ch);

struct strel {
    int count;
    char *stp;
    char *next;
};

struct strel *sht;

int				MAX_MEMORY;
int				PERM_MEMORY_SIZE;
int				MAX_OVERHEAD;

int				mob_start_stat = 12;
int				run_mult = 10000;
int				loud_log = 0;

int				mem_allocated = 0;
int				mem_freed	  = 0;

ROOM_DATA		*wld_tab [ZONE_SIZE];
OBJ_DATA		*obj_tab [ZONE_SIZE];
CHAR_DATA		*mob_tab [ZONE_SIZE];

ACCOUNT_DATA		*account_list		= NULL;
ROLE_DATA			*role_list			= NULL;
OBJ_DATA			*object_list		= NULL;
CHAR_DATA			*character_list		= NULL;
CHAR_DATA			*full_mobile_list	= NULL;
ROOM_DATA			*full_room_list		= NULL;
OBJ_DATA			*full_object_list	= NULL;
MOBPROG_DATA		*full_prog_list		= NULL;
AFFECTED_TYPE		*world_affects		= NULL;
BOARD_DATA			*full_board_list	= NULL;
SECOND_AFFECT		*second_affect_list	= NULL;
COMBAT_MSG_DATA		*cm_list			= NULL;
CHAR_DATA			*loaded_list		= NULL;
QE_DATA				*quarter_event_list = NULL;
COMMON_DATA			*common_list		= NULL;
NEWBIE_HINT			*hint_list			= NULL;
RANDOM_CH_DATA		*random_char_list	= NULL;
HELP_DATA			*help_list			= NULL;
HELP_DATA			*bhelp_list			= NULL;
TEXT_DATA			*text_list			= NULL;
TEXT_DATA			*document_list		= NULL;
CHAR_DATA			*free_char_list		= NULL;
OBJ_DATA			*free_obj_list		= NULL;
RECIPE_DATA			*recipe_list		= NULL;
SUBCRAFT_HEAD_DATA	*crafts				= NULL;
NAME_SWITCH_DATA	*clan_name_switch_list = NULL;
CLAN_DATA			*clan_list			= NULL;
SITE_INFO			*banned_site;

int booting 			= 1;

int				hash_len;
int				hash_val;
int				knockout;				/* Cleanup dead pcs */

char			*use_memory_base = NULL;
char			*use_memory_top = NULL;
char			*use_memory_next = NULL;
char			*overhead_base = NULL;
char			*overhead_top = NULL;
char			*overhead_next = NULL;
char			*perm_memory = NULL;
char			*perm_memory_top = NULL;
char			*perm_memory_next = NULL;

struct hash_data		*hash_buckets [NUM_BUCKETS];

char			*null_string;
char			*emergency_data = NULL;

		/* do_hour messes up time, so we keep track of how many times do_hour
           was used, so we can make time adjustments. */

int						times_do_hour_called = 0;
int						next_mob_coldload_id = 0;
int						next_pc_coldload_id = 0;/* get_next_pc_coldload_id () */
int						next_obj_coldload_id = 0;

int						count_max_online = 0;
int						count_guest_online = 0;
char					*max_online_date;
int						MAX_ZONE = 100;
int						second_affect_active = 0;
int						hash_dup_strings = 0;
int						hash_dup_length = 0;
int						x1 = 0;						/* Debugging variable */
int						check_objects;
int						check_characters;
long					mud_time;
char					*mud_time_str = NULL;

int	mp_dirty			= 0;	/* 1 if mob programs need to be written out */
time_t					next_hour_update;		/* Mud hour pulse (15 min) */
time_t					next_minute_update;		/* RL minute (1 min) */

int pulse_violence		= 8;
REGISTRY_DATA    		*registry [MAX_REGISTRY];
struct zone_data 		*zone_table;
int advance_hour_now 	= 0;
struct message_list 	fight_messages[MAX_MESSAGES];
extern char				 	BOOT [24];
char		BOOT [24];

FILE		*fp_log;
FILE		*imm_log;
FILE		*guest_log;
FILE		*sys_log;

struct time_info_data time_info;
struct weather_data weather_info [100];

/* local procedures */
void verify_exits (void);
void boot_zones(void);
void setup_dir(FILE *fl, ROOM_DATA *room, int dir, int type);
ROOM_DATA *allocate_room (int virtual);
char *file_to_string(char *name);
void reset_time(void);
void reload_hints(void);
int change;
int port;
bool Memory_check = FALSE;

/* external refs */
extern struct descriptor_data *descriptor_list;
extern struct msg_data *msg_list;
extern void load_messages(void);
extern void boot_social_messages(void);

ROOM_DATA *vtor (int virtual)
{
	static ROOM_DATA	*room = NULL;
	static int			shortcuts = 0;
	static int			longways  = 0;
	static int			failures  = 0;

		/* Short cut...caller might want last used room */

	if ( room && room->virtual == virtual ) {
		shortcuts++;
		return room;
	}

	if ( virtual < 0 )
		return NULL;

	for ( room = wld_tab [virtual % ZONE_SIZE]; room; room = room->hnext )
		if ( room->virtual == virtual ) {
			longways++;
			return (room);
		}

	failures++;

	return NULL;
}

void add_room_to_hash (ROOM_DATA *add_room)
{
	int					hash;
	ROOM_DATA			*troom;
	static ROOM_DATA	*last_room = NULL;

	add_room->lnext = NULL;

	if ( booting && last_room )
		last_room->lnext = add_room;

	else if ( !full_room_list )
		full_room_list = add_room;

	else {
		troom = full_room_list;

		while ( troom->lnext )
			troom = troom->lnext;

		troom->lnext = add_room;
	}

	last_room = add_room;

	hash = add_room->virtual % ZONE_SIZE;

	add_room->hnext = wld_tab [hash];
	wld_tab [hash] = add_room;
}

CHAR_DATA *vtom (int virtual)
{
	CHAR_DATA	*mob;

	if ( virtual < 0 )
		return NULL;

	for (mob = mob_tab [virtual % ZONE_SIZE]; mob; mob = mob->mob->hnext)
		if ( mob->mob->virtual == virtual )
			return (mob);

	return NULL;
}

void add_mob_to_hash (CHAR_DATA *add_mob)
{
	int					hash;
	CHAR_DATA			*tmob;
	static CHAR_DATA	*last_mob = NULL;

	add_mob->mob->lnext = NULL;

	if ( booting && last_mob )
		last_mob->mob->lnext = add_mob;
		
	else if ( !full_mobile_list )
		full_mobile_list = add_mob;

	else {
		tmob = full_mobile_list;

		while ( tmob->mob->lnext )
			tmob = tmob->mob->lnext;

		tmob->mob->lnext = add_mob;
	}

	last_mob = add_mob;

	hash = add_mob->mob->virtual % ZONE_SIZE;

	add_mob->mob->hnext = mob_tab [hash];
	mob_tab [hash] = add_mob;
}

OBJ_DATA *vtoo (int virtual)
{
	OBJ_DATA	*obj;

	if ( virtual < 0 )
		return NULL;

	for (obj = obj_tab [virtual % ZONE_SIZE]; obj; obj = obj->hnext)
		if ( obj->virtual == virtual )
			return (obj);

	return NULL;
}

void add_obj_to_hash (OBJ_DATA *add_obj)
{
	int					hash;
	OBJ_DATA			*tobj;
	static OBJ_DATA		*last_obj = NULL;

	add_obj->lnext = NULL;

	if ( booting && last_obj )
		last_obj->lnext = add_obj;

	else if ( !full_object_list )
		full_object_list = add_obj;

	else {
		tobj = full_object_list;

		while ( tobj->lnext )
			tobj = tobj->lnext;

		tobj->lnext = add_obj;
	}

	last_obj = add_obj;

	hash = add_obj->virtual % ZONE_SIZE;

	add_obj->hnext = obj_tab [hash];
	obj_tab [hash] = add_obj;
}

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

void boot_db (void)
{
	int		i;
	char		buf [MAX_STRING_LENGTH];

	/* get the name of the person who rebooted */

	mm ("in boot_db");
	
	mud_time = time (NULL);

	mob_start_stat = 12;
	MAX_MEMORY = 12000000;
	PERM_MEMORY_SIZE = 2900000;
	MAX_OVERHEAD = 1000000;

	system_log ("Initializing read-only memory.", FALSE);
	init_memory ();

	mm ("post init_memory");

	system_log("Beginning database initialization.", FALSE);

	system_log("Initialising registry.", FALSE);
	setup_registry ();

	mm ("post setup_registry");

	system_log("Loading dynamic registry.", FALSE);
	load_clan_registry ();

	mm ("post clan_registry");

	system_log("Reloading banned site list.", FALSE);
	reload_sitebans();

	system_log("Loading special chargen roles.", FALSE);
	reload_roles();

	system_log ("Loading newbie hints.", FALSE);
	reload_hints();

	system_log ("Reading external text files.", FALSE);
	(void)add_text (&text_list, WIZLIST_FILE,	"wizlist");
	(void)add_text (&text_list, NEWS_FILE,	"news");
	(void)add_text (&text_list, GREET_FILE,		"greetings");
	(void)add_text (&text_list, MAINTENANCE_FILE,		"greetings.maintenance");
	(void)add_text (&text_list, MENU1_FILE,		"menu1");
    (void)add_text (&text_list, QSTAT_FILE,		"qstat_message");
	(void)add_text (&text_list, ACCT_APP_FILE,	"account_application");
	(void)add_text (&text_list, ACCT_REF_FILE,	"account_referral");
	(void)add_text (&text_list, ACCT_EMAIL_FILE,		"account_email");
	(void)add_text (&text_list, ACCT_POLICIES_FILE,	"account_policies");
	(void)add_text (&text_list, THANK_YOU_FILE,		"thankyou");
	(void)add_text (&text_list, RACE_SELECT,		"race_select");
	(void)add_text (&text_list, ROLE_SELECT,		"role_select");
	(void)add_text (&text_list, SPECIAL_ROLE_SELECT,	"special_role_select");
	(void)add_text (&text_list, SEX_SELECT,			"sex_select");
	(void)add_text (&text_list, AGE_SELECT,			"age_select");
	(void)add_text (&text_list, PLDESC_FILE,		"help_pldesc");
	(void)add_text (&text_list, PSDESC_FILE,		"help_psdesc");
	(void)add_text (&text_list, PDESC_FILE,			"help_pdesc");
	(void)add_text (&text_list, NAME_FILE,			"help_name");
	(void)add_text (&text_list, PKEYWORDS_FILE,		"help_pkeywords");
	(void)add_text (&text_list, HEIGHT_FRAME,		"height_frame");
	(void)add_text (&text_list, LOCATION,			"location");
	(void)add_text (&text_list, COMMENT_HELP,		"comment_help");
	(void)add_text (&text_list, SKILL_SELECT,		"skill_select");
	(void)add_text (&text_list, PROFESSION_SELECT,	"professions");

	mm ("post file_to_string of files");

	system_log("Loading zone table.", FALSE);
	boot_zones();

	system_log ("Initializing dynamic weather zones.", FALSE);
	initialize_weather_zones();

	mm ("post boot_zones");

	system_log("Resetting the game time:", FALSE);
	reset_time();

	mm ("post reset_time");

	system_log("Loading rooms.", FALSE);
	load_rooms();

	mm ("post load_rooms");

	system_log("Generating hash table for mobile files.", FALSE);
	boot_mobiles ();

	mm ("post boot_mobiles");

	system_log("Generating hash table for object files.", FALSE);
	boot_objects();

	mm ("post boot_objects");

	system_log("Loading craft information.", FALSE);
	boot_crafts ();

	mm ("post boot_crafts");

	system_log("Load mobile triggers (mobprogs).", FALSE);
	boot_mobprogs ();

	mm ("post boot_mobprogs");

	system_log("Loading fight messages.", FALSE);
	load_messages();

	mm ("post load_messages");

	system_log("Loading social messages.", FALSE);
	boot_social_messages();

	mm ("post boot_social_messages");

	system_log ("Reading online record.", FALSE);
	load_online_stats();

	booting = 0;

	system_log ("Reloading persistent tracks.", FALSE);
	load_tracks();

	if ( port == PLAYER_PORT ) {
	        system_log ("Loading persistent mobiles...", FALSE);
		load_stayput_mobiles();
		system_log ("Stocking any new deliveries set since last reboot...", FALSE);
		stock_new_deliveries();
	}

	mm ("post load_save_rooms");

	mem_free (overhead_base);

	mm ("freed overhead_base");

/******* Never used Is it really needed *******/
	emergency_data = malloc (20000);

	mm ("Allocated emergency_data");
/***********************************************/

	for (i = 0; i < MAX_ZONE; i++)
	{
		if ( !zone_table [i].cmd )
			continue;

		if ( port != BUILDER_PORT )
			REMOVE_BIT (zone_table [i].flags, Z_FROZEN);

		if ( strncmp (zone_table [i].name, "Empty Zone", 10) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Performing boot-time reset of %s: %d.", zone_table [i].name, i);
			system_log (buf, FALSE);
		}

		reset_zone (i);

		snprintf (buf, MAX_STRING_LENGTH,  "loaded zone %d", i);
		mm (buf);
	}

	print_mem_stats (NULL);

	system_log ("Boot db -- DONE.", FALSE);
}

void reload_hints(void)
{
	NEWBIE_HINT	*hint, *thint;
	FILE		*fp;
	char		*string;
	char		buf [MAX_STRING_LENGTH];

	hint_list = NULL;

	if ( !(fp = fopen("/text/hints", "r")) )
		return;

	(void)fgets (buf, 256, fp);

	while ( 1 ) {
		string = fread_string (fp);
		if ( !string || !*string )
			break;
		CREATE (hint, NEWBIE_HINT, 1);
		hint->hint = string;
		if ( !hint_list )
			hint_list = hint;
		else {
			thint = hint_list;
			while ( thint->next )
				thint = thint->next;
			thint->next = hint;
		}
	}

	fclose (fp);
}

void create_guest_avatar (DESCRIPTOR_DATA *d)
{

	CHAR_DATA		*ch = NULL, *tch = NULL;
    char            buf [MAX_STRING_LENGTH];
    char            buf2 [MAX_STRING_LENGTH];
    char            account [MAX_STRING_LENGTH];
    char            tname_buf [MAX_STRING_LENGTH];
    char			*ipbuf = NULL;
        
	int				i = 1;

	*buf = '\0';
	*buf2 = '\0';
	*account = '\0';
	*tname_buf = '\0';


	if ( !d )
		return;

	snprintf (account, MAX_STRING_LENGTH, "Guest");

	/* 
	* If we aren't recreating for an already-defined guest handle, find a new
	* one either based on matching the IP to an account or counting the number
	* of anonymous guests already in-game.
	*
	* If it's an unidentified guest login, attach the generic "Guest"
	* account; otherwise, load up the player's account found for the IP.
	*/


	if ( !reference_ip (NULL, d->host) | (d->parm == 1)){
		for ( tch = character_list; tch; tch = tch->next ) {
			if ( tch->deleted )
				continue;
			if ( !tch->tname )
				continue;
		    if ( strstr (tch->tname, "Special Guest") )
		       	i++;
		}
		
		snprintf (account, 6, "Guest");
       	snprintf (tname_buf, MAX_STRING_LENGTH, "Special Guest %d", i);
		}
		else {
			ipbuf = reference_ip (NULL, d->host);
			snprintf (account, MAX_STRING_LENGTH, "%s", ipbuf);
			snprintf (tname_buf, MAX_STRING_LENGTH, "%s-Guest", account);
			i = 0;

		       	for ( tch = character_list; tch; tch = tch->next ) {
				if ( tch->deleted )
					continue;
				if ( !tch->tname )
					continue;
		                if ( strstr (tch->tname, tname_buf) )
		                        i++;
		        }
		
			if ( i >= 1 )
				snprintf (tname_buf + strlen(tname_buf), MAX_STRING_LENGTH, "%d", i);
		}


	/** create the guest character description **/
	d->account = load_account (account);
    d->character = new_char(1);
	clear_char (d->character);
	d->original = NULL;

 	ch = d->character; 

    d->character->pc->create_state = STATE_APPROVED;
    d->prompt_mode = 1;
    d->character->desc = d;
    d->character->pc->owner = d;
	d->character->pc->load_count = 1;

	ch->race = RACE_HUMAN;

	d->character->race = ch->race;

	/* Bestow the random traits to the new guest avatar */

        randomize_mobile(ch);
        
	if ( ch->pc->account && strlen(ch->pc->account) > 1 )
		mem_free (ch->pc->account);

        ch->pc->account = str_dup (account);
        ch->fight_mode = 2;
	ch->clans = add_hash("");

	/* Address naming issues with our user's new account handle */

	ch->tname = add_hash(tname_buf);

	snprintf (buf2, MAX_STRING_LENGTH,  "%s %s", ch->name, ch->tname);
	if ( ch->name )
		mem_free (ch->name);
	ch->name = add_hash(buf2);
        ch->hit = 100;
        ch->max_hit = 100;
        ch->move = 100;
        ch->max_move = 100;
        
    
        SET_BIT (d->character->flags, FLAG_GUEST);

	if ( d->character->race >= 0 && d->character->race <= 29 ) {
	        for ( i = 0; i <= MAX_SKILLS; i++ ) {
       	        	d->character->skills[i] = 0;
       	        	d->character->pc->skills[i] = 0;
        	}

	        d->character->speaks =
	        			db_race_table[d->character->race].race_speaks;
	        			
       	 	d->character->skills [d->character->speaks] =
       	 				calc_lookup(d->character, REG_CAP,
       	 				db_race_table[d->character->race].race_speaks);
       	 				
       	 	d->character->pc->skills [d->character->speaks] =
       	 				calc_lookup(d->character, REG_CAP,
       	 				db_race_table[d->character->race].race_speaks);
	}
	

     	guest_conns++;
   
	if ( ch->description )
		mem_free (ch->description);
        ch->description = str_dup("Another Guest is here, passing through. Be sure to welcome them!\n");
        
	SET_BIT (ch->plr_flags, NEWBIE_HINTS);

	ch->hunger = -1;
	ch->thirst = -1;
}

void stock_new_deliveries(void)
{
	CHAR_DATA	*tch = NULL;
	OBJ_DATA	*obj = NULL;
	ROOM_DATA	*room = NULL;
	int			i = 0;

	if ( port != PLAYER_PORT )
		return;

        for ( tch = character_list; tch; tch = tch->next ) {
		if ( tch->deleted )
			continue;
                if ( !IS_NPC (tch) || !IS_SET (tch->flags, FLAG_KEEPER) || !tch->shop )
                        continue;
                room = vtor (tch->shop->store_vnum);
                if ( !room )
                        continue;
		if ( !room->psave_loaded )
			load_save_room (room);
                for ( i = 0; i < MAX_TRADES_IN; i++ ) {
                        if ( !get_obj_in_list_num (tch->shop->delivery[i], room->contents) ) {
                                obj = load_object (tch->shop->delivery[i]);
                                obj_to_room (obj, room->virtual);
                        }
                }
        }

	system_log ("New shopkeeper deliveries stocked.", FALSE);
	return;
}

void copyover_recovery (void)
{
	ROOM_DATA		*room = NULL;
	MYSQL_RES		*result = NULL;
	MYSQL_ROW		row;
	DESCRIPTOR_DATA		*d = NULL;
	DESCRIPTOR_DATA		*td = NULL;
	FILE			*fp = NULL;
	OBJ_DATA		*obj = NULL;
	AFFECTED_TYPE	*af = NULL;
	char			name [MAX_STRING_LENGTH];
	char			table_name [MAX_STRING_LENGTH];
	char			host [MAX_STRING_LENGTH];
	char			account [MAX_STRING_LENGTH];
	char			buf [MAX_STRING_LENGTH];
	int				desc = 0;
	int				online = 0;
	int				guest = 0;
	int				dcheck = 0;
	int				chkdirect = 0;
	int				i = 0;
	int				conn = 0;
	int				pos = 0;
	int				count = 0;
	char			*temp_arg = NULL;
	

	system_log("Soft reboot recovery sequence initiated.", FALSE);

	if ( !(fp = fopen (".copyover_data", "r")) ) {
		system_log("Copyover file not found! Aborting copyover and shutting down.", TRUE);
		abort();
	}
	
	descriptor_list = NULL;

	for ( ; ; )
	{
		conn = 0;

		fscanf (fp, "%d %s %s %s %d %d", &desc, host, account, name, &pos, &count);
		
		if ( desc == -1 ) {
			break;
		}

		CREATE (d, DESCRIPTOR_DATA, 1);
		memset ((char *)d, 0, sizeof (struct descriptor_data));

		init_descriptor (d, desc);

/*		descriptor_list = d; */

		if ( !descriptor_list )
			descriptor_list = d;
		else for ( td = descriptor_list; td; td = td->next ) {
			if ( !td->next ) {
				d->next = NULL;
				td->next = d;
				break;
			}
		}

		if ( d->descriptor > maxdesc )
			maxdesc = d->descriptor;
/*
		d->character = new_char(1);
		clear_char (d->character);
*/
		d->host = add_hash(host);

		if ( !strstr (name, "Guest") )
			d->character = load_pc (name);
		else
			nanny_create_guest (d, "recreate");

		if ( !d->character )
			continue;

		d->character->desc = d;
		d->prompt_mode = 1;
		d->connected = -15;
	
		d->character->pc->owner = d;
		d->character->pc->last_connect = time(0);

		if ( !IS_SET (d->character->flags, FLAG_GUEST) ) {
			d->account = load_account (account);
		}
		
		if ( !IS_SET (d->character->flags, FLAG_GUEST) && d->account && d->account->color )
			d->character->color++;

		if ( IS_SET (d->character->flags, FLAG_GUEST) )
			d->character->color++;
		
		REMOVE_BIT (d->character->flags, FLAG_ENTERING);
		REMOVE_BIT (d->character->flags, FLAG_LEAVING);
		
		pc_to_game (d->character);

		if ( !IS_SET (d->character->flags, FLAG_GUEST) ) {
			load_char_objs (d->character, GET_NAME(d->character));
		
			if ( !d->character->in_room || d->character->in_room == NOWHERE )
				char_to_room (d->character, OOC_LOUNGE);
			else
				char_to_room (d->character, d->character->in_room);
			
			snprintf (buf, MAX_STRING_LENGTH,  "save/player/%c/%s.a", LOWER (*GET_NAME(d->character)), CAP (GET_NAME(d->character)));
			
			
			if ( !IS_SET (d->character->flags, FLAG_GUEST) )
				(void)load_saved_mobiles (d->character, buf);
		}
		
		online = 0;
		guest = 0;
		
		for ( td = descriptor_list; td; td = td->next ) {
			if ( !td->character )
				continue;
			if ( td->character->pc->level )
				continue;
			if ( td->character->pc->create_state != 2 )
				continue;
			if ( td->connected )
				continue;
			if ( IS_SET (td->character->flags, FLAG_GUEST) ) {
				guest++;
			}
			if ( IS_MORTAL (td->character) && !IS_SET(td->character->flags, FLAG_GUEST) ) {
				online++;
			}
		}

		if ( online >= count_max_online ) {
			count_max_online = online;
			max_online_date = timestr(max_online_date);
		}

		if ( guest >= count_guest_online ) {
			count_guest_online = guest;
		}
			
		d->character->pc->last_logon = time(0);
		
		temp_arg = fread_string(fp);
		snprintf (table_name, MAX_STRING_LENGTH, "%s", temp_arg);

		if ( str_cmp (table_name, "none") ) {
			i = 0;
			for ( obj = d->character->room->contents; obj; obj = obj->next_content ) {
				if ( !str_cmp (table_name, obj->name) ) {
					i++;
					if ( i == count ) {
						magic_add_affect (d->character, MAGIC_SIT_TABLE, -1, 0, 0, 0, 0);
						af = get_affect (d->character, MAGIC_SIT_TABLE);
						af->a.table.obj = obj;
						break;
					}
				}
			}
		}

		GET_POS (d->character) = pos;

		d->connected = CON_PLYNG;

	}
	
	fclose(fp);

	if ( port == PLAYER_PORT ) {

		mysql_safe_query ("SELECT * FROM copyover_doors");
		result = mysql_store_result (database);

		if ( result && mysql_num_rows(result) ) {
			while ( (row = mysql_fetch_row(result)) ) {
				if ( !(room = vtor(strtol(row[0], NULL, 10))) )
					continue;
				if ( !room->dir_option [strtol(row[1], NULL, 10)] )
					continue;
				
				dcheck = strtol(row[2], NULL, 10);
				chkdirect = strtol(row[1], NULL, 10);
				
				if ( dcheck == 2 ) { /* closed and locked */
					SET_BIT (room->dir_option[chkdirect]->exit_info, PASSAGE_CLOSED);
					SET_BIT (room->dir_option[chkdirect]->exit_info, PASSAGE_LOCKED);

				}
				
				else if ( dcheck == 1 ) { /* closed and unlocked */
					SET_BIT (room->dir_option[chkdirect]->exit_info, PASSAGE_CLOSED);
					REMOVE_BIT (room->dir_option[chkdirect]->exit_info, PASSAGE_LOCKED);

				}
				
				else if ( dcheck == 0 ) { /* open and unlocked */
					REMOVE_BIT (room->dir_option[chkdirect]->exit_info, PASSAGE_CLOSED);
					REMOVE_BIT (room->dir_option[chkdirect]->exit_info, PASSAGE_LOCKED);

				}
			}
			mysql_free_result (result);
		}

		mysql_safe_query ("DELETE FROM copyover_doors");

	}
}

void reset_time(void)
{
	char buf[MAX_STRING_LENGTH];
/*	long beginning_of_time = 14336715; */

/* Subtract this by 10800 seconds to advance gametime by 12 hours */
      long beginning_of_time = 830615184 - 32743;

	int 	qz[] = {  6,  6,  6,  6,  5,  5,  4,  4,  5,  6,  6,  7 };
	int 	sun_up;

	int	i = 0;

	struct time_info_data mud_time_passed(time_t t2, time_t t1);

	next_hour_update = time (0) + ((time (0) - beginning_of_time) % 900);
	next_minute_update = time (0);

	time_info = mud_time_passed(time(0), beginning_of_time);

	sun_up = qz [(int)time_info.month]; /* sunrise is easy */
	
	snprintf (buf, MAX_STRING_LENGTH, "   Current Gametime: %dH %dD %dM %dY.",
	    time_info.hour, time_info.day,
	    time_info.month, time_info.year);
	system_log(buf, FALSE);

	if ( time_info.month == 0 || time_info.month == 1 || time_info.month == 11 )
		time_info.season = WINTER;
	else if (time_info.month < 11 && time_info.month > 7)
	        time_info.season = AUTUMN;
	else if (time_info.month < 8 && time_info.month > 4 )
        	time_info.season = SUMMER;
    else time_info.season = SPRING;

	for ( i = 0; i <= 99; i++ ) {
		weather_info[i].trend = number(0,15);
		weather_info[i].clouds = number(0,3);
    	
    	if (time_info.season == SUMMER)
        	weather_info[i].clouds = number(0,1);
		
		weather_info[i].fog = 0;
    	if ( weather_info[i].clouds > 0 )
			weather_info[i].state = number(0,1);
		weather_info[i].temperature = seasonal_temp[zone_table[i].weather_type][time_info.month];
		weather_info[i].wind_speed = number(0,2);
		
		if (time_info.hour >= sun_up && time_info.hour < sunset[time_info.month]) {
	    	sun_light = 1;
       		weather_info[i].temperature += 15;
    	}
    	else
        	weather_info[i].temperature -= 15;
	}

	time_info.holiday = 0;
}

void create_room_zero (void)
{
	ROOM_DATA *room;

	room = allocate_room (0);
	room->zone = 0;
	room->name = str_dup ("Heaven");
	room->description = str_dup ("You are in heaven.\n");

	if ( str_cmp (zone_table [0].name, "Empty Zone") )
		return;

	zone_table [0].name = str_dup ("Heaven");
	zone_table [0].top = 0;
	zone_table [0].lifespan = 0;
	zone_table [0].reset_mode = 0;
	zone_table [0].flags = 0;
	zone_table [0].jailer = 0;
	zone_table [0].jail_room_num = 0;
	zone_table [0].weather_type = 0;

	SET_BIT (zone_table [0].flags, Z_FROZEN);
}

void load_rooms (void)
{
	FILE *fl;
	int zon, flag=0, tmp, sdir;
	int virtual_nr;
	char *temp, chk[50], errbuf[80], wfile[80];
	struct extra_descr_data *new_descr;
    struct written_descr_data *w_desc;
	struct room_prog *r_prog;
	struct secret *r_secret;
	struct stat fstatus;
	ROOM_DATA			*room = NULL;
	int					i;

        for (i = 0; i < ZONE_SIZE; i++) {
                wld_tab [i] = NULL;   
                obj_tab [i] = NULL;
                mob_tab [i] = NULL; 
        }

	for (zon = 0; zon < MAX_ZONE; zon++) {

		snprintf (wfile, MAX_STRING_LENGTH, "%s/rooms.%d", REGIONS, zon);

		if ( stat (wfile, &fstatus) ) {
			snprintf (errbuf, MAX_STRING_LENGTH, "Zone %d rooms did not load. Aborting.", zon);
			system_log(errbuf, TRUE);
			abort();
		}

		if ( fstatus.st_size == 0 ) {
			snprintf (s_buf, MAX_STRING_LENGTH, "ERROR: Corrupt %d.wld :: aborting.",zon);
			system_log(s_buf, TRUE);
			abort();
		}

		if ( (fl = fopen (wfile, "r")) == NULL ) {
			snprintf (errbuf, MAX_STRING_LENGTH, "Could not load rooms.%d. Aborting.", zon);
			system_log(errbuf, TRUE);
			abort();
		}

		do {
			fscanf(fl, " #%d\n", &virtual_nr);
			temp = fread_string(fl);
	    	if ( !temp ) 
				continue;		/* KILLER CDR */

			if ((flag=(*temp != '$'))) {
				room = allocate_room (virtual_nr);
				room->zone = zon;
				room->name = temp;
				room->description = fread_string(fl);
				fscanf (fl, "%d", &tmp);

				fscanf (fl, " %d ", &tmp);
				room->room_flags = tmp;

					/* The STORAGE bit is set when loading in shop keepers */

				REMOVE_BIT (room->room_flags, STORAGE);
				REMOVE_BIT (room->room_flags, PC_ENTERED);
				SET_BIT (room->room_flags, SAVE);

				fscanf (fl, " %d ", &tmp);
				room->sector_type = tmp;

				fscanf (fl, "%d\n", &tmp);
				room->deity = tmp;

				room->contents = 0;
				room->people = 0;
				room->light = 0; 

				for (tmp = 0; tmp <= 5; tmp++)
					room->dir_option [tmp] = 0;

				room->ex_description = 0;
				room->wdesc = 0;
				room->prg = 0;
				for (tmp = 0; tmp <= 5; tmp++)
					room->secrets [tmp] = 0;

				for (;;) {
					fscanf(fl, " %s \n", chk);

					if (*chk == 'D')  /* direction field */
						setup_dir(fl, room, strtol(chk + 1, NULL, 10), 0);
                    
					else if (*chk == 'H')  /* Secret (hidden) */
						setup_dir(fl, room, strtol(chk + 1, NULL, 10), 1);
				
                	else if (*chk == 'T')  /* Trapped door */
						setup_dir(fl, room, strtol(chk + 1, NULL, 10), 2);
                
                	else if (*chk == 'B')  /* Trapped hidden door */
						setup_dir(fl, room, strtol(chk + 1, NULL, 10), 3);
                
					else if(*chk == 'Q') { /* Secret search desc */
						r_secret = get_perm (sizeof (struct secret));
						sdir = strtol(chk+1, NULL, 10);	
						fscanf(fl,"%d\n",&tmp);
						r_secret->diff=tmp;
						r_secret->stext=fread_string(fl);
						room->secrets[sdir]=r_secret;
					}

					else if (*chk == 'E')  /* extra description field */
					{
						struct extra_descr_data	*tmp_extra;

						new_descr = get_perm (sizeof (struct extra_descr_data));
						new_descr->keyword = fread_string(fl);
						new_descr->description = fread_string(fl);
						new_descr->next = NULL;

						if ( !room->ex_description )
							room->ex_description = new_descr;
						else {
							tmp_extra = room->ex_description;

							while ( tmp_extra->next )
								tmp_extra = tmp_extra->next;

							tmp_extra->next = new_descr;
						}
					}

					else if (*chk == 'W') {
						w_desc = get_perm (sizeof (struct written_descr_data));
   	                	fscanf(fl,"%d\n",&tmp);
   	                	w_desc->language = tmp;
   	                	w_desc->description = fread_string(fl);
   	                	room->wdesc = w_desc;

	                } 
	                
	                else if (*chk == 'R'){
						larg_setup_room_triggers(fl, room);
					}
					
	                else if(*chk == 'P') {
						struct room_prog *tmp_prg;
						r_prog = get_perm (sizeof (struct room_prog));
						r_prog->command=fread_string(fl);
						r_prog->keys=fread_string(fl);
						r_prog->prog=fread_string(fl);
						r_prog->next = NULL;

						/* Make sure that the room program is stored at
						   end of the list.  This way when the room is
						   saved, the rprogs get saved in the same order
							- Rassilon
						*/

						if ( !room->prg )
							room->prg = r_prog;
						else {
							tmp_prg = room->prg;

						while ( tmp_prg->next )
							tmp_prg = tmp_prg->next;

						tmp_prg->next = r_prog;
						}
					}

					else if ( *chk == 'A' ) { /* Additional descriptions */

						CREATE (room->extra, ROOM_EXTRA_DATA, 1);

						for ( i = 0; i < WR_DESCRIPTIONS; i++ ) {
							room->extra->weather_desc [i] = fread_string (fl);
							if ( !strlen (room->extra->weather_desc [i]) )
								room->extra->weather_desc [i] = NULL;
						}

						for ( i = 0; i < 6; i++ ) {
							room->extra->alas [i] = fread_string (fl);
							if ( !strlen (room->extra->alas [i]) )
								room->extra->alas [i] = NULL;
						}
					}

					else if (*chk == 'S')	/* end of current room */
						break;
				}
			}
		} while (flag);
		fclose (fl);
	}

	for ( i = 0; i < MAX_ZONE; i++ )
		if ( zone_table [i].jail_room_num )
			zone_table [i].jail_room = vtor (zone_table [i].jail_room_num);

	if ( !vtor (0) )
		create_room_zero ();
}

#define MAX_PREALLOC_ROOMS		14000

ROOM_DATA *get_room (void)
{
	ROOM_DATA			*room;
	static ROOM_DATA	*prealloc_rooms = NULL;
	static int			prealloc_rooms_count = 0;

	if ( !prealloc_rooms )
		CREATE (prealloc_rooms, ROOM_DATA, MAX_PREALLOC_ROOMS);

	if ( prealloc_rooms_count >= MAX_PREALLOC_ROOMS )
		CREATE (room, ROOM_DATA, 1);
	else {
		room = prealloc_rooms + prealloc_rooms_count;
		prealloc_rooms_count++;
	}

	return room;
}

ROOM_DATA *allocate_room (int virtual)
{
	ROOM_DATA		*new_room;
	char			buf [MAX_STRING_LENGTH];

	new_room = get_room ();

	new_room->virtual = virtual;
	new_room->psave_loaded = 0;

#define CHECK_DOUBLE_DEFS_ROOMS 0
#ifdef CHECK_DOUBLE_DEFS_ROOMS

	if ( vtor (virtual) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "Room %d multiply defined!!", virtual);
		system_log(buf, TRUE);
	} else
#endif
		add_room_to_hash (new_room);

	return new_room;
}

void setup_dir (FILE *fl, ROOM_DATA *room, int dir, int type)
{
	int tmp2;

	room->dir_option [dir] = get_perm (sizeof (struct room_direction_data));

	room->dir_option [dir]->general_description = fread_string(fl);
	room->dir_option [dir]->keyword = fread_string(fl);

	fscanf(fl, " %d ", &tmp2);
	if (tmp2 == 1)
		room->dir_option[dir]->exit_info = PASSAGE_ISDOOR;
	else if (tmp2 == 2)
		room->dir_option[dir]->exit_info = PASSAGE_ISDOOR | PASSAGE_PICKPROOF;
	else
		room->dir_option[dir]->exit_info = 0;

	fscanf (fl, " %d ", &room->dir_option [dir]->key);

	fscanf (fl, " %d ", &room->dir_option [dir]->pick_penalty);

	fscanf(fl, " %d", &room->dir_option [dir]->to_room);

    switch (type) {
        case 1:
            SET_BIT(room->dir_option[dir]->exit_info, PASSAGE_SECRET);
            break;
        case 2:
            SET_BIT(room->dir_option[dir]->exit_info, PASSAGE_TRAP);
            break;
        case 3:
            SET_BIT(room->dir_option[dir]->exit_info, PASSAGE_SECRET);
            SET_BIT(room->dir_option[dir]->exit_info, PASSAGE_TRAP);
            break;
    }
}

/* load the zone table and command tables */

void boot_zones(void)
{
	FILE			*fl;
	int			zon;
	int			cmd_no;
	int			tmp;
	int			jail_room_num;
	char			c;
	char			*p;
	char			buf [MAX_STRING_LENGTH];
	char			zfile [MAX_STRING_LENGTH];
	struct stat		fstatus;
	RESET_AFFECT	*ra;

	CREATE (zone_table, struct zone_data, MAX_ZONE);

	for (zon = 0; zon < MAX_ZONE; zon++) {

		snprintf (zfile, MAX_STRING_LENGTH, "%s/resets.%d", REGIONS, zon);

		if ( stat (zfile, &fstatus) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Zone %d resets did not load.", zon);
			system_log(buf, TRUE);
			continue;
		}

		if((fl=fopen(zfile,"r"))==NULL) {
			system_log(zfile, TRUE);
			perror("boot_zone");
			break;
		}

		fscanf (fl, " #%*d\n");
		(void)fread_string (fl);

		(void)fgets (buf, 80, fl);		/* Zone number header stuff */
		(void)fgets (buf, 80, fl);

		for ( cmd_no = 1;; ) {

			fscanf (fl, " ");
			fscanf (fl, "%c", &c);

			if ( c == 'S' )
				break;

			if ( c != '*' )
				cmd_no++;

			(void)fgets (buf, 80, fl);
		}

		CREATE (zone_table [zon].cmd, struct reset_com, cmd_no);

		zone_table[zon].weather_type = 0;

		rewind (fl);

		cmd_no = 0;

		fscanf (fl, " #%*d\nLead: ");
		zone_table [zon].lead = fread_string (fl);
		zone_table [zon].name = fread_string (fl);

		jail_room_num = 0;

		(void)fgets (buf, 80, fl);
		(void)fgets (buf, 80, fl);

				/* Note:  At this point, no rooms have been read in yet */

		sscanf (buf, " %d %d %d %ld %d %d",
						 &zone_table [zon].top,
						 &zone_table [zon].lifespan,
						 &zone_table [zon].reset_mode,
						 &zone_table [zon].flags,
						 &zone_table [zon].jailer,
						 &zone_table [zon].jail_room_num);

		SET_BIT (zone_table [zon].flags, Z_FROZEN);

			/* read the command table */

		cmd_no = 0;

		for ( ;; ) {

			fscanf (fl, " "); /* skip blanks */
			fscanf (fl, "%c", &zone_table [zon].cmd [cmd_no].command);

			if ( zone_table [zon].cmd [cmd_no].command == 'C' ) {

				(void)fgets (buf, 80, fl);

				while ( *buf && isspace (buf [strlen (buf) - 1]) )
					buf [strlen (buf) - 1] = '\0';

				zone_table [zon].cmd [cmd_no].arg1 = (int) str_dup (buf);

				cmd_no++;

				continue;
			}

			if ( zone_table [zon].cmd [cmd_no].command == 'R' ) {

				fscanf (fl, "%d %d %d %d %d %d %d %d %d %d",
							&zone_table [zon].cmd [cmd_no].arg1,
							&tmp,
							&tmp,
							&tmp,
							&tmp,
							&tmp,
							&tmp,
							&tmp,
							&tmp,
							&tmp);

				(void)fgets (buf, 80, fl);

				cmd_no++;

				continue;
			}

			if ( zone_table [zon].cmd [cmd_no].command == 'm' ) {

				fscanf (fl, "%d", &zone_table [zon].cmd [cmd_no].arg1);

				if ( zone_table [zon].cmd [cmd_no].arg1 == RESET_REPLY ) {
					(void)fgets (buf, MAX_STRING_LENGTH - 100, fl);
					buf [strlen (buf) - 1] = '\0';		/* Remove new line */

					p = buf;
					while ( isspace (*p) )
						p++;

					zone_table [zon].cmd [cmd_no].arg2 = (int) str_dup (p);
				}

				else {
					system_log("UNKNOWN m type RESET, ignored.", TRUE);
					(void)fgets (buf, 80, fl);
				}

				cmd_no++;

				continue;
			}

			if ( zone_table [zon].cmd [cmd_no].command == 'A' ||
				 zone_table [zon].cmd [cmd_no].command == 'r' ) {

				ra = (RESET_AFFECT *)alloc (sizeof (RESET_AFFECT), 34);

				fscanf (fl, "%d %d %d %d %d %d %d",
							&ra->type,
							&ra->duration,
							&ra->modifier,
							&ra->location,
							&ra->bitvector,
							&ra->sn,
							&ra->t);

					/* putting ra into an in will most certainly create
                       a migration problem.  A migrator should create a
                       new element in zone_table for ra, and not use arg1 */

				zone_table [zon].cmd [cmd_no].arg1 = (int) ra;

				(void)fgets (buf, 80, fl);

				cmd_no++;

				continue;
			}

			if ( zone_table [zon].cmd [cmd_no].command == 'S' )
				break;

			if ( zone_table [zon].cmd [cmd_no].command == '*' ) {
				(void)fgets (buf, 80, fl); /* skip command */
				continue;
			}

			fscanf(fl, " %d %d %d", 
				    &tmp,
				    &zone_table [zon].cmd [cmd_no].arg1,
				    &zone_table [zon].cmd [cmd_no].arg2);

			zone_table [zon].cmd [cmd_no].if_flag = tmp;

			if ( zone_table [zon].cmd [cmd_no].command == 'M' ||
			     zone_table [zon].cmd [cmd_no].command == 'O' ||
			     zone_table [zon].cmd [cmd_no].command == 'E' ||
			     zone_table [zon].cmd [cmd_no].command == 'P' ||
			     zone_table [zon].cmd [cmd_no].command == 'a' ||
				 zone_table [zon].cmd [cmd_no].command == 'D')
				fscanf (fl, " %d", &zone_table [zon].cmd [cmd_no].arg3);

			if ( zone_table [zon].cmd [cmd_no].command == 'M' ) {
				zone_table [zon].cmd [cmd_no].enabled = 1;
				fscanf (fl, " %d", &zone_table [zon].cmd [cmd_no].arg4);
			}

			(void)fgets (buf, 80, fl);	/* read comment */

			cmd_no++;
		}
		fclose (fl);
	}
}

/*************************************************************************
*  stuff related to the save/load player system								  *
*********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */

int load_char_objs(CHAR_DATA *ch, char *name)
{
	FILE *pf;
	char fbuf[265];

	if ( !name ) {
		system_log ("BUG: name NULL in load_char_objs: db.c", TRUE);
		return 0;
	}
	else if ( !*name ) {
		system_log ("BUG: name empty in load_char_objs: db.c\n", TRUE);
		return 0;
	}	

	snprintf(fbuf, MAX_STRING_LENGTH, "save/objs/%c/%s",tolower(*name),name);
	if(!(pf=fopen(fbuf,"r"))) {
		equip_newbie(ch);
		return 0;
	}
	
        read_obj_suppliment (ch, pf);

	fclose(pf);

	return 1;
}

void autosave (void)
{
	int			save_count = 0;
	CHAR_DATA	*t;

	for (t = character_list; t; t = t->next) {

		if ( t->deleted || IS_NPC (t) )
			continue;

		if ( t->desc && t->desc->connected == CON_PLYNG ) {
			save_char (t, TRUE);
			save_count++;
		}
	}
}

void autosave_stayputs (void)
{
	FILE		*fp;
	CHAR_DATA	*ch;

	if ( !(fp = fopen (STAYPUT_FILE ".new", "w")) ) {
		system_log("UNABLE TO OPEN stayput.new FILE!!!", TRUE);
		return;
	}

	for ( ch = character_list; ch; ch = ch->next ) {

		if ( ch->deleted )
			continue;

		if ( IS_SET (ch->act, ACT_STAYPUT) )
			save_mobile (ch, fp, "STAYPUT", FALSE);
	}

	fclose (fp);

	system ("mv " STAYPUT_FILE ".new " STAYPUT_FILE);
}

/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */

char *fread_string (FILE *fp)
{
	char	c;
	char	string_space [MAX_STRING_LENGTH];
	char	*plast;

	plast = string_space;

	while ( isspace ((c = getc (fp))) ) {
		*plast++ = c;
		if ( c != '\t' && c != ' ' )
			plast = string_space;
	}

	if ( ( *plast++ = c ) == '~' )
		return null_string;

    for ( ;; ) {
		switch ( *plast = getc( fp ) ) {
		default:
		    plast++;
		    break;

		case EOF:
			*plast++ = '\0';
			system_log ("Fread_string() error.", TRUE);
		    exit(EXIT_FAILURE);
		    break;

		case '~':
			*plast = '\0';
			return add_hash (string_space);
		}
    }
}

char *read_string (char *string)
{
	char	buf [MAX_STRING_LENGTH];

	*buf = '\0';

/* move the pointer to the end of the string */
	while ( isspace (*string) ) {
		string++;
	}

/* is the last character a tilde? */
	if ( *string == '~' )
		return null_string;

/* read characters and add them to buf, until the tilde is found */
	for ( ;; ) {
		switch ( *string ) {
		default:
			string++;
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%c", *string);
			break;
		case '~':
			return add_hash (buf);
		}
	}
}

CHAR_DATA *new_char (int pc_type)
{
	CHAR_DATA	*ch;

		/* NOTE:  get_perm gets memory from perm_memory during bootup,
				  thereafter gets it from malloc.
		*/

	if ( booting )
		ch = get_perm (sizeof (CHAR_DATA));
	else
		ch = (CHAR_DATA *)alloc ((int)sizeof (CHAR_DATA), 19);


	if ( ch->pc )
		mem_free (ch->pc);

	if ( ch->mob )
		mem_free (ch->mob);


	if ( pc_type ){
		mem_free (ch->pc);
		ch->pc = NULL;
	}
	else{
		mem_free (ch->mob);
		ch->mob = NULL;
	}
		
	ch->hour_affects = NULL;

	ch->pc = NULL;
	ch->mob = NULL;


	if ( pc_type ) {
		ch->pc = get_perm (sizeof (PC_DATA));
		
	}
	else{
		ch->mob = get_perm (sizeof (MOB_DATA));
		
	}
	
	return ch;
}

OBJ_DATA *new_object (void)
{
	OBJ_DATA	*obj = NULL;

	if ( booting )
		obj = get_perm ((int)sizeof (OBJ_DATA));
	else
		obj = (OBJ_DATA *)alloc ((int)sizeof (OBJ_DATA), 18);
	return obj;
}

void free_descriptor (DESCRIPTOR_DATA *d)
{
	if ( d->host && *d->host )
		mem_free (d->host);
	if ( d->ip_addr && *d->ip_addr )
		mem_free (d->ip_addr);
	if ( d->showstr_head && *d->showstr_head )
		mem_free (d->showstr_head);
	if ( d->header && *d->header )
		mem_free (d->header);
	if ( d->edit_string && *d->edit_string )
		mem_free (d->edit_string);

	mem_free (d);
}

void free_account (ACCOUNT_DATA *account)
{
	STORED_PC_DATA	*pc;

	Memory_check = TRUE;

	if ( account->name && *account->name ) {
		mem_free (account->name);
		account->name = NULL;
	}
	if ( account->pwd && *account->pwd ) {
		mem_free (account->pwd);
		account->pwd = NULL;
	}
	if ( account->email && *account->email ) {
		mem_free (account->email);
		account->email = NULL;
	}
	if ( account->last_ip && *account->last_ip ) {
		mem_free (account->last_ip);
		account->last_ip = NULL;
	}

	while ( account->pc ) {
		pc = account->pc->next;
		if ( account->pc->name && *account->pc->name )
			mem_free (account->pc->name);
		mem_free (account->pc);
		account->pc = pc;
	}

	mem_free (account);

	Memory_check = FALSE;

	account = NULL;
}

/* release memory allocated for a char struct */

void free_char (CHAR_DATA *ch)
{
	struct memory_data	*mem;
	ROLE_DATA		*role;
	DREAM_DATA		*dream;
	PC_DATA			*pc;
	MOB_DATA		*mob;
	VAR_DATA		*var;
	ALIAS_DATA		*tmp_alias;

	Memory_check = TRUE;

	if ( ch->pc ) {
		while ( ch->pc->aliases ) {
			tmp_alias = ch->pc->aliases;
			ch->pc->aliases = ch->pc->aliases->next_alias;
			alias_free (tmp_alias);
		}
	}

	clear_pmote(ch);

	while ( ch->hour_affects )
		affect_remove (ch, ch->hour_affects);

	while ( ch->pc && ch->pc->dreams ) {

		dream = ch->pc->dreams;

		ch->pc->dreams = ch->pc->dreams->next;

		if ( dream->dream && *dream->dream ) {
			mem_free (dream->dream);
			dream->dream = NULL;
		}

		mem_free (dream);
		dream = NULL;
	}

	while ( ch->pc && ch->pc->dreamed ) {
		dream = ch->pc->dreamed;
		ch->pc->dreamed = ch->pc->dreamed->next;
		if ( dream->dream && *dream->dream ) {
			mem_free (dream->dream);
			dream->dream = NULL;
		}
		mem_free (dream);
		dream = NULL;
	}


	while ( ch && ch->remembers ) {
		mem = ch->remembers;
		ch->remembers = mem->next;
		if (mem->name && *mem->name){
			mem_free (mem->name);
			mem->name = NULL;
		}
		mem_free(mem);
		mem = NULL;
	}


	while ( ch->vartab && ch->vartab->next ) {

		var = ch->vartab;
		ch->vartab = var->next;

		if ( var->name && *var->name ) {
			mem_free (var->name);
			var->name = NULL;
		}

		mem_free (var);
		var = NULL;
	}


	if ( ch->pc ) {

		if ( (role = ch->pc->special_role) != NULL ) {
			if ( role->summary && *role->summary )
				mem_free (role->summary);
			if ( role->body && *role->body )
				mem_free (role->body);
			if ( role->date && *role->date )
				mem_free (role->date);
			if ( role->poster && *role->poster )
				mem_free (role->poster);
			mem_free (role);
			ch->pc->special_role = NULL;	
		}

		if ( ch->pc->account && *ch->pc->account ) {
			mem_free (ch->pc->account);
			ch->pc->account = NULL;
		}

		if ( ch->pc->site_lie && *ch->pc->site_lie ) {
			mem_free (ch->pc->site_lie);
			ch->pc->site_lie = NULL;
		}

		if ( ch->pc->imm_leave && *ch->pc->imm_leave ) {
			mem_free (ch->pc->imm_leave);
			ch->pc->imm_leave = NULL;
		}

		if ( ch->pc->imm_enter && *ch->pc->imm_enter ) {
			mem_free (ch->pc->imm_enter);
			ch->pc->imm_enter = NULL;
		}

		if ( ch->pc->creation_comment && *ch->pc->creation_comment ) {
			mem_free (ch->pc->creation_comment);
			ch->pc->creation_comment = NULL;
		}

		if ( ch->pc->email_address && *ch->pc->email_address ) {
			mem_free (ch->pc->email_address);
			ch->pc->email_address = NULL;
		}

		if ( ch->pc->msg && *ch->pc->msg ) {
			mem_free (ch->pc->msg);
			ch->pc->msg = NULL;
		}

		ch->pc->owner = NULL;
	}

	/*
	* for the typical mob proto (loaded at boot time)
	* these pointers are in perm memory and do not get
	* deleted. however newly minit'd mobs share memory
	* that is not protected, so freeing here will corrupt
	* the proto if it is not saved.
	*/
	
	if (!IS_NPC(ch)){

	if ( ch->voice_str && *ch->voice_str ) {
		mem_free (ch->voice_str);
		ch->voice_str = NULL;
	}

	if ( ch->travel_str && *ch->travel_str ) {
		mem_free (ch->travel_str);
		ch->travel_str = NULL;
	}

	if ( ch->pmote_str && *ch->pmote_str ) {
		mem_free (ch->pmote_str);
		ch->pmote_str = NULL;
	}

		if (ch->tname && *ch->tname){
			mem_free (ch->tname);
			ch->tname = NULL;
		}

		if (ch->name && *ch->name){
			mem_free (ch->name);
			ch->name = NULL;
		}

		if (ch->short_descr && *ch->short_descr){
			mem_free (ch->short_descr);
			ch->short_descr = NULL;
		}

		if (ch->long_descr && *ch->long_descr){
			mem_free (ch->long_descr);
			ch->long_descr = NULL;
		}

		if (ch->description && *ch->description){
			mem_free (ch->description);
			ch->description = NULL;
		}
	}

	else{

      CHAR_DATA* proto = vtom(ch->mob->virtual);
		if (proto){
         if (ch->tname && *ch->tname
				&& ch->tname != proto->tname){
             mem_free (ch->tname);
             ch->tname = NULL;
           }

         if (ch->name && *ch->name
				&& ch->name != proto->name){
             mem_free (ch->name);
             ch->name = NULL;
           }

         if (ch->short_descr && *ch->short_descr
				&& ch->short_descr != proto->short_descr ){
             mem_free (ch->short_descr);
             ch->short_descr = NULL;
           }

         if (ch->long_descr && *ch->long_descr
				&& ch->long_descr != proto->long_descr){
             mem_free (ch->long_descr);
             ch->long_descr = NULL;
           }

         if (ch->description && *ch->description
				&& ch->description != proto->description){
				mem_free (ch->description);
				ch->description = NULL;
			}
		}
		else{
			fprintf (stderr,
			"Proto not defined for NPC %d?\n",
			ch->mob->virtual);
		}
	}

	if ( ch->clans && *ch->clans ) {
		mem_free (ch->clans);
		ch->clans = NULL;
	}

	if ( ch->combat_log && *ch->combat_log ) {
		mem_free (ch->combat_log);
		ch->combat_log = NULL;
	}

	if ( ch->enemy_direction && *ch->enemy_direction ) {
		mem_free (ch->enemy_direction);
		ch->enemy_direction = NULL;
	}

	if ( ch->delay_who && !isdigit(*ch->delay_who) && *ch->delay_who ) {
		mem_free (ch->delay_who);
		ch->delay_who = NULL;
	}

	if ( ch->delay_who2 && !isdigit(*ch->delay_who2) && *ch->delay_who2 ) {
		mem_free (ch->delay_who2);
		ch->delay_who2 = NULL;
	}

	while ( ch->wounds )
		wound_from_char (ch, ch->wounds);

	while ( ch->lodged )
		lodge_from_char (ch, ch->lodged);

	while ( ch->attackers )
		attacker_from_char (ch, ch->attackers);

	while ( ch->threats )
		threat_from_char (ch, ch->threats);

	/* Don't erase the pc and mob data structures, since they need
	   to sit around until cleanup_the_dead pulses.  new_char will
	   clean them up.
	*/

	pc = ch->pc;
	mob = ch->mob;

	memset (ch, 0, sizeof (CHAR_DATA));

	ch->pc = pc;
	ch->mob = mob;

	if ( pc )
		mem_free (ch->pc);

	if ( mob )
		mem_free (ch->mob);

	mem_free (ch);

	ch = NULL;

	Memory_check = FALSE;
}

void free_obj (OBJ_DATA *obj)
{
	AFFECTED_TYPE		*af;
	OBJ_DATA		*tobj;
	WRITING_DATA		*writing;

	tobj = vtoo (obj->virtual);

		/* Make sure these arn't duplicate fields of the prototype */

	if ( !tobj || tobj->name != obj->name )
		mem_free (obj->name);

	if ( !tobj || tobj->short_description != obj->short_description )
		mem_free (obj->short_description);

	if ( !tobj || tobj->description != obj->description )
		mem_free (obj->description);

	if ( !tobj || tobj->full_description != obj->full_description )
		mem_free (obj->full_description);

	if ( !tobj || tobj->var_color != obj->var_color )
		mem_free (obj->var_color);

	if ( !tobj || tobj->desc_keys != obj->desc_keys )
		mem_free (obj->desc_keys);

	clear_omote(obj);

	obj->short_description = NULL;
	obj->description = NULL;
	obj->full_description = NULL;
	obj->var_color = NULL;
	obj->name = NULL;
	obj->desc_keys = NULL;

	while ( (af = obj->xaffected) ) {
		obj->xaffected = af->next;
		mem_free (af);
	}

	while ( obj->wounds )
		wound_from_obj (obj, obj->wounds);

	while ( obj->lodged )
		lodge_from_obj (obj, obj->lodged);

	while ( obj->writing ) {
		writing = obj->writing;
		obj->writing = writing->next_page;
		if ( writing->message )
			mem_free (writing->message);
		if ( writing->author )
			mem_free (writing->author);
		if ( writing->date )
			mem_free (writing->date);
		if ( writing->ink )
			mem_free (writing->ink);
		mem_free (writing);
	}

	memset (obj, 0, sizeof (OBJ_DATA));

	mem_free (obj);

}

/* read contents of a text file, and place in buf */

char *file_to_string(char *name)
{
	FILE *fl;
	char tmp[MAX_STRING_LENGTH]; /* max size on the string */
	char *string;
	int num_chars;
	
	if (!(fl = fopen(name, "r")))
	{
		perror("file-to-string");
		string = (char *)alloc (1, 4); 
		*string = '\0';
		return(string);
	}

	num_chars = fread (tmp, 1, MAX_STRING_LENGTH-1, fl);
	tmp [num_chars] = '\0';
	string = (char *)alloc (num_chars + 2, 4);
	strcpy (string, tmp);
	strcat (string, "\r\0");

	fclose (fl);

	return (string);
}

void clear_char (CHAR_DATA *ch)
{
	PC_DATA		*pc;
	MOB_DATA	*temp_mob;

	temp_mob = ch->mob;
	if (temp_mob){
		memset (temp_mob, 0, sizeof(MOB_DATA));
	}
	
	pc = ch->pc;
	if (pc)
		memset (pc, 0, sizeof(PC_DATA));
				
	memset (ch, 0, sizeof(CHAR_DATA));

	ch->pc = pc;
	ch->mob = temp_mob;

	ch->damage = 0;

	ch->room = NULL;
	ch->in_room = NOWHERE;
	ch->was_in_room = NOWHERE;
	ch->position = POSITION_STANDING;
	ch->default_pos = POSITION_STANDING;
	ch->wounds = NULL;
	ch->lodged = NULL;
	ch->mount = NULL;
	ch->following = NULL;
	ch->fighting = NULL;
	ch->subdue = NULL;
	ch->vehicle = NULL;
	ch->shop = NULL;
	ch->hour_affects = NULL;
	ch->equip = NULL;
	ch->desc = NULL;
	ch->hitcher = NULL;
	ch->hitchee = NULL;
	ch->aiming_at = NULL;
	ch->targeted_by = NULL;

	ch->next = NULL;
	ch->next_in_room = NULL;

	if ( ch->pc ) {
		ch->pc->is_guide = 0;
		ch->pc->admin_loaded = FALSE;
		ch->pc->profession = 0;
	}

	return;
}


void clear_object (OBJ_DATA *obj)
{
	memset (obj, 0, sizeof (OBJ_DATA));

	obj->silver = 0;
	obj->coppers = 0;
	obj->virtual = -1;
	obj->in_room = NOWHERE;
	obj->wounds = NULL;
	obj->lodged = NULL;
	obj->description = NULL;
	obj->short_description = NULL;
	obj->full_description = NULL;
	obj->equiped_by = NULL;
	obj->carried_by = NULL;
}

void save_char_objs (CHAR_DATA *ch, char *name)
{
	FILE		*of;
	char		fbuf [MAX_STRING_LENGTH];
	char		buf2 [MAX_STRING_LENGTH];

	snprintf (fbuf, MAX_STRING_LENGTH, "save/objs/%c/%s", tolower (*name), name);

	if ( !IS_NPC (ch) && ch->pc->create_state == STATE_DIED ) {
		snprintf (buf2, MAX_STRING_LENGTH,  "mv %s %s.died", fbuf, fbuf);
		system (buf2);
	}

	if( !(of = fopen (fbuf, "w")) ) {
		snprintf (buf2, MAX_STRING_LENGTH,  "ERROR: Opening obj save file. (%s)", ch->tname);
		system_log (buf2, TRUE);
		return;
	}

	write_obj_suppliment (ch, of);

	fclose (of);
}


/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
	char c;
	long int number;
	bool sign;

	do {
		c = getc( fp );
	} while ( isspace( c ) );

	number = 0;

	sign   = FALSE;
	if ( c == '+' ) {
		c = getc( fp );
	} else if ( c == '-' ) {
		sign = TRUE;
		c = getc( fp );
	}

	if ( !isdigit( c ) ) {
		system_log ("Fread_number(): bad format.", TRUE);
		abort();
	}

	while ( isdigit(c) ) {
		number = number * 10 + c - '0';
		c      = getc( fp );
	}

	if ( sign )
		number = 0 - number;

	if ( c == '|' )
		number += fread_number( fp );
	else if ( c != ' ' )
		ungetc( c, fp );

	return number;
}


/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
	static char	word [ MAX_INPUT_LENGTH ];
	char 		*pword;
	char		cEnd;

	do {
		cEnd = getc( fp );
	} while ( isspace( cEnd ) );

	if ( cEnd == '\'' || cEnd == '"' ) {
		pword   = word;
	} else {
		word[0] = cEnd;
		pword   = word+1;
		cEnd    = ' ';
	}

	for ( ; pword < word + MAX_INPUT_LENGTH; pword++ ) {
		*pword = getc( fp );
		if ( cEnd == ' ' ? isspace( *pword ) : *pword == cEnd ) {
			if ( cEnd == ' ' )
				ungetc( *pword, fp );
			*pword = '\0';
			return word;
		}
	}

	system_log ("Fread_word(): word too long.", TRUE);
	abort();
}

void boot_mobiles (void)
{
	char	buf [MAX_STRING_LENGTH];
	int		virtual;
	int		zone;
	FILE	*fp;

	for ( zone = 0; zone < MAX_ZONE; zone++ ) {

		snprintf (buf, MAX_STRING_LENGTH,  "%s/mobs.%d", REGIONS, zone);

		if ( (fp = fopen (buf, "r")) == NULL )
			continue;

		while (1) {
			if ( !fgets (buf, 81, fp) ) {
				system_log("Error reading mob file:", TRUE);
				system_log(buf, TRUE);
				perror ("Reading mobfile");
				abort();
			}

			if ( *buf == '#' ) {
				sscanf (buf, "#%d", &virtual);
				(void)fread_mobile (virtual, zone, fp);
			} else if ( *buf == '$' )
				break;
		}

		fclose (fp);
	}
}


void create_ticket_proto (void)
{
	OBJ_DATA		*obj;

	if ( vtoo (VNUM_TICKET) != NULL )
		return;

	obj = new_object ();

	clear_object (obj);

	obj->virtual			= VNUM_TICKET;

	add_obj_to_hash (obj);

	obj->name				= add_hash ("ticket small number paper");
	obj->short_description  = add_hash ("a small ostler's ticket");
	obj->description        = add_hash ("A small paper ticket with a number "
										"is here.");
	obj->full_description	= null_string;

	obj->obj_flags.weight     = 1;
	obj->obj_flags.type_flag  = ITEM_TICKET;
	obj->obj_flags.wear_flags = ITEM_TAKE;

	obj->in_room = NOWHERE;
}

void create_head_proto (void)
{
	OBJ_DATA		*obj;

	if ( vtoo (VNUM_HEAD) != NULL )
		return;

	obj = new_object ();

	clear_object (obj);

	obj->virtual			= VNUM_HEAD;

	add_obj_to_hash (obj);

	obj->name				= add_hash ("head");
	obj->short_description  = add_hash ("a head");
	obj->description        = add_hash ("A head is here.");
	obj->full_description	= null_string;

	obj->obj_flags.weight     = 10;
	obj->obj_flags.type_flag  = ITEM_HEAD;
	obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;

	obj->in_room = NOWHERE;
}

void create_corpse_proto (void)
{
	OBJ_DATA		*obj;

	if ( vtoo (VNUM_CORPSE) != NULL )
		return;

	obj = new_object ();

	clear_object (obj);

	obj->virtual			= VNUM_CORPSE;

	add_obj_to_hash (obj);

	obj->name				= add_hash ("corpse");
	obj->short_description	= add_hash ("a corpse");
	obj->description		= add_hash ("A corpse is here.");
	obj->full_description	= null_string;

	obj->obj_flags.weight	  = 1000;
	obj->o.container.capacity = 0;			/* No keeping things on a corpse */
	obj->obj_flags.type_flag  = ITEM_CONTAINER;
	obj->obj_flags.wear_flags = ITEM_TAKE;

	obj->in_room = NOWHERE;
}

void create_statue_proto (void)
{
	OBJ_DATA		*obj;

	if ( vtoo (VNUM_STATUE) != NULL )
		return;

	obj = new_object ();

	clear_object (obj);

	obj->virtual			= VNUM_STATUE;

	add_obj_to_hash (obj);

	obj->name				= add_hash ("statue");
	obj->short_description	= add_hash ("a remarkably lifelike statue");
	obj->description		= add_hash ("A remarkably lifelike statue looms here.");
	obj->full_description	= null_string;

	obj->obj_flags.weight	  = 1000;
	obj->o.container.capacity = 0;			/* No keeping things on a statue */
	obj->obj_flags.type_flag  = ITEM_CONTAINER;

	obj->in_room = NOWHERE;
	
	return;
}

void boot_objects (void)
{
	char		buf [MAX_STRING_LENGTH];
	int			virtual;
	int			zone;
	int			i = 0;
	CLAN_DATA	*clan;
	OBJ_DATA	*obj;
	FILE		*fp;

	for ( zone = 0; zone < MAX_ZONE; zone++ ) {

		snprintf (buf, MAX_STRING_LENGTH,  "%s/objs.%d", REGIONS, zone);

		if ( (fp = fopen (buf, "r")) == NULL )
			continue;

		while (1) {
			if ( !fgets (buf, 81, fp) ) {
				system_log("Error reading obj file:", TRUE);
				perror ("Reading objfile");
				abort();
			}

			if ( *buf == '#' ) {
				sscanf (buf, "#%d", &virtual);
				(void)fread_object (virtual, zone, fp);
			} else if ( *buf == '$' )
				break;
		}

		fclose (fp);
	}

	for ( clan = clan_list; clan; clan = clan->next ) {
		if ( clan->leader_vnum ) {
			if ( !(obj = vtoo (clan->leader_vnum)) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Note:  Clan leader obj %d does not exist for %d.",
							  clan->leader_vnum, i);
				system_log(buf, TRUE);
			} else
				SET_BIT (obj->obj_flags.extra_flags, ITEM_LEADER);
		}

		if ( clan->member_vnum ) {
			if ( !(obj = vtoo (clan->member_vnum)) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Note:  Clan member obj %d does not exist for %d.",
							  clan->member_vnum, i);
				system_log(buf, TRUE);
			} else
				SET_BIT (obj->obj_flags.extra_flags, ITEM_MEMBER);
		}

		if ( clan->omni_vnum ) {
			if ( !(obj = vtoo (clan->omni_vnum)) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Note:  Clan omni obj %d does not exist for %d.",
							  clan->omni_vnum, i);
				system_log(buf, TRUE);
			} else
				SET_BIT (obj->obj_flags.extra_flags, ITEM_OMNI);
		}
	}

	create_ticket_proto ();
	create_head_proto ();
	create_corpse_proto ();
	create_statue_proto ();
}

struct hash_data {
	int					len;
	char				*string;
	struct hash_data	*next;
};

void init_memory (void)
{
	int		i;

	if ( !(use_memory_base = (char *)alloc (MAX_MEMORY + PERM_MEMORY_SIZE, 5))) {
		perror ("memory allocation");
		system_log ("Init_memory() error - unable to malloc.", TRUE);
		abort();
	}

	use_memory_next = use_memory_base;
	use_memory_top = use_memory_base + MAX_MEMORY - 1;

	perm_memory = use_memory_base + MAX_MEMORY;
	perm_memory_next = perm_memory;
	perm_memory_top = perm_memory + PERM_MEMORY_SIZE - 1;

	for ( i = 0; i < NUM_BUCKETS; i++ )
		hash_buckets [i] = NULL;

	if ( !(overhead_base = (char *)alloc (MAX_OVERHEAD, 5)) ) {
		perror ("memory overhead allocation");
		system_log ("Init_memory() error - unable to malloc overhead.", TRUE);
		abort();
	}

	overhead_next = overhead_base;
	overhead_top = overhead_base + MAX_OVERHEAD - 1;

	null_string = get_perm (1);
	*null_string = '\0';
}

char *get_mem (int size)
{
	char	*allocated_block;

	if ( size <= 0 ) {
		system_log ("Get_mem() - allocation of negative bytes attempted.", TRUE);
		return NULL;
	}

	if ( use_memory_next + size + 4 > use_memory_top ) {
		system_log ("Get_mem() - exceeded allocation limit.", TRUE);
		abort();
	}

	allocated_block = use_memory_next;
	use_memory_next += size;

	while ( (int) use_memory_next % 4 )
		use_memory_next++;

	return allocated_block;
}

malloc_t get_perm (int size)
{
	char	*allocated_block;
	static int notified = 0;

	if ( !booting )
		return malloc (size);

	if ( perm_memory_next + size + 4 > perm_memory_top ) {
		if ( !notified ) {
			printf ("****************Out of perm space.\n");
			fflush (stdout);
			notified = 1;
		}
		return malloc (size);
	}

	allocated_block = perm_memory_next;
	perm_memory_next += size;

	while ( (int) perm_memory_next % 4 )
		perm_memory_next++;

	return allocated_block;
}

malloc_t get_overhead (int size)
{
	char	*allocated_block;

	if ( size <= 0 ) {
		system_log ("Get_overhead() - attempted to allocate negative or 0 bytes.", TRUE);
		return NULL;
	}

	if ( overhead_next + size + 4 > overhead_top ) {
		system_log ("Get_overhead() - exceeded allocation limit.", TRUE);
		abort();
	}

	allocated_block = overhead_next;
	overhead_next += size;

	while ( (int) overhead_next % 4 )
		overhead_next++;

	return allocated_block;
}

extern int     bytes_allocated;
extern int     first_free;
extern int     mud_memory;

#ifdef MEMORY_CHECK

extern MEMORY_T    *alloc_ptrs [];
MEMORY_T    *alloc_ptrs [100000];

int mem_free (malloc_t string)
{
	char			*p;
	MEMORY_T		*m;
	int				entry;
	int				bytes;
	int				dtype;
	extern int		mem_freed;

	if ( string >= (malloc_t) use_memory_base &&
		 string <= (malloc_t) use_memory_top )
		return 0;

	if ( string >= (malloc_t) perm_memory &&
		 string <= (malloc_t) perm_memory_top )
		return 0;

	if ( booting &&
		 string >= (malloc_t) overhead_base &&
		 string <= (malloc_t) overhead_top )
		return 0;

	if ( string == null_string )
		return 0;

	p = string;
	m = (MEMORY_T *) (p - sizeof (MEMORY_T));

	dtype = m->dtype;
	entry = m->entry;
	bytes = m->bytes;

	if ( x1 )
		printf ("- #%d @ %Xd for %d bytes: %d\n",
					entry, (int) p, bytes, dtype);

	if ( alloc_ptrs [entry] != m ) {
		system_log("Memory deallocation problem.", TRUE);
		printf ("Entry : %d\n", entry);
		printf ("m     : %d\n", (int) (m));
		printf ("allocs: %d\n", (int) alloc_ptrs [entry]);
		((int *) 0) [-1] = 0;
	}

	mud_memory -= bytes - sizeof (MEMORY_T);
	bytes_allocated -= bytes;

	mem_freed += bytes;

	first_free--;

	((MEMORY_T *) alloc_ptrs [first_free])->entry = entry;
	alloc_ptrs [entry] = alloc_ptrs [first_free];

	free (m);

	return 1;
}

/*
void set_bit (int bit_no)
{
	bit_no -= (int) alloc_ptrs [0];

    super_map [bit_no / 32] |= 1 << (bit_no % 32);
}

void reset_bit (int bit_no)
{
	bit_no -= (int) alloc_ptrs [0];

    super_map [bit_no / 32] &= ~(1 << (bit_no % 32));
}

int is_bit_set (int bit_no)
{
	bit_no -= (int) alloc_ptrs [0];

	return IS_SET (super_map [bit_no / 32], 1 << (bit_no % 32));
}

void clear_bits (void)
{
	int			i;

    for ( i = 0; i < SUPER_SIZE_WORDS; i++ )
        super_map [i] = 0;
}
*/

void print_bit_map (void)
{
	int				i;
	unsigned int	addr;
	int				old_addr;
	int				bytes;
	int				old_bytes;
	FILE			*fp;
	FILE			*fp2;

	if ( !(fp = fopen ("map", "w+")) ) {
		perror ("map");
		system_log("Unable to open 'map'.\n", TRUE);
		return;
	}

	for ( i = 0; i < first_free; i++ ) {
		fprintf (fp, "%09d  %10d\n",
					 (unsigned int) alloc_ptrs [i], alloc_ptrs [i]->bytes);
	}

	fclose (fp);

	system ("sort map > map_sort");

	if ( !(fp = fopen ("map_sort", "r")) ) {
		perror ("map_sort");
		system_log("Unable to open 'map_sort'.\n", TRUE);
		return;
	}

	if ( !(fp2 = fopen ("map_sort_x", "w+")) ) {
		perror ("map_sort_x");
		system_log("Unable to open 'map_sort_x'.\n", TRUE);
		return;
	}

	old_addr = 0;
	old_bytes = 0;

	while ( fscanf (fp, "%d %d\n", &addr, &bytes) == 2 ) {
		fprintf (fp2, "%4d  %09d  %10d\n",
						addr - old_addr - old_bytes,
						addr, bytes);
		old_addr = addr;
		old_bytes = bytes;
	}

	fclose (fp);
	fclose (fp2);
}

void check_memory (void)
{
	int			i;
	int			entry;
	int			bytes;
	int			check_mud_memory = 0;
	int			check_bytes_allocated = 0;
	int			failed = 0;
	int			dtype;
	int			objects_in_list = 0;
	int			characters_in_list = 0;
	OBJ_DATA	*o;
	CHAR_DATA	*tch;
	MEMORY_T	*m;

	check_objects = 0;
	check_characters = 0;

	for ( i = 0; i < first_free; i++ ) {

		m = alloc_ptrs [i];

		dtype = m->dtype;
		entry = m->entry;
		bytes = m->bytes;

		if ( entry != i ) {
			abort();
		}

		check_mud_memory += bytes - sizeof (MEMORY_T);
		check_bytes_allocated += bytes;

		if ( dtype == 18 )
			check_objects++;

		if ( dtype == 19 )
			check_characters++;
	}

	if ( check_mud_memory != mud_memory ) {
		failed = 1;
	}

	if ( check_bytes_allocated != bytes_allocated ) {
		failed = 1;
	}

	if ( failed )
		((int *)0)[-1]=0;

	for ( o = object_list; o; o = o->next )
        objects_in_list++;

    for ( tch = character_list; tch; tch = tch->next )
        characters_in_list++;
}

#else		/* NOVELL */

int mem_free (malloc_t string)
{

	if ( string >= (malloc_t) use_memory_base &&
		 string <= (malloc_t) use_memory_top ) {
		return 0;
	}

	if ( string >= (malloc_t) perm_memory &&
		 string <= (malloc_t) perm_memory_top ) {
		return 0;
	}

	if ( booting &&
		 string >= (malloc_t) overhead_base &&
		 string <= (malloc_t) overhead_top ) {
		return 0;
	}

	if ( string == null_string || string == NULL ) {
		return 0;
	}
        
        return 1;
}

#endif		/* NOVELL */

char *is_hashed (char *string)
{
	struct hash_data	*hash_entry;
	char				*tmp_string;

	hash_val = 0;
	hash_len = 0;

	if ( !string ) {
		return NULL;
	}

	tmp_string = string;

	while ( *tmp_string ) {
		hash_val += (int) *(tmp_string++);
		hash_len++;
	}

	hash_val = hash_val % NUM_BUCKETS;

	hash_entry = hash_buckets [hash_val];

	while ( hash_entry ) {
		if ( hash_entry->len == hash_len &&
			 !strcmp (string, hash_entry->string) )
			return hash_entry->string;

		hash_entry = hash_entry->next;
	}

	return 0;
}


char *add_hash (char *string)
{
	struct hash_data	*hash_entry;
	char			*hashed_string = NULL;

	if ( !string || !*string )
		return null_string;

	if ( !booting )
		return str_dup(string);

	if ( (hashed_string = is_hashed (string)) ) {
		hash_dup_strings++;
		hash_dup_length += strlen (string);
		return hashed_string;
	}

	hash_entry = get_overhead (sizeof (struct hash_data));

			/* hash_len and hash_val are statically maintained */

	hash_entry->string = get_mem (hash_len + 1);
	strcpy (hash_entry->string, string);
	hash_entry->len = hash_len;
	hash_entry->next = hash_buckets [hash_val];

	hash_buckets [hash_val] = hash_entry;

	return (hash_entry->string);
}

char *str_dup (char *this_string)
{
        char    *ret = NULL;
        int		temp = 0;
        

        if ( !this_string )
                return NULL;

        if ( this_string >= use_memory_base && this_string <= use_memory_top )
                return this_string;
		
 		temp = strlen(this_string)+1;
 		ret = (char *)alloc ((int)strlen(this_string)+1, 15);
 		strcpy(ret, this_string);
        memcpy (ret, this_string, temp);

        return ret;
}

void boot_recipe (void)
{
	FILE			*fp;
	int				obj_num;
	int				i1, i2, i3, i4, i5, i6, i7;
	int				r1, r2, r3, r4, r5, r6, r7;
	RECIPE_DATA		*recipe_entry;
	RECIPE_DATA		*alc_tmp;
	char			buf [MAX_STRING_LENGTH];

	if ( (fp = fopen (RECIPE_FILE, "r")) == NULL ) {
		snprintf (buf, MAX_STRING_LENGTH,  "NO RECIPE FILE:  %s.", RECIPE_FILE);
		return;
	}

	while (1) {

		obj_num = i1 = i2 = i3 = i4 = i5 = i6 = i7 = 0;
		r1 = r2 = r3 = r4 = r5 = r6 = r7 = 0;

		fscanf (fp, "#%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
				&obj_num, &i1, &i2, &i3, &i4, &i5, &i6, &i7,
						  &r1, &r2, &r3, &r4, &r5, &r6, &r7);

		if ( !obj_num ) {
			fclose (fp);
			return;
		}

		CREATE (recipe_entry, RECIPE_DATA, 1);

		recipe_entry->obj_created = obj_num;
		recipe_entry->ingredient [0] = i1;
		recipe_entry->ingredient [1] = i2;
		recipe_entry->ingredient [2] = i3;
		recipe_entry->ingredient [3] = i4;
		recipe_entry->ingredient [4] = i5;
		recipe_entry->ingredient [5] = i6;
		recipe_entry->ingredient [6] = i7;

		recipe_entry->reuse [0] = r1;
		recipe_entry->reuse [1] = r2;
		recipe_entry->reuse [2] = r3;
		recipe_entry->reuse [3] = r4;
		recipe_entry->reuse [4] = r5;
		recipe_entry->reuse [5] = r6;
		recipe_entry->reuse [6] = r7;

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
	}
}

void verify_exits (void)
{
	int				i;
	char			buf [MAX_STRING_LENGTH];
	ROOM_DATA		*room;

	for ( room = full_room_list; room; room = room->lnext ) {
		for ( i = 0; i < 6; i++ ) {
			if ( room->dir_option [i] &&
				 !vtor (room->dir_option [i]->to_room) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "Room %d doesn't go %s (%d) to room %d",
					room->virtual,
					dirs [i],
					i,
					room->dir_option [i]->to_room);
				system_log (buf, TRUE);
			}
		}
	}
}

void load_common_knowledge (void)
{
	char		*msg;
	FILE		*fp;
	COMMON_DATA	*cd;
	COMMON_DATA	*tcd;
	char		buf [MAX_STRING_LENGTH];

	common_list = NULL;

	if ( (fp = fopen (COMMON, "r")) == NULL ) {
		snprintf (buf, MAX_STRING_LENGTH,  "NO COMMON KNOWLEDGE FILE:  %s.", COMMON);
		return;
	}

	(void)fgets (buf, 256, fp);

	while ( 1 ) {

		msg = fread_string (fp);

		if ( !msg || !*msg )
			break;

		CREATE (cd, COMMON_DATA, 1);

		cd->message = msg;

		if ( !common_list )
			common_list = cd;
		else {
			tcd = common_list;

			while ( tcd->next )
				tcd = tcd->next;

			tcd->next = cd;
		}
	}

	fclose (fp);
}
