// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0051ba48 | Name: FUN_0051ba48


undefined4 __thiscall FUN_0051ba48(int param_1,undefined1 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x214) == 0) {
    uVar1 = FUN_0054ab66(param_2,param_3);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

