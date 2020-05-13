/*------------------------------------------------------------------------\
|  structs.h : Data Structures                        www.yourmud.org     | 
|  Copyright (C) 2006, Project Argila: Auroness                           |
|                                                                         |
|  All rights reserved. See Licensing documents included.                 |
|                                                                         |
|  Based upon Shadows of Isildur RPI Engine                               |
|  Copyright C. W. McHenry [Traithe], 2004                                |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <sys/types.h>
#include <sys/time.h>

typedef struct	account_data		ACCOUNT_DATA;
typedef struct	affected_type		AFFECTED_TYPE;
typedef struct	alias_data			ALIAS_DATA;
typedef struct	attacker_data		ATTACKER_DATA;
typedef struct	board_data			BOARD_DATA;
typedef struct	body_info			BODY_INFO;
typedef struct	char_ability_data	CHAR_ABILITY_DATA;
typedef struct	char_data			CHAR_DATA;
typedef struct	charm_data			CHARM_DATA;
typedef struct	clan_data			CLAN_DATA;
typedef struct	combat_data			COMBAT_DATA;
typedef struct	combat_msg_data		COMBAT_MSG_DATA;
typedef struct	common_data			COMMON_DATA;
typedef struct	default_item_data	DEFAULT_ITEM_DATA;
typedef struct	delayed_affect_data	DELAYED_AFFECT_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	dream_data			DREAM_DATA;
typedef struct	extra_descr_data	XTRA_DESCR_DATA;
typedef struct	help_data			HELP_DATA;
typedef struct	help_info			HELP_INFO;
typedef struct	known_spell_data	KNOWN_SPELL_DATA;
typedef struct	line_data			LINE_DATA;
typedef struct	lodged_missile_info	LODGED_MISSILE_INFO;
typedef struct	lodged_object_info	LODGED_OBJECT_INFO;
typedef struct	memory_t			MEMORY_T;
typedef struct	message_data		MESSAGE_DATA;
typedef struct	mob_data			MOB_DATA;
typedef struct	mobprog_data		MOBPROG_DATA;
typedef struct	move_data			MOVE_DATA;
typedef struct	mudmail_data		MUDMAIL_DATA;
typedef struct	name_switch_data	NAME_SWITCH_DATA;
typedef struct	negotiation_data	NEGOTIATION_DATA;
typedef struct	newbie_hint			NEWBIE_HINT;
typedef struct	obj_data			OBJ_DATA;
typedef struct	object_damage		OBJECT_DAMAGE;
typedef struct	pc_data				PC_DATA;
typedef struct	phase_data			PHASE_DATA;
typedef struct	poison_data			POISON_DATA;
typedef struct	prog_data			PROG_DATA;
typedef struct	qe_data				QE_DATA;
typedef struct	random_ch_data		RANDOM_CH_DATA;
typedef struct	recipe_data			RECIPE_DATA;
typedef struct	reset_affect		RESET_AFFECT;
typedef struct	reset_data			RESET_DATA;
typedef struct	reset_time_data		RESET_TIME_DATA;
typedef struct	role_data			ROLE_DATA;
typedef struct	room_data			ROOM_DATA;
typedef struct	room_direction_data	ROOM_DIRECTION_DATA;
typedef struct	room_extra_data		ROOM_EXTRA_DATA;
typedef struct	room_prog			ROOM_PROG;
typedef struct	second_affect		SECOND_AFFECT;
typedef struct	shop_data			SHOP_DATA;
typedef struct	site_info			SITE_INFO;
typedef struct	social_data			SOCIAL_DATA;
typedef struct	stored_character	STORED_PC_DATA;
typedef struct	subcraft_head_data	SUBCRAFT_HEAD_DATA;
typedef struct	text_data			TEXT_DATA;
typedef struct	threat_data			THREAT_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	track_data			TRACK_DATA;
typedef struct  trigger_data 		TRIGGER_DATA;
typedef struct	var_data			VAR_DATA;
typedef struct	viewed_data			VIEWED_DATA;
typedef struct	wound_data			WOUND_DATA;
typedef struct	writing_data		WRITING_DATA;
typedef struct	written_descr_data	WRITTEN_DESCR_DATA;

typedef char bool;
typedef char byte;
typedef unsigned long bitflag;
typedef signed char shortint;

struct timeval time_now;

#define IMPLEMENTOR_ACCOUNT	"God"

#define MYSQL_USERNAME		"Someone"
#define MYSQL_PASS		"password"
#define MYSQL_HOST		"localhost"
#define MYSQL_SOCK		NULL

#define PRIMARY_DATABASE	"shadows"
#define PFILES_DATABASE		"shadows_pfiles"
#define LOG_DATABASE		"server_logs"
#define CRYPT_PASS		"specpassword"

#define STAFF_EMAIL		"Someone@yourmud.org"
#define SERVER_LOCATION		"127.0.0.1"

#define MUD_NAME		"Your Mud"
#define MUD_EMAIL		"newmud@yourmud.org"

/* Define top directory containing all three ports */

//#define PATH_TO_TOPDIR		"~"
//#define PATH_TO_SENDMAIL	"/usr/sbin/sendmail"



/* Define the individual port directories */

#define PATH_TO_PP		PATH_TO_TOPDIR "/pp"
#define PATH_TO_BP		PATH_TO_TOPDIR "/bp"
#define PATH_TO_TP		PATH_TO_TOPDIR "/tp"

/* Other miscellaneous filepath defines; absolute filepaths only! */

#define PATH_TO_WEBSITE		"~/public_html"

#define B_BUF_SIZE				200000
#define ADDBUF	buf + strlen (buf)


/* Misc defines */

/* Special Rooms */
#define NOWHERE    	-1

#define AMPITHEATRE			70
#define LINKDEATH_HOLDING_ROOM		71
#define JUNKYARD			71
#define OOC_LOUNGE			111	
#define BOAT_ROOM             1500 /* TODO: make up ths room??!! */


#define ALPHA_START_LOC		111 
#define BETA_START_LOC		111
#define GAMMA_START_LOC		111

#define ALPHA_RELEASE_ROOM    111     /* released prisoners transfered here */
#define BETA_RELEASE_ROOM     111    /* see act.other.c to use these rooms */
#define GAMMA_RELEASE_ROOM	111


#define TOLL_CURRENCY 1

/** PC Races  rpp < 999 **/

#define RACE_HUMAN			0
/*#define RACE_HUMAN          1*/
/*#define RACE_HUMAN          2*/
/*#define RACE_HUMAN          3*/
/*#define RACE_HUMAN          4*/
/*#define RACE_HUMAN          5*/
/*#define RACE_HUMAN          6*/

/** Special NPC Races  rpp = 999 **/

/** NPC Races rpp = 999 **/
#define RACE_AMPHIBIAN		7
#define RACE_ARACHNID		8	
#define RACE_BAT			9
#define RACE_BEAR			10
#define RACE_BIRD			11
#define RACE_BOAR			12
#define RACE_BOBCAT			13
#define RACE_CAT			14			
#define RACE_COW			15
#define RACE_DEER			16
#define RACE_DOG			17
#define RACE_FERRET			18
#define RACE_FISH			19
#define RACE_FOX			20
#define RACE_FROG			21
#define RACE_GOAT			22
#define RACE_GOOSE			23
#define RACE_HORSE			24
#define RACE_INSECT			25
#define RACE_MINK			26
#define RACE_MOUSE			27
#define RACE_PONY			28
#define RACE_RABBIT			29
#define RACE_RAT			30
#define RACE_REPTILE		31
#define RACE_SCORPION		32
#define RACE_SERPENT		33
#define RACE_SHEEP			34
#define RACE_SNAKE			35
#define RACE_SWINE			36
#define RACE_TOAD			37
#define RACE_WARHORSE		38
#define RACE_WOLF			39


#define MAX_RACE			39
#define LAST_PC_RACE		 6

#define LAST_ROLE			9
#define MAX_SPECIAL_ROLES		50

/* Damage absorption limit for any humanoid N/PC is 50 + con x multiplier */
#define CONSTITUTION_MULTIPLIER        3    

#define MAX_CLANS		        100
						
#define CLAN_MEMBER			(1 << 0)
#define CLAN_LEADER			(1 << 1)
#define CLAN_MEMBER_OBJ		(1 << 2)
#define CLAN_LEADER_OBJ		(1 << 3)
#define CLAN_RECRUIT		(1 << 4)
#define CLAN_PRIVATE		(1 << 5)
#define CLAN_CORPORAL		(1 << 6)
#define CLAN_SERGEANT		(1 << 7)
#define CLAN_LIEUTENANT		(1 << 8)
#define CLAN_CAPTAIN		(1 << 9)
#define CLAN_GENERAL		(1 << 10)
#define CLAN_COMMANDER		(1 << 11)
#define CLAN_APPRENTICE		(1 << 12)
#define CLAN_JOURNEYMAN		(1 << 13)
#define CLAN_MASTER		(1 << 14)

#define TREAT_ALL		(1 << 0)
#define TREAT_SLASH		(1 << 1)
#define TREAT_BLUNT		(1 << 2)
#define TREAT_PUNCTURE		(1 << 3)
#define TREAT_BURN		(1 << 6)
#define TREAT_FROST		(1 << 7)

#define SEARCH_KEYWORD		1
#define SEARCH_SDESC		2
#define SEARCH_LDESC		3
#define SEARCH_FDESC		4
#define SEARCH_RACE		5
#define SEARCH_CLAN		6
#define SEARCH_SKILL		7
#define SEARCH_ROOM		8
#define SEARCH_LEVEL		9

#define SPA_STAND                1
#define SPA_GET_OBJ              2
#define SPA_WEAR_OBJ             3
#define SPA_CLOSE_DOOR           4
#define SPA_WORLD_SWAP           5
#define SPA_CORONAN_ARENA        6
#define SPA_CORONAN_OPEN_PEN     7
#define SPA_CORONAN_PEN          8
#define SPA_CORONAN_TICKLE       9
#define SPA_CORONAN_THREAT       10
#define SPA_CORONAN_TOSS_CHUTE   11
#define SPA_WALK                 12
#define SPA_RUN				    13
#define SPA_FOLLOW			    14
#define SPA_SWIM				    15
#define SPA_SHADOW			    16
#define SPA_FLOOD			    17
#define SPA_KNOCK_OUT		    18
#define SPA_ESCAPE				19
#define SPA_MOVE					20
#define SPA_RESCUE				21

#define COMBAT_BRUTALITY        1.85    /** A quick way to adjust the brutality of combat; a setting of 175% seems pretty nasty, brutish, and short. Be careful with this! **/


#define VNUM_JAILBAG			43
#define VNUM_TICKET				44
#define VNUM_HEAD				45
#define VNUM_CORPSE				46
#define VNUM_SPEAKER_TOKEN		175
#define VNUM_STATUE				195
#define LEANTO_OBJ_VNUM         95 /*TODO: I need to prototype this room!!!*/
#define HEALER_KIT_VNUM			900
#define DEFAULT_COIN_VNUM		13002	/**TODO: make a default coin prototype - needed for do_pay**/

#define MAX_COIN 10


#define HOLIDAY_METTARE		1
#define HOLIDAY_YESTARE		2
#define HOLIDAY_TUILERE		3
#define HOLIDAY_LOENDE		4
#define HOLIDAY_ENDERI		5
#define HOLIDAY_YAVIERE		6

#define SPRING	        0
#define SUMMER  	    1
#define AUTUMN		    2
#define WINTER	        3
#define MAX_SEASONS		4

#define DIR_WEST_WIND       0
#define DIR_NORTH_WIND      1

#define CLEAR_SKY       0
#define LIGHT_CLOUDS    1
#define HEAVY_CLOUDS    2
#define OVERCAST        3

#define CALM            0
#define BREEZE          1
#define WINDY           2
#define GALE            3
#define STORMY          4

#define NO_FOG          0
#define THIN_FOG        1
#define THICK_FOG       2

#define NO_RAIN         0
#define CHANCE_RAIN     1
#define LIGHT_RAIN      2
#define STEADY_RAIN     3
#define HEAVY_RAIN      4
#define LIGHT_SNOW      5
#define STEADY_SNOW     6
#define HEAVY_SNOW      7

#define BASE_SPELL		1000

#define BLEEDING_INTERVAL 1         /* 1 minute per "bleeding pulse".*/
#define BASE_SPECIAL_HEALING 30     /* Increased healing rate, for special mobs/PCs.*/
#define BASE_PC_HEALING 40          /* Number of minutes per healing pulse for standard PCs.*/

/* debug parameters */

#define DEBUG_FIGHT		1
#define DEBUG_MISC		2
#define DEBUG_SKILLS	4
#define DEBUG_SUMMARY	8
#define DEBUG_SUBDUE	16

/* registry */

#define REG_REGISTRY		0
#define REG_MAXBONUS		1
#define REG_DURATION		3
#define REG_OV				4
#define REG_LV				5
#define REG_CAP				6
#define REG_SKILLS			7		/* Link to DURATION, OV, LV, CAP */
#define REG_MISC			8
#define REG_MISC_NAMES		9		/* Link to MISC */
#define REG_MISC_VALUES		10
#define REG_MAX_RATES		13		/* Learning rates table skill vs formula */

#define MISC_DELAY_OFFSET	0
#define MISC_MAX_CARRY_N	1
#define MISC_MAX_CARRY_W	2
#define MISC_MAX_MOVE		3
#define MISC_STEAL_DEFENSE	4

#define CRIM_FIGHTING	5
#define CRIM_STEALING	3

/* vehicle types */

#define VEHICLE_BOAT	1
#define VEHICLE_HITCH	2

/* color system */

#define BELL "\007"

/* main loop pulse control */

#define PULSES_PER_SEC		4

#define PULSE_ZONE		(PULSES_PER_SEC * 60)
#define PULSE_MOBILE		40
#define PULSE_STATS		(PULSES_PER_SEC * 60 * 5)
#define PULSE_AUTOSAVE		(PULSES_PER_SEC * 60)
#define PULSE_DELAY		4
#define PULSE_SMART_MOBS	(PULSES_PER_SEC * 1)
#define PULSE_MOB_SURVIVAL	(PULSES_PER_SEC * 1)

/* generic find defines */

#define FIND_CHAR_ROOM    	(1<<0) 
#define FIND_CHAR_WORLD    	(1<<1)
#define FIND_OBJ_INV      	(1<<2) 
#define FIND_OBJ_ROOM     	(1<<3) 
#define FIND_OBJ_WORLD   	(1<<4)
#define FIND_OBJ_EQUIP   	(1<<5) 

/* mob/object hash */

#define GEN_MOB 	1	
#define GEN_OBJ 	2
#define GEN_WLD 	3
#define MAX_HASH    1000  /* 100 vnums per hash */

#ifndef SIGCHLD
#define SIGCHLD SIGCLD
#endif

/* string stuff */

#ifndef shroud
#define MAX_STRING_LENGTH	20000
#define SMALL_STRING_LENGTH 24
#define MAX_LINE_LENGTH 80
#else
#define MAX_STRING_LENGTH	20000
#define SMALL_STRING_LENGTH 24
#define MAX_LINE_LENGTH 80
#endif

#define MAX_INPUT_LENGTH     4000
#define MAX_MESSAGES          60
#define MAX_ITEMS            153

