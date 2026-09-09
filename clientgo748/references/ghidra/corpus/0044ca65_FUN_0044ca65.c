// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044ca65 | Name: FUN_0044ca65


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0044ca65(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_38;
  int local_30;
  int local_2c;
  int local_10;
  int local_c;
  
  if (*(int *)(param_1 + 0x27d98) != 0) {
    DAT_005b12bc = 0xffff;
    iVar2 = *(int *)(*(int *)(param_1 + 0x27d98) + 0x28);
    if (iVar2 == 0) {
      for (local_c = 0; local_c < 0x100; local_c = local_c + 1) {
        if (_DAT_005a3660 <= _DAT_005bde1c) {
          (**(code **)(**(int **)(param_1 + 0x281d4 + local_c * 4) + 0x60))(1);
          (**(code **)(**(int **)(param_1 + 0x27dd4 + local_c * 4) + 0x60))(0);
        }
        else {
          (**(code **)(**(int **)(param_1 + 0x281d4 + local_c * 4) + 0x60))(1);
          (**(code **)(**(int **)(param_1 + 0x27dd4 + local_c * 4) + 0x60))(0);
        }
      }
    }
    else {
      for (local_10 = 0; local_10 < 0x100; local_10 = local_10 + 1) {
        if (_DAT_005a3660 <= _DAT_005bde1c) {
          (**(code **)(**(int **)(param_1 + 0x281d4 + local_10 * 4) + 0x60))(0);
          (**(code **)(**(int **)(param_1 + 0x27dd4 + local_10 * 4) + 0x60))(0);
        }
        else {
          (**(code **)(**(int **)(param_1 + 0x281d4 + local_10 * 4) + 0x60))(1);
          (**(code **)(**(int **)(param_1 + 0x27dd4 + local_10 * 4) + 0x60))(1);
        }
      }
    }
    if (DAT_005b892c == 2) {
      if (iVar2 == 1) {
        piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x3229);
        if (piVar1 != (int *)0x0) {
          (**(code **)(*piVar1 + 0x60))(0);
        }
        if (*(int *)(param_1 + 0x27dcc) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27dcc) + 0x60))(0);
        }
        if (*(int *)(param_1 + 0x27dc8) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27dc8) + 0x60))(1);
        }
        if (*(int *)(param_1 + 0x27dc4) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27dc4) + 0x60))(0);
        }
        if (_DAT_005a3660 <= _DAT_005bde1c) {
          if (*(int *)(param_1 + 0x27d98) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27d98) + 0x70))
                      ((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                       *(float *)(*(int *)(param_1 + 0x27d98) + 0x54),0);
            (**(code **)(**(int **)(param_1 + 0x27d98) + 0x60))(0);
          }
          if (*(int *)(param_1 + 0x27d9c) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x60))(0);
          }
          for (local_30 = 0; local_30 < 8; local_30 = local_30 + 1) {
            if (*(int *)(param_1 + 0x27da0 + local_30 * 4) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27da0 + local_30 * 4) + 0x60))(0);
            }
          }
        }
        else {
          _DAT_005bde1c = 1.5;
          if (*(int *)(param_1 + 0x27d98) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27d98) + 0x74))(0x43c80000,0x43c80000);
            (**(code **)(**(int **)(param_1 + 0x27d98) + 0x70))
                      ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                       *(float *)(*(int *)(param_1 + 0x27d98) + 0x54) * _DAT_005a34a0,
                       (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                       *(float *)(*(int *)(param_1 + 0x27d98) + 0x58) * _DAT_005a34a0);
          }
          if (*(int *)(param_1 + 0x27d94) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27d94) + 0x70))
                      (_DAT_005a34a0 * 400.0,_DAT_005a34a0 * 400.0);
          }
          if (*(int *)(param_1 + 0x27d9c) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x60))(0);
            if (*(int *)(param_1 + 0x27da0) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27da0) + 0x70))
                        (*(undefined4 *)(*(int *)(param_1 + 0x27da0) + 0x4c),
                         *(float *)(*(int *)(param_1 + 0x27da0) + 0x50) - _DAT_005a3660);
            }
            if (*(int *)(param_1 + 0x27da4) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27da4) + 0x70))
                        (*(undefined4 *)(*(int *)(param_1 + 0x27da4) + 0x4c),
                         *(float *)(*(int *)(param_1 + 0x27da4) + 0x50) - _DAT_005a3660);
            }
            if (*(int *)(param_1 + 0x27dac) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27dac) + 0x70))
                        (*(undefined4 *)(*(int *)(param_1 + 0x27dac) + 0x4c),
                         *(float *)(*(int *)(param_1 + 0x27dac) + 0x50) - _DAT_005a3660);
            }
            if (*(int *)(param_1 + 0x27da8) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27da8) + 0x70))
                        ((400.0 - _DAT_005a366c) + _DAT_005a3660,
                         *(float *)(*(int *)(param_1 + 0x27da8) + 0x50) - _DAT_005a3660);
            }
            if (*(int *)(param_1 + 0x27db0) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27db0) + 0x70))
                        ((400.0 - _DAT_005a366c) + _DAT_005a3660,
                         *(float *)(*(int *)(param_1 + 0x27db0) + 0x50) - _DAT_005a3660);
            }
            if (*(int *)(param_1 + 0x27db4) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27db4) + 0x70))
                        (0,(400.0 - _DAT_005a366c) - _DAT_005a3660);
            }
            if (*(int *)(param_1 + 0x27db8) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27db8) + 0x70))
                        (0x41000000,(400.0 - _DAT_005a366c) - _DAT_005a3660);
            }
            if (*(int *)(param_1 + 0x27dbc) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27dbc) + 0x70))
                        ((400.0 - _DAT_005a366c) + _DAT_005a3660,
                         (400.0 - _DAT_005a366c) - _DAT_005a3660);
            }
            if (*(int *)(param_1 + 0x27da4) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27da4) + 0x74))
                        (400.0 - _DAT_005a3970,*(undefined4 *)(*(int *)(param_1 + 0x27da4) + 0x58));
            }
            if (*(int *)(param_1 + 0x27dac) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27dac) + 0x74))
                        (*(undefined4 *)(*(int *)(param_1 + 0x27dac) + 0x54),400.0 - _DAT_005a3970);
            }
            if (*(int *)(param_1 + 0x27db0) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27db0) + 0x74))
                        (*(undefined4 *)(*(int *)(param_1 + 0x27db0) + 0x54),400.0 - _DAT_005a3970);
            }
            if (*(int *)(param_1 + 0x27db8) != 0) {
              (**(code **)(**(int **)(param_1 + 0x27db8) + 0x74))
                        (400.0 - _DAT_005a3970,*(undefined4 *)(*(int *)(param_1 + 0x27db8) + 0x58));
            }
            for (local_2c = 0; local_2c < 8; local_2c = local_2c + 1) {
              if (*(int *)(param_1 + 0x27da0 + local_2c * 4) != 0) {
                (**(code **)(**(int **)(param_1 + 0x27da0 + local_2c * 4) + 0x60))(1);
              }
            }
          }
          if (*(int *)(param_1 + 0x27dc0) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27dc0) + 0x70))(400.0 - _DAT_005a4368,0x43c80000);
            (**(code **)(**(int **)(param_1 + 0x27dc0) + 0x74))
                      (0x43090000,*(undefined4 *)(*(int *)(param_1 + 0x27dc0) + 0x58));
          }
          if (*(int *)(param_1 + 0x27b1c) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27b1c) + 0x70))
                      (400.0 - _DAT_005a4364,400.0 - _DAT_005a4314);
          }
          if (*(int *)(param_1 + 0x27dc4) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27dc4) + 0x70))
                      (400.0 - _DAT_005a4360,_DAT_005a4104 + 400.0);
          }
          if (*(int *)(param_1 + 0x27dc8) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27dc8) + 0x70))(400.0 - _DAT_005a3c68,0);
          }
          if (*(int *)(param_1 + 0x27dcc) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27dcc) + 0x70))(400.0 - _DAT_005a435c,0x43c80000);
          }
          if (*(int *)(param_1 + 0x27dc4) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x27dc4) + 0x1e8) = 1;
          }
          if (*(int *)(param_1 + 0x27dc8) != 0) {
            *(undefined4 *)(*(int *)(param_1 + 0x27dc8) + 0x1e8) = 0;
          }
        }
      }
      else {
        _DAT_005bde1c = 0.6;
        piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x3229);
        if (piVar1 != (int *)0x0) {
          (**(code **)(*piVar1 + 0x60))(1);
        }
        if (*(int *)(param_1 + 0x27dc8) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27dc8) + 0x60))(0);
        }
        if (*(int *)(param_1 + 0x27dc4) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27dc4) + 0x60))(1);
          (**(code **)(**(int **)(param_1 + 0x27dc4) + 0x70))(0x42ec0000,0xc1900000);
        }
        if (*(int *)(param_1 + 0x27d98) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27d98) + 0x74))(0x43090000,0x43090000);
          (**(code **)(**(int **)(param_1 + 0x27d98) + 0x70))
                    (((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                     *(float *)(*(int *)(param_1 + 0x27d98) + 0x54)) - _DAT_005a365c,0x41b80000);
        }
        (**(code **)(**(int **)(param_1 + 0x27dcc) + 0x60))(1);
        if (*(int *)(param_1 + 0x27dcc) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27dcc) + 0x70))(0,0xc1980000);
        }
        if (*(int *)(param_1 + 0x27d9c) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x70))(0,0);
          (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x74))(0x43090000,0x43090000);
          (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x60))(iVar2 == 0);
        }
        if (*(int *)(param_1 + 0x27d98) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27d98) + 0x60))(iVar2 == 0);
        }
        if (*(int *)(param_1 + 0x27d94) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27d94) + 0x70))(0x42880000,0x42880000);
        }
        for (local_38 = 0; local_38 < 8; local_38 = local_38 + 1) {
          if (*(int *)(param_1 + 0x27da0 + local_38 * 4) != 0) {
            (**(code **)(**(int **)(param_1 + 0x27da0 + local_38 * 4) + 0x60))(0);
          }
        }
        if (*(int *)(param_1 + 0x27dc0) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27dc0) + 0x70))(0,0x43090000);
          (**(code **)(**(int **)(param_1 + 0x27dc0) + 0x74))
                    (0x43090000,*(undefined4 *)(*(int *)(param_1 + 0x27dc0) + 0x58));
        }
        if (*(int *)(param_1 + 0x27b1c) != 0) {
          (**(code **)(**(int **)(param_1 + 0x27b1c) + 0x70))(0x41500000,0x42f00000);
        }
        if ((*(int *)(param_1 + 0x27dc4) != 0) && (*(int *)(param_1 + 0x27dc8) != 0)) {
          (**(code **)(**(int **)(param_1 + 0x27dc8) + 0x70))(0x42ee0000,0x430e0000);
          *(undefined4 *)(*(int *)(param_1 + 0x27dc8) + 0x1e8) = 1;
        }
      }
      iVar2 = *(int *)(*(int *)(param_1 + 0x27d98) + 0x28);
      if ((iVar2 == 1) && (*(int *)(param_1 + 0x40) != 0)) {
        (**(code **)(**(int **)(param_1 + 0x40) + 0x28))();
      }
      piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x128);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 0x8c))(*(undefined4 *)(*(int *)(param_1 + 0x27d98) + 0x28));
      }
      (**(code **)(**(int **)(param_1 + 0x27b1c) + 0x60))(iVar2);
    }
    else {
      if (iVar2 == 1) {
        if (_DAT_005a3660 <= _DAT_005bde1c) {
          (**(code **)(**(int **)(param_1 + 0x27d98) + 0x60))(0);
          (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x60))(0);
        }
        else {
          _DAT_005bde1c = 1.5;
          (**(code **)(**(int **)(param_1 + 0x27d98) + 0x70))(0x43480000,0x42200000);
          (**(code **)(**(int **)(param_1 + 0x27d98) + 0x74))(0x43c80000,0x43c80000);
          (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x70))(0x43440000,0x42100000);
          (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x74))(0x43cc0000,0x43cc0000);
          (**(code **)(**(int **)(param_1 + 0x27d94) + 0x70))(0x43440000,0x43380000);
        }
      }
      else {
        _DAT_005bde1c = 0.6;
        (**(code **)(**(int **)(param_1 + 0x27d98) + 0x70))(0x441f0000,0x40800000);
        (**(code **)(**(int **)(param_1 + 0x27d98) + 0x74))(0x43200000,0x43200000);
        (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x70))(0x441e0000,0);
        (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x74))(0x43280000,0x43280000);
        (**(code **)(**(int **)(param_1 + 0x27d98) + 0x60))(iVar2 == 0);
        (**(code **)(**(int **)(param_1 + 0x27d9c) + 0x60))(iVar2 == 0);
        (**(code **)(**(int **)(param_1 + 0x27d94) + 0x70))(0x42980000,0x42800000);
      }
      iVar2 = *(int *)(*(int *)(param_1 + 0x27d98) + 0x28);
      if ((iVar2 == 1) && (*(int *)(param_1 + 0x40) != 0)) {
        (**(code **)(**(int **)(param_1 + 0x40) + 0x28))();
      }
      piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x128);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 0x8c))(*(undefined4 *)(*(int *)(param_1 + 0x27d98) + 0x28));
      }
      (**(code **)(**(int **)(param_1 + 0x27b1c) + 0x60))(iVar2);
    }
    if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x33), iVar2 != 0)) {
      uVar4 = 0;
      uVar3 = 0;
      FUN_00429a6d(0x33);
      FUN_0042ad2b(uVar3,uVar4);
    }
  }
  return;
}

