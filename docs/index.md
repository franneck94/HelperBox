# HelperBox for GuildWars

This software is based on the GWCA API and has some utilities for UWSC.

If you want to use ToolBox++ in parallel, you have to start Toolbox **before** starting this HelperBox.  

If you want to launch the HelperBox for a certain GW instance (Multi-launcher use-case), you have to get the process id of the GW instance:

Then launch the HelperBox.exe from the CMD:

```bash
HelperBox.exe /pid PID
```

For that, you can run the **get_pids.py** python script which will scan for all open GW instances and will output the PIDs.

The helper windows are hidden, whenever:

- The player is not in the UW or an outpost where you can join the UW
- A certain window is not displayed if the player is not on the profession for that window
    - Terra Window
        - Ranger/Assasin
    - Emo Window
        - Elementalist/Monk
    - Mainteam Window
        - Mesmer/X
        - Ritualist/X
        - Dervish/X
        - Elementalist/Monk

For a detailed description:

- For EMO Helper see [here](./Emo.md)
- For Mainteam Helper see [here](./Mainteam.md)
- For Terra (currently T2) Helper see [here](./Terra.md)
