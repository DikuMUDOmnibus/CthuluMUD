/*
 * CthulhuMud
 */

#ifdef IN_DB_C
#define GSN(x) int gsn_ ##x;
#else
#define GSN(x) extern int gsn_ ## x;
#endif

GSN(survival)
GSN(backstab)
GSN(blackjack)
GSN(assassinate)
GSN(circle)
GSN(envenom)
GSN(hide)
GSN(peek)
GSN(pick_lock)
GSN(recruit)
GSN(leadership)
GSN(sneak)
GSN(steal)
GSN(teach)
GSN(disarm)
GSN(strong_grip)
GSN(kick)
GSN(strangle)
GSN(rescue)
GSN(rotate)
GSN(sharpen)
GSN(blindness)
GSN(charm_person)
GSN(curse)
GSN(invis)
GSN(mass_invis)
GSN(plague)
GSN(poison)
GSN(sleep)
GSN(search)
GSN(detection)
GSN(theology)
GSN(yuggoth)
GSN(tame)
GSN(axe)
GSN(dagger)
GSN(flail)
GSN(mace)
GSN(polearm)
GSN(spear)
GSN(sword)
GSN(whip)
GSN(staff)
GSN(axe_master)
GSN(dagger_master)
GSN(flail_master)
GSN(mace_master)
GSN(polearm_master)
GSN(spear_master)
GSN(sword_master)
GSN(whip_master)
GSN(staff_master)
GSN(gun)
GSN(handgun)
GSN(smg)
GSN(marksman)
GSN(bow)
GSN(master_archer)
GSN(hand_to_hand)
GSN(martial_arts)
GSN(black_belt)
GSN(enhanced_damage)
GSN(ultra_damage)
GSN(lethal_damage)
GSN(shield_block)
GSN(dodge)
GSN(parry)
GSN(second_attack)
GSN(third_attack)
GSN(fourth_attack)
GSN(bash)
GSN(berserk)
GSN(dual)
GSN(dirt)
GSN(trip)
GSN(tail)
GSN(crush)
GSN(fast_healing)
GSN(haggle)
GSN(lore)
GSN(meditation)
GSN(spell_casting)
GSN(ritual_mastery)
GSN(elder_magic)
GSN(scrying)
GSN(forging)
GSN(tailor)
GSN(bandage)
GSN(psychology)
GSN(intimidate)
GSN(riding)
GSN(traps)
GSN(explosives)
GSN(chemistry)
GSN(voodoo)
GSN(scrolls)
GSN(staves)
GSN(wands)
GSN(recall)
GSN(swim)
GSN(climb)
GSN(dreaming)
GSN(master_dreamer)
GSN(tracking)
GSN(debating)
GSN(interrogate)
GSN(english)
GSN(spanish)
GSN(french)
GSN(german)
GSN(gaelic)
GSN(polish)
GSN(italien)
GSN(chinese)
GSN(japanese)
GSN(hebrew)
GSN(arabic)
GSN(greek)
GSN(latin)
GSN(heiroglyphics)
GSN(old_english)
GSN(romany)
GSN(stygian)
GSN(atlantean)
GSN(cthonic)
GSN(mayan)
GSN(natural_magic)
GSN(occult)
GSN(necromancy)
GSN(tactics)
GSN(music)
GSN(singing)
GSN(percussion)
GSN(strings)
GSN(flute)
GSN(brass)
GSN(piano)
GSN(organ)
GSN(crystal)
