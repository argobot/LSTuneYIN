// For 6 pole buterworth LP, Fc = 4000hz; HP, first order HP Fc=20Hz, For Fsample = 44100Hz
// Coefficients for x(k), x(k-1), .... x(k-n)
cx = [6.458708e-03, 3.229354e-02, 5.812837e-02,  3.229354e-02, -3.229354e-02, -5.812837e-02, -3.229354e-02, -6.458708e-03];

// Coefficients for y(k), y(k-1), .... y(k-n)
cy = [0,           -2.624508e+00, 3.394598e+00, -2.828718e+00 , 1.476595e+00, -5.052747e-01,  9.840300e-02, -8.738424e-03];

cx = [5.627019e-07, 3.938913e-06, 1.125404e-05, 1.575565e-05, 7.877826e-06, -7.877826e-06, -1.575565e-05, -1.125404e-05, -3.938913e-06, -5.627019e-07 ]
cy = [0, -7.835480e+00, 2.784745e+01, -5.888774e+01, 8.161700e+01, -7.685786e+01, 4.916226e+01, -2.059406e+01, 5.126064e+00, -5.776367e-01]

// For 6 pole chebychev typ1 LP, Fc = 4000hz, pb ripple = 10%; 1st order HP, Fc=20Hz, For Fsample = 44100Hz
//cx = [1.511040e-03, 7.555198e-03, 1.359936e-02, 7.555198e-03, -7.555198e-03, -1.359936e-02, -7.555198e-03, -1.511040e-03]
//cy = [0, -4.239140e+00, 8.775292e+00, -1.139560e+01, 9.863594e+00, -5.670258e+00, 2.002239e+00, -3.355176e-01]


order = length(cx) - 1;
// Sample frequency
fsample = 44100

// Ramp waveform from 5Hz to 20kHz over 10 seconds
samples = fsample * 10;
t = linspace(0, 10, samples);
f = linspace(5, 15000, samples);
// wave_in = sin(2 * %pi .* f .* t);
wave_in = sin(2 * %pi .* f .* t) + 10;


printf("Order: %d", order);

// Deal with initial condtions. Zero pad output.
wave_out = zeros(samples, 1);

// Do filter
for k = (order+1):samples
  for filt_coef = 0:length(cx)-1
    wave_out(k) = wave_out(k) + (cx(filt_coef+1) * wave_in(k - filt_coef)) - (cy(filt_coef+1) * wave_out(k - filt_coef));
  end
end
  // cx(1)*wave_in(n) + cx(2)*wave_in(n-1) + cx(3)*wave_in(n-2) ...

// plot2d(t, wave_in);
// plot2d(t, wave_out);
// plot2d(f, wave_out);



