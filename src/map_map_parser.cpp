// https://falloutmods.fandom.com/wiki/MAP_File_Format
#include <memory.h>
#include <math.h>
#include "map_map_parser.h"
#include "B_Endian/B_Endian.h"

enum map_flags
{
    MAP_IS_SAVEGAME = 0x1,  // map is a savegame map (.SAV).
    MAP_ELEV_0      = 0x2,  // map has an elevation at level 0.
    MAP_ELEV_1      = 0x4,  // map has an elevation at level 1.
    MAP_ELEV_2      = 0x8   // map has an elevation at level 2.
};

struct vars
{
    int32_t* mvars = NULL;
    int32_t* lvars = NULL;
};
struct script
{
    int32_t scr_id;           // 0x00  Packed script id.
    int32_t scr_next;         // 0x04  Linked-list field in the original engine; usually not useful to external tools.
    int32_t spatial_tile;     // 0x08  Only present for spatial scripts. Packed tile/elevation value.
    int32_t spatial_radius;   // 0x0C  Only present for spatial scripts. Spatial trigger radius.

    // Only present for timed scripts.
    int32_t time;             // 0x08  Game-time value for the timed script.

    // After the optional spatial/timed fields,
    // every script record stores the same 14 integers:
    int32_t scr_flags;        // 0x10  (0 in maps, value in saves)
    int32_t scr_index;        // 0x14  Script id. Script filename is found in LST file script.lst at index id
    int32_t program_ptr;      // 0x18  not used?
    int32_t scr_obj_id;       // 0x1C  Object id for this script
    int32_t lvar_offset;      // 0x20  Offset into lvars data struct from earlier
    int32_t lvar_cnt;         // 0x24  (0 in maps, value in saves?)
    int32_t last_used_val;    // 0x28  possibly used for savegames?
    int32_t current_action;   // 0x2C  possibly used for savegames?
    int32_t fixed_param;      // 0x30  possibly used for savegames?
    int32_t action_id;        // 0x34  possibly used for savegames?
    int32_t override_flags;   // 0x38  possibly used for savegames?
    int32_t unknown_1;        // 0x3C  unknown
    int32_t how_much;         // 0x40  also unknown
    int32_t unknown_2;        // 0x44  also unknown
};
struct inventory
{
    int32_t inv_size;         // 0x48  Number of inventory entries owned by this object
    int32_t inv_capacity;     // 0x4C  Allocated inventory capacity in the saved object data
    int32_t* items_ptr;       // 0x50  Serialized pointer placeholder from the original engine (savegames?)
};
struct scenery
{
    // scenery instance data
    int32_t flags;            // 0x54  instance flags
    int32_t door_flags;       // 0x58  open/shut flags, stairs/elevator/ladders have destination_built_tile
    int32_t destination;      // 0x5C  destination map/level
};
struct critter
{
    // PID_TYPE == 1 // critter
    int32_t flags;            // 0x54  instance flags
    int32_t damage_last_turn; // 0x58  Combat damage tracker        Only valid for .SAV
    int32_t combat_flags;     // 0x5C  Combat maneuver flags/state  Only valid for .SAV
    int32_t action_points;    // 0x60  Current combat action points Only valid for .SAV
    int32_t combat_result;    // 0x64  Combat result flags/state    Only valid for .SAV
    int32_t ai_packet;        // 0x68  Runtime AI packet
    int32_t team;             // 0x6C  Runtime team number
    int32_t last_hit_cid;     // 0x70  Combat id of the critter that last hit this critter, or -1   Only valid for .SAV
    int32_t hit_points;       // 0x74  Current hit points
    int32_t radiation;        // 0x78  Current radiation level
    int32_t poison;           // 0x7C  Current poison level
};
struct exit_grid
{
    // exit grid runtime data
    int32_t flags;            // 0x54  instance flags
    int32_t dest_map;         // 0x58  Destination map id.
    int32_t dest_tile;        // 0x5C  Destination tile.
    int32_t dest_elev;        // 0x60  Destination elevation.
    int32_t dest_dir;         // 0x64  Destination rotation.
};
struct ammo
{
    // other object types
    int32_t flags;            // 0x54  instance flags
    int32_t ammo_amt;         // 0x58  Current rounds in the stack
    int32_t ammo_pid;         // 0x5C
};
struct object
{
    int32_t obj_id;           // 0x00  Unknown 0. I don't think this is part of the object, but some kind of separator.
    int32_t obj_tile;         // 0x04  A value of -1 means that the object is not on the grid (typically it is in an inventory)
    int32_t x;                // 0x08  Pixel x offset
    int32_t y;                // 0x0C  Pixel y offset
    int32_t sx;               // 0x10  cached screen x
    int32_t sy;               // 0x14  cached screen y
    int32_t frame;            // 0x18  current animation frame (for save?)
    int32_t rotation;         // 0x1C  object rotation
    int32_t obj_fid;//PID?    // 0x20  art FID for object
    int32_t obj_flags;        // 0x24  0x01000000 right hand, 0x02000000 left hand, 0x03000000 armor worn
    int32_t elevation;        // 0x28
    int32_t obj_pid;          // 0x2C
    int32_t obj_cid;          // 0x30  combat id (save files?) -1 for normal objects 
    int32_t light_radius;     // 0x34  in hexes
    int32_t light_intensity;  // 0x38  (0..65536, interpreted as 0-100%)
    int32_t outline_color;    // 0x3C  Outline color/state in saved maps during combat
    int32_t obj_sid;          // 0x40  runtime script id for this object
    int32_t obj_scr_id;       // 0x44  Script index from scripts.lst, or -1.
    inventory* inv;
};
struct objects_list
{
    int32_t count;
    object* objects;
};
struct scripts_list
{
    int32_t count;
    script* scripts;
};
struct tiles
{
    int32_t elev_0[10000];
    int32_t elev_1[10000];
    int32_t elev_2[10000];
};


