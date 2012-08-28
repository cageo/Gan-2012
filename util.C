#include "gray_2d.H"
using namespace std;

extern int debug;
extern int begin_x;
extern int cumulated_pixel_num;

void init_rgb_point(rgb_point *t_pt)
{
  t_pt->pixel_num=0;
  t_pt->value_r = 0;
  t_pt->value_g = 0;
  t_pt->value_b = 0;
}

void init_varve_point(t_varve *t_var)
{
  init_rgb_point(&(t_var->begin_pt));
  init_rgb_point(&(t_var->end_pt));
  init_rgb_point(&(t_var->peak_pt));
  t_var->sign = DARK;
  t_var->median = 0.0;
  t_var->mean = 0.0;
  t_var->sd = 0.0;
  t_var->kurtosis = 0.0;
  t_var->confidence = 0.0;
  t_var->checked = 0;
  t_var->varve_index=-1;
  t_var->width = 0;
}

void print_rgb_point(rgb_point pt)
{
  printf("	pixel number: %d r: %d g: %d b: %d gray %d\n", pt.pixel_num, pt.value_r, pt.value_g, pt.value_b, pt.value_gray);
}

void copy_rgb_point(rgb_point *t_pt, rgb_point s_pt)
{
  t_pt->pixel_num = s_pt.pixel_num;
  t_pt->value_r = s_pt.value_r;
  t_pt->value_g = s_pt.value_g;
  t_pt->value_b = s_pt.value_b;
  t_pt->value_gray = s_pt.value_gray;
}

void print_varve_point(t_varve t_var)
{
  printf("\n 	begin_pt: ");
  print_rgb_point(t_var.begin_pt);
  printf("\n 	end_pt: ");
  print_rgb_point(t_var.end_pt);
  printf("\n 	peak pt: ");
  print_rgb_point(t_var.peak_pt);
  printf("\n    sign = %d median= %5.2f mean = %5.2f standard deviation = %6.2f kurtosis = %5.2f confidence= %5.2f width = %d \n",
                  t_var.sign, t_var.median, t_var.mean, t_var.sd, t_var.kurtosis, t_var.confidence, t_var.width );
}

void copy_varve_point(t_varve *t_var, t_varve s_var)
{
  copy_rgb_point(&(t_var->begin_pt), s_var.begin_pt);
  copy_rgb_point(&(t_var->end_pt), s_var.end_pt);
  copy_rgb_point(&(t_var->peak_pt), s_var.peak_pt);
  t_var->sign = s_var.sign;
  t_var->median = s_var.median;
  t_var->mean = s_var.mean;
  t_var->sd = s_var.sd;
  t_var->kurtosis = s_var.kurtosis;
  t_var->confidence = s_var.confidence;
  t_var->width = s_var.width;
}

void print_varve_line(t_varve_line t_line)
{
  int i;

  printf("\n ###### print varve line: the number of varves: %d ", t_line.num_varves);
  for(i=0; i<t_line.num_varves; i++)
  {
   printf("\n  	varve_pt index %d\n", i);
   print_varve_point(t_line.varve_pt[i]);
  }
}


void copy_varve_line(t_varve_line *t_line, t_varve_line s_line)
{
  int i;
 
  for(i=0; i<s_line.num_varves; i++)
  {
   copy_varve_point(&(t_line->varve_pt[i]), s_line.varve_pt[i]);
  }
}


void print_rgb_point_array(rgb_point *rpt_array, int num_pt)
{
  int i;
  printf("the number point: %d\n", num_pt);
  for (i=0; i<num_pt; i++)
    print_rgb_point(rpt_array[i]);
}

void copy_varve_position(t_varve_position *t_p, t_varve_position s_p)
{
  t_p->pixel_num = s_p.pixel_num;
  t_p->section_num = s_p.section_num;
  t_p->varve_num = s_p.varve_num;
  t_p->checked = s_p.checked; 
}

void print_varve_position(t_varve_position s_p)
{
  printf("   varve position - varve_line: %d varve_num %d pixel_num %d checked %d \n", s_p.section_num, s_p.varve_num, s_p.pixel_num, s_p.checked);
}


