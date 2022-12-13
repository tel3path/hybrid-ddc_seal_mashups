#if !defined(__aarch64__)
#error "These mashup functions are Morello only"
#endif

void switch_compartments(void *__capability acap, void *__capability bcap);
