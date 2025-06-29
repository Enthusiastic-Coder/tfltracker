#include "MouseAreaManager.h"

MouseArea& MouseAreaManager::add(const std::string &label)
{
     _areas.push_back(label);
     return _areas.back();
}

void MouseAreaManager::update()
{
    for(MouseArea& m:_areas)
        m.update();
}

bool MouseAreaManager::handleMouseDown(const QPoint &pt)
{
    _areaDown = areaFromPoint(pt);
    if( _areaDown == nullptr)
        return false;

    return _areaDown->handleMouseDown(pt);
}

bool MouseAreaManager::handleMouseMove(const QPoint &pt)
{
    if( _areaDown == nullptr)
        return false;

    return _areaDown->handleMouseMove(pt);
}

bool MouseAreaManager::handleMouseUp(const QPoint &pt)
{
    if( _areaDown == nullptr)
        return false;

    bool bHandled = _areaDown->handleMouseUp(pt);
    _areaDown = nullptr;
    return bHandled;
}

void MouseAreaManager::handleResize(const QSize &sz)
{
    for(MouseArea& m:_areas)
        m.handleResize(sz);
}

void MouseAreaManager::paint(QPainter &p)
{
    for(MouseArea& m:_areas)
        m.paint(p);
}

bool MouseAreaManager::isMouseActive() const
{
    return _areaDown != nullptr;
}

MouseArea* MouseAreaManager::areaFromPoint(const QPoint &pt)
{
    for( MouseArea& m:_areas)
        if( m.isOverArea(pt))
            return &m;

    return nullptr;
}
