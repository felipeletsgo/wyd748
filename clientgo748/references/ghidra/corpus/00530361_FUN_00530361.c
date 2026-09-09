// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00530361 | Name: FUN_00530361


void __thiscall FUN_00530361(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  
  FUN_0053e47d(param_2,param_3,param_4);
  *(undefined4 *)(param_1 + 0x1a4) = param_2;
  *(undefined4 *)(param_1 + 0x1a8) = param_3;
  *(undefined4 *)(param_1 + 0x1ac) = param_4;
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x1b0) = uVar1;
  return;
}

