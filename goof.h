#ifndef __GOOF_H__
#define __GOOF_H__

#include <glib.h>
#include <stdarg.h>
#include <glib-object.h>

typedef struct _Action Action;

typedef void (*action_func) (Action *a, GValue *ret);

struct _Action {
  action_func do_;
};

#define DO(action, ret) action->do_(action, ret)

Action *block (int dummy, ...);
Action *print (gchar *arg);
Action *call (action_func func);

/* useful macros */

#define BLOCK(...) block(0,  __VA_ARGS__, NULL)
#define OBJ(...) object(__VA_ARGS__, NULL)

#define BEGIN {\
    GValue ret = { 0 };\
    Action *x;\
    g_type_init ();\
    x = block(0, 

#define END\
    , NULL);\
    x->do_(x, &ret);}

#endif

#define ACTION(name, ...)\
  typedef struct {\
    Action parent;\
    __VA_ARGS__\
  } name##_struct;

#define ACTION_IMPL(name)\
  static void name##_impl (name##_struct *self, GValue *ret)

#define ACTION_CONSTRUCTOR(name, ...)\
  Action *name (__VA_ARGS__) {\
    name##_struct *self;\
    self = g_new0(name##_struct, 1);\
    self->parent.do_ = (action_func) name##_impl;\

#define END_ACTION\
    return (Action *) self;\
  }

