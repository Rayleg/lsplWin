#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QTextStream>

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

#include "matchtablemodel.h"

namespace Ui {
class MainWindow;
} // End of namespace 'Ui'

/* The main window representation class */
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui; // user interface
    lspl::NamespaceRef lsplNS; // storage for patterns
    lspl::patterns::PatternBuilderRef lsplPatternBuilderTransform; // pattern builder
    lspl::text::TextRef lsplText; // lspl text representation
    MatchTableModel matchTableModel; // model for match table

    // Add pattern by text definition to system function */
    void addPattern( const QString &pattern );
    /* Get text from edit view and load it to inner representation */
    void getTextFromView();
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    /* Add pattern from interface form function */
    void addTemplate();
    /* Load patterns from file function */
    void loadTemplate();
    /* Load text file function (on 'load text' menu item slot) */
    void loadText();
    /* Build table of matches for registered patterns and show it (on 'tableMatch' buuton slot) */
    void applyTemplates();
    /* Create dialog with omonims list function */
    void showOmonims();
    /* Save statistic about pattern matching */
    void saveData();
    /* Save text from edit view to file in CP1251 encoding */
    void savePatterns();
    /* Save text from edit view to file in CP1251 encoding */
    void saveText();
    /* Show info about text (Number of words in text) */
    void textInfo();
};

#endif // MAINWINDOW_H
