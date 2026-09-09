// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00431d00 | Name: FUN_00431d00


undefined4 * __fastcall FUN_00431d00(undefined4 *param_1)

{
  void *pvVar1;
  undefined4 local_2c;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f0e4;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00493e70();
  local_8 = 0;
  FUN_00421910(param_1 + 0x9e16,0x10,800,FUN_00433a40);
  *param_1 = &PTR_FUN_005a421c;
  param_1[9] = 0x7535;
  *(undefined1 *)(param_1 + 0x9b8b) = 0;
  _memset(param_1 + 0xaa96,0,800);
  _memset(param_1 + 0x9e16,0,0x3200);
  DAT_005b892c = 1;
  if (*(int *)(DAT_005ccf9c + 0x2a708) != 0) {
    if (*(undefined4 **)(DAT_005ccf9c + 0x2a708) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(DAT_005ccf9c + 0x2a708))(1);
    }
    *(undefined4 *)(DAT_005ccf9c + 0x2a708) = 0;
  }
  FUN_0042d600();
  if (*(int *)(DAT_013b71e0 + 0x10) == 0x280) {
    DAT_005b4914 = 10;
    DAT_005b4918 = 0x100;
  }
  if (*(int *)(DAT_013b71e0 + 0x10) == 800) {
    DAT_005b4914 = 0xc;
    DAT_005b4918 = 0x100;
  }
  if (*(int *)(DAT_013b71e0 + 0x10) == 0x400) {
    DAT_005b4914 = 0xe;
    DAT_005b4918 = 0x200;
  }
  if (*(int *)(DAT_013b71e0 + 0x10) == 0x500) {
    DAT_005b4914 = 0x14;
    DAT_005b4918 = 0x200;
  }
  if (*(int *)(DAT_013b71e0 + 0x10) == 0x640) {
    DAT_005b4914 = 0x18;
    DAT_005b4918 = 0x200;
  }
  pvVar1 = operator_new(0x1b0);
  local_8._0_1_ = 1;
  if (pvVar1 == (void *)0x0) {
    local_2c = 0;
  }
  else {
    local_2c = FUN_004219a0();
  }
  local_8 = (uint)local_8._1_3_ << 8;
  *(undefined4 *)(DAT_005ccf9c + 0x2a708) = local_2c;
  FUN_0042ced1();
  ExceptionList = local_10;
  return param_1;
}

