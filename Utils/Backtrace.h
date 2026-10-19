/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2026 by Teddy / Dominik Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#pragma once

#if defined(__linux__)
#include <features.h>
#endif

#if defined(__CYGWIN__) || defined(__GLIBC__)

#include <cstdio>
#include <string>
#include <vector>

#if defined(__CYGWIN__)
#include <unwind.h>
#ifdef CopyFile
#undef CopyFile
#endif
#endif

namespace Utils
{
	class Backtrace
	{
		public:
			Backtrace() = delete;
			Backtrace(const Backtrace&) = delete;
			Backtrace& operator=(const Backtrace&) = delete;

			static void LogBacktrace(const unsigned long address);
			static void SymbolizeBacktraceFrames(void** trace, const int traceSize, std::vector<std::string>& messages);

		private:
			static std::string ReadAddr2LineOutputLine(FILE* pipe);
			static std::string ShellQuote(const std::string& value);
			static std::string GetExecutablePath();
			static std::string FrameNumberString(const int frameNumber);

#if defined(__CYGWIN__)
		public:
			struct BacktraceState
			{
				void** current;
				void** end;
			};

			static _Unwind_Reason_Code CollectUnwindBacktrace(struct _Unwind_Context* context, void* argument);
			static int GenerateUnwindBacktrace(void** trace, const int maxTrace);
#endif
	};
} // namespace Utils
#endif