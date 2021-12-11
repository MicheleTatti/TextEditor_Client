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

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMimeData>
#include <QLabel>
#include <QMimeDatabase>
#include <QScrollBar>
#include "showuridialog.h"
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printer)
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#include <QPrinter>
#if QT_CONFIG(printpreviewdialog)
#include <QPrintPreviewDialog>
#endif
#endif
#endif

#include "textedit.h"

#ifdef Q_OS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif

TextEdit::TextEdit(QWidget *parent, std::shared_ptr<Client> client, QString filename, int fileIndex)
    : QMainWindow(parent), client(client), fileName(filename), fileIndex(fileIndex)
{
    counter=0;
    siteId=client.get()->getSiteId();
    writingFlag = false;
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
    setWindowTitle(QCoreApplication::applicationName());
    textEdit = new QTextEdit(this);
    connect(textEdit, &QTextEdit::currentCharFormatChanged,
            this, &TextEdit::currentCharFormatChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged,
            this, &TextEdit::cursorPositionChanged);
    /*------------Aggiunta da noi------*/
    connect(this, &TextEdit::message_ready,
            client.get(), &Client::onMessageReady);
    connect(client.get(), &Client::messages_from_server,
            this, &TextEdit::onMessagesFromServer);
    connect(textEdit->document(), &QTextDocument::contentsChange,
            this, &TextEdit::onTextChanged);
    connect(client.get(), &Client::file_ready,this,&TextEdit::onFileReady);
    connect(client.get(), &Client::URI_Ready, this, &TextEdit::onURIReady);
    connect(client.get(), &Client::disconnect_URI, this, &TextEdit::onFileClosed);
    connect(client.get(), &Client::signal_connection, this, &TextEdit::onSignalConnection);
    connect(client.get(), &Client::signal_owners, this, &TextEdit::onSignalOwners);
    connect(this, &TextEdit::my_cursor_position_changed,
            client.get(), &Client::onMyCursorPositionChanged);
    connect(client.get(), &Client::remote_cursor_changed,
            this, &TextEdit::onRemoteCursorChanged);
    connect(client.get(), &Client::file_erased, this, &TextEdit::onFileErased);
    connect(client.get(), &Client::refresh_text_edit, this, &TextEdit::onRefreshTextEdit);

    connect(textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextEdit::onUpdateCursors);


    colorId=0;
    /*------------Fine aggiunta--------*/
    setCentralWidget(textEdit);

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupTextActions();

    {
        QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
        helpMenu->addAction(tr("About"), this, &TextEdit::about);
        helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    }

    QFont textFont("Arial");
    textFont.setStyleHint(QFont::SansSerif);
    textFont.setPointSize(12); //FONT SIZE CAMBIATA
    textEdit->setFont(textFont);
    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());

    //connect(textEdit->document(), &QTextDocument::modificationChanged, actionSave, &QAction::setEnabled);        //capire crush: da commentare.
    connect(textEdit->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);
    connect(textEdit->document(), &QTextDocument::undoAvailable,
            actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable,
            actionRedo, &QAction::setEnabled);

    setWindowModified(textEdit->document()->isModified());
    //actionSave->setEnabled(textEdit->document()->isModified()); //capire crush: da commentare
    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

#ifndef QT_NO_CLIPBOARD
    actionCut->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
    actionCopy->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TextEdit::clipboardDataChanged);
#endif

    textEdit->setFocus();
    setCurrentFileName(fileName);

    this->bar =  statusBar();
    QString s("Utenti Connessi: " + QString::number(this->cursorsMap.size() + 1));
    bar->showMessage(tr(qPrintable(s)));



#ifdef Q_OS_MACOS
    // Use dark text on light background on macOS, also in dark mode.
    QPalette pal = textEdit->palette();
    pal.setColor(QPalette::Base, QColor(Qt::white));
    pal.setColor(QPalette::Text, QColor(Qt::black));
    textEdit->setPalette(pal);
#endif
}


/**
 * @brief SLOT chiamata ogni volta ci sia uno scroll event, serve per la gestione dei Cursori di altri utenti nel testo
 *
 */

void TextEdit::onUpdateCursors(){
    if(cursorsMap.size() > 0){
        for( std::shared_ptr<UserCursor> uc : cursorsMap){
            uc->getPos();
            QTextCursor cursor(textEdit->textCursor());
            cursor.setPosition( uc->getPos());
            QRect rt = textEdit->cursorRect(cursor);
            uc->getCursor()->hide();
            uc->getCursor()->move(rt.x(),rt.y());
            uc->getCursor()->show();
        }
    }
}

/**
 * @brief riceve notifica di una nuova connessione/disconnessione da parte di un utente, modifica l'informazione nella tendina apposita e inserisce il suo cursore
 * @param siteId: siteId dell'utente
 * @param nickname: nickname dell'utente
 * @param ins: 1 se si è connesso, 0 se si è disconnesso
 */
void TextEdit::onSignalConnection(int siteId, QString nickname, int ins){
    if(ins == 1){
        if(colorableUsers.contains(siteId)){
            // comboUser->setItemText(colorableUsers.keys().indexOf(siteId) + 2,  QString::number(siteId) + " - " + nickname +  "- connesso" );
            comboUser->setItemText(comboUser->findData(siteId),  QString::number(siteId) + " - " + nickname +  " (connesso)" );
        }
        if(!colorableUsers.contains(siteId)){
            cursorsMap.insert(siteId, std::make_shared<UserCursor>(UserCursor(siteId, nickname, colorId, textEdit)));
            User user(siteId, nickname, colorId);
            colorableUsers.insert(siteId,  std::make_shared<User>(user));
            //
            QPixmap px(15,15);
            px.fill(colorableUsers[siteId]->getColor());
            QIcon icon(px);
            comboUser->addItem(icon, QString::number(siteId) + " - " + nickname + " (connesso)", siteId);
        }else{
            cursorsMap.insert(siteId, std::make_shared<UserCursor>(UserCursor(siteId, nickname, colorableUsers[siteId]->getColorId(), textEdit)));
        }
        colorId++;
        bar->clearMessage();
        QString s("Utenti Connessi: " + QString::number(this->cursorsMap.size() +1 ));
        bar->showMessage(tr(qPrintable(s)));
    }else if(ins == 0){
        if(cursorsMap.contains(siteId)){
            cursorsMap[siteId]->getCursor()->hide();
            cursorsMap.remove(siteId);
            bar->clearMessage();
            QString s("Utenti Connessi: " + QString::number(this->cursorsMap.size() + 1) );
            bar->showMessage(tr(qPrintable(s)));
            comboUser->setItemText(comboUser->findData(siteId),  QString::number(siteId) + " - " + nickname +  " (disconnesso)" );
        }
    }
}

/**
 * @brief riceve dal server i dati su tutti gli utenti che possono modificare il file, in modo da poter inserire tutti nella tendina apposita
 * @param owners: mappa siteId-nickname degli editor
 */
void TextEdit::onSignalOwners(QMap<int, QString> owners){
    for(int siteId: owners.keys()){
        //perché se ci sono già connessi entra prima nella onSignalConnection, quindi raddoppio gli item
        if(!colorableUsers.contains(siteId)){
            User user(siteId, owners[siteId], colorId++);
            colorableUsers.insert(siteId, std::make_shared<User>(user)); //TODO deve contenere anche un colore, nuova classe? PROVA
            QPixmap px(15,15);
            px.fill(colorableUsers[siteId]->getColor());
            QIcon icon(px);
            if( this->siteId == siteId){
                comboUser->addItem(icon, QString::number(siteId) + " - " + colorableUsers[siteId]->getNickname() + " - (Io)", siteId);
            }else{
                if(this->cursorsMap.contains(siteId)){
                    comboUser->addItem(icon, QString::number(siteId) + " - " + colorableUsers[siteId]->getNickname() + " - connesso", siteId);
                }else{
                    comboUser->addItem(icon, QString::number(siteId) + " - " + colorableUsers[siteId]->getNickname() + " - disconnesso", siteId);
                }
            }
        }
    }
}

