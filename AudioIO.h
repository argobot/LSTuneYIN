/*   Copyright 2010 Craig Eaton
 *
 *   This file is part of LSTune.
 *
 *    LSTune is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) 
 * any later version.
 *
 *   LSTune is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 *   You should have received a copy of the GNU General Public License along
 * with LSTune. If not, see http://www.gnu.org/licenses/.
 */ 

#ifndef AUDIOIO_H
#define AUDIOIO_H

#include <QWidget>
#include <QBuffer>
#include <QList>
#include <QAudioDevice>
#include <QAudioSource>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QTimer>

#include "filter.h"

class AudioIO : public QWidget
{
  Q_OBJECT

public:
  explicit AudioIO(int fsample, int inFrameRate = 30, QWidget *parent = nullptr);
  ~AudioIO() override;
  void setNotifyInterval(int ms);
  int getSampleRate() const;

public Q_SLOTS:
  qint64 getAudio(float *inBuffer, int maxSamples);
  QList<QAudioDevice> getDevices();
  void start();
  void stop();
  void switchDevice(int deviceIndex);

Q_SIGNALS:
  void notify();

private:
  void initHW(const QAudioDevice &inDevice);

  QAudioSource *audioSource;
  QBuffer *IODevice;
  QTimer *notifyTimer;

  int fSample;
  int frameRate;

  QList<QAudioDevice> devices;

  qint64 readPointer; 
  qint64 maxBufSize;
  bool started;

  int notifyInterval;
};

#endif // AUDIOIO_H