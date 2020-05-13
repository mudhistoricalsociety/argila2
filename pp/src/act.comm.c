/** 
*	\file act.comm.c
*	Provides services for verbal and non-verbal communication between players
*
*	The purpose of this module is to provide services for speech in various
*	modes from whispers to shouts, as well as personal, object, vocal and travel
*	emotes.
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"

/* extern variables */

extern ROOM_DATA *world;
extern DESCRIPTOR_DATA *descriptor_list;
extern struct lang_info speak_tab [];
extern struct lang_info script_tab [];

#define TOK_WORD		0
#define	TOK_NEWLINE		1
#define TOK_PARAGRAPH	2
#define TOK_END			3
#define TOK_SENTENCE	4
#define VOICE_RESET "normal"	/* Users use this to unset their voices*/

void reformat_say_string (char *source, char **target)
{
	int		token_value = 0;
	int		first_line = 1;
	int		line_len = 0;
	char	*sourc = NULL;
	char	token [MAX_STRING_LENGTH] = {'\0'};
	char	result [MAX_STRING_LENGTH] = {'\0'};

	sourc = source;

	line_len = 0;

	while ( token_value != TOK_END ) {

		token_value = get_token (&sourc, token);

		if ( token_value == TOK_PARAGRAPH ) {

			if ( first_line )
				first_line = 0;
			else
				strcat (result, "\n");

			strcat (result, "   ");
			line_len = 3;
			continue;
		}

		if ( token_value == TOK_NEWLINE ) {
			if ( line_len != 0 )
				strcat (result, "\n");		/* Catch up */
			strcat (result, "\n");
			line_len = 0;
			continue;
		}

		if ( token_value == TOK_WORD ) {
			if ( line_len + strlen (token) > 72 ) {
				strcat (result, "\n    ");
				line_len = 0;
			}

			strcat (result, token);
			strcat (result, " ");
			line_len += strlen (token) + 1;
		}
	}

	result [strlen(result) - 1] = '\0';

	if ( result [strlen(result) - 1] != '.' &&
             result [strlen(result) - 1] != '!' &&
             result [strlen(result) - 1] != '?' )
		result [strlen(result)] = '.';

	*target = str_dup (result);
}

void do_ooc (CHAR_DATA *ch, char *argument, int cmd)
{
        int i = 0;
        char buf[MAX_STRING_LENGTH] = { '\0' };
        

	if ( ch->room->virtual == AMPITHEATRE && IS_MORTAL(ch) ) {
		if ( !get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
			!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand) ) {
			send_to_char ("You decide against making a commotion. PETITION to request to speak.\n", ch);
			return;
		}
	}

        for (i = 0; *(argument + i) == ' '; i++);

        if ( (argument + i) == NULL )
                send_to_char ("Surely you can think of more to say than that!\n", ch);

        else {
		CAP (argument);
		reformat_say_string (argument, &argument);
                snprintf (buf, MAX_STRING_LENGTH,  "$n says, out of character,\n   \"%s\"", argument + i);
                act (buf, FALSE, ch, 0, 0, TO_ROOM);
                snprintf (buf, MAX_STRING_LENGTH, "You say, out of character,\n   \"%s\"\n", argument + i);
                send_to_char (buf, ch);
        }
}

void do_pmote(CHAR_DATA *ch, char *argument, int cmd)
{

	char 	buf [MAX_STRING_LENGTH] = { '\0' };
	char 	*temp_arg = NULL;
	
	if (!ch)
		return;
		
	while(isspace(*argument))
	    argument++;

	

	if(!*argument) {
	    snprintf (buf, MAX_STRING_LENGTH,   "Your current pmote: (#2%s#0)\n", 
				(ch->pmote_str) ? ch->pmote_str : "#2none#0");
	    send_to_char(buf, ch);
	} 
	
    else if(!strcmp(argument, "normal")) {
    	snprintf (buf, MAX_STRING_LENGTH,   "Your current pmote has been cleared.");
        act(buf,FALSE,ch,0,0,TO_CHAR);
	    clear_pmote(ch);
	}

	else {
		temp_arg = char_short(ch);
		
    	if ( *argument == '\'' )
        	snprintf (buf, MAX_STRING_LENGTH,   "You pmote: %s%s", temp_arg, argument);
        
        else
        	snprintf (buf, MAX_STRING_LENGTH,  "You pmote: %s %s", temp_arg, argument);

		ch->pmote_str = add_hash(argument);

        act(buf,FALSE,ch,0,0,TO_CHAR | TO_ACT_FORMAT);
    }
	
	return;
}

void do_omote(CHAR_DATA *ch, char *argument, int cmd)
{

	char 		buf [MAX_STRING_LENGTH] = { '\0' };
	char 		arg1 [MAX_STRING_LENGTH] = { '\0' };
	OBJ_DATA 	*obj = NULL;

	argument = one_argument (argument, arg1);

	if(arg1 == NULL){
	    send_to_char("What would you like to omote on?\n", ch);
	    return;
	}

    if(!(obj = get_obj_in_list_vis (ch, arg1, ch->room->contents))) {
    	send_to_char("You don't see that here.\n", ch);
        return;
	}

	if (!CAN_WEAR (obj, ITEM_TAKE) && IS_MORTAL(ch) ) {
	    send_to_char("You can't omote on that.\n", ch);
	    return;
	}

	if ( IS_SET (ch->room->room_flags, OOC) ) {
		send_to_char ("You can't do this in an OOC area.\n", ch);
		return;
	}

	if(argument == NULL) {
	    send_to_char("What will you omote?\n", ch);
	    return;
	}

	obj->omote_str = add_hash(argument);

	snprintf (buf, MAX_STRING_LENGTH,   "You omote: %s %s", obj->short_description,
		obj->omote_str);


        act(buf,FALSE,ch,0,0,TO_CHAR | TO_ACT_FORMAT);
    
}

void do_think(CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*tch = NULL;
	char		*p = NULL;
	char 		buf[MAX_STRING_LENGTH] = {'\0'};
	char 		buf1[MAX_STRING_LENGTH] = {'\0'};
	char 		buf2[MAX_STRING_LENGTH] = {'\0'};
	char 		*temp_arg = NULL;
	
	while (isspace(*argument))
	    argument++;
		
	if(argument == NULL) {
    	send_to_char("What would you like to think?\n", ch);
		return;
	}

    if ( IS_SET (ch->plr_flags, NOPETITION) ) {
    	act ("Your ability to petition/think has been revoked by an admin.", FALSE, ch, 0, 0, TO_CHAR);
        return;
	}

	temp_arg = char_short(ch);
	
	snprintf (buf, MAX_STRING_LENGTH,   "You thought: %s", argument);
	snprintf (buf1, MAX_STRING_LENGTH,  "%s thinks, \"%s\"", GET_NAME(ch), argument);

	reformat_say_string (argument, &p);
	snprintf (buf2, MAX_STRING_LENGTH,  "#6You hear #3%s#6 think,\n   \"%s\"#0\n", temp_arg, p);
	mem_free (p);
	
	act (buf, FALSE, ch, 0, 0, TO_CHAR | TO_ACT_FORMAT);

	for ( tch = character_list; tch; tch = tch->next ) {
		if ( tch->deleted )
			continue;
		if ( IS_MORTAL(tch) )
			continue;
		if ( tch == ch )
			continue;
		if ( !IS_SET (tch->flags, FLAG_TELEPATH) )
			continue;
		act (buf1, FALSE, tch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	}

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
		if ( !IS_MORTAL(ch) )
			continue;
		if ( tch == ch )
			continue;
		if ( !IS_MORTAL(tch) )
			continue;
		if ( skill_use (tch, SKILL_TELEPATHY, ch->skills[SKILL_TELEPATHY]/3) )
			send_to_char (buf2, tch);
	}
	
	return;
}

void personalize_string (CHAR_DATA *src, CHAR_DATA *tar, char *emote)
{
	char		desc [MAX_STRING_LENGTH] = { '\0' };
	char		output [MAX_STRING_LENGTH] = { '\0' };
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	CHAR_DATA	*tch = NULL, *target = NULL;
	char		*charshortbuf = NULL;

	*output = '\0';

	while ( *emote ) {
		*desc = '\0';
		if ( *emote == '#' ) {
			emote++;
			if ( *emote == '5' ) {
				emote++;
				while ( *emote != '#' ) {
					snprintf (desc + strlen(desc), MAX_STRING_LENGTH, "%c", *emote);
					emote++;					
				}

/** find first character in the room matching desc **/		
				for ( tch = tar->room->people; tch; tch = tch->next_in_room )
					charshortbuf = char_short(tch);
					if ( !str_cmp (charshortbuf, desc) ) {
						break;
					}
			
				emote++;
				emote++;

				if ( *emote == '\'' ) {
					strcat (desc, "\'");
					emote++;

					if ( *emote == 's' ) {
						strcat (desc, "s");
					}
					else {
						emote--;
					}
				}
				else {
					emote--;
					emote--;
				}
				if ( !tch )
					continue;
				if ( tch == tar ) {
					snprintf (buf, MAX_STRING_LENGTH,   "%c", desc[strlen(desc)-1]);

					if ( desc[strlen(desc)-1] == '\'' || desc[strlen(desc)-2] == '\'' ) {
						strcat (output, "#5your#0");
						emote--;
					}
					else{
						strcat (output, "#5you#0");
					}
					
					target = tch;
					emote++;
				}
				else {
					charshortbuf = char_short(tch);
					snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0", charshortbuf);
					strcat (output, buf);
					emote--;
		
					if ( *emote == '\'' )
						emote--;
				}
			}
			else{
				snprintf (output + strlen(output), MAX_STRING_LENGTH,  "#%c", *emote);
			}
		}
		else {
			snprintf (output + strlen(output), MAX_STRING_LENGTH,  "%c", *emote);
		}	
		emote++;
	}
	
	if ( target ) {
		if ( *output == '#' )
			output[2] = toupper (output[2]);
		act (output, FALSE, src, 0, target, TO_VICT | TO_ACT_FORMAT);
		magic_add_affect (target, MAGIC_SENT, -1, 0, 0, 0, 0);	
	}
	
	return;
}

