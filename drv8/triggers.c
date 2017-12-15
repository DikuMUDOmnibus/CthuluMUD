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
#include "triggers.h"
#include "wev.h"
#include "mob.h"
#include "econd.h"
#include "olc.h"
#include "society.h"
#include "chat.h"
#include "skill.h"

struct ev_mapping ev_map[] = {

 { "none", 
      { "none", NULL } },

 { "comm", 
      { "none", NULL } },

 { "give", 
      { "none", "gold", "item", NULL } },

 { "get",
      { "none", "gold", "item", NULL } },

 { "put",
      { "none", "item", NULL } },

 { "drop", 
      { "none", "gold", "item", "plant", "melt", NULL } },

 { "poison", 
      { "none", "food", "weapon", "fountain", NULL } },

 { "fill", 
      { "none", "fountain", "light", NULL } },

 { "drink", 
      { "none", "fountain_ok", "fountain_bad", "item_ok", "item_bad", 
         "feed_ok", "feed_bad", NULL } },

 { "eat", 
      { "none", "food_ok", "food_bad", "pill", "herb", "corpse",
         "feed_ok", "feed_bad",  NULL } },

 { "sacrifice", 
      { "none", "pc_corpse", "corpse", "trash", "treasure", 
        NULL } },

 { "gadget", 
      { "none", "gadget_ok", "gadget_bad", NULL } },

 { "search", 
      { "none", "item", "room", "none", "none", 
        "find_item", "find_mob", "find_door", "find_hound", "conceal", NULL } },

 { "depart", 
      { "none", "walk", "sneak", "fly", "swim", 
        "sail", "portal", "magic", "current", "flee", NULL } },

 { "arrive", 
      { "none", "walk", "sneak", "fly", "swim", 
        "sail", "portal", "magic", "current", "flee", NULL} },

 { "pulse", 
      { "none", "1_sec", "3_sec", "4_sec", "5_sec",
        "10_sec", "30_sec", "area", NULL } },

 { "time", 
      { "none", "hour", "day", "sunrise", "sunset", 
        "dawn", "dusk", NULL } },

 { "control", 
      { "none", "login", "logout", "linkdead", "reconnect", NULL } },

 { "mob", 
      { "none", "stop", "select", "echo", NULL } },

 { "death", 
      { "none", "slain","stun", NULL } },

 { "attack", 
      { "none", "kill", "kick", "trip", "bash", 
        "dirt", "backstab", "disarm", "circle", "rotate", "murder",
        "tail", "crush", "distract", "sprinkle", NULL} },

 { "combat", 
      { "none", "miss", "hit", "block", "parry", 
        "dodge", "absorb", "immune", "fade", NULL } },

 { "damage", 
      { "none", "injured", "hurt", "dirt", "trip", 
        "bash", "env", "tail", "crush", NULL } },

 { "oocc", 
      { "none", "beep", "gossip", "music", "immtalk", 
        "question", "answer", "tell", "gtell", "herotalk",
        "social", "stell", "mtell", "investigator", "chat", NULL } },

 { "icc", 
      { "none", "tell", "say", "shout", "scream", 
        "yell", "mtell", "telepathy", "pray", NULL } },

 { "social", 
      { "none", "emote", "fsoul", "nsoul", "hsoul", "csoul",
        NULL } },

 { "idol", 
      { "none", "hold", "pray", "object", "gold", 
        NULL } },

 { "lock", 
      { "none", "d_open", "i_open", "d_close", "i_close", 
        "d_lock", "i_lock", "d_unlock", "i_unlock", "d_pick", "i_pick",
        NULL } },

 { "society", 
      { "none", "invite", "join", "advance", "resign", 
        "expel", "demote", "test", "foe", "pardon", NULL } },

 { "dream", 
      { "none", "walk", "awaken", "say", "psay", 
        "emote", "pemote", "cast", "pcast", NULL } },

 { "activity", 
      { "none", "start", "stop", "poschange", NULL } },

 { "spell", 
      { "none", "prep", "start", "chant", "cast", 
        "recover", "fail", "consume", "echo", "power",
        "yell", NULL } },

 { "heal", 
      { "none", "hits", "move", "mana", "healer", "therapy", NULL } },

