#include "omonimdialog.h"
#include "ui_omonimdialog.h"

#include <lspl/text/markup/Word.h>

#include <QTextCodec>
#include <QtAlgorithms>

OmonimDialog::OmonimDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OmonimDialog)
{
    ui->setupUi(this);
    // Set model for dialog
    ui->treeView->setModel(&tableModel);
}
/* Set data for show function */
void OmonimDialog::setData( const lspl::text::markup::WordList &wordList ) {
    tableModel.setNewData(wordList);
}

OmonimTableModel::OmonimTableModel(QObject *parent) : QAbstractItemModel(parent) {
}
OmonimTableModel::~OmonimTableModel(){
}
/* Find word in word list function */
OmonimTableModel::NodeInfoList::iterator OmonimTableModel::findWord( NodeInfoList& nodeList, NodeInfo &node ) {
    NodeInfoList::iterator iter = nodeList.begin();

    while (iter != nodeList.end()) {
        if (iter->word->getRangeString() == node.word->getRangeString())
            return iter;
        iter++;
    }
    return iter;
}

/* Set data for table function */
void OmonimTableModel::setNewData( const lspl::text::markup::WordList &wordList ) {
    // Build word tree
    for (int i = 0; i < (int)wordList.size(); i++) {
        NodeInfo newWord(wordList[i]);

        NodeInfoList::iterator position = findWord(rootNodes, newWord);
        if (position == rootNodes.end()) {
            newWord.parent = 0;
            rootNodes.push_back(newWord);
            rootNodes.last().children.push_back(newWord);
        }
        else {
            NodeInfoList::const_iterator pos = const_cast<NodeInfoList::iterator>(qFind(position->children, newWord));
            if (pos == position->children.end()) {
                newWord.parent = const_cast<NodeInfo *>(position);
                position->children.push_back(newWord);
            }
        }
    }
    // Remove similar words
    QVector<int> removeList;
    for (int i = 0; i < rootNodes.size(); i++) {
        if (rootNodes[i].children.size() <= 1)
            removeList.push_front(i);
    }
    for (int i = 0; i < removeList.size(); i++)
        rootNodes.removeAt(removeList[i]);

    QTextCodec *codec = QTextCodec::codecForName("CP1251");
    // Print debug tree
    for (int i = 0; i < rootNodes.size(); i++) {
        std::cout << codec->toUnicode(rootNodes[i].word->getRangeString().c_str()).toStdString() << &rootNodes[i] << std::endl;
        for (int j = 0; j < rootNodes[i].children.size(); j++) {
            rootNodes[i].children[j].parent = &rootNodes[i];
            std::cout << "    " << codec->toUnicode(rootNodes[i].children[j].word->getRangeString().c_str()).toStdString() <<
                         rootNodes[i].children[j].parent << std::endl;
        }
    }

}
/* Get index for cell by coordinations and parent function */
QModelIndex OmonimTableModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (!parent.isValid()) { // запрашивают индексы корневых узлов
        return createIndex(row, column, const_cast<NodeInfo*>(&rootNodes[row]));
    }

    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
    return createIndex(row, column, &parentInfo->children[row]);
}
/* Get parent for cell function */
QModelIndex OmonimTableModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }

    NodeInfo* childInfo = static_cast<NodeInfo*>(child.internalPointer());
    NodeInfo* parentInfo = childInfo->parent;
    if (parentInfo != 0) { // parent запрашивается не у корневого элемента
        return createIndex(findRow(parentInfo), RamificationColumn, parentInfo);
    }
    else {
        return QModelIndex();
    }

}
/* Get row of node from parent function */
int OmonimTableModel::findRow(const NodeInfo *nodeInfo) const
{
    const NodeInfoList& parentInfoChildren = nodeInfo->parent != 0 ? nodeInfo->parent->children: rootNodes;
    NodeInfoList::const_iterator position = qFind(parentInfoChildren.begin(), parentInfoChildren.end(), *nodeInfo);
    return std::distance(parentInfoChildren.begin(), position);
}
/* Get number of rows in table function */
int OmonimTableModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return rootNodes.size();
    }
    const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
    return parentInfo->children.size();

}
/* Get number of columns in table function */
int OmonimTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return ColumnCount;
}
/* Get data (data for show in cell) for cell by index function */
QVariant OmonimTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        const NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());

        switch (index.column()) {
        case NameColumn:
            return nodeInfo->wordStr;
        case AttribColumn:
            if (nodeInfo->parent == 0)
                return QVariant();
            return nodeInfo->attrib;
        default:
            break;
        }
    }
    return QVariant();
}

/* Get header name for colums by colums number function */
QVariant OmonimTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    QStringList headers = {"Слово", "Атрибуты"};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < headers.size()) {
        return headers[section];
    }
    return QVariant();
}

OmonimDialog::~OmonimDialog() {
    delete ui;
}
