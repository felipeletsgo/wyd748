// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040656c | Name: FUN_0040656c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_0040656c(int *param_1,int param_2,uint param_3,undefined4 param_4,undefined4 param_5)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  if (param_1[0xf] != 0) {
    uVar1 = __ftol();
    uVar2 = __ftol(uVar1);
    iVar3 = FUN_004011eb(uVar2,uVar1);
    uVar1 = __ftol();
    uVar2 = __ftol(uVar1);
    iVar4 = FUN_004011eb(uVar2,uVar1);
    iVar5 = FUN_00401000(param_4,param_5,
                         ((float)param_1[0x13] + (float)param_1[0x15]) - _DAT_005a3970,
                         (float)param_1[0x14] + (float)param_1[0x20],0x41700000,
                         (float)param_1[0x16] - _DAT_005a365c * (float)param_1[0x20]);
    if ((iVar3 == 0) || (param_2 != 0x202)) {
      if (DAT_005b892c != 2) {
        iVar3 = (**(code **)(*(int *)param_1[0x22] + 0x84))();
        *(undefined4 *)(iVar3 + 0x40) = 0;
      }
    }
    else {
      (**(code **)(*param_1 + 0x90))();
    }
    if ((iVar4 == 0) || (param_2 != 0x202)) {
      if (DAT_005b892c != 2) {
        iVar3 = (**(code **)(*(int *)param_1[0x23] + 0x84))();
        *(undefined4 *)(iVar3 + 0x40) = 0;
      }
    }
    else {
      (**(code **)(*param_1 + 0x94))();
    }
    if (iVar5 == 0) {
      if (DAT_005b892c != 2) {
        iVar3 = (**(code **)(*(int *)param_1[0x24] + 0x84))();
        *(undefined4 *)(iVar3 + 0x40) = 0;
      }
    }
    else {
      if (DAT_005b892c != 2) {
        iVar3 = (**(code **)(*(int *)param_1[0x24] + 0x84))();
        *(undefined4 *)(iVar3 + 0x40) = 1;
      }
      if ((param_2 == 0x200) && ((param_3 & 1) != 0)) {
        if (param_1[0x1a] == 0) {
          iVar3 = __ftol();
          param_1[0x1d] = iVar3;
        }
        else {
          iVar3 = __ftol();
          param_1[0x1d] = iVar3;
        }
        iVar3 = __ftol(param_1[0x1c] * param_1[0x1d]);
        param_1[0x1b] = iVar3;
        if ((0 < param_1[0x1b]) && (param_1[0x1b] < param_1[0x1c])) {
          (**(code **)(*param_1 + 0x54))();
        }
      }
    }
  }
  return 0;
}

