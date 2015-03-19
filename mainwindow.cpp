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
#include <QInputDialog>

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

//using lspl::text::attributes::AttributeKey;

std::string getAttrString( lspl::text::MatchRef m ) {
    lspl::text::attributes::AttributeKey attributes[13] = {
        lspl::text::attributes::AttributeKey::CASE,
        lspl::text::attributes::AttributeKey::NUMBER,
        lspl::text::attributes::AttributeKey::GENDER,
        lspl::text::attributes::AttributeKey::DOC,
        lspl::text::attributes::AttributeKey::TENSE,
        lspl::text::attributes::AttributeKey::ANIMATE,
        lspl::text::attributes::AttributeKey::FORM,
        lspl::text::attributes::AttributeKey::MODE,
        lspl::text::attributes::AttributeKey::PERSON,
        lspl::text::attributes::AttributeKey::REFLEXIVE,
        lspl::text::attributes::AttributeKey::STEM,
        lspl::text::attributes::AttributeKey::BASE,
        lspl::text::attributes::AttributeKey::TEXT
    };

    std::string res("");
    bool isFirst = true;
    for (int i = 0; i < 13; i++) {
        if (m->getAttribute(attributes[i]) != lspl::text::attributes::AttributeValue::UNDEFINED) {
            if (isFirst) {
                isFirst = false;
                res = m->getAttribute(attributes[i]).getString();
            }
            else
               res += "," + m->getAttribute(attributes[i]).getString();
        }
    }
    return res;
}

std::string getAttrString2( lspl::text::MatchRef m ) {


    std::string res;
    res =
    //cout << "Case attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::CASE).getString() + "," +
    //cout << "Number attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::NUMBER).getString() + "," +
    //cout << "Gender attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::GENDER).getString() + "," +
    //cout << "Doc attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::DOC).getString() + "," +
    //cout << "Tense attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::TENSE).getString() + "," +
    //cout << "Animate attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::ANIMATE).getString() + "," +
    //cout << "Form attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::FORM).getString() + "," +
    //cout << "Mode attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::MODE).getString() + "," +
    //cout << "Person attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::PERSON).getString() + "," +
    //cout << "Reflexive attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::REFLEXIVE).getString() + "," +
    //cout << "Stem attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::STEM).getString() + "," +
    //cout << "Base attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::BASE).getString() + "," +
    //cout << "Text attr: " <<
        m->getAttribute(lspl::text::attributes::AttributeKey::TEXT).getString();
    return res;
}

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
    //ui->pbAddTemplate->setVisible(false);

    // LSPL initializing

    // Create namespace for everything
    lsplNS = new lspl::Namespace();
    // Define builder in namespace ns
    lsplPatternBuilder = new lspl::patterns::PatternBuilder(lsplNS);
}

/* On the push button 'Add template' slot */
void MainWindow::addTemplate() {
    qDebug() << "add template slot";
    bool isOk;
    QString pattern = QInputDialog::getText(this, "Input pattern", "Pattern:", QLineEdit::Normal, "", &isOk);
    if (!isOk)
        return;
    try {
        lspl::patterns::PatternBuilder::BuildInfo info = lsplPatternBuilder->build(pattern.toStdString());

        if ( info.parseTail.length() != 0 ) {
            std::cout << "Error during parsing '" << pattern.toStdString() << "': '" << info.parseTail << "' not parsed" << std::endl;
        }
        else
            ui->listTemplate->addItem(pattern);
    }
    catch (lspl::patterns::PatternBuildingException ex) {
        QString error_content(pattern);

        error_content.append(QString(":\n"));
        error_content.append(QString(ex.what()));
        QMessageBox::warning(0, "Wrong pattern", error_content, QMessageBox::Ok);
    }
}

