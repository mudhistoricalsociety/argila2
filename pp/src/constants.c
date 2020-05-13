/** 
*	\file constants.c
*	Program Constants
*
*	The purpose of this module is to provide listing of constant arrays, ranging
*	from colors, to skills, and weather states
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
#include "structs.h"

char s_buf [4096];
char b_buf [B_BUF_SIZE];

/*** contains the vnums for each currecny type ***/

const int coin_type0 [] = {
	13002,    /* gold tree quid */
	13003,    /* gold crown rose */
	13007,    /* silver boar harp shilling */
	13008,    /* copper pence gate */
	13009,    /* bronze farthing sword shield  */
	0,
	0,
	0,
	0,
	0
};


const int coin_type1 [] = {
	13004,	/* grain money handful */
	13005,	/* catnip bundle money */
	13010,	/* small pile of salt */
	0,
	0,
	0,
	0,
	0,
	0,
	0
} ;

const char 	*accent[] = {
	"with very crude enunciation",
	"with crude enunciation",
	"with awkward enunciation",
	"with slightly awkard enunciation",
	"with a very faintly awkward enunciation",
	"with a faint accent",
	"\n"};

const struct lang_info speak_tab [] = {
		{ "Atliduk",		SKILL_SPEAK_ATLIDUK },
		{ "Adunaic",		SKILL_SPEAK_ADUNAIC },
		{ "Haradaic",		SKILL_SPEAK_HARADAIC },
		{ "Westron",		SKILL_SPEAK_WESTRON },
		{ "Dunael",		SKILL_SPEAK_DUNAEL },
		{ "Labba",		SKILL_SPEAK_LABBA },
		{ "Norliduk",		SKILL_SPEAK_NORLIDUK },
		{ "Rohirric",		SKILL_SPEAK_ROHIRRIC },
		{ "Talathic",		SKILL_SPEAK_TALATHIC },
		{ "Umitic",		SKILL_SPEAK_UMITIC },
		{ "Nahaiduk",		SKILL_SPEAK_NAHAIDUK },
		{ "Pukael",		SKILL_SPEAK_PUKAEL },
		{ "Sindarin",		SKILL_SPEAK_SINDARIN },
		{ "Quenya",		SKILL_SPEAK_QUENYA },
		{ "Silvan",		SKILL_SPEAK_SILVAN },
		{ "Khuzdul",		SKILL_SPEAK_KHUZDUL },
		{ "Orkish",		SKILL_SPEAK_ORKISH },
		{ "Black-Speech",	SKILL_SPEAK_BLACK_SPEECH },
		{ "Black-Wise",		SKILL_BLACK_WISE },
		{ "Grey-Wise",		SKILL_GREY_WISE },
		{ "White-Wise",		SKILL_WHITE_WISE },
		{ "\0",			0 }
	};						
	
struct lang_info script_tab [] = {
		{ "Sarati",		SKILL_SCRIPT_SARATI },
		{ "Tengwar",		SKILL_SCRIPT_TENGWAR },
		{ "Beleriand-tengwar",	SKILL_SCRIPT_BELERIAND_TENGWAR },
		{ "Certhas-daeron",	SKILL_SCRIPT_CERTHAS_DAERON },
		{ "Angerthas-daeron",	SKILL_SCRIPT_ANGERTHAS_DAERON },
		{ "Quenyan-tengwar",	SKILL_SCRIPT_QUENYAN_TENGWAR },
		{ "Angerthas-moria",	SKILL_SCRIPT_ANGERTHAS_MORIA },
		{ "Gondorian-tengwar",	SKILL_SCRIPT_GONDORIAN_TENGWAR },
		{ "Arnorian-tengwar",	SKILL_SCRIPT_ARNORIAN_TENGWAR },
		{ "Numenian-tengwar",	SKILL_SCRIPT_NUMENIAN_TENGWAR },
		{ "Northern-tengwar",	SKILL_SCRIPT_NORTHERN_TENGWAR },
		{ "Angerthas-erebor",	SKILL_SCRIPT_ANGERTHAS_EREBOR },
		{ "\0",			0 }
	};
	
const char *professions [] = {
	"None",
	"Armorsmith",
	"Beggar",
	"Baker",
	"Brewer",
	"Cavalier",
	"Chandler",
	"Clothier",
	"Cook",
	"Criminal",
	"Farmer",
	"Fletcher",
	"Glassblower",
	"Heavy Infantry",
	"Herbalist",
	"Hunter",
	"Infantry",
	"Jeweller",
	"Law Enforcement",
	"Leatherworker",
	"Miner",
	"Peddler",
	"Scout",
	"Scribe",
	"Survivalist",
	"Timberwright",
	"Weaponsmith",
	"Woodworker",
	"\n"
};


const int role_costs [10] = {
	1,
	2,
	2,
	2,
	3,
	3,
	4,
	5,
	999,
	999
};

const char *unspecified_conditions [] = {
	"It has been completely destroyed, and is clearly unfit for use.\n",
	"It has been savaged, rendering the item nearly unusable.\n",
	"This item is in shambles, having sustained quite a bit of damage.\n",
	"It is in ill repair, and rather damaged in appearance.\n",
	"It seems to have seen a decent amount of usage.\n",
	"It appears to be nearly flawless in condition.\n"
};

const char *fabric_conditions [] = {
	"All that remains of this item is an unsalvageable mass of torn fabric.\n",
	"What few hale stitches remain between large tears are frayed and worn.\n",
	"It is torn, frayed, and altogether quite piteous in appearance.\n",
	"It bears a few noticeable tears and rips from hard usage.\n",
	"This item has seen some usage, looking frayed and somewhat worn.\n",
	"It is nearly flawless, frayed only very slightly in a few spots.\n"
};

const char *leather_conditions [] = {
	"This item has been utterly ravaged, and is entirely unsalvageable.\n",
	"What little undamaged material remains is dark with age and use.\n",
	"It has been cracked and worn in many places with time and abuse.\n",
	"It bears a number of small cracks and scars, looking quite used.\n",
	"It has seen some usage, appearing cracked and worn in a few spots.\n",
	"It is in excellent condition, bearing only a few blemishes and scars.\n"
};

const char *wood_conditions [] = {
	"All that remains of this item is a few jagged splinters of wood.\n",
	"This object has been cracked and splintered to the point of unusability.\n",
	"It has been cracked in a number of places, bearing numerous splinters.\n",
	"A few small but noticeable cracks have been made in this object.\n",
	"Splintered in a few places, this looks to have seen some usage.\n",
	"It is nearly flawless, only a small splinter or two out of place.\n"	
};

const char *bone_conditions [] = {
	"This item has been cracked, chipped and flaked into oblivion.\n",
	"Nearly unusable, it has been cracked and chipped beyond recognition.\n",
	"It is cracked and chipped in a significant number of places.\n",
	"A few small, very faint cracks can be seen on this item.\n",
	"Nearly without flaw, only a small bit of wear is visible.\n",
};

const char *metal_conditions [] = {
	"This item has been cracked, dented and rusted into oblivion.\n",
	"Almost unusable, it has been dented and corroded beyond recognition.\n",
	"It is rusted and dented in many places.\n",
	"A few dents and small amounts of rust can be seen on this item.\n",
	"Nearly without flaw, only a small amount of corrosion is visible.\n"
	
};

const char *materials [] = {
	"Unspecified",
	"Fabric",
	"Leather",
	"Wood",
	"Bone",
	"Metal"
	"Stone",
	"\n"
};

const char *role_lines [] = {
	"Your character comes from a fairly well-to-do middle-class family.",
	"Your character is an apprentice or low-ranking military sort.",
	"Your character was given a full set of leather armor before leaving home.",
	"Your character received extra training over a broad range of skills.",
	"Your character trained extraordinarily hard as a young child.",
	"Your character is exceptionally talented in one attribute.",
	"Your character is a journeyman or mid-ranked military sort.",
	"Your character is a full-fledged Fellow, or minor military officer.",
	"Your character is from one of the area's lesser noble families.",
	"Your character has been tutored as an apprentice in the magickal arts.",
	"\n"
};

