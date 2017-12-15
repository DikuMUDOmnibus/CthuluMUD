/*
 * CthulhuMud
 */

/***************************************************************************
 *  File: olc.h                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */


/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [DT 1.1 based on Beta 1.0]"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\r\n"             \
      "     Modified for ROM 2.3 by Hans Birkeland (hansbi@ifi.uio.no)\r\n" \
      "     Modified for SunderMud by Lotherius (elfren@aros.net)\r\n"      \
      "     Modified for CthulhuMud by St.Toad (mykael@vianet.net.au)"
#define DATE	"     Original - Apr. 7, 1995\r\n" \
                "     ROM mod - Apr 16, 1995\r\n"  \
                "     CthulhuMud - Jan, 1999 onwards"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"



/*
 * New typedefs.
 */
typedef	bool OLC_FUN		args( ( CHAR_DATA *ch, char *argument ) );
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun


/* Command procedures needed ROM OLC */
DECLARE_DO_FUN(    do_help    );
DECLARE_SPELL_FUN( spell_null );


/*
 * Connected states for editor.
 */
#define ED_AREA 1
#define ED_ROOM 2
#define ED_OBJECT 3
#define ED_MOBILE 4
#define ED_HELP 5
#define ED_SOCIAL 6


/*
 * Interpreter Prototypes
 */
void    aedit           args( ( CHAR_DATA *ch, char *argument ) );
void    redit           args( ( CHAR_DATA *ch, char *argument ) );
void    medit           args( ( CHAR_DATA *ch, char *argument ) );
void    oedit           args( ( CHAR_DATA *ch, char *argument ) );
void hedit       args( ( CHAR_DATA *ch, char *argument ) );
void sedit       args( ( CHAR_DATA *ch, char *argument ) );



/*
 * OLC Constants
 */
#define MAX_MOB	1		/* Default maximum number for resetting mobs */
#define HELP_PATH                         "../html/help"


/*
 * Structure for an OLC editor command.
 */
struct olc_cmd_type
{
    char * const	name;
    OLC_FUN *		olc_fun;
};



/*
 * Structure for an OLC editor startup command.
 */
struct	editor_cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
};



/*
 * Utils.
 */
AREA_DATA *get_vnum_area	(VNUM vnum);
AREA_DATA *get_area_data	(int vnum);
void add_reset			(ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index);


/*
 * Interpreter Table Prototypes
 */
extern const struct olc_cmd_type	aedit_table[];
extern const struct olc_cmd_type	redit_table[];
extern const struct olc_cmd_type	oedit_table[];
extern const struct olc_cmd_type	medit_table[];
extern const struct olc_cmd_type hedit_table[];
extern const struct olc_cmd_type sedit_table[];


/*
 * Editor Commands.
 */
DECLARE_DO_FUN( do_aedit        );
DECLARE_DO_FUN( do_redit        );
DECLARE_DO_FUN( do_oedit        );
DECLARE_DO_FUN( do_medit        );
DECLARE_DO_FUN( do_hedit );
DECLARE_DO_FUN( do_sedit );



/*
 * General Functions
 */
bool show_types			args ( ( CHAR_DATA *ch, char *argument ) );
bool show_commands		args ( ( CHAR_DATA *ch, char *argument ) );
bool show_help			args ( ( CHAR_DATA *ch, char *argument ) );
bool edit_done			args ( ( CHAR_DATA *ch ) );
bool show_version		args ( ( CHAR_DATA *ch, char *argument ) );



/*
 * Area Editor Prototypes
 */
DECLARE_OLC_FUN( aedit_show);
DECLARE_OLC_FUN( aedit_create);
DECLARE_OLC_FUN( aedit_name);
DECLARE_OLC_FUN( aedit_copyr);
DECLARE_OLC_FUN( aedit_file);
DECLARE_OLC_FUN( aedit_flags);
DECLARE_OLC_FUN( aedit_age);
DECLARE_OLC_FUN( aedit_recall);
DECLARE_OLC_FUN( aedit_respawn);
DECLARE_OLC_FUN( aedit_morgue);
DECLARE_OLC_FUN( aedit_dream	);
DECLARE_OLC_FUN( aedit_prison);
DECLARE_OLC_FUN( aedit_mare);
DECLARE_OLC_FUN( aedit_reset);
DECLARE_OLC_FUN( aedit_security);
DECLARE_OLC_FUN( aedit_builder);
DECLARE_OLC_FUN( aedit_map);
DECLARE_OLC_FUN( aedit_vnum);
DECLARE_OLC_FUN( aedit_zone);
DECLARE_OLC_FUN( aedit_lvnum);
DECLARE_OLC_FUN( aedit_uvnum);
DECLARE_OLC_FUN( aedit_climate);
DECLARE_OLC_FUN( aedit_currency);
DECLARE_OLC_FUN( aedit_law);



