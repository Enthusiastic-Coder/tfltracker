#include "Line.h"
#include <QDebug>
#include <QTime>
LineType Line::_lineType;

const QString Line::inbound = "inbound";
const QString Line::outbound = "outbound";
const QString Line::hammersmithCityLine = "hammersmith-city";
const QString Line::circleLine = "circle";
const QString Line::piccadillyLine = "piccadilly";
const QString Line::centralLine = "central";
const QString Line::districtLine = "district";
const QString Line::jubileeLine = "jubilee";
const QString Line::northernLine = "northern";
const QString Line::bakerlooLine = "bakerloo";
const QString Line::victoriaLine = "victoria";
const QString Line::waterlooCityLine = "waterloo-city";
const QString Line::metropolitanLine = "metropolitan";
const QString Line::tramLine = "tram";

const QString Line::lionessLine = "lioness";
const QString Line::mildmayLine = "mildmay";
const QString Line::windrushLine = "windrush";
const QString Line::weaverLine = "weaver";
const QString Line::suffragetteLine = "suffragette";
const QString Line::libertyLine = "liberty";


const QString Line::elizabethLine = "elizabeth";
const QString Line::thamesLink = "thameslink";
const QString Line::dlrLine = "dlr";
const QString Line::london_cable_car = "london-cable-car";
const QString Line::mode_busLine = "bus";
const QString Line::mode_overground = "overground";
const QString Line::mode_elizabeth = "elizabeth-line";
const QString Line::mode_riverBus = "river-bus";
const QString Line::mode_riverTour = "river-tour";
const QString Line::mode_nationalRail = "national-rail";
const QString Line::mode_tube = "tube";
const QString Line::mode_tram = "tram";
const QString Line::mode_dlr = "dlr";
const QString Line::mode_cable_car = "cable-car";

bool Line::isDLR() const
{
    return _id == Line::dlrLine;
}

bool Line::isNorthern() const
{
    return _id == Line::northernLine;
}

const QString &Line::id() const
{
    return _id;
}

const QString &Line::name() const
{
    return _name;
}

const QString &Line::direction() const
{
    return _direction;
}

QColor Line::getColor() const
{
    return _color;
}

void Line::setShowStops(bool bShow)
{
    _showStops = bShow;
}

bool Line::getShowStops() const
{
    return _showStops;
}

void Line::setOffSet(int offset)
{
    _offset = offset;
}

bool Line::isOutbound() const
{
    return _direction == Line::outbound;
}

LineType::mode Line::getType() const
{
    return _lineType.getType(_mode);
}

int Line::getOffset() const
{
    return _offset;
}

LineType Line::getTypeInfo()
{
    return _lineType;
}

bool Line::isReadOnly() const
{
    return _readOnly;
}

bool Line::isTrain() const
{
    auto t = getType();
    return t == LineType::dlr
            || t == LineType::tram
            || t == LineType::tube
            || t == LineType::elizabeth
            || t == LineType::overground
            || t == LineType::national_rail;
}

bool Line::isRiverBus() const
{
    auto t = getType();
    return t == LineType::river_bus || t == LineType::river_tour;
}

bool Line::isBus() const
{
    auto t = getType();
    return t == LineType::bus || t == LineType::coach;
}

bool Line::isLondonOverground() const
{
    return getType() == LineType::overground;
}

bool Line::isElizabethRail() const
{
    return getType() == LineType::elizabeth;
}

bool Line::isNationalRail() const
{
    return getType() == LineType::national_rail;
}

bool Line::isOverground() const
{
    return getType() == LineType::overground;
}

bool Line::isVisible() const
{
    return _visible;
}

void Line::setVisible(bool bShow)
{
    _visible = bShow;
    updateBranches();
}

void Line::updateBranches()
{
    for(auto& branch : _branches)
        branch->build(_visible);
}

void Line::setColor(QColor color)
{
    _color = color;
}

std::shared_ptr<StopPoint> Line::getStopPoint(const QString& id) const
{
    auto it = _stopPoints.find(id);

    if( it == _stopPoints.end())
        return nullptr;

    return it->second;
}

