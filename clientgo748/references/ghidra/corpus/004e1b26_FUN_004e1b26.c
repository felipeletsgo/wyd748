// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004e1b26 | Name: FUN_004e1b26


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004e1b26(int param_1)

{
  int *piVar1;
  
  if (_DAT_005a439c <= *(float *)(param_1 + 0xac)) {
    piVar1 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0xa0),1,180000);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
    if (*(int *)(param_1 + 0x9c) == 0) {
      piVar1[0xcb] = 0x3fc00000;
      piVar1[0xcc] = 0x3fc00000;
    }
    else if (*(int *)(param_1 + 0x9c) == 1) {
      piVar1[0xcb] = 0x40a00000;
      piVar1[0xcc] = 0x40000000;
    }
    if (*(int *)(param_1 + 0x9c) != 3) {
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),*(undefined4 *)(param_1 + 0xa4),
                 *(undefined4 *)(param_1 + 0xa8),0x3fc90fdb,0,0);
    }
    (**(code **)(**(int **)(param_1 + 0xb8) + 0x40))();
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
  }
  return 1;
}

