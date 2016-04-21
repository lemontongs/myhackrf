
clear; close all; clc

%% Collect data

!hackrf_transfer -r rx.dat -d 22be1 -f 2490000000 -a 1 -l 40 -g 0 -s 8000000 -n 8000000

%% Process

filename = 'rx.dat';
fd = fopen(filename,'r');
data = fread(fd, 'int8');
fclose(fd);
data = reshape(data, 2, []);
data = complex(data(1,:), data(2,:));


pri = 0.001;
prf = 1/pri;
fs = 8e6;
pw = 1e-3;
df = 1.4998e6;
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

data = data - mean(data);
data = data.*exp(1i*2*pi*(-(df+(chirp_width/2)))*t); % shifted version
data = filtfilt(b,1, data);

data = data(8000*10:end); % Remove initial charge up
t = t(1:length(data));

% figure(100); plot(linspace(-fs/2,fs/2,length(data)), 10*log10(fftshift(abs(fft(data)))));
% figure(101); plot(t,real(data));

% match filter
slopeFactor = (chirp_width)/(2 * pw);
t2 = 0:1/fs:pri-(1/fs);
match_signal = exp(1i * 2.0 * pi * (((-chirp_width/2)*t2)+(slopeFactor*(t2.^2))));
% match_signal=  100 * exp(1i * 2   * pi * ( t2*(-chirp_width/2) + slopeFactor*t2.^2));

% data = data(3799198:end);
% corrResult = xcorr(data,match_signal);
% corrResult(1:length(data)-1) = [];

DATA = fft(data); 
MATCH_SIGNAL = fft(match_signal, length(DATA));
CORR_RESULT = DATA.*conj(MATCH_SIGNAL);
corrResult = ifft(CORR_RESULT);
% figure; plot(10*log10(abs(ifft(CORR_RESULT))))

[pks,locs] = findpeaks(abs(corrResult), 'MINPEAKDISTANCE', 7500);


% figure(102); plot(linspace(-fs/2,fs/2,length(corrResult)), 10*log10(fftshift(abs(fft(corrResult)))));


pulses = [];
nsamples = pri*fs;
for k = 1:length(locs)-3
    pulses(k,:) = data(locs(k):locs(k)+nsamples-1) - mean(data(locs(k):locs(k)+nsamples-1));
end

num_pulses = size(pulses,1)-1;
pulse_size = size(pulses,2);

y = zeros(num_pulses, pulse_size);
for k = 1:num_pulses
%     y(k,:) = pulses(k,:) - 2*pulses(k+1,:) + pulses(k+2,:); % -2 from num_pulse
    y(k,:) = pulses(k,:) - pulses(k+1,:);
end


% Compute range time intensity
rti = zeros(pulse_size,num_pulses);
for ii = 1:num_pulses
    tmp = abs(xcorr(y(ii,:), match_signal));
    rti(:,ii) = tmp(pulse_size:end);
end


% Compute range doppler map
rdm = zeros(size(y));
for ii = 1:pulse_size
    rdm(:,ii) = fftshift(abs(fft(y(:,ii))));
end

%
% r = tc/2
%
range = ((0:size(pulses,2)-1) .* (1/fs)) .* 3e8 ./ 2;

figure(10);

subplot(221);
imagesc(1:num_pulses, range, abs(y'));
set(gca,'YDir','normal')
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
imagesc(range, 1:num_pulses, 20*log10(rti));
title('Range Time Intensity')
xlabel('Range (m)')
ylabel('Pulse Number')


subplot(224);
plot(10*log10(abs(corrResult)));
hold on
plot(locs,10*log10(pks), 'r*');
title('Match Filter Response')
