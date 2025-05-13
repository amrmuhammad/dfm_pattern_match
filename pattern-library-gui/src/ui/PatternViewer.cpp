// filepath: /pattern-library-gui/src/ui/PatternViewer.cpp

#include "PatternViewer.h"
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QtMath> // Added for qFloor

PatternViewer::PatternViewer(QWidget *parent)
    : QWidget(parent)
    , m_scale(1.0)
    , m_panning(false)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void PatternViewer::setPattern(const QPolygonF &pattern)
{
    m_pattern = pattern;
    if (!pattern.isEmpty()) {
        m_patternBounds = pattern.boundingRect();
        resetView();
    }
    update();
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
    if (m_pattern.isEmpty())
        return;

    // Calculate scale to fit pattern in view with some margin
    QRectF viewRect = rect();
    qreal xScale = viewRect.width() / m_patternBounds.width();
    qreal yScale = viewRect.height() / m_patternBounds.height();
    m_scale = qMin(xScale, yScale) * 0.9;

    // Center the pattern
    QPointF center = m_patternBounds.center();
    m_pan = QPointF(viewRect.width()/2, viewRect.height()/2) - center * m_scale;

    updateTransform();
    update();
}

void PatternViewer::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fill background
    painter.fillRect(rect(), Qt::white);

    // Draw grid
    QPen gridPen(Qt::lightGray, 0);
    painter.setPen(gridPen);
    
    qreal gridStep = 50.0 / m_scale;
    QPointF topLeft = mapToScene(QPoint(0, 0));
    QPointF bottomRight = mapToScene(QPoint(width(), height()));
    
    for (qreal x = qFloor(topLeft.x()/gridStep)*gridStep; x < bottomRight.x(); x += gridStep) {
        painter.drawLine(mapFromScene(QPointF(x, topLeft.y())), 
                        mapFromScene(QPointF(x, bottomRight.y())));
    }
    for (qreal y = qFloor(topLeft.y()/gridStep)*gridStep; y < bottomRight.y(); y += gridStep) {
        painter.drawLine(mapFromScene(QPointF(topLeft.x(), y)),
                        mapFromScene(QPointF(bottomRight.x(), y)));
    }

    // Draw pattern
    if (!m_pattern.isEmpty()) {
        painter.setPen(QPen(Qt::blue, 0));
        painter.setBrush(QBrush(QColor(200, 200, 255, 100)));
        painter.setTransform(m_transform);
        painter.drawPolygon(m_pattern);
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