/**
 * @brief alla chiusura dell'editor segnalo con la closeWindow che deve riaprire il filesSelection e notifico al server la chiusura del file
 * @param e
 */
void TextEdit::closeEvent(QCloseEvent *e)
{
    client.get()->closeFile(this->fileIndex);
    emit closeWindow();
    disconnect(this, &TextEdit::message_ready, client.get(), &Client::onMessageReady);
    hide();
}

/**
 * @brief se ho un file aperto che è stato appena cancellato lo chiudo e mostro una dialog
 * @param index: indice del file
 */
void TextEdit::onFileErased(int index) {
    if(this->fileIndex == index && !this->isHidden()) {
        QMessageBox::information(this,"OPS!","Il creatore ha eliminato il file.");
        emit closeWindow();
        disconnect(this, &TextEdit::message_ready, client.get(), &Client::onMessageReady);
        hide();
    }
}

/**
 * @brief se un utente ha cambiato il proprio nickname riaggiorno il suo nickname nella tendina apposita
 * @param oldNick: vecchio nickname
 * @param newNick: nuovo nickname
 */
void TextEdit::onRefreshTextEdit(QString oldNick, QString newNick) {
    for(std::shared_ptr<User> user : colorableUsers) {
        if(user->getNickname() == oldNick) {
            user->setNickname(newNick);
        }
    }
    comboUser->clear();
    comboUser->addItem("Non evidenziare", -2);
    comboUser->addItem("Evidenzia tutti", -1);
    for(int siteId: colorableUsers.keys()){
        QPixmap px(15,15);
        px.fill(colorableUsers[siteId]->getColor());
        QIcon icon(px);
        if( this->siteId == siteId){
            comboUser->addItem(icon, QString::number(siteId) + " - " + colorableUsers[siteId]->getNickname() + " - (Io)", siteId);
        }else{
            if(this->cursorsMap.contains(siteId)){
                comboUser->addItem(icon, QString::number(siteId) + " - " + colorableUsers[siteId]->getNickname() + " - (connesso)", siteId);
            }else{

                comboUser->addItem(icon, QString::number(siteId) + " - " + colorableUsers[siteId]->getNickname() + " - (disconnesso)", siteId);
            }
        }
    }
}


void TextEdit::setupFileActions()
{
    QToolBar *tb = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));


    const QIcon newIcon = QIcon::fromTheme("document-share", QIcon(rsrcPath + "/share.png"));   //capire crush: togliere commento
    QAction *a = menu->addAction( tr("&Condividi Documento"), this, &TextEdit::onShareURIButtonPressed);  //capire crush: togliere commento
    a->setPriority(QAction::LowPriority);
    menu->addSeparator();


#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png"));
    a = menu->addAction(printIcon, tr("&Print..."), this, &TextEdit::filePrint);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    tb->addAction(a);

    const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png"));
    menu->addAction(filePrintIcon, tr("Print Preview..."), this, &TextEdit::filePrintPreview);

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png"));
    a = menu->addAction(exportPdfIcon, tr("&Export PDF..."), this, &TextEdit::filePrintPdf);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);

    menu->addSeparator();
#endif

    a = menu->addAction(tr("&Quit"), this, &QWidget::close);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
}

void TextEdit::setupEditActions()
{
    QToolBar *tb = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Edit"));

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png"));
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/editredo.png"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb->addAction(actionRedo);
    menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/editcut.png"));
    actionCut = menu->addAction(cutIcon, tr("Cu&t"), textEdit, &QTextEdit::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    tb->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/editcopy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copy"), textEdit, &QTextEdit::copy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/editpaste.png"));
    actionPaste = menu->addAction(pasteIcon, tr("&Paste"), textEdit, &QTextEdit::paste);
    actionPaste->setPriority(QAction::LowPriority);
    actionPaste->setShortcut(QKeySequence::Paste);
    tb->addAction(actionPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void TextEdit::onPrintOnPDF(){
    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    textEdit->document()->print(&printer);
    statusBar()->showMessage(tr("Exported \"%1\"")
                             .arg(QDir::toNativeSeparators(fileName)));

}

void TextEdit::setupTextActions()
{
    QToolBar *tb = addToolBar(tr("Format Actions"));
    QMenu *menu = menuBar()->addMenu(tr("F&ormat"));

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(rsrcPath + "/textbold.png"));
    actionTextBold = menu->addAction(boldIcon, tr("&Bold"), this, &TextEdit::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(rsrcPath + "/textitalic.png"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Italic"), this, &TextEdit::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/textunder.png"));
    actionTextUnderline = menu->addAction(underlineIcon, tr("&Underline"), this, &TextEdit::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    tb->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/textleft.png"));
    actionAlignLeft = new QAction(leftIcon, tr("&Left"), this);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/textcenter.png"));
    actionAlignCenter = new QAction(centerIcon, tr("C&enter"), this);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/textright.png"));
    actionAlignRight = new QAction(rightIcon, tr("&Right"), this);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/textjustify.png"));
    actionAlignJustify = new QAction(fillIcon, tr("&Justify"), this);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    // Make sure the alignLeft  is always left of the alignRight
    QActionGroup *alignGroup = new QActionGroup(this);
    connect(alignGroup, &QActionGroup::triggered, this, &TextEdit::textAlign);

    if (QApplication::isLeftToRight()) {
        alignGroup->addAction(actionAlignLeft);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignRight);
    } else {
        alignGroup->addAction(actionAlignRight);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignLeft);
    }
    alignGroup->addAction(actionAlignJustify);

    tb->addActions(alignGroup->actions());
    menu->addActions(alignGroup->actions());
    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = menu->addAction(pix, tr("&Color..."), this, &TextEdit::textColor);
    tb->addAction(actionTextColor);

    menu->addSeparator();

    tb = addToolBar(tr("Format Actions"));
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    comboFont = new QFontComboBox(tb);
    comboFont->setEditable(false);
    tb->addWidget(comboFont);
    connect(comboFont, &QComboBox::textActivated, this, &TextEdit::textFamily);

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);

    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    for (int size : standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    connect(comboSize, &QComboBox::textActivated, this, &TextEdit::textSize);
    /*AGGIUNTA DA NOI */
    tb->addSeparator();
    menu->addSeparator();

    comboUser = new QComboBox(tb);
    comboUser->setObjectName("comboUser");
    comboUser->setMinimumWidth(200);
    tb->addWidget(comboUser);
    comboUser->setEditable(false);
    comboUser->clear();
    comboUser->addItem("Non evidenziare", -2);
    comboUser->addItem("Evidenzia tutti", -1);
    for(int siteId: colorableUsers.keys()){
        QPixmap px(15,15);
        px.fill(colorableUsers[siteId]->getColor());
        QIcon icon(px);
        if( this->siteId == siteId){
            comboUser->addItem(icon, QString::number(siteId) + " - " + colorableUsers[siteId]->getNickname() + " - (Io)", siteId);
        }else{
            if(this->cursorsMap.contains(siteId)){
                comboUser->addItem(icon, QString::number(siteId) + " - " + colorableUsers[siteId]->getNickname() + " - connesso", siteId);
            }else{
                comboUser->addItem(icon, QString::number(siteId) + " - " + colorableUsers[siteId]->getNickname() + " - disconnesso", siteId);
            }
        }
    }
    connect(comboUser, &QComboBox::textActivated, this, &TextEdit::highlightUserText);

    /*FINE AGGIUNTA DA NOI*/
}

