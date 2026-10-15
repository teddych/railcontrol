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

#include <string>
#include <vector>

#if !defined(USE_CPPTRACE) && defined(__CYGWIN__)
#include <unwind.h>
#ifdef CopyFile
#undef CopyFile
#endif

namespace Utils
{
	class Unwind
	{
		public:
			Unwind() = delete;
			Unwind(const Unwind&) = delete;
			Unwind& operator=(const Unwind&) = delete;

			struct BacktraceState
			{
				void** current;
				void** end;
			};

			static _Unwind_Reason_Code CollectBacktrace(struct _Unwind_Context* context, void* argument);
			static int GenerateBacktrace(void** trace, const int maxTrace);
			static void SymbolizeBacktraceFrames(void** trace, const int traceSize, std::vector<std::string>& messages);
	};
} // namespace Utils
#endif