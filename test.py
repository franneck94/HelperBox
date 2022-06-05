import pymem


def main():
    pm = pymem.Pymem('GW.exe')
    modules = list(pm.list_modules())
    for module in modules:
        print(module.name)


if __name__ == "__main__":
    main()
