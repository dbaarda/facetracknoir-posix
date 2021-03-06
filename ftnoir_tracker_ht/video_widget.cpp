/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "video_widget.h"

#include <QDebug>

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
    QMutexLocker lck(&mtx);
	if (!resized_qframe.isNull() && resized_qframe.size() == size())
        glDrawPixels(resized_qframe.width(), resized_qframe.height(), GL_RGB, GL_UNSIGNED_BYTE, resized_qframe.bits());
	glFlush();
}

void VideoWidget::resize_frame(QImage& qframe)
{
    QMutexLocker lck(&mtx);
    if (qframe.size() == size())
        resized_qframe = qframe.copy();
    else
        resized_qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation).copy();
}


void VideoWidget::updateImage(unsigned char *frame, int width, int height)
{
    QImage foo = QImage(frame, width, height, 3 * width, QImage::Format_RGB888).rgbSwapped().mirrored();
    resize_frame(foo);
}

void VideoWidget::update() {
    updateGL();
}
