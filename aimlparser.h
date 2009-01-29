#ifndef AIMLPARSER_H
#define AIMLPARSER_H

#include <QDomNode>
#include <QList>
#include <QMap>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>

#define MAX_LIST_LENGTH 50

struct Node;

struct Leaf
{
  Node *parent;
  QDomNode tmplate;
  QString topic;
  QString that;
  Leaf();
};

struct Node
{
  Node *parent;
  QString word;
  Node();
  QList<Node*> childs;
  QList<Leaf*> leafs;
  void debug(QTextStream* logStream, 
	     uint indent = 0);
  bool match(QStringList::const_iterator, 
	     const QStringList&,
             const QString&, 
	     const QString&, 
	     QStringList &, 
	     QStringList &, 
	     Leaf *&);
};

class AIMLParser
{
public:
  AIMLParser(QTextStream*);
  virtual ~AIMLParser();

  bool loadAiml(const QString&);
  bool loadSubstitutions(const QString&);
  bool loadVars(const QString&, const bool&);
  bool saveVars(const QString &);

  QString getResponse(QString, const bool &srai = false);

  void displayTree();
  void runRegression();

private:
  QString executeCommand(const QString&);
  QString resolveNode(QDomNode*, 
		      const QStringList & = QStringList(), 
		      const QStringList & = QStringList(), 
		      const QStringList & = QStringList());

  void parseCategory(QDomNode*);
  void normalizeString(QString &);

  QMap<QString, QString> parameterValue;
  QMap<QString, QString> botVarValue;

  QList<QRegExp> subOld;
  QList<QDomNode*> visitedNodeList;
  
  QStringList subNew;
  QStringList inputList;

  QList<QStringList> thatList;

  Node root;
  
  int indent;

  QTextStream *logStream;
};

#endif




