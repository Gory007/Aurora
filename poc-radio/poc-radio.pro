# ---------------------------------------------------------------
# poc-radio.pro — файл проекта qmake ("рецепт" сборки).
# Aurora SDK собирает приложения именно через qmake, поэтому
# формат здесь максимально "авроро-подобный".
# На обычном Linux/Windows можно собрать командой:
#   qmake && make
# (Qt5 и пакет qtbase5-dev должны быть установлены)
# ---------------------------------------------------------------

QT += core network
QT += quick qml

# Qt Quick/QML подключён на Этапе 3 (экран с кнопкой PTT, папка qml/).

CONFIG += c++17

TARGET = poc-radio
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/radiostate.cpp \
    src/signalingclient.cpp

HEADERS += \
    src/radiostate.h \
    src/signalingclient.h

# QML-интерфейс (Этап 3). Пока запускаем файлом с диска (видно в main.cpp);
# при переносе в Aurora SDK их можно будет упаковать в ресурс .qrc.
DISTFILES += \
    qml/main.qml

# Тестовый mock-сервер для отладки сети без настоящего сервера
TESTS_DIR = tests

OTHER_FILES += \
    tests/mock_server.py \
    README.md

# poc-radio.desktop и иконки подключим на этапе сборки .apm (Этап 8),
# когда проект будет переноситься в Aurora SDK.
