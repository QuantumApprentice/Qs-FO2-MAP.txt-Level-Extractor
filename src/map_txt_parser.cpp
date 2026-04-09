#include <string.h>
#include <stdlib.h>
#include "map_txt_parser.h"
#include "io_Platform.h"

//remove after finished
#include <stdio.h>

char* find_str(uint8_t* map_txt, char* str)
{
    //TODO: check if map.data (what's being passed as map_txt)
    //      has a "\0" terminator so string parsing works
    //      else use map.file_siz
    char* str_start = NULL;
    char* map_str   = (char*)map_txt;
    int map_len     = strlen(map_str);
    int str_len     = strlen(str);
    for (size_t i = 0; i < map_len; i++)
    {
        if ((map_str[i] == str[0])
        && (io_strncasecmp(&map_str[i], str, str_len) == 0)) {
            str_start = &map_str[i];
        }
    }
    return str_start;
}

// void parse_map_txt(uint8_t* mapL, uint8_t* mapR, const char** mapM, int header)
//QTODO: make map_lvls a return instead of passing in?
void parse_map_txt(uint8_t* map_data, map_lvls* map)
{
    if (!map_data) {
        return;
    }
    if (!map) {
        return;
    }

    map->data  = map_data;
    map->level[0] = find_str(map_data,(char*)"square_elev: 0");
    map->level[1] = find_str(map_data,(char*)"square_elev: 1");
    map->level[2] = find_str(map_data,(char*)"square_elev: 2");

    map->scripts = find_str(map_data,(char*)">>>>>>>>>>: SCRIPTS <<<<<<<<<<");
    map->objects = find_str(map_data,(char*)">>>>>>>>>>: OBJECTS <<<<<<<<<<");
}

//assigns level string sizes
//some levels might not exist
//so we have to check each one in turn
void map_level_sizes(map_lvls* map)
{
    if (map->level[0]) {
        if (map->level[1]) {
            map->lvl_sizes[0] = map->level[1] - map->level[0];
        } else
        if (map->level[2]) {
            map->lvl_sizes[0] = map->level[2] - map->level[0];
        } else {
            map->lvl_sizes[0] = map->scripts  - map->level[0];
        }
    }
    if (map->level[1]) {
        if (map->level[2]) {
            map->lvl_sizes[1] = map->level[2] - map->level[1];
        } else {
            map->lvl_sizes[1] = map->scripts  - map->level[1];
        }
    }
    if (map->level[2]) {
        map->lvl_sizes[2] = map->scripts - map->level[2];
    }

    if (map->level[0]) {
        map->header_size = (uint64_t)(map->level[0] - (char*)map->data);
    } else
    if (map->level[1]) {
        map->header_size = (uint64_t)(map->level[1] - (char*)map->data);
    } else
    if (map->level[2]) {
        map->header_size = (uint64_t)(map->level[2] - (char*)map->data);
    } else {
        map->header_size = map->scripts - (char*)map->data;
    }
}

// returns a string of all objects located on the level passed in
char* parse_objects(map_lvls* map, int level)
{
    char* begin = NULL;
    char* end   = NULL;
    int objects_size = map->file_siz - (map->objects - (char*)map->data);
    char* objects_str = (char*)malloc(objects_size);
    char* objects_ptr = objects_str;
    for (size_t i = 0; i < objects_size; i++)
    {
        if ((map->objects[i] != '[') && (map->objects[i] != 'o')) {
            continue;
        }
        if (io_strncmp(&map->objects[i], "[OBJECT BEGIN]", sizeof("[OBJECT BEGIN]")-1) == 0) {
            begin = &map->objects[i];
            continue;
        }
        if (io_strncmp(&map->objects[i], "obj_elev: ", sizeof("obj_elev: ")-1) == 0) {
            char l = level + '0';
            char o = map->objects[i + sizeof("obj_elev: ")-1];
            if (map->objects[i + sizeof("obj_elev: ")-1] != (level + '0')) {
                continue;
            }
        }
        if (io_strncmp(&map->objects[i], "[OBJECT END]", sizeof("[OBJECT END]")-1) == 0) {
            end = &map->objects[i + sizeof("[OBJECT END]") + 3];
            int size = end - begin;
            memcpy(objects_ptr, begin, size);
            objects_ptr += size;
        }
    }

    objects_ptr[0] = '\0'; // null terminate the string

    return objects_str;
}

