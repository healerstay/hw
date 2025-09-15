#include "sort.h"

template <size_t N>
void sort(std::array<Complex, N>& arr) {
    for (int i = 1; i < static_cast<int>(N); ++i) {
        Complex key = arr[i];
        int j = i;
        while (j > 0) {
            bool move = false;
            if (arr[j-1].real() > key.real()) {
                move = true;
            }
	    else if (arr[j-1].real() == key.real() && arr[j-1].imag() > key.imag()) {
                move = true;
            }
            if (!move) break;
            arr[j] = arr[j-1];
            --j;
        }
        arr[j] = key;
    }
}
