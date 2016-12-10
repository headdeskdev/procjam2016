struct RandomGenerator {	
	U32 seed;

	U32 rand() {
	   seed = seed * 2147001325 + 715136305; 
	   return 0x31415926 ^ ((seed >> 16) + (seed << 16));
	}

	U32 getInteger(U32 max) {
		return rand() % max;
	}

	F32 frand() {
		return rand() / ((double) (1 << 16) * (1 << 16));
	}
};