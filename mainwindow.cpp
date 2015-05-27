#include "mainwindow.h"
#include "omonimdialog.h"
#include "ui_mainwindow.h"
#include "matchtablemodel.h"

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
#include <lspl/transforms/TextTransformBuilder.h>
#include <lspl/morphology/Morphology.h>


#include <sys/stat.h>

//using lspl::text::attributes::AttributeKey;


/* The main window representation class constructor.
 * Attach handler for controlls items.
 * Initialize lspl objects.
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Attach slots for signals for interface buttons
    connect(ui->pbAddTemplate, SIGNAL(clicked()), this, SLOT(addTemplate()));
    connect(ui->pbApplyTemplate, SIGNAL(clicked()), this, SLOT(applyTemplates()));
    connect(ui->loadText, SIGNAL(triggered()), this, SLOT(loadText()));
    connect(ui->loadTemplate, SIGNAL(triggered()), this, SLOT(loadTemplate()));
    connect(ui->showOmonims, SIGNAL(triggered()), this, SLOT(showOmonims()));
    connect(ui->saveStatistic, SIGNAL(triggered()), this, SLOT(saveData()));
    connect(ui->saveText, SIGNAL(triggered()), this, SLOT(saveText()));
    connect(ui->textInfo, SIGNAL(triggered()), this, SLOT(textInfo()));

    ui->tableMatch->setModel(&matchTableModel);

    // LSPL initializing

    // Create lspl namespace (storage for patterns)
    lsplNS = new lspl::Namespace();
    // Define pattern builder
    lsplPatternBuilderTransform = new lspl::patterns::PatternBuilder(lsplNS, new lspl::transforms::TextTransformBuilder(lsplNS));
}

/* Create dialog with omonims list function */
void MainWindow::showOmonims() {
    qDebug() << "show omonims slot\n";
    OmonimDialog * dialog = new OmonimDialog();

    // Get last version of text
    getTextFromView();
    // Set data for view
    dialog->setData(lsplText->getWords());
    // Run dialog window
    dialog->exec();
}

// Add pattern by text definition to system function */
void MainWindow::addPattern( const QString &pattern ) {
    QTextCodec * codec = QTextCodec::codecForName("CP1251");
    lspl::patterns::PatternBuilder::BuildInfo info;

    try {
        // Try to build pattern
        info = lsplPatternBuilderTransform->build(std::string(codec->fromUnicode(pattern).constData()));
    }
    catch (lspl::patterns::PatternBuildingException ex) {
        // Catch exception (Pattern may be added to system)
        QString error_content(pattern);

        error_content.append(QString(":\n"));
        error_content.append(QString(ex.what()));
        qDebug() << error_content.toStdString().c_str() << "\n";
    }
    // Analyze pattern parsing
    if (info.parseTail.length() != 0) {
        qDebug() << "Error during parsing '" << codec->toUnicode(pattern.toStdString().c_str()) << "': '" << info.parseTail.c_str() << "' not parsed\n";
    }
    else
        // Show pattern in list if success
        ui->listTemplate->addItem(pattern); // pattern vs codec->toUnicode(pattern.toStdString().c_str())
}

/* Add pattern from interface form function */
void MainWindow::addTemplate() {
    qDebug() << "add template slot";
    bool isOk;

    // Get text definition of pattern from dialog
    QString pattern = QInputDialog::getText(this, "Input pattern", "Pattern:", QLineEdit::Normal, "", &isOk);
    if (!isOk)
        return;
    // Add pattern to system
    addPattern(pattern);
}

/* Load patterns from file function */
void MainWindow::loadTemplate() {
    qDebug() << "load template slot\n";
    // Get file name for loading
    QString filename = QFileDialog::getOpenFileName(this, QString("Open template file"));
    if (filename == "")
        return;
    QFile tempfile(filename);

    if (!tempfile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, QString("Error"), QString("Cannot open template file"));
        return;
    }
    QTextStream ts(&tempfile);
    ts.setCodec("CP1251");

    /* Add any file line to template list */
    while (!ts.atEnd())
        addPattern(ts.readLine().toUtf8());
    tempfile.close();
}

