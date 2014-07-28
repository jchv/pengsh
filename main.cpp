#include <QApplication>
#include <QtCore>
#include <QtWidgets>
#include <QtGui>
#include <QtMultimedia>
#include <QtNetwork>

#include "settings.h"
#include "settingsdialog.h"
#include "logindialog.h"
#include "x11-util.h"
#include "qxtglobalshortcut.h"


void setupMenu();

const QString apiRoot = "https://puush.me";
const QString accountUrl = "https://puush.me/login/go/?k=%1";

QNetworkAccessManager *nm;
QSystemTrayIcon *trayicon;
QMenu *menu = nullptr;

QAction
    *captureDesktopAction,
    *captureWindowAction,
    *captureAreaAction,
    *uploadClipboardAction,
    *uploadFileAction,
    *quitAction;

struct PuushedEntry {
    int id;
    QDateTime time;
    QString url;
    QString filename;
};

QList<PuushedEntry> recent;

/******************************************************************************/

void playSound(QString path) {
    QMediaPlayer *player = new QMediaPlayer;
    player->setMedia(QUrl(path));
    player->play();

    QObject::connect(player, &QMediaPlayer::stateChanged, &QMediaPlayer::deleteLater);
}

void setClipboardText(QString &text) {
    qApp->clipboard()->setText(text);
}

void openAccountPage() {
    QDesktopServices::openUrl(accountUrl.arg(settings.apiKey));
}

/******************************************************************************/

struct FormField {
    QString disposition;
    QString type;
    QByteArray body;
};

FormField textField(QString key, QString val) {
    FormField field;
    field.disposition = "form-data; name=\"" + key + "\"";
    field.body = val.toUtf8();
    return field;
}

FormField dataField(QString key, QString fn, QByteArray data) {
    FormField field;
    field.disposition = "form-data; name=\"" + key + "\"; filename=\"" + fn + "\"";
    field.type = "application/octet-stream";
    field.body = data;
    return field;
}

QHttpMultiPart *createForm(QList<FormField> fields) {
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    for (int i = 0; i < fields.count(); ++i) {
        FormField &field = fields[i];
        QHttpPart part;
        if (!field.type.isEmpty())
            part.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(field.type));
        if (!field.disposition.isEmpty())
            part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(field.disposition));
        part.setBody(field.body);
        multiPart->append(part);
    }
    return multiPart;
}

/******************************************************************************/

QNetworkReply *apiCall(const QString &path, QHttpMultiPart *form) {
    QString apiUrl = apiRoot + path;
    QNetworkRequest request(apiUrl);
    QNetworkReply *reply = nm->post(request, form);
    if (!reply) return nullptr;
    form->setParent(reply);

    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &eventLoop, SLOT(quit()));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        trayicon->showMessage(QObject::tr("Communication error."), reply->errorString(), QSystemTrayIcon::Critical);
        return nullptr;
    }

    return reply;
}


QString authenticate(QString user, QString pass) {
    QList<FormField> formData = {
        textField("e", user),
        textField("p", pass)
    };

    QHttpMultiPart *form = createForm(formData);
    QNetworkReply *reply = apiCall("/api/auth", form);

    if (!reply)
        return QString();

    QString replyText = QString::fromUtf8(reply->readAll());
    QStringList replyParts = replyText.split(',');

    if (replyParts.length() < 4)
        return QString();

    settings.loginName = user;
    settings.apiKey = replyParts[1];

    return replyParts[1];
}


QStringList history() {
    QList<FormField> formData = {
        textField("k", settings.apiKey.toUtf8())
    };

    QHttpMultiPart *form = createForm(formData);
    QNetworkReply *reply = apiCall("/api/hist", form);

    if (!reply)
        return QStringList();

    QString replyText = QString::fromUtf8(reply->readAll());
    QStringList lines = replyText.split('\n', QString::SkipEmptyParts);

    if (lines.isEmpty())
        return QStringList();
    else if (lines.at(0).trimmed() != "0")
        return QStringList();
    else
        lines.removeFirst();

    for (int i = 0; i < lines.count(); ++i) {
        QStringList lineParts = lines.at(i).split(',');

        if (lineParts.length() < 4)
            continue;

        int id = lineParts[0].toInt();
        QDateTime time = QDateTime::fromString(lineParts[1], "yyyy-MM-dd HH:mm:ss");
        QString url = lineParts[2];
        QString fn = lineParts[3];
        recent << PuushedEntry{id, time, url, fn};
    }

    return QStringList();
}

struct PuushFile {
    QByteArray data;
    QString name;
};

