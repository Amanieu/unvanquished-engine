//@@COPYRIGHT@@

// Random number generation

// Fast generator, based on glibc's rand() implementation
class FastRandom {
public:
	// Initialize the generator with an optional seed
	FastRandom(int32_t seed = 1)
	{
		state = seed;
	}

	// Seed the generator
	void Seed(int32_t seed)
	{
		state = seed;
	}

	// Generate an integer in the range [0, MAX]
	int32_t GenInt()
	{
		state = ((state * 1103515245) + 12345) & MAX;
		return state;
	}

	// Generate a floating point number in the range [0, 1)
	float GenFloat()
	{
		return GenInt() / static_cast<float>(MAX + 1u);
	}

	// Maximum value this generator can generate
	static const int32_t MAX = 0x7fffffffu;

private:
	int32_t state;
};

// Global generator and functions to use it
extern FastRandom fastRandom;
inline int32_t Random()
{
	return fastRandom.GenInt();
}
inline float RandomFloat()
{
	return fastRandom.GenFloat();
}

// Randomly shuffle a range
template<typename Iter> inline void Shuffle(Iter first, Iter last)
{
	typedef typename std::iterator_traits<Iter>::difference_type distance_type;
	distance_type length = std::distance(first, last);
	while (length--) {
		distance_type k = Random() % (length + 1);
		std::swap(first[length], first[k]);
	}
}

// Cryptographically secure random number generator
EXPORT void CryptoRandomBytes(void *buffer, int length);
