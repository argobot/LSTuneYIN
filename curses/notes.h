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

struct note_t
{
  const char *name;
  int octave;
  float freq;
  int a4_offset;
};

class notes_t
{
private:
  
  float a4_freq;
  note_t *notes;

public:
  static const int step_min = -57;
  static const int step_max = 51;
  static const int notes_in_octave = 12;
  static const int total_notes = step_max - step_min + 1;

  notes_t(float new_a4_freq);
  ~notes_t();

  note_t get_note_offset(int offset);
  void set_a4freq(float new_a4);

  // Finds nearest note by frequecny
  note_t *find_nearest_freq(float freq);

  // Return frequency of note or -1.0
  float note_to_freq(const char *name);
};

