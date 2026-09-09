// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00422930 | Name: FUN_00422930


undefined4 * __thiscall
FUN_00422930(undefined4 *param_1,char *param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,int param_7,undefined2 param_8,undefined4 param_9,
            undefined4 param_10,undefined4 param_11)

{
  int iVar1;
  size_t sVar2;
  char local_2c;
  int local_28;
  char local_24;
  int local_20;
  int local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059efb8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  FUN_00430f10();
  FUN_00430f10();
  FUN_004219a0();
  local_8 = CONCAT31(local_8._1_3_,1);
  *param_1 = &PTR_FUN_005a4108;
  param_1[0x35] = 0x3f333333;
  param_1[0x34] = 0;
  param_1[9] = 0;
  param_1[0x2c] = 0xffffffff;
  param_1[0x2b] = param_11;
  param_1[0xa4] = param_10;
  param_1[0x2e] = param_3;
  param_1[0x2f] = param_4;
  FUN_0040bd30(param_1 + 0x2e);
  *(undefined2 *)(param_1 + 0x2d) = param_8;
  param_1[0x33] = param_9;
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x32] = iVar1 + param_7;
  FUN_00421b8f(param_2,param_5,0);
  param_1[0x3a] = param_6;
  param_1[0xa2] = param_7;
  param_1[0xa3] = 0;
  if (param_1[0x2b] == 1) {
    param_1[0x35] = 0x3f19999a;
    param_1[0x2c] = 0x89;
  }
  else if (param_1[0x2b] == 2) {
    param_1[0x2c] = 0x8a;
  }
  else if (param_1[0x2b] == 3) {
    param_1[0x2c] = 0x8b;
  }
  else if (param_1[0x2b] == 4) {
    param_1[0x2c] = 0x8c;
  }
  else if (param_1[0x2b] == 5) {
    param_1[0x2c] = 0x8d;
    param_1[0x35] = 0x3f8ccccd;
  }
  else if (param_1[0x2b] == 6) {
    param_1[0x2c] = 0x8d;
    param_1[0x35] = 0x3f4ccccd;
  }
  iVar1 = FUN_004b8c45(param_1[0x2c]);
  if ((0 < (int)param_1[0x2b]) && (iVar1 != 0)) {
    _memset(param_1 + 10,0,0x84);
    for (local_18 = 0; local_18 < 0xb; local_18 = local_18 + 1) {
      param_1[local_18 * 3 + 0xc] = 0xffffffff;
    }
    sVar2 = _strlen(param_2);
    param_1[9] = sVar2;
    if (0xb < (int)param_1[9]) {
      param_1[9] = 0xb;
    }
    local_1c = 0;
    if (*param_2 == 'm') {
      param_1[9] = 1;
      param_1[10] = 0;
      param_1[0xb] = 0;
      param_1[0xc] = 0xb;
    }
    else if (*param_2 == '?') {
      param_1[9] = 1;
      param_1[10] = 0;
      param_1[0xb] = 0;
      switch(param_1[0x2c]) {
      case 0x89:
        param_1[0xc] = 0xe;
        break;
      case 0x8a:
        param_1[0xc] = 0xb;
        break;
      case 0x8b:
        param_1[0xc] = 0xb;
        break;
      case 0x8c:
        param_1[0xc] = 0xc;
        break;
      case 0x8d:
        param_1[0xc] = 0xb;
      }
    }
    else if ((*param_2 == 'E') || (*param_2 == 'C')) {
      param_1[10] = 0;
      param_1[0xb] = 0;
      if (*param_2 == 'E') {
        param_1[9] = param_1[9] + -2;
        param_1[0xc] = 0xb;
      }
      else {
        param_1[9] = param_1[9] + -1;
        param_1[0xc] = 0xf;
      }
      local_1c = __ftol();
      for (local_20 = 1; local_20 < (int)param_1[9]; local_20 = local_20 + 1) {
        param_1[local_20 * 3 + 10] = local_1c;
        param_1[local_20 * 3 + 0xb] = 0;
        local_24 = param_2[local_20 + 2];
        if (*param_2 == 'C') {
          local_24 = param_2[local_20 + 1];
        }
        if ((local_24 < '0') || ('9' < local_24)) {
          if (local_24 == '+') {
            local_24 = '\n';
            goto LAB_00422e32;
          }
          if (local_24 == '-') {
            local_24 = '\f';
            goto LAB_00422e32;
          }
          if (local_24 == ':') {
            local_24 = '\r';
            goto LAB_00422e32;
          }
          param_1[local_20 * 3 + 0xc] = 0xffffffff;
          iVar1 = __ftol();
        }
        else {
          local_24 = local_24 + -0x30;
LAB_00422e32:
          param_1[local_20 * 3 + 0xc] = (int)local_24;
          iVar1 = __ftol();
        }
        local_1c = local_1c + iVar1;
      }
    }
    else {
      for (local_28 = 0; local_28 < (int)param_1[9]; local_28 = local_28 + 1) {
        param_1[local_28 * 3 + 10] = local_1c;
        param_1[local_28 * 3 + 0xb] = 0;
        local_2c = param_2[local_28];
        if ((local_2c < '0') || ('9' < local_2c)) {
          if (local_2c == '+') {
            local_2c = '\n';
            goto LAB_00422f48;
          }
          if (local_2c == '-') {
            local_2c = '\f';
            goto LAB_00422f48;
          }
          if (local_2c == ':') {
            local_2c = '\r';
            goto LAB_00422f48;
          }
          param_1[local_28 * 3 + 0xc] = 0xffffffff;
          iVar1 = __ftol();
        }
        else {
          local_2c = local_2c + -0x30;
LAB_00422f48:
          param_1[local_28 * 3 + 0xc] = (int)local_2c;
          iVar1 = __ftol();
        }
        local_1c = local_1c + iVar1;
      }
    }
  }
  param_1[0xa5] = (uint)param_1[0x33] / 700;
  ExceptionList = local_10;
  return param_1;
}

