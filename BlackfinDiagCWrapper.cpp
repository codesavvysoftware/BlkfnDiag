#pragma once
#include "BlackfinDiagRuntime.hpp"

extern "C" void RunDiagnostics() {
	BlackfinDiagRuntimeEnvironment::BlackfinDiagRuntime::ExecuteDiagnostics();
}
