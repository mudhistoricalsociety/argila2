/** 
*	\file comm.c
*	Central Game Loop
*
*	Contains the main functions for running the game. Also includes staff 
*	related echoes, web site updates, and related utility functions.
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/file.h> 
#include <sys/resource.h> 
#include <getopt.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME   256

#define BOOT_DB_ABORT_THRESHOLD		15
#define RUNNING_ABORT_THRESHOLD		 5
#define ALARM_FREQUENCY				20

extern int port;


/* local globals */
bool	debug_mode = TRUE;	/* True to enable core dumps in crash recovery. */
bool	disable_timer_abort = FALSE;
bool	fCopyOver = FALSE;
bool	maintenance_lock = FALSE;
bool	pending_reboot = FALSE;
bool	socket_closed = FALSE;
bool	valgrind = FALSE;

DESCRIPTOR_DATA		*descriptor_list = NULL;
DESCRIPTOR_DATA		*next_to_process = NULL;
int		abort_threshold = BOOT_DB_ABORT_THRESHOLD;
int		last_checkpoint;
int 	connected = 0;
int 	conns=0;
int 	finished_booting = 0;
int 	forkedpid = 0;
int 	game_loop(int s);
int 	get_from_q(struct txt_q *queue, char *dest);
int 	guest_conns=0;
int 	init_socket(int port);
int 	maxdesc, avail_descs;
int 	new_descriptor(int s);
int 	process_input(DESCRIPTOR_DATA *t);
int 	process_output(DESCRIPTOR_DATA *t);
int 	reboot_mud = 0;		/* reboot the game after a shutdown */
int 	run_the_game(int port);
int 	shutd = 0;    		/* clean shutdown */
int 	slow_death = 0;		/* Shut her down, Martha, she's sucking mud */
int 	tics = 0;			/* for extern checkpointing */

long	bootstart;
long 	starttime;
long	crashtime;

struct timeval timediff(struct timeval *a, struct timeval *b);

void	check_sitebans(void);
void	close_socket(DESCRIPTOR_DATA *d);
void	close_sockets(int s);
void	flush_queues(DESCRIPTOR_DATA *d);
void	nonblock(int s);
void	parse_name(DESCRIPTOR_DATA *desc, char *arg);
void	secondary_socket(int t);

volatile sig_atomic_t	crashed = 0;

/* extern fcnts */

CHAR_DATA *make_char(char *name, DESCRIPTOR_DATA *desc);
void 	boot_db(void);
void 	zone_update(void);
void 	point_update( void );  /* In limits.c */
void 	mobile_activity(void);
void 	add_string(DESCRIPTOR_DATA *d, char *str);
void 	perform_violence(void);
void 	stop_fighting(CHAR_DATA *ch);
void 	show_string(DESCRIPTOR_DATA *d, char *input);

/* *********************************************************************
*  main game loop and related stuff				       *
********************************************************************* */

int main(int argc, char **argv)
{
	char buf[512];
	int short_option = 0; /* For the result of getopt() */
/*
 * Want to use GNU-style long options if getopt_long() is available
 * q.v. http://www.gnu.org/software/libc/manual/html_node/Getopt.html
 */

	int option_index = 0; /* Index into long_options */
	static struct option long_options[3] = {
	  	{"valgrind", no_argument, NULL, 'v'}, 
	  	{"copyover", required_argument, NULL, 'c'},
		{0, 0, 0, 0}
	};


/* Loop through command-line arguments */
	while (1) {

/*
* Want to use GNU-style long options if getopt_long() is available
*/
		short_option = getopt_long (argc, argv, "vc:", 
			long_options, &option_index);


/* -1 means we've reached the end of the arguments */
		if (short_option == -1) {
			break;
		}
	
		switch (short_option) {

			case 0:
				break;

			case 'v':
				valgrind = TRUE;
				break;

/* copyover - passes the mother descriptor on reboot */
			case 'c':
				fCopyOver = TRUE;
				s = strtol (optarg, 0, 10);
				break;

/* Unknown Option */
			case '?':
/* 
* If we are using the GNU getopt_long (), then it should have already
* printed an appropriate error message, and we can exit
*/
				if (isprint (optopt)) {
					fprintf (stderr, "ERROR: main "
						"Unknown option '-%c'.\n", optopt);
				}
				else {
					fprintf (stderr, "ERROR: main "
						"Unknown option '\\x%x'.\n",
						optopt);
				}
				exit (EXIT_FAILURE);
				break;

/* Unhandled Option */
			default:
				if (isprint (short_option)) {
					fprintf (stderr, "ERROR: main: "
						"Unhandled short_option '%c'.\n", 
						short_option);
				}
				else {
					fprintf (stderr, "ERROR: main "
						"Unhandled short_option '\\x%x'.\n",
						short_option);
				}

				exit (EXIT_FAILURE);
				break;
		}
	}

/* Process any non-option arguments (namely the port number) */
	if ( (optind >= argc) 
		|| !isdigit(*argv[optind]) 
		|| (port = atoi (argv[optind])) <= 1024) {
			fprintf (stderr, "ERROR: main: "
				"Please specify a port number above 1024.\n");
			exit (EXIT_FAILURE);
	} 

/* LUA set-up and initialization **/
	luaVM = luaL_newstate();
	luaL_openlibs (luaVM);


	if ( chdir (DFLT_DIR) < 0 ) {
		perror ("chdir");
		exit (EXIT_FAILURE);
	}
	

	bootstart=time(0);

	init_mysql();

	snprintf (buf, MAX_STRING_LENGTH,  "Running game on port %d.", port);

	system_log (buf, FALSE);
	
	srand(time(0));

	run_the_game(port);
	
	lua_close(luaVM);
	
	return(0);
}

#define	RPINET		0 	/* Currently disabled. */

/* Init sockets, run game, and cleanup sockets */
int run_the_game(int port)
{
	FILE		*fp;
	struct rlimit	rlp;

	void signal_setup(void);

	if ( !(fp = fopen("booting", "r")) ) {
		fp = fopen ("booting", "w+");
		fclose(fp);
		system_log ("Crash loop check initiated.", FALSE);
	}
	else {
		system_log ("Lockfile found during bootup - shutting down.", FALSE);
		fclose (fp);
		abort();
	}

	system_log ("Signal trapping.", FALSE);
	signal_setup();

	if ( (fp = fopen (".reboot", "r")) ) {
		(void)fgets (BOOT, 26, fp);
		fclose (fp);
		unlink (".reboot");
	}

	system ("ulimit -c unlimited");

	system_log ("Initializing CPU cycle alarm.", FALSE);
	init_alarm_handler();

	if ( !fCopyOver ) {
		system_log ("Opening mother connection.", FALSE);
		s = init_socket (port);
	}

	boot_db();

	system_log ("Entering game loop.", FALSE);
	starttime = time(0);

    	getrlimit (RLIMIT_CORE, &rlp);
    	rlp.rlim_cur = rlp.rlim_max;
    	setrlimit (RLIMIT_CORE, &rlp);
    	getrlimit (RLIMIT_CORE, &rlp);

        if ( (fp = fopen ("last_crash", "r")) ) {
                crashtime = fread_number(fp);
                fclose (fp);
        }

	if ( port == PLAYER_PORT ) {
		mysql_safe_query ("UPDATE server_statistics SET last_reboot = %d", (int)(time(0)));
	}

	game_loop (s);

	if ( port == PLAYER_PORT ) {
		save_tracks ();
		save_stayput_mobiles ();
		save_player_rooms ();
		save_banned_sites ();
	}

	if ( port == BUILDER_PORT )
		update_crafts_file ();

	mysql_safe_query ("DELETE FROM players_online WHERE port = %d", port);

	close_sockets(s);

	if ( port == PLAYER_PORT ) {
		fp = fopen (PATH_TO_WEBSITE "/stats.shtml", "w+");

		if ( fp != NULL ) {
			fprintf (fp, "<br><i>Our game server seems to be down. Our apologies for any inconvenience.</i></font><br><br>\n");
			fclose (fp);
		}
	}
	system_log ("Normal termination of game.", FALSE); /* Sito fix */
	mysql_close (database);
    	return 0;
}



/* Accept new connects, relay commands, and call 'heartbeat-functs' */