/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN( redit_show		);
DECLARE_OLC_FUN( redit_used		);
DECLARE_OLC_FUN( redit_unused		);
DECLARE_OLC_FUN( redit_create		);
DECLARE_OLC_FUN( redit_name		);
DECLARE_OLC_FUN( redit_subarea		);
DECLARE_OLC_FUN( redit_desc		);
DECLARE_OLC_FUN( redit_ed		);
DECLARE_OLC_FUN( redit_flags            ); /* added elfren */
DECLARE_OLC_FUN( redit_format		);
DECLARE_OLC_FUN( redit_gadget		);
DECLARE_OLC_FUN( redit_sector           ); /* added elfren */
DECLARE_OLC_FUN( redit_north		);
DECLARE_OLC_FUN( redit_east		);
DECLARE_OLC_FUN( redit_south		);
DECLARE_OLC_FUN( redit_west		);
DECLARE_OLC_FUN( redit_northeast	);
DECLARE_OLC_FUN( redit_southeast	);
DECLARE_OLC_FUN( redit_southwest	);
DECLARE_OLC_FUN( redit_northwest	);
DECLARE_OLC_FUN( redit_up		);
DECLARE_OLC_FUN( redit_down		);
DECLARE_OLC_FUN( redit_in		);
DECLARE_OLC_FUN( redit_out		);
DECLARE_OLC_FUN( redit_mreset		);
DECLARE_OLC_FUN( redit_oreset		);
DECLARE_OLC_FUN( redit_mlist		);
DECLARE_OLC_FUN( redit_olist		);
DECLARE_OLC_FUN( redit_mshow		);
DECLARE_OLC_FUN( redit_oshow		);
DECLARE_OLC_FUN( redit_current		);
DECLARE_OLC_FUN( redit_recall		);
DECLARE_OLC_FUN( redit_respawn		);
DECLARE_OLC_FUN( redit_morgue		);
DECLARE_OLC_FUN( redit_dream		);
DECLARE_OLC_FUN( redit_mare	);
DECLARE_OLC_FUN( redit_night		);
DECLARE_OLC_FUN( redit_day		);
DECLARE_OLC_FUN( redit_affect		);
DECLARE_OLC_FUN( redit_revnum		);
DECLARE_OLC_FUN( redit_ptracks		);
DECLARE_OLC_FUN( redit_image		);
DECLARE_OLC_FUN( redit_rent		);
DECLARE_OLC_FUN( redit_heal		);
DECLARE_OLC_FUN( redit_mana		);
DECLARE_OLC_FUN( redit_delete		);



/*
 * Object Editor Prototypes
 */
DECLARE_OLC_FUN( oedit_show		);
DECLARE_OLC_FUN( oedit_repop	);
DECLARE_OLC_FUN( oedit_create		);
DECLARE_OLC_FUN( oedit_used		);
DECLARE_OLC_FUN( oedit_unused		);
DECLARE_OLC_FUN( oedit_default          );
DECLARE_OLC_FUN( oedit_name		);
DECLARE_OLC_FUN( oedit_short		);
DECLARE_OLC_FUN( oedit_wears		);
DECLARE_OLC_FUN( oedit_owears		);
DECLARE_OLC_FUN( oedit_removes		);
DECLARE_OLC_FUN( oedit_oremoves		);
DECLARE_OLC_FUN( oedit_uses		);
DECLARE_OLC_FUN( oedit_ouses		);
DECLARE_OLC_FUN( oedit_long		);
DECLARE_OLC_FUN( oedit_addapply		);
DECLARE_OLC_FUN( oedit_addskill		);
DECLARE_OLC_FUN( oedit_addeffect	);
DECLARE_OLC_FUN( oedit_addaffect	);
DECLARE_OLC_FUN( oedit_addmax);
DECLARE_OLC_FUN( oedit_delaffect	);
DECLARE_OLC_FUN( oedit_value0		);
DECLARE_OLC_FUN( oedit_value1		);
DECLARE_OLC_FUN( oedit_value2		);
DECLARE_OLC_FUN( oedit_value3		);
DECLARE_OLC_FUN( oedit_value4		);
DECLARE_OLC_FUN( oedit_weight		);
DECLARE_OLC_FUN( oedit_cost		);
DECLARE_OLC_FUN( oedit_ed		);
DECLARE_OLC_FUN( oedit_anchor	);  
DECLARE_OLC_FUN( oedit_artifact	);  
DECLARE_OLC_FUN( oedit_delete	);  

DECLARE_OLC_FUN( oedit_extra            ); 
DECLARE_OLC_FUN( oedit_wear             );
DECLARE_OLC_FUN( oedit_type             );
DECLARE_OLC_FUN( oedit_affect           );
DECLARE_OLC_FUN( oedit_material);  
DECLARE_OLC_FUN( oedit_level            );
DECLARE_OLC_FUN( oedit_condition        );

DECLARE_OLC_FUN( oedit_zone             );  
DECLARE_OLC_FUN( oedit_revnum           );
DECLARE_OLC_FUN( oedit_image            );  


