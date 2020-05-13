/** 
*	\file roomprogs.c
*	Module provides interpter for room-based programs 
*
*	The purpose of this module is provide utilities and functionality for
*	scripted room-programs
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

void rxp(CHAR_DATA *ch, char *prg);
char *next_line(char *old);
void doit(CHAR_DATA *ch, char *func, char *arg);
void r_link(char *argument);
void r_unlink(char *argument);
void r_exit(char *argument);
void r_give(CHAR_DATA *ch,char *argument);
void r_put(char *argument);
void r_get(char *argument);
void r_take(CHAR_DATA *ch,char *argument);
void r_unexit(char *argument);
void r_atlook(CHAR_DATA *ch, char *argument);
void r_atecho(CHAR_DATA *ch, char *argument);
void r_loadmob(char *argument);
void r_exmob(char *argument);
void r_rftog(char *arg);
void r_force(CHAR_DATA *ch, char *argument);
void r_pain(CHAR_DATA *ch, char *argument) ;
void r_painmess(CHAR_DATA *victim, int dam);

bool ifin;

int r_name_is(char *str, char *namelist)
{
	char	*curname = NULL;
	char	*curstr = NULL;

    if ( !str )
		return 0;

	if ( !namelist )
		return 0;

	curname = namelist;
	while(0) {
		for (curstr = str;; curstr++, curname++){
			if ((!*curstr && !isalpha(*curname)) || !str_cmp (curstr,curname))
				return(1);

			if (!*curname)
				return(0);

			if (!*curstr || *curname == ' ')
				break;

			if (LOWER(*curstr) != LOWER(*curname))
				break;
		}

		/* skip to next name */

		for (; isalpha(*curname); curname++);
		if (!*curname)
			return(0);
		curname++;			/* first char of new name */
	}
	return(0);
}

void update_pos (CHAR_DATA *victim)
{
	return;
}


int r_program (CHAR_DATA *ch, char *argument)
{
	char	cmd [80];
	char	arg [80];
	struct room_prog *p = NULL;
	
	ifin = 0;
	

	if (ch == NULL)
		return(0);

	if ( !ch->room )
		return (0);

	if ( !ch->room->prg )
		return (0);

	half_chop(argument,cmd,arg);

	for ( p = ch->room->prg; p; p = p->next ) {

		if ( !p->prog || !*p->prog )
			continue;

		if ( r_name_is (cmd, p->command) ) {
			if ( !arg || name_is (arg, p->keys) ) {
				rxp (ch, p->prog);
				return (1);
			}
		}
	}

	return (0);
}

void rxp(CHAR_DATA *ch, char *prg)
{
	char 	func[512];
	char	*arg;
	int 	i;
	int		j;
	int		n;
	char 	*tprog;
	char	*line;
	char 	tmpf[4096];

	strcpy(tmpf, prg);
	tprog=tmpf;
	line=strtok(tprog,"\n");

	do{
		if (*line) {
			func[0] = '\0';
			arg = strpbrk (line, " ");
			
			if (line == NULL || arg == NULL)
				return;

			i = strlen (line);
			j = strlen (arg);
			strncat (func, line, i - j);

			for ( ; isspace(*arg); arg++ )
				; /* Get rid of whitespaces */
				
			n = strlen(arg);

			for ( ; isspace (*(arg + n)); n-- )
				; /* Get rid of large? whitespaces */
				
			
				*(arg + n) = '\0';
				doit (ch, func, arg);
		}
	
		line = strtok (NULL,"\n");
	} while ( line && *line );
	
	return;
}

/* change the %'s to $'s and remove the trailing \n */
void ref (char *str, char *return_string)
{
	*return_string = '\0';

	for ( ; *str; str++, return_string++ ) {
		if ( *str == '%' )
			*return_string = '$';
		else
			*return_string = *str;
	}

	*return_string = '\0';
	
	return;
}

/* Handles the if statement -- returns TRUE if the if is false */
/* Yeah I know, it's backwards.....I'll change it when I have time */

