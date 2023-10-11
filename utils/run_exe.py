import subprocess
import os


def run_lox_test_exe(test_file):
    '''
    NOTE: lox executable has to have been make for this function to work
    Takes a file from the test directory and runs it with the executable 'lox' 
    from the top directory 

    Example usage: 
        run_lox_test_exe('tests/speedFib.lox')
    '''
    script_directory = os.path.dirname(os.path.realpath(__file__)) + '/../'
    EXECUTABLE = os.path.join(script_directory, 'lox')
    test_file = os.path.join(script_directory, f'{test_file}')

    try:
        command = [EXECUTABLE] + [test_file]
        result = subprocess.run(
            command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
        return str(result.stdout)
    except subprocess.CalledProcessError as e:
        return f"Error running 'lox':\n{e.stderr}"
    except FileNotFoundError:
        return f"'{EXECUTABLE}' executable not found. Please specify the correct path to '{EXECUTABLE}'."


# test
if __name__ == '__main__':
    script_name = 'tests/testArithmatic/test_negate.lox'

    print(run_lox_test_exe(script_name))
