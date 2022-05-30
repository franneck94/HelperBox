# EMO Helper

The window will only be rendered if you are on an E/MO.

- Player Bonding:
    - Expected to have some players in the target
    - Balth and Bond will be cast, if not yet present

- Tank Bonding:
    - If no target is selected then the 2nd last player will be targeted in UW, elsewhere the first player
    - Balth, Life, and Bond will be cast if not present

- Fuse Pull:
    1. Cancel Animation
    2. Walk to the target at a distance of 1220
    3. Cancel Animation
    4. Fuse

- Standard Skill Routine - Pumping:
    - On-hold while walking or talking to reaper (yellow colored)
    - Self-Bonds
        - Cast Balth and Bond on self, if not present
        - Whenever Ether is castable, cast it
        - Whenever SB or Burning is not up, cast it
        - If HP or Energy is low, cast sb or burning
    - Turtle
        - If the turtle is nearby spawned, cast bond and life on it
        - If turtle hp <70% fuse
        - Else If turtle hp <90% sb
    - Dhuum fight
        - If GDW is in skillbar, cast on a party member
        - If Wisdom is in the skillbar, cast on cooldown
        - If PI is in the skillbar, cast whenever Dhuum cast judgment

- Move-To Feature:
    - Pre-defined set of positions for E/Mo, like goto Lab Reaper, etc.
    - Move-to button has the name of the next position, if the button is clicked the move-to signal is sent
    - Below the move-to button there is the *Next Pos.* and *Prev Pos.* button to circle in the list of positions
