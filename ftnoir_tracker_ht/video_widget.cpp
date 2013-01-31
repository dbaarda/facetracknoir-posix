/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "video_widget.h"

#include <QDebug>

using namespace cv;
using namespace std;

// ----------------------------------------------------------------------------
void VideoWidget::initializeGL()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void VideoWidget::resizeGL(int w, int h)
{
	// setup 1 to 1 projection
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);
    resize_frame(resized_qframe);
}

void VideoWidget::paintGL()
{
    //glClear(GL_COLOR_BUFFER_BIT);
	if (!resized_qframe.isNull())
        glDrawPixels(resized_qframe.width(), resized_qframe.height(), GL_RGB, GL_UNSIGNED_BYTE, resized_qframe.bits());
	glFlush();
}

void VideoWidget::resize_frame(QImage& qframe)
{
	if (!qframe.isNull())
		resized_qframe = qframe.scaled(this->size(), Qt::KeepAspectRatio);
}


void VideoWidget::update(unsigned char *frame, int width, int height)
{
    QImage foo = QImage(frame, width, height, 3 * width, QImage::Format_RGB888).rgbSwapped().mirrored();
    resize_frame(foo);
	updateGL();
}
