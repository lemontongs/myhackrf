#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QByteArray>
#include <QMainWindow>
#include <QThread>
#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>

#include "packet.pb.h"
#include "receiver.h"

namespace Ui {
class MainWindow;
}

class SpectrogramData;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void handleNewPacket(Packet p);
    void handleNewParameters(uint64_t fc_hz, uint64_t fs_hz, QString host);

signals:
    void startReceivingPackets(void);

private slots:
    void on_pushButton_setIP_clicked();
    void on_pushButton_tune_clicked();
    void on_pushButton_setSampleRate_clicked();

private:

    void closeEvent(QCloseEvent *event);

    Ui::MainWindow *ui;

    Receiver m_receiver;
    QThread m_receiveThread;

    QwtPlot * p1;
    QwtPlotCurve d_curve;

    QwtPlot * p2;
    QwtPlotSpectrogram *d_spectrogram;
    SpectrogramData* m_specData;

    QVector<QPointF> data;
    int num_fft_bins;
    std::vector<double> freq_bins_mhz;
};

#endif // MAINWINDOW_H
