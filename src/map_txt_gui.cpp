#include <imgui_internal.h>
#include "io_Platform.h"
#include "map_txt_gui.h"
#include "map_txt_parser.h"


bool is_hovering = false;
int list_box     = -1;

#define NAME_LENGTH     (16)
#define LEFT            (0)
#define MIDDLE          (1)
#define RIGHT           (2)

map_lvls map_L;
map_lvls map_R;
char label_M[3][16] = {"empty"};
char head_L[NAME_LENGTH] = {"empty##1"};
char head_M[NAME_LENGTH] = {"empty##2"};
char head_R[NAME_LENGTH] = {"empty##3"};
void update_labels(map_lvls* map, int list_box)
{
    if (list_box == -1) {
        return;
    }

    snprintf((list_box == 0) ? head_L : head_R, NAME_LENGTH, "%s", map->map_name);
    memset(label_M,0,sizeof(label_M));
    strncpy(label_M[0],"empty",sizeof("empty"));

    for (size_t i = 0; i < 3; i++) {
        map->label_ptr[i] = map->label[i];
        if (map->level[i]) {
            snprintf(map->label_ptr[i], NAME_LENGTH, "%d:%s", i, map->map_name);
        } else {
            snprintf(map->label_ptr[i], NAME_LENGTH, "empty");
        }
    }
}

void file_drop_callback(const char* full_path)
{
    if (list_box == -1) {
        return;
    }

    char* file_name   = NULL;
    int len = strlen(full_path) + 1;
    file_name = (char*)malloc(len);
    memcpy(file_name, full_path, len);


    map_lvls* map_ptr = NULL;
    if (list_box == 0) {
        map_ptr   = &map_L;
    } else
    if (list_box == 1) {
        map_ptr   = &map_R;
    }

    if (map_ptr->data) {
        free(map_ptr->data);
        free(map_ptr->file_str);
        memset(map_ptr,0,sizeof(*map_ptr));
    }
    file_info* file   = io_load_file(file_name);
    map_ptr->file_str = file_name;
    map_ptr->file_siz = file->size;
    map_ptr->data     = file->data;
    free(file);

    map_ptr->map_name = strrchr(file_name,'/')+1;

    parse_map_txt(map_ptr->data, map_ptr);
    update_labels(map_ptr, list_box);
    map_level_sizes(map_ptr);

    list_box = -1;
}

void drag_file(ImVec2 pos)
{
    ImGui::TeleportMousePos(pos);
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[0] = true;

    is_hovering = true;
}
void drag_dropped()
{
    is_hovering = false;

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[0] = false;
    list_box = -1;
}

// kind of dumb, but...
// if mouse enters the window but is not over previous item
// then draw highlight border around the previous item
// (in this case it's always one of the two map lists)
// and return false
// if mouse enters the boundary of the previous item
// then return true
// the return value is used to determine which list item
// to use when storing the map.txt information
bool hover_box()
{
    if (is_hovering) {
        ImVec2 list_min  = ImGui::GetItemRectMin();
        ImVec2 list_max  = ImGui::GetItemRectMax();
        ImRect rect = ImRect{list_min.x,list_min.y,list_max.x,list_max.y};

        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRect(list_min, list_max, ImColor(232,232,2,255), 0, 0, 5.0f);

        ImVec2 m_pos = ImGui::GetMousePos();
        if ((m_pos.x > list_min.x)
        &&  (m_pos.y > list_min.y)
        &&  (m_pos.x < list_max.x)
        &&  (m_pos.y < list_max.y)) {
            return true;
        }
    }
    return false;
}


