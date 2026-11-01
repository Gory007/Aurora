// main.qml — экран рации (Этап 3 роадмапа: «Кнопка и экран»).
//
// QML читается как обычный текст-разметка: каждый блок — это элемент
// интерфейса, а внутри него свойства (color, text, ...) и обработчики.
// Слово `radio` здесь берётся из C++ (setContextProperty в main.cpp):
// QML видит свойства модели (channelName, speakerName...) и может
// вызывать её действия (joinChannel, requestTalk...). Это и есть
// связка MODEL (C++) ↔ VIEW (QML).
//
// ВАЖНО для переноса в Aurora SDK: там этот файл станет Page1.qml
// внутри ApplicationWindow из модуля sailfish.application, а вместо
// кнопок Qt Quick Controls будут компоненты Sailfish Silica. ЛОГИКА
// при этом не меняется — те же вызовы radio.requestTalk() и т.д.

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

// Наш C++-класс, зарегистрированный в main.cpp (qmlRegisterUncreatableType).
// Нужен, чтобы писать в QML RadioState.Talking — наши режимы работы рации.
import org.pocradio 1.0

Window {
    id: root
    width: 480
    height: 800
    visible: true                    // окно видно сразу после запуска
    title: "PoC-рация"
    color: "#1d1f22"                 // тёмный фон, как тема Авроры

    // Текстовое поле ввода канала появится позже; пока канал и имя
    // задаются здесь (в реальном приложении — из настроек/QSettings):
    property string channelName: "test-channel"
    property string myName: "Ivan"

    // Когда окно открылось — заходим в канал (как делал таймер в консоли).
    // Даём полсекунды на установку TCP-соединения: joinChannel раньше умеет
    // только послать команду, когда соединение уже есть.
    Timer {
        id: joinTimer
        interval: 500
        repeat: false
        onTriggered: radio.joinChannel(root.channelName, root.myName)
    }
    Component.onCompleted: joinTimer.start()

    Column {
        anchors.centerIn: parent     // разместить колонку по центру окна
        spacing: 30
        width: parent.width

        // --- Название канала ---
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Канал: " + radio.channelName
            color: "#9aa0a6"
            font.pixelSize: 22
        }

        // --- Главный статус: свободен / говорю / слушает другой ---
        // binding (привязка): цвет и текст сами пересчитаются, когда в C++
        // изменится mode и вылетит сигнал modeChanged.
        Label {
            id: statusLabel
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 34
            font.bold: true
            color: {
                if (radio.mode === RadioState.Talking)   return "#ff5340"; // говорим МЫ
                if (radio.speakerName !== "")            return "#fbbc04"; // говорит ДРУГОЙ
                if (radio.mode === RadioState.Listening) return "#34a853"; // эфир свободен
                return "#9aa0a6";                                          // offline
            }
            text: {
                if (radio.mode === RadioState.Talking)   return "ГОВОРЮ...";
                if (radio.speakerName !== "")            return "Говорит: " + radio.speakerName;
                if (radio.mode === RadioState.Listening) return "Слушаю эфир";
                return "Не в канале";
            }
        }

        // --- Круглая кнопка PTT (press-and-hold) ---
        // В настоящей рации: нажал — говоришь, отпустил — слушаешь.
        RoundButton {
            id: pttButton
            anchors.horizontalCenter: parent.horizontalCenter
            width: 220
            height: 220
            radius: 110              // половина размера = идеальный круг
            text: "PTT"
            font.pixelSize: 44
            font.bold: true
            // Цвет самой кнопки (фон) и текста на ней задаём через contentItem
            // и background — так надёжнее во всех версиях Qt Quick Controls 2.
            contentItem: Text {
                text: pttButton.text
                font: pttButton.font
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                radius: pttButton.radius
                // down = кнопка удерживается пальцем/мышью → краснеем
                color: pttButton.down ? "#ff5340" : "#3c4043"
            }

            // onPressed/onReleased — аналог «нажал/отпустил» пальцем.
            // На десктопной проверке это левая кнопка мыши.
            onPressed: radio.requestTalk()
            onReleased: radio.releaseTalk()
            // Если палец ушёл за край кнопки — тоже освобождаем эфир!
            onCanceled: radio.releaseTalk()
        }

        // --- Список участников канала ---
        // radio.members приходит с сервера (команда USERS). model — это
        // и есть та самая «модель» из MODEL/VIEW: список данных, который
        // ListView превращает в строчки на экране.
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "В канале: " + radio.members.join(", ")
            color: "#9aa0a6"
            font.pixelSize: 18
        }
    }

    // --- Журнал событий внизу экрана (logEntry из C++) ---
    // Сигнал logEntry ловим Connections'ом и дописываем строки в ListView.
    ListModel { id: logModel }

    Connections {
        target: radio
        function onLogEntry(text) {
            logModel.append({ line: text })
            listView.positionViewAtEnd()   // прокручиваем к последней строке
        }
    }

    ListView {
        id: listView
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 140
        clip: true                         // не вылезаем за свою область
        model: logModel
        delegate: Label {
            text: line
            color: "#5f6368"
            font.pixelSize: 14
            width: ListView.view.width
        }
    }
}
