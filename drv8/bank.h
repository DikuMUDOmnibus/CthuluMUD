/*
* CthulhuMud
*/

/* Process balance... */

void do_balance(CHAR_DATA *teller, CHAR_DATA *cust, char *bank_name, char *account_name);

/* Process deposit... */

void do_deposit(CHAR_DATA	*teller, CHAR_DATA *cust, char *ammount, int currency, char *bank_name, char *account_name);
void do_deposit_raw(CHAR_DATA *teller, int ammount, int currency, char *bank_name, char *account_name);

/* Process withdraw... */

void do_withdraw(CHAR_DATA *teller, CHAR_DATA *cust, char *ammount, int currency, char *bank_name, char *account_name);

/* Locate a bank teller... */

CHAR_DATA *find_teller(CHAR_DATA *ch);

/* Save bank and account details... */

void save_banks();

/* Delete a players bank accounts... */

void delete_accounts(CHAR_DATA *ch);

/* Typedefs and structures... */

typedef struct bank    BANK;
typedef struct account ACCOUNT;
typedef struct share_data SHARE_DATA;

struct bank {
  BANK     *next;
  ACCOUNT  *accounts;
  char     *name;
};

struct account {
  ACCOUNT  *next;
  char     *holder;
  int       gold;
  int       last_day;
  int       last_month;
  int       last_year; 
};

struct share_data {
  SHARE_DATA *next;
  char     *name;
  char     *desc;
  char     *owner;
  bool          ok;
  short       id;
  int              raw;
  int              emission;
  int              on_npc;
  short       balance[32]; 
};


/* Find banks and accounts... */

BANK *find_bank(char *name, bool make);
ACCOUNT *find_account(BANK *bp, char *name, bool make);


void update_stock_market(void);
void do_material(CHAR_DATA *ch, char *arg2);
void do_change(CHAR_DATA *ch, char *arg2, char *arg3, char *arg4);
void clear_stock_balance(void);
void do_stock(CHAR_DATA *cust, char *arg2, char *arg3, char *arg);
void save_shares(void);
void load_shares(void);
int calculate_value(SHARE_DATA *share);
int get_share_id(char *name);
SHARE_DATA *get_share_by_id(int id);
SHARE_DATA *get_share(char *share_name);
bool identify_teller(char* bank_name, char* account_name);

#define SHARE_UNDEFINED -1
