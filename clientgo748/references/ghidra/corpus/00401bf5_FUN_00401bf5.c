// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401bf5 | Name: FUN_00401bf5


undefined4 __thiscall
FUN_00401bf5(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x3c) == 0) {
    uVar1 = 0;
  }
  else if (param_2 == 0x200) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_0040bf90(param_2,param_3,param_4,param_5);
  }
  return uVar1;
}

