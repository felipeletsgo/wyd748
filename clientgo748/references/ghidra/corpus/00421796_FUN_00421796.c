// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00421796 | Name: FUN_00421796


undefined2 FUN_00421796(int param_1)

{
  undefined2 local_8;
  
  local_8 = 0xffff;
  if (param_1 == 2) {
    local_8 = 1;
  }
  else if (param_1 == 4) {
    local_8 = 2;
  }
  else if (param_1 == 8) {
    local_8 = 3;
  }
  else if (param_1 == 0x10) {
    local_8 = 4;
  }
  else if (param_1 == 0x20) {
    local_8 = 5;
  }
  else if (param_1 == 0x80) {
    local_8 = 7;
  }
  else if (param_1 == 0x40) {
    local_8 = 6;
  }
  else if (param_1 == 0x100) {
    local_8 = 8;
  }
  else if (param_1 == 0x200) {
    local_8 = 9;
  }
  else if (param_1 == 0x400) {
    local_8 = 10;
  }
  else if (param_1 == 0x800) {
    local_8 = 0xb;
  }
  else if (param_1 == 0x1000) {
    local_8 = 0xc;
  }
  else if (param_1 == 0x2000) {
    local_8 = 0xd;
  }
  else if (param_1 == 0x4000) {
    local_8 = 0xe;
  }
  else if (param_1 == 0x8000) {
    local_8 = 0xf;
  }
  return local_8;
}

