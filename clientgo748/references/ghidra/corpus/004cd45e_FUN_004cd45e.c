// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cd45e | Name: FUN_004cd45e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004cd45e(int *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  float10 fVar4;
  undefined1 *puVar5;
  undefined1 local_60 [12];
  undefined1 local_54 [12];
  undefined1 local_48 [24];
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  float local_24;
  float local_20;
  int local_1c;
  int local_18;
  int local_14;
  uint local_10;
  int local_c;
  float local_8;
  
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  FUN_004310a0(param_1 + 0x22);
  (**(code **)(*param_1 + 0x38))();
  if (param_1[0x21] == 0) {
    local_8 = (float)(uVar1 % 1000) / _DAT_005a43c0;
  }
  else {
    local_8 = (float)(uVar1 - param_1[0x20]) / (float)param_1[0x21];
  }
  if ((param_1[0x21] != 0) && ((uint)param_1[0x21] < uVar1 - param_1[0x20])) {
    param_1[10] = 0;
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  if ((param_1[0x5f] == 1) && (local_8 < _DAT_005a3660)) {
    fVar4 = (float10)FUN_00423b00(local_8 * _DAT_005a4148);
    local_20 = (float)fVar4;
    local_c = __ftol();
    local_14 = __ftol();
    local_1c = __ftol();
    local_10 = __ftol();
    for (local_18 = 0; local_18 < 4; local_18 = local_18 + 1) {
      param_1[local_18 * 6 + 0x25] = local_c << 0x18 | local_14 << 0x10 | local_1c << 8 | local_10;
      param_1[local_18 * 6 + 0x3d] = local_c << 0x18 | local_14 << 0x10 | local_1c << 8 | local_10;
    }
  }
  if (param_1[0x60] != 0) {
    local_24 = (float)(uVar1 - param_1[0x60]) /
               (float)((param_1[0x20] + param_1[0x21]) - param_1[0x60]);
    FUN_00430f50(param_1[0x1d],(float)param_1[0x1e] + (float)param_1[0x53],param_1[0x1f]);
    FUN_00430f50(param_1[0x28],(float)param_1[0x29] + (float)param_1[0x53],param_1[0x2a]);
    uVar2 = FUN_004c8c60(local_54,local_24);
    puVar5 = local_60;
    FUN_004c8c60(local_48,_DAT_005a3660 - local_24);
    puVar3 = (undefined4 *)FUN_00493d50(puVar5,uVar2);
    FUN_004ccfed(local_30,local_2c,local_28,*puVar3,puVar3[1],puVar3[2]);
  }
  return 1;
}

