static void clamp(float& x) {
	x = GPinToUnit(x);
}

static void repeat(float& x) {
	x = x - GFloorToInt(x);
}

static void mirror(float& x) {
	x = x / 2;
	x -= GFloorToInt(x);
	if (x > 0.5) {
		x = 1 - x;
	}
	x = x * 2;
}


static void(*shadeMode[3])(float& x) = { clamp, repeat, mirror };