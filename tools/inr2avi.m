%INR2AVI  convert a INR file movie to AVI format
% avi2inr(filename, fps, show_cb, title_str) opens filename.inr and
% creates a filename.avi file.
% Since the INR file has no information about frame rate, the frames per
% second of the results avi file must be specified through this parameter.
% If show_cb is true, the colorbar is showed, otherwise,
% the video is showed in gray scale
% if title_str is not empty, the specified title is added to the figure
%
%   See also AVI2INR.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.5 $  $Date: 2/11/2016 $
%   (adapted from noout2par from EDLUT repository)

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
function inr2avi(filename, fps, show_cb, title_str)
[images, read_dim_size] = inrload([filename '.inr']); % Load all data from file , 0,Inf, 0,Inf, 0,10
if ~isempty(images) % File successfully read
    fprintf(1,'Dimensions of read data (X,Y,Z,V): %i x %i x %i x %i\n', arrayfun( @(dim) (size(images,dim)), 1:4));

    images = permute(images,[2 1 4 3]); % for immovie image must be an M-by-N-by-1-by-K array, where K is the number of images

    vid_obj = VideoWriter([filename '.avi']);
    vid_obj.FrameRate = fps;
    open(vid_obj);

    for nframe=1:size(images,4)
        imagesc(images(:,:,:,nframe))
        axis off
        if ~isempty(title_str)
            hand_t=title(title_str);
            set(hand_t,'FontName','Courier 10 Pitch')
            set(hand_t,'FontSize',18)
        end
        if show_cb
            hand_cb=colorbar;
            set(hand_cb,'FontName','Courier 10 Pitch')
            set(hand_cb,'FontSize',18)
        else
            colormap(gray)
        end
        curr_frame = getframe(gcf);
        writeVideo(vid_obj,curr_frame);
    end
    close(vid_obj);
end