 { "debate", 
      { "none", "start", "continue", "finish", NULL } },

 { "knock", 
      { "none", "knock", NULL } },

 { "interpret", 
      { "none", "strange", NULL } },

 { "oprog", 
      { "none", "use", "wear", "remove", "explosion", "photo" ,
         "destroy", "rebuild", "effect", "combine", NULL } },

 { "learn", 
      { "none", "practice", "learn", NULL } },

 { "duel", 
      { "none", "start", "continue", "finish" "offensive", NULL } },

 { "trap", 
      { "none", "lay", "disarm", "info" "trigger", NULL } },

 { "show", 
      { "none", "item", "passport", NULL } },

 { "music", 
      { "none", "sing", "play", NULL } },

 { "peek", 
      { "none", "item", NULL } },

}; 

/* Trigger Management... */

static MOB_TRIGGER *free_trigger_chain = NULL;

/* Get a blank trigger... */

MOB_TRIGGER *get_trigger() {

  MOB_TRIGGER *new_trigger;

  if (free_trigger_chain == NULL) {
    new_trigger = (MOB_TRIGGER *) alloc_perm(sizeof(*new_trigger));
  } else {
    new_trigger = free_trigger_chain;
    free_trigger_chain = new_trigger->next;
  }

  new_trigger->next = NULL;
  new_trigger->next_type = NULL;
  new_trigger->cond = NULL; 
  new_trigger->cond = NULL; 

  new_trigger->seq = 0;
  new_trigger->type = 0;
  new_trigger->sub_type = 0;
  new_trigger->script_id = 0; 

  return new_trigger;
}

/* Free an unwanted trigger... */

void free_trigger(MOB_TRIGGER *trigger) {

  ECOND_DATA *ec;

  if (trigger == NULL) {
    return;
  }

  if (trigger->next != NULL) {
    free_trigger(trigger->next);
    trigger->next = NULL;
  }

  if (trigger->next_type != NULL) {
    free_trigger(trigger->next_type);
    trigger->next_type = NULL;
  }  

  while (trigger->cond != NULL) {
    ec = trigger->cond;
    trigger->cond = ec->next;
    free_ec_norecurse(ec);
  }

  if (trigger->text != NULL) {
    free_string(trigger->text);
    trigger->text = NULL;
  }

  trigger->next = free_trigger_chain;
  free_trigger_chain = trigger;

  return;
}

/* Trigger set management... */

static MOB_TRIGGER_SET *free_trs_chain = NULL;

MOB_TRIGGER_SET *get_trs() {
  
  MOB_TRIGGER_SET *new_trs;

  if (free_trs_chain == NULL) {
    new_trs = (MOB_TRIGGER_SET *) alloc_perm(sizeof(*new_trs));
  } else {
    new_trs = free_trs_chain;
    free_trs_chain = new_trs->next;
  }

  new_trs->next 	= NULL;
  new_trs->challange 	= NULL;
  new_trs->reaction 	= NULL;
  new_trs->membership 	= NULL; 
  new_trs->scripts 	= NULL;
  new_trs->chat		= NULL;

  return new_trs;
}

void free_trs(MOB_TRIGGER_SET *trs) {

  if (trs->scripts != NULL) {
    release_scripts(trs->scripts);
    trs->scripts = NULL; 
  }

  if (trs->challange != NULL) {
    free_trigger(trs->challange);
    trs->challange = NULL;
  }
 
  if (trs->reaction != NULL) {
    free_trigger(trs->reaction);
    trs->reaction = NULL;
  } 

  if (trs->membership != NULL) {
    free_membership(trs->membership);
    trs->membership = NULL;
  }

  if (trs->chat != NULL) {
    free_chat(trs->chat);
    trs->chat = NULL;
  } 
 
  trs->next = free_trs_chain;
  free_trs_chain = trs;

  return;
}

// Trigger Structure
//
//   trigger  -->  trigger  -->  trigger  -->  trigger
//    type1         type2         type3         type4
//     seq1          seq1          seq1          seq1
//      V             V             V             V
//     seq2          seq2          seq2          seq2
//      V                           V             V
//     seq3                        seq3          seq3
//      V                           V
//     seq4                        seq4
//      V
//     seq5
//
//  trigger->next is a down a type chain
//  trigger->next_type is across to the next chain
//  

