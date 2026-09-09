// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040acd2 | Name: FUN_0040acd2


void __thiscall
FUN_0040acd2(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,undefined4 param_6)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  float local_70 [2];
  int local_68;
  undefined1 local_64 [64];
  int *local_24;
  int local_20;
  int local_1c;
  int local_18;
  int *local_14;
  int local_10;
  uint local_c;
  uint local_8;
  
  FUN_00401637(param_2,param_3,param_4,param_5,param_6);
  local_8 = (**(code **)(*DAT_0092e654 + 8))();
  local_c = local_8 - *(int *)(param_1 + 900);
  if ((*(char *)(param_1 + 0x369) == '\0') && (*(int *)(param_1 + 0x388) == 0)) {
    if (*(int *)(param_1 + 0x39c) + 500U < local_8) {
      *(uint *)(param_1 + 0x39c) = local_8;
      for (local_10 = 0; local_10 < 9; local_10 = local_10 + 1) {
        if (*(int *)(*(int *)(param_1 + 0x2ec + local_10 * 4) + 0x28) == 1) {
          (**(code **)(**(int **)(param_1 + 0x2ec + local_10 * 4) + 0x60))(0);
          (**(code **)(**(int **)(param_1 + 0x310 + local_10 * 4) + 0x60))(1);
        }
        else if (*(int *)(*(int *)(param_1 + 0x310 + local_10 * 4) + 0x28) == 1) {
          (**(code **)(**(int **)(param_1 + 0x310 + local_10 * 4) + 0x60))(0);
          (**(code **)(**(int **)(param_1 + 0x2ec + local_10 * 4) + 0x60))(1);
        }
      }
    }
    if ((((*(int *)(param_1 + 0x398) + 2000U < local_8) && (DAT_0067cf38 != (int *)0x0)) &&
        (DAT_0067cf38[9] == 30000)) &&
       (local_14 = DAT_0067cf38, (char)DAT_0067cf38[0x9bad] == '\x01')) {
      (**(code **)(*DAT_0067cf38 + 0x58))(0x1901,0);
      *(uint *)(param_1 + 0x398) = local_8 + 10000;
    }
  }
  if ((*(char *)(param_1 + 0x369) == '\x01') || (*(int *)(param_1 + 0x388) != 0)) {
    if (DAT_005b892c == 2) {
      local_1c = __ftol();
    }
    else {
      local_1c = __ftol();
    }
    if (local_c < *(uint *)(param_1 + 0x388)) {
      *(uint *)(param_1 + 0x388) = *(int *)(param_1 + 0x388) - local_c;
      if (*(int *)(param_1 + 0x388) == 0) {
        *(undefined4 *)(param_1 + 0x388) = 1;
      }
    }
    else {
      for (local_20 = 0; local_20 < 9; local_20 = local_20 + 1) {
        (**(code **)(**(int **)(param_1 + 0x2ec + local_20 * 4) + 0x60))
                  (*(undefined1 *)(param_1 + local_20 + 0x360));
      }
      if ((DAT_0067cf38 != (int *)0x0) && (DAT_0067cf38[9] == 30000)) {
        if (DAT_013b71e8 != 0) {
          *(int *)(DAT_013b71e8 + 0x704) =
               *(int *)(DAT_013b71e8 + 0x704) + *(int *)(param_1 + 0x3a0) % 100000000;
        }
        local_24 = DAT_0067cf38;
        FUN_004431e4(0);
        *(undefined1 *)(param_1 + 0x369) = 0;
        *(undefined4 *)(param_1 + 0x388) = 0;
        *(uint *)(param_1 + 0x398) = local_8;
        if (0 < *(int *)(param_1 + 0x3a0)) {
          FUN_0058f078(local_64,s_X___1f__d_s_005b10f0,
                       (double)*(int *)(param_1 + 0x3a0) / (double)*(int *)(param_1 + 0x3b0),
                       *(undefined4 *)(param_1 + 0x3a0),&DAT_00a3cf78);
          FUN_00403df2(local_64,0x1194);
          (**(code **)(*(int *)local_24[0x21] + 0x88))(1,1);
          if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x151), iVar1 != 0)) {
            uVar3 = 0;
            uVar2 = 0;
            FUN_00429a6d();
            FUN_0042ad2b(uVar2,uVar3);
          }
          *(undefined4 *)(param_1 + 0x38c) = 0;
          *(undefined4 *)(param_1 + 0x390) = 0;
          *(undefined4 *)(param_1 + 0x3a0) = 0;
        }
      }
    }
    for (local_18 = 0; local_18 < 3; local_18 = local_18 + 1) {
      if ((2000000000 < *(int *)(param_1 + 0x36c + local_18 * 4)) ||
         (*(int *)(param_1 + 0x36c + local_18 * 4) < 0)) {
        *(undefined4 *)(param_1 + 0x36c + local_18 * 4) = 0;
      }
      if (((*(uint *)(param_1 + 0x388) < 300) && (local_18 == 2)) &&
         (*(char *)(param_1 + 0x369) == '\0')) {
        if (*(int *)(param_1 + 0x374) != (0x17 - *(int *)(param_1 + 0x380)) * local_1c) {
          *(int *)(param_1 + 0x374) = (0x17 - *(int *)(param_1 + 0x380)) * local_1c;
          if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x153), iVar1 != 0)) {
            uVar3 = 0;
            uVar2 = 0;
            FUN_00429a6d();
            FUN_0042ad2b(uVar2,uVar3);
          }
          goto LAB_0040b449;
        }
      }
      else {
        if (((*(uint *)(param_1 + 0x388) < 900) && (local_18 == 1)) &&
           (*(char *)(param_1 + 0x369) == '\0')) {
          if (*(int *)(param_1 + 0x370) == (0x17 - *(int *)(param_1 + 0x37c)) * local_1c)
          goto LAB_0040b0ec;
          *(int *)(param_1 + 0x370) = (0x17 - *(int *)(param_1 + 0x37c)) * local_1c;
          if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x153), iVar1 != 0)) {
            uVar3 = 0;
            uVar2 = 0;
            FUN_00429a6d();
            FUN_0042ad2b(uVar2,uVar3);
          }
        }
        else if (((*(uint *)(param_1 + 0x388) < 0x5dc) && (local_18 == 0)) &&
                (*(char *)(param_1 + 0x369) == '\0')) {
          if (*(int *)(param_1 + 0x36c) == (0x17 - *(int *)(param_1 + 0x378)) * local_1c)
          goto LAB_0040b0ec;
          *(int *)(param_1 + 0x36c) = (0x17 - *(int *)(param_1 + 0x378)) * local_1c;
          if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x153), iVar1 != 0)) {
            uVar3 = 0;
            uVar2 = 0;
            FUN_00429a6d();
            FUN_0042ad2b(uVar2,uVar3);
          }
        }
        else if (local_18 == 0) {
          uVar2 = __ftol();
          *(undefined4 *)(param_1 + 0x36c + local_18 * 4) = uVar2;
        }
        else if (local_18 == 1) {
          uVar2 = __ftol();
          *(undefined4 *)(param_1 + 0x36c + local_18 * 4) = uVar2;
        }
        else if (local_18 == 2) {
          uVar2 = __ftol();
          *(undefined4 *)(param_1 + 0x36c + local_18 * 4) = uVar2;
        }
