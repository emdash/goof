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
                _(x is positive)
              ),
              _(x is negative)
            )
        )
      ),

      print (APPLY(baz, VAL(foo))),
      print (APPLY(baz, boolean (FALSE))),
      print (call (another_function)),

      /* test closures */

      DEF (outer, FUNCTION("x",
        FUNCTION("ignored", VAL(x)))),
      DEF (one_ret, APPLY(outer, _(1))),

      DEF (two_ret, APPLY(outer, _(2))),
      print (APPLY (one_ret, _())),
      print (APPLY (two_ret, _()))
  END
}
