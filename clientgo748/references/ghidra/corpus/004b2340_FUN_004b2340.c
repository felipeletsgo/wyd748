// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2340 | Name: FUN_004b2340


void FUN_004b2340(void)

{
  int iVar1;
  int local_198;
  int local_194;
  int local_190;
  int local_188;
  undefined1 local_184 [256];
  undefined1 local_84 [128];
  
  FUN_0058f078(local_184,s_AniSound4_txt_005b9a8c);
  local_188 = 0;
  iVar1 = FUN_0058f716(local_184,&DAT_005b9a9c);
  if (iVar1 != 0) {
    for (local_190 = 0; local_190 < 0x3c; local_190 = local_190 + 1) {
      FUN_00590a68(iVar1,s__s__d_005b9aa0,local_84,&local_188);
      if (local_188 < 2) {
        for (local_194 = 0; local_194 < 0x38; local_194 = local_194 + 1) {
          FUN_00590a68(iVar1,s__s__d__d__d__d__d__d__d__d__d__d_005b9aa8,local_84,
                       &DAT_0067c210 + local_194 * 4 + local_188 * 0x1c0,
                       &DAT_0067c130 + local_194 * 4 + local_188 * 0x1c0,
                       local_188 * 0x1c0 + 0x67c590 + local_194 * 4,
                       &DAT_0067c4b0 + local_194 * 4 + local_188 * 0x1c0,
                       local_188 * 0x1c0 + 0x67c910 + local_194 * 4,
                       &DAT_0067c830 + local_194 * 4 + local_188 * 0x1c0,
                       local_188 * 0x1c0 + 0x67cc90 + local_194 * 4,
                       &DAT_0067cbb0 + local_194 * 4 + local_188 * 0x1c0,
                       &DAT_00672570 + local_194 * 4 + local_188 * 0x2a0);
          *(undefined4 *)(&DAT_00672490 + local_194 * 4 + local_188 * 0x2a0) =
               *(undefined4 *)(&DAT_0067c210 + local_194 * 4 + local_188 * 0x1c0);
          *(undefined4 *)(&DAT_006723b0 + local_194 * 4 + local_188 * 0x2a0) =
               *(undefined4 *)(&DAT_0067c130 + local_194 * 4 + local_188 * 0x1c0);
        }
      }
      else {
        for (local_198 = 0; local_198 < 0x38; local_198 = local_198 + 1) {
          FUN_00590a68(iVar1,s__s__d__d__d_005b9ad0,local_84,
                       &DAT_00672490 + local_198 * 4 + local_188 * 0x2a0,
                       &DAT_006723b0 + local_198 * 4 + local_188 * 0x2a0,
                       &DAT_00672570 + local_198 * 4 + local_188 * 0x2a0);
        }
      }
    }
    FUN_0058f62c(iVar1);
  }
  return;
}

