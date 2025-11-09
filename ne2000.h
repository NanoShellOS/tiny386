#ifndef NE2000_H
#define NE2000_H

#include <stdint.h>

typedef struct NE2000State NE2000State;

#ifndef DISABLE_NET

void ne2000_ioport_write(void *opaque, uint32_t addr, uint32_t val);
uint32_t ne2000_ioport_read(void *opaque, uint32_t addr);
void ne2000_reset_ioport_write(void *opaque, uint32_t addr, uint32_t val);
uint32_t ne2000_reset_ioport_read(void *opaque, uint32_t addr);
void ne2000_asic_ioport_write(void *opaque, uint32_t addr, uint32_t val);
uint32_t ne2000_asic_ioport_read(void *opaque, uint32_t addr);

void ne2000_step(NE2000State *s);
NE2000State *isa_ne2000_init(int base, int irq,
                             void *pic,
                             void (*set_irq)(void *pic, int irq, int level));

#else

#define ne2000_ioport_write(x, y, z)
#define ne2000_ioport_read(x, y) (0)
#define ne2000_reset_ioport_write(x, y, z)
#define ne2000_reset_ioport_read(x, y) (0)
#define ne2000_asic_ioport_write(x, y, z)
#define ne2000_asic_ioport_read(x, y) (0)
#define ne2000_step(x)
#define isa_ne2000_init(x, y, z, t) (NULL)

#endif

#endif /* NE2000_H */
