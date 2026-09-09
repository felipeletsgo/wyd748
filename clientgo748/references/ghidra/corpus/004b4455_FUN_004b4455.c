// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b4455 | Name: FUN_004b4455


undefined4 FUN_004b4455(undefined4 param_1,char *param_2)

{
  size_t sVar1;
  undefined1 local_28 [4];
  undefined1 local_24 [16];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a04b0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_004b6270(local_28);
  local_8 = 1;
  local_14 = 0;
  while (sVar1 = _strlen(param_2), local_14 < sVar1) {
    switch(param_2[local_14]) {
    case '\"':
      FUN_004b63b0(s__quot__005b9efc);
      break;
    default:
      FUN_004b6390(param_2[local_14]);
      break;
    case '&':
      FUN_004b63b0(s__amp__005b9ee4);
      break;
    case '\'':
      FUN_004b63b0(s__apos__005b9f04);
      break;
    case '<':
      FUN_004b63b0(&DAT_005b9eec);
      break;
    case '>':
      FUN_004b63b0(&DAT_005b9ef4);
    }
    local_14 = local_14 + 1;
  }
  FUN_004b62a0(local_24);
  local_8 = local_8 & 0xffffff00;
  FUN_004b6310();
  ExceptionList = local_10;
  return param_1;
}

