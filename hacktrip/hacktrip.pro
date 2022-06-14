QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += RT_AUDIO
CONFIG += c++11

# pkg-config is required for building with system-provided rtaudio
CONFIG += link_pkgconfig
PKGCONFIG += rtaudio
win32 {
  # even though we get linker flags from pkg-config, define -lrtaudio again to enforce linking order
  CONFIG += no_lflags_merge
  LIBS += -lrtaudio -lole32 -lwinmm -lksuser -lmfplat -lmfuuid -lwmcodecdspuuid # -ldsound # -ldsound only needed if rtaudio is built with directsound support
}

DESTDIR = .

SOURCES += \
    duplex.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    duplex.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
