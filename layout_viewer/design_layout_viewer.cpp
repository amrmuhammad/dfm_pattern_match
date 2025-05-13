#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QWidget>
#include <limits>
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

#include "layoutwidget.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex> // For thread-safe logging

// Global mutex for thread-safe file writing
QMutex logMutex;
// Global QFile to keep it open
QFile logFile;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QMutexLocker locker(&logMutex); // Lock for thread safety
    if (!logFile.isOpen()) { // Open file on first use, ensures it's not opened multiple times
        logFile.setFileName("debug_log.txt");
        if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            // Cannot open file, so write to stderr instead
            fprintf(stderr, "Error opening log file: %s\n", qPrintable(logFile.errorString()));
            fprintf(stderr, "Log message: %s (%s:%u, %s)\n", qPrintable(msg), context.file, context.line, context.function);
            return;
        }
    }

    QTextStream out(&logFile);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    out << timestamp << " ";

    switch (type) {
    case QtDebugMsg:
        out << "[Debug] ";
        break;
    case QtInfoMsg:
        out << "[Info] ";
        break;
    case QtWarningMsg:
        out << "[Warning] ";
        break;
    case QtCriticalMsg:
        out << "[Critical] ";
        break;
    case QtFatalMsg:
        out << "[Fatal] ";
        break;
    }

    out << msg;
    if (context.file) { // Only add file/line if available
         out << " (" << context.file << ":" << context.line << ", " << context.function << ")";
    }
    out << Qt::endl;
    out.flush(); // Ensure data is written immediately
}

QPointF LayoutWidget::computeSceneCoordinates(const QPointF& screen_pos) const {
    // Convert screen coordinates to scene coordinates
    // Compute widget center and apply zoom and pan transformations
    QPointF center(width() / 2.0, height() / 2.0);
    
    // Correctly apply inverse transformations: 
    // 1. Translate screen_pos relative to widget center
    // 2. Scale by inverse zoom (y-zoom is inverted in paintEvent)
    // 3. Translate by inverse pan_offset
    double intermediate_x = (screen_pos.x() - center.x()) / m_zoom_factor;
    double intermediate_y = (screen_pos.y() - center.y()) / (-m_zoom_factor); // Use -m_zoom_factor for y

    QPointF scene_pos(
        intermediate_x - m_pan_offset.x(),
        intermediate_y - m_pan_offset.y()
    );

    // Enhanced debug logging with more context
    qDebug() << "=== Screen to Scene Coordinate Conversion ==="
             << "\n  Input Screen Position: " << screen_pos
             << "\n  Widget Center: " << center
             << "\n  Computed Scene Position: " << scene_pos
             << "\n  Current Zoom Factor: " << m_zoom_factor
             << "\n  Current Pan Offset: " << m_pan_offset
             << "\n  Widget Dimensions: " << QSize(width(), height());

    return scene_pos;
}

LayoutWidget::ZoomParameters LayoutWidget::computeZoomParameters(const QRectF& zoom_rect) {
    // Comprehensive zoom parameter computation
    
    // Validate zoom rectangle size
    const double MIN_ZOOM_RECT_SIZE = 10.0;
    if (zoom_rect.width() < MIN_ZOOM_RECT_SIZE || zoom_rect.height() < MIN_ZOOM_RECT_SIZE) {
        qDebug() << "=== Zoom Rectangle Validation Failed ===" 
                 << "\n  Zoom Rectangle: " << zoom_rect
                 << "\n  Rectangle Width: " << zoom_rect.width()
                 << "\n  Rectangle Height: " << zoom_rect.height()
                 << "\n  Minimum Size Required: " << MIN_ZOOM_RECT_SIZE;
        return {m_zoom_factor, m_pan_offset, false};
    }

    // Compute scene coordinates of zoom rectangle corners
    QPointF scene_start = computeSceneCoordinates(zoom_rect.topLeft());
    QPointF scene_end = computeSceneCoordinates(zoom_rect.bottomRight());

    // Compute scene dimensions of the zoom rectangle
    double scene_rect_width = fabs(scene_end.x() - scene_start.x());
    double scene_rect_height = fabs(scene_end.y() - scene_start.y());

    // Avoid division by zero if the scene rectangle has no area
    const double MIN_SCENE_DIM = 1e-6; // Minimum dimension in scene units
    if (scene_rect_width < MIN_SCENE_DIM || scene_rect_height < MIN_SCENE_DIM) {
        qDebug() << "Zoom rectangle has zero or near-zero area in scene coordinates."
                 << "\n  Scene Rect Width: " << scene_rect_width
                 << "\n  Scene Rect Height: " << scene_rect_height;
        return {m_zoom_factor, m_pan_offset, false}; // Keep current zoom, indicate invalid
    }

    // Compute zoom factors for width and height based on SCENE dimensions
    double zoom_width = width() / scene_rect_width;
    double zoom_height = height() / scene_rect_height;
    
    // Choose the more constrained zoom factor to fit entire rectangle
    double new_zoom_factor = std::min(zoom_width, zoom_height);
    
    // Clamp zoom factor within predefined limits
    new_zoom_factor = std::max(MIN_ZOOM, std::min(new_zoom_factor, MAX_ZOOM));

    // Compute new pan offset to center the zoomed area
    QPointF scene_zoom_center = (scene_start + scene_end) / 2.0;
    QPointF new_pan_offset = QPointF(-scene_zoom_center.x(), -scene_zoom_center.y());

    // Comprehensive debug logging
    qDebug() << "=== Zoom Parameters Computation ===" 
             << "\n  Input Zoom Rectangle (Screen): " << zoom_rect
             << "\n  Scene Start Coordinate: " << scene_start
             << "\n  Scene End Coordinate: " << scene_end
             << "\n  Scene Rect Width: " << scene_rect_width
             << "\n  Scene Rect Height: " << scene_rect_height
             << "\n  Widget Width: " << width() << " Widget Height: " << height()
             << "\n  Current Zoom Factor: " << m_zoom_factor
             << "\n  Computed Zoom Width (widget/scene_w): " << zoom_width
             << "\n  Computed Zoom Height (widget/scene_h): " << zoom_height
             << "\n  Chosen New Zoom Factor (clamped): " << new_zoom_factor
             << "\n  Current Pan Offset: " << m_pan_offset
             << "\n  Scene Zoom Center: " << scene_zoom_center
             << "\n  Computed New Pan Offset: " << new_pan_offset;

    return {
        new_zoom_factor,
        new_pan_offset, // Use the new_pan_offset
        true
    };
}

