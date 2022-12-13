/***
 * This program causes an "In-address space security Exception" by trying to
 * write a null capability to the DDC.
 * This is a mashup of the null DDC example and the seal example.
 * First we show that it's possible to seal a null capability.
 * We then unseal the null capability.
 * Then we try writing the unsealed null capability to the DDC,
 * and it will cause a crash because it's null,
 * despite being unsealed.
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
	
	printf("About to try to seal a null capability\n");
	void * __capability sealed = cheri_seal(NULL, sealcap);
	
	// Did we really seal the null capability?
	assert(cheri_is_sealed(sealed));
	
	// Ensure the null capability is really sealed, even though it can't be valid
	printf("Did we really just seal a null capability?!?\n");
	pp_cap(sealed);
	
	printf("About to try to unseal a null capability\n");
	void * __capability unsealed = cheri_unseal(sealed, sealcap);
	
	// Did we really seal the null capability?
	assert(!cheri_is_sealed(unsealed));
	
	printf("Unsealed:\n");
	pp_cap(unsealed);
	
	printf("About to try to write an unsealed null capability to the DDC. Prepare to crash:\n");
	write_ddc(unsealed);
	
	return 0;
}
	
	