/**
 * @brief funzione per evidenziare il testo degli utenti, selezionabile tramite tendina apposita
 * @param str: testo della scelta selezionata nella tendina
 */
void TextEdit::highlightUserText(const QString &str){
    disconnect(textEdit->document(), &QTextDocument::contentsChange, this, &TextEdit::onTextChanged);
    if(str == "Evidenzia tutti"){
        flag_all_highlighted = true;
        flag_one_highlighted = -1;
        int inizio = 0; //posizione iniziale del testo da  sottolineare
        int j = 0;
        int siteIdTmp = _symbols[0]->getSiteId();
        QTextCursor cursor = textEdit->textCursor();
        int dim = textEdit->toPlainText().size()-1;
        while(j < dim){
            j++;
            if(_symbols[j]->getSiteId() != siteIdTmp){

                cursor.setPosition(inizio, QTextCursor::MoveAnchor); //per selezionare un carattere
                cursor.setPosition(j, QTextCursor::KeepAnchor);
                if (colorableUsers.contains(siteIdTmp)) {
                    QTextCharFormat fmt;
                    fmt.setBackground(colorableUsers[siteIdTmp]->getColor());
                    cursor.mergeCharFormat(fmt);

                }
                //i = i+j;
                inizio = j;
                siteIdTmp = _symbols[j]->getSiteId();
                //j=0;
            }
        }
        cursor.setPosition(inizio, QTextCursor::MoveAnchor); //per selezionare un carattere
        cursor.setPosition(j+1, QTextCursor::KeepAnchor);
        if (colorableUsers.contains(siteIdTmp)) {
            QTextCharFormat fmt;
            fmt.setBackground(colorableUsers[siteIdTmp]->getColor());
            cursor.mergeCharFormat(fmt);
        }

    }else if(str == "Non evidenziare"){

        QTextCharFormat fmt;
        fmt.setBackground(Qt::white);
        QTextCursor cursor = textEdit->textCursor();
        cursor.select(QTextCursor::Document);
        cursor.mergeCharFormat(fmt);
        flag_all_highlighted = false;
        flag_one_highlighted = -1;

    }else if(str.contains("Modifica testo")){

        int pos = str.split(" - ")[1].toInt();
        int add = str.split(" - ")[2].toInt();
        QTextCursor cursor = textEdit->textCursor();
        QTextCharFormat fmt;

        cursor.setPosition(pos, QTextCursor::MoveAnchor); //per selezionare un carattere
        cursor.setPosition(pos + add, QTextCursor::KeepAnchor);

        if(flag_all_highlighted || flag_one_highlighted == siteId){
            fmt.setBackground(colorableUsers[siteId]->getColor());
            cursor.setPosition(pos,QTextCursor::MoveAnchor);
            cursor.setPosition(pos+add,QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(fmt);
        }else{
            fmt.setBackground(Qt::white);
            cursor.mergeCharFormat(fmt);
        }

    }else{

        int highlightedUserSiteId = str.split(" - ")[0].toInt();
        QTextCursor cursor = textEdit->textCursor();
        QTextCharFormat fmt;
        int i = 0;
        int j = 0;
        int dim =  textEdit->toPlainText().size()-1;
        bool highlight = false;
        flag_all_highlighted = false;
        flag_one_highlighted = highlightedUserSiteId;

        fmt.setBackground(Qt::white);
        cursor.select(QTextCursor::Document);
        cursor.mergeCharFormat(fmt);

        while(i < dim){

            if(_symbols[i]->getSiteId() == highlightedUserSiteId){
                if(!highlight){
                    highlight = true;
                    j=i;
                }
            }else{
                if(highlight){
                    cursor.setPosition(j, QTextCursor::MoveAnchor); //per selezionare un carattere
                    cursor.setPosition(i, QTextCursor::KeepAnchor);
                    if (colorableUsers.contains(highlightedUserSiteId)) {
                        QTextCharFormat fmt;
                        fmt.setBackground(colorableUsers[highlightedUserSiteId]->getColor());
                        cursor.mergeCharFormat(fmt);
                    }
                    highlight = false;
                }
            }
            i++;
        }

        if(highlight){
            cursor.setPosition(j, QTextCursor::MoveAnchor); //per selezionare un carattere
            cursor.setPosition(i+1, QTextCursor::KeepAnchor);
            if (colorableUsers.contains(highlightedUserSiteId)) {
                QTextCharFormat fmt;
                fmt.setBackground(colorableUsers[highlightedUserSiteId]->getColor());
                cursor.mergeCharFormat(fmt);
            }
            highlight = false;
        }
    }

    connect(textEdit->document(), &QTextDocument::contentsChange, this, &TextEdit::onTextChanged);
}

bool TextEdit::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        QUrl baseUrl = (f.front() == QLatin1Char(':') ? QUrl(f) : QUrl::fromLocalFile(f)).adjusted(QUrl::RemoveFilename);
        textEdit->document()->setBaseUrl(baseUrl);
        textEdit->setHtml(str);
    } else {
#if QT_CONFIG(textmarkdownreader)
        QMimeDatabase db;
        if (db.mimeTypeForFileNameAndData(f, data).name() == QLatin1String("text/markdown"))
            textEdit->setMarkdown(QString::fromUtf8(data));
        else
#endif
            textEdit->setPlainText(QString::fromUtf8(data));
    }

    setCurrentFileName(f);
    return true;
}

bool TextEdit::maybeSave()
{
    if (!textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
            QMessageBox::warning(this, QCoreApplication::applicationName(),
                                 tr("The document has been modified.\n"
                                    "Do you want to save your changes?"),
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}
/**
 * @brief mostra il nome del file in alto nella finestra
 * @param fileName: nome del file
 */
void TextEdit::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1[*] - %2").arg(shownName, QCoreApplication::applicationName()));
    setWindowModified(false);
}

void TextEdit::fileNew()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFileName(QString());
    }
}

void TextEdit::fileOpen()
{
    QFileDialog fileDialog(this, tr("Open File..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setMimeTypeFilters(QStringList()
                              #if QT_CONFIG(texthtmlparser)
                                  << "text/html"
                              #endif
                              #if QT_CONFIG(textmarkdownreader)

                                  << "text/markdown"
                              #endif
                                  << "text/plain");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    const QString fn = fileDialog.selectedFiles().first();
    if (load(fn))
        statusBar()->showMessage(tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fn)));
    else
        statusBar()->showMessage(tr("Could not open \"%1\"").arg(QDir::toNativeSeparators(fn)));
}

bool TextEdit::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();
    if (fileName.startsWith(QStringLiteral(":/")))
        return fileSaveAs();

    QTextDocumentWriter writer(fileName);
    bool success = writer.write(textEdit->document());
    if (success) {
        textEdit->document()->setModified(false);
        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    } else {
        statusBar()->showMessage(tr("Could not write to file \"%1\"")
                                 .arg(QDir::toNativeSeparators(fileName)));
    }
    return success;
}

