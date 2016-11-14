#include "launcherform.h"
#include "ui_launcherform.h"

#include <QTimer>
#include <QPixmap>
#include <QMessageBox>
#include <QFileInfo>

LauncherForm::LauncherForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LauncherForm)
{
    ui->setupUi(this);

    pop_up_needed_.setKey("4668074d-98a4-4052-b8fb-0a3a7a84905c");

    bool is_shared_memory_created = pop_up_needed_.create(1);
    if (   !is_shared_memory_created
        && (pop_up_needed_.error() == QSharedMemory::SharedMemoryError::AlreadyExists))
    {
        pop_up_needed_.attach();
    }

    connect(&kv_process_, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &LauncherForm::onProcessEnd);
    connect(&timer_, &QTimer::timeout, this, &LauncherForm::checkSharedMemory);

    timer_.start(300);
    setWindowFlags(Qt::SubWindow);
    CreateTrayIcon();
}

LauncherForm::~LauncherForm()
{
    delete ui;
}

void LauncherForm::changeEvent(QEvent* e)
{
    switch (e->type())
    {
    case QEvent::WindowStateChange:
        if (this->windowState() & Qt::WindowMinimized)
        {
            // SLOT is workaround for travis build
            QTimer::singleShot(0, this, SLOT(hide()));
        }
        break;
    default:
        break;
    }

    QWidget::changeEvent(e);
}

void LauncherForm::onShowHide(QSystemTrayIcon::ActivationReason reason)
{
    if (reason)
    {
        if (reason != QSystemTrayIcon::DoubleClick)
        {
            return;
        }
    }

    Popup();
}

void LauncherForm::checkSharedMemory()
{
    pop_up_needed_.lock();

    bool* is_needed = static_cast<bool*>(pop_up_needed_.data());

    if (is_needed && *is_needed)
    {
        Popup();
        *is_needed = false;
    }

    pop_up_needed_.unlock();
}

void LauncherForm::onProcessEnd(int exitCode, QProcess::ExitStatus status)
{
    Popup();
    ui->connectPushButton->setEnabled(true);
}

void LauncherForm::CreateTrayIcon()
{
    QPixmap pixmap(32, 32);
    pixmap.fill(QColor("red"));
    QIcon red_icon(pixmap);

    icon_ = new QSystemTrayIcon(red_icon, this);
    connect(icon_, &QSystemTrayIcon::activated, this, &LauncherForm::onShowHide);
    icon_->show();
}

void LauncherForm::on_exitButton_clicked()
{
    if (kv_process_.state() == QProcess::Running)
    {
        QMessageBox::StandardButton reply
                = QMessageBox::question(
                    this, "Terminate launcher", "Are you sure? The game window will be closed!",
                    QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
        {
            return;
        }
    }
    QApplication::exit();
}

void LauncherForm::Popup()
{
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();  // for MacOS
    activateWindow(); // for Windows
    show();
    setFocus();
}

void LauncherForm::on_connectPushButton_clicked()
{
    QStringList args;
    if (ui->localhostCheckBox->isChecked() == true)
    {
        args.push_back("ip=127.0.0.1");
    }
    else
    {
        args.push_back("ip=" + ui->serverLineEdit->text());
    }
    if (ui->guestCheckBox->isChecked() == true)
    {
        args.push_back("login=Guest");
    }
    else
    {
        args.push_back("login=" + ui->loginLineEdit->text());
        args.push_back("password=" + ui->passwordLineEdit->text());
    }
    args.push_back("port=" + ui->portLineEdit->text());
    args.push_back("-auto_connect");
    args.push_back("mapgen_name=maps/brig_small.gen");

    QString custom_args = ui->customArgsLineEdit->text();
    args.append(custom_args.split(" "));

    hide();
    ui->connectPushButton->setEnabled(false);

    const QString TARGET_NAME = "KVEngine";

    // The binary does not have .exe extension on Linux

    QFileInfo file_info(TARGET_NAME + ".exe");
    if (file_info.exists())
    {
        kv_process_.start(TARGET_NAME + ".exe", args);
    }
    else
    {
        kv_process_.start("./" + TARGET_NAME, args);
    }
}

void LauncherForm::on_hidePushButton_clicked()
{
    hide();
}

void LauncherForm::on_guestCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        ui->passwordLineEdit->setEnabled(false);
        ui->loginLineEdit->setEnabled(false);
    }
    else
    {
        ui->passwordLineEdit->setEnabled(true);
        ui->loginLineEdit->setEnabled(true);
    }
}

void LauncherForm::on_localhostCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        ui->serverLineEdit->setEnabled(false);
    }
    else
    {
        ui->serverLineEdit->setEnabled(true);
    }
}
