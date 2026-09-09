// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004df250 | Name: FUN_004df250


undefined4 * __thiscall
FUN_004df250(undefined4 *param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  short sVar1;
  short sVar2;
  void *pvVar3;
  undefined4 uVar4;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0d69;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004c8ce0();
  local_8 = 0;
  *param_1 = &PTR_FUN_005a4cf8;
  if (param_2 < 2) {
    param_2 = 2;
  }
  param_1[0x2e] = 0;
  param_1[0x26] = 0;
  param_1[0x25] = param_2;
  param_1[0x23] = (param_1[0x25] + 1) * (param_1[0x25] + 1);
  param_1[0x24] = param_1[0x25] * param_1[0x25] * 6;
  param_1[0x22] = 0;
  pvVar3 = _malloc(param_1[0x24] << 1);
  param_1[0x22] = pvVar3;
  param_1[0x21] = 0;
  pvVar3 = _malloc(param_1[0x23] * 0x18);
  param_1[0x21] = pvVar3;
  param_1[0x1b] = 0;
  param_1[0x2d] = 1;
  param_1[9] = 1;
  param_1[0x2f] = param_4;
  param_1[0x20] = param_3;
  if ((param_1[0x22] == 0) || (param_1[0x21] == 0)) {
    if (param_1[0x22] != 0) {
      FUN_0058ffd8(param_1[0x22]);
    }
    if (param_1[0x21] != 0) {
      FUN_0058ffd8(param_1[0x21]);
    }
  }
  if (param_1[0x22] != 0) {
    local_14 = 0;
    for (local_18 = 0; local_18 < (int)param_1[0x25]; local_18 = local_18 + 1) {
      for (local_1c = 0; local_1c < (int)param_1[0x25]; local_1c = local_1c + 1) {
        sVar2 = (short)local_18;
        sVar1 = (short)local_1c;
        *(short *)(param_1[0x22] + local_14 * 2) = sVar1 + sVar2 * ((short)param_1[0x25] + 1);
        *(short *)(param_1[0x22] + (local_14 + 1) * 2) =
             sVar1 + (sVar2 + 1) * ((short)param_1[0x25] + 1);
        *(short *)(param_1[0x22] + (local_14 + 2) * 2) =
             sVar1 + 1 + sVar2 * ((short)param_1[0x25] + 1);
        *(short *)(param_1[0x22] + (local_14 + 3) * 2) =
             sVar1 + (sVar2 + 1) * ((short)param_1[0x25] + 1);
        *(short *)(param_1[0x22] + (local_14 + 4) * 2) =
             sVar1 + 1 + (sVar2 + 1) * ((short)param_1[0x25] + 1);
        *(short *)(param_1[0x22] + (local_14 + 5) * 2) =
             sVar1 + 1 + sVar2 * ((short)param_1[0x25] + 1);
        local_14 = local_14 + 6;
      }
    }
  }
  if (param_1[0x21] != 0) {
    for (local_20 = 0; local_20 < (int)param_1[0x23]; local_20 = local_20 + 1) {
      *(undefined4 *)(param_1[0x21] + 0xc + local_20 * 0x18) = 0xffffffff;
    }
  }
  param_1[0x28] = 0;
  if (DAT_0092e654 != (int *)0x0) {
    uVar4 = (**(code **)(*DAT_0092e654 + 8))();
    param_1[0x27] = uVar4;
  }
  ExceptionList = local_10;
  return param_1;
}

