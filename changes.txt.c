//allmain.c
    //at the end of moveloop()
    curses_update_inventory(); // enables cursed items to be identified immediately
//allmain.c

//obsolete:
    //if ( color == 0 ) {
    //   // color = 16;  /* makes navy-blue brown
    //                init_color(CURSES_DARK_GRAY, 600, 600, 600);
    //                /* just override black colorpair entry here */
    //                init_pair(1, CURSES_DARK_GRAY, -1);
    //   //init_pair(1, 16, -1); /* 0=purple, 16=black 24=cyan*/
    //map[y][x].color = NO_COLOR;
    //map[y][x].attr = A_NORMAL;
    //} else {

    //144=purple/purple //143=purple/purple //129=purple/purple //128=purple/purple //127=white/pink //112=purple/pink //106=lime/cyan //105=orange/cyan //104=pink/cyan //96=purple/cyan
    //95=white/fucia //80=purple/fucia //64=purple/brown //63=white/green //62=cyan/green //61=fucia/green //60=blue/green //59=yellow/green //58=lime/green //57=orange/Lgreen //56=pink/Lgreen
    //55=pink/Lgreen //54=cyan/Lgreen //53=magenta/Lgreen //52=blue/Lgreen //51=brown/Lgreen //50=Lgreen/Lgreen //49=red/Lgreen //48=purple/Lgreen //47=white/Lblue //46=white/red
    //45=cyan/Lblue //44=cyan/red //43=pink/Lblue //42=pink/red //41=blue/Lblue //40=blue/red //39=yellow/yellow/Lblue //38=yellow/red //37=lime/Lblue //36=lime/red //35=orange/blue
    //34=orange/red //33=pink/blue // 32=pink/red /* 31=gray/Lblue
    /* 27=fucia/blue 28=Lgreen/red 29=Lgreen/blue 30=brown/red */
    /* 16=Darkgray/red 17=Darkgray/blue 18=red/red 19=red/lblue 20=green/red 21=gray/Lblue 22=brown/red 23=brown/blue 24=blue/red 25=Lblue/Lblue 26=magenta/red */
    /* 1=red 2=green 3=brown 4=Lblue 5=magenta 6=cyan 7=gray 8=pink 9=orange 10=green 11=yellow 12=Lblue 13=Lmagenta 14=cyan 15=white */


