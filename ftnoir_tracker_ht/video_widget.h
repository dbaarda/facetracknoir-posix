/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QGLWidget>
#include <QTime>
#include <opencv2/opencv.hpp>
#include <QFrame>
#include <QImage>
#include <QWidget>
// ----------------------------------------------------------------------------
class VideoWidget : public QGLWidget
{
	Q_OBJECT

public:
	VideoWidget(QWidget *parent) : QGLWidget(parent) {
		setAttribute(Qt::WA_NativeWindow, true);
	}

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

    void update(unsigned char* frame, int width, int height);

private:
    void resize_frame(QImage& qframe);
    QImage resized_qframe;
};

#endif // VIDEOWIDGET_H
