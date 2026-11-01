#ifndef RADIOSTATE_H
#define RADIOSTATE_H

#include <QObject>
#include <QString>

class SignalingClient;

/**
 * RadioState — «мозг» рации.
 *
 * Это класс-модель: он хранит текущее состояние приложения
 * (кто мы в канале, говорим ли сейчас, слышим ли кого-то)
 * и решает, КАКИЕ команды нужно отправить серверу в ответ
 * на действия пользователя. Про интерфейс (кнопки) он ничего
 * не знает — благодаря этому один и тот же код потом
 * послужит и QML-экрану, и тестам.
 *
 * Термины:
 *  - "канал" (channel/room) — группа людей, которые слышат друг друга;
 *  - "спикер" (speaker) — тот, кто сейчас говорит (в рации одновременно
 *    говорить может только один человек — это называется "сеанс").
 */
class RadioState : public QObject
{
    Q_OBJECT // магическая строка Qt: включает сигналы и слоты в этом классе

public:
    // Простое перечисление наших состояний в канале.
    enum Mode {
        Offline,    // не подключены ни к какому каналу
        Listening,  // слушаем канал (никто не говорит или говорит другой)
        Talking     // говорим сами (нажата и удерживается кнопка PTT)
    };
    Q_ENUM(Mode) // разрешаем Qt понимать этот enum из QML/сигналов

    explicit RadioState(SignalingClient *client, QObject *parent = nullptr);

    Mode mode() const { return m_mode; }
    QString channel() const { return m_channel; }
    QString speakerName() const { return m_speakerName; }

public slots:
    // --- Действия пользователя (их будет вызывать интерфейс) ---

    // Войти в канал под своим именем.
    void joinChannel(const QString &channel, const QString &myName);
    // Выйти из канала.
    void leaveChannel();
    // Нажали кнопку PTT ("говорю!") — просим у сервера право говорить.
    void requestTalk();
    // Отпустили кнопку PTT ("закончил") — освобождаем эфир.
    void releaseTalk();

    // --- События от сервера (их будет вызывать SignalingClient) ---
    void onServerMessage(const QString &type, const QString &data);

signals:
    // Сигналы — это "крики": когда что-то меняется, мы оповещаем всех,
    // кто подписался. Интерфейс подключится сюда и перерисует экран.
    void modeChanged(Mode mode);
    void speakerChanged(const QString &name);
    void logEntry(const QString &text);

private:
    void setMode(Mode newMode);

    SignalingClient *m_client;
    Mode m_mode = Offline;
    QString m_channel;
    QString m_myName;
    QString m_speakerName;   // кто сейчас говорит (пусто — если никто)
    bool m_talkingGranted = false; // сервер подтвердил, что эфир наш?
};

#endif // RADIOSTATE_H
