#include <imgui.h>
#include <imgui_internal.h>
#include <GLFW/glfw3.h>
#include "map_txt_gui.h"




bool drag_drop();

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
    drag_drop();
    















    ImGui::SetCursorPos(ImVec2{posB.x+size.x   +  5, posB.y});
    if (ImGui::Button(">##L->M", ImVec2{30,ImGui::GetItemRectSize().y})) {
        //move from left to middle
    }

    ImGui::SetCursorPos(ImVec2{posB.x+size.x   + 40, posB.y});
    ImGui::ListBox("##M", &selection[1], map_M, IM_COUNTOF(map_M));

    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 45, posB.y});
    if (ImGui::Button("<##R->M", ImVec2{30,ImGui::GetItemRectSize().y})) {
        //move from right to middle
    }

    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 80, posB.y});
    ImGui::ListBox("##R", &selection[2], map_R, IM_COUNTOF(map_R));

    ImGui::PopItemWidth();




    return false;
}


bool drag_drop()
{

    ImVec2 list_min  = ImGui::GetItemRectMin();
    ImVec2 list_max  = ImGui::GetItemRectMax();
    ImRect rect = ImRect{list_min.x,list_min.y,list_max.x,list_max.y};
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiWindow* window = ImGui::GetCurrentWindow();






    bool drag_frame = false;// = io_file_drag();


    // if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
    //     if (ImGui::GetDragDropPayload()) {
    //         ImGui::Text("11111");
    //     }
    //     if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern | ImGuiDragDropFlags_AcceptBeforeDelivery))	// we use an external source (i.e. not ImGui-created)
    //     {
    //         // replace "FILES" with whatever identifier you want - possibly dependant upon what type of files are being dragged
    //         // you can specify a payload here with parameter 2 and the sizeof(parameter) for parameter 3.
    //         // I store the payload within a vector of strings within the application itself so don't need it.
    //         // ImGui::SetDragDropPayload("FILES", nullptr, 0);
    //         // // const ImGuiPayload* pl = ImGui::GetDragDropPayload();
    //         // const ImGuiPayload* pl = ImGui::AcceptDragDropPayload(
    //         //     IMGUI_PAYLOAD_TYPE_EXTERNAL_FILE,
    //         //     ImGuiDragDropFlags_AcceptPeekOnly
    //         // );
    //         // if (pl) {
    //         //     flip = true;
    //         // }
    //         // ImGui::BeginTooltip();
    //         // ImGui::EndTooltip();
    //         ImGui::EndDragDropSource();
    //     }
    //     // if (ImGui::BeginDragDropTargetCustom(rect,viewport->ID)) {
    //     //     if (ImGui::GetDragDropPayload()) {
    //     //         printf("test");
    //     //     }
    //     //     if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
    //     //     {
    //     //         printf("custom");
    //     //     }
    //     // }
    //     // if (ImGui::BeginDragDropTargetViewport(viewport, &rect))
    //     // {
    //     //     if (ImGui::GetDragDropPayload()) {
    //     //         ImGui::Text("xxxxx");
    //     //     }
    //     //     if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILES"))
    //     //     {
    //     //         // ImGui::Text("ooooo");
    //     //         // flip = true;
    //     //     }
    //     //     ImGui::EndDragDropTarget();
    //     // }
    // }


    if (drag_frame) {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern))
        {
            ImGui::SetDragDropPayload("FILES", nullptr, 0);
        }
        ImGui::EndDragDropSource();
    }

    // if (ImGui::BeginDragDropTargetCustom(rect,viewport->ID))
    if (ImGui::BeginDragDropTarget())
    {

        if (ImGui::AcceptDragDropPayload("FILES"))
        {
            // flip = true;
        }
        ImGui::EndDragDropTarget();
    }










    return false;
}