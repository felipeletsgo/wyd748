// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00595f15 | Name: FUN_00595f15


undefined4 * FUN_00595f15(void)

{
  undefined4 *puVar1;
  LPVOID pvVar2;
  
  if (DAT_013b85f8 == DAT_013b85e8) {
    pvVar2 = HeapReAlloc(DAT_013b8604,0,DAT_013b85fc,(DAT_013b85e8 * 5 + 0x50) * 4);
    if (pvVar2 == (LPVOID)0x0) {
      return (undefined4 *)0x0;
    }
    DAT_013b85e8 = DAT_013b85e8 + 0x10;
    DAT_013b85fc = pvVar2;
  }
  puVar1 = (undefined4 *)((int)DAT_013b85fc + DAT_013b85f8 * 0x14);
  pvVar2 = HeapAlloc(DAT_013b8604,8,0x41c4);
  puVar1[4] = pvVar2;
  if (pvVar2 != (LPVOID)0x0) {
    pvVar2 = VirtualAlloc((LPVOID)0x0,0x100000,0x2000,4);
    puVar1[3] = pvVar2;
    if (pvVar2 != (LPVOID)0x0) {
      puVar1[2] = 0xffffffff;
      *puVar1 = 0;
      puVar1[1] = 0;
      DAT_013b85f8 = DAT_013b85f8 + 1;
      *(undefined4 *)puVar1[4] = 0xffffffff;
      return puVar1;
    }
    HeapFree(DAT_013b8604,0,(LPVOID)puVar1[4]);
  }
  return (undefined4 *)0x0;
}

