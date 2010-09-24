#include "goof.h"
#include <gtk/gtk.h>

CALLBACK(a_function)
{
  g_printf ("called a c function\n");
}

int main (int argc, char **argv) {
  BEGIN
      DEF (foo, boolean (TRUE)),
      DEF (bar, _(blah)),

      print (VAL (bar)),

      IF (VAL (foo),
        BLOCK (
          print (str("foo")),
          call (a_function)
        ),

        print (str("huston, we have a problem"))
      )
  END
}