/* September 22, 2007
   modify output_varve_attributes to output depth and attributes.
*/
void output_varve_depth_attributes(float begin_depth, double d_p_factor, t_varve_line a_varve_line, char *attr_name, 
                              char outfile_name[], char *fmode)
{
  int i;
  FILE *fp;

  fp = fopen(outfile_name, fmode);
  if (fp == NULL)
  {
      printf("cannot open file %s for writing! \n", outfile_name);
      exit(303);
  }

  if (strcmp(fmode, "w") == 0)
  {
       // outfile_name contains the sample ID + something not needed here. To lazzy to chop them off
    fprintf(fp, "%s %d\n", outfile_name, a_varve_line.num_varves);
  }

  if (strcmp(attr_name, "thickness") == 0)
  {
     for (i=0; i<a_varve_line.num_varves-1; i++)
    {
       float this_depth = begin_depth + (a_varve_line.varve_pt[i].begin_pt.pixel_num - begin_x) * d_p_factor;
       fprintf(fp, "%10.2f %d\n", this_depth, a_varve_line.varve_pt[i].width);
    }

    fclose(fp);
    return;
  }
  
  if (strcmp(attr_name, "kurtosis") == 0)
  {
     for (i=0; i<a_varve_line.num_varves-1; i++)
    {
       float this_depth = begin_depth + (a_varve_line.varve_pt[i].begin_pt.pixel_num - begin_x) * d_p_factor;
       fprintf(fp, "%10.2f %8.4f\n", this_depth, a_varve_line.varve_pt[i].kurtosis);
    }

    fclose(fp);
    return;
  }
  
  if (strcmp(attr_name, "mean") == 0)
  {
     for (i=0; i<a_varve_line.num_varves-1; i++)
    {
       float this_depth = begin_depth + (a_varve_line.varve_pt[i].begin_pt.pixel_num - begin_x) * d_p_factor;
       fprintf(fp, "%10.2f %8.4f\n", this_depth, a_varve_line.varve_pt[i].mean);
    }

    fclose(fp);
    return;
  }
  
  if (strcmp(attr_name, "sd") == 0)
  {
     for (i=0; i<a_varve_line.num_varves-1; i++)
    {
       float this_depth = begin_depth + (a_varve_line.varve_pt[i].begin_pt.pixel_num - begin_x) * d_p_factor;
       fprintf(fp, "%10.2f %8.4f\n", this_depth, a_varve_line.varve_pt[i].sd);
    }

    fclose(fp);
    return;
  }
  
  if (strcmp(attr_name, "peak_rgb") == 0)
  {
     for(i=0; i<a_varve_line.num_varves; i++)
     {
       float this_depth = begin_depth + (a_varve_line.varve_pt[i].peak_pt.pixel_num - begin_x) * d_p_factor;
       fprintf(fp, "%10.2f %d %d %d %d \n", this_depth, 
                                  a_varve_line.varve_pt[i].peak_pt.value_r, 
			          a_varve_line.varve_pt[i].peak_pt.value_g, 
			          a_varve_line.varve_pt[i].peak_pt.value_b, 
                                  a_varve_line.varve_pt[i].peak_pt.value_gray); 
     }

     fclose(fp);
     return;
  }

}

