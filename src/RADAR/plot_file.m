clear

filename = '/home/mlamonta/git/myhackrf/src/RADAR/rx.dat';

fs = 20e6;
fc = 2478e6;

N = 2^12;
f = -(N/2):(N/2)-1;
f = f*fs/N;
f = f + fc;
f_mhz = f/1e6;

fd = fopen(filename,'r');
data = fread(fd, 'int8');
fclose(fd);

data = reshape(data, 2, []);
iq = complex(data(1,:), data(2,:));

t = [0:length(iq)-1]*(1/fs);

clear data

lim_l=1;
lim_h=length(iq);

iq = iq(lim_l:lim_h);
t  =  t(lim_l:lim_h);


%% Plots 

figure
%subplot(211);
plot(t, real(iq), '.-')

%subplot(212);
%plot(f_mhz, 20*log10(fftshift(abs(fft(iq, N)/N))))

