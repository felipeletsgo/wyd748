// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b3b10 | Name: FUN_004b3b10


void FUN_004b3b10(int param_1,int param_2,int param_3)

{
  int iVar1;
  HANDLE pvVar2;
  BOOL BVar3;
  DWORD DVar4;
  uint nSize;
  LPCVOID pvVar5;
  int *piVar6;
  SIZE_T SVar7;
  SIZE_T *pSVar8;
  _MEMORY_BASIC_INFORMATION local_2c;
  SIZE_T local_10;
  int *local_c;
  int local_8;
  
  DAT_0092e5b0 = DAT_0092e5b0 + 1;
  DAT_0092e5c0 = DAT_0092e5c0 + 1;
  if (DAT_0092e5bc < DAT_0092e5c0) {
    DAT_0092e5bc = DAT_0092e5c0;
  }
  local_8 = FUN_004b3d86(param_1);
  local_c = (int *)(&DAT_0069cfc8 + local_8 * 0x524);
  if (*local_c != 0) {
    DAT_0092e5b4 = DAT_0092e5b4 + 1;
    DAT_0092e5c8 = DAT_0092e5c8 + 1;
    if (DAT_0092e5c4 < DAT_0092e5c8) {
      DAT_0092e5c4 = DAT_0092e5c8;
    }
    for (; local_c[3] != 0; local_c = (int *)local_c[3]) {
    }
    iVar1 = FUN_005916ba(0x524,1);
    local_c[3] = iVar1;
    local_c = (int *)local_c[3];
  }
  *local_c = param_1;
  local_c[1] = param_3;
  local_c[3] = 0;
  local_c[4] = *(int *)(param_2 + 0xb8);
  pSVar8 = (SIZE_T *)(local_c + 5);
  SVar7 = 4;
  piVar6 = local_c + 6;
  pvVar5 = *(LPCVOID *)(param_2 + 0xb8);
  pvVar2 = GetCurrentProcess();
  BVar3 = ReadProcessMemory(pvVar2,pvVar5,piVar6,SVar7,pSVar8);
  if (BVar3 == 0) {
    _memset(local_c + 6,0,4);
    local_c[5] = 0;
    local_c[4] = 0;
  }
  local_c[7] = *(int *)(param_2 + 0xb4);
  pSVar8 = (SIZE_T *)(local_c + 8);
  SVar7 = 0x500;
  piVar6 = local_c + 9;
  pvVar5 = *(LPCVOID *)(param_2 + 0xb4);
  pvVar2 = GetCurrentProcess();
  BVar3 = ReadProcessMemory(pvVar2,pvVar5,piVar6,SVar7,pSVar8);
  if (BVar3 == 0) {
    _memset(local_c + 9,0,0x500);
    local_c[8] = 0;
    local_c[7] = 0;
    DVar4 = GetLastError();
    if ((((DVar4 == 299) &&
         (local_10 = VirtualQuery(*(LPCVOID *)(param_2 + 0xb4),&local_2c,0x1c), local_10 != 0)) &&
        (nSize = (int)local_2c.BaseAddress + (local_2c.RegionSize - *(int *)(param_2 + 0xb4)),
        nSize != 0)) && (nSize < 0x500)) {
      pSVar8 = (SIZE_T *)(local_c + 8);
      piVar6 = local_c + 9;
      pvVar5 = *(LPCVOID *)(param_2 + 0xb4);
      pvVar2 = GetCurrentProcess();
      BVar3 = ReadProcessMemory(pvVar2,pvVar5,piVar6,nSize,pSVar8);
      if (BVar3 == 0) {
        _memset(local_c + 9,0,0x500);
        local_c[8] = 0;
        local_c[7] = 0;
      }
      else {
        local_c[7] = *(int *)(param_2 + 0xb4);
      }
    }
  }
  return;
}

