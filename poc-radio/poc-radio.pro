# ---------------------------------------------------------------
# poc-radio.pro — файл проекта qmake ("рецепт" сборки).
# Aurora SDK собирает приложения именно через qmake, поэтому
# формат здесь максимально "авроро-подобный".
# На обычном Linux/Windows можно собрать командой:
#   qmake && make
# (Qt5 и пакет qtbase5-dev должны быть установлены)
# ---------------------------------------------------------------

QT += core network

# Qt Quick/QML подключим на следующем этапе, когда займёмся интерфейсом.
# QT += quick

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

# Тестовый "эхо-сервер" для отладки сети без настоящего сервера
TESTS_DIR = tests/echo-server

OTHER_FILES += \
    qml/Page1.qml \
    README.md
