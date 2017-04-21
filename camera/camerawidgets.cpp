#include "camerawidgets.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <gst/gst.h>

cameraWidgets::cameraWidgets(QWidget *parent):baseWidget(parent)
{
    setObjectName("cameraWidgets");
    setStyleSheet("#cameraWidgets{border-image: url(:/image/camera/camera_bg.jpg);}");
    init();
    initLayout();
}

cameraWidgets::~cameraWidgets()
{
    qDebug() << "cameraWidgets destroy";
    
}

void cameraWidgets::openCamera()
{
    qDebug() << "new Qcamera start";
    qDebug() << "gst_debug level set";
    gst_debug_set_default_threshold(GST_LEVEL_FIXME);
    //gst_debug_set_threshold_for_name ("v4l2allocator", GST_LEVEL_DEBUG);
    m_camera = new QCamera(this);
    
    qDebug() << "new Qcamera end:" << m_camera;
    //m_imageCapture=new QCameraImageCapture(m_camera);

    if (m_camera) {
        connect(m_camera, SIGNAL(error(QCamera::Error)),this,SLOT(slot_cameraerror(QCamera::Error)));
        m_camera->setViewfinder(m_viewfinder);
        //m_viewfinder->hide();
        qDebug() << "Qcamera setViewfinder show!!!";

        m_camera->load();
        
        qDebug() << "Qcamera start";
        m_imageCapture = new QCameraImageCapture(m_camera);
        QImageEncoderSettings imageSettings;
        imageSettings.setCodec("image/jpeg");
        imageSettings.setResolution(640, 480);

        m_imageCapture->setEncodingSettings(imageSettings);

        QList<QSize> supportResolution = m_imageCapture->supportedResolutions();
        if (!supportResolution.isEmpty()) {
            qDebug() << "supportResolution:" << supportResolution.size();
            for (int i = 0; i < supportResolution.size(); i++) {
                qDebug() << "support resolution:" << supportResolution.at(i);
            }
        }

        m_mediaRecorder = new QMediaRecorder(m_camera);
        
        QStringList supportCodecs = m_mediaRecorder->supportedVideoCodecs();

        qDebug() << "supportCodecs size :" << supportCodecs.size();
        //video codecs
        foreach (const QString &codecName, m_mediaRecorder->supportedVideoCodecs()) {
            QString description = m_mediaRecorder->videoCodecDescription(codecName);
            qDebug() << codecName;
        }

        qDebug() << "======";
        foreach (const QString &format, m_mediaRecorder->supportedContainers()) {
            qDebug() << format;
        }
        
        QVideoEncoderSettings videoEncorder;
        videoEncorder.setResolution(640,480);
        videoEncorder.setCodec(supportCodecs.at(19));
        m_mediaRecorder->setContainerFormat("video/mpeg4");
        m_mediaRecorder->setVideoSettings(videoEncorder);
        //m_mediaRecorder->setOutputLocation(QUrl::fromLocalFile("test"));
        
        
        connect(m_mediaRecorder, SIGNAL(stateChanged(QMediaRecorder::State)), this, SLOT(updateRecorderState(QMediaRecorder::State)));
        connect(m_mediaRecorder, SIGNAL(durationChanged(qint64)), this, SLOT(updateRecordTime()));
        connect(m_mediaRecorder, SIGNAL(error(QMediaRecorder::Error)), this, SLOT(displayRecorderError()));

        connect(m_imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCapture(bool)));
        connect(m_imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
        connect(m_imageCapture, SIGNAL(imageSaved(int,QString)), this, SLOT(imageSaved(int,QString)));
        connect(m_imageCapture, SIGNAL(error(int, QCameraImageCapture::Error,QString)),
            this,SLOT(slot_captureerror(int, QCameraImageCapture::Error,QString)));

        m_camera->setCaptureMode(QCamera::CaptureStillImage);
        m_camera->start();
        qDebug() << "Qcamera start end";
        qDebug() << "isCaptureModeSupported:" << m_camera->isCaptureModeSupported(QCamera::CaptureStillImage);
        qDebug() << "isVideoModeSupported" << m_camera->isCaptureModeSupported(QCamera::CaptureVideo);

        QList<QSize> supportViewFinderSizes = m_camera->supportedViewfinderResolutions();
        qDebug() << "get supportedViewfinderResolutions  :" << supportViewFinderSizes;
        if (!supportViewFinderSizes.isEmpty()) {
            qDebug() << "supportViewFinderSizes:" << supportViewFinderSizes.size();
            for (int i = 0; i < supportViewFinderSizes.size(); i++)
                qDebug() << "supportViewFinderSizes:" << supportViewFinderSizes.at(i);
        }
    }
}

