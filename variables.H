t_varve_image varve_image;
t_varve_image wi;
t_varve_image f_image;
t_varve_image cleaned_varve_image;

rgb_point *data, *peaks;
int num_lines, num_points, num_peaks, num_varves;

unsigned char *rarray, *original_rarray;
unsigned char *garray, *original_garray;
unsigned char *barray, *original_barray;
unsigned char *original_gray;
int **gray_array;
long unsigned int width;
long int height;
int begin_x, begin_y, end_x, end_y, num_p; 
int min_section_distance;

char       infile_stub[100];
int       *local_minmax;
int       *section_min;
int       *section_max;
int        num_minmax;
int        outliners_min = 10;
int        outliners_max = 245;
int        cumulated_pixel_num = 0;
float      begin_depth, end_depth;
double     d_p_factor;
float      begin_age, end_age;
double     a_p_factor;

t_varve_position *varve_positions;
int num_varve_positions;
int *num_line_extended;
int **final_varve_index;

float *original_snr, *improved_snr;
int   num_sector;

int debug = 0;
int get_border_welcome = 1;
int threshold_peak;

int *mean_thickness;
int *mean_value_r, *mean_value_g, *mean_value_b;
int *mean_laminae_r, *mean_laminae_g, *mean_laminae_b;
