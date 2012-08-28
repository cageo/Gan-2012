# include <cstdlib>
# include <iostream>
# include <cstring>
# include <string>
# include <iomanip>
# include <fstream>
# include <ctime>
# include <stdio.h>
#include <math.h>

#define threshold_varve 2
#define threshold_value_r_shift 5
#define threshold_percent_connected 50
#define MAX_PTS 35000
#define DARK -1
#define LIGHT 1

typedef struct POINT
{
  int x;
  int y;
} point;

typedef struct RGB_POINT
{ 
  int pixel_num;
  int value_r;
  int value_g;
  int value_b;
  int value_gray;
} rgb_point;


typedef struct T_VARVE
{
   rgb_point begin_pt;
   rgb_point end_pt;
   rgb_point peak_pt;
   int       sign; 
   float     median;
   float     mean;
   float     sd;
   float     kurtosis;
   float     confidence;
   int	     checked;
   int       varve_index;
   int       width;
} t_varve;
   
typedef struct T_VARVE_LINE
{
  t_varve   *varve_pt;
  int       num_varves;
} t_varve_line;

typedef struct T_VARVE_IMAGE
{
  t_varve_line   *varve_line;
  int       num_lines;
} t_varve_image;

typedef struct VARVE_POSITION
{
  int pixel_num;
  int section_num;
  int varve_num;
  int checked;
} t_varve_position;

