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

#include "tuner.h"

#include <QTimer>
#include <QLabel>
#include <QMessageBox>
#include <QVariant>
#include <QString>

#include "sound.h"
#include "wheel.h"
#include "notes.h"
#include "AudioIO.h"
#include "LED.h"

const int fsample = 44100;
const double A4_DEFAULT = 440.0;

notes_t notes(A4_DEFAULT);

Tuner::Tuner(QWidget *parent)
  : QWidget(parent),
    aProc(nullptr),
    soundIO(nullptr),
    curNote(nullptr)
{
  ui.setupUi(this);   

  Wheel *sWheel = new Wheel(ui.strobeFrame);
  
  LED *flatLED = new LED(this);
  LED *sharpLED = new LED(this);
  flatLED->setState(false);
  sharpLED->setState(false);

  QLabel *flatLabel = new QLabel("<<<< Flat <<<<", this);
  QLabel *sharpLabel = new QLabel(">>>> Sharp >>>>", this);
  sharpLabel->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

  ui.sharpFlatLayout->addWidget(flatLED);
  ui.sharpFlatLayout->addWidget(flatLabel);
  ui.sharpFlatLayout->addWidget(sharpLabel);
  ui.sharpFlatLayout->addWidget(sharpLED);

  aProc = new AudioProc(fsample, &notes);
  soundIO = aProc->getAudioIO();

  connect(aProc, &AudioProc::newAngle, sWheel, &Wheel::setAnglef);
  connect(aProc, &AudioProc::changedNote, this, &Tuner::setDisplayedNote);

  connect(aProc, &AudioProc::isSharp, sharpLED, &LED::setState);
  connect(aProc, &AudioProc::isFlat, flatLED, &LED::setState);

  connect(ui.autoDetect, &QAbstractButton::clicked, aProc, &AudioProc::setNoteDetect);
  connect(ui.A4SpinBox, &QDoubleSpinBox::valueChanged, aProc, &AudioProc::setA4Freq);
  connect(ui.noiseGateSpinBox, &QDoubleSpinBox::valueChanged, aProc, &AudioProc::setHysteresis);

  for (int n = 0; n < notes.total_notes; n++) {
    ui.noteSelBox->addItem(notes.get_note_offset(n)->fullName, QVariant(n));
  }
  
  aProc->start();
// Retardo de 500ms para asegurar que PipeWire/ALSA haya inicializado el QAudioSource
    QTimer::singleShot(500, this, [this]() {
    const int actualSampleRate = aProc->getSampleRate();
    ui.lblSampleRate->setText(QString("Capturando a %1 Hz").arg(actualSampleRate));
    //this->setWindowTitle(QString("LSTune - Capturando a %1 Hz").arg(actualSampleRate));
});
}

Tuner::~Tuner()
{
  delete aProc;
}

void Tuner::switchDevice(int device)
{
  soundIO->switchDevice(device);
}

QList<QAudioDevice> Tuner::getDevices()
{
  return soundIO->getDevices();
}

void Tuner::setDisplayedNote(note_t *note)
{
  curNote = note;
  QString nname = curNote->name;
  if (nname.size() == 1) {
    nname.append(QString(" "));
  }
  ui.noteDisp->setText(nname);
  ui.freqDisp->display(curNote->freq);
  aProc->setWheelFreq(note->freq);
}

void Tuner::on_autoDetect_clicked(bool clicked)
{
  ui.noteSelBox->setEnabled(clicked);
  
  if (clicked) {
    int curIndex = ui.noteSelBox->currentIndex();
    int noteIdx = ui.noteSelBox->itemData(curIndex).toInt();
    note_t *note = notes.get_note_offset(noteIdx);
    setDisplayedNote(note);
  } else {
    setDisplayedNote(aProc->getCurNote());
  }
}

void Tuner::on_A4SpinBox_valueChanged(double freq)
{
  aProc->setA4Freq(freq);
  setDisplayedNote(curNote);
}

void Tuner::on_noteSelBox_currentIndexChanged(int index)
{
  int noteIdx = ui.noteSelBox->itemData(index).toInt();
  note_t *note = notes.get_note_offset(noteIdx);
  setDisplayedNote(note);
}

void Tuner::about()
{
  QMessageBox::about(this, tr("About"), 
                     tr("LS Tune Strobe Tuner, by Craig Eaton\n\n"
                        "   LS Tune is free software: you can redistribute it and/or modify it under"
                        " the terms of the GNU General Public License as published by the Free"
                        " Software Foundation, either version 3 of the License, or (at your option)"
                        " any later version."
                        "\n\n"
                        "   LS Tune is distributed in the hope that it will be useful, but"
                        " WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY"
                        " or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License"
                        " for more details."
                        "\n\n"
                        "   You should have received a copy of the GNU General Public License along"
                        " with LS Tune. If not, see http://www.gnu.org/licenses/."));
}