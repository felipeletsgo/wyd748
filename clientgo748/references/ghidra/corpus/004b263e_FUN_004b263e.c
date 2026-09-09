// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b263e | Name: FUN_004b263e


void __thiscall FUN_004b263e(int param_1,undefined4 param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  int *local_8;
  
  piVar1 = DAT_0067cf38;
  local_8 = DAT_0067cf38;
  if (DAT_0067cf38 != (int *)0x0) {
    if (((DAT_0067cf38[9] != 0x7531) || (param_3 == 0)) || (*(short *)(param_3 + 4) != 0x182)) {
      do {
        if ((char)local_8[5] == '\0') {
          iVar2 = (**(code **)(*local_8 + 4))(param_2,param_3);
          if (iVar2 == 1) {
            return;
          }
          if (local_8[4] == 0) goto LAB_004b26fc;
          local_8 = (int *)local_8[4];
        }
        else {
LAB_004b26fc:
          while (local_8[3] == 0) {
            local_8 = (int *)local_8[1];
            if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_004b2727;
          }
          local_8 = (int *)local_8[3];
        }
LAB_004b2727:
        if (local_8 == piVar1) {
          return;
        }
        if (local_8 == (int *)0x0) {
          return;
        }
      } while( true );
    }
    if (*(short *)(param_3 + 0xc) == 2) {
      FUN_0058f220(param_1 + 0x2ec + *(short *)(param_3 + 0xe) * 8,param_3 + 0x10,8);
    }
  }
  return;
}

