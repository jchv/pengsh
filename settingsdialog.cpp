#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "settings.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    load();
}

SettingsDialog::~SettingsDialog()
{
    save();
    delete ui;
}

void SettingsDialog::load()
{
    ui->playNotificationSoundCheckBox->setChecked(settings.onPuushActions & Settings::PlaySound);
    ui->copyLinkToClipboardCheckBox->setChecked(settings.onPuushActions & Settings::CopyLink);
    ui->openInBrowserCheckBox->setChecked(settings.onPuushActions & Settings::OpenInBrowser);
    ui->saveLocalCopyCheckBox->setChecked(settings.onPuushActions & Settings::SaveToDisk);
    ui->savePathLineEdit->setText(settings.savePath);

    ui->showSettingsRadioButton->setChecked(settings.onDoubleClickAction == Settings::ShowSettings);
    ui->beginCaptureModeRadioButton->setChecked(settings.onDoubleClickAction == Settings::CaptureScreen);
    ui->openFileDialogRadioButton->setChecked(settings.onDoubleClickAction == Settings::UploadFile);

    ui->captureCurrentWindowKeySequenceEdit->setKeySequence(settings.captureWindowKey);
    ui->captureDesktopKeySequenceEdit->setKeySequence(settings.captureDesktopKey);
    ui->captureAreaKeySequenceEdit->setKeySequence(settings.captureAreaKey);
    ui->uploadClipboardKeySequenceEdit->setKeySequence(settings.uploadClipboardKey);
    ui->uploadFileKeySequenceEdit->setKeySequence(settings.uploadFileKey);

    ui->loggedInAs->setText(settings.loginName);
    ui->apiKey->setText(settings.apiKey);
}

void SettingsDialog::save()
{
    settings.onPuushActions = Settings::NoAction;
    if (ui->playNotificationSoundCheckBox->isChecked())
        settings.onPuushActions |= Settings::PlaySound;
    if (ui->copyLinkToClipboardCheckBox->isChecked())
        settings.onPuushActions |= Settings::CopyLink;
    if (ui->openInBrowserCheckBox->isChecked())
        settings.onPuushActions |= Settings::OpenInBrowser;
    if (ui->saveLocalCopyCheckBox->isChecked())
        settings.onPuushActions |= Settings::SaveToDisk;
    settings.savePath = ui->savePathLineEdit->text();

    if (ui->showSettingsRadioButton->isChecked())
        settings.onDoubleClickAction = Settings::ShowSettings;
    else if (ui->beginCaptureModeRadioButton->isChecked())
        settings.onDoubleClickAction = Settings::CaptureScreen;
    else if (ui->openFileDialogRadioButton->isChecked())
        settings.onDoubleClickAction = Settings::UploadFile;

    settings.captureWindowKey = ui->captureCurrentWindowKeySequenceEdit->keySequence();
    settings.captureDesktopKey = ui->captureDesktopKeySequenceEdit->keySequence();
    settings.captureAreaKey = ui->captureAreaKeySequenceEdit->keySequence();
    settings.uploadClipboardKey = ui->uploadClipboardKeySequenceEdit->keySequence();
    settings.uploadFileKey = ui->uploadFileKeySequenceEdit->keySequence();

    settings.flush();
}

void openAccountPage();
void SettingsDialog::on_myAccountButton_clicked() {
    openAccountPage();
}

bool loginPrompt();
void SettingsDialog::on_logoutButton_clicked() {
    close();

    settings.apiKey = QString();
    settings.loginName = QString();

    if(!loginPrompt())
        exit(0);

    settings.flush();
}
