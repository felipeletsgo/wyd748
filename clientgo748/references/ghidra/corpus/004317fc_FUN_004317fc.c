// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004317fc | Name: FUN_004317fc


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004317fc(int param_1)

{
  undefined4 uVar1;
  int local_34;
  float local_30;
  float local_2c;
  float local_28;
  int local_24;
  tagRECT local_20;
  float local_10;
  float local_c;
  int local_8;
  
  if (((*(int *)(param_1 + 0x11c) != 0) && (*(int *)(param_1 + 0x120) != 0)) &&
     (local_8 = (**(code **)(**(int **)(param_1 + 0x120) + 0x88))
                          (*(undefined4 *)(param_1 + 0x120),&local_24,&local_34),
     local_8 != -0x7fffbffe)) {
    GetClientRect(*(HWND *)(DAT_013b71e0 + 4),&local_20);
    local_10 = (float)(local_20.right - local_20.left);
    local_28 = (float)(local_20.bottom - local_20.top);
    local_30 = local_10 / (float)local_24;
    local_2c = (float)local_34 * local_30;
    local_c = (local_28 - local_2c) / _DAT_005a365c;
    uVar1 = __ftol();
    uVar1 = __ftol(local_20.right,uVar1);
    (**(code **)(**(int **)(param_1 + 0x11c) + 0x9c))
              (*(undefined4 *)(param_1 + 0x11c),local_20.left,uVar1);
  }
  return;
}

