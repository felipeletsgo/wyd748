// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004063a5 | Name: FUN_004063a5


void FUN_004063a5(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  int *extraout_var;
  int *piVar2;
  undefined4 uVar3;
  
  uVar3 = 0x3f800000;
  FUN_00401275(param_1,param_2);
  if (extraout_var[0x1a] == 0) {
    piVar2 = extraout_var;
    (**(code **)(*(int *)extraout_var[0x23] + 0x70))
              (*(undefined4 *)(extraout_var[0x23] + 0x4c),
               ((float)extraout_var[0x16] - (float)extraout_var[0x20]) / 1.0,extraout_var,uVar3);
    (**(code **)(*(int *)piVar2[0x21] + 0x74))(*(undefined4 *)(piVar2[0x21] + 0x54),param_2);
    iVar1 = __ftol();
    piVar2[0x1e] = iVar1;
    (**(code **)(*piVar2 + 0x54))();
  }
  return;
}

