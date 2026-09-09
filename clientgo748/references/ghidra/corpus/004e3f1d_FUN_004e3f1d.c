// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e3f1d | Name: FUN_004e3f1d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e3f1d(int param_1)

{
  undefined4 uVar1;
  undefined4 local_8;
  
  if (_DAT_005a439c <= *(float *)(param_1 + 0x94)) {
    if (*(int *)(param_1 + 0x9c) < 2) {
      local_8 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0xa8),1,180000);
    }
    else {
      local_8 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0xa8),0,180000);
    }
    if (local_8 == (int *)0x0) {
      uVar1 = 0;
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      if (*(int *)(param_1 + 0x9c) < 2) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      }
      if (*(int *)(param_1 + 0xa4) == 1) {
        (**(code **)(*local_8 + 8))
                  (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                   *(undefined4 *)(param_1 + 0x7c),
                   *(float *)(param_1 + 0x94) * _DAT_005a4104 + _DAT_005a4380,0,
                   *(float *)(param_1 + 0x94) * _DAT_005a4104 + _DAT_005a4380,0,0);
      }
      else {
        (**(code **)(*local_8 + 8))
                  (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                   *(undefined4 *)(param_1 + 0x7c),0x3fc90fdb,0,0x3fc90fdb,0,0);
      }
      if (*(int *)(param_1 + 0x9c) < 2) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      }
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