#define MAX_TEXTS			 100

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN   60
#define SECS_PER_REAL_HOUR  (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY   (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR  (365*SECS_PER_REAL_DAY)

#define UPDATE_PULSE		(4 * 4)
#define SECOND_PULSE		(4)

#define BASE_YEAR 		2433

#define MAX_CONNECTIONS		400

#define DESCRIPTOR_DISCONNECT_SECS	(5 * 60)

#define PLAYER_IDLE_SECS		(15 * 60)
#define PLAYER_DISCONNECT_SECS		(30 * 60)

#define WEATHER_TEMPERATE	0
#define WEATHER_COOL		1
#define WEATHER_COLD		2
#define WEATHER_ARCTIC		3
#define WEATHER_WARM		4
#define WEATHER_HOT		5
#define WEATHER_DESERT		6

	/* Weather-room descrition constants - see weather_room in constants.c */

#define		WR_DESCRIPTIONS		12			/* Doesn't include WR_NORMAL */

#define		WR_FOGGY			0
#define		WR_CLOUDY			1
#define		WR_RAINY			2
#define		WR_STORMY			3
#define		WR_SNOWY			4
#define		WR_BLIZARD			5
#define     WR_NIGHT       		6
#define		WR_NIGHT_FOGGY		7
#define		WR_NIGHT_RAINY		8
#define		WR_NIGHT_STORMY		9
#define		WR_NIGHT_SNOWY		10
#define		WR_NIGHT_BLIZARD	11
#define		WR_NORMAL			12

/* For 'type_flag' */

#define ITEM_LIGHT          1
#define ITEM_SCROLL         2
#define ITEM_WAND           3
#define ITEM_STAFF          4
#define ITEM_WEAPON         5
#define ITEM_SHIELD 	    6
#define ITEM_MISSILE        7
#define ITEM_TREASURE       8
#define ITEM_ARMOR          9
#define ITEM_POTION    	    10
#define ITEM_WORN      	    11
#define ITEM_OTHER     	    12
#define ITEM_TRASH     	    13
#define ITEM_TRAP      	    14
#define ITEM_CONTAINER 	    15
#define ITEM_NOTE      	    16
#define ITEM_DRINKCON   	17
#define ITEM_KEY       	    18
#define ITEM_FOOD      	    19
#define ITEM_MONEY     	    20	/* coins and the like */
#define ITEM_ORE       	    21
#define ITEM_BOARD     	    22
#define ITEM_FOUNTAIN  	    23
#define ITEM_GRAIN		    24
#define ITEM_PERFUME	    25
#define ITEM_POTTERY	    26
#define ITEM_SALT		    27
#define ITEM_ZONE		    28
#define ITEM_PLANT          29
#define ITEM_COMPONENT	    30
#define ITEM_HERB		    31
#define ITEM_SALVE		    32
#define ITEM_LOCKPICK	    33
#define ITEM_INST_WIND      34
#define ITEM_INST_PERCU     35
#define ITEM_INST_STRING 	36
#define ITEM_FUR        	37
#define ITEM_WOODCRAFT  	38
#define ITEM_SPICE		    39
#define ITEM_TOOL		    40
#define ITEM_USURY_NOTE		41
#define ITEM_BRIDLE			42
#define ITEM_TICKET			43
#define ITEM_HEAD			44
#define ITEM_DYE			45
#define ITEM_CLOTH			46
#define ITEM_INGOT			47
#define ITEM_TIMBER			48
#define ITEM_FLUID			49
#define ITEM_LIQUID_FUEL	50
#define ITEM_HEALER_KIT    51    /* set as type remedy */
#define ITEM_PARCHMENT		52
#define ITEM_BOOK			53
#define ITEM_WRITING_INST	54
#define ITEM_INK			55
#define ITEM_QUIVER			56
#define ITEM_SHEATH			57
#define ITEM_KEYRING		58
#define ITEM_BULLET			59
#define ITEM_NPC_OBJECT		60
#define ITEM_DWELLING		61
#define ITEM_TENT			62
#define ITEM_REPAIR_KIT		63
#define ITEM_BLINDFOLD		64

/* Bitvector For 'wear_flags' */

#define ITEM_WEAR_HEAD			( 1 << 0 )
#define ITEM_WEAR_HAIR			( 1 << 1 ) 
#define ITEM_WEAR_EAR			( 1 << 2 )
#define ITEM_WEAR_EYE       	( 1 << 3 )
#define ITEM_WEAR_FACE			( 1 << 4 ) 
#define ITEM_WEAR_MOUTH			( 1 << 5 ) 
#define ITEM_WEAR_THROAT		( 1 << 6 )
#define ITEM_WEAR_NECK      	( 1 << 7 )
#define ITEM_WEAR_BODY			( 1 << 8 )
#define ITEM_WEAR_ABOUT			( 1 << 9 )
#define ITEM_WEAR_BACK			( 1 << 10 )
#define ITEM_WEAR_SHOULDER		( 1 << 11 )
#define ITEM_WEAR_ARMS      	( 1 << 12 )
#define ITEM_WEAR_WRIST    		( 1 << 13 )
#define ITEM_WEAR_HANDS     	( 1 << 14 )
#define ITEM_WEAR_FINGER    	( 1 << 15 )
#define ITEM_WEAR_WAIST   		( 1 << 16 )
#define ITEM_WEAR_BELT			( 1 << 17 )
#define ITEM_WEAR_LEGS      	( 1 << 18 )
#define ITEM_WEAR_FEET      	( 1 << 19 )

#define ITEM_TAKE           ( 1 << 20 )
#define ITEM_WEAR_SHIELD    ( 1 << 21 )
#define ITEM_WIELD         	( 1 << 22 )
#define ITEM_HOLD         	( 1 << 23 )
#define ITEM_THROW        	( 1 << 24 )
#define ITEM_WEAR_SHEATH	( 1 << 25 )
#define ITEM_WEAR_BLINDFOLD	( 1 << 26 )

/* NOTE: UPDATE wear_bits in constants.c */

/* Bitvector for 'extra_flags' */

#define ITEM_DESTROYED      ( 1 << 0 )
#define ITEM_HUM            ( 1 << 1 )
#define ITEM_INVISIBLE      ( 1 << 2 )
#define ITEM_MAGIC          ( 1 << 3 )
#define ITEM_NODROP         ( 1 << 4 )
#define ITEM_BLESS          ( 1 << 5 )
#define ITEM_GET_AFFECT	    ( 1 << 6 )
#define ITEM_DROP_AFFECT    ( 1 << 7 )
#define ITEM_MULTI_AFFECT   ( 1 << 8 )
#define ITEM_WEAR_AFFECT    ( 1 << 9 )
#define ITEM_WIELD_AFFECT   ( 1 << 10 )
#define ITEM_HIT_AFFECT	    ( 1 << 11 )
#define ITEM_OK		        ( 1 << 12 )
#define ITEM_COMBINABLE	    ( 1 << 13 )
#define ITEM_LEADER	        ( 1 << 14 )
#define ITEM_MEMBER	        ( 1 << 15 )
#define ITEM_OMNI	        ( 1 << 16 )
#define ITEM_ILLEGAL	    ( 1 << 17 )
#define ITEM_RESTRICTED	    ( 1 << 18 )
#define ITEM_MASK	        ( 1 << 19 )
#define ITEM_MOUNT			( 1 << 20 )
#define ITEM_TABLE			( 1 << 21 )
#define ITEM_STACK			( 1 << 22 )		/* Item stack with same vnum objs */
#define ITEM_CAPITAL		( 1 << 23 )		/* can be used for trade */
#define ITEM_LOADS			( 1 << 24 )		/* Item is loaded by a reset */
#define ITEM_VARIABLE		( 1 << 25 )
#define ITEM_TIMER			( 1 << 26 )			/* Will decay */
#define ITEM_PC_SOLD        ( 1 << 27 )            /* Sold to shopkeep by PC.*/
#define ITEM_THROWING       ( 1 << 28 )            /* Weapon is suitable for throwing */
#define ITEM_NEWSKILLS      ( 1 << 29 )            /* Doesn't need to be converted */
#define ITEM_PITCHED        ( 1 << 30 )            /* Whether tent has been pitched or not*/
#define ITEM_VNPC           ( 1 << 31 )            /* Item exists but isn't visible to players*/


/* for containers  - value[1] */

#define CONT_CLOSEABLE  ( 1 << 0 )
#define CONT_PICKPROOF  ( 1 << 1 )
#define CONT_CLOSED     ( 1 << 2 )
#define CONT_LOCKED     ( 1 << 3 )
#define CONT_TRAP       ( 1 << 4 )
#define CONT_BEHEADED	( 1 << 5 )


#define OBJ_NOTIMER	-7000000


/* Bitvector for obj 'tmp_flags' */

#define SPA_DROPPED	( 1 << 0 )

/* Bitvector For 'room_flags' */

#define DARK       	( 1 << 0 )
#define RUINS      	( 1 << 1 )
#define NO_MOB     	( 1 << 2 )
#define INDOORS    	( 1 << 3 )
#define LAWFUL     	( 1 << 4 )
#define NO_MAGIC   	( 1 << 5 )
#define TUNNEL     	( 1 << 6 )
#define CAVE       	( 1 << 7 )
#define SAFE_Q     	( 1 << 8 )
#define DEEP	   	( 1 << 9 )
#define FALL		( 1 << 10 )
#define NO_MOUNT	( 1 << 11 )
#define VEHICLE		( 1 << 12 )
#define STIFLING_FOG	( 1 << 13 )
#define NO_MERCHANT	( 1 << 14 )
#define CLIMB		( 1 << 15 )
#define SAVE		( 1 << 16 )
#define LAB		( 1 << 17 )
#define ROAD		( 1 << 18 )
#define WEALTHY		( 1 << 19 )
#define POOR		( 1 << 20 )
#define SCUM		( 1 << 21 )
#define MARKET		( 1 << 22 )
#define ARENA		( 1 << 23 )
#define DOCK		( 1 << 24 )
#define WILD		( 1 << 25 )
#define LIGHT		( 1 << 26 )
#define NOHIDE		( 1 << 27 )
#define STORAGE		( 1 << 28 )
#define PC_ENTERED	( 1 << 29 )
#define ROOM_OK		( 1 << 30 )
#define OOC		( 1 << 31 )

/* For 'dir_option' */

#define DIR_NORTH	0
#define DIR_EAST	1
#define DIR_SOUTH	2
#define DIR_WEST	3
#define DIR_UP	4
#define DIR_DOWN	5

/* exit_info */

#define PASSAGE_ISDOOR       ( 1 << 0 )
#define PASSAGE_CLOSED	    ( 1 << 1 )
#define PASSAGE_LOCKED	    ( 1 << 2 )
#define PASSAGE_RSCLOSED	    ( 1 << 3 )
#define PASSAGE_RSLOCKED	    ( 1 << 4 )
#define PASSAGE_PICKPROOF	( 1 << 5 )
#define PASSAGE_SECRET   	( 1 << 6 )
#define PASSAGE_TRAP	    	( 1 << 7 )
#define PASSAGE_TOLL			( 1 << 8 )

/* For 'Sector types' */

#define SECT_INSIDE	    0
#define SECT_CITY      	    1
#define SECT_ROAD      	    2
#define SECT_TRAIL	    3
#define SECT_FIELD	    4
#define SECT_WOODS	    5
#define SECT_FOREST	    6
#define SECT_HILLS	    7
#define SECT_MOUNTAIN  	    8
#define SECT_SWAMP     	    9
#define SECT_DOCK	    10
#define SECT_CROWSNEST	    11
#define SECT_PASTURE	    12
#define SECT_HEATH	    13
#define SECT_PIT	    14
#define SECT_LEANTO	    15
#define SECT_LAKE	    16
#define SECT_RIVER	    17
#define SECT_OCEAN	    18
#define SECT_REEF	    19
#define SECT_UNDERWATER	    20

#define MAX_SECTOR		20

/* ---- For new herb stuff ---- */
#define NUM_FORAGEABLES 17
/* Herb sectors */

#define HERB_NUMSECTORS		9

#define HERB_FIELD		0
#define HERB_PASTURE		1
#define HERB_WOODS		2
#define HERB_FOREST		3
#define HERB_MOUNTAINS		4
#define HERB_SWAMP		5
#define HERB_HEATH		6
#define HERB_HILLS		7
#define HERB_WATERNOSWIM	8

#define HERB_RARITIES		5

#define MAX_HERBS_PER_ROOM	3

#define HERB_RESET_DURATION	24

/* For 'equip' */
/** Position on character 
* see also const char *locations
**/

#define WEAR_PRIM		0
#define WEAR_SEC		1
#define WEAR_BOTH		2
#define WEAR_HEAD		3
#define WEAR_HAIR		4
#define WEAR_EAR		5
#define WEAR_EYE		6
#define WEAR_FACE		7
#define WEAR_MOUTH		8
#define WEAR_THROAT		9
#define WEAR_NECK_1		10
#define WEAR_NECK_2		11
#define WEAR_BODY_1		12
#define WEAR_BODY_2		13
#define WEAR_BODY_3		14
#define WEAR_BODY_4		15
#define WEAR_BODY_5		16
#define WEAR_ABOUT		17
#define WEAR_BACK		18
#define WEAR_SHOULDER_R	19
#define WEAR_SHOULDER_L	20
#define WEAR_ARMS		21
#define WEAR_WRIST_R	22
#define WEAR_WRIST_L	23
#define WEAR_HANDS		24
#define WEAR_FINGER_R	25
#define WEAR_FINGER_L	26
#define WEAR_WAIST		27
#define WEAR_BELT_1		28
#define WEAR_BELT_2		29
#define WEAR_LEGS_1		30
#define WEAR_LEGS_2		31
#define WEAR_LEGS_3		32
#define WEAR_FEET_1		33
#define WEAR_FEET_2		34
#define WEAR_SHIELD		35
#define WEAR_BLINDFOLD	36

#define MAX_WEAR        36

#define MAX_SKILLS		150	
#define MAX_AFFECT		25


/* conditions */

#define DRUNK			0
#define FULL			1
#define THIRST			2

/* Bitvector for 'affected_by' */

#define AFF_UNDEF1              ( 1 << 0 )
#define AFF_INVISIBLE	        ( 1 << 1 )
#define AFF_INFRAVIS        	( 1 << 2 )
#define AFF_DETECT_INVISIBLE 	( 1 << 3 )
#define AFF_DETECT_MAGIC     	( 1 << 4 ) 
#define AFF_SENSE_LIFE       	( 1 << 5 ) 
#define AFF_SANCTUARY			( 1 << 7 ) 
#define AFF_GROUP            	( 1 << 8 ) 
#define AFF_CURSE            	( 1 << 9 )
#define AFF_FLAMING          	( 1 << 10 ) 
#define AFF_SCAN				( 1 << 11 ) 
#define AFF_SNEAK	        	( 1 << 12 )
#define AFF_HIDE				( 1 << 13 )
#define AFF_FOLLOW           	( 1 << 14 )
#define AFF_HOODED              ( 1 << 15 )

/* modifiers to char's abilities */

