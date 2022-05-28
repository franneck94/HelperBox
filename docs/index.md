# HelperBox for GuildWars

This software is based on the GWCA API and has some utilities for UWSC.

If you want to use ToolBox++ in parallel, you have to start Toolbox **before** starting this HelperBox.  

If you want to launch the HelperBox for a certain GW instance (Multi-launcher use-case), you have to get the process id of the GW instance:

```bash
tasklist | grep Gw # for unix like shell
tasklist # for standard cmd
```

Then launch the HelperBox.exe from the CMD:

```bash
HelperBox.exe /pid PID
```

The helper windows are hidden, whenever the player is not in the Underworld or an outpost where you can join the Underworld.

- For EMO Helper see [here](./Emo.md)
- For Mesmer (LT/Spiker) Helper see [here](./Mesmer.md)
- For Terra (currently T2) Helper see [here](./Terra.md)
