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

void save_leases();


void do_lease( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
CHAR_DATA *keeper;
SHOP_DATA *pShop;
int iTrade;
long cost;
bool hotel = FALSE;
int currency = find_currency(ch);

      if (IS_SET(ch->in_room->room_flags, ROOM_RENT))    {
	if (ch->in_room->rented_by)  {
	    if (ch->in_room->rented_by != ch->name)  {
		send_to_char("This room is not currently for rent.\r\n",ch);
		return; 
	    } 
	} 

                     hotel = FALSE;
               	     if ((keeper = find_keeper(ch, TRUE)) != NULL) {
                              pShop = keeper->pIndexData->pShop;
                              for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
	                     if ( pShop->buy_type[iTrade] == 998 ) hotel = TRUE;
                              }
                     }

                    if (hotel) cost = ch->in_room->room_rent * keeper->pIndexData->pShop->profit_buy / 100;
                    else cost = ch->in_room->room_rent;

	sprintf(buf,"{CThe monthly rent on this room is ${Y%ld.{x\r\n", cost); 
	send_to_char( buf , ch );

	if (ch->in_room->rented_by) {
	    sprintf(buf,"{CYour rent is paid until Month %d of Year %d.{x\r\n", ch->in_room->paid_month, ch->in_room->paid_year );
	    send_to_char(buf,ch);
	}
    
	if (argument[0] != '\0') {
	    int timeleft; 
	    int months;

	    if (!is_number(argument))  {
		send_to_char("{RInvalid argument. Syntax: lease <number>.{x\r\n",ch);
		return;
	    }

	    months = atoi(argument);

	    if (months < 1 ) {
		send_to_char("{MYou must rent for at least 1 full month.{x\r\n",ch);
		return;
	    } 

	    if (months > 12 ) {
		send_to_char("{MYou can only rent for a maximum of 1 year.{x\r\n",ch);
		return;
	    } 

	    timeleft = 0;

	    if (ch->in_room->paid_year < time_info.year) {
		timeleft = 0;
	    } else if (ch->in_room->paid_year == time_info.year) {
		timeleft = ch->in_room->paid_month - time_info.month;
	    } else if (ch->in_room->paid_year > time_info.year) {
		timeleft = ch->in_room->paid_month - time_info.month;
		timeleft += 12;
	    }

	    if (timeleft >= 12) {
		send_to_char("{YYou are currently paid for a full year.{x\r\n",ch);
		return;
	    }

	    if ( (timeleft + months) > 12 ) {
		send_to_char("{MYou can't rent past 1 year. Try fewer months.{x\r\n",ch);
		return;
	    } 

                     hotel = FALSE;
   	     if ((keeper = find_keeper(ch, TRUE)) != NULL) {
                              pShop = keeper->pIndexData->pShop;
                               for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
	                     if ( pShop->buy_type[iTrade] == 998 ) hotel = TRUE;
                              }
                     }

                    if (hotel) cost = months * ch->in_room->room_rent * keeper->pIndexData->pShop->profit_buy / 100;
                    else cost = months * ch->in_room->room_rent;

	    if (cost > ch->gold[currency] ) {
		sprintf_to_char(ch, "{RYou don't have enough %s.{x\r\n",flag_string(currency_type, currency));
		return;
	    } 

	    ch->gold[currency] -= cost;
                     if (hotel) keeper->gold[currency] += cost;

	    ch->in_room->paid_month = time_info.month;
	    ch->in_room->paid_year = time_info.year;

	    if ( (timeleft + months) > 12 ) {
		++ch->in_room->paid_year;
	    } else {
		if ( ( time_info.month + (months + timeleft) ) > 12 ) {
		    ++ch->in_room->paid_year;
		    ch->in_room->paid_month += ( (months + timeleft) - 12 );
		} else {
		    ch->in_room->paid_month += (months + timeleft);
		}
	    }

	    sprintf_to_char(ch,"{CYou have now rented this room for {Y%ld{C %s coins for a total of %d months.{x\r\n", cost, flag_string(currency_type, currency), months );
	    sprintf_to_char(ch,"{CThe rent is NOW paid until Month %d of Year %d.{x\r\n", ch->in_room->paid_month, ch->in_room->paid_year );
	    act( "$n has signed a lease on this room.", ch, NULL, NULL, TO_ROOM );
	    free_string( ch->in_room->rented_by );
                    ch->in_room->rented_by = str_dup(ch->short_descr);
	    save_leases();
	    return;
	} 
    } else {
       send_to_char("This room cannot be rented.\r\n",ch);
       return;
    }
    return;
} 


