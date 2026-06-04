// https://falloutmods.fandom.com/wiki/MAP_File_Format
#include <memory.h>
#include "map_map_parser.h"
#include "B_Endian/B_Endian.h"

enum flags
{
    MAP_IS_SAVEGAME = 0x1,  // map is a savegame map (.SAV).
    MAP_ELEV_0      = 0x2,  // map has an elevation at level 0.
    MAP_ELEV_1      = 0x4,  // map has an elevation at level 1.
    MAP_ELEV_2      = 0x8   // map has an elevation at level 2.
};

map_header parse_header(uint8_t* map_file)
{
    map_header h;
    h.version        = B_Endian::read_u32(&map_file[0]);
    memcpy(            h.filename,        &map_file[4], 16);
    h.dude_start     = B_Endian::read_i32(&map_file[20]);
    h.elev_start     = B_Endian::read_i32(&map_file[24]);
    h.face_start     = B_Endian::read_i32(&map_file[28]);
    h.local_var_cnt  = B_Endian::read_i32(&map_file[32]);
    h.map_script_id  = B_Endian::read_i32(&map_file[36]);
    h.map_flags      = B_Endian::read_i32(&map_file[40]);
    h.light_level    = B_Endian::read_i32(&map_file[44]);
    h.map_var_cnt    = B_Endian::read_i32(&map_file[48]);
    h.map_id         = B_Endian::read_i32(&map_file[52]);
    h.game_ticks     = B_Endian::read_u32(&map_file[56]);
    memcpy(            h.unknown,         &map_file[60], 4*44);

    return h;
}

void parse_map_map(map_lvls* map)
{
    map_header h = parse_header(map->data);

    // when a level is marked that means there's NO level information (ffs why do it that way?)
    if (!(h.map_flags & MAP_ELEV_0)) {
        //QTODO: replace "Level 0" etc. with proper markers (possibly ptrs to map level data?)
        map->level[0] = "Level 0";
    }
    if (!(h.map_flags & MAP_ELEV_1)) {
        map->level[1] = "Level 1";
    }
    if (!(h.map_flags & MAP_ELEV_2)) {
        map->level[2] = "Level 2";
    }

}


