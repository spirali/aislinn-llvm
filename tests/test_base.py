
from utils import check_prefix, TestCase
import unittest

class BaseTests(TestCase):

    def test_helloworld(self):
        self.program("base", "helloworld")

        self.execute(1)
        self.no_errors()
        self.assertEquals(self.report.number_of_nodes, 1)

        self.execute(2)
        self.no_errors()
        self.assertEquals(self.report.number_of_nodes, 1)

    def test_exitcode(self):
        self.program("base", "exitcode")
        self.execute(1)
        self.exit_code_error(0, 21)
        self.assertEquals(self.report.number_of_nodes, 1)

    def test_arg_p(self):
        self.program("base", "exitcode")
        self.execute(-1, exitcode=1, stderr="Invalid number of processes\n")
        self.execute(0, exitcode=1, stderr="Invalid number of processes\n")

    def test_address_space(self):
        self.program("base", "malloc")

        self.execute(1, ("100",))
        self.no_errors()


        self.execute(1, ("100",), address_space_size="1M")
        self.no_errors()

        self.execute(1,
                     ("10000000",),
                     exitcode=1,
                     stderr=check_prefix("Address space is full"),
                     address_space_size="1M")

        self.execute(1, ("10000000",), address_space_size="100M")
        self.no_errors()

    def test_malloc_free(self):
        self.program("base", "mallocfree")
        self.execute(1)
        self.no_errors()

if __name__ == "__main__":
    unittest.main()
