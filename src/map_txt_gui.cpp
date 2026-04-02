#include "map_txt_gui.h"
// #include "imgui.h"
#include <imgui.h>


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
    if (ImGui::Button(">##L->M", ImVec2{30,145})) {
        //move from left to middle
    }

    ImGui::SetCursorPos(ImVec2{posB.x+size.x   + 40, posB.y});
    ImGui::ListBox("##M", &selection[1], map_M, IM_COUNTOF(map_M));

    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 45, posB.y});
    if (ImGui::Button("<##R->M", ImVec2{30,145})) {
        //move from right to middle
    }

    ImGui::SetCursorPos(ImVec2{posB.x+size.x*2 + 80, posB.y});
    ImGui::ListBox("##R", &selection[2], map_R, IM_COUNTOF(map_R));

    ImGui::PopItemWidth();




    return false;
}

#include <imgui_internal.h>
#include <imgui_impl_glfw.h>



// #define GLFW_EXPOSE_NATIVE_X11      <GLFW/glfw3native.h>     X11
// #define GLFW_EXPOSE_NATIVE_WAYLAND  <GLFW/glfw3native.h>     Wayland
// #define GLFW_EXPOSE_NATIVE_GLX      <GLFW/glfw3native.h>     GLX
// X11 server first
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <X11/Xlib.h>

Display* display;
Window  native_window;
struct X11_Atoms {
    Atom xdndEnter;
    Atom xdndLeave;
    Atom xdndDrop ;
    Atom xdndPos  ;
    Atom xdndCopy ;
    Atom xdndStatus;
} atoms;

bool io_x11_init(GLFWwindow* window)
{
    native_window = glfwGetX11Window(window);

    int backend = glfwGetPlatform(); // Available in GLFW 3.4+
    if (backend == GLFW_PLATFORM_X11) {
        // Use Xlib/XCB hooks
        display          = glfwGetX11Display();
        atoms.xdndEnter  = XInternAtom(display, "XdndEnter"     , false);
        atoms.xdndLeave  = XInternAtom(display, "XdndLeave"     , false);
        atoms.xdndDrop   = XInternAtom(display, "XdndDrop"      , false);
        atoms.xdndPos    = XInternAtom(display, "XdndPosition"  , false);
        atoms.xdndCopy   = XInternAtom(display, "XdndActionCopy", false);
        atoms.xdndStatus = XInternAtom(display, "XdndStatus"    , false);
    } else if (backend == GLFW_PLATFORM_WAYLAND) {
        // Use Wayland-client hooks
    }

    return true;
}

void xdnd_reply(Window src)
{

    XEvent reply = {};

    reply.xclient.type         = ClientMessage;
    reply.xclient.display      = display;
    reply.xclient.window       = src;
    reply.xclient.message_type = atoms.xdndStatus;
    reply.xclient.format       = 32;

    reply.xclient.data.l[0] = native_window;
    reply.xclient.data.l[1] = 1;
    reply.xclient.data.l[2] = 0;
    reply.xclient.data.l[3] = 0;
    reply.xclient.data.l[4] = atoms.xdndCopy;

    XSendEvent(display, src, false, NoEventMask, &reply);
    XFlush(display);
}


Bool is_file(Display* d, XEvent* e, XPointer arg)
{
    return e->type == ClientMessage || e->type == SelectionNotify;
}
bool io_file_drag()
{
    static bool file_hover;
    XEvent event;
    if (XCheckIfEvent(display, &event, is_file, NULL)) {
        // ImGui::Text("file hovering");
        if (event.xclient.message_type == atoms.xdndEnter) {
            file_hover = true;
        }
        if (event.xclient.message_type == atoms.xdndLeave) {
            file_hover = false;
        }
        if (event.xclient.message_type == atoms.xdndDrop) {
            file_hover = false;
        }
        if (event.xclient.message_type == atoms.xdndPos) {
            xdnd_reply(event.xclient.data.l[0]);
            file_hover = true;
        }
    }

    return file_hover;

}


bool drag_drop()
{

    ImVec2 list_min  = ImGui::GetItemRectMin();
    ImVec2 list_max  = ImGui::GetItemRectMax();
    ImRect rect = ImRect{list_min.x,list_min.y,list_max.x,list_max.y};
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiWindow* window = ImGui::GetCurrentWindow();






    bool flip = io_file_drag();


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


    if (flip) {
        // ImGui::Text("dragging files?");

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern))	// we use an external source (i.e. not ImGui-created)
        {
            ImGui::SetDragDropPayload("FILES", nullptr, 0);
        }
        ImGui::EndDragDropSource();
    } else {
        ImGui::Text("wtfffff????");
    }


    // if (ImGui::BeginDragDropTargetCustom(rect,viewport->ID))
    if (ImGui::BeginDragDropTarget())
    {

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILES"))
        {
            // flip = true;
        }
        ImGui::EndDragDropTarget();
    }










    return false;
}