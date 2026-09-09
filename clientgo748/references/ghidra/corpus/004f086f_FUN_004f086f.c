// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f086f | Name: FUN_004f086f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_004f086f(int *param_1,undefined4 *param_2)

{
  float fVar1;
  undefined4 uVar2;
  int iVar3;
  float10 fVar4;
  float10 fVar5;
  undefined1 *puVar6;
  undefined1 *puVar7;
  undefined1 local_80 [12];
  undefined1 local_74 [12];
  undefined1 local_68 [12];
  undefined1 local_5c [12];
  undefined1 local_50 [12];
  undefined1 local_44 [16];
  undefined4 local_34;
  undefined4 local_30;
  int local_2c;
  int local_28;
  float local_24;
  float local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  float local_8;
  
  CSimpleArray<>();
  uVar2 = (**(code **)(*param_1 + 0x40))(local_50);
  puVar7 = local_5c;
  puVar6 = local_44;
  (**(code **)(*param_1 + 0x38))(puVar6,puVar7,uVar2);
  uVar2 = FUN_00493d50(puVar6,puVar7);
  FUN_004310a0(uVar2);
  local_8 = -10000.0;
  FUN_00430f20(param_1[0x12],param_1[0x14]);
  if (DAT_0067cf38 != 0) {
    fVar4 = (float10)FUN_0049c80b(local_10,local_c);
    local_8 = (float)fVar4;
    if ((float10)(float)param_1[0x13] - (float10)_DAT_005a5368 < fVar4) {
      fVar1 = (local_8 - (float)param_1[0x13]) + _DAT_005a5368;
      fVar4 = (float10)FUN_00423b00(param_1[0xb]);
      local_20 = (float)(((float10)fVar1 / fVar4) * (float10)_DAT_005a3660);
      if (local_20 < _DAT_005a3534) {
        param_1[0xd] = (int)((float)param_1[0xd] - _DAT_005a34a0);
        param_1[0x2c] = (int)((float)param_1[0xd] - _DAT_005a34a0);
      }
      else {
        param_1[0xd] = (int)((float)param_1[0xd] - local_20);
        param_1[0x2c] = (int)((float)param_1[0x2c] - local_20);
      }
      puVar7 = local_74;
      uVar2 = (**(code **)(*param_1 + 0x40))(puVar7);
      (**(code **)(*param_1 + 0x38))(local_68,local_80,uVar2);
      uVar2 = FUN_00493d50(uVar2,puVar7);
      FUN_004310a0(uVar2);
      param_1[0x19] = 0;
    }
  }
  if ((float)param_1[0x2c] != _DAT_005a3534) {
    param_1[0xd] = (int)((float)param_1[0xd] - (float)param_1[0x2c]);
    param_1[0x2c] = 0;
  }
  if ((DAT_0067cf38 != 0) && (*(int *)(DAT_0067cf38 + 0x4c) != 0)) {
    fVar4 = (float10)FUN_0049c80b(local_10,local_c);
    local_24 = (float)fVar4;
    local_28 = __ftol();
    local_2c = __ftol();
    iVar3 = __ftol();
    local_34 = FUN_005910c4(local_28 - iVar3);
    iVar3 = __ftol();
    local_30 = FUN_005910c4(local_2c - iVar3);
    if (((0x19 < (int)(local_28 + (local_28 >> 0x1f & 0x7fU)) >> 7) &&
        ((((int)(local_28 + (local_28 >> 0x1f & 0x7fU)) >> 7 < 0x1f &&
          (7 < (int)(local_2c + (local_2c >> 0x1f & 0x7fU)) >> 7)) &&
         ((int)(local_2c + (local_2c >> 0x1f & 0x7fU)) >> 7 < 0xd)))) && (local_24 == -10000.0)) {
      fVar5 = (float10)FUN_00423b00((float)param_1[0xb] + _DAT_005a4158);
      fVar4 = (float10)_DAT_005a40f0;
      param_1[0xd] = (int)((float)param_1[0xd] - (float)(fVar5 * fVar4));
      param_1[0x2d] = (int)((float)param_1[0x2d] - (float)(fVar5 * fVar4));
    }
  }
  *param_2 = local_1c;
  param_2[1] = local_18;
  param_2[2] = local_14;
  return param_2;
}