int game_loop(int s)
{
	fd_set			readfds;
	fd_set			writefds;
	fd_set			exceptfds;
	fd_set			visedit_fds;
	fd_set			tmp_read_fds;
	fd_set			tmp_write_fds;
	char			comm [MAX_INPUT_LENGTH] = { '\0' };
	char			dambuf [MAX_STRING_LENGTH] = { '\0' };
	char			fatbuf [MAX_STRING_LENGTH] = { '\0' };
	char			buf2 [MAX_STRING_LENGTH] = { '\0' };
	char			*temp_dam = NULL;
	char			*temp_fat = NULL;
	char			*temp_mana = NULL;
	char			*temp_breath = NULL;
	DESCRIPTOR_DATA *point = NULL;
	DESCRIPTOR_DATA *next_point = NULL;
	DESCRIPTOR_DATA	*d = NULL;
	CHAR_DATA		*tch = NULL;
	CHAR_DATA		*next_ch = NULL;
	int				pulse = 0;
	int				purse = 0;
	int				mask = 0;
	int				i = 0;
	struct timeval	null_time;
	struct timeval	pulse_time;
	struct timeval	current_time;
	struct rlimit	limit;
	bool				first_loop = TRUE;

	timerclear (&null_time);		/* a define in sys/time.h */
	FD_ZERO (&readfds);
	FD_ZERO (&writefds);
	FD_ZERO (&exceptfds);
	FD_ZERO (&visedit_fds);
	FD_ZERO (&tmp_read_fds);
	FD_ZERO (&tmp_write_fds);

	maxdesc = s;
	
	if ( fCopyOver ) {
		copyover_recovery();
		fCopyOver = FALSE;
		if ( s <= 0 ) {
			system_log ("Mother descriptor not found after copyover. Shutting down.", TRUE);
			shutd = 1;
		}
	}

	check_maintenance();

#ifndef MACOSX

	getrlimit(RLIMIT_NOFILE, &limit);		/* Determine max # descriptors */

#else

	limit.rlim_cur = limit.rlim_max = (rlim_t) 1024;

#endif
	avail_descs = (int)(limit.rlim_max - 2);

/** sigmask is a function returning int in signal.h **/
	mask = sigmask(SIGINT) |
		sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
		sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);

	gettimeofday (&time_now, NULL);

	finished_booting = 1;

	unlink ("booting");
	unlink ("recovery");

	while ( !shutd ) {

		FD_ZERO (&readfds);
		FD_ZERO (&writefds);
		FD_ZERO (&exceptfds);
		FD_ZERO (&visedit_fds);
		FD_SET (s, &readfds);

		for ( d = descriptor_list; d; d = d->next) {

			FD_SET (d->descriptor, &readfds);
			FD_SET (d->descriptor, &exceptfds);
			FD_SET (d->descriptor, &writefds);

			if ( !IS_SET (d->edit_mode, MODE_DONE_EDITING) )
				FD_SET (d->descriptor, &visedit_fds);
		}

		gettimeofday (&current_time, NULL);

		pulse_time = timediff (&current_time, &time_now);

				/* Compensate if we're not getting enough CPU */

		if ( pulse_time.tv_sec || pulse_time.tv_usec > 25 * run_mult ) {
			if ( pulse_time.tv_sec > 2 ) {
				snprintf (comm, MAX_STRING_LENGTH, "Insufficient CPU! %ld:%ld sec between slices",
							   (long) pulse_time.tv_sec,
							   (long) pulse_time.tv_usec);
				/* system_log (comm, TRUE); */
			}
			pulse_time.tv_sec = 0;
			pulse_time.tv_usec = 1;
			time_now = current_time;
		} else
			pulse_time.tv_usec = 25 * run_mult - pulse_time.tv_usec;

		sigprocmask(SIG_SETMASK,(sigset_t *)&mask,0);

		if ( select (maxdesc + 1,
				 &readfds,
				 &writefds,
				 &exceptfds,
				 &null_time) < 0 ) {
			perror ("Select poll");
			return -1;
		}

        if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, 
			&pulse_time) < 0) {
            perror("Select sleep");
            exit(EXIT_FAILURE);
        }

	    /* time_now is set to the exact time we expected our time slice.
               It may be slightly off.  However, it allows us to maintain a
               perfect 1/4 second pulse on average. */

		time_now.tv_usec += 25 * run_mult;
		if ( time_now.tv_usec > 100 * run_mult ) {
			time_now.tv_usec -= 100 * run_mult;
			time_now.tv_sec++;
		}

		sigprocmask(SIG_SETMASK,(sigset_t *)0,0);

		alarm_update();

		connected = 0;

                                /* Drop connections */
                 
		for (point = descriptor_list; point; point = next_point){
                        next_point = point->next;
			if (FD_ISSET(point->descriptor, &exceptfds)){
                                FD_CLR (point->descriptor, &readfds);
                                FD_CLR (point->descriptor, &exceptfds);
                                FD_CLR (point->descriptor, &writefds);
                                close_socket(point);
                        }
			else connected++;
                }

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

				/* New connections */

	 	if (FD_ISSET(s, &readfds)){
			if (new_descriptor(s) < 0){
				perror("New connection");
			}
		}

		for (point = descriptor_list; point; point = next_point){
                        next_point = point->next;
			if (FD_ISSET(point->descriptor, &readfds)){
                                if (process_input(point) < 0) {
                                        FD_CLR (point->descriptor, &readfds);
                                        FD_CLR (point->descriptor, &exceptfds);
                                        FD_CLR (point->descriptor, &writefds);
                                        close_socket(point);
                                }
                }
		}

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

		/* process_commands; */

		for (point = descriptor_list; point; point = next_to_process ){
			next_to_process = point->next;

			if ((--(point->wait) <= 0) && get_from_q(&point->input, comm)){
				point->wait = 1;
							
				/* reset idle time */

				point->time_last_activity = mud_time;

				if ( point->character && !point->character->deleted && point->character->pc != NULL ){
					point->character->pc->time_last_activity = mud_time;
				}

				if ( point->original && !point->original->deleted && point->original->pc != NULL ){
					point->original->pc->time_last_activity = time(0);
				}

				point->prompt_mode = 1;

				if ( point->edit_index != -1 ){
					edit_string (point, comm);
				}
				else if ( point->str ){
					add_string (point, comm);
				}
				else if ( point->showstr_point ){
					show_string (point, comm);
				}
				else if ( !point->connected ) {
					if ( point->showstr_point ){
						show_string (point, comm);
					}
					else if ( point->character ){
						command_interpreter (point->character, comm);
				}
				}
				else {
					nanny (point, comm);
			}
		}
		}

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

		for ( point = descriptor_list; point; point = next_point ) {
			next_point = point->next;

			if (FD_ISSET(point->descriptor, &writefds) && point->output.head) {
				if (process_output(point) < 0) {
					FD_CLR (point->descriptor, &readfds);
					FD_CLR (point->descriptor, &exceptfds);
					FD_CLR (point->descriptor, &writefds);
					close_socket(point);
				} 
				else
					point->prompt_mode = 1;
			}
		}

		for ( point = descriptor_list; point; point = next_point ) {

			next_point = point->next;

			if ( !IS_SET (point->edit_mode, MODE_DONE_EDITING) )
				continue;

			if ( point->prompt_mode ) {

				if ( point->edit_index != -1 )
					/** do nothing in this case **/;

				else if ( point->str )
					write_to_descriptor (point, point->connected == CON_CREATION ? "> " : "] ");

				else if ( point->showstr_point )
					write_to_descriptor (point, "*** Press return to continue - 'q' to quit *** ");

				else if ( !point->connected ) {
					if ( GET_FLAG(point->character, FLAG_NOPROMPT) ) {
						snprintf (s_buf, MAX_STRING_LENGTH,  "%s%s%s> ",
								IS_NPC (point->character) ? "" : "",
								IS_SET (point->character->flags, FLAG_WIZINVIS) ? "##" : "",
								IS_SET (point->character->flags, FLAG_ANON) ? "" : "");
							write_to_descriptor (point, s_buf);
					}
					else {
						temp_dam = wound_total(point->character);
						snprintf (dambuf, SMALL_STRING_LENGTH, "%s", temp_dam);	
						temp_fat = fatigue_bar(point->character);
						snprintf (fatbuf, SMALL_STRING_LENGTH, "%s", temp_fat);
					

						if ( !get_affect (point->character, AFFECT_HOLDING_BREATH) ) {
							if ( !IS_SET (point->character->flags, FLAG_HARNESS) ){ 
								snprintf (buf2, MAX_LINE_LENGTH,  "%s / %s> ", dambuf, fatbuf);
							}
							else{
								temp_mana = mana_bar(point->character);
								snprintf (buf2, MAX_LINE_LENGTH,  "%s / %s / %s> ", dambuf, temp_mana, fatbuf);
							}
						}
						else {
							if ( !IS_SET (point->character->flags, FLAG_HARNESS) ){
								temp_breath = breath_bar (point->character);
								snprintf (buf2, MAX_LINE_LENGTH,  "%s / %s / %s> ", dambuf, fatbuf, temp_breath);
							}
					
							else {
								temp_mana = mana_bar(point->character);
								temp_breath = breath_bar (point->character);
								snprintf (buf2, MAX_LINE_LENGTH,  "%s / %s / %s / %s> ", dambuf, temp_mana, fatbuf, temp_breath);
							}
						}/* ? */
						snprintf (s_buf, MAX_LINE_LENGTH,  "<%s%s%s",
							IS_NPC (point->character) ? "=" : "",
							IS_SET (point->character->flags, FLAG_WIZINVIS) ? "##" : "",
							IS_SET (point->character->flags, FLAG_ANON) ? "##" : "");

						strcat (s_buf, buf2);

						write_to_descriptor (point, s_buf);
					}
				}

				point->prompt_mode = 0;
			}
		}

		/* handle heartbeat stuff */
		/* Note: pulse now changes every 1/4 sec  */

		pulse++;

/* Every second */
		if ( !(pulse % SECOND_PULSE) ) {
			second_affect_update ();
			if ( pending_reboot )
				check_reboot();
		}

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

/* every fourth pulse , offset by 1  */
		if ( !((pulse + 1) % 4) )
			delayed_trigger_activity ();

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

/* Every delay checkpoint (pulse offset by 3) */
		if ( !((pulse + 3) % PULSE_DELAY) ) 
			update_delays ();

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

/* whatever pulse mobs change on */
		if ( !((pulse + 1) % PULSE_SMART_MOBS) && port != BUILDER_PORT )
			mobile_routines(pulse);

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

/* every update check (pulse offset by 2)(every 4 seconds currently) */
		if ( !((pulse + 2) % UPDATE_PULSE) )
			point_update();

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

/* 10 seconds (10, 20, 30...) game_time */
		if ( !(pulse % (10 * SECOND_PULSE)) ){
			ten_second_update ();
			larg_trigger_time_select();	
		}

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

