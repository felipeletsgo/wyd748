// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bead1 | Name: FUN_004bead1


undefined4 FUN_004bead1(void)

{
  int iVar1;
  undefined4 uVar2;
  int unaff_EBP;
  
  *(int *)(unaff_EBP + -0x3c) = *(int *)(unaff_EBP + -0x3c) + 1;
  if (*(uint *)(&DAT_0065a49c + **(int **)(unaff_EBP + -0xd0) * 900) <= *(uint *)(unaff_EBP + -0x3c)
     ) {
    FUN_004c08bb(*(undefined4 *)(*(int *)(unaff_EBP + -0xd0) + 0xb4));
    ExceptionList = *(void **)(unaff_EBP + -0xc);
    return 0;
  }
  FUN_0058f078(unaff_EBP + -0x30,s__s_02d_02d_msh_005bab24,
               &DAT_0065a418 + **(int **)(unaff_EBP + -0xd0) * 900,*(int *)(unaff_EBP + -0x3c) + 1,
               *(ushort *)(*(int *)(unaff_EBP + -0x10) + *(int *)(unaff_EBP + -0x3c) * 4) + 1 +
               *(int *)(*(int *)(unaff_EBP + -0xd0) + 8) * 0x14);
  if ((((((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'g') ||
          ((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'o')) ||
         (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'd' &&
          ((((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'r' &&
            ((&DAT_0065a420)[**(int **)(unaff_EBP + -0xd0) * 900] == '2')) &&
           (*(int *)(unaff_EBP + -0x3c) == 1)))))) ||
        ((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'd' &&
          ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'r')) &&
         ((&DAT_0065a420)[**(int **)(unaff_EBP + -0xd0) * 900] == '1')))) ||
       ((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'b' &&
         ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'd')) &&
        (*(int *)(unaff_EBP + -0x3c) == 1)))) ||
      ((((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'b' &&
          ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'e')) ||
         ((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'b' &&
           ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'o')) ||
          (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'b' &&
           ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'm')))))) ||
        (((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'h' &&
           ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'y')) ||
          (((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 's' &&
             ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'p')) ||
            (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'c' &&
             ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'r')))) ||
           (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'w' &&
            ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'b')))))) ||
         (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'w' &&
          ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'f')))))) ||
       (((((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'b' &&
            ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'e')) ||
           (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'c' &&
            ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'b')))) ||
          ((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'm' &&
            ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'i')) ||
           (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'm' &&
            ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'o')))))) ||
         ((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 't' &&
           ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'w')) ||
          (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 't' &&
           ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'r')))))) ||
        (((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'h' &&
           ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 's')) &&
          (*(int *)(unaff_EBP + -0x3c) == 1)) ||
         (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'e' &&
          ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 't')))))))))) ||
     ((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'b' &&
       ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'n')) ||
      ((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'r' &&
        ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'c')) ||
       (((((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'f' &&
          ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'n')) ||
         (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 'b' &&
          ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'l')))) ||
        (((&DAT_0065a41d)[**(int **)(unaff_EBP + -0xd0) * 900] == 't' &&
         ((&DAT_0065a41e)[**(int **)(unaff_EBP + -0xd0) * 900] == 'g')))))))))) {
    FUN_0058f078(unaff_EBP + -0x80,s__s_02d_02d_wyt_005bab34,
                 &DAT_0065a418 + **(int **)(unaff_EBP + -0xd0) * 900,1,
                 (uint)*(ushort *)(*(int *)(unaff_EBP + -0x10) + *(int *)(unaff_EBP + -0x3c) * 4) +
                 (*(ushort *)(*(int *)(unaff_EBP + -0x10) + 2 + *(int *)(unaff_EBP + -0x3c) * 4) &
                 0xfff) + 1 + *(int *)(*(int *)(unaff_EBP + -0xd0) + 8) * 0x14);
    goto LAB_004bf5e0;
  }
  FUN_0058f078(unaff_EBP + -0x80,s__s_02d_02d_wyt_005bab44,
               &DAT_0065a418 + **(int **)(unaff_EBP + -0xd0) * 900,*(int *)(unaff_EBP + -0x3c) + 1,
               (uint)*(ushort *)(*(int *)(unaff_EBP + -0x10) + *(int *)(unaff_EBP + -0x3c) * 4) +
               (*(ushort *)(*(int *)(unaff_EBP + -0x10) + 2 + *(int *)(unaff_EBP + -0x3c) * 4) &
               0xfff) + 1 + *(int *)(*(int *)(unaff_EBP + -0xd0) + 8) * 0x14);
  iVar1 = _strcmp((char *)(unaff_EBP + -0x30),s_mesh_ch010218_msh_005bab54);
  if (iVar1 == 0) {
    iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_ch010219_wyt_005bab68);
    if (iVar1 == 0) {
      FUN_0058f078(unaff_EBP + -0x80,s_mesh_ch010214_wyt_005bab7c);
      goto LAB_004bf5e0;
    }
  }
  iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101170_wyt_005bab90);
  if (iVar1 != 0) {
    iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101171_wyt_005baba4);
    if (iVar1 != 0) {
      iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101172_wyt_005babb8);
      if (iVar1 != 0) {
        iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101173_wyt_005babcc);
        if (iVar1 != 0) {
          iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101174_wyt_005babe0);
          if (iVar1 != 0) {
            iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101175_wyt_005babf4);
            if (iVar1 != 0) {
              iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101176_wyt_005bac08);
              if (iVar1 != 0) {
                iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101177_wyt_005bac1c);
                if (iVar1 != 0) {
                  iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101178_wyt_005bac30);
                  if (iVar1 != 0) {
                    iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101179_wyt_005bac44);
                    if (iVar1 != 0) {
                      iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101180_wyt_005bac58);
                      if (iVar1 != 0) {
                        iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101181_wyt_005bac6c);
                        if (iVar1 != 0) {
                          iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_mt0101182_wyt_005bac80)
                          ;
                          if (iVar1 != 0) {
                            iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                            s_mesh_mt0101183_wyt_005bac94);
                            if (iVar1 != 0) {
                              iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                              s_mesh_mt0101184_wyt_005baca8);
                              if (iVar1 != 0) {
                                iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                s_mesh_mt0101185_wyt_005bacbc);
                                if (iVar1 != 0) {
                                  iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                  s_mesh_mt0101186_wyt_005bacd0);
                                  if (iVar1 != 0) {
                                    iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                    s_mesh_mt0101187_wyt_005bace4);
                                    if (iVar1 != 0) {
                                      iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                      s_mesh_mt0101188_wyt_005bacf8);
                                      if (iVar1 != 0) {
                                        iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                        s_mesh_mt0101189_wyt_005bad0c);
                                        if (iVar1 != 0) {
                                          iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                          s_mesh_mt0101190_wyt_005bad20);
                                          if (iVar1 != 0) {
                                            iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                            s_mesh_mt0101191_wyt_005bad34);
                                            if (iVar1 != 0) {
                                              iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                              s_mesh_mt0101192_wyt_005bad48);
                                              if (iVar1 != 0) {
                                                iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                s_mesh_mt0101193_wyt_005bad5c);
                                                if (iVar1 != 0) {
                                                  iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                  s_mesh_mt0101194_wyt_005bad70);
                                                  if (iVar1 != 0) {
                                                    iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                    s_mesh_mt0101195_wyt_005bad84);
                                                    if (iVar1 != 0) {
                                                      iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                      s_mesh_mt0101196_wyt_005bad98)
                                                      ;
                                                      if (iVar1 != 0) {
                                                        iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                                                                                                
                                                  s_mesh_mt0101197_wyt_005badac);
                                                  if (iVar1 != 0) {
                                                    iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                    s_mesh_mt0101198_wyt_005badc0);
                                                    if (iVar1 != 0) {
                                                      iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                      s_mesh_mt0101199_wyt_005badd4)
                                                      ;
                                                      if (iVar1 != 0) {
                                                        iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                                                                                                
                                                  s_mesh_mt0101200_wyt_005bade8);
                                                  if (iVar1 != 0) {
                                                    iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                    s_mesh_mt010124_wyt_005bae10);
                                                    if (iVar1 == 0) {
                                                      FUN_0058f078(unaff_EBP + -0x30,
                                                                   s_mesh_mt010124_msh_005bae24);
                                                    }
                                                    else {
                                                      iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                      s_mesh_mt010132_wyt_005bae38);
                                                      if (iVar1 != 0) {
                                                        iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                        s_mesh_mt010133_wyt_005bae4c
                                                                       );
                                                        if (iVar1 != 0) {
                                                          iVar1 = _strcmp((char *)(unaff_EBP + -0x80
                                                                                  ),
                                                                                                                                                    
                                                  s_mesh_mt010134_wyt_005bae60);
                                                  if (iVar1 != 0) {
                                                    iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                    s_mesh_mt010135_wyt_005bae74);
                                                    if (iVar1 != 0) {
                                                      iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                      s_mesh_mt010136_wyt_005bae88);
                                                      if (iVar1 != 0) {
                                                        iVar1 = _strcmp((char *)(unaff_EBP + -0x80),
                                                                        s_mesh_mt010137_wyt_005bae9c
                                                                       );
                                                        if (iVar1 != 0) {
                                                          iVar1 = _strcmp((char *)(unaff_EBP + -0x80
                                                                                  ),
                                                                                                                                                    
                                                  s_mesh_mt010124_wyt_005baec4);
                                                  if (iVar1 == 0) {
                                                    FUN_0058f078(unaff_EBP + -0x30,
                                                                 s_mesh_mt010124_msh_005baed8);
                                                  }
                                                  goto LAB_004bf5e0;
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  FUN_0058f078(unaff_EBP + -0x30,
                                                               s_mesh_mt010131_msh_005baeb0);
                                                  }
                                                  goto LAB_004bf5e0;
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
      }
    }
  }
  FUN_0058f078(unaff_EBP + -0x30,s_mesh_mt010131_msh_005badfc);
