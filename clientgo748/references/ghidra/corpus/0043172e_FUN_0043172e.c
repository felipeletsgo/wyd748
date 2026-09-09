// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0043172e | Name: FUN_0043172e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0043172e(int param_1)

{
  undefined4 uVar1;
  tagRECT local_1c;
  float local_c;
  float local_8;
  
  if (*(int *)(param_1 + 0x120) == 0) {
    uVar1 = 0;
  }
  else {
    GetClientRect(*(HWND *)(DAT_013b71e0 + 4),&local_1c);
    local_c = (float)(local_1c.right - local_1c.left);
    local_8 = ((float)(local_1c.bottom - local_1c.top) - (local_c / 640.0) * 320.0) / _DAT_005a365c;
    uVar1 = __ftol();
    uVar1 = __ftol(local_1c.right,uVar1);
    uVar1 = (**(code **)(**(int **)(param_1 + 0x11c) + 0x9c))
                      (*(undefined4 *)(param_1 + 0x11c),local_1c.left,uVar1);
  }
  return uVar1;
}