/* 10 seconds (15, 25, 35...) game_time */
		if ( !((pulse + 5) % (10 * SECOND_PULSE)) ) {
			check_maintenance ();
			update_website();
			vote_notifications();
		}

/* 1 minute Real time */
		if ( !(pulse % (PULSES_PER_SEC*60)) ) {
			check_idlers ();
			check_linkdead();
			check_sitebans();
			process_reviews();
		}

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

/* every autosave checkpoint (pulse offset by 5) */
		if ( !((pulse + 5) % PULSE_AUTOSAVE*1) ) {
			autosave ();
		}

                if ( !((pulse + 5) % PULSE_AUTOSAVE*12) && port == PLAYER_PORT ) {
                        save_stayput_mobiles ();
                }

		if ( !((pulse + 5) % PULSE_AUTOSAVE*13) ) {
			save_tracks();
		}
		if ( !((pulse + 5) % PULSE_AUTOSAVE*14) ) {
			save_banned_sites();
		}
		if ( !((pulse + 5) % PULSE_AUTOSAVE*15) ) {
			save_player_rooms();
		}
		
/* 2 minutes of gametime */		
		if ( !(pulse % (120 * SECOND_PULSE)) ) {
			newbie_hints();
		}

		if ( !((pulse + 5) % (PULSE_ZONE*3)) )
			refresh_zone ();

		if ( !((pulse + 9) % (PULSE_AUTOSAVE*5)) ) {
			if ( port == BUILDER_PORT ) {
				update_crafts_file();
			}
			update_website_statistics();
		}



/* 4 hours of gametime */
		if ( !(pulse % (SECOND_PULSE * 60 * 60 * 4)) ) {
			if ( port != BUILDER_PORT ) {
				for ( tch = character_list; tch; tch = tch->next ) {
					if ( tch->deleted )
						continue;
					if ( !IS_NPC (tch) || !IS_SET (tch->flags, FLAG_KEEPER) )
						continue;
					purse = number(20, 60);
					while ( purse > 0 )
						purse -= vnpc_customer (tch, purse);
					refresh_colors (tch);
				}
			}
		}
		
/* 30 minutes of game time */
		if ( !(pulse % (SECOND_PULSE * 60 * 30)) ) {
			
		}

/* RL minute */
		if ( time (0) >= next_minute_update )
			rl_minute_affect_update ();

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

/* RL hour */
		if ( time (0) >= next_hour_update ) {
			hourly_update ();
			update_room_tracks();
			weather_and_time (1);
			hour_affect_update ();
			room_update ();
		}

		if ( knockout ) {
			cleanup_the_dead (0);
			knockout = 0;
		}

		if ( !((pulse + 2) % pulse_violence) ) {
			perform_violence();
			for ( i = 0; i <= 99; i++ ) {
	            		if (weather_info[i].lightning)
       		         		if (number(90,150) < weather_info[i].clouds)
       		             			send_outside("A fork of lightning flashes in the sky.\n");
       			}
	 	}

		if ( !((pulse + 1) % PULSE_MOBILE) ) {
			cleanup_the_dead (0);
		}

		if ( pulse > 86400 ) {
			pulse = 0;
		}

		if ( quarter_event_list )
			process_quarter_events ();

		while ( random_char_list )
			free_random_char (random_char_list->id);

		tics++;        /* tics since last checkpoint signal */

		if ( first_loop ) {
			for ( tch = character_list; tch; tch = next_ch ) {
				next_ch = tch->next;
				if ( tch->deleted )
					continue;
				trigger(tch, "", TRIG_HOUR);
			}
		}

		first_loop = FALSE;
	}
	
    return 0;
}

void handle_sec_input (int clntSocket)
{
	FILE		*fp;
	char		buf [MAX_STRING_LENGTH];
	char		file [MAX_STRING_LENGTH];
	int		recvMsgSize;

	snprintf (file, MAX_STRING_LENGTH, "rpinet");
	*buf = '\0';

	if ( (recvMsgSize = recvfrom(clntSocket, buf, MAX_STRING_LENGTH, 0, NULL, 0)) < 0)
		return;

	buf [recvMsgSize-1] = '\0';

	if ( !(fp = fopen (file, "w+")) )
		return;
	fprintf (fp, "%s", buf);
	fclose (fp);

	close (clntSocket);
}

void secondary_socket (int t)
{
	int 			clntSock, i;
	struct sockaddr_in	ServAddr;
	struct sockaddr_in	ClntAddr;

#ifdef __socklen_t_defined
	socklen_t		clntLen;
#else
	int			clntLen;
#endif

	char			ip [MAX_STRING_LENGTH];
	char			buf [MAX_STRING_LENGTH];

	memset (&ServAddr, 0, sizeof(ServAddr));
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	ServAddr.sin_port = htons(14566);

	for ( ;; ) {

		clntLen = sizeof(ClntAddr);

		if ( (clntSock = accept(t, (struct sockaddr *) &ClntAddr, &clntLen)) < 0 )
			continue;

		*ip = '\0';

	        for ( i = 0; i < sizeof (ClntAddr.sin_addr); i++ ) {
       	        	snprintf (ip + strlen (ip), MAX_STRING_LENGTH, "%d", *(((char *) &ClntAddr.sin_addr)+i));
       	        	if ( i != sizeof (ClntAddr.sin_addr) -1 )
       	                	snprintf (ip + strlen (ip), MAX_STRING_LENGTH, ".");
       		}

		if ( str_cmp (ip, "127.0.0.1") ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Connection refused.\n");
			(void)send (clntSock, buf, strlen(buf), 0);
			close (clntSock);
			continue;
		}

		handle_sec_input (clntSock);
	}
}

void newbie_hints (void)
{
	DESCRIPTOR_DATA		*d;
	NEWBIE_HINT		*hint;
	char			buf [MAX_STRING_LENGTH];
	char			*p;
	int			i, limit, hintnum;

	for ( hint = hint_list, limit = 1; hint; hint = hint->next, limit++ )
		;
	limit--;

	for ( d = descriptor_list; d; d = d->next ) {
		if ( !d->character )
			continue;
		if ( IS_NPC (d->character) )
			continue;
		if ( !IS_SET (d->character->plr_flags, NEWBIE_HINTS) )
			continue;
		hintnum = number(1, limit);
		for ( hint = hint_list, i = 1; hint; hint = hint->next, i++ ) {
			if ( i != hintnum )
				continue;
			reformat_string (hint->hint, &p);
			snprintf (buf, MAX_STRING_LENGTH,  "\r\n#6%s#0", p);
			send_to_char (buf, d->character);
			mem_free (p);
			break;
		}
	}
}

void unban_site (SITE_INFO *site)
{
	SITE_INFO	*tmp_site;
	char		buf [MAX_STRING_LENGTH];

	if ( !site )
		return;

	if ( banned_site == site ) {
        	snprintf (buf, MAX_STRING_LENGTH,  "The siteban has been lifted on %s.\n", banned_site->name);
                send_to_gods (buf);
		banned_site = site->next;
	}

	for ( tmp_site = banned_site; tmp_site; tmp_site = tmp_site->next ) {
		if ( tmp_site->next == site ) {
                        snprintf (buf, MAX_STRING_LENGTH,  "The siteban has been lifted on %s.\n", tmp_site->next->name);
                        send_to_gods (buf);
                        tmp_site->next = site->next;
			continue;
		}
	}

	mem_free (site->name);
	mem_free (site->banned_by);
	mem_free (site);
	site = NULL;

	save_banned_sites();
}

void check_sitebans()
{
	SITE_INFO	*site, *next_site;

	if ( port != PLAYER_PORT )
		return;

	if ( banned_site ) {
		if ( banned_site->banned_until != -1 && time(0) >= banned_site->banned_until ) {
			unban_site (banned_site);
		}
	}

	for ( site = banned_site; site; site = next_site ) {
		next_site = site->next;
		if ( !site->next || site->next->banned_until == -1 )
			continue;
		if ( site->next->banned_until != -1 && time(0) >= site->next->banned_until ) {
			unban_site (site->next);
			continue;
		}
	}
}

void spellcheck (CHAR_DATA *ch, char *description)
{
}

void check_maintenance (void)
{
	FILE	*fp;

	if ( (fp = fopen (".mass_emailing", "r")) ) {
		send_to_gods ("...mass-emailing completed.\n");
		fclose(fp);
		unlink (".mass_emailing");
	}

	if ( (fp = fopen ("maintenance_lock", "r")) && port == PLAYER_PORT ) {
		if ( !maintenance_lock )
			send_to_all ("#2The server is now locked down for maintenance.#0\n");
		maintenance_lock = TRUE;
		fclose (fp);
	}
	else if ( !fp && port == PLAYER_PORT ) {
		if ( maintenance_lock )
			send_to_all ("#2The server is now open for play.#0\n");
		maintenance_lock = FALSE;
	}
}

void vote_notifications (void)
{
	MYSQL_RES	*result;
	MYSQL_ROW	row;
	DESCRIPTOR_DATA	*d;

	if ( port != PLAYER_PORT )
		return;

	mysql_safe_query ("SELECT * FROM vote_notifications");
	result = mysql_store_result (database);

	while ( (row = mysql_fetch_row(result)) ) {
		for ( d = descriptor_list; d; d = d->next ) {
			if ( !d->ip_addr || !d->account || !d->character || d->connected != CON_PLYNG )
				continue;
			if ( IS_SET (d->account->flags, ACCOUNT_NOVOTE) )
				continue;
			if ( !str_cmp (d->ip_addr, row[0]) )
				send_to_char ("#6Your vote has been recorded. Thank you for supporting our community!#0\n", d->character);
		}
	}

	if ( result )
		mysql_free_result (result);

	mysql_safe_query ("DELETE FROM vote_notifications");
	return;
}

