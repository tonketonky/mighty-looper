#include "m_pd.h"  
 
static t_class *counter_class;  
 
typedef struct _counter {  
  t_object  x_obj;  
  t_int i_count;

} t_counter;  
 
void counter_get(t_counter *x) {  
  outlet_float(x->x_obj.ob_outlet, x->i_count);
}  

void counter_increase(t_counter *x) {  
  x->i_count++;
  outlet_float(x->x_obj.ob_outlet, x->i_count);
} 

void counter_decrease(t_counter *x) {  
  x->i_count--;
  outlet_float(x->x_obj.ob_outlet, x->i_count);
} 

void counter_reset(t_counter *x) {  
  x->i_count=0;
  outlet_float(x->x_obj.ob_outlet, x->i_count);
} 

void *counter_new(void) {  
  t_counter *x = (t_counter *)pd_new(counter_class);  
  
  x->i_count=0;
  outlet_new(&x->x_obj, &s_float);

  return (void *)x;  
}  
 
void counter_setup(void) {  
  counter_class = class_new(gensym("counter"),  
        (t_newmethod)counter_new,  
        0, sizeof(t_counter),  
        CLASS_DEFAULT, 0);  

  class_addbang(counter_class, counter_get);

  class_addmethod(counter_class,
        (t_method)counter_increase,
        gensym("incr"), 0);

  class_addmethod(counter_class,
        (t_method)counter_decrease,
        gensym("decr"), 0);

  class_addmethod(counter_class,
        (t_method)counter_reset,
        gensym("reset"), 0);
}
