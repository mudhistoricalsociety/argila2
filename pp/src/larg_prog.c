

/* constants */
#include "structs.h"
#include "protos.h"
#include "utils.h"


/** Cyle through all rooms looking for scripts **/
void larg_trigger_time_select(void)
{
	CHAR_DATA 	*tch = NULL;
	OBJ_DATA  	*tobj = NULL;
	ROOM_DATA	*troom = NULL;
	char 		buf[80] = {'\0'};
	int 		rnum = 0;
	
	/* go through the rooms */
	for (troom = full_room_list; troom; troom = troom->lnext)
	{		
		if (troom->people){
			if (troom->triggers){
				larg_room_trigger_time(troom, troom->people);
			}
			
			for ( tobj = troom->contents; tobj; tobj = tobj->lnext ) {
				if (tobj->triggers){
					larg_obj_trigger_time(tobj, troom->people);
				}
			}
			
			for ( tch = troom->people; tch; tch = tch->next_in_room ) {
				if (tch->triggers){
					larg_mob_trigger_time(tch, troom->people);
				}
			}
			
		}//end of people

	}//end of rooms
	
	return;
}
/**********************************************************************/
/*********************** TIMMER TRIGGERS ******************************/
/**********************************************************************/

void larg_room_trigger_time(ROOM_DATA *targ, CHAR_DATA *ch_targ)
{
	TRIGGER_DATA 	*trig = NULL;
	TRIGGER_DATA 	*next_t = NULL;
	int 				result = 0;
	
	//trig = targ->triggers->list;
	trig = targ->triggers;
	
	while(trig){
		next_t = trig->next;
		if (trig->type == TT_TIME) {
			result = larg_execute_script(trig->script,
										trig->func,
										TS_ROOM,
										trig->me,
										targ->virtual,
										ch_targ,
										NULL,
										NULL,
										NULL);
		}
		trig = next_t;
	}
	
	return;
}
/**********************************************************************/
void larg_obj_trigger_time(OBJ_DATA *targ, CHAR_DATA *tch)
{
	TRIGGER_DATA 	*trig = NULL;
	TRIGGER_DATA 	*next_t = NULL;
	int 				result = 0;
	OBJ_DATA		*tobj = NULL;
	
	//trig = targ->triggers->list;
	trig = targ->triggers;

	while(trig){
		next_t = trig->next;
		
		if (trig->type == TT_TIME) {
		
			tobj = get_obj_in_list_vis (tch, targ->name, tch->room->contents);
			if (tobj){
				result = larg_execute_script(trig->script,
										trig->func,
										TS_ROOM,
										trig->me,
										tobj->in_room,  //room
										tch,
										NULL,
										NULL,
										NULL);
			}
		}
		
		trig = next_t;
	}
	
	return;
}
/**********************************************************************/
void larg_mob_trigger_time(CHAR_DATA *targ, CHAR_DATA *tch)
{
	TRIGGER_DATA 	*trig = NULL;
	TRIGGER_DATA 	*next_t = NULL;
	int 			result = 0;
	CHAR_DATA		*tmob = NULL;
	
	//trig = targ->triggers->list;
	trig = targ->triggers;
	
	while(trig){
		next_t = trig->next;
		
		if (trig->type == TT_TIME) {
			result = larg_execute_script(trig->script,
										trig->func,
										TS_ROOM,
										trig->me,
										targ->in_room,  //room
										tch,
										NULL,
										NULL,
										NULL);
		}
		
		trig = next_t;
	}
	
	return;
}
/**********************************************************************/
/*************************** CREATE TRIGGERS **************************/
/**********************************************************************/

TRIGGER_DATA *larg_trigger_room_create(int type, char *script, int rvnum, 																		int source){

	TRIGGER_DATA 	*t;
	char 			*ptr;

	CREATE(t, TRIGGER_DATA, 1);
	t->type   = type;
	t->me     = rvnum;
	t->source = source;

	if ((ptr = strchr(script, ':')) != NULL){
		*ptr = 0;
		t->script = str_dup(script);
		t->func   = add_hash(str_dup(ptr + 1));
	}
	else{
		t->script = add_hash(str_dup(script));
	}

	return t;
}


/**********************************************************************/
TRIGGER_DATA *larg_trigger_obj_create(int type, char *script, int ovnum, 																		int source){

	TRIGGER_DATA 	*t;
	char 			*ptr;

	CREATE(t, TRIGGER_DATA, 1);
	t->type   = type;
	t->me     = ovnum;
	t->source = source;

	if ((ptr = strchr(script, ':')) != NULL)
	{
		*ptr = 0;
		t->script = str_dup(script);
		t->func   = add_hash(str_dup(ptr + 1));
	}
	else
	{
		t->script = add_hash(str_dup(script));
	}

	return t;
}

