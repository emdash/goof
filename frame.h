#ifndef __FRAME_H__

#include <glib.h>
#include <glib-object.h>

typedef struct _Frame Frame;


Frame *frame_new (Frame *parent);
void frame_set_local(Frame *frame, gchar *name, GValue *value);
gboolean frame_update (Frame *frame, gchar *name, GValue *value);
GValue *frame_get_value(Frame *frame, gchar *name);
GValue *frame_get_local (Frame *frame, gchar *name);
void frame_dump_stack (Frame *frame);

#endif
