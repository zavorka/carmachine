#include "imageviewer.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMatrix>
#include <QPoint>
#include <QCursor>
#include <QDebug>

ImageViewer::ImageViewer(QWidget *parent) : QWidget(parent)
  ,m_scale(0.0)
  ,m_percentage(0.0)
  ,m_originX(0.0), m_originY(0.0)
  ,m_basicX(0.0), m_basicY(0.0)
  ,currentStepScaleFactor(1)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    this->grabGesture(Qt::PinchGesture);
}

void ImageViewer::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;

    showSuitableSize();
}

void ImageViewer::ariseScale(int rate)
{
    double old_percentage = m_percentage;
    double step = static_cast<double>(rate)/100.0*5*old_percentage;
    m_percentage += step;
    if(m_percentage < 0.01)
    {
        m_percentage = 0.01;
    }
    else if(m_percentage >1.5)
    {
        m_percentage = 1.5;
    }

    m_scale = m_percentage*m_scale/old_percentage;

    update();
    emit percentageChanged( m_percentage );
}

void ImageViewer::showOriginalSize()
{
    double old_percentage = m_percentage;
    m_percentage = 1.0;
    m_scale = m_percentage*m_scale/old_percentage;

    update();
    emit percentageChanged( m_percentage );
}

void ImageViewer::showSuitableSize()
{
    double pixwidth = static_cast<double>(m_pixmap.width());
    double pixheight = static_cast<double>(m_pixmap.height());
    double showwidth = static_cast<double>(width());
    double showheight = static_cast<double>(height());

    //缩放比例
    double Wpercentage = showwidth / pixwidth;
    double Hpercentage = showheight / pixheight;
    m_percentage = qMin(Wpercentage, Hpercentage);
    m_suitableWidth = pixwidth*m_percentage;
    m_suitableHeight = pixheight*m_percentage;
    if(m_percentage < 0.01)
    {
        m_percentage = 0.01;
    }
    else if( m_percentage > 1)
    {
        m_percentage = 1.0;
    }
    m_scale = 1.0;

    //图片初始位置（用于移动）
    m_basicX = showwidth/2.0 - pixwidth*m_percentage/2.0;
    m_originX = m_basicX;
    m_basicY = showheight/2.0- pixheight*m_percentage/2.0;
    m_originY = m_basicY;

    update();
    emit percentageChanged( m_percentage );
}

void ImageViewer::zoomIn()
{
    ariseScale(1);
}

void ImageViewer::zoomOut()
{
    ariseScale(-1);
}

void ImageViewer::clockwise90()
{
    QMatrix matrix;
    matrix.rotate(90);

    m_pixmap = m_pixmap.transformed(matrix, Qt::FastTransformation);
    showSuitableSize();
}

void ImageViewer::anticlockwise90()
{
    QMatrix matrix;
    matrix.rotate(-90);

    m_pixmap = m_pixmap.transformed(matrix, Qt::FastTransformation);
    showSuitableSize();
}

void ImageViewer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    double pixwidth = static_cast<double>(m_pixmap.width());
    double pixheight = static_cast<double>(m_pixmap.height());
    double showwidth = static_cast<double>(width());
    double showheight = static_cast<double>(height());

    double Wscalerate = pixwidth / showwidth;
    double Hscalerate = pixheight / showheight;
    double compare = qMax(Wscalerate, Hscalerate);

    if( compare < 1.0 )
        compare = 1.0;

    QPainter painter(this);
    //Draw background
    painter.save();
    QRect backgroundRect = rect();
    QColor color(0, 0, 0);
    painter.setPen(color);
    painter.setBrush(QBrush(color));
    painter.drawRect(backgroundRect);
    painter.restore();

    //Draw Image
    QRect showRect = QRect( m_originX, m_originY, pixwidth/compare, pixheight/compare);
    painter.save();
    painter.translate(showwidth/2.0, (showheight/2.0));
    painter.scale(m_scale, m_scale);
    painter.translate(-(showwidth/2.0), -(showheight/2.0));
    painter.drawPixmap(showRect, m_pixmap);
    painter.restore();
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);
    int numDegrees = event->delta() / 8;    //滚动的角度，*8就是鼠标滚动的距离
    int numSteps = numDegrees / 15;         //滚动的步数，*15就是鼠标滚动的角度
    if (event->orientation() == Qt::Horizontal) {
        event->accept();
    } else {
        ariseScale(numSteps);
    }
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton  )
    {
        m_pressPoint = event->pos();
    }
    else if( event->button() == Qt::RightButton )
    {
        emit rightButtonClicked();
    }

    QWidget::mousePressEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if( event->button() == Qt::LeftButton )
    {
        m_basicX = m_originX;
        m_basicY = m_originY;
    }
    QWidget::mouseReleaseEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if(m_pressPoint != QPoint(-1,-1))
    {
        //鼠标相对于屏幕的位置
        QPoint move_pos = event->pos();

        if( rect().contains(event->pos()) )
        {
            move_pos -= m_pressPoint;
            m_originX = m_basicX + move_pos.x()/m_scale;
            m_originY = m_basicY + move_pos.y()/m_scale;
            update();
        }
        else
        {
            QPoint point;
            if( event->pos().x() < 0 )
            {
                point = QPoint( 0, event->pos().y() );
            }
            else if( event->pos().x() > rect().width()-1 )
            {
                point = QPoint( rect().width()-1, event->pos().y() );
            }
            else if( event->pos().y() < 0 )
            {
                point = QPoint( event->pos().x(), 0 );
            }
            else if( event->pos().y() > rect().height()-1 )
            {
                point = QPoint( event->pos().x(), rect().height()-1 );
            }
        }
    }
    QWidget::mouseMoveEvent(event);
}

bool ImageViewer::event(QEvent *event)
{
    if(event->type() == QEvent::Gesture)
    {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent*>(event);
        if(QGesture *gesture = gestureEvent->gesture(Qt::PinchGesture))
        {
            m_pressPoint = QPoint(-1,-1);
            QPinchGesture *pinchGesture = static_cast<QPinchGesture*>(gesture);
            QPinchGesture::ChangeFlags changeFlags = pinchGesture->changeFlags();
            if(changeFlags & QPinchGesture::ScaleFactorChanged)
            {
#ifdef DEVICE_EVB
                if(pinchGesture->totalScaleFactor()>1.2)
                {
                    ariseScale(1);
                }
                else if(pinchGesture->totalScaleFactor()<0.8)
                {
                    ariseScale(-1);
                }
#else
                if(pinchGesture->totalScaleFactor()>1.6)
                {
                    ariseScale(1);
                }
                else if(pinchGesture->totalScaleFactor()<0.4)
                {
                    ariseScale(-1);
                }
#endif
            }
        }
    }
    return QWidget::event(event);
}
