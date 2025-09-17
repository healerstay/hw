#ifndef SORT_H
#define SORT_H

#include <array>
#include "complex.h"

template <size_t n>
void sort(std::array<Complex, n>& arr) {
	for (int i = 1; i < n; ++i) {
		Complex key = arr[i];
		int j = i - 1;
		while (j >= 0 && arr[j].abs() > key.abs()) {
			arr[j+1] = arr[j];
			--j;
		}
		arr[j+1] = key;
	}
}

#endif //SORT_H

