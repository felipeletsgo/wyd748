// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6a40 | Name: FUN_004b6a40


int __thiscall FUN_004b6a40(undefined4 param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  
  iVar1 = FUN_004b6540(param_1);
  FUN_004b6ed0(param_2,1,param_3);
  iVar2 = FUN_004b6540();
  return iVar2 + ((param_2 - iVar1) / 0x28) * 0x28;
}

