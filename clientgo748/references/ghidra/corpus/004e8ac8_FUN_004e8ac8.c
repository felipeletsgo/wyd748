// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e8ac8 | Name: FUN_004e8ac8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e8ac8(int param_1)

{
  int *piVar1;
  int *piVar2;
  float local_20;
  float local_10;
  
  if ((((DAT_0092e660 != 1) && (_DAT_005a439c <= *(float *)(param_1 + 0xa4))) &&
      (*(int *)(param_1 + 0x9c) != 0)) && (*(int *)(param_1 + 0x28) == 1)) {
    if (*(int *)(param_1 + 0xa8) == 4) {
      piVar1 = (int *)FUN_004b14fd(0x1f5,1,180000);
      piVar2 = (int *)FUN_004b14fd(0x1f6,1,180000);
      if (piVar1 == (int *)0x0) {
        return 0;
      }
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      piVar2[0xa6] = 0xca;
      piVar1[0xa6] = 0xca;
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
      local_10 = *(float *)(param_1 + 0x90);
      if (((*(int *)(param_1 + 0x1c) != 0) && (*(int *)(*(int *)(param_1 + 0x1c) + 0x24) == 3)) &&
         (*(char *)(*(int *)(param_1 + 0x1c) + 0x79c) == '\x01')) {
        local_10 = 1.0;
      }
      piVar1[0xcb] = (int)(_DAT_005a3f2c * local_10);
      piVar2[0xcb] = (int)(_DAT_005a3f2c * local_10);
      (**(code **)(*piVar1 + 8))
                (*(float *)(param_1 + 0x74) - _DAT_005a34a0,
                 (_DAT_005a414c + *(float *)(param_1 + 0xac)) * local_10 +
                 *(float *)(param_1 + 0x78),*(undefined4 *)(param_1 + 0x7c),0xbf060a92,0xbf060a92,
                 _DAT_005a4148 - *(float *)(param_1 + 0xa0),0,0);
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 (_DAT_005a414c + *(float *)(param_1 + 0xac)) * local_10 +
                 *(float *)(param_1 + 0x78),*(undefined4 *)(param_1 + 0x7c),0xbf060a92,0x3f060a92,
                 _DAT_005a4148 - *(float *)(param_1 + 0xa0),0,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,DAT_005ccf9c[0xac0e]);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    }
    else {
      piVar1 = (int *)FUN_004b14fd(0x2c0,1,180000);
      piVar2 = (int *)FUN_004b14fd(0x2c1,1,180000);
      if (piVar1 == (int *)0x0) {
        return 0;
      }
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      if (*(int *)(param_1 + 0xa8) == 0) {
        piVar2[0xa6] = 0x39;
        piVar1[0xa6] = 0x39;
      }
      else if (*(int *)(param_1 + 0xa8) == 1) {
        piVar2[0xa6] = 0x2d;
        piVar1[0xa6] = 0x2d;
      }
      else if (*(int *)(param_1 + 0xa8) == 3) {
        piVar2[0xa6] = 99;
        piVar1[0xa6] = 99;
      }
      else if (*(int *)(param_1 + 0xa8) == 2) {
        piVar2[0xa6] = 99;
        piVar1[0xa6] = 99;
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
      local_20 = 1.0;
      if (((*(int *)(param_1 + 0x1c) != 0) && (*(int *)(*(int *)(param_1 + 0x1c) + 0x24) == 3)) &&
         (*(char *)(*(int *)(param_1 + 0x1c) + 0x79c) == '\x01')) {
        local_20 = 1.5;
      }
      piVar1[0xcb] = (int)(_DAT_005a3660 * local_20);
      piVar2[0xcb] = (int)(_DAT_005a3660 * local_20);
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) + *(float *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),-*(float *)(param_1 + 0xa0),0,0x3fc90fdb,0,0);
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) + *(float *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),-*(float *)(param_1 + 0xa0),0,0x3fc90fdb,0,0);
      piVar1[0xcb] = (int)(_DAT_005a430c * local_20);
      piVar2[0xcb] = (int)(_DAT_005a430c * local_20);
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) +
                 (*(float *)(param_1 + 0x78) - _DAT_005a4214),*(undefined4 *)(param_1 + 0x7c),
                 -*(float *)(param_1 + 0xa0) + _DAT_005a437c,0,0x3fc90fdb,0,0);
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) +
                 (*(float *)(param_1 + 0x78) - _DAT_005a4214),*(undefined4 *)(param_1 + 0x7c),
                 -*(float *)(param_1 + 0xa0) + _DAT_005a437c,0,0x3fc90fdb,0,0);
      piVar1[0xcb] = (int)(_DAT_005a365c * local_20);
      piVar2[0xcb] = (int)(_DAT_005a365c * local_20);
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) +
                 (*(float *)(param_1 + 0x78) - _DAT_005a4158),*(undefined4 *)(param_1 + 0x7c),
                 -*(float *)(param_1 + 0xa0) + _DAT_005a4380,0,0x3fc90fdb,0,0);
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) +
                 (*(float *)(param_1 + 0x78) - _DAT_005a4158),*(undefined4 *)(param_1 + 0x7c),
                 -*(float *)(param_1 + 0xa0) + _DAT_005a4380,0,0x3fc90fdb,0,0);
      piVar1[0xcb] = (int)(_DAT_005a365c * local_20);
      piVar2[0xcb] = (int)(_DAT_005a365c * local_20);
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) + *(float *)(param_1 + 0x78) + _DAT_005a4214,
                 *(undefined4 *)(param_1 + 0x7c),-*(float *)(param_1 + 0xa0) - _DAT_005a437c,0,
                 0x3fc90fdb,0,0);
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) + *(float *)(param_1 + 0x78) + _DAT_005a4214,
                 *(undefined4 *)(param_1 + 0x7c),-*(float *)(param_1 + 0xa0) - _DAT_005a437c,0,
                 0x3fc90fdb,0,0);
      piVar1[0xcb] = (int)(_DAT_005a430c * local_20);
      piVar2[0xcb] = (int)(_DAT_005a430c * local_20);
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) + *(float *)(param_1 + 0x78) + _DAT_005a4158,
                 *(undefined4 *)(param_1 + 0x7c),-*(float *)(param_1 + 0xa0) - _DAT_005a4380,0,
                 0x3fc90fdb,0,0);
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x74),
                 local_20 * *(float *)(param_1 + 0xac) + *(float *)(param_1 + 0x78) + _DAT_005a4158,
                 *(undefined4 *)(param_1 + 0x7c),-*(float *)(param_1 + 0xa0) - _DAT_005a4380,0,
                 0x3fc90fdb,0,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,DAT_005ccf9c[0xac0e]);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    }
  }
  return 1;
}

