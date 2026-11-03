#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "signalingclient.h"
#include "radiostate.h"

/**
 * Консольная версия рации (Этапы 1–2 роадмапа): команды управляют
 * RadioState, а ответы сервера печатаются в лог.
 * Интерфейс (QML) — это следующий Этап 3, он будет подключаться сюда.
 *
 * Запуск для проверки без сервера:  ./poc-radio --echo-server 5000
 * Запуск клиента:                   ./poc-radio --server 127.0.0.1 --port 5000
 */

int main(int argc, char *argv[])
{
    // QCoreApplication — приложение "без окна": идеально для проверки
    // сетевой логики прямо в терминале.
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("poc-radio");
    QCoreApplication::setApplicationVersion("0.1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("PoC radio test console client");
    parser.addHelpOption();
    parser.addOption({{"s", "server"}, "Host of signaling server", "host", ""});
    parser.addOption({{"p", "port"}, "Port of signaling server", "port", "5000"});
    parser.addOption({{"e", "echo-server"}, "Run local echo test server on given port", "port"});
    parser.process(app);

    SignalingClient client;
    RadioState radio(&client);

    // Логируем всё, что происходит
    QObject::connect(&radio, &RadioState::logEntry, [](const QString &t) {
        qInfo().noquote() << "[log]  " << t;
    });
    QObject::connect(&client, &SignalingClient::connected, []() {
        qInfo() << "[net]  соединение установлено";
    });
    QObject::connect(&client, &SignalingClient::disconnected, []() {
        qInfo() << "[net]  соединение разорвано";
    });
    QObject::connect(&client, &SignalingClient::errorOccurred, [](const QString &e) {
        qWarning() << "[net]  ошибка:" << e;
    });
    QObject::connect(&client, &SignalingClient::messageReceived, [](const QString &t) {
        qDebug() << "[recv]" << t;
    });
    // Серверные сообщения кормим "мозгу" рации
    QObject::connect(&client, &SignalingClient::parsedMessage,
                     &radio, &RadioState::onServerMessage);

    const QString echoPort = parser.value("echo-server");
    if (!echoPort.isEmpty()) {
        // Мини-сервер прямо здесь: принимает соединения и печатает,
        // что ему написали. Нужен только для проверки сокета.
        auto *server = new QTcpServer(&app);
        QObject::connect(server, &QTcpServer::newConnection, server, [server]() {
            QTcpSocket *s = server->nextPendingConnection();
            QObject::connect(s, &QTcpSocket::readyRead, s, [s]() {
                qDebug() << "[echo-server] получил:" << s->readAll();
            });
        });
        if (!server->listen(QHostAddress::LocalHost, quint16(echoPort.toUInt()))) {
            qCritical() << "не удалось запустить echo-сервер:" << server->errorString();
            return 1;
        }
        qInfo() << "[echo-server] слушаю порт" << echoPort;
        return app.exec();
    }

    const QString host = parser.value("server");
    if (host.isEmpty()) {
        qWarning() << "Укажите --server <host> или запустите --echo-server <port>";
        return 1;
    }
    client.connectToServer(host, quint16(parser.value("port").toUInt()));

    // Простейшая последовательность для ручной проверки:
    // через секунду заходим в канал, ещё через две — «нажимаем PTT».
    QTimer::singleShot(1000, [&]() { radio.joinChannel("test-channel", "Ivan"); });
    QTimer::singleShot(3000, [&]() { radio.requestTalk(); });
    QTimer::singleShot(6000, [&]() { radio.releaseTalk(); });

    return app.exec();
}