bool TextEdit::fileSaveAs()
{
    QFileDialog fileDialog(this, tr("Save as..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList mimeTypes;
    mimeTypes << "text/plain"
             #if QT_CONFIG(textodfwriter)
              << "application/vnd.oasis.opendocument.text"
             #endif
             #if QT_CONFIG(textmarkdownwriter)
              << "text/markdown"
             #endif
              << "text/html";
    fileDialog.setMimeTypeFilters(mimeTypes);
#if QT_CONFIG(textodfwriter)
    fileDialog.setDefaultSuffix("odt");
#endif
    if (fileDialog.exec() != QDialog::Accepted)
        return false;
    const QString fn = fileDialog.selectedFiles().first();
    setCurrentFileName(fn);
    return fileSave();
}

void TextEdit::getURI(){

}

void TextEdit::filePrint()
{
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (textEdit->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted)
        textEdit->print(&printer);
    delete dlg;
#endif
}

void TextEdit::filePrintPreview()
{
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printpreviewdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &TextEdit::printPreview);
    preview.exec();
#endif
}

void TextEdit::printPreview(QPrinter *printer)
{
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    textEdit->print(printer);
#else
    Q_UNUSED(printer)
#endif
}


void TextEdit::filePrintPdf()
{
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    //! [0]
    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    textEdit->document()->print(&printer);
    statusBar()->showMessage(tr("Exported \"%1\"")
                             .arg(QDir::toNativeSeparators(fileName)));
    //! [0]
#endif
}

void TextEdit::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    QTextCursor cursor = textEdit->textCursor();
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void TextEdit::textStyle(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();
    QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;
    QTextBlockFormat::MarkerType marker = QTextBlockFormat::MarkerType::NoMarker;

    switch (styleIndex) {
    case 1:
        style = QTextListFormat::ListDisc;
        break;
    case 2:
        style = QTextListFormat::ListCircle;
        break;
    case 3:
        style = QTextListFormat::ListSquare;
        break;
    case 4:
        if (cursor.currentList())
            style = cursor.currentList()->format().style();
        else
            style = QTextListFormat::ListDisc;
        marker = QTextBlockFormat::MarkerType::Unchecked;
        break;
    case 5:
        if (cursor.currentList())
            style = cursor.currentList()->format().style();
        else
            style = QTextListFormat::ListDisc;
        marker = QTextBlockFormat::MarkerType::Checked;
        break;
    case 6:
        style = QTextListFormat::ListDecimal;
        break;
    case 7:
        style = QTextListFormat::ListLowerAlpha;
        break;
    case 8:
        style = QTextListFormat::ListUpperAlpha;
        break;
    case 9:
        style = QTextListFormat::ListLowerRoman;
        break;
    case 10:
        style = QTextListFormat::ListUpperRoman;
        break;
    default:
        break;
    }

    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    if (style == QTextListFormat::ListStyleUndefined) {
        blockFmt.setObjectIndex(-1);
        int headingLevel = styleIndex >= 11 ? styleIndex - 11 + 1 : 0; // H1 to H6, or Standard
        blockFmt.setHeadingLevel(headingLevel);
        cursor.setBlockFormat(blockFmt);

        int sizeAdjustment = headingLevel ? 4 - headingLevel : 0; // H1 to H6: +3 to -2
        QTextCharFormat fmt;
        fmt.setFontWeight(headingLevel ? QFont::Bold : QFont::Normal);
        fmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.mergeCharFormat(fmt);
        textEdit->mergeCurrentCharFormat(fmt);
    } else {
        blockFmt.setMarker(marker);
        cursor.setBlockFormat(blockFmt);
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }
        listFmt.setStyle(style);
        cursor.createList(listFmt);
    }

    cursor.endEditBlock();
}

void TextEdit::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void TextEdit::textAlign(QAction *a) //qui scatenata la ontextchanged
{
    FLAG_MODIFY_SYMBOL= true;  // si potrebbe sfruttare per evitare di fare delete + add ma chiedere al server di modificare
    //  il simbolo
    if (a == actionAlignLeft)
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        textEdit->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        textEdit->setAlignment(Qt::AlignJustify);
    FLAG_MODIFY_SYMBOL= false;
}

void TextEdit::setChecked(bool checked)
{
    textStyle(checked ? 5 : 4);
}

void TextEdit::indent()
{
    modifyIndentation(1);
}

void TextEdit::unindent()
{
    modifyIndentation(-1);
}

void TextEdit::modifyIndentation(int amount)
{
    QTextCursor cursor = textEdit->textCursor();
    cursor.beginEditBlock();
    if (cursor.currentList()) {
        QTextListFormat listFmt = cursor.currentList()->format();
        // See whether the line above is the list we want to move this item into,
        // or whether we need a new list.
        QTextCursor above(cursor);
        above.movePosition(QTextCursor::Up);
        if (above.currentList() && listFmt.indent() + amount == above.currentList()->format().indent()) {
            above.currentList()->add(cursor.block());
        } else {
            listFmt.setIndent(listFmt.indent() + amount);
            cursor.createList(listFmt);
        }
    } else {
        QTextBlockFormat blockFmt = cursor.blockFormat();
        blockFmt.setIndent(blockFmt.indent() + amount);
        cursor.setBlockFormat(blockFmt);
    }
    cursor.endEditBlock();
}

void TextEdit::currentCharFormatChanged(const QTextCharFormat &format)
{
    QString f = format.font().toString(); //DEBUG
    QString x = textEdit->font().toString(); //DEBUG
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}

/**
 * @brief scatenata a ogni cambiamento di posizione del cursore, commentata la parte relativa agli elenchi puntati, che sono stati rimossi
 */
void TextEdit::cursorPositionChanged()
{
    alignmentChanged(textEdit->alignment());
    if(!writingFlag){
        int index = textEdit->textCursor().anchor();
        my_cursor_position_changed(index);
    }
    writingFlag=false;
}

/**
 * @brief quando viene effettuata una copia o una taglia, setto un vettore charsFormat con lo stile della parte messa negli appunti
 */
void TextEdit::clipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData()){
        actionPaste->setEnabled(md->hasText());
        QTextCursor cursor = textEdit->textCursor();
        if(textEdit->textCursor().hasSelection()){
            charsFormat.clear();

            int inizio = cursor.selectionStart();
            int fine = cursor.selectionEnd();
            cursor.setPosition(cursor.selectionStart());
            for(int i = inizio ; i < fine; i++){
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,1);
                QTextCharFormat textFormat(cursor.charFormat());
                charsFormat.push_back(textFormat);
            }
        }
    }
#endif
}

void TextEdit::about()
{
    QMessageBox::about(this, tr("About"), tr("This example demonstrates Qt's "
                                             "rich text editing facilities in action, providing an example "
                                             "document for you to experiment with."));
}

void TextEdit::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    //FLAG_MODIFY_SYMBOL= true;// si potrebbe sfruttare per evitare di fare delete + add ma chiedere al server di modificare
    //  il simbolo
    QTextCursor cursor;
    cursor= textEdit->textCursor();
    /*if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);*/ // tolta per evitare che modifichi tutta la parola, voglio gestire carattere per carattere
    if (!cursor.hasSelection()){
        cursor.mergeCharFormat(format);
    }
    textEdit->mergeCurrentCharFormat(format);
    //FLAG_MODIFY_SYMBOL=false;
}

void TextEdit::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void TextEdit::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void TextEdit::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (a & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (a & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);
}

/**
 * @brief a ogni modifica del testo viene scatenata, si occupa di gestire i simboli che vengono poi mandati al server
 * @param pos
 * @param del
 * @param add
 */
