// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055fad6 | Name: FUN_0055fad6


uint __fastcall FUN_0055fad6(int param_1,int param_2)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  uint *puVar5;
  uint local_8;
  
  uVar3 = 0;
  if (*(int *)(&DAT_005a6770 + (*(int *)(param_1 + 4) * 5 + *(int *)(param_2 + 4)) * 4) != 0) {
    local_8 = 0;
    puVar5 = (uint *)(param_1 + 0xc);
    iVar4 = 5;
    do {
      uVar1 = *puVar5;
      if (uVar1 != 0) {
        local_8 = local_8 + 1;
      }
      uVar2 = *(uint *)((param_2 - param_1) + (int)puVar5);
      if (uVar1 < uVar2) {
        if (uVar1 == 0) {
          uVar3 = uVar3 + 0x100;
        }
        else {
          uVar3 = uVar3 + (uVar2 - uVar1);
        }
      }
      else if (uVar2 < uVar1) {
        if (uVar2 == 0) {
          uVar3 = uVar3 + 0x1000000;
        }
        else {
          uVar3 = uVar3 + (uVar1 - uVar2) * 0x10000;
        }
      }
      puVar5 = puVar5 + 1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    if (local_8 != uVar3 >> 0x18) {
      return uVar3;
    }
  }
  return 0xffffffff;
}

