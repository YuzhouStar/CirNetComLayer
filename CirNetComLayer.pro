TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

target.path = /home/root
INSTALLS += target

SOURCES += \
        main.c \
    CirNetComLayer.c


LIBS +=-lpthread

HEADERS += \
    CirNetComLayer.h