void reval(CHAR_DATA *ch,char *arg)
{
	int 	i = 0;
	int		dsiz = 0;
	int		dir = 0;
	int		tsiz = 0;
	long 	virt = 0;
	long	who = 0;
	char 	tmp[80];
	char	tmp2[80];
	char	*dbuf = NULL;
	char	rbuf[80];
	char	sarg[80];
	CHAR_DATA 	*tmp_ch = NULL;
	OBJ_DATA 	*obj = NULL;
	ROOM_DATA	*troom = NULL;

	*rbuf = '\0';
	strcpy(sarg, arg);
	
	while(*arg != '(')
		arg++;
	
	arg++;
	i = 0;
	
	while(*arg != ')') {
		tmp[i] = *arg++;
		i++;
	}
	
	tmp[i++] = '\0';
	
	tsiz = strlen(tmp);
	strcpy(tmp2, tmp);
	dbuf = strchr(tmp, ',');
	dsiz = strlen(dbuf);
	dbuf++;
	
	for( ; isspace(*dbuf); dbuf++)
		; /* strips whitespace */
		
	strncat(rbuf, tmp2, (tsiz-dsiz));

/* Check to see if a mob exists in a given room */
/* Usage: if mexist(mobvnum,roomvnum)           */

	if(!strncmp(sarg, "mexist", 6)) {
		virt = atol(dbuf);
		who = atol(rbuf);
		
		for(tmp_ch=vtor (virt)->people;
			tmp_ch;
			tmp_ch=tmp_ch->next_in_room) {
			if(IS_NPC(tmp_ch) && tmp_ch->mob->virtual == who) 
				break;
		}
		
		if(!tmp_ch) {
			ifin = 1;
			return;
		}
	}

/* Check to see if mob/player has object (also checks eq) */
/* Usage: if haso(mobvnum,objvnum)                        */
/* Only checks in current room. To denote player use -1   */

	if(!strncmp(sarg, "haso", 4)) {
		who = atol(rbuf);
		if(who == -1) {
			tmp_ch = ch;
		} 
		else {
			for(tmp_ch=vtor (ch->in_room)->people;
				tmp_ch;
				tmp_ch=tmp_ch->next_in_room) {
				if( tmp_ch->mob && tmp_ch->mob->virtual == who)
					break;
			}
			
			if(!tmp_ch) {
				ifin = 1;
				return;
			}
		}

		obj = get_obj_in_list_num(atol(dbuf), tmp_ch->right_hand);
		
		if ( !obj )
			obj = get_obj_in_list_num(atol(dbuf), tmp_ch->left_hand);
		
		if(!obj) {
			if(!(get_obj_in_equip_num(tmp_ch, atol(dbuf)))) {
				ifin=1;
				return;
			}
		}
	}

/* Checks to see if a link exist in a given room and direction */
/* Usage: if link(roomvnum, dir)                               */

	if(!strncmp(sarg, "link", 4)) {
		virt=atol(rbuf);
		
		switch(*dbuf) {
			case 'n': dir=0; break;
			case 'e': dir=1; break;
			case 's': dir=2; break;
			case 'w': dir=3; break;
			case 'u': dir=4; break;
			case 'd': dir=5; break;
			default:
				system_log("Unknown direction in reval::link", TRUE);
				ifin=1;
				return;
		}

		if ( !(troom = vtor (virt)) ) {
			system_log("ERROR: tar room not found in reval::link", TRUE);
			ifin = 1;
			return;
		}
			
		if ( !troom->dir_option [dir] ) {
			ifin = 1;
			return;
		}
	}
	
	return;
}
		
