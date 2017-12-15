/*********************************************************
 *                   CTHULHUMUD                       	 *
 * CthulhuMud  driver  version 8.x copyright (C) 2000 	 *
 * by Mik Clarke (mykael@vianet.net.au)                  *
 * and Joachim Häusler (mystery@chello.at).              *
 *                                                       *
 * While the code is original, many concepts and the     *
 * typical look & feel are derived from MERC and its     *
 * derivatives - especially the SunderMud 1.0 prototype. *
 *                                                       *
 * Therefore we'd like to thank:                         *
 * Lotherius                                             *
 * Russ Taylor, Gabrielle Taylor, Brian Moore            *
 * Michael Chastain, Michael Quan, and Mitchell Tse      *
 *                                                       *
 * Please keep this code open-source and share your      *
 * ideas. It's still a long way to go.                   *
 *********************************************************/

 
#include "everything.h"
#include "spell.h"
#include "magic.h"
#include "skill.h"
#include "affect.h"
#include "wev.h"
#include "econd.h"
#include "fight.h"
#include "music.h"

#define SLOT(n)	n

/* Spell table... */

SPELL_INFO spell_array[MAX_SPELL]; 

/* Effect table... */

EFFECT_INFO effect_array[MAX_SPELL] = { 

  { "none", 
       	spell_null,
                SLOT(0),
	TAR_IGNORE, 
	"", "" },

  { "armor", 
	spell_armor,
	SLOT(1),
	TAR_CHAR_DEFENSIVE,
        "", "armor" },

  { "teleport", 
	spell_teleport,
 	SLOT(2),
	TAR_CHAR_DEFENSIVE,
        "", "" },

  { "bless", 
	spell_bless,
                SLOT(3),
	TAR_CHAR_DEFENSIVE,
	"", "bless" },

  { "blindness", 
	spell_blindness,
 	SLOT(4),
	TAR_CHAR_OFFENSIVE,
        "", "blindness" },

  { "burning hands",
	spell_burning_hands,
	SLOT(5),
	TAR_CHAR_OFFENSIVE,
	"burning hands", "" },

  { "call lightning",
	spell_call_lightning,
	SLOT(6),
	TAR_IGNORE,
	"lightning bolt", "" },

  { "charm person",
	spell_charm_person,
	SLOT(7),
	TAR_CHAR_OFFENSIVE,
	"", "charm" },

  { "chill touch",
	spell_chill_touch,
	SLOT(8),
	TAR_CHAR_OFFENSIVE,
	"chilling touch", "chill touch" },

  { "colour spray",
	spell_colour_spray,
	SLOT(10),
	TAR_CHAR_OFFENSIVE,
	"colour spray", "" },

  { "control weather",
	spell_control_weather,
	SLOT(11),
	TAR_IGNORE,
	"", "" },

  { "create food",
	spell_create_food,
	SLOT(12),
	TAR_IGNORE,
	"", "" },

  { "create water",
	spell_create_water,
	SLOT(13),
	TAR_OBJ_INV,
	"", "" },

  { "cure blindness",
	spell_cure_blindness,
	SLOT(14),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "cure critical",
	spell_cure_critical,
	SLOT(15),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "cure light",
	spell_cure_light,
	SLOT(16),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "curse",
	spell_curse,
	SLOT(17),
	TAR_CHAR_OFFENSIVE,
	"curse", "curse" },

  { "detect evil",
	spell_detect_evil,
	SLOT(18),
	TAR_CHAR_SELF,
	"", "detect evil" },

  { "detect invis",
	spell_detect_invis,
	SLOT(19),
	TAR_CHAR_SELF,
	"", "detect invis" },

  { "detect magic",
	spell_detect_magic,
	SLOT(20),
	TAR_CHAR_SELF,
	"", "detect magic" },

  { "detect poison",
	spell_detect_poison,
	SLOT(21),
	TAR_OBJ_INV,
	"", "" },

  { "dispel evil",
	spell_dispel_evil,
	SLOT(22),
	TAR_CHAR_OFFENSIVE,
	"dispel evil", "" },

  { "earthquake",
	spell_earthquake,
	SLOT(23),
	TAR_IGNORE,
	"earthquake", "" },

  { "enchant weapon",
	spell_enchant_weapon,
	SLOT(24),
	TAR_OBJ_INV,
	"", "enchant weapon" },

  { "energy drain",
	spell_energy_drain,
	SLOT(25),
	TAR_CHAR_OFFENSIVE,
	"energy drain", "" },

  { "fireball",
	spell_fireball,
	SLOT(26),
	TAR_CHAR_OFFENSIVE,
	"fireball", "" },

  { "harm",
	spell_harm,
	SLOT(27),
	TAR_CHAR_OFFENSIVE,
	"harm spell", "" },
  
  { "heal",
	spell_heal,
	SLOT(28),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "invis",
	spell_invis,
	SLOT(29),
	TAR_CHAR_DEFENSIVE,
	"", "invisability" },

  { "lightning bolt",
 	spell_lightning_bolt,
	SLOT(30),
	TAR_CHAR_OFFENSIVE,
	"lightning bolt", "" },

  { "locate object",
	spell_locate_object,
	SLOT(31),
	TAR_IGNORE,
	"", "" },

  { "magic missile",
	spell_magic_missile,
	SLOT(32),
	TAR_CHAR_OFFENSIVE,
	"magic missile", "" },

  { "poison",
	spell_poison,
	SLOT(33),
	TAR_CHAR_OFFENSIVE,
	"poison", "" },

  { "protection evil",
	spell_protection_evil,
	SLOT(34),
	TAR_CHAR_SELF,
	"", "protection evil" },
    
  { "remove curse",
	spell_remove_curse,
	SLOT(35),
	TAR_CHAR_DEFENSIVE,
	"", "" },
    
  { "sanctuary",
    	spell_sanctuary,
	SLOT(36),
	TAR_CHAR_DEFENSIVE,
	"", "sanctuary" },

  { "restore limb",
	spell_restore_limb,
	SLOT(37),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "sleep",
	spell_sleep,
	SLOT(38),
	TAR_CHAR_OFFENSIVE,
	"", "sleep" },

  { "giant strength",
	spell_giant_strength,
	SLOT(39),
	TAR_CHAR_DEFENSIVE,
	"", "giant strength" },

  { "summon",
	spell_summon,
	SLOT(40),
	TAR_CHAR_ANYWHERE,
	"", "" },

  { "ventriloquate",
	spell_ventriloquate,
	SLOT(41),
	TAR_IGNORE,
	"", "" },

  { "word of recall",
	spell_word_of_recall,
	SLOT(42),
	TAR_CHAR_SELF,
	"", "" },

  { "cure poison",
	spell_cure_poison,
	SLOT(43),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "detect hidden",
	spell_detect_hidden,
	SLOT(44),
	TAR_CHAR_SELF,
	"", "detect hidden" },

  { "aura",
	spell_aura,
	SLOT(45),
	TAR_CHAR_SELF,
	"", "aura" },

  { "hallucinate",
	spell_hallucinate,
	SLOT(46),
	TAR_CHAR_DEFENSIVE,
	"hallucinate", "" },

  { "protect fire",
	spell_protection_fire,
	SLOT(47),
	TAR_CHAR_SELF,
	"", "protect fire" },

  { "protect frost",
	spell_protection_frost,
	SLOT(48),
	TAR_CHAR_SELF,
	"", "protect frost" },

  { "protect lightning",
	spell_protection_lightning,
	SLOT(49),
	TAR_CHAR_SELF,
	"", "protect lightning" },

  { "cause riot",
	spell_cause_riot,
	SLOT(50),
	TAR_IGNORE,
	"cause riot", "" },

  { "slow",
	spell_slow,
	SLOT(51),
	TAR_CHAR_OFFENSIVE,
	"", "slow" },

  { "identify",
	spell_identify,
	SLOT(52),
	TAR_OBJ_INV,
	"", "" },

  { "shocking grasp",
	spell_shocking_grasp,
	SLOT(53),
	TAR_CHAR_OFFENSIVE,
	"shocking grasp", "" },

  { "globe of protection",
	spell_globe_of_protection,
	SLOT(54),
	TAR_CHAR_SELF,
	"", "globe of protection" },

  { "personalize weapon",
	spell_personalize_weapon,
	SLOT(55),
	TAR_OBJ_INV,
	"", "personalize weapon" },

  { "fly",
	spell_fly,
	SLOT(56),
	TAR_CHAR_DEFENSIVE,
	"", "fly" },

  { "continual light",
	spell_continual_light,
	SLOT(57),
	TAR_IGNORE,
	"", "" },

  { "know alignment",
	spell_know_alignment,
	SLOT(58),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "dispel magic",
	spell_dispel_magic,
	SLOT(59),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "animate weapon",
	spell_animate_weapon,
	SLOT(60),
	TAR_OBJ_INV,
	"", "animate weapon" },
 
  { "cure serious",
	spell_cure_serious,
	SLOT(61),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "cause light",
	spell_cause_light,
	SLOT(62),
	TAR_CHAR_OFFENSIVE,
	"spell", "" },

  { "cause critical",
	spell_cause_critical,
	SLOT(63),
	TAR_CHAR_OFFENSIVE,
	"spell", "" },

  { "cause serious",
	spell_cause_serious,
	SLOT(64),
	TAR_CHAR_OFFENSIVE,
	"spell", "" },

  { "flamestrike",
	spell_flamestrike,
	SLOT(65),
	TAR_CHAR_OFFENSIVE,
	"flamestrike", "" },

  { "hard skin",
	spell_hard_skin,
	SLOT(66),
	TAR_CHAR_SELF,
	"", "hard skin" },

  { "shield",
	spell_shield,
	SLOT(67),
	TAR_CHAR_DEFENSIVE,
	"", "shield" },

  { "weaken",
	spell_weaken,
	SLOT(68),
	TAR_CHAR_OFFENSIVE,
	"spell", "weakness" },

  { "mass invis",
	spell_mass_invis,
	SLOT(69),
	TAR_IGNORE,
	"", "invisability" },

  { "acid blast",
	spell_acid_blast,
	SLOT(70),
	TAR_CHAR_OFFENSIVE,
	"acid blast", "" },

  { "faerie fire",
	spell_faerie_fire,
	SLOT(72),
	TAR_CHAR_OFFENSIVE,
	"", "faerie fire" },

  { "faerie fog", 
	spell_faerie_fog,
	SLOT(73),
	TAR_IGNORE,
	"", "" },

  { "pass door",
	spell_pass_door,
	SLOT(74),
	TAR_CHAR_SELF,
	"", "pass door" },

  { "infravision",
	spell_infravision,
	SLOT(77),
	TAR_CHAR_DEFENSIVE,
	"", "infravision" },

  { "cause death",
	spell_cause_death,
	SLOT(78),
	TAR_CHAR_OFFENSIVE,
	"cause death", "" },

  { "create spring",
	spell_create_spring,
	SLOT(80),
	TAR_IGNORE,
	"", "" },

  { "refresh",
	spell_refresh,
	SLOT(81),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "change sex",
	spell_change_sex,
	SLOT(82),
	TAR_CHAR_DEFENSIVE,
	"", "sex change" },

  { "gate",
	spell_gate,
	SLOT(83),
	TAR_IGNORE,
	"", "" },

 { "lesser possession",
	spell_lesser_possession,
	SLOT(84),
	TAR_CHAR_DEFENSIVE,
	"", "lesser possession" },

  { "transformation",
	spell_transformation,
	SLOT(85),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "fatigue",
	spell_fatigue,
	SLOT(86),
	TAR_CHAR_OFFENSIVE,
	"fatigue spell", "" },

  { "mana transfer",
	spell_mana_transfer,
	SLOT(87),
	TAR_CHAR_DEFENSIVE,
	"mana transfer", "" },

  { "consecrate doll",
	spell_consecrate_doll,
	SLOT(88),
	TAR_CHAR_ANYWHERE,
	"", "consecrate doll" },

  { "relax", 
	spell_relax,
	SLOT(89),
	TAR_CHAR_DEFENSIVE,
	"", "relax" },

 { "greater possession",
	spell_greater_possession,
	SLOT(90),
	TAR_CHAR_DEFENSIVE,
	"", "greater possession" },

  { "acid breath",
	spell_acid_breath,
	SLOT(200),
	TAR_CHAR_OFFENSIVE,
	"blast of acid", "" },

  { "fire breath",
	spell_fire_breath,
	SLOT(201),
	TAR_CHAR_OFFENSIVE,
	"blast of flame", "" },

  { "frost breath",
	spell_frost_breath,
	SLOT(202),
	TAR_CHAR_OFFENSIVE,
	"blast of frost", "" },

  { "gas breath",
	spell_gas_breath,
	SLOT(203),
	TAR_IGNORE,
	"blast of gas", "" },

  { "lightning breath",
	spell_lightning_breath,
	SLOT(204),
	TAR_CHAR_OFFENSIVE,
	"blast of lightning", "" },

  { "chaos",
	spell_chaos,
 	SLOT(205),
	TAR_CHAR_OFFENSIVE,
	"chaos", "" },

  { "create buffet",
        spell_create_buffet,
	SLOT(230),
	TAR_IGNORE,
        "", "" },

  { "general purpose",
        spell_general_purpose,
	SLOT(401), 
	TAR_CHAR_OFFENSIVE,
        "general purpose ammo", "" },
 
  { "high explosive",
        spell_high_explosive,
	SLOT(402),
	TAR_CHAR_OFFENSIVE,
        "high explosive ammo", "" }, 

  { "mind meld",
        spell_mind_meld,
	SLOT(403),
	TAR_CHAR_OFFENSIVE,
        "mental blast", "mind meld" },

  { "animate dead",
        spell_animate,
	SLOT(404),
	TAR_IGNORE,
        "", "" },

  { "soul blade",
        spell_soul_blade,
	SLOT(405),
	TAR_IGNORE,
        "", "" },

  { "minor creation",
	spell_minor_creation,
	SLOT(406),
	TAR_IGNORE,
	"", "" },

  { "psi twister",
	spell_psi_twister,
	SLOT(407),
	TAR_IGNORE,
	"psi twister", "" },

  { "exorcism",
	spell_exorcism,
	SLOT(408),
	TAR_CHAR_OFFENSIVE,
	"angels", "" },

  { "youth",
	spell_youth,
 	SLOT(409),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "age",
	spell_age,
	SLOT(410),
	TAR_CHAR_OFFENSIVE,
        "", "" },

  { "summon familier",
        spell_summon_familier,
	SLOT(411),
	TAR_IGNORE,
        "", "" },

  { "summon spirit",
        spell_summon_spirit,
	SLOT(412),
	TAR_IGNORE,
        "", "" },

 { "elder watcher",
        spell_elder_watcher,
	SLOT(413),
	TAR_IGNORE,
        "", "" },

{ "greater creation",
	spell_greater_creation,
	SLOT(414),
	TAR_IGNORE,
	"", "" },

  { "mummify",
        spell_mummify,
	SLOT(415),
	TAR_IGNORE,
        "", "" },

  { "lich",
        spell_lich,
	SLOT(416),
	TAR_CHAR_SELF,
        "", "" },

  { "wrath of cthuga",
	spell_wrath_cthuga,
	SLOT(417),
	TAR_IGNORE,
	"", "" },

  { "chain lightning",
	spell_chain_lightning,
	SLOT(500),
	TAR_CHAR_OFFENSIVE,
	"lightning", "" }, 

  { "cure disease",
	spell_cure_disease,
	SLOT(501),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "haste",
	spell_haste,
	SLOT(502),
	TAR_CHAR_DEFENSIVE,
	"", "haste" },

  { "plague",
	spell_plague,
	SLOT(503),
	TAR_CHAR_OFFENSIVE,
	"sickness", "plague" },

  { "frenzy",
        spell_frenzy,
	SLOT(504),
	TAR_CHAR_DEFENSIVE,
        "", "frenzy" },

  { "demonfire",
	spell_demonfire,
	SLOT(505),
	TAR_CHAR_OFFENSIVE,
	"torments", "" },

  { "holy word",
	spell_holy_word,
	SLOT(506), 
	TAR_IGNORE,
	"divine wrath", "" },

  { "cancellation",
	spell_cancellation,
	SLOT(507),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "mass healing",
	spell_mass_healing,
	SLOT(508),
	TAR_IGNORE,
	"", "" },

  { "calm",
	spell_calm,
	SLOT(509),
	TAR_IGNORE,
	"", "calm" }, 

  { "enchant armor",
	spell_enchant_armor,
	SLOT(510),
	TAR_OBJ_INV,
	"", "enchant armor" },

  { "brand",
	spell_brand,
	SLOT(511),
	TAR_OBJ_INV,
	"", "" }, 

  { "negate alignment",
	spell_negate_alignment,
	SLOT(512),
	TAR_OBJ_INV,
	"", "" },

  { "mask self",
	spell_mask_self, 
	SLOT(513),
	TAR_CHAR_DEFENSIVE,
	"", "mask self" },

  { "absorb magic",
	spell_absorb_magic,
	SLOT(514),
	TAR_CHAR_SELF,
	"", "absorb magic" }, 

  { "psychic anchor",
	spell_psychic_anchor,
	SLOT(515),
	TAR_IGNORE,
	"", "" },
	
  { "fear",
	spell_fear,
	SLOT(516),
	TAR_CHAR_DEFENSIVE,
	"", "fear" },

  { "protection good",
	spell_protection_good,
	SLOT(517),
	TAR_CHAR_SELF,
	"", "protection good" },

  { "detect good",
	spell_detect_good,
	SLOT(518),
	TAR_CHAR_SELF,
	"", "detect good" },

  { "dispel good",
	spell_dispel_good,
	SLOT(519),
	TAR_CHAR_OFFENSIVE,
	"dispel good", "" },

  { "regeneration",
	spell_regeneration,
	SLOT(520),
	TAR_CHAR_DEFENSIVE,
	"", "regeneration" },

  { "fire shield",
	spell_fire_shield,
	SLOT(521),
	TAR_CHAR_SELF,
	"",  "fire shield" },

  { "portal",
	spell_portal,
	SLOT(522),
	TAR_IGNORE,
	"", "" },

  { "remove invis",
	spell_remove_invis,
	SLOT(523),
	TAR_OBJ_INV,
	"", "" },

  { "vocalize",
	spell_vocalize,
	SLOT(524),
	TAR_CHAR_DEFENSIVE,
	"", "vocalize" },

  { "entropy",
        spell_entropy,
	SLOT(525),
	TAR_CHAR_OFFENSIVE,
	"entropy", "" },

  { "remove fear",
	spell_remove_fear,
	SLOT(526),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "mute",
	spell_mute,
  	SLOT(527),
	TAR_CHAR_OFFENSIVE,
	"", "mute" },

  { "silence",
	spell_silence,
  	SLOT(528),
	TAR_IGNORE,
	"", "silence" },

  { "darkness",
	spell_darkness,
  	SLOT(529),
	TAR_IGNORE,
	"", "darkness" },

  { "frost shield",
	spell_frost_shield,
	SLOT(530),
	TAR_CHAR_SELF,
	"",  "frost shield" },

  { "lightning shield",
	spell_lightning_shield,
	SLOT(531),
	TAR_CHAR_SELF,
	"",  "lightning shield" },

  { "personalize portal",
	spell_personalize_portal,
	SLOT(532),
	TAR_OBJ_ROOM,
	"", "personalize portal" },

  { "pestillence",
	spell_pestillence,
	SLOT(533),
	TAR_IGNORE,
	"pestillence", "" },

  { "destroy portal",
	spell_destroy_portal,
	SLOT(534),
	TAR_OBJ_ROOM,
	"", "destroy portal" },

  { "bestow blessing",
	spell_bestow_blessing,
	SLOT(536),
	TAR_OBJ_ROOM,
	"", "bestow blessing" },

  { "create potion",
        spell_create_potion,
	SLOT(600),
	TAR_CHAR_SELF,
        "", "" }, 

  { "water breathing",
        spell_water_breathing,
	SLOT(601),
	TAR_CHAR_DEFENSIVE,
        "", "water breathing" }, 

  { "universality",
        spell_universality,
	SLOT(602),
	TAR_OBJ_INV,
        "", "" }, 

  { "true dreaming",
        spell_true_dreaming,
	SLOT(603),
	TAR_CHAR_DEFENSIVE,
        "", "" }, 

  { "rude awakening",
        spell_rude_awakening,
	SLOT(604),
	TAR_CHAR_OFFENSIVE,
        "", "" }, 

  { "recurring dream",
        spell_recurring_dream,
	SLOT(605),
	TAR_CHAR_DEFENSIVE,
        "", "" }, 

  { "enchanted sleep",
        spell_enchanted_sleep,
	SLOT(606),
	TAR_CHAR_OFFENSIVE,
        "", "" }, 

  { "gnawing hunger",
        spell_gnawing_hunger,
	SLOT(607),
	TAR_CHAR_OFFENSIVE,
        "", "" }, 

  { "burning thirst",
        spell_burning_thirst,
	SLOT(608),
	TAR_CHAR_OFFENSIVE,
        "", "" }, 

  { "free grog",
        spell_free_grog,
	SLOT(609),	
	TAR_CHAR_OFFENSIVE,
        "", "" }, 

  { "ghastly sobriety",
        spell_ghastly_sobriety,
	SLOT(610),
	TAR_CHAR_DEFENSIVE,
        "", "" }, 

  { "burden of blubber",
        spell_burden_of_blubber,
	SLOT(611),
	TAR_CHAR_OFFENSIVE,
        "", "" }, 

  { "slender lines",
        spell_slender_lines,
	SLOT(611),	
	TAR_CHAR_DEFENSIVE,
        "", "" }, 

  { "mage light",
	spell_mage_light,
	SLOT(612),
	TAR_IGNORE,
	"", "" },

  { "astral blast",
	spell_astral_blast,
	SLOT(613),
	TAR_CHAR_OFFENSIVE,
	"", "astral blast" },

  { "permanence",
        spell_permanence,
	SLOT(614),
	TAR_OBJ_INV,
        "", "" }, 

  { "consistence",
        spell_consistence,
	SLOT(615),
	TAR_OBJ_INV,
        "", "" }, 

  { "trance",
        spell_trance,
	SLOT(616),
	TAR_CHAR_SELF,
        "trance", "" }, 

  { "metamorphosis",
	spell_metamorphosis,
	SLOT(617),
	TAR_CHAR_SELF,
	"", "metamorphosis" },

  { "mortalize",
	spell_mortalize,
	SLOT(618),
	TAR_CHAR_DEFENSIVE,
	"", "mortalize" },

  { "incognito",
	spell_incognito, 
	SLOT(619),
	TAR_CHAR_ANYWHERE,
	"", "incognito" },

  { "create seed",
	spell_create_seed,
	SLOT(620),
	TAR_IGNORE,
        "", "create seed" },

  { "call pet",
        spell_call_pet,
        SLOT(621),
	TAR_IGNORE,
        "", "call pet" },

  { "curse of the mummy",
	spell_curse_mummy,
	SLOT(622),
	TAR_CHAR_ANYWHERE,
	"", "curse of the mummy" },

  { "enclosure",
	spell_enclosure,
  	SLOT(623),
	TAR_IGNORE,
	"", "enclosure" },

  { "recharge",
	spell_recharge,
	SLOT(624),
	TAR_OBJ_INV,
	"", "recharge" },

  { "create figurine",
        spell_create_figurine,
	SLOT(625),	
	TAR_CHAR_DEFENSIVE,
        "", "" }, 

  { "elder shield",
        spell_elder_shield,
	SLOT(626),	
	TAR_CHAR_DEFENSIVE,
        "", "" }, 

  { "telekinesis",
	spell_telekinesis,
	SLOT(627),
	TAR_IGNORE,
	"telekinesis", "" },

  { "magical duel",
	spell_duel,
	SLOT(628),
	TAR_CHAR_DEFENSIVE,
	"", "" },

  { "heatstrike",
	spell_heat_weapon,
	SLOT(629),
	TAR_CHAR_OFFENSIVE,
	"", "" },

  { "agony",
	spell_agony,
	SLOT(630),
	TAR_CHAR_ANYWHERE,
	"", "word of pain" },

  { "true sight",
	spell_true_sight,
	SLOT(631),
	TAR_CHAR_DEFENSIVE,
	"", "true sight" },

  { "drain vitality",
	spell_drain_vitality,
	SLOT(632),
	TAR_OBJ_ROOM,
	"", "drain vitality" },

  { "confuse hunters",
        spell_confuse_hunters,
        SLOT(633),
        TAR_IGNORE,
        "", "confuse hunters" },

  { "farsight",
        spell_farsight,
        SLOT(634),
        TAR_CHAR_DEFENSIVE,
        "", "farsight" },

  { "change size",
        spell_change_size,
        SLOT(635),
        TAR_CHAR_SELF,
        "", "change size" },

  { "astral walk", 
	spell_astral_walk,
	SLOT(636),
	TAR_IGNORE,
	"", "astral walk" },

  { "oracle", 
	spell_oracle,
	SLOT(637),
	TAR_IGNORE,
	"", "oracle" },

  { "doppelganger", 
	spell_doppelganger,
	SLOT(638),
	TAR_IGNORE,
	"", "doppelganger" },

  { "blade affect",
	spell_blade_affect,
	SLOT(639),
	TAR_OBJ_INV,
	"", "blade_affect" },

  { "store mana",
	spell_store_mana,
	SLOT(640),
	TAR_IGNORE,
	"", "" },

  { "release mana",
	spell_release_mana,
	SLOT(641),
	TAR_IGNORE,
	"", "" },

  { "asceticism",
	spell_asceticism,
	SLOT(642),
	TAR_CHAR_SELF,
	"", "" },

  { "antipsychotica",
	spell_antipsychotica,
	SLOT(643),
	TAR_CHAR_SELF,
	"", "" },

  { NULL,
	spell_null,
	SLOT(0),
	TAR_IGNORE,
	"", "" }
                  
};

