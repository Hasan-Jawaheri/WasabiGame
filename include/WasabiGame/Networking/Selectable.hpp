#pragma once

namespace WasabiGame {

	class Selectable {
	protected:
		int m_fd;

	public:
		Selectable(int fd) {
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
