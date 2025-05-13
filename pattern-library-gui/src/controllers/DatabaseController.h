#ifndef DATABASECONTROLLER_H
#define DATABASECONTROLLER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlError>

class DatabaseController {
public:
    explicit DatabaseController(const QString& dbName = "pattern_library");
    ~DatabaseController();

    bool connect();
    void disconnect();
    QSqlQuery executeQuery(const QString& queryStr);
    QSqlError lastError() const { return m_lastError; }
    bool isConnected() const { return m_db.isOpen(); }

private:
    QSqlDatabase m_db;
    QSqlError m_lastError;
    QString m_dbName;
};

#endif // DATABASECONTROLLER_H