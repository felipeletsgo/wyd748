// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e7d5 | Name: ~exception


/* Library Function - Single Match
    public: virtual __thiscall exception::~exception(void)
   
   Libraries: Visual Studio 2003 Release, Visual Studio 2005 Release */

void __thiscall exception::~exception(exception *this)

{
  *(undefined ***)this = &PTR_FUN_005aa91c;
  if (*(int *)(this + 8) != 0) {
    FUN_0058ea3a(*(undefined4 *)(this + 4));
  }
  return;
}

