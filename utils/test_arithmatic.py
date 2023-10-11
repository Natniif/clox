import unittest
from run_exe import run_lox_test_exe


class ArithmeticTests(unittest.TestCase):
    def test_addition(self):
        # bit hacky but only way to test for now
        result = str.strip(run_lox_test_exe(
            'tests/testArithmatic/test_addition.lox'))
        self.assertEqual(int(result), 2)

    def test_subtraction(self):
        result = str.strip(run_lox_test_exe(
            'tests/testArithmatic/test_minus.lox'))
        self.assertEqual(int(result), 2)

    def test_multiplication(self):
        result = str.strip(run_lox_test_exe(
            'tests/testArithmatic/test_multiply.lox'))
        self.assertEqual(int(result), 2)

    def test_division(self):
        result = str.strip(run_lox_test_exe(
            'tests/testArithmatic/test_divide.lox'))
        self.assertEqual(int(result), 2)

    def test_negate(self):
        result = str.strip(run_lox_test_exe(
            'tests/testArithmatic/test_negate.lox'))
        self.assertEqual(float(result), -2.2)

    def test_string_addition(self):
        result = str.strip(run_lox_test_exe(
            'tests/testArithmatic/test_string_addition.lox'))
        self.assertEqual(result, "Hello World!")


if __name__ == '__main__':
    unittest.main()
