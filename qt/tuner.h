#ifndef TUNER_H
#define TUNER_H

#include <QObject>
#include <QWidget>
#include <QAudioInput>

#include "notes.h"
#include "sound.h"
#include "ui_tunerForm.h"

class AudioIO;

class Tuner : public QWidget
{
  Q_OBJECT;
public:
  Tuner(QWidget *parent = 0);
  ~Tuner();
  QList<QAudioDeviceInfo> getDevices();

public slots:
  void about();
  void switchDevice(int device);
  void setDisplayedNote(note_t *note);

private slots:
  void on_autoDetect_clicked (bool clicked);
  void on_noteSelBox_currentIndexChanged(int index);
  void on_A4SpinBox_valueChanged (double freq);


 private:
  AudioProc *aProc;
  AudioIO *soundIO;

  note_t *curNote;

  Ui::tunerForm ui;

};

#endif