void personalize_emote (CHAR_DATA *src, char *emote)
{
	char		desc [MAX_STRING_LENGTH] = { '\0' };
	char		copy [MAX_STRING_LENGTH] = { '\0' };
	char		*charshortbuf = NULL;
	CHAR_DATA	*tch = NULL;
	AFFECTED_TYPE *af_targ = NULL;
	
	snprintf (copy, MAX_STRING_LENGTH, "%s", emote);

	while ( *emote ) {
		*desc = '\0';
		if ( *emote == '#' ) {
			emote++;
			if ( *emote == '5' ) {
				emote++;
				while ( *emote != '#' ) {
					snprintf (desc + strlen(desc), MAX_STRING_LENGTH, "%c", *emote);
					emote++;
				}
				
				tch = get_char_room_vis (src, desc);
				
/** Find the first person who matches the description **/
				for ( tch = src->room->people; tch; tch = tch->next_in_room ) {
					charshortbuf = (char_short(tch));
					if ( !str_cmp (charshortbuf, desc) )
						break;
					
					if ( !tch )
						continue;
						
					if ( !get_affect (tch, MAGIC_SENT) ){
						personalize_string (src, tch, copy);
					}	
				}
			} /* end emote == '5'*/
		}/* end emote == '#'*/
		emote++;
	}

	for ( tch = src->room->people; tch; tch = tch->next_in_room ) {
		if ( tch == src )
			continue;
		if ( get_affect (tch, MAGIC_SENT) )
			continue;
		act (copy, TRUE, tch, 0, 0, TO_CHAR | TO_ACT_FORMAT);
	}

	for ( tch = src->room->people; tch; tch = tch->next_in_room ) {
		af_targ = get_affect (tch, MAGIC_SENT);
		if ( af_targ )
			affect_remove (tch, af_targ);
	}
	
	free(af_targ);
	return;
}

void do_emote(CHAR_DATA *ch, char *argument, int cmd)
{
	char 		buf [MAX_STRING_LENGTH] = { '\0' };
	char 		copy[MAX_STRING_LENGTH] = { '\0' };
	char 		key [MAX_STRING_LENGTH] = { '\0' };
	bool 		tochar = FALSE;
	bool 		is_imote = FALSE;
	OBJ_DATA	*obj = NULL;
	int 		key_e = 0;
	char 		*p = NULL;
	char		*char_desc = NULL;
	char		*obj_desc = NULL;
	CHAR_DATA	*char_vis = NULL;

	while (isspace(*argument))
	    argument++;
	
	if ( ch->room->virtual == AMPITHEATRE && IS_MORTAL(ch) ) {
		if ( !get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
			!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand) ) {
			send_to_char ("You decide against making a commotion. PETITION to request to speak.\n", ch);
			return;
		}
	}

	if ( strstr (argument, "\"") ) {
		send_to_char ("Conveying speech via emote is considered code abuse here. Please use SAY instead.\n", ch);
		return;
	}

	if (argument == NULL)
		send_to_char("What would you like to emote?\n", ch);
	else {
		p = copy;
		while(*argument) {

			if ( *argument == '@' ) {
				is_imote = TRUE;
				char_desc = char_short(ch);
				snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", char_desc);
				p += strlen(p);
				argument++;
			}

		    if(*argument == '*') {

				argument++;
				while(*argument>='0' && *argument<='9'){ 
					key[key_e++] = *(argument++); 
 				}
 			
 				if(*argument=='.'){ 
					key[key_e++] = *(argument++); 
 				}
 			
 				while(isalpha(*argument) || *argument=='-') { 
					key[key_e++] = *(argument++); 
 				}
 			
	        	key[key_e] = '\0';
				key_e = 0;

				if (!get_obj_in_list_vis(ch, key, ch->room->contents) &&
			    	!get_obj_in_list_vis(ch, key, ch->right_hand) &&
			    	!get_obj_in_list_vis (ch, key, ch->left_hand) &&
			    	!get_obj_in_list_vis(ch, key, ch->equip) ) {
              		snprintf (buf, MAX_STRING_LENGTH,  "I don't see %s here.\n",key); 
              		send_to_char(buf,ch);
              		return; 
          		}

				obj = get_obj_in_list_vis (ch, key, ch->right_hand); 
          
          		if ( !obj ) 
             		obj = get_obj_in_list_vis (ch, key, ch->left_hand); 
          		if ( !obj ) 
             		obj = get_obj_in_list_vis (ch, key, ch->room->contents);
          		if ( !obj ) 
             		obj = get_obj_in_list_vis (ch, key, ch->equip);
             
             	obj_desc = obj_short_desc(obj);
          		snprintf (p, MAX_STRING_LENGTH,  "#2%s#0", obj_desc); 
          		p += strlen(p); 

          	} /*end if(*argument == '*')*/
          	
          	else if(*argument == '~') { 

				argument++; 

          		while(*argument>='0' && *argument<='9'){ 
          			key[key_e++] = *(argument++); 
          		}
          		
          		if(*argument=='.'){ 
            		key[key_e++] = *(argument++); 
          		} 

 				while(isalpha(*argument) || *argument=='-') { 
					key[key_e++] = *(argument++); 
 				}
 			 			
	        	key[key_e] = '\0';
				key_e = 0;

				if (!get_char_room_vis(ch,key)) {
            		snprintf (buf, MAX_STRING_LENGTH,  "Who is %s?\n",key); 
			    	send_to_char(buf,ch);
			    	return;
				}

				char_vis = get_char_room_vis (ch,key);
				if (char_vis == ch) {
					send_to_char ("You shouldn't refer to yourself using the token system.\n", ch);
					free(char_vis);
					return;
				}
				
				char_desc = char_short(char_vis);
				snprintf (p, MAX_STRING_LENGTH,  "#5%s#0", char_desc);
		    	p += strlen(p);
				tochar = TRUE;
		    } /* end if(*argument == '~')*/
		    else 
		        *(p++) = *(argument++);
		        
		}/* end while *argument*/
		
		*p = '\0';

		if ( copy [0] == '\'' ) {
			if ( !is_imote ) {
				char_desc = char_short(ch);
				snprintf (buf, MAX_STRING_LENGTH,  "#5%s#0%s", char_desc, copy);
				buf[2] = toupper (buf[2]);
			}
			else { 
            	snprintf (buf, MAX_STRING_LENGTH,   "%s", copy); 
			}
		}
		else {
			if ( !is_imote ) {
				char_desc = char_short(ch);
				snprintf (buf, MAX_STRING_LENGTH, "#5%s#0 %s", char_desc, copy);
				buf[2] = toupper (buf[2]);
			}
			else { 
             snprintf (buf, MAX_STRING_LENGTH,   "%s", copy); 
          	} 
		}

		if ( buf[strlen(buf)-1] != '.' && buf[strlen(buf)-1] != '!' && buf[strlen(buf)-1] != '?' )
			strcat (buf, ".");

		if ( !tochar )
			act(buf,FALSE,ch,0,0,TO_ROOM | TO_ACT_FORMAT);
		else
			personalize_emote (ch, buf);

		act(buf,FALSE,ch,0,0,TO_CHAR | TO_ACT_FORMAT);
	}
	
	return;
}

void reply_reset (CHAR_DATA *ch, CHAR_DATA *target, char *buf, int cmd)
{
	static int		avoid_loop = 0;
	RESET_DATA		*reset = NULL;
	char			*argument = NULL;
	char			keywords [MAX_STRING_LENGTH] = { '\0' };
	char			reply [MAX_STRING_LENGTH] = { '\0' };
	char			question [MAX_STRING_LENGTH] = { '\0' };
	char			buf2 [MAX_STRING_LENGTH] = { '\0' };

	if ( avoid_loop )	/* Don't get into infinite loops between mobs */
		return;

	for ( reset = target->mob->resets; reset; reset = reset->next ) {

		if ( reset->type != RESET_REPLY )
			continue;

		argument = one_argument (reset->command, keywords);

		while ( isspace (*argument) )
			argument++;

		strcpy (reply, argument);

		(void)one_argument (buf, question);
		
		for ( argument = one_argument (keywords, buf2);
			  *buf2;
			  argument = one_argument (argument, buf2) ) {

			if ( !str_cmp (buf2, question) ) {
				name_to_ident (ch, buf2);
				snprintf (buf2 + strlen (buf2), MAX_STRING_LENGTH, " %s", reply);
				avoid_loop = 1;

				send_to_char ("\n", ch);

				if ( cmd == 4 )
					do_whisper (target, buf2, cmd);
				else
					do_say (target, buf2, cmd);

				avoid_loop = 0;
				
				return;
			}
		}
	}
}

void do_speak (CHAR_DATA *ch, char *argument, int cmd)
{
	int			i = 0;
  	char		buf [MAX_INPUT_LENGTH] = { '\0' };
	
	argument = one_argument (argument, buf);

	for ( i = 0; speak_tab [i].skill; i++ )
		if ( !str_cmp (buf, speak_tab [i].lang) )
			break;
	
	if ( !speak_tab [i].skill ) {
		snprintf(buf, MAX_INPUT_LENGTH, "The possible choices are:\n   %s\n ", speak_tab[0].lang);
		
		for ( i = 1; speak_tab [i].skill; i++ ) {
			snprintf(ADDBUF, MAX_INPUT_LENGTH, "   %s\n", speak_tab[i].lang);
		}
		send_to_char(buf, ch);
		return;
	}
	

	if ( !real_skill (ch, speak_tab [i].skill) && !get_affect (ch, MAGIC_AFFECT_TONGUES) ) {
		snprintf (buf, MAX_STRING_LENGTH,   "You are unfamiliar with %s.\n",
					  (speak_tab [i].lang));
		send_to_char (buf, ch);
		return;
	}

	if ( ch->speaks == speak_tab [i].skill ) {
		snprintf (buf, MAX_STRING_LENGTH,   "You are already speaking %s.\n",
					  (speak_tab [i].lang));
		send_to_char (buf, ch);
		return;
	}

	ch->speaks = speak_tab [i].skill;


	snprintf (buf, MAX_STRING_LENGTH,   "You begin speaking %s.\n",
					(speak_tab [i].lang));
	send_to_char (buf, ch);
	return;
}

void do_select_script (CHAR_DATA *ch, char *argument, int cmd)
{
	int		i = 0;
  	char		buf [MAX_INPUT_LENGTH] = { '\0' };
	
	
	argument = one_argument (argument, buf);

	for ( i = 0; script_tab [i].skill; i++ )
		if ( !str_cmp (buf, script_tab [i].lang) )
			break;

	if ( !script_tab [i].skill ) {
		snprintf(buf, MAX_INPUT_LENGTH, "The possible choices are:\n   %s\n", script_tab[0].lang);
		
		for ( i = 1; script_tab [i].skill; i++ ) {
			snprintf(ADDBUF, MAX_INPUT_LENGTH, "   %s\n", script_tab[i].lang);
		}
		
		send_to_char(buf, ch);
		return;
	}

	if ( !real_skill (ch, script_tab [i].skill) && !get_affect (ch, MAGIC_AFFECT_TONGUES) ) {
		snprintf (buf, MAX_STRING_LENGTH,  "You are unfamiliar with %s.\n",
					  (script_tab [i].lang));
		send_to_char (buf, ch);
		return;
	}

	if ( ch->speaks == script_tab [i].skill ) {
		snprintf (buf, MAX_STRING_LENGTH,  "You are already writing in %s.\n",
					  (script_tab [i].lang));
		send_to_char (buf, ch);
		return;
	}

	ch->writes = script_tab [i].skill;

	snprintf (buf, MAX_STRING_LENGTH,   "You will now write in %s.\n",
					(script_tab [i].lang));

	send_to_char (buf, ch);
	return;
}

