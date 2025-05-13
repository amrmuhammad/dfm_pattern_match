#include "DatabaseController.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseController::DatabaseController(const QString& dbName) : m_dbName(dbName) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbName);
}

bool DatabaseController::connect() {
    if (!m_db.open()) {
        m_lastError = m_db.lastError();
        qDebug() << "Database connection failed:" << m_lastError.text();
        return false;
    }
    return true;
}

void DatabaseController::disconnect() {
    m_db.close();
}

QSqlQuery DatabaseController::executeQuery(const QString& queryStr) {
    QSqlQuery query;
    query.prepare(queryStr);
    if (!query.exec()) {
        qDebug() << "Query execution failed:" << query.lastError().text();
    }
    return query;
}