#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "PatternViewer.h"
#include "../utils/FileHandler.h"
#include "../models/PatternModel.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument> // For parsing JSONB properties // Added QDebug include

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_patternModel(new PatternModel(this))
    , m_patternViewer(new PatternViewer(this))
{
    ui->setupUi(this);
    setupUi();
    createConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUi()
{
    ui->patternListView->setModel(m_patternModel);
    ui->patternListView->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Add pattern viewer to the preview container
    QVBoxLayout* previewLayout = new QVBoxLayout(ui->previewContainer);
    previewLayout->addWidget(m_patternViewer);
    ui->previewContainer->setLayout(previewLayout);
}

void MainWindow::createConnections()
{
    connect(ui->actionImport, &QAction::triggered, this, &MainWindow::onImportPattern);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::onExportPattern);
    connect(ui->importButton, &QPushButton::clicked, this, &MainWindow::onImportPattern);
    connect(ui->exportButton, &QPushButton::clicked, this, &MainWindow::onExportPattern);
    connect(ui->patternListView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onPatternSelected);
}

void MainWindow::onImportPattern()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Import Pattern"), QString(),
        tr("OASIS Files (*.oas);;GDSII Files (*.gds);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    try {
        Pattern pattern;
        if (fileName.endsWith(".oas", Qt::CaseInsensitive)) {
            pattern = FileHandler::loadFromOasis(fileName);
        } else if (fileName.endsWith(".gds", Qt::CaseInsensitive)) {
            pattern = FileHandler::loadFromGds(fileName);
        } else {
            throw std::runtime_error("Unsupported file format");
        }
        
        m_patternModel->addPattern(pattern);
        statusBar()->showMessage(tr("Pattern imported successfully"), 3000);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Import Error"),
            tr("Failed to import pattern: %1").arg(e.what()));
    }
}

void MainWindow::onExportPattern()
{
    QModelIndex currentIndex = ui->patternListView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, tr("Export Error"),
            tr("Please select a pattern to export"));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export Pattern"), QString(),
        tr("OASIS Files (*.oas);;GDSII Files (*.gds);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    try {
        Pattern pattern = m_patternModel->getPattern(currentIndex.row());
        
        if (fileName.endsWith(".oas", Qt::CaseInsensitive)) {
            FileHandler::saveToOasis(fileName, pattern);
        } else if (fileName.endsWith(".gds", Qt::CaseInsensitive)) {
            FileHandler::saveToGds(fileName, pattern);
        } else {
            throw std::runtime_error("Unsupported file format");
        }
        
        statusBar()->showMessage(tr("Pattern exported successfully"), 3000);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Export Error"),
            tr("Failed to export pattern: %1").arg(e.what()));
    }
}

void MainWindow::onPatternSelected(const QModelIndex& index)
{
    if (index.isValid()) {
        Pattern pattern = m_patternModel->getPattern(index.row());
        m_patternViewer->setPattern(pattern.geometry());
    } else {
        m_patternViewer->setPattern(QPolygonF());
    }
}

void MainWindow::on_actionOpenPatternLibrary_triggered()
{
    qDebug() << "Action 'Open a pattern library' triggered!";

    QSqlDatabase db = QSqlDatabase::database(); // Get the existing connection if already added, or add a new one.
    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QPSQL");
        db.setHostName("localhost");
        db.setDatabaseName("patterns_library");
        db.setUserName("test123");
        db.setPassword("test123");
    }

    bool connectionOpen = db.isOpen();
    if (!connectionOpen) {
        connectionOpen = db.open();
    }

    if (!connectionOpen) {
        qDebug() << "Database connection failed:";
        qDebug() << db.lastError().text();
        QMessageBox::critical(this, "Database Error",
            tr("Failed to connect to the pattern library database: %1").arg(db.lastError().text()));
        return;
    }

    qDebug() << "Database connection successful! Querying patterns...";
    m_patternModel->clearPatterns(); // Clear existing patterns

    QSqlQuery query(db);
    if (!query.exec("SELECT id, name, description, gds_data, category, tags, properties FROM patterns ORDER BY name")) {
        qDebug() << "Failed to query patterns:";
        qDebug() << query.lastError().text();
        QMessageBox::warning(this, "Query Error",
            tr("Failed to retrieve patterns from the database: %1").arg(query.lastError().text()));
        // db.close(); // Consider if we want to close on query failure or keep open for other ops
        return;
    }

    int patternsLoaded = 0;
    while (query.next()) {
        Pattern pattern;
        pattern.setId(query.value("id").toInt());
        pattern.setName(query.value("name").toString());
        pattern.setDescription(query.value("description").toString());
        pattern.setGdsData(query.value("gds_data").toByteArray());
        pattern.setCategory(query.value("category").toString());
        
        // Handle TEXT[] for tags - QPSQL driver might return QStringList directly or QVariantList
        QVariant tagsVariant = query.value("tags");
        if (tagsVariant.typeId() == QMetaType::QStringList) {
            pattern.setTags(tagsVariant.toStringList());
        } else if (tagsVariant.typeId() == QMetaType::QVariantList) {
            QStringList tagsList;
            for (const QVariant& item : tagsVariant.toList()) {
                tagsList.append(item.toString());
            }
            pattern.setTags(tagsList);
        } else if (tagsVariant.typeId() == QMetaType::QString) { // Fallback: sometimes it's a string like "{tag1,tag2}"
            QString tagsString = tagsVariant.toString();
            if (tagsString.startsWith('{') && tagsString.endsWith('}')) {
                tagsString = tagsString.mid(1, tagsString.length() - 2);
            }
            if (!tagsString.isEmpty()) {
                 pattern.setTags(tagsString.split(','));
            }
        }

        QByteArray jsonData = query.value("properties").toByteArray();
        if (!jsonData.isNull() && !jsonData.isEmpty()) {
            QJsonDocument doc = QJsonDocument::fromJson(jsonData);
            if (doc.isObject()) {
                pattern.setProperties(doc.object());
            }
        }
        
        m_patternModel->addPattern(pattern);
        patternsLoaded++;
    }

    if (patternsLoaded > 0) {
        QMessageBox::information(this, "Open Pattern Library",
            tr("Successfully loaded %1 patterns from the database.").arg(patternsLoaded));
    } else {
        QMessageBox::information(this, "Open Pattern Library",
            tr("Successfully connected to the database. No patterns found or loaded."));
    }

    // db.close(); // Decide on connection management strategy - often good to keep it open while app runs.
    // For now, the default QSqlDatabase connection will persist until app closes or it's explicitly removed.
}