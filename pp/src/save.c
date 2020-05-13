/** 
*	\file save.c
*	Provides services for saving and retrieving game data 
*
*	The purpose of this module is provide utilities and functionality for
*	saveing, writing, reading and loading of game date, including rooms, mobs,
*	objects, and players. Some statistics are also calculated here.
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

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "protos.h"
#include "decl.h"


#define POSSESS_UNDEF   0
#define	POSSESS_CARRY	1
#define POSSESS_WEAR	2
#define POSSESS_ROOM	3

static int		last_wc = POSSESS_UNDEF;
static int		last_eqloc;
static int		last_eqlev;
static int		file_eof_found = 0;
static int		save_stop_obj_processing = 0;

int				load_char_objects;

extern int fread_number (FILE *fp);
extern char *fread_word (FILE *fp);

void write_char_data ( CHAR_DATA *ch, FILE *fp );
void write_obj_data ( OBJ_DATA *obj,
		      		char *wc,
		      		int pos,
		      		int objstack,
		      		FILE *fp );
int load_player (char *name, CHAR_DATA *ch, int load_type);
void read_obj_list (int cur_wc,
                    int cur_eqloc,
                    int cur_eqlev,
				    CHAR_DATA *ch,
					OBJ_DATA *holder,
					FILE *fp,
					int room_num);
OBJ_DATA *fread_obj (FILE *fp);
void read_aliases (CHAR_DATA *ch, FILE *fp);

char *unspace (char *s)
{
	char	*result;
	char	*orig;

	if ( !s ) {
		((int *) -1) [0] = 1;
	}

	if ( *s != ' ' )
		return s;

	orig = s;

	result = malloc (strlen (s) + 1);

    while (s && *s == ' ')
       s++;

	strcpy (result, s);

	mem_free (orig);

    return result;
}

void fwrite_a_obj (OBJ_DATA *obj, FILE *fp)
{
	int				affect_count = 0;
	int				modifiers = 1;
	AFFECTED_TYPE	*af = NULL;
	WOUND_DATA		*wound;
	OBJECT_DAMAGE		*damage;
	LODGED_OBJECT_INFO	*lodged;
	OBJ_DATA		*proto = NULL;
	TRIGGER_DATA	*trig = NULL;
	proto = vtoo (obj->virtual);

	if ( !proto )
		return;

	for ( af = obj->xaffected; af; af = af->next ) {

		if ( af->a.spell.location ) {
			affect_count++;
			continue;
		}
	}
	
	if ( obj->omote_str )
		modifiers++;

	if ( obj->loaded )
		modifiers += 1;

	for ( wound = obj->wounds; wound; wound = wound->next ) {
		modifiers++;
	}

	for ( damage = obj->damage; damage; damage = damage->next ) {
		modifiers++;
	}

	for ( lodged = obj->lodged; lodged; lodged = lodged->next ) {
		modifiers++;
	}

	if ( obj->size )
		modifiers++;

	if ( obj->count > 1 )
		modifiers++;

	if ( obj->obj_flags.extra_flags )
		modifiers++;

	if ( obj->obj_flags.type_flag == ITEM_DRINKCON )
		modifiers++;

	if ( obj->obj_timer )		/* obj_flags.timer */
		modifiers++;

	if (obj->clock && (obj->morphto >= 0) && obj->morphTime)
		modifiers +=3;

	if ( obj->book_title && obj->title_skill )
		modifiers += 4;

	if ( obj->var_color && str_cmp (obj->var_color, "none") )
		modifiers++;

	if ( obj->item_wear )
		modifiers++;

	if ( obj->obj_flags.type_flag != ITEM_DRINKCON ) {

		if(proto->name && obj->name && *obj->name)
		  if ( strcmp (obj->name, proto->name) )
			modifiers++;

		if(proto->short_description && obj->short_description && *obj->short_description)
		  if ( strcmp (obj->short_description, proto->short_description) )
			modifiers++;

		if(proto->description && obj->description && *obj->description)
		  if ( strcmp (obj->description, proto->description) )
			modifiers++;

		if(proto->full_description && obj->full_description && *obj->full_description)
		  if ( strcmp (obj->full_description, proto->full_description) )
			modifiers++;
		
	}

	if ( IS_SET(proto->obj_flags.extra_flags, ITEM_MASK) && proto->obj_flags.type_flag == ITEM_ARMOR && obj->desc_keys && strlen(obj->desc_keys) > 1 ) {
		if ( !proto->desc_keys && obj->desc_keys )
			modifiers++;
		else if ( strcmp (obj->desc_keys, proto->desc_keys) )
			modifiers++;
	}

	if ( IS_SET(proto->obj_flags.extra_flags, ITEM_MASK) && proto->obj_flags.type_flag == ITEM_WORN && obj->desc_keys && strlen(obj->desc_keys) > 1 ) {
		if ( strcmp (obj->desc_keys, proto->desc_keys) )
			modifiers++;
	}

	if ( obj->coldload_id )
		modifiers++;

    	fprintf (fp, "Id       %d %d %d\n", obj->virtual, affect_count, modifiers);

	if ( obj->coldload_id )
		fprintf (fp, "coldload       %d\n", obj->coldload_id);

	if ( obj->obj_flags.type_flag != ITEM_DRINKCON ) {

		if(proto->name && obj->name && *obj->name)
		  if ( strcmp (obj->name, proto->name) )
			fprintf (fp, "name %s~\n", obj->name);

		if(proto->short_description && obj->short_description && *obj->short_description)
		  if ( strcmp (obj->short_description, proto->short_description) )
			fprintf (fp, "short %s~\n", obj->short_description);

		if(proto->description && obj->description && *obj->description)
		  if ( strcmp (obj->description, proto->description) )
			fprintf (fp, "long %s~\n", obj->description);

		if(proto->full_description && obj->full_description && *obj->full_description )
		  if ( strcmp (obj->full_description, proto->full_description) )
			fprintf (fp, "full %s~\n", obj->full_description);

		if ( IS_SET(proto->obj_flags.extra_flags, ITEM_MASK) && (proto->obj_flags.type_flag == ITEM_WORN || proto->obj_flags.type_flag == ITEM_ARMOR) && obj->desc_keys && strlen(obj->desc_keys) > 1 )
			if ( strcmp (obj->desc_keys, proto->desc_keys) )
				fprintf (fp, "desc_keys %s~\n", obj->desc_keys);
	}

	if ( obj->book_title && obj->title_skill ) {
		fprintf (fp, "book_title     %s~\n", obj->book_title);
		fprintf (fp, "title_skill    %d\n", obj->title_skill);
		fprintf (fp, "title_language %d\n", obj->title_language);
		fprintf (fp, "title_script   %d\n", obj->title_script);
	}

	if ( obj->loaded )
		fprintf (fp, "loaded %d\n", obj->loaded->virtual);

	if ( obj->obj_flags.extra_flags )
		fprintf (fp, "extraflags %d\n", obj->obj_flags.extra_flags);

	if ( obj->item_wear )
		fprintf (fp, "itemwear %d\n", obj->item_wear);

	fprintf (fp, "values %d %d %d %d %d %d\n", obj->o.od.value [0],
	obj->o.od.value [1], obj->o.od.value [2], obj->o.od.value [3],
	obj->o.od.value [4], obj->o.od.value [5]);

	if ( obj->obj_flags.type_flag == ITEM_DRINKCON )
		fprintf (fp, "weight   %d\n", obj->obj_flags.weight);

	if ( obj->size )
		fprintf (fp, "size     %d\n", obj->size);

	if ( obj->count > 1 )
		fprintf (fp, "count    %d\n", obj->count);

	if ( obj->obj_timer )
		fprintf (fp, "timer    %d\n", obj->obj_timer);

	if(obj->clock && (obj->morphto >= 0) && obj->morphTime) {
		fprintf(fp, "clock	%d\n", obj->clock);
		fprintf(fp, "morphto	%d\n", obj->morphto);
		fprintf(fp, "morphTime	%d\n", obj->morphTime);
	}

	if ( obj->var_color && str_cmp (obj->var_color, "none") )
		fprintf (fp, "VarColor  %s~\n", obj->var_color);

	if ( obj->omote_str ) {
		fprintf(fp, "OmoteStr   %s~\n", obj->omote_str);
	}

	if ( obj->wounds ) {
		for ( wound = obj->wounds; wound; wound = wound->next )
                	fprintf (fp, "Wound      %s %s %s %s %d %d %d %d %d %d %d\n",
                                     wound->location, wound->type, wound->severity, wound->name,
                                     wound->damage, wound->bleeding, wound->poison,
                                     wound->infection, wound->healerskill, wound->lasthealed,
                                     wound->lastbled);

	}

	if ( obj->damage ) {
		for ( damage = obj->damage; damage; damage = damage->next ) {
			fprintf (fp, "Damage     %s %s %s %d\n", damage->type, damage->name, damage->severity, damage->damage);
		}
	}

	if ( obj->lodged ) {
		for ( lodged = obj->lodged; lodged; lodged = lodged->next )
			fprintf (fp, "Lodged     %s %d\n", lodged->location, lodged->vnum);
	}

	for ( af = obj->xaffected; af; af = af->next ) {
		if ( af->a.spell.location ) {
			fprintf (fp, "Afflocmod %d %d %d %d %d %d %d\n",
				af->a.spell.location,
				af->a.spell.modifier,
				af->a.spell.duration,
				af->a.spell.bitvector,
				af->a.spell.t,
				af->a.spell.sn,
				af->type);
			continue;
		}
	}
/** Lua functions **/
	for ( trig = obj->triggers; trig; trig = trig->next ){
		if (obj->triggers){
			fprintf (fp, "Prog\n%d\n%s:%s\n",
						trig->type,
						trig->script,
						trig->func);
		}
	}
/** end lua functions **/
	return;
}

void write_obj_data (OBJ_DATA *obj, char *wc, int pos, int objstack, FILE *fp)
{
    if ( !obj )
       return;

	if ( !vtoo (obj->virtual) )
		return;

	if ( obj->virtual == LEANTO_OBJ_VNUM )		/* Leantos. */
		return;

    fprintf (fp, "%s %d %d\n", wc, pos, objstack);

	/* Save object */

    fwrite_a_obj (obj, fp);

        /* Save everything this object contains */

    if ( obj->contains )
       write_obj_data (obj->contains, wc, pos, objstack + 1, fp);

	/* Save the next object in the list */

    if ( (obj->location == -1 && obj->next_content && obj != obj->next_content ) )
       write_obj_data (obj->next_content, wc, pos, objstack, fp);

    if ( obj->next_content && obj->next_content == obj )
	obj->next_content = NULL;

	return;
}

