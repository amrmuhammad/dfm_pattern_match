// filepath: /pattern-library-gui/src/ui/PatternViewer.h

#ifndef PATTERNVIEWER_H
#define PATTERNVIEWER_H

#include <QWidget>
#include <QMap>
#include <QPolygonF>
#include <QList> // Added for QList
#include <QTransform>

class PatternViewer : public QWidget {
    Q_OBJECT

public:
    explicit PatternViewer(QWidget *parent = nullptr);

public slots:
    void setPattern(const QMap<int, QList<QPolygonF>>& layer_patterns); 
    void zoomIn();
    void zoomOut();
    void resetView();
    void setLayerVisibility(int layerNum, bool visible);
    void setAllLayersVisibility(bool visible);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void updateTransform();
    QPointF mapToScene(const QPoint &pos) const;
    QPoint mapFromScene(const QPointF &pos) const;

    QMap<int, QList<QPolygonF>> m_layer_patterns; // Changed to a list of polygons
    QMap<int, bool> m_layer_visibility; // Stores visibility state for each layer
    QTransform m_transform;
    qreal m_scale;
    QPointF m_pan;
    bool m_panning;
    QPoint m_lastMousePos;
    QRectF m_patternBounds;
};

#endif // PATTERNVIEWER_H