/* Find the spn of a given spell... */

int get_spell_spn(char *name) {

  int spn;

  for(spn = 0; spn < MAX_SPELL; spn++) {

    if ( spell_array[spn].name == NULL ) {
      return SPELL_UNDEFINED;
    }
 
    if ( name[0] == spell_array[spn].name[0] 
      && !strcmp(name, spell_array[spn].name) ) {
      return spn;
    }
  }

  return SPELL_UNDEFINED;
}

/* Find the epn of a given effect... */

int get_effect_efn(char *name) {

  int efn;

  for(efn = 0; efn < MAX_EFFECT; efn++) {

    if ( effect_array[efn].name == NULL ) {
      return EFFECT_UNDEFINED;
    }
 
    if ( name[0] == effect_array[efn].name[0] 
      && !strcmp(name, effect_array[efn].name) ) {
      return efn;
    }
  }

  return EFFECT_UNDEFINED;
}

/* Find the gspn for a spell and complain if it can't be found... */

int get_spell_gspn(char *name) {

  int gspn;
  char buf[MAX_STRING_LENGTH];

  gspn = get_spell_spn(name);

  if (gspn == SPELL_UNDEFINED) {
    sprintf(buf, "Spell '%s' undefined, can't set gspn.", name);
    bug(buf, 0);
    exit(1);
  } 

  return gspn;
}

