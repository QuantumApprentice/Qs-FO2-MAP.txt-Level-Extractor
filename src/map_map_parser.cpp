// https://falloutmods.fandom.com/wiki/MAP_File_Format
// https://fodev.net/files/fo2/map.html
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
enum object_types
{
    // The PID is a fundamental type used in the MAP file.
    // It is an identifier for describing objects.
    // It consists of a 4 byte integer of the form:
    //      0xaa00bbbb.
    // The byte aa is the type of the object,
    // while the 2 bytes bbbb are the id of the object.
    // The id is typically an index into a LST file.
    // Valid types include:
    OBJ_ITEM     = 0x00,
    OBJ_CRITTER  = 0x01,
    OBJ_SCENERY  = 0x02,
    OBJ_WALL     = 0x03,
    OBJ_TILE     = 0x04,
    OBJ_MISC     = 0x05,
    OBJ_INTRFACE = 0x06,
    OBJ_INVEN    = 0x07,
    OBJ_HEAD     = 0x08,
    OBJ_BG       = 0x09
};

struct vars
{
    int32_t mvars_siz = 0;
    int32_t* mvars = NULL;
    int32_t lvars_siz = 0;
    int32_t* lvars = NULL;
};
struct script
{
    int32_t scr_id;           // 0x00  Packed script id.
    int32_t scr_next;         // 0x04  Linked-list field in the original engine; usually not useful to external tools.

    // Only present for spatial scripts.
    int32_t spatial_tile;     // 0x08  Packed tile/elevation value.
    int32_t spatial_radius;   // 0x0C  Spatial trigger radius.

    // Only present for timed scripts.
    int32_t time;             // 0x08  Game-time value for the timed script.

    // After the optional spatial/timed fields,
    // every script record stores the same 14 integers:
    int32_t scr_flags;        // 0x10  (0 in maps, value in saves)
    int32_t scr_index;        // 0x14  Script id. Script filename is found in LST file script.lst at index id
    int32_t program_ptr;      // 0x18  not used?
    uint32_t scr_obj_id;      // 0x1C  Object id for this script
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
struct scripts_list
{
    int32_t count   = 0;
    script* scripts = nullptr;
};

struct scenery
{
    // scenery instance data
    int32_t flags;            // 0x54  instance flags
    int32_t door_flags;       // 0x58  open/shut flags, stairs/elevator/ladders have destination_built_tile
    // int32_t destination;      // 0x5C  destination map/level
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
    uint32_t obj_flags;       // 0x24  0x01000000 right hand, 0x02000000 left hand, 0x03000000 armor worn
    int32_t elevation;        // 0x28
    int32_t obj_pid;          // 0x2C
    int32_t obj_cid;          // 0x30  combat id (save files?) -1 for normal objects 
    int32_t light_radius;     // 0x34  in hexes
    int32_t light_intensity;  // 0x38  (0..65536, interpreted as 0-100%)
    int32_t outline_color;    // 0x3C  Outline color/state in saved maps during combat
    int32_t obj_sid;          // 0x40  runtime script id for this object
    int32_t obj_scr_index;    // 0x44  Script index from scripts.lst, or -1.

