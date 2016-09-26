% AVI2INR convert a AVI file movie to INRimage format
% avi2inr(filename) opens filename.avi and creates a filename.inr file
% In Linux Matlab may requiere the installation of packages:
%  - gstreamer0.10-plugins-good and gstreamer0.10-tools
% to open .avi files.
$ Some old versions of Matlab may also require that
% /usr/local/MATLAB/MATLAB_Production_Server/R2013a/sys/os/glnxa64/libstdc++.so.6 link
% is changed to point to system libstdc++.so.6 as well
% Example to create movie.inr from movie.avi:
%  avi2inr('movie')
%
%   See also INRWRITE, INRVIDEOWRITE.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.0 $  $Date: 26/9/2016 $

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
function avi2inr(filename)
fps_mult=4;
vid = VideoReader([filename '.avi']);
vid_out=inrfile([filename '.inr']);
ant_frame=mean(read(vid,1),3);
for ii = 2:10 % num_frames
   cur_frame = mean(read(vid,ii),3);
   for subii=1:fps_mult
      v(1,:,:)=ant_frame;
      v(2,:,:)=cur_frame;
      int_frame=permute(uint8(interp1([0 fps_mult], v, subii-1)),[2 3 1]);
      % imwrite(int_frame, [sprintf('fr_%04d',(ii-2)*fps_mult + subii) '.pgm']);
      % inrwrite(int_frame,[sprintf('fr_%04d',(ii-2)*fps_mult + subii) '.inr'])
      vid_out.add_frame(int_frame);
      % figure(subii),imshow(int_frame);
   end
 ant_frame=cur_frame;
end
delete(vid_out);
