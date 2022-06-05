# HelperBox for GuildWars

This software is based on the GWCA API and has some utilities for UWSC.

If you want to use ToolBox++ in parallel, you have to start Toolbox **before** starting this HelperBox.  

If you want to launch the HelperBox for a certain GW instance (Multi-launcher use-case), you have to get the process id of the GW instance.

Then launch the HelperBox.exe from the CMD:

```bash
HelperBox.exe /pid PID
```

For that, you can run the **select_instance.py** python script which will scan for all open GW instances.

The helper windows are hidden, whenever:

- The following helper windows are displayed in the UW
    - Terra Window
        - Ranger/Assasin
        - Mesmer/Elementalist
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
- For Terra Helper see [here](./Terra.md)