void doit(CHAR_DATA *ch, char *func, char *arg)
{
	int 	i = 0;
	char 	tmp [MAX_STRING_LENGTH] = {'\0'};
	
	for ( i = 0; (*rfuncs [i] != '\n'); i++ )
		if ( !strcmp (rfuncs [i], func) )
			break;

	switch (i) {
		case RP_ATECHO:
			if(!ifin)
				r_atecho(ch,arg);
			return;
        
        case RP_EXIT:
           	if(!ifin)
           		r_exit(arg);
           	return;
        
        case RP_LINK:
           	if(!ifin)
           		r_link(arg);
           	return;
		
		case RP_ATLOOK:
			if(!ifin)
				r_atlook(ch,arg);
			return;
		
		case RP_TRANS:
			if(!ifin) {
				if ( !vtor (strtol(arg, NULL, 10)) )
					return;
				
				if ( ch->mount ) {
					char_from_room (ch->mount);
					char_to_room (ch->mount, vtor(strtol(arg, NULL, 10))->virtual);
				}
				
				char_from_room (ch);
				char_to_room (ch, vtor (strtol(arg, NULL, 10))->virtual);
			}
			return;
		
		case RP_VSTR:
			if(!ifin) {
				ref (arg, tmp);
				act (tmp,FALSE,ch,0,0,TO_CHAR | TO_ACT_FORMAT);
			}
			return;
		
		case RP_OSTR:
			if(!ifin) {
				ref (arg, tmp);
				act (tmp,FALSE,ch,0,0,TO_ROOM | TO_ACT_FORMAT);
			}
			return;
		
		case RP_UNLINK:
			if(!ifin)
				r_unlink(arg);
			return;
		
		case RP_UNEXIT:
			if(!ifin)
				r_unexit(arg);
			return;
		
		case RP_GIVE:
			if(!ifin)
				r_give(ch,arg);
			return;
		
		case RP_TAKE:
			if(!ifin)
				r_take(ch,arg);
			return;
		
		case RP_PUT:
			if(!ifin)
				r_put(arg);
			return;
		
		case RP_GET:
			if(!ifin)
				r_get(arg);
			return;
		
		case RP_GETCASH:
		case RP_GIVECASH:
			return;
		
		case RP_LOADMOB:
			if(!ifin)
				r_loadmob(arg);
			return;
		
		case RP_EXMOB:
			if(!ifin)
				r_exmob(arg);
			return;
		
		case RP_FORCE:
			if(!ifin)
				r_force(ch,arg);
			return;
		
		case RP_IF:
			if(!ifin)
				reval(ch,arg);
			return;
		
		case RP_FI:
			ifin = 0;
			return;
		
		case RP_ELSE:
			if(ifin)
				ifin = 0;
			else 
				ifin = 1;
			return;
		
		case RP_RFTOG:
			if(!ifin) 
				r_rftog(arg);
			return;
		
		case RP_PAIN:
			if(!ifin)
				r_pain(ch,arg);
			return;
		
		default:
			system_log("ERROR: unknown command in program", TRUE);
			return;
	}
	return;
}

void do_rpadd(CHAR_DATA *ch, char *argument, int cmd)
{
	struct room_prog 	*t = NULL;
	struct room_prog	*old = NULL;
	struct room_prog	*tmp = NULL;

	CREATE(t, struct room_prog, 1);

	t->next = NULL;
	t->command = NULL;
	t->keys = NULL;
	t->prog = NULL;
	
	if (!vtor (ch->in_room)->prg){
		vtor (ch->in_room)->prg = t;
	} 
	else {
		old = vtor (ch->in_room)->prg;
		tmp = old;
		
		while(tmp) {
			old = tmp;
			tmp = tmp->next;
		}
		
		old->next=t;
	}
	
	send_to_char("New program initialized.\n\r",ch);
	
	return;
}
		
void do_rpdel(CHAR_DATA *ch, char *argument, int cmd)
{
	int 	i = 1;
	int		j = 0;
	struct room_prog 	*p = NULL;
	struct room_prog	*tmp = NULL;
	
	j = strtol(argument, NULL, 10);

	for	(tmp = vtor (ch->in_room)->prg;
		tmp;
		tmp=tmp->next) {
		
		if(i == j) {
			if(!p) 
				vtor (ch->in_room)->prg = tmp->next;
			else
				p->next = tmp->next;
			
			mem_free (tmp);
			send_to_char("Done.\n\r",ch);
			return;
		}
		
		p = tmp;
		i++;
	}
	
	send_to_char("No such program, can you count past ten with your shoes on?\n\r",ch);
	return;
}

