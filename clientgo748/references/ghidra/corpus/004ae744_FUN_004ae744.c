// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ae744 | Name: FUN_004ae744


bool __thiscall FUN_004ae744(int param_1,undefined4 param_2)

{
  int iVar1;
  
  *(undefined4 *)(param_1 + 0x444) = param_2;
  iVar1 = FUN_004ae862(param_2);
  return iVar1 != 0;
}

