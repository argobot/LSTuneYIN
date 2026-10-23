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

#ifndef NOTES_H
#define NOTES_H

#include <QString>
#include <QVector>
#include <QQueue>

struct note_t
{
  QString name;
  QString fullName;
  int octave;
  float freq;
  int a4_offset;
  int index;
};

class notes_t
{
private:
  float a4_freq;
  QVector <note_t>notes;

public:
  static const int step_min = -48;
  static const int step_max = 38;
  static const int notes_in_octave = 12;
  static const int total_notes = step_max - step_min + 1;

  notes_t(float new_a4_freq);
  ~notes_t();

  note_t *get_note_offset(int offset);
  void set_a4freq(float new_a4);

  // Finds nearest note by frequecny
  note_t *find_nearest_freq(float freq);
};


#endif
