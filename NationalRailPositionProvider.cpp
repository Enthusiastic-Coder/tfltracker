
#include <jibbs/utilities/stdafx.h>

#include "NationalRailPositionProvider.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtConcurrent>
#include <QFuture>

#include "Line.h"

namespace  {

static QString topicID = "/topic/TRAIN_MVT_%1_TOC";

void connectStomp(QStompClient& client)
{
    if( client.isUnconnected())
    {
        client.connectToHost("publicdatafeeds.networkrail.co.uk", 61618);
    }
}

void disconnectStomp(QStompClient& client)
{
    if( !client.isUnconnected())
    {
        client.disconnectFromHost();
    }
}

QString getTopicID(QString id)
{
    return topicID.arg(id);
}

void unsubcribeTopic(QStompClient& client, QString id)
{
    client.unsubscribe(getTopicID(id).toLocal8Bit());
}

void subscribeTopic(QStompClient& client, QString id)
{
    client.subscribe(getTopicID(id).toLocal8Bit(), true);
}

std::shared_ptr<NetworkRailScheduleJSON> loadNetworkRailInThread(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open file:" << filePath;
        return nullptr;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "JSON Parse Error:" << parseError.errorString();
        return nullptr;
    }

    auto model = std::make_shared<NetworkRailScheduleJSON>();
    model->loadFromJson(jsonDoc);
    return model;
}

}

const QHash<QString, QString> &NationalRailPositionProvider::getLineIdToLineCodeMap() const
{
    static const QHash<QString, QString> stompLineMap = {
        {"c2c", "HT"},
        {"heathrow-express", "HM"},
        {"southeastern", "HU"},
        {"south-western-railway", "HY"},
        {Line::thamesLink, "ET"},
        {Line::elizabethLine, "EX"}
    };
    return stompLineMap;
}

const QHash<QString, QString> &NationalRailPositionProvider::getTocIdToLineCodeMap() const
{
    static const QHash<QString, QString> tocMap{
        {"79", "HT"},
        {"86", "HM"},
        {"80", "HU"},
        {"84", "HY"},
        {"88", "ET"},
        {"33", "EX"}
    };

    return tocMap;
}

void NationalRailPositionProvider::loadScheduleDataForToday()
{
    if( QDate::currentDate() ==_lastCheckedDate)
    {
        return;
    }

    _future.waitForFinished();

    _lastCheckedDate = QDate::currentDate();
    _networkRailScheduleJSON.reset();
    _future = QtConcurrent::run(loadNetworkRailInThread, ":/data/NetworkRail/schedule-toc.json");

    // Timer to check the shared pointer in the main/UI thread
    QTimer *timer = new QTimer{this};
    QObject::connect(timer, &QTimer::timeout, [this, timer]() {

        if (_future.isFinished() && !_networkRailScheduleJSON)
        {
            _networkRailScheduleJSON = _future.result();
            timer->stop();
            timer->deleteLater();

            if (_networkRailScheduleJSON)
            {
                qDebug() << "schedule-toc successfully loaded in the worker thread!";
            }
            else
            {
                qWarning() << "Failed to schedule-toc.";
            }
        }
        else if (!_networkRailScheduleJSON)
        {
            qDebug() << "schedule-toc not loaded yet. Waiting...";
        }
    });

    timer->start(1000); // Check every {} ms

}

void NationalRailPositionProvider::sendHeartBeat()
{
    if(!_ids.empty())
    {
        _stompClient.send(QByteArray(), QString("\n"));
    }
}

NationalRailPositionProvider::NationalRailPositionProvider(QObject *parent) : QObject(parent)
{
    QObject::connect(&_stompClient, &QStompClient::socketConnected, this, [this] {
        qDebug() << "QStompClient::socketConnected";

        _stompClient.login("jebaramo@gmail.com", "6MHAk3Nmy!tL7NQ");
        resume();
    });

    QObject::connect(&_stompClient, &QStompClient::frameReceived, this, [this] {

        QStompResponseFrame frame = _stompClient.fetchFrame();
        QByteArray str = frame.body().toLocal8Bit();
        QJsonDocument doc = QJsonDocument::fromJson(str);

        if( !doc.isNull())
            onStompNetworkRail(doc);
    });

    QObject::connect(&_stompClient, &QStompClient::socketDisconnected,  this, [] {
        qDebug() << "QStompClient::socketDisconnected";
    });

    QObject::connect(&_stompClient, &QStompClient::socketError, this, [](QAbstractSocket::SocketError err) {
        qDebug() << "QStompClient::socketError :" << err;
    });

    _networkRailStnCSV.Load( ":/data/NetworkRail/network_rail_stns.txt", 3);

    loadScheduleDataForToday();

    QTimer *scheduleTimer = new QTimer(this);
    connect(scheduleTimer, &QTimer::timeout, this, &NationalRailPositionProvider::loadScheduleDataForToday);
    scheduleTimer->start(60000);

    QTimer *heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &NationalRailPositionProvider::sendHeartBeat);
    heartbeatTimer->start(5000);
}

