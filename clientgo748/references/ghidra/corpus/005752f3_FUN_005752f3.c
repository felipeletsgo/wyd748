// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005752f3 | Name: FUN_005752f3


void FUN_005752f3(int param_1)

{
  char cVar1;
  int iVar2;
  byte *pbVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int local_8;
  
  iVar2 = param_1;
  cVar1 = *(char *)(param_1 + 0x116);
  if ((*(uint *)(param_1 + 0x60) & 0x602000) != 0) {
    FUN_00574fea(param_1);
    if (cVar1 == '\x03') {
      uVar5 = (uint)*(ushort *)(param_1 + 0x108);
      if (uVar5 != 0) {
        pbVar3 = (byte *)(*(int *)(param_1 + 0x104) + 2);
        do {
          pbVar3[-2] = *(byte *)((uint)pbVar3[-2] + *(int *)(param_1 + 0x138));
          pbVar3[-1] = *(byte *)((uint)pbVar3[-1] + *(int *)(param_1 + 0x138));
          *pbVar3 = *(byte *)((uint)*pbVar3 + *(int *)(param_1 + 0x138));
          pbVar3 = pbVar3 + 3;
          uVar5 = uVar5 - 1;
        } while (uVar5 != 0);
      }
    }
  }
  if (((*(byte *)(param_1 + 0x60) & 8) != 0) && (cVar1 == '\x03')) {
    pbVar3 = (byte *)(param_1 + 0x152);
    iVar6 = 8 - (uint)*(byte *)(param_1 + 0x150);
    param_1 = 8 - (uint)*(byte *)(param_1 + 0x151);
    local_8 = 8 - (uint)*pbVar3;
    if ((iVar6 < 0) || (8 < iVar6)) {
      iVar6 = 0;
    }
    if ((param_1 < 0) || (8 < param_1)) {
      param_1 = 0;
    }
    if ((local_8 < 0) || (8 < local_8)) {
      local_8 = 0;
    }
    if (*(ushort *)(iVar2 + 0x108) != 0) {
      iVar4 = 0;
      uVar5 = (uint)*(ushort *)(iVar2 + 0x108);
      do {
        pbVar3 = (byte *)(iVar4 + *(int *)(iVar2 + 0x104));
        *pbVar3 = *pbVar3 >> ((byte)iVar6 & 0x1f);
        pbVar3 = (byte *)(iVar4 + 1 + *(int *)(iVar2 + 0x104));
        *pbVar3 = *pbVar3 >> ((byte)param_1 & 0x1f);
        pbVar3 = (byte *)(iVar4 + 2 + *(int *)(iVar2 + 0x104));
        *pbVar3 = *pbVar3 >> ((byte)local_8 & 0x1f);
        iVar4 = iVar4 + 3;
        uVar5 = uVar5 - 1;
      } while (uVar5 != 0);
    }
  }
  return;
}

