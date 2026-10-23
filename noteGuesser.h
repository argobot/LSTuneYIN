#ifndef NOTEGUESSER_H
#define NOTEGUESSER_H

#include <QtCore>
#include "notes.h"

class noteGuesser
{
 private:
  notes_t *notes;
  note_t *prevGuess;
  float fSample;
  bool sharp, flat;

  // Length of queue used for note detection
  static const int peakQueueLen = 20;
  
  // Notes found, by index
  int peakQueueHead;
  QVector <int> peakTimes;
  QVector <float> peakVals;

 public:
  noteGuesser(notes_t *newNotes, float fsample);
  void addFoundPeak(int t, float val);
  void addFoundFreq(float freq);
  note_t *guessNotes(note_t *curNote, bool forceCurrent = false);
  bool isSharp();
  bool isFlat();


};

#endif
