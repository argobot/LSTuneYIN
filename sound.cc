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

#include "sound.h"

#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <QtDebug>

#include "AudioIO.h"
#include "noteGuesser.h"

//--------------------------------------
// Constants
//--------------------------------------
const double DEFAULT_HYSTERESIS = 0.005;
const int FRAMERATE = 30;
const int SAFTEYMARGIN = 2;

//--------------------------------------
AudioProc::AudioProc(int fsample, notes_t *inotes, QWidget *parent)
  : QWidget(parent),
    soundIO(nullptr),
    notes(inotes),
    curNote(nullptr),
    guesser(nullptr),
    noteDetect(true),
    hysteresis(DEFAULT_HYSTERESIS),
    frames(0),
    maxFrames(0),
    filter(nullptr),
    fTime(0),
    lastUpdate(0),
    prevRise(0),
    wheelFreq(440.0),
    fSample(fsample),
    tSample(1.0 / static_cast<double>(fsample)),
    yinBufferSize(1024),
    yinThreshold(0.15f),
    freqHistory(5, 0.0f),
    freqHistoryIdx(0)
{
  yinBuffer.resize(yinBufferSize);

  guesser = new noteGuesser(notes, static_cast<float>(fSample));
  curNote = notes->get_note_offset(0);

  soundIO = new AudioIO(fsample, FRAMERATE);
  frames = static_cast<qint64>(fsample) / FRAMERATE;
  maxFrames = frames * SAFTEYMARGIN;

  soundIO->setNotifyInterval(1000 * 2 / (3 * FRAMERATE));
  connect(soundIO, &AudioIO::notify, this, &AudioProc::readAudio);

  filter = new dtFilter(static_cast<int>(maxFrames));
}

AudioProc::~AudioProc()
{
  stop();
  delete guesser;
  delete soundIO;
  delete filter;
}

AudioIO *AudioProc::getAudioIO()
{
  return soundIO;
}

int AudioProc::getSampleRate() const
{
  if (soundIO) {
    return soundIO->getSampleRate();
  }
  return 0;
}

void AudioProc::start()
{
  soundIO->start();
}

void AudioProc::stop()
{
  soundIO->stop();
}

void AudioProc::setWheelFreq(double newFreq)
{
  wheelFreq = newFreq;
}

void AudioProc::setNoteDetect(bool state)
{
  noteDetect = state;
}

void AudioProc::setHysteresis(double newValue)
{
  hysteresis = newValue;
}

void AudioProc::setA4Freq(double newValue)
{
  notes->set_a4freq(newValue);
  setWheelFreq(curNote->freq);
}

note_t *AudioProc::getCurNote()
{
  return curNote;
}

