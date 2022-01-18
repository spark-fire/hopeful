QT -= core gui


TEMPLATE = app
DESTDIR += ./bin


CONFIG += c++17 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(common/common.pri)

SOURCES += \
        communicationserver.cpp \
        main.cpp \
        protobuf/person.pb.cc

HEADERS += \
    communicationserver.h \
    protobuf/person.pb.h

unix{
    contains(QT_ARCH, i386){

             }
    contains(QT_ARCH, x86_64){
        INCLUDEPATH += $$PWD/depends/libev/linux_64/inc
        LIBS += -L$$PWD/depends/libev/linux_64/lib -lev

        INCLUDEPATH += $$PWD/depends/protobuf/linux_64
        LIBS += -L$$PWD/depends/protobuf/linux_64/lib/ -lprotobuf
#        LIBS += $$PWD/depends/protobuf/linux_64/lib/libprotobuf.a
    }


}else{
    contains(QT_ARCH, i386){
        #32bit  python
        LIBS += -L$$PWD/lib/python/ -lpython37
        INCLUDEPATH += $$PWD/lib/python/include
    }
    contains(QT_ARCH, x86_64){
        #64bit
        message(STATUS"********Win64********$$")
    }
}

DISTFILES += \
    protobuf/person.proto




