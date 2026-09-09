// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00594baa | Name: FUN_00594baa


void FUN_00594baa(uint param_1)

{
  uint *puVar1;
  undefined4 *puVar2;
  int iVar3;
  
  puVar1 = (uint *)FUN_00594c26();
  iVar3 = 0;
  *puVar1 = param_1;
  puVar1 = &DAT_005c9d70;
  do {
    if (param_1 == *puVar1) {
      puVar2 = (undefined4 *)FUN_00594c1d();
      *puVar2 = *(undefined4 *)(iVar3 * 8 + 0x5c9d74);
      return;
    }
    puVar1 = puVar1 + 2;
    iVar3 = iVar3 + 1;
  } while (puVar1 < &DAT_005c9ed8);
  if ((0x12 < param_1) && (param_1 < 0x25)) {
    puVar2 = (undefined4 *)FUN_00594c1d();
    *puVar2 = 0xd;
    return;
  }
  if ((0xbb < param_1) && (param_1 < 0xcb)) {
    puVar2 = (undefined4 *)FUN_00594c1d();
    *puVar2 = 8;
    return;
  }
  puVar2 = (undefined4 *)FUN_00594c1d();
  *puVar2 = 0x16;
  return;
}

