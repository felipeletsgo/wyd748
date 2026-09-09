// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0045288b | Name: FUN_0045288b


undefined4 __thiscall FUN_0045288b(int *param_1,char param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == '\'') || (param_2 == '\"')) {
    (**(code **)(*param_1 + 0x58))(299,0);
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

