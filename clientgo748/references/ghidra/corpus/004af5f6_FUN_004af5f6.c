// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004af5f6 | Name: FUN_004af5f6


void __thiscall FUN_004af5f6(int param_1,undefined4 param_2,int param_3)

{
  HGLOBAL pvVar1;
  undefined1 local_a4 [8];
  int local_9c;
  undefined1 local_98 [128];
  uint local_18;
  int *local_14;
  SIZE_T local_10;
  HIMC local_c;
  int local_8;
  
  local_8 = DAT_0067cf38;
  if ((DAT_0067cf38 != 0) && (*(int *)(DAT_0067cf38 + 0x5c) != 0)) {
    if (param_3 == 0) {
      (**(code **)(**(int **)(DAT_0067cf38 + 0x5c) + 0x60))(0);
    }
    else {
      if (*(int *)(param_1 + 0xa58) == 0) {
        pvVar1 = GlobalAlloc(0x40,0x1c);
        *(HGLOBAL *)(param_1 + 0xa58) = pvVar1;
      }
      local_c = ImmGetContext(*(HWND *)(param_1 + 0x444));
      if (local_c != (HIMC)0x0) {
        local_10 = ImmGetCandidateListA(local_c,0,(LPCANDIDATELIST)0x0,0);
        if (local_10 == 0) {
          _memset(*(void **)(param_1 + 0xa58),0,0x1c);
        }
        else {
          GlobalFree(*(HGLOBAL *)(param_1 + 0xa58));
          pvVar1 = GlobalAlloc(0x40,local_10);
          *(HGLOBAL *)(param_1 + 0xa58) = pvVar1;
          ImmGetCandidateListA(local_c,0,*(LPCANDIDATELIST *)(param_1 + 0xa58),local_10);
          local_14 = (int *)(*(int *)(param_1 + 0xa58) + 0x18 +
                            *(int *)(*(int *)(param_1 + 0xa58) + 0x10) * 4);
          _memset(local_98,0,0x80);
          for (local_18 = *(uint *)(*(int *)(param_1 + 0xa58) + 0x10);
              (local_18 < *(uint *)(*(int *)(param_1 + 0xa58) + 8) &&
              (local_18 <
               (uint)(*(int *)(*(int *)(param_1 + 0xa58) + 0x10) +
                     *(int *)(*(int *)(param_1 + 0xa58) + 0x14)))); local_18 = local_18 + 1) {
            local_9c = *(int *)(param_1 + 0xa58) + *local_14;
            local_14 = local_14 + 1;
            FUN_0058f078(local_a4,s__d__s_005b9888,
                         (local_18 + 1) - *(int *)(*(int *)(param_1 + 0xa58) + 0x10),local_9c);
            FUN_0058ee30(local_98,local_a4);
          }
          (**(code **)(**(int **)(local_8 + 0x5c) + 0x80))(local_98,0);
        }
        (**(code **)(**(int **)(local_8 + 0x5c) + 0x60))(1);
        ImmReleaseContext(*(HWND *)(param_1 + 0x444),local_c);
      }
    }
  }
  return;
}