/* Insert a trigger into a structure... */

MOB_TRIGGER *add_trigger(MOB_TRIGGER *base, MOB_TRIGGER *new_trigger) {

  MOB_TRIGGER *old;
  MOB_TRIGGER *cur;
  MOB_TRIGGER *last;

 /* Safety check first... */

  if (new_trigger == NULL) {
    return base;
  } 

 /* Find the base chain... */

  old = NULL;
  cur = base;

  while ( cur != NULL
       && cur->type != new_trigger->type) {
    old = cur;
    cur = cur->next_type;
  }  

 /* New chain? */

  if (cur == NULL) {
    if (old == NULL) {
      return new_trigger;
    } 
  
    old->next_type = new_trigger;
    return base;
  }

 /* Find place by sequence number... */

  last = NULL;

  while ( cur != NULL
       && cur->seq <= new_trigger->seq) {
    last = cur; 
    cur = cur->next;
  }

 /* End of chain? */

  if (cur == NULL) {
    last->next = new_trigger;
    return base;
  }

 /* Splice at top of list... */

  if (last == NULL) {
    new_trigger->next = cur;
    new_trigger->next_type = cur->next_type;
    cur->next_type = NULL;

    if (old == NULL) {
      return new_trigger;
    }

    old->next_type = new_trigger;

    return base;
  }   

 /* Splice into the list... */ 

  last->next = new_trigger;
  new_trigger->next = cur;

  return base;
}

/* Remove (and delete) a trigger from a structure... */

MOB_TRIGGER *del_trigger(MOB_TRIGGER *base, MOB_TRIGGER *junk) {

  MOB_TRIGGER *old;
  MOB_TRIGGER *cur;
  MOB_TRIGGER *last;

 /* Safety check first... */

  if (junk == NULL) {
    return base;
  } 

 /* Find the base chain... */

  old = NULL;
  cur = base;

  while ( cur != NULL
       && cur->type != junk->type) {
    old = cur;
    cur = cur->next_type;
  }  

 /* Not found? */

  if (cur == NULL) {
    return base;
  }

 /* Find place by sequence number... */

  last = NULL;

  while ( cur != NULL
       && cur != junk ) {
    last = cur; 
    cur = cur->next;
  }

 /* Not found? */

  if (cur == NULL) {
    return base;
  }

 /* Remove from top of list... */

  if (last == NULL) {
 
   /* Head of the very first chain... */
 
    if (old == NULL) {

      if (junk->next == NULL) {
        cur = junk->next_type;
        junk->next_type = NULL;
        free_trigger(junk);
        return cur;
      }

      cur = junk->next; 
      cur->next_type = junk->next_type;
      
      junk->next_type = NULL;
      junk->next = NULL;
      free_trigger(junk);

      return cur;
    }

   /* Head of a later chain... */

    if (junk->next == NULL) {

     /* Chain of one... */  

      old->next_type = junk->next_type;
    } else {

     /* Chain of many... */

      old->next_type = junk->next;
      junk->next->next_type = junk->next_type;
    }

   /* Lose the deleted one... */
 
    junk->next = NULL;
    junk->next_type = NULL;

    free_trigger(junk); 

    return base;
  }   

 /* Splice out of the list... */ 

  last->next = junk->next;
  
  junk->next = NULL;
  junk->next_type = NULL;
  free_trigger(junk); 

  return base;
}

/* Find a trigger chain by type... */

MOB_TRIGGER *find_trigger_chain(MOB_TRIGGER *base, int type) {

  MOB_TRIGGER *cur;

 /* Find the base chain... */

  cur = base;

  while ( cur != NULL
       && cur->type != type) {
    cur = cur->next_type;
  }  

 /* Give whatever we have got... */

  return cur;
}

/* Find a trigger within a chain... */

MOB_TRIGGER *find_trigger(MOB_TRIGGER *chain, int seq) {

  MOB_TRIGGER *cur;

 /* Find the trigger... */

  cur = chain;

  while ( cur != NULL
       && cur->seq != seq) {
    cur = cur->next;
  }

 /* Give what we've got... */

  return cur;
}

