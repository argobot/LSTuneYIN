// Scilab filter stuff

// Sample frequency
fsample = 44100;

// Corner frequencies for hp and lp filters
hp_corner = 30;
lp_corner = 2000;

// Generate transfer functions
hz_hp = iir(1, 'hp', 'butt', [2*hp_corner/fsample, 0], [0,0]);
hz_lp = iir(8, 'lp', 'butt', [2*lp_corner/fsample, 0], [0,0]);

// Type 1 chebychev with 10% pass band ripple
//hz_lp = iir(8, 'lp', 'cheb1', [2*lp_corner/fsample, 0], [0.03,0.03]);
hz_total_z = hz_lp * hz_hp;

// to express the result in terms of the delay operator q=z^-1
q=poly(0,'q');     
hz_total = horner(hz_total_z, 1/q)

// Get coefficients
coeff_den = coeff(hz_total.den);
coeff_num = coeff(hz_total.num);

eq_string = "y(k) =";

x_coef_string = "";
y_coef_string = "";
// Denomitator coefficein zero
d0 = coeff_den(1);
for pos = 1:length(coeff_num)
  order = pos - 1;
  this_coeff = coeff_num(pos) / d0;

  x_coef_string = x_coef_string + sprintf("%e, ", this_coeff)
  
  if pos == 1 then
    this_str = sprintf("%e*x(k)", this_coeff);
    eq_string = eq_string + " " + this_str;
  else
    this_str = sprintf("%e*x(k-%d)", this_coeff, order);
    eq_string = eq_string + " + " + this_str;
  end
end

// Print up in vector form
for pos = 2:length(coeff_den)
  order = pos - 1;
  this_coeff = coeff_den(pos) / d0;
  
  y_coef_string = y_coef_string + sprintf("%e, ", this_coeff)
  
  this_str = sprintf("%f*y(k-%d)", this_coeff, order);
  eq_string = eq_string + " - " + this_str;
end

printf("%s\n", eq_string);
printf("cx = {%s};\n", x_coef_string);
printf("cy = {0, %s};\n", y_coef_string);

// Plot up results
[hzm,fr]=frmag(hz_total_z,1024);
plot2d(fr',hzm')
xtitle('Discrete IIR filter band pass',' ',' ');
