# include "gray_2d.H"
# include "util.H"

using namespace std;
# include "bmp_io.H"
# include "variables.H"

/**********
Author: Stoney Q. Gan
Description:
	this is the main program for lamina analyses. It has been revised as necessary since its
original version. But it has not undergone any review process. I was hoping to port this program to
the netbeans or eclipse IDE under which I would re-write this program in Java and add GUI for adjusting 
input parameters. My current plan is to do this next summer. Stay tuned!
****************/

/* September 27, 2007
   This is the version after sponsor's meeting.  It is pain to manage memory manually. This method
is just a temp fix.
*/

void reinit_global_variables(int a_num_lines, int a_num_points)
{
  int i;

    if(debug)
      printf("welcome to reinit global variables \n");

    free(data);
    data = (rgb_point *)malloc(sizeof(rgb_point)*num_points);

    free(varve_image.varve_line);
    varve_image.varve_line = (t_varve_line *)malloc(sizeof(t_varve_line)*a_num_lines);
    for (i=0; i<a_num_lines; i++)
       varve_image.varve_line[i].varve_pt = (t_varve *) malloc(sizeof(t_varve)*a_num_points);

    free(wi.varve_line);
    wi.varve_line = (t_varve_line *)malloc(sizeof(t_varve_line)*a_num_lines);
    for (i=0; i<a_num_lines; i++)
        wi.varve_line[i].varve_pt = (t_varve *) malloc(sizeof(t_varve)*a_num_points);

    free(f_image.varve_line);
    f_image.varve_line = (t_varve_line *)malloc(sizeof(t_varve_line)*a_num_lines);
    for (i=0; i<a_num_lines; i++)
        f_image.varve_line[i].varve_pt = (t_varve *) malloc(sizeof(t_varve)*a_num_points);

    free(final_varve_index);
    final_varve_index = (int **)malloc(sizeof(int *)*a_num_lines);
    for (i=0; i<a_num_lines; i++)
         final_varve_index[i] = (int *)malloc(sizeof(int)*a_num_points);

    free(cleaned_varve_image.varve_line);
    cleaned_varve_image.varve_line = (t_varve_line *)malloc(sizeof(t_varve_line)*a_num_lines);
    for (i=0; i<a_num_lines; i++)
       cleaned_varve_image.varve_line[i].varve_pt = (t_varve *) malloc(sizeof(t_varve)*a_num_points);

    free(mean_thickness);
    free(mean_value_r);
    free(mean_value_g);
    free(mean_value_b);
    mean_thickness =(int *)malloc(sizeof(int)*(a_num_points));
    mean_value_r =(int *)malloc(sizeof(int)*(a_num_points));
    mean_value_g =(int *)malloc(sizeof(int)*(a_num_points));
    mean_value_b =(int *)malloc(sizeof(int)*(a_num_points));

    free(mean_laminae_r);
    free(mean_laminae_g);
    free(mean_laminae_b);
    mean_laminae_r =(int *)malloc(sizeof(int)*(a_num_points));
    mean_laminae_g =(int *)malloc(sizeof(int)*(a_num_points));
    mean_laminae_b =(int *)malloc(sizeof(int)*(a_num_points));

    if(debug)
      printf("exiting to reinit global variables \n");
}

/* May 19, 2008:
output the grayscale value in a text file 
*/

void output_grayscale_data(char outfile_name[])
{
  int i, j;
  FILE *fp;

  if (debug)
     printf("welcome to output_grayscale_data \n");

  fp = fopen(outfile_name, "w");
  if (fp == NULL)
  {
      printf("cannot open file %s for writing! \n", outfile_name);
      exit(303);
  }

  fprintf(fp, "%d %d\n", num_lines, num_points);

  for (i=0; i< num_lines; i++)
  {
    for (j=0; j< num_points-1; j++) //XXXXXX why -1?
    {
      fprintf(fp, "%d ", gray_array[begin_y+i][begin_x+j]);
    }
    fprintf(fp, "\n");
   }
   
   fclose(fp);
}

/* read rgb data into three arrays that hold r, g, b values, plus the height and width of the image */
int read_bmp_data (char input_bmp[]) 
{
  
  bool error;
  int i, j;
 
  error = bmp_read (input_bmp, &width, &height, &rarray, &garray, &barray );

  // January 21, 2008 
  original_rarray = (unsigned char *)malloc(sizeof(unsigned char)*width*height);
  original_garray = (unsigned char *)malloc(sizeof(unsigned char)*width*height);
  original_barray = (unsigned char *)malloc(sizeof(unsigned char)*width*height);
  original_gray   = (unsigned char *)malloc(sizeof(unsigned char)*width*height);

  memcpy(original_rarray, rarray, sizeof(unsigned char)*width*height);
  memcpy(original_garray, garray, sizeof(unsigned char)*width*height);
  memcpy(original_barray, barray, sizeof(unsigned char)*width*height);

  if (error)
  {
     printf("fatal error to read data %s \n", input_bmp); 
     exit(100);
  }

  if (debug)
      cout << "width = " << width << " height = " << height << endl;
  
   gray_array = (int **)malloc(sizeof(int *) * height);
   
   for (i=0; i< height; i++)
   {
      gray_array[i] = (int *)malloc(sizeof(int)*width);
      for (j=0; j< width; j++)
      {
        gray_array[i][j] = ((int)(0.2126*((float)(rarray[j + i * width])) + 0.7152*((float)(garray[j + i * width])) + 0.0722 * ((float)(barray[j + i * width]))));
        original_gray[i*width+j] = gray_array[i][j]; // April 4, 2011 keep original gray here
 
        if (debug == 9 && j%1000==0 && i%10==0)
           printf("i = %d, j= %d, gray_array[i][j] = %d rvalue = %d \n", i, j, gray_array[i][j], (int)(rarray[j+i*width]));
      }
   }
  return (1);
}

// Using ITR formula converting rgb to grayscale.
int convert2gray(rgb_point rgb)
{
  int gray_value;

  return ((int)(0.2126*((float)(rgb.value_r)) + 0.7152*((float)(rgb.value_g)) + 0.0722 * ((float)(rgb.value_b))));
}


/* August 25, 2007
   Computing statistics for a lamina. 

   begin_pt, end_pt, peak_pt, and sing are set already in the 
   get_initial_varves, this procedure will set other attributes of a varve.
or a sample of n values the sample kurtosis is

    g_2 = \frac{m_4}{m_{2}^2} -3 = \frac{n\,\sum_{i=1}^n (x_i - \overline{x})^4}{\left(\sum_{i=1}^n (x_i - \overline{x})^2\right)^2} - 3 

where m4 is the fourth sample moment about the mean, m2 is the second sample moment about the mean (that is, the sample variance), xi is the ith value, and \overline{x} is the sample mean.

*/
void get_varve_attributes(t_varve *a_varve)
{
  int gray_total = 0;
  int i;
  int sample_sq = 0;
  double  total_dev_2, total_dev_4;

  a_varve->width = a_varve->end_pt.pixel_num - a_varve->begin_pt.pixel_num; 

  if (a_varve->width == 0)
  {
    printf(" get_varve_attributes: begin point and end point cannot be the same for a varve \n");
    print_varve_point(*a_varve);
    output_rgb_array(data, num_points, "datafile.txt");
    output_rgb_array(peaks, num_peaks, "peaks.txt");
    exit(2000);
  }

  for (i=a_varve->begin_pt.pixel_num; i<=a_varve->end_pt.pixel_num; i++)
  {
    gray_total += data[i-begin_x].value_gray;
    sample_sq += (data[i-begin_x].value_gray * data[i-begin_x].value_gray);
  }

  a_varve->mean = (float) (gray_total) / (float) (a_varve->width+1);

  a_varve->sd = sqrt(sample_sq/(a_varve->width+ 1) - a_varve->mean*a_varve->mean); 

  /* kurtosis */
  total_dev_4 = 0.0;
  total_dev_2 = 0.0;
  for (i=a_varve->begin_pt.pixel_num; i<=a_varve->end_pt.pixel_num; i++)
  {
    total_dev_4 += pow((data[i-begin_x].value_gray - a_varve->mean), 4);
    total_dev_2 += pow((data[i-begin_x].value_gray - a_varve->mean), 2);
  }

  
   a_varve->kurtosis = (a_varve->width + 1)*total_dev_4 /(total_dev_2*total_dev_2) - 3;
 
  a_varve->median = 0;
  a_varve->confidence = 1;

}

/* March 22, 2010
   Need to get maximal gain betweek black and first peak in the
   light band.
   return index in the data array, not pixel num
*/

int get_division(rgb_point a_pt, rgb_point b_pt)
{
 /*
   the border point will give the max gain of areas;
 */

  int i, j;
  int max_gain = 0;
  int left_gain = 0, right_gain = 0, pt_gain = 0;
  int border_px;

  if (debug == 8)
  {
    printf("get_border: points::::\n");
    print_rgb_point(a_pt);
    print_rgb_point(b_pt);
  }

  for (i=a_pt.pixel_num-begin_x; i<=b_pt.pixel_num-begin_x; i++)
  {
    left_gain = 0;
    for (j=a_pt.pixel_num-begin_x; j<i; j++)
    {
       left_gain  += abs(b_pt.value_gray - data[j].value_gray);
    }

    if(debug == 9)
      printf("this is left gain: %d %d \n", i, left_gain);

    right_gain = 0;
    for (j=i; j<=b_pt.pixel_num-begin_x; j++)
    {
       right_gain  += abs(data[j].value_gray - a_pt.value_gray);
    }

    if(debug == 9)
      printf("this is right gain: %d %d \n", i, right_gain);

    pt_gain=left_gain + right_gain;
    if (pt_gain > max_gain)
    {
       max_gain = pt_gain;
       border_px = i;
    }
  }
  
  return border_px;
}

// find out whether there is a white peak between two points.
// return 0 if no, else return pt.pixel_num;
// March 25, 2010
int have_white_peak(rgb_point a_pt, rgb_point b_pt)
{
 int white_noise = 3; // difference < 2 considered white noise
 int cur_peak_indx;
 int cur_peak;
 int first_peak = 0;
 int rise = 0;

 if(debug)
 {
    	printf("welcome to have_white_peak \n");
	print_rgb_point(a_pt);
	print_rgb_point(b_pt);
 }

 
 int i= a_pt.pixel_num-begin_x;

 if (threshold_peak/8 > white_noise)
     white_noise = threshold_peak/8;

 cur_peak = data[i].value_gray;

  // find the first flank of the peak;
 i++;
 while (i <= b_pt.pixel_num-begin_x && first_peak == 0)
 {

   if (data[i].value_gray > cur_peak + white_noise)
   {
     cur_peak = data[i].value_gray;
     cur_peak_indx = i;
     rise = 1;
     first_peak = 1;
   }
   
   i++;
 }

 if (first_peak)
 {
 	// if pixel value keeps increasing
   while (data[i].value_gray > cur_peak
	   && i <= b_pt.pixel_num - begin_x)
   {
     cur_peak = data[i].value_gray;
     cur_peak_indx = i;
     i++;
   }

   return i-1;

 } else
    return 0;
}


int get_boundary(rgb_point a_pt, rgb_point b_pt)
{
  rgb_point tmp_pt;
  int a_div;

  if (debug == 8)
    {
  	printf("welcome to get_boundary \n");
	print_rgb_point(a_pt);
	print_rgb_point(b_pt);
    }

   // use simple algorithm if two points is less than 10 pixel apart
  //if (b_pt.pixel_num - a_pt.pixel_num < 10) 
	//return get_division(a_pt, b_pt);

// March 28, 2010: it is not necessary for BOS
  a_div = get_division(a_pt, b_pt);


  // first determine which one is black peak
  // March 27, 2010: assume that we do not need to do further
  // process is black band is on the left (winter to summer)
  if (a_pt.value_gray > b_pt.value_gray)
  { // b_pt is black
    
    int a_peak; 
    copy_rgb_point(&a_pt, data[a_div]);
    a_peak = have_white_peak(a_pt, b_pt);
    
    if (a_peak == 0)
    {
	return a_div;
     }
    while ( a_peak)
    {
       // a_peak = 0 if no peak, else return pixel_num of the peak
      copy_rgb_point(&a_pt, data[a_peak]);
      a_div = get_division(a_pt, b_pt);
      copy_rgb_point(&a_pt, data[a_div]);
      a_peak = have_white_peak(a_pt, b_pt);
     }
  } 

  return a_div;
}

