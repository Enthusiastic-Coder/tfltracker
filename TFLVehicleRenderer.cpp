#include <jibbs/math/qhdgtable.h>
#include <jibbs/math/degreesRadians.h>
#include <jibbs/math/SinTable.h>

#include "TFLVehicleRenderer.h"
#include "Vehicle.h"
#include "TFLView.h"
#include "Vehicle.h"
#include "Line.h"

#include "screenUtils.h"

TFLVehicleRenderer::TFLVehicleRenderer(const TFLView *view)
    : _view(view)
{

}

void TFLVehicleRenderer::updateCache(const ViewState &viewState)
{
}

void TFLVehicleRenderer::paint(const ViewState &viewState, QPainter &p)
{
    QHash<QRgb, QVector<QPoint>> trainSymbolPts;
    QHash<QString, std::shared_ptr< Vehicle>> latestCache;

    TFLView* view = const_cast<TFLView*>(getView());

    const bool proxWarning = view->_proximityWarningInProgress;
    bool updatedProxWarning = false;

    auto& qHdg = QHdgTable::get();

    const int box = 10;
    const int stickLength = 20;

    const auto& vehicles = viewState.world->getTrains();

    for(const auto& vehicle : vehicles)
    {
        auto* trkPoint = vehicle->getTrackPoint();

        if( trkPoint == nullptr)
            continue;

        GPSLocation pos = vehicle->getPos();

        if( !screenUtils::ptInScreen(viewState, pos))
            continue;

        latestCache[vehicle->_vehicleId] =  vehicle;

        bool bProxmityWarning = getView()->isProximityWarning(vehicle);
        updatedProxWarning |= bProxmityWarning;

        QPoint trainPt(screenUtils::toScreen(viewState, pos));

        p.setPen(viewState.mapNight ? Qt::yellow: Qt::black);

        QColor lineColor;
        if( viewState.mapNight && vehicle->_lineId == Line::northernLine)
            lineColor = Qt::white;
        else
            lineColor = vehicle->_line->getColor();

        p.setBrush(lineColor);
        const int proxBox = box * (bProxmityWarning? 2:1);

        QRect rc(trainPt.x()-proxBox/2, trainPt.y()-proxBox/2, proxBox, proxBox);

        p.drawEllipse(rc);

        if( viewState.pixelsPerMile > 10.0f)
        {
            auto& colorPts = trainSymbolPts[lineColor.rgb()];

            Vector3F v = QVRotate(qHdg.Hdg(trkPoint->hdg), Vector3F(0,0,-stickLength));

            QPoint ptV(v.x,v.z);

            const auto& pts = _symbols.GetSimpleBlip(viewState.pixelsPerMile > 200 ? 20 : 10, trkPoint->hdg);
            for(const auto& p:pts)
            {
                colorPts << p.toPoint() + trainPt + ptV;
            }
        }
    }

    if( updatedProxWarning != proxWarning)
    {
        view->_proximityWarningInProgress = updatedProxWarning;
        emit view->onProximityWarningActive(updatedProxWarning);
    }

    _cache = latestCache;

    paintHashCollection( p, trainSymbolPts, 255, 2 );
}

