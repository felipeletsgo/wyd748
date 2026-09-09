// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00572127 | Name: FUN_00572127


undefined4 FUN_00572127(int *param_1)

{
  byte *pbVar1;
  int iVar2;
  byte bVar3;
  undefined4 *puVar4;
  int *piVar5;
  char cVar6;
  int iVar7;
  ushort uVar8;
  byte *pbVar9;
  byte local_1c [4];
  char local_18;
  byte local_17;
  byte local_16;
  undefined1 local_15;
  byte local_14;
  byte local_13;
  byte local_12;
  byte local_11;
  byte local_10;
  byte local_f;
  int local_c;
  int local_8;
  
  piVar5 = param_1;
  puVar4 = (undefined4 *)param_1[5];
  pbVar9 = (byte *)*puVar4;
  iVar2 = puVar4[1];
  if (puVar4[1] == 0) {
    cVar6 = (*(code *)puVar4[3])(param_1);
    if (cVar6 == '\0') {
      return 0;
    }
    pbVar9 = (byte *)*puVar4;
    iVar2 = puVar4[1];
  }
  param_1 = (int *)iVar2;
  param_1 = (int *)((int)param_1 + -1);
  local_8 = (uint)*pbVar9 << 8;
  pbVar9 = pbVar9 + 1;
  if (param_1 == (int *)0x0) {
    cVar6 = (*(code *)puVar4[3])(piVar5);
    if (cVar6 == '\0') {
      return 0;
    }
    param_1 = (int *)puVar4[1];
    pbVar9 = (byte *)*puVar4;
  }
  param_1 = (int *)((int)param_1 + -1);
  local_8 = local_8 + (uint)*pbVar9 + -2;
  pbVar9 = pbVar9 + 1;
  if (local_8 < 0xe) {
    *(undefined4 *)(*piVar5 + 0x14) = 0x4c;
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
        pbVar9 = (byte *)*puVar4;
      }
      param_1 = (int *)((int)param_1 + -1);
      bVar3 = *pbVar9;
      pbVar9 = pbVar9 + 1;
      iVar2 = local_c + 1;
      pbVar1 = local_1c + local_c;
      local_c = iVar2;
      *pbVar1 = bVar3;
    } while (iVar2 < 0xe);
    iVar2 = local_8 + -0xe;
    if ((((local_1c[0] == 0x4a) && (local_1c[1] == 'F')) && (local_1c[2] == 'I')) &&
       ((local_1c[3] == 'F' && (local_18 == '\0')))) {
      if (local_17 == 1) {
        local_8 = iVar2;
        if (2 < local_16) {
          *(undefined4 *)(*piVar5 + 0x14) = 0x58;
          *(undefined4 *)(*piVar5 + 0x18) = 1;
          *(uint *)(*piVar5 + 0x1c) = (uint)local_16;
          (**(code **)(*piVar5 + 4))(piVar5,1);
        }
      }
      else {
        *(undefined4 *)(*piVar5 + 0x14) = 0x73;
        *(uint *)(*piVar5 + 0x18) = (uint)local_17;
        *(uint *)(*piVar5 + 0x1c) = (uint)local_16;
        local_8 = iVar2;
        (**(code **)(*piVar5 + 4))(piVar5,0xffffffff);
      }
      *(undefined1 *)((int)piVar5 + 0x101) = local_15;
      *(undefined1 *)(piVar5 + 0x40) = 1;
      uVar8 = (ushort)local_13 + (ushort)local_14 * 0x100;
      *(ushort *)((int)piVar5 + 0x102) = uVar8;
      *(ushort *)(piVar5 + 0x41) = (ushort)local_12 * 0x100 + (ushort)local_11;
      iVar2 = *piVar5;
      *(uint *)(iVar2 + 0x18) = (uint)uVar8;
      *(uint *)(iVar2 + 0x1c) = (uint)*(ushort *)(piVar5 + 0x41);
      *(uint *)(iVar2 + 0x20) = (uint)*(byte *)((int)piVar5 + 0x101);
      *(undefined4 *)(*piVar5 + 0x14) = 0x56;
      (**(code **)(*piVar5 + 4))(piVar5,1);
      if (local_f != 0 || local_10 != 0) {
        *(undefined4 *)(*piVar5 + 0x14) = 0x59;
        *(uint *)(*piVar5 + 0x18) = (uint)local_10;
        *(uint *)(*piVar5 + 0x1c) = (uint)local_f;
        (**(code **)(*piVar5 + 4))(piVar5,1);
      }
      if (local_8 == (uint)local_f * (uint)local_10 * 3) goto LAB_00572342;
      *(undefined4 *)(*piVar5 + 0x14) = 0x57;
      iVar7 = local_8;
    }
    else {
      *(undefined4 *)(*piVar5 + 0x14) = 0x4c;
      iVar7 = local_8;
      local_8 = iVar2;
    }
    *(int *)(*piVar5 + 0x18) = iVar7;
  }
  (**(code **)(*piVar5 + 4))(piVar5,1);
LAB_00572342:
  *puVar4 = pbVar9;
  puVar4[1] = param_1;
  if (0 < local_8) {
    (**(code **)(piVar5[5] + 0x10))(piVar5,local_8);
  }
  return 1;
}

