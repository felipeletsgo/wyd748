// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004819d5 | Name: FUN_004819d5


void __thiscall FUN_004819d5(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  
  DAT_005d03e4 = param_2;
  iVar2 = __ftol();
  if (iVar2 >> 7 == 0x1d) {
LAB_00481a12:
    iVar2 = __ftol();
    if (iVar2 >> 7 != 0x16) goto LAB_00481a31;
    bVar1 = true;
  }
  else {
    iVar2 = __ftol();
    if (iVar2 >> 7 == 0x1e) goto LAB_00481a12;
LAB_00481a31:
    bVar1 = false;
  }
  if (bVar1) {
    param_2 = 0;
    goto LAB_00481aff;
  }
  if (((param_2 == 2) || (param_2 == 3)) && ((DAT_005ccfa0 == 3 || (DAT_005ccfa0 == 4)))) {
    param_2 = 1;
    goto LAB_00481aff;
  }
  iVar2 = __ftol();
  if (iVar2 >> 7 < 0x1b) {
LAB_00481adc:
    bVar1 = false;
  }
  else {
    iVar2 = __ftol();
    if (0x1e < iVar2 >> 7) goto LAB_00481adc;
    iVar2 = __ftol();
    if (iVar2 >> 7 < 0x15) goto LAB_00481adc;
    iVar2 = __ftol();
    if (0x18 < iVar2 >> 7) goto LAB_00481adc;
    bVar1 = true;
  }
  if (bVar1) {
    if (param_2 == 1) {
      param_2 = 3;
    }
    else {
      param_2 = 2;
    }
  }
LAB_00481aff:
  if (param_2 == 1) {
    *(undefined4 *)(*(int *)(param_1 + 0x26e9c) + 0x28) = 1;
    *(undefined4 *)(*(int *)(param_1 + 0x26ea0) + 0x28) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x26ea4) + 0x28) = 0;
    FUN_00549bbf(0xb);
  }
  else if (param_2 == 2) {
    *(undefined4 *)(*(int *)(param_1 + 0x26e9c) + 0x28) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x26ea0) + 0x28) = 1;
    *(undefined4 *)(*(int *)(param_1 + 0x26ea4) + 0x28) = 0;
    FUN_00549bbf(0xb);
  }
  else if (param_2 == 3) {
    *(undefined4 *)(*(int *)(param_1 + 0x26e9c) + 0x28) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x26ea0) + 0x28) = 1;
    *(undefined4 *)(*(int *)(param_1 + 0x26ea4) + 0x28) = 1;
    FUN_00549bbf(0xb);
  }
  return;
}

