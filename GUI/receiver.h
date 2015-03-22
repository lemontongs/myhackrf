#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>

#include "zhelpers.h"

class Receiver : public QObject
{
    Q_OBJECT

public:
    Receiver();
    void tune(u_int64_t fc_hz);

public slots:
    void receivePackets();
    void stop();

signals:
    void newPacket(QByteArray packet);

private:
    void updateParameters();

    bool isRunning;
    QString data_target;
    QString comm_target;

    zmq::context_t * comm_context;
    zmq::socket_t * comm_socket;

    u_int64_t fc_hz;    // center freq
    double    fs_hz;    // sample rate
    u_int32_t lna_gain; // gain
};

#endif // RECEIVER_H
