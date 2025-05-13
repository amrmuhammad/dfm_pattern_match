#include "PatternController.h"
#include "../models/PatternModel.h"
#include "../utils/ImportExport.h"

PatternController::PatternController(PatternModel* model) : model(model) {}

void PatternController::importPattern(const QString& filePath) {
    if (!model) return;
    auto importedPattern = ImportExport::importPattern(filePath);
    if (!importedPattern.name().isEmpty()) {
        model->addPattern(importedPattern);
    }
}

void PatternController::exportPattern(const QString& filePath) {
    if (!model) return;
    int selectedRow = 0; // TODO: Get selected pattern index
    if (selectedRow >= 0) {
        Pattern pattern = model->getPattern(selectedRow);
        ImportExport::exportPattern(filePath, pattern);
    }
}

QList<Pattern> PatternController::searchPatterns(const QString& criteria) {
    if (!model) return QList<Pattern>();
    QList<Pattern> results;
    for (const auto& pattern : model->getAllPatterns()) {
        if (pattern.name().contains(criteria, Qt::CaseInsensitive)) {
            results.append(pattern);
        }
    }
    return results;
}

void PatternController::addPattern(const Pattern& pattern) {
    if (model) {
        model->addPattern(pattern);
    }
}

void PatternController::removePattern(const QString& patternId) {
    if (!model) return;
    for (int i = 0; i < model->rowCount(); ++i) {
        Pattern pattern = model->getPattern(i);
        if (pattern.name() == patternId) {
            model->removePattern(i);
            break;
        }
    }
}