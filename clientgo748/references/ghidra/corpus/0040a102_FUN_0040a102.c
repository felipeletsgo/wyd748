// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040a102 | Name: FUN_0040a102


undefined4 __thiscall FUN_0040a102(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if (param_2 == 0) {
    uVar1 = 0;
  }
  else if (*(int *)(param_1 + 0x1e4) == 0) {
    *(undefined4 *)(param_1 + 0x1e4) = 2;
    *(int *)(param_1 + 0x1e8) = param_2;
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

