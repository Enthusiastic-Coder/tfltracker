#ifndef LINE_H
#define LINE_H

#include "TFLModel.h"
#include "LineType.h"
#include <unordered_map>

class Line
{
public:
    static const QString inbound;
    static const QString outbound;
    static const QString hammersmithCityLine;
    static const QString circleLine;
    static const QString piccadillyLine;
    static const QString centralLine;
    static const QString districtLine;
    static const QString jubileeLine;
    static const QString northernLine;
    static const QString bakerlooLine;
    static const QString victoriaLine;
    static const QString waterlooCityLine;
    static const QString metropolitanLine;
    static const QString tramLine;

    static const QString lionessLine;
    static const QString mildmayLine;
    static const QString windrushLine;
    static const QString weaverLine;
    static const QString suffragetteLine;
    static const QString libertyLine;

    static const QString elizabethLine;
    static const QString thamesLink;
    static const QString dlrLine;
    static const QString london_cable_car;
    static const QString mode_busLine;
    static const QString mode_overground;
    static const QString mode_elizabeth;
    static const QString mode_riverBus;
    static const QString mode_riverTour;
    static const QString mode_nationalRail;
    static const QString mode_tube;
    static const QString mode_tram;
    static const QString mode_dlr;
    static const QString mode_cable_car;

    bool isHammersmithyCity() const;
    bool isCircle() const;
    bool isPiccadilly() const;
    bool isCentral() const;
    bool isDistrict() const;
    bool isDLR() const;
    bool isJubilee() const;
    bool isNorthern() const;
    bool isBakerloo() const;
    bool isLondonOverground() const;

    friend class LineBuilder;

    const QString& id() const;
    const QString& name() const;
    const QString& direction() const;

    void setColor(QColor color);
    QColor getColor() const;

    void setShowStops(bool bShow);
    bool getShowStops() const;

    void setOffSet(int offset);
    int getOffset() const;

    LineType::mode getType() const;

    static LineType getTypeInfo();

    bool isReadOnly() const;
    bool isOutbound() const;
    bool isVisible() const;

    bool isTrain() const;
    bool isRiverBus() const;
    bool isBus() const;
    bool isElizabethRail() const;
    bool isNationalRail() const;
    bool isOverground() const;

    void setVisible(bool bShow);
    void updateBranches();

    std::shared_ptr<StopPoint> getStopPoint(const QString &id) const;
    std::vector<std::shared_ptr<const StopPoint> > getStopPointSlowRetry(const QString &id) const;
    std::vector<std::shared_ptr<const StopPoint> > getStopPointSlow(const QString &id) const;
    const Branches& getBranches() const;
    std::shared_ptr<const Branch> getBranch(int idx) const;
    const std::unordered_map<QString, std::shared_ptr<StopPoint> > &getStopPoints() const;

    int getStopPointDiff(const QString& idFrom, const QString& idTo) const;

private:
    bool _readOnly = false;
    QString _id;
    QString _name;
    QString _mode;
    QString _direction;
    QColor _color;
    bool _showStops = false;
    bool _visible = false;
    int _offset = 50;

    std::unordered_map<QString, std::shared_ptr<StopPoint>> _stopPoints;
    std::unordered_map<QString, QString> _stopPointNames;
    std::vector<std::shared_ptr<Branch>> _branches;
    std::vector<std::vector<QString>> _orderedRoutes;

    static LineType _lineType;
};

#endif // LINE_H
