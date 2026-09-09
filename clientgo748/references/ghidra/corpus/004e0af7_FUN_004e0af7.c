// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e0af7 | Name: FUN_004e0af7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e0af7(int *param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  int iVar8;
  void *pvVar9;
  int local_50;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0dbc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar5 = (**(code **)(*DAT_0092e654 + 8))();
  iVar6 = (**(code **)(*param_1 + 0x38))();
  if (iVar6 == 0) {
    uVar7 = 0;
  }
  else {
    fVar3 = (float)(uint)(iVar5 - param_1[0x20]) / (float)param_1[0x21];
    if (DAT_0067cf38 == 0) {
      uVar7 = 0;
    }
    else {
      if (_DAT_005a3660 < fVar3) {
        (**(code **)(*DAT_013b71e8 + 100))(param_1);
      }
      else if (0xfa < (uint)(iVar5 - param_1[0x23])) {
        fVar1 = (float)param_1[0x1d];
        iVar6 = param_1[0x1e];
        fVar2 = (float)param_1[0x1f];
        iVar8 = _rand();
        fVar4 = ((float)(iVar8 % 3) - fVar3 * _DAT_005a3670) * _DAT_005a4158;
        iVar8 = _rand();
        fVar3 = ((float)(iVar8 % 3) - fVar3 * _DAT_005a3670) * _DAT_005a4158;
        pvVar9 = operator_new(0x90);
        local_8 = 0;
        if (pvVar9 == (void *)0x0) {
          local_50 = 0;
        }
        else {
          local_50 = FUN_004edd60(fVar4 + fVar1,iVar6,fVar3 + fVar2,1);
        }
        local_8 = 0xffffffff;
        if (local_50 != 0) {
          FUN_0054ac09(local_50);
        }
        param_1[0x23] = iVar5;
      }
      uVar7 = 1;
    }
  }
  ExceptionList = local_10;
  return uVar7;
}

