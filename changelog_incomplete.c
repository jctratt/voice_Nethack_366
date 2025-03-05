// ###########################################################################################
//  win/curses/cursinvt.c
// ###########################################################################################
// :ret(ab)
// include gauntlets-of-reflection-3.4.3.diff
// helmet, fedora = CLAIRVOYANT

        /* change the ')' in Inventory menu to + or - for uppercase or lowercase letters 
           this is permenant inventory window */
        if ( accelerator >= 'A' && accelerator <= 'Z' )
            wprintw(win, "+ ");
        else
            wprintw(win, "- ");
        // see win/curses/cursdial.c below for 'i' inventory
// ###########################################################################################
// win/curses/cursdial.c
// ###########################################################################################
        /* change the ')' in Inventory menu to + or - for uppercase or lowercase letters
           and change < to > for selcted items while retaining + or -  */

1154             if (menu_item_ptr->selected) {
1155                 curses_toggle_color_attr(win, HIGHLIGHT_COLOR, A_REVERSE, ON);
1156                 if (curletter >= 'A' && curletter <= 'Z' )
1157                     mvwprintw(win, menu_item_ptr->line_num + 1, 1, "+");
1158                 else
1159                     mvwprintw(win, menu_item_ptr->line_num + 1, 3, "-");
1160                 mvwaddch(win, menu_item_ptr->line_num + 1, 2, curletter);
1161                 mvwaddch(win, menu_item_ptr->line_num + 1, 3, '<');
1162                 curses_toggle_color_attr(win, HIGHLIGHT_COLOR, A_REVERSE, OFF);
1163             } else {
1164                 curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, ON);
1165                 mvwaddch(win, menu_item_ptr->line_num + 1, 2, curletter);
1166                 curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, OFF);
1167                 if (curletter >= 'A' && curletter <= 'Z' )
1168                     mvwprintw(win, menu_item_ptr->line_num + 1, 1, "+");
1169                 else
1170                     mvwprintw(win, menu_item_ptr->line_num + 1, 1, "-");
1171                 mvwaddch(win, menu_item_ptr->line_num + 1, 3, ')');
1172             }

// for select and deselect of menu items
1503     if (operation == DESELECT || (item->selected && operation == INVERT)) {
1504         item->selected = FALSE;
1505         if (visible) {
1506                 if (curletter >= 'A' && curletter <= 'Z' )
1507                     mvwaddch(win, item->line_num + 1, 1, '+');
1508                 else
1509                     mvwaddch(win, item->line_num + 1, 1, '-');
1510             curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, ON);
1511             mvwaddch(win, item->line_num + 1, 2, curletter);
1512             curses_toggle_color_attr(win, HIGHLIGHT_COLOR, NONE, OFF);
1513             mvwaddch(win, item->line_num + 1, 3, ')');
1514         }

            // note 1, 1 designates [row], [column]; row is 1.
            // ################################################


// ###########################################################################################
/* src/allmain.c */
// ###########################################################################################

                    if ((moves % 600)+61 == 600)
                        pline("Save in 50 moves!");
188,191d187
<                     if ((moves % 600)+11 == 600)
<                         iflags.autosave=1;
<                     if (iflags.autosave > 0)
<                         iflags.autosave--;
349,350d344
< 
<         time_check();

