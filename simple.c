#include "goof.h"

CALLBACK(a_function)
{
  g_printf ("called a c function\n");
}

CALLBACK(another_function)
{
  g_value_init (ret, G_TYPE_FLOAT);
  g_value_set_float (ret, 1.234);
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
      print (APPLY(baz, boolean (FALSE))),
      print (call (another_function))
  END
}
