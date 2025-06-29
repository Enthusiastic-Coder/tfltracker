#include "LineBuilder.h"
#include "helpers.h"
#include <QVector>
#include "Branch.h"
#include "Line.h"

std::shared_ptr<Line> LineBuilder::buildLine(const QJsonDocument &document)
{
    std::shared_ptr<Line> line = std::make_shared<Line>();

    QJsonObject rootObject = document.object();

    line->_id = rootObject[QStringLiteral("lineId")].toString();
    line->_name = rootObject[QStringLiteral("lineName")].toString();
    line->_mode = rootObject[QStringLiteral("mode")].toString();
    line->_direction = rootObject[QStringLiteral("direction")].toString();

    QJsonArray sequencesArray = rootObject[QStringLiteral("stopPointSequences")].toArray();

    int passpointcount= 0;

    for(const QJsonValue& obj: std::as_const(sequencesArray))
    {
        std::shared_ptr<Branch> branch = std::make_shared<Branch>();

        branch->parent = line;

        branch->_offset = obj[QStringLiteral("offSet")].toInt();

        QJsonArray stopPointsArray = obj[QStringLiteral("stopPoint")].toArray();

        std::vector<QString> pureStnPoints;

        GPSLocation prevPos;
        float prevBearing(0.0f);
        std::shared_ptr<StopPoint> prevStopPoint;
        int order(0);

        for(const QJsonValue &obj: std::as_const(stopPointsArray))
        {
            std::shared_ptr<StopPoint> stopPoint = std::make_shared<StopPoint>();

            stopPoint->order = order++;
            stopPoint->mode = line->_mode;
            stopPoint->instantTurn = obj[QStringLiteral("instantTurn")].toBool();

            stopPoint->isPassPoint = obj[QStringLiteral("isPassPoint")].toBool();

            QJsonObject oobj = obj.toObject();

            stopPoint->position._lat = obj[QStringLiteral("lat")].toDouble();
            stopPoint->position._lng = obj[QStringLiteral("lon")].toDouble();
            stopPoint->visible = obj[QStringLiteral("visible")].toBool(true);

            if( stopPoint->position._lat == 0.0)
            {
                stopPoint->position._lat = prevPos._lat;
                stopPoint->position._lng = prevPos._lng;
            }

            if( stopPoint->isPassPoint )
            {
                QJsonValue junctionObj = obj[QStringLiteral("junction")];

                if( junctionObj.isString())
                {
                    stopPoint->id = junctionObj.toString();
                    pureStnPoints.push_back(stopPoint->id);
                }
                else
                {
                    stopPoint->id = QString("{%1/%2/%3}").
                                    arg(stopPoint->position._lat).
                                    arg(stopPoint->position._lng).
                                    arg(passpointcount++);
                }

                stopPoint->name = stopPoint->displayName = stopPoint->id;
            }
            else
            {
                stopPoint->id = obj[QStringLiteral("id")].toString();

                pureStnPoints.push_back(stopPoint->id);

                stopPoint->towards = obj[QStringLiteral("towards")].toString();

                QString name = obj[QStringLiteral("name")].toString();
                stopPoint->name = name;

                if( !line->isBus())
                    cleanStationName(name);

                stopPoint->displayName = name;

                QJsonValue stopPointValue = obj[QStringLiteral("stopLetter")];

                if( stopPointValue.isString())
                {
                    stopPoint->stopLetter = stopPointValue.toString();
                    stopPoint->stopLetter.remove("->");
                }
            }

            if( prevStopPoint!= nullptr)
            {
                prevStopPoint->bearing = prevPos.bearingTo(stopPoint->position);
                prevBearing = prevStopPoint->bearing;
            }

            prevPos._lat = stopPoint->position._lat;
            prevPos._lng = stopPoint->position._lng;
            prevStopPoint = stopPoint;

            if(stopPoint->id.isEmpty())
                stopPoint->id = stopPoint->name;

            branch->_ids.push_back(stopPoint->id);
            branch->_stnPositions.push_back(stopPoint->position);

            line->_stopPointNames[stopPoint->name+stopPoint->id] = stopPoint->id;

            line->_stopPoints[stopPoint->id] = stopPoint;
        }

        if( prevStopPoint !=nullptr)
            prevStopPoint->bearing = prevBearing-180.0f;

        branch->_id = (int)line->_branches.size();
        line->_branches.push_back(branch);

        line->_orderedRoutes.push_back(pureStnPoints);
    }

    auto type = line->getType();

    line->_readOnly = type == LineType::tram
                    || type == LineType::tube
                    || type == LineType::elizabeth
                    || type == LineType::dlr
                    || type == LineType::overground
                      || type == LineType::cable_car;

    return line;
}

