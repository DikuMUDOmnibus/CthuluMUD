/* 
 * CthulhuMud
 */

/* Colours */

 static const char desc_col[][20] = {
   "red", "orange", "yellow", "blue", "green", "indigo", "purple",
   "tangerine", "vermillion", "crimson", "scarlet", "lilcal",
   "lilly white", "cream", "coffee", "brown", "tan", "sky blue",
   "pale blue", "pink", "rose", "peach", "black", "mud coloured",
   "lemon", "lime", "aquamarine", "turquoise" 
 };

/* Metals */

#define NUM_DESC_COL 28

 static const char desc_met[][20] = {  
   "gold", "silver", "bronze", "brass", "tin", "copper", "zinc",
   "iron", "steel", "mercury", "mitheral", "adamantite", "red gold",
   "white gold", "aluminium"
 };

#define NUM_DESC_MET 15

/* Gems */

 static const char desc_gem[][20] = {
   "ruby", "diamond", "saphire", "emerald", "tiger eye", "citrone",
   "lapis lazuli", "quartz", "amythyst", "pearl"
 };

#define NUM_DESC_GEM 10

/* Liquid consistencies */ 

 static const char desc_con[][20] = {
   "thin", "clear", "runny", "thick", "syrupy", "bubbly", 
   "swirling", "misty", "disgusting", "viscid", "frothy",
   "effervescent", "acidic", "tart", "smoky", "congealed",
   "flat", "plain", "lumpy", "sugery", "translucent",
   "oily", "slimey"
 };

#define NUM_DESC_CON 23

 static const int pot_spells[] = {
     1,      // Armour 
     3,      // Bless
    13,      // Cure blindness
    13,      // Cure blindness
    14,      // Cure critical
    15,      // Cure light 
    15,      // Cure light 
    15,      // Cure light 
    17,      // Detect evil
    18,      // Detect invisible
    19,      // Detect magic
    20,      // Detect poison
    28,      // Invisability
    33,      // Protect Evil
    35,      // Sanctuary
    37,      // Giant Strength
    41,      // Cure poison
    41,      // Cure poison
    42,      // Detect hidden
    45,      // Fly
    47,      // Know alignment
    49,      // Cure serious
    54,      // Stone skin
    55,      // Shield
    55,      // Shield
    62,      // Infravision
    64,      // Refresh
    64,      // Refresh
    64,      // Refresh
    85,      // Cure Disease
    85,      // Cure Disease
    86,      // Haste
    93,      // Calm
   101,      // Protection from good
   102,      // Detect good
   110,      // Remove fear                36

     4,      // Blindness
    16,      // Curse
    32,      // Poison
    36,      // Sleep
    50,      // Cause light
    56,      // Weaken
    59,      // Faerie Fire
    65,      // Change sex
    88,      // Frenzy
   100,      // Fear
   107,      // Remove invisability        11

    27,      // Heal
    51,      // Cause critical 
    61,      // Pass Door
    82,      // Youth
    83,      // Age   
    87,      // Plague    
    98,      // Absorb magic
   104,      // Regeneration
   105,      // Fire shield
   108,      // Vocalize
   111,      // Mute                       11
 }; 

#define NUM_POT_SPELLS 58

#define NUM_PS_RANK1 36
#define NUM_PS_RANK2 47
#define NUM_PS_RANK3 58

