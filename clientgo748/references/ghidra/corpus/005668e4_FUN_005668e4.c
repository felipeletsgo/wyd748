// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005668e4 | Name: FUN_005668e4


undefined4 __thiscall FUN_005668e4(int *param_1,HANDLE param_2)

{
  undefined4 uVar1;
  LPCVOID lpBuffer;
  uint uVar2;
  int iVar3;
  DWORD nNumberOfBytesToWrite;
  int *piVar4;
  int *piVar5;
  uint *puVar6;
  int *piVar7;
  uint uVar8;
  uint local_98 [5];
  uint local_84;
  int local_80;
  int local_50 [8];
  uint local_30;
  byte local_2b;
  byte local_2a;
  undefined4 local_1c;
  int *local_18;
  uint local_14;
  DWORD local_10;
  int *local_c;
  uint local_8;
  
  local_8 = 0;
  for (piVar4 = param_1; piVar4 != (int *)0x0; piVar4 = (int *)piVar4[0x13]) {
    local_8 = local_8 + 1;
  }
  piVar4 = &DAT_005c52f8;
  if (DAT_005c52f8 != 0) {
    iVar3 = DAT_005c52f8;
    do {
      if (iVar3 == *param_1) break;
      piVar4 = piVar4 + 9;
      iVar3 = *piVar4;
    } while (iVar3 != 0);
  }
  if (*piVar4 == 0) {
    uVar1 = 0x80004005;
  }
  else {
    local_18 = param_1;
    local_c = (int *)FUN_0055fab2(*param_1);
    local_14 = *(uint *)((int)local_c + 8) >> 3;
    local_1c = 0x20534444;
    WriteFile(param_2,&local_1c,4,&local_10,(LPOVERLAPPED)0x0);
    puVar6 = local_98;
    for (iVar3 = 0x1f; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar6 = 0;
      puVar6 = puVar6 + 1;
    }
    local_98[2] = param_1[4];
    local_98[3] = param_1[3];
    uVar2 = piVar4[2];
    local_98[0] = 0x7c;
    local_98[1] = 0x1007;
    piVar7 = local_50;
    for (iVar3 = 8; piVar5 = local_18, piVar4 = piVar4 + 1, iVar3 != 0; iVar3 = iVar3 + -1) {
      *piVar7 = *piVar4;
      piVar7 = piVar7 + 1;
    }
    local_30 = 0x1000;
    if ((uVar2 & 3) != 0) {
      local_30 = 0x1002;
    }
    if ((uVar2 & 0x20) != 0) {
      local_30 = local_30 | 0x100;
    }
    if (local_18[0x13] != 0) {
      local_30 = local_30 | 0x400008;
      local_98[1] = 0x21007;
      local_80 = local_8;
    }
    if (local_18[0x14] != 0) {
      local_30 = local_30 | 8;
      local_2b = local_2b | 0xfe;
    }
    if (1 < (uint)local_18[5]) {
      local_98[1] = local_98[1] | 0x800000;
      local_2a = local_2a | 0x20;
      local_84 = local_18[5];
    }
    WriteFile(param_2,local_98,0x7c,&local_10,(LPOVERLAPPED)0x0);
    if (*(int *)((int)local_c + 4) == 1) {
      WriteFile(param_2,(LPCVOID)piVar5[2],0x400,&local_10,(LPOVERLAPPED)0x0);
    }
    local_c = piVar5;
    do {
      for (; piVar5 != (int *)0x0; piVar5 = (int *)piVar5[0x13]) {
        local_8 = 0;
        if (piVar5[5] != 0) {
          uVar2 = piVar5[4];
          do {
            uVar8 = 0;
            if (uVar2 != 0) {
              do {
                iVar3 = *local_18;
                if (iVar3 == 0x31545844) {
                  iVar3 = (piVar5[7] + uVar8 >> 2) * piVar5[0xc] +
                          (piVar5[10] + local_8) * piVar5[0xd] + ((uint)piVar5[6] >> 2) * 8;
                  nNumberOfBytesToWrite = (piVar5[3] + 3U >> 2) << 3;
LAB_00566b34:
                  lpBuffer = (LPCVOID)(iVar3 + piVar5[1]);
                  uVar8 = uVar8 + 3;
                }
                else {
                  if (iVar3 == 0x32545844) {
LAB_00566ad6:
                    iVar3 = (piVar5[7] + uVar8 >> 2) * piVar5[0xc] +
                            (piVar5[10] + local_8) * piVar5[0xd] + ((uint)piVar5[6] >> 2) * 0x10;
                    nNumberOfBytesToWrite = (piVar5[3] + 3U >> 2) << 4;
                    goto LAB_00566b34;
                  }
                  if (iVar3 == 0x32595559) {
LAB_00566aaa:
                    lpBuffer = (LPCVOID)((piVar5[7] + uVar8) * piVar5[0xc] +
                                         (piVar5[10] + local_8) * piVar5[0xd] + piVar5[6] * local_14
                                        + piVar5[1]);
                    nNumberOfBytesToWrite = (piVar5[3] + 1U >> 1) << 2;
                  }
                  else {
                    if (((iVar3 == 0x33545844) || (iVar3 == 0x34545844)) || (iVar3 == 0x35545844))
                    goto LAB_00566ad6;
                    if (iVar3 == 0x59565955) goto LAB_00566aaa;
                    lpBuffer = (LPCVOID)((piVar5[7] + uVar8) * piVar5[0xc] +
                                         (piVar5[10] + local_8) * piVar5[0xd] + piVar5[6] * local_14
                                        + piVar5[1]);
                    nNumberOfBytesToWrite = piVar5[3] * local_14;
                  }
                }
                WriteFile(param_2,lpBuffer,nNumberOfBytesToWrite,&local_10,(LPOVERLAPPED)0x0);
                uVar2 = piVar5[4];
                uVar8 = uVar8 + 1;
              } while (uVar8 < uVar2);
            }
            local_8 = local_8 + 1;
          } while (local_8 < (uint)piVar5[5]);
        }
      }
      piVar5 = (int *)local_c[0x14];
      local_c = piVar5;
    } while (piVar5 != (int *)0x0);
    uVar1 = 0;
  }
  return uVar1;
}

