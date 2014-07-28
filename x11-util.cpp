#include "qxtwindowsystem.h"

#include "x11-util.h"
#include "x11.h"

/* Todo:
 *  - XCB?
 *  - Wayland support
 */

QRect rectSelect()
{
    XEvent ev;
    QPoint start, end;
    QRect rect;

    XDisplay disp;

    if (!disp.d)
        return QRect();

    XCursor cursor(disp, XC_crosshair);

    XPointerGrab ptrGrab(disp, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, cursor);
    XKeyboardGrab keyGrab(disp);

    XGCValues gcval;
    gcval.function = GXinvert;
    gcval.subwindow_mode = IncludeInferiors;
    gcval.line_width = 2;

    XGC gc(disp, GCFunction | GCSubwindowMode | GCLineWidth, &gcval);

    while (start.isNull()) {
        XNextEvent(disp, &ev);

        if (ev.type == ButtonPress)
            start = QPoint(ev.xbutton.x_root, ev.xbutton.y_root);
        else if (ev.type == KeyPress)
            return QRect();
    }

    while (true) {
        XNextEvent(disp, &ev);

        if (ev.type == MotionNotify) {
            XDrawRectangle(disp, disp.root, gc, rect.left(), rect.top(), rect.width(), rect.height());

            end = QPoint(ev.xbutton.x_root, ev.xbutton.y_root);
            rect = QRect(start, end).normalized();

            XDrawRectangle(disp, disp.root, gc, rect.left(), rect.top(), rect.width(), rect.height());
            XFlush(disp);
        } else if (ev.type == ButtonRelease)
            break;
    }

    XDrawRectangle(disp, disp.root, gc, rect.left(), rect.top(), rect.width(), rect.height());
    XFlush(disp);

    return rect;
}


QRect activeWinRect()
{
    Window activeWin, rootWin, parentWin, *children = nullptr;
    unsigned int nchildren;
    int revert;

    XDisplay disp;

    if (!disp.d)
        return QRect();

    XGetInputFocus(disp, &activeWin, &revert);

    while(1) {
        Status s = XQueryTree(disp, activeWin, &rootWin, &parentWin, &children, &nchildren);
        if (children)
            XFree((char *) children);
        if (!s || !parentWin || parentWin == rootWin)
            break;
        activeWin = parentWin;
    }

    activeWin = findFrame(disp, activeWin);

    Window childWin;
    int rx, ry, x, y;
    unsigned int width, height, border, depth;

    XGetGeometry(disp, activeWin, &rootWin, &rx, &ry, &width, &height, &border, &depth);
    XTranslateCoordinates(disp, activeWin, rootWin, 0, 0, &x, &y, &childWin);

    return QRect(x, y, width, height);
}