/* return index of data array, not pixel num */
int get_boundary_simple(rgb_point a_pt, rgb_point b_pt)
{
  int i;
  float  half_value;
 
  if (debug && get_border_welcome)
  {
     cout << "welcome to get_boundary! " << endl; 
     get_border_welcome = 0;
  }

  if (debug == 9)
  {
    printf("get_boundary: points::::\n");
    print_rgb_point(a_pt);
    print_rgb_point(b_pt);
  }
  
  i = a_pt.pixel_num - begin_x; // or while data[i] < a_pt.pixel_num i++ 
  half_value = (a_pt.value_gray + b_pt.value_gray)/2;  

  if (a_pt.value_gray > b_pt.value_gray) /* down slope */
  {
    while ((float)data[i].value_gray > half_value && i < b_pt.pixel_num)
    {
       i++;
    }
  }
  else /* up slope */
  {
    while ((float)data[i].value_gray <= half_value && i < b_pt.pixel_num)
    {
       i++;
    }
  }
  
  if (debug && get_border_welcome)
     cout << "exiting get_boundary" << endl;

  return i;
}

/* July 24, 2007
   get initial peaks from the data and store peak information in array peaks. 
*/
void get_initial_peaks( int p_value_threshold) 
 { 
   int prev_trend, cur_trend; /* 1 up, -1 down */
   //int cur_peak_pixel, prev_peak_pixel;
   //int cur_peak, prev_peak;
   rgb_point cur_peak, prev_peak; // December 28, 2007
   int find_1_peak = 0;
   int i, j;

   if (debug) 
     cout << "welcome to get_initial_peaks: " << p_value_threshold << endl;

   i = 0; 
   copy_rgb_point(&prev_peak, data[i]);
   find_1_peak = 0;

   while(i < num_points && find_1_peak == 0)
   {
     i++;
     if (abs(prev_peak.value_gray - data[i].value_gray ) >= p_value_threshold)
        find_1_peak = 1;
   }
    
   if (prev_peak.value_gray > data[i].value_gray ) 
      cur_trend = -1; /* dark to bright downward */
   else
      cur_trend = 1; /* bright to dark upward */
  
   copy_rgb_point(&cur_peak, data[i]);

   j=0;
   i++;
   while (i<num_points)
   {
     if (cur_trend == 1 )
     {
        if (data[i].value_gray > cur_peak.value_gray) /* keep increase, not peak yet */
        {
            copy_rgb_point(&cur_peak, data[i]);
        }
        else
        {
            if (cur_peak.value_gray - data[i].value_gray > p_value_threshold ) 
            {
     	       	peaks = (rgb_point *)realloc(peaks, sizeof(rgb_point)*(j+1));
                copy_rgb_point(&(peaks[j]), prev_peak);

               	j++;

                copy_rgb_point(&prev_peak, cur_peak);
                copy_rgb_point(&cur_peak, data[i]);

               	cur_trend = -1;
            }
        }
     } else /* cur_trend == -1 */
     {
        if (data[i].value_gray < cur_peak.value_gray)
        {
            copy_rgb_point(&cur_peak, data[i]);
        }
        else
        {
            if ( data[i].value_gray - cur_peak.value_gray > p_value_threshold ) 
            {
     	       	peaks = (rgb_point *)realloc(peaks, sizeof(rgb_point)*(j+1));
                copy_rgb_point(&(peaks[j]), prev_peak);
               	j++;

                copy_rgb_point(&prev_peak, cur_peak);
                copy_rgb_point(&cur_peak, data[i]);

               	cur_trend = 1;
	    }
        }
     } /* end cur_trend = -1 */
     
     i++;
   } /* end while */

   /* the last peak */
   peaks = (rgb_point *)realloc(peaks, sizeof(rgb_point)*(j+1));
   copy_rgb_point(&(peaks[j]), prev_peak);

// March 13, 2010... copy the last data point as the last peak.
   if (abs(prev_peak.value_gray - cur_peak.value_gray) > p_value_threshold )  {
   	j++;
   	peaks = (rgb_point *)realloc(peaks, sizeof(rgb_point)*(j+1));
   	copy_rgb_point(&(peaks[j]), data[num_points-1]);
   }

   num_peaks = j+1;

/* August 26, 2007: this is more performance efficient to remove the first point which is not a peak */
/*
   for (i=0; i<num_peaks; i++)
     copy_rgb_point(&(peaks[i]), peaks[i+1]);

   num_peaks -= 1;
*/

   //if (debug)
   if (debug == 8)
     print_rgb_point_array(peaks, num_peaks);

   if (debug)
     cout << "  exiting get_initial_peaks: number of peaks " << num_peaks << endl;


 }

/* Dec 6, 2009
   from closed2 to varve_image
   cleaned image contains interested area with cleaned laminas
information in the raw pixel image.
  If pixel is black, then it belongs to black lamina.
  if lamina boundary is where color changes.
  peak point will be found in the cleaned image.
*/

void get_cleaned_varve_image()
{
  int i, j, k;
  int max_gray, pvalue;
  rgb_point bgpt, edpt;
  rgb_point pkpt;
  int mysign;

  for (j=begin_y; j<=end_y; j++)
  {
    i=begin_x; 
    pvalue =  (int)rarray[j*width+i];

    if (pvalue == 0)
       mysign = DARK;
    else
       mysign = LIGHT;

    bgpt.pixel_num = i;

// I do not think next 4 lines are useful 
    bgpt.value_r = (int)original_rarray[j*width+i];
    bgpt.value_g = (int)original_garray[j*width+i];
    bgpt.value_b = (int)original_barray[j*width+i];
    //bgpt.value_gray = (int)gray_array[j][i];
    bgpt.value_gray = (int)original_gray[j*width+i];

    pkpt.pixel_num = i;

    max_gray = bgpt.value_gray;

    k=-1;
    while ( i < end_x)
    {
      if ( pvalue ==  (int)rarray[j*width+i]) // same laminae
      { // need to compare to original gray array.
        if ( (max_gray < (int)original_gray[j*width+i] && mysign == LIGHT) || //max
            (max_gray > (int)original_gray[j*width+i] && mysign == DARK)) //min 
        {
	 max_gray = (int)original_gray[j*width+i];
         pkpt.pixel_num = i;
        }
      } else { // started a new lamina
        edpt.pixel_num = i-1;
        edpt.value_r = (int)original_rarray[j*width+i-1];
        edpt.value_g = (int)original_garray[j*width+i-1];
        edpt.value_b = (int)original_barray[j*width+i-1];
        edpt.value_gray = (int)original_gray[j*width+i-1];

        pkpt.value_r = (int)original_rarray[j*width+pkpt.pixel_num];
        pkpt.value_g = (int)original_garray[j*width+pkpt.pixel_num];
        pkpt.value_b = (int)original_barray[j*width+pkpt.pixel_num];
        pkpt.value_gray = (int)original_gray[j*width+pkpt.pixel_num];

	k++;

        //cleaned_varve_image.varve_line[j-begin_y].varve_pt = (t_varve *)realloc(cleaned_varve_image.varve_line[j-begin_y].varve_pt, sizeof(t_varve)*(k+1));

        copy_rgb_point(&(cleaned_varve_image.varve_line[j-begin_y].varve_pt[k].begin_pt), bgpt);
        copy_rgb_point(&(cleaned_varve_image.varve_line[j-begin_y].varve_pt[k].peak_pt), pkpt);
        copy_rgb_point(&(cleaned_varve_image.varve_line[j-begin_y].varve_pt[k].end_pt), edpt);
        cleaned_varve_image.varve_line[j-begin_y].varve_pt[k].sign = mysign;

	  // start a new laminae
        bgpt.pixel_num = i;
        bgpt.value_r = (int)original_rarray[j*width+i];
        bgpt.value_g = (int)original_garray[j*width+i];
        bgpt.value_b = (int)original_barray[j*width+i];
        bgpt.value_gray = (int)original_gray[j*width+i];
        
        max_gray = bgpt.value_gray;
        pkpt.pixel_num = i;

        pvalue =  (int)rarray[j*width+i];
        if (pvalue == 0)
           mysign = DARK;
        else
           mysign = LIGHT;

      }

      i++;
    }

    edpt.pixel_num = i-1;
    edpt.value_r = (int)original_rarray[j*width+i-1];
    edpt.value_g = (int)original_garray[j*width+i-1];
    edpt.value_b = (int)original_barray[j*width+i-1];
    edpt.value_gray = (int)original_gray[j*width+i-1];

    pkpt.value_r = (int)original_rarray[j*width+pkpt.pixel_num];
    pkpt.value_g = (int)original_garray[j*width+pkpt.pixel_num];
    pkpt.value_b = (int)original_barray[j*width+pkpt.pixel_num];
    pkpt.value_gray = (int)original_gray[j*width+pkpt.pixel_num];

    k++;
    copy_rgb_point(&(cleaned_varve_image.varve_line[j-begin_y].varve_pt[k].begin_pt), bgpt);
    copy_rgb_point(&(cleaned_varve_image.varve_line[j-begin_y].varve_pt[k].peak_pt), pkpt);
    copy_rgb_point(&(cleaned_varve_image.varve_line[j-begin_y].varve_pt[k].end_pt), edpt);
    cleaned_varve_image.varve_line[j-begin_y].varve_pt[k].sign = mysign;

    cleaned_varve_image.varve_line[j-begin_y].num_varves = k+1;
  }
 
  if (debug)
     printf("exiting get_cleaned_varve_image ... num_varves = %d \n", k);
}

/* August 19, 2007
   get varves by peak and data of a line
*/
t_varve_line get_initial_varves () 
 { 
   int i, k=0;
   t_varve_line tvl; 

   if (debug)
      cout << "welcome to get_initial_varves" << endl;

   tvl.varve_pt = (t_varve *)malloc(sizeof(t_varve)*1);
   init_varve_point(&(tvl.varve_pt[0]));
   copy_rgb_point(&(tvl.varve_pt[0].begin_pt), data[0]);
   
   //if (peaks[0].value_gray > data[0].value_gray) 
   if (peaks[0].value_gray > peaks[1].value_gray) 
      tvl.varve_pt[0].sign = LIGHT;
   else
      tvl.varve_pt[0].sign = DARK;

   k = 0;
   for (i=0; i<num_peaks - 1; i++)
   {
       int j = get_boundary(peaks[i], peaks[i+1]); 
       copy_rgb_point(&(tvl.varve_pt[k].end_pt), data[j]);
       copy_rgb_point(&(tvl.varve_pt[k].peak_pt), peaks[i]);
 	
/*
for (int ii=tvl.varve_pt[k].begin_pt.pixel_num; ii<tvl.varve_pt[k].end_pt.pixel_num; ii++) {
      print_rgb_point(data[ii]);
} 
print_varve_point(tvl.varve_pt[k]);
*/

       get_varve_attributes(&(tvl.varve_pt[k]));
       k++;
       tvl.varve_pt = (t_varve *)realloc(tvl.varve_pt, sizeof(t_varve)*(k+1));
       init_varve_point(&(tvl.varve_pt[k]));
       if (data[j].pixel_num < begin_x){
		printf("this is not right: begin_pixel < begin_x \n");
		print_rgb_point(data[j]);
	}

       copy_rgb_point(&(tvl.varve_pt[k].begin_pt), data[j]);
       tvl.varve_pt[k].sign = tvl.varve_pt[k-1].sign*(-1);
   }
  
// last point as the end point for the lasts varve
   copy_rgb_point(&(tvl.varve_pt[k].peak_pt), peaks[i]);
   copy_rgb_point(&(tvl.varve_pt[k].end_pt), data[num_points-1]);
   tvl.num_varves = k+1; 

   // if (debug)
    if (debug == 9)
    {
	printf(" ****get initial varves: \n");
   	print_varve_line(tvl);
    }

   if (debug)
     cout << "exiting get initial varves " << tvl.num_varves << endl;

   return tvl;
 }


