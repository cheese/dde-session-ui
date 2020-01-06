#include "sessionbasewindow.h"

#include <QDebug>
#include <QScrollArea>
#include <QResizeEvent>
#include <QApplication>

SessionBaseWindow::SessionBaseWindow(QWidget *parent)
    : QFrame(parent)
    , m_centerWidget(nullptr)
    , m_leftBottomWidget(nullptr)
    , m_rightBottomWidget(nullptr)
{
    initUI();
}

void SessionBaseWindow::setLeftBottomWidget(QWidget * const widget)
{
    if (m_leftBottomWidget != nullptr) {
        m_leftBottomLayout->removeWidget(m_leftBottomWidget);
    }

    m_leftBottomLayout->addWidget(widget, 0, Qt::AlignBottom);
    m_leftBottomWidget = widget;
}

void SessionBaseWindow::setRightBottomWidget(QWidget * const widget)
{
    if (m_rightBottomWidget != nullptr) {
        m_rightBottomLayout->removeWidget(m_rightBottomWidget);
    }

    m_rightBottomLayout->addWidget(widget, 0, Qt::AlignBottom);
    m_rightBottomWidget = widget;
}

void SessionBaseWindow::setCenterContent(QWidget * const widget)
{
    if (m_centerWidget != nullptr) {
        m_centerLayout->removeWidget(m_centerWidget);
        m_centerWidget->hide();
    }

    m_centerLayout->addWidget(widget, 0, Qt::AlignCenter);
    m_centerWidget = widget;

    widget->show();
}

void SessionBaseWindow::initUI()
{
    m_mainLayou = new QVBoxLayout;

    m_centerLayout = new QHBoxLayout;
    m_leftBottomLayout = new QHBoxLayout;
    m_rightBottomLayout = new QHBoxLayout;

    QFrame *bottomWidget = new QFrame;
    QWidget *centerWidget = new QWidget;
    centerWidget->setLayout(m_centerLayout);

    QHBoxLayout *bottomLayout = new QHBoxLayout;

    bottomLayout->addLayout(m_leftBottomLayout);
    bottomLayout->addLayout(m_rightBottomLayout);

    bottomWidget->setLayout(bottomLayout);
    bottomWidget->setFixedHeight(132);

    m_mainLayou->addWidget(centerWidget);
    m_mainLayou->addWidget(bottomWidget);

    setLayout(m_mainLayou);
}

