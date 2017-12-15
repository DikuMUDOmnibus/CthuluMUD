CC      = g++
PROF    = -Dyog   
C_FLAGS = -O3 -Wall -g3 $(PROF)
L_FLAGS = -O -lcrypt $(PROF)

#Uncomment to enable Eliza chat - it is a separate download requirement.
#ELIZA_CHAT = 1

O_FILES = act_comm.o act_creative.o act_info.o act_move.o act_obj.o \
          act_wiz.o inter_act.o comm.o const.o conv.o db.o db2.o mem.o fight.o handler.o \
          interp.o casting.o magic.o spell.o notify.o save.o special.o update.o \
          mob_commands.o mob_prog.o obj_cond.o lease.o bit.o string.o olc.o \
          olc_act.o olc_save.o olc_plr.o olc_help.o olc_soc.o board.o locker.o bank.o \
          skill.o affect.o exp.o prof.o econd.o mob.o doors.o wev.o deeds.o \
          gadgets.o triggers.o society.o current.o monitor.o anchor.o rooms.o profile.o \
          chat.o quests.o online.o text.o tracks.o debate.o duel.o race.o nightsky.o \
          mission.o music.o vlib.o cult.o map.o partner.o

ifdef ELIZA_CHAT
   O_FILES := eliza.o $(O_FILES)
   C_FLAGS := $(C_FLAGS) -DELIZA_CHAT
endif

cthulhu: $(O_FILES)
	rm -f cthulhu
	$(CC) -o cthulhu $(O_FILES) $(L_FLAGS)

act_comm.o: act_comm.c everything.h cthulhu.h affect.h mob.h wev.h skill.h \
                       profile.h bank.h text.h race.h exp.h board.h partner.h gsn.h
	$(CC) -c $(C_FLAGS) $< 

act_info.o: act_info.c everything.h cthulhu.h magic.h statdesc.h skill.h fight.h \
                       affect.h exp.h spell.h prof.h doors.h society.h race.h gsn.h \
                       profile.h mob.h race.h vlib.h cult.h bank.h partner.h mob.h wev.h
	$(CC) -c $(C_FLAGS) $< 

act_move.o: act_move.c everything.h cthulhu.h skill.h affect.h doors.h mob.h \
                       wev.h profile.h olc.h tracks.h exp.h society.h gsn.h partner.h
	$(CC) -c $(C_FLAGS) $< 

act_obj.o: act_obj.c everything.h cthulhu.h skill.h spell.h doors.h econd.h \
                     mob.h wev.h affect.h exp.h magic.h profile.h olc.h race.h\
                     bank.h gsn.h cult.h
	$(CC) -c $(C_FLAGS) $< 

act_wiz.o: act_wiz.c everything.h cthulhu.h skill.h affect.h exp.h spell.h \
                     prof.h econd.h profile.h race.h vlib.h partner.h version.h \
                     mob.h wev.h
	$(CC) -c $(C_FLAGS) $< 

inter_act.o: inter_act.c everything.h cthulhu.h affect.h skill.h gsn.h \
                       profile.h exp.h olc.h race.h
	$(CC) -c $(C_FLAGS) $< 

nightsky.o: nightsky.c everything.h cthulhu.h 
	$(CC) -c $(C_FLAGS) $< 

mission.o: mission.c everything.h deeds.h cthulhu.h
	$(CC) -c $(C_FLAGS) $< 

comm.o: comm.c everything.h cthulhu.h skill.h exp.h mob.h wev.h profile.h \
                olc.h version.h text.h race.h econd.h cult.h board.h partner.h \
              	gsn.h
	$(CC) -c $(C_FLAGS) $< 

const.o: const.c everything.h cthulhu.h magic.h fight.h
	$(CC) -c $(C_FLAGS) $< 

db.o: db.c everything.h cthulhu.h db.h descrip.h skill.h affect.h exp.h prof.h \
           econd.h conv.h mob.h doors.h wev.h deeds.h gadgets.h triggers.h \
           society.h monitor.h fight.h current.h profile.h quests.h tracks.h \
           race.h olc.h music.h cult.h board.h partner.h gsn.h help.h
	$(CC) -c $(C_FLAGS) $< 

db2.o: db2.c everything.h cthulhu.h db.h skill.h econd.h conv.h mob.h deeds.h \
             triggers.h monitor.h chat.h race.h anchor.h
	$(CC) -c $(C_FLAGS) $< 

fight.o: fight.c everything.h cthulhu.h skill.h fight.h exp.h affect.h gsn.h \
                 doors.h mob.h wev.h profile.h race.h olc.h cult.h
	$(CC) -c $(C_FLAGS) $< 

act_creative.o: act_creative.c everything.h cthulhu.h skill.h fight.h exp.h affect.h gsn.h \
                 doors.h mob.h wev.h tracks.h olc.h econd.h
	$(CC) -c $(C_FLAGS) $< 

