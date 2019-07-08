//Blend mode implementations

static const GPixel kClear(GPixel& source, GPixel& dest) {	//!<     0
	GPixel out = 0;
	return out;
}

static const GPixel kSrc(GPixel& source, GPixel& dest) {	//!<     S
	return source;
}

static const GPixel kDst(GPixel& source, GPixel& dest) {	//!<     D
	return dest;
}


static const GPixel kSrcOver(GPixel& source, GPixel& dest) {	//!<     S + (1 - Sa)*D
	int Sa = GPixel_GetA(source);

	if (Sa == 0) {
		return source + dest;
	}
	else if (Sa == 255) {
		return source;
	}

	GPixel out = source + quad_mul_div255(dest, 255 - Sa);
	return out;
}

static const GPixel kDstOver(GPixel& source, GPixel& dest) {	//!<     D + (1 - Da)*S
	int Da = GPixel_GetA(dest);

	if (Da == 0) {
		return source + dest;
	}
	else if (Da == 255) {
		return dest;
	}

	GPixel out = dest + quad_mul_div255(source, 255 - Da);
	return out;
}

static const GPixel kSrcIn(GPixel& source, GPixel& dest) {	//!<     Da * S
	int Da = GPixel_GetA(dest);
	if (Da == 0) {
		return 0;
	}
	else if (Da == 255) {
		return source;
	}

	GPixel out = quad_mul_div255(source, Da);
	return out;
}

static const GPixel kDstIn(GPixel& source, GPixel& dest) {	//!<     Sa * D
	int Sa = GPixel_GetA(source);
	if (Sa == 0) {
		return 0;
	}
	else if (Sa == 255) {
		return dest;
	}

	GPixel out = quad_mul_div255(dest, Sa);
	return out;
}

static const GPixel kSrcOut(GPixel& source, GPixel& dest) {  //!<     (1 - Da)*S
	int Da = GPixel_GetA(dest);

	if (Da == 0) {
		return source;
	}
	else if (Da == 255) {
		return 0;
	}

	GPixel out = quad_mul_div255(source, 255 - Da);
	return out;
}

static const GPixel kDstOut(GPixel& source, GPixel& dest) {  //!<     (1 - Sa)*D
	int Sa = GPixel_GetA(source);
	if (Sa == 0) {
		return dest;
	}
	else if (Sa == 255) {
		return 0;
	}

	GPixel out = quad_mul_div255(dest, 255 - Sa);
	return out;
}

static const GPixel kSrcATop(GPixel& source, GPixel& dest) { //!<     Da*S + (1 - Sa)*D
	int Sa = GPixel_GetA(source);
	int Da = GPixel_GetA(dest);

	if (Sa == 0) {  //Da*S+D
		return quad_mul_div255(source, dest) + dest;
	}
	else if (Sa == 255) { //Da*S
		return kSrcIn(source, dest);
	}
	else if (Da == 0) {  //(1-Sa)*D
		return kDstOut(source, dest);
	}
	else if (Da == 255) {  //S+(1-Sa)*D
		return kSrcOver(source, dest);
	}


	GPixel out = quad_div255(quad_mul(source, Da) + quad_mul(dest, 255 - Sa));
	return out;
}

static const GPixel kDstATop(GPixel& source, GPixel& dest) { //!<     Sa*D + (1 - Da)*S
	int Sa = GPixel_GetA(source);
	int Da = GPixel_GetA(dest);

	if (Sa == 0) {
		return kSrcOut(source, dest);
	}
	else if (Sa == 255) {
		return kDstOver(source, dest);
	}
	else if (Da == 0) { //Sa*D +S
		return quad_mul_div255(dest, Sa) + source;
	}
	else if (Da == 255) {
		return kDstIn(source, dest);
	}


	GPixel out = quad_div255(quad_mul(dest, Sa) + quad_mul(source, 255 - Da));
	return out;
}


static const GPixel kXor(GPixel& source, GPixel& dest) { //!<     (1 - Sa)*D + (1 - Da)*S
	int Sa = GPixel_GetA(source);
	int Da = GPixel_GetA(dest);

	if (Sa == 0) { //D+(1-Da)*S
		return kDstOver(source, dest);
	}
	else if (Sa == 255) { //(1-Da)*S
		return kSrcOut(source, dest);
	}
	else if (Da == 255) { //(1-Sa)*D
		return kDstOut(source, dest);
	}
	else if (Da == 0) { //S + (1-Sa)*D
		return kSrcOver(source, dest);
	}

	GPixel out = quad_div255(quad_mul(dest, 255 - Sa) + quad_mul(source, 255 - Da));
	return out;
}

static const GPixel(*BlendProc[12])(GPixel&, GPixel&) = { kClear, kSrc, kDst, kSrcOver, kDstOver, kSrcIn, kDstIn, kSrcOut, kDstOut, kSrcATop, kDstATop, kXor };
