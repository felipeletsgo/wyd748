// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ae3d6 | Name: FUN_004ae3d6


undefined4 __thiscall FUN_004ae3d6(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  int local_c;
  
  iVar1 = FUN_0049889a(param_2,param_3);
  if (iVar1 == 1) {
    uVar2 = 1;
  }
  else if (param_3 == 0) {
    uVar2 = 0;
  }
  else if (*(short *)(param_3 + 4) == 0x10e) {
    (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(0,1);
    (**(code **)(*DAT_0092e654 + 4))(*(undefined4 *)(param_3 + 8));
    FUN_0058f220(DAT_013b71e8 + 1,param_3 + 0xc,0x2e8);
    FUN_0058f220(DAT_013b71e8 + 0xbb,param_3 + 0x2f4,0x400);
    DAT_013b71e8[0x316] = *(int *)(param_3 + 0x6f4);
    _memset(DAT_013b71e8 + 0x6c25,0,0x3390);
    for (local_c = 0; local_c < 0x10; local_c = local_c + 1) {
      *(undefined1 *)(DAT_013b71e4 + local_c + 0x20) = *(undefined1 *)(param_3 + local_c + 0x708);
    }
    *(undefined4 *)(DAT_013b71e4 + 0x40) = 0;
    *(undefined4 *)(DAT_013b71e4 + 0x44) = 0;
    (**(code **)(*DAT_013b71e8 + 0x54))(5);
    uVar2 = 1;
  }
  else if ((*(short *)(param_3 + 4) == 0x11d) || (*(short *)(param_3 + 4) == 0x11c)) {
    FUN_00403df2(&DAT_00a34778,4000);
    (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
    (**(code **)(**(int **)(param_1 + 0x26e60) + 100))(1);
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}