// Constructor for LayoutWidget
LayoutWidget::LayoutWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(400, 300);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    // Zero-initialize the entire struct first to ensure Array members are safe
    memset(&m_gdstk_lib, 0, sizeof(gdstk::Library));
    m_gdstk_lib.init("", 0.0, 0.0); // Initialize gdstk::Library with an empty name

    // Connect signals
    // connect(this, &LayoutWidget::selectionChanged, this, &LayoutWidget::updateSelectionStatus); // Removed: updateSelectionStatus is gone
    // connect(this, &LayoutWidget::polygonInfoUpdated, m_polygon_info_label, &QLabel::setText); // Removed: m_polygon_info_label is gone
}

// Selects polygons within the given rectangle using the specified modifier
void LayoutWidget::selectPolygons(const QRectF& selection_rect, SelectionModifier modifier) {
    // Compute scene coordinates of selection rectangle
    QRectF scene_selection_rect(computeSceneCoordinates(selection_rect.topLeft()),
                                 computeSceneCoordinates(selection_rect.bottomRight()));
    scene_selection_rect = scene_selection_rect.normalized();

    // Perform selection based on modifier
    switch (modifier) {
        case SelectionModifier::Replace:
            m_selected_polygons.clear();
            break;
        case SelectionModifier::Subtract:
            { // Start new scope for 'to_remove'
                // Temporary vector to store polygons to remove
                std::vector<gdstk::Polygon*> to_remove;
                for (const auto& poly : m_selected_polygons) {
                    if (isPolygonInSelection(poly)) { 
                        to_remove.push_back(poly);
                    }
                }
                // Remove selected polygons
                m_selected_polygons.erase(
                    std::remove_if(m_selected_polygons.begin(), m_selected_polygons.end(),
                        [&to_remove](const gdstk::Polygon* poly) {
                            return std::find(to_remove.begin(), to_remove.end(), poly) != to_remove.end();
                        }),
                    m_selected_polygons.end()
                );
            } // End scope for 'to_remove'
            break;
        case SelectionModifier::Add:
            // Logic is handled in the loop below
            break;
        case SelectionModifier::Toggle:
            // Logic is handled in the loop below
            break;
        default:
            qDebug() << "Warning: Unexpected SelectionModifier:" << static_cast<int>(modifier);
            break;
    }

    // Add or toggle polygons
    for (const auto& poly : m_polygons_to_draw) {
        if (isPolygonInSelection(poly)) {
            if (modifier == SelectionModifier::Toggle) {
                auto it = std::find(m_selected_polygons.begin(), m_selected_polygons.end(), poly);
                if (it != m_selected_polygons.end()) {
                    m_selected_polygons.erase(it);
                } else {
                    m_selected_polygons.push_back(poly);
                }
            } else if (modifier == SelectionModifier::Add || modifier == SelectionModifier::Replace) {
                // Only add if not already in selection
                if (std::find(m_selected_polygons.begin(), m_selected_polygons.end(), poly) == m_selected_polygons.end()) {
                    m_selected_polygons.push_back(poly);
                }
            }
        }
    }

    // Emit signals
    emit selectionChanged(m_selected_polygons.size());
    if (m_selected_polygons.size() == 1) {
        emit polygonInfoUpdated(getPolygonInfo(m_selected_polygons[0]));
    } else if (m_selected_polygons.size() > 1) {
        emit polygonInfoUpdated(QString("Multiple Polygons Selected: %1").arg(m_selected_polygons.size()));
    } else {
        emit polygonInfoUpdated("No Polygon Selected");
    }
    update();
}

