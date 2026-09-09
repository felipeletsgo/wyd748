// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00480c5d | Name: FUN_00480c5d


void FUN_00480c5d(int param_1,char param_2)

{
  int local_1ac [106];
  
  if (param_1 != 0) {
    local_1ac[0x69] = 0xdd;
    local_1ac[0x68] = 0xd8;
    if (*(uint *)(param_1 + 0x20) < 1000) {
      local_1ac[0] = 1;
      local_1ac[1] = 5;
      local_1ac[2] = 3;
      local_1ac[3] = 5;
      local_1ac[4] = 1;
      local_1ac[5] = 5;
      local_1ac[6] = 5;
      local_1ac[7] = 5;
      local_1ac[8] = 4;
      local_1ac[9] = 0;
      local_1ac[10] = 5;
      local_1ac[0xb] = 0;
      local_1ac[0xc] = 0;
      local_1ac[0xd] = 5;
      local_1ac[0xe] = 5;
      local_1ac[0xf] = 5;
      local_1ac[0x10] = 2;
      local_1ac[0x11] = 2;
      local_1ac[0x12] = 4;
      local_1ac[0x13] = 1;
      local_1ac[0x14] = 3;
      local_1ac[0x15] = 2;
      local_1ac[0x16] = 4;
      local_1ac[0x17] = 5;
      local_1ac[0x18] = 1;
      local_1ac[0x19] = 5;
      local_1ac[0x1a] = 3;
      local_1ac[0x1b] = 1;
      local_1ac[0x1c] = 5;
      local_1ac[0x1d] = 5;
      local_1ac[0x1e] = 5;
      local_1ac[0x1f] = 5;
      local_1ac[0x20] = 3;
      local_1ac[0x21] = 1;
      local_1ac[0x22] = 1;
      local_1ac[0x23] = 3;
      local_1ac[0x24] = 1;
      local_1ac[0x25] = 0;
      local_1ac[0x26] = 0;
      local_1ac[0x27] = 5;
      local_1ac[0x28] = 2;
      local_1ac[0x29] = 0;
      local_1ac[0x2a] = 5;
      local_1ac[0x2b] = 1;
      local_1ac[0x2c] = 1;
      local_1ac[0x2d] = 5;
      local_1ac[0x2e] = 1;
      local_1ac[0x2f] = 5;
      local_1ac[0x30] = 0;
      local_1ac[0x31] = 0;
      local_1ac[0x32] = 1;
      local_1ac[0x33] = 1;
      local_1ac[0x34] = 0;
      local_1ac[0x35] = 1;
      local_1ac[0x36] = 5;
      local_1ac[0x37] = 5;
      local_1ac[0x38] = 1;
      local_1ac[0x39] = 0;
      local_1ac[0x3a] = 3;
      local_1ac[0x3b] = 0;
      local_1ac[0x3c] = 0;
      local_1ac[0x3d] = 0;
      local_1ac[0x3e] = 5;
      local_1ac[0x3f] = 5;
      local_1ac[0x40] = 2;
      local_1ac[0x41] = 5;
      local_1ac[0x42] = 2;
      local_1ac[0x43] = 5;
      local_1ac[0x44] = 0;
      local_1ac[0x45] = 5;
      local_1ac[0x46] = 5;
      local_1ac[0x47] = 5;
      local_1ac[0x48] = 0;
      local_1ac[0x49] = 2;
      local_1ac[0x4a] = 1;
      local_1ac[0x4b] = 0;
      local_1ac[0x4c] = 2;
      local_1ac[0x4d] = 1;
      local_1ac[0x4e] = 5;
      local_1ac[0x4f] = 5;
      local_1ac[0x50] = 5;
      local_1ac[0x51] = 2;
      local_1ac[0x52] = 3;
      local_1ac[0x53] = 2;
      local_1ac[0x54] = 3;
      local_1ac[0x55] = 5;
      local_1ac[0x56] = 5;
      local_1ac[0x57] = 5;
      local_1ac[0x58] = 0;
      local_1ac[0x59] = 5;
      local_1ac[0x5a] = 1;
      local_1ac[0x5b] = 2;
      local_1ac[0x5c] = 5;
      local_1ac[0x5d] = 5;
      local_1ac[0x5e] = 5;
      local_1ac[0x5f] = 5;
      local_1ac[0x60] = 1;
      local_1ac[0x61] = 5;
      local_1ac[0x62] = 5;
      local_1ac[99] = 5;
      local_1ac[100] = 5;
      local_1ac[0x65] = 0;
      local_1ac[0x66] = 0;
      local_1ac[0x67] = 0;
      if ((-1 < param_2) && (param_2 < 'h')) {
        local_1ac[0x69] = local_1ac[param_2] + 0xd8;
      }
      if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) {
        *(int *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = local_1ac[0x69];
        if (param_2 == 'K') {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2904) = 1;
        }
        else {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2904) = 0;
        }
        if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0) {
          *(int *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = local_1ac[0x69];
          if (param_2 == 'K') {
            *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2904) = 1;
          }
          else {
            *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2904) = 0;
          }
        }
      }
    }
    else {
      switch(DAT_005b2980) {
      case 2:
        if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) &&
           (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = 0xde,
           *(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0)) {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = 0xde;
        }
        break;
      default:
        if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) &&
           (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = 0xdd,
           *(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0)) {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = 0xdd;
        }
        break;
      case 4:
        if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) &&
           (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = 0xdf,
           *(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0)) {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = 0xdf;
        }
        break;
      case 5:
      case 7:
        if ((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) &&
           (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = 0xe0,
           *(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0)) {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = 0xe0;
          if (*(short *)(param_1 + 0x250) == 0x11f) {
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) == 0) {
              return;
            }
            *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = 0xe2;
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) == 0) {
              return;
            }
            *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = 0xe2;
          }
          if (*(short *)(param_1 + 0x250) == 0x11b) {
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) == 0) {
              return;
            }
            *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = 0xe3;
            if (*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) == 0) {
              return;
            }
            *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = 0xe3;
          }
          if (((*(short *)(param_1 + 0x250) == 0xaf) &&
              (*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0)) &&
             (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = 0xe4,
             *(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0)) {
            *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = 0xe4;
          }
        }
        break;
      case 9:
        if ((((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0) &&
             (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = 0xe1,
             *(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0)) &&
            (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = 0xe1,
            *(short *)(param_1 + 0x250) == 0xaf)) &&
           ((*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) != 0 &&
            (*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2f8) + 0x2900) = 0xe4,
            *(int *)(*(int *)(param_1 + 0x60) + 0x2fc) != 0)))) {
          *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x60) + 0x2fc) + 0x2900) = 0xe4;
        }
      }
    }
  }
  return;
}

