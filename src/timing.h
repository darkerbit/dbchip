#ifndef DBCHIP_SRC_TIMING_H
#define DBCHIP_SRC_TIMING_H

void timing_seed_random();

void timing_60_start();
void timing_60_end();

void timing_vm_init(long step);
void timing_vm_clock();
int timing_vm_step();

#endif //DBCHIP_SRC_TIMING_H
