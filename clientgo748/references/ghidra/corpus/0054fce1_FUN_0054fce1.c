// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054fce1 | Name: FUN_0054fce1


void FUN_0054fce1(void *param_1)

{
  int local_8;
  
  _memset(param_1,0,0x2f4);
  *(undefined2 *)((int)param_1 + 0x20) = 0x840;
  *(undefined2 *)((int)param_1 + 0x22) = 0x840;
  _memset((void *)((int)param_1 + 0x24),0,0x1c);
  _memset((void *)((int)param_1 + 0x40),0,0x1c);
  for (local_8 = 0; local_8 < 0x10; local_8 = local_8 + 1) {
    FUN_00552fdf((int)param_1 + local_8 * 8 + 0x5c);
  }
  for (local_8 = 0; local_8 < 0x40; local_8 = local_8 + 1) {
    FUN_00552fdf((int)param_1 + local_8 * 8 + 0xdc);
  }
  *(undefined1 *)((int)param_1 + 0x2e8) = 0xff;
  *(undefined1 *)((int)param_1 + 0x2e9) = 0xff;
  *(undefined1 *)((int)param_1 + 0x2ea) = 0xff;
  *(undefined1 *)((int)param_1 + 0x2eb) = 0xff;
  return;
}

