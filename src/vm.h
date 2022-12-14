#ifndef DBCHIP_SRC_VM_H
#define DBCHIP_SRC_VM_H

int vm_init(char *filename, unsigned int speed);
void vm_quit();
void vm_reset();

char *vm_get_error();

void vm_run();

#include <stdint.h>

extern int running;
extern int paused;

extern uint16_t pc;

void vm_stack_push(uint16_t n);
uint16_t vm_stack_pop();
uint8_t vm_stack_ptr();
uint16_t vm_stack_get(uint8_t i);

extern uint8_t *memory;

extern uint8_t regs[16];
extern uint16_t addr;

extern uint8_t flags[16];

extern uint8_t plane;

extern uint8_t delay;

extern int waitreg;

void vm_decode(uint8_t s, uint8_t x, uint8_t y, uint8_t n, uint8_t nn, uint16_t nnn);
void vm_step();

#endif //DBCHIP_SRC_VM_H