/* August 23, 2007
   A totally re-written procedure.
   total number of varves will be the first line in the output file
   other line will have a format: begin_pixel width
*/
void output_varve_attributes( t_varve_line a_varve_line, char *attr_name, 
                              char outfile_name[], char *fmode)
{
  int i;
  FILE *fp;

  fp = fopen(outfile_name, fmode);
  if (fp == NULL)
  {
      printf("cannot open file %s for writing! \n", outfile_name);
      exit(303);
  }

  if (strcmp(fmode, "w") == 0)
  {
       // outfile_name contains the sample ID + something not needed here. To lazzy to chop them off
    fprintf(fp, "%s %d\n", outfile_name, a_varve_line.num_varves);
    cumulated_pixel_num = 0;
  }

  if (strcmp(attr_name, "thickness") == 0)
  {
     for (i=0; i<a_varve_line.num_varves-1; i++)
    {
       fprintf(fp, "%d %d\n", cumulated_pixel_num + a_varve_line.varve_pt[i].begin_pt.pixel_num, a_varve_line.varve_pt[i].width);
    }

    fclose(fp);
    return;
  }
  
  if (strcmp(attr_name, "kurtosis") == 0)
  {
     for (i=0; i<a_varve_line.num_varves-1; i++)
    {
       fprintf(fp, "%d %8.4f\n", cumulated_pixel_num + a_varve_line.varve_pt[i].begin_pt.pixel_num, a_varve_line.varve_pt[i].kurtosis);
    }

    fclose(fp);
    return;
  }
  
  if (strcmp(attr_name, "mean") == 0)
  {
     for (i=0; i<a_varve_line.num_varves-1; i++)
    {
       fprintf(fp, "%d %8.4f\n", cumulated_pixel_num + a_varve_line.varve_pt[i].begin_pt.pixel_num, a_varve_line.varve_pt[i].mean);
    }

    fclose(fp);
    return;
  }
  
  if (strcmp(attr_name, "sd") == 0)
  {
     for (i=0; i<a_varve_line.num_varves-1; i++)
    {
       fprintf(fp, "%d %8.4f\n", cumulated_pixel_num + a_varve_line.varve_pt[i].begin_pt.pixel_num, a_varve_line.varve_pt[i].sd);
    }

    fclose(fp);
    return;
  }
  
  if (strcmp(attr_name, "peak_rgb") == 0)
  {
     for(i=0; i<a_varve_line.num_varves; i++)
     {
        fprintf(fp, "%d %d %d %d\n", cumulated_pixel_num + a_varve_line.varve_pt[i].peak_pt.pixel_num, 
                                  a_varve_line.varve_pt[i].peak_pt.value_r, 
			          a_varve_line.varve_pt[i].peak_pt.value_g, 
			          a_varve_line.varve_pt[i].peak_pt.value_b, 
                                  a_varve_line.varve_pt[i].peak_pt.value_gray); 
     }

     fclose(fp);
     return;
  }

}

void output_rgb_array(rgb_point rgb_array[], int sizeofarray, char outfile_name[])
{
  int i;
  FILE *fp;

  if (debug)
  {
     printf("the size of array %d, outfile_name: %s \n", sizeofarray, outfile_name);
  }

  if (sizeofarray < 1)
  {
     printf("this is not right, the size of array has to be greater than 0! \n");
     exit(-909);
  }

  fp = fopen(outfile_name, "w");
  if (fp == NULL)
  {
      printf("cannot open file %s for writing! \n", outfile_name);
      exit(303);
  }

  fprintf(fp, "%d\n", sizeofarray);

  for(i=0; i<sizeofarray; i++)
  {
     fprintf(fp, "%d %d %d %d\n", rgb_array[i].pixel_num, 
                                  rgb_array[i].value_r, 
			          rgb_array[i].value_g, 
			          rgb_array[i].value_b, 
                                  rgb_array[i].value_gray); 
  }
 
  fclose(fp);
}

void output_varve_line_peak_rgb(t_varve_line t_line, char outfile_name[])
{
  int i;
  FILE *fp;

  fp = fopen(outfile_name, "w");
  if (fp == NULL)
  {
      printf("cannot open file %s for writing! \n", outfile_name);
      exit(303);
  }

  fprintf(fp, "%d\n", t_line.num_varves);

  for(i=0; i<t_line.num_varves; i++)
  {
     fprintf(fp, "%d %d %d %d\n", t_line.varve_pt[i].peak_pt.pixel_num, 
                                  t_line.varve_pt[i].peak_pt.value_r, 
			          t_line.varve_pt[i].peak_pt.value_g, 
			          t_line.varve_pt[i].peak_pt.value_b, 
                                  t_line.varve_pt[i].peak_pt.value_gray); 
  }
 
 fclose(fp);
}

