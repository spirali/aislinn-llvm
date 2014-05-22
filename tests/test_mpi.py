
from utils import TestCase
import unittest

class MPITests(TestCase):

    def test_isend_irecv(self):
        self.program("mpi", "isir")

        self.execute(2, ("1",))
        self.no_errors()

        self.execute(2, ("0",))
        self.single_error("deadlock")

    def test_test_recv(self):
        self.program("mpi", "test_recv")

        self.execute(2, ("0", "0"))
        self.no_errors()

        self.execute(2, ("1", "0"))
        self.exit_code_error(0, 1)

        self.execute(2, ("0", "2"))
        self.exit_code_error(0, 2)

    def test_match1(self):
        self.program("mpi", "match1")
        self.execute(2)
        self.no_errors()

    def test_waitall(self):
        self.program("mpi", "waitall")
        self.execute(3, ("a",))
        self.no_errors()
        self.assertEquals(4, self.report.number_of_nodes)

        self.execute(3, ("b",))
        self.no_errors()
        self.assertEquals(11, self.report.number_of_nodes)

        self.execute(3, ("c",))
        self.no_errors()
        self.assertEquals(15, self.report.number_of_nodes)


if __name__ == "__main__":
    unittest.main()
