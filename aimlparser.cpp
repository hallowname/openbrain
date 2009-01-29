#include "aimlparser.h"

#include <cstdlib>

#include <QFile>
#include <QDir>
#include <QtXml>

bool exactMatch(QString regExp,
		QString str, 
		QStringList &capturedText)
{
    QStringList regExpWords = regExp.split(' ');
    QStringList strWords = str.split(' ');

    if ((!regExpWords.count() || !strWords.count()) && (regExpWords.count() != strWords.count()))
        return false;
    if (regExpWords.count() > strWords.count())
        return false;
    QStringList::ConstIterator regExpIt = regExpWords.begin();
    QStringList::ConstIterator strIt = strWords.begin();
    while ((strIt != strWords.end()) && (regExpIt != regExpWords.end()))
    {
        if ( (*regExpIt == "*") || (*regExpIt == "_") )
        {
            regExpIt++;
            QStringList capturedStr;
            if (regExpIt != regExpWords.end())
            {
                QString nextWord = *regExpIt;
                if ( (nextWord != "*") && (nextWord != "_") )
                {
                    while (true)
                    {
                        if (*strIt == nextWord)
                            break;
                        capturedStr += *strIt;
                        if (++strIt == strWords.end())
                            return false;
                    }
                }
                else
                {
                    capturedStr += *strIt;
                    regExpIt --;
                }
            }
            else
            {
                while (true)
                {
                    capturedStr += *strIt;
                    if (++strIt == strWords.end())
                        break;
                }
                capturedText += capturedStr.join(" ");
                return true;
            }
            capturedText += capturedStr.join(" ");
        }
        else if (*regExpIt != *strIt)
            return false;
        regExpIt++;
        strIt++;
    }
    return (regExpIt == regExpWords.end()) && (strIt == strWords.end());
}

QList<QDomNode> elementsByTagName(QDomNode *node, const QString& tagName)
{
    QList<QDomNode> list;
    QDomNodeList childNodes = node->toElement().elementsByTagName(tagName);
    for (uint i = 0; i < childNodes.count(); i++)
    {
        QDomNode n = childNodes.item(i);
        if (n.parentNode() == *node)
	        list.append(n);
    }
    return list;
}

Leaf::Leaf()
{
    topic = that = "";
}

Node::Node()
{
    word = "";
}

bool Node::match(QStringList::const_iterator input, const QStringList &inputWords,
    const QString &currentThat, const QString &currentTopic, QStringList &capturedThatTexts,
    QStringList &capturedTopicTexts, Leaf *&leaf)
{
    if (input == inputWords.end())
       return false;
      
    if ((word == "*") || (word == "_"))
    {
	    ++input;
	    for (;input != inputWords.end(); input++)
	    {
		Node *child;
		for(int i = 0; i < childs.count() ; i++)
		{
		    child = childs[i];
		    if (child->match(input, inputWords, currentThat, currentTopic, capturedThatTexts, capturedTopicTexts, leaf))
		    {
			return true;
		    }
		}
	    }
    }
    else
    {
	    if (!word.isEmpty())
	    {
		   if (word != *input)        
	          return false;
	       ++input;
	    }
	    Node *child;
	    for (int i = 0; i < childs.count() ; i++)
	    {
		child = childs[i];
		if (child->match(input, inputWords, currentThat, currentTopic, capturedThatTexts, capturedTopicTexts, leaf))
		{
		    return true;		  
		}
	    }
    }
    if (input == inputWords.end())
    {
	for (int i = 0; i < leafs.count(); i++)
	{
	      leaf = leafs[i];
	      capturedThatTexts.clear();
	      capturedTopicTexts.clear();
	      if ( (!leaf->that.isEmpty() && !exactMatch(leaf->that, currentThat, capturedThatTexts)) || (!leaf->topic.isEmpty() && !exactMatch(leaf->topic, currentTopic, capturedTopicTexts)) )
	      {
		  continue;
	      }
	      return true;
	}
    }

    return false;
}