#define APPLY_NONE          0
#define APPLY_STR           1
#define APPLY_DEX           2
#define APPLY_INT		    3
#define APPLY_CHA		    4
#define APPLY_AUR		    5
#define APPLY_WIL           6
#define APPLY_CON           7
#define APPLY_SEX           8
#define APPLY_AGE           9 
#define APPLY_CHAR_WEIGHT	10
#define APPLY_CHAR_HEIGHT	11
#define APPLY_DEFENSE		12			/* Free - APPLY_DEFENSE not used */
#define APPLY_HIT		    13
#define APPLY_MOVE		    14
#define APPLY_CASH		    15
#define APPLY_AC		    16
#define APPLY_ARMOR		    16
#define APPLY_OFFENSE		17			/* Free - APPLY_OFFENSE not used */
#define APPLY_DAMROLL		18
#define APPLY_SAVING_PARA	19 
#define APPLY_SAVING_ROD	20
#define APPLY_SAVING_PETRI	21
#define APPLY_SAVING_BREATH	22
#define APPLY_SAVING_SPELL	23
#define APPLY_AGI			24

/* Above 100, don't reapply upon restore of character. */

#define APPLY_BRAWLING		123
#define APPLY_CLUB		    124
#define APPLY_SPEAR		    125
#define APPLY_SWORD		    126
#define APPLY_DAGGER		127
#define APPLY_AXE		    128
#define APPLY_WHIP		    129
#define APPLY_POLEARM		130
#define APPLY_DUAL		    131
#define APPLY_BLOCK		    132
#define APPLY_PARRY		    133
#define APPLY_SUBDUE		134
#define APPLY_DISARM		135
#define APPLY_SNEAK		    136
#define APPLY_HIDE 		    137
#define APPLY_STEAL		    138
#define APPLY_PICK		    139
#define APPLY_SEARCH		140
#define APPLY_LISTEN		141
#define APPLY_FORAGE		142
#define APPLY_RITUAL		143
#define APPLY_SCAN		    144
#define APPLY_BACKSTAB 		145
#define APPLY_BARTER		146
#define APPLY_RIDE		    147
#define APPLY_CLIMB		    148
#define APPLY_PEEP		    149		/* Obsoleted */
#define APPLY_HUNT		    150
#define APPLY_SKIN		    151
#define APPLY_SAIL		    152
#define APPLY_ALCHEMY		153
#define APPLY_HERBALISM		154

#define FIRST_APPLY_SKILL	APPLY_BRAWLING
#define LAST_APPLY_SKILL 	APPLY_HERBALISM

/* NOTE:  Change affect_modify in handler.c if new APPLY's are added */

/* sex */

#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */

#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_UNCONSCIOUS 2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8

/* for mobile actions: specials.act */

#define ACT_MEMORY	    ( 1 << 0 )
#define ACT_SENTINEL    ( 1 << 1 )
#define ACT_SCAVENGER   ( 1 << 2 )
#define ACT_ISNPC       ( 1 << 3 )
#define ACT_NOVNPC		( 1 << 4 )		/* Shopkeep doesn't have vNPC buyers */
#define ACT_AGGRESSIVE 	( 1 << 5 )
#define ACT_STAY_ZONE  	( 1 << 6 )
#define ACT_WIMPY     	( 1 << 7 )
#define ACT_SAGGRESS	( 1 << 8 )
#define ACT_PURSUE		( 1 << 8 )
#define ACT_THIEF	    ( 1 << 9 )
#define ACT_HUNTER	    ( 1 << 10 )
#define ACT_KILLER	    ( 1 << 11 )
#define ACT_ENFORCER	( 1 << 12 )
#define ACT_STEALTHY	( 1 << 13 )
#define ACT_VEHICLE	    ( 1 << 14 )
#define ACT_STOP	    ( 1 << 15 )		/* Lines up with PLR_STOP */
#define ACT_CRIMINAL	( 1 << 16 )
#define ACT_PARIAH	    ( 1 << 17 )
#define ACT_MOUNT		( 1 << 18 )
#define ACT_DONTUSE     ( 1 << 19 )		/* Was AUTOFLEE, use FLAG_AUTOFLEE */
#define ACT_PCOWNED		( 1 << 20 )
#define ACT_WILDLIFE	( 1 << 21 )		/* Wildlife doesn't attack wildlife */
#define ACT_STAYPUT		( 1 << 22 )		/* Saves between reboots */
#define ACT_PASSIVE		( 1 << 23 )		/* Won't assist clan members */
#define ACT_NICE_THIEF 	( 1 << 24 )
#define ACT_ECONZONE	( 1 << 25 )		/* NPC uses econ zone discount/markups*/
#define ACT_JAILER		( 1 << 26 ) 	/* Jailer flag for NPCs */
#define ACT_NOBIND		( 1 << 27 )
#define ACT_NOBLEED		( 1 << 28 )
#define ACT_FLYING		( 1 << 29 )
#define ACT_PHYSICIAN	( 1 << 30 )
#define ACT_PREY		( 1 << 31 )	/* Animals may only be ambushed or hit with ranged weapons */

/* For players : specials.act */

#define PLR_QUIET	    ( 1 << 4 )
#define PLR_STOP	    ( 1 << 15 )


/* Target classes: */

#define TAR_AREA            0
#define TAR_CHAR_OFFENSIVE  1
#define TAR_CHAR_DEFENSIVE  2
#define TAR_CHAR_SELF       3
#define TAR_OBJ             4
#define TAR_OBJ_INV         5
#define TAR_CHAR_SECRET     6
#define TAR_IGNORE          7

#define JOB_1					600
#define JOB_2					601
#define JOB_3					602

/* Affect for toggling listen/mute */
#define MUTE_EAVESDROP				620

/* Affect for counting the number of times a room is herbed */
#define HERBED_COUNT				621

#define CRAFT_FIRST_OLD				650	/* Crafts can be assigned in the */
#define CRAFT_LAST_OLD				899 /*  range CRAFT_FIRST..CRAFT_LAST */

#define AFFECT_SHADOW			900 /* Affected n/pc shadows another n/pc */
#define AFFECT_GUARD_DIR		950 /* Affected n/pc shadows another n/pc */

#define AFFECT_LOST_CON			980

#define AFFECT_HOLDING_BREATH		990 /* Swimming underwater! */

#define MAGIC_CRIM_BASE		    1000 /* Criminal tags reservd from 1000..1100 */
#define MAGIC_CRIM_RESERVED	    1100 /* 1000..1100 are reserved */

#define MAGIC_CLAN_MEMBER_BASE		1200 /* Clan tags reserved from 1200..1299 */
#define MAGIC_CLAN_LEADER_BASE		1300 /* Clan tags reserved from 1300..1399 */
#define MAGIC_CLAN_OMNI_BASE		1400 /* Clan tags reserved from 1400..1499 */

#define MAGIC_SKILL_GAIN_STOP		1500 /* Skill use spam 1500..1500+MAX_SKILLS */
#define MAGIC_SPELL_GAIN_STOP		1800
#define MAGIC_CRAFT_BRANCH_STOP		2000
#define MAGIC_CRAFT_DELAY		2010

#define MAGIC_HIDDEN			2011
#define MAGIC_SNEAK			2012
#define MAGIC_NOTIFY			2013
#define MAGIC_CLAN_NOTIFY		2014
#define MAGIC_TOLL			2015
#define MAGIC_TOLL_PAID			2016
#define MAGIC_DRAGGER			2017
#define MAGIC_GUARD			2018
#define MAGIC_WATCH1			2019
#define MAGIC_WATCH2			2020
#define MAGIC_WATCH3			2021
#define MAGIC_STAFF_SUMMON		2022

#define MAGIC_CRIM_HOODED		2600 /* Hooded criminal observed in zone .. */

#define MAGIC_STARED			2700 /* Don't stare again until this expires */

#define MAGIC_SKILL_MOD_FIRST	3000 /* Reserve 200 for skill mod affects */
#define MAGIC_SKILL_MOD_LAST	3200

#define MAGIC_SENT			3250	/* Used with the emote system. */

#define MAGIC_SMELL_FIRST		3300
#define MAGIC_AKLASH_ODOR		3300
#define MAGIC_ROSE_SCENT		3301
#define MAGIC_JASMINE_SCENT		3302
#define MAGIC_SEWER_STENCH		3303
#define MAGIC_SOAP_AROMA		3304
#define MAGIC_CINNAMON_SCENT		3305
#define MAGIC_LEORTEVALD_STENCH		3306
#define MAGIC_YULPRIS_ODOR		3307
#define MAGIC_FRESH_BREAD		3308
#define MAGIC_MOWN_HAY			3309
#define MAGIC_FRESH_LINEN		3310
#define MAGIC_INCENSE_SMOKE		3311
#define MAGIC_WOOD_SMOKE		3312
#define MAGIC_SMELL_LAST		3399

#define MAGIC_AFFECT_FIRST			3400

#define MAGIC_AFFECT_DMG			3400
#define MAGIC_AFFECT_HEAL			3401
#define MAGIC_AFFECT_PROJECTILE_IMMUNITY	3402
#define MAGIC_AFFECT_INFRAVISION		3403
#define MAGIC_AFFECT_CONCEALMENT		3404
#define MAGIC_AFFECT_INVISIBILITY		3405
#define MAGIC_AFFECT_SEE_INVISIBLE		3406
#define MAGIC_AFFECT_SENSE_LIFE			3407
#define MAGIC_AFFECT_TONGUES			3408
#define MAGIC_AFFECT_LEVITATE			3409
#define MAGIC_AFFECT_SLOW			3410
#define MAGIC_AFFECT_SPEED			3411
#define MAGIC_AFFECT_SLEEP			3412
#define MAGIC_AFFECT_PARALYSIS			3413
#define MAGIC_AFFECT_FEAR			3414
#define MAGIC_AFFECT_REGENERATION		3415
#define MAGIC_AFFECT_CURSE			3416
#define MAGIC_AFFECT_DIZZINESS			3417
#define MAGIC_AFFECT_FURY			3418
#define MAGIC_AFFECT_INVULNERABILITY		3419
#define MAGIC_AFFECT_ARMOR			3420
#define MAGIC_AFFECT_BLESS			3421

#define MAGIC_AFFECT_LAST			4999

#define MAGIC_ROOM_CALM			5000
#define MAGIC_ROOM_LIGHT		5001
#define MAGIC_ROOM_DARK			5002
#define MAGIC_ROOM_DEBUG		5003
#define MAGIC_ROOM_FLOOD		5004
#define MAGIC_WORLD_CLOUDS		5005	/* Blocks the sun */
#define MAGIC_WORLD_SOLAR_FLARE	5006	/* Creates an artificial sun */
#define MAGIC_WORLD_MOON		5006	/* Moonlight in all rooms */

#define MAGIC_BUY_ITEM			5400

#define MAGIC_ROOM_FIGHT_NOISE		5500

#define MAGIC_PETITION_MESSAGE		5600

#define MAGIC_STABLING_PAID		5700
#define MAGIC_STABLING_LAST		5999

#define MAGIC_FLAG_NOGAIN		6000

#define MAGIC_WARNED			6500

#define MAGIC_RAISED_HOOD		6600

#define MAGIC_SIT_TABLE			6700 /* PC acquires this affect when at a table*/


#define CRAFT_FIRST			8000	/* Crafts can be assigned in the */
#define CRAFT_LAST			10000   /*  range CRAFT_FIRST..CRAFT_LAST */

#define MAGIC_FIRST_SOMA                11000   /* SOMATIC EFFECTS TBA */
#define SOMA_MUSCULAR_CRAMP             11001 /* pain echo */
#define SOMA_MUSCULAR_TWITCHING         11002
#define SOMA_MUSCULAR_TREMOR            11003
#define SOMA_MUSCULAR_PARALYSIS         11004
#define SOMA_DIGESTIVE_ULCER            11005 /* pain echo */
#define SOMA_DIGESTIVE_VOMITING         11006
#define SOMA_DIGESTIVE_BLEEDING         11007 /* vomiting blood */
#define SOMA_EYE_BLINDNESS              11008
#define SOMA_EYE_BLURRED                11009
#define SOMA_EYE_DOUBLE                 11010
#define SOMA_EYE_DILATION               11011 /* light sensitivity */
#define SOMA_EYE_CONTRACTION            11012 /* tunnel vision? */
#define SOMA_EYE_LACRIMATION            11013 /* watery eyes */
#define SOMA_EYE_PTOSIS                 11014 /* drooping lids */
#define SOMA_EAR_TINNITUS               11015 /* noise echo */
#define SOMA_EAR_DEAFNESS               11016
#define SOMA_EAR_EQUILLIBRIUM           11017 /* dizziness */
#define SOMA_NOSE_ANOSMIA               11018 /* ignore aroma effects */
#define SOMA_NOSE_RHINITIS              11019 /* itchy/runny nose */
#define SOMA_MOUTH_SALIVATION           11020 /* heh */
#define SOMA_MOUTH_TOOTHACHE            11021 /* pain echo / looseness */
#define SOMA_MOUTH_DRYNESS              11022 /* echo */
#define SOMA_MOUTH_HALITOSIS            11023
#define SOMA_CHEST_DIFFICULTY           11024 /* pain echo */
#define SOMA_CHEST_WHEEZING             11025
#define SOMA_CHEST_RAPIDBREATH          11026
#define SOMA_CHEST_SLOWBREATH           11027
#define SOMA_CHEST_FLUID                11028
#define SOMA_CHEST_PALPITATIONS         11029
#define SOMA_CHEST_COUGHING             11030
#define SOMA_CHEST_PNEUMONIA            11031
#define SOMA_NERVES_PSYCHOSIS           11032
#define SOMA_NERVES_DELIRIUM            11033
#define SOMA_NERVES_COMA                11034 /* depression & drowsiness too */
#define SOMA_NERVES_CONVULSIONS         11035
#define SOMA_NERVES_HEADACHE            11036
#define SOMA_NERVES_CONFUSION           11037 /* misdirection? ;) */
#define SOMA_NERVES_PARETHESIAS         11038 /* am i on fire? ;) */
#define SOMA_NERVES_ATAXIA              11039 /* --dex */
#define SOMA_NERVES_EQUILLIBRIUM        11040
#define SOMA_SKIN_CYANOSIS              11041
#define SOMA_SKIN_DRYNESS               11042
#define SOMA_SKIN_CORROSION             11043
#define SOMA_SKIN_JAUNDICE              11044
#define SOMA_SKIN_REDNESS               11045
#define SOMA_SKIN_RASH                  11046
#define SOMA_SKIN_HAIRLOSS              11047
#define SOMA_SKIN_EDEMA                 11048
#define SOMA_SKIN_BURNS                 11049
#define SOMA_SKIN_PALLOR                11050
#define SOMA_SKIN_SWEATING              11051
#define SOMA_GENERAL_WEIGHTLOSS         11052
#define SOMA_GENERAL_LETHARGY           11053
#define SOMA_GENERAL_APPETITELOSS       11054
#define SOMA_GENERAL_PRESSUREDROP       11055
#define SOMA_GENERAL_PRESSURERISE       11056
#define SOMA_GENERAL_FASTPULSE          11057
#define SOMA_GENERAL_SLOWPULSE          11058 
#define SOMA_GENERAL_HYPERTHERMIA       11059
#define SOMA_GENERAL_HYPOTHERMIA        11060
#define MAGIC_LAST_SOMA                 11060

#define TYPE_UNDEFINED		    -1

#define TYPE_SUFFERING		    200	/* KILLER CDR: Eliminate this line somehow */