void do_mute (CHAR_DATA *ch, char *argument, int cmd)
{

    char 		buf[MAX_STRING_LENGTH] = { '\0' };
    AFFECTED_TYPE 	*af = NULL;

    if(!real_skill(ch, SKILL_LISTEN)) {
	snprintf (buf, MAX_STRING_LENGTH,   "You don't have any skill to try and overhear conversations, so you are already muted by default.\n");
	send_to_char(buf, ch);
	return;
    } 

    while(isspace(*argument))
        argument++;

    if(argument == NULL) {
	snprintf (buf, MAX_STRING_LENGTH,   "You %s listening to others' conversations.\n",
		get_affect(ch, MUTE_EAVESDROP) ? "aren't" : "are"); 
	send_to_char(buf, ch);
	return;
    }

    if(strcasecmp(argument, "on") == 0) {

	if(!get_affect(ch, MUTE_EAVESDROP)) {
            af = (AFFECTED_TYPE *)alloc(sizeof(AFFECTED_TYPE), 13);
	
	    af->type 	= MUTE_EAVESDROP;
	    af->a.listening.duration = -1;
	    af->a.listening.on = 1;

	    affect_to_char (ch, af);
	}
	snprintf (buf, MAX_STRING_LENGTH,   "You will now not listen to others' conversations.\n");
	send_to_char(buf, ch);
    } else 
	if(strcasecmp(argument, "off") == 0) {

	    if(get_affect(ch, MUTE_EAVESDROP)) {
	        remove_affect_type(ch, MUTE_EAVESDROP);
	    }
	    snprintf (buf, MAX_STRING_LENGTH,   "You will now listen to others' conversations.\n");
	    send_to_char(buf, ch);
        } else {
	    snprintf (buf, MAX_STRING_LENGTH,   "You can change your mute status by 'mute on' or 'mute off'.  To see what your mute status is use 'mute'\n");
	    send_to_char(buf, ch);
	}
}


void do_voice (CHAR_DATA *ch, char *argument, int cmd)
{

	char buf [MAX_STRING_LENGTH] = { '\0' };

    while(isspace(*argument))
    	argument++;

	if ( strchr (argument, '~') ) {
		send_to_char ("Sorry, but you can't use tildae when setting a voice string.\n", ch);
		return;
	}

    if(argument == NULL) {
        
		if ( ch->voice_str ) {
        	snprintf (buf, MAX_STRING_LENGTH,   "Your current voice string: (#2%s#0)\n",ch->voice_str);
		}
		else {
			snprintf (buf, MAX_STRING_LENGTH,   "You do not currently have a voice string set.\n");
		}
		send_to_char(buf, ch);
           
    }
    else {
	  	if(strcasecmp(argument, VOICE_RESET) == 0) {
			clear_voice(ch);
			snprintf (buf, MAX_STRING_LENGTH,   "Your voice string has been cleared.");
	    }
	   	else {
       		snprintf (buf, MAX_STRING_LENGTH,   "Your voice string has been set to: (#2%s#0)", argument);
            ch->voice_str = add_hash(argument);
	    }

        act(buf,FALSE,ch,0,0,TO_CHAR | TO_ACT_FORMAT);
    }
}

int decipher_speaking (CHAR_DATA *ch, int skillnum, int skill)
{
	int		check = 0;

	if ( skill > 0 && skill <= 15 )
		check = 70;
	else if ( skill > 15 && skill < 30 )
		check = 50;
	else if ( skill >= 30 && skill < 50 )
		check = 30;
	else if ( skill >= 50 && skill < 70 )
		check = 20;
	else if ( skill >= 70 )
		check = 10;

	skill_use (ch, skillnum, 0);

	if ( ch->skills [skillnum] >= check )
		return 1;
	else return 0;
}

char *accent_desc (int skill) 
{
	if ( skill < 10 ){
		return accent[0];
	}
	else if ( skill >= 10 && skill < 20 ){
		return accent[1];
	}
	else if ( skill >= 20 && skill < 30 ){
		return accent[2];
	}
	else if ( skill >= 30 && skill < 40 ){
		return accent[3];
	}
	else if ( skill >= 40 && skill < 50 ){
		return accent[4];
	}
	else{
		return accent[5];
	}
}

