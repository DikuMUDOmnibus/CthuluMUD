/*
 * CthulhuMud
 */

/* this is a listing of all the commands and command related data */

/* for command types */
#define ML 	DIV_IMPLEMENTOR	/* 60: implementor */
#define L1	DIV_GREATER	  	/* 59: creator */
#define L2	DIV_GREATER		/* 58: supreme being */
#define L3	DIV_GREATER		/* 57: deity */
#define L4 	DIV_GOD		/* 56: god */
#define L5	DIV_GOD		/* 55: immortal */
#define L6	DIV_GOD		/* 54: demigod */
#define L7	DIV_LESSER		/* 53: angel */
#define L8	DIV_CREATOR		/* 52: avatar */
#define IM	DIV_CREATOR	 	/* 52: angel */
#define HE	DIV_HERO	
#define IN      	DIV_INVESTIGATOR
#define HU      	DIV_HUMAN              

#define COMMAND_GENERAL 		0
#define COMMAND_MOVEMENT		1
#define COMMAND_COMMUNICATION	2
#define COMMAND_COMBAT		3
#define COMMAND_MAGIC		4
#define COMMAND_INTERACTIVE	5
#define COMMAND_SPECIAL		6
#define COMMAND_MANIPULATION	7

#define COMMAND_ADMIN		9

