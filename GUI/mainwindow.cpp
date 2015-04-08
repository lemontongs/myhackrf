#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QColor>
#include <qwt_symbol.h>
#include <math.h>
#include <fftw3.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    num_fft_bins = 2048;

    connect(&m_receiver, SIGNAL(newParameters(double,double)), this, SLOT(handleNewParameters(double,double)));
    connect(&m_receiver, SIGNAL(newPacket(QByteArray)), this, SLOT(handleNewPacket(QByteArray)), Qt::QueuedConnection);
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

    ui->gridLayout->addWidget( p, 2, 0, 1, 3 );

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

void MainWindow::calculateFrequencyBins(int N, double fs, u_int64_t fc)
{
    // Frequency Bins
    // f = -(N/2):(N/2)-1;
    // f = f*fs/N;
    // f = f + fc;
    // f = f / 1e6;
    double fs_over_N = double(fs)/double(N);
    freq_bins_mhz.clear();
    for (int ii = (-N/2); ii < (N/2); ii++)
        freq_bins_mhz.push_back( ( ii * fs_over_N + fc ) / double(1e6) );
}

void MainWindow::handleNewParameters(double fc_hz, double fs_hz)
{
    qDebug() << "Got new parameters " << num_fft_bins << " " << fc_hz << " " << fs_hz;
    calculateFrequencyBins( num_fft_bins, fs_hz, u_int64_t(fc_hz) );
    ui->doubleSpinBox->setValue(fc_hz/1e6);
}

void MainWindow::handleNewPacket(QByteArray packet)
{
    int num_samples  = packet.size()/2;

    fftw_complex *in, *out;
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*num_samples);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*num_fft_bins);

    for (int ii = 0; ii < packet.size(); ii+=2)
    {
        in[ii/2][0] = ( double(packet[ii+0] + u_int8_t(128) ) - double(128) ) / double(128);
        in[ii/2][1] = ( double(packet[ii+1] + u_int8_t(128) ) - double(128) ) / double(128);
    }

    fftw_plan my_plan;
    my_plan = fftw_plan_dft_1d(num_fft_bins, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(my_plan);

    double val_sum = 0.0;
    double val_max = -1000.0;
    data.resize(num_fft_bins);
    for (int ii = 0; ii < num_fft_bins; ii++)
    {
        int idx = ii+(num_fft_bins/2); // This effectively does an fftshift
        if (ii >= (num_fft_bins/2))
            idx = ii - (num_fft_bins/2);

        double val = fabs(out[idx][0]);
        if (0 != val)
            val = 20*log10(val);

        if ( val > val_max )
            val_max = val;
        val_sum += val;

        //qDebug() << abs(out[idx][0]) << " " << log10(abs(out[idx][0])) << " " << 20*log10(abs(out[idx][0]));
        data[ii] = QPointF(freq_bins_mhz[ii], val);
    }

    p->setAxisAutoScale(0,false);
    p->setAxisScale(0, val_sum/num_fft_bins, val_max);

    fftw_destroy_plan(my_plan);
    fftw_free(in);
    fftw_free(out);

    d_curve.setPen( Qt::blue );
    d_curve.setStyle( QwtPlotCurve::Lines );
    d_curve.setSamples( data );
}

void MainWindow::on_pushButton_clicked()
{
    u_int64_t fc_hz = u_int64_t(ui->doubleSpinBox->value() * double(1e6));

    m_receiver.tune( fc_hz );
}

void MainWindow::on_pushButton_2_clicked()
{
    double fs_hz = ui->doubleSpinBox_2->value();

    m_receiver.setSampleRate( fs_hz );
}
