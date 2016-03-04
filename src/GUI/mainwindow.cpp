#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QColor>
#include <qwt_symbol.h>
#include <math.h>
#include <stdint.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    num_fft_bins = 2048;

    qRegisterMetaType<Packet>("Packet");

    connect(&m_receiver, SIGNAL(newParameters(double,double,QString)), this, SLOT(handleNewParameters(double,double,QString)));
    connect(&m_receiver, SIGNAL(newPacket(Packet)), this, SLOT(handleNewPacket(Packet)), Qt::QueuedConnection);
    connect(&m_receiveThread, SIGNAL(finished()), &m_receiveThread, SLOT(deleteLater()));
    connect(this, SIGNAL(startReceivingPackets()), &m_receiver, SLOT(receivePackets()));
    connect(this, SIGNAL(destroyed()), &m_receiver, SLOT(stop()));

    m_receiver.initialize();
    m_receiver.moveToThread(&m_receiveThread);
    m_receiveThread.start();

    //
    //  define curve style
    //
    p = new QwtPlot();
    p->setAutoReplot(true);

    d_curve.setPen( Qt::blue );
    d_curve.setStyle( QwtPlotCurve::Lines );
    d_curve.attach(p);

    ui->gridLayout->addWidget( p, 3, 0, 1, 3 );

    emit startReceivingPackets();
}

MainWindow::~MainWindow()
{
    m_receiveThread.quit();
    if (!m_receiveThread.wait(1000))
        m_receiveThread.terminate();

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    (void)event;
    m_receiver.stop();
}

void MainWindow::handleNewParameters(double fc_hz, double fs_hz, QString host)
{
    qDebug() << "Got new parameters " << num_fft_bins << " " << fc_hz << " " << fs_hz;
    ui->spinBox_centerFrequency->setValue(fc_hz/1e6);
    ui->spinBox_sampleRate->setValue(fs_hz);
    ui->lineEdit_IPaddr->setText(host);
}

void MainWindow::handleNewPacket(Packet pak)
{
    data.resize(pak.signal_size());
    for (int ii = 0; ii < pak.signal_size(); ii++)
    {
        data[ii] = QPointF( (pak.freq_bins_mhz(ii)/1e6), pak.signal(ii));
    }

    p->setAxisAutoScale(0,false);
    p->setAxisScale(0, -60.0, 50.0);

    d_curve.setPen( Qt::blue );
    d_curve.setStyle( QwtPlotCurve::Lines );
    d_curve.setSamples( data );
}

void MainWindow::on_pushButton_setIP_clicked()
{
    QString host = ui->lineEdit_IPaddr->text();

    m_receiver.stop();
    m_receiver.setIP(host);
    m_receiver.initialize();
    emit startReceivingPackets();
}

void MainWindow::on_pushButton_tune_clicked()
{
    u_int64_t fc_hz = u_int64_t(ui->spinBox_centerFrequency->value() * double(1e6));

    m_receiver.tune( fc_hz );
}

void MainWindow::on_pushButton_setSampleRate_clicked()
{
    double fs_hz = ui->spinBox_sampleRate->value();

    m_receiver.setSampleRate( fs_hz );
}
