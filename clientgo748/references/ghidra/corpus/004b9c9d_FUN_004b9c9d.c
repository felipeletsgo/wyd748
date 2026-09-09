// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b9c9d | Name: FUN_004b9c9d


undefined4 __thiscall FUN_004b9c9d(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if ((*(char *)(param_1 + 0x43427 + param_2 * 0x108) == 'A') ||
     (*(char *)(param_1 + 0x43427 + param_2 * 0x108) == 'a')) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

