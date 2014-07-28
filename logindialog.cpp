#include "logindialog.h"
#include "ui_logindialog.h"

#include <QPropertyAnimation>
#include <QMessageBox>


LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    ui->progressBar->setMaximumHeight(0);
    ui->progressBar->setVisible(false);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::showEvent(QShowEvent *)
{
    baseHeight = sizeHint().height();
    resize(sizeHint());
}

QString authenticate(QString user, QString pass);

void LoginDialog::on_logInButton_clicked()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "size");
    animation->setDuration(150);
    animation->setStartValue(size());
    animation->setEndValue(QSize(width(), baseHeight + ui->progressBar->sizeHint().height()));
    animation->setEasingCurve(QEasingCurve::OutExpo);
    animation->start();
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    connect(animation, &QPropertyAnimation::finished, [this](){
        ui->progressBar->setMaximumHeight(32);
        ui->progressBar->setVisible(true);

        QString key = authenticate(ui->usernameEdit->text(), ui->passwordEdit->text());
        if (key.isEmpty()) {
            ui->progressBar->setVisible(false);
            QPropertyAnimation *animation = new QPropertyAnimation(this, "size");
            animation->setDuration(250);
            animation->setStartValue(size());
            animation->setEndValue(QSize(width(), baseHeight));
            animation->setEasingCurve(QEasingCurve::OutExpo);
            animation->start();
            connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
            connect(animation, &QPropertyAnimation::finished, [this](){
                ui->logInButton->setEnabled(true);
                ui->loginDetailsGroupBox->setEnabled(true);
                QMessageBox::critical(this, QString(), "Login failed. Please check your credentials and try again.");
            });
        } else {
            QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
            animation->setDuration(250);
            animation->setStartValue(1.0);
            animation->setEndValue(0.0);
            animation->setEasingCurve(QEasingCurve::OutExpo);
            animation->start();
            connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
            connect(animation, &QPropertyAnimation::finished, [this](){
                done(0);
            });
        }
    });

    ui->logInButton->setDisabled(true);
    ui->loginDetailsGroupBox->setDisabled(true);
}

void LoginDialog::on_exitButton_clicked()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(250);
    animation->setStartValue(1);
    animation->setEndValue(0);
    animation->setEasingCurve(QEasingCurve::OutExpo);
    animation->start();
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    connect(animation, &QPropertyAnimation::finished, [this](){
        done(1);
    });
}
