#include "matchtablemodel.h"

#include <lspl/Namespace.h>
#include <lspl/patterns/Pattern.h>
#include<lspl/text/Match.h>
#include <lspl/text/Text.h>
#include <QTextCodec>

#include <iostream>

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

MatchTableModel::MatchTableModel(QObject *parent) :
    QAbstractItemModel(parent) {
}

QModelIndex MatchTableModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (!parent.isValid()) { // запрашивают индексы корневых узлов
        return createIndex(row, column, rootNodes[row]);
    }

    NodeInfo* parentInfo = static_cast<NodeInfo*>(parent.internalPointer());
    return createIndex(row, column, parentInfo->children[row]);
}
QModelIndex MatchTableModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }

    NodeInfo* childInfo = static_cast<NodeInfo*>(child.internalPointer());
    NodeInfo* parentInfo = childInfo->parent;
    if (parentInfo != 0) { // parent запрашивается не у корневого элемента
        //return createIndex(findRow(parentInfo), RamificationColumn, parentInfo);
        return createIndex(findRow(parentInfo), parentInfo->parentColumn, parentInfo);
    }
    else {
        return QModelIndex();
    }

}
int MatchTableModel::findRow(const NodeInfo *nodeInfo) const
{
    const NodeInfoList& parentInfoChildren = nodeInfo->parent != 0 ? nodeInfo->parent->children: rootNodes;
    NodeInfoList::const_iterator position = qFind(parentInfoChildren.begin(), parentInfoChildren.end(), nodeInfo);
    return std::distance(parentInfoChildren.begin(), position);
}
int MatchTableModel::rowCount(const QModelIndex &parent) const {
    //return 15;
    if (!parent.isValid()) {
        return rootNodes.size();
    }
    const NodeInfo* parentInfo = static_cast<const NodeInfo*>(parent.internalPointer());
    return parentInfo->children.size();

}
int MatchTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return ColumnCount;
}

void MatchTableModel::setNewData( const lspl::NamespaceRef &ns, const lspl::text::TextRef text ) {
    // Clear old data
    clear();

    int patternCount = ns->getPatternCount();
    lspl::text::MatchList matches;
    NodeInfo * parentPattern = 0, * parentMatch = 0;

    for (int i = 0; i < patternCount; i++) {
        lspl::patterns::PatternRef pattern = ns->getPatternByIndex(i);
        parentPattern = new PatternNode(pattern, 0);
        rootNodes.push_back(parentPattern);

        // For each match add it to the tree to parent pattern
        matches = text->getMatches(pattern);
        for (int i = 0; i < matches.size(); i++) {
            lspl::text::MatchRef match = matches[i];
            parentMatch = new MatchNode(match, parentPattern, DefinitionColumn);
            parentPattern->children.push_back(parentMatch);
            uint varCount = match->getVariantCount();
            // For each match variant add it to the tree to the parent match
            for (uint varIdx = 0; varIdx < varCount; ++varIdx) {
                if (match->getVariant(varIdx)->alternative.hasTransform())
                    parentMatch->children.push_back(new VariantNode(match->getVariant(varIdx), parentMatch, MatchColumn));
            }
        }
    }
    text->getWords();
    //insertRows(0, rootNodes.size());
    for (int i = 0; i < rootNodes.size(); i++) {
        std::cout << rootNodes[i]->getView(NameColumn).toString().toStdString() << std::endl;
    }
}

QVariant MatchTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    QStringList headers = {"Название", "Определение", "Сопоставление", "Атрибуты", "Извлечение" };
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < headers.size()) {
        return headers[section];
    }
    return QVariant();
}

QVariant MatchTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        const NodeInfo* nodeInfo = static_cast<NodeInfo*>(index.internalPointer());

        return nodeInfo->getView(index.column());
    }
    return QVariant();
}

QVariant MatchTableModel::NodeInfo::getView(int col) const {
    switch (col) {
    case NameColumn:
        return QString("Name");
    case DefinitionColumn:
        return QString("Definition");
    case MatchColumn:
        return QString("Match");
    case AttribColumn:
        return QString("Attributes");
    case ExtractColumn:
        return QString("Extract");
    default:
        break;
    }
    return QVariant();
}

QVariant MatchTableModel::PatternNode::getView(int col)  const {
    switch (col) {
    case NameColumn:
        return name;
        //return QString::fromStdString(pattern->getName());
    case DefinitionColumn:
        return definition;
        //return QString::fromStdString(pattern->getSource());
    default:
        break;
    }
    return QVariant();
}
QVariant MatchTableModel::MatchNode::getView(int col)  const {
    //QTextCodec *codec = QTextCodec::codecForName("CP1251");

    switch (col) {
    case MatchColumn:
        return matchstr;
        //return codec->toUnicode(match->getRangeString().c_str());
    case AttribColumn:
        return attributes;
        //return codec->toUnicode(getAttrString(match).c_str());
    default:
        break;
    }
    return QVariant();
}
QVariant MatchTableModel::VariantNode::getView(int col)  const {
    //QTextCodec *codec = QTextCodec::codecForName("CP1251");

    switch (col) {
    case ExtractColumn:
        return extraction;
        //return codec->toUnicode(variant->getTransformResult<std::string>().c_str());
    default:
        break;
    }
    return QVariant();
}

void MatchTableModel::clear() {
    for (int i = 0; i < rootNodes.size(); i++) {
        for (int j = 0; j < rootNodes[i]->children.size(); j++) {
            for (int k = 0; k < rootNodes[i]->children[j]->children.size(); k++)
                delete rootNodes[i]->children[j]->children[k];
            delete rootNodes[i]->children[j];
        }
        delete rootNodes[i];
    }
    rootNodes.clear();
}

MatchTableModel::~MatchTableModel() {
    clear();
}