void TextEdit::onTextChanged(int pos, int del, int add){

    QString added = textEdit->toPlainText().mid(pos, add);
    qDebug() << charsFormat.size();
    if( charsFormat.size() != 0  && (del == add - charsFormat.size())){
        added = textEdit->toPlainText().mid(0, this->charsFormat.size());
        del = 0;
        add = charsFormat.size();
    }


    QTextCursor cursor(textEdit->textCursor());
    QVector<QFont> fonts;
    QVector<QTextCharFormat> changeFormatVect;
    highlightUserText("Modifica testo - " + QString::number(pos) + " - " + QString::number(add));

    if(cursor.position() == pos){
        for(int i=0; i<del; i++){
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
            QTextCharFormat plainFormat(cursor.charFormat());
            changeFormatVect.push_back(plainFormat);
            //fonts.push_back(plainFormat.font());
        }
    }else if (cursor.position() == pos + del){
        for(int i=0; i<del; i++){
            QTextCharFormat plainFormat(cursor.charFormat());
            //fonts.push_front(plainFormat.font());
            changeFormatVect.push_back(plainFormat);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
        }
    }
    int counter = 0;
    //qDebug() << "Modifica: " << FLAG_MODIFY_SYMBOL;

    QVector<Message> messagesDel;
    for(int i=0; i<del; i++){
        writingFlag= true;

        if(pos+i != this->_symbols.size()){
            counter++;
            Message mess{'d', this->_symbols[pos+i]};
            //this->_symbols.erase(this->_symbols.begin() + pos);
            messagesDel.push_back(mess);
        }
    }
    if(del>0){
        this->_symbols.remove(pos, counter);
    }
    if(messagesDel.size() != 0){
        message_ready(messagesDel, this->fileIndex);
    }
    QVector<Message> messagesAdd;

    if(add==1){
        writingFlag=true;
        Message mess{};
        if(del > 0 && changeFormatVect.size() != 0){
            localInsert(pos, added[0], &(changeFormatVect[0]), mess);
        }else if( add == charsFormat.size()){
            localInsert(pos, added[0], &(charsFormat[0]), mess);
        } else{
            localInsert(pos, added[0], nullptr, mess);
        }
        messagesAdd.push_back(mess);
    }else{
        writingFlag=true;
        messagesAdd = localInsert(pos, added, del, add, changeFormatVect);
    }
    if(messagesAdd.size() != 0){
        message_ready(messagesAdd, this->fileIndex);
    }
    writingFlag=false;
}

/**
 * @brief funzione chiamata quando arrivano dal server messaggi con simboli da inserire o cancellare
 * @param messages
 * @param siteIdSender
 */
void TextEdit::onMessagesFromServer(QVector<Message> messages, int siteIdSender){
    if( messages[0].getAction() == 'i'){
        this->remoteInsert(messages);
    }else if(messages[0].getAction()=='d'){
        this->remoteDelete(messages, siteIdSender);
    }
}

//  Funzione utilizzata per creare un symbolo corrispondente all'inserimento di un singolo carattere appena fatto.
//  Alla fine di questa funzione possiamo utilizzare il messaggio passato come parametro per notificare al server l'inserimento
bool TextEdit::localInsert(int index, QChar value, QTextCharFormat *format, Message& m){
    QFont qf;
    if(format != nullptr){
        qf = (*format).font();
    }else{
        QTextCharFormat plainFormat(textEdit->textCursor().charFormat());
        qf = plainFormat.font();
    }
    QVector<int> pos;
    if ((index > (this->_symbols.size())) || index < 0) {
        return false;
    }
    this->counter++;
    pos = generatePos(index);
    if (pos.size() == 0) {
        return false;
    }

    std::shared_ptr<Symbol> s = std::make_shared<Symbol>(Symbol(pos, this->counter, this->siteId, value, actionTextBold->isChecked(),
                                                                actionTextItalic->isChecked(), actionTextUnderline->isChecked(),
                                                                alignToInt(textEdit->textCursor().blockFormat().alignment()) ,
                                                                qf.pointSize(),  textEdit->textColor(), qf.family()));

    this->_symbols.insert(this->_symbols.begin() + index, s);

    m.setAction('i');
    m.setSymbol(s);

    return true;
}


// Funzione utilizzata per creare un blocco di simboli corrispondente all'inserimento di un blocco di caratteri appena fatto.
//  Alla fine di questa funzione ci viene restituito il vettore contenente i messaggi da spedire al server.
QVector<Message> TextEdit::localInsert(int startIndex, QString added, int del, int add, QVector<QTextCharFormat> changeFormatVect){
    QVector<Message> messages;
    QVector<std::shared_ptr<Symbol>> newSymbols;
    QVector<QVector<int>> positionVector = generatePos(startIndex, added.size());

    for(int i=0; i<add; i++){
        writingFlag=true;
        std::shared_ptr<Symbol> s;
        if(added.size() > i ){
            if(del > 0 && changeFormatVect.size() != 0){
                s = createSymbol(startIndex+i, added[i], &(changeFormatVect[i]), positionVector[i]);
            }else if(add == charsFormat.size()){
                s = createSymbol(startIndex+i, added[i], &(charsFormat[i]), positionVector[i]);
            } else{
                s = createSymbol(startIndex+i, added[i], nullptr, positionVector[i]);
            }
            newSymbols.push_back(s);
            Message mess{'i', s};
            messages.push_back(mess);
        }
    }
    QVector<std::shared_ptr<Symbol>> inizio = this->_symbols.mid(0, startIndex);
    QVector<std::shared_ptr<Symbol>> fine = this->_symbols.mid(startIndex, this->_symbols.size() - startIndex);

    if(inizio.size() != 0){
        this->_symbols = inizio;
        this->_symbols.append(newSymbols);
    }else{
        this->_symbols = newSymbols;
    }

    if(fine.size() != 0){
        this->_symbols.append(fine);
    }

    return messages;
}

std::shared_ptr<Symbol> TextEdit::createSymbol(int index, QChar value, QTextCharFormat *format, QVector<int> position){
    QFont qf;
    if(format != nullptr){
        qf = (*format).font();
    }else{
        QTextCharFormat plainFormat(textEdit->textCursor().charFormat());
        qf = plainFormat.font();
    }

    this->counter++;
    if (position.size() == 0) {
        return nullptr;
    }
    std::shared_ptr<Symbol> s = std::make_shared<Symbol>(Symbol(position, this->counter, this->siteId, value, actionTextBold->isChecked(),
                                                                actionTextItalic->isChecked(), actionTextUnderline->isChecked(),
                                                                alignToInt(textEdit->textCursor().blockFormat().alignment()) ,
                                                                qf.pointSize(),  textEdit->textColor(), qf.family()));



    return s;
}


/**
 * @brief chiamata quando viene aperto un file, renderizza a schermo tutto il testo in maniera corretta
 * @param s: vettore di simboli da inserire nell'editor
 */
void TextEdit::onFileReady(QVector<std::shared_ptr<Symbol>> s){
    disconnect(textEdit, &QTextEdit::cursorPositionChanged,
               this, &TextEdit::cursorPositionChanged);

    this->_symbols = s;
    textEdit->textCursor().beginEditBlock();
    disconnect(textEdit->document(), &QTextDocument::contentsChange,
               this, &TextEdit::onTextChanged);
    int flag=0;
    QString buffer;
    QTextCharFormat headingFormat;
    QTextCursor cursor(textEdit->textCursor());
    QTextCharFormat plainFormat(cursor.charFormat());
    for(int i=0; i< s.size(); i++){
        std::shared_ptr<Symbol> sym = s[i];
        if(sym->getSiteId() == this->siteId){
            if(this->counter < sym->getCounter()){
                this->counter = sym->getCounter();
            }
        }

        if(flag==0){
            flag=1;
            cursor.setPosition(i);
            headingFormat.setFontWeight(sym->isBold() ? QFont::Bold : QFont::Normal);
            headingFormat.setFontItalic(sym->isItalic());
            headingFormat.setFontUnderline(sym->isUnderlined());
            headingFormat.setForeground(sym->getColor());
            headingFormat.setFontPointSize(sym->getTextSize());
            headingFormat.setFontFamily(sym->getFont());
            Qt::Alignment x = intToAlign(sym->getAlignment());
            textEdit->setAlignment(x);
            /*buffer.clear();
            buffer.append(sym->getValue());*/
        }
        if(i==(s.size()-1)){ //ultimo carattere devo scrivere tutto quello che ho nel buffer
            buffer.append(sym->getValue());
            cursor.insertText(buffer, headingFormat);
        }else{
            if(styleIsEqual(sym, s[i+1])){//il prossimo simbolo ha lo stesso font, appendo al buffer.
                buffer.append(sym->getValue());
            }else{
                flag=0;
                buffer.append(sym->getValue());
                cursor.insertText(buffer, headingFormat);
                buffer.clear();
            }
        }
    }

    textEdit->textCursor().endEditBlock();
    connect(textEdit->document(), &QTextDocument::contentsChange,
            this, &TextEdit::onTextChanged);
    textEdit->textCursor();
    connect(textEdit, &QTextEdit::cursorPositionChanged,
            this, &TextEdit::cursorPositionChanged);
}

