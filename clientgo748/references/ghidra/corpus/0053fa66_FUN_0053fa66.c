// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053fa66 | Name: FUN_0053fa66


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0053fa66(int *param_1)

{
  bool bVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  float local_70;
  float local_6c;
  float local_68;
  undefined4 local_64;
  int local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  int local_50;
  float local_4c;
  float local_48;
  float local_44;
  int local_40;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  float local_1c;
  float local_18;
  float local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  if (DAT_005d03f0 == 1) {
    return 0;
  }
  iVar2 = (**(code **)(*param_1 + 0x70))();
  if (iVar2 == 0) {
    if (param_1[9] != 0x61a) {
      return 0;
    }
    if (*(int *)(DAT_0067cf38 + 0x24) != 0x7531) {
      return 0;
    }
  }
  if (param_1[9] == -1) {
    return 1;
  }
  if (param_1[0x13] == 1) {
    return 1;
  }
  iVar2 = (**(code **)(*param_1 + 0x4c))();
  if (iVar2 == 0) {
    return 1;
  }
  local_60 = DAT_0067cf38;
  _memset(&local_4c,0,0x44);
  if (((DAT_005ccfa0 == 0) || (DAT_005ccfa0 == 3)) || (DAT_005ccfa0 == 4)) {
    local_5c = 0x3f333333;
    local_58 = 0x3f333333;
    local_54 = 0x3f333333;
    local_50 = 0x3f800000;
    local_1c = 0.3;
    local_18 = 0.3;
    local_14 = 0.3;
    local_4c = 0.7;
    local_48 = 0.7;
    local_44 = 0.7;
    local_2c = 0x3f333333;
    local_28 = 0x3f333333;
    local_24 = 0x3f333333;
    local_c = 0;
  }
  else {
    FUN_0049cb58(&local_70,param_1[10],param_1[0xb]);
    local_70 = (float)DAT_005ccf9c[0xa9b9] * _DAT_005a34a0 + local_70 * _DAT_005a34a0;
    local_6c = (float)DAT_005ccf9c[0xa9ba] * _DAT_005a34a0 + local_6c * _DAT_005a34a0;
    local_68 = (float)DAT_005ccf9c[0xa9bb] * _DAT_005a34a0 + local_68 * _DAT_005a34a0;
    local_64 = 0x3f800000;
    local_2c = 0x3f800000;
    local_28 = 0x3f800000;
    local_24 = 0x3f800000;
    local_1c = local_70 * _DAT_005a4154 + (float)DAT_005ccf9c[0xa9bd] * _DAT_005a40ec;
    local_18 = local_70 * _DAT_005a4154 + (float)DAT_005ccf9c[0xa9be] * _DAT_005a40ec;
    local_14 = local_70 * _DAT_005a4154 + (float)DAT_005ccf9c[0xa9bf] * _DAT_005a40ec;
    local_4c = local_70;
    local_48 = local_6c;
    local_44 = local_68;
  }
  local_10 = 0x3f800000;
  local_20 = 0x3f800000;
  local_40 = 0x3f800000;
  (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,DAT_005ccf9c[0xac0e]);
  local_8 = 0;
  if (*(int *)(DAT_0067cf38 + 0x24) != 30000) goto LAB_0053fe7c;
  if (param_1[0x57] == 1) {
LAB_0053fe49:
    bVar1 = true;
  }
  else {
    iVar2 = __ftol();
    if ((iVar2 >> 7 == 0x1f) && (iVar2 = __ftol(), iVar2 >> 7 == 0x1f)) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if (bVar1) goto LAB_0053fe49;
    iVar2 = __ftol();
    if (((iVar2 >> 7 < 0x11) || (iVar2 = __ftol(), 0x13 < iVar2 >> 7)) ||
       (iVar2 = __ftol(), iVar2 >> 7 < 0x1e)) {
      bVar1 = false;
    }
    else {
      bVar1 = true;
    }
    if (bVar1) {
      bVar1 = true;
    }
    else {
      iVar2 = __ftol();
      if (((iVar2 >> 7 == 0xd) || (iVar2 = __ftol(), iVar2 >> 7 == 0xe)) &&
         (iVar2 = __ftol(), iVar2 >> 7 == 0x1c)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
    }
  }
  if ((bVar1) && (local_8 = (**(code **)(*param_1 + 0x74))(), local_8 != 0)) {
    param_1[0x56] = local_8;
  }
LAB_0053fe7c:
  if (param_1[0x56] == 0) {
    param_1[0x55] = 0x3f800000;
  }
  else {
    if (local_8 == 0) {
      if (_DAT_005a5a90 <= (float)param_1[0x55]) {
        param_1[0x55] = 0x3f800000;
        param_1[0x56] = 0;
      }
      else {
        param_1[0x55] = (int)((float)param_1[0x55] + _DAT_005a439c);
      }
    }
    else if ((float)param_1[0x55] <= _DAT_005a45e0) {
      param_1[0x55] = 0x3f333333;
    }
    else {
      param_1[0x55] = (int)((float)param_1[0x55] - _DAT_005a41e8);
    }
    local_54 = 0x3f333333;
    local_58 = 0x3f333333;
    local_5c = 0x3f333333;
    local_50 = param_1[0x55];
    local_4c = 0.7;
    local_48 = 0.7;
    local_44 = 0.7;
    local_40 = local_50;
    if (DAT_005ccf9c[0xa944] == 0x20) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xaa);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff);
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x34))(0,4,4);
    (**(code **)(*DAT_005ccf9c + 0x34))(0,5,2);
    (**(code **)(*DAT_005ccf9c + 0x34))(0,6,0);
    if ((local_8 == 2) && (param_1[0x57] == 0)) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    }
  }
  if (DAT_005ccf9c[0xa8e4] == 1) {
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_4c);
    iVar2 = (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
    if (((iVar2 == 0) || (*(float *)(iVar2 + 0x28) <= (float)param_1[10])) ||
       ((*(float *)(iVar2 + 0x2c) <= (float)param_1[0xb] ||
        ((0xfa < (uint)param_1[9] && ((uint)param_1[9] < 0xff)))))) {
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,9);
    }
    (**(code **)(*DAT_005ccf9c + 0x38))(1,0);
    (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    piVar3 = (int *)FUN_004b14fd(param_1[9],0,180000);
    if (piVar3 == (int *)0x0) {
      return 0;
    }
    (**(code **)(*piVar3 + 8))(param_1[10],param_1[0xc],param_1[0xb],param_1[0xd],0,0,0,0);
    (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
  }
  else {
    piVar3 = (int *)FUN_004b14fd(param_1[9],0,180000);
    if (piVar3 == (int *)0x0) {
      return 0;
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
    if (param_1[0x56] == 0) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,0);
    }
    if ((*(char *)(DAT_0092e640 + 0x43427 + piVar3[0xa6] * 0x108) == 'N') &&
       (((uint)param_1[9] < 0x9c || (0xb9 < (uint)param_1[9])))) {
      (**(code **)(*DAT_005ccf9c + 0x38))(1,0);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    }
    else {
      if (DAT_005ccf9c[0xa944] == 0x20) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xaa);
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x18,0xff);
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
    }
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_4c);
    if (((param_1[9] == 0x78e) || (param_1[9] == 0x7b8)) || (param_1[9] == 0x7b9)) {
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xb);
      uVar4 = FUN_004b925c(*(undefined4 *)(*(int *)(DAT_0067cf38 + 0x44) + 0x160),360000);
      (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar4);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0x18,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,0x30000);
    }
    (**(code **)(*piVar3 + 8))(param_1[10],param_1[0xc],param_1[0xb],param_1[0xd],0,0,0,0);
    if (((param_1[9] == 0x78e) || (param_1[9] == 0x7b8)) || (param_1[9] == 0x7b9)) {
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0x18,0);
    }
    if (param_1[0x56] != 0) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,4,2);
      if (local_8 == 2) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,2);
      }
    }
    (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
  }
  return 1;
}

