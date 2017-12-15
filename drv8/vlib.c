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
#include "vlib.h"

void format_vnum(VNUM vnum, char *outbuf) {

	int lib, num;

	// Must have a buffer...

	if (outbuf == NULL) {
		return;
	}

	// Special cases...

	if (vnum <= 0) {
		sprintf(outbuf,"%5ld",vnum);
		return;
	}

	// Split and format...

	lib = LIB_VNUM(vnum);
	num = NUM_VNUM(vnum);

	if (lib == 0) {
		sprintf(outbuf,"%5d",num);
	} else {
		sprintf(outbuf,"%5d:%5d",lib,num);
	}

	return;
}

