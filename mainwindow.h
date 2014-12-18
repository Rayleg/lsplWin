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


namespace Ui {
class MainWindow;
} // End of namespace 'Ui'

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;
    lspl::NamespaceRef lsplNS;
    lspl::patterns::PatternBuilderRef lsplPatternBuilder;
    lspl::text::TextRef lsplText;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void addTemplate();
    void loadTemplate();
    void loadText();
    void applyTemplates();
    void showMatches( int row, int col );
};

#endif // MAINWINDOW_H
