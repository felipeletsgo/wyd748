// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7170 | Name: FUN_004b7170


void FUN_004b7170(uint param_1)

{
  uint uVar1;
  uint local_20;
  void *pvStack_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a05f0;
  pvStack_10 = ExceptionList;
  local_20 = param_1 | 0x1f;
  ExceptionList = &pvStack_10;
  uVar1 = FID_conflict_max_size();
  if (uVar1 < local_20) {
    local_20 = param_1;
  }
  local_8 = 0;
  FUN_004b73c0(local_20 + 2,0);
  FUN_004b71f3();
  return;
}

