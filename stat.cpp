/* Programmed by VK24 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

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

#include <boost/ptr_container/ptr_map.hpp>

/* Comfortable name reducing */
using std::cout;
using std::cin;
using std::endl;

using lspl::uint64;
using lspl::text::MatchList;
using lspl::patterns::Alternative;
using lspl::patterns::matchers::Matcher;
using lspl::patterns::matchers::Variable;
using lspl::patterns::PatternRef;
using lspl::text::attributes::AttributeKey;
using lspl::text::attributes::AttributeValue;
using lspl::text::markup::Word;
using lspl::text::TextRef;



class Statistic {
    MatchList m_matches; // список сопоставлений для сбора статистики

public:
    typedef std::map<AttributeKey, AttributeValue> AttributesMap;
    typedef std::multimap<AttributeKey, AttributeValue> AttributesMultimap;
    typedef std::map<std::string, std::vector<std::pair<int, lspl::uint64> > > ConfigureSet;

    Statistic() {}
    //Statistic( MatchList match ) : m_matches(match) {}
    Statistic( const PatternRef & pr, const TextRef & tr ) {
        m_matches = tr->getMatches(pr);
    }
    void Attach( const PatternRef & pr, const TextRef & tr ) {
        m_matches = tr->getMatches(pr);
    }
    //void Attach( MatchList m )
    //{
    //  m_matches = m;
    //}
    // печать всех атрибутов шаблона (параметров)
    void printAttr() {
        for (MatchList::iterator i = m_matches.begin(); i != m_matches.end(); ++i) {
            cout << "match: " << (*i)->getRangeString() << endl;
            cout << "Case attr: " <<
                (*i)->getAttribute(AttributeKey::CASE).getString() << " " << endl;
            cout << "Number attr: " <<
                (*i)->getAttribute(AttributeKey::NUMBER).getString() << " " << endl;
            cout << "Gender attr: " <<
                (*i)->getAttribute(AttributeKey::GENDER).getString() << " " << endl;
            cout << "Doc attr: " <<
                (*i)->getAttribute(AttributeKey::DOC).getString() << " " << endl;
            cout << "Tense attr: " <<
                (*i)->getAttribute(AttributeKey::TENSE).getString() << " " << endl;
            cout << "Animate attr: " <<
                (*i)->getAttribute(AttributeKey::ANIMATE).getString() << " " << endl;
            cout << "Form attr: " <<
                (*i)->getAttribute(AttributeKey::FORM).getString() << " " << endl;
            cout << "Mode attr: " <<
                (*i)->getAttribute(AttributeKey::MODE).getString() << " " << endl;
            cout << "Person attr: " <<
                (*i)->getAttribute(AttributeKey::PERSON).getString() << " " << endl;
            cout << "Reflexive attr: " <<
                (*i)->getAttribute(AttributeKey::REFLEXIVE).getString() << " " << endl;
            cout << "Stem attr: " <<
                (*i)->getAttribute(AttributeKey::STEM).getString() << " " << endl;
            cout << "Base attr: " <<
                (*i)->getAttribute(AttributeKey::BASE).getString() << " " << endl;
            cout << "Text attr: " <<
                (*i)->getAttribute(AttributeKey::TEXT).getString() << " " << endl;
        }
    }
    // получение списка наложений для шаблона, где параметры соответствуют аргументу
    MatchList getStatWith( const AttributesMap & attr_map ) {
        MatchList res;
        bool add;

        // For each match
        for (MatchList::iterator match_iter = m_matches.begin();
             match_iter != m_matches.end();
             ++match_iter) {
            add = true;
            // For each attribute in argument
            for (AttributesMap::const_iterator i = attr_map.begin();
                 i != attr_map.end();
                 ++i) {
                AttributeValue av = (*match_iter)->getAttribute(i->first);
                if (av != i->second && av != AttributeValue::UNDEFINED) {
                    add = false;
                    break;
                }
            }
            // If all attributes are similar then add match to result
            if (add)
                res.push_back(*match_iter);
        }
        return res;
    }
    // получение списка наложений шаблона, где параметры шаблона удовлетворяют аргументу функции
    MatchList getStatWith( const AttributesMultimap & attr_map ) {
        MatchList res;
        bool add;

        // For each match
        for (MatchList::iterator match_iter = m_matches.begin();
             match_iter != m_matches.end();
             ++match_iter) {
            add = true;
            // For each attribute in argument
            for (AttributesMultimap::const_iterator i = attr_map.begin();
                 i != attr_map.end();) {
                AttributeValue av = (*match_iter)->getAttribute(i->first);
                bool same = false;
                for (int a_count = 0; a_count < attr_map.count(i->first); ++a_count, ++i) {
                    if (av == i->second)
                        same = true;
                }
                if (!same)
                    add = false;
            }
            // If all attributes are similar then add match to result
            if (add)
                res.push_back(*match_iter);
        }
        return res;
    }
    // получение множества начальных форм слов, которые при наложении
    // соответствуют элементу-слову с индексом index, а также для каждой такой формы множество
    // форм (заданых набором атрибутов) с указанием числа встречаемости
    ConfigureSet getStatById( int index )
    {
        if (m_matches.empty())
            return ConfigureSet();
        ConfigureSet res;
        Variable var;
        auto res_it = res.begin();

        lspl::transforms::ContextRetriever cr;
        const boost::ptr_vector<Matcher> & bmv =
            m_matches[0]->getPattern().getAlternatives()[0].getMatchers();
        for (int i = 0; i < bmv.size(); ++i)
            if (bmv[i].variable.index == index) {
                var = bmv[i].variable;
                break;
            }
        // не нашлось переменной - уточнить
        if (var.index == 0 && var.type == 0)
            return ConfigureSet();
        // Если все нормально, строим множество
        for (auto it = m_matches.begin(); it != m_matches.end(); ++it) {
            lspl::patterns::matchers::Context c = cr.apply(*(*it)->getVariant(0));

            lspl::transforms::TextTransform::ConstRange val = c.getValues(var);

            for (auto val_it = val.first; val_it != val.second; ++val_it) {
                const Word * word =  dynamic_cast<const Word *>(&*val_it->second);
                // формируем результирующее множество
                if ((res_it = res.find(word->getBase())) == res.end())
                    res[word->getBase()].push_back(std::pair<int, uint64>(1, word->getAttributes()));
                else {
                    uint64 wattr = word->getAttributes();
                    auto vec_it = res_it->second.begin();
                    for (; vec_it != res_it->second.end(); ++vec_it)
                        if (vec_it->second == wattr) {
                            ++(vec_it->first);
                            break;
                        }
                        if (vec_it == res_it->second.end()) // Такой конфигурации еще нет
                            res_it->second.push_back(std::pair<int, uint64>(1, wattr));
                }
            }
        }
        return res;
    }
    static std::string getAttributesString( uint64 attr ){
        return lspl::morphology::Morphology::instance().getAttributesString(attr);
    }
    static void printConfigureSet( const ConfigureSet & cs ){
        for (auto map_it = cs.begin(); map_it != cs.end(); ++map_it) {
            cout << "Base: " << map_it->first << endl;
            for (auto set_it = map_it->second.begin(); set_it != map_it->second.end(); ++set_it)
                cout << "\t" << set_it->first << " items:" <<
                    getAttributesString(set_it->second) << endl;
        }
    }
};


