#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <QString>
#include <QPolygonF>
#include "../models/PatternModel.h"
#include <memory>

#ifdef HAVE_GDSTK
#include <gdstk/gdstk.hpp> // Added for gdstk types
#endif

class FileHandler {
public:
    static Pattern loadFromOasis(const QString& filename);
    static Pattern loadFromGds(const QString& filename);
    static void saveToOasis(const QString& filename, const Pattern& pattern);
    static void saveToGds(const QString& filename, const Pattern& pattern);
    static QPolygonF convertGdstkPolygonToQt(const gdstk::Polygon* poly);

private:
    static gdstk::Polygon* convertQtPolygonToGdstk(const QPolygonF& polygon);
};

#endif // FILEHANDLER_H