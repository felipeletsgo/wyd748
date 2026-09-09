// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004dcb83 | Name: FUN_004dcb83


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004dcb83(int param_1)

{
  float fVar1;
  uint uVar2;
  void *pvVar3;
  undefined4 uVar4;
  uint uVar5;
  int local_3c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0cde;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  uVar5 = uVar2 - *(int *)(param_1 + 0x48);
  if (((*(int *)(param_1 + 0x4c) == 0) || (uVar5 <= *(uint *)(param_1 + 0x4c))) ||
     (uVar2 <= *(uint *)(param_1 + 0x48))) {
    fVar1 = (float)uVar5 / (float)*(int *)(param_1 + 0x4c);
    if (((_DAT_005a3534 <= fVar1) && (fVar1 < (float)_DAT_005a4c28)) &&
       ((DAT_0092e660 == 0 && ((float)*(int *)(param_1 + 0x50) < fVar1 * _DAT_005a4314)))) {
      *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + 1;
      pvVar3 = operator_new(0x180);
      local_8 = 0;
      if (pvVar3 == (void *)0x0) {
        local_3c = 0;
      }
      else {
        uVar2 = *(uint *)(param_1 + 0x50) & 0x80000001;
        if ((int)uVar2 < 0) {
          uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
        }
        local_3c = FUN_004c8f70(*(int *)(param_1 + 0x54) + uVar2,1000,0x3f000000,0x3f000000,
                                0x3f000000,
                                (float)*(int *)(param_1 + 0x50) * _DAT_005a4c20 + _DAT_005a3534,1,
                                0x50);
      }
      local_8 = 0xffffffff;
      if (local_3c != 0) {
        *(undefined4 *)(local_3c + 0x6c) = 1;
        uVar4 = FUN_004310a0(param_1 + 0x24);
        FUN_004310a0(uVar4);
        *(undefined4 *)(local_3c + 0x148) = 0x11;
        FUN_004310a0(param_1 + 0x3c);
        FUN_0054ac09(local_3c);
      }
    }
  }
  else {
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  ExceptionList = local_10;
  return 1;
}

