#ifndef TFLLINE_H
#define TFLLINE_H

#include <QObject>
#include <QColor>
#include <QVariant>
#include <QDateTime>
#include <memory>
#include "LineType.h"
#include <set>

class Line;
class StopPoint;

class TFLLine : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool ReadOnly READ isReadOnly CONSTANT)
    Q_PROPERTY(QColor Color READ getColor WRITE setColor NOTIFY onColorChange)
    Q_PROPERTY(bool Visible READ isVisible WRITE setVisible NOTIFY onVisibleChanged)
    Q_PROPERTY(int OffSet READ getOffset WRITE setOffSet NOTIFY onOffSetChanged)
    Q_PROPERTY(bool ShowStops READ getShowStops WRITE setShowStops NOTIFY onShowStopsChanged)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(bool isBus READ isBus CONSTANT)
    Q_PROPERTY(QString downloadStatusDescription READ getDownloadStatusDescription NOTIFY onLastDownloadedChanged)
    Q_PROPERTY(bool downloadedOK READ getUpdatedOK NOTIFY onUpdatedOk)

public:
    explicit TFLLine(QObject *parent = nullptr);
    ~TFLLine();

    void setLines(std::shared_ptr<Line> inbound, std::shared_ptr<Line> outbound);

    bool isReadOnly() const;
    void setColor(QColor color);
    QColor getColor() const;

    void setOffSet(int offset);
    int getOffset() const;

    void setVisible(bool bShow);
    bool isVisible() const;

    void setShowStops(bool bShow);
    bool getShowStops() const;

    QString getName() const;
    QString id() const;

    bool isBus() const;
    bool isNationalRail() const;
    bool isRiverBus() const;

    void updateBranches();
    bool isOffSetDirty() const;

    Q_INVOKABLE void setUpdatedOK(bool);

    QString getDownloadStatusDescription() const;
    bool getUpdatedOK() const;

    void setUpdateDate(QDateTime date);
    QDateTime getUpdateDate() const;

    LineType::mode getType() const;

    Q_INVOKABLE QVariantList getStopPoints() const;

    static const std::set<QString> &getFullVisibleList();
    static const std::set<QString> &getNationalRailList();
    static const std::set<QString> &getRiverBoatList();

signals:
    void onColorChange(QColor);
    void onVisibleChanged(bool);
    void onActiveChanged(bool);
    void onOffSetChanged(int);
    void onLastDownloadedChanged(QString);
    void onUpdatedOk(bool);
    void onShowStopsChanged(bool);

public slots:


private:
    std::shared_ptr<Line> _inbound;
    std::shared_ptr<Line> _outbound;
    bool _offSetDirty = false;
    QDateTime _updateDate;
    bool _updatedOK;
    static std::set<QString> static_visibleList;
    static std::set<QString> static_NationalRaiList;
    static std::set<QString> static_RiverBoatList;;
};

#endif // TFLLINE_H
