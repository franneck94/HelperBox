import subprocess


def main() -> None:
    proc = subprocess.Popen(
        'tasklist',
        shell=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE
    )
    stdout, _ = proc.communicate()
    output = str(stdout, encoding='utf-8')
    splitted_out = output.replace('  ', ' ').split("\n")
    gw_instances = list(filter(lambda x: "gw.exe" in x.lower(), splitted_out))

    pids = []
    for gw_instance in gw_instances:
        info = gw_instance[:gw_instance.find(' Console')]
        pid = info[info.rfind(" "):].replace('  ', ' ')
        pids.append(pid)

    print("Select PID: ")
    for idx, pid in enumerate(pids):
        print(f"{idx}: {pid}")

    selected_pid_idx = int(input("PID: "))
    selected_pid = pids[selected_pid_idx]
    print(selected_pid)

    command = f'.\\bin\\Release\\Helperbox.exe /pid {selected_pid}'
    print(f"Command to run:\n{command}")

    proc = subprocess.Popen(
        command,
        shell=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE
    )
    stdout, _ = proc.communicate()


if __name__ == "__main__":
    main()
