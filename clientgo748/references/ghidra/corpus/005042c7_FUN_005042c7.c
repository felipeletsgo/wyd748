// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005042c7 | Name: FUN_005042c7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_005042c7(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  int unaff_EBP;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  FUN_004be1ef(*(undefined4 *)(unaff_EBP + -0x44),param_2);
  FUN_00430f10();
  FUN_00430f50(*(float *)(unaff_EBP + -0x3c) / *(float *)(*(int *)(unaff_EBP + -0x68) + 0x7a4),
               *(float *)(unaff_EBP + -0x44) / *(float *)(*(int *)(unaff_EBP + -0x68) + 0x7a4),0);
  thunk_FUN_0056144f(unaff_EBP + -0x28,unaff_EBP + -0x38,
                     *(int *)(*(int *)(unaff_EBP + -0x68) + 0x60) + 0x34);
  *(undefined4 *)(*(int *)(unaff_EBP + -0x68) + 0x50) = *(undefined4 *)(unaff_EBP + -0x28);
  *(float *)(*(int *)(unaff_EBP + -0x68) + 0x54) =
       (*(float *)(unaff_EBP + -0x24) -
       _DAT_005a34a0 * *(float *)(*(int *)(unaff_EBP + -0x68) + 0x38)) +
       *(float *)(unaff_EBP + -0x2c);
  *(undefined4 *)(*(int *)(unaff_EBP + -0x68) + 0x58) = *(undefined4 *)(unaff_EBP + -0x20);
  if ((*(char *)(*(int *)(unaff_EBP + -0x68) + 0x795) == '\x01') ||
     (*(char *)(*(int *)(unaff_EBP + -0x68) + 0x796) == '\x01')) {
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
  }
  if (('\0' < *(char *)(*(int *)(unaff_EBP + -0x68) + 0x79b)) &&
     (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x198) != 0)) {
    *(undefined4 *)(unaff_EBP + -0x4c) = 0x3f800000;
    *(undefined4 *)(unaff_EBP + -0x48) = 4;
    if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x24c) == 1) {
      *(undefined4 *)(unaff_EBP + -0x48) = 0;
    }
    else if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x24c) == 2) {
      *(undefined4 *)(unaff_EBP + -0x48) = 1;
    }
    else if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x24c) == 4) {
      *(undefined4 *)(unaff_EBP + -0x48) = 2;
    }
    else if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x24c) == 8) {
      *(undefined4 *)(unaff_EBP + -0x48) = 3;
    }
    else if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x24c) == 0x26) {
      *(undefined4 *)(unaff_EBP + -0x48) = 1;
    }
    else if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x24c) == 0x19) {
      *(undefined4 *)(unaff_EBP + -0x48) = 5;
    }
    else if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x24c) == 0x1a) {
      *(undefined4 *)(unaff_EBP + -0x48) = 6;
    }
    else if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x24c) == 0x21) {
      *(undefined4 *)(unaff_EBP + -0x48) = 7;
    }
    else if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x24c) == 0x28) {
      *(undefined4 *)(unaff_EBP + -0x48) = 8;
    }
    else {
      *(undefined4 *)(unaff_EBP + -0x48) = 0;
    }
    if (*(int *)(unaff_EBP + -0x48) < 4) {
      if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x5c) == 1) {
        *(undefined4 *)(unaff_EBP + -0x4c) = 0x3f666666;
      }
      *(uint *)(unaff_EBP + -0x58) = (uint)*(ushort *)(*(int *)(unaff_EBP + -0x68) + 0x1da);
      *(undefined4 *)(unaff_EBP + -0x54) =
           *(undefined4 *)
            (*(int *)(unaff_EBP + -0x48) * 0x50 + 0x5bb488 + *(int *)(unaff_EBP + -0x58) * 4);
      if (0x27 < *(int *)(unaff_EBP + -0x58)) {
        *(undefined4 *)(unaff_EBP + -0x54) =
             *(undefined4 *)
              (&DAT_005bb528 + *(int *)(unaff_EBP + -0x58) * 4 + *(int *)(unaff_EBP + -0x48) * 0x50)
        ;
      }
      *(undefined4 *)(unaff_EBP + -0x50) = 0;
      if ((0xc7c < *(short *)(*(int *)(unaff_EBP + -0x68) + 0x7e0)) &&
         (*(short *)(*(int *)(unaff_EBP + -0x68) + 0x7e0) < 0xc80)) {
        *(undefined4 *)(unaff_EBP + -0x54) = 0x3df5c28f;
        *(undefined4 *)(unaff_EBP + -0x50) = 0x3d23d70a;
      }
      puVar2 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x60) + 0x74);
      puVar3 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x198) + 0x34);
      for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = *puVar2;
        puVar2 = puVar2 + 1;
        puVar3 = puVar3 + 1;
      }
      FUN_004be1ef(*(undefined4 *)(unaff_EBP + -0x54),*(undefined4 *)(unaff_EBP + -0x4c),
                   *(undefined4 *)(unaff_EBP + -0x50));
    }
    else if (*(int *)(unaff_EBP + -0x48) == 6) {
      puVar2 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x60) + 0x74);
      puVar3 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x198) + 0x34);
      for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = *puVar2;
        puVar2 = puVar2 + 1;
        puVar3 = puVar3 + 1;
      }
      FUN_004be1ef(0x3d8f5c29,0x3f99999a,0xbe4ccccd);
    }
    else if (*(int *)(unaff_EBP + -0x48) == 7) {
      puVar2 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x60) + 0x74);
      puVar3 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x198) + 0x34);
      for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = *puVar2;
        puVar2 = puVar2 + 1;
        puVar3 = puVar3 + 1;
      }
      FUN_004be1ef(0,0x3f99999a,0);
    }
    else if (*(int *)(unaff_EBP + -0x48) == 8) {
      puVar2 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x60) + 0x74);
      puVar3 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x198) + 0x34);
      for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = *puVar2;
        puVar2 = puVar2 + 1;
        puVar3 = puVar3 + 1;
      }
      FUN_004be1ef(0x3c23d70a,0x3f800000,0);
    }
    else {
      puVar2 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x60) + 0x74);
      puVar3 = (undefined4 *)(*(int *)(*(int *)(unaff_EBP + -0x68) + 0x198) + 0x34);
      for (iVar1 = 0x10; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar3 = *puVar2;
        puVar2 = puVar2 + 1;
        puVar3 = puVar3 + 1;
      }
      FUN_004be1ef(0x3d8f5c29,0x3f99999a,0xbdf5c28f);
    }
  }
  FUN_00500ebe();
  if (*(int *)(*(int *)(unaff_EBP + -0x68) + 0x48) == 1) {
    FUN_00504a80();
  }
  return 1;
}