/* Renumber all triggers within a chain... */

MOB_TRIGGER *renum_triggers(MOB_TRIGGER *base) {

  MOB_TRIGGER *chain;
  MOB_TRIGGER *cur;

  int seq;

  chain = base;

  while (chain != NULL) {

    cur = chain;
    seq = 10;

    while (cur != NULL) {
      
      cur->seq = seq;
      seq += 10;

      cur = cur->next;
    }

    chain = chain->next_type;
  } 
   
  return base;
}
   
/* Check if a triggers text string matches an event... */

bool trigger_text_matched(char *text, char *test) {
  char comp[MAX_STRING_LENGTH];
  char smain[MAX_STRING_LENGTH];
  char sub[MAX_STRING_LENGTH];

  bool smain_match, sub_match;

  char *smain_ptr;
  char *sub_ptr;

  char *token;

  int i;

 /* No comparison string means it matches anything... */

  if (test == NULL) return TRUE;
  
 /* No text string means it can't match... */

  if (text == NULL) return FALSE;
  
 /* Matching is aaa&bbb|ccc&ddd, with | having precedence */

 /* Chop up the match string... */

  strcpy(comp, text);

  i = 0;

  while (comp[i] != '\0') {
    if (isalnum(comp[i])) {
      comp[i] = tolower(comp[i]);
    } else {
      comp[i] = ' ';
    }
    i += 1;
  }

  comp[i++] = ' ';
  comp[i] = '\0';

  strcpy(smain, test);

  smain_ptr = smain;

  token = strsep(&smain_ptr, "|");

  smain_match = FALSE;

  while ( token != NULL
       && !smain_match) {

    strcpy(sub, token);

    sub_ptr = sub;

    token = strsep(&sub_ptr, "&"); 

    sub_match = TRUE;

    while ( token != NULL 
         && sub_match) {

      if (strstr(comp, token) == NULL) {
        sub_match = FALSE;
      }

      token = strsep(&sub_ptr, "&");
    }

    smain_match |= sub_match;

    token = strsep(&smain_ptr, "|");
  }

 /* Default to saying it matched... */

  return smain_match;
}

/* Check if a trigger matches an event. */
/* If we get a match, schedule any script and return FALSE. */
 
bool check_trigger(MOB_TRIGGER *base, WEV *wev) {
  MOB_TRIGGER *chain;
  MOB_TRIGGER *trig;
  MOB_SCRIPT *script;
  char *sectext = NULL;
  bool ok;

 /* Safety first... */

  if (wev == NULL) return TRUE;
  if (base == NULL) return TRUE;

 /* Ok, we got something to check... */

  chain = find_trigger_chain(base, wev->type);

 /* No chain, means no match... */

  if (chain == NULL) return TRUE;

 /* Now search and check by subtype... */

  trig = chain;

  while (trig != NULL) {

    if ( trig->sub_type == wev->subtype
    || trig->sub_type == WEV_SUBTYPE_NONE ) {

      if ( trig->cond == NULL
      || ec_satisfied_mcc(wev->context, trig->cond, TRUE)) {

        if ( trig->text != NULL
        && wev->type == WEV_LEARN) {
            if (wev->context->number != 0)  sectext = get_skill_group_name(wev->context->number);
        }

        ok = FALSE;
        if ( trig->text == NULL) {
            ok = TRUE;
        } else {
            if (trigger_text_matched(wev->context->text, trig->text)) ok = TRUE;
            if (sectext) {
               if (trigger_text_matched(sectext, trig->text)) ok = TRUE;
            }
        }

        if (ok) {

          if (trig->script_id != 0) {
            script = find_script(wev->context->mob, trig->script_id);
           
            if (script != NULL) {
              enqueue_script(wev->context->mob, script, wev->context);
            }
             
          }

          return FALSE;
        }
      }
    }

    trig = trig->next;
  }

  return TRUE;
}

/* Write a trigger structure out to a file... */