handler.o: handler.c everything.h cthulhu.h skill.h affect.h mob.h deeds.h \
                   monitor.h profile.h affect.h quests.h econd.h race.h gsn.h partner.h \
	   olc.h
	$(CC) -c $(C_FLAGS) $< 

interp.o: interp.c everything.h cthulhu.h interp.h mob.h wev.h profile.h
	$(CC) -c $(C_FLAGS) $< 

casting.o: casting.c everything.h cthulhu.h skill.h fight.h affect.h spell.h exp.h \
                 profile.h wev.h mob.h econd.h rooms.h society.h gsn.h music.h 
	$(CC) -c $(C_FLAGS) $< 

magic.o: magic.c everything.h cthulhu.h skill.h fight.h affect.h spell.h exp.h \
                 profile.h wev.h mob.h econd.h tracks.h  olc.h race.h gsn.h magic.h
	$(CC) -c $(C_FLAGS) $< 

notify.o: notify.c everything.h cthulhu.h profile.h wev.h mob.h
	$(CC) -c $(C_FLAGS) $< 

save.o: save.c everything.h cthulhu.h board.h skill.h spell.h affect.h exp.h \
               prof.h conv.h deeds.h triggers.h quests.h race.h board.h gsn.h
	$(CC) -c $(C_FLAGS) $< 

special.o: special.c everything.h cthulhu.h magic.h affect.h spell.h exp.h \
                     society.h race.h profile.h skill.h mob.h wev.h map.h
	$(CC) -c $(C_FLAGS) $< 

update.o: update.c everything.h cthulhu.h skill.h exp.h affect.h mob.h doors.h \
                  wev.h profile.h chat.h online.h race.h cult.h bank.h spell.h gsn.h
	$(CC) -c $(C_FLAGS) $< 

mob_commands.o: mob_commands.c everything.h cthulhu.h interp.h conv.h mob.h \
                               exp.h deeds.h wev.h chat.h quests.h spell.h
	$(CC) -c $(C_FLAGS) $< 

mob_prog.o: mob_prog.c everything.h cthulhu.h
	$(CC) -c $(C_FLAGS) $< 

obj_cond.o: obj_cond.c everything.h cthulhu.h
	$(CC) -c $(C_FLAGS) $< 

lease.o: lease.c everything.h cthulhu.h
	$(CC) -c $(C_FLAGS) $< 

olc.o: olc.c everything.h cthulhu.h olc.h profile.h
	$(CC) -c $(C_FLAGS) $< 

bit.o: bit.c everything.h cthulhu.h profile.h
	$(CC) -c $(C_FLAGS) $< 

string.o: string.c everything.h cthulhu.h text.h
	$(CC) -c $(C_FLAGS) $< 

olc_act.o: olc_act.c everything.h cthulhu.h olc.h skill.h spell.h prof.h \
                     econd.h conv.h mob.h doors.h gadgets.h deeds.h \
                     profile.h text.h race.h vlib.h society.h
	$(CC) -c $(C_FLAGS) $< 

mem.o: mem.c everything.h cthulhu.h econd.h doors.h deeds.h triggers.h \
             monitor.h tracks.h skill.h
	$(CC) -c $(C_FLAGS) $< 

olc_save.o: olc_save.c everything.h cthulhu.h olc.h spell.h prof.h econd.h \
                       conv.h mob.h doors.h gadgets.h deeds.h triggers.h \
                       monitor.h chat.h current.h tracks.h race.h anchor.h
	$(CC) -c $(C_FLAGS) $< 

olc_help.o: olc_help.c everything.h cthulhu.h olc.h help.h econd.h mob.h \
	profile.h
	$(CC) -c $(C_FLAGS) $< 

olc_soc.o: olc_soc.c everything.h cthulhu.h olc.h
	$(CC) -c $(C_FLAGS) $< 

olc_plr.o: olc_plr.c everything.h cthulhu.h olc.h doors.h gadgets.h deeds.h \
                       profile.h text.h
	$(CC) -c $(C_FLAGS) $< 

board.o: board.c everything.h cthulhu.h colordef.h board.h partner.h
	$(CC) -c $(C_FLAGS) $< 

locker.o: locker.c everything.h cthulhu.h spell.h affect.h skill.h profile.h
	$(CC) -c $(C_FLAGS) $<

bank.o: bank.c everything.h cthulhu.h bank.h olc.h
	$(CC) -c $(C_FLAGS) $<

skill.o: skill.c everything.h cthulhu.h skill.h prof.h econd.h mob.h society.h \
                 exp.h wev.h race.h gsn.h
	$(CC) -c $(C_FLAGS) $<

affect.o: affect.c everything.h cthulhu.h affect.h skill.h race.h olc.h
	$(CC) -c $(C_FLAGS) $<

