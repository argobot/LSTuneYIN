#include <QDebug>
#include "noteGuesser.h"

// --------------------------------------------------------------
const float CENTS_SHARP_THRESHOLD_ON = 12.0;
const float CENTS_SHARP_THRESHOLD_OFF = 8.0;

//---------------------------------------------------------------
noteGuesser::noteGuesser(notes_t *newNotes, float fsample)
{
  notes = newNotes;
  fSample = fsample;
  prevGuess = notes->get_note_offset(0);

  // Queue of the last guessQueueLen notes to appear
  peakTimes = QVector <int>(peakQueueLen, 0);
  peakVals = QVector <float>(peakQueueLen, 0.0);
  peakQueueHead = 0;
}

// ---------------------------------------------
// Add a new peak to the peak queue
// ---------------------------------------------
void noteGuesser::addFoundPeak(int t, float val)
{
  peakTimes[peakQueueHead] = t;
  peakVals[peakQueueHead] = val;

  peakQueueHead += 1;
  if (peakQueueHead >= peakQueueLen)
    peakQueueHead = 0;
}


// ---------------------------------------------
// Look at the queue, and guess which note we want
// ---------------------------------------------
note_t *noteGuesser::guessNotes(note_t *curNote, bool forceCurrent)
{
  float minFreq = notes->get_note_offset(0)->freq;

  // Find max value in queue
  float maxVal = -1e6;
  for (int n = 0; n < peakQueueLen; n++)
    if (peakVals[n] >= maxVal)
      maxVal = peakVals[n];
  float valScale = 1.0/maxVal; // For normalizing queue values
  // qDebug() << "Maxval = " << maxVal;

  // How often each note was found
  QVector <int>foundCount(notes->total_notes, 0); 
  // For finding average frequency. Keep running total of log of each found peak's frequency
  QVector <float>foundFreq(notes->total_notes, 0.0); 

  // Look at all peaks within 10% of the maximum peak.
  // Then figure out which note coresponds to said peak, and add up how many times
  // each note appears in queue
  const float bucketSize = 0.2;
  int peaksUsed = 0;
  int prevPeakTime = 0;
  bool firstPeak = true;  
  for (int n = 0; n < peakQueueLen; n++)
    {
      int queueIdx = (peakQueueHead + n) % peakQueueLen;

      if (valScale * peakVals[queueIdx] >= (1.0 - bucketSize))
	{
	  int t = peakTimes[queueIdx];
	  float freq = fSample/(float)(t - prevPeakTime);
	  if (!firstPeak && freq > minFreq)
	    {
	      note_t *foundNote = notes->find_nearest_freq(freq);
	      foundCount[foundNote->index] += 1;
	      foundFreq[foundNote->index] += logf(freq); // Store the log of the freq. Makes for better averages

	      peaksUsed += 1;
	    }
	  else
	    firstPeak = false;
	  prevPeakTime = t;
	}
    } // END for (int n = 0; n < peakQueueLen; n++)

  note_t *newNote = curNote;
  if (peaksUsed > 0)
    {
      // Figure out which note appears most frequently
      int totalCount = 0;
      int maxCount = 0;
      int maxIndex = 0;
      for (int n = 0; n < notes->total_notes; n++)
	{
	  int count = foundCount[n];
	  totalCount += count;
	  if (count >= maxCount) {
	    maxCount = count;
	    maxIndex = n;
	  }
	}

      // Only believe this if the most frequent note represents at least
      // half of the samples
      if (maxCount >= (totalCount*2)/3) 
	{
	  note_t *newGuess = notes->get_note_offset(maxIndex);

	  if (newGuess == prevGuess && maxCount >= totalCount/2)
	    {
	      if (!forceCurrent)
		newNote = newGuess;
	      else
		newNote = curNote;
	      
	      float avgFreq = expf(foundFreq[maxIndex]/foundCount[maxIndex]);
	      float centsOff = 1200.0 * log2f(avgFreq/newNote->freq);
	      sharp = (centsOff >= CENTS_SHARP_THRESHOLD_ON)  || (sharp && (centsOff >= CENTS_SHARP_THRESHOLD_OFF));
	      flat  = (centsOff <= -CENTS_SHARP_THRESHOLD_ON) || (flat &&  (centsOff <= -CENTS_SHARP_THRESHOLD_OFF));
	    }
	  prevGuess = newGuess;
	}
    } // END if (peaksUsed > 0)
  // qDebug() << "\tNewNote =" << newNote->fullName;
  return newNote;

}

bool noteGuesser::isSharp()
{
  return sharp;
}

bool noteGuesser::isFlat()
{
  return flat;
}