OBJ_DATA *fread_obj (FILE *fp)
{
	int			obj_vnum = 0;
	int			affect_count = 0;
	int			modifiers = 0;
	int			i, page = 0;
	int			old_money = 0;
	char			*p = NULL;
	OBJ_DATA		*obj = NULL;
	OBJ_DATA		*tobj = NULL;
	WRITING_DATA	*writing = NULL;
	AFFECTED_TYPE	*af = NULL;
	AFFECTED_TYPE	*taf = NULL;
	WOUND_DATA		*wound = NULL;
	OBJECT_DAMAGE		*damage = NULL;
	LODGED_OBJECT_INFO	*lodged = NULL;
	TRIGGER_DATA		*trig = NULL;
	char			*temp_arg = NULL;

	if ( strcmp (p = fread_word (fp), "Id") ) {
		abort();
	}

	obj_vnum = fread_number (fp);	
	affect_count = fread_number (fp);
    
	if ( !(obj = load_object (obj_vnum)) ) {
		if ( !(obj = load_object (666)) ) {
			if ( !obj ) {
				return NULL;
			}
		}
	}

		/* Remove prototype affects */

	while ( (af = obj->xaffected) ) {
		obj->xaffected = af->next;
		mem_free (af);
	}

	fscanf (fp, "%d", &modifiers);

	for ( ; modifiers; modifiers-- ) {

		p = fread_word (fp);

		if ( !strcmp (p, "name") ) {
			fgetc (fp);
			obj->name = fread_string (fp);
		} 
		else if ( !strcmp (p, "short") ) {
			fgetc (fp);
			obj->short_description = fread_string (fp);
		} 
		else if ( !strcmp (p, "loaded") ) {
			obj->loaded = vtoo(fread_number(fp));
		} 
		else if ( !strcmp (p, "coldload") ) {
			obj->coldload_id = fread_number(fp);
		} 
		else if ( !strcmp (p, "extraflags") ) {
			obj->obj_flags.extra_flags = fread_number(fp);
		} 
		else if ( !strcmp (p, "itemwear") ) {
			obj->item_wear = fread_number(fp);
		} 
		else if ( !strcmp (p, "long") ) {
			fgetc (fp);
			obj->description = fread_string (fp);
		} 
		else if ( !strcmp (p, "full") ) {
			fgetc (fp);
			obj->full_description = fread_string (fp);
		} 
		else if ( !strcmp (p, "values") ) {
			obj->o.od.value [0] = fread_number (fp);
			obj->o.od.value [1] = fread_number (fp);
			obj->o.od.value [2] = fread_number (fp);
			obj->o.od.value [3] = fread_number (fp);
			obj->o.od.value [4] = fread_number (fp);
			obj->o.od.value [5] = fread_number (fp);
			if ( GET_ITEM_TYPE (obj) == ITEM_BOOK || GET_ITEM_TYPE (obj) == ITEM_PARCHMENT )
				writing = obj->writing;
		}
		else if ( !strcmp (p, "weight") )
			obj->obj_flags.weight = fread_number (fp);
		
		else if ( !strcmp (p, "size") )
			obj->size = fread_number (fp);
		
		else if ( !strcmp (p, "count") )
			obj->count = fread_number (fp);
		
		else if ( !str_cmp (p, "timer") )
			obj->obj_timer = fread_number(fp);
		
		else if ( !strcmp (p, "clock") )
			obj->clock = fread_number (fp);
		
		else if ( !strcmp (p, "morphTime") )
			obj->morphTime = fread_number (fp);
		
		else if ( !strcmp (p, "morphto") )
			obj->morphto = fread_number (fp);
		
		else if ( !strcmp (p, "VarColor") ){
			temp_arg = fread_string(fp);
			obj->var_color = unspace(temp_arg);
		}
		
		else if ( !strcmp (p, "book_title") ){
			temp_arg = fread_string(fp);
			obj->book_title = unspace(temp_arg);
		}
		
		else if ( !str_cmp (p, "title_skill") )
			obj->title_skill = fread_number(fp);
		
		else if ( !str_cmp (p, "title_script") )
			obj->title_script = fread_number(fp);
		
		else if ( !str_cmp (p, "title_language") )
			obj->title_language = fread_number(fp);
		
		else if ( !strcmp (p, "page:") ) {
			if ( GET_ITEM_TYPE (obj) == ITEM_BOOK )
				obj->o.od.value[1] = 0;
			else 
				obj->o.od.value[0] = 0;
			writing = obj->writing;
			page = fread_number (fp);
			for ( i = 1; i; i++ ) {
				if ( i == page )
					break;
				else  writing = writing->next_page;
			}
		}
		
		else if ( !strcmp (p, "message:") ) {
			if ( GET_ITEM_TYPE (obj) == ITEM_BOOK )
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			
			fgetc (fp);
			writing->message = fread_string(fp);
		}
		
		else if ( !strcmp (p, "author:") ) {
			if ( GET_ITEM_TYPE (obj) == ITEM_BOOK )
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			
			fgetc (fp);
			writing->author = fread_string(fp);
		}
		
		else if ( !strcmp (p, "date:") ) {
			if ( GET_ITEM_TYPE (obj) == ITEM_BOOK )
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			
			fgetc (fp);
			writing->date = fread_string(fp);
		}
		
		else if ( !strcmp (p, "language:") )
			writing->language = fread_number (fp);
		
		else if ( !strcmp (p, "script:") )
			writing->script = fread_number (fp);
		
		else if ( !strcmp (p, "ink:") ) {
			if ( GET_ITEM_TYPE (obj) == ITEM_BOOK )
				obj->o.od.value[1] = 0;
			else
				obj->o.od.value[0] = 0;
			
			fgetc (fp);
			writing->ink = fread_string(fp);
		}
		
		else if ( !strcmp (p, "skill:") )
			writing->skill = fread_number(fp);
		
		else if ( !strcmp (p, "desc_keys") ) {
			fgetc (fp);
			obj->desc_keys = fread_string (fp);
		}
		
		else if ( !strcmp (p, "OmoteStr") ) {
			temp_arg = fread_string(fp);
			obj->omote_str = unspace(temp_arg);
		} 
		
		else if ( !strcmp (p, "Wound") ) {
			if ( !obj->wounds ) {
				CREATE (obj->wounds, WOUND_DATA, 1);
				wound = obj->wounds;
				obj->wounds->next = NULL;
			}
			else {
				CREATE (wound->next, WOUND_DATA, 1);
				wound = wound->next;
				wound->next = NULL;
			}
			temp_arg = fread_word(fp);
			wound->location = add_hash (temp_arg);
		
			temp_arg = fread_word(fp);
			wound->type = add_hash(temp_arg);
			
			temp_arg = fread_word(fp);
			wound->severity = add_hash (temp_arg);
			
			temp_arg = fread_word(fp);
			wound->name = add_hash(temp_arg);
			wound->damage = fread_number(fp);
			wound->bleeding = fread_number(fp);
			wound->poison = fread_number(fp);
			wound->infection = fread_number(fp);
			wound->healerskill = fread_number(fp);
			wound->lasthealed = fread_number(fp);
			wound->lastbled = fread_number(fp);
		} 
		
		else if ( !strcmp (p, "Damage") ) {
			if ( !obj->damage ) {
				CREATE (obj->damage, OBJECT_DAMAGE, 1);
				damage = obj->damage;
				damage->next = NULL;
			}
			else {
				CREATE (damage->next, OBJECT_DAMAGE, 1);
				damage = damage->next;
				damage->next = NULL;
			}
			temp_arg = fread_word(fp);
			damage->type = str_dup (temp_arg);
		
			temp_arg = fread_word(fp);
			damage->name = str_dup (temp_arg);
			
			temp_arg = fread_word(fp);
			damage->severity = str_dup (temp_arg);
			damage->damage = fread_number(fp);
		} 
		
		else if ( !strcmp (p, "Lodged") ) {
			if ( !obj->lodged ) {
				CREATE (obj->lodged, LODGED_OBJECT_INFO, 1);
				lodged = obj->lodged;
				obj->lodged->next = NULL;
			}
			else {
				CREATE (lodged->next, LODGED_OBJECT_INFO, 1);
				lodged = lodged->next;
				lodged->next = NULL;
			}
			temp_arg = fread_word(fp);
			lodged->location = add_hash(temp_arg);
			lodged->vnum = fread_number(fp);
		}
	}

	if ( old_money ) {
		obj->count = obj->o.od.value [0];
		obj->o.od.value [0] = 0;
	}

	  	/* read_object probably created associated some affects with the
           new object.  Lets erase all those and put in saved affects */

	for ( i = 0; obj && i < affect_count; i++ ) {
		temp_arg = fread_word (fp);
		if ( !strcmp (temp_arg, "Afflocmod") ) {

			CREATE (af, AFFECTED_TYPE, 1);
	
			af->next		= NULL;
	
			fscanf (fp, "%d %d %d %d %d %d %d\n",
					&af->a.spell.location,
					&af->a.spell.modifier,
					&af->a.spell.duration,
					&af->a.spell.bitvector,
					&af->a.spell.t,
					&af->a.spell.sn,
					&af->type);
	
			if ( !obj->xaffected ){
				obj->xaffected = af;
			}
			else {
				taf = obj->xaffected;
			
				while ( taf->next )
					taf = taf->next;
			
				taf->next = af;
			}
		}
		
		/**** for lua functions  **/
	
		if ( !strcmp (p, "Prog") ) {
			if ( !obj->triggers ) {
				CREATE (obj->triggers, TRIGGER_DATA, 1);
				trig = obj->triggers;
				obj->triggers->next = NULL;
			}
			else {
				CREATE (trig->next, TRIGGER_DATA, 1);
				trig = trig->next;
				trig->next = NULL;
			}
			trig->type = fread_number(fp);
			temp_arg = fread_word(fp);
			trig->script = add_hash(temp_arg);
			temp_arg = fread_word(fp);
			trig->func = add_hash(temp_arg);
		}
	/** end lua functions **/
	
	}

	
	if ( GET_ITEM_TYPE (obj) == ITEM_DRINKCON ||                                
         GET_ITEM_TYPE (obj) == ITEM_LIGHT ||                                   
         GET_ITEM_TYPE (obj) == ITEM_FOUNTAIN )
        if ( !(tobj = vtoo (obj->o.drinkcon.liquid)) ||                         
             GET_ITEM_TYPE (tobj) != ITEM_FLUID )
			obj->o.drinkcon.liquid = vtoo (obj->virtual)->o.drinkcon.liquid;

	if ( !IS_SET (obj->obj_flags.extra_flags, ITEM_NEWSKILLS) )
		update_weapon_skills (obj);

	/* Record what object previously was */

	if ( obj->virtual == 666 )
		obj->o.od.value[0] = obj_vnum;

    return obj;
}

