import argparse
import os
import subprocess
import sys


def check_for_boolean_value(val):
    if val == "True" or val == "1":
        return True
    return False


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--path",
        help="Path to the executable",
        type=str,
        required=False,
        default=".",
    )
    parser.add_argument(
        "--print",
        help="Whether to only print the process ids",
        type=check_for_boolean_value,
        choices=[True, False],
        required=False,
        default=False,
    )
    args = parser.parse_args()
    print_only = args.print
    exe_path = args.path

    proc = subprocess.Popen(
        "tasklist", shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE
    )
    stdout, _ = proc.communicate()
    output = str(stdout, encoding="utf-8")
    splitted_out = output.replace("  ", " ").split("\n")
    gw_instances = list(filter(lambda x: "gw.exe" in x.lower(), splitted_out))

    pids = []
    for gw_instance in gw_instances:
        info = gw_instance[: gw_instance.find(" Console")]
        pid = info[info.rfind(" ") :].replace("  ", " ")
        pids.append(pid)

    if len(pids) == 0:
        print("No GW instance found")
        return 1

    print("Select PID: ")
    for idx, pid in enumerate(pids):
        print(f"{idx}: {pid}")

    if print_only:
        return 0

    selected_pid_idx = int(input("PID: "))
    selected_pid = pids[selected_pid_idx].replace(" ", "")
    print(selected_pid)

    full_exe_path = os.path.join(exe_path, "HelperBox.exe")
    command = f"{full_exe_path} /pid {selected_pid}"

    print(f"Command to run:\n{command}")

    proc = subprocess.Popen(
        command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE
    )
    stdout, _ = proc.communicate()
    return 0


if __name__ == "__main__":
    sys.exit(main())