void parse_scripts(char** label_ptr_M, map_lvls* map_L, map_lvls* map_R)
{
    // >>>>>>>>>>: SCRIPTS <<<<<<<<<<


    // SCRS:
    // scr_num: 0      //  ==> unknown
    // scr_num: 0      //  ==> spatial scripts
    // scr_num: 0      //  ==> unknown
    // scr_num: 0      //  ==> all non-critter objects
    // scr_num: 0      //  ==> all critter objects
    //
    // typedef enum ScriptType {                // ==> from fallout2-re scripts.h
    //     SCRIPT_TYPE_SYSTEM,  // s_system     // ==> 0 ==> seems like the most likely match
    //     SCRIPT_TYPE_SPATIAL, // s_spatial    // ==> 1
    //     SCRIPT_TYPE_TIMED,   // s_time       // ==> 2 ==> not sure how to get system or timed scripts tho
    //     SCRIPT_TYPE_ITEM,    // s_item       // ==> 3     its unlikely that system/timed scripts are sorted by level tho
    //     SCRIPT_TYPE_CRITTER, // s_critter    // ==> 4     so it shouldn't be necessary to track them? I hope?
    //     SCRIPT_TYPE_COUNT,                   // ==> 5
    // } ScriptType;


    // [[SCRIPT]]           //  ==> immediately follows the scr_num: count for each script type
    //                      //  ==> occurs every 16 script entries in each scr_num type

    // scr_id: 50331652     // ==> matches obj_sid on the object its attached to 
    //                      // ==> is not the same as an id for the script,
    //                      //     but more like an id for this particular instance of the script
    //                      //     for that reason each object should have a unique obj_sid matching scr_id
    //                      //     even if the same script is reused for multiple objects
    //                      // ==> seems to be generated whenever a script is attached
    // scr_id: 67108864     // ==> 0x40000000  ==> indicates a critter object is attached?
    // scr_id: 50331648     // ==> 0x30000000  ==> all other object types?
    // scr_id: 16777216     // ==> 0x10000000  ==> spatial script types?
    //                      // ==> 0x0 and 0x2 appear to be unused?


    // ...
    /* spatial scripts have these extra fields */
    // scr_udata.sp.built_tile: 39999       // ==> highest tile number for level 0, starts at 0
    // scr_udata.sp.radius: 0
    /*                                         */

    // scr_udata.sp.built_tile: 536870912   // ==> 0x20000000  ==> start number for level 1
    // scr_udata.sp.built_tile: 1073741824  // ==> 0x40000000  ==> start number for level 2


    // [OBJECT BEGIN]
    // ...
    // obj_sid: 4294967295        //  ==> 0xffffffff  ==> no assigned script ==> otherwise matches scr_id on script
    // ...
    // [OBJECT END]

    
}

void export_map_txt(char** label_ptr_M, map_lvls* map_L, map_lvls* map_R, int header)
{
    if (header == -1) {
        return;
    }

    uint8_t* head   = (header == 0) ? map_L->data : map_R->data;
    int   H_size    = (header == 0) ? map_L->header_size : map_R->header_size;
    char* out_lvl[3]      = {0};
    int   out_lvl_size[3] = {0};

    for (size_t i = 0; i < 3; i++) {
        if (io_strncmp(label_ptr_M[i], "empty", NAME_LENGTH) == 0) {
            continue;
        }
        for (size_t j = 0; j < 3; j++) {
            if (io_strncmp(label_ptr_M[i], map_L->label_ptr[j], NAME_LENGTH) == 0) {
                out_lvl[i] = map_L->level[j];
                out_lvl_size[i] = map_L->lvl_sizes[j];
            } else
            if (io_strncmp(label_ptr_M[i], map_R->label_ptr[j], NAME_LENGTH) == 0) {
                out_lvl[i] = map_R->level[j];
                out_lvl_size[i] = map_R->lvl_sizes[j];
            }
        }
    }

    int out_size = map_L->file_siz + map_R->file_siz;
    uint8_t* out_map = (uint8_t*)malloc(out_size);
    char* out_ptr = (char*)out_map;
    //header
    snprintf(out_ptr, H_size, "%s", (char*)head);
    out_ptr += H_size-1; //"-1" to over-write the '\0' character
    //map tiles
    for (size_t i = 0; i < 3; i++) {
        if (out_lvl[i]) {
            snprintf(out_ptr, out_lvl_size[i], "%s", out_lvl[i]);
            out_ptr += out_lvl_size[i];
        }
    }

    printf("%s\n",(char*)out_map);

    //objects
    // copy object strings from source maps into char* allocated memory
    // only objects on copied levels are copied over
    char* objects[3] = {0};
    for (size_t i = 0; i < 3; i++) {
        if (io_strncmp(label_ptr_M[i], "empty", NAME_LENGTH) == 0) {
            continue;
        }
        for (size_t j = 0; j < 3; j++) {
            if (io_strncmp(label_ptr_M[i], map_L->label_ptr[j], NAME_LENGTH) == 0) {
                objects[i] = parse_objects(map_L, i);
            } else
            if (io_strncmp(label_ptr_M[i], map_R->label_ptr[j], NAME_LENGTH) == 0) {
                objects[i] = parse_objects(map_R, i);
            }
        }
    }

    // assign new levels (matching new map) to copied objects
    for (size_t elevation = 0; elevation < 3; elevation++)
    {
        if (!objects[elevation]) {
            continue;
        }

        char* obj_ptr = objects[elevation];
        int len = strlen(obj_ptr);
        int obj_elev_len = sizeof("obj_elev: ")-1;  //-1 to account for the auto-appended '\0' character
        for (size_t j = 0; j < len; j++)
        {
            if (obj_ptr[j] != 'o') {
                continue;
            }
            if (io_strncmp(&obj_ptr[j], "obj_elev: ", obj_elev_len) == 0) {
                j += obj_elev_len;
                obj_ptr[j] = elevation + '0';
            }
        }

        printf("%s\n", objects[elevation]);
    }


    // parse_scripts();

}
