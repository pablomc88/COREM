% This Matlab script plot a raster plot of the output spike activity of file spikes.spk
% For large activity files consider displaying only part of the file using the script:
%  retspkplotpar.m
%
%   See also SPKPLOTPAR.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.1 $  $Date: 26/9/2016 $
%   (adapted from noout2 from EDLUT repository)

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
output=load('../COREM/results/spikes.spk');
lx=max(output(:,2));

spk=sort(output(:,1)');

nspk=[];
if length(spk) > 0
    reps=1;
    nspk(1)=spk(1);
else
    reps=0;
end
for n=2:length(spk),
    if spk(n-1) ~= spk(n)
        reps=reps+1;
        nspk(reps)=spk(n);
    end
end

tot_spks=0;
for n=1:reps
    tspk=output(find(output(:,1)==nspk(n)),2);
    tot_spks=tot_spks+length(tspk);
    line((tspk*ones(1,2))',(ones(length(tspk),1)*[n-0.25,n+0.25])','Color','b');
end
axis tight
xlabel('time');
ylabel('neuron number');
display(['Total number of spikes: ' num2str(tot_spks)]);
display(['Number of spiking neurons: ' num2str(reps)]);
set(gca,'YTick',1:reps);
set(gca,'YTickLabel',nspk);
