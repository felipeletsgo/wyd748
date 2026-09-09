// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042af62 | Name: FUN_0042af62


uint __fastcall FUN_0042af62(int param_1)

{
  uint uVar1;
  undefined4 local_c;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 4) == 0) {
    local_8 = 0x800401f0;
  }
  else {
    local_8 = 0;
    for (local_c = 0; local_c < *(uint *)(param_1 + 0x10); local_c = local_c + 1) {
      uVar1 = (**(code **)(**(int **)(*(int *)(param_1 + 4) + local_c * 4) + 0x34))
                        (*(undefined4 *)(*(int *)(param_1 + 4) + local_c * 4),0);
      local_8 = local_8 | uVar1;
    }
  }
  return local_8;
}

