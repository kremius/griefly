#include "Log.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QUuid>

#include "KVAbort.h"

QFile logs;
QTextStream logstream;

const char* const DATETIME_FORMAT = "yyyy.MM.dd.HH.mm.ss.zzz";

void KvMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    QString datetime = QDateTime::currentDateTimeUtc().toString(DATETIME_FORMAT);
    QString information;
    if (!context.file)
    {
        information = QString("%1: %2").arg(datetime).arg(message);
    }
    else
    {
        information
            = QString("%1 [%2 %3 %4 %5]:")
                .arg(datetime)
                .arg(context.file)
                .arg(context.line)
                .arg(context.function)
                .arg(message);
    }
    switch (type)
    {
    case QtDebugMsg:
        logstream << information << endl;
        break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    case QtInfoMsg:
        logstream << information << endl;
        break;
#endif // QT_VERSION
    case QtWarningMsg:
        logstream << information << endl;
        break;
    case QtCriticalMsg:
        logstream << information << endl;
        break;
    case QtFatalMsg:
        logstream << information << endl;
        KvAbort();
    }
}
void InstallMessageHandler()
{
    qInstallMessageHandler(KvMessageHandler);
}

void InitializeLog()
{
    const QString LOG_DIRECTORY = "debug_reports";

    if (!QDir(LOG_DIRECTORY).exists())
    {
        if (!QDir().mkdir(LOG_DIRECTORY))
        {
            qDebug() << "Unable to create directory:" << LOG_DIRECTORY;
            KvAbort();
        }
    }

    QString datetime = QDateTime::currentDateTimeUtc().toString(DATETIME_FORMAT);
    QString uuid = QUuid::createUuid().toString();
    QString filename
        = QString("%1/log-%2-%3.txt")
           .arg(LOG_DIRECTORY)
           .arg(datetime)
           .arg(uuid);
    logs.setFileName(filename);
    if (!logs.open(QIODevice::WriteOnly))
    {
        qDebug() << logs.fileName() << " cannot be opened.";
        KvAbort();
    }
    logstream.setDevice(&logs);

    InstallMessageHandler();
}

