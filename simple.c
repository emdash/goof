#include "goof.h"
#include <gtk/gtk.h>

static void a_function (Action * a, GValue *ret)
{
  g_printf ("called a c function\n");
}

int main (int argc, char **argv) {
  BEGIN
      boolean (FALSE),
      print ("bar"),
      IF (boolean (TRUE),
        BLOCK (
          print ("foo"),
          print ("baz"),
          call (a_function)
          ),
        print ("huston, we have a problem")
      )
  END
}