void do_say (CHAR_DATA *ch, char *argument, int cmd) 
{
	int				talked_to_another = 0,
					i = 0, 
					key_e = 0; /* index to key[]*/
					
	CHAR_DATA		*tch = NULL; /* target character index in loops*/
	CHAR_DATA		*target = NULL; /* target character */
	OBJ_DATA		*obj = NULL;
	AFFECTED_TYPE	*tongues = NULL;
	AFFECTED_TYPE	*af_table = NULL;
	
	bool			deciphered = FALSE,
					allocd = FALSE,
					voice_changed = FALSE;
					
	char			key [MAX_STRING_LENGTH]; /* person or item used in string */
	char 			buf [MAX_STRING_LENGTH]; /* to manipulate arguments */
	char 			buf2 [MAX_STRING_LENGTH]; /* to manipulate arguments */
	char			buf3 [MAX_STRING_LENGTH]; /* Staff use: names */
	char 			buf4 [MAX_STRING_LENGTH]; /* Message, sent to the player */
	char			buf5 [MAX_STRING_LENGTH]; /* used to decipher languages */
	char			target_key [MAX_STRING_LENGTH];
	char			voice [MAX_STRING_LENGTH];
	char			argbuf [MAX_STRING_LENGTH];
	char			*char_desc = NULL;
	char			*obj_desc = NULL;
	CHAR_DATA		*char_vis = NULL;
	char			*acc_desc = NULL;
	
	char			*utters [] = { "say", "sing", "tell", "murmur",
								   "wouldbewhisper" };
								   
/** cmd == 0  Talk, not at table, or Say **/
/** cmd == 1  Singing **/
/** cmd == 2  Tell **/
/** cmd == 3  Talk, at table **/

	*key = '\0';
	*target_key = '\0';
	*buf = '\0';
	*buf2 = '\0';
	*buf3 = '\0';
	*buf4 = '\0';
	*buf5 = '\0';
	*voice = '\0';
	*argbuf = '\0';

	if ( ch->room->sector_type == SECT_UNDERWATER ) {
		send_to_char ("You can't do that underwater!\n", ch);
		return;
	}

	strcpy (argbuf, argument);		/* We modify *argument, make sure we don't*/
	argument = argbuf;              /*  have a problem with const arguments   */

	while ( isspace (*argument) )
		argument++;

	if ( ch->room->virtual == AMPITHEATRE && IS_MORTAL(ch) ) {
		if ( !get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
			!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand) ) {
			send_to_char ("You decide against speaking out of turn. PETITION to request to speak.\n", ch);
			return;
		}
	}

	if ( argument == NULL ) {
		send_to_char ("What would you like to say?\n", ch);
		return;
	}

	if ( cmd == 2 ) { /** Tell, select target **/
		*target_key = '\0';
		argument = one_argument (argument, target_key);
	}

	if ( *argument == '(' ) {
		*voice = '\0';
		if ( ch->voice_str ) {
			snprintf (voice, MAX_STRING_LENGTH, "%s", ch->voice_str);
			clear_voice (ch);
		}
		*buf = '\0';
		snprintf (buf, MAX_STRING_LENGTH,  "%s", argument);
		i = 1;
		*buf2 = '\0';
		
		while ( buf[i] != ')' ) {
			if ( buf[i] == '\0' ) {
				send_to_char ("What did you wish to say?\n", ch);
				return;
			}
			
			if ( buf[i] == '*' ) {
				i++;
				
				while(buf[i]>='0' && buf[i]<='9'){ 
				key[key_e++] = buf[i++]; 
 				} 
 
 				if(buf[i] =='.'){ 
				key[key_e++] = buf[i++]; 
 				}
 				
 				while(isalpha(buf[i]) || buf[i] =='-') { 
				key[key_e++] = buf[i++]; 
 				}
 				
				key[key_e] = '\0';
				key_e = 0;

				if ( !get_obj_in_list_vis(ch, key, ch->room->contents) &&
          	    	 !get_obj_in_list_vis(ch, key, ch->right_hand) &&
          	         !get_obj_in_list_vis(ch, key, ch->left_hand) &&
                     !get_obj_in_list_vis(ch, key, ch->equip) ) {
                    	snprintf (buf, MAX_STRING_LENGTH, "I don't see %s here.\n",key);
                        send_to_char(buf,ch);
                        return;
                }

				obj = get_obj_in_list_vis (ch, key, ch->right_hand);
				if ( !obj )
					obj = get_obj_in_list_vis (ch, key, ch->left_hand);
				if ( !obj )
					obj = get_obj_in_list_vis (ch, key, ch->room->contents);
				if ( !obj )
					obj = get_obj_in_list_vis (ch, key, ch->equip);
				
				obj_desc = obj_short_desc(obj);
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "#2%s#0", obj_desc);
				*key = '\0';
				continue;
			}
			
			if ( buf[i] == '~' ) {
				i++;
				
				while(buf[i]>='0' && buf[i]<='9'){ 
				key[key_e++] = buf[i++]; 
 				}
 				
 				if(buf[i]=='.'){ 
				key[key_e++] = buf[i++]; 
 				}
 				
 				while(isalpha(buf[i]) || buf[i]=='-') { 
				key[key_e++] = buf[i++]; 
 				}
 				
				key[key_e] = '\0';
				key_e = 0;

				if ( !get_char_room_vis (ch, key)) {
					snprintf (buf, MAX_STRING_LENGTH,  "I don't see %s here.\n", key);
					send_to_char (buf, ch);
					return;
				}
				char_vis = get_char_room_vis(ch, key);
				char_desc = char_short(char_vis);
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "#5%s#0", char_desc);
				*key = '\0';
				continue;
			}
			snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%c", buf[i]);
			i++;
		}
		ch->voice_str = str_dup(buf2);
		
		while ( *argument != ')' )
			argument++;
		argument += 2;
		voice_changed = TRUE;
		i = 0;
		*buf = '\0';
		
		if ( cmd == 2 && *target_key )
			snprintf (buf, MAX_STRING_LENGTH,  "%s %s", target_key, argument);
		else snprintf (buf, MAX_STRING_LENGTH,  "%s", argument);
        *argument = '\0';
        argument = buf;
        
        if ( argument == NULL ) {
            send_to_char ("What did you wish to say?\n", ch);
            if ( voice_changed ) {
                clear_voice (ch);
                if ( *voice ) {
                    ch->voice_str = str_dup(voice);
                }
            }
            return;
        }
	}
	else if ( cmd == 2 && *target_key ) {
		snprintf (buf, MAX_STRING_LENGTH,   "%s %s", target_key, argument);
		snprintf (argument, MAX_STRING_LENGTH,  "%s", buf);
	}

	if ( argument == NULL ) {
		send_to_char ("What did you wish to say?\n", ch);
	    if ( voice_changed ) {
            clear_voice (ch);
           	if ( *voice ) {
	 			ch->voice_str = str_dup(voice);
	 		}
        }
		return;
	}

	if ( cmd == 2 ) {		/* Tell message */

		argument = one_argument (argument, buf);
		reformat_say_string (argument, &argument);

		if ( argument == NULL ) {
			send_to_char ("What did you wish to tell?\n", ch);
		    if ( voice_changed ) {
       	    	clear_voice (ch);
       	       			if ( *voice )
		 			ch->voice_str = str_dup(voice);
       		}
			return;
		}

		if ( !(target = get_char_room_vis (ch, buf)) ) {
			send_to_char ("Tell who?\n", ch);
	        if ( voice_changed ) {
            	clear_voice (ch);
				if ( *voice ) {
                	ch->voice_str = str_dup(voice);
                }
        	}
        	return;
		}

		if ( target == ch ) {
			send_to_char ("You want to tell yourself?\n", ch);
	        if ( voice_changed ) {
               	clear_voice (ch);
				if ( *voice ) {
           			ch->voice_str = str_dup(voice);
        		}
        	}
        	return;
		}

		while ( isspace (*argument) )
			argument++;
	}
	else reformat_say_string (argument, &argument);

	if ( argument == NULL ) {
		if ( cmd == 1 ) { /** Singing **/
			send_to_char ("What are the words to the song?\n", ch);
		}
		else if ( cmd == 2 ) {
			send_to_char ("What would you like to tell?\n", ch);
		}
		else
			send_to_char ("What would you like to say?\n", ch);
        	if ( voice_changed ) {
               	clear_voice (ch);
				if ( *voice ) { 
           			ch->voice_str = str_dup(voice);
           		}
       		}
		return;
	}

	tongues = get_affect (ch, MAGIC_AFFECT_TONGUES);

	if ( cmd == 3 ) {
		af_table = get_affect (ch, MAGIC_SIT_TABLE);
	}
	
	if ( !tongues && !real_skill (ch, ch->speaks) ) {
		send_to_char ("You can't even make a guess at the language you want to speak.\n", ch);
	    if ( voice_changed ) {
            clear_voice (ch);
			if ( *voice ) {
           		ch->voice_str = str_dup(voice);
        	}
		return;
		}
	}
	
	snprintf (buf4, MAX_STRING_LENGTH, "%s",  argument);	/* Intended message, sent to the player.*/
	snprintf (buf5, MAX_STRING_LENGTH, "%s",  argument);	/* manipulate arguments */
	snprintf (buf2, MAX_STRING_LENGTH, "%s",  argument);	/* manipulate arguments */

	if ( cmd == 0 ) {
		if ( buf4 [strlen(buf4)-1] == '?' ) {
			utters [cmd] = str_dup("ask");
			allocd = TRUE;
		}
		else if ( buf4 [strlen(buf4)-1] == '!' ) {
			utters [cmd] = str_dup("exclaim");
			allocd = TRUE;
		}
	}
	
	else if ( cmd == 2 ) {
		if ( buf4 [strlen(buf4)-1] == '?' ) {
			utters [cmd] = str_dup("ask");
			allocd = TRUE;
		}
		else if ( buf4 [strlen(buf4)-1] == '!' ) {
			utters [cmd] = str_dup("emphatically tell");
			allocd = TRUE;
		}
	}

	skill_use (ch, ch->speaks, 0);

	for ( tch = vtor (ch->in_room)->people; tch; tch = tch->next_in_room ) {

		if ( tch == ch )	/* Don't say it to ourselves */
			continue;

		if ( !tch->desc )
			continue;

		if ( af_table && !is_at_table (tch, af_table->a.table.obj) ) {

			/* If the guy is muting, punt */
			if(get_affect(tch, MUTE_EAVESDROP))
				continue;

			snprintf (buf2, MAX_STRING_LENGTH,  "%s",  argument);

			if ( !tch->skills [ch->speaks] && !tongues )
				continue;

			if ( !whisper_it (tch, SKILL_LISTEN, buf2, buf2) )
				continue;

			if ( decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]) || IS_SET(ch->room->room_flags, OOC) ) {
				if ( !IS_SET (ch->room->room_flags, OOC) ) {
					snprintf (buf, MAX_STRING_LENGTH,  "You overhear $N say in %s,", skill_data[ch->speaks].skill_name);
					
					if ( tch->skills [ch->speaks] >= 50 && ch->skills[ch->speaks] < 50 ){
						acc_desc = accent_desc(ch->skills[ch->speaks]);
						snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
					}
					
				}
				else {
					snprintf (buf, MAX_STRING_LENGTH,  "You overhear $N say,");
				}
				if (ch->voice_str)
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
				deciphered = TRUE;
			}
			else {
				snprintf (buf, MAX_STRING_LENGTH,  "You overhear $N say something in %s,", skill_data[ch->speaks].skill_name);
				if ( tch->skills [ch->speaks] >= 50 && ch->skills[ch->speaks] < 50 ){
					acc_desc = accent_desc(ch->skills[ch->speaks]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
				}
				
				if (ch->voice_str)
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " but you are unable to decipher %s words.", HSHR(ch));
				deciphered = FALSE;
			}				
			
			act(buf,FALSE,tch,0,ch,TO_CHAR | TO_ACT_FORMAT);

			if ( tch->desc && deciphered ) {
				*buf2 = toupper (*buf2);
				snprintf (buf, MAX_STRING_LENGTH,   "   \"%s\"\n", buf2);
				send_to_char (buf, tch);
			}

			continue;
		}

		if ( tch->desc )
			talked_to_another = 1;

		if ( GET_TRUST (tch) && !IS_NPC (tch) && GET_FLAG (tch, FLAG_SEE_NAME) )
			snprintf (buf3, MAX_STRING_LENGTH,  " (%s)", GET_NAME (ch));
		else
			*buf3 = '\0';

		if ( cmd == 0 || cmd == 1 ) {
			if ( !IS_SET (ch->room->room_flags, OOC) && decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss in %s,", buf3, utters [cmd],
					(tch->skills [ch->speaks] || tongues) ? skill_data[ch->speaks].skill_name :
					   "an unknown tongue");
				if ( tch->skills [ch->speaks] >= 50 && ch->skills[ch->speaks] < 50 ){
					acc_desc = accent_desc(ch->skills[ch->speaks]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
				}
				
				deciphered = TRUE;
				if ( ch->voice_str )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
			}
			else if ( !IS_SET (ch->room->room_flags, OOC) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss something in %s,", buf3, utters [cmd],
					(tch->skills [ch->speaks] || tongues) ? skill_data[ch->speaks].skill_name:
					   "an unknown tongue");
				if ( ch->voice_str )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " but you are unable to decipher %s words.", HSHR(ch));
				deciphered = FALSE;
			}
			else if ( IS_SET (ch->room->room_flags, OOC) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss,", buf3, utters [cmd]);
				if ( ch->voice_str )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
				deciphered = TRUE;
			}
		}

		else if ( cmd == 2 ) {
			if ( tch == target ) {
				if ( !IS_SET (ch->room->room_flags, OOC) && decipher_speaking(tch, ch->speaks, ch->skills[ch->speaks]) ) {
					snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss you in %s,", buf3,
						utters [cmd], (tch->skills [ch->speaks] || tongues) ?
							skill_data[ch->speaks].skill_name : "an unknown tongue");
					deciphered = TRUE;
					if ( tch->skills [ch->speaks] >= 50 && ch->skills[ch->speaks] < 50 ){
						acc_desc = accent_desc(ch->skills[ch->speaks]);
						snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
					}
					
					if ( ch->voice_str )
						snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
				}
				else if ( !IS_SET (ch->room->room_flags, OOC) ) {
					snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss you something in %s,", buf3,
						utters [cmd], (tch->skills [ch->speaks] || tongues) ?
							skill_data[ch->speaks].skill_name : "an unknown tongue");
					if ( ch->voice_str )
						snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " but you are unable to decipher %s words.", HSHR(ch));
					deciphered = FALSE;
				}
				else {
					snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss you,", buf3, utters [cmd]);
					deciphered = TRUE;
				}
			}
			else {
				if ( !IS_SET (ch->room->room_flags, OOC) && decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]) ) {
					char_desc = char_short(target);
					snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss %s in %s,", buf3, utters [cmd], char_desc, (tch->skills [ch->speaks] || tongues) ?skill_data[ch->speaks].skill_name : "an unknown tongue");
					deciphered = TRUE;
					
					if ( tch->skills [ch->speaks] >= 50 && ch->skills[ch->speaks] < 50 ){
						acc_desc = accent_desc(ch->skills[ch->speaks]);
						snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
					}
					
					if ( ch->voice_str )
						snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
				}
				else if ( !IS_SET (ch->room->room_flags, OOC) ) {
					char_desc = char_short (target);
					snprintf (buf, MAX_STRING_LENGTH,   "$N%s %ss %s something in %s,", buf3, utters [cmd], char_desc, (tch->skills [ch->speaks] || tongues) ? skill_data[ch->speaks].skill_name : "an unknown tongue");
					
					if ( ch->voice_str )
						snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " but you are unable to decipher %s words.", HSHR(ch));
					deciphered = FALSE;
				}
				else {
					char_desc = char_short (target);
					snprintf (buf, MAX_STRING_LENGTH,   "$N%s %ss %s,", buf3, utters [cmd], char_desc);
					if ( ch->voice_str )
						snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
					deciphered = TRUE;
				}
			}
		}

		else if ( cmd == 3 ) {
			if ( !IS_SET (ch->room->room_flags, OOC) && decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss in %s,", buf3,
						utters [cmd],
						(tch->skills [ch->speaks] || tongues) ?
                        	skill_data[ch->speaks].skill_name : "an unknown tongue");
				deciphered = TRUE;
				if ( tch->skills [ch->speaks] >= 50 && ch->skills[ch->speaks] < 50 ){
					acc_desc = accent_desc(ch->skills[ch->speaks]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
				}
				
				if ( ch->voice_str )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
			}
			else if ( !IS_SET (ch->room->room_flags, OOC) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss something in %s,", buf3,
						utters [cmd],
						(tch->skills [ch->speaks] || tongues) ?
                        	skill_data[ch->speaks].skill_name : "an unknown tongue");
				if ( ch->voice_str )
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", ch->voice_str);
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " but you are unable to decipher %s words.", HSHR(ch));
				deciphered = FALSE;
			}
			else {
				snprintf (buf, MAX_STRING_LENGTH,  "$N%s %ss,", buf3, utters[cmd]);
				deciphered = TRUE;
			}
		}

		act (buf, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);

		if ( tch->desc && deciphered ) {
			*buf4 = toupper (*buf4);
			snprintf (buf, MAX_STRING_LENGTH,   "   \"%s\"\n", buf4);
		        send_to_char (buf, tch);
		}

		snprintf (argument, MAX_STRING_LENGTH,  "%s", buf5);
		deciphered = FALSE;
	}

	*buf4 = toupper(*buf4);

	if ( cmd == 2 ) {
		if(ch->voice_str) {
			if ( !IS_SET (ch->room->room_flags, OOC) ){
				char_desc = char_short (target);
		    	snprintf (buf, MAX_STRING_LENGTH,  "You %s #5%s#0 in %s, %s,\n   \"%s\"\n", utters [cmd], char_desc,skill_data[ch->speaks].skill_name, ch->voice_str, buf4);
			}
			else{
				char_desc = char_short (target);
				snprintf (buf, MAX_STRING_LENGTH,  "You %s #5%s#0, %s,\n   \"%s\"\n", utters [cmd], char_desc,  ch->voice_str, buf4);
			}			
		}
		else {
			if ( !IS_SET (ch->room->room_flags, OOC) ){
				char_desc = char_short (target);
				snprintf (buf, MAX_STRING_LENGTH,  "You %s #5%s#0 in %s,\n   \"%s\"\n", utters [cmd], char_desc,skill_data[ch->speaks].skill_name, buf4);
			}				
			else{
				char_desc = char_short (target);
				snprintf (buf, MAX_STRING_LENGTH,  "You %s #5%s#0,\n   \"%s\"\n", utters [cmd], char_desc, buf4);
			}
		}
	}
	else {
		if(ch->voice_str) {
			if ( !IS_SET (ch->room->room_flags, OOC) )
				snprintf (buf, MAX_STRING_LENGTH,  "You %s in %s, %s,\n   \"%s\"\n",
					utters [cmd],skill_data[ch->speaks].skill_name, ch->voice_str,
					buf4);
			else
				snprintf (buf, MAX_STRING_LENGTH,  "You %s, %s,\n   \"%s\"\n", utters [cmd], ch->voice_str, buf4);
		}
		else {
			if ( !IS_SET (ch->room->room_flags, OOC) )
		    		snprintf (buf, MAX_STRING_LENGTH,  "You %s in %s,\n   \"%s\"\n", utters [cmd],
		  			skill_data[ch->speaks].skill_name, buf4);
			else
				snprintf (buf, MAX_STRING_LENGTH,   "You %s,\n   \"%s\"\n", utters [cmd], buf4);
		}
	}

	send_to_char (buf, ch);

	trigger (ch, argument, TRIG_SAY);

	if ( cmd == 2 && IS_NPC (target) )
		reply_reset (ch, target, argument, cmd);

	if ( voice_changed ) {
		clear_voice (ch);
		if ( *voice )
			ch->voice_str = str_dup(voice);
	}

	if ( cmd == 0 ) {
		if ( allocd )
			mem_free (utters[cmd]);
	}

	mem_free (argument);
}


