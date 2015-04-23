#ifndef OMONIMDIALOG_H
#define OMONIMDIALOG_H

#include <lspl/text/Match.h>

#include <QDialog>
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
        NodeInfo *parent;
        QVector<NodeInfo> children;
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

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

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