void update_website (void)
{
	FILE		*ft = NULL;
	CHAR_DATA 	*tch = NULL;
	DESCRIPTOR_DATA *d = NULL;
	DESCRIPTOR_DATA *d_next = NULL;
	int		online = 0;
	int		j = 0;
	int		count = 0;
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	char	*temp_arg = NULL;
	MYSQL_RES	*result = NULL;
	MYSQL_ROW	row;

	mysql_safe_query ("DELETE FROM players_online WHERE port = %d", port);

	for ( d = descriptor_list; d; d = d_next ) {
		j++;
		d_next = d->next;
		if ( !d->character || IS_NPC (d->character) )
			continue;
		if ( d->connected != CON_PLYNG )
			continue;
		if ( !d->account )
			continue;
		if ( IS_MORTAL (d->character) )
			count++;
		mysql_safe_query ("INSERT INTO players_online VALUES ('%s', '%s', '%s', %d, %d)", d->account->name, d->character->tname, d->host, d->character->in_room, port);
	}

	if ( port == PLAYER_PORT ) {
		if ( !(ft = fopen(PATH_TO_WEBSITE "/stats.shtml", "w+")) )
			return;

		for ( tch = character_list; tch; tch = tch->next ) {
			if ( tch->deleted )
				continue;
			if ( tch->desc && IS_MORTAL(tch) )
				online++;
		}
		
		temp_arg = time_string(NULL);
		snprintf (buf, MAX_STRING_LENGTH,  "<br><i>%s</i></font><br><br>\n", temp_arg);

		fputs (buf, ft);

		fclose (ft);

		mysql_safe_query ("SELECT most_online FROM newsletter_stats");
		result = mysql_store_result (database);
		if ( !result || !mysql_num_rows(result) )
			return;
		
		row = mysql_fetch_row(result);

		if ( strtol(row[0], NULL, 10) < count ) {
			mysql_safe_query ("UPDATE newsletter_stats SET most_online = %d", count);
		}

		if ( result )
			mysql_free_result (result);

		system ("chmod 644 " PATH_TO_WEBSITE "/stats.shtml");

		mysql_safe_query ("UPDATE server_statistics SET max_players = %d", count_max_online);

	}

	return;
}

/* ******************************************************************
  general utility stuff (for local use)				
****************************************************************** */

int get_from_q(struct txt_q *queue, char *dest)
{
	struct txt_block *tmp;

	/* Q empty? */
	if (!queue->head)
		return(0);

	tmp = queue->head;
	strcpy (dest, queue->head->text);
	queue->head = queue->head->next;

	if ( queue->head == NULL )
		queue->tail = NULL;

	mem_free (tmp->text);
	mem_free (tmp);

	return(1);
}

void write_to_q(char *txt, struct txt_q *queue)
{
	struct txt_block	*new = NULL;
	int				seen_cr = 0;
	int				char_count = 0;
	char			*temp = NULL;
	char			*mess = NULL;
	int				index = 0;

	if ( !queue )
		return;

	CREATE (new, struct txt_block, 1);
		
	temp = txt;

/** counts characters in string, and determines if CR/NL has been seen**/
	while ( temp[index] ) {

		if ( (temp[index] == '\n') ) {

			if ( !seen_cr ) {
				seen_cr = 1;
				char_count++;
			}

			char_count++;
		}

		else if ( (temp[index] != '\r')) {
			char_count++;
			seen_cr = 0;
		}

		index++;
	}

	new->text = (char *)alloc (char_count + 1, 2);

	temp = txt;
	mess = new->text;
	seen_cr = 0;

	while ( *temp ) {

		if ( *temp == '\n' ) {

			if ( !seen_cr ) {
				seen_cr = 1;
				*mess = '\r';
				mess++;
			}

			*mess = '\n';
			mess++;
		}

		else if ( *temp != '\r' ) {
			*mess = *temp;
			mess++;
			seen_cr = 0;
		}

		temp++;
	}

	*mess = '\0';

		/* Q empty? */

	if ( !queue->head ) {
		new->next = NULL;
		queue->head = queue->tail = new;
	}

	else {
		queue->tail->next = new;
		queue->tail = new;
		new->next = NULL;
	}
	return;
}

struct timeval timediff(struct timeval *a, struct timeval *b)
{
	struct timeval rslt, tmp;

	tmp = *a;

	if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0)
	{
		rslt.tv_usec += 100 * run_mult;
		--(tmp.tv_sec);
	}
	if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0)
	{

		rslt.tv_usec = 0;
		rslt.tv_sec =0;
	}
	return(rslt);
}

/* Empty the queues before closing connection */
void flush_queues(DESCRIPTOR_DATA *d)
{
	char dummy[MAX_STRING_LENGTH];

	while (get_from_q(&d->output, dummy));
	while (get_from_q(&d->input, dummy));
}