/* skills */
/** see also skill_data[]  in constants.c **/

#define SKILL_DEFENSE		-3		/* Special cased; a pseudo skill */
#define SKILL_OFFENSE		-2		/* Special cased; a pseudo skill */

/** Innate Skills **/
#define SKILL_BRAWLING		1
#define SKILL_LIGHT_EDGE	2
#define SKILL_MEDIUM_EDGE	3
#define SKILL_HEAVY_EDGE	4
#define SKILL_LIGHT_BLUNT	5
#define SKILL_MEDIUM_BLUNT	6
#define SKILL_HEAVY_BLUNT	7
#define SKILL_LIGHT_PIERCE	8
#define SKILL_MEDIUM_PIERCE	9
#define SKILL_HEAVY_PIERCE	10
#define SKILL_STAFF			11
#define SKILL_POLEARM		12
#define SKILL_THROWN		13
#define SKILL_BLOWGUN		14
#define SKILL_SLING			15
#define SKILL_SHORTBOW		16
#define SKILL_LONGBOW		17
#define SKILL_CROSSBOW		18
#define SKILL_DUAL          19
#define SKILL_BLOCK         20
#define SKILL_PARRY         21
#define SKILL_SUBDUE        22
#define SKILL_DISARM        23
#define SKILL_SNEAK         24
#define SKILL_HIDE          25
#define SKILL_STEAL         26
#define SKILL_PICK          27
#define SKILL_SEARCH        28
#define SKILL_LISTEN        29
#define SKILL_FORAGE        30
#define SKILL_RITUAL        31
#define SKILL_SCAN          32
#define SKILL_BACKSTAB      33
#define SKILL_BARTER       	34
#define SKILL_RIDE			35
#define SKILL_CLIMB			36
#define SKILL_SWIMMING		37
#define SKILL_HUNT			38
#define SKILL_SKIN			39
#define SKILL_SAIL			40
#define SKILL_ALCHEMY		41
#define SKILL_HERBALISM		42
#define SKILL_CLAIRVOYANCE	43
#define SKILL_DANGER_SENSE	44
#define SKILL_EMPATHIC_HEAL	45
#define SKILL_HEX			46
#define SKILL_MENTAL_BOLT	47
#define SKILL_PRESCIENCE	48
#define SKILL_SENSITIVITY	49
#define SKILL_TELEPATHY		50
#define SKILL_SEAFARING		51
#define SKILL_DODGE			52
#define SKILL_TAME			53
#define SKILL_BREAK			54
#define SKILL_METALCRAFT	55
#define SKILL_WOODCRAFT		56
#define SKILL_TEXTILECRAFT	57
#define SKILL_COOKERY		58
#define SKILL_BAKING		59
#define SKILL_HIDEWORKING	60
#define SKILL_STONECRAFT	61
#define SKILL_CANDLERY		62
#define SKILL_BREWING		63
#define SKILL_DISTILLING	64
#define SKILL_LITERACY		65
#define SKILL_DYECRAFT		66
#define SKILL_APOTHECARY	67
#define SKILL_GLASSWORK		68
#define SKILL_GEMCRAFT		69
#define SKILL_MILLING		70
#define SKILL_MINING		71
#define SKILL_PERFUMERY		72
#define SKILL_POTTERY		73
#define SKILL_TRACKING		74
#define SKILL_FARMING		75
#define SKILL_HEALING		76
#define SKILL_SPEAK_ATLIDUK     77
#define SKILL_SPEAK_ADUNAIC     78
#define SKILL_SPEAK_HARADAIC    79
#define SKILL_SPEAK_WESTRON     80
#define SKILL_SPEAK_DUNAEL      81
#define SKILL_SPEAK_LABBA       82
#define SKILL_SPEAK_NORLIDUK    83
#define SKILL_SPEAK_ROHIRRIC    84
#define SKILL_SPEAK_TALATHIC	85
#define SKILL_SPEAK_UMITIC		86
#define SKILL_SPEAK_NAHAIDUK	87
#define SKILL_SPEAK_PUKAEL		88
#define SKILL_SPEAK_SINDARIN	89
#define SKILL_SPEAK_QUENYA		90
#define SKILL_SPEAK_SILVAN		91
#define SKILL_SPEAK_KHUZDUL	    92
#define SKILL_SPEAK_ORKISH		93
#define SKILL_SPEAK_BLACK_SPEECH	94
#define SKILL_SCRIPT_SARATI		95
#define SKILL_SCRIPT_TENGWAR	96
#define SKILL_SCRIPT_BELERIAND_TENGWAR	97
#define SKILL_SCRIPT_CERTHAS_DAERON		98
#define SKILL_SCRIPT_ANGERTHAS_DAERON	99
#define SKILL_SCRIPT_QUENYAN_TENGWAR	100
#define SKILL_SCRIPT_ANGERTHAS_MORIA	101
#define SKILL_SCRIPT_GONDORIAN_TENGWAR	102
#define SKILL_SCRIPT_ARNORIAN_TENGWAR	103
#define SKILL_SCRIPT_NUMENIAN_TENGWAR	104
#define SKILL_SCRIPT_NORTHERN_TENGWAR	105
#define SKILL_SCRIPT_ANGERTHAS_EREBOR	106
#define SKILL_BLACK_WISE		107
#define SKILL_GREY_WISE			108
#define SKILL_WHITE_WISE		109
#define SKILL_RUNECASTING		110

#define LAST_SKILL			SKILL_RUNECASTING

#define PSIONIC_TALENTS			8
#define skill_lev(val) val >= 70 ? " Master " : val >= 50 ? " Adroit " : val >= 30 ? "Familiar" : " Novice "

/* How much light is in the land ? */

#define SUN_DARK		0
#define SUN_RISE		1
#define SUN_LIGHT		2
#define SUN_SET			3
#define MOON_RISE		4
#define MOON_SET		5

/* And how is the sky ? */

#define SKY_CLOUDLESS	0
#define SKY_CLOUDY   	1
#define SKY_RAINING  	2
#define SKY_LIGHTNING	3
#define SKY_STORMY		4
#define SKY_FOGGY 		5

#define MAX_OBJ_SAVE	15

/* modes of connectedness */

#define CON_PLYNG   			0
#define CON_LOGIN    			1
#define CON_NEW_ACCT_NAME		2
#define CON_ACCT_EMAIL			3
#define CON_ENTER_ACCT_NME		4
#define CON_EMAIL_CONFIRM		5
#define CON_ACCOUNT_SETUP		6
#define CON_PENDING_DISC		7
#define CON_PWDCHK				8
#define CON_ACCOUNT_MENU		9
#define CON_WRITING_EMAIL		10
#define CON_CHG_EMAIL			11
#define CON_CHG_EMAIL_CNF		12
#define CON_CHOOSE_PC			13
#define CON_VIEW_PC				14
#define CON_READING_WAIT		15
#define CON_RACE_CONFIRM		16
#define CON_NAME_CONFIRM		17
#define CON_SEX	    			18
#define CON_RMOTD   			19
#define CON_SLCT1   			20
#define CON_PWDNEW  			21
#define CON_PWDNCNF	 			22
#define CON_QSTATS 	 			23
#define CON_RACE				24
#define CON_DECOY				25
#define CON_CREATION			26
#define CON_ATTRIBUTES			27
#define CON_NEW_MENU        28    /*depreciated*/
#define CON_DOCUMENTS       29    /*depreciated*/
#define CON_WAIT_MSG			30
#define CON_DOC_WAIT			31
#define CON_SKILLS				32
#define CON_PLAYER_NEW			33
#define CON_AGE					34
#define CON_HEIGHT_WEIGHT		35
#define CON_INTRO_MSG			36
#define CON_INTRO_WAIT			37
#define CON_COMMENT				38
#define CON_READ_REJECT			39
#define CON_WEB_CONNECTION		40
#define CON_RACE_SELECT			41
#define CON_DELETE_PC			42
#define CON_TERMINATE_CONFIRM	43
#define CON_PWDGET				44
#define CON_PWDCNF				45
#define CON_CREATE_GUEST		46
#define CON_GUEST_ARRIVE		47
#define CON_ROLE_SELECT			48
#define CON_ROLE_CONFIRM		49
#define CON_SPECIAL_ROLE_SELECT		50
#define CON_SPECIAL_ROLE_CONFIRM	51
#define CON_MAIL_MENU			52
#define CON_COMPOSE_SUBJECT		53
#define CON_COMPOSE_MAIL_TO		54
#define CON_COMPOSING_MESSAGE	55
#define CON_COMPOSE_MESSAGE		56
#define CON_READ_MESSAGE		57
#define CON_PRIV_CONFIRM		58
#define CON_LOCATION			59
#define CON_RETIRE				60
#define CON_ACCT_POLICIES		61
#define CON_PROFESSION			62
#define CON_REFERRAL			63

/* script program commands */

#define RP_ATECHO 	0
#define RP_GIVE		1
#define RP_TAKE		2
#define RP_TRANS	3
#define RP_LOAD		4
#define RP_FORCE	5
#define RP_LINK		6
#define RP_EXIT		7
#define RP_UNLOCK	8
#define RP_ATLOOK	9
#define RP_VSTR		10
#define RP_OSTR		11
#define RP_UNLINK	12
#define RP_UNEXIT	13
#define RP_PUT		14
#define RP_GET		15
#define RP_LOCK		16
#define RP_GETCASH	17
#define RP_GIVECASH	18
#define RP_LOADMOB	19
#define RP_EXMOB	20
#define RP_IF		21
#define RP_FI		22
#define RP_ELSE		23
#define RP_RFTOG	24
#define RP_PAIN		25
#define RP_SPARE2	26
#define RP_SPARE3	27
#define RP_SPARE4	28
#define RP_SPARE5	29
#define RP_SPARE6	30

/* Delay types */

#define DEL_PICK  	        1
#define DEL_SEARCH	        2
#define DEL_RITUAL	        3
#define DEL_BACKSTAB	    4
#define	DEL_SPELL	        5
#define DEL_WORSHIP	        6
#define DEL_FORAGE_SEEK	    7
#define DEL_JOIN_FAITH 	    8
#define DEL_APP_APPROVE	    9
#define DEL_CAST       	    10

#define DEL_SKIN_1       	4002
#define DEL_SKIN_2       	4003
#define DEL_SKIN_3       	4004

#define DEL_COUNT_COIN		12
#define DEL_IDENTIFY	    13
#define DEL_GATHER	        14
#define DEL_COMBINE	        15
#define DEL_WHAP	        16
#define DEL_GET_ALL	        17
#define DEL_AWAKEN	        18
#define DEL_EMPATHIC_HEAL   19
#define DEL_MENTAL_BOLT	    20
#define DEL_ALERT	        21
#define DEL_INVITE	        22
#define DEL_CAMP1	        23
#define DEL_CAMP2	        24
#define DEL_CAMP3	        25
#define DEL_CAMP4	        26
#define DEL_TAKE			28
#define DEL_PUTCHAR			29
#define DEL_STARE			30
#define DEL_HIDE			31
#define DEL_SCAN			32
#define DEL_QUICK_SCAN		33
#define DEL_HIDE_OBJ		34
#define DEL_PICK_OBJ		35
#define DEL_COUNT_copper	36
#define DEL_RUMMAGE		37
#define DEL_QUAFF		38
#define DEL_BIND_WOUNDS		39
#define DEL_TREAT_WOUND		40
#define DEL_LOAD_WEAPON		41
#define DEL_OOC			42
#define DEL_WEAVESIGHT		43
#define DEL_TRACK		44
#define DEL_FORAGE	        45
#define DEL_PITCH		46
#define DEL_PURCHASE_ITEM	47
#define DEL_WATER_REMOVE	48

/* Zone flags */

#define Z_FROZEN    1
#define Z_LOCKED    2

/* Projectile-related defines */

#define NUM_BODIES              1
#define MAX_HITLOC              5
 
struct body_info { 
        char    part [10];
        int             damage_mult;
        int             damage_div;
        int             percent;
        int             wear_loc1;
        int             wear_loc2;
};

#define RESULT_CS       1
#define RESULT_MS       2
#define RESULT_MF       3
#define RESULT_CF       4

#define GLANCING_HIT    1
#define HIT             2
#define CRITICAL_HIT    3
#define MISS            4
#define CRITICAL_MISS   5
#define SHIELD_BLOCK    7

/* Profession defines */

#define PROF_NONE		0
#define PROF_ARMORSMITH		1
#define PROF_BEGGAR		2
#define PROF_BAKER		3
#define PROF_BREWER		4
#define PROF_CAVALIER		5
#define PROF_CHANDLER		6
#define PROF_CLOTHIER		7
#define PROF_COOK		8
#define PROF_CRIMINAL		9
#define PROF_FARMER		10
#define PROF_FLETCHER		11
#define PROF_GLASSBLOWER	12
#define PROF_HEAVY_INF		13
#define PROF_HERBALIST		14
#define PROF_HUNTER		15
#define PROF_INF		16
#define PROF_JEWELLER		17
#define PROF_LAW_ENF		18
#define PROF_LEATHERWORKER	19
#define PROF_MINER		20
#define PROF_PEDDLER		21
#define PROF_SCOUT		22
#define PROF_SCRIBE		23
#define PROF_SURVIVALIST	24
#define PROF_TIMBERWRIGHT	25
#define PROF_WEAPONSMITH	26
#define PROF_WOODWORKER		27

#define PROF_LAST		PROF_WOODWORKER


struct recipe_data {
	int     obj_created;
	int     ingredient [7];
	int		reuse [7];
	struct  recipe_data	*next;
};

struct forage_data {
	long    virt;
	int     sector;
	struct  forage_data *next;
};

struct extra_descr_data
{
	char    *keyword;                 
	char    *description;            
	struct  extra_descr_data *next; 
};

struct writing_data
{
	char		*message;
	char		*author;
	char		*date;
	char		*ink;
	int		language;
	int		script;
	bool		torn;
	int		skill;
	WRITING_DATA	*next_page;
};

struct written_descr_data   /* for descriptions written in languages */
{
    byte    language;
	char    *description;
};

struct obj_flag_data
{
	byte		type_flag;
	bitflag		wear_flags;
	int			extra_flags;
	int			weight;
	int			cost;
	long		bitvector;
};

struct armor_data {
	int		armor_value;
	int		armor_type;
	int		v2;
	int		v3;
	int		v4;

};

struct weapon_data {
	int		handedness;
	int		dice;
	int		sides;
	int		use_skill;
	int		hit_type;
	int		delay;
	int		basedamage;
	int		attackclass;
	int		defenseclass;
	int		range;
};

struct light_data {
	int		capacity;
	int		hours;
	int		liquid;
	int		on;
	int		v4;
	int		v5;
};

struct drink_con_data {
	int		capacity;
	int		volume;
	int		liquid;
	int		spell_1;
	int		spell_2;
	int		spell_3;
};

struct fountain_data {
	int		capacity;
	int		volume;
	int		liquid;
	int		spell_1;
	int		spell_2;
	int		spell_3;
};

struct container_data {
	int		capacity;
	int		flags;
	int		key;
	int		pick_penalty;
	int		v4;
	int		table_max_sitting;
};

struct cloak_data {
	int		v0;
	int		v1;
	int		v2;
	int		v3;
	int		v4;
};

