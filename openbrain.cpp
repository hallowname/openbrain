#include <limits.h>

#include <QAction>
#include <QDir>
#include <QFile>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QTextStream>
#include <QTimer>
#include <QWebFrame>

#include <KConfigDialog>
#include <KIcon>
#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/LineEdit>
#include <Plasma/Slider>

#include "openbrain.h"

using namespace std;

OpenBrain::OpenBrain(QObject *parent, const QVariantList &args)
: Plasma::Applet(parent, args)
{
  setHasConfigurationInterface(true);
  setAspectRatioMode(Plasma::IgnoreAspectRatio);
  setBackgroundHints(TranslucentBackground);

  m_layout = 0;
  resize(240,200);
}

void OpenBrain::setupConnections()
{
  timer = new QTimer(this);

  connect(m_goAction, SIGNAL(triggered()), this, SLOT(returnPressed()));
  connect(m_homeAction, SIGNAL(triggered()), this, SLOT(displayHome()));
  connect(m_inputLineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
  connect(timer, SIGNAL(timeout()), this, SLOT(loadBrain()));
}

void OpenBrain::init()
{
  KConfigGroup cg = config();

  QSizePolicy expanding_policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QSizePolicy minimal_policy(QSizePolicy::Expanding, QSizePolicy::Maximum);

  m_layout = new QGraphicsLinearLayout(Qt::Vertical);
  m_inputLayout = new QGraphicsLinearLayout(Qt::Horizontal);
  m_progressLayout = new QGraphicsLinearLayout(Qt::Horizontal);

  m_inputLineEdit = new Plasma::LineEdit(this);

  m_response = new Plasma::Label(this);

  m_progress = new Plasma::Slider(this);
  m_progress->setMaximum(100);
  m_progress->setMinimum(0);
  m_progress->setValue(50);
  m_progress->setOrientation(Qt::Horizontal);
  m_progress->setSizePolicy(minimal_policy);

  m_inputLayout->addItem(m_inputLineEdit);

  m_go = addTool("go-jump-locationbar", m_inputLayout);
  m_goAction = m_go->action();

  m_home = addTool("go-home", m_inputLayout);
  m_homeAction = m_home->action();

  m_progressLayout->addItem(m_progress);

  m_layout->addItem(m_inputLayout);
  m_layout->addItem(m_response);
  m_layout->addItem(m_progressLayout);

  m_response->nativeWidget()->setSizePolicy(expanding_policy);
  m_response->nativeWidget()->setWordWrap(true);
  m_response->nativeWidget()->setTextInteractionFlags(Qt::NoTextInteraction);
  m_response->nativeWidget()->setAlignment(Qt::AlignCenter);

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
  m_inputLineEdit->setText(m_inputLineEdit->text().replace("?", ""));
  m_inputLineEdit->setText(m_inputLineEdit->text().replace(".", ""));
  m_inputLineEdit->setText(m_inputLineEdit->text().replace(",", ""));
  m_inputLineEdit->setText(m_inputLineEdit->text().replace("/", ""));
  m_response->nativeWidget()->setText(thinkOf(parser->getResponse(m_inputLineEdit->text())));

  QFile f(QDir::homePath() + "/.openbrain/send_to_author");
  QTextStream out(&f);
  if (f.open(QIODevice::Append))
    out << m_inputLineEdit->text() << "\n";
    out << m_response->text() << "\n\n";

  f.close();

  m_inputLineEdit->setText("");
  m_inputLineEdit->setFocus();
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
  m_progress->setMaximum(files.count());
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
  m_response->nativeWidget()->setText(str);
}

void OpenBrain::updateLoadingPercentage(int val)
{
  m_progress->setValue(val);
}

void OpenBrain::doneLoading()
{
  m_progress->setMaximum(100);
  m_progress->hide();
  m_inputLineEdit->setText("openbrain home display");
  emit returnPressed();
  m_inputLineEdit->setFocus();
}

#include "openbrain.moc"
