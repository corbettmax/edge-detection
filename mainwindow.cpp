#include <QtWidgets>
#include <functional>
#include "mainwindow.h"
#include "algorithms.h"
#include <QElapsedTimer> // Include QElapsedTimer


using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    // Set the default directory to the parent directory of the current executable
    QString defaultDir = QDir::currentPath();
    defaultDir = QString("%1/../img").arg(defaultDir);

    filename = QFileDialog::getOpenFileName(this, tr("Select a single image"), defaultDir);
    original = QImage(filename);
    display(original);
    grayscale = original.convertToFormat(QImage::Format_Grayscale8);
    ui->comboBox->setCurrentIndex(0);
}


void MainWindow::display(const QImage& image) {
    scene->clear();
    scene->addPixmap(QPixmap::fromImage(image));
    scene->setSceneRect(image.rect());
    ui->image->setScene(scene);
    ui->image->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    current = image;
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    QElapsedTimer timer;
    timer.start(); // Start the timer

    switch (index) {
    case 0:
        display(original);
        break;
    case 1:
        display(canny(grayscale, 1, 40, 120));
        break;
    default:
        function<QImage(const QImage&)> functions[] = {
            sobel,
            prewitt,
            roberts,
            scharr
        };
        display(functions[index - 2](grayscale));
        break;
    }

    qint64 elapsed = timer.elapsed(); // Get elapsed time

    // Update the QLabel with the elapsed time
    ui->labelElapsedTime->setText(QString("Time to apply algorithm: %1 ms").arg(elapsed));
}


void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    ui->image->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}


void MainWindow::on_pushButton_2_clicked()
{
    auto path = QFileDialog::getExistingDirectory(this, tr("Destination"));
    auto name = QString("%1_%2").arg(ui->comboBox->currentText(), QFileInfo(filename).fileName());
    current.save(QDir(path).filePath(name));
}
