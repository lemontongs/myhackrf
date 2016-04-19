clear

filename = 'rx.dat';

fd = fopen(filename,'r');

save_data = [];
try
    while 1
        data = fread(fd, 10e7, 'int8');

        data = reshape(data, 2, []);
        amp = abs(complex(data(1,:), data(2,:)));

        inds = find(amp > 80);

        for i = 1:length(inds)
            save_data = [ save_data ; amp(inds(i):inds(i)+19) ];
        end
        
        if size(save_data,1) == 25
            fclose(fd);
            break
        end
        fprintf('%d\n', size(save_data,1));
    end
catch e
    disp(e)
    fclose(fd);
end

clear data



