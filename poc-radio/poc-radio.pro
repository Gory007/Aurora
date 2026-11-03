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

# Тестовый mock-сервер для отладки сети без настоящего сервера
TESTS_DIR = tests

OTHER_FILES += \
    tests/mock_server.py \
    README.md

# qml/app.qml и poc-radio.desktop подключим на этапе QML-интерфейса,
# когда проект будет переноситься в Aurora SDK.
