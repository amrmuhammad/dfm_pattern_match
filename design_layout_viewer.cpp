// design_layout_viewer.cpp
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <vector>
#include <gdstk/gdstk.hpp>

class LayoutWidget : public QWidget {
    Q_OBJECT
public:
    std::vector<gdstk::Polygon*> polygons;
    double zoom = 1.0;
    QPointF pan = QPointF(0, 0);

    LayoutWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(800, 600);
        setBackgroundRole(QPalette::Base);
        setAutoFillBackground(true);
    }

    void load_oasis(const char* filename) {
        gdstk::Library lib;
        lib.read_oas(filename);
        polygons.clear();
        for (size_t i = 0; i < lib.cell_array.count; ++i) {
            gdstk::Cell* cell = lib.cell_array[i];
            for (size_t j = 0; j < cell->polygon_array.count; ++j) {
                polygons.push_back(cell->polygon_array[j]);
            }
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), Qt::white);
        p.save();
        p.translate(width() / 2 + pan.x(), height() / 2 + pan.y());
        p.scale(zoom, -zoom); // Flip Y for layout convention

        QPen pen(Qt::black);
        pen.setWidthF(0.1 / zoom);
        p.setPen(pen);

        for (const auto& poly : polygons) {
            QPolygonF qpoly;
            for (size_t i = 0; i < poly->point_array.count; ++i) {
                qpoly << QPointF(poly->point_array[i].x, poly->point_array[i].y);
            }
            p.drawPolygon(qpoly);
        }
        p.restore();
    }

    void wheelEvent(QWheelEvent* event) override {
        double factor = (event->angleDelta().y() > 0) ? 1.2 : 1.0 / 1.2;
        zoom *= factor;
        update();
    }

    void mousePressEvent(QMouseEvent* event) override {
        last_mouse = event->pos();
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (event->buttons() & Qt::LeftButton) {
            QPointF delta = event->pos() - last_mouse;
            pan += delta;
            last_mouse = event->pos();
            update();
        }
    }

private:
    QPoint last_mouse;
};

// Main window with file open dialog
class MainWindow : public QWidget {
    Q_OBJECT
public:
    LayoutWidget* view;
    MainWindow() {
        view = new LayoutWidget(this);
        setWindowTitle("Design Layout Viewer");
        resize(900, 700);
        if (QFileDialog dialog(this); dialog.exec() == QDialog::Accepted) {
            QString file = dialog.selectedFiles().first();
            try {
                view->load_oasis(file.toStdString().c_str());
            } catch (...) {
                QMessageBox::critical(this, "Error", "Failed to load OASIS file.");
            }
        }
    }
protected:
    void resizeEvent(QResizeEvent*) override {
        view->setGeometry(rect());
    }
};

#include "design_layout_viewer.moc"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