/**
 * @brief funzione per paragonare lo stile di 2 simboli
 * @param s1: simbolo 1
 * @param s2: simbolo 2
 * @return true se sono uguali, altrimenti false
 */
bool TextEdit::styleIsEqual(std::shared_ptr<Symbol> s1, std::shared_ptr<Symbol> s2){
    return ((s1->getAlignment()==s2->getAlignment()) && (s1->isBold()==s2->isBold()) && (s1->isItalic()==s2->isItalic()) &&
            (s1->isUnderlined()==s2->isUnderlined()) && (s1->getTextSize()==s2->getTextSize()) &&
            (s1->getColor().name().compare(s2->getColor().name())==0) && (s1->getFont().compare(s2->getFont())==0));
}

/**
 * @brief disconnette la signal Uri_ready alla chiusura di un file
 */
void TextEdit::onFileClosed() {
    disconnect(client.get(), &Client::URI_Ready, this, &TextEdit::onURIReady);
}

/**
 * @brief corrispondenza tra tipo di allineamento e intero (valori presi sperimentalmente)
 * @param align: tipo di allineamento
 * @return
 */
int TextEdit::alignToInt(int align){
    if(align == 1){ //left
        return 0;
    }else if(align == 18){ //right
        return 1;
    }else if(align == 4){ //hcenter
        return 2;
    }else if(align == 8){ //justify
        return 3;
    }else{
        return 0;
    }
}

/**
 * @brief inverso della alignToInt, a partire dal codice intero con cui sono salvati gli allineamenti mappa sull'alignment corrispondente
 * @param val: valore intero corrispondente a un allineamento
 * @return
 */
Qt::Alignment TextEdit::intToAlign(int val){
    if(val == 0){
        return Qt::AlignLeft;
    }else if(val == 1){
        return Qt::AlignRight;
    }else if(val == 2){
        return Qt::AlignHCenter;
    }else if(val == 3){
        return Qt::AlignJustify;
    }else{
        return 0;
    }
}
/**
 * @brief genera il vettore posizione secondo il protocollo crdt
 * @param index: indice di posizione nell'editor
 * @return vettore di interi con la posizione all'interno
 */

//Restituisce più di un vettore di posizione. Utile nel caso di inserimenti in blocco.
QVector<QVector<int>> TextEdit::generatePos(int index, int nPosition) {
    QVector<QVector<int>> vector;
    bool firstPosition=true;
    int i;

    if ((index > (this->_symbols.size())) || index < 0) {
        return vector;
    }
    for(int k=0; k<nPosition; k++){
        QVector<int> pos;
        if (this->_symbols.empty() && firstPosition == true) { //il vettore è vuoto, quindi creo un vettore di posizione [1 , siteid] e lo metto da parte
            pos.push_back(index + 1);
        }
        else {
            if (index == 0 && firstPosition==true) { //indice uguale a 0, inserisco con compresa tra 0 e 1. Inserimento in TESTA
                //Non è più un inserimento in testa, nel caso sia già stato inserito un altro elemento (firstPosition==false)
                QVector<int> pos_successivo = _symbols[index]->getPosition();   //OTTENGO IL POS DELLA PRECEDENTE TESTA, ORA DEVO GENERARE IL NUOVO.
                QVector<int> vuoto;
                vuoto.push_back(0);
                pos = calcIntermediatePos(pos_successivo, vuoto);
            }
            else {
                QVector<int> pos_precedente;
                if(firstPosition==true){
                    pos_precedente = _symbols[index - 1]->getPosition();
                }else{
                    pos_precedente = vector[k - 1];
                }
                if (_symbols.size() == index) {         //Inserimento in CODA
                    for (i = 0; i < pos_precedente.size(); i++) {
                        if (pos_precedente[i] == INT_MAX) {
                            pos.push_back(INT_MAX);
                        }
                        else break;
                    }
                    pos.push_back(pos_precedente[i] + 1);

                }
                else {     //Inserimento in un indice diverso dalla coda o dalla testa.
                    QVector<int> pos_successivo = _symbols[index]->getPosition();
                    pos = calcIntermediatePos(pos_successivo, pos_precedente);
                }
            }
        }
        pos.push_back(this->siteId);
        vector.push_back(pos);
        firstPosition=false;
    }
    /*Il siteId viene messo per garantire che se due client scrivono nello stesso istante un carattere in una certa posizione,
     l'unicit� della posizione e garantito da questa ultima cifra.*/
    return vector;
}

/**
 * @brief genera il vettore posizione secondo il protocollo crdt
 * @param index: indice di posizione nell'editor
 * @return vettore di interi con la posizione all'interno
 */
// index: indice in cui inserire. Restituisco un vettore della posizione adatto.
QVector<int> TextEdit::generatePos(int index) {
    QVector<int> pos;
    int i;
    if ((index > (this->_symbols.size())) || index < 0) {
        return pos;//IO NON PERMETTEREI DI INSERIRE IN QUALSIASI PUNTO DEL NOSTRO VETTORE. SOLO INDICI DA 1 A SIZE+1 TODO ECCEZIONE
    }
    if (this->_symbols.empty()) {
        pos.push_back(index + 1);
    }
    else {
        if (index == 0) { //indice uguale a 0, inserisco con compresa tra 0 e 1. Inserimento in TESTA
            QVector<int> pos_successivo = _symbols[index]->getPosition();   //OTTENGO IL POS DELLA PRECEDENTE TESTA, ORA DEVO GENERARE IL NUOVO.
            QVector<int> vuoto;
            vuoto.push_back(0);
            pos = calcIntermediatePos(pos_successivo, vuoto);
        }
        else {
            QVector<int> pos_precedente = _symbols[index - 1]->getPosition();

            if (_symbols.size() == index) {         //Inserimento in CODA
                for (i = 0; i < _symbols[index - 1]->getPosition().size(); i++) {
                    if (pos_precedente[i] == INT_MAX) {
                        pos.push_back(INT_MAX);
                    }
                    else break;
                }
                pos.push_back(pos_precedente[i] + 1);

            }
            else {     //Inserimento in un indice diverso dalla coda o dalla testa.
                QVector<int> pos_successivo = _symbols[index]->getPosition();
                pos = calcIntermediatePos(pos_successivo, pos_precedente);
            }
        }
    }
    pos.push_back(this->siteId);
    /*Il siteId viene messo per garantire che se due client scrivono nello stesso istante un carattere in una certa posizione,
     l'unicit� della posizione e garantito da questa ultima cifra.*/
    return pos;
}