/* Find the gefn for a spell and complain if it can't be found... */

int get_effect_gefn(char *name) {

  int gefn;
  char buf[MAX_STRING_LENGTH];

  gefn = get_effect_efn(name);

  if (gefn == EFFECT_UNDEFINED) {
    sprintf(buf, "Effect '%s' undefined, can't set gefn.", name);
    bug(buf, 0);
    exit(1);
  } 

  return gefn;
}

/* See is a spn is valid... */

bool valid_spell(int spn) {

  if ( spn < 0
    || spn >= MAX_SPELL ) {
    return FALSE;
  }

  if (spell_array[spn].loaded != TRUE) {
    return FALSE;
  }

  return TRUE;
}

/* See is an efn is valid... */

bool valid_effect(int efn) {

  if ( efn < 0
    || efn >= MAX_EFFECT ) {
    return FALSE;
  }

  if (effect_array[efn].spell_fun == spell_null) {
    return FALSE;
  }

  return TRUE;
}

/* Allocate a spell instruction... */

/* Free chain... */

static SEQ_STEP *free_seq_chain = NULL;

/* Allocate a new sequence step... */

SEQ_STEP *new_seq_step() {

  SEQ_STEP *new_step;

  if (free_seq_chain != NULL) {
    new_step = free_seq_chain;
    free_seq_chain = free_seq_chain->next;
  } else {  
    new_step = (SEQ_STEP *) alloc_perm(sizeof(*new_step));
  }

  new_step->delay 	= 0;
  new_step->instr	= STEP_NONE;
  new_step->iparms[0]	= 0;
  new_step->iparms[1]	= 0;
  new_step->iparms[2]	= 0;
  new_step->iparms[3]	= 0;
  new_step->iparms[4]	= 0;
  new_step->sparms[0]	= NULL;
  new_step->sparms[1]	= NULL;
  new_step->sparms[2]	= NULL;
  new_step->sparms[3]	= NULL;
  new_step->cond	= NULL;
  new_step->next	= NULL;

  return new_step;
}

