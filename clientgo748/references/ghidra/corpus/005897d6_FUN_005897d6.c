// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005897d6 | Name: FUN_005897d6


void FUN_005897d6(void)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  int unaff_ESI;
  int iVar3;
  
  iVar3 = 0;
  if (0 < *(int *)(unaff_ESI + 100)) {
    puVar2 = (undefined4 *)(*(int *)(unaff_ESI + 0x1a4) + 0x44);
    do {
      uVar1 = (**(code **)(*(int *)(unaff_ESI + 4) + 4))();
      *puVar2 = uVar1;
      iVar3 = iVar3 + 1;
      puVar2 = puVar2 + 1;
    } while (iVar3 < *(int *)(unaff_ESI + 100));
  }
  return;
}