void do_rpcmd(CHAR_DATA *ch, char *argument, int cmd)
{
	char 	arg1[80];
	char	arg2[80];
	int 	i = 0;
	int		j = 0;
	struct room_prog	*t = NULL;
	
	half_chop(argument,arg1, arg2);

	if(!isdigit(arg1[0])) {
		send_to_char("Specify a program number to edit.\r\n",ch);
		return;
	}
	
	if(!arg2[0]) {
		send_to_char("Specify command(s) to install.\n\r",ch);
		return;
	}
	
	i = strtol(arg1, NULL, 10);
	
	for (j=1,t=vtor (ch->in_room)->prg;
		t;
		j++,t=t->next) {
		
		if(i==j) {
			t->command = add_hash (arg2);
			send_to_char("Command installed.\n\r",ch);
			return;
		}
	}

	send_to_char("That program does not exist.\n\r",ch);
	return;
}

void do_rpkey(CHAR_DATA *ch, char *argument, int cmd)
{
	char 	arg1[80];
	char	arg2[80];
	int 	i = 0;
	int		j = 0;
	struct room_prog *t = NULL;
	
	half_chop(argument,arg1, arg2);

	if(!isdigit(arg1[0])) {
		send_to_char("Specify a program number to edit.\r\n",ch);
		return;
	}
	i = strtol(arg1, NULL, 10);
	for (j = 1, t = vtor (ch->in_room)->prg;
		t;
		j++, t = t->next) {
		
		if(i == j) {
			t->keys=add_hash (arg2);
			send_to_char("Keywords installed.\n\r",ch);
			return;
		}
	}
	send_to_char("That program does not exist.\n\r",ch);
	return;
}

void do_rpprg(CHAR_DATA *ch, char *argument, int cmd)
{
	char 	arg1[80];
	int 	i = 0;
	int		j = 0;
	struct room_prog *t = NULL;
	
	(void)one_argument(argument,arg1);

	if(!isdigit(arg1[0])) {
		send_to_char("Specify a program number to edit.\r\n",ch);
		return;
	}
	i = strtol(arg1, NULL, 10);
	for (j = 1,t = vtor (ch->in_room)->prg;
		t;
		j++,t = t->next) {
		
		if(i == j) {
            make_quiet(ch);
            send_to_char("Enter program now, Terminate entry with an '@'\n\r",ch);
            ch->desc->str = &t->prog;
            t->prog = 0;
            ch->desc->max_str = 2000;
			return;
		}
	}
	send_to_char("That program does not exist.\n\r",ch);
	return;
}

void do_rpapp(CHAR_DATA *ch, char *argument, int cmd)
{
	char 	arg1[80];
	int 	i = 0;
	int		j = 0;
	struct room_prog *t = NULL;
	
	(void)one_argument(argument,arg1);

	if(!isdigit(arg1[0])) {
		send_to_char("Specify a program number to edit.\r\n",ch);
		return;
	}
	i = strtol(arg1, NULL, 10);
	for(j = 1, t = vtor (ch->in_room)->prg;
	 	t;
	 	j++, t = t->next) {
		
		if(i == j) {
            make_quiet(ch);
            send_to_char("Append to program now, Terminate entry with an '@'\n\r",ch);
            ch->desc->str = &t->prog;
            ch->desc->max_str = 2000;
			return;
		}
	}
	send_to_char("That program does not exist.\n\r",ch);
	return;
}

void do_rpstat(CHAR_DATA *ch, char *argument, int cmd)
{
	char 	buf[MAX_STRING_LENGTH] = {'\0'};
	int 	i = 1;
	struct room_prog 	*r = NULL;


	if(!vtor (ch->in_room)->prg) {
		send_to_char("No program for this room.\n\r",ch);
		return;
	}

	for(r = vtor (ch->in_room)->prg;
		r;
		r = r->next,i++) {
		
		snprintf (ADDBUF, MAX_STRING_LENGTH, "Program Number[%d]\n\r", i);
		snprintf (ADDBUF, MAX_STRING_LENGTH, "Command words[%s]\n\r", r->command);
		snprintf (ADDBUF, MAX_STRING_LENGTH, "Argument Keywords[%s]\n\r", r->keys);
		snprintf (ADDBUF, MAX_STRING_LENGTH, "Program -\n\r%s\n\r", r->prog);
	}
	page_string(ch->desc,buf);
	return;
}

