#include "LineType.h"

LineType::LineType()
{
    _typeMap["bus"] = mode::bus;
    _typeMap["coach"] = mode::coach;
    _typeMap["cycle"] = mode::cycle;
    _typeMap["cycle-hire"] = mode::cycle_hire;
    _typeMap["dlr"] = mode::dlr;
    _typeMap["cable-car"] = mode::cable_car;
    _typeMap["overground"] = mode::overground;
    _typeMap["replacement_bus"] = mode::replacement_bus;
    _typeMap["river-bus"] = mode::river_bus;
    _typeMap["river-tour"] = mode::river_tour;
    _typeMap["taxi"] = mode::taxi;
    _typeMap["elizabeth-line"] = mode::elizabeth;
    _typeMap["tram"] = mode::tram;
    _typeMap["tube"] = mode::tube;
    _typeMap["national-rail"] = mode::national_rail;
}

LineType::mode LineType::getType(const QString &type) const
{
    auto it = _typeMap.find(type);
    if( it == _typeMap.end())
        return mode::unknown;

    return it->second;
}
