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

      print (VAL (bar)),

      IF (VAL (foo),
        BLOCK (
          print (str(static_string)),
          call (a_function)
        ),

        print (_(huston, we have a problem))
      )
  END
}
