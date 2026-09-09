// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6d20 | Name: FUN_004b6d20


void __thiscall FUN_004b6d20(int param_1,int param_2)

{
  undefined1 local_8 [4];
  
  local_8[0] = 0;
  *(int *)(param_1 + 8) = param_2;
  FUN_004b6d60(*(int *)(param_1 + 4) + param_2,local_8);
  return;
}

