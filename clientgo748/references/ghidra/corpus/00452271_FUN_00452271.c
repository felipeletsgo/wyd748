// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00452271 | Name: FUN_00452271


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00452271(int param_1,char param_2)

{
  int *piVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 uVar5;
  
  if ((param_2 == '=') || (param_2 == '+')) {
    *(int *)(param_1 + 0x26ee8) = *(int *)(param_1 + 0x26ee8) + 1;
    uVar2 = *(uint *)(param_1 + 0x26ee8) & 0x80000003;
    if ((int)uVar2 < 0) {
      uVar2 = (uVar2 - 1 | 0xfffffffc) + 1;
    }
    *(uint *)(param_1 + 0x26ee8) = uVar2;
    if (*(int *)(param_1 + 0x27ae8) == 0) {
      uVar3 = 0;
    }
    else {
      piVar1 = *(int **)(param_1 + 0x27ae8);
      (**(code **)(*piVar1 + 0x74))(0x43960000,(float)(*(int *)(param_1 + 0x26ee8) * 0x8c + 0x70));
      iVar4 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x3150);
      if ((DAT_005b892c == 2) && (iVar4 != 0)) {
        (**(code **)(**(int **)(param_1 + 0x27ae8) + 0x70))
                  (*(undefined4 *)(*(int *)(param_1 + 0x27ae8) + 0x4c),
                   (*(float *)(iVar4 + 0x50) - (float)piVar1[0x16]) - _DAT_005a4104);
      }
      piVar1[0x7d] = *(int *)(param_1 + 0x26ee8) * 10 + 8;
      if (*(int *)(param_1 + 0x26ee8) == 3) {
        (**(code **)(*piVar1 + 0x60))(0);
      }
      else if (piVar1[10] == 0) {
        (**(code **)(*piVar1 + 0x60))(1);
      }
      if (DAT_005ccf98 != 0) {
        iVar4 = FUN_00429a6d(0x33);
        if (iVar4 != 0) {
          uVar5 = 0;
          uVar3 = 0;
          FUN_00429a6d(0x33);
          FUN_0042ad2b(uVar3,uVar5);
        }
      }
      uVar3 = 1;
    }
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}

