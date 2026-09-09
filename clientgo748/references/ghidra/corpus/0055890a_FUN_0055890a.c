// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055890a | Name: FUN_0055890a


uint FUN_0055890a(short *param_1)

{
  ushort uVar1;
  undefined1 local_108 [256];
  uint local_8;
  
  if (param_1 == (short *)0x0) {
    local_8 = 0;
  }
  else {
    local_8 = 0;
    uVar1 = param_1[2];
    if (uVar1 < 0x36c) {
      if (uVar1 == 0x36b) {
        local_8 = (uint)(*param_1 != 0x3c);
      }
      else if (uVar1 < 0x212) {
        if (uVar1 == 0x211) {
          local_8 = (uint)(*param_1 != 0x2c);
        }
        else if (uVar1 < 0x170) {
          if (uVar1 == 0x16f) {
            local_8 = (uint)(*param_1 != 0x10);
          }
          else {
            switch(uVar1) {
            case 0x101:
              local_8 = (uint)(*param_1 != 0x6c);
              break;
            case 0x102:
              local_8 = (uint)(*param_1 != 0x74);
              break;
            case 0x103:
              local_8 = (uint)(*param_1 != 0x14);
              break;
            case 0x104:
              local_8 = (uint)(*param_1 != 0x98);
              break;
            case 0x10e:
              local_8 = (uint)(*param_1 != 0x720);
              break;
            case 0x110:
              local_8 = (uint)(*param_1 != 0x2f4);
              break;
            case 0x112:
              local_8 = (uint)(*param_1 != 0x2f4);
              break;
            case 0x114:
              local_8 = (uint)(*param_1 != 0x4dc);
              break;
            case 0x116:
              local_8 = (uint)(*param_1 != 0xc);
              break;
            case 0x117:
              local_8 = (uint)(*param_1 != 0xc);
              break;
            case 0x119:
              local_8 = (uint)(*param_1 != 0xc);
              break;
            case 0x11a:
              local_8 = (uint)(*param_1 != 0xc);
              break;
            case 0x11b:
              local_8 = (uint)(*param_1 != 0xc);
              break;
            case 0x11c:
              local_8 = (uint)(*param_1 != 0xc);
              break;
            case 0x11d:
              local_8 = (uint)(*param_1 != 0xc);
              break;
            case 0x165:
              local_8 = (uint)(*param_1 != 0x10);
            }
          }
        }
        else {
          switch(uVar1) {
          case 0x171:
            local_8 = (uint)(*param_1 != 0x1c);
            break;
          case 0x175:
            local_8 = (uint)(*param_1 != 0x1c);
            break;
          case 0x17c:
            local_8 = (uint)(*param_1 != 0xec);
            break;
          case 0x181:
            local_8 = (uint)(*param_1 != 0x14);
            break;
          case 0x182:
            local_8 = (uint)(*param_1 != 0x18);
            break;
          case 0x185:
            local_8 = (uint)(*param_1 != 0x210);
            break;
          case 0x18a:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x18b:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x18d:
            local_8 = (uint)(*param_1 != 0xc);
            break;
          case 0x193:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x194:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x1bf:
            local_8 = (uint)(*param_1 != 0x24);
            break;
          case 0x1c1:
            local_8 = (uint)(*param_1 != 0x18);
            break;
          case 0x20a:
            local_8 = (uint)(*param_1 != 0xe0);
            break;
          case 0x20d:
            local_8 = (uint)(*param_1 != 0x74);
            break;
          case 0x20f:
            local_8 = (uint)(*param_1 != 0x24);
          }
        }
      }
      else if (uVar1 < 0x2be) {
        if (uVar1 == 0x2bd) {
          local_8 = (uint)(*param_1 != 0x18);
        }
        else {
          switch(uVar1) {
          case 0x213:
            local_8 = (uint)(*param_1 != 0x24);
            break;
          case 0x26e:
            local_8 = (uint)(*param_1 != 0x20);
            break;
          case 0x270:
            local_8 = (uint)(*param_1 != 0x1c);
            break;
          case 0x272:
            local_8 = (uint)(*param_1 != 0x20);
            break;
          case 0x277:
            local_8 = (uint)(*param_1 != 0x14);
            break;
          case 0x27b:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x289:
            local_8 = (uint)(*param_1 != 0xc);
            break;
          case 0x28b:
            local_8 = (uint)(*param_1 != 0x14);
            break;
          case 0x28c:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x28e:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x28f:
            local_8 = (uint)(*param_1 != 0x14);
            break;
          case 0x290:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x291:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x292:
            local_8 = (uint)(*param_1 != 0x10);
            break;
          case 0x295:
            local_8 = (uint)(*param_1 != 0x80);
            break;
          case 700:
            local_8 = (uint)(*param_1 != 0x6c);
          }
        }
      }
      else {
        switch(uVar1) {
        case 0x2be:
          local_8 = (uint)(*param_1 != 0x14);
          break;
        case 0x2c2:
          local_8 = (uint)(*param_1 != 0x18);
          break;
        case 0x2c3:
          local_8 = (uint)(*param_1 != 0x54);
          break;
        case 0x2c4:
          local_8 = (uint)(*param_1 != 0x54);
          break;
        case 0x2c7:
          local_8 = (uint)(*param_1 != 0x10);
          break;
        case 0x333:
          local_8 = (uint)(*param_1 != 0x6c);
          break;
        case 0x334:
          local_8 = (uint)(*param_1 != 0x80);
          break;
        case 0x336:
          local_8 = (uint)(*param_1 != 0x5c);
          break;
        case 0x337:
          local_8 = (uint)(*param_1 != 0x24);
          break;
        case 0x338:
          local_8 = (uint)(*param_1 != 0x18);
          break;
        case 0x364:
          local_8 = (uint)(*param_1 != 0xb0);
          break;
        case 0x366:
          local_8 = (uint)(*param_1 != 0x34);
          break;
        case 0x367:
          local_8 = (uint)(*param_1 != 0x34);
          break;
        case 0x369:
          local_8 = (uint)(*param_1 != 0x10);
          break;
        case 0x36a:
          local_8 = (uint)(*param_1 != 0x14);
        }
      }
    }
    else if (uVar1 < 0x41f) {
      if (uVar1 == 0x41e) {
        local_8 = (uint)(*param_1 != 0xc);
      }
      else {
        switch(uVar1) {
        case 0x36c:
          local_8 = (uint)(*param_1 != 0x60);
          break;
        case 0x373:
          local_8 = (uint)(*param_1 != 0x24);
          break;
        case 0x374:
          local_8 = (uint)(*param_1 != 0x14);
          break;
        case 0x376:
          local_8 = (uint)(*param_1 != 0x14);
          break;
        case 0x378:
          local_8 = (uint)(*param_1 != 0x20);
          break;
        case 0x379:
          local_8 = (uint)(*param_1 != 0x18);
          break;
        case 0x37a:
          local_8 = (uint)(*param_1 != 0x14);
          break;
        case 0x37d:
          local_8 = (uint)(*param_1 != 0x28);
          break;
        case 0x37e:
          local_8 = (uint)(*param_1 != 0x10);
          break;
        case 0x37f:
          local_8 = (uint)(*param_1 != 0x2c);
          break;
        case 0x380:
          local_8 = (uint)(*param_1 != 0x10);
          break;
        case 899:
          local_8 = (uint)(*param_1 != 0x9c);
          break;
        case 900:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x386:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x387:
          local_8 = (uint)(*param_1 != 0x10);
          break;
        case 0x388:
          local_8 = (uint)(*param_1 != 0x10);
          break;
        case 0x3c0:
          local_8 = (uint)(*param_1 != 0x54);
          break;
        case 0x3c9:
          local_8 = (uint)(*param_1 != 0x34);
          break;
        case 0x3ca:
          if (*param_1 != 0x24) {
            local_8 = 1;
          }
          break;
        case 0x401:
          local_8 = (uint)(*param_1 != 0x6c);
          break;
        case 0x402:
          local_8 = (uint)(*param_1 != 0x74);
          break;
        case 0x403:
          local_8 = (uint)(*param_1 != 0x98);
          break;
        case 0x404:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x40a:
          local_8 = (uint)(*param_1 != 0x20);
          break;
        case 0x40b:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x416:
          local_8 = (uint)(*param_1 != 0x720);
          break;
        case 0x417:
          local_8 = (uint)(*param_1 != 0x4dc);
          break;
        case 0x418:
          local_8 = (uint)(*param_1 != 0x2f4);
          break;
        case 0x419:
          local_8 = (uint)(*param_1 != 0x2f4);
          break;
        case 0x41a:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x41b:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x41c:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x41d:
          local_8 = (uint)(*param_1 != 0xc);
        }
      }
    }
    else if (uVar1 < 0x7bf) {
      if (uVar1 == 0x7be) {
        local_8 = (uint)(*param_1 != 0xb4);
      }
      else {
        switch(uVar1) {
        case 0x41f:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x420:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x421:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x422:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x423:
          local_8 = (uint)(*param_1 != 0x18);
          break;
        case 0x424:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x425:
          local_8 = (uint)(*param_1 != 0xc);
        }
      }
    }
    else if (uVar1 < 0xbc0) {
      if (uVar1 == 0xbbf) {
        local_8 = (uint)(*param_1 != 0xb4);
      }
      else {
        switch(uVar1) {
        case 0x801:
          local_8 = (uint)(*param_1 != 0xe0);
          break;
        case 0x802:
          local_8 = (uint)(*param_1 != 0x24);
          break;
        case 0x803:
          local_8 = (uint)(*param_1 != 0x74);
          break;
        case 0x804:
          local_8 = (uint)(*param_1 != 0x24);
          break;
        case 0x805:
          local_8 = (uint)(*param_1 != 0xc);
          break;
        case 0x806:
          local_8 = (uint)(*param_1 != 0x8e8);
          break;
        case 0x807:
          local_8 = (uint)(*param_1 != 0x8ec);
          break;
        case 0x809:
          local_8 = (uint)(*param_1 != 0x2c);
        }
      }
    }
    else if (uVar1 < 0x2003) {
      if (uVar1 == 0x2002) {
        local_8 = (uint)(*param_1 != 0x30);
      }
      else if (uVar1 < 0x1002) {
        if (uVar1 == 0x1001) {
          local_8 = (uint)(*param_1 != 0x30);
        }
        else if (uVar1 == 0xe0e) {
          local_8 = (uint)(*param_1 != 0x14);
        }
        else if (uVar1 == 0xfaa) {
          local_8 = (uint)(*param_1 != 0x34);
        }
        else if (uVar1 == 0xfbc) {
          local_8 = (uint)(*param_1 != 0xb4);
        }
      }
      else if (uVar1 == 0x1004) {
        local_8 = (uint)(*param_1 != 0xc);
      }
      else if (uVar1 == 0x1005) {
        local_8 = (uint)(*param_1 != 0x17f4);
      }
      else if (uVar1 == 0x13bd) {
        local_8 = (uint)(*param_1 != 0xb4);
      }
    }
    else if (uVar1 < 0x3009) {
      if (uVar1 == 0x3008) {
        local_8 = (uint)(*param_1 != 0x1c);
      }
      else if (uVar1 == 0x2003) {
        local_8 = (uint)(*param_1 != 0x2c);
      }
      else if (uVar1 == 0x2006) {
        local_8 = (uint)(*param_1 != 0x17f4);
      }
      else if (uVar1 == 0x3007) {
        local_8 = (uint)(*param_1 != 0x1c);
      }
    }
    else if (uVar1 == 0x300a) {
      local_8 = (uint)(*param_1 != 0x10);
    }
    else if (uVar1 == 0x3409) {
      local_8 = (uint)(*param_1 != 0x84);
    }
    else if (uVar1 == 0x3c0f) {
      local_8 = (uint)(*param_1 != 0x754);
    }
    if (local_8 != 0) {
      FUN_0058f078(local_108,s_sys__Type_check_missing_type__d_s_005c48d0,param_1[2],*param_1);
      FUN_00423b20(local_108,&DAT_005c48fc,0);
    }
  }
  return local_8;
}

