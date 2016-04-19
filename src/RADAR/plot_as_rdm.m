clear

%filename = 'first_contact.dat';
filename = 'rx.dat';
%filename = 'data/rx.dat';
%filename = 'data/rx_far.dat';
%filename = 'data/rx_tree.dat';

fd = fopen(filename,'r');

prf = 20e6/262144;
pulse_size = 30;
pulse_offset = 4; % number of pulses in waveform
pulses = [];
try
    while 1
        data = fread(fd, 10e7, 'int8');
        
        if isempty(data)
            fclose(fd);
            break
        end

        data = reshape(data, 2, []);
        data = complex(data(1,:), data(2,:));
        amp = abs(data);

        inds = find(amp > 80);
        
        last = -1;
        for ii = 1:length(inds)
            
            if inds(ii) ~= (last+1)
                pulses = [ pulses ; data( inds(ii)+pulse_offset : inds(ii)+pulse_size-1+pulse_offset ) ];
            end
            
            last = inds(ii);
        end
        
        fprintf('num pulses: %d\n', size(pulses,1));
    end
catch e
    disp(e)
    fclose(fd);
end
clear data

pulses(117,:) = [];

num_pulses = size(pulses,1);

% Compute range doppler map
rdm = zeros(size(pulses));
for ii = 1:pulse_size
    rdm(:,ii) = fftshift(abs(fft(pulses(:,ii))));
end

%
% r = tc/2
%
range = ((pulse_offset:size(pulses,2)-1+pulse_offset) .* (1/8e6)) .* 3e8 ./ 2;

subplot(221);
imagesc(range, 1:num_pulses, abs(pulses));
title('Pulse Amplitude')
xlabel('Range (m)')
ylabel('Pulse Number')
colorbar;

subplot(222);
imagesc(linspace(-num_pulses/2, num_pulses/2, num_pulses), range, rdm');
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


