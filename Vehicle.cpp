#include <jibbs/math/qhdgtable.h>

#include <QDebug>
#include <QTime>
#include <QStringLiteral>

#include "Vehicle.h"
#include "Line.h"

struct PossibleAttactment {
    Branch* branch = nullptr;
    float index = 0;
};

QString Vehicle::toString() const
{
    return QString("Key:%1 | Line:%2 | Loc:%3 | Stn:%4 | Dest:%5")
            .arg(_key)
            .arg(_lineId)
            .arg(_currentLocation)
            .arg(_displayStationName)
            .arg(_destinationName);
}

void Vehicle::qqDebug(QString title) const
{
#ifdef Q_OS_WIN
    qDebug() << QTime::currentTime().toString();
    qDebug() << title;
    qDebug() << "KEY : " << _key;
    qDebug() << "LineID : "<< _lineId;
    qDebug() << "VehID : " << _vehicleId;
    qDebug() << "Stn Name : " << _displayStationName;
    qDebug() << "DestN :"  << _destinationName;
    qDebug() << "CurrLoc : " << _currentLocation;
    qDebug() << "NextLoc : " << _nextLocation;
    qDebug() << "PreVLoc : " << _previousLocation;
    qDebug() << "Towards : " << _towards;
    qDebug() << "Platform : " << _platform;
    qDebug() << "Behaviour: "<< _behaviour;
    qDebug() << "Direction: "<< _direction;
    qDebug() << "TimeLeft : " << _timeToStation;
    qDebug() << "==============================";
#endif
}

void Vehicle::updatePosAndDir()
{   
    updateBehaviourPosition();
    updateIfUpminsterOnHighStKenBranch();
    updateHammtoCircleIfNeeded();
    updateDirectionIfNeeded();
    updateDirectionOverride();
    updateDirectionIfBlank();
    updateNextStopIfNeeded();
    updateDisplayTowards();
}

void Vehicle::updateIfUpminsterOnHighStKenBranch()
{
    if( isDistrict() )
        if( _towards == QStringLiteral("Upminster"))
            if( _nextLocation == QStringLiteral("High Street Kensington"))
                _nextLocation = QStringLiteral("Gloucester Road");
}

void Vehicle::updateHammtoCircleIfNeeded()
{
    if( !isHammersmithyCity())
        return;

    if( _towards.startsWith(QStringLiteral("Edgware Road (Circle)")))
        _lineId = Line::circleLine;

    if( isCircle())
    {
        _wantToBeCircle = true;
        return;
    }

    if( !_nextLocation.isEmpty() && isHammersmithyCity() )
    {
        static std::vector<QString> stns = {
            {"South Kensington"},
            {"Sloane Square"},
            {"Victoria"},
            {"St. James's Park"},
            {"Westminster"},
            {"Embankment"},
            {"Temple"},
            {"Blackfriars"},
            {"Mansion House"},
            {"Cannon Street"},
            {"Monument"},
            {"Tower Hill"},
            {"Aldgate"},
        };

       int idx =-1;
       for( size_t i=0; i < stns.size(); ++i)
           if( stns[i].startsWith(_nextLocation) || (!_previousLocation.isEmpty() && stns[i].startsWith(_previousLocation)))
           {
               idx = i;
               break;
           }

       if( idx !=-1)
       {
           _lineId = Line::circleLine;
           _direction = Line::inbound;
       }
    }

    if( !_nextLocation.isEmpty() && isHammersmithyCity() )
    {
        static std::vector<QString> stns = {
            {"Notting Hill Gate"},
            {"Bayswater"},
            {"High Street Kensington"},
        };

       int idx =-1;
       for( size_t i=0; i < stns.size(); ++i)
           if( stns[i].startsWith(_nextLocation)|| (!_previousLocation.isEmpty() && stns[i].startsWith(_previousLocation)))
           {
               idx = i;
               break;
           }

       if( idx !=-1)
       {
           _lineId = Line::circleLine;
           _direction = Line::outbound;
           if( _nextLocation.startsWith(QStringLiteral("Paddington")))
               _nextLocation = QStringLiteral("Paddington Un");
       }
    }
}

void Vehicle::updateBehaviourPosition()
{
    if( _currentLocation.isEmpty())
    {
        _behaviour = Unknown;
        _useVehicleBehaviour = false;
        return;
    }

    if( _currentLocation.startsWith("At Platform"))
        _currentLocation = QStringLiteral("At ") + _stationName;

    if( _currentLocation.startsWith("Approaching"))
    {
        _nextLocation = _currentLocation.mid(QString("Approaching").length()+1);
        _behaviour = Approaching;
    }
    else if( _currentLocation.startsWith(QLatin1String("At Leaving")))
    {
        _nextLocation = _currentLocation.mid(QString("At Leaving").length()+1);
        _behaviour = Departed;
    }
    else if( _currentLocation.startsWith(QLatin1String("At")))
    {
        _nextLocation = _currentLocation.mid(3);
        _behaviour = At;
    }
    else if( _currentLocation.startsWith(QLatin1String("Left")))
    {
        _nextLocation = _currentLocation.mid(QString("Left").length()+1);
        _behaviour = Departed;
    }
    else if( _currentLocation.startsWith(QLatin1String("Leaving")))
    {
        _nextLocation = _currentLocation.mid(QString("Leaving").length()+1);
        _behaviour = Departed;
    }
    else if( _currentLocation.startsWith(QLatin1String("Departed")))
    {
        _nextLocation = _currentLocation.mid(QString("Departed").length()+1);
        _behaviour = Departed;
    }
    else if( _currentLocation.startsWith(QLatin1String("Between")) || _currentLocation.startsWith(QLatin1String("In between")))
    {
        const bool isElephant = _currentLocation.startsWith(QStringLiteral("Between Elephant"));

        int idx = isElephant ? _currentLocation.lastIndexOf(QLatin1String(" and")) : _currentLocation.indexOf(QLatin1String(" and"));

        if( _currentLocation[0] == QChar('I'))
            _previousLocation = _currentLocation.mid(11, idx-11);
        else
            _previousLocation = _currentLocation.mid(8, idx-8);

        _nextLocation = _currentLocation.mid(idx+5);
        _behaviour = Between;
    }
    else if( _currentLocation.startsWith(QLatin1String("Near")))
    {
        _nextLocation = _currentLocation.mid(QString("Near").length()+1);
        _behaviour = Approaching;
    }
    else if( _currentLocation.startsWith(QStringLiteral("Queen's Park North Sidings"))
             || _currentLocation.startsWith(QStringLiteral("Stonebridge Park Depot"))
             || _currentLocation.startsWith(QStringLiteral("Harrow & Wealdstone Siding")))
    {
        _nextLocation = _currentLocation;
        _stationName = _currentLocation;
        _behaviour = Between;
    }
    else if( _currentLocation.startsWith(QStringLiteral("North of Queen's Park"))
             || _currentLocation.startsWith(QStringLiteral("South of Queen's Park")))
    {
        _nextLocation = "Queen's Park";
        _behaviour = Between;
    }

    {
        int idx = _nextLocation.lastIndexOf(QLatin1String(" Platform"));
        if( idx != -1)
            _nextLocation = _nextLocation.left(idx);
    }

    {
        int idx = _nextLocation.lastIndexOf(QLatin1String(" fast"));
        if( idx != -1)
        {
            _fast = true;
            _nextLocation = _nextLocation.left(idx);
        }
    }

    _nextLocation = _nextLocation.trimmed();
    _previousLocation = _previousLocation.trimmed();

    cleanUpLocation(_nextLocation);
    cleanUpLocation(_previousLocation);
    cleanUpLocation(_stationName);
}

