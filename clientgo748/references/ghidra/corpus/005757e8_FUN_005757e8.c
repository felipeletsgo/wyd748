// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005757e8 | Name: FUN_005757e8


undefined4 * FUN_005757e8(undefined4 param_1,int param_2,int param_3)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  undefined4 *puVar6;
  
  uVar5 = param_2 * param_3;
  puVar1 = (undefined4 *)FUN_005799ef(param_1,uVar5);
  puVar2 = (undefined4 *)0x0;
  if (puVar1 != (undefined4 *)0x0) {
    puVar6 = puVar1;
    if (0x8000 < uVar5) {
      puVar2 = puVar1;
      for (iVar3 = 0x2000; iVar3 != 0; iVar3 = iVar3 + -1) {
        *puVar2 = 0;
        puVar2 = puVar2 + 1;
      }
      uVar5 = uVar5 - 0x8000;
      puVar6 = puVar1 + 0x2000;
    }
    for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *puVar6 = 0;
      puVar6 = puVar6 + 1;
    }
    for (uVar5 = uVar5 & 3; puVar2 = puVar1, uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined1 *)puVar6 = 0;
      puVar6 = (undefined4 *)((int)puVar6 + 1);
    }
  }
  return puVar2;
}

