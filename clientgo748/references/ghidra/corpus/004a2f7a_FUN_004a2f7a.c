// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a2f7a | Name: FUN_004a2f7a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004a2f7a(int *param_1,char param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  (**(code **)(*(int *)param_1[10] + 0x48))(0x1212);
  if (param_2 == '\r') {
    iVar1 = FUN_00493c10();
    if (iVar1 == 1) {
      (**(code **)(*param_1 + 0x58))(0x609,0);
    }
  }
  else if (param_2 == '\x1b') {
    if (DAT_005b892c == 2) {
      iVar1 = FUN_0040c0f0();
      if (iVar1 == 1) {
        (**(code **)(*param_1 + 0x58))(0x1208,0);
        return 1;
      }
      iVar1 = FUN_0040c0f0();
      if (iVar1 == 1) {
        (**(code **)(*param_1 + 0x58))(0x610,0);
        return 1;
      }
      (**(code **)(*param_1 + 0x58))(0x1629,0);
      return 1;
    }
    iVar1 = FUN_0040c0f0();
    if (iVar1 == 1) {
      (**(code **)(*(int *)param_1[0x24] + 0x60))(0);
      return 1;
    }
    iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x606);
    iVar2 = FUN_0040c0f0();
    if (iVar2 == 1) {
      if (_DAT_005a43cc <= *(float *)(iVar1 + 0x50) / DAT_005b4910) {
        if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x33), iVar1 != 0)) {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(0x33);
          FUN_0042ad2b(uVar3,uVar4);
        }
        (**(code **)(*param_1 + 0x68))(1);
      }
      else {
        if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x39), iVar1 != 0)) {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(0x39);
          FUN_0042ad2b(uVar3,uVar4);
        }
        iVar1 = (**(code **)(*DAT_0092e654 + 8))();
        param_1[0x9ba0] = iVar1;
        FUN_004a24b4(s_UI_CreCamAction3_005b93d4);
      }
    }
    else {
      iVar1 = (**(code **)(*(int *)param_1[10] + 0x48))(0x502);
      if (_DAT_005a43cc <= *(float *)(iVar1 + 0x50) / DAT_005b4910) {
        (**(code **)(*(int *)param_1[0x23] + 0x8c))(&DAT_00a346f8,0x130,0);
        (**(code **)(*(int *)param_1[0x23] + 0x60))(1);
      }
      else {
        if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x39), iVar1 != 0)) {
          uVar4 = 0;
          uVar3 = 0;
          FUN_00429a6d(0x39);
          FUN_0042ad2b(uVar3,uVar4);
        }
        iVar1 = (**(code **)(*DAT_0092e654 + 8))();
        param_1[0x9b9e] = iVar1;
        FUN_004a24b4(s_UI_SelCamAction3_005b93e8);
      }
    }
  }
  uVar3 = FUN_0049ac84(param_2,param_3);
  return uVar3;
}

