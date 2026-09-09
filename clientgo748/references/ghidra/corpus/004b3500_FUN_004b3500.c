// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b3500 | Name: FUN_004b3500


void __thiscall FUN_004b3500(int *param_1,int param_2)

{
  void *pvVar1;
  int iVar2;
  int *local_54;
  int *local_50;
  int *local_4c;
  int *local_48;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a047c;
  local_10 = ExceptionList;
  if (param_2 == 9) {
    ExceptionList = &local_10;
    param_1[0x6c20] = 0;
  }
  else {
    if (param_1[0x6c20] == param_2) {
      return;
    }
    ExceptionList = &local_10;
    param_1[0x6c20] = param_2;
  }
  FUN_004ef8e0();
  local_14 = (int *)0x0;
  switch(param_1[0x6c20]) {
  case 0:
    pvVar1 = operator_new(0x28ecc);
    local_8 = 3;
    if (pvVar1 == (void *)0x0) {
      local_54 = (int *)0x0;
    }
    else {
      local_54 = (int *)FUN_004343a4();
    }
    local_8 = 0xffffffff;
    local_14 = local_54;
    break;
  case 5:
    pvVar1 = operator_new(0x26efc);
    local_8 = 2;
    if (pvVar1 == (void *)0x0) {
      local_50 = (int *)0x0;
    }
    else {
      local_50 = (int *)FUN_0049ee30();
    }
    local_8 = 0xffffffff;
    local_14 = local_50;
    break;
  case 7:
    pvVar1 = operator_new(0x27d34);
    local_8 = 0;
    if (pvVar1 == (void *)0x0) {
      local_48 = (int *)0x0;
    }
    else {
      local_48 = (int *)FUN_004a8ccf();
    }
    local_8 = 0xffffffff;
    local_14 = local_48;
    break;
  case 8:
    pvVar1 = operator_new(0x3a780);
    local_8 = 1;
    if (pvVar1 == (void *)0x0) {
      local_4c = (int *)0x0;
    }
    else {
      local_4c = (int *)FUN_00431d00();
    }
    local_8 = 0xffffffff;
    local_14 = local_4c;
  }
  if (local_14 == (int *)0x0) {
    MessageBoxA(*(HWND *)(DAT_013b71e0 + 4),s_Create_Scene_Fail__005b9ae8,s_Error_005b9ae0,0x1000);
    PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
  }
  else {
    (**(code **)(*param_1 + 0x58))(local_14);
    (**(code **)(*DAT_005ccec0 + 0x8c))(0);
    iVar2 = (**(code **)(*local_14 + 0x4c))();
    if (iVar2 == 0) {
      if ((local_14 != (int *)0x0) && (local_14 != (int *)0x0)) {
        (**(code **)*local_14)(1);
      }
      MessageBoxA(*(HWND *)(DAT_013b71e0 + 4),s_Initialize_Scene_Fail__005b9b04,s_Error_005b9afc,
                  0x1000);
      PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
    }
    else {
      FUN_0054ac09(local_14);
    }
  }
  ExceptionList = local_10;
  return;
}

