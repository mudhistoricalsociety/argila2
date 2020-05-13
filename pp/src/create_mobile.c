/** 
*	\file create_mobile.c
*	Mobile Autocreation Module
*
*	This module deals with creating mobs with variable descriptions for random
*	mobs within certain styles as enforcers, general populace, and wildlife 
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
#include <unistd.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "decl.h"

const char *cat_adj1 [] = {
	"albino",
	"beautiful",
	"black-striped",
	"brown-striped",
	"brown",
	"chalky-coated",
	"charcoal-grey",
	"coal-black",
	"compact",
	"dainty",
	"dappled",
	"dark-brown",
	"dark-grey",
	"delicate",
	"dusty-grey",
	"elegant",
	"fierce",
	"gorgeous",
	"gray-furred",
	"grey-striped",
	"grey",
	"inky-furred",
	"jet-black",
	"lanky",
	"large",
	"lean",
	"light-brown",
	"light-grey",
	"mottle-coated",
	"muscular",
	"sable",
	"sinewy",
	"sleek",
	"small",
	"spotted",
	"striped",
	"tawny",
	"tiny",
	"well-groomed",
	"white",
	"\n"
};

const char *cat_adj2 [] = {
	"black-eyed",
	"blue-eyed",
	"brown-eyed",
	"crook-tailed",
	"glassy-eyed",
	"glint-eyed",
	"golden-eyed",
	"grey-eyed",
	"hook-tailed",
	"keen-eyed",
	"large-pawed",
	"large-tailed",
	"long-haired",
	"long-legged",
	"long-muzzled",
	"long-necked",
	"long-tailed",
	"narrow-eyed",
	"one-eared",
	"sharp-eared",
	"sharp-eyed",
	"short-legged",
	"short-muzzled",
	"short-necked",
	"short-tailed",
	"white-eyed",
	"white-pawed",
	"wide-eared",
	"wide-eyed",
	"\n"
};

const char *spider_adj1 [] = {
	"brown-striped",
	"coal-black",
	"grey",
	"red-eyed",
	"albino",
	"grey-striped",
	"black-eyed",
	"beady-eyed",
	"loathesome",
	"vile",
	"red-striped",
	"brown",
	"black-striped",
	"dark-green",
	"thick-carapiced",
	"repulsive",
	"green-striped",
	"dusty-grey",
	"\n"
};

const char *spider_adj2 [] = {
	"long-legged",
	"hairy",
	"hairless",
	"hairy-legged",
	"blotchy",
	"black-haired",
	"spikey",
	"ooze-crusted",
	"squat",
	"spindly-legged",
	"greasy-bodied",
	"dust-covered",
	"bloated",
	"nimble",
	"thick-bodied",
	"smooth",
	"sleek",
	"\n"
};

const char *rat_adj1 [] = {
	"brown",
	"ochre-furred",
	"grey",
	"chalky-coated",
	"charcoal-furred",
	"sable",
	"albino",
	"white",
	"red-eyed",
	"yellow-eyed",
	"black-eyed",
	"glossy-eyed",
	"dull-eyed",
	"long-tailed",
	"short-tailed",
	"tailless",
	"sharp-toothed",
	"toothless",
	"jagged-roothed",
	"rotting-toothed",
	"large-pawed",
	"small-pawed",
	"\n"
};

const char *rat_adj2 [] = {
	"scrawny",
	"emaciated",
	"gaunt",
	"puny",
	"bony",
	"raw-boned",
	"malnourished",
	"lean",
	"scab-skinned",
	"foul",
	"greasy",
	"filthy",
	"dirt-covered",
	"disease-ridden",
	"vile",
	"nimble",
	"large",
	"slick",
	"sleek",
	"agile",
	"small",
	"maimed",
	"crippled",
	"\n"
};


const char *bird_adj1 [] = {
	"tiny",
	"miniscule",
	"small",
	"minute",
	"large",
	"sleek",
	"elegant",
	"plump",
	"majestic",
	"beautiful",
	"predatory",
	"gorgeous",
	"broad-winged",
	"wide-winged",
	"large-winged",
	"brightly-plumed",
	"wildly-plumed",
	"fiery-plumed",
	"dull-plumed",
	"drab-plumed",
	"darkly-plumed",
	"ebon-plumed",
	"reddish-plumed",
	"glossy-feathered",
	"fluffy-feathered",
	"bright-red-feathered",
	"drab-feathered",
	"dun-feathered",
	"white-feathered",
	"pink-feathered",
	"greyish-brown-feathered",
	"brownish-grey-feathered",
	"dull-feathered",
	"black-feathered",
	"blue-black-feathered",
	"midnight-feathered",
	"brown-feathered",
	"tan-feathered",
	"beryl-feathered",
	"blue-feathered",
	"dull-red-feathered",
	"dull-blue-feathered",
	"grey-feathered",
	"dull-grey-feathered",
	"striped",
	"spotted",
	"dappled",
	"\n"
};

const char *bird_adj2 [] = {
	"sharp-beaked",
	"pointy-beaked",
	"wide-beaked",
	"broad-beaked",
	"small-beaked",
	"narrow-beaked",
	"large-beaked",
	"huge-beaked",
	"spear-beaked",
	"hook-beaked",
	"wide-billed",
	"broad-billed",
	"small-billed",
	"large-billed",
	"hook-billed",
	"sharp-eyed",
	"black-eyed",
	"wide-eyed",
	"sharp-taloned",
	"fiercely-taloned",
	"large-taloned",
	"long-tailed",
	"short-tailed",
	"wide-tailed",
	"thin-tailed",
	"fan-tailed",
	"large-tailed",
	"long-necked",
	"short-necked",
	"long-legged",
	"short-legged",
	"crowned",
	"\n"
};

const char *war_horse_adj1 [] = {
	"regal",
	"well-muscled",
	"heavily-muscled",
	"thickly-muscled",
	"delicate",
	"sinewy",
	"hale",
	"noble",
	"hearty",
	"sturdy",
	"proud",
	"striking",
	"massive",
	"compact",
	"narrow-chested",
	"massive-chested",
	"broad-chested",
	"deep-chested",
	"long-backed",
	"well-groomed",
	"elegant",
	"dainty",
	"\n"
};

const char *horse_adj1 [] = {
	"dappled",
	"dun-coated",
	"bay-colored",
	"grey-coated",
	"black-coated",
	"roan-colored",
	"chestnut-coated",
	"spotty-coated",
	"brilliant-white-coated",
	"obsidian-colored",
	"blue-black-coated",
	"midnight-black-colored",
	"pearl-white-coated",
	"flaxen-coated",
	"blotch-brown-coated",
	"mottled-brown-coated",
	"blotchy-grey-coated",
	"mottled-grey-coated",
	"smoky-grey-coated",
	"ash-grey-coated",
	"ebon-coated",
	"white-coated",
	"dark-brown-colored",
	"slate-grey-coated",
	"cream-colored",
	"glossy-coated",
	"curly-coated",
	"rough-coated",
	"\n"
};

const char *horse_adj2 [] = {
	"slender-legged",
	"long-legged",
	"short-legged",
	"straight-legged",
	"shaggy-fetlocked",
	"long-necked",
	"short-necked",
	"arch-necked",
	"wide-necked",
	"long-muzzled",
	"short-muzzled",
	"short-tailed",
	"wavy-tailed",
	"long-tailed",
	"flowing-maned",
	"silky-maned",
	"short-maned",
	"abundantly-maned",
	"coarse-maned",
	"white-maned",
	"ebon-maned",
	"brown-maned",
	"auburn-maned",
	"dun-maned",
	"wiry-maned",
	"thick-maned",
	"grey-maned",
	"black-maned",
	"long-maned",
	"wavy-maned",
	"long-eared",
	"small-eared",
	"amber-eyed",
	"black-eyed",
	"glint-eyed",
	"\n"
};



const char *wolf_adj1 [] = {
	"sinewy",
	"muscular",
	"powerfully-muscled",
	"thickly-muscled",
	"gaunt",
	"bony",
	"skeletal",
	"rawboned",
	"emaciated",
	"lean",
	"lanky",
	"tawny",
	"grizzled",
	"scraggly",
	"fierce",
	"mangy",
	"thickly-furred",
	"mottle-coated",
	"sleek",
	"brambly-furred",
	"wiry-furred",
	"shaggy-furred",
	"long-whiskered",
	"silken-furred",
	"bristly-furred",
	"spiky-furred",
	"gray-furred",
	"smoky-grey-furred",
	"ash-grey-furred",
	"black-furred",
	"ebony-furred",
	"sooty-grey-furred",
	"charcoal-grey-furred",
	"coal-black-furred",
	"inky-furred",
	"ebon-furred",
	"brown-furred",
	"muddy-brown-furred",
	"dark-brown-furred",
	"jet-black",
	"light-grey",
	"dark-grey",
	"light-brown",
	"dark-brown",
	"muddy-brown",
	"matted-furred",
	"\n"
};

const char *wolf_adj2 [] = {
	"narrow-muzzled",
	"sharp-muzzled",
	"short-muzzled",
	"long-muzzled",
	"crook-tailed",
	"hook-tailed",
	"long-necked",
	"sharp-eared",
	"wide-eared",
	"one-eared",
	"narrow-eyed",
	"glassy-eyed",
	"black-eyed",
	"grey-eyed",
	"blue-eyed",
	"white-eyed",
	"glint-eyed",
	"razor-fanged",
	"yellow-fanged",
	"frothy-mawed",
	"sharp-toothed",
	"long-legged",
	"large-pawed",
	"golden-eyed",
	"black-eyed",
	"brown-eyed",
	"keen-eyed",
	"large-pawed",
	"white-pawed",
	"feral",
	"\n"
};


const char *human_adj1 [] = {
	"acned",
	"cadaverous",
	"dirty",
	"dust-covered",
	"doughy",
	"fair",
	"greasy",
	"jaundiced",
	"pale",
	"livid",
	"pallid",
	"hearty",
	"scarred",
	"sun-browned",
	"swarthy",
	"wan",
	"waxy",
	"weatherbeaten",
	"almond-eyed",
	"beady-eyed",
	"cock-eyed",
	"owlish",
	"rheumy-eyed",
	"squinty-eyed",
	"aquiline-nosed",
	"beak-nosed",
	"bent-nosed",
	"knob-nosed",
	"flat-nosed",
	"hawk-nosed",
	"pig-nosed",
	"pug-nosed",
	"athletic",
	"brawny",
	"bent",
	"bow-spined",
	"burly",
	"chubby",
	"colossal",
	"brawny",
	"delicate",
	"diminutive",
	"lithe",
	"large",
	"thin",
	"fat",
	"fleshy",
	"fragile",
	"gangly",
	"gaunt",
	"haggard",
	"hunched",
	"husky",
	"lanky",
	"lean",
	"lithe",
	"lissome",
	"lissome",
	"lithe",
	"muscled",
	"obese",
	"lanky",
	"paunchy",
	"slender",
	"petite",
	"portly",
	"pot-bellied",
	"pudgy",
	"reedy",
	"rickety",
	"willowy",
	"robust",
	"rotund",
	"rugged",
	"scrawny",
	"runty",
	"sinewy",
	"runty",
	"skeletal",
	"sleek",
	"slight",
	"slender",
	"slim",
	"spindly",
	"squat",
	"stalwart",
	"statuesque",
	"svelte",
	"tall",
	"thickset",
	"thin",
	"waspish",
	"well-muscled",
	"whip-thin",
	"willowy",
	"wiry",
	"blue-eyed",
	"azure-eyed",
	"green-eyed",
	"emerald-eyed",
	"jade-eyed",
	"brown-eyed",
	"chocolate-eyed",
	"dark-eyed",
	"grey-eyed",
	"stormy-eyed",
	"hazel-eyed",
	"\n"
};

const char *human_adj2 [] = {
	"black-haired",
	"coal-haired",
	"ebony-haired",
	"jet-haired",
	"midnight-haired",
	"onyx-haired",
	"raven-haired",
	"auburn-haired",
	"copper-haired",
	"red-haired",
	"scarlet-haired",
	"sepia-haired",
	"blonde-haired",
	"golden-haired",
	"ginger-haired",
	"honey-haired",
	"flaxen-haired",
	"sandy-haired",
	"sorrel-haired",
	"tawny-haired",
	"bronze-haired",
	"brown-haired",
	"chestnut-haired",
	"dun-haired",
	"russet-haired",
	"sable-haired",
	"taupe-haired",
	"wheat-haired",
	"henna-haired",
	"dusky-haired",
	"ecru-haired",
	"angular-faced",
	"aristocratic",
	"comely-faced",
	"careworn",
	"cherubic",
	"comely-faced",
	"drawn-faced",
	"feline-faced",
	"narrow-faced",
	"square-faced",
	"stoop-shouldered",
	"broad-shouldered",
	"drooping-shouldered",
	"delicate-shouldered",
	"\n"
};



char *return_adj2 (CHAR_DATA *mob)
{
	int		roll, limit;
	static char	adj [MAX_STRING_LENGTH];

	if ( !str_cmp (db_race_table [mob->race].name, "Human") ) {
		for ( limit = 0; *human_adj2[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", human_adj2[roll]);
		return adj;
	}
	
	else if ( !str_cmp (db_race_table [mob->race].name, "Wolf") ) {
		for ( limit = 0; *wolf_adj2[limit] != '\n'; limit++ )	
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", wolf_adj2[roll]);
		return adj;
	}
	
	else if ( !str_cmp (db_race_table [mob->race].name, "Horse") ||
		  !str_cmp (db_race_table [mob->race].name, "WarHorse") ) {
		for ( limit = 0; *horse_adj2[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", horse_adj2[roll]);
		return adj;
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Bird") ) {
		for ( limit = 0; *bird_adj2[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", bird_adj2[roll]);
		return adj;
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Rat") ) {
		for ( limit = 0; *rat_adj2[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", rat_adj2[roll]);
		return adj;
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Arachnid") ) {
		for ( limit = 0; *spider_adj2[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", spider_adj2[roll]);
		return adj;
	}
	
	else if ( !str_cmp (db_race_table [mob->race].name, "Cat") ) {
		for ( limit = 0; *cat_adj2[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", cat_adj2[roll]);
		return adj;
	}

	return NULL;
}

char *return_adj1 (CHAR_DATA *mob)
{
	int		roll, limit;
	static char	adj [MAX_STRING_LENGTH];

	if ( !str_cmp (db_race_table [mob->race].name, "Human") ) {
		for ( limit = 0; *human_adj1[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);
		snprintf (adj, MAX_STRING_LENGTH, "%s", human_adj1[roll]);
		return adj;
	}
	
	else if ( !str_cmp (db_race_table [mob->race].name, "Wolf") ) {
		for ( limit = 0; *wolf_adj1[limit] != '\n'; limit++ )	
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", wolf_adj1[roll]);
		return adj;
	}
		
	else if ( !str_cmp (db_race_table [mob->race].name, "Bird") ) {
		for ( limit = 0; *bird_adj1[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", bird_adj1[roll]);
		return adj;
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Rat") ) {
		for ( limit = 0; *rat_adj1[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", rat_adj1[roll]);
		return adj;
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Arachnid") ) {
		for ( limit = 0; *spider_adj1[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", spider_adj1[roll]);
		return adj;
	}
	
	else if ( !str_cmp (db_race_table [mob->race].name, "Horse") ||
		  !str_cmp (db_race_table [mob->race].name, "WarHorse") ) {
		if ( !str_cmp (db_race_table [mob->race].name, "WarHorse") ) {
			for ( limit = 0; *war_horse_adj1[limit] != '\n'; limit++ )
				;
			limit--;
			roll = number(0,limit);			
			snprintf (adj, MAX_STRING_LENGTH, "%s", war_horse_adj1[roll]);
			return adj;
		}
		else {
			for ( limit = 0; *horse_adj1[limit] != '\n'; limit++ )
				;
			limit--;
			roll = number(0,limit);			
			snprintf (adj, MAX_STRING_LENGTH, "%s", horse_adj1[roll]);
			return adj;
		}
	}
	
	else if ( !str_cmp (db_race_table [mob->race].name, "Cat") ) {
		for ( limit = 0; *cat_adj1[limit] != '\n'; limit++ )
			;
		limit--;
		roll = number(0,limit);			
		snprintf (adj, MAX_STRING_LENGTH, "%s", cat_adj1[roll]);
		return adj;
	}
	return NULL;
}

char *return_name (CHAR_DATA *mob)
{
	static char	buf [MAX_STRING_LENGTH];
	int		roll, roll2;

	*buf = '\0';

	if ( !str_cmp (db_race_table [mob->race].name, "Cat") ) {
		if ( GET_SEX(mob) == SEX_MALE )
			return "tom cat";
		else if ( GET_SEX(mob) == SEX_FEMALE )
			return "cat";
		else return "cat";
	}
	
	else if ( !str_cmp (db_race_table [mob->race].name, "Bird") )
		return "bird";
	else if ( !str_cmp (db_race_table [mob->race].name, "Arachnid") )
		return "spider";
	else if ( !str_cmp (db_race_table [mob->race].name, "Rat") )
		return "rat";
	else if ( !str_cmp (db_race_table [mob->race].name, "Horse") ||
		  !str_cmp (db_race_table [mob->race].name, "WarHorse") ) {
		if ( GET_SEX(mob) == SEX_MALE ) {
			if ( !number(0,4) )
				return "young stallion";
			return "stallion";
		}
		else if ( GET_SEX(mob) == SEX_FEMALE ) {
			if ( !number(0,4) )
				return "young mare";
			else return "mare";
		}
		else return "horse";
	}
	
	roll = number(0,5);

	if ( roll == 0 && !IS_SET (mob->act, ACT_ENFORCER) )
		snprintf (buf, MAX_STRING_LENGTH,  "old ");
	else if ( roll == 4 && !IS_SET(mob->act, ACT_ENFORCER) )
		snprintf (buf, MAX_STRING_LENGTH,  "young ");

	if ( IS_SET(mob->act, ACT_ENFORCER) ) {
		if ( GET_SEX(mob) == SEX_MALE )
			snprintf (buf, MAX_STRING_LENGTH,  "man");
		else
			snprintf (buf, MAX_STRING_LENGTH,  "woman");
		return buf;
	}

	if ( roll == 0 && mob->sex == SEX_MALE ) {
		roll2 = number(1,3);
		if ( roll2 == 1 )
			strcat (buf, "gaffer");
		else if ( roll2 == 2 )
			strcat (buf, "geezer");
		else if ( roll2 == 3 )
			strcat (buf, "man");
		else strcat (buf, "man");
	}
	else if ( roll == 0 && mob->sex == SEX_FEMALE ) {
		roll2 = number(1,7);
		if ( roll2 == 1 )
			strcat (buf, "crone");
		else if ( roll2 == 2 )
			strcat (buf, "harridan");
		else if ( roll2 == 3 )
			strcat (buf, "matron");
		else if ( roll2 == 4 )
			strcat (buf, "spinster");
		else strcat (buf, "woman");
	}
	else if ( roll == 4 && mob->sex == SEX_MALE ) {
		roll2 = number(1,4);
		if ( roll2 == 1 )
			strcat (buf, "lad");
		else if ( roll2 == 2 )
			strcat (buf, "waif");
		else strcat (buf, "man");
	}
	else if ( roll == 4 && mob->sex == SEX_FEMALE ) {
		roll2 = number(1,4);
		if ( roll2 == 1 )
			strcat (buf, "lass");
		else if ( roll2 == 2 )
			strcat (buf, "maid");
		else strcat (buf, "woman");
	}
	else if ( mob->sex == SEX_MALE )
		strcat (buf, "man");
	else if ( mob->sex == SEX_FEMALE )
		strcat (buf, "woman");

	return buf;	
}

/*                                                                          *
 * function: create_description                                             *
 *                                                                          *
 * 09/28/2004 [JWW] - Added travel strings some arbitrary mobs              *
 *                                                                          */
