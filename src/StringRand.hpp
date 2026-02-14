#include <iostream>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>

class StringRand
{
public:
	static std::string Simple(size_t length)
	{
		// Character set includes printable ASCII characters (excluding space for simplicity)
		const std::string chars =
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"1234567890!@#$%^&*()"
			;
		
		// Use a random device to seed the generator
		std::random_device rd;
		std::mt19937 generator(rd());
		// Define the distribution based on the character set size
		std::uniform_int_distribution<> dist(0, chars.length() - 1);

		std::string result;
		result.reserve(length); // Reserve memory for efficiency

		// Generate random characters and append to the string
		for(size_t i = 0; i < length; ++i)
			result += chars[dist(generator)];

		return result;
	}
};