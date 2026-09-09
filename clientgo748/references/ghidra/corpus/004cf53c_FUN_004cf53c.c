// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cf53c | Name: FUN_004cf53c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004cf53c(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  float local_70;
  float local_6c;
  float local_68;
  undefined4 local_64;
  undefined4 local_60;
  int local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  float local_48;
  float local_44;
  float local_40;
  undefined4 local_3c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  float local_18;
  float local_14;
  float local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (DAT_0092e660 == 1) {
    uVar1 = 1;
  }
  else if ((((param_1[6] == -1) || (param_1[6] == DAT_005bdef0)) || (param_1[6] == DAT_005bdef4)) ||
          ((param_1[6] == DAT_005bdef8 || (param_1[6] == DAT_005bdefc)))) {
    if (param_1[0x20] == 1) {
      iVar2 = (**(code **)(*param_1 + 0x38))();
      if (iVar2 == 0) {
        return 1;
      }
      local_60 = DAT_0067cf38;
      _memset(&local_48,0,0x44);
      if (((DAT_005ccfa0 == 0) || (DAT_005ccfa0 == 3)) || (DAT_005ccfa0 == 4)) {
        local_58 = 0x3f333333;
        local_54 = 0x3f333333;
        local_50 = 0x3f333333;
        local_18 = 0.3;
        local_14 = 0.3;
        local_10 = 0.3;
        local_48 = 0.7;
        local_44 = 0.7;
        local_40 = 0.7;
        local_28 = 0x3f333333;
        local_24 = 0x3f333333;
        local_20 = 0x3f333333;
        local_1c = local_4c;
        local_8 = 0;
      }
      else {
        puVar3 = (undefined4 *)FUN_00430f20(param_1[0x1d],param_1[0x1f]);
        FUN_0049cb58(&local_70,*puVar3,puVar3[1]);
        local_70 = (float)DAT_005ccf9c[0xa9b9] * _DAT_005a34a0 + local_70 * _DAT_005a34a0;
        local_6c = (float)DAT_005ccf9c[0xa9ba] * _DAT_005a34a0 + local_6c * _DAT_005a34a0;
        local_68 = (float)DAT_005ccf9c[0xa9bb] * _DAT_005a34a0 + local_68 * _DAT_005a34a0;
        local_64 = 0x3f800000;
        local_1c = 0x3f800000;
        local_c = 0x3f800000;
        local_28 = 0x3f800000;
        local_24 = 0x3f800000;
        local_20 = 0x3f800000;
        local_18 = local_70 * _DAT_005a4154 + (float)DAT_005ccf9c[0xa9bd] * _DAT_005a40ec;
        local_14 = local_70 * _DAT_005a4154 + (float)DAT_005ccf9c[0xa9be] * _DAT_005a40ec;
        local_10 = local_70 * _DAT_005a4154 + (float)DAT_005ccf9c[0xa9bf] * _DAT_005a40ec;
        local_48 = local_70;
        local_44 = local_6c;
        local_40 = local_68;
      }
      local_3c = local_1c;
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,DAT_005ccf9c[0xac0e]);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,0);
      uVar1 = FUN_004b925c(6,360000);
      (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,4);
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_48);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
      for (local_5c = 0; local_5c < 4; local_5c = local_5c + 1) {
        if (param_1[local_5c + 0x23] != 0) {
          (**(code **)(*(int *)param_1[local_5c + 0x23] + 8))
                    (param_1[local_5c * 3 + 0x27],param_1[local_5c * 3 + 0x28],
                     param_1[local_5c * 3 + 0x29],0,0,0,0,0);
        }
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