void AudioProc::readAudio()
{
  float *inBuffer = filter->get_inbuf();
  float *outBuffer = filter->get_outbuf();

  qint64 readFrames = soundIO->getAudio(inBuffer, maxFrames);
  if (readFrames <= 0) {
    return;
  }

  filter->filter(readFrames);

  float rms = 0.0f;
  float maxSignal = 0.0f;
  for (qint64 i = 0; i < readFrames; ++i) {
    float absVal = std::abs(outBuffer[i]);
    if (absVal > maxSignal) {
      maxSignal = absVal;
    }
    rms += outBuffer[i] * outBuffer[i];
  }
  rms = std::sqrt(rms / static_cast<float>(readFrames));

  int maxNoUpdate = fSample / 10;
  float detectedFreq = -1.0f;
  int tauSelected = -1;

  if (rms >= hysteresis && readFrames >= (yinBufferSize * 2)) {
    // =========================================================================
    // Normalización de Bloque (AGC)
    // =========================================================================
    const int agcSamples = yinBufferSize * 2;
    float maxAbs = 0.0f;
    for (int i = 0; i < agcSamples; ++i) {
      float absVal = std::abs(outBuffer[i]);
      if (absVal > maxAbs) {
        maxAbs = absVal;
      }
    }

    if (maxAbs > 1e-5f) {
      float invMax = 1.0f / maxAbs;
      for (int i = 0; i < agcSamples; ++i) {
        outBuffer[i] *= invMax;
      }
    }

    // =========================================================================
    // Algoritmo YIN
    // =========================================================================

    // Paso 1: Diferencia cuadrática
    for (int tau = 0; tau < yinBufferSize; ++tau) {
      yinBuffer[tau] = 0.0f;
      for (int i = 0; i < yinBufferSize; ++i) {
        float delta = outBuffer[i] - outBuffer[i + tau];
        yinBuffer[tau] += delta * delta;
      }
    }

    // Paso 2: CMND
    yinBuffer[0] = 1.0f;
    float runningSum = 0.0f;
    for (int tau = 1; tau < yinBufferSize; ++tau) {
      runningSum += yinBuffer[tau];
      if (runningSum > 0.0f) {
        yinBuffer[tau] *= static_cast<float>(tau) / runningSum;
      } else {
        yinBuffer[tau] = 1.0f;
      }
    }

    // Paso 3: Umbral absoluto
    for (int tau = 2; tau < yinBufferSize; ++tau) {
      if (yinBuffer[tau] < yinThreshold) {
        while (tau + 1 < yinBufferSize && yinBuffer[tau + 1] < yinBuffer[tau]) {
          tau++;
        }
        tauSelected = tau;
        break;
      }
    }

    // Mínimo global si no cruza el umbral
    if (tauSelected == -1) {
      float minVal = yinBuffer[2];
      tauSelected = 2;
      for (int tau = 3; tau < yinBufferSize; ++tau) {
        if (yinBuffer[tau] < minVal) {
          minVal = yinBuffer[tau];
          tauSelected = tau;
        }
      }
    }

    // Paso 4: Interpolación parabólica y compuerta de confianza
    if (tauSelected > 0 && yinBuffer[tauSelected] <= yinThreshold) {
      float betterTau = static_cast<float>(tauSelected);
      if (tauSelected < yinBufferSize - 1) {
        float s0 = yinBuffer[tauSelected - 1];
        float s1 = yinBuffer[tauSelected];
        float s2 = yinBuffer[tauSelected + 1];
        float bottom = 2.0f * (2.0f * s1 - s2 - s0);
        if (std::abs(bottom) > 1e-6f) {
          betterTau = static_cast<float>(tauSelected) + (s2 - s0) / bottom;
        }
      }

      if (betterTau > 0.0f) {
        float candidateFreq = static_cast<float>(fSample) / betterTau;
        if (candidateFreq >= 30.0f && candidateFreq <= 2000.0f) {
          detectedFreq = candidateFreq;
        }
      }
    }
  }

  // =========================================================================
  // Filtro de Mediana (Orden 5)
  // =========================================================================
  float medianFreq = -1.0f;

  if (detectedFreq > 0.0f) {
    freqHistory[freqHistoryIdx] = detectedFreq;
    freqHistoryIdx = (freqHistoryIdx + 1) % 5;

    std::vector<float> validSamples;
    validSamples.reserve(5);
    for (float f : freqHistory) {
      if (f > 0.0f) {
        validSamples.push_back(f);
      }
    }

    if (!validSamples.empty()) {
      std::sort(validSamples.begin(), validSamples.end());
      medianFreq = validSamples[validSamples.size() / 2];
    }
  } else {
    // Purga gradual del filtro en ausencia de señal
    freqHistory[freqHistoryIdx] = 0.0f;
    freqHistoryIdx = (freqHistoryIdx + 1) % 5;
  }

  // =========================================================================
  // Integración y Renderizado Estroboscópico (Wheel)
  // =========================================================================
  if (medianFreq > 0.0f) {
    int t = static_cast<int>(fTime);
    double wheelRemainder = std::fmod(static_cast<double>(t) * tSample, 1.0 / wheelFreq);
    double wheelPos = wheelRemainder * wheelFreq * 2.0 * M_PI;
    float deltaT = static_cast<float>(t - prevRise) * static_cast<float>(tSample);

    emit newAngle(static_cast<float>(wheelPos), deltaT);
    prevRise = lastUpdate = t;

    guesser->addFoundPeak(t, medianFreq);
  }

  fTime += readFrames;
  if (fTime > 10000000) {
    fTime = 0;
  }

  // =========================================================================
  // Mapeo Musical y Emisión Directa a la GUI
  // =========================================================================
  if (medianFreq > 0.0f) {
    if (noteDetect) {
      // Modo automático: encontrar la nota más cercana en frecuencia fundamental
      note_t *bestNote = nullptr;
      double minDiff = 1e9;

      for (int n = 0; n < notes->total_notes; ++n) {
        note_t *cand = notes->get_note_offset(n);
        if (cand && cand->freq > 0.0) {
          double diff = std::abs(std::log2(static_cast<double>(medianFreq) / cand->freq));
          if (diff < minDiff) {
            minDiff = diff;
            bestNote = cand;
          }
        }
      }

      if (bestNote != nullptr && bestNote != curNote) {
        curNote = bestNote;
        setWheelFreq(curNote->freq);
        qDebug() << "[FASE 4 - Emisión GUI] Nueva nota detectada:" << curNote->name 
                 << "| Freq objetivo:" << curNote->freq << "Hz | Freq entrada:" << medianFreq << "Hz";
        emit changedNote(curNote);
      }
    }

    // Indicación de afinación Sharp / Flat relativa a curNote
    if (curNote && curNote->freq > 0.0) {
      double cents = 1200.0 * std::log2(static_cast<double>(medianFreq) / curNote->freq);
      constexpr double TUNE_TOLERANCE_CENTS = 3.0;

      if (cents < -TUNE_TOLERANCE_CENTS) {
        emit isFlat(true);
        emit isSharp(false);
      } else if (cents > TUNE_TOLERANCE_CENTS) {
        emit isFlat(false);
        emit isSharp(true);
      } else {
        emit isFlat(false);
        emit isSharp(false);
      }
    }
  } else {
    // Apagar indicadores si no hay señal o cae bajo la puerta
    emit isFlat(false);
    emit isSharp(false);
  }

  if ((fTime - lastUpdate) > maxNoUpdate) {
    emit newAngle(-100.0f, static_cast<float>(fTime - lastUpdate) * static_cast<float>(tSample));
    lastUpdate = static_cast<int>(fTime);
  }
}