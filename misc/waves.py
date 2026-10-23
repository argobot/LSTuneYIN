#!/usr/bin/env python
import sys
import scipy as sp
import pylab

srate = 44100
frame_period = 1/30.0
t = sp.linspace(0, 2*frame_period, 2*srate*frame_period)
a3_freq = 220;

y = 1.0 * sp.sin(2*sp.pi * 1.0 * a3_freq * t) + 1.0 * sp.cos(2*sp.pi * 2.0 * a3_freq * t) + \
    1.5 * sp.cos(2*sp.pi * 3.0 * a3_freq * t) + 0.5 * sp.sin(2*sp.pi * 4.0 * a3_freq * t)
# Normalize
y = y/y.max() * 0.8

#fres = abs(sp.fft(y))
#scount = sp.array(range(0, len(fres)))
# fres[15] = 220Hz
# fres[29] = 440Hz

# pylab.plot(scount, fres, 'b-')
# pylab.grid(True)
# pylab.show()
# sys.exit()

rises, rise_vals, rise_peaks, rise_ptimes = [], [], [], []
falls, fall_vals, fall_peaks, fall_ptimes = [], [], [], []
state = False
hysteresis = 0.02

peak, peak_time = None, None
for n in xrange(0, len(y)):
    val = y[n]
    # Did we find a falling edge?    
    if state:
        if val > peak or peak == None:
            peak, peak_time = val, n
        if val < -hysteresis:
            state = False
            falls.append(t[n])
            fall_vals.append(val)
            rise_peaks.append(peak)
            rise_ptimes.append(t[peak_time])
    elif not state:
        if val < peak or peak == None:
            peak, peak_time = val, n
        if val > hysteresis:
            state = True
            rises.append(t[n])
            rise_vals.append(val)
            fall_peaks.append(peak)
            fall_ptimes.append(t[peak_time])

rises = sp.array(rises)
rise_vals = sp.array(rise_vals)

# look at the peaks
peaks_norm = rise_peaks/max(rise_peaks)
bucket_size = 0.1
bigpeak_times = []
for v in xrange(0, len(peaks_norm)-1):

    print "Peaks norm", peaks_norm[v], "v=", v

    if peaks_norm[v] < (1.0+bucket_size) and peaks_norm[v] > (1.0-bucket_size):
        bigpeak_times.append(rise_ptimes[v])


bigpeak_freqs = [1.0/(bigpeak_times[k+1] - bigpeak_times[k]) 
                 for k in xrange(0, len(bigpeak_times)-1)]

print "Peaka Mean = ", sp.exp(sp.log(bigpeak_freqs).mean())
print bigpeak_freqs
      
    

# Try to figure out the frequency of each rise
for n in range(0, 5):
    freqs = sp.array([1.0/(rises[k+n] - rises[k]) for k in xrange(0, len(rises)-n)])
    print "N =", n, "Mean = ", sp.exp(sp.log(freqs).mean())
    print freqs

# Plot
pylab.plot(t, y, 'b-', 
           rises, rise_vals, 'bo', 
           falls, fall_vals, 'go',
           rise_ptimes, rise_peaks, 'ro',
           rise_ptimes, peaks_norm, 'co',
           fall_ptimes, fall_peaks, 'yo')

pylab.xlabel('Time(s)')
pylab.ylabel('Amplitute')
pylab.title('Amplitute vs Time')
pylab.grid(True)
# pylab.savefig('simple_plot')
pylab.show()
