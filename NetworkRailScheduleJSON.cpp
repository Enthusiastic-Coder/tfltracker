#include "NetworkRailScheduleJSON.h"

#include <QFile>
#include <QJsonDocument>
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>


const std::shared_ptr<NRScheduleDATA> NetworkRailScheduleJSON::getDestination(const QString &toc_id, const QString &serviceCode, const QString &stanox, const QString &eventType, const QTime &now) const
{
    const auto& servicesAvailable = _services.values(toc_id +"|"+ serviceCode);

    for(const auto& service : servicesAvailable)
    {
        const NRScheduleTimesDATA& times = service->stations.value(stanox);

        QTime scheduleTime;

        if (eventType == "ARRIVAL")
        {
            scheduleTime = times.arrival;
        }
        else if (eventType == "DEPARTURE")
        {
            scheduleTime = times.departure;
        }

        if( !scheduleTime.isValid())
        {
            scheduleTime = times.pass;
        }

        if( !scheduleTime.isValid())
        {
            continue;
        }

        if( qAbs(now.secsTo(scheduleTime)) <=30 )
        {
            return service;
        }
    }

    return {};
}

void NetworkRailScheduleJSON::loadFromJson(const QJsonDocument &doc)
{
    QDate today = QDate::currentDate();

    QJsonArray services = doc.array();

    for(const auto& service: std::as_const(services))
    {
        QJsonObject serviceObj = service.toObject();

        QDate startDate = QDate::fromString(serviceObj["startDate"].toString(), "yyyy-MM-dd");
        QDate endDate = QDate::fromString(serviceObj["endDate"].toString(), "yyyy-MM-dd");

        if( today < startDate || today > endDate)
        {
            continue;
        }

        const QString daysRun = serviceObj["daysRun"].toString();

        QBitArray bitArray(7);

        for (int i = 0; i < 7; ++i)
        {
            bitArray.setBit(i, daysRun.at(i) == '1');
        }

        if( !bitArray.testBit(today.dayOfWeek()-1))
        {
            continue;
        }

        QString toc_id = serviceObj["toc_id"].toString();
        QString serviceCode = serviceObj["serviceCode"].toString();

        QJsonArray stns = serviceObj["stns"].toArray();

        std::shared_ptr<NRScheduleDATA> serviceDATA = std::make_shared<NRScheduleDATA>();
        serviceDATA->serviceCode = serviceCode;
        serviceDATA->toc_id	 = toc_id;
        serviceDATA->startDate = startDate;
        serviceDATA->endDate = endDate;
        serviceDATA->originStanox = stns.first()["stanox"].toString();
        serviceDATA->destinationStanox = stns.last()["stanox"].toString();

        for(const auto&stn : std::as_const(stns))
        {
            QJsonObject pointObj = stn.toObject();

            NRScheduleTimesDATA& times = serviceDATA->stations[pointObj["stanox"].toString()];

            times.arrival = QTime::fromString(pointObj["arrivalTime"].toString(), "hhmm");
            times.departure = QTime::fromString(pointObj["departureTime"].toString(), "hhmm");
            times.pass = QTime::fromString(pointObj["passTime"].toString(), "hhmm");
        }

        _services.insert(toc_id + "|" + serviceCode, serviceDATA);
    }

    qDebug() << "Network Rail Schedules loaded today --->" << _services.count();
}
