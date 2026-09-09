// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044dc1f | Name: FUN_0044dc1f


void __fastcall FUN_0044dc1f(int param_1)

{
  undefined1 uVar1;
  int iVar2;
  int local_24;
  int local_20;
  int *local_1c;
  int local_18;
  int local_14;
  int local_c;
  undefined1 local_8;
  
  local_c = 10;
  if (*(int *)(*(int *)(param_1 + 0x2871c) + 0x28) != 0) {
    local_c = 0x14;
  }
  iVar2 = local_c - *(int *)(param_1 + 0x27ae4);
  if (*(char *)(param_1 + 0x26eb4) == '\x01') {
    uVar1 = *(undefined1 *)(DAT_013b71e8 + 0xef5);
    local_8 = (undefined1)iVar2;
    if (*(char *)(DAT_013b71e8 + 0xef5) < iVar2) {
      *(undefined1 *)(DAT_013b71e8 + 0xef5) = local_8;
    }
    else {
      *(char *)(DAT_013b71e8 + 0xef5) = *(char *)(DAT_013b71e8 + 0xef5) + '\x01';
      if (local_c <= *(char *)(DAT_013b71e8 + 0xef5)) {
        *(undefined1 *)(DAT_013b71e8 + 0xef5) = local_8;
      }
    }
    local_1c = *(int **)(param_1 + 0x28718);
    if (local_c == 0x14) {
      local_1c = *(int **)(param_1 + 0x2871c);
    }
    local_18 = (**(code **)(*local_1c + 0xb4))
                         ((int)*(char *)(DAT_013b71e8 + 0xef5) +
                          *(int *)(*(int *)(param_1 + 0x2871c) + 0x28) * -10,0);
    if (local_18 == 0) {
      local_24 = *(char *)(DAT_013b71e8 + 0xef5) + 1;
      if (local_c <= local_24) {
        local_24 = iVar2;
      }
      for (local_20 = local_24; local_20 < local_c; local_20 = local_20 + 1) {
        local_18 = (**(code **)(*local_1c + 0xb4))
                             (local_20 + *(int *)(*(int *)(param_1 + 0x2871c) + 0x28) * -10,0);
        if (local_18 != 0) {
          *(undefined1 *)(DAT_013b71e8 + 0xef5) = (undefined1)local_20;
          break;
        }
      }
    }
    if (local_18 == 0) {
      *(undefined1 *)(DAT_013b71e8 + 0xef5) = uVar1;
      iVar2 = (**(code **)(*local_1c + 0xb4))
                        ((int)*(char *)(DAT_013b71e8 + 0xef5) +
                         *(int *)(*(int *)(param_1 + 0x2871c) + 0x28) * -10,0);
      if (iVar2 == 0) {
        return;
      }
    }
    for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
      iVar2 = (**(code **)(**(int **)(param_1 + 0x28718) + 0xb4))(local_14,0);
      if ((local_14 == *(char *)(DAT_013b71e8 + 0xef5)) && (iVar2 != 0)) {
        *(undefined4 *)(iVar2 + 0xa0) = 2;
        if (DAT_005b892c == 2) {
          *(undefined4 *)(iVar2 + 0xa0) = 200;
        }
      }
      else if ((iVar2 != 0) && (*(undefined4 *)(iVar2 + 0xa0) = 1, DAT_005b892c == 2)) {
        *(undefined4 *)(iVar2 + 0xa0) = 199;
      }
    }
    for (local_14 = 0; local_14 < 10; local_14 = local_14 + 1) {
      iVar2 = (**(code **)(**(int **)(param_1 + 0x2871c) + 0xb4))(local_14,0);
      if ((local_14 + 10 == (int)*(char *)(DAT_013b71e8 + 0xef5)) && (iVar2 != 0)) {
        *(undefined4 *)(iVar2 + 0xa0) = 2;
        if (DAT_005b892c == 2) {
          *(undefined4 *)(iVar2 + 0xa0) = 200;
        }
      }
      else if ((iVar2 != 0) && (*(undefined4 *)(iVar2 + 0xa0) = 1, DAT_005b892c == 2)) {
        *(undefined4 *)(iVar2 + 0xa0) = 199;
      }
    }
  }
  return;
}