/* September 28, 2007 
   
*/
void get_varve_image()
{
  int i;
  char outfile_name[128];
  
  for (i=0; i<varve_image.num_lines; i++)
  {
    int j;
    for (j=begin_x; j<=end_x; j++) // populate data
    {
     	data[j-begin_x].pixel_num = j;
     	data[j-begin_x].value_r = (int) rarray[j+(begin_y+i)*width];   
     	data[j-begin_x].value_g = (int) garray[j+(begin_y+i)*width];   
     	data[j-begin_x].value_b = (int) barray[j+(begin_y+i)*width];   
     	data[j-begin_x].value_gray = convert2gray(data[j-begin_x]);   
    }

    /* get initial peaks for this line */
     get_initial_peaks(threshold_peak);

     if (debug == 8)
     {
        sprintf(outfile_name, "%s_line_%d_peaks.txt", infile_stub, i);
        output_rgb_array(peaks, num_peaks, outfile_name);
        //output_rgb_array(data, end_x-begin_x+1, outfile_name);
     }

    /* get initial varves for this line */
     varve_image.varve_line[i] = get_initial_varves();
  } // end for num_lines 

  if (debug)
     printf("existing get_vare_image \n");
}

/* sort the varves by their pixel positions so that they can be checked or correlated */
void q_sort(t_varve_position *varve_p, int left, int right)
{
  int pivot, l_hold, r_hold;
  t_varve_position pivot_position;

  l_hold = left;
  r_hold = right;
  pivot = varve_p[left].pixel_num;
  copy_varve_position(&pivot_position, varve_p[left]);
  while (left < right)
  {
    while ((varve_p[right].pixel_num >= pivot) && (left < right))
      right--;
    if (left != right)
    {
      copy_varve_position(&(varve_p[left]), varve_p[right]);
      left++;
    }
    while ((varve_p[left].pixel_num <= pivot) && (left < right))
      left++;
    if (left != right)
    {
      copy_varve_position(&(varve_p[right]), varve_p[left]);
      right--;
    }
  }
  copy_varve_position(&(varve_p[left]), pivot_position); 
  pivot = left;
  left = l_hold;
  right = r_hold;
  if (left < pivot)
    q_sort(varve_p, left, pivot-1);
  if (right > pivot)
    q_sort(varve_p, pivot+1, right); 
}

void sort_varve_positions(t_varve_position *varve_p, int *num_varve_p)
{
  int i, j;

  q_sort(varve_p, 0, *num_varve_p-1);

  j=0;
  for (i=0; i<*num_varve_p; i++)
  {
    if (varve_p[i].pixel_num != varve_p[j].pixel_num)
    {
      copy_varve_position(&(varve_p[++j]), varve_p[i]);
// print_varve_position(varve_p[j]);
    }
  }

  *num_varve_p = j;
}

/* project all varves to the axis of pixel numbers so that we can make sure that 
every varve is examined or we can examine varves by the position of varve's pixel */
void project_varves(t_varve_image vi)
{
 int i, j, k;
 
 num_varve_positions=0;
 for (i=0; i< vi.num_lines; i++)
 {
   for (j=0; j<vi.varve_line[i].num_varves; j++)
   {
        k = num_varve_positions;
        varve_positions = (t_varve_position *)realloc(varve_positions, sizeof(t_varve_position)*(num_varve_positions+1));

         // insert the new position
        varve_positions[k].pixel_num = vi.varve_line[i].varve_pt[j].peak_pt.pixel_num;
        varve_positions[k].section_num = i; 
        varve_positions[k].varve_num = j;
        varve_positions[k].checked = 0;
        num_varve_positions++;
    } // end for section
 } // end for varves
}

// November 25, 2007 this is for show do local contrast
// Dec 6, 2009: it seems that we can change this to copy gray to primary array.
void insert_varve_gray(t_varve_image s_vi, int target_image)
// target_image == 0: working rgb arrays, 1, original arrays
{
  int i, j,  k;

  if(debug)
    cout << "welcome to insert_varve_gray: num_lines: " << s_vi.num_lines << endl;


  for (i=0; i<s_vi.num_lines; i++) 
  {
     for (j=0; j<s_vi.varve_line[i].num_varves; j++)
        {
          for (k= s_vi.varve_line[i].varve_pt[j].begin_pt.pixel_num;
                   k < s_vi.varve_line[i].varve_pt[j].end_pt.pixel_num;
                   k++ )
          {
            if (target_image == 0)
            {
            rarray[k+(i+begin_y)*width] = (unsigned char) (gray_array[i+begin_y][k]);
            garray[k+(i+begin_y)*width] = (unsigned char) (gray_array[i+begin_y][k]);
            barray[k+(i+begin_y)*width] = (unsigned char) (gray_array[i+begin_y][k]);
            } else {
            original_rarray[k+(i+begin_y)*width] = (unsigned char) (gray_array[i+begin_y][k]);
            original_garray[k+(i+begin_y)*width] = (unsigned char) (gray_array[i+begin_y][k]);
            original_barray[k+(i+begin_y)*width] = (unsigned char) (gray_array[i+begin_y][k]);
           
            }
          } // each varves
       } // end each varve line
  } // end each varve image
}

/* August 19, 2007
   write initial varves back to image
   October 9, 2007: assume that each line is 1 pixel wide.
*/

void insert_varve_image(t_varve_image s_vi, int target_image)
/* write the varves data back to r[g,b]arrays so that we can write rgb array back to bmp file 
   target_image: 0 is working (rarray), 1 is original (original_rarray).
*/
{
  int i, j,  k;

  if(debug)
    cout << "welcome to insert_varve_image: num_lines: " << s_vi.num_lines << endl;


  for (i=0; i<s_vi.num_lines; i++) 
  {
     for (j=0; j<s_vi.varve_line[i].num_varves; j++)
        {
          if (s_vi.varve_line[i].varve_pt[j].begin_pt.pixel_num < begin_x)
	  {
	    printf("the number of lines: %d num_varves %d \n", s_vi.num_lines, s_vi.varve_line[i].num_varves);
 	    printf("this is a problem: line %d, varve pt %d begin_pt.pixel_num %d \n",
		    i, j, s_vi.varve_line[i].varve_pt[j].begin_pt.pixel_num);
	  }
          for (k= s_vi.varve_line[i].varve_pt[j].begin_pt.pixel_num;
                   k < s_vi.varve_line[i].varve_pt[j].end_pt.pixel_num;
                   k++ )
          {
            if (target_image == 0)
            {
              if (s_vi.varve_line[i].varve_pt[j].sign == LIGHT)
              {
                 rarray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                 garray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                 barray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                 gray_array[i+begin_y][k] = 255;
              } else
              {
                 rarray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                 garray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                 barray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                 gray_array[i+begin_y][k] = 0;
              }
            } else
            {
              if (s_vi.varve_line[i].varve_pt[j].sign == LIGHT)
              {
                 original_rarray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                 original_garray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                 original_barray[k+(i+begin_y)*width] = (unsigned char) ( 255);
              } else
              {
                 original_rarray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                 original_garray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                 original_barray[k+(i+begin_y)*width] = (unsigned char) ( 0);
              }
            }
/*
            rarray[k+(i+begin_y)*width] = (unsigned char) (s_vi.varve_line[i].varve_pt[j].peak_pt.value_gray); 
            garray[k+(i+begin_y)*width] = (unsigned char) (s_vi.varve_line[i].varve_pt[j].peak_pt.value_gray); 
            barray[k+(i+begin_y)*width] = (unsigned char) (s_vi.varve_line[i].varve_pt[j].peak_pt.value_gray); 
*/
          } // each varves
       } // end each varve line
  } // end each varve image
}

/* October 3, 2007
  vertical direction means physical direction, it is x direction for the implementation.
  vertical moving average will filter out the anomalies in the vertical direction.  
  This routine will perform on the rgb array directly.
*/

void x_vertical_mv_ave( int num_p)
{
  int i, j, k;

  if(debug)
    cout << "welcome to x_vertical mv ave : " << num_p << endl;

  if (num_p < 2)
  {
    printf(" number of pixel to be averaged has to be greater than 1 !\n");
    exit(405);
  }

  for (i=begin_y; i<end_y; i++)
  {
     int value_ave;
     for (j = begin_x; j< end_x; j++) 
     {
       int total_value = 0;
       for (k=j; k< j+num_p; k++)
       {
          total_value += (int)(rarray[k+i*width]);
       }

       value_ave = total_value/num_p;
       rarray[j+i*width] = (unsigned char) (value_ave);
       garray[j+i*width] = (unsigned char) (value_ave);
       barray[j+i*width] = (unsigned char) (value_ave);
       gray_array[i][j] = ((int)(0.2126*((float)(rarray[j + i * width])) + 0.7152*((float)(garray[j + i * width])) + 0.0722 * ((float)(barray[j + i * width]))));

     } // end for j <= x_end
  } // end for y
}

/* August 1, 2009
   Define a weighted moving average filter. It is assumed that the width of filter
  will be always odd number of pixels. THe filter is generated as follows
   - first iteration of the filter will be the same as simple average moving average. 
     However, the data will be central mv, i. e., 
	mv(i) = sum of data(i-width/2) to data(i+ width/2)
   - the second interation of the filter will be:
	mv2(i) = sum of mv(i-width/2) to mv(i+width/2).
   -    mvn(i) = sum of mvn-1(i-width/2) to mvn-1(i+width/2).
*/

void weighted_y_mv(int f_width, int level, int *y_begin, int *y_end)
// assume that f_width is odd
{
   int i, j, k, ii;
   int total_r, total_g, total_b, total_gray;
   int *r_ave, *g_ave, *b_ave, *gray_ave;
   int y_start, y_stop;
   int half_width = f_width/2;

   if (debug)
	printf( "welcome to weighted_y_mv f_width = %d level = %d\n", f_width, level);

   y_start = begin_y;
   y_stop   = end_y;

   r_ave = (int *)malloc(sizeof(int) * (y_stop - y_start + 1));
   g_ave = (int *)malloc(sizeof(int) * (y_stop - y_start + 1));
   b_ave = (int *)malloc(sizeof(int) * (y_stop - y_start + 1));
   gray_ave = (int *)malloc(sizeof(int) * (y_stop - y_start + 1) );

   for (k=1; k <= level; k++)
   {
	y_start += (half_width);
        y_stop   -= (half_width);

        if (y_stop - y_start >= f_width)
	{
           for (i=begin_x; i<end_x; i++)
	   {
	      for (j = y_start; j< y_stop; j++)
	      {
        	total_r = 0;
		total_g = 0;
		total_b = 0;
		total_gray = 0;
      
		int jj = j - (half_width); // so that j will be the center.
                for (ii = jj; ii< jj+f_width; ii++)
		{
		    total_r += rarray[i+ii*width];
		    total_g += garray[i+ii*width];
		    total_b += barray[i+ii*width];
		    total_gray += gray_array[ii][i];
		}

         	r_ave[j-y_start] = total_r / f_width;  
         	g_ave[j-y_start] = total_g / f_width;  
         	b_ave[j-y_start] = total_b / f_width;  
         	gray_ave[j-y_start] = total_gray / f_width;  
	      }

		// update arrays with aved values
	      for (j = y_start; j< y_stop; j++)
	      {
	         rarray[i+j*width] = (unsigned char) r_ave[j-y_start]; 
	         garray[i+j*width] = (unsigned char) g_ave[j-y_start]; 
	         barray[i+j*width] = (unsigned char) b_ave[j-y_start]; 
	         gray_array[j][i] = gray_ave[j-y_start]; 
	      }
	   } 
	}
   }
	
   *y_begin = y_start;
   *y_end = y_stop;

   free(r_ave);
   free(g_ave);
   free(b_ave);
   free(gray_ave);

   if (debug)
	printf("Bye from weighted_y_mv()");   
}

/* March 12, 2010
   modified from weighted_y
   Define a weighted moving average filter. It is assumed that the width of filter
  will be always odd number of pixels. THe filter is generated as follows
   - first iteration of the filter will be the same as simple average moving average. 
     However, the data will be central mv, i. e., 
	mv(i) = sum of data(i-width/2) to data(i+ width/2)
   - the second interation of the filter will be:
	mv2(i) = sum of mv(i-width/2) to mv(i+width/2).
   -    mvn(i) = sum of mvn-1(i-width/2) to mvn-1(i+width/2).
*/

