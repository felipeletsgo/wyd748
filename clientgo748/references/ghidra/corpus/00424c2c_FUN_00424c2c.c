// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00424c2c | Name: FUN_00424c2c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00424c2c(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  uint uVar2;
  undefined1 local_14;
  undefined4 local_c;
  
  local_c = 0;
  if (*(char *)(param_1 + 0x20) != '\0') {
    if (*(int *)(param_1 + 0x40) < 0x10) {
      if (*(char *)(param_1 + 0x20) != '\0') {
        if (*(int *)(param_1 + 0x40) < 0x10) {
          local_c = (int)*(char *)(param_1 + 0x20 + *(int *)(param_1 + 0x40)) ^ 0xff;
          *(int *)(param_1 + 0x40) = *(int *)(param_1 + 0x40) + 1;
        }
        else {
          uVar2 = (int)*(char *)(param_1 + 0x2f) & 0x80000001;
          if ((int)uVar2 < 0) {
            uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
          }
          if (uVar2 == 0) {
            local_c = (int)*(char *)(param_1 + 0x21) + (int)*(char *)(param_1 + 0x23) + -0x57 +
                      (int)*(char *)(param_1 + 0x25);
          }
          else {
            local_c = (((int)*(char *)(param_1 + 0x2d) + (int)*(char *)(param_1 + 0x2b)) -
                      (int)*(char *)(param_1 + 0x29)) + 4;
          }
          local_c = local_c ^ 0xff;
        }
      }
    }
    else {
      if (_DAT_005ccf88 == 0) {
        uVar2 = (int)*(char *)(param_1 + 0x2f) & 0x80000001;
        if ((int)uVar2 < 0) {
          uVar2 = (uVar2 - 1 | 0xfffffffe) + 1;
        }
        if (uVar2 == 0) {
          local_c = (int)*(char *)(param_1 + 0x21) + (int)*(char *)(param_1 + 0x23) + -0x57 +
                    (int)*(char *)(param_1 + 0x25);
        }
        else {
          local_c = (((int)*(char *)(param_1 + 0x2d) + (int)*(char *)(param_1 + 0x2b)) -
                    (int)*(char *)(param_1 + 0x29)) + 4;
        }
      }
      else {
        local_14 = DAT_005ccf88;
        if (DAT_005ccf88 == '\0') {
          local_14 = DAT_005ccf8b;
        }
        if (local_14 == '\0') {
          local_14 = '\r';
        }
        local_c = ((int)DAT_005ccf8a + (int)DAT_005ccf8b) - (int)DAT_005ccf89 * (int)local_14;
        if (local_c == 0) {
          local_c = (uint)DAT_005ccf88;
        }
      }
      local_c = local_c ^ 0xff;
    }
  }
  uVar1 = FUN_00424dfe(param_2,param_3,local_c);
  return uVar1;
}

