// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b6a90 | Name: FUN_004b6a90


undefined4 __thiscall FUN_004b6a90(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  uVar1 = FUN_004b6430(param_2);
  uVar1 = FUN_004b74c0(param_3,uVar1);
  uVar2 = FUN_004b6430();
  FUN_004b6ae0(uVar1,uVar2);
  *(undefined4 *)(param_1 + 8) = uVar1;
  return param_2;
}

