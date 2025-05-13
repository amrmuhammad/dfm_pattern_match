#include "FileHandler.h"
#include <QFileInfo>
#include <stdexcept>

#ifdef HAVE_GDSTK
#include <gdstk/gdstk.hpp>
#endif

Pattern FileHandler::loadFromOasis(const QString& filename)
{
#ifdef HAVE_GDSTK
    gdstk::Library lib;
    gdstk::ErrorCode error_code;
    lib = gdstk::read_oas(filename.toStdString().c_str(), 0.0, 0.0, &error_code);

    if (error_code != gdstk::ErrorCode::NoError) {
        throw std::runtime_error("Failed to read OASIS file");
    }

    Pattern pattern(QFileInfo(filename).baseName());
    QPolygonF geometry;

    // Process the first cell found
    for (uint64_t i = 0; i < lib.cell_array.count; ++i) {
        gdstk::Cell* cell = lib.cell_array[i];
        if (cell && cell->polygon_array.count > 0) {
            gdstk::Polygon* poly = cell->polygon_array[0];
            geometry = convertGdstkPolygonToQt(poly);
            break;
        }
    }

    pattern.setGeometry(geometry);
    lib.clear();
    return pattern;
#else
    throw std::runtime_error("GDSTK support not available. Cannot load OASIS files.");
#endif
}

Pattern FileHandler::loadFromGds(const QString& filename)
{
#ifdef HAVE_GDSTK
    gdstk::Library lib;
    gdstk::ErrorCode error_code;
    lib = gdstk::read_gds(filename.toStdString().c_str(), 0.0 /*unit*/, 0.001 /*tolerance*/, nullptr /*shape_tags*/, &error_code);

    if (error_code != gdstk::ErrorCode::NoError) {
        throw std::runtime_error("Failed to read GDSII file");
    }

    Pattern pattern(QFileInfo(filename).baseName());
    QPolygonF geometry;

    // Process the first cell found
    for (uint64_t i = 0; i < lib.cell_array.count; ++i) {
        gdstk::Cell* cell = lib.cell_array[i];
        if (cell && cell->polygon_array.count > 0) {
            gdstk::Polygon* poly = cell->polygon_array[0];
            geometry = convertGdstkPolygonToQt(poly);
            break;
        }
    }

    pattern.setGeometry(geometry);
    lib.clear();
    return pattern;
#else
    throw std::runtime_error("GDSTK support not available. Cannot load GDSII files.");
#endif
}

void FileHandler::saveToOasis(const QString& filename, const Pattern& pattern)
{
#ifdef HAVE_GDSTK
    gdstk::Library lib;
    lib.unit = 1e-6;
    lib.precision = 1e-9;

    gdstk::Cell* cell = (gdstk::Cell*)gdstk::allocate_clear(sizeof(gdstk::Cell));
    cell->name = gdstk::copy_string(pattern.name().toStdString().c_str(), NULL);

    gdstk::Polygon* poly = convertQtPolygonToGdstk(pattern.geometry());
    cell->polygon_array.append(poly);
    lib.cell_array.append(cell);

    gdstk::ErrorCode error_code = lib.write_oas(filename.toStdString().c_str(), 0, 0, 0);
    if (error_code != gdstk::ErrorCode::NoError) {
        lib.clear();
        throw std::runtime_error("Failed to write OASIS file");
    }

    lib.clear();
#else
    throw std::runtime_error("GDSTK support not available. Cannot save OASIS files.");
#endif
}

void FileHandler::saveToGds(const QString& filename, const Pattern& pattern)
{
#ifdef HAVE_GDSTK
    gdstk::Library lib;
    lib.unit = 1e-6;
    lib.precision = 1e-9;

    gdstk::Cell* cell = (gdstk::Cell*)gdstk::allocate_clear(sizeof(gdstk::Cell));
    cell->name = gdstk::copy_string(pattern.name().toStdString().c_str(), NULL);

    gdstk::Polygon* poly = convertQtPolygonToGdstk(pattern.geometry());
    cell->polygon_array.append(poly);
    lib.cell_array.append(cell);

    gdstk::ErrorCode error_code = lib.write_gds(filename.toStdString().c_str(), (uint64_t)0 /*max_points*/, nullptr /*timestamp*/);
    if (error_code != gdstk::ErrorCode::NoError) {
        lib.clear();
        throw std::runtime_error("Failed to write GDSII file");
    }

    lib.clear();
#else
    throw std::runtime_error("GDSTK support not available. Cannot save GDSII files.");
#endif
}

QPolygonF FileHandler::convertGdstkPolygonToQt(const gdstk::Polygon* poly)
{
#ifdef HAVE_GDSTK
    QPolygonF qpoly;
    for (uint64_t i = 0; i < poly->point_array.count; ++i) {
        qpoly << QPointF(poly->point_array[i].x, poly->point_array[i].y);
    }
    return qpoly;
#else
    throw std::runtime_error("GDSTK support not available. Cannot convert GDSTK polygon to Qt.");
#endif
}

gdstk::Polygon* FileHandler::convertQtPolygonToGdstk(const QPolygonF& polygon)
{
#ifdef HAVE_GDSTK
    gdstk::Polygon* poly = (gdstk::Polygon*)gdstk::allocate_clear(sizeof(gdstk::Polygon));
    
    for (const QPointF& pt : polygon) {
        poly->point_array.append({pt.x(), pt.y()});
    }
    
    return poly;
#else
    throw std::runtime_error("GDSTK support not available. Cannot convert Qt polygon to GDSTK.");
#endif
}