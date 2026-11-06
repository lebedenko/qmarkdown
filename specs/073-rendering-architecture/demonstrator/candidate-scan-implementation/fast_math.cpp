#include "ScanBound.h"
int main() { return scan_bound::platform<float>()==scan_bound::environment ? 0 : 1; }
