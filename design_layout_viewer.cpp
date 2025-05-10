#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QtWidgets>

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <limits>
#include <gdstk/gdstk.hpp>

class LayoutWidget : public QWidget {
    Q_OBJECT

public:
    explicit LayoutWidget(QWidget *parent = nullptr);
    ~LayoutWidget();

    enum class InteractionMode {
        Normal,
        Zoom
    };

    void load_oasis(const QString& filename);
    void resetView();
    void setInteractionMode(InteractionMode mode);
    void toggleGrid();
    void setGridSpacing(double spacing);
    QVector<int> getAvailableLayers() const;
    void toggleLayerVisibility(int layer);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    // Zoom and pan variables
    double m_zoom_factor = 1.0;
    QPointF m_pan_offset{0, 0};
    QPointF m_last_mouse_pos;
    bool m_is_panning = false;

    // Zoom constants
    static constexpr double MIN_ZOOM = 0.1;
    static constexpr double MAX_ZOOM = 10.0;
    static constexpr double ZOOM_STEP = 1.2;

    gdstk::Library m_gdstk_lib;
    std::map<int, std::vector<gdstk::Polygon*>> m_layers;
    bool m_show_grid = false;
    double m_grid_spacing = 1.0;
    InteractionMode m_interaction_mode = InteractionMode::Normal;
    QPointF m_zoom_start_pos;
    QPointF m_zoom_current_pos;
    bool m_is_zooming = false;
    std::vector<gdstk::Polygon*> m_polygons_to_draw;
    QString m_current_file;
};

LayoutWidget::LayoutWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    // Zero-initialize the entire struct first to ensure Array members are safe
    memset(&m_gdstk_lib, 0, sizeof(gdstk::Library));
    m_gdstk_lib.init("", 0.0, 0.0); // Initialize gdstk::Library with an empty name
}

LayoutWidget::~LayoutWidget() {
    m_gdstk_lib.free_all();
}

void LayoutWidget::load_oasis(const QString& filename) {
    // Convert QString to std::string
    std::string filename_str = filename.toStdString();
    // Clear existing layers
    m_layers.clear();
    m_polygons_to_draw.clear();
    // Validate input filename
    if (filename_str.empty()) {
        QMessageBox::critical(this, "Load Error", "Empty filename");
        return;
    }

    // Check file existence and readability
    QFileInfo fileInfo(filename);
    if (!fileInfo.exists()) {
        qDebug() << "Error: File does not exist" << filename;
        QMessageBox::critical(this, "Load Error", QString("File does not exist: %1").arg(filename));
        return;
    }

    gdstk::Library temp_lib;
    gdstk::ErrorCode local_err_code = gdstk::ErrorCode::NoError;

    try {
        // Use read_oas with all parameters
        temp_lib = gdstk::read_oas(filename_str.c_str(), 0.0, 0.0, &local_err_code);
        qDebug() << "read_oas error code:" << static_cast<int>(local_err_code);

        if (local_err_code != gdstk::ErrorCode::NoError) {
            QString errorDetails = QString("Failed to read OASIS file: %1\nError code: %2").arg(filename).arg(static_cast<int>(local_err_code));
            QMessageBox::critical(this, "OASIS Load Error", errorDetails);
            return;
        }

        // Free existing library
        m_gdstk_lib.free_all();

        // Manually copy library properties to avoid potential string copy issues
        m_gdstk_lib.copy_from(temp_lib, true); // true for deep_copy
        temp_lib.free_all(); // Fully free temp_lib after its data has been copied or is no longer needed
        qDebug() << "OASIS file loaded successfully";

        m_polygons_to_draw.clear();
        m_layers.clear();
        
        // Find bounding box to center the view
        double min_x = std::numeric_limits<double>::max();
        double min_y = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double max_y = std::numeric_limits<double>::lowest();

        for (size_t i = 0; i < m_gdstk_lib.cell_array.count; ++i) {
            const gdstk::Cell* cell = m_gdstk_lib.cell_array[i];
            for (size_t j = 0; j < cell->polygon_array.count; ++j) {
                gdstk::Polygon* poly = cell->polygon_array[j];
                // Store polygons for drawing
                m_polygons_to_draw.push_back(poly);

                // Update bounding box
                for (size_t k = 0; k < poly->point_array.count; ++k) {
                    min_x = std::min(min_x, poly->point_array[k].x);
                    min_y = std::min(min_y, poly->point_array[k].y);
                    max_x = std::max(max_x, poly->point_array[k].x);
                    max_y = std::max(max_y, poly->point_array[k].y);
                }

                // Collect layers
                m_layers[j].push_back(poly);
            }
        }

        // Reset view to center on the loaded polygons
        double poly_width = max_x - min_x;
        double poly_height = max_y - min_y;
        double center_x = (min_x + max_x) / 2.0;
        double center_y = (min_y + max_y) / 2.0;

        m_pan_offset = QPointF(-center_x, -center_y);
        
        // Calculate zoom factor based on widget size
        double widget_width = width();
        double widget_height = height();
        m_zoom_factor = std::min(widget_width / poly_width, widget_height / poly_height) * 0.9;

        // Update the display
        update();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "OASIS Load Error", QString("Exception: %1").arg(e.what()));
    }
}

