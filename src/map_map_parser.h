#pragma once
#include "map_structs.h"

void parse_map_map(map_lvls* map);
void parse_map_scripts(map_lvls* map, int* offset, scripts_list scripts[SCRIPT_TYPE_COUNT]);
void export_map_map(char** label_ptr_M, map_lvls* map_L, map_lvls* map_R, int header, char* path_buff);