QString upload(PuushFile file) {
    QDateTime time = QDateTime::currentDateTime();
    QList<FormField> formData = {
        textField("k", settings.apiKey.toUtf8()),
        textField("z", "poop"),
        dataField("f", file.name, file.data)
    };

    QHttpMultiPart *form = createForm(formData);
    QNetworkReply *reply = apiCall("/api/up", form);

    if (!reply)
        return QString();

    QString replyText = QString::fromUtf8(reply->readAll());
    QStringList replyParts = replyText.split(',');

    if (replyParts.length() < 3) {
        if (!replyParts.isEmpty()) {
            if (replyParts.first() == "-2")
                trayicon->showMessage(QObject::tr("Upload error."), QObject::tr("Server rejected request."), QSystemTrayIcon::Critical);
            else if (replyParts.first() == "-1")
                trayicon->showMessage(QObject::tr("Upload error."), QObject::tr("Server error."), QSystemTrayIcon::Critical);
            else
                trayicon->showMessage(QObject::tr("Upload error."), QObject::tr("Unknown server error."), QSystemTrayIcon::Critical);
        } else {
            trayicon->showMessage(QObject::tr("Upload error."), QObject::tr("Empty response."), QSystemTrayIcon::Critical);
        }
        return QString();
    }

    recent.prepend({replyParts.at(2).toInt(), time, replyParts.at(1), file.name});
    setupMenu();

    QString url = replyParts.at(1);

    trayicon->showMessage(QObject::tr("Upload successful."), url, QSystemTrayIcon::Information, 5000);

    if (settings.onPuushActions & Settings::PlaySound)
        playSound("qrc:/notify.wav");
    if (settings.onPuushActions & Settings::CopyLink)
        setClipboardText(url);
    if (settings.onPuushActions & Settings::OpenInBrowser)
        QDesktopServices::openUrl(url);

    return url;
}

/******************************************************************************/

QString generateFilename(QString fmt) {
    QDateTime time = QDateTime::currentDateTime();
    return QString(fmt).arg(time.toString("yyyy-MM-dd HH:mm:ss"));
}

template <typename T>
PuushFile compressImage(QString type, T &pix) {
    QByteArray jpgData, pngData;
    QBuffer jpgBuffer(&jpgData), pngBuffer(&pngData);

    pix.save(&jpgBuffer, "JPG", 95);
    pix.save(&pngBuffer, "PNG");

    if (pngData.size() < jpgData.size())
        return PuushFile { pngData, generateFilename(type + " (%1).png") };
    else
        return PuushFile { jpgData, generateFilename(type + " (%1).jpg") };
}

bool writeFileToDisk(PuushFile d) {
    QFile file(QString("%1/%2").arg(settings.savePath, d.name));

    if (!QDir(settings.savePath).exists())
        QDir().mkpath(settings.savePath);

    if (!file.open(QFile::WriteOnly))
        return false;

    file.write(d.data);
    file.close();

    return true;
}

QPixmap grabEntireScreen() {
    auto screen = qApp->screens().first();
    auto desktopWId = qApp->desktop()->winId();
    return screen->grabWindow(desktopWId);
}

QString takeScreenshot(QRect area) {
    QPixmap shot = grabEntireScreen();

    if (area.isValid())
        shot = shot.copy(area);

    PuushFile file = compressImage("ss", shot);

    if (settings.onPuushActions & Settings::SaveToDisk)
        writeFileToDisk(file);

    return upload(file);
}

void takeCurrentWindow() {
    takeScreenshot(activeWinRect());
}

void takeEntireDesktop() {
    takeScreenshot(QRect());
}

void takeArea() {
    takeScreenshot(rectSelect());
}

void uploadClipboard() {
    QClipboard *clipboard = qApp->clipboard();
    const QMimeData *data = clipboard->mimeData();

    if (data->hasText()) {
        upload(PuushFile{data->text().toUtf8(), generateFilename("clipboard (%1).txt")});
    } else if (data->hasImage()) {
        QImage img = data->imageData().value<QImage>();
        upload(compressImage("clipboard", img));
    } else {
        QStringList formats = data->formats();
        if (formats.isEmpty()) {
            trayicon->showMessage(QObject::tr("Error."), QObject::tr("The clipboard is empty."), QSystemTrayIcon::Critical);
            return;
        }

        QByteArray buffer = data->data(formats.first());
        if (buffer.isEmpty()) {
            trayicon->showMessage(QObject::tr("Error."), QObject::tr("The clipboard is empty."), QSystemTrayIcon::Critical);
            return;
        }

        upload(PuushFile{buffer, generateFilename("clipboard (%1).bin")});
    }
}

/* TODO: File manager integration? */
void uploadFile() {
    QString filepath;
    PuushFile fileobject;
    QFile file;

    filepath = QFileDialog::getOpenFileName(0, QObject::tr("Puush file"));

    if (filepath.isEmpty())
        return;

    file.setFileName(filepath);
    if (!file.open(QFile::ReadOnly)) {
        trayicon->showMessage(QObject::tr("Error."), QObject::tr("Could not open file for reading."), QSystemTrayIcon::Critical);
        return;
    }

    fileobject.name = QFileInfo(filepath).fileName();
    fileobject.data = file.readAll();

    file.close();
    upload(fileobject);
}

void showSettings() {
    SettingsDialog diag;
    diag.exec();
}

void trayClicked(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        switch (settings.onDoubleClickAction) {
        case Settings::ShowSettings:
            showSettings();
            break;
        case Settings::CaptureScreen:
            takeArea();
            break;
        case Settings::UploadFile:
            uploadFile();
            break;
        }
    }
}

