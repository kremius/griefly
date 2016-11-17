#include "mainform.h"
#include "ui_mainform.h"

#include <iostream>

#include "core/Map.h"
#include "Params.h"
#include "core/Game.h"
#include "net/MagicStrings.h"

#include "representation/Sound.h"

#include "representation/Screen.h"
#include "representation/Metadata.h"

#include "qtopengl.h"

#include "representation/Chat.h"
#include "net/Network2.h"
#include "net/NetworkMessagesTypes.h"
#include "representation/Text.h"

#include <QDebug>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTime>
#include <QTextBlock>
#include <QMessageBox>

#include "Version.h"

MainForm::MainForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainForm),
    fps_cap_(-1)
{
    ui->setupUi(this);

    ui->textBrowser->setAcceptRichText(false);
    ui->textBrowser->setContextMenuPolicy(Qt::NoContextMenu);
    ui->textBrowser->setOpenLinks(false);
    ui->textBrowser->setReadOnly(true);
    ui->textBrowser->setUndoRedoEnabled(false);

    {
        QList<int> sizes;
        sizes.push_back(512);
        sizes.push_back(256);
        ui->splitter->setSizes(sizes);
    }
    {
        QList<int> sizes;
        sizes.push_back(100);
        sizes.push_back(256);
        ui->splitterRight->setSizes(sizes);
    }


    setWindowTitle("Griefly " + QString(GetGameVersion()));

    left_column = ui->leftColumn->width();
    right_column = ui->rightColumn->width();

    ui->widget->hide();

    connect(&Network2::GetInstance(), &Network2::connectionSuccess,
            this, &MainForm::startGameLoop);
    connect(&Network2::GetInstance(), &Network2::connectionFailed,
            this, &MainForm::connectionFailed);

    connect(&Network2::GetInstance(), &Network2::mapSendingStarted,
            this, &MainForm::uploadStarted);
    connect(&Network2::GetInstance(), &Network2::mapSendingFinished,
            this, &MainForm::uploadFinished);

    connect(ui->widget, &QtOpenGL::enterPressed,
            this, &MainForm::setFocusOnLineEdit);
    connect(ui->widget, &QtOpenGL::f2Pressed,
            this, &MainForm::oocPrefixToLineEdit);

    connect(ui->lineEdit, &GamingLineEdit::keyToPass, ui->widget, &QtOpenGL::handlePassedKey);

    QTimer::singleShot(0, Qt::PreciseTimer, this, SLOT(connectToHost()));

    map_sending_ = false;
}

void MainForm::setFocusOnLineEdit()
{
    ui->lineEdit->setFocus();
}

MainForm::~MainForm()
{
    delete ui;
}

void MainForm::addSystemText(QString key, QString text)
{
    texts_[key] = text;
    if (text == "")
    {
        texts_.remove(key);
    }
}

void MainForm::resizeEvent(QResizeEvent* event) {
    ui->lineEdit->resize(width(), ui->lineEdit->height());
    ui->lineEdit->move(ui->lineEdit->x(), height() - ui->lineEdit->height());
    ui->splitter->resize(width(), ui->lineEdit->y());

    QList<int> sizes;
    sizes.push_back((width() * left_column) / (left_column + right_column));
    sizes.push_back((width() * right_column) / (left_column + right_column));
    ui->splitter->setSizes(sizes);
    on_splitter_splitterMoved(0, 0);
}

