#ifndef OPENBRAIN_H
#define OPENBRAIN_H

#include <Plasma/Applet>
#include <Plasma/DataEngine>
#include <Plasma/Label>
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
  class Label;
  class LineEdit;
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

  QString executeCommand(const QString&);
  QString thinkOf(QString s);

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
  Plasma::Label *m_response;
  Plasma::Slider *m_progress;

  AIMLParser *parser;

  Ui::OpenBrainConfig ui;
};

K_EXPORT_PLASMA_APPLET(openbrain, OpenBrain)

#endif