const char *role_info [] = {
	"   Your character's family bequeathed him or her a modest sum of\n"
        "coin, in addition to their personal savings, before they left\n"
        "home, allowing for the purchase of extra goods and services.\n",
	"   Your character is currently engaged as an Apprentice or Private in a\n"
	"Fellowship or fighting order of your choosing. Obviously, the details\n"
	"will need to be worked out with the applications administrator,\n"
	"pending things like availability and space; in your comments\n"
	"section, please include which trade you would wish to ply or which\n"
	"order you would like to belong to if you choose this role.\n",
	"   Before leaving home, one of your character's family members,\n"
	"perhaps a retired military sort, gave them a full suit of leather\n"
	"armor, in the hopes that it would offer protection and safety.\n",
	"   Your character trained extensively in a wide variety of things\n"
	"as a child, resulting in a slight increase to most of his or her\n"
	"starting skill levels.\n",
	"   Your character was given an extraordinarily challenging amount\n"
	"of training as a child, resulting in an extra skill slot during the\n"
	"skill selection phase of character generation.\n",
	"   Your character has always been particularly gifted in one area,\n"
	"whether it be extraordinary physical strength or mental presence.\n",
	"   Your character has reached Journeyman or Corporal status in a\n"
	"given Fellowship or military order. Please be sure to note the\n"
	"Fellowship or fighting order you wish to start in under your\n"
	"comments section, and understand that due to availability and space\n"
	"we won't always be able to accomodate your first choices.\n",
	"   Your character has attained the status of a full-fledged Fellow\n"
	"in a craft, or Sergeant in a military order. Please be sure to make\n"
	"a note of the Fellowship or order you wish to start in under your\n"
	"comments section; also, understand that, pending availability and\n"
	"space, we may or may not be able to accomodate your first choices.\n",
	"   Your character is a member of one of the area's lesser noble\n"
	"families. Obviously, the details will need to be worked out with the\n"
	"applications admin, and so you shouldn't feel like you need to submit\n"
	"a completely researched application should you choose this role.\n",
	"   Your character is undergoing training in a secretive organization\n"
	"of magick-users, in order to master the arcane arts. Please note that\n"
	"only humans may apply for this role, and anyone who does choose this\n"
	"MUST start within the city of Minas Tirith in Gondor.\n",
	"\n",
};

const char *state_name [] = {
	"Playing",
	"Login",
	"New Account Name",
	"Account Email",
	"Enter Account Name",
	"Email Confirm",
	"Account Setup",
	"Pending Disconnection",
	"Password Check",
	"Main Account Menu",
	"Writing Email",
	"Change Email Address",
	"Email Change Confirm",
	"Choosing PC",
	"Viewing PC",
	"Reading",
	"Race Confirm",
	"Name Confirm",
	"Choosing Gender",
	"Message of the Day",
	"Selection One",
	"New Password",
	"Confirm Password",
	"Queue Statistics",
	"Select Race",
	"Decoy",
	"Creation Menu",
	"Choosing Attributes",
	"New Menu",
	"Wait Message",
	"Skill Select",
	"New Player",
	"Age Select",
	"Height/Weight Select",
	"Intro Message",
	"Intro Wait",
	"Creation Comment",
	"Reading Rejection",
	"Web Connection",
	"Select Race",
	"Deleting a PC",
	"Terminating Account",
	"\n"
};

const char *variable_races [] =
{
	"Arachnid",
	"Bird",
	"Cat",
	"Horse",
	"Human",
	"Rat",
	"Warhorse",
	"Wolf",
        "\n"
};

const char *standard_object_colors [] = {
	"black",
	"white",
	"grey",
	"red",
	"orange",
	"yellow",
	"green",
	"blue",
	"indigo",
	"purple",
	"brown",
	"tan",
	"\n"
};

const char *drab_object_colors [] = {
	"faded black",
	"tattered black",
	"shabby black",
	"grimy black",
	"off-white",
	"dingy grey",
	"blotched red",
	"dull orange",
	"bland yellow",
	"faded green",
	"faded blue",
	"faded indigo",
	"faded purple",
	"drab brown",
	"dim grey",
	"dusky slate grey",
	"sooty grey",
	"chalky pale grey",
	"dull mist grey",
	"ashen off-white",
	"dirty bone-white",
	"wan ivory",
	"spotted white",
	"stained white",
	"blotched white",
	"dingy off-white",
	"stained ivory",
	"shabby sallow-colored",
	"lurid pale yellow",
	"dingy yellow",
	"gaudy mustard yellow",
	"sickly pale yellow",
	"shabby pale yellow",
	"murky brown",
	"stained brown",
	"dreary brown",
	"bland brown",
	"spotted muddy brown",
	"dismal sandy brown",
	"dreary beige",
	"grimy beige",
	"shabby beige",
	"dirty beige",
	"tattered beige",
	"bland wheat-colored",
	"drab olive",
	"murky olive",
	"dim olive",
	"faded green",
	"dingy green",
	"shabby green",
	"dull green",
	"sickly greyish-green",
	"grisly brownish-green",
	"discolored green",
	"blotchy green",
	"grimy rust-red",
	"blotchy rust-red",
	"grimy salmon",
	"stained salmon",
	"blotched red",
	"dull red",
	"faded red",
	"stained red",
	"dingy red",
	"faded salmon",
	"well-worn blue",
	"faded slate blue",
	"pallid blue",
	"stained blue",
	"grimy blue",
	"dim blue-black",
	"faded blue-black",
	"dreary blue-black",
	"dull orange",
	"faded reddish-orange",
	"tattered reddish-orange",
	"discolored orange",
	"stained orange-red",
	"drab peach-colored",
	"lurid peach-colored",
	"sickly peach-colored",
	"tattered violet",
	"grimy lavender",
	"spotted lavender",
	"discolored purple",
	"dirty purple",
	"dingy purple",
	"faded purple",
	"stained purple",
	"dusty faded purple",
	"\n"
};

const char *fine_object_colors [] = {
	"ebony",
	"onyx",
	"obsidian",
	"midnight black",
	"ink black",
	"jet black",
	"pitch black",
	"ivory",
	"seashell",
	"snow white",
	"gleaming white",
	"pure white",
	"pearl white",
	"bright white",
	"bone white",
	"ghost white",
	"mist grey",
	"charcoal grey",
	"thistle grey",
	"smoky grey",
	"slate grey",
	"silver grey",
	"soft grey",
	"ash grey",
	"crimson",
	"scarlet",
	"ruby red",
	"blood red",
	"rose red",
	"wine red",
	"flame red",
	"coral",
	"copper",
	"fiery orange",
	"ocher",
	"sunset orange",
	"amber",
	"goldenrod",
	"pale yellow",
	"golden yellow",
	"sand yellow",
	"topaz-hued",
	"gold-colored",
	"spring green",
	"sea green",
	"hunter green",
	"olive green",
	"sage green",
	"pine green",
	"bright green",
	"rich green",
	"pale green",
	"emerald green",
	"dark green",
	"verdant green",
	"forest green",
	"chartreuse",
	"slate blue",
	"bright blue",
	"powder blue",
	"sapphire blue",
	"royal blue",
	"ocean blue",
	"teal",
	"azure",
	"beryl",
	"cerulean",
	"cobalt",
	"rich indigo",
	"deep indigo",
	"vivid indigo",
	"earthen brown",
	"deep brown",
	"rich brown",
	"burnt sienna",
	"chocolate",
	"cinnamon",
	"mahogany",
	"nut",
	"umber",
	"amethyst",
	"mauve",
	"mulberry",
	"orchid",
	"plum",
	"lavender",
	"royal purple",
	"violet",
	"\n"
};

const char *wound_locations [] = {
	"skull",
	"reye",
	"leye",
	"face",
	"neck",
	"rshoulder",
	"lshoulder",
	"rupperarm",
	"lupperarm",
	"relbow",
	"lelbow",
	"rforearm",
	"lforearm",
	"rhand",
	"lhand",
	"thorax",
	"abdomen",
	"hip",
	"groin",
	"rthigh",
	"lthigh",
	"rknee",
	"lknee",
	"rcalf",
	"lcalf",
	"rfoot",
	"lfoot",
	"rforeleg",
	"lforeleg",
	"rhindleg",
	"lhindleg",
	"rforefoot",
	"lforefoot",
	"rhindfoot",
	"lhindfoot",
	"rforepaw",
	"lforepaw",
	"rhindpaw",
	"lhindpaw",
	"rforehoof",
	"lforehoof",
	"rhindhoof",
	"lhindhoof",
	"stinger",
	"muzzle",
	"rleg",
	"lleg",
	"rwing",
	"lwing",
	"tail",
	"\n" };

const char *attrs [] = {
	"str",
	"dex",
	"con",
	"wil",
	"int",
	"aur",
	"agi",
	"\n" };

const char *smells [] = {		/* Add also to handler's registry, and */
	"aklash odor",				/*  add message for show_char_to_char */
	"rose scent",
	"jasmine scent",
	"sewer stench",
	"soap aroma",
	"cinnamon scent",
	"leortevald stench",
	"yulpris odor",
	"fresh bread",
	"mown hay",
	"\n"
};

const char *subcraft_flags [] = {
	"object",
	"character",
	"defensive",
	"offensive",	
	"area",
	"room",
    "self",
	"\n"
};

const char *targets [] = {
	"Self",
	"\n"
};

const char *sizes [] = {
	"Sizeless",
	"XXS",
	"XS",
	"S",
	"M",
	"L",
	"XL",
	"XXL",
	"\n"
};

const char *sizes_named [] = {
	"\01",				/* Binary 1 (^A) should be hard to enter for players */
	"XX-Small",
	"X-Small",
	"Small",
	"Medium",
	"Large",
	"X-Large",
	"XX-Large",
	"\n"
};

const char *frames [] = {
	"fragile",
	"scant",
	"light",
	"medium",
	"heavy",
	"massive",
	"monstrous",
	"mammoth",
	"gigantic",
	"gargantuan",
	"colossal",
	"\n"
};

