// filepath: /pattern-library-gui/pattern-library-gui/src/utils/GeometryUtils.h

#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

#include <vector>
#include <QPointF>

namespace GeometryUtils {

    // Function to calculate the area of a polygon
    double calculatePolygonArea(const std::vector<QPointF>& points);

    // Function to check if a point is inside a polygon
    bool isPointInPolygon(const QPointF& point, const std::vector<QPointF>& polygon);

    // Function to simplify a polygon using the Ramer-Douglas-Peucker algorithm
    std::vector<QPointF> simplifyPolygon(const std::vector<QPointF>& points, double epsilon);

}

#endif // GEOMETRY_UTILS_H