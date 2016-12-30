%RETSPKPLOTPAR represent graphically neural spike activity
%   This script reads the simulation spikes generated during the retina 
%   simulation from an activity output file and plot them.
%   RETSPKPLOTPAR mode represent the spikes stored in the default spike
%   output file of COREM (in results directory).
%   Aditional arguments can be specified to limit the amount of spikes
%   loaded and to load from a specific file:
%   RETSPKPLOTPAR mode filename represents the spikes stored in filename
%   using the mode mode.
%   RETSPKPLOTPAR mode filename t_ini t_end only reads the spikes from
%   time t_ini to t_end (in seconds)
%   RETSPKPLOTPAR mode filename t_ini t_end neu_ini neu_end only reads
%   the spikes of neurons from neu_ini to neu_end and in time from t_ini
%   to t_end.
%   mode specifies how the activity will be plotted:
%   var_type must be:
%    ra: Plots a normal raster plot of the activity using thin marks
%        (the specified period of the simulation is plotted)
%    rf: Plots a normal raster plot of the activity using thik marks
%        This mode is faster than ra mode when many spikes are loaded
%    hi: Plots a histogram of the firing periods of all the neurons
%    ph: Plots a histogram of first spike time of all the neurons
%    rs: Plots a normal raster plot of the activity and zooms in a window
%        which is then scrolled from the beginning to the end, creating a
%        video named raster.avi
%    specifed.
%   example to generate a raster plot of simulation activity in the time interval 0 1:
%      retspkplotpar ra spikes.spk 0 1
%
%   See also SPKPLOT.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.6 $  $Date: 9/12/2016 $
%   (adapted from noout2par from EDLUT repository)

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
function spkplotpar(varargin)


sim_slot_time=0.1e-3; % approx. simulation step length in seconds

nargs=nargin; % Number of input arguments specified

if nargs ~= 1 && nargs ~= 2 && nargs ~= 4 && nargs ~= 6
      disp('You must specify 1, 2, 4 or 6 arguments');
      disp('retspkplotpar plot_mode [filename [start_time end_time [first_neuron last_neuron]]]');
      disp('type: help spkplotpar for more help');
      error('Incorrect number of input arguments')
end

if nargin > 0
   plot_mode=varargin{1};
   if ~isequal(plot_mode,'ra') && ~isequal(plot_mode,'hi') && ~isequal(plot_mode,'ph') && ~isequal(plot_mode,'rf')  && ~isequal(plot_mode,'rs')
      nargs=0;
      error('Incorrect value of mode (first argument)');
   end
   if nargin > 1
       filename=varargin{2}; % specified file name
   else
       filename='../COREM/results/spikes.spk'; % default log file name
   end
end


if nargs <= 2 % the whole file must be loaded
    disp(['loading activity file: ' filename ' completely...']);
    activity_list=load(filename);
    disp(' 100%');
    tot_num_cols=size(activity_list,2);
else % only a part of the file must be loaded
    % we use str2num instead of str2double because str2double does not evaluate arithmetic expreions in arguments
    if ischar(varargin{3})
        start_time=str2num(varargin{3});
    else
        start_time=varargin{3};
    end
    if ischar(varargin{4})
        end_time=str2num(varargin{4});
    else
        end_time=varargin{4};
    end
    disp(['Partially loading activity file ' filename ' from time ' num2str(start_time) ' to ' num2str(end_time)]);
    fid = fopen(filename,'rt');
    if fid ~= -1
        fseek(fid,-1,'eof');
        filelength=ftell(fid); % get log file size
        findline_backwards(fid); % set file reading pointer to the line start
        last_line_txt=fgetl(fid); % load last register
        last_line=str2num(last_line_txt);
        last_file_time=last_line(2); % last register time
        tot_num_cols=length(last_line); % total number of columns in the file
        fseek(fid,fix(filelength*(start_time/last_file_time)),'bof');
        findline_backwards(fid);
        % load spikes in the specified time interval
        activity_list=read_file_part(fid,start_time,end_time);
        fclose(fid);
        if last_file_time < start_time
            error('Error: The specified start_time is no included in the file')
        end
    else
        error(['Cannot open output spike file: ' filename]);
    end
end
if nargs == 6 % only some neurons must be plotted
    if ischar(varargin{5})
        first_neu=str2num(varargin{5});
    else
        first_neu=varargin{5};
    end
    if ischar(varargin{6})
        last_neu=str2num(varargin{6});
    else
        last_neu=varargin{6};
    end
    
    activity_list=activity_list(activity_list(:,1) >= first_neu & activity_list(:,1) <= last_neu,:); % remove unwanted neurons from the list
end

if isempty(activity_list)
    activity_list=zeros(0,2); % To avoid errors when file constains no activity
end