void r_link(char *argument)
{
	char 	buf1[80];
	char	buf2[80];
	char	buf3[80];
	int 	dir = 0;
    int 	location = 0;
    int		location2 = 0;
	ROOM_DATA	*source_room = NULL;
	ROOM_DATA	*target_room = NULL;
	
	arg_splitter(3, argument, buf1, buf2, buf3);

	if(!*buf1 || !*buf2 || !*buf3) {
		system_log("ERROR: Missing args in r_link", TRUE);
		return;
	}

	switch (*buf2) {
		case 'n': dir = 0; break;
		case 'e': dir = 1; break;
		case 's': dir = 2; break;
		case 'w': dir = 3; break;
		case 'u': dir = 4; break;
		case 'd': dir = 5; break;
		default: dir = -1; break;
	}

	if (dir == -1) {
		system_log("ERROR: Invalid direction in r_link", TRUE);
		return;
	}

	location = atol(buf1);

	location2 = atol(buf3);

	if ( !(target_room = vtor (location2)) ) {
		system_log("ERROR: tar room not found in r_link", TRUE);
		return;
	}

	if ( !(source_room = vtor (location)) ) {
		system_log("ERROR: cha room not found in r_link", TRUE);
		return;
	}
	
	if ( source_room->dir_option[dir] )
		vtor (source_room->dir_option[dir]->to_room)->dir_option[rev_dir[dir]] = 0;

	CREATE (source_room->dir_option[dir], struct room_direction_data, 1);
	
	source_room->dir_option[dir]->general_description = 0;
	source_room->dir_option[dir]->keyword = 0;
	source_room->dir_option[dir]->exit_info = 0;
	source_room->dir_option[dir]->key = -1;
	source_room->dir_option[dir]->to_room = target_room->virtual;

	CREATE(target_room->dir_option[rev_dir[dir]], struct room_direction_data, 1);
	
	target_room->dir_option[rev_dir[dir]]->general_description = 0;
	target_room->dir_option[rev_dir[dir]]->keyword = 0;
	target_room->dir_option[rev_dir[dir]]->exit_info = 0;
	target_room->dir_option[rev_dir[dir]]->key = -1;
	target_room->dir_option[rev_dir[dir]]->to_room = source_room->virtual;
}

void r_exit(char *argument)
{
	char	buf1[80];
	char	buf2[80];
	char	buf3[80];
	int 	dir = 0;
	int 	location = 0;
	int		location2 = 0;
	ROOM_DATA	*source_room = NULL;
	ROOM_DATA	*target_room = NULL;

	arg_splitter(3,argument, buf1, buf2, buf3);

	if(!*buf1 || !*buf2 || !*buf3) {
		system_log("ERROR: Missing args in r_link", TRUE);
		return;
	}

	switch (*buf2) {
		case 'n': dir = 0; break;
		case 'e': dir = 1; break;
		case 's': dir = 2; break;
		case 'w': dir = 3; break;
		case 'u': dir = 4; break;
		case 'd': dir = 5; break;
		default: dir = -1; break;
	}

	if (dir == -1) {
		system_log("ERROR: Invalid direction in r_link", TRUE);
		return;
	}
	location = strtol(buf1, NULL, 10);

	location2 = strtol(buf3, NULL, 10);

	if ( !(target_room = vtor (location2)) ) {
		system_log("ERROR: tar room not found in r_link", TRUE);
		return;
	}

	if ( !(source_room = vtor (location)) ) {
		system_log("ERROR: cha room not found in r_link", TRUE);
		return;
	}

	if ( !source_room->dir_option[dir])
		CREATE(source_room->dir_option[dir], struct room_direction_data, 1);

	source_room->dir_option[dir]->general_description = 0;
	source_room->dir_option[dir]->keyword = 0;
	source_room->dir_option[dir]->exit_info = 0;
	source_room->dir_option[dir]->key = -1;
	source_room->dir_option[dir]->to_room = target_room->virtual;

}

