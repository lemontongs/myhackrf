
clear; close all; clc

%% Collect data

% Collect samples from the device and save them to a file named
% rx.dat in the form of repeated unsigned 8 bit I Q values
!hackrf_transfer -r rx.dat -d 22be1 -f 2490000000 -a 1 -l 40 -g 0 -s 8000000 -n 8000000

%% Process

% Read in the collected data and reshape the IQ into a complex data type
filename = 'rx.dat';
fd = fopen(filename,'r');
data = fread(fd, 'int8');
fclose(fd);
data = reshape(data, 2, []);
data = complex(data(1,:), data(2,:));

% Waveform parameters (need to match the transmit c++ code)
pri = 0.001;
prf = 1/pri;
fs = 8e6;
pw = pri;
chirp_start = 1.4998e6;
chirp_width = 200e3;
chirp_center = chirp_start+(chirp_width/2);
nsamples = pri*fs;
show_intermediate_plots = 1;

% The SDR seems to have a amplifier ramp up time in the first few waveforms
% this removes that ramp-up
data = data(nsamples*10:end);


if show_intermediate_plots
    figure(98);
    subplot(211)
    t = 0:1/fs:length(data)/fs;
    t(end) = [];
    plot(t(1:nsamples*2) * 1e6, real(data(1:nsamples*2)));
    title('Signal')
    xlabel('Time (us)')
    ylabel('Magnitude')
    
    subplot(212)
    DATA = 10*log10(fftshift(abs(fft(data))));
    
    plot(linspace(-fs/2,fs/2,length(DATA)), DATA);
    title('Signal - FFT')
    xlabel('Frequency (Hz)')
    ylabel('dB counts')
end



% Remove the DC offset
data = data - mean(data);

% Shift the signal of interest down to the center of baseband
t = 0:1/fs:length(data)/fs;
t(end) = [];
data = data.*exp(1i*2*pi*(-chirp_center)*t);

if show_intermediate_plots
    figure(99);
    subplot(211)
    plot(t(1:nsamples*2) * 1e6, real(data(1:nsamples*2)));
    title('Signal')
    xlabel('Time (us)')
    ylabel('Magnitude')
    
    subplot(212)
    DATA = 10*log10(fftshift(abs(fft(data))));
    
    plot(linspace(-fs/2,fs/2,length(DATA)), DATA);
    title('Signal - FFT')
    xlabel('Frequency (Hz)')
    ylabel('dB counts')
end

% Define a filter to remove nearby signals and noise
N    = 200;         % Order
Fc   = chirp_width; % Cutoff Frequency
flag = 'scale';     % Sampling Flag
Beta = 0.5;         % Window Parameter
win = kaiser(N+1, Beta);
b  = fir1(N, Fc/(fs/2), 'low', win, flag);

% Apply the filter
data = filtfilt(b,1, data);

% Create a time vector
t = 0:1/fs:length(data)/fs;
t(end) = [];

if show_intermediate_plots
    figure(100);
    subplot(211)
    plot(t(1:nsamples*2) * 1e6, real(data(1:nsamples*2)));
    title('Signal (Filtered)')
    xlabel('Time (us)')
    ylabel('Magnitude')
    
    subplot(212)
    DATA = 10*log10(fftshift(abs(fft(data))));
    FILT = 10*log10(fftshift(abs(fft(b))));
    
    plot(linspace(-fs/2,fs/2,length(DATA)), DATA);
    hold on;
    plot(linspace(-fs/2,fs/2,length(b)), FILT + mean(DATA)-mean(FILT),'r');
    title('Signal (Filtered) - FFT')
    xlabel('Frequency (Hz)')
    ylabel('dB counts')
    legend('Signal','Filter')
end

% Create a match filter
slopeFactor = (chirp_width)/(2 * pw);
t2 = 0:1/fs:pri-(1/fs);
match_signal_shifted = exp(1i * 2.0 * pi * (((-chirp_width/2)*t2)+(slopeFactor*(t2.^2))));

