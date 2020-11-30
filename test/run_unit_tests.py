import argparse
import os
import subprocess
import sys

is_failed = False

def get_tests():
    """ Gets all files matching the following criteria in the current working directory.
        Matching files start with the character sequence "test_", have no filename extension, and are executable.
    """
    for node in os.listdir("."):
        if (os.path.isfile(node)):
            if(os.access(node, os.X_OK) and node.startswith("test") and node.find(".") == -1):
                yield node

def run_test(executable):
    """ Runs the given executable, filtering out ERS log messages and XML metadata.
    """
    proc = subprocess.Popen(f"./{executable} --catch_system_error=yes --report_level=no --log_format=JUNIT", shell=True, stdout=subprocess.PIPE)
    out = b""
    for line in proc.stdout:
        # Need to filter out ERS logs...
        if (line.find(b"LOG") == -1 and (line.find(b"<?xml") == -1 or line.find(b"?>") == -1)):
            out += line

    out += b"\n"
    # 10 second timeout until termination
    proc.wait(10)
    if (proc.returncode != 0):
        global is_failed
        is_failed = True
    return out.decode("utf-8")

def write_report(reports, output_file):
    """ Collates the report strings in the reports list to output_file, re-adding XML metadata.
    """
    outfile = open(output_file, "w+")
    outfile.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
    outfile.write("<testsuites>\n")
    for report in reports:
        outfile.write(report)
    outfile.write("</testsuites>\n")
    outfile.close()
    
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--test_directory", help="The directory where the test files are found", required=True)
    parser.add_argument("-o", "--output_file", help="Filepath for the report XML to be written to", required=True)
    args = parser.parse_args()
    
    prev_wd = os.getcwd()

    os.chdir(args.test_directory)
    print(f"Running unit tests in {os.getcwd()}")
    reports = []
    for filename in get_tests():
        print(f"\tRunning test executable {filename}")
        reports += run_test(filename)
    
    os.chdir(prev_wd)
    print(f"Writing report to {args.output_file}")
    write_report(reports, args.output_file)
    if (is_failed):
        print("One or more tests failed, setting exit code to 1!")
        sys.exit(1)
    
if __name__ == "__main__":
    main()
