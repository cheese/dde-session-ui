#ifndef SESSIONLOGIN
#define SESSIONLOGIN
#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "view/logowidget.h"
#include "view/switchframe.h"

#include "backgroundlabel.h"
#include "util_updateui.h"
#include "passwdedit.h"
#include "userwidget.h"

//#include "signalmanagement/signalmanager.h"

class BackgroundLabel;
class SessionLogin: public QFrame {
    Q_OBJECT
public:
    SessionLogin(QWidget*parent=0);
    ~SessionLogin();
    void initUI();
    void initData();
    void initConnection();
protected:
    void keyPressEvent(QKeyEvent *e);
public slots:
    void Loging();
    void testing(QString id);
    void moveUserWidget();
private:
    BackgroundLabel* m_backgroundLabel;
    LogoWidget* m_logoWidget;
    SwitchFrame* m_switchFrame;
    UserWidget* m_userWidget;
    PassWdEdit* m_passWdEdit;
    QHBoxLayout* m_passWdEditLayout;
    QVBoxLayout* m_Layout;


};
#endif // SESSIONLOGIN

