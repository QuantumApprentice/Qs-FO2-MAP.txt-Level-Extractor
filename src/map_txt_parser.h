#pragma once
#include <stdint.h>
#define NAME_LENGTH               (16)

struct map_lvls
{
    char* file_str = NULL;
    int   file_siz = 0;     //should not be more than a couple MBs ever
    char* map_name = NULL;
    uint8_t* data  = NULL;

    int header_size = 0;
    int lvl_sizes[3] = {0};

    char label[3][NAME_LENGTH] = {"Level 1","Level 2","Level 3"};
    char* label_ptr[3] = {label[0],label[1],label[2]};
    char* level[3] = {NULL,NULL,NULL};    // pointers to "square_elev:" entries in data

    char* scripts = NULL;
    char* objects = NULL;
};


void parse_map_txt(uint8_t* map, map_lvls* lvls);
void map_level_sizes(map_lvls* map);
void export_map_txt(char** label_ptr_M, map_lvls* map_L, map_lvls* map_R, int header);