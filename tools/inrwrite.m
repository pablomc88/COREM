% inrwrite creatres INRimage file (image or movie)
% 
% inrwrite(im_mat,filename)
% When used to create a image inrwrite has similar parameter format as
% imwrite.
% Example to create a 320x240 white image:
%  inrwrite(uint8(255*ones(240,320)),'image.inr')
% When used to create a movie in_mat has an extra fourth dimension whose
% coordinates indicate the frame number
%
%   See also AVI2INR, INRVIDEOWRITE.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.0 $  $Date: 26/9/2016 $

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
function inrwrite(im_mat, filename)
out_vid=inrvideowrite(filename);
for n_frame=1:size(im_mat,4)
    out_vid.add_frame(im_mat(:,:,:,n_frame));
end
delete(out_vid);