// Clears the current selection of polygons
void LayoutWidget::clearSelection() {
    m_selected_polygons.clear();
    emit selectionChanged(0);
    emit polygonInfoUpdated("No Polygon Selected");
    update();
}

// Returns a string containing information about the given polygon (points, area, centroid)
QString LayoutWidget::getPolygonInfo(const gdstk::Polygon* poly) const {
    if (!poly) return "No polygon selected";

    // Compute polygon properties
    double area = 0.0;
    QPointF centroid(0, 0);
    size_t point_count = poly->point_array.count;

    // Compute area and centroid using shoelace formula
    for (size_t i = 0; i < point_count; ++i) {
        size_t j = (i + 1) % point_count;
        double cross_prod = poly->point_array[i].x * poly->point_array[j].y - poly->point_array[j].x * poly->point_array[i].y;
        area += cross_prod;
        centroid.rx() += (poly->point_array[i].x + poly->point_array[j].x) * cross_prod;
        centroid.ry() += (poly->point_array[i].y + poly->point_array[j].y) * cross_prod;
    }

    area = std::abs(area) / 2.0;
    centroid.rx() /= (6.0 * area);
    centroid.ry() /= (6.0 * area);

    // Format polygon info
    return QString("Polygon Info:\n"
                   "Points: %1\n"
                   "Area: %2\n"
                   "Centroid: (%3, %4)")
        .arg(point_count)
        .arg(area, 0, 'f', 2)
        .arg(centroid.x(), 0, 'f', 2)
        .arg(centroid.y(), 0, 'f', 2);
}

// void LayoutWidget::updateSelectionStatus() { ... } // Removed

// void LayoutWidget::updatePolygonInfo() { ... } // Removed

// Destructor for LayoutWidget, frees GDS/OASIS library resources
LayoutWidget::~LayoutWidget() {
    m_gdstk_lib.free_all();
}

// Loads an OASIS file, populates polygon data, and updates the view
bool LayoutWidget::loadOasisFile(const QString& filename) {
    clearSelection(); // Reset selection state
    qDebug() << "Attempting to load OASIS file:" << filename;
    // Convert QString to std::string
    std::string filename_str = filename.toStdString();
    // Clear existing layers
    m_layers.clear();
    m_polygons_to_draw.clear();
    // Validate input filename
    if (filename_str.empty()) {
        QMessageBox::critical(this, "Load Error", "Empty filename");
        return false;
    }

    // Check file existence and readability
    QFileInfo fileInfo(filename);
    if (!fileInfo.exists()) {
        qDebug() << "Error: File does not exist" << filename;
        QMessageBox::critical(this, "Load Error", QString("File does not exist: %1").arg(filename));
        return false;
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
            return false;
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
            if (!cell) { // ADDED NULL CHECK FOR CELL
                qWarning() << "Warning: Encountered null cell at index" << i << "in library" << (m_gdstk_lib.name ? m_gdstk_lib.name : "Unnamed Library");
                continue; 
            }

            for (size_t j = 0; j < cell->polygon_array.count; ++j) {
                gdstk::Polygon* poly = cell->polygon_array[j];
                if (!poly) { // ADDED NULL CHECK FOR POLYGON
                    qWarning() << "Warning: Encountered null polygon in cell" << (cell->name ? cell->name : "Unnamed Cell") << "at polygon index" << j;
                    continue;
                }
                // Store polygons for drawing
                m_polygons_to_draw.push_back(poly);
                qDebug() << "Added polygon with" << poly->point_array.count << "points";

                // Update bounding box
                for (size_t k = 0; k < poly->point_array.count; ++k) {
                    min_x = std::min(min_x, poly->point_array[k].x);
                    min_y = std::min(min_y, poly->point_array[k].y);
                    max_x = std::max(max_x, poly->point_array[k].x);
                    max_y = std::max(max_y, poly->point_array[k].y);
                }

                // Collect layers
                m_layers[gdstk::get_layer(poly->tag)].push_back(poly);
            }
        }
        qDebug() << "Finished processing polygons loop.";
        qDebug() << "TEST LOG AFTER POLYGON LOOP"; // <-- NEW MINIMAL LOG
        // qFatal removed. The next line is the one that was failing to log.

        // Update current file path
        qDebug() << "[loadOasisFile] PRE-ASSIGNMENT m_current_file. (Size access removed)";
        m_current_file = filename;
        qDebug() << "[loadOasisFile] POST-ASSIGNMENT m_current_file. PRE-CALL to zoomToFit().";

        // Adjust view to fit loaded polygons
        // qDebug() << "[loadOasisFile] PRE-CALL to zoomToFit(). m_polygons_to_draw.size():" << m_polygons_to_draw.size(); // Original position, now moved up
        zoomToFit(); 
        qDebug() << "[loadOasisFile] POST-CALL to zoomToFit()."; // Simplified for now

        qDebug().noquote() << QString("[loadOasisFile] After zoomToFit: ZoomFactor: %1 PanOffset: (%2,%3)")
                                .arg(m_zoom_factor).arg(m_pan_offset.x()).arg(m_pan_offset.y());
        // zoomToFit() calls update(), so no need to call it again here.

        return true;
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "OASIS Load Error", QString("Exception: %1").arg(e.what()));
        return false;
    }
}

