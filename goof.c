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

#define DEFINE_MACHINE_TYPE(goof_name, c_name, particle, gtype)\
  DEFINE_TYPE(goof_name, c_name, particle, gtype)\
  {\
    self->value = value;\
  }\
  END_TYPE

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

/* unary operators */

ACTION(not,
  Action *arg;
)

ACTION_IMPL(not)
{
  GValue result = { 0 };
  DO(self->arg, &result);
  g_value_init (ret, G_TYPE_BOOLEAN);
  g_value_transform (&result, ret);
  g_value_set_boolean (ret, ! g_value_get_boolean (ret));
}

ACTION_CONSTRUCTOR(not, Action *arg)
{
  self->arg = arg;
}
END_ACTION

/* binary operators */

static promote (GType l, GType r) {

  if ((l == G_TYPE_DOUBLE) || (r == G_TYPE_DOUBLE))
    return G_TYPE_DOUBLE;

  if ((l == G_TYPE_INT64) || (r == G_TYPE_INT64))
    return G_TYPE_INT64;

  if ((l == G_TYPE_UINT64) || (r == G_TYPE_UINT64))
    return G_TYPE_UINT64;

  if ((l == G_TYPE_INT) && (r == G_TYPE_INT))
    return G_TYPE_INT;

  if ((l == G_TYPE_STRING) && (r == G_TYPE_STRING))
    return G_TYPE_STRING;

  return G_TYPE_INVALID;
}

#define INFIX_OP(gtype, particle, operator) \
  case gtype:\
    (g_value_set_##particle (ret, \
        g_value_get_##particle (ret) \
          operator g_value_get_##particle (&temp)));\
   break;

#define OP(gtype) \
  case gtype: {\

#define ENDOP\
  }\
  break;

#define OP_INTERNAL(operator, ...)\
  switch (t) {\
    INFIX_OP(G_TYPE_INT, int, operator)\
    INFIX_OP(G_TYPE_DOUBLE, double, operator)\
    INFIX_OP(G_TYPE_UINT64, uint64, operator)\
    INFIX_OP(G_TYPE_INT64, int64, operator)\
    __VA_ARGS__\
  };


ACTION(plus,
    Action *l;
    Action *r;
)

ACTION_IMPL(plus)
{
  GValue l = { 0 };
  GValue r = { 0 };
  GValue temp = { 0 };
  GType t, rt, lt;

  DO(self->l, &l);
  DO(self->r, &r);

  lt = G_VALUE_TYPE(&l);
  rt = G_VALUE_TYPE(&r);

  if ((t = promote (lt, rt)) == G_TYPE_INVALID) 
    g_error ("incompatible types for operation 'plus' (%s, %s)",
        g_type_name (lt), g_type_name (rt));

  g_value_init (ret, t);
  g_value_init (&temp, t);
  g_value_transform (&l, ret);
  g_value_transform (&r, &temp);

  OP_INTERNAL (+,
      OP(G_TYPE_STRING)
        const gchar *l, *r;
        l = g_value_get_string (ret);
        r = g_value_get_string (&temp);
        g_value_set_string (ret, g_strconcat (l, r, NULL));
      ENDOP
  )
}

ACTION_CONSTRUCTOR(plus, Action *l, Action *r)
{
  self->r = r;
  self->l = l;
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
