from subprocess import Popen, PIPE
import os

rand_generator_path = "/home/ajuncosa/Desktop/malloc/test/random_tester/rand-generator.py"
instructions_output_file = "/home/ajuncosa/Desktop/malloc/build/test"
rand_test_executable_path = "/home/ajuncosa/Desktop/malloc/build/bin/rand_test"

cwd = os.path.dirname(os.path.realpath(__file__))

for i in range(0, 1000):

    print(f"Test index: {i}")

    process = Popen(["python3", rand_generator_path, f"{instructions_output_file}/tmp_rand_commands", "5000", "3000", "0", "50000", "1000"], stdout=PIPE, cwd=cwd)
    (output, err) = process.communicate()
    exit_code = process.wait()

    process = Popen([rand_test_executable_path, f"{instructions_output_file}/tmp_rand_commands"], stdout=PIPE, cwd=cwd)
    (output, err) = process.communicate()
    exit_code = process.wait()

    if exit_code < 0:
        print("TEST FAILED")
        exit(1)

print("Test completed OK")