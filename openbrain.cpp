#include <limits.h>

#include <QAction>
#include <QDir>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QWebFrame>

#include <KConfigDialog>
#include <KIcon>
#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/LineEdit>
#include <Plasma/Meter>
#include <Plasma/Slider>
#include <Plasma/TreeView>
#include <Plasma/WebView>

#include "openbrain.h"

using namespace std;

OpenBrain::OpenBrain(QObject *parent, const QVariantList &args)
: Plasma::Applet(parent, args)
{
  setHasConfigurationInterface(true);
  setAspectRatioMode(Plasma::IgnoreAspectRatio);
  setBackgroundHints(TranslucentBackground);

  m_layout = 0;
  resize(230,180);
}

void OpenBrain::setupConnections()
{
  timer = new QTimer(this);

  connect(m_goAction, SIGNAL(triggered()), this, SLOT(returnPressed()));
  connect(m_homeAction, SIGNAL(triggered()), this, SLOT(displayHome()));
  connect(m_inputLineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
  connect(m_zoom, SIGNAL(sliderMoved(int)), this, SLOT(zoom(int)));
  connect(timer, SIGNAL(timeout()), this, SLOT(loadBrain()));
}

void OpenBrain::init()
{
  KConfigGroup cg = config();

  QSizePolicy expanding_policy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  m_layout = new QGraphicsLinearLayout(Qt::Vertical);
  m_inputLayout = new QGraphicsLinearLayout(Qt::Horizontal);
  m_zoomLayout = new QGraphicsLinearLayout(Qt::Horizontal);
  m_progressLayout = new QGraphicsLinearLayout(Qt::Horizontal);

  m_inputLineEdit = new Plasma::LineEdit(this);

  m_browser = new Plasma::WebView(this);
  m_browser->setSizePolicy(expanding_policy);

  m_zoom = new Plasma::Slider(this);
  m_zoom->setMaximum(100);
  m_zoom->setMinimum(0);
  m_zoom->setValue(50);
  m_zoom->setOrientation(Qt::Horizontal);

  m_inputLayout->addItem(m_inputLineEdit);

  m_go = addTool("go-jump-locationbar", m_inputLayout);
  m_goAction = m_go->action();

  m_home = addTool("go-home", m_inputLayout);
  m_homeAction = m_home->action();

  m_zoomLayout->addItem(m_zoom);

  m_layout->addItem(m_inputLayout);
  m_layout->addItem(m_browser);
  m_layout->addItem(m_zoomLayout);

  setLayout(m_layout);

  setupConnections();

  if (QDir(QDir::homePath() + "/.openbrain").exists())
  {
    timer->start(2000);
  }
  else
  {
    QProcess p(this);
    p.start("cp " + executeCommand("kde4-config --install data") + "/openbrain/openbrain_configuration " + QDir::homePath() + "/.openbrain -arf");
    if (!p.waitForFinished())
    {
      QMessageBox::information(0, "Error", "~/.openbrain didn't exist, and copying the default 'brain' from " + executeCommand("kde4-config --install data") + " failed.");
    }
    else
    {
      timer->start(1000);
    }
  }
}

QString OpenBrain::executeCommand(const QString &commandStr)
{
/*  QString returnString("");
  QProcess proc;
  proc.start(commandStr);
  if (!proc.waitForFinished())
  {
    return commandStr + " could not be executed.";
  }
  while (!proc.atEnd())
  {
    QByteArray line = proc.readLine();
    if (line.length())
      line.truncate(line.length()-1);
    if (!line.isEmpty())
      returnString += (QString) line;
  }
  return returnString;*/
  QString returnString("");
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
  return returnString.left(returnString.length() - 1).trimmed();
}

OpenBrain::~OpenBrain()
{
  KConfigGroup cg = config();
  saveState(cg);
}

void OpenBrain::returnPressed()
{
  if (m_inputLineEdit->text().contains("?"))
  m_inputLineEdit->setText(m_inputLineEdit->text().replace("?", ""));
  m_browser->setHtml("<html><body bgcolor=black><font color=white>" + thinkOf(parser->getResponse(m_inputLineEdit->text())) + "</font></body></html>");
  m_inputLineEdit->setText("");
  m_inputLineEdit->setFocus();
}

void OpenBrain::zoom(int value)
{
  m_browser->mainFrame()->setTextSizeMultiplier((qreal)0.2 + ((qreal)value/(qreal)50));
}

QString OpenBrain::thinkOf(QString s)
{
  return s;
}

void OpenBrain::createConfigurationInterface(KConfigDialog *parent)
{
  QWidget *widget = new QWidget();
  ui.setupUi(widget);

  parent->setButtons(KDialog::Ok | KDialog::Cancel);
  parent->addPage(widget, i18n("General"), icon());

  connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
  connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

void OpenBrain::configAccepted()
{
  KConfigGroup cg = config();

  emit configNeedsSaving();
}

void OpenBrain::loadBrain()
{
  timer->stop();

  parser = new AIMLParser();

  connect(parser, SIGNAL(aimlLoad_doc(QString)), this, SLOT(updateLoadingPage(QString)));
  connect(parser, SIGNAL(aimlLoad_percent(int)), this, SLOT(updateLoadingPercentage(int)));
  connect(parser, SIGNAL(finished()), this, SLOT(doneLoading()));

  QDir dir(QDir::homePath() + "/.openbrain/aiml/default");
  QStringList files = dir.entryList(QStringList("*.aiml"));
  m_zoom->setMaximum(files.count());
  parser->start();
}

void OpenBrain::displayDetails()
{
  m_inputLineEdit->setText("openbrain details display");
  emit returnPressed();
}

void OpenBrain::displayHome()
{
  m_inputLineEdit->setText("openbrain home display");
  emit returnPressed();
}

void OpenBrain::teach()
{
  m_inputLineEdit->setText("openbrain teach display");
  emit returnPressed();
}

Plasma::IconWidget *OpenBrain::addTool(const QString &iconString, QGraphicsLinearLayout *layout)
{
  Plasma::IconWidget *icon = new Plasma::IconWidget(this);
  QAction *action = new QAction(KIcon(iconString), QString(), this);
  icon->setAction(action);
  icon->setPreferredSize(icon->sizeFromIconSize(IconSize(KIconLoader::Toolbar)));
  layout->addItem(icon);

  return icon;
}

void OpenBrain::updateLoadingPage(const QString &str)
{
  m_browser->setHtml("<html><body bgcolor=black><font color=white>" + str + "</font></body></html>");
}

void OpenBrain::updateLoadingPercentage(int val)
{
  m_zoom->setValue(val);
}

void OpenBrain::doneLoading()
{
  m_zoom->setMaximum(100);
  m_zoom->setValue(50);
  m_inputLineEdit->setText("openbrain home display");
  emit returnPressed();
  m_inputLineEdit->setFocus();
}

#include "openbrain.moc"
