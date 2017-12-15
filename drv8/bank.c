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
#include "bank.h"
#include "olc.h"

/* External functions... */

DECLARE_DO_FUN(do_yell);


static SHARE_DATA *share_list = NULL;

/* Check for interest payments... */

void check_interest(CHAR_DATA *teller, ACCOUNT *ap, bool save);

/* Process balance... */

void do_balance(CHAR_DATA *teller, CHAR_DATA *cust, char *bank_name, char *account_name) {
  BANK *bp;
  ACCOUNT *ap;
  char buf[MAX_STRING_LENGTH];

 /* Locate the bank... */

  bp = find_bank(bank_name, FALSE);

  if (bp == NULL) {
    do_say(teller, "We do not have any customers!");
    return;
  } 
   
 /* Locate the account... */

  ap = find_account(bp, account_name, FALSE);

  if (ap == NULL) {
    do_say(teller, "You do not have an account with us!");
    return;
  }  

 /* Check the interest... */

  check_interest(teller, ap, TRUE); 

 /* Report time... */

  sprintf(buf, "You have {Y%d{g {rCopper{x coins on deposit, %s.", ap->gold, ap->holder);
  do_say(teller, buf);
  return;
} 

/* Process deposit... */

void do_deposit(CHAR_DATA *teller, CHAR_DATA *cust, char *ammount, int currency, char *bank_name, char *account_name) {
BANK *bp;
ACCOUNT *ap;
int amt;
float modi, modi2;
char buf[MAX_STRING_LENGTH];

 /* Check the cash first... */

  if (ammount[0] == '\0') {
    do_say(teller, "How much do you want to deposit?");
    return;
  }

  amt = atoi(ammount);

  if (amt <= 0 || amt > 50000000) {
    do_say(teller, "Please, be sensible. You can't deposit that much.");
    return;
  }

  if (amt > cust->gold[currency]) {
    do_say(teller, "Ummm. You seem to be a bit short.");
    return;
  }

 /* Locate the bank... */

  bp = find_bank(bank_name, TRUE);

  if (bp == NULL) {
    do_say(teller, "We do not have any customers!");
    return;
  } 
    
 /* Locate the account... */

  ap = find_account(bp, account_name, TRUE);

  if (ap == NULL) {
    do_say(teller, "You do not have an account with us!");
    return;
  }
  
 /* Check the interest... */

  check_interest(teller, ap, FALSE); 
 
 /* Now we put some money in it... */

  if ((modi = get_currency_modifier(teller->in_room->area, currency)) == 0) {
        send_to_char("We don't accept that kind of money here.\r\n", cust);
        return;
  }
  modi2 = get_currency_modifier(teller->in_room->area, 0);

  cust->gold[currency] -= amt;

  if (currency == find_currency(teller) || currency == 0) amt = (int) (amt * modi / modi2);
  else amt = (int) (amt *modi / modi2 * 0.8);

  ap->gold    += amt; 
  sprintf(buf, "Thank you, your {Y%d{g {rCopper{x coins have been depositied.", amt);
  do_say(teller, buf);
  if (ap->gold > 999999999) {
        ap->gold = 999999999;
        send_to_char("You have reached the maximum balance.\r\n", cust);
  }
  save_banks();

  return;
} 


void do_deposit_raw(CHAR_DATA *teller, int amt, int currency, char *bank_name, char *account_name) {
BANK *bp;
ACCOUNT *ap;
float modi, modi2;

  if (amt <= 0) return;

  bp = find_bank(bank_name, TRUE);
  if (bp == NULL) return;

  ap = find_account(bp, account_name, TRUE);
  if (ap == NULL) return;

  if ((modi = get_currency_modifier(teller->in_room->area, currency)) == 0) return;
  modi2 = get_currency_modifier(teller->in_room->area, 0);

  if (currency == find_currency(teller) || currency == 0) amt = (int) (amt *modi /modi2);
  else amt = (int) (amt * modi /modi2 * 0.8);

  ap->gold += amt; 
  if (ap->gold > 999999999) ap->gold = 999999999;

  save_banks();
  return;
} 


/* Process withdraw... */

void do_withdraw(CHAR_DATA *teller, CHAR_DATA *cust, char *ammount, int currency, char *bank_name, char *account_name) {
BANK *bp;
ACCOUNT *ap;
char buf[MAX_STRING_LENGTH];
int amt, amt2;
float modi, modi2;

 /* Check the cash first... */

  if (ammount[0] == '\0') {
    do_say(teller, "How much do you want to withdraw?");
    return;
  }

  amt = atoi(ammount);

  if (amt <= 0 || amt > 50000000) {
    do_say(teller, "Please, be sensible. You can't withdraw that much.");
    return;
  }

 /* Locate the bank... */

  bp = find_bank(bank_name, FALSE);

  if (bp == NULL) {
    do_say(teller, "We do not have any customers!");
    return;
  } 
   
 /* Locate the account... */

  ap = find_account(bp, account_name, FALSE);

  if (ap == NULL) {
    do_say(teller, "You do not have an account with us!");
    return;
  }
  
 /* Check the interest... */

  check_interest(teller, ap, FALSE); 

 /* Givng money out time... */
 
  if ((modi = get_currency_modifier(teller->in_room->area, currency)) == 0) {
        send_to_char("We don't have that kind of money here.\r\n", cust);
        return;
  }
  modi2 = get_currency_modifier(teller->in_room->area, 0);

  if (currency == find_currency(teller) || currency == 0) amt2 = (int) (amt *modi /modi2);
  else amt2 = (int) (amt *modi /modi2 * 1.25);

  if (amt2 > ap->gold) { 
    do_say(teller, "You don't have that much in your account.");
    return;
  }

  cust->gold[currency] += amt;
  ap->gold -= amt2;
  if (currency == 0) sprintf(buf, "Thank you, {Y%d{g {rCopper{x coins have been withdrawn from your account.", amt);
  else sprintf(buf, "Thank you, {Y%d{g {rCopper{x coins have been withdrawn and changed into %s.", amt2, flag_string(currency_type, currency));
  do_say(teller, buf);

  save_banks();
  return;
} 


