// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c0052 | Name: FUN_004c0052


/* WARNING (jumptable): Unable to track spacebase fully for stack */

undefined4 FUN_004c0052(void)

{
  void *pvVar1;
  undefined4 uVar2;
  int iVar3;
  int unaff_EBP;
  
  *(undefined4 *)(unaff_EBP + -0xa4) = 0;
  pvVar1 = operator_new(0xd4);
  *(void **)(unaff_EBP + -0xc4) = pvVar1;
  *(undefined4 *)(unaff_EBP + -4) = 2;
  if (*(int *)(unaff_EBP + -0xc4) == 0) {
    *(undefined4 *)(unaff_EBP + -0xe0) = 0;
  }
  else {
    uVar2 = FUN_004c372e(*(undefined4 *)(unaff_EBP + -0xd0));
    *(undefined4 *)(unaff_EBP + -0xe0) = uVar2;
  }
  *(undefined4 *)(unaff_EBP + -0xc0) = *(undefined4 *)(unaff_EBP + -0xe0);
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  *(undefined4 *)(unaff_EBP + -0xa4) = *(undefined4 *)(unaff_EBP + -0xc0);
  if (*(int *)(unaff_EBP + -0xa4) == 0) {
    ExceptionList = *(void **)(unaff_EBP + -0xc);
    return 0x80004005;
  }
  *(uint *)(unaff_EBP + -0xa0) =
       (uint)*(byte *)(*(int *)(unaff_EBP + -0x40) + *(int *)(unaff_EBP + -0x3c));
  *(uint *)(unaff_EBP + -0x98) =
       (uint)*(byte *)(*(int *)(unaff_EBP + -0x40) + *(int *)(unaff_EBP + -0x3c) + 8);
  if (0xc < *(int *)(unaff_EBP + -0xa0)) {
    *(undefined4 *)(unaff_EBP + -0xa0) = 0xc;
  }
  if (0xc < *(int *)(unaff_EBP + -0x98)) {
    *(undefined4 *)(unaff_EBP + -0x98) = 0xc;
  }
  uVar2 = FUN_004b9bb9(unaff_EBP + -0x80);
  *(undefined4 *)(*(int *)(unaff_EBP + -0xa4) + 200) = uVar2;
  *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc4) = *(undefined2 *)(unaff_EBP + -0xa0);
  *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = *(undefined2 *)(unaff_EBP + -0x98);
  *(uint *)(unaff_EBP + -0x9c) =
       (uint)*(byte *)(*(int *)(unaff_EBP + -0x34) + *(int *)(unaff_EBP + -0x3c));
  if (0xc < *(int *)(unaff_EBP + -0x9c)) {
    *(undefined4 *)(unaff_EBP + -0xa0) = 0xc;
  }
  *(undefined4 *)(unaff_EBP + -0xe4) = *(undefined4 *)(unaff_EBP + -0x9c);
  *(int *)(unaff_EBP + -0xe4) = *(int *)(unaff_EBP + -0xe4) + -0x74;
  switch(*(undefined4 *)(unaff_EBP + -0xe4)) {
  case 0:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x74;
    break;
  case 1:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x75;
    break;
  case 2:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x76;
    break;
  case 3:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x77;
    break;
  case 4:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x78;
    break;
  case 5:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x79;
    break;
  case 6:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x7a;
    break;
  case 7:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x7b;
    break;
  case 8:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x7c;
    break;
  case 9:
    *(undefined2 *)(*(int *)(unaff_EBP + -0xa4) + 0xc6) = 0x7d;
  }
  if ((**(int **)(unaff_EBP + -0xd0) < 0x13) &&
     ((*(int *)(unaff_EBP + -0x3c) == 6 || (*(int *)(unaff_EBP + -0x3c) == 7)))) {
    if (*(int *)(unaff_EBP + -0x3c) == 6) {
      *(undefined4 *)(*(int *)(unaff_EBP + -0xa4) + 8) =
           *(undefined4 *)(&DAT_005baa88 + **(int **)(unaff_EBP + -0xd0) * 8);
    }
    else if (*(int *)(unaff_EBP + -0x3c) == 7) {
      *(undefined4 *)(*(int *)(unaff_EBP + -0xa4) + 8) =
           *(undefined4 *)(&DAT_005baa8c + **(int **)(unaff_EBP + -0xd0) * 8);
    }
    uVar2 = FUN_004c15fc(*(undefined4 *)(*(int *)(unaff_EBP + -0xa4) + 8));
    *(undefined4 *)(unaff_EBP + -0x38) = uVar2;
    if (*(int *)(unaff_EBP + -0x38) == 0) {
      FUN_00431159(s_Can_t_Find_Parent_Node_in_ID_____005bb2ac,
                   *(undefined4 *)(*(int *)(unaff_EBP + -0xa4) + 8),unaff_EBP + -0x30);
      uVar2 = FUN_004bead1();
      return uVar2;
    }
    *(undefined4 *)(*(int *)(unaff_EBP + -0x38) + 0x90) = *(undefined4 *)(unaff_EBP + -0xa4);
    FUN_004c37c4();
  }
  else {
    iVar3 = FUN_004c097c(unaff_EBP + -0x30);
    if (iVar3 == 1) {
      uVar2 = FUN_004c15fc(*(undefined4 *)(*(int *)(unaff_EBP + -0xa4) + 8));
      *(undefined4 *)(unaff_EBP + -0x38) = uVar2;
      if (*(int *)(unaff_EBP + -0x38) == 0) {
        FUN_00431159(s_Can_t_Find_Parent_Node_in_ID_____005bb2e0,
                     *(undefined4 *)(*(int *)(unaff_EBP + -0xa4) + 8),unaff_EBP + -0x30);
        uVar2 = FUN_004bead1();
        return uVar2;
      }
      *(undefined4 *)(*(int *)(unaff_EBP + -0x38) + 0x90) = *(undefined4 *)(unaff_EBP + -0xa4);
    }
    else if (*(int *)(unaff_EBP + -0xa4) != 0) {
      *(undefined4 *)(unaff_EBP + -0xcc) = *(undefined4 *)(unaff_EBP + -0xa4);
      *(undefined4 *)(unaff_EBP + -200) = *(undefined4 *)(unaff_EBP + -0xcc);
      if (*(int *)(unaff_EBP + -200) == 0) {
        *(undefined4 *)(unaff_EBP + -0xe8) = 0;
      }
      else {
        uVar2 = FUN_004c5b10(1);
        *(undefined4 *)(unaff_EBP + -0xe8) = uVar2;
      }
      *(undefined4 *)(unaff_EBP + -0xa4) = 0;
    }
  }
  uVar2 = FUN_004bead1();
  return uVar2;
}