void do_setlease( CHAR_DATA *ch, char *argument ) {
char arg[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];


    if (!IS_IMMORTAL(ch)) {
         send_to_char("You can't do that.\r\n",ch);
         return;
    }
        
    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg2);

    if (!str_cmp(arg, "name")) {
        free_string(ch->in_room->rented_by);
        ch->in_room->rented_by = str_dup(arg2);        
        send_to_char("Lease Name set.\r\n",ch);

    } else if (!str_cmp(arg, "month")) {
        ch->in_room->paid_month = atoi(arg2);
        send_to_char("Lease Month set.\r\n",ch);

    } else if (!str_cmp(arg, "year")) {
        ch->in_room->paid_year = atoi(arg2);
        send_to_char("Lease Year set.\r\n",ch);

    } else {
         send_to_char("SYNTAX: SETLEASE name <player name>\r\n",ch);
         send_to_char("        SETLEASE month <month>\r\n",ch);
         send_to_char("        SETLEASE year <year>\r\n",ch);
    }
    return;
}


void do_checklease ( CHAR_DATA *ch, char *argument ) {
char buf[MAX_STRING_LENGTH];
char arg[MAX_INPUT_LENGTH];
CHAR_DATA *keeper;
SHOP_DATA *pShop;
int iTrade;
bool hotel;

    if (!IS_SET(ch->in_room->room_flags, ROOM_RENT) ) {
         send_to_char("{GThis is a non-rentable room.{x\r\n",ch);
         return;
    }

    argument = one_argument(argument, arg);    

    if (arg[0] == '\0') {
	if (ch->in_room->rented_by) {
	    sprintf (buf,"{GRented By:     {W%s\r\n", ch->in_room->rented_by );
	    send_to_char(buf,ch);
	    sprintf (buf,"{GMonthly Rent:  {W%ld\r\n", ch->in_room->room_rent );
	    send_to_char(buf,ch);
	    sprintf (buf,"{GLease Expires: {WMonth %d of Year %d.{x\r\n", ch->in_room->paid_month, ch->in_room->paid_year );
	    send_to_char(buf,ch);
	} else {
	    send_to_char("{GNot currently being rented.\r\n",ch);
	    sprintf (buf,"Monthly Rent:  {W%ld{x\r\n", ch->in_room->room_rent);
	    send_to_char(buf,ch);
	} 

              	if ((keeper = find_keeper(ch, TRUE)) != NULL) {
                      pShop = keeper->pIndexData->pShop;
                      hotel = FALSE;
                      for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
	             if ( pShop->buy_type[iTrade] == 998 ) hotel = TRUE;
                      }
                      if (hotel) sprintf_to_char(ch, "The profit goes to %s.\r\n", keeper->short_descr);
                }
                return;
    }
   
    if (!strcmp(arg, "delete")) {
       if (!IS_IMMORTAL(ch)) {
           send_to_char("Mortals can't do that!\r\n",ch);
           return;
       }

       ch->in_room->rented_by = NULL;
       ch->in_room->paid_month = 0;
       ch->in_room->paid_year = 0;
       send_to_char("Lease Data deleted!\r\n",ch);
       return;
    }

    send_to_char("Lease Commands::\r\n",ch);
    send_to_char("LEASE\r\n",ch);
    send_to_char("LEASE <months>\r\n",ch);
    if (IS_IMMORTAL(ch)) send_to_char("LEASE DELETE\r\n",ch);
    return;
} 


void save_leases() {
    FILE *fp = NULL;
    ROOM_INDEX_DATA *pRoomIndex; 

    int iHash;
    char buf[MAX_STRING_LENGTH];

    sprintf (buf, "%s/lease.dat", DATA_DIR);

    fp = fopen (buf, "w");

    if (!fp)   {
	bug ("Could not open Lease.DAT for writing!",0);
	return;
    }

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )    {
	for ( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next ) {
	    if (IS_SET(pRoomIndex->room_flags, ROOM_RENT)
                    || pRoomIndex->rented_by) {
		fprintf (fp, "L %ld %ld %d %d %d %s~\n",
                              pRoomIndex->vnum,
                              pRoomIndex->room_rent,
                              pRoomIndex->paid_month,
                              pRoomIndex->paid_day,
                              pRoomIndex->paid_year,
                              pRoomIndex->rented_by);
	    } 
	} 
    } 

    fprintf( fp, "END\n" );
    fclose ( fp );
    return;
}


void load_leases(){
    FILE *fp = NULL;
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    bool done=FALSE;
    ROOM_INDEX_DATA *pRoomIndex; 

    sprintf (buf, "%s/lease.dat", DATA_DIR);

    fp = fopen (buf, "r");

    if (!fp)   {
	bug ("Could not open Lease.DAT for Reading!",0);
	return;
    }

    fMatch = FALSE;

    while (!done)  {
	word = fread_word( fp );
	if (!str_cmp(word,"L")) {
	    pRoomIndex = get_room_index ( fread_number( fp ) );
	    pRoomIndex->room_rent	= fread_number( fp );
	    pRoomIndex->paid_month	= fread_number( fp );
	    pRoomIndex->paid_day		= fread_number( fp );
	    pRoomIndex->paid_year		= fread_number( fp );
	    free_string(pRoomIndex->rented_by);
	    pRoomIndex->rented_by	= fread_string( fp );
	}

	if (!str_cmp(word,"END"))	{
	    done = TRUE;
	    break;
	}
    } 

    fclose (fp);
    return;
}


