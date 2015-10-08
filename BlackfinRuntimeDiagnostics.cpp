#include "BlackfinDiagExecution.h"
#include "BlackfinRuntimeDiagnostics.h"

//BlackfinDiagRegister BlackfinRuntimeDiagnostics::RegisterTest;
 
BlackfinDiagInstructionRam BlackfinRuntimeDiagnostics::InstructionRamTest;
	
BlackfinDiagDataRam BlackfinRuntimeDiagnostics::DataRamTest;
    
BlackfinDiagTest * BlackfinRuntimeDiagnostics::DiagnosticTests[] = {&BlackfinRuntimeDiagnostics::DataRamTest, 
                                                                    &BlackfinRuntimeDiagnostics::InstructionRamTest };

std::vector <BlackfinDiagTest *> BlackfinRuntimeDiagnostics::Diagnostics(DiagnosticTests, end(DiagnosticTests));

void BlackfinRuntimeDiagnostics::ExecuteDiagnostics() { 
   
    static BlackfinDiagExecution de(&BlackfinRuntimeDiagnostics::Diagnostics);

	de.RunScheduled(); 
}

BlackfinRuntimeDiagnostics::BlackfinRuntimeDiagnostics() {

}