void LayoutWidget::resetView() {
    m_zoom_factor = 1.0;
    m_pan_offset = QPointF(0, 0);
    update();
}

void LayoutWidget::setInteractionMode(InteractionMode mode) {
    m_interaction_mode = mode;
}

void LayoutWidget::toggleGrid() {
    m_show_grid = !m_show_grid;
    update();
}

void LayoutWidget::setGridSpacing(double spacing) {
    m_grid_spacing = spacing;
    update();
}

QVector<int> LayoutWidget::getAvailableLayers() const {
    QSet<int> unique_layers;
    
    // Iterate through all cells in the library
    for (size_t i = 0; i < m_gdstk_lib.cell_array.count; ++i) {
        const gdstk::Cell* cell = m_gdstk_lib.cell_array[i];
        
        // Collect unique layers from polygons
        for (size_t j = 0; j < cell->polygon_array.count; ++j) {
            if (cell->polygon_array[j]) {
                // Manually extract layer information
                int layer_num = -1;
                for (size_t k = 0; k < cell->polygon_array[j]->point_array.count; ++k) {
                    // Use a unique identifier or index as a layer
                    layer_num = j;
                    break;
                }
                if (layer_num != -1) {
                    unique_layers.insert(layer_num);
                }
            }
        }
    }
    
    // Convert to sorted QVector
    QVector<int> layers;
    for (int layer : unique_layers) {
        layers.push_back(layer);
    }
    std::sort(layers.begin(), layers.end());
    
    return layers;
}

void LayoutWidget::toggleLayerVisibility(int layer) {
    // Placeholder for layer visibility toggle
    // You might want to implement actual layer visibility logic
    update();
}

void LayoutWidget::wheelEvent(QWheelEvent *event) {
    // Compute the mouse position relative to the widget center
    QPointF mouse_pos = event->position();
    QPointF center(width() / 2.0, height() / 2.0);
    
    // Map mouse position to scene coordinates
    QPointF mouse_scene_pos = (mouse_pos - center) / m_zoom_factor - m_pan_offset;
    
    // Adjust zoom based on wheel delta
    double zoom_delta = event->angleDelta().y() / 120.0; // Normalize to increments
    double zoom_factor_old = m_zoom_factor;
    
    // Compute new zoom factor with exponential zoom
    m_zoom_factor *= std::pow(ZOOM_STEP, zoom_delta);
    
    // Clamp zoom factor
    m_zoom_factor = std::max(MIN_ZOOM, std::min(m_zoom_factor, MAX_ZOOM));
    
    // Compute new pan offset to keep mouse point stable
    QPointF mouse_scene_pos_new = (mouse_pos - center) / m_zoom_factor - m_pan_offset;
    m_pan_offset += (mouse_scene_pos - mouse_scene_pos_new);
    
    // Trigger a redraw
    update();
}

void LayoutWidget::mousePressEvent(QMouseEvent *event) {
    // Store initial mouse position in scene coordinates
    QPointF center(width() / 2.0, height() / 2.0);
    m_last_mouse_pos = (event->localPos() - center) / m_zoom_factor - m_pan_offset;

    if (event->button() == Qt::LeftButton) {
        m_is_panning = true;
    } else if (event->button() == Qt::RightButton) {
        m_is_zooming = true;
        m_zoom_start_pos = event->localPos();
        m_zoom_current_pos = event->localPos();
    }
}

void LayoutWidget::mouseMoveEvent(QMouseEvent *event) {
    QPointF center(width() / 2.0, height() / 2.0);
    QPointF current_mouse_pos = (event->localPos() - center) / m_zoom_factor - m_pan_offset;

    if (m_is_panning) {
        // Compute pan delta in scene coordinates
        QPointF pan_delta = current_mouse_pos - m_last_mouse_pos;
        m_pan_offset += pan_delta;
        m_last_mouse_pos = current_mouse_pos;
        update();
    }

    if (m_is_zooming) {
        // Update zoom rectangle
        m_zoom_current_pos = event->localPos();
        update();
    }
}

void LayoutWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // End panning
        m_is_panning = false;
    } else if (event->button() == Qt::RightButton) {
        // End zooming
        if (m_is_zooming) {
            // Compute zoom rectangle
            QRectF zoom_rect = QRectF(m_zoom_start_pos, m_zoom_current_pos).normalized();
            
            // Only zoom if the rectangle is large enough
            if (zoom_rect.width() > 10 && zoom_rect.height() > 10) {
                // Compute new zoom based on the zoom rectangle
                double zoom_width = width() / zoom_rect.width();
                double zoom_height = height() / zoom_rect.height();
                
                // Use the smaller zoom factor to fit the rectangle
                m_zoom_factor *= std::min(zoom_width, zoom_height);
                
                // Compute new pan offset to center the zoomed area
                QPointF center(width() / 2.0, height() / 2.0);
                QPointF zoom_center = zoom_rect.center();
                QPointF scene_zoom_center = (zoom_center - center) / m_zoom_factor - m_pan_offset;
                m_pan_offset -= scene_zoom_center;
                
                // Clamp zoom factor
                m_zoom_factor = std::max(MIN_ZOOM, std::min(m_zoom_factor, MAX_ZOOM));
            }
            
            m_is_zooming = false;
            update();
        }
    }
}

void LayoutWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw grid if enabled
    if (m_show_grid) {
        QPen gridPen(Qt::lightGray, 0.5);
        painter.setPen(gridPen);

        double gridSize = m_grid_spacing * m_zoom_factor;
        QPointF offset = m_pan_offset * m_zoom_factor + QPointF(width()/2, height()/2);

        // Horizontal lines
        for (double y = fmod(offset.y(), gridSize); y < height(); y += gridSize) {
            painter.drawLine(0, y, width(), y);
        }

        // Vertical lines
        for (double x = fmod(offset.x(), gridSize); x < width(); x += gridSize) {
            painter.drawLine(x, 0, x, height());
        }
    }

    // Draw polygons
    if (m_polygons_to_draw.empty()) {
        // Draw a placeholder or message
        painter.setPen(QPen(Qt::red, 2));
        painter.drawText(rect(), Qt::AlignCenter, "No polygons loaded");
        return;
    }

    // Find global bounding box to normalize polygon drawing
    double min_x = std::numeric_limits<double>::max();
    double min_y = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double max_y = std::numeric_limits<double>::lowest();

    for (const auto& poly : m_polygons_to_draw) {
        if (poly && poly->point_array.count > 2) {
            for (size_t j = 0; j < poly->point_array.count; ++j) {
                min_x = std::min(min_x, poly->point_array[j].x);
                min_y = std::min(min_y, poly->point_array[j].y);
                max_x = std::max(max_x, poly->point_array[j].x);
                max_y = std::max(max_y, poly->point_array[j].y);
            }
        }
    }

    // Prepare drawing
    double line_width = 1.0 / m_zoom_factor;
    painter.setPen(QPen(Qt::blue, line_width));
    painter.setBrush(QColor(0, 0, 255, 50)); // Semi-transparent blue fill

    // Draw each polygon
    for (const auto& poly : m_polygons_to_draw) {
        if (poly && poly->point_array.count > 2) {
            QPolygonF qpoly;
            for (size_t j = 0; j < poly->point_array.count; ++j) {
                QPointF point(poly->point_array[j].x, poly->point_array[j].y);
                
                // Normalize point to view
                point = (point * m_zoom_factor) + m_pan_offset * m_zoom_factor + QPointF(width()/2, height()/2);
                qpoly << point;
            }
            painter.drawPolygon(qpoly);
        }
    }

    // Draw zoom rectangle if zooming
    if (m_is_zooming) {
        painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
        painter.setBrush(QColor(255, 0, 0, 50)); // Semi-transparent red
        QRectF zoom_rect = QRectF(m_zoom_start_pos, m_zoom_current_pos).normalized();
        painter.drawRect(zoom_rect);
    }

    // Debug info
    painter.setPen(Qt::black);
    painter.drawText(10, 20, QString("Polygons: %1").arg(m_polygons_to_draw.size()));
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QMainWindow main_window;
    LayoutWidget* widget = new LayoutWidget(&main_window);
    main_window.setCentralWidget(widget);
    
    // Create toolbar
    QToolBar* toolbar = main_window.addToolBar("Main Toolbar");
    
    // Zoom mode action
    QAction* zoom_action = new QAction("Zoom Mode", &main_window);
    zoom_action->setCheckable(true);
    zoom_action->setChecked(false);
    zoom_action->setToolTip("Activate Zoom Mode (Left-click and drag to zoom)\nRight/drag up-left to zoom out, right/drag down-right to zoom in");
    
    // Grid toggle action
    QAction* grid_action = new QAction("Toggle Grid", &main_window);
    grid_action->setCheckable(true);
    grid_action->setChecked(false);
    grid_action->setToolTip("Show/Hide Grid");
    
    // Connect zoom mode toggle
    QObject::connect(zoom_action, &QAction::toggled, widget, [widget](bool checked) {
        widget->setInteractionMode(checked ? LayoutWidget::InteractionMode::Zoom : LayoutWidget::InteractionMode::Normal);
    });
    
    // Connect grid toggle
    QObject::connect(grid_action, &QAction::toggled, widget, [widget](bool) {
        widget->toggleGrid();
    });
    
    toolbar->addAction(zoom_action);
    toolbar->addAction(grid_action);
    
    main_window.show();
    
    // Try to load an OASIS file if provided as an argument
    if (argc > 1) {
        widget->load_oasis(QString::fromUtf8(argv[1]));
    }
    
    return app.exec();
}

#include "design_layout_viewer.moc"
