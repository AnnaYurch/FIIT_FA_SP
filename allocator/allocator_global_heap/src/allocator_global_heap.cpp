#include "../include/allocator_global_heap.h"

#include <not_implemented.h>

#include <sstream>
//n - возвращаемое значение не должно быть проигнорено
[[nodiscard]] void *allocator_global_heap::do_allocate_sm(size_t size) {
	void *ptr;
	debug_with_guard("Allocation of size started (size=" + std::to_string(size) + ")");
	
	try {
		ptr = ::operator new(size);
	} catch (std::bad_alloc &e) {
		error_with_guard("Failed to allocate memory of size ");
		throw;
	}

	debug_with_guard("Successfully allocated memory (size=" + std::to_string(size) + ")");
	return ptr;
}

void allocator_global_heap::do_deallocate_sm(void *at) {
	if (!at) {
		error_with_guard("Nothing to deallocate");
		return;
	}

	debug_with_guard("Deallocation of memory started");
	::operator delete(at);
	debug_with_guard("Successfully deallocated memory finished");
}

inline logger *allocator_global_heap::get_logger() const {
	return _logger;
}

inline std::string allocator_global_heap::get_typename() const {
	return "allocator_global_heap";
}

allocator_global_heap::allocator_global_heap(logger *logger) : _logger(logger) {
	trace_with_guard("Constructor of allocator_global_heap worked");
}

allocator_global_heap::~allocator_global_heap() {
	trace_with_guard("Destructor of allocator_global_heap worked");
}
//копирующий конструктор
allocator_global_heap::allocator_global_heap(const allocator_global_heap &other) : _logger(other._logger) {
	trace_with_guard("Copy constructor of allocator_global_heap worked");
}

allocator_global_heap &allocator_global_heap::operator=(const allocator_global_heap &other) {
	if (this != &other) {
		_logger = other._logger;
		trace_with_guard("== of allocator_global_heap worked");
	}

	return *this;
}

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource &other) const noexcept {
	return typeid(*this) == typeid(other);
}
//перемещающий конструктор
allocator_global_heap::allocator_global_heap(allocator_global_heap &&other) noexcept : _logger(other._logger) {
	trace_with_guard("&& constructor of allocator_global_heap worked");
}
//перемещающее присваивание
allocator_global_heap &allocator_global_heap::operator=(allocator_global_heap &&other) noexcept {
	if (this != &other) {
		_logger = other._logger;
		trace_with_guard("&&= of allocator_global_heap worked");
	}

	return *this;
}