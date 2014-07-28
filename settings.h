#pragma once

#include <QString>
#include <QFlags>
#include <QKeySequence>

class Settings {
    Q_GADGET
public:
    enum PuushAction {
        NoAction = 0x0,
        PlaySound = 0x1,
        CopyLink = 0x2,
        OpenInBrowser = 0x4,
        SaveToDisk = 0x8
    };
    Q_DECLARE_FLAGS(PuushActions, PuushAction)

    enum DoubleClickAction {
        ShowSettings,
        CaptureScreen,
        UploadFile
    };

    Settings();
    ~Settings();

    bool load();
    void flush();

    PuushActions onPuushActions = PuushAction(PlaySound | CopyLink);
    DoubleClickAction onDoubleClickAction = ShowSettings;
    QString savePath;

    QKeySequence captureDesktopKey = QKeySequence("Ctrl+Shift+2");
    QKeySequence captureWindowKey = QKeySequence("Ctrl+Shift+3");
    QKeySequence captureAreaKey = QKeySequence("Ctrl+Shift+4");
    QKeySequence uploadClipboardKey = QKeySequence("Ctrl+Shift+5");
    QKeySequence uploadFileKey = QKeySequence("Ctrl+Shift+U");

    QString loginName, apiKey;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Settings::PuushActions)

extern Settings settings;