/** forageables are not the same as herbs, but close **/
/** these need to be vnums **/
const int forageables [NUM_FORAGEABLES] = {
	161,
	97002,
	92072,
	97132,
	97202,
	97238,
	97372,
	97405,
	97554,
	97594,
	97739,
	97742,
	97759,
	97784,
	97853,
	98043,
	98098
};

const int herbArray[HERB_NUMSECTORS * HERB_RARITIES][5] =

/* Arrays for herbs by rsector and rarity by obj number */

/* { #herbs, obj#'s } by rarity, from very rare to very common */

/* field herb plants */
{{1, 2196, 0, 0, 0}, {2, 76847, 76849, 0, 0}, {1, 76843, 0, 0, 0}, 
{1, 1049, 0, 0, 0}, {1, 2198, 0, 0, 0},

/* pasture herb plants */
{1, 10406, 0, 0, 0}, {1, 2318, 0, 0, 0}, {1, 2321, 0, 0, 0}, 
{1, 76818, 0, 0, 0}, {1, 1024, 0, 0, 0},

/* wood herb plants */
{1, 2296, 0, 0, 0}, {1, 76851, 0, 0, 0}, {2, 17039, 76824, 0, 0}, 
{1, 76822, 0, 0, 0}, {1, 2194, 0, 0, 0},

/* forest herb plants */
{1, 2298, 0, 0, 0}, {2, 25031, 76853, 0, 0}, {3, 76831, 1080, 76839, 0},
{3, 1500, 76816, 76820, 0}, {3, 76806, 76809, 76833, 0},

/* mountain herb plants */
{1, 2287, 0, 0, 0}, {1, 2326, 0, 0, 0}, {3, 25553, 76829, 76837, 0},
{1, 2324, 0, 0, 0}, {1, 11094, 0, 0, 0},

/* swamp herb plants */
{1, 2308, 0, 0, 0}, {2, 76845, 17040, 0, 0}, {4, 76826, 76835, 30031, 76841}, 
{1, 2305, 0, 0, 0}, {1, 81100, 0, 0, 0},

/* heath herb plants */
{1, 10738, 0, 0, 0}, {1, 2338, 0, 0, 0}, {1, 2336, 0, 0, 0}, 
{1, 2334, 0, 0, 0}, {1, 2332, 0, 0, 0},

/* hill herb plants */
{1, 76810, 0, 0, 0}, {1, 11093, 0, 0, 0}, {1, 2276, 0, 0, 0}, 
{1, 76813, 0, 0, 0}, {1, 2294, 0, 0, 0},

/* waternoswim herb plants */
{1, 34002, 0, 0, 0}, {1, 12040, 0, 0, 0}, {1, 2330, 0, 0, 0},
{1, 1071, 0, 0, 0}, {1, 2328, 0, 0, 0}};


const char *econ_flags [] = {
	"minas tirith",         /* 1 << 0 */ 
	"osgiliath",            /* 1 << 1 */         
	"gondor",
	"minas morgul",
	"magical",			
	"rare",	
	"valuable",
	"foreign",
	"junk",
	"illegal",
	"wild",
	"poor",
	"fine",
	"haradaic",
	"orkish",
	"\n"
};

const char *speeds [] = {
	"walk",				/* 1.00 */
	"trudge",           /* 2.50 */
	"pace",            /* 1.60 */
	"jog",              /* 0.66 */
	"run",              /* 0.50 */
	"sprint",           /* 0.33 */
	"immwalk",          /* 0    */
	"swim",				/* 1.60 */
	"\n"
};

const char *mount_speeds [] = {
	"trot",
	"walk",
	"canter",
	"gallop",
	"run",
	"race",
	"immwalk",
	"swim",
	"\n"
};

const char *mount_speeds_ing [] = {
	"trotting",
	"walking",
	"cantering",
	"galloping",
	"running",
	"racing",
	"immwalking",
	"swimming",
	"\n"
};

const float move_speeds [] =  { 
	1.00, 	/* walk */
	2.50, 	/* trudge */
	1.60, 	/* pace */
	0.66, 	/* jog */
	0.50, 	/* run */
	0.33, 	/* sprint */
	1.60	/* swim */
};

const char *mobprog_triggers [] = {
	"place-holder",
	"say",
	"enter",
	"exit",
	"hit",
	"mobact",
	"alarm",
	"hour",
	"day",
	"teach",
	"whisper",
	"prisoner",
	"knock",
	"\n"
};

const char *rfuncs[] = 
{
	"atecho",
	"give",
	"take",
	"trans",
	"load",
	"force",
	"link",
	"exit",
	"unlock",
	"atlook",
	"vstr",
	"ostr",
	"unlink",
	"unexit",
	"put",
	"get",
	"lock",
	"getcash",
	"givecash",
	"loadmob",
	"exmob",
	"if",
	"fi",
	"else",
	"rftog",
	"pain",
	"spare2",
	"spare3",
	"spare4",
	"spare5",
	"spare6",
	"\n"
};

const struct religion_data religion_trees [] = {

	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }},
	{{ "\n" }}
};

const char *deity_name[] =
{
	"None",
	"Manwe Sulimo",
	"Varda Elentari",
	"Ulmo",
	"Yavanna Kementari",
	"Aule the Smith",
	"Nienna",
	"Orome",
	"Este the Gentle",
	"Mandos",
	"Vaire the Weaver",
	"Lorien",
	"Vana the Ever-young",
	"Tulkas Astaldo",
	"Nessa",
	"\n"
};

const int rev_dir[] = 
{
	2,
	3,
	0,
	1,	
	5,
	4
}; 

const int movement_loss[]=
{
	1,		/* Inside */
	1,		/* City */
   	2,		/* Road */
	3,		/* Trail */
	4,		/* Field */
	5,		/* Woods */
	6,		/* Forest */
   	7,		/* Hills */
   	48,		/* Mountain */
   	8,		/* Swamp */
   	1,		/* Dock */
   	1,		/* Crowsnest */
	4,		/* Pasture	 */	
	5,		/* Heath */
	3,		/* Pit */
	1,		/* Leanto */
	5,		/* Lake */
	15,		/* River */
	9,		/* Ocean */
	7,		/* Reef */
	10		/* Underwater */
};

const char *sector_types[] = {
	"Inside",
	"City",
	"Road",
	"Trail",
	"Field",
	"Woods",
	"Forest",
	"Hills",
	"Mountains",
	"Swamp",
	"Dock",
	"CrowsNest",
	"Pasture",
	"Heath",
	"Pit",
	"Leanto",
	"Lake",
	"River",
	"Ocean",
	"Reef",
	"Underwater",
	"\n"
};
const char *relative_dirs[] =
{
	"northern",
	"eastern",
	"southern",
	"western",
	"above",
	"below",
};

const char *fancy_dirs[] =
{
	"northward",
	"eastward",
	"southward",
	"westward",
	"upward",
	"downward",
	"\n"
};

const char *verbose_dirs[] =
{
	"the north",
	"the east",
	"the south",
	"the west",
	"above",
	"below",
	"\n"
};

const char *dirs[] = 
{
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"
};

const int seasonal_temp [7][12] =  {
	{ 40, 47, 55, 63, 71, 79, 85, 79, 71, 63, 55, 47 },	/*  Temperate baseline */

	{ 22, 29, 37, 45, 53, 61, 67, 61, 53, 45, 37, 29 },	/*  Cool */
	{ 15, 22, 28, 38, 46, 50, 55, 50, 46, 38, 28, 22 },	/*  Cold */
	{  7, 14, 18, 22, 25, 27, 30, 27, 25, 22, 18, 14 },	/*  Arctic */

	{ 55, 57, 60, 65, 73, 81, 89, 81, 73, 65, 60, 53 },	/*  Hot */
	{ 60, 62, 65, 70, 78, 86, 94, 86, 78, 70, 65, 58 },	/*  Hot */
	{ 75, 77, 80, 85, 93, 101, 109, 101, 93, 85, 80, 73 }	/*  Desert */
};

const int sunrise [] =  {  7,  6,  6,  6,  6,  5,  4,  4,  5,  5,  6,  6 };
const int sunset []  =  { 18, 18, 19, 19, 20, 21, 22, 22, 21, 20, 19, 18 };

const char *fog_states [] = {
	"no fog",
	"thin fog",
	"thick fog",
	"\n"
};

const char *weather_states[] = {
	"no rain",
	"chance of rain",
	"light rain",
	"steady rain",
	"heavy rain",
	"light snow",
	"steady snow",
	"heavy snow",
	"\n"
};

const char *weather_clouds [] = {
	"clear sky",
	"light clouds",
	"heavy clouds",
	"overcast",
	"\n"
};

const char *wind_speeds [] = {
	"calm",
	"breeze",
	"windy",
	"gale",
	"stormy",
	"\n"
};

