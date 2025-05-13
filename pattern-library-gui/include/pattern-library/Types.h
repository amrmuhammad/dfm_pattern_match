// filepath: /pattern-library-gui/include/pattern-library/Types.h

#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QVector>

// Define a structure for Pattern
struct Pattern {
    QString name;
    QString metadata;
    QVector<double> geometry; // Assuming geometry is represented as a vector of doubles
};

// Define a structure for SearchCriteria
struct SearchCriteria {
    QString name;
    QString metadata;
    double minSize;
    double maxSize;
};

#endif // TYPES_H