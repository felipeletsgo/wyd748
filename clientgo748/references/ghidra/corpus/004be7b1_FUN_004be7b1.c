// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004be7b1 | Name: FUN_004be7b1


undefined4 __fastcall FUN_004be7b1(int *param_1)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  int iVar4;
  int local_e0;
  int local_dc;
  int local_90;
  char acStack_84 [5];
  char cStack_7f;
  char cStack_7e;
  char cStack_7c;
  char cStack_7a;
  char cStack_79;
  char cStack_78;
  int *local_44;
  uint local_40;
  undefined4 local_3c;
  int *local_38;
  char acStack_34 [32];
  ushort *local_14;
  void *pvStack_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a06ba;
  pvStack_10 = ExceptionList;
  ExceptionList = &pvStack_10;
  param_1[1] = 0;
  if ((param_1[0x2d] != 0) && (param_1[0x2d] != 0)) {
    if (param_1[0x2d] != 0) {
      FUN_004c5ae0(1);
    }
    param_1[0x2d] = 0;
  }
  pvVar2 = operator_new(0x98);
  local_8 = 0;
  if (pvVar2 == (void *)0x0) {
    local_dc = 0;
  }
  else {
    local_dc = FUN_004c151c(0);
  }
  local_8 = 0xffffffff;
  param_1[0x2d] = local_dc;
  if (param_1[0x2d] == 0) {
    uVar3 = 0x80004005;
  }
  else {
    param_1[0x2e] = param_1[0x2d];
    *(int **)(param_1[0x2d] + 0x94) = param_1;
    for (local_40 = 0; local_40 < *(uint *)(&DAT_0065a498 + *param_1 * 900); local_40 = local_40 + 1
        ) {
      iVar4 = *(int *)(*(int *)(&DAT_0065a4a0 + *param_1 * 900) + local_40 * 8);
      local_90 = iVar4;
      if (iVar4 == -1) {
        local_90 = 0;
      }
      iVar1 = *(int *)(*(int *)(&DAT_0065a4a0 + *param_1 * 900) + 4 + local_40 * 8);
      pvVar2 = operator_new(0x98);
      local_8 = 1;
      if (pvVar2 == (void *)0x0) {
        local_e0 = 0;
      }
      else {
        local_e0 = FUN_004c151c(iVar1);
      }
      local_8 = 0xffffffff;
      if (local_e0 == 0) {
        ExceptionList = pvStack_10;
        return 0x80004005;
      }
      *(int *)(local_e0 + 4) = iVar4;
      *(int **)(local_e0 + 0x94) = param_1;
      param_1[iVar1 + 0x2e] = local_e0;
      iVar4 = FUN_004c15fc(local_90);
      if (iVar4 != 0) {
        FUN_004c167b(local_e0);
      }
    }
    local_3c = 0;
    local_14 = (ushort *)(param_1 + 0x92);
    local_44 = param_1 + 0x9a;
    local_38 = param_1 + 0x9e;
    local_40 = 0;
    if (*(int *)(&DAT_0065a49c + *param_1 * 900) != 0) {
      FUN_0058f078(acStack_34,s__s_02d_02d_msh_005bab24,&DAT_0065a418 + *param_1 * 900,1,
                   *local_14 + 1 + param_1[2] * 0x14);
      if ((((((((&DAT_0065a41d)[*param_1 * 900] == 'g') || ((&DAT_0065a41d)[*param_1 * 900] == 'o'))
             || (((&DAT_0065a41d)[*param_1 * 900] == 'd' &&
                 ((((&DAT_0065a41e)[*param_1 * 900] == 'r' &&
                   ((&DAT_0065a420)[*param_1 * 900] == '2')) && (local_40 == 1)))))) ||
            ((((&DAT_0065a41d)[*param_1 * 900] == 'd' && ((&DAT_0065a41e)[*param_1 * 900] == 'r'))
             && ((&DAT_0065a420)[*param_1 * 900] == '1')))) ||
           ((((((((((&DAT_0065a41d)[*param_1 * 900] == 'b' &&
                   ((&DAT_0065a41e)[*param_1 * 900] == 'd')) && (local_40 == 1)) ||
                 (((&DAT_0065a41d)[*param_1 * 900] == 'b' &&
                  ((&DAT_0065a41e)[*param_1 * 900] == 'e')))) ||
                (((&DAT_0065a41d)[*param_1 * 900] == 'b' && ((&DAT_0065a41e)[*param_1 * 900] == 'o')
                 ))) || (((&DAT_0065a41d)[*param_1 * 900] == 'b' &&
                         ((&DAT_0065a41e)[*param_1 * 900] == 'm')))) ||
              (((&DAT_0065a41d)[*param_1 * 900] == 'h' && ((&DAT_0065a41e)[*param_1 * 900] == 'y')))
              ) || ((((&DAT_0065a41d)[*param_1 * 900] == 's' &&
                     ((&DAT_0065a41e)[*param_1 * 900] == 'p')) ||
                    (((&DAT_0065a41d)[*param_1 * 900] == 'c' &&
                     ((&DAT_0065a41e)[*param_1 * 900] == 'r')))))) ||
            (((&DAT_0065a41d)[*param_1 * 900] == 'w' && ((&DAT_0065a41e)[*param_1 * 900] == 'b')))))
           ) || (((&DAT_0065a41d)[*param_1 * 900] == 'w' && ((&DAT_0065a41e)[*param_1 * 900] == 'f')
                 ))) ||
         ((((((&DAT_0065a41d)[*param_1 * 900] == 'b' && ((&DAT_0065a41e)[*param_1 * 900] == 'e')) ||
            (((&DAT_0065a41d)[*param_1 * 900] == 'c' && ((&DAT_0065a41e)[*param_1 * 900] == 'b'))))
           || (((&DAT_0065a41d)[*param_1 * 900] == 'm' && ((&DAT_0065a41e)[*param_1 * 900] == 'i')))
           ) || ((((((&DAT_0065a41d)[*param_1 * 900] == 'm' &&
                    ((&DAT_0065a41e)[*param_1 * 900] == 'o')) ||
                   (((((&DAT_0065a41d)[*param_1 * 900] == 't' &&
                      ((&DAT_0065a41e)[*param_1 * 900] == 'w')) ||
                     (((&DAT_0065a41d)[*param_1 * 900] == 't' &&
                      ((&DAT_0065a41e)[*param_1 * 900] == 'r')))) ||
                    (((((&DAT_0065a41d)[*param_1 * 900] == 'h' &&
                       ((&DAT_0065a41e)[*param_1 * 900] == 's')) && (local_40 == 1)) ||
                     (((&DAT_0065a41d)[*param_1 * 900] == 'e' &&
                      ((&DAT_0065a41e)[*param_1 * 900] == 't')))))))) ||
                  (((&DAT_0065a41d)[*param_1 * 900] == 'b' &&
                   ((&DAT_0065a41e)[*param_1 * 900] == 'n')))) ||
                 (((((&DAT_0065a41d)[*param_1 * 900] == 'r' &&
                    ((&DAT_0065a41e)[*param_1 * 900] == 'c')) ||
                   ((((&DAT_0065a41d)[*param_1 * 900] == 'f' &&
                     ((&DAT_0065a41e)[*param_1 * 900] == 'n')) ||
                    (((&DAT_0065a41d)[*param_1 * 900] == 'b' &&
                     ((&DAT_0065a41e)[*param_1 * 900] == 'l')))))) ||
                  (((&DAT_0065a41d)[*param_1 * 900] == 't' &&
                   ((&DAT_0065a41e)[*param_1 * 900] == 'g')))))))))) {
        FUN_0058f078(acStack_84,s__s_02d_02d_wyt_005bab34,&DAT_0065a418 + *param_1 * 900,1,
                     (uint)local_14[local_40 * 2] + (local_14[local_40 * 2 + 1] & 0xfff) + 1 +
                     param_1[2] * 0x14);
      }
      else {
        FUN_0058f078(acStack_84,s__s_02d_02d_wyt_005bab44,&DAT_0065a418 + *param_1 * 900,
                     local_40 + 1,
                     (uint)local_14[local_40 * 2] + (local_14[local_40 * 2 + 1] & 0xfff) + 1 +
                     param_1[2] * 0x14);
        iVar4 = _strcmp(acStack_34,s_mesh_ch010218_msh_005bab54);
        if ((iVar4 == 0) && (iVar4 = _strcmp(acStack_84,s_mesh_ch010219_wyt_005bab68), iVar4 == 0))
        {
          FUN_0058f078(acStack_84,s_mesh_ch010214_wyt_005bab7c);
        }
        else {
          iVar4 = _strcmp(acStack_84,s_mesh_mt0101170_wyt_005bab90);
          if (((((iVar4 == 0) ||
                (((iVar4 = _strcmp(acStack_84,s_mesh_mt0101171_wyt_005baba4), iVar4 == 0 ||
                  (iVar4 = _strcmp(acStack_84,s_mesh_mt0101172_wyt_005babb8), iVar4 == 0)) ||
                 (iVar4 = _strcmp(acStack_84,s_mesh_mt0101173_wyt_005babcc), iVar4 == 0)))) ||
               (((iVar4 = _strcmp(acStack_84,s_mesh_mt0101174_wyt_005babe0), iVar4 == 0 ||
                 (iVar4 = _strcmp(acStack_84,s_mesh_mt0101175_wyt_005babf4), iVar4 == 0)) ||
                (iVar4 = _strcmp(acStack_84,s_mesh_mt0101176_wyt_005bac08), iVar4 == 0)))) ||
              ((((iVar4 = _strcmp(acStack_84,s_mesh_mt0101177_wyt_005bac1c), iVar4 == 0 ||
                 (iVar4 = _strcmp(acStack_84,s_mesh_mt0101178_wyt_005bac30), iVar4 == 0)) ||
                ((iVar4 = _strcmp(acStack_84,s_mesh_mt0101179_wyt_005bac44), iVar4 == 0 ||
                 (((iVar4 = _strcmp(acStack_84,s_mesh_mt0101180_wyt_005bac58), iVar4 == 0 ||
                   (iVar4 = _strcmp(acStack_84,s_mesh_mt0101181_wyt_005bac6c), iVar4 == 0)) ||
                  (iVar4 = _strcmp(acStack_84,s_mesh_mt0101182_wyt_005bac80), iVar4 == 0)))))) ||
               ((iVar4 = _strcmp(acStack_84,s_mesh_mt0101183_wyt_005bac94), iVar4 == 0 ||
                (iVar4 = _strcmp(acStack_84,s_mesh_mt0101184_wyt_005baca8), iVar4 == 0)))))) ||
             ((((iVar4 = _strcmp(acStack_84,s_mesh_mt0101185_wyt_005bacbc), iVar4 == 0 ||
                (((iVar4 = _strcmp(acStack_84,s_mesh_mt0101186_wyt_005bacd0), iVar4 == 0 ||
                  (iVar4 = _strcmp(acStack_84,s_mesh_mt0101187_wyt_005bace4), iVar4 == 0)) ||
                 ((iVar4 = _strcmp(acStack_84,s_mesh_mt0101188_wyt_005bacf8), iVar4 == 0 ||
                  (((((iVar4 = _strcmp(acStack_84,s_mesh_mt0101189_wyt_005bad0c), iVar4 == 0 ||
                      (iVar4 = _strcmp(acStack_84,s_mesh_mt0101190_wyt_005bad20), iVar4 == 0)) ||
                     (iVar4 = _strcmp(acStack_84,s_mesh_mt0101191_wyt_005bad34), iVar4 == 0)) ||
                    ((iVar4 = _strcmp(acStack_84,s_mesh_mt0101192_wyt_005bad48), iVar4 == 0 ||
                     (iVar4 = _strcmp(acStack_84,s_mesh_mt0101193_wyt_005bad5c), iVar4 == 0)))) ||
                   (iVar4 = _strcmp(acStack_84,s_mesh_mt0101194_wyt_005bad70), iVar4 == 0)))))))) ||
               (((iVar4 = _strcmp(acStack_84,s_mesh_mt0101195_wyt_005bad84), iVar4 == 0 ||
                 (iVar4 = _strcmp(acStack_84,s_mesh_mt0101196_wyt_005bad98), iVar4 == 0)) ||
                (iVar4 = _strcmp(acStack_84,s_mesh_mt0101197_wyt_005badac), iVar4 == 0)))) ||
              (((iVar4 = _strcmp(acStack_84,s_mesh_mt0101198_wyt_005badc0), iVar4 == 0 ||
                (iVar4 = _strcmp(acStack_84,s_mesh_mt0101199_wyt_005badd4), iVar4 == 0)) ||
               (iVar4 = _strcmp(acStack_84,s_mesh_mt0101200_wyt_005bade8), iVar4 == 0)))))) {
            FUN_0058f078(acStack_34,s_mesh_mt010131_msh_005badfc);
          }
          else {
            iVar4 = _strcmp(acStack_84,s_mesh_mt010124_wyt_005bae10);
            if (iVar4 == 0) {
              FUN_0058f078(acStack_34,s_mesh_mt010124_msh_005bae24);
            }
            else {
              iVar4 = _strcmp(acStack_84,s_mesh_mt010132_wyt_005bae38);
              if ((((iVar4 == 0) ||
                   (iVar4 = _strcmp(acStack_84,s_mesh_mt010133_wyt_005bae4c), iVar4 == 0)) ||
                  ((iVar4 = _strcmp(acStack_84,s_mesh_mt010134_wyt_005bae60), iVar4 == 0 ||
                   ((iVar4 = _strcmp(acStack_84,s_mesh_mt010135_wyt_005bae74), iVar4 == 0 ||
                    (iVar4 = _strcmp(acStack_84,s_mesh_mt010136_wyt_005bae88), iVar4 == 0)))))) ||
                 (iVar4 = _strcmp(acStack_84,s_mesh_mt010137_wyt_005bae9c), iVar4 == 0)) {
                FUN_0058f078(acStack_34,s_mesh_mt010131_msh_005baeb0);
              }
              else {
                iVar4 = _strcmp(acStack_84,s_mesh_mt010124_wyt_005baec4);
                if (iVar4 == 0) {
                  FUN_0058f078(acStack_34,s_mesh_mt010124_msh_005baed8);
                }
              }
            }
          }
        }
      }
      if ((((cStack_7f == 'c') && (cStack_7e == 'h')) && (cStack_7c == '2')) &&
         ((cStack_79 == '1' && (cStack_78 == '3')))) {
        if (cStack_7a == '1') {
          FUN_0058f078(acStack_84,s_mesh_ch010130_wyt_005baeec);
        }
        if (cStack_7a == '4') {
          FUN_0058f078(acStack_84,s_mesh_ch010430_wyt_005baf00);
        }
        if (cStack_7a == '5') {
          FUN_0058f078(acStack_84,s_mesh_ch010530_wyt_005baf14);
        }
      }
      iVar4 = _strcmp(acStack_84,s_mesh_ch020315_wyt_005baf28);
      if (iVar4 == 0) {
        FUN_0058f078(acStack_84,s_mesh_ch020314_wyt_005baf3c);
      }
      else {
        iVar4 = _strcmp(acStack_84,s_mesh_bm010102_wyt_005baf50);
        if (iVar4 == 0) {
          FUN_0058f078(acStack_84,s_mesh_mi010105_wyt_005baf64);
        }
        else {
          iVar4 = _strncmp(acStack_84,s_mesh_tr13_005baf78,9);
          if (iVar4 == 0) {
            FUN_0058f078(acStack_84,s_mesh_tr130101_wyt_005baf84);
          }
          else {
            iVar4 = _strncmp(acStack_84,s_mesh_tr14_005baf98,9);
            if (iVar4 == 0) {
              FUN_0058f078(acStack_84,s_mesh_tr130101_wyt_005bafa4);
            }
            else {
              iVar4 = _strncmp(acStack_84,s_mesh_tr15_005bafb8,9);
              if (iVar4 == 0) {
                FUN_0058f078(acStack_84,s_mesh_tr130101_wyt_005bafc4);
              }
              else {
                iVar4 = _strncmp(acStack_84,s_mesh_tr16_005bafd8,9);
                if (iVar4 == 0) {
                  FUN_0058f078(acStack_84,s_mesh_tr130101_wyt_005bafe4);
                }
                else {
                  iVar4 = _strncmp(acStack_84,s_mesh_tr17_005baff8,9);
                  if (iVar4 == 0) {
                    FUN_0058f078(acStack_84,s_mesh_tr130101_wyt_005bb004);
                  }
                  else {
                    iVar4 = _strncmp(acStack_84,s_mesh_tr190101_005bb018,0xd);
                    if (iVar4 == 0) {
                      FUN_0058f078(acStack_84,s_mesh_tr180101_wyt_005bb028);
                    }
                    else {
                      iVar4 = _strncmp(acStack_84,s_mesh_tr190102_005bb03c,0xd);
                      if (iVar4 == 0) {
                        FUN_0058f078(acStack_84,s_mesh_tr180102_wyt_005bb04c);
                      }
                      else {
                        iVar4 = _strncmp(acStack_84,s_mesh_tr200101_005bb060,0xd);
                        if (iVar4 == 0) {
                          FUN_0058f078(acStack_84,s_mesh_tr180101_wyt_005bb070);
                        }
                        else {
                          iVar4 = _strncmp(acStack_84,s_mesh_tr200102_005bb084,0xd);
                          if (iVar4 == 0) {
                            FUN_0058f078(acStack_84,s_mesh_tr180102_wyt_005bb094);
                          }
                          else {
                            iVar4 = _strncmp(acStack_84,s_mesh_ch010237_005bb0a8,0xd);
                            if (iVar4 == 0) {
                              FUN_0058f078(acStack_84,s_mesh_ch010137_wyt_005bb0b8);
                            }
                            else {
                              iVar4 = _strncmp(acStack_84,s_mesh_ch010238_005bb0cc,0xd);
                              if (iVar4 == 0) {
                                FUN_0058f078(acStack_84,s_mesh_ch010138_wyt_005bb0dc);
                              }
                              else {
                                iVar4 = _strncmp(acStack_84,s_mesh_ch020217_005bb0f0,0xd);
                                if (iVar4 == 0) {
                                  FUN_0058f078(acStack_84,s_mesh_ch020117_wyt_005bb100);
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      uVar3 = FUN_013c2100();
      return uVar3;
    }
    FUN_004c08bb(param_1[0x2d]);
    uVar3 = 0;
  }
  ExceptionList = pvStack_10;
  return uVar3;
}