const char *holiday_names [] = {
	"(null)",
	"the feastday of mettare",
	"the feastday of yestare",
	"the feastday of tuilere",
	"the feastday of loende",
	"the feastday of enderi",
	"the feastday of yaviere",
	"\n"
};

const char *holiday_short_names [] = {
	"(null)",
	"mettare",
	"yestare",
	"tuilere",
	"loende",
	"enderi",
	"yaviere",
	"\n"
};

const char *season_string[12] = {
	"deep winter",
	"late winter",
	"early spring",
	"mid-spring",
	"late spring",
	"early summer",
	"high summer",
	"late summer",
	"early autumn",
	"mid-autumn",
	"late autumn",
	"early winter"
};

const char *month_name[12] = {
	"month of Narvinye",
	"month of Nenime",
	"month of Sulime",
	"month of Viresse",
	"month of Lotesse",
	"month of Narie",
	"month of Cermie",
	"month of Urime",
	"month of Yavannie",
	"month of Narquelie",
	"month of Hisime",
	"month of Ringare"
};

const char *month_short_name[12] = {
	"Narvinye",
	"Nenime",
	"Sulime",
	"Viresse",
	"Lotesse",
	"Narie",
	"Cermie",
	"Urime",
	"Yavannie",
	"Narquelie",
	"Hisime",
	"Ringare"
};


const struct db_race_data	db_race_table [] = {

/** PC races Must be at the top ***/

	{ "Human",						/* racial name */
	{  0,  0,  0,  0,  0,  0,  0 },	/* mods to str dex con wil int aur agi */
	{ 10, 75 },						/* Human age limit */
	0,								/* RPP for Human */
	SKILL_SPEAK_WESTRON,			/* race_speaks */
	{ 	/* possible skills to start - some may be RPP restricted */
		SKILL_BAKING, SKILL_BARTER, SKILL_COOKERY, SKILL_CROSSBOW,
		SKILL_DUAL, SKILL_FARMING, SKILL_FORAGE, SKILL_HEALING,
		SKILL_HEAVY_BLUNT, SKILL_HEAVY_EDGE, SKILL_HEAVY_PIERCE,
		SKILL_HERBALISM, SKILL_HIDE, SKILL_HIDEWORKING, SKILL_HUNT,
		SKILL_LIGHT_BLUNT, SKILL_LIGHT_EDGE, SKILL_LIGHT_PIERCE,
		SKILL_LITERACY, SKILL_LONGBOW, SKILL_MEDIUM_BLUNT,
		SKILL_MEDIUM_EDGE, SKILL_MEDIUM_PIERCE, SKILL_PICK,
		SKILL_POLEARM, SKILL_RIDE, SKILL_SHORTBOW, SKILL_SKIN, 
		SKILL_SNEAK, SKILL_STAFF, SKILL_STEAL, SKILL_TRACKING,
		SKILL_WOODCRAFT
    }, 						
	3, 6, 60,	/* male height dice, sides, constant  (3d6+60) */
	3, 2, 0,	/* male frame dice, sides, constant 	(3d2+0) */
	3, 6, 56,	/* female height dice, sides, constant	(3d6+56) */
	2, 2, 0,	/* female frame dice, sides, consant (2d2+0) */
	1, 			/* 1 - can subdue/be subdued 0 - can't subdue/be subdued*/
	/**racial description **/
	"#4Humans#0\n"
	"------\n"
	"Common humans have very little that's particularly remarkable about them;\n"
	"they're included mainly as a remainder that not everyone is of some special\n"
	"race. In fact, the majority of northwestern Middle Earth's denizens could be\n"
	"considered to belong to this ubiquitous group, their original racial heritage\n"
	"having been diluted and lost over the centuries.\n\n"
	"Due to their cultural diversity, they may be found on either side of the war.\n"
	"\n"
	"#2Racial Data:#0\n"
	"------------\n"
	"Hair: Black, brown, red, blond        Skin: Any color\n"
	"Eyes: Blue, green, brown, hazel       Lifespan: 60-80\n"
	"Height: M 70\" F 68\"    Weight: M 175 F 135\n"
	"\n"
	"#6Quirks:#0 None.\n"
	"#1Enmities:#0 None.\n"
	}, /*  end human */

/*** Special NPC Races ***/	
	{ "Human",						/* racial name */
	{  0,  0,  0,  0,  0,  0,  0 },	/* mods to str dex con wil int aur agi */
	{ 10, 75 },						/* Human age limit */
	0,								/* RPP for Human */
	SKILL_SPEAK_WESTRON,			/* race_speaks */
	{ 	/* possible skills to start - some may be RPP restricted */
		
    }, 						
	3, 6, 60,	/* male height dice, sides, constant  (3d6+60) */
	3, 2, 0,	/* male frame dice, sides, constant 	(3d2+0) */
	3, 6, 56,	/* female height dice, sides, constant	(3d6+56) */
	2, 2, 0,	/* female frame dice, sides, consant (2d2+0) */
	1, 			/* 1 - can subdue/be subdued 0 - can't subdue/be subdued*/
	/**racial description **/
	"#4Humans#0\n"
	"------\n"
	"#6Quirks:#0 None.\n"
	"#1Enmities:#0 None.\n"
	}, /* end human */
	
	{ "Human",						/* racial name */
	{  0,  0,  0,  0,  0,  0,  0 },	/* mods to str dex con wil int aur agi */
	{ 10, 75 },						/* Human age limit */
	0,								/* RPP for Human */
	SKILL_SPEAK_WESTRON,			/* race_speaks */
	{ 	/* possible skills to start - some may be RPP restricted */
		
    }, 						
	3, 6, 60,	/* male height dice, sides, constant  (3d6+60) */
	3, 2, 0,	/* male frame dice, sides, constant 	(3d2+0) */
	3, 6, 56,	/* female height dice, sides, constant	(3d6+56) */
	2, 2, 0,	/* female frame dice, sides, consant (2d2+0) */
	1, 			/* 1 - can subdue/be subdued 0 - can't subdue/be subdued*/
	/**racial description **/
	"#4Humans#0\n"
	"#6Quirks:#0 None.\n"
	"#1Enmities:#0 None.\n"
	}, /* end human */
	
	{ "Human",						/* racial name */
	{  0,  0,  0,  0,  0,  0,  0 },	/* mods to str dex con wil int aur agi */
	{ 10, 75 },						/* Human age limit */
	0,								/* RPP for Human */
	SKILL_SPEAK_WESTRON,			/* race_speaks */
	{ 	/* possible skills to start - some may be RPP restricted */
		
    }, 						
	3, 6, 60,	/* male height dice, sides, constant  (3d6+60) */
	3, 2, 0,	/* male frame dice, sides, constant 	(3d2+0) */
	3, 6, 56,	/* female height dice, sides, constant	(3d6+56) */
	2, 2, 0,	/* female frame dice, sides, consant (2d2+0) */
	1, 			/* 1 - can subdue/be subdued 0 - can't subdue/be subdued*/
	/**racial description **/
	"#4Humans#0\n"
	"------\n"
	"#6Quirks:#0 None.\n"
	"#1Enmities:#0 None.\n"
	}, /* end human */

{ "Human",						/* racial name */
	{  0,  0,  0,  0,  0,  0,  0 },	/* mods to str dex con wil int aur agi */
	{ 10, 75 },						/* Human age limit */
	0,								/* RPP for Human */
	SKILL_SPEAK_WESTRON,			/* race_speaks */
	{ 	/* possible skills to start - some may be RPP restricted */
		
    }, 						
	3, 6, 60,	/* male height dice, sides, constant  (3d6+60) */
	3, 2, 0,	/* male frame dice, sides, constant 	(3d2+0) */
	3, 6, 56,	/* female height dice, sides, constant	(3d6+56) */
	2, 2, 0,	/* female frame dice, sides, consant (2d2+0) */
	1, 			/* 1 - can subdue/be subdued 0 - can't subdue/be subdued*/
	/**racial description **/
	"#4Humans#0\n"
	"------\n"
	"#6Quirks:#0 None.\n"
	"#1Enmities:#0 None.\n"
	}, /* end human */

{ "Human",						/* racial name */
	{  0,  0,  0,  0,  0,  0,  0 },	/* mods to str dex con wil int aur agi */
	{ 10, 75 },						/* Human age limit */
	0,								/* RPP for Human */
	SKILL_SPEAK_WESTRON,			/* race_speaks */
	{ 	/* possible skills to start - some may be RPP restricted */
		
    }, 						
	3, 6, 60,	/* male height dice, sides, constant  (3d6+60) */
	3, 2, 0,	/* male frame dice, sides, constant 	(3d2+0) */
	3, 6, 56,	/* female height dice, sides, constant	(3d6+56) */
	2, 2, 0,	/* female frame dice, sides, consant (2d2+0) */
	1, 			/* 1 - can subdue/be subdued 0 - can't subdue/be subdued*/
	/**racial description **/
	"#4Humans#0\n"
	"------\n"
	"#6Quirks:#0 None.\n"
	"#1Enmities:#0 None.\n"
	}, /* end human */

{ "Human",						/* racial name */
	{  0,  0,  0,  0,  0,  0,  0 },	/* mods to str dex con wil int aur agi */
	{ 10, 75 },						/* Human age limit */
	0,								/* RPP for Human */
	SKILL_SPEAK_WESTRON,			/* race_speaks */
	{ 	/* possible skills to start - some may be RPP restricted */
		
    }, 						
	3, 6, 60,	/* male height dice, sides, constant  (3d6+60) */
	3, 2, 0,	/* male frame dice, sides, constant 	(3d2+0) */
	3, 6, 56,	/* female height dice, sides, constant	(3d6+56) */
	2, 2, 0,	/* female frame dice, sides, consant (2d2+0) */
	1, 			/* 1 - can subdue/be subdued 0 - can't subdue/be subdued*/
	/**racial description **/
	"#4Humans#0\n"
	"------\n"
	"#6Quirks:#0 None.\n"
	"#1Enmities:#0 None.\n"
	}, /* end human */

/*** NPC Races - not variable ***/
/***	Animals		***/

/* 31 */{ "Amphibian",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_SWIMMING	
	},
	1, 6, 6,
	1, 1, 0,
	1, 6, 4,
	1, 1, 0,
	0, 	
	"/n" },