/**********************************************************************/
TRIGGER_DATA *larg_trigger_mob_create(int type, char *script, int mvnum, 																		int source){

	TRIGGER_DATA 	*t;
	char 			*ptr;

	CREATE(t, TRIGGER_DATA, 1);
	t->type   = type;
	t->me     = mvnum;
	t->source = source;

	if ((ptr = strchr(script, ':')) != NULL)
	{
		*ptr = 0;
		t->script = str_dup(script);
		t->func   = add_hash(str_dup(ptr + 1));
	}
	else
	{
		t->script = add_hash(str_dup(script));
	}

	return t;
}

/**********************************************************************/
/************************* TRIGGER UTILITIES **************************/
/**********************************************************************/
void larg_trigger_add(TRIGGER_DATA *t, TRIGGER_DATA **l)
{
	TRIGGER_DATA *add_trigger;

	t->next = *l;
	*l = t;

	for (add_trigger = *l; add_trigger; add_trigger = add_trigger->next)
	{
		add_trigger->list = t;
	}
	return;
}

/**********************************************************************/
void larg_trigger_remove(TRIGGER_DATA *t)
{
	TRIGGER_DATA	*temp;
	TRIGGER_DATA	*ind;
	

	for (ind = t->list; ind; ind = ind->next)
	{
		if (ind == t){
			ind->next = t->next;
			t->next = NULL;
			t->list = NULL;
		}
	}


	return;
	
}

/**********************************************************************/
/*
 * FUNCTION:  execute_script
 * ARGUMENTS:  char *script, char *func, int source, int me, int room,  OBJ_DATA *              obj,  CHAR_DATA *ch,  char *txt
  DESCRIPTION: Load a Lua script, and eventually call a function in it.
 * RETURNS:     -1 if something went wrong
 *               0 if script was executed
 */
int larg_execute_script(char *script, char *func, int source, int me, int room, CHAR_DATA *targ_ch,  OBJ_DATA *obj,  CHAR_DATA *ch,  char *txt)
{
	
	char buf[MAX_STRING_LENGTH] = {'\0'};
	int err = 0;
	char *mess = NULL;
	char *command = NULL;
	int mess_room_num = 0;
	int room_sect = 0;
	
	snprintf(buf, MAX_STRING_LENGTH, "%s/%s", SCRIPT_DIR, script);
	
	err = luaL_dofile(luaVM, buf);

	if (err == 1){
		send_to_gods("Error in execute_script");
		return (-1);
	}
		
	
	lua_getglobal(luaVM, func);
	
	lua_newtable(luaVM);        /* We will pass a table */

/* To put values into the table, we first push the index, then the
 * value, and then call lua_rawset() with the index of the table in the
 * stack. Let's see why it's -3: In Lua, the value -1 always refers to
 * the top of the stack. When you create the table with lua_newtable(),
 * the table gets pushed into the top of the stack. When you push the
 * index and then the cell value, the stack looks like:
 *
 * <- [stack bottom] -- table, index, value [top]
 *
 * So the -1 will refer to the cell value, thus -3 is used to refer to
 * the table itself. Note that lua_rawset() pops the two last elements
 * of the stack, so that after it has been called, the table is at the
 * top of the stack.
*/
	
	lua_pushnumber(luaVM, 1);   /* Push the table index */
	lua_pushnumber(luaVM, time_info.hour); /* Push the cell value */
	lua_rawset(luaVM, -3);      /* Stores the pair in the table*/

	lua_pushnumber(luaVM, 2);   /* Push the table index */
	lua_pushnumber(luaVM, time_info.day); /* Push the cell value */
	lua_rawset(luaVM, -3);      /* Stores the pair in the table*/
	
	room_sect = vtor(room)->sector_type;
	
	lua_pushnumber(luaVM, 3);   /* Push the table index */
	lua_pushnumber(luaVM, room_sect); /* Push the cell value */
	lua_rawset(luaVM, -3);      /* Stores the pair in the table*/

/*
 * A table must be terminated by a cell which is indexed by the literal
 * "n" and contains the total number of elements in the table.
 */
	lua_pushliteral(luaVM, "n");  /* Pushes the literal */
    lua_pushnumber(luaVM, 4);     /* Pushes the total number of cells */
    lua_rawset(luaVM, -3);        /* Stores the pair in the table */


	/* Send data to be used by Lua scripts */
	
	lua_pushnumber (luaVM, source);
	lua_pushnumber (luaVM, me);
	lua_pushnumber (luaVM, room);
	
/***************************************************************
 call script and get results back 
 sent table, source, me value and room number
 returns the command string to be used by 
 command_interpreter (CHAR_DATA *ch, char *argument)
 Could use a case statment here or call selection function
****************************************************************/


	lua_call(luaVM, 4, 2);
	
	command = (char *)lua_tostring(luaVM, -2);
	mess = (char *)lua_tostring(luaVM, -1);
	
	if (!strcmp(command, "echo")){
		send_to_room(mess, room);
	}
	
	else if (!strcmp(command, "r_loadmob")){
		r_loadmob(mess);
	}

	return (0);

	
}

