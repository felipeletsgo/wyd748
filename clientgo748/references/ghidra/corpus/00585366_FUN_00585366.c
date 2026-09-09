// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00585366 | Name: FUN_00585366


void FUN_00585366(int param_1,int param_2,int *param_3)

{
  char cVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  int *piVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  undefined1 *puVar9;
  int iVar10;
  undefined4 *puVar11;
  undefined1 *puVar12;
  int aiStack_520 [257];
  char local_11c [260];
  byte *local_18;
  int local_14;
  int local_10;
  uint local_c;
  int local_8;
  
  if (*param_3 == 0) {
    iVar2 = (*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x5d4);
    *param_3 = iVar2;
  }
  iVar2 = *param_3;
  iVar3 = 0;
  *(int *)(iVar2 + 0xd0) = param_2;
  local_8 = 0;
  param_3 = (int *)0x1;
  do {
    uVar6 = (uint)*(byte *)((int)param_3 + param_2);
    local_c = uVar6;
    if (uVar6 != 0) {
      pcVar4 = local_11c + iVar3;
      for (uVar7 = (uint)(*(byte *)((int)param_3 + param_2) >> 2); uVar7 != 0; uVar7 = uVar7 - 1) {
        *(uint *)pcVar4 =
             CONCAT22(CONCAT11((char)param_3,(char)param_3),CONCAT11((char)param_3,(char)param_3));
        pcVar4 = pcVar4 + 4;
      }
      for (uVar7 = uVar6 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
        *pcVar4 = (char)param_3;
        pcVar4 = pcVar4 + 1;
      }
      iVar3 = local_8 + uVar6;
      local_8 = iVar3;
    }
    param_3 = (int *)((int)param_3 + 1);
  } while ((int)param_3 < 0x11);
  local_11c[iVar3] = '\0';
  iVar10 = (int)local_11c[0];
  iVar3 = 0;
  iVar8 = 0;
  if (local_11c[0] != '\0') {
    pcVar4 = local_11c;
    do {
      cVar1 = *pcVar4;
      while (cVar1 == iVar10) {
        cVar1 = local_11c[iVar3 + 1];
        aiStack_520[iVar3] = iVar8;
        iVar3 = iVar3 + 1;
        iVar8 = iVar8 + 1;
      }
      iVar8 = iVar8 << 1;
      pcVar4 = local_11c + iVar3;
      iVar10 = iVar10 + 1;
    } while (*pcVar4 != '\0');
  }
  iVar3 = 0;
  iVar8 = 1;
  piVar5 = (int *)(iVar2 + 0x48);
  do {
    if (*(char *)(iVar8 + param_2) == '\0') {
      *piVar5 = -1;
    }
    else {
      iVar10 = aiStack_520[iVar3];
      piVar5[0x12] = iVar3;
      piVar5[-0x11] = iVar10;
      iVar3 = iVar3 + (uint)*(byte *)(iVar8 + param_2);
      *piVar5 = aiStack_520[iVar3 + -1];
    }
    iVar8 = iVar8 + 1;
    piVar5 = piVar5 + 1;
  } while (iVar8 < 0x11);
  local_8 = 0;
  *(undefined4 *)(iVar2 + 0x88) = 0xfffff;
  puVar11 = (undefined4 *)(iVar2 + 0xd4);
  for (iVar3 = 0x100; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar11 = 0;
    puVar11 = puVar11 + 1;
  }
  param_3 = (int *)0x1;
  local_10 = 7;
  do {
    local_18 = (byte *)((int)param_3 + param_2);
    local_14 = 1;
    if (*local_18 != 0) {
      iVar3 = 1 << ((byte)local_10 & 0x1f);
      puVar9 = (undefined1 *)(local_8 + 0x11 + param_2);
      do {
        iVar8 = aiStack_520[local_8] << ((byte)local_10 & 0x1f);
        if (0 < iVar3) {
          puVar12 = (undefined1 *)(iVar8 + 0x4d4 + iVar2);
          piVar5 = (int *)(iVar2 + 0xd4 + iVar8 * 4);
          local_c = iVar3;
          do {
            *piVar5 = (int)param_3;
            piVar5 = piVar5 + 1;
            *puVar12 = *puVar9;
            puVar12 = puVar12 + 1;
            local_c = local_c + -1;
          } while (local_c != 0);
        }
        local_14 = local_14 + 1;
        local_8 = local_8 + 1;
        puVar9 = puVar9 + 1;
      } while (local_14 <= (int)(uint)*local_18);
    }
    param_3 = (int *)((int)param_3 + 1);
    local_10 = local_10 + -1;
  } while (-1 < local_10);
  return;
}