int fread_look_obj_header (int *wc, int *eqloc, int *eqlev, FILE *fp)
{
    char		*ptr = NULL;

    if ( last_wc != POSSESS_UNDEF ) {
       *wc = last_wc;
       *eqloc = last_eqloc;
       *eqlev = last_eqlev;
       return (1);
    }

    if ( file_eof_found || save_stop_obj_processing )
       return (0);

    ptr = fread_word (fp);

    if ( *ptr == '\0' ) 	/* Probably the end of file mark */
       return (0);

    else if ( !strcmp (ptr, "CARRY") )
       *wc = POSSESS_CARRY;

    else if ( !strcmp (ptr, "WEAR") )
       *wc = POSSESS_WEAR;

	else if ( !strcmp (ptr, "ROOM") )
		*wc = POSSESS_ROOM;

    else if ( !strcmp (ptr, "DONE") ) {
		save_stop_obj_processing = 1;
		return (0);
	}
       
    else if ( !strcmp (ptr, "END") ) {
       file_eof_found = 1;
       return (0);
    }

    else
       return (0);

    *eqloc = fread_number (fp);
    *eqlev = fread_number (fp);

    last_wc = *wc;
    last_eqloc = *eqloc;
    last_eqlev = *eqlev;

    return (1);
}

void read_obj_list (int cur_wc, int cur_eqloc, int cur_eqlev, CHAR_DATA *ch,
					OBJ_DATA *holder, FILE *fp, int room_num)
{
	int				eqloc = 0;
	int				eqlev = 0;
	int				wc = 0;
	OBJ_DATA		*obj = NULL;

	while (1) {

		if ( !fread_look_obj_header (&wc, &eqloc, &eqlev, fp) )
			return;

		if ( cur_eqlev > eqlev )
			return;

                if ( (cur_wc != wc || cur_eqloc != eqloc) && eqloc != -1 && eqloc != WEAR_PRIM && eqloc != WEAR_SEC && eqloc != WEAR_BOTH ) {
                       	return;
                }

		if ( (cur_eqlev < eqlev) )
			read_obj_list (wc, eqloc, eqlev, ch, obj, fp, room_num);
		else {
			last_wc = POSSESS_UNDEF;

		obj = fread_obj (fp);

		if ( !obj ) return;
		else if ( holder ) {
			if ( ch )		/* All non-player-held books handled by load_all_writing(); */
				load_writing(obj);
			obj_to_obj (obj, holder);
		}
		else if ( wc == POSSESS_ROOM ) {
			if ( !vtor (room_num) )
				printf ("Room %d does not exist for object %d.\n",
						room_num, obj->virtual);
			else
				obj_to_room (obj, room_num);
		} else if ( cur_eqloc == -1 ) {
			obj->in_room = NOWHERE;
			if ( ch )
				load_writing(obj);
			obj_to_char (obj, ch);
			if ( eqloc == WEAR_PRIM || eqloc == WEAR_SEC || eqloc == WEAR_BOTH )
				equip_char (ch, obj, eqloc);
		}
		else {
			if ( ch )
				load_writing(obj);
			obj->carried_by = ch;
			obj->in_room = NOWHERE;
			equip_char (ch, obj, eqloc);
		}

		}  /* cur_eqlev < eqlev */
	} /* while (1) */
	return;
}

OBJ_DATA *get_equipped (CHAR_DATA *ch, int location)
{
        OBJ_DATA        *obj = NULL;
        
        for ( obj = ch->equip; obj; obj = obj->next_content )
        	if ( obj->location == location )
                	return obj;

        return NULL;
}

void write_obj_suppliment (CHAR_DATA *ch, FILE *fp)
{
	int		i = 0;
	OBJ_DATA 	*temp_obj = NULL;

	if ( ch->right_hand ) {
		write_obj_data (ch->right_hand, "CARRY", ch->right_hand->location, 0, fp);
	}

	if ( ch->left_hand ) {
		write_obj_data (ch->left_hand, "CARRY", ch->left_hand->location, 0, fp);
	}

	for ( i = 0; i < MAX_WEAR; i++ ) {
		if ( i == WEAR_PRIM || i == WEAR_BOTH || i == WEAR_SEC || i == WEAR_SHIELD )
			continue;
		
		temp_obj = get_equipped (ch, i);
		write_obj_data (temp_obj, "WEAR", i, 0, fp);
	}

	fprintf (fp, "END\n");
	return;
}

void read_obj_suppliment (CHAR_DATA *ch, FILE *fp)
{
	int		i = 0;

    file_eof_found = 0;
	save_stop_obj_processing = 0;
    last_wc = POSSESS_UNDEF;
	load_char_objects = 1;			/* Global variable */

    ch->right_hand = NULL;
	ch->left_hand = NULL;
	ch->equip = NULL;

    read_obj_list (POSSESS_CARRY, -1, 0, ch, NULL, fp, 0);

    for (i = 0; i < MAX_WEAR; i++) {
	if ( i == WEAR_PRIM || i == WEAR_SEC || i == WEAR_BOTH )
		continue;
       read_obj_list (POSSESS_WEAR, i, 0, ch, NULL, fp, 0);
	}

	load_char_objects = 0;
	return;
}

#define		TYPE_STRING			1
#define		TYPE_INT			2
#define		TYPE_SKILL			3
#define		TYPE_END			5
#define		TYPE_AFFECT			6
#define		TYPE_SHORTINT		7
#define		TYPE_DREAM			8
#define		TYPE_OBSOLETE		9
#define		TYPE_LONG			10
#define		TYPE_SUBCRAFT		11
#define		TYPE_ALIAS			12
#define		TYPE_WOUND			13
#define		TYPE_STORED_PC		14
#define		TYPE_LODGED			15
#define		TYPE_SPELL			16
#define		TYPE_CHARM			17
#define		TYPE_VOICE			18
#define		TYPE_HOODED			19
#define		TYPE_NANNY			21
#define		TYPE_APPCOST		22
#define		TYPE_ROLE			23
#define		TYPE_ROLE_SUMMARY	24
#define		TYPE_ROLE_BODY		25
#define		TYPE_ROLE_DATE		26
#define		TYPE_ROLE_POSTER	27
#define		TYPE_ROLE_COST		28
#define		TYPE_MAIL_MESSAGE	29
#define		TYPE_IP				30
#define		TYPE_AFFECTEDBY		31
#define		TYPE_NEWSLETTER		32
#define		TYPE_OWNER			33
#define		TYPE_STABLED		34
#define		TYPE_HAS_INV		35
#define		TYPE_SPAWN			36
#define		TYPE_MOUNT			37
#define		TYPE_RESET			38
#define		TYPE_RESETZ			39
#define		TYPE_DONE			40


struct key_data {
	char		key [14];
	int			key_type;
	void		*ptr;
};