void do_transgold(CHAR_DATA *teller, CHAR_DATA *cust, char *other_name , char *bank_name, char *account_name, char *ammount) {
  BANK *bp;
  ACCOUNT *ap;
  ACCOUNT *ap2;
  char buf[MAX_STRING_LENGTH];
  int amt;

  if (ammount[0] == '\0') {
    do_say(teller, "How much do you want to transfer?");
    return;
  }

  amt = atoi(ammount);

  if (amt <= 0 || amt > 50000000) {
    do_say(teller, "Please, be sensible. You can't transfer that much.");
    return;
  }

  bp = find_bank(bank_name, FALSE);
  if (bp == NULL) {
    do_say(teller, "We do not have any customers!");
    return;
  } 
   
  ap = find_account(bp, account_name, FALSE);
  if (ap == NULL) {
    do_say(teller, "You do not have an account with us!");
    return;
  }
 
  ap2 = find_account(bp, capitalize(other_name), FALSE);
  if (ap2 == NULL) {
    do_say(teller, "The other account does not exist!");
    return;
  }
 
  check_interest(teller, ap, FALSE); 

  if (amt > ap->gold) { 
    do_say(teller, "You don't have that much in your account.");
    return;
  }

  ap->gold -= amt;
  ap2->gold += amt;

  do_say(teller, "The money has been transferred.");
  save_banks();
  
  sprintf(buf, "%d$ have been transferred from %ss account to yours.\r\nYour new balance is %d$.\r\n", amt, account_name, ap2->gold);
  make_note ("Observer", bank_name, other_name, "Bank Transfer", 7, buf);

  return;
} 


/* Bank command... */

void do_bank(CHAR_DATA *cust, char *argument) {
  CHAR_DATA *teller;
  SHOP_DATA *pShop;
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  bool bank = FALSE;
  bool stock = FALSE;
  bool change = FALSE;
  bool material = FALSE;
  int currency =0;
	
  if (IS_NPC(cust)
  || cust->desc->original) {
    send_to_char("You can't have an account!\r\n", cust);
    return;
  }  

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);
  argument = one_argument(argument, arg4);

  if ( (arg1[0] == '\0')
  || (!str_cmp(arg1,"help"))) {
    send_to_char("\r\nBank Commands:{x\r\n\r\n",cust);
    send_to_char("  bank balance\r\n", cust);
    send_to_char("  bank desposit <amount> [currency]\r\n", cust);
    send_to_char("  bank withdraw <amount> [currency]\r\n", cust);
    send_to_char("  bank transfer <account> <amount>\r\n", cust);
    send_to_char("  bank change all/<amount> <currency> <currency>\r\n", cust);
    send_to_char("  bank material <obj>\r\n", cust);
    send_to_char("  bank share\r\n", cust);
    send_to_char("  bank help\r\n", cust);
    send_to_char("\r\nInterest is 1% per game month.\r\n", cust);
    return;
  }

 /* Safty check... */

  if (cust->in_room == NULL) {
    send_to_char("This is not a bank!\r\n", cust);
    return;
  }

 /* See if we can find a teller... */ 

  teller = find_teller(cust);
  if ( teller == NULL ) return;

  pShop = teller->pIndexData->pShop;
  if (pShop != NULL ) {
      int i;

      for(i = 0; i < MAX_TRADE; i++) {
          if (pShop->buy_type[i] == 999) bank = TRUE;
          if (pShop->buy_type[i] == 997) stock = TRUE;
          if (pShop->buy_type[i] == 996) material = TRUE;
          if (pShop->buy_type[i] == 995) change = TRUE;
      }
  }

 /* Yep, so say hi... */ 

  sprintf(buf, "Welcome to the {w%s{g", cust->in_room->name);
  do_say(teller, buf);

  if (!str_cmp(arg1,"balance")) {
    if (!bank) {
         send_to_char("There is no bank here.\r\n", cust);
         return;
    }
    do_balance(teller, cust, cust->in_room->name, cust->name);

  } else if (!str_cmp(arg1, "deposit")) {
    if (!bank) {
         send_to_char("There is no bank here.\r\n", cust);
         return;
    }
    if (arg3[0] != '\0') {
        currency = flag_value(currency_accept, arg3);
        if (currency < 0) {
           do_say(teller, "Deposit what?");
           return;
        }
    } else {
        currency = 0;
    }
    do_deposit(teller, cust, arg2, currency, cust->in_room->name, cust->name);

  } else if (!str_cmp(arg1, "withdraw")) {
    if (!bank) {
         send_to_char("There is no bank here.\r\n", cust);
         return;
    }
    if (arg3[0] != '\0') {
        currency = flag_value(currency_accept, arg3);
        if (currency < 0) {
           do_say(teller, "Deposit what?");
           return;
        }
    } else {
        currency = 0;
    }
    do_withdraw(teller, cust, arg2, currency, cust->in_room->name, cust->name);

  } else if (!str_cmp(arg1, "transfer")) {
    if (!bank) {
         send_to_char("There is no bank here.\r\n", cust);
         return;
    }
    do_transgold(teller, cust, arg2, cust->in_room->name, cust->name, arg3);

  } else if (!str_cmp(arg1, "share")) {
    if (!stock) {
         send_to_char("There is no stock exchange here.\r\n", cust);
         return;
    }
    do_stock(cust, arg2, arg3, argument);

  } else if (!str_cmp(arg1, "material")) {
    if (!material) {
         send_to_char("There is no raw material market here.\r\n", cust);
         return;
    }
    do_material(cust, arg2);

  } else if (!str_cmp(arg1, "change")) {
    if (!change) {
         send_to_char("There is no money exchange here.\r\n", cust);
         return;
    }
    do_change(cust, arg2, arg3, arg4);
  } else {
    do_say(teller, "Sorry, I don't know how to do that.");
    send_to_char("\r\nBank Commands:{x\r\n\r\n",cust);
    send_to_char("  bank balance\r\n", cust);
    send_to_char("  bank desposit <amount> [currency]\r\n", cust);
    send_to_char("  bank withdraw <amount> [currency]\r\n", cust);
    send_to_char("  bank transfer <account> <amount>\r\n", cust);
    send_to_char("  bank change all/<amount> <currency> <currency>\r\n", cust);
    send_to_char("  bank material <obj>\r\n", cust);
    send_to_char("  bank share\r\n", cust);
    send_to_char("  bank help\r\n", cust);
    send_to_char("\r\nInterest is 1% per game month.\r\n", cust);
  }

  return;
} 

