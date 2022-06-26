QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
# pkg-config is required for building with system-provided rtaudio

# build needs environment set
# export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
# or equiv in Projects : Build : Environment
# in ../rtaudip
# ./autogen.sh
# make, make install

CONFIG += link_pkgconfig
PKGCONFIG += rtaudio

QMAKE_LFLAGS += '-Wl,-rpath,\'\$//usr/local/lib\''

SOURCES += \
        api.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    api.h