/**********************************************************************/
/*************************** SETUP TRIGGERS **************************/
/**********************************************************************/

void larg_setup_room_triggers(FILE *fl, ROOM_DATA *room)
{
	int 			trig_type;
	char			*trig_script = NULL;
	TRIGGER_DATA	*trig = NULL;


	fscanf(fl, "%d", &trig_type);
	trig_script = fread_string(fl);

	trig = larg_trigger_room_create(trig_type, trig_script, room->virtual, TS_ROOM);

	larg_trigger_add(trig, &room->triggers);
	
	return;
}
/**********************************************************************/
void larg_setup_obj_triggers(FILE *fl, OBJ_DATA *obj)
{
	int 			trig_type;
	char			*trig_script = NULL;
	char			*temp_arg = NULL;
	TRIGGER_DATA	*trig = NULL;

	temp_arg = fread_word(fl);  //gets rid of the R
	trig_type = fread_number(fl);
	trig_script = fread_string(fl);
	
	
	trig = larg_trigger_obj_create(trig_type, trig_script, obj->virtual, TS_OBJECT);

	larg_trigger_add(trig, &obj->triggers);
	
	return;
}
/**********************************************************************/
void larg_setup_mob_triggers(FILE *fl, CHAR_DATA *tch)
{
	int 			trig_type;
	char			*trig_script = NULL;
	char			*temp_arg = NULL;
	TRIGGER_DATA	*trig = NULL;

	temp_arg = fread_word(fl);  //gets rid of the R
	trig_type = fread_number(fl);
	trig_script = fread_string(fl);
	
	
	trig = larg_trigger_mob_create(trig_type, trig_script, tch->mob->virtual, TS_MOBILE);

	larg_trigger_add(trig, &tch->triggers);
	
	return;
}

/**********************************************************************/
/*************************** ROOM UTILITIES **************************/
/**********************************************************************/
void do_rscript_add (CHAR_DATA *ch, char *argument, int cmd)
{
	ROOM_DATA 	*temp_room = NULL;
	
	temp_room = vtor (ch->in_room);
	larg_room_script_add (ch, argument, temp_room);
	return;
}
/**********************************************************************/
void do_rscript_del (CHAR_DATA *ch, char *argument, int cmd)
{
	ROOM_DATA 	*temp_room = NULL;
	
	temp_room = vtor (ch->in_room);
	larg_room_script_delete (ch, argument, temp_room);
	return;
}
/**********************************************************************/
void do_rscript_list (CHAR_DATA *ch, char *argument, int cmd)
{
	ROOM_DATA 	*temp_room = NULL;
		
	temp_room = vtor (ch->in_room);
	larg_room_script_list (ch, argument, temp_room);
}

