#ifndef IMPORT_EXPORT_H
#define IMPORT_EXPORT_H

#include <QString>
#include <QList>
#include "../models/PatternModel.h"

namespace ImportExport {
    // Import a single pattern from a file
    Pattern importPattern(const QString& filePath);

    // Export a single pattern to a file
    bool exportPattern(const QString& filePath, const Pattern& pattern);

    // Import multiple patterns from a file
    QList<Pattern> importPatterns(const QString& filePath);

    // Export multiple patterns to a file
    bool exportPatterns(const QString& filePath, const QList<Pattern>& patterns);

} // namespace ImportExport

#endif // IMPORT_EXPORT_H