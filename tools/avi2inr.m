% AVI2INR convert a AVI file movie to INRimage format
% avi2inr(filename, fps_mult, last_input_frame) opens filename.avi and
% creates a filename.inr file. fps_mult indicates how many (linearly
% interpolated) output frames will be created from each input frame.
% last_input_frame indicates now many input frames will be read (and
% saved). The outut video will contain aproximately
% fps_mult * last_input_frame frames.
% avi2inr(filename, fps_mult, last_input_frame, convert_gray) specifies
% whether the frames must be converted to grayscale before saved.
% Parameters fps_mult, last_input_frame and convert_gray are optional.
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
function avi2inr(filename, fps_mult, last_input_frame, convert_gray)
% Set default values
if nargin<4
    convert_gray=false;
end
if nargin<3
    last_input_frame=Inf;
end
if nargin<2
    fps_mult=1;
end

vid_in = VideoReader([filename '.avi']); % Open input video file
vid_out=inrvideowrite([filename '.inr']); % Create output video file
ant_frame=read(vid_in,1);
pixel_data_type=class(ant_frame); % Save the original pixel data type
if convert_gray
   ant_frame=mean(ant_frame,3); % Average reg, green and blue channels
end
num_frames = vid_in.NumberOfFrames;
if num_frames>0
    % Always save the first frame
    int_frame=cast(ant_frame, pixel_data_type); % Recover original pixel data type
    vid_out.add_frame(int_frame); % Add frame to output file
    % Get and save sucessive frames
    input_frame_index = 2;
    while input_frame_index <= num_frames && input_frame_index <= last_input_frame
       cur_frame = read(vid_in,input_frame_index); % Load next frame form input
       if convert_gray
          cur_frame=mean(cur_frame,3);
       end
       for frame_subindex=1:fps_mult
          % Interpolate using previous and current frame
          v(1,:,:,:)=ant_frame;
          v(2,:,:,:)=cur_frame;
          int_frame=interp1([0 fps_mult], double(v), frame_subindex);
          % place dimenion create for interp1 (first one) at the end to
          % that it can be ignored
          int_frame=permute(int_frame, [2 3 4 1]);
          int_frame=cast(int_frame, pixel_data_type); % Recover original pixel data type
          % In Matlab image matrices encode Y pixel coordinates in the
          % first dimension and color channel in the third dimension.
          % add_frame makes the corresponding permutations to obtain INR
          % frames from this Matlab encoding
          vid_out.add_frame(int_frame); % Add frame to output file
       end
       ant_frame=cur_frame;
       input_frame_index = input_frame_index + 1;
    end
end
delete(vid_out); % Update output file content and close 