/* File/Data routines... */

static BANK *banks;

/* Load the banks... */ 

void load_banks() {
  FILE *fp;
  BANK *bp, *nbp;
  ACCOUNT *ap, *nap;
  char code;
  char *name;
  bool done;

 /* Initialize incase of problems... */

  banks = NULL; 
  bp = NULL;
  ap = NULL;

 /* Find the banks file... */

  fp = fopen(BANK_FILE, "r");

  if (fp == NULL) {
    log_string("No bank records!");
    return;
  }

 /* Read through it and see what we've got... */
 
  done = FALSE;

  while(!done) {

    code = fread_letter(fp);

    switch (code) {

    /* Banks have a B... */

      case 'B':

      /* Get memory... */ 

        nbp = (BANK *) alloc_mem(sizeof(*nbp));
        
      /* Append to chain... */ 

        if (banks == NULL) { 
          banks = nbp;
        } else {
          bp->next = nbp;
        }

        name = fread_string(fp); 

       /* Initialize data... */ 

        nbp->next = NULL;
        nbp->accounts = NULL;
        nbp->name = str_dup(name);
 
        bp = nbp; 
        ap = nbp->accounts;

        break;

     /* Accounts start with an A... */

      case 'A':

       /* Check we have an active account... */ 

        if (bp == NULL) {
          bug("Sequence error in bank file - A before B", 0);
          done = TRUE;
          break;
        } 

       /* Get some memory... */ 

        nap = (ACCOUNT *) alloc_mem(sizeof(*nap));

       /* Fill in the details... */

        nap->gold = fread_number(fp);
        nap->next = NULL;
 
        nap->last_day = fread_number(fp);
        nap->last_month = fread_number(fp);
        nap->last_year = fread_number(fp);

        name = fread_string(fp);
 
        nap->holder = str_dup(name);
              
       /* Splice... */

        if (ap == NULL) {
          bp->accounts = nap;
        } else {
          ap->next = nap;
        } 

        ap = nap;

        break; 

     /* File ends with an E */

      case 'E':
        done = TRUE;
        break; 

     /* Anything else is an error... */

      default:
        bug("Unexpected code '%c' in bank file!", code);
        done = TRUE;
    }
  }

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* All done... */

  log_string("...banks loaded"); 

  return;
}

/* Save the banks... */ 

void save_banks() {
  FILE *fp;
  BANK *bp;
  ACCOUNT *ap;
  char buf[MAX_STRING_LENGTH];

 /* Find the banks file... */

  fp = fopen(BANK_TEMP, "w+");

  if (fp == NULL) {
    bug("Unable to open bank file for save!", 0);
    return;
  }

 /* Write out the banks... */

  bp = banks;

  while (bp != NULL) {

    fprintf(fp,"B %s~\n",bp->name);

    ap = bp->accounts;

    while (ap != NULL) {
  
      if (ap->gold > 0) {
        fprintf(fp, "A %d %d %d %d %s~\n", ap->gold, ap->last_day, ap->last_month, ap->last_year, ap->holder);
      }

      ap = ap->next;
    }

    bp = bp->next;
  }

 /* Write trailing E... */ 

  fprintf(fp, "E\n");

 /* Close it now we're finished with it... */ 

  fclose(fp);

 /* Rename over the old file... */

  sprintf(buf, "mv %s %s", BANK_TEMP, BANK_FILE);
 
  system(buf);

 /* All done... */

  log_string("Banks saved...");
 
  return;
}

/* Find a specific bank... */

BANK *find_bank(char *name, bool make) {

  BANK *obp, *bp;

 /* Safty check... */

  if (name[0] == '\0') {
    return NULL;
  }

 /* Search.. */

  bp = banks;
  obp = NULL;

  while (bp != NULL) {

    if (!str_cmp(bp->name, name)) {
      return bp;
    }

    obp = bp;
    bp  = bp->next;
  }

 /* Make if not found? */ 

  if (make) {
    
   /* Get memory... */ 

    bp = (BANK *) alloc_mem(sizeof(*bp));
        
   /* Append to chain... */ 

    if (banks == NULL) { 
      banks = bp;
    } else {
      obp->next = bp;
    }

   /* Initialize data... */ 

    bp->next = NULL;
    bp->accounts = NULL;
    bp->name = str_dup(name);
 
   /* All done, created new bank... */ 

    return bp;    
  }

 /* All done, not found, not made... */

  return NULL;
} 

/* Find a specific account... */

ACCOUNT *find_account(BANK *bp, char *name, bool make) {

  ACCOUNT *oap, *ap;

 /* Safty check... */ 

  if (name[0] == '\0') {
    return NULL;
  }

  if (bp == NULL) {
    return NULL;
  }

 /* Search... */

  ap = bp->accounts;
  oap = NULL;

  while (ap != NULL) {

    if (!str_cmp(ap->holder, name)) {
      return ap;
    }

    oap = ap;
    ap = ap->next;
  }

 /* Make if not found... */ 

  if (make) {

    ap = (ACCOUNT *) alloc_mem(sizeof(*ap));

   /* Splice */

    if (bp->accounts == NULL) {
      bp->accounts = ap;
    } else {
      oap->next = ap;
    }     

   /* Initialize... */

    ap->next = NULL;
    ap->holder = str_dup(name);
    ap->gold = 0; 
   
    ap->last_day   = time_info.day;
    ap->last_month = time_info.month;
    ap->last_year  = time_info.year;

   /* All done, created new account... */

    return ap;
  }

 /* Not done, not made... */

  return NULL;
}

