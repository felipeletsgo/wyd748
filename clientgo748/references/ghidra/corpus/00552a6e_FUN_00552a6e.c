// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00552a6e | Name: FUN_00552a6e


void FUN_00552a6e(byte *param_1,byte *param_2)

{
  int iVar1;
  int local_8;
  
  if ((((char)*param_1 < 'A') || ('Z' < (char)*param_1)) &&
     (((char)*param_1 < 'a' || ('z' < (char)*param_1)))) {
    if ((char)*param_1 < '\0') {
      iVar1 = (*param_1 - 0xb0) * 0x5e + -0xa1 + (uint)param_1[1];
      if ((iVar1 < 0) || (0x92d < iVar1)) {
        FUN_0058ee20(param_2,&PTR_DAT_005c4190);
      }
      else {
        for (local_8 = 0; (local_8 < 0x12 && (*(int *)(&DAT_005c3f08 + local_8 * 4) <= iVar1));
            local_8 = local_8 + 1) {
        }
        if ((local_8 < 0) || (0x11 < local_8)) {
          FUN_0058ee20(param_2,&PTR_DAT_005c4194);
        }
        else {
          *param_2 = (&DAT_005c3ee0)[local_8 * 2];
          param_2[1] = (&DAT_005c3ee1)[local_8 * 2];
          param_2[2] = 0;
        }
      }
    }
    else {
      FUN_0058ee20(param_2,&PTR_DAT_005c4198);
    }
  }
  else {
    *param_2 = *param_1;
    param_2[1] = 0;
  }
  return;
}

