#include "BlackfinDiagExecution.h"
#include "BlackfinRuntimeDiagnostics.h"

BlackfinDiagInstructionRam BlackfinRuntimeDiagnostics::InstructionRamTest;
	
BlackfinDiagDataRam BlackfinRuntimeDiagnostics::DataRamTest;
    
BlackfinDiagRegistersTest BlackfinRuntimeDiagnostics::RegisterTest;
 
BlackfinDiagTest * BlackfinRuntimeDiagnostics::DiagnosticTests[] 
	= {
		&BlackfinRuntimeDiagnostics::RegisterTest,
		&BlackfinRuntimeDiagnostics::DataRamTest, 
		&BlackfinRuntimeDiagnostics::InstructionRamTest
	  };

std::vector <BlackfinDiagTest *> BlackfinRuntimeDiagnostics::Diagnostics(DiagnosticTests, end(DiagnosticTests));

void BlackfinRuntimeDiagnostics::ExecuteDiagnostics() { 
   
    static BlackfinDiagExecution de(&BlackfinRuntimeDiagnostics::Diagnostics);

	de.RunScheduled(); 
}

BlackfinRuntimeDiagnostics::BlackfinRuntimeDiagnostics() {

}

