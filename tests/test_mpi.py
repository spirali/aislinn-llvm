
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

if __name__ == "__main__":
    unittest.main()