void do_sing (CHAR_DATA *ch, char *argument, int cmd)
{
	do_say (ch, argument, 1);		/* 1 = sing */
}

void do_ichat(CHAR_DATA *ch, char *argument, int cmd)
{
	char buf1 [MAX_STRING_LENGTH] = { '\0' };
	char		*p = NULL;
  	DESCRIPTOR_DATA *i = NULL;

	if ( !GET_TRUST(ch) && !IS_SET (ch->flags, FLAG_ISADMIN) ) {
		send_to_char ("Eh?\n", ch);
		return;
	}

	for (; *argument == ' '; argument++);

	if (argument == NULL) {
		send_to_char("What message would you like to send?\n", ch);
		return;
	} else {

		if ( !GET_FLAG(ch, FLAG_WIZNET) && !IS_NPC(ch) ) {
			send_to_char ("You are not currently tuned into the wiznet. Type SET WIZNET to change this.\n", ch);
			return;
		}

		if ( IS_NPC (ch) && ch->desc->original )
			snprintf (buf1, MAX_STRING_LENGTH,  "#1[Wiznet: %s (%s)]#0 %s\n",
					 GET_NAME (ch->desc->original), GET_NAME (ch), argument);
		else
			snprintf (buf1, MAX_STRING_LENGTH, "#1[Wiznet: %s]#0 %s\n",GET_NAME(ch), CAP(argument));

		reformat_string (buf1, &p);
		CAP (p);

    	for (i = descriptor_list; i; i = i->next)
		if ( i->character && (GET_TRUST (i->character) || IS_SET (i->character->flags, FLAG_ISADMIN)) ) {
      		if (!i->connected) {
				*s_buf='\0';
				if(!IS_SET(i->character->act, PLR_QUIET) && (GET_FLAG(i->character, FLAG_WIZNET) || i->original) ) {
					send_to_char(p,i->character);
				} else {
					if ( IS_SET(i->character->act, PLR_QUIET) )
						snprintf (s_buf, MAX_STRING_LENGTH, "#2[%s is editing.]#0\n",GET_NAME(i->character));
					else if ( !GET_FLAG (i->character, FLAG_WIZNET) && !IS_MORTAL (i->character) )
						snprintf (s_buf, MAX_STRING_LENGTH,  "#2[%s is not listening to the wiznet.]#0\n",GET_NAME(i->character));
					send_to_char(s_buf,ch);
				}
			}
		}

	}

	mem_free (p);
}

void do_fivenet(CHAR_DATA *ch, char *argument, int cmd)
{
	char buf1 [MAX_STRING_LENGTH] = { '\0' };
	char		*p = NULL;
  	DESCRIPTOR_DATA *i = NULL;

	for (; *argument == ' '; argument++);

	if (!(*argument)) {
		send_to_char("What message would you like to send?\n", ch);
		return;
	} else {
		if ( !GET_FLAG(ch, FLAG_WIZNET) && !IS_NPC(ch) ) {
			send_to_char ("You are not currently tuned into the wiznet. Type SET WIZNET to change this.\n", ch);
			return;
		}

		if ( IS_NPC (ch) && ch->desc->original )
			snprintf (buf1, MAX_STRING_LENGTH,  "#C[Fivenet: %s (%s)]#0 %s\n",
					 GET_NAME (ch->desc->original), GET_NAME (ch), argument);
		else
			snprintf (buf1, MAX_STRING_LENGTH, "#C[Fivenet: %s]#0 %s\n",GET_NAME(ch), CAP(argument));

		reformat_string (buf1, &p);

    	for (i = descriptor_list; i; i = i->next)
		if ( i->character && GET_TRUST (i->character) == 5 ) {
      		if (!i->connected) {
				*s_buf='\0';
				if(!IS_SET(i->character->act, PLR_QUIET) && (GET_FLAG(i->character, FLAG_WIZNET) || i->original) ) {
					send_to_char(p,i->character);
				} else {
					if ( IS_SET(i->character->act, PLR_QUIET) )
						snprintf (s_buf, MAX_STRING_LENGTH, "#2[%s is editing.]#0\n",GET_NAME(i->character));
					else if ( !GET_FLAG (i->character, FLAG_WIZNET) )
						snprintf (s_buf, MAX_STRING_LENGTH,  "#2[%s is not listening to the wiznet.]#0\n",GET_NAME(i->character));
					send_to_char(s_buf,ch);
				}
			}
		}

	}

	mem_free (p);
}

void do_tell (CHAR_DATA *ch, char *argument, int cmd)
{
	do_say (ch, argument, 2);
}

void do_immtell (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA			*vict = NULL;
	DESCRIPTOR_DATA		*d = NULL;
	char				*p = NULL;
	char				name [MAX_STRING_LENGTH] = { '\0' };
	char				message [MAX_STRING_LENGTH] = { '\0' };
	char				buf [MAX_STRING_LENGTH] = { '\0' };

	half_chop (argument, name, message);

	if ( !*name || !*message )
		send_to_char ("Who do you wish to tell what??\n", ch);

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
				if ( d == vict->pc->owner )
					break;
			}

			if ( !d ) {
				send_to_char ("That player has disconnected.\n", ch);
				return;
			}
		}

		snprintf (buf, MAX_STRING_LENGTH,  "#2[From %s]#0 %s\n", (IS_NPC(ch) ? ch->short_descr : GET_NAME(ch)), CAP(message));
		reformat_string (buf, &p);
		send_to_char (p, vict);
		mem_free (p);

		snprintf (buf, MAX_STRING_LENGTH,  "#5[To %s]#0 %s\n", GET_NAME(vict), CAP(message));
		reformat_string (buf, &p);
		send_to_char (p, ch);
		mem_free (p);
	}
}

int whisper_it (CHAR_DATA *ch, int skill, char *source, char *target)
{
	int			missed = 0;
	int			got_one = 0;
	int			bonus = 0;
	char		*in = NULL;
	char		*out = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	if ( !real_skill (ch, skill) )
		return 0;

	skill_use (ch, skill, 0);

	if ( skill == SKILL_TELEPATHY )
		bonus = 20;

	in = source;
	out = buf;
	*out = '\0';

	while ( *in ) {

		while ( *in == ' ' ) {
			in++;
			*out = ' ';
			out++;
		}
	
		*out = '\0';

		if ( ch->skills [skill] + bonus < number (1, 100) ) {
			if ( !missed ) {
				strcat (out, " . . .");
				out += strlen (out);
			}

			missed = 1;

			while ( *in && *in != ' ' )
				in++;
		}

		else {
			while ( *in && *in != ' ' ) {
				*out = *in;
				out++;
				in++;
			}

			got_one = 1;
			missed = 0;
		}

		*out = '\0';
	}

	strcpy (target, buf);

	return got_one;
}

