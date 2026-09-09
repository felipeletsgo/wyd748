// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054e5c2 | Name: FUN_0054e5c2


int FUN_0054e5c2(short *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_c;
  
  if ((*param_1 < 0x91a) || (0x955 < *param_1)) {
    if ((*param_1 < 0xf8c) || (3999 < *param_1)) {
      if ((param_2 == 0) || (param_2 == 1)) {
        iVar1 = FUN_0054e06c(param_1);
        iVar1 = iVar1 % 0xc;
        iVar2 = FUN_0054e41a(param_1);
        iVar3 = FUN_0054cd07(param_1,0x57);
        if (iVar3 == 0) {
          if ((*param_1 < 0x8fc) || (0x919 < *param_1)) {
            local_c = 0;
          }
          else {
            local_c = *(int *)(&DAT_005be344 + iVar1 * 4 + param_2 * 0x30) +
                      *(int *)(&DAT_005be644 + iVar1 * 4) * iVar2;
          }
        }
        else {
          if (param_2 == 0) {
            local_c = *(int *)(&DAT_005be404 + iVar1 * 4 + (iVar3 + -1) * 0x30);
          }
          else {
            local_c = *(int *)(&DAT_005be524 + iVar1 * 4 + (iVar3 + -1) * 0x30);
          }
          local_c = local_c + *(int *)(&DAT_005be644 + iVar1 * 4) * iVar2;
        }
      }
      else {
        local_c = 0;
      }
    }
    else {
      local_c = 0;
    }
  }
  else {
    local_c = 0;
  }
  return local_c;
}

