#ifndef TFLVEHICLERENDERER_H
#define TFLVEHICLERENDERER_H

#include "TFLRenderer.h"
#include "RadarSymbols.h"
#include <QHash>

struct Vehicle;

class TFLVehicleRenderer : public TFLRenderer
{
public:
    TFLVehicleRenderer(const TFLView* view);

    void updateCache(const ViewState& viewState) override;
    void paint(const ViewState& viewState, QPainter& p) override;
    void paintText(const ViewState& viewState, QPainter& p) override;

    const TFLView *getView() const;

private:
    const TFLView* _view = nullptr;

    QHash<QString, std::shared_ptr< Vehicle>> _cache;
    RadarSymbols _symbols;

};

#endif // TFLVEHICLERENDERER_H
