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
      DEF (id(foo) BOOL (TRUE))
      DEF (id(bar) _(blah))
      DEF (id(baz) FUNCTION (id(x)
            IF(VAL (x)
              BLOCK (
                CALL (a_function)
                PRINT (VAL (bar))
                _(x is positive)
              )
              _(x is negative)
            )
        )
      )

//      PRINT (PLUS (I(3) I(4)))

      DEF (id(+) FUNCTION (id(x) id(y)
            PLUS (VAL(x) VAL(y))))

      PRINT (APPLY(VAL(+) _(abc) _(def)))
      PRINT (APPLY(VAL(+) I(3) R(2.4)))

      PRINT (MUL(MINUS (I(5) I(3)) I(4)))
      PRINT (MUL(MINUS (I(5) R(3)) I(4)))

      PRINT (_(test negation))
      PRINT (NOT (BOOL (FALSE)))

      PRINT (APPLY(VAL(baz) VAL(foo)))
      PRINT (APPLY(VAL(baz) BOOL (FALSE)))
      PRINT (CALL (another_function))

      /* test closures */

      DEF (id(outer) FUNCTION(id(x)
        FUNCTION(id(ignored) VAL(x))))
      DEF (id(one_ret) APPLY(VAL(outer) _(1)))

      DEF (id(two_ret) APPLY(VAL(outer) _(2)))
      PRINT (APPLY (VAL(one_ret) _()))
      PRINT (APPLY (VAL(two_ret) _()))

      /* do something recursive */
      PRINT (_(\ncounting backward from 5\n))

      DEF (id(count_backward) FUNCTION(id(x)
            IF(NOT(VAL(x))
                PRINT (_(done))
                BLOCK(
                  PRINT (VAL(x))
                  APPLY(VAL(count_backward) MINUS(VAL(x) I(1)))))))

      APPLY(VAL(count_backward) I(5))
  END
}