/* Return an unused sequence step... */

void free_seq_step(SEQ_STEP *old_step) {

  old_step->delay 	= 0;
  old_step->instr	= STEP_NONE;
  old_step->iparms[0]	= 0;
  old_step->iparms[1]	= 0;
  old_step->iparms[2]	= 0;

  if (old_step->sparms[0] != NULL) {
    free_string(old_step->sparms[0]);
    old_step->sparms[0]	= NULL;
  }

  if (old_step->sparms[1] != NULL) {
    free_string(old_step->sparms[1]);
    old_step->sparms[1]	= NULL;
  }

  if (old_step->sparms[2] != NULL) {
    free_string(old_step->sparms[2]);
    old_step->sparms[2]	= NULL;
  }

  old_step->next	= free_seq_chain;

  free_seq_chain = old_step;

  return;
}

/* Generate a default instruction sequence for a spell... */

SEQ_STEP *generate_default_seq(int spn, char *chant, int efn) {

  SEQ_STEP *new_seq, *base_seq;

  base_seq = new_seq_step();

  new_seq = base_seq;

  new_seq->delay	= 1;
  new_seq->instr	= STEP_PREPARE_SPELL;

  new_seq->next		= new_seq_step(); 
  new_seq		= new_seq->next;

  new_seq->delay	= 1;
  new_seq->instr	= STEP_CHANT;
  new_seq->sparms[0]	= chant;

  new_seq->next		= new_seq_step(); 
  new_seq		= new_seq->next;

  new_seq->delay	= 1;
  new_seq->instr	= STEP_CAST_SPELL;

  new_seq->next		= new_seq_step(); 
  new_seq		= new_seq->next;

  new_seq->delay	= 1;
  new_seq->instr	= STEP_EFFECT;
  new_seq->iparms[0]	= efn; 

  return base_seq;
}

