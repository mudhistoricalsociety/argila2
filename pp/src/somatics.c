/*------------------------------------------------------------------------\
|  somatics.c : Short and Long Term Somatic Effects   www.yourmud.org |
|  Copyright (C) 2004, Shadows of Isildur: Sighentist                     |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
 
#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

void soma_stat (CHAR_DATA *ch, AFFECTED_TYPE *af)
{
	char			buf[MAX_STRING_LENGTH];
	char			buf2[MAX_STRING_LENGTH];

	switch (af->type) {
	case SOMA_MUSCULAR_CRAMP:	snprintf (buf2, MAX_STRING_LENGTH, "a muscle cramp"); break;
	case SOMA_MUSCULAR_TWITCHING:	snprintf (buf2, MAX_STRING_LENGTH, "twitching"); break;
	case SOMA_MUSCULAR_TREMOR:	snprintf (buf2, MAX_STRING_LENGTH, "tremors"); break;
	case SOMA_MUSCULAR_PARALYSIS:	snprintf (buf2, MAX_STRING_LENGTH, "paralysis"); break;
	case SOMA_DIGESTIVE_ULCER:	snprintf (buf2, MAX_STRING_LENGTH, "stomach ulcer"); break; 
	case SOMA_DIGESTIVE_VOMITING:	snprintf (buf2, MAX_STRING_LENGTH, "vomiting"); break;
	case SOMA_DIGESTIVE_BLEEDING:	snprintf (buf2, MAX_STRING_LENGTH, "vomiting blood"); break; 
	case SOMA_EYE_BLINDNESS:	snprintf (buf2, MAX_STRING_LENGTH, "blindness"); break;
	case SOMA_EYE_BLURRED:		snprintf (buf2, MAX_STRING_LENGTH, "blurred vision"); break;
	case SOMA_EYE_DOUBLE:		snprintf (buf2, MAX_STRING_LENGTH, "double vision"); break;
	case SOMA_EYE_DILATION:		snprintf (buf2, MAX_STRING_LENGTH, "dilated pupils"); break; 
	case SOMA_EYE_CONTRACTION:	snprintf (buf2, MAX_STRING_LENGTH, "contracted pupils"); break; 
	case SOMA_EYE_LACRIMATION:	snprintf (buf2, MAX_STRING_LENGTH, "lacrimation"); break;
	case SOMA_EYE_PTOSIS:		snprintf (buf2, MAX_STRING_LENGTH, "ptosis"); break;
	case SOMA_EAR_TINNITUS:		snprintf (buf2, MAX_STRING_LENGTH, "tinnitus"); break;
	case SOMA_EAR_DEAFNESS:		snprintf (buf2, MAX_STRING_LENGTH, "deafness"); break;
	case SOMA_EAR_EQUILLIBRIUM:	snprintf (buf2, MAX_STRING_LENGTH, "ear imbalance"); break;
	case SOMA_NOSE_ANOSMIA:		snprintf (buf2, MAX_STRING_LENGTH, "anosmia"); break;
	case SOMA_NOSE_RHINITIS:	snprintf (buf2, MAX_STRING_LENGTH, "rhinitis"); break;
	case SOMA_MOUTH_SALIVATION:	snprintf (buf2, MAX_STRING_LENGTH, "salivation"); break;
	case SOMA_MOUTH_TOOTHACHE:	snprintf (buf2, MAX_STRING_LENGTH, "toothache"); break;
	case SOMA_MOUTH_DRYNESS:	snprintf (buf2, MAX_STRING_LENGTH, "dry mouth"); break;
	case SOMA_MOUTH_HALITOSIS:	snprintf (buf2, MAX_STRING_LENGTH, "halitosis"); break;
	case SOMA_CHEST_DIFFICULTY:	snprintf (buf2, MAX_STRING_LENGTH, "difficulty breathing"); break;
	case SOMA_CHEST_WHEEZING:	snprintf (buf2, MAX_STRING_LENGTH, "wheezing"); break;
	case SOMA_CHEST_RAPIDBREATH:	snprintf (buf2, MAX_STRING_LENGTH, "rapid breathing"); break;
	case SOMA_CHEST_SLOWBREATH:	snprintf (buf2, MAX_STRING_LENGTH, "shallow breathing"); break;
	case SOMA_CHEST_FLUID:		snprintf (buf2, MAX_STRING_LENGTH, "fluidous lungs"); break;
	case SOMA_CHEST_PALPITATIONS:	snprintf (buf2, MAX_STRING_LENGTH, "heart palpitations"); break;
	case SOMA_CHEST_COUGHING:	snprintf (buf2, MAX_STRING_LENGTH, "coughing fits"); break;
	case SOMA_CHEST_PNEUMONIA:	snprintf (buf2, MAX_STRING_LENGTH, "pneumonia"); break;
	case SOMA_NERVES_PSYCHOSIS:	snprintf (buf2, MAX_STRING_LENGTH, "psychosis"); break;
	case SOMA_NERVES_DELIRIUM:	snprintf (buf2, MAX_STRING_LENGTH, "delerium "); break;
	case SOMA_NERVES_COMA:		snprintf (buf2, MAX_STRING_LENGTH, "a comatose state"); break;
	case SOMA_NERVES_CONVULSIONS:	snprintf (buf2, MAX_STRING_LENGTH, "convulsions"); break;
	case SOMA_NERVES_HEADACHE:	snprintf (buf2, MAX_STRING_LENGTH, "headache"); break;
	case SOMA_NERVES_CONFUSION:	snprintf (buf2, MAX_STRING_LENGTH, "confusion"); break;
	case SOMA_NERVES_PARETHESIAS:	snprintf (buf2, MAX_STRING_LENGTH, "parethesias"); break;
	case SOMA_NERVES_ATAXIA:	snprintf (buf2, MAX_STRING_LENGTH, "ataxia"); break;
	case SOMA_NERVES_EQUILLIBRIUM:	snprintf (buf2, MAX_STRING_LENGTH, "nervous imbalance"); break;
	case SOMA_SKIN_CYANOSIS:	snprintf (buf2, MAX_STRING_LENGTH, "cyanosis of the skin"); break;
	case SOMA_SKIN_DRYNESS:		snprintf (buf2, MAX_STRING_LENGTH, "dryness of the skin"); break;
	case SOMA_SKIN_CORROSION:	snprintf (buf2, MAX_STRING_LENGTH, "corrosion of the skin"); break;
	case SOMA_SKIN_JAUNDICE:	snprintf (buf2, MAX_STRING_LENGTH, "jaundice of the skin"); break;
	case SOMA_SKIN_REDNESS:		snprintf (buf2, MAX_STRING_LENGTH, "redness of the skin"); break;
	case SOMA_SKIN_RASH:		snprintf (buf2, MAX_STRING_LENGTH, "a rash on the skin"); break;
	case SOMA_SKIN_HAIRLOSS:	snprintf (buf2, MAX_STRING_LENGTH, "hairloss"); break;
	case SOMA_SKIN_EDEMA:		snprintf (buf2, MAX_STRING_LENGTH, "edema of the skin"); break;
	case SOMA_SKIN_BURNS:		snprintf (buf2, MAX_STRING_LENGTH, "burns on the skin"); break;
	case SOMA_SKIN_PALLOR:		snprintf (buf2, MAX_STRING_LENGTH, "pallor of the skin"); break;
	case SOMA_SKIN_SWEATING:	snprintf (buf2, MAX_STRING_LENGTH, "the sweats"); break;
	case SOMA_GENERAL_WEIGHTLOSS:	snprintf (buf2, MAX_STRING_LENGTH, "weight loss"); break;
	case SOMA_GENERAL_LETHARGY:	snprintf (buf2, MAX_STRING_LENGTH, "lethargy"); break;
	case SOMA_GENERAL_APPETITELOSS:	snprintf (buf2, MAX_STRING_LENGTH, "appetite loss"); break;
	case SOMA_GENERAL_PRESSUREDROP:	snprintf (buf2, MAX_STRING_LENGTH, "low blood pressure"); break;
	case SOMA_GENERAL_PRESSURERISE:	snprintf (buf2, MAX_STRING_LENGTH, "high blood pressure"); break;
	case SOMA_GENERAL_FASTPULSE:	snprintf (buf2, MAX_STRING_LENGTH, "a fast pulse"); break;
	case SOMA_GENERAL_SLOWPULSE:	snprintf (buf2, MAX_STRING_LENGTH, "a slow pulse"); break;
	case SOMA_GENERAL_HYPERTHERMIA:	snprintf (buf2, MAX_STRING_LENGTH, "hyperthermia"); break;
	case SOMA_GENERAL_HYPOTHERMIA:	snprintf (buf2, MAX_STRING_LENGTH, "hypothermia"); break;
	default: snprintf (buf2, MAX_STRING_LENGTH, "an unknown somatic effect"); break;
	}
	
	snprintf (buf, MAX_STRING_LENGTH,  "#2%5d#0   Suffers from %s for %d more in-game hours.\n        Latency: %d hrs Power: %d to %d (%d @ %d min)\n        A: %d min, D: %d min, S: %d min, R: %d min\n", 
		af->type, buf2, af->a.soma.duration,
		af->a.soma.latency, af->a.soma.max_power, af->a.soma.lvl_power, 
		af->a.soma.atm_power, af->a.soma.minute,
		af->a.soma.attack, af->a.soma.decay,
		af->a.soma.sustain, af->a.soma.release
		);
	send_to_char (buf, ch);
}



void soma_ten_second_affect(CHAR_DATA *ch, AFFECTED_TYPE *af)
{
	int save = 0, stat = 0;
	stat = GET_CON(ch);
	if ( (number (1,1000) > af->a.soma.atm_power) || (number(1,(stat>20)?stat:20) <= stat) ) return;

	switch(af->type) {
	  /*	case SOMA_MUSCULAR_CRAMP:	snprintf (buf2, MAX_STRING_LENGTH, "a muscle cramp"); break;
		case SOMA_MUSCULAR_TWITCHING:	snprintf (buf2, MAX_STRING_LENGTH, "twitching"); break;
		case SOMA_MUSCULAR_TREMOR:	snprintf (buf2, MAX_STRING_LENGTH, "tremors"); break;
		case SOMA_MUSCULAR_PARALYSIS:	snprintf (buf2, MAX_STRING_LENGTH, "paralysis"); break;
		case SOMA_DIGESTIVE_ULCER:	snprintf (buf2, MAX_STRING_LENGTH, "stomach ulcer"); break; 
		case SOMA_DIGESTIVE_VOMITING:	snprintf (buf2, MAX_STRING_LENGTH, "vomiting"); break;
		case SOMA_DIGESTIVE_BLEEDING:	snprintf (buf2, MAX_STRING_LENGTH, "vomiting blood"); break; 
		case SOMA_EYE_BLINDNESS:	snprintf (buf2, MAX_STRING_LENGTH, "blindness"); break;
		case SOMA_EYE_BLURRED:		snprintf (buf2, MAX_STRING_LENGTH, "blurred vision"); break;
		case SOMA_EYE_DOUBLE:		snprintf (buf2, MAX_STRING_LENGTH, "double vision"); break;
		case SOMA_EYE_DILATION:		snprintf (buf2, MAX_STRING_LENGTH, "dilated pupils"); break; 
		case SOMA_EYE_CONTRACTION:	snprintf (buf2, MAX_STRING_LENGTH, "contracted pupils"); break; 
		case SOMA_EYE_LACRIMATION:	snprintf (buf2, MAX_STRING_LENGTH, "lacrimation"); break;
		case SOMA_EYE_PTOSIS:		snprintf (buf2, MAX_STRING_LENGTH, "ptosis"); break;
		case SOMA_EAR_TINNITUS:		snprintf (buf2, MAX_STRING_LENGTH, "tinnitus"); break;
		case SOMA_EAR_DEAFNESS:		snprintf (buf2, MAX_STRING_LENGTH, "deafness"); break;
		case SOMA_EAR_EQUILLIBRIUM:	snprintf (buf2, MAX_STRING_LENGTH, "ear imbalance"); break;
		case SOMA_NOSE_ANOSMIA:		snprintf (buf2, MAX_STRING_LENGTH, "anosmia"); break;
		case SOMA_NOSE_RHINITIS:	snprintf (buf2, MAX_STRING_LENGTH, "rhinitis"); break;
		case SOMA_MOUTH_SALIVATION:	snprintf (buf2, MAX_STRING_LENGTH, "salivation"); break;
		case SOMA_MOUTH_TOOTHACHE:	snprintf (buf2, MAX_STRING_LENGTH, "toothache"); break;
		case SOMA_MOUTH_DRYNESS:	snprintf (buf2, MAX_STRING_LENGTH, "dry mouth"); break;
		case SOMA_MOUTH_HALITOSIS:	snprintf (buf2, MAX_STRING_LENGTH, "halitosis"); break;
		case SOMA_CHEST_DIFFICULTY:	snprintf (buf2, MAX_STRING_LENGTH, "difficulty breathing"); break;
		case SOMA_CHEST_WHEEZING:	snprintf (buf2, MAX_STRING_LENGTH, "wheezing"); break;
		case SOMA_CHEST_RAPIDBREATH:	snprintf (buf2, MAX_STRING_LENGTH, "rapid breathing"); break;
		case SOMA_CHEST_SLOWBREATH:	snprintf (buf2, MAX_STRING_LENGTH, "shallow breathing"); break;
		case SOMA_CHEST_FLUID:		snprintf (buf2, MAX_STRING_LENGTH, "fluidous lungs"); break;
		case SOMA_CHEST_PALPITATIONS:	snprintf (buf2, MAX_STRING_LENGTH, "heart palpitations"); break;*/

	case SOMA_CHEST_COUGHING:

		stat = GET_WIL ( ch );
		save = number ( 1, (stat>20)?stat:20 );
				
		if ( get_affect (ch, MAGIC_HIDDEN) && would_reveal (ch)) {
			if ( save > stat ) {
				remove_affect_type (ch, MAGIC_HIDDEN);
				act ("$n reveals $mself with an audible cough.", TRUE, ch, 0, 0, TO_ROOM);
			}
			else if ( save > ( stat / 2 ) ) {
				act ("You hear a muffled sound from somewhere nearby.", TRUE, ch, 0, 0, TO_ROOM);
			}
		}
		else if ( ( save <= stat ) && ( save > ( stat / 2 ) )) {
			act ("$n tries to stifle a cough.", TRUE, ch, 0, 0, TO_ROOM);
		}

		if ( save > stat ) {
			act ("You cough audibly.", TRUE, ch, 0, 0, TO_CHAR);
		}
		else {
			act ("You try to stifle a cough silently.", TRUE, ch, 0, 0, TO_CHAR);
		}
		break;


    /*	case SOMA_CHEST_PNEUMONIA:	snprintf (buf2, MAX_STRING_LENGTH, "pneumonia"); break;
	case SOMA_NERVES_PSYCHOSIS:	snprintf (buf2, MAX_STRING_LENGTH, "psychosis"); break;
	case SOMA_NERVES_DELIRIUM:	snprintf (buf2, MAX_STRING_LENGTH, "delerium "); break;
	case SOMA_NERVES_COMA:		snprintf (buf2, MAX_STRING_LENGTH, "a comatose state"); break;
	case SOMA_NERVES_CONVULSIONS:	snprintf (buf2, MAX_STRING_LENGTH, "convulsions"); break;
	case SOMA_NERVES_HEADACHE:	snprintf (buf2, MAX_STRING_LENGTH, "headache"); break;
	case SOMA_NERVES_CONFUSION:	snprintf (buf2, MAX_STRING_LENGTH, "confusion"); break;
	case SOMA_NERVES_PARETHESIAS:	snprintf (buf2, MAX_STRING_LENGTH, "parethesias"); break;
	case SOMA_NERVES_ATAXIA:	snprintf (buf2, MAX_STRING_LENGTH, "ataxia"); break;
	case SOMA_NERVES_EQUILLIBRIUM:	snprintf (buf2, MAX_STRING_LENGTH, "nervous imbalance"); break;
	case SOMA_SKIN_CYANOSIS:	snprintf (buf2, MAX_STRING_LENGTH, "cyanosis of the skin"); break;
	case SOMA_SKIN_DRYNESS:		snprintf (buf2, MAX_STRING_LENGTH, "dryness of the skin"); break;
	case SOMA_SKIN_CORROSION:	snprintf (buf2, MAX_STRING_LENGTH, "corrosion of the skin"); break;
	case SOMA_SKIN_JAUNDICE:	snprintf (buf2, MAX_STRING_LENGTH, "jaundice of the skin"); break;
	case SOMA_SKIN_REDNESS:		snprintf (buf2, MAX_STRING_LENGTH, "redness of the skin"); break;
	case SOMA_SKIN_RASH:		snprintf (buf2, MAX_STRING_LENGTH, "a rash on the skin"); break;
	case SOMA_SKIN_HAIRLOSS:	snprintf (buf2, MAX_STRING_LENGTH, "hairloss"); break;
	case SOMA_SKIN_EDEMA:		snprintf (buf2, MAX_STRING_LENGTH, "edema of the skin"); break;
	case SOMA_SKIN_BURNS:		snprintf (buf2, MAX_STRING_LENGTH, "burns on the skin"); break;
	case SOMA_SKIN_PALLOR:		snprintf (buf2, MAX_STRING_LENGTH, "pallor of the skin"); break;
	case SOMA_SKIN_SWEATING:	snprintf (buf2, MAX_STRING_LENGTH, "the sweats"); break;
	case SOMA_GENERAL_WEIGHTLOSS:	snprintf (buf2, MAX_STRING_LENGTH, "weight loss"); break;
	case SOMA_GENERAL_LETHARGY:	snprintf (buf2, MAX_STRING_LENGTH, "lethargy"); break;
	case SOMA_GENERAL_APPETITELOSS:	snprintf (buf2, MAX_STRING_LENGTH, "appetite loss"); break;
	case SOMA_GENERAL_PRESSUREDROP:	snprintf (buf2, MAX_STRING_LENGTH, "low blood pressure"); break;
	case SOMA_GENERAL_PRESSURERISE:	snprintf (buf2, MAX_STRING_LENGTH, "high blood pressure"); break;
	case SOMA_GENERAL_FASTPULSE:	snprintf (buf2, MAX_STRING_LENGTH, "a fast pulse"); break;
	case SOMA_GENERAL_SLOWPULSE:	snprintf (buf2, MAX_STRING_LENGTH, "a slow pulse"); break;
	case SOMA_GENERAL_HYPERTHERMIA:	snprintf (buf2, MAX_STRING_LENGTH, "hyperthermia"); break;
	case SOMA_GENERAL_HYPOTHERMIA:	snprintf (buf2, MAX_STRING_LENGTH, "hypothermia"); break;
								 */
  default: break;
  }
}


