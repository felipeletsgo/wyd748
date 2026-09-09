// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040bea0 | Name: FUN_0040bea0


undefined4 * __thiscall
FUN_0040bea0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
            undefined4 param_9)

{
  *param_1 = param_2;
  param_1[1] = param_4;
  param_1[2] = param_5;
  param_1[3] = param_6;
  param_1[4] = param_7;
  param_1[6] = 0;
  param_1[7] = 0x3f800000;
  param_1[0xc] = param_9;
  param_1[0xd] = 1;
  param_1[0xe] = 0;
  param_1[0xf] = param_3;
  param_1[0x10] = 0;
  *(undefined2 *)(param_1 + 0x11) = 0;
  param_1[0x52] = 0xffffffff;
  param_1[0x53] = 0;
  param_1[0x54] = 0;
  param_1[0x57] = 0;
  *(undefined1 *)((int)param_1 + 0x46) = 0;
  param_1[0x58] = 0xffffffff;
  param_1[0x59] = 3;
  param_1[0x5a] = 0;
  return param_1;
}

