# CS 300 Final Project (Group9) - Kingdom of Frupal
Drew McKinney, Thomas Abel, Jacob Padgett, Joseph Brower, Mohamed-Amin Cheaito

### How to Compile & Run the Game

1. Copy the files into a directory of your choice and navigate to that directory.
2. Compile the program by running the [make] command.
3. Play the game by running the [./frupal] command.

The map file is included and run by default.
The minimum sized terminal allowed to run the game is 80 columns by 24 rows.



### How to Play

1. You play as the Hero looking for the Royal Diamonds of Frupal.
2. The kingdom is split into two halves by a river through the center, flowing north to south. A bridge connects the eastern and western halves and water surrounds the entire kingdom.
3. The Hero starts in the southeast. Explore the eastern lands until you have enough resources to cross the bridge.
4. On the western side to the north is the Royal Castle of Frupal. Many dangers, and many treasures, await inside. To the south is the village that has its own secrets.
5. There are many clues that hint at the location of the Royal Diamonds. Some may be true, but others may not.

### Tips

1. The Hero starts with only 100 energy and it requires 1 energy to move to a meadow grovenik. Always be aware of your energy and buy as much food as you can find.
2. The binoculars are close to the starting position but cost quite a bit of whiffles. You may choose to purchase them immediately to make exploration easier, or wait a bit to gather some resources.
3. There are multiple ships in the kingdom. The sea hides plenty of small islands with food, treasure, and tools.
4. Beware the Tigers of Frupal.
5. The Royal Diamonds are hidden quite well and require a test of endurance to reach.



### Controls

1. Hero movement
E: North
S: West
D: South
F: East

2. Inspect cursor movement
I: North
J: West
K: South
L: East

3. Options
H: Browse inventory
Q: Quit game

4. Inventory/Tool selection
I: Scroll up
K: Scroll down
H: Exit inventory



### Features

All of the main features of the game were implemented, including: whiffles, energy, exploration, tile types, tools, obstacles, food, treasure, clues, ships, binoculars, diamonds, and a user interface system. The ship was implemented in an interesting way in that it can only be used on water and once the Hero leaves the water the ship is left behind. The map was designed by hand, both the types of tiles and the items strewn about. The user interface was designed to be useable at the minimum resolution, but it can be upscaled to any arbitrary size without issue. All major bugs were fixed.

We were originally planning on attempting the stretch goals, but we quickly realized that our time was short so we decided not to do any of them. If we had had some extra time the level editor would have easily been the first choice, since placing items by hand in a text editor takes quite a while and is prone to errors. A feature we implemented without any real payoff was that maps can be loaded in by passing a text file when starting the game, but since the item file is separate it has limited usage.



### Links

1. Requirements - https://docs.google.com/document/d/1eO3Qxe4bGjRmtbw9keGfjp54r2aGr8LwUNp9Xk3Etuc/edit?usp=sharing
2. Controls - https://docs.google.com/document/d/1Ynmvcy1JkCGKl10r9R4XyHOqnYAbeUh9D1fvDPeDUaY/edit?usp=sharing
3.  UML - https://docs.google.com/drawings/d/1xa9cJx-4tPxWv4_FS4QFIEyKf5LPfYNefdGexXFmJHs/edit?ts=5faa68d7
4. Github - https://github.com/jpadgett314/cs300-frupal-group9
5. Trailer - https://media.pdx.edu/media/1_u44jf79n
6. Presentation - https://media.pdx.edu/media/1_hb1twnff
