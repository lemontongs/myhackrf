#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>

#include "zhelpers.h"

class Receiver : public QObject
{
    Q_OBJECT

public:
    Receiver();
    void tune(int hertz);

public slots:
    void receivePackets();
    void stop();

signals:
    void newPacket(QByteArray packet);

private:
    bool isRunning;
    QString data_target;
    QString comm_target;

    zmq::context_t * comm_context;
    zmq::socket_t * comm_socket;
};

#endif // RECEIVER_H
