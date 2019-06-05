#pragma once

namespace HBUtils {

	struct CircularBuffer {
		char* mem;
		int start, end;
		size_t memSize;

		void Initialize(size_t _size) {
			mem = new char[_size];
			memSize = _size;
			start = 0;
			end = 0;
		}

		void Expand() {
			char* newBuf = new char[memSize * 2];
			memcpy(newBuf, mem + start, memSize - start);
			memcpy(newBuf + (memSize - start), mem, end);
			int oldSize = GetSize();

			start = 0;
			end = oldSize;
			memSize *= 2;
			delete[] mem;
			mem = newBuf;
		}

		void Destroy() {
			delete[] mem;
		}

		inline char* GetMem() {
			return mem + start;
		}

		inline size_t GetAvailableContigiousInsert() {
			// number of bytes available from "end" to "memSize", or "end" to "start" if "end" < "start"
			if (end >= start)
				return memSize - end;
			else
				return start - end;
		}

		inline size_t GetAvailableContigiousConsume() {
			// number of bytes from "start" to "end", or "start" to "memSize" if "end" < "start"
			if (end >= start)
				return end - start;
			else
				return memSize - start;
		}

		inline size_t GetSize() {
			if (end >= start)
				return end - start;
			else
				return (memSize - start) + end;
		}

		inline void OnInserted(size_t numInserted) {
			end = (end + numInserted) % memSize;
		}

		inline void OnConsumed(size_t numConsumed) {
			start = (start + numConsumed) % memSize;
		}
	};

};
