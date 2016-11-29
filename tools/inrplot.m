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
%   frame z_end). First frame is number zero.
%   INRPLOT filename mode x_ini x_end y_ini y_end z_ini z_end v_ini v_end
%   only reads a subset of color channels of a square from a specific
%   interval of source frames. First channel is number zero.
%   
%   Inf can be specified as end coordinate value to indicate that we want to
%   load up to the last coordinate in file.
%   mode can be:
%     mo: to play the images as a movie
%     pl: to plot the evolution of each pixel magnitude as a graph curve,
%     that is, pixel magnitude versus frame number.
%     If the number of frames is only one, it plots the pixel magnitude
%     versus the X coordinate (or Y if we have more Y coordinates than X
%     coordinate) of the pixel. That is, one curve per row (or column).
%     ra: to plot a raster plot of the magnitude of specified pixels
%
%   For example:
%   inrplot 'sequence.inr'  mo  0 Inf  0 Inf  0 99
%   it loads the 100 first complete frames and play them as a movie.
%   inrplot 'sequence.inr'  pl  4 4  5 5
%   it plots the evolution of the magnitude of pixel (4,5) through the simulation.
%   inrplot 'sequence.inr'  pl  0 Inf  Inf Inf  100 100
%   it plots the magnitude of pixels of last image row in frame 100
%   inrplot 'sequence.inr'  pl  10 11  0 Inf  100 100
%   it plots the pixel magnitudes of rows 10 and 11 of frame 100
%   inrplot 'sequence.inr'  ra  0 Inf  10 10
%   it plots a raster plot of pixel magnitudes of row 10. That is, it shows
%   all pixel magnitudes versus frame number.
%
%   See also INRLOAD.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.3 $  $Date: 2/11/2016 $

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
function inrplot(varargin)

nargs=nargin;

if nargs ~= 2 && nargs ~= 4 && nargs ~= 6 && nargs ~= 8 && nargs ~= 10
      disp('You must specify 2, 4, 6, 8 or 10 arguments');
      disp('inrload filename mode [x_ini x_end [y_ini y_end [z_ini z_end]]]');
      disp('All arguments must be numeric except the first two ones')
      disp('type: help inrplot for more help');
      error('Incorrect number of input arguments')
else
    N_dims=4;
    filename = varargin{1};
    mode = varargin{2};

    % If fn argument dimension data types are strings, convert to doubles
    % The type may change denpending on how the fn is called: as fn or script
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
    % Load images and specific coordinates of first and last read pixel for each matrix dimension
    [images, read_dim_size] = inrload(filename, user_dim_size_cell{:}); % Load all required data from file
    if ~isempty(images) % File successfully read
        fprintf(1,'Dimensions of read data (X,Y,Z,V): %i x %i x %i x %i\n', arrayfun( @(dim) (size(images,dim)), 1:N_dims));
        
        if ~strcmp(mode,'mo') && size(images,4) > 1
            warning('Fourth dimension (V) of data in input file should only have 1 coordinate (that is, the file should contain escalar pixels). Using only the first V coordinate.')
            images=images(:,:,:,1);
        end

        switch mode
            case 'mo'
                images = permute(images,[2 1 4 3]); % for immovie image must be an M-by-N-by-1-by-K array, where K is the number of images
                if size(images,3) == 1 % grayscaled frames
                    % normalize pixel values to interval 1 255
                    images = images - min(images(:));
                    images = 1 + double(images)/double(max(images(:)))*255;
                    images_mov = immovie(images,gray(256));
                else
                    if size(images,3) == 3 % color frames
                        images_mov = immovie(images);
                    else
                        error('Input frames must be grayscaled (V dim. size = 1) or true color (V dim. size = 3)')
                    end
                end
                implay(images_mov);
            case 'pl'
                if size(images,3) ~= 1 % If we have several frames, plot magnitudes versus frame number
                    % Arrange each image pixel evolution as a row vector
                    pixel_vect=reshape(images, [size(images,1)*size(images,2) size(images,3)]); % first matrix dimension (image row) traveled first
                    plot(read_dim_size(1,3):read_dim_size(2,3), pixel_vect') % plot one curve per pixel (row of pixel_vect)
                    title('Evolution of pixel magnitudes')
                    xlabel('frame')
                    ylabel('activaton level')
                    % Calculate the label for each plot curve (image pixel evolution)
                    x_coord=repmat((read_dim_size(1,1):read_dim_size(2,1))', 1, read_dim_size(2,2)-read_dim_size(1,2)+1); % x coord. of the labels
                    y_coord=repmat(read_dim_size(1,2):read_dim_size(2,2), read_dim_size(2,1)-read_dim_size(1,1)+1, 1); % y coord. of the labels
                    pixel_labels=arrayfun(@(x,y)(sprintf('(%i,%i)',x,y)), x_coord, y_coord, 'UniformOutput', false);
                    legend(pixel_labels(:))
                else % If we have only one frame, plot magnitudes versus pixel coordinate
                    if size(images,1) >= size(images,2) % If we have X coordinate and y coordinates, plot magnitudes versus X coordinates
                        plot(read_dim_size(1,1):read_dim_size(2,1), images') % plot one curve per pixel (row of pixel_vect)
                        title(['Pixel magnitudes in frame ' num2str(read_dim_size(1,3))])
                        xlabel('X coordinate')
                        ylabel('activaton level')
                        % Calculate the label for each plot curve (image row)
                        pixel_labels=arrayfun(@(y)(sprintf('row %i',y)), read_dim_size(1,2):read_dim_size(2,2), 'UniformOutput', false);
                        legend(pixel_labels(:))
                    else % If we have more Y coordinate than X coordinates, plot magnitudes versus Y coordinates
                        plot(read_dim_size(1,2):read_dim_size(2,2), images) % plot one curve per pixel (row of pixel_vect)
                        title(['Pixel magnitudes in frame ' num2str(read_dim_size(1,3))])
                        xlabel('Y coordinate')
                        ylabel('activaton level')
                        % Calculate the label for each plot curve (image row)
                        pixel_labels=arrayfun(@(x)(sprintf('col. %i',x)), read_dim_size(1,1):read_dim_size(2,1), 'UniformOutput', false);
                        legend(pixel_labels(:))
                    end
                end
            case 'ra'
                % Arrange image pixels as row vectors
                pixel_vect=reshape(images, [size(images,1)*size(images,2) size(images,3)]);
                imagesc(read_dim_size(1,3):read_dim_size(2,3), 1:(read_dim_size(2,1)-read_dim_size(1,1)+1)*(read_dim_size(2,2)-read_dim_size(1,2)+1), pixel_vect);
                % set(gca,'XTick', read_dim_size(1,3):read_dim_size(2,3)) % Show all frame numbers
                colorbar
                title('Evolution of pixel magnitudes')
                xlabel('frame')
                ylabel(sprintf('pixel index from (%i,%i) to (%i,%i)',read_dim_size(:,1:2)'))

            case 'hi'
                tot_pixels=numel(images);
                % Determine a proper number of histogram bins according to the num. of pixels
                if tot_pixels < 1000
                    n_bins = 10;
                else
                    if tot_pixels > 100000
                        n_bins = 1000;
                    else
                        n_bins = tot_pixels/200;
                    end
                end

                hist(double(images(:)), n_bins);
                xlabel('pixel magnitude');
                ylabel('pixel count');
            otherwise
                error('Unknown plot mode specified.')
        end
    else
        error('No data could be loaded.')
    end
end

