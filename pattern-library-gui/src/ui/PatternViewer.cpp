// filepath: /pattern-library-gui/src/ui/PatternViewer.cpp

#include "PatternViewer.h"
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QtMath> // Added for qFloor

PatternViewer::PatternViewer(QWidget *parent)
    : QWidget(parent)
    , m_layer_patterns() // Initialize m_layer_patterns
    , m_scale(1.0)
    , m_panning(false)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void PatternViewer::setPattern(const QMap<int, QList<QPolygonF>>& layer_patterns)
{
    m_layer_patterns = layer_patterns;
    m_layer_visibility.clear(); // Clear previous visibility states
    if (!m_layer_patterns.isEmpty()) {
        for (int layerNum : m_layer_patterns.keys()) {
            m_layer_visibility.insert(layerNum, true); // Initially all layers visible
        }
        QRectF totalBounds;
        for (const QList<QPolygonF>& poly_list : m_layer_patterns.values()) {
            for (const QPolygonF &poly : poly_list) {
                if (totalBounds.isNull()) {
                    totalBounds = poly.boundingRect();
                } else {
                    totalBounds = totalBounds.united(poly.boundingRect());
                }
            }
        }
        m_patternBounds = totalBounds;
        resetView(); // This will also call update()
    } else {
        m_patternBounds = QRectF(); // Reset bounds if no patterns
        m_layer_patterns.clear(); // Ensure it's clear
        // m_layer_visibility is already cleared
        update(); // Request a repaint to clear the view
    }
}

void PatternViewer::zoomIn()
{
    m_scale *= 1.2;
    updateTransform();
    update();
}

void PatternViewer::zoomOut()
{
    m_scale /= 1.2;
    updateTransform();
    update();
}

void PatternViewer::resetView()
{
    if (m_layer_patterns.isEmpty() || !m_patternBounds.isValid() || m_patternBounds.isEmpty())
        return;

    QRectF viewRect = rect();
    if (viewRect.isEmpty() || m_patternBounds.width() == 0 || m_patternBounds.height() == 0) {
        m_scale = 1.0;
        m_pan = QPointF(viewRect.width()/2, viewRect.height()/2);
    } else {
        qreal xScale = viewRect.width() / m_patternBounds.width();
        qreal yScale = viewRect.height() / m_patternBounds.height();
        m_scale = qMin(xScale, yScale) * 0.9;

        QPointF center = m_patternBounds.center();
        m_pan = QPointF(viewRect.width()/2, viewRect.height()/2) - center * m_scale;
    }

    updateTransform();
    update();
}

void PatternViewer::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    QPen gridPen(Qt::lightGray, 0);
    painter.setPen(gridPen);
    
    qreal gridStep = 50.0 / m_scale; // Adjust grid step based on zoom
    QPointF topLeft = mapToScene(QPoint(0, 0));
    QPointF bottomRight = mapToScene(QPoint(width(), height()));
    
    // Draw grid lines
    for (qreal x = qFloor(topLeft.x()/gridStep)*gridStep; x < bottomRight.x(); x += gridStep) {
        painter.drawLine(mapFromScene(QPointF(x, topLeft.y())), 
                        mapFromScene(QPointF(x, bottomRight.y())));
    }
    for (qreal y = qFloor(topLeft.y()/gridStep)*gridStep; y < bottomRight.y(); y += gridStep) {
        painter.drawLine(mapFromScene(QPointF(topLeft.x(), y)),
                        mapFromScene(QPointF(bottomRight.x(), y)));
    }

    if (!m_layer_patterns.isEmpty()) {
        painter.setTransform(m_transform);
        QList<QColor> layerColors = {
            QColor(200, 200, 255, 150), // Light Blue
            QColor(255, 200, 200, 150), // Light Red
            QColor(200, 255, 200, 150), // Light Green
            QColor(255, 255, 200, 150), // Light Yellow
            QColor(200, 255, 255, 150), // Light Cyan
            QColor(255, 200, 255, 150)  // Light Magenta
        };
        int colorIndex = 0;
        for (auto it = m_layer_patterns.constBegin(); it != m_layer_patterns.constEnd(); ++it) {
            int layerNum = it.key(); // Get current layer number
            const QList<QPolygonF>& poly_list = it.value();
            
            QColor baseColor = layerColors[colorIndex % layerColors.size()];
            painter.setPen(QPen(baseColor.darker(150), 0)); 
            painter.setBrush(QBrush(baseColor));
            
            if (m_layer_visibility.value(layerNum, true)) { // Check visibility, default to true
                for (const QPolygonF &poly : poly_list) {
                    painter.drawPolygon(poly);
                }
            }
            colorIndex++;
        }
    }
}

void PatternViewer::wheelEvent(QWheelEvent *event)
{
    QPointF scenePos = mapToScene(event->position().toPoint());
    
    if (event->angleDelta().y() > 0)
        m_scale *= 1.2;
    else
        m_scale /= 1.2;

    updateTransform();
    
    // Adjust pan to keep mouse position fixed
    QPointF newScenePos = mapToScene(event->position().toPoint());
    m_pan += (newScenePos - scenePos) * m_scale;
    updateTransform();
    
    update();
}

void PatternViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton) {
        m_panning = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void PatternViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_pan += delta;
        updateTransform();
        update();
        m_lastMousePos = event->pos();
    }
}

void PatternViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void PatternViewer::updateTransform()
{
    m_transform = QTransform();
    m_transform.translate(m_pan.x(), m_pan.y());
    m_transform.scale(m_scale, m_scale);
}

QPointF PatternViewer::mapToScene(const QPoint &pos) const
{
    return m_transform.inverted().map(QPointF(pos));
}

QPoint PatternViewer::mapFromScene(const QPointF &pos) const
{
    return m_transform.map(pos).toPoint();
}

void PatternViewer::setLayerVisibility(int layerNum, bool visible)
{
    if (m_layer_visibility.contains(layerNum)) {
        m_layer_visibility[layerNum] = visible;
        update(); // Request a repaint
    }
}

void PatternViewer::setAllLayersVisibility(bool visible)
{
    for (auto it = m_layer_visibility.begin(); it != m_layer_visibility.end(); ++it) {
        it.value() = visible;
    }
    update(); // Request a repaint
}