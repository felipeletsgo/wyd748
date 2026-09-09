// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00452691 | Name: FUN_00452691


undefined4 __thiscall FUN_00452691(int *param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'z') || (param_2 == 'Z')) {
    if (*(int *)(param_1[0xa1c6] + 0x28) == 0) {
      (**(code **)(*param_1 + 0x58))(0x24b,0);
    }
    else {
      (**(code **)(*param_1 + 0x58))(0x24c,0);
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

