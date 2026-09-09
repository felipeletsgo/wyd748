// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059b902 | Name: FUN_0059b902


uint FUN_0059b902(LPWSTR param_1,byte *param_2,uint param_3)

{
  byte bVar1;
  int iVar2;
  undefined4 *puVar3;
  
  if ((param_2 != (byte *)0x0) && (param_3 != 0)) {
    bVar1 = *param_2;
    if (bVar1 != 0) {
      if (DAT_013b82e4 == 0) {
        if (param_1 != (LPWSTR)0x0) {
          *param_1 = (ushort)bVar1;
        }
        return 1;
      }
      if ((PTR_DAT_005cc110[(uint)bVar1 * 2 + 1] & 0x80) == 0) {
        iVar2 = MultiByteToWideChar(DAT_013b82f4,9,(LPCSTR)param_2,1,param_1,
                                    (uint)(param_1 != (LPWSTR)0x0));
        if (iVar2 != 0) {
          return 1;
        }
      }
      else {
        if (1 < (int)DAT_005cc31c) {
          if ((int)param_3 < (int)DAT_005cc31c) goto LAB_0059b994;
          iVar2 = MultiByteToWideChar(DAT_013b82f4,9,(LPCSTR)param_2,DAT_005cc31c,param_1,
                                      (uint)(param_1 != (LPWSTR)0x0));
          if (iVar2 != 0) {
            return DAT_005cc31c;
          }
        }
        if ((DAT_005cc31c <= param_3) && (param_2[1] != 0)) {
          return DAT_005cc31c;
        }
      }
LAB_0059b994:
      puVar3 = (undefined4 *)FUN_00594c1d();
      *puVar3 = 0x2a;
      return 0xffffffff;
    }
    if (param_1 != (LPWSTR)0x0) {
      *param_1 = L'\0';
    }
  }
  return 0;
}