% plot_tx_csv;
% match_signal = tx_sig';
% % Shift the match signal down to the center of baseband
% t = 0:1/fs:length(match_signal)/fs;
% t(end) = [];
% match_signal_shifted = match_signal.*exp(1i*2*pi*(chirp_center)*t);


if show_intermediate_plots
    figure(104);
    %MATCH   = 10*log10(fftshift(abs(fft(match_signal))));
    MATCH_S = 10*log10(fftshift(abs(fft(match_signal_shifted))));
    %plot(linspace(-fs/2, fs/2, length(MATCH)), MATCH);
    %hold on
    plot(linspace(-fs/2, fs/2, length(MATCH_S)), MATCH_S);
    title('Match Signal')
    xlabel('Frequency (Hz)')
    ylabel('dB')
end


% Apply the match filter to the received signal
corrResult = xcorr(data, match_signal_shifted);
corrResult = corrResult(end-length(data):end);

% Find the peaks of the match filter response (this is where the waveforms start)
[pks,locs] = findpeaks(abs(corrResult), 'MINPEAKDISTANCE', floor(nsamples*0.9));

if show_intermediate_plots
    figure(102);
    plot(10*log10(abs(corrResult)));
    hold on
    plot(locs,10*log10(pks), 'r*');
    title('Match Filter Response')
    xlabel('Sample number')
    ylabel('Correlation (dB)')
end

% Reogranize the detected waveforms into an m by n matrix
num_pulses = length(locs)-5;
pulses = zeros(num_pulses,nsamples) + 1i * ones(num_pulses,nsamples);
for k = 1:num_pulses
    % Remove the DC component on each pulse
    pulses(k,:) = data(locs(k):locs(k)+nsamples-1) - mean(data(locs(k):locs(k)+nsamples-1));
end



if show_intermediate_plots
    figure(103);
    imagesc(1:num_pulses, 1:nsamples, abs(pulses'));
    set(gca,'YDir','normal')
    title('Pulse Amplitude')
    xlabel('Pulse Number')
    ylabel('Sample Number')
    colorbar;
end


pulse_size = size(pulses,2);

% Apply an MTI (Moving Target Indicator) filter to the pulses
num_pulses = num_pulses - 1;
pulses_mti = zeros(num_pulses, pulse_size);
for k = 1:num_pulses
    pulses_mti(k,:) = pulses(k+1,:) - pulses(k,:);
end


% Compute range time intensity
rti = zeros(pulse_size,num_pulses);
for ii = 1:num_pulses
    tmp = abs(xcorr(pulses_mti(ii,:), match_signal_shifted));
    rti(:,ii) = tmp(pulse_size:end);
end


% Compute range doppler map
rdm = zeros(pulse_size, num_pulses);
for ii = 1:pulse_size
    rdm(ii,:) = fftshift(abs(fft(pulses_mti(:,ii))));
end

%
% range = (t*c)/2
%
range_km = ((0:size(pulses,2)-1) .* (1/fs)) .* 3e8 ./ 2 ./ 1e3;

figure(10);
subplot(221);
imagesc(1:num_pulses, range_km, abs(pulses_mti'));
set(gca,'YDir','normal')
title('Pulse Amplitude')
xlabel('Pulse Number')
ylabel('Range (km)')
colorbar;

subplot(222);
speed = linspace(-prf/2, prf/2, num_pulses) / (2490000000 + chirp_center) * 3e8;
speed = speed * 2.23694; % meters/second to miles/hour
imagesc(speed, range_km, 20*log10(rdm));
set(gca,'YDir','normal')
title('Range Doppler Map')
xlabel('Speed (mph)')
ylabel('Range (km)')
colorbar;

subplot(223);
imagesc(1:num_pulses, range_km, 20*log10(rti));
set(gca,'YDir','normal')
title('Range Time Intensity')
xlabel('Pulse Number')
ylabel('Range (km)')

subplot(224);
plot(10*log10(abs(corrResult)));
hold on
plot(locs,10*log10(pks), 'r*');
title('Match Filter Response')
