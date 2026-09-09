// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040dee8 | Name: FUN_0040dee8


undefined4 __thiscall FUN_0040dee8(int param_1,int param_2,int param_3)

{
  undefined4 local_10;
  
  if ((((param_2 < *(int *)(param_1 + 0x1d0)) || (param_3 < *(int *)(param_1 + 0x1d4))) ||
      (*(int *)(param_1 + 0x1d0) + *(int *)(param_1 + 0x1d8) <= param_2)) ||
     (*(int *)(param_1 + 0x1d4) + *(int *)(param_1 + 0x1dc) <= param_3)) {
    local_10 = 0;
  }
  else {
    local_10 = 1;
  }
  return local_10;
}

