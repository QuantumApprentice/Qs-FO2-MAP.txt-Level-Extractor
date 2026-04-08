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

void export_map_txt(char** label_ptr_M, map_lvls* map_L, map_lvls* map_R, int header)
{
    if (header == -1) {
        return;
    }

    uint8_t* head   = (header == 0) ? map_L->data : map_R->data;
    int   H_size    = (header == 0) ? map_L->header_size : map_R->header_size;
    char* out_lvl[3]      = {0};
    int   out_lvl_size[3] = {0};

    for (size_t i = 0; i < 2; i++) {
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

    //scripts
    //objects




}
