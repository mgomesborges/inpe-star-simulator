#include "remotecontrol.h"
#include "ui_remotecontrol.h"

#include <iostream>
#include <QFile>
#include <QDataStream>

RemoteControl::RemoteControl(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteControl)
{
    ui->setupUi(this);
    socket = NULL;
    port   = 6000;
    numberOfConnections = 0;
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(setPort()));
}

RemoteControl::~RemoteControl()
{
    delete ui;
}

void RemoteControl::listen()
{
    server->listen(QHostAddress::Any, port);
}

void RemoteControl::newConnection()
{
    if (numberOfConnections == 0) {
        socket = server->nextPendingConnection();
        numberOfConnections = 1;
        connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    }
}

void RemoteControl::readyRead()
{
    while (socket->canReadLine()) {
        QByteArray byteArray = socket->readAll();
        QList<QByteArray> tokens;

        int index1 = byteArray.indexOf("(");
        int index2 = byteArray.indexOf(")");

        if (index1 < 5 || index2 < 6) {
            sendAnswer(INVALID_COMMAND);
            continue;
        }

        tokens.append(byteArray.mid(0, index1));
        tokens.append(byteArray.mid(index1+1, index2-index1-1));

        if(tokens[0] == "SMS500Connect") {
            if (tokens[1].size() == 0) {
                emit SMS500Connect();
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "SMS500Disconnect") {
            if (tokens[1].size() == 0) {
                emit SMS500Disconnect();
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "SMS500StartScan") {
            if (tokens[1].size() == 0) {
                emit SMS500StartScan();
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "SMS500StopScan") {
            if (tokens[1].size() == 0) {
                emit SMS500StopScan();
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setSMS500NumberOfScans") {
            bool ok;
            int value = tokens[1].toInt(&ok);

            if (ok == true && value >= -1 && value != 0) {
                emit setSMS500NumberOfScans(tokens[1].data());
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setSMS500AutoRange") {
            if (tokens[1] == "true") {
                emit setSMS500AutoRange(true);
                sendAnswer(SUCCESS);
            } else if (tokens[1] == "false") {
                emit setSMS500AutoRange(false);
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setSMS500IntegrationTime") {
            bool ok;
            int value = tokens[1].toInt(&ok);

            if (ok == true && value >= 0 && value <= 22) {
                emit setSMS500IntegrationTime(value);
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setSMS500SamplesToAverage") {
            bool ok;
            int value = tokens[1].toInt(&ok);

            if (ok == true && value >= 1 && value <= 99) {
                emit setSMS500SamplesToAverage(value);
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setSMS500BoxcarSmothing") {
            bool ok;
            int value = tokens[1].toInt(&ok);

            if (ok == true && value >= 1 && value <= 50) {
                emit setSMS500BoxcarSmothing(value);
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setSMS500NoiseReduction") {
            if (tokens[1] == "true") {
                emit setSMS500NoiseReduction(true);
                sendAnswer(SUCCESS);
            } else if (tokens[1] == "false") {
                emit setSMS500NoiseReduction(false);
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setSMS500NoiseReductionFactor") {
            bool ok;
            double value = tokens[1].toDouble(&ok);

            if (ok == true && value >= 1 && value <= 50) {
                emit setSMS500NoiseReductionFactor(tokens[1].data());
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setSMS500CorrectForDynamicDark") {
            if (tokens[1] == "true") {
                emit setSMS500CorrectForDynamicDark(true);
                sendAnswer(SUCCESS);
            } else if (tokens[1] == "false") {
                emit setSMS500CorrectForDynamicDark(false);
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else  if(tokens[0] == "LEDDriverConnect") {
            if (tokens[1].size() == 0) {
                emit LEDDriverConnect();
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "LEDDriverDisconnect") {
            if (tokens[1].size() == 0) {
                emit LEDDriverDisconnect();
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else  if(tokens[0] == "startLEDDriverModeling") {
            if (tokens[1].size() == 0) {
                emit startLEDDriverModeling();
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "stopLEDDriverModeling") {
            if (tokens[1].size() == 0) {
                emit stopLEDDriverModeling();
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setLEDDriverV2Ref") {
            if (tokens[1] == "true") {
                emit setLEDDriverV2Ref(true);
                sendAnswer(SUCCESS);
            } else if (tokens[1] == "false") {
                emit setLEDDriverV2Ref(false);
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setLEDDriverValues") {
            QList<QByteArray> array = tokens[1].split(',');
            QVector<double> vector;
            bool ok;

            for (int i = 0; i < array.length(); i++) {
                vector.append((double)array[i].toInt(&ok));

                if (ok == false || vector[i] < 0 || vector[i] > 4095) {
                    sendAnswer(ERROR_WRONG_PARAMETERS);
                    return;
                }
            }

            emit loadLEDDriverValues(vector);
            sendAnswer(SUCCESS);
        }

        else if(tokens[0] == "setStarMagnitude") {
            bool ok;
            double value = tokens[1].toDouble(&ok);

            if (ok == true && value >= -9 && value <= 9) {
                emit setStarMagnitude(tokens[1].data());
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setStarTemperature") {
            bool ok;
            int value = tokens[1].toInt(&ok);

            if (ok == true && value >= 100 && value <= 999999) {
                emit setStarTemperature(tokens[1].data());
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setStarSimulatorAlgorithm") {
            if (tokens[1] == "lm") {
                emit setStarSimulatorAlgorithmLM(true);
                sendAnswer(SUCCESS);
            } else if (tokens[1] == "gd") {
                emit setStarSimulatorAlgorithmGD(true);
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "setStarSimulatorX0") {
            if (tokens[1] == "random") {
                emit setStarSimulatorX0random(true);
                sendAnswer(SUCCESS);
            } else if (tokens[1] == "userDefined") {
                emit setStarSimulatorX0userDefined(true);
                sendAnswer(SUCCESS);
            } else if (tokens[1] == "ledDriver") {
                emit setStarSimulatorX0ledDriver(true);
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "starSimulatorLoadInitialSolution") {
            QList<QByteArray> array = tokens[1].split(',');
            QVector<double> vector;
            bool ok;

            for (int i = 0; i < array.length(); i++) {
                vector.append((double)array[i].toInt(&ok));

                if (ok == false || vector[i] < 0 || vector[i] > 4095) {
                    sendAnswer(ERROR_WRONG_PARAMETERS);
                    return;
                }
            }

            emit starSimulatorLoadInitialSolution(vector);
            sendAnswer(SUCCESS);
        }

        else if(tokens[0] == "startStarSimulator") {
            if (tokens[1].size() == 0) {
                emit startStarSimulator();
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "stopStarSimulator") {
            if (tokens[1].size() == 0) {
                emit stopStarSimulator();
                sendAnswer(SUCCESS);
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "starSimulatorStatus") {
            if (tokens[1].size() == 0) {
                emit starSimulatorStatus();
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if(tokens[0] == "starSimulatorIrradiances") {
            if (tokens[1].size() == 0) {
                emit starSimulatorIrradiances();
            } else {
                sendAnswer(ERROR_WRONG_PARAMETERS);
            }
        }

        else if (tokens[0] == "closeConnection") {
            sendAnswer(SUCCESS);
            socket->disconnectFromHost();
        }

        else {
            sendAnswer(INVALID_COMMAND);
        }
    }
}

void RemoteControl::disconnected()
{
    disconnect(socket, SIGNAL(disconnected()));
    disconnect(socket, SIGNAL(readyRead()));
    socket->deleteLater();
    numberOfConnections = 0;
}

void RemoteControl::setPort()
{
    server->close();
    this->disconnected();
    port = ui->port->text().toInt();
    listen();
    this->close();
}

bool RemoteControl::isConnected()
{
    if (numberOfConnections == 0) {
        return false;
    }
    return true;
}

void RemoteControl::sendAnswer(int errorCode)
{
    if (numberOfConnections >= 1) {
        socket->write(tr("%1").arg(errorCode).toStdString().data());
    }
}

void RemoteControl::sendAnswer(QString data)
{
    if (numberOfConnections >= 1) {
        socket->write(data.toUtf8().data());
    }
}
