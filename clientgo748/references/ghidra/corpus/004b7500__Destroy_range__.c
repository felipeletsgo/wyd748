// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7500 | Name: _Destroy_range<>


/* Library Function - Multiple Matches With Same Base Name
    void __cdecl std::_Destroy_range<struct std::_Wrap_alloc<class std::allocator<class
   std::shared_ptr<struct Concurrency::details::_Task_impl<unsigned char> > > > >(class
   std::shared_ptr<struct Concurrency::details::_Task_impl<unsigned char> > *,class
   std::shared_ptr<struct Concurrency::details::_Task_impl<unsigned char> > *,struct
   std::_Wrap_alloc<class std::allocator<class std::shared_ptr<struct
   Concurrency::details::_Task_impl<unsigned char> > > > &,struct std::_Nonscalar_ptr_iterator_tag)
    void __cdecl std::_Destroy_range<struct std::_Wrap_alloc<class std::allocator<class
   std::shared_ptr<struct Concurrency::details::_Task_impl<struct std::pair<unsigned char,class
   Concurrency::details::_CancellationTokenState *> > > > > >(class std::shared_ptr<struct
   Concurrency::details::_Task_impl<struct std::pair<unsigned char,class
   Concurrency::details::_CancellationTokenState *> > > *,class std::shared_ptr<struct
   Concurrency::details::_Task_impl<struct std::pair<unsigned char,class
   Concurrency::details::_CancellationTokenState *> > > *,struct std::_Wrap_alloc<class
   std::allocator<class std::shared_ptr<struct Concurrency::details::_Task_impl<struct
   std::pair<unsigned char,class Concurrency::details::_CancellationTokenState *> > > > > &,struct
   std::_Nonscalar_ptr_iterator_tag)
    void __cdecl std::_Destroy_range<struct std::_Wrap_alloc<class std::allocator<class
   Concurrency::details::_ContextCallback> > >(class Concurrency::details::_ContextCallback *,class
   Concurrency::details::_ContextCallback *,struct std::_Wrap_alloc<class std::allocator<class
   Concurrency::details::_ContextCallback> > &,struct std::_Nonscalar_ptr_iterator_tag)
   
   Libraries: Visual Studio 2012 Debug, Visual Studio 2012 Release */

void _Destroy_range<>(int param_1,int param_2,undefined4 param_3)

{
  for (; param_1 != param_2; param_1 = param_1 + 0x28) {
    FUN_004b7640(param_3);
  }
  return;
}

