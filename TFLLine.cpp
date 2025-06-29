#include "TFLLine.h"
#include "Line.h"
#include <QMap>

std::set<QString> TFLLine::static_visibleList;
std::set<QString> TFLLine::static_NationalRaiList;
std::set<QString> TFLLine::static_RiverBoatList;

TFLLine::TFLLine(QObject *parent) :
    QObject(parent)
{

}

TFLLine::~TFLLine()
{
}

void TFLLine::setLines(std::shared_ptr<Line> inbound, std::shared_ptr<Line> outbound)
{
    _inbound = inbound;
    _outbound = outbound;

    if( isNationalRail())
        static_NationalRaiList.insert(id());
    else if( isRiverBus())
        static_RiverBoatList.insert(id());
}

bool TFLLine::isReadOnly() const
{
    return _outbound->isReadOnly();
}

void TFLLine::setColor(QColor color)
{
    _inbound->setColor(color);
    _outbound->setColor(color);
}

QColor TFLLine::getColor() const
{
    return _outbound->getColor();
}

void TFLLine::setOffSet(int offset)
{
    _offSetDirty = _outbound->getOffset() != offset;
    _inbound->setOffSet(offset);
    _outbound->setOffSet(offset);
}

int TFLLine::getOffset() const
{
    return _outbound->getOffset();
}

void TFLLine::setVisible(bool bShow)
{
    if( bShow)
        static_visibleList.insert(id());
    else
        static_visibleList.erase(id());

    _inbound->setVisible(bShow);
    _outbound->setVisible(bShow);
}

bool TFLLine::isVisible() const
{
    return _outbound->isVisible();
}

void TFLLine::setShowStops(bool bShow)
{
    if( _outbound->getShowStops() == bShow)
        return;

    _inbound->setShowStops(bShow);
    _outbound->setShowStops(bShow);

    emit onShowStopsChanged(bShow);
}

bool TFLLine::getShowStops() const
{
    return _outbound->getShowStops();
}

QString TFLLine::getName() const
{
    return _outbound->name();
}

QString TFLLine::id() const
{
    return _inbound->id();
}

bool TFLLine::isBus() const
{
    return _outbound->isBus();
}

bool TFLLine::isNationalRail() const
{
    return _outbound->isNationalRail();
}

bool TFLLine::isRiverBus() const
{
    return _outbound->isRiverBus();
}

void TFLLine::updateBranches()
{
    _offSetDirty = false;
    _outbound->updateBranches();
    _inbound->updateBranches();
}

bool TFLLine::isOffSetDirty() const
{
    return _offSetDirty;
}

void TFLLine::setUpdatedOK(bool b)
{
    _updatedOK = b;
    _updateDate = QDateTime::currentDateTime();
    emit onLastDownloadedChanged(getDownloadStatusDescription());
    emit onUpdatedOk(b);
}

QString TFLLine::getDownloadStatusDescription() const
{
    if( _updateDate.isNull())
        return QStringLiteral("");

    if( !_updatedOK)
        return QStringLiteral("Failed to Download.");

    QDateTime today = QDateTime::currentDateTime();

    int daysTo = _updateDate.daysTo(today);

    if( daysTo ==0)
        return QStringLiteral("Updated Today.");

    else if( daysTo ==1 )
        return QStringLiteral("Updated yesterday");

    return "Updated " + QString::number(daysTo)  + " days ago.";
}

bool TFLLine::getUpdatedOK() const
{
    return _updatedOK;
}

void TFLLine::setUpdateDate(QDateTime date)
{
    _updateDate = date;
}

QDateTime TFLLine::getUpdateDate() const
{
    return _updateDate;
}

LineType::mode TFLLine::getType() const
{
    return _outbound->getType();
}

QVariantList TFLLine::getStopPoints() const
{
    QVariantList list;

    const auto& inStopPoints =  _inbound->getStopPoints();

    std::map<QString,StopPoint*> tempMap;

    for(const auto& item : inStopPoints )
        tempMap[item.first] = item.second.get();

    const auto& outStopPoints =  _outbound->getStopPoints();

    for(const auto& item : outStopPoints )
        tempMap[item.first] = item.second.get();

    for(const auto& item : tempMap)
    {
        if( !item.second->isPassPoint && !item.second->name.isEmpty())
            list << QVariant::fromValue(item.second);
    }

    return list;
}

const std::set<QString> &TFLLine::getFullVisibleList()
{
    return static_visibleList;
}

const std::set<QString> &TFLLine::getNationalRailList()
{
    return static_NationalRaiList;
}

const std::set<QString> &TFLLine::getRiverBoatList()
{
    return static_RiverBoatList;
}
