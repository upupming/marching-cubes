#include "main_window.h"

MainWindow::MainWindow(bool autoTest) : autoTest(autoTest) {
    readDataProcess = QtConcurrent::run(this, &MainWindow::readData);
    readSettings();
    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    QVBoxLayout *vBoxLayout = new QVBoxLayout;
    QWidget *mWidget = new QWidget;

    meshViewWidget = new MeshViewWidget;

    slider = new QSlider;
    slider->setOrientation(Qt::Orientation::Horizontal);
    slider->setFocusPolicy(Qt::StrongFocus);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setTickInterval(100);
    slider->setSingleStep(1);
    slider->setMaximum(2000);

    QLabel *label = new QLabel;
    label->setText("isoValue: 0");
    label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    vBoxLayout->addWidget(meshViewWidget);
    hBoxLayout->addWidget(slider);
    hBoxLayout->addWidget(label);
    vBoxLayout->addLayout(hBoxLayout);

    mWidget->setLayout(vBoxLayout);
    setCentralWidget(mWidget);

    connect(this, &MainWindow::marchingCubesFinished,
            this, &MainWindow::updateMeshView);
    connect(slider, &QSlider::sliderReleased,
            this, [=]() {
                updateIsoValue(slider->value());
            });
    connect(slider, &QSlider::valueChanged,
            this, [=](int value) {
                label->setText(QString("isoValue: ") + QString::number(value));
            });

    updateIsoValue(800);
}

MainWindow::~MainWindow() {
    delete rawReader;
    delete mc;
}

void MainWindow::readData() {
    rawReader = new RawReader("../../data/cbct_sample_z=507_y=512_x=512.raw", Z, Y, X);
    std::array<int, 3> dim{Z, Y, X};
    std::array<float, 3> spacing{0.3f, 0.3f, 0.3f};
    mc = new MarchingCubes(rawReader->data(), dim, spacing, true);
}

// 必须在 GUI 线程里面更新 OpenGL 不然会报错，因为 context 不同了
void MainWindow::updateMeshView() {
    meshViewWidget->setMarchingCubes(mc);
    meshViewWidget->update();

    // auto test with random isoValue
    if (autoTest) {
        float isoValue;
        do {
            isoValue = QRandomGenerator::global()->generate() % 2000;
        } while (isoValue == currentIsoValue);
        updateIsoValue(isoValue);
    }
}

void MainWindow::updateIsoValue(float isoValue) {
    std::cout << "void MainWindow::updateIsoValue(float isoValue): " << isoValue << std::endl;
    if (currentIsoValue == isoValue) return;
    // 正在运行中的话，不允许更新
    if (mcProcess.isStarted() && !mcProcess.isFinished()) {
        std::cout << "skip cause process is running" << std::endl;
        return;
    }
    currentIsoValue = isoValue;
    // 直接调用 updateIsoValue 的话，UI 记得更新
    if (slider->value() != currentIsoValue) {
        slider->setValue(currentIsoValue);
    }
    mcProcess = QtConcurrent::run(this, &MainWindow::runMarchingCubes, currentIsoValue);
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

void MainWindow::runMarchingCubes(float isoValue) {
    readDataProcess.waitForFinished();

    mc->runAlgorithm(isoValue);
    mc->saveObj("../../data/test.obj");

    emit marchingCubesFinished();
}
