// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b00e4 | Name: FUN_004b00e4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004b00e4(int param_1)

{
  int iVar1;
  void *pvVar2;
  int iVar3;
  int iVar4;
  int local_2e8;
  int local_2e4;
  int local_2dc;
  int local_2d8;
  int local_2d4;
  int local_2c4;
  int local_2c0;
  int local_2b0;
  int local_2ac;
  int local_29c;
  int local_298;
  int local_294;
  int local_290;
  int local_280;
  int local_27c;
  int local_278;
  int local_274;
  int local_270;
  int local_26c;
  int local_268;
  int local_264;
  int local_260;
  uint local_250;
  int local_24c;
  size_t local_248;
  int local_244;
  size_t local_240;
  uint local_23c;
  int local_238;
  int local_234;
  undefined4 local_230;
  int local_22c;
  size_t local_228;
  int local_224;
  undefined4 local_220;
  int local_21c;
  undefined4 local_218;
  int local_214;
  int local_210;
  undefined1 local_20c [256];
  undefined4 local_10c;
  int local_108;
  undefined1 local_104 [128];
  undefined1 local_84 [128];
  
  local_108 = 0;
  _memset(&DAT_0065a418,0,90000);
  FUN_0058f078(local_84,s_Mesh_BoneAni4_txt_005b99dc);
  local_108 = FUN_0058f716(local_84,&DAT_005b99f0);
  if (local_108 != 0) {
    local_21c = 0;
    local_21c = FUN_0058f716(s_mesh_ValidIndex_bin_005b99f8,&DAT_005b99f4);
    if (local_21c != 0) {
      FUN_00590d68(param_1 + 0x577c8,0x122a0,1,local_21c);
      FUN_0058f62c(local_21c);
    }
    local_210 = 0;
    local_220 = 0;
    local_218 = 0;
    local_10c = 0;
    for (local_214 = 0; local_214 < 100; local_214 = local_214 + 1) {
      local_234 = 0;
      iVar1 = FUN_00590a68(local_108,s__d__d__d__s_005b9a0c,&local_210,&local_220,&local_218,
                           local_20c);
      if (iVar1 == -1) break;
      *(undefined4 *)(&DAT_0065a4a4 + local_210 * 900) = local_220;
      *(undefined4 *)(&DAT_0065a49c + local_210 * 900) = local_218;
      FUN_0058f078(&DAT_0065a418 + local_210 * 900,&DAT_005b9a1c,local_20c);
      FUN_0058f078(local_104,s__s_bon_005b9a20,&DAT_0065a418 + local_210 * 900);
      local_238 = FUN_0058fb07(local_104,0x8000,0);
      local_240 = FUN_0058f83f(local_238);
      pvVar2 = _malloc(local_240);
      *(void **)(&DAT_0065a4a0 + local_210 * 900) = pvVar2;
      FUN_0058f8c9(local_238,*(undefined4 *)(&DAT_0065a4a0 + local_210 * 900),local_240);
      FUN_0058f75f(local_238);
      *(int *)(&DAT_0065a498 + local_210 * 900) =
           (int)(local_240 + ((int)local_240 >> 0x1f & 7U)) >> 3;
      local_23c = 0;
      local_230 = 0;
      for (local_22c = 0; local_22c < *(int *)(&DAT_0065a4a4 + local_210 * 900);
          local_22c = local_22c + 1) {
        FUN_0058f078(local_104,s__s_04d_ani_005b9a28,&DAT_0065a418 + local_210 * 900,
                     *(int *)(param_1 + 0x577c8 + local_210 * 0x2e8 + local_22c * 4) + 1);
        local_238 = FUN_0058fb07(local_104,0x8000,0);
        if (-1 < local_238) {
          local_244 = 0;
          FUN_0058f8c9(local_238,&local_244,4);
          FUN_0058f8c9(local_238,&DAT_0065a790 + local_210 * 900,4);
          FUN_0058f75f(local_238);
          local_234 = local_234 + local_244;
          local_23c = local_23c + 1;
        }
      }
      _DAT_006703ac = local_234;
      local_228 = local_234 * 0x40 * *(int *)(&DAT_0065a790 + local_210 * 900);
      pvVar2 = _malloc(local_228);
      *(void **)(&DAT_0065a794 + local_210 * 900) = pvVar2;
      if ((local_210 == 1) || (local_210 == 0)) {
        local_248 = local_234 * 0x10 * *(int *)(&DAT_0065a790 + local_210 * 900);
        pvVar2 = _malloc(local_248);
        *(void **)(&DAT_0065a798 + local_210 * 900) = pvVar2;
      }
      local_230 = 0;
      local_23c = 0;
      local_224 = 0;
      for (local_22c = 0; local_22c < *(int *)(&DAT_0065a4a4 + local_210 * 900);
          local_22c = local_22c + 1) {
        FUN_0058f078(local_104,s__s_04d_ani_005b9a34,&DAT_0065a418 + local_210 * 900,
                     *(int *)(param_1 + 0x577c8 + local_210 * 0x2e8 + local_22c * 4) + 1);
        local_238 = FUN_0058fb07(local_104,0x8000,0);
        if (-1 < local_238) {
          FUN_0058f8c9(local_238,&local_24c,4);
          FUN_0058f8c9(local_238,&DAT_0065a790 + local_210 * 900,4);
          *(int *)(&DAT_0065a4a8 + local_23c * 4 + local_210 * 900) = local_24c;
          FUN_0058f8c9(local_238,*(int *)(&DAT_0065a794 + local_210 * 900) + local_224 * 0x40,
                       local_24c * 0x40 * *(int *)(&DAT_0065a790 + local_210 * 900));
          if ((local_210 == 1) || (local_210 == 0)) {
            for (local_250 = 0;
                local_250 < (uint)(local_24c * *(int *)(&DAT_0065a790 + local_210 * 900));
                local_250 = local_250 + 1) {
              thunk_FUN_00562ad1(*(int *)(&DAT_0065a798 + local_210 * 900) +
                                 (local_224 + local_250) * 0x10,
                                 *(int *)(&DAT_0065a794 + local_210 * 900) +
                                 (local_224 + local_250) * 0x40);
            }
          }
          local_224 = local_224 + local_24c * *(int *)(&DAT_0065a790 + local_210 * 900);
          FUN_0058f75f(local_238);
          if ((local_210 == 0) || (local_210 == 1)) {
            iVar3 = *(int *)(param_1 + 0x577c8 + local_210 * 0x2e8 + local_22c * 4) + 1;
            iVar1 = iVar3 / 100 + -1;
            local_274 = iVar3 % 100;
            iVar4 = local_274 + -1;
            *(undefined2 *)(&DAT_005d0508 + iVar4 * 2 + iVar1 * 0x70 + local_210 * 0x1a40) =
                 (undefined2)local_23c;
            if (local_210 == 0) {
              if ((3 < iVar4) && (local_260 = local_274, iVar4 < 9)) {
                for (; local_260 < 10; local_260 = local_260 + 1) {
                  *(undefined2 *)(&DAT_005d0508 + local_260 * 2 + iVar1 * 0x70) =
                       *(undefined2 *)(&DAT_005d0508 + iVar4 * 2 + iVar1 * 0x70);
                }
              }
              if ((0x18 < iVar4) && (local_264 = local_274, iVar4 < 0x1d)) {
                for (; local_264 < 0x1e; local_264 = local_264 + 1) {
                  *(undefined2 *)(&DAT_005d0508 + local_264 * 2 + iVar1 * 0x70) =
                       *(undefined2 *)(&DAT_005d0508 + iVar4 * 2 + iVar1 * 0x70);
                }
              }
              if (0xb < iVar1) {
                *(undefined2 *)(&DAT_005d051e + iVar1 * 0x70) = _DAT_005d09ee;
                *(undefined2 *)(&DAT_005d0520 + iVar1 * 0x70) = _DAT_005d09f0;
                *(undefined2 *)(&DAT_005d0524 + iVar1 * 0x70) = _DAT_005d09f4;
              }
              if (iVar3 == 0x8a) {
                for (local_268 = 1; local_268 < 0x3c; local_268 = local_268 + 1) {
                  for (local_26c = 0; local_26c < 0x38; local_26c = local_26c + 1) {
                    *(undefined2 *)(&DAT_005d0508 + local_26c * 2 + local_268 * 0x70) =
                         *(undefined2 *)(&DAT_005d0508 + local_26c * 2);
                  }
                }
              }
            }
            else if (local_210 == 1) {
              if ((3 < iVar4) && (local_270 = local_274, iVar4 < 9)) {
                for (; local_270 < 10; local_270 = local_270 + 1) {
                  *(undefined2 *)(iVar1 * 0x70 + 0x5d1f48 + local_270 * 2) =
                       *(undefined2 *)(iVar1 * 0x70 + 0x5d1f48 + iVar4 * 2);
                }
              }
              if ((0x18 < iVar4) && (iVar4 < 0x1d)) {
                for (; local_274 < 0x1e; local_274 = local_274 + 1) {
                  *(undefined2 *)(iVar1 * 0x70 + 0x5d1f48 + local_274 * 2) =
                       *(undefined2 *)(iVar1 * 0x70 + 0x5d1f48 + iVar4 * 2);
                }
              }
              if ((iVar1 == 2) && (iVar4 == 4)) {
                for (local_278 = 0; local_278 < 4; local_278 = local_278 + 1) {
                  *(undefined2 *)(local_278 * 2 + 0x5d2028) =
                       *(undefined2 *)(local_278 * 2 + 0x5d1fb8);
                }
              }
              if (iVar3 == 0x89) {
                for (local_27c = 1; local_27c < 0x3c; local_27c = local_27c + 1) {
                  for (local_280 = 0; local_280 < 0x38; local_280 = local_280 + 1) {
                    *(undefined2 *)(local_27c * 0x70 + 0x5d1f48 + local_280 * 2) =
                         *(undefined2 *)(local_280 * 2 + 0x5d1f48);
                  }
                }
              }
            }
          }
          else if (local_210 == 2) {
            iVar1 = *(int *)(param_1 + 0x57d98 + local_22c * 4) + 1;
            iVar3 = iVar1 / 100 + -1;
            local_290 = iVar1 % 100;
            *(undefined2 *)(iVar3 * 0x70 + 0x5d3988 + (local_290 + -1) * 2) = (undefined2)local_23c;
            if (local_290 + -1 == 3) {
              for (; local_290 < 5; local_290 = local_290 + 1) {
                *(undefined2 *)(iVar3 * 0x70 + 0x5d3988 + local_290 * 2) =
                     *(undefined2 *)(iVar3 * 0x70 + 0x5d398e);
              }
            }
            if (iVar1 == 0x133) {
              for (local_294 = 0; local_294 < 0x38; local_294 = local_294 + 1) {
                *(undefined2 *)(local_294 * 2 + 0x5d3ad8) =
                     *(undefined2 *)(local_294 * 2 + 0x5d3a68);
              }
            }
            if (iVar1 == 0x6b) {
              for (local_298 = 1; local_298 < 0x3c; local_298 = local_298 + 1) {
                for (local_29c = 0; local_29c < 0x38; local_29c = local_29c + 1) {
                  *(undefined2 *)(local_298 * 0x70 + 0x5d3988 + local_29c * 2) =
                       *(undefined2 *)(local_29c * 2 + 0x5d3988);
                }
              }
            }
          }
          else if (local_210 == 5) {
            iVar1 = *(int *)(param_1 + 0x58650 + local_22c * 4) + 1;
            *(undefined2 *)((iVar1 / 100 + -1) * 0x70 + 0x5d8848 + (iVar1 % 100 + -1) * 2) =
                 (undefined2)local_23c;
            if (iVar1 == 0x6f) {
              for (local_2ac = 1; local_2ac < 0x3c; local_2ac = local_2ac + 1) {
                for (local_2b0 = 0; local_2b0 < 0x38; local_2b0 = local_2b0 + 1) {
                  *(undefined2 *)(local_2ac * 0x70 + 0x5d8848 + local_2b0 * 2) =
                       *(undefined2 *)(local_2b0 * 2 + 0x5d8848);
                }
              }
            }
          }
          else if (local_210 == 3) {
            iVar1 = *(int *)(param_1 + 0x58080 + local_22c * 4) + 1;
            *(undefined2 *)((iVar1 / 100 + -1) * 0x70 + 0x5d53c8 + (iVar1 % 100 + -1) * 2) =
                 (undefined2)local_23c;
            if (iVar1 == 0x6c) {
              for (local_2c0 = 1; local_2c0 < 0x3c; local_2c0 = local_2c0 + 1) {
                for (local_2c4 = 0; local_2c4 < 0x38; local_2c4 = local_2c4 + 1) {
                  *(undefined2 *)(local_2c0 * 0x70 + 0x5d53c8 + local_2c4 * 2) =
                       *(undefined2 *)(local_2c4 * 2 + 0x5d53c8);
                }
              }
            }
          }
          else if (local_210 == 4) {
            iVar3 = *(int *)(param_1 + 0x58368 + local_22c * 4) + 1;
            iVar1 = iVar3 / 100 + -1;
            local_2d4 = iVar3 % 100;
            *(undefined2 *)(iVar1 * 0x70 + 0x5d6e08 + (local_2d4 + -1) * 2) = (undefined2)local_23c;
            if (local_2d4 + -1 == 3) {
              for (; local_2d4 < 5; local_2d4 = local_2d4 + 1) {
                *(undefined2 *)(iVar1 * 0x70 + 0x5d6e08 + local_2d4 * 2) =
                     *(undefined2 *)(iVar1 * 0x70 + 0x5d6e0e);
              }
            }
            if (iVar3 == 0x6c) {
              for (local_2d8 = 1; local_2d8 < 0x3c; local_2d8 = local_2d8 + 1) {
                for (local_2dc = 0; local_2dc < 0x38; local_2dc = local_2dc + 1) {
                  *(undefined2 *)(local_2d8 * 0x70 + 0x5d6e08 + local_2dc * 2) =
                       *(undefined2 *)(local_2dc * 2 + 0x5d6e08);
                }
              }
            }
          }
          else if (((local_210 == 0x1c) || (local_210 == 7)) &&
                  (*(int *)(param_1 + 0x577c8 + local_210 * 0x2e8 + local_22c * 4) == 0x6b)) {
            for (local_2e4 = 1; local_2e4 < 0x3c; local_2e4 = local_2e4 + 1) {
              for (local_2e8 = 0; local_2e8 < 0x38; local_2e8 = local_2e8 + 1) {
                *(undefined2 *)
                 (&DAT_005d0508 + local_2e8 * 2 + local_2e4 * 0x70 + local_210 * 0x1a40) =
                     *(undefined2 *)(&DAT_005d0508 + local_2e8 * 2 + local_210 * 0x1a40);
              }
            }
          }
          local_23c = local_23c + 1;
          if (*(uint *)(&DAT_0065a4a4 + local_210 * 900) <= local_23c) {
            *(int *)(&DAT_0065a288 + local_210 * 4) = local_224 << 6;
            break;
          }
        }
      }
    }
    FUN_0058f62c(local_108);
  }
  return 1;
}

