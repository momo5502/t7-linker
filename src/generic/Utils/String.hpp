#pragma once

#ifndef STD_INCLUDED
#error "Missing standard header"
#endif

namespace Utils
{
	class String : public std::string
	{
	public:
		template <size_t Buffers, size_t MinBufferSize>
		class VAProvider
		{
		public:
			static_assert(Buffers != 0 && MinBufferSize != 0, "Buffers and MinBufferSize mustn't be 0");

			VAProvider() : currentBuffer(0) {}
			~VAProvider() {}

			char* get(const char* format, va_list ap)
			{
				++this->currentBuffer %= ARRAYSIZE(this->stringPool);
				auto entry = &this->stringPool[this->currentBuffer];

				if (!entry->size || !entry->buffer)
				{
					throw std::runtime_error("String pool not initialized");
				}

				while (true)
				{
					#ifdef _WIN32
					int res = vsnprintf_s(entry->buffer, entry->size, _TRUNCATE, format, ap);
					#else
					int res = vsnprintf(entry->buffer, entry->size, format, ap);
					#endif
					
					if (res > 0) break; // Success
					if (res == 0)
					{
						entry->buffer[0] = 0;
						break; // Error
					}

					entry->doubleSize();
				}

				return entry->buffer;
			}

		private:
			class Entry
			{
			public:
				Entry(size_t _size = MinBufferSize) : size(_size), buffer(nullptr)
				{
					if (this->size < MinBufferSize) this->size = MinBufferSize;
					this->allocate();
				}

				~Entry()
				{
					if (this->buffer) Utils::Memory::GetAllocator()->free(this->buffer);
					this->size = 0;
					this->buffer = nullptr;
				}

				void allocate()
				{
					if (this->buffer) Utils::Memory::GetAllocator()->free(this->buffer);
					this->buffer = Utils::Memory::GetAllocator()->allocateArray<char>(this->size + 1);
				}

				void doubleSize()
				{
					this->size *= 2;
					this->allocate();
				}

				size_t size;
				char* buffer;
			};

			size_t currentBuffer;
			Entry stringPool[Buffers];
		};

		static const char *VA(const char *fmt, ...);
		static std::string DumpHex(std::string data, std::string separator);
        static std::vector<String> split(const String& rawInput, const std::vector<char>& delims);
        static std::vector<String> split(const String& rawInput, const char& delim) { return split(rawInput, std::vector<char>{ delim }); }

//----------------------------------------------------------------------------------------

		String() : std::string() {}
		String(std::string data) : std::string(data) {}
		String(const char* data, size_t len) : std::string(data, len) {}
		String(const char* data) : std::string(data) {}

		String toLower();
		String toUpper();

		bool startsWith(Utils::String otherString);
		bool endsWith(Utils::String otherString);
		String replaceAll(std::string find, std::string replace);

        std::vector<String> split(const std::vector<char>& delims) { return String::split(*this, delims); }
        std::vector<String> split(const char& delim) { return String::split(*this, std::vector<char>{ delim }); }
	};
}
