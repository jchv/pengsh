#pragma once

extern "C" {
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
}

class XDisplay
{
public:
    Display *d;
    Window root;

    explicit XDisplay(const char *dpyname = nullptr);
    XDisplay(const XDisplay &other) = delete;
    ~XDisplay();

    operator Display* ();
};

class XPointerGrab
{
public:
    XDisplay &d;

    explicit XPointerGrab(XDisplay &d, unsigned int eventMask, Cursor cursor);
    XPointerGrab(const XPointerGrab &other) = delete;
    ~XPointerGrab();
};

class XKeyboardGrab
{
public:
    XDisplay &d;

    explicit XKeyboardGrab(XDisplay &d);
    XKeyboardGrab(const XKeyboardGrab &other) = delete;
    ~XKeyboardGrab();
};

class XGC
{
public:
    XDisplay &d;
    GC c;

    explicit XGC(XDisplay &d, unsigned long valuesMask, XGCValues *values);
    XGC(const XGC &other) = delete;
    ~XGC();

    operator GC ();
};

class XCursor
{
public:
    XDisplay &d;
    Cursor c;

    explicit XCursor(XDisplay &d, unsigned int shape);
    XCursor(const XCursor &other) = delete;
    ~XCursor();

    operator Cursor ();
};

Window findFrame(XDisplay &disp, Window win);
