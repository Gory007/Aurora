#include "radiostate.h"
#include "signalingclient.h"

#include <QDebug>

RadioState::RadioState(SignalingClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
}

// Вспомогательная функция: меняем режим и, только если он реально
// изменился, рассылаем сигнал (чтобы не перерисовывать экран лишний раз).
void RadioState::setMode(Mode newMode)
{
    if (m_mode == newMode)
        return;
    m_mode = newMode;
    emit modeChanged(m_mode);
}

void RadioState::joinChannel(const QString &channel, const QString &myName)
{
    if (channel.isEmpty() || myName.isEmpty()) {
        emit logEntry(QStringLiteral("Укажите канал и имя перед входом"));
        return;
    }
    m_channel = channel;
    m_myName = myName;

    // Формат команды серверу: JOIN <канал> <имя>
    m_client->sendMessage(QStringLiteral("JOIN %1 %2").arg(channel, myName));
    emit logEntry(QStringLiteral("Вхожу в канал «%1» как «%2»").arg(channel, myName));
}

void RadioState::leaveChannel()
{
    if (m_mode == Offline)
        return;
    m_client->sendMessage(QStringLiteral("LEAVE"));
    m_channel.clear();
    m_speakerName.clear();
    m_members.clear();
    m_talkingGranted = false;
    emit speakerChanged(m_speakerName);
    emit membersChanged(m_members);
    setMode(Offline);
    emit logEntry(QStringLiteral("Вышел из канала"));
}

void RadioState::requestTalk()
{
    if (m_mode == Offline) {
        emit logEntry(QStringLiteral("Сначала войдите в канал"));
        return;
    }
    // Говорить можно только если мы слушаем и никто другой не говорит.
    if (m_mode != Listening || !m_speakerName.isEmpty()) {
        emit logEntry(QStringLiteral("Эфир занят: говорит %1")
                      .arg(m_speakerName.isEmpty() ? m_myName : m_speakerName));
        return;
    }
    m_client->sendMessage(QStringLiteral("PTT_START"));
    setMode(Talking);
    emit logEntry(QStringLiteral("Прошу эфир... (PTT нажат)"));
}

void RadioState::releaseTalk()
{
    if (m_mode != Talking)
        return;
    m_client->sendMessage(QStringLiteral("PTT_STOP"));
    m_talkingGranted = false;
    setMode(Listening);
    emit logEntry(QStringLiteral("Освободил эфир (PTT отпущен)"));
}

// Обработка ответов сервера. Сервер отвечает простыми текстовыми
// командами, например: "ACK PTT_GRANTED Ivan".
void RadioState::onServerMessage(const QString &type, const QString &data)
{
    if (type == QLatin1String("ACK")) {
        if (data == QLatin1String("JOINED")) {
            setMode(Listening);
            emit logEntry(QStringLiteral("В канале. Слушаю."));
        } else if (data == QLatin1String("PTT_GRANTED")) {
            m_talkingGranted = true;
            emit logEntry(QStringLiteral("Эфир мой — говорите!"));
        } else if (data == QLatin1String("PTT_RELEASED")) {
            emit logEntry(QStringLiteral("Эфир освобождён."));
        }
    } else if (type == QLatin1String("SPEAKER")) {
        // data — имя того, кто начал/закончил говорить (пусто — никто)
        m_speakerName = data;
        emit speakerChanged(m_speakerName);
        if (!m_speakerName.isEmpty() && m_speakerName != m_myName) {
            emit logEntry(QStringLiteral("Говорит: %1").arg(m_speakerName));
        }
    } else if (type == QLatin1String("ERROR")) {
        emit logEntry(QStringLiteral("Ошибка сервера: %1").arg(data));
        // Если нам отказали в эфире — возвращаемся в режим слушания
        if (m_mode == Talking)
            setMode(Listening);
    } else if (type == QLatin1String("USERS")) {
        // Сервер прислал список участников через запятую: "Ivan, Petr"
        // Разбиваем строку на список (QStringList) — QML покажет его как модель списка.
        QStringList list = data.split(QLatin1Char(','), Qt::SkipEmptyParts);
        for (QString &s : list)
            s = s.trimmed();
        m_members = list;
        emit membersChanged(m_members);
        emit logEntry(QStringLiteral("В канале: %1").arg(data));
    }
}
