// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044ecae | Name: FUN_0044ecae


void __fastcall FUN_0044ecae(int param_1)

{
  int *piVar1;
  undefined1 local_14 [4];
  undefined2 local_10;
  undefined2 local_e;
  uint local_8;
  
  DAT_005d0404 = (uint)(DAT_005d0404 == 0);
  (**(code **)(**(int **)(param_1 + 0x27914) + 0x60))(DAT_005d0404);
  FUN_0044eca3();
  piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x132);
  _memset(local_14,0,0x10);
  local_e = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
  local_10 = 0x399;
  local_8 = DAT_005d0404;
  FUN_0055f2dd(local_14,0x10);
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x8c))(DAT_005d0404);
  }
  return;
}

