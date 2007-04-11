unix {
  TEMPLATE = lib 
  VERSION = 1.0
  DEFINES += UNIX kTWAIN_DS_DIR=\"/usr/local/lib/twain\"
}


TARGET = twaindsm
target.path = /usr/local/lib
INSTALLS += target

INCLUDEPATH = ../includes


SOURCES += \
  dsm.cpp

HEADERS += \
  dsm.h \
  dsmdefs.h \
  dsmprv.h

CONFIG  += warn_on debug 

# dev headers
headers.path = /usr/local/include/twain
headers.files = ../includes/twain/twain.h
INSTALLS += headers 

# this project doesn't need any QT libs.
QMAKE_LIBS_QT           = 
QMAKE_LIBS_QT_THREAD    = 

