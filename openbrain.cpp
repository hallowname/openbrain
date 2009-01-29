#include "openbrain.h"

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

using namespace std;

OpenBrain::OpenBrain(QObject *parent,
		    const QVariantList &args)
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
//     connect(m_detailsAction, SIGNAL(triggered()), this, SLOT(displayDetails()));
    connect(m_homeAction, SIGNAL(triggered()), this, SLOT(displayHome()));
//     connect(m_teachAction, SIGNAL(triggered()), this, SLOT(teach()));
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

//     m_progress = new Plasma::Meter(this);
//     m_progress->setMeterType(Plasma::Meter::BarMeterHorizontal);
//     m_progress->setMinimum(0);
//     m_progress->setMaximum(100);

    m_zoom = new Plasma::Slider(this);
    m_zoom->setMaximum(100);
    m_zoom->setMinimum(0);
    m_zoom->setValue(50);
    m_zoom->setOrientation(Qt::Horizontal);

    m_inputLayout->addItem(m_inputLineEdit);

    m_go = addTool("go-jump-locationbar", m_inputLayout);
    m_goAction = m_go->action();

//     m_details = addTool("edit-find-user", m_inputLayout);
//     m_detailsAction = m_details->action();
//     
//     m_teach = addTool("help-hint", m_inputLayout);
//     m_teachAction = m_teach->action();
    
    m_home = addTool("go-home", m_inputLayout);
    m_homeAction = m_home->action();

//     m_progressLayout->addItem(m_progress);
    m_zoomLayout->addItem(m_zoom);

    m_layout->addItem(m_inputLayout);
    m_layout->addItem(m_browser);
    m_layout->addItem(m_zoomLayout);
//     m_layout->addItem(m_progressLayout);

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
    parser = new AIMLParser(&logStream);
    parser->loadVars(QDir::homePath() + "/.openbrain/utils/vars.xml", false);
    parser->loadVars(QDir::homePath() + "/.openbrain/utils/bot.xml", true);
    parser->loadSubstitutions(QDir::homePath() + "/.openbrain/utils/substitutions.xml");
        
    QString dirname = QDir::homePath() + "/.openbrain/aiml/default";
    QDir dir(dirname);
    QStringList files = dir.entryList(QStringList("*.aiml"));
    m_zoom->setMaximum(files.count());
    uint i = 0;
    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
	m_browser->setHtml("<html><body bgcolor=black><font color=white><center>Parsing:<br>" + *it + "...</center></font></body></html>");
        m_zoom->setValue( i );

        qApp->processEvents();

        parser->loadAiml(dirname + "/" + *it);
        i++;
    }
    m_zoom->setMaximum(100);
    m_zoom->setValue(50);
    m_inputLineEdit->setText("openbrain home display");
    emit returnPressed();
    timer->stop();
    m_inputLineEdit->setFocus();
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

#include "openbrain.moc"
