// filepath: /pattern-library-gui/src/ui/PatternBrowser.h

#ifndef PATTERNBROWSER_H
#define PATTERNBROWSER_H

#include <QWidget>
#include <QPushButton>
#include <QListView>
#include <QVBoxLayout>
#include "../models/PatternModel.h"

class PatternBrowser : public QWidget {
    Q_OBJECT

public:
    explicit PatternBrowser(QWidget *parent = nullptr);
    void displayPatterns(const QStringList &patterns);
    
signals:
    void patternSelected(const QString &pattern);

private slots:
    void onPatternClicked(const QModelIndex &index);
    void onImportPattern();
    void onExportPattern();

private:
    QPushButton *importButton;
    QPushButton *exportButton;
    QListView *patternListView;
    QVBoxLayout *layout;
    PatternModel *patternModel;
};

#endif // PATTERNBROWSER_H