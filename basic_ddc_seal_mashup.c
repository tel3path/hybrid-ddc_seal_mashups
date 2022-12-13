/***
 * This program shows a very basic level of compartmentalisation. It first
 * `malloc` two blocks, and then flip the DDC to point from one to the other.
 *
 * It also incorporates the actions of seal.c in a hybrid context.
 * The original hybrid/basic_ddc.c warned that too many writes to the DDC
 * would cause a crash.
 * This mashup of the two examples deliberately causes a crash 
 * by sealing one of the capabilities before trying to write it to the DDC.
 *
 ***/

#include "../include/common.h"
#include "include/utils.h"
#include "mashup.h"
#include <assert.h>
#include <cheriintrin.h>
#include <machine/sysarch.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/sysctl.h>

#define CHERI_TOCAP __cheri_tocap

#if !defined(__CHERI_CAPABILITY_WIDTH__) || defined(__CHERI_PURE_CAPABILITY__)
#error "This example only works on CHERI hybrid mode"
#endif

int main()
{
	// Create the first pointer. 
	void *first_ptr = malloc(sizeof(uint16_t));
	uint16_t first_int = 200;
	first_ptr = &first_int;
	
	// Make sure the first pointer isn't at the same address as the DDC.
	assert(cheri_address_get(cheri_ddc_get()) != (uintptr_t)first_ptr);
	
	// Now we create the second pointer, twice as big
	void *second_ptr = (uint32_t *) malloc(sizeof(uint32_t));
	
	// Now we switch compartments by pointing the DDC at each
	// pointer in turn.
	switch_compartments((void *__capability)first_ptr, (void *__capability)second_ptr);
	
	// Prepare to seal the first capability.
	void *theseal;
	void * __capability sealcap = (CHERI_TOCAP void * __capability)theseal;
	size_t sealcap_size = sizeof(sealcap);
	
	if (sysctlbyname("security.cheri.sealcap", &sealcap, &sealcap_size, NULL, 0) < 0)
	{
		error("Fatal error. Cannot get `security.cheri.sealcap`.");
		exit(1);
	}

	// Let's look at the capability we just created.
	printf("\n---- sealcap ----\n");
	pp_cap(sealcap);
	
	// Now convert first_ptr into a capability explicitly.
	void * __capability firstcap = (CHERI_TOCAP void * __capability)first_ptr;
	
    // Now let's look at the first capability before we seal it.
	printf("\n---- firstcap (before sealing) ----\n");
	pp_cap(firstcap);
	
	// The capability is named sealed_first to imply that there will be a sealed_second.
	// But you won't get that far, because as soon as you try
	// to point the DDC at sealed_first, it will crash.
	void * __capability sealed_first = cheri_seal(firstcap, sealcap);
	
	// Now let's look at the newly-sealed first capability.
	printf("\n---- sealed_first ----\n");
	pp_cap(sealed_first);
	
	printf("\nSwitching compartments again: one of them is sealed, so expect a crash.\n");
	switch_compartments(sealed_first, (void *__capability)second_ptr);
	
	printf("If you got this far, the example is not working the way I expected.\n");
	// You didn't get this far, did you?
	free(first_ptr);
	free(second_ptr);
	
	return 0;
}

/***
 * Switches compartments by pointing the DDC at the second capability,
 * and then flipping it over to the first capability.
 *
 * param: acap, the first of two void capability pointers
 * param: bcap, the second of two void capability pointers
 ***/
void switch_compartments(void *__capability acap, void *__capability bcap) 
{	
	// WRITING THE SECOND CAPABILITY TO THE DDC 
	// This may or may not be switching compartments, depending on recent actions.
	printf("Writing the second capability to the DDC.\n");
	write_ddc(bcap);

	// Make sure the DDC's address is the one we expect, having just written to it
	assert(cheri_address_get(cheri_ddc_get()) != __builtin_cheri_address_get(acap));
	assert(cheri_address_get(cheri_ddc_get()) == __builtin_cheri_address_get(bcap));
	
	// WRITING THE FIRST CAPABILITY TO THE DDC: SWITCHING COMPARTMENTS
	// Note: this program is very simple and writing to the DDC in this fashion
	// would cause a crash if the program were to execute much further.
	printf("Switching compartments: writing first capability to the DDC.\n");
	write_ddc(acap);
	// Make sure the DDC's address is the one we expect, having just written to it
	assert(cheri_address_get(cheri_ddc_get()) != __builtin_cheri_address_get(bcap));
	assert(cheri_address_get(cheri_ddc_get()) == __builtin_cheri_address_get(acap));
}
