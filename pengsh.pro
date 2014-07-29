QT        += core widgets gui gui-private network x11extras multimedia
SOURCES   += main.cpp qxtglobalshortcut.cpp qxtglobalshortcut_x11.cpp qxtglobal.cpp qxtwindowsystem.cpp qxtwindowsystem_x11.cpp settingsdialog.cpp settings.cpp logindialog.cpp x11.cpp x11-util.cpp
HEADERS   += qxtglobalshortcut.h qxtglobalshortcut_p.h qxtglobal.h qxtwindowsystem.h settingsdialog.h settings.h logindialog.h x11.h x11-util.h
RESOURCES += resource.qrc
FORMS     += settingsdialog.ui logindialog.ui
CONFIG    += c++11 link_pkgconfig

packagesExist(x11) {
    PKGCONFIG += x11
    DEFINES += USE_X11
}

target.path = /usr/bin/
INSTALLS += target
