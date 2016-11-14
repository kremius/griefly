#include "Network2.h"

#include <QMutexLocker>
#include <QtEndian>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QVariant>
#include <QUrl>
#include <QElapsedTimer>

#include <QCoreApplication>

#include "NetworkMessagesTypes.h"
#include "Version.h"

QJsonObject Network2::ParseJson(Message2 message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.json);
    return doc.object();
}

bool Network2::IsKey(const QJsonObject& json, const QString& key)
{
    if (json["key"] == key)
    {
        return true;
    }
    return false;
}

quint32 Network2::ExtractObjId(const QJsonObject &json)
{
    QJsonValue val = json["obj"];

    return val.toVariant().toInt();
}

QString Network2::ExtractAction(const QJsonObject &json)
{
    QJsonValue val = json["action"];

    return val.toVariant().toString();
}

Message2 Network2::MakeClickMessage(int object_id, QString click_type)
{
    Message2 msg;

    msg.type = MessageType::MOUSE_CLICK;

    QJsonObject obj;
    obj.insert("action", click_type);

    QJsonValue value(object_id);
    obj.insert("obj", value);
    QJsonDocument doc(obj);
    msg.json = doc.toJson(QJsonDocument::Compact);

    return msg;
}

Network2 &Network2::GetInstance()
{
    static Network2* network = new Network2;
    return *network;
}

bool Network2::IsGood()
{
    return is_good_;
}

void Network2::sendMap(QString url, QByteArray data)
{
    QString type_header = "application/octet-stream";
    if (prefer_compress_)
    {
        type_header = "application/zip";
        data = qCompress(data);
    }

    QUrl send_url(url);
    QNetworkRequest request(send_url);
    request.setHeader(QNetworkRequest::ContentLengthHeader, data.length());
    request.setHeader(QNetworkRequest::ContentTypeHeader, type_header);

    emit mapSendingStarted();
    net_manager_->post(request, data);

    qDebug() << "Map has been sended to " << url << ", length: " << data.length();
}

Network2::Network2()
    : handler_(this)
{
    prefer_compress_ = true;

    net_manager_ = new QNetworkAccessManager(this);

    handler_.moveToThread(&thread_);
    connect(&thread_, &QThread::started, &handler_, &SocketHandler::process);

    connect(net_manager_, &QNetworkAccessManager::finished, this, &Network2::mapDownloaded);

    connect(this, &Network2::connectRequested, &handler_, &SocketHandler::tryConnect);
    connect(&handler_, &SocketHandler::readyToStart, this, &Network2::downloadMap);
    connect(this, &Network2::sendMessage, &handler_, &SocketHandler::sendMessage);
    connect(&handler_, &SocketHandler::connectionEnd, this, &Network2::onConnectionEnd);
    connect(this, &Network2::disconnectRequested, &handler_, &SocketHandler::disconnectSocket);
}

void Network2::TryConnect(QString host, int port, QString login, QString password)
{
    thread_.start();
    emit connectRequested(host, port, login, password);
}

void Network2::SendMsg(Message2 message)
{
    emit sendMessage(message);
}

void Network2::SendOrdinaryMessage(QString text)
{
    Message2 msg;
    msg.type = MessageType::ORDINARY;
    msg.json.append("{\"key\":\"" + text + "\"}");

    SendMsg(msg);
}

void Network2::SendPing(QString ping_id)
{
    Message2 msg;
    msg.type = MessageType::PING;
    msg.json.append("{\"ping_id\":\"" + ping_id + "\"}");

    SendMsg(msg);
}

void Network2::Disconnect()
{
    emit disconnectRequested();
    thread_.wait();
}

bool Network2::IsMessageAvailable()
{
    QMutexLocker locker(&queue_mutex_);

    if (received_messages_.size() == 0)
    {
        queue_wait_.wait(&queue_mutex_, MAX_WAIT_ON_QUEUE);
    }
    return received_messages_.size() > 0;
}