/* The main program function */
int main2( int argc, char **argv )
{
    //return 0;
    //QLibrary lsplLib("lspl");

    //lsplLib.setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint | QLibrary::LoadArchiveMemberHint);
    //lsplLib.loadHints();

//    if (lsplLib.load())
//        cout << "Library load" << endl;
//    else
//        cout << "Library is not load" << endl;
    cout << "Start  program" << endl;

    // Create namespace for everything
    lspl::NamespaceRef ns = new lspl::Namespace(); // SIGSEGV received here
    // Define builder in namespace ns
    lspl::patterns::PatternBuilderRef builder = new lspl::patterns::PatternBuilder(ns);

    const char * fname;
    if (argc > 1)
        fname = argv[1];
    else
        fname = "text.txt";
    std::ifstream is(fname);

    if (is.bad())
        cout << "Ooops! Cannot open file" << endl;
    else {
        cout << is.is_open() << endl;
        cout << is.get() << endl;
        lspl::text::readers::PlainTextReader reader;
        lspl::text::TextRef text = reader.readFromStream(is);

        cout << "Content: " << text->getContent() << endl;

        std::ifstream pattern_file;
        if (argc > 2)
            pattern_file.open(argv[2]);
        else
            pattern_file.open("pattern.txt");
        if (!pattern_file.bad() && pattern_file.is_open()) {
            std::string pname, pdef;
            Statistic s;

            getline(pattern_file, pname, ':');
            getline(pattern_file, pdef);
            cout << pname << " delim " << pdef << endl;

            builder->build(pdef.c_str());
            //s.Attach(text->getMatches(ns->getPatternByName(pname.c_str())));
            s.Attach(ns->getPatternByName(pname.c_str()), text);
            s.printAttr();

            std::map<AttributeKey, AttributeValue> a_map;

            a_map[AttributeKey::CASE] = AttributeValue::NOMINATIVE;
            MatchList m = s.getStatWith(a_map);
            for (MatchList::iterator i = m.begin(); i != m.end(); ++i)
                cout << "Match with: " << (*i)->getRangeString() << endl;
       
            std::multimap<AttributeKey, AttributeValue> a_multimap;

            typedef std::multimap<AttributeKey, AttributeValue>::value_type ValType;
            a_multimap.insert(ValType(AttributeKey::CASE, AttributeValue::ACCUSATIVE));
            a_multimap.insert(ValType(AttributeKey::CASE, AttributeValue::NOMINATIVE));
            m = s.getStatWith(a_multimap);
//            AttributeKey::

            for (MatchList::iterator i = m.begin(); i != m.end(); ++i)
                cout << "Match with (multi): " << (*i)->getRangeString() << endl;

            const boost::ptr_vector<Alternative> & bav =
                m[0]->getPattern().getAlternatives();
            const boost::ptr_vector<Matcher> & bmv = bav[0].getMatchers();

            cout << "Matcher count:" << bmv.size() << endl;
            for (int i = 0; i < bmv.size(); ++i) {
                bmv[i].dump(cout);
                cout << endl;
            }
            Statistic::printConfigureSet(s.getStatById(1));

            for (int i = 0; i < m.size(); ++i) {
                cout << m[i]->getVariantCount() << endl;
                const lspl::text::MatchVariantRef & mr = m[i]->getVariants()[0];

                lspl::transforms::ContextRetriever cr;
                lspl::patterns::matchers::Context c = cr.apply(*mr);

                Variable var(lspl::text::attributes::SpeechPart::ADJECTIVE, 1);
                lspl::transforms::TextTransform::ConstRange val = c.getValues(var);

                for (auto it = val.first; it != val.second; ++it) {
                    const Word * word =  dynamic_cast<const Word *>(&*it->second);
                    cout << "Var word:" << word->getToken() << endl;
                }

                try {
                    cout << "Succ:" << mr->getTransformResult<std::string>() << endl;
                } catch ( const lspl::base::Exception & ex ) {
                    cout << "Except:" << ex.what() << endl;
                }
            }
            const lspl::text::markup::WordList words = text->getWords();
            for (int i = 0; i < words.size(); i++) {
                lspl::text::markup::WordRef word = words[i];
                cout << word->getAttributesString() << endl;
                word->getRangeString();
            }
            pattern_file.close();
        }
        is.close();
    }
    //Debug print
    cout << "Finish  program" << std::endl;
  
    return 0;
}

