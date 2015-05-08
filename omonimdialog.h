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

class OmonimTableModel : public QAbstractItemModel {
    Q_OBJECT

public:
    enum Columns {
        RamificationColumn,
        NameColumn = RamificationColumn,
        AttribColumn,
        ColumnCount // число столбцов
    };

    class NodeInfo {
    public:
        lspl::text::markup::WordRef word;
        QString wordStr, attrib;
        NodeInfo *parent;
        QVector<NodeInfo> children;
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
    void setDataa( const lspl::text::markup::WordList &wordList );
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    ~OmonimTableModel();
private:
    NodeInfoList::iterator findWord( NodeInfoList& nodeList, NodeInfo &node );
    int findRow(const NodeInfo *nodeInfo) const;
    NodeInfoList rootNodes; // список корневых узлов
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