map_header parse_header(uint8_t* map_file)
{
    map_header h;

    h.version        = B_Endian::read_u32(&map_file[0]);
    memcpy(            h.filename,        &map_file[4], 16);
    h.dude_start     = B_Endian::read_i32(&map_file[20]);
    h.elev_start     = B_Endian::read_i32(&map_file[24]);
    h.face_start     = B_Endian::read_i32(&map_file[28]);
    h.lvar_cnt       = B_Endian::read_i32(&map_file[32]);
    h.map_script_id  = B_Endian::read_i32(&map_file[36]);
    h.map_flags      = B_Endian::read_i32(&map_file[40]);
    h.light_level    = B_Endian::read_i32(&map_file[44]);
    h.mvar_cnt       = B_Endian::read_i32(&map_file[48]);
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

vars parse_map_vars(map_lvls* map)
{
    uint8_t* data = map->data;
    int mvars_cnt = map->header.mvar_cnt;
    int mvars_siz = mvars_cnt * sizeof(int32_t);
    int lvars_cnt = map->header.lvar_cnt;
    int lvars_siz = lvars_cnt * sizeof(int32_t);

    vars v;
    v.mvars = (int32_t*)malloc(mvars_siz);
    v.lvars = (int32_t*)malloc(lvars_siz);

    memcpy(v.mvars, &map->data[236], mvars_siz);
    memcpy(v.lvars, &map->data[236 + mvars_siz], lvars_siz);

    return v;
}

tiles parse_map_tiles(map_lvls* map)
{
    int copy_size = 10000*sizeof(int32_t);
    int offset = (map->header.mvar_cnt + map->header.lvar_cnt) * sizeof(int32_t);
    tiles t;

    if (!(map->header.map_flags & MAP_ELEV_0)) {
        memcpy(t.elev_0, &map->data[236 + offset], copy_size);
        offset += copy_size;
    } else {
        t.elev_0[0] = -1;
    }
    if (!(map->header.map_flags & MAP_ELEV_1)) {
        memcpy(t.elev_1, &map->data[236 + offset], copy_size);
        offset += copy_size;
    } else {
        t.elev_1[0] = -1;
    }
    if (!(map->header.map_flags & MAP_ELEV_2)) {
        memcpy(t.elev_2, &map->data[236 + offset], copy_size);
    } else {
        t.elev_2[0] = -1;
    }

    return t;
}

scripts_list parse_map_scripts(map_lvls* map)
{
    int offset = 236 + (map->header.mvar_cnt + map->header.lvar_cnt) * sizeof(int32_t);
    if (!(map->header.map_flags & MAP_ELEV_0)) {
        offset += 10000*sizeof(int32_t);
    }
    if (!(map->header.map_flags & MAP_ELEV_1)) {
        offset += 10000*sizeof(int32_t);
    }
    if (!(map->header.map_flags & MAP_ELEV_2)) {
        offset += 10000*sizeof(int32_t);
    }

    uint8_t* data_ptr = &map->data[offset];

    scripts_list scripts[5];
    for (int i = 0; i < 5; i++) {
        scripts[i].count = B_Endian::read_i32(data_ptr);
        data_ptr += 4;
        if (scripts[i].count > 0) {
            int mod_count = ceil(scripts[i].count % 16);
            if (i == SCRIPT_SYSTEM) {
                scripts[i].scripts = (script*)malloc(mod_count * sizeof(script));
                for (int j = 0; j < mod_count; j++) {
                    scripts->scripts[j].scr_id         = B_Endian::read_i32(&data_ptr[0x00]); // 0x00  Packed script id.
                    scripts->scripts[j].scr_next       = B_Endian::read_i32(&data_ptr[0x04]); // 0x04  Linked-list field in the original engine; usually no
                    scripts->scripts[j].spatial_tile   = B_Endian::read_i32(&data_ptr[0x08]); // 0x08  Only present for spatial scripts. Packed tile/elevation value
                    // Only present for timed scripts.
                    // scripts->scripts[j].time;                                              // 0x08  Game-time value for the timed script.
                    scripts->scripts[j].spatial_radius = B_Endian::read_i32(&data_ptr[0x0C]); // 0x0C  Only present for spatial scripts. Spatial trigger radius.
                    // After the optional spatial/timed fields,
                    // every script record stores the same 14 integers:
                    scripts->scripts[j].scr_flags      = B_Endian::read_i32(&data_ptr[0x10]); // 0x10  (0 in maps, value in saves)
                    scripts->scripts[j].scr_index      = B_Endian::read_i32(&data_ptr[0x14]); // 0x14  Script id. Script filename is found in LST file scri
                    scripts->scripts[j].program_ptr    = B_Endian::read_i32(&data_ptr[0x18]); // 0x18  not used?
                    scripts->scripts[j].scr_obj_id     = B_Endian::read_i32(&data_ptr[0x1C]); // 0x1C  Object id for this script
                    scripts->scripts[j].lvar_offset    = B_Endian::read_i32(&data_ptr[0x20]); // 0x20  Offset into lvars data struct from earlier
                    scripts->scripts[j].lvar_cnt       = B_Endian::read_i32(&data_ptr[0x24]); // 0x24  (0 in maps, value in saves?)
                    scripts->scripts[j].last_used_val  = B_Endian::read_i32(&data_ptr[0x28]); // 0x28  possibly used for savegames?
                    scripts->scripts[j].current_action = B_Endian::read_i32(&data_ptr[0x2C]); // 0x2C  possibly used for savegames?
                    scripts->scripts[j].fixed_param    = B_Endian::read_i32(&data_ptr[0x30]); // 0x30  possibly used for savegames?
                    scripts->scripts[j].action_id      = B_Endian::read_i32(&data_ptr[0x34]); // 0x34  possibly used for savegames?
                    scripts->scripts[j].override_flags = B_Endian::read_i32(&data_ptr[0x38]); // 0x38  possibly used for savegames?
                    scripts->scripts[j].unknown_1      = B_Endian::read_i32(&data_ptr[0x3C]); // 0x3C  unknown
                    scripts->scripts[j].how_much       = B_Endian::read_i32(&data_ptr[0x40]); // 0x40  also unknown
                    scripts->scripts[j].unknown_2      = B_Endian::read_i32(&data_ptr[0x44]); // 0x44  also unknown

                }
            }
        }
    }

}

objects_list parse_map_objects(map_lvls* map)
{
    objects_list ol;

    //QTODO: fill this out



    return ol;
}

void export_map_map(char** label_ptr_M, map_lvls* map_L, map_lvls* map_R, int header, char* path_buff)
{
    map_header* head = (header == 0) ? &map_L->header     : &map_R->header;
    int H_size       = (header == 0) ? map_L->header_size : map_R->header_size;

    vars v_L = parse_map_vars(map_L);
    vars v_R = parse_map_vars(map_R);

    tiles t_L = parse_map_tiles(map_L);
    tiles t_R = parse_map_tiles(map_R);

    scripts_list s_L = parse_map_scripts(map_L);
    scripts_list s_R = parse_map_scripts(map_R);

    objects_list o_L = parse_map_objects(map_L);
    objects_list o_R = parse_map_objects(map_R);
}