void MainForm::startGameLoop(int id, QString map)
{
    ui->widget->show();
    on_splitter_splitterMoved(0, 0);

    if (GetParamsHolder().GetParamBool("-nodraw"))
    {
        NODRAW = true;
    }

    if (GetParamsHolder().GetParamBool("-max_fps"))
    {
        fps_cap_ = GetParamsHolder().GetParam<int>("-max_fps");
    }

    SetRepresentation(new Representation);
    connect(ui->widget, &QtOpenGL::focusLost,
    []()
    {
        GetRepresentation().ResetKeysState();
    });

    Game* game = new Game;

    connect(game, &Game::insertHtmlIntoChat,
            this, &MainForm::insertHtmlIntoChat);
    connect(game, &Game::addSystemText,
            this, &MainForm::addSystemText);
    connect(game, &Game::playMusic,
            this, &MainForm::playMusic);

    connect(game, &Game::sendMap,
            &Network2::GetInstance(), &Network2::sendMap);

    game->InitWorld(id, map);

    connect(this, &MainForm::closing, game, &Game::endProcess);
    connect(this, &MainForm::generateUnsync, game, &Game::generateUnsync);


    QTimer text_updater;
    text_updater.setInterval(500);
    connect(&text_updater, &QTimer::timeout,
    [&]()
    {
        ui->mainTabTextBrowser->clear();
        ui->performanceTextBrowser->clear();
        for (auto it = texts_.begin(); it != texts_.end(); ++it)
        {
            // TODO: constants?
            if (it.key().startsWith("{Perf}"))
            {
                ui->performanceTextBrowser->insertHtml(*it + "<br>");
                continue;
            }
            ui->mainTabTextBrowser->insertHtml(*it + "<br>");
        }
    });

    text_updater.start();

    QElapsedTimer fps_timer;
    fps_timer.start();
    int fps_counter = 0;

    QElapsedTimer process_performance;
    qint64 max_process_time = 0;

    int time_per_frame_ms;
    if (fps_cap_ <= 0)
    {
        time_per_frame_ms = 0;
    }
    else
    {
        time_per_frame_ms = 1000 / fps_cap_;
    }
    while (true)
    {
        process_performance.start();
        GetRepresentation().Process();
        GetScreen().Swap();
        ++fps_counter;
        qint64 process_time = process_performance.nsecsElapsed();
        if (process_time > max_process_time)
        {
            max_process_time = process_time;
        }
        int sleep_time = time_per_frame_ms - process_time / 1e6;
        if (sleep_time > 0)
        {
            QThread::msleep(sleep_time);
        }
        if (isHidden())
        {
            break;
        }

        if (fps_timer.elapsed() > 1000)
        {
            addSystemText("FPS", "FPS: " + QString::number(fps_counter));
            addSystemText(
                "{Perf}Represent",
                QString("Represent max: %1 ms").arg(max_process_time / 1e6));
            qint64 mutex_ns = GetRepresentation().GetPerformance().mutex_ns;
            addSystemText(
                "{Perf}RepresentMutex",
                QString("Represent mutex lock max: %1 ms").arg(mutex_ns / 1e6));
            GetRepresentation().ResetPerformance();
            max_process_time = 0;
            fps_timer.restart();
            fps_counter = 0;
        }
    }

    emit closing();
    QCoreApplication::processEvents(QEventLoop::AllEvents);

    game->WaitForExit();

    Network2::GetInstance().Disconnect();

    return;
}

void MainForm::connectionFailed(QString reason)
{
    insertHtmlIntoChat("<b>Connection failed!</b>");
    insertHtmlIntoChat(reason);
    insertHtmlIntoChat("<br>If you would like to reconnect then do it manually (close the game window and start the game again)<br>");
}

void MainForm::insertHtmlIntoChat(QString html)
{
    QTextCursor cursor = ui->textBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser->setTextCursor(cursor);

    cursor.insertHtml(html);
    cursor.insertBlock();

    const int MIN_TEXT_BLOCKS = 3;
    const int MAX_SIZE_OF_DOCUMENT = 1500.0;

    while (   (ui->textBrowser->document()->blockCount() > MIN_TEXT_BLOCKS)
           && (ui->textBrowser->document()->size().height() > MAX_SIZE_OF_DOCUMENT))
    {
        RemoveFirstBlockFromTextEditor();
    }

    cursor = ui->textBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser->setTextCursor(cursor);
}

void MainForm::RemoveFirstBlockFromTextEditor()
{
    QTextCursor cursor = ui->textBrowser->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.removeSelectedText();
    cursor.select(QTextCursor::BlockUnderCursor);
    while (cursor.selectedText().length() == 0)
    {
        cursor.deleteChar();
        cursor.select(QTextCursor::BlockUnderCursor);
    }
}

void MainForm::playMusic(QString name, float volume)
{
    qDebug() << name << " " << volume;
    if (name != "")
    {
        GetSoundPlayer().PlayMusic(name, volume);
    }
    else
    {
        GetSoundPlayer().StopMusic();
    }
}

void MainForm::oocPrefixToLineEdit()
{
    ui->lineEdit->setText("OOC ");
}