void write_triggers(MOB_TRIGGER *base, FILE *fp, char *hdr) {

  MOB_TRIGGER *trig;
  
 /* Save the current chain... */

  trig = base;

  while (trig != NULL) {

    fprintf(fp, "%s %d %d %d %d\n",
                 hdr, 
                 trig->type,
                 trig->seq,
                 trig->sub_type,
                 trig->script_id);
                 
    if (trig->text != NULL) {
      fprintf(fp, "TriggerText %s~\n", trig->text);
    }

    if (trig->cond != NULL) {
      write_econd(trig->cond, fp, "TriggerCond");
    }

    trig = trig->next;
  }

 /* Save the next chain... */

  if (base->next_type != NULL) {
    write_triggers(base->next_type, fp, hdr);
  }

 /* All done... */ 

  return;
}

/* Read a trigger in from a file... */

MOB_TRIGGER *read_trigger(FILE *fp) {

  MOB_TRIGGER *new_trigger;

  new_trigger = get_trigger();

  new_trigger->type = fread_number(fp);
  new_trigger->seq = fread_number(fp);
  new_trigger->sub_type = fread_number(fp);
  new_trigger->script_id = fread_number(fp);

  return new_trigger;
}
  
int ev_map_type_to_int(char *type) {

  int index;

  index = 0;

  while (index < MAX_WEV) {

    if (!strcmp(ev_map[index].name, type)) {
      return index;
    }
     
    index += 1; 
  }

  return WEV_TYPE_NOT_FOUND;
}
  
int ev_map_sub_type_to_int(char *subtype, int type) {

  int index;

  index = 0;

  while ( ev_map[type].events[index] != NULL ) {

    if (!strcmp(ev_map[type].events[index], subtype)) {
      return index;
    }
     
    index += 1; 
  }

  return WEV_SUBTYPE_NOT_FOUND;
}
  
/* OLC Support for triggers... */

/* List triggers... */

void medit_trigger_list(CHAR_DATA *ch, MOB_TRIGGER *base, char *args) {

  int type;

  MOB_TRIGGER *trig;

  char buf[MAX_STRING_LENGTH];

  int count;

 /* Do we have a specific type? */

  if (args[0] != '\0') {
    type = ev_map_type_to_int(args);

    trig = base;

    while ( trig != NULL
         && trig->type != type) {
      trig = trig->next_type;
    }

    if (trig == NULL) {
      send_to_char("No triggers of that type defined.\r\n", ch); 
      return;
    }

    sprintf(buf, "Triggers for event [%s]\r\n"
                 "Seq    Subtype       Script  Text\r\n"
                 "-----  ------------  ------  -------------------------\r\n",
                  ev_map[type].name);

    send_to_char(buf, ch);

    while (trig != NULL) {

      sprintf(buf, "%5d  %12s  %6d  %s\r\n", trig->seq, ev_map[type].events[trig->sub_type], trig->script_id, ( trig->text ? trig->text : "-"));
      send_to_char(buf, ch);

      if (trig->cond != NULL) print_econd(ch, trig->cond, " ");
      trig = trig->next;
    }

    return;
  }

 /* Otherwise, we'll just list the event types... */
  
  trig = base;

  if (trig == NULL) {
    send_to_char("No triggers defined.\r\n", ch);
    return;
  }

  send_to_char("Triggers defined for:" , ch);

  count = 0;

  while (trig != NULL) {

    sprintf(buf, " [%s]", ev_map[trig->type].name);
    send_to_char(buf, ch);

    if ((++count % 5) == 0) {
      send_to_char("\r\n                     ", ch);
    } 

    trig = trig->next_type;
  }

  if ((count % 5) != 0) {
    send_to_char("\r\n", ch);
  }  

  return;
}

/* Add a trigger... */

