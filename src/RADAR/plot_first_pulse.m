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


thresh = 50;

inds = find(amp>thresh);
if length(inds) > 0
    
    pre_time  = 0.0000005;
    post_time = 0.000003;
    
    start_index = max(     inds(1) - ( pre_time * fs), 1);
    end_index   = min( start_index + (post_time * fs), length(amp));
    
    frame_iq  =  iq(start_index:end_index);
    frame_amp = amp(start_index:end_index);
    frame_t   = ([start_index:end_index]-inds(1)).*(1e6/fs); % us
    frame_d   = frame_t ./ 1e6 .* 299792458 .* 3.28084; % feet
    
    
    % Plots 
    fig = figure;
    set(fig, 'Position', get(0,'Screensize')); % Maximize figure.
    
    % I and Q
    subplot(211)
    plot(frame_t, real(frame_iq), '.-')
    hold on
    plot(frame_t, imag(frame_iq), 'r.-')
    xlabel('Time (us)');
    legend('I','Q');

    % Amplitude
    subplot(212)
    plot(frame_d, frame_amp, '.-')
    hold on
    plot( [frame_d(1) frame_d(end)], [ thresh thresh ], 'r')
    xlabel('Distance (feet)');
else
    fprintf('No pulse detected\n')
    exit
end


% Wait for the figure to close
while ishandle(fig)
    pause(0.1)
end
exit






