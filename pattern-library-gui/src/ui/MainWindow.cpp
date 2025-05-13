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
#include <QJsonDocument> // For parsing JSONB properties
#include <QTemporaryFile> // For GDS parsing from QByteArray
#include <QListWidget> // Add this for QListWidget and QListWidgetItem
#include <gdstk/gdstk.hpp> // Make sure this is the correct include for gdstk main functionalities

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
    if (ui->layerListWidget) { // Check if it's available
        connect(ui->layerListWidget, &QListWidget::itemChanged, this, &MainWindow::onLayerListItemChanged);
    }
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
    if (ui->layerListWidget) {
        ui->layerListWidget->clear();
    }
    if (!index.isValid()) {
        m_patternViewer->setPattern(QMap<int, QList<QPolygonF>>()); // Clear viewer if no selection
        return;
    }

    Pattern selectedPattern = m_patternModel->getPattern(index.row());
    QByteArray gdsBytes = selectedPattern.gdsData();
    QMap<int, QList<QPolygonF>> displayLayerGeometries; // Map to store polygons grouped by layer

    if (gdsBytes.isEmpty()) {
        qDebug() << "Selected pattern has no GDS data.";
        if (ui->layerListWidget) {
            ui->layerListWidget->clear();
        }
        m_patternViewer->setPattern(QMap<int, QList<QPolygonF>>()); // Clear viewer with empty list
        return;
    }

    QTemporaryFile tempFile;
    if (tempFile.open()) {
        qint64 bytesWritten = tempFile.write(gdsBytes);
        if (bytesWritten != gdsBytes.size()) {
            qDebug() << "Error writing all GDS data to temporary file:" << tempFile.errorString();
            if (ui->layerListWidget) {
                ui->layerListWidget->clear();
            }
            m_patternViewer->setPattern(QMap<int, QList<QPolygonF>>()); // Clear viewer with empty list on error
            tempFile.close();
            return;
        }
        tempFile.close(); // Close the file so gdstk can open it for reading

        gdstk::Library lib;
        gdstk::ErrorCode error_code = gdstk::ErrorCode::NoError;
        
        lib = gdstk::read_gds(
            tempFile.fileName().toStdString().c_str(),
            0.0,     // unit (0.0 means use file's unit)
            0.001,   // tolerance
            nullptr, // shape_tags
            &error_code
        );

        // QList<QPolygonF> displayGeometries; // Already declared at the start of the function

        if (error_code == gdstk::ErrorCode::NoError) {
            if (lib.cell_array.count > 0) {
                gdstk::Cell* cell = lib.cell_array[0]; // Get the first cell
                if (cell) { // Check if cell is not null
                    for (size_t i = 0; i < cell->polygon_array.count; ++i) {
                        gdstk::Polygon* poly = cell->polygon_array[i];
                        if (poly) { // Check if polygon is not null
                            int layer = gdstk::get_layer(poly->tag);
                            displayLayerGeometries[layer].append(FileHandler::convertGdstkPolygonToQt(poly));
                        }
                    }
                    if (displayLayerGeometries.isEmpty() && cell->polygon_array.count > 0) {
                         qDebug() << "First cell has polygons, but conversion might have failed or all were null.";
                    } else if (cell->polygon_array.count == 0) {
                        qDebug() << "First cell in GDS data has no polygons.";
                    }
                } else {
                    qDebug() << "First cell in GDS data is null.";
                }
            } else {
                qDebug() << "Selected GDS data has no cells.";
            }
            lib.clear(); // Don't forget to clear the library to free memory
        } else {
            qDebug() << "gdstk failed to read GDS from temporary file. Error code:" << static_cast<int>(error_code) << "File:" << tempFile.fileName();
            // tempFile is automatically removed when it goes out of scope, but if gdstk errored,
            // it might be useful to know the temp file wasn't immediately deleted if autoRemove is on.
            // QTemporaryFile by default has autoRemove ON.
        }
        
        m_patternViewer->setPattern(displayLayerGeometries); // Set map of layer polygons

        if (ui->layerListWidget) {
            ui->layerListWidget->clear(); // Clear previous layer list
            if (!displayLayerGeometries.isEmpty()) {
                // Block signals temporarily to prevent itemChanged from firing during population
                bool oldSignalsBlocked = ui->layerListWidget->blockSignals(true);

                QList<int> layerKeys = displayLayerGeometries.keys();
                std::sort(layerKeys.begin(), layerKeys.end()); // Sort keys for consistent order

                for (int layerNum : layerKeys) {
                    QListWidgetItem *item = new QListWidgetItem(QString("Layer %1").arg(layerNum), ui->layerListWidget);
                    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                    item->setCheckState(Qt::Checked); // Initially checked
                    item->setData(Qt::UserRole, layerNum);
                }
                m_patternViewer->setAllLayersVisibility(true); // Sync viewer state

                ui->layerListWidget->blockSignals(oldSignalsBlocked); // Restore signals
            }
        }

    } else {
        qDebug() << "Failed to open temporary file:" << tempFile.errorString();
        if (ui->layerListWidget) {
            ui->layerListWidget->clear();
        }
        m_patternViewer->setPattern(QMap<int, QList<QPolygonF>>()); // Clear viewer with empty list on error
        return;
    }
    // tempFile is automatically removed when it goes out of scope if open() succeeded.
}

void MainWindow::onLayerListItemChanged(QListWidgetItem *item)
{
    if (!item || !m_patternViewer) return; // Basic safety check

    int layerNum = item->data(Qt::UserRole).toInt();
    bool isVisible = (item->checkState() == Qt::Checked);
    m_patternViewer->setLayerVisibility(layerNum, isVisible);
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