NationalRailPositionProvider::~NationalRailPositionProvider()
{
    _future.waitForFinished();
}

void NationalRailPositionProvider::resume()
{
    for(const auto& id : std::as_const(_ids))
        subscribeTopic(_stompClient, getLineIdToLineCodeMap()[id]);
}

void NationalRailPositionProvider::pause()
{
    for(const auto& id : std::as_const(_ids))
        unsubcribeTopic(_stompClient, getLineIdToLineCodeMap()[id]);
}

void NationalRailPositionProvider::fetch(const QSet<QString> &lines)
{
    if( lines.isEmpty())
        disconnectStomp(_stompClient);
    else
        connectStomp(_stompClient);

    for(const auto &id : lines)
    {
        if( _ids.find(id) == _ids.end())
            subscribeTopic(_stompClient, getLineIdToLineCodeMap()[id]);
    }

    // old current - lines is what is left over -> iterate over these and remove
    const QSet<QString> oldLines = _ids.subtract(lines);

    for( const auto &id : oldLines)
        unsubcribeTopic(_stompClient, getLineIdToLineCodeMap()[id]);

    _ids = lines;
}

void NationalRailPositionProvider::onStompNetworkRail(const QJsonDocument &doc)
{
    QJsonArray a = doc.array();

    for(const QJsonValue &item : std::as_const(a))
    {
        QJsonObject obj = item.toObject();
        QJsonObject header = obj["header"].toObject();
        QJsonObject body = obj["body"].toObject();

        if( header["msg_type"].toString().toInt() != 3)
            continue;

        //if( body["trained_terminated"].toString().compare("true") == 0)
          //  continue;

        const QString lineCode = getTocIdToLineCodeMap()[body["toc_id"].toString()];

        const auto& stompMap = getLineIdToLineCodeMap();

        auto it = std::find_if(stompMap.constKeyValueBegin(), stompMap.constKeyValueEnd(),
                               [lineCode](const QPair<QString,QString> & pair) {
                                   return pair.second == lineCode;
                               });

        if(it == stompMap.constKeyValueEnd())
            continue;

        const QString nextStanox = body["next_report_stanox"].toString();

        QString originName(QLatin1String("<waiting for info>"));
        QString destinationName(QLatin1String("<waiting for info>"));

        if(_networkRailScheduleJSON)
        {
            const QString toc_id = body["toc_id"].toString();
            const QString serviceCode = body["train_service_code"].toString();

            QDateTime plannedTime = QDateTime::fromMSecsSinceEpoch(body["planned_timestamp"].toVariant().toLongLong());

            QDateTime localDateTime = plannedTime.toLocalTime();

            QString serviceStanox;

            const std::shared_ptr<NRScheduleDATA> schedule = _networkRailScheduleJSON->getDestination(toc_id,
                                                                             serviceCode,
                                                                             body["loc_stanox"].toString(),
                                                                             body["event_type"].toString(),
                                                                             localDateTime.time());

            if( schedule)
            {
                originName = _networkRailStnCSV[schedule->originStanox].location;
                destinationName = _networkRailStnCSV[schedule->destinationStanox].location;
            }
            else
            {
                originName = QLatin1String("<unavailable>");
                destinationName = QLatin1String("<unavailable>");
            }
        }

        Train train;
        train.lineId = it->first;
        train.vehicleId = body["train_id"].toString();
        train.serviceCode = body["train_service_code"].toString();
        train.platform = body["platform"].toString();
        train.nextStn = _networkRailStnCSV[body["next_report_stanox"].toString()].location;
        train.reportingStn = _networkRailStnCSV[body["reporting_stanox"].toString()].location;
        train.locStn = _networkRailStnCSV[body["loc_stanox"].toString()].location;
        train.direction = body["direction_ind"].toString() == QStringLiteral("UP") ? "inbound" : "outbound";
        train.timeTo = body["next_report_run_time"].toString().toInt();
        train.eventType = body["event_type"].toString();
        train.originName = originName;
        train.destinationName = destinationName;

        if(  train.nextStn.isEmpty())
        {
            train.nextStn = train.locStn;
        }

#ifdef Q_OS_WIN
        qDebug() << "NetworkRail:"
                 << train.lineId << ", "
                 << "FROM: "<< train.originName <<", "
                 << train.direction << ", "
                 << "TO: "<< train.destinationName << ", "
                 << train.locStn << ", "
                 << train.nextStn << ", "
                 << train.serviceCode << ", "
                 << train.vehicleId  << ", "
                 << train.eventType << ", "
                 << train.timeTo;

#endif

        emit onTrainData(train);
    }
}
