#include <fujinet-endian.h>

#ifdef _CMOC_VERSION_
#define fntohs(w) (((uint16_t)U16_LSB(w) << 8) | U16_MSB(w))
#else /* ! _CMOC_VERSION_ */
#define fntohs(w) (w)
#endif
