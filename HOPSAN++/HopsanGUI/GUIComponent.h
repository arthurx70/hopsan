//$Id$

#ifndef GUICOMPONENT_H
#define GUICOMPONENT_H

#include <QGraphicsWidget>
//#include <QGraphicsSvgItem>
//#include <QGraphicsTextItem>
//#include <QWidget>
#include <QGraphicsView>
//#include "GUIConnector.h"
#include <vector>
//#include <QGraphicsItem>
#include "GUIComponentSelectionBox.h"


class GUIConnector;
class QGraphicsSvgItem;
class GUIComponentTextItem;
class HopsanEssentials;
class Component;
class GUIComponentSelectionBox;
class GUIPort;

class GUIComponent : public QGraphicsWidget
{
    Q_OBJECT
public:
    GUIComponent(HopsanEssentials *hopsan, const QString &fileName, QString componentName, QPoint position, QGraphicsView *parentView, QGraphicsItem *parent = 0);
    ~GUIComponent();
    QGraphicsView *getParentView();
    void addConnector(GUIConnector *item);

    //Core interaction
    Component *mpCoreComponent;
    //

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );

signals:
    void componentMoved();
    void componentDeleted();

public slots:
     void deleteComponent();

private:
    QGraphicsSvgItem *icon;
    GUIComponentTextItem *text;
    QGraphicsView *mpParentView;
    GUIComponentSelectionBox *mpSelectionBox;
    QGraphicsLineItem *mpTempLine;
    //std::vector<GUIConnector*> mConnectors;        //Inteded to store connectors for each component

    QList<GUIPort*> mPortListPtrs;

private slots:
    void fixTextPosition(QGraphicsSceneMouseEvent * event);

};


class GUIComponentTextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    GUIComponentTextItem(const QString &text, QGraphicsItem *parent = 0);

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

signals:
    void textMoved(QGraphicsSceneMouseEvent * event);

};

#endif // GUICOMPONENT_H
