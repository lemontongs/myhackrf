#include "receiver.h"

#include <QByteArray>
#include <QDebug>
#include <QThread>

Receiver::Receiver()
{
    isRunning = false;
    data_target = "tcp://192.168.1.174:5555";
    comm_target = "tcp://192.168.1.174:5556";

    // Setup the data receiver
    comm_context = new zmq::context_t(1);
    comm_socket = new zmq::socket_t(*comm_context, ZMQ_REQ);
    comm_socket->connect(comm_target.toLocal8Bit().data());

    updateParameters();
}

void Receiver::stop()
{
    isRunning = false;
}

void Receiver::updateParameters()
{
    qDebug() << "Requesting Fc...";
    s_send(*comm_socket, std::string("get-fc"));
    std::stringstream ss1(s_recv(*comm_socket));
    ss1 >> fc_hz;
    qDebug() << "Got: " << fc_hz;

    qDebug() << "Requesting Fs...";
    s_send(*comm_socket, std::string("get-fs"));
    std::stringstream ss2(s_recv(*comm_socket));
    ss2 >> fs_hz;
    qDebug() << "Got: " << fs_hz;
}

void Receiver::receivePackets()
{
    // Setup the data receiver
    zmq::context_t recv_context(1);
    zmq::socket_t socket(recv_context, ZMQ_SUB);
    socket.connect(data_target.toLocal8Bit().data());
    socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    int timeout = 100; // ms
    socket.setsockopt(ZMQ_RCVTIMEO, &timeout, sizeof(timeout));

    isRunning = true;
    while (isRunning)
    {
        zmq::message_t msg;
        if ( socket.recv(&msg) )
        {
            char* data = static_cast<char*>(msg.data());
            QByteArray ba(data, msg.size());
            emit newPacket(ba);
        }
    }

    // Send quit message

}


void Receiver::tune(u_int64_t fc_hz)
{
    QString command = QString("tune ") + QString::number(fc_hz);
    s_send(*comm_socket, command.toStdString());
    std::string response = s_recv(*comm_socket);

    updateParameters();
}