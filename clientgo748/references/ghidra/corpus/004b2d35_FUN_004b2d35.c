// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2d35 | Name: FUN_004b2d35


void __thiscall FUN_004b2d35(int *param_1,undefined4 param_2)

{
  int *piVar1;
  int iVar2;
  int *local_8;
  
  piVar1 = DAT_0067cf38;
  if (DAT_0067cf38 != (int *)0x0) {
    DAT_0067cf3c = 0;
    local_8 = DAT_0067cf38;
    if (DAT_0067cf38 != (int *)0x0) {
      do {
        if ((char)local_8[5] == '\0') {
          iVar2 = (**(code **)(*local_8 + 0x20))(param_2);
          DAT_0067cf3c = DAT_0067cf3c + 1;
          if ((iVar2 == 0) || (local_8[4] == 0)) goto LAB_004b2dc4;
          local_8 = (int *)local_8[4];
        }
        else {
          param_1[0x6c23] = 1;
LAB_004b2dc4:
          while (local_8[3] == 0) {
            local_8 = (int *)local_8[1];
            if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_004b2def;
          }
          local_8 = (int *)local_8[3];
        }
LAB_004b2def:
      } while ((local_8 != piVar1) && (local_8 != (int *)0x0));
      if (*(int *)(DAT_005ccf9c + 0x2b03c) != 0) {
        (**(code **)(*param_1 + 0x2c))(DAT_0067cf38[0xb],param_2);
      }
      if (-1 < DAT_005d040c) {
        DAT_005d040c = DAT_005d040c + -1;
      }
    }
  }
  return;
}

