#include <string.h>
#include "map_txt_parser.h"
#include "io_Platform.h"


char* find_str(uint8_t* map_txt, char* str)
{
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