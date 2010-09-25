#include "goof.h"

/* c function call */

Action *call (action_func func)
{
  Action *ret;

  ret = g_new0 (Action, 1);
  ret->do_ = func;

  return (Action *) ret;
}

/* types */

DEFINE_MACHINE_TYPE(boolean, gboolean, boolean, G_TYPE_BOOLEAN)
DEFINE_MACHINE_TYPE(integer, gint, int, G_TYPE_INT)
DEFINE_MACHINE_TYPE(real, gdouble, double, G_TYPE_DOUBLE)
DEFINE_MACHINE_TYPE(big, gint64, int64, G_TYPE_INT64)
DEFINE_MACHINE_TYPE(ubig, guint64, uint64, G_TYPE_UINT64)

DEFINE_TYPE(str, gchar *, string, G_TYPE_STRING)
{
  self->value = g_strdup(value);
}
END_TYPE

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
    DO(((Action *) cur->data), ret);
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
  Action *a;
)

ACTION_IMPL(print) {
  GValue v = { 0 };
  GValue string = { 0 };

  DO (self->a, &v);
  g_value_init (&string, G_TYPE_STRING);
  g_value_transform (&v, &string);
  g_printf ("%s\n", g_value_get_string (&string));
}

ACTION_CONSTRUCTOR(print, Action *a) 
{
  self->a = a;
}
END_ACTION

/* vars */

ACTION(def,
  gchar *name;
  Action *expr;
)

ACTION_IMPL(def) {
  DO(self->expr, ret);
  frame_set_local (vars, self->name, ret);
}

ACTION_CONSTRUCTOR(def, gchar *name, Action *value_expr)
{
  self->name = name;
  self->expr = value_expr;
}
END_ACTION

ACTION(val,
    gchar *name;)

ACTION_IMPL(val) {
  GValue *v;
  v = frame_get_value (vars, self->name);
  if (!v) {
    g_error ("value '%s' is undefined", self->name);
  }
  g_value_init (ret, G_VALUE_TYPE (v));
  g_value_copy (v, ret);
}

ACTION_CONSTRUCTOR(val, gchar *name)
{
  self->name = name;
}
END_ACTION

/* function */

ACTION(function,
    GSList *params;
    Action *body;
)

typedef struct {
  function_struct *f;
  Frame *vars;
} closure;

static closure *closure_new(function_struct *f, Frame *v){
  closure *ret = g_new0 (closure, 1);
  ret->f = f;
  ret->vars = frame_new(v);

  return ret;
}

ACTION_IMPL(function)
{
  g_value_init (ret, G_TYPE_POINTER);
  g_value_set_pointer (ret, closure_new (self, vars));
}

ACTION_CONSTRUCTOR(function, int dummy, ...)
{
  va_list l;
  gchar *arg;

  self->params = NULL;

  va_start (l, dummy);
  arg = va_arg (l, gchar *);
  while (arg) {
    self->params = g_slist_prepend (self->params, arg);
    arg = va_arg (l, gchar *);
  }
  va_end(l);

  /* chop the last argument, it's the body */
  self->body = (Action *) self->params->data;
  self->params->data = NULL;
  /* FIXME: this leaks the first list node */
  self->params = g_slist_delete_link (self->params, self->params);
}
END_ACTION

ACTION(apply,
    GSList *params;
    Action *code;)

ACTION_IMPL(apply)
{
  GValue f = { 0 };
  GSList *name, *value;
  Frame *frame;
  closure *c;
  function_struct *code;

  DO(self->code, &f);

  /* FIXME: notice this leaks a frame, but what are you going to do? without
   * ref-counting it's impossible to know when a frame can be freed. a closure
   * could escape and need this frame later. */

  c = g_value_get_pointer (&f);
  code = c->f;
  frame = frame_new(c->vars);

  for (name = code->params, value = self->params; name && value; name =
      name->next, value = value->next)
  {
    GValue *v = g_new0(GValue, 1);
    DO(((Action *) value->data), v);
    frame_set_local (frame, name->data, v);
  }

  if (name || value)
  {
    g_error ("incorrect number of args");
  }

  DO_WITH(code->body, ret, frame);
}

ACTION_CONSTRUCTOR (apply, Action *f, ...)
{
  va_list l;
  Action *arg;

  self->code = f;

  va_start (l, f);
  arg = va_arg (l, Action *);
  while (arg)
  {
    self->params = g_slist_prepend (self->params, arg);
    arg = va_arg (l, Action *);
  }
  va_end (l);
}
END_ACTION