MOB_TRIGGER *medit_trigger_add(	CHAR_DATA	*ch, 
				MOB_TRIGGER	*base, 
				char		*args) {

  char next_word[MAX_INPUT_LENGTH];

  int type;
  int sub_type;
  int seq;
  int script_id;

  MOB_TRIGGER *trig, *chain, *old_trig;

 /* Extract parms... */

  args = one_argument(args, next_word);

  type = ev_map_type_to_int(next_word);

  args = one_argument(args, next_word);

  seq = atoi(next_word);

  args = one_argument(args, next_word);

  if ( type < 1
    || type >= MAX_WEV) {
    sub_type = WEV_SUBTYPE_NOT_FOUND;
  } else {
    sub_type = ev_map_sub_type_to_int(next_word, type);
  }

  args = one_argument(args, next_word);

  script_id = atoi(next_word);

 /* Validation... */

  if ( type < 1
    || type >= MAX_WEV) {
    send_to_char("Invalid type!\r\n", ch);
    return base;
  }

  if ( seq < 1) {
    send_to_char("Invalid sequence number!\r\n", ch); 
    return base;
  }

  if ( sub_type < WEV_SUBTYPE_NONE 
    || sub_type == WEV_SUBTYPE_NOT_FOUND ) {
    send_to_char("Invalid sub_type!\r\n", ch);
    return base;
  }

  if ( script_id < 1 ) {
    send_to_char("Invalid script id!\r\n", ch);
    return base;
  }   

 /* Ok, make the new trigger... */

  trig = get_trigger();

  trig->type 		= type;
  trig->sub_type 	= sub_type;
  trig->seq 		= seq;
  trig->script_id	= script_id;

  if (args[0] != '\0') {
    trig->text = strdup(args);
    smash_tilde(trig->text);
  }

 /* Now, see if there's an old trigger... */

  chain = find_trigger_chain(base, type);

  if (chain != NULL) {
   
    old_trig = find_trigger(chain, seq);

    if (old_trig != NULL) {

      trig->cond = old_trig->cond;
      old_trig->cond = NULL;

      base = del_trigger(base, old_trig);

      old_trig = NULL;
    }
  }

 /* Ok, safe to add the new one... */

  base = add_trigger(base, trig);

  send_to_char("Trigger added/updated.\r\n", ch);

 /* All done... */

  return base;
}

/* Add a trigger condition... */

MOB_TRIGGER *medit_trigger_add_cond(	CHAR_DATA	*ch, 
					MOB_TRIGGER	*base, 
					char		*args) {

  char next_word[MAX_INPUT_LENGTH];

  int type;
  int seq;

  MOB_TRIGGER *trig, *chain;

  ECOND_DATA *ec;

 /* Extract parms... */

  args = one_argument(args, next_word);

  type = ev_map_type_to_int(next_word);

  args = one_argument(args, next_word);

  seq = atoi(next_word);

 /* Validation... */

  if ( args[0] == '\0' ) {
    medit_trigger(ch, "");
    return base;
  }

  if ( type < 1
    || type >= MAX_WEV) {
    send_to_char("Invalid type!\r\n", ch);
    return base;
  }

  if ( seq < 1) {
    send_to_char("Invalid sequence number!\r\n", ch); 
    return base;
  }

 /* Now, see if there's an old trigger... */

  chain = find_trigger_chain(base, type);

  if (chain == NULL) {
    send_to_char("No events of that type defined!\r\n", ch);
    return base;
  }

  trig = find_trigger(chain, seq);

  if (trig == NULL) {
    send_to_char("No trigger with that sequence number defined!\r\n", ch);
    return base;  
  }

 /* Condition none means to remove all conditions... */

  if (!str_cmp(args, "none")) {

    while (trig->cond != NULL) {
      ec = trig->cond;
      trig->cond = ec->next;
      free_ec_norecurse(ec);
    }

    send_to_char("Trigger conditions removed.\r\n", ch);

    return base;
  }

 /* Now get the condition... */

  ec = create_econd(args, ch);

  if (ec == NULL) {
    return base;
  }

 /* Ok, now we add the condition... */

  ec->next = trig->cond;
  trig->cond = ec;

  send_to_char("Trigger condition added.\r\n", ch);

 /* All done... */

  return base;
}

/* Delete a trigger... */

MOB_TRIGGER *medit_trigger_del(	CHAR_DATA	*ch, 
				MOB_TRIGGER	*base, 
				char		*args) {

  char next_word[MAX_INPUT_LENGTH];

  int type;
  int seq;

  MOB_TRIGGER *junk, *chain;

 /* Extract parms... */

  args = one_argument(args, next_word);

  type = ev_map_type_to_int(next_word);

  args = one_argument(args, next_word);

  seq = atoi(next_word);

 /* Validation... */

  if ( args[0] != '\0' ) {
    medit_trigger(ch, "");
    return base;
  }

  if ( type < 1
    || type >= MAX_WEV) {
    send_to_char("Invalid type!\r\n", ch);
    return base;
  }

  if ( seq < 1) {
    send_to_char("Invalid sequence number!\r\n", ch); 
    return base;
  }

 /* Delete the old trigger... */

  chain = find_trigger_chain(base, type);
 
  if (chain != NULL) {

    junk = find_trigger(chain, seq);

    if (junk != NULL) {
      base = del_trigger(base, junk);
    }
  }

  send_to_char("Trigger deleted.\r\n", ch);

 /* All done... */

  return base;
}