/* 32 */{ "Arachnid",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK	
	},
	1, 2, 0,
	0, 0, 0,
	1, 3, 0,
	0, 0, 0,
	0, 	
	"/n" },

/* 33 */{ "Bat",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL	
	},
	1, 3, 0,
	0, 0, 0,
	1, 3, 0,
	0, 0, 0,
	0, 	
	"/n" },

/* 34 */{ "Bear",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	2, 6, 48,
	3, 2, 2,
	2, 6, 48,
	3, 2, 2,
	0, 	
	"/n" },

/* 35 */{ "Bird",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING	
	},
	3, 2, 0,
	0, 0, 0,
	3, 2, 0,
	0, 0, 0,
	0, 	
	"/n" },

/* 36 */{ "Boar",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_SWIMMING
	},
	2, 6, 20,
	1, 2, 0,
	2, 6, 20,
	1, 2, 0,
	0, 	
	"/n" },

/* 37 */{ "Bobcat",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	1, 6, 10,
	1, 1, 0,
	1, 6, 10,
	1, 1, 0,
	0, 	
	"/n" },

/* 38 */{ "Cat",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_TRACKING	
	},
	1, 6, 10,
	1, 1, 0,
	1, 6, 10,
	1, 1, 0,
	0, 	
	"/n" },

/* 39 */{ "Cow",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_SNEAK,
		SKILL_SWIMMING
	},
	2, 6, 44,
	3, 2, 2,
	2, 6, 44,
	3, 2, 2,
	0, 	
	"/n" },

/* 91 */{ "Deer",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_SNEAK,
		SKILL_SWIMMING	
	},
	3, 6, 24,
	1, 5, 0,
	3, 6, 24,
	1, 5, 0,
	0, 	
	"/n" },
	
/* 40 */{ "Dog",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	1, 6, 14,
	1, 1, 0,
	1, 6, 14,
	1, 1, 0,
	0, 	
	"/n" },

/* 41 */{ "Ferret", 
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	1, 3, 2,
	0, 0, 0,
	1, 3, 2,
	0, 0, 0,
	0, 	
	"/n" },

/* 42 */{ "Fish",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH, SKILL_SNEAK,
		SKILL_STEAL, SKILL_SWIMMING	
	},
	1, 3, 0,
	0, 0, 0,
	1, 3, 0,
	0, 0, 0,
	0, 	
	"/n" },

/* 43 */{ "Fox", 
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_BRAWLING, SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT,
		SKILL_SEARCH, SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	1, 3, 2,
	0, 0, 0,
	1, 3, 2,
	0, 0, 0, 
	0, 	
	"/n" },

/* 44 */{ "Frog",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_SWIMMING
	},
	1, 3, 2,
	0, 0, 0,
	1, 3, 2,
	0, 0, 0,
	0, 	
	"/n" },

/* 45 */{ "Goat",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_SNEAK,
		SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	3, 6, 24,
	1, 2, 1,
	3, 6, 24,
	1, 2, 1,
	0, 	
	"/n" },

/* 46 */{ "Goose", 
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_SNEAK, SKILL_STEAL,
		SKILL_SWIMMING	
	},
	1, 6, 10, 
	1, 1, 0,
	1, 6, 10,
	1, 1, 0,
	0, 	
	"/n" },

/* 47 */{ "Horse",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_STEAL,
		SKILL_SWIMMING	
	},
	3, 6, 53,
	0, 0, 6,
	3, 6, 53,
	0, 0, 6,
	0, 	
	"/n" },

/* 48 */{ "Insect",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING	
	},
	1, 2, 0,
	0, 0, 0,
	1, 2, 0,
	0, 0, 0,
	0, 	
	"/n" },

/* 49 */{ "Mink",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	1, 3, 2,
	0, 0, 0,
	1, 3, 2,
	0, 0, 0,
	0, 	
	"/n" },

/* 50 */{ "Mouse", 
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING	
	},
	1, 2, 1,
	0, 0, 0,
	1, 2, 1,
	0, 0, 0,
	0, 	
	"/n" },

/* 51 */{ "Pony",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_STEAL,
		SKILL_SWIMMING
	},
	3, 6, 40,
	0, 0, 5,
	3, 6, 40,
	0, 0, 5,
	0, 	
	"/n" },

/* 52 */{ "Rabbit", 
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_SNEAK,
		SKILL_STEAL, SKILL_SWIMMING	
	},
	1, 3, 5,
	0, 0, 0,
	1, 3, 5,
	0, 0, 0,
	0, 	
	"/n" },

/* 53 */{ "Rat", 
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	1, 3, 1,
	0, 0, 0,
	1, 3, 1,
	0, 0, 0,
	0, 	
	"/n" },

/* 54 */{ "Reptile",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING	
	},
	1, 3, 7,
	0, 0, 0,
	1, 3, 7,
	0, 0, 0,
	0, 	
	"/n" },

/* 55 */{ "Scorpion",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL	
	},
	1, 2, 1,
	0, 0, 0, 
	1, 2, 1, 
	0, 0, 0, 
	0, 	
	"/n" },

/* 56 */{ "Serpent",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING	
	},
	1, 2, 1,
	0, 0, 0,
	1, 2, 1,
	0, 0, 0,
	0, 	
	"/n" },

/* 57 */{ "Sheep",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_SNEAK,
		SKILL_SWIMMING	
	},
	3, 6, 24,
	2, 2, 0,
	3, 6, 24,
	2, 2, 0,
	0, 	
	"/n" },

/* 58 */{ "Snake",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING	
	},
	1, 2, 1,
	0, 0, 0,
	1, 2, 1,
	0, 0, 0,
	0, 	
	"/n" },

/* 59 */{ "Swine",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_SNEAK,
		SKILL_STEAL, SKILL_SWIMMING	
	},
	3, 4, 20,
	3, 2, 0,
	3, 4, 20,
	3, 2, 0, 
	0, 	
	"/n" },

/* 60 */{ "Toad",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0, 
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_SNEAK,
		SKILL_STEAL, SKILL_SWIMMING	
	},
	1, 3, 1,
	0, 0, 0,
	1, 3, 1,
	0, 0, 0,
	0, 	
	"/n" },

/* 61 */{ "Warhorse",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0, 
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	4, 6, 55,
	0, 0, 8,
	4, 6, 55,
	0, 0, 8,
	0, 	
	"/n" },

/* 62 */{ "Wolf",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_HUNT, SKILL_SEARCH,
		SKILL_SNEAK, SKILL_STEAL, SKILL_SWIMMING, SKILL_TRACKING	
	},
	3, 6, 24,
	2, 2, 0,
	3, 6, 24,
	2, 2, 0,
	0, 	
	"/n" },

/* 85 */{ "Swan",
	{  0,  0,  0,  0,  0,  0,  0 },
	{ 0, 0 },
	999,
	0,
	{
		SKILL_CLIMB, SKILL_FORAGE, SKILL_HIDE, SKILL_SEARCH, SKILL_SNEAK,
		SKILL_STEAL, SKILL_SWIMMING	
	},
	5, 6, 50,
	1, 5, 2,
	5, 6, 50,
	1, 5, 2,
	0, 	
	"/n" },

	
	{ "",							/* racial name */
	{  0,  0,  0,  0,  0,  0,  0 },	/* attr mods */
	{ 0, 0 },						/* min, max age limits */
	999,							/* rpp for this race */					
	0, 								/* race speaks */
	{ 0 },							/* possible skills */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* height and frame numbers	*/
	0, 			/* 1 - can subdue/be subdued 0 - can't subdue/be subdued*/
	"\n"		/**racial description **/
	} /*  end empty race */
}; 
	
/*
	0	Non-Restricted
	1	Innate Skill
	2	Unimplemented
	3	Prerequisites (Magic, Psionics, RPP, etc.)
	4	Language
	5 	Script
	6	Requires 1 RPP or more
	7	Requires 2 RPP or more
*/


