// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004509fa | Name: FUN_004509fa


undefined4 __thiscall FUN_004509fa(int param_1,char param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int iVar5;
  int local_13c;
  int local_11c;
  int local_fc;
  int local_dc;
  int local_bc;
  int local_9c;
  int local_7c;
  int local_5c;
  int local_3c;
  int local_1c;
  float local_18;
  int local_14;
  int local_10;
  float local_c;
  int local_8;
  
  iVar2 = FUN_0040c0f0();
  if (iVar2 != 0) {
    local_8 = 0;
    local_14 = 8;
    local_10 = 1;
    local_c = DAT_005b490c;
    local_18 = DAT_005b4910;
    if (DAT_005b892c == 2) {
      local_c = 1.0;
      local_18 = 1.0;
      local_14 = 4;
      local_10 = 2;
    }
    iVar2 = FUN_0040c0f0();
    if (iVar2 == 1) {
      local_8 = 10;
    }
    if (param_2 == '!') {
      local_1c = 0;
      do {
        if (0x17 < local_1c) {
LAB_00450b48:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            __ftol();
            __ftol();
            uVar3 = __ftol();
            uVar4 = __ftol();
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_1c * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8,uVar3);
          goto LAB_00450b48;
        }
        local_1c = local_1c + 1;
      } while( true );
    }
    if (param_2 == '@') {
      local_3c = 0;
      do {
        if (0x17 < local_3c) {
LAB_00450d6f:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            __ftol();
            __ftol();
            uVar3 = __ftol();
            uVar4 = __ftol();
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8 + 1,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_3c * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8 + 1,uVar3);
          goto LAB_00450d6f;
        }
        local_3c = local_3c + 1;
      } while( true );
    }
    if (param_2 == '#') {
      local_5c = 0;
      do {
        if (0x17 < local_5c) {
LAB_00450f99:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            __ftol();
            __ftol();
            uVar3 = __ftol();
            uVar4 = __ftol();
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8 + 2,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_5c * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8 + 2,uVar3);
          goto LAB_00450f99;
        }
        local_5c = local_5c + 1;
      } while( true );
    }
    if (param_2 == '$') {
      local_7c = 0;
      do {
        if (0x17 < local_7c) {
LAB_004511c3:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            __ftol();
            __ftol();
            uVar3 = __ftol();
            uVar4 = __ftol();
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8 + 3,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_7c * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8 + 3,uVar3);
          goto LAB_004511c3;
        }
        local_7c = local_7c + 1;
      } while( true );
    }
    if (param_2 == '%') {
      local_9c = 0;
      do {
        if (0x17 < local_9c) {
LAB_00451438:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            __ftol();
            __ftol();
            uVar3 = __ftol();
            uVar4 = __ftol();
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8 + 4,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_9c * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8 + 4,uVar3);
          goto LAB_00451438;
        }
        local_9c = local_9c + 1;
      } while( true );
    }
    if (param_2 == '^') {
      local_bc = 0;
      do {
        if (0x17 < local_bc) {
LAB_004516ad:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            __ftol();
            __ftol();
            uVar3 = __ftol();
            uVar4 = __ftol();
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8 + 5,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_bc * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8 + 5,uVar3);
          goto LAB_004516ad;
        }
        local_bc = local_bc + 1;
      } while( true );
    }
    if (param_2 == '&') {
      local_dc = 0;
      do {
        if (0x17 < local_dc) {
LAB_00451922:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            __ftol();
            __ftol();
            uVar3 = __ftol();
            uVar4 = __ftol();
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8 + 6,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_dc * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8 + 6,uVar3);
          goto LAB_00451922;
        }
        local_dc = local_dc + 1;
      } while( true );
    }
    if (param_2 == '*') {
      local_fc = 0;
      do {
        if (0x17 < local_fc) {
LAB_00451b97:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            __ftol();
            __ftol();
            uVar3 = __ftol();
            uVar4 = __ftol();
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8 + 7,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_fc * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8 + 7,uVar3);
          goto LAB_00451b97;
        }
        local_fc = local_fc + 1;
      } while( true );
    }
    if (param_2 == '(') {
      local_11c = 0;
      do {
        if (0x17 < local_11c) {
LAB_00451e0c:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            __ftol();
            __ftol();
            uVar3 = __ftol();
            uVar4 = __ftol();
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8 + 8,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_11c * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8 + 8,uVar3);
          goto LAB_00451e0c;
        }
        local_11c = local_11c + 1;
      } while( true );
    }
    if (param_2 == ')') {
      local_13c = 0;
      do {
        if (0x17 < local_13c) {
LAB_00452081:
          uVar3 = __ftol(*(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x68),
                         *(undefined4 *)(*(int *)(param_1 + 0x28714) + 0x6c),
                         local_c * *(float *)(*(int *)(param_1 + 0x28714) + 0x70),
                         local_18 * *(float *)(*(int *)(param_1 + 0x28714) + 0x74));
          uVar3 = __ftol(uVar3);
          iVar2 = FUN_00401000(uVar3);
          if (iVar2 == 1) {
            iVar2 = __ftol();
            iVar5 = __ftol();
            local_14 = local_14 * iVar2;
            uVar3 = __ftol();
            uVar4 = __ftol(local_10 * iVar5,local_14);
            iVar2 = (**(code **)(**(int **)(param_1 + 0x28714) + 0xb8))(uVar3,uVar4);
            if (iVar2 != 0) {
              FUN_0044e494(local_8 + 9,iVar2);
            }
          }
          return 1;
        }
        piVar1 = *(int **)(param_1 + 0x279bc + local_13c * 4);
        uVar3 = __ftol(piVar1[0x1a],piVar1[0x1b],local_c * (float)piVar1[0x1c],
                       local_18 * (float)piVar1[0x1d]);
        uVar3 = __ftol(uVar3);
        iVar2 = FUN_00401000(uVar3);
        if (iVar2 == 1) {
          uVar3 = (**(code **)(*piVar1 + 0xb4))(0,0);
          FUN_0044e494(local_8 + 9,uVar3);
          goto LAB_00452081;
        }
        local_13c = local_13c + 1;
      } while( true );
    }
  }
  return 0;
}

