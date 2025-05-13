// filepath: /pattern-library-gui/src/models/PatternModel.h

#ifndef PATTERNMODEL_H
#define PATTERNMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVector>
#include <QPolygonF>

class Pattern {
public:
    Pattern() = default;
    Pattern(const QString& name, const QPolygonF& geometry = QPolygonF())
        : m_name(name), m_geometry(geometry) {}

    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name; }
    
    QPolygonF geometry() const { return m_geometry; }
    void setGeometry(const QPolygonF& geometry) { m_geometry = geometry; }

private:
    QString m_name;
    QPolygonF m_geometry;
};

class PatternModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit PatternModel(QObject* parent = nullptr);
    ~PatternModel() override;

    // QAbstractListModel overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    // Pattern management
    void addPattern(const Pattern& pattern);
    void removePattern(int index);
    Pattern getPattern(int index) const;
    QVector<Pattern> getAllPatterns() const;
    void clearPatterns();

private:
    QVector<Pattern> m_patterns;
};

#endif // PATTERNMODEL_H