const struct skill_data_list skill_data [] = {
	/* {skill_name, skill_type, rpp_cost}, */
	{"Unused",  2, 0},
	{"Brawling",  1, 0},
	{"Light-Edge", 0, 0},
	{"Medium-Edge", 0, 0},
	{"Heavy-Edge", 0, 0},
	{"Light-Blunt", 0, 0},
	{"Medium-Blunt", 0, 0},
	{"Heavy-Blunt", 0, 0},
	{"Light-Pierce", 0, 0},
	{"Medium-Pierce", 0, 0},
	{"Heavy-Pierce", 0, 0},
	{"Staff", 0, 0},
	{"Polearm", 0, 0},
	{"Throwing", 0, 0},
	{"Blowgun", 2, 0},
	{"Sling", 0, 0},
	{"Shortbow", 0, 0},
	{"Longbow", 0, 0},
	{"Crossbow", 0, 0},
	{"Dual-Wield", 0, 0},
	{"Block", 1, 0},
	{"Parry", 1, 0},
	{"Subdue", 1, 0},
	{"Disarm", 2, 0},
	{"Sneak", 0, 0},
	{"Hide", 0, 0},
	{"Steal", 6, 1},
	{"Picklock", 6, 1},
	{"Search", 1, 0},
	{"Listen", 1, 0},
	{"Forage", 0, 0},
	{"Ritual", 2, 0},
	{"Scan", 1, 0},
	{"Backstab", 2, 0},
	{"Barter", 0, 0},
	{"Ride", 0, 0},
	{"Climb", 1, 0},
	{"Swim", 1, 0},
	{"Hunt", 0, 0},
	{"Skin", 0, 0},
	{"Sail", 2, 0},
	{"Alchemy", 2, 0},
	{"Herbalism", 0, 0},
	{"Clairvoyance", 3, 0},
	{"Danger Sense", 3, 0},
	{"Empathy", 3, 0},
	{"Hex", 3, 0},
	{"Psychic Bolt", 3, 0},
	{"Prescience", 3, 0},
	{"Aura Sight", 3, 0},
	{"Telepathy", 3, 0},
	{"Seafaring", 2, 0},
	{"Dodge", 1, 0},
	{"Tame", 2, 0},
	{"Break", 2, 0},
	{"Metalcraft", 0, 0},
	{"Woodcraft", 0, 0},
	{"Textilecraft", 0, 0},
	{"Cookery", 0, 0},
	{"Baking", 0, 0},
	{"Hideworking", 0, 0},
	{"Stonecraft", 0, 0},
	{"Candlery", 0, 0},
	{"Brewing", 0, 0},
	{"Distilling", 0, 0},
	{"Literacy", 0, 0},
	{"Dyecraft", 0, 0},
	{"Apothecary", 0, 0},
	{"Glasswork", 0, 0},
	{"Gemcraft", 0, 0},
	{"Milling", 0, 0},
	{"Mining", 0, 0},
	{"Perfumery", 0, 0},
	{"Pottery", 0, 0},
	{"Tracking", 0, 0},
	{"Farming", 0, 0},
	{"Healing", 7, 2},
	{"Atliduk", 4, 0},
	{"Adunaic", 4, 0},
	{"Haradaic", 4, 0},
	{"Westron", 4, 0},
	{"Dunael", 4, 0},
	{"Labba", 4, 0},
	{"Norliduk", 4, 0},
	{"Rohirric", 4, 0},
	{"Talathic", 4, 0},
	{"Umitic", 4, 0},
	{"Nahaiduk", 4, 0},
	{"Pukael", 4, 0},
	{"Sindarin", 4, 0},
	{"Quenya", 4, 0},
	{"Silvan", 4, 0},
	{"Khuzdul", 4, 0},
	{"Orkish", 4, 0},
	{"Black-Speech", 4, 0},
	{"Sarati", 5, 0},
	{"Tengwar", 5, 0},
	{"Beleriand-Tengwar", 5, 0},
	{"Certhas-Daeron", 5, 0},
	{"Angerthas-Daeron", 5, 0},
	{"Quenyan-Tengwar", 5, 0},
	{"Angerthas-Moria", 5, 0},
	{"Gondorian-Tengwar", 5, 0},
	{"Arnorian-Tengwar", 5, 0},
	{"Numenian-Tengwar", 5, 0},
	{"Northern-Tengwar", 5, 0},
	{"Angerthas-Erebor", 5, 0},
	{"Black-Wise", 3, 0},
	{"Grey-Wise", 3, 0},
	{"White-Wise", 3, 0},
	{"Runecasting", 3, 0}	
};

const char *where[] = {
	"<wielded in right hand>  ",     /*  0 */
	"<wielded in left hand>   ",
	"<wielded in both hands>  ",
	"<worn on head>           ",
	"<worn in hair>           ",
	"<worn on the ears>       ",     /*  5 */
	"<over the eyes>          ",
	"<worn on face>           ",
	"<carried in mouth>       ",
	"<worn at throat>         ",
	"<worn at neck>           ",     /*  10 */
	"<worn at neck>           ",
	"<worn on body>           ",
	"<worn on body>           ",
	"<worn on body>           ",
	"<worn on body>           ",	/*  15 */
	"<worn on body>           ",
	"<worn about body>        ",
	"<across the back>        ",
	"<worn over shoulder>     ",
	"<worn over shoulder>     ",	/*  20 */
	"<worn on arms>           ",
	"<worn on right wrist>    ",
	"<worn on left wrist>     ",
	"<worn on hands>          ",
	"<worn on finger>         ",	/*  25 */
	"<worn on finger>         ",
	"<worn about waist>       ",
	"<worn on belt>           ",
	"<worn on belt>           ",
	"<worn on legs>           ",	/*  30 */
	"<worn on legs>           ",
	"<worn on legs>           ",
	"<worn on feet>           ",
	"<worn on feet>           ",	/*  34 */
	"\n"
	
}; 

const char *locations [] = {
	"head",
	"hair",
	"ears",
	"eyes",
	"face",
	"mouth",
	"throat",
	"neck",
	"body",
	"about",
	"back",
	"shoulder",
	"arms",
	"wrist",
	"hands",
	"finger",
	"waist",
	"belt",
	"legs",
	"feet",
	"hand", /* shield */
	"eyes", /* blindfold */
	"\n"
};	
	


const char *drinks[]=
{
	"water",
	"ale",
	"beer",
	"cider",
	"mead",
	"wine",
	"brandy",
	"soup",
	"milk",
	"tea",
	"saltwater",
	"dark ale",
	"amber ale",
	"pale ale",
	"dark beer",
	"white wine",
	"red wine",
	"herbal tea",
	"black tea",
	"chamomile tea",
	"tonic",
	"blood",
	"apple juice",
	"peach nectar",
	"pear juice",
	"carrot juice",
	"hard cider",
	"rye whiskey",
	"red ale",
	"stout ale",
	"stout beer",
	"oil",
	"wax",
	"\n"
};

const char *color_liquid[]=
{
	"clear",
	"brown",
	"clear",
	"brown",
	"dark",
	"golden",
	"red",
	"green",
	"clear",
	"light green",
	"white",
	"brown",
	"black",
	"red",
	"clear",
	"black"
};

const char *fullness[] =
{
	"less than half ",
	"about half ",
	"more than half ",
	""
};

const char *item_types[] = {
	"Undefined",
	"Light",
	"Scroll",
	"Wand",
	"Staff",
	"Weapon",
	"Shield",
	"Missile",
	"Treasure",
	"Armor",
	"Potion",
	"Worn",
	"Other",
	"Trash",
	"Trap",
	"Container",
	"Note",
	"Liquid_container",
	"Key",
	"Food",
	"Money",
	"Ore",
	"Board",
	"Fountain",
	"Grain",
	"Perfume",
	"Pottery",
	"Salt",
    "Zone",
	"Plant",
	"Component",
	"Herb",
	"Salve",
	"Lockpick",
	"Wind_inst",
	"Percu_inst",
	"String_inst",
	"Fur",
	"Woodcraft",
	"Spice",
	"Tool",
	"Usury_note",
	"Bridle",
	"Ticket",
	"Skull",
	"Dye",
	"Cloth",
	"Ingot",
	"Timber",
	"Fluid",
	"Liquid_Fuel",
	"Remedy",
	"Parchment",
	"Book",
	"Writing_inst",
	"Ink",
	"Quiver",
	"Sheath",
	"Keyring",
	"Bullet",
	"NPC_Object",
	"Dwelling",
	"Tent",
	"Repair",
	"\n"
};

/* Used in OLC display 
 * Bitvector For 'wear_flags' 
 * #define ITEM_TAKE  (1<<0) for example
 **/