/* Calculate interest... */

void check_interest(CHAR_DATA *teller, ACCOUNT *ap, bool save) {
int ddays;
char buf[MAX_STRING_LENGTH];
int i;
long interest = 0;
long highint = 0;
long base = 0;
long highbase = 0;

  if (ap == NULL) return;

 /* How long since last check... */  

  ddays  =       (time_info.month - ap->last_month);
  ddays +=  12 * (time_info.year  - ap->last_year);

 /* Do we need to fix the system time? */

  if (ddays < 0) {
    time_info.day = ap->last_day;
    time_info.month = ap->last_month;
    time_info.year = ap->last_year;

    bug("Time_info fixed from interest records!", 0);
  }

 /* There is a limit though (prevents overflows after timewarps)... */

  if (ddays > 36) {
    ddays = 36;
    do_say(teller, "My this is an old account.");
  }

 /* Pay 1% per game month (32 days) (=> 15% per game year) */ 
 /* Original code paid per day, that explains old var names... */

  if (ddays > 0) {

    base = (ap->gold % 1000000);

    highbase = (ap->gold - base)/1000;
 
    for(i = 0; i < ddays; i++) {
      interest += base + ((interest + 30)/100);
      highint += highbase + ((highint + 30)/100);
    }

    interest = (interest + 30)/100;
    highint = (highint + 30)/100;

    interest += (highint * 1000);

    if (teller->pIndexData->pShop) {
        int diffper = (teller->pIndexData->pShop->profit_buy - teller->pIndexData->pShop->profit_sell)/20;
        diffper =10 - URANGE(0, diffper, 5);
        interest = interest * diffper / 10;
    }
 
    if ( interest > 0 
      && highint >= 0 ) {
      sprintf(buf, "You are due %d months interest.", ddays);
      do_say(teller, buf);

      sprintf(buf, "This comes to {Y%ld{g {rCopper{x coins.", interest);
      do_say(teller, buf);

      do_say(teller, "They have been depositied in your account.");

      ap->gold += interest;

      ap->last_day   = time_info.day;
      ap->last_month = time_info.month;
      ap->last_year  = time_info.year;

      if (save) {
        save_banks(); 
      } 
    } 
  }

  return;
}

/* Utility routines... */

/* Find a bank teller... */
 
/* A bank teller is a shop keeper without any trade items... */

CHAR_DATA *find_teller( CHAR_DATA *ch ) {
  CHAR_DATA *teller;
  SHOP_DATA *pShop;
  int i;
  pShop = NULL;
  bool found = FALSE;

  for ( teller = ch->in_room->people; teller; teller = teller->next_in_room ) {
    found = FALSE;
    if ( IS_NPC(teller)) {
      pShop = teller->pIndexData->pShop;
      if (pShop != NULL ) {
          for(i = 0; i < MAX_TRADE; i++) {
              if (pShop->buy_type[i] == 999 || pShop->buy_type[i] == 995) {
                 found = TRUE;
                 break;
              }
          }

          if (found) break;
      }
    }
  }

  if (!found) {
    send_to_char( "There isn't a bank teller here.\r\n", ch );
    return NULL;
  }

 /* We don't trade if we're closed... */

  if ( time_info.hour < pShop->open_hour ) {
      do_say( teller, "Sorry, I am closed. Come back later." );
      return NULL;
  }
    
  if ( time_info.hour > pShop->close_hour ) {
      do_say( teller, "Sorry, I am closed. Come back tomorrow." );
      return NULL;
  }

 /* Don't trade with hidden or invisible people. */

  if ( !can_see( teller, ch ) ) {
      do_say( teller, "What? Whay said that? Who's there?" );
      return NULL;
  }

 /* Ok, we can go ahead and trade... */
  return teller;
}

/* Delete all of a players accounts... */

void delete_accounts(CHAR_DATA *ch) {
  
  BANK *bp;

  ACCOUNT *ap;

 /* Search all accounts in all banks... */

  bp = banks;

  while (bp != NULL) {

    ap = bp->accounts;

    while (ap != NULL) {
 
     /* Wipe out whatever savings they may have... */
 
      if (!str_cmp(ap->holder, ch->name)) {
        ap->gold = 0;
      }

      ap = ap->next;
    }

    bp = bp->next;
  }

  return;
}


SHARE_DATA *get_share(char *share_name) {
  SHARE_DATA *share;

  for(share = share_list; share != NULL; share = share->next) {
    if (share == NULL) break;

    if (share->name[0] == share_name[0]) {
      if (!str_cmp(share->name, share_name)) break;
    } 

  }
  return share;
}



int get_share_id(char *name) {
  SHARE_DATA *share;

  share = get_share(name);

  if (share == NULL) {
    return SHARE_UNDEFINED;
  }

  return share->id; 
}



SHARE_DATA *get_share_by_id(int id) {
  SHARE_DATA *share;

  if (id == SHARE_UNDEFINED) return NULL;

  for(share = share_list; share != NULL; share = share->next) {
    if (share == NULL) break;
    if (share->id == id) break;
  }

  return share;
}


