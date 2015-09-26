# -------------------------------------------------------------------------------
# Project configuration.
# Created: 2015-09-22
# -------------------------------------------------------------------------------

QT += core gui

TARGET = render
DESTDIR = bin
TEMPLATE = app
CONFIG += warn_on debug qwt

SOURCES += \
    src/main.cpp \
    src/Chromosome.cpp

HEADERS += \
        src/Chromosome.hpp

# QMAKE_CFLAGS+=-pg
# QMAKE_CXXFLAGS+=-pg
# QMAKE_LFLAGS+=-pg

OBJECTS_DIR = build
MOC_DIR     = build
RCC_DIR     = build
UI_DIR      = build