CHAR_DATA *load_char (CHAR_DATA *ch, char *name)
{
	int					i = 0;
	int					n = 0;
	int					sn = 0;
	int					last_key = 0;
	int					num_keys = 0;
	int					clan1 = 0;
	int					clan2 = 0;
	char				*p = NULL;
	char				*p2 = NULL;
	FILE				*fp = NULL;
	AFFECTED_TYPE		*af = NULL;
	DREAM_DATA			*dream = NULL;
	DREAM_DATA			*dream_list = NULL;
	CHAR_DATA			*tch = NULL;
	SUBCRAFT_HEAD_DATA	*craft = NULL;
	ALIAS_DATA			*current_alias = NULL;
	ALIAS_DATA			*alias = NULL;
	LODGED_OBJECT_INFO		*lodged = NULL;
	LODGED_OBJECT_INFO		*tmplodged = NULL;
	CHARM_DATA			*ench = NULL;
	CHARM_DATA			*tmpench = NULL;
	WOUND_DATA			*wound = NULL;
	WOUND_DATA			*tmpwound = NULL;
	char				current_alias_name [MAX_STRING_LENGTH] = { '\0' };
	char				buf [MAX_STRING_LENGTH] = { '\0' };
	char				buf2 [MAX_STRING_LENGTH] = { '\0' };
	char				*temp_arg = NULL;

	struct key_data key_table [] = {
		{ "Keywords",	TYPE_STRING,	NULL },
		{ "Account",	TYPE_STRING,	NULL },
		{ "Description",TYPE_STRING,	NULL },
		{ "Short",		TYPE_STRING,	NULL },
		{ "Long",		TYPE_STRING,	NULL },
		{ "Msg",		TYPE_STRING,	NULL },
		{ "CreateCom",	TYPE_STRING,	NULL },
		{ "Email",		TYPE_STRING,	NULL },
		{ "CreateState",TYPE_INT,		NULL },
		{ "Level",      TYPE_INT, 		NULL },
		{ "Sex",        TYPE_INT, 		NULL },
		{ "Clan",		TYPE_INT,		NULL },
		{ "Deity",		TYPE_INT,		NULL },
		{ "Height",     TYPE_INT, 		NULL },
		{ "Frame",      TYPE_INT, 		NULL },
		{ "Race",		TYPE_INT,		NULL },
		{ "Room",       TYPE_INT, 		NULL },
		{ "Str",        TYPE_INT, 		NULL },
		{ "Int",        TYPE_INT, 		NULL },
		{ "Wil",        TYPE_INT, 		NULL },
   		{ "Con",        TYPE_INT, 		NULL },
   		{ "Dex",        TYPE_INT, 		NULL },
   		{ "Aur",        TYPE_INT, 		NULL },
		{ "Agi",		TYPE_INT,		NULL },
		{ "StartStr",   TYPE_INT, 		NULL },
		{ "StartInt",   TYPE_INT, 		NULL },
		{ "StartWil",   TYPE_INT, 		NULL },
   		{ "StartCon",   TYPE_INT, 		NULL },
   		{ "StartDex",   TYPE_INT, 		NULL },
   		{ "StartAur",   TYPE_INT, 		NULL },
		{ "StartAgi",	TYPE_INT,		NULL },
   		{ "Played",     TYPE_LONG, 		NULL },
   		{ "Birth",      TYPE_INT, 		NULL },
   		{ "Time",       TYPE_INT, 		NULL },
   		{ "Cond0",      TYPE_INT, 		NULL },
   		{ "Cond1",      TYPE_INT, 		NULL },
   		{ "Cond2",      TYPE_INT, 		NULL },
		{ "Offense",	TYPE_SHORTINT,	NULL },
		{ "Clan_2",		TYPE_INT,		NULL },
		{ "PPoints",	TYPE_INT,		NULL },
		{ "Hit",		TYPE_INT,		NULL },
		{ "NatAttType",	TYPE_INT,		NULL },
		{ "Move",		TYPE_INT,		NULL },
		{ "Maxmove",	TYPE_INT,		NULL },
		{ "Circle",		TYPE_INT,		NULL },
		{ "FightMode",	TYPE_INT,		NULL },
		{ "Color",		TYPE_INT,		NULL },
		{ "Speaks",		TYPE_INT,		NULL },
		{ "Nightmare",	TYPE_INT,		NULL },
		{ "Flags",		TYPE_INT,		NULL },
		{ "PlrFlags",	TYPE_INT,		NULL },
		{ "Talents",	TYPE_INT,		NULL },
		{ "BoatVnum",	TYPE_INT,		NULL },
		{ "Speed",		TYPE_INT,		NULL },
		{ "MountSpeed", TYPE_INT,		NULL },
		{ "Common",		TYPE_INT,		NULL },
		{ "LastPCMsg",  TYPE_INT,		NULL },
		{ "LastStaffM", TYPE_INT,		NULL },
		{ "SleepNeeded",TYPE_INT,		NULL },
		{ "LastLogon",  TYPE_INT,		NULL },
		{ "LastLogoff", TYPE_INT,		NULL },
		{ "LastDis",	TYPE_INT,		NULL },
		{ "LastConnect",TYPE_INT,		NULL },
		{ "LastDied",   TYPE_INT,		NULL },
		{ "Autotoll",	TYPE_INT,		NULL },
		{ "ColdloadID", TYPE_INT,		NULL },
		{ "ImmEnter",	TYPE_STRING,	NULL },
		{ "ImmLeave",	TYPE_STRING,	NULL },
		{ "SiteLie",	TYPE_STRING,	NULL },
		{ "Clans",		TYPE_STRING,	NULL },
		{ "Dream",		TYPE_DREAM,		NULL },
		{ "Dreamed",	TYPE_DREAM,		NULL },
		{ "StaffNotes",	TYPE_INT,		NULL },
		{ "Age",		TYPE_INT,		NULL },
		{ "Intoxication", TYPE_INT,		NULL },
		{ "Hunger",		TYPE_INT,		NULL },
		{ "Thirst",		TYPE_INT,		NULL },
		{ "Totem",		TYPE_INT,		NULL },
		{ "Damage",		TYPE_INT,		NULL },
		{ "LastRegen",	TYPE_INT,		NULL },
		{ "LastRoom",	TYPE_INT,		NULL },
		{ "Harness",	TYPE_INT,		NULL },
		{ "MaxHarness",	TYPE_INT,		NULL },
		{ "Wound",		TYPE_WOUND,		NULL },
		{ "Spell",		TYPE_SPELL,		NULL },
		{ "Charm",		TYPE_CHARM,		NULL },
		{ "VoiceStr",	TYPE_VOICE,		NULL },
		{ "NannyState",	TYPE_NANNY,		NULL },
		{ "AppCost",	TYPE_APPCOST,	NULL },
		{ "Role",		TYPE_ROLE,		NULL },
		{ "RoleSummary",	TYPE_ROLE_SUMMARY,	NULL },
		{ "RoleBody",	TYPE_ROLE_BODY,	NULL },
		{ "RoleDate",	TYPE_ROLE_DATE,	NULL },
		{ "RolePoster",	TYPE_ROLE_POSTER,	NULL },
		{ "RoleCost",	TYPE_ROLE_COST,	NULL },
		{ "Lodged",		TYPE_LODGED,	NULL },
		{ "Skill",		TYPE_SKILL,		NULL },
		{ "Affect",		TYPE_AFFECT,	NULL },
		{ "AffectedBy",	TYPE_AFFECTEDBY,		NULL },
		{ "Subcraft",	TYPE_SUBCRAFT,	NULL },
		{ "IsHooded",	TYPE_HOODED,	NULL },
		{ "Alias",		TYPE_ALIAS,		NULL },
		{ "End",		TYPE_END,		NULL },
		{ "\0",			TYPE_INT,		NULL }
	};

	if ( !isalpha (*name) ) {
		return NULL;
	}

	key_table [n++].ptr = &ch->name;
	key_table [n++].ptr = &ch->pc->account;
	key_table [n++].ptr = &ch->description;
	key_table [n++].ptr = &ch->short_descr;
	key_table [n++].ptr = &ch->long_descr;
	key_table [n++].ptr = &ch->pc->msg;
	key_table [n++].ptr = &ch->pc->creation_comment;
	key_table [n++].ptr = &ch->pc->email_address;
	key_table [n++].ptr = &ch->pc->create_state;
	key_table [n++].ptr = &ch->pc->level;
	key_table [n++].ptr = &ch->sex;
	key_table [n++].ptr = &clan1;
	key_table [n++].ptr = &ch->deity;
	key_table [n++].ptr = &ch->height;
	key_table [n++].ptr = &ch->frame;
	key_table [n++].ptr = &ch->race;
	key_table [n++].ptr = &ch->in_room;
	key_table [n++].ptr = &ch->str;
	key_table [n++].ptr = &ch->intel;
	key_table [n++].ptr = &ch->wil;
   	key_table [n++].ptr = &ch->con;
   	key_table [n++].ptr = &ch->dex;
   	key_table [n++].ptr = &ch->aur;
	key_table [n++].ptr = &ch->agi;
	key_table [n++].ptr = &ch->pc->start_str;
	key_table [n++].ptr = &ch->pc->start_intel;
	key_table [n++].ptr = &ch->pc->start_wil;
   	key_table [n++].ptr = &ch->pc->start_con;
   	key_table [n++].ptr = &ch->pc->start_dex;
   	key_table [n++].ptr = &ch->pc->start_aur;
	key_table [n++].ptr = &ch->pc->start_agi;
   	key_table [n++].ptr = &ch->time.played;
   	key_table [n++].ptr = &ch->time.birth;
   	key_table [n++].ptr = &ch->time.logon;
   	key_table [n++].ptr = &ch->intoxication;
   	key_table [n++].ptr = &ch->hunger;
   	key_table [n++].ptr = &ch->thirst;
	key_table [n++].ptr = &ch->offense;
	key_table [n++].ptr = &clan2;
	key_table [n++].ptr = &ch->ppoints;
	key_table [n++].ptr = &ch->hit;
	key_table [n++].ptr = &ch->nat_attack_type;
	key_table [n++].ptr = &ch->move;
	key_table [n++].ptr = &ch->max_move;
	key_table [n++].ptr = &ch->circle;
	key_table [n++].ptr = &ch->fight_mode;
	key_table [n++].ptr = &ch->color;
	key_table [n++].ptr = &ch->speaks;
	key_table [n++].ptr = &ch->nightmare;
	key_table [n++].ptr = &ch->flags;
	key_table [n++].ptr = &ch->plr_flags;
	key_table [n++].ptr = &ch->psionic_talents;
	key_table [n++].ptr = &ch->pc->boat_virtual;
	key_table [n++].ptr = &ch->speed;
	key_table [n++].ptr = &ch->pc->mount_speed;
	key_table [n++].ptr = &ch->pc->common;
	key_table [n++].ptr = &ch->pc->last_global_pc_msg;
	key_table [n++].ptr = &ch->pc->last_global_staff_msg;
	key_table [n++].ptr = &ch->pc->sleep_needed;
	key_table [n++].ptr = &ch->pc->last_logon;
	key_table [n++].ptr = &ch->pc->last_logoff;
	key_table [n++].ptr = &ch->pc->last_disconnect;
	key_table [n++].ptr = &ch->pc->last_connect;
	key_table [n++].ptr = &ch->pc->last_died;
	key_table [n++].ptr = &ch->pc->auto_toll;
	key_table [n++].ptr = &ch->coldload_id;
	key_table [n++].ptr = &ch->pc->imm_enter;
	key_table [n++].ptr = &ch->pc->imm_leave;
	key_table [n++].ptr = &ch->pc->site_lie;
	key_table [n++].ptr = &ch->clans;
	key_table [n++].ptr = &ch->pc->dreams;
	key_table [n++].ptr = &ch->pc->dreamed;
	key_table [n++].ptr = &ch->pc->staff_notes;
	key_table [n++].ptr = &ch->age;
	key_table [n++].ptr = &ch->intoxication;
	key_table [n++].ptr = &ch->hunger;
	key_table [n++].ptr = &ch->thirst;
	key_table [n++].ptr = &ch->idol;
	key_table [n++].ptr = &ch->damage;
	key_table [n++].ptr = &ch->lastregen;
	key_table [n++].ptr = &ch->was_in_room;
	key_table [n++].ptr = &ch->harness;
	key_table [n++].ptr = &ch->max_harness;
	key_table [n++].ptr = &ch->voice_str;
	key_table [n++].ptr = NULL;			/* Nanny State */
	key_table [n++].ptr = NULL;			/* Role Flag */
	key_table [n++].ptr = NULL;			/* Special Role (Summary)*/
	key_table [n++].ptr = NULL;			/* Role Body */
	key_table [n++].ptr = NULL;			/* Role Date */
	key_table [n++].ptr = NULL;			/* Role Poster */
	key_table [n++].ptr = NULL;			/* Role Cost */
	key_table [n++].ptr = NULL;			/* App Cost */
	key_table [n++].ptr = NULL;			/* Skill */
	key_table [n++].ptr = NULL;			/* Voice */
	key_table [n++].ptr = NULL;			/* Spell */
	key_table [n++].ptr = NULL;			/* Affect */
	key_table [n++].ptr = NULL;			/* AffectedBy */
	key_table [n++].ptr = NULL;			/* Subcraft */
	key_table [n++].ptr = NULL;			/* Wound */
	key_table [n++].ptr = NULL;			/* CHARM */
	key_table [n++].ptr = NULL;			/* Lodged */
	key_table [n++].ptr = NULL;			/* Alias */
	key_table [n++].ptr = NULL;			/* End */

	GET_NAME (ch) = str_dup (name);

	snprintf (buf, MAX_STRING_LENGTH,  "save/player/%c/%s", tolower (*name), CAP (name));

	if ( !(fp = fopen (buf, "r")) ) {
		return NULL;
	}

	ch->pc->msg = NULL;
	ch->pc->creation_comment = NULL;
	ch->pc->email_address = NULL;
	ch->pc->site_lie = NULL;
	ch->deleted = 0;
	ch->hour_affects = NULL;
	ch->wounds = NULL;
	ch->lodged = NULL;
	ch->subdue = NULL;

	ch->tmp_str    = 0;
	ch->tmp_intel	= 0;
	ch->tmp_dex	= 0;
	ch->tmp_wil	= 0;
	ch->tmp_con	= 0;
	ch->tmp_aur	= 0;
	ch->tmp_agi = 0;

	ch->str 	= 0;
	ch->intel	= 0;
	ch->dex		= 0;
	ch->wil		= 0;
	ch->con		= 0;
	ch->aur		= 0;
	ch->agi		= 0;

	ch->clans = str_dup ("");

	for ( i = 0; i < MAX_SKILLS; i++ ) {
		ch->pc->skills [i] = 0;
		ch->skills [i] = 0;
	}

	for ( num_keys = 0; *key_table [num_keys].key; )
		num_keys++;

	for ( ; !feof (fp); ) {

		if ( !(p = fread_word (fp)) ) {
			fclose (fp);
			free_char (ch);
			return NULL;
		}

		if ( !str_cmp (p, "Defense") ) {
			fread_number (fp);
			continue;
		}

		for ( i = last_key;
			  i < last_key + num_keys &&
				str_cmp (key_table [i % num_keys].key, p);)
			i++;

		i = i % num_keys;

		if ( str_cmp (key_table [i].key, p) ) {
			continue;
		}

		if ( key_table [i].key_type == TYPE_SKILL ||
			 key_table [i].key_type == TYPE_AFFECT ||
			 key_table [i].key_type == TYPE_WOUND ||
			 key_table [i].key_type == TYPE_SPELL ||
			 key_table [i].key_type == TYPE_LODGED ||
			 key_table [i].key_type == TYPE_VOICE ||
			 key_table [i].key_type == TYPE_HOODED ||
			 key_table [i].key_type == TYPE_NANNY ||
			 key_table [i].key_type == TYPE_ROLE ||
			 key_table [i].key_type == TYPE_ROLE_SUMMARY ||
			 key_table [i].key_type == TYPE_ROLE_BODY ||
			 key_table [i].key_type == TYPE_ROLE_DATE ||
			 key_table [i].key_type == TYPE_ROLE_POSTER ||
			 key_table [i].key_type == TYPE_ROLE_COST ||
			 key_table [i].key_type == TYPE_AFFECTEDBY ||
			 key_table [i].key_type == TYPE_APPCOST )
			last_key = i;
		else
			last_key = i + 1;

		if ( key_table [i].key_type == TYPE_INT )
			* (int *) key_table [i].ptr = fread_number (fp);

		else if ( key_table [i].key_type == TYPE_SHORTINT )
			* (shortint *) key_table [i].ptr = fread_number (fp);

		else if ( key_table [i].key_type == TYPE_STRING ){
			temp_arg = fread_string (fp);
			* (char **) key_table [i].ptr = unspace (temp_arg);
		}

		else if ( key_table [i].key_type == TYPE_LONG )
			* (long *) key_table [i].ptr = fread_number (fp);

		else if ( key_table [i].key_type == TYPE_OBSOLETE )
			sn = fread_number (fp);

		else if ( key_table [i].key_type == TYPE_ROLE_SUMMARY ) {
			CREATE (ch->pc->special_role, ROLE_DATA, 1);
			temp_arg = fread_string(fp);
			ch->pc->special_role->summary = unspace(temp_arg);
		}

		else if ( key_table [i].key_type == TYPE_ROLE_BODY ) {
			temp_arg = fread_string(fp);
			ch->pc->special_role->body = unspace(temp_arg);
		}

		else if ( key_table [i].key_type == TYPE_ROLE_DATE ) {
			temp_arg = fread_string(fp);
			ch->pc->special_role->date = unspace(temp_arg);
		}

		else if ( key_table [i].key_type == TYPE_AFFECTEDBY ) {
			ch->affected_by = fread_number(fp);
		}

		else if ( key_table [i].key_type == TYPE_ROLE_POSTER ) {
			temp_arg = fread_string(fp);
			ch->pc->special_role->poster = unspace(temp_arg);
		}

		else if ( key_table [i].key_type == TYPE_ROLE_COST ) {
			ch->pc->special_role->cost = fread_number(fp);
		}

		else if ( key_table [i].key_type == TYPE_END ) {

			ch->tmp_str	 = ch->str;
			ch->tmp_dex	 = ch->dex;
			ch->tmp_intel 	 = ch->intel;
			ch->tmp_wil	 = ch->wil;
			ch->tmp_con	 = ch->con;
			ch->tmp_aur	 = ch->aur;
			ch->tmp_agi  = ch->agi;

			GET_MAX_MOVE (ch) = calc_lookup (ch, REG_MISC, MISC_MAX_MOVE);

			REMOVE_BIT (ch->flags, FLAG_ENTERING);
			REMOVE_BIT (ch->flags, FLAG_LEAVING);

			ch->max_hit = 50 + GET_CON (ch) * CONSTITUTION_MULTIPLIER;

			fclose (fp);

			if ( ch->pc->boat_virtual ) {
				for ( tch = character_list; tch; tch = tch->next ) {
					if ( tch->deleted )
						continue;
					if ( IS_NPC (tch) &&
						 tch->mob->virtual == ch->pc->boat_virtual ) {
						ch->vehicle = tch;
						break;
					}
				}

				if ( !ch->vehicle ) {
					ch->pc->boat_virtual = 0;
					ch->in_room = BOAT_ROOM;
				}
			}

			if ( get_affect (ch, MAGIC_AFFECT_SLEEP) )
				GET_POS (ch) = SLEEP;

			if ( !ch->coldload_id )
				ch->coldload_id = get_next_coldload_id (1);

			fix_offense (ch);

			p = ch->clans;
			p2 = p;
			ch->clans = str_dup ("");

			while ( 1 ) {

				p = one_argument (p, buf);		/* flags     */
				p = one_argument (p, buf2);		/* clan name */

				if ( !*buf2 )
					break;

				add_clan_id (ch, buf2, buf);
			}

			mem_free (p2);

			if ( clan1 ) {
				snprintf (buf, MAX_STRING_LENGTH,  "%d", clan1);
				add_clan_id (ch, buf,
					GET_FLAG (ch, FLAG_LEADER_1) ? "leader" : "member");
			}

			if ( clan2 ) {
				snprintf (buf, MAX_STRING_LENGTH,  "%d", clan2);
				add_clan_id (ch, buf,
					GET_FLAG (ch, FLAG_LEADER_2) ? "leader" : "member");
			}

			REMOVE_BIT (ch->flags, FLAG_LEADER_1);
			REMOVE_BIT (ch->flags, FLAG_LEADER_2);

			ch->time.logon = time (0);

			return ch;
		}

		else if ( key_table [i].key_type == TYPE_NANNY )
			ch->pc->nanny_state = fread_number(fp);

		else if ( key_table [i].key_type == TYPE_APPCOST )
			ch->pc->app_cost = fread_number(fp);

		else if ( key_table [i].key_type == TYPE_ROLE )
			ch->pc->role = fread_number(fp);

		else if ( key_table [i].key_type == TYPE_SKILL ) {
			p = fread_word (fp);
			sn = skill_index_lookup (p);

			if ( sn == -1 )
				sn = skill_index_lookup (p);

			else if ( sn > MAX_SKILLS )
				printf ("Skill Num # %d (learned %d) out of range.\n",
						sn, fread_number (fp));
			else {
				ch->skills [sn] = fread_number (fp);
				ch->pc->skills [sn] = ch->skills [sn];
			}
		}

		else if ( key_table [i].key_type == TYPE_AFFECT ) {

			af = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);

			fscanf (fp, "%d %d %d %d %d %d %d\n",
							&af->type,
							&af->a.spell.duration,
							&af->a.spell.modifier,
							&af->a.spell.location,
							&af->a.spell.bitvector,
							&af->a.spell.sn,
							&af->a.spell.t);
			af->next		= NULL;

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

		else if ( key_table [i].key_type == TYPE_HOODED ) {
			SET_BIT (ch->affected_by, AFF_HOODED);
		}

		else if ( key_table [i].key_type == TYPE_SUBCRAFT ) {

			p = fread_word (fp);

			for ( craft = crafts;
				  craft && str_cmp (craft->subcraft_name, p);
				  craft = craft->next )
				;

			if ( !craft ) {
				continue;
			}

		    for ( i = CRAFT_FIRST; i <= CRAFT_LAST; i++ )
				if ( !get_affect (ch, i) )
					break;

			if ( i > CRAFT_LAST ) {
				continue;
			}

			magic_add_affect (ch, i, -1, 0, 0, 0, 0);

			af = get_affect (ch, i);

			af->a.craft = (struct affect_craft_type *)alloc (sizeof (struct affect_craft_type), 23);

			af->a.craft->subcraft = craft;
		}

		else if ( key_table [i].key_type == TYPE_VOICE ) {
			temp_arg = fread_string(fp);
			ch->voice_str = unspace(temp_arg);
		}

		else if ( key_table [i].key_type == TYPE_SPELL ) {
			continue;
		}

		else if ( key_table [i].key_type == TYPE_CHARM ) {
			ench =  (CHARM_DATA *)alloc ((int)sizeof (CHARM_DATA), 36);
			ench->next = NULL;

			ench->name = fread_string(fp);
			ench->original_hours = fread_number(fp);
			ench->current_hours = fread_number(fp);
			ench->power_source = fread_number(fp);
			ench->caster_skill = fread_number(fp);

			if ( !ch->charms )
				ch->charms = ench;
			else {
				tmpench = ch->charms;
				while ( tmpench->next )
					tmpench = tmpench->next;
				tmpench->next = ench;
			}
		}

		else if ( key_table [i].key_type == TYPE_WOUND ) {
			CREATE (wound, WOUND_DATA, 1);
			wound->next		= NULL;
			
			temp_arg = fread_word(fp);
			wound->location = add_hash(temp_arg);
			
			temp_arg = fread_word(fp);
			wound->type = add_hash(temp_arg);
			
			temp_arg = fread_word(fp);
			wound->severity = add_hash(temp_arg);
			
			temp_arg = fread_word(fp);
			wound->name = add_hash(temp_arg);
			
			wound->damage = fread_number(fp);
			wound->bleeding = fread_number(fp);
			wound->poison = fread_number(fp);
			wound->infection = fread_number(fp);
			wound->healerskill = fread_number(fp);
			wound->lasthealed = fread_number(fp);
			wound->lastbled = fread_number(fp);

			if ( !ch->wounds )
				ch->wounds = wound;
			else {
				tmpwound = ch->wounds;
				while ( tmpwound->next )
					tmpwound = tmpwound->next;
				tmpwound->next = wound;
			}

		}
		
		else if ( key_table [i].key_type == TYPE_LODGED ) {
			lodged = (LODGED_OBJECT_INFO *)alloc ((int)sizeof (LODGED_OBJECT_INFO), 36);
			lodged->next		= NULL;
			
			temp_arg = fread_word(fp);
			lodged->location = add_hash(temp_arg);
			lodged->vnum = fread_number(fp);

			if ( !ch->lodged )
				ch->lodged = lodged;
			else {
				tmplodged = ch->lodged;
				while ( tmplodged->next )
					tmplodged = tmplodged->next;
				tmplodged->next = lodged;
			}

		}
		
		else if ( key_table [i].key_type == TYPE_DREAM ) {

			CREATE (dream, DREAM_DATA, 1);
			temp_arg = fread_string (fp);
			dream->dream = unspace (temp_arg);
			dream->next  = NULL;

			if ( * (DREAM_DATA **) key_table [i].ptr == NULL )
				* (DREAM_DATA **) key_table [i].ptr = dream;

			else {
				dream_list = * (DREAM_DATA **) key_table [i].ptr;

				while ( dream_list->next != NULL )
					dream_list = dream_list->next;

				dream_list->next = dream;
			}
		}

		else if ( key_table [i].key_type == TYPE_ALIAS ) {

			temp_arg = fread_word(fp);
			strcpy (buf, temp_arg);

			if ( !str_cmp (buf, current_alias_name) ) {
				for ( alias = current_alias;
					  alias->next_line;
					  alias = alias->next_line )
					;
				alias->next_line = (ALIAS_DATA *)alloc ((int)sizeof (ALIAS_DATA), 28);
				alias = alias->next_line;
			}

			else {
				strcpy (current_alias_name, buf);

				alias = (ALIAS_DATA *)alloc ((int)sizeof (ALIAS_DATA), 28);
				alias->command = str_dup (current_alias_name);

				if ( !ch->pc->aliases )
					ch->pc->aliases = alias;
				else
					current_alias->next_alias = alias;

				current_alias = alias;
			}

			p = strcpy (buf, fgets (buf, MAX_STRING_LENGTH, fp));
			buf [strlen (buf) - 1] = '\0';

			while ( *p == ' ' || *p == '\t' )
				p++;

			alias->line = str_dup (p);
		}
	}

	fclose (fp);

	free_char (ch);
	return NULL;
}

