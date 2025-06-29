#ifndef NETWORKRAILSCHEDULEJSON_H
#define NETWORKRAILSCHEDULEJSON_H

#include <QString>
#include <QTime>
#include <QDate>
#include <QBitArray>
#include <QList>
#include <QMultiHash>
#include <QHash>


struct NRScheduleTimesDATA
{
    QTime arrival;
    QTime departure;
    QTime pass;
};

struct NRScheduleDATA {
    QString toc_id;
    QString serviceCode;
    QDate startDate;
    QDate endDate;
    QBitArray daysRun;
    QString originStanox;
    QString destinationStanox;

    QHash<QString, NRScheduleTimesDATA> stations;
};

class NetworkRailScheduleJSON
{
public:
    void loadFromJson(const QJsonDocument &doc);
    const std::shared_ptr<NRScheduleDATA> getDestination(const QString& toc_id, const QString& serviceCode, const QString& stanox, const QString &eventType, const QTime& now) const;

private:
    QMultiHash<QString,std::shared_ptr<NRScheduleDATA>> _services;
};

#endif // NETWORKRAILSCHEDULEJSON_H