void Vehicle::cleanUpLocation(QString &location)
{
    if( location.isEmpty())
        return;

    if( location.startsWith(QStringLiteral("Kings Cross")))
        location = QStringLiteral("King's Cross");

    else if( location == QStringLiteral("Elephant and Castle"))
        location = QStringLiteral("Elephant & Castle");

    else if( location.startsWith(QStringLiteral("Earls Court")))
        location = QStringLiteral("Earl's Court");

    else if( location.startsWith(QStringLiteral("Paddington (Suburban)")))
        location = QStringLiteral("Paddington (H&C");

    else if( location == QStringLiteral("Heathrow"))
        location = QStringLiteral("Heathrow Terminals 2 & 3");

    else if( location.startsWith(QStringLiteral("Heathrow Terminal 1")))
        location = QStringLiteral("Heathrow Terminals 2 & 3");

    else if( location.startsWith(QStringLiteral("Chalfont and Latimer")))
        location = QStringLiteral("Chalfont & Latimer");

    else if( location.startsWith(QStringLiteral("Shepherds Bush")))
        location = QStringLiteral("Shepherd's Bush");

    else if( location.startsWith(QStringLiteral("Regents Park")))
        location = QStringLiteral("Regent's Park");

    else if( location.startsWith(QStringLiteral("Angel")))
        location = QStringLiteral("Angel");

    else if( location.startsWith(QStringLiteral("White City")))
        location = QStringLiteral("White City");

    else if( location.startsWith(QStringLiteral("London Bridge")))
        location = QStringLiteral("London Bridge");

    else if( location.startsWith(QStringLiteral("East Finchley")))
        location = QStringLiteral("East Finchley");

    else if( location.startsWith(QStringLiteral("Turnham Green")))
        location = QStringLiteral("Turnham Green");

    else if( location.startsWith(QStringLiteral("Camden Town")))
        location = QStringLiteral("Camden Town");

    else if( location.startsWith(QStringLiteral("West Finchley")))
        location = QStringLiteral("West Finchley");

    else if( isPiccadilly() && location.startsWith(QStringLiteral("Ealing Broadway")))
        location = QStringLiteral("Ealing Common");

    else if( location.startsWith(QStringLiteral("Newbury Park Loop")))
        location = QStringLiteral("Newbury Park");

    else if( location.startsWith(QStringLiteral("North Acton Junction")))
        location = QStringLiteral("North Acton");

    else if( location.startsWith(QStringLiteral("Woodford")))
        location = QStringLiteral("Woodford");

    else if( location.startsWith(QStringLiteral("South Woodford")))
        location = QStringLiteral("South Woodford");

    else if( location.startsWith(QStringLiteral("Finchley Central")))
        location = QStringLiteral("Finchley Central");

    else if( location.startsWith(QStringLiteral("Wood Green Sidings")))
        location = QStringLiteral("Wood Green");

    else if( location.startsWith(QStringLiteral("Northolt Sidings")))
        location = QStringLiteral("Northolt");

    else if( location == QStringLiteral("Willlesden Green"))
        location = QStringLiteral("Willesden Green");

    if( isBakerloo())
    {
        if( location.startsWith(QStringLiteral("Paddington")))
            location = QStringLiteral("Paddington");
    }
}


namespace {
using LineMap = const std::map<QString, QString>;

    LineMap& getPiccadillyMap()
    {
        static LineMap line = {
        // { "Heathrow T2&3", Line::inbound },
            { "Cockfosters", Line::outbound },
            { "Arnos Grove", Line::outbound },
            { "Rayners Lane", Line::inbound },
            { "Uxbridge", Line::inbound },
            { "Northfields", Line::inbound },
            { "Oakwood", Line::outbound },
            { "Wood Green", Line::outbound },
        };
        return line;
    }

    LineMap& getCentralMap()
    {
        static LineMap line = {
               { "Ealing Broadway", Line::inbound },
               { "Loughton", Line::outbound },
     //        { "Woodford", Line::outbound },
               { "Epping", Line::outbound },
               { "Hainault", Line::outbound },
               { "West Ruislip", Line::inbound },
               { "Newbury Park", Line::outbound },
     //        { "White City", Line::inbound },
     //        { "North Acton", Line::inbound },
               { "Northolt", Line::inbound },
               { "Debden", Line::outbound },
     //        { "Ruislip Gardens", Line::inbound },
               { "Grange Hill", Line::outbound },
        };

        return line;
    }

    LineMap& getMetropolitanMap()
    {
        static LineMap line = {
            { "Aldgate", Line::outbound },
            { "Watford", Line::inbound },
            { "Uxbridge", Line::inbound },
            { "Amersham", Line::inbound },
            { "Chesham", Line::inbound },
            { "Chalfont", Line::inbound },
            { "Baker Street", Line::outbound },
        };

        return line;
    }

    LineMap& getJubileeMap()
    {

        static LineMap line = {
            { "Stratford", Line::outbound },
            { "North Greenwich", Line::outbound },
            { "Stanmore", Line::inbound },
            { "Willesden Green", Line::inbound },
            { "West Ham", Line::outbound },
        };

        return line;
    }

    LineMap& getCircleMap()
    {
        static LineMap line = {
            { "Edgware", Line::outbound },
            { "Hammersmith", Line::inbound },
            { "Circle Line", Line::outbound},
        };

        return line;
    }

    LineMap& getHammersmithMap()
    {
        static LineMap line = {
            { "Edgware", Line::outbound },
            { "Plaistow", Line::outbound },
            { "Barking", Line::outbound },
            { "Hammersmith", Line::inbound },
        };

        return line;
    }

    LineMap& getDistrictMap()
    {
        static LineMap line = {
            { "Edgware", Line::outbound },
            { "Barking", Line::outbound },
            { "Upminster", Line::outbound },
            { "Embankment", Line::outbound },
            { "Dagenham East", Line::outbound },
            { "Ealing B", Line::inbound },
            { "Richmond", Line::inbound },
            { "Wimbledon", Line::inbound },
            { "Olympia", Line::inbound },
            { "Tower H", Line::outbound },
            { "Ealing Common", Line::outbound}
        };

        return line;
    }

