// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00557aff | Name: FUN_00557aff


int FUN_00557aff(int param_1,int param_2,int param_3,int param_4,int param_5)

{
  int iVar1;
  longlong lVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int local_18;
  int local_c;
  
  iVar5 = *(int *)(&DAT_0092ff0c + param_1 * 0x60);
  local_c = (int)*(short *)(param_2 + 0x40);
  if (local_c < 0) {
    local_c = 0;
  }
  if (399 < local_c) {
    local_c = 400;
  }
  uVar6 = (uint)*(byte *)(param_2 + ((int)(param_1 % 0x18 + (param_1 % 0x18 >> 0x1f & 7U)) >> 3) +
                         0x59);
  iVar1 = *(int *)(&DAT_0092ff10 + param_1 * 0x60);
  iVar4 = *(int *)(&DAT_0092ff20 + param_1 * 0x60);
  iVar3 = param_1 + (param_1 >> 0x1f & 7U);
  lVar2 = CONCAT44(iVar3 >> 0x1f,iVar3 >> 3) % 3;
  iVar3 = (int)lVar2;
  local_18 = 0;
  if (iVar5 == 0) {
    if (param_1 == 0xb) {
      local_18 = iVar4 + uVar6 / 10;
    }
    else if (param_1 == 0xd) {
      local_18 = iVar4 + uVar6 * 3;
    }
    else if (param_1 == 0x29) {
      local_18 = uVar6 / 0x19 + 2;
    }
    else if (param_1 == 0x2b) {
      local_18 = iVar4 + 0xf + uVar6 / 3;
    }
    else if (param_1 == 0x2c) {
      local_18 = (uVar6 / 3 + 0xf) * 5;
    }
    else if (param_1 == 0x2d) {
      local_18 = iVar4 + uVar6 / 10;
    }
  }
  else if ((iVar5 < 1) || (5 < iVar5)) {
    if (iVar5 == 6) {
      local_18 = iVar1 + (uVar6 * 3) / 2;
      if ((param_1 == 0x1d) && ((*(uint *)(param_2 + 0x2dc) & 0x80) != 0)) {
        local_18 = (local_18 * 0x78) / 100;
      }
    }
    else {
      local_18 = iVar1;
      if (iVar5 != 0xb) {
        local_18 = (uint)*(byte *)(param_2 + 0x2ed) << 1;
      }
    }
  }
  else {
    iVar4 = (int)(param_1 + (param_1 >> 0x1f & 7U)) >> 3;
    if (param_5 % 10 < 6) {
      if (param_1 == 0x61) {
        local_18 = iVar1 + local_c * 0xf;
      }
      else if ((*(char *)(param_2 + 0x14) == '\0') && (iVar4 == 1)) {
        local_18 = iVar1 + uVar6 + local_c / 2 + (int)*(short *)(param_2 + 0x50) / 2 + param_4 * 3;
      }
      else if ((*(char *)(param_2 + 0x14) == '\0') && (iVar4 != 1)) {
        local_18 = ((int)((int)*(short *)(param_2 + 0x52) +
                         ((int)*(short *)(param_2 + 0x52) >> 0x1f & 3U)) >> 2) +
                   local_c / 2 + param_4 + iVar1 + uVar6;
      }
      else if (*(char *)(param_2 + 0x14) == '\x01') {
        local_18 = iVar1 + uVar6 + local_c / 2 + (int)*(short *)(param_2 + 0x52) / 3;
      }
      else if (*(char *)(param_2 + 0x14) == '\x02') {
        local_18 = iVar1 + uVar6 + local_c / 2 + (int)*(short *)(param_2 + 0x52) / 3;
      }
      else if (*(char *)(param_2 + 0x14) == '\x03') {
        if (param_1 == 0x5f) {
          local_18 = (int)*(short *)(param_2 + 0x44);
        }
        else {
          local_18 = iVar1 + uVar6 + local_c / 2 + (int)*(short *)(param_2 + 0x50) / 2 + param_4 * 3
          ;
        }
      }
    }
    else if (param_1 == 0x61) {
      local_18 = iVar1 + local_c * 0xf;
    }
    else if ((*(char *)(param_2 + 0x14) == '\0') && (iVar4 == 1)) {
      local_18 = iVar1 + uVar6 + local_c + (int)*(short *)(param_2 + 0x50) / 2 + param_4 * 3;
    }
    else if ((*(char *)(param_2 + 0x14) == '\0') && (iVar4 != 1)) {
      local_18 = ((int)((int)*(short *)(param_2 + 0x52) +
                       ((int)*(short *)(param_2 + 0x52) >> 0x1f & 3U)) >> 2) +
                 param_4 + iVar1 + uVar6 + local_c;
    }
    else if (*(char *)(param_2 + 0x14) == '\x01') {
      local_18 = iVar1 + uVar6 * 2 + local_c + (int)*(short *)(param_2 + 0x52) / 3;
    }
    else if (*(char *)(param_2 + 0x14) == '\x02') {
      local_18 = iVar1 + uVar6 * 2 + local_c + (int)*(short *)(param_2 + 0x52) / 3;
    }
    else if (*(char *)(param_2 + 0x14) == '\x03') {
      if (param_1 == 0x5f) {
        local_18 = (int)*(short *)(param_2 + 0x44);
      }
      else {
        local_18 = iVar1 + uVar6 + local_c / 2 + (int)*(short *)(param_2 + 0x50) / 2 + param_4 * 3;
      }
    }
    if (param_3 == 1) {
      if (iVar5 == 2) {
        local_18 = (local_18 * 0x5a) / 100;
      }
      if (iVar5 == 5) {
        local_18 = (local_18 * 0x82) / 100;
      }
    }
    else if ((param_3 == 2) && (iVar5 == 3)) {
      local_18 = (local_18 * 0x78) / 100;
    }
    if (param_1 != 0x61) {
      if (param_1 == 0x5f) {
        return local_18;
      }
      if (((*(char *)(param_2 + 0x14) == '\0') && (iVar4 == 1)) ||
         (*(char *)(param_2 + 0x14) == '\x03')) {
        local_18 = (int)(local_18 * 5 + (local_18 * 5 >> 0x1f & 3U)) >> 2;
      }
      else {
        iVar5 = ((int)(local_18 * ((uint)*(byte *)(param_2 + 0x2ed) * 4 + 100)) / 100) * 5;
        local_18 = (int)(iVar5 + (iVar5 >> 0x1f & 3U)) >> 2;
      }
    }
    if ((*(uint *)(param_2 + 0x2dc) & 1 << ((char)lVar2 * '\b' + 7U & 0x1f)) != 0) {
      switch(*(undefined1 *)(param_2 + 0x14)) {
      case 0:
        if (iVar3 == 0) {
          local_18 = (local_18 * 0x73) / 100;
        }
        else if (iVar3 == 1) {
          local_18 = (local_18 * 0x78) / 100;
        }
        else if (iVar3 == 2) {
          local_18 = (local_18 * 0x73) / 100;
        }
        break;
      case 1:
        if (iVar3 == 0) {
          local_18 = (local_18 * 0x6e) / 100;
        }
        else if (iVar3 == 1) {
          local_18 = (local_18 * 0x73) / 100;
        }
        else if (iVar3 == 2) {
          local_18 = (local_18 * 0x73) / 100;
        }
        break;
      case 2:
        if (iVar3 == 0) {
          local_18 = (local_18 * 0x6e) / 100;
        }
        break;
      case 3:
        if (iVar3 == 0) {
          local_18 = (local_18 * 0x6e) / 100;
        }
        else if (iVar3 == 1) {
          local_18 = (local_18 * 0x6e) / 100;
        }
        else if (iVar3 == 2) {
          local_18 = (local_18 * 0x78) / 100;
        }
      }
    }
  }
  return local_18;
}

