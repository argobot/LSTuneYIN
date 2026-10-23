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

#include <math.h>
#include <QString>
#include <QVector>
#include <QDebug>

#include <sstream>

#include "notes.h"

using namespace std;

// Finds nearest note by frequecny
note_t *notes_t::find_nearest_freq(float freq)
{
  // This could be done with a more efficient bisection search, or even
  // by calculating the nearest fequency. But whatever.
  note_t *closest = NULL;
  float closest_freq = -1e9;
  for (int n = 0; n < total_notes; n++)
    {
      float this_freq = notes[n].freq;
      if (fabs(this_freq - freq) <= fabs(closest_freq - freq)) {
	closest = &notes[n];
	closest_freq = this_freq;
      }
    }
      
  return closest;
}

note_t *notes_t::get_note_offset(int offset)
{
  if (offset > total_notes || offset < 0)
    offset = 0;
  return &notes[offset];
}

void notes_t::set_a4freq(float new_a4)
{
  a4_freq = new_a4;
  for (int step = step_min, n = 0; step <= step_max; step++, n++)
    {  
      note_t *this_note = &notes[n];
      this_note->freq  = a4_freq * powl(2.0, ((float)step)/notes_in_octave);      
    }
}

// Re-sets the A4 frequency.
notes_t::notes_t(float new_a4_freq)
{
  a4_freq = new_a4_freq;

  // Allocate memory for everything
  notes = QVector <note_t>(total_notes);

  const int reference_octave = 4;
  QString note_table_sharp[] = 
    {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
  //string note_table_flat[]  = 
  //  {"A", "Bb", "B", "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab"};

  for (int step = step_min, n = 0; step <= step_max; step++, n++)
    {
      int spelling_offset;
      if (step >= 0) {
        spelling_offset = step % notes_in_octave;
        notes[n].octave = (step+9)/notes_in_octave + reference_octave;
      }
      else {
        spelling_offset = 11 + ((step+1) % notes_in_octave);
        notes[n].octave = (step-2)/notes_in_octave + reference_octave;
      }
      
      notes[n].name = note_table_sharp[spelling_offset];
      notes[n].fullName = QString("%1%2")
	.arg(note_table_sharp[spelling_offset])
	.arg(notes[n].octave);
      notes[n].a4_offset = step;
      notes[n].freq  = a4_freq * powl(2.0, ((float)step)/notes_in_octave);      
      notes[n].index = n;
    }
  // set_a4freq(a4_freq);
   
}

notes_t::~notes_t()
{
  ;
  //delete notes;
}

