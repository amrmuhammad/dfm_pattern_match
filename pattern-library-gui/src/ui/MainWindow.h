#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../models/PatternModel.h"
#include "PatternViewer.h"

class QListWidgetItem; // Forward declaration

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onImportPattern();
    void onExportPattern();
    void onPatternSelected(const QModelIndex& index);
    void on_actionOpenPatternLibrary_triggered();
    void onLayerListItemChanged(QListWidgetItem *item);

private:
    void setupUi();
    void createConnections();

    Ui::MainWindow *ui;
    PatternModel *m_patternModel;
    PatternViewer *m_patternViewer;
};

#endif // MAINWINDOW_H