struct ticket_data {
	int		ticket_num;
	int		keeper_vnum;
	int		stable_date;
	int		v3;
	int		v4;
	int		v5;
};

struct perfume_data {
	int		type;
	int		duration;
	int		aroma_strength;
	int		doses;
	int		v4;
	int		v5;
};

struct food_data {
	int		food_value;
	int		spell1;
	int		spell2;
	int		spell3;
	int		spell4;
	int		bites;
};

struct poison_obj_data {
	int		poison1;
	int		poison2;
	int		poison3;
	int		poison4;
	int		poison5;
	int		doses;
};

struct fluid_data {
	int		alcohol;
	int		water;
	int		food;
	int		v3;
	int		v4;
	int		v5;
};

struct lodged_object_info {
	char		*location;
	int		vnum;
	LODGED_OBJECT_INFO *next;
};

struct lodged_missile_info {
	int			vnum;
	LODGED_MISSILE_INFO	*next;
};

struct wound_data {
	char		*location;
	char		*type;
	char		*name;
	char		*severity;
	int		damage;
	int		bleeding;
	int		poison;
	int		infection;
	int		healerskill;
	int		lasthealed;
	int		lastbled;
	WOUND_DATA	*next;
};

struct object_damage {
	char		*type;
	char		*name;
	char		*severity;
	int		damage;
	OBJECT_DAMAGE	*next;
};

struct default_obj_data {
	int		value [6];
};

union obj_info {
	struct weapon_data		weapon;
	struct drink_con_data	drinkcon;
	struct container_data   container;
	struct default_obj_data od;
	struct light_data		light;
	struct armor_data		armor;
	struct cloak_data		cloak;
	struct ticket_data		ticket;
	struct perfume_data		perfume;
	struct poison_obj_data	poison;
	struct food_data		food;
	struct fluid_data		fluid;
	struct fountain_data	fountain;
};

/* ======================== Structure for object ========================= */
struct obj_data
{
	int				deleted;
	int				virtual;
	int				zone;
	int				in_room;
	struct obj_flag_data 		obj_flags;
	union obj_info			o;
	AFFECTED_TYPE			*xaffected;
	char				*name;
	char				*description;
	char				*short_description;
	char				*full_description;
	char				*omote_str;
	XTRA_DESCR_DATA		*ex_description;
	WRITTEN_DESCR_DATA		*wdesc;
	CHAR_DATA			*carried_by;
	CHAR_DATA			*equiped_by;
	OBJ_DATA			*in_obj;
	OBJ_DATA			*contains;
	OBJ_DATA			*next_content;
	OBJ_DATA			*next;
	OBJ_DATA			*hnext;
	OBJ_DATA			*lnext;
	int				clock;
	int				morphTime;
	int				morphto;
	int				location;
	int				contained_wt;
	int				activation;			/* Affect applied when picked up */
	int				quality;
	int				econ_flags;			/* Flag means used enhanced prices*/
	int				size;
	int				count;				/* How many this obj represents */
	int				obj_timer;
	float				coppers;			/* value in coppers    */
	float				silver;				/* Partial value in silver coins */
	WOUND_DATA			*wounds;			/* For corpses */
	int				item_wear;			/* Percentile; 100%, brand new */
	WRITING_DATA			*writing;
	char				*ink_color;
	int				open;
	OBJ_DATA			*loaded;
	LODGED_OBJECT_INFO		*lodged;
	char				*desc_keys;
	char				*var_color;
	char				*book_title;
	int				title_skill;
	int				title_language;
	int				title_script;
	int				material;
	int				tmp_flags;
	bool				writing_loaded;
	int				coldload_id;
	OBJECT_DAMAGE			*damage;
	char				*indoor_desc;
	int				sold_at;
	int				sold_by;
	TRIGGER_DATA	*triggers;

};


struct room_direction_data
{
	char	*general_description;
	char	*keyword;
	int		exit_info;
	int		key;
	int		pick_penalty;
	int		to_room;
};

struct prog_vars {
	char	*name;
	int		value;
	struct	prog_vars *next;
};

struct room_prog {
	char *command; /* List of commands to trigger this program */
	char *keys;    /* List of valid arguments, NULL always executes */
	char *prog;    /* The program itself */
	struct prog_vars *vars;
	struct room_prog *next; /* next program for this room */
};

struct secret {
	int diff;		/* difficulty (search skill abil) */
	char *stext;
};

#define MAX_DELIVERIES			200
#define MAX_TRADES_IN			200

struct negotiation_data {
	int			ch_coldload_id;
	int			obj_vnum;
	int			time_when_forgotten;
	int			price_delta;
	int			transactions;
	int			true_if_buying;
	struct negotiation_data	*next;
};

struct shop_data {
	float		markup;				/* Objects sold are multiplied by this	*/
	float		discount;			/* Objects bought are muliplied by this	*/
	int			shop_vnum;			/* Rvnum of shop						*/
	int			store_vnum;			/* Rvnum of inventory					*/
	char		*no_such_item1;		/* Message if keeper hasn't got an item */
	char		*no_such_item2;		/* Message if player hasn't got an item */
	char		*missing_cash1;		/* Message if keeper hasn't got cash    */
	char		*missing_cash2;		/* Message if player hasn't got cash    */
	char		*do_not_buy;		/* If keeper dosn't buy such things.	*/
	char		*message_buy;		/* Message when player buys item		*/
	char		*message_sell;		/* Message when player sells item		*/
	int			delivery [MAX_DELIVERIES];		/* Merchant replaces these	*/
	int			trades_in [MAX_TRADES_IN];		/* item_type that can buy	*/
	int			econ_flags1;		/* Bits which enhance price             */
	int			econ_flags2;
	int			econ_flags3;
	int			econ_flags4;
	int			econ_flags5;
	int			econ_flags6;
	int			econ_flags7;
	float		econ_markup1;		/* Sell markup for flagged items        */
	float		econ_markup2;
	float		econ_markup3;
	float		econ_markup4;
	float		econ_markup5;
	float		econ_markup6;
	float		econ_markup7;
	float		econ_discount1;		/* Buy markup for flagged items         */
	float		econ_discount2;
	float		econ_discount3;
	float		econ_discount4;
	float		econ_discount5;
	float		econ_discount6;
	float		econ_discount7;
	int			nobuy_flags;		/* Any econ flags set here arn't traded */
	NEGOTIATION_DATA	*negotiations;	/* Haggling information				*/
	int		opening_hour;
	int		closing_hour;
	int		exit;
};

struct room_extra_data {
	char		*alas [6];
	char		*weather_desc [WR_DESCRIPTIONS];
};

#define PC_TRACK	( 1 << 0 )
#define BLOODY_TRACK	( 1 << 1 )
#define FLEE_TRACK	( 1 << 2 )

struct track_data {
	shortint	race;
	shortint	from_dir;
	shortint	to_dir;
	shortint	hours_passed;
	shortint	speed;
	bitflag		flags;
	TRACK_DATA	*next;
};

struct room_data {
	int					virtual;		/* Make sure this is 1st element */
	shortint			zone;			/* Make sure this is 2nd element */
	shortint			sector_type;
	shortint			deity;			/* Deity owner of the room */
	shortint			light;
	int					search_sequence;
	char				*name;
	char				*description;
	XTRA_DESCR_DATA	*ex_description;
	struct				written_descr_data *wdesc;
	struct				room_direction_data *dir_option[6];
	struct secret		*secrets[6];
	int					room_flags;
	OBJ_DATA			*contents;
	CHAR_DATA			*people;
	struct room_prog	*prg;
	ROOM_DATA			*hnext;		/* next in hash list */
	ROOM_DATA			*lnext;		/* Link next room in room_list */
	AFFECTED_TYPE		*affects;
	ROOM_EXTRA_DATA		*extra;
	char			*creator;
	char			*description_dup;
	int			earth;
	int			wind;
	int			fire;
	int			water;
	int			shadow;
	int			psave_loaded;
	TRACK_DATA		*tracks;
	int			entrance;
	int			occupants;
	TRIGGER_DATA	*triggers;

};

struct reset_time_data {
	int					month;
	int					day;
	int					minute;
	int					hour;
	int					second;
	int					flags;
};

struct time_info_data {
	int					second;
	int					hour;
	int					day;
	int					month;
	int					year;
	int                 season;
	int					minute;
	int					holiday;
};

struct time_data
{
	time_t birth;    /* This represents the characters age                */
	time_t logon;    /* Time of the last logon (used to calculate played) */
	long played;      /* This is the total accumulated time played in secs */
};

struct memory_data {
	char *name;
	struct memory_data *next;
};

struct char_ability_data
{
	int str; 
	int intel;
	int wil; 
	int dex; 
	int con; 
	int aur;
	int agi;
};

struct newbie_hint {
	char		*hint;
	NEWBIE_HINT	*next;
};

struct role_data {
	char 		*summary;
	char 		*body;
	char 		*poster;
	char		*date;
	int		cost;
	int		timestamp;
	ROLE_DATA	*next;
};

struct known_spell_data {
	char *name;
	int rating;
	struct known_spell_data *next;
};

struct affect_spell_type {
	int				duration;	/* For how long its effects will last		*/
	int				modifier;	/* This is added to apropriate ability		*/
	int				location;	/* Tells which ability to change(APPLY_XXX)	*/
	int				bitvector;	/* Tells which bits to set (AFF_XXX)		*/
	int				t;			/* Extra information                        */
	int				sn;			/* Acquired by spell number					*/
	int				technique;
	int				form;
	int				magnitude;
	int				discipline;
	int				mana_cost;
};

struct charm_data {
	char			*name;
	int			original_hours;
	int			current_hours;
	int			power_source;
	int			caster_skill;
	CHARM_DATA	*next;
};

struct affect_job_type {
	int				days;
	int				pay_date;
	int				cash;
	int				count;
	int				object_vnum;
	int				employer;
};

struct affect_table_type {
	int				uu1;
	int				uu2;
	int				uu3;
	int				uu4;
	OBJ_DATA		*obj;
	int				uu6;
};

struct affect_smell_type {
	int				duration;
	int				aroma_strength;
	int				uu3;
	int				uu4;
	int				uu5;
	int				uu6;
};

struct affect_paralyze {
	int				duration;
	int				minutes_until_paralyzed;
	int				uu3;
	int				uu4;
	int				uu5;
	int				sn;
};

struct affect_shadow_type {
	CHAR_DATA		*shadow;		/* target begin shadowed              */
	int				edge;			/* -1, center.  0-5 edge by direction */
};

struct affect_hidden_type {
	int				duration;
	int				hidden_value;
	int				coldload_id;
	int				uu4;
	int				uu5;
	int				uu6;
};

struct affect_toll_type {
	int				duration;		/* Generally forever. */
	int				dir;
	int				room_num;		/* for verification */
	int				charge;			/* Cost for others to pass toll crossing */
	int				uu5;
	int				uu6;
};

struct affect_room_type {
    int             duration;
	int				uu2;
	int				uu3;
	int				uu4;
	int				uu5;
	int				uu6;
};

struct affect_craft_type {
	int					timer;
	int				skill_check;
	PHASE_DATA			*phase;
	SUBCRAFT_HEAD_DATA	*subcraft;
	CHAR_DATA			*target_ch;
	OBJ_DATA			*target_obj;
};

struct affect_listen_type {  /* For muting and later, directed listening */
	int				duration; /* Always on if it exists */
	int 				on; /* nonzero is on*/
};

struct affect_herbed_type { /* For counting the number of herbs found in rooms*/
	int				duration;
	int				timesHerbed;
};

/* Agent FX - let the suffering begin (soon)
enum AGENT_FORM {
	AGENT_NONE=0, 
	AGENT_FUME, 
	AGENT_POWDER, 
	AGENT_SOLID, 
	AGENT_SALVE, 
	AGENT_LIQUID, 
	AGENT_SPRAY
};
enum AGENT_METHOD {
	AGENT_NONE=0, 
	AGENT_INJECTED, 
	AGENT_INHALED, 
	AGENT_INGESTED, 
	AGENT_INWOUND, 
	AGENT_TOUCHED
};
****************************************/

/*
  SOMATIC RESPONSES by Sighentist
  
  Basically the idea here, is that the magnitude of the effect is
  enveloped over time. So after a delay, the effect begins to grow
  in strength until it reaches a peak. After cresting the peak the
  strength weakens to a fraction of the peak strength. It remains
  fixed there until the effect begins to wear off completely.
  
  Notes:
  If latency == duration the affected is a carrier.

*/
struct affect_soma_type {	/* SOMA EFFECTS TBA */
	int			duration;	/* rl hours */
	unsigned short int	latency;	/* rl hours of delay  */

	unsigned short int	minute;		/* timer  */
	unsigned short int	max_power;	/* type-dependant value */
	unsigned short int	lvl_power;	/* fraction of max_power */
	unsigned short int 	atm_power;	/* the current power */

	unsigned short int	attack;		/* minutes to amp to max_power */
	unsigned short int	decay;		/* minutes to decay to lvl_power */
	unsigned short int	sustain;	/* minutes to lvl_power drops */
	unsigned short int	release;	/* minutes to end of effect */
};

union affected_union {
	struct affect_spell_type	spell;
	struct affect_job_type		job;
	struct affect_table_type	table;
	struct affect_shadow_type	shadow;
	struct affect_paralyze      paralyze;
	struct affect_smell_type	smell;
	struct affect_hidden_type	hidden;
	struct affect_room_type		room;
	struct affect_toll_type		toll;
	struct affect_craft_type	*craft;
	struct affect_listen_type	listening;
	struct affect_herbed_type	herbed;
        struct affect_soma_type         soma;
};

struct affected_type {
	int						type;
	union affected_union	a;
	AFFECTED_TYPE			*next;
};

#ifdef HAHA
struct affected_type {
	int				type;		/* The type of spell that caused this		*/
	int				duration;	/* For how long its effects will last		*/
	int				modifier;	/* This is added to apropriate ability		*/
	int				location;	/* Tells which ability to change(APPLY_XXX)	*/
	int				bitvector;	/* Tells which bits to set (AFF_XXX)		*/
	int				t;			/* Extra information                        */
	int				sn;			/* Acquired by spell number					*/
	AFFECTED_TYPE	*next;
};
#endif

struct second_affect {
	int				type;
	int				seconds;
	CHAR_DATA		*ch;
	OBJ_DATA		*obj;
	char			*info;
	int				info2;
	SECOND_AFFECT	*next;
};

struct var_data {
	char				*name;
	int					value;
	int					type;
	struct var_data		*next;
};

#define MPF_BROKEN		(1 << 0)

struct mobprog_data {
	char				*trigger_name;
	char				*prog;
	char				*line;
	int					busy;
	int					flags;
	int					mob_virtual;
	struct mobprog_data	*next;
	struct mobprog_data	*next_full_prog;
};

struct dream_data {
	char				*dream;
	DREAM_DATA			*next;
};

struct site_info {
	char				*name;
	char				*banned_by;
	int				banned_on;
	int				banned_until;
	SITE_INFO			*next;
};

struct stored_character {
	char				*name;
	int				level;
	int				state;
	STORED_PC_DATA			*next;
};