void weighted_x_mv(int f_width, int level, int *x_begin, int *x_end)
// assume that f_width is odd
{
   int i, j, k, ii;
   int total_r, total_g, total_b, total_gray;
   int *r_ave, *g_ave, *b_ave, *gray_ave;
   int x_start, x_stop;
   int half_width = f_width/2;

   if (debug)
	printf( "welcome to weighted_x_mv f_width = %d level = %d\n", f_width, level);

   x_start = begin_x;
   x_stop   = end_x;

   r_ave = (int *)malloc(sizeof(int) * (x_stop - x_start + 1));
   g_ave = (int *)malloc(sizeof(int) * (x_stop - x_start + 1));
   b_ave = (int *)malloc(sizeof(int) * (x_stop - x_start + 1));
   gray_ave = (int *)malloc(sizeof(int) * (x_stop - x_start + 1) );

   for (k=1; k <= level; k++)
   {
	x_start += (half_width);
        x_stop   -= (half_width);
        if (x_stop - x_start >= f_width)
	{
           for (i=begin_y; i<end_y; i++)
	   {
	      for (j = x_start; j< x_stop; j++)
	      {
        	total_r = 0;
		total_g = 0;
		total_b = 0;
		total_gray = 0;
      
		int jj = j - (half_width); // so that j will be the center
                for (ii = jj; ii< jj+f_width; ii++)
		{
		    total_r += rarray[ii+i*width];
		    total_g += garray[ii+i*width];
		    total_b += barray[ii+i*width];
		    total_gray += gray_array[i][ii];
		}

         	r_ave[j-x_start] = total_r / f_width;  
         	g_ave[j-x_start] = total_g / f_width;  
         	b_ave[j-x_start] = total_b / f_width;  
         	gray_ave[j-x_start] = total_gray / f_width;  
	      }

		// update arrays with aved values
	      for (j = x_start; j< x_stop; j++)
	      {
	         rarray[j+i*width] = (unsigned char) r_ave[j-x_start]; 
	         garray[j+i*width] = (unsigned char) g_ave[j-x_start]; 
	         barray[j+i*width] = (unsigned char) b_ave[j-x_start]; 
	         gray_array[i][j] = gray_ave[j-x_start]; 
	      }
	   } 
	}
   }
	
   *x_begin = x_start;
   *x_end = x_stop;

   free(r_ave);
   free(g_ave);
   free(b_ave);
   free(gray_ave);

   if (debug)
	printf("Bye from weighted_x_mv()");   
}


/* September 9, 2007
   This routine is to find the varves that extented through the interested area. 
varves in the adjacent two lines are considered continueous if they have the following attributes:
   - they have to same sign (positive or negative);
   - they have overlapped in pixel number range;
   - one varve might be equavalent to more than one -- fork, or more than one varves may
          merge to one.
   - they have similar rgb values in the threshold range?? not implemented.
*/
int is_connected(int line_idx, t_varve vpt, int varve_sign, int *pixel_idx, int *pixel_begin, int *pixel_end)
{
  int i, j;
  int value_diff;

  i = line_idx;
  j = 0;

  *pixel_idx = 0;

  if (debug == 10)
  {
     printf("  line index **%d varve_sign %d pixel_begin %d pixel_end %d\n", line_idx, varve_sign, *pixel_begin, *pixel_end);
  }

    /* find the varve in the range */
  while ( (varve_image.varve_line[i].varve_pt[j].end_pt.pixel_num <= *pixel_begin 
             || varve_image.varve_line[i].varve_pt[j].checked) 
          && j < varve_image.varve_line[i].num_varves)
  {
     j++; 
  } 

     // case 1: either all varves were checked already or no overlapped range 
  if (j == varve_image.varve_line[i].num_varves 
        || varve_image.varve_line[i].varve_pt[j].begin_pt.pixel_num >= *pixel_end)
  {
     if (debug == 10)
         printf("    case 1 returned j = %d \n", j);
     return 0;
  }

     // case 2a and 2b: varve span over previous one and sign of varve[j] is the same as in one
  if ( varve_image.varve_line[i].varve_pt[j].end_pt.pixel_num >= *pixel_end )
  {
     if ( varve_image.varve_line[i].varve_pt[j].sign == varve_sign
         || abs(vpt.peak_pt.value_gray - varve_image.varve_line[i].varve_pt[j].peak_pt.value_gray) <= threshold_peak/2)
     {    
        *pixel_begin = varve_image.varve_line[i].varve_pt[j].begin_pt.pixel_num;
        *pixel_end = varve_image.varve_line[i].varve_pt[j].end_pt.pixel_num;
        *pixel_idx = j;
        varve_image.varve_line[i].varve_pt[j].checked = 1; 

        if (debug == 10)
        {
           printf( "    case 2 returned: j = %d  pixel begin %d pixel_end %d \n", j, *pixel_begin, *pixel_end);
        }

        return 1;
     } else
     {
       if (debug == 10)
          cout << "    case 2b, return 0 " << endl;
        return 0; 
     }
  } 
     // or case 3: overlap on the top: one of j and j+1 must connect to
  if ( varve_image.varve_line[i].varve_pt[j+1].begin_pt.pixel_num < *pixel_end
        &&  varve_image.varve_line[i].varve_pt[j+1].end_pt.pixel_num >= *pixel_end)
  {
     if ( varve_image.varve_line[i].varve_pt[j].sign == varve_sign
         || abs(vpt.peak_pt.value_gray - varve_image.varve_line[i].varve_pt[j].peak_pt.value_gray) <= threshold_peak/2)
     {
        *pixel_begin = varve_image.varve_line[i].varve_pt[j].begin_pt.pixel_num;
        *pixel_end = varve_image.varve_line[i].varve_pt[j].end_pt.pixel_num;
        *pixel_idx = j;
        varve_image.varve_line[i].varve_pt[j].checked = 1; 
        if (debug == 10)
        {
           printf( "    case 3a returned: j = %d  pixel begin %d pixel_end %d \n", j, *pixel_begin, *pixel_end);
        }
        return 1;
     } else
     {
        if (varve_image.varve_line[i].varve_pt[j+1].sign == varve_sign 
         || abs(vpt.peak_pt.value_gray - varve_image.varve_line[i].varve_pt[j].peak_pt.value_gray) <= threshold_peak/2)
       {
           j++;
          *pixel_begin = varve_image.varve_line[i].varve_pt[j].begin_pt.pixel_num;
          *pixel_end = varve_image.varve_line[i].varve_pt[j].end_pt.pixel_num;
          *pixel_idx = j;
          varve_image.varve_line[i].varve_pt[j].checked = 1; 

          if (debug == 10)
          {
            printf( "    case 3b returned: j = %d  pixel begin %d pixel_end %d \n", j, *pixel_begin, *pixel_end);
          }

          return 1;
        } else
        {
          if (debug == 10)
             printf("    case 3c impossible, return 0 \n");
          return 0;
        }
     }
  }

     // case 4: split to multiple varves 
     // we will find a closed peak value as connected one
  int case4_j = j;
  value_diff = 255;        
  while (varve_image.varve_line[i].varve_pt[j].end_pt.pixel_num < *pixel_end)
  {
     if (debug == 10)
     {
        printf("     case 4 \n");
        printf("vpt.peak_pt.value_gray = %d varve_pt[%d].end_pt.pixel_num = %d peak_pt.value_gray = %d \n", vpt.peak_pt.value_gray, j, 
                varve_image.varve_line[i].varve_pt[j].end_pt.pixel_num, 
                varve_image.varve_line[i].varve_pt[j].peak_pt.value_gray);
     }

     if ( value_diff > abs(vpt.peak_pt.value_gray -
                      varve_image.varve_line[i].varve_pt[j].peak_pt.value_gray))
     {

         value_diff = abs(vpt.peak_pt.value_gray - varve_image.varve_line[i].varve_pt[j].peak_pt.value_gray);
         *pixel_idx = j;
     }
     j++; 
  }
  
    // make sure all varves before returned one are marked checked.
// November 15, 2007 only marked the one connnected.

  varve_image.varve_line[i].varve_pt[*pixel_idx].checked = 1;

  *pixel_begin = varve_image.varve_line[i].varve_pt[*pixel_idx].begin_pt.pixel_num;
  *pixel_end = varve_image.varve_line[i].varve_pt[*pixel_idx].end_pt.pixel_num;

  if (debug == 10)
  {
      printf("        value_diff = %d j = %d, pixel_begin = %d pixel_end = %d \n", value_diff, *pixel_idx, *pixel_begin, *pixel_end);

  }

  if (debug == 10)
  {
     printf("    case 4: varve_sign %d pixel_begin %d pixel_end %d\n", varve_sign, *pixel_begin, *pixel_end);
  }
  
  return 1;
}

void mark_varve_position_checked( int position_idx, int varve_idx)
{
  int i, j, k, found;

  k = varve_idx;
  for (i=0; i<varve_image.num_lines; i++)
  {
     j=position_idx; 
     found = 0;
     while ( ! found  && j < num_varve_positions 
            && varve_positions[j].pixel_num <= wi.varve_line[i].varve_pt[k].peak_pt.pixel_num)
     {
       if (varve_positions[j].section_num == i
            && varve_positions[j].varve_num == final_varve_index[i][k] )
       {
          varve_positions[j].checked = 1;
          found = 1;
       }
       
       j++;
     }
  }
}


/* December 16, 2007 
  this is one of the major routine needed careful review.
  Algorithem: 
   the x values of peak points that were defined using one dimension routine are project
   to x axis and are sorted. Therefore, we have all possible positions of peak points 
   (laminae points that are defined in one dimension) in x axis. A purpose of this routine 
   is to examine how these laminae points are connected.

   We use the minimal point in x-axis as a pivot point, check how many points are connected
   with this point. We look both sides of this point in y axis. We mark the point checked if
   a point is connected to its adjacent point in y axis.
 
*/
void get_laminae_image()
{
  int i, j, k, position_j = 0, ij;
  t_varve reference_varve_pt;
 
  if (debug)
     printf("welcome to get final varve image \n");

   project_varves(varve_image);
   
   if (debug)
     printf("after project varve position : num_varve_positions=%d\n", num_varve_positions);

   sort_varve_positions(varve_positions, &num_varve_positions);
   if (debug)
     printf("after sorting varve position : num_varve_positions=%d\n", num_varve_positions);

    // initialize the wi 
  for (i=0; i<wi.num_lines; i++)
  {
    wi.varve_line[i].num_varves = varve_image.varve_line[i].num_varves;
    for (j=0; j<wi.varve_line[i].num_varves; j++)
    {
      init_varve_point(&(wi.varve_line[i].varve_pt[j]));
    }
  }

   // nov 10, 2007
  j=-1; // j is the final varve index
  position_j = 0;
  while ( position_j < num_varve_positions)
  {
    if (varve_positions[position_j].checked == 0 )
    {
     int p_line = varve_positions[position_j].section_num;
     int p_varve_num = varve_positions[position_j].varve_num;
     int p_pixel_num = varve_positions[position_j].pixel_num;

     int pixel_begin = varve_image.varve_line[p_line].varve_pt[p_varve_num].begin_pt.pixel_num;
     int pixel_end = varve_image.varve_line[p_line].varve_pt[p_varve_num].end_pt.pixel_num;
     int varve_sign = varve_image.varve_line[p_line].varve_pt[p_varve_num].sign;

     varve_positions[position_j].checked = 1;
     copy_varve_point(&(reference_varve_pt), varve_image.varve_line[p_line].varve_pt[p_varve_num]);
     ++j;
     num_line_extended = (int *)realloc(num_line_extended, sizeof(int)*(j+1));
     num_line_extended[j] = 1;

     // copy the pivot varve 
     copy_varve_point(&(wi.varve_line[p_line].varve_pt[j]), varve_image.varve_line[p_line].varve_pt[p_varve_num]);

     final_varve_index[p_line][j] = p_varve_num; 
     wi.varve_line[p_line].num_varves = j+1;
     if (wi.varve_line[p_line].varve_pt[p_varve_num].sign == LIGHT)
     {
         wi.varve_line[p_line].varve_pt[p_varve_num].peak_pt.value_r = 255;
         wi.varve_line[p_line].varve_pt[p_varve_num].peak_pt.value_g = 255;
         wi.varve_line[p_line].varve_pt[p_varve_num].peak_pt.value_b = 255;
         wi.varve_line[p_line].varve_pt[p_varve_num].peak_pt.value_gray = 255;
      } else
      {
         wi.varve_line[p_line].varve_pt[p_varve_num].peak_pt.value_r = 0;
         wi.varve_line[p_line].varve_pt[p_varve_num].peak_pt.value_g = 0;
         wi.varve_line[p_line].varve_pt[p_varve_num].peak_pt.value_b = 0;
         wi.varve_line[p_line].varve_pt[p_varve_num].peak_pt.value_gray = 0;
      }

       // left half
     copy_varve_point(&(reference_varve_pt), varve_image.varve_line[p_line].varve_pt[p_varve_num]);
     for(i=p_line-1; i>=0; i--)
     {
        wi.varve_line[i].num_varves = j+1;
        final_varve_index[i][j] = 0; //initialize it
        if ( is_connected(i, reference_varve_pt, varve_sign, &k, &pixel_begin, &pixel_end))
        {
// QQQQQ
           num_line_extended[j] ++;  
           final_varve_index[i][j] = k; 
           copy_varve_point(&(reference_varve_pt), varve_image.varve_line[i].varve_pt[k]);
           copy_varve_point(&(wi.varve_line[i].varve_pt[j]), varve_image.varve_line[i].varve_pt[k]);
           wi.varve_line[i].varve_pt[j].begin_pt.pixel_num = pixel_begin;
           wi.varve_line[i].varve_pt[j].end_pt.pixel_num = pixel_end;
           wi.varve_line[i].varve_pt[j].sign = varve_sign; 
        } 
     } // end for i num_lines
     
       // right half
     copy_varve_point(&(reference_varve_pt), varve_image.varve_line[p_line].varve_pt[p_varve_num]);
     for (i=p_line+1; i<varve_image.num_lines; i++)
     {
        wi.varve_line[i].num_varves = j+1;
        final_varve_index[i][j] = 0; //initialize it
        if ( is_connected(i, reference_varve_pt, varve_sign, &k, &pixel_begin, &pixel_end))
        {
           num_line_extended[j] ++;  
           final_varve_index[i][j] = k; 
           copy_varve_point(&(reference_varve_pt), varve_image.varve_line[i].varve_pt[k]);
           copy_varve_point(&(wi.varve_line[i].varve_pt[j]), varve_image.varve_line[i].varve_pt[k]);
           wi.varve_line[i].varve_pt[j].begin_pt.pixel_num = pixel_begin;
           wi.varve_line[i].varve_pt[j].end_pt.pixel_num = pixel_end;
           wi.varve_line[i].varve_pt[j].sign = varve_sign; 
        } 
     } // end for i num_lines

     mark_varve_position_checked(position_j, j);

    } // end if checked == 0

    position_j ++;
    
  } // end all varve positions
 
  if (debug) 
     printf("exiting get final varve \n");
}


