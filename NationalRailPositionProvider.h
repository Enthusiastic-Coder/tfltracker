#ifndef NATIONALRAILPOSITIONPROVIDER_H
#define NATIONALRAILPOSITIONPROVIDER_H

#include <QObject>
#include <QSet>
#include <QFuture>

#include <qstomp.h>
#include "NetworkRailStnsCSV.h"
#include "NetworkRailScheduleJSON.h"

class NationalRailPositionProvider : public QObject
{
    Q_OBJECT
public:
    explicit NationalRailPositionProvider(QObject *parent = nullptr);
    ~NationalRailPositionProvider();

    struct Train{
        QString lineId;
        QString vehicleId;
        QString serviceCode;
        QString platform;
        QString nextStn;
        QString reportingStn;
        QString locStn;
        QString direction;
        QString eventType;
        QString originName;
        QString destinationName;
        int timeTo;
    };

    void resume();
    void pause();
    void fetch(const QSet<QString>& lines);

signals:
    void onTrainData(NationalRailPositionProvider::Train t);

private slots:
    void loadScheduleDataForToday();
    void sendHeartBeat();

private:
    void onStompNetworkRail(const QJsonDocument &doc);

    const QHash<QString, QString>& getLineIdToLineCodeMap() const;
    const QHash<QString, QString>& getTocIdToLineCodeMap() const;

private:
    QFuture<std::shared_ptr<NetworkRailScheduleJSON>> _future;

    QDate _lastCheckedDate;
    QStompClient _stompClient;
    bool _started = true;
    NetworkRailStnsCSV _networkRailStnCSV;
    std::shared_ptr<NetworkRailScheduleJSON> _networkRailScheduleJSON;
    QSet<QString> _ids;
};

#endif // NATIONALRAILPOSITIONPROVIDER_H
