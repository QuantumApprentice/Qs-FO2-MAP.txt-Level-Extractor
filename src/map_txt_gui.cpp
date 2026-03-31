#include "map_txt_gui.h"
#include "imgui.h"

bool map_txt_gui()
{

    ImVec2 size = ImGui::CalcTextSize("AAAAAAAAA");
    ImGui::PushItemWidth(size.x);
    const char* map_L[] = { "Level 1", "Level 2", "Level 3" };
    const char* map_M[] = { "empty",   "empty",   "empty"   };
    const char* map_R[] = { "Level 1", "Level 2", "Level 3" };

    ImGui::Text("Lists:");


    ImVec2 posA = ImGui::GetCursorPos();

    static char hl[10] = {"empty##1"};
    static char hm[10] = {"empty##2"};
    static char hr[10] = {"empty##3"};

    if (ImGui::Button(hl, ImVec2{size.x,0})) {
        strncpy(hm,"Header1##",sizeof("Header1##"));
    }
    ImGui::SetCursorPos(ImVec2{posA.x+size.x   + 40, posA.y});
    if (ImGui::Button(hm, ImVec2{size.x,0})) {
        strncpy(hm,"empty",sizeof("empty"));
    }
    ImGui::SetCursorPos(ImVec2{posA.x+size.x*2 + 80, posA.y});
    if (ImGui::Button(hr, ImVec2{size.x,0})) {
        strncpy(hm,"Header2##",sizeof("Header2##"));
    }
    ImGui::SameLine();
    ImGui::Text("Map Header");


    ImVec2 posB = ImGui::GetCursorPos();

    static int selection[3] = { 0, 1, 2 };
    ImGui::ListBox("##L", &selection[0], map_L, IM_COUNTOF(map_L));

    ImGui::SetCursorPos(ImVec2{posB.x+size.x   +  5, posB.y});
    if (ImGui::Button(">##L->M", ImVec2{30,145})) {

    }

    ImGui::SetCursorPos(ImVec2{posB.x+size.x   + 40, posB.y});
    ImGui::ListBox("##M", &selection[1], map_M, IM_COUNTOF(map_M));

    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 45, posB.y});
    if (ImGui::Button("<##R->M", ImVec2{30,145})) {

    }

    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 80, posB.y});
    ImGui::ListBox("##R", &selection[2], map_R, IM_COUNTOF(map_R));

    ImGui::PopItemWidth();


    return false;
}