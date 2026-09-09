// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004efb89 | Name: FUN_004efb89


void __thiscall FUN_004efb89(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  *(undefined4 *)(param_1 + 0x6c) = param_2;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x68) = uVar1;
  return;
}

