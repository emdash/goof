#include "frame.h"

#define G_VALUE_UPDATE(source, dest) {\
  GValue *src = source, *dst = dest;\
    g_value_unset (src);\
    g_value_init (src, G_VALUE_TYPE(src));\
    g_value_copy (src, dest);\
}

struct _Frame {
  Frame *parent;
  GHashTable *symbols;
};


Frame *frame_new (Frame *parent)
{
  Frame *ret;

  ret = g_new0 (Frame, 1);
  ret->parent = parent;
  ret->symbols = g_hash_table_new (g_str_hash, g_str_equal);

  return ret;
}

/* creates a local instance of value */

void frame_set_local(Frame *frame, gchar *name, GValue *value)
{
  GValue *stored;

  /* check whether the key exists, if so overwrite previous GValue */
  /* TODO: freelists? */

  if ((stored = g_hash_table_lookup (frame->symbols, name))) {
    g_value_unset (stored);
  } else {
    stored = g_new0 (GValue, 1);
    g_hash_table_insert (frame->symbols, name, stored);
  }

  g_value_init (stored, G_VALUE_TYPE (value));
  g_value_copy (value, stored);
}

/* updates the symbol in the inner-most frame in which it is defined. If the
 * symbol is defined in the current scope, the effect is the same as calling
 * frame_set_local. Has no effect if the frame is undefined. */

gboolean frame_update (Frame *frame, gchar *name, GValue *value)
{
  GValue *stored;

  if (!frame)
    return FALSE;

  if ((stored = frame_get_local (frame, name))) {
    G_VALUE_UPDATE (stored, value);
    return TRUE;
  }

  return frame_update (frame->parent, name, value);
}

/* recursive search of parent frames for value */

GValue *frame_get_value(Frame *frame, gchar *name)
{
  GValue *stored = NULL;

  if (!frame) {
    return NULL;
  }

  if ((stored = frame_get_local(frame, name)))
    return stored;

  return frame_get_value (frame->parent, name);
}

/* returns local copy of value or NULL */

GValue *frame_get_local (Frame *frame, gchar *name)
{
  GValue *stored = NULL;

  return g_hash_table_lookup (frame->symbols, name);
}
