/***
 * This program causes an "In-address space security Exception" by clearing the
 * tag of the capability in the DDC
 * This is a mashup of the invalid DDC example and the seal example:
 * instead of making it invalid by clearing the tag,
 * we seal it
 ***/

#include "../include/common.h"
#include "include/utils.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/sysctl.h>

#define CHERI_TOCAP __cheri_tocap

#if !defined(__CHERI_CAPABILITY_WIDTH__) || defined(__CHERI_PURE_CAPABILITY__)
#error "This example only works on CHERI hybrid mode"
#endif

int main()
{
	// Prepare to seal the capability.
	void *theseal;
	void * __capability sealcap = (CHERI_TOCAP void * __capability)theseal;
	size_t sealcap_size = sizeof(sealcap);
	
	if (sysctlbyname("security.cheri.sealcap", &sealcap, &sealcap_size, NULL, 0) < 0)
	{
		error("Fatal error. Cannot get `security.cheri.sealcap`.");
		exit(1);
	}
	
	// Before sealing the capability, we ensure we have a valid one
	assert(cheri_tag_get(cheri_ddc_get()));
	printf("We've got a valid capability here:\n");
	pp_cap(cheri_ddc_get());
	
	void * __capability sealed = cheri_seal(cheri_ddc_get(), sealcap);
	
	printf("Now we've sealed it:\n");
	pp_cap(sealed);
	
	printf("Going to write a sealed capability to the ddc. Prepare to crash\n");
	
	// Trying to write a sealed capability will cause an exception
	write_ddc(sealed);
	return 0;
}