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


/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <stdio.h>
#include <filter.h>
#include <ncurses.h>
#include <math.h>
#include <notes.h>

// -----------------------------------------------
// Initialize screen for curses junk
// -----------------------------------------------
void init_screen()
{
  initscr(); // Start curses mode
  //raw(); // Disable line buffering
  cbreak(); // Disable line buffering, keep ctl-c and ctl-z working.
  keypad(stdscr, TRUE); // Get control characters (F1, F2, etc)
  noecho(); // DOn't echo keyboard input
  curs_set(0); // Make cursor invisible
}

//--------------------------------------------------------
// Create an array of text strings to represent the 
// wheel in each position
//--------------------------------------------------------
char *init_wheel(char *wheel, int segments, int pos)
{
  for (int n = 0; n < segments; n++)
    {
      int str_pos = (n+pos) % segments;
      if (2*n < segments) 
        wheel[str_pos] = '*';
      else 
        wheel[str_pos] = ' ';
    }
  wheel[segments] = '\0';

  return wheel;
}

char **gen_wheel_strings(int segments)
{
  char **wheel = new char*[segments];
  char scratch1[segments+1];
  char scratch2[segments+1];
  char scratch3[segments+1];

  if ((segments % 4) != 0) {
    fprintf(stderr, "Error: Number of wheel segments must be divisible by 8\n");
    exit(1);
  }

  const int lines = 3;
  for (int pos = 0; pos < segments; pos++)
    {
      // This make wheel move the the right when note is sharp 
      int idx = segments - 1 - pos;
      int buflen = (segments*lines) + lines;
      wheel[idx] = new char[buflen];

      init_wheel(scratch1, segments, pos);    // Fundamental
      init_wheel(scratch2, segments>>1, pos); // 1st harmonic (octave)
      init_wheel(scratch3, segments/3, pos);  // 2nd harmonic (5th)

      snprintf(wheel[idx], sizeof(char) * buflen, "%s\n%s%s\n%s%s%s",
               scratch1, scratch2, scratch2, scratch3,scratch3,scratch3);
    }
  
  return wheel;
}

//--------------------------------------------------------

inline void print_wheel(char *s)
{
  mvprintw (1, 0, "Wheel\n-------------------------------------\n%s\n------------------------------------", s);
  refresh();
}


inline void print_wheel_freq(float fwheel)
{
  mvprintw(0, 0, "Wheel frequency = %f", fwheel);
}
//--------------------------------------------------------

