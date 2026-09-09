// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7780 | Name: FUN_004b7780


undefined4 * __fastcall FUN_004b7780(undefined4 *param_1)

{
  void *pvVar1;
  int local_8;
  
  param_1[0x3a0ca] = 0;
  param_1[0x3a0cb] = 0;
  param_1[0x3a0cc] = 0;
  param_1[0x3a0cd] = 0;
  param_1[0x3a0ce] = 0;
  *param_1 = &PTR_FUN_005a4684;
  param_1[0xc9] = 0;
  FUN_0058f078(param_1 + 0xc1,s_TRUEVISION_XFILE_005ba69c);
  FUN_0058f078((int)param_1 + 0x317,&DAT_005ba6b0);
  *(undefined1 *)(param_1 + 0xc5) = 0x2e;
  *(undefined1 *)((int)param_1 + 0x315) = 0;
  pvVar1 = operator_new(0x800);
  param_1[0x3a0ca] = pvVar1;
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1[0x3a0ca] + local_8 * 4) = 0;
    FUN_0058f078(param_1 + local_8 * 0x42 + 0x4ca,&DAT_0092e644);
    *(undefined1 *)((int)param_1 + local_8 * 0x108 + 0x1427) = 0x4e;
  }
  _memset(param_1 + 0xca,0,0x1000);
  pvVar1 = operator_new(0x800);
  param_1[0x3a0cb] = pvVar1;
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1[0x3a0cb] + local_8 * 4) = 0;
    FUN_0058f078(param_1 + local_8 * 0x42 + 0x88ca,&DAT_0092e648);
    *(undefined1 *)((int)param_1 + local_8 * 0x108 + 0x22427) = 0x4e;
  }
  pvVar1 = operator_new(0x2000);
  param_1[0x3a0cc] = pvVar1;
  for (local_8 = 0; local_8 < 0x800; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1[0x3a0cc] + local_8 * 4) = 0;
    FUN_0058f078(param_1 + local_8 * 0x42 + 0x10cca,&DAT_0092e64c);
    *(undefined1 *)((int)param_1 + local_8 * 0x108 + 0x43427) = 0x4e;
  }
  pvVar1 = operator_new(0x800);
  param_1[0x3a0cd] = pvVar1;
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1[0x3a0cd] + local_8 * 4) = 0;
    FUN_0058f078(param_1 + local_8 * 0x42 + 0x31cca,&DAT_0092e650);
    *(undefined1 *)((int)param_1 + local_8 * 0x108 + 0xc7427) = 0x4e;
  }
  pvVar1 = operator_new(0x120);
  param_1[0x3a0ce] = pvVar1;
  for (local_8 = 0; local_8 < 0x48; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1[0x3a0ce] + local_8 * 4) = 0;
  }
  _memset(param_1 + 0x3a0cf,0,0x120);
  FUN_004baae7();
  DAT_0092e640 = param_1;
  return param_1;
}