LAB_0040b449:
        for (local_68 = 0; local_68 < 0x16; local_68 = local_68 + 1) {
          FUN_0040c110(local_70);
          if (DAT_005b892c != 2) {
            local_70[0] = ((float)*(int *)(param_1 + 0x3ac) + *(float *)(param_1 + 0x3a4)) *
                          (float)local_18;
          }
          if (local_1c * 3 <
              (*(int *)(param_1 + 0x36c + local_18 * 4) + local_1c * local_68) % (local_1c * 0x16) -
              local_1c) {
            (**(code **)(**(int **)(param_1 + 0x1e4 + local_18 * 0x58 + local_68 * 4) + 0x60))(0);
          }
          else {
            (**(code **)(**(int **)(param_1 + 0x1e4 + local_18 * 0x58 + local_68 * 4) + 0x60))(1);
          }
          (**(code **)(**(int **)(param_1 + 0x1e4 + local_18 * 0x58 + local_68 * 4) + 0x70))();
        }
      }
LAB_0040b0ec:
    }
  }
  if (((*(int *)(param_1 + 0x390) == 0) || (*(uint *)(param_1 + 0x394) < *(uint *)(param_1 + 0x390))
      ) && (*(int *)(param_1 + 0x394) != 0)) {
    if (*(int *)(param_1 + 0x390) == 0) {
      *(int *)(param_1 + 0x394) = *(int *)(param_1 + 0x394) + 1;
    }
    else {
      *(uint *)(param_1 + 0x394) =
           *(int *)(param_1 + 0x394) + 1 +
           ((uint)(*(int *)(param_1 + 0x390) - *(int *)(param_1 + 0x394)) / 500) *
           ((uint)(*(int *)(param_1 + 0x390) - *(int *)(param_1 + 0x394)) / 500);
      if (*(uint *)(param_1 + 0x390) < *(uint *)(param_1 + 0x394)) {
        *(undefined4 *)(param_1 + 0x394) = *(undefined4 *)(param_1 + 0x390);
      }
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x394) = *(undefined4 *)(param_1 + 0x390);
  }
  FUN_0040b67a();
  *(uint *)(param_1 + 900) = local_8;
  return;
}

