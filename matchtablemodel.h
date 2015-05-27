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

/* Match table model class */
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

    // Node of tree table class
    class NodeInfo {
    public:
        NodeInfo *parent;             // reference to parent foe cell
        QVector<NodeInfo *> children; // children list
        int parentColumn;             // Number of parent column

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
    // Types of cells
    // Node for pattern describing
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
    // Node for match describing
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
    // Node for variant (transform of patterns) describing
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


    /* Set data for table */
    void setNewData( const lspl::NamespaceRef &ns, const lspl::text::TextRef text );
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
    /* Clear memory of data tree */
    void clear();
    ~MatchTableModel();
private:
    /* Get row of node from parent function */
    int findRow(const NodeInfo *nodeInfo) const;
    NodeInfoList rootNodes; // root nodes list
};

#endif // MATCHTABLEMODEL_H
