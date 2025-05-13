// filepath: /pattern-library-gui/src/ui/PatternBrowser.cpp

#include "PatternBrowser.h"
#include <QVBoxLayout>
#include <QListView>
#include <QPushButton>

PatternBrowser::PatternBrowser(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    patternListView = new QListView(this);
    layout->addWidget(patternListView);
    
    QPushButton *importButton = new QPushButton("Import Pattern", this);
    layout->addWidget(importButton);
    
    QPushButton *exportButton = new QPushButton("Export Pattern", this);
    layout->addWidget(exportButton);
    
    connect(importButton, &QPushButton::clicked, this, &PatternBrowser::onImportPattern);
    connect(exportButton, &QPushButton::clicked, this, &PatternBrowser::onExportPattern);
    connect(patternListView, &QListView::clicked, this, &PatternBrowser::onPatternClicked); // Added connection
}

void PatternBrowser::displayPatterns(const QStringList &patterns) {
    // Code to display patterns in the patternListView
}

void PatternBrowser::onImportPattern() {
    // Code to handle pattern import
}

void PatternBrowser::onExportPattern() {
    // Code to handle pattern export
}

void PatternBrowser::onPatternClicked(const QModelIndex &index) {
    // Implementation for when a pattern is clicked
    if (index.isValid()) {
        QString patternName = patternModel->data(index, Qt::DisplayRole).toString();
        emit patternSelected(patternName);
    }
}