int save_char (CHAR_DATA *ch, int save_objs)
{
	if ( IS_NPC (ch) || IS_SET (ch->flags, FLAG_GUEST) || !ch->tname || !*ch->tname ) {
		return 1;
	}

	save_char_mysql (ch);

	if ( save_objs )
		save_char_objs (ch, ch->tname);

	if ( ch->room )
		save_attached_mobiles (ch, 0);

	return 0;
}

void load_online_stats (void)
{
	FILE		*fp = NULL;

	if ( !(fp = fopen("online_stats", "r")) ) {
		system_log("Error opening online_stats!", TRUE);
		return;
	}

	fscanf (fp, "%d\n", &count_max_online);
	max_online_date = fread_string(fp);

	fclose (fp);
	return;
}

void load_leantos (void)
{
	FILE		*fp = NULL;
	OBJ_DATA	*obj = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	int			toroom = 0;
	int			inroom = 0;
    struct room_prog        *t, *old, *tmp;

	if ( !(fp = fopen("../regions/leantos", "r")) )
		return;

	while ( !feof (fp) ) {
		fscanf (fp, "leanto from room %d to room %d\n", &inroom, &toroom);
		obj = load_object(LEANTO_OBJ_VNUM);
		obj_to_room (obj, inroom);
		snprintf (buf, MAX_STRING_LENGTH,  "vstr You enter the lean-to.\nostr %%n enters the lean-to.\natecho %d %%n enters the lean-to.\ntrans %d\natlook %d\n", toroom, toroom, toroom);
                CREATE (t, struct room_prog,1);
                t->next = NULL;
                t->command = add_hash ("enter");
                t->keys = add_hash ("lean-to leanto");
                t->prog = add_hash (buf);
                if ( !vtor (inroom)->prg) {
                        vtor (inroom)->prg = t;
                } else {
                        old = vtor (inroom)->prg;
                        tmp = old;
                        while ( tmp ) {
                                old = tmp;
                                tmp = tmp->next;
                        }
                        old->next = t;
                }
	}

	fclose(fp);
	return;
}		