void cameraWidgets::closeCamera()
{
    if (m_camera) {
        m_camera->unload();
        m_camera->stop();
        delete m_camera;
    }
    if (m_mediaRecorder)
        delete m_mediaRecorder;
}

void cameraWidgets::slot_cameraerror(QCamera::Error value)
{
    qDebug() << "camera error:" << value;
}

void cameraWidgets::slot_pressed()
{
    qDebug() << "capture btn pressed";
    /*if (m_camera && m_imageCapture) {
        m_camera->searchAndLock();
    }*/
}

void cameraWidgets::slot_released()
{
    qDebug() << "capture btn released";
    /*if (m_camera && m_imageCapture) {
        m_camera->unlock();
    }*/
}

void cameraWidgets::slot_captureerror(int id, QCameraImageCapture::Error error, const QString &errorString)
{
    qDebug() << "id:" << id << " capture error:" << errorString;
}

void cameraWidgets::readyForCapture(bool ready)
{
    qDebug() << "readyForCapture:" << ready;
}

void cameraWidgets::processCapturedImage(int requestId,const QImage & img)
{
    qDebug() << requestId << " processCapturedImage: " << img;
}

void cameraWidgets::imageSaved(int id,const QString & filename)
{
    qDebug() << id << "imageSaved = " << filename;
    //m_viewfinder->setVisible(true);
    /*if (m_camera) {
        qDebug() << "restart camera";
        m_camera->unload();
        m_camera->stop();
        m_camera->load();
        m_camera->start();
    }*/
}

void cameraWidgets::slot_takepictrue()
{
    qDebug() << "capture btn clicked";
    if (m_camera && m_imageCapture) {
        m_camera->setCaptureMode(QCamera::CaptureStillImage);
        //m_camera->searchAndLock();
        //m_viewfinder->setVisible(false);
        m_imageCapture->capture();
        //m_camera->unlock();
    }
}

void cameraWidgets::slot_recorder()
{
    qDebug() << "recorder btn clicked";
    if (m_mediaRecorder && m_camera) {
        m_camera->setCaptureMode(QCamera::CaptureVideo);
        if (m_mediaRecorder->state() == QMediaRecorder::RecordingState)
            m_mediaRecorder->stop();
        else
            m_mediaRecorder->record();
    }
}

void cameraWidgets::updateRecorderState(QMediaRecorder::State state)
{
    switch (state) {
    case QMediaRecorder::StoppedState:
        m_recorder->setText("start recorder");
        break;
    case QMediaRecorder::PausedState:
        m_recorder->setText("start recorder");
        break;
    case QMediaRecorder::RecordingState:
        m_recorder->setText("stop recorder");
        break;
    }
}

void cameraWidgets::updateRecordTime()
{
    qDebug() << "Recorded " << m_mediaRecorder->duration()/1000 << " sec";
    qDebug() << "Recorded format:" << m_mediaRecorder->containerFormat();
    qDebug() << "Recorded videoSettings:"  << m_mediaRecorder->videoSettings().codec();
}

void cameraWidgets::displayRecorderError()
{
    qDebug() << "Capture error:" << m_mediaRecorder->errorString();
}

void cameraWidgets::init()
{
    m_viewfinder = new QCameraViewfinder(this);
    m_capture = new QPushButton("Take Picture", this);
    m_capture->resize(100, 50);
    m_capture->setGeometry(10, 10, 100, 50);
    m_recorder = new QPushButton("start record", this);
    m_recorder->resize(100, 50);
    m_recorder->setGeometry(10, 10, 100, 50);
    

    connect(m_capture, SIGNAL(pressed()),this,SLOT(slot_pressed()));
    connect(m_capture, SIGNAL(clicked()),this,SLOT(slot_takepictrue()));
    connect(m_capture, SIGNAL(released()),this,SLOT(slot_released()));

    connect(m_recorder, SIGNAL(clicked()),this,SLOT(slot_recorder()));
}

void cameraWidgets::initLayout()
{
    QVBoxLayout *vmainlyout = new QVBoxLayout;

    m_topWid = new cameraTopWidgets(this);

    // 下半部分布局
    QVBoxLayout *hmiddlelyout = new QVBoxLayout;
    hmiddlelyout->addStretch(3);
    hmiddlelyout->addWidget(m_viewfinder);
    hmiddlelyout->addStretch(1);
    hmiddlelyout->addWidget(m_capture);
    hmiddlelyout->addStretch(1);
    hmiddlelyout->addWidget(m_recorder);
    hmiddlelyout->setContentsMargins(10,10,10,10);
    hmiddlelyout->setSpacing(0);


    vmainlyout->addWidget(m_topWid);
    vmainlyout->addLayout(hmiddlelyout);
    vmainlyout->addStretch(0);
    vmainlyout->setContentsMargins(0,0,0,0);
    vmainlyout->setSpacing(0);

    setLayout(vmainlyout);
}
