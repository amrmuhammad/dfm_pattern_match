// filepath: /pattern-library-gui/include/pattern-library/Constants.h

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace Constants {
    const QString DATABASE_CONNECTION_ERROR = "Failed to connect to the database.";
    const QString PATTERN_IMPORT_SUCCESS = "Pattern imported successfully.";
    const QString PATTERN_EXPORT_SUCCESS = "Pattern exported successfully.";
    const QString PATTERN_NOT_FOUND = "Pattern not found.";
    const QString INVALID_PATTERN_FORMAT = "Invalid pattern file format.";
    const QString DEFAULT_WINDOW_TITLE = "DFM Pattern Library GUI";
    const int MAX_PATTERN_COUNT = 1000; // Maximum number of patterns to display
}

#endif // CONSTANTS_H