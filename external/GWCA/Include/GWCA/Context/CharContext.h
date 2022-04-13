#pragma once

#include <GWCA/GameContainers/Array.h>


namespace GW {
    struct ObserverMatch;

    struct ProgressBar {
        int     pips;
        uint8_t color[4];      // RGBA
        uint8_t background[4]; // RGBA
        int     unk[7];
        float   progress;      // 0 ... 1
        // possibly more
    };

    struct CharContext { // total: 0x42C
        /* +h0000 */ Array<void *> h0000;
        /* +h0010 */ uint32_t h0010;
        /* +h0014 */ Array<void *> h0014;
        /* +h0024 */ uint32_t h0024[4];
        /* +h0034 */ Array<void *> h0034;
        /* +h0044 */ Array<void *> h0044;
        /* +h0054 */ uint32_t h0054[8];
        /* +h0074 */ wchar_t player_name[0x14];
        /* +h009C */ uint32_t h009C[20];
        /* +h00EC */ Array<void *> h00EC;
        /* +h00FC */ uint32_t h00FC[37]; // 40
        /* +h0190 */ uint32_t h0190;
        /* +h0194 */ uint32_t token1; // world id
        /* +h0198 */ uint32_t map_id;
        /* +h019C */ uint32_t is_explorable;
        /* +h01A0 */ uint8_t host[0x18];
        /* +h01B8 */ uint32_t token2; // player id
        /* +h01BC */ uint32_t h01BC[25];
        /* +h0220 */ uint32_t district_number;
        /* +h0224 */ uint32_t language;
        /* +h0228 */ uint32_t observe_map_id;
        /* +h022C */ uint32_t current_map_id;
        /* +h0230 */ uint32_t h0230[2];
        /* +h0238 */ Array<ObserverMatch *> observer_matchs;
        /* +h0248 */ uint32_t h0248[23];
        /* +h02A4 */ uint32_t player_number;
        /* +h02A8 */ uint32_t h02A8[40];
        /* +h0348 */ ProgressBar *progress_bar; // seems to never be nullptr
        /* +h034C */ uint32_t h034C[27];
        /* +h03B8 */ wchar_t player_email[0x40];
    };
    static_assert(sizeof(CharContext) == 0x438, "struct CharContext has incorrect size");
}