if  ~isequal(plot_mode,'rf')
    disp('Sorting spikes...');
    disp(' 00%')

    % Create an array of cells. Each cell containing the activity of a
    % particular neuron and a list of different neuron number
    % These variables will be used by the following code
    tot_spks=size(activity_list,1); % Total number of spikes
    neu_list=zeros(numel(unique(activity_list(:,1))),1); % Allocate space for neuron numbers
    neu_spk=num2cell(neu_list); % Allocate space for spike times

    [neu_sort, neu_sort_ind] = sort(activity_list(:,1)); % Sort neuron numbers
    nneu=1; % Index of current (differnt) neuron
    diff_neus=0; % Total number of different neuron numbers
    neu_sort=[neu_sort;Inf]; % Add an extra neuron number so that find always find a next (different) neuron number in the loop
    while nneu <= length(neu_sort_ind)
        diff_neus = diff_neus+1;
        curr_neu = neu_sort(nneu); % Current neuron number
        neu_list(diff_neus) = curr_neu; % Add new neuron number to the list
        next_neu_ind = find(neu_sort((nneu+1):end) ~= curr_neu,1) + nneu; % Find index of the next (differnt) neuron in neu_sort
        neu_spk{diff_neus} = sort(activity_list(neu_sort_ind(nneu:(next_neu_ind-1)),2)); % Store all spike times of this neuron in the cell
        nneu=next_neu_ind; % Pass to the next neuron number
        if mod(diff_neus,fix(length(neu_spk)/100)) == 0
            fprintf(1,'\b\b\b\b% 3.f%%',diff_neus*100/length(neu_spk));
        end
    end
    fprintf(1,'\b\b\b\b\b100%%\n');
end

disp('Creating figure...');

switch(plot_mode)
case 'ra' % Raster plot
    
    for nneu=1:diff_neus
        cur_neu_spk_times=neu_spk{nneu};
        line((cur_neu_spk_times*ones(1,2))',(ones(length(cur_neu_spk_times),1)*[nneu-0.25,nneu+0.25])','Color','b');
    end
    axis tight
    xlabel('time');
    ylabel('neuron number');
    set(gca,'YTick',1:diff_neus);
    
    % find out what ticksthin out y-axis tick labels
    if ~isempty(neu_list) % Check to avoid errors when no spiking neurons
        neu_tick_list={neu_list(1)};
    else
        neu_tick_list={};
    end
    last_included_neu=1;
    max_number_of_yticks=30; % in order not to overlap them
    max_intertick_interval=diff_neus/max_number_of_yticks;
    for nneu=2:diff_neus,
        if neu_list(nneu-1) ~= neu_list(nneu)-1 || (nneu-last_included_neu) > max_intertick_interval || nneu==diff_neus
            neu_tick_list{nneu}=neu_list(nneu); % include nonconsecutive neuron ticks or spaced-enough ones
            last_included_neu=nneu;
        else
            neu_tick_list{nneu}=[]; % do not show this tick
        end
    end
    set(gca,'YTickLabel',neu_tick_list);

case 'hi' % Histogram plot
    spk_periods=[];
    for nneu=1:diff_neus
        cur_neu_spk_times=neu_spk{nneu};
        spk_periods=[spk_periods ; diff(cur_neu_spk_times*1e3)];
    end
    % Determine a proper number of histogram bins according to the num. of spikes
    if tot_spks < 1000
        n_bins = 10;
    else
        if tot_spks > 100000
            n_bins = 1000;
        else
            n_bins = tot_spks/100;
        end
    end
    
    hist(spk_periods, n_bins);
    xlabel('firing periods (ms)');
    ylabel('spike count');
    
    display(['Mean interspike interval: ' num2str(mean(spk_periods))]);
    display(['Std. dev. of interspike interval: ' num2str(std(spk_periods))]);
    display(['Gamma dist. param. fit of interspike interval (k, theta): ' num2str(gamfit(spk_periods))]);
    [muhat,sigmahat]=normfit(spk_periods);
    display(['Normal dist. param. fit of interspike interval (mu, sigma): ' num2str([muhat,sigmahat])]);

case 'ph' % PHase plot
    spk_phases=[];
    for nneu=1:diff_neus
        cur_neu_spk_times=neu_spk{nneu};
        spk_phases=[spk_phases ; cur_neu_spk_times(1)*1e3];
    end
    % Determine a proper number of histogram bins according to the num. of spikes
    if diff_neus < 1000
        n_bins = 10;
    else
        if diff_neus > 100000
            n_bins = 1000;
        else
            n_bins = diff_neus/100;
        end
    end
    
    hist(spk_phases, n_bins);
    xlabel('firing phase (ms)');
    ylabel('spike count');
case {'rf', 'rs'} % Fast Raster plot
    plot(activity_list(:,2), activity_list(:,1), '.')
    set(gca, 'FontName', 'Courier 10 Pitch')
    set(gca, 'FontSize', 18)
    xlabel('time (s)');
    ylabel('neuron number');
