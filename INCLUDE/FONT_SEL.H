#ifdef FONT_SEL
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN int max_ht, char_w, char_h, total_fonts;
EXTERN char has_gdos;
EXTERN int vdi_hand, work_out[57];
EXTERN OBJECT *form;

void cur_point(void);
void init_font( int *id, int *point );
int make_form( int tree, int num, char *s, void func( OBJECT *o, int obj ) );
unsigned int do_popup( OBJECT *o, int obj, OBJECT *pop, unsigned int val );
void no_fonts(void);
void load_fonts(void);
int font_sel( int tree, int sys, int *out_id, int *out_pt );
void set_if( int num, int truth );
int hide_if(OBJECT *tree, int truth, int idx );
