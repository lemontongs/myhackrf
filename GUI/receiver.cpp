#include "receiver.h"

#include <QByteArray>
#include <QDebug>
#include <QThread>

Receiver::Receiver()
{
    isRunning = false;
    isInitialized = false;
    data_port = "5555";
    comm_port = "5556";
    data_target = "tcp://192.168.1.174:" + data_port;
    comm_target = "tcp://192.168.1.174:" + comm_port;

    // Setup the data receiver
    comm_context = new zmq::context_t(1);
    comm_socket = new zmq::socket_t(*comm_context, ZMQ_REQ);
}

void Receiver::setIP(QString ipAddress)
{
    data_target = "tcp://" + ipAddress + ":" + data_port;
    comm_target = "tcp://" + ipAddress + ":" + comm_port;
}

bool Receiver::initialize()
{
    if(!isInitialized)
    {
        try
        {
            comm_socket->connect(comm_target.toLocal8Bit().data());
        }
        catch (zmq::error_t &e)
        {
            qDebug() << QString(e.what());
            isInitialized = false;
            return false;
        }

        isInitialized = true;
        updateParameters();
        return true;
    }
    return false;
}

void Receiver::stop()
{
    isRunning = false;

    while(isInitialized)
        sleep(1);

    comm_socket->close();
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

    emit newParameters(double(fc_hz), fs_hz);
    qDebug() << "Emitting new parameters";
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
            char* data_p = static_cast<char*>(msg.data());
            std::string data(data_p, msg.size());

            Packet p;
            p.ParseFromString(data);

            emit newPacket(p);
        }
    }

    isInitialized = false;
}


void Receiver::tune(u_int64_t fc_hz)
{
    QString command = QString("set-fc ") + QString::number(fc_hz);
    s_send(*comm_socket, command.toStdString());
    std::string response = s_recv(*comm_socket);

    updateParameters();
}

void Receiver::setSampleRate(double fs_hz)
{
    QString command = QString("set-fs ") + QString::number(fs_hz);
    s_send(*comm_socket, command.toStdString());
    std::string response = s_recv(*comm_socket);

    updateParameters();
}
