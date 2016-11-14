#ifndef LAUNCHERFORM_H
#define LAUNCHERFORM_H

#include <QWidget>
#include <QEvent>
#include <QSystemTrayIcon>
#include <QSharedMemory>
#include <QTimer>
#include <QProcess>

namespace Ui {
class LauncherForm;
}

class LauncherForm : public QWidget
{
    Q_OBJECT

public:
    explicit LauncherForm(QWidget *parent = 0);
    ~LauncherForm();

    void changeEvent(QEvent* e);
public slots:
    void onShowHide(QSystemTrayIcon::ActivationReason reason);
    void checkSharedMemory();
    void onProcessEnd(int exitCode, QProcess::ExitStatus status);
private slots:
    void on_exitButton_clicked();
    void on_connectPushButton_clicked();

    void on_hidePushButton_clicked();

    void on_guestCheckBox_stateChanged(int arg1);

    void on_localhostCheckBox_stateChanged(int arg1);

private:
    void Popup();

    QProcess kv_process_;

    QSharedMemory pop_up_needed_;
    QTimer timer_;

    void CreateTrayIcon();

    QSystemTrayIcon* icon_;

    Ui::LauncherForm *ui;
};

#endif // LAUNCHERFORM_H
