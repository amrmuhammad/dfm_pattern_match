// filepath: /pattern-library-gui/src/models/PatternModel.cpp
#include "PatternModel.h"

// Constructor
PatternModel::PatternModel(QObject *parent) : QAbstractListModel(parent) {
}

// Destructor
PatternModel::~PatternModel() = default;

// Method to get the row count
int PatternModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return m_patterns.size();
}

// Method to get data
QVariant PatternModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_patterns.size())
        return QVariant();

    const Pattern& pattern = m_patterns[index.row()];
    
    switch (role) {
        case Qt::DisplayRole:
            return pattern.name();
        case Qt::UserRole:
            return QVariant::fromValue(pattern.geometry());
        default:
            return QVariant();
    }
}

// Method to add a pattern
void PatternModel::addPattern(const Pattern &pattern) {
    beginInsertRows(QModelIndex(), m_patterns.size(), m_patterns.size());
    m_patterns.append(pattern);
    endInsertRows();
}

// Method to remove a pattern
void PatternModel::removePattern(int index) {
    if (index >= 0 && index < m_patterns.size()) {
        beginRemoveRows(QModelIndex(), index, index);
        m_patterns.remove(index);
        endRemoveRows();
    }
}

// Method to get a pattern
Pattern PatternModel::getPattern(int index) const {
    if (index >= 0 && index < m_patterns.size()) {
        return m_patterns[index];
    }
    return Pattern(); // Return a default Pattern if index is out of bounds
}

// Method to get all patterns
QVector<Pattern> PatternModel::getAllPatterns() const {
    return m_patterns;
}

// Method to clear all patterns
void PatternModel::clearPatterns() {
    if (!m_patterns.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_patterns.size() - 1);
        m_patterns.clear();
        endRemoveRows();
    }
}