// Resets the view to its default zoom and pan state, or fits to loaded polygons
void LayoutWidget::resetView() {
    // Reset zoom and pan to default state
    m_zoom_factor = 1.0;
    m_pan_offset = QPointF(0, 0);

    // If a file is loaded, compute best fit zoom
    if (!m_current_file.isEmpty()) {
        zoomToFit();
    }

    update();
}

// Zooms the view to fit all loaded polygons
// Zooms the view to fit all loaded polygons
void LayoutWidget::zoomToFit() {
    qDebug() << "[zoomToFit] Entered zoomToFit function.";
    qDebug().noquote() << QString("[zoomToFit] Called. Polygons: %1. Widget: %2x%3")
                            .arg(m_polygons_to_draw.size()).arg(width()).arg(height());
    // Compute bounding box of all polygons
    if (m_polygons_to_draw.empty()) {
        // Added a log for this case as well
        qDebug() << "[zoomToFit] No polygons to draw, returning.";
        return;
    }

    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();

    for (const auto& poly : m_polygons_to_draw) {
        for (size_t i = 0; i < poly->point_array.count; ++i) {
            min_x = std::min(min_x, poly->point_array[i].x);
            max_x = std::max(max_x, poly->point_array[i].x);
            min_y = std::min(min_y, poly->point_array[i].y);
            max_y = std::max(max_y, poly->point_array[i].y);
        }
    }

    QRectF bounding_box(QPointF(min_x, min_y), QPointF(max_x, max_y));
    QRectF widget_rect(0, 0, width(), height());
    qDebug().noquote() << QString("[zoomToFit] BoundingBox: (%1,%2)-(%3,%4) W:%5 H:%6")
                            .arg(bounding_box.left()).arg(bounding_box.top())
                            .arg(bounding_box.right()).arg(bounding_box.bottom())
                            .arg(bounding_box.width()).arg(bounding_box.height());

    // Compute zoom to fit the entire bounding box
    if (bounding_box.width() == 0 || bounding_box.height() == 0) {
        qDebug() << "[zoomToFit] Bounding box has zero width or height. Resetting zoom/pan.";
        m_zoom_factor = 1.0; // Default zoom
        m_pan_offset = QPointF(0,0); // Default pan
    } else {
        double zoom_width = static_cast<double>(width()) / bounding_box.width();
        double zoom_height = static_cast<double>(height()) / bounding_box.height();
        m_zoom_factor = std::min(zoom_width, zoom_height);  // No padding, fit to edges

        // Center the view (revised calculation for m_pan_offset)
        m_pan_offset = QPointF(
            -bounding_box.center().x(),
            -bounding_box.center().y()
        );
    }

    qDebug().noquote() << QString("[zoomToFit] Calculated m_zoom_factor: %1 m_pan_offset: (%2,%3)")
                           .arg(m_zoom_factor).arg(m_pan_offset.x()).arg(m_pan_offset.y());
    qDebug() << "[zoomToFit] Exiting zoomToFit function.";
    update();
}

// Zooms the view to fit the currently selected polygons
void LayoutWidget::zoomToSelection() {
    if (m_selected_polygons.empty()) {
        qDebug() << "No polygons selected";
        return;
    }

    // Compute bounding box of selected polygons
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();

    for (const auto& poly : m_selected_polygons) {
        for (size_t i = 0; i < poly->point_array.count; ++i) {
            min_x = std::min(min_x, poly->point_array[i].x);
            max_x = std::max(max_x, poly->point_array[i].x);
            min_y = std::min(min_y, poly->point_array[i].y);
            max_y = std::max(max_y, poly->point_array[i].y);
        }
    }

    QRectF bounding_box(QPointF(min_x, min_y), QPointF(max_x, max_y));

    // Compute zoom to fit the selected polygons
    double zoom_width = width() / bounding_box.width();
    double zoom_height = height() / bounding_box.height();
    m_zoom_factor = std::min(zoom_width, zoom_height) * 0.9;  // 90% to add some padding

    // Center the view
    m_pan_offset = QPointF(
        -(bounding_box.center().x() * m_zoom_factor - width() / 2.0),
        -(bounding_box.center().y() * m_zoom_factor - height() / 2.0)
    );

    update();
}

