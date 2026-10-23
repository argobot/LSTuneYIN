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
#include <filter.h>

fltType *dtFilter::get_inbuf()
{
  return &inbuf[order];
}

fltType *dtFilter::get_outbuf()
{
  return &outbuf[order];
}

int dtFilter::filter(int toFilter)
{
  // A coefficients for a band pass filter with a 44100Hz sample frequency
  // Consists of a 1st order high pass filter placed at 20Hz,
  // and a 6th order butterworth low pass filter at 4kHz
  // This results in a 7th order system.
  // Coefficients for x(k), x(k-1), .... x(k-n)
//   const fltType cx[] = {6.458708e-03, 3.229354e-02, 5.812837e-02,  3.229354e-02, 
//                      -3.229354e-02, -5.812837e-02, -3.229354e-02, -6.458708e-03};
//   // Coefficients for y(k), y(k-1), .... y(k-n)
//   const fltType cy[] = {0.0,         -2.624508e+00, 3.394598e+00, -2.828718e+00, 
//                      1.476595e+00, -5.052747e-01,  9.840300e-02, -8.738424e-03};

  // Alternate filter. First order 10Hz high pass + 8 order butterworth LP at
  // 3kHz
  const fltType cx[] = {1.219565e-03, 8.536952e-03, 2.439129e-02, 3.414781e-02,
                        1.707390e-02, -1.707390e-02, -3.414781e-02, -2.439129e-02,
                        -8.536952e-03, -1.219565e-03};
  const fltType cy[] = {0, -3.179958e+00, 5.166898e+00, -5.512706e+00, 
                        4.023904e+00, -2.096830e+00, 7.595685e-01, -1.853689e-01, 
                        2.722328e-02, -1.840134e-03};
   
  // Actually do the filtering
  int in_bounds = order + toFilter;
  for (int k = order; k <= in_bounds; k++)
    {
      fltType this_y = 0.0;

      for (int n = 0; n <= order; n++)
        this_y += cx[n]*inbuf[k - n] - cy[n]*outbuf[k-n];
      outbuf[k] = this_y;
    }

  // Put the last n samples into the first n positions.
  for (int k = 0; k <= order; k++)
    {
      inbuf[k] = inbuf[toFilter + k];
      outbuf[k] = outbuf[toFilter + k];
    }

  return 0;
}

dtFilter::~dtFilter()
{
  delete inbuf;
  delete outbuf;
}

dtFilter::dtFilter(int buf_length)
{
  buffer_length = buf_length + order;
  inbuf = new fltType[buffer_length];
  outbuf = new fltType[buffer_length];

  // Just for simplicity, zero out the buffers
  for (int n = 0; n < buffer_length; n++) 
      inbuf[n] = outbuf[n] = 0.0;
}

