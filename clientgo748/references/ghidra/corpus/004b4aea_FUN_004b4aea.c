// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b4aea | Name: FUN_004b4aea


void FUN_004b4aea(undefined4 param_1,int param_2,undefined *param_3,undefined4 param_4,
                 undefined4 param_5)

{
  DWORD DVar1;
  undefined4 uVar2;
  size_t sVar3;
  int iVar4;
  char *_Source;
  uint uVar5;
  undefined4 uVar6;
  undefined *puVar7;
  undefined1 local_d08 [16];
  undefined1 local_cf8 [16];
  undefined1 local_ce8 [16];
  undefined1 local_cd8 [16];
  undefined1 local_cc8 [16];
  undefined1 local_cb8 [16];
  undefined1 local_ca8 [20];
  undefined1 local_c94 [16];
  undefined1 local_c84 [20];
  undefined1 local_c70 [16];
  undefined1 local_c60 [20];
  undefined1 local_c4c [16];
  undefined1 local_c3c [20];
  undefined1 local_c28 [16];
  undefined1 local_c18 [20];
  undefined1 local_c04 [4];
  undefined1 local_c00 [80];
  LPSTR local_bb0;
  char *local_bac;
  undefined4 local_ba8;
  undefined4 local_ba4 [2];
  undefined4 local_b9c;
  undefined4 local_b98;
  int local_b84;
  undefined1 local_b80 [288];
  undefined1 local_a60 [260];
  HANDLE local_95c;
  undefined4 local_958;
  undefined4 local_954;
  undefined1 local_950 [1024];
  undefined4 local_550;
  int local_54c;
  basic_string<char,std::char_traits<char>,std::allocator<char>_> local_548 [16];
  int local_538;
  int local_534;
  undefined4 local_52c;
  int local_528;
  int local_524;
  undefined4 local_518;
  undefined4 local_514;
  undefined4 local_50c;
  uint local_430;
  int local_42c;
  char local_428 [1024];
  undefined4 local_28 [2];
  undefined4 local_20;
  undefined4 local_1c;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a057a;
  local_10 = ExceptionList;
  local_550 = 0x14c;
  ExceptionList = &local_10;
  local_95c = GetCurrentProcess();
  FUN_004b6270(local_c04);
  local_8 = 0;
  if (param_3 == (undefined *)0x0) {
    param_3 = &DAT_005c9998;
  }
  _memset(&local_538,0,0x108);
  if ((DAT_0092e5cc == 0) && (DAT_005b9c68 == 1)) {
    FUN_004b3f12();
  }
  if (DAT_0092e5cc == 0) {
    DAT_005b9c68 = 0;
    FUN_0058f6a9(param_3,s__lu__ErrorLog_not_initialized__o_005ba130,DAT_0092e5d4);
    local_8 = 0xffffffff;
    FUN_004b6310();
    ExceptionList = local_10;
    return;
  }
  EnterCriticalSection((LPCRITICAL_SECTION)&DAT_0092e5d8);
  InterlockedIncrement((LONG *)&DAT_0092e5d4);
  if ((DAT_0092e62c == (undefined4 *)0x0) &&
     (DAT_0092e62c = _malloc(0x420), DAT_0092e62c == (undefined4 *)0x0)) goto LAB_004b5adb;
  if (DAT_0092e588 != 2) {
    FUN_0058f6a9(param_3,s__lu__005ba170,DAT_0092e5d4);
    FUN_004b5b5b(param_3,0);
    FUN_0058f6a9(param_3,&DAT_005ba178);
  }
  if (DAT_005b9c68 != 0) {
    local_bb0 = _malloc(0x1fa0);
    if (local_bb0 == (LPSTR)0x0) goto LAB_004b5adb;
    std::basic_string<char,std::char_traits<char>,std::allocator<char>_>::operator=
              (local_548,&DAT_0092e63c);
    DVar1 = GetCurrentDirectoryA(0x1fa0,local_bb0);
    if (DVar1 != 0) {
      uVar2 = FUN_004b62e0(&DAT_005ba17c,local_c18);
      local_8._0_1_ = 1;
      uVar2 = FUN_004b7430(local_c28,local_bb0,uVar2);
      local_8._0_1_ = 2;
      FUN_004b6370(uVar2);
      local_8._0_1_ = 1;
      FUN_004b6310();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_004b6310();
    }
    DVar1 = GetModuleFileNameA((HMODULE)0x0,local_bb0,0x1fa0);
    if (DVar1 != 0) {
      sVar3 = _strlen(local_bb0);
      for (local_bac = local_bb0 + (sVar3 - 1);
          (((local_bb0 <= local_bac && (*local_bac != '\\')) && (*local_bac != '/')) &&
          (*local_bac != ':')); local_bac = local_bac + -1) {
      }
      if (local_bac != local_bb0) {
        if (*local_bac == ':') {
          local_bac = local_bac + 1;
        }
        *local_bac = '\0';
        uVar2 = FUN_004b62e0(&DAT_005ba180,local_c3c);
        local_8._0_1_ = 3;
        uVar2 = FUN_004b7430(local_c4c,local_bb0,uVar2);
        local_8._0_1_ = 4;
        FUN_004b6370(uVar2);
        local_8._0_1_ = 3;
        FUN_004b6310();
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_004b6310();
      }
    }
    DVar1 = GetEnvironmentVariableA(s__NT_SYMBOL_PATH_005ba184,local_bb0,0x1fa0);
    if (DVar1 != 0) {
      uVar2 = FUN_004b62e0(&DAT_005ba194,local_c60);
      local_8._0_1_ = 5;
      uVar2 = FUN_004b7430(local_c70,local_bb0,uVar2);
      local_8._0_1_ = 6;
      FUN_004b6370(uVar2);
      local_8._0_1_ = 5;
      FUN_004b6310();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_004b6310();
    }
    DVar1 = GetEnvironmentVariableA(s__NT_ALTERNATE_SYMBOL_PATH_005ba198,local_bb0,0x1fa0);
    if (DVar1 != 0) {
      uVar2 = FUN_004b62e0(&DAT_005ba1b4,local_c84);
      local_8._0_1_ = 7;
      uVar2 = FUN_004b7430(local_c94,local_bb0,uVar2);
      local_8._0_1_ = 8;
      FUN_004b6370(uVar2);
      local_8._0_1_ = 7;
      FUN_004b6310();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_004b6310();
    }
    DVar1 = GetEnvironmentVariableA(s_SYSTEMROOT_005ba1b8,local_bb0,0x1fa0);
    if (DVar1 != 0) {
      uVar2 = FUN_004b62e0(&DAT_005ba1c4,local_ca8);
      local_8._0_1_ = 9;
      uVar2 = FUN_004b7430(local_cb8,local_bb0,uVar2);
      local_8._0_1_ = 10;
      FUN_004b6370(uVar2);
      local_8._0_1_ = 9;
      FUN_004b6310();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_004b6310();
    }
    iVar4 = FUN_004b6430();
    if (iVar4 != 0) {
      iVar4 = FUN_004b6430();
      uVar2 = FUN_004b6450(local_cc8,0,iVar4 + -1);
      local_8._0_1_ = 0xb;
      FUN_004b6330(uVar2);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_004b6310();
    }
    sVar3 = 0x1fa0;
    _Source = (char *)FUN_004b6400();
    _strncpy(local_bb0,_Source,sVar3);
    local_bb0[0x1f9f] = '\0';
    iVar4 = (*DAT_0092e614)(local_95c,local_bb0,0);
    if (iVar4 == 0) {
      if (DAT_0092e588 != 2) {
        DVar1 = GetLastError();
        FUN_0058f6a9(param_3,s__lu__SymInitialize____GetLastErr_005ba1c8,DAT_0092e5d4,DVar1);
      }
      if (local_bb0 != (LPSTR)0x0) {
        FUN_0058ffd8(local_bb0);
      }
      goto LAB_004b5adb;
    }
    uVar5 = (*DAT_0092e60c)();
    local_430 = uVar5 & 0xfffffff9 | 0x10;
    (*DAT_0092e61c)(local_430);
    puVar7 = param_3;
    DVar1 = GetCurrentProcessId();
    FUN_004b5bbd(local_95c,DVar1,puVar7);
    if (local_bb0 != (LPSTR)0x0) {
      FUN_0058ffd8(local_bb0);
    }
  }
  DAT_005b9c68 = 0;
  local_538 = *(int *)(param_2 + 0xb8);
  local_534 = 0;
  local_52c = 3;
  local_518 = *(undefined4 *)(param_2 + 0xb4);
  local_514 = 0;
  local_50c = 3;
  _memset(DAT_0092e62c,0,0x420);
  *DAT_0092e62c = 0x20;
  DAT_0092e62c[6] = 0x400;
  _memset(local_28,0,0x18);
  local_28[0] = 0x18;
  _memset(local_ba4,0,0x248);
  local_ba4[0] = 0x248;
  local_42c = 0;
  while (iVar4 = (*DAT_0092e620)(local_550,param_5,param_1,&local_538,0,param_4,DAT_0092e5fc,
                                 DAT_0092e604,0), iVar4 != 0) {
    local_54c = 0;
    if (DAT_0092e588 == 1) {
      FUN_0058f6a9(param_3,s__lu___3d_005ba1f4,DAT_0092e5d4,local_42c);
    }
    if (local_538 == 0 && local_534 == 0) {
      if (DAT_0092e588 == 0) {
        FUN_0058f6a9(param_3,s__lu____nosymbols__PC____0__005ba200,DAT_0092e5d4);
      }
      else if (DAT_0092e588 == 1) {
        FUN_0058f6a9(param_3,s___nosymbols__PC____0__005ba21c);
      }
      else if (DAT_0092e588 == 2) {
        FUN_0058f6a9(param_3,s_<STACKENTRY_decl__(_nosymbols__P_005ba238);
      }
    }
    else {
      local_428[0] = '\0';
      local_950[0] = 0;
      local_958 = 0;
      local_954 = 0;
      iVar4 = (*DAT_0092e610)(local_95c,local_538,local_534,&local_958,DAT_0092e62c);
      if (iVar4 == 0) {
        if (DAT_0092e588 == 1) {
          DVar1 = GetLastError();
          if (DVar1 == 0x1e7) {
            FUN_0058f6a9(param_3,&DAT_005ba290);
          }
          else {
            DVar1 = GetLastError();
            FUN_0058f6a9(param_3,s_SymGetSymFromAddr____GetLastErro_005ba264,DVar1);
          }
        }
      }
      else {
        (*DAT_0092e624)(DAT_0092e62c + 7,local_428,0x400,0x1000);
        (*DAT_0092e624)(DAT_0092e62c + 7,local_950,0x400,0);
        if (DAT_0092e588 == 1) {
          sVar3 = _strlen(local_428);
          if (sVar3 == 0) {
            FUN_0058f6a9(param_3,s_Sig___s___ld_bytes_005ba2a8,DAT_0092e62c + 7,local_958);
            FUN_0058ee20(local_428,DAT_0092e62c + 7);
          }
          else {
            FUN_0058f6a9(param_3,s__s___ld_bytes_005ba294,local_428,local_958);
          }
          FUN_0058f6a9(param_3,s__lu__Decl___s_005ba2c4,DAT_0092e5d4,local_950);
        }
      }
      local_ba8 = 0;
      if (DAT_0092e600 != (code *)0x0) {
        iVar4 = (*DAT_0092e600)(local_95c,local_538,local_534,&local_ba8,local_28);
        if (iVar4 == 0) {
          DVar1 = GetLastError();
          if ((DVar1 != 0x1e7) && (0 < local_42c)) {
            if (DAT_0092e588 == 2) {
              FUN_0058f6a9(param_3,s_<STACKENTRY_005ba2d8);
              local_54c = 1;
              FUN_004b4455(local_cd8,local_428,local_958);
              local_8._0_1_ = 0xc;
              uVar2 = FUN_004b6400();
              FUN_0058f6a9(param_3,s_decl___s__decl_offset____ld__005ba2e8,uVar2);
              local_8 = (uint)local_8._1_3_ << 8;
              FUN_004b6310();
              DVar1 = GetLastError();
              FUN_0058f6a9(param_3,s_srcfile__SymGetLineFromAddr____G_005ba308,DVar1);
            }
            else {
              DVar1 = GetLastError();
              FUN_0058f6a9(param_3,s__lu__SymGetLineFromAddr____GetLa_005ba33c,DAT_0092e5d4,DVar1);
            }
          }
        }
        else if (DAT_0092e588 == 0) {
          FUN_0058f6a9(param_3,s__lu___s__lu____ld_bytes___s__005ba390,DAT_0092e5d4,local_1c,
                       local_20,local_ba8,local_428);
        }
        else if (DAT_0092e588 == 1) {
          FUN_0058f6a9(param_3,s__lu__Line___s__lu____ld_bytes_005ba36c,DAT_0092e5d4,local_1c,
                       local_20,local_ba8);
        }
        else if (DAT_0092e588 == 2) {
          FUN_0058f6a9(param_3,s_<STACKENTRY_005ba3b0);
          local_54c = 1;
          FUN_004b4455(local_ce8,local_428,local_958);
          local_8._0_1_ = 0xd;
          uVar2 = FUN_004b6400();
          FUN_0058f6a9(param_3,s_decl___s__decl_offset____ld__005ba3c0,uVar2);
          local_8._0_1_ = 0;
          FUN_004b6310();
          FUN_004b4455(local_cf8,local_1c,local_20,local_ba8,local_428);
          local_8._0_1_ = 0xe;
          uVar2 = FUN_004b6400();
          FUN_0058f6a9(param_3,s_srcfile___s__line___lu__line_off_005ba3e0,uVar2);
          local_8 = (uint)local_8._1_3_ << 8;
          FUN_004b6310();
        }
      }
      if ((DAT_0092e588 == 1) || (DAT_0092e588 == 2)) {
        iVar4 = (*DAT_0092e608)(local_95c,local_538,local_534,local_ba4);
        if (iVar4 == 0) {
          if (DAT_0092e588 == 1) {
            DVar1 = GetLastError();
            FUN_0058f6a9(param_3,s__lu__SymGetModuleInfo___GetLastE_005ba40c,DAT_0092e5d4,DVar1);
          }
        }
        else {
          switch(local_b84) {
          case 0:
            FUN_0058ee20(local_c00,s__nosymbols__005ba438);
            break;
          case 1:
            FUN_0058ee20(local_c00,&DAT_005ba444);
            break;
          case 2:
            FUN_0058ee20(local_c00,&DAT_005ba44c);
            break;
          case 3:
            FUN_0058ee20(local_c00,&DAT_005ba450);
            break;
          case 4:
            FUN_0058ee20(local_c00,s__exported__005ba454);
            break;
          case 5:
            FUN_0058ee20(local_c00,s__deferred__005ba460);
            break;
          case 6:
            FUN_0058ee20(local_c00,&DAT_005ba46c);
            break;
          case 7:
            FUN_0058ee20(local_c00,&DAT_005ba470);
            break;
          default:
            FUN_00591dbd(local_c00,0x50,s_symtype__ld_005ba474,local_b84);
          }
          if ((DAT_0092e588 == 2) && (local_54c == 0)) {
            FUN_0058f6a9(param_3,s_<STACKENTRY_005ba480);
            local_54c = 1;
            uVar2 = local_958;
            uVar6 = FUN_004b4455(local_d08,local_428,local_958);
            local_8._0_1_ = 0xf;
            uVar2 = FUN_004b6400(uVar2,uVar6);
            FUN_0058f6a9(param_3,s_decl___s__decl_offset____ld__005ba490,uVar2);
            local_8 = (uint)local_8._1_3_ << 8;
            FUN_004b6310();
            FUN_0058f6a9(param_3,s_srcfile____005ba4b0);
            local_54c = 1;
          }
          if (DAT_0092e588 == 1) {
            FUN_0058f6a9(param_3,s__lu__Mod___s__base___08lxh_005ba4bc,DAT_0092e5d4,local_b80,
                         local_b9c,local_b98);
            if (local_b84 == 0) {
              FUN_0058f6a9(param_3,s__lu__Offset__0x_8_8x_005ba4e0,DAT_0092e5d4,local_538,local_534)
              ;
              FUN_0058f6a9(param_3,s__lu__Sym__type___s__file___s_005ba4fc,DAT_0092e5d4,local_c00,
                           local_a60);
            }
          }
          else if (local_54c == 1) {
            FUN_0058f6a9(param_3,s_module___s__base___08lx__005ba520,local_b80,local_b9c,local_b98);
          }
        }
      }
      if ((DAT_0092e588 == 2) && (local_54c == 1)) {
        FUN_0058f6a9(param_3,&DAT_005ba53c);
      }
    }
    if (local_528 == 0 && local_524 == 0) {
      SetLastError(0);
      break;
    }
    local_42c = local_42c + 1;
  }
  if ((DAT_0092e588 != 2) && (DVar1 = GetLastError(), DVar1 != 0)) {
    DVar1 = GetLastError();
    FUN_0058f6a9(param_3,s__lu__GetLastError____lu_005ba540,DAT_0092e5d4,DVar1);
  }
LAB_004b5adb:
  if ((param_3 != (undefined *)0x0) &&
     (FUN_0058f6a9(param_3,&DAT_005ba55c), DAT_0092e5d4 % 1000 != 0)) {
    FUN_00591c57(param_3);
  }
  LeaveCriticalSection((LPCRITICAL_SECTION)&DAT_0092e5d8);
  local_8 = 0xffffffff;
  FUN_004b6310();
  ExceptionList = local_10;
  return;
}

