// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042be46 | Name: FUN_0042be46


undefined4 __thiscall FUN_0042be46(int param_1,short *param_2)

{
  MMRESULT MVar1;
  undefined4 uVar2;
  LONG LVar3;
  _MMCKINFO local_1c;
  char local_8 [4];
  
  local_8[0] = -1;
  local_8[1] = -1;
  local_8[2] = -1;
  local_8[3] = -1;
  *(undefined4 *)(param_1 + 0x24) = 0x45564157;
  *(undefined4 *)(param_1 + 0x20) = 0;
  MVar1 = mmioCreateChunk(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 0x1c),0x20);
  if (MVar1 == 0) {
    *(undefined4 *)(param_1 + 8) = 0x20746d66;
    *(undefined4 *)(param_1 + 0xc) = 0x10;
    MVar1 = mmioCreateChunk(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 8),0);
    if (MVar1 == 0) {
      if (*param_2 == 1) {
        LVar3 = mmioWrite(*(HMMIO *)(param_1 + 4),(char *)param_2,0x10);
        if (LVar3 != 0x10) {
          return 0x80004005;
        }
      }
      else {
        LVar3 = mmioWrite(*(HMMIO *)(param_1 + 4),(char *)param_2,(ushort)param_2[8] + 0x12);
        if (LVar3 != (ushort)param_2[8] + 0x12) {
          return 0x80004005;
        }
      }
      MVar1 = mmioAscend(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 8),0);
      if (MVar1 == 0) {
        local_1c.ckid = 0x74636166;
        local_1c.cksize = 0;
        MVar1 = mmioCreateChunk(*(HMMIO *)(param_1 + 4),&local_1c,0);
        if (MVar1 == 0) {
          LVar3 = mmioWrite(*(HMMIO *)(param_1 + 4),local_8,4);
          if (LVar3 == 4) {
            MVar1 = mmioAscend(*(HMMIO *)(param_1 + 4),&local_1c,0);
            if (MVar1 == 0) {
              uVar2 = 0;
            }
            else {
              uVar2 = 0x80004005;
            }
          }
          else {
            uVar2 = 0x80004005;
          }
        }
        else {
          uVar2 = 0x80004005;
        }
      }
      else {
        uVar2 = 0x80004005;
      }
    }
    else {
      uVar2 = 0x80004005;
    }
  }
  else {
    uVar2 = 0x80004005;
  }
  return uVar2;
}

