// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054be92 | Name: FUN_0054be92


undefined4 FUN_0054be92(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  short local_14;
  short local_10;
  short local_c;
  int local_8;
  
  iVar2 = FUN_00559ef5(param_2);
  if (iVar2 == 0) {
    uVar3 = 0;
  }
  else {
    for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
      iVar2 = FUN_005588b2(*(undefined1 *)(param_3 + 0x81),local_8);
      if (iVar2 != 0) {
        uVar3 = *(undefined4 *)(&DAT_005c38c8 + local_8 * 0xc);
        uVar1 = *(undefined4 *)(&DAT_005c38cc + local_8 * 0xc);
        local_10 = (short)*(undefined4 *)(&DAT_005c38c4 + local_8 * 0xc);
        *(short *)(param_1 + 0x2e) = *(short *)(param_1 + 0x2e) + local_10;
        *(short *)(param_1 + 0x4a) = *(short *)(param_1 + 0x4a) + local_10;
        local_c = (short)uVar1;
        *(short *)(param_1 + 0x2c) = *(short *)(param_1 + 0x2c) + local_c;
        *(short *)(param_1 + 0x48) = *(short *)(param_1 + 0x48) + local_c;
        local_14 = (short)uVar3;
        *(short *)(param_1 + 0x26) = *(short *)(param_1 + 0x26) + local_14;
      }
    }
    uVar3 = 1;
  }
  return uVar3;
}

