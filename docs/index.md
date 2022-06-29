# HelperBox for GuildWars

This software is based on the GWCA API and has some utilities for (low-man) UWSC.

If you want to use ToolBox++ in parallel, you have to start Toolbox **before** starting this HelperBox.  

If you want to launch the HelperBox for a certain GW instance (Multi-launcher use-case), you have to get the process id of the GW instance.  
For that, you can run the **select_instance.py** python script which will scan for all open GW instances.

Or you can launch it by yourself:

```bash
HelperBox.exe /pid PID
```

## Features

### UW Helper

- Terra
    - Ranger/Assasin
    - Mesmer/Elementalist
- Emo
    - Elementalist/Monk
- Dhuum and Vale Bitch
  - Ritualist/Ranger
  - Dervish/Ranger
- Mainteam (Mesmer)
    - Mesmer/Assassin
    - Mesmer/Elementalist

For a detailed description:

- For EMO see [here](./Emo.md)
- For Mainteam Mesmer see [here](./Mainteam.md)
- For Terra see [here](./Terra.md)
- For DB see [here](./Db.md)

### Auto-Follow Button

- If is active (button is green) the targeted player is followed
- Will be inactive when there is no target or the button state was set to inactive
