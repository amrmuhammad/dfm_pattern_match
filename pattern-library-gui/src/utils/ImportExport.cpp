// filepath: /pattern-library-gui/pattern-library-gui/src/utils/ImportExport.cpp

#include "ImportExport.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPolygonF>
#include <QPointF>
#include <QDebug>

namespace ImportExport {

// Helper functions
QJsonObject patternToJson(const Pattern& pattern) {
    QJsonObject obj;
    obj["name"] = pattern.name();
    
    QJsonArray geometryArray;
    for (const QPointF& point : pattern.geometry()) {
        QJsonObject pointObj;
        pointObj["x"] = point.x();
        pointObj["y"] = point.y();
        geometryArray.append(pointObj);
    }
    obj["geometry"] = geometryArray;
    
    return obj;
}

Pattern jsonToPattern(const QJsonObject& obj) {
    QString name = obj["name"].toString();
    QPolygonF geometry;
    
    QJsonArray geometryArray = obj["geometry"].toArray();
    for (const QJsonValue& value : geometryArray) {
        QJsonObject pointObj = value.toObject();
        double x = pointObj["x"].toDouble();
        double y = pointObj["y"].toDouble();
        geometry.append(QPointF(x, y));
    }
    
    return Pattern(name, geometry);
}

Pattern importPattern(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for reading:" << filePath;
        return Pattern();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(content.toUtf8());
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qDebug() << "Invalid JSON format in file:" << filePath;
        return Pattern();
    }

    return jsonToPattern(jsonDoc.object());
}

bool exportPattern(const QString& filePath, const Pattern& pattern) {
    QJsonObject json = patternToJson(pattern);
    QJsonDocument doc(json);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing:" << filePath;
        return false;
    }
    
    QTextStream out(&file);
    out << doc.toJson();
    file.close();
    
    return true;
}

QList<Pattern> importPatterns(const QString& filePath) {
    QList<Pattern> patterns;
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for reading:" << filePath;
        return patterns;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    QJsonDocument jsonDoc = QJsonDocument::fromJson(content.toUtf8());
    if (jsonDoc.isNull() || !jsonDoc.isArray()) {
        qDebug() << "Invalid JSON format in file:" << filePath;
        return patterns;
    }
    
    QJsonArray jsonArray = jsonDoc.array();
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject()) continue;
        patterns.append(jsonToPattern(value.toObject()));
    }
    
    return patterns;
}

bool exportPatterns(const QString& filePath, const QList<Pattern>& patterns) {
    QJsonArray jsonArray;
    for (const Pattern& pattern : patterns) {
        jsonArray.append(patternToJson(pattern));
    }
    
    QJsonDocument doc(jsonArray);
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file for writing:" << filePath;
        return false;
    }
    
    QTextStream out(&file);
    out << doc.toJson();
    file.close();
    
    return true;
}

} // namespace ImportExport