void do_whisper (CHAR_DATA *ch, char *argument, int cmd)
{
	int			heard_something = 0;
	CHAR_DATA		*vict = NULL;
	CHAR_DATA		*tch = NULL;
	char			*acc_desc = NULL;
	AFFECTED_TYPE	*tongues = NULL;
	char			*p = NULL;
	char			name [MAX_STRING_LENGTH] = { '\0' };
	char			message [MAX_STRING_LENGTH] = { '\0' };
	char			buf [MAX_STRING_LENGTH] = { '\0' };
	char			buf2 [MAX_STRING_LENGTH] = { '\0' };
	char			buf3 [MAX_STRING_LENGTH] = { '\0' };
	char			buf4 [MAX_STRING_LENGTH] = { '\0' };

        if ( ch->room->sector_type == SECT_UNDERWATER ) {
                send_to_char ("You can't do that underwater!\n", ch);
                return;
        }

	half_chop (argument, name, message);

	*message = toupper (*message);

	reformat_say_string (message, &p);

	if ( ch->room->virtual == AMPITHEATRE && IS_MORTAL(ch) ) {
		if ( !get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
			!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand) ) {
			send_to_char ("You decide against speaking out of turn. PETITION to request to speak.\n", ch);
			return;
		}
	}

	if ( !*name || !*message ) {
		send_to_char ("Who do you want to whisper to.. and what?\n", ch);
		return;
	}

	if ( !(vict = get_char_room_vis (ch, name)) ) {
		send_to_char ("No-one by that name here.\n", ch);
		return;
	}

	else if ( vict == ch ) {
		act ("$n whispers quietly to $mself.", FALSE, ch, 0, 0, TO_ROOM);
		send_to_char ("You whisper to yourself.\n", ch);
		return;
	}

	if ( !IS_SET (ch->room->room_flags, OOC) )
		snprintf (buf, MAX_STRING_LENGTH,  "You whisper to $N in %s,\n   \"%s\"",
					 skill_data[ch->speaks].skill_name, p);
	else
		snprintf (buf, MAX_STRING_LENGTH,  "You whisper to $N,\n   \"%s\"", p);

	act (buf, TRUE, ch, 0, vict, TO_CHAR);

	tongues = get_affect (ch, MAGIC_AFFECT_TONGUES);

	if ( !tongues && !real_skill (ch, ch->speaks) && !IS_SET (ch->room->room_flags, OOC) ) {
		send_to_char ("You don't know the language you want to "
					  "whisper\n", ch);
		return;
	}

	snprintf (buf4, MAX_STRING_LENGTH,  "%s", p);
	*buf4 = toupper(*buf4);
	snprintf (buf3, MAX_STRING_LENGTH,  "%s", p);
	*buf3 = toupper(*buf3);

	skill_use (ch, ch->speaks, 0);

	for ( tch = vtor (ch->in_room)->people; tch; tch = tch->next_in_room ) {

		if ( tch == ch )	/* Don't say it to ourselves */
			continue;

		snprintf (buf2, MAX_STRING_LENGTH,  "%s", p);

		heard_something = 1;

		if ( tch != vict ) {
			if(get_affect(tch, MUTE_EAVESDROP))
			    continue;
			heard_something = whisper_it (tch, SKILL_LISTEN, buf2, buf2);
		}
		if ( !heard_something ) {
			act ("$n whispers something to $3, but you can't quite make out the words.",
					TRUE, ch, (OBJ_DATA *) vict, tch, TO_VICT | TO_ACT_FORMAT);
			continue;
		}

		if ( tch == vict ) {
			if ( !IS_SET (ch->room->room_flags, OOC) && decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "$3 whispers to you in %s,",
                 		(tch->skills [ch->speaks] || tongues) ?
                        	skill_data[ch->speaks].skill_name : "an unknown tongue");
				if ( tch->skills [ch->speaks] >= 50 && ch->skills[ch->speaks] < 50 ){
					acc_desc = accent_desc(ch->skills[ch->speaks]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
				}
				
				act (buf, FALSE, tch, (OBJ_DATA *)ch, 0, TO_CHAR | TO_ACT_FORMAT);

				snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"", buf3);
				act (buf, FALSE, tch, 0, 0, TO_CHAR);
			}
			else if ( !IS_SET (ch->room->room_flags, OOC) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "$3 whispers something to you in %s, but you cannot decipher %s words.",
                 		(tch->skills [ch->speaks] || tongues) ?
                        	skill_data[ch->speaks].skill_name : "an unknown tongue", HSHR(ch));
				act (buf, FALSE, tch, (OBJ_DATA *)ch, 0, TO_CHAR | TO_ACT_FORMAT);
			}
			else {
				snprintf (buf, MAX_STRING_LENGTH,  "$3 whispers to you,\n   \"%s\"", buf4);
				act (buf, FALSE, tch, (OBJ_DATA *)ch, 0, TO_CHAR);
			}
			if ( IS_NPC (vict) )
				reply_reset (ch, vict, buf2, 4);		/* 4 = whisper */
		}

		else {
			if ( !IS_SET (ch->room->room_flags, OOC) && decipher_speaking (tch, ch->speaks, ch->skills[ch->speaks]) ) {
				snprintf (buf, MAX_STRING_LENGTH, "You overhear $3 whispering in %s to $N,",
                 		(tch->skills [ch->speaks] || tongues) ?
                        	skill_data[ch->speaks].skill_name : "an unknown tongue");
				if ( tch->skills [ch->speaks] >= 50 && ch->skills[ch->speaks] < 50 ){
					acc_desc = accent_desc(ch->skills[ch->speaks]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
				}
				
				act (buf, FALSE, tch, (OBJ_DATA *)ch, vict, TO_CHAR | TO_ACT_FORMAT);

				snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"", buf2);
				act (buf, FALSE, tch, 0, 0, TO_CHAR);
			}
			else if ( !IS_SET (ch->room->room_flags, OOC) ) {
				snprintf (buf, MAX_STRING_LENGTH, "You overhear $3 whispering something in %s to $N, but you cannot decipher %s words.",
                 		(tch->skills [ch->speaks] || tongues) ?
                        	skill_data[ch->speaks].skill_name : "an unknown tongue", HSHR(ch));
				act (buf, FALSE, tch, (OBJ_DATA *)ch, vict, TO_CHAR | TO_ACT_FORMAT);
			}
			else {
				snprintf (buf, MAX_STRING_LENGTH,  "You overhear $3 whisper to $N,\n   \"%s\"", buf2);
				act (buf, FALSE, tch, (OBJ_DATA *)ch, vict, TO_CHAR);
			}
		}

		snprintf (p, MAX_STRING_LENGTH,  "%s", buf3);
	}

	mem_free (p);

	trigger (ch, argument, TRIG_WHISPER);
}


void do_ask (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*vict = NULL;
	char		name [MAX_STRING_LENGTH] = { '\0' };
	char		message [MAX_STRING_LENGTH] = { '\0' };
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	half_chop(argument,name,message);

	if(!*name || !*message)
		send_to_char("Who do you want to ask something.. and what??\n", ch);
	else if (!(vict = get_char_room_vis(ch, name)))
		send_to_char("No-one by that name here.\n", ch);
	else if (vict == ch)
	{
		act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
		send_to_char("You think about it for a while...\n", ch);
	}
	else
	{
		snprintf (buf, MAX_STRING_LENGTH, "$n asks you '%s'",message);
		act(buf, FALSE, ch, 0, vict, TO_VICT);
		send_to_char("Ok.\n", ch);
		act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
	}
}

void do_talk (CHAR_DATA *ch, char *argument, int cmd)
{
    if ( !is_at_table (ch, NULL) )
		do_say (ch, argument, 0);
	else
		do_say (ch, argument, 3);
}

void do_petition (CHAR_DATA *ch, char *argument, int cmd)
{
	CHAR_DATA	*admin = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };
	char		*p = NULL;
	bool		sent = FALSE;
	char		*date = NULL;

	argument = one_argument (argument, buf);

	while ( isspace (*argument) )
		argument++;

	if ( argument == NULL ) {
		send_to_char ("Petition what message?\n", ch);
		return;
	}

	if ( IS_SET (ch->plr_flags, NOPETITION) ) {
		act ("Your ability to petition/think has been revoked by an admin.", FALSE, ch, 0, 0, TO_CHAR);
		return;
	}


	if ( ch->desc->account && IS_SET (ch->desc->account->flags, ACCOUNT_NOPETITION) ) {
		act ("Your ability to petition has been revoked by an admin.", FALSE, ch, 0, 0, TO_CHAR);
		return;
	}

	if ( !str_cmp (buf, "all") ) {

		snprintf (buf, MAX_STRING_LENGTH, "#6[Petition: %s]#0 %s\n", IS_NPC(ch) ? ch->short_descr : GET_NAME(ch), CAP(argument));
		reformat_string (buf, &p);
		
		for ( admin = character_list; admin; admin = admin->next ) {

			if ( admin->deleted )
				continue;

			if ( !GET_TRUST (admin) )
				continue;

			if ( !admin->desc )
				continue;

			send_to_char (p, admin);
			
			sent = TRUE;
		}

		mem_free (p);

		snprintf (buf, MAX_STRING_LENGTH, "You petitioned: %s\n", CAP(argument));
		reformat_string (buf, &p);
		send_to_char(p,ch);
		mem_free (p);

		if ( !get_affect (ch, MAGIC_PETITION_MESSAGE) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "\n#6Please understand that, as we are a staff of volunteers, we cannot be online\n"
				      "to respond to petitions 24 hours a day. If you find your petitions going largely\n"
				      "unanswered, email may be a more efficient recourse: "STAFF_EMAIL".#0\n");
			send_to_char (buf, ch);
			snprintf (buf, MAX_STRING_LENGTH,  "\n#6If there are no staff currently online, your petition will be logged for review\n"
				      "and responded to within a few days by an administrator via email if necessary.#0\n");
			send_to_char (buf, ch);
			snprintf (buf, MAX_STRING_LENGTH,  "\n#6If you have not already, please read #0HELP PETITION#6 for petitioning guidelines.\n#0");
			send_to_char (buf, ch);
                        magic_add_affect (ch, MAGIC_PETITION_MESSAGE, 480, 0, 0, 0, 0);
		}

		if ( !sent ) {
			date = timestr(date);
			
			snprintf (buf, MAX_STRING_LENGTH,  "From: %s [%d]\n\n", ch->tname, ch->in_room);
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%s\n", argument);
			add_message ("Petitions", 2, ch->tname, date, "Logged Petition", "", buf, 0);
			mem_free (date);
		}

		return;
	}

	admin = load_pc (buf);

	if ( !admin ) {
		send_to_char ("Are you sure you didn't mistype the name?\n", ch);
		return;
	}

	if ( admin == ch ) {
		send_to_char ("Petition yourself? I see...\n", ch);
		unload_pc (admin);
		return;
	}

	if ( !is_he_somewhere (admin) || !IS_SET (admin->flags, FLAG_AVAILABLE) || !admin->pc->level ) {
		send_to_char ("Sorry, but that person is currently unavailable.\n", ch);
		unload_pc (admin);
		return;
	}

	if ( IS_SET (admin->act, PLR_QUIET) ) {
		send_to_char ("That admin is editing.  Please try again in a minute.\n",
					  ch);
		unload_pc (admin);
		return;
	}

	snprintf (buf, MAX_STRING_LENGTH, "#5[Private Petition: %s]#0 %s\n", IS_NPC(ch) ? ch->short_descr : GET_NAME(ch), CAP(argument));
	reformat_string (buf, &p);
	send_to_char(p, admin);
	mem_free (p);
	snprintf (buf, MAX_STRING_LENGTH, "You petitioned %s: %s\n", GET_NAME(admin), CAP(argument));
	reformat_string (buf, &p);
	send_to_char(p,ch);
	mem_free (p);

	unload_pc (admin);
}


