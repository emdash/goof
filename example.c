#include <goof.h>

BEGIN

window (
    connect ("destroy",
      call_c (gtk_main_quit)
      ),
    add (
      button ("click me",
        connect ("clicked",
          block (
            print ("ouch!"),
            set ("this", "title", "ouch!")
            )
          )
        )
      )
    )


END
