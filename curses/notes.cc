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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <notes.h>


// Finds nearest note by frequency
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

// Return frequency of note or -1.0
float notes_t::note_to_freq(const char *name) {
  for (int n = 0; n < total_notes; n++)
    {
      char tmp[9];
      sprintf(tmp, "%s%d", notes[n].name, notes[n].octave);
      //printf("Comparing name '%s' to note name '%s'\n", name, tmp);
      int rv = strcmp(tmp, name);
      printf(" %d\n", rv);
      if (rv == 0) {
    	  return notes[n].freq;
      }
    }
  return -1.0;
}

note_t notes_t::get_note_offset(int offset)
{
  if (offset > total_notes)
    offset = 0;
  return notes[offset];
}

void notes_t::set_a4freq(float new_a4)
{
  for (int step = step_min, n = 0; step <= step_max; step++, n++)
    {  
      note_t *this_note = &notes[n];
      this_note->freq  = a4_freq * pow(2.0, ((float)step)/notes_in_octave);      
    }
}

// Re-sets the A4 frequency.
notes_t::notes_t(float new_a4_freq)
{
  a4_freq = new_a4_freq;

  // Allocate memory for everything
  notes = new note_t[total_notes];

  
  const int reference_octave = 4;
  static const char* note_table_sharp[] = 
    {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
  //static const char* note_table_flat[]  = 
  //  {"A", "Bb", "B", "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab"};

  for (int step = step_min, n = 0; step <= step_max; step++, n++)
    {
      note_t *this_note = &notes[n];

      int spelling_offset;
      if (step >= 0) {
        spelling_offset = step % notes_in_octave;
        this_note->octave = (step-3)/notes_in_octave + reference_octave;
      }
      else {
        spelling_offset = 11 + ((step+1) % notes_in_octave);
        this_note->octave = (step-2)/notes_in_octave + reference_octave;
      }
      
      this_note->name = note_table_sharp[spelling_offset];
      this_note->a4_offset = step;
      this_note->freq  = a4_freq * pow(2.0, ((float)step)/notes_in_octave);      
    }
  // set_a4freq(a4_freq);
   
}

notes_t::~notes_t()
{
  delete notes;
}

