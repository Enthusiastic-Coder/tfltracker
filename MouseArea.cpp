#include "MouseArea.h"

MouseArea::MouseArea(std::string strLabel):
    _strLabel(strLabel)
{

}

void MouseArea::update()
{
    if( _enabledHandler == nullptr)
        _isEnabled = true;
    else
        _isEnabled = _enabledHandler();
}

bool MouseArea::isOverArea(const QPoint &pt)
{
    return _rect.contains(pt);
}

void MouseArea::paint(QPainter &p)
{
    if( _isEnabled == false)
    {
        return;
    }

    if (!_target || !_paintHandler)
    {
        p.fillRect(_rect, QColor(255,255,255,20));
        return;
    }

    (_target->*_paintHandler)(p, _mouseDown, _rect);
}

void MouseArea::setEnabledHandler(std::function<bool ()> listener)
{
    _enabledHandler = listener;
}

void MouseArea::setSizeHandler(std::function<QRect (const QSize &)> listener)
{
    _sizeListener = listener;
}

void MouseArea::setMouseMoveListener(std::function<void (const QPoint &)> listener)
{
    _mouseMoveListener = listener;
}

void MouseArea::setPaintHandler(TFLView *target, PaintHandler handler)
{
    _target = target;
    _paintHandler = handler;
}

bool MouseArea::handleMouseDown(const QPoint &pt)
{
    if( _isEnabled == false)
        return false;

    _ptDown = pt;
    _mouseDown = true;
    return true;
}

bool MouseArea::handleMouseMove(const QPoint &pt)
{
    if( _isEnabled == false)
        return false;

    QPoint ptDiff = pt - _ptDown;

    if(_mouseMoveListener != nullptr)
        _mouseMoveListener(ptDiff);

    _ptDown = pt;
    return true;
}

bool MouseArea::handleMouseUp(const QPoint &pt)
{
    _mouseDown = false;

    if( _isEnabled == false)
        return false;

    return _rect.contains(pt);
}

void MouseArea::handleResize(const QSize &sz)
{
    if( _sizeListener != nullptr)
        _rect = _sizeListener(sz);
}
