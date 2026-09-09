// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042afd0 | Name: FUN_0042afd0


uint __fastcall FUN_0042afd0(int param_1)

{
  uint local_10;
  uint local_c;
  uint local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 4) == 0) {
    local_8 = 0;
  }
  else {
    for (local_c = 0; local_c < *(uint *)(param_1 + 0x10); local_c = local_c + 1) {
      if (*(int *)(*(int *)(param_1 + 4) + local_c * 4) != 0) {
        local_10 = 0;
        (**(code **)(**(int **)(*(int *)(param_1 + 4) + local_c * 4) + 0x24))
                  (*(undefined4 *)(*(int *)(param_1 + 4) + local_c * 4),&local_10);
        local_8 = local_8 | (local_10 & 1) != 0;
      }
    }
  }
  return local_8;
}

