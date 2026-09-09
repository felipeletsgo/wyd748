// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00596c04 | Name: FUN_00596c04


int FUN_00596c04(byte *param_1,byte *param_2)

{
  byte *pbVar1;
  int iVar2;
  uint uVar3;
  byte *pbVar4;
  ushort uVar5;
  ushort uVar6;
  uint uVar7;
  byte *local_10;
  byte *local_c;
  byte local_8;
  byte local_7;
  
  if (DAT_013b83cc == 0) {
    iVar2 = FUN_0059b540(param_1,param_2);
  }
  else {
    FUN_00596aad(0x19);
    local_10 = param_2 + -1;
    local_c = param_1 + -1;
    do {
      uVar7 = (uint)*param_1;
      pbVar4 = param_1 + 1;
      pbVar1 = local_c + 1;
      if ((*(byte *)((int)&DAT_013b84e0 + uVar7 + 1) & 4) == 0) {
        param_1 = pbVar4;
        local_c = pbVar1;
        if ((*(byte *)((int)&DAT_013b84e0 + uVar7 + 1) & 0x10) == 0x10) {
          uVar7 = (uint)(byte)(&DAT_013b83e0)[uVar7];
        }
      }
      else if (*pbVar4 == 0) {
        uVar7 = 0;
        param_1 = pbVar4;
        local_c = pbVar1;
      }
      else {
        iVar2 = FUN_0059b2e2(DAT_013b85e4,0x200,pbVar1,2,&local_8,2,DAT_013b83b4,1);
        if (iVar2 == 1) {
          uVar7 = (uint)local_8;
        }
        else {
          if (iVar2 != 2) goto LAB_00596d68;
          uVar7 = (uint)local_8 * 0x100 + (uint)local_7;
        }
        param_1 = param_1 + 2;
        local_c = local_c + 2;
      }
      uVar3 = (uint)*param_2;
      uVar5 = (ushort)*param_2;
      pbVar4 = param_2 + 1;
      pbVar1 = local_10 + 1;
      if ((*(byte *)((int)&DAT_013b84e0 + uVar3 + 1) & 4) == 0) {
        param_2 = pbVar4;
        local_10 = pbVar1;
        if ((*(byte *)((int)&DAT_013b84e0 + uVar3 + 1) & 0x10) == 0x10) {
          uVar5 = (ushort)(byte)(&DAT_013b83e0)[uVar3];
        }
      }
      else if (*pbVar4 == 0) {
        uVar5 = 0;
        param_2 = pbVar4;
        local_10 = pbVar1;
      }
      else {
        iVar2 = FUN_0059b2e2(DAT_013b85e4,0x200,pbVar1,2,&local_8,2,DAT_013b83b4,1);
        if (iVar2 == 1) {
          uVar5 = (ushort)local_8;
        }
        else {
          if (iVar2 != 2) {
LAB_00596d68:
            FUN_00596b0e(0x19);
            return 0x7fffffff;
          }
          uVar5 = (ushort)local_8 * 0x100 + (ushort)local_7;
        }
        param_2 = param_2 + 2;
        local_10 = local_10 + 2;
      }
      uVar6 = (ushort)uVar7;
      if (uVar6 != uVar5) {
        FUN_00596b0e(0x19);
        return (-(uint)(uVar5 < uVar6) & 2) - 1;
      }
    } while (uVar6 != 0);
    FUN_00596b0e(0x19);
    iVar2 = 0;
  }
  return iVar2;
}