/* Set up the spells... */

void setup_spells() {  

  char buf[MAX_STRING_LENGTH];

  int spn;
  int efn;

  for(spn = 0; spn < MAX_SPELL; spn++) {

    if (spell_array[spn].loaded != TRUE) {
      break;
    }

   /* Check spell and effect targets match... */

    efn = spell_array[spn].effect;

    if ( efn != EFFECT_UNDEFINED
      && spell_array[spn].target != effect_array[efn].target ) {
      sprintf(buf, "Mismatch spell and effect targets for '%s'\r\n", spell_array[spn].name);
      bug(buf,0);
      exit(1);
    }

   /* Generate default sequence if needed... */

    if ( spell_array[spn].seq == NULL ) {
      spell_array[spn].seq = generate_default_seq(spn, spell_array[spn].name, efn);
    }
  }

  return;
}

/* Lookup a spell by slot number... */

int get_effect_for_slot( int slot ) {

    extern bool fBootDb;
    int efn;

    if ( slot <= 0 )
	return SPELL_UNDEFINED;

    for ( efn = 0; efn < MAX_EFFECT; efn++ ) {
    
	if ( slot == effect_array[efn].slot )
	    return efn;
    }

    if ( fBootDb )
    {
	bug( "Slot_lookup: bad slot %d.", slot );
	exit(1);
    }

    return EFFECT_UNDEFINED;
}

/* Lookup the affect created by a spell... */

int get_affect_afn_for_effect(int efn) {

  if (!valid_spell(efn)) {
    return AFFECT_UNDEFINED;
  }  

  return get_affect_afn(effect_array[efn].affect);
}


/* Read a standard focus specification for a step... */

int read_step_focus(FILE *fp) {

  int tgt;

  char *target;

  target = fread_word(fp);

  if (!str_cmp(target, "actor")) {
    tgt = STEP_FCS_ACTOR;
  } else if (!str_cmp(target, "victim")) {
    tgt = STEP_FCS_VICTIM;
  } else if (!str_cmp(target, "friend")) {
    tgt = STEP_FCS_FRIEND;
  } else if (!str_cmp(target, "foe")) {
    tgt = STEP_FCS_FOE;
  } else if (!str_cmp(target, "all")) {
    tgt = STEP_FCS_ALL;
  } else if (!str_cmp(target, "not_friend")) {
    tgt = STEP_FCS_NOT_FRIEND;
  } else if (!str_cmp(target, "not_foe")) {
    tgt = STEP_FCS_NOT_FOE;
  } else{
    tgt = STEP_FCS_NONE;
  }

  return tgt;
}

/* Read a standard scope specification for a step... */

int read_step_scope(FILE *fp) {

  int scope;

  char *name;

  name = fread_word(fp);

  if (!str_cmp(name, "room")) {
    scope = WEV_SCOPE_ROOM;
  } else if (!str_cmp(name, "room_plus")) {
    scope = WEV_SCOPE_ADJACENT;
  } else if (!str_cmp(name, "subarea")) {
    scope = WEV_SCOPE_SUBAREA;
  } else if (!str_cmp(name, "subarea_plus")) {
    scope = WEV_SCOPE_SUBAREA_PLUS;
  } else if (!str_cmp(name, "area")) {
    scope = WEV_SCOPE_AREA;
  } else if (!str_cmp(name, "area_plus")) {
    scope = WEV_SCOPE_AREA_PLUS;
  } else if (!str_cmp(name, "zone")) {
    scope = WEV_SCOPE_ZONE;
  } else if (!str_cmp(name, "universe")) {
    scope = WEV_SCOPE_UNIVERSE;
  } else if (!str_cmp(name, "group")) {
    scope = WEV_SCOPE_GROUP;
  } else {
    scope = WEV_SCOPE_BAD;
  }     

  return scope;
}

/* Read standard block flags... */

int read_step_block_flags(FILE *fp) {

  int flg;

  char *name;
  char flag[MAX_STRING_LENGTH];

  name = fread_word(fp);

  flag[0] = '\0';

  flg = 0;

  while ( name[0] != '\0' ) {

    name = one_argument(name, flag);

    if (!str_cmp(flag,"block_absorb")) {
      flg |= BLOCK_ABSORB;
    } else 
         
    if (!str_cmp(flag,"block_shield")) {
      flg |= BLOCK_SHIELD;
    } else
         
    if (!str_cmp(flag,"block_parry")) {
      flg |= BLOCK_PARRY;
    } else
         
    if (!str_cmp(flag,"block_dodge")) {
      flg |= BLOCK_DODGE;
    } else
         
    if (!str_cmp(flag,"block_armor")) {
      flg |= BLOCK_ARMOR;
    } else
         
    if (!str_cmp(flag,"block_all")) {
      flg |= BLOCK_ALL;
    } else

    if (!str_cmp(flag,"block_noparry")) {
      flg |= BLOCK_NOPARRY;
    } else

    if (!str_cmp(flag,"block_combat")) {
      flg |= BLOCK_COMBAT;
    } else

    if (!str_cmp(flag,"block_noshield")) {
      flg |= BLOCK_NOSHIELD;
    } else

    if (!str_cmp(flag,"block_ball")) {
      flg |= BLOCK_BALL;
    } else

    if (!str_cmp(flag,"block_bolt")) {
      flg |= BLOCK_BOLT;
    } else

    if (!str_cmp(flag,"block_lightning")) {
      flg |= BLOCK_LIGHTNING;
    } else

    if (!str_cmp(flag,"block_light")) {
      flg |= BLOCK_LIGHT;
    } else {

      bug("Bad BLOCK flag.", 0);
    }
  }

  return flg;
}
 
/* Read standard damage flags... */

int read_step_damage_flags(FILE *fp) {

  int flg;

  char *name;
  char flag[MAX_STRING_LENGTH];

  bool ok;

  name = fread_word(fp);

  flag[0] = '\0';

  flg = 0;

  while ( name[0] != '\0' ) {

    name = one_argument(name, flag);

    ok = FALSE;

    switch (flag[0]) {

      case 'd':

        if (!str_cmp(flag,"dmg_minimum")) {
          flg |= STEP_DMG_MIN;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"dmg_low")) {
          flg |= STEP_DMG_LOW;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"dmg_medium_low")) {
          flg |= STEP_DMG_MED_LOW;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"dmg_medium")) {
          flg |= STEP_DMG_MED;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"dmg_medium_high")) {
          flg |= STEP_DMG_MED_HIGH;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"dmg_high")) {
          flg |= STEP_DMG_HIGH;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"dmg_fatal")) {
          flg |= STEP_DMG_FATAL;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"dmg_overkill")) {
          flg |= STEP_DMG_OVERKILL;
          ok = TRUE;
          break;
        }
         
        break;
 
      case 'h':

        if (!str_cmp(flag,"hit")) {
          flg |= STEP_DMG_HIT;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_minimum")) {
          flg |= STEP_DMG_MIN;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_low")) {
          flg |= STEP_DMG_LOW;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_medium_low")) {
          flg |= STEP_DMG_MED_LOW;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_medium")) {
          flg |= STEP_DMG_MED;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_medium_high")) {
          flg |= STEP_DMG_MED_HIGH;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_high")) {
          flg |= STEP_DMG_HIGH;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_fatal")) {
          flg |= STEP_DMG_FATAL;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_overkill")) {
          flg |= STEP_DMG_OVERKILL;
          ok = TRUE;
          break;
        }
         
        break;
 
      case 's':

        if (!str_cmp(flag,"save_half")) {
          flg |= STEP_DMG_SAVE_HALF;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"save_none")) {
          flg |= STEP_DMG_SAVE_NONE;
          ok = TRUE;
          break;
        }
         
        break;
 
      default:
        break;
    }

    if (!ok) {
      bug("Bad DAMAGE flag", 0);
    }
  }

  return flg;
}