    LineMap& getBakerlooMap()
    {
        static LineMap line = {
            { "Queens", Line::outbound },
            { "Queen's", Line::outbound },
            { "Harrow", Line::outbound },
            { "Stonebridge", Line::outbound },
            { "Elephant", Line::inbound },
        };

        return line;
    }

    LineMap& getVictoriaMap()
    {
        static LineMap line = {
            { "Brixton", Line::inbound },
            { "Waltham", Line::outbound },
        };

        return line;
    }

    LineMap& getNorthernMap()
    {
        static LineMap line = {
            { "High Bar", Line::outbound },
            { "Kennin", Line::inbound },
            { "Edgware", Line::outbound},
            { "Mill H", Line::outbound},
            { "Morden", Line::inbound },
            { "Tooting Broadway", Line::inbound },
        };

        return line;
    }

    LineMap& getWaterlooCityMap()
    {
        static LineMap line = {
            { "Waterloo", Line::inbound },
            { "Bank", Line::outbound }
        };

        return line;
    }

    LineMap& getElizabathRailMap()
    {
        static LineMap line = {
            { "London Paddington", Line::inbound},
            { "Reading", Line::outbound },
            { "Shenfield", Line::inbound },
            { "London Liverpool Street", Line::outbound},

            { "Heathrow T4", Line::outbound},
            { "Heathrow T5", Line::outbound},
            { "Gidea Park", Line::inbound},
            { "Romford", Line::inbound},
            { "Heathrow T2 & 3", Line::outbound},

            { "Heathrow", Line::outbound},
            { "London Heathrow", Line::outbound},

            { "Shenfield", Line::inbound },
            { "Abbey Wood", Line::inbound},
            { "Paddington", Line::outbound},
        };

        return line;
    }

    LineMap& getOvergroundMap()
    {
        static LineMap line = {
            { "Watford Junction", Line::outbound },
            { "London Euston", Line::inbound},
            { "Clapham Junction", Line::outbound},
            { "Highbury & Islington", Line::inbound},
            { "Dalston Junction", Line::inbound},
            { "Crystal Palace", Line::outbound},
            { "Stratford (London)", Line::inbound},
            { "Chingford", Line::outbound},
            { "Cheshunt", Line::outbound},
            { "West Croydon", Line::outbound},
            { "Richmond (London)", Line::outbound},
            { "Battersea Park", Line::outbound},
            { "New Cross Gate", Line::outbound},
            { "Upminster", Line::outbound},
            { "Enfield Town", Line::outbound},
            { "Hackney Wick", Line::inbound},
            { "New Cross ELL",Line::outbound},
            { "Barking",Line::outbound},
            { "Shadwell",Line::inbound},
        };

        return line;
    }

    LineMap& getTramMap()
    {
        static LineMap line = {
            { "Wimbledon", Line::outbound},
            {"Elmers End", Line::inbound},
            {"East Croydon", Line::outbound},
        };

        return line;
    }

    LineMap& getDLRMap()
    {
        static LineMap line = {
    //        { "West Ham", Line::outbound},
    //        { "Beckton", Line::outbound},
            { "Gallions Reach", Line::outbound},
        };

        return line;
    }

    LineMap& getThamesLinkMap()
    {
        static LineMap line = {
            { "Brighton", Line::outbound},
            { "Rainham", Line::outbound},
            { "Sutton", Line::outbound},
            { "Horsham", Line::outbound},
            { "London King's Cross", Line::outbound},
            { "Cambridge", Line::inbound},
            { "Peterborough", Line::inbound},
            { "Bedford", Line::inbound},
            { "Littlehampton", Line::outbound},
            { "East Grinstead", Line::outbound},
            { "Luton", Line::inbound},
            { "Royston", Line::inbound},
            { "Sevenoaks", Line::outbound},
            { "Orpington", Line::outbound},
            { "London Victoria", Line::inbound},
            { "Gatwick Airport", Line::outbound},
        };

        return line;
    }

    // Master map lazy initialization
    const std::map<QString, std::map<QString, QString>>& getLineMap()
    {
        static const std::map<QString, std::map<QString,QString>> lineMap =
        {
            { Line::piccadillyLine,     getPiccadillyMap() },
            { Line::centralLine,        getCentralMap() },
            { Line::metropolitanLine,   getMetropolitanMap() },
            { Line::jubileeLine,        getJubileeMap() },
            { Line::circleLine,         getCircleMap() },
            { Line::hammersmithCityLine, getHammersmithMap() },
            { Line::districtLine,       getDistrictMap() },
            { Line::bakerlooLine,       getBakerlooMap() },
            { Line::victoriaLine,       getVictoriaMap() },
            { Line::northernLine,       getNorthernMap() },
            { Line::waterlooCityLine,   getWaterlooCityMap() },
            { Line::elizabethLine,        getElizabathRailMap() },
            { Line::lionessLine,        getOvergroundMap()},
            { Line::mildmayLine,        getOvergroundMap()},
            { Line::windrushLine,       getOvergroundMap()},
            { Line::weaverLine,         getOvergroundMap()},
            { Line::suffragetteLine,     getOvergroundMap()},
            { Line::libertyLine,        getOvergroundMap()},
            { Line::tramLine,           getTramMap()},
            { Line::dlrLine,            getDLRMap()},
            { Line::thamesLink,         getThamesLinkMap()},
         };

        return lineMap;
    }
}

void Vehicle::updateDirectionIfBlank()
{
    if( !_direction.isEmpty())
        return;

    if( isHammersmithyCity() )
    {
        if( _stationName.startsWith(QStringLiteral("Edgware Road (Cir")))
            _direction = Line::outbound;
    }

    if( _towards.startsWith(QStringLiteral("Check Front")))
    {
        if(_platform.startsWith(QLatin1String("Eastbound")))
            _direction = Line::outbound;

        else if(_platform.startsWith(QLatin1String("Westbound")))
            _direction = Line::inbound;

        else if(_platform.startsWith(QLatin1String("Northbound")))
            _direction = Line::inbound;

        else if(_platform.startsWith(QLatin1String("Southbound")))
            _direction = Line::outbound;
    }
}

void Vehicle::updateDirectionOverride()
{
    const auto& lineMap = getLineMap();
    auto it = getLineMap().find(_lineId);
    if( it != lineMap.end())
    {
        for( const auto& destn : it->second)
            if( _towards.isEmpty())
            {
                if( _destinationName.startsWith(destn.first, Qt::CaseInsensitive))
                    _direction = destn.second;
            }
            else
            {
                if( _towards.startsWith(destn.first, Qt::CaseInsensitive))
                    _direction = destn.second;
            }
    }
}

