#include "dmemorywarningdialog.h"
#include "processinfotable.h"
#include "processinfomodel.h"
#include "processinfodelegate.h"
#include "buttondelegate.h"

#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <DTitlebar>

DMemoryWarningDialog::DMemoryWarningDialog(QWidget *parent)
    : DMainWindow(parent)

    , m_infoModel(new ProcessInfoModel)
{
    titlebar()->setTitle(QString());

    ProcessInfoTable *table = new ProcessInfoTable;
    table->setModel(m_infoModel);
    table->setItemDelegate(new ProcessInfoDelegate);
    table->setItemDelegateForColumn(3, new ButtonDelegate);
    table->setFixedHeight(300);

    QPushButton *cancel = new QPushButton;
    cancel->setText(tr("Cancel"));

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addWidget(cancel);

    QLabel *icon = new QLabel;
    icon->setAlignment(Qt::AlignCenter);
    icon->setPixmap(QIcon::fromTheme("messagebox_warning").pixmap(64, 64));

    QLabel *label = new QLabel;
    label->setText(tr("Please free some memory to start new process"));
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    QFont f = label->font();
    f.setPointSize(12);
    label->setFont(f);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(icon);
    vLayout->addWidget(label);
    vLayout->addWidget(table);
    vLayout->addStretch();
    vLayout->addLayout(btnsLayout);

    QWidget *w = new QWidget;
    w->setLayout(vLayout);

    setCentralWidget(w);
    setFixedSize(400, 500);
    move(qApp->primaryScreen()->geometry().center() - rect().center());
}

void DMemoryWarningDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:    qApp->quit();   break;
#endif
    default:;
    }
}

void DMemoryWarningDialog::showEvent(QShowEvent *e)
{
    DMainWindow::showEvent(e);

    m_infoModel->startRefreshData();
}

void DMemoryWarningDialog::hideEvent(QHideEvent *e)
{
    DMainWindow::hideEvent(e);

    m_infoModel->stopRefreshData();
}
