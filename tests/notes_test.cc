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

#include <QtCore>
#include <QDebug>
#include <stdio.h>
#include <notes.h>


int main()
{
  notes_t mynotes(440.0);

  for (int n = 0; n < mynotes.total_notes; n++)
    {
      note_t *note = mynotes.get_note_offset(n);
      
      QString out = QString("%1 : %2 : %3") 
	.arg(note->fullName, 3).arg(note->freq, 7, 'f', 3).arg(note->a4_offset);
      qDebug() << out;
    } 
  return 0;
}