void Vehicle::updateNextStopIfNeeded()
{
    if( isPiccadilly())
    {
        if( _stationName.startsWith(QStringLiteral("South Ealing")))
            if(_platform == QStringLiteral("Westbound - Platform 4"))
                _stationName = QStringLiteral("South Ealing Platform 4");

        if( _stationName.startsWith(QStringLiteral("Northfields")))
            if( _platform == QStringLiteral("Westbound - Platform 1"))
                _stationName = QStringLiteral("Northfields Platform 1");

        if( _currentLocation.startsWith("South Ealing Platform 4"))
            _stationName = QStringLiteral("South Ealing Platform 4");
    }

    if( isMetropolitan())
    {
        if( _stationName == QStringLiteral("Harrow-on-the-Hill"))
            if( _platform == QStringLiteral("Northbound Main - Platform 1"))
                _stationName = QStringLiteral("Harrow-on-the-Hill Platform 1");
    }

    if( isDLR())
    {
        if( _stationName.startsWith(QStringLiteral("Stratford")))
            if( _platform == QStringLiteral("Platform 17"))
                _stationName = QStringLiteral("Stratford Platform 17");
    }
}

void Vehicle::updateDirectionIfNeeded()
{
    if( isCentral() )
    {
        if( _towards == QStringLiteral("Woodford") && _nextLocation == QStringLiteral("Roding Valley"))
            _direction = Line::inbound;

        if( _stationName == QStringLiteral("White City"))
            if( _platform == QStringLiteral("Eastbound - Platform 4"))
             _direction = Line::outbound;

        if( _stationName == QStringLiteral("Woodford"))
            if( _platform == QStringLiteral("Eastbound - Platform 3"))
             _direction = Line::outbound;

        if( _towards == QStringLiteral("Woodford Via Hainault"))
            _direction = Line::outbound;
}
    else if( isLondonOverground())
    {


        if( _stationName == QStringLiteral("South Acton"))
            if( _platform == QStringLiteral("Platform 1"))
                _direction = Line::outbound;
    }

    else if( isVictoria())
    {
        if( _stationName == QStringLiteral("Seven Sisters"))
            if( _platform == QStringLiteral("Northbound - Platform 4"))
             _direction = Line::outbound;

    }

    else if( isPiccadilly())
    {
        if( _nextLocation.startsWith(QStringLiteral("Heathrow Terminal 4")))
            _direction = Line::outbound;

        if( _nextLocation.startsWith(QStringLiteral("Hatton")) && _destinationName.startsWith(QStringLiteral("Heathrow")))
            _direction = Line::inbound;
    }

    else if( isDistrict() )
    {
        if( _nextLocation.startsWith(QStringLiteral("Edgware")))
            _direction = Line::outbound;
    }

    else if( isMetropolitan() )
    {
        if( _towards.startsWith(QStringLiteral("Check")))
        {
            if( _stationName.startsWith(QStringLiteral("Amersham")) && _nextLocation.isEmpty())
            {
                _nextLocation = "Amersham";
                _direction = Line::outbound;
                _behaviour = Vehicle::At;
            }
        }
    }
    else if( isDLR() )
     {
        struct platformEntry {QString stnName;QString Platform; QString dir;};

        static std::vector<platformEntry> platform_entries = {
            {"Stratford", "4a", Line::outbound},
            {"Stratford International", "Platform 1", Line::outbound},
            {"Stratford", "Platform 17", Line::outbound},
            {"Stratford High Street", "Platform 1", Line::outbound},
            {"Abbey Road", "Platform 1", Line::outbound},
            {"West Ham", "Platform 3", Line::outbound},
            {"Star Lane", "Platform 1", Line::outbound},
            {"Pudding Mill Lane", "Platform 2", Line::outbound},
            {"Bow Church", "Platform 2", Line::outbound},
            {"Devons Road", "Platform 2", Line::outbound},
            {"Langdon Park", "Platform 2", Line::outbound},
            {"Shadwell", "Platform 1", Line::outbound},
            {"Limehouse", "Platform 3", Line::outbound},
            {"Westferry", "Platform 1", Line::outbound},
            {"Bank", "Platform 9", Line::outbound},
            {"Gallions Reach", "Platform 2", Line::inbound},
            {"Cyprus", "Platform 2", Line::inbound},
            {"Beckton Park", "Platform 2", Line::inbound},
            {"Royal Albert", "Platform 2", Line::inbound},
            {"Prince Regent", "Platform 2", Line::inbound},
            {"Custom House", "Platform 4", Line::inbound},
            {"Royal Victoria", "Platform 2", Line::inbound},
            {"King George V", "Platform 2", Line::inbound},
            {"London City Airport", "Platform 2", Line::inbound},
            {"Pontoon Dock", "Platform 2", Line::inbound},
            {"West Silvertown", "Platform 2", Line::inbound},
            {"Elverson Road", "Platform 2", Line::inbound},
            {"Deptford Bridge", "Platform 2", Line::inbound},
            {"Greenwich", "Platform 3", Line::inbound},
            {"Cutty Sark", "Platform 2", Line::inbound},
            {"Island Gardens", "Platform 2", Line::inbound},
            {"Mudchute", "Platform 2", Line::inbound},
            {"Crossharbour", "Platform 2", Line::inbound},
            {"South Quay", "Platform 2", Line::inbound},
            {"Heron Quays", "Platform 2", Line::inbound},
        };

        struct destinationEntry {QString stnName;QString destination; QString dir;};

        static std::vector<destinationEntry> destination_entries = {
            {"Tower Gateway", "Lewisham", Line::outbound},
            {"Tower Gateway", "Woolwich", Line::outbound},
            {"Tower Gateway", "Beckton", Line::outbound},
        };

        for(auto& entry:platform_entries)
        {
            if( _stationName == entry.stnName )
                if( _platform == entry.Platform)
                    _direction = entry.dir;
        }

        for(auto& entry:destination_entries)
        {
            if( _stationName == entry.stnName)
                if( _destinationName == entry.destination)
                    _direction = entry.dir;
        }
     }
 }

void Vehicle::updatePreviousStopCache(QHash<QString,QString>& previousStopPoint )
{
    if(_previousLocation.isEmpty())
    {
        auto prevIt = previousStopPoint.find(_key);
        if( prevIt != previousStopPoint.end())
            _previousLocation = prevIt.value();
    }
    else
    {
        previousStopPoint[_key] = _previousLocation;
    }
}

