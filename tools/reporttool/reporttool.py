
import argparse
from report import Report

def main():
    parser = argparse.ArgumentParser(description="Aislinn report tool")
    parser.add_argument("report",
                        metavar="FILENAME",
                        type=str,
                        help="path to report")
    parser.add_argument("--output",
                        metavar="TYPE",
                        type=str,
                        help="Type of output. Value 'html' is now the only supported option.")
    args = parser.parse_args()

    report = Report(args.report)
    report.export()

if __name__ == "__main__":
    main()
