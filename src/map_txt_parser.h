#pragma once
#include <stdint.h>
#define NAME_LENGTH               (16)

struct map_lvls
{
    char* file_str = NULL;
    char* map_name = NULL;
    uint8_t* data  = NULL;

    char label[3][NAME_LENGTH] = {"Level 1","Level 2","Level 3"};
    char* label_ptr[3] = {label[0],label[1],label[2]};
    char* level[3] = {NULL,NULL,NULL};    // pointers to "square_elev:" entries in data

    char* scripts = NULL;
    char* objects = NULL;
};


void parse_map_txt(uint8_t* map, map_lvls* lvls);