std::vector<Branch *> Vehicle::buildBranchExcludeList(std::shared_ptr<const StopPoint> stopPoint)
{
    if( stopPoint->attachedBranches.size() < 2)
        return {};

    std::vector<Branch*> exclude;
    std::vector<int> avoidList;
    int wantID= -1;

    if( isNorthern() )
    {
//        const bool isViaCX = _towards.contains(QStringLiteral("via CX"));

//        if( _line->isOutbound())
//        {
//            if( _currentLocation.startsWith( QStringLiteral("At Camden"))
//                    && (_behaviour == CurrentBehaviour::At || _behaviour == CurrentBehaviour::Departed ))
//            {
//                if( _destinationName.startsWith("High"))
//                    wantID = 3;
//                else
//                    wantID = 7;
//            }
//            else
//            {
//                avoidList.push_back(isViaCX ? 1:6);

//                if( isViaCX && _nextLocation.startsWith(QStringLiteral("Kennington"))
//                        && (_behaviour == CurrentBehaviour::At || _behaviour == CurrentBehaviour::Departed) )
//                    avoidList.push_back(0);
//            }
//        }
//        else
//        {
            //1 Finchely to Camden Town.
            //2 Bank
            //4 Is Charing X

//            if( isViaCX )
//            {
//                avoidList.push_back(1);//All via CX avoid this upper line
//                avoidList.push_back(2);
//            }
//            else
//                avoidList.push_back(4);


            if( _stationName == QStringLiteral("Kennington"))
                if( _platform == QStringLiteral("Southbound - Platform 2"))
                {
                    avoidList.push_back(4);
                    avoidList.push_back(6);
                    avoidList.push_back(9);
                }

            if( _stationName == QStringLiteral("Euston"))
                if( _platform == QStringLiteral("Southbound - Platform 2"))
                {
                    avoidList.push_back(2);
                    avoidList.push_back(3);
                }

            if ( _towards == QStringLiteral("Morden via Bank"))
            {
                avoidList.push_back(5);
                avoidList.push_back(6);
            }

            if ( _towards == QStringLiteral("Morden via CX"))
            {
                avoidList.push_back(2);
                avoidList.push_back(3);
            }

            if ( _towards == QStringLiteral("Battersea via CX"))
            {
                avoidList.push_back(2);
                avoidList.push_back(3);
            }

            if ( _towards == QStringLiteral("Kennington via CX"))
            {
                avoidList.push_back(2);
                avoidList.push_back(3);
            }

            if ( _towards == QStringLiteral("High Barnet via Bank"))
            {
                avoidList.push_back(5);
                avoidList.push_back(6);
            }

            if ( _towards == QStringLiteral("High Barnet via CX"))
            {
                avoidList.push_back(1);
                avoidList.push_back(2);
            }

            if ( _towards == QStringLiteral("Mill Hill East via CX"))
            {
                avoidList.push_back(1);
                avoidList.push_back(2);
            }

            if ( _towards == QStringLiteral("Mill Hill East via Bank"))
            {
                avoidList.push_back(5);
                avoidList.push_back(6);
            }

            if ( _towards == QStringLiteral("Edgware via Bank"))
            {
                avoidList.push_back(5);
                avoidList.push_back(6);
            }

            if ( _towards == QStringLiteral("Edgware via CX"))
            {
                avoidList.push_back(1);
                avoidList.push_back(2);
            }

            if ( _towards == QStringLiteral("Finchley Central via Bank"))
            {
                avoidList.push_back(5);
                avoidList.push_back(6);
            }

            if ( _towards == QStringLiteral("Golders Green via Bank"))
            {
                avoidList.push_back(5);
                avoidList.push_back(6);
            }
        }

    if ( _towards == QStringLiteral("Golders Green via CX"))
    {
        avoidList.push_back(1);
        avoidList.push_back(2);
    }



//    }

    else if( isCircle() )
    {
        if( _towards == QStringLiteral("Edgware Road"))
            avoidList.push_back(0);

        else if( _towards.startsWith(QStringLiteral("Edgware Road (Cir")))
            avoidList.push_back(1);
    }
    else if( isDLR())
      {
          if( _line->isOutbound())
          {
              if( _destinationName.startsWith(QStringLiteral("Canary Wharf")) )
              {
                  avoidList.push_back(2);
                  avoidList.push_back(3);
                  avoidList.push_back(9);
                  avoidList.push_back(10);
              }

              if( _destinationName.startsWith(QStringLiteral("Beckton")) )
              {
                  avoidList.push_back(1);
                  avoidList.push_back(11);
              }

              if( _destinationName.startsWith(QStringLiteral("Gallions Reach")) )
              {
                  avoidList.push_back(1);
                  avoidList.push_back(11);
              }

              if( _destinationName.startsWith(QStringLiteral("Woolwich")))
              {
                  avoidList.push_back(1);
                  avoidList.push_back(11);

              }

              if( _stationName == QStringLiteral("Canning Town"))
                  if( _platform == QStringLiteral("Platform 1"))
              {
                  avoidList.push_back(3);
                  avoidList.push_back(4);
                  avoidList.push_back(8);

              }

              if( _stationName == QStringLiteral("Shadwell"))
                  if( _platform == QStringLiteral("Platform 1"))
              {
                  avoidList.push_back(1);

              }

              if( _stationName == QStringLiteral("Westferry"))
                  if( _platform == QStringLiteral("Platform 1"))
              {
                  avoidList.push_back(2);
                  avoidList.push_back(5);

              }

              if( _destinationName.startsWith(QStringLiteral("Lewisham")))
              {
                  avoidList.push_back(9);
                  avoidList.push_back(10);
              }

              if( _stationName == QStringLiteral("Poplar"))
                  if( _platform == QStringLiteral("Platform 3"))
               {
                    avoidList.push_back(4);
               }
          }
          else
          {
              if( _destinationName.startsWith(QStringLiteral("Stratford International")))
              {
                  avoidList.push_back(2);
                  avoidList.push_back(5);
                  avoidList.push_back(6);

              }
              if( _destinationName.startsWith(QStringLiteral("Stratford")))
              {
                   avoidList.push_back(0);
              }

              if( _destinationName.startsWith(QStringLiteral("Tower Gateway")))
              {
                  avoidList.push_back(1);
                  avoidList.push_back(2);
              }

              if( _destinationName.startsWith(QStringLiteral("Bank")))
              {
                  avoidList.push_back(1);
                  avoidList.push_back(2);
              }

              if( _stationName == QStringLiteral("Stratford"))
                  if( _platform == QStringLiteral("Platform 17"))
                  {
                    avoidList.push_back(10);
                    avoidList.push_back(11);
                  }

                if( _stationName == QStringLiteral("Stratford"))
                   if( _platform == QStringLiteral("4a"))
                      {
                        avoidList.push_back(9);
                        avoidList.push_back(10);
                      }
          }
      }
    else if( isLondonOverground())
    {

        if( _stationName == QStringLiteral("Canonbury"))
            if( _platform == QStringLiteral("Platform 3"))
            {
                avoidList.push_back(0);
            }

        if( _stationName == QStringLiteral("Highbury & Islington"))
            if( _platform == QStringLiteral("Platform 7"))
            {
                avoidList.push_back(0);
            }

        if( _stationName == QStringLiteral("Hackney Downs"))
            if( _platform == QStringLiteral("Platform 3"))
            {
                avoidList.push_back(6);
            }

        if( _stationName == QStringLiteral("Gospel Oak"))
            if( _platform == QStringLiteral("Platform 3"))
            {
                avoidList.push_back(13);
            }

        if( _destinationName == QStringLiteral("Barking Riverside"))
            {
                avoidList.push_back(12);
            }

        if( _destinationName == QStringLiteral("Watford Junction"))
            {
                avoidList.push_back(12);
                avoidList.push_back(13);
                avoidList.push_back(14);
            }

        if( _destinationName == QStringLiteral("Stratford (London)"))
            {
                avoidList.push_back(2);
                avoidList.push_back(5);
                avoidList.push_back(15);
                avoidList.push_back(16);
            }

        if( _destinationName == QStringLiteral("London Euston"))
            {
                avoidList.push_back(12);
                avoidList.push_back(13);
                avoidList.push_back(14);
            }

        if( _destinationName == QStringLiteral("Richmond (London)"))
        {
                avoidList.push_back(0);
        }

        if( _destinationName == QStringLiteral("Willesden Junction"))
        {

        }

        if( _destinationName == QStringLiteral("South Acton"))
        {
                avoidList.push_back(0);
        }

        if( _destinationName == QStringLiteral("Kensal Rise"))
        {

        }
    }

    else if( isDistrict() )
    {
        if( _towards == QStringLiteral("Olympia"))
        {
            for( auto& attach : stopPoint->attachedBranches)
            {
                if( attach.branch->getId() != 5 && attach.branch->getId()  != 6)
                    exclude.push_back(attach.branch);
            }
        }

        if( _towards == QStringLiteral("Upminster") || _towards == QStringLiteral("Barking") || _towards == QStringLiteral("Tower Hill"))
        {
            if( _nextLocation == QStringLiteral("Earl's Court") || _nextLocation == QStringLiteral("Gloucester Road"))
                avoidList.push_back(4);

            avoidList.push_back(5);
        }

        if( _towards == QStringLiteral("Wimbledon"))
        {
            if( _nextLocation == QStringLiteral("Earl's Court"))
                wantID = 4;
        }

        if( _previousLocation.startsWith(QStringLiteral("Chiswick Park")))
            avoidList.push_back(3);

        if( _towards == QStringLiteral("Check Front of Train"))
            if( _stationName == QStringLiteral("Turnham Green"))
            {
                avoidList.push_back(0);
                avoidList.push_back(1);
            }
    }
    else if( isCentral() )
    {
        if( _previousLocation.startsWith(QStringLiteral("West Acton")))
            if( _nextLocation == QStringLiteral("North Acton"))
                avoidList.push_back(0);

        if( _destinationName == QStringLiteral("Woodford"))
            avoidList.push_back(4);
    }

    else if( isMetropolitan())
    {
        if( _stationName == QStringLiteral("Harrow-on-the-Hill"))
        if( _platform == QStringLiteral("Northbound Fast - Platform 3"))
            avoidList.push_back(8);

        if( _nextLocation == QStringLiteral("Harrow-on-the-Hill"))
            if( _platform == QStringLiteral("Northbound Fast - Platform 3 "))
               if( _previousLocation == QStringLiteral("Wembley Park"))
                avoidList.push_back(8);
    }

    else if( isPiccadilly() )
    {
        if( _nextLocation == QStringLiteral("Heathrow Terminal 4"))
        {
           avoidList.push_back(2);
           avoidList.push_back(3);
        }

        if( _destinationName == QStringLiteral("Heathrow T4"))
        {
            avoidList.push_back(3);
        }

        if( _currentLocation == QStringLiteral("Left Heathrow"))
            if( _platform == QStringLiteral("Eastbound"))
        {
                avoidList.push_back(2);
                avoidList.push_back(5);
        }

        if( _stationName == QStringLiteral("Hatton Cross"))
            if( _platform == QStringLiteral("Eastbound"))
        {
                avoidList.push_back(5);
        }

//        if( _nextLocation == QStringLiteral("Acton Town"))
//        {
//            if( _towards.startsWith(QStringLiteral("Heathrow")) || _towards.startsWith(QStringLiteral("Northfi")) )
//            {
//                avoidList.push_back(0);
//                avoidList.push_back(1);
//            }
//            else if( _previousLocation.startsWith(QStringLiteral("South Ealing")))
//            {
//                avoidList.push_back(0);
//            }
    }
    else if( isElizabeth())
    {
        if( _direction == Line::outbound)
        {
            // to fix liverpool street area
            if(_destinationName.contains(QStringLiteral("Liverpool St"), Qt::CaseInsensitive))
            {
                wantID = 11;
            }
            else
            {
                avoidList.push_back(6);
                avoidList.push_back(11);
            }
        }
    }

    if( wantID == -1)
    {
        for( auto& attach : stopPoint->attachedBranches)
        {
            if( std::find(avoidList.begin(), avoidList.end(), attach.branch->getId()) != avoidList.end())
                exclude.push_back(attach.branch);
        }
    }
    else
    {
        for( auto& attach : stopPoint->attachedBranches)
        {
            if( attach.branch->getId() != wantID)
                exclude.push_back(attach.branch);
        }
    }

    return exclude;
}