// Zooms in the view by a fixed step
void LayoutWidget::zoomIn() {
    m_zoom_factor = std::min(m_zoom_factor * ZOOM_STEP, MAX_ZOOM);
    update();
}

// Zooms out the view by a fixed step
void LayoutWidget::zoomOut() {
    m_zoom_factor = std::max(m_zoom_factor / ZOOM_STEP, MIN_ZOOM);
    update();
}

// Handles key press events for various actions (zoom, pan, selection, etc.)
void LayoutWidget::keyPressEvent(QKeyEvent *event) {
    // TODO: Consider re-introducing selection_modifier if key-based selection is implemented
    // SelectionModifier selection_modifier = SelectionModifier::Replace;
    // if (event->modifiers() & Qt::ControlModifier) {
    //     selection_modifier = SelectionModifier::Add;
    // } else if (event->modifiers() & Qt::ShiftModifier) {
    //     selection_modifier = SelectionModifier::Subtract;
    // } else if (event->modifiers() & Qt::AltModifier) {
    //     selection_modifier = SelectionModifier::Toggle;
    // }

    switch (event->key()) {
        case Qt::Key_0: // Reset view
            if (event->modifiers() & Qt::ControlModifier) {
                resetView();
            }
            break;
        case Qt::Key_Plus: // Zoom in
        case Qt::Key_Equal: // Some keyboards require Shift for '+'
            if (event->modifiers() & Qt::ControlModifier) {
                zoomIn();
            }
            break;
        case Qt::Key_Minus: // Zoom out
            if (event->modifiers() & Qt::ControlModifier) {
                zoomOut();
            }
            break;
        case Qt::Key_F: // Fit to view
            if (event->modifiers() & Qt::ControlModifier) {
                zoomToFit();
            }
            break;
        case Qt::Key_S: // Toggle selection mode
            if (event->modifiers() & Qt::ControlModifier) {
                setInteractionMode(m_interaction_mode == InteractionMode::Select 
                    ? InteractionMode::Normal : InteractionMode::Select);
            }
            break;
        case Qt::Key_A: // Select All
            if (event->modifiers() & Qt::ControlModifier) {
                selectPolygons(QRectF(QPointF(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()),
                                      QPointF(std::numeric_limits<double>::max(), std::numeric_limits<double>::max())),
                               SelectionModifier::Replace);
            }
            break;
        case Qt::Key_D: // Deselect All
            if (event->modifiers() & Qt::ControlModifier) {
                clearSelection();
            }
            break;
        case Qt::Key_I: // Invert Selection
            if (event->modifiers() & Qt::ControlModifier) {
                std::vector<gdstk::Polygon*> current_selection = m_selected_polygons;
                clearSelection();
                qDebug() << "Total cells in library:" << m_gdstk_lib.cell_array.count;
                for (size_t i = 0; i < m_gdstk_lib.cell_array.count; ++i) {
                    qDebug() << "Cell has" << m_gdstk_lib.cell_array[i]->polygon_array.count << "polygons";
                }
                for (const auto& poly : m_polygons_to_draw) {
                    if (std::find(current_selection.begin(), current_selection.end(), poly) == current_selection.end()) {
                        m_selected_polygons.push_back(poly);
                    }
                }
                emit selectionChanged(m_selected_polygons.size());
                if (m_selected_polygons.size() == 1) {
                    emit polygonInfoUpdated(getPolygonInfo(m_selected_polygons[0]));
                } else if (m_selected_polygons.size() > 1) {
                    emit polygonInfoUpdated(QString("Multiple Polygons Selected: %1").arg(m_selected_polygons.size()));
                } else {
                    emit polygonInfoUpdated("No Polygon Selected");
                }
                update();
            }
            break;
        case Qt::Key_Z: // Toggle Zoom Mode
            if (event->modifiers() & Qt::ControlModifier) {
                setInteractionMode(m_interaction_mode == InteractionMode::Zoom 
                    ? InteractionMode::Normal : InteractionMode::Zoom);
            }
            break;
        case Qt::Key_P: // Toggle Pan Mode
            if (event->modifiers() & Qt::ControlModifier) {
                setInteractionMode(m_interaction_mode == InteractionMode::Pan 
                    ? InteractionMode::Normal : InteractionMode::Pan);
            }
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

// Starts a new selection at the given position
void LayoutWidget::startSelection(const QPointF& pos) {
    m_selection_start = pos;
    m_selection_end = pos;
    m_is_selecting = true;
    m_selected_polygons.clear();
}

// Updates the current selection to the given position and repaints the view
void LayoutWidget::updateSelection(const QPointF& pos) {
    if (!m_is_selecting) return;

    m_selection_end = pos;
    
    // Find polygons in selection rectangle
    m_selected_polygons.clear();
    for (const auto& poly : m_polygons_to_draw) {
        if (isPolygonInSelection(poly)) {
            m_selected_polygons.push_back(poly);
        }
    }

    update(); // Trigger repaint to show selection
}

// Ends the current selection and repaints the view
void LayoutWidget::endSelection() {
    m_is_selecting = false;
    update();
}

// Checks if any point of the given polygon is within the current selection rectangle
bool LayoutWidget::isPolygonInSelection(const gdstk::Polygon* poly) const {
    // Create selection rectangle in scene coordinates
    QRectF selection_rect(m_selection_start, m_selection_end);
    selection_rect = selection_rect.normalized(); // Ensure top-left and bottom-right

    // Check if any polygon point is inside the selection rectangle
    for (size_t i = 0; i < poly->point_array.count; ++i) {
        QPointF poly_point(poly->point_array[i].x, poly->point_array[i].y);
        if (selection_rect.contains(poly_point)) {
            return true;
        }
    }

    return false;
}

// Sets the current interaction mode for the widget
void LayoutWidget::setInteractionMode(InteractionMode mode) {
    m_interaction_mode = mode;
}

// Toggles the visibility of the grid and repaints the view
void LayoutWidget::toggleGrid() {
    m_show_grid = !m_show_grid;
    update();
}

// Sets the spacing of the grid lines and repaints the view
void LayoutWidget::setGridSpacing(double spacing) {
    m_grid_spacing = spacing;
    update();
}

// Returns a sorted list of available layers from the loaded GDS/OASIS file
QVector<int> LayoutWidget::getAvailableLayers() {
    QVector<int> layers;
    for (const auto& layer_pair : m_layers) {
        layers.push_back(layer_pair.first);
    }
    std::sort(layers.begin(), layers.end());
    return layers;
}

// Stub implementations for missing Qt event handlers

void LayoutWidget::paintEvent(QPaintEvent *event) {
    qDebug().noquote() << QString("[paintEvent] Called. Zoom: %1 Pan: (%2,%3) Widget: %4x%5 Polys: %6")
                            .arg(m_zoom_factor).arg(m_pan_offset.x()).arg(m_pan_offset.y())
                            .arg(width()).arg(height()).arg(m_polygons_to_draw.size());
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // Smoother lines

    // Fill background
    painter.fillRect(rect(), Qt::black);

    // Save painter state
    painter.save();

    // 1. Translate to center of the widget
    painter.translate(width() / 2.0, height() / 2.0);

    // 2. Apply zoom (inverting Y-axis for Y-up gdstk coordinates)
    painter.scale(m_zoom_factor, -m_zoom_factor);

    // 3. Apply pan
    painter.translate(m_pan_offset);

    // Draw polygons

    bool first_poly_debug_done = false;
    for (const gdstk::Polygon* poly : m_polygons_to_draw) {
        if (poly && poly->point_array.count > 0) {
            QPolygonF q_poly;
            for (size_t i = 0; i < poly->point_array.count; ++i) {
                q_poly << QPointF(poly->point_array[i].x, poly->point_array[i].y);
            }
            
            uint32_t layer_num = gdstk::get_layer(poly->tag);
            // uint32_t datatype_num = gdstk::get_type(poly->tag); // Not used yet

            if (!first_poly_debug_done && poly && poly->point_array.count > 0) {
                QString points_str;
                for(size_t k=0; k < std::min((size_t)5, poly->point_array.count); ++k) {
                    points_str += QString("(%1,%2) ").arg(poly->point_array[k].x).arg(poly->point_array[k].y);
                }
                qDebug().noquote() << QString("[paintEvent] First polygon (Layer %1, %2 pts): %3")
                                      .arg(gdstk::get_layer(poly->tag)).arg(poly->point_array.count).arg(points_str);
                first_poly_debug_done = true;
            }
            QColor polyColor = Qt::gray; // Default color
            switch (layer_num % 7) { // Cycle 7 colors for variety
                case 0: polyColor = QColor(255, 0, 0); break;     // Red
                case 1: polyColor = QColor(0, 255, 0); break;     // Green
                case 2: polyColor = QColor(0, 0, 255); break;     // Blue
                case 3: polyColor = QColor(255, 255, 0); break;   // Yellow
                case 4: polyColor = QColor(0, 255, 255); break;   // Cyan
                case 5: polyColor = QColor(255, 0, 255); break;   // Magenta
                case 6: polyColor = QColor(128, 128, 128); break; // Fallback Gray
            }
            painter.setBrush(polyColor);
            painter.setPen(QPen(polyColor.lighter(130), 0)); // Use a lighter shade of polyColor for outline
            painter.drawPolygon(q_poly);
        }
    }

    // Restore painter state (removes transforms for UI elements like selection rect)
    painter.restore();

    // Draw selection rectangle or zoom rectangle (on top of polygons, without world transforms)
    if (m_is_selecting || m_is_zooming) {
        painter.setPen(Qt::white); // Use a visible color like white for the rectangle
        painter.setBrush(Qt::NoBrush);
        if (m_is_selecting) {
            painter.drawRect(QRectF(m_selection_start, m_selection_end).normalized());
        } else if (m_is_zooming) {
             painter.drawRect(QRectF(m_zoom_start_pos, m_zoom_current_pos).normalized());
        }
    }
}

void LayoutWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event); // Call base class implementation
    m_last_mouse_pos = event->pos(); 
    qDebug() << "LayoutWidget::mousePressEvent at" << event->pos();

    if (event->button() == Qt::LeftButton) {
        if (m_interaction_mode == InteractionMode::Pan) {
            m_is_panning = true;
            setCursor(Qt::ClosedHandCursor);
        } else if (m_interaction_mode == InteractionMode::Select) {
            startSelection(event->pos());
        } else if (m_interaction_mode == InteractionMode::Zoom) {
            m_is_zooming = true;
            m_zoom_start_pos = event->pos();
            m_zoom_current_pos = event->pos();
            update(); 
        }
    }
}

void LayoutWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event); // Call base class implementation
    qDebug() << "LayoutWidget::mouseReleaseEvent at" << event->pos();

    if (event->button() == Qt::LeftButton) {
        if (m_is_panning) {
            m_is_panning = false;
            setCursor(Qt::ArrowCursor); 
        } else if (m_is_selecting) {
            endSelection();
        } else if (m_is_zooming) {
            m_is_zooming = false;
            QRectF zoom_rect_screen = QRectF(m_zoom_start_pos, event->pos()).normalized();
            if (zoom_rect_screen.width() > 5 && zoom_rect_screen.height() > 5) { 
                ZoomParameters params = computeZoomParameters(zoom_rect_screen);
                if(params.is_valid) {
                    m_zoom_factor = params.zoom_factor;
                    m_pan_offset = params.pan_offset;
                }
            }
            update(); 
        }
    }
}

void LayoutWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event); // Call base class implementation
    // qDebug() << "LayoutWidget::mouseMoveEvent at" << event->pos();

    if (m_is_panning) {
        QPointF delta = event->pos() - m_last_mouse_pos;
        m_pan_offset += delta;
        m_last_mouse_pos = event->pos();
        update(); 
    } else if (m_is_selecting) {
        updateSelection(event->pos());
    } else if (m_is_zooming) {
        m_zoom_current_pos = event->pos();
        update(); 
    }

    // Update mouse coordinate display
    QPointF scene_pos = computeSceneCoordinates(event->pos());
    QString status_text = QString("X: %1, Y: %2 | Zoom: %3x")
                              .arg(scene_pos.x(), 0, 'f', 2)
                              .arg(scene_pos.y(), 0, 'f', 2)
                              .arg(m_zoom_factor, 0, 'f', 2);
    emit mouseInfoUpdated(status_text);
}

