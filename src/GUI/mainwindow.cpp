#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QColor>
#include <qwt_symbol.h>
#include <math.h>
#include <stdint.h>
#include <deque>

class SpectrogramData: public QwtRasterData
{
public:
    SpectrogramData()
    {
        setInterval( Qt::XAxis, QwtInterval( 0, 1 ) );
        setInterval( Qt::YAxis, QwtInterval( 0, 100 ) );
        setInterval( Qt::ZAxis, QwtInterval( -50.0, 10.0 ) );
    }

    std::deque<Packet> m_packets;

    void addPacket(Packet p)
    {
        m_packets.push_front(p);
        if ( m_packets.size() > 100 )
            m_packets.pop_back();
        
        setInterval( Qt::XAxis, QwtInterval( 0, p.fft_packet().fft().size() ) );
    }

    virtual double value( double x, double y ) const
    {
        uint32_t xi = uint32_t(x);
        uint32_t yi = uint32_t(y);
        if ( yi < m_packets.size() && xi < m_packets[yi].fft_packet().fft().size() )
            return m_packets[yi].fft_packet().fft(xi);
        else
            return -50.0;
    }
};

class ColorMap: public QwtLinearColorMap
{
public:
    ColorMap():
        QwtLinearColorMap( Qt::darkCyan, Qt::red )
    {
        addColorStop( 0.1, Qt::cyan );
        addColorStop( 0.6, Qt::green );
        addColorStop( 0.95, Qt::yellow );
    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    num_fft_bins = 2048;

    qRegisterMetaType<Packet>("Packet");

    connect(&m_receiver, SIGNAL(newParameters(uint64_t,uint64_t,QString)), this, SLOT(handleNewParameters(uint64_t,uint64_t,QString)));
    connect(&m_receiver, SIGNAL(newPacket(Packet)), this, SLOT(handleNewPacket(Packet)), Qt::QueuedConnection);
    connect(&m_receiveThread, SIGNAL(finished()), &m_receiveThread, SLOT(deleteLater()));
    connect(this, SIGNAL(startReceivingPackets()), &m_receiver, SLOT(receivePackets()));
    connect(this, SIGNAL(destroyed()), &m_receiver, SLOT(stop()));

    m_receiver.setIP("rpi4");
    m_receiver.initialize();
    m_receiver.moveToThread(&m_receiveThread);
    m_receiveThread.start();

    //
    //  define curve style
    //
    p1 = new QwtPlot();
    p1->setAutoReplot(true);

    d_curve.setPen( Qt::blue );
    d_curve.setStyle( QwtPlotCurve::Lines );
    d_curve.attach(p1);

    ui->gridLayout->addWidget( p1, 3, 0, 1, 3 );

    p2 = new QwtPlot();
    m_specData = new SpectrogramData();
    d_spectrogram = new QwtPlotSpectrogram();
    d_spectrogram->setRenderThreadCount( 0 ); // use system specific thread count
    d_spectrogram->setColorMap( new ColorMap() );
    d_spectrogram->setData( m_specData );
    d_spectrogram->attach( p2 );

    ui->gridLayout->addWidget( p2, 4, 0, 1, 3 );

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

void MainWindow::handleNewParameters(uint64_t fc_hz, uint64_t fs_hz, QString host)
{
    qDebug() << "Got new parameters " << num_fft_bins << " " << fc_hz << " " << fs_hz;
    ui->spinBox_centerFrequency->setValue(fc_hz/1e6);
    ui->spinBox_sampleRate->setValue(fs_hz);
    ui->lineEdit_IPaddr->setText(host);
}

void MainWindow::handleNewPacket(Packet pak)
{
    data.resize(pak.fft_packet().fft_size());
    for (int ii = 0; ii < pak.fft_packet().fft().size(); ii++)
    {
        data[ii] = QPointF( (pak.fft_packet().freq_bins_hz(ii)/1e6), pak.fft_packet().fft(ii));
    }

    p1->setAxisAutoScale(0, false);
    p1->setAxisScale(0, -60.0, 50.0);

    p1->setAxisAutoScale(1, false);
    p1->setAxisScale(1, pak.fft_packet().freq_bins_hz(0), 
	                    pak.fft_packet().freq_bins_hz(pak.fft_packet().freq_bins_hz().size()-1));

    d_curve.setPen( Qt::blue );
    d_curve.setStyle( QwtPlotCurve::Lines );
    d_curve.setSamples( data );

    m_specData->addPacket(pak);
    d_spectrogram->setData( m_specData );
    
    p2->setAxisAutoScale(0, true);
    p2->setAxisAutoScale(1, false);
    p1->setAxisScale(1, 0, pak.fft_packet().freq_bins_hz().size()-1);
    p2->replot();
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

    qDebug() << "Requesting tune to: " << fc_hz;
    m_receiver.tune( fc_hz );
}

void MainWindow::on_pushButton_setSampleRate_clicked()
{
    double fs_hz = ui->spinBox_sampleRate->value();

    m_receiver.setSampleRate( fs_hz );
}
