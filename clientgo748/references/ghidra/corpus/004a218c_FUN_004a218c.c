// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a218c | Name: FUN_004a218c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004a218c(int param_1,int param_2,int param_3,int param_4)

{
  undefined4 uVar1;
  undefined1 local_80 [72];
  float local_38;
  float local_30;
  uint local_2c;
  int *local_28;
  uint local_24;
  undefined4 local_20;
  float local_1c;
  float local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  CSimpleArray<>();
  CSimpleArray<>();
  CSimpleArray<>();
  local_24 = (uint)(param_3 != 0);
  local_2c = (uint)(param_3 == 0);
  if (param_4 == 0) {
    uVar1 = FUN_00430f50(0x45018000,0x40b00000,0x45018000);
    FUN_004310a0(uVar1);
    local_20 = 0x40a66666;
    uVar1 = FUN_00430f50(*(undefined4 *)(*(int *)(param_1 + 0x26e40 + param_2 * 4) + 0x28),
                         *(undefined4 *)(*(int *)(param_1 + 0x26e40 + param_2 * 4) + 0x30),
                         *(undefined4 *)(*(int *)(param_1 + 0x26e40 + param_2 * 4) + 0x2c));
    FUN_004310a0(uVar1);
  }
  else {
    uVar1 = FUN_00430f50(0x44ff8000,0x40000000,0x44ff8000);
    FUN_004310a0(uVar1);
    local_20 = 0x3fa66666;
    uVar1 = FUN_00430f50(*(undefined4 *)(*(int *)(param_1 + 0x26e30 + param_2 * 4) + 0x28),
                         *(undefined4 *)(*(int *)(param_1 + 0x26e30 + param_2 * 4) + 0x30),
                         *(undefined4 *)(*(int *)(param_1 + 0x26e30 + param_2 * 4) + 0x2c));
    FUN_004310a0(uVar1);
  }
  local_28 = *(int **)(DAT_013b71e8 + 0x1b084);
  local_28[0x23] = 1;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x2009c) = uVar1;
  _memset((void *)(param_1 + 0x200b4),0,28000);
  *(undefined4 *)(param_1 + 0x200b0) = 2;
  *(undefined4 *)(param_1 + 0x200b4) = 0;
  *(undefined2 *)(param_1 + 0x200b8 + local_24 * 0x1c) = 0;
  *(undefined2 *)(param_1 + 0x200b8 + local_2c * 0x1c) = 0;
  *(undefined4 *)(param_1 + 0x200bc + local_2c * 0x1c) = local_10;
  *(undefined4 *)(param_1 + 0x200c0 + local_2c * 0x1c) = local_c;
  *(undefined4 *)(param_1 + 0x200c4 + local_2c * 0x1c) = local_8;
  *(undefined4 *)(param_1 + 0x200c8 + local_24 * 0x1c) = 0x4233126f;
  *(undefined4 *)(param_1 + 0x200c8 + local_2c * 0x1c) = 0x4233126f;
  *(undefined4 *)(param_1 + 0x200cc + local_24 * 0x1c) = 0;
  *(undefined4 *)(param_1 + 0x200cc + local_2c * 0x1c) = 0;
  *(undefined4 *)(param_1 + 0x200d0) = 400;
  uVar1 = FUN_00430f50(*(undefined4 *)(param_1 + 0x200bc + local_2c * 0x1c),
                       *(undefined4 *)(param_1 + 0x200c0 + local_2c * 0x1c),
                       *(undefined4 *)(param_1 + 0x200c4 + local_2c * 0x1c));
  FUN_004310a0(uVar1);
  uVar1 = (**(code **)(*local_28 + 0x40))(local_80);
  FUN_004310a0(uVar1);
  *(float *)(param_1 + 0x200bc + local_24 * 0x1c) =
       *(float *)(DAT_005ccf9c + 0x2a62c) * _DAT_005a34a0 + (local_1c - local_38 * _DAT_005a3834);
  *(undefined4 *)(param_1 + 0x200c0 + local_24 * 0x1c) = local_20;
  *(float *)(param_1 + 0x200c4 + local_24 * 0x1c) =
       *(float *)(DAT_005ccf9c + 0x2a64c) * _DAT_005a34a0 + (local_14 - local_30 * _DAT_005a3834);
  *(undefined2 *)(param_1 + 0x20098) = 1;
  return;
}

