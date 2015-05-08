#ifndef MATCHTABLEMODEL_H
#define MATCHTABLEMODEL_H

#include <QString>
#include <QAbstractItemModel>
#include <QTextCodec>

#include <lspl/Namespace.h>
#include <lspl/text/Match.h>
#include <lspl/text/markup/Word.h>
#include <lspl/patterns/Pattern.h>

std::string getAttrString( lspl::text::MatchRef m );

class MatchTableModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit MatchTableModel(QObject *parent = 0);

public:
    enum Columns {
        RamificationColumn,
        NameColumn = RamificationColumn,
        DefinitionColumn,
        MatchColumn,
        AttribColumn,
        ExtractColumn,
        ColumnCount // число столбцов
    };

    class NodeInfo {
    public:
        NodeInfo *parent;
        QVector<NodeInfo *> children;
        int parentColumn;

        NodeInfo( NodeInfo * prnt = 0, int parentCol = RamificationColumn) : parent(prnt), parentColumn(parentCol) {}
        virtual QVariant getView(int col)  const;
        bool operator==(const NodeInfo &ni) const {
            return children == ni.children;
        }
        bool operator==(NodeInfo &ni) const {
            return children == ni.children;
        }
        virtual ~NodeInfo() {}
    };
    class PatternNode : public NodeInfo {
        lspl::patterns::PatternRef pattern;
        QString name, definition;
    public:
        PatternNode(NodeInfo * parent = 0) : NodeInfo(parent){}
        PatternNode(lspl::patterns::PatternRef p, NodeInfo * parent = 0) : NodeInfo(parent), pattern(p) {
            QTextCodec * codec = QTextCodec::codecForName("CP1251");
            name = codec->toUnicode(p->getName().c_str());
            definition = codec->toUnicode(p->getSource().c_str());
        }
        virtual QVariant getView(int col) const;
    };
    class MatchNode : public NodeInfo {
        lspl::text::MatchRef match;
        QString matchstr, attributes;
    public:
        MatchNode(NodeInfo * parent = 0) : NodeInfo(parent) {}
        MatchNode(lspl::text::MatchRef &m, NodeInfo * parent = 0, int parentCol = RamificationColumn) : NodeInfo(parent, parentCol), match(m) {
            QTextCodec * codec = QTextCodec::codecForName("CP1251");
            matchstr = codec->toUnicode(match->getRangeString().c_str());
            attributes = codec->toUnicode(getAttrString(m).c_str());
        }
        virtual QVariant getView(int col) const;
    };
    class VariantNode : public NodeInfo {
        lspl::text::MatchVariantRef variant;
        QString extraction;
    public:
        VariantNode( NodeInfo * parent = 0) : NodeInfo(parent) {}
        VariantNode(lspl::text::MatchVariantRef var, NodeInfo * parent = 0, int parentCol = RamificationColumn ) : NodeInfo(parent, parentCol), variant(var) {
            QTextCodec * codec = QTextCodec::codecForName("CP1251");
            extraction = codec->toUnicode(var->getTransformResult<std::string>().c_str());
        }
        virtual QVariant getView(int col) const;
    };

    typedef QVector<NodeInfo *> NodeInfoList;

    void setNewData( const lspl::NamespaceRef &ns, const lspl::text::TextRef text );
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void clear();
    ~MatchTableModel();
private:
    NodeInfoList::iterator findWord( NodeInfoList& nodeList, NodeInfo &node );
    int findRow(const NodeInfo *nodeInfo) const;
    NodeInfoList rootNodes; // список корневых узлов
};

#endif // MATCHTABLEMODEL_H
