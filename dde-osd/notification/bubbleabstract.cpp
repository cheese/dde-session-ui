/*
 * Copyright (C) 2014 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
 *             listenerri <listenerri@gmail.com>
 *
 * Maintainer: listenerri <listenerri@gmail.com>
 *             fanpengcheng <fanpengcheng_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bubbleabstract.h"
#include "notificationentity.h"
#include <QPainter>
#include "appicon.h"
#include "appbody.h"
#include "actionbutton.h"
#include "button.h"
#include "icondata.h"
#include "constants.h"

#include <QDebug>
#include <QDir>

DWIDGET_USE_NAMESPACE

static const QStringList HintsOrder {
    "desktop-entry",
    "image-data",
    "image-path",
    "image_path",
    "icon_data"
};

BubbleWidget_Bg::BubbleWidget_Bg(QWidget *parent)
    : DWidget(parent)
{
    installEventFilter(this);
}

bool BubbleWidget_Bg::eventFilter(QObject *obj, QEvent *event)
{
    // 这里只是为了让界面不被拖动
    if (obj) {
        if (event->type() == QEvent::MouseMove
                || event->type() == QEvent::Move) {
            return true;
        }
    }
    return false;
}

void BubbleWidget_Bg::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QPalette pe = this->palette();
    QColor c = pe.color(QPalette::Base);

    QColor brushColor(c);
    brushColor.setAlpha(m_hover ? m_hoverAlpha : m_unHoverAlpha);
    painter.setBrush(brushColor);

    QPen borderPen;
    borderPen.setColor(Qt::transparent);
    painter.setPen(borderPen);
    painter.drawRect(QRectF(0, 0, width(), height()));

    return DWidget::paintEvent(event);

}

void BubbleWidget_Bg::enterEvent(QEvent *event)
{
    m_hover = true;

    update();

    return DWidget::enterEvent(event);
}

void BubbleWidget_Bg::leaveEvent(QEvent *event)
{
    m_hover = false;

    update();

    return DWidget::leaveEvent(event);
}

BubbleAbStract::BubbleAbStract(QWidget *parent, std::shared_ptr<NotificationEntity> entity)
    : DBlurEffectWidget(parent)
    , m_entity(entity)
    , m_bgWidget(new BubbleWidget_Bg(this))
    , m_titleWidget(new BubbleWidget_Bg(this))
    , m_bodyWidget(new BubbleWidget_Bg(this))
    , m_appNameLabel(new DLabel(this))
    , m_appTimeLabel(new DLabel(this))
    , m_icon(new AppIcon(this))
    , m_body(new AppBody(this))
    , m_actionButton(new ActionButton(this))
    , m_closeButton(new Button(this))
{
    setAttribute(Qt::WA_TranslucentBackground);

    m_wmHelper = DWindowManagerHelper::instance();
    m_handle = new DPlatformWindowHandle(this);
    m_handle->setTranslucentBackground(true);
    m_handle->setShadowOffset(QPoint(0, 4));

    compositeChanged();

    setBlendMode(DBlurEffectWidget::BehindWindowBlend);
    setMaskColor(DBlurEffectWidget::AutoColor);
    setMouseTracking(true);

    //controls
    m_appNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_appTimeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_actionButton->clear();

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &BubbleAbStract::compositeChanged);
}

void BubbleAbStract::compositeChanged()
{
    if (!m_wmHelper->hasComposite()) {
        m_handle->setWindowRadius(0);
        m_handle->setShadowColor(QColor("#E5E5E5"));
    } else {
        m_handle->setWindowRadius(5);
        m_handle->setShadowColor(QColor(0, 0, 0, 100));
    }
}

void BubbleAbStract::updateContent()
{
    m_body->setTitle(m_entity->summary());
    m_body->setText(m_entity->body());
    m_appNameLabel->setText(m_entity->appName());

    processIconData();
    processActions();
}

inline void copyLineRGB32(QRgb *dst, const char *src, int width)
{
    const char *end = src + width * 3;
    for (; src != end; ++dst, src += 3) {
        *dst = qRgb(src[0], src[1], src[2]);
    }
}

inline void copyLineARGB32(QRgb *dst, const char *src, int width)
{
    const char *end = src + width * 4;
    for (; src != end; ++dst, src += 4) {
        *dst = qRgba(src[0], src[1], src[2], src[3]);
    }
}

static QImage decodeNotificationSpecImageHint(const QDBusArgument &arg)
{
    int width, height, rowStride, hasAlpha, bitsPerSample, channels;
    QByteArray pixels;
    char *ptr;
    char *end;

    arg.beginStructure();
    arg >> width >> height >> rowStride >> hasAlpha >> bitsPerSample >> channels >> pixels;
    arg.endStructure();
    //qDebug() << width << height << rowStride << hasAlpha << bitsPerSample << channels;

#define SANITY_CHECK(condition) \
    if (!(condition)) { \
        qWarning() << "Sanity check failed on" << #condition; \
        return QImage(); \
    }

    SANITY_CHECK(width > 0);
    SANITY_CHECK(width < 2048);
    SANITY_CHECK(height > 0);
    SANITY_CHECK(height < 2048);
    SANITY_CHECK(rowStride > 0);

#undef SANITY_CHECK

    QImage::Format format = QImage::Format_Invalid;
    void (*fcn)(QRgb *, const char *, int) = nullptr;
    if (bitsPerSample == 8) {
        if (channels == 4) {
            format = QImage::Format_ARGB32;
            fcn = copyLineARGB32;
        } else if (channels == 3) {
            format = QImage::Format_RGB32;
            fcn = copyLineRGB32;
        }
    }
    if (format == QImage::Format_Invalid) {
        qWarning() << "Unsupported image format (hasAlpha:" << hasAlpha << "bitsPerSample:" << bitsPerSample << "channels:" << channels << ")";
        return QImage();
    }

    QImage image(width, height, format);
    ptr = pixels.data();
    end = ptr + pixels.length();
    for (int y = 0; y < height; ++y, ptr += rowStride) {
        if (ptr + channels * width > end) {
            qWarning() << "Image data is incomplete. y:" << y << "height:" << height;
            break;
        }
        fcn((QRgb *)image.scanLine(y), ptr, width);
    }

    return image;
}

void BubbleAbStract::saveImg(const QImage &image)
{
    QDir dir;
    dir.mkdir(CachePath);

    image.save(CachePath + QString::number(m_entity->id()) + ".png");
}

const QPixmap BubbleAbStract::converToPixmap(const QDBusArgument &value)
{
    // use plasma notify source code to conver photo, solving encoded question.
    const QImage &img = decodeNotificationSpecImageHint(value);
    saveImg(img);
    return QPixmap::fromImage(img).scaled(m_icon->width(), m_icon->height(),
                                          Qt::KeepAspectRatioByExpanding,
                                          Qt::SmoothTransformation);
}

// Each even element in the list (starting at index 0) represents the identifier for the action.
// Each odd element in the list is the localized string that will be displayed to the user.
void BubbleAbStract::processActions()
{
    m_actionButton->clear();

    QStringList list = m_entity->actions();
    // the "default" is identifier for the default action
    if (list.contains("default")) {
        const int index = list.indexOf("default");
        m_defaultAction = list[index];
        // Default action does not need to be displayed, removed from the list
        list.removeAt(index + 1);
        list.removeAt(index);
    }

    m_actionButton->addButtons(list);

    m_canClose = list.isEmpty();
}

void BubbleAbStract::processIconData()
{
    const QVariantMap &hints = m_entity->hints();
    QString imagePath;
    QPixmap imagePixmap;

    for (const QString &hint : HintsOrder) {
        const QVariant &source = hints.contains(hint) ? hints[hint] : QVariant();

        if (source.isNull()) continue;

        if (source.canConvert<QDBusArgument>()) {
            QDBusArgument argument = source.value<QDBusArgument>();
            imagePixmap = converToPixmap(argument);
            break;
        }

        imagePath = source.toString();
    }

    if (!imagePixmap.isNull()) {
        m_icon->setPixmap(imagePixmap);
    } else {
        m_icon->setIcon(imagePath.isEmpty() ? m_entity->appIcon() : imagePath, m_entity->appName());
    }
}

bool BubbleAbStract::containsMouse() const
{
    return geometry().contains(QCursor::pos());
}

void BubbleAbStract::enterEvent(QEvent *event)
{
    if (m_canClose) {
        m_closeButton->setVisible(true);
    }

    return DBlurEffectWidget::enterEvent(event);
}

void BubbleAbStract::leaveEvent(QEvent *event)
{
    if (m_canClose) {
        m_closeButton->setVisible(false);
    }

    return DBlurEffectWidget::leaveEvent(event);
}