/* the purpose of this routine is to define a region for local contrast manipulation */
int get_next_varve_idx(int cur_varve_idx, int min_distance)
// April 5, 2010: should just pass begin pixel num instead of cur_varve_idx
{
  int i, j;
  int cur_peak_pixel, next_peak_pixel;

  i = cur_varve_idx;
  if (i == 0)
    cur_peak_pixel = begin_x;
  else
     cur_peak_pixel = wi.varve_line[0].varve_pt[cur_varve_idx].peak_pt.pixel_num;

  next_peak_pixel = cur_peak_pixel + min_distance;

  if (next_peak_pixel >= end_x)
	return wi.varve_line[0].num_varves-1; // March 12, 2010 not return end_x;

  while ((wi.varve_line[0].varve_pt[i].peak_pt.pixel_num < next_peak_pixel
          || num_line_extended[i] < wi.num_lines) 
          && i < wi.varve_line[0].num_varves - 1)
  {
	// April 5, 2010 printf("num_line_extended [%d] %d \n", i, num_line_extended[i]);
         i++;
  }
 
  return i;
}

/* September 7, 2007 
   modified October 29, 2007 */
void get_outliers(float cutoff_percent, int begin_varve_idx, int end_varve_idx,
                        int *outliers_min, int *outliers_max)
{
  int i, j;
  int total_pixel, cutoff_pixel;
  int histgram[256];
  int total_pixel_min, total_pixel_max;
  int min_gray, max_gray;

  if (begin_varve_idx >= end_varve_idx)
  {
    printf("begin_varve_idx %d has to be less than end_varve_idx  %d ! \n", begin_varve_idx, end_varve_idx);
    exit(-9999);
  }

  for(i=0; i<256; i++)
     histgram[i]=0;

  max_gray = 0;
  min_gray = 255;
  for (i=0; i<wi.num_lines; i++)
  {
    int jj, kk;

    //April, 6, 2010
    //jj = wi.varve_line[0].varve_pt[begin_varve_idx].peak_pt.pixel_num;
    jj = wi.varve_line[0].varve_pt[begin_varve_idx].begin_pt.pixel_num;

// November 26, 2007 temp bandage
    if (jj < begin_x)
      jj = begin_x;

    //April, 6, 2010
    //kk = wi.varve_line[0].varve_pt[end_varve_idx].peak_pt.pixel_num;
    kk = wi.varve_line[0].varve_pt[end_varve_idx].end_pt.pixel_num;

    for (j=jj; j<kk; j++)
    {
       int value_gray = gray_array[i+begin_y][j]; 

       if (value_gray > max_gray)
          max_gray = value_gray;
       else
       {
          if (value_gray < min_gray)
            min_gray = value_gray;
       }
       
       histgram[value_gray]++;
    }
  }

  // should be total pixels in this local area!
  total_pixel = 0;
  for(i=0; i<255; i++)
   total_pixel += histgram[i];

  cutoff_pixel = (int) (cutoff_percent * total_pixel/100.0);

  total_pixel_min= 0;
  i=0;
  while ( total_pixel_min < cutoff_pixel )
  {
    total_pixel_min += histgram[i];
    i++;
  }
 
  *outliers_min = i;

  total_pixel_max= 0;
  i=255;
  while ( total_pixel_max < cutoff_pixel )
  {
    total_pixel_max += histgram[i];
    i--;
  }
 
  *outliers_max = i;
 
  if (debug)
  {
    printf("outlierss: min = %d, max = %d \n", *outliers_min, *outliers_max);
  }
}

 /* SNR is defined as mean of pixel over the standard deviation */

 float SNR(int begin_varve_idx, int end_varve_idx)
 {
  int i, j;
  double sample_sq = 0.0, gray_total = 0.0;
  int  num_sample = 0;
  float regional_mean, regional_sd;

  if (begin_varve_idx >= end_varve_idx)
  {
    printf("SNR: begin_varve_idx has to be less than end_varve_idx ! \n");
    exit(-9999);
  }

  
  for (i=0; i<wi.num_lines; i++)
  {
    int jj, kk;

    jj = wi.varve_line[i].varve_pt[begin_varve_idx].begin_pt.pixel_num;

    if (jj < begin_x)
      jj = begin_x;

    kk = wi.varve_line[i].varve_pt[end_varve_idx].end_pt.pixel_num;

    for (j=jj; j<kk; j++)
    {
       gray_total += gray_array[i+begin_y][j]; 
       sample_sq += (gray_array[i+begin_y][j] * gray_array[i+begin_y][j]);
       num_sample++; 
    }

  }

  regional_mean = (gray_total) / (double) (num_sample);
  regional_sd  = sqrt(sample_sq/(num_sample) -  regional_mean * regional_mean);

  if (debug)
  {
    printf("SNR: return NSR= %6.2f \n", regional_mean/regional_sd);
  }

  return (regional_mean/regional_sd);
}

void do_local_contrast(float cutoff_percent)
{
  int i, j, k;
  int begin_varve_idx=0, end_varve_idx = 0;
  int outliers_min, outliers_max;
  float local_scale_factor;
  int sector_index = 0;
  
  // 2010-11-22: while(end_varve_idx < wi.varve_line[0].num_varves  - 1 )
  while(begin_varve_idx < wi.varve_line[0].num_varves  - 1 )
  {
    end_varve_idx = get_next_varve_idx(begin_varve_idx, min_section_distance);

    get_outliers(cutoff_percent, begin_varve_idx, end_varve_idx,
                        &outliers_min, &outliers_max);

    local_scale_factor = 255.0 /((float)(outliers_max - outliers_min));

    // get new gray values for this section
    for (i=0; i<wi.num_lines; i++)
    {
      j = wi.varve_line[0].varve_pt[begin_varve_idx].begin_pt.pixel_num;
      if (j < begin_x)
          j = begin_x; // initialize it, the first lamination may not completed connected. 04-29-08

      while (j < wi.varve_line[0].varve_pt[end_varve_idx].end_pt.pixel_num)
      {
        if (gray_array[i+begin_y][j] > outliers_max)
            gray_array[i+begin_y][j] = 255;
        else
        {
          if (gray_array[i+begin_y][j] < outliers_min)
              gray_array[i+begin_y][j] = 0;
          else
          {
             gray_array[i+begin_y][j] = (int)((gray_array[i+begin_y][j] - outliers_min) * local_scale_factor);
          } 
        }

	// added this on nov 24, 2009
	rarray[(i+begin_y)*width + j] = (unsigned char) gray_array[i+begin_y][j]; 
/*
	garray[(i+begin_y)*width + j] = (unsigned char) gray_array[i+begin_y][j]; 
	barray[(i+begin_y)*width + j] = (unsigned char) gray_array[i+begin_y][j]; 
	rarray[(i+begin_y)*width + j] = (unsigned char) (
			((int) rarray[(i+begin_y)*width + j] - outliers_min) * local_scale_factor);
	garray[(i+begin_y)*width + j] = (unsigned char) (
			((int) garray[(i+begin_y)*width + j] - outliers_min) * local_scale_factor);
	barray[(i+begin_y)*width + j] = (unsigned char) (
			((int) barray[(i+begin_y)*width + j] - outliers_min) * local_scale_factor);
*/

       j++;

      }
    }
  
/*
    improved_snr = (float *)realloc(improved_snr, sizeof(float)*sector_index);
    improved_snr[sector_index-1] = SNR(begin_varve_idx, end_varve_idx);
*/

    begin_varve_idx = end_varve_idx+1;
    while (wi.varve_line[0].varve_pt[begin_varve_idx].peak_pt.pixel_num == 0
		&& begin_varve_idx < wi.varve_line[0].num_varves)
	begin_varve_idx++;

    wi.varve_line[0].varve_pt[begin_varve_idx].begin_pt.pixel_num =
    	wi.varve_line[0].varve_pt[end_varve_idx].end_pt.pixel_num; 
       	// the varve_pt may not have value because the varve is not totally connected
  }
 
  if (debug)
      printf("exiting do local contrast \n");
}

/* September 22, 2007
   section depth data format: 
       section_name section_length begin_depth end_depth (in meters)
*/
int get_section_depth_info(char section_name[], float *begin_depth, float *end_depth)
{
  FILE *fp;
  int found = 0;
  char section_name_read[128];
  float section_length;

  if (debug)
  	printf(" welcome to get section info -- section name: %s \n", section_name);

  fp = fopen("section_depth_info.txt", "r");
  if (!fp)
  {
       cout << "cannot open the section_depth_info! " << "\n";
       exit(105);
  } 
  
  while (!found && !feof(fp))
  {
    fscanf(fp, "%s %f %f %f\n", section_name_read, &section_length, begin_depth, end_depth);
    if (strcmp(section_name_read, section_name) == 0)
      found = 1;
  }
  
  fclose(fp);
  
  if (debug)
  	printf(" exiting get section info \n");
  return found; 
}

int get_section_age_info(float a_begin_depth, float a_end_depth, float *begin_age, float *end_age)
{
  // file format: depth in meter, age in ky
  int found_b = 0, found_e = 0;
  float p_depth=0.0, p_age=0.0, u_depth, u_age;
  FILE *fp;
  
  if (debug)
     printf("welcome to get_section_age_info! %8.4f %8.4f\n", a_begin_depth, a_end_depth);;

  fp = fopen("section_age_info.txt", "r");
  if (!fp)
  {
       cout << "cannot open the section_age_info.txt file! " << "\n";
       exit(105);
  }

  while ((!found_b || !found_e) && !feof(fp))
  {
    fscanf(fp, "%f %f\n", &u_depth, &u_age);
    if (u_depth >= a_begin_depth && found_b == 0)
    {
      found_b = 1;
      if (u_depth == a_begin_depth)
          *begin_age = u_age;
      else
          *begin_age = p_age + (u_age - p_age)/(u_depth-p_depth)*(a_begin_depth - p_depth);
    } else
    {
      if (u_depth >= a_end_depth && found_e == 0)
      {
         found_e = 1;
         if (u_depth == a_end_depth)
            *end_age = u_age;
         else
           *end_age = p_age + (u_age - p_age)/(u_depth-p_depth)*(a_end_depth - p_depth);
      } else
      {
         p_depth = u_depth;
         p_age = u_age;
      }
    } 
  }
  
  fclose(fp);
  return (found_b && found_e); 
  
}

