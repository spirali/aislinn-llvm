import os
import subprocess
import shutil
import sys

AISLINN_TESTS = os.path.dirname(os.path.abspath(__file__))
AISLINN_ROOT = os.path.dirname(AISLINN_TESTS)
AISLINN_BUILD = os.path.join(AISLINN_TESTS, "build")
AISLINN_BIN = os.path.join(AISLINN_ROOT, "bin")

AISLINN = os.path.join(AISLINN_BIN, "aislinn")
AISLINN_CPP = os.path.join(AISLINN_BIN, "aislinn-c++")

sys.path.append(os.path.join(AISLINN_ROOT, "tools", "reporttool"))
import report

def cleanup_build_dir():
    if os.path.isdir(AISLINN_BUILD):
        for item in os.listdir(AISLINN_BUILD):
            path = os.path.join(AISLINN_BUILD, item)
            if os.path.isfile(path):
                os.unlink(path)
            else:
                shutil.rmtree(path)
    else:
        os.makedirs(AISLINN_BUILD)

def run(args, cwd=None, no_output=False):
    p = subprocess.Popen(args,
                         cwd=cwd,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()
    if p.returncode != 0:
        raise Exception("Nonzero return code ({0})\n"
                "program: {1}\nstdout:\n{2}stderr:\n{3}\n"
                .format(p.returncode, args, stdout, stderr))
    if no_output and stdout:
        raise Exception("Expected empty std ouput, but got {0} (program: {1})"
                .format(stdout, args))
    if no_output and stderr:
        raise Exception("Expected empty std error, but got {0} (program: {1})"
                .format(stderr, args))

class Program:

    def __init__(self, *args):
        self.path = os.path.join(*((AISLINN_TESTS,) + args))
        self.is_built = False

    def build(self):
        cleanup_build_dir()
        args = (AISLINN_CPP,
                self.path + ".cpp")
        run(args, cwd=AISLINN_BUILD)
        self.is_built = True

    def run(self, processes=1, args=()):
        if not self.is_built:
            self.build()
        run_args = (AISLINN,
                    "-p={0}".format(processes),
                    os.path.basename(self.path) + ".bc") + args
        run(run_args, cwd=AISLINN_BUILD)
        return self.report()

    def build_and_run(self, *args, **kw):
        self.build()
        return self.run(*args, **kw)

    def report(self):
        filename = os.path.join(AISLINN_BUILD, "report.xml")
        return report.Report(filename)