void MainForm::uploadStarted()
{
    map_sending_ = true;
}

void MainForm::uploadFinished()
{
    map_sending_ = false;
}

void MainForm::closeEvent(QCloseEvent* event)
{
    const int WAIT_MAP_SENDING = 10 * 1000;
    if (map_sending_)
    {
        if (!close_request_timer_.isValid())
        {
            close_request_timer_.start();
        }
        QMessageBox::StandardButton answer;
        if (close_request_timer_.elapsed() < WAIT_MAP_SENDING)
        {
            answer = QMessageBox::information(
                this, "Griefly", "Wait a little bit (about 10 seconds)...");
        }
        else
        {
            answer = QMessageBox::question(
                this, "Griefly", "Are you sure? Something still in the process...",
                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                QMessageBox::No);
        }
        if (answer != QMessageBox::Yes)
        {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainForm::connectToHost()
{
    QString adrs = "127.0.0.1";
    if (GetParamsHolder().GetParamBool("ip"))
    {
        adrs = GetParamsHolder().GetParam<QString>("ip");
    }

    int port = 1111;
    if (GetParamsHolder().GetParamBool("port"))
    {
        port = GetParamsHolder().GetParam<int>("port");
    }

    QString login = "Guest";
    if (GetParamsHolder().GetParamBool("login"))
    {
        login = GetParamsHolder().GetParam<QString>("login");
    }

    QString password = "";
    if (GetParamsHolder().GetParamBool("password"))
    {
        password = GetParamsHolder().GetParam<QString>("password");
    }

    Network2::GetInstance().TryConnect(adrs, port, login, password);
}

bool IsOOCMessage(const QString& text)
{
    if (    text.length() >= 3
        && (   (text.left(3) == "OOC")
            || (text.left(3) == "ooc"))
        )
    {
        return true;
    }
    return false;
}

void MainForm::on_lineEdit_returnPressed()
{
    QString text = ui->lineEdit->text();
    ui->lineEdit->clear();
    if (text.length() == 0)
    {
        return;
    }

    if (text == "/generate_unsync")
    {
        qDebug() << "Unsync signal will be sended to the core...";
        emit generateUnsync();
        return;
    }
    if (text == "/restart_round")
    {
        qDebug() << "Restart round message will be sended to the server...";
        Message2 message;
        message.type = MessageType::RESTART_ROUND;
        Network2::GetInstance().SendMsg(message);
        return;
    }
    if (text.startsWith("/next_tick"))
    {
        QStringList list = text.split(" ");
        int count = 1;
        if (list.size() >= 2)
        {
            QString number = list.at(1);
            bool ok = false;
            count = number.toInt(&ok);
            if (!ok)
            {
                count = 1;
            }
        }
        qDebug() <<
            QString("%1 nexttick messages will be sended to the server...")
                .arg(count);
        Message2 message;
        message.type = MessageType::NEXT_TICK;
        for (int i = 0; i < count; ++i)
        {
            Network2::GetInstance().SendMsg(message);
        }
        return;
    }

    Message2 message;
    message.type = MessageType::MESSAGE;
    QJsonObject object;
    if (IsOOCMessage(text))
    {
        message.type = MessageType::OOC_MESSAGE;
        object["login"] = QString("");
        object["text"] = text.mid(3).trimmed();
    }
    else
    {
        object["text"] = text;
    }

    if (object["text"].toString().length() == 0)
    {
        return;
    }

    QJsonDocument doc(object);
    message.json = doc.toJson();

    Network2::GetInstance().SendMsg(message);
}

void MainForm::on_splitter_splitterMoved(int pos, int index)
{
    left_column = ui->leftColumn->width();
    right_column = ui->rightColumn->width();

    int min_size = std::min(ui->leftColumn->width(), ui->leftColumn->height());
    ui->widget->resize(min_size, min_size);

    ui->splitterRight->resize(ui->rightColumn->width(), ui->rightColumn->height());

    if (IsScreenValid())
    {
        GetScreen().PerformSizeUpdate();
        GetScreen().Clear();
        GetScreen().Swap();
    }

    int x_pos = (ui->leftColumn->width() - min_size) / 2;
    int y_pos = (ui->leftColumn->height() - min_size) / 2;
    ui->widget->move(x_pos, y_pos);
}