static int set_hwparams(snd_pcm_t *handle,
                        snd_pcm_hw_params_t *params,
                        snd_pcm_access_t access,
                        snd_pcm_format_t format,
                        unsigned int rate)
{
  int err, dir;


  /* choose all parameters */
  err = snd_pcm_hw_params_any(handle, params);
  if (err < 0) {
    printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
    return err;
  }

  /* set the interleaved read/write format */
  err = snd_pcm_hw_params_set_access (handle, params, access);
  if (err < 0) {
    printf("Access type not available for playback: %s\n", snd_strerror(err));
    return err;
  }

  /* set the sample format */
  err = snd_pcm_hw_params_set_format(handle, params, format);
  if (err < 0) {
    printf("Sample format not available for playback: %s\n", snd_strerror(err));
    return err;
  }

  // Set the count of channels. Only do mono for now.
  int channels = 1;
  err = snd_pcm_hw_params_set_channels(handle, params, channels);
  if (err < 0) {
    printf("Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
    return err;
  }
  
  /* set the stream rate */
  unsigned int rrate = rate;
  err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
  if (err < 0) {
    printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
    return err;
  }
  if (rrate != rate) {
    printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
    return -EINVAL;
  }


  // Set period size to roughly 1/30th of a second
  snd_pcm_uframes_t frames = rate/30;
  err = snd_pcm_hw_params_set_period_size_near (handle, params, &frames, &dir);
  if (err < 0) {
    printf("Unable to set period size %i: %s\n", (int) frames, snd_strerror(err));
    return err;
  }

  /* write the parameters to device */
  err = snd_pcm_hw_params(handle, params);
  if (err < 0) {
    printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
    return err;
  }

  return 0;
}


snd_pcm_t *init_sound(snd_pcm_uframes_t *frames, int fsample)
{
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;

  // -------------------------------------------------------
  // Initialize sound junk;
  // -------------------------------------------------------
  /* Open PCM device for recording (capture). */
  int err = snd_pcm_open (&handle, "default", SND_PCM_STREAM_CAPTURE, 0);
  if (err < 0) {
    fprintf(stderr, "Unable to open pcm device: %s\n", snd_strerror(err));
    exit(1);
  }

  // Allocate a hardware parameters object.
  snd_pcm_hw_params_alloca (&params);

  // Initialize hardware
  set_hwparams(handle, params,
               SND_PCM_ACCESS_RW_INTERLEAVED, // Access type
               SND_PCM_FORMAT_FLOAT_LE, // Format. Little endian float
               fsample); // Rate. CD quality audio

  // Get number of frames, for filter buffer allocation
  snd_pcm_uframes_t uframes;
  int dir;
  snd_pcm_hw_params_get_period_size (params, &uframes, &dir);
  *frames = uframes;

  return handle;
}

//--------------------------------------------------------
// This example reads from the default PCM device
// and writes to standard output for 5 seconds of data.
int main (int argc, char **argv)
{
  bool auto_adjust = true; // DO we automatically select the frequency?
  const int fsample = 44100; // Sample frequency
  const float tsample = 1.0/(float)fsample;
  
  notes_t mynotes(440.0); // List of notes. A4 = 440Hz

  // Read wheel frequecny from command line
  double fwheel = 110.0;
  if (argc > 1) {
    auto_adjust = false;
    // Assume note is a string.  If not, assume it is a float
    fwheel = mynotes.note_to_freq(argv[1]);
    if (fwheel == -1.0) {
      fwheel = atof(argv[1]);
    }
  }

  // -------------------------------------------------------
  // Initialize sound junk;
  // -------------------------------------------------------
  snd_pcm_uframes_t frames;  // Get number of frames for filter buffer allocation
  snd_pcm_t *handle = init_sound(&frames, fsample);

  // Create filter
  dtFilter filter(frames);  // frames number of floats, 1 channel 
  float *inbuffer = filter.get_inbuf(); // Filter input buffer
  float *outbuffer = filter.get_outbuf(); // Filter output buffer

  // ----------------------------------------------------------
  // Init curses stuff 
  // ----------------------------------------------------------
  init_screen();

  // ----------------------------------------------------------
  // Initialize wheel
  // ----------------------------------------------------------
  const int WHEEL_SEGMENTS = 64;
  // Holds various text strings for wheel positions
  char **wheel = gen_wheel_strings(WHEEL_SEGMENTS);

  // Hysteresis, as fraction of full scale value
  const float hysteresis = 0.05;
  const float neg_hysteresis = 0.0 - hysteresis;
  bool strobe_state = false;

  mvprintw (7, 0, " -> -> Sharp -> ->      <- <- Flat <-<-"); 

  print_wheel_freq(fwheel);
  print_wheel(wheel[0]); // Print wheel in position zero
  int ftime = 0; // Time in number of frames since start of program
  int prev_rise = 0; // For calculation of frequency
  //int total_read_frames = 0;
  while (1)
    {
      int read_frames = snd_pcm_readi (handle, (char *) inbuffer, frames);
      //mvprintw(12, 0, "Read frame: %d\n", total_read_frames++);
      //refresh();

      if (read_frames == -EPIPE)        {
        /* EPIPE means overrun */
        fprintf (stderr, "overrun occurred\n");
        snd_pcm_prepare (handle);
      }
      else if (read_frames < 0)
        fprintf (stderr, "error from read: %s\n", snd_strerror (read_frames));
      else if (read_frames != (int) frames)
        fprintf (stderr, "short read, read %d frames\n", read_frames);

      // Process buffer
      filter.filter(read_frames);
      for (int i = 0; i < read_frames; i++)
        {
          //float val = buffer[i] + buffer[i+1];
          float val = outbuffer[i];

          if (strobe_state and val <= neg_hysteresis)
            strobe_state = false;
          else if (!strobe_state and val >= hysteresis)
            {
              strobe_state = true;
              
              int t = ftime + i;
              float input_freq = 1/((float)(t - prev_rise) * tsample);

              // Calculate position of wheel, print it up
              double wheel_remainder = fmod((double)t * tsample, 1/fwheel);
              int wheel_pos = (int) floor(wheel_remainder * (double)fwheel * (double)WHEEL_SEGMENTS);
              print_wheel(wheel[wheel_pos]);

	      // Find nearest note
	      note_t *nearest_note = mynotes.find_nearest_freq(input_freq);
              mvprintw (8, 0, "Found rising edge %f at %10d, f = %f Hz\n", val, t, input_freq);
              mvprintw (9, 0, "Note = %3s%d", nearest_note->name, nearest_note->octave);

	      // Update wheel frequency. Need some sort of filter algorithm here
	      // to prevent jittery frequency updates.
	      if (auto_adjust && fwheel != nearest_note->freq) 
		{
		  fwheel = nearest_note->freq;
		  print_wheel_freq(fwheel);
		}


              // Reset counter every so often to prevent eventual 
              // loss of floating point precission
              if (ftime > 1000000)
                ftime = t = 0;
              prev_rise = t;
            }
        }

      // Increment timer
      ftime += read_frames; 
    }

  snd_pcm_drain (handle);
  snd_pcm_close (handle);
  return 0;
}
