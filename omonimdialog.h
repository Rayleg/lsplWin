#ifndef OMONIMDIALOG_H
#define OMONIMDIALOG_H

#include <lspl/text/Match.h>
#include <lspl/text/markup/Word.h>

#include <QDialog>
#include <QTextCodec>
#include <QAbstractTableModel>

namespace Ui {
class OmonimDialog;
}

// Omonim table model class */
class OmonimTableModel : public QAbstractItemModel {
    Q_OBJECT

public:
    enum Columns {
        RamificationColumn,
        NameColumn = RamificationColumn,
        AttribColumn,
        ColumnCount // число столбцов
    };

    // Node of tree (cell in table) class
    class NodeInfo {
    public:
        lspl::text::markup::WordRef word; // reference for word
        QString wordStr, attrib;          // word attributes
        NodeInfo *parent;                 // reference to parent node
        QVector<NodeInfo> children;       // children list

        NodeInfo(lspl::text::markup::WordRef w = 0, NodeInfo *p = 0) : word(w), parent(p) {
            QTextCodec *codec = QTextCodec::codecForName("CP1251");

            wordStr = codec->toUnicode(w->getRangeString().c_str());
            attrib = codec->toUnicode((w->getAttributesString() + std::string(",") + word->getSpeechPart().getAbbrevation()).c_str());
        }

        bool operator==(const NodeInfo &ni) const {
            return word == ni.word;
        }
        bool operator==(NodeInfo &ni) const {
            return word == ni.word;
        }

    };
    typedef QVector<NodeInfo> NodeInfoList;

    explicit OmonimTableModel(QObject *parent = 0);
    /* Set data for table function */
    void setNewData( const lspl::text::markup::WordList &wordList );
    /* Get index for cell by coordinations and parent function */
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    /* Get parent for cell function */
    virtual QModelIndex parent(const QModelIndex &child) const;
    /* Get number of rows in table function */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    /* Get number of columns in table function */
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    /* Get data (data for show in cell) for cell by index function */
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    /* Get header name for colums by colums number function */
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    ~OmonimTableModel();
private:
    /* Find word in word list function */
    NodeInfoList::iterator findWord( NodeInfoList& nodeList, NodeInfo &node );
    /* Get row of node from parent function */
    int findRow(const NodeInfo *nodeInfo) const;
    NodeInfoList rootNodes; // root nodes list
};

class OmonimDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OmonimDialog(QWidget *parent = 0);
    void setData( const lspl::text::markup::WordList &wordList );
    ~OmonimDialog();

private:
    Ui::OmonimDialog *ui;
    OmonimTableModel tableModel;
};

#endif // OMONIMDIALOG_H