const char *wear_bits [] = {
	"Head",			/*  0 */
	"Hair",
	"Ears",
	"Eyes",
	"Face",
	"Mouth",		/*  5 */
	"Throat",
	"Neck",
	"Body",
	"About",
	"Back",			/*  10 */
	"Shoulder",
	"Arms",
	"Wrist",
	"Hands",
	"Finger",		/*  15 */
	"Waist",
	"Belt",
	"Legs",
	"Feet",
	"Take",			/*  20 */
	"Wshield",
	"Wield",
	"Unused",
	"Unused",
	"Sheath",		/*  25 */
	"Blindfold",
	
	"\n"
};

const char *extra_bits[] = {
	"Glow",
	"Hum",
	"Invisible",
	"Magic",
	"Nodrop",
	"Bless",
	"Get-affect",
	"Drop-affect",
	"Multi-affect",
	"Wear-affect",
	"Wield-affect",
	"Hit-affect",
	"Ok",
	"Combinable",
	"item-leader",
	"item-member",
	"item-omni",
	"Illegal",
	"Restricted",
	"Mask",
	"Mount",
	"Table",
	"Stack",
	"Capital",
   	"Loads",
	"Variable",	
	"Timer",
	"PC-Sold",
	"Thrown",
	"NewSkills",
	"Pitched",
	"VNPC",
	"\n"
};

const char *room_bits[] = {
	"Dark",
	"Ruins",
	"NoMob",
	"Indoors",
	"Lawful",
	"NoMagic",
	"Tunnel",
	"Cave",
	"SafeQuit",
	"Deep",
	"Fall",
	"NoMount",
	"Vehicle",
	"Fog",
	"NoMerchant",
	"Climb",
	"Psave",
	"Lab",
	"Road",
	"Wealthy",
	"Poor",
	"Scum",
	"Market",
	"Arena",
	"Dock",
	"Wild",
	"Light",
	"NoHide",
	"Storage",
	"PcEntered",
	"Ok",
	"OOC",
	"\n"
};

const char *exit_bits[] = {
	"IsDoor",
	"Closed",
	"Locked",
	"RSClosed",
	"RSLocked",
	"PickProof",
	"Secret",
	"Trapped",
	"Toll",
	"\n"
};


const char *seasons[] = {
	"Spring",
	"Summer",
	"Autumn",
	"Winter",
	"\n"
};

const char *affected_bits[] = 
{	
	"Undefined",
	"Invisible",
	"Infravision",
	"Detect-Invisible",
	"Detect-Magic",
	"Sense-Life",
	"Hold",
	"Sanctuary",
	"Group",
	"Curse",
	"Magic-only",
	"AScan",
	"Paralysis",
	"Undefined",
	"Undefined",
	"Sleep",
	"Dodge",
	"ASneak",
	"AHide",
	"Fear",
	"Charm",
	"Follow",
	"Hooded",
	"\n"
};

const char *apply_types[] = {
	"None",
	"Strength",
	"Dexterity",
	"Intelligence",
	"Charisma",
	"Aura",
	"Will",
	"Constitution",
	"Sex",
	"Age",
	"Weight",
	"Height",
	"Defense",
	"Hitpoints",
	"Movement",
	"Cash",
	"Armor",
	"Offense",
	"Damagebonus",
	"Save_Par",
	"Save_Rod",
	"Save_Pet",
	"Save_Bre",
	"Save_Spe",
	"\n"
};

const char *action_bits[] = {
	"Memory",
	"Sentinel",
	"Scavenger",
	"IsNPC",
	"NoVNPC",
	"Aggressive",
	"Stayzone",
	"Wimpy",
	"Pursue",
	"Thief",
	"Hunter",
	"Killer",
	"Enforcer",
	"Stealthy",
	"Vehicle",
	"Stop",
	"Criminal",
	"Pariah",
	"Mount",
   	"Unused",
	"PCOwned",
	"Wildlife",			/* Mob won't attack other wildlife */
	"Stayput",			/* Mob saves and reloads after boot */
	"Passive",			/* Mob won't assist clan brother in combat */
	"Unused",			/* PC can't petition */
	"Econzone",			/* NPC, if keeper, uses econ zone price dis/markups */
	"Jailer",
	"\n"
};

const char *player_bits[] = {
	"Brief",
	"NoShout",
	"Compact",
	"DONTSET",
	"Quiet",
	"Reboot",
	"Shutdown",
	"Build",
	"Approval",
	"Outlaw",
	"\n"
};


const char *position_types[] = {
	"Dead",
	"Mortally wounded",
	"Unconscious",
	"Stunned",
	"Sleeping",
	"Resting",
	"Sitting",
	"Fighting",
	"Standing",
	"\n"
};

const char *connected_types[]	=	{
	"Playing",
	"Entering Name",
	"Confirming Name",
	"Entering Password",
	"Entering New Password",
	"Confirming New password",
	"Choosing Gender",
	"Reading Message of the Day",
	"Main Menu",
	"Changing Password",
	"Confirming Changed Password",
	"Rolling Attributes",
	"Selecting Race",
	"Decoy Screen",
	"Creation Menu",
	"Selecting Attributes",
	"New Player Menu",
	"Picking Skills",
	"New Player",
	"Age Select",
	"Height-Frame Select",
	"New Char Intro Msg",
	"New Char Intro Wait",
	"Creation Comment",
	"Read Reject Message",
    "Web Connection",
	"\n"

};

const char *sex_types[] = {
	"Sexless",
	"Male",
	"Female",
	"\n"
};

const char *weather_room [] = {
	"foggy",
	"cloudy",
	"rainy",
	"stormy",
	"snowy",
	"blizzard",
	"night",
	"nfoggy",
	"nrainy",
	"nstormy",
	"nsnowy",
	"nblizzard",
	"day",
	"\n"
};

const struct constant_data constant_info [] = {
	{ "item-types",		"OSET                   ", (void **) item_types },
	{ "wear-bits", 		"OSET flag              ", (void **) wear_bits },
	{ "extra-bits",		"OSET flag              ", (void **) extra_bits },
	{ "apply-types",    "OSET affect            ", (void **) apply_types },
	{ "econ-flags",     "OSET/MSET flag         ", (void **) econ_flags },
	{ "drinks",         "OSET oval2 <#>         ", (void **) drinks },
	{ "position-types", "MSET                   ", (void **) position_types },
	{ "sex-types",		"MSET                   ", (void **) sex_types },
	{ "races",			"MSET                   ", (void **) db_race_table},
	{ "action-bits",	"MSET flag              ", (void **) action_bits },
	{ "affected-bits",  "MSET flag              ", (void **) affected_bits },
	{ "skills",			"MSET                   ", (void **) skill_data },
	{ "speeds",         "MSET speed             ", (void **) speeds },
	{ "room-bits",		"RFLAGS flag            ", (void **) room_bits },
	{ "exit-bits",		"RDFLAGS dir flag		", (void **) exit_bits },
	{ "sector-types",   "RSECTOR                ", (void **) sector_types },
	{ "weather-room",	"WEATHER                ", (void **) weather_room },
	{ "rfuncs",			"Room Programs          ", (void **) rfuncs },
	{ "colors",			"Random Object Colors   ", (void **) standard_object_colors },
	{ "drabcolors",		"Random Object Colors   ", (void **) drab_object_colors },
	{ "finecolors",		"Random Object Colors   ", (void **) fine_object_colors },
	{ "woundlocs",      "Wound Locations        ", (void **) wound_locations },
	{ "variable-races",	"Variable Races         ", (void **) variable_races },
	{ "",				"",                        (void **) variable_races }
};


const struct fight_data fight_tab [] = {
	{	"Frantic",		1.10,	0.75,	-4 },		/* Frantic */
	{	"Aggressive",	1.05,	0.85,	0 },		/* Aggressive */
	{	"Normal",		1.00,	1.00,	0 },		/* Normal */
	{	"Careful",		0.90,	1.10,	0 },		/* Careful */
	{	"Defensive",	0.80,	1.20,	4 },		/* Defensive */
};

/* NOTICE:  Set the define CUMBERANCE_ENTRIES in structs.h if
            more entries are added to encumberance_info.  */

const struct encumberance_info enc_tab [] = { 
	{	300,	0,		0,		0,		0,		1.00, "unencumbered" },
	{	500,	5,		1,		0,		0,		1.00, "lightly encumbered" },
	{	800,	10,		2,		5,		1,		0.95, "encumbered" },
	{	1300,	15,		3,		10,		2,		0.90, "heavily encumbered" },
	{	2000,	25,		4,		15,		2,		0.80, "critically encumbered" },
	{	9999900,25,		4,		15,		2,		0.70, "immobile" },
};

		/* REMEMBER:  Set NUM_NIGHTMARES in magic.c */

const char *nightmare [] = {
	"\n"
};

const struct fatigue_data fatigue [] = {
	{	5,		0.75,		"Completely Exhausted" },
	{	15,		0.80,		"Exhausted" },
	{	25,		0.85,		"Extremely Tired" },
	{	35,		0.90,		"Tired" },
	{	45,		0.95,		"Somewhat Tired" },
	{	55,		1.00,		"Winded" },
	{	75,		1.00,		"Somewhat Winded" },	
	{	90,		1.00,		"Relatively Fresh" },
	{	999,	1.00,		"Completely Rested" }
};