/**
 * @brief TextEdit::calcIntermediatePos
 * @param pos_sup
 * @param pos_inf
 * @return
 */
QVector<int> TextEdit::calcIntermediatePos(QVector<int> pos_sup, QVector<int> pos_inf) {
    QVector<int> pos;
    int inf, sup, k = 20, i = 0, nuovo_valore, flag = 0, MAX = INT_MAX - 100;
    //todo con un comparatore posso verificare che successivo sia davvero un num frazionario maggiore di prec

    /***
     In questa funzione si cerca di creare un vettore di posizioni che abbia un valore compreso tra quello indicato
     come pos_sup e quello indicato come pos_inf. Si � deciso di non limitarci alle cifre che vanno da 0 fino
     a 9 ma di arrivare fino a MAX. Per ogni posizione del vettore si cerca di inserire nel nuovo vettore un valore
     che sia maggiore di pos_inf[i] (se non esiste questa posizione del vettore si usa il valore 0) e minore di
     pos_sup[i] (se non esiste questa posizione del vettore si usa il valore MAX), e nel caso questo valore intermedio
     non esista si copia il valore pos_inf[i] per continuare la ricerca incrementando i. Quando troviamo un valore
     intermedio si inserisce nel nuovo vettore e finisce qui la nostra ricerca di un vettore intermedio.
     Il calcolo del valore intermedio viene fatto nella seguente maniera: se la differenza tra i due numeri � superiore
     a un valore constante (k) si inserisce il valore inf + k, altrimenti si cerca di inserire un valore che stia a
     met� tra pos_inf[i] e pos_sup[i]. Questo viene fatto per provare a mantenere dei valori intermedi liberi tra i vari
     campi del vettore e allo stesso tempo in caso di normale scrittura sequenziale, non arrivare velocemente al valore
     MAX ma con incrementi costanti di k. La variabile flag assume il valore 1 quando abbiamo trovato un valore di
     pos_sup[i] superiore a pos_inf[i]. Infatti dal prossimo valore di i non abbiamo pi� bisogno di trovare un valore
     tra pos_sup[i] e pos_inf[i], ma ci basta solo che sia superiore a pos_inf[i] e inferiore a MAX (guardare esempio).


                                                    Dopo questo 6 il valore di flag sar� 1
     Esempio:   vettore con indice maggiore     0 |6| 1
                vettore con indice minore       0 |5| 1
                                                -------
                                 Risultato      0 |5|(k+1)  <--- Qui non ho pi� bisogno di cercare un valore intermedio
                                                                 perch� il flag vale 1.
    ***/



    int lung_vett_max = (int)std::max(pos_sup.size(), pos_inf.size());

    for (i = 0; i < lung_vett_max; i++) {
        if (pos_inf.size() <= i)
            inf = 0;
        else
            inf = pos_inf[i];
        if (pos_sup.size() <= i)
            sup = MAX;
        else
            sup = pos_sup[i];

        if (flag == 1 && (inf + k) < MAX) {
            pos.push_back(inf + k);
            return pos;
        }
        if ((inf + 1) < sup) {              //cerco un valore intermedio
            nuovo_valore = k + inf;
            if (nuovo_valore >= sup || nuovo_valore > MAX) {
                nuovo_valore = ((sup - inf) / 2) + inf;
                if (nuovo_valore >= sup || nuovo_valore > MAX) {
                    pos.push_back(inf);     //non esiste un valore intermedio
                    continue;               //continua a ciclare il for
                }
            }
            pos.push_back(nuovo_valore);    //valore intermedio trovato, finisco la ricerca.
            return pos;
        }
        else {                            // se non esiste un valore intermedio
            if (inf < sup) {
                flag = 1;                   //da ora in poi qualsiasi cifra superiore a inf andr� bene
            }
            pos.push_back(inf);
        }
    }
    pos.push_back(k);
    return pos;
}

/**
 * @brief funzione che inserisce simboli arrivati dal server
 * @param messages: messaggi contenenti i simboli da inserire
 */
void TextEdit::remoteInsert(QVector<Message> messages){ //per ora gestito solo il caso in cui ci siano solo caratteri normali nella nostra app.
    disconnect(textEdit->document(), &QTextDocument::contentsChange,this, &TextEdit::onTextChanged);

    QString buffer;
    int index = _symbols.size();
    int count = 0;
    QTextCharFormat headingFormat;
    QTextCursor cursor = textEdit->textCursor();
    disconnect(textEdit, &QTextEdit::cursorPositionChanged,this, &TextEdit::cursorPositionChanged);
    for(int i=0; i<messages.size(); i++) {
        std::shared_ptr<Symbol> sym = messages[i].getSymbol();
        int size = _symbols.size();
        if (count == 0) {
            index = findIndexFromNewPosition(sym->getPosition());
            if(index!=-1) {
                cursor.setPosition(index);
                QTextCharFormat plainFormat(cursor.charFormat());
                headingFormat.setFontWeight(sym->isBold() ? QFont::Bold : QFont::Normal);
                headingFormat.setFontItalic(sym->isItalic());
                headingFormat.setFontUnderline(sym->isUnderlined());
                headingFormat.setForeground(sym->getColor());
                headingFormat.setFontPointSize(sym->getTextSize());
                headingFormat.setFontFamily(sym->getFont());
                /*In un inserimento non si possono avere messaggi da diversi client*/
                if(sym->getSiteId() == flag_one_highlighted || flag_all_highlighted){
                    headingFormat.setBackground(colorableUsers[sym->getSiteId()]->getColor());
                }else{
                    headingFormat.setBackground(Qt::white);
                }
                Qt::Alignment intAlign = intToAlign(sym->getAlignment());
                textEdit->setAlignment(intAlign);
                count++;
                buffer.append(sym->getValue());
            }
        }
        else
        {
            bool successivo = false;
            if (sym->getPosition() > messages[i-1].getSymbol()->getPosition()) {
                if ((index != size && sym->getPosition() < _symbols[index]->getPosition()) || (index == size)) {
                    if(styleIsEqual(sym, messages[i-1].getSymbol())) {
                        count++;
                        successivo = true;
                        buffer.append(sym->getValue());
                    }
                }
            }
            if (!successivo) {
                QVector<std::shared_ptr<Symbol>> inizio = _symbols.mid(0, index);
                QVector<std::shared_ptr<Symbol>> fine = _symbols.mid(index, _symbols.size()-index);
                QVector<std::shared_ptr<Symbol>> added;
                for(int j = i-count; j < i; j++) {
                   added.append(messages[j].getSymbol());
                }
                _symbols = inizio;
                _symbols.append(added);
                _symbols.append(fine);
                count = 0;
                i--;
                cursor.insertText(buffer, headingFormat);
                buffer.clear();
            }
        }
    }
    if (index != -1) {
        QVector<std::shared_ptr<Symbol>> inizio = _symbols.mid(0, index);
        QVector<std::shared_ptr<Symbol>> fine = _symbols.mid(index, _symbols.size()-index);
        QVector<std::shared_ptr<Symbol>> added;
        for(int j = messages.size()-count; j < messages.size(); j++) {
           added.append(messages[j].getSymbol());
        }
        _symbols = inizio;
        _symbols.append(added);
        _symbols.append(fine);
        cursor.insertText(buffer, headingFormat);
        buffer.clear();
    }
    cursorPositionChanged();
    connect(textEdit, &QTextEdit::cursorPositionChanged,this, &TextEdit::cursorPositionChanged);
    if(index != -1){
        remoteCursorChangePosition(index+1, messages[messages.size()-1].getSymbol()->getSiteId());
    }else{
        remoteCursorChangePosition(0, messages[messages.size()-1].getSymbol()->getSiteId());
    }
    connect(textEdit->document(), &QTextDocument::contentsChange,this, &TextEdit::onTextChanged);
}


