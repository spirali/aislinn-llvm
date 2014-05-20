
from utils import check_prefix, TestCase
import unittest

class BasicTests(TestCase):

    def test_nompi(self):
        self.program("basic", "nompi")

        self.execute(1)
        self.no_errors()
        self.assertEquals(self.report.number_of_nodes, 1)

        self.execute(2)
        self.no_errors()
        self.assertEquals(self.report.number_of_nodes, 1)

    def test_isend_irecv(self):
        self.program("basic", "isir")

        self.execute(2, ("1",))
        self.no_errors()

        self.execute(2, ("0",))
        self.single_error("deadlock")

    def test_exitcode(self):
        self.program("basic", "exitcode")
        self.execute(1)
        self.exit_code_error(0, 21)
        self.assertEquals(self.report.number_of_nodes, 1)

    def test_test_recv(self):
        self.program("basic", "test_recv")

        self.execute(2, ("0", "0"))
        self.no_errors()

        self.execute(2, ("1", "0"))
        self.exit_code_error(0, 1)

        self.execute(2, ("0", "2"))
        self.exit_code_error(0, 2)

    def test_address_space(self):
        self.program("basic", "malloc")

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

if __name__ == "__main__":
    unittest.main()
