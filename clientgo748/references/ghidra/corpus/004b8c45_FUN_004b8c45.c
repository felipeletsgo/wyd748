// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b8c45 | Name: FUN_004b8c45


int __thiscall FUN_004b8c45(int param_1,int param_2)

{
  int iVar1;
  
  if ((param_2 < 0x200) && (-1 < param_2)) {
    iVar1 = param_1 + 0x328 + param_2 * 8;
  }
  else {
    iVar1 = 0;
  }
  return iVar1;
}