void Node::debug(QTextStream* logStream, uint indent)
{
    QString indentStr = QString().fill('\t', indent);
    Node *child = childs.first();
    for (int i = 0; i < childs.count() ; i++)
    {
	child = childs[i];
	child->debug(logStream, indent + 1);
    }
    
    indentStr = QString().fill('\t', indent + 1);
}

void AIMLParser::runRegression()
{
    QDomDocument doc;
    QFile file( QDir::homePath() + "/.openbrain/utils/TestSuite.xml" );
    if ( !file.open( QIODevice::ReadOnly ) )
        return;
    if ( !doc.setContent( &file ) )
    {
        file.close();
        return;
    }
    file.close();

    loadAiml("utils/TestSuite.aiml");

    QDomElement docElem = doc.documentElement();
    QDomNodeList testCaseList = docElem.elementsByTagName ("TestCase");
    for (int i = 0; i < testCaseList.count(); i++)
    {
        QDomElement n = testCaseList.item(i).toElement();
        QString description = n.namedItem("Description").firstChild().nodeValue();
        QString input = n.namedItem("Input").firstChild().nodeValue();

        QString expectedAnswer = "";
        QDomNode child = n.namedItem("ExpectedAnswer").firstChild();
        while (!child.isNull())
        {
            if (child.isText())
               expectedAnswer += child.toText().nodeValue();
            child = child.nextSibling();
        }
        QString answer = getResponse(input);
    }
}

void AIMLParser::displayTree()
{
    root.debug(logStream);
}

void AIMLParser::normalizeString(QString &str)
{
    QString newStr;
    for (int i = 0; i < str.length(); i++)
    {
        QChar c = str.at(i);
        if (c.isLetterOrNumber() || (c == '*') || (c == '_') || (c == ' '))
            newStr += c.toLower();
    }
    str = newStr;
}

AIMLParser::AIMLParser(QTextStream* logStream): logStream(logStream)
{
    indent = 0;
    root.parent = NULL;
    QTime currentTime = QTime::currentTime();
    int val = currentTime.msec() + currentTime.second() + currentTime.minute();
    srand(val);
}

AIMLParser::~AIMLParser()
{}

bool AIMLParser::loadSubstitutions(const QString &filename)
{
    QDomDocument doc;
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly ) )
        return false;
    if ( !doc.setContent( &file ) )
    {
        file.close();
        return false;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    QDomNodeList subsList = docElem.elementsByTagName ("substitution");
    for (int i = 0; i < subsList.count(); i++)
    {
        QDomElement n = subsList.item(i).toElement();
        subOld.append(QRegExp(n.namedItem("old").firstChild().nodeValue()));
        subNew.append(n.namedItem("new").firstChild().nodeValue());
    }
    return true;
}

bool AIMLParser::loadVars(const QString &filename, const bool &bot)
{
    QDomDocument doc;
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly ) )
        return false;
    if ( !doc.setContent( &file ) )
    {
        file.close();
        return false;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    QDomNodeList setList = docElem.elementsByTagName ("set");
    for (int i = 0; i < setList.count(); i++)
    {
        QDomElement n = setList.item(i).toElement();
        QString name = n.attribute("name");
        QString value = n.firstChild().nodeValue();
        if (bot)
            botVarValue[name] = value;
        else
            parameterValue[name] = value;
    }
    return true;
}

bool AIMLParser::saveVars(const QString &filename)
{
    QDomDocument doc;
    QDomElement root = doc.createElement("vars");
    doc.appendChild(root);

    QMap<QString, QString>::ConstIterator it;
    for ( it = parameterValue.begin(); it != parameterValue.end(); ++it )
    {
        QDomElement setElem = doc.createElement("set");
        setElem.setAttribute("name", it.key());
        QDomText text = doc.createTextNode(it.value());
        setElem.appendChild(text);
        root.appendChild(setElem);
    }

    QFile fileBackup( filename + ".bak" );
    if ( !fileBackup.open( QIODevice::WriteOnly ) )
        return false;
    QTextStream tsBackup(&fileBackup);
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly ) )
        return false;
    tsBackup << QString(file.readAll());
    fileBackup.close();
    file.close();

    if ( !file.open( QIODevice::WriteOnly ) )
        return false;
    QTextStream ts(&file);
    ts << doc.toString();
    file.close();
    return true;
}

