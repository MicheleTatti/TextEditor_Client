/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QScreen>
#include <client.h>
#include <iostream>
#include <chrono>
#include <thread>
#include "client.h"
#include "symbol.h"
#include "message.h"
#include "usercursor.h"
#include <QPrinter>
QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QTextEdit;
class QTextCharFormat;
class QMenu;
QT_END_NAMESPACE

class TextEdit : public QMainWindow
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent = 0, std::shared_ptr<Client> client=nullptr, QString filename="", int fileIndex = -1);

    bool load(const QString &f);

public slots:
    void fileNew();
    void onRemoteCursorChanged(int cursorIndex, int siteIdSender);
    void onFileErased(int index);

signals:
    /*-----AGGIUNTE DA NOI------*/
    void message_ready(QVector<Message> messages, int fileIndex);
    void closeWindow();
    void my_cursor_position_changed(int index);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);
    void setChecked(bool checked);
    void indent();
    void unindent();

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void about();
    void printPreview(QPrinter *);
    /*---SLOTS AGGIUNTE DA NOI----*/
    void onTextChanged(int pos, int del, int add);
    //void onMessageFromServer(Message m, int siteIdSender);
    void onMessagesFromServer(QVector<Message> messages, int siteIdSender);
    void onFileReady(QVector<std::shared_ptr<Symbol>> s);
    void onShareURIButtonPressed();
    void onURIReady(QString uri);
    void onFileClosed();
    void onSignalConnection(int siteId, QString nickname, int ins);
    void onPrintOnPDF();
    void onRefreshTextEdit(QString oldNick, QString newNick);
    void onUpdateCursors();

private:
    /*----AGGIUNTE DA NOI -----*/
    std::shared_ptr<Client> client;                     //client responsabile di comunicare col server, passato dalla filesselection
    QMap<int, std::shared_ptr<UserCursor>> cursorsMap;  //mappa siteId-cursore dei client connessi al file
    int colorId;                                        //counter dei colori utilizzati per i diversi client
    bool flag_all_highlighted = false;                  //flag per dire se ho la selezione "Evidenzia tutti" attiva
    int flag_one_highlighted = -1;                      //flag che indica quale siteId particolare deve avere il testo evidenziato, -1 significa nessuno
    QVector<std::shared_ptr<Symbol>> _symbols;          //vettore di simboli del file
    int counter;                                        //counter di simboli inseriti da questo client, inizializzato nella onFileReady al valore attuale
    int siteId;                                         //siteId del client connesso
    bool uriRequest = false;                            //flago che indica se ho fatto richiesta di una URI
    QMap<int, std::shared_ptr<User>> colorableUsers;    //mappa int-utente per segnare i possibili editor del file nella tendina degli highlight
    bool writingFlag; //Questo flag permette di non mandare la modifica del puntatore aggiornato nel caso si
                              //stia scrivendo, verrà dedotto dal fatto che si sta scrivendo in una determinata posizione.
    QVector<QTextCharFormat> charsFormat;               //vettore per il formato delle copia/taglia-incolla
    QVector<int> calcIntermediatePos(QVector<int> pos_sup, QVector<int> pos_inf);
    QVector<int> generatePos(int index);
    QVector<QVector<int>> generatePos(int index, int nPosVector);
    bool localInsert(int index, QChar value, QTextCharFormat *format, Message& m);
    QVector<Message> localInsert(int startIndex, QString added, int del, int add, QVector<QTextCharFormat> changeFormatVect);

    void remoteInsert(QVector<Message> messages);
    void remoteDelete(QVector<Message> messages, int siteIdSender);
    int findIndexFromNewPosition(QVector<int> position);
    int findIndexFromExistingPosition(QVector<int> position);
    int findIndexFromPosition(QVector<int> position);
    int alignToInt(int align);
    Qt::Alignment intToAlign(int val);
    void getURI();
    void setUriRequest(bool status);
    void highlightUserText(const QString &str);
    void onSignalOwners(QMap<int,QString> owners);
    void remoteCursorChangePosition(int cursorPos, int siteId);
    bool styleIsEqual(std::shared_ptr<Symbol> s1, std::shared_ptr<Symbol> s2);
    void deleteFromEditor(int firstIndex, int lastIndex, QTextCursor cursor);
    std::shared_ptr<Symbol> createSymbol(int index, QChar value, QTextCharFormat *format, QVector<int> position);
    /*----FINE AGGIUNTE--------*/
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    bool maybeSave();
    void setCurrentFileName(const QString &fileName);
    void modifyIndentation(int amount);



    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionIndentLess;
    QAction *actionIndentMore;
    QAction *actionToggleCheckState;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionURI;
#ifndef QT_NO_CLIPBOARD
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
#endif

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;
    /*AGGIUNTO DA NOI*/
    QComboBox *comboUser;   //tendina per l'highlight
    /*FINE AGGIUNTA DA NOI*/
    QToolBar *tb;
    QString fileName;
    int fileIndex;
    QTextEdit *textEdit;
    QStatusBar *bar;

    //FLAGS
    bool FLAG_MODIFY_SYMBOL = false;
};

#endif // TEXTEDIT_H