void r_atlook(CHAR_DATA *ch, char *argument)
{
	char 	loc_str[MAX_INPUT_LENGTH];
	int 	loc_nr = 0;
	int		original_loc = 0;
	CHAR_DATA	*target_mob = NULL;
	ROOM_DATA	*troom = NULL;
	
	strcpy(loc_str, argument);
	
	loc_nr = strtol(loc_str, NULL, 10);

	if ( !(troom = vtor (loc_nr)) ) {
		system_log("ERROR: Room not found in r_at", TRUE);
		return;
	}

	original_loc = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, loc_nr);
	do_look (ch, "", 0);
	
	/* check if the guy's still there */
	for (target_mob = troom->people;
		target_mob; 
		target_mob = target_mob->next_in_room){
		
		if (ch == target_mob){
			char_from_room(ch);
			char_to_room(ch, original_loc);
		}
	}
	return;
}

void r_atecho(CHAR_DATA *ch, char *argument)
{
	char	loc_str[MAX_INPUT_LENGTH] = {'\0'};
	char	buf[MAX_INPUT_LENGTH] = {'\0'};
	
	half_chop(argument, loc_str, buf);
	
	if ( !isdigit(*loc_str) ) {
		system_log("ERROR: atecho location not a digit", TRUE);
		return;
	}

	if ( !vtor (strtol(loc_str, NULL, 10)) ) {
		system_log("ERROR: Room not found in r_atecho", TRUE);
		return;
	}

	strcat(buf,"\n\r");
	send_to_room (buf, vtor (strtol(loc_str, NULL, 10))->virtual);
	
	return;
}

void r_unlink(char *argument)
{
	char	arg1[MAX_STRING_LENGTH] = {'\0'};
	char	arg2[MAX_STRING_LENGTH] = {'\0'};
	char 	buf[MAX_STRING_LENGTH] = {'\0'};
	int 	dir = 0;
   	int 	old_rnum = 0;
   	int		location = 0;
	ROOM_DATA	*troom = NULL;

	half_chop(argument,arg1,arg2);

	location = strtol(arg2, NULL, 10);

	switch (*arg1) {
		case 'n': dir = 0; break;
		case 'e': dir = 1; break;
		case 's': dir = 2; break;
		case 'w': dir = 3; break;
		case 'u': dir = 4; break;
		case 'd': dir = 5; break;
		default: dir = -1; break;
	}

	if (dir == -1) {
		system_log("ERROR: Invalid direction in r_unlink", TRUE);
		return;
	}

	if ( !(troom = vtor (location)) ) {
		system_log("ERROR: cha room not found in r_unlink", TRUE);
		return;
	}

	if ( troom->dir_option[dir]) {
		old_rnum = troom->dir_option[dir]->to_room;
	} 
	else {
		snprintf (buf, MAX_STRING_LENGTH,  "ERROR: Unknown exit in r_unlink [%d]: %s", troom->virtual, argument);
		system_log(buf, TRUE);
		return;
	}

	troom->dir_option[dir] = 0;
	vtor (old_rnum)->dir_option[rev_dir[dir]] = 0;
	
	return;
}

void r_unexit(char *argument)
{
	char	arg1[80] = {'\0'};
	char	arg2[80] = {'\0'};
	int 	dir = 0;
    int 	location = 0;
	ROOM_DATA	*troom = NULL;

	half_chop(argument, arg1, arg2);

	location = strtol(arg2, NULL, 10);

	switch (*arg1) {
		case 'n': dir = 0; break;
		case 'e': dir = 1; break;
		case 's': dir = 2; break;
		case 'w': dir = 3; break;
		case 'u': dir = 4; break;
		case 'd': dir = 5; break;
		default: dir = -1; break;
	}

	if (dir == -1) {
		system_log("ERROR: Invalid direction in r_unexit", TRUE);
		return;
	}

	if ( !(troom = vtor (location)) ) {
		system_log("ERROR: cha room not found in r_unexit", TRUE);
		return;
	}

	troom->dir_option[dir] = 0;
	
	return;
}

