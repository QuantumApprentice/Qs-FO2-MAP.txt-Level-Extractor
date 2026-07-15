#pragma once
#include "map_structs.h"

void parse_map_map(map_lvls* map);
bool export_map_map(char** label_ptr_M, map_lvls* map_L, map_lvls* map_R, int header, char* path_buff, char* game_path);