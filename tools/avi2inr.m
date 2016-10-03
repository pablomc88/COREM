% AVI2INR convert a AVI file movie to INRimage format
% avi2inr(filename, fps_mult, last_input_frame) opens filename.avi and
% creates a filename.inr file. fps_mult indicates how many (linearly
% interpolated) output frames will be created from each input frame.
% last_input_frame indicates now many input frames will be read (and
% saved). The outut video will contain aproximately
% fps_mult * last_input_frame frames.
% In Linux Matlab may requiere the installation of packages:
%  - gstreamer0.10-plugins-good and gstreamer0.10-tools
% to open .avi files.
% Some old versions of Matlab may also require that
% /usr/local/MATLAB/MATLAB_Production_Server/R2013a/sys/os/glnxa64/libstdc++.so.6 link
% is changed to point to system libstdc++.so.6 as well
% Example to create movie.inr from movie.avi with same frame rate and approx. length:
%  avi2inr('movie',1,Inf)
%
%   See also INRWRITE, INRVIDEOWRITE.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.0 $  $Date: 26/9/2016 $

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
function avi2inr(filename, fps_mult, last_input_frame)
vid_in = VideoReader([filename '.avi']);
vid_out=inrvideowrite([filename '.inr']);
ant_frame=mean(read(vid_in,1),3);
num_frames = vid_in.NumberOfFrames;
input_frame_index = 2;
while input_frame_index <= num_frames && input_frame_index <= last_input_frame
   cur_frame = mean(read(vid_in,input_frame_index),3);
   for frame_subindex=1:fps_mult
      v(1,:,:)=ant_frame;
      v(2,:,:)=cur_frame;
      int_frame=permute(uint8(interp1([0 fps_mult], v, frame_subindex-1)),[2 3 1]);
      % imwrite(int_frame, [filename sprintf('_%04d',(ii-2)*fps_mult + frame_subindex) '.pgm']);
      vid_out.add_frame(int_frame);
      % imshow(int_frame);
   end
   ant_frame=cur_frame;
   input_frame_index = input_frame_index + 1;
end
delete(vid_out);
