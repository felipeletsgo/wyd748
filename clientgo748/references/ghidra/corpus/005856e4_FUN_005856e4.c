// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005856e4 | Name: FUN_005856e4


undefined4 FUN_005856e4(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  
  iVar2 = *(int *)(param_1 + 0x194);
  piVar1 = (int *)(*(int *)(param_1 + 400) + 0x58);
  *piVar1 = *piVar1 + *(int *)(iVar2 + 0x14) / 8;
  *(undefined4 *)(iVar2 + 0x14) = 0;
  uVar3 = (**(code **)(*(int *)(param_1 + 400) + 8))(param_1);
  if ((char)uVar3 != '\0') {
    iVar5 = 0;
    if (0 < *(int *)(param_1 + 0x120)) {
      puVar4 = (undefined4 *)(iVar2 + 0x20);
      do {
        *puVar4 = 0;
        iVar5 = iVar5 + 1;
        puVar4 = puVar4 + 1;
      } while (iVar5 < *(int *)(param_1 + 0x120));
    }
    uVar3 = *(undefined4 *)(param_1 + 0xfc);
    *(undefined4 *)(iVar2 + 0x30) = uVar3;
    *(undefined1 *)(iVar2 + 0x18) = 0;
    uVar3 = CONCAT31((int3)((uint)uVar3 >> 8),1);
  }
  return uVar3;
}

