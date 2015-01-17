//
//  levels.h
//  Claws
//
//  Created by Glenn Hill on 10/01/2015.
//  Copyright (c) 2015 com.unwisegames. All rights reserved.
//

#ifndef Claws_levels_h
#define Claws_levels_h

struct LevelParams {
    int     characters;
    int     grey_bats;
    int     yellow_bats;
    float   bird_freq = 3.5;
    float   bird_speed = 1;
    int     max_simul_bats = 1; // Max bats created simultaneously
};

LevelParams const lp[20] = {
    // Level        Chars   Grey    Yellow  Freq    Speed   Simul
    /* 1    */ {    8,      3,      0,      4.0,    1,      1 },
    /* 2    */ {    8,      5,      0,      3.5,    1,      1 },
    /* 3    */ {    8,      8,      0,      3.5,    1,      1 },
    /* 4    */ {    8,      10,     0,      3.5,    1,      1 },
    /* 5    */ {    7,      10,     0,      3.5,    1,      1 },
    /* 6    */ {    7,      11,     0,      3.0,    1,      2 },
    /* 7    */ {    7,      13,     0,      3.0,    1,      2 },
    /* 8    */ {    7,      15,     0,      3.0,    1,      2 },
    /* 9    */ {    7,      16,     0,      3.0,    1,      2 },
    /* 10   */ {    6,      10,     1,      3.5,    1.2,    2 },
    /* 11   */ {    6,      10,     2,      3.5,    1.2,    2 },
    /* 12   */ {    6,      11,     3,      3.5,    1.2,    2 },
    /* 13   */ {    6,      12,     5,      3.5,    1.2,    2 },
    /* 14   */ {    6,      14,     6,      3.0,    1.2,    3 },
    /* 15   */ {    6,      15,     6,      3.0,    1.3,    3 },
    /* 16   */ {    6,      18,     5,      3.0,    1.3,    3 },
    /* 17   */ {    6,      21,     5,      3.0,    1.3,    3 },
    /* 18   */ {    6,      19,     7,      3.0,    1.3,    3 },
    /* 19   */ {    5,      20,     8,      3.0,    1.3,    3 },
    /* 20   */ {    5,      23,     10,     3.0,    1.4,    3 },
};

#endif