std::vector<PossibleAttactment> Vehicle::getPossibleAttachments(const std::vector<Branch *> &exclude)
{
    std::vector<PossibleAttactment> possibleAttachments;

    auto addPossibility = [&possibleAttachments](const BranchConnect& connect) {

        PossibleAttactment p;
        p.branch =  connect.branch;
        p.index =  connect.idx;
        possibleAttachments.push_back(p);
    };

    for( auto& attach : _nextStopPoint->attachedBranches)
    {
        auto it = std::find( exclude.begin(), exclude.end(), attach.branch);
        if( it != exclude.end())
            continue;

        addPossibility(attach);
    }

    if( possibleAttachments.empty())
    {
        qqDebug("FAILED Attachment:");
        addPossibility(_nextStopPoint->attachedBranches[0]);
    }

    return possibleAttachments;
}

namespace  {

std::vector<std::shared_ptr<const StopPoint>> searchforStation(Vehicle* v, const Line *line, QString toStation)
{
    if( v->isTube())
        toStation.append(QStringLiteral(" Underground"));

    else if( v->isDLR())
        toStation.append(QStringLiteral(" DLR"));

    else if( v->isElizabeth() || v->isLondonOverground())
        toStation.append(QStringLiteral(" Rail"));

    std::vector<std::shared_ptr<const StopPoint>> nextStopPoints = line->getStopPointSlowRetry(toStation);

#ifdef Q_OS_WIN__IGNORE
    if( nextStopPoints.size() >1)
    {
        qDebug() << "Multiple Stops BEGIN : " << QString::fromStdString(curGPS.toString());
        for(const StopPoint* stopPoint : nextStopPoints)
            qDebug() << "NextStop:" << stopPoint->name << ":" << _line->name() << ":" <<curGPS.distanceTo(stopPoint->position);
        qDebug() << "END";
    }
#endif

    if( nextStopPoints.empty() )
    {
        int pos = toStation.indexOf(QChar('/'));

        if( pos != -1)
        {
            nextStopPoints = line->getStopPointSlowRetry(toStation.mid(pos+1));
        }
    }

    if( nextStopPoints.empty())
        v->qqDebug("WARNING: Not found : " + toStation );

    return nextStopPoints;
}
}