/* ******************************************************************
*  socket handling							 *
****************************************************************** */
int init_socket(int port)
{
     int skt, opt;
     struct sockaddr_in sa;
     struct linger ld;

     memset(&sa, 0, sizeof(struct sockaddr_in));
     sa.sin_family = AF_INET;
     sa.sin_port = htons(port);

     if ((skt = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("init_socket: socket()");
		exit(EXIT_FAILURE);
     }
    opt = 1;
    if (setsockopt(skt,SOL_SOCKET,SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
		perror("init_socket:setsockopt()");
		exit(EXIT_FAILURE);
        }
         ld.l_onoff = 0;	/* let's make sure this isn't on */
         ld.l_linger = 1000; /* who cares what this is */

    if (setsockopt(skt, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof(ld)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
    }

    if (bind(skt, (struct sockaddr *)&sa, sizeof(sa)) < 0) {

		perror("bind()");

		if( close(skt) ) {
			perror("error closing socket");
			exit(EXIT_FAILURE);
		}

		exit(EXIT_FAILURE);
    }

    if( listen(skt, 5) < 0 ) {
		perror("listen");
		exit(EXIT_FAILURE);
    }
    return(skt);
}

void init_descriptor (struct descriptor_data *newd, int desc)
{
	/* init desc data */
	conns++;

	newd->descriptor = desc;
	newd->wait = 1;
	newd->prompt_mode = 1;   
	
	newd->str = NULL;
	newd->showstr_head = NULL;
	newd->showstr_point = NULL;
	newd->header = NULL;
	newd->edit_string = NULL;
	newd->screen = NULL;
	newd->stored = NULL;
	newd->account = NULL;
	newd->character = NULL;
	newd->original = NULL;
	newd->snoop.snooping = NULL;
	newd->snoop.snoop_by = NULL;
	newd->proc = NULL;
	newd->pending_message = NULL;
	*newd->last_input = '\0';	     
	*newd->buf = '\0';
	SET_BIT (newd->edit_mode, MODE_DONE_EDITING);
	newd->login_time = time_now;
	newd->time_last_activity = mud_time;
	newd->edit_index = -1;
}

int new_descriptor (int s)
{
	int			desc = 0;

#ifdef __socklen_t_defined
	socklen_t		size = 0;
#else
 	int			size = 0;
#endif

	struct sockaddr_in	sock;
	DESCRIPTOR_DATA		*newd = NULL;
	DESCRIPTOR_DATA		*td = NULL;
	char				ip [MAX_STRING_LENGTH] ={'\0'};
	struct hostent		*from = NULL;
	char				*temp_arg = NULL;
	
	size = sizeof (sock);

	if((desc=accept(s,(struct sockaddr *)&sock, &size))<0) {
		perror("Accept");
		return(-1);
	}

	nonblock(desc);
	if ((maxdesc + 1) >= avail_descs) {
		(void)write (desc, "The game is full.  Try later.\n\r", 31);
		close(desc);
		return(0);
	}
	else
		if (desc > maxdesc)
			maxdesc = desc;

	CREATE (newd, DESCRIPTOR_DATA, 1);

#include <arpa/inet.h>
	strcpy(ip,inet_ntoa(sock.sin_addr));
	/******** end commented out section **********/
 /******changed to above 2 lines***********
	
	int			i;

	*ip = '\0';
	
	for ( i = 0; i < sizeof (sock.sin_addr); i++ ) {
		snprintf (ip + strlen (ip), MAX_STRING_LENGTH, "%d", *(((char *) &sock.sin_addr)+i));
		if ( i != sizeof (sock.sin_addr) -1 )
			snprintf (ip + strlen (ip), MAX_STRING_LENGTH, ".");
	}
*******************end of change***********/

	newd->ip_addr = str_dup(ip);
	newd->host = str_dup(ip);

	if ( !resolved_host (newd->host) ) {
		from = gethostbyaddr ((char *) &sock.sin_addr, sizeof((char *) &sock.sin_addr), AF_INET);
		if ( from && *from->h_name != '-' ) {
			mysql_safe_query ("INSERT INTO resolved_hosts VALUES ('%s', '%s', %d)", ip, from->h_name, (int)time(0)); 
			mem_free (newd->host);
			newd->host = str_dup (from->h_name);
		}
		else {
			mysql_safe_query ("INSERT INTO resolved_hosts VALUES ('%s', '%s', %d)", ip, ip, (int)time(0)); 
		}
	}
	else {
		mem_free (newd->host);
		newd->host = NULL;
		temp_arg = resolved_host(ip);
		newd->host = str_dup (temp_arg);
		newd->resolved = 1;
	}

	init_descriptor (newd, desc);

	if ( !descriptor_list )
		descriptor_list = newd;
	else for ( td = descriptor_list; td; td = td->next ) {
		if ( !td->next ) {
			newd->next = NULL;
			td->next = newd;
			break;
		}
	}

	if ( connected > MAX_CONNECTIONS ) {
		SEND_TO_Q ("\r\nWe apologize for the inconvenience, but the MUD is currently full.\r\n\r\nPlease try back again later. Thank you.\r\n\r\n", newd);
		newd->connected = CON_PENDING_DISC;
		return (0);
	}

	if ( !maintenance_lock ){
		temp_arg = get_text_buffer (NULL, text_list, "greetings");
		SEND_TO_Q (temp_arg, newd);
	
	}else {
		temp_arg = get_text_buffer (NULL, text_list, "greetings.maintenance");
		SEND_TO_Q (temp_arg, newd);
	}
	
	SEND_TO_Q ("Your Selection: ", newd);
	newd->connected = CON_LOGIN;

	return(0);
}

int process_output(DESCRIPTOR_DATA *t)
{
	char i[MAX_STRING_LENGTH + 1];

	if (!t->prompt_mode && !t->connected && t->edit_index == -1 )
		if (write_to_descriptor(t, "\r\n") < 0)
			return(-1);

	/* Cycle thru output queue */
	while (get_from_q(&t->output, i))
	{  
		if(t->snoop.snoop_by && t->snoop.snoop_by->desc != NULL && !IS_NPC (t->snoop.snoop_by) )
		{
			write_to_q("% ",&t->snoop.snoop_by->desc->output);
			write_to_q(i,&t->snoop.snoop_by->desc->output);
		}
		if (write_to_descriptor(t, i))
			return(-1);
	}
	
	if ( !t->connected && !(t->character && !IS_NPC(t->character) && 
		 GET_FLAG (t->character, FLAG_COMPACT)))
		if ( IS_SET (t->edit_mode, MODE_DONE_EDITING) &&
			 t->edit_index == -1 )
			if (write_to_descriptor(t, "\r\n") < 0)
				return(-1);

	return(1);
}

int write_to_descriptor (DESCRIPTOR_DATA *d, char *txt)
{
	int		sofar;
	int		thisround;
	int		total;
	char		ansi_buf [MAX_STRING_LENGTH * 2];

	(void)colorize (txt, ansi_buf, d);

	total = strlen (ansi_buf);
	sofar = 0;

	do {
		thisround = write (d->descriptor, ansi_buf + sofar, total - sofar);
		if ( thisround < 0 ) {
			perror ("Write to socket");
			return -1;
		}

		sofar += thisround;

	} while ( sofar < total );

	return 0;
}

int process_input (DESCRIPTOR_DATA *t)
{
	int		sofar;
	int		thisround;
	int		begin;
	int		squelch;
	int		i;
	int		k;
	int		flag;
	char	tmp [MAX_STRING_LENGTH + 100];
	char	buffer [MAX_STRING_LENGTH + 100];

	sofar = 0;
	flag = 0;
	begin = strlen (t->buf);

	/* Read in some stuff */
	do {
		if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
			MAX_INPUT_LENGTH - (begin + sofar) - 1)) > 0)
			sofar += thisround;		
		else
			if ( thisround < 0 )
				if ( errno != EWOULDBLOCK ) {
					return -1;
				} else
					break;
			else {
				return -1;
		}
	} while ( !ISNEWL (*(t->buf + begin + sofar - 1)) );
	

	*(t->buf + begin + sofar) = 0;

	if ( !IS_SET (t->edit_mode, MODE_DONE_EDITING) ) {
		*t->buf = '\0';			/* This may cause some data to be lost if */
		return (0);				/* chars are typed after @ & before processing */
	}

	/* if no newline is contained in input, return without proc'ing */
	for (i = begin; !ISNEWL(*(t->buf + i)); i++)
		if (!*(t->buf + i))
			return(0);

	/* input contains 1 or more newlines; process the stuff */
	for (i = 0, k = 0; *(t->buf + i);)
	{
		if (!ISNEWL(*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))))
			if(*(t->buf + i) == '\b')	 /* backspace */
				if (k)  /* more than one char ? */
				{
					i++;
		  		}
			else
	 			i++;  /* no or just one char.. Skip backsp */
		else

					/* KILLER CDR:  $$ problem here. */
				if (isascii(*(t->buf + i)) && isprint(*(t->buf + i)))
				{
					*(tmp + k) = *(t->buf + i);

					k++;
					i++;
		  		}
				else
	 				i++;
		else
		{
			*(tmp + k) = 0;
			if(*tmp == '!')
				strcpy(tmp,t->last_input);
			else
				strcpy(t->last_input,tmp);
			
			write_to_q(tmp, &t->input);

			if(t->snoop.snoop_by && t->snoop.snoop_by->desc != NULL && !IS_NPC (t->snoop.snoop_by) )
				{
				write_to_q("% ",&t->snoop.snoop_by->desc->output);
				write_to_q(tmp,&t->snoop.snoop_by->desc->output);
				write_to_q("\n\r",&t->snoop.snoop_by->desc->output);
			}

			if (flag)
			{
				snprintf (buffer, MAX_STRING_LENGTH,  
					"Line too long. Truncated to:\n\r%s\n\r", tmp);
				if (write_to_descriptor(t, buffer) < 0)
					return(-1);

				/* skip the rest of the line */
				for (; !ISNEWL(*(t->buf + i)); i++);
			}

			/* find end of entry */
			for (; ISNEWL(*(t->buf + i)); i++);

			/* squelch the entry from the buffer */
			for (squelch = 0;; squelch++)
				if ((*(t->buf + squelch) = 
					*(t->buf + i + squelch)) == '\0')
		  			break;
			k = 0;
			i = 0;
		}
	}

	return(1);
}

void close_sockets(int s)
{
	system_log ("Closing all sockets.", FALSE);

	while (descriptor_list)
		close_socket(descriptor_list);

	close(s);
}

void close_socket (DESCRIPTOR_DATA *d)
{
	DESCRIPTOR_DATA	*tmp;
	char			buf [100];

	if ( d->connected == -15 )	/*  Soft reboot sockets. */
		return;
		
	if ( d->character )		/* I don't know about this one. . . */
		d->character->desc = 0;

	close (d->descriptor);
	flush_queues (d);

	if ( d->descriptor == maxdesc )
		--maxdesc;

	if ( d->character && d->connected != CON_PLYNG ) {
		unload_pc (d->character);
		d->character = NULL;
	}
	else if ( d->character && d->connected == CON_PLYNG ) {

		if ( d->character->pc )
			d->character->pc->last_disconnect = time (0);

		save_char (d->character, TRUE);

	    /* KLUDGE:  If the player is disconnecting is staff, he will
               get a message about himself disconnecting.  However, he will
               be gone before that message gets to him, and the message
               will sit around in memory.  By saying he isn't connected, the
               message will not be sent.  (connected = -1)
            */

		if ( d->original ) {
			d->character = d->original;
			d->original = NULL;
		}

		snprintf (s_buf, MAX_STRING_LENGTH,  "%s has lost link.\n", d->character->tname);
		d->connected = -1;
		send_to_gods (s_buf);
		d->connected = CON_PLYNG;

		if ( d->snoop.snooping && d->snoop.snooping->desc ) {
			d->snoop.snooping->desc->snoop.snoop_by = 0;
			d->snoop.snooping = 0;
		}

		if ( d->snoop.snoop_by && d->snoop.snoop_by->desc ) {
			d->snoop.snoop_by->desc->snoop.snooping = 0;
			d->snoop.snoop_by = 0;
		}

		if ( d->character->pc )
			d->character->pc->owner = 0;

		if ( IS_MORTAL (d->character) && !IS_SET (d->character->flags, FLAG_GUEST) ) {
			act ("$n has lost link.", TRUE, d->character, 0, 0, TO_ROOM);

			snprintf (buf, MAX_STRING_LENGTH,  "Closing link to: %s.", GET_NAME (d->character));
			system_log (buf, FALSE);

			d->character->desc = 0;
		}
		else if ( IS_SET (d->character->flags, FLAG_GUEST) )
			do_quit (d->character, "", 0);
	}

	if ( d->account && d->account->name && *d->account->name ) {
		free_account (d->account);
		d->account = NULL;
	}

	if (next_to_process == d)
		next_to_process = next_to_process->next;   

	if (d == descriptor_list) { /* this is the head of the list */
		descriptor_list = descriptor_list->next;
	} else {
		for ( tmp = descriptor_list; tmp->next; tmp = tmp->next )
			if ( tmp->next == d ) {
				tmp->next = tmp->next->next;
				break;
			}
	}

	d->next = NULL;

	free_descriptor (d);

	socket_closed = TRUE;
}

void nonblock(int s)
{
/** The fcntl() function provides control of open file descriptors. **/
	if (fcntl(s, F_SETFL, FNDELAY) == -1)
	{
		perror("Noblock");
		exit(EXIT_FAILURE);
	}
}

void send_to_char (char *message, CHAR_DATA *ch)
{
	DESCRIPTOR_DATA		*d;

	if ( !message )
		return;

	if ( !ch )
		return;

	d = ch->desc;

	if ( !d && IS_NPC (ch) )
		return;

		/* Check to see if real PC owner is still online */

	if ( !d && ch->pc && ch->pc->owner )
		for ( d = descriptor_list; d; d = d->next )
			if ( d == ch->pc->owner )
				break;

	if ( !d )
		return;

	if ( d->character && IS_SET (d->character->act, PLR_QUIET) )
		return;

	write_to_q (message, &d->output);
	return;
}

