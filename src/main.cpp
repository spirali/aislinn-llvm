//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Main file of Aislinn
///
//===----------------------------------------------------------------------===//

#include "MManagment/MemoryManager.h"
#include "MPI/InterpreterMPI.h"
#include "MPI/StateSpace.h"
#include "Utils/XML.h"
#include "Utils/Debug.h"
#include "Utils/String.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/DynamicLibrary.h"
#include <stdlib.h>

using namespace llvm;
using namespace aislinn;

namespace {
  cl::opt<std::string>
  InputFile(cl::desc("<input bitcode>"), cl::Positional, cl::init("-"));

  cl::opt<int>
  NumberOfProcesses("p", cl::desc("Number of processes."), cl::init(1));

  cl::opt<bool>
  WriteDotFile("write-dot",
               cl::desc("Create statespace.dot"), cl::init(false));

  cl::list<std::string>
  InputArgv(cl::ConsumeAfter,
            cl::desc("<program arguments>..."));

  cl::opt<int>
  VerbosityLevelArg("verbose",
                    cl::desc("Verbosity level. Default: 1"),
                    cl::init(1));

  cl::opt<std::string>
  AddressSpaceSizeStr("address-space-size",
    cl::desc("Size of address space in bytes. "
             "Default: 1G (on 64b) / 128M (on 32b)"),
    cl::init("default"));

}

static bool init(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal();
  atexit(llvm::llvm_shutdown);

  cl::ParseCommandLineOptions(argc, argv,
                              "MPI verification tool\n");

  if (VerbosityLevelArg < 0) {
    errs() << "Invalid verbosity level\n";
    return false;
  }
  VerbosityLevel = VerbosityLevelArg;

  if (NumberOfProcesses <= 0) {
    errs() << "Invalid number of processes\n";
    return false;
  }

  size_t ASpaceSize = 0;
  if (AddressSpaceSizeStr != "default") {
    ASpaceSize = parseSizeString(AddressSpaceSizeStr);
    if (ASpaceSize == 0) {
      llvm::errs() << "Invalid address space size\n";
      return 1;
    }
  }
  MemoryManager::init(ASpaceSize);


  std::string ErrorStr;
  // NULL = load program itself, not a library
  if (sys::DynamicLibrary::LoadLibraryPermanently(NULL, &ErrorStr)) {
    errs() << "LoadLibraryPermanently: " << ErrorStr;
    return false;
  }

  /*if (sys::DynamicLibrary::LoadLibraryPermanently("/usr/lib/libblas.so.3", &ErrorStr)) {
    errs() << "LoadLibraryPermanently: " << ErrorStr;
    return false;
  }*/

  return true;
}

static void writeReport(StateSpace &SSpace)
{
  IFVERBOSE(2) {
    errs() << "Writing report file\n";
  }

  XML Report("report.xml");
  Report.child("report");
  Report.set("version", "0");

  Report.child("program");
  Report.simpleChild("name", InputFile);
  Report.simpleChild("processes", NumberOfProcesses);
  Report.back(); // program

  Report.child("settings");
  Report.simpleChild("pagesize", PageSize);
  Report.back(); // runinfo

  SSpace.writeReport(Report);

  Report.back(); // report
}

int main(int argc, char **argv, char* const* envp)
{
  if (!init(argc, argv)) {
    return 1;
  }

  LLVMContext &Context = getGlobalContext();
  SMDiagnostic Err;
  Module *Mod = ParseIRFile(InputFile, Err, Context);
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }

  InterpreterMPI Intr(Mod);
  // This should be probably need to be called in context of a particular
  // ExecutionState
  Intr.runStaticConstructorsDestructors(false);

  ProgramState *InitialState = new ProgramState(NumberOfProcesses);
  Intr.setProgramState(InitialState);
  InitialState->runInitialization(&Intr);

  std::vector<std::string> Args;
  Args.push_back(InputFile);
  for (size_t i = 0; i < InputArgv.size(); i++) {
    Args.push_back(InputArgv[i]);
  }
  StateSpace SSpace(&Intr, InitialState, Args);
  Args.clear();

  SSpace.build();

  if (WriteDotFile) {
    SSpace.writeDotFile("statespace.dot");
  }
  writeReport(SSpace);
  IFVERBOSE(1) {
    int Errors = SSpace.getErrorsSize();
    if (Errors == 0) {
      outs() << "No errors found\n";
    } else {
      outs() << Errors << " errors(s) found\n";
    }
  }
  return 0;
}
