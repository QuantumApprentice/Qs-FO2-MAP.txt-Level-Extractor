
#ifdef X11
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

bool io_x11_init(GLFWwindow* gl_window)
{
    native_window = glfwGetX11Window(gl_window);

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

void xdnd_reply(Window x_src)
{
    XEvent reply = {};

    reply.xclient.type         = ClientMessage;
    reply.xclient.display      = display;
    reply.xclient.window       = x_src;
    reply.xclient.message_type = atoms.xdndStatus;
    reply.xclient.format       = 32;

    reply.xclient.data.l[0] = native_window;
    reply.xclient.data.l[1] = 1;
    reply.xclient.data.l[2] = 0;
    reply.xclient.data.l[3] = 0;
    reply.xclient.data.l[4] = atoms.xdndCopy;

    XSendEvent(display, x_src, false, NoEventMask, &reply);
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
        if (event.xclient.message_type == atoms.xdndEnter) {
            file_hover = true;
        }
        if (event.xclient.message_type == atoms.xdndLeave) {
            // not getting any Leave signal?
            file_hover = false;
            ImGui::ClearDragDrop();
        }
        if (event.xclient.message_type == atoms.xdndDrop) {
            file_hover = false;
            // ImGui::ClearDragDrop();
        }
        if (event.xclient.message_type == atoms.xdndPos) {
            xdnd_reply(event.xclient.data.l[0]);
            file_hover = true;
        }
    }

    return file_hover;

}
#endif