char* const channel_col[3] ={
"{m",
"{g",
"{b"
};
 

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
/* elfren added beep */
DECLARE_DO_FUN(do_alist);
DECLARE_DO_FUN(do_ainfo);
DECLARE_DO_FUN(do_advance);
DECLARE_DO_FUN(do_shell);
DECLARE_DO_FUN(do_award);
DECLARE_DO_FUN(do_affect);
DECLARE_DO_FUN(do_room_affect);
DECLARE_DO_FUN(do_afk);
DECLARE_DO_FUN(do_alias);
DECLARE_DO_FUN(do_unalias);
DECLARE_DO_FUN(do_allow);
DECLARE_DO_FUN(do_answer);
DECLARE_DO_FUN(do_areas);
DECLARE_DO_FUN(do_arebase);
DECLARE_DO_FUN(do_amerge);
DECLARE_DO_FUN(do_asplit);
DECLARE_DO_FUN(do_asuck);
DECLARE_DO_FUN(do_at);
DECLARE_DO_FUN(do_autoassist);
DECLARE_DO_FUN(do_autoexit);
DECLARE_DO_FUN(do_autogold);
DECLARE_DO_FUN(do_autolist);
DECLARE_DO_FUN(do_autoloot);
DECLARE_DO_FUN(do_autosac);
DECLARE_DO_FUN(do_autosave);
DECLARE_DO_FUN(do_autosplit);
DECLARE_DO_FUN(do_autokill);
DECLARE_DO_FUN(do_autocon);
DECLARE_DO_FUN(do_assassinate);
DECLARE_DO_FUN(do_backstab);
DECLARE_DO_FUN(do_blackjack);
DECLARE_DO_FUN(do_bamfin);
DECLARE_DO_FUN(do_bamfout);
DECLARE_DO_FUN(do_ban);
DECLARE_DO_FUN(do_bash);
DECLARE_DO_FUN(do_intimidate);
DECLARE_DO_FUN(do_beep);
DECLARE_DO_FUN(do_berserk);
DECLARE_DO_FUN(do_board);
DECLARE_DO_FUN(do_brandish);
DECLARE_DO_FUN(do_brief);
DECLARE_DO_FUN(do_brew);
DECLARE_DO_FUN(do_bug);
DECLARE_DO_FUN(do_buy);
DECLARE_DO_FUN(do_translate);
DECLARE_DO_FUN(do_build);
DECLARE_DO_FUN(do_shop);
DECLARE_DO_FUN(do_auction);
DECLARE_DO_FUN(do_appraise);
DECLARE_DO_FUN(do_restring);
DECLARE_DO_FUN(do_cast);
DECLARE_DO_FUN(do_ritual);
DECLARE_DO_FUN(do_channels);
DECLARE_DO_FUN(do_bank);
DECLARE_DO_FUN(do_gamble);
DECLARE_DO_FUN(do_clan_tell);
DECLARE_DO_FUN(do_clone);
DECLARE_DO_FUN(do_close);
DECLARE_DO_FUN(do_colour);
DECLARE_DO_FUN(do_cursor);
DECLARE_DO_FUN(do_commands);
DECLARE_DO_FUN(do_autocombine);
DECLARE_DO_FUN(do_compact);
DECLARE_DO_FUN(do_compare);
DECLARE_DO_FUN(do_consider);
DECLARE_DO_FUN(do_converse);
DECLARE_DO_FUN(do_copy);
DECLARE_DO_FUN(do_count);
DECLARE_DO_FUN(do_crush);
DECLARE_DO_FUN(do_currents);
DECLARE_DO_FUN(do_deaf);
DECLARE_DO_FUN(do_deeds);
DECLARE_DO_FUN(do_delet);
DECLARE_DO_FUN(do_delete);
DECLARE_DO_FUN(do_deny);
DECLARE_DO_FUN(do_setannoying);
DECLARE_DO_FUN(do_quitannoying);
DECLARE_DO_FUN(do_description);
DECLARE_DO_FUN(do_debate);
DECLARE_DO_FUN(do_bio);
DECLARE_DO_FUN(do_devour);
DECLARE_DO_FUN(do_incarnate);
DECLARE_DO_FUN(do_dirt);
DECLARE_DO_FUN(do_disable);
DECLARE_DO_FUN(do_disarm);
DECLARE_DO_FUN(do_disconnect);
DECLARE_DO_FUN(do_down);
DECLARE_DO_FUN(do_drink);
DECLARE_DO_FUN(do_dream);
DECLARE_DO_FUN(do_drop);
DECLARE_DO_FUN(do_plant);
DECLARE_DO_FUN(do_photograph);
DECLARE_DO_FUN(do_dual);
DECLARE_DO_FUN(do_dump);
DECLARE_DO_FUN(do_east);
DECLARE_DO_FUN(do_eat);
DECLARE_DO_FUN(do_echo);
DECLARE_DO_FUN(do_email);
DECLARE_DO_FUN(do_enable);
DECLARE_DO_FUN(do_check);
DECLARE_DO_FUN(do_end); 
DECLARE_DO_FUN(do_enter); 
DECLARE_DO_FUN(do_circle);
DECLARE_DO_FUN(do_envenom);
DECLARE_DO_FUN(do_equipment);
DECLARE_DO_FUN(do_examine);
DECLARE_DO_FUN(do_exits);
DECLARE_DO_FUN(do_fill);
DECLARE_DO_FUN(do_pour);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_ride);
DECLARE_DO_FUN(do_drive);
DECLARE_DO_FUN(do_tame);
DECLARE_DO_FUN(do_follow);
DECLARE_DO_FUN(do_force);
DECLARE_DO_FUN(do_forceremove);
DECLARE_DO_FUN(do_freeze);
DECLARE_DO_FUN(do_pload);
DECLARE_DO_FUN(do_punload);
DECLARE_DO_FUN(do_cleanup);
DECLARE_DO_FUN(do_fullfight); 
DECLARE_DO_FUN(do_fullcast); 
DECLARE_DO_FUN(do_fullweather); 
DECLARE_DO_FUN(do_gadget_pull); 
DECLARE_DO_FUN(do_gadget_push); 
DECLARE_DO_FUN(do_gadget_twist); 
DECLARE_DO_FUN(do_gadget_turn); 
DECLARE_DO_FUN(do_gadget_turnback); 
DECLARE_DO_FUN(do_gadget_move); 
DECLARE_DO_FUN(do_gadget_lift); 
DECLARE_DO_FUN(do_gadget_press); 
DECLARE_DO_FUN(do_gadget_digin); 
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_tamp);
DECLARE_DO_FUN(do_smoke);
DECLARE_DO_FUN(do_use);
DECLARE_DO_FUN(do_carry);
DECLARE_DO_FUN(do_drag);
DECLARE_DO_FUN(do_cut);
DECLARE_DO_FUN(do_voodoo);
DECLARE_DO_FUN(do_give);
DECLARE_DO_FUN(do_glance);
DECLARE_DO_FUN(do_gossip);
DECLARE_DO_FUN(do_goto);
DECLARE_DO_FUN(do_group);
DECLARE_DO_FUN(do_gtell);
DECLARE_DO_FUN(do_heal);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_hide);
DECLARE_DO_FUN(do_hlist);
DECLARE_DO_FUN(do_holylight);
DECLARE_DO_FUN(do_home);
DECLARE_DO_FUN(do_homepage);
DECLARE_DO_FUN(do_hours);
DECLARE_DO_FUN(do_image);
DECLARE_DO_FUN(do_immtalk);
DECLARE_DO_FUN(do_immtitle);
DECLARE_DO_FUN(do_imotd);
DECLARE_DO_FUN(do_imp_html);
DECLARE_DO_FUN(do_in);
DECLARE_DO_FUN(do_inventory);
DECLARE_DO_FUN(do_invis);
DECLARE_DO_FUN(do_interrogate);
DECLARE_DO_FUN(do_cloak);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_bite);
DECLARE_DO_FUN(do_strangle);
DECLARE_DO_FUN(do_kill);
DECLARE_DO_FUN(do_knock);
DECLARE_DO_FUN(do_learn);
DECLARE_DO_FUN(do_yith);
DECLARE_DO_FUN(do_list);
DECLARE_DO_FUN(do_listen);
DECLARE_DO_FUN(do_load);
DECLARE_DO_FUN(do_lock);
DECLARE_DO_FUN(do_log);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_remember	);
DECLARE_DO_FUN(do_lore); 
DECLARE_DO_FUN(do_last); 
DECLARE_DO_FUN(do_memory);
DECLARE_DO_FUN(do_mfind);
DECLARE_DO_FUN(do_mload);
DECLARE_DO_FUN(do_mset);
DECLARE_DO_FUN(do_mstat);
DECLARE_DO_FUN(do_mwhere);
DECLARE_DO_FUN(do_mlevel);
DECLARE_DO_FUN(do_olevel);
DECLARE_DO_FUN(do_owhere);
DECLARE_DO_FUN(do_pwhere);
DECLARE_DO_FUN(do_motd);
DECLARE_DO_FUN(do_murde);
DECLARE_DO_FUN(do_murder);
DECLARE_DO_FUN(do_music);
DECLARE_DO_FUN(do_newlock);
DECLARE_DO_FUN(do_nochannels);
DECLARE_DO_FUN(do_noemote);
DECLARE_DO_FUN(do_nofollow);
DECLARE_DO_FUN(do_nosummon);
DECLARE_DO_FUN(do_noloot);
DECLARE_DO_FUN(do_north);
DECLARE_DO_FUN(do_northeast);
DECLARE_DO_FUN(do_northwest);
DECLARE_DO_FUN(do_noshout);
DECLARE_DO_FUN(do_notes);
DECLARE_DO_FUN(do_note);
DECLARE_DO_FUN(do_notell);
DECLARE_DO_FUN(do_notify);
DECLARE_DO_FUN(do_offer);
DECLARE_DO_FUN(do_ofind);
DECLARE_DO_FUN(do_oload);
DECLARE_DO_FUN(do_open);
DECLARE_DO_FUN(do_smash);
DECLARE_DO_FUN(do_throw);
DECLARE_DO_FUN(do_order);
DECLARE_DO_FUN(do_petname);
DECLARE_DO_FUN(do_oset);
DECLARE_DO_FUN(do_ostat);
DECLARE_DO_FUN(do_out);
DECLARE_DO_FUN(do_outfit);
DECLARE_DO_FUN(do_pardon);
DECLARE_DO_FUN(do_distribute);
DECLARE_DO_FUN(do_password);
DECLARE_DO_FUN(do_peace);
DECLARE_DO_FUN(do_pecho);
DECLARE_DO_FUN(do_permapk);
DECLARE_DO_FUN(do_pick);
DECLARE_DO_FUN(do_practice);
DECLARE_DO_FUN(do_pray);
DECLARE_DO_FUN(do_prompt);
DECLARE_DO_FUN(do_prof);
DECLARE_DO_FUN(do_purge);
DECLARE_DO_FUN(do_put);
DECLARE_DO_FUN(do_quaff);
DECLARE_DO_FUN(do_quests);
DECLARE_DO_FUN(do_question);
DECLARE_DO_FUN(do_queue);
DECLARE_DO_FUN(do_qui);
DECLARE_DO_FUN(do_quiet);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_read);
DECLARE_DO_FUN(do_write);
DECLARE_DO_FUN(do_reboo);
DECLARE_DO_FUN(do_reboot);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_recho);
DECLARE_DO_FUN(do_recite);
DECLARE_DO_FUN(do_remove);
DECLARE_DO_FUN(do_reload);
DECLARE_DO_FUN(do_rent);
DECLARE_DO_FUN(do_lease);
DECLARE_DO_FUN(do_checklease);
DECLARE_DO_FUN(do_reply);
DECLARE_DO_FUN(do_report);
DECLARE_DO_FUN(do_rescue);
DECLARE_DO_FUN(do_resize);
DECLARE_DO_FUN(do_repair);
DECLARE_DO_FUN(do_rest);
DECLARE_DO_FUN(do_restore);
DECLARE_DO_FUN(do_return);
DECLARE_DO_FUN(do_rotate);
DECLARE_DO_FUN(do_rset);
DECLARE_DO_FUN(do_rstat);
DECLARE_DO_FUN(do_rules);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_save);
DECLARE_DO_FUN(do_scan);
DECLARE_DO_FUN(do_score);
DECLARE_DO_FUN(do_body);
DECLARE_DO_FUN(do_scream);
DECLARE_DO_FUN(do_scribe);
DECLARE_DO_FUN(do_scroll);
DECLARE_DO_FUN(do_scry);
DECLARE_DO_FUN(do_search);
DECLARE_DO_FUN(do_sell);
DECLARE_DO_FUN(do_set);
DECLARE_DO_FUN(do_sharpen);
DECLARE_DO_FUN(do_fix);
DECLARE_DO_FUN(do_forge);
DECLARE_DO_FUN(do_gunsmith);
DECLARE_DO_FUN(do_skin);
DECLARE_DO_FUN(do_refit);
DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_show);
DECLARE_DO_FUN(do_shutdow);
DECLARE_DO_FUN(do_shutdown);
DECLARE_DO_FUN(do_sit);
DECLARE_DO_FUN(do_skills);
DECLARE_DO_FUN(do_sla);
DECLARE_DO_FUN(do_slay);
DECLARE_DO_FUN(do_sleep);
DECLARE_DO_FUN(do_slookup);
DECLARE_DO_FUN(do_smell);
DECLARE_DO_FUN(do_sneak);
DECLARE_DO_FUN(do_snoop);
DECLARE_DO_FUN(do_socials);
DECLARE_DO_FUN(do_south);
DECLARE_DO_FUN(do_southeast	);
DECLARE_DO_FUN(do_southwest);
DECLARE_DO_FUN(do_society);
DECLARE_DO_FUN(do_sockets);
DECLARE_DO_FUN(do_rename);
DECLARE_DO_FUN(do_spells);
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_speak);
DECLARE_DO_FUN(do_sset);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_stat);
DECLARE_DO_FUN(do_steal);
DECLARE_DO_FUN(do_rob);
DECLARE_DO_FUN(do_study);
DECLARE_DO_FUN(do_story);
DECLARE_DO_FUN(do_string);
DECLARE_DO_FUN(do_switch);
DECLARE_DO_FUN(do_tail);
DECLARE_DO_FUN(do_teach);
DECLARE_DO_FUN(do_tell);
DECLARE_DO_FUN(do_telepathy);
DECLARE_DO_FUN(do_bandage);
DECLARE_DO_FUN(do_psychology);
DECLARE_DO_FUN(do_vampire);
DECLARE_DO_FUN(do_mindtransfer);
DECLARE_DO_FUN(do_were);
DECLARE_DO_FUN(do_lich);
DECLARE_DO_FUN(do_bind);
DECLARE_DO_FUN(do_untie);
DECLARE_DO_FUN(do_brainsuck	);
DECLARE_DO_FUN(do_sermonize	);
DECLARE_DO_FUN(do_therapy);
DECLARE_DO_FUN(do_time);
DECLARE_DO_FUN(do_title);
DECLARE_DO_FUN(do_worship);
DECLARE_DO_FUN(do_marry);
DECLARE_DO_FUN(do_style);
DECLARE_DO_FUN(do_tracks);
DECLARE_DO_FUN(do_train);
DECLARE_DO_FUN(do_transfer);
DECLARE_DO_FUN(do_trip);
DECLARE_DO_FUN(do_trap);
DECLARE_DO_FUN(do_bomb);
DECLARE_DO_FUN(do_trust);
DECLARE_DO_FUN(do_typo);
DECLARE_DO_FUN(do_unlock);
DECLARE_DO_FUN(do_unread);
DECLARE_DO_FUN(do_up);
DECLARE_DO_FUN(do_value);
DECLARE_DO_FUN(do_view);
DECLARE_DO_FUN(do_visible);
DECLARE_DO_FUN(do_vnum);
DECLARE_DO_FUN(do_wake);
DECLARE_DO_FUN(do_wear);
DECLARE_DO_FUN(do_weigh);
DECLARE_DO_FUN(do_west);
DECLARE_DO_FUN(do_where);
DECLARE_DO_FUN(do_who);
DECLARE_DO_FUN(do_whois);
DECLARE_DO_FUN(do_bounty);
DECLARE_DO_FUN(do_mission);
DECLARE_DO_FUN(do_wimpy);
DECLARE_DO_FUN(do_wizhelp);
DECLARE_DO_FUN(do_wizlock);
DECLARE_DO_FUN(do_wizlist);
DECLARE_DO_FUN(do_wiznet);
DECLARE_DO_FUN(do_worth);
DECLARE_DO_FUN(do_world);
DECLARE_DO_FUN(do_xinfo);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_zap);
DECLARE_DO_FUN(do_zlist);
DECLARE_DO_FUN(do_zcopy);
DECLARE_DO_FUN(do_zclear);
DECLARE_DO_FUN(do_remort);
DECLARE_DO_FUN(do_mpasound);
DECLARE_DO_FUN(do_mpat);
DECLARE_DO_FUN(do_mpecho);
DECLARE_DO_FUN(do_mpechoaround);
DECLARE_DO_FUN(do_mpechoat);
DECLARE_DO_FUN(do_mpforce);
DECLARE_DO_FUN(do_mpgoto);
DECLARE_DO_FUN(do_mpjunk);
DECLARE_DO_FUN(do_mpkill);
DECLARE_DO_FUN(do_mpmload);
DECLARE_DO_FUN(do_mpoload);
DECLARE_DO_FUN(do_mppurge);
DECLARE_DO_FUN(do_mptransfer);
DECLARE_DO_FUN(do_mpsanity);
DECLARE_DO_FUN(do_mpscript);
DECLARE_DO_FUN(do_mpreward);
DECLARE_DO_FUN(do_mpfame);
DECLARE_DO_FUN(do_mpconv);
DECLARE_DO_FUN(do_mpdeed);
DECLARE_DO_FUN(do_mpquest);
DECLARE_DO_FUN(do_mpstop);
DECLARE_DO_FUN(do_mphurt);
DECLARE_DO_FUN(do_mpselect);
DECLARE_DO_FUN(do_mpalign);
DECLARE_DO_FUN(do_mprelev);
DECLARE_DO_FUN(do_mpeffect);
DECLARE_DO_FUN(do_mpmemory);
DECLARE_DO_FUN(do_mphunt);
DECLARE_DO_FUN(do_mpsteal);
DECLARE_DO_FUN(do_mppet);
DECLARE_DO_FUN(do_mpinvasion);
DECLARE_DO_FUN(do_mposet);
DECLARE_DO_FUN(do_mpmset);
DECLARE_DO_FUN(do_mpstate);
DECLARE_DO_FUN(do_mpartifact);
DECLARE_DO_FUN(do_research);
DECLARE_DO_FUN(do_play);
DECLARE_DO_FUN(do_destroy);
DECLARE_DO_FUN(do_rebuild);
DECLARE_DO_FUN(do_store);
DECLARE_DO_FUN(do_fetch);
DECLARE_DO_FUN(do_connect);
DECLARE_DO_FUN(do_version);
DECLARE_DO_FUN(do_mudversion);
DECLARE_DO_FUN(do_mudreply);
DECLARE_DO_FUN(do_mudwhois);
DECLARE_DO_FUN(do_mudprompt);
DECLARE_DO_FUN(do_mudwho);
DECLARE_DO_FUN(do_mudgossip);
DECLARE_DO_FUN(do_mudbug);
DECLARE_DO_FUN(do_mudimmtalk);
DECLARE_DO_FUN(do_mudhero);
DECLARE_DO_FUN(do_mudping);
DECLARE_DO_FUN(do_note_prepare);
DECLARE_DO_FUN(do_note_line);
DECLARE_DO_FUN(do_note_stop);
DECLARE_DO_FUN(do_note_subject);
DECLARE_DO_FUN(do_mudplayerexists);
DECLARE_DO_FUN(do_mudreturn);
DECLARE_DO_FUN(do_autotax);
DECLARE_DO_FUN(do_duel);
DECLARE_DO_FUN(do_magical_duel);
DECLARE_DO_FUN(do_herotalk);
DECLARE_DO_FUN(do_investigatortalk);
DECLARE_DO_FUN(do_mpemergency);
DECLARE_DO_FUN(do_mppay);
DECLARE_DO_FUN(do_idoltalk);
DECLARE_DO_FUN(do_feed);
DECLARE_DO_FUN(do_distract);
DECLARE_DO_FUN(do_guard);
DECLARE_DO_FUN(do_noport);
DECLARE_DO_FUN(do_light);
DECLARE_DO_FUN(do_extinguish);
DECLARE_DO_FUN(do_sprinkle);
DECLARE_DO_FUN(do_stell);
DECLARE_DO_FUN(do_mtell);
DECLARE_DO_FUN(do_righteouskill);
DECLARE_DO_FUN(do_mpwound);
DECLARE_DO_FUN(do_land);
DECLARE_DO_FUN(do_launch);
DECLARE_DO_FUN(do_wield);
DECLARE_DO_FUN(do_hold);
DECLARE_DO_FUN(do_setlease);
DECLARE_DO_FUN(do_mpreset);
DECLARE_DO_FUN(do_map);
DECLARE_DO_FUN(do_refresh);
DECLARE_DO_FUN(do_ignore);
DECLARE_DO_FUN(do_authorize);
DECLARE_DO_FUN(do_become);
DECLARE_DO_FUN(do_haggle);
DECLARE_DO_FUN(do_muddelete);
DECLARE_DO_FUN(do_muddescriptor);
DECLARE_DO_FUN(do_mudhome);
DECLARE_DO_FUN(do_mudtransfer);
DECLARE_DO_FUN(do_mpnote);
DECLARE_DO_FUN(do_conceal);
DECLARE_DO_FUN(do_combine);
DECLARE_DO_FUN(do_cr);
DECLARE_DO_FUN(do_camp);
DECLARE_DO_FUN(do_mppassport);
DECLARE_DO_FUN(do_mppeace);
DECLARE_DO_FUN(do_chunk);
DECLARE_DO_FUN(do_approve);
DECLARE_DO_FUN(do_disapprove);
DECLARE_DO_FUN(do_sing);
DECLARE_DO_FUN(do_mpanswer);
DECLARE_DO_FUN(do_mppktimer);
DECLARE_DO_FUN(do_chatter);
DECLARE_DO_FUN(do_credits);
