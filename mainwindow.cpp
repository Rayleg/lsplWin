#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QCoreApplication>
#include <QtCore>
#include <QFile>
#include <QFileDialog>
#include <QTextEdit>
#include <QMessageBox>
#include <QByteArray>
#include <QTextStream>
#include <QTableWidget>
#include <QMainWindow>

#include <lspl/Namespace.h>
#include <lspl/patterns/PatternBuilder.h>
#include <lspl/patterns/Alternative.h>
#include <lspl/patterns/matchers/Matcher.h>
#include <lspl/patterns/Pattern.h>
#include <lspl/patterns/matchers/Context.h>
#include <lspl/text/readers/PlainTextReader.h>
#include <lspl/text/Text.h>
#include <lspl/text/attributes/SpeechPart.h>
#include <lspl/text/Match.h>
#include <lspl/base/Exception.h>
#include <lspl/transforms/ContextRetriever.h>
#include <lspl/transforms/TextTransform.h>
#include <lspl/morphology/Morphology.h>


#include <sys/stat.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pbAddTemplate, SIGNAL(clicked()), this, SLOT(addTemplate()));
    connect(ui->pbApplyTemplate, SIGNAL(clicked()), this, SLOT(applyTemplates()));
    connect(ui->loadText, SIGNAL(triggered()), this, SLOT(loadText()));
    connect(ui->loadTemplate, SIGNAL(triggered()), this, SLOT(loadTemplate()));
    connect(ui->tableMatch, SIGNAL(cellClicked(int,int)), this, SLOT(showMatches(int,int)));
    ui->pbAddTemplate->setVisible(false);

    // LSPL initializing

    // Create namespace for everything
    lsplNS = new lspl::Namespace(); // SIGSEGV received here
    // Define builder in namespace ns
    lsplPatternBuilder = new lspl::patterns::PatternBuilder(lsplNS);
}

/* On the push button 'Add template' slot */
void MainWindow::addTemplate() {
    qDebug() << "add template slot";
}

/* Load template file function (on 'load template' menu item slot) */
void MainWindow::loadTemplate() {
    // Clear old data: TODO
    //lsplNS->

    qDebug() << "load template slot";
    QString filename = QFileDialog::getOpenFileName(this, QString("Open template file"));
    if (filename == "")
        return;
    QFile tempfile(filename);

    if (!tempfile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QString("Error"), QString("Cannot open template file"));
        return;
    }
    QTextStream ts(&tempfile);
    ts.setCodec("cp1251");

    /* Add any file line to template list */
    QString temp;
    while (!ts.atEnd()) {
        temp = ts.readLine();
        try {
            lsplPatternBuilder->build(temp.toStdString());
        }
        catch (lspl::patterns::PatternBuildingException ex) {
            QString error_content(temp);

            error_content.append(QString(":\n"));
            error_content.append(QString(ex.what()));
            QMessageBox::warning(0, "Wrong pattern", temp, QMessageBox::Ok);
        }
        ui->listTemplate->addItem(temp);
    }
    tempfile.close();
}

void MainWindow::applyTemplates() {\
    // Get text from field
    lspl::text::readers::PlainTextReader reader;
    QTextCodec *codec = QTextCodec::codecForName("CP1251");
    QByteArray ba = codec->fromUnicode(ui->textEdit->toPlainText());
    lsplText = reader.readFromString(ba.constData());
    //QMessageBox::about(0, "Text content", QString::fromStdString(lsplText->getContent()));

    QTableWidget *tbl = ui->tableMatch;

    int cc = lsplNS->getPatternCount();
    tbl->setRowCount(cc);
    QString m("");
    lspl::text::MatchList matches;
    // Fill table
    for (int i = 0; i < cc; i++) {
        lspl::patterns::PatternRef pattern = lsplNS->getPatternByIndex(i);
        tbl->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(pattern->getName())));
        matches = lsplText->getMatches(pattern);

        // For each match
        tbl->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(pattern->getSource())));
        m = QString("");
        for (lspl::text::MatchList::iterator match_iter = matches.begin();
             match_iter != matches.end();
             ++match_iter) {
            lspl::text::MatchConstRef match = *match_iter;
            //lspl::text::MatchVariantConstRef mv = *match_iter;

            QString mstr = codec->toUnicode(match->getRangeString().c_str());
            m += mstr + QString(", ");
        }
        tbl->setItem(i, 2, new QTableWidgetItem(m));
    }
}

/* Load text file function (on 'load text' menu item slot) */
void MainWindow::loadText() {
    qDebug() << "load text slot";
    QString filename = QFileDialog::getOpenFileName(this, QString("Open text file"));
    if (filename == "")
        return;
    QFile text(filename);

    if (!text.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QString("Error"), QString("Cannot open text file"));
        return;
    }
    // The following can be changed with dynamic library
    QTextStream ts(&text);
    ts.setCodec("cp1251");

    ui->textEdit->setText(ts.readAll());
    text.close();
}

/* Show all matches in the text for appropriate template (by row) */
void MainWindow::showMatches( int row, int ) {
    QTextEdit::ExtraSelection es;
    QList<QTextEdit::ExtraSelection> ls;
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.clearSelection();
    ui->textEdit->setTextCursor(cursor);
    for (int i = 0; i < templates[row].matches.size(); i++) {
        es.format.setBackground(QBrush(Qt::yellow));
        es.cursor = ui->textEdit->textCursor();
        es.cursor.setPosition(templates[row].matches[i].start);
        es.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, templates[row].matches[i].finish - templates[row].matches[i].start);
        ls.append(es);
    }
    ui->textEdit->setExtraSelections(ls);
}


/* Destructor for the main window */
MainWindow::~MainWindow()
{
    //ui->listTemplate->re
    delete ui;
}
