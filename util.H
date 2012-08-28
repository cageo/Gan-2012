void init_rgb_point(rgb_point *);
void init_varve_point(t_varve *);
void print_rgb_point(rgb_point pt);
void copy_rgb_point(rgb_point *t_pt, rgb_point s_pt);
void print_varve_point(t_varve t_var);
void copy_varve_point(t_varve *t_var, t_varve s_var);
void print_varve_line(t_varve_line t_line);
void copy_varve_line(t_varve_line *t_line, t_varve_line s_line);
void print_rgb_point_array(rgb_point *rpt_array, int num_pt);
void output_varve_depth_attributes(float d, double dp, t_varve_line, char *attr_name,
                              char outfile_name[], char *fmode);
void output_varve_attributes( t_varve_line a_varve_line, 
			      char *attr_name, 
			      char outfile_name[], char *fmode);
void output_rgb_array(rgb_point rgb_array[], 
			int sizeofarray, char outfile_name[]);
void output_varve_line_peak_rgb(t_varve_line t_line, 
			char outfile_name[]);
void copy_varve_position(t_varve_position *t_p, 
			t_varve_position s_p);
void print_varve_position(t_varve_position s_p);

