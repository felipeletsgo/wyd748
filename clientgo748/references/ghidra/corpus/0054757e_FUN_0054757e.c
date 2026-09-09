// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054757e | Name: FUN_0054757e


undefined4 __fastcall FUN_0054757e(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 local_8;
  
  if ((((param_1[6] == -1) || (param_1[6] == DAT_005bdef0)) || (param_1[6] == DAT_005bdef4)) ||
     ((param_1[6] == DAT_005bdef8 || (param_1[6] == DAT_005bdefc)))) {
    uVar2 = (**(code **)(*DAT_0092e654 + 8))();
    iVar3 = (**(code **)(*param_1 + 0x4c))();
    if (iVar3 == 1) {
      _memset((void *)(param_1[0x18] + 0x288),0,0x44);
      *(undefined4 *)(param_1[0x18] + 0x2b8) = 0x3e99999a;
      *(undefined4 *)(param_1[0x18] + 700) = 0x3e99999a;
      *(undefined4 *)(param_1[0x18] + 0x2c0) = 0x3e99999a;
      iVar3 = param_1[0x18];
      *(undefined4 *)(iVar3 + 0x288) = 0x3f333333;
      *(undefined4 *)(iVar3 + 0x28c) = 0x3f333333;
      *(undefined4 *)(iVar3 + 0x290) = 0x3f333333;
      *(undefined4 *)(iVar3 + 0x294) = local_8;
      iVar3 = param_1[0x18];
      iVar1 = param_1[0x18];
      *(undefined4 *)(iVar1 + 0x2a8) = *(undefined4 *)(iVar3 + 0x288);
      *(undefined4 *)(iVar1 + 0x2ac) = *(undefined4 *)(iVar3 + 0x28c);
      *(undefined4 *)(iVar1 + 0x2b0) = *(undefined4 *)(iVar3 + 0x290);
      *(undefined4 *)(iVar1 + 0x2b4) = *(undefined4 *)(iVar3 + 0x294);
      *(undefined4 *)(param_1[0x18] + 0x2c8) = 0;
      FUN_004bda22(uVar2);
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

