#pragma once

#include <memory>


namespace WasabiGame {

	class Selectable : public std::enable_shared_from_this<Selectable> {
	protected:
		int m_fd;

	public:
		Selectable(int fd) : enable_shared_from_this<Selectable>() {
			m_fd = fd;
		}

		virtual ~Selectable() {
		}

		virtual inline int fd() {
			return m_fd;
		}

		virtual bool HasPendingWrites() = 0;
		virtual bool OnReadReady() = 0;
		virtual bool OnWriteReady() = 0;
	};

};