Message2 Network2::PopMessage()
{
    QMutexLocker locker(&queue_mutex_);
    return received_messages_.dequeue();
}

void Network2::PushMessage(Message2 message)
{
    QMutexLocker locker(&queue_mutex_);
    received_messages_.enqueue(message);
    queue_wait_.wakeOne();
}

QByteArray Network2::GetMapData()
{
    return map_data_;
}

void Network2::onConnectionEnd(QString reason)
{
    is_good_ = false;
    emit connectionFailed(reason);
}

void Network2::mapDownloaded(QNetworkReply* reply)
{
    if (reply->request().url() != map_url_)
    {
        emit mapSendingFinished();
        qDebug() << "End map upload";
        reply->deleteLater();
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        emit connectionFailed("Unable download map: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    map_data_ = reply->readAll();
    if (reply->header(QNetworkRequest::ContentTypeHeader) == "application/zip")
    {
        qDebug() << "Compressed map length: " << map_data_.length();
        map_data_ = qUncompress(map_data_);
    }
    reply->deleteLater();

    qDebug() << "Map length: " << map_data_.length();

    is_good_ = true;
    emit connectionSuccess(your_id_, "map_buffer");
}

void Network2::downloadMap(int your_id, QString map)
{
    your_id_ = your_id;
    map_url_ = map;
    if (map == "no_map")
    {
        is_good_ = true;
        emit connectionSuccess(your_id, map);
        return;
    }

    qDebug() << "Begin download map from " << map_url_;

    QUrl url(map);
    QNetworkRequest r(url);

    net_manager_->get(r);
}

SocketHandler::SocketHandler(Network2* network)
    : network_(network),
      socket_(this)
{    
    state_ = NetworkState::NOT_CONNECTED;

    net_codec_ = QTextCodec::codecForName("UTF-8");

    connect(&socket_, &QTcpSocket::connected, this, &SocketHandler::socketConnected);
    connect(&socket_,
            static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error),
            this,
            &SocketHandler::errorSocket);

    connect(this, &SocketHandler::firstMessage, this, &SocketHandler::handleFirstMessage);
    connect(&socket_, &QTcpSocket::readyRead, this, &SocketHandler::handleNewData);
}

void SocketHandler::process()
{
    reading_state_ = ReadingState::HEADER;
    is_first_message_ = true;
    buffer_pos_ = 0;
}

void SocketHandler::handleNewData()
{
    QByteArray new_data = socket_.readAll();
    buffer_.append(new_data);

    bool is_continue = true;
    while (is_continue)
    {
        switch (reading_state_)
        {
        case ReadingState::HEADER:
            is_continue = HandleHeader();
            break;
        case ReadingState::BODY:
            is_continue = HandleBody();
            break;
        }
    }

    const int MAX_BUFFER_SIZE = 1024 * 128;
    if (buffer_.size() > MAX_BUFFER_SIZE)
    {
        buffer_.remove(0, buffer_pos_);
        buffer_pos_ = 0;
    }
}

const int HEADER_SIZE = 8;

bool SocketHandler::HandleHeader()
{
    if (buffer_.size() - buffer_pos_ < HEADER_SIZE)
    {
        return false;
    }
    message_size_ = qFromBigEndian<qint32>(
        (const uchar*)&(buffer_.constData()[buffer_pos_]));
    message_type_ = qFromBigEndian<qint32>(
        (const uchar*)&(buffer_.constData()[buffer_pos_ + 4]));

    buffer_pos_ += 8;
    reading_state_ = ReadingState::BODY;
    return true;
}

bool SocketHandler::HandleBody()
{
    if (buffer_.size() - buffer_pos_ < message_size_)
    {
        return false;
    }

    Message2 new_message;
    new_message.type = message_type_;

    new_message.json.append(net_codec_->toUnicode(
        (const char*)&(buffer_.constData()[buffer_pos_]),
        message_size_));
    buffer_pos_ += message_size_;

    if (is_first_message_)
    {
        emit firstMessage(new_message);
        is_first_message_ = false;
    }
    else
    {
        network_->PushMessage(new_message);
    }
    reading_state_ = ReadingState::HEADER;

    return true;
}


void SocketHandler::tryConnect(QString host, int port, QString login, QString password)
{
    login_ = login;
    password_ = password;

    socket_.setSocketOption(QAbstractSocket::LowDelayOption, 1);

    socket_.connectToHost(host, port);

    qDebug() << "tryConnect()";
}

void SocketHandler::socketConnected()
{
    qDebug() << "socketConnected()";

    SendData("S132");

    Message2 login_message;
    login_message.type = MessageType::INITAL_LOGIN_MESSAGE;

    QJsonObject obj;
    obj["login"] = login_;
    obj["password"] = password_;

    // It is compile time macro with version (/D or -D)
    obj["game_version"] = QString(GetGameVersion());

    bool is_guest = (login_ == "Guest");
    obj["guest"] = is_guest;

    QJsonDocument doc(obj);

    login_message.json = doc.toJson();

    qDebug() << login_message.json;

    sendMessage(login_message);

    state_ = NetworkState::CONNECTED;
}

void SocketHandler::sendMessage(Message2 message)
{
    QByteArray json = net_codec_->fromUnicode(message.json);

    QByteArray data;

    uchar temp[4];

    qToBigEndian(json.size(), temp);
    data.append((char*)temp, 4);

    qToBigEndian(message.type, temp);
    data.append((char*)temp, 4);

    data.append(json);

    SendData(data);
}

void SocketHandler::disconnectSocket()
{
    socket_.close();
    network_->thread_.exit(-1);
}

void SocketHandler::errorSocket(QAbstractSocket::SocketError error)
{
    qDebug() << "socketError()";
    qDebug() << error;
    state_ = NetworkState::NOT_CONNECTED;
    network_->thread_.exit(-1);
    emit connectionEnd(
              "Socket error: "
            + QString::number(error)
            + ", reason: "
            + possible_error_reason_);
}

void SocketHandler::handleFirstMessage(Message2 m)
{
    switch (m.type)
    {
    case MessageType::WRONG_GAME_VERSION:
        qDebug() << "Wrong game version";
        qDebug() << m.json;
        possible_error_reason_ = "Wrong game version: " + m.json;
        break;
    case MessageType::WRONG_AUTHENTICATION:
        qDebug() << "Wrong authentication";
        qDebug() << m.json;
        possible_error_reason_ = "Wrong authentication";
        break;
    case MessageType::SUCCESS_CONNECTION:
        qDebug() << "Success";
        qDebug() << m.json;
        HandleSuccessConnection(m);
        break;
    case MessageType::MASTER_CLIENT_IS_NOT_CONNECTED:
        qDebug() << "Master client is not connected";
        possible_error_reason_ = "Master client is not connected";
        break;
    case MessageType::INTERNAL_SERVER_ERROR:
        qDebug() << "The server experiences some internal troubles";
        possible_error_reason_ = "The server experiences some internal troubles: " + m.json;
        break;
    default:
        qDebug() << "Unknown message type: " << m.type;
        qDebug() << m.json;
        possible_error_reason_ = "Unknown message type";
        break;
    }
}

void SocketHandler::HandleSuccessConnection(Message2 message)
{
    QJsonObject obj = Network2::ParseJson(message);

    qDebug() << obj["map"];
    qDebug() << obj["your_id"];

    map_ = obj["map"].toString();

    QJsonValue val = obj["your_id"];
    your_id_ = val.toVariant().toInt();

    emit readyToStart(your_id_, map_);
}

void SocketHandler::SendData(const QByteArray &data)
{
    int counter = 0;
    while (counter != data.length() && socket_.isValid())
    {
        counter += socket_.write(data.data() + counter, data.length() - counter);
    }
}