bool AIMLParser::loadAiml(const QString &filename)
{
    QDomDocument doc( "mydocument" );
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly ) )
        return false;

    QXmlInputSource src(&file);
    QXmlSimpleReader reader;
    reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", true);

    QString msg;
    int line, col;
    if ( !doc.setContent( &src, &reader, &msg, &line, &col ) )
    {
        file.close();
        return false;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    QDomNodeList categoryList = docElem.elementsByTagName ("category");
    for (int i = 0; i < categoryList.count(); i++)
    {
        QDomNode n = categoryList.item(i);
        parseCategory(&n);
    }
    return true;
}

void AIMLParser::parseCategory(QDomNode* categoryNode)
{
    QDomNode patternNode = categoryNode->namedItem("pattern");
    QString pattern = resolveNode(&patternNode);
    normalizeString(pattern);
    
    QStringList words = pattern.split(' ');
    Node *whereToInsert = &root;

    for ( QStringList::ConstIterator it = words.begin(); it != words.end(); ++it )
    {
        bool found = false;
	Node *child;
	for (int i = 0; i < whereToInsert->childs.count() ; i++)
	{
	    child = whereToInsert->childs[i];
	    if (child->word == *it)
            {
		whereToInsert = child;
		found = true;
		break;
            }
	}

	if (!found)
        {
            for (; it != words.end(); ++it )
            {
                Node *n = new Node;
                n->word = *it;
                n->parent = whereToInsert;
		        int index = 0;
                if (*it == "*")
                   index = whereToInsert->childs.count();
                else if ((*it != "_") && whereToInsert->childs.count() &&
                   (whereToInsert->childs.at(0)->word == "_"))
		           index = 1;
		        whereToInsert->childs.insert(index, n);
                whereToInsert = n;
            }
            break;
        }
    }

    Leaf *leaf = new Leaf;
    leaf->parent = whereToInsert;
    QDomNode thatNode = categoryNode->namedItem("that");

    if (!thatNode.isNull())
    {
        leaf->that = thatNode.firstChild().toText().nodeValue();
        normalizeString(leaf->that);
    }

    leaf->tmplate = categoryNode->namedItem("template");
    QDomNode parentNode = categoryNode->parentNode();

    if (!parentNode.isNull() && (parentNode.nodeName() == "topic"))
    {
        leaf->topic = parentNode.toElement().attribute("name");
        normalizeString(leaf->topic);
    }

    int index = 0;
    int leafWeight = !leaf->that.isEmpty() + !leaf->topic.isEmpty() * 2;

    Leaf* childLeaf;
    for (int i = 0; i < whereToInsert->leafs.count(); i++)
    {
	childLeaf = whereToInsert->leafs[i];
	int childLeafWeight = !childLeaf->that.isEmpty() + !childLeaf->topic.isEmpty() * 2;

	if (leafWeight >= childLeafWeight)
           break;

	index++;
    }

    whereToInsert->leafs.insert(index, leaf);
}

