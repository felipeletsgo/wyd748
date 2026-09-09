// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059a054 | Name: FUN_0059a054


void FUN_0059a054(DWORD param_1)

{
  undefined4 *puVar1;
  DWORD *pDVar2;
  DWORD DVar3;
  size_t sVar4;
  HANDLE hFile;
  int iVar5;
  char acStackY_1e3 [7];
  undefined1 *puStackY_1dc;
  char *pcStackY_1d8;
  undefined4 uStackY_1d4;
  undefined1 *puStackY_1d0;
  undefined4 uStackY_1cc;
  undefined1 *puStackY_1c8;
  undefined *puStackY_1c4;
  LPCVOID lpBuffer;
  LPDWORD lpNumberOfBytesWritten;
  LPOVERLAPPED lpOverlapped;
  CHAR local_1a8 [260];
  undefined1 local_a4 [160];
  
  iVar5 = 0;
  pDVar2 = &DAT_005cc658;
  do {
    if (param_1 == *pDVar2) break;
    pDVar2 = pDVar2 + 2;
    iVar5 = iVar5 + 1;
  } while (pDVar2 < &DAT_005cc6e8);
  if (param_1 == (&DAT_005cc658)[iVar5 * 2]) {
    if ((DAT_013b8030 == 1) || ((DAT_013b8030 == 0 && (DAT_005c9c04 == 1)))) {
      lpNumberOfBytesWritten = &param_1;
      puVar1 = (undefined4 *)(iVar5 * 8 + 0x5cc65c);
      lpOverlapped = (LPOVERLAPPED)0x0;
      sVar4 = _strlen((char *)*puVar1);
      lpBuffer = (LPCVOID)*puVar1;
      puStackY_1c4 = (undefined *)0x59a19d;
      hFile = GetStdHandle(0xfffffff4);
      puStackY_1c4 = (undefined *)0x59a1a4;
      WriteFile(hFile,lpBuffer,sVar4,lpNumberOfBytesWritten,lpOverlapped);
    }
    else if (param_1 != 0xfc) {
      DVar3 = GetModuleFileNameA((HMODULE)0x0,local_1a8,0x104);
      if (DVar3 == 0) {
        FUN_0058ee20();
      }
      sVar4 = _strlen(local_1a8);
      if (0x3c < sVar4 + 1) {
        sVar4 = _strlen(local_1a8);
        puStackY_1c4 = (undefined *)0x59a11e;
        _strncpy(acStackY_1e3 + sVar4,"...",3);
      }
      FUN_0058ee20();
      puStackY_1c4 = (undefined *)0x59a13f;
      FUN_0058ee30();
      puStackY_1c8 = local_a4;
      puStackY_1c4 = &DAT_005aa61c;
      uStackY_1cc = 0x59a150;
      FUN_0058ee30();
      uStackY_1cc = *(undefined4 *)(iVar5 * 8 + 0x5cc65c);
      puStackY_1d0 = local_a4;
      uStackY_1d4 = 0x59a162;
      FUN_0058ee30();
      uStackY_1d4 = 0x12010;
      puStackY_1dc = local_a4;
      pcStackY_1d8 = "Microsoft Visual C++ Runtime Library";
      acStackY_1e3[3] = 'x';
      acStackY_1e3[4] = -0x5f;
      acStackY_1e3[5] = 'Y';
      acStackY_1e3[6] = '\0';
      FUN_0059c9fd();
    }
  }
  return;
}