void send_to_all_unf(char *messg)
{
	DESCRIPTOR_DATA *i;

	if (messg)
		for (i = descriptor_list; i; i = i->next)
			if (!i->connected)
				write_to_q(messg, &i->output);
}

void send_to_all(char *messg)
{
	DESCRIPTOR_DATA *i;
	char		*formatted;

	if ( !messg || !*messg )
		return;

	reformat_string (messg, &formatted);

	if (messg)
		for (i = descriptor_list; i; i = i->next)
			if (!i->connected)
				write_to_q(formatted, &i->output);

	mem_free (formatted);
}

void send_to_gods (char *message)
{
	DESCRIPTOR_DATA		*d = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			*formatted = NULL;

	if ( !message || !*message )
		return;

	snprintf (buf, MAX_STRING_LENGTH,  "\n#2[System Message]#0 %s", message);

	reformat_string (buf, &formatted);

	for ( d = descriptor_list; d; d = d->next ){
		if ( !d->connected &&
			 !IS_MORTAL (d->character) &&
			 !IS_SET (d->character->act, PLR_QUIET) ){
			write_to_q (formatted, &d->output);
		}
	}
	mem_free (formatted);
	return;
}

void send_to_imms (char *message)
{
	DESCRIPTOR_DATA		*d;
	char			buf [MAX_STRING_LENGTH];
	char			*formatted;

	if ( !message || !*message )
		return;

	snprintf (buf, MAX_STRING_LENGTH,  "\n%s", message);

	reformat_string (buf, &formatted);

	for ( d = descriptor_list; d; d = d->next )
		if ( !d->connected &&
			 !IS_MORTAL (d->character) &&
			 !IS_SET (d->character->act, PLR_QUIET) )
			write_to_q (formatted, &d->output);
	mem_free (formatted);
}

void send_outside (char *message)
{
	DESCRIPTOR_DATA		*d;
	char			*formatted;

	if ( !message || !*message )
		return;

	reformat_string (message, &formatted);

	for ( d = descriptor_list; d; d = d->next ) {

		if ( d->connected || !d->character || !d->character->room )
			continue;

		if ( OUTSIDE (d->character) &&
			 !IS_SET (d->character->act, PLR_QUIET) &&
			 AWAKE (d->character) )
			write_to_q (formatted, &d->output);
	}
	mem_free (formatted);
}

void send_outside_zone (char *message, int zone)
{
	DESCRIPTOR_DATA		*d;
	char			*formatted;

	if ( !message || !*message )
		return;

	reformat_string (message, &formatted);

	for ( d = descriptor_list; d; d = d->next ) {

		if ( d->connected || !d->character || !d->character->room )
			continue;

		if ( d->character->room->zone != zone )
			continue;

		if ( OUTSIDE (d->character) &&
			 !IS_SET (d->character->act, PLR_QUIET) &&
			 AWAKE (d->character) )
			write_to_q (formatted, &d->output);
	}

	mem_free (formatted);
}

void send_to_room (char *message, int room_num)
{
	CHAR_DATA	*tch = NULL;
	ROOM_DATA	*room = NULL;
	char		*formatted = NULL;

	if ( !message || !*message )
		return;

	if ( !(room = vtor (room_num)) )
		return;

	reformat_string (message, &formatted);

	for ( tch = room->people; tch; tch = tch->next_in_room )
		if ( tch->desc && !IS_SET (tch->act, PLR_QUIET) )
			write_to_q (formatted, &tch->desc->output);

	mem_free (formatted);
	return;
}

void send_to_room_unf (char *message, int room_num)
{
	CHAR_DATA	*tch;
	ROOM_DATA	*room;

	if ( !message || !*message )
		return;

	if ( !(room = vtor (room_num)) )
		return;

	for ( tch = room->people; tch; tch = tch->next_in_room )
		if ( tch->desc && !IS_SET (tch->act, PLR_QUIET) )
			write_to_q (message, &tch->desc->output);
			
	return;
}

/* higher-level communication */

void act (char *str, int hide_invisible, CHAR_DATA *ch,
		  OBJ_DATA *obj, void *vict_obj, int type)
{
	char		*strp = NULL;
	char		*point = NULL;
	char		*i = NULL;
	char		*p = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		immbuf1 [MAX_STRING_LENGTH] = {'\0'};
	char		immbuf2 [MAX_STRING_LENGTH] = {'\0'};
	int			chsex = 0;
	int			color = 0;
	int			do_cap = 0;
	int			dietyCap = 0;
	int			idolCap = 0;
	CHAR_DATA	*to = NULL;
	CHAR_DATA	*tch = NULL;

/** Todo - needs a check on parameters to deal with possible NULL values **/
	if ( !str || !*str ){
		return;
	}

	if ( !ch ) {
		return;
	}

	if ( IS_SET (ch->flags, FLAG_COMPETE) ){
		return;
	}

	if ( IS_SET (type, TO_VICT) ) {
		tch = (CHAR_DATA *) vict_obj;
		to = tch;
	}
	else if ( IS_SET (type, TO_CHAR) ){
		to = ch;
	}
	else if ( !ch->room ){
		return;
	}
	else{
		to = ch->room->people;
	}

	for (; to; to = to->next_in_room){
		if ( to->desc &&
			(to != ch || IS_SET (type, TO_CHAR)) &&
			(CAN_SEE (to, ch) || !hide_invisible) &&
			AWAKE (to) &&
			(!IS_SET (type, TO_IMMS) || (GET_TRUST (to) && !GET_TRUST (ch))) &&
			!(IS_SET (type, TO_NOTVICT) && to == (CHAR_DATA *) vict_obj) ) {

			for (strp = str, point = buf;;){
				if (*strp == '$'){
					switch (*(++strp)){

						/** n - name of actor**/
						case 'n':
							i = PERS(ch, to);
							if ( GET_TRUST (to) && is_hooded (ch) ) {
								strcpy (immbuf1, i);
								snprintf (immbuf1, MAX_STRING_LENGTH, "%s (%s)",
								i, ch->tname);
						
								i = immbuf1;
							}

							color = 5;
							break;

						/** N - name of target**/
						case 'N':
							tch = (CHAR_DATA *) vict_obj;
							i = PERS (tch, to);
							if ( GET_TRUST (to) && is_hooded (tch) ) {
								strcpy (immbuf2, i);
								snprintf (immbuf2, MAX_STRING_LENGTH, "%s (%s)",
								i,tch->tname);

								i = immbuf2;
							}

							color = 5;
							break;
						
						/** 3 - ????? **/
						case '3':
							i = PERS((CHAR_DATA *) obj, to);
							color = 5;
							break;

						/** m - him/her  actor**/
						case 'm': 
							if (is_hooded(ch)) {
								chsex = ch->sex;
								ch->sex = 0;
								i = HMHR(ch); 
								ch->sex = chsex;
							}
							else{
								i = HMHR(ch);
							}

							color = 0;
								break;

						/** M - him/her target **/
						case 'M': 
							tch = (CHAR_DATA *) vict_obj;
							if (is_hooded(tch)) {
								chsex = tch->sex;
								tch->sex = 0;
								i = HMHR(tch); 
								tch->sex = chsex;
							}
							else{
								i = HMHR(tch);
							}

							color = 0;
							break;

						/** s - his/hers actor **/
						case 's': 
							if (is_hooded(ch)) {
								chsex = ch->sex;
								ch->sex = 0;
								i = HSHR(ch); 
								ch->sex = chsex;
							}
							else{
								i = HSHR(ch);
							}
							
							color = 0;
							break;

						/** S - his/hrs target **/
						case 'S': 
							tch = (CHAR_DATA *) vict_obj;
							if (is_hooded(tch)) {
								chsex = tch->sex;
								tch->sex = 0;
								i = HSHR(tch); 
								tch->sex = chsex;
							}
							else{
								i = HSHR(tch);
							}

							color = 0;
							break;

						/** e - he/she actor **/
						case 'e': 
							if (is_hooded(ch)) {
								chsex = ch->sex;
								ch->sex = 0;
								i = HSSH(ch); 
								ch->sex = chsex;
							}
							else{
								i = HSSH(ch);
							}
							
							color = 0;
							break;

						/** E - he/she target */
						case 'E': 
							tch = (CHAR_DATA *) vict_obj;
							if (is_hooded(tch)) {
								chsex = tch->sex;
								tch->sex = 0;
								i = HSSH(tch); 
								tch->sex = chsex;
							}
							else{
								i = HSSH(tch);
							}
							
							color = 0;
							break;

						/** o/O - object in a container of actor/target **/ 
						case 'o':
							i = OBJN(obj, to);
							color = 2;
							break;

						case 'O':
							i = OBJN((OBJ_DATA *) vict_obj, to);
							color = 2;
							break;

						/** p/P - object seen by actor/target **/
						case 'p':
							i = OBJS(obj, to);
							color = 2;
							break;

						case 'P':
							i = OBJS((OBJ_DATA *) vict_obj, to);
							color = 2;
							break;

						/** a/A - affects on actor/target **/
						case 'a':
							i = SANA(obj);
							break;
							
						case 'A':
							i = SANA((OBJ_DATA *) vict_obj);
							break;

						/** T - object held by victim ?**/
						case 'T':
							i = (char *) vict_obj;
							break;

						/** F - name of object held by vict? **/
						case 'F':
							i = fname((char *) vict_obj);
							break;

						/** g/G diety of actor/target **/
						case 'g':
							i = deity_name [ch->deity];
							color = 4;
							/* Make sure name is capitalized */
							dietyCap = 1;
							break;

						case 'G':
							i = deity_name [((CHAR_DATA *) (vict_obj))->deity]; 
							color = 4; 
							/* Make sure name is capitalized */
							dietyCap = 1;
							break;
				
						/** $ ??? **/
						case '$':
							i = "$";
							break;

						default:
							break;
					}

					if ( i && point) {
						if ( point == buf || dietyCap || idolCap ){
							do_cap = 1;
						}
						else{
							do_cap = 2;
						}

						if ( color != 0 ) {
							*point++ = '#';
							*point++ = '0' + color;
						}

						if ( *i ) {
							*point = *(i++);

							if ( do_cap == 1 ){
								CAP (point);
							}
							else if ( do_cap == 2 ){
								LOW (point);
							}

							point++;
							do_cap = 0;
						}

						while ( ((*point) = (*(i++))) ){
							++point;
						}

						if ( color != 0 ) {
							*point++ = '#';
							*point++ = '0';
						}
					}

					++strp;
				} /* if (*strp == '$') */
				
				else if (!(*(point++) = *(strp++))){
					break;
				}
			}/* for (strp = str, point = buf;;) */

			*(--point) = '\r';
			*(++point) = '\n';
			*(++point) = '\0';
			
/******* this section does nothing ????
			if ( IS_SET (type, TO_ACT_COMBAT) &&
				 IS_SET (to->plr_flags, COMBAT_FILTER) &&
				 to->fighting != vict_obj &&
				 to->fighting != ch ) {
		
				if ( vict_obj && ((CHAR_DATA *)vict_obj)->fighting == to )
				;
				else if ( ch->fighting == to )
				;
				else 
				;
				}
*******************/

			if ( IS_SET (type, TO_ACT_FORMAT) ) {
				reformat_string (buf, &p);

				if ( *p != '\r' &&
					 *p != '\n' &&
					 IS_SET (type, TO_VICT) && to == vict_obj ){
					send_to_char ("\r\n", to);
				}
				
				send_to_char (p, to);
				mem_free (p);
			} 
			else{
				send_to_char (buf, to);
			}
		}/* if ( to->desc && */

		if ( IS_SET (type, TO_VICT) || IS_SET (type, TO_CHAR) )
			return;
	}/* for (; to; to = to->next_in_room) */

}

