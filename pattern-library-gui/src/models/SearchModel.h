#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <QList>
#include <QSqlDatabase>
#include <QDateTime> // Added for QDateTime

struct SearchResult {
    QString name;
    QString id;
    QString description;
    QString path;
    QDateTime lastModified;
};

class SearchModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit SearchModel(QObject* parent = nullptr);
    ~SearchModel() override;

    // QAbstractTableModel overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    enum Column {
        Name,
        Id,
        Description,
        Path,
        LastModified,
        ColumnCount
    };

public slots:
    void executeSearch(const QString& criteria);

private:
    QList<SearchResult> searchResults;
    QSqlDatabase m_db;
    bool connectToDatabase();
    void setupSearchTable();

};

#endif // SEARCHMODEL_H