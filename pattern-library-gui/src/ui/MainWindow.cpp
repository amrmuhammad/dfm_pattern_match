#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "PatternViewer.h"
#include "../utils/FileHandler.h"
#include "../models/PatternModel.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug> // Added QDebug include

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
    // Placeholder for actual database loading logic
    // For example, you might open a dialog to select a library from the DB
    // or directly load a pre-configured one.
    QMessageBox::information(this, "Open Pattern Library", "Functionality to open pattern library from database is not yet implemented.");
}