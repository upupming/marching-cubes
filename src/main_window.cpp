#include "main_window.h"

#include <QtWidgets>
MainWindow::MainWindow() {
    readSettings();
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    QWidget *mWidget = new QWidget;
    QSlider *slider = new QSlider;
    slider->setOrientation(Qt::Orientation::Horizontal);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(10);
    slider->setSingleStep(1);
    vBoxLayout->addWidget(slider);
    mWidget->setLayout(vBoxLayout);
    setCentralWidget(mWidget);
}

void MainWindow::readSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings() {
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

void MainWindow::closeEvent(QCloseEvent *event) {
    writeSettings();
    event->accept();
}
