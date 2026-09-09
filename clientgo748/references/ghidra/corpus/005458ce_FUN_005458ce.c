// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005458ce | Name: FUN_005458ce


void __thiscall FUN_005458ce(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  
  FUN_0053e47d(param_2,param_3,param_4);
  uVar1 = __ftol();
  *(undefined4 *)(param_1 + 0x168) = uVar1;
  uVar1 = __ftol();
  *(undefined4 *)(param_1 + 0x170) = uVar1;
  uVar1 = __ftol();
  *(undefined4 *)(param_1 + 0x16c) = uVar1;
  uVar1 = __ftol();
  *(undefined4 *)(param_1 + 0x174) = uVar1;
  return;
}

