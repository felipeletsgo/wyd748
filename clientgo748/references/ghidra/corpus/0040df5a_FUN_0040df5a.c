// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040df5a | Name: FUN_0040df5a


undefined4 __thiscall FUN_0040df5a(int param_1,int param_2,int param_3)

{
  undefined4 local_10;
  
  if ((param_2 == *(int *)(param_1 + 0x1d0)) && (param_3 == *(int *)(param_1 + 0x1d4))) {
    local_10 = 1;
  }
  else {
    local_10 = 0;
  }
  return local_10;
}