/* Get text from edit view and load it to inner representation */
void MainWindow::getTextFromView() {
    lspl::text::readers::PlainTextReader reader;
    QTextCodec *codec = QTextCodec::codecForName("CP1251");
    // Get text from edit view and convert it to CP1251 encoding
    QByteArray ba = codec->fromUnicode(ui->textEdit->toPlainText());
    // Translate to innner presentation
    lsplText = reader.readFromString(ba.constData());
}

/* Build table of matches for registered patterns and show it (on 'tableMatch' buuton slot) */
void MainWindow::applyTemplates() {\
    // Get text from field
    getTextFromView();

    // Update data in table model
    matchTableModel.setNewData(lsplNS, lsplText);
    // Update table
    ui->tableMatch->reset();
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
    QTextStream ts(&text);
    ts.setCodec("CP1251");

    // Show text on edit view
    ui->textEdit->setText(ts.readAll());
    text.close();
}

/* Save statistic about pattern matching */
void MainWindow::saveData() {
    qDebug() << "save data slot";
    // Get save file name from dialog
    QString filename = QFileDialog::getSaveFileName(this, QString("Save data file"));
    if (filename == "")
        return;
    QFile text(filename);

    if (!text.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QString("Error"), QString("Cannot open text file ") + filename);
        return;
    }
    QTextStream ts(&text);
    int patternCount = lsplNS->getPatternCount();
    lspl::text::MatchList matches;
    int start = -1, end = -1, dif = 0;

    // For each patterns in system save statistic
    for (int i = 0; i < patternCount; i++) {
        lspl::patterns::PatternRef pattern = lsplNS->getPatternByIndex(i);
        ts << pattern->getName().c_str() << " = " << pattern->getSource().c_str() << ":" << "\n";
        // For each match add it to the tree to parent pattern
        if (!lsplText) // No text in system
            continue;
        matches = lsplText->getMatches(pattern);
        ts << "all matches: " << matches.size() << "\n";
        // Count no crossing matches
        dif = 0;
        for (int i = 0; i < (int)matches.size(); i++) {
            // Using order of matches during test
            lspl::text::MatchRef match = matches[i];
            // Test for crossing matches
            if (end < (int)match->getRangeStart() || (int)match->getRangeEnd() < start) { // No crossing
                dif++;
                start = match->getRangeStart();
                end = match->getRangeEnd();
            }
            else { // Crossing
                start = qMin(start, (int)match->getRangeStart());
                end = qMax(end, (int)match->getRangeEnd());
            }
        }
        ts << "No crossing matches: " << dif << "\n";
    }
    text.close();
}

/* Save text from edit view to file in CP1251 encoding */
void MainWindow::saveText() {
    qDebug() << "save text slot";
    QString filename = QFileDialog::getSaveFileName(this, QString("Save text file"));
    if (filename == "")
        return;
    QFile text(filename);

    if (!text.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QString("Error"), QString("Cannot open text file ") + filename);
        return;
    }
    getTextFromView();
    text.write(lsplText->getContent().c_str(), lsplText->getContent().length());
    text.close();
}

/* Save text from edit view to file in CP1251 encoding */
void MainWindow::savePatterns() {
    qDebug() << "save patterns slot";
    QString filename = QFileDialog::getSaveFileName(this, QString("Save patterns file"));
    if (filename == "")
        return;
    QFile text(filename);

    if (!text.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QString("Error"), QString("Cannot open text file ") + filename);
        return;
    }
    int pattern_count = lsplNS->getPatternCount();
    for (int i = 0; i < pattern_count; i++) {
        lspl::patterns::PatternRef pattern = lsplNS->getPatternByIndex(i);
        text.write(pattern->getName().c_str(), pattern->getName().length());
        text.write(" = ", 3);
        text.write(pattern->getSource().c_str(), pattern->getSource().length());
        text.write("\n", 1); // Maybe 2 bytes? (Unix vs Windows)... Ohh, these problems...
    }
    text.close();
}

/* Show info about text (Number of words in text) */
void MainWindow::textInfo() {
    getTextFromView();
    int wordNum = 0;
    if (!!lsplText)
        wordNum = lsplText->getWords().size();
    QMessageBox::about(this, QString("Text information"), QString("Number words in text: ") + QString::number(wordNum));
}

/* Destructor for the main window */
MainWindow::~MainWindow() {
    delete ui;
}
