#ifdef _GRAPHICS
  char scalable, speedo, is_scalable, have_fonts, NVDI3;
  int clip_arr[4], clip_on, gr_font, txt_chw, txt_chh,
      txt_clw, txt_clh, mouse_hide;
  int pfont_id, pfont_mode;		/* 007 */
#else
  extern char scalable, speedo, is_scalable, have_fonts, NVDI3;
  extern int text_arr[], clip_arr[4], clip_on, gr_font, txt_chw, txt_chh,
      txt_clw, txt_clh, mouse_hide;
  extern int pfont_id, pfont_mode;	/* 007 */
#endif

void	vdi_reset(void);
void	_v_mouse( int flag );
void	_vs_clip( int flag, Rect *r );
void	_vswr_mode( int mode );
int	_vst_point( int point, int *char_width, int *char_height,
                    int *cell_width, int *cell_height );
void	_vst_color( int color );
int	_vst_font( int id, int scale );
void	_vst_alignment( int hor, int vert );
void	_vst_effects( int eff );
void	_vsl_udsty( int pat );
void	_vsl_type( int type );
void	_vsl_ends( int start, int end );
void	_vsl_color( int color );
void	_vsl_width( int width );
void	_vsf_style( int style );
void	_vsf_color( int color );
void	_vsf_interior( int style );
void	text_2_arr( unsigned char *str, int *num );
void	text_box( int x, int y, int len, int *arr );
void	ftext16_mono( int x, int y, int len );
void	_vst_charmap( int mode );