std::vector<std::shared_ptr<const StopPoint>> Line::getStopPointSlowRetry(const QString& id) const
{
    if( id.isEmpty())
        return {};

    std::vector<std::shared_ptr<const StopPoint>> stopPoints = getStopPointSlow(id);

    if( stopPoints.empty() )
    {
        if( id.startsWith("Harrow on the Hill"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Harrow-on"));
        }
        else if( id.startsWith("South Ealing"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Ealing Common"));
        }
        else if( id.startsWith("Heathrow T1"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Heathrow Terminals 2"));
        }
        else if( id.startsWith("Heathrow T2 & 3"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Heathrow Terminals 2"));
        }
        else if( id.startsWith("Heathrow via T4"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Heathrow Terminal 4"));
        }
        else if( id.startsWith("Heathrow T4"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Heathrow Terminal 4"));
        }
        else if( id.startsWith("Heathrow T5"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Heathrow Terminal 5"));
        }
        else if( id.startsWith("Woodford Junction"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Woodford"));
        }
        else if( id.startsWith("St. John Wood"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("St. John's Wood"));
        }
        else if( id.startsWith("Earl’s Court"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Earl's Court"));

            if( stopPoints.empty())
                stopPoints = getStopPointSlow(QStringLiteral("Earls Court"));
        }
        else if( id.startsWith("Paddington (H&C Line)"))
        {
            stopPoints = getStopPointSlow(QStringLiteral("Paddington (H&C Line)"));
        }
    }

    if( stopPoints.empty())
    {
        QStringList stnParts = id.split(QChar(' '), Qt::SkipEmptyParts);
        for(int i = stnParts.size()-1; i >0; --i)
        {
            QString shortedStn = stnParts.mid(0, i).join(' ');

            stopPoints = getStopPointSlow(shortedStn);
            if( !stopPoints.empty())
            {
                break;
            }
        }

    }

#ifdef Q_OS_WIN
    if( stopPoints.empty())
    {
        qDebug() << QTime::currentTime().toString();
        qDebug() << "Attempt at : " << id << "(" << name()<< "): NOT FOUND!";
    }
#endif

   return stopPoints;
}

std::vector<std::shared_ptr<const StopPoint>> Line::getStopPointSlow(const QString& id) const
{
    std::vector<std::shared_ptr<const StopPoint>> stopPoints;

    for(auto it = _stopPointNames.begin(); it != _stopPointNames.end(); ++it)
    {
        if( it->first.contains(id, Qt::CaseInsensitive) || id.contains(it->first, Qt::CaseInsensitive))
        {
            auto itStopPoint = _stopPoints.find(it->second);

            if( itStopPoint != _stopPoints.end())
                stopPoints.push_back(itStopPoint->second);
        }
    }

    return stopPoints;
}

const Branches &Line::getBranches() const
{
    return _branches;
}

std::shared_ptr<const Branch> Line::getBranch(int idx) const
{
    if( idx <0 || idx >= _branches.size())
        return nullptr;

    return _branches[idx];
}

const std::unordered_map<QString, std::shared_ptr<StopPoint>>& Line::getStopPoints() const
{
    return _stopPoints;
}

int Line::getStopPointDiff(const QString &idFrom, const QString &idTo) const
{
    std::vector<std::shared_ptr<const StopPoint>> ptsFrom = getStopPointSlowRetry(idFrom);
    if( ptsFrom.empty())
        return 0;

    std::vector<std::shared_ptr<const StopPoint>> ptsTo = getStopPointSlowRetry(idTo);
    if( ptsTo.empty())
        return 0;

    for (size_t x = 0; x < ptsFrom.size(); ++x)
    {
        for(size_t y = 0; y < ptsTo.size(); ++y)
        {
            auto ptFrom = ptsFrom[x];
            auto ptTo = ptsTo[y];

            for( const auto& route : _orderedRoutes)
            {
                int fromIdx = -1;
                int toIdx = -1;

                const size_t routeSize = route.size();
                for( size_t i = 0; i < routeSize; ++i)
                {
                    if(ptFrom->id == route[i])
                    {
                        fromIdx = i;
                    }
                    if(ptTo->id == route[i])
                    {
                        toIdx = i;
                    }

                    if( fromIdx != -1 && toIdx != -1)
                        return toIdx - fromIdx;
                }
            }
        }
    }

    return 0;
}
