// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b67e0 | Name: FUN_004b67e0


int __thiscall FUN_004b67e0(int param_1,int param_2,uint param_3,uint param_4)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  byte *pbVar5;
  char *pcVar6;
  uint local_8;
  
  uVar2 = FUN_004b6430();
  if (uVar2 < param_3) {
    FUN_0059e4cc();
  }
  iVar3 = FUN_004b6430();
  local_8 = iVar3 - param_3;
  if (param_4 < local_8) {
    local_8 = param_4;
  }
  if (param_1 == param_2) {
    FUN_004b6c30(param_3 + local_8,DAT_005a467c);
    FUN_004b6c30(0,param_3);
  }
  else {
    if ((local_8 != 0) && (uVar2 = FUN_004b6430(), local_8 == uVar2)) {
      uVar4 = FUN_004b6400();
      pbVar5 = (byte *)FUN_004b6eb0(uVar4);
      if ((*pbVar5 < 0xfe) && (cVar1 = FUN_004b74b0(param_1,param_2), cVar1 != '\0')) {
        FUN_004b6980(1);
        uVar4 = FUN_004b6400();
        *(undefined4 *)(param_1 + 4) = uVar4;
        uVar4 = FUN_004b6430();
        *(undefined4 *)(param_1 + 8) = uVar4;
        uVar4 = FUN_004b6d00();
        *(undefined4 *)(param_1 + 0xc) = uVar4;
        pcVar6 = (char *)FUN_004b6eb0(*(undefined4 *)(param_1 + 4));
        *pcVar6 = *pcVar6 + '\x01';
        return param_1;
      }
    }
    cVar1 = FUN_004b6d70(local_8,1);
    if (cVar1 != '\0') {
      iVar3 = FUN_004b6400(local_8);
      FUN_004b66f0(*(undefined4 *)(param_1 + 4),iVar3 + param_3);
      FUN_004b6d20(local_8);
    }
  }
  return param_1;
}

