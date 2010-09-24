#include "goof.h"

/* c function call */

Action *call (action_func func)
{
  Action *ret;

  ret = g_new0 (Action, 1);
  ret->do_ = func;

  return (Action *) ret;
}

/* expression */

ACTION(boolean, 
    gboolean value;
)

ACTION_IMPL(boolean)
{
  g_value_init (ret, G_TYPE_BOOLEAN);
  g_value_set_boolean (ret, self->value);
}

ACTION_CONSTRUCTOR(boolean, gboolean v)
{
  self->value = v;
}
END_ACTION

/* if */

ACTION(IF,
  Action *condition;
  Action *positive;
  Action *negative;
)

ACTION_IMPL(IF)
{
  GValue cond = { 0 };
  DO(self->condition, &cond);
  if (g_value_get_boolean (&cond))
  {
    DO(self->positive, ret);
  } else
  {
    DO(self->negative, ret);
  }
}

ACTION_CONSTRUCTOR(IF, Action *cond, Action *t, Action *f)
{
  self->condition = cond;
  self->positive = t;
  self->negative = f;
}
END_ACTION

/* statement block */

ACTION(block,
  GList *actions;
)

ACTION_IMPL(block)
{
  GList *cur;
  for (cur = self->actions; cur; cur=cur->next)
  {
    if (G_IS_VALUE (ret))
      g_value_unset (ret);
    ((Action *) cur->data)->do_((Action *) cur->data, ret);
  }
}

ACTION_CONSTRUCTOR(block, int dummy, ...)
{
  va_list l;
  Action *arg;
  va_start (l, dummy);

  arg = va_arg (l, Action *);
  while (arg)
  {
    self->actions = g_list_prepend(self->actions, arg);
    arg = va_arg (l, Action *);
  }
  va_end (l);

  self->actions = g_list_reverse (self->actions);
}
END_ACTION

/* print statement */
ACTION(print,
  gchar *arg;
)

ACTION_IMPL(print) {
  g_printf ("%s\n", self->arg);
}

ACTION_CONSTRUCTOR(print, gchar *s) 
{
  self->arg = g_strdup (s);
}
END_ACTION

/* object */

#if 0

typedef struct _Object Object;
struct _Object {
  GType type;
  GList *methods;
};

void object_impl (Object *obj, GValue *ret)
{
  GObject *instance;
  GList *method;

  instance = g_object_new (obj->type, NULL);
  g_value_init (ret, obj->type);
  g_value_set_object (ret, instance);

  for (method = obj->methods; method; method=method->next)
  {
    GValue unused = { 0 };
    Action *a = method->data;
    DO(a, &unused);
  }
}

Action *object(GType type, ...)
{
  va_list l;
  Object *ret;
  Action *arg;

  ret = g_new0 (Object, 1);
  ret->parent.do_ = (action_func) object_impl;
  ret->type = type;

  va_start (l, type);

  arg = va_arg (l, Action *);

  while (arg)
  {
    ret->methods = g_list_prepend (ret->methods, arg);
    arg = va_arg (l, Action *);
  }
  va_end (l);

  ret->methods = g_list_reverse (ret->methods);

  return (Action *) ret;
}
#endif
