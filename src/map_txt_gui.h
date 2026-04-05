#pragma once
#include <imgui.h>

void drag_file(ImVec2 pos);
void drag_dropped();
void file_drop_callback(const char* file);
bool map_txt_gui();
