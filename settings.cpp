#include "settings.h"
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSettings>

#include <QDebug>

Settings settings;

Settings::Settings() {
    savePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    if (savePath.isEmpty())
        savePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    if (savePath.isEmpty())
        savePath = ".";
}

Settings::~Settings() {
}

bool Settings::load() {
    QSettings settings;

    /* General settings */
    if (settings.contains("onPuushActions"))
        onPuushActions = PuushAction(settings.value("onPuushActions").toInt());
    if (settings.contains("onDoubleClickAction"))
        onDoubleClickAction = DoubleClickAction(settings.value("onDoubleClickAction").toInt());

    /* Keyboard shortcuts */
    if (settings.contains("shortcuts/captureDesktop"))
        captureDesktopKey = settings.value("shortcuts/captureDesktop").value<QKeySequence>();
    if (settings.contains("shortcuts/captureWindow"))
        captureWindowKey = settings.value("shortcuts/captureWindow").value<QKeySequence>();
    if (settings.contains("shortcuts/captureArea"))
        captureAreaKey = settings.value("shortcuts/captureArea").value<QKeySequence>();
    if (settings.contains("shortcuts/uploadClipboard"))
        uploadClipboardKey = settings.value("shortcuts/uploadClipboard").value<QKeySequence>();
    if (settings.contains("shortcuts/uploadFile"))
        uploadFileKey = settings.value("shortcuts/uploadFile").value<QKeySequence>();

    /* Account */
    if (settings.contains("account/loginName"))
        loginName = settings.value("account/loginName").toString();
    if (settings.contains("account/apiKey"))
        apiKey = settings.value("account/apiKey").toString();

    return true;
}

void Settings::flush() {
    QSettings settings;

    /* General settings */
    settings.setValue("onPuushActions", int(onPuushActions));
    settings.setValue("onDoubleClickAction", int(onDoubleClickAction));

    /* Keyboard shortcuts */
    settings.setValue("shortcuts/captureDesktop", captureDesktopKey);
    settings.setValue("shortcuts/captureWindow", captureWindowKey);
    settings.setValue("shortcuts/captureArea", captureAreaKey);
    settings.setValue("shortcuts/uploadClipboard", uploadClipboardKey);
    settings.setValue("shortcuts/uploadFile", uploadFileKey);

    /* Account */
    settings.setValue("account/loginName", loginName);
    settings.setValue("account/apiKey", apiKey);

    settings.sync();
}
