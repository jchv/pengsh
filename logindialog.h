#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

protected:
    void showEvent(QShowEvent *);

private slots:
    void on_logInButton_clicked();

    void on_exitButton_clicked();

private:
    Ui::LoginDialog *ui;
    int baseHeight;
};

#endif // LOGINDIALOG_H
