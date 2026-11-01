#ifndef SIGNALINGCLIENT_H
#define SIGNALINGCLIENT_H

#include <QObject>
#include <QString>

class QTcpSocket;

/**
 * SignalingClient — «почтальон» приложения.
 *
 * Его задача: установить TCP-соединение с сервером, отправлять
 * туда текстовые команды и, наоборот, принимать ответы сервера,
 * разбирая их на "тип + данные". Про то, ЧТО означают команды,
 * он не думает — этим занимается RadioState. Такое разделение
 * называется "разделение ответственности" (single responsibility).
 *
 * Формат сообщений в сети (протокол): одна строка = одно сообщение,
 * например:  "JOIN work Иван"  или ответ  "SPEAKER Petr".
 * Символ переноса строки '\n' — разделитель сообщений.
 */
class SignalingClient : public QObject
{
    Q_OBJECT

public:
    explicit SignalingClient(QObject *parent = nullptr);
    ~SignalingClient() override;

    bool isConnected() const;

public slots:
    // Подключиться к серверу (host — адрес, port — номер порта).
    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();

    // Отправить текстовую команду серверу (добавит '\n' сама).
    void sendMessage(const QString &message);

signals:
    void connected();
    void disconnected();
    // Пришёл полный текст от сервера (без перевода строки)
    void messageReceived(const QString &text);
    // Разобранное сообщение: type="SPEAKER", data="Petr"
    void parsedMessage(const QString &type, const QString &data);
    void errorOccurred(const QString &errorText);

private slots:
    void onReadyRead();      // в сокет пришли новые байты
    void onConnectedSlot();  // соединение установлено
    void onDisconnectedSlot();

private:
    QTcpSocket *m_socket;
    QByteArray m_rxBuffer; // накопитель входящих байтов (см. onReadyRead)
};

#endif // SIGNALINGCLIENT_H