extern int     bytes_allocated;
extern int     first_free;
extern int     mud_memory;

void do_gstat (CHAR_DATA *ch, char *argument, int cmd)
{
  	char            buf [MAX_STRING_LENGTH] = {'\0'};
  	int             uc = 0;
	int				con = 0;
	int				days = 0;
	int				hours = 0;
	int				minutes = 0;
	int				crashdays = 0;
	int				crashminutes = 0;
	int				crashhours = 0;
	long			uptime = 0;
	long			since_crash = 0;
	char			*tmstr = NULL;
	DESCRIPTOR_DATA	*e = NULL;

 	for ( e = descriptor_list; e; e = e->next )
		if ( !e->connected || e->account )
			uc++;
    	else
			con++;

	
	tmstr = asctime (localtime (&starttime));
	if (strlen(tmstr) > 1)
		tmstr[strlen(tmstr)-1] = '\0';
	

	uptime = time (0) - starttime;
	since_crash = time (0) - crashtime;
	days = uptime / 86400;
	hours = (uptime / 3600) % 24;
	minutes = (uptime / 60) % 60;
	crashdays = since_crash / 86400;
	crashhours = (since_crash / 3600) % 24;
	crashminutes = (since_crash / 60) % 60;

	send_to_char("\n#6Current Game Statistics#0\n", ch);
	send_to_char("#6-----------------------#0\n", ch);

  	snprintf (buf, MAX_STRING_LENGTH,  "#2Connected Descriptors:          #0%d\n", uc);
	send_to_char(buf,ch);
    	snprintf (buf, MAX_STRING_LENGTH,  "#2Connections Since Last Boot:    #0%d\n", conns);
	send_to_char (buf, ch);
	snprintf (buf, MAX_STRING_LENGTH,  "#2Guest Logins Since Last Boot:   #0%d\n", guest_conns);
	send_to_char (buf, ch);
	snprintf (buf, MAX_STRING_LENGTH,  "#2New Accounts Since Last Boot:   #0%d\n", new_accounts);
	send_to_char (buf, ch);  
	snprintf (buf, MAX_STRING_LENGTH,  "#2Mother Descriptor:              #0%d\n", s);
	send_to_char (buf, ch);
	snprintf (buf, MAX_STRING_LENGTH,  "#2Descriptors Pending:            #0%d\n", con);
	send_to_char(buf,ch);
  	snprintf (buf, MAX_STRING_LENGTH,  "#2Free Descriptors:               #0%d\n", avail_descs-maxdesc);
	send_to_char(buf,ch);
 	snprintf (buf, MAX_STRING_LENGTH,  "#2Shell Process ID:               #0%d\n", getpid ());
	send_to_char(buf,ch);
	snprintf (buf, MAX_STRING_LENGTH,  "#2MySQL Database Host:		#0%s\n", mysql_get_host_info(database));
	send_to_char (buf, ch);
	snprintf (buf, MAX_STRING_LENGTH,  "#2MySQL Server Version:		#0%s\n", mysql_get_server_info (database));
	send_to_char (buf, ch);
	snprintf (buf, MAX_STRING_LENGTH,  "#2Running on Port:                #0%d\n", port);
	send_to_char(buf,ch);
	snprintf (buf, MAX_STRING_LENGTH,  "#2Last Reboot By:                 #0%s",((BOOT[0]) ? BOOT : "Startup Script\n"));
	send_to_char(buf,ch);
 	snprintf (buf, MAX_STRING_LENGTH,  "#2Last Reboot Time:               #0%s\n", tmstr);
	send_to_char(buf,ch);
    	snprintf (buf, MAX_STRING_LENGTH,  "#2Time Spent on Last Boot:        #0%ld seconds\n", starttime-bootstart);
	send_to_char(buf,ch);
    	snprintf (buf, MAX_STRING_LENGTH,  "#2Current Uptime:                 #0%d day%s %d hour%s %d minute%s\n", days,((days==1) ? "" : "s"),hours,((hours==1) ? "" : "s"),minutes,((minutes==1) ? "" : "s"));
	send_to_char(buf,ch);
	if ( crashtime ) {
		snprintf (buf, MAX_STRING_LENGTH,  "#2Last Crashed:                   #0%d day%s %d hour%s %d minute%s ago\n", crashdays,((crashdays==1) ? "" : "s"),crashhours,((crashhours==1) ? "" : "s"),crashminutes,((crashminutes==1) ? "" : "s"));
		send_to_char(buf,ch);
	}
	snprintf (buf, MAX_STRING_LENGTH,  "#2Next PC Coldload:               #0%d\n", next_pc_coldload_id);
	send_to_char (buf, ch);
	snprintf (buf, MAX_STRING_LENGTH,  "#2Next Mobile Coldload:           #0%d\n", next_mob_coldload_id);
	send_to_char (buf, ch);
	send_to_char("#6-----------------------#0\n", ch);
}

char *colnum_to_color (int colnum)
{
	switch (colnum) {
	case COLNUM_NONE:			return COL_NONE;
	case COLNUM_RED:			return COL_RED;
	case COLNUM_GREEN:			return COL_GREEN;
	case COLNUM_YELLOW:			return COL_YELLOW;
	case COLNUM_BLUE:			return COL_BLUE;
	case COLNUM_MAGENTA:		return COL_MAGENTA;
	case COLNUM_CYAN:			return COL_CYAN;
	case COLNUM_WHITE:			return COL_WHITE;
	case COLNUM_OFF:			return COL_OFF;
	case COLNUM_BOLD_RED:		return COL_BOLD_RED;
	case COLNUM_BOLD_GREEN:		return COL_BOLD_GREEN;
	case COLNUM_BOLD_YELLOW:	return COL_BOLD_YELLOW;
	case COLNUM_BOLD_BLUE:		return COL_BOLD_BLUE;
	case COLNUM_BOLD_MAGENTA:	return COL_BOLD_MAGENTA;
	case COLNUM_BOLD_CYAN:		return COL_BOLD_CYAN;
	case COLNUM_BOLD_WHITE:		return COL_BOLD_WHITE;
	default:					return "";
	}
}

char *colorize (char *source, char *target, struct descriptor_data *d)
{
	char	*retval = NULL;
	int		colnum = 0;
	char	*temp_arg = NULL;
	
	retval = target;

	*target = '\0';

	while ( *source != '\0' ) {

				/* $ is the escape character for color */

		if ( *source == '#' ) {

			source++;		/* Point to character after $ */

						/* if string has $$, convert it to just a $ */

		if ( *source == '#' )
			*target++ = *source++;

		else if ( (*source >= '0' && *source <= '9') ||
				  (toupper (*source) >= 'A' && toupper (*source) <= 'F') ) {

			if ( !isdigit (*source) )
				colnum = toupper (*source) - 'A' + 10;
			 else
				colnum = *source - '0';

			if ( (d->character && d->character->color) ||
			     (d && d->color) ) {
			     temp_arg = colnum_to_color (colnum);
	         	strcpy (target, temp_arg);
			 	target = &target [strlen (target)];
			}

	        source++;
	      }
						/* Didn't recognize string...no substitution */
		  else {
	         *target++ = '#';
	         *target++ = *source++;
	      }
	   } else
	      *target++ = *source++;
	}

	*target = '\0';
	return retval;
}

void sigusr1(int signo)
{
	system_log ("SIGUSR1 received: running cleanup_the_dead().", FALSE);
	cleanup_the_dead(0);
}

