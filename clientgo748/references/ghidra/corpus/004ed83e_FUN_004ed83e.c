// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ed83e | Name: FUN_004ed83e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004ed83e(int *param_1)

{
  int iVar1;
  int iVar2;
  float fVar3;
  int iVar4;
  void *pvVar5;
  float10 fVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  float fVar12;
  undefined4 uVar13;
  int local_b0;
  int local_ac;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  int local_28;
  int local_24;
  int local_1c;
  uint local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a14ed;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar4 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  fVar3 = (float)(uint)(iVar4 - param_1[0x20]) / (float)param_1[0x21];
  if (_DAT_005a3660 < fVar3) {
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  else {
    if (param_1[0x25] != 0) {
      local_28 = 0xff;
      local_1c = __ftol();
      local_24 = 0;
      local_18 = __ftol();
      if (_DAT_005a4160 < fVar3) {
        local_28 = __ftol();
        local_28 = 0xff - local_28;
        iVar4 = __ftol();
        local_1c = local_1c - iVar4;
        local_24 = __ftol();
        local_24 = -local_24;
        iVar4 = __ftol();
        local_18 = local_18 - iVar4;
      }
      (**(code **)(*(int *)param_1[0x25] + 0x44))
                (local_28 << 0x18 | local_1c << 0x10 | local_24 << 8 | local_18);
    }
    if (((_DAT_005a34a0 < fVar3) && (param_1[0x26] == 0)) && (param_1[0x23] == 0)) {
      fVar6 = (float10)FUN_00423b00(param_1[0x22]);
      fVar12 = (float)-fVar6;
      uVar11 = 0;
      fVar6 = (float10)FUN_00493bf0(param_1[0x22],0,fVar12);
      FUN_00430f50((float)fVar6,uVar11,fVar12);
      iVar4 = param_1[0x1d];
      iVar1 = param_1[0x1e];
      iVar2 = param_1[0x1f];
      if (param_1[7] != 0) {
        uVar11 = FUN_00430f50(*(undefined4 *)(param_1[7] + 0x28),
                              *(float *)(param_1[7] + 0x30) + _DAT_005a365c,
                              *(undefined4 *)(param_1[7] + 0x2c));
        FUN_004310a0(uVar11);
      }
      pvVar5 = operator_new(0x90);
      local_8 = 0;
      if (pvVar5 == (void *)0x0) {
        local_ac = 0;
      }
      else {
        local_ac = FUN_004d4de0(iVar4,iVar1,iVar2,5,0x14,0x3f4ccccd,&DAT_00ff7777,0,0xe7,0x3fc00000,
                                1,local_40,local_3c,local_38,0x960);
      }
      local_8 = 0xffffffff;
      param_1[0x26] = local_ac;
      if (param_1[0x26] != 0) {
        FUN_0054ac09(param_1[0x26]);
      }
    }
    if (param_1[0x23] == 1) {
      iVar4 = param_1[0x1d];
      iVar1 = param_1[0x1e];
      iVar2 = param_1[0x1f];
      fVar6 = (float10)FUN_00423b00(fVar3 * _DAT_005a3658);
      fVar12 = (float)-fVar6;
      uVar11 = 0;
      fVar6 = (float10)FUN_00493bf0(fVar3 * _DAT_005a3658,0,fVar12);
      FUN_00430f50((float)fVar6,uVar11,fVar12);
      pvVar5 = operator_new(0x90);
      local_8 = 1;
      if (pvVar5 == (void *)0x0) {
        local_b0 = 0;
      }
      else {
        uVar13 = 700;
        uVar10 = 1;
        uVar9 = 0x3f8ccccd;
        uVar8 = 0xe7;
        uVar7 = 0;
        uVar11 = __ftol(0,0xe7,0x3f8ccccd,1,local_4c,local_48,local_44,700);
        local_b0 = FUN_004d4de0(iVar4,iVar1,iVar2,5,2,0x3e99999a,uVar11,uVar7,uVar8,uVar9,uVar10,
                                local_4c,local_48,local_44,uVar13);
      }
      local_8 = 0xffffffff;
      param_1[0x26] = local_b0;
      if (param_1[0x26] != 0) {
        FUN_0054ac09(param_1[0x26]);
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}