bool Vehicle::placeOnTrack()
{
    if( _line == nullptr)
    {
        qqDebug("Not found Line: ");
        return false;
    }

    if( _nextLocation.contains("Liverpool", Qt::CaseInsensitive) || _stationName.contains("Liverpool", Qt::CaseInsensitive))
    {
        int i;
        i = 3;
    }

    auto searchUsingStationName = [this] () -> std::shared_ptr<const StopPoint> {

        if( _hailAndRide)
            return nullptr;

        QString toStation = _useVehicleBehaviour? _nextLocation : _stationName;
        auto nextStopPoints = searchforStation(this, _line.get(), toStation);

        if( nextStopPoints.empty())
            return nullptr;

        std::sort( nextStopPoints.begin(), nextStopPoints.end(), [curGPS = getPos()](std::shared_ptr<const StopPoint>& left, std::shared_ptr<const StopPoint>& right) {
            return curGPS.distanceTo(left->position) < curGPS.distanceTo(right->position);
        });

        return nextStopPoints[0];
    };

    auto searchUsingNaptan = [this] () {
        return _line->getStopPoint(_naptanId);
    };

    using criteriaFuncs = std::vector<std::function<std::shared_ptr<const StopPoint>()>>;

    auto searchStation = [] (const criteriaFuncs& searchList) -> std::shared_ptr<const StopPoint>{

        for(const auto& criteria : searchList)
        {
            if( auto stop = criteria())
                return stop;
        }

        return nullptr;
    };

    std::shared_ptr<const StopPoint> nextStopPoint;

    if( !isTube() || !_useVehicleBehaviour)
        nextStopPoint = searchStation(criteriaFuncs({ searchUsingNaptan, searchUsingStationName }));
    else
        nextStopPoint = searchStation(criteriaFuncs({ searchUsingStationName, searchUsingNaptan }));

    if( nextStopPoint == nullptr)
        return false;

    _nextStopPoint = nextStopPoint;

    auto& attachedBranches = _nextStopPoint->attachedBranches;

    if(attachedBranches.empty())
    {
#ifdef Q_OS_WIN
        qDebug() << "!!!NO BRANCHES FOUND!!!! : " << _lineId <<":"<< _nextStopPoint->id << ":" << _nextStopPoint->name;
#endif
        return false;
    }

    std::vector<std::shared_ptr<const StopPoint>> prevPoints;

    if( !_previousLocation.isEmpty())
        prevPoints = searchforStation(this, _line.get(), _previousLocation);

    if( prevPoints.empty())
        _prevStopPoint.reset();
    else
        _prevStopPoint = prevPoints[0];

    PossibleAttactment defaultAttach;

    if( attachedBranches.size() == 1)
    {
        defaultAttach.branch = attachedBranches[0].branch;
        defaultAttach.index = attachedBranches[0].idx;
    }
    //This code stopped Sidings from working. Now you need to test all underground lines to see if anything gets broke a
    // as this code operates on all trains.
    // oK? ok
    else if( _prevStopPoint != nullptr)
    {
        for(auto& toItem  : attachedBranches)
        {
            for(auto& fromItem : _prevStopPoint->attachedBranches)
            {
                if( toItem.branch != fromItem.branch)
                    continue;

                defaultAttach.branch = toItem.branch;
                defaultAttach.index = toItem.idx;
                break;
            }

            if( defaultAttach.branch != nullptr)
                break;
        }
    }

    std::vector<PossibleAttactment> possibleAttachments;

    if( defaultAttach.branch != nullptr)
    {
        if( !isBus() )
        {
            BranchConnect bc;
            bc.idx = defaultAttach.index;
            bc.branch = defaultAttach.branch;
        }
        possibleAttachments.push_back(defaultAttach);
    }
    else
    {
        std::vector<Branch*> exclude = buildBranchExcludeList(_nextStopPoint);
        possibleAttachments = getPossibleAttachments(exclude);
    }

    const PossibleAttactment& possible = possibleAttachments[0];

    bool attached(false);

    Branch* prevBranch = _attachment.branch;

    if( !attached )
    {
        _attachment.branch = possible.branch;
        _attachment.idx = possible.index;
    }

    if( prevBranch != _attachment.branch)
        _trackOffSet = 0;

    return true;

//    if( attachedBranches.size() >1 && _behaviour != Unknown)
//        qqDebug(QString("Selected Branch : %1").arg(possible.branch->getId()));
}

void Vehicle::updateDisplayTowards()
{
    if( isBus() || isRiverBoat() || isElizabeth() )
        _displayTowards = _destinationName;
    else
        _displayTowards = _towards;
}

void Vehicle::updateHeathrowTowards(bool b)
{
    if( _lineId == QStringLiteral("piccadilly"))
    {
        if( b)
        {
            _displayTowards = _towards;
            return;
        }

        if( _displayTowards.startsWith(QStringLiteral("Heathrow T1")))
            _displayTowards = QStringLiteral("T1235 Heathrow");

        else if( _displayTowards.startsWith(QStringLiteral("Heathrow via T4")))
            _displayTowards = QStringLiteral("T4 Heathrow");

        else if( _displayTowards.startsWith(QStringLiteral("Heathrow Terminal 5")))
            _displayTowards = QStringLiteral("T5-Heathrow");
    }
}

float Vehicle::calcTrackPos(const BranchConnect& attach)
{
    const auto& smoothPoints = attach.branch->getSmoothPoints();

    if( attach.idx >= static_cast<int>(smoothPoints.size()))
        return 0.0f;

    float factor = 0.0f;

    if( _useVehicleBehaviour && _behaviour != Vehicle::Unknown)
    {
        float secsNextStn(100);

        if( _prevStopPoint != nullptr)
            secsNextStn = _prevStopPoint->position.distanceTo(_nextStopPoint->position)/1609.3f * 160.0f;

        auto calcFactor = [this,secsNextStn] (float start, float end) {
            return std::max( end, start - _timeInBetweenStation/secsNextStn);
        };

        auto calcFactorMin = [this,secsNextStn] (float start, float end) {
            return std::min( end, start - _timeInBetweenStation/secsNextStn);
        };


        if( _behaviour == Vehicle::Approaching)
            factor = calcFactor(0.30f, 0.15f);

        else if( _behaviour == Vehicle::Between)
            factor = calcFactor(0.85f, 0.30f);

        else if( _behaviour == Vehicle::Departed)
            factor = calcFactorMin(0.0f, -0.33f);
    }
    else
    {
        if( _nextStopPoint && _nextStopPoint->timeTo > 0)
        {
            factor = _timeToStation / (60.0f*_nextStopPoint->timeTo);
        }
        else
        {
            if(isTram())
                factor = _timeToStation/300.0f;
            else if(isOverground())
                factor = _timeToStation/300.0f;
            else if(isElizabeth())
                factor = _timeToStation/600.0f;
            else if(isTube())
                factor = _timeToStation/320.0f;
            else if(isDLR())
                factor = _timeToStation/180.0f;
            else if(isNationalRail())
                factor = _timeToStation/400.0f;
            else if(isRiverBoat())
                factor = _timeToStation/300.0f;
            else if(isBus())
                factor = _timeToStation/250.0f;
            else
                factor = _timeToStation/60.0f;
        }
    }

    if( isBus())
    {
        factor = std::clamp(factor, 0.0f, 1.0f);
    }

    float idx = attach.idx -  smoothPoints[attach.idx]->prevStopOffset * factor;
    return std::clamp(idx, 0.0f,static_cast<float>(smoothPoints.size()-1));
}


