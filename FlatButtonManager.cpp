#include "FlatButtonManager.h"


QPoint toPixelSpace(const QSize& sz, float u, float v)
{
    return QPoint( sz.width() * u , sz.height() * v);
}

void FlatButtonManager::addButton(QLatin1String id, float u, float v, VAlignment a, bool bVis)
{
    button& but = _buttons[id];

    but.U = u;
    but.V = v;
    but.bVis = bVis;
    but.vAlign = a;

    updateButtonUV(but);
}

void FlatButtonManager::paint(QPainter &p, bool bInverted, bool bOpaque)
{
    if( !getVisibility())
        return;

    const int shift = 2;
    button& butDown = _buttons[_buttonDown];
    QPen pen;
    pen.setColor(Qt::darkGray);
    pen.setWidth(shift);
    QPen oldPen = p.pen();
    p.setPen(pen);

    for(auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        button& b =it->second;
        if( !b.bVis)
            continue;

        const QImage& img = bOpaque ? b.opaqueImg : (bInverted ? b.img : b.invImg);

        QTransform t;
        t.rotate(b.rot);

        bool bHoverPressed = &b == &butDown && _bHover;

        QRect rc = b.rect;

        if(bHoverPressed)
            rc.adjust(shift,shift,shift,shift);

        p.drawImage( rc, img.transformed(t));

        if( bHoverPressed)
        {
            p.drawLine(rc.left(), rc.top()-1, rc.right(), rc.top()-1);
            p.drawLine(rc.left(), rc.top()-1, rc.left(), rc.bottom());
        }
    }

    p.setPen(oldPen);
}

void FlatButtonManager::handleIgnore()
{
    _bHover = false;
    _buttonDown = QLatin1String("");
}

bool FlatButtonManager::handleMouseDown(const QPoint &pt)
{
    if( !getVisibility())
        return false;

    _buttonDown = buttonFromPt(pt);
    _bHover = true;
    return _buttonDown.size() > 0;
}

bool FlatButtonManager::handleMouseMove(const QPoint &pt)
{
    if( !getVisibility())
        return false;

    _bHover = buttonFromPt(pt) == _buttonDown;

    return _buttonDown.size() > 0;
}

bool FlatButtonManager::handleMouseUp(const QPoint &pt)
{
    if( !getVisibility())
    {
        _buttonDown = QLatin1String("");
        return false;
    }

    bool bHandled(false);

    if( _buttonDown.size() > 0 && _bHover)
    {
        emit buttonPressed(_buttonDown);
        bHandled = true;
    }

    _bHover = false;
    _buttonDown = QLatin1String("");
    return bHandled;
}

void FlatButtonManager::setObeySize(QLatin1String id, bool bObey)
{
    button& but = _buttons[id];
    but.bObeySize = bObey;
    updateButtonUV(but);
}

void FlatButtonManager::setHAlign(QLatin1String id, FlatButtonManager::HAlignment a)
{
    button& but = _buttons[id];
    but.hAlign = a;
    updateButtonUV(but);
}

void FlatButtonManager::setImage(QLatin1String id, QString filename, QColor transparent)
{
    button& but = _buttons[id];

    QImage img;
    img.load(filename);
    img = img.convertToFormat(QImage::Format_ARGB32);
    setButtonProperties(but, img, transparent);
}

void FlatButtonManager::updateButtonUV(FlatButtonManager::button &b)
{
    int w = b.img.width();
    int h = b.img.height();

    b.rect = fromUV(b.U, b.V, 0,0);

    float yTopOffset = _topOffset.y();
    float yBottomOffset = _bottomOffset.y();

    if( !b.bObeySize)
    {
        yTopOffset = 0.0f;
        yBottomOffset = 0.0f;
    }

    if( b.vAlign == AlignTop)
        b.rect.adjust(_topOffset.x(), yTopOffset, 0,0);
    else
        b.rect.adjust(_bottomOffset.x(), yBottomOffset-h, 0,0);

    b.rect.setRect(b.rect.x(), b.rect.y(), w, h);

    int adjust = 0;

    if(b.hAlign == AlignMiddle )
        adjust = -b.img.width()/2;
    else if( b.hAlign == AlignRight)
        adjust = -b.img.width();

    b.rect.adjust(adjust, 0, adjust, 0);

    b.fingerRect = b.rect;
    b.fingerRect.adjust(-10,-10, 10, 10);
}

