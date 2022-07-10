QT       += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# build needs environment set
# export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
# or equiv in Projects : Build : Environment
# in ../rtaudip
# ./autogen.sh
# make, make install
# and needs
# QMAKE_LFLAGS += '-Wl,-rpath,\'\$//usr/local/lib\''

CONFIG += link_pkgconfig
PKGCONFIG += rtaudio
QMAKE_LFLAGS += '-Wl,-rpath,\'\$//usr/local/lib\''

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    hacktrip.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    hacktrip.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
