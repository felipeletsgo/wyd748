// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00571aec | Name: FUN_00571aec


undefined4 FUN_00571aec(void)

{
  undefined1 *puVar1;
  int iVar2;
  int *unaff_ESI;
  
  *(undefined4 *)(*unaff_ESI + 0x14) = 0x65;
  (**(code **)(*unaff_ESI + 4))();
  if (*(char *)(unaff_ESI[100] + 0x50) != '\0') {
    *(undefined4 *)(*unaff_ESI + 0x14) = 0x3c;
    (**(code **)*unaff_ESI)();
  }
  puVar1 = (undefined1 *)((int)unaff_ESI + 0xda);
  iVar2 = 0x10;
  do {
    puVar1[-0x10] = 0;
    *puVar1 = 1;
    puVar1[0x10] = 5;
    puVar1 = puVar1 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  iVar2 = unaff_ESI[100];
  unaff_ESI[0x3f] = 0;
  unaff_ESI[9] = 0;
  *(undefined1 *)(unaff_ESI + 0x42) = 0;
  *(undefined1 *)(unaff_ESI + 0x40) = 0;
  *(undefined1 *)((int)unaff_ESI + 0x101) = 0;
  *(undefined1 *)((int)unaff_ESI + 0x106) = 0;
  *(undefined1 *)((int)unaff_ESI + 0x107) = 0;
  *(undefined2 *)((int)unaff_ESI + 0x102) = 1;
  *(undefined2 *)(unaff_ESI + 0x41) = 1;
  *(undefined1 *)(iVar2 + 0x50) = 1;
  return CONCAT31((int3)((uint)iVar2 >> 8),1);
}

