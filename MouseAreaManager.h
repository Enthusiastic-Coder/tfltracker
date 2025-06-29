#ifndef MOUSEAREAMANAGER_H
#define MOUSEAREAMANAGER_H

#include "MouseArea.h"
#include <vector>
#include <QPainter>
#include <QPoint>
#include <functional>

class MouseAreaManager
{
public:
    MouseArea& add(const std::string& label);

    void update();

    bool handleMouseDown(const QPoint& pt);
    bool handleMouseMove(const QPoint& pt);
    bool handleMouseUp(const QPoint& pt);
    void handleResize(const QSize& sz);

    void paint(QPainter& p);

    bool isMouseActive() const;

protected:
    MouseArea *areaFromPoint(const QPoint& pt);

private:
    MouseArea* _areaDown = nullptr;
    std::vector<MouseArea> _areas;
};

#endif // MOUSEAREAMANAGER_H
