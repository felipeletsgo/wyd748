// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c0451 | Name: FUN_004c0451


void __fastcall FUN_004c0451(int *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined1 local_ec [64];
  undefined1 local_ac [64];
  int local_6c;
  int local_68;
  uint local_64;
  uint local_60;
  int local_5c;
  uint local_58;
  int local_54;
  int local_50;
  undefined4 local_4c [16];
  int local_c;
  int local_8;
  
  local_c = param_1[0xbe];
  local_8 = param_1[0xbf];
  local_68 = *(int *)(&DAT_0065a790 + *param_1 * 900);
  local_6c = param_1[0xb3] * local_68;
  local_58 = *(uint *)(&DAT_0065a4a8 + param_1[0xb4] * 4 + *param_1 * 900);
  local_54 = *(int *)(&DAT_005baa88 + *param_1 * 8);
  local_50 = *(int *)(&DAT_005baa8c + *param_1 * 8);
  local_5c = 0;
  FUN_00430f10();
  FUN_00430fc0(local_4c);
  local_5c = *(int *)(&DAT_0065a794 + *param_1 * 900) + local_6c * 0x40;
  if (local_c != 0) {
    for (local_60 = 0; local_60 < 0x30; local_60 = local_60 + 1) {
      FUN_00430fc0(local_c + 0x1c50 + local_60 * 0x40);
    }
    for (local_60 = 0; (local_60 < local_58 && (local_60 < 0x30)); local_60 = local_60 + 1) {
      puVar1 = (undefined4 *)(local_5c + (local_60 * local_68 + local_54) * 0x40);
      puVar3 = local_4c;
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar3 = *puVar1;
        puVar1 = puVar1 + 1;
        puVar3 = puVar3 + 1;
      }
      for (local_64 = 0; local_64 < *(int *)(local_c + 0x1c4c) - 1U; local_64 = local_64 + 1) {
        puVar1 = (undefined4 *)
                 FUN_004c5b90(local_ac,local_5c +
                                       (local_60 * local_68 +
                                       *(int *)(local_c + 0x1b8c + local_64 * 4)) * 0x40);
        puVar3 = local_4c;
        for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
          *puVar3 = *puVar1;
          puVar1 = puVar1 + 1;
          puVar3 = puVar3 + 1;
        }
      }
      puVar1 = local_4c;
      puVar3 = (undefined4 *)(local_c + 0x1c50 + local_60 * 0x40);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar3 = *puVar1;
        puVar1 = puVar1 + 1;
        puVar3 = puVar3 + 1;
      }
    }
    *(uint *)(local_c + 0x28d0) = local_58;
  }
  if (local_8 != 0) {
    for (local_60 = 0; local_60 < 0x30; local_60 = local_60 + 1) {
      FUN_00430fc0(local_8 + 0x1c50 + local_60 * 0x40);
    }
    for (local_60 = 0; (local_60 < local_58 && (local_60 < 0x30)); local_60 = local_60 + 1) {
      puVar1 = (undefined4 *)(local_5c + (local_60 * local_68 + local_50) * 0x40);
      puVar3 = local_4c;
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar3 = *puVar1;
        puVar1 = puVar1 + 1;
        puVar3 = puVar3 + 1;
      }
      for (local_64 = 0; local_64 < *(int *)(local_8 + 0x1c4c) - 1U; local_64 = local_64 + 1) {
        puVar1 = (undefined4 *)
                 FUN_004c5b90(local_ec,local_5c +
                                       (local_60 * local_68 +
                                       *(int *)(local_8 + 0x1b8c + local_64 * 4)) * 0x40);
        puVar3 = local_4c;
        for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
          *puVar3 = *puVar1;
          puVar1 = puVar1 + 1;
          puVar3 = puVar3 + 1;
        }
      }
      puVar1 = local_4c;
      puVar3 = (undefined4 *)(local_8 + 0x1c50 + local_60 * 0x40);
      for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
        *puVar3 = *puVar1;
        puVar1 = puVar1 + 1;
        puVar3 = puVar3 + 1;
      }
    }
    *(uint *)(local_8 + 0x28d0) = local_58;
  }
  return;
}

