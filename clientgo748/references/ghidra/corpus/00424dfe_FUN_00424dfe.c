// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00424dfe | Name: FUN_00424dfe


undefined4 __thiscall FUN_00424dfe(int *param_1,undefined2 *param_2,int param_3,int param_4)

{
  byte bVar1;
  char cVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  uint local_11c;
  uint local_110;
  undefined1 local_10c [256];
  uint local_c;
  uint local_8;
  
  if (param_1[3] + param_3 < 0x20000) {
    if (*param_1 == 0) {
      FUN_0058f078(local_10c,s_err_add_buffer_invalid__d__d__d___005b20a0,param_1[3],param_3,
                   param_2[2],*param_1);
      FUN_00423b20(local_10c,s__system_005b20c4,0);
      uVar3 = 0;
    }
    else {
      local_c = CONCAT31(local_c._1_3_,(undefined1)param_4);
      if (param_4 == 0) {
        iVar4 = _rand();
        local_c = CONCAT31(local_c._1_3_,(char)iVar4);
      }
      bVar1 = (&DAT_005b1d10)[(local_c & 0xff) * 2];
      *param_2 = (undefined2)param_3;
      *(undefined1 *)(param_2 + 1) = (undefined1)local_c;
      *(undefined1 *)((int)param_2 + 3) = 0;
      *(undefined4 *)(param_2 + 4) = DAT_0092e658;
      DAT_0092e65c = DAT_0092e658;
      cVar2 = '\0';
      local_8 = local_8 & 0xffffff00;
      local_11c = (uint)bVar1;
      for (local_110 = 4; (int)local_110 < param_3; local_110 = local_110 + 1) {
        cVar2 = cVar2 + *(char *)((int)param_2 + local_110);
        uVar5 = local_11c & 0x800000ff;
        if ((int)uVar5 < 0) {
          uVar5 = (uVar5 - 1 | 0xffffff00) + 1;
        }
        bVar1 = (&DAT_005b1d11)[uVar5 * 2];
        uVar5 = local_110 & 3;
        if (uVar5 == 0) {
          *(byte *)(param_1[1] + param_1[3] + local_110) =
               *(char *)((int)param_2 + local_110) + bVar1 * '\x02';
        }
        if (uVar5 == 1) {
          *(char *)(param_1[1] + param_1[3] + local_110) =
               *(char *)((int)param_2 + local_110) - (char)((int)(uint)bVar1 >> 3);
        }
        if (uVar5 == 2) {
          *(byte *)(param_1[1] + param_1[3] + local_110) =
               *(char *)((int)param_2 + local_110) + bVar1 * '\x04';
        }
        if (uVar5 == 3) {
          *(char *)(param_1[1] + param_1[3] + local_110) =
               *(char *)((int)param_2 + local_110) - (char)((int)(uint)bVar1 >> 5);
        }
        local_8 = CONCAT31(local_8._1_3_,
                           (char)local_8 + *(char *)(param_1[1] + param_1[3] + local_110));
        local_11c = local_11c + 1;
      }
      *(char *)((int)param_2 + 3) = (char)local_8 - cVar2;
      FUN_0058f220(param_1[1] + param_1[3],param_2,4);
      param_1[3] = param_1[3] + param_3;
      uVar3 = 1;
    }
  }
  else {
    FUN_0058f078(local_10c,s_err_add_buffer_full__d__d__d__d_005b2078,param_1[3],param_3,param_2[2],
                 *param_1);
    FUN_00423b20(local_10c,s__system_005b2098,0);
    uVar3 = 0;
  }
  return uVar3;
}

