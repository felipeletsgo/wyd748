// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f08b | Name: FUN_0054f08b


uint FUN_0054f08b(uint param_1)

{
  uint in_EAX;
  uint uVar1;
  undefined4 local_8;
  
  local_8 = param_1 & 0xff;
  uVar1 = in_EAX & 0xffffff00;
  if (((local_8 < 0xb0) || (uVar1 = local_8, 0xc9 < local_8)) || ((byte)(param_1 >> 8) < 0xa1)) {
    uVar1 = uVar1 & 0xffffff00;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