#define	ACCOUNT_NOPETITION		( 1 << 0 )
#define ACCOUNT_NOBAN			( 1 << 1 )
#define ACCOUNT_NOGUEST			( 1 << 2 )
#define ACCOUNT_NOPSI			( 1 << 3 )
#define ACCOUNT_NORETIRE		( 1 << 4 )
#define ACCOUNT_NOVOTE			( 1 << 5 )
#define ACCOUNT_IPSHARER		( 1 << 6 )
#define ACCOUNT_RPPDISPLAY		( 1 << 7 )

struct account_data {
	char				*name;
	char				*pwd;
	char				*email;
	char				*last_ip;
	STORED_PC_DATA			*pc;
	int				roleplay_points;
	int				created_on;
	int				pwd_attempt;
	int				color;
	int				web_login;
	bool				admin;
	bool				newsletter;
	int				flags;
	int				forum_posts;
	int				timezone;
	int				deleted;
	int				code;
	int				last_rpp;
	ACCOUNT_DATA			*next;
};

#define CHARGEN_INT_FIRST		( 1 << 0 )
#define CHARGEN_INT_SECOND		( 1 << 1 )
#define CHARGEN_DPASSAGE_1ST2ND		( 1 << 2 )


struct pc_data {
	DREAM_DATA			*dreams;
	DREAM_DATA			*dreamed;
	int					create_state;	/* Approval system */
	int					mortal_mode;	/* Immortals can act mortal */
	int					edit_obj;
	int					edit_mob;
	int					level;
	int					boat_virtual;
	int					staff_notes;
	int					mount_speed;
	CHAR_DATA			*edit_player;
	CHAR_DATA			*target_mob;	/* Used by 'mob' and 'resets' */
	char				*msg;			/* SUBMIT/APPROVAL system */
	char				*email_address;
	char				*creation_comment;
	char				*imm_enter;
	char				*imm_leave;
	char				*site_lie;		/* Lie about connecting site */
	int					start_str;
	int					start_dex;
	int					start_con;
	int					start_wil;
	int					start_aur;
	int					start_intel;
	int					start_agi;
	int					load_count;		/* See load_pc */
	int					common;			/* Msg seen of common knowledge */
	int					chargen_flags;
	int					last_global_pc_msg;
	int					last_global_staff_msg;
	int					sleep_needed;
	int					auto_toll;		/* Amount willing to pay if following */
	int					doc_type;       /* Not saved.  Current doc */
	int					doc_index;		/* Not saved.  type relative index */
	DESCRIPTOR_DATA		*owner;
	shortint			skills [MAX_SKILLS];
	CHAR_DATA			*dot_shorthand;	/* '.' indicates last referenced n/pc */
	ALIAS_DATA			*aliases;
	ALIAS_DATA			*execute_alias;
	time_t				last_logon;
	time_t				last_logoff;
	time_t				last_disconnect;
	time_t				last_connect;
	time_t				last_died;
	char				*account;
	OBJ_DATA			*writing_on;
	SUBCRAFT_HEAD_DATA		*edit_craft;
	int				app_cost;
	bitflag				nanny_state;
	bitflag				role;
	ROLE_DATA			*special_role;
	bool				admin_loaded;
	int				time_last_activity;
	int				is_guide;
	int				profession;
	int				last_rpp;
};

struct threat_data {
	CHAR_DATA			*source;
	int				level;
	THREAT_DATA			*next;
};

struct attacker_data {
	CHAR_DATA			*attacker;
	ATTACKER_DATA			*next;
};

struct viewed_data {
	CHAR_DATA			*target;
	VIEWED_DATA			*next;
};

struct mob_data {
	int					virtual;
	int					zone;
	int					spawnpoint;
	int					merch_seven;    /* Merchant set up for 7 economic rules like Regi wanted - punt in time */
	int					skinned_vnum;	/* What mob skins into */
	int					carcass_vnum;	/* What mob leaves for a carcass*/
	int					vehicle_type;	/* Mobile value: boat, etc */
	int					helm_room;		/* If boat, defines helm */
	int					access_flags;	/* Flags; mob room access */
	int					noaccess_flags;	/* Flags; mob room noaccess */
	int					reset_zone;		/* mobs only */
	int					reset_cmd;		/* mobs only */
	int					damnodice;
	int					damsizedice;
	int					damroll;
	CHAR_DATA			*hnext;
	CHAR_DATA			*lnext;
	COMBAT_DATA			*combat;
	RESET_DATA			*resets;
	int				currency_type;
	char				*owner;

};

/* ================== Structure for player/non-player ===================== */
struct char_data
{
	int					in_room;
	ROOM_DATA			*room;
	int					deleted;
	int					circle;			/* Rank within church */
	int					fight_mode;		/* Frantic..defensive */
	int					debug_mode;
	int					primary_delay;	/* ticks to next hit */
	int					secondary_delay;/* ticks to next hit */
	int					coldload_id;	/* Uniq id of mob */
	int					natural_delay;	/* Added delay for natural */
	int					body_type;		/* Determines hit locations */
	int					poison_type;	/* Bite causes poison */
	int					nat_attack_type;/* Bite, claw, slime, etc */
	int					flags;			/* FLAG_ stuff */
	int					move_points;	/* Not saved; move remainder */
	int					hit_points;		/* Not saved; hit remainder */
	int					nightmare;		/* Nightmare sequence # */
	int					speaks;			/* Currently spoken language */
	int					alarm;			/* Not saved. */
	int					trigger_delay;	/* Not saved. */
	int					trigger_line;	/* Used with trigger_delay */
	int					trigger_id;		/* Used with trigger_delay */
	int					psionic_talents;
	CHAR_DATA			*subdue;		/* Subduer or subduee */
	MOBPROG_DATA		*prog;
	struct var_data		*vartab;		/* Mob program variable table */
	SHOP_DATA			*shop;
	CHAR_DATA			*vehicle;		/* Char that is the vehicle */
	int					str;
	int					intel;
	int					wil;
	int					dex;
	int					con;
	int					aur;
	int					agi;
	int					tmp_str;
	int					tmp_intel;
	int					tmp_wil;
	int					tmp_dex;
	int					tmp_con;
	int					tmp_aur;
	int					tmp_agi;
	shortint			skills [MAX_SKILLS];
	AFFECTED_TYPE		*hour_affects;
	OBJ_DATA			*equip;
	DESCRIPTOR_DATA		*desc;
	CHAR_DATA			*next_in_room;
	CHAR_DATA			*next;
	CHAR_DATA			*next_fighting;
	CHAR_DATA			*following;
	PC_DATA				*pc;
	MOB_DATA			*mob;
	MOVE_DATA			*moves;
	char				*casting_arg;	/* Contains delayed spell */
	int					hit;
	int					max_hit;
	int					move;
	int					max_move;
	int					armor;
	int					offense;
	int					ppoints;
	CHAR_DATA			*fighting;
	int				distance_to_target;
	struct memory_data	*remembers;
	long				affected_by;
	int					position;
	int					default_pos;
	bitflag				act;
    bitflag                hmflags; /* depreciated*/
	int					carry_weight;
	int					carry_items;
	int					delay_type;
	int					delay;
	char				*delay_who;
	char				*delay_who2;
	CHAR_DATA			*delay_ch;
	OBJ_DATA			*delay_obj;
	int					delay_info1;
	int					delay_info2;
	int					was_in_room;
	int					intoxication;
	int					hunger;
	int					thirst;
	int					last_room;
	int					attack_type;
	char				*name;
	char				*tname;
	char				*short_descr;
	char				*long_descr;
	char				*pmote_str;
	char				*voice_str;
	char				*description;
	int					sex;
	int					deity;
	int					race;
	int					color;
	int					speed;
	int					age;
	int					height;
	int					frame;
	int					idol;
 	struct time_data	time;
	char				*clans;
	CHAR_DATA			*mount;				/* Rider/Ridee */
	CHAR_DATA			*hitcher;
	CHAR_DATA			*hitchee;
	char				*combat_log;
	WOUND_DATA			*wounds;
	int				damage;
	int				lastregen;
	int				defensive;
	int				cell_1, cell_2, cell_3;		/* NPC jailer cell VNUMS */
	int				laststuncheck;
	int				knockedout;
	int				writes;
	int				stun;
	int				curse;
	CHAR_DATA			*aiming_at;
	CHAR_DATA			*targeted_by;
	int				aim;
	LODGED_OBJECT_INFO		*lodged;
	int				mana;
	int				max_mana;
	int				harness;
	int				max_harness;
	int				preparing_id;
	int				preparing_time;
	CHARM_DATA		*charms;
	int				roundtime;
	OBJ_DATA			*right_hand;
	OBJ_DATA			*left_hand;
	bitflag				plr_flags;
	CHAR_DATA			*ranged_enemy;
	char				*enemy_direction;
	THREAT_DATA			*threats;
	ATTACKER_DATA			*attackers;
	int				whirling;
	int				from_dir;
	VIEWED_DATA			*sighted;
	POISON_DATA			*venom;
	int				balance;
    char                            *travel_str;
    int                spells [1] [2];/* depreciated*/
	TRIGGER_DATA	*triggers;

};


/* ======================================================================== */


struct weather_data
{
	int fog;
    int sunlight;
    int trend;
    int temperature;
	int state;
    int clouds;
	int lightning;
	int wind_dir;
	int wind_speed;
};

/* ***********************************************************
*  The Following structures are related to descriptor_data   *
*********************************************************** */

struct txt_block
{
	char *text;
	struct txt_block *next;
};

struct txt_q
{
	struct txt_block *head;
	struct txt_block *tail;
};

struct snoop_data
{
	struct char_data *snooping;	
	struct char_data *snoop_by;
};

typedef void CALL_PROC ( DESCRIPTOR_DATA *d);

struct line_data {
	char			*line [2000];
};

struct descriptor_data
{
	int descriptor;	              /* file descriptor for socket */
	char *host;                /* hostname                   */
	char *ip_addr;		   /* original IP, just in case	    */
	int connected;                /* mode of 'connectedness'    */
	int wait;                     /* wait for how many loops    */
	char *showstr_head;	          /* for paging through texts	*/
	char *showstr_point;	      /*       -                    */
	char *header;
	char **str;                   /* for the modify-str system  */
	int	edit_type;				  /* Type of document being edited */
	int edit_index;				  /* Type relative index for edit */
	char *edit_string;			  /* String being edited by edit command */
	int edit_length;			  /* Length of piece being edited */
	int edit_line_first;		  /* Starting line edited in document */
	int max_str;                  /* -                          */
	int prompt_mode;              /* control of prompt-printing */
	char buf[MAX_STRING_LENGTH];  /* buffer for raw input       */
	char last_input[MAX_INPUT_LENGTH];/* the last input         */
	struct txt_q	output;       /* q of strings to send       */
	struct txt_q	input;        /* q of unprocessed input     */
	ACCOUNT_DATA		*account;	/* login account */
	CHAR_DATA		*character;	  /* linked to char             */
   	CHAR_DATA		*original;	  /* original char              */
	struct snoop_data snoop;      /* to snoop people.	         */
	DESCRIPTOR_DATA	*next;		  /* link to next descriptor    */
	CALL_PROC		*proc;		  /* Procedure to call after input */
	int				parm;
	char			*screen;	/* data being edited */
	LINE_DATA		*lines;
	int				col;
	int				line;
	int				tos_line;	/* top of screen line 1 */
	int				max_lines;
	int				max_columns;
	int				edit_mode;
	struct timeval	login_time;
	MESSAGE_DATA	*pending_message;
	int				time_last_activity;
	int				idle;
	char			*web_buf;
	char			*inputstream;
	int				bytes_in_stream;
	char *stored;			/* miscellaneous storage */
	int		color;		/* ANSI color */
	int				resolved;
};

struct msg_type
{
	char *attacker_msg;
	char *victim_msg;
	char *room_msg;
};

struct message_type
{
	struct msg_type die_msg;
	struct msg_type miss_msg;
	struct msg_type hit_msg;
	struct msg_type sanctuary_msg;
	struct msg_type god_msg;
	struct message_type *next;
};

struct message_list
{
	int a_type;
	int number_of_attacks;
	struct message_type *msg;
};

struct dex_skill_type
{
	int p_pocket;
	int p_locks;
	int traps;
	int sneak;
	int hide;
};

struct str_app_type
{
	int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
	int todam;    /* Damage Bonus/Penalty                */
	int carry_w;  /* Maximum weight that can be carrried */
	int wield_w;  /* Maximum weight that can be wielded  */
};

struct constant_data {
	char		constant_name [20];
	char		description [80];
	void		**index;
};

#define args( list )	list


#define STR_ONE_LINE		2001
#define STR_MULTI_LINE		2000

#define MAX_NATURAL_SKILLS	50

struct skill_data_list {
    char    skill_name[30];
    int        skill_type;
    int        rpp_cost;
};

struct race_data {
	char		name [30];
	int			attr_mods [7];
	int			height_short;
	int			height_average;
	int			height_tall;
	int			female_height_diff;
	int			race_speaks;
	int			natural_skills [MAX_NATURAL_SKILLS];
};

struct religion_data {
	char	*tree [30];
};

#define MAX_REGISTRY        50

typedef struct registry_data    REGISTRY_DATA;

struct registry_data {
    int                 value;
    char                *string;
    REGISTRY_DATA       *next;
};

struct fight_data {
	char		name [12];
	float		offense_modifier;
	float		defense_modifier;
	int			delay;
};

struct language_data {
	char name [16];
};

struct lang_info {
		char	lang [20];
		int		skill;
	};
	
struct poison_data {
	int		poison_type;
	int		duration_die_1;
	int		duration_die_2;
	int		effect_die_1;
	int		effect_die_2;
	POISON_DATA	*next;
};

struct encumberance_info {
	int		str_mult_wt;	/* if wt <= str * str_mult, then element applies */
	int		defense_pct;
	int		delay;
	int		offense_pct;
	int		move;
	float	penalty;
	char	*encumbered_status;
};

#define NUM_BUCKETS 1024
#define CUMBERANCE_ENTRIES	6

	/* The FLAG_ bits are saved with the player character */

#define FLAG_KEEPER			( 1 << 0 )
#define FLAG_COMPACT		( 1 << 1 )		/* Player in compact mode */
#define FLAG_BRIEF			( 1 << 2 )		/* Player in brief mode */
#define FLAG_WIZINVIS		( 1 << 3 )
#define FLAG_SUBDUEE		( 1 << 4 )
#define FLAG_SUBDUER		( 1 << 5 )
#define FLAG_SUBDUING		( 1 << 6 )
#define FLAG_ANON			( 1 << 7 )
#define FLAG_COMPETE		( 1 << 8 )
#define FLAG_LEADER_1		( 1 << 9 )		/* Clan 1 leader */
#define FLAG_LEADER_2		( 1 << 10 )		/* Clan 2 leader */
#define FLAG_DEAD			( 1 << 11 )		/* Player has been killed */
#define FLAG_KILL			( 1 << 12 )		/* Player intends to kill */
#define FLAG_FLEE			( 1 << 13 )		/* Player wants to flee combat */
#define FLAG_BINDING		( 1 << 14 )		/* NPC is curently tending wounds */
#define FLAG_SEE_NAME		( 1 << 15 )		/* Show mortal name in says */
#define FLAG_AUTOFLEE		( 1 << 16 )		/* Flee automatically in combat */
#define FLAG_ENTERING		( 1 << 17 )
#define FLAG_LEAVING		( 1 << 18 )
#define FLAG_INHIBITTED		( 1 << 19 )		/* Mob event blocking on program */
#define FLAG_NOPROMPT		( 1 << 20 )		/* Make prompt disappear */
#define FLAG_WILLSKIN       ( 1 << 21 )     /* Everyone can skin corpse */
#define FLAG_ALIASING		( 1 << 22 )		/* Executing an alias */
#define FLAG_OSTLER			( 1 << 23 )     /* Stablemaster */
#define FLAG_TELEPATH 		( 1 << 24 )		/* Hears PC thoughts */
#define FLAG_PACIFIST		( 1 << 25 )		/* Character won't fight back */
#define FLAG_WIZNET		( 1 << 26 )		/* Immortal wiznet toggle */
#define FLAG_HARNESS		( 1 << 27 )		/* Display harness in prompt */
#define FLAG_VARIABLE		( 1 << 28 )		/* Randomized mob prototype */
#define FLAG_ISADMIN		( 1 << 29 )		/* Is an admin's mortal PC */
#define FLAG_AVAILABLE		( 1 << 30 )		/* Available for petitions */
#define FLAG_GUEST		( 1 << 31 )		/* Guest login */