void TFLVehicleRenderer::paintText(const ViewState &viewState, QPainter &p)
{
    const std::shared_ptr<Vehicle>& selected = viewState.world->getSelectedVehicle(0);

    auto paintTexts = [this,selected, &viewState] (QPainter &p, const auto& cache) {

        QFont labelFont;
        labelFont.setFamily("Verdana");
        labelFont.setPixelSize(10);
        labelFont.setUnderline(true);
        p.setFont(labelFont);

        QFontMetrics fm = p.fontMetrics();
        const int fontHeight = fm.height();

        QPen pen;

        QColor finalColor(viewState.mapNight ? Qt::yellow: Qt::white);
        QColor backgroundColor(Qt::darkGray);

        p.setBackgroundMode(Qt::OpaqueMode);

        p.setBackground(backgroundColor);

        pen.setColor(finalColor);

        p.setPen(pen);

        for(const auto &vehicle : cache)
        {
            if( vehicle->isBus() )
            {
                if(getView()->_busBlipVerbosity == BlipVerbosity::none)
                    continue;
            }
            else
            {
                if(getView()->_trainBlipVerbosity == BlipVerbosity::none)
                    continue;
            }

            QPoint trainPt( screenUtils::toScreen(viewState, vehicle->getPos()));

            QStringList labels;

            if( vehicle->_line != nullptr)
            {
                const bool busLineId = vehicle->isBus() && (getView()->_busBlipVerbosity == BlipVerbosity::LineId || getView()->_busBlipVerbosity == BlipVerbosity::All);
                const bool trainLineId = !vehicle->isBus() && (getView()->_trainBlipVerbosity == BlipVerbosity::LineId || getView()->_trainBlipVerbosity == BlipVerbosity::All);

                if( busLineId || trainLineId)
                    labels << vehicle->_line->name();
            }

            QString strText = vehicle->_displayTowards;

            if( strText.isEmpty())
                strText = vehicle->_destinationName;

            if( viewState.pixelsPerMile < 500)
                strText = strText.left(6);

            if( !strText.isEmpty())
                labels << strText;

            const bool busVehicleId = vehicle->isBus() && (getView()->_busBlipVerbosity == BlipVerbosity::VehicleId || getView()->_busBlipVerbosity == BlipVerbosity::All);
            const bool trainVehicleId = !vehicle->isBus() && (getView()->_trainBlipVerbosity == BlipVerbosity::VehicleId || getView()->_trainBlipVerbosity == BlipVerbosity::All);

            if(busVehicleId || trainVehicleId)
            {
                if( !vehicle->_vehicleId.isEmpty())
                    labels << vehicle->_vehicleId;

                if( vehicle->isFast())
                    labels << QStringLiteral("(fast)");
            }

            if( labels.empty())
                continue;

            if( labels.size() ==1)
                labels << "";

            QSize sz;
            sz.setHeight( fm.height() * std::max(2,static_cast<int>(labels.size())));

            for(const auto &it: labels)
            {
                int iWidth = fm.horizontalAdvance(it);
                if (iWidth > sz.width())
                    sz.setWidth(iWidth);
            }

            const int labelSize = labels.size();
            const float sZWHalf = sz.width()/2.0f;
            const float sZHhalf = sz.height()/2.0f;

            bool proximityWarning = getView()->isProximityWarning(vehicle);

            if( proximityWarning )
                p.setBackground(Qt::red);

            QPoint pt;
            int angleWanted = 0;

            auto* trkPt = vehicle->getTrackPoint();

            if( trkPt != nullptr)
                angleWanted = trkPt->hdg - 90;

            angleWanted -= viewState.compassValue;

            const float SINW = SinTable::get().sin(angleWanted);
            const float COSW = SinTable::get().cos(angleWanted);

            float stickX = sZWHalf*1.5f * SINW;
            float stickY = sZHhalf*1.5f * COSW;

            pt.setX(trainPt.x() + stickX );
            pt.setY(trainPt.y() - stickY );

            QTransform prevTransform = p.transform();

            p.translate(trainPt.x(), trainPt.y());
            p.rotate(viewState.compassValue);
            p.translate(-trainPt.x(), -trainPt.y());

            if( selected == vehicle)
            {
                QRect rc(pt.x()-sZWHalf, pt.y()-sZHhalf, sz.width(), sz.height());
                rc.adjust(-3,-1,5,8);

                if( vehicle->_line != nullptr)
                    p.setBrush(vehicle->_line->getColor());
                else
                    p.setBrush(backgroundColor);

                p.setBackground(p.brush().color());
                p.drawRect(rc);
            }

            for (size_t i = 0; i < labelSize; ++i)
                p.drawText(pt.x()-sZWHalf, pt.y() - i*fontHeight+sZHhalf, labels[labelSize-i-1]);

            if( selected == vehicle)
                p.setBackground(backgroundColor);

            p.setTransform(prevTransform);
            if( proximityWarning)
                p.setBackground(backgroundColor);
        }

        p.setBackgroundMode(Qt::TransparentMode);

    };

    if( viewState.pixelsPerMile > 250)
        paintTexts(p, _cache);

    if( selected != nullptr && screenUtils::ptInScreen(viewState, selected->getPos()))
    {
        QMap<QString,std::shared_ptr<const Vehicle>> sel;
        sel[selected->_vehicleId] = selected;
        paintTexts(p, sel);
    }
}

const TFLView *TFLVehicleRenderer::getView() const
{
    return _view;
}
