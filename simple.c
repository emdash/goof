#include "goof.h"
#include <gtk/gtk.h>

CALLBACK(a_function)
{
  g_printf ("called a c function\n");
}

static gchar *static_string = "foo";

int main (int argc, char **argv) {
  BEGIN
      DEF (foo, boolean (TRUE)),
      DEF (bar, _(blah)),
      DEF (baz, FUNCTION ("x",
            IF(VAL (x),
              BLOCK (
                call (a_function),
                print (VAL (bar)),
                _("x is positive")
              ),
              _("x is negative")
            )
        )
      ),

      print (APPLY(baz, VAL(foo))),
      print (APPLY(baz, boolean (FALSE)))
  END
}
