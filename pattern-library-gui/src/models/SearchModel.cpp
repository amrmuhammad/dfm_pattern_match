#include "SearchModel.h"
#include <QSqlQuery>
#include <QVariant>

SearchModel::SearchModel(QObject *parent) : QAbstractTableModel(parent) {
    connectToDatabase();
    setupSearchTable();
}

SearchModel::~SearchModel() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

int SearchModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return searchResults.size(); // Return the number of search results
}

int SearchModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return Column::ColumnCount;
}

QVariant SearchModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        if (index.row() < 0 || index.row() >= searchResults.size())
            return QVariant();

        const auto &result = searchResults.at(index.row());
        switch (index.column()) {
            case Column::Name: return result.name;
            case Column::Id: return result.id;
            case Column::Description: return result.description;
            case Column::Path: return result.path;
            case Column::LastModified: return result.lastModified;
            default: return QVariant();
        }
    }
    return QVariant();
}

QVariant SearchModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case Column::Name: return tr("Name");
            case Column::Id: return tr("ID");
            case Column::Description: return tr("Description");
            case Column::Path: return tr("Path");
            case Column::LastModified: return tr("Last Modified");
            default: return QVariant();
        }
    }
    return QVariant();
}

void SearchModel::executeSearch(const QString &criteria) {
    beginResetModel();
    // Clear previous results
    searchResults.clear();

    // Example: Execute a database query based on the criteria
    if (!m_db.isOpen() && !connectToDatabase()) {
        endResetModel();
        return;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT name, id, description, path, last_modified FROM patterns WHERE name LIKE :criteria OR description LIKE :criteria");
    query.bindValue(":criteria", "%" + criteria + "%");
    
    if (query.exec()) {
        while (query.next()) {
            SearchResult result;
            result.name = query.value(0).toString();
            result.id = query.value(1).toString();
            result.description = query.value(2).toString();
            result.path = query.value(3).toString();
            result.lastModified = query.value(4).toDateTime();
            searchResults.append(result);
        }
    }

    // Notify the model that data has changed
    beginResetModel();
    endResetModel();
}

bool SearchModel::connectToDatabase() {
    m_db = QSqlDatabase::addDatabase("QSQLITE", "search_connection");
    m_db.setDatabaseName("pattern_library.db");
    return m_db.open();
}

void SearchModel::setupSearchTable() {
    if (!m_db.isOpen() && !connectToDatabase()) {
        return;
    }

    QSqlQuery query(m_db);
    query.exec(
        "CREATE TABLE IF NOT EXISTS patterns ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "description TEXT,"
        "path TEXT NOT NULL,"
        "last_modified DATETIME"
        ")"
    );
}