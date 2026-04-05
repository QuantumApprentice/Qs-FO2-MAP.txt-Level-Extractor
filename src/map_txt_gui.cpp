#include <imgui_internal.h>
#include "map_txt_gui.h"
#include "io_Platform.h"


bool is_hovering   = false;
int  map_box       = -1;
char*   name_left  = NULL;
uint8_t* map_left  = NULL;
char*   name_right = NULL;
uint8_t* map_right = NULL;


#define NAME_LENGTH         (16)
static char hl[NAME_LENGTH] = {"empty##1"};
static char hm[NAME_LENGTH] = {"empty##2"};
static char hr[NAME_LENGTH] = {"empty##3"};
void file_drop_callback(const char* full_path)
{
    if (map_box == -1) {
        return;
    }

    uint8_t* file_box = NULL;
    char* file_name   = NULL;
    int len = strlen(full_path) + 1;
    file_name = (char*)malloc(len);
    memcpy(file_name, full_path, len);

    char* start = strrchr(file_name,'/')+1;

    if (map_box == 0) {
        file_box  = map_left;
        if (name_left) {
            free(name_left);
            name_left = NULL;
        }
        name_left = file_name;
        strncpy(hl,start,strlen(start) < NAME_LENGTH ? strlen(start) : NAME_LENGTH);
    } else
    if (map_box == 1) {
        file_box   = map_right;
        if (name_right) {
            free(name_right);
            name_right = NULL;
        }
        name_right = file_name;
        strncpy(hr,start,strlen(start) < NAME_LENGTH ? strlen(start) : NAME_LENGTH);
    }

    if (file_box) {
        free(file_box);
        file_box = NULL;
    }

    file_box = io_load_file(file_name);

    map_box = -1;
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
    // map_box = -1;
}

// kind of dumb, but...
// if mouse enters the boundaries of the previous item
// (in this case it's always one of the two map lists)
// then draw highlight border around the list
// and return true
// the return value is used to determine which list item
// to use when storing the map.txt information
bool drag_imgui()
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

bool map_txt_gui()
{
    ImVec2 size = ImGui::CalcTextSize("AAAAAAAAA");
    ImGui::PushItemWidth(size.x);
    static const char* map_L[] = { "Left 1", "Left 2", "Left 3" };
    static const char* map_M[] = { "empty",   "empty",   "empty"   };
    static const char* map_R[] = { "Right 1", "Right 2", "Right 3" };

    ImGui::Text("Map Names:");


    ImVec2 posA = ImGui::GetCursorPos();

    char* start = NULL;
    if (ImGui::Button(hl, ImVec2{size.x,0})) {
        if (name_left) {
            start = strrchr(name_left,'/')+1;
            snprintf(hm, strlen(start) < NAME_LENGTH ? strlen(start) : NAME_LENGTH, "%s##", start);
            // strncpy(hm,start,strlen(start) < NAME_LENGTH ? strlen(start) : NAME_LENGTH);
        } else {
            strncpy(hm,"Header1##",sizeof("Header1##"));
        }
    }
    ImGui::SetCursorPos(ImVec2{posA.x+size.x   + 40, posA.y});
    if (ImGui::Button(hm, ImVec2{size.x,0})) {
        strncpy(hm,"empty",sizeof("empty"));
    }
    ImGui::SetCursorPos(ImVec2{posA.x+size.x*2 + 80, posA.y});
    if (ImGui::Button(hr, ImVec2{size.x,0})) {
        if (name_right) {
            start = strrchr(name_right,'/')+1;
            snprintf(hm, strlen(start) < NAME_LENGTH ? strlen(start) : NAME_LENGTH, "%s##", start);
            // strncpy(hm,start,strlen(start) < NAME_LENGTH ? strlen(start) : NAME_LENGTH);
        } else {
            strncpy(hm,"Header2##",sizeof("Header2##"));
        }
    }


    ImVec2 posB = ImGui::GetCursorPos();
    static int selection[3] = { 0, 1, 2 };



    ImGui::ListBox("##L", &selection[0], map_L, IM_COUNTOF(map_L));
    if (drag_imgui()) {
        map_box = 0;
    }

    ImGui::SetCursorPos(ImVec2{posB.x+size.x   +  5, posB.y});
    if (ImGui::Button(">##L->M", ImVec2{30,ImGui::GetItemRectSize().y})) {
        //QTODO: move from left to middle
    }

    ImGui::SetCursorPos(ImVec2{posB.x+size.x   + 40, posB.y});
    ImGui::ListBox("##M", &selection[1], map_M, IM_COUNTOF(map_M));

    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 45, posB.y});
    if (ImGui::Button("<##R->M", ImVec2{30,ImGui::GetItemRectSize().y})) {
        //QTODO: move from right to middle
    }



    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 80, posB.y});
    ImGui::ListBox("##R", &selection[2], map_R, IM_COUNTOF(map_R));
    if (drag_imgui()) {
        map_box = 1;
    }

    ImGui::PopItemWidth();


    return false;
}