bool loginPrompt() {
    LoginDialog login;
    int result = login.exec();
    settings.flush();

    return result == 0;
}

/******************************************************************************/

/* Create the actions, link them up. */
void setupActions() {
    captureWindowAction = new QAction(QObject::tr("Capture Current &Window"), qApp);
    captureWindowAction->setShortcut(settings.captureWindowKey);
    QObject::connect(captureWindowAction, &QAction::triggered, &takeCurrentWindow);
    captureDesktopAction = new QAction(QObject::tr("Capture &Desktop"), qApp);
    captureDesktopAction->setShortcut(settings.captureDesktopKey);
    QObject::connect(captureDesktopAction, &QAction::triggered, &takeEntireDesktop);
    captureAreaAction = new QAction(QObject::tr("Capture &Area..."), qApp);
    captureAreaAction->setShortcut(settings.captureAreaKey);
    QObject::connect(captureAreaAction, &QAction::triggered, &takeArea);
    uploadClipboardAction = new QAction(QObject::tr("Upload &Clipboard"), qApp);
    uploadClipboardAction->setShortcut(settings.uploadClipboardKey);
    QObject::connect(uploadClipboardAction, &QAction::triggered, &uploadClipboard);
    uploadFileAction = new QAction(QObject::tr("Upload &File"), qApp);
    uploadFileAction->setShortcut(settings.uploadFileKey);
    QObject::connect(uploadFileAction, &QAction::triggered, &uploadFile);
    quitAction = new QAction(QObject::tr("&Quit"), qApp);
    QObject::connect(quitAction, &QAction::triggered, qApp, QApplication::quit);
}

/* Create the menu. */
void setupMenu() {
    if (!menu)
        menu = new QMenu("pengsh");
    else
        menu->clear();
    QAction *acct = menu->addAction(QObject::tr("My account"));
    QObject::connect(acct, &QAction::triggered, &openAccountPage);
    QAction *cfg = menu->addAction(QObject::tr("Settings"));
    QObject::connect(cfg, &QAction::triggered, &showSettings);
    menu->addSection(QObject::tr("Recent puushes"));
    if (!recent.isEmpty()) {
        for (int i = 0; i < 5; ++i) {
            if (i >= recent.count())
                break;
            QAction *recentItem = menu->addAction(recent.at(i).filename);
            QObject::connect(recentItem, &QAction::triggered, [=](){
                QDesktopServices::openUrl(recent.at(i).url);
            });
        }
    } else {
        menu->addAction("No recent puushes.")->setDisabled(true);
    }
    menu->addSection(QObject::tr("Take Screenshot"));
    menu->addActions({captureDesktopAction, captureWindowAction, captureAreaAction});
    menu->addActions({uploadClipboardAction, uploadFileAction});
    menu->addSeparator();
    menu->addActions({quitAction});
}

/* Setup the keyboard shortcuts. */
void setupShortcuts() {
    QxtGlobalShortcut *captureWindowShortcut = new QxtGlobalShortcut(settings.captureWindowKey, qApp);
    QObject::connect(captureWindowShortcut, &QxtGlobalShortcut::activated, &takeCurrentWindow);
    QxtGlobalShortcut *captureDesktopShortcut = new QxtGlobalShortcut(settings.captureDesktopKey, qApp);
    QObject::connect(captureDesktopShortcut, &QxtGlobalShortcut::activated, &takeEntireDesktop);
    QxtGlobalShortcut *captureAreaShortcut = new QxtGlobalShortcut(settings.captureAreaKey, qApp);
    QObject::connect(captureAreaShortcut, &QxtGlobalShortcut::activated, &takeArea);
    QxtGlobalShortcut *uploadClipboardShortcut = new QxtGlobalShortcut(settings.uploadClipboardKey, qApp);
    QObject::connect(uploadClipboardShortcut, &QxtGlobalShortcut::activated, &uploadClipboard);
    QxtGlobalShortcut *uploadFileShortcut = new QxtGlobalShortcut(settings.uploadFileKey, qApp);
    QObject::connect(uploadFileShortcut, &QxtGlobalShortcut::activated, &uploadFile);
}

/* Entrypoint */
int main(int argc, char **argv) {
    int result = 0;

    QApplication::setApplicationDisplayName("pengsh");
    QApplication::setApplicationName("pengsh");
    QApplication::setOrganizationName("jchadwick");
    QApplication::setOrganizationDomain("jchadwick.net");

    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    settings.load();

    nm = new QNetworkAccessManager();

    setupActions();
    setupMenu();
    setupShortcuts();

    if (settings.apiKey.isEmpty())
        if (!loginPrompt())
            return 0;

    trayicon = new QSystemTrayIcon(QIcon(":/icon.png"));
    trayicon->setContextMenu(menu);
    trayicon->show();
    QObject::connect(trayicon, &QSystemTrayIcon::activated, &trayClicked);

    history();
    setupMenu();

    result = a.exec();

    settings.flush();
    return result;
}
