QT       += core gui network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    changeprofiledialog.cpp \
    client.cpp \
    fileinfo.cpp \
    filesselection.cpp \
    main.cpp \
    mainwindow.cpp \
    message.cpp \
    newfiledialog.cpp \
    newfilefromuridialog.cpp \
    showuridialog.cpp \
    symbol.cpp \
    texteditor/textedit.cpp \
    user.cpp \
    usercursor.cpp

HEADERS += \
    changeprofiledialog.h \
    client.h \
    fileinfo.h \
    filesselection.h \
    mainwindow.h \
    message.h \
    newfiledialog.h \
    newfilefromuridialog.h \
    showuridialog.h \
    symbol.h \
    texteditor/textedit.h \
    user.h \
    usercursor.h

FORMS += \
    changeprofiledialog.ui \
    filesselection.ui \
    mainwindow.ui \
    newfiledialog.ui \
    newfilefromuridialog.ui \
    showuridialog.ui

TRANSLATIONS += \
    PDSClient_it_IT.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

SUBDIRS += \
    texteditor/textedit.pro

RESOURCES += \
    texteditor/textedit.qrc

DISTFILES += \
    texteditor/.idea/QText Edit.iml \
    texteditor/.idea/misc.xml \
    texteditor/.idea/modules.xml \
    texteditor/.idea/workspace.xml \
    texteditor/debug.log \
    texteditor/example.html \
    texteditor/example.md \
    texteditor/images/logo32.png \
    texteditor/images/mac/checkbox-checked.png \
    texteditor/images/mac/checkbox.png \
    texteditor/images/mac/editcopy.png \
    texteditor/images/mac/editcut.png \
    texteditor/images/mac/editpaste.png \
    texteditor/images/mac/editredo.png \
    texteditor/images/mac/editundo.png \
    texteditor/images/mac/exportpdf.png \
    texteditor/images/mac/filenew.png \
    texteditor/images/mac/fileopen.png \
    texteditor/images/mac/fileprint.png \
    texteditor/images/mac/filesave.png \
    texteditor/images/mac/format-indent-less.png \
    texteditor/images/mac/format-indent-more.png \
    texteditor/images/mac/textbold.png \
    texteditor/images/mac/textcenter.png \
    texteditor/images/mac/textitalic.png \
    texteditor/images/mac/textjustify.png \
    texteditor/images/mac/textleft.png \
    texteditor/images/mac/textright.png \
    texteditor/images/mac/textunder.png \
    texteditor/images/mac/zoomin.png \
    texteditor/images/mac/zoomout.png \
    texteditor/images/win/checkbox-checked.png \
    texteditor/images/win/checkbox.png \
    texteditor/images/win/editcopy.png \
    texteditor/images/win/editcut.png \
    texteditor/images/win/editpaste.png \
    texteditor/images/win/editredo.png \
    texteditor/images/win/editundo.png \
    texteditor/images/win/exportpdf.png \
    texteditor/images/win/filenew.png \
    texteditor/images/win/fileopen.png \
    texteditor/images/win/fileprint.png \
    texteditor/images/win/filesave.png \
    texteditor/images/win/format-indent-less.png \
    texteditor/images/win/format-indent-more.png \
    texteditor/images/win/textbold.png \
    texteditor/images/win/textcenter.png \
    texteditor/images/win/textitalic.png \
    texteditor/images/win/textjustify.png \
    texteditor/images/win/textleft.png \
    texteditor/images/win/textright.png \
    texteditor/images/win/textunder.png \
    texteditor/images/win/zoomin.png \
    texteditor/images/win/zoomout.png \
    texteditor/textedit.pro.user \
    texteditor/textedit.pro.user.519c975 \
    texteditor/textedit.qdoc
