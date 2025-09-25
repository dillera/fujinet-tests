#include <fujinet-endian.h>

#ifdef _CMOC_VERSION_
#define fntohs(w) (((uint16_t)U16_MSB(w) << 8) | U16_LSB(w))
#else /* ! _CMOC_VERSION_ */
#define fntohs(w) (w)
#endif