void do_shout(CHAR_DATA *ch, char *argument, int cmd)
{
	ROOM_DATA		*room;
	CHAR_DATA		*tch = NULL;
	OBJ_DATA		*tobj = NULL;
	AFFECTED_TYPE	*tongues = NULL;
	int 			door = 0;
	int				return_to = 0;
	int				to = 0;
	char 			buf [MAX_STRING_LENGTH] = { '\0' };
	char 			buf2 [MAX_STRING_LENGTH] = { '\0' };
	char			buf3 [MAX_STRING_LENGTH] = { '\0' };
	char			buf4 [MAX_STRING_LENGTH] = { '\0' };
	char			*acc_desc = NULL;
	char			*obj_desc = NULL;
	
	char *rev_d[] = {
		"the south,",
		"the west,",
		"the north,",
		"the east,",
		"below,",
		"above,"
	};

        if ( ch->room->sector_type == SECT_UNDERWATER ) {
                send_to_char ("You can't do that underwater!\n", ch);
                return;
        }

	tongues = get_affect (ch, MAGIC_AFFECT_TONGUES);
	for(;isspace(*argument);argument++);

	if ( ch->room->virtual == AMPITHEATRE && IS_MORTAL(ch) ) {
		if ( !get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->right_hand) &&
			!get_obj_in_list_num (VNUM_SPEAKER_TOKEN, ch->left_hand) ) {
			send_to_char ("You decide against speaking out of turn. PETITION to request to speak.\n", ch);
			return;
		}
	}

	if ( !tongues && !real_skill (ch, ch->speaks) ) {
		send_to_char ("You don't know that language!\n", ch);
		return;
	}

	if ( argument == NULL ) {
		send_to_char ("What would you like to shout?\n", ch);
		return;
	}

	CAP (argument);
	reformat_say_string (argument, &argument);

	snprintf (buf4, MAX_STRING_LENGTH,  "%s",  argument);	/* The intended message, sent to the player.*/

	for ( tch = vtor (ch->in_room)->people; tch; tch = tch->next_in_room ) {

		if ( tch == ch ) {
			if ( !IS_SET (ch->room->room_flags, OOC) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "You shout in %s,",
				skill_data[ch->speaks].skill_name);
			}
			else {
				snprintf (buf, MAX_STRING_LENGTH,  "You shout,");
			}
			snprintf (buf2, MAX_STRING_LENGTH,  "   \"%s\"", buf4);
			act (buf, FALSE, ch, 0, 0, TO_CHAR);
			act (buf2, FALSE, ch, 0, 0, TO_CHAR);
			continue;
		}

		if ( !IS_SET (ch->room->room_flags, OOC) )
			snprintf (buf, MAX_STRING_LENGTH,  "$N shouts in %s,",
						 skill_data[ch->speaks].skill_name);
		else
			snprintf (buf, MAX_STRING_LENGTH,  "$N shouts,");

		if ( !tch->skills [ch->speaks] && !tongues && !IS_SET (ch->room->room_flags, OOC) ) {
			snprintf (buf, MAX_STRING_LENGTH,  "$N shouts, in an unknown tongue,");
		}

		acc_desc = accent_desc(ch->skills[ch->speaks]);
		if ( tch->skills [ch->speaks] >= 50 && ch->skills[ch->speaks] < 50 )
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);

		if ( !decipher_speaking (tch, ch->speaks, ch->skills [ch->speaks]) && !IS_SET (ch->room->room_flags, OOC) ) {
			snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " something that you fail to decipher.");
			act (buf, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);
		}
		else if ( tch->desc ) {
			act (buf, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);
			if ( decipher_speaking (tch, ch->speaks, ch->skills [ch->speaks]) ) {
				snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"\n", buf4);
				send_to_char (buf, tch);
			}
		}

		if ( GET_TRUST (tch) && !IS_NPC (tch) && GET_FLAG (tch, FLAG_SEE_NAME) )
			snprintf (buf3, MAX_STRING_LENGTH,  " (%s)", GET_NAME (ch));
		else
			*buf3 = '\0';
		snprintf (buf2, MAX_STRING_LENGTH,  "%s", argument);		/* Reset, for next listener. */
		continue;

	}

	if ( ch->in_room > 100000 && ch->room->entrance ) {
		return_to = ch->in_room;
		to = ch->room->entrance;
		char_from_room (ch);
		char_to_room (ch, to);
	}

	for(door=0; door<6; door++) {
		if(PASSAGE(ch,door) && (PASSAGE(ch,door)->to_room != -1)) {
	 		for ( tch = vtor (PASSAGE(ch,door)->to_room)->people; tch; tch = tch->next_in_room ) {

				if ( tch == ch )
					continue;

				if (GET_SEX(ch) == SEX_MALE) {
					if ( !IS_SET (ch->room->room_flags, OOC) ) {
						if (!tch->skills [ch->speaks])
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a male voice shout in an unknown tongue from %s",
								  rev_d[door]);
						else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a male voice shout in %s from %s",
								 skill_data[ch->speaks].skill_name, rev_d[door]);
					}
					else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a male voice shout from %s", rev_d[door]);
				}

				if (GET_SEX(ch) == SEX_FEMALE) {
					if ( !IS_SET (ch->room->room_flags, OOC) ) {
						if (!tch->skills [ch->speaks])
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a female voice shout in an unknown tongue from %s",
								  rev_d[door]);
						else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a female voice shout in %s from %s",
								 skill_data[ch->speaks].skill_name, rev_d[door]);
					}
					else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a female voice shout from %s", rev_d[door]);
				}

				if (GET_SEX(ch) == SEX_NEUTRAL) {
					if ( !IS_SET (ch->room->room_flags, OOC) ) {
						if (!tch->skills [ch->speaks])
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a voice shout in an unknown tongue from %s",
								  rev_d[door]);
						else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a voice shout in %s from %s",
								 skill_data[ch->speaks].skill_name, rev_d[door]);
					}
					else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a voice shout from %s", rev_d[door]);
				}
				
				if ( tch->skills[ch->speaks] >= 50 && ch->skills [ch->speaks] < 50 ){
					acc_desc = accent_desc(ch->skills[ch->speaks]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
				}
				
				if ( !decipher_speaking (tch, ch->speaks, ch->skills [ch->speaks]) ) {
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " though you cannot decipher the words.");
					act (buf, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);
				}
				
				else if ( tch->desc ) {
					act (buf, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);
					if ( decipher_speaking (tch, ch->speaks, ch->skills [ch->speaks]) ) {
						if ( !IS_SET (ch->room->room_flags, OOC) )
							snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"\n", buf4);
						else
							snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"\n", buf4);
						send_to_char (buf, tch);
					}
				}
				
				if ( GET_TRUST (tch) && !IS_NPC (tch) && GET_FLAG (tch, FLAG_SEE_NAME) )
					snprintf (buf3, MAX_STRING_LENGTH,  " (%s)", GET_NAME (ch));
				else
					*buf3 = '\0';

				snprintf (buf2, MAX_STRING_LENGTH,  "%s", argument);	/* Reset. */

			}
		}
	}

	if ( return_to > 100000 ) {
		char_from_room (ch);
		char_to_room (ch, return_to);

		for ( tobj = vtor(ch->room->entrance)->contents; tobj; tobj = tobj->next_content ) {
			if ( tobj->o.od.value[0] == ch->in_room )
				break;
		}

		if ( tobj ) {
		 	for ( tch = vtor (ch->room->entrance)->people; tch; tch = tch->next_in_room ) {
				if ( tch == ch )
					continue;
				if (GET_SEX(ch) == SEX_MALE) {
					if ( !IS_SET (ch->room->room_flags, OOC) ) {
						if (!tch->skills [ch->speaks]){
							obj_desc = obj_short_desc(tobj);
							snprintf (buf, MAX_STRING_LENGTH,  "You hear a male voice shout in an unknown tongue from inside #2%s#0,", obj_desc);
						}
						else{
							obj_desc = obj_short_desc(tobj);
							snprintf (buf, MAX_STRING_LENGTH,  "You hear a male voice shout in %s from inside #2%s#0,",skill_data[ch->speaks].skill_name, obj_desc);
						}
					}
					else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a male voice shout from the stands,");
				}	
				if (GET_SEX(ch)	 == SEX_FEMALE) {
					if ( !IS_SET (ch->room->room_flags, OOC) ) {
						if (!tch->skills [ch->speaks]){
							obj_desc = obj_short_desc(tobj);
							snprintf (buf, MAX_STRING_LENGTH,  "You hear a female voice shout in an unknown tongue from inside #2%s#0,", obj_desc);
						}
						else{
							obj_desc = obj_short_desc(tobj);
							snprintf (buf, MAX_STRING_LENGTH,  "You hear a female voice shout in %s from inside #2%s#0,",skill_data[ch->speaks].skill_name, obj_desc);
							}
					}
					else{
						obj_desc = obj_short_desc(tobj);
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a female voice shout from inside #2%s#0,", obj_desc);
					}						
				}	
				if (GET_SEX(ch) == SEX_NEUTRAL) {
					if ( !IS_SET (ch->room->room_flags, OOC) ) {
						if (!tch->skills [ch->speaks]){
							obj_desc = obj_short_desc(tobj);
							snprintf (buf, MAX_STRING_LENGTH,  "You hear a voice shout in an unknown tongue from inside #2%s#0,", obj_desc);
						}
						else{
							obj_desc = obj_short_desc(tobj);
							snprintf (buf, MAX_STRING_LENGTH,  "You hear a voice shout in %s from inside #2%s#0,",skill_data[ch->speaks].skill_name, obj_desc);
						}
					}
					else{
						obj_desc = obj_short_desc(tobj);
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a voice shout from inside #2%s#0,", obj_desc);
					}
					
				}
				
				if ( tch->skills[ch->speaks] >= 50 && ch->skills [ch->speaks] < 50 ){
					acc_desc = accent_desc(ch->skills[ch->speaks]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
				}
				
				if ( !decipher_speaking (tch, ch->speaks, ch->skills [ch->speaks]) ) {
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " though you cannot decipher the words.");
					act (buf, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);
				}
				else if ( tch->desc ) {
					act (buf, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);
					if ( decipher_speaking (tch, ch->speaks, ch->skills [ch->speaks]) ) {
						if ( !IS_SET (ch->room->room_flags, OOC) )
							snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"\n", buf4);
						else
							snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"\n", buf4);
						send_to_char (buf, tch);
					}
				}
					
				if ( GET_TRUST (tch) && !IS_NPC (tch) && GET_FLAG (tch, FLAG_SEE_NAME) )
					snprintf (buf3, MAX_STRING_LENGTH,  " (%s)", GET_NAME (ch));
				else
					*buf3 = '\0';
				snprintf (buf2, MAX_STRING_LENGTH,  "%s", argument);	/* Reset. */
			}
		}
	}

	for ( room = full_room_list; room; room = room->lnext ) {
		if ( room->entrance == ch->in_room ) {
	 		for ( tch = room->people; tch; tch = tch->next_in_room ) {

				if ( tch == ch )
					continue;

				if (GET_SEX(ch) == SEX_MALE) {
					if ( !IS_SET (ch->room->room_flags, OOC) ) {
						if (!tch->skills [ch->speaks])
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a male voice shout in an unknown tongue from the outside,");
						else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a male voice shout in %s from the outside,",
								 skill_data[ch->speaks].skill_name);
					}
					else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a male voice shout from the outside,");
				}

				if (GET_SEX(ch) == SEX_FEMALE) {
					if ( !IS_SET (ch->room->room_flags, OOC) ) {
						if (!tch->skills [ch->speaks])
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a female voice shout in an unknown tongue from the outside,");
						else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a female voice shout in %s from the outside,",
								 skill_data[ch->speaks].skill_name);
					}
					else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a female voice shout from the outside,");
				}

				if (GET_SEX(ch) == SEX_NEUTRAL) {
					if ( !IS_SET (ch->room->room_flags, OOC) ) {
						if (!tch->skills [ch->speaks])
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a voice shout in an unknown tongue from the outside,");
						else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a voice shout in %s from the outside,",
								 skill_data[ch->speaks].skill_name);
					}
					else
						snprintf (buf, MAX_STRING_LENGTH,  "You hear a voice shout from the outside,");
				}
				
				if ( tch->skills[ch->speaks] >= 50 && ch->skills [ch->speaks] < 50 ){
					acc_desc = accent_desc(ch->skills[ch->speaks]);
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " %s,", acc_desc);
				}
				
				if ( !decipher_speaking (tch, ch->speaks, ch->skills [ch->speaks]) ) {
					snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  " though you cannot decipher the words.");
					act (buf, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);
				}
				
				else if ( tch->desc ) {
					act (buf, FALSE, tch, 0, ch, TO_CHAR | TO_ACT_FORMAT);
					if ( decipher_speaking (tch, ch->speaks, ch->skills [ch->speaks]) ) {
						if ( !IS_SET (ch->room->room_flags, OOC) )
							snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"\n", buf4);
						else
							snprintf (buf, MAX_STRING_LENGTH,  "   \"%s\"\n", buf4);
						send_to_char (buf, tch);
					}
				}
				
				if ( GET_TRUST (tch) && !IS_NPC (tch) && GET_FLAG (tch, FLAG_SEE_NAME) )
					snprintf (buf3, MAX_STRING_LENGTH,  " (%s)", GET_NAME (ch));
				else
					*buf3 = '\0';

				snprintf (buf2, MAX_STRING_LENGTH,  "%s", argument);	/* Reset. */

			}
		}
	}
	mem_free (argument);
	return;
}