#define MAX_ARMOR_TYPE 4
#define MAX_WEAPON_HIT_TYPE 4

const char *armor_types [] = {
	"Quilted",
	"Leather",
	"Ring",
	"Scale",
	"Mail",
	"Plate"
};

const struct body_info body_tab [NUM_BODIES] [MAX_HITLOC] = {
        {       { "body",       2,      1,      45,     WEAR_ABOUT, WEAR_ABOUT },
                { "leg",        1,      1,      25,     WEAR_LEGS_1, WEAR_LEGS_2 },
                { "arms",       1,      1,      20,     WEAR_ARMS, WEAR_HANDS },
                { "head",       5,      2,      8,      WEAR_HEAD, WEAR_HEAD },
                { "neck",       3,      1,      2,      WEAR_NECK_1, WEAR_NECK_2 },
        }
};

const int weapon_armor_table [6] [6] = {
	/* 	quilted		leather		ring		scale		mail		plate */
	{	-1,		-1,		-1,		-1,		-2,		-2  },	  /* stab */
	{	1,		1,		1,		1,		0,		-1  },	  /* pierce */
	{	0,		-1,		0,		0,		1,		2   },	  /* chop */
	{	-1,		-2,		-1,		-1,		2,		2   },	  /* bludgeon */
	{	1,		2,		1,		0,		-1,		-1  },	  /* slash */
	{  	-1,		-3,		-3,		-3,		-3,		-4  }     /* lash */
};

const int weapon_nat_attack_table [4] [6] = {
	/* 	quilted		leather		ring		scale		mail		plate */
	{	-1,		-2,		-1,		-1,		2,		2   },	  /* punch */
	{	1,		1,		1,		1,		-2,		-2  },	  /* bite */
	{	0,		-1,		0,		0,		1,		2   },	  /* claw */
	{	1,		1,		1,		1,		0,		-1  }	  /* peck */
};

const char *weapon_theme [] = { "stab", "pierce", "chop", "bludgeon", "slash", "lash" };

	/* use_table_data is filled in at boot time from registry */

struct use_table_data use_table [] = {
	{ 8 },			/* BRAWLING */
	{ 10 },
	{ 10 },
	{ 10 },
	{ 16 },
	{ 16 },
	{ 16 },
	{ 22 },
	{ 22 },
	{ 22 },
	{ 25 },
	{ 25 },
	{ 0 },
	{ 0 },
	{ 0 },
        { 0 }
};

const char *cs_name [] = { "CritFail", "ModrFail", "ModrSucc", "CritSucc", "\n" };
const char *rs_name [] = { "None", "Advant", "Block", "Parry", "Fumble",
			   "Hit0", "Hit1", "Hit2", "Hit3", "Hit4", "Stumble",
			   "NearFumble", "NearStumble", "Dead", "Any", "WeaponBreak",
			   "ShieldBreak", "KO", "Just_KO", "\n" };

const int shield_parry_offense [4] [4] = {
		{ RESULT_FUMBLE, RESULT_NONE,   RESULT_NONE,  RESULT_NONE }, 
		{ RESULT_NONE,   RESULT_BLOCK,  RESULT_NONE,  RESULT_NONE },
		{ RESULT_HIT1,   RESULT_HIT,    RESULT_BLOCK, RESULT_NONE },
		{ RESULT_HIT3,   RESULT_HIT1,   RESULT_HIT,   RESULT_BLOCK }
};

		/*     CF             MF            MS            CS    */
const int shield_parry_defense [4] [4] = {
		{ RESULT_FUMBLE, RESULT_NONE,   RESULT_ADV,   RESULT_ADV },  /* CF */
		{ RESULT_NONE,   RESULT_BLOCK,  RESULT_NONE,  RESULT_ADV },  /* MF */
		{ RESULT_NONE,   RESULT_NONE,   RESULT_BLOCK, RESULT_ADV },  /* MS */
		{ RESULT_NONE,   RESULT_NONE,   RESULT_NONE,  RESULT_BLOCK } /* CS */
};

const int dodge_offense [4] [4] = {
		{ RESULT_FUMBLE, RESULT_NONE,   RESULT_NONE,  RESULT_NONE },
		{ RESULT_NONE,   RESULT_NONE,   RESULT_NONE,  RESULT_NONE },
		{ RESULT_HIT2,   RESULT_HIT,    RESULT_NONE,  RESULT_NONE },
		{ RESULT_HIT4,   RESULT_HIT2,   RESULT_HIT,   RESULT_NONE }
};

const int dodge_defense [4] [4] = {
		{ RESULT_FUMBLE, RESULT_NONE,   RESULT_ADV,   RESULT_ADV },
		{ RESULT_NONE,   RESULT_NONE,   RESULT_NONE,  RESULT_ADV },
		{ RESULT_NONE,   RESULT_NONE,   RESULT_NONE,  RESULT_NONE },
		{ RESULT_NONE,   RESULT_NONE,   RESULT_NONE,  RESULT_NONE }
};

const int ignore_offense [4] =
		{ RESULT_FUMBLE, RESULT_HIT,    RESULT_HIT2,  RESULT_HIT4 };

const char *crithit [] = {
		"deft", "nimble",			/* stab */
		"forceful", "deft",			/* pierce */
		"powerful", "staggering",	/* chop */
		"crushing", "staggering",	/* bludgeon */
		"savage", "brutal",			/* slash */
		"hurtful", "hurtful",		/* lash */
		"savage", "brutal" };

const char *crithits [] = {
		"deftly", "nimbly",			/* stab */
		"forcefully", "deftly",		/* pierce */
		"savagely", "brutally",		/* chop */
		"powerfully", "brutally",	/* bludgeon */
		"savagely", "brutally",		/* slash */
		"hurtfully", "hurtfully",	/* lash */
		"savagely", "brutally" };	/* natural */

const char *critfail [] = { "wide open", "off guard" };

const char *wtype [] = {
		"stab", "stab",				/* stab */
		"thrust", "lunge",			/* pierce */
		"chop", "chop",				/* chop */
		"strike", "strike",		/* bludgeon */
		"slash", "slice",			/* slash */
		"lash", "lash" };			/* lash */

const char *wtype2 [] = {
		"stabs", "stabs",			/* stab */
		"thrusts", "lunges",		/* pierce */
		"chops", "chops",			/* chop */
		"strikes", "strikes",		/* bludgeon */
		"slashes", "slices",		/* slash */
		"lashes", "lashes" };		/* lash */

const char *attack_names [] = {
    	"punch",
	"bite",
	"claw",
	"peck",
	"\n"
};

const char *attack_names_plural [] = {
	"punches",
	"bites",
	"claws",
	"pecks"
};

const char *attack_part [] = {
	"fist",
	"sharp teeth",
	"paw",
	"sharp beak"
};

const char *break_def [] = {
		"slip past", "slips past",				/* stab */
		"avoid", "avoids",						/* pierce */
		"break down", "breaks down",			/* chop */
		"force through", "forces through",		/* bludgeon */
		"knock aside", "knocks aside",			/* slash */
		"maneuver around", "maneuvers around",	/* lash */
		"force through", "forces through" };	/* natural */

const char *scary_dreams [] = {
	"\n"
};

const char *phase_flags [] = {
	"cannot-leave-room",
	"open_on_self",				/* "open:" the skill on self */
	"require_on_self",          /* "req:" skill on self */
	"require_greater",			/* "req:" comparison is greater than */
	"\n"
};

const char *item_default_flags [] = {
	"in-room",
	"in-inventory",
	"held",
	"wielded",
	"used",
	"produced",
	"worn",
	"\n"
};


/*
Adding a new econ zone?  Remember to update zone_to_econ_zone ()
*/

struct econ_data default_econ_info [] = {
/* sold from     sold to -->
      V            minas tirith   osgiliath     gondor     minas morgul   foreign */
{ "minas tirith", { {1.00,0.50}, {1.25,0.75}, {1.40,0.90}, {2.25,1.75}, {1.60,1.25} } },
{ "osgiliath",    { {1.00,0.75}, {1.00,0.50}, {1.20,0.80}, {2.00,1.50}, {1.40,0.95} } },
{ "gondor",       { {1.10,0.85}, {1.20,0.95}, {1.00,0.50}, {2.10,1.65}, {1.50,1.05} } },
{ "minas morgul", { {0.50,0.25}, {0.75,0.50}, {0.60,0.40}, {1.00,0.50}, {0.70,0.45} } },
{ "foreign",      { {1.10,0.85}, {1.20,0.95}, {1.25,1.00}, {1.60,1.25}, {1.00,0.50} } },
{ "\n",           { {1.00,0.50}, {1.00,0.50}, {1.00,0.50}, {1.00,0.50}, {1.00,0.50} } }
};

const char *trigger_types[] =
{
	"TIME",
	"ENTER",
	"LEAVE",
	"TELL",
	"ASK",
	"SAY",
	"GIVE",
	"\n"
};