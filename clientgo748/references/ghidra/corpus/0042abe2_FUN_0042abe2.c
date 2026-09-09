// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042abe2 | Name: FUN_0042abe2


undefined4 __fastcall FUN_0042abe2(int param_1)

{
  undefined4 uVar1;
  uint uVar2;
  uint local_10;
  uint local_c;
  undefined4 local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 4) == 0) {
    uVar1 = 0;
  }
  else {
    for (local_c = 0; local_c < *(uint *)(param_1 + 0x10); local_c = local_c + 1) {
      if (*(int *)(*(int *)(param_1 + 4) + local_c * 4) != 0) {
        local_10 = 0;
        (**(code **)(**(int **)(*(int *)(param_1 + 4) + local_c * 4) + 0x24))
                  (*(undefined4 *)(*(int *)(param_1 + 4) + local_c * 4),&local_10);
        if ((local_10 & 1) == 0) break;
      }
    }
    if (local_c == *(uint *)(param_1 + 0x10)) {
      uVar2 = _rand();
      uVar1 = *(undefined4 *)(*(int *)(param_1 + 4) + (uVar2 % *(uint *)(param_1 + 0x10)) * 4);
    }
    else {
      uVar1 = *(undefined4 *)(*(int *)(param_1 + 4) + local_c * 4);
    }
  }
  return uVar1;
}

