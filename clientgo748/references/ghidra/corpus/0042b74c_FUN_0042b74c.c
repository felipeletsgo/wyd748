// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042b74c | Name: FUN_0042b74c


undefined4 __thiscall
FUN_0042b74c(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            int param_5)

{
  undefined4 uVar1;
  
  *param_1 = param_4;
  param_1[0x23] = param_3;
  param_1[0x21] = param_2;
  param_1[0x22] = param_1[0x21];
  param_1[0x20] = 1;
  if (param_5 == 1) {
    uVar1 = 0;
  }
  else {
    uVar1 = 0x80004001;
  }
  return uVar1;
}

