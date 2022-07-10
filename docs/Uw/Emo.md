# EMO Helper

The window will only be rendered if you are on an E/MO.

- Standard Skill Routine:
    - On-hold while walking or talking to reaper (yellow colored)
    - Self-Bonds
        - Cast Balth and Bond on self, if not present
        - Whenever Ether is castable, cast it
        - Whenever SB or Burning is not up, cast it
        - If HP or Energy is low, cast sb or burning
    - LT
      - On Spawn: Bond LT, or whenever reaper ported and LT has no bonds
      - If LT is in a distance $d \in [1225, 1248]$ and you and the LT are not moving
        - Maintain SB on the LT
        - If LT drops in HP => Fuse
    - Turtle
        - If the turtle is nearby spawned, cast bond and life on it
        - If turtle hp <99% cast fuse and sb if neccessary
    - Dhuum fight
        - If GDW is in skillbar, cast on party member
        - If Wisdom is in the skillbar, cast on cooldown
        - If PI is in the skillbar, cast whenever Dhuum cast judgment

- Move-To Feature:
    - Auto moving at instance load is only enabled for Solo LT tactics
    - Pre-defined set of positions, like goto Lab Reaper, etc.
    - Most position movements are executed automatically
    - The following position movements are triggered by *HM ping* of the LT
        - From Lab Reaper to Fuse Position
        - From Last Keeper to Wastes