spell.o: spell.c everything.h cthulhu.h spell.h affect.h wev.h econd.h \
                 fight.h music.h
	$(CC) -c $(C_FLAGS) $<

exp.o: exp.c everything.h cthulhu.h exp.h prof.h mob.h affect.h profile.h skill.h \
	gsn.h
	$(CC) -c $(C_FLAGS) $<

prof.o: prof.c everything.h cthulhu.h prof.h skill.h econd.h mob.h profile.h
	$(CC) -c $(C_FLAGS) $<

econd.o: econd.c everything.h cthulhu.h econd.h prof.h skill.h conv.h deeds.h \
                 gadgets.h affect.h society.h quests.h race.h profile.h partner.h
	$(CC) -c $(C_FLAGS) $<

conv.o: conv.c everything.h cthulhu.h conv.h econd.h mob.h skill.h gsn.h
	$(CC) -c $(C_FLAGS) $<

mob.o: mob.c everything.h cthulhu.h mob.h skill.h profile.h
	$(CC) -c $(C_FLAGS) $<

doors.o: doors.c everything.h cthulhu.h doors.h mob.h
	$(CC) -c $(C_FLAGS) $<

wev.o: wev.c everything.h cthulhu.h wev.h mob.h triggers.h current.h rooms.h \
             exp.h profile.h
	$(CC) -c $(C_FLAGS) $<

deeds.o: deeds.c everything.h cthulhu.h deeds.h
	$(CC) -c $(C_FLAGS) $<

quests.o: quests.c everything.h cthulhu.h quests.h
	$(CC) -c $(C_FLAGS) $<

gadgets.o: gadgets.c everything.h cthulhu.h econd.h mob.h wev.h
	$(CC) -c $(C_FLAGS) $<

triggers.o: triggers.c everything.h cthulhu.h triggers.h wev.h mob.h econd.h \
                       olc.h society.h chat.h skill.h
	$(CC) -c $(C_FLAGS) $<

society.o: society.c everything.h cthulhu.h society.h econd.h prof.h mob.h wev.h \
                     bank.h
	$(CC) -c $(C_FLAGS) $<

current.o: current.c everything.h cthulhu.h current.h econd.h doors.h wev.h \
                     mob.h olc.h
	$(CC) -c $(C_FLAGS) $<

monitor.o: monitor.c everything.h cthulhu.h monitor.h olc.h wev.h
	$(CC) -c $(C_FLAGS) $<

anchor.o: anchor.c everything.h cthulhu.h anchor.h olc.h wev.h
	$(CC) -c $(C_FLAGS) $<

rooms.o: rooms.c everything.h cthulhu.h rooms.h
	$(CC) -c $(C_FLAGS) $<

profile.o: profile.c everything.h cthulhu.h profile.h
	$(CC) -c $(C_FLAGS) $<

chat.o: chat.c everything.h chat.h mob.h olc.h triggers.h
	$(CC) -c $(C_FLAGS) $<

online.o: online.c everything.h cthulhu.h online.h profile.h version.h
	$(CC) -c $(C_FLAGS) $<

text.o: text.c everything.h cthulhu.h text.h
	$(CC) -c $(C_FLAGS) $<

tracks.o: tracks.c everything.h cthulhu.h tracks.h skill.h mob.h olc.h race.h \
	gsn.h
	$(CC) -c $(C_FLAGS) $<

debate.o: debate.c everything.h cthulhu.h skill.h mob.h wev.h exp.h gsn.h
	$(CC) -c $(C_FLAGS) $<

duel.o: duel.c everything.h cthulhu.h mob.h wev.h exp.h magic.h
	$(CC) -c $(C_FLAGS) $<

race.o: race.c everything.h cthulhu.h race.h society.h skill.h affect.h
	$(CC) -c $(C_FLAGS) $<

music.o: music.c everything.h cthulhu.h music.h wev.h mob.h gsn.h skill.h
	$(CC) -c $(C_FLAGS) $<

cult.o: cult.c everything.h cthulhu.h cult.h mob.h econd.h
	$(CC) -c $(C_FLAGS) $<

vlib.o: vlib.c everything.h cthulhu.h vlib.h
	$(CC) -c $(C_FLAGS) $<

map.o: map.c everything.h cthulhu.h doors.h vlib.h map.h
	$(CC) -c $(C_FLAGS) $<

partner.o: partner.c everything.h cthulhu.h partner.h profile.h version.h interp.h race.h \
	  society.h wev.h mob.h text.h board.h gsn.h
	$(CC) -c $(C_FLAGS) $<

ifdef ELIZA_CHAT
eliza.o: eliza.c everything.h eliza.h
	$(CC) -c $(C_FLAGS) $<
endif

clean:
	rm *.o cthulhu

.c.o: cthulhu.h
	$(CC) -c $(C_FLAGS) $<
