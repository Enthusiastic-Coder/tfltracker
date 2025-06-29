#ifndef TFLMODEL_H
#define TFLMODEL_H

#include <jibbs/gps/GPSLocation.h>
#include <jibbs/boundary/BoundingBox.h>
#include <vector>
#include <QString>
#include <QJsonDocument>
#include <QColor>
#include <QObject>

#include "BranchConnect.h"

class StopPoint : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id MEMBER id CONSTANT)
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QString displayName MEMBER displayName CONSTANT)

public:
    bool isPassPoint;
    int order;
    QString id;
    QString name;
    QString mode;
    QString displayName;
    QString stopLetter;
    QString towards;
    GPSLocation position;
    float heading = 0.0f;
    float bearing = 0.0f;
    bool instantTurn;
    bool    visible;
    mutable int     timeTo = -1;
    std::vector<BranchConnect> attachedBranches;
};

#endif // TFLMODEL_H
