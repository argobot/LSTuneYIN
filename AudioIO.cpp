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

#include "AudioIO.h"
#include "noteGuesser.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <QDebug>
#include <QVarLengthArray>

const double maxBuffLength = 2.0;

AudioIO::AudioIO(int fsample, int inFrameRate, QWidget *parent)
  : QWidget(parent),
    audioSource(nullptr),
    IODevice(nullptr),
    notifyTimer(nullptr),
    fSample(fsample),
    frameRate(inFrameRate),
    readPointer(0),
    maxBufSize(0),
    started(false),
    notifyInterval(-1)
{
  IODevice = new QBuffer(this);
  if (!IODevice->open(QIODevice::ReadWrite)) {
    qWarning("Unable to open IODevice buffer");
  }

  notifyTimer = new QTimer(this);
  connect(notifyTimer, &QTimer::timeout, this, &AudioIO::notify);

  devices = QMediaDevices::audioInputs();
  initHW(QMediaDevices::defaultAudioInput());
}

AudioIO::~AudioIO()
{
  stop();

  delete audioSource;
  audioSource = nullptr;

  if (IODevice) {
    IODevice->close();
  }
}

void AudioIO::switchDevice(int deviceIndex)
{
  bool wasStarted = started;
  if (wasStarted) {
    stop();
  }

  devices = QMediaDevices::audioInputs();
  if (deviceIndex >= devices.size() || deviceIndex < 0) {
    qWarning() << "Error: invalid device index. must be between 0 and " << devices.size() - 1;
    exit(1);
  }

  const QAudioDevice &devInfo = devices.at(deviceIndex);
  initHW(devInfo);

  if (wasStarted) {
    start();
  }
}

void AudioIO::initHW(const QAudioDevice &inDevice)
{
  if (audioSource != nullptr) {
    stop();
    delete audioSource;
    audioSource = nullptr;
  }

  QAudioDevice info = inDevice;
  if (info.isNull()) {
    info = QMediaDevices::defaultAudioInput();
  }

  qDebug() << "Selected Device:" << info.description();

  QAudioFormat audioFormat;
  audioFormat.setChannelCount(1);
  audioFormat.setSampleRate(fSample);

#if defined(BUILD_WIN32) && BUILD_WIN32
  audioFormat.setSampleFormat(QAudioFormat::Int16);
#else
  audioFormat.setSampleFormat(QAudioFormat::Float);
#endif

  if (!info.isFormatSupported(audioFormat)) {
    qWarning() << "Default audio format not directly supported. Negotiating alternative format.";
    
    audioFormat.setSampleFormat(QAudioFormat::Int16);
    if (!info.isFormatSupported(audioFormat)) {
      audioFormat.setSampleFormat(QAudioFormat::Int32);
    }

    qWarning() << "Negotiated Format -> Channels:" << audioFormat.channelCount()
               << "Rate:" << audioFormat.sampleRate()
               << "SampleFormat:" << audioFormat.sampleFormat();
  }

  const QAudioFormat::SampleFormat sFormat = audioFormat.sampleFormat();
  if (sFormat != QAudioFormat::Float &&
      sFormat != QAudioFormat::Int16 &&
      sFormat != QAudioFormat::Int32) {
    qWarning("Only Float, Int16, or Int32 formats are supported by the DSP pipeline! Exiting.");
    exit(1);
  }

  audioSource = new QAudioSource(info, audioFormat, this);

#if defined(BUILD_LINUX) && BUILD_LINUX
  audioSource->setBufferSize((fSample * audioFormat.bytesPerSample()) / frameRate);
#endif

  maxBufSize = static_cast<qint64>(maxBuffLength * audioFormat.sampleRate() * audioFormat.bytesPerSample() * audioFormat.channelCount());
}

QList<QAudioDevice> AudioIO::getDevices()
{
  devices = QMediaDevices::audioInputs();
  return devices;
}

int AudioIO::getSampleRate() const
{
  if (audioSource) {
    return audioSource->format().sampleRate();
  }
  return fSample;
}

void AudioIO::setNotifyInterval(int ms)
{
  notifyInterval = ms;
  if (notifyInterval > 0) {
    notifyTimer->setInterval(notifyInterval);
  }
}

void AudioIO::start()
{
  if (!audioSource) return;

  IODevice->seek(0);
  IODevice->buffer().clear();
  readPointer = 0; 

  audioSource->start(IODevice);
  
  if (notifyInterval > 0) {
    notifyTimer->start(notifyInterval);
  }
  
  started = true;
}

void AudioIO::stop()
{
  if (notifyTimer->isActive()) {
    notifyTimer->stop();
  }

  if (audioSource) {
    audioSource->stop();
  }

  started = false;
}

qint64 AudioIO::getAudio(float *inBuffer, int maxSamples)
{
  if (!audioSource) return 0;

  const QAudioFormat format = audioSource->format();
  const qint64 bytesPerSample = format.bytesPerSample();
  const qint64 channelCount = format.channelCount();
  const qint64 frameSize = bytesPerSample * channelCount;

  if (frameSize == 0) return 0;

  const qint64 writePointer = IODevice->pos();
  qint64 bytesToRead = writePointer - readPointer;
  qint64 samplesToRead = bytesToRead / frameSize;

  bool readAll = true;
  if (samplesToRead > maxSamples) {
    samplesToRead = maxSamples;
    bytesToRead = maxSamples * frameSize;
    readAll = false;
  }

  IODevice->seek(readPointer);
  qint64 readSamples = 0;

  if (format.sampleFormat() == QAudioFormat::Float) {
    readSamples = IODevice->read(reinterpret_cast<char*>(inBuffer), bytesToRead) / frameSize;
  } 
  else if (format.sampleFormat() == QAudioFormat::Int16) {
    constexpr float scale16 = 1.0f / 32768.0f;
    QVarLengthArray<qint16, 4096> int16Buf(samplesToRead);

    const qint64 bytesRead = IODevice->read(reinterpret_cast<char*>(int16Buf.data()), bytesToRead);
    readSamples = bytesRead / frameSize;

    for (qint64 i = 0; i < readSamples; ++i) {
      inBuffer[i] = static_cast<float>(int16Buf[i]) * scale16;
    }
  } 
  else if (format.sampleFormat() == QAudioFormat::Int32) {
    constexpr float scale32 = 1.0f / 2147483648.0f;
    QVarLengthArray<qint32, 4096> int32Buf(samplesToRead);

    const qint64 bytesRead = IODevice->read(reinterpret_cast<char*>(int32Buf.data()), bytesToRead);
    readSamples = bytesRead / frameSize;

    for (qint64 i = 0; i < readSamples; ++i) {
      inBuffer[i] = static_cast<float>(int32Buf[i]) * scale32;
    }
  }

  if (readAll || IODevice->size() > maxBufSize) {
    IODevice->seek(0);
    IODevice->buffer().clear();
    readPointer = 0;
  } else {
    readPointer += bytesToRead;
    IODevice->seek(writePointer);
  }

  return readSamples;
}