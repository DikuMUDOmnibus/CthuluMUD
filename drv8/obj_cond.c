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

void show_obj_cond (CHAR_DATA *ch, OBJ_DATA *obj);
void check_damage_obj (CHAR_DATA *ch, OBJ_DATA *obj, int chance);
void damage_obj (CHAR_DATA *ch,	OBJ_DATA *obj, int damage);
void set_obj_cond (OBJ_DATA *obj, int condition);

void show_obj_cond (CHAR_DATA *ch, OBJ_DATA *obj)
	{
	char buf[MAX_STRING_LENGTH];
	int condition=0;
	
	if (IS_SET(obj->extra_flags, ITEM_NO_COND)) /*no show condition */
		return;
	
	if (obj->condition == 100)
		condition = 0;
	else if (obj->condition > 90)
		condition = 1 ;
	else if (obj->condition > 75)
		condition = 2;
	else if (obj->condition > 50)
		condition = 3;
	else if (obj->condition > 25)
		condition = 4;
	else if (obj->condition > 10)
		condition = 5;
	else if (obj->condition >  0)
		condition = 6;
	else if (obj->condition == 0)
		condition = 7;
	
	sprintf (buf, "Condition: %s\r\n", cond_table[condition]);
	send_to_char (buf, ch);
	return;
	}	

void check_damage_obj (CHAR_DATA *ch, OBJ_DATA *obj, int chance) {
 
   /* Assumption - NULL obj means check all equipment */

    bool done=FALSE;
    int damage_pos;
    OBJ_DATA *dobj=NULL;
    int stop=0;

   /* Increase wear and tear to stimulate economy... */

       chance *= 2;
 
   /* Null object means damage a random one... */
	
    if (obj == NULL) {
        if (number_percent () <= chance) {
    
            while ( !done 
                 &&  stop <= 30 ) {

	        damage_pos = number_range (1, MAX_WEAR);

	        dobj = get_eq_char (ch, damage_pos); 

	        if ( dobj != NULL ) { 
	             done=TRUE;
	             damage_obj(ch, dobj, dice(1,3));
                }

	        stop++;
	    }
	}
 
        return;
    }

    if ( number_percent () <= chance ) {
        damage_obj(ch, obj, dice(1,3));
    }

    return;
} 		

void damage_obj (CHAR_DATA *ch, OBJ_DATA *obj, int damage)
	{
	int counter;
	
	if (obj == NULL) {
		bug ("NULL obj passed to damage_obj",0);
		return;
	}

               if (!IS_SET(obj->extra_flags, ITEM_MAGIC)) {
                    damage /=2;
               }

                if (!IS_SET(obj->extra_flags, ITEM_NO_COND)) {
                      obj->condition -= damage;
                }

	obj->condition = URANGE (0, obj->condition, 100);
	 
	/*Check for item falling apart*/
	if (obj->condition == 0)
		{
		char mesbuf[256];
		sprintf (mesbuf, "{c%s{c has become too badly damaged to use!{x\r\n", 
		( (obj->short_descr && obj->short_descr[0] != '\0') ? obj->short_descr :
			"Something") );
		send_to_char (mesbuf, ch);
                if (obj->wear_loc != WEAR_NONE) {
	   	  unequip_char (ch, obj);
                }
		return;
		}
	/* Now make adjustments to armor values and modify char AC */	
		if (obj->item_type == ITEM_ARMOR)
			{
			/* restore char AC as if not wearing object, then damage
				the object, then recalculate character AC */
			for (counter=0 ; counter < 4 ; counter ++)		
				{
				ch->armor[counter] += apply_ac (obj, obj->wear_loc, counter);
				obj->value[counter] = obj->valueorig[counter]*(obj->condition)/100;
				if ((obj->value[counter] == 0) && (obj->valueorig[counter] != 0))  
					obj->value[counter] = 1; /*always worth something til it breaks*/
				ch->armor[counter] -= apply_ac (obj, obj->wear_loc, counter);
				}
			}
	return;
	}			

void set_obj_cond (OBJ_DATA *obj, int condition)
	{
	int counter;

	obj->condition = condition;
	if (obj->item_type == ITEM_ARMOR)
		for (counter=0 ; counter < 4 ; counter ++)		
			{
			obj->value[counter] = obj->valueorig[counter]*(obj->condition)/100;
			if ((obj->value[counter] == 0) && (obj->valueorig[counter] != 0))  
				obj->value[counter] = 1; /*always worth something til it breaks*/
			}
	return;
	}