void load_shares() {
  SHARE_DATA *new_share = NULL;
  FILE *fp;
  char *kwd;
  char code;
  bool done;
  bool ok;
  char *name;
  int pid, i;
  char buf[MAX_STRING_LENGTH];

  fp = fopen(SHARES_FILE, "r");

  if (fp == NULL) {
    log_string("No shares file!");
    exit(1);
    return;
  }
 
  done = FALSE;
  pid = SHARE_UNDEFINED;

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
 
      case 'B':
        if (!str_cmp(kwd, "Balance")) {
          ok = TRUE;

          for (i=0; i<32; i++) {
              new_share->balance[i] = fread_number(fp);
          }
        }
        break;

      case 'E':
        if (!str_cmp(kwd, "Emission")) {
          ok = TRUE;

          new_share->emission = fread_number(fp);
          new_share->emission = UMAX(new_share->emission, 0);
        }

        if (!str_cmp(kwd, "END")) {
          ok = TRUE;
          done = TRUE;
        }
        break; 

      case 'N':
        if (!str_cmp(kwd, "Npc")) {
          ok = TRUE;

          new_share->on_npc = fread_number(fp);
          new_share->on_npc = UMAX(new_share->on_npc, 0);
        }
        break;

      case 'O':
        if (!str_cmp(kwd, "Ok")) {
          ok = TRUE;

          name = fread_word(fp);
          if (!str_cmp(name, "YES")) new_share->ok = TRUE;
          else new_share->ok = FALSE;
        }

        if (!str_cmp(kwd, "Owner")) {
          ok = TRUE;

          new_share->owner = strdup(fread_word(fp));
        }
        break;

      case 'R':
        if (!str_cmp(kwd, "Raw")) {
          ok = TRUE;

          new_share->raw = fread_number(fp);
          new_share->raw = UMAX(new_share->raw, 0);
        }
        break;

      case 'S':
        if (!str_cmp(kwd, "SHARE")) {
          ok = TRUE;
          name = fread_word(fp); 
          if (name[0] == '\0') {
            bug("Unnammed share in shares file!", 0);
            exit(1);
          }

          sprintf(buf, "Loading shares '%s'...", name);
          log_string(buf);

          if (new_share == NULL) {
            new_share = (SHARE_DATA *) alloc_perm( sizeof( *new_share));
            share_list = new_share;
            new_share->next = NULL;
          } else { 
            new_share->next = (SHARE_DATA *) alloc_perm( sizeof( *new_share));
            new_share = new_share->next;
            new_share->next = NULL;
          }

          if (new_share == NULL) {
            bug("Failed to allocate memory for new share!", 0);
            exit(1);
          }

          new_share->name = strdup(name);  
          new_share->desc = strdup(fread_string(fp));
          new_share->id = ++pid;
          new_share->emission = 0;
          new_share->on_npc = 0;
          new_share->raw = 0;
          new_share->ok = FALSE;
          new_share->owner = NULL;

          for (i=0; i<32; i++) {
               new_share->balance[i] = 0;
          }
          break; 

        }

     /* Anything else is an error... */

      default:
        break;
    }

    if (!ok) {
      sprintf(buf, "Unrecognized keyword in shares file: %s", kwd);
      bug(buf, 0); 
      exit(1);
    }
 
  }

  fclose(fp);
  log_string("...shares loaded");
  return;
}

void save_shares() {
  SHARE_DATA *share;
  FILE *fp;
  int i;

  fp = fopen(SHARES_FILE, "w");

  for (share = share_list; share; share = share->next) {
       fprintf(fp, "SHARE '%s'\n", share->name);
       fprintf(fp, "%s~\n", share->desc);
       if (share->owner) fprintf(fp, "Owner '%s'\n", share->owner);
       if (share->ok) fprintf(fp, "Ok YES\n");
       else fprintf(fp, "Ok NO\n");
       fprintf(fp, "Emission %d\n", share->emission);
       fprintf(fp, "Npc %d\n", share->on_npc);
       fprintf(fp, "Raw %d\n", share->raw);
       fprintf(fp, "Balance ");
       for (i=0; i<32; i++) {
            fprintf(fp, "%d ", share->balance[i]);
       }
       fprintf(fp, "\n");
  }
  fprintf(fp, "END\n");
  fclose(fp);
  return;
}


