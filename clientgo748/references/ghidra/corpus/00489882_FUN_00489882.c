// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489882 | Name: FUN_00489882


undefined4 __thiscall FUN_00489882(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  uint local_c;
  
  if (*(char *)(param_2 + 0xd) == '\x0e') {
    *(uint *)(DAT_013b71e8 + 0x704) =
         *(int *)(DAT_013b71e8 + 0x704) + *(uint *)(param_2 + 0x20) % 100000000;
    *(undefined4 *)(*(int *)(param_1 + 0x2899c) + 0x390) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x289a0) + 0x390) = 0;
    if (DAT_005ccf98 != 0) {
      iVar1 = FUN_00429a6d(0x154);
      if (iVar1 != 0) {
        uVar3 = 0;
        uVar2 = 0;
        FUN_00429a6d(0x154);
        FUN_0042ad2b(uVar2,uVar3);
      }
    }
  }
  else {
    *(undefined4 *)(*(int *)(param_1 + 0x2899c) + 0x390) = *(undefined4 *)(param_2 + 0x20);
    *(undefined4 *)(*(int *)(param_1 + 0x289a0) + 0x390) = *(undefined4 *)(param_2 + 0x20);
  }
  for (local_c = 0; (int)local_c < 5; local_c = local_c + 1) {
    if ('\0' < *(char *)(param_2 + local_c + 0xc)) {
      FUN_0040ac07(local_c & 0xff);
      FUN_0040ac07(local_c & 0xff);
    }
  }
  if (0 < *(int *)(param_2 + 0x1c)) {
    *(undefined4 *)(*(int *)(param_1 + 0x2899c) + 0x3a0) = *(undefined4 *)(param_2 + 0x1c);
    *(undefined4 *)(*(int *)(param_1 + 0x289a0) + 0x3a0) = *(undefined4 *)(param_2 + 0x1c);
  }
  FUN_0040a8de(0,(int)*(char *)(param_2 + 0x11),(int)*(char *)(param_2 + 0x12),
               (int)*(char *)(param_2 + 0x13),3000);
  FUN_0040a8de(0,(int)*(char *)(param_2 + 0x11),(int)*(char *)(param_2 + 0x12),
               (int)*(char *)(param_2 + 0x13),3000);
  return 1;
}

