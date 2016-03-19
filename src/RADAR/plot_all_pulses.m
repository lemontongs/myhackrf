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


thresh = 20;
inds = find(amp>thresh);

if length(inds) == 0
    fprintf('No pulse detected\n')
    exit
end


% Window size
pre_time  = 0.0000005; % seconds before the detection
post_time = 0.000003;  % seconds after the detection
window_size_samples = (pre_time + post_time) * fs;

stack_size = 50;
stack_index = 1;

data = [];

fig = figure;
set(fig, 'Position', get(0,'Screensize')); % Maximize figure.

while length(inds) > 0 && stack_index <= stack_size
    
    % Grab first index
    index = inds(1);
    
    % Throw away all inds between this one and the next "window size"
    inds = inds(inds > (index + window_size_samples));
    
    % Find the data
    start_index = max(       index - ( pre_time * fs), 1);
    end_index   = min( start_index + (post_time * fs), length(amp));
    
    % Copy the data
    frame_iq  =  iq(start_index:end_index);
    frame_amp = amp(start_index:end_index);
    frame_t   = ([start_index:end_index]-index).*(1e6/fs); % us
    frame_d   = frame_t ./ 1e6 .* 299792458 .* 3.28084;    % feet
    
    data(stack_index,:) = frame_amp;
    
    stack_index = stack_index + 1;
end

% Plot the pulses
subplot(211)
surf(data)
view( 20, 15 )
title(['Amplitude of ' num2str(stack_size) ' pulses'])

subplot(212)
plot(frame_d, mean(data), '.-')
hold on

% Plot the threshold
plot( [frame_d(1) frame_d(end)], [ thresh thresh ], 'r')
xlabel('Distance (feet)');
title(['Mean of ' num2str(stack_size) ' pulse sample values'])


% Wait for the figure to close
while ishandle(fig)
    pause(0.1)
end
exit