void load_mobiles (void)
{
	CHAR_DATA	*mob = NULL;
	WOUND_DATA	*wound = NULL;
	WOUND_DATA	*tmpwound = NULL;
	FILE		*fp = NULL;
	FILE		*op = NULL;
	char		*key = NULL;
	char		name[MAX_STRING_LENGTH] = {'\0'};
	char		location[MAX_STRING_LENGTH] = {'\0'};
	char		severity[MAX_STRING_LENGTH] = {'\0'};
	char		type[MAX_STRING_LENGTH] = {'\0'};
	char 		buf[MAX_STRING_LENGTH] = {'\0'};
	char		line[MAX_STRING_LENGTH] = {'\0'};
	char		*temp_arg = NULL;
	char		*temp_hold = NULL;
	int			i = 0;
	int			num = 0;
	int			to_room = 0;
	int			sn = 0;


	for ( i = 0; i <= 99; i++ ) {
		snprintf (buf, MAX_STRING_LENGTH,  "save/mobiles/mobs.%d", i);
		if ( !(fp = fopen(buf, "r+")) )
			continue;
		
		for ( ; !feof(fp) ; ) {
			key = fread_word(fp);
			if ( !str_cmp (key, "END") ) {
				if ( mob ) {
					SET_BIT (mob->act, ACT_STAYPUT);
					char_to_room (mob, to_room);
					if ( !mob->room ) {
						extract_char (mob);
					}
					mob = NULL;
				}
			}
			
			else if ( !str_cmp (key, "Keywords") ) {
				temp_arg = fread_string(fp);
				temp_hold = unspace(temp_arg);
				mob->name = temp_hold;
			}
			else if ( !str_cmp (key, "Owner") ) {
				temp_arg = fread_string(fp);
				temp_hold = unspace(temp_arg);
				mob->mob->owner = temp_hold;
			}
			else if ( !str_cmp (key, "SDesc") ) {
				temp_arg = fread_string(fp);
				temp_hold = unspace(temp_arg);
				mob->short_descr = temp_hold;
			}
			else if ( !str_cmp (key, "Skill") ) {
				key = fread_word(fp);
				sn = skill_index_lookup (key);
				
				if ( sn == -1 )
					sn = skill_index_lookup (key);
				if ( sn == -1 )
					continue;
				if ( sn > MAX_SKILLS )
					continue;
				else mob->skills [sn] = fread_number(fp);
			}
			else if ( !str_cmp (key, "LDesc") ) {
				temp_arg = fread_string(fp);
				temp_hold = unspace(temp_arg);
				mob->long_descr = temp_hold;
			}
			else if ( !str_cmp (key, "DatabaseNumber") ) {
				num = fread_number(fp);
				mob = new_char (0);
				mob = load_mobile (num);
			}
			else if ( !str_cmp (key, "ColdloadID") ) {
				num = fread_number(fp);
				mob->coldload_id = num;
			}
			else if ( !str_cmp (key, "CurrentRoom") ) {
				num = fread_number(fp);
				to_room = num;
			}
			else if ( !str_cmp (key, "SpawnLocation") ) {
				num = fread_number(fp);
				mob->mob->spawnpoint = num;
			}

			else if ( !str_cmp (key, "Wound") ) {
				CREATE (wound, WOUND_DATA, 1);
				wound->next = NULL;
				temp_arg = fread_string(fp);
				temp_hold = unspace(temp_arg);
				sscanf (line, "Wound              %s %s %s %s %d %d %d %d %d %d %d\n",
					location, type, severity, name,
					&wound->damage, &wound->bleeding, &wound->poison,
					&wound->infection, &wound->healerskill, &wound->lasthealed,
					&wound->lastbled);
				wound->location = add_hash(temp_hold);
				wound->type = add_hash(temp_hold);
				wound->severity = add_hash(temp_hold);
				wound->name = add_hash(temp_hold);
				wound->damage = fread_number(fp);
				wound->bleeding = fread_number(fp);
				wound->poison = fread_number(fp);
				wound->infection = fread_number(fp);
				wound->healerskill = fread_number(fp);
				wound->lasthealed = fread_number(fp);
				wound->lastbled = fread_number(fp);
				if ( !mob->wounds )
					mob->wounds = wound;
				else {
					tmpwound = mob->wounds;
					while ( tmpwound->next )
						tmpwound = tmpwound->next;
					tmpwound->next = wound;
				}
			}
			else if ( !str_cmp (key, "HasInventory") ) {
				op = NULL;
				*buf = '\0';
				snprintf (buf, MAX_STRING_LENGTH,  "save/mobiles/inventory/%d", mob->coldload_id);
				if ( !(op = fopen(buf, "r+")) )
					continue;
				read_obj_suppliment (mob, op);
				fclose (op);
			}
			else if ( !str_cmp (key, "IsHooded") ) {
				SET_BIT (mob->affected_by, AFF_HOODED);
			}
			else if ( !str_cmp (key, "ENDFILE") )
				break;
			else continue;
		}
		fclose (fp);
	}
	return;
}