void do_stock(CHAR_DATA *ch, char *arg2, char *arg3, char *arg) {
SHARE_DATA *share;
OBJ_DATA *obj;
char buf[MAX_STRING_LENGTH];
char outbuf[10*MAX_STRING_LENGTH];

  if ( (arg2[0] == '\0')
  || (!str_cmp(arg2,"help"))) {
    send_to_char("\r\nStock Exchange Commands:{x\r\n\r\n",ch);
    send_to_char("  bank share balance\r\n", ch);
    send_to_char("  bank share info [name]\r\n", ch);
    send_to_char("  bank share buy [name] [ammount]\r\n", ch);
    send_to_char("  bank share sell [name] [ammount]\r\n", ch);
    if (IS_HERO(ch)) send_to_char("  bank share emission [name] [ammount]\r\n", ch);
    if (IS_IMMORTAL(ch)) {
         send_to_char("  bank share allow [name] [0/1]\r\n", ch);
         send_to_char("  bank share desc [name]\r\n", ch);
    }
    send_to_char("  bank share help\r\n", ch);
    return;
  }

  if (!str_cmp(arg2, "balance")) {
      sprintf(outbuf, "   {gStock Exchange:{x\r\n");
      strcat(outbuf, "{c-----------------------------------------{x\r\n");

      for(share = share_list; share; share = share->next) {
            if (share->ok
            || IS_IMMORTAL(ch)) {
                 if (IS_IMMORTAL(ch)) {
                       if (share->ok) sprintf(buf, "{g%25s{x    %5d $    %5d\r\n", share->name, calculate_value(share), share->emission);
                       else sprintf(buf, "{r%25s{x    %5d $    %5d\r\n", share->name, calculate_value(share), share->emission);
                 } else {
                       sprintf(buf, "{g%25s{x    %5d $\r\n", share->name, calculate_value(share));
                 }
            }
            strcat(outbuf, buf);
      }
      send_to_char(outbuf, ch);
      return;

  } else if (!str_cmp(arg2, "buy")) {
      SHARE_DATA *share = get_share(arg3);
      EXTRA_DESCR_DATA *ed;
      int n, price;

      if (!share) {
          send_to_char("That kind of share doesn't exist.\r\n", ch);
          send_to_char("SYNTAX:  bank share buy [name] [ammount]\r\n", ch);
          return;
      }
      
      if (!share->ok) {
          send_to_char("Those shares aren't allowed yet - be patient..\r\n", ch);
          return;
      }

      n = atoi(arg);
      if (n<1) {
          send_to_char("That doesn't make sense.\r\n", ch);
          send_to_char("SYNTAX:  bank share buy [name] [ammount]\r\n", ch);
          return;
      }

      if (time_info.hour < 8
      || time_info.hour > 12
      || (time_info.day + 1) % 7 == 0
      || (time_info.day + 1) % 7 == 6) {
          send_to_char("Stock exchange is closed.\r\n", ch);
          send_to_char("Monday to Friday 8-12\r\n", ch);
          return;
      }

      if (n> share->emission) {
          send_to_char("There aren't that many shares availiable.\r\n", ch);
          n = share->emission;
          if (n<1) return;
          sprintf_to_char(ch,"Changed to %d.\r\n", n);
      }

      share->balance[time_info.day] += (n/2 +1);
      price = calculate_value(share);
      share->balance[time_info.day] -= (n/2 +1);
      price *= n;
      price = price * 30 / 29;      

      if (ch->gold[0] < price) {
          send_to_char("You can't afford that.\r\n", ch);
          return;
      }

      ch->gold[0] -=price;
      share->emission -= n;
      share->balance[time_info.day] += n;

      obj = ch->carrying;
      while (obj != NULL
      && ( obj->item_type != ITEM_SHARE
          || !can_see_obj(ch, obj)
          || obj->value[0] != share->id)) {
              obj = obj->next_content;
      }

      if (obj) {
          obj->value[1] +=n;
      } else {
          char buf[MAX_STRING_LENGTH];

          obj = create_object(get_obj_index(OBJ_SHARE), 0);
          obj->item_type = ITEM_SHARE;
          SET_BIT(obj->extra_flags, ITEM_NO_SAC);
          SET_BIT(obj->extra_flags, ITEM_NOPURGE);

          sprintf(buf, "A bundle of documents lies here.\n");
          free_string(obj->description);
          obj->description = strdup(buf);
          sprintf(buf, "%s shares", share->name);
          free_string(obj->short_descr);
          obj->short_descr = strdup(buf);
          sprintf(buf, "%s documents shares", share->name);
          free_string(obj->name);
          obj->name = strdup(buf);
           
          obj->value[0] = share->id;
          obj->value[1] = n;
          
          ed	= new_extra_descr();
          sprintf(buf, "%s documents shares", share->name);
          ed->keyword = strdup(buf);
          ed->description = strdup(share->desc);
          ed->next = obj->extra_descr;
          obj->extra_descr = ed;
          obj_to_char(obj, ch);
      }
      sprintf_to_char(ch, "%d shares of %s bought for %d $.\r\n", n, share->name, price);
      return;

  } else if (!str_cmp(arg2, "sell")) {
      SHARE_DATA *share;
      int n, price;

      obj = ch->carrying;
      while (obj != NULL
      && (obj->item_type != ITEM_SHARE
          || !can_see_obj(ch, obj)
          || !is_name(arg3, obj->name))) {
              obj = obj->next_content;
      }

      if (!obj) {
          send_to_char("You don't have those shares.\r\n", ch);
          send_to_char("SYNTAX:  bank share sell [name] [ammount]\r\n", ch);
          return;
      }
      
      n = atoi(arg);
      if (n<1) {
          send_to_char("That doesn't make sense.\r\n", ch);
          send_to_char("SYNTAX:  bank share sell [name] [ammount]\r\n", ch);
          return;
      }

      if (time_info.hour < 8
      || time_info.hour > 12
      || (time_info.day + 1) % 7 == 0
      || (time_info.day + 1) % 7 == 6) {
          send_to_char("Stock exchange is closed.\r\n", ch);
          send_to_char("Monday to Friday 8-12\r\n", ch);
          return;
      }

      if (n > obj->value[1]) {
          send_to_char("You haven't got that many shares availiable.\r\n", ch);
          n = obj->value[1];
          if (n<1) return;
          sprintf_to_char(ch,"Changed to %d.\r\n", n);
      }

      share = get_share_by_id(obj->value[0]);     
      if (!share) return;

      if (!share->ok) {
          send_to_char("Those shares aren't allowed yet - be patient..\r\n", ch);
          return;
      }

      share->balance[time_info.day] -= (n/2 +1);
      price = calculate_value(share);
      share->balance[time_info.day] += (n/2 +1);
      price *= n;
      price = price * 29 / 30;
      
      ch->gold[0] +=price;
      share->emission += n;
      share->balance[time_info.day] -= n;

      if (n < obj->value[1]) {
          obj->value[1] -= n;
      } else {
          obj_from_char(obj);
          extract_obj(obj);
      }
      sprintf_to_char(ch, "%d shares of %s sold for %d $.\r\n", n, share->name, price);
      return;

  } else if (!str_cmp(arg2, "emission")
  && IS_HERO(ch)) {
      SHARE_DATA *share;
      SHARE_DATA *new_share;
      OBJ_DATA *obj;
      EXTRA_DESCR_DATA *ed;
      int n, price;

      n = atoi(arg);
      if (n<500) {
          send_to_char("500 shares is the minimum for an emission.\r\n", ch);
          return;
      }

      share = get_share(arg3);
      if (share) {

           if (!share->ok) {
              send_to_char("Those shares aren't allowed yet - be patient..\r\n", ch);
              return;
           }

           if (str_cmp(share->owner, ch->name)) {
              send_to_char("This is not your company.\r\n", ch);
              return;
           }

           price = calculate_value(share) * n * 3 / 2;
           if (ch->gold[0] < price) {
              send_to_char("You can't afford.\r\n", ch);
              return;
           }

           ch->gold[0] -= price;

           obj = ch->carrying;
           while (obj != NULL
           && ( obj->item_type != ITEM_SHARE
              || !can_see_obj(ch, obj)
              || obj->value[0] != share->id)) {
                  obj = obj->next_content;
           }

           if (obj) {
               obj->value[1] +=n;
           } else {
               char buf[MAX_STRING_LENGTH];

               obj = create_object(get_obj_index(OBJ_SHARE), 0);
               obj->item_type = ITEM_SHARE;
               SET_BIT(obj->extra_flags, ITEM_NO_SAC);
               SET_BIT(obj->extra_flags, ITEM_NOPURGE);

               sprintf(buf, "A bundle of documents lies here.\n");
               free_string(obj->description);
               obj->description = strdup(buf);
               sprintf(buf, "%s shares", share->name);
               free_string(obj->short_descr);
               obj->short_descr = strdup(buf);
               sprintf(buf, "%s documents shares", share->name);
               free_string(obj->name);
               obj->name = strdup(buf);
           
               obj->value[0] = share->id;
               obj->value[1] = n;
          
               ed	= new_extra_descr();
               sprintf(buf, "%s documents shares", share->name);
               ed->keyword = strdup(buf);
               ed->description = strdup(share->desc);
               ed->next = obj->extra_descr;
               obj->extra_descr = ed;
               obj_to_char(obj, ch);
           }
           sprintf_to_char(ch, "%d shares of %s produced for %d $.\r\n", n, share->name, price);
      } else {
           char buf[MAX_STRING_LENGTH];
           int i;

           price = 500 * n * 3 / 2;
           if (ch->gold[0] < price) {
              send_to_char("You can't afford.\r\n", ch);
              return;
           }
           ch->gold[0] -=price;

           for (share = share_list; share->next; share = share->next);
           share->next = (SHARE_DATA *) alloc_perm( sizeof( *share));
           new_share = share->next;
           new_share->next = NULL;

           new_share->name = strdup(arg3);
           if (IS_IMMORTAL(ch)) new_share->owner = NULL;
           else new_share->owner = strdup(ch->name);
           new_share->emission = 0;
           new_share->on_npc = 0;
           new_share->raw = 500;
           new_share->ok = FALSE;
           new_share->id = share->id++;

           for (i=0; i<32; i++) {
               new_share->balance[i] = 0;
           }

           obj = create_object(get_obj_index(OBJ_SHARE), 0);
           obj->item_type = ITEM_SHARE;
           SET_BIT(obj->extra_flags, ITEM_NO_SAC);
           SET_BIT(obj->extra_flags, ITEM_NOPURGE);

           sprintf(buf, "A bundle of documents lies here.\n");
           free_string(obj->description);
           obj->description = strdup(buf);
           sprintf(buf, "%s shares", new_share->name);
           free_string(obj->short_descr);
           obj->short_descr = strdup(buf);
           sprintf(buf, "%s documents shares", new_share->name);
           free_string(obj->name);
           obj->name = strdup(buf);
           
           obj->value[0] = share->id;
           obj->value[1] = n;
          
           obj_to_char(obj, ch);
           sprintf_to_char(ch, "%d shares of %s produced for %d $.\r\n", n, share->name, price);
           send_to_char("Emission scheduled.\r\n", ch);
           sprintf(buf, "Share %s(%d) needs to be checked and allowed.\n", new_share->name, new_share->id);
           make_note ("Immortals", "Stock Exchange", "imm", "new shares", 10, buf);
      }
      save_shares();
      return;
    
  } else if (!str_cmp(arg2, "allow")
  && IS_IMMORTAL(ch)) {
      SHARE_DATA *share = get_share(arg3);
      int i;
      
      if (!share) {
          send_to_char("That kind of share doesn't exist.\r\n", ch);
          send_to_char("SYNTAX:  bank share allow [name] [0/1]\r\n", ch);
          return;
      }

      i = atoi(arg);
      if (i == 1) {
          sprintf_to_char(ch, "%s now allowed.\r\n", share->name);
          share->ok= TRUE;
      } else {
          sprintf_to_char(ch, "%s now banned.\r\n", share->name);
          share->ok= FALSE;
      }
      return;

  } else if (!str_cmp(arg2, "info")) {
      SHARE_DATA *share = get_share(arg3);
      
      if (!share) {
          send_to_char("That kind of share doesn't exist.\r\n", ch);
          send_to_char("SYNTAX:  bank share info [name]\r\n", ch);
          return;
      }

      if (share->owner) {
          sprintf_to_char(ch, "{g%s{x (%s)\r\n", share->name, share->owner);
      } else {
         sprintf_to_char(ch, "{g%s{x\r\n", share->name);
      }

      send_to_char("{c--------------------------------------{x\r\n", ch);
      sprintf_to_char(ch, "%s\r\n", share->desc);
      return;

  } else if (!str_cmp(arg2, "desc")
  && IS_IMMORTAL(ch)) {
      SHARE_DATA *share = get_share(arg3);
     
      if (!share) {
          send_to_char("That kind of share doesn't exist.\r\n", ch);
          send_to_char("SYNTAX:  bank share desc [name]\r\n", ch);
          return;
      }

      string_append(ch, &share->desc);
      return;

  } else {
    send_to_char("\r\nStock Exchange Commands:{x\r\n\r\n",ch);
    send_to_char("  bank share balance\r\n", ch);
    send_to_char("  bank share info [name]\r\n", ch);
    send_to_char("  bank share buy [name] [ammount]\r\n", ch);
    send_to_char("  bank share sell [name] [ammount]\r\n", ch);
    if (IS_HERO(ch)) send_to_char("  bank share emission [name] [ammount]\r\n", ch);
    if (IS_IMMORTAL(ch)) {
         send_to_char("  bank share allow [name] [0/1]\r\n", ch);
         send_to_char("  bank share desc [name]\r\n", ch);
    }
    send_to_char("  bank share help\r\n", ch);
    return;
  }

  return;
}


