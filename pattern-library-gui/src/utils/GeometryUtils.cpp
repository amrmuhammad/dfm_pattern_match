// filepath: /pattern-library-gui/src/utils/GeometryUtils.cpp
#include "GeometryUtils.h"
#include <cmath> // For std::abs
#include <GL/gl.h> // For OpenGL functions

namespace GeometryUtils {

// Function to calculate the area of a polygon
double calculatePolygonArea(const std::vector<QPointF>& vertices) {
    double area = 0.0;
    int n = vertices.size();
    
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        area += vertices[i].x() * vertices[j].y();
        area -= vertices[j].x() * vertices[i].y();
    }
    area = std::abs(area) / 2.0;
    return area;
}

// Function to render a polygon using OpenGL
void renderPolygon(const std::vector<QPointF>& vertices) {
    glBegin(GL_POLYGON);
    for (const auto& vertex : vertices) {
        glVertex2f(vertex.x(), vertex.y());
    }
    glEnd();
}

// Function to check if a point is inside a polygon
bool isPointInPolygon(const QPointF& point, const std::vector<QPointF>& vertices) {
    bool inside = false;
    int n = vertices.size();
    
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if ((vertices[i].y() > point.y()) != (vertices[j].y() > point.y()) &&
            (point.x() < (vertices[j].x() - vertices[i].x()) * (point.y() - vertices[i].y()) / (vertices[j].y() - vertices[i].y()) + vertices[i].x())) {
            inside = !inside;
        }
    }
    return inside;
}

} // namespace GeometryUtils