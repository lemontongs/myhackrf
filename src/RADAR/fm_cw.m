
clear; close all; clc

%% Read from file

!hackrf_transfer -r rx.dat -d 22be1 -f 2490000000 -a 1 -l 40 -g 0 -s 8000000 -n 4000000
filename = 'rx.dat';
fd = fopen(filename,'r');
data = fread(fd, 'int8');
fclose(fd);
data = reshape(data, 2, []);
data = complex(data(1,:), data(2,:));

%% Process

pri = 0.001;
prf = 1/pri;
fs = 8e6;
pulse_offset = 0;
pw = 1e-3;
df = 1.5e6;
chirp_width = 200e3;



% filter
N    = 200;      % Order
Fc   = 200000;   % Cutoff Frequency
flag = 'scale';  % Sampling Flag
Beta = 0.5;      % Window Parameter
win = kaiser(N+1, Beta);

% Calculate the coefficients using the FIR1 function.
b  = fir1(N, Fc/(fs/2), 'low', win, flag);
t = 0:1/fs:length(data)/fs; t(end) = [];

data = data.*exp(1i * 2 * pi * (-1.6e6) * t); % shifted version
data = filtfilt(b,1, data);

%figure(100); plot(linspace(-fs/2,fs/2,length(data)), 10*log10(fftshift(abs(fft(data)))));
%figure(101); plot(t,real(data));

% match filter
slopeFactor = (chirp_width)/(2 * pw);
t2 = 0:1/fs:pri-(1/fs);
match_signal = 100 * exp(1i * 2.0 * pi * ((df*t)+(slopeFactor*(t.^2))));

% Apply the match filter
corrResult = xcorr(data,match_signal);
corrResult(1:length(data)-1) = [];

[pks,locs] = findpeaks(abs(corrResult), 'MINPEAKDISTANCE', 7500);

%figure(102); plot(linspace(-fs/2,fs/2,length(corrResult)), 10*log10(fftshift(abs(fft(corrResult))))); title('Matched Filter Response')
figure(103);
plot(10*log10(abs(corrResult)));
hold on
plot(locs,10*log10(abs(pks)),'r*')
title('Matched Filter Response')

% Convert the detected waveforms into the pulsed-doppler form
pulses = [];
nsamples = pri*fs;
for k =1:(length(locs)-5)
    pulses(k,:) = data(locs(k):locs(k)+nsamples-1);
end

num_pulses = size(pulses,1);
pulse_size = size(pulses,2);

% Compute range doppler map
rdm = zeros(size(pulses));
for ii = 1:pulse_size
    rdm(:,ii) = fftshift(abs(fft(pulses(:,ii))));
end

%
% r = tc/2
%
range = ((pulse_offset:size(pulses,2)-1+pulse_offset) .* (1/fs)) .* 3e8 ./ 2;

figure(10);

subplot(221);
imagesc(range, 1:num_pulses, abs(pulses'));
title('Pulse Amplitude')
xlabel('Pulse Number')
ylabel('Range (m)')
colorbar;

subplot(222);
imagesc(linspace(-prf/2, prf/2, num_pulses), range, 20*log10(rdm'));
set(gca,'YDir','normal')
title('Range Doppler Map')
xlabel('Doppler shift (Hz)')
ylabel('Range (m)')
colorbar;

subplot(223);
plot(range, sum(abs(pulses))./num_pulses, '.-');
title('Mean Amplitude')
xlabel('Range (m)')
ylabel('Amplitude (counts)')

