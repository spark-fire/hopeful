!isEmpty(COMMON_PRI_INCLUDED):error("common.pri already included")
COMMON_PRI_INCLUDED = 1

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

INCLUDEPATH += \
            += $$PWD/MetaType \

HEADERS += \
    $$PWD/MetaType/global.h \
    $$PWD/log/tracelog.h

SOURCES += \
    $$PWD/log/tracelog.cpp

unix{
    contains(QT_ARCH, i386){

             }
    contains(QT_ARCH, x86_64){
        message(STATUS"********Linux 64********$$")
        INCLUDEPATH += $$PWD/depends/log4cplus/linux_64/inc
        LIBS += -L$$PWD/depends/log4cplus/linux_64/lib/ -llog4cplus

    }


}else{
    contains(QT_ARCH, i386){
        #32bit  python
        message(STATUS"********Win32********$$")
    }
    contains(QT_ARCH, x86_64){
        #64bit
        message(STATUS"********Win64********$$")
    }
}
