#ifndef __GOOF_H__
#define __GOOF_H__

#include <glib.h>
#include <stdarg.h>
#include <glib-object.h>
#include "frame.h"

#define ACTION_ARGS Action *self, GValue *ret, Frame *vars
#define CALLBACK(name) static void name (ACTION_ARGS)

typedef struct _Action Action;

typedef void (*action_func) (ACTION_ARGS);

struct _Action {
  action_func do_;
};

#define DO(action, ret) action->do_(action, ret, vars)
#define DO_WITH(action, ret, vars) action->do_(action, ret, vars)

Action *block (int dummy, ...);
Action *print (Action *value);
Action *call (action_func func);
Action *def (gchar *string, Action *value_expr);
Action *val (gchar *string);
Action *boolean (gboolean value);
Action *integer (gint value);
Action *real (gdouble value);
Action *big (gint64 value);
Action *ubig (guint64 value);
Action *str (gchar *value);
Action *object (GType type, ...);
Action *function (int dummy, ...);
Action *apply (Action *f, ...);

Action *not (Action *f);
Action *plus (Action *r, Action *l);
Action *minus (Action *r, Action *l);
Action *mul (Action *r, Action *l);
Action *div (Action *r, Action *l);

/* useful macros */

#define BLOCK(...) block(0,  __VA_ARGS__, NULL)
#define OBJ(...) object(__VA_ARGS__, NULL)
#define DEF(name, value) def(#name, value)
#define VAL(name) val(#name)
#define _(...) str(#__VA_ARGS__)
#define FUNCTION(...) function(0, __VA_ARGS__, NULL)
#define APPLY(f, ...) apply(VAL(f), __VA_ARGS__, NULL)

#define BEGIN {\
    GValue ret = { 0 };\
    Frame *vars;\
    Action *x;\
    g_type_init ();\
    vars = frame_new (NULL);\
    x = block(0, 

#define END\
    , NULL);\
    x->do_(x, &ret, vars);}

#endif

#define ACTION(name, ...)\
  typedef struct {\
    Action parent;\
    __VA_ARGS__\
  } name##_struct;

#define ACTION_IMPL(name)\
  static void name##_impl (name##_struct *self, GValue *ret, Frame *vars)

#define ACTION_CONSTRUCTOR(name, ...)\
  Action *name (__VA_ARGS__) {\
    name##_struct *self;\
    self = g_new0(name##_struct, 1);\
    self->parent.do_ = (action_func) name##_impl;\

#define END_ACTION\
    return (Action *) self;\
  }\


#define DEFINE_TYPE(goof_name, c_name, gtype_particle, gtype)\
ACTION(goof_name, \
    c_name value;\
)\
\
ACTION_IMPL(goof_name)\
{\
  g_value_init (ret, gtype);\
  g_value_set_##gtype_particle (ret, self->value);\
}\
\
ACTION_CONSTRUCTOR(goof_name, c_name value)

#define END_TYPE END_ACTION

