#ifndef LINEBUILDER_H
#define LINEBUILDER_H

#include <QJsonDocument>
#include <QColor>
#include <QJsonObject>
#include <QJsonArray>

#include "Line.h"

class LineBuilder
{
public:
    std::shared_ptr<Line> buildLine(const QJsonDocument& document);
};

#endif // LINEBUILDER_H