//    int status;
//    char txt2sp[255] = "";
//    char tosay[255] = "";
//    char sayit[255] = "";
//    char *match;
//    char *pch;
//    int ii,jj,kk=0;
//    //char str[] = "HELLO'All";
//    char rep[255]={0};
//    //match = "Welcome";
//
//        //memcpy(txt2sp,"/usr/bin/pico2wave -w=./tmp/test.wav ", 255);
//        //memcpy(txt2sp,"/usr/bin/espeak -a 90 -v en-us -g 1 -k 10 -s 250 -z ", 255);
//        /* create batch file containing:'/usr/bin/espeak -a 90 -v en-us -g 1 -k 10 -s 250 -z "$1"' */
//        memcpy(txt2sp,"~/bin/myespeak ", 255); /* requires trialing space */
//        txt2sp[255] = '\0';
//        memcpy(tosay, message, 255);
//        tosay[255] = '\0';
//
//        /* eliminate troublesome characters passed to sh */
//        kk=strlen(tosay);
//    for(ii=jj=0; ii< kk; ii++)
//    {
//      if(tosay[ii]=='\'') /* ' */
//      {
//          //pline("\nB:%s",rep);
//        if (ii >= kk) /* skip if <kk else insert NULL terminator */
//            rep[jj]='\0';
//          //pline("\nA:%s",rep);
//        continue;
//    } else if(tosay[ii]=='\"') /* " */
//      {
//        if (ii >= kk) /* skip if <kk else insert NULL terminator */
//            rep[jj]='\0';
//        continue;
//    } else if(tosay[ii]=='`') /* ` */
//      {
//        if (ii >= kk) /* skip if <kk else insert NULL terminator */
//            rep[jj]='\0';
//        continue;
//    }  else if(tosay[ii]=='|') /* | */
//      {
//        rep[jj]='_';
//        rep[++jj]=tosay[ii];
//        continue;
//    }  else if(tosay[ii]=='>') /* > */
//      {
//        rep[jj]='_';
//        rep[++jj]=tosay[ii];
//        continue;
//    } else if(tosay[ii]=='<') /* < */
//      {
//        rep[jj]='_';
//        rep[++jj]=tosay[ii];
//        continue;
//    } else if(tosay[ii]=='(') /* ( */
//      {
//        rep[jj]='_';
//        rep[++jj]=tosay[ii];
//        continue;
//    } else if(tosay[ii]==')') /* ) */
//      {
//        rep[jj]='_';
//        rep[++jj]=tosay[ii];
//        continue;
//    } else if(tosay[ii]==';') /* ; */
//      {
//        rep[jj]='_';
//        rep[++jj]=tosay[ii];
//        continue;
//    }
//
//
//    rep[jj++]=tosay[ii];
//  }
//
//        memcpy(tosay, tosay, 255);
//        tosay[255] = '\0';
//
//        memcpy(sayit, txt2sp, 17);
//        sayit[17] = '\0';
//        strcat(sayit, "\"");
//        strcat(sayit, rep);
//        strcat(sayit, "\"");
////pline("tosay: %s", sayit);
//    status = 0;
//    match = "welcome"; pch = strstr(message, match); if (pch) status++;
//    match = "What do you want to"; pch = strstr(message, match); if (pch) status++;
//    //match = "What do you want to rub on the stone"; pch = strstr(message, match); if (pch) status++;
//    //match = "What do you want to drop"; pch = strstr(message, match); if (pch) status++;
//    //match = "What do you want to use or apply"; pch = strstr(message, match); if (pch) status++;
//    match = "Choice: h"; pch = strstr(message, match); if (pch) status++;
//    match = "Choice: l"; pch = strstr(message, match); if (pch) status++;
//    match = "Choice: p"; pch = strstr(message, match); if (pch) status++;
//    match = "Choice: b"; pch = strstr(message, match); if (pch) status++;
//    match = "Choice: g"; pch = strstr(message, match); if (pch) status++;
//    match = "Choice: o"; pch = strstr(message, match); if (pch) status++;
//    match = "Choice: s"; pch = strstr(message, match); if (pch) status++;
//    match = "Choice: k"; pch = strstr(message, match); if (pch) status++;
//    match = "You put "; pch = strstr(message, match); if (pch) status++;
//    match = "You take "; pch = strstr(message, match); if (pch) status++;
//    match = "You write in the dust with your fingertip"; pch = strstr(message, match); if (pch) status++;
//    match = "Unknown command"; pch = strstr(message, match); if (pch) status++;
//    //match = "What do you want to throw"; pch = strstr(message, match); if (pch) status++;
//    //match = "What do you want to remove"; pch = strstr(message, match); if (pch) status++;
//    //match = "What do you want to put on"; pch = strstr(message, match); if (pch) status++;
//    match = "You were wearing"; pch = strstr(message, match); if (pch) status++;
//    match = "loot it?"; pch = strstr(message, match); if (pch) status++;
//    match = "With great effort you move the boulder"; pch = strstr(message, match); if (pch) status++;
//    match = "For instructions type"; pch = strstr(message, match); if (pch) status++;
//    match = "Check it for traps?"; pch = strstr(message, match); if (pch) status++;
//    match = "not wielded"; pch = strstr(message, match); if (pch) status++;
//    match = "In what direction"; pch = strstr(message, match); if (pch) status++;
//    match = "You find no traps on the"; pch = strstr(message, match); if (pch) status++;
//    match = "Really save?"; pch = strstr(message, match); if (pch) status++;
//    match = "Saving..."; pch = strstr(message, match); if (pch) status++;
//    //match = "What do you want to write with?"; pch = strstr(message, match); if (pch) status++;
//    match = "ynq"; pch = strstr(message, match); if (pch) status++;
//    match = "You wipe out the message that was written in the dust here"; pch = strstr(message, match); if (pch) status++;
//    match = "Something is written here in the dust"; pch = strstr(message, match); if (pch) status++;
//    match = "for sale"; pch = strstr(message, match); if (pch) status++;
//    match = "unpaid"; pch = strstr(message, match); if (pch) status++;
//    match = "Pick an object."; pch = strstr(message, match); if (pch) status++;
//    match = "Sell it?"; pch = strstr(message, match); if (pch) status++;
//    match = "want to name"; pch = strstr(message, match); if (pch) status++;
//    match = "want to call"; pch = strstr(message, match); if (pch) status++;
//    match = "You finish your dressing"; pch = strstr(message, match); if (pch) status++;
//    match = "You finish taking off"; pch = strstr(message, match); if (pch) status++;
//    match = "__ drops"; pch = strstr(message, match); if (pch) status++;
//    match = "__ picks up"; pch = strstr(message, match); if (pch) status++;
//    //match = "What do you want to adjust"; pch = strstr(message, match); if (pch) status++;
//    //match = "What do you want to take off"; pch = strstr(message, match); if (pch) status++;
//    match = "Swapping"; pch = strstr(message, match); if (pch) status++;
//    match = "Choice:"; pch = strstr(message, match); if (pch) status++;
//    match = "Pilgrim"; pch = strstr(message, match); if (pch) status++;
//    match = "intones"; pch = strstr(message, match); if (pch) status++;
//    match = "forbidding feeling"; pch = strstr(message, match); if (pch) status++;
//    match = "You are now wearing"; pch = strstr(message, match); if (pch) status++;
//    match = "Where do you want to travel to"; pch = strstr(message, match); if (pch) status++;
//    match = "The boulder falls into and plugs a hole in the floor"; pch = strstr(message, match); if (pch) status++;
//    match = "p = an"; pch = strstr(message, match); if (pch) status++;
//    match = "Moving"; pch = strstr(message, match); if (pch) status++;
//    match = "You hit the rock with all your might"; pch = strstr(message, match); if (pch) status++;
//    match = "You hit the boulder with all your might"; pch = strstr(message, match); if (pch) status++;
//    match = "Something is engraved here on"; pch = strstr(message, match); if (pch) status++;
//    match = "The thonged club returns"; pch = strstr(message, match); if (pch) status++;
//    match = "The aklys returns"; pch = strstr(message, match); if (pch) status++;
//    match = " - "; pch = strstr(message, match); if (pch) status++;
//    match = " = "; pch = strstr(message, match); if (pch) status++;
//    match = "Nothing happens."; pch = strstr(message, match); if (pch) status++;
//    match = "(I hope you don't mind.)"; pch = strstr(message, match); if (pch) status++;
//    match = "objects here"; pch = strstr(message, match); if (pch) status++;
//    match = "You cannot stash something you are"; pch = strstr(message, match); if (pch) status++;
//    match = "You open the"; pch = strstr(message, match); if (pch) status++;
//    match = "You drop"; pch = strstr(message, match); if (pch) status++;
//    match = "You engrave"; pch = strstr(message, match); if (pch) status++;
//    match = "You see here"; pch = strstr(message, match); if (pch) status++;
//    match = "You make an opening in the wall"; pch = strstr(message, match); if (pch) status++;
//    match = "For you,"; pch = strstr(message, match); if (pch) status++;
//    match = "You hear nothing special"; pch = strstr(message, match); if (pch) status++;
//    match = "Adjust letter to what"; pch = strstr(message, match); if (pch) status++;
//    match = "You read: \"?\"."; pch = strstr(message, match); if (pch) status++;
//    match = "Your armor is rather cumbersome"; pch = strstr(message, match); if (pch) status++;
//    match = "Talk to whom"; pch = strstr(message, match); if (pch) status++;
//    match = "murmurs in your ear"; pch = strstr(message, match); if (pch) status++;
//    match = "Time stands still"; pch = strstr(message, match); if (pch) status++;
//    match = "A boulder fill a pit"; pch = strstr(message, match); if (pch) status++;
//    match = "A boulder fills the pit"; pch = strstr(message, match); if (pch) status++;
//    match = "Shall I pick"; pch = strstr(message, match); if (pch) status++;
//    //match = "Really quit? [yn] (n)"; pch = strstr(message, match); if (pch) status++;
//    match = "You try to move the boulder, but in vain."; pch = strstr(message, match); if (pch) status++;
//    match = "There is nothing here to pick up."; pch = strstr(message, match); if (pch) status++;
//    match = "staircase"; pch = strstr(message, match); if (pch) status++;
//    match = "You fall into the water"; pch = strstr(message, match); if (pch) status++;
//    match = "ock it?"; pch = strstr(message, match); if (pch) status++;
//    match = "Really quit?"; pch = strstr(message, match); if (pch) status++;
//    match = "Unknown type of gloves"; pch = strstr(message, match); if (pch) status++;
//    match = "You try to crawl out of the water."; pch = strstr(message, match); if (pch) status++;
//    match = "Pheew!"; pch = strstr(message, match); if (pch) status++;
//    match = "That was close"; pch = strstr(message, match); if (pch) status++;
//    match = "The Soldier is not ready for battle!"; pch = strstr(message, match); if (pch) status++;
//    match = "sense of peace"; pch = strstr(message, match); if (pch) status++;
//    match = "There is a doorway here."; pch = strstr(message, match); if (pch) status++;
//    match = "There is an open door here."; pch = strstr(message, match); if (pch) status++;
//    match = "Welcome again to"; pch = strstr(message, match); if (pch) status++;
//    match = " asks you for a contribution for the temple."; pch = strstr(message, match); if (pch) status++;
//    match = "Thou art indeed a pious individual."; pch = strstr(message, match); if (pch) status++;
//    match = "You give the priest of "; pch = strstr(message, match); if (pch) status++;
//    match = "Z"; pch = strstr(message, match); if (pch) status++;
//
//    if ( status )
//    {
//    //    status = system(sayit);
//    }
//    else
//    {
//        //status = system(sayit);
//        flushinp();
//    }