void save_player_rooms (void)
{
	FILE		*fp = NULL;
	ROOM_DATA	*room = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

        for ( room = full_room_list; room; room = room->lnext ) {
        	if ( room->psave_loaded && room->contents ) {
            	snprintf (buf, MAX_STRING_LENGTH,  "save/rooms/%d", room->virtual);
				fp = fopen (buf, "w");
            	if ( !fp )
                	continue;
                    
                write_obj_data (room->contents, "ROOM", 0, 0, fp);
				fprintf (fp, "DONE\n");
                fclose (fp);
            }
		
			else if ( room->psave_loaded && !room->contents ) {
				snprintf (buf, MAX_STRING_LENGTH,  "save/rooms/%d", room->virtual);
				unlink (buf);
			}
        }
    return;
}

void do_saverooms (CHAR_DATA *ch, char *argument, int cmd)
{
	save_player_rooms ();
	send_to_char ("Ok.\n\r", ch);
	return;
}

void load_save_rooms (void)
{
	int			room_num = 0;
	int			line_no = 1;
	int			rooms = 0;
	int			room_obj_count = 0;
	OBJ_DATA	*obj = NULL;
	FILE		*rp = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};


	if ( !(rp = fopen ("save_rooms", "r")) ) {
		perror ("save_rooms");
		return;
	}

	file_eof_found = 0;
	save_stop_obj_processing = 0;
    	last_wc = POSSESS_UNDEF;

	while (1) {
		if ( !fgets (buf, 81, rp) ) {
			perror ("save_rooms");
			fclose (rp);
			return;
		}

		if ( !str_cmp (buf, "\n") )
			continue;

		line_no++;

		if ( *buf == '#' ) {

			rooms++;

			sscanf (buf, "#%d", &room_num);

			save_stop_obj_processing = 0;

			read_obj_list (POSSESS_ROOM, 0, 0, NULL, NULL, rp, room_num);

			if ( vtor (room_num) ) {

				for ( room_obj_count = 0, obj = vtor (room_num)->contents;
					  obj;
					  obj = obj->next_content )
					room_obj_count++;
			}
		}

		else if ( !str_cmp (buf, "END\n") ) {
			snprintf (buf, MAX_STRING_LENGTH,  "Save rooms restored:  %d", rooms);
			break;
		}

		else
			printf ("Unexpected line: %s\n", buf);
	}

	fclose (rp);
	return;
}

void load_save_room (ROOM_DATA *room)
{
	FILE		*rp = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};

	if ( !room || room->psave_loaded )
		return;

	room->psave_loaded = 1;

	snprintf (buf, MAX_STRING_LENGTH,  "save/rooms/%d", room->virtual);

	if ( !(rp = fopen (buf, "r")) ) {
		return;
	}

	file_eof_found = 0;
	save_stop_obj_processing = 0;
    	last_wc = POSSESS_UNDEF;

	read_obj_list (POSSESS_ROOM, 0, 0, NULL, NULL, rp, room->virtual);

	fclose (rp);
	
	return;
}

CHAR_DATA *load_saved_mobiles (CHAR_DATA *ch, char *name)
{
	int			virtual = 0;
	CHAR_DATA	*mob = NULL;
	CHAR_DATA	*last_mob = NULL;
	CHAR_DATA	*return_mob = NULL;
	FILE		*fp = NULL;
	char		buf [MAX_STRING_LENGTH] = {'\0'};
	char		hookup [MAX_STRING_LENGTH] = {'\0'};

	if ( !(fp = fopen (name, "r")) ) {
		return NULL;
	}

	last_mob = ch;

	while ( fgets (buf, 256, fp) ) {

		if ( *buf == ' ' || *buf == '\n' )
			(void)fgets (buf, 255, fp);

		if ( sscanf (buf, "%d %s", &virtual, hookup) != 2 ) {
			fclose (fp);
			return return_mob;
		}

		mob = load_a_saved_mobile (virtual, fp, TRUE);

		if ( !return_mob )
			return_mob = mob;

		char_to_room (mob, mob->in_room);

		if ( last_mob ){
			if ( !str_cmp (hookup, "HITCH") ){
				hitch_char (last_mob, mob);
			}
			else if ( !str_cmp (hookup, "RIDE") ) {
				last_mob->mount = mob;
				mob->mount = last_mob;
			}
		}
		last_mob = mob;
	}

	fclose (fp);

	return return_mob;
}

CHAR_DATA *load_a_saved_mobile (int virtual, FILE *fp, bool stable)
{
	int				i = 0; /* indices */
	int				n = 0;
	int				sn = 0;
	int				last_key = 0;
	int				num_keys = 0;
	int				stabled = 0;
	int				checks = 0;
	struct time_info_data		healing_time;
	char			*p = NULL; /* words */
	char			*p2 = NULL;
	char			buf [MAX_STRING_LENGTH] = {'\0'};
	char			buf2 [MAX_STRING_LENGTH] = {'\0'};
	AFFECTED_TYPE	*af = NULL;
	CHAR_DATA		*mob = NULL;
	CHAR_DATA		*tch = NULL;
	WOUND_DATA		*wound = NULL;
	WOUND_DATA		*tmpwound = NULL;
	LODGED_OBJECT_INFO	*lodged = NULL;
	LODGED_OBJECT_INFO	*tmplodged = NULL;
	char			*temp_arg = NULL;

	struct key_data key_table [] = {
		{ "Room",       TYPE_INT, 		NULL },
		{ "SDesc",		TYPE_STRING,		NULL },
		{ "LDesc",		TYPE_STRING,		NULL },
		{ "Keys",		TYPE_STRING,		NULL },
		{ "FightMode",	TYPE_INT,		NULL },
		{ "Clans",		TYPE_STRING,	NULL },
		{ "ColdloadId",	TYPE_INT,		NULL },
		{ "Flags",      TYPE_INT,		NULL },
		{ "Moves",		TYPE_INT,		NULL },
		{ "Hits",		TYPE_INT,		NULL },
		{ "Speaks",		TYPE_INT,		NULL },
		{ "AffectedBy",	TYPE_INT,		NULL },
		{ "Position",	TYPE_INT,		NULL },
		{ "Act",		TYPE_INT,		NULL },
   		{ "Cond0",      TYPE_INT, 		NULL },
   		{ "Cond1",      TYPE_INT, 		NULL },
   		{ "Cond2",      TYPE_INT, 		NULL },
		{ "Sex",        TYPE_INT, 		NULL },
		{ "Height",     TYPE_INT, 		NULL },
		{ "Frame",      TYPE_INT, 		NULL },
		{ "AccessFlags",TYPE_INT,		NULL },
		{ "SpawnLocation",	TYPE_SPAWN,	NULL },
		{ "Rider",		TYPE_MOUNT,	NULL },
		{ "Mount",		TYPE_MOUNT,	NULL },
		{ "ResetCmd",		TYPE_RESET,	NULL },
		{ "ResetZone",		TYPE_RESETZ,	NULL },
		{ "Skill",		TYPE_SKILL,		NULL },
		{ "Owner",		TYPE_OWNER,		NULL },
		{ "Stabled",		TYPE_STABLED,		NULL },
		{ "Affect",		TYPE_AFFECT,	NULL },
		{ "Wound",		TYPE_WOUND,		NULL },
		{ "Lodged",		TYPE_LODGED,		NULL },
		{ "HasInventory",	TYPE_HAS_INV,		NULL },
		{ "DONE",		TYPE_DONE,		NULL },
		{ "End",		TYPE_END,		NULL },
		{ "\0",			TYPE_INT,		NULL }
	};

	if ( !(mob = load_mobile (virtual)) ) {
		return NULL;
	}

	key_table [n++].ptr = &mob->in_room;
	key_table [n++].ptr = &mob->short_descr;
	key_table [n++].ptr = &mob->long_descr;
	key_table [n++].ptr = &mob->name;
	key_table [n++].ptr = &mob->fight_mode;
	key_table [n++].ptr = &mob->clans;
	key_table [n++].ptr = &mob->coldload_id;
	key_table [n++].ptr = &mob->flags;
	key_table [n++].ptr = &mob->move;
	key_table [n++].ptr = &mob->hit;
	key_table [n++].ptr = &mob->speaks;
	key_table [n++].ptr = &mob->affected_by;
	key_table [n++].ptr = &mob->position;
	key_table [n++].ptr = &mob->act;
	key_table [n++].ptr = &mob->intoxication;
	key_table [n++].ptr = &mob->hunger;
	key_table [n++].ptr = &mob->thirst;
	key_table [n++].ptr = &mob->sex;
	key_table [n++].ptr = &mob->height;
	key_table [n++].ptr = &mob->frame;
	key_table [n++].ptr = &mob->mob->access_flags;
	key_table [n++].ptr = NULL;
	key_table [n++].ptr = NULL;
	key_table [n++].ptr = NULL;
	key_table [n++].ptr = NULL;
	key_table [n++].ptr = NULL;
	key_table [n++].ptr = NULL;

	for ( num_keys = 0; *key_table [num_keys].key; )
		num_keys++;

	for ( ; !feof (fp);) {

		if ( !(p = fread_word (fp)) ) {
			free_char (mob);
			return NULL;
		}

		for ( i = last_key;
			  i < last_key + num_keys &&
				str_cmp (key_table [i % num_keys].key, p);)
			i++;

		i = i % num_keys;

		if ( str_cmp (key_table [i].key, p) ) {
			continue;
		}

		if ( key_table [i].key_type == TYPE_SKILL ||
			 key_table [i].key_type == TYPE_AFFECT ||
			 key_table [i].key_type == TYPE_WOUND ||
			 key_table [i].key_type == TYPE_LODGED ||
			 key_table [i].key_type == TYPE_OWNER ||
			 key_table [i].key_type == TYPE_STABLED ||
			 key_table [i].key_type == TYPE_HAS_INV ||
			 key_table [i].key_type == TYPE_SPAWN ||
			 key_table [i].key_type == TYPE_RESET ||
			 key_table [i].key_type == TYPE_RESETZ ||
			 key_table [i].key_type == TYPE_MOUNT )
			last_key = i;
		else
			last_key = i + 1;

		if ( key_table [i].key_type == TYPE_INT )
			* (int *) key_table [i].ptr = fread_number (fp);

		else if ( key_table [i].key_type == TYPE_SHORTINT )
			* (shortint *) key_table [i].ptr = fread_number (fp);

		else if ( key_table [i].key_type == TYPE_STRING ){
			temp_arg = fread_string(fp);
			* (char **) key_table [i].ptr = unspace (temp_arg);
		}

		else if ( key_table [i].key_type == TYPE_LONG )
			* (long *) key_table [i].ptr = fread_number (fp);

		else if ( key_table [i].key_type == TYPE_OBSOLETE )
			sn = fread_number (fp);

		else if ( key_table [i].key_type == TYPE_SPAWN )
			mob->mob->spawnpoint = fread_number(fp);

		else if ( key_table [i].key_type == TYPE_RESET )
			mob->mob->reset_cmd = fread_number(fp);

		else if ( key_table [i].key_type == TYPE_RESETZ )
			mob->mob->reset_zone = fread_number(fp);

		else if ( key_table [i].key_type == TYPE_STABLED ) {
			stabled = fread_number(fp);
			if ( !stable )
				continue;
			healing_time = real_time_passed (time(0) - stabled, 0);
			checks = ( healing_time.day * 12 );
			checks += ( healing_time.hour / 2 );
			for ( wound = mob->wounds; wound; wound = tmpwound ) {
				tmpwound = wound->next;
				for ( i = 0; i < checks; i++ )
						natural_healing_check(mob, wound);
			}
		}
		else if ( key_table [i].key_type == TYPE_HAS_INV )
			read_obj_suppliment (mob, fp);

		else if ( key_table [i].key_type == TYPE_SKILL ) {
			p = fread_word (fp);
			sn = skill_index_lookup (p);

			if ( sn == -1 )
				sn = skill_index_lookup (p);

			if ( sn == -1 ) {
				return NULL;
			}

			if ( sn > MAX_SKILLS )
				printf ("Skill Num # %d (learned %d) out of range.\n",
						sn, fread_number (fp));
			else
				mob->skills [sn] = fread_number (fp);
		}

		else if ( key_table [i].key_type == TYPE_OWNER ) {
			temp_arg = fread_string(fp);
			mob->mob->owner = unspace(temp_arg);
		}

		else if ( key_table [i].key_type == TYPE_AFFECT ) {

			af = (AFFECTED_TYPE *)alloc (sizeof (AFFECTED_TYPE), 13);

			fscanf (fp, "%d %d %d %d %d %d %d\n",
							&af->type,
							&af->a.spell.duration,
							&af->a.spell.modifier,
							&af->a.spell.location,
							&af->a.spell.bitvector,
							&af->a.spell.sn,
							&af->a.spell.t);
			af->next		= NULL;

			affect_to_char (mob, af);
		}

		else if ( key_table [i].key_type == TYPE_MOUNT ) {
			if ( !(tch = get_char_id (fread_number(fp))) )
				continue;
			if ( mob->mount || tch->mount )
				continue;
			tch->mount = mob;
			mob->mount = tch;
		}

        else if ( key_table [i].key_type == TYPE_WOUND ) {
			CREATE (wound, WOUND_DATA, 1);
            wound->next             = NULL;
			
			temp_arg = fread_word(fp);
			wound->location = add_hash(temp_arg);
			
			temp_arg = fread_word(fp);
			wound->type = add_hash(temp_arg);
			
			temp_arg = fread_word(fp);
			wound->severity = add_hash(temp_arg);
			
			temp_arg = fread_word(fp);
			wound->name = add_hash(temp_arg);
			wound->damage = fread_number(fp);
			wound->bleeding = fread_number(fp);
			wound->poison = fread_number(fp);
			wound->infection = fread_number(fp);
			wound->healerskill = fread_number(fp);
			wound->lasthealed = fread_number(fp);
			wound->lastbled = fread_number(fp);
					
			if ( !mob->wounds )
					mob->wounds = wound;
			else {
					tmpwound = mob->wounds;
					while ( tmpwound->next )
							tmpwound = tmpwound->next;
					tmpwound->next = wound;
			}
		}		

		else if ( key_table [i].key_type == TYPE_LODGED ) {
			lodged = (LODGED_OBJECT_INFO *)alloc ((int)sizeof (LODGED_OBJECT_INFO), 36);
			lodged->next		= NULL;

			temp_arg = fread_word(fp);
			lodged->location = add_hash(temp_arg);
			lodged->vnum = fread_number(fp);

			if ( !mob->lodged )
				mob->lodged = lodged;
			else {
				tmplodged = mob->lodged;
				while ( tmplodged->next )
					tmplodged = tmplodged->next;
				tmplodged->next = lodged;
			}

		}

		else if ( key_table [i].key_type == TYPE_DONE ) {

			REMOVE_BIT (mob->flags, FLAG_ENTERING);
			REMOVE_BIT (mob->flags, FLAG_LEAVING);

			fix_offense (mob);

			if ( get_affect (mob, MAGIC_AFFECT_SLEEP) )
				GET_POS (mob) = SLEEP;

			mob->time.logon = time (0);

			return mob;
		}

		else if ( key_table [i].key_type == TYPE_END ) {
                        p = mob->clans;
                        p2 = p;
                        mob->clans = str_dup ("");

                        while ( 1 ) {

                                p = one_argument (p, buf);              /* flags     */
                                p = one_argument (p, buf2);             /* clan name */

                                if ( !*buf2 )
                                        break;

                                add_clan_id (mob, buf2, buf);
                        }

                        mem_free (p2);
                       
			return mob;
		}
	}

	free_char (mob);

	return NULL;
}