int add_to_list (ROOM_DATA **list, ROOM_DATA *start_room, int *elements)
{
	int			i = 0;

	for ( i = 0; i < *elements; i++ )
		if ( list [i] == start_room )
			return 0;

	list [*elements] = start_room;

	(*elements)++;

	return 1;
}

void get_room_list (int radius, ROOM_DATA *base_room, ROOM_DATA **near_rooms,
					int *num_rooms)
{
	int			room_set_top = 0;
	int			room_set_bot = 0;
	int			dir = 0;
	ROOM_DATA 	*this_room = NULL;
	
	*num_rooms = 0;

	add_to_list (near_rooms, base_room, num_rooms);

	room_set_top = 0;
	room_set_bot = 0;

	while ( radius-- ) {
		while ( room_set_top <= room_set_bot ) {

			for ( dir = 0; dir < 6; dir++ )
				if ( near_rooms [room_set_top]->dir_option [dir] ){
					this_room = vtor (near_rooms [room_set_top]->
										dir_option [dir]->to_room);					
					add_to_list (near_rooms, this_room, num_rooms);			
				}

			room_set_top++;
		}

		room_set_bot = *num_rooms - 1;

		if ( room_set_top >= *num_rooms )
			break;		/* Ran out of rooms */
	}
	return;
}

void wolf_howl (CHAR_DATA *ch)
{
	int			num_rooms = 0;
	int			i = 0;
	int			dir = 0;
	ROOM_DATA	*near_rooms [220];
	CHAR_DATA	*tch = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	for ( i = 0; i <= 219; i++ )
		near_rooms [i] = NULL;

	get_room_list (3, ch->room, near_rooms, &num_rooms);

	act ("$n howls mournfully.", FALSE, ch, 0, 0, TO_ROOM);
	act ("You howl mournfully.", FALSE, ch, 0, 0, TO_CHAR);

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( !IS_NPC (tch) )
				continue;
			if ( tch->race != RACE_WOLF )
				continue;
			if ( !ch->fighting )
				continue;
			if ( tch->fighting ||
				 tch->delay)
				continue;
			set_fighting (ch, tch);
	}

	for ( i = 1; i < num_rooms; i++ ) {

		if ( near_rooms [i]->people ) {
			dir = track (near_rooms [i]->people, ch->room->virtual);

			if ( dir == -1 )
				continue;

			snprintf (buf, MAX_STRING_LENGTH,  "There is a loud, mournful howl coming from the %s.\n",
						  dirs [dir]);

			send_to_room (buf, near_rooms [i]->virtual);
		}

		for ( tch = near_rooms [i]->people; tch; tch = tch->next_in_room ) {

			if ( !IS_NPC (tch) )
				continue;

			if ( tch->race != RACE_WOLF )
				continue;

			if ( tch->fighting ||
				 tch->delay ||
				 IS_SET (tch->act, ACT_SENTINEL) )
				continue;

			act ("$n stands upright suddenly, cocking $s head.",
						TRUE, tch, 0, 0, TO_ROOM);

			tch->delay		 = 2;
			tch->delay_type  = DEL_ALERT;
			tch->delay_info1 = ch->in_room;
			tch->delay_info2 = 8;
			if ( ch->fighting )
				add_threat (tch, ch->fighting, 3);
		}
	}
}

void do_alert (CHAR_DATA *ch, char *argument, int cmd)
{
	int			num_rooms = 0;
	int			i = 0;
	int			dir = 0;
	ROOM_DATA	*rooms [220];
	CHAR_DATA	*tch = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	for ( i = 0; i <= 219; i++ )
		rooms [i] = NULL;

	if ( GET_POS (ch) < REST )
		return;

	if ( ch->race == RACE_WOLF ) {
		wolf_howl (ch);
		return;
	}

	get_room_list (3, ch->room, rooms, &num_rooms);

	act ("$n whistles very loudly.", FALSE, ch, 0, 0, TO_ROOM);
	act ("You whistle very loudly.", FALSE, ch, 0, 0, TO_CHAR);

	for ( tch = ch->room->people; tch; tch = tch->next_in_room ) {
			if ( !IS_NPC (tch) )
				continue;
			if ( !ch->fighting )
				continue;
			if ( tch->fighting ||
				 tch->delay)
				continue;
			if ( !is_brother (ch, tch) )
				continue;
			set_fighting (ch, tch);
	}

	for ( i = 1; i < num_rooms; i++ ) {

		if ( rooms [i]->people ) {
			dir = track (rooms [i]->people, ch->room->virtual);

			if ( dir == -1 )
				continue;

			snprintf (buf, MAX_STRING_LENGTH,  "There is a loud whistle coming from the %s.\n",
						  dirs [dir]);

			send_to_room (buf, rooms [i]->virtual);
		}

		for ( tch = rooms [i]->people; tch; tch = tch->next_in_room ) {

			if ( !IS_NPC (tch) )
				continue;

			if ( tch->fighting ||
				 tch->delay ||
				 IS_SET (tch->act, ACT_SENTINEL) )
				continue;

			if ( !is_brother (ch, tch) )
				continue;

			act ("$n glances up suddenly with a concerned look on $s face.",
						TRUE, tch, 0, 0, TO_ROOM);

			tch->delay		 = 2;
			tch->delay_type  = DEL_ALERT;
			tch->delay_info1 = ch->in_room;
			tch->delay_info2 = 8;
			if ( ch->fighting )
				add_threat (tch, ch->fighting, 3);
		}
	}
}

void delayed_alert (CHAR_DATA *ch)
{
	int			dir = 0;
	int			save_speed = 0;
	int			current_room = 0;
	ROOM_DATA	*to_room = NULL;
	char		buf [MAX_STRING_LENGTH] = { '\0' };

	dir = track (ch, ch->delay_info1);

	if ( dir == -1 ) {
		send_to_char ("You can't figure out where the whistle came from.\n",
					  ch);
		ch->delay	= 0;
		ch->delay_type  = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		return;
	}

	current_room = ch->in_room;

    if ( IS_HITCHEE (ch) )
        return;

    if ( IS_SUBDUEE (ch) )
        return;

	if ( !PASSAGE (ch, dir) )
		return;

	to_room = vtor (PASSAGE (ch, dir)->to_room);
	
	if ( !(to_room) )
		return;

	if ( IS_SET (to_room->room_flags, NO_MOB) ){
		free(to_room);
		return;
	}	

	if ( IS_SET (ch->flags, FLAG_KEEPER) && IS_SET (to_room->room_flags, NO_MERCHANT) ){
		free(to_room);
		return;
	}
	
	if ( ch->mob && ch->mob->access_flags &&
		 !(ch->mob->access_flags & to_room->room_flags) ){
		free(to_room);
		return;
	}
	
	if ( IS_SET (ch->act, ACT_STAY_ZONE) &&
		 ch->room->zone != to_room->zone )
		return;
	
	save_speed = ch->speed;
	ch->speed = SPEED_RUN;

	snprintf (buf, MAX_STRING_LENGTH,  "%s", dirs [dir]);

	command_interpreter (ch, buf);

	ch->speed = save_speed;

	ch->delay_info2--;

	if ( current_room == ch->in_room || ch->delay_info2 <= 0 ) {
		send_to_char ("You can't locate the whistle.\n", ch);
		ch->delay	 = 0;
		ch->delay_type  = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
		ch->delay_ch = 0;
		return;
	}

	if ( ch->in_room != ch->delay_info1 )
		ch->delay = 8;
	else {
		ch->delay	 = 0;
		ch->delay_type  = 0;
		ch->delay_info1 = 0;
		ch->delay_info2 = 0;
	}
}

void clear_voice(CHAR_DATA *ch)
{
        if(ch->voice_str) {
            mem_free(ch->voice_str);
            ch->voice_str = NULL;
        }
}


/****************************************************************************
 *                                                            TRAVEL STRING *
 ****************************************************************************/

#define TRAVEL_RESET "normal"	     /* keyword to reset user travel string */

/*                                                                          *
 * function: clear_travel               < e.g.> travel normal               *
 *                                                                          */
void clear_travel(CHAR_DATA *ch)
{
  if(ch->travel_str) {
    mem_free(ch->travel_str);
    ch->travel_str = NULL;
    send_to_char("Your travel string has been cleared.\n", ch);
  }
}

/*                                                                          *
 * function: clear_travel               < e.g.> travel [ normal|<string> ]  *
 *                                                                          */
void do_travel (CHAR_DATA *ch, char *argument, int cmd)
{
  char buf [MAX_STRING_LENGTH] = { '\0' };
  
  while(isspace(*argument))
    argument++;
  
  if ( strchr (argument, '~') ) {
    send_to_char ("Sorry, but you can't use tildae when setting a travel string.\n", ch);
    return;
  }

  if(argument == NULL) {
    if ( ch->travel_str ) {
      snprintf (buf, MAX_STRING_LENGTH,  "Your current travel string: (#2%s#0)\n",ch->travel_str);
    }
    else {
      snprintf (buf, MAX_STRING_LENGTH,  "You do not currently have a travel string set.\n");
    }
    send_to_char(buf, ch);
  } 
  else {
    if(strcasecmp(argument, TRAVEL_RESET) == 0) {
      clear_travel(ch);
    } 
    else {
      snprintf (buf, MAX_STRING_LENGTH,  "Your travel string has been set to: (#2%s#0)", argument);
      ch->travel_str = add_hash(argument);
    }  
    act(buf,FALSE,ch,0,0,TO_CHAR | TO_ACT_FORMAT);
  }
}

