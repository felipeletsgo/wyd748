// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d90dd | Name: FUN_004d90dd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d90dd(int param_1)

{
  int *piVar1;
  
  if ((((DAT_0092e660 != 1) && (_DAT_005a439c <= *(float *)(param_1 + 0x8c))) &&
      (*(int *)(param_1 + 0x28) == 1)) &&
     (piVar1 = (int *)FUN_004b14fd(0x2bf,1,180000), piVar1 != (int *)0x0)) {
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    piVar1[0xa6] = *(int *)(param_1 + 0x90) + 0x33;
    if (*(int *)(param_1 + 0x90) == 2) {
      piVar1[0xa6] = 0x36;
    }
    else if (*(int *)(param_1 + 0x90) == 3) {
      piVar1[0xa6] = 0x14;
    }
    else if (*(int *)(param_1 + 0x90) == 4) {
      piVar1[0xa6] = 0x33;
    }
    else if (*(int *)(param_1 + 0x90) == 5) {
      piVar1[0xa6] = 0x98;
    }
    if (((*(int *)(param_1 + 0x90) == 0) || (*(int *)(param_1 + 0x90) == 2)) ||
       ((*(int *)(param_1 + 0x90) == 3 ||
        ((*(int *)(param_1 + 0x90) == 4 || (*(int *)(param_1 + 0x90) == 5)))))) {
      piVar1[0xcc] = 0x3f800000;
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),*(undefined4 *)(param_1 + 0x88),0,0x3fc90fdb,0,0);
    }
    else {
      piVar1[0xcc] = 0x3f800000;
      (**(code **)(*piVar1 + 8))
                (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                 *(undefined4 *)(param_1 + 0x7c),0,0,0x3fc90fdb,0,0);
    }
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

