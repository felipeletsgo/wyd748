// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040c26d | Name: FUN_0040c26d


undefined4 FUN_0040c26d(int param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  
  if (param_3 < 0x1e) {
    if (param_2 != 0) {
      *(int *)(param_2 + 0x148) = param_3;
    }
    if (*(int *)(param_1 + param_3 * 8) == 0) {
      *(int *)(param_1 + param_3 * 8) = param_2;
    }
    else {
      *(int *)(*(int *)(param_1 + 4 + param_3 * 8) + 0x150) = param_2;
    }
    *(int *)(param_1 + 4 + param_3 * 8) = param_2;
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

