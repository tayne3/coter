#include "coter/container/vector.h"

bool _ct__vector_reserve(void** p_ptr, size_t* p_cap, size_t elem_size, size_t new_cap) {
	if (!p_ptr || !p_cap || elem_size == 0) { return false; }
	if (*p_cap >= new_cap) { return true; }
	if (new_cap > CT_VEC_MEMORY_MAX / elem_size) { return false; }

	size_t target_cap = new_cap;
	target_cap--;
	target_cap |= target_cap >> 1;
	target_cap |= target_cap >> 2;
	target_cap |= target_cap >> 4;
	target_cap |= target_cap >> 8;
	target_cap |= target_cap >> 16;
#if SIZE_MAX > 0xFFFFFFFF
	target_cap |= target_cap >> 32;  // 64 位系统
#endif
	target_cap++;
	if (target_cap > CT_VEC_MEMORY_MAX / elem_size) { return false; }

	void* new_ptr = realloc(*p_ptr, elem_size * target_cap);
	if (!new_ptr) { return false; }

	*p_ptr = new_ptr;
	*p_cap = target_cap;
	return true;
}

bool _ct__vector_resize(void** p_ptr, size_t* p_size, size_t* p_cap, size_t elem_size, size_t new_size) {
	if (!p_ptr || !p_size || !p_cap || elem_size == 0) { return false; }

	if (new_size > *p_cap) {
		if (!_ct__vector_reserve(p_ptr, p_cap, elem_size, new_size)) { return false; }
	}
	if (new_size > *p_size) {
		char* base = (char*)(*p_ptr);
		memset(base + (*p_size * elem_size), 0, (new_size - *p_size) * elem_size);
	}

	*p_size = new_size;
	return true;
}

bool _ct__vector_insert(void** p_ptr, size_t* p_size, size_t* p_cap, size_t elem_size, size_t idx, const void* data) {
	if (!p_ptr || !p_size || !p_cap || elem_size == 0) { return false; }
	if (idx > *p_size) { return false; }
	if (*p_size >= *p_cap) {
		if (!_ct__vector_reserve(p_ptr, p_cap, elem_size, *p_size + 1)) { return false; }
	}

	char* base = (char*)(*p_ptr);
	if (idx < *p_size) { memmove(base + ((idx + 1) * elem_size), base + (idx * elem_size), (*p_size - idx) * elem_size); }
	if (data) { memcpy(base + (idx * elem_size), data, elem_size); }

	++(*p_size);
	return true;
}

bool _ct__vector_erase(void* ptr, size_t* p_size, size_t elem_size, size_t idx) {
	if (!ptr || !p_size || elem_size == 0) { return false; }
	if (idx >= *p_size) { return false; }

	char* base = (char*)ptr;
	if (idx < *p_size - 1) { memmove(base + (idx * elem_size), base + ((idx + 1) * elem_size), (*p_size - idx - 1) * elem_size); }

	--(*p_size);
	return true;
}

bool _ct__vector_shrink(void** p_ptr, size_t size, size_t* p_cap, size_t elem_size) {
	if (!p_ptr || !p_cap || elem_size == 0) { return false; }
	if (*p_cap == size) { return true; }
	if (size == 0) {
		free(*p_ptr);
		*p_ptr = NULL;
		*p_cap = 0;
		return true;
	}

	void* new_ptr = realloc(*p_ptr, elem_size * size);
	if (!new_ptr) { return false; }

	*p_ptr = new_ptr;
	*p_cap = size;
	return true;
}