/* Main parsing and dispatch routine... */

bool medit_trigger(CHAR_DATA *ch, char *args) {

  MOB_INDEX_DATA *pMob;

  char command[MAX_INPUT_LENGTH];
  char *parms;

 /* What are we editing... */

  EDIT_MOB(ch, pMob);

 /* Give help if needed... */ 

  if ( args[0] == '\0'
    || args[0] == '?' ) {
    send_to_char(
     "Syntax: trigger clist\r\n"
     "        trigger rlist\r\n"
     "        trigger cadd [type] [seq] [subtype] [script] [text]\r\n"
     "        trigger radd [type] [seq] [subtype] [script] [text]\r\n"
     "        trigger ccond [type] [seq] [condition]\r\n"
     "        trigger rcond [type] [seq] [condition]\r\n"
     "        trigger cdel [type] [seq]\r\n"
     "        trigger rdel [type] [seq]\r\n"
     "        trigger crenum\r\n"
     "        trigger rrenum\r\n", ch);

    return FALSE;
  }   

 /* Extract command and parms... */

  parms = one_argument(args, command);

 /* Ensure we have a trigger block... */

  if (pMob->triggers == NULL) {
    pMob->triggers = get_trs();
  } 

 /* List challange triggers... */

  if (!str_cmp(command, "clist")) {
    medit_trigger_list(ch, pMob->triggers->challange, parms);
    return FALSE;
  } 

 /* List reaction triggers... */

  if (!str_cmp(command, "rlist")) {
    medit_trigger_list(ch, pMob->triggers->reaction, parms);
    return FALSE;
  } 

 /* Add a challange trigger... */

  if (!str_cmp(command, "cadd")) {
    pMob->triggers->challange =
                     medit_trigger_add(ch, pMob->triggers->challange, parms);

    return TRUE;
  } 

 /* Add a reaction trigger... */

  if (!str_cmp(command, "radd")) {
    pMob->triggers->reaction = 
                     medit_trigger_add(ch, pMob->triggers->reaction, parms);

    return TRUE;
  } 

 /* Add a challange trigger condition... */

  if (!str_cmp(command, "ccond")) {
    pMob->triggers->challange =
                medit_trigger_add_cond(ch, pMob->triggers->challange, parms);

    return TRUE;
  } 

 /* Add a reaction trigger condition... */

  if (!str_cmp(command, "rcond")) {
    pMob->triggers->reaction = 
                 medit_trigger_add_cond(ch, pMob->triggers->reaction, parms);

    return TRUE;
  } 

 /* Delete a challange trigger... */

  if (!str_cmp(command, "cdel")) {
    pMob->triggers->challange =
                     medit_trigger_del(ch, pMob->triggers->challange, parms);

    return TRUE;
  } 

 /* Delete a reaction trigger... */

  if (!str_cmp(command, "rdel")) {
    pMob->triggers->reaction = 
                     medit_trigger_del(ch, pMob->triggers->reaction, parms);

    return TRUE;
  } 

 /* Renumber challange triggers... */

  if (!str_cmp(command, "crenum")) {
    pMob->triggers->challange = renum_triggers(pMob->triggers->challange);

    send_to_char("Challange triggers renumbered.\r\n", ch);
    return TRUE;
  } 

 /* Renumber reaction triggers... */

  if (!str_cmp(command, "rrenum")) {
    pMob->triggers->reaction = renum_triggers(pMob->triggers->reaction);

    send_to_char("Reaction triggers renumbered.\r\n", ch);
    return TRUE;
  } 

 /* Not matched, give help... */

  medit_trigger(ch, "");

 /* All done... */

  return FALSE;
}

