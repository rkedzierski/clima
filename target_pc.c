#include "target_pc.h"
#include <stdlib.h>

void* clima_malloc(clima_size_t size)
{
	return malloc(size);
}