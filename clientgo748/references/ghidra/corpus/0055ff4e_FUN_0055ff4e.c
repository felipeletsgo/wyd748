// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055ff4e | Name: FUN_0055ff4e


undefined4 __fastcall
FUN_0055ff4e(undefined4 param_1,uint param_2,int *param_3,uint *param_4,uint *param_5,uint *param_6,
            uint *param_7,int *param_8,int param_9,int param_10)

{
  int *piVar1;
  undefined4 uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  undefined1 local_e0 [60];
  uint local_a4;
  uint local_88;
  uint local_84;
  uint local_80;
  int local_78;
  int local_c;
  uint local_8;
  
  piVar1 = param_3;
  if (param_3 == (int *)0x0) {
    return 0x8876086c;
  }
  if (param_2 == 0xffffffff) {
    param_2 = 0;
  }
  if (param_2 != (param_2 & 0x20b)) {
    return 0x8876086c;
  }
  if ((((param_9 != 0) && (param_9 != 1)) && (param_9 != 2)) && (param_9 != 3)) {
    return 0x8876086c;
  }
  if (param_8 == (int *)0x0) {
    local_c = 0;
  }
  else {
    local_c = *param_8;
  }
  if (param_9 == 3) {
    if (local_c == 0) {
      local_c = 0x15;
    }
  }
  else {
    uVar2 = FUN_0055fab2(local_c);
    local_c = FUN_0055fbb9(param_3,param_2,param_10,uVar2);
    if (local_c == 0) {
      return 0x8876086a;
    }
  }
  if (param_4 == (uint *)0x0) {
    param_3 = (int *)0xffffffff;
  }
  else {
    param_3 = (int *)*param_4;
  }
  if (param_5 == (uint *)0x0) {
    uVar7 = 0xffffffff;
  }
  else {
    uVar7 = *param_5;
  }
  if (param_6 == (uint *)0x0) {
    local_8 = 0xffffffff;
  }
  else {
    local_8 = *param_6;
  }
  if (param_3 == (int *)0xffffffff) {
    param_3 = (int *)uVar7;
    if (uVar7 != 0xffffffff) goto LAB_00560039;
    uVar7 = 0x100;
    param_3 = (int *)0x100;
  }
  else {
    if (uVar7 == 0xffffffff) {
      uVar7 = (uint)param_3;
    }
LAB_00560039:
    if (param_3 == (int *)0x0) {
      param_3 = (int *)0x1;
    }
    if (uVar7 == 0) {
      uVar7 = 1;
    }
  }
  uVar5 = (uint)param_3;
  if ((param_10 == 5) && (uVar5 = uVar7, uVar7 < param_3)) {
    uVar7 = (uint)param_3;
    uVar5 = (uint)param_3;
  }
  param_3 = (int *)uVar5;
  if ((local_8 == 0xffffffff) || (local_8 == 0)) {
    local_8 = 1;
  }
  uVar5 = uVar7;
  if (param_9 == 3) goto LAB_0056014a;
  (**(code **)(*piVar1 + 0x1c))(piVar1,local_e0);
  if (param_10 == 4) {
    if (local_80 < local_8) {
      local_8 = local_80;
    }
    if (local_80 < param_3) {
      param_3 = (int *)local_80;
    }
LAB_005600e2:
    if (local_80 < uVar7) {
      uVar7 = local_80;
    }
  }
  else {
    if (local_88 < param_3) {
      param_3 = (int *)local_88;
    }
    if (local_84 < uVar7) {
      uVar7 = local_84;
    }
    if (param_10 == 3) {
      if ((local_a4 & 0x20) != 0) {
        local_78 = 1;
      }
      if (local_78 != 0) {
        if (local_78 * uVar7 < param_3) {
          param_3 = (int *)(local_78 * uVar7);
        }
        local_80 = local_78 * (int)param_3;
        goto LAB_005600e2;
      }
    }
  }
  uVar6 = local_8;
  uVar8 = (uint)param_3;
  if (param_10 == 3) {
    uVar3 = 2;
  }
  else if (param_10 == 4) {
    uVar3 = 0x40000;
  }
  else {
    uVar3 = (uint)param_3;
    if (param_10 == 5) {
      uVar3 = 0x20000;
    }
  }
  uVar5 = uVar7;
  if ((local_a4 & uVar3) != 0) {
    uVar5 = 1;
    param_3 = (int *)0x1;
    if (1 < uVar8) {
      do {
        param_3 = (int *)((int)param_3 << 1);
      } while (param_3 < uVar8);
    }
    if (1 < uVar7) {
      do {
        uVar5 = uVar5 << 1;
      } while (uVar5 < uVar7);
    }
    local_8 = 1;
    if (1 < uVar6) {
      do {
        local_8 = local_8 << 1;
      } while (local_8 < uVar6);
    }
  }
LAB_0056014a:
  if ((((local_c == 0x31545844) || (local_c == 0x32545844)) || (local_c == 0x33545844)) ||
     ((local_c == 0x34545844 || (local_c == 0x35545844)))) {
    param_3 = (int *)((int)param_3 + 3U & 0xfffffffc);
    uVar5 = uVar5 + 3 & 0xfffffffc;
  }
  if (param_10 == 3) {
    uVar7 = 0x4000;
  }
  else if (param_10 == 4) {
    uVar7 = 0x8000;
  }
  else {
    uVar7 = (uint)param_3;
    if (param_10 == 5) {
      uVar7 = 0x10000;
    }
  }
  if (param_7 == (uint *)0x0) {
    uVar8 = 0xffffffff;
  }
  else {
    uVar8 = *param_7;
  }
  if ((param_9 == 3) ||
     (((local_a4 & uVar7) != 0 &&
      (((local_a4 & 0x100) == 0 ||
       (((((uint)param_3 & (int)param_3 - 1U) == 0 && ((uVar5 & uVar5 - 1) == 0)) &&
        ((local_8 & local_8 - 1) == 0)))))))) {
    uVar7 = 0;
    for (uVar6 = (uint)param_3; uVar6 != 0; uVar6 = uVar6 >> 1) {
      uVar7 = uVar7 + 1;
    }
    uVar6 = 0;
    for (uVar3 = uVar5; uVar3 != 0; uVar3 = uVar3 >> 1) {
      uVar6 = uVar6 + 1;
    }
    uVar3 = 0;
    for (uVar4 = local_8; uVar4 != 0; uVar4 = uVar4 >> 1) {
      uVar3 = uVar3 + 1;
    }
    if (uVar7 <= uVar6) {
      uVar7 = uVar6;
    }
    if ((param_10 == 4) && (uVar7 < uVar3)) {
      uVar7 = uVar3;
    }
    if ((uVar7 < uVar8) || (uVar8 == 0)) {
      uVar8 = uVar7;
    }
  }
  else {
    uVar8 = 1;
  }
  if (param_4 != (uint *)0x0) {
    *param_4 = (uint)param_3;
  }
  if (param_5 != (uint *)0x0) {
    *param_5 = uVar5;
  }
  if (param_6 != (uint *)0x0) {
    *param_6 = local_8;
  }
  if (param_7 != (uint *)0x0) {
    *param_7 = uVar8;
  }
  if (param_8 != (int *)0x0) {
    *param_8 = local_c;
  }
  return 0;
}

