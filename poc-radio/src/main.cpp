#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QDir>

// ---- QML-интерфейс (Этап 3 роадмапа) ----
// QGuiApplication — "облегчённая" версия приложения: без окон десктопа,
// именно так устроены приложения в Авроре.
#include <QGuiApplication>
// QQmlApplicationEngine — запускает qml/main.qml и показывает окно.
#include <QQmlApplicationEngine>
#include <QQmlContext>   // чтобы "передать" C++-объект внутрь QML
#include <QtQml>         // qmlRegisterUncreatableType — регистрация наших типов в QML
#include <QUrl>          // адрес файла для загрузки QML

#include "signalingclient.h"
#include "radiostate.h"

/**
 * Пока интерфейс (QML) не подключён, main.cpp работает как
 * консольная версия рации: команды с клавиатуры управляют
 * RadioState, а ответы сервера печатаются в лог.
 *
 * Запуск для проверки без сервера:  ./poc-radio --echo-server 5000
 * Запуск клиента:                   ./poc-radio --server 127.0.0.1 --port 5000
 * То же самое, но с окошком (QML):  добавь флаг  --gui
 */

int main(int argc, char *argv[])
{
    // QCoreApplication — приложение "без окна" (для тестов и фоновой логики),
    // QGuiApplication — то же самое + умение показывать QML-интерфейс.
    // В Авроре приложения всегда строятся на QGuiApplication, поэтому
    // тренируемся сразу на нём: консольный режим просто не создаёт окно.
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("poc-radio");
    QCoreApplication::setApplicationVersion("0.1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("PoC radio test console client");
    parser.addHelpOption();
    parser.addOption({{"s", "server"}, "Host of signaling server", "host", ""});
    parser.addOption({{"p", "port"}, "Port of signaling server", "port", "5000"});
    parser.addOption({{"e", "echo-server"}, "Run local echo test server on given port", "port"});
    // Флаг --gui: показать QML-окошко с кнопкой PTT (проверка Этапа 3 на десктопе).
    // В Aurora SDK интерфейс из qml/ запускается всегда, этот флаг нужен только
    // для проверки на обычном компьютере без телефона.
    QCommandLineOption guiOption(QStringList{"gui"}, "Show QML interface window");
    parser.addOption(guiOption);
    parser.process(app);

    const bool useGui = parser.isSet(guiOption);

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

    // ---- QML-интерфейс (Этап 3) ----
    // "Передать" наш C++-объект radio внутрь QML под именем `radio`.
    // После этого в main.qml можно писать radio.requestTalk() или text: radio.channelName.
    QQmlApplicationEngine engine;
    if (useGui) {
        engine.rootContext()->setContextProperty("radio", &radio);
        // Чтобы писать в QML `RadioState.Talking` (наши режимы), регистрируем
        // класс модели как тип QML. В Aurora SDK это делается так же.
        qmlRegisterUncreatableType<RadioState>(
            "org.pocradio", 1, 0, "RadioState",
            QStringLiteral("RadioState создаётся только на стороне C++"));
        // Ищем main.qml рядом с программой — при сборке из исходников это папка qml/.
        const QString qmlFile = QDir::current().filePath("qml/main.qml");
        engine.load(QUrl::fromLocalFile(qmlFile));
        if (engine.rootObjects().isEmpty()) {
            qCritical() << "Не удалось загрузить" << qmlFile
                        << "(запускай из папки poc-radio/)";
            return 1;
        }
    } else {
        // Консольный режим без интерфейса: Простейшая последовательность для ручной проверки:
        // через секунду заходим в канал, ещё через две — «нажимаем PTT».
        QTimer::singleShot(1000, [&]() { radio.joinChannel("test-channel", "Ivan"); });
        QTimer::singleShot(3000, [&]() { radio.requestTalk(); });
        QTimer::singleShot(6000, [&]() { radio.releaseTalk(); });
    }

    return app.exec();
}
