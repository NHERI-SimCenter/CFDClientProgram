#ifndef CWE_TABWIDGET_H
#define CWE_TABWIDGET_H

#include <QWidget>
#include <QFrame>
#include <QMap>
#include <QList>
#include <QTabWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QLayout>

#include "../cwe_defines.h"

class CWE_StageTab;
enum class SimCenterViewState;
enum class StageState;
class VWTinterfaceDriver;

namespace Ui {
class CWE_TabWidget;
}

class CWE_TabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CWE_TabWidget(QWidget *parent = 0);
    ~CWE_TabWidget();
    void setupDriver(VWTinterfaceDriver * theDriver);
    void resetView();

    void setViewState(SimCenterViewState);
    SimCenterViewState viewState();

    void addStageTab(QString s, QJsonObject &obj);

    int  addGroupTab(QString key, const QString &label, StageState currentState);

    void setCurrentWidget(QWidget *);
    bool addVariable(QString varName, QJsonObject JSONvariable, const QString &key, const QString &label , QString *setVal = NULL);

    int  index() { return activeIndex;};
    void setIndex(int );
    void setWidget(QWidget *);

    QWidget * currentWidget();
    QWidget * widget(int);

    int addVarTab(QString key, const QString &label);
    int addVarTab(QString key, const QString &label, QJsonArray *varList, QJsonObject *varsInfo, QMap<QString, QString> *setVars);
    void addVarsToTab(QString key, const QString &label, QJsonArray *varList, QJsonObject *varsInfo, QMap<QString,QString> *setVars);
    void addVSpacer(const QString &key, const QString &label);

    void addVarsData(QJsonObject , QJsonObject );

    QMap<QString, QString> collectParamData();

private slots:
    void on_pbtn_run_clicked();
    void on_pbtn_cancel_clicked();
    void on_pbtn_results_clicked();
    void on_pbtn_rollback_clicked();
    void on_groupTabSelected(int, QString selectedStage);

signals:
    void switchToResultsTab();
    void switchToFileTab();
    void switchToCreateTab();
    void switchToParameterTab();
    void switchToHelpTab();

protected:
    void setButtonMode(uint mode);

private:
    static QString getStateText(StageState theState);

    //TODO: This is a mess. Need to seriously reconsider interaction between UI state and UI elements.
    VWTinterfaceDriver * myDriver;
    QString currentSelectedStage;

    Ui::CWE_TabWidget *ui;
    int activeIndex;
    QWidget *displayWidget;

    SimCenterViewState m_viewState;

    QMap<QString, CWE_StageTab *> *m_stageTabs;

    //QMap<QString, CWE_WithStatusButton *> *groupWidget;
    //QMap<QString, QTabWidget *> *groupTabList;
    //QMap<QString, QMap<QString, QWidget *> *> *varTabWidgets;
    //QMap<QString, InputDataType *> *variableWidgets;
};

#endif // CWE_TABWIDGET_H
