#ifndef FLATBUTTONMANAGER_H
#define FLATBUTTONMANAGER_H


#include <QSize>
#include <QObject>
#include <QPainter>
#include <unordered_map>
#include <QLatin1String>


class FlatButtonManager : public QObject
{
    Q_OBJECT

public:
    enum VAlignment
    {
        AlignTop,
        AlignBottom,
    };
    enum HAlignment
    {
        AlignLeft,
        AlignMiddle,
        AlignRight,
    };

private:
    struct button
    {
        float U, V;
        QRect rect;
        QRect fingerRect;
        QImage opaqueImg;
        QImage img;
        QImage invImg;
        float rot = 0.0f;
        bool bVis = true;
        VAlignment vAlign = VAlignment::AlignTop;
        HAlignment hAlign = HAlignment::AlignMiddle;
        bool bObeySize = true;
    };

public:
    void addButton(QLatin1String id, float u, float v, VAlignment a, bool bVis=true);
    void paint(QPainter& p, bool bInverted, bool bOpaque=false);

    void handleIgnore();

    bool handleMouseDown(const QPoint& pt);
    bool handleMouseMove(const QPoint& pt);
    bool handleMouseUp(const QPoint& pt);

    void setObeySize(QLatin1String id, bool bObey);
    void setHAlign(QLatin1String id, HAlignment a);
    void setImage(QLatin1String id, QString filename, QColor transparent);
    void setText(QLatin1String id, QPainter &p, QString text, QColor penColor, QColor transparent);
    void setSize(QSize sz, QPoint topOffset, QPoint bottomOffset);
    void setImgRot(QLatin1String id, float f);
    void setImgVisibility(QLatin1String id, bool bVis);

    bool getImgVisibility(QLatin1String id) const;

    void setVisibility(bool);
    bool getVisibility() const;

protected:
    void setButtonProperties(button& but, QImage& img, QColor transparent);
    void updateButtonUV(button& b);
    QRect fromUV(float U1, float V1, float U2, float V2);
    QLatin1String buttonFromPt(const QPoint& pt);

signals:
    void buttonPressed(QString id);

private:
    QSize _sz;
    std::map<QLatin1String, button> _buttons;
    QLatin1String _buttonDown;
    bool _bHover = false;
    QPoint _topOffset;
    QPoint _bottomOffset;
    bool _bVisible = true;
};



#endif // FLATBUTTONMANAGER_H
