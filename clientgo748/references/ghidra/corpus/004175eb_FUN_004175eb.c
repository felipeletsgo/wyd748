// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004175eb | Name: FUN_004175eb


undefined4 FUN_004175eb(int param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  
  if (param_1 < 0x40) {
    if ((param_2 == 0x3c) || (param_2 == 0x44)) {
      if (param_1 == 2) {
        if (param_3 < 0xc) {
          uVar1 = 0xff99ee99;
        }
        else if ((param_3 < 0xc) || (0xe < param_3)) {
          uVar1 = 0xffffaa00;
        }
        else {
          uVar1 = 0xffffffaa;
        }
      }
      else if (param_3 < 6) {
        uVar1 = 0xff99ee99;
      }
      else if (param_3 == 6) {
        uVar1 = 0xffffffaa;
      }
      else {
        uVar1 = 0xffffaa00;
      }
    }
    else if ((param_2 == 0x2a) || (param_2 == 0x47)) {
      if (param_3 < 0x32) {
        uVar1 = 0xff99ee99;
      }
      else if ((param_3 < 0x32) || (0x3b < param_3)) {
        uVar1 = 0xffffaa00;
      }
      else {
        uVar1 = 0xffffffaa;
      }
    }
    else if (param_2 == 0x1a) {
      if (param_3 < 0xc) {
        uVar1 = 0xff99ee99;
      }
      else if (param_3 == 0xc) {
        uVar1 = 0xffffffaa;
      }
      else {
        uVar1 = 0xffffaa00;
      }
    }
    else if (param_2 == 0x4a) {
      if (param_3 < 0xc) {
        uVar1 = 0xff99ee99;
      }
      else if (param_3 == 0xc) {
        uVar1 = 0xffffffaa;
      }
      else {
        uVar1 = 0xffffaa00;
      }
    }
    else if (((param_2 == 3) || (param_2 == 0x35)) || (param_2 == 0x48)) {
      if (param_1 == 0x10) {
        if (param_3 < 0x1e) {
          uVar1 = 0xff99ee99;
        }
        else if (param_3 == 0x1e) {
          uVar1 = 0xffffffaa;
        }
        else {
          uVar1 = 0xffffaa00;
        }
      }
      else if (param_3 < 0xf) {
        uVar1 = 0xff99ee99;
      }
      else if (param_3 == 0xf) {
        uVar1 = 0xffffffaa;
      }
      else {
        uVar1 = 0xffffaa00;
      }
    }
    else if (((param_2 == 2) || (param_2 == 0x49)) || (param_2 == 0x43)) {
      if (param_1 == 0x20) {
        if (param_3 < 0x18) {
          uVar1 = 0xff99ee99;
        }
        else if ((param_3 < 0x18) || (0x1e < param_3)) {
          uVar1 = 0xffffaa00;
        }
        else {
          uVar1 = 0xffffffaa;
        }
      }
      else if (param_3 < 0x12) {
        uVar1 = 0xff99ee99;
      }
      else if (param_3 == 0x12) {
        uVar1 = 0xffffffaa;
      }
      else {
        uVar1 = 0xffffaa00;
      }
    }
    else if (((param_2 == 4) || (param_2 == 0x2d)) || (param_2 == 0x45)) {
      if (param_3 < 0x28) {
        uVar1 = 0xff99ee99;
      }
      else if (param_3 == 0x28) {
        uVar1 = 0xffffffaa;
      }
      else {
        uVar1 = 0xffffaa00;
      }
    }
    else {
      uVar1 = 0xff99ee99;
    }
  }
  else if (((param_1 == 0x40) || (param_1 == 0x80)) || (param_1 == 0xc0)) {
    if (((param_2 == 2) || (param_2 == 0x49)) || (param_2 == 0x43)) {
      if (param_3 < 0x2d) {
        uVar1 = 0xff99ee99;
      }
      else if ((param_3 < 0x2d) || (0x36 < param_3)) {
        uVar1 = 0xffffaa00;
      }
      else {
        uVar1 = 0xffffffaa;
      }
    }
    else if ((param_2 == 0x3c) || (param_2 == 0x44)) {
      if (param_3 < 0x14) {
        uVar1 = 0xff99ee99;
      }
      else if ((param_3 < 0x14) || (0x18 < param_3)) {
        uVar1 = 0xffffaa00;
      }
      else {
        uVar1 = 0xffffffaa;
      }
    }
    else if (param_2 == 0x1a) {
      if (param_3 < 0x15) {
        uVar1 = 0xff99ee99;
      }
      else if ((param_3 < 0x15) || (0x18 < param_3)) {
        uVar1 = 0xffffaa00;
      }
      else {
        uVar1 = 0xffffffaa;
      }
    }
    else if (param_2 == 0x4a) {
      if (param_3 < 0x15) {
        uVar1 = 0xff99ee99;
      }
      else if ((param_3 < 0x15) || (0x18 < param_3)) {
        uVar1 = 0xffffaa00;
      }
      else {
        uVar1 = 0xffffffaa;
      }
    }
    else {
      uVar1 = 0xff99ee99;
    }
  }
  else {
    uVar1 = 0xff99ee99;
  }
  return uVar1;
}

