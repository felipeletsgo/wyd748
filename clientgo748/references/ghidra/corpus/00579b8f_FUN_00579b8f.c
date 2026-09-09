// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00579b8f | Name: FUN_00579b8f


void FUN_00579b8f(int param_1,byte *param_2,uint param_3)

{
  byte bVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  byte *pbVar5;
  int iVar6;
  byte *pbVar7;
  byte *pbVar8;
  uint local_c;
  byte local_5;
  
  bVar1 = *(byte *)(param_1 + 0xfb);
  uVar3 = (uint)bVar1;
  if (param_3 == 0xff) {
    uVar3 = uVar3 * *(int *)(param_1 + 0xb8) + 7;
    pbVar5 = (byte *)(*(int *)(param_1 + 0xdc) + 1);
    for (uVar4 = uVar3 >> 5; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined4 *)param_2 = *(undefined4 *)pbVar5;
      pbVar5 = pbVar5 + 4;
      param_2 = param_2 + 4;
    }
    for (uVar3 = uVar3 >> 3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
      *param_2 = *pbVar5;
      pbVar5 = pbVar5 + 1;
      param_2 = param_2 + 1;
    }
  }
  else if (uVar3 == 1) {
    iVar2 = *(int *)(param_1 + 0xb8);
    pbVar5 = (byte *)(*(int *)(param_1 + 0xdc) + 1);
    local_c = 0x80;
    iVar6 = 7;
    for (; iVar2 != 0; iVar2 = iVar2 + -1) {
      if ((local_c & param_3) != 0) {
        bVar1 = (byte)iVar6;
        *param_2 = (byte)(0x7f7f >> (7 - bVar1 & 0x1f)) & *param_2 |
                   (*pbVar5 >> (bVar1 & 0x1f) & 1) << (bVar1 & 0x1f);
      }
      if (iVar6 == 0) {
        pbVar5 = pbVar5 + 1;
        iVar6 = 7;
        param_2 = param_2 + 1;
      }
      else {
        iVar6 = iVar6 + -1;
      }
      if (local_c == 1) {
        local_c = 0x80;
      }
      else {
        local_c = (int)local_c >> 1;
      }
    }
  }
  else if (uVar3 == 2) {
    iVar2 = *(int *)(param_1 + 0xb8);
    pbVar5 = (byte *)(*(int *)(param_1 + 0xdc) + 1);
    local_c = 0x80;
    iVar6 = 6;
    for (; iVar2 != 0; iVar2 = iVar2 + -1) {
      if ((local_c & param_3) != 0) {
        bVar1 = (byte)iVar6;
        *param_2 = (byte)(0x3f3f >> (6 - bVar1 & 0x1f)) & *param_2 |
                   (*pbVar5 >> (bVar1 & 0x1f) & 3) << (bVar1 & 0x1f);
      }
      if (iVar6 == 0) {
        pbVar5 = pbVar5 + 1;
        iVar6 = 6;
        param_2 = param_2 + 1;
      }
      else {
        iVar6 = iVar6 + -2;
      }
      if (local_c == 1) {
        local_c = 0x80;
      }
      else {
        local_c = (int)local_c >> 1;
      }
    }
  }
  else if (uVar3 == 4) {
    iVar2 = *(int *)(param_1 + 0xb8);
    pbVar5 = (byte *)(*(int *)(param_1 + 0xdc) + 1);
    local_c = 0x80;
    iVar6 = 4;
    for (; iVar2 != 0; iVar2 = iVar2 + -1) {
      if ((local_c & param_3) != 0) {
        bVar1 = (byte)iVar6;
        *param_2 = (byte)(0xf0f >> (4 - bVar1 & 0x1f)) & *param_2 |
                   (*pbVar5 >> (bVar1 & 0x1f) & 0xf) << (bVar1 & 0x1f);
      }
      if (iVar6 == 0) {
        pbVar5 = pbVar5 + 1;
        iVar6 = 4;
        param_2 = param_2 + 1;
      }
      else {
        iVar6 = iVar6 + -4;
      }
      if (local_c == 1) {
        local_c = 0x80;
      }
      else {
        local_c = (int)local_c >> 1;
      }
    }
  }
  else {
    iVar2 = *(int *)(param_1 + 0xb8);
    uVar3 = (uint)(bVar1 >> 3);
    pbVar5 = (byte *)(*(int *)(param_1 + 0xdc) + 1);
    local_5 = 0x80;
    for (; iVar2 != 0; iVar2 = iVar2 + -1) {
      if ((local_5 & (byte)param_3) != 0) {
        pbVar7 = pbVar5;
        pbVar8 = param_2;
        for (uVar4 = (uint)(bVar1 >> 5); uVar4 != 0; uVar4 = uVar4 - 1) {
          *(undefined4 *)pbVar8 = *(undefined4 *)pbVar7;
          pbVar7 = pbVar7 + 4;
          pbVar8 = pbVar8 + 4;
        }
        for (uVar4 = uVar3 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
          *pbVar8 = *pbVar7;
          pbVar7 = pbVar7 + 1;
          pbVar8 = pbVar8 + 1;
        }
      }
      pbVar5 = pbVar5 + uVar3;
      param_2 = param_2 + uVar3;
      if (local_5 == 1) {
        local_5 = 0x80;
      }
      else {
        local_5 = local_5 >> 1;
      }
    }
  }
  return;
}

