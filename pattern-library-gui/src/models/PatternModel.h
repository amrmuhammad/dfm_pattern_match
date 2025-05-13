// filepath: /pattern-library-gui/src/models/PatternModel.h

#ifndef PATTERNMODEL_H
#define PATTERNMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVector>
#include <QPolygonF>
#include <QByteArray>    // For gds_data
#include <QStringList>   // For tags
#include <QJsonObject>   // For properties

class Pattern {
public:
    Pattern() : m_id(-1) {}; // Default constructor, invalid ID
    // Constructor for basic name and geometry (existing)
    Pattern(const QString& name, const QPolygonF& geometry = QPolygonF())
        : m_id(-1), m_name(name), m_geometry(geometry) {}

    // Getters
    int id() const { return m_id; }
    QString name() const { return m_name; }
    QString description() const { return m_description; }
    QByteArray gdsData() const { return m_gdsData; }
    QString category() const { return m_category; }
    QStringList tags() const { return m_tags; }
    QJsonObject properties() const { return m_properties; }
    QPolygonF geometry() const { return m_geometry; } // Assuming geometry is still needed, maybe derived from gds_data later?

    // Setters
    void setId(int id) { m_id = id; }
    void setName(const QString& name) { m_name = name; }
    void setDescription(const QString& description) { m_description = description; }
    void setGdsData(const QByteArray& gdsData) { m_gdsData = gdsData; }
    void setCategory(const QString& category) { m_category = category; }
    void setTags(const QStringList& tags) { m_tags = tags; }
    void setProperties(const QJsonObject& properties) { m_properties = properties; }
    void setGeometry(const QPolygonF& geometry) { m_geometry = geometry; }

private:
    int m_id;
    QString m_name;
    QString m_description;
    QByteArray m_gdsData;
    QString m_category;
    QStringList m_tags;
    QJsonObject m_properties;
    QPolygonF m_geometry; // Retained for now, consider if it's populated from gds_data
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