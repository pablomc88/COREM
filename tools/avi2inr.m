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
      %imwrite(int_frame, [sprintf('fr_%04d',(ii-2)*fps_mult + subii) '.pgm']);
      %inrwrite(int_frame,[sprintf('fr_%04d',(ii-2)*fps_mult + subii) '.inr'])
      vid_out.add_frame(int_frame);
      % figure(subii),imshow(int_frame);
   end
 ant_frame=cur_frame;
end
delete(vid_out);

