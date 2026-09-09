// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423ca0 | Name: FUN_00423ca0


undefined4 * __fastcall FUN_00423ca0(undefined4 *param_1)

{
  void *pvVar1;
  
  *param_1 = 0;
  param_1[7] = 0;
  pvVar1 = _malloc(0x20000);
  param_1[1] = pvVar1;
  pvVar1 = _malloc(0x20000);
  param_1[2] = pvVar1;
  param_1[3] = 0;
  param_1[6] = 0;
  param_1[4] = 0;
  param_1[5] = 0;
  _memset(param_1 + 8,0,0x10);
  _memset(param_1 + 0xc,0,0x10);
  param_1[0x10] = 0;
  return param_1;
}

