/* x11.cpp - C++ wrappers for X11 classes
 *
 * This was primarily developed to leverage C++ RAII for some simpler
 * control flow.
 */

#include "x11.h"

XDisplay::XDisplay(const char *dpyname) {
    d = XOpenDisplay(dpyname);
    root = DefaultRootWindow(d);
}

XDisplay::~XDisplay() {
    XSync(d, 1);
    XCloseDisplay(d);
}

XDisplay::operator Display *() {
    return d;
}

XPointerGrab::XPointerGrab(XDisplay &d, unsigned int eventMask, Cursor cursor) : d(d) {
    XGrabPointer(d, d.root, True, eventMask, GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime);
}

XPointerGrab::~XPointerGrab() {
    XUngrabPointer(d, CurrentTime);
}

XKeyboardGrab::XKeyboardGrab(XDisplay &d) : d(d) {
    XGrabKeyboard(d, d.root, False, GrabModeAsync, GrabModeAsync, CurrentTime);
}

XKeyboardGrab::~XKeyboardGrab() {
    XUngrabKeyboard(d, CurrentTime);
}

XGC::XGC(XDisplay &d, unsigned long valuesMask, XGCValues *values) : d(d) {
    c = XCreateGC(d, d.root, valuesMask, values);
}

XGC::~XGC() {
    XFreeGC(d, c);
}

XGC::operator GC() {
    return c;
}

XCursor::XCursor(XDisplay &d, unsigned int shape) : d(d) {
    c = XCreateFontCursor(d, shape);
}

XCursor::~XCursor() {
    XFreeCursor(d, c);
}

XCursor::operator Cursor() {
    return c;
}

Window findFrame(XDisplay &disp, Window win) {
    Atom netFrame;
    Atom type = None;
    int format, status;
    unsigned char *data;
    unsigned long after, items;

    netFrame = XInternAtom(disp, "_NET_FRAME_WINDOW", True);

    if (!netFrame)
        return win;

    status = XGetWindowProperty(disp, win, netFrame, 0L, 1L, False, (Atom) AnyPropertyType, &type, &format, &items, &after, &data);

    if (status != Success || !type || format != 32 || items != 1)
        return win;

    win = *(Window*)data;

    if (data)
        XFree(data);

    return win;
}