/* November 26, 2007
  a laminae is defined if:
    num_line_extended >= varve_threshold or
    previous laminae and next laminae has the same sign, we defined a laminae by default;

  a laminae is an array of varve points that connected:
    Need to write a routine to novelly fit a line cross all varve lines using varve_pt.peak_pt, and get their ave thickness -- so do begin and end points
      if all lines are connected, simply connect peak points;
      otherwise, take linear extrapolates
*/

int get_thickness_ave(int varve_idx)
{
  int i, j;
  int total_thickness = 0;

  j = 0;
  for (i=0; i<num_lines; i++)
  {
    if (wi.varve_line[i].varve_pt[varve_idx].width > 0)
    {
       j++;
       total_thickness += wi.varve_line[i].varve_pt[varve_idx].width ;
    }
  }
  
  if (total_thickness < num_lines)
     return 1;
  return (total_thickness/j);
}

void linear_regression(float xarray[], float yarray[], int num_pts, float *a, float *b)
{
 int i;
 float sx = 0.0, sy = 0.0, sxy = 0.0;
 float xtotal = 0.0, ytotal = 0.0;
 float xave, yave;

 for (i=0; i< num_pts; i++)
 {
   ytotal += yarray[i];
   xtotal += xarray[i];
 }

 xave = xtotal/num_pts; 
 yave = ytotal/num_pts; 


 for (i=0; i< num_pts; i++)
 {
    sx += (xarray[i] - xave)*(xarray[i] - xave);
    sy += (yarray[i] - yave)*(yarray[i] - yave);
    sxy += (xarray[i] - xave)*(yarray[i] - yave);
 }

 if (debug == 10)
     printf("xave = %10.2f yave= %10.2f sxy = %10.2f sx = %10.2f sy=%10.2f \n", xave, yave, sxy, sx, sy);

 *b = sxy / sx;
 *a = yave - (*b) * xave;

 if (debug == 10)
    printf("linear regression coefficients: a = %10.2f b = %10.2f \n", *a, *b);
}

/* 11-28-2009
   Dilation is a term used in morphorlogical image processing.
   dilation is to switch sign of pixels from 255 to 0 if the pixel does not connect
   to other signal pixels assuming that 0 is signal, 255 is background.
   This implemention is on y direction (horizontal)  only, i. e., we only check whether a pixel
   connects to other signal pixels in x direction.
*/
void dilation_erosion(int flag)
// flag: 1 for dilation, 0 for erosion
{
  int i=0, j=0, k=0;

  for (i=begin_x; i<end_x; i++)
  {
    for (j=begin_y; j<end_y; j++)
    {
      int value_j = (int) garray[j*width+i];
      int value_j2 = (int) garray[(j+1)*width+i];
      
	// one black and one white, make both of them black.
      if (value_j != value_j2){
         if (flag == 1) { // dilation
	    if (value_j == 255){ // j is white, j+1 is black 
      	      rarray[(j)*width+i] = (unsigned char) 0;
            } else { // this one is black, j+1 is white
      	      rarray[(j+1)*width+i] = (unsigned char) 0;
	    }
	} else { // erosion flag = 0
	    if (value_j == 0){
      	      rarray[(j)*width+i] = (unsigned char) 255;
            } else{
      	      rarray[(j+1)*width+i] = (unsigned char) 255;
	    }
        }
      }
    }
  } 
 
  memcpy(garray, rarray, sizeof(unsigned char)*width*height);
  memcpy(barray, rarray, sizeof(unsigned char)*width*height);
}

/* Dec 2, 2009
   erosion and dilation edges: switch the pixel values if the number of 
   consecutive pixels having the same values less that num_erosion * 2
*/

void dilation_erosion_edge(int num_pixel)
{

  int i=0, j=0, k=0;

  for (i=begin_x; i<end_x; i++)
  {
    // erosion lower edge

    j=begin_y;
    int num_equals = 1;
    int value_j = (int) garray[j*width+i];

    j++;
    while (j <= begin_y + 2*num_pixel)
    {
       int value_j2 = (int) garray[(j)*width+i];
       if (value_j == value_j2)
	  num_equals++;
       else
	 break;
       j++;
    }
    
    if (num_equals <= 2*num_pixel)
    {
       for(k=begin_y; k< begin_y + num_equals; k++)
       {
         if(value_j == 255)
	 {
      	  rarray[(k)*width+i] = (unsigned char) 0;
      	  garray[(k)*width+i] = (unsigned char) 0;
      	  barray[(k)*width+i] = (unsigned char) 0;
         } else {
      	  rarray[(k)*width+i] = (unsigned char) 255;
      	  garray[(k)*width+i] = (unsigned char) 255;
      	  barray[(k)*width+i] = (unsigned char) 255;
         }
       }
    }
      
    j=end_y;
    num_equals = 1;
    value_j = (int) garray[j*width+i];

    j--;

    while (j >= end_y - 2*num_pixel)
    {
       int value_j2 = (int) garray[(j)*width+i];
       if (value_j == value_j2)
	  num_equals++;
       else
	 break;
       j--;
    }
    
    if (num_equals <= 2*num_pixel)
    {
       for(k=end_y; k > end_y - num_equals; k--)
       {
         if(value_j == 255)
	 {
      	  rarray[(k)*width+i] = (unsigned char) 0;
      	  garray[(k)*width+i] = (unsigned char) 0;
      	  barray[(k)*width+i] = (unsigned char) 0;
         } else {
      	  rarray[(k)*width+i] = (unsigned char) 255;
      	  garray[(k)*width+i] = (unsigned char) 255;
      	  barray[(k)*width+i] = (unsigned char) 255;
         }
       }
    }
  }
}

/*
  Dec 10, 2009
  after the laminae is identified, we need to compute all attribute values.
  It is assumed that laminas are perfect from begin_y to end_y.
  varve_image.line[0] and line[num_lines-1] is identical in turn of geometry.
*/
void get_laminae_attributes()
{
  int i, j, k;
  int total_r, total_g, total_b;
  int laminae_total_r, laminae_total_g, laminae_total_b;
  int height;
  
  if (debug)
  	printf("welcome to get_laminae_attributes\n");
  
  height = end_y - begin_y + 1 ; 
  
  for (i=0; i<cleaned_varve_image.varve_line[0].num_varves; i++)
  {
     int bk, ek;

     bk = cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num; 
     ek = cleaned_varve_image.varve_line[0].varve_pt[i].end_pt.pixel_num; 

     if (bk > ek)
     {
	printf("  #### this should not happen: i= %d bk = %d ek= %d \n", i, bk, ek);
	exit(-345);
     }

     mean_thickness[i] = ek - bk + 1;  //begin and end points could be the same
     laminae_total_r = 0, laminae_total_g = 0, laminae_total_b = 0;

     for ( k = bk; k<=ek; k++)
     {
       total_r = 0, total_g = 0, total_b = 0;
       for (j=begin_y; j<=end_y; j++)
       {
          total_r += (int) original_rarray[j*width+k];
          total_g += (int) original_garray[j*width+k];
          total_b += (int) original_barray[j*width+k];
       }

       mean_value_r[k] = total_r/height;
       mean_value_g[k] = total_g/height;
       mean_value_b[k] = total_b/height;
       laminae_total_r += total_r;
       laminae_total_g += total_g;
       laminae_total_b += total_b;
     } 
     
     int total_pixel = mean_thickness[i] * num_lines;
     mean_laminae_r[i] = laminae_total_r/total_pixel;
     mean_laminae_g[i] = laminae_total_g/total_pixel;
     mean_laminae_b[i] = laminae_total_b/total_pixel;
  }
  
  if (debug)
  	printf("bye from get_laminae_attributes\n");
}

void print_laminae_attributes(char *outfile, char *attr_name, char *fmode)
{
  FILE *fpout;
  int i;
 
  fpout = fopen(outfile, fmode);
  if (fpout == NULL)
  {
      printf("cannot open file %s for writing! \n", outfile);
      exit(303);
  }

  if (strcmp(attr_name, "mean_thickness") == 0)
  {
	// Dec 12, 2009 print varve number and mean thickness
 	// however, it should be depth and mean thickness

     for (i=0; i<cleaned_varve_image.varve_line[0].num_varves; i++)
     {
	char lcolor;
	if (cleaned_varve_image.varve_line[0].varve_pt[i].sign == LIGHT)
	    lcolor = 'L';
        else
	  lcolor = 'D';
        
	// sequence number, D/L, begin_pixel, end_pixel, thickness
	fprintf(fpout, "%d\t%c\t%d\t%d\t%d\n", i, lcolor, 
	  cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num,
	  cleaned_varve_image.varve_line[0].varve_pt[i].end_pt.pixel_num,
		mean_thickness[i]);	
     }
  }

  if (strcmp(attr_name, "mean_laminae_rgb") == 0)
  {
	// Dec 12, 2009 print varve number and mean rgb
 	// however, it should be depth and mean rgb
     for (i=0; i<cleaned_varve_image.varve_line[0].num_varves; i++)
     {
	fprintf(fpout, "%d\t%d\t%d\t%d\t%d\t%d\t%d\n", i, 
	  cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num,
	  cleaned_varve_image.varve_line[0].varve_pt[i].end_pt.pixel_num,
		mean_laminae_r[i], mean_laminae_g[i], mean_laminae_b[i], (int)(0.2126*((float)(mean_laminae_r[i])) + 0.7152*((float)(mean_laminae_g[i])) + 0.0722 * ((float)(mean_laminae_b[i]))));
     }
  }

  if (strcmp(attr_name, "mean_rgb") == 0)
  {
	// Dec 12, 2009 print varve number and mean rgb
 	// however, it should be depth and mean rgb
     for (i=begin_x; i<end_x; i++)
     {
        if (mean_value_r[i] > 255 ||
		mean_value_g[i] > 255 ||
		mean_value_b[i] > 255 )
	{
		continue;	
	} else 
	fprintf(fpout, "%d\t%d\t%d\t%d\t%d\n", i, mean_value_r[i], mean_value_g[i], mean_value_b[i],(int)(0.2126*((float)(mean_value_r[i])) + 0.7152*((float)(mean_value_g[i])) + 0.0722 * ((float)(mean_value_b[i]))));
     }
  }

  fclose(fpout);
  if (debug)
    printf("exiting output attributes ... \n");
}


/**
January 17, 2010
pixel_num is the pixel position in vertical direction.
*/

float get_depth(int pixel_num)
{
   float depth_i = begin_depth + d_p_factor * (pixel_num - 1);
   return depth_i;
}

/** January 17, 2009
age model:
y = 1.8310 + 1.6813 * depth + 0.042 * depth**2 - 0.00071944 * depth **3.

// age model: -2.496 + 2.748 * d - 0.0116 *d *d
*/
// deprecated: March 1, 2011
float get_age(float depth)
{
  float age=0.0;
  depth /=1000.0;
//  age = 1.8310 + 1.6813 * depth + 0.042 * depth*depth - 0.00071944 * depth*depth*depth;
  age = -2.496 + 2.748 * depth - 0.0116 *depth*depth;
  age *= 1000.0;
  return age;

}

// March 1, 2011
float calculate_age_from_depth(float depth){
   // we use the cubic fitted curve
   double y0, a, b, c;
   double adepth;
   adepth = (double) depth / 1000.0;
   if (adepth < 1.9){
   	y0=0.0107;
	a= 0.9558;
	b = 0.3707;
	c = -0.0389;
   } else {
    if (adepth < 5.40){
     y0 = -17.3086;
     a = 15.2625; 
     b =-2.9041; 
     c = 0.2222; 
    } else {
     if (adepth < 10.0){
	y0 = -4.6886;
	a = 7.4712;
	b = -0.9291;
	c = 0.0436;
     } else {	
       if (adepth < 32.0){
         y0 = 6.3810; 
         a = 0.9423; 
         b = 0.0715; 
         c = -0.001; 
       } else {
         y0 = 31.0694;
         a = 1.4581; 
         b = 0.000001651; 
         c = -0.00000001536; 
       }
    }
   }
  }
  
   float age = y0 + a * adepth + b * adepth * adepth + c* adepth*adepth*adepth;
   return age*1000.0;
}

