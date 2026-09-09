// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00567355 | Name: FUN_00567355


undefined4 * __thiscall FUN_00567355(undefined4 *param_1,byte param_2)

{
  undefined4 *puVar1;
  
  if ((param_2 & 2) == 0) {
    FUN_0058ea3a(*param_1);
    puVar1 = param_1;
    if ((param_2 & 1) != 0) {
      FUN_0058ea3a(param_1);
    }
  }
  else {
    puVar1 = param_1 + -1;
    FUN_005903c2(param_1,0xc,*puVar1,&LAB_0056708f);
    if ((param_2 & 1) != 0) {
      FUN_0058ea3a(puVar1);
    }
  }
  return puVar1;
}

