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

#include "Utils/Backtrace.h"

#if !defined(USE_CPPTRACE) && (defined(__CYGWIN__) || defined(__GLIBC__))

#include <cstdio>
#include <cstdint>
#include <iomanip>
#include <map>
#include <sstream>
#include <unistd.h>

#if defined(__GLIBC__)
#include <execinfo.h>
#include <link.h>
#endif

#include "Languages.h"
#include "Logger/Logger.h"
#include "Utils/Integer.h"

using std::string;
using std::vector;

namespace Utils
{
#if defined(__GLIBC__)
	namespace
	{
		struct FrameModule
		{
			uintptr_t address;
			uintptr_t loadBias;
			string path;
		};

		int FindFrameModule(struct dl_phdr_info* info, size_t, void* argument)
		{
			FrameModule& frame = *static_cast<FrameModule*>(argument);
			for (unsigned int i = 0; i < info->dlpi_phnum; ++i)
			{
				const auto& segment = info->dlpi_phdr[i];
				if (segment.p_type != PT_LOAD)
				{
					continue;
				}
				const uintptr_t start = info->dlpi_addr + segment.p_vaddr;
				if (frame.address >= start && frame.address - start < segment.p_memsz)
				{
					frame.loadBias = info->dlpi_addr;
					frame.path = info->dlpi_name;
					return 1;
				}
			}
			return 0;
		}
	}
#endif

	string Backtrace::ReadAddr2LineOutputLine(FILE* pipe)
	{
		char buffer[512];
		if (fgets(buffer, sizeof(buffer), pipe) == nullptr)
		{
			return "";
		}

		string line(buffer);
		while (!line.empty() && (line[line.length() - 1] == '\n' || line[line.length() - 1] == '\r'))
		{
			line.resize(line.length() - 1);
		}
		return line;
	}

	string Backtrace::ShellQuote(const string& value)
	{
		string quoted("'");
		for (const char character : value)
		{
			if (character == '\'')
			{
				quoted += "'\\''";
			}
			else
			{
				quoted += character;
			}
		}
		quoted += "'";
		return quoted;
	}

	string Backtrace::GetExecutablePath()
	{
		char buffer[4096];
		const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
		if (length < 0)
		{
			return "/proc/" + std::to_string(getpid()) + "/exe";
		}
		buffer[length] = 0;
		return string(buffer);
	}

	string Backtrace::FrameNumberString(const int frameNumber)
	{
		std::stringstream frameNumberString;
		frameNumberString << std::setfill(' ') << std::setw(2) << frameNumber;
		return frameNumberString.str();
	}

	void Backtrace::LogBacktrace(const unsigned long address)
	{
		Logger::Logger* logger = Logger::Logger::GetLogger("Backtrace");

		logger->Error(Languages::TextSigSegvReceivedWithBacktrace, Integer::IntegerToHex(address, 8));

		static const int MaxTrace = 32;
#if defined(__GLIBC__)
		static const int SkipTraceFrames = 3;
#else
		static const int SkipTraceFrames = 9;
#endif

		void* trace[MaxTrace];

#if defined(__GLIBC__)
		const int traceSize = backtrace(trace, MaxTrace);
#else
		const int traceSize = GenerateUnwindBacktrace(trace, MaxTrace);
#endif
		const int skippedTraceSize = traceSize > SkipTraceFrames ? traceSize - SkipTraceFrames : 0;
		vector<string> messages;
		SymbolizeBacktraceFrames(trace + SkipTraceFrames, skippedTraceSize, messages);

		for(int i = 0; i < skippedTraceSize; ++i)
		{
			logger->Error(Languages::TextBacktraceLine, FrameNumberString(i), messages[i]);
		}
	}

	void Backtrace::SymbolizeBacktraceFrames(void** trace, const int traceSize, vector<string>& messages)
	{
		messages.clear();
		if (traceSize <= 0)
		{
			return;
		}

		const string executablePath = GetExecutablePath();
		std::map<string, vector<int>> moduleFrames;
		vector<uintptr_t> symbolAddresses;
		for(int i = 0; i < traceSize; ++i)
		{
			const uintptr_t address = reinterpret_cast<uintptr_t>(trace[i]);
			messages.push_back("0x" + Integer::IntegerToHex(address, 12));
			string modulePath = executablePath;
			uintptr_t symbolAddress = address;
#if defined(__GLIBC__)
			FrameModule frame = { address, 0, "" };
			if (dl_iterate_phdr(FindFrameModule, &frame) != 0)
			{
				// ELF load bias is zero for non-PIE executables.
				symbolAddress -= frame.loadBias;
				if (!frame.path.empty())
				{
					modulePath = frame.path;
				}
			}
#endif
			symbolAddresses.push_back(symbolAddress);
			moduleFrames[modulePath].push_back(i);
		}

		for (const auto& module : moduleFrames)
		{
			string command = "/usr/bin/addr2line -f -C -e " + ShellQuote(module.first);
			for (const int i : module.second)
			{
				command += " 0x" + Integer::IntegerToHex(symbolAddresses[i], 12);
			}
			command += " 2>/dev/null";

			FILE* pipe = popen(command.c_str(), "r");
			if (!pipe)
			{
				continue;
			}

			for (const int i : module.second)
			{
				const string functionName = ReadAddr2LineOutputLine(pipe);
				const string fileAndLine = ReadAddr2LineOutputLine(pipe);
				if (!functionName.empty() && !fileAndLine.empty() && (functionName != "??" || fileAndLine != "??:0"))
				{
					messages[i] += ": " + functionName + " at " + fileAndLine;
				}
			}
			pclose(pipe);
		}
	}

#if defined(__CYGWIN__)
	_Unwind_Reason_Code Backtrace::CollectUnwindBacktrace(struct _Unwind_Context* context, void* argument)
	{
		BacktraceState* state = static_cast<BacktraceState*>(argument);
		uintptr_t instructionPointer = _Unwind_GetIP(context);
		if (instructionPointer == 0)
		{
			return _URC_NO_REASON;
		}

		if (state->current == state->end)
		{
			return _URC_END_OF_STACK;
		}

		*state->current = reinterpret_cast<void*>(instructionPointer);
		++state->current;
		return _URC_NO_REASON;
	}

	int Backtrace::GenerateUnwindBacktrace(void** trace, const int maxTrace)
	{
		BacktraceState state = { trace, trace + maxTrace };
		_Unwind_Backtrace(CollectUnwindBacktrace, &state);
		return state.current - trace;
	}
#endif
} // namespace Utils
#endif