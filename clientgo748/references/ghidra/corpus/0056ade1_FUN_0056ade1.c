// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056ade1 | Name: FUN_0056ade1


/* WARNING: Type propagation algorithm not settling */

int __thiscall
FUN_0056ade1(int *param_1,undefined4 *param_2,int *param_3,undefined4 param_4,uint *param_5,
            undefined4 param_6,uint param_7)

{
  bool bVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint *puVar7;
  uint *puVar8;
  int local_6c [2];
  uint local_64;
  int local_60;
  uint local_58;
  uint local_54;
  uint local_50;
  uint local_4c [4];
  uint local_3c;
  uint local_38;
  uint local_34 [4];
  uint local_24;
  uint local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int *local_10;
  uint local_c;
  int *local_8;
  
  local_10 = param_1;
  if (*param_1 != 0) {
    FUN_0056a9ae();
  }
  (**(code **)(*param_3 + 0x20))(param_3,local_6c);
  if (param_5 == (uint *)0x0) {
    bVar1 = false;
    local_34[2] = local_58;
    local_34[3] = local_54;
    uVar4 = ~param_7 & 1;
    local_34[0] = 0;
    local_34[1] = 0;
    local_24 = 0;
    local_20 = local_50;
  }
  else {
    puVar7 = local_34;
    for (iVar5 = 6; iVar5 != 0; iVar5 = iVar5 + -1) {
      *puVar7 = *param_5;
      param_5 = param_5 + 1;
      puVar7 = puVar7 + 1;
    }
    if (local_58 < local_34[2]) {
      return -0x7789f794;
    }
    if (local_34[2] < local_34[0]) {
      return -0x7789f794;
    }
    if (local_54 < local_34[3]) {
      return -0x7789f794;
    }
    if (local_34[3] < local_34[1]) {
      return -0x7789f794;
    }
    if (local_50 < local_20) {
      return -0x7789f794;
    }
    if (local_20 < local_24) {
      return -0x7789f794;
    }
    if (((((local_34[0] != 0) || (local_34[2] != local_58)) || (local_34[1] != 0)) ||
        ((local_34[3] != local_54 || (local_24 != 0)))) || (bVar1 = false, local_20 != local_50)) {
      bVar1 = true;
    }
    if (((param_7 & 1) != 0) || (bVar1)) {
      uVar4 = 0;
    }
    else {
      uVar4 = 1;
    }
  }
  uVar2 = local_20;
  if ((local_60 == 0) && ((local_64 & 0x200) == 0)) {
    return -0x7789f794;
  }
  uVar6 = (param_7 & 1 | 0x80) << 4;
  local_c = uVar6;
  if ((uVar4 == 0) || ((local_64 & 0x200) == 0)) {
LAB_0056af49:
    if (bVar1) {
      if ((local_6c[0] == 0x31545844) || (local_6c[0] == 0x32545844)) {
LAB_0056b014:
        uVar4 = local_34[0] & 0xfffffffc;
        local_4c[2] = local_34[2] + 3 & 0xfffffffc;
        local_4c[3] = local_34[3] + 3 & 0xfffffffc;
        if (local_58 < local_4c[2]) {
          local_4c[2] = local_58;
        }
        uVar3 = local_34[1] & 0xfffffffc;
        uVar6 = local_c;
        if (local_54 < local_4c[3]) {
          local_4c[3] = local_54;
        }
joined_r0x0056b05e:
        local_3c = local_24;
        local_38 = uVar2;
        if (((uVar4 == 0) && (local_4c[2] == local_58)) &&
           ((uVar3 == 0 && ((local_4c[3] == local_54 && (local_24 == 0)))))) {
          local_4c[1] = 0;
          local_4c[0] = 0;
          if (uVar2 == local_50) goto LAB_0056b087;
        }
      }
      else {
        if (local_6c[0] == 0x32595559) {
LAB_0056afaa:
          uVar4 = local_34[0] & 0xfffffffe;
          local_4c[2] = local_34[2] + 1 & 0xfffffffe;
          uVar3 = local_34[1];
          local_4c[3] = local_34[3];
          if (local_58 < local_4c[2]) {
            local_4c[2] = local_58;
          }
          goto joined_r0x0056b05e;
        }
        if (((local_6c[0] == 0x33545844) || (local_6c[0] == 0x34545844)) ||
           (local_6c[0] == 0x35545844)) goto LAB_0056b014;
        if (local_6c[0] == 0x59565955) goto LAB_0056afaa;
        puVar7 = local_34;
        puVar8 = local_4c;
        for (iVar5 = 6; uVar6 = local_c, uVar4 = local_4c[0], uVar3 = local_4c[1], iVar5 != 0;
            iVar5 = iVar5 + -1) {
          *puVar8 = *puVar7;
          puVar7 = puVar7 + 1;
          puVar8 = puVar8 + 1;
        }
      }
      local_4c[1] = uVar3;
      local_4c[0] = uVar4;
      iVar5 = (**(code **)(*param_3 + 0x24))(param_3,&local_1c,local_4c,uVar6);
      if (iVar5 < 0) {
        return iVar5;
      }
      local_34[0] = local_34[0] - local_4c[0];
      local_34[2] = local_34[2] - local_4c[0];
      local_34[1] = local_34[1] - local_4c[1];
      local_34[3] = local_34[3] - local_4c[1];
      local_24 = local_24 - local_3c;
      local_20 = uVar2 - local_3c;
      goto LAB_0056b0d6;
    }
  }
  else {
    if ((param_7 & 0x20000) == 0) {
      FUN_00564a04(1);
      iVar5 = (**(code **)(*param_3 + 0x1c))(param_3,&DAT_005a6d20,&local_8);
      if (-1 < iVar5) {
        iVar5 = (**(code **)(*local_8 + 0x34))(local_8);
        if (iVar5 == 1) {
          uVar6 = uVar6 | 0x2000;
          bVar1 = false;
          local_c = uVar6;
        }
        if (local_8 != (int *)0x0) {
          (**(code **)(*local_8 + 8))(local_8);
          local_8 = (int *)0x0;
        }
      }
      FUN_00564a04(0);
      goto LAB_0056af49;
    }
    uVar6 = uVar6 | 0x2000;
  }
LAB_0056b087:
  iVar5 = (**(code **)(*param_3 + 0x24))(param_3,&local_1c,0,uVar6);
  if (iVar5 < 0) {
    return iVar5;
  }
LAB_0056b0d6:
  *param_2 = local_14;
  param_2[1] = local_6c[0];
  param_2[2] = local_1c;
  param_2[3] = local_18;
  param_2[4] = 0;
  param_2[5] = 0;
  param_2[8] = 0;
  param_2[9] = local_50;
  puVar7 = local_34;
  puVar8 = param_2 + 10;
  for (iVar5 = 6; iVar5 != 0; iVar5 = iVar5 + -1) {
    *puVar8 = *puVar7;
    puVar7 = puVar7 + 1;
    puVar8 = puVar8 + 1;
  }
  param_2[0x11] = param_6;
  param_2[6] = local_58;
  param_2[0x12] = param_4;
  param_2[7] = local_54;
  param_2[0x10] = 1;
  *local_10 = (int)param_3;
  return 0;
}

