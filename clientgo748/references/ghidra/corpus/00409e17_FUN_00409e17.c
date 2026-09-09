// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00409e17 | Name: FUN_00409e17


void __thiscall
FUN_00409e17(int *param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  if (param_2 == 0x200) {
    (**(code **)(*param_1 + 0x88))(param_4,param_5);
  }
  FUN_0040bf90(param_2,param_3,param_4,param_5);
  return;
}