// gui interface for the whole map_txt editor
// divided into thirds, a left map, a right map, 
// and the new map in the middle
bool map_txt_gui()
{
    ImVec2 size = ImGui::CalcTextSize("AAAAAAAAA");
    ImGui::PushItemWidth(size.x);
    static int header = -1;
    #define PATH_SIZE           (MAX_PATH)
    static char path_buff[PATH_SIZE] = "/path/to/some/folder/with/long/filename.cpp";

    ImGui::Text("Map Names:");


    ImVec2 posA = ImGui::GetCursorPos();
    if (ImGui::Button(head_L, ImVec2{size.x,0})) {
        if (map_L.data) {
            snprintf(head_M, NAME_LENGTH, "%s##", map_L.map_name);
            snprintf(path_buff, PATH_SIZE, "%s.Q.txt", map_L.file_str);
            header = 0;
        } else {
            strncpy(head_M,"HeaderL##",sizeof("HeaderL##"));
        }
    }
    ImGui::SetCursorPos(ImVec2{posA.x+size.x   + 40, posA.y});
    if (ImGui::Button(head_M, ImVec2{size.x,0})) {
        header = -1;
        strncpy(head_M,"empty",sizeof("empty"));
    }
    ImGui::SetCursorPos(ImVec2{posA.x+size.x*2 + 80, posA.y});
    if (ImGui::Button(head_R, ImVec2{size.x,0})) {
        if (map_R.data) {
            snprintf(head_M, NAME_LENGTH, "%s##", map_R.map_name);
            snprintf(path_buff, PATH_SIZE, "%s.Q.txt", map_R.file_str);
            header = 1;
        } else {
            strncpy(head_M,"HeaderR##",sizeof("HeaderR##"));
        }
    }


    ImVec2 posB = ImGui::GetCursorPos();
    static int selection[3] = { 0, 1, 2 };


    // left third
    ImGui::ListBox("##L", &selection[0], map_L.label_ptr, IM_COUNTOF(map_L.label_ptr));
    if (hover_box()) {
        list_box = 0;
    }

    ImGui::SetCursorPos(ImVec2{posB.x+size.x   +  5, posB.y});
    if (ImGui::Button(">##L->M", ImVec2{30,ImGui::GetItemRectSize().y})) {
        // replace middle selection with selection on left
        strncpy(label_M[selection[MIDDLE]],map_L.label_ptr[selection[LEFT]],NAME_LENGTH);
    }

    // middle third
    char* label_ptr_M[] = {label_M[0],label_M[1],label_M[2]};
    ImGui::SetCursorPos(ImVec2{posB.x+size.x   + 40, posB.y});
    ImGui::ListBox("##M", &selection[1], label_ptr_M, IM_COUNTOF(label_ptr_M));

    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 45, posB.y});
    if (ImGui::Button("<##R->M", ImVec2{30,ImGui::GetItemRectSize().y})) {
        // replace middle selection with selection on right
        strncpy(label_M[selection[MIDDLE]],map_R.label_ptr[selection[RIGHT]],NAME_LENGTH);
    }

    // right third
    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 80, posB.y});
    ImGui::ListBox("##R", &selection[2], map_R.label_ptr, IM_COUNTOF(map_R.label_ptr));
    if (hover_box()) {
        list_box = 1;
    }

    ImGui::PopItemWidth();


    if (ImGui::BeginPopup("Overwrite?")) {
        ImGui::Text("Don't save over the original files\n"
                    "for now, I'm not sure they would\n"
                    "be recoverable.");
        ImGui::Text("File already exists, overwrite?");
        if (ImGui::Button("Overwrite")) {
            export_map_txt(label_ptr_M, &map_L, &map_R, header, path_buff);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }


    if (ImGui::Button("Export")) {
        if (io_file_exists(path_buff)) {
            ImGui::OpenPopup("Overwrite?");
        } else {
            export_map_txt(label_ptr_M, &map_L, &map_R, header, path_buff);
        }
    }
    if (header == -1) {
        ImGui::SameLine();
        ImGui::Text("Pick a header first");
    }

    ImGui::InputText("Path", path_buff, IM_COUNTOF(path_buff), ImGuiInputTextFlags_ElideLeft);


    return false;
}