void
stop_occupation()
{
    if (occupation && !is_digging()) { /* added digging */

// ###########################################################################################
/* src/cmd.c */
// ###########################################################################################

/* change 'or' to '|' in extended commands
 641             prevaccelerator = accelerator;
 642             if (!acount || one_per_line) {
 643                 Sprintf(prompt, "%s%s [%s]", wastoolong ? "| " : "",
 644                         choices[i]->ef_txt, choices[i]->ef_desc);
 645             } else if (acount == 1) {
 646                 Sprintf(prompt, "%s%s | %s", wastoolong ? "| " : "",
 647                         choices[i - 1]->ef_txt, choices[i]->ef_txt);
 648             } else {
 649                 Strcat(prompt, " | ");
 650                 Strcat(prompt, choices[i]->ef_txt);

 /* change some exteded command names so they autocomplete correctly */

    { 'T', "takeoff", "take off one piece of armor", dotakeoff },
    { 'T', "tk-takeoffitem", "take off one piece of armor", dotakeoff },
    { 'A', "takeoffall", "remove all armor", doddoremarm },
    { 'A', "ta-takeoffall", "remove all armor", doddoremarm },

    { 'd', "drop", "drop an item", dodrop },
    { 'd', "dropitem", "drop an item", dodrop },
    { 'D', "droptype", "drop specific item types", doddrop },
    { 'D', "dt-droptype", "drop specific item types", doddrop },

    { '`', "knownclass", "show discovered types for one class of objects",
    { '`', "kc-knownclass", "show discovered types for one class of objects",

    { M('v'), "version",
    { M('v'), "vc-versioncompile",
    { 'v', "versionshort", "show version", doversion, IFBURIED | GENERALCMD },
    { 'v', "vs-versionshort", "show version", doversion, IFBURIED | GENERALCMD },

// ###########################################################################################
/* include/color.h */
// ###########################################################################################

// change 0 to 3 if black is navy blue
#define CLR_BLACK 0
#define CLR_BLACK 3

#define CLR_RED 1
#define CLR_GREEN 2
#define CLR_BROWN 3 /* on IBM, low-intensity yellow is brown */

// ###########################################################################################
/* src/drawing.c */
// ###########################################################################################

// Change color of Stairs and dark room

163 /*20*/ { '.', "dark part of a room", C(CLR_BROWN) }, /* dark room */
164        { '#', "corridor", C(CLR_GRAY) },             /* dark corr */
165        { '#', "lit corridor", C(CLR_GRAY) },   /* lit corr (see mapglyph.c) */
166        { '<', "staircase up", C(CLR_RED) },         /* upstair */
167        { '>', "staircase down", C(CLR_GREEN) },       /* dnstair */

// ###########################################################################################
// src/botl.c
// ###########################################################################################

383     if(sengr_at("Elbereth", u.ux, u.uy, TRUE))
384         //Strcpy(nb = eos(nb), " Elbereth");
385 ╙───╙───utotal=777;
386     return utotal;


boolean saveit = TRUE;
boolean luckit = TRUE;

    /* Time (moves) */
    blstats[idx][BL_TIME].a.a_long = moves;
    if ((moves % 300)+50 == 300) {
        if (saveit)
        {
            pline("SAVE GAME!!");
            saveit=FALSE;
        }
    } else {
        saveit=TRUE;
    }
    if ((moves % 600)+50 == 600) {
        if (luckit)
        {
            pline("LUCK in 50 moves!!");
            luckit=FALSE;
        }
    } else {
        luckit=TRUE;
    }


    IMPORTANT: Makefile:
        vim ~/source/Nethack366_patch/sys/unix/hints/linux
            uncomment:  #CFLAGS+=-DSCORE_ON_BOTL
        cd ~/source/Nethack366_patch/sys/unix
            sh setup.sh hints/linux
        cd ~/source/Nethack366_patch/
            make clean ; make all ; make install

// ###########################################################################################
// src/display.c
// ###########################################################################################

 482         //if (mon->mtame && !Hallucination) {
 483         if (mon->mpeaceful && !Hallucination) {

// ###########################################################################################
// src/dothrow.c
// ###########################################################################################
 23 static NEARDATA const char toss_objs[] = { ALLOW_COUNT, COIN_CLASS,
 24                                            //ALL_CLASSES, WEAPON_CLASS, 0 };
                                               //ALL_CLASSES, WEAPON_CLASS, FOOD_CLASS, GEM_CLASS,0 };
 25                                           ALL_CLASSES, WEAPON_CLASS, FOOD_CLASS, GEM_CLASS, POTION_CLASS, 0 };
 26/* different default choices when wielding a sling (gold must be included) */
 27static NEARDATA const char bullets[] = { ALLOW_COUNT, COIN_CLASS, ALL_CLASSES,
 28                                         //GEM_CLASS, 0 };
 29                                         GEM_CLASS, POTION_CLASS, 0 };

// ###########################################################################################
// src/do.c
// ###########################################################################################
static NEARDATA const char drop_types[] = { ALLOW_COUNT, COIN_CLASS,
//                                            ALL_CLASSES, 0 };
//static NEARDATA const char drop_types[] = { ALLOW_COUNT, /*COIN_CLASS,*/
                                            ALL_CLASSES,
    AMULET_CLASS, WEAPON_CLASS, ARMOR_CLASS, FOOD_CLASS,
    SCROLL_CLASS, SPBOOK_CLASS, POTION_CLASS, RING_CLASS, WAND_CLASS,
    TOOL_CLASS, /*GEM_CLASS, ROCK_CLASS,*/ BALL_CLASS, CHAIN_CLASS, 0,
    };

/* 'd' command: drop one inventory item */
// ###########################################################################################
// include/winX.h
// ###########################################################################################

#define MAX_HISTORY 6000             /* max history saved on message window */


// ###########################################################################################
// src/hacklib.c
// ###########################################################################################

1129 int
1130 night()
1131 {
1132     register int hour = getlt()->tm_hour;
1133
1134     return (hour < 0 || hour > 24);
1135     //return (hour < 6 || hour > 21);
1136 }
1137
1138 int
1139 midnight()
1140 {
1141     return (getlt()->tm_hour == 25);
1142     //return (getlt()->tm_hour == 0);
1143 }

// ###########################################################################################
// src/hacklib.c
// ###########################################################################################

// ###########################################################################################
// win/curses/cursstat.c
// ###########################################################################################
## original
2160     /* Encumbrance */
2161     int enc = near_capacity();
2162     statprob(enc_stat[enc], enc > UNENCUMBERED);
2163 #undef statprob

## revised
2160     /* Encumbrance */
2161     int enc = near_capacity();
2162     statprob(enc_stat[enc], enc > UNENCUMBERED);
2163     if (sengr_at("Elbereth", u.ux, u.uy)) {
2164         statprob("Elbereth", !Blind);
2165         pline("What's here?");
2166 #undef statprob

// ###########################################################################################
// win/curses/cursmesg.c
// ###########################################################################################

## This changes the active message line color.
## Change line 140, 153, 160 from NONE to CLR_GREEN
138     bold = (height > 1 && !last_messages);
139     if (bold)
140         curses_toggle_color_attr(win, CLR_GREEN, A_BOLD, ON);
141 
142     /* will this message fit as-is or do we need to split it? */
143     if (mx == border_space && message_length > width - 2) {
144         /* split needed */
145         tmpstr = curses_break_str(message, (width - 2), 1);
146         mvwprintw(win, my, mx, "%s", tmpstr), mx += (int) strlen(tmpstr);
147         /* one space to separate first part of message from rest [is this
148            actually useful?] */
149         if (mx < width - 2)
150             ++mx;
151         free(tmpstr);
152         if (bold)
153             curses_toggle_color_attr(win, CLR_GREEN, A_BOLD, OFF);
154         tmpstr = curses_str_remainder(message, (width - 2), 1);
155         curses_message_win_puts(tmpstr, TRUE);
156         free(tmpstr);
157     } else {
158         mvwprintw(win, my, mx, "%s", message), mx += message_length;
159         if (bold)
160             curses_toggle_color_attr(win, CLR_GREEN, A_BOLD, OFF);
161     }
162     wrefresh(win);
// add text to speech
    int status;
    char txt2sp[255] = "";
    char tosay[255] = "";
    char sayit[255] = "";
    char *match;
    char *pch;

    //match = "Welcome";


        //memcpy(txt2sp,"/usr/bin/pico2wave -w=/tmp/test.wav ", 255);
        memcpy(txt2sp,"/usr/bin/espeak -s 170 -z ", 255);
        txt2sp[255] = '\0';
        memcpy(tosay, message, 255);
        tosay[255] = '\0';
        memcpy(sayit, txt2sp, 36);
        sayit[36] = '\0';
        strcat(sayit, "\"");
        strcat(sayit, tosay);
        strcat(sayit, "\"");
    status = 0;
    match = "welcome"; pch = strstr(message, match); if (pch) status++;
    match = "What do you want to rub on the stone"; pch = strstr(message, match); if (pch) status++;
    match = "What do you want to drop"; pch = strstr(message, match); if (pch) status++;
    match = "What do you want to use or apply"; pch = strstr(message, match); if (pch) status++;
    match = "Choice: h"; pch = strstr(message, match); if (pch) status++;
    match = "Choice: l"; pch = strstr(message, match); if (pch) status++;
    match = "Choice: p"; pch = strstr(message, match); if (pch) status++;
    match = "Choice: b"; pch = strstr(message, match); if (pch) status++;
    match = "Choice: g"; pch = strstr(message, match); if (pch) status++;
    match = "Choice: o"; pch = strstr(message, match); if (pch) status++;
    match = "Choice: s"; pch = strstr(message, match); if (pch) status++;
    match = "Choice: k"; pch = strstr(message, match); if (pch) status++;
    match = "You put "; pch = strstr(message, match); if (pch) status++;
    match = "You take "; pch = strstr(message, match); if (pch) status++;
    match = "You write in the dust with your fingertip"; pch = strstr(message, match); if (pch) status++;
    match = "Unknown command"; pch = strstr(message, match); if (pch) status++;
    match = "What do you want to throw"; pch = strstr(message, match); if (pch) status++;
    match = "What do you want to remove"; pch = strstr(message, match); if (pch) status++;
    match = "What do you want to put on"; pch = strstr(message, match); if (pch) status++;
    match = "You were wearing"; pch = strstr(message, match); if (pch) status++;
    match = "loot it?"; pch = strstr(message, match); if (pch) status++;
    match = "With great effort you move the boulder"; pch = strstr(message, match); if (pch) status++;
    match = "For instructions type"; pch = strstr(message, match); if (pch) status++;
    match = "Z"; pch = strstr(message, match); if (pch) status++;
    if ( status )
    {
    //    status = system(sayit);
    }
    else
    {
        status = system(sayit);
    }

// ###########################################################################################
// win/tty/wintty.c
// ###########################################################################################

1458         else if (iflags.msg_history > 6000)
1459             iflags.msg_history = 6000;

// ###########################################################################################
// src/objnam.c
// ###########################################################################################

48 STATIC_OVL struct Jitem Japanese_items[] = { { SHORT_SWORD, "wakizashi (short_sword)" },
                                             { BROADSWORD, "ninja-to (broadsword)" },
                                             { FLAIL, "nunchaku (flail)" },
                                             { GLAIVE, "naginata (glaive)" },
                                             { LOCK_PICK, "osaku (lock pick)" },
                                             { WOODEN_HARP, "koto (wooden harp)" },
                                             { KNIFE, "shito (knife)" },
                                             { PLATE_MAIL, "tanko (plate mail)" },
                                             { HELMET, "kabuto (helmet)" },
                                             { LEATHER_GLOVES, "yugake (leather gloves)" },
                                             { FOOD_RATION, "gunyoki (food ration)" },
                                             { POT_BOOZE, "sake (booze)" },

996         Strcat(prefix, "EMPTY ");
1038             Strcat(prefix, "LOCKED ");
1040             Strcat(prefix, "UNlocked ");

// ###########################################################################################
// src/sounds.c
// ###########################################################################################

added troublesomely, hungrily, and contentedly
                       # dog        cat        horse         peaceful    not
added "troublesomely" to howls 638, yowls 659, neighs   698, snarls 670, growls 673
added "hungrily"      to wines 643, meows 661, whinnies 700
added "contentedly"   to yipw  645, purrs 663, whickers 702

// ###########################################################################################
// src/sit.c
// ###########################################################################################

            case 9:
                /* Magical voice not affected by deafness */
                pline("A voice echoes:");
                verbalize(
                 "A curse upon thee for sitting upon this most holy throne!");
                if (Luck > 0) {
                    make_blinded(Blinded + rn1(100, 250), TRUE);
                    change_luck((Luck > 1) ? -rnd(2) : -1);
                    You_feel("unlucky."); // added
                } else
                    rndcurse();
                break;
// ###########################################################################################
// src/invent.c
// ###########################################################################################

## added lines 2700-2708

2700     /* Show weight total and item limit. */
2701     char invheading[QBUFSZ];
2702     int wcap = weight_cap();
2703     Sprintf(invheading, "Wt: %d(%d)%ld (%d/52 slots) Spd:%d Exr:%ld",
2704             inv_weight() + wcap, wcap,((long)weight_cap() - ( (long)(inv_weight()+weight_cap()) )), \
2705             inv_cnt(TRUE), youmonst.movement, context.next_attrib_check-moves);
2706     add_menu(win, NO_GLYPH, &any, 0, 0, ATR_BOLD, invheading, MENU_UNSELECTED);
2707
2708
2709  nextclass:

## original
3936     const char *ocsymfmt = "  ('%c')";
3937     const int invbuf_sympadding = 8; /* arbitrary */
## new
3936     //const char *ocsymfmt = "  ('%c')";
3937     //const int invbuf_sympadding = 8; /* arbitrary */
3938     /* less cluttered menu */
3939     const char *ocsymfmt = "[  %c  ]";
3940     const int invbuf_sympadding = 12; /* arbitrary; 12 makes symbols right align */

## added line 1828
1828     pline("Choice: %c", ilet);
1829     return otmp;

## Swapped 6 & 7 in lines 103 and 104
103             armcat[ARM_SHIRT]  = 7; /* [6] */
104             armcat[ARM_SUIT]   = 6; /* [0] */



// ###########################################################################################
/* src/do_wear.c */
// ###########################################################################################

1916                 /*do {
1917                     Sprintf(qbuf, "Which %s%s, Right or Left?",
1918                             humanoid(youmonst.data) ? "ring-" : "",
1919                             body_part(FINGER));
1920                     answer = yn_function(qbuf, "rl", '\0');
1921                     switch (answer) {
1922                     case '\0':
1923                         return 0;
1924                     case 'l':
1925                     case 'L':
1926                         mask = LEFT_RING;
1927                         break;
1928                     case 'r':
1929                     case 'R':
1930                         mask = RIGHT_RING;
1931                         break;
1932                     }
1933                 } while (!mask);*/
       add:
1934                         mask = LEFT_RING;

// ###########################################################################################
/* src/uhitm.c */
// ###########################################################################################

## original
 222             Sprintf(qbuf, "Really attack %s?", mon_nam(mtmp));
 223             if (!paranoid_query(ParanoidHit, qbuf)) {
 224                 context.move = 0;
 225                 return TRUE;
 226             }
## new
 222             Sprintf(qbuf, "Use 'F' to really attack %s?", mon_nam(mtmp));
 223             //if (!paranoid_query(ParanoidHit, qbuf)) {
 224                 context.move = 0;
 225                 return TRUE;
 226             //}



// ###########################################################################################
/* include/hack.h */
// ###########################################################################################

## original
351 #define ynaq(query) yn_function(query, ynaqchars, 'y')
## new
351 #define ynaq(query) yn_function(query, ynaqchars, 'n')

// ###########################################################################################
/* src/lock.c */
// ###########################################################################################

# change to uppercase
 391                 if (otmp->obroken)
 392                     verb = "FIX";
 393                 else if (!otmp->olocked)
 394                     verb = "LOCK", it = 1;
 395                 else if (picktyp != LOCK_PICK)
 396                     verb = "UNLOCK", it = 1;
 397                 else
 398                     verb = "PICK"

 706             mesg = " is ALREADY OPEN";
 709             mesg = " is LOCKED"

// ###########################################################################################
//win/curses/curswins.c
// ###########################################################################################

## Replace if with else so both the same; original puts inventory over map;
## changing it makes the inventory pop-up menu cover the inventory.
120     case RIGHT:
121         if (invent || (moves > 1)) {
122             //startx = (mapw + mapx + (mapb_offset * 2)) - width;
124             startx = term_cols - width;
123         } else {
124             startx = term_cols - width;
125         }


// ###########################################################################################
/* src/dog.c */
/* src/apply.c */
// ###########################################################################################

## I opted to just remove the -- in both lines and skip the message
########################################################################3
//## original line 708
//        mtmp->mtame--;
//
//## new
//        mtmp->mtame--;
//            pline("%s seems less friendly.", Monnam(mtmp));

// ###########################################################################################
// src/pray.c
// ###########################################################################################

## swapped speed & steath order
1161             } else if (!(HStealth & INTRINSIC)) {
1162                 HStealth |= FROMOUTSIDE;
1163                 pline(msg, "Stealth");
1164             } else if (!(HFast & INTRINSIC)) {
1165                 HFast |= FROMOUTSIDE;
1166                 pline(msg, "Speed");

// ###########################################################################################
// src/spell.c
// ###########################################################################################


983             intell = acurr(A_INT);
984             if (!Role_if(PM_WIZARD))
985                 intell = intell - 3; // original: intell = 10;
986             switch (intell) {

// ###########################################################################################
// src/attrib.c
// ###########################################################################################

    if (Fixed_abil || !incr)
    { pline("Too bad your abilities are fixed!"); return FALSE;}

        pline("exerchk: next check at %ld.", context.next_attrib_check);


## original
                 { 1, &(HFast), "quick", "slow" },
## new (changed all lines containing 1 to 2
                 { 2, &(HFast), "quick", "slow" },

26 } arc_abil[] = { { 1, &(HStealth), "stealthy", "" },
27                  { 2, &(HFast), "quick", "slow" },

46   mon_abil[] = { { 2, &(HFast), "quick", "slow" },

59   pri_abil[] = { { 2, &(HStealth), "stealthy", "" },

73   sam_abil[] = { { 2, &(HFast), "quick", "slow" },

// add infravision to humans ; also see monst.c
  hum_abil[] = { { 1, &HInfravision, "vision is redder", "vision is grayer" },
                 { 0, 0, 0, 0 } };

        /*debugpline3("%s, %s AEXE = %d",*/
        debugpline3("%s,%s=%d",

        /*pline("%s, %s AEXE = %d",*/
        pline("%s,%s=%d",

    if (msgflg <= 0)
        You_feel("%s%s!", (incr > 1 || incr < -1) ? (incr > 5 ? "very " : "") : "", attrstr); /* jeff vary message more */
      /*You_feel("%s%s!", (incr > 1 || incr < -1) ? "very "                   : "", attrstr);*/
    context.botl = TRUE;

// ###########################################################################################
// src/monst.c
// ###########################################################################################

// add infravision to humans
// the added part is "M3_INFRAVISION |"
    MON("human", S_HUMAN, LVL(0, 12, 10, 0, 0), G_NOGEN, /* for corpses */
        A(ATTK(AT_WEAP, AD_PHYS, 1, 6), NO_ATTK, NO_ATTK, NO_ATTK, NO_ATTK,
          NO_ATTK),
        SIZ(WT_HUMAN, 400, MS_HUMANOID, MZ_HUMAN), 0, 0,
        M1_HUMANOID | M1_OMNIVORE,
        M2_NOPOLY | M2_HUMAN | M2_STRONG | M2_COLLECT, M3_INFRAVISION | M3_INFRAVISIBLE,
        2, HI_DOMESTIC),
// ###########################################################################################
// src/monst.c
// ###########################################################################################


// ###########################################################################################
// src/potions.c
// ###########################################################################################

        } else if (!strcmp(potion_descr, "smoky")
                   && !(mvitals[PM_DJINNI].mvflags & G_GONE)
                   // always jinni for smoky
                   /*&& !rn2(POTION_OCCUPANT_CHANCE(mvitals[PM_DJINNI].born)*/)) {
            djinni_from_bottle(otmp);

// ###########################################################################################
/* src/wield.c */
// ###########################################################################################

//  remove shield restriction

    /* check shield 
    if (uarms && bimanual(obj)) {
        You("cannot %s a two-handed %s while wearing a shield.", verb,
            (obj->oclass == WEAPON_CLASS) ? "weapon" : "tool");
        return FALSE;
    } */

    } else if (wep->otyp == CORPSE && cant_wield_corpse(wep)) {
        /* hero must have been life-saved to get here; use a turn */
        res++; /* corpse won't be wielded */
    } /*else if (uarms && bimanual(wep)) {
        You("cannot wield a two-handed %s while wearing a shield.",
            is_sword(wep) ? "sword" : wep->otyp == BATTLE_AXE ? "axe"
                                                              : "weapon");
    }*/ else if (!retouch_object(&wep, FALSE)) {

    } /*else if (uarms)
        You_cant("use two weapons while wearing a shield.");*/

// ###########################################################################################
/* src/u_init.c */
// ###########################################################################################

    case PM_ARCHEOLOGIST:
        ini_inv(Archeologist);
        if (!rn2(1))
            ini_inv(Tinopener);
        else if (!rn2(1))
            ini_inv(Lamp);
        else if (!rn2(1))
            ini_inv(Magicmarker);
        knows_object(SACK);
        knows_object(TOUCHSTONE);
        skill_init(Skill_A);
        break;

// ###########################################################################################
/* src/detect.c */
// ###########################################################################################

    if (!level.flags.hero_memory || unconstrained || mdetected || odetected) {
        //flush_screen(1);                 /* flush temp screen */
        You("sense your surroundings.");
        if (extended || glyph_is_monster(glyph_at(u.ux, u.uy)))
            ter_typ |= TER_MON;
        browse_map(ter_typ, "anything of interest");
        refresh = TRUE;
    }
    reconstrain_map();
    EDetect_monsters = save_EDetect_mons;
    viz_array[u.uy][u.ux] = save_viz_uyux;

    /* replace monsters with remembered,unseen monster, then run
       see_monsters() to update visible ones and warned-of ones */
    for (zx = lo_x; zx <= hi_x; zx++)
        for (zy = lo_y; zy <= hi_y; zy++) {
            if (zx == u.ux && zy == u.uy)
                continue;
            newglyph = glyph_at(zx, zy);
            if (glyph_is_monster(newglyph)
                && glyph_to_mon(newglyph) != PM_LONG_WORM_TAIL) {
                /* map_invisible() was unconditional here but that made
                   remembered objects be forgotten for the case where a
                   monster is immediately redrawn by see_monsters() */
                if ((mtmp = m_at(zx, zy)) == 0 || !canspotmon(mtmp))
                    map_invisible(zx, zy);
            }
        }
    see_monsters();

    if (refresh)
        docrt();
}
// ###########################################################################################
/* src/objects.c */
// ###########################################################################################

/* weapons */
WEAPON("shuriken", "throwing star",
       0, 1, 0, 35,   1,   5,  8,  6, 2, P,   -P_SHURIKEN, SILVER, HI_SILVER),
WEAPON("quarterstaff", "staff",
       1, 1, 0, 11,  20,   5,  6,  6, 0, B,   P_QUARTERSTAFF, SILVER, HI_SILVER),
/* helmets */
HELM("elven leather helm", "leather hat",
     0, 0,           STEALTH,  6, 1,  3,  8,  9, 0, LEATHER, HI_LEATHER),
HELM("fedora", None,
     1, 1, CLAIRVOYANT,  0, 0,  3,  1, 10, 0, CLOTH, CLR_BROWN),
HELM("helmet", "plumed helmet",
     0, 0,  CLAIRVOYANT, 10, 1, 30, 10,  9, 0, IRON, HI_METAL),
GLOVES("gauntlets of reflection", "archery gloves",
       0, 1,  REFLECTING, 7, 1, 10, 50,  9, 0, SILVER, CLR_BROWN),
GLOVES("gauntlets of power", "riding gloves",
       0, 1,  REFLECTING, 7, 1, 30, 50, 9, 0,  SILVER, HI_SILVER),
GLOVES("gauntlets of dexterity", "fencing gloves",
       0, 1,  REFLECTING, 7, 1, 10, 50, 9, 0,    SILVER, HI_SILVER),

    /* weight changes */
WEAPON("dwarvish mattock", "broad pick",
       0, 0, 1, 13, /*12*/50,  50, 12,  8, -1, B,  P_PICK_AXE, IRON, HI_METAL),
TOOL("crystal ball", "glass orb", 0, 0, 1, 1, 15,15/*0*/, 60, GLASS, HI_GLASS),
TOOL("tinning kit",         None, 1, 0, 0, 1/*00*/, 15,1, 30, IRON, HI_METAL),
WEAPON("dwarvish mattock", "broad pick",
       0, 0, 1, 13, /*120*/50,  50, 12,  8, -1, B,  P_PICK_AXE, IRON, HI_METAL),
WEPTOOL("pick-axe", None,
        1, 0, 0, 20, 30/*100*/,  50,  6,  3, WHACK,  P_PICK_AXE, STEEL, HI_METAL),
//WEAPON("morning star", None,
       //1, 0, 0, 12, 1/*20*/,  10,  4,  6, 0, B,   P_MORNING_STAR, IRON, HI_METAL),
        /* +d4 small, +1 large */
//WEAPON("war hammer", None,
       //1, 0, 0, 15,  5/*0*/,   5,  4,  4, 0, B,   P_HAMMER, IRON, HI_METAL),
        /* +1 small */
WEAPON("quarterstaff", "staff",
       0, 0, 1, 11,  20/*40*/,   5,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD),

OBJECT(OBJ("Book of the Dead", "papyrus"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 0, 0, 2/*0*/, 10000, 0, 0, 0, 7, 20, HI_PAPER),
OBJECT(OBJ("Bell of Opening", "silver bell"),
       BITS(0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, P_NONE, SILVER),
       0, TOOL_CLASS, 0, 0, 1/*0*/, 5000, 0, 0, 0, 0, 50, HI_SILVER),
OBJECT(OBJ("Candelabrum of Invocation", "candelabrum"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, GOLD),
       0, TOOL_CLASS, 0, 0, 1, 5000, 0, 0, 0, 0, 200, HI_GOLD),
// ###########################################################################################
/* src/shk.c */
// ###########################################################################################

        if (Invis && u.usteed) { //buggy but works

        switch (sell_response ? sell_response : nyaq(qbuf)) {

                /*badinv =
                    (carrying(PICK_AXE) || carrying(DWARVISH_MATTOCK)
                     || (Fast && (sobj_at(PICK_AXE, u.ux, u.uy)
                                  || sobj_at(DWARVISH_MATTOCK, u.ux, u.uy))))*/;



// ###########################################################################################
/* src/mkobj.c */
// ###########################################################################################

        /*
         *  The weight of bags of holding is calculated as the weight
         *  of the bag plus the weight of the bag's contents modified
         *  as follows:
         *
         *      Bag status      Weight of contents
         *      ----------      ------------------
         *      cursed                  2x
         *      blessed                 x/4 [rounded up: (x+3)/4]
         *      otherwise               x/2 [rounded up: (x+1)/2]
         *
         *  The macro DELTA_CWT in pickup.c also implements these
         *  weight equations.
         */
        if (obj->otyp == BAG_OF_HOLDING)
            cwt = obj->cursed ? (cwt * 2) : obj->blessed ? ((cwt + 3) / 10)
                                                         : ((cwt + 1) / 5);

            cwt = obj->cursed ? (cwt * 2) : obj->blessed ? ((cwt + 3) / cwt) // timelord tech
                                                         : ((cwt + 1) / (cwt/2));


// ###########################################################################################
/* include/artilist.h */
// ###########################################################################################

    A("The Staff of Aesculapius", QUARTERSTAFF,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_ATTK | SPFX_INTEL | SPFX_DRLI
       | SPFX_REGEN | SPFX_DEFN),
      0, 0, DRLI(0, 0), DRLI(0, 0), NO_CARY, HEALING, A_NEUTRAL, PM_HEALER,
      NON_PM, 5000L, NO_COLOR),

    A("Mjollnir", WAR_HAMMER, /* Mjo:llnir */
      (SPFX_RESTR | SPFX_ATTK | SPFX_DEFN), 0, 0, 
      ELEC(5, 24), DFNS(AD_MAGM)/*NO_DFNS*/, NO_CARY, 0,
      A_NEUTRAL, PM_VALKYRIE, NON_PM, 4000L, NO_COLOR),

    A("The Orb of Fate", CRYSTAL_BALL,
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL | SPFX_LUCK),
      (SPFX_WARN | SPFX_HSPDAM | SPFX_HPHDAM | SPFX_SEARCH), 0, NO_ATTK, NO_DFNS, NO_CARY,
      LEV_TELE, A_NEUTRAL, PM_VALKYRIE, NON_PM, 3500L, NO_COLOR),

    A("The Heart of Ahriman", LUCKSTONE,
    /* Orb of Detection here*/
      (SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL), (SPFX_ESP | SPFX_HSPDAM | SPFX_STLTH), 0,
      NO_ATTK, NO_DFNS, CARY(AD_MAGM), LEVITATION, A_NEUTRAL, PM_BARBARIAN,
      NON_PM, 2500L, NO_COLOR),
    /* heart original */
      //(SPFX_NOGEN | SPFX_RESTR | SPFX_INTEL), SPFX_STLTH, 0,
      /* this stone does double damage if used as a projectile weapon */
      //PHYS(5, 0), NO_DFNS, NO_CARY, LEVITATION, A_NEUTRAL, PM_BARBARIAN,
      //NON_PM, 2500L, NO_COLOR),

// ###########################################################################################
/* src/artifact.c */
// ###########################################################################################

        } else if (otmp->oartifact == ART_VORPAL_BLADE)
                    dieroll=d(1,100);

                   if (dieroll <= 33 || dieroll > 66 || mdef->data == &mons[PM_JABBERWOCK]
                       || is_rider(mdef->data)) {
            static const char *const behead_msg[2] = { "%s beheads %s!",
                                                       "%s decapitates %s!" };
            pline("Vorpal:%d",dieroll); 

// ###########################################################################################
/* src/eat.c */
// ###########################################################################################

                                        > (otmp->blessed ? 500000L : 30L)


// ###########################################################################################
/* src/dog.c */
// ###########################################################################################

    else if (preferred_pet == 'c')
        return  PM_PONY;
        return  rn2(2) ? PM_PONY : PM_LITTLE_DOG;
    EDOG(mtmp)->apport = ACURR(A_CHA)+50000;
            edog->apport = 50000;

    mtmp->mtame = is_domestic(mtmp->data) ? 10 : 5;
    mtmp->mtame = is_domestic(mtmp->data) ? 20 : 20;

    /* reduce tameness for every 150 moves you are separated */
    if (mtmp->mtame) {
        int wilder = (imv + 75) / 1500;

    if (mtmp->mleashed) {
        mtmp->mtame++;
        m_unleash(mtmp, TRUE);

    if (Aggravate_monster || Conflict)
        pline("Agravated or Conflict: Tameness=%d",mtmp->mtame); /* /= 2;*/

// ###########################################################################################
/* src/dogmove.c */
// ###########################################################################################

        case CHEST:
        case LARGE_BOX:
            break; // don't pick them up
        /*  return &dummy;*/
        case UNICORN_HORN:

                if (edog->apport < 20)
                //if (edog->apport < 1)
                    edog->apport++
                    //edog->apport--;

        /* if you have dog food it'll follow you more closely */
        if (appr == 0)
            for (obj = invent; obj; obj = obj->nobj)
                if (dogfood(mtmp, obj) == DOGFOOD) {
                    appr = 1;
                    break;
                }


                { pline("apport:%d; tameness:%d hunger:%d hungertime:%ld",edog->apport++,mtmp->mtame,dog_hunger(mtmp, edog),edog->hungrytime-monstermoves);} 

// ###########################################################################################
/* src/role.c */
// ###########################################################################################

      // give the monk a horse
      PM_MONK,
      NON_PM,
      PM_PONY,
      /*NON_PM,*/

// ###########################################################################################
/* src/steed.c */
// ###########################################################################################


    /* Make the steed less tame and check if it resists */
    if (u.usteed->mtame<20)
        u.usteed->mtame++;//--;
