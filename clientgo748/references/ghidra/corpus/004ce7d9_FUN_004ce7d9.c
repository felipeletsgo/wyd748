// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ce7d9 | Name: FUN_004ce7d9


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004ce7d9(int param_1)

{
  int *piVar1;
  int *piVar2;
  float local_10;
  
  if (((DAT_0092e660 != 1) && (_DAT_005a439c <= *(float *)(param_1 + 0x9c))) &&
     (*(int *)(param_1 + 0x28) == 1)) {
    piVar1 = (int *)FUN_004b14fd(0x2c0,1,180000);
    piVar2 = (int *)FUN_004b14fd(0x2c1,1,180000);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    local_10 = 1.0;
    if ((*(int *)(param_1 + 0x1c) != 0) && (*(int *)(*(int *)(param_1 + 0x1c) + 0x24) == 3)) {
      local_10 = 1.8;
    }
    if (*(int *)(param_1 + 0xa0) == 3) {
      if (piVar1 != (int *)0x0) {
        piVar1[0xcb] = (int)(_DAT_005a4520 * local_10);
      }
      if (piVar2 != (int *)0x0) {
        piVar2[0xcb] = (int)(_DAT_005a4520 * local_10);
      }
    }
    else {
      if (piVar1 != (int *)0x0) {
        piVar1[0xcb] = (int)(_DAT_005a415c * local_10);
      }
      if (piVar2 != (int *)0x0) {
        piVar2[0xcb] = (int)(_DAT_005a415c * local_10);
      }
    }
    if (*(int *)(param_1 + 0xa0) == 0) {
      if (piVar1 != (int *)0x0) {
        piVar1[0xa6] = 0x39;
      }
      if (piVar2 != (int *)0x0) {
        piVar2[0xa6] = 0x39;
      }
    }
    else {
      if (piVar1 != (int *)0x0) {
        piVar1[0xa6] = 99;
      }
      if (piVar2 != (int *)0x0) {
        piVar2[0xa6] = 99;
      }
    }
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),-*(float *)(param_1 + 0x98),0,0x3fc90fdb,0,0);
    }
    if (piVar2 != (int *)0x0) {
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),-*(float *)(param_1 + 0x98),0,0x3fc90fdb,0,0);
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
  }
  return 1;
}

