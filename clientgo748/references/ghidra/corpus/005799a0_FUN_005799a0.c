// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005799a0 | Name: FUN_005799a0


undefined4 * FUN_005799a0(int param_1)

{
  undefined4 *puVar1;
  uint uVar2;
  int iVar3;
  undefined4 *puVar4;
  
  if (param_1 == 2) {
    uVar2 = 0x40;
  }
  else {
    if (param_1 != 1) {
      return (undefined4 *)0x0;
    }
    uVar2 = 0x19c;
  }
  puVar1 = _malloc(uVar2);
  if (puVar1 != (undefined4 *)0x0) {
    puVar4 = puVar1;
    for (uVar2 = uVar2 >> 2; uVar2 != 0; uVar2 = uVar2 - 1) {
      *puVar4 = 0;
      puVar4 = puVar4 + 1;
    }
    for (iVar3 = 0; iVar3 != 0; iVar3 = iVar3 + -1) {
      *(undefined1 *)puVar4 = 0;
      puVar4 = (undefined4 *)((int)puVar4 + 1);
    }
  }
  return puVar1;
}

