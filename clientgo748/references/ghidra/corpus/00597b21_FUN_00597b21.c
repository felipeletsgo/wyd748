// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00597b21 | Name: FUN_00597b21


uint FUN_00597b21(undefined4 *param_1)

{
  int *piVar1;
  byte bVar2;
  uint uVar3;
  
  piVar1 = param_1 + 1;
  *piVar1 = *piVar1 + -1;
  if (-1 < *piVar1) {
    bVar2 = *(byte *)*param_1;
    *param_1 = (byte *)*param_1 + 1;
    return (uint)bVar2;
  }
  uVar3 = FUN_00597b76(param_1);
  return uVar3;
}

