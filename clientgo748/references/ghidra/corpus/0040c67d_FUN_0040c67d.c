// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040c67d | Name: FUN_0040c67d


int __thiscall
FUN_0040c67d(int *param_1,undefined4 param_2,undefined4 param_3,int param_4,int param_5)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int local_24;
  int local_1c;
  int local_18;
  int local_10;
  int *local_8;
  
  if (*(int *)(param_1[0xc] + 0x28) != 0) {
    (**(code **)(*(int *)param_1[0xc] + 8))(param_2,param_3,param_4,param_5);
  }
  local_10 = 0;
  local_18 = 0;
  local_24 = 0;
  local_8 = (int *)param_1[10];
  for (local_1c = 0; local_1c < 8; local_1c = local_1c + 1) {
    iVar3 = param_1[local_1c + 0x4d];
    if (((iVar3 != 0) && (*(int *)(iVar3 + 0x28) == 1)) && (*(int *)(iVar3 + 0x60) == 1)) {
      local_8 = (int *)param_1[local_1c + 0x4d];
      break;
    }
  }
  piVar1 = local_8;
  if (local_8 == (int *)0x0) {
    local_24 = 1;
  }
  else {
    do {
      if (((char)local_8[5] == '\0') && (local_8[10] != 0)) {
        iVar3 = local_8[0xc];
        iVar2 = (**(code **)(*local_8 + 8))(param_2,param_3,param_4 - local_10,param_5 - local_18);
        if (((local_8[0xc] != 0) && ((iVar3 == 0 && (iVar2 == 1)))) && (local_8[0x10] == 0xd)) {
          (**(code **)(*param_1 + 0x40))(local_8);
        }
        if (iVar2 == 1) {
          local_24 = 1;
        }
        if (local_8[4] == 0) goto LAB_0040c80c;
        iVar3 = __ftol();
        local_10 = local_10 + iVar3;
        iVar3 = __ftol();
        local_18 = local_18 + iVar3;
        local_8 = (int *)local_8[4];
      }
      else {
LAB_0040c80c:
        do {
          if (local_8[3] != 0) {
            local_8 = (int *)local_8[3];
            break;
          }
          local_8 = (int *)local_8[1];
          if (local_8 == (int *)0x0) break;
          iVar3 = __ftol();
          local_10 = local_10 - iVar3;
          iVar3 = __ftol();
          local_18 = local_18 - iVar3;
        } while (local_8 != piVar1);
      }
    } while ((local_8 != piVar1) && (local_8 != (int *)0x0));
    if ((local_24 == 0) && (*(int **)(DAT_0067cf38 + 0x58) != (int *)0x0)) {
      (**(code **)(**(int **)(DAT_0067cf38 + 0x58) + 0x60))(0);
    }
  }
  return local_24;
}

