# include "gray_2d.H" 

using namespace std;

/*******
Author: Stoney Q. Gan
copyright: Stoney Q. Gan 

Description
This is a driver to process individual images.

*********/

int main ( long argc, char *argv[] )
{
  char str_line[128], p_str[256];
  int all_in_one = 0, debug = 0;
  int outbmp = 0;
  FILE *fp, *fps;
 
   /* argv[1]: input file;
      argv[2]: all_in_one or not -- default is 0;
      argv[3]: whether output wmv, local_contrast bmp files -- default is 0;
      argv[4]: debug level -- default is 0;
   */
   

  if (argc < 2)
  {
    printf("usage: LA-2D input_file [whether collate output]  [whether output enhanced image files] [debug_level]\n");
    exit(105);
  }

  if (argc > 2)
     all_in_one = atoi(argv[2]);

  if (argc > 3)
     outbmp = atoi(argv[3]);

  if (argc == 5)
     debug = atoi(argv[4]);

  fps = fopen("section_summary.txt", "w");
  if (!fps)
  {
       printf("This program requires the section_depth_info.txt file \n");
       printf("I cannot open the section_summary.txt file! This file either does not exist or cannot be opened.\n");
       exit(-999);
  }

  //fprintf(fps, "section_name begin_x end_x num_pixel begin_y end_y width height section_b_d section_e_d section_e_d-section_b_d begin_age end_age end_age-begin_age num_pixel num_varves varve/depth pixel/depth varve/year pixel/year\n");
  fprintf(fps, "section_name begin_x end_x num_pixel begin_y end_y width height section_b_d section_e_d section_e_d-section_b_d begin_age end_age end_age-begin_age num_pixel num_laminas laminas/mm pixel/mm laminas/year pixel/year\n");   
  fclose(fps);
  
  if (all_in_one)
  {
     fp = fopen("all_depth_laminae_mean_thickness.txt", "w");
     if (!fp)
     {
       printf("cannot open the all_depth_laminae_thickness.txt file!\n");
       exit(105);
     }
     fclose(fp);

     fp = fopen("all_depth_laminae_mean_rgb.txt", "w");
     if (!fp)
     {
       printf("cannot open the all_depth_laminae_mean_rgb.txt file!\n");
       exit(105);
     }
     fclose(fp);

     fp = fopen("all_depth_mean_rgb.txt", "w");
     if (!fp)
     {
       printf("cannot open the all_depth_mean_rgb.txt file!\n");
       exit(105);
     }
     fclose(fp);

     fp = fopen("all_age_laminae_mean_thickness.txt", "w");
     if (!fp)
     {
       printf("cannot open the all_age_laminae_mean_thickness.txt file!\n");
       exit(105);
     }
     fclose(fp);

     fp = fopen("all_age_laminae_mean_rgb.txt", "w");
     if (!fp)
     {
       printf("cannot open the all_age_laminae_mean_rgb.txt file!\n");
       exit(105);
     }
     fclose(fp);

     fp = fopen("all_age_mean_rgb.txt", "w");
     if (!fp)
     {
       printf("cannot open the all_age_mean_rgb.txt file!\n");
       exit(105);
     }
     fclose(fp);
  }

	// open input for reading
  fp = fopen(argv[1], "r");
  if (!fp)
  {
    printf("cannot open the input file %s \n",  argv[1]);
    exit(105);
  }

  while (fgets(str_line, 128, fp))
  {
    sprintf(p_str, "./laminasID \"%s\" %d %d %d", str_line, all_in_one, outbmp, debug);
    system(p_str);
  } 
 
  fclose(fp);
}
