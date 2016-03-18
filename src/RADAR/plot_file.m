clear

filename = 'rx.dat';

fs = 20e6;
fc = 2480e6;

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

iq  = iq(lim_l:lim_h);
t   =  t(lim_l:lim_h);
amp = abs(iq);


thresh = mean(real(amp)) + 2.5;

inds = find(amp>thresh);
if length(inds) > 0
    
    start_index = max(inds(1) - (0.000001 * fs), 0);
    end_index   = min(start_index + (0.00001 * fs), length(amp));
    
    frame_iq  =  iq(start_index:end_index);
    frame_amp = amp(start_index:end_index);
    frame_t   = [0:length(frame_iq)-1].*(1e6/fs); % us
    
    
    % Plots 
    fig = figure;
    set(fig, 'Position', get(0,'Screensize')); % Maximize figure.
    
    % I and Q
    subplot(211)
    plot(frame_t, real(frame_iq), '.-')
    hold on
    plot(frame_t, imag(frame_iq), 'r.-')
    xlabel('Time (us)');

    % Amplitude
    subplot(212)
    plot(frame_t, frame_amp, '.-')
    hold on
    plot( [frame_t(1) frame_t(end)], [ thresh thresh ], 'r')
    xlabel('Time (us)');
else
    fprintf('No pulse detected\n')
    exit
end


% Wait for the figure to close
while ishandle(fig)
    pause(0.1)
end

exit






