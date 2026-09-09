// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ec305 | Name: FUN_004ec305


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004ec305(int param_1)

{
  int *piVar1;
  int local_10;
  
  if (_DAT_005a439c <= *(float *)(param_1 + 0xa4)) {
    FUN_004c8d88();
    if (*(int *)(param_1 + 0x9c) == 1) {
      piVar1 = (int *)FUN_004b14fd(0x2c4,1,180000);
      if (piVar1 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
      piVar1[0xcb] = 0x3f333333;
      piVar1[0xcc] = 0x3f333333;
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),*(float *)(param_1 + 0xa0) + _DAT_005a4380,
                 0x3f490fdb,0,0,0);
      piVar1[0xcb] = 0x3f800000;
      piVar1[0xcc] = 0x3f800000;
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    }
    else if (*(int *)(param_1 + 0x9c) == 2) {
      piVar1 = (int *)FUN_004b14fd(0x648,0,180000);
      if (piVar1 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,0);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),0,0,0,0,0);
    }
    else if (*(int *)(param_1 + 0x9c) == 3) {
      for (local_10 = 0; local_10 < 2; local_10 = local_10 + 1) {
        piVar1 = (int *)FUN_004b14fd(local_10 + 0xb00,0,180000);
        if (piVar1 == (int *)0x0) {
          return 0;
        }
        (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
        (**(code **)(*piVar1 + 8))
                  (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                   *(undefined4 *)(param_1 + 0x7c),*(undefined4 *)(param_1 + 0xa0),0,0,0,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      }
    }
    else if (*(int *)(param_1 + 0x9c) == 5) {
      piVar1 = (int *)FUN_004b14fd(0xb18,1,180000);
      if (piVar1 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,4);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,3);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),*(float *)(param_1 + 0xa0) + _DAT_005a4148,0,0,0,0)
      ;
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