LAB_004bf5e0:
  if ((((*(char *)(unaff_EBP + -0x7b) == 'c') && (*(char *)(unaff_EBP + -0x7a) == 'h')) &&
      (*(char *)(unaff_EBP + -0x78) == '2')) &&
     ((*(char *)(unaff_EBP + -0x75) == '1' && (*(char *)(unaff_EBP + -0x74) == '3')))) {
    if (*(char *)(unaff_EBP + -0x76) == '1') {
      FUN_0058f078(unaff_EBP + -0x80,s_mesh_ch010130_wyt_005baeec);
    }
    if (*(char *)(unaff_EBP + -0x76) == '4') {
      FUN_0058f078(unaff_EBP + -0x80,s_mesh_ch010430_wyt_005baf00);
    }
    if (*(char *)(unaff_EBP + -0x76) == '5') {
      FUN_0058f078(unaff_EBP + -0x80,s_mesh_ch010530_wyt_005baf14);
    }
  }
  iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_ch020315_wyt_005baf28);
  if (iVar1 == 0) {
    FUN_0058f078(unaff_EBP + -0x80,s_mesh_ch020314_wyt_005baf3c);
  }
  else {
    iVar1 = _strcmp((char *)(unaff_EBP + -0x80),s_mesh_bm010102_wyt_005baf50);
    if (iVar1 == 0) {
      FUN_0058f078(unaff_EBP + -0x80,s_mesh_mi010105_wyt_005baf64);
    }
    else {
      iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_tr13_005baf78,9);
      if (iVar1 == 0) {
        FUN_0058f078(unaff_EBP + -0x80,s_mesh_tr130101_wyt_005baf84);
      }
      else {
        iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_tr14_005baf98,9);
        if (iVar1 == 0) {
          FUN_0058f078(unaff_EBP + -0x80,s_mesh_tr130101_wyt_005bafa4);
        }
        else {
          iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_tr15_005bafb8,9);
          if (iVar1 == 0) {
            FUN_0058f078(unaff_EBP + -0x80,s_mesh_tr130101_wyt_005bafc4);
          }
          else {
            iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_tr16_005bafd8,9);
            if (iVar1 == 0) {
              FUN_0058f078(unaff_EBP + -0x80,s_mesh_tr130101_wyt_005bafe4);
            }
            else {
              iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_tr17_005baff8,9);
              if (iVar1 == 0) {
                FUN_0058f078(unaff_EBP + -0x80,s_mesh_tr130101_wyt_005bb004);
              }
              else {
                iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_tr190101_005bb018,0xd);
                if (iVar1 == 0) {
                  FUN_0058f078(unaff_EBP + -0x80,s_mesh_tr180101_wyt_005bb028);
                }
                else {
                  iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_tr190102_005bb03c,0xd);
                  if (iVar1 == 0) {
                    FUN_0058f078(unaff_EBP + -0x80,s_mesh_tr180102_wyt_005bb04c);
                  }
                  else {
                    iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_tr200101_005bb060,0xd);
                    if (iVar1 == 0) {
                      FUN_0058f078(unaff_EBP + -0x80,s_mesh_tr180101_wyt_005bb070);
                    }
                    else {
                      iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_tr200102_005bb084,0xd);
                      if (iVar1 == 0) {
                        FUN_0058f078(unaff_EBP + -0x80,s_mesh_tr180102_wyt_005bb094);
                      }
                      else {
                        iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch010237_005bb0a8,0xd);
                        if (iVar1 == 0) {
                          FUN_0058f078(unaff_EBP + -0x80,s_mesh_ch010137_wyt_005bb0b8);
                        }
                        else {
                          iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch010238_005bb0cc,0xd)
                          ;
                          if (iVar1 == 0) {
                            FUN_0058f078(unaff_EBP + -0x80,s_mesh_ch010138_wyt_005bb0dc);
                          }
                          else {
                            iVar1 = _strncmp((char *)(unaff_EBP + -0x80),s_mesh_ch020217_005bb0f0,
                                             0xd);
                            if (iVar1 == 0) {
                              FUN_0058f078(unaff_EBP + -0x80,s_mesh_ch020117_wyt_005bb100);
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
  uVar2 = FUN_013c2100();
  return uVar2;
}

