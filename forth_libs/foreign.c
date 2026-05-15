#include "../core/forth.h"

// Put your foreign functions here, they must take a Ctx* and return a Cell
// Return value semantics still underspecified, but return 0 only if there were no errors
Cell nomffi_add(Ctx *c) {
    unsigned a = dataPop(c);
    unsigned b = dataPop(c);
    dataPush(c, a + b);
    return 0;
}
