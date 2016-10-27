%INRPLOT loads an INR file and plots its content graphically
%   This function allows the user to partially read a INR file and plot
%   part of its content in different modes.
%   INRPLOT filename mode
%   load the whole INR filename file and plot it in mode mode.
%   If additional arguments are specified, only a specific part of the
%   file is loaded:

%   INRPLOT filename mode x_ini x_end only reads (and plot) an interval of
%   pixel columns from all source images (from column x_ini to column x_end).
%   First column coordinate is number zero.
%   INRPLOT filename mode x_ini x_end y_ini y_end only reads a square from 
%   all source images (from column x_ini to column x_end and from row x_ini
%   to row x_end).
%   INRPLOT filename mode x_ini x_end y_ini y_end z_ini z_end only reads a
%   square from a specific interval of source images (from frame z_ini to
%   frame z_end).  First frame is number zero.
%   
%   Inf can be specified as end coordinate value to indicate that we want to
%   load up to the last coordinate in file.
%   mode can be:
%     mo: to play the images as a movie
%     pl: to plot the evolution of each pixel magnitude as a graph curve
%
%   For example:
%   inrplot 'sequence.inr' mo 0 Inf 0 Inf 0 99
%   it loads the 100 first complete frames and play them as a movie.
%
%   See also INRLOAD.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.0 $  $Date: 26/10/2016 $

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
function inrplot(varargin)

nargs=nargin;

if nargs ~= 2 && nargs ~= 4 && nargs ~= 6 && nargs ~= 8
      disp('You must specify 2, 4, 6 or 8 arguments');
      disp('inrload filename mode [x_ini x_end [y_ini y_end [z_ini z_end]]]');
      disp('All arguments must be numeric except the first two ones')
      disp('type: help inrplot for more help');
else
    N_dims=4;
    filename = varargin{1};
    mode = varargin{2};

    % If fn argument dimension data types are strings, convert to doubles
    % The type may change denpending on how the fn is called
    for arg_ind=3:nargs
        if isa(varargin{arg_ind},'char')
            vargs_num(arg_ind-2) = str2double(varargin{arg_ind});
        else
            vargs_num(arg_ind-2) = varargin{arg_ind};
        end
    end

    user_dim_size = [zeros(1,N_dims) ; ones(1,N_dims)*Inf]; % Default dimension coordinates of data to load: load all data: 0 -> Inf
    % Replace dim_size values with the dimension limits specified by user
    for arg_ind=1:(nargs-2)/2 % Parse input dimension coordenate intervals
        user_dim_size(:,arg_ind) = [vargs_num(arg_ind*2 - 1) vargs_num(arg_ind*2)];
    end

    user_dim_size_cell = num2cell(user_dim_size(:)); % Convert array into an array of cells in order to pass these numbers as arguments to the inrload fn
    images = inrload(filename, user_dim_size_cell{:}); % Load all required data from file
    if ~isempty(images) % File successfully read
        fprintf(1,'Dimensions of read data (X,Y,Z,V): %i x %i x %i x %i\n', arrayfun( @(dim) (size(images,dim)), 1:N_dims));
        
        if size(images,4) == 4
            warning('Fourth dimension (V) of data in input file should only have 1 coordinate (that is, the file should contain escalar pixels). Using only the first V coordinate.')
            images=images(:,:,:,1);
        end
        
        % Calculate specific coordinates of pixels in matrix, that is,
        % replace Inf values by definite dimension coordinates
        read_dim_size=user_dim_size;
        for n_dim=1:N_dims
            if isinf(read_dim_size(2,n_dim))
                read_dim_size(2,n_dim)=size(images, n_dim)-1;
            end
        end
        
        switch mode
            case 'mo'
                images = permute(images,[2 1 4 3]); % image must be an M-by-N-by-1-by-K array, where K is the number of images
                images_mov = immovie(1+images,gray(max(images(:))));
                implay(images_mov);
            case 'pl'
                % Arrange image pixels as row vectors
                pixel_vect=reshape(images, [size(images,1)*size(images,2) size(images,3)]); % first matrix dimension (image row) traveled first
                plot(read_dim_size(1,3):read_dim_size(2,3), pixel_vect') % plot one curve per pixel (row of pixel_vect)
                title('Evolution of pixel magnitudes')
                xlabel('frame')
                ylabel('activaton level')
                % Calculate the label for each plot curve (iamge pixel evolution)
                x_coord=repmat((read_dim_size(1,1):read_dim_size(2,1))', 1, read_dim_size(2,2)-read_dim_size(1,2)+1); % x coord. of the labels
                y_coord=repmat(read_dim_size(1,2):read_dim_size(2,2), read_dim_size(2,1)-read_dim_size(1,1)+1, 1); % y coord. of the labels
                pixel_labels=arrayfun(@(x,y)(sprintf('(%i,%i)',x,y)), x_coord, y_coord, 'UniformOutput', false);
                legend(pixel_labels(:))
            case 'ra'
                
            otherwise
                disp('Unknown plot mode specified.')
        end
    end
end

