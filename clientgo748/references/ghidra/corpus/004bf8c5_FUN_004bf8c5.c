// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bf8c5 | Name: FUN_004bf8c5


/* WARNING (jumptable): Unable to track spacebase fully for stack */

undefined4 __fastcall FUN_004bf8c5(int param_1)

{
  size_t sVar1;
  undefined4 uVar2;
  void *pvVar3;
  int iVar4;
  int unaff_EBP;
  
  if (*(short *)(param_1 + 0x356) != 1) {
    if (*(short *)(*(int *)(unaff_EBP + -0xd0) + 0x356) == 2) {
      iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0101_005bb114,0xb);
      if ((iVar4 == 0) ||
         (iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0201_005bb120,0xb), iVar4 == 0)) {
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x89 + sVar1) = 0x31;
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x86 + sVar1) = 0x33;
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x85 + sVar1) = 0x30;
      }
      else {
        iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch01_005bb12c,9);
        if ((iVar4 == 0) ||
           (iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch02_005bb138,9), iVar4 == 0)) {
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x89 + sVar1) = 0x31;
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x86 + sVar1) = 0x33;
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x85 + sVar1) = 0x31;
        }
      }
      iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch0101_005bb144,0xb);
      if ((iVar4 == 0) ||
         (iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch0201_005bb150,0xb), iVar4 == 0)) {
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x39 + sVar1) = 0x31;
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x36 + sVar1) = 0x33;
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x35 + sVar1) = 0x30;
      }
      else {
        iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch01_005bb15c,9);
        if ((iVar4 == 0) ||
           (iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch02_005bb168,9), iVar4 == 0)) {
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x39 + sVar1) = 0x31;
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x36 + sVar1) = 0x33;
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x35 + sVar1) = 0x31;
        }
      }
    }
    else if (*(short *)(*(int *)(unaff_EBP + -0xd0) + 0x356) == 3) {
      iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0101_005bb174,0xb);
      if ((iVar4 == 0) ||
         (iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0201_005bb180,0xb), iVar4 == 0)) {
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x89 + sVar1) = 0x31;
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x86 + sVar1) = 0x33;
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x85 + sVar1) = 0x30;
      }
      else {
        iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch01_005bb18c,9);
        if ((iVar4 == 0) ||
           (iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch02_005bb198,9), iVar4 == 0)) {
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x89 + sVar1) = 0x32;
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x86 + sVar1) = 0x33;
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x85 + sVar1) = 0x37;
        }
      }
      iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch0101_005bb1a4,0xb);
      if ((iVar4 == 0) ||
         (iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch0201_005bb1b0,0xb), iVar4 == 0)) {
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x39 + sVar1) = 0x32;
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x36 + sVar1) = 0x31;
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x35 + sVar1) = 0x33;
      }
      else {
        iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch01_005bb1bc,9);
        if ((iVar4 == 0) ||
           (iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch02_005bb1c8,9), iVar4 == 0)) {
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x39 + sVar1) = 0x32;
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x36 + sVar1) = 0x33;
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x35 + sVar1) = 0x37;
        }
      }
    }
    else if (*(short *)(*(int *)(unaff_EBP + -0xd0) + 0x356) == 4) {
      iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0101_005bb1d4,0xb);
      if ((iVar4 == 0) ||
         (iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0201_005bb1e0,0xb), iVar4 == 0)) {
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x89 + sVar1) = 0x31;
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x86 + sVar1) = 0x33;
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x85 + sVar1) = 0x37;
      }
      else {
        iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0102_005bb1ec,0xb);
        if ((iVar4 == 0) ||
           (iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0201_005bb1f8,0xb), iVar4 == 0)) {
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x89 + sVar1) = 0x31;
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x87 + sVar1) = 0x31;
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x86 + sVar1) = 0x33;
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x85 + sVar1) = 0x37;
        }
        else {
          iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch01_005bb204,9);
          if ((iVar4 == 0) ||
             (iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch02_005bb210,9), iVar4 == 0)) {
            sVar1 = _strlen((char *)(unaff_EBP + -0x80));
            *(undefined1 *)(unaff_EBP + -0x89 + sVar1) = 0x31;
            sVar1 = _strlen((char *)(unaff_EBP + -0x80));
            *(undefined1 *)(unaff_EBP + -0x86 + sVar1) = 0x33;
            sVar1 = _strlen((char *)(unaff_EBP + -0x80));
            *(undefined1 *)(unaff_EBP + -0x85 + sVar1) = 0x37;
          }
        }
      }
      iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch0101_005bb21c,0xb);
      if ((iVar4 == 0) ||
         (iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch0201_005bb228,0xb), iVar4 == 0)) {
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x39 + sVar1) = 0x31;
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x36 + sVar1) = 0x33;
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x35 + sVar1) = 0x37;
      }
      else {
        iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch01_005bb234,9);
        if ((iVar4 == 0) ||
           (iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch02_005bb240,9), iVar4 == 0)) {
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x39 + sVar1) = 0x31;
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x36 + sVar1) = 0x33;
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x35 + sVar1) = 0x37;
        }
      }
    }
    else if (*(short *)(*(int *)(unaff_EBP + -0xd0) + 0x356) == 5) {
      iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0101_005bb24c,0xb);
      if ((iVar4 == 0) ||
         (iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch0201_005bb258,0xb), iVar4 == 0)) {
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x89 + sVar1) = 0x32;
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x86 + sVar1) = 0x31;
        sVar1 = _strlen((char *)(unaff_EBP + -0x80));
        *(undefined1 *)(unaff_EBP + -0x85 + sVar1) = 0x37;
      }
      else {
        iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch01_005bb264,9);
        if ((iVar4 == 0) ||
           (iVar4 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch02_005bb270,9), iVar4 == 0)) {
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x89 + sVar1) = 0x32;
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x86 + sVar1) = 0x31;
          sVar1 = _strlen((char *)(unaff_EBP + -0x80));
          *(undefined1 *)(unaff_EBP + -0x85 + sVar1) = 0x37;
        }
      }
      iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch0101_005bb27c,0xb);
      if ((iVar4 == 0) ||
         (iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch0201_005bb288,0xb), iVar4 == 0)) {
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x39 + sVar1) = 0x32;
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x36 + sVar1) = 0x31;
        sVar1 = _strlen((char *)(unaff_EBP + -0x30));
        *(undefined1 *)(unaff_EBP + -0x35 + sVar1) = 0x37;
      }
      else {
        iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch01_005bb294,9);
        if ((iVar4 == 0) ||
           (iVar4 = _strncmp((char *)(unaff_EBP + -0x30),s_mesh_ch02_005bb2a0,9), iVar4 == 0)) {
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x39 + sVar1) = 0x32;
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x36 + sVar1) = 0x31;
          sVar1 = _strlen((char *)(unaff_EBP + -0x30));
          *(undefined1 *)(unaff_EBP + -0x35 + sVar1) = 0x37;
        }
      }
    }
  }
  if (((0x59 < **(ushort **)(unaff_EBP + -0x10)) && (*(int *)(unaff_EBP + -0x3c) != 0)) &&
     (*(short *)(*(int *)(unaff_EBP + -0x10) + *(int *)(unaff_EBP + -0x3c) * 4) == 0)) {
    uVar2 = FUN_004bead1();
    return uVar2;
  }
  *(undefined4 *)(unaff_EBP + -0xa4) = 0;
  pvVar3 = operator_new(0xd4);
  *(void **)(unaff_EBP + -0xc4) = pvVar3;
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
    iVar4 = FUN_004c097c(unaff_EBP + -0x30);
    if (iVar4 == 1) {
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