void r_give (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA	*obj = NULL;

	obj = load_object (strtol(argument, NULL, 10));

	if (obj) 
		obj_to_char (obj, ch);
	else
		system_log("ERROR: Object does not exist in r_give", TRUE);
	
	return;
}

void r_take (CHAR_DATA *ch,char *argument)
{
	OBJ_DATA 	*obj = NULL;

	obj = get_obj_in_list_num(atol(argument),ch->right_hand);
	if ( !obj )
		obj = get_obj_in_list_num(atol(argument),ch->left_hand);

	if(obj) {
		obj_from_char (&obj, 0);
		extract_obj (obj);
	}
	else
		system_log("ERROR: Object not found in r_take", TRUE);

	return;
}

void r_put(char *argument)
{
	char 	arg1[80] = {'\0'}; 
	char	arg2[80] = {'\0'};
	OBJ_DATA	*temp_arg = NULL;
	
	half_chop(argument, arg1, arg2);

	if ( !vtor (strtol(arg2, NULL, 10)) || !vtoo (strtol(arg1, NULL, 10)) )
		system_log("ERROR: Object does not exist in r_put", TRUE);
	else {
		temp_arg = load_object (strtol(arg1, NULL, 10));
		obj_to_room (temp_arg, vtor (strtol(arg2, NULL, 10))->virtual);
		}
	
	return;
}

void r_get(char *argument)
{
	char 	arg1[80] = {'\0'}; 
	char	arg2[80] = {'\0'};
	OBJ_DATA 	*obj = NULL;
	int 	virt = 0;

	half_chop(argument, arg1, arg2);

	virt = strtol(arg2, NULL, 10);

	if ( !vtor (virt) ) {
		system_log("ERROR: Object-room not found in r_get", TRUE);
		return;
	}

	obj = get_obj_in_list_num (strtol(arg1, NULL, 10), vtor (virt)->contents);
	if ( obj ) {
		obj_from_room (&obj, 0);
		
		extract_obj (obj);
	}
	else {
		system_log("ERROR: Object not found in r_get", TRUE);
	}
	
	return;
}


void r_loadmob(char *argument)
{
	char 	arg1[80] = {'\0'}; 
	char	arg2[80] = {'\0'};
	CHAR_DATA 	*temp_char = NULL;
	
	half_chop(argument, arg1, arg2);

	if ( !vtom (strtol(arg1, NULL, 10)) || !vtor (strtol(arg2, NULL, 10)) ) {
		system_log("ERROR: Mobile does not exist in r_loadmob", TRUE);
		return;
	}
	
	temp_char = load_mobile (strtol(arg1, NULL, 10));
	char_to_room (temp_char, vtor (strtol(arg2, NULL, 10))->virtual);
	
	return;
}

void r_exmob(char *argument)
{
	CHAR_DATA 	*mob = NULL;
	char 	arg1[80] = {'\0'}; 
	char	arg2[80] = {'\0'};
	int 	virt = 0;

	half_chop(argument, arg1, arg2);

	virt = atol(arg2);

	if ( !vtor (virt) ) {
		system_log("ERROR: Mobile-room does not exist in r_exmob", TRUE);
		return;
	}

	if ( !(mob = get_char_room (arg1, virt)) ) {
		system_log("ERROR: Mobile does not exist in r_exmob", TRUE);
		return;
	}

	extract_char (mob);
	return;
}

void r_rftog(char *arg)
{
	int 	flag = 0;
	char 	buf[80] = {'\0'}; 
	char	rbuf[80] = {'\0'};
	ROOM_DATA	*troom = NULL;

	half_chop(arg, buf, rbuf);

	if ( !(troom = vtor (strtol(rbuf, NULL, 10))) ) {
		system_log("ERROR: Unknown room in r_rftog.", TRUE);
		return;
	}

	flag = index_lookup (room_bits, buf);

	if(!IS_SET(troom->room_flags,(1<<flag)))
		SET_BIT(troom->room_flags,(1<<flag));
	else
		REMOVE_BIT(troom->room_flags,(1<<flag));
		
	return;
}

