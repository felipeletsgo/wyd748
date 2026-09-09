// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042bae7 | Name: FUN_0042bae7


undefined4 __thiscall FUN_0042bae7(int param_1,int param_2,uint param_3,uint *param_4)

{
  undefined4 uVar1;
  MMRESULT MVar2;
  _MMIOINFO local_54;
  uint local_c;
  uint local_8;
  
  if (*(int *)(param_1 + 0x80) == 0) {
    if (*(int *)(param_1 + 4) == 0) {
      uVar1 = 0x800401f0;
    }
    else if ((param_2 == 0) || (param_4 == (uint *)0x0)) {
      uVar1 = 0x80070057;
    }
    else {
      if (param_4 != (uint *)0x0) {
        *param_4 = 0;
      }
      MVar2 = mmioGetInfo(*(HMMIO *)(param_1 + 4),&local_54,0);
      if (MVar2 == 0) {
        local_c = param_3;
        if (*(uint *)(param_1 + 0xc) < param_3) {
          local_c = *(uint *)(param_1 + 0xc);
        }
        *(uint *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) - local_c;
        for (local_8 = 0; local_8 < local_c; local_8 = local_8 + 1) {
          if (local_54.pchNext == local_54.pchEndRead) {
            MVar2 = mmioAdvance(*(HMMIO *)(param_1 + 4),&local_54,0);
            if (MVar2 != 0) {
              return 0x80004005;
            }
            if (local_54.pchNext == local_54.pchEndRead) {
              return 0x80004005;
            }
          }
          *(char *)(param_2 + local_8) = *local_54.pchNext;
          local_54.pchNext = local_54.pchNext + 1;
        }
        MVar2 = mmioSetInfo(*(HMMIO *)(param_1 + 4),&local_54,0);
        if (MVar2 == 0) {
          if (param_4 != (uint *)0x0) {
            *param_4 = local_c;
          }
          uVar1 = 0;
        }
        else {
          uVar1 = 0x80004005;
        }
      }
      else {
        uVar1 = 0x80004005;
      }
    }
  }
  else if (*(int *)(param_1 + 0x88) == 0) {
    uVar1 = 0x800401f0;
  }
  else {
    if (param_4 != (uint *)0x0) {
      *param_4 = 0;
    }
    if ((uint)(*(int *)(param_1 + 0x84) + *(int *)(param_1 + 0x8c)) <
        *(int *)(param_1 + 0x88) + param_3) {
      param_3 = *(int *)(param_1 + 0x8c) - (*(int *)(param_1 + 0x88) - *(int *)(param_1 + 0x84));
    }
    FUN_0058f220(param_2,*(undefined4 *)(param_1 + 0x88),param_3);
    if (param_4 != (uint *)0x0) {
      *param_4 = param_3;
    }
    uVar1 = 0;
  }
  return uVar1;
}

