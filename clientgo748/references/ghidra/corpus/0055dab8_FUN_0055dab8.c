// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055dab8 | Name: FUN_0055dab8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

LRESULT __thiscall FUN_0055dab8(int *param_1,HWND param_2,uint param_3,uint param_4,uint param_5)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  LRESULT LVar4;
  undefined1 local_15c [4];
  undefined2 local_158;
  undefined2 local_156;
  undefined4 local_150;
  int local_14c;
  undefined2 *local_148;
  undefined2 *local_144;
  uint local_140;
  undefined4 local_13c;
  int local_138;
  HDC local_134;
  HGDIOBJ local_130;
  uint local_12c;
  uint local_128;
  uint local_124;
  HWND local_120;
  CHAR local_11c [4];
  char local_118 [252];
  ushort local_1c;
  undefined2 uStack_1a;
  ushort local_18;
  undefined2 uStack_16;
  undefined4 local_14;
  ushort local_10;
  undefined2 uStack_e;
  ushort local_c;
  undefined2 uStack_a;
  int local_8;
  
  if (param_3 < 0x110) {
    if (param_3 == 0x10f) {
      if (param_1[0x3c] != 0) {
        FUN_004af550(param_4 & 0xff,param_5);
      }
    }
    else if (param_3 < 0x52) {
      if (param_3 == 0x51) {
        _memset(local_11c,0,0x100);
        GetKeyboardLayoutNameA(local_11c);
        iVar2 = _strncmp(local_118,&DAT_005c4f5c,4);
        if (iVar2 == 0) {
          SendMessageA(param_2,0x10,0,0);
        }
      }
      else {
        switch(param_3) {
        case 1:
          (**(code **)(*param_1 + 0x10))(param_2,param_4,param_5);
          break;
        case 3:
        case 5:
          if (param_1[0x3a] != 0) {
            FUN_004317fc();
          }
          break;
        case 6:
          SendMessageA(param_2,0x281,0,-0x3ffffff1);
          if (param_4 == 0) {
            if (param_1[0x3c] != 0) {
              *(undefined4 *)(param_1[0x3c] + 0x434) = 0;
            }
            if (param_1[0x3b] != 0) {
              FUN_004294a0(0,0xffffd8f0);
            }
          }
          else {
            if (param_1[0x3c] != 0) {
              *(undefined4 *)(param_1[0x3c] + 0x434) = 0;
            }
            if (param_1[0x3b] != 0) {
              FUN_004294a0(0,*(undefined4 *)param_1[0x3b]);
            }
          }
          break;
        case 0x10:
          if ((DAT_0067cf38 != (int *)0x0) && (DAT_0067cf38[9] == 30000)) {
            if (DAT_013b7220 == 0) {
              DAT_013b7220 = (**(code **)(*(int *)param_1[0x3d] + 8))();
              local_156 = *(undefined2 *)(DAT_0067cf38[0x13] + 0x20);
              local_158 = 0x3ae;
              local_150 = 0;
              FUN_0042550e(local_15c,0x10);
              return 0;
            }
            uVar3 = (**(code **)(*(int *)param_1[0x3d] + 8))();
            if (uVar3 < DAT_013b7220 + 3000U) {
              return 0;
            }
          }
          (**(code **)(*param_1 + 8))();
          if (0 < DAT_013b71f0) {
            FUN_0058f75f(DAT_013b71f0);
            DAT_013b71f0 = 0;
          }
          _DAT_013b7228 = 1;
          FUN_00423c61();
          DestroyWindow(param_2);
          PostQuitMessage(0);
          DAT_013b7220 = 0;
          return 0;
        case 0x20:
          return 0;
        case 0x2b:
          local_12c = param_5;
          local_134 = CreateCompatibleDC(*(HDC *)(param_5 + 0x18));
          local_130 = (HGDIOBJ)0x0;
          local_130 = SelectObject(local_134,(HGDIOBJ)param_1[0x44]);
          StretchBlt(*(HDC *)(local_12c + 0x18),*(int *)(local_12c + 0x1c),
                     *(int *)(local_12c + 0x20),
                     *(int *)(local_12c + 0x24) - *(int *)(local_12c + 0x1c),
                     *(int *)(local_12c + 0x28) - *(int *)(local_12c + 0x20),local_134,0,0,0x56,0x15
                     ,0xcc0020);
          SelectObject(local_134,local_130);
          DeleteDC(local_134);
        }
      }
    }
    else {
      switch(param_3) {
      case 0x100:
        if ((param_1[0x3a] != 0) && (*(int *)(param_1[0x3a] + 0x10c) == 2)) {
          FUN_0043199b();
          if (param_1[0x3a] != 0) {
            if ((undefined4 *)param_1[0x3a] != (undefined4 *)0x0) {
              (*(code *)**(undefined4 **)param_1[0x3a])(1);
            }
            param_1[0x3a] = 0;
          }
          FUN_0055ca18();
        }
        if (param_1[0x3c] != 0) {
          uVar1 = GetKeyState(0x11);
          _local_10 = CONCAT22(uStack_e,uVar1);
          if ((int)(uint)uVar1 >> 8 == 0) {
            *(undefined4 *)(param_1[0x3c] + 0x438) = 0;
          }
          else {
            *(undefined4 *)(param_1[0x3c] + 0x438) = 1;
          }
          uVar1 = GetKeyState(0x10);
          _local_c = CONCAT22(uStack_a,uVar1);
          if ((int)(uint)uVar1 >> 8 == 0) {
            *(undefined4 *)(param_1[0x3c] + 0x440) = 0;
          }
          else {
            *(undefined4 *)(param_1[0x3c] + 0x440) = 1;
          }
          if (param_4 == 0x11) {
            local_14 = 0;
          }
          else if ((DAT_0067cf38 != (int *)0x0) && (*(int *)(param_1[0x3c] + 0x438) != 0)) {
            if ((param_4 == 0xbd) || (param_4 == 0x6d)) {
              (**(code **)(*DAT_0067cf38 + 0x5c))(0x9c70);
              break;
            }
            if ((param_4 == 0xbb) || (param_4 == 0x6b)) {
              (**(code **)(*DAT_0067cf38 + 0x5c))(0x9c6f);
              break;
            }
            if (param_4 == 0xdb) {
              (**(code **)(*DAT_0067cf38 + 0x5c))(0x9c5b);
              break;
            }
            if (param_4 == 0xdd) {
              (**(code **)(*DAT_0067cf38 + 0x5c))(0x9c5c);
              break;
            }
            if (param_4 == 0xde) {
              (**(code **)(*DAT_0067cf38 + 0x5c))(0x9c5d);
              break;
            }
          }
          FUN_004af484(param_4);
        }
        break;
      case 0x101:
        if (param_1[0x3c] != 0) {
          if (param_4 == 0x2c) {
            FUN_00427aa8();
          }
          uVar1 = GetKeyState(0x11);
          _local_1c = CONCAT22(uStack_1a,uVar1);
          if ((int)(uint)uVar1 >> 8 == 0) {
            *(undefined4 *)(param_1[0x3c] + 0x438) = 0;
          }
          else {
            *(undefined4 *)(param_1[0x3c] + 0x438) = 1;
          }
          uVar1 = GetKeyState(0x10);
          _local_18 = CONCAT22(uStack_16,uVar1);
          if ((int)(uint)uVar1 >> 8 == 0) {
            *(undefined4 *)(param_1[0x3c] + 0x440) = 0;
          }
          else {
            *(undefined4 *)(param_1[0x3c] + 0x440) = 1;
          }
          FUN_004af4cc(param_4);
        }
        break;
      case 0x102:
        if ((param_1[0x3c] != 0) && (*(int *)(param_1[0x3c] + 0x438) == 0)) {
          FUN_004af514(param_4 & 0xff,param_5);
        }
        break;
      case 0x104:
        if (((param_4 == 0x12) && (DAT_013b722c == 0)) && (param_1[0x3c] != 0)) {
          *(undefined4 *)(param_1[0x3c] + 0x434) = 1;
        }
        if ((param_4 == 0x79) && (param_1[0x3c] != 0)) {
          FUN_004af484(0x79);
        }
        break;
      case 0x105:
        if (((param_4 == 0x12) && (DAT_013b722c == 0)) && (param_1[0x3c] != 0)) {
          *(undefined4 *)(param_1[0x3c] + 0x434) = 0;
        }
        if ((param_4 == 0x79) && (param_1[0x3c] != 0)) {
          FUN_004af4cc(0x79);
        }
        break;
      case 0x10e:
        if (param_1[0x3c] != 0) {
          FUN_004af5f6(param_5,0);
        }
      }
    }
    goto switchD_0055db30_caseD_2;
  }
  if (param_3 < 0x206) {
    if (param_3 != 0x205) {
      switch(param_3) {
      case 0x111:
        if (param_4 == 999) {
          FUN_0055ee59(0);
        }
        else {
          local_124 = param_4 & 0xffff;
          local_128 = param_4 >> 0x10;
          local_120 = GetFocus();
          if (local_120 != *(HWND *)(DAT_013b71e0 + 4)) {
            if (local_124 == 0x9c96) {
              PostMessageA(local_120,0x301,0,0);
              return 1;
            }
            if (local_124 == 0x9ca4) {
              PostMessageA(local_120,0x302,0,0);
              return 1;
            }
            if (local_124 == 0x9ca6) {
              PostMessageA(local_120,0x300,0,0);
              return 1;
            }
          }
          if (DAT_0067cf38 != (int *)0x0) {
            (**(code **)(*DAT_0067cf38 + 0x5c))(local_124);
          }
        }
        break;
      case 0x112:
        if (param_4 == 0xf100) {
          SendMessageA(param_2,0x100,0xa4,0);
          return 0;
        }
        break;
      case 0x200:
      case 0x202:
        goto switchD_0055dbb7_caseD_200;
      case 0x201:
      case 0x203:
      case 0x204:
        goto switchD_0055dbb7_caseD_201;
      }
      goto switchD_0055db30_caseD_2;
    }
  }
  else {
    if (0x40d < param_3) {
      if (param_3 == 0x464) {
        if (param_1[0x3f] != 0) {
          if ((param_5 & 0xffff) == 1) {
            iVar2 = FUN_004245db();
            if (iVar2 == 0) {
              FUN_00423d7a();
            }
            else {
              local_138 = 0;
              local_13c = 0;
              while (local_148 = (undefined2 *)FUN_00424642(&local_138,&local_13c),
                    local_148 != (undefined2 *)0x0) {
                if (local_138 != 0) {
                  FUN_00431159(s_WSA_READ_Error___0x_x_005c4f64,local_13c);
                }
                local_144 = local_148;
                local_140 = (**(code **)(*(int *)param_1[0x3d] + 8))();
                DAT_013b7214 = *(uint *)(local_144 + 4);
                _DAT_013b7218 = local_140 / 1000;
                if (0 < DAT_013b71f0) {
                  local_14c = local_140 - _DAT_013b7200;
                  FUN_00590b78(DAT_013b71f0,&local_14c,4);
                  FUN_00590b78(DAT_013b71f0,local_144,*local_144);
                }
                if (DAT_013b721c + 10000 < local_140) {
                  if ((DAT_0067cf38 != (int *)0x0) && (DAT_0067cf38[0x802b] != 0)) {
                    if (local_140 + 500 < DAT_013b7214) {
                      FUN_00431159(s__CorrectionServerTime__OldTime___005c4f7c,local_140,
                                   DAT_013b7214 + 0x5a,DAT_013b7214 - local_140);
                      (**(code **)(*(int *)param_1[0x3d] + 4))(local_140 + 0x55);
                    }
                    else if (DAT_013b7214 < local_140 - 500) {
                      FUN_00431159(s__CorrectionServerTime__OldTime___005c4fc0,local_140,
                                   DAT_013b7214 + 0x5a,DAT_013b7214 - local_140);
                      (**(code **)(*(int *)param_1[0x3d] + 4))(local_140 - 0x55);
                    }
                  }
                  DAT_013b721c = local_140;
                }
                (**(code **)(*(int *)param_1[0x3e] + 8))(local_144[2],local_148);
              }
              local_148 = (undefined2 *)0x0;
            }
          }
          else {
            FUN_00423d7a();
            (**(code **)(*(int *)param_1[0x3e] + 8))(0,0);
          }
        }
      }
      else if ((param_3 == 0x465) && (param_1[0x3b] != 0)) {
        FUN_00428fc1();
      }
      goto switchD_0055db30_caseD_2;
    }
    if (param_3 == 0x40d) {
      if (param_1[0x3a] != 0) {
        local_8 = FUN_00431c30();
        if (local_8 == 1) {
          if (param_1[0x3a] != 0) {
            if ((undefined4 *)param_1[0x3a] != (undefined4 *)0x0) {
              (*(code *)**(undefined4 **)param_1[0x3a])(1);
            }
            param_1[0x3a] = 0;
          }
          FUN_0055ca18();
        }
        if (param_1[7] == 0) {
          FUN_004317fc();
        }
      }
      goto switchD_0055db30_caseD_2;
    }
    if (param_3 != 0x206) {
      if (param_3 == 0x281) {
        param_4 = (uint)(param_5 == 0xffffffff);
        LVar4 = DefWindowProcA(param_2,0x281,param_4,-0x7ffffff2);
        return LVar4;
      }
      if (param_3 == 0x282) {
        switch(param_4) {
        case 3:
        case 5:
          if (param_1[0x3c] != 0) {
            FUN_004af5eb();
            FUN_004af5f6(param_5,1);
          }
          break;
        case 4:
          if (param_1[0x3c] != 0) {
            FUN_004af5f6(param_5,0);
          }
          break;
        case 6:
          if (param_1[0x3e] != 0) {
            (**(code **)(*(int *)param_1[0x3e] + 0x1c))();
          }
        }
      }
      goto switchD_0055db30_caseD_2;
    }
switchD_0055dbb7_caseD_201:
    if (DAT_005ccec8 == 0) {
      SetCursor((HCURSOR)0x0);
    }
    else if ((DAT_005ccec8 == 2) && (DAT_005ccec0 != 0)) {
      if (*(int *)(DAT_005ccec0 + 0xa4) == 0) {
        SetCursor(DAT_005ccecc);
      }
      else if (*(int *)(DAT_005ccec0 + 0xa4) == 1) {
        SetCursor(DAT_005cced0);
      }
    }
    if ((param_1[0x3a] != 0) && (*(int *)(param_1[0x3a] + 0x10c) == 2)) {
      FUN_0043199b();
      if (param_1[0x3a] != 0) {
        if ((undefined4 *)param_1[0x3a] != (undefined4 *)0x0) {
          (*(code *)**(undefined4 **)param_1[0x3a])(1);
        }
        param_1[0x3a] = 0;
      }
      FUN_0055ca18();
    }
  }
switchD_0055dbb7_caseD_200:
  if (DAT_005ccec8 == 0) {
    SetCursor((HCURSOR)0x0);
  }
  else if ((DAT_005ccec8 == 2) && (DAT_005ccec0 != 0)) {
    if (*(int *)(DAT_005ccec0 + 0xa4) == 0) {
      SetCursor(DAT_005ccecc);
    }
    else if (*(int *)(DAT_005ccec0 + 0xa4) == 1) {
      SetCursor(DAT_005cced0);
    }
  }
  if (param_1[0x3c] != 0) {
    FUN_004afa19(param_3,param_4,param_5 & 0xffff,param_5 >> 0x10);
  }
switchD_0055db30_caseD_2:
  if ((param_1[0x3a] != 0) && (*(int *)(param_1[0x3a] + 0x11c) != 0)) {
    (**(code **)(**(int **)(param_1[0x3a] + 0x11c) + 0x98))
              (*(undefined4 *)(param_1[0x3a] + 0x11c),param_2,param_3,param_4,param_5);
  }
  LVar4 = DefWindowProcA(param_2,param_3,param_4,param_5);
  return LVar4;
}

