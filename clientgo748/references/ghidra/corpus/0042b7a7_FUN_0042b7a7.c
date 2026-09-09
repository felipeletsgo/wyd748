// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042b7a7 | Name: FUN_0042b7a7


undefined4 __fastcall FUN_0042b7a7(int *param_1)

{
  MMRESULT MVar1;
  undefined4 uVar2;
  LONG LVar3;
  void *pvVar4;
  uint uVar5;
  uint local_2c;
  uint local_28 [4];
  _MMCKINFO local_18;
  
  *param_1 = 0;
  MVar1 = mmioDescend((HMMIO)param_1[1],(LPMMCKINFO)(param_1 + 7),(MMCKINFO *)0x0,0);
  if (MVar1 == 0) {
    if ((param_1[7] == 0x46464952) && (param_1[9] == 0x45564157)) {
      local_18.ckid = 0x20746d66;
      MVar1 = mmioDescend((HMMIO)param_1[1],&local_18,(MMCKINFO *)(param_1 + 7),0x10);
      if (MVar1 == 0) {
        if (local_18.cksize < 0x10) {
          uVar2 = 0x80004005;
        }
        else {
          LVar3 = mmioRead((HMMIO)param_1[1],(HPSTR)local_28,0x10);
          if (LVar3 == 0x10) {
            if ((local_28[0] & 0xffff) == 1) {
              pvVar4 = operator_new(0x12);
              *param_1 = (int)pvVar4;
              if (*param_1 == 0) {
                return 0x80004005;
              }
              FUN_0058f220(*param_1,local_28,0x10);
              *(undefined2 *)(*param_1 + 0x10) = 0;
            }
            else {
              local_2c = local_2c & 0xffff0000;
              LVar3 = mmioRead((HMMIO)param_1[1],(HPSTR)&local_2c,2);
              if (LVar3 != 2) {
                return 0x80004005;
              }
              pvVar4 = operator_new((local_2c & 0xffff) + 0x12);
              *param_1 = (int)pvVar4;
              if (*param_1 == 0) {
                return 0x80004005;
              }
              FUN_0058f220(*param_1,local_28,0x10);
              *(undefined2 *)(*param_1 + 0x10) = (undefined2)local_2c;
              uVar5 = mmioRead((HMMIO)param_1[1],(HPSTR)(*param_1 + 0x12),local_2c & 0xffff);
              if (uVar5 != (local_2c & 0xffff)) {
                if (*param_1 != 0) {
                  FUN_0058ea3a(*param_1);
                  *param_1 = 0;
                }
                return 0x80004005;
              }
            }
            MVar1 = mmioAscend((HMMIO)param_1[1],&local_18,0);
            if (MVar1 == 0) {
              uVar2 = 0;
            }
            else {
              if (*param_1 != 0) {
                FUN_0058ea3a(*param_1);
                *param_1 = 0;
              }
              uVar2 = 0x80004005;
            }
          }
          else {
            uVar2 = 0x80004005;
          }
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

