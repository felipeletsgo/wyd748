// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054b8c6 | Name: FUN_0054b8c6


undefined4 FUN_0054b8c6(int param_1,undefined4 param_2,int param_3)

{
  char cVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  short sVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  uint uVar11;
  int iVar12;
  int local_4c;
  int local_44;
  int local_24;
  int local_18;
  int local_14;
  
  for (local_14 = 0; local_14 < 100; local_14 = local_14 + 1) {
    iVar12 = (int)*(short *)(param_1 + 0x24);
    if (((iVar12 <= DAT_0067cf34 + -1) || (400 < iVar12)) ||
       (*(uint *)(&DAT_005c12b0 + iVar12 * 4) <= *(uint *)(param_1 + 0x1c))) break;
    *(short *)(param_1 + 0x24) = *(short *)(param_1 + 0x24) + -1;
    *(short *)(param_1 + 0x40) = *(short *)(param_1 + 0x40) + -1;
  }
  uVar11 = (uint)*(byte *)(param_1 + 0x14);
  if (uVar11 < 4) {
    local_24 = FUN_00559ef5(param_2);
    if ((local_24 == 1) && ((*(uint *)(param_1 + 0x2dc) & 0x40000000) != 0)) {
      local_24 = 2;
    }
    sVar2 = *(short *)(param_1 + 0x34);
    uVar10 = *(undefined4 *)(&DAT_005be0f8 + uVar11 * 0x18);
    sVar3 = *(short *)(param_1 + 0x36);
    uVar7 = *(undefined4 *)(&DAT_005be0fc + uVar11 * 0x18);
    sVar4 = *(short *)(param_1 + 0x38);
    uVar8 = *(undefined4 *)(&DAT_005be100 + uVar11 * 0x18);
    sVar5 = *(short *)(param_1 + 0x3a);
    uVar9 = *(undefined4 *)(&DAT_005be104 + uVar11 * 0x18);
    sVar6 = *(short *)(param_1 + 0x24);
    iVar12 = (int)sVar6;
    if (local_24 == 0) {
      if (iVar12 < 0x163) {
        local_18._0_2_ = sVar6 * 5;
        if (0xfd < iVar12) {
          local_18._0_2_ = sVar6 * 10 + -0x4f6;
        }
        if (0x12a < iVar12) {
          local_18._0_2_ = (short)local_18 + (sVar6 + -299) * 10;
        }
      }
      else {
        local_18._0_2_ = 0x6ea;
        if (0xfd < iVar12) {
          local_18._0_2_ = 0x8de;
        }
        if (0x12a < iVar12) {
          local_18._0_2_ = (short)local_18 + 0x226;
        }
        local_18._0_2_ = (short)local_18 + (sVar6 + -0x162) * 0xc;
      }
    }
    else if (local_24 == 1) {
      if (iVar12 < 0x163) {
        iVar12 = FUN_0054cd07(param_1 + 0x5c,0x56);
        if (iVar12 < 0x39) {
          local_18._0_2_ = sVar6 * 6 + (short)iVar12 * 3;
        }
        else {
          local_18._0_2_ = sVar6 * 6 + -0x118 + (short)iVar12 * 8;
        }
      }
      else {
        iVar12 = FUN_0054cd07(param_1 + 0x5c,0x56);
        if (iVar12 < 0x39) {
          local_18._0_2_ = (short)iVar12 * 3 + 0x84c;
        }
        else {
          local_18._0_2_ = (short)iVar12 * 8 + 0x734;
        }
        local_18._0_2_ = (short)local_18 + (sVar6 + -0x162) * 0xc;
      }
    }
    else {
      local_4c = 0;
      if (0x77 < iVar12) {
        local_4c = (iVar12 + -0x78) * 4;
      }
      local_18 = iVar12 * 6 + local_4c;
      for (local_44 = 0; local_44 < 4; local_44 = local_44 + 1) {
        iVar12 = FUN_005588b2(*(undefined1 *)(param_3 + 1),local_44);
        if (iVar12 != 0) {
          local_18 = local_18 + 100;
        }
      }
      cVar1 = *(char *)(param_3 + 3);
      if (cVar1 == '\x01') {
        local_18._0_2_ = (short)local_18 + 100;
      }
      else if (cVar1 == '\x02') {
        local_18._0_2_ = (short)local_18 + 300;
      }
      else if (cVar1 == '\x03') {
        local_18._0_2_ = (short)local_18 + 600;
      }
      else if (cVar1 == '\x04') {
        local_18._0_2_ = (short)local_18 + 900;
      }
      else if (cVar1 == '\x05') {
        local_18._0_2_ = (short)local_18 + 0x4b0;
      }
      local_18._0_2_ = (short)local_18 + 1000;
    }
    *(short *)(param_1 + 0x2e0) =
         (short)local_18 -
         ((sVar2 - (short)uVar10) + (sVar3 - (short)uVar7) + (sVar4 - (short)uVar8) +
         (sVar5 - (short)uVar9));
    uVar10 = 1;
  }
  else {
    uVar10 = 0;
  }
  return uVar10;
}