void save_mobile (CHAR_DATA *mob, FILE *fp, char *save_reason, int extract)
{
	AFFECTED_TYPE	*af = NULL;
	int				i = 0;
	WOUND_DATA		*wound = NULL;
	LODGED_OBJECT_INFO	*lodged = NULL;

	if ( !IS_NPC (mob) )
		return;

	fprintf (fp, "%d %s\n", mob->mob->virtual, save_reason);

	fprintf (fp, "Room             %d\n", mob->in_room);
	fprintf (fp, "SDesc            %s~\n", mob->short_descr);
	fprintf (fp, "LDesc            %s~\n", mob->long_descr);
	fprintf (fp, "Keys             %s~\n", mob->name);
	fprintf (fp, "Fightmode        %d\n", mob->fight_mode);
	fprintf (fp, "Clans            %s~\n", mob->clans);
	fprintf (fp, "ColdloadId       %d\n", mob->coldload_id);
	fprintf (fp, "Flags            %d\n", mob->flags);
	fprintf (fp, "Moves            %d\n", mob->move);
	fprintf (fp, "Hits             %d\n", mob->hit);
	fprintf (fp, "Speaks           %d\n", mob->speaks);
	fprintf (fp, "AffectedBy       %ld\n", mob->affected_by);
	fprintf (fp, "Position         %d\n", mob->position);
	fprintf (fp, "Act              %ld\n", mob->act);
	fprintf (fp, "Cond0            %d\n", mob->intoxication);
	fprintf (fp, "Cond1            %d\n", mob->hunger);
	fprintf (fp, "Cond2            %d\n", mob->thirst);
	fprintf (fp, "Sex              %d\n", mob->sex);
	fprintf (fp, "Height           %d\n", mob->height);
	fprintf (fp, "Frame            %d\n", mob->frame);
	fprintf (fp, "AccessFlags      %d\n", mob->mob->access_flags);
	fprintf (fp, "SpawnLocation    %d\n", mob->mob->spawnpoint);
	fprintf (fp, "ResetCmd         %d\n", mob->mob->reset_cmd);
	fprintf (fp, "ResetZone        %d\n", mob->mob->reset_zone);

	if ( IS_RIDEE (mob) && IS_NPC (mob->mount) )
		fprintf (fp, "Rider            %d\n", mob->mount->coldload_id);

	if ( IS_RIDER (mob) && IS_NPC (mob->mount) )
		fprintf (fp, "Mount            %d\n", mob->mount->coldload_id);

	for ( i = 1; i <= LAST_SKILL; i++ ) {
		if ( mob->skills [i] )
			fprintf (fp, "Skill %s %d\n", skill_data[i].skill_name, mob->skills[i]);
	}

	if ( mob->mob->owner )
		fprintf (fp, "Owner            %s~\n", mob->mob->owner);

    for ( af = mob->hour_affects; af; af = af->next )
        if ( (af->type < MAGIC_CLAN_MEMBER_BASE ||
              af->type > MAGIC_CLAN_OMNI_BASE + MAX_CLANS) &&
			 af->type != MAGIC_CLAN_NOTIFY &&
             af->type != MAGIC_NOTIFY &&
             af->type != MAGIC_WATCH1 &&
             af->type != MAGIC_WATCH2 &&
             af->type != MAGIC_WATCH3 &&
			 af->type != MAGIC_GUARD )
            fprintf (fp, "Affect       %d %d %d %d %d %d %d\n",
                        af->type, 
						af->a.spell.duration,
						af->a.spell.modifier,
						af->a.spell.location,
                        af->a.spell.bitvector,
						af->a.spell.sn,
						af->a.spell.t);

        for ( wound = mob->wounds; wound; wound = wound->next ){
        	fprintf (fp, "Wound        %s %s %s %s %d %d %d %d %d %d %d\n",
            	wound->location,
				wound->type,
				wound->severity,
				wound->name,
				wound->damage,
				wound->bleeding,
				wound->poison,
				wound->infection,
				wound->healerskill,
				wound->lasthealed,
				wound->lastbled);
        }


	for ( lodged = mob->lodged; lodged; lodged = lodged->next )
		fprintf (fp, "Lodged        %s %d\n", lodged->location, lodged->vnum);

	if ( mob->equip || mob->right_hand || mob->left_hand ) {
		fprintf (fp, "HasInventory\n");
		write_obj_suppliment (mob, fp);
	}

	fprintf (fp, "Done\n");

	if ( IS_HITCHER (mob) )
		save_mobile (mob->hitchee, fp, "HITCH", extract);
	else
		fprintf (fp, "End\n");

	if ( extract )
		extract_char (mob);
	
	return;
}

void save_attached_mobiles (CHAR_DATA *ch, int extract)
{
	FILE		*fp = NULL;
	char		save_name [MAX_STRING_LENGTH] = {'\0'};

	snprintf (save_name, MAX_STRING_LENGTH, "save/player/%c/%s.a", tolower (*ch->tname), ch->tname);

	if ( !(fp = fopen (save_name, "w")) ) {
		return;
	}

	if ( !IS_HITCHER (ch) && !IS_RIDER (ch) )
		fprintf (fp, "end\n");

	if ( IS_HITCHER (ch) )
		save_mobile (ch->hitchee, fp, "HITCH", extract);

	if ( IS_RIDER (ch) )
		save_mobile (ch->mount, fp, "RIDE", extract);

	fclose (fp);
	return;
}