QString AIMLParser::resolveNode(QDomNode* node, const QStringList &capturedTexts,
    const QStringList &capturedThatTexts, const QStringList &capturedTopicTexts)
{
    QString result("");
    QString nodeName = node->nodeName();
    QDomElement element = node->toElement();

    if (nodeName == "random")
    {
        QList<QDomNode> childNodes = elementsByTagName(node, "li");
        uint childCount = childNodes.count();
        uint random = rand() % childCount;
        QDomNode child = childNodes[random];
        result = resolveNode(&child, capturedTexts, capturedThatTexts, capturedTopicTexts);
    }
    else if (nodeName == "condition")
    {
        QString name("");
        uint condType = 2;

	if (element.hasAttribute("name"))
        {
            condType = 1;
            name = element.attribute("name");

	    if (element.hasAttribute("value"))
            {
                condType = 0;
                QString value = element.attribute("value").toUpper();
                QStringList dummy;

		if (exactMatch(value, parameterValue[name].toUpper(), dummy))
                {
                    element.setTagName("parsedCondition");
                    result = resolveNode(&element, capturedTexts, capturedThatTexts, capturedTopicTexts);
                    element.setTagName("condition");
                }
            }
        }
        if (condType)
        {
            QList<QDomNode> childNodes = elementsByTagName(node, "li");
            for (uint i = 0; i < childNodes.count(); i++)
            {
                QDomNode n = childNodes[i];

		if (n.toElement().hasAttribute("value"))
                {
                    if (condType == 2)
                        name = n.toElement().attribute("name");

		    QString value = n.toElement().attribute("value").toUpper();
                    QStringList dummy;

		    if (exactMatch(value, parameterValue[name].toUpper(), dummy))
                    {
                        result = resolveNode(&n, capturedTexts, capturedThatTexts, capturedTopicTexts);
                        break;
                    }
                }
                else
                {
                    result = resolveNode(&n, capturedTexts, capturedThatTexts, capturedTopicTexts);
                    break;
                }
            }
        }
    }
    else
    {
        QDomNode n = node->firstChild();
        while (!n.isNull())
        {
            result += resolveNode(&n, capturedTexts, capturedThatTexts, capturedTopicTexts);
            n = n.nextSibling();
        }
        if (node->isText())
            result = node->toText().nodeValue();
        else if (nodeName == "set")
            parameterValue[element.attribute("name")] = result.trimmed();
        else if (nodeName == "srai")
            result = getResponse(result, true);
        else if (nodeName == "think")
            result = "";
        else if (nodeName == "system")
            result = executeCommand(result);
        else if (nodeName == "learn")
        {
            loadAiml(result);
            result = "";
        }
        else if (nodeName == "uppercase")
        {
            result = result.toUpper();
        }
        else if (nodeName == "lowercase")
        {
            result = result.toLower();
        }
        else if (!node->hasChildNodes())
        {
            if (nodeName == "star")
            {
                uint index = element.attribute("index", "1").toUInt() - 1;
                result = index < capturedTexts.count() ? capturedTexts[index] : QString("");
            }
            else if (nodeName == "thatstar")
            {
                uint index = element.attribute("index", "1").toUInt() - 1;
                result = index < capturedThatTexts.count() ? capturedThatTexts[index] : QString("");
            }
            else if (nodeName == "topicstar")
            {
                uint index = element.attribute("index", "1").toUInt() - 1;
                result = index < capturedTopicTexts.count() ? capturedTopicTexts[index] : QString("");
            }
            else if (nodeName == "that")
            {
                QString indexStr = element.attribute("index", "1,1");
                if (!indexStr.contains(","))
                   indexStr = "1," + indexStr;
                uint index1 = indexStr.section(',', 0, 0).toUInt() - 1;
                uint index2 = indexStr.section(',', 1, 1).toUInt() - 1;
                result = (index1 < thatList.count()) && (index2 < thatList[index1].count()) ?
                   thatList[index1][index2] : QString("");
            }
            else if (nodeName == "sr")
                result = getResponse(capturedTexts.count() ? capturedTexts[0] : QString(""), true);
            else if ( (nodeName == "br") || (nodeName == "html:br") )
                result = "<br>";
	    else if ( nodeName == "get" )
                result = parameterValue[element.attribute("name")];
            else if ( nodeName == "bot")
                result = botVarValue[element.attribute("name")];
            else if ( (nodeName == "person") || (nodeName == "person2") || (nodeName == "gender") )
                result = capturedTexts.count() ? capturedTexts[0] : QString("");
            else if (nodeName == "input")
            {
                uint index = element.attribute("index", "1").toUInt() - 1;
                result = index < inputList.count() ? inputList[index] : QString("");
            }
        }
    }
    return result;
}

