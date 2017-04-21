#ifndef CAMERAWIDGETS_H
#define CAMERAWIDGETS_H

#include "basewidget.h"
#include "cameratopwidgets.h"

#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QPushButton>
#include <QMediaRecorder>

class cameraWidgets:public baseWidget
{
	Q_OBJECT
public:
    cameraWidgets(QWidget *parent=0);
	~cameraWidgets();

	void closeCamera();
	void openCamera();

    cameraTopWidgets *m_topWid;
private:
    QCamera *m_camera;
    QCameraViewfinder *m_viewfinder;
    QCameraImageCapture *m_imageCapture;
	QMediaRecorder *m_mediaRecorder;
	QPushButton *m_capture;
	QPushButton *m_recorder;

    void init();
    void initLayout();

private slots:
	void slot_pressed();
	void slot_takepictrue();
 	void readyForCapture(bool ready);
	void processCapturedImage(int requestId, const QImage& img);
	void imageSaved(int id,const QString &filename);
	void slot_released();
	void slot_captureerror(int id, QCameraImageCapture::Error error, const QString &errorString);
	void slot_cameraerror(QCamera::Error value);

	void slot_recorder();
	void updateRecorderState(QMediaRecorder::State state);
	void updateRecordTime();
	void displayRecorderError();
};

#endif // CAMERAWIDGETS_H