/* plr_flags */

#define NEWBIE_HINTS	( 1 << 0 )		/* Toggle the hint system on or off */
#define NEWBIE			( 1 << 1 )		/* Has not yet commenced */
#define MORALE_BROKEN	( 1 << 2 )
#define MORALE_HELD		( 1 << 3 )
#define FLAG_PETRIFIED	( 1 << 4 )
#define NEW_PLAYER_TAG	( 1 << 5 )		/* Displays (new player) in their ldescs */
#define MENTOR			( 1 << 6 )		/* PC Mentor flag */
#define NOPETITION		( 1 << 7 )		/* No Petition */
#define PRIVATE			( 1 << 8 )		/* Non-Guide-reviewable app */
#define START_ALPHA     ( 1 << 9 )      /* Human chose to start in ALPHA room */
#define START_BETA      ( 1 << 10 )     /* Human chose to start in BETA room*/
#define START_GAMMA		( 1 << 11 )		/* Human chose to start in GAMMA room */
#define NO_PLAYERPORT	( 1 << 12 )		/* Admins w/out admin access to 4500 */
#define MUTE_BEEPS		( 1 << 13 )		/* Doesn't receive NOTIFY beeps */
#define COMBAT_FILTER	( 1 << 14 )		/* Only receive local combat messages */
#define GROUP_CLOSED	( 1 << 15 )		/* Not accepting any other followers */

#define STATE_NAME		( 1 << 0 )		/* Enter name */
#define STATE_GENDER	( 1 << 1 )		/* Choose gender */
#define STATE_RACE		( 1 << 2 )		/* Choose race */
#define STATE_AGE		( 1 << 3 )		/* Input age */
#define STATE_ATTRIBUTES	( 1 << 4 )		/* Distribute attributes */
#define STATE_SDESC		( 1 << 5 )		/* Enter short desc */
#define STATE_LDESC		( 1 << 6 )		/* Enter long desc */
#define STATE_FDESC		( 1 << 7 )		/* Enter full desc */
#define STATE_KEYWORDS	( 1 << 8 )		/* Enter keywords */
#define STATE_FRAME		( 1 << 9 )		/* Choose frame */
#define STATE_SKILLS	( 1 << 10 )		/* Skill selection */
#define STATE_COMMENT	( 1 << 11 )		/* Creation comment */
#define STATE_ROLES		( 1 << 12 )		/* Hardcoded roles/advantages */
#define STATE_SPECIAL_ROLES	( 1 << 13 )		/* Admin-posted special roles */
#define STATE_PRIVACY		( 1 << 14 )		/* Flag app private? */
#define STATE_LOCATION		( 1 << 15 )		/* Humans choose start loc */
#define STATE_PROFESSION	( 1 << 16 )		/* Choosing profession */

/* Hardcoded starting roles/options */

#define XTRA_COIN		( 1 << 0 )		/* Starts with surplus coin; 1 point */
#define APPRENTICE		( 1 << 1 )		/* Starts as low-ranking member of established org; 1 point */
#define STARTING_ARMOR		( 1 << 2 )		/* Starts with a full set of leather armor; 2 points */
#define SKILL_BONUS		( 1 << 3 )		/* Small bonus to starting skills; 2 points */
#define XTRA_SKILL		( 1 << 4 )		/* Extra starting skill; 3 points */
#define MAXED_STAT		( 1 << 5 )		/* Starts with one guaranteed 18; 3 points */
#define JOURNEYMAN		( 1 << 6 )		/* As apprentice, but higher-level; 4 points */
#define FELLOW			( 1 << 7 )		/* Starts as a fully-fledged Fellow; 5 points */
#define LESSER_NOBILITY		( 1 << 8 )		/* Starts in a minor noble/merchant family; 6 points */
#define APPRENTICE_MAGE		( 1 << 9 )		/* Self-explanatory; 7 points */

#define COLNUM_NONE			0
#define COLNUM_RED			1
#define COLNUM_GREEN		2
#define COLNUM_YELLOW		3
#define COLNUM_BLUE			4
#define COLNUM_MAGENTA		5
#define COLNUM_CYAN			6
#define COLNUM_WHITE		7
#define COLNUM_OFF			8
#define COLNUM_BOLD_RED     9
#define COLNUM_BOLD_GREEN   10
#define COLNUM_BOLD_YELLOW  11
#define COLNUM_BOLD_BLUE    12
#define COLNUM_BOLD_MAGENTA 13
#define COLNUM_BOLD_CYAN    14
#define COLNUM_BOLD_WHITE   15

#define COL_NONE        "\x1B[0m"
#define COL_RED         "\x1B[31m"
#define COL_GREEN       "\x1B[32m"
#define COL_YELLOW      "\x1B[33m"
#define COL_BLUE        "\x1B[34m"
#define COL_MAGENTA     "\x1B[35m"
#define COL_CYAN        "\x1B[36m"
#define COL_WHITE       "\x1B[37m"
#define COL_OFF         "\x1B[0m"
#define COL_BOLD_RED        "\x1B[1;31m"
#define COL_BOLD_GREEN      "\x1B[1;32m"
#define COL_BOLD_YELLOW     "\x1B[1;33m"
#define COL_BOLD_BLUE       "\x1B[1;34m"
#define COL_BOLD_MAGENTA    "\x1B[1;35m"
#define COL_BOLD_CYAN       "\x1B[1;36m"
#define COL_BOLD_WHITE      "\x1B[1;37m"

#define ANSI_AUTO_DETECT	"\x1B[6n"

#define TRIG_DONT_USE	0
#define TRIG_SAY		1
#define TRIG_ENTER		2
#define TRIG_PASSAGE	3
#define TRIG_HIT		4
#define TRIG_MOBACT		5
#define TRIG_ALARM		6
#define TRIG_HOUR		7
#define TRIG_DAY		8
#define TRIG_TEACH		9
#define TRIG_WHISPER	10
#define TRIG_PRISONER	11
#define TRIG_KNOCK		12

#define PLAYER_PORT		4500
#define BUILDER_PORT	4501
#define TEST_PORT		4502

#define MP_TYPE_INTEGER		1
#define MP_TYPE_CHAR_DATA	2
#define MP_TYPE_OBJ_DATA	3
#define MP_TYPE_ROOM_DATA	4
#define MP_TYPE_STRING		5
#define MP_FUNCTION			6
#define MP_LESS_THAN		7		/* First valid binary operations */
#define MP_GREATER_THAN		8
#define MP_EQUAL			9
#define MP_GREATER_EQUAL	10
#define MP_LESSER_EQUAL		11
#define MP_NOT_EQUAL		12
#define MP_MOD				13
#define MP_DIV				14
#define MP_SUB				15
#define MP_PLUS				16
#define MP_AND				17
#define MP_OR				18
#define MP_MULT				19		/* Last valid binary operation (ranged) */
#define MP_OPEN_PAR			20
#define MP_CLOSE_PAR		21
#define MP_COMMA			22
#define MP_TYPE_FOLLOW_DATA	23

#define MT_OPEN_PAREN		1
#define MT_VARIABLE			2
#define MT_COMMA			3
#define MT_EQUAL			4

#define MUD_YEAR		12 * MUD_MONTH
#define MUD_MONTH		30 * MUD_DAY
#define MUD_DAY			24 * MUD_HOUR
#define MUD_HOUR		60 * 60
#define MUD_MINUTE		60

#define MODE_COMMAND		(1 << 0)
#define MODE_DONE_EDITING	(1 << 1)

#define HOME				"\033[H"
#define CLEAR				"\033[2J"

#define DEAD		0
#define MORT		1
#define UNCON		2
#define STUN		3
#define SLEEP		4
#define REST		5
#define SIT			6
#define FIGHT		7
#define STAND		8

#define C_LV1		( 1 << 0 )		/* Immortal level 1 */
#define C_LV2		( 1 << 1 )		/* Immortal level 2 */
#define C_LV3		( 1 << 2 )		/* Immortal level 3 */
#define C_LV4		( 1 << 3 )		/* Immortal level 4 */
#define C_LV5		( 1 << 4 )		/* Immortal level 5 */
#define	C_DEL		( 1 << 5 )		/* Will not break a delay */
#define C_SUB		( 1 << 6 )		/* Commands legal while subdued */
#define C_HID		( 1 << 7 )		/* Commands that keep character hidden */
#define C_DOA		( 1 << 8 )		/* Commands allowed when dead */
#define C_BLD		( 1 << 9 )		/* Commands allowed when blind */
#define C_WLK		( 1 << 10 )		/* Commands NOT allowed while moving */
#define C_XLS		( 1 << 11 )		/* Don't list command */
#define C_MNT		( 1 << 12 )		/* Commands NOT allowed while mounted */
#define C_PAR		( 1 << 13 )     	/* Things you CAN do paralyzed */
#define C_GDE		( 1 << 14 )		/* Guide-only command */
#define C_SPL		( 1 << 15 )		/* Commands legal while preparing spell */
#define C_NLG		( 1 << 16 )		/* Command is not logged */
#define C_NWT		( 1 << 17 )		/* Command doesn't show up in SNOOP or WATCH */

struct command_data {
		char	*command;
	    void	(*proc) (CHAR_DATA *ch, char *argument, int cmd);
		int		min_position;
		int		flags;
};

struct social_data {
	char	*social_command;
	int		hide;
	int		min_victim_position;	/* Position of victim */

				/* No argument was supplied */

	char	*char_no_arg;
	char	*others_no_arg;

				/* An argument was there, and a victim was found */

	char	*char_found;			/* if NULL, read no further, ignore args */
	char	*others_found;
	char	*vict_found;

				/* An argument was there, but no victim was found */

	char	*not_found;

				/* The victim turned out to be the character */

	char	*char_auto;
	char	*others_auto;
};

	/* data files used by the game system */

#define DFLT_DIR        "lib"                /* default data directory     */
#define NEWS_FILE       "../lib/news" 	     /* for the 'news' command     */
#define QSTAT_FILE	 	"text/chargen/stat_message"
#define GREET_FILE	  	"text/greetings"
#define MAINTENANCE_FILE	"text/greetings.maintenance"
#define MENU1_FILE	  	"text/menu1"
#define ACCT_APP_FILE	"text/account_application"
#define ACCT_REF_FILE	"text/account_referral"
#define ACCT_EMAIL_FILE	"text/account_email"
#define ACCT_POLICIES_FILE	"text/account_policies"
#define THANK_YOU_FILE	"text/thankyou"

#define PDESC_FILE		"text/chargen/new_desc"
#define NAME_FILE		"text/chargen/new_name"
#define PKEYWORDS_FILE	"text/chargen/new_keyword"
#define PLDESC_FILE		"text/chargen/new_ldesc"
#define PSDESC_FILE		"text/chargen/new_sdesc"
#define RACE_SELECT		"text/chargen/race_select"	/* Race choice question */
#define AGE_SELECT		"text/chargen/age_select"	/* Age choice question */
#define SEX_SELECT		"text/chargen/sex_select"	/* Sex choice question */
#define ROLE_SELECT		"text/chargen/role_select"	/* Explanation of roles */
#define SPECIAL_ROLE_SELECT	"text/chargen/special_role_select"	/* Explanation of special roles */
#define HEIGHT_FRAME	"text/chargen/height_frame"
#define COMMENT_HELP	"text/chargen/comment_help"
#define LOCATION		"text/chargen/location"
#define SKILL_SELECT	"text/chargen/skills_select"
#define PROFESSION_SELECT	"text/chargen/professions"

#define COMMON			"text/common"		 /* Common knowledge           */
#define PLAYER_NEW		"text/player_new"
#define IDEA_FILE       "../log/ideas"       /* for the 'idea'-command     */
#define TYPO_FILE       "../log/typos"       /*         'typo'             */
#define BUG_FILE        "../log/bugs"        /*         'bug'              */
#define MESS_FILE       "text/messages"      /* damage message             */
#define SOCMESS_FILE    "text/actions"       /* messgs for social acts     */
#define HELP_FILE		"text/help_table"    /* for HELP <keywrd>          */
#define BHELP_FILE		"text/bhelp_table"   /* for BHELP <keywrd>         */
#define HELP_PAGE_FILE  "text/help"          /* for HELP <CR>              */
#define BHELP_PAGE_FILE "text/bhelp"         /* for BHELP <CR>             */
#define INFO_FILE       "text/info"          /* for INFO                   */
#define WIZLIST_FILE    "text/wizlist"       /* for WIZLIST                */
#define RECIPE_FILE		"text/alchemy"		 /* alchemy combination defs   */


#define SAVE_DIR		"save"
#define HELP_DIR		"../lib/help/"
#define INFO_DIR		"../lib/info/"
#define REGIONS			"../regions"
#define REGISTRY_FILE	REGIONS "/registry"
#define CRAFTS_FILE		REGIONS "/crafts"
#define BOARD_DIR		"boards"
#define JOURNAL_DIR		"player_journals"
#define BOARD_ARCHIVE	"archive"
#define PLAYER_BOARD_DIR "player_boards"
#define VIRTUAL_BOARD_DIR	"vboards"
#define TICKET_DIR		"tickets"
#define STATS_FILE		"stats"
#define COLDLOAD_IDS	"coldload_ids"
#define CLAN_REGISTRY REGIONS "/clan_registry"
#define STAYPUT_FILE	"stayput"

#define ZONE_SIZE	1000
#define MAX_TRADE	5
#define MAX_PROD	5

		/* structure for the reset commands */

struct reset_affect {
    char	*craft_name;
	int		type;
	int		duration;
	int		modifier;
	int		location;
	int		bitvector;
	int		t;
	int		sn;
};

struct reset_com {
	char command;   /* current command                      */ 
	bool if_flag;   /* if TRUE: exe only if preceding exe'd */
	int arg1;       /* (Can be ptr to reset_affect data)    */
	int arg2;       /* Arguments to the command             */
	int arg3;       /*                                      */
	int arg4;       /*                                      */
	int arg5;       /*                                      */
	int enabled;	/* true if this reset should be used	*/

	/* 
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
	*  'C': Craft (an affect) *
	*  'A': Affect            *
	*  'R': Room #            *
	*  'r': Room Affect       *
	*  'm': Mob reset (timed, reply variety)
	*/
};

	/* zone definition structure. for the 'zone-table'   */