int calculate_value(SHARE_DATA *share) {
int i, rd;
int balance = 0;
float md;

    for(i = 0; i<32; i++) {
         balance += share->balance[i];
    }

    md =  balance / float(share->emission + 1);

    rd = share->raw + int(share->raw * md * 5.0);
    rd = UMAX(rd, 50);
    return rd;
}


void update_stock_market() {
SHARE_DATA *share;
int best_buy = 0;
float best_buy_val = 0;
int best_sell = 0;
float best_sell_val = 0;
float curr;
int n;

    for(share = share_list; share; share = share->next) {
         share->raw = share->raw * (28 + number_range(0,4)) / 30;
         share->raw = UMAX(share->raw, 50);
         curr =(calculate_value(share) - share->raw) / share->raw;

         if (curr >best_sell_val
         && share->on_npc > 0) {
                best_sell_val = curr;
                best_sell = share->id;
         }

         if (curr <best_buy_val
         && share->emission > 0) {
                best_buy_val = curr;
                best_buy = share->id;
         }
         
         if (number_percent() > 50) {
               if (share->emission > share->on_npc*4) {
                   share->emission--;
                   share->on_npc++;
                   share->balance[time_info.day]++;
               }
         } else {
               if (share->on_npc > share->emission / 10) {
                   share->emission++;
                   share->on_npc--;
                   share->balance[time_info.day]--;
               }
         }               
    }
    
    if (best_buy == best_sell) return;

    if (best_buy != 0
    && number_percent() < 5) {
        share = get_share_by_id (best_buy);
        if (share) {
             n = number_range(1, 10);
             share->emission -= n;
             share->on_npc +=n;
             share->balance[time_info.day] += n;
        }
    }

    if (best_sell != 0
    && number_percent() < 5) {
        share = get_share_by_id (best_sell);
        if (share) {
             n = number_range(1, 10);
             share->emission += n;
             share->on_npc -=n;
             share->balance[time_info.day] -= n;
        }
    }

    return;
}