void Vehicle::update()
{
    if( _attachment.branch == nullptr)
        return;

    auto& pts = _attachment.branch->getSmoothPoints();

    _trackOffSet = std::clamp(_trackOffSet,0.0f,  static_cast<float>(pts.size()-1));
    _timeToStation--;
    _timeToStation = std::max(_timeToStation, isBus()? -15 :0);
    _timeInBetweenStation++;

    if( _behaviour == Vehicle::Between && _timeToStation < 20)
        _timeToStation = 20;

#ifdef Q_OS_WIN
    //        qDebug() << "BUS : " << _lineId << ":" << _vehicleId << ":" << _trackOffSet << ":" << _timeToStation << ":" << _timeInBetweenStation;
#endif

    float reqTrackOffSet = calcTrackPos(_attachment);
    float diff = reqTrackOffSet - _trackOffSet;

    if(_trackOffSet ==0 )
        _trackOffSet = reqTrackOffSet;

    if( /*(isBus() &&  diff < 0) ||*/ _nextStopPoint == nullptr)
        return;

    else if( diff < -100)
        _trackOffSet = reqTrackOffSet;

    else if( diff > 0.0f )
        _trackOffSet += std::min(0.4f, diff/8.0f);

    _trackOffSet = std::clamp(_trackOffSet,0.0f,  static_cast<float>(pts.size()-1));
}

bool Vehicle::nextStopTheSame(const std::shared_ptr<Vehicle> &vehicle) const
{
    return _nextStopPoint == vehicle->_nextStopPoint;
}

QString Vehicle::getDataSource() const
{
    if( _dataSource == DataSource_TFL)
        return QStringLiteral("Tfl OpenData");

    if( _dataSource == DataSource_NetworkRail)
        return QStringLiteral("Network Rail");

    return QStringLiteral("Unknown");
}

const TrackPoint *Vehicle::getTrackPoint() const
{
    if( _attachment.branch == nullptr)
        return nullptr;

    const auto& branchPts = _attachment.branch->getSmoothPoints();

    size_t offSet = static_cast<size_t>(_trackOffSet);
    if( offSet >= branchPts.size())
        return nullptr;

    return branchPts[offSet].get();
}

GPSLocation Vehicle::getPos() const
{
    if( _attachment.branch == nullptr)
        return GPSLocation();

    const auto& branchPts = _attachment.branch->getSmoothPoints();

    size_t trackOffset = static_cast<size_t>(std::floor(_trackOffSet));

    if( trackOffset >= branchPts.size())
        return GPSLocation();

    GPSLocation posFrom = branchPts[trackOffset]->position;

    if( trackOffset == branchPts.size()-1)
        return posFrom;

    GPSLocation posTo = branchPts[trackOffset+1]->position;

    float distOffset = posTo.distanceTo(posFrom) * (_trackOffSet - std::floor(_trackOffSet));

    const QuarternionF& qHdg = QHDG(branchPts[trackOffset]->hdg);

    Vector3F vecUnit = QVRotate(qHdg, Vector3F(0,0,-1));

    Vector3F platformOffSet;

    if( /*!isBus() && */_platformOffSet != 0)
        platformOffSet += QVRotate(QHDG(branchPts[trackOffset]->hdg + 90),
                        Vector3F(0,0,-_platformOffSet/2.0f * _line->getOffset()));

    return  posFrom + distOffset * vecUnit + platformOffSet;
}

bool Vehicle::isFast() const
{
    return _fast;
}

bool Vehicle::isBus() const
{
    return _modeName == Line::mode_busLine;
}

bool Vehicle::isDLR() const
{
    return _modeName == Line::dlrLine;
}

bool Vehicle::isTube() const
{
    return _modeName == Line::mode_tube;
}

bool Vehicle::isOverground() const
{
    return _modeName == Line::mode_overground;
}

bool Vehicle::isElizabeth() const
{
    return _modeName == Line::mode_elizabeth;
}

bool Vehicle::isRiverBoat() const
{
    return _modeName == Line::mode_riverBus;
}

bool Vehicle::isNationalRail() const
{
    return _modeName == Line::mode_nationalRail;
}

QString Vehicle::currentBehaviour() const
{
    switch(_behaviour)
    {
    case Approaching:
        return QStringLiteral("Approaching");

    case Between:
        return QStringLiteral("Between");

    case Departed:
        return QStringLiteral("Departed");

    case At:
        return QStringLiteral("At");

    default:
        return QStringLiteral("Unknown");
    };
}

bool Vehicle::isHammersmithyCity() const
{
    return _lineId == Line::hammersmithCityLine;
}

bool Vehicle::isCircle() const
{
    return _lineId == Line::circleLine;
}

bool Vehicle::isPiccadilly() const
{
    return _lineId == Line::piccadillyLine;
}

bool Vehicle::isCentral() const
{
    return _lineId == Line::centralLine;
}

bool Vehicle::isDistrict() const
{
    return _lineId == Line::districtLine;
}

bool Vehicle::isJubilee() const
{
    return _lineId == Line::jubileeLine;
}

bool Vehicle::isNorthern() const
{
    return _lineId == Line::northernLine;
}

bool Vehicle::isBakerloo() const
{
    return _lineId == Line::bakerlooLine;
}

bool Vehicle::isMetropolitan() const
{
    return _lineId == Line::metropolitanLine;
}

bool Vehicle::isLondonOverground() const
{
    return _lineId == Line::lionessLine
        || _lineId == Line::mildmayLine
        || _lineId == Line::windrushLine
        || _lineId == Line::weaverLine
        || _lineId == Line::suffragetteLine
        || _lineId == Line::libertyLine;
}

bool Vehicle::isTram() const
{
    return _lineId == Line::tramLine;
}

bool Vehicle::isVictoria() const
{
    return _lineId == Line::victoriaLine;
}
