// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00576b56 | Name: FUN_00576b56


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00576b56(float *param_1,ushort *param_2)

{
  float *pfVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  float local_5c [4];
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  float local_c;
  float local_8;
  
  uVar2 = *param_2;
  FUN_005758cb();
  uVar3 = param_2[1];
  FUN_005758cb();
  if (uVar3 < uVar2) {
    local_3c = (local_4c - local_5c[0]) * _DAT_005a69ac + local_5c[0];
    local_38 = (local_48 - local_5c[1]) * _DAT_005a69ac + local_5c[1];
    local_8 = local_44 - local_5c[2];
    local_34 = local_8 * _DAT_005a69ac + local_5c[2];
    local_c = local_40 - local_5c[3];
    local_30 = local_c * _DAT_005a69ac + local_5c[3];
    local_2c = (local_4c - local_5c[0]) * _DAT_005a8604 + local_5c[0];
    local_28 = (local_48 - local_5c[1]) * _DAT_005a8604 + local_5c[1];
    local_24 = local_8 * _DAT_005a8604 + local_5c[2];
    local_20 = local_c * _DAT_005a8604 + local_5c[3];
  }
  else {
    local_3c = (local_4c - local_5c[0]) * _DAT_005a67d4 + local_5c[0];
    local_38 = (local_48 - local_5c[1]) * _DAT_005a67d4 + local_5c[1];
    local_34 = (local_44 - local_5c[2]) * _DAT_005a67d4 + local_5c[2];
    local_30 = (local_40 - local_5c[3]) * _DAT_005a67d4 + local_5c[3];
    local_1c = 0;
    local_18 = 0;
    local_14 = 0;
    local_10 = 0;
    local_2c = 0.0;
    local_28 = 0.0;
    local_24 = 0.0;
    local_20 = 0.0;
  }
  uVar6 = *(uint *)(param_2 + 2);
  iVar4 = 0x10;
  do {
    uVar5 = uVar6 & 3;
    *param_1 = local_5c[uVar5 * 4];
    param_1[1] = local_5c[uVar5 * 4 + 1];
    pfVar1 = param_1 + 3;
    param_1[2] = local_5c[uVar5 * 4 + 2];
    param_1 = param_1 + 4;
    uVar6 = uVar6 >> 2;
    iVar4 = iVar4 + -1;
    *pfVar1 = local_5c[uVar5 * 4 + 3];
  } while (iVar4 != 0);
  return 0;
}

