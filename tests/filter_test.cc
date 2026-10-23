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

#include <stdio.h>
#include <math.h>
#include "filter.h"

int main()
{

  int fsample = 44100;
  int t_buffer = 10; // 10 seconds
  int samples = fsample * t_buffer;
  dtFilter myfilter(samples);

  fltType *inbuf  = myfilter.get_inbuf();
  fltType *outbuf = myfilter.get_outbuf();

  // Fill up buffer
  for (int s = 0; s < samples; s++)
    {
      double t = (double)t_buffer * (double)s/(double)samples;
      double f = 15000.0 * (double)s/(double)samples;
      inbuf[s] = (fltType) sin(2.0 * M_PI * f * t);
    }

  // Run filter
  myfilter.filter(samples);
  for (int s = 0; s < samples; s++)
    printf ("%f\n", outbuf[s]);

  return 0;

}