void signal_setup (void)
{
	(void)signal (SIGUSR2, shutdown_request);

	(void)signal (SIGUSR1, sigusr1);
	(void)signal (SIGPIPE, SIG_IGN);
	(void)signal (SIGALRM, logsig);
	(void)signal (SIGSEGV, sigsegv);
	(void)signal (SIGCHLD, sigchld);

}

int get_user_seconds (void)
{
	struct rusage rus;
	getrusage (RUSAGE_SELF, &rus);
	return rus.ru_utime.tv_sec;
}

void alarm_update ()
{
	last_checkpoint = get_user_seconds();
	if (abort_threshold == BOOT_DB_ABORT_THRESHOLD)
	{
		abort_threshold = RUNNING_ABORT_THRESHOLD;
	}
}

void reset_itimer ()
{
	struct itimerval itimer;
	itimer.it_interval.tv_usec = 0; /* miliseconds */
	itimer.it_interval.tv_sec  = ALARM_FREQUENCY;
	itimer.it_value.tv_usec = 0;
	itimer.it_value.tv_sec = ALARM_FREQUENCY;

	if (setitimer (ITIMER_VIRTUAL, &itimer, NULL) < 0)
	{
		perror ("reset_itimer:setitimer");
		abort();
	}
}

const char * szFrozenMessage = "Alarm_handler: Not checkpointed recently, aborting!\n";

void alarm_handler (int signo)
{
	int usage_now = get_user_seconds();

	if ( valgrind )
		return;

	if (!disable_timer_abort && (usage_now - last_checkpoint > abort_threshold) )
	{
		send_to_gods ("Infinite loop detected - attempting recovery via reboot...");
		system_log ("Loop detected - attempting copyover to recover.", TRUE);
		shutdown_request (SIGUSR2);
	}	
}

void init_alarm_handler()
{
	struct sigaction sa;
	
	sa.sa_handler = alarm_handler;
	sa.sa_flags = SA_RESTART; /* Restart interrupted system calls */
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGVTALRM, &sa, NULL) < 0)
	{
		perror ("init_alarm_handler:sigaction");
		abort();
	}
	last_checkpoint = get_user_seconds();
	reset_itimer(); /* start timer */
	
}

void checkpointing(int signo)
{
	system_log ("Checkpointing...", FALSE);

	if ( signo == SIGVTALRM )
		(void)signal (SIGVTALRM, checkpointing);	/* Guess we have to rearm */

	if ( !tics ) {
		system_log ("Checkpoint shutdown - tics not updated!", TRUE);
		sigsegv(SIGSEGV);
	} else
		tics = 0;
}

void shutdown_request(int signo)
{
	FILE 	*fp;

	system_log ("Received USR2 - reboot request.", FALSE);

	if ( port != PLAYER_PORT && port != TEST_PORT ) {
		system_log ("Not player port, ignoring...", FALSE);
		return;
	}

	if ( !finished_booting || (fp = fopen ("booting", "r")) ) {	/*  Rebooting already. */
		system_log ("Signal ignored - already in the middle of a reboot.", TRUE);
		return;
	}

        if ( !(fp = fopen (".reboot", "w")) ) {
		system_log ("Error opening reboot file - aborting!", TRUE);
		abort();
        }
        fprintf (fp, "System Reboot Signal\n");
        fclose (fp);

	prepare_copyover (NULL, 0);
}

void logsig(int signo)
{
	system_log ("Signal received. Ignoring.", TRUE);
}

void sigchld (int signo)
{
	pid_t pid;
	int stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0 );
		(void)signal (SIGCHLD, sigchld);

	return;
}

void sigsegv (int signo)
{
	FILE		*fp;
	char		*p;
	char		buf [MAX_STRING_LENGTH];

	system_log ("Game has crashed!", TRUE);

	if ( (fp = fopen("recovery", "r")) ) {
		system_log ("Crash loop detected. Shutting down and dumping core...", TRUE);
		abort();
	}

	fp = fopen ("last_crash", "w+");
        if ( fp ) {
                fprintf (fp, "%d\n", (int)time(0));
                fclose(fp);
        }

	fp = fopen(".reboot", "w");
	if ( !(fp) ) {
		system_log ("Error opening reboot file in sigsegv()!", TRUE);
		abort();
	}
	fprintf (fp, "Crash Recovery Script\n");
	fclose (fp);

	if ( last_descriptor != NULL && last_descriptor->connected == CON_PLYNG && last_descriptor->character != NULL ) {
		snprintf (buf, MAX_STRING_LENGTH,    "Something in the last command you entered, '#6%s#0', has crashed the "
				"MUD. Although it may have been due to a random memory corruption, please "
				"try to use this command as sparingly as possible after the recovery, just in case. A "
				"core dump has been generated to assist debugging, and a coder will look into this issue as "
				"soon as possible. Thank you for your patience.", last_command);
		reformat_string (buf, &p);

		write_to_descriptor (last_descriptor, "\n#6--- ATTENTION ---#0\n");
		write_to_descriptor (last_descriptor, p);
		write_to_descriptor (last_descriptor, "#6-----------------#0\n");

		if ( port != TEST_PORT ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s [%d]: %s\r\n", last_descriptor->character->tname, last_descriptor->character->in_room, last_command);
			add_message (last_descriptor->character->tname, 3, "Server", NULL, "Caused Server Crash", "", buf, 0);
			add_message ("Crashes", 2, last_descriptor->character->tname, NULL, "Recorded Server Crash", "", buf, 0);
		}

	}

	abort();
}

/* Add user input to the 'current' string (as defined by d->str) */
void add_string(DESCRIPTOR_DATA *d, char *str)
{
	char			*scan = NULL;
	int				terminator = 0;
	char			end_char = '\0';
	char			*p = NULL;
	CALL_PROC		*proc;
	
	if (d->max_str == STR_ONE_LINE){
		end_char = -1;
		terminator = 1;
	}
	else{
		end_char = '@';
	}
		/* Get rid of the \r's that may be included in the user input */

	/* determine if this is the terminal string, and truncate if so */

	for ( scan = str; *scan; scan++ ){
		if (*scan == end_char) {
			terminator = 1;
			*scan = '\0';
			break;
	   }
	}
	
	if ( !*d->str ) {
		if ( strlen (str) > d->max_str ) {
			send_to_char("String too long - Truncated.\n\r", d->character);
			*(str + d->max_str) = '\0';
			terminator = 1;
		}
		CREATE(*d->str, char, strlen(str) + 3);
		strcpy(*d->str, str);
	}
	else {
				
		if (  (strlen(str) + strlen(*d->str) ) > d->max_str ) {
			send_to_char("String too long. Last line skipped.\n\r",
			   d->character);
			terminator = 1;
		}
		else {
			p = (char *)alloc ((int)(strlen (*d->str) + strlen(str) + 3), 3);
			strcpy (p, *d->str);
			strcat (p, str);
			mem_free (*d->str);
			*d->str = p;
		}
	}

	if ( terminator ) {
		d->str = 0;

		if ( d->character )
			REMOVE_BIT (d->character->act, PLR_QUIET );

		if ( d->proc ) {
			proc = d->proc;
			d->proc = NULL;
			(*proc) (d);
		}

		else if ( d->connected == CON_CREATION )
			create_menu_options (d);
	}

	else
	   strcat (*d->str, "\n");
}


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char *one_word(char *argument, char *first_arg )
{
	int found, begin, look_at;

	found = begin = 0;

	do
	{
		for ( ;isspace(*(argument + begin)); begin++);

		if (*(argument+begin) == '\"') {  /* is it a quote */

			begin++;

			for (look_at=0; (*(argument+begin+look_at) >= ' ') && 
			    (*(argument+begin+look_at) != '\"') ; look_at++)
				*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

			if (*(argument+begin+look_at) == '\"')
				begin++;

		} else {

			for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)
				*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

		}

		*(first_arg + look_at) = '\0';
		begin += look_at;
	}
	while (fill_word(first_arg));

	return(argument+begin);
}

void page_string (DESCRIPTOR_DATA *d, char *str)
{
	if ( !d )
		return;

	if ( d->showstr_head && *d->showstr_head )
		mem_free (d->showstr_head);

	d->showstr_head = str_dup (str);
	d->showstr_point = d->showstr_head;

	show_string (d, "");
}

void show_string (DESCRIPTOR_DATA *d, char *input)
{
	char		buffer [MAX_STRING_LENGTH];
	char		buf [MAX_INPUT_LENGTH];
	char		*scan;
	char		*chk;
	int			lines = 0;

	(void)one_argument (input, buf);

	if ( *buf ) {

		if ( d->showstr_head ) {
			mem_free (d->showstr_head);
			d->showstr_head = NULL;
		}

		if ( d->header )
			mem_free (d->header);

		d->header = NULL;
		d->showstr_point = NULL;

		return;
	}

	/* show a chunk */

	if ( d->header ) {
		SEND_TO_Q (d->header, d);
		lines++;
	}

	for ( scan = buffer;; scan++, d->showstr_point++ ) {

		*scan = *d->showstr_point;

		if ( *scan == '\n' )
			lines++;

		else if ( !*scan || lines >= 22 ) {
			*scan = '\0';

			/* see if this is the end (or near the end) of the string */

			for (chk = d->showstr_point; isspace(*chk); chk++)
				;

			if ( !*chk ) {

				if ( d->showstr_head ) {
					mem_free (d->showstr_head);
					d->showstr_head = NULL;
				}

				d->showstr_point = NULL;

				if ( d->header )
					mem_free (d->header);

				d->header = NULL;
			}
			else SEND_TO_Q ("\r\n", d);

			SEND_TO_Q (buffer, d);

			return;
		}
	}
}