/* Read standard heal flags... */

int read_step_heal_flags(FILE *fp) {
   int flg;
  char *name;
  char flag[MAX_STRING_LENGTH];
  bool ok;

  name = fread_word(fp);

  flag[0] = '\0';
  flg = 0;

  while ( name[0] != '\0' ) {
    name = one_argument(name, flag);
    ok = FALSE;
    switch (flag[0]) {

      case 'h':

        if (!str_cmp(flag,"heal_hits")) {
          flg |= STEP_HEAL_HITS;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_move")) {
          flg |= STEP_HEAL_MOVE;
          ok = TRUE;
          break;
        }
         
        if (!str_cmp(flag,"heal_mana")) {
          flg |= STEP_HEAL_MANA;
          ok = TRUE;
          break;
        }
         
        break;
 
      default:
        break;
    }

    if (!ok) {
      bug("Bad HEAL flag", 0);
    }
  }

  return flg;
}

/* Read a sequence step from a file... */

SEQ_STEP *read_step(FILE *fp) {

  SEQ_STEP *new_step;

  char *name;

  char buf[MAX_STRING_LENGTH];

 /* Get a new step and read its delay... */

  new_step = new_seq_step(); 

  new_step->delay = fread_number(fp);

  if ( new_step->delay < 0
    || new_step->delay > 15 ) { 
    bug("Bad delay '%d' on step.", new_step->delay);
    free_seq_step(new_step);
    return NULL;
  }

 /* Now find out what it is... */

  name = fread_word(fp);

  switch ( name[0] ) {

    case 'A':

     /* n 'Abort' */

      if (!str_cmp(name, "Abort")) {
        new_step->instr = STEP_ABORT;
      }

     /* n 'Area_Damage' 'scope' 'focus-center' 'focus-target' 
                        'atk_desc' 'dam_type' 
                        'block_flags' 'dam_flags'                 */

      if (!str_cmp(name, "Area_Damage")) {
        new_step->instr = STEP_AREA_DAMAGE;

       /* scope */

        new_step->iparms[4] = read_step_scope(fp);

        if ( new_step->iparms[4] == WEV_SCOPE_BAD ) {
          bug("Bad scope string on AREA_DAMAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* focus - center */

        new_step->iparms[2] = read_step_focus(fp);

        if ( new_step->iparms[2] != STEP_FCS_ACTOR
          && new_step->iparms[2] != STEP_FCS_VICTIM ) {
          bug("Bad focus center string on AREA_DAMAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* focus - target */

        new_step->iparms[2] += read_step_focus(fp);

        if ( new_step->iparms[2] < STEP_FCS_FRIEND ) {
          bug("Bad focus target string on AREA_DAMAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* Attack description */ 

        new_step->sparms[0] = str_dup(fread_word(fp));

        if ( new_step->sparms[0][0] == '\0' ) {
          bug("Bad attack description on AREA_DAMAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        } 

       /* damage type */

        name = fread_word(fp);

        new_step->iparms[0] = flag_value(damage_type, name);

        if (new_step->iparms[0] == NO_FLAG) {
          bug("Bad damage type string on AREA_DAMAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* 'block flags' */

        new_step->iparms[1] = read_step_block_flags(fp);

       /* 'damage flags' */

        new_step->iparms[3] = read_step_damage_flags(fp);

      }

     /* n 'Area_Heal'   'scope' 'focus-center' 'focus-target' 
                        'atk_desc' 'dam_type' 
                        'heal_flags' 'dam_flags'                 */

      if (!str_cmp(name, "Area_Heal")) {
        new_step->instr = STEP_AREA_HEAL;

       /* scope */

        new_step->iparms[3] = read_step_scope(fp);

        if ( new_step->iparms[3] == WEV_SCOPE_BAD ) {
          bug("Bad scope string on AREA_HEAL step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* focus - center */

        new_step->iparms[2] = read_step_focus(fp);

        if ( new_step->iparms[2] != STEP_FCS_ACTOR
          && new_step->iparms[2] != STEP_FCS_VICTIM ) {
          bug("Bad focus center string on AREA_HEAL step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* focus - target */

        new_step->iparms[2] += read_step_focus(fp);

        if ( new_step->iparms[2] < STEP_FCS_FRIEND ) {
          bug("Bad focus target string on AREA_HEAL step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* Attack description */ 

        new_step->sparms[0] = str_dup(fread_word(fp));

        if ( new_step->sparms[0][0] == '\0' ) {
          bug("Bad attack description on AREA_HEAL step.", 0);
          free_seq_step(new_step);
          return NULL;  
        } 

       /* 'heal flags' */

        new_step->iparms[1] = read_step_heal_flags(fp);

       /* 'damage flags' */

        new_step->iparms[0] = read_step_damage_flags(fp);

      }

      break;

    case 'C':

     /* n 'Cast Spell' */

      if (!str_cmp(name, "Cast Spell")) {
        new_step->instr = STEP_CAST_SPELL;
      }

     /* n 'Chant' 'words' */

      if (!str_cmp(name, "Chant")) {
        new_step->instr = STEP_CHANT;
        new_step->sparms[0] = str_dup(fread_word(fp));

        if ( new_step->sparms[0][0] == '\0' ) {
          bug("Bad chant string on CHANT step.", 0);
          free_seq_step(new_step);
          return NULL;  
        } 
      }

      break;

    case 'D':

     /* n 'Damage' */

      if (!str_cmp(name, "Damage")) {
        new_step->instr = STEP_DAMAGE;

       /* focus */

        new_step->iparms[2] = read_step_focus(fp);

        if ( new_step->iparms[2] != STEP_FCS_ACTOR
          && new_step->iparms[2] != STEP_FCS_VICTIM ) {
          bug("Bad focus string on DAMAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* Attack description */ 

        new_step->sparms[0] = str_dup(fread_word(fp));

        if ( new_step->sparms[0][0] == '\0' ) {
          bug("Bad attack description on DAMAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        } 

       /* damage type */

        name = fread_word(fp);

        new_step->iparms[0] = flag_value(damage_type, name);

        if (new_step->iparms[0] == NO_FLAG) {
          bug("Bad damage type string on DAMAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* 'block flags' */

        new_step->iparms[1] = read_step_block_flags(fp);

       /* 'damage flags' */

        new_step->iparms[3] = read_step_damage_flags(fp);

      }

      break;

    case 'E':

     /* n 'Effect' 'effect' */

      if (!str_cmp(name, "Effect")) {
        new_step->instr = STEP_EFFECT;

        name = fread_word(fp);
 
        new_step->iparms[0] = get_effect_gefn(name);

        if ( new_step->iparms[0] == EFFECT_UNDEFINED ) {
          sprintf(buf, "Bad effect name '%s' on effect step.", name); 
          bug(buf, 0);
          free_seq_step(new_step);
          return NULL;  
        } 
      }

     /* n 'Echo' 'scope' 'focus' 'actor' 'victim' 'observers' */

      if (!str_cmp(name, "Echo")) {

        new_step->instr = STEP_ECHO;

        new_step->iparms[0] = read_step_scope(fp);

        if (new_step->iparms[0] == WEV_SCOPE_BAD) {
          bug("Bad scope string on ECHO step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }     
 
        new_step->iparms[1] = read_step_focus(fp);

        if ( new_step->iparms[1] != STEP_FCS_ACTOR
          && new_step->iparms[1] != STEP_FCS_VICTIM) {
          bug("Bad focus string on ECHO step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

        new_step->sparms[0] = str_dup(fread_word(fp));

        if ( new_step->sparms[0][0] == '\0' ) {
          free_string(new_step->sparms[0]);
          new_step->sparms[0] = NULL;
        } 

        new_step->sparms[1] = str_dup(fread_word(fp));

        if ( new_step->sparms[1][0] == '\0' ) {
          free_string(new_step->sparms[1]);
          new_step->sparms[1] = NULL;
        } 

        new_step->sparms[2] = str_dup(fread_word(fp));

        if ( new_step->sparms[2][0] == '\0' ) {
          free_string(new_step->sparms[2]);
          new_step->sparms[2] = NULL;
        } 

      }

      break;

    case 'H':

     /* n 'Heal' */

      if (!str_cmp(name, "Heal")) {
        new_step->instr = STEP_HEAL;

       /* focus - affected character */

        new_step->iparms[2] = read_step_focus(fp);

        if ( new_step->iparms[2] != STEP_FCS_ACTOR
          && new_step->iparms[2] != STEP_FCS_VICTIM ) {
          bug("Bad focus center string on AREA_HEAL step.", 0);
          free_seq_step(new_step);
          return NULL;  
        }

       /* Healing description */ 

        new_step->sparms[0] = str_dup(fread_word(fp));

        if ( new_step->sparms[0][0] == '\0' ) {
          bug("Bad heal description on HEAL step.", 0);
          free_seq_step(new_step);
          return NULL;  
        } 

       /* 'heal flags' */

        new_step->iparms[1] = read_step_heal_flags(fp);

       /* 'damage flags' */

        new_step->iparms[0] = read_step_damage_flags(fp);

      }

      break;

    case 'M':

     /* n 'Message' 'focus' 'message' */

      if (!str_cmp(name, "Message")) {

        new_step->instr = STEP_MESSAGE;

        new_step->iparms[0] = read_step_focus(fp);

        if ( new_step->iparms[0] != STEP_FCS_ACTOR 
          && new_step->iparms[0] != STEP_FCS_VICTIM ) {
          bug("Bad focus string on MESSAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        } 

        new_step->sparms[0] = str_dup(fread_word(fp));

        if ( new_step->sparms[0][0] == '\0' ) {
          bug("Bad message string on MESSAGE step.", 0);
          free_seq_step(new_step);
          return NULL;  
        } 
      }

      break;

    case 'P':

     /* n 'Prepare Spell' */

      if (!str_cmp(name, "Prepare Spell")) {
        new_step->instr = STEP_PREPARE_SPELL;
      }

     /* n 'PowerWord' 'word' */

      if (!str_cmp(name, "PowerWord")) {

        new_step->instr = STEP_POWER_WORD;

       /* Null string means use user supplied powerword... */

        new_step->sparms[0] = str_dup(fread_word(fp));

      }

      break;

    case 'S':

     /* n 'Skip' n */

      if (!str_cmp(name, "Skip")) {
        new_step->instr = STEP_SKIP;
        new_step->iparms[0] = fread_number(fp);

        if ( new_step->iparms[0] <= 0 ) {
          bug("Bad skip value %d on SKIP step.", new_step->iparms[0]);
          free_seq_step(new_step);
          return NULL;  
        } 
      }

     /* n 'Sanity' caster victim observer */

      if (!str_cmp(name, "Sanity")) {
        new_step->instr = STEP_SANITY;

        new_step->iparms[0] = fread_number(fp);

        if ( new_step->iparms[0] < 50
          && new_step->iparms[0] != 0 ) {
          bug("Bad actor value %d on SANITY step.", new_step->iparms[0]);
          free_seq_step(new_step);
          return NULL;  
        } 

        new_step->iparms[1] = fread_number(fp);

        if ( new_step->iparms[1] < 50
          && new_step->iparms[1] != 0 ) {
          bug("Bad victim value %d on SANITY step.", new_step->iparms[1]);
          free_seq_step(new_step);
          return NULL;  
        } 

        new_step->iparms[2] = fread_number(fp);

        if ( new_step->iparms[2] < 50
          && new_step->iparms[2] != 0 ) {
          bug("Bad observer value %d on SANITY step.", new_step->iparms[2]);
          free_seq_step(new_step);
          return NULL;  
        } 
      }

      break;

    case 'Y':
      if (!str_cmp(name, "Yell")) {
        new_step->instr = STEP_YELL;
        new_step->sparms[0] = str_dup(fread_word(fp));

        if ( new_step->sparms[0][0] == '\0' ) {
          bug("Bad yell string on YELL step.", 0);
          free_seq_step(new_step);
          return NULL;  
        } 
      }
      break;

    default:
      sprintf(buf, "Unrecognized step name '%s'.", name); 
      bug(buf, 0);
      free_seq_step(new_step);
      return NULL;  
  }

  return new_step;
}


/* Load the skills from disk... */

/* NB This routines aborts the mud if it finds an error, as it can't
 *    guarentee that the skills will be properly loaded.  This could
 *    cause serious damage (lost skills) to characters.
 */ 

void load_spells() {
  FILE *fp;
  char *kwd;
  char code;
  bool done,  ok;
  char *name;
  int sn, num, skn;
   ECOND_DATA *new_ec, *last_ec;
   char buf[MAX_STRING_LENGTH];
  SEQ_STEP *last_step, *new_step;

 /* Initialize incase of problems... */

  for(sn = 0; sn < MAX_SPELL; sn++) {
    spell_array[sn].loaded = FALSE;
  } 

 /* Initialize skill 0 */

  sn = 0;

  spell_array[sn].name 		= "none";
  spell_array[sn].desc 		= NULL;
  spell_array[sn].info		= NULL;
  spell_array[sn].target 		= TAR_IGNORE; 
  spell_array[sn].mana		= 0;
  spell_array[sn].diff		= 0;
  spell_array[sn].range		= WEV_SCOPE_NONE;
  spell_array[sn].sn		= SKILL_UNDEFINED;
  spell_array[sn].sn2		= SKILL_UNDEFINED;
  spell_array[sn].discipline		= 0;
  spell_array[sn].music_style	= 0;
  spell_array[sn].music_power	= 0;
  spell_array[sn].effect		= EFFECT_UNDEFINED;
  spell_array[sn].component	= NULL;
  spell_array[sn].seq		= NULL;
  spell_array[sn].cond		= NULL;
  spell_array[sn].loaded 		= TRUE;
  spell_array[sn].parse_info	= FALSE;

 /* Find the banks file... */

  fp = fopen(SPELL_FILE, "r");

  if (fp == NULL) {
    log_string("No spell file!");
    exit(1);
    return;
  }

 /* Read through it and see what we've got... */
 
  done = FALSE;

  last_step = NULL;
  new_step = NULL;

  last_ec = NULL;
  new_ec = NULL;

  while(!done) {

    kwd = fread_word(fp);

    code = kwd[0];

    ok = FALSE;

    switch (code) {

    /* Comments */

      case '#':
        
        ok = TRUE;

        fread_to_eol(fp); 

        break; 

    /* Comp */

      case 'C':
 
       /* Comp 'name' */

        if (!strcmp(kwd, "Comp")) {

          ok = TRUE;

         /* Read the component name... */

          name = fread_word(fp); 

          if (name[0] == '\0') {
            sprintf(buf, "Unnammed component for spell %s.", spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            spell_array[sn].component = str_dup(name);
          }
        }

       /* Cond econd_parms */

        if (!strcmp(kwd, "Cond")) {

          ok = TRUE;

         /* Read the component name... */

          new_ec = read_econd(fp); 

          if (new_ec == NULL) {
            sprintf(buf, "Bad condition for spell %s.", spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            if (last_ec == NULL) { 
              spell_array[sn].cond = new_ec;
            } else {
              last_ec->next = new_ec;
            }
            last_ec = new_ec;
          }
        }

        break; 

     /* Diff */

      case 'D':

       /* Desc description~ */ 

        if (!strcmp(kwd, "Desc")) {
 
          ok = TRUE;

         /* Pull in the description... */

          spell_array[sn].desc = fread_string(fp);

        }

       /* Diff d */ 

        if (!strcmp(kwd, "Diff")) {
           ok = TRUE;

         /* Pull in the number... */
          num = fread_number(fp);
          if ( num < 0 
            || num > 999 ) {
            sprintf(buf, "Bad diff %d for spell %s.", num, spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            spell_array[sn].diff = num;
          }
        }


        if (!strcmp(kwd, "Dis")) {
           ok = TRUE;

          name = fread_word(fp);
          spell_array[sn].discipline = flag_value(discipline_type, name);
        }

        break; 

     /* Effect */

      case 'E':

       /* Effect 'name' */

        if (!strcmp(kwd, "Effect")) {

          ok = TRUE;

         /* Read the effect name... */

          name = fread_word(fp); 

          if (name[0] == '\0') {
            sprintf(buf, "Unnammed effect for spell %s.",  spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            spell_array[sn].effect = get_effect_efn(name);

            if (spell_array[sn].effect == EFFECT_UNDEFINED) {
              sprintf(buf, "Bad effect '%s' for spell %s.", name, spell_array[sn].name);
              bug(buf, 0);
              exit(1);
            }
          }
        }

       /* File ends with an 'End' */

        if (!strcmp(kwd, "End")) {
          ok = TRUE;
          done = TRUE;
        } 

        break; 

     /* Mana */

      case 'I':
        if (!strcmp(kwd, "Info")) {
           ok = TRUE;
           name = fread_word(fp); 

           if (name[0] == '\0') {
               sprintf(buf, "Unnammed info for spell %s.", spell_array[sn].name);
               bug(buf, 0);
               exit(1);
           } else {
               if (!str_cmp(name, "parse")) {
                    spell_array[sn].parse_info = TRUE;
               } else {
                    spell_array[sn].info = str_dup(name);
                    spell_array[sn].parse_info = FALSE;
               }                    
           }
        }
        break;

     /* Mana */

      case 'M':

        if (!strcmp(kwd, "Mana")) {
          ok = TRUE;
          num = fread_number(fp);

          if ( num < 0 || num > 999 ) {
            sprintf(buf, "Bad mana %d for spell %s.", num, spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            spell_array[sn].mana = num;
          }
        }

        if (!strcmp(kwd, "Music")) {
          ok = TRUE;
          name = fread_word(fp);
          num = music_number(name);
          if (num == 0) {
            sprintf(buf, "Bad music style %d for spell %s.", num, spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          }
          spell_array[sn].music_style = num;

          num = fread_number(fp);
          if (num < 0 || num > 999 ) {
            sprintf(buf, "Bad music power %d for spell %s.", num, spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          }
          spell_array[sn].music_power = num;
        }
        break; 

      case 'R':

        if (!strcmp(kwd, "Range")) {
            ok = TRUE;
            name = fread_word(fp); 

            if (name[0] == '\0') {
                 sprintf(buf, "Unnammed range for spell %s.", spell_array[sn].name);
                 bug(buf, 0);
                 exit(1);
            } else {
                 if (!str_cmp(name, "room")) {
                      spell_array[sn].range = WEV_SCOPE_ROOM;
                 } else if (!str_cmp(name, "subarea")) {
                      spell_array[sn].range = WEV_SCOPE_SUBAREA;
                 } else if (!str_cmp(name, "area")) {
                      spell_array[sn].range = WEV_SCOPE_AREA;
                 } else if (!str_cmp(name, "zone")) {
                      spell_array[sn].range = WEV_SCOPE_ZONE;
                 } else if (!str_cmp(name, "universe")) {
                      spell_array[sn].range = WEV_SCOPE_UNIVERSE;
                 } else {
                      spell_array[sn].range = WEV_SCOPE_BAD;
                 }     
            }
        }
        break;

    /* Skill... */

      case 'S':
 
       /* SKILL name */

        if (!strcmp(kwd, "SPELL")) {
          ok = TRUE;

         /* Read the skill name... */
          name = fread_word(fp); 
          if (name[0] == '\0') {
            bug("Unnammed spell in spell file!", 0);
            exit(1);
          }

         /* Get the spell number... */
 
          sn++;

          if (sn >= MAX_SPELL) { 
            bug("Too many spells in spell file!",0);
            exit(1);
          }

         /* Initialize data... */ 

          spell_array[sn].name 		= str_dup(name);
          spell_array[sn].desc 		= NULL;
          spell_array[sn].info 		= NULL;
          spell_array[sn].target 	= TAR_IGNORE; 
          spell_array[sn].mana		= 0;
          spell_array[sn].diff		= 0;
          spell_array[sn].range		= WEV_SCOPE_NONE;
          spell_array[sn].sn		= SKILL_UNDEFINED;
          spell_array[sn].sn2		= SKILL_UNDEFINED;
          spell_array[sn].effect	= EFFECT_UNDEFINED;
          spell_array[sn].component	= NULL;
          spell_array[sn].seq		= NULL;
          spell_array[sn].cond		= NULL;
          spell_array[sn].loaded 	= TRUE;

          last_step = NULL;

          last_ec = NULL;

        }

       /* Skill 'name' */

        if (!strcmp(kwd, "Skill")) {

          ok = TRUE;

         /* Read the skill name... */

          name = fread_word(fp); 

          if (name[0] == '\0') {
            sprintf(buf, "Unnammed skill for spell %s.", spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            skn = get_skill_sn(name);

            if (skn == SKILL_UNDEFINED) {
              sprintf(buf, "Bad skill '%s' for spell %s.", name, spell_array[sn].name);
              bug(buf, 0);
              exit(1);
            }

            if ( spell_array[sn].sn == SKILL_UNDEFINED ) {
              spell_array[sn].sn = skn;
            } else if ( spell_array[sn].sn2 == SKILL_UNDEFINED ) {
              spell_array[sn].sn2 = skn;
            } else {
              sprintf(buf, "To many skills for spell %s.",
                            spell_array[sn].name);
              bug(buf, 0);
              exit(1);
            }
          }
        }

       /* Step 'type' delay parms */

        if (!strcmp(kwd, "Step")) {

          ok = TRUE;

         /* Read the step name... */

          new_step = read_step(fp);

          if (new_step == NULL) {
            sprintf(buf, "Bad step for spell %s.",
                          spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          }

         /* Link in... */

          if (last_step == NULL ) {
            
            if ( new_step->instr != STEP_PREPARE_SPELL ) {
              sprintf(buf,  "Bad sequence for spell '%s' - first step not prepare.", spell_array[sn].name);
              bug(buf, 0);
              exit(1);
            }

            spell_array[sn].seq = new_step;

          } else {
            last_step->next = new_step;  
          }  

          last_step = new_step;

          last_ec = NULL;
        }

       /* StepCond econd_parms */

        if (!strcmp(kwd, "StepCond")) {

          ok = TRUE;

         /* Read the component name... */

          new_ec = read_econd(fp); 

          if (new_ec == NULL) {
            sprintf(buf, "Bad step condition for spell %s.",
                          spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            if (last_ec == NULL) {
              if (last_step == NULL) {
                sprintf(buf, "Bad step condition sequence for spell %s.",
                              spell_array[sn].name);
                bug(buf, 0);
                exit(1);
              } else { 
                last_step->cond = new_ec;
              } 
            } else {
              last_ec->next = new_ec;
            }
            last_ec = new_ec;
          }
        }

        break;

     /* Target */

      case 'T':

       /* Target m */ 

        if (!strcmp(kwd, "Target")) {
 
          ok = TRUE;

         /* Pull in the number... */

          num = fread_number(fp);

          if ( num < 0 
            || num > 6 ) {
            sprintf(buf, "Bad target %d for spell %s.", num, spell_array[sn].name);
            bug(buf, 0);
            exit(1);
          } else {
            spell_array[sn].target = num;
          }
        }

        break; 

     /* Anything else is an error... */

      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in spells file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
  }

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* All done... */

  log_string("...spells loaded");
  return;
}