/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN( medit_show		);
DECLARE_OLC_FUN( medit_create	);
DECLARE_OLC_FUN( medit_default 	);
DECLARE_OLC_FUN( medit_used		);
DECLARE_OLC_FUN( medit_unused	);
DECLARE_OLC_FUN( medit_name		);
DECLARE_OLC_FUN( medit_short		);
DECLARE_OLC_FUN( medit_long		);
DECLARE_OLC_FUN( medit_shop		);
DECLARE_OLC_FUN( medit_bio		);
DECLARE_OLC_FUN( medit_desc		);
DECLARE_OLC_FUN( medit_level		);
DECLARE_OLC_FUN( medit_align		);
DECLARE_OLC_FUN( medit_spec		);
DECLARE_OLC_FUN( medit_delete	);
DECLARE_OLC_FUN( medit_sex		);  
DECLARE_OLC_FUN( medit_act		);  
DECLARE_OLC_FUN( medit_affect	);  
DECLARE_OLC_FUN( medit_ac		);  
DECLARE_OLC_FUN( medit_form		);  
DECLARE_OLC_FUN( medit_part		);  
DECLARE_OLC_FUN( medit_imm		);  
DECLARE_OLC_FUN( medit_envimm	);  
DECLARE_OLC_FUN( medit_res		);  
DECLARE_OLC_FUN( medit_vuln		);  
DECLARE_OLC_FUN( medit_material	);  
DECLARE_OLC_FUN( medit_off		);  
DECLARE_OLC_FUN( medit_size		);  
DECLARE_OLC_FUN( medit_hitdice	);  
DECLARE_OLC_FUN( medit_manadice	);  
DECLARE_OLC_FUN( medit_damdice	);  
DECLARE_OLC_FUN( medit_race		);  
DECLARE_OLC_FUN( medit_language	);  
DECLARE_OLC_FUN( medit_position	);  
DECLARE_OLC_FUN( medit_gold		);  
DECLARE_OLC_FUN( medit_fright		);
DECLARE_OLC_FUN( medit_hitroll	);  
DECLARE_OLC_FUN( medit_attack	); 
DECLARE_OLC_FUN( medit_damtype	); 
DECLARE_OLC_FUN( medit_skills		);  
DECLARE_OLC_FUN( medit_setprof		);  
DECLARE_OLC_FUN( medit_conv		);  
DECLARE_OLC_FUN( medit_script		);  
DECLARE_OLC_FUN( medit_macro);  
DECLARE_OLC_FUN( medit_events		);  
DECLARE_OLC_FUN( medit_trigger		);  
DECLARE_OLC_FUN( medit_nature		);  
DECLARE_OLC_FUN( medit_timesub		);  
DECLARE_OLC_FUN( medit_monitor		);  
DECLARE_OLC_FUN( medit_revnum		);  
DECLARE_OLC_FUN( medit_chat		);  
DECLARE_OLC_FUN( medit_vision		); 
DECLARE_OLC_FUN( medit_image		);  
DECLARE_OLC_FUN( medit_society);  


/*
 * Macros
 */

#define IS_SWITCHED( ch )       ( ch->desc->original )    /* ROM OLC */

#define IS_BUILDER(ch, Area)	( ( ch->pcdata->security >= Area->security  \
				|| strstr( Area->builders, ch->name )	    \
				|| strstr( Area->builders, "All" ) )	    \
				&& !IS_SWITCHED( ch ) )

#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

/* Return pointers to what is being edited. */
#define EDIT_MOB(Ch, Mob)	( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)	( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)	( Room = Ch->in_room )
#define EDIT_AREA(Ch, Area)	( Area = (AREA_DATA *)Ch->desc->pEdit )

typedef struct social_type SOCIAL_DATA; 

/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
#define ED	EXTRA_DESCR_DATA
RESET_DATA	*new_reset_data		args ( ( void ) );
void		free_reset_data		args ( ( RESET_DATA *pReset ) );
AREA_DATA	*new_area		args ( ( void ) );
void		free_area		args ( ( AREA_DATA *pArea ) );
EXIT_DATA	*new_exit		args ( ( void ) );
void		free_exit		args ( ( EXIT_DATA *pExit ) );
ED 		*new_extra_descr	args ( ( void ) );
void		free_extra_descr	args ( ( ED *pExtra ) );
ROOM_INDEX_DATA *new_room_index		args ( ( void ) );
void		free_room_index		args ( ( ROOM_INDEX_DATA *pRoom ) );
AFFECT_DATA	*new_affect		args ( ( void ) );
void		free_affect		args ( ( AFFECT_DATA* pAf ) );
SHOP_DATA	*new_shop		args ( ( void ) );
void		free_shop		args ( ( SHOP_DATA *pShop ) );
OBJ_INDEX_DATA	*new_obj_index		args ( ( void ) );
void		free_obj_index		args ( ( OBJ_INDEX_DATA *pObj ) );
MOB_INDEX_DATA	*new_mob_index		args ( ( void ) );
void		free_mob_index		args ( ( MOB_INDEX_DATA *pMob ) );
HELP_DATA    *new_help       args ( (void) );
void free_help args ( ( HELP_DATA *pHelp ) ); 
extern HELP_DATA     *   help_last;

#undef ED 

