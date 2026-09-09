// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042e8a1 | Name: FUN_0042e8a1


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0042e8a1(int *param_1,undefined4 *param_2)

{
  int iVar1;
  int *piVar2;
  undefined2 extraout_var;
  undefined4 auStackY_ba0 [698];
  undefined4 uStackY_b8;
  undefined4 uStackY_b4;
  undefined4 uStackY_b0;
  int iStackY_ac;
  undefined1 *puStackY_a8;
  undefined4 uStackY_a4;
  undefined4 uStackY_a0;
  undefined4 uStackY_9c;
  undefined4 uStackY_98;
  float fStackY_94;
  float fStackY_90;
  undefined4 uStackY_8c;
  undefined4 *puStackY_88;
  undefined4 uStackY_84;
  int iStackY_80;
  int iStackY_7c;
  undefined4 uStackY_78;
  undefined4 uStackY_74;
  int *piStackY_70;
  undefined4 local_54;
  float local_4c;
  int local_44;
  undefined1 local_40 [16];
  uint local_30;
  int local_28;
  int *local_24;
  float local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  size_t local_c;
  int local_8;
  
  if (param_2 == (undefined4 *)0x0) {
    return;
  }
  if (param_2[0xd] != 1) {
    return;
  }
  if (DAT_005ccf9c[0xa8e5] == 1) {
    if (DAT_005ccf9c[0xa944] == 0x20) {
      (**(code **)(*DAT_005ccf9c + 0x30))();
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x30))();
    }
  }
  else if (DAT_005ccf9c[0xa944] == 0x20) {
    (**(code **)(*DAT_005ccf9c + 0x30))();
  }
  else {
    (**(code **)(*DAT_005ccf9c + 0x30))();
  }
  switch(*param_2) {
  case 0:
  case 1:
    if (param_2[0x53] != 0) {
      local_8 = 0;
      if (param_1[0xa941] == 0x280) {
        local_8 = -1;
      }
      piStackY_70 = (int *)0x42e9a5;
      iVar1 = __ftol();
      piStackY_70 = (int *)(iVar1 + local_8);
      uStackY_74 = 0x42e9b4;
      uStackY_74 = __ftol();
      uStackY_78 = 0x42e9c3;
      FUN_0042261f();
    }
    break;
  case 2:
  case 3:
  case 4:
  case 6:
    piStackY_70 = (int *)0x42e9d4;
    FUN_0042f2e7();
    if (param_2 != (undefined4 *)0xffffffba) {
      piStackY_70 = (int *)0x42e9ee;
      local_c = _strlen((char *)((int)param_2 + 0x46));
      if (((0 < (int)local_c) && ((int)local_c < 0x40)) && (param_2[0x53] != 0)) {
        local_10 = 0;
        if (param_1[0xa941] == 0x280) {
          local_10 = -1;
        }
        piStackY_70 = (int *)0x42ea54;
        iVar1 = __ftol();
        piStackY_70 = (int *)(iVar1 + local_10);
        uStackY_74 = 0x42ea7b;
        uStackY_74 = __ftol();
        uStackY_78 = 0x42ea8a;
        FUN_0042261f();
      }
    }
    break;
  case 5:
    if (((int)param_2[0xe] < 0x2e1) || (0x2e3 < (int)param_2[0xe])) {
      piStackY_70 = (int *)0x42ecd8;
      (**(code **)(*param_1 + 0x40))();
      piStackY_70 = (int *)0x8;
      uStackY_74 = 0x17;
      uStackY_78 = 0x42ecec;
      (**(code **)(*DAT_005ccf9c + 0x30))();
      uStackY_78 = 0;
      iStackY_7c = 0x1c;
      iStackY_80 = 0x42ecfb;
      (**(code **)(*param_1 + 0x30))();
      iStackY_80 = 180000;
      uStackY_84 = 0;
      puStackY_88 = (undefined4 *)param_2[0xe];
      uStackY_8c = 0x42ed14;
      piVar2 = (int *)FUN_004b14fd();
      local_4c = 1.0;
      if (DAT_005b892c == 2) {
        local_4c = _DAT_005a3660 / DAT_005b490c;
      }
      if (param_2[0x57] == -1) {
        local_4c = local_4c * _DAT_005a415c;
      }
      if (((0x3a8 < (int)param_2[0xe]) && ((int)param_2[0xe] < 0x3b3)) ||
         ((299 < (int)param_2[0xe] && ((int)param_2[0xe] < 0x130)))) {
        local_4c = 0.5;
      }
      if (piVar2 != (int *)0x0) {
        iStackY_80 = CONCAT22((short)((uint)param_2 >> 0x10),*(undefined2 *)(param_2 + 0x11));
        uStackY_84 = param_2[0xf];
        puStackY_88 = (undefined4 *)param_2[0x10];
        uStackY_8c = param_2[0xc];
        fStackY_90 = local_4c * (float)param_2[7];
        fStackY_94 = (float)param_2[6];
        uStackY_98 = 0x42edc7;
        uStackY_98 = __ftol();
        uStackY_9c = 0x42edd3;
        uStackY_9c = __ftol();
        uStackY_a0 = 0x42eddf;
        (**(code **)(*piVar2 + 0x10))();
      }
      if (param_2[0x57] != 0) {
        local_54 = 0x7b;
        if (param_2[0x57] == -0x56) {
          local_54 = 0x38;
        }
        if (param_2[0x57] == -1) {
          local_54 = 0x111;
        }
        if (param_2[0x57] == -0x77ff45) {
          local_54 = 0x7a;
        }
        if (param_2[0x57] == -0xbbbb78) {
          local_54 = 0x7a;
        }
        iStackY_80 = local_54;
        uStackY_84 = param_2[0x57];
        puStackY_88 = param_2;
        uStackY_8c = 0x42ee5e;
        FUN_0042eeea();
      }
      iStackY_80 = param_1[0xac0e];
      uStackY_84 = 0x1c;
      puStackY_88 = (undefined4 *)0x42ee75;
      (**(code **)(*param_1 + 0x30))();
      puStackY_88 = (undefined4 *)0x4;
      uStackY_8c = 0x17;
      fStackY_90 = 6.146713e-39;
      (**(code **)(*DAT_005ccf9c + 0x30))();
    }
    else {
      local_1c = 0xffff4400;
      local_18 = 0xff3366ff;
      local_14 = 0xff00ff00;
      piStackY_70 = (int *)0x17;
      uStackY_74 = 0x42ead9;
      (**(code **)(*DAT_005ccf9c + 0x30))();
      uStackY_74 = 0;
      uStackY_78 = 0x89;
      iStackY_7c = 0x42eaf0;
      (**(code **)(*DAT_005ccf9c + 0x30))();
      iStackY_7c = 2;
      iStackY_80 = 0x14;
      uStackY_84 = 0x42eb04;
      (**(code **)(*DAT_005ccf9c + 0x30))();
      uStackY_84 = 2;
      puStackY_88 = (undefined4 *)0x4;
      uStackY_8c = 0;
      fStackY_90 = 6.145481e-39;
      (**(code **)(*DAT_005ccf9c + 0x34))();
      fStackY_90 = 7.00649e-45;
      fStackY_94 = 2.66247e-44;
      uStackY_98 = 0x42eb2e;
      (**(code **)(*DAT_005ccf9c + 0x30))();
      uStackY_98 = 1;
      uStackY_9c = 0x1b;
      uStackY_a0 = 0x42eb42;
      (**(code **)(*DAT_005ccf9c + 0x30))();
      uStackY_a0 = 0;
      uStackY_a4 = 0x1c;
      puStackY_a8 = (undefined1 *)0x42eb51;
      (**(code **)(*param_1 + 0x30))();
      local_20 = 1.0;
      if (DAT_005b892c == 2) {
        local_20 = _DAT_005a3660 / DAT_005b490c;
      }
      puStackY_a8 = (undefined1 *)0x2bf20;
      iStackY_ac = 1;
      uStackY_b0 = param_2[0xe];
      uStackY_b4 = 0x42eb89;
      local_24 = (int *)FUN_004b14fd();
      if (local_24 != (int *)0x0) {
        puStackY_a8 = local_40;
        iStackY_ac = local_24[0xc6];
        uStackY_b0 = 0x42ebb2;
        iVar1 = (**(code **)(*(int *)local_24[0xc6] + 0x34))();
        if (iVar1 < 0) break;
        piStackY_70 = &local_28;
        uStackY_74 = 0;
        uStackY_78 = 0;
        iStackY_7c = local_24[0xc6];
        iStackY_80 = 0x42ebdd;
        (**(code **)(*(int *)local_24[0xc6] + 0x2c))();
        for (local_44 = 0; local_44 < (int)(local_30 / 0x18); local_44 = local_44 + 1) {
          *(undefined4 *)(local_28 + 0xc + local_44 * 0x18) = auStackY_ba0[param_2[0xe]];
        }
        iStackY_80 = local_24[0xc6];
        uStackY_84 = 0x42ec3a;
        (**(code **)(*(int *)local_24[0xc6] + 0x30))();
        local_24[0xa6] = 0xcc;
        uStackY_84 = CONCAT22(extraout_var,*(undefined2 *)(param_2 + 0x11));
        puStackY_88 = (undefined4 *)param_2[0xf];
        uStackY_8c = param_2[0x10];
        fStackY_90 = (float)param_2[0xc];
        fStackY_94 = local_20 * (float)param_2[7];
        uStackY_98 = param_2[6];
        uStackY_9c = 0x42ec83;
        uStackY_9c = __ftol();
        uStackY_a0 = 0x42ec8f;
        uStackY_a0 = __ftol();
        uStackY_a4 = 0x42ec9b;
        (**(code **)(*local_24 + 0x10))();
      }
      puStackY_a8 = (undefined1 *)param_1[0xac0e];
      iStackY_ac = 0x1c;
      uStackY_b0 = 0x42ecb2;
      (**(code **)(*param_1 + 0x30))();
      uStackY_b0 = 4;
      uStackY_b4 = 0x17;
      uStackY_b8 = 0x42ecc6;
      (**(code **)(*DAT_005ccf9c + 0x30))();
    }
  }
  if (DAT_005ccf9c[0xa944] == 0x20) {
    piStackY_70 = (int *)0x18;
    uStackY_74 = 0x42eeaf;
    (**(code **)(*DAT_005ccf9c + 0x30))();
  }
  else {
    piStackY_70 = (int *)0x18;
    uStackY_74 = 0x42eec8;
    (**(code **)(*DAT_005ccf9c + 0x30))();
  }
  return;
}

