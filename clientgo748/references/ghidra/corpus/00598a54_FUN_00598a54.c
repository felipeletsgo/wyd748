// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00598a54 | Name: FUN_00598a54


void FUN_00598a54(void)

{
  char cVar1;
  char cVar2;
  char *_Str1;
  DWORD DVar3;
  int iVar4;
  size_t sVar5;
  char *_Source;
  int local_8;
  
  FUN_00596aad(0xc);
  DAT_005cc4f0 = 0xffffffff;
  DAT_005cc4e0 = 0xffffffff;
  DAT_013b80b8 = 0;
  _Str1 = (char *)FUN_0059bba6(&DAT_005aa334);
  if (_Str1 == (char *)0x0) {
    FUN_00596b0e(0xc);
    DVar3 = GetTimeZoneInformation((LPTIME_ZONE_INFORMATION)&DAT_013b80c0);
    if (DVar3 == 0xffffffff) {
      return;
    }
    DAT_005cc448 = DAT_013b80c0 * 0x3c;
    DAT_013b80b8 = 1;
    if (DAT_013b8106 != 0) {
      DAT_005cc448 = DAT_005cc448 + DAT_013b8114 * 0x3c;
    }
    if ((DAT_013b815a == 0) || (DAT_013b8168 == 0)) {
      DAT_005cc44c = 0;
      DAT_005cc450 = 0;
    }
    else {
      DAT_005cc44c = 1;
      DAT_005cc450 = (DAT_013b8168 - DAT_013b8114) * 0x3c;
    }
    iVar4 = WideCharToMultiByte(DAT_013b82f4,0x220,(LPCWSTR)&DAT_013b80c4,-1,PTR_DAT_005cc4d4,0x3f,
                                (LPCSTR)0x0,&local_8);
    if ((iVar4 == 0) || (local_8 != 0)) {
      *PTR_DAT_005cc4d4 = 0;
    }
    else {
      PTR_DAT_005cc4d4[0x3f] = 0;
    }
    iVar4 = WideCharToMultiByte(DAT_013b82f4,0x220,(LPCWSTR)&DAT_013b8118,-1,PTR_DAT_005cc4d8,0x3f,
                                (LPCSTR)0x0,&local_8);
    if ((iVar4 != 0) && (local_8 == 0)) {
      PTR_DAT_005cc4d8[0x3f] = 0;
      return;
    }
LAB_00598cc5:
    *PTR_DAT_005cc4d8 = 0;
  }
  else {
    if ((*_Str1 != '\0') &&
       ((DAT_013b816c == (char *)0x0 || (iVar4 = _strcmp(_Str1,DAT_013b816c), iVar4 != 0)))) {
      FUN_0058ffd8(DAT_013b816c);
      sVar5 = _strlen(_Str1);
      DAT_013b816c = _malloc(sVar5 + 1);
      if (DAT_013b816c != (char *)0x0) {
        FUN_0058ee20(DAT_013b816c,_Str1);
        FUN_00596b0e(0xc);
        _strncpy(PTR_DAT_005cc4d4,_Str1,3);
        _Source = _Str1 + 3;
        PTR_DAT_005cc4d4[3] = 0;
        cVar1 = *_Source;
        if (cVar1 == '-') {
          _Source = _Str1 + 4;
        }
        iVar4 = FUN_005910cf(_Source);
        DAT_005cc448 = iVar4 * 0xe10;
        for (; (cVar2 = *_Source, cVar2 == '+' || (('/' < cVar2 && (cVar2 < ':'))));
            _Source = _Source + 1) {
        }
        if (*_Source == ':') {
          _Source = _Source + 1;
          iVar4 = FUN_005910cf(_Source);
          DAT_005cc448 = DAT_005cc448 + iVar4 * 0x3c;
          for (; ('/' < *_Source && (*_Source < ':')); _Source = _Source + 1) {
          }
          if (*_Source == ':') {
            _Source = _Source + 1;
            iVar4 = FUN_005910cf(_Source);
            DAT_005cc448 = DAT_005cc448 + iVar4;
            for (; ('/' < *_Source && (*_Source < ':')); _Source = _Source + 1) {
            }
          }
        }
        if (cVar1 == '-') {
          DAT_005cc448 = -DAT_005cc448;
        }
        DAT_005cc44c = (int)*_Source;
        if (DAT_005cc44c != 0) {
          _strncpy(PTR_DAT_005cc4d8,_Source,3);
          PTR_DAT_005cc4d8[3] = 0;
          return;
        }
        goto LAB_00598cc5;
      }
    }
    FUN_00596b0e(0xc);
  }
  return;
}

