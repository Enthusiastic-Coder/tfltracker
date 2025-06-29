#ifndef MOUSEAREA_H
#define MOUSEAREA_H

#include <QRect>
#include <QString>
#include <QPainter>
#include <QPoint>
#include <functional>

class TFLView;

class MouseArea
{
public:
    MouseArea(std::string strLabel);

    using PaintHandler = void (TFLView::*)(QPainter&, bool, const QRect&);

    void update();
    bool isOverArea(const QPoint& pt);
    void paint(QPainter& p);

    void setEnabledHandler(std::function<bool()>);
    void setPaintHandler(TFLView* target, PaintHandler handler);
    void setSizeHandler(std::function<QRect(const QSize &)> listener);
    void setMouseMoveListener(std::function<void(const QPoint &)> listener);

    bool handleMouseDown(const QPoint& pt);
    bool handleMouseMove(const QPoint& pt);
    bool handleMouseUp(const QPoint& pt);
    void handleResize(const QSize& sz);

private:
    std::string _strLabel;
    QRect _rect;
    QPoint _ptDown;
    bool _isEnabled = true;
    bool _mouseDown = false;

    TFLView* _target = nullptr;
    PaintHandler _paintHandler = nullptr;

    std::function<bool()> _enabledHandler;

    std::function<void(const QPoint&)> _mouseMoveListener;
    std::function<QRect(const QSize&)> _sizeListener;
};

#endif // MOUSEAREA_H