void soma_rl_minute_affect(CHAR_DATA *ch, AFFECTED_TYPE *af)
{
	unsigned short int	minute = ++af->a.soma.minute;
	unsigned short int	max_power = af->a.soma.max_power;
	unsigned short int	lvl_power = af->a.soma.lvl_power;

	unsigned short int	attack = af->a.soma.attack;
	unsigned short int	decay = af->a.soma.decay;
	unsigned short int	sustain = af->a.soma.sustain;
	unsigned short int	release = af->a.soma.release;

	switch(af->type) {
	  /*	case SOMA_MUSCULAR_CRAMP:	snprintf (buf2, MAX_STRING_LENGTH, "a muscle cramp"); break;
		case SOMA_MUSCULAR_TWITCHING:	snprintf (buf2, MAX_STRING_LENGTH, "twitching"); break;
		case SOMA_MUSCULAR_TREMOR:	snprintf (buf2, MAX_STRING_LENGTH, "tremors"); break;
		case SOMA_MUSCULAR_PARALYSIS:	snprintf (buf2, MAX_STRING_LENGTH, "paralysis"); break;
		case SOMA_DIGESTIVE_ULCER:	snprintf (buf2, MAX_STRING_LENGTH, "stomach ulcer"); break; 
		case SOMA_DIGESTIVE_VOMITING:	snprintf (buf2, MAX_STRING_LENGTH, "vomiting"); break;
		case SOMA_DIGESTIVE_BLEEDING:	snprintf (buf2, MAX_STRING_LENGTH, "vomiting blood"); break; 
		case SOMA_EYE_BLINDNESS:	snprintf (buf2, MAX_STRING_LENGTH, "blindness"); break;
		case SOMA_EYE_BLURRED:		snprintf (buf2, MAX_STRING_LENGTH, "blurred vision"); break;
		case SOMA_EYE_DOUBLE:		snprintf (buf2, MAX_STRING_LENGTH, "double vision"); break;
		case SOMA_EYE_DILATION:		snprintf (buf2, MAX_STRING_LENGTH, "dilated pupils"); break; 
		case SOMA_EYE_CONTRACTION:	snprintf (buf2, MAX_STRING_LENGTH, "contracted pupils"); break; 
		case SOMA_EYE_LACRIMATION:	snprintf (buf2, MAX_STRING_LENGTH, "lacrimation"); break;
		case SOMA_EYE_PTOSIS:		snprintf (buf2, MAX_STRING_LENGTH, "ptosis"); break;
		case SOMA_EAR_TINNITUS:		snprintf (buf2, MAX_STRING_LENGTH, "tinnitus"); break;
		case SOMA_EAR_DEAFNESS:		snprintf (buf2, MAX_STRING_LENGTH, "deafness"); break;
		case SOMA_EAR_EQUILLIBRIUM:	snprintf (buf2, MAX_STRING_LENGTH, "ear imbalance"); break;
		case SOMA_NOSE_ANOSMIA:		snprintf (buf2, MAX_STRING_LENGTH, "anosmia"); break;
		case SOMA_NOSE_RHINITIS:	snprintf (buf2, MAX_STRING_LENGTH, "rhinitis"); break;
		case SOMA_MOUTH_SALIVATION:	snprintf (buf2, MAX_STRING_LENGTH, "salivation"); break;
		case SOMA_MOUTH_TOOTHACHE:	snprintf (buf2, MAX_STRING_LENGTH, "toothache"); break;
		case SOMA_MOUTH_DRYNESS:	snprintf (buf2, MAX_STRING_LENGTH, "dry mouth"); break;
		case SOMA_MOUTH_HALITOSIS:	snprintf (buf2, MAX_STRING_LENGTH, "halitosis"); break;
		case SOMA_CHEST_DIFFICULTY:	snprintf (buf2, MAX_STRING_LENGTH, "difficulty breathing"); break;
		case SOMA_CHEST_WHEEZING:	snprintf (buf2, MAX_STRING_LENGTH, "wheezing"); break;
		case SOMA_CHEST_RAPIDBREATH:	snprintf (buf2, MAX_STRING_LENGTH, "rapid breathing"); break;
		case SOMA_CHEST_SLOWBREATH:	snprintf (buf2, MAX_STRING_LENGTH, "shallow breathing"); break;
		case SOMA_CHEST_FLUID:		snprintf (buf2, MAX_STRING_LENGTH, "fluidous lungs"); break;
		case SOMA_CHEST_PALPITATIONS:	snprintf (buf2, MAX_STRING_LENGTH, "heart palpitations"); break;*/

	case SOMA_CHEST_COUGHING:		

		if ( minute <= attack ) {
			af->a.soma.atm_power = ( max_power * minute ) / attack;
		}
		else if ( minute <= decay ) {
			af->a.soma.atm_power = max_power - ( ( ( max_power - lvl_power ) * ( minute - attack ) ) / ( decay - attack ) );
		}
		else if ( minute <= sustain ) {
			af->a.soma.atm_power = lvl_power;
		}
		else if ( minute <= release ) {
			af->a.soma.atm_power = lvl_power - ( ( ( lvl_power ) * ( minute - sustain ) ) / ( release - sustain ) );
		}
		else {
			affect_remove (ch, af);
		}
		break;
		
	  /*
	    case SOMA_CHEST_PNEUMONIA:	snprintf (buf2, MAX_STRING_LENGTH, "pneumonia"); break;
	    case SOMA_NERVES_PSYCHOSIS:	snprintf (buf2, MAX_STRING_LENGTH, "psychosis"); break;
	    case SOMA_NERVES_DELIRIUM:	snprintf (buf2, MAX_STRING_LENGTH, "delerium "); break;
	    case SOMA_NERVES_COMA:		snprintf (buf2, MAX_STRING_LENGTH, "a comatose state"); break;
	    case SOMA_NERVES_CONVULSIONS:	snprintf (buf2, MAX_STRING_LENGTH, "convulsions"); break;
	    case SOMA_NERVES_HEADACHE:	snprintf (buf2, MAX_STRING_LENGTH, "headache"); break;
	    case SOMA_NERVES_CONFUSION:	snprintf (buf2, MAX_STRING_LENGTH, "confusion"); break;
	    case SOMA_NERVES_PARETHESIAS:	snprintf (buf2, MAX_STRING_LENGTH, "parethesias"); break;
	    case SOMA_NERVES_ATAXIA:	snprintf (buf2, MAX_STRING_LENGTH, "ataxia"); break;
	    case SOMA_NERVES_EQUILLIBRIUM:	snprintf (buf2, MAX_STRING_LENGTH, "nervous imbalance"); break;
	    case SOMA_SKIN_CYANOSIS:	snprintf (buf2, MAX_STRING_LENGTH, "cyanosis of the skin"); break;
	    case SOMA_SKIN_DRYNESS:		snprintf (buf2, MAX_STRING_LENGTH, "dryness of the skin"); break;
	    case SOMA_SKIN_CORROSION:	snprintf (buf2, MAX_STRING_LENGTH, "corrosion of the skin"); break;
	    case SOMA_SKIN_JAUNDICE:	snprintf (buf2, MAX_STRING_LENGTH, "jaundice of the skin"); break;
	    case SOMA_SKIN_REDNESS:		snprintf (buf2, MAX_STRING_LENGTH, "redness of the skin"); break;
	    case SOMA_SKIN_RASH:		snprintf (buf2, MAX_STRING_LENGTH, "a rash on the skin"); break;
	    case SOMA_SKIN_HAIRLOSS:	snprintf (buf2, MAX_STRING_LENGTH, "hairloss"); break;
	    case SOMA_SKIN_EDEMA:		snprintf (buf2, MAX_STRING_LENGTH, "edema of the skin"); break;
	    case SOMA_SKIN_BURNS:		snprintf (buf2, MAX_STRING_LENGTH, "burns on the skin"); break;
	    case SOMA_SKIN_PALLOR:		snprintf (buf2, MAX_STRING_LENGTH, "pallor of the skin"); break;
	    case SOMA_SKIN_SWEATING:	snprintf (buf2, MAX_STRING_LENGTH, "the sweats"); break;
	    case SOMA_GENERAL_WEIGHTLOSS:	snprintf (buf2, MAX_STRING_LENGTH, "weight loss"); break;
	    case SOMA_GENERAL_LETHARGY:	snprintf (buf2, MAX_STRING_LENGTH, "lethargy"); break;
	    case SOMA_GENERAL_APPETITELOSS:	snprintf (buf2, MAX_STRING_LENGTH, "appetite loss"); break;
	    case SOMA_GENERAL_PRESSUREDROP:	snprintf (buf2, MAX_STRING_LENGTH, "low blood pressure"); break;
	    case SOMA_GENERAL_PRESSURERISE:	snprintf (buf2, MAX_STRING_LENGTH, "high blood pressure"); break;
	    case SOMA_GENERAL_FASTPULSE:	snprintf (buf2, MAX_STRING_LENGTH, "a fast pulse"); break;
	    case SOMA_GENERAL_SLOWPULSE:	snprintf (buf2, MAX_STRING_LENGTH, "a slow pulse"); break;
	    case SOMA_GENERAL_HYPERTHERMIA:	snprintf (buf2, MAX_STRING_LENGTH, "hyperthermia"); break;
	    case SOMA_GENERAL_HYPOTHERMIA:	snprintf (buf2, MAX_STRING_LENGTH, "hypothermia"); break;
	  */
	default: break;
	}
}
