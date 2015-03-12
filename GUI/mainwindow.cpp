#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QColor>
#include <qwt_symbol.h>

#include <fftw3.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&m_receiver, SIGNAL(newPacket(QByteArray)), this, SLOT(handleNewPacket(QByteArray)), Qt::QueuedConnection);
    connect(&m_receiveThread, SIGNAL(finished()), &m_receiveThread, SLOT(deleteLater()));
    connect(this, SIGNAL(startReceivingPackets()), &m_receiver, SLOT(receivePackets()));
    connect(this, SIGNAL(destroyed()), &m_receiver, SLOT(stop()));

    m_receiver.moveToThread(&m_receiveThread);
    m_receiveThread.start();

    //
    //  define curve style
    //
    p = new QwtPlot();
    p->setAutoReplot(true);
    //p->setAxisAutoScale(0,false);
    //p->setAxisScale(0, -1.0, 1.0);

    // Frequency Bins
    // f = -(N/2):(N/2)-1;
    // f = f*fs/N;
    // f = f + fc;
    // f = f / 1e6;
    int N = 262144;
    double fs = double(1000000.0);
    double fc = double(433900000.0);
    double fs_over_N = double(fs/N);
    freq_bins_mhz.clear();
    for (int ii = (-N/2); ii < ((N/2)-1); ii++)
        freq_bins_mhz.push_back( ( ii * fs_over_N + fc ) / double(1e6) );


    d_curve.setPen( Qt::blue );
    d_curve.setStyle( QwtPlotCurve::Lines );
    d_curve.attach(p);

    this->setCentralWidget( p );

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

void MainWindow::handleNewPacket(QByteArray packet)
{
    int N = packet.size()/2;

    fftw_complex *in, *out;
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);

    for (int ii = 0; ii < packet.size(); ii+=2)
    {
        in[ii/2][0] = ( double(packet[ii+0] + u_int8_t(128) ) - double(128) ) / double(128);
        in[ii/2][1] = ( double(packet[ii+1] + u_int8_t(128) ) - double(128) ) / double(128);
    }

    fftw_plan my_plan;
    my_plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(my_plan);

    data.resize(N);
    for (int ii = 0; ii < N; ii++)
    {
        int idx = ii+(N/2); // This effectively does an fftshift
        if (ii >= N)
            idx = idx - N;

        qDebug() << idx;
        data[ii] = QPointF(freq_bins_mhz[ii],out[idx][0]);
    }

    fftw_destroy_plan(my_plan);
    fftw_free(in);
    fftw_free(out);

    d_curve.setPen( Qt::blue );
    d_curve.setStyle( QwtPlotCurve::Lines );
    d_curve.setSamples( data );
}
