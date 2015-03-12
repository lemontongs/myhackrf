#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QByteArray>
#include <QMainWindow>
#include <QThread>
#include <qwt_plot_curve.h>
#include <qwt_plot.h>

#include "receiver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void handleNewPacket(QByteArray packet);

signals:
    void startReceivingPackets(void);

private:

    void closeEvent(QCloseEvent *event);

    Ui::MainWindow *ui;

    Receiver m_receiver;
    QThread m_receiveThread;

    QwtPlotCurve d_curve;
    QwtPlot * p;

    QVector<QPointF> data;
    std::vector<double> freq_bins_mhz;
};

#endif // MAINWINDOW_H
