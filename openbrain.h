#ifndef OPENBRAIN_H
#define OPENBRAIN_H

#include <Plasma/Applet>
#include <Plasma/DataEngine>
#include <Plasma/LineEdit>

#include <iostream>
#include <string>

#include "ui_openbrainconfig.h"
#include "aimlparser.h"

using namespace std;

class QGraphicsLinearLayout;
class QTimer;
class QAction;

namespace Plasma
{
  class IconWidget;
  class LineEdit;
  class Meter;
  class WebView;
  class Slider;
}

class OpenBrain 
: public Plasma::Applet
{
  Q_OBJECT

  public:
  void init();

  OpenBrain(QObject *parent, const QVariantList &args);
  ~OpenBrain();

  protected:
  Plasma::IconWidget *addTool(const QString &iconString, QGraphicsLinearLayout *layout);
  void createConfigurationInterface(KConfigDialog *parent);

  protected Q_SLOTS:
  void configAccepted();
  void displayDetails();
  void displayHome();
  void loadBrain();
  void returnPressed();
  void teach();
  void zoom(int);
  void updateLoadingPage(const QString&);
  void updateLoadingPercentage(int);
  void doneLoading();

  private:
  void setupInitialProperties();
  void setupConnections();
  void setupLayout();

  QAction *m_goAction;
  QAction *m_detailsAction;
  QAction *m_homeAction;
  QAction *m_teachAction;

  QGraphicsLinearLayout *m_layout;
  QGraphicsLinearLayout *m_inputLayout;
  QGraphicsLinearLayout *m_progressLayout;
  QGraphicsLinearLayout *m_zoomLayout;

  QString executeCommand(const QString&);
  QString thinkOf(QString s);

  QTextStream logStream;

  QTimer *timer;

  bool cleartext;

  int m_horizontalScrollValue;
  int m_verticalScrollValue;

  Plasma::IconWidget *m_details;
  Plasma::IconWidget *m_go;
  Plasma::IconWidget *m_home;
  Plasma::IconWidget *m_teach;
  Plasma::IconWidget *m_stop;
  Plasma::LineEdit *m_inputLineEdit;
  Plasma::WebView *m_browser;
  Plasma::Meter *m_progress;
  Plasma::Slider *m_zoom;

  AIMLParser *parser;

  Ui::OpenBrainConfig ui;
};

K_EXPORT_PLASMA_APPLET(openbrain, OpenBrain)

#endif
