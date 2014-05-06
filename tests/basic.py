
from utils import Program
import unittest

class BasicTests(unittest.TestCase):

    # Utils ---------------------------------------------------------

    def no_errors(self, report):
        self.assertEquals(len(report.errors()), 0)

    def single_error(self, report, error_type):
        errors = report.errors()
        self.assertEquals(len(errors), 1)
        error = errors[0]
        self.assertEquals(error.get("type"), error_type)
        return error

    def check_child(self, element, name, value):
        child = element.find(name)
        self.assertTrue(child is not None,
                        "Node {0} has not child {1}".format(element, name))
        self.assertEquals(element.find(name).text, str(value))

    def exit_code_error(self, report, rank, exitcode=0):
        error = self.single_error(report, "exitcode")
        self.assertEquals(error.get("rank"), str(rank))
        self.assertEquals(error.get("exitcode"), str(exitcode))

    # Tests ---------------------------------------------------------

    def test_nompi(self):
        program = Program("basic", "nompi")

        report = program.run(1)
        self.no_errors(report)
        self.assertEquals(report.number_of_nodes, 1)

        report = program.run(2)
        self.no_errors(report)
        self.assertEquals(report.number_of_nodes, 1)

    def test_isend_irecv(self):
        program = Program("basic", "isir")

        report = program.run(2, ("1",))
        self.no_errors(report)

        report = program.run(2, ("0",))
        self.single_error(report, "deadlock")

    def test_exitcode(self):
        report = Program("basic", "exitcode").run(1)
        self.assertEquals(report.number_of_nodes, 1)
        self.exit_code_error(report, 0, 21)

    def test_test_recv(self):
        program = Program("basic", "test_recv")

        report = program.run(2, ("0", "0"))
        self.no_errors(report)

        report = program.run(2, ("1", "0"))
        self.exit_code_error(report, 0, 1)

        report = program.run(2, ("0", "2"))
        self.exit_code_error(report, 0, 2)

if __name__ == "__main__":
    unittest.main()