/**********************************************************************/
void larg_room_script_list(CHAR_DATA *ch, char *argument, ROOM_DATA *room)
{
	char		buf	[MAX_STRING_LENGTH] = { '\0' };
	TRIGGER_DATA		*temp_trig = NULL;
	
	snprintf (buf, MAX_STRING_LENGTH,  "This room has the following scripts:\n\n Type     Script     Function\n");
		send_to_char (buf, ch);
			
		for (temp_trig = room->triggers; temp_trig; temp_trig = temp_trig->next){
			snprintf (buf, MAX_STRING_LENGTH,  "%d     %s     %s\n", temp_trig->type, temp_trig->script, temp_trig->func);
			send_to_char (buf, ch);
		}
	return;	
}
/**********************************************************************/
void larg_room_script_add (CHAR_DATA *ch, char *argument, ROOM_DATA *room)
{
	char		buf	[MAX_STRING_LENGTH] = { '\0' };
	TRIGGER_DATA		*temp_trig = NULL;
	int			sct_type = 0;
	char		*sct_script = NULL;
	char		*sct_func = NULL;


		if ( !*argument ) {
			send_to_char("Correct format is - rscriptadd Type Script_name  Function_name.\nTypes are 1-Timed, 2-Enter, 3-Leave, 4-Tell, 5-Ask, 6-Say, 7-Give\nType 1 is the only one implemented at this time\n", ch);
			return;
		}
		else {	
			sscanf (argument, "%d%s%s", &sct_type, &sct_script, &sct_func);

			
			temp_trig = (TRIGGER_DATA *)alloc ((int)sizeof (TRIGGER_DATA), 24);

			argument = one_argument (argument, buf);
			if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 7 ) {
				send_to_char ("Expected type 1..7\n", ch);
				return;
			}
			temp_trig->type = strtol(buf, NULL, 10);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of script\n", ch);
				return;
			}
			temp_trig->script = add_hash(buf);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of function\n", ch);
				return;
			}
			temp_trig->func = add_hash(buf);
			

			temp_trig->source = 0; //0-room, 1-obj, 2-mobile
			temp_trig->me = ch->in_room;
			
			larg_trigger_add(temp_trig, &room->triggers);
		}
		

	return;
}
/**********************************************************************/
void larg_room_script_delete (CHAR_DATA *ch, char *argument, ROOM_DATA *room)
{
	char		buf	[MAX_STRING_LENGTH] = { '\0' };
	TRIGGER_DATA		*head_trig = NULL;
	TRIGGER_DATA		*current_trig = NULL;
	TRIGGER_DATA		*temp_trig = NULL;

	int			trig_type = 0;
	int			trig_source = 0;
	int			trig_me = 0;
	char		*trig_script = NULL;
	char		*trig_func = NULL;


		if ( !*argument ) {
			send_to_char("Correct format is - rscriptdel Type Script_name  Function_name.\nTypes are 1-Timed, 2-Enter, 3-Leave, 4-Tell, 5-Ask, 6-Say, 7-Give\nType 1 is the only one implemented at this time\n", ch);
			return;
		}
		else {	
						
			argument = one_argument (argument, buf);
			if ( !isdigit (*buf) || strtol(buf, NULL, 10) > 7 ) {
				send_to_char ("Expected type 1..7\n", ch);
				return;
			}
			trig_type = strtol(buf, NULL, 10);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of script\n", ch);
				return;
			}
			trig_script = add_hash(buf);
			
			argument = one_argument (argument, buf);
			if ( !(*buf)) {
				send_to_char ("Expected Name of function\n", ch);
				return;
			}
			trig_func = add_hash(buf);
			
			trig_source = 0; //0-room, 1-obj, 2-mobile
			trig_me = ch->in_room;
		}

/** delete the head node **/
		if (room->triggers &&
			room->triggers->type == trig_type &&
			room->triggers->source == trig_source &&
			!(strcmp(room->triggers->script,trig_script)) &&
			!(strcmp(room->triggers->func, trig_func)) &&
			room->triggers->me == trig_me){
				temp_trig = room->triggers;
				room->triggers = temp_trig->next;
		}

/** delete other nodes **/
		else {
			for (current_trig = room->triggers; current_trig; current_trig = current_trig->next){
				if (!(current_trig->next == NULL) &&
					current_trig->next->type == trig_type &&
					current_trig->next->source == trig_source &&
					!(strcmp(current_trig->next->script,trig_script)) &&
					!(strcmp(current_trig->next->func, trig_func)) &&
					current_trig->next->me == trig_me){
										
						temp_trig = current_trig->next;
						current_trig->next = temp_trig->next;
				}
			}
		}
		
		

	return;
}

/**********************************************************************/
/*************************** MOB UTILITIES ***************************/
/** See mset for add, del, and list utilities                      ***/
/*********************************************************************/
void larg_mo_script_delete (TRIGGER_DATA *targ, TRIGGER_DATA *list_scrpt)
{
	char		buf	[MAX_STRING_LENGTH] = { '\0' };
	TRIGGER_DATA		*head_trig = NULL;
	TRIGGER_DATA		*current_trig = NULL;
	TRIGGER_DATA		*temp_trig = NULL;
		
/** delete the head node **/
		if (list_scrpt &&
			list_scrpt->type == targ->type &&
			list_scrpt->source == targ->source &&
			!(strcmp(list_scrpt->script, targ->script)) &&
			!(strcmp(list_scrpt->func, targ->func)) &&
			list_scrpt->me == targ->me){
				temp_trig = list_scrpt;
				list_scrpt = temp_trig->next;
		}

/** delete other nodes **/
		else {
			for (current_trig = list_scrpt; current_trig; current_trig = current_trig->next){
				if (!(current_trig->next == NULL) &&
					current_trig->next->type == targ->type &&
					current_trig->next->source == targ->source &&
					!(strcmp(current_trig->next->script, targ->script)) &&
					!(strcmp(current_trig->next->func, targ->func)) &&
					current_trig->next->me == targ->me){
										
						temp_trig = current_trig->next;
						current_trig->next = temp_trig->next;
				}
			}
		}
	return;
}
/**********************************************************************/
/*************************** OBJ UTILITIES ***************************/
/** See oset for add, del, and list utilities                      ***/
/*********************************************************************/

