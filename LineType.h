#ifndef LINETYPE_H
#define LINETYPE_H

#include <QObject>
#include <QString>
#include <map>


class LineType
{
    Q_GADGET
public:
    LineType();

    enum mode {
        unknown,
        bus,
        coach,
        cycle,
        cycle_hire,
        dlr,
        overground,
        cable_car,
        replacement_bus,
        river_bus,
        river_tour,
        taxi,
        elizabeth,
        national_rail,
        tram,
        tube
    };
    Q_ENUM(mode)

    mode getType(const QString& type) const;

private:
    std::map<QString,mode> _typeMap;
};

#endif // LINETYPE_H