end
display(['Total number of spikes: ' num2str(size(activity_list,1))])
display(['Number of spiking neurons: ' num2str(length(unique(activity_list(:,1))))])
display(['First neuron index: ' num2str(min(activity_list(:,1))) ' Last neuron index: ' num2str(max(activity_list(:,1)))])

if isequal(plot_mode,'rs') % Raster scroll
    title('Spike raster plot')
    set(gca, 'Ydir', 'reverse')
    scroll_wnd=1;
    scroll_step=0.05;
    curr_axis=axis;
    vid_obj = VideoWriter('raster.avi');
    vid_obj.FrameRate = 1/scroll_step;
    open(vid_obj);
    for curr_t=min(activity_list(:,2)):scroll_step:max(activity_list(:,2))
        axis([curr_t-scroll_wnd curr_t curr_axis(3) curr_axis(4)])
        curr_frame = getframe(gcf);
        writeVideo(vid_obj,curr_frame);
    end
    close(vid_obj);
end

% READ_LINE_TIME gets the time of the next register from the simulation-log file.
%    REGTIME = READ_FILE_PART(FID) advances the file position indicator in the
%    file associated with the given FID to the beginning of the next text
%    line, then read and returns that register's time.
   function reg_time=read_line_time(fid)
      time_is_read=0;
      reg_time=-1;
      while time_is_read==0
         [reg_time,time_is_read]=fscanf(fid,'%*d %g',1);
         if time_is_read==0
            fgetl(fid);
            if feof(fid)
               time_is_read=1;
               reg_time=Inf;
            end
         end
      end
   end

% FINDLINE_BACKWARDS move the a file pointer back to the beginning of the
% previous text line.
%    FINDLINE_BACKWARDS(FID) repositions the file position indicator in the
%    file associated with the given FID. FINDLINE_BACKWARDS sets the
%    position indicator to the byte at beginning of the line before the 
%    current one.
   function findline_backwards(fid)
      newline=sprintf('\n');
      tchar=' '; % look for the current-file-line start position
      while ~isempty(tchar) && isempty(strfind(tchar,newline))
         if fseek(fid,-2,'cof')==-1
             fseek(fid,0,'bof');
             break
         end
         tchar = fscanf(fid,'%1c',1);
      end
   end

% READ_FILE_PART loads registers from the simulation-log file.
%    REGS = READ_FILE_PART(FID,STARTTIME,ENDTIME) returns the registers of
%    a file associated with file identifier FID as a MATLAB matrix. Only
%    the registers from STARTTIME to ENDTIME are loaded.
   function regs=read_file_part(fid,starttime,endtime)
      disp(' Searching for specified registers in the file...')
      while ftell(fid) > 0 && read_line_time(fid) > starttime
         findline_backwards(fid); % go back to the current line start
         fseek(fid,-1,'cof'); % jump before \n
         findline_backwards(fid); % go back to the previous line start
      end
      findline_backwards(fid);
      while read_line_time(fid) < starttime
         fgetl(fid); % jump after \n
      end
      findline_backwards(fid);      
      disp(' Loading registers from the file...')
      disp(' 00%')
      app_regs_size=ceil((endtime-starttime)/sim_slot_time);  % estimated size for regs
      regs=zeros(app_regs_size,tot_num_cols); % allocate matrix memory (for execution time optmization)
      regs_size=0;
      cur_file_time=starttime;
      tline=' ';
      while cur_file_time < endtime && ischar(tline) && ~isempty(tline)
         [vline, vline_vals]=fscanf(fid,'%u%*[ \t]%f%*[^\n]\n',2);
         if vline_vals == 0 && ~feof(fid) % Assume this line is a comment: skip it
             fscanf(fid,'%*[^\n]\n',1);
         end
         if vline_vals == 2 % Neuron and time stamp read
            regs_size=regs_size+1;
            regs(regs_size,:)=vline;
            cur_file_time=vline(2);
            if mod(regs_size,fix(app_regs_size/100)) == 0 && cur_file_time > starttime
               new_app_regs_size=ceil(regs_size*(endtime-starttime)/(cur_file_time-starttime)); % new estimation about the size of the matrix needed to store all the registers
               if new_app_regs_size > app_regs_size
                   regs=[regs;zeros(new_app_regs_size-app_regs_size,tot_num_cols)]; % resize (extend) the matrix size
                   app_regs_size=new_app_regs_size;
               end
               progress_percentage=(cur_file_time-starttime)/(endtime-starttime)*100;
               if progress_percentage > 99
                   progress_percentage=99;
               end
               fprintf(1,'\b\b\b\b% 3.f%%',progress_percentage);
            end
         end
      end
      regs=regs(1:regs_size,:); % trim array in case of initial overdimensioning
      fprintf(1,'\b\b\b\b100%%\n');
   end

end