// 2012-05-27
void print_all_attributes(char *outfile, int noDT, char *mode) 
{

  FILE *fp;
  int i;
 
  fp = fopen(outfile, mode);
  if (fp == NULL)
  {
      printf("cannot open file %s for writing! \n", outfile);
      exit(303);
  }

  for (i=0; i<cleaned_varve_image.varve_line[0].num_varves; i++)
  {
	char lcolor[10];
	if (cleaned_varve_image.varve_line[0].varve_pt[i].sign == LIGHT)
	    sprintf(lcolor, "%s","Light");
        else
	    sprintf(lcolor, "%s","Dark");
        
	// lamina sequence, type, begin pixel, end pixel, thickness, begin depth, end depth, start age, end age, ave rgb, grayscale
	fprintf(fp, "%d\t%s\t%d\t%d\t%d\t", i, lcolor, 
	  cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num,
	  cleaned_varve_image.varve_line[0].varve_pt[i].end_pt.pixel_num,
		mean_thickness[i]);	

        if (noDT == 0) { // have DT
	  float depth_i = begin_depth + d_p_factor * (cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num - 1);
	  float depth_e = begin_depth + d_p_factor * (cleaned_varve_image.varve_line[0].varve_pt[i].end_pt.pixel_num - 1);
	  float age_i = get_age(get_depth(cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num));
	  float age_e = get_age(get_depth(cleaned_varve_image.varve_line[0].varve_pt[i].end_pt.pixel_num));

	  fprintf(fp, "%9.2f\t%9.2f\t%9.2f%9.2f\t%9.2f\t%9.2f\t", depth_i, depth_e, depth_e-depth_i, 
			age_i, age_e, age_e-age_i);
	}
	fprintf(fp, "%d\t%d\t%d\t%d\n", mean_laminae_r[i], mean_laminae_g[i], mean_laminae_b[i], 
		(int)(0.2126*((float)(mean_laminae_r[i])) + 0.7152*((float)(mean_laminae_g[i])) + 0.0722 * ((float)(mean_laminae_b[i]))));
  }

  fclose(fp);
  if (debug)
     printf("exiting print all attributes\n");
}

/* Dec 20, 2009: print depth and attributies.
*/
void print_laminae_depth_attributes(char *outfile, char *attr_name, char *fmode)
{
  FILE *fp;
  int i;
 
  fp = fopen(outfile, fmode);
  if (fp == NULL)
  {
      printf("cannot open file %s for writing! \n", outfile);
      exit(303);
  }

  if (strcmp(attr_name, "mean_thickness") == 0)
  {
	// Dec 12, 2009 print varve number and mean thickness
 	// however, it should be depth and mean thickness
     for (i=0; i<cleaned_varve_image.varve_line[0].num_varves; i++)
     {
	float depth_i = begin_depth + d_p_factor * (cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num - 1);
	fprintf(fp, "%9.2f\t%d\t\%d\n", depth_i, cleaned_varve_image.varve_line[0].varve_pt[i].sign, mean_thickness[i]);	
     }
  }

  if (strcmp(attr_name, "mean_laminae_rgb") == 0)
  {
	// Dec 20, 2009 print depth  and mean rgb
     for (i=0; i<cleaned_varve_image.varve_line[0].num_varves; i++)
     {
	float depth_i = begin_depth + d_p_factor * (cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num - 1);
	fprintf(fp, "%9.2f\t%d\t%d\t%d\t%d\t%d\n", depth_i, cleaned_varve_image.varve_line[0].varve_pt[i].sign, mean_laminae_r[i], mean_laminae_g[i], mean_laminae_b[i], (int)(0.2126*((float)(mean_laminae_r[i])) + 0.7152*((float)(mean_laminae_g[i])) + 0.0722 * ((float)(mean_laminae_b[i]))));
     }
  }

  if (strcmp(attr_name, "mean_rgb") == 0)
  {
	// Dec 12, 2009 print varve number and mean rgb
 	// however, it should be depth and mean rgb
	// February 12, 2011: do not need to print out the lamina sign
     for (i=begin_x; i<end_x; i++)
     {
	float depth_i = begin_depth + d_p_factor * (i - 1);
        if (mean_value_r[i] > 255 ||
		mean_value_g[i] > 255 ||
		mean_value_b[i] > 255 )
	{
           continue;
	} else 
	  fprintf(fp, "%9.2f\t%d\t%d\t%d\t%d\n", depth_i, mean_value_r[i], mean_value_g[i], mean_value_b[i],(int)(0.2126*((float)(mean_value_r[i])) + 0.7152*((float)(mean_value_g[i])) + 0.0722 * ((float)(mean_value_b[i]))));
     }
  }
  
  fclose(fp);
}

/* Dec 20, 2009: print age and attributies.
*/
void print_laminae_age_attributes(char *outfile, char *attr_name, char *fmode)
{
  FILE *fp;
  int i;
 
  fp = fopen(outfile, fmode);
  if (fp == NULL)
  {
      printf("cannot open file %s for writing! \n", outfile);
      exit(303);
  }

  if (strcmp(attr_name, "mean_thickness") == 0)
  {
	// Dec 12, 2009 print varve number and mean thickness
 	// however, it should be depth and mean thickness
     for (i=0; i<cleaned_varve_image.varve_line[0].num_varves; i++)
     {
	float age_i = get_age(get_depth(cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num));
	fprintf(fp, "%9.2f\t%d\t%d\n", age_i, cleaned_varve_image.varve_line[0].varve_pt[i].sign, mean_thickness[i]);	
     }
  }

  if (strcmp(attr_name, "mean_laminae_rgb") == 0)
  {
	// Dec 20, 2009 print depth  and mean rgb
     for (i=0; i<cleaned_varve_image.varve_line[0].num_varves; i++)
     {
	float age_i = calculate_age_from_depth( get_depth(cleaned_varve_image.varve_line[0].varve_pt[i].begin_pt.pixel_num));
	fprintf(fp, "%9.2f\t%d\t%d\t%d\t%d\t%d\n", age_i, cleaned_varve_image.varve_line[0].varve_pt[i].sign, mean_laminae_r[i], mean_laminae_g[i], mean_laminae_b[i], (int)(0.2126*((float)(mean_laminae_r[i])) + 0.7152*((float)(mean_laminae_g[i])) + 0.0722 * ((float)(mean_laminae_b[i]))));
     }
  }

  if (strcmp(attr_name, "mean_rgb") == 0)
  {
	// Dec 12, 2009 print varve number and mean rgb
 	// however, it should be depth and mean rgb
     for (i=begin_x; i<end_x; i++)
     {
	float age_i = calculate_age_from_depth(get_depth(i));
	fprintf(fp, "%9.2f\t%d\t%d\t%d\t%d\n", age_i, mean_value_r[i], mean_value_g[i], mean_value_b[i],(int)(0.2126*((float)(mean_value_r[i])) + 0.7152*((float)(mean_value_g[i])) + 0.0722 * ((float)(mean_value_b[i]))));
     }
  }
  
  fclose(fp);
}

/*** January 25, 2011: write out grayscale value to a file ***************/
void write_grayscale_values(char *outfile){
 FILE *fp;
 int i, j;

 fp = fopen(outfile, "w");
 for (i=0; i<end_x; i++)
 {
   for (j=0; j<height; j++)
 	fprintf(fp, "%d ", gray_array[j][i]);
   fprintf(fp, "\n");
 }
 fclose(fp);
}

/** April 5, 2011 print out peaks for figure illustrating 1-D algorithm */
void print_data_4_1d_algorithm()
{
  int i, j, k;
  FILE *fp, *fp2, *fp3;
  int not_done = 1;

  fp = fopen("original_rgb.txt", "w");
  fp2 = fopen("peaks.txt", "w");
  fp3 = fopen("borders.txt", "w");

  k=0;
  for (j=begin_x; j<end_x; j++){
     fprintf(fp, "%d\t%d\t\%d\t%d\t%d\n", j, 
 			rarray[(begin_y+k)*width+j],
			garray[(begin_y+k)*width+j],
			barray[(begin_y+k)*width+j],
			gray_array[begin_y+k][j]);
  }

  for (i=0; i<varve_image.varve_line[k].num_varves; i++){ 
	int pixel = varve_image.varve_line[k].varve_pt[i].peak_pt.pixel_num;
        int r = varve_image.varve_line[k].varve_pt[i].peak_pt.value_r; 
        int g = varve_image.varve_line[k].varve_pt[i].peak_pt.value_g; 
        int b = varve_image.varve_line[k].varve_pt[i].peak_pt.value_b; 
        int gray_v = varve_image.varve_line[k].varve_pt[i].peak_pt.value_gray; 
        //int gray_v = (int)(original_gray[(begin_y+k)*width+pixel]);
	fprintf(fp2, "%d\t%d\t%d\t%d\t%d\n", pixel, r, g, b, gray_v); 
	int start_p= varve_image.varve_line[k].varve_pt[i].begin_pt.pixel_num;
	int end_p = varve_image.varve_line[k].varve_pt[i].end_pt.pixel_num;
	fprintf(fp3, "%d\t%d\n%d\t%d\n", start_p, gray_v, end_p, gray_v);  
  }
  fclose(fp);
  fclose(fp2);
  fclose(fp3);
}

// 2012-05-27 this is to insert user identified laminae to final image. user data will be just a column of boundaries

void insert_user_laminae(char *datafileName, int usrSeq)
/* read user ided lamina data from datafileName, and write them back to [r,g,b]arrays so that we can write rgb array back to bmp file 
   target_image: 0 is working (rarray), 1 is original (original_rarray).
*/
{
  int i, j,  k;
  int r1; 
  int s1;
  FILE *fp;
  int interval = cleaned_varve_image.num_lines/4; 

  printf("welcome to insert_user_laminae: datafile=%s user seq = %d \n", datafileName, usrSeq); 
  fp = fopen(datafileName, "r");
  if (fp == NULL)
  {
      printf("cannot open file %s for reading! \n", datafileName);
      exit(303);
  } else {
      printf("open file %s for reading! \n", datafileName);
  }

  fscanf(fp, "%d\n", &r1);
  s1 = r1;

  int ii=0;
  while (!feof(fp)){
	ii++;
	fscanf(fp, "%d\n", &r1);

  	for (i=interval*usrSeq; i< interval*(usrSeq+1); i++) 
  	{
          for (k= s1; k < r1; k++ )
          {
		if ( ii%2 == 0)
                {
                 rarray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                 garray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                 barray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                 gray_array[i+begin_y][k] = 255;
                } else
                {
                   		rarray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                   		garray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                   		barray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                   		gray_array[i+begin_y][k] = 0;
		   switch (usrSeq){
			case 1:
                   		rarray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                   		garray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                   		barray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                   		gray_array[i+begin_y][k] = 0;
				break;
			case 2: 
                   		rarray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                   		garray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                   		barray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                   		gray_array[i+begin_y][k] = 0;
				break;
			case 3:
                   		rarray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                   		garray[k+(i+begin_y)*width] = (unsigned char) ( 0);
                   		barray[k+(i+begin_y)*width] = (unsigned char) ( 255);
                   		gray_array[i+begin_y][k] = 0;
				break;
		   }
                }
          } // each varves
       } // end each varve line

	s1=r1;
  } // end while 

  fclose(fp);
}

//****************************************************************************
int main ( long argc, char *argv[] )

