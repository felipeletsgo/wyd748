// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004526ee | Name: FUN_004526ee


undefined4 __thiscall FUN_004526ee(int *param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 'x') || (param_2 == 'X')) {
    (**(code **)(*param_1 + 0x58))(0x13b,0);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

