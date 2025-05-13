#ifndef PATTERNCONTROLLER_H
#define PATTERNCONTROLLER_H

#include <QString>
#include <QList>
#include "../models/PatternModel.h"
#include "../utils/ImportExport.h"

class PatternController {
public:
    explicit PatternController(PatternModel* model = nullptr);
    ~PatternController() = default;

    void importPattern(const QString& filePath);
    void exportPattern(const QString& filePath);
    QList<Pattern> searchPatterns(const QString& criteria);
    void addPattern(const Pattern& pattern);
    void removePattern(const QString& patternId);

private:
    PatternModel* model;
};

#endif // PATTERNCONTROLLER_H