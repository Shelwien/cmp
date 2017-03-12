
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)

__declspec(naked)
qword rdtsc( void ) {
  __asm _emit 0x0F
  __asm _emit 0x31
  __asm _emit 0xC3
}

#else

qword rdtsc( void ) {
  qword r;
  __asm__( "rdtsc" : "=A"(r) : );
  return r;
}

#endif

qword lasttsc,curtsc;

