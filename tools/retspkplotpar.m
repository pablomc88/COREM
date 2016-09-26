%RETSPKPLOTPAR plots a raster plot of the neural spike activity
%   This script reads the simulation spikes generated during the retina 
%   simulation from the activity output file and plot them.
%   RETSPKPLOTPAR var_type t_ini t_end only reads the spikes from
%   time t_ini to t_end (in seconds)
%   RETSPKPLOTPAR var_type t_ini t_end neu_ini neu_end only reads the spikes
%   of neurons from neu_ini to neu_end and in time from t_ini to t_end
%   var_type specifies how the activity will be plotted.
%   var_type must be: ra
%    ra: Displais a normal raster plot of the activity
%        (the whole specified period of the simulation is plotted)
%   example to generate a raster plot of simulation activity in the time interval 0 1:
%      retspkplotpar ra 0 1
%
%   See also RETSPKPLOT.

%   Copyright (C) 2016 by Richard R. Carrillo 
%   $Revision: 1.3 $  $Date: 26/9/2016 $
%   (adapted from noout2par from EDLUT repository)

%   This program is free software; you can redistribute it and/or modify
%   it under the terms of the GNU General Public License as published by
%   the Free Software Foundation; either version 3 of the License, or
%   (at your option) any later version.
function retspkplotpar(varargin)

LOG_FILE='spikes.spk'; % log file name
sim_slot_time=0.1e-3; % approx. simulation step length in seconds

nargs=nargin;
if nargin > 0
   v=varargin{1};
   if ~isequal(v,'ra') && ~isequal(v,'ra')
      display('Incorrect value of the first argument');
      nargs=0;
   end
end

if nargs ~= 1 && nargs ~= 3 && nargs ~= 5
      disp('You must specify 1, 3 or 5 arguments');
      disp('retspkplotpar plot_type [start_time end_time [first_neuron last_neuron]]');
      disp('type: help retspkplotpar for more help');
end

if nargs == 1 % the whole file must be loaded
    disp(['loading log file: ' LOG_FILE ' completely']);
    disp(' 100%');
    activity_list=load(LOG_FILE);
    tot_num_cols=size(activity_list,2);
else % only a part of the file must be loaded
    start_time=str2num(varargin{2});
    end_time=str2num(varargin{3});
    disp(['Partially loading activity file ' LOG_FILE ' from time ' num2str(start_time) ' to ' num2str(end_time)]);
    fid = fopen(LOG_FILE,'rt');
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
        activity_list=read_file_part(fid,start_time,end_time);
        fclose(fid);
        if last_file_time < start_time
            disp('Error: The specified start_time is no included in the file')
            return
        end
    else
        disp(['Cannot output spike file: ' LOG_FILE]);
        return
    end
end
if nargs == 5 % only some neurons must be plotted
    first_neu=str2num(varargin{4});
    last_neu=str2num(varargin{5});
    activity_list=activity_list(activity_list(:,1) >= first_neu & activity_list(:,1) <= last_neu,:); % remove unwanted neurons from the list
end

neu_list_rep=sort(activity_list(:,1)');

neu_list=[];
if ~isempty(neu_list_rep)
    num_spk_neus=1;
    neu_list(1)=neu_list_rep(1);
else
    num_spk_neus=0;
end
for nneu=2:length(neu_list_rep),
    if neu_list_rep(nneu-1) ~= neu_list_rep(nneu)
        num_spk_neus=num_spk_neus+1;
        neu_list(num_spk_neus)=neu_list_rep(nneu);
    end
end

disp('Creating figure...');

switch(v)
case 'ra'
    tot_spks=0;
    for nneu=1:num_spk_neus
        cur_neu_spk_times=activity_list(find(activity_list(:,1)==neu_list(nneu)),2);
        tot_spks=tot_spks+length(cur_neu_spk_times);
        line((cur_neu_spk_times*ones(1,2))',(ones(length(cur_neu_spk_times),1)*[nneu-0.25,nneu+0.25])','Color','b');
    end
    axis tight
    xlabel('time');
    ylabel('neuron number');
    set(gca,'YTick',1:num_spk_neus);
    
    % find out what ticksthin out y-axis tick labels
    neu_tick_list={neu_list(1)};
    last_included_neu=1;
    max_number_of_yticks=30; % in order not to overlap them
    max_intertick_interval=num_spk_neus/max_number_of_yticks;
    for nneu=2:num_spk_neus,
        if neu_list(nneu-1) ~= neu_list(nneu)-1 || (nneu-last_included_neu) > max_intertick_interval || nneu==num_spk_neus
            neu_tick_list{nneu}=neu_list(nneu); % include nonconsecutive neuron ticks or spaced-enough ones
            last_included_neu=nneu;
        else
            neu_tick_list{nneu}=[]; % do not show this tick
        end
    end
    set(gca,'YTickLabel',neu_tick_list);
    display(['Total number of spikes: ' num2str(tot_spks)]);
    display(['Number of spiking neurons: ' num2str(num_spk_neus)]);
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
         tline=fgetl(fid);
         if ischar(tline) && ~isempty(tline) && isempty(~strfind(tline,'%'))
            vline=str2num(tline);
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