struct zone_data {
	char 			*name;             /* name of this zone                  */
	char 			*lead;		/* Name of the project lead		*/
	int 			lifespan;           /* how long between resets (minutes)  */
	int 			age;                /* current age of this zone (minutes) */
	int				top;
	int 			frozen;
	unsigned long 	flags;
	int 			reset_mode;         /* conditions for reset (see below)   */
	struct reset_com 	*cmd;  /* command table for reset	           */
	int			jailer;
	int			jail_room_num;
	ROOM_DATA	*jail_room;
	int			earth_mod;
	int			wind_mod;
	int			fire_mod;
	int			water_mod;
	int			shadow_mod;
	int			player_in_zone;
	int			weather_type;
};

#define MAX_MSGS_PER_BOARD		5000

struct board_data {
	char			*name;
	char			*title;
	int				level;
	int				next_virtual;
	int				msg_nums [MAX_MSGS_PER_BOARD];
	char			*msg_titles [MAX_MSGS_PER_BOARD];
	BOARD_DATA		*next;
};

#define MF_READ		( 1 << 0 )
#define MF_ANON		( 1 << 1 )
#define MF_PRIVATE	( 1 << 2 )
#define MF_URGENT	( 1 << 3 )
#define MF_DREAM	( 1 << 4 )
#define MF_REPLIED	( 1 << 5 )

struct mudmail_data {
	long			flags;
	char			*from;
	char			*from_account;
	char			*date;
	char			*subject;
	char			*message;
	MUDMAIL_DATA		*next_message;
};

struct message_data {
	int			virtual;
	long			flags;
	char			*poster;
	char			*date;
	char			*subject;
	char			*info;
	char			*message;
	char			*icdate;
};

#define TO_ROOM		( 1 << 0 )
#define TO_VICT		( 1 << 1 )
#define TO_NOTVICT	( 1 << 2 )
#define TO_CHAR		( 1 << 3 )
#define TO_ACT_FORMAT	( 1 << 4 )
#define TO_IMMS		( 1 << 5 )
#define TO_ACT_COMBAT	( 1 << 6 )

struct fatigue_data {
	int			percent;
	float		penalty;
	char		name [25];
};

struct use_table_data {
	int		delay;
};

	/* NOTE:  If adding a new COMBAT_, update cs_name in fight.c */

#define SUC_CF				0
#define SUC_MF				1
#define SUC_MS				2
#define SUC_CS				3

	/* NOTE:  If adding a new RESULT_, update rs_name in fight.c */

#define RESULT_NONE			0
#define RESULT_ADV			1
#define RESULT_BLOCK			2
#define RESULT_PARRY			3
#define RESULT_FUMBLE			4
#define RESULT_HIT			5
#define RESULT_HIT1			6
#define RESULT_HIT2			7
#define RESULT_HIT3			8
#define RESULT_HIT4			9
#define RESULT_STUMBLE			10
#define RESULT_NEAR_FUMBLE		11
#define RESULT_NEAR_STUMBLE		12
#define RESULT_DEAD			13
#define RESULT_ANY			14
#define RESULT_WEAPON_BREAK 		15
#define RESULT_SHIELD_BREAK 		16
#define RESULT_KO			17
#define RESULT_JUST_KO			18

struct combat_msg_data {
	int				off_result;
	int				def_result;
	int				table;
	char			*def_msg;
	char			*off_msg;
	char			*other_msg;
	COMBAT_MSG_DATA	*next;
};

struct clan_data {
	char			*name;
	int				zone;
	char			*literal;
	int				member_vnum;
	int				leader_vnum;
	int				omni_vnum;
	CLAN_DATA		*next;
};

#define STATE_REJECTED		-1
#define STATE_APPLYING		0
#define STATE_SUBMITTED		1
#define STATE_APPROVED		2
#define STATE_SUSPENDED		3
#define STATE_DIED			4

#define MAP_FLEE_BACKSTAB	0

struct map_free_backstab_dt {
	int			origin;
	CHAR_DATA	*attacker;
	int			hid;
	int			backstabed;
};

struct combat_data {
	int		prog;
	union {
		struct map_free_backstab_dt backstab;
	} c;
};

struct move_data {
	int			dir;
	int			flags;
	int			desired_time;
	MOVE_DATA	*next;
        char            *travel_str;
};

struct qe_data {			/* Quarter second events data structure */
	CHAR_DATA	*ch;
	int			dir;
	int			speed_type;
	int			flags;
	ROOM_DATA	*from_room;
	int			event_time;
	int			arrive_time;
	int			move_cost;
	QE_DATA		*next;
        char            *travel_str;
};

#define MF_WALK			( 1 << 0 )
#define MF_RUN			( 1 << 1 )
#define MF_SWIM			( 1 << 2 )
#define MF_PASSDOOR		( 1 << 3 )
#define MF_ARRIVAL		( 1 << 4 )
#define MF_TOEDGE		( 1 << 5 )
#define MF_TONEXT_EDGE	( 1 << 6 )
#define MF_SNEAK		( 1 << 7 )

struct common_data {
	char		*message;
	COMMON_DATA	*next;
};

#define SPEED_WALK		0
#define SPEED_CRAWL		1
#define SPEED_PACED		2
#define SPEED_JOG		3
#define SPEED_RUN		4
#define SPEED_SPRINT	5
#define SPEED_IMMORTAL	6
#define SPEED_SWIM		7



#define FRAME_FEATHER		0
#define FRAME_SCANT			1
#define FRAME_LIGHT			2
#define FRAME_MEDIUM		3
#define FRAME_HEAVY			4
#define FRAME_MASSIVE		5
#define FRAME_SUPER_MASSIVE	6

#define SIZE_UNDEFINED		0
#define SIZE_XXS			1		/* Smaller than PC sized mobs */
#define SIZE_XS				2
#define SIZE_S				3
#define SIZE_M				4
#define SIZE_L				5
#define SIZE_XL				6
#define SIZE_XXL			7		/* Larger than PC sized mobs */

struct mob_race_data {
    char        race_name [30];
	int			male_ht_dice;
	int			male_ht_sides;
	int			male_ht_constant;
	int			male_fr_dice;
	int			male_fr_sides;
	int			male_fr_constant;
	int			female_ht_dice;
	int			female_ht_sides;
	int			female_ht_constant;
	int			female_fr_dice;
	int			female_fr_sides;
	int			female_fr_constant;
	int			can_subdue;
};

struct db_race_data {
	char		name [30];
	int			attr_mods [7];
	int 		race_age_limits [2];
	int			race_point_reqs;		/* rpp needed to play this race */
	int			race_speaks;
	int			natural_skills [MAX_NATURAL_SKILLS]; /* possible skills to start */
	int			male_ht_dice;
	int			male_ht_sides;
	int			male_ht_constant;
	int			male_fr_dice;
	int			male_fr_sides;
	int			male_fr_constant;
	int			female_ht_dice;
	int			female_ht_sides;
	int			female_ht_constant;
	int			female_fr_dice;
	int			female_fr_sides;
	int			female_fr_constant;
	int			can_subdue;
	char		race_desc[MAX_STRING_LENGTH];
	};

struct cbt {
	char		bt [100];
};

struct ibt {
	int			i [25];
};

#define CRIME_KILL			1			/* 5 hours, see criminalize() */
#define CRIME_STEAL			2			/* 3 hours */
#define CRIME_PICKLOCK		3			/* 1 hour */
#define CRIME_BACKSTAB		4			/* 5 hours */
#define CRIME_SUBDUE		5			/* 1 hour */

struct memory_t {
	int			dtype;
	int			entry;
	int			bytes;
	int			time_allocated;
};

struct random_ch_data {
    int                 *id;
    int                 random_char_count;
	int					in_room;
    CHAR_DATA           **ch_arr;
    struct random_ch_data  *next;
};

#define PHASE_CANNOT_LEAVE_ROOM ( 1 << 0 )
#define PHASE_OPEN_ON_SELF		( 1 << 1 )
#define PHASE_REQUIRE_ON_SELF   ( 1 << 2 )
#define PHASE_REQUIRE_GREATER	( 1 << 3 )

		/* These flags apply to objects within a phase */

#define SUBCRAFT_IN_ROOM		( 1 << 0 )
#define SUBCRAFT_IN_INVENTORY		( 1 << 1 )
#define SUBCRAFT_HELD			( 1 << 2 )
#define SUBCRAFT_WIELDED		( 1 << 3 )
#define SUBCRAFT_USED			( 1 << 4 )
#define SUBCRAFT_PRODUCED		( 1 << 5 )
#define SUBCRAFT_WORN			( 1 << 6 )

		/* Subcraft flags */

#define SCF_TARGET_OBJ			( 1 << 0 )	/* Target object */
#define SCF_TARGET_CHAR			( 1 << 1 )  /* Target character */
#define SCF_DEFENSIVE			( 1 << 2 )  /* Defensive - default */
#define SCF_OFFENSIVE			( 1 << 3 )  /* Offensive spell */
#define SCF_AREA				( 1 << 4 )  /* Area spell, all objs or chars */
#define SCF_ROOM				( 1 << 5 )  /* Target room */
#define SCF_TARGET_SELF			( 1 << 6 )  /* Target self */

#define SCF_CANNOT_LEAVE_ROOM		( 1 << 0 )	/* SCF = Subcraft flag */

#define MAX_ITEMS_PER_SUBCRAFT	150

struct subcraft_head_data {
	char				*craft_name;
	char				*subcraft_name;
	char				*command;
	char				*failure;		/* Failure message for subcraft */
	char				*failobjs;		/* String of failure object numbers */
	char				*help;
	char				*clans;
	PHASE_DATA			*phases;
	DEFAULT_ITEM_DATA	*items [MAX_ITEMS_PER_SUBCRAFT];
	int					subcraft_flags;
	long				crafts_start;
	long				crafts_end;
    int                 sectors[MAX_SECTOR];
    int                 seasons[MAX_SEASONS];
	int				opening[25];
    int                race[LAST_PC_RACE];
	int				failmob;		/* VNUM of mob it loads up on failure */
	int				delay;
	SUBCRAFT_HEAD_DATA	*next;
};

struct phase_data {
	char				*first;			/* First person message */
	char				*second;		/* Second person message */
	char				*third;			/* Third person message */
	char				*self;			/* If targets self, use this as first */
	char				*second_failure;/* Failure message to 2nd person */
	char				*third_failure; /* Failure message to 3rd persons */
	char				*failure;		/* Failure message for phase */
	int					flags;			/* PHase flags */
	int					phase_seconds;	/* Time in seconds of phase */
	int					skill;			/* Only used for skill checks */
	int					dice;			/* dice v skill */
	int					sides;			/* sides v skill (diceDsides v skill) */
	int					targets;		/* Spell target flags */
	int					duration;		/* Hours on spell */
	int					power;			/* Power of spell.  eg -3 to +3 str */
	int					hit_cost;		/* Hit cost of phase */
	int					move_cost;		/* Move cost of phase */
	int					spell_type;		/* Spell number */
	int					open_skill;		/* Skill to open (crafter or target) */
	int					req_skill;		/* Required skill */
	int					req_skill_value;/* Value req_skill must be > or < */
	int					attribute;      /* Used for attribute check (like skill) */
	DEFAULT_ITEM_DATA	*tool;			/* Usable tools */
	int				load_mob;		/* Mob VNUM */
	PHASE_DATA			*next;
};

#define MAX_DEFAULT_ITEMS		20

struct default_item_data {
	int					flags;			/* See SUBCRAFT_ flags */
	int					items [MAX_DEFAULT_ITEMS];/* Up to 20 default items */
	short				item_counts;    /* instances of items */
	PHASE_DATA			*phase;			/* Phase mentioned */
	char				*color;
};



struct alias_data {
    char        *command;
    char        *line;
    ALIAS_DATA  *next_line;
    ALIAS_DATA  *next_alias;
};

#define RESET_TIMED		1
#define RESET_REPLY		2

struct reset_data {
	int				type;
	char			*command;
	int				planned;	/* Gets set a minute before reset activates */
	RESET_TIME_DATA	when;
	RESET_DATA		*next;
};

struct text_data {
	char		*filename;
    char   		*name;
    char   		*text;
	TEXT_DATA	*next;
};

#define COMPOSE_TYPE_DOCUMENT	0
#define COMPOSE_TYPE_HELP		1
#define COMPOSE_TYPE_BHELP		2
#define COMPOSE_TYPE_CRAFT		3
#define COMPOSE_TYPE_TEXT		4

struct help_info {
	char			*keyword;
	char			*master_list;
	int			required_level;
	char			*info;
	HELP_INFO		*master_element;
	HELP_INFO		*next;
};

struct help_data {
	char			*keyword;
	char			*keywords;
	char			*help_info;			/* Will be null if main_element used */
	HELP_DATA		*master_element;	/* Contains actual help_info */
	HELP_DATA		*next;
};

struct name_switch_data {
	char				*old_name;
	char				*new_name;
	NAME_SWITCH_DATA	*next;
};

#define SHOP_ZONES 13
struct econ_data {
	char	flag_name [25];
	struct {
		float	markup;
		float	discount;
	} obj_econ_info [SHOP_ZONES];
};

/** Used for AI programs for mobs, objects and rooms **/
struct prog_data
{
	char				*trigger;	/* TIME, SAY, RND, KNOCK, NOD */
	char				*script;	/* script with the prog */		
	int					busy;		/* are we in the middle of a prog? */
	int					delay;		/* delay timer */
	int					vnum;		/* virtual number of the obj/mob/room */
	int					type;		/* 1-mob, 2-obj, 3-room */
	struct prog_data	*next;		/* link to the next prog */
	
};

struct trigger_data {
	int               type;   /* type of this trigger */
	int               source; /* what is this trigger set on? */
	char             *script; /* name of the script this trigger calls */
	char             *func;   /* name of the function this trigger calls */
	int	             me;     /* the mob/room/item this trigger is set on */

	TRIGGER_DATA *list;  /* pointer to the head of the list  */
	TRIGGER_DATA *next;  /* link to next trigger on the list */
};

struct larg_data {
	void *data;
	int   type;  /* one of the TS_* (trigger source) constants */
	struct larg_data *next;
};

/* Time scripts are called every 10 second */
#define PULSE_TIME_TRIGGER (10 RL_SEC)

/* Trigger types (used by trigger_data.type) */
#define TT_TIME    1
#define TT_ENTER   2
#define TT_LEAVE   3
#define TT_TELL    4
#define TT_ASK     5
#define TT_SAY     6
#define TT_GIVE    7

#define MAX_TRIGGER 1


/* Trigger type flags, used by the room/object/char that has triggers */
#define TTF_TIME   (1 << TT_TIME)
#define TTF_ENTER  (1 << TT_ENTER)
#define TTF_LEAVE  (1 << TT_LEAVE)
#define TTF_TELL   (1 << TT_TELL)
#define TTF_ASK    (1 << TT_ASK)
#define TTF_SAY    (1 << TT_SAY)
#define TTF_GIVE   (1 << TT_GIVE)

/* Trigger sources (used by trigger_data.source) */
#define TS_ROOM    0
#define TS_OBJECT  1
#define TS_MOBILE  2
#define TS_PLAYER  3
#define TS_CHAR    4
#define TS_TEXT    5

/* Location of scripts **/
#define SCRIPT_DIR	"../regions/Lscripts"