/* Load template file function (on 'load template' menu item slot) */
void MainWindow::loadTemplate() {
    // Clear old data: TODO
    // Test for test
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
            lspl::patterns::PatternBuilder::BuildInfo info = lsplPatternBuilder->build(temp.toStdString());

            if ( info.parseTail.length() != 0 ) {
                std::cout << "Error during parsing '" << temp.toStdString() << "': '" << info.parseTail << "' not parsed" << std::endl;
            }
        }
        catch (lspl::patterns::PatternBuildingException ex) {
            QString error_content(temp);

            error_content.append(QString(":\n"));
            error_content.append(QString(ex.what()));
            QMessageBox::warning(0, "Wrong pattern", error_content, QMessageBox::Ok);
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

    // Clear table
    int n = tbl->rowCount();
    for (int i = 0; i < n; i++)
        tbl->removeRow(0);

    lspl::text::MatchList matches;
    int rowCount = 0;
    int patternCount = lsplNS->getPatternCount();
    // Calculate number of rows
    for (int i = 0; i < patternCount; i++) {
        lspl::patterns::PatternRef pattern = lsplNS->getPatternByIndex(i);
        matches = lsplText->getMatches(pattern);
        // For each match
        for (lspl::text::MatchList::iterator match_iter = matches.begin();
             match_iter != matches.end();
             ++match_iter) {
            lspl::text::MatchConstRef match = *match_iter;
            rowCount += match->getVariantCount();
        }
    }

    tbl->setRowCount(rowCount);

    // Fill table
    int patternRowIndex = 0, matchRowIndex = 0, transRowIndex = 0;
    for (int i = 0; i < patternCount; i++) {
        lspl::patterns::PatternRef pattern = lsplNS->getPatternByIndex(i);
        tbl->setItem(patternRowIndex, 0, new QTableWidgetItem(QString::fromStdString(pattern->getName())));
        matches = lsplText->getMatches(pattern);

        // For each match
        tbl->setItem(patternRowIndex, 1, new QTableWidgetItem(QString::fromStdString(pattern->getSource())));
        matchRowIndex = patternRowIndex;
        for (lspl::text::MatchList::iterator match_iter = matches.begin();
             match_iter != matches.end();
             ++match_iter) {
            lspl::text::MatchRef match = *match_iter;

            QString mstr = codec->toUnicode(match->getRangeString().c_str());
            tbl->setItem(matchRowIndex, 2, new QTableWidgetItem(mstr));
            //match->
            transRowIndex = matchRowIndex;
            uint varCount = match->getVariantCount();
            for (uint varIdx = 0; varIdx < varCount; ++varIdx) {
                try {
                    tbl->setItem(transRowIndex, 3,
                                 new QTableWidgetItem(
                                     codec->toUnicode(
                                         getAttrString(match).c_str())));
                    if (match->getVariant(varIdx)->alternative.hasTransform())
                        tbl->setItem(transRowIndex, 4,
                                     new QTableWidgetItem(
                                         QString::fromStdString(
                                             match->getVariant(varIdx)->getTransformResult<std::string>())));
                    else
                        tbl->setItem(transRowIndex, 4,
                                     new QTableWidgetItem(
                                         QString("No transform for variant")));
                } catch (lspl::base::Exception ex) {
                    std::cout << ex.what() << std::endl;
                }
                transRowIndex++;

            }
            matchRowIndex += varCount;
        }
        patternRowIndex = matchRowIndex;
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
    row = row;
//    QTextEdit::ExtraSelection es;
//    QList<QTextEdit::ExtraSelection> ls;
//    QTextCursor cursor = ui->textEdit->textCursor();
//    cursor.clearSelection();
//    ui->textEdit->setTextCursor(cursor);
//    for (int i = 0; i < templates[row].matches.size(); i++) {
//        es.format.setBackground(QBrush(Qt::yellow));
//        es.cursor = ui->textEdit->textCursor();
//        es.cursor.setPosition(templates[row].matches[i].start);
//        es.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, templates[row].matches[i].finish - templates[row].matches[i].start);
//        ls.append(es);
//    }
//    ui->textEdit->setExtraSelections(ls);
}


/* Destructor for the main window */
MainWindow::~MainWindow()
{
    //ui->listTemplate->re
    delete ui;
}
