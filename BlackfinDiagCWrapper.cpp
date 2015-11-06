#pragma once
#include "BlackfinDiagRuntime.h"

extern "C" void RunDiagnostics() {
	BlackfinDiagRuntimeEnvironment::BlackfinDiagRuntime::ExecuteDiagnostics();
}