/**
 * @brief funzione che cancella i simboli arrivati dal server
 * @param messages: messaggi contenenti i simboli da cancellare
 * @param siteIdSender: siteId del mittente dei simboli da cancellare
 */
void TextEdit::remoteDelete(QVector<Message> messages, int siteIdSender){

    disconnect(textEdit->document(), &QTextDocument::contentsChange,this, &TextEdit::onTextChanged);
    disconnect(textEdit, &QTextEdit::cursorPositionChanged,this, &TextEdit::cursorPositionChanged);

    //int  nextIndex = -1, counter=1, startRemove;
    QTextCursor cursor = textEdit->textCursor();

    int index;
    int size;

    int count = 0;

    for(int i=0; i<messages.size(); i++){
        size = this->_symbols.size();
        std::shared_ptr<Symbol> sym = messages[i].getSymbol();
        if (count == 0) {
            index = findIndexFromExistingPosition(sym->getPosition());
            if(index != -1 && this->_symbols[index]->getSiteId() == sym->getSiteId() && this->_symbols[index]->getCounter() == sym->getCounter()) {
                count++;
            }
            else {
                index = -1;
            }
        }
        else {
            bool successivo = false;
            if (index + count < size && this->_symbols[index + count]->getPosition() == sym->getPosition()) {
                if (this->_symbols[index + count]->getSiteId() == sym->getSiteId() && this->_symbols[index + count]->getCounter() == sym->getCounter()) {
                    count++;
                    successivo = true;
                }
            }
            if (!successivo) {
                deleteFromEditor(index, index + count - 1, cursor);
                count = 0;
                i--;
            }
        }

    }
    if (count != 0) {
        deleteFromEditor(index, index + count - 1, cursor);
    }
    connect(textEdit, &QTextEdit::cursorPositionChanged,this, &TextEdit::cursorPositionChanged);
    cursorPositionChanged();
    if(index!=-1){
        remoteCursorChangePosition(index, siteIdSender);
    }
    connect(textEdit->document(), &QTextDocument::contentsChange,this, &TextEdit::onTextChanged);
}



void TextEdit::deleteFromEditor(int firstIndex, int lastIndex, QTextCursor cursor){
    cursor.setPosition(firstIndex, QTextCursor::MoveAnchor);
    cursor.setPosition(lastIndex+1, QTextCursor::KeepAnchor);
    cursor.selectedText();
    cursor.removeSelectedText();
    QVector<std::shared_ptr<Symbol>> fine = this->_symbols.mid(lastIndex+1, this->_symbols.size() - lastIndex+1);
    this->_symbols = this->_symbols.mid(0, firstIndex);
    this->_symbols.append(fine);
}


/**
 * @brief TextEdit::findIndexFromNewPosition
 * @param position
 * @return
 */
int TextEdit::findIndexFromNewPosition(QVector<int> position){
    int index = this->_symbols.size();
    int size = this->_symbols.size();
    if (size == 0) {
        index = 0;
    }

    if (size == 1) {
        if (this->_symbols[0]->getPosition() > position) {
            index = 0;
        }
        else {
            index = 1;
        }
    }
    if (size > 1) {
        int flag = 0;
        if (position < this->_symbols[0]->getPosition()) {
            index = 0;
            flag = 1;
        }
        else if(position > this->_symbols[size-1]->getPosition()){
            index = size;
            flag = 1;
        }
        int i;
        int dx, sx;
        dx = size -1;
        sx = 0;

        while (flag == 0)
        {
            i = (dx + sx) / 2;

            if (this->_symbols[i]->getPosition() < position && position < this->_symbols[i+1]->getPosition()) {
                flag = 1;
                index = i+1;
            }else if(dx == sx+1){
                flag = 1;
                index = -1;
            }else{
                if (this->_symbols[i]->getPosition() > position) {// il nostro simbolo ha pos minore del simbolo indicizzato -> andare a sinistra;
                    dx = i;
                }
                else if(this->_symbols[i+1]->getPosition() < position){
                    sx = i;
                }else{
                    flag = 1;
                    index = -1;
                }
            }

        }
    }
    return index;
}

/**
 * @brief TextEdit::findIndexFromExistingPosition
 * @param position
 * @return
 */
int TextEdit::findIndexFromExistingPosition(QVector<int> position){
    int index = this->_symbols.size();
    int size = this->_symbols.size();
    if (size == 0) {
        index = -1;
    }

    if (size >= 1) {
        int flag = 0;
        if (position == this->_symbols[0]->getPosition()) {
            index = 0;
            flag = 1;
        }
        else if(position == this->_symbols[size-1]->getPosition()){
            index = size-1;
            flag = 1;
        }
        int i;
        int dx, sx;
        dx = size -1;
        sx = 0;

        while (flag == 0)
        {
            i = (dx + sx) / 2;
            if (this->_symbols[i]->getPosition() == position) {
                flag = 1;
                index = i;
            }else if(dx == sx+1 || dx == sx){
                flag = 1;
                index = -1;
            }
            else {
                if (this->_symbols[i]->getPosition() > position) {// il nostro simbolo ha pos minore del simbolo indicizzato -> andare a sinistra;
                    dx = i;
                }
                else {
                    sx = i;
                }
            }

        }
    }

    return index;
}

/**
 * @brief setta il flag che dice se ho fatto la richiesta di una URI
 * @param status: valore da settare
 */
void TextEdit::setUriRequest(bool status) {
    this->uriRequest = status;
}

/**
 * @brief mostra in una dialog l'URI del file
 * @param uri: uri del file
 */
void TextEdit::onURIReady(QString uri) {
    if(uriRequest) {
        setUriRequest(false);
        ShowUriDialog dialog;
        dialog.setUri(uri);
        dialog.setModal(true);
        if(dialog.exec()){}
    }
}

/**
 * @brief richiede al server l'uri per condividere il file
 */
void TextEdit::onShareURIButtonPressed(){

    setUriRequest(true);
    client.get()->requestURI(this->fileIndex);
}


/**
 * @brief cambia posizione del cursore di un altro utente collegato
 * @param cursorPos: posizione del cursore
 * @param siteId: siteId dell'utente proprietario del cursore
 */
void TextEdit::remoteCursorChangePosition(int cursorPos, int siteId) {
    QTextCursor cursor(textEdit->textCursor());
    cursor.setPosition(cursorPos);//setto la posizione per poter prendere le coordinate
    QTextCharFormat plainFormat(cursor.charFormat());
    QRect editor = textEdit->rect();

    int editor_height = editor.height();//altezza editor;
    QRect rt = textEdit->cursorRect(cursor);
    int rt_height = rt.height();
    std::shared_ptr<UserCursor> uc = cursorsMap[siteId];
    uc->setPos(cursorPos);

    uc->getCursor()->setFixedHeight(rt_height);
    uc->getCursor()->setFixedWidth(2);
    int x2 = rt.x() - 1;
    int y2 = rt.y();
    if (y2 < 0) y2 = 0;
    if (y2 > editor_height) y2 = editor_height - 10;
    uc->getCursor()->hide();
    uc->getCursor()->move(x2, y2);
    uc->getCursor()->show();

}

/**
 * @brief cambia posizione del cursore di un altro utente collegato
 * @param cursorIndex: posizione del cursore
 * @param siteIdSender: siteId dell'utente proprietario del cursore
 */
void TextEdit::onRemoteCursorChanged(int cursorIndex, int siteIdSender){
    remoteCursorChangePosition(cursorIndex, siteIdSender);
}
