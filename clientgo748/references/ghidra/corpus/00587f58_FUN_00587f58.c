// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00587f58 | Name: FUN_00587f58


void FUN_00587f58(int param_1,int param_2,undefined4 param_3,int param_4)

{
  short *psVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  byte *pbVar5;
  int iVar6;
  
  iVar3 = *(int *)(param_1 + 0x5c);
  iVar4 = *(int *)(*(int *)(param_1 + 0x1a4) + 0x18);
  iVar6 = 0;
  if (0 < param_4) {
    do {
      pbVar5 = *(byte **)(param_2 + iVar6 * 4);
      for (iVar2 = iVar3; iVar2 != 0; iVar2 = iVar2 + -1) {
        psVar1 = (short *)(*(int *)(iVar4 + (uint)(*pbVar5 >> 3) * 4) +
                          ((uint)(pbVar5[1] >> 2) * 0x20 + (uint)(pbVar5[2] >> 3)) * 2);
        *psVar1 = *psVar1 + 1;
        if (*psVar1 == 0) {
          *psVar1 = -1;
        }
        pbVar5 = pbVar5 + 3;
      }
      iVar6 = iVar6 + 1;
    } while (iVar6 < param_4);
  }
  return;
}