QRect FlatButtonManager::fromUV(float U1, float V1, float U2, float V2)
{
    QPoint ptA = toPixelSpace(_sz, U1, V1);
    QPoint ptB = toPixelSpace(_sz, U2, V2);

    return QRect(ptA, ptB);
}

QLatin1String FlatButtonManager::buttonFromPt(const QPoint &pt)
{
    for(auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        button& b = it->second;

        if( !b.bVis)
            continue;

        if( b.fingerRect.contains(pt))
            return it->first;
    }

    return QLatin1String("");
}

void FlatButtonManager::setSize(QSize sz, QPoint topOffset, QPoint bottomOffset)
{
    _sz = sz;
    _topOffset = topOffset;
    _bottomOffset = bottomOffset;
    for(auto it = _buttons.begin(); it != _buttons.end(); ++it)
        updateButtonUV(it->second);
}

void FlatButtonManager::setImgRot(QLatin1String id, float f)
{
    button& but = _buttons[id];
    but.rot = f;
}

void FlatButtonManager::setImgVisibility(QLatin1String id, bool bVis)
{
    button& but = _buttons[id];
    but.bVis = bVis;
}

bool FlatButtonManager::getImgVisibility(QLatin1String id) const
{
    auto it = _buttons.find(id);
    if( it == _buttons.end())
        return false;

    return it->second.bVis;
}

void FlatButtonManager::setText(QLatin1String id, QPainter &p, QString text, QColor penColor, QColor transparent)
{
    QFontMetrics fm = p.fontMetrics();
    const int border = 5;
    const int w = fm.horizontalAdvance(text) + border;
    const int h = fm.height() + border;

    QImage img(w, h, QImage::Format_ARGB32);

    QPainter m;
    m.begin(&img);
    m.setBackgroundMode(Qt::TransparentMode);
    m.setBackground(Qt::white);
    m.setPen(penColor);
    m.setFont(p.font());
    m.fillRect(0,0,w,h, transparent);
    QRect rc(0,0,w,h);
    rc.adjust(border/2, border/2, 0,0);
    m.drawText(rc, text);
    m.end();

    button& but = _buttons[id];
    setButtonProperties(but, img, transparent);
}

void FlatButtonManager::setVisibility(bool v)
{
    _bVisible = v;
}

bool FlatButtonManager::getVisibility() const
{
    return _bVisible;
}

void FlatButtonManager::setButtonProperties(FlatButtonManager::button &but, QImage &img, QColor transparent)
{
    QPainter p;
    p.begin(&img);
    p.setPen(Qt::black);
    QRect rc = img.rect();
    rc.adjust(0,0,-1,-1);
    p.drawRect(rc);
    p.end();
    but.opaqueImg = img;

    for(int x= 0; x < img.width(); ++x)
        for( int y = 0; y < img.height(); ++y)
            if( img.pixelColor(x,y) == transparent)
            {
                img.setPixelColor(x, y, QColor(0,0,0,0));
            }
            else
            {   QColor c = img.pixelColor(x,y);
                c.setAlpha(192);
                img.setPixelColor(x, y, c);
            }

    but.img = img;

    updateButtonUV(but);

    for(int x = 0; x < img.width(); ++x)
        for( int y = 0; y < img.height(); ++y)
        {
            QColor c = img.pixelColor(x,y);
            c = QColor(255-c.red(), 255-c.green(), 255-c.blue(), c.alpha());
            img.setPixelColor(x, y, c);
        }

    but.invImg = img;
}
