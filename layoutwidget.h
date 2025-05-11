#ifndef LAYOUTWIDGET_H
#define LAYOUTWIDGET_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QRectF>
#include <QPointF>
#include <QWheelEvent> // For method declarations
#include <QMouseEvent> // For method declarations
#include <QPaintEvent> // For method declarations
#include <QKeyEvent>   // For method declarations
#include <QLabel>      // For m_status_label, m_polygon_info_label

// Forward declaration if gdstk::Polygon is only used as pointer/reference in header
// However, since m_gdstk_lib and m_layers use gdstk types directly, full include is better.
#include <gdstk/gdstk.hpp>

#include <vector>
#include <map>
#include <set> // Though not directly in class declaration, often used with gdstk

// Class for displaying and interacting with GDSII/OASIS layout data
class LayoutWidget : public QWidget {
    Q_OBJECT // Macro for Qt's meta-object system (signals, slots, etc.)

public:
    // Constructor
    explicit LayoutWidget(QWidget *parent = nullptr);
    // Destructor
    ~LayoutWidget();

    // Enum for different interaction modes (normal, zoom, pan, select)
    enum class InteractionMode {
        Normal,
        Zoom,
        Pan,
        Select
    };

    // Enum for selection modifiers (replace, add, subtract, toggle)
    enum class SelectionModifier {
        Replace,   // Replace current selection
        Add,       // Add to current selection
        Subtract,  // Remove from current selection
        Toggle     // Toggle selection status
    };

    // Load an OASIS file into the viewer
    bool loadOasisFile(const QString& filename);
    // Reset the view to its default state (zoom, pan)
    void resetView();
    // Set the current interaction mode
    void setInteractionMode(InteractionMode mode);
    // Toggle the visibility of the grid
    void toggleGrid();
    // Set the spacing of the grid
    void setGridSpacing(double spacing);
    // Get a list of available layers in the loaded file
    QVector<int> getAvailableLayers();
    // Toggle the visibility of a specific layer
    void toggleLayerVisibility(int layer);
    // Zoom to fit all loaded polygons in the view
    void zoomToFit();
    // Zoom to fit the currently selected polygons in the view
    void zoomToSelection();
    // Zoom in the view
    void zoomIn();
    // Zoom out the view
    void zoomOut();

    // New selection methods
    // Select polygons within a given rectangle using a specified modifier
    void selectPolygons(const QRectF& selection_rect, SelectionModifier modifier = SelectionModifier::Replace);
    // Clear the current selection
    void clearSelection();

    // Polygon info retrieval
    // Get a string containing information about a given polygon
    QString getPolygonInfo(const gdstk::Polygon* poly) const;

Q_SIGNALS: // Qt signals declaration
    // Signal emitted when the selection changes
    void selectionChanged(int selected_count);
    // Signal emitted when the polygon information is updated
    void polygonInfoUpdated(const QString& info);
    void mouseInfoUpdated(const QString &info);

protected:
    // Handle mouse wheel events (for zooming)
    void wheelEvent(QWheelEvent *event) override;
    // Handle mouse press events (for panning and selection)
    void mousePressEvent(QMouseEvent *event) override;
    // Handle mouse move events (for panning and selection)
    void mouseMoveEvent(QMouseEvent *event) override;
    // Handle mouse release events (for panning and selection)
    void mouseReleaseEvent(QMouseEvent *event) override;
    // Handle paint events (for drawing polygons, grid, etc.)
    void paintEvent(QPaintEvent *event) override;
    // Handle key press events (for shortcuts)
    void keyPressEvent(QKeyEvent *event) override;

private:
    // === Private Member Variables ===

    // --- Zoom and Pan ---
    double m_zoom_factor = 1.0; // Current zoom factor
    QPointF m_pan_offset{0, 0}; // Current pan offset
    QPointF m_last_mouse_pos;   // Last recorded mouse position (for panning)
    bool m_is_panning = false;  // Flag indicating if panning is active

    // --- Zoom Constants ---
    static constexpr double MIN_ZOOM = 1e-5;  // Minimum zoom factor
    static constexpr double MAX_ZOOM = 10.0; // Maximum zoom factor
    static constexpr double ZOOM_STEP = 1.2; // Zoom step increment

    // --- Selection ---
    std::vector<gdstk::Polygon*> m_selected_polygons; // Vector of selected polygons
    QPointF m_selection_start;                        // Start point of selection rectangle
    QPointF m_selection_end;                          // End point of selection rectangle
    bool m_is_selecting = false;                      // Flag indicating if selection is active
    SelectionModifier m_current_selection_modifier = SelectionModifier::Replace; // Current selection modifier

    // --- GDS/OASIS Data ---
    gdstk::Library m_gdstk_lib; // Gdstk library object for storing layout data
    std::map<int, std::vector<gdstk::Polygon*>> m_layers; // Map of layers and their polygons
    std::vector<gdstk::Polygon*> m_polygons_to_draw; // Vector of all polygons to draw
    QString m_current_file; // Path to the currently loaded file

    // --- Grid ---
    bool m_show_grid = false;       // Flag indicating if the grid should be shown
    double m_grid_spacing = 1.0;  // Spacing of the grid lines

    // --- Interaction State ---
    InteractionMode m_interaction_mode = InteractionMode::Normal; // Current interaction mode
    QPointF m_zoom_start_pos;   // Start position for zoom rectangle
    QPointF m_zoom_current_pos; // Current position for zoom rectangle
    bool m_is_zooming = false;    // Flag indicating if zoom rectangle is active

    // === Private Helper Methods ===

    // --- Selection ---
    // Start a new selection at the given position
    void startSelection(const QPointF& pos);
    // Update the current selection to the given position
    void updateSelection(const QPointF& pos);
    // End the current selection
    void endSelection();
    // Check if a polygon is within the current selection rectangle
    bool isPolygonInSelection(const gdstk::Polygon* poly) const;

    // --- Zoom ---
    // Compute scene coordinates from screen coordinates
    QPointF computeSceneCoordinates(const QPointF& screen_pos) const;
    // Structure for storing zoom parameters
    struct ZoomParameters {
        double zoom_factor;
        QPointF pan_offset;
        bool is_valid;
    };
    // Compute zoom parameters for a given zoom rectangle
    ZoomParameters computeZoomParameters(const QRectF& zoom_rect);
};

#endif // LAYOUTWIDGET_H
