// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00572363 | Name: FUN_00572363


undefined4 FUN_00572363(int *param_1)

{
  byte *pbVar1;
  int iVar2;
  byte bVar3;
  undefined4 *puVar4;
  int *piVar5;
  char cVar6;
  byte *pbVar7;
  byte local_18 [4];
  char local_14;
  undefined1 local_13;
  undefined1 local_12;
  undefined1 local_11;
  undefined1 local_10;
  undefined1 local_f;
  undefined1 local_e;
  byte local_d;
  int local_c;
  int local_8;
  
  piVar5 = param_1;
  puVar4 = (undefined4 *)param_1[5];
  pbVar7 = (byte *)*puVar4;
  iVar2 = puVar4[1];
  if (puVar4[1] == 0) {
    cVar6 = (*(code *)puVar4[3])(param_1);
    if (cVar6 == '\0') {
      return 0;
    }
    pbVar7 = (byte *)*puVar4;
    iVar2 = puVar4[1];
  }
  param_1 = (int *)iVar2;
  param_1 = (int *)((int)param_1 + -1);
  local_8 = (uint)*pbVar7 << 8;
  pbVar7 = pbVar7 + 1;
  if (param_1 == (int *)0x0) {
    cVar6 = (*(code *)puVar4[3])(piVar5);
    if (cVar6 == '\0') {
      return 0;
    }
    param_1 = (int *)puVar4[1];
    pbVar7 = (byte *)*puVar4;
  }
  param_1 = (int *)((int)param_1 + -1);
  local_8 = local_8 + (uint)*pbVar7 + -2;
  pbVar7 = pbVar7 + 1;
  if (local_8 < 0xc) {
    *(undefined4 *)(*piVar5 + 0x14) = 0x4d;
    *(int *)(*piVar5 + 0x18) = local_8;
  }
  else {
    local_c = 0;
    do {
      if (param_1 == (int *)0x0) {
        cVar6 = (*(code *)puVar4[3])(piVar5);
        if (cVar6 == '\0') {
          return 0;
        }
        param_1 = (int *)puVar4[1];
        pbVar7 = (byte *)*puVar4;
      }
      param_1 = (int *)((int)param_1 + -1);
      bVar3 = *pbVar7;
      pbVar7 = pbVar7 + 1;
      iVar2 = local_c + 1;
      pbVar1 = local_18 + local_c;
      local_c = iVar2;
      *pbVar1 = bVar3;
    } while (iVar2 < 0xc);
    if ((((local_18[0] == 0x41) && (local_18[1] == 'd')) && (local_18[2] == 'o')) &&
       ((local_18[3] == 'b' && (local_14 == 'e')))) {
      iVar2 = *piVar5;
      *(uint *)(iVar2 + 0x18) = (uint)CONCAT11(local_13,local_12);
      *(uint *)(iVar2 + 0x1c) = (uint)CONCAT11(local_11,local_10);
      *(uint *)(iVar2 + 0x20) = (uint)CONCAT11(local_f,local_e);
      *(uint *)(iVar2 + 0x24) = (uint)local_d;
      *(undefined4 *)(*piVar5 + 0x14) = 0x4b;
      local_8 = local_8 + -0xc;
      (**(code **)(*piVar5 + 4))(piVar5,1);
      *(undefined1 *)((int)piVar5 + 0x106) = 1;
      *(byte *)((int)piVar5 + 0x107) = local_d;
      goto LAB_005724ba;
    }
    *(undefined4 *)(*piVar5 + 0x14) = 0x4d;
    *(int *)(*piVar5 + 0x18) = local_8;
    local_8 = local_8 + -0xc;
  }
  (**(code **)(*piVar5 + 4))(piVar5,1);
LAB_005724ba:
  *puVar4 = pbVar7;
  puVar4[1] = param_1;
  if (0 < local_8) {
    (**(code **)(piVar5[5] + 0x10))(piVar5,local_8);
  }
  return 1;
}