QString AIMLParser::getResponse(QString input, const bool &srai)
{

    if (srai)
        indent ++;
    QString indentSpace = QString().fill(' ', 2*indent);

    QList<QRegExp>::Iterator itOld = subOld.begin();
    QStringList::Iterator itNew = subNew.begin();
    for (; itOld != subOld.end(); ++itOld, ++itNew )
        input.replace(*itOld, *itNew);
    if (!srai)
    {
        inputList.prepend(input);
        if (inputList.count() > MAX_LIST_LENGTH)
            inputList.pop_back();
    }

    QStringList capturedTexts, capturedThatTexts, capturedTopicTexts;
    QString curTopic = parameterValue["topic"];
    normalizeString(curTopic);
    Leaf *leaf = NULL;
    QString result("");
    QStringList sentences = input.split(QRegExp("[\\.\\?!;]"));
    QStringList::Iterator sentence = sentences.begin();
    while (true)
    {
        //normalizeString(*sentence);
        *sentence = (*sentence).toLower();
        QStringList inputWords = (*sentence).split(' ');
        QStringList::ConstIterator it = inputWords.begin();

        if (!root.match(it, inputWords, thatList.count() && thatList[0].count() ?
            thatList[0][0] : QString(""), curTopic, capturedThatTexts, capturedTopicTexts, leaf))
            return "Internal Error!";
        Node *parentNode = leaf->parent;
        QString matchedPattern = parentNode->word;
        while (parentNode->parent->parent)
        {
            parentNode = parentNode->parent;
            matchedPattern = parentNode->word + " " + matchedPattern;
        }
        if (!leaf->that.isEmpty())
        if (!leaf->topic.isEmpty())
        capturedTexts.clear();
        exactMatch(matchedPattern, *sentence, capturedTexts);

        if (visitedNodeList.contains(&leaf->tmplate))
        	result += "OpenBrain: Infinite loop detected!";
        else
        {
            visitedNodeList.append(&leaf->tmplate);
            result += resolveNode(&leaf->tmplate, capturedTexts, capturedThatTexts, capturedTopicTexts).trimmed();
        }
        sentence++;
        if (sentence != sentences.end())
            result += " ";
        else
            break;
    }
    if (!srai)
    {
        QString tempResult = result.simplified();
        //get the sentences of the result splitted by: . ? ! ; and "arabic ?"
        QStringList thatSentencesList = tempResult.split(QRegExp("[\\.\\?!;]"));
        QStringList inversedList;
        for (QStringList::Iterator it = thatSentencesList.begin(); it != thatSentencesList.end(); ++it)
        {
	        //perform substitutions for that string
	        itOld = subOld.begin();
	        itNew = subNew.begin();
	        for (; itOld != subOld.end(); ++itOld, ++itNew )
	            tempResult.replace(*itOld, *itNew);
        	normalizeString(*it);
            inversedList.prepend(*it);
        }
      	thatList.prepend(inversedList);
        if (thatList.count() > MAX_LIST_LENGTH)
            thatList.pop_back();
        visitedNodeList.clear();
    }

    if (srai)
        indent --;

    return result;
}

QString AIMLParser::executeCommand(const QString &commandStr)
{
    QString returnString("");
    QString spaceIndent = QString().fill(' ', 2*indent);

    FILE *file = popen(commandStr.toAscii().data(), "r");
    if (!file)
    {
        return "";
    }

    char c = 0;
    while (c != EOF)
    {
        c = (char)getc(file);
        returnString += c;
    }

    fclose(file);

    //FIXME: funny hack to avoid umlaut-y symbol ?! help someone
    return returnString.left(returnString.length() - 1);
}