    int32_t inventory_cnt;    // 0x48  Number of inventory entries owned by this object
    int32_t inventory_size;   // 0x4C  Allocated inventory capacity in the saved object data
    object* inv_ptr;          // 0x50  Serialized pointer placeholder from the original engine (savegames?)
    // inventory inv;            // 0x50  Serialized pointer placeholder from the original engine
};

struct objects_list
{
    int32_t count_total;
    int32_t count_elev[3];
    object* objects;
};

struct tiles
{
    uint32_t elev[3][10000];
};


map_header parse_header(uint8_t* map_file)
{
    map_header h;

    h.version        = B_Endian::read_u32(&map_file[0]);         // 0x0000  int32   version	Map version, usually 19 or 20.
    memcpy(            h.filename,        &map_file[4], 16);     // 0x0004  char[16]	name	Map file name stored in the header.
    h.dude_start     = B_Endian::read_i32(&map_file[20]);        // 0x0014  int32   entering_tile	Default starting hex tile.
    h.elev_start     = B_Endian::read_i32(&map_file[24]);        // 0x0018  int32   entering_elevation	Default starting elevation, 0 through 2.
    h.face_start     = B_Endian::read_i32(&map_file[28]);        // 0x001C  int32   entering_rotation	Default starting rotation, 0 through 5.
    h.lvar_cnt       = B_Endian::read_i32(&map_file[32]);        // 0x0020  int32   local_vars_count	Number of local map variables following the global variable array.
    h.map_script_id  = B_Endian::read_i32(&map_file[36]);        // 0x0024  int32   script_index	Map script list index. Fallout 2 CE creates a map script only when this value is greater than zero, then subtracts one for the zero-based scripts.lst index.
    h.map_flags      = B_Endian::read_i32(&map_file[40]);        // 0x0028  int32   flags	Save/elevation flags.
    h.light_level    = B_Endian::read_i32(&map_file[44]);        // 0x002C  int32   darkness	Mapper darkness field. Fallout 2 CE writes 1.
    h.mvar_cnt       = B_Endian::read_i32(&map_file[48]);        // 0x0030  int32   global_vars_count	Number of global map variables immediately after the header.
    h.map_id         = B_Endian::read_i32(&map_file[52]);        // 0x0034  int32   map_index	Worldmap/maps.txt map index.
    h.game_ticks     = B_Endian::read_u32(&map_file[56]);        // 0x0038  uint32  last_visit_time	Game time tick when the map was last visited.
    memcpy(            h.unknown,         &map_file[60], 4*44);  // 0x003C  int32[44]   unknown

    return h;
}

// parse map.map file header
//QTODO: rename to something better
void parse_map_map(map_lvls* map)
{
    map_header h = parse_header(map->data);
    map->header_size = sizeof(h);

    // when a level IS marked that means there's NO level information (ffs why do it that way?)
    if (!(h.map_flags & MAP_ELEV_0)) {
        //QTODO: replace "Level 0" etc. with proper markers (possibly ptrs to map level data?)
        //QTODO: yeah, this definitely needs a better marker
        map->level[0] = "Level 0";
    }
    if (!(h.map_flags & MAP_ELEV_1)) {
        map->level[1] = "Level 1";
    }
    if (!(h.map_flags & MAP_ELEV_2)) {
        map->level[2] = "Level 2";
    }

    map->header = h;
}

vars parse_map_vars(map_lvls* map, int* offset)
{
    uint8_t* data = map->data;
    int mvars_cnt = map->header.mvar_cnt;
    int mvars_siz = mvars_cnt * sizeof(int32_t);
    int lvars_cnt = map->header.lvar_cnt;
    int lvars_siz = lvars_cnt * sizeof(int32_t);

    vars v;
    if (mvars_cnt > 0) {
        v.mvars_siz = mvars_siz;
        v.mvars = (int32_t*)malloc(mvars_siz);
    }
    if (lvars_cnt > 0) {
        v.lvars_siz = lvars_siz;
        v.lvars = (int32_t*)malloc(lvars_siz);
    }

    memcpy(v.mvars, &map->data[*offset], mvars_siz);
    *offset += mvars_siz;
    memcpy(v.lvars, &map->data[*offset], lvars_siz);
    *offset += lvars_siz;

    return v;
}

tiles parse_map_tiles(map_lvls* map, int* offset)
{
    int copy_size = 10000*sizeof(uint32_t);
    tiles t;

    for (int i = 0; i < 3; i++)
    {
        if (map->level[i]) {
            memcpy(t.elev[i], &map->data[*offset], copy_size);
            *offset += copy_size;
        } else {
            //QTODO: this needs a better check than -1 since the array is now uint32_t instead of int32_t
            t.elev[i][0] = -1;
        }
    }

    return t;
}

int32_t read_adv(map_lvls* map, int* offset)
{
    if ((*offset + sizeof(int32_t)) > map->file_siz) {
        return 0;
    }

    uint8_t* data = map->data;
    int32_t out = B_Endian::read_i32(&data[*offset]);
    *offset += sizeof(int32_t);

    return out;
}

scripts_list* parse_map_scripts(map_lvls* map, int* offset)
{
    // scripts_list scripts[5];
    scripts_list* scripts = (scripts_list*)calloc(1, 5*sizeof(scripts_list));

    for (int type = SCRIPT_SYSTEM; type <= SCRIPT_CRITTER; type++) {
        scripts[type].count = read_adv(map, offset);

        int this_type[6] = {0};
        int check = 0;
        if (scripts[type].count > 0) {
            int slot_cnt   = ceil(scripts[type].count / 16.0f) * 16;
            int scr_cnt    = scripts[type].count;
            int this_check = 0;

            scripts[type].scripts = (script*)calloc(1, slot_cnt * sizeof(script));
            for (int j = 0; j < slot_cnt; j++) {
                scripts[type].scripts[j].scr_id         = read_adv(map, offset); // 0x00  Packed script id.
                scripts[type].scripts[j].scr_next       = read_adv(map, offset); // 0x04  Linked-list field in the original engine; usually no

                // the game engine parses out the type using this method only (no masking)
                // refer to Alex Batalov's reverse engineered code
                // https://github.com/alexbatalov/fallout2-re/blob/b135fc46ef40c4aecd156f3cebcf88ec531bb8ac/src/game/scripts.c#L1867
                // https://github.com/alexbatalov/fallout2-re/blob/b135fc46ef40c4aecd156f3cebcf88ec531bb8ac/src/game/object_types.h#L32
                // also see Jan Simek's mapper code
                // https://github.com/JanSimek/gecko/issues/78
                int scr_type = scripts[type].scripts[j].scr_id >> 24;
                switch (scr_type)
                {
                case SCRIPT_SYSTEM:
                    this_type[SCRIPT_SYSTEM]++;
                    //QTODO: not a clue what goes here, if anything
                    break;
                case SCRIPT_SPATIAL:
                    this_type[SCRIPT_SPATIAL]++;
                    // Only present for spatial scripts.
                    scripts[type].scripts[j].spatial_tile   = read_adv(map, offset); // 0x08  Packed tile/elevation value
                    scripts[type].scripts[j].spatial_radius = read_adv(map, offset); // 0x0C  Spatial trigger radius.
                    break;
                case SCRIPT_TIMED:
                    this_type[SCRIPT_TIMED]++;
                    // Only present for timed scripts. (Savegame only?)
                    scripts[type].scripts[j].time           = read_adv(map, offset); // 0x08  Game-time value for the timed script
                    break;
                case SCRIPT_OBJECTS:
                    this_type[SCRIPT_OBJECTS]++;
                    // do nothing, already the right size?
                    break;
                case SCRIPT_CRITTER:
                    this_type[SCRIPT_CRITTER]++;
                    // also do nothing?
                    break;
                default:
                    //It looks like the original mapper engine might have placed
                    // proper script ids in some extent filler slots, and garbage in others.
                    //There is probably an underlying rule about how to parse filler script
                    // slots that isn't documented anywhere, but for now it looks like
                    // we can assume an unrecognized script type is always 16 int32's long,
                    // but all others are the defined length of the script type (16/17/18 int32).
                    //Not a clue why scripts are exported this way, or why the extent size seems to be
                    // arbitrary, or why 16 filler slot counts have to be filled out with garbage either.
                    this_type[SCRIPT_GARBAGE]++;
                    printf("DEBUG: map_map_parser.cpp: Unrecognized script type: %d\n", scr_type);
                    break;
                }

                // After the optional spatial/timed fields,
                // every script record stores the same 14 integers:
                scripts[type].scripts[j].scr_flags      = read_adv(map, offset); // 0x10  (0 in maps, value in saves)
                scripts[type].scripts[j].scr_index      = read_adv(map, offset); // 0x14  Script id. Script filename is found in LST file scri
                scripts[type].scripts[j].program_ptr    = read_adv(map, offset); // 0x18  not used?
                scripts[type].scripts[j].scr_obj_id     = (uint32_t)read_adv(map, offset); // 0x1C  Object id for this script
                scripts[type].scripts[j].lvar_offset    = read_adv(map, offset); // 0x20  Offset into lvars data struct from earlier
                scripts[type].scripts[j].lvar_cnt       = read_adv(map, offset); // 0x24  (0 in maps, value in saves?)
                scripts[type].scripts[j].last_used_val  = read_adv(map, offset); // 0x28  possibly used for savegames?
                scripts[type].scripts[j].current_action = read_adv(map, offset); // 0x2C  possibly used for savegames?
                scripts[type].scripts[j].fixed_param    = read_adv(map, offset); // 0x30  possibly used for savegames?
                scripts[type].scripts[j].action_id      = read_adv(map, offset); // 0x34  possibly used for savegames?
                scripts[type].scripts[j].override_flags = read_adv(map, offset); // 0x38  possibly used for savegames?
                scripts[type].scripts[j].unknown_1      = read_adv(map, offset); // 0x3C  unknown
                scripts[type].scripts[j].how_much       = read_adv(map, offset); // 0x40  also unknown
                scripts[type].scripts[j].unknown_2      = read_adv(map, offset); // 0x44  also unknown

                if ((j % 16) == 15) {
                    this_check = read_adv(map, offset);
                    if ((this_check != 16) && (this_check != (scr_cnt % 16))) {
                        printf("ERROR: map_map_parser.cpp: Current check value: (%d) not matching expected value (%d).\n");
                        free(scripts);
                        return nullptr;
                    }
                    check += this_check;
                    printf("type: %d :: total: %d :: current: %d\n", type, scr_cnt, j);
                    int buffer_int = read_adv(map, offset);
                    if (buffer_int != 0) {
                        printf("DEBUG: Weird number found after extent script count check: %d\n", buffer_int);
                    }
                }
            }

            if (scripts[type].count != check) {
                //QTODO: this needs to be a crash or an error or something
                printf("ERROR: map_map_parser.cpp: Script extent check failed. Expected: %d, got: %d\n", scr_cnt, check);
                printf("ERROR: map_map_parser.cpp: Last extent slot count: %d, Expected: %d\n", this_check, (scr_cnt % 16));
                free(scripts);
                return nullptr;
            } else {
                printf("Script extent check Passed. Expected: %d, got: %d\n", scr_cnt, check);
                printf("Last extent slot count: %d, Expected: %d\n", this_check, (scr_cnt % 16));
            }
        } else {
            printf("type: %d :: total: %d\n", type, scripts[type].count);
        }
        for (int i = 0; i < 6; i++) {
            printf("DEBUG: TYPE %d :: SID_type %d has %d scripts\n", type, i, this_type[i]);
        }
    }

    return scripts;
}

objects_list parse_map_objects(map_lvls* map, int* offset)
{
    uint8_t* data_ptr = &map->data[*offset];
    objects_list ol;

    ol.count_total = read_adv(map, offset);
    ol.objects = (object*)calloc(1, sizeof(object)*ol.count_total);
    object* obj = ol.objects;

    for (int elev = 0; elev < 3; elev++) {
        ol.count_elev[elev] = read_adv(map, offset);

        for (int i = 0; i < ol.count_elev[elev]; i++) {
            obj[i].obj_id          = read_adv(map, offset);     // 0x00   id	Object id.
            obj[i].obj_tile        = read_adv(map, offset);     // 0x04   tile	Object hex tile, or -1 for objects not placed on the map.
            obj[i].x               = read_adv(map, offset);     // 0x08   x	Pixel x offset.
            obj[i].y               = read_adv(map, offset);     // 0x0C   y	Pixel y offset.
            obj[i].sx              = read_adv(map, offset);     // 0x10   sx	Cached screen x.
            obj[i].sy              = read_adv(map, offset);     // 0x14   sy	Cached screen y.
            obj[i].frame           = read_adv(map, offset);     // 0x18   frame	Current FRM frame.
            obj[i].rotation        = read_adv(map, offset);     // 0x1C   rotation	Rotation, 0 through 5.
            obj[i].obj_fid         = read_adv(map, offset);     // 0x20   fid	Current art FID.
            obj[i].obj_flags       = read_adv(map, offset);     // 0x24   flags	Instance object flags.
            obj[i].elevation       = read_adv(map, offset);     // 0x28   elevation	Object elevation. During load the engine also forces this to the current elevation loop.
            obj[i].obj_pid         = read_adv(map, offset);     // 0x2C   pid	Prototype PID. Use this with PRO files to enrich the object.
            obj[i].obj_cid         = read_adv(map, offset);     // 0x30   cid	Combat id, mostly relevant to saved/in-combat state.
            obj[i].light_radius    = read_adv(map, offset);     // 0x34   light_distance	Instance light radius.
            obj[i].light_intensity = read_adv(map, offset);     // 0x38   light_intensity	Instance light intensity.
            obj[i].outline_color   = read_adv(map, offset);     // 0x3C   outline	Outline color/state in saved maps. Clean map reads ignore this value.
            obj[i].obj_sid         = read_adv(map, offset);     // 0x40   sid	Runtime script id attached to the object.
            obj[i].obj_scr_index   = read_adv(map, offset);     // 0x44   script_index	Script index from scripts.lst, or -1.

            obj[i].inventory_cnt   = read_adv(map, offset);     // 0x48
            obj[i].inventory_size  = read_adv(map, offset);     // 0x4C

            if (obj[i].inventory_cnt > 0) {
                // obj[i].inv.inv_ptr = (object*)malloc(obj[i].inventory_size);
            }
            //QTODO: parse inventory

            // switch ((obj[i].obj_pid >> 24) & 0x7)
            uint8_t pid = obj[i].obj_pid >> 24;
            switch (pid)
            {
            case OBJ_ITEM:
                // parse item
                break;
            case OBJ_CRITTER:
                // parse critter
                break;
            case OBJ_SCENERY:
                scenery scen;
                scen = {0};
                scen.flags = read_adv(map, offset);
                scen.door_flags  = read_adv(map, offset);
                // scen.destination = read_adv(map, offset);
                // parse scenery
                break;
            case OBJ_WALL:
                // parse wall
                break;
            case OBJ_TILE:
                // parse tile
                break;
            case OBJ_MISC:
                // parse misc
                break;
            case OBJ_INTRFACE:
                // parse interface
                break;
            case OBJ_INVEN:
                // parse inventory
                break;
            case OBJ_HEAD:
                // parse head?
                break;
            case OBJ_BG:
                // parse background
                break;
            
            default:
                printf("ERROR: Object type not recognized.");
                break;
            }
        }
        

    }


    return ol;
}

// BakerStaunch
// --Yeah, so your object struct could just be a pointer to 
// where in that data the object is (or possibly better an offset)
//  & a length and then whatever fields you might want to manipulate
// --Then when you read an object, you pull out the pid type and 
// flags or whatever you need to determine the length, 
// and create the object struct from those
// --One thing that might make deserialization a little simpler 
// is having a struct which is just the file data pointer, 
// a size and a cursor of where you're up to in the file 
// (and possibly flags if you want to do some extra error handling), 
// then your B_Endian::read* functions can take that and adjust the 
// cursor position (and verify they won't try to read more data than
//  what is in the file)


bool export_map_map(char** label_ptr_M, map_lvls* map_L, map_lvls* map_R, int header, char* path_buff)
{
    map_header* head = (header == 0) ? &map_L->header     : &map_R->header;
    int H_size       = (header == 0) ? map_L->header_size : map_R->header_size;
    int offset       = sizeof(map_header);

    vars v_L = parse_map_vars(map_L, &offset);
    // vars v_R = parse_map_vars(map_R, &offset);

    tiles t_L = parse_map_tiles(map_L, &offset);
    // tiles t_R = parse_map_tiles(map_R, &offset);

    scripts_list* s_L = parse_map_scripts(map_L, &offset);
    if (s_L == nullptr) {
        printf("ERROR: map_L - Unable to parse scripts.\n");
        return false;
    }
    // scripts_list s_R = parse_map_scripts(map_R, &offset);

    objects_list o_L = parse_map_objects(map_L, &offset);
    // objects_list o_R = parse_map_objects(map_R, &offset);
}
