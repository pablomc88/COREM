% inrwrite creatres INRimage file (image or movie)
% 
% inrwrite(im_mat,filename)
% When used to create a image inrwrite has similar parameter format as
% imwrite.
% Example to create a 320x240 white image:
%  inrwrite(uint8(255*ones(240,320)),'image.inr')
% When used to create a movie in_mat has an extra dimension whose
% coordinates indicate the frame number
%
%   See also AVI2INR, INRVIDEOWRITE.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.0 $  $Date: 26/9/2016 $

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
function inrwrite(im_mat,filename)
inr_header_len=256;
inr_header_start_temp=[...
   '#INRIMAGE-4#{\n' ...
   'XDIM=%d\n' ...
   'YDIM=%d\n' ...
   'ZDIM=%d\n' ...
   'VDIM=1\n' ...
   'VX=1\n' ...
   'VY=1\n' ...
   'VZ=1\n' ...
   'TYPE=%s\n' ...
   'PIXSIZE=%d bits\n' ...
   'SCALE=2**0\n' ...
   'CPU=pc\n'];
inr_header_end=sprintf('\n##}\n');

inr_type_list={'float', 'signed fixed', 'unsigned fixed'};
switch(class(im_mat))
   case {'single', 'double'}
      inr_type=inr_type_list{1};
   case {'int8', 'int16', 'int32', 'int64'}
      inr_type=inr_type_list{2};
   case {'uint8', 'uint16', 'uint32', 'uint64'}
      inr_type=inr_type_list{3};
end

switch(class(im_mat))
   case {'int8', 'uint8'}
      inr_type_len=8;
   case {'int16', 'uint16'}
      inr_type_len=16;
   case {'int32', 'uint32', 'single'}
      inr_type_len=32;
   case {'int64', 'uint64', 'double'}
      inr_type_len=64;
end  
[ydim,xdim,zdim]=size(im_mat);
inr_header_start=sprintf(inr_header_start_temp, xdim, ydim, zdim, inr_type, inr_type_len);
fh=fopen(filename,'w');
fprintf(fh,inr_header_start);
fprintf(fh,repmat('\n',1,inr_header_len-length(inr_header_start)-length(inr_header_end)));
fprintf(fh,inr_header_end);
fwrite(fh,permute(im_mat,[2 1 3]),class(im_mat));
fclose(fh);