void create_description (CHAR_DATA *mob)
{
	char	sdesc_frame [MAX_STRING_LENGTH] = {'\0'};
	char	sdesc [MAX_STRING_LENGTH] = {'\0'};
	char	adj1 [MAX_STRING_LENGTH] = {'\0'};
	char	adj2 [MAX_STRING_LENGTH] = {'\0'};
	char	name [MAX_STRING_LENGTH] = {'\0'};
	char	buf [MAX_STRING_LENGTH] = {'\0'};
	char	buf2 [MAX_STRING_LENGTH] = {'\0'};
	bool	found = FALSE;
	char	*temp_arg = NULL;
	int	roll, i, j;

	for ( i = 0; *variable_races[i] != '\n'; i++ )
		if ( !strcmp (variable_races[i], db_race_table [mob->race].name) )
			found = TRUE;

	if ( !found )
		return;

	if ( !number(0,1) ) {
		if ( !number(0,1) )
			snprintf (sdesc_frame, MAX_STRING_LENGTH, "$adj1, $adj2 $name");
		else
			snprintf (sdesc_frame, MAX_STRING_LENGTH, "$adj2, $adj1 $name");
	}
	else {
		if ( !number(0,1) )
			snprintf (sdesc_frame, MAX_STRING_LENGTH, "$adj1 $name");
		else
			snprintf (sdesc_frame, MAX_STRING_LENGTH, "$adj2 $name");
	}

	*sdesc = '\0';
	*adj1 = '\0';
	*adj2 = '\0';
	*name = '\0';
	*buf2 = '\0';

	temp_arg = return_name(mob);
	snprintf (name, MAX_STRING_LENGTH, "%s", temp_arg);

	for ( i = 0; i <= strlen(sdesc_frame); i++ ) {
		if ( sdesc_frame[i] == '$' ) {
			j = i;
			*buf = '\0';
			while ( sdesc_frame[i] && sdesc_frame[i] != ' ' && sdesc_frame[i] != ',' ) {
				snprintf (buf + strlen(buf), MAX_STRING_LENGTH,  "%c", sdesc_frame[i]);
				i++;
			}
			i = j;
			if ( !str_cmp (buf, "$adj1") ) {
				temp_arg = return_adj1(mob);
				snprintf (adj1, MAX_STRING_LENGTH, "%s", temp_arg);
				if ( !*sdesc && (adj1[0] == 'a' || adj1[0] == 'e' || adj1[0] == 'i' || adj1[0] == 'o' || adj1[0] == 'u') )
					snprintf (sdesc + strlen(sdesc), MAX_STRING_LENGTH, "an ");
				else if ( !*sdesc )
					snprintf (sdesc + strlen(sdesc), MAX_STRING_LENGTH, "a ");
				snprintf (sdesc + strlen(sdesc), MAX_STRING_LENGTH, "%s", adj1);
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%s ", adj1);
			}
			else if ( !str_cmp (buf, "$adj2") ) {
				temp_arg = return_adj2(mob);
				snprintf (adj2, MAX_STRING_LENGTH, "%s", temp_arg);
				if ( !*sdesc && (adj2[0] == 'a' || adj2[0] == 'e' || adj2[0] == 'i' || adj2[0] == 'o' || adj2[0] == 'u') )
					snprintf (sdesc + strlen(sdesc), MAX_STRING_LENGTH, "an ");
				else if ( !*sdesc )
					snprintf (sdesc + strlen(sdesc), MAX_STRING_LENGTH, "a ");
				while ( !str_cmp (adj1, adj2) ){
					temp_arg = return_adj2(mob);
					snprintf (adj2, MAX_STRING_LENGTH, "%s", temp_arg);
				}
				snprintf (sdesc + strlen(sdesc), MAX_STRING_LENGTH, "%s", adj2);
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%s ", adj2);
			}
			else if ( !str_cmp (buf, "$name") ) {
				snprintf (sdesc + strlen(sdesc), MAX_STRING_LENGTH, "%s", name);
				snprintf (buf2 + strlen(buf2), MAX_STRING_LENGTH,  "%s", name);
			}
			i += strlen(buf)-1;
			continue;
		}
		else snprintf (sdesc + strlen(sdesc), MAX_STRING_LENGTH, "%c", sdesc_frame[i]);
	}

	mob->delay_info1 = 0;

	if ( mob->short_descr )
		mem_free (mob->short_descr);
	mob->short_descr = add_hash(sdesc);
	*buf = '\0';
	if ( IS_SET (mob->act, ACT_ENFORCER) && IS_SET (mob->act, ACT_SENTINEL) ) {
		roll = number (1, 3);
		if ( roll == 1 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s stands at attention here.", sdesc);
		else if ( roll == 2 ) 
			snprintf (buf, MAX_STRING_LENGTH,  "%s stands here, watching for signs of trouble.", sdesc);
		else if ( roll == 3 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s patrols here, looking hawkishly about.", sdesc);
			mob->travel_str = add_hash("looking hawkishly about");
	}
	else if ( IS_SET (mob->act, ACT_ENFORCER) && !IS_SET (mob->act, ACT_SENTINEL) ) {
		roll = number (1, 3);
		if ( roll == 1 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s patrols here, looking for signs of trouble.", sdesc);
			mob->travel_str = add_hash("looking about purposefully");
		}
		else if ( roll == 2 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s moves by, watching the area attentively.", sdesc);
			mob->travel_str = add_hash("looking about the area attentively");
		}
		else if ( roll == 3 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s strides through, watching intently.", sdesc);
			mob->travel_str = add_hash("looking hawkishly about");
		}
	}
	
	else if ( !str_cmp (db_race_table [mob->race].name, "Horse") ||
		  !str_cmp (db_race_table [mob->race].name, "WarHorse") ) {
		roll = number (1, 5);
		if ( roll == 1 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s stands here, stamping the ground.", sdesc);
		else if ( roll == 2 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s is here, whickering softly.", sdesc);
		else if ( roll == 3 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s stands here quietly.", sdesc);
		else if ( roll == 4 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s is here, flicking its tail.", sdesc);
		else if ( roll == 5 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s stands here calmly.", sdesc);
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Bird") ) {
		roll = number (1,4);
		if ( roll == 1 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s perches here.", sdesc);
		else if ( roll == 2 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s perches here, observing quietly.", sdesc);
		else if ( roll == 3 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s flies through the area.", sdesc);
		else if ( roll == 4 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s is here, watching in silence.", sdesc);
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Cat") ) {
		roll = number (1, 4);
		if ( roll == 1 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s prowls through the area.", sdesc);
		else if ( roll == 2 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s paces here.", sdesc);
		
		else if ( roll == 3 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s pauses here, sniffing the air.", sdesc);
			mob->travel_str = add_hash("sniffing the air");
		}
		else if ( roll == 4 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s pads around, sniffing the ground.", sdesc);
			mob->travel_str = add_hash("sniffing the ground");
		}
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Wolf") ) {
		roll = number (1, 4);
		if ( roll == 1 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s prowls through the area.", sdesc);
		else if ( roll == 2 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s paces here.", sdesc);
		
		else if ( roll == 3 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s pauses here, sniffing the air.", sdesc);
			mob->travel_str = add_hash("sniffing the air");
		}
		else if ( roll == 4 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s pads around, sniffing the ground.", sdesc);
			mob->travel_str = add_hash("sniffing the ground");
		}
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Rat") ) {
		roll = number (1,7);
		if ( roll == 1 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s skitters around the area.", sdesc);
			mob->travel_str = add_hash("skittering about nervously");
		}
		else if ( roll == 2 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s is here, skulking about.", sdesc);
		else if ( roll == 3 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s moves quietly by.", sdesc);
		else if ( roll == 4 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s waits here, unmoving.", sdesc);
		else if ( roll == 5 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s is here.", sdesc);
		else if ( roll == 6 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s lies low to the ground here.", sdesc);
		else if ( roll == 7 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s sneaks about quietly.", sdesc);
			mob->travel_str = add_hash("padding along quietly in the shadows");
		}
	}
	else if ( !str_cmp (db_race_table [mob->race].name, "Arachnid") ) {
		roll = number (1,7);
		if ( roll == 1 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s crawls about silently.", sdesc);
		else if ( roll == 2 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s moves slowly.", sdesc);
		else if ( roll == 3 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s crouches low to the ground.", sdesc);
		else if ( roll == 4 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s stands here, clacking its mandibles.", sdesc);
		else if ( roll == 5 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s is here.", sdesc);
		else if ( roll == 6 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s moves across the rough ground.", sdesc);
		else if ( roll == 7 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s sits here, unmoving.", sdesc);
	}
	
	else if ( !str_cmp (db_race_table [mob->race].name, "Human") ) {
		roll = number (1, 9);
		if ( roll == 1 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s is here, gazing about.", sdesc);
			mob->travel_str = add_hash("gazing about absently");
		}
		else if ( roll == 2 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s wanders through the area.", sdesc);
		else if ( roll == 3 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s passes through, looking about.", sdesc);
			mob->travel_str = add_hash("looking about absently");
		}
		else if ( roll == 4 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s moves by, lost in thought.", sdesc);
			mob->travel_str = add_hash("lost in thought");
		}
		else if ( roll == 5 ) {
			snprintf (buf, MAX_STRING_LENGTH,  "%s strides purposefully through the area.", sdesc);
			mob->travel_str = add_hash("looking ahead purposefully");
		}
		else if ( roll == 6 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s is here.", sdesc);
		else if ( roll == 7 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s stands here.", sdesc);
		else if ( roll == 8 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s lingers here.", sdesc);
		else if ( roll == 9 )
			snprintf (buf, MAX_STRING_LENGTH,  "%s loiters here.", sdesc);
	}
       
	if ( !*buf )
		snprintf (buf, MAX_STRING_LENGTH,  "%s is here.", sdesc);

	*buf = toupper(*buf);

	if ( mob->long_descr )
		mem_free (mob->long_descr);
	mob->long_descr = add_hash(buf);

	if ( mob->name )
		mem_free (mob->name);
	mob->name = add_hash(buf2);
}

void randomize_mobile (CHAR_DATA *mob)
{
        int             attr_starters [] = { 16, 15, 12, 12, 11, 10, 8 };
        int             attr_priorities [] = { -1, -1, -1, -1, -1, -1, 1 };
        int             slots_taken [] = { 0, 0, 0, 0, 0, 0, 0 };
        int             i = 0;
        int 			roll = 0;
        int				bonus = 0;
        int				skill = 0;
        int				sk_cnt = 0;

/** Choose sex **/
	mob->sex = number(1,2);

	if ( IS_SET(mob->act, ACT_ENFORCER) ) {
		roll = number(1,5);

		if ( roll == 5 ){
			mob->sex = SEX_FEMALE;
		}
		else{
			mob->sex = SEX_MALE;
		}
	}

/** random choice for attribute ranking **/
	for ( i = 0; i <= 6; i++ ) {
		roll = number(0,6);

		if ( slots_taken [roll] ) {
			i--;
			continue;
		}

		slots_taken [roll] = 1;
		attr_priorities [i] = roll;
	}	

/** add in attribute bonus points **/
	for ( bonus = 8; bonus; ) {
		roll = number(0,6);

		if ( attr_starters[attr_priorities[roll]] < 18 ) {
			attr_starters [attr_priorities[roll]]++;
			bonus--;
		}

	}

/** add in racial attribute adjustments **/
	mob->str   = attr_starters [attr_priorities [0]];
	mob->str   += db_race_table [mob->race].attr_mods[0];

	mob->dex   = attr_starters [attr_priorities [1]];
	mob->dex   += db_race_table [mob->race].attr_mods[1];

	mob->con   = attr_starters [attr_priorities [2]];
	mob->con   += db_race_table [mob->race].attr_mods[2];

	mob->wil   = attr_starters [attr_priorities [3]];
	mob->wil   += db_race_table [mob->race].attr_mods[3];

	mob->intel = attr_starters [attr_priorities [4]];
	mob->intel += db_race_table [mob->race].attr_mods[4];

	mob->aur   = attr_starters [attr_priorities [5]];
	mob->aur   += db_race_table [mob->race].attr_mods[5];

	mob->agi   = attr_starters [attr_priorities [6]];
	mob->agi   += db_race_table [mob->race].attr_mods[6];

/** assign temp attribute to match orginal attibutes **/
	mob->tmp_str = mob->str;
	mob->tmp_dex = mob->dex;
	mob->tmp_intel = mob->intel;
	mob->tmp_aur = mob->aur;
	mob->tmp_agi = mob->agi;
	mob->tmp_con = mob->con;
	mob->tmp_wil = mob->wil;

/** height and frame **/
	make_height(mob, 1); /* average height */
	mob->height += number(1,8); 

	make_frame (mob);

/** Set all skills to 0 **/
	for ( i = 1; i <= LAST_SKILL; i++ ){
		mob->skills [i] = 0;
	}

	if ( IS_SET (mob->act, ACT_ENFORCER) ){
		mob->skills [SKILL_SUBDUE] = 30 + number(1,10);
	}

/** count potential skills  **/
	for (i = 0; i <= MAX_NATURAL_SKILLS; i ++){
		skill = db_race_table [mob->race].natural_skills[i];
		if (skill){
			sk_cnt ++;
		}
	}
/** skills everybody gets **/
	mob->skills [SKILL_BRAWLING] = 10 + number(1,20);
	mob->skills [SKILL_DODGE] = 10 + number(1,20);
	mob->skills [SKILL_BLOCK] = 10 + number(1,20);
	mob->skills [SKILL_PARRY] = 10 + number(1,20);
	mob->skills [SKILL_SCAN] = 10 + number(40,60);
	mob->skills [SKILL_LISTEN] = 10 + number(1,40);
	
/** choose 6 skills to have higher values (simulates players choosing ther own skills) , all others in skill list are 1 to 10% ability **/
	for ( i = 0; i <= 7; i++ ) {
		roll = number (1, sk_cnt);
		
/** is the choosen skill in the mob's list yet? **/
		skill = db_race_table [mob->race].natural_skills[roll];
		if (skill) {
			if ( !mob->skills [skill] ) {
				mob->skills [skill] = 35 + number(-20, 20); 
			}
			else {
				continue;
			}
		}
		else {
			i--;
			continue;
		}
		
	}


/** give special skills thier values **/
	mob->skills [SKILL_PARRY] = number(30,50);
	mob->skills [SKILL_BLOCK] = number(30,50);
	mob->skills [SKILL_DODGE] = number(30,50);
	mob->skills [SKILL_BRAWLING] = number(10,30);


/** check cap value for skills and adjust if needed **/
/** SKILL_SUBDUE is an extra bonus skill for enforcers **/
	for ( i = 1; i <= LAST_SKILL; i++ ) {
		if ( IS_SET (mob->act, ACT_ENFORCER) && i == SKILL_SUBDUE ){
			continue;
		}

		if ( mob->skills [i] > calc_lookup (mob, REG_CAP, i) ){
			mob->skills [i] = calc_lookup (mob, REG_CAP, i);
		}

		if ( mob->skills [i] < 0 ){
			mob->skills [i] = number(1,10);
		}
	}


/**set up language **/
	mob->speaks = db_race_table [mob->race].race_speaks;
	if (mob->speaks){
		mob->skills [mob->speaks] = calc_lookup(mob, REG_CAP,
				db_race_table[mob->race].race_speaks);
	}
	else {
		mob->skills [mob->speaks] = 0;
	}


/** set skills up for PC to match what we calculate here for general mobs **/

	if ( mob->pc ) {
		for ( i = 1; i <= LAST_SKILL; i++ ){
			mob->pc->skills [i] = mob->skills [i];
		}
	}

/** sets up Brawling to be highest skill, and sets offense to half of brawling **/
	fix_offense (mob);
	
/** find move rate **/	
	mob->max_move = calc_lookup (mob, REG_MISC, MISC_MAX_MOVE);
	mob->move_points = mob->max_move;

/** create description for mobs **/
	if ( IS_SET (mob->flags, FLAG_VARIABLE) || mob->pc ) {
		create_description(mob);
	}
	
/** choose base speed for mobs **/
	switch(number(1,5)) {
		case 1:
			mob->speed = SPEED_CRAWL;
			break;
		case 2:
			mob->speed = SPEED_PACED;
			break;
		default:
		mob->speed = SPEED_WALK;
		break;
	}
	return;
}