void r_force(CHAR_DATA *ch, char *argument)
{
	char 	arg1[80] = {'\0'}; 
	char	arg2[80] = {'\0'};
	char	arg3[256] = {'\0'};
	CHAR_DATA 	*tmp_ch = NULL;
	int 	room = 0;
	int		mob = 0;
	char 	buf[1024] = {'\0'};

	/* argument is (mob, room#, command)
	** force mob (in room#) to do soemthing
	*/
	arg_splitter(3, argument, arg1, arg2, arg3);
	
	mob = strtol(arg1, NULL, 10);

	if(mob == -1) {
		command_interpreter(ch, arg3);
		return;
	}

	room = strtol(arg2, NULL, 10);

	if ( !vtor (room) ) {
		system_log("ERROR: unknown room in r_force.", TRUE);
		return;
	}

	for(tmp_ch= vtor (room)->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) 
		if ( tmp_ch->mob && tmp_ch->mob->virtual == mob ) {
				snprintf (buf, MAX_STRING_LENGTH, "%s %s", arg3, GET_NAME(ch));
				command_interpreter(tmp_ch, buf);
		}

}

void r_pain(CHAR_DATA *ch, char *argument) 
{
	char 	room_num[80] = {'\0'}; 
	char	low_value[80] = {'\0'};
	char	high_value[80] = {'\0'};
	char 	vict[80] = {'\0'}; 
	int 	high = 0;
	int		low = 0;
	int		dam = 0;
	int 	room = 0;
	CHAR_DATA 	*victim = NULL;

	arg_splitter(4, argument, room_num, low_value, high_value, vict);
	
	room = strtol(room_num, NULL, 10);

	if ( !vtor (room) ) {
		system_log("ERROR: unknown room in r_pain.", TRUE);
		return;
	}
	
	low = strtol(low_value, NULL, 10);
	high = strtol(high_value, NULL, 10);
	
	if(!strncmp(vict, "all", 3)) {
		for(victim=vtor (room)->people; victim; victim=victim->next_in_room) {
			dam = number(low,high);
			if(dam>0)
				GET_HIT(victim) -= dam;
			
			update_pos(victim);
			r_painmess(victim, dam);
		}
	} 
	else {
		victim = ch;
		dam = number(low, high);
		if(dam>0) 
			GET_HIT(victim)-=dam;
		
		update_pos(victim);
		r_painmess(victim, dam);

	}
	return;
}

void r_painmess(CHAR_DATA *victim, int dam)
{
	int 	max_hit = 0;

	switch (GET_POS(victim)) {
		case POSITION_MORTALLYW:
			act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
			send_to_char("You are mortally wounded, and will die soon, if not aided.", victim);
			break;
	
		case POSITION_DEAD:
			act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
			send_to_char("You are dead!  Sorry...",  victim);
			break;

		default:  /* >= POSITION SLEEPING */

			max_hit = GET_MAX_HIT (victim);

			if (dam > (max_hit/5))
				send_to_char("That Really did HURT!", victim);

			if (GET_HIT(victim) < (max_hit/5)) {
				send_to_char("You wish that your wounds would stop BLEEDING that much!",victim);
				if (IS_NPC(victim))
					if (IS_SET(victim->act, ACT_WIMPY))
						do_flee(victim, "", 0);
			}
			break;
	}

	if (!IS_NPC(victim) && !(victim->desc)) {
		do_flee(victim, "", 0);
		if (!victim->fighting) {
			act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
			victim->was_in_room = victim->in_room;
			char_from_room(victim);
			char_to_room(victim, 0);
		}
	}

	if (!AWAKE(victim))
		if (victim->fighting)
			stop_fighting(victim);

	if (GET_POS(victim) == POSITION_DEAD) {
		if (IS_NPC(victim) || victim->desc)
			if (!IS_NPC(victim)) {
				snprintf (s_buf, MAX_STRING_LENGTH,  "%s killed by prog at %s", GET_NAME(victim), vtor (victim->in_room)->name);
				system_log(s_buf, TRUE);
			}
			
		die(victim);
	}
	return;
}
