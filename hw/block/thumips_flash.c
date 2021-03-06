/*
 * =====================================================================================
 *
 *       Filename:  thumips_flash.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/16/2012 05:58:23 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Chen Yuheng (Chen Yuheng), chyh1990@163.com
 *   Organization:  Tsinghua Unv.
 *
 * =====================================================================================
 */
#include "hw/hw.h"
#include "hw/block/flash.h"
#include "sysemu/block-backend.h"
#include "qemu/timer.h"
#include "qemu/bitops.h"
#include "exec/address-spaces.h"
#include "qemu/host-utils.h"
#include "hw/sysbus.h"

struct thumips_flash_state {
  uint32_t base;
  uint32_t size;
  uint8_t *data;
  MemoryRegion *mr;
};

static struct thumips_flash_state flash_state;

static uint64_t flash_read(void *opaque, hwaddr offset,
                           unsigned size)
{
  struct thumips_flash_state *s = (struct thumips_flash_state*)opaque;
  if(size != 4){
    fprintf(stderr, "thumips_flash: must use lw!\n");
    return 0;
  }
  size_t real_off = offset >> 1;
  // if(!(real_off & 0xfff))
  //   fprintf(stderr, "%s real_off=0x%zx\n", __func__, real_off);
  return *(uint16_t*)(s->data + real_off);
}

static void flash_write(void *opaque, hwaddr offset,
                        uint64_t value, unsigned size)
{
  fprintf(stderr, "thumips_flash: Never write!\n");
}


static const MemoryRegionOps flash_ops = {
    .read = flash_read,
    .write = flash_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

void thumips_flash_init(uint32_t base, uint32_t size, const char *filename)
{
  FILE *f;
  uint8_t *data = NULL;
  uint32_t memsize = size / 2;
  fprintf(stderr, "qemu-thumips: load ROM %s, base: 0x%08x, size: 0x%08x\n", filename, base, size);
  data = (uint8_t*)g_malloc(memsize);
  memset(data, 0, memsize);
  flash_state.base = base;
  flash_state.size = size;
  flash_state.data = data;
  
  flash_state.mr = g_new(MemoryRegion, 1);
  memory_region_init_io(flash_state.mr, NULL, &flash_ops, &flash_state, "thumips_flash", size);
  memory_region_add_subregion(get_system_memory(), base, flash_state.mr);

  f = fopen(filename, "rb");
  if(!f){
    perror("qemu-thumips: failed to read file");
    return;
  }
  fseek(f, 0L, SEEK_END);
  long sz = ftell(f);
  fseek(f, 0L, SEEK_SET);
  if(sz > memsize){
    sz = memsize;
    fprintf(stderr, "qemu-thumips: Warning: %s is larger than %ldKB\n",
      filename, sz>>10);
  }
  sz = fread(data, sz, 1, f);
  fclose(f);
}