void LayoutWidget::wheelEvent(QWheelEvent *event) {
    QWidget::wheelEvent(event); // Call base class implementation
    qDebug() << "LayoutWidget::wheelEvent delta" << event->angleDelta();

    if (event->angleDelta().y() > 0) {
        zoomIn(); 
    } else if (event->angleDelta().y() < 0) {
        zoomOut(); 
    }
}

// main function to be added
#ifndef IS_TEST_BUILD
int main(int argc, char *argv[]) {
    qInstallMessageHandler(myMessageOutput); // Install before QApplication
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    LayoutWidget *layoutWidget = new LayoutWidget(&mainWindow);
    mainWindow.setCentralWidget(layoutWidget);

    // Setup status bar
    QLabel *statusMouseLabel = new QLabel(&mainWindow);
    statusMouseLabel->setMinimumWidth(200); // Give it some space
    mainWindow.statusBar()->addWidget(statusMouseLabel);

    QLabel *statusPolyLabel = new QLabel(&mainWindow);
    statusPolyLabel->setMinimumWidth(300); // Give it some space
    statusPolyLabel->setText("No Polygon Selected"); // Set initial text
    mainWindow.statusBar()->addPermanentWidget(statusPolyLabel); // Permanent widgets are on the right

    // Connect signals from layoutWidget to update these labels
    QObject::connect(layoutWidget, &LayoutWidget::mouseInfoUpdated, statusMouseLabel, &QLabel::setText);
    QObject::connect(layoutWidget, &LayoutWidget::polygonInfoUpdated, statusPolyLabel, &QLabel::setText);

    mainWindow.show();

    // Create toolbars
    QToolBar *fileToolBar = mainWindow.addToolBar("File");
    QToolBar *viewToolBar = mainWindow.addToolBar("View");
    QToolBar *modeToolBar = mainWindow.addToolBar("Mode");

    // --- File Actions ---
    QAction *loadAction = new QAction("Load OASIS/GDS", &mainWindow);
    // TODO: Add icon: loadAction->setIcon(QIcon::fromTheme("document-open"));
    fileToolBar->addAction(loadAction);
    QObject::connect(loadAction, &QAction::triggered, [&mainWindow, layoutWidget]() { // Capture mainWindow for parent
        QString fileName = QFileDialog::getOpenFileName(&mainWindow, "Open Layout File", "", "OASIS Files (*.oas);;GDSII Files (*.gds);;All Files (*)");
        if (!fileName.isEmpty()) {
            if (!layoutWidget->loadOasisFile(fileName)) {
                QMessageBox::warning(&mainWindow, "Error", "Could not load file: " + fileName);
            }
        }
    });

    // --- View Actions ---
    QAction *zoomFitAction = new QAction("Zoom to Fit", &mainWindow);
    // TODO: Add icon: zoomFitAction->setIcon(QIcon::fromTheme("zoom-fit-best"));
    viewToolBar->addAction(zoomFitAction);
    QObject::connect(zoomFitAction, &QAction::triggered, layoutWidget, &LayoutWidget::zoomToFit);
    
    QAction *resetViewAction = new QAction("Reset View", &mainWindow);
    // TODO: Add icon: resetViewAction->setIcon(QIcon::fromTheme("view-refresh"));
    viewToolBar->addAction(resetViewAction);
    QObject::connect(resetViewAction, &QAction::triggered, layoutWidget, &LayoutWidget::resetView);


    // --- Mode Actions ---
    QAction *normalModeAction = new QAction("Normal Mode", &mainWindow);
    normalModeAction->setCheckable(true);
    // TODO: Add icon
    modeToolBar->addAction(normalModeAction);
    QObject::connect(normalModeAction, &QAction::triggered, layoutWidget, [layoutWidget]() {
        layoutWidget->setInteractionMode(LayoutWidget::InteractionMode::Normal);
        qDebug() << "Normal Mode activated";
    });

    QAction *zoomModeAction = new QAction("Zoom Mode", &mainWindow);
    zoomModeAction->setCheckable(true);
    // TODO: Add icon
    modeToolBar->addAction(zoomModeAction);
    QObject::connect(zoomModeAction, &QAction::triggered, layoutWidget, [layoutWidget]() {
        layoutWidget->setInteractionMode(LayoutWidget::InteractionMode::Zoom);
        qDebug() << "Zoom Mode activated";
    });

    QAction *panModeAction = new QAction("Pan Mode", &mainWindow);
    panModeAction->setCheckable(true);
    // TODO: Add icon
    modeToolBar->addAction(panModeAction);
    QObject::connect(panModeAction, &QAction::triggered, layoutWidget, [layoutWidget]() {
        layoutWidget->setInteractionMode(LayoutWidget::InteractionMode::Pan);
        qDebug() << "Pan Mode activated";
    });

    QAction *selectModeAction = new QAction("Select Mode", &mainWindow);
    selectModeAction->setCheckable(true);
    // TODO: Add icon
    modeToolBar->addAction(selectModeAction);
    QObject::connect(selectModeAction, &QAction::triggered, layoutWidget, [layoutWidget]() {
        layoutWidget->setInteractionMode(LayoutWidget::InteractionMode::Select);
        qDebug() << "Select Mode activated";
    });
    
    // Action group for mode buttons to make them exclusive
    QActionGroup *modeActionGroup = new QActionGroup(&mainWindow);
    modeActionGroup->addAction(normalModeAction);
    modeActionGroup->addAction(zoomModeAction);
    modeActionGroup->addAction(panModeAction);
    modeActionGroup->addAction(selectModeAction);
    normalModeAction->setChecked(true); // Default mode


    mainWindow.setWindowTitle("Design Layout Viewer");
    mainWindow.resize(1024, 768);
    mainWindow.show();

    if (argc > 1) {
        QString filePath = QString::fromLocal8Bit(argv[1]);
        if (!layoutWidget->loadOasisFile(filePath)) {
            QMessageBox::warning(&mainWindow, "Error", "Could not load file: " + filePath);
        }
    }
    // Optional: Trigger file dialog if no argument provided
    // else {
    //     loadAction->trigger();
    // }
    return app.exec();
}
#endif // IS_TEST_BUILD