void clear_stock_balance() {
SHARE_DATA *share;

    for(share = share_list; share; share = share->next) {
         share->balance[time_info.day] = 0;
    }
    return;
}


void do_change(CHAR_DATA *ch, char *arg2, char *arg3, char *arg4) {
long amount = 0;
int from, to, i;
float modi, modi2;
int local;

    if (arg2[0] == '\0') {
        send_to_char("Exchange Courses:\r\n", ch);
        local = find_currency(ch);
        for (i = 0; i < MAX_CURRENCY; i++) {
             if (i == local) continue;
             if ((modi = get_currency_modifier(ch->in_room->area, i)) == 0) continue;
             if ((modi2 = get_currency_modifier(ch->in_room->area, local)) == 0) continue;
             amount = (long) (100.0 * (modi / modi2 * 0.8));
             if (amount == 0) continue;
             sprintf_to_char(ch, "100 %15s => ", flag_string(currency_type, i));
             sprintf_to_char(ch, "%6ld %s\r\n", amount, flag_string(currency_type, local));
        }                       
        return;
    }

    from = flag_value(currency_accept, arg3);
    if (from < 0 || from > MAX_CURRENCY) {
        send_to_char("Invalid currency.\r\n", ch);
        return;
    }

    if (!str_cmp(arg2, "all")) {
        amount = ch->gold[from];        
    } else {
        if (!is_number(arg2)) {
             send_to_char("Invalid amount.\r\n", ch);
             return;
        }
        if ((amount = atoi(arg2)) == 0) {
             send_to_char("Invalid amount.\r\n", ch);
             return;
        }
    }

    to = flag_value(currency_accept, arg4);
    if (to < 0 || to > MAX_CURRENCY) {
        send_to_char("Invalid currency.\r\n", ch);
        return;
    }

    if (from == to) {
        send_to_char("This transaction doesn't make sense.\r\n", ch);
        return;
    }

    if ((modi = get_currency_modifier(ch->in_room->area, from)) == 0) {
        send_to_char("We don't accept that kind of money here.\r\n", ch);
        return;
    }

    if ((modi2 = get_currency_modifier(ch->in_room->area, to)) == 0) {
        send_to_char("We don't have that kind of money here.\r\n", ch);
        return;
    }

    if (ch->gold[from] < amount) {
        sprintf_to_char(ch, "You don't have that much %s.\r\n", flag_string(currency_type, from));
        return;
    }

    ch->gold[from] -= amount;
    sprintf_to_char(ch, "You exchange %ld %s ", amount, flag_string(currency_type, from));
    amount = (long) (amount * (modi / modi2 * 0.8));
    ch->gold[to] +=amount;
    sprintf_to_char(ch, "into %ld %s.\r\n", amount, flag_string(currency_type, to));
    return;
}

void do_material(CHAR_DATA *ch, char *arg2) {
OBJ_DATA *obj;
int price;

    if (arg2[0] == '\0') {
        send_to_char("Change what?\r\n", ch);
        return;
    }

    obj = get_obj_carry(ch, arg2);
    if (!obj) {
        send_to_char("You haven't got that.\r\n", ch);
        return;
    }

    if (obj->item_type != ITEM_RAW) {
        send_to_char("That's no raw material.\r\n", ch);
        return;
    }

    switch (obj->material) {
           default:
             send_to_char("That material isn't very interesting.\r\n", ch);
             return;

           case MATERIAL_SILVER:
                price = 50;
                break;

           case MATERIAL_GOLD:
                price = 300;
                break;

           case MATERIAL_ADAMANTITE:
                price = 2000;
                break;

           case MATERIAL_MITHRIL:
                price = 900;
                break;

           case MATERIAL_CRYSTAL:
                price = 75;
                break;

           case MATERIAL_DIAMOND:
                price = 5000;
                break;

           case MATERIAL_PEARL:
                price = 25;
                break;
    }

    ch->gold[0] += price;
    sprintf_to_char(ch, "You exchange %s for %d $.\r\n", obj->short_descr, price);
    act( "$n exchanges $p.", ch, obj, NULL, TO_ROOM);
    obj_from_char(obj);
    extract_obj(obj);
    return;
}                  


bool identify_teller(char* bank_name, char* account_name) {
BANK *bp;
ACCOUNT *ap;

        bp = find_bank(bank_name, TRUE);
        if (!bp) return FALSE;

        ap = find_account(bp, account_name, TRUE);
        if (!ap) return FALSE;

        return TRUE;
}