//****************************************************************************
// Stoney Q. Gan December 29, 2007 
// argv[1] is a string that will be read by:
// argv[2] is either 0 or 1 indicating whether all output data will be collated. default is 0
// argv[3] is the debug flag, default is 0 
//
{
  
  bool error;
  char str_line[128], file_name[100];
  int i, j, k;
  int char_count;
  int file_len;
  char outfile_name[100];
  int wmv_x_width, wmv_x_level, wmv_y_width, wmv_y_level;
  int all_in_one = 0;
  FILE *fp, *fps;
  int  nomoredepthdata = 0;
  int outbmp = 0;
  int area_width, area_height;
 
  if (argc < 2)
  {
    cout << "usage: laminasID \"input data string\" [collate output in one or not] [output bmp file] [debug_level]" << "\n";
    exit(105);
  }

  if (argc > 2)
     all_in_one = atoi(argv[2]);

  if (argc > 3) {
     outbmp = atoi(argv[3]);
  }

  if (argc == 5 )
    debug = atoi(argv[4]);

  fps = fopen("section_summary.txt", "a");
  if (!fps)
  {
       cout << "cannot open the section_summary.txt file! " << "\n";
       exit(-999);
  }

  sscanf(argv[1], "%s %d %d %d %d %d %d %d %d %d %d\n", file_name, &begin_x, &begin_y, &area_width, &area_height, &wmv_x_width, &wmv_x_level, &wmv_y_width, &wmv_y_level, &threshold_peak, &min_section_distance);
  
  if (area_width < 2 || area_height < 1) {
  	printf("Processing area has to be specified, please refer to readme for details \n");
	exit(-103);
  }
 
  if (area_height%4 > 0){
	printf("I'd suggest to set the height of processing area to a product of 4 \n");
  }
  
  if (min_section_distance < 50) {
	printf("minimal section size for sectional contrast enhancement should be greater to 100 pixel for effective results.\n");
	printf("I will set it to 100 for you this time \n");
  }

  end_x = begin_x + area_width + (wmv_x_width/2) * wmv_x_level * 2 - 1;
  end_y = begin_y + area_height + (wmv_y_width/2) * wmv_y_level * 2 - 1;

  read_bmp_data(file_name);

 // sanity checkings
  if (end_x < begin_x)
  {
     cout << "I do not like the begin_x >= end_x" << endl;
     exit(106);
  }

  if ( end_y < begin_y )
  {
    cout << "I do not like the begin_y >= end_y" << endl;
    exit(107);
  }

  if (begin_x < 0)
     begin_x = 1;

  if (begin_y < 0)
    begin_y = 1;

  if (end_x >= width)
      end_x = width-1;

  if (end_y >=  height)
      end_y = height-1;

  cout << "\n**********************************\n";
  printf("read bmp info: image width= %d, image height = %d\n", width, height);
  printf("processing ... file_name is %s \nbegin_x = %d end_x = %d begin_y = %d end_y = %d \nMPMV in x direction: kernel width  = %d  number of passes = %d kernel width in y  = %d number of passes = %d \n",
          file_name, begin_x, end_x, begin_y, end_y, wmv_x_width, wmv_x_level, wmv_y_width, wmv_y_level);

  file_len = strlen(file_name) - 4;
  strncpy(infile_stub, file_name, file_len); 
  infile_stub[file_len] = '\0';


  // January 25, 2011: temporary for the next 3 lines
  /*
  strcpy(outfile_name, infile_stub);
  strcat(outfile_name, "_orig_gray.txt");
  write_grayscale_values(outfile_name);
  */

  if (wmv_x_width > 1 && wmv_x_level > 0)
  	weighted_x_mv(wmv_x_width, wmv_x_level, &begin_x, &end_x);
  if (wmv_y_width > 1 && wmv_y_level > 0)
  	weighted_y_mv(wmv_y_width, wmv_y_level, &begin_y, &end_y);

  if (outbmp == 1)
  {
    printf("write out the image after multiple pass moving average processing.\n");
    strcpy(outfile_name, infile_stub);
    strcat(outfile_name, "_wmv.bmp");
    bmp_24_write(outfile_name, width, height, rarray, garray, barray);
  }

  //printf("write out an grayscale image after multiple pass moving average. \n");
  //strcpy(outfile_name, infile_stub);
  //strcat(outfile_name, "_wmv_gray.txt");
  //write_grayscale_values(outfile_name);

  num_points = end_x - begin_x + 1;
  num_lines = end_y - begin_y + 1; 

  f_image.num_lines = num_lines;
  wi.num_lines = num_lines;
  varve_image.num_lines = num_lines;
  cleaned_varve_image.num_lines = num_lines;

  reinit_global_variables(num_lines, num_points);
  
  printf("to get varve image\n");
  get_varve_image();
 
  // April 5, 2011
  // print_data_4_1d_algorithm();

  printf("to get laminae image \n");
  get_laminae_image();
 
  printf("to do local contrast: \n");
  do_local_contrast(3.0);

  // write results to both original and working copies of images
  // 0 is for working, 1 for original
  //insert_varve_gray(varve_image, 0);
  //insert_varve_gray(varve_image, 1);

  if (outbmp == 1)
  {
    printf("write out an image after local contrast processing. \n");
    strcpy(outfile_name, infile_stub);
    strcat(outfile_name, "_local_contrast.bmp");
    bmp_24_write(outfile_name, width, height, rarray, rarray, rarray);
  }

  // do it one more time after do local contrast
 printf("to get varvae image after local contrast enhancement\n");
  get_varve_image();

  printf("to write laminae to images: \n");
  insert_varve_image(varve_image, 0);
 
  if (outbmp == 1)
  {
      // output the bmp file
    printf("write out an image with initial varves. \n");
    strcpy(outfile_name, infile_stub);
    strcat(outfile_name, "_f_init_varves.bmp");
    bmp_24_write(outfile_name, width, height, rarray, garray, barray);
  }

   // Dec 1, 2009
   // multiple erosion and dilation (eliminate small black island), 
   // and multiple dilation and erosion to eliminate small white island.
   // reasons we have to do this in alternate way is to avoid one erodes 
   // another. e. g., 111-111, if we move 1 first, then it becomes ----111
   // such that the - is never removed.

   int num_erosion = num_lines/4;
   printf("to perform blob analyses \n");
   for (i=0; i<num_erosion; i++){
        // dilation first, it will remove small white island
     for (j=0; j<=i; j++){
       dilation_erosion(1);
     }

     for (j=0; j<=i; j++){
       dilation_erosion(0);
     }

	// erosion first, this will eliminate small black island
     for (j=0; j<=i; j++){
       dilation_erosion(0);
     }

     for (j=0; j<=i; j++){
       dilation_erosion(1);
     }

	// Dec 30, 2009
	// it is more reasonable to process edge in each iteration here.
     dilation_erosion_edge(i+1);
      // output the bmp file
      //sprintf(outfile_name,"%s_clean%d.bmp", infile_stub,i);
     //bmp_24_write(outfile_name, width, height, rarray, garray, barray);
  }
 
  printf("to get cleaned varve image\n");
  get_cleaned_varve_image();

  insert_varve_image(cleaned_varve_image, 0);

      // output the bmp file
   printf("write out the final image ... \n");
   strcpy(outfile_name, infile_stub);
   strcat(outfile_name, "_2D-laminae.bmp");
   bmp_24_write(outfile_name, width, height, rarray, garray, barray);

   /*
  	// 2012-05-27
  	for (int uu=1; uu<=3; uu++) {
		char usrDataFile[20];
		sprintf(usrDataFile, "userLaminaeData%d.txt", uu);
  		insert_user_laminae(usrDataFile, uu);
  	}
  	strcpy(outfile_name, infile_stub);
  	strcat(outfile_name, "_withUserData.bmp");
  	bmp_24_write(outfile_name, width, height, rarray, garray, barray);

    */

   printf("calculate attributes .... \n");
   get_laminae_attributes();

   printf(" done calculate attributes .... \n");

   if (all_in_one == 0)
   {
     printf("output individual image attributes .... \n");
     sprintf(outfile_name,"%s_laminae_mean_thickness.txt", infile_stub);
     print_laminae_attributes(outfile_name, "mean_thickness", "w");

     sprintf(outfile_name,"%s_laminae_mean_rgb.txt", infile_stub);
     print_laminae_attributes(outfile_name, "mean_laminae_rgb", "w");


     sprintf(outfile_name,"%s_pixel_mean_rgb.txt", infile_stub);
     print_laminae_attributes(outfile_name, "mean_rgb", "w");
   }

  if (get_section_depth_info(infile_stub, &begin_depth, &end_depth))
  {
     float section_b_d, section_e_d;
     begin_depth *= 1000.0;
     end_depth *= 1000.0;
	
	// December 19, 2009: end_x-begin_x is changed to width
     d_p_factor = (end_depth - begin_depth)/width;
     if (debug) {
        printf("filename: %s ", infile_stub);
        printf("begin_depth %8.4f end_depth %8.4f d_p_factor %8.4f \n", begin_depth, end_depth, d_p_factor);
     }

     // December 19, 2009: need to convert pixel value to depth value and print attribute values out.
     if (all_in_one){
     	sprintf(outfile_name,"all_depth_laminae_mean_thickness.txt");
     	print_laminae_depth_attributes(outfile_name, "mean_thickness", "a");

     	sprintf(outfile_name,"all_depth_laminae_mean_rgb.txt");
     	print_laminae_depth_attributes(outfile_name, "mean_laminae_rgb", "a");

     	sprintf(outfile_name,"all_depth_mean_rgb.txt");
     	print_laminae_depth_attributes(outfile_name, "mean_rgb", "a");

     } else {
     	sprintf(outfile_name,"%s_depth_laminae_mean_thickness.txt", infile_stub);
     	print_laminae_depth_attributes(outfile_name, "mean_thickness", "w");

     	sprintf(outfile_name,"%s_depth_laminae_mean_rgb.txt", infile_stub);
     	print_laminae_depth_attributes(outfile_name, "mean_laminae_rgb", "w");

     	sprintf(outfile_name,"%s_depth_mean_rgb.txt", infile_stub);
     	print_laminae_depth_attributes(outfile_name, "mean_rgb", "w");
      }

           // turn them back to meters
      section_b_d = begin_depth/1000.0;
      section_e_d = end_depth/1000.0;

	// March 1, 2011: use the curve fitted age models
/*
      if (get_section_age_info(section_b_d, section_e_d, &begin_age, &end_age))
      {
          if (debug)
  	      printf("begin_age: %9.4f end_age %9.4f\n", begin_age, end_age);

            // output data in mm and yrs instead of meter and ky
          begin_age *= 1000.0;
          end_age *= 1000.0;

     	  a_p_factor = (end_age - begin_age)/width;
*/	  
        if (section_e_d < 50) // we have the top 50 m age model
  	{
          section_b_d *= 1000.0;
          section_e_d *= 1000.0;
	  begin_age = calculate_age_from_depth(section_b_d);
	  end_age = calculate_age_from_depth(section_e_d);
  	  printf("begin_age: %9.4f end_age %9.4f\n", begin_age, end_age);

		// Janunary 08, 2010 print out summary info
  	  fprintf(fps, "%s %d %d %d %d %d %d %d %8.4f %8.4f %8.4f %8.4f %8.4f %8.4f %d %d %8.4f %8.4f %8.4f %8.4f\n",
                 infile_stub, begin_x, end_x, end_x-begin_x, begin_y, end_y, width, height, 
                 section_b_d, section_e_d, section_e_d-section_b_d, 
                 begin_age, end_age, end_age-begin_age, 
                 num_points, cleaned_varve_image.varve_line[0].num_varves, 
                 (float)(cleaned_varve_image.varve_line[0].num_varves/(section_e_d-section_b_d)), 
                 (float)(num_points/(section_e_d-section_b_d)), 
                 (float)(cleaned_varve_image.varve_line[0].num_varves/(end_age-begin_age)), 
                 (float)(num_points/(end_age-begin_age))); 


	  if (all_in_one)
	  {
     	    sprintf(outfile_name,"all_age_laminae_mean_thickness.txt");
     	    print_laminae_age_attributes(outfile_name, "mean_thickness", "a");

     	    sprintf(outfile_name,"all_age_laminae_mean_rgb.txt");
     	    print_laminae_age_attributes(outfile_name, "mean_laminae_rgb", "a");

     	    sprintf(outfile_name,"all_age_mean_rgb.txt");
     	    print_laminae_age_attributes(outfile_name, "mean_rgb", "a");

	  } else {
     	    sprintf(outfile_name,"%s_age_laminae_mean_thickness.txt", infile_stub);
     	    print_laminae_age_attributes(outfile_name, "mean_thickness", "w");

     	    sprintf(outfile_name,"%s_age_laminae_mean_rgb.txt", infile_stub);
     	    print_laminae_age_attributes(outfile_name, "mean_laminae_rgb", "w");

     	    sprintf(outfile_name,"%s_age_mean_rgb.txt", infile_stub);
     	    print_laminae_age_attributes(outfile_name, "mean_rgb", "w");
	  }
      }
  } else
  {
     printf("I cannot find depth data for this image file! \n assume no more depth data ... \n");
     nomoredepthdata = 1;
  }

  		// 2012-05-27 
   if (all_in_one) {
        sprintf(outfile_name,"%s_all_attributes.txt", infile_stub);
        print_all_attributes(outfile_name, nomoredepthdata, "a");
    } else {
  	sprintf(outfile_name,"%s_all_attributes.txt", infile_stub);
  	print_all_attributes(outfile_name, nomoredepthdata, "w");
     }
    
  if (fps != NULL ){
	printf("closing section summary file ... \n");
  	fclose(fps);
	printf("done with this image ... \n");
  } else {
  	printf("the section summary file pointer is null ... we are done with this image. \n");
  }
  
  printf("free memories \n");
  free(original_rarray);
  free(original_garray);
  free(original_barray);
  free(original_gray);
  free(gray_array);


  cout << "**********